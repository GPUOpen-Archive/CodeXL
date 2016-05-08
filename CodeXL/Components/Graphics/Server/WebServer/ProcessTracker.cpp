//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ProcessTracker.cpp
/// \brief File contains a class that tracks which processes were injected into
///        and which plugins are in each process.
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <unordered_map>
#include <Interceptor.h>

#include "../Common/SharedGlobal.h"
#include "../Common/parser.h"
#include "../Common/Logger.h"
#include "../Common/OSWrappers.h"
#include "../Common/HTTPRequest.h"
#include "../Common/ICommunication.h"
#include "../Common/CommandTimingManager.h"
#include "../Common/misc.h"
#ifdef _WIN32
    #include "../Common/Windows/DllReplacement.h"
    #include "../MicroDLL/MicroDLLName.h"
#endif

#include "Inject.h"
#include "OSDependent.h"
#include "ProcessTracker.h"
#include "RequestsInFlightDatabase.h"

#ifdef _WIN32
    #include <cstdio>
    #include <windows.h>
    #include <tlhelp32.h>
    #include "Registry.h"
#endif

///  Used to protect access to maps from multiple threads.
static mutex s_mutex;

/// Control the inclusion of the Vulkan code.
#define ENABLE_VULKAN 1

//--------------------------------------------------------------
/// Generates XML that describes the injected processes and which
/// plugins were injected.
/// \return string containing the XML
//--------------------------------------------------------------
gtASCIIString ProcessTracker::GetProcessesXML()
{
    std::unordered_map< DWORD, gtASCIIString > procXMLMap;

    this->UpdateListOfInjectedProcesses();

    ProcessInfoList injectedProcesses = this->GetListOfInjectedProcesses();

    WrapperMap wrappers = GetWrapperMap();

    // the strPlatform is named this way to match options in the client.
#ifdef X64
    gtASCIIString strPlatform = "Win64";
#else
    gtASCIIString strPlatform = "Win32";
#endif

#ifndef CODEXL_GRAPHICS

    if (injectedProcesses.empty() == true)
    {
        LogConsole(logERROR, "There are no processes running which have been injected with the GPU PerfStudio server plugin\n");
        LogConsole(logERROR, "Please ensure that the server has the same bitness as the target application\n");
        LogConsole(logERROR, "For example, use the 64-bit GPU PerfStudio server with a 64-bit application\n");
    }

#endif

    for (ProcessInfoList::iterator procIter = injectedProcesses.begin();
         procIter != injectedProcesses.end();
         ++procIter)
    {

        DWORD pid = procIter->th32ProcessID;

        // only add the process info if it wasn't already found.
        if (procXMLMap.find(pid) == procXMLMap.end())
        {
            // insert new process and the process info
            gtASCIIString tmpString = XML("Name", XMLEscape(procIter->szExeFile).asCharArray());
            tmpString += XML("PID", pid);
            tmpString += XML("Path", XMLEscape(procIter->szPath).asCharArray());
            tmpString += XML("Platform",  strPlatform.asCharArray());
            procXMLMap[ pid ] = tmpString;

            // if this process is the one that was launched, then we know what the args and working directory were.
            // we could probably get the actual args and working dir from MicroDLL if it is a different app though, which
            // would be the case if this app uses a launcher app.
            if (m_injectedAppName.compare(procIter->szPath) == 0)
            {
                tmpString = XML("Args", XMLEscape(m_injectedAppArgs.c_str()).asCharArray());
                tmpString += XML("WDir", XMLEscape(m_injectedAppDir.c_str()).asCharArray());
                procXMLMap[ pid ] += tmpString.asCharArray();
            }
        }

        // add an API node for each of the wrappers that are injected into the app
        for (WrapperMap::const_iterator wrapperIter = wrappers.begin(); wrapperIter != wrappers.end(); ++wrapperIter)
        {
            // List of plugin extensions to check for. On Windows, test for 32 and 64 bit plugins.
            // On linux, just check for the plugin corresponding to the server bitness
            static const char* pluginExtensions[] =
            {
#ifdef WIN32
                GDT_DEBUG_SUFFIX GDT_BUILD_SUFFIX "." DLL_EXTENSION,
                "-x64" GDT_DEBUG_SUFFIX GDT_BUILD_SUFFIX "." DLL_EXTENSION
#else
                GDT_PROJECT_SUFFIX "." DLL_EXTENSION
#endif
            };

            int numPlugins = sizeof(pluginExtensions) / sizeof(pluginExtensions[0]);

            for (int loop = 0; loop < numPlugins; loop++)
            {
                // check to see if this wrapper is in the application
                gtASCIIString strPluginName = wrapperIter->second.strPluginName;

                if (SG_GET_BOOL(OptionDllReplacement) == false)
                {
                    strPluginName += pluginExtensions[loop];
                }

                if (IsLibraryLoadedInProcess(pid, strPluginName.asCharArray(), NULL))
                {
                    bool attached = false;

                    if (g_activeWrappersMap.find(FormatText("%lu/%s", pid, wrapperIter->first.c_str()).asCharArray()) != g_activeWrappersMap.end())
                    {
                        // the pid/plugin string was listed in the active wrappers map, so the plugin must be active.
                        attached = true;
                    }

                    procXMLMap[pid] += XMLAttrib("API", FormatText("attached='%s'", attached ? "TRUE" : "FALSE").asCharArray(), wrapperIter->second.strPluginShortDesc.asCharArray());
                }
            }
        }
    }

    // concatenate the process XML and additional info
    gtASCIIString xml;

    for (std::unordered_map< DWORD, gtASCIIString >::iterator iterP = procXMLMap.begin();
         iterP != procXMLMap.end();
         iterP++)
    {
        xml += XML("Process", (iterP->second).asCharArray());
    }

    gtASCIIString out = XMLHeader();
    out += XML("ProcessList", xml.asCharArray());

    return out;
}

