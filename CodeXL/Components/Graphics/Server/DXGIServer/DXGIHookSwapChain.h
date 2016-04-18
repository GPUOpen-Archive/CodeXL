//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Contains the mechanisms responsible for hooking and wrapping
///         the swap chain.
//==============================================================================

#ifndef DXGI_HOOKSWAPCHAIN_H
#define DXGI_HOOKSWAPCHAIN_H

#include <DXGI.h>
#include "../DXCommonSource/HookVtableImmediate.h"

struct DXGI_PRESENT_PARAMETERS;

/// Function pointer vtable offsets
enum DXGI_Swapchain_Func_Ordinal
{
    IDXGISwapChain_Release_Func_Ordinal = 2,
    IDXGISwapChain_Present_Func_Ordinal = 8,
    IDXGISwapChain1_Present1_Func_Ordinal = 22,
    IDXGISwapChain3_ResizeBuffers1_Func_Ordinal = 39,
};

/// Function pointer typedef for intercepted API functions
typedef ULONG(WINAPI* IDXGISwapChain_Release_type)(IDXGISwapChain* pSwapChain);
/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* IDXGISwapChain_Present_type)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* IDXGISwapChain_Present1_type)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);
/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* IDXGISwapChain_ResizeBuffers1_type)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue);

/// Helper function to hook the swapchain
extern DWORD  HookIDXGISwapChain(IDXGISwapChain* pSwapChain);
/// Helper function to unhook the swapchain
DWORD  UnhookIDXGISwapChain();

/// VTable to store the real to mine mappings
extern HookVtableImmediate HookSwapChainRelease;

#endif // DXGI_HOOKSWAPCHAIN_H
