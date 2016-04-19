//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains helper functions for the MicroDLL plugin.
//==============================================================================

#include "PluginInfo.h"
#include <vector>
#include <direct.h>
#include "..\Common\Defs.h"
#include "..\Common\StringUtils.h"
#include "..\Common\FileUtils.h"
#include "..\Common\Logger.h"
#include "DLLMain.h"
#include <tchar.h>

#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

using std::vector;
using std::string;
using namespace GPULogger;

static vector<PluginInfo> g_sPluginList;

bool CheckDXRuntime()
{
    bool res = false;
    TCHAR szWindir[SP_MAX_PATH];

    if (GetEnvironmentVariable(_T("windir"), szWindir, SP_MAX_PATH) != 0)
    {
        TCHAR szSys32D3D11[SP_MAX_PATH];
        TCHAR szSys64D3D11[SP_MAX_PATH];

        _stprintf_s(szSys32D3D11, SP_MAX_PATH, _T("%s\\System32\\d3d11.dll"), szWindir);
        _stprintf_s(szSys64D3D11, SP_MAX_PATH, _T("%s\\SysWOW64\\d3d11.dll"), szWindir);
        gtString sys32FileName(szSys32D3D11);
        gtString sys64FileName(szSys64D3D11);
        osFile sys32File(sys32FileName);
        osFile sys64File(sys32FileName);

        // Check if the file exists using unicode path:
        res = res || sys32File.exists();
        res = res || sys64File.exists();
    }

    return res;
}

bool CheckCLRuntime()
{
    bool res = false;
    TCHAR szWindir[SP_MAX_PATH];
    TCHAR szCurrentPathCL[SP_MAX_PATH];

    if (GetEnvironmentVariable(_T("windir"), szWindir, SP_MAX_PATH) != 0)
    {
        TCHAR szSys32CL[SP_MAX_PATH];
        TCHAR szSys64CL[SP_MAX_PATH];

        _stprintf_s(szSys32CL, SP_MAX_PATH, _T("%s\\System32\\OpenCL.dll"), szWindir);
        _stprintf_s(szSys64CL, SP_MAX_PATH, _T("%s\\SysWOW64\\OpenCL.dll"), szWindir);
        gtString cl32FileName(szSys32CL);
        gtString cl64FileName(szSys64CL);
        osFile sys32(cl32FileName);
        osFile sys64(cl64FileName);
        res = res || sys32.exists();
        res = res || sys64.exists();
    }

    TCHAR* buffer = _tgetcwd(NULL, 0);

    if (buffer == NULL)
    {
        return res;
    }

    _stprintf_s(szCurrentPathCL, SP_MAX_PATH, _T("%s\\OpenCL.dll"), buffer);
    gtString workDirName(szCurrentPathCL);
    osFile workDir(workDirName);
    res = res || workDir.exists();
    free(buffer);
    return res;
}

void InitAvailablePluginInfo()
{
    Parameters params;
    FileUtils::GetParametersFromFile(params);

    SP_TODO("Solve Issues for installed libraries in unicode directory");
    // The converted path is used since detours is currently only installed in ascii folder:
    std::string convertedDllPath;
    StringUtils::WideStringToUtf8String(params.m_strDLLPath.asCharArray(), convertedDllPath);

    g_strOutputFile = params.m_strOutputFile;
    g_strDLLPath = convertedDllPath;
    g_strCounterFile = params.m_strCounterFile;

    // Add DCServer.dll directory to dll search path
    // TODO: Refer to GPUPerfstudio changelist 361757
    SetDllDirectoryA(g_strDLLPath.c_str());

    g_strMicroDllPath = g_strDLLPath + MICRO_DLL;

    PluginInfo piDC;
    piDC.strPluginPath = g_strDLLPath + DC_SERVER_DLL;
    piDC.strPluginName = "Direct Compute Server";
    piDC.shouldForceLoad = true;

    PluginInfo piCLOcc;
    piCLOcc.strPluginPath = g_strDLLPath + CL_OCCUPANCY_AGENT_DLL;
    piCLOcc.strPluginName = "OpenCL Occupancy Agent";
    piCLOcc.shouldForceLoad = false;

    PluginInfo piCLTrace;
    piCLTrace.strPluginPath = g_strDLLPath + CL_TRACE_AGENT_DLL;
    piCLTrace.strPluginName = "OpenCL Trace Agent";
    piCLTrace.shouldForceLoad = false;

    if (CheckDXRuntime())
    {
        g_sPluginList.push_back(piDC);
    }
    else
    {
        Log(logMESSAGE, "DX runtime doesn't exist\n");
    }

    g_sPluginList.push_back(piCLTrace);
    g_sPluginList.push_back(piCLOcc);


    for (vector<PluginInfo>::iterator it = g_sPluginList.begin(); it != g_sPluginList.end(); it++)
    {
        if (!it->IsLoaded() && it->shouldForceLoad)
        {
            it->hModule = LoadLibraryA(it->strPluginPath.c_str());
        }
    }
}

void CheckOnLoadLibrary()
{
    for (vector<PluginInfo>::iterator it = g_sPluginList.begin(); it != g_sPluginList.end(); it++)
    {
        if (it->IsLoaded())
        {
            CallFunction(it->hModule, "UpdateHooks");
        }
        else
        {
            it->hModule = GetModuleHandleA(it->strPluginPath.c_str());
        }
    }
}

void CheckOnFreeLibrary()
{
    for (vector<PluginInfo>::iterator it = g_sPluginList.begin(); it != g_sPluginList.end(); it++)
    {
        if (it->IsLoaded() && NULL == GetModuleHandleA(it->strPluginPath.c_str()))
        {
            it->hModule = NULL;
        }
    }
}


bool CallFunction(HMODULE hLib, LPCSTR functionName)
{
    UpdateHooks_type pUpdateHooksFunc = (UpdateHooks_type)GetProcAddress(hLib, functionName);

    if (pUpdateHooksFunc != NULL)
    {
        return pUpdateHooksFunc();
    }
    else
    {
        return false;
    }
}

void NotifyOnExitProcess()
{
    for (vector<PluginInfo>::iterator it = g_sPluginList.begin(); it != g_sPluginList.end(); it++)
    {
        if (it->IsLoaded())
        {
            CallFunction(it->hModule, "OnExitProcess");
        }
    }
}
