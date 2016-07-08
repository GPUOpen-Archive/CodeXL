//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief GPUPerfServer main program loop. Starts the GPUPerfStudio server,
///        reads command line arguments and starts the target application
//==============================================================================

// Suppress warnings for boost
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning ( push, 3 ) // disable warning level 4 for boost
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

// Boost libraries used for path manipulation
#include <boost/filesystem/path.hpp>

// Boost libraries used to parse command line options
#include <boost/program_options.hpp>

// pop the warning suppression pragmas
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning ( pop )
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #pragma GCC diagnostic pop
#endif

#include "ClientRequestThread.h"

namespace po = boost::program_options;

using namespace std;

#if defined (_WIN32)
    #include <windows.h>
    #include <psapi.h>
    #include <atlbase.h>
    #include <shlobj.h>
    #include <AMDTOSWrappers/Include/osGeneralFunctions.h>
    #include "../MicroDLL/MicroDLLName.h"
#else
    #include <signal.h>
    #include <dirent.h>
    #include <fnmatch.h>
    #include <dlfcn.h>
    extern char* program_invocation_name; ///< Program invocation name
    extern char* program_invocation_short_name; ///< Program invocation short name
#endif

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// from Common
#include "../Common/Logger.h"
#include "../Common/SharedGlobal.h"
#include "../Common/PerfStudioServer_Version.h"
#include "../Common/NamedSemaphore.h"
#include "../Common/OSWrappers.h"
#include "../Common/ProgramInstance.h"
#include "../Common/CommandTimingManager.h"
#include "../Common/HTTPRequest.h"
#include "../Common/misc.h"

#include "WebServer.h"
#include "Registry.h"
#include "ProcessTracker.h"
#include "OSDependent.h"
#include "ServerDefinitions.h"

#ifdef _LINUX
    #include <boost/tokenizer.hpp>
    #include "../Common/Linux/WinDefs.h"

    using boost::tokenizer;
    using boost::escaped_list_separator;

    typedef tokenizer<escaped_list_separator<char> > so_tokenizer;

    using namespace std;

    extern char** environ;
#endif

#ifdef _WIN32
    typedef _TCHAR TARGV; ///< typedef for TARGV
    #define GPS_PLUGIN_DIR "Plugins\\" ///< Plugin dir string definition
    static char s_GPSPluginsDir[PS_MAX_PATH]; ///< Plugin dir path
#else
    typedef char TARGV; ///< typedef for TARGV
    #define GPS_PLUGIN_DIR "Plugins/" ///< Plugin dir string definition
#endif

// Paths to wrappers and shared memory dll's
static gtASCIIString s_LibPath; ///< Path to wrappes and dlls
static gtASCIIString s_WWWRootPath; ///< Path to www root

static std::vector< WrapperInfo > s_pluginArray; ///< List of plugins
static WrapperMap s_wrapperMap; ///< Map of wrappers

WrapperMap g_allowedWrappersMap; ///< Map of allowed wrappers

static gtASCIIString s_OptionAPI;  ///< Used to store value passed in via the --api=  command line option

// Functions required by a plugin
static const char* (* GetPluginVersion)(); ///< Plugin function pointer
static const char* (* GetShortDescription)();///< Plugin function pointer
static const char* (* GetLongDescription)(); ///< Plugin function pointer
static bool (* ProcessRequest)(CommunicationID requestID); ///< Plugin function pointer

// Functions required by a wrapper
static const char* (* GetWrappedDLLName)(); ///< Plugin function pointer
static bool (* UpdateHooks)(); ///< Plugin function pointer

/// indicate which frame to capture. Primarily used with XCaptan/APITrace playback but can also
/// be used to specify a frame to capture from a native application. Frames are 0-based.
#define PS_DEFAULT_CAPTUREFRAME        "-1"     // disabled

/// GPU PerfStudio app name and version string
static const char AppNameAndVersion[] = PERFSTUDIOSERVER_APP_NAME " - Version " PERFSTUDIOSERVER_VERSION_STRING;

/// Gets the wrapper map
/// \return Wrapper map
WrapperMap GetWrapperMap()
{
    return s_wrapperMap;
}

#ifdef _WIN32
/// Restore registry settings to their default values. This includes clearing out the DX Counters entry
/// and the AppInit_DLLs entry (if enabled)
void RestoreRegistrySettings()
{
    if (SG_GET_BOOL(OptionAppInitDll) == true)
    {
        RestoreAppInit();
    }
}
#endif

#ifdef _LINUX
/// Delete the shared memory files. Called when the server starts up and shuts down incase Linux
/// didn't clean up properly (the last instance of server or plugin crashed or was terminated
/// incorrectly)
/// NOTE: No logging should be done in this function (since the log uses a mutex which can be
/// implemented with shared memory)
static void DeleteAllSharedMemories()
{
    // shared memory folder on Ubuntu
    static const char* MEMORY_DIR =  "/dev/shm";

    // list of substrings in shared memory filenames to look for. Any files containing
    // these strings will be removed
    static const char* FILE_TEMPLATES[] =
    {
        "OpenGL",
        "Vulkan",
        "sem.",
        "GPS_",
        "PLUGINS_TO_GPS",
        "ActivePlugins",
        "CLIENT_THREAD_SEMAPHORE",
        "PerfStudioLogfileMutex",
        "PerfStudioSharedGlobals",
    };
    static const int NUM_TEMPLATES = sizeof(FILE_TEMPLATES) / sizeof(FILE_TEMPLATES[0]);

    DIR*              dip;
    struct dirent*    dit;

    // open shared memory directory
    if ((dip = opendir(MEMORY_DIR)) != NULL)
    {
        // build a list of files to remove
        std::vector<std::string> filesToDelete;

        while ((dit = readdir(dip)) != NULL)
        {
            for (int l = 0; l < NUM_TEMPLATES; l++)
            {
                if (strstr(dit->d_name, FILE_TEMPLATES[l]) != NULL)
                {
                    filesToDelete.push_back(dit->d_name);
                    break;
                }
            }
        }

        if (closedir(dip) == -1)
        {
            // error
        }

        // remove the files
        std::vector<std::string>::const_iterator it;

        for (it = filesToDelete.begin(); it != filesToDelete.end(); ++it)
        {
            char fileName[PS_MAX_PATH];
            sprintf_s(fileName, PS_MAX_PATH, "%s/%s", MEMORY_DIR, (*it).c_str());
            remove(fileName);
        }
    }
}

