//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Currently not used
//==============================================================================

#include "DCDetourIDXGIFactory.h"
#include "Interceptor.h"
#include "..\Common\Logger.h"
#include "DCDetourHelper.h"
#include "DCFuncDefs.h"
#include "DCDetourID3D11DeviceContext.h"
#include "DCDetourIDXGISwapChain.h"

using namespace GPULogger;

static bool g_sbAttached = false;

ULONG WINAPI Mine_IDXGIFactory1_Release(IDXGIFactory1* pFactory1)
{
    ULONG res = Real_IDXGIFactory1_Release(pFactory1);
    return res;
}
HRESULT WINAPI Mine_IDXGIFactory1_CreateSwapChain(IDXGIFactory1* pFactory1, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    HRESULT hr = Real_IDXGIFactory1_CreateSwapChain(pFactory1, pDevice, pDesc, ppSwapChain);

    if (hr == S_OK)
    {
        if ((ppSwapChain != NULL) && ((*ppSwapChain) != NULL))
        {
            DetourAttachIDXGISwapChain(*ppSwapChain);
        }
    }

    return hr;
}


bool DetourAttachIDXGIFactory(IDXGIFactory1* pFactory)
{
    if (g_sbAttached)
    {
        return false;
    }

    Real_IDXGIFactory1_Release = (IDXGIFactory1_Release_type)GetD3D11StaticOffset(pFactory, 2);
    Real_IDXGIFactory1_CreateSwapChain = (IDXGIFactory1_CreateSwapChain_type)GetD3D11StaticOffset(pFactory, 10);       //create swapchain

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::HookAPICall(&(PVOID&)Real_IDXGIFactory1_CreateSwapChain, Mine_IDXGIFactory1_CreateSwapChain);
        error |= AMDT::EndHook();
    }

    if (error  != NO_ERROR)
    {
        Log(traceMESSAGE, "DetourAttach - IDXGIFactory failed");
    }

    Log(traceMESSAGE, "DetourAttach - IDXGIFactory(0x%p) : Successful", pFactory);

    g_sbAttached = true;

    return true;
}

bool DetourDetachIDXGIFactory()
{
    if (!g_sbAttached)
    {
        return false;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_IDXGIFactory1_CreateSwapChain, Mine_IDXGIFactory1_CreateSwapChain);
        error |= AMDT::EndHook();
    }

    if (error  != NO_ERROR)
    {
        Log(traceMESSAGE, "DetourDetach - IDXGIFactory failed");
    }

    g_sbAttached = false;

    Log(traceMESSAGE, "DetourDetach - IDXGIFactory successful");
    return true;
}