#ifdef CODEXL_GRAPHICS
#ifdef USE_GRAPHICS_SERVER_STATUS_RETURN_CODES
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sends the correct form of the server status value back to XML, HTML, TEXT, and PNG client requests.
/// \param serverState The server status value to send.
/// \param pRequestHeader The request header.
/// \param client_socket The socket used for the request
//////////////////////////////////////////////////////////////////////////////////////////////////////////
static void HandleServerStatusResponse(GRAPHICS_SERVER_STATE serverState, HTTPRequestHeader* pRequestHeader, NetSocket* client_socket)
{
    char* strResquest = pRequestHeader->GetUrl();

    gtASCIIString requestString(strResquest);

    gtASCIIString xmlSubString(".xml");

    if (requestString.find(xmlSubString, 0) != -1)
    {
        SendServerStatusMessageAsXML(serverState, client_socket, requestString);
        return;
    }

    gtASCIIString htmlSubString(".html");

    if (requestString.find(htmlSubString, 0) != -1)
    {
        SendServerStatusMessageAsHTML(serverState, client_socket);
        return;
    }

    gtASCIIString pngSubString(".png");

    if (requestString.find(pngSubString, 0) != -1)
    {
        SendServerStatusMessageAsIMG(serverState, client_socket);
        return;
    }

    gtASCIIString ddsSubString(".dds");

    if (requestString.find(ddsSubString, 0) != -1)
    {
        SendServerStatusMessageAsIMG(serverState, client_socket);
        return;
    }

    gtASCIIString txtSubString(".txt");

    if (requestString.find(txtSubString, 0) != -1)
    {
        SendServerStatusMessageAsHTML(serverState, client_socket);
        return;
    }

    // Assume all other commands (without an extension) are text commands
    gtASCIIString noSubString(".");

    if (requestString.find(noSubString, 0) == -1)
    {
        SendServerStatusMessageAsHTML(serverState, client_socket);
        return;
    }

    return;
}
#endif
#endif

/// Record the clear state of the registry
static bool registryCleared = false;

bool ProcessTracker::HandleRequest(HTTPRequestHeader* pRequestHeader,
                                   CommunicationID requestID,
                                   NetSocket* pClientSocket,
                                   bool renderLoopStalled)
{
    char* ptr = pRequestHeader->GetUrl();
    char* sCmd = &ptr[1];

#ifdef CODEXL_GRAPHICS
#ifdef USE_GRAPHICS_SERVER_STATUS_RETURN_CODES

    // Handle process not running condition
    if (pRequestHeader->CheckProcessStillRunning() == false)
    {
        Log(logMESSAGE, "Rejecting the command above due to process no longer running: %s\n", pRequestHeader->GetUrl());
        // Need to return the correct error data for process not running
        HandleServerStatusResponse(GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING, pRequestHeader, pClientSocket);
        // This request never gets passed to the graphics server.
        return true;
    }

    // Check if renering has stalled.
    if (renderLoopStalled == true)
    {
        Log(logMESSAGE, "Rejecting the command above due to render stall: %s\n", pRequestHeader->GetUrl());
        // Need to return the correct error data for process not running
        HandleServerStatusResponse(GRAPHICS_SERVER_STATE_STALLED, pRequestHeader, pClientSocket);
        // This request never gets passed to the graphics server.
        return true;
    }

#else
    UNREFERENCED_PARAMETER(renderLoopStalled);
#endif
#else
    PS_UNREFERENCED_PARAMETER(renderLoopStalled);
#endif


#ifdef _WIN32

    // if using AppInit_Dll, clear the registry as soon as a process.xml request is sent.
    // TODO: move this to where a connection has definately been made
    if (SG_GET_BOOL(OptionAppInitDll) == true)
    {
        if (registryCleared == false)
        {
            RestoreAppInit();
            registryCleared = true;
        }
    }

#endif

    if (IsToken(&sCmd, "inject?"))
    {
        DoInjectCommand(requestID, &sCmd, pClientSocket);
    }
    else
    {
        // do commands that depend on PID

        for (WrapperMap::iterator wrapperIter = g_activeWrappersMap.begin();
             wrapperIter != g_activeWrappersMap.end();
             ++wrapperIter)
        {
            // parse out the process ID
            unsigned long pid = 0;
            sscanf_s(wrapperIter->first.c_str(), "%lu/", &pid);

            // make PID string for easier parsing of the command
            gtASCIIString strPID;
            strPID.appendFormattedString("%lu", pid);

            gtASCIIString strPidSlashPlugin = wrapperIter->first.c_str();

            // the key in the activeWrappersMap is formatted as pid/plugin (ie: "435/DX11")
            // and this conveniently matches the format of the commands coming into the server, so parse for matching commands
            gtASCIIString tmpString = strPidSlashPlugin;
            tmpString += "/";

            if (IsToken(&sCmd, tmpString.asCharArray()))
            {
                // we know this command is targetting the current plugin
                pRequestHeader->SetUrl(sCmd);

                // pass the request to the plugin
#ifdef _WIN32
                const char* memoryName = strPidSlashPlugin.asCharArray();
#else
                // the '/' character can't be used as a filename in Linux, so just use the plugin name as the shared memory name
                // (ignore the process ID)
                char memoryName[PS_MAX_PATH];
                char pluginShortDesc[ PS_MAX_PATH ];
                sscanf_s(strPidSlashPlugin.asCharArray(), "%lu/%s", &pid, pluginShortDesc, sizeof(pluginShortDesc));
                sprintf_s(memoryName, PS_MAX_PATH, "%lu %s", pid, pluginShortDesc);
#endif

                if (PassRequestToPlugin(memoryName, pRequestHeader, pid, pClientSocket))
                {
                    return true;
                }
                else
                {
                    Log(logERROR, "Request '%s' is not targeted to an active plugin.\n", GetRequestText(requestID));
                    SendHTMLResponse(requestID, "<html>Error: Targeted plugin is not currently active.</html>", pClientSocket);
                }
            }
            else
            {
                tmpString = strPID;
                tmpString += "/Kill";

                if (IsToken(&sCmd, tmpString.asCharArray()))
                {
                    if (KillProcess(pid))
                    {
                        SendTextResponse(requestID, "Ok", pClientSocket);

                        // if the app was launched from the command line or drag and drop
                        // then killing the app should cause the server to shutdown also
                        if (g_bAppSpecifiedAtCmdLine)
                        {
                            g_shutdownEvent.Signal();
                            CloseStreamThread();
                            CloseStreamSockets();
                            SendTextResponse(requestID, "OK", pClientSocket);
                            return true;
                        }
                    }
                    else
                    {
                        Log(logERROR, "Failed to Kill the process\n");
                        SendTextResponse(requestID, "Error: Failed to Kill the process.", pClientSocket);
                    }
                }
            }
        }
    }

    return false;
}

