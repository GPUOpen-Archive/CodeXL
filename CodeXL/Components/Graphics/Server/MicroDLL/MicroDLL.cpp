//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Defines the entry point for the DLL application.
//==============================================================================

#include "MicroDLL.h"
#include "MicroDLLName.h"

#include <windows.h>

#include <Interceptor.h>

#include "../Common/SharedGlobal.h"
#include "../Common/OSWrappers.h"
#include "../Common/Windows/DllReplacement.h"

#include "WrapperInfo.h"
#include "LoadLibrary.h"
#include "CreateProcess.h"

#ifdef _WIN32
    #include "UnhandledExceptionHandler.h"
#endif

static const char* sWebServerName = PERFSERVERNAME; ///< Webserver name

char g_MicroDLLPath[ PS_MAX_PATH ]; ///< micro dll path name

//--------------------------------------------------------------
/// Is micro dll allowed to be injected into this process?
/// check the current executable with the filelist specified. If
/// it is in the list, or the list contains 'all', then injection
/// is allowed.
/// \param modulename name of the executable being started
/// \returns true if injection is allowed, false otherwise
static bool InjectionAllowed(const char* modulename)
{
    // if not using AppInit_DLLs registry key, always allow injection
    if (SG_GET_BOOL(OptionAppInitDll) == false)
    {
        return true;
    }

    // get the filename from the path. If there's an error here, don't inject
    char path[PS_MAX_PATH];
    char* filename;
    DWORD res = GetFullPathName(modulename, PS_MAX_PATH, path, &filename);

    if (res == 0)
    {
        return false;
    }

    gtASCIIString fileString = SG_GET_PATH(AppInitDllFileList);

    // if 'all' specified, always inject
    if (fileString.compareNoCase("all") == 0)
    {
        return true;
    }

    // split the list up and see if the current exe is in the list
    std::list<gtASCIIString> fileList;
    fileString.Split(",", false, fileList);

    for (std::list<gtASCIIString>::const_iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        if ((*it).compareNoCase(filename) == 0)
        {
            // file is in list, inject
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------
///   DllMain
/// \param hModule Module
/// \param ul_reason_for_call Reason for call
/// \param lpReserved Reserved
/// \return True or false
//--------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    PS_UNREFERENCED_PARAMETER(hModule);
    PS_UNREFERENCED_PARAMETER(lpReserved);

    if (AMDT::InitHookDLL(ul_reason_for_call) == false)
    {
        return TRUE;
    }

    BOOL retVal = TRUE;
    char modulename[MAX_PATH];
    GetModuleFileNameA(NULL, modulename, MAX_PATH);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            if (OSWrappers::IsProcessRunning(sWebServerName, true) && InjectionAllowed(modulename))
            {
                Log(logMESSAGE, "Attaching to %s\n", modulename);
                Log(logMESSAGE, "DllMain DLL_PROCESS_ATTACH module %s\n", modulename);

                if (SG_GET_BOOL(OptionDllReplacement) == true)
                {
                    UpdateHooksOnLoadLibrary();
                }

#ifdef _WIN32
                {
                    // Initialize and register the unhandled exception handler:
                    bool rc1 = UnhandledExceptionHandler::init();

                    if (rc1)
                    {
                        Log(logMESSAGE, "Registered unhandled exception handler\n");
                    }
                    else
                    {
                        Log(logERROR, "Failed to register unhandled exception handler\n");
                    }
                }
#endif

                // @Note: Do we need to do this? Overwriting it doesn't seem to break anything?
                // get current directory so we can specify as a location for DLLs to be loaded from
                char curDir[PS_MAX_PATH];
                GetCurrentDirectory(PS_MAX_PATH, curDir);

                // SetDllDirectory requires XP SP1 or later.
                SetDllDirectory(curDir);

                sprintf_s(g_MicroDLLPath, PS_MAX_PATH, "%s", SG_GET_PATH(MicroDLLPath));

                CollectWrapperInfo();

                if (SG_GET_BOOL(OptionNoProcessTrack) == false)
                {
                    HookCreateProcess();
                    Log(logMESSAGE, "Process Tracking is ON\n");
                }
                else
                {
                    Log(logMESSAGE, "Process Tracking is OFF\n");
                }

                // At the moment, dxgi.dll is implicitly loaded from HookLoadLibrary
                // Force the Dll folder to point to our replaced dll's so the replace version
                // of dxgi.dll is loaded
                if (SG_GET_BOOL(OptionDllReplacement) == true)
                {
                    // Get architecture of parent application (32 or 64 bit)
                    osModuleArchitecture binaryType;
                    OSWrappers::GetBinaryType(modulename, &binaryType);
                    DllReplacement::SetDllDirectory(binaryType == OS_X86_64_ARCHITECTURE);
                }

                if (SG_GET_BOOL(OptionManualDllReplacement) == false)
                {
                    HookLoadLibrary();
                }
            }
            else
            {
                // set return value to FALSE. This will indicate a load error so the loader will
                // next unload this dll
                retVal = FALSE;
            }

            break;
        }

        case DLL_THREAD_ATTACH:
        {
            Log(logMESSAGE, "DllMain DLL_THREAD_ATTACH to %i %s\n", GetCurrentProcessId(), modulename);
        }
        break;

        case DLL_THREAD_DETACH:
        {
            Log(logMESSAGE, "DllMain DLL_THREAD_DETACH to %i %s\n", GetCurrentProcessId(), modulename);
        }
        break;

        case DLL_PROCESS_DETACH:
        {
            if (OSWrappers::IsProcessRunning(sWebServerName, true))
            {
                Log(logMESSAGE, "DllMain DLL_PROCESS_DETACH from module %s\n", modulename);

                if (SG_GET_BOOL(OptionNoProcessTrack) == false)
                {
                    UnhookCreateProcess();
                }

                UnhookLoadLibrary();
            }
        }
        break;

        default:
        {
            Log(logMESSAGE, "DllMain Unhandled switch case module %s\n", modulename);
        }
        break;
    }

    return retVal;
}

#if 0
// This is an example of an exported function.
MICRODLL_API int fnMicroDLL(void)
{
    return 42;
}
#endif
