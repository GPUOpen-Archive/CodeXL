//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCDetourID3D11Device.h"
#include "DCDetourID3D11DeviceContext.h"
#include "DCDetourCreateDevice.h"
#include "DCDetourIDXGISwapChain.h"
#include "DCVtableOffsets.h"
#include "DCFuncDefs.h"
#include "DCGPAProfiler.h"

#include "D3Dcompiler.h"
#include "..\Common\Defs.h"
#include "..\Common\Logger.h"
#include "DCID3D11DeviceContext_wrapper.h"

#include <iostream>

extern DCGPAProfiler g_Profiler;

ULONG WINAPI Mine_ID3D11Device_Release(ID3D11Device* pObj);

static DCProfilerDevice11VTManager DetourID3D11Device((ptrdiff_t*)Mine_ID3D11Device_Release);

DCProfilerDevice11VTManager* GetID3D11DeviceVTableManager()
{
    return &DetourID3D11Device;
}

HRESULT WINAPI Mine_ID3D11Device_CreateUnorderedAccessView(ID3D11Device* pObj, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
{
    HRESULT hr = Real_ID3D11Device_CreateUnorderedAccessView(&DetourID3D11Device, pObj, pResource, pDesc, ppUAView);

    if (FAILED(hr))
    {
        return hr;
    }

    //create backup UAV
    ID3D11UnorderedAccessView* backUAV;
    backUAV = g_Profiler.GetContextManager().CreateBackupUAV(pResource, pDesc);

    if (backUAV)
    {
        Log(traceMESSAGE, "Backup UAV(0x%p) created", backUAV);
        g_Profiler.GetContextManager().AddtoUAVTable(*ppUAView, backUAV);
    }

    return hr;
}

// Detour create resources(buffer, tex1D, tex2D, tex3D), make a copy of the resources with D3D11_BIND_UNORDERED_ACCESS
HRESULT WINAPI Mine_ID3D11Device_CreateBuffer(ID3D11Device* pObj, const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer)
{
    HRESULT hr;

    if (pDesc == NULL)
    {
        // Client app problem, let DX runtime to notify the client
        return Real_ID3D11Device_CreateBuffer(&DetourID3D11Device, pObj, pDesc, pInitialData, ppBuffer);
    }

    if (pDesc->BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        ID3D11Buffer* backupBuffer;

        hr = Real_ID3D11Device_CreateBuffer(&DetourID3D11Device, pObj, pDesc, pInitialData, ppBuffer);

        if (FAILED(hr))  //client code bug, report to client
        {
            return hr;
        }

        if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
        {
            //don't need to make a copy
            return hr;
        }
        else if (pDesc->Usage == D3D11_USAGE_DYNAMIC)
        {
            //D3D11_USAGE_DYNAMIC ! Can't do GPU-WRITE
            return hr;
        }

        backupBuffer = g_Profiler.GetContextManager().CreateBackupBuffer(pDesc, pInitialData);

        if (backupBuffer)
        {
            Log(traceMESSAGE, "Backup buffer(0x%p) created", backupBuffer);
            g_Profiler.GetContextManager().AddBackupResource(*ppBuffer, backupBuffer);
        }
    }
    else
    {
        hr = Real_ID3D11Device_CreateBuffer(&DetourID3D11Device, pObj, pDesc, pInitialData, ppBuffer);
    }

    return hr;

}
HRESULT WINAPI Mine_ID3D11Device_CreateTexture1D(ID3D11Device* pObj, const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D)
{
    HRESULT hr;

    if (pDesc == NULL)
    {
        // Client app problem, let DX runtime to notify the client
        return Real_ID3D11Device_CreateTexture1D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture1D);
    }

    if (pDesc->BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        ID3D11Texture1D* backupTex;
        hr = Real_ID3D11Device_CreateTexture1D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture1D);

        if (FAILED(hr))  //client code bug, report to client
        {
            return hr;
        }

        if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
        {
            //don't need to make a copy
            return hr;
        }
        else if (pDesc->Usage == D3D11_USAGE_DYNAMIC)
        {
            //D3D11_USAGE_DYNAMIC ! Can't do GPU-WRITE
            return hr;
        }

        backupTex = g_Profiler.GetContextManager().CreateBackupTextur1D(pDesc, pInitialData);

        if (backupTex)
        {
            Log(traceMESSAGE, "Backup Texture1D(0x%p) created", backupTex);
            g_Profiler.GetContextManager().AddBackupResource(*ppTexture1D, backupTex);
        }
    }
    else
    {
        hr = Real_ID3D11Device_CreateTexture1D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture1D);
    }

    return hr;
}
HRESULT WINAPI Mine_ID3D11Device_CreateTexture2D(ID3D11Device* pObj, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
    HRESULT hr;

    if (pDesc == NULL)
    {
        // Client app problem, let DX runtime to notify the client
        return Real_ID3D11Device_CreateTexture2D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture2D);
    }

    if (pDesc->BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        ID3D11Texture2D* backupTex;
        hr = Real_ID3D11Device_CreateTexture2D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture2D);

        if (FAILED(hr))
        {
            return hr;
        }

        if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
        {
            //don't need to make a copy
            return hr;
        }
        else if (pDesc->Usage == D3D11_USAGE_DYNAMIC)
        {
            //D3D11_USAGE_DYNAMIC ! Can't do GPU-WRITE
            return hr;
        }

        backupTex = g_Profiler.GetContextManager().CreateBackupTextur2D(pDesc, pInitialData);

        if (backupTex)
        {
            Log(traceMESSAGE, "Backup Texture2D(0x%p) created", backupTex);
            g_Profiler.GetContextManager().AddBackupResource(*ppTexture2D, backupTex);
        }
    }
    else
    {
        hr = Real_ID3D11Device_CreateTexture2D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture2D);
    }

    return hr;
}
HRESULT WINAPI Mine_ID3D11Device_CreateTexture3D(ID3D11Device* pObj, const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D)
{
    HRESULT hr;

    if (pDesc == NULL)
    {
        // Client app problem, let DX runtime to notify the client
        return Real_ID3D11Device_CreateTexture3D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture3D);
    }

    if (pDesc->BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        ID3D11Texture3D* backupTex;
        hr = Real_ID3D11Device_CreateTexture3D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture3D);

        if (FAILED(hr))  //client code bug, report to client
        {
            return hr;
        }

        if (pDesc->Usage == D3D11_USAGE_IMMUTABLE)
        {
            //don't need to make a copy
            return hr;
        }
        else if (pDesc->Usage == D3D11_USAGE_DYNAMIC)
        {
            //D3D11_USAGE_DYNAMIC ! Can't do GPU-WRITE
            return hr;
        }

        backupTex = g_Profiler.GetContextManager().CreateBackupTextur3D(pDesc, pInitialData);

        if (backupTex)
        {
            Log(traceMESSAGE, "Backup Texture3D(0x%p) created", backupTex);
            g_Profiler.GetContextManager().AddBackupResource(*ppTexture3D, backupTex);
        }
    }
    else
    {
        hr = Real_ID3D11Device_CreateTexture3D(&DetourID3D11Device, pObj, pDesc, pInitialData, ppTexture3D);
    }

    return hr;
}