void ProcessTracker::UpdateListOfInjectedProcesses()
{
    this->m_injectedProcessList.clear();

    ProcessInfoList localProcessInfoArray;
#ifdef _WIN32

    // If using DLL replacement, get any process running that has a replaced
    // dll loaded into it
    // just D3D12.dll & OpenGL32.dll at the moment (D3D11 / GLES can be added later if needed)
    if (SG_GET_BOOL(OptionDllReplacement) == true)
    {
        get_process_list("D3D12.dll", localProcessInfoArray);
        get_process_list("OpenGL32.dll", localProcessInfoArray);
    }
    else
    {
        get_process_list(MICRODLLNAME GDT_DEBUG_SUFFIX GDT_BUILD_SUFFIX ".dll", localProcessInfoArray);
        get_process_list(MICRODLLNAME "-x64" GDT_DEBUG_SUFFIX GDT_BUILD_SUFFIX ".dll", localProcessInfoArray);
    }

#else
    get_process_list("GLServer" GDT_PROJECT_SUFFIX ".so", localProcessInfoArray);
    get_process_list("GLESServer" GDT_PROJECT_SUFFIX ".so", localProcessInfoArray);
#endif

    for (ProcessInfoList::const_iterator iter = localProcessInfoArray.begin();
         iter != localProcessInfoArray.end();
         ++iter)
    {

        // find if we've already listed this process (because it has more than 1 plugin attached)
        bool bAlreadyHaveProcess = false;

        for (ProcessInfoList::iterator aProc = this->m_injectedProcessList.begin();
             aProc != this->m_injectedProcessList.end();
             ++aProc)
        {
            if (aProc->th32ProcessID == iter->th32ProcessID)
            {
                bAlreadyHaveProcess = true;
            }
        }

        // only add new processes
        if (bAlreadyHaveProcess == false)
        {
            this->m_injectedProcessList.push_back(*iter);
        }
    }
}

/// Gets the list of processes which have microDLL injected
/// \return a list of processes which have microDLL inside
ProcessInfoList ProcessTracker::GetListOfInjectedProcesses()
{
    return m_injectedProcessList;
}

//--------------------------------------------------------------
/// Create the shared memory needed to communicate between the
/// web server and the plugin
/// \return true if successful, false if error
//--------------------------------------------------------------
bool ProcessTracker::CreateSharedMemory()
{
#ifdef _WIN32

    if (smExists("GPS_TO_MDLL"))
    {
        if (smOpen("GPS_TO_MDLL"))
        {
            if (smLockGet("GPS_TO_MDLL"))
            {
                //the shared memory already exists, so we need to read everything from it, to make sure it's empty
                // closing and recreated the shared memory does not work because the MicroDLL has it open
                // so that it can inject into any spawned processes (via CreateProcess). If MicroDLL doesn't keep
                // a handle to the shared memory, it could be closed by everyone, get destroyed and then it wouldn't
                // be able to follow CreateProcess.
                char tmp[ PS_MAX_PATH ];

                while (smGet("GPS_TO_MDLL", NULL, 0) != 0)
                {
                    // the smGet call will clear the shared memory
                    smGet("GPS_TO_MDLL", tmp, PS_MAX_PATH);
                }

                smUnlockGet("GPS_TO_MDLL");
            }
        }
    }

    // now write the plugin info into the shared memory
    // Put wrapper info into a shared memory for micro DLL
    // make the shared memory big enough for all the wrappers that are known about
    // even though we're only going to write in the wrappers that are allowed
    if (smCreate("GPS_TO_MDLL", (unsigned long)GetWrapperMap().size() * 3, PS_MAX_PATH))
    {
        // lock the buffer if it has enough space for 3 strings for each of the allowed wrappers
        unsigned long ulNumBuffers = (unsigned long) g_allowedWrappersMap.size() * 3;

        if (smLockPut("GPS_TO_MDLL", ulNumBuffers * PS_MAX_PATH, ulNumBuffers))
        {
            char strPath[PS_MAX_PATH];
            char strName[PS_MAX_PATH];
            char strDlls[PS_MAX_PATH];

            for (WrapperMap::const_iterator iter = g_allowedWrappersMap.begin();
                 iter != g_allowedWrappersMap.end();
                 ++iter)
            {
                strcpy_s(strPath, PS_MAX_PATH, iter->second.strPluginPath.asCharArray());
                strcpy_s(strName, PS_MAX_PATH, iter->second.strPluginName.asCharArray());
                strcpy_s(strDlls, PS_MAX_PATH, iter->second.strWrappedDll.asCharArray());

                if (smPut("GPS_TO_MDLL", (void*) strPath, PS_MAX_PATH) == false ||
                    smPut("GPS_TO_MDLL", (void*) strName, PS_MAX_PATH) == false ||
                    smPut("GPS_TO_MDLL", (void*) strDlls, PS_MAX_PATH) == false)
                {
                    Log(logERROR, "Couldn't put wrapper info into shared memory.\n");
                    smUnlockPut("GPS_TO_MDLL");
                    return false;
                }
            }

            smUnlockPut("GPS_TO_MDLL");
        }
        else
        {
            Log(logERROR, "There is not enough space in the shared memory for the desired content.\n");
            return false;
        }
    }
    else
    {
        Log(logERROR, "Couldn't create shared memory to pass wrapper registration\n");
        return false;
    }

#else

    for (WrapperMap::const_iterator iter = g_allowedWrappersMap.begin();
         iter != g_allowedWrappersMap.end();
         ++iter)
    {
        // On Linux, Create the shared memory from the PerfStudio server so that
        // it is cleaned up properly.
        if (smCreate(iter->second.strPluginShortDesc.asCharArray(), 100, sizeof(HTTPRequestHeader)) == false)
        {
            Log(logERROR, "Couldn't create shared memory for plugin.\n");
            return false;
        }
    }

#endif // def _WIN32
    return true;
}

