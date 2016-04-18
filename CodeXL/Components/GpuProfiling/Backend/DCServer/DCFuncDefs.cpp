//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================


#include "DCFuncDefs.h"
#include "DCDetourHelper.h"
#include "DCVtableOffsets.h"


IDXGISwapChain_Present_type Real_IDXGISwapChain_Present = NULL;

IDXGIFactory1_Release_type Real_IDXGIFactory1_Release = NULL;
IDXGIFactory1_CreateSwapChain_type Real_IDXGIFactory1_CreateSwapChain = NULL;

D3D11CreateDeviceAndSwapChain_type Real_D3D11CreateDeviceAndSwapChain = NULL;
D3D11CreateDevice_type Real_D3D11CreateDevice = NULL;

//D3DX11CompileFromFileW_type Real_D3DX11CompileFromFileW = NULL;
//D3DX11CompileFromResourceW_type Real_D3DX11CompileFromResourceW = NULL;
//
//D3DX11CompileFromFileA_type Real_D3DX11CompileFromFileA = NULL;
//D3DX11CompileFromResourceA_type Real_D3DX11CompileFromResourceA = NULL;
//
//D3DX11CompileFromMemory_type Real_D3DX11CompileFromMemory = NULL;
CreateDXGIFactory1_type Real_CreateDXGIFactory1 = NULL;
D3DCompile_type Real_D3DCompile = NULL;
D3DCompile2_type Real_D3DCompile2 = NULL;
D3DCompileFromFile_type Real_D3DCompileFromFile = NULL;

void Real_ID3D11DeviceContext_VSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetConstantBuffers);
    ((ID3D11DeviceContext_VSSetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_PSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetShaderResources);
    ((ID3D11DeviceContext_PSSetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_PSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetShader);
    ((ID3D11DeviceContext_PSSetShader_type) pFn)(pObj, pPixelShader, ppClassInstances, NumClassInstances);
}

void Real_ID3D11DeviceContext_PSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetSamplers);
    ((ID3D11DeviceContext_PSSetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_VSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetShader);
    ((ID3D11DeviceContext_VSSetShader_type) pFn)(pObj, pVertexShader, ppClassInstances, NumClassInstances);
}

void Real_ID3D11DeviceContext_DrawIndexed(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawIndexed);
    ((ID3D11DeviceContext_DrawIndexed_type) pFn)(pObj, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void Real_ID3D11DeviceContext_Draw(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT VertexCount, UINT StartVertexLocation)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_Draw);
    ((ID3D11DeviceContext_Draw_type) pFn)(pObj, VertexCount, StartVertexLocation);
}

HRESULT Real_ID3D11DeviceContext_Map(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_Map);
    HRESULT ret = ((ID3D11DeviceContext_Map_type) pFn)(pObj, pResource, Subresource, MapType, MapFlags, pMappedResource);
    return ret;
}

void Real_ID3D11DeviceContext_Unmap(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_Unmap);
    ((ID3D11DeviceContext_Unmap_type) pFn)(pObj, pResource, Subresource);
}

void Real_ID3D11DeviceContext_PSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetConstantBuffers);
    ((ID3D11DeviceContext_PSSetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_IASetInputLayout(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11InputLayout* pInputLayout)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetInputLayout);
    ((ID3D11DeviceContext_IASetInputLayout_type) pFn)(pObj, pInputLayout);
}

void Real_ID3D11DeviceContext_IASetVertexBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetVertexBuffers);
    ((ID3D11DeviceContext_IASetVertexBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
}

void Real_ID3D11DeviceContext_IASetIndexBuffer(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetIndexBuffer);
    ((ID3D11DeviceContext_IASetIndexBuffer_type) pFn)(pObj, pIndexBuffer, Format, Offset);
}

