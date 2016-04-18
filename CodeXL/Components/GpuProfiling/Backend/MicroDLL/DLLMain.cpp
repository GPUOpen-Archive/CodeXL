//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the entry point for the MicroDLL plugin.
//==============================================================================

#include "DLLMain.h"

#include "DetourLoadLibrary.h"
#include "PluginInfo.h"
#include "Interceptor.h"
#include "..\Common\Defs.h"
#include "DetourCreateProcess.h"
#include "DetourExitProcess.h"
#include "..\Common\Logger.h"
#include "..\Common\FileUtils.h"

using namespace GPULogger;

std::string g_strMicroDllPath;
std::string g_strOutputFile;
std::string g_strDLLPath;
std::string g_strCounterFile;

//--------------------------------------------------------------
//   DllMain
//--------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD  ul_reason_for_call, LPVOID /* lpReserved */)
{
    if (!AMDT::InitHookDLL(ul_reason_for_call))
    {
        return TRUE;
    }

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            std::string strLogFile = FileUtils::GetDefaultOutputPath() + "microdll.log";
            LogFileInitialize(strLogFile.c_str());
            InitAvailablePluginInfo();
            DetoursAttachLoadLibrary();
            DetoursAttachCreateProcess();
            DetoursAttachExitProcess();
        }
        break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            DetoursDetachLoadLibrary();
            DetoursDetachCreateProcess();
            DetoursDetachExitProcess();
            break;
    }

    return TRUE;
}

MICRODLL_API const char* GetInfo()
{
    return "CodeXL GPU Profiler - MicroDll";
}