//--------------------------------------------------------------
/// Writes information about the allowed plugins into shared memory
/// and launches the application specified in s_strInjectedAppName
/// \return true if the app is successfully launched; false otherwise
//--------------------------------------------------------------
bool ProcessTracker::WritePluginsToSharedMemoryAndLaunchApp()
{
    bool bGetType = false;

    if (OSDependentModulesInitialization() == false)
    {
        Log(logERROR, "Failed to initialize for plugins\n");
        return false;
    }

    osModuleArchitecture binaryType;

    bGetType = OSWrappers::GetBinaryType(m_injectedAppName.c_str(), &binaryType);

#ifdef _WIN32

    if (!bGetType)
    {
        // try without quotes, should be W8x preference.  This will update m_injectedAppName for later use also.
        m_injectedAppName.erase(0, 1);
        m_injectedAppName.resize(m_injectedAppName.size() - 1);

        bGetType = OSWrappers::GetBinaryType(m_injectedAppName.c_str(), &binaryType);
    }

#endif // def _WIN32

    if (!bGetType)
    {
        // file is not executable
        LogConsole(logERROR, "%s is not an executable file\n", m_injectedAppName.c_str());
        return false;
    }

    Log(logMESSAGE, "WritePluginsToSharedMemoryAndLaunchApp()\n");

    if (false == CreateSharedMemory())
    {
        return false;
    }

    // now actually start the application
    PROCESS_INFORMATION pi = LaunchAppInNewProcess(m_injectedAppName.c_str(), m_injectedAppDir.c_str(), m_injectedAppArgs.c_str(), binaryType);

    if (pi.dwProcessId == 0)
    {
        LogConsole(logWARNING, "Failed to start application: %s %s\n\n", m_injectedAppName.c_str(), m_injectedAppArgs.c_str());
        return false;
    }

    Log(logMESSAGE, "About to resume application thread\n");
#ifdef _WIN32

    if (osResumeSuspendedProcess(0, 0, pi.hThread, false) == false)
    {
        osSystemErrorCode systemLastError = osGetLastSystemError();

        gtString systemErrorString;
        osGetLastSystemErrorAsString(systemErrorString);

        Log(logERROR, "Resuming thread failed; Error %d: %s\n", systemLastError, systemErrorString.asASCIICharArray());
    }

#endif

    m_injectedAppID = pi.dwProcessId;

    return true;
}

