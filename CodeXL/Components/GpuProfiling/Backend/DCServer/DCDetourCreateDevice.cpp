//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCDetourCreateDevice.h"
#include "DCDetourID3D11DeviceContext.h"
#include "Interceptor.h"
#include "DCDetourID3D11Device.h"
#include "DCDetourIDXGISwapChain.h"
#include "DCFuncDefs.h"
#include "DCGPAProfiler.h"
#include "..\Common\Defs.h"
#include "..\Common\Logger.h"
#include "..\Common\FileUtils.h"
#include "..\Common\GlobalSettings.h"
#include "..\Common\Windows\RefTracker.h"

using std::string;
using namespace GPULogger;

extern DCGPAProfiler g_Profiler;

RefTrackerCounter g_dwInsideWrapper;

//-----------------------------------------------------------------------------
// wrapper for creating a D3D11 Device
//-----------------------------------------------------------------------------
HRESULT WINAPI Mine_D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter,
                                                  D3D_DRIVER_TYPE DriverType,
                                                  HMODULE Software,
                                                  UINT Flags,
                                                  CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                                  UINT FeatureLevels,
                                                  UINT SDKVersion,
                                                  DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
                                                  IDXGISwapChain** ppSwapChain,
                                                  ID3D11Device** ppDevice,
                                                  D3D_FEATURE_LEVEL* pFeatureLevel,
                                                  ID3D11DeviceContext** ppImmediateContext)

{
    HRESULT hr;

    hr = Real_D3D11CreateDeviceAndSwapChain(pAdapter,
                                            DriverType,
                                            Software,
                                            Flags,
                                            pFeatureLevels,
                                            FeatureLevels,
                                            SDKVersion,
                                            pSwapChainDesc,
                                            ppSwapChain,
                                            ppDevice,
                                            pFeatureLevel,
                                            ppImmediateContext);
    RefTracker rf(&g_dwInsideWrapper);

    if (SUCCEEDED(hr))
    {
        if ((ppSwapChain != NULL) && ((*ppSwapChain) != NULL))
        {
            DetourAttachIDXGISwapChain(*ppSwapChain);
        }

        if (g_dwInsideWrapper == 1)
        {
            SpAssert((*ppImmediateContext) != NULL);
            GetID3D11DeviceContextVTableManager()->Patch(ppImmediateContext, true);
            GetID3D11DeviceVTableManager()->Patch(ppDevice, true);
            g_Profiler.SetID3D11Device(*ppDevice);
            g_Profiler.GetContextManager().SetID3D11Device(*ppDevice);
            g_Profiler.GetContextManager().SetID3D11DeviceContext(*ppImmediateContext);
            g_Profiler.GetCommandRecorder().SetID3D11Device(*ppDevice);
            g_Profiler.GetCommandRecorder().SetID3D11DeviceContext(*ppImmediateContext);
        }
    }

    return hr;
}


HRESULT WINAPI Mine_D3D11CreateDevice(IDXGIAdapter* pAdapter,
                                      D3D_DRIVER_TYPE DriverType,
                                      HMODULE Software,
                                      UINT Flags,
                                      CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                      UINT FeatureLevels,
                                      UINT SDKVersion,
                                      ID3D11Device** ppDevice,
                                      D3D_FEATURE_LEVEL* pFeatureLevel,
                                      ID3D11DeviceContext** ppImmediateContext)
{
    HRESULT hr;
    RefTracker rf(&g_dwInsideWrapper);
    hr = Real_D3D11CreateDevice(pAdapter,
                                DriverType,
                                Software,
                                Flags,
                                pFeatureLevels,
                                FeatureLevels,
                                SDKVersion,
                                ppDevice,
                                pFeatureLevel,
                                ppImmediateContext);

    if (SUCCEEDED(hr))
    {
        // TODO this is a workaround to BUG445931
        // There is an over release of the D3D11 device in the DC server (or possibly the GPA, although
        // investigation points to the DC server), this causes an application crash, or if the device memory
        // was not deleted to cause the application to think the device is not released - the ref count is corrupt.
        // To avoid this issue, we increment the device ref count here.
        (*ppDevice)->AddRef();

        if (ppImmediateContext != NULL && g_dwInsideWrapper == 1)
        {
            SpAssert((*ppImmediateContext) != NULL);
            GetID3D11DeviceContextVTableManager()->Patch(ppImmediateContext, true);
            GetID3D11DeviceVTableManager()->Patch(ppDevice, true);
            g_Profiler.SetID3D11Device(*ppDevice);
            g_Profiler.GetContextManager().SetID3D11Device(*ppDevice);
            g_Profiler.GetContextManager().SetID3D11DeviceContext(*ppImmediateContext);
            g_Profiler.GetCommandRecorder().SetID3D11Device(*ppDevice);
            g_Profiler.GetCommandRecorder().SetID3D11DeviceContext(*ppImmediateContext);
        }
    }

    return hr;
}

