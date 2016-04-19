//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Contains the mechanisms responsible for hooking and wrapping
///         the swap chain.
//==============================================================================

#include "DXGIHookSwapChain.h"
#include "DXGILayerManager.h"
#include "../Common/IUnknownWrapperGUID.h"

class ID3D12CommandQueue;

/// Counter for the number of times the sap chain was hooked
static DWORD s_dwSwapChainHookedTimes = 0;

/// Define the swapchain release function
ULONG WINAPI IDXGISwapChain_Release(IUnknown* pObj);

/// Define the HookSwapChainRelease release function
HookVtableImmediate HookSwapChainRelease("SwapChain", IDXGISwapChain_Release_Func_Ordinal, (ptrdiff_t*)IDXGISwapChain_Release);

/// Handle the destruction of the swapchain object
/// \param pObj Input swapchain pointer
void DestroySwapchain(IUnknown* pObj)
{
    LogTrace(traceENTER, "pSwapChain = 0x%p", pObj);

    bool val = DXGILayerManager::Instance()->OnDestroy(DX11_SWAPCHAIN, pObj);

    LogTrace(traceEXIT, "returned %d", val);
}

/// Handle the release of the swapchain object
/// \param pObj Input swapchain pointer
ULONG WINAPI IDXGISwapChain_Release(IUnknown* pObj)
{
    LogTrace(traceENTER, "pSwapChain = 0x%p", pObj);

    ULONG val = HookSwapChainRelease.RemoveAndDetach(pObj, "DXGIHookSwapchain");

    LogTrace(traceEXIT, "returned %ld", val);

    return val;
}

/// Our interception of the Present call
/// \param pSwapChain Input swapchain
/// \param SyncInterval Input sync interval
/// \param Flags Input flags
/// \return HRESULT return code
HRESULT WINAPI Mine_IDXGISwapChain_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    LogTrace(traceENTER, "pSwapChain = 0x%p, SyncInterval = 0x%u, Flags = 0x%08x",
             pSwapChain, SyncInterval, Flags);

    RefTracker rf(&g_dwInsideDXGI);

    void* pFn = HookSwapChainRelease.GetRealFunction(pSwapChain, IDXGISwapChain_Present_Func_Ordinal);
    HRESULT hRes = ((IDXGISwapChain_Present_type)pFn)(pSwapChain, SyncInterval, Flags);

    DXGILayerManager::Instance()->OnPresent(SyncInterval, Flags);

    if (g_dwInsideDXGI == 1)
    {
        DXGILayerManager::Instance()->EndFrame();

        DXGILayerManager::Instance()->OnPresent(pSwapChain);

        DXGILayerManager::Instance()->RespondToCommands(pSwapChain);

        DXGILayerManager::Instance()->BeginFrame();
    }

    LogTrace(traceEXIT, "returned 0x%08x", hRes);

    return hRes;
}

/// Our interception of the Present1 API call
/// \param pSwapChain Input swapchain
/// \param SyncInterval Input sync interval
/// \param Flags Input flags
/// \param pPresentParameters Present params
/// \return HRESULT return code
HRESULT WINAPI Mine_IDXGISwapChain_Present1(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    LogTrace(traceENTER, "pSwapChain = 0x%p, SyncInterval = 0x%u, Flags = 0x%08x, PresentParams = IDK",
             pSwapChain, SyncInterval, Flags);

    RefTracker rf(&g_dwInsideDXGI);

    void* pFn = HookSwapChainRelease.GetRealFunction(pSwapChain, IDXGISwapChain1_Present1_Func_Ordinal);
    HRESULT hRes = ((IDXGISwapChain_Present1_type)pFn)(pSwapChain, SyncInterval, Flags, pPresentParameters);

    DXGILayerManager::Instance()->OnPresent(SyncInterval, Flags);

    if (g_dwInsideDXGI == 1)
    {
        DXGILayerManager::Instance()->EndFrame();

        DXGILayerManager::Instance()->OnPresent(pSwapChain);

        DXGILayerManager::Instance()->RespondToCommands(pSwapChain);

        DXGILayerManager::Instance()->BeginFrame();
    }

    LogTrace(traceEXIT, "returned 0x%08x", hRes);

    return hRes;
}