//--------------------------------------------------------------
//  LaunchAppInNewProcess
//--------------------------------------------------------------
PROCESS_INFORMATION ProcessTracker::LaunchAppInNewProcess(gtASCIIString strApp, gtASCIIString strDir, gtASCIIString strArgs, osModuleArchitecture binaryType)
{
    LogConsole(logMESSAGE, "About to launch: %s\n", strApp.asCharArray());
    LogConsole(logMESSAGE, "Params: %s\n", strArgs.asCharArray());
    LogConsole(logMESSAGE, "Working Directory: %s\n", strDir.asCharArray());

    // Get app directory and make it default
    if (strDir.isEmpty())
    {
        size_t pos = strApp.find_last_of("\\");

        if (pos != std::string::npos)
        {
            strDir = strApp.substr(0, (int)pos);

            if (strApp[0] == '\"')
            {
                strDir += "\"";
            }
        }
    }

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

#ifdef _WIN32
    DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

    SetLastError(0);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
#endif

    // Cmd line has to include the exe name since many apps expect the executable name to be in argv[0]!
    // Note argv[0] on the command line needs to be surrounded with quotes if it contains spaces.
    // The arguments in strArgs have already been "quoted" as they are parsed.

    gtASCIIString strCmdLine = AddQuotesIfStringHasSpaces(strApp.asCharArray());
    strCmdLine += " ";
    strCmdLine += strArgs;

    LogConsole(logMESSAGE, "strApp: %s\n", strApp.asCharArray());
    LogConsole(logMESSAGE, "strCmdLine: %s\n", strCmdLine.asCharArray());

    // Attempt to initialize the environment that the new process will run in. The child process should inherit "this" environment.
    if (!PrelaunchEnvironmentInitialization())
    {
        // Log a warning if this failed- initializing the environment for the new process can fail if Mantle support isn't installed.
        // In these cases, if the user is attempting to debug a Mantle application, they will have bigger problems to deal with.
        // In cases where a DX/GL app is being debugged, this warning can be ignored without any side effects.
        Log(logWARNING, "Environment initialization failed. If using DX/GL, it is safe to ignore this warning.\n");
    }

    BOOL succeeded = FALSE;

#ifdef _WIN32

    char microDLLPath[PS_MAX_PATH];
    const char* strServerPath;
    strServerPath = SG_GET_PATH(ServerPath);

    if (SG_GET_BOOL(OptionDllReplacement) == true)
    {
        DllReplacement::SetDllDirectory(binaryType == OS_X86_64_ARCHITECTURE);
    }

    // if using manual dll replacement or the AppInit_DLLs registry setting, don't use any kind of dll injection
    if (SG_GET_BOOL(OptionManualDllReplacement) == true || SG_GET_BOOL(OptionAppInitDll))
    {
        succeeded = CreateProcess(strApp.asCharArray(), (LPSTR)strCmdLine.asCharArray(), NULL, NULL, TRUE, dwFlags, NULL, strDir.asCharArray(), &si, &pi);
    }
    else
    {
#ifdef X64

        // can only launch 64 bit applications
        if (binaryType != OS_X86_64_ARCHITECTURE)
        {
            sprintf_s(microDLLPath, PS_MAX_PATH, "%s" MICRODLLNAME "%s%s.dll", SG_GET_PATH(ServerPath), GDT_DEBUG_SUFFIX, GDT_BUILD_SUFFIX);
            succeeded = AMDT::CreateProcessAndInjectDll(strApp.asCharArray(), (LPSTR)strCmdLine.asCharArray(),
                                                        NULL, NULL, TRUE, dwFlags, NULL,
                                                        strDir.asCharArray(),
                                                        &si, &pi,
                                                        microDLLPath);
        }

#else

        if (binaryType != OS_I386_ARCHITECTURE)
        {
            sprintf_s(microDLLPath, PS_MAX_PATH, "%s" MICRODLLNAME "-x64%s%s.dll", SG_GET_PATH(ServerPath), GDT_DEBUG_SUFFIX, GDT_BUILD_SUFFIX);
            succeeded = AMDT::CreateProcessAndInjectDll(strApp.asCharArray(), (LPSTR)strCmdLine.asCharArray(),
                                                        NULL, NULL, TRUE, dwFlags, NULL,
                                                        strDir.asCharArray(),
                                                        &si, &pi,
                                                        microDLLPath);
        }

#endif // X64
        else
        {
            succeeded = AMDT::CreateProcessAndInjectDll(strApp.asCharArray(), (LPSTR)strCmdLine.asCharArray(),
                                                        NULL, NULL, TRUE, dwFlags, NULL,
                                                        strDir.asCharArray(),
                                                        &si, &pi,
                                                        SG_GET_PATH(MicroDLLPath));
        }
    }

#else

    // Create the app process
    succeeded = CreateProcess(strApp.asCharArray(), strCmdLine.asCharArray(), strDir.asCharArray(), &pi);
#endif // _WIN32

    if (!succeeded)
    {
        osSystemErrorCode systemLastError = osGetLastSystemError();

        gtString systemErrorString;
        osGetLastSystemErrorAsString(systemErrorString);

        Log(logERROR, "CreateProcessAndInjectDll failed; Error %d: %s\n", systemLastError, systemErrorString.asASCIICharArray());
        pi.dwProcessId = 0;
    }

#ifdef _WIN32
    else
    {
        // Check to see if the Steam.exe has been hooked and if so, set the value in shared memory
        // If Steam.exe was used to launch the target application, then the checks for cmd.exe and fcx.exe
        // need to be ignored.
        if (strApp.length() > 0)
        {
            if (strstr(strApp.toLowerCase().asCharArray(), "steam.exe") != NULL)
            {
                SG_SET_BOOL(SteamInjected, true);
            }

            ShowLauncherReminder(strApp.toLowerCase().asCharArray());
        }
        else
        {
            if (strstr(strCmdLine.toLowerCase().asCharArray(), "steam.exe") != NULL)
            {
                SG_SET_BOOL(SteamInjected, true);
            }

            ShowLauncherReminder(strCmdLine.toLowerCase().asCharArray());
        }
    }

#endif // _WIN32

    return pi;
}

