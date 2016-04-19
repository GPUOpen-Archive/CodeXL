//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Defines the hooked functionality of the CreateProcess API
///         functions.
//==============================================================================

#include <windows.h>

#include <Interceptor.h>
#include "../Common/logger.h"
#include "../Common/misc.h"
#include "../Common/OSWrappers.h"
#include "../Common/SharedGlobal.h"
#include "../Common/Windows/DllReplacement.h"

#include "MicroDLL.h"
#include "MicroDLLName.h"

static RefTrackerCounter s_dwInsideWrapper; ///< refcounter to track when the execution is inside an already wrapped section of code. Used to mask out sections of code that are called by the runtime (not application).

/// List of all applications which MicroDLL should not be injected into.
/// Files MUST be in lower case
static LPCSTR sBlackList[] =
{
    { "fxc.exe" },
    { "cmd.exe" },
    { "dev.exe" },
    { "movescdevoutput.bat" },
};

//=============================================================================
/// GetApplicationName - Get the name of the application from either the
/// application name, or the command line if the application name is NULL
/// \param lpApplicationName name of the target application. If this is NULL the application name will be taken from the command line parameter
/// \param lpCommandLine full command line of the target application
/// \param buffer pointer to a buffer where the application name is written to
//=============================================================================
static void GetApplicationName(LPCSTR lpApplicationName, LPCSTR lpCommandLine, char* buffer)
{
    if (lpApplicationName == NULL || strlen(lpApplicationName) == 0)
    {
        // if there's no application name, then get it from the command line.
        // Take into account whether the command line is in quotes and ignore spaces if so

        // if the command line string contains a '.exe' string, assume this is the application name and use that
        const char* pExe = strstr(lpCommandLine, ".exe");

        if (pExe != NULL)
        {
            // find length of string up to start of '.exe'
            int length = (int)(pExe - lpCommandLine);

            // add length of '.exe'
            length += 4;

            // copy the application name from the command line string, ignoring '"' characters
            int index = 0;

            for (int loop = 0; loop < length; loop++)
            {
                char currentChar = lpCommandLine[loop];

                if (currentChar != '\"')
                {
                    buffer[index++] = currentChar;
                }

                buffer[index] = '\0';
            }
        }
        else
        {
            int insideQuotes = 0;
            int index = 0;

            for (int loop = 0; loop < PS_MAX_PATH; loop++)
            {
                char currentChar = lpCommandLine[loop];

                if (currentChar == '\"')
                {
                    // it's a '"', so toggle the quote flag. Don't copy character to the buffer
                    insideQuotes = 1 - insideQuotes;   // toggle value 0 -> 1, 1 -> 0
                }
                else if (currentChar == ' ')
                {
                    // it's a space character, so decide what to do. If inside a quoted string, treat it as a normal
                    // character. Otherwise, use it as a string terminator
                    if (insideQuotes == 0)
                    {
                        buffer[index] = '\0';
                        break;
                    }
                    else
                    {
                        buffer[index++] = currentChar;
                    }
                }
                else
                {
                    // copy the character
                    buffer[index++] = currentChar;
                }
            }
        }
    }
    else
    {
        strcpy(buffer, lpApplicationName);
    }
}