void Real_ID3D11DeviceContext_DrawIndexedInstanced(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawIndexedInstanced);
    ((ID3D11DeviceContext_DrawIndexedInstanced_type) pFn)(pObj, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void Real_ID3D11DeviceContext_DrawInstanced(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawInstanced);
    ((ID3D11DeviceContext_DrawInstanced_type) pFn)(pObj, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void Real_ID3D11DeviceContext_GSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetConstantBuffers);
    ((ID3D11DeviceContext_GSSetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_GSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetShader);
    ((ID3D11DeviceContext_GSSetShader_type) pFn)(pObj, pShader, ppClassInstances, NumClassInstances);
}

void Real_ID3D11DeviceContext_IASetPrimitiveTopology(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY Topology)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetPrimitiveTopology);
    ((ID3D11DeviceContext_IASetPrimitiveTopology_type) pFn)(pObj, Topology);
}

void Real_ID3D11DeviceContext_VSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetShaderResources);
    ((ID3D11DeviceContext_VSSetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_VSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetSamplers);
    ((ID3D11DeviceContext_VSSetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_Begin(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_Begin);
    ((ID3D11DeviceContext_Begin_type) pFn)(pObj, pAsync);
}

void Real_ID3D11DeviceContext_End(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_End);
    ((ID3D11DeviceContext_End_type) pFn)(pObj, pAsync);
}

HRESULT Real_ID3D11DeviceContext_GetData(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetData);
    HRESULT ret = ((ID3D11DeviceContext_GetData_type) pFn)(pObj, pAsync, pData, DataSize, GetDataFlags);
    return ret;
}

void Real_ID3D11DeviceContext_SetPredication(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Predicate* pPredicate, BOOL PredicateValue)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_SetPredication);
    ((ID3D11DeviceContext_SetPredication_type) pFn)(pObj, pPredicate, PredicateValue);
}

void Real_ID3D11DeviceContext_GSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetShaderResources);
    ((ID3D11DeviceContext_GSSetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_GSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetSamplers);
    ((ID3D11DeviceContext_GSSetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_OMSetRenderTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetRenderTargets);
    ((ID3D11DeviceContext_OMSetRenderTargets_type) pFn)(pObj, NumViews, ppRenderTargetViews, pDepthStencilView);
}

void Real_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews);
    ((ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews_type) pFn)(pObj, NumRTVs, ppRenderTargetViews, pDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
}

void Real_ID3D11DeviceContext_OMSetBlendState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11BlendState* pBlendState, const FLOAT BlendFactor[ 4 ], UINT SampleMask)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetBlendState);
    ((ID3D11DeviceContext_OMSetBlendState_type) pFn)(pObj, pBlendState, BlendFactor, SampleMask);
}

void Real_ID3D11DeviceContext_OMSetDepthStencilState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetDepthStencilState);
    ((ID3D11DeviceContext_OMSetDepthStencilState_type) pFn)(pObj, pDepthStencilState, StencilRef);
}

void Real_ID3D11DeviceContext_SOSetTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_SOSetTargets);
    ((ID3D11DeviceContext_SOSetTargets_type) pFn)(pObj, NumBuffers, ppSOTargets, pOffsets);
}

void Real_ID3D11DeviceContext_DrawAuto(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawAuto);
    ((ID3D11DeviceContext_DrawAuto_type) pFn)(pObj);
}

void Real_ID3D11DeviceContext_DrawIndexedInstancedIndirect(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawIndexedInstancedIndirect);
    ((ID3D11DeviceContext_DrawIndexedInstancedIndirect_type) pFn)(pObj, pBufferForArgs, AlignedByteOffsetForArgs);
}

void Real_ID3D11DeviceContext_DrawInstancedIndirect(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawInstancedIndirect);
    ((ID3D11DeviceContext_DrawInstancedIndirect_type) pFn)(pObj, pBufferForArgs, AlignedByteOffsetForArgs);
}