//--------------------------------------------------------------
/// Injects the application specified by the Inject command and
/// sends the results of the injection to the requester
/// \param requestID the ID of the incoming request
/// \param sCmd pointer to the string containing the request
/// \param pClientSocket the socket used to send the response
//--------------------------------------------------------------
void ProcessTracker::DoInjectCommand(CommunicationID requestID, char** sCmd, NetSocket* pClientSocket)
{
    // clear the previous data since it is about to get reset
    g_allowedWrappersMap.clear();
    m_injectedAppName = "";
    m_injectedAppArgs = "";

    // parse the command
    gtASCIIString strCmd(*sCmd);
    std::list< gtASCIIString > paramList;
    strCmd.Split("&", true, paramList);

    for (std::list< gtASCIIString >::const_iterator iter = paramList.begin();
         iter != paramList.end();
         ++iter)
    {
        char* pParam = (char*) iter->asCharArray();
        char** ppParam = &pParam;

        if (IsToken(ppParam, "plugins="))
        {
            // plugins= is followed by a list of comma separated short descriptions of wrappers
            gtASCIIString strPlugins(*ppParam);
            std::list< gtASCIIString > pluginList;
            strPlugins.Split(",", true, pluginList);

            for (std::list< gtASCIIString >::const_iterator iter1 = pluginList.begin();
                 iter1 != pluginList.end();
                 ++iter1)
            {
                if (*iter1 == "")
                {
                    Log(logERROR, "Failed to parse plugin name\n");
                    SendTextResponse(requestID, "Error: Failed to parse plugin name.", pClientSocket);
                    return;
                }

                WrapperMap::iterator iWrapper = GetWrapperMap().find(iter1->asCharArray());

                if (iWrapper == GetWrapperMap().end())
                {
                    Log(logERROR, "Invalid plugin requested, you requested plugin %s\n", (*iter1).asCharArray());
                    SendFormattedTextResponse(requestID, pClientSocket, "Error: Invalid plugin requested, you requested plugin %s\n", (*iter1).asCharArray());
                    return;
                }

                // we have a valid index
                g_allowedWrappersMap.insert(*iWrapper);
            }
        }
        else if (IsToken(ppParam, "app="))
        {
            m_injectedAppName = *ppParam;
        }
        else if (IsToken(ppParam, "args="))
        {
            m_injectedAppArgs = *ppParam;
        }
    }

    // make sure an app name was specified
    if (m_injectedAppName.empty() == true)
    {
        Log(logERROR, "Application name not specified\n");
        SendTextResponse(requestID, "Error: Application name not specified", pClientSocket);
        return;
    }

    // if no wrappers were specified, then use them all
    if (g_allowedWrappersMap.empty() == true)
    {
        g_allowedWrappersMap = GetWrapperMap();
    }

    // fix the path
    gtASCIIString appName = m_injectedAppName.c_str();
    appName.replace("%20", " ");
    appName.replace("%22", "\"");
    appName.replace("%5C", "\\");
    appName.replace("%E2%80%93", "-");
    appName.replace("%26", "&");
    appName.replace("%27", "'");
    appName.replace("%60", "`");
    appName.replace("%E2%80%98", "`");
    m_injectedAppName = appName.asCharArray();

    gtASCIIString appArgs = m_injectedAppArgs.c_str();
    appArgs.replace("%22", "\"");
    appArgs.replace("%20", " ");
    appArgs.replace("%5C", "\\");
    appArgs.replace("%E2%80%93", "-");
    appArgs.replace("%26", "&");
    appArgs.replace("%27", "'");
    appArgs.replace("%60", "`");
    appArgs.replace("%E2%80%98", "`");
    m_injectedAppArgs = appArgs.asCharArray();

    if (WritePluginsToSharedMemoryAndLaunchApp() == true)
    {
        // loop up to 500 times while checking to see if a
        // wrapper name has been written into the shared memory from MicroDLL
        unsigned int uTimeout = 500;

        while (smGet("ActivePlugins", NULL, 0) == 0 && uTimeout > 0)
        {
            uTimeout--;
            osSleep(10);
        }

        if (uTimeout > 0)
        {
            SendHTMLResponse(requestID, "<html>OK</html>", pClientSocket);
        }
        else
        {
            Log(logWARNING, "The server has timed out while trying to receive the current active plugins\n");
            gtASCIIString strErrorResponse;
            strErrorResponse = "<html>The server is taking a while to respond. <p/>Either the application does not use one of the selected APIs or it takes a while to load.<p/><br/>";
            strErrorResponse.appendFormattedString("Please <a href='/page/index.html'>try to reconnect</a> if you expect it to continue or<br/> <a href='/%d/Kill'>click here to kill the process</a></html>", m_injectedAppID);
            SendHTMLResponse(requestID, strErrorResponse.asCharArray(), pClientSocket);
        }
    }
    else
    {
        Log(logERROR, "The plugins could not be written to shared memory or the application could not be launched\n");
        SendTextResponse(requestID, "Error, see log file for more details", pClientSocket);
    }

    return;
}

void ProcessTracker::KillAllProcesses()
{
    for (ProcessInfoList::iterator procInfo = this->m_injectedProcessList.begin();
         procInfo != this->m_injectedProcessList.end();
         ++procInfo)
    {
        KillProcess(procInfo->th32ProcessID);
    }

    // Kill the root process. If a 32bit application is started with the 64bit server it will not be registered in the injectedProcessList
    // because the shared memory comms does not work between a server and app of different bitness.
    // This can leave Steam.exe running after shutdown and will be blocking the port and preventing the server from being relaunched.
    if (m_injectedAppID != 0)
    {
        KillProcess(m_injectedAppID);
    }

    this->m_injectedProcessList.clear();
}

//-----------------------------------------------------------------------------
/// Has the attached app been terminated
/// \return true if it has been terminated, false if it is still running
//-----------------------------------------------------------------------------
bool ProcessTracker::AppTerminated()
{
    long exitCode = 0;
    return osWaitForProcessToTerminate(m_injectedAppID, 10, &exitCode);
}