HRESULT WINAPI Mine_ID3D11Device_CreateComputeShader(ID3D11Device* pObj, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
{
    HRESULT hr;
    hr = Real_ID3D11Device_CreateComputeShader(&DetourID3D11Device, pObj, pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);

    if (SUCCEEDED(hr))
    {
        g_Profiler.GetKernelAssemblyManager().AddComputeShader(*ppComputeShader, pShaderBytecode, BytecodeLength);
    }



    return hr;
}

HRESULT WINAPI Mine_ID3D11Device_CreateDeferredContext(ID3D11Device* pObj,
                                                       UINT ContextFlags,
                                                       ID3D11DeviceContext** ppDeferredContext)
{
    HRESULT hr = Real_ID3D11Device_CreateDeferredContext(&DetourID3D11Device, pObj, ContextFlags, ppDeferredContext);

    if (SUCCEEDED(hr))
    {
        GetID3D11DeviceContextVTableManager()->Patch(ppDeferredContext, true);
        g_Profiler.GetCommandRecorder().AddDeferredDeviceContext(*ppDeferredContext);
    }

    return hr;
}

void WINAPI Mine_ID3D11Device_GetImmediateContext(ID3D11Device* pObj, ID3D11DeviceContext** ppImmediateContext)
{
    // DX Runtime returns the real ImmediateContext instead of wrapped one.
    // We need to return client wrapped one so that we can detour every API calls since we only patched ID3D11DeviceContext_wrapper
    Real_ID3D11Device_GetImmediateContext(&DetourID3D11Device, pObj, ppImmediateContext);

    if (ppImmediateContext && *ppImmediateContext)
    {
        *ppImmediateContext = GetWrappedDevice(*ppImmediateContext);
    }
    else
    {
        Log(logWARNING, "Client error - GetImmediateContext() failed\n");
    }
}

ULONG WINAPI Mine_ID3D11Device_Release(ID3D11Device* pObj)
{
    ULONG ret = DetourID3D11Device.RemoveAndDetach(pObj);

    if (ret > 1)
    {
        // We have unreleased backup resources
        // when ret = 1, it means only ImmediateContext is still alive
        UINT backupResxCount = g_Profiler.GetContextManager().GetNumBackupResourcesCreated();

        if ((ret - backupResxCount) <= 1 && (ret - backupResxCount) >= 0)
        {
            // cleanup the backup resources
            DetourID3D11Device.Detach();
            g_Profiler.GetContextManager().Cleanup();
            GetID3D11DeviceContextVTableManager()->UnpatchAndRemoveAll();
            return DetourID3D11Device.RemoveAndDetach(pObj);
        }
    }

    return ret;
}

DCProfilerDevice11VTManager::DCProfilerDevice11VTManager(ptrdiff_t* pFnMine) : DCID3D11DeviceVTManager(pFnMine)
{
    m_pMine_ID3D11Device_CreateBufferFn = Mine_ID3D11Device_CreateBuffer;
    m_pMine_ID3D11Device_CreateTexture1DFn = Mine_ID3D11Device_CreateTexture1D;
    m_pMine_ID3D11Device_CreateTexture2DFn = Mine_ID3D11Device_CreateTexture2D;
    m_pMine_ID3D11Device_CreateTexture3DFn = Mine_ID3D11Device_CreateTexture3D;
    m_pMine_ID3D11Device_CreateUnorderedAccessViewFn = Mine_ID3D11Device_CreateUnorderedAccessView;
    m_pMine_ID3D11Device_CreateComputeShaderFn = Mine_ID3D11Device_CreateComputeShader;
    m_pMine_ID3D11Device_CreateDeferredContextFn = Mine_ID3D11Device_CreateDeferredContext;
    m_pMine_ID3D11Device_GetImmediateContextFn = Mine_ID3D11Device_GetImmediateContext;
}
