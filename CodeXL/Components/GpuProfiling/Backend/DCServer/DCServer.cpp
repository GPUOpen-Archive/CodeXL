//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include <windows.h>

#include "DCServer.h"

#include "DCDetourCreateDevice.h"
#include "DCDetourCompileShader.h"
#include "DCDetourCreateDXGIFactory1.h"
#include "DCID3D11DeviceContext_wrapper.h"
#include "..\Common\Logger.h"
#include "..\Common\FileUtils.h"

static DCDetourD3D11CreateDevice s_DCDetourD3D11CreateDevice;
static DCDetourD3DCompile s_DCDetourD3DCompile;
static DCDetourCreateDXGIFactory1 s_DCDetourCreateDXGIFactory1;

BOOL APIENTRY DllMain(HMODULE,
                      DWORD   ul_reason_for_call,
                      LPVOID)
{
    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "DCServer.log";

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            // We try to catch all possible module the client apps load
            // If an app uses a DX version that is not supported by the profiler,
            // the profiler won't be able to get the shader name (can't detour the D3DXCompileShader)
            // and will default to a generic name.
            // TODO: If new version come out, we need to add it here
            s_DCDetourD3DCompile.AddDLL(L"D3DCompiler_42.dll");
            s_DCDetourD3DCompile.AddDLL(L"D3DCompiler_43.dll");
            s_DCDetourD3DCompile.AddDLL(L"D3DCompiler_44.dll");
            s_DCDetourD3DCompile.AddDLL(L"D3DCompiler_45.dll");
            s_DCDetourD3DCompile.AddDLL(L"D3DCompiler_46.dll");
            s_DCDetourD3DCompile.AddDLL(L"D3DCompiler_47.dll");

            LogFileInitialize(strLogFile.c_str());
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:

            s_DCDetourD3D11CreateDevice.Detach();
            s_DCDetourD3DCompile.Detach();
            s_DCDetourCreateDXGIFactory1.Detach();

            CleanupWrappers();
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
/// UpdateHooks
///
/// Function which causes all necessary entrypoints to be detoured
///
/// \return true if detouring was successful; false will unload the wrapper
//-----------------------------------------------------------------------------
DCSERVER_PLUGIN_EXPORTS bool UpdateHooks()
{
    s_DCDetourD3D11CreateDevice.Attach();
    s_DCDetourD3DCompile.Attach();
    s_DCDetourCreateDXGIFactory1.Attach();

    return true;
}