/// Generates an XML command tree for each of the injected processes.
/// \return the XML command tree
std::string ProcessTracker::GetCommandTree()
{
    std::string strXML;

    for (ProcessInfoList::iterator iter = this->m_injectedProcessList.begin();
         iter != this->m_injectedProcessList.end();
         ++iter)
    {
        gtASCIIString strWrapperXML;

        // we should only include the processes that are active
        bool include = false;

        // check the active wrappers to see if one of them is for this process
        for (WrapperMap::iterator wrapperIter = g_activeWrappersMap.begin();
             wrapperIter != g_activeWrappersMap.end();
             ++wrapperIter)
        {
            // parse out the process ID
            unsigned long pid = 0;
            sscanf_s(wrapperIter->first.c_str(), "%lu/", &pid);

            // if this process ID matches, then we want to include this information in the commandTreeXML
            if (pid == iter->th32ProcessID)
            {
                include = true;
                gtASCIIString tmpString = "id='";
                tmpString += wrapperIter->second.strPluginShortDesc;
                tmpString += "'";
                strWrapperXML += XMLAttrib("Plugin", tmpString.asCharArray());
            }
        }

        // only include if there was an active wrapper was found in this process
        if (include)
        {
            gtASCIIString strAttrib;
            strAttrib.appendFormattedString("id='%lu' name='%s'", iter->th32ProcessID, iter->szExeFile);

            gtASCIIString tmpString = XMLAttrib("Kill_Process", "url='Kill' name='Kill Process'");
            tmpString += XML("PID", iter->th32ProcessID);
            tmpString += strWrapperXML;
            strXML += XMLAttrib("Process", strAttrib.asCharArray(), tmpString.asCharArray()).asCharArray();
        }
    }

    return strXML;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Passes the supplied request header to the specifed plugin
/// \param strDestination the short description of the plugin to pass the request to
/// \param pRequestHeader the request header to pass to the plugin
/// \param pid the process id to pass the request to.
/// \param pClientSocket the socket used to read the request
/// \return true if the request was passed to the plugin; false otherwise
////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessTracker::PassRequestToPlugin(const char* strDestination,
                                         HTTPRequestHeader* pRequestHeader,
                                         unsigned long pid,
                                         NetSocket* pClientSocket)
{
    if (strstr(pRequestHeader->GetHeaderData()->url, STR_STREAM_TOKEN) != NULL)
    {
        Log(logTRACE, "Duplicating Socket for request: %s\n", pRequestHeader->GetHeaderData()->url);

        // this is a streaming request, duplicate the socket
        // to reduce the overhead incurred by the shared memory approach
#if defined (_WIN32)

        if (pClientSocket->DuplicateToPID(pid, &pRequestHeader->GetHeaderData()->ProtoInfo) != 0)
#else
        PS_UNREFERENCED_PARAMETER(pid);

        if (CreateStreamSocket(pRequestHeader, pClientSocket) == false)
#endif
        {
            Log(logERROR, "Failed to duplicate socket for streaming request. Error %lu\n", osGetLastSystemError());
            return false;
        }
    }

    bool bResult = false;

    if (smLockPut(strDestination, sizeof(HTTPRequestHeader), 1))
    {
        // Check to see if the postdata failed to come over in some way.
        if (pRequestHeader->GetPostDataSize() > 0 &&  pRequestHeader->GetPostData() == NULL)
        {
            Log(logERROR, "PostData size is %d, but PostData is NULL\n", pRequestHeader->GetPostDataSize());
            // Force the data size to zero
            pRequestHeader->SetPostDataSize(0);
        }

        // Create a new record of this request
        // We will check to see when it comes back from the server.
        //RequestInFlight* pNewRequest = new RequestInFlight(pRequestHeader, pClientSocket);

        // Add the new record to the DB.
        //RequestsInFlightDatabase::Instance()->Add(pClientSocket, pNewRequest);

        bResult = smPut(strDestination, pRequestHeader->GetHeaderData(), sizeof(HTTPHeaderData));

        // Keep in for debugging
        //StreamLog::Ref() << "1) smPut HTTPRequestHeader\n" ;

        // If there is POST data we must send that too.
        if (pRequestHeader->GetPostDataSize() > 0 && pRequestHeader->GetPostData() != NULL)
        {
            bResult = smPut(strDestination, pRequestHeader->GetPostData(), pRequestHeader->GetPostDataSize());

            // Keep in for debugging
            //StreamLog::Ref() << "2) smPut Shader Code: " << pRequestHeader->pPostData << "\n";
        }

        smUnlockPut(strDestination);
#ifdef DEBUG_COMMS_PERFORMANCE
        CommandTimingManager::Instance()->IncrementServerLoadingCount(1) ;
#endif
    }

#ifdef DEBUG_COMMS_PERFORMANCE
    // Record the time now that we have sent the command over the shared memory.
    CommandTiming* cmdTiming = CommandTimingManager::Instance()->GetTimingFromPendingList(pClientSocket);

    if (cmdTiming != NULL)
    {
        LARGE_INTEGER nPerformanceCount;
        QueryPerformanceCounter(&nPerformanceCount);
        cmdTiming->SetPostSharedMemorySend(nPerformanceCount);
        // Set the current server loading value
        cmdTiming->SetServerLoadingCount(CommandTimingManager::Instance()->GetServerLoadingCount());
    }

#endif

    return bResult;
}

//--------------------------------------------------------------------------
/// Setup Vulkan-specific environment variables.
/// This code needs to be called during tool initialization.
//--------------------------------------------------------------------------
void SetupVulkanEnvVariables()
{
#if ENABLE_VULKAN

    // Windows
#ifdef _WIN32

#ifdef _DEBUG
#ifdef X64
    gtString layerName = L"CXLGraphicsServerVulkan-x64-d";
#else
    gtString layerName = L"CXLGraphicsServerVulkan-d";
#endif
#else
#ifdef X64
    gtString layerName = L"CXLGraphicsServerVulkan-x64";
#else
    gtString layerName = L"CXLGraphicsServerVulkan";
#endif
#endif

    gtASCIIString serverPath;
    GetModuleDirectory(serverPath);

    // Set VK_LAYER_PATH equal to where our layer lives
    osEnvironmentVariable layerPath;
    layerPath._name = L"VK_LAYER_PATH";
    layerPath._value.fromASCIIString(serverPath.asCharArray());
    layerPath._value.append(L"Plugins");

    // Set VK_INSTANCE_LAYERS equal to our layer above
    osEnvironmentVariable instanceLayerName;
    instanceLayerName._name =  L"VK_INSTANCE_LAYERS";
    instanceLayerName._value = layerName;

    // Set VK_DEVICE_LAYERS equal to our layer above
    osEnvironmentVariable deviceLayerName;
    deviceLayerName._name = L"VK_DEVICE_LAYERS";
    deviceLayerName._value = layerName;

    // Setting these environment variables should register our server as a layer
    osSetCurrentProcessEnvVariable(layerPath);
    osSetCurrentProcessEnvVariable(instanceLayerName);
    osSetCurrentProcessEnvVariable(deviceLayerName);

    // Linux
#else

#endif

#endif
}

//--------------------------------------------------------------------------
/// Before launching a new process, this function will initialize the environment for the
/// new process.
/// \return true if the the environment for the new process was initialized fully.
//--------------------------------------------------------------------------
bool ProcessTracker::PrelaunchEnvironmentInitialization()
{
    // Start by assuming that this will work without a problem.
    bool bEnvironmentInitialized = true;

#ifndef _WIN32
    // Get the server path. The GLServer plugin will be in the same place.
    // LD_PRELOAD has to be set up before the app is run so the plugin can be attached
    const char* strServerPath;
    strServerPath = SG_GET_PATH(ServerPath);
    char ext[PS_MAX_PATH];
    sprintf_s(ext, PS_MAX_PATH, "%s.so", GDT_PROJECT_SUFFIX);
    char strLdPreload[PS_MAX_PATH];
    sprintf_s(strLdPreload, PS_MAX_PATH, "%sPlugins/%sServer%s", strServerPath, m_injectedAppPluginName.c_str(), ext);
    //   LogConsole(logMESSAGE, "LD_PRELOAD is >>>%s<<<\n", strLdPreload);

    // Set the LD_PRELOAD environment variable
    setenv("LD_PRELOAD", strLdPreload, 1);
#endif

#if ENABLE_VULKAN
    SetupVulkanEnvVariables();
#endif

    return bEnvironmentInitialized;
}

