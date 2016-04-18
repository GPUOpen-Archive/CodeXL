//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCDetourID3D11Device.h"
#include "DCDetourID3D11DeviceContext.h"
#include "DCVtableOffsets.h"
#include "DCFuncDefs.h"
#include "DCCommandDefs.h"
#include "DCUtils.h"
#include "DCGPAProfiler.h"
#include "..\Common\StringUtils.h"
#include "..\Common\Logger.h"


DCGPAProfiler g_Profiler;

using std::string;

ULONG WINAPI Mine_ID3D11DeviceContext_Release(IUnknown* pObj);

static DCProfilerDeviceContext11VTManager DetourID3D11DeviceContext((ptrdiff_t*)Mine_ID3D11DeviceContext_Release);

ULONG WINAPI Mine_ID3D11DeviceContext_Release(IUnknown* pObj)
{
    //return DetourID3D11DeviceContext.RemoveAndDetach(pObj);
    // NOTE: We don't unpatch device context here, we let Device.Release
    return DetourID3D11DeviceContext.CallRealRelease(pObj);
}

DCProfilerDeviceContext11VTManager* GetID3D11DeviceContextVTableManager()
{
    return &DetourID3D11DeviceContext;
}

/// Detoured function for ID3D11DeviceContext::Unmap()
/// \param pObj Parameter for ID3D11DeviceContext::Unmap()
/// \param pResource Parameter for ID3D11DeviceContext::Unmap()
/// \param Subresource Parameter for ID3D11DeviceContext::Unmap()
void WINAPI Mine_ID3D11DeviceContext_Unmap(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource)
{
    Real_ID3D11DeviceContext_Unmap(&DetourID3D11DeviceContext, pObj, pResource, Subresource);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_Unmap* pCMD = new DC_CMD_Unmap();
        DCCommandBuffer* pContext = g_Profiler.GetCommandRecorder().GetContext(pObj);
        pCMD->OnCreate(pObj, pResource, Subresource, pContext);
        g_Profiler.GetCommandRecorder().EndEnqueueBackupBuffer(pObj, pResource);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }

    //#if _DEBUG
    //   DWORD dwTID;
    //   dwTID = GetCurrentThreadId();
    //   LogTrace( traceMESSAGE, "[%s][%lu]Command: Unmap",(dcType == D3D11_DEVICE_CONTEXT_DEFERRED)?"Deferred":"Immediate", dwTID );
    //#endif
}

/// Detoured function for ID3D11DeviceContext::Map()
/// \param pObj Parameter for ID3D11DeviceContext::Map()
/// \param pResource Parameter for ID3D11DeviceContext::Map()
/// \param Subresource Parameter for ID3D11DeviceContext::Map()
/// \param MapType Parameter for ID3D11DeviceContext::Map()
/// \param MapFlags Parameter for ID3D11DeviceContext::Map()
/// \param pMappedResource Parameter for ID3D11DeviceContext::Map()
HRESULT WINAPI Mine_ID3D11DeviceContext_Map(ID3D11DeviceContext* pObj,
                                            ID3D11Resource*      pResource,
                                            UINT                 Subresource,
                                            D3D11_MAP            MapType,
                                            UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource)
{
    HRESULT hr;
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        hr = Real_ID3D11DeviceContext_Map(&DetourID3D11DeviceContext,  pObj, pResource, Subresource, MapType, MapFlags, pMappedResource);

        //Called from Deferred Device Context
        DC_CMD_Map* param = new DC_CMD_Map();
        DCCommandBuffer* pContext = g_Profiler.GetCommandRecorder().GetContext(pObj);
        param->OnCreate(pObj, pResource, Subresource, MapType, MapFlags, pMappedResource, pContext);
        g_Profiler.GetCommandRecorder().BeginEnqueueBackupBuffer(pObj, *pMappedResource);

        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, param);
        return hr;
    }
    else
    {
        g_Profiler.GetTimer()->Reset();
        g_Profiler.GetTimer()->Start();
        hr = Real_ID3D11DeviceContext_Map(&DetourID3D11DeviceContext,  pObj, pResource, Subresource, MapType, MapFlags, pMappedResource);

        if (FAILED(hr))
        {
            return hr;
        }

        // 2014/06/13
        // Comment out dump memory status to make DirectCokmpute output compatible with OpenCL output.
        // Leaving commented out to make it easy to revert this change.
        //float elapsedTime = g_Profiler.GetTimer()->GetElapsedTime() / 1000000.f; // convert to millisec
        //g_Profiler.DumpMemoryStats(pResource, elapsedTime, MapType, MapFlags);
    }

    return hr;
}