//=============================================================================
/// SetMicroDLLPathA - Set the path of the MicroDLL plugin to use based on the
/// bitness of the target application
/// \param lpApplicationName name of the target application. If this is NULL
/// the application name will be taken from the command line parameter
/// \param lpCommandLine full command line of the target application
//=============================================================================
static void SetMicroDLLPathA(LPCSTR lpApplicationName, LPCSTR lpCommandLine)
{
    char appName[PS_MAX_PATH];
    GetApplicationName(lpApplicationName, lpCommandLine, appName);

    // Get the binary type of the process this MicroDLL is running in.
#if defined X64
    osModuleArchitecture appBinaryType = OS_X86_64_ARCHITECTURE;
    const osModuleArchitecture currentBinaryType = OS_X86_64_ARCHITECTURE;
#else
    osModuleArchitecture appBinaryType = OS_I386_ARCHITECTURE;
    const osModuleArchitecture currentBinaryType = OS_I386_ARCHITECTURE;
#endif
    sprintf_s(g_MicroDLLPath, PS_MAX_PATH, "%s" MICRODLLNAME "%s.dll", SG_GET_PATH(ServerPath), GDT_PROJECT_SUFFIX);

    // Get binary type of the application that's loaded in and make sure the correct version
    // of MicroDLL is injected (either 32 or 64 bit)
    bool bGetType = false;
    bGetType = OSWrappers::GetBinaryType(appName, &appBinaryType);

    if (bGetType)
    {
        if (currentBinaryType != appBinaryType)
        {
            // if binary types of application and the current process are different, a loader is going to be needed;
            // Example: a 64-bit dll can't be loaded into a new 64-bit process from within a 32 bit process. This is
            // the example of steam loading a 64-bit game. In this case, hooking fails when trying to patch the import
            // tables (I think)
            if (appBinaryType == OS_X86_64_ARCHITECTURE)
            {
                sprintf_s(g_MicroDLLPath, PS_MAX_PATH, "%s" MICRODLLNAME "-x64%s%s.dll", SG_GET_PATH(ServerPath), GDT_DEBUG_SUFFIX, GDT_BUILD_SUFFIX);
            }
            else if (appBinaryType == OS_I386_ARCHITECTURE)
            {
                sprintf_s(g_MicroDLLPath, PS_MAX_PATH, "%s" MICRODLLNAME "%s%s.dll", SG_GET_PATH(ServerPath), GDT_DEBUG_SUFFIX, GDT_BUILD_SUFFIX);
            }
        }
    }
    else
    {
        //        LogConsole(logERROR, "SetMicroDLLPath(%s) can't determine bitness of file '%s'\n", GDT_PROJECT_SUFFIX, appName);
    }
}

//=============================================================================
/// SetMicroDLLPathW - Wide char version of SetMicroDLLPathA
/// \param lpApplicationName name of the target application. If this is NULL
/// the application name will be taken from the command line parameter
/// \param lpCommandLine full command line of the target application
//=============================================================================
static void SetMicroDLLPathW(LPCWSTR lpApplicationName, LPCWSTR lpCommandLine)
{
    gtString appName(lpApplicationName);
    gtString commandLine(lpCommandLine);

    SetMicroDLLPathA(appName.asASCIICharArray(), commandLine.asASCIICharArray());
}

//=============================================================================
/// GetApplicationTypeA - Get the type (bitness) of the application. Used to
/// determine if an application is 32 or 64 bit
///
/// \param lpApplicationName name of the target application. If this is NULL
/// the application name will be taken from the command line parameter
/// \param lpCommandLine full command line of the target application
/// \return the type of the application, as a value from the osModuleArchitecture
/// enum
//=============================================================================
static osModuleArchitecture GetApplicationTypeA(LPCSTR lpApplicationName, LPCSTR lpCommandLine)
{
    char appName[PS_MAX_PATH];
    osModuleArchitecture appBinaryType = OS_I386_ARCHITECTURE;
    GetApplicationName(lpApplicationName, lpCommandLine, appName);
    OSWrappers::GetBinaryType(appName, &appBinaryType);
    return appBinaryType;
}