void Real_ID3D11DeviceContext_Dispatch(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_Dispatch);
    ((ID3D11DeviceContext_Dispatch_type) pFn)(pObj, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void Real_ID3D11DeviceContext_DispatchIndirect(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DispatchIndirect);
    ((ID3D11DeviceContext_DispatchIndirect_type) pFn)(pObj, pBufferForArgs, AlignedByteOffsetForArgs);
}

void Real_ID3D11DeviceContext_RSSetState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11RasterizerState* pRasterizerState)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSSetState);
    ((ID3D11DeviceContext_RSSetState_type) pFn)(pObj, pRasterizerState);
}

void Real_ID3D11DeviceContext_RSSetViewports(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumViewports, const D3D11_VIEWPORT* pViewports)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSSetViewports);
    ((ID3D11DeviceContext_RSSetViewports_type) pFn)(pObj, NumViewports, pViewports);
}

void Real_ID3D11DeviceContext_RSSetScissorRects(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumRects, const D3D11_RECT* pRects)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSSetScissorRects);
    ((ID3D11DeviceContext_RSSetScissorRects_type) pFn)(pObj, NumRects, pRects);
}

void Real_ID3D11DeviceContext_CopySubresourceRegion(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CopySubresourceRegion);
    ((ID3D11DeviceContext_CopySubresourceRegion_type) pFn)(pObj, pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
}

void Real_ID3D11DeviceContext_CopyResource(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CopyResource);
    ((ID3D11DeviceContext_CopyResource_type) pFn)(pObj, pDstResource, pSrcResource);
}

void Real_ID3D11DeviceContext_UpdateSubresource(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_UpdateSubresource);
    ((ID3D11DeviceContext_UpdateSubresource_type) pFn)(pObj, pDstResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
}

void Real_ID3D11DeviceContext_CopyStructureCount(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView* pSrcView)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CopyStructureCount);
    ((ID3D11DeviceContext_CopyStructureCount_type) pFn)(pObj, pDstBuffer, DstAlignedByteOffset, pSrcView);
}

void Real_ID3D11DeviceContext_ClearRenderTargetView(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[ 4 ])
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearRenderTargetView);
    ((ID3D11DeviceContext_ClearRenderTargetView_type) pFn)(pObj, pRenderTargetView, ColorRGBA);
}

void Real_ID3D11DeviceContext_ClearUnorderedAccessViewUint(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[ 4 ])
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearUnorderedAccessViewUint);
    ((ID3D11DeviceContext_ClearUnorderedAccessViewUint_type) pFn)(pObj, pUnorderedAccessView, Values);
}

void Real_ID3D11DeviceContext_ClearUnorderedAccessViewFloat(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const FLOAT Values[ 4 ])
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearUnorderedAccessViewFloat);
    ((ID3D11DeviceContext_ClearUnorderedAccessViewFloat_type) pFn)(pObj, pUnorderedAccessView, Values);
}

void Real_ID3D11DeviceContext_ClearDepthStencilView(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearDepthStencilView);
    ((ID3D11DeviceContext_ClearDepthStencilView_type) pFn)(pObj, pDepthStencilView, ClearFlags, Depth, Stencil);
}

void Real_ID3D11DeviceContext_GenerateMips(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11ShaderResourceView* pShaderResourceView)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GenerateMips);
    ((ID3D11DeviceContext_GenerateMips_type) pFn)(pObj, pShaderResourceView);
}

void Real_ID3D11DeviceContext_SetResourceMinLOD(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource, FLOAT MinLOD)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_SetResourceMinLOD);
    ((ID3D11DeviceContext_SetResourceMinLOD_type) pFn)(pObj, pResource, MinLOD);
}

FLOAT Real_ID3D11DeviceContext_GetResourceMinLOD(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetResourceMinLOD);
    FLOAT ret = ((ID3D11DeviceContext_GetResourceMinLOD_type) pFn)(pObj, pResource);
    return ret;
}

void Real_ID3D11DeviceContext_ResolveSubresource(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ResolveSubresource);
    ((ID3D11DeviceContext_ResolveSubresource_type) pFn)(pObj, pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
}