DCDetourD3D11CreateDevice::DCDetourD3D11CreateDevice(): DetourBase(L"d3d11.dll")
{

}


bool DCDetourD3D11CreateDevice::OnAttach()
{
    // don't detour twice
    if (Real_D3D11CreateDevice != NULL ||
        Real_D3D11CreateDeviceAndSwapChain != NULL)
    {
        /// m_GPALoader.load() loads D3D11.dll which cause this happen
        Log(traceMESSAGE, "DetourAttach - D3D11CreateDevice : Already detoured");
        return true;
    }

    Parameters params;
    FileUtils::GetParametersFromFile(params);

    // If CL/DirectCompute perf counter mode is not specified, we don't detour anything
    if (!params.m_bPerfCounter)
    {
        return true;
    }

    GlobalSettings::GetInstance()->m_bVerbose = params.m_bVerbose;

    Real_D3D11CreateDevice = (D3D11CreateDevice_type)GetProcAddress(m_hMod, "D3D11CreateDevice");    //D3D11CreateDevice;
    Real_D3D11CreateDeviceAndSwapChain = (D3D11CreateDeviceAndSwapChain_type)GetProcAddress(m_hMod, "D3D11CreateDeviceAndSwapChain");    //D3D11CreateDeviceAndSwapChain;

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {

        error |= AMDT::HookAPICall(&(PVOID&)Real_D3D11CreateDevice, Mine_D3D11CreateDevice);
        error |= AMDT::HookAPICall(&(PVOID&)Real_D3D11CreateDeviceAndSwapChain, Mine_D3D11CreateDeviceAndSwapChain);
        error |= AMDT::EndHook();
    }

    if (NO_ERROR != error)
    {
        Log(traceMESSAGE, "DetourAttach - D3D11CreateDevice() failed");
    }

    if (!g_Profiler.Loaded())
    {
        std::string strError;
        g_Profiler.Init(params, strError);
    }

    Log(traceMESSAGE, "DetourAttach - D3D11CreateDevice : Successful");

    return true;
}

bool DCDetourD3D11CreateDevice::Detach()
{
    // don't detach detour if not attached - this is a valid operation
    if (Real_D3D11CreateDevice == NULL)
    {
        return true;
    }

    DetourBase::Detach();
    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_D3D11CreateDevice, Mine_D3D11CreateDevice);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_D3D11CreateDeviceAndSwapChain, Mine_D3D11CreateDeviceAndSwapChain);
        error |= AMDT::EndHook();
    }

    if (NO_ERROR != error)
    {
        Log(logERROR, "DetourDetach - D3D11CreateDevice failed\n");
    }

    Real_D3D11CreateDevice = NULL;
    Real_D3D11CreateDeviceAndSwapChain = NULL;

    if (g_Profiler.Loaded())
    {
        g_Profiler.Unload();
    }

    Log(traceMESSAGE, "DetourDetach - D3D11CreateDevice : Successful");

    return true;
}