//=============================================================================
/// GetApplicationTypeW - Wide char version of GetApplicationTypeA
///
/// \param lpApplicationName name of the target application. If this is NULL
/// the application name will be taken from the command line parameter
/// \param lpCommandLine full command line of the target application
/// \return the type of the application, as a value from the osModuleArchitecture
/// enum
//=============================================================================
static osModuleArchitecture GetApplicationTypeW(LPCWSTR lpApplicationName, LPCWSTR lpCommandLine)
{
    gtString appName(lpApplicationName);
    gtString commandLine(lpCommandLine);

    return GetApplicationTypeA(appName.asASCIICharArray(), commandLine.asASCIICharArray());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Returns true if the input name is on the blacklist of processes not to inject into
/// \param appPathOrName The path/exe name
/// \return true if the app is blacklisted
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool BlockInjectionCheck(LPCWSTR appPathOrName)
{
    gtString appNameCopy(appPathOrName);
    appNameCopy.toLowerCase();

    gtString blacklistFile;

    int count = sizeof(sBlackList) / sizeof(sBlackList[0]);

    for (int loop = 0; loop < count; loop++)
    {
        blacklistFile.fromASCIIString(sBlackList[loop]);

        if (wcsstr(appNameCopy.asCharArray(), blacklistFile.asCharArray()) != NULL)
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Returns true if the input name is on the blacklist of processes not to inject into
/// \param appPathOrName The path/exe name
/// \return true if the app is blacklisted
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool BlockInjectionCheck(LPCSTR appPathOrName)
{
    gtASCIIString appNameCopy(appPathOrName);
    appNameCopy.toLowerCase();

    int count = sizeof(sBlackList) / sizeof(sBlackList[0]);

    for (int loop = 0; loop < count; loop++)
    {
        if (strstr(appNameCopy.asCharArray(), sBlackList[loop]) != NULL)
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Returns true if either of the input names is on the blacklist of processes not to inject into
/// \param lpApplicationName A path/exe name
/// \param lpCommandLine A path/exe name
/// \return true if either of the the apps is blacklisted
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool BlockInjection(LPCSTR lpApplicationName, LPSTR lpCommandLine)
{
    bool bBlockLoad = false;

    if (lpApplicationName != NULL && strlen(lpApplicationName) > 0)
    {
        bBlockLoad = BlockInjectionCheck(lpApplicationName);

        if (bBlockLoad == true)
        {
            return true;
        }
    }

    if (lpCommandLine != NULL)
    {
        bBlockLoad = BlockInjectionCheck((LPCSTR)lpCommandLine);

        if (bBlockLoad == true)
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Returns true if either of the input names is on the blacklist of processes not to inject into
/// \param lpApplicationName A path/exe name
/// \param lpCommandLine A path/exe name
/// \return true if either of the the apps is blacklisted
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool BlockInjection(LPCWSTR lpApplicationName, LPWSTR lpCommandLine)
{
    bool bBlockLoad = false;

    if (lpApplicationName != NULL && wcslen(lpApplicationName) > 0)
    {
        bBlockLoad = BlockInjectionCheck(lpApplicationName);

        if (bBlockLoad == true)
        {
            return true;
        }
    }

    if (lpCommandLine != NULL)
    {
        bBlockLoad = BlockInjectionCheck((LPCWSTR)lpCommandLine);

        if (bBlockLoad == true)
        {
            return true;
        }
    }

    return false;
}

/// Function pointer typedef to intercepted function
typedef BOOL (WINAPI* CreateProcessA_type)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

/// Assign the real function to our real function pointer
CreateProcessA_type Real_CreateProcessA = CreateProcessA;

/// Entry point for the intercepted function
/// \param lpApplicationName Application name
/// \param lpCommandLine Command line
/// \param lpProcessAttributes Process attributes
/// \param lpThreadAttributes Thread attributes
/// \param bInheritHandles Inherit handles flag
/// \param dwCreationFlags flags
/// \param lpEnvironment Environment
/// \param lpCurrentDirectory Current directory
/// \param lpStartupInfo Startup info
/// \param lpProcessInformation Process info
/// \return True or false
BOOL WINAPI Mine_CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    LogTrace(traceENTER, "%s, %s", lpApplicationName, lpCommandLine);

    if (SG_GET_BOOL(OptionDllReplacement) == true)
    {
        osModuleArchitecture appBinaryType = GetApplicationTypeA(lpApplicationName, lpCommandLine);
        DllReplacement::SetDllDirectory(appBinaryType == OS_X86_64_ARCHITECTURE);
    }

    SetMicroDLLPathA(lpApplicationName, lpCommandLine);

    BOOL rVal;

    if (s_dwInsideWrapper == 0)
    {
        RefTracker rf(&s_dwInsideWrapper);

        LogTrace(traceMESSAGE, "With microdll!");

        // Get the max hop count
        int hopCountMax = SG_GET_INT(OptionHopCountMax);

        // Get the current hop count
        int hopCount = SG_GET_INT(OptionHopCount);

        bool bBlockLoad = false;

        // if steam is running, ignore the checks for fxc.exe and cmd.exe
        if (SG_GET_BOOL(SteamInjected) == false)
        {
            bBlockLoad = BlockInjection(lpApplicationName, lpCommandLine);
        }

        // Decide if we need to inject into the new process
        if (hopCount >= hopCountMax || bBlockLoad == true)
        {
            rVal = Real_CreateProcessA(lpApplicationName,
                                       lpCommandLine,
                                       lpProcessAttributes,
                                       lpThreadAttributes,
                                       bInheritHandles,
                                       dwCreationFlags,
                                       lpEnvironment,
                                       lpCurrentDirectory,
                                       lpStartupInfo,
                                       lpProcessInformation);
        }
        else
        {
            // Update the current hop count
            SG_SET_INT(OptionHopCount, hopCount + 1);

            rVal = AMDT::CreateProcessAndInjectDllA(lpApplicationName,
                                                    lpCommandLine,
                                                    lpProcessAttributes,
                                                    lpThreadAttributes,
                                                    bInheritHandles,
                                                    dwCreationFlags,
                                                    lpEnvironment,
                                                    lpCurrentDirectory,
                                                    lpStartupInfo,
                                                    lpProcessInformation,
                                                    g_MicroDLLPath);

            // if succeeded in injecting this DLL, check if the application is Steam.exe
            // and set a flag indicating so. If Steam loads fxc or cmd, the check for these
            // processes needs to be ignored.
            if (rVal)
            {
                if (lpApplicationName != NULL)
                {
                    gtASCIIString appNameCopy(lpApplicationName);
                    appNameCopy.toLowerCase();

                    if (strstr(appNameCopy.toLowerCase().asCharArray(), "steam.exe") != NULL)
                    {
                        SG_SET_BOOL(SteamInjected, true);
                    }

                    ShowLauncherReminder(appNameCopy.toLowerCase().asCharArray());
                }
                else
                {
                    gtASCIIString appNameCopy(lpCommandLine);
                    appNameCopy.toLowerCase();

                    if (strstr(appNameCopy.toLowerCase().asCharArray(), "steam.exe") != NULL)
                    {
                        SG_SET_BOOL(SteamInjected, true);
                    }

                    ShowLauncherReminder(appNameCopy.toLowerCase().asCharArray());
                }
            }
        }
    }
    else
    {
        rVal = Real_CreateProcessA(lpApplicationName,
                                   lpCommandLine,
                                   lpProcessAttributes,
                                   lpThreadAttributes,
                                   bInheritHandles,
                                   dwCreationFlags,
                                   lpEnvironment,
                                   lpCurrentDirectory,
                                   lpStartupInfo,
                                   lpProcessInformation);
    }

    LogTrace(traceEXIT, "returned %d", rVal);
    return rVal;
}

//=============================================================================
/// CreateProcessW_type typedef
//=============================================================================
typedef BOOL (WINAPI* CreateProcessW_type)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

CreateProcessW_type Real_CreateProcessW = CreateProcessW; ///< Assign the real function pointer to the real function

/// Entry point for the intercepted function
/// \param lpApplicationName Application name
/// \param lpCommandLine Command line
/// \param lpProcessAttributes Process attributes
/// \param lpThreadAttributes Thread attributes
/// \param bInheritHandles Inherit handles flag
/// \param dwCreationFlags flags
/// \param lpEnvironment Environment
/// \param lpCurrentDirectory Current directory
/// \param lpStartupInfo Startup info
/// \param lpProcessInformation Process info
/// \return True or false
BOOL WINAPI Mine_CreateProcessW(LPCWSTR lpApplicationName,
                                LPWSTR lpCommandLine,
                                LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles,
                                DWORD dwCreationFlags,
                                LPVOID lpEnvironment,
                                LPCWSTR lpCurrentDirectory,
                                LPSTARTUPINFOW lpStartupInfo,
                                LPPROCESS_INFORMATION lpProcessInformation)
{
    LogTrace(traceENTER, "%S %S", lpApplicationName, lpCommandLine);

    if (SG_GET_BOOL(OptionDllReplacement) == true)
    {
        osModuleArchitecture appBinaryType = GetApplicationTypeW(lpApplicationName, lpCommandLine);
        DllReplacement::SetDllDirectory(appBinaryType == OS_X86_64_ARCHITECTURE);
    }

    SetMicroDLLPathW(lpApplicationName, lpCommandLine);

    BOOL rVal;

    if (s_dwInsideWrapper == 0)
    {
        RefTracker rf(&s_dwInsideWrapper);

        LogTrace(traceMESSAGE, "With microdll!");

        // Get the max hop count
        int hopCountMax = SG_GET_INT(OptionHopCountMax);

        // Get the current hop count
        int hopCount = SG_GET_INT(OptionHopCount);

        bool bBlockLoad = false;

        // if steam is running, ignore the checks for fxc.exe and cmd.exe
        if (SG_GET_BOOL(SteamInjected) == false)
        {
            bBlockLoad = BlockInjection(lpApplicationName, lpCommandLine);
        }

        // Decide if we need to inject into the new process
        if (hopCount >= hopCountMax || bBlockLoad == true)
        {
            rVal = Real_CreateProcessW(lpApplicationName,
                                       lpCommandLine,
                                       lpProcessAttributes,
                                       lpThreadAttributes,
                                       bInheritHandles,
                                       dwCreationFlags,
                                       lpEnvironment,
                                       lpCurrentDirectory,
                                       lpStartupInfo,
                                       lpProcessInformation);
        }
        else
        {
            // Update the current hop count
            SG_SET_INT(OptionHopCount, hopCount + 1);

            rVal = AMDT::CreateProcessAndInjectDllW(lpApplicationName,
                                                    lpCommandLine,
                                                    lpProcessAttributes,
                                                    lpThreadAttributes,
                                                    bInheritHandles,
                                                    dwCreationFlags,
                                                    lpEnvironment,
                                                    lpCurrentDirectory,
                                                    lpStartupInfo,
                                                    lpProcessInformation,
                                                    g_MicroDLLPath);

            // if succeeded in injecting this DLL, check if the application is Steam.exe
            // and set a flag indicating so. If Steam loads fxc or cmd, the check for these
            // processes needs to be ignored.
            if (rVal)
            {
                if (lpApplicationName != NULL)
                {
                    gtString appNameCopy(lpApplicationName);
                    appNameCopy.toLowerCase();

                    if (wcsstr(appNameCopy.toLowerCase().asCharArray(), L"steam.exe") != NULL)
                    {
                        SG_SET_BOOL(SteamInjected, true);
                    }

                    ShowLauncherReminder(appNameCopy.toLowerCase().asCharArray());
                }
                else if (lpCommandLine != NULL)
                {
                    gtString appNameCopy(lpCommandLine);
                    appNameCopy.toLowerCase();

                    if (wcsstr(appNameCopy.toLowerCase().asCharArray(), L"steam.exe") != NULL)
                    {
                        SG_SET_BOOL(SteamInjected, true);
                    }

                    ShowLauncherReminder(appNameCopy.toLowerCase().asCharArray());
                }
            }
        }
    }
    else
    {
        rVal = Real_CreateProcessW(lpApplicationName,
                                   lpCommandLine,
                                   lpProcessAttributes,
                                   lpThreadAttributes,
                                   bInheritHandles,
                                   dwCreationFlags,
                                   lpEnvironment,
                                   lpCurrentDirectory,
                                   lpStartupInfo,
                                   lpProcessInformation);
    }

    LogTrace(traceEXIT, "returned %d", rVal);
    return rVal;
}

//=============================================================================
/// Hook the craete process
/// \return True if success, false if fail.
//=============================================================================
bool HookCreateProcess()
{
    AMDT::BeginHook();

    LONG error = AMDT::HookAPICall(&(PVOID&)Real_CreateProcessA, Mine_CreateProcessA);
    PsAssert(error == NO_ERROR);

    if (error != NO_ERROR)
    {
        return false;
    }

    error = AMDT::HookAPICall(&(PVOID&)Real_CreateProcessW, Mine_CreateProcessW);
    PsAssert(error == NO_ERROR);

    if (error != NO_ERROR)
    {
        return false;
    }

    error = AMDT::EndHook();

    return (error == NO_ERROR);
}

//=============================================================================
/// Unhook the craete process
/// \return True if success, false if fail.
//=============================================================================
bool UnhookCreateProcess()
{
    AMDT::BeginHook();

    LONG error = AMDT::UnhookAPICall(&(PVOID&)Real_CreateProcessA, Mine_CreateProcessA);
    PsAssert(error == NO_ERROR);

    if (error != NO_ERROR)
    {
        return false;
    }

    error = AMDT::UnhookAPICall(&(PVOID&)Real_CreateProcessW, Mine_CreateProcessW);
    PsAssert(error == NO_ERROR);

    if (error != NO_ERROR)
    {
        return false;
    }

    error = AMDT::EndHook();

    if (error != NO_ERROR)
    {
        return false;
    }

    // Restore Real functions to original values in case they aren't restored correctly by the unhook call
    Real_CreateProcessW = CreateProcessW;
    Real_CreateProcessA = CreateProcessA;
    return true;
}