void Real_ID3D11DeviceContext_ExecuteCommandList(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11CommandList* pCommandList, BOOL RestoreContextState)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ExecuteCommandList);
    ((ID3D11DeviceContext_ExecuteCommandList_type) pFn)(pObj, pCommandList, RestoreContextState);
}

void Real_ID3D11DeviceContext_HSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetShaderResources);
    ((ID3D11DeviceContext_HSSetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_HSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetShader);
    ((ID3D11DeviceContext_HSSetShader_type) pFn)(pObj, pHullShader, ppClassInstances, NumClassInstances);
}

void Real_ID3D11DeviceContext_HSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetSamplers);
    ((ID3D11DeviceContext_HSSetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_HSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetConstantBuffers);
    ((ID3D11DeviceContext_HSSetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_DSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetShaderResources);
    ((ID3D11DeviceContext_DSSetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_DSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetShader);
    ((ID3D11DeviceContext_DSSetShader_type) pFn)(pObj, pDomainShader, ppClassInstances, NumClassInstances);
}

void Real_ID3D11DeviceContext_DSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetSamplers);
    ((ID3D11DeviceContext_DSSetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_DSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetConstantBuffers);
    ((ID3D11DeviceContext_DSSetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_CSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetShaderResources);
    ((ID3D11DeviceContext_CSSetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_CSSetUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetUnorderedAccessViews);
    ((ID3D11DeviceContext_CSSetUnorderedAccessViews_type) pFn)(pObj, StartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
}

void Real_ID3D11DeviceContext_CSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetShader);
    ((ID3D11DeviceContext_CSSetShader_type) pFn)(pObj, pComputeShader, ppClassInstances, NumClassInstances);
}

void Real_ID3D11DeviceContext_CSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetSamplers);
    ((ID3D11DeviceContext_CSSetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_CSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetConstantBuffers);
    ((ID3D11DeviceContext_CSSetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_VSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetConstantBuffers);
    ((ID3D11DeviceContext_VSGetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_PSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetShaderResources);
    ((ID3D11DeviceContext_PSGetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_PSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetShader);
    ((ID3D11DeviceContext_PSGetShader_type) pFn)(pObj, ppPixelShader, ppClassInstances, pNumClassInstances);
}

void Real_ID3D11DeviceContext_PSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetSamplers);
    ((ID3D11DeviceContext_PSGetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_VSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetShader);
    ((ID3D11DeviceContext_VSGetShader_type) pFn)(pObj, ppVertexShader, ppClassInstances, pNumClassInstances);
}

void Real_ID3D11DeviceContext_PSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetConstantBuffers);
    ((ID3D11DeviceContext_PSGetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_IAGetInputLayout(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11InputLayout** ppInputLayout)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetInputLayout);
    ((ID3D11DeviceContext_IAGetInputLayout_type) pFn)(pObj, ppInputLayout);
}

void Real_ID3D11DeviceContext_IAGetVertexBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetVertexBuffers);
    ((ID3D11DeviceContext_IAGetVertexBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
}

void Real_ID3D11DeviceContext_IAGetIndexBuffer(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetIndexBuffer);
    ((ID3D11DeviceContext_IAGetIndexBuffer_type) pFn)(pObj, pIndexBuffer, Format, Offset);
}

void Real_ID3D11DeviceContext_GSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetConstantBuffers);
    ((ID3D11DeviceContext_GSGetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_GSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11GeometryShader** ppGeometryShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetShader);
    ((ID3D11DeviceContext_GSGetShader_type) pFn)(pObj, ppGeometryShader, ppClassInstances, pNumClassInstances);
}

void Real_ID3D11DeviceContext_IAGetPrimitiveTopology(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY* pTopology)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetPrimitiveTopology);
    ((ID3D11DeviceContext_IAGetPrimitiveTopology_type) pFn)(pObj, pTopology);
}