//-----------------------------------------------------------------------------
/// Set up a thread object to handle the communication between the plugin and
/// the client.
/// \param clientSocket Socket to listen to.
/// \return New worker thread.
//-----------------------------------------------------------------------------
osThread* ForkAndWaitForPluginStream(NetSocket* clientSocket)
{
    class StreamThread : public osThread
    {
    public:
        explicit StreamThread(const gtString& threadName, NetSocket* clientSocket)
            : osThread(threadName)
            , m_clientSocket(clientSocket)
        {
        }

        ~StreamThread()
        {
        }

    protected:
        virtual int entryPoint()
        {
            ProcessTracker::Instance()->WaitForPluginStream(m_clientSocket, m_transferBuffer);
            return 0;
        }

    private:
        NetSocket* m_clientSocket;
        char       m_transferBuffer[COMM_MAX_URL_SIZE];
    };

    // Create a new Thread and set it running
    gtString str;
    str.fromASCIIString("StreamThread");
    osThread* streamThread = new StreamThread(str, clientSocket);
    streamThread->execute();
    return streamThread;
}

//-----------------------------------------------------------------------------
/// Wait for data coming in from the plugin on the streaming socket and forward
/// it to the client.
/// \param clientSocket the socket that the streaming data from the plugin
/// should be sent to
/// \param transferBuffer the buffer where the incoming stream data is written
/// to
//-----------------------------------------------------------------------------
void   ProcessTracker::WaitForPluginStream(NetSocket* clientSocket, char* transferBuffer)
{
    // accept a connection
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(sockaddr_in);

    m_streamSocket = m_serverSocket->Accept((struct sockaddr*) & client_address, &client_address_len);

    // Forward data back to the client from the plugin
    bool done = false;

    do
    {
        gtSize_t readDataSize = 0;

        // read the data from the socket connected to the plugin
        if (m_streamSocket->Receive(transferBuffer, COMM_MAX_URL_SIZE, readDataSize) == false)
        {
            done = true;
        }
        else if (readDataSize != 0)
        {
            // send data back to the client
            if (clientSocket == NULL)
            {
                done = true;
            }
            else if (clientSocket->Send(transferBuffer, (int)readDataSize) == false)
            {
                done = true;
            }
        }
        else
        {
            done = true;
        }
    }
    while (done == false);

    // close the sockets
    CloseStreamSockets();
}

//-----------------------------------------------------------------------------
/// Create a streaming socket to communicate between the plugin and the
/// webserver. This avoids the duplication. A new thread is created each time a
/// streaming request comes in. This is because the client socket changes so
/// this change will need propagating to the thread so it knows where to send
/// the requests back to.
/// The streaming socket uses the next port after the web server port, so if
/// the webserver is using port 8080, the streaming port will be 8081
/// \param pRequestHeader the request header to pass to the plugin
/// \param pClientSocket the socket used for communication
/// \return true is successful, false otherwise
//-----------------------------------------------------------------------------
bool ProcessTracker::CreateStreamSocket(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket)
{
    CloseStreamThread();
    CloseStreamSockets();

    // figure out which port to use. For now, use the port the web server is using + 1
    DWORD port = GetWebServerPort() + 1;
    pRequestHeader->GetHeaderData()->dwPort = port;

    // no other threads will be running now so this should be safe
    m_serverSocket = NetSocket::Create();

    if (m_serverSocket == NULL)
    {
        Log(logERROR, "Error creating streamSocket\n");
        return false;
    }

    // try and bind to the streaming port
    if (m_serverSocket->Bind((u_short)port) == false)
    {
        MessageBoxError(FormatText("Port %u is already in use by another application.", port));
        m_serverSocket->close();
        return false;
    }

    // try to listen next
    if (m_serverSocket->Listen() == false)
    {
        m_serverSocket->close();
        return false;
    }

    m_streamThread = ForkAndWaitForPluginStream(pClientSocket);

    return true;
}

//-----------------------------------------------------------------------------
/// close any sockets used for streaming
//-----------------------------------------------------------------------------
void  ProcessTracker::CloseStreamSockets()
{
    ScopeLock lock(s_mutex);

    if (m_streamSocket != NULL)
    {
        m_streamSocket->close();
        m_streamSocket = NULL;
    }

    if (m_serverSocket != NULL)
    {
        m_serverSocket->close();
        m_serverSocket = NULL;
    }
}

//-----------------------------------------------------------------------------
/// shut down the thread used by streaming sockets
//-----------------------------------------------------------------------------
void  ProcessTracker::CloseStreamThread()
{
    if (m_streamThread)
    {
        m_streamThread->terminate();

        while (m_streamThread->isAlive())
        {
            ;
        }

        delete m_streamThread;
        m_streamThread = NULL;
    }
}

/// Add a socket corresponding to a handle to the map
/// \param handle the handle of interest
/// \param socket the socket corresponding to the handle
void ProcessTracker::AddSocketToMap(CommunicationID handle, NetSocket* socket)
{
    m_handleToSocketMap.insert(std::make_pair(handle, socket));
}

/// Remove a socket from the map
/// \param handle the handle whose socket to remove
void ProcessTracker::RemoveSocketFromMap(CommunicationID handle)
{
    m_handleToSocketMap.erase(handle);
}

/// Get a socket corresponding to a handle in the map
/// \param handle the handle of interest
/// \return socket corresponding to the handle
NetSocket* ProcessTracker::GetSocketFromHandle(CommunicationID handle)
{
    std::map<CommunicationID, NetSocket*>::const_iterator it = m_handleToSocketMap.find(handle);

    if (it != m_handleToSocketMap.end())
    {
        return it->second;
    }

    return NULL;
}