/// Our interception of the ResizeBuffers1 API call
/// \param pSwapChain Input swapchain
/// \param BufferCount Input buffer count
/// \param Width Input Width
/// \param Height Input Height
/// \param Format Input Format
/// \param SwapChainFlags Input flags
/// \param pCreationNodeMask Input node mask
/// \param ppPresentQueue Input present queue
/// \return HRESULT return code
HRESULT Mine_IDXGISwapChain_ResizeBuffers1(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    RefTracker rf(&g_dwInsideDXGI);

    IUnknown** presentQueueArgs = const_cast<IUnknown**>(ppPresentQueue);

    if (BufferCount > 0)
    {
        // Create a new CommandQueue array with the same size, because each instance may be wrapped.
        IUnknown** unwrappedQueues = new IUnknown*[BufferCount];

        for (UINT index = 0; index < BufferCount; index++)
        {
            unwrappedQueues[index] = ppPresentQueue[index];

            // Attempt to unwrap each instance to retrieve a pointer to the "real" runtime instance.
            ID3D12CommandQueue* presentQueue = nullptr;
            HRESULT unwrapped = (unwrappedQueues[index])->QueryInterface(IID_IWrappedObject, (void**)&presentQueue);

            if (unwrapped == S_OK)
            {
                unwrappedQueues[index] = (IUnknown*)(presentQueue);
            }
        }

        presentQueueArgs = unwrappedQueues;
    }

    void* pFn = HookSwapChainRelease.GetRealFunction(pSwapChain, IDXGISwapChain3_ResizeBuffers1_Func_Ordinal);
    HRESULT hRes = ((IDXGISwapChain_ResizeBuffers1_type)pFn)(pSwapChain, BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, presentQueueArgs);

    if (hRes != S_OK)
    {
        Log(logERROR, "Call to IDXGISwapChain3::ResizeBuffers1 failed.\n");
    }

    // If we had to unwrap the CommandQueues above, be sure to destroy the copy of the CommandQueue array.
    if (BufferCount > 0)
    {
        SAFE_DELETE_ARRAY(presentQueueArgs);
    }

    return hRes;
}

/// Helper function to hook the swapchain functions
/// \param pSwapChain Input swapchain
/// \return Number of times the swapchain has been hooked.
DWORD HookIDXGISwapChain(IDXGISwapChain* pSwapChain)
{
    //Log(logDEBUG, " DXGIServer: HookIDXGISwapChain (Uses VTable patching)\n" );

    LogTrace(traceENTER, "pSwapChain = 0x%p", pSwapChain);

    s_dwSwapChainHookedTimes++;

    if (HookSwapChainRelease.AddAndHookIfUnique(pSwapChain, true))
    {
        HookSwapChainRelease.AddVtableFunctionToPatch(pSwapChain, IDXGISwapChain_Present_Func_Ordinal, (ptrdiff_t*)Mine_IDXGISwapChain_Present, true);
        HookSwapChainRelease.AddVtableFunctionToPatch(pSwapChain, IDXGISwapChain1_Present1_Func_Ordinal, (ptrdiff_t*)Mine_IDXGISwapChain_Present1, true);
        HookSwapChainRelease.AddVtableFunctionToPatch(pSwapChain, IDXGISwapChain3_ResizeBuffers1_Func_Ordinal, (ptrdiff_t*)Mine_IDXGISwapChain_ResizeBuffers1, true);
    }

    HookSwapChainRelease.SetOnReleaseCallBack(DestroySwapchain);
    HookSwapChainRelease.Attach();

    LogTrace(traceEXIT, "");

    return s_dwSwapChainHookedTimes;
}

/// Helper function to un hook the swapchain functions
/// \return Number of times the swapchain has been hooked.
DWORD UnhookIDXGISwapChain()
{
    LogTrace(traceENTER, "");

    s_dwSwapChainHookedTimes--;

    HookSwapChainRelease.Detach();

    LogTrace(traceEXIT, "");

    return s_dwSwapChainHookedTimes;
}