//--------------------------------------------------------------
/// Find the next file in the folder given by the pDirectory
/// structure. Duplicate of the Windows function of the same name
/// and is used to get a list of plugins. The function will also
/// ignore GPA shared libraries
/// \param strSearch File to look for
/// \param pDirectory Directory to look in
/// \param pFile Output file pointer
/// \return True if success, false if fail.
//--------------------------------------------------------------
static bool FindNextFile(const char* strSearch, DIR* pDirectory, struct dirent** pFile)
{
    while (NULL != (*pFile = readdir(pDirectory)))
    {
        if (fnmatch(strSearch, (*pFile)->d_name, 0) == 0)
        {
            if (strstr((*pFile)->d_name, "GPUPerfAPI") == NULL)
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------
/// Parse an application command line string. This is either read
/// as a line from a script file or directly from the command
/// line arguments wrapped in quotes
/// \param cmdLine input command line string
//--------------------------------------------------------------
static void ParseCommandLineString(std::string& cmdLine)
{
    gtASCIIString   appArgs;

    vector<string> argList;
    so_tokenizer tok(cmdLine, escaped_list_separator<char>('\\', ' ', '\"'));

    for (so_tokenizer::iterator tokIt = tok.begin(); tokIt != tok.end(); ++tokIt)
    {
        const char* str = (*tokIt).c_str();

        if (strlen(str) > 0)
        {
            argList.push_back(str);
        }
    }

    ProcessTracker::Instance()->SetInjectedAppName(argList[0].c_str());

    unsigned int numArgs = argList.size();

    for (unsigned int loop = 1; loop < numArgs; loop++)
    {
        char* arg = (char*)argList[loop].c_str();
        appArgs += AddQuotesIfStringHasSpaces(arg);
        appArgs += " ";
    }

    ProcessTracker::Instance()->SetInjectedAppArgs(appArgs.asCharArray());

    fflush(NULL);
}

//--------------------------------------------------------------
/// Read a shell script (cmd.sh), parse it and set up the
/// parameters for PerfStudio to use
/// \param scriptFilename Script file name
/// \return true if successful, false if error
//--------------------------------------------------------------
static bool ReadShellScript(const char* scriptFilename)
{
    std::string line;
    ifstream cmdfile(scriptFilename);

    if (cmdfile.is_open())
    {
        while (cmdfile.good())
        {
            getline(cmdfile, line);

            // make sure line has some data in and that it isn't a comment line
            if (line.length() > 0 && line.at(0) != '#')
            {
                ParseCommandLineString(line);
            }
        }
    }
    else
    {
        return false;
    }

    cmdfile.close();
    return true;
}

#endif

//--------------------------------------------------------------
/// Loads plugins from disk and stores information about each of
/// them in the appropriate collection
//
// \return true if any wrapper information is available; false otherwise
//--------------------------------------------------------------
static bool CollectWrapperInfo()
{
    Log(logMESSAGE, "OSDependent: CollectWrapperInfo\n");

    char strSearch[ PS_MAX_PATH ] = {};
#ifdef _WIN32
    sprintf_s(strSearch, PS_MAX_PATH, "%s%s*.dll", s_LibPath.asCharArray(), s_GPSPluginsDir);
#else
    char dirSearch[PS_MAX_PATH] = {};
    sprintf_s(strSearch, PS_MAX_PATH, "*.so");
    sprintf_s(dirSearch, PS_MAX_PATH, "%s%s", s_LibPath.asCharArray(), GPS_PLUGIN_DIR);
#endif

#if defined (_WIN32)
    // Search all files with .dll extension
    WIN32_FIND_DATA findData;
    HANDLE hHandle = FindFirstFile(strSearch, &findData);

    if (hHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

#else
    DIR* pDirectory = opendir(dirSearch);

    if (pDirectory == NULL)
    {
        return false;
    }

    // get the first file
    struct dirent* pFile = NULL;

    if (FindNextFile(strSearch, pDirectory, &pFile) == false)
    {
        return false;
    }

#endif // _WIN32

    GetPluginVersion    = NULL;
    GetShortDescription = NULL;
    GetLongDescription  = NULL;
    ProcessRequest      = NULL;
    GetWrappedDLLName = NULL;
    UpdateHooks = NULL;

    BOOL bContinue = TRUE;

    while (bContinue)
    {
#ifdef _WIN32
        // call load library on the file
        gtASCIIString strPathToDLL(s_GPSPluginsDir);
        strPathToDLL.append(findData.cFileName);

        // Get the target operating system bitness (32 or 64-bit)
        bool canLoadLibrary = true;
        int addressSpace = 0;

        if (osGetOSAddressSpace(addressSpace) == true)
        {
            if (AMDT_32_BIT_ADDRESS_SPACE == addressSpace)
            {
                // if running on a 32-bit OS, ignore the 64-bit dll's since they
                // won't be recognised by the OS and will generate a "bad image" error
                // on load
                if (strstr(findData.cFileName, "64") != NULL)
                {
                    // it's a 64 bit plugin, so indicate that it isn't to be loaded
                    canLoadLibrary = false;
                }
            }
        }

        HMODULE hModule = NULL;

        if (canLoadLibrary)
        {
            hModule = LoadLibraryEx(strPathToDLL.asCharArray(), NULL, DONT_RESOLVE_DLL_REFERENCES);
        }

#else
        dlerror();                           // Clear any existing error

        // only consider plugins of the same bitness as this server
        const char* pLoc = strstr(pFile->d_name, "32");
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        if (pLoc != NULL)
#else
        if (pLoc == NULL)
#endif
        {
            // the param wasn't found, so return false
            bContinue = FindNextFile(strSearch, pDirectory, &pFile);
            continue;
        }

        char libraryPath[PS_MAX_PATH];
        sprintf_s(libraryPath, PS_MAX_PATH, "%s%s", dirSearch, pFile->d_name);
        LogConsole(logMESSAGE, "library path is %s\n", libraryPath);
        void* hModule = dlopen(libraryPath, RTLD_LAZY);
#endif

        if (hModule != NULL)
        {
#ifdef _WIN32
            // make sure the necessary entrypoints exist
            *((FARPROC*)&GetPluginVersion)    = ::GetProcAddress(hModule, "GetPluginVersion");
            *((FARPROC*)&GetShortDescription) = ::GetProcAddress(hModule, "GetShortDescription");
            *((FARPROC*)&GetLongDescription)  = ::GetProcAddress(hModule, "GetLongDescription");
            *((FARPROC*)&ProcessRequest)      = ::GetProcAddress(hModule, "ProcessRequest");
#else

            GetPluginVersion    = (const char* (*)())dlsym(hModule, "GetPluginVersion");
            GetShortDescription = (const char* (*)())dlsym(hModule, "GetShortDescription");
            GetLongDescription  = (const char* (*)())dlsym(hModule, "GetLongDescription");
            ProcessRequest      = (bool (*)(CommunicationID))dlsym(hModule, "ProcessRequest");
#endif

            if ((GetPluginVersion != NULL) &&
                (GetShortDescription != NULL) &&
                (GetLongDescription != NULL) &&
                (ProcessRequest != NULL))
            {
                // DLL is a plugin!
                WrapperInfo wrapperInfo;
                wrapperInfo.strPluginVersion = GetPluginVersion();
                wrapperInfo.strPluginShortDesc = GetShortDescription();
                wrapperInfo.strPluginLongDesc = GetLongDescription();

                // Now check to see if the plugin is a Wrapper
#ifdef _WIN32
                *((FARPROC*)&GetWrappedDLLName) = ::GetProcAddress(hModule, "GetWrappedDLLName");
                *((FARPROC*)&UpdateHooks) = ::GetProcAddress(hModule, "UpdateHooks");
#else
                GetWrappedDLLName = (const char* (*)())dlsym(hModule, "GetWrappedDLLName");
                UpdateHooks = (bool(*)())dlsym(hModule, "UpdateHooks");
#endif

                bool bPluginIsWrapper = false;

                if ((GetWrappedDLLName != NULL) &&
                    (UpdateHooks != NULL))
                {
                    if (GetWrappedDLLName() != NULL)
                    {
                        wrapperInfo.strWrappedDll = GetWrappedDLLName();
                        bPluginIsWrapper = true;
                    }
                }

                // Is --api option specified - if so, check if this plugin is the one required.
                // otherwise load everything

                Log(logMESSAGE, "API Option: %s compare with %s\n", s_OptionAPI.asCharArray(), wrapperInfo.strPluginShortDesc.asCharArray());

                bool addPlugin = false;

                gtASCIIString DXGIstring = "DXGI";

                if (DXGIstring.compareNoCase(wrapperInfo.strPluginShortDesc) == 0)
                {
                    addPlugin = true;
                }
                else if (s_OptionAPI.compareNoCase(wrapperInfo.strPluginShortDesc) == 0)
                {
                    addPlugin = true;
                }
                else if (s_OptionAPI.isEmpty())
                {
                    addPlugin = true;
                }

                if (addPlugin == true)
                {
                    gtASCIIString strPath(s_LibPath);

#ifdef _WIN32
                    // strip GDT_PROJECT_SUFFIX from the filename to determine
                    // the root of the plugin name
                    gtASCIIString pluginName(findData.cFileName);
                    int pos;

                    if (strlen(GDT_PROJECT_SUFFIX) > 0)
                    {
                        pos = pluginName.find(GDT_PROJECT_SUFFIX);
                    }
                    else
                    {
                        // if there is no project suffix, the file extension still needs to be removed
                        pos = pluginName.find(".dll");
                    }

                    if (pos != -1)
                    {
                        pluginName.truncate(0, pos - 1);
                    }

                    Log(logMESSAGE, "plugin name is %s\n", pluginName.asCharArray());

                    strPath.appendFormattedString("%s%s", s_GPSPluginsDir, pluginName.asCharArray());
#else
                    gtASCIIString pluginName(pFile->d_name);
                    int pos;

                    if (strlen(GDT_PROJECT_SUFFIX) > 0)
                    {
                        pos = pluginName.find(GDT_PROJECT_SUFFIX);
                    }
                    else
                    {
                        // if there is no project suffix, the file extension still needs to be removed
                        pos = pluginName.find(".so");
                    }

                    if (pos != -1)
                    {
                        pluginName.truncate(0, pos - 1);
                    }

                    strPath.appendFormattedString("%s", pluginName.asCharArray());
#endif
                    wrapperInfo.strPluginName = pluginName.asCharArray();
                    wrapperInfo.strPluginPath = strPath;

                    if (bPluginIsWrapper == true)
                    {
                        s_wrapperMap[ wrapperInfo.strPluginShortDesc.asCharArray()] = wrapperInfo;
                    }
                    else
                    {
                        s_pluginArray.push_back(wrapperInfo);
                    }
                }
            }

            // Free the DLL
#ifdef _WIN32
            ::FreeLibrary(hModule);
#else
            dlclose(hModule);
#endif
            GetPluginVersion    = NULL;
            GetShortDescription = NULL;
            GetLongDescription  = NULL;
            ProcessRequest      = NULL;
            GetWrappedDLLName   = NULL;
            UpdateHooks   = NULL;
        }

#ifdef _LINUX
        else
        {
            // If the shared library failed to load, there may be unresolved functions when it was linked
            // use linker option -Wl,-z,defs to see anything undefined in the shared library
            char* error = dlerror();

            if (error != NULL)
            {
                LogConsole(logERROR, "Failed to open shared library %s. Error: %s\n", libraryPath, error);
            }
            else
            {
                LogConsole(logERROR, "Failed to open shared library %s.\n", libraryPath);
            }
        }

        bContinue = FindNextFile(strSearch, pDirectory, &pFile);
#else
        bContinue = FindNextFile(hHandle, &findData);
#endif
    } // End while

#ifdef _WIN32
    FindClose(hHandle);
#else
    closedir(pDirectory);
#endif

    if ((s_pluginArray.empty() == true) && (s_wrapperMap.empty() == true))
    {
        return false;
    }

    if (s_wrapperMap.empty() == false)
    {
        LogConsole(logMESSAGE, "Available Wrappers:\n");

        for (WrapperMap::const_iterator w = s_wrapperMap.begin(); w != s_wrapperMap.end(); ++w)
        {
            gtASCIIString strPluginName = w->second.strPluginName;

            if (SG_GET_BOOL(OptionDllReplacement) == false)
            {
                strPluginName += GDT_PROJECT_SUFFIX "." DLL_EXTENSION;
            }

            LogConsole(logMESSAGE, "   %s\t%s\t%s\n", strPluginName.asCharArray(), w->second.strPluginVersion.asCharArray(), w->second.strPluginShortDesc.asCharArray());
        }

        LogConsole(logRAW, "\n");
    }

    if (s_pluginArray.empty() == false)
    {
        LogConsole(logMESSAGE, "Available Plugins:\n");

        for (unsigned int p = 0; p < s_pluginArray.size(); p++)
        {
            gtASCIIString strPluginName = s_pluginArray[p].strPluginName;

            if (SG_GET_BOOL(OptionDllReplacement) == false)
            {
                strPluginName += GDT_PROJECT_SUFFIX "." DLL_EXTENSION;
            }

            LogConsole(logMESSAGE, "%s\t%s\t%s\n", strPluginName.asCharArray(), s_pluginArray[p].strPluginVersion.asCharArray(), s_pluginArray[p].strPluginShortDesc.asCharArray());
        }

        LogConsole(logRAW, "\n");
    }

    // create a shared memory big enough for the active plugins to register with.
    // there may be more than 1 process loading the plugins, so lets make the shared memory big enough for 10 copies of all the plugins.
    if (smCreate("ActivePlugins", (unsigned long)(s_wrapperMap.size() * 10), PS_MAX_PATH) == false)
    {
        Log(logERROR, "Couldn't create shared memory for receiving injected plugin names\n");
        return false;
    }

    return true;
}

//--------------------------------------------------------------
/// Sends the requested file from the web root to the supplied requestID
/// \param requestID id of the request
/// \param file relative path to a file in the web root directory
/// \param pClientSocket the socket used to send the response
//--------------------------------------------------------------
void ReturnFileFromWebRoot(CommunicationID requestID, const char* file, NetSocket* pClientSocket)
{
    char sPageWithPath[ PS_MAX_PATH ];

    strcpy_s(sPageWithPath, PS_MAX_PATH, s_WWWRootPath.asCharArray());
    strcat_s(sPageWithPath, PS_MAX_PATH, file);
    SendFileResponse(requestID, sPageWithPath, pClientSocket);
}

//--------------------------------------------------------------
/// Sends the requested file from the server root to the supplied requestID
/// \param requestID id of the request
/// \param file relative path to a file in the root directory
/// \param pClientSocket the socket used to send the response
//--------------------------------------------------------------
void ReturnFileFromRoot(CommunicationID requestID, const char* file, NetSocket* pClientSocket)
{
    char sPageWithPath[ PS_MAX_PATH ];

    strcpy_s(sPageWithPath, PS_MAX_PATH, s_LibPath.asCharArray());
    strcat_s(sPageWithPath, PS_MAX_PATH, file);
    SendFileResponse(requestID, sPageWithPath, pClientSocket);
}

//--------------------------------------------------------------
/// Sends a response to the supplied requestID with XML describing
/// the available plugins / wrappers
/// \param requestID the incoming request for wrapper information
/// \param pClientSocket the socket used to send the response
//--------------------------------------------------------------
void DoWrappersCommand(CommunicationID requestID, NetSocket* pClientSocket)
{
    gtASCIIString out;
    gtASCIIString xml;

    for (WrapperMap::const_iterator iter = s_wrapperMap.begin(); iter != s_wrapperMap.end(); ++iter)
    {
        gtASCIIString strPluginName = iter->second.strPluginName;
        strPluginName += GDT_PROJECT_SUFFIX "." DLL_EXTENSION;
        gtASCIIString tmpString = XML("name", XMLEscape(strPluginName).asCharArray());
        tmpString += XML("API", XMLEscape(iter->second.strPluginShortDesc).asCharArray());
        tmpString += XML("Description", XMLEscape(iter->second.strPluginLongDesc).asCharArray());
        xml += XML("wrapper", tmpString.asCharArray());
    }

    xml = XML("WrapperList", xml.asCharArray());
    xml += "\n";
    xml += XML("AppToLaunch", XMLEscape(ProcessTracker::Instance()->GetInjectedAppName().c_str()).asCharArray());
    xml += "\n";
    xml += XML("AppArgs", XMLEscape(ProcessTracker::Instance()->GetInjectedAppArgs().c_str()).asCharArray());

    out = XMLHeader();
    out += XML("PerfStudio" , xml.asCharArray());

    SendXMLResponse(requestID, out.asCharArray(), pClientSocket);
}

//----------------------------------------------------------------
// Performs initialization
//----------------------------------------------------------------
bool OSDependentModulesInitialization()
{
#if defined (_WIN32)
    // We want to ensure that the communication is handled at a relatively-high priority
    // so set this process to have above-normal priority.
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

    //this thread generally just waits for a shutdown command,
    //so it can have normal priority
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#else
#pragma message "IMPLEMENT ME! (if thread priority needs changing)"
#endif

    return true;
}

//----------------------------------------------------------------
// Surrounds a string with quotes if it has any spaces
//----------------------------------------------------------------
gtASCIIString AddQuotesIfStringHasSpaces(const char* arg)
{
    gtASCIIString argument = arg;

    // if the argument has a space add quotes
    if (argument.find(" ") != (int)std::string::npos)
    {
        gtASCIIString quotes = "\"";
        gtASCIIString str = quotes;
        str += argument;
        str += quotes;
        return str;
    }
    else
    {
        return argument;
    }
}

//--------------------------------------------------------------
/// Closes all the shared memories that were created
//--------------------------------------------------------------
static void CloseAllSharedMemories()
{
    // close all shared memories
    for (WrapperMap::iterator iter = g_activeWrappersMap.begin();
         iter != g_activeWrappersMap.end();
         ++iter)
    {
#ifdef WIN32
        const char* memoryName = iter->first.c_str();
#else
        // rebuild the correct memory name from the plugin. The '/' can't be used in a filename
        // in Linux, so it's replaced with a '.'
        unsigned long pid = 0;
        char pluginShortDesc[ PS_MAX_PATH ];
        sscanf_s(iter->first.c_str(), "%lu/%s", &pid, pluginShortDesc, sizeof(pluginShortDesc));

        // the '/' character can't be used as a filename in Linux, so just use the plugin name as the shared memory name
        // (ignore the process ID)
        char memoryName[PS_MAX_PATH];
        sprintf_s(memoryName, PS_MAX_PATH, "%lu %s", pid, pluginShortDesc);
#endif
        smClose(memoryName);
    }

#ifdef _WIN32
    smClose("GPS_TO_MDLL");
#endif
    smClose("ActivePlugins");
}

/// initial setup of shared memory values
static bool OSDependentGlobalsInitialization()
{
    SG_SET_PATH(ServerPath, s_LibPath.asCharArray());
    gtASCIIString pathString;

#ifdef _WIN32
    pathString = s_LibPath;
    pathString += MICRODLLNAME GDT_PROJECT_SUFFIX ".dll";
    SG_SET_PATH(MicroDLLPath, pathString.asCharArray());
#endif

    pathString = s_LibPath;
#ifdef _WIN32
    pathString += s_GPSPluginsDir;
#else
    pathString += GPS_PLUGIN_DIR;
#endif
    SG_SET_PATH(PluginsPath, pathString.asCharArray());

    // set the media directory to be the one from the tree
    if (s_LibPath.find("bin\\debug") >= 0 ||
        s_LibPath.find("bin\\release") >= 0)
    {
        s_WWWRootPath = s_LibPath;
        s_WWWRootPath += "..\\..\\media\\";
    }
    else
    {
        s_WWWRootPath = s_LibPath;
        s_WWWRootPath += "media\\";
    }

    const char* cpPerfAPIPath = SG_GET_PATH(GPUPerfAPIPath);

    if (strcmp(cpPerfAPIPath, "") == 0)
    {
        SG_SET_PATH(GPUPerfAPIPath, s_LibPath.asCharArray());
    }

    return true;
}

//--------------------------------------------------------------
// generates an array of wrappers that are injected into the
// sepecified process
// \param uPID process to check for injected wrappers
// \param rWrapperArray array that will contain the injected wrappers
//     after this call is complete.
// \return true if there were no errors; false otherwise
//--------------------------------------------------------------
#if 0
// Unused code
bool GetListOfInjectedWrappers(unsigned long uPID, std::vector< WrapperInfo >& rWrapperArray)
{
    bool bRes = true;

    for (WrapperMap::const_iterator iter = s_wrapperMap.begin();
         iter != s_wrapperMap.end();
         ++iter)
    {
        std::vector< ProcessInfo > procList;
        bRes = bRes && get_process_list(iter->second.strPluginName.asCharArray(), procList);

        for (size_t p = 0; p < procList.size(); p++)
        {
            if (procList[ p ].th32ProcessID == uPID)
            {
                rWrapperArray.push_back(iter->second);
            }
        }
    }

    return bRes;
}
#endif

/// Extracts the full path and arguments from a Shell link object (shortcut).
/// Note: This function is called before the logfile is initialized so all error messages go directly to stdout
/// \param shortcutPath - input string that specifies a path and file name of a shortcut
/// \param exePath - output path to executable
/// \param exeDirectory - working directory for executable
/// \param exeArgs - working directory for arguments
/// \return         true on success, false on failure. Prints an error message on stdout on failure

static bool ResolveShortcut(const char* shortcutPath, gtASCIIString& exePath, gtASCIIString& exeDirectory, gtASCIIString& exeArgs)
{
#if defined (_WIN32)
    CComPtr<IShellLink> ipShellLink; // buffer that receives the null-terminated string
    // for the drive and path
    WCHAR wszTemp[MAX_PATH];         //Unicode string required for ShellLink interface

    // Get a pointer to the IShellLink interface
    if (S_OK != CoInitialize(NULL))
    {
        return false;
    }

    if (S_OK != CoCreateInstance(CLSID_ShellLink,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_IShellLink,
                                 (void**)&ipShellLink))
    {
        cout << "Error: Unable to get a pointer to the IShellLink Interface\n";
        return false;
    }

    // Get a pointer to the IPersistFile interface
    CComQIPtr<IPersistFile> ipPersistFile(ipShellLink);

    // IPersistFile is using LPCOLESTR, so make sure that the string is Unicode

    MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wszTemp, MAX_PATH);

    // Open the shortcut file and initialize it from its contents
    if (S_OK != ipPersistFile->Load(wszTemp, STGM_READ))
    {
        cout << "Error: Unable to open the shortcut file\n";
        return false;
    }

    // Try to find the target of a shortcut, even if it has been moved or renamed

    if (S_OK != ipShellLink->Resolve(NULL, SLR_UPDATE))
    {
        cout << "Error: Unable to resolve the shortcut file\n";
        return false;
    }

    char tmpString[4096]; // Assumes that arguments list won't be more than 4K long

    // Get the path to the shortcut target
    if (S_OK != ipShellLink->GetPath(tmpString, MAX_PATH, NULL, SLGP_RAWPATH))
    {
        cout << "Error: Unable to retrieve exe path from shortcut\n";
        return false;
    }

    exePath = tmpString;

    // Get the working directory for the shortcut target
    if (S_OK != ipShellLink->GetWorkingDirectory(tmpString, MAX_PATH))
    {
        cout << "Error: Unable to retrieve working directory from shortcut\n";
        return false;
    }

    exeDirectory = tmpString;

    if (S_OK != ipShellLink->GetArguments(tmpString, 4096))
    {
        cout << "Error: Unable to retrieve arguments from shortcut\n";
        return false;
    }

    exeArgs = tmpString;

    return true;
#else
#pragma message "IMPLEMENT ME! (if dragging shortcut is supported)"

    PS_UNREFERENCED_PARAMETER(shortcutPath);
    PS_UNREFERENCED_PARAMETER(exePath);
    PS_UNREFERENCED_PARAMETER(exeDirectory);
    PS_UNREFERENCED_PARAMETER(exeArgs);

    return false;
#endif
}

///  Wait for specified number of milli seconds.
/// \param waitTime Number of milli seconds to wait
void PrintWait(UINT waitTime)
{
    LogConsole(logERROR, "Closing in %.2f seconds...\n", (float(waitTime) / float(1000)));
    osSleep(waitTime);
}

#ifdef _LINUX
//--------------------------------------------------------------
/// SendShutdownRequest Open a client socket and send a shutdown
/// request to the thread listening for client connections.
/// This has the same effect as sending a shutdown from
/// the PerfStudio client or via a web browser
//--------------------------------------------------------------
static void SendShutdownRequest()
{
    // create a client socket and send a shutdown message to the server. This
    // will then send a signal to the other threads to request they terminate.
    // Make sure that the request is only sent once.
    static bool requestSent = false;

    if (false == requestSent)
    {
        DWORD port = GetWebServerPort();
        NetSocket* client_socket = NetSocket::Create();

        if (client_socket != NULL)
        {
            osPortAddress portAddress((unsigned short)port);
            client_socket->Connect(portAddress);

            char sendData[PS_MAX_PATH];
            sprintf_s(sendData, PS_MAX_PATH, "GET /Shutdown HTTP/1.1 \r\n\r\n");
            client_socket->Send(sendData, (int)strlen(sendData));
            osSleep(100);
            client_socket->close();               // close will delete the instance
            requestSent = true;
            LogConsole(logMESSAGE, "PerfServer shutting down\n");

            // reset the library path in shared memory
            SG_SET_PATH(GPUPerfAPIPath, "");
        }
    }
}
#endif // _LINUX


/// Registered as a callback when the user closes the server console window
/// see http://msdn.microsoft.com/en-us/library/ms683242%28v=VS.85%29.aspx for more info
/// \param dwCtrlType CTRL type
/// \return Always returns true
BOOL ConsoleCloseHandler(DWORD dwCtrlType)
{
    PS_UNREFERENCED_PARAMETER(dwCtrlType);
    // someone is trying to close the server
    // this happens in a separate thread, and seems to quit early sometimes...

#ifdef _WIN32
    // first make sure the reg key is removed
    RestoreRegistrySettings();
#endif

#ifdef _LINUX
    SendShutdownRequest();
#else
    // then set the shutdown event so that (hopefully) everything else is triggered to clean up.
    g_shutdownEvent.Signal();
#endif

    // return true to indicate that we handled the event
    return TRUE;
}

#ifdef _LINUX
//--------------------------------------------------------------
/// Handler that gets called when the SIGHUP signal is caught
/// \return Always returns true
//--------------------------------------------------------------
BOOL SigHupHandler(DWORD dwCtrlType)
{
    PS_UNREFERENCED_PARAMETER(dwCtrlType);
    SendShutdownRequest();
    return TRUE;
}

//--------------------------------------------------------------
/// Handler that gets called when the SIGTERM signal is caught
/// \return Always returns true
//--------------------------------------------------------------
BOOL SigTermHandler(DWORD dwCtrlType)
{
    PS_UNREFERENCED_PARAMETER(dwCtrlType);
    SendShutdownRequest();
    return TRUE;
}

//--------------------------------------------------------------
/// Set up a signal handler. This sets up a callback that gets
/// called when the signal is caught
/// \param handler Signal handler callback function
/// \signum the signal ID (SIGINT, SIGHUP etc)
//--------------------------------------------------------------
static void SetSignalHandler(__sighandler_t handler, int signum)
{
    // use the sigaction() function to capture the signal on Linux.
    // signal() could be used but sigaction is more robust
    struct sigaction new_action, old_action;

    // Set up the structure to specify the new action
    new_action.sa_handler = handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    // set up the handler.
    sigaction(signum, NULL, &old_action);

    if (old_action.sa_handler != SIG_IGN)
    {
        sigaction(signum, &new_action, NULL);
    }
}

//--------------------------------------------------------------
/// Linux-specific implementation of the Windows function of the
/// same name. Set up a handler which gets executed when Ctrl-C
/// is pressed
/// \param handler Signal handler
//--------------------------------------------------------------
static void SetConsoleCtrlHandler(__sighandler_t handler)
{
    SetSignalHandler(handler, SIGINT);
}
#endif

//--------------------------------------------------------------
/// Parse the command line parameters provided to the server
/// NOTE: This currently uses boost and has been broken out
/// so that a different solution can be used on Linux.
/// \param argc number of command line parameters
/// \param argv pointer to a list of command line arguments
/// \param optionPort port to use
/// \param remoteLaunch whether or not to remote launch the application (currently Linux only)
/// \return 0 if OK, anything else is an error
//--------------------------------------------------------------
static int ParseCommandLine(int argc, TARGV* argv[], int* optionPort, bool* remoteLaunch)
{
#ifdef _LINUX
    bool useScript = false;
#else
    PS_UNREFERENCED_PARAMETER(remoteLaunch);
#endif

    //----- Command Line Options Parsing
    try
    {
        // Parse command line options using Boost library
        // Note: Logfile not used until after options are parsed - all messages use iostream or printf

        // Declare a group of options that will be allowed only on command line
        po::options_description generic("Generic Options");
        generic.add_options()
        ("help,h", "Show this help message.")
#ifdef _WIN32
        ("openwith", "Add an <Open with GPU PerfServer> option to the right-click context menu in Windows.")
        ("clean", "Remove any changes made to the system configuration by GPU PerfServer. This includes the <Open with GPU PerfServer> option on the right-click context menu and the enabling of DX hardware performance counters in the driver.")
#else
        ("script,S", po::value<string>(), "Run the application as specified by the shell script file, located in the target directory. This allows applications with complex arguments to be executed.")
        ("remote,R", "Allow remote launching of applications. Start the GPU PerfServer first, then launch the app.")
        ("es,E", "LD_PRELOAD the GLES plugin. By default, the GL plugin gets preloaded.")
        ("vk,V", "LD_PRELOAD the Vulkan plugin.")
#endif
        ("version,v", "Print the GPU PerfServer version number.")
        ("appargs", po::value< vector<string> >(), "Specified once for application path, and again for each of the application's custom arguments.")
        ("appworkingdir", po::value<string>(), "Working directory to set when launching application.")
        ;

        // Declare a group of options that will be allowed both on command line
        // and in config file
        po::options_description config("Configuration Options (these may also be specified in " PS_CONFIG_FILE ").");
        config.add_options()
        ("no-logfile", "Disable writing to log file.")
        ("logfile,l", po::value<string>(), "Path to Logfile ( default = " PS_DEFAULT_LOGFILE " ).")
        ("loglevel,L", po::value<int>(), "Verbosity of LogFile messages( default = " PS_DEFAULT_LOGLEVEL " ).")
        ("port,p", po::value<int>(optionPort), "Web Server Port ( default = port " PS_DEFAULT_PORT " )")
        ("real-pause,r", "Enable Real Pause mode. \nWhen this option is enabled, GPU PerfServer will return a delta time value of 0 to the application when paused in the frame debugger. Note: many applications will crash if this option is enabled, however if supported, using this option can give better debugging and profiling results.")
        ("speed,s", po::value<float>(), "The playback speed of the application. \nSet this option to 0 and enable real-pause (if supported by the application) to pause on the first frame.")
        ("api,a", po::value<string>(), "Force GPU PerfServer to only load a single 3D API plug-in (dx11, opengl, opengles).")
#ifdef _WIN32
        ("debug-runtime,d", "Enable/Disable the D3D Debug Runtime\nWhen this option is enabled, GPU PerfServer will force the DX Debug Runtime to be used.")
        ("force-ref-rast", "GPU PerfServer will force the reference rasterized device to be used.")
        ("warp-device,w", "Use the D3D WARP device.\nWhen this option is enabled, GPU PerfServer will force the D3D WARP device to be used.")
        ("hopcount-max,H", po::value<int>(), "Max number of process hops before the server binds to the current process.")
        ("dll-replacement,D", "Use DLL Replacement rather than API-Hooking.")
        ("manual-dll-replacement,M", "Use manual DLL Replacement. Doesn't use Dll injection or API-Hooking. Dll's need to be manually copied to the application folder.")
        ("use-appinit-dll,A", "Use the AppInit_DLLs registry entry to inject MicroDll into applications. Requires GPU PerfServer to be run with elevated privileges.")
        ("appinit-dll-filelist,F", "List of executable files to inject " MICRODLLNAME "into, comma delimited. Specify 'all' to inject into all executables.")
#endif
        ("capture-frame,C", po::value<int>(), "Specify which frame to capture (1-based).\nWhen a non-negative value is specified, the application is executed until the required number of frames have been rendered and a frame capture will be performed automatically without the client being connected. ( default = " PS_DEFAULT_CAPTUREFRAME " (no autocapture) )")
        ("wireframe-color,W", po::value<int>(), "Specify color to use for wireframe display ( default = " PS_DEFAULT_WIREFRAMECOLOR " ).")
        ("wireframe-overlay", "Enable/disable wireframe overlay.")
#ifdef _WIN32
        ("liquidvr,V", "Enable support for LiquidVR applications. When enabled, PerfStudio will delay interception of the DX Device until after LiquidVR has wrapped it.")
#endif
        ;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden Options");
        hidden.add_options()
        ("gpa-counters,c", po::value<string>(), "Path to the GPUPerfAPI counter selection file ( default = " PS_DEFAULT_COUNTERFILE " ).")
        ("break", "Break for debugger attach at launch.")
        ("time-override-mode", po::value<string>(), "Time Override Mode.")
        ("copy-mapped-buffers-using-cpu", "Copy mapped buffers using CPU.")
        ("frame-capture-on-pause", "Frame capture on pause.")
        ("flatten-command-lists", "Flatten command lists.")
        ("filter-draw-calls", po::value<int>(), "Filter non-Draw/Dispatch drawcalls.")
        ("trace-type", po::value<string>(), "Specify which type of trace to capture on the frame specified by 'CaptureFrame'.")
#ifdef _WIN32
        ("enable-dx-counters", "Turn on hardware performance counters in the DirectX Driver.")
#endif
        ("gpa-dllpath", po::value<string>(), "Directory containing the GPUPerfAPI DLLs.")
        ("layer-flag,l", po::value<unsigned int>(), "Bitfield to control disabling specific server layers for debugging.")
        ("framerate-stats-collection", "Enable the collection of minimum, maximum, and average framerate over a duration specified in milliseconds.")
        ("framerate-stats-duration,l", po::value<unsigned int>(), "The total duration to collect frame statistics for after stats collection has been triggered.")
        ("framerate-stats-trigger,l", po::value<unsigned int>(), "The collection trigger is any valid Virtual-Key Code used to start the collection of frame statistics.")
        ("sd-disable-depth-copy", "Disable copying the depth buffer with the shader debugger.")
        ("no-process-track", "Disable process tracking during startup.")
        ("mdo-mode", po::value<unsigned int>(), "Map Delta Optimization: Decide how to enable it. 0 = disabled, 1 = per-map delta storage, 2 = per-byte delta storage.")
        ("hopcount,y", po::value<int>(), "Current number of process hops.")
        ;

        // all options available from command line
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        // subset available for specification in config file
        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        // options available to user and described in help
        po::options_description visible("Allowed Options");
        visible.add(generic).add(config);

        // All options that don't have an explicit '-' in front will be assumed to be an application to launch
        po::positional_options_description p;
        p.add("appargs", -1);

        po::variables_map vm;

        // Parse command line
        store(po::command_line_parser(argc, argv).
              options(cmdline_options).positional(p).run(), vm);

        // Parse config file

        gtASCIIString configFile = s_LibPath;
        configFile += PS_CONFIG_FILE;

        ifstream ifs(configFile.asCharArray());
        store(parse_config_file(ifs, config_file_options), vm);

        // Handle Options
        notify(vm);

        if (vm.count("help"))
        {
            cout << visible << "\n";
            return 1;
        }

        if (vm.count("version"))
        {
            cout << PERFSTUDIOSERVER_APP_NAME << " - " << PERFSTUDIOSERVER_VERSION_STRING << "\n";
            return 1;
        }

#ifdef _WIN32

        if (vm.count("openwith"))
        {
            SetOpenWithRegistryKey();
            return 1;
        }

        if (vm.count("clean"))
        {
            DeleteOpenWithRegistryKey();
            RestoreRegistrySettings();
            return 1;
        }

#else
        const char* scriptFileName = "cmd.sh";

        if (vm.count("script"))
        {
            useScript = true;
            scriptFileName = vm["script"].as<string>().c_str();
        }

        if (vm.count("remote"))
        {
            *remoteLaunch = true;
        }

        if (vm.count("es"))
        {
            ProcessTracker::Instance()->SetInjectedAppPluginName("GLES");
        }

        if (vm.count("vk"))
        {
            ProcessTracker::Instance()->SetInjectedAppPluginName("Vulkan");
        }

#ifdef CODEXL_GRAPHICS
        // CodeXL will default to Vulkan on Linux
        ProcessTracker::Instance()->SetInjectedAppPluginName("Vulkan");
#endif // CODEXL_GRAPHICS

#endif // def _WIN32

        if (vm.count("loglevel"))
        {
            SG_SET_INT(OptionLogLevel, vm["loglevel"].as<int>());
        }
        else
        {
            SG_SET_INT(OptionLogLevel, atoi(PS_DEFAULT_LOGLEVEL));
        }

        if (vm.count("wireframe-overlay"))
        {
            SG_SET_BOOL(OptionWireFrameOverlay, vm["wireframe-overlay"].as<bool>());
        }
        else
        {
            // default to display the overlay
            SG_SET_BOOL(OptionWireFrameOverlay, true);
        }

        // Set wireframe color
        if (vm.count("wireframe-color"))
        {
            SG_SET_INT(OptionWireFrameColor, vm["wireframe-color"].as<int>());
        }
        else
        {
            // default to RED/pink
            SG_SET_INT(OptionWireFrameColor, atoi(PS_DEFAULT_WIREFRAMECOLOR));
        }

        // read logfile from options line (or default)
        boost::filesystem::path logfile;

        if (vm.count("logfile"))
        {
            logfile = boost::filesystem::path(vm["logfile"].as<string>().c_str(), (void*)boost::filesystem::native);
        }
        else
        {
            logfile = boost::filesystem::path(PS_DEFAULT_LOGFILE, (void*)boost::filesystem::native);
        }

        // set up PerfStudio subdirectory string
        gtString gpsFolder;
        gpsFolder.fromASCIIString(GetPerfStudioDirName());
        osDirectory tempDir;

        // log file goes into temp folder
        osFilePath logFilePath;
        logFilePath.setPath(osFilePath::OS_TEMP_DIRECTORY);
        logFilePath.appendSubDirectory(gpsFolder);

        tempDir.setDirectoryFullPathFromString(logFilePath.fileDirectoryAsString());

        if (!tempDir.exists())
        {
            if (!tempDir.create())
            {
                // if fail to create GPUPerfStudio subdirectory, just use temp root
                logFilePath.setPath(osFilePath::OS_TEMP_DIRECTORY);
            }
        }

        // check if this is a relative or absolute path to the logfile
        if (!logfile.has_root_path())
        {
            // relative path - add GPUPerfServer.exe directory name to generate full path
            logfile = logFilePath.fileDirectoryAsString().asCharArray() / logfile;
        }

        //      LogConsole(logMESSAGE, "Temp path is %s\n", logfile.string().c_str());
        SG_SET_PATH(LogfilePath, logfile.string().c_str());

        // Set GPUPerfAPI related settings
        if (vm.count("gpa-dllpath"))
        {
            // assume that this path if specified is absolute
            SG_SET_PATH(GPUPerfAPIPath, vm[ "gpa-dllpath" ].as<string>().c_str());
        }

        // read GPA counters file from options line (or default)
        boost::filesystem::path counterFile;

        if (vm.count("gpa-counters"))
        {
            // read counters file from options line (or default)
            counterFile = boost::filesystem::path(vm["gpa-counters"].as<string>().c_str(), (void*)boost::filesystem::native);
        }
        else
        {
            counterFile = boost::filesystem::path(PS_DEFAULT_COUNTERFILE, (void*)boost::filesystem::native);
        }

        // counter settings file goes into roaming folder. It's customary to put all app folders into a company
        // folder first (ie AMD/GPUPerfStudio)
        osFilePath counterFilePath;
        counterFilePath.setPath(osFilePath::OS_USER_APPLICATION_DATA);

        // first, create the AMD folder
        gtString amdFolder;
        amdFolder.fromASCIIString("AMD");
        counterFilePath.appendSubDirectory(amdFolder);

        tempDir.setDirectoryFullPathFromString(counterFilePath.fileDirectoryAsString());
        bool folderExistsAMD = true;

        if (!tempDir.exists())
        {
            if (!tempDir.create())
            {
                // if fail to create the AMD subdirectory, just use temp root
                counterFilePath.setPath(osFilePath::OS_USER_APPLICATION_DATA);
                folderExistsAMD = false;
            }
        }

        if (true == folderExistsAMD)
        {
            // AMD folder exists or created, so create GPUPerfStudio subdirectory
            counterFilePath.appendSubDirectory(gpsFolder);
            tempDir.setDirectoryFullPathFromString(counterFilePath.fileDirectoryAsString());

            if (!tempDir.exists())
            {
                if (!tempDir.create())
                {
                    // if fail to create GPUPerfStudio subdirectory, just use temp root/AMD
                    counterFilePath.setPath(osFilePath::OS_USER_APPLICATION_DATA);
                    counterFilePath.appendSubDirectory(amdFolder);
                }
            }
        }

        // check if this is a relative or absolute path to the counters file
        if (!counterFile.has_root_path())
        {
            // relative path - add GPUPerfServer.exe directory name to generate full path
            counterFile = counterFilePath.fileDirectoryAsString().asCharArray() / counterFile;
        }

        //      LogConsole(logMESSAGE, "Roaming folder is %s\n", counterFile.string().c_str());
        SG_SET_PATH(CounterFile, counterFile.string().c_str());

        // Load only a single API (uses plugin short name for comparison)
        if (vm.count("api"))
        {
            s_OptionAPI = vm["api"].as<string>().c_str();
        }

        // Is logfile writing enabled?
        if (vm.count("no-logfile"))
        {
            SG_SET_BOOL(OptionNoLogfile, true);
        }
        else
        {
            SG_SET_BOOL(OptionNoLogfile, false);
        }

        // Is the Debug Runtime enabled
        if (vm.count("force-ref-rast"))
        {
            SG_SET_BOOL(OptionForceRefRast, true);
        }
        else
        {
            SG_SET_BOOL(OptionForceRefRast, false);
        }

        // Is the Debug Runtime enabled
        if (vm.count("debug-runtime"))
        {
            SG_SET_BOOL(OptionDebugRuntime, true);
        }
        else
        {
            SG_SET_BOOL(OptionDebugRuntime, false);
        }

        // Is the Warp Device enabled
        if (vm.count("warp-device"))
        {
            SG_SET_BOOL(OptionWARPDevice, true);
        }
        else
        {
            SG_SET_BOOL(OptionWARPDevice, false);
        }

        // Does pause do super slow motion or actual pause?
        if (vm.count("real-pause"))
        {
            SG_SET_BOOL(OptionRealPause, true);
        }
        else
        {
            SG_SET_BOOL(OptionRealPause, false);
        }

        // Set the default playback speed
        if (vm.count("speed"))
        {
            SG_SET_FLOAT(OptionSpeed, vm["speed"].as<float>());
        }
        else
        {
            SG_SET_FLOAT(OptionSpeed, 1.0f);
        }

        // Set Application Process Attach breakpoint
        if (vm.count("break"))
        {
            SG_SET_BOOL(OptionBreak, true);
        }
        else
        {
            SG_SET_BOOL(OptionBreak, false);
        }

        // Set Application to not use Process Tracking
        if (vm.count("no-process-track"))
        {
            SG_SET_BOOL(OptionNoProcessTrack, true);
        }
        else
        {
            SG_SET_BOOL(OptionNoProcessTrack, false);
        }

        // Set the process hop count
        if (vm.count("hopcount-max"))
        {
            SG_SET_INT(OptionHopCountMax, vm["hopcount-max"].as<int>());
        }
        else
        {
            SG_SET_INT(OptionHopCountMax, 100);
        }

        // Set frame to capture
        if (vm.count("capture-frame"))
        {
            SG_SET_INT(OptionCaptureFrame, vm["capture-frame"].as<int>());
        }
        else
        {
            // default to 0
            SG_SET_INT(OptionCaptureFrame, atoi(PS_DEFAULT_CAPTUREFRAME));
        }

        // Set "Time Override Mode" for autocapture
        if (vm.count("time-override-mode"))
        {
            const char* timeOverride = vm["time-override-mode"].as<string>().c_str();

            const char firstLetter = timeOverride[0];

            if (firstLetter == 'F' || firstLetter == 'f')
            {
                SG_SET_INT(OptionTimeOverrideMode, TIME_OVERRIDE_FREEZE);
            }
            else if (firstLetter == 'S' || firstLetter == 's')
            {
                SG_SET_INT(OptionTimeOverrideMode, TIME_OVERRIDE_SLOWMOTION);
            }
            else
            {
                SG_SET_INT(OptionTimeOverrideMode, TIME_OVERRIDE_NONE);
            }
        }
        else
        {
            SG_SET_INT(OptionTimeOverrideMode, TIME_OVERRIDE_FREEZE);
        }

        // Set the "TraceType" flags. This only applies to AutoCapture in Mantle/DX12.
        int32 traceTypeFlags = kTraceType_None;

        if (vm.count("trace-type"))
        {
            gtASCIIString traceTypeString(vm["trace-type"].as<string>().c_str());

            if (traceTypeString.compareNoCase("API") == 0)
            {
                traceTypeFlags = kTraceType_API;
            }
            else if (traceTypeString.compareNoCase("GPU") == 0)
            {
                traceTypeFlags = kTraceType_GPU;
            }
            else if (traceTypeString.compareNoCase("Linked") == 0)
            {
                traceTypeFlags = kTraceType_Linked;
            }
        }

        SG_SET_INT(OptionTraceType, traceTypeFlags);

        // Set "Copy mapped buffers using CPU" for autocapture
        if (vm.count("copy-mapped-buffers-using-cpu"))
        {
            SG_SET_BOOL(OptionCopyMappedBuffersUsingCPU, true);
        }
        else
        {
            SG_SET_BOOL(OptionCopyMappedBuffersUsingCPU, false);
        }

        // Set "Flatten CommandLists" for autocapture
        if (vm.count("flatten-command-lists"))
        {
            SG_SET_BOOL(OptionFlattenCommandLists, true);
        }
        else
        {
            SG_SET_BOOL(OptionFlattenCommandLists, false);
        }

        // Enable LiquidVR support
        if (vm.count("liquidvr"))
        {
            SG_SET_BOOL(OptionLiquidVR, true);
        }
        else
        {
            SG_SET_BOOL(OptionLiquidVR, false);
        }

        // Set "Filter non-Draw/Dispatch draw calls" for autocapture
        if (vm.count("filter-draw-calls"))
        {
            SG_SET_INT(OptionFilterDrawCalls, vm["filter-draw-calls"].as<int>());
        }
        else
        {
            SG_SET_BOOL(OptionFilterDrawCalls, 0);
        }

        // Set the initial hop count
        SG_SET_INT(OptionHopCount, 0);

#ifdef _WIN32
        // Set the initial value for whether Steam.exe was injected with MicroDLL
        SG_SET_BOOL(SteamInjected, false);

        // Is Dll replacement being used
        if (vm.count("dll-replacement"))
        {
            SG_SET_BOOL(OptionDllReplacement, true);
        }
        else
        {
            SG_SET_BOOL(OptionDllReplacement, false);
        }

        // Is manual Dll replacement being used
        if (vm.count("manual-dll-replacement"))
        {
            SG_SET_BOOL(OptionManualDllReplacement, true);
        }
        else
        {
            SG_SET_BOOL(OptionManualDllReplacement, false);
        }

        // Is the AppInit_DLL registry setting being used
        if (vm.count("use-appinit-dll"))
        {
            SG_SET_BOOL(OptionAppInitDll, true);
        }
        else
        {
            SG_SET_BOOL(OptionAppInitDll, false);
        }

        // Is the AppInit_DLL registry setting being used
        if (vm.count("appinit-dll-filelist"))
        {
            SG_SET_PATH(AppInitDllFileList, vm["appinit-dll-filelist"].as<string>().c_str());
        }

#endif

        // Set Application Debug flag
        if (vm.count("layer-flag"))
        {
            SG_SET_UINT(OptionLayerFlag, vm["layer-flag"].as<unsigned int>());
        }
        else
        {
            SG_SET_UINT(OptionLayerFlag, 0);
        }

        // Set if framerate statistics collection is enabled.
        if (vm.count("framerate-stats-collection"))
        {
            SG_SET_BOOL(OptionCollectFrameStats, true);
        }
        else
        {
            SG_SET_BOOL(OptionCollectFrameStats, false);
        }

        // Set the framerate statistics duration.
        if (vm.count("framerate-stats-duration"))
        {
            SG_SET_UINT(OptionStatsDuration, vm["framerate-stats-duration"].as<unsigned int>());
        }
        else
        {
            // If not set within the configuration file, default the stats collection to 60 seconds.
            SG_SET_UINT(OptionStatsDuration, 60000);
        }

        // Set the framerate statistics trigger key
        if (vm.count("framerate-stats-trigger"))
        {
            unsigned int triggerValue = vm["framerate-stats-trigger"].as<unsigned int>();
            SG_SET_UINT(OptionStatsTrigger, triggerValue);
        }
        else
        {
            // If not specified, use the "5" number key to collect frame statistics.
            SG_SET_UINT(OptionStatsTrigger, '5');
        }

        // Disable copying the depth buffer with the shader debugger?
        if (vm.count("sd-disable-depth-copy"))
        {
            SG_SET_BOOL(OptionSDDisableDepthCopy, true);
        }
        else
        {
            SG_SET_BOOL(OptionSDDisableDepthCopy, false);
        }

        if (vm.count("mdo-mode"))
        {
            SG_SET_UINT(OptionMdoMode, vm["mdo-mode"].as<unsigned int>());
        }
        else
        {
            SG_SET_UINT(OptionMdoMode, 0);
        }

        g_bAppSpecifiedAtCmdLine = false;

        // All command line arguments that are not explicitly options will be mapped here
        if (vm.count("appargs"))
        {
            vector <string> app = vm["appargs"].as< vector<string> >();
            g_bAppSpecifiedAtCmdLine = true;

            // Check to determine if this file is a shortcut
            const char* tmpStr = app.at(0).c_str();
            size_t tmpLen = strlen(tmpStr);

            if ((tmpLen > 5)
                && ((_stricmp(&tmpStr[tmpLen - 4], ".lnk") == 0)
                    || _stricmp(&tmpStr[tmpLen - 5], ".lnk\"") == 0))
            {
                // It's a shortcut - extract the app name and arguments
                gtASCIIString appName;
                gtASCIIString appDir;
                gtASCIIString appArgs;

                if (!ResolveShortcut(tmpStr, appName, appDir, appArgs))
                {
                    // Unable to resolve shortcut - exit
                    cout << "Unable to resolve shortcut" << app.at(0) << "\n";
                    return -1;
                }

#ifdef _WIN32
                // Sometimes, windows changes the appName when it shouldn't, ie if running a 32-bit
                // app from "Program Files", windows will change the path to use "Program Files (x86)"
                // If the app isn't found where it is supposed to be, append the executable name
                // to the appDir path and use that instead
                osModuleArchitecture binaryType;
                bool bGetType = OSWrappers::GetBinaryType(appName.asCharArray(), &binaryType);

                if (!bGetType)
                {
                    gtString filePath;
                    filePath.fromASCIIString(appName.asCharArray());

                    // the app name is not a valid executable or doesn't exist. Build the new appName from the
                    // appDir and appending the executable name
                    osFilePath appFilePath(filePath);
                    gtString filename;
                    appFilePath.getFileNameAndExtension(filename);
                    appName = appDir;
                    appName.append("\\");
                    appName.append(filename.asASCIICharArray());
                }

#endif // _WIN32
                ProcessTracker::Instance()->SetInjectedAppName(appName.asCharArray());
                ProcessTracker::Instance()->SetInjectedAppDir(appDir.asCharArray());
                ProcessTracker::Instance()->SetInjectedAppArgs(appArgs.asCharArray());
            }
            else
            {
                // Not a shortcut - take app name and arguments from the command line
                ProcessTracker::Instance()->SetInjectedAppName(app.at(0).c_str());

                gtASCIIString appArgs;

                for (unsigned int i = 1 ; i < app.size(); i++)
                {
                    appArgs += app.at(i).c_str();
                    appArgs += " ";
                }

                ProcessTracker::Instance()->SetInjectedAppArgs(appArgs.asCharArray());

#ifdef _LINUX
                // if the application contains spaces and has no arguments, the command line may have been
                // enclosed in quotes. In this case, process the command line appropriately.
                std::string& commandLine = app.at(0);

                if (strstr(commandLine.c_str(), " ") != NULL && appArgs.length() == 0)
                {
                    ParseCommandLineString(commandLine);
                }

#endif // _LINUX

                // also take working directory if specified
                if (vm.count("appworkingdir"))
                {
                    string dir = vm["appworkingdir"].as<string>();
                    ProcessTracker::Instance()->SetInjectedAppDir(dir.c_str());
                }
            }
        }

#ifdef _LINUX
        else if (true == useScript)
        {
            if (true == ReadShellScript(scriptFileName))
            {
                // also take working directory if specified
                if (vm.count("appworkingdir"))
                {
                    string dir = vm["appworkingdir"].as<string>();
                    ProcessTracker::Instance()->SetInjectedAppDir(dir.c_str());
                }
            }
        }

#endif
        return 0;
    }
    catch (exception& e)
    {
        // Problem parsing options - report and exit
        cout << "GPUPerfServer: " << e.what() << "\n";
        return -1;
    }
}

#ifdef _WIN32
//--------------------------------------------------------------
/// Given a process ID, return its name as a string.
/// \param processID The process ID.
/// \return The name of the process.
//--------------------------------------------------------------
std::string GetProcessName(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    // Get the process name.
    if (hProcess != NULL)
    {
        HMODULE hMod = 0;
        DWORD cbNeeded = 0;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    // Release the handle to the process.
    CloseHandle(hProcess);

    return szProcessName;
}

//--------------------------------------------------------------
/// Determine if an app is known to be conflicting with GPS.
/// \param appName Name of the app in question.
/// \return False if the app is incompatible.
//--------------------------------------------------------------
bool IsAppConflictive(const std::string& appName)
{
    bool conflictive = false;

    static LPCSTR conflictApps[] =
    {
        { "fraps.exe" },
        { "raptr.exe" },
    };

    const int conflictCount = sizeof(conflictApps) / sizeof(conflictApps[0]);

    for (int i = 0; i < conflictCount; i++)
    {
        if (conflictApps[i] == appName)
        {
            conflictive = true;
            break;
        }
    }

    return conflictive;
}

//--------------------------------------------------------------
/// Detect and report presence of conflicting apps.
//--------------------------------------------------------------
void DetectConflictingProcesses()
{
    // Get the list of process identifiers.
    DWORD aProcesses[1024] = {};
    DWORD cbNeeded = 0;
    DWORD cProcesses = 0;

    unsigned int i = 0;

    std::vector<std::string> conflictives;

    if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        // Calculate how many process identifiers were returned.
        cProcesses = cbNeeded / sizeof(DWORD);

        // Print the name and process identifier for each process.
        for (i = 0; i < cProcesses; i++)
        {
            if (aProcesses[i] != 0)
            {
                std::string name = GetProcessName(aProcesses[i]);

                if (IsAppConflictive(name) == true)
                {
                    conflictives.push_back(name);
                }
            }
        }
    }

    if (conflictives.empty() == false)
    {
        std::stringstream s;

        if (conflictives.size() == 1)
        {
            s << "Please close " << conflictives[0] << " to prevent issues with GPU PerfStudio.";

            LogConsole(logMESSAGE, "Detected conflicting process in the background!\n");
        }
        else
        {
            s << "Please close the following processes to prevent issues with GPU PerfStudio:";

            s << std::endl;
            s << std::endl;

            for (unsigned int j = 0; j < conflictives.size(); j++)
            {
                s << conflictives[j] << std::endl;
            }

            LogConsole(logMESSAGE, "Detected conflicting processes in the background!\n");
        }

        MessageBoxWarning(s.str().c_str());
    }
}
#endif

//--------------------------------------------------------------
///  main
/// \param argc number of args
/// \param argv array of args
//--------------------------------------------------------------
#ifdef _WIN32
    int _tmain(int argc, TARGV* argv[])
#else
    int main(int argc, TARGV* argv[])
#endif
{
    char globalMutexName[PS_MAX_PATH];
    bool remoteLaunch = false;
    sprintf_s(globalMutexName, PS_MAX_PATH, "Graphics_server_global_mutex");
    ProgramInstance SingleInstance(globalMutexName);

    if (SingleInstance.IsProgramAlreadyRunning())
    {
        LogConsole(logMESSAGE, "%s\n", AppNameAndVersion);
        LogConsole(logWARNING, "%s is already running - please close the existing version before creating a new instance\n", PERFSTUDIOSERVER_APP_NAME);
#ifdef _LINUX
        LogConsole(logWARNING, "If no other %s processes are running, try deleting the shared memory files using the rmshm.sh script\n", PERFSTUDIOSERVER_APP_NAME);
#endif // _LINUX
        PrintWait(3000);
        return -1;
    }

#ifdef _LINUX
    DeleteAllSharedMemories();
#endif

    int optionPort = atoi(PS_DEFAULT_PORT);

    GetModuleDirectory(s_LibPath);    // command line options may be relative to this location

    // parse the command line and abort if there are any problems
    int commandLineError = ParseCommandLine(argc, argv, &optionPort, &remoteLaunch);

    if (commandLineError != 0)
    {
        return commandLineError;
    }

    // Initialize LogFile - this will delete the previous logfile
    LogFileInitialize();

    LogConsole(logMESSAGE, "%s\n", AppNameAndVersion);

    LogHeader();

#if defined (_LINUX) || defined (GDT_INTERNAL)
    int serverFlags = 0;
#endif

#ifdef _DEBUG
    LogConsole(logMESSAGE, "This is a debug build\n");
#endif
#if defined GDT_INTERNAL
    LogConsole(logMESSAGE, "This is an internal build\n");
    serverFlags |= FLAG_BUILD_INTERNAL;
#endif

#ifdef _LINUX
    // Find out if this server is 32 or 64 bit.
    osModuleArchitecture binaryType;

    bool bGetType = OSWrappers::GetBinaryType(argv[0], &binaryType);

    if (bGetType)
    {
        if (binaryType == OS_I386_ARCHITECTURE)
        {
            LogConsole(logMESSAGE, "This is a 32-bit server\n");
            serverFlags |= FLAG_BUILD_32BIT;
        }
        else if (binaryType == OS_X86_64_ARCHITECTURE)
        {
            LogConsole(logMESSAGE, "This is a 64-bit server\n");
        }
    }

    // save the build flags to shared memory so that the plugin knows the bitness of the
    // webserver running and whether it is an internal build or not
    SG_SET_INT(BuildFlags, serverFlags);
#endif

    if (!SG_GET_BOOL(OptionNoLogfile))
    {
        LogConsole(logMESSAGE, "Using Logfile: %s\n", SG_GET_PATH(LogfilePath));
    }

#ifdef _WIN32

    if (SG_GET_BOOL(OptionDllReplacement) == true)
    {
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        sprintf_s(s_GPSPluginsDir, PS_MAX_PATH, "%sx64\\", GPS_PLUGIN_DIR);
#else
        sprintf_s(s_GPSPluginsDir, PS_MAX_PATH, "%sx86\\", GPS_PLUGIN_DIR);
#endif
    }
    else
    {
        sprintf_s(s_GPSPluginsDir, PS_MAX_PATH, "%s", GPS_PLUGIN_DIR);
    }

#endif // WIN32

    OSDependentGlobalsInitialization();

    //  register a callback when the user closes the console window so that we can clean up the DX reg keys
#if defined (_WIN32)
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCloseHandler, TRUE);
#else
    SetConsoleCtrlHandler((__sighandler_t)ConsoleCloseHandler);
    SetSignalHandler((__sighandler_t)SigHupHandler, SIGHUP);
    SetSignalHandler((__sighandler_t)SigTermHandler, SIGTERM);
#endif

    if (CollectWrapperInfo() == false)
    {
        LogConsole(logERROR, "Collecting wrapper information failed\n");
#ifdef _WIN32
        RestoreRegistrySettings();
#endif
        smClose("ActivePlugins");
        return -1;
    }

    // This check will prevent us from doing remote launching
    // Does the server have an app to launch?
    if ((ProcessTracker::Instance()->GetInjectedAppName().empty() == true) && (false == remoteLaunch))
    {
        LogConsole(logMESSAGE, "You have started the server without an application!\n");
#ifdef _LINUX
        LogConsole(logMESSAGE, "Please restart by passing your application name as a command line argument to the GPU PerfStudio server.\n");
        LogConsole(logMESSAGE, "Start your application from where it is normally started from so that it can access any resources it needs.\n");
        LogConsole(logMESSAGE, "See the README file or the help file provided with the client for more details and examples.\n");
#else
        LogConsole(logMESSAGE, "Please restart by dragging and dropping your application onto the server.\n");
        LogConsole(logMESSAGE, "In some cases shortcuts to your application and/or the Graphics server can cause this error.\n");
        LogConsole(logMESSAGE, "Try again by dragging the actual application exe onto GPUPerfServer.exe\n");
#endif // _LINUX
        smClose("ActivePlugins");
#ifdef _WIN32
        PrintWait(10000);
        RestoreRegistrySettings();
#endif
        return -1;
    }

    LogConsole(logMESSAGE, "Starting web server on port %d\n", optionPort);
    NetSocket* server_socket = WebServerSetup(optionPort);

    if (server_socket == NULL)
    {
        LogConsole(logERROR, "Exiting, cannot initialize web server\n");
        smClose("ActivePlugins");
        PrintWait(3000);
#ifdef _WIN32
        RestoreRegistrySettings();
#endif
        return -1;
    }

    if (false == g_shutdownEvent.Create("GPS_SHUTDOWN_SERVER"))
    {
        Log(logERROR, "Failed to create Shutdown Event. Server cannot continue safely.\n");
        smClose("ActivePlugins");
        WebServerCleanup(server_socket);
        server_socket = NULL;
#ifdef _WIN32
        RestoreRegistrySettings();
#endif
        return -1;
    }

    // Create a semaphore to indicate when the webserver thread has finished
    NamedSemaphore clientDoneSemaphore;
    clientDoneSemaphore.Create(CLIENT_THREAD_SEMAPHORE);

    osThread* clientThread = ForkAndWaitForClientRequests(server_socket);

#ifdef _WIN32

    if (SG_GET_BOOL(OptionAppInitDll) == true)
    {
        EnableAppInit();
    }

#endif

    // Does the server have an app to launch?
    if ((ProcessTracker::Instance()->GetInjectedAppName().empty() == true) && (false == remoteLaunch))
    {
        LogConsole(logMESSAGE, "Please connect with the client to specify which app to run\n");
    }
    else
    {
        // use all of the allowed wrappers
        g_allowedWrappersMap = s_wrapperMap;

        if (false == remoteLaunch)
        {
            if (ProcessTracker::Instance()->WritePluginsToSharedMemoryAndLaunchApp() == false)
            {
                // allow for thread initialization to complete. Some threads are initializing shared
                // memory and this needs to be completed before they can be shut down. If a thread
                // holds a valid lock when its process is terminated, bad things will happen when
                // that shared memory needs to be released.
                g_shutdownEvent.Signal();
            }
        }
        else
        {
            if (ProcessTracker::Instance()->CreateSharedMemory() == false)
            {
                g_shutdownEvent.Signal();
            }
            else
            {
                LogConsole(logMESSAGE, "Please run your application with LD_PRELOAD specifying the GLServer plugin\n");
            }
        }
    }

#ifdef _WIN32
#ifndef CODEXL_GRAPHICS
    // CodeXL client is handling conflicting processes
    DetectConflictingProcesses();
#endif
#endif

    // wait for the webserver thread to finish. The webserver thread will place the semaphore
    // in the signaled state when it's done.
    clientDoneSemaphore.Wait();

    clientDoneSemaphore.Close();

    delete clientThread;

    WebServerCleanup(server_socket);

    // close all shared memories
    CloseAllSharedMemories();

    g_shutdownEvent.Close();
#ifdef _WIN32
    RestoreRegistrySettings();
#endif
    LogFooter();

#ifdef _LINUX
    DeleteAllSharedMemories();
#endif

    return 0;
}

