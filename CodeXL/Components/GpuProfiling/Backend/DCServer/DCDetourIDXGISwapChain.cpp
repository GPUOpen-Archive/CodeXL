//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Currently not used
//==============================================================================

#include "DCDetourIDXGISwapChain.h"
#include "Interceptor.h"
#include "..\Common\Logger.h"
#include "DCDetourHelper.h"
#include "DCFuncDefs.h"
#include "DCDetourID3D11DeviceContext.h"

using namespace GPULogger;

static bool g_sbAttached = false;

//DX11VTableManager DetourSwapChainRelease( "SwapChain", 2, ( ptrdiff_t * )IDXGISwapChain_Release );
//
//ULONG WINAPI IDXGISwapChain_Release( IUnknown *pObj )
//{
//   return DetourSwapChainRelease.RemoveAndDetach( pObj );
//}

HRESULT WINAPI Mine_IDXGISwapChain_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    HRESULT hr = Real_IDXGISwapChain_Present(pSwapChain, SyncInterval, Flags);
    return hr;
}

bool DetourAttachIDXGISwapChain(IDXGISwapChain* pSwapChain)
{
    if (g_sbAttached)
    {
        return false;
    }

    Real_IDXGISwapChain_Present = (IDXGISwapChain_Present_type)GetD3D11StaticOffset(pSwapChain, 8);

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::HookAPICall(&(PVOID&)Real_IDXGISwapChain_Present, Mine_IDXGISwapChain_Present);
        error |= AMDT::EndHook();
    }

    if (NO_ERROR != error)
    {
        Log(traceMESSAGE, "DetourAttach - IDXGISwapChain failed");
    }

    Log(traceMESSAGE, "DetourAttach - IDXGISwapChain(0x%p) : successful", pSwapChain);

    g_sbAttached = true;

    return true;
}

bool DetourDetachIDXGISwapChain()
{
    if (!g_sbAttached)
    {
        return false;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_IDXGISwapChain_Present, Mine_IDXGISwapChain_Present);
        error |= AMDT::EndHook();
    }

    if (error  != NO_ERROR)
    {
        Log(traceMESSAGE, "DetourDetach - IDXGISwapChain failed");
    }

    Log(traceMESSAGE, "DetourDetach - IDXGISwapChain : Successful");

    g_sbAttached = false;

    return true;
}