void Real_ID3D11DeviceContext_VSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetShaderResources);
    ((ID3D11DeviceContext_VSGetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_VSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetSamplers);
    ((ID3D11DeviceContext_VSGetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_GetPredication(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Predicate** ppPredicate, BOOL* pPredicateValue)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetPredication);
    ((ID3D11DeviceContext_GetPredication_type) pFn)(pObj, ppPredicate, pPredicateValue);
}

void Real_ID3D11DeviceContext_GSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetShaderResources);
    ((ID3D11DeviceContext_GSGetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_GSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetSamplers);
    ((ID3D11DeviceContext_GSGetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_OMGetRenderTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetRenderTargets);
    ((ID3D11DeviceContext_OMGetRenderTargets_type) pFn)(pObj, NumViews, ppRenderTargetViews, ppDepthStencilView);
}

void Real_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews);
    ((ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews_type) pFn)(pObj, NumRTVs, ppRenderTargetViews, ppDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews);
}

void Real_ID3D11DeviceContext_OMGetBlendState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11BlendState** ppBlendState, FLOAT BlendFactor[ 4 ], UINT* pSampleMask)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetBlendState);
    ((ID3D11DeviceContext_OMGetBlendState_type) pFn)(pObj, ppBlendState, BlendFactor, pSampleMask);
}

void Real_ID3D11DeviceContext_OMGetDepthStencilState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetDepthStencilState);
    ((ID3D11DeviceContext_OMGetDepthStencilState_type) pFn)(pObj, ppDepthStencilState, pStencilRef);
}

void Real_ID3D11DeviceContext_SOGetTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer** ppSOTargets)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_SOGetTargets);
    ((ID3D11DeviceContext_SOGetTargets_type) pFn)(pObj, NumBuffers, ppSOTargets);
}

void Real_ID3D11DeviceContext_RSGetState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11RasterizerState** ppRasterizerState)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSGetState);
    ((ID3D11DeviceContext_RSGetState_type) pFn)(pObj, ppRasterizerState);
}

void Real_ID3D11DeviceContext_RSGetViewports(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT* pNumViewports, D3D11_VIEWPORT* pViewports)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSGetViewports);
    ((ID3D11DeviceContext_RSGetViewports_type) pFn)(pObj, pNumViewports, pViewports);
}

void Real_ID3D11DeviceContext_RSGetScissorRects(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT* pNumRects, D3D11_RECT* pRects)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSGetScissorRects);
    ((ID3D11DeviceContext_RSGetScissorRects_type) pFn)(pObj, pNumRects, pRects);
}

void Real_ID3D11DeviceContext_HSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetShaderResources);
    ((ID3D11DeviceContext_HSGetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_HSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11HullShader** ppHullShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetShader);
    ((ID3D11DeviceContext_HSGetShader_type) pFn)(pObj, ppHullShader, ppClassInstances, pNumClassInstances);
}

void Real_ID3D11DeviceContext_HSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetSamplers);
    ((ID3D11DeviceContext_HSGetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_HSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetConstantBuffers);
    ((ID3D11DeviceContext_HSGetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_DSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetShaderResources);
    ((ID3D11DeviceContext_DSGetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_DSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DomainShader** ppDomainShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetShader);
    ((ID3D11DeviceContext_DSGetShader_type) pFn)(pObj, ppDomainShader, ppClassInstances, pNumClassInstances);
}

void Real_ID3D11DeviceContext_DSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetSamplers);
    ((ID3D11DeviceContext_DSGetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_DSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetConstantBuffers);
    ((ID3D11DeviceContext_DSGetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_CSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetShaderResources);
    ((ID3D11DeviceContext_CSGetShaderResources_type) pFn)(pObj, StartSlot, NumViews, ppShaderResourceViews);
}

void Real_ID3D11DeviceContext_CSGetUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetUnorderedAccessViews);
    ((ID3D11DeviceContext_CSGetUnorderedAccessViews_type) pFn)(pObj, StartSlot, NumUAVs, ppUnorderedAccessViews);
}

