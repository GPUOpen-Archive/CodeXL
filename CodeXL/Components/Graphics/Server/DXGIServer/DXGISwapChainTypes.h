//============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Types used in the VTable patching of the swapchain
//============================================================================


#ifndef DXGI_SWAPCHAIN_TYPES
#define DXGI_SWAPCHAIN_TYPES

#include <dxgi1_4.h>

/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* IDXGISwapChain_Present_type)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* IDXGISwapChain_Present1_type)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);
/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* IDXGISwapChain_ResizeBuffers1_type)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue);

/// Function pointer vtable offsets
enum DXGI_Swapchain_Func_Ordinal
{
    IDXGISwapChain_Release_Func_Ordinal = 2,
    IDXGISwapChain_Present_Func_Ordinal = 8,
    IDXGISwapChain1_Present1_Func_Ordinal = 22,
    IDXGISwapChain3_ResizeBuffers1_Func_Ordinal = 39,
};


#endif // DXGI_SWAPCHAIN_TYPES