/// Detoured function for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param pObj Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param pUAVInitialCounts Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
void WINAPI Mine_ID3D11DeviceContext_CSSetUnorderedAccessViews(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
{
    Real_ID3D11DeviceContext_CSSetUnorderedAccessViews(&DetourID3D11DeviceContext, pObj, StartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CSSetUnorderedAccessViews* pCMD = new DC_CMD_CSSetUnorderedAccessViews();
        pCMD->OnCreate(pObj, StartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
    else
    {
        //Save UAV
        g_Profiler.GetContextManager().SaveUAV(StartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
    }
}

/// Detoured function for ID3D11DeviceContext::Dispatch()
/// \param pObj Parameter for ID3D11DeviceContext::Dispatch()
/// \param ThreadGroupCountX Parameter for ID3D11DeviceContext::Dispatch()
/// \param ThreadGroupCountY Parameter for ID3D11DeviceContext::Dispatch()
/// \param ThreadGroupCountZ Parameter for ID3D11DeviceContext::Dispatch()
static void WINAPI Mine_ID3D11DeviceContext_Dispatch(ID3D11DeviceContext* pObj, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        Real_ID3D11DeviceContext_Dispatch(&DetourID3D11DeviceContext, pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        DC_CMD_Dispatch* pCMD = new DC_CMD_Dispatch();
        pCMD->OnCreate(pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
        return;
    }

    if (!g_Profiler.Loaded())
    {
        Real_ID3D11DeviceContext_Dispatch(&DetourID3D11DeviceContext, pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        return;
    }

    bool status = false;
    status = g_Profiler.Open();

    if (!status)
    {
        Log(logERROR, "Mine_ID3D11DeviceContext_Dispatch() : g_Profiler.Open() Failed\n");
        Real_ID3D11DeviceContext_Dispatch(&DetourID3D11DeviceContext, pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        return;
    }

    status = g_Profiler.EnableCounters();

    if (!status)
    {
        Log(logERROR, "Mine_ID3D11DeviceContext_Dispatch() : EnableCounters() Failed\n");
        Real_ID3D11DeviceContext_Dispatch(&DetourID3D11DeviceContext, pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        return;
    }

    gpa_uint32 uSessionIDOut;
    status = g_Profiler.FullProfile(pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ, uSessionIDOut);

    if (status)
    {
        g_Profiler.DumpSession(uSessionIDOut, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    g_Profiler.Close();
}

/// Detoured function for ID3D11DeviceContext::CSSetShader()
/// \param pObj Parameter for ID3D11DeviceContext::CSSetShader()
/// \param pComputeShader Parameter for ID3D11DeviceContext::CSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::CSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::CSSetShader()
void WINAPI Mine_ID3D11DeviceContext_CSSetShader(ID3D11DeviceContext* pObj, ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    Real_ID3D11DeviceContext_CSSetShader(&DetourID3D11DeviceContext, pObj, pComputeShader, ppClassInstances, NumClassInstances);

    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CSSetShader* pCMD = new DC_CMD_CSSetShader();
        pCMD->OnCreate(pObj, pComputeShader, ppClassInstances, NumClassInstances);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
    else
    {
        g_Profiler.GetContextManager().SetCurrentComputeShader(pComputeShader);
    }
}

/// Detoured function for ID3D11DeviceContext::ExecuteCommandList()
/// \param pObj Parameter for ID3D11DeviceContext::ExecuteCommandList()
/// \param pCommandList Parameter for ID3D11DeviceContext::ExecuteCommandList()
/// \param RestoreContextState Parameter for ID3D11DeviceContext::ExecuteCommandList()
void WINAPI Mine_ID3D11DeviceContext_ExecuteCommandList(ID3D11DeviceContext* pObj,
                                                        ID3D11CommandList* pCommandList,
                                                        BOOL RestoreContextState)
{
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_IMMEDIATE)
    {
        // ignore Real_ID3D11DeviceContext_ExecuteCommandList
        // execute command from our own command list
        g_Profiler.GetCommandRecorder().ExecuteCommands(pCommandList, RestoreContextState);
    }

    //else ExecuteCommandList can't be called on deferred context, DX Runtime error, not our problem
}

/// Detoured function for ID3D11DeviceContext::FinishCommandList()
/// \param pObj Parameter for ID3D11DeviceContext::FinishCommandList()
/// \param RestoreDeferredContextState Parameter for ID3D11DeviceContext::FinishCommandList()
/// \param ppCommandList Parameter for ID3D11DeviceContext::FinishCommandList()
HRESULT WINAPI Mine_ID3D11DeviceContext_FinishCommandList(ID3D11DeviceContext* pObj, BOOL RestoreDeferredContextState, ID3D11CommandList** ppCommandList)
{
    HRESULT ret = Real_ID3D11DeviceContext_FinishCommandList(&DetourID3D11DeviceContext, pObj, RestoreDeferredContextState, ppCommandList);

    if (SUCCEEDED(ret))
    {
        g_Profiler.GetCommandRecorder().CreateFlattenedCommandList(pObj, *ppCommandList);
    }

    return ret;
}

/// Detoured function for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_VSSetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    Real_ID3D11DeviceContext_VSSetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_VSSetConstantBuffers* pCMD = new DC_CMD_VSSetConstantBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppConstantBuffers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }

    //#if _DEBUG
    //   DWORD dwTID;
    //   dwTID = GetCurrentThreadId();
    //   LogTrace( traceMESSAGE, "[%s][%lu]Command: VSSetConstantBuffers",(dcType == D3D11_DEVICE_CONTEXT_DEFERRED)?"Deferred":"Immediate", dwTID );
    //#endif
}

/// Detoured function for ID3D11DeviceContext::PSSetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::PSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::PSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::PSSetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_PSSetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_PSSetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_PSSetShaderResources* pCMD = new DC_CMD_PSSetShaderResources();
        pCMD->OnCreate(pObj, StartSlot, NumViews, ppShaderResourceViews);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::PSSetShader()
/// \param pObj Parameter for ID3D11DeviceContext::PSSetShader()
/// \param pPixelShader Parameter for ID3D11DeviceContext::PSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::PSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::PSSetShader()
void WINAPI Mine_ID3D11DeviceContext_PSSetShader(ID3D11DeviceContext* pObj, ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    Real_ID3D11DeviceContext_PSSetShader(&DetourID3D11DeviceContext, pObj, pPixelShader, ppClassInstances, NumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_PSSetShader* pCMD = new DC_CMD_PSSetShader();
        pCMD->OnCreate(pObj, pPixelShader, ppClassInstances, NumClassInstances);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }

    //#if _DEBUG
    //   DWORD dwTID;
    //   dwTID = GetCurrentThreadId();
    //   LogTrace( traceMESSAGE, "[%s][%lu]Command: PSSetShader",(dcType == D3D11_DEVICE_CONTEXT_DEFERRED)?"Deferred":"Immediate", dwTID );
    //#endif
}

/// Detoured function for ID3D11DeviceContext::PSSetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::PSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::PSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::PSSetSamplers()
void WINAPI Mine_ID3D11DeviceContext_PSSetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    Real_ID3D11DeviceContext_PSSetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_PSSetSamplers* pCMD = new DC_CMD_PSSetSamplers();
        pCMD->OnCreate(pObj, StartSlot, NumSamplers, ppSamplers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::VSSetShader()
/// \param pObj Parameter for ID3D11DeviceContext::VSSetShader()
/// \param pVertexShader Parameter for ID3D11DeviceContext::VSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::VSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::VSSetShader()
void WINAPI Mine_ID3D11DeviceContext_VSSetShader(ID3D11DeviceContext* pObj, ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    Real_ID3D11DeviceContext_VSSetShader(&DetourID3D11DeviceContext, pObj, pVertexShader, ppClassInstances, NumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_VSSetShader* pCMD = new DC_CMD_VSSetShader();
        pCMD->OnCreate(pObj, pVertexShader, ppClassInstances, NumClassInstances);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DrawIndexed()
/// \param pObj Parameter for ID3D11DeviceContext::DrawIndexed()
/// \param IndexCount Parameter for ID3D11DeviceContext::DrawIndexed()
/// \param StartIndexLocation Parameter for ID3D11DeviceContext::DrawIndexed()
/// \param BaseVertexLocation Parameter for ID3D11DeviceContext::DrawIndexed()
void WINAPI Mine_ID3D11DeviceContext_DrawIndexed(ID3D11DeviceContext* pObj, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
    Real_ID3D11DeviceContext_DrawIndexed(&DetourID3D11DeviceContext, pObj, IndexCount, StartIndexLocation, BaseVertexLocation);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DrawIndexed* pCMD = new DC_CMD_DrawIndexed();
        pCMD->OnCreate(pObj, IndexCount, StartIndexLocation, BaseVertexLocation);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::Draw()
/// \param pObj Parameter for ID3D11DeviceContext::Draw()
/// \param VertexCount Parameter for ID3D11DeviceContext::Draw()
/// \param StartVertexLocation Parameter for ID3D11DeviceContext::Draw()
void WINAPI Mine_ID3D11DeviceContext_Draw(ID3D11DeviceContext* pObj, UINT VertexCount, UINT StartVertexLocation)
{
    Real_ID3D11DeviceContext_Draw(&DetourID3D11DeviceContext, pObj, VertexCount, StartVertexLocation);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_Draw* pCMD = new DC_CMD_Draw();
        pCMD->OnCreate(pObj, VertexCount, StartVertexLocation);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_PSSetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    Real_ID3D11DeviceContext_PSSetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_PSSetConstantBuffers* pCMD = new DC_CMD_PSSetConstantBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppConstantBuffers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::IASetInputLayout()
/// \param pObj Parameter for ID3D11DeviceContext::IASetInputLayout()
/// \param pInputLayout Parameter for ID3D11DeviceContext::IASetInputLayout()
void WINAPI Mine_ID3D11DeviceContext_IASetInputLayout(ID3D11DeviceContext* pObj, ID3D11InputLayout* pInputLayout)
{
    Real_ID3D11DeviceContext_IASetInputLayout(&DetourID3D11DeviceContext, pObj, pInputLayout);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_IASetInputLayout* pCMD = new DC_CMD_IASetInputLayout();
        pCMD->OnCreate(pObj, pInputLayout);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::IASetVertexBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param ppVertexBuffers Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param pStrides Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param pOffsets Parameter for ID3D11DeviceContext::IASetVertexBuffers()
void WINAPI Mine_ID3D11DeviceContext_IASetVertexBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets)
{
    Real_ID3D11DeviceContext_IASetVertexBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_IASetVertexBuffers* pCMD = new DC_CMD_IASetVertexBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::IASetIndexBuffer()
/// \param pObj Parameter for ID3D11DeviceContext::IASetIndexBuffer()
/// \param pIndexBuffer Parameter for ID3D11DeviceContext::IASetIndexBuffer()
/// \param Format Parameter for ID3D11DeviceContext::IASetIndexBuffer()
/// \param Offset Parameter for ID3D11DeviceContext::IASetIndexBuffer()
void WINAPI Mine_ID3D11DeviceContext_IASetIndexBuffer(ID3D11DeviceContext* pObj, ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
{
    Real_ID3D11DeviceContext_IASetIndexBuffer(&DetourID3D11DeviceContext, pObj, pIndexBuffer, Format, Offset);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_IASetIndexBuffer* pCMD = new DC_CMD_IASetIndexBuffer();
        pCMD->OnCreate(pObj, pIndexBuffer, Format, Offset);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }

    //else
    //{
    //   LogTrace( traceMESSAGE, "[Immediate]Command: IASetIndexBuffer" );
    //}
}

/// Detoured function for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param pObj Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param IndexCountPerInstance Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param InstanceCount Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param StartIndexLocation Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param BaseVertexLocation Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param StartInstanceLocation Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
void WINAPI Mine_ID3D11DeviceContext_DrawIndexedInstanced(ID3D11DeviceContext* pObj, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
    Real_ID3D11DeviceContext_DrawIndexedInstanced(&DetourID3D11DeviceContext, pObj, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DrawIndexedInstanced* pCMD = new DC_CMD_DrawIndexedInstanced();
        pCMD->OnCreate(pObj, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DrawInstanced()
/// \param pObj Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param VertexCountPerInstance Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param InstanceCount Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param StartVertexLocation Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param StartInstanceLocation Parameter for ID3D11DeviceContext::DrawInstanced()
void WINAPI Mine_ID3D11DeviceContext_DrawInstanced(ID3D11DeviceContext* pObj, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    Real_ID3D11DeviceContext_DrawInstanced(&DetourID3D11DeviceContext, pObj, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DrawInstanced* pCMD = new DC_CMD_DrawInstanced();
        pCMD->OnCreate(pObj, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_GSSetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    Real_ID3D11DeviceContext_GSSetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_GSSetConstantBuffers* pCMD = new DC_CMD_GSSetConstantBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppConstantBuffers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GSSetShader()
/// \param pObj Parameter for ID3D11DeviceContext::GSSetShader()
/// \param pShader Parameter for ID3D11DeviceContext::GSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::GSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::GSSetShader()
void WINAPI Mine_ID3D11DeviceContext_GSSetShader(ID3D11DeviceContext* pObj, ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    Real_ID3D11DeviceContext_GSSetShader(&DetourID3D11DeviceContext, pObj, pShader, ppClassInstances, NumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_GSSetShader* pCMD = new DC_CMD_GSSetShader();
        pCMD->OnCreate(pObj, pShader, ppClassInstances, NumClassInstances);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::IASetPrimitiveTopology()
/// \param pObj Parameter for ID3D11DeviceContext::IASetPrimitiveTopology()
/// \param Topology Parameter for ID3D11DeviceContext::IASetPrimitiveTopology()
void WINAPI Mine_ID3D11DeviceContext_IASetPrimitiveTopology(ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY Topology)
{
    Real_ID3D11DeviceContext_IASetPrimitiveTopology(&DetourID3D11DeviceContext, pObj, Topology);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_IASetPrimitiveTopology* pCMD = new DC_CMD_IASetPrimitiveTopology();
        pCMD->OnCreate(pObj, Topology);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::VSSetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::VSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::VSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::VSSetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_VSSetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_VSSetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_VSSetShaderResources* pCMD = new DC_CMD_VSSetShaderResources();
        pCMD->OnCreate(pObj, StartSlot, NumViews, ppShaderResourceViews);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::VSSetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::VSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::VSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::VSSetSamplers()
void WINAPI Mine_ID3D11DeviceContext_VSSetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    Real_ID3D11DeviceContext_VSSetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_VSSetSamplers* pCMD = new DC_CMD_VSSetSamplers();
        pCMD->OnCreate(pObj, StartSlot, NumSamplers, ppSamplers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::Begin()
/// \param pObj Parameter for ID3D11DeviceContext::Begin()
/// \param pAsync Parameter for ID3D11DeviceContext::Begin()
void WINAPI Mine_ID3D11DeviceContext_Begin(ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync)
{
    Real_ID3D11DeviceContext_Begin(&DetourID3D11DeviceContext, pObj, pAsync);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_Begin* pCMD = new DC_CMD_Begin();
        pCMD->OnCreate(pObj, pAsync);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::End()
/// \param pObj Parameter for ID3D11DeviceContext::End()
/// \param pAsync Parameter for ID3D11DeviceContext::End()
void WINAPI Mine_ID3D11DeviceContext_End(ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync)
{
    Real_ID3D11DeviceContext_End(&DetourID3D11DeviceContext, pObj, pAsync);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_End* pCMD = new DC_CMD_End();
        pCMD->OnCreate(pObj, pAsync);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GetData()
/// \param pObj Parameter for ID3D11DeviceContext::GetData()
/// \param pAsync Parameter for ID3D11DeviceContext::GetData()
/// \param pData Parameter for ID3D11DeviceContext::GetData()
/// \param DataSize Parameter for ID3D11DeviceContext::GetData()
/// \param GetDataFlags Parameter for ID3D11DeviceContext::GetData()
HRESULT WINAPI Mine_ID3D11DeviceContext_GetData(ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags)
{
    HRESULT ret = Real_ID3D11DeviceContext_GetData(&DetourID3D11DeviceContext, pObj, pAsync, pData, DataSize, GetDataFlags);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }

    return ret;
}

/// Detoured function for ID3D11DeviceContext::SetPredication()
/// \param pObj Parameter for ID3D11DeviceContext::SetPredication()
/// \param pPredicate Parameter for ID3D11DeviceContext::SetPredication()
/// \param PredicateValue Parameter for ID3D11DeviceContext::SetPredication()
void WINAPI Mine_ID3D11DeviceContext_SetPredication(ID3D11DeviceContext* pObj, ID3D11Predicate* pPredicate, BOOL PredicateValue)
{
    Real_ID3D11DeviceContext_SetPredication(&DetourID3D11DeviceContext, pObj, pPredicate, PredicateValue);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_SetPredication* pCMD = new DC_CMD_SetPredication();
        pCMD->OnCreate(pObj, pPredicate, PredicateValue);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GSSetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::GSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::GSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::GSSetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_GSSetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_GSSetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_GSSetShaderResources* pCMD = new DC_CMD_GSSetShaderResources();
        pCMD->OnCreate(pObj, StartSlot, NumViews, ppShaderResourceViews);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GSSetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::GSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::GSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::GSSetSamplers()
void WINAPI Mine_ID3D11DeviceContext_GSSetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    Real_ID3D11DeviceContext_GSSetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_GSSetSamplers* pCMD = new DC_CMD_GSSetSamplers();
        pCMD->OnCreate(pObj, StartSlot, NumSamplers, ppSamplers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::OMSetRenderTargets()
/// \param pObj Parameter for ID3D11DeviceContext::OMSetRenderTargets()
/// \param NumViews Parameter for ID3D11DeviceContext::OMSetRenderTargets()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMSetRenderTargets()
/// \param pDepthStencilView Parameter for ID3D11DeviceContext::OMSetRenderTargets()
void WINAPI Mine_ID3D11DeviceContext_OMSetRenderTargets(ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView)
{
    Real_ID3D11DeviceContext_OMSetRenderTargets(&DetourID3D11DeviceContext, pObj, NumViews, ppRenderTargetViews, pDepthStencilView);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_OMSetRenderTargets* pCMD = new DC_CMD_OMSetRenderTargets();
        pCMD->OnCreate(pObj, NumViews, ppRenderTargetViews, pDepthStencilView);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param pObj Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param NumRTVs Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param pDepthStencilView Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param UAVStartSlot Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param pUAVInitialCounts Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
void WINAPI Mine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
{
    Real_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews(&DetourID3D11DeviceContext, pObj, NumRTVs, ppRenderTargetViews, pDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews* pCMD = new DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews();
        pCMD->OnCreate(pObj, NumRTVs, ppRenderTargetViews, pDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::OMSetBlendState()
/// \param pObj Parameter for ID3D11DeviceContext::OMSetBlendState()
/// \param pBlendState Parameter for ID3D11DeviceContext::OMSetBlendState()
/// \param BlendFactor Parameter for ID3D11DeviceContext::OMSetBlendState()
/// \param SampleMask Parameter for ID3D11DeviceContext::OMSetBlendState()
void WINAPI Mine_ID3D11DeviceContext_OMSetBlendState(ID3D11DeviceContext* pObj, ID3D11BlendState* pBlendState, const FLOAT BlendFactor[ 4 ], UINT SampleMask)
{
    Real_ID3D11DeviceContext_OMSetBlendState(&DetourID3D11DeviceContext, pObj, pBlendState, BlendFactor, SampleMask);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_OMSetBlendState* pCMD = new DC_CMD_OMSetBlendState();
        pCMD->OnCreate(pObj, pBlendState, BlendFactor, SampleMask);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::OMSetDepthStencilState()
/// \param pObj Parameter for ID3D11DeviceContext::OMSetDepthStencilState()
/// \param pDepthStencilState Parameter for ID3D11DeviceContext::OMSetDepthStencilState()
/// \param StencilRef Parameter for ID3D11DeviceContext::OMSetDepthStencilState()
void WINAPI Mine_ID3D11DeviceContext_OMSetDepthStencilState(ID3D11DeviceContext* pObj, ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef)
{
    Real_ID3D11DeviceContext_OMSetDepthStencilState(&DetourID3D11DeviceContext, pObj, pDepthStencilState, StencilRef);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_OMSetDepthStencilState* pCMD = new DC_CMD_OMSetDepthStencilState();
        pCMD->OnCreate(pObj, pDepthStencilState, StencilRef);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::SOSetTargets()
/// \param pObj Parameter for ID3D11DeviceContext::SOSetTargets()
/// \param NumBuffers Parameter for ID3D11DeviceContext::SOSetTargets()
/// \param ppSOTargets Parameter for ID3D11DeviceContext::SOSetTargets()
/// \param pOffsets Parameter for ID3D11DeviceContext::SOSetTargets()
void WINAPI Mine_ID3D11DeviceContext_SOSetTargets(ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets)
{
    Real_ID3D11DeviceContext_SOSetTargets(&DetourID3D11DeviceContext, pObj, NumBuffers, ppSOTargets, pOffsets);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_SOSetTargets* pCMD = new DC_CMD_SOSetTargets();
        pCMD->OnCreate(pObj, NumBuffers, ppSOTargets, pOffsets);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DrawAuto()
/// \param pObj Parameter for ID3D11DeviceContext::DrawAuto()
void WINAPI Mine_ID3D11DeviceContext_DrawAuto(ID3D11DeviceContext* pObj)
{
    Real_ID3D11DeviceContext_DrawAuto(&DetourID3D11DeviceContext, pObj);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DrawAuto* pCMD = new DC_CMD_DrawAuto();
        pCMD->OnCreate(pObj);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
/// \param pObj Parameter for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
/// \param pBufferForArgs Parameter for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
/// \param AlignedByteOffsetForArgs Parameter for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
void WINAPI Mine_ID3D11DeviceContext_DrawIndexedInstancedIndirect(ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
    Real_ID3D11DeviceContext_DrawIndexedInstancedIndirect(&DetourID3D11DeviceContext, pObj, pBufferForArgs, AlignedByteOffsetForArgs);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DrawIndexedInstancedIndirect* pCMD = new DC_CMD_DrawIndexedInstancedIndirect();
        pCMD->OnCreate(pObj, pBufferForArgs, AlignedByteOffsetForArgs);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DrawInstancedIndirect()
/// \param pObj Parameter for ID3D11DeviceContext::DrawInstancedIndirect()
/// \param pBufferForArgs Parameter for ID3D11DeviceContext::DrawInstancedIndirect()
/// \param AlignedByteOffsetForArgs Parameter for ID3D11DeviceContext::DrawInstancedIndirect()
void WINAPI Mine_ID3D11DeviceContext_DrawInstancedIndirect(ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
    Real_ID3D11DeviceContext_DrawInstancedIndirect(&DetourID3D11DeviceContext, pObj, pBufferForArgs, AlignedByteOffsetForArgs);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DrawInstancedIndirect* pCMD = new DC_CMD_DrawInstancedIndirect();
        pCMD->OnCreate(pObj, pBufferForArgs, AlignedByteOffsetForArgs);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DispatchIndirect()
/// \param pObj Parameter for ID3D11DeviceContext::DispatchIndirect()
/// \param pBufferForArgs Parameter for ID3D11DeviceContext::DispatchIndirect()
/// \param AlignedByteOffsetForArgs Parameter for ID3D11DeviceContext::DispatchIndirect()
void WINAPI Mine_ID3D11DeviceContext_DispatchIndirect(ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
    Real_ID3D11DeviceContext_DispatchIndirect(&DetourID3D11DeviceContext, pObj, pBufferForArgs, AlignedByteOffsetForArgs);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DispatchIndirect* pCMD = new DC_CMD_DispatchIndirect();
        pCMD->OnCreate(pObj, pBufferForArgs, AlignedByteOffsetForArgs);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::RSSetState()
/// \param pObj Parameter for ID3D11DeviceContext::RSSetState()
/// \param pRasterizerState Parameter for ID3D11DeviceContext::RSSetState()
void WINAPI Mine_ID3D11DeviceContext_RSSetState(ID3D11DeviceContext* pObj, ID3D11RasterizerState* pRasterizerState)
{
    Real_ID3D11DeviceContext_RSSetState(&DetourID3D11DeviceContext, pObj, pRasterizerState);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_RSSetState* pCMD = new DC_CMD_RSSetState();
        pCMD->OnCreate(pObj, pRasterizerState);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::RSSetViewports()
/// \param pObj Parameter for ID3D11DeviceContext::RSSetViewports()
/// \param NumViewports Parameter for ID3D11DeviceContext::RSSetViewports()
/// \param pViewports Parameter for ID3D11DeviceContext::RSSetViewports()
void WINAPI Mine_ID3D11DeviceContext_RSSetViewports(ID3D11DeviceContext* pObj, UINT NumViewports, const D3D11_VIEWPORT* pViewports)
{
    Real_ID3D11DeviceContext_RSSetViewports(&DetourID3D11DeviceContext, pObj, NumViewports, pViewports);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_RSSetViewports* pCMD = new DC_CMD_RSSetViewports();
        pCMD->OnCreate(pObj, NumViewports, pViewports);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::RSSetScissorRects()
/// \param pObj Parameter for ID3D11DeviceContext::RSSetScissorRects()
/// \param NumRects Parameter for ID3D11DeviceContext::RSSetScissorRects()
/// \param pRects Parameter for ID3D11DeviceContext::RSSetScissorRects()
void WINAPI Mine_ID3D11DeviceContext_RSSetScissorRects(ID3D11DeviceContext* pObj, UINT NumRects, const D3D11_RECT* pRects)
{
    Real_ID3D11DeviceContext_RSSetScissorRects(&DetourID3D11DeviceContext, pObj, NumRects, pRects);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_RSSetScissorRects* pCMD = new DC_CMD_RSSetScissorRects();
        pCMD->OnCreate(pObj, NumRects, pRects);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pObj Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pDstResource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstSubresource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstX Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstY Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstZ Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pSrcResource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param SrcSubresource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pSrcBox Parameter for ID3D11DeviceContext::CopySubresourceRegion()
void WINAPI Mine_ID3D11DeviceContext_CopySubresourceRegion(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox)
{
    Real_ID3D11DeviceContext_CopySubresourceRegion(&DetourID3D11DeviceContext, pObj, pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CopySubresourceRegion* pCMD = new DC_CMD_CopySubresourceRegion();
        pCMD->OnCreate(pObj, pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::CopyResource()
/// \param pObj Parameter for ID3D11DeviceContext::CopyResource()
/// \param pDstResource Parameter for ID3D11DeviceContext::CopyResource()
/// \param pSrcResource Parameter for ID3D11DeviceContext::CopyResource()
void WINAPI Mine_ID3D11DeviceContext_CopyResource(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource)
{
    Real_ID3D11DeviceContext_CopyResource(&DetourID3D11DeviceContext, pObj, pDstResource, pSrcResource);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CopyResource* pCMD = new DC_CMD_CopyResource();
        pCMD->OnCreate(pObj, pDstResource, pSrcResource);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::UpdateSubresource()
/// \param pObj Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param pDstResource Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param DstSubresource Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param pDstBox Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param pSrcData Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param SrcRowPitch Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param SrcDepthPitch Parameter for ID3D11DeviceContext::UpdateSubresource()
void WINAPI Mine_ID3D11DeviceContext_UpdateSubresource(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
{
    Real_ID3D11DeviceContext_UpdateSubresource(&DetourID3D11DeviceContext, pObj, pDstResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_UpdateSubresource* pCMD = new DC_CMD_UpdateSubresource();
        pCMD->OnCreate(pObj, pDstResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::CopyStructureCount()
/// \param pObj Parameter for ID3D11DeviceContext::CopyStructureCount()
/// \param pDstBuffer Parameter for ID3D11DeviceContext::CopyStructureCount()
/// \param DstAlignedByteOffset Parameter for ID3D11DeviceContext::CopyStructureCount()
/// \param pSrcView Parameter for ID3D11DeviceContext::CopyStructureCount()
void WINAPI Mine_ID3D11DeviceContext_CopyStructureCount(ID3D11DeviceContext* pObj, ID3D11Buffer* pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView* pSrcView)
{
    Real_ID3D11DeviceContext_CopyStructureCount(&DetourID3D11DeviceContext, pObj, pDstBuffer, DstAlignedByteOffset, pSrcView);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CopyStructureCount* pCMD = new DC_CMD_CopyStructureCount();
        pCMD->OnCreate(pObj, pDstBuffer, DstAlignedByteOffset, pSrcView);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::ClearRenderTargetView()
/// \param pObj Parameter for ID3D11DeviceContext::ClearRenderTargetView()
/// \param pRenderTargetView Parameter for ID3D11DeviceContext::ClearRenderTargetView()
/// \param ColorRGBA Parameter for ID3D11DeviceContext::ClearRenderTargetView()
void WINAPI Mine_ID3D11DeviceContext_ClearRenderTargetView(ID3D11DeviceContext* pObj, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[ 4 ])
{
    Real_ID3D11DeviceContext_ClearRenderTargetView(&DetourID3D11DeviceContext, pObj, pRenderTargetView, ColorRGBA);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_ClearRenderTargetView* pCMD = new DC_CMD_ClearRenderTargetView();
        pCMD->OnCreate(pObj, pRenderTargetView, ColorRGBA);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
/// \param pObj Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
/// \param pUnorderedAccessView Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
/// \param Values Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
void WINAPI Mine_ID3D11DeviceContext_ClearUnorderedAccessViewUint(ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[ 4 ])
{
    Real_ID3D11DeviceContext_ClearUnorderedAccessViewUint(&DetourID3D11DeviceContext, pObj, pUnorderedAccessView, Values);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_ClearUnorderedAccessViewUint* pCMD = new DC_CMD_ClearUnorderedAccessViewUint();
        pCMD->OnCreate(pObj, pUnorderedAccessView, Values);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
/// \param pObj Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
/// \param pUnorderedAccessView Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
/// \param Values Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
void WINAPI Mine_ID3D11DeviceContext_ClearUnorderedAccessViewFloat(ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const FLOAT Values[ 4 ])
{
    Real_ID3D11DeviceContext_ClearUnorderedAccessViewFloat(&DetourID3D11DeviceContext, pObj, pUnorderedAccessView, Values);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_ClearUnorderedAccessViewFloat* pCMD = new DC_CMD_ClearUnorderedAccessViewFloat();
        pCMD->OnCreate(pObj, pUnorderedAccessView, Values);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::ClearDepthStencilView()
/// \param pObj Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param pDepthStencilView Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param ClearFlags Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param Depth Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param Stencil Parameter for ID3D11DeviceContext::ClearDepthStencilView()
void WINAPI Mine_ID3D11DeviceContext_ClearDepthStencilView(ID3D11DeviceContext* pObj, ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
{
    Real_ID3D11DeviceContext_ClearDepthStencilView(&DetourID3D11DeviceContext, pObj, pDepthStencilView, ClearFlags, Depth, Stencil);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_ClearDepthStencilView* pCMD = new DC_CMD_ClearDepthStencilView();
        pCMD->OnCreate(pObj, pDepthStencilView, ClearFlags, Depth, Stencil);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GenerateMips()
/// \param pObj Parameter for ID3D11DeviceContext::GenerateMips()
/// \param pShaderResourceView Parameter for ID3D11DeviceContext::GenerateMips()
void WINAPI Mine_ID3D11DeviceContext_GenerateMips(ID3D11DeviceContext* pObj, ID3D11ShaderResourceView* pShaderResourceView)
{
    Real_ID3D11DeviceContext_GenerateMips(&DetourID3D11DeviceContext, pObj, pShaderResourceView);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_GenerateMips* pCMD = new DC_CMD_GenerateMips();
        pCMD->OnCreate(pObj, pShaderResourceView);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::SetResourceMinLOD()
/// \param pObj Parameter for ID3D11DeviceContext::SetResourceMinLOD()
/// \param pResource Parameter for ID3D11DeviceContext::SetResourceMinLOD()
/// \param MinLOD Parameter for ID3D11DeviceContext::SetResourceMinLOD()
void WINAPI Mine_ID3D11DeviceContext_SetResourceMinLOD(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, FLOAT MinLOD)
{
    Real_ID3D11DeviceContext_SetResourceMinLOD(&DetourID3D11DeviceContext, pObj, pResource, MinLOD);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_SetResourceMinLOD* pCMD = new DC_CMD_SetResourceMinLOD();
        pCMD->OnCreate(pObj, pResource, MinLOD);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GetResourceMinLOD()
/// \param pObj Parameter for ID3D11DeviceContext::GetResourceMinLOD()
/// \param pResource Parameter for ID3D11DeviceContext::GetResourceMinLOD()
FLOAT WINAPI Mine_ID3D11DeviceContext_GetResourceMinLOD(ID3D11DeviceContext* pObj, ID3D11Resource* pResource)
{
    FLOAT ret = Real_ID3D11DeviceContext_GetResourceMinLOD(&DetourID3D11DeviceContext, pObj, pResource);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }

    return ret;
}

/// Detoured function for ID3D11DeviceContext::ResolveSubresource()
/// \param pObj Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param pDstResource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param DstSubresource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param pSrcResource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param SrcSubresource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param Format Parameter for ID3D11DeviceContext::ResolveSubresource()
void WINAPI Mine_ID3D11DeviceContext_ResolveSubresource(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
{
    Real_ID3D11DeviceContext_ResolveSubresource(&DetourID3D11DeviceContext, pObj, pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_ResolveSubresource* pCMD = new DC_CMD_ResolveSubresource();
        pCMD->OnCreate(pObj, pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::HSSetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::HSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::HSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::HSSetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_HSSetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_HSSetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_HSSetShaderResources* pCMD = new DC_CMD_HSSetShaderResources();
        pCMD->OnCreate(pObj, StartSlot, NumViews, ppShaderResourceViews);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::HSSetShader()
/// \param pObj Parameter for ID3D11DeviceContext::HSSetShader()
/// \param pHullShader Parameter for ID3D11DeviceContext::HSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::HSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::HSSetShader()
void WINAPI Mine_ID3D11DeviceContext_HSSetShader(ID3D11DeviceContext* pObj, ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    Real_ID3D11DeviceContext_HSSetShader(&DetourID3D11DeviceContext, pObj, pHullShader, ppClassInstances, NumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_HSSetShader* pCMD = new DC_CMD_HSSetShader();
        pCMD->OnCreate(pObj, pHullShader, ppClassInstances, NumClassInstances);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::HSSetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::HSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::HSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::HSSetSamplers()
void WINAPI Mine_ID3D11DeviceContext_HSSetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    Real_ID3D11DeviceContext_HSSetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_HSSetSamplers* pCMD = new DC_CMD_HSSetSamplers();
        pCMD->OnCreate(pObj, StartSlot, NumSamplers, ppSamplers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_HSSetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    Real_ID3D11DeviceContext_HSSetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_HSSetConstantBuffers* pCMD = new DC_CMD_HSSetConstantBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppConstantBuffers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DSSetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::DSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::DSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::DSSetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_DSSetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_DSSetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DSSetShaderResources* pCMD = new DC_CMD_DSSetShaderResources();
        pCMD->OnCreate(pObj, StartSlot, NumViews, ppShaderResourceViews);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DSSetShader()
/// \param pObj Parameter for ID3D11DeviceContext::DSSetShader()
/// \param pDomainShader Parameter for ID3D11DeviceContext::DSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::DSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::DSSetShader()
void WINAPI Mine_ID3D11DeviceContext_DSSetShader(ID3D11DeviceContext* pObj, ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    Real_ID3D11DeviceContext_DSSetShader(&DetourID3D11DeviceContext, pObj, pDomainShader, ppClassInstances, NumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DSSetShader* pCMD = new DC_CMD_DSSetShader();
        pCMD->OnCreate(pObj, pDomainShader, ppClassInstances, NumClassInstances);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DSSetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::DSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::DSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::DSSetSamplers()
void WINAPI Mine_ID3D11DeviceContext_DSSetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    Real_ID3D11DeviceContext_DSSetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DSSetSamplers* pCMD = new DC_CMD_DSSetSamplers();
        pCMD->OnCreate(pObj, StartSlot, NumSamplers, ppSamplers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_DSSetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    Real_ID3D11DeviceContext_DSSetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_DSSetConstantBuffers* pCMD = new DC_CMD_DSSetConstantBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppConstantBuffers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::CSSetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::CSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::CSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::CSSetSamplers()
void WINAPI Mine_ID3D11DeviceContext_CSSetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    Real_ID3D11DeviceContext_CSSetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CSSetSamplers* pCMD = new DC_CMD_CSSetSamplers();
        pCMD->OnCreate(pObj, StartSlot, NumSamplers, ppSamplers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_CSSetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    Real_ID3D11DeviceContext_CSSetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CSSetConstantBuffers* pCMD = new DC_CMD_CSSetConstantBuffers();
        pCMD->OnCreate(pObj, StartSlot, NumBuffers, ppConstantBuffers);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::CSSetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::CSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::CSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::CSSetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_CSSetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_CSSetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_CSSetShaderResources* pCMD = new DC_CMD_CSSetShaderResources();
        pCMD->OnCreate(pObj, StartSlot, NumViews, ppShaderResourceViews);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_VSGetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    Real_ID3D11DeviceContext_VSGetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::PSGetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::PSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::PSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::PSGetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_PSGetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_PSGetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::PSGetShader()
/// \param pObj Parameter for ID3D11DeviceContext::PSGetShader()
/// \param ppPixelShader Parameter for ID3D11DeviceContext::PSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::PSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::PSGetShader()
void WINAPI Mine_ID3D11DeviceContext_PSGetShader(ID3D11DeviceContext* pObj, ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    Real_ID3D11DeviceContext_PSGetShader(&DetourID3D11DeviceContext, pObj, ppPixelShader, ppClassInstances, pNumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::PSGetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::PSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::PSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::PSGetSamplers()
void WINAPI Mine_ID3D11DeviceContext_PSGetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    Real_ID3D11DeviceContext_PSGetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::VSGetShader()
/// \param pObj Parameter for ID3D11DeviceContext::VSGetShader()
/// \param ppVertexShader Parameter for ID3D11DeviceContext::VSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::VSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::VSGetShader()
void WINAPI Mine_ID3D11DeviceContext_VSGetShader(ID3D11DeviceContext* pObj, ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    Real_ID3D11DeviceContext_VSGetShader(&DetourID3D11DeviceContext, pObj, ppVertexShader, ppClassInstances, pNumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_PSGetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    Real_ID3D11DeviceContext_PSGetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::IAGetInputLayout()
/// \param pObj Parameter for ID3D11DeviceContext::IAGetInputLayout()
/// \param ppInputLayout Parameter for ID3D11DeviceContext::IAGetInputLayout()
void WINAPI Mine_ID3D11DeviceContext_IAGetInputLayout(ID3D11DeviceContext* pObj, ID3D11InputLayout** ppInputLayout)
{
    Real_ID3D11DeviceContext_IAGetInputLayout(&DetourID3D11DeviceContext, pObj, ppInputLayout);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param ppVertexBuffers Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param pStrides Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param pOffsets Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
void WINAPI Mine_ID3D11DeviceContext_IAGetVertexBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets)
{
    Real_ID3D11DeviceContext_IAGetVertexBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param pObj Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param pIndexBuffer Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param Format Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param Offset Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
void WINAPI Mine_ID3D11DeviceContext_IAGetIndexBuffer(ID3D11DeviceContext* pObj, ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset)
{
    Real_ID3D11DeviceContext_IAGetIndexBuffer(&DetourID3D11DeviceContext, pObj, pIndexBuffer, Format, Offset);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_GSGetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    Real_ID3D11DeviceContext_GSGetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::GSGetShader()
/// \param pObj Parameter for ID3D11DeviceContext::GSGetShader()
/// \param ppGeometryShader Parameter for ID3D11DeviceContext::GSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::GSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::GSGetShader()
void WINAPI Mine_ID3D11DeviceContext_GSGetShader(ID3D11DeviceContext* pObj, ID3D11GeometryShader** ppGeometryShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    Real_ID3D11DeviceContext_GSGetShader(&DetourID3D11DeviceContext, pObj, ppGeometryShader, ppClassInstances, pNumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::IAGetPrimitiveTopology()
/// \param pObj Parameter for ID3D11DeviceContext::IAGetPrimitiveTopology()
/// \param pTopology Parameter for ID3D11DeviceContext::IAGetPrimitiveTopology()
void WINAPI Mine_ID3D11DeviceContext_IAGetPrimitiveTopology(ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY* pTopology)
{
    Real_ID3D11DeviceContext_IAGetPrimitiveTopology(&DetourID3D11DeviceContext, pObj, pTopology);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::VSGetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::VSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::VSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::VSGetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_VSGetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_VSGetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::VSGetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::VSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::VSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::VSGetSamplers()
void WINAPI Mine_ID3D11DeviceContext_VSGetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    Real_ID3D11DeviceContext_VSGetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::GetPredication()
/// \param pObj Parameter for ID3D11DeviceContext::GetPredication()
/// \param ppPredicate Parameter for ID3D11DeviceContext::GetPredication()
/// \param pPredicateValue Parameter for ID3D11DeviceContext::GetPredication()
void WINAPI Mine_ID3D11DeviceContext_GetPredication(ID3D11DeviceContext* pObj, ID3D11Predicate** ppPredicate, BOOL* pPredicateValue)
{
    Real_ID3D11DeviceContext_GetPredication(&DetourID3D11DeviceContext, pObj, ppPredicate, pPredicateValue);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::GSGetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::GSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::GSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::GSGetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_GSGetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_GSGetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::GSGetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::GSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::GSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::GSGetSamplers()
void WINAPI Mine_ID3D11DeviceContext_GSGetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    Real_ID3D11DeviceContext_GSGetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::OMGetRenderTargets()
/// \param pObj Parameter for ID3D11DeviceContext::OMGetRenderTargets()
/// \param NumViews Parameter for ID3D11DeviceContext::OMGetRenderTargets()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMGetRenderTargets()
/// \param ppDepthStencilView Parameter for ID3D11DeviceContext::OMGetRenderTargets()
void WINAPI Mine_ID3D11DeviceContext_OMGetRenderTargets(ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView)
{
    Real_ID3D11DeviceContext_OMGetRenderTargets(&DetourID3D11DeviceContext, pObj, NumViews, ppRenderTargetViews, ppDepthStencilView);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param pObj Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param NumRTVs Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param ppDepthStencilView Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param UAVStartSlot Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
void WINAPI Mine_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
    Real_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews(&DetourID3D11DeviceContext, pObj, NumRTVs, ppRenderTargetViews, ppDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::OMGetBlendState()
/// \param pObj Parameter for ID3D11DeviceContext::OMGetBlendState()
/// \param ppBlendState Parameter for ID3D11DeviceContext::OMGetBlendState()
/// \param BlendFactor Parameter for ID3D11DeviceContext::OMGetBlendState()
/// \param pSampleMask Parameter for ID3D11DeviceContext::OMGetBlendState()
void WINAPI Mine_ID3D11DeviceContext_OMGetBlendState(ID3D11DeviceContext* pObj, ID3D11BlendState** ppBlendState, FLOAT BlendFactor[ 4 ], UINT* pSampleMask)
{
    Real_ID3D11DeviceContext_OMGetBlendState(&DetourID3D11DeviceContext, pObj, ppBlendState, BlendFactor, pSampleMask);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::OMGetDepthStencilState()
/// \param pObj Parameter for ID3D11DeviceContext::OMGetDepthStencilState()
/// \param ppDepthStencilState Parameter for ID3D11DeviceContext::OMGetDepthStencilState()
/// \param pStencilRef Parameter for ID3D11DeviceContext::OMGetDepthStencilState()
void WINAPI Mine_ID3D11DeviceContext_OMGetDepthStencilState(ID3D11DeviceContext* pObj, ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef)
{
    Real_ID3D11DeviceContext_OMGetDepthStencilState(&DetourID3D11DeviceContext, pObj, ppDepthStencilState, pStencilRef);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::SOGetTargets()
/// \param pObj Parameter for ID3D11DeviceContext::SOGetTargets()
/// \param NumBuffers Parameter for ID3D11DeviceContext::SOGetTargets()
/// \param ppSOTargets Parameter for ID3D11DeviceContext::SOGetTargets()
void WINAPI Mine_ID3D11DeviceContext_SOGetTargets(ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer** ppSOTargets)
{
    Real_ID3D11DeviceContext_SOGetTargets(&DetourID3D11DeviceContext, pObj, NumBuffers, ppSOTargets);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::RSGetState()
/// \param pObj Parameter for ID3D11DeviceContext::RSGetState()
/// \param ppRasterizerState Parameter for ID3D11DeviceContext::RSGetState()
void WINAPI Mine_ID3D11DeviceContext_RSGetState(ID3D11DeviceContext* pObj, ID3D11RasterizerState** ppRasterizerState)
{
    Real_ID3D11DeviceContext_RSGetState(&DetourID3D11DeviceContext, pObj, ppRasterizerState);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::RSGetViewports()
/// \param pObj Parameter for ID3D11DeviceContext::RSGetViewports()
/// \param pNumViewports Parameter for ID3D11DeviceContext::RSGetViewports()
/// \param pViewports Parameter for ID3D11DeviceContext::RSGetViewports()
void WINAPI Mine_ID3D11DeviceContext_RSGetViewports(ID3D11DeviceContext* pObj, UINT* pNumViewports, D3D11_VIEWPORT* pViewports)
{
    Real_ID3D11DeviceContext_RSGetViewports(&DetourID3D11DeviceContext, pObj, pNumViewports, pViewports);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::RSGetScissorRects()
/// \param pObj Parameter for ID3D11DeviceContext::RSGetScissorRects()
/// \param pNumRects Parameter for ID3D11DeviceContext::RSGetScissorRects()
/// \param pRects Parameter for ID3D11DeviceContext::RSGetScissorRects()
void WINAPI Mine_ID3D11DeviceContext_RSGetScissorRects(ID3D11DeviceContext* pObj, UINT* pNumRects, D3D11_RECT* pRects)
{
    Real_ID3D11DeviceContext_RSGetScissorRects(&DetourID3D11DeviceContext, pObj, pNumRects, pRects);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::HSGetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::HSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::HSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::HSGetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_HSGetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_HSGetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::HSGetShader()
/// \param pObj Parameter for ID3D11DeviceContext::HSGetShader()
/// \param ppHullShader Parameter for ID3D11DeviceContext::HSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::HSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::HSGetShader()
void WINAPI Mine_ID3D11DeviceContext_HSGetShader(ID3D11DeviceContext* pObj, ID3D11HullShader** ppHullShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    Real_ID3D11DeviceContext_HSGetShader(&DetourID3D11DeviceContext, pObj, ppHullShader, ppClassInstances, pNumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::HSGetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::HSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::HSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::HSGetSamplers()
void WINAPI Mine_ID3D11DeviceContext_HSGetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    Real_ID3D11DeviceContext_HSGetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_HSGetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    Real_ID3D11DeviceContext_HSGetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::DSGetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::DSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::DSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::DSGetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_DSGetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_DSGetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::DSGetShader()
/// \param pObj Parameter for ID3D11DeviceContext::DSGetShader()
/// \param ppDomainShader Parameter for ID3D11DeviceContext::DSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::DSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::DSGetShader()
void WINAPI Mine_ID3D11DeviceContext_DSGetShader(ID3D11DeviceContext* pObj, ID3D11DomainShader** ppDomainShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    Real_ID3D11DeviceContext_DSGetShader(&DetourID3D11DeviceContext, pObj, ppDomainShader, ppClassInstances, pNumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::DSGetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::DSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::DSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::DSGetSamplers()
void WINAPI Mine_ID3D11DeviceContext_DSGetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    Real_ID3D11DeviceContext_DSGetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_DSGetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    Real_ID3D11DeviceContext_DSGetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::CSGetShaderResources()
/// \param pObj Parameter for ID3D11DeviceContext::CSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::CSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::CSGetShaderResources()
void WINAPI Mine_ID3D11DeviceContext_CSGetShaderResources(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    Real_ID3D11DeviceContext_CSGetShaderResources(&DetourID3D11DeviceContext, pObj, StartSlot, NumViews, ppShaderResourceViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param pObj Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
void WINAPI Mine_ID3D11DeviceContext_CSGetUnorderedAccessViews(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
    Real_ID3D11DeviceContext_CSGetUnorderedAccessViews(&DetourID3D11DeviceContext, pObj, StartSlot, NumUAVs, ppUnorderedAccessViews);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::CSGetShader()
/// \param pObj Parameter for ID3D11DeviceContext::CSGetShader()
/// \param ppComputeShader Parameter for ID3D11DeviceContext::CSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::CSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::CSGetShader()
void WINAPI Mine_ID3D11DeviceContext_CSGetShader(ID3D11DeviceContext* pObj, ID3D11ComputeShader** ppComputeShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    Real_ID3D11DeviceContext_CSGetShader(&DetourID3D11DeviceContext, pObj, ppComputeShader, ppClassInstances, pNumClassInstances);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::CSGetSamplers()
/// \param pObj Parameter for ID3D11DeviceContext::CSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::CSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::CSGetSamplers()
void WINAPI Mine_ID3D11DeviceContext_CSGetSamplers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    Real_ID3D11DeviceContext_CSGetSamplers(&DetourID3D11DeviceContext, pObj, StartSlot, NumSamplers, ppSamplers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param pObj Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
void WINAPI Mine_ID3D11DeviceContext_CSGetConstantBuffers(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    Real_ID3D11DeviceContext_CSGetConstantBuffers(&DetourID3D11DeviceContext, pObj, StartSlot, NumBuffers, ppConstantBuffers);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }
}

/// Detoured function for ID3D11DeviceContext::ClearState()
/// \param pObj Parameter for ID3D11DeviceContext::ClearState()
void WINAPI Mine_ID3D11DeviceContext_ClearState(ID3D11DeviceContext* pObj)
{
    Real_ID3D11DeviceContext_ClearState(&DetourID3D11DeviceContext, pObj);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_ClearState* pCMD = new DC_CMD_ClearState();
        pCMD->OnCreate(pObj);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::Flush()
/// \param pObj Parameter for ID3D11DeviceContext::Flush()
void WINAPI Mine_ID3D11DeviceContext_Flush(ID3D11DeviceContext* pObj)
{
    Real_ID3D11DeviceContext_Flush(&DetourID3D11DeviceContext, pObj);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        DC_CMD_Flush* pCMD = new DC_CMD_Flush();
        pCMD->OnCreate(pObj);
        g_Profiler.GetCommandRecorder().AddToCommandList(pObj, pCMD);
    }
}

/// Detoured function for ID3D11DeviceContext::GetContextFlags()
/// \param pObj Parameter for ID3D11DeviceContext::GetContextFlags()
UINT WINAPI Mine_ID3D11DeviceContext_GetContextFlags(ID3D11DeviceContext* pObj)
{
    UINT ret = Real_ID3D11DeviceContext_GetContextFlags(&DetourID3D11DeviceContext, pObj);
    D3D11_DEVICE_CONTEXT_TYPE dcType = pObj->GetType();

    if (dcType == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        //Called from Deferred Device Context
        assert(!"Get*() called on deferred context");
    }

    return ret;
}

DCProfilerDeviceContext11VTManager::DCProfilerDeviceContext11VTManager(ptrdiff_t* pFnMine) : ID3D11DeviceContextVTableManager(pFnMine)
{
    m_pMine_ID3D11DeviceContext_VSSetConstantBuffersFn = Mine_ID3D11DeviceContext_VSSetConstantBuffers;
    m_pMine_ID3D11DeviceContext_PSSetShaderResourcesFn = Mine_ID3D11DeviceContext_PSSetShaderResources;
    m_pMine_ID3D11DeviceContext_PSSetShaderFn = Mine_ID3D11DeviceContext_PSSetShader;
    m_pMine_ID3D11DeviceContext_PSSetSamplersFn = Mine_ID3D11DeviceContext_PSSetSamplers;
    m_pMine_ID3D11DeviceContext_VSSetShaderFn = Mine_ID3D11DeviceContext_VSSetShader;
    m_pMine_ID3D11DeviceContext_DrawIndexedFn = Mine_ID3D11DeviceContext_DrawIndexed;
    m_pMine_ID3D11DeviceContext_DrawFn = Mine_ID3D11DeviceContext_Draw;
    m_pMine_ID3D11DeviceContext_MapFn = Mine_ID3D11DeviceContext_Map;
    m_pMine_ID3D11DeviceContext_UnmapFn = Mine_ID3D11DeviceContext_Unmap;
    m_pMine_ID3D11DeviceContext_PSSetConstantBuffersFn = Mine_ID3D11DeviceContext_PSSetConstantBuffers;
    m_pMine_ID3D11DeviceContext_IASetInputLayoutFn = Mine_ID3D11DeviceContext_IASetInputLayout;
    m_pMine_ID3D11DeviceContext_IASetVertexBuffersFn = Mine_ID3D11DeviceContext_IASetVertexBuffers;
    m_pMine_ID3D11DeviceContext_IASetIndexBufferFn = Mine_ID3D11DeviceContext_IASetIndexBuffer;
    m_pMine_ID3D11DeviceContext_DrawIndexedInstancedFn = Mine_ID3D11DeviceContext_DrawIndexedInstanced;
    m_pMine_ID3D11DeviceContext_DrawInstancedFn = Mine_ID3D11DeviceContext_DrawInstanced;
    m_pMine_ID3D11DeviceContext_GSSetConstantBuffersFn = Mine_ID3D11DeviceContext_GSSetConstantBuffers;
    m_pMine_ID3D11DeviceContext_GSSetShaderFn = Mine_ID3D11DeviceContext_GSSetShader;
    m_pMine_ID3D11DeviceContext_IASetPrimitiveTopologyFn = Mine_ID3D11DeviceContext_IASetPrimitiveTopology;
    m_pMine_ID3D11DeviceContext_VSSetShaderResourcesFn = Mine_ID3D11DeviceContext_VSSetShaderResources;
    m_pMine_ID3D11DeviceContext_VSSetSamplersFn = Mine_ID3D11DeviceContext_VSSetSamplers;
    m_pMine_ID3D11DeviceContext_BeginFn = Mine_ID3D11DeviceContext_Begin;
    m_pMine_ID3D11DeviceContext_EndFn = Mine_ID3D11DeviceContext_End;
    m_pMine_ID3D11DeviceContext_GetDataFn = Mine_ID3D11DeviceContext_GetData;
    m_pMine_ID3D11DeviceContext_SetPredicationFn = Mine_ID3D11DeviceContext_SetPredication;
    m_pMine_ID3D11DeviceContext_GSSetShaderResourcesFn = Mine_ID3D11DeviceContext_GSSetShaderResources;
    m_pMine_ID3D11DeviceContext_GSSetSamplersFn = Mine_ID3D11DeviceContext_GSSetSamplers;
    m_pMine_ID3D11DeviceContext_OMSetRenderTargetsFn = Mine_ID3D11DeviceContext_OMSetRenderTargets;
    m_pMine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViewsFn = Mine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews;
    m_pMine_ID3D11DeviceContext_OMSetBlendStateFn = Mine_ID3D11DeviceContext_OMSetBlendState;
    m_pMine_ID3D11DeviceContext_OMSetDepthStencilStateFn = Mine_ID3D11DeviceContext_OMSetDepthStencilState;
    m_pMine_ID3D11DeviceContext_SOSetTargetsFn = Mine_ID3D11DeviceContext_SOSetTargets;
    m_pMine_ID3D11DeviceContext_DrawAutoFn = Mine_ID3D11DeviceContext_DrawAuto;
    m_pMine_ID3D11DeviceContext_DrawIndexedInstancedIndirectFn = Mine_ID3D11DeviceContext_DrawIndexedInstancedIndirect;
    m_pMine_ID3D11DeviceContext_DrawInstancedIndirectFn = Mine_ID3D11DeviceContext_DrawInstancedIndirect;
    m_pMine_ID3D11DeviceContext_DispatchFn = Mine_ID3D11DeviceContext_Dispatch;
    m_pMine_ID3D11DeviceContext_DispatchIndirectFn = Mine_ID3D11DeviceContext_DispatchIndirect;
    m_pMine_ID3D11DeviceContext_RSSetStateFn = Mine_ID3D11DeviceContext_RSSetState;
    m_pMine_ID3D11DeviceContext_RSSetViewportsFn = Mine_ID3D11DeviceContext_RSSetViewports;
    m_pMine_ID3D11DeviceContext_RSSetScissorRectsFn = Mine_ID3D11DeviceContext_RSSetScissorRects;
    m_pMine_ID3D11DeviceContext_CopySubresourceRegionFn = Mine_ID3D11DeviceContext_CopySubresourceRegion;
    m_pMine_ID3D11DeviceContext_CopyResourceFn = Mine_ID3D11DeviceContext_CopyResource;
    m_pMine_ID3D11DeviceContext_UpdateSubresourceFn = Mine_ID3D11DeviceContext_UpdateSubresource;
    m_pMine_ID3D11DeviceContext_CopyStructureCountFn = Mine_ID3D11DeviceContext_CopyStructureCount;
    m_pMine_ID3D11DeviceContext_ClearRenderTargetViewFn = Mine_ID3D11DeviceContext_ClearRenderTargetView;
    m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewUintFn = Mine_ID3D11DeviceContext_ClearUnorderedAccessViewUint;
    m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewFloatFn = Mine_ID3D11DeviceContext_ClearUnorderedAccessViewFloat;
    m_pMine_ID3D11DeviceContext_ClearDepthStencilViewFn = Mine_ID3D11DeviceContext_ClearDepthStencilView;
    m_pMine_ID3D11DeviceContext_GenerateMipsFn = Mine_ID3D11DeviceContext_GenerateMips;
    m_pMine_ID3D11DeviceContext_SetResourceMinLODFn = Mine_ID3D11DeviceContext_SetResourceMinLOD;
    m_pMine_ID3D11DeviceContext_GetResourceMinLODFn = Mine_ID3D11DeviceContext_GetResourceMinLOD;
    m_pMine_ID3D11DeviceContext_ResolveSubresourceFn = Mine_ID3D11DeviceContext_ResolveSubresource;
    m_pMine_ID3D11DeviceContext_ExecuteCommandListFn = Mine_ID3D11DeviceContext_ExecuteCommandList;
    m_pMine_ID3D11DeviceContext_HSSetShaderResourcesFn = Mine_ID3D11DeviceContext_HSSetShaderResources;
    m_pMine_ID3D11DeviceContext_HSSetShaderFn = Mine_ID3D11DeviceContext_HSSetShader;
    m_pMine_ID3D11DeviceContext_HSSetSamplersFn = Mine_ID3D11DeviceContext_HSSetSamplers;
    m_pMine_ID3D11DeviceContext_HSSetConstantBuffersFn = Mine_ID3D11DeviceContext_HSSetConstantBuffers;
    m_pMine_ID3D11DeviceContext_DSSetShaderResourcesFn = Mine_ID3D11DeviceContext_DSSetShaderResources;
    m_pMine_ID3D11DeviceContext_DSSetShaderFn = Mine_ID3D11DeviceContext_DSSetShader;
    m_pMine_ID3D11DeviceContext_DSSetSamplersFn = Mine_ID3D11DeviceContext_DSSetSamplers;
    m_pMine_ID3D11DeviceContext_DSSetConstantBuffersFn = Mine_ID3D11DeviceContext_DSSetConstantBuffers;
    m_pMine_ID3D11DeviceContext_CSSetShaderResourcesFn = Mine_ID3D11DeviceContext_CSSetShaderResources;
    m_pMine_ID3D11DeviceContext_CSSetUnorderedAccessViewsFn = Mine_ID3D11DeviceContext_CSSetUnorderedAccessViews;
    m_pMine_ID3D11DeviceContext_CSSetShaderFn = Mine_ID3D11DeviceContext_CSSetShader;
    m_pMine_ID3D11DeviceContext_CSSetSamplersFn = Mine_ID3D11DeviceContext_CSSetSamplers;
    m_pMine_ID3D11DeviceContext_CSSetConstantBuffersFn = Mine_ID3D11DeviceContext_CSSetConstantBuffers;

    m_pMine_ID3D11DeviceContext_ClearStateFn = Mine_ID3D11DeviceContext_ClearState;
    m_pMine_ID3D11DeviceContext_FlushFn = Mine_ID3D11DeviceContext_Flush;
    m_pMine_ID3D11DeviceContext_GetContextFlagsFn = Mine_ID3D11DeviceContext_GetContextFlags;
    m_pMine_ID3D11DeviceContext_FinishCommandListFn = Mine_ID3D11DeviceContext_FinishCommandList;
}