void Real_ID3D11DeviceContext_CSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11ComputeShader** ppComputeShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetShader);
    ((ID3D11DeviceContext_CSGetShader_type) pFn)(pObj, ppComputeShader, ppClassInstances, pNumClassInstances);
}

void Real_ID3D11DeviceContext_CSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetSamplers);
    ((ID3D11DeviceContext_CSGetSamplers_type) pFn)(pObj, StartSlot, NumSamplers, ppSamplers);
}

void Real_ID3D11DeviceContext_CSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetConstantBuffers);
    ((ID3D11DeviceContext_CSGetConstantBuffers_type) pFn)(pObj, StartSlot, NumBuffers, ppConstantBuffers);
}

void Real_ID3D11DeviceContext_ClearState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearState);
    ((ID3D11DeviceContext_ClearState_type) pFn)(pObj);
}

void Real_ID3D11DeviceContext_Flush(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_Flush);
    ((ID3D11DeviceContext_Flush_type) pFn)(pObj);
}

UINT Real_ID3D11DeviceContext_GetContextFlags(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetContextFlags);
    UINT ret = ((ID3D11DeviceContext_GetContextFlags_type) pFn)(pObj);
    return ret;
}

HRESULT Real_ID3D11DeviceContext_FinishCommandList(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, BOOL RestoreDeferredContextState, ID3D11CommandList** ppCommandList)
{
    const void* pFn = (const void*)pVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11DeviceContext_FinishCommandList);
    HRESULT ret = ((ID3D11DeviceContext_FinishCommandList_type) pFn)(pObj, RestoreDeferredContextState, ppCommandList);
    return ret;
}

HRESULT Real_ID3D11Device_CreateBuffer(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateBuffer);
    HRESULT ret = ((ID3D11Device_CreateBuffer_type) pFn)(pObj, pDesc, pInitialData, ppBuffer);
    return ret;
}

HRESULT Real_ID3D11Device_CreateTexture1D(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture1D);
    HRESULT ret = ((ID3D11Device_CreateTexture1D_type) pFn)(pObj, pDesc, pInitialData, ppTexture1D);
    return ret;
}

HRESULT Real_ID3D11Device_CreateTexture2D(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture2D);
    HRESULT ret = ((ID3D11Device_CreateTexture2D_type) pFn)(pObj, pDesc, pInitialData, ppTexture2D);
    return ret;
}

HRESULT Real_ID3D11Device_CreateTexture3D(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture3D);
    HRESULT ret = ((ID3D11Device_CreateTexture3D_type) pFn)(pObj, pDesc, pInitialData, ppTexture3D);
    return ret;
}

HRESULT Real_ID3D11Device_CreateUnorderedAccessView(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateUnorderedAccessView);
    HRESULT ret = ((ID3D11Device_CreateUnorderedAccessView_type) pFn)(pObj, pResource, pDesc, ppUAView);
    return ret;
}

HRESULT Real_ID3D11Device_CreateDeferredContext(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, UINT ContextFlags, ID3D11DeviceContext** ppDeferredContext)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateDeferredContext);
    HRESULT ret = ((ID3D11Device_CreateDeferredContext_type) pFn)(pObj, ContextFlags, ppDeferredContext);
    return ret;
}

void Real_ID3D11Device_GetImmediateContext(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, ID3D11DeviceContext** ppImmediateContext)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_GetImmediateContext);
    ((ID3D11Device_GetImmediateContext_type) pFn)(pObj, ppImmediateContext);
}

HRESULT Real_ID3D11Device_CreateComputeShader(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
{
    const void* pFn = (const void*)pDeviceVTMgr->CallReal(pObj, DX11_VTABLE_OFFSET_ID3D11Device_CreateComputeShader);
    HRESULT ret = ((ID3D11Device_CreateComputeShader_type) pFn)(pObj, pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);
    return ret;
}