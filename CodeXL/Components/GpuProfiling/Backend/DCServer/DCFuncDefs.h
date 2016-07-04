//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================


#ifndef _DC_FUNC_DEFS_H_
#define _DC_FUNC_DEFS_H_

#include "D3Dcommon.h"
#include "D3Dcompiler.h"

#include "DCID3D11DeviceContext_typedefs.h"
#include "DCID3D11Device_typedefs.h"
#include "DCID3D11DeviceContextVTableManager.h"
#include "DCID3D11DeviceVTManager.h"

/// \addtogroup DCVirtualTablePatching
// @{

typedef HRESULT(WINAPI* CreateDXGIFactory1_type)(
    REFIID riid,
    void** ppFactory
);

typedef HRESULT(WINAPI* D3D11CreateDeviceAndSwapChain_type)(
    IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    CONST D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels, // New e.g. D3D_FEATURE_LEVEL_11_0
    UINT SDKVersion,
    CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel, // New e.g. D3D_FEATURE_LEVEL_11_0
    ID3D11DeviceContext** ppImmediateContext // New
);

typedef HRESULT(WINAPI* D3D11CreateDevice_type)(IDXGIAdapter* pAdapter,
                                                D3D_DRIVER_TYPE DriverType,
                                                HMODULE Software,
                                                UINT Flags,
                                                CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                                UINT FeatureLevels,
                                                UINT SDKVersion,
                                                ID3D11Device** ppDevice,
                                                D3D_FEATURE_LEVEL* pFeatureLevel,
                                                ID3D11DeviceContext** ppImmediateContext);

typedef HRESULT(WINAPI* D3DCompile_type)(
    LPCVOID pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    CONST D3D10_SHADER_MACRO* pDefines,
    LPD3D10INCLUDE pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    LPD3D10BLOB* ppCode,
    LPD3D10BLOB* ppErrorMsgs
);

typedef HRESULT (WINAPI* D3DCompile2_type)(
    LPCVOID pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    UINT SecondaryDataFlags,
    LPCVOID pSecondaryData,
    SIZE_T SecondaryDataSize,
    ID3DBlob** ppCode,
    ID3DBlob** ppErrorMsgs
);

typedef HRESULT (WINAPI* D3DCompileFromFile_type)(
    LPCWSTR pFileName,
    D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    ID3DBlob** ppCode,
    ID3DBlob** ppErrorMsgs
);


typedef HRESULT(WINAPI* IDXGISwapChain_Present_type)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

extern IDXGISwapChain_Present_type Real_IDXGISwapChain_Present;

typedef ULONG(WINAPI* IDXGIFactory1_Release_type)(IDXGIFactory1* pFactory1);
typedef HRESULT(WINAPI* IDXGIFactory1_CreateSwapChain_type)(IDXGIFactory1* pFactory1, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);

extern IDXGIFactory1_Release_type Real_IDXGIFactory1_Release;
extern IDXGIFactory1_CreateSwapChain_type Real_IDXGIFactory1_CreateSwapChain;

extern D3D11CreateDeviceAndSwapChain_type Real_D3D11CreateDeviceAndSwapChain;
extern D3D11CreateDevice_type Real_D3D11CreateDevice;

extern CreateDXGIFactory1_type Real_CreateDXGIFactory1;

extern D3DCompile_type Real_D3DCompile;
extern D3DCompile2_type Real_D3DCompile2;
extern D3DCompileFromFile_type Real_D3DCompileFromFile;

/// Real function wrapper for ID3D11DeviceContext::Release
/// \param pObj Parameter for ID3D11DeviceContext::Release()
ULONG Real_ID3D11DeviceContext_Release(ID3D11DeviceContext* pObj);

/// Real function wrapper for ID3D11DeviceContext::VSSetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::VSSetConstantBuffers()
void Real_ID3D11DeviceContext_VSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::PSSetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::PSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::PSSetShaderResources()
void Real_ID3D11DeviceContext_PSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::PSSetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSSetShader()
/// \param pPixelShader Parameter for ID3D11DeviceContext::PSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::PSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::PSSetShader()
void Real_ID3D11DeviceContext_PSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::PSSetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::PSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::PSSetSamplers()
void Real_ID3D11DeviceContext_PSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::VSSetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSSetShader()
/// \param pVertexShader Parameter for ID3D11DeviceContext::VSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::VSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::VSSetShader()
void Real_ID3D11DeviceContext_VSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::DrawIndexed
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DrawIndexed()
/// \param IndexCount Parameter for ID3D11DeviceContext::DrawIndexed()
/// \param StartIndexLocation Parameter for ID3D11DeviceContext::DrawIndexed()
/// \param BaseVertexLocation Parameter for ID3D11DeviceContext::DrawIndexed()
void Real_ID3D11DeviceContext_DrawIndexed(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

/// Real function wrapper for ID3D11DeviceContext::Draw
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::Draw()
/// \param VertexCount Parameter for ID3D11DeviceContext::Draw()
/// \param StartVertexLocation Parameter for ID3D11DeviceContext::Draw()
void Real_ID3D11DeviceContext_Draw(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT VertexCount, UINT StartVertexLocation);

/// Real function wrapper for ID3D11DeviceContext::Map
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::Map()
/// \param pResource Parameter for ID3D11DeviceContext::Map()
/// \param Subresource Parameter for ID3D11DeviceContext::Map()
/// \param MapType Parameter for ID3D11DeviceContext::Map()
/// \param MapFlags Parameter for ID3D11DeviceContext::Map()
/// \param pMappedResource Parameter for ID3D11DeviceContext::Map()
HRESULT Real_ID3D11DeviceContext_Map(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource);

/// Real function wrapper for ID3D11DeviceContext::Unmap
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::Unmap()
/// \param pResource Parameter for ID3D11DeviceContext::Unmap()
/// \param Subresource Parameter for ID3D11DeviceContext::Unmap()
void Real_ID3D11DeviceContext_Unmap(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource);

/// Real function wrapper for ID3D11DeviceContext::PSSetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::PSSetConstantBuffers()
void Real_ID3D11DeviceContext_PSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::IASetInputLayout
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IASetInputLayout()
/// \param pInputLayout Parameter for ID3D11DeviceContext::IASetInputLayout()
void Real_ID3D11DeviceContext_IASetInputLayout(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11InputLayout* pInputLayout);

/// Real function wrapper for ID3D11DeviceContext::IASetVertexBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param ppVertexBuffers Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param pStrides Parameter for ID3D11DeviceContext::IASetVertexBuffers()
/// \param pOffsets Parameter for ID3D11DeviceContext::IASetVertexBuffers()
void Real_ID3D11DeviceContext_IASetVertexBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets);

/// Real function wrapper for ID3D11DeviceContext::IASetIndexBuffer
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IASetIndexBuffer()
/// \param pIndexBuffer Parameter for ID3D11DeviceContext::IASetIndexBuffer()
/// \param Format Parameter for ID3D11DeviceContext::IASetIndexBuffer()
/// \param Offset Parameter for ID3D11DeviceContext::IASetIndexBuffer()
void Real_ID3D11DeviceContext_IASetIndexBuffer(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset);

/// Real function wrapper for ID3D11DeviceContext::DrawIndexedInstanced
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param IndexCountPerInstance Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param InstanceCount Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param StartIndexLocation Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param BaseVertexLocation Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
/// \param StartInstanceLocation Parameter for ID3D11DeviceContext::DrawIndexedInstanced()
void Real_ID3D11DeviceContext_DrawIndexedInstanced(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

/// Real function wrapper for ID3D11DeviceContext::DrawInstanced
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param VertexCountPerInstance Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param InstanceCount Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param StartVertexLocation Parameter for ID3D11DeviceContext::DrawInstanced()
/// \param StartInstanceLocation Parameter for ID3D11DeviceContext::DrawInstanced()
void Real_ID3D11DeviceContext_DrawInstanced(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);

/// Real function wrapper for ID3D11DeviceContext::GSSetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::GSSetConstantBuffers()
void Real_ID3D11DeviceContext_GSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::GSSetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSSetShader()
/// \param pShader Parameter for ID3D11DeviceContext::GSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::GSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::GSSetShader()
void Real_ID3D11DeviceContext_GSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::IASetPrimitiveTopology
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IASetPrimitiveTopology()
/// \param Topology Parameter for ID3D11DeviceContext::IASetPrimitiveTopology()
void Real_ID3D11DeviceContext_IASetPrimitiveTopology(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY Topology);

/// Real function wrapper for ID3D11DeviceContext::VSSetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::VSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::VSSetShaderResources()
void Real_ID3D11DeviceContext_VSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::VSSetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::VSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::VSSetSamplers()
void Real_ID3D11DeviceContext_VSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::Begin
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::Begin()
/// \param pAsync Parameter for ID3D11DeviceContext::Begin()
void Real_ID3D11DeviceContext_Begin(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync);

/// Real function wrapper for ID3D11DeviceContext::End
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::End()
/// \param pAsync Parameter for ID3D11DeviceContext::End()
void Real_ID3D11DeviceContext_End(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync);

/// Real function wrapper for ID3D11DeviceContext::GetData
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GetData()
/// \param pAsync Parameter for ID3D11DeviceContext::GetData()
/// \param pData Parameter for ID3D11DeviceContext::GetData()
/// \param DataSize Parameter for ID3D11DeviceContext::GetData()
/// \param GetDataFlags Parameter for ID3D11DeviceContext::GetData()
HRESULT Real_ID3D11DeviceContext_GetData(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags);

/// Real function wrapper for ID3D11DeviceContext::SetPredication
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::SetPredication()
/// \param pPredicate Parameter for ID3D11DeviceContext::SetPredication()
/// \param PredicateValue Parameter for ID3D11DeviceContext::SetPredication()
void Real_ID3D11DeviceContext_SetPredication(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Predicate* pPredicate, BOOL PredicateValue);

/// Real function wrapper for ID3D11DeviceContext::GSSetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::GSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::GSSetShaderResources()
void Real_ID3D11DeviceContext_GSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::GSSetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::GSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::GSSetSamplers()
void Real_ID3D11DeviceContext_GSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::OMSetRenderTargets
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMSetRenderTargets()
/// \param NumViews Parameter for ID3D11DeviceContext::OMSetRenderTargets()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMSetRenderTargets()
/// \param pDepthStencilView Parameter for ID3D11DeviceContext::OMSetRenderTargets()
void Real_ID3D11DeviceContext_OMSetRenderTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView);

/// Real function wrapper for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param NumRTVs Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param pDepthStencilView Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param UAVStartSlot Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
/// \param pUAVInitialCounts Parameter for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
void Real_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts);

/// Real function wrapper for ID3D11DeviceContext::OMSetBlendState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMSetBlendState()
/// \param pBlendState Parameter for ID3D11DeviceContext::OMSetBlendState()
/// \param BlendFactor Parameter for ID3D11DeviceContext::OMSetBlendState()
/// \param SampleMask Parameter for ID3D11DeviceContext::OMSetBlendState()
void Real_ID3D11DeviceContext_OMSetBlendState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11BlendState* pBlendState, const FLOAT BlendFactor[ 4 ], UINT SampleMask);

/// Real function wrapper for ID3D11DeviceContext::OMSetDepthStencilState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMSetDepthStencilState()
/// \param pDepthStencilState Parameter for ID3D11DeviceContext::OMSetDepthStencilState()
/// \param StencilRef Parameter for ID3D11DeviceContext::OMSetDepthStencilState()
void Real_ID3D11DeviceContext_OMSetDepthStencilState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef);

/// Real function wrapper for ID3D11DeviceContext::SOSetTargets
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::SOSetTargets()
/// \param NumBuffers Parameter for ID3D11DeviceContext::SOSetTargets()
/// \param ppSOTargets Parameter for ID3D11DeviceContext::SOSetTargets()
/// \param pOffsets Parameter for ID3D11DeviceContext::SOSetTargets()
void Real_ID3D11DeviceContext_SOSetTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets);

/// Real function wrapper for ID3D11DeviceContext::DrawAuto
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DrawAuto()
void Real_ID3D11DeviceContext_DrawAuto(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj);

/// Real function wrapper for ID3D11DeviceContext::DrawIndexedInstancedIndirect
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
/// \param pBufferForArgs Parameter for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
/// \param AlignedByteOffsetForArgs Parameter for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
void Real_ID3D11DeviceContext_DrawIndexedInstancedIndirect(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);

/// Real function wrapper for ID3D11DeviceContext::DrawInstancedIndirect
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DrawInstancedIndirect()
/// \param pBufferForArgs Parameter for ID3D11DeviceContext::DrawInstancedIndirect()
/// \param AlignedByteOffsetForArgs Parameter for ID3D11DeviceContext::DrawInstancedIndirect()
void Real_ID3D11DeviceContext_DrawInstancedIndirect(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);

/// Real function wrapper for ID3D11DeviceContext::Dispatch
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::Dispatch()
/// \param ThreadGroupCountX Parameter for ID3D11DeviceContext::Dispatch()
/// \param ThreadGroupCountY Parameter for ID3D11DeviceContext::Dispatch()
/// \param ThreadGroupCountZ Parameter for ID3D11DeviceContext::Dispatch()
void Real_ID3D11DeviceContext_Dispatch(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

/// Real function wrapper for ID3D11DeviceContext::DispatchIndirect
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DispatchIndirect()
/// \param pBufferForArgs Parameter for ID3D11DeviceContext::DispatchIndirect()
/// \param AlignedByteOffsetForArgs Parameter for ID3D11DeviceContext::DispatchIndirect()
void Real_ID3D11DeviceContext_DispatchIndirect(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);

/// Real function wrapper for ID3D11DeviceContext::RSSetState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::RSSetState()
/// \param pRasterizerState Parameter for ID3D11DeviceContext::RSSetState()
void Real_ID3D11DeviceContext_RSSetState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11RasterizerState* pRasterizerState);

/// Real function wrapper for ID3D11DeviceContext::RSSetViewports
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::RSSetViewports()
/// \param NumViewports Parameter for ID3D11DeviceContext::RSSetViewports()
/// \param pViewports Parameter for ID3D11DeviceContext::RSSetViewports()
void Real_ID3D11DeviceContext_RSSetViewports(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumViewports, const D3D11_VIEWPORT* pViewports);

/// Real function wrapper for ID3D11DeviceContext::RSSetScissorRects
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::RSSetScissorRects()
/// \param NumRects Parameter for ID3D11DeviceContext::RSSetScissorRects()
/// \param pRects Parameter for ID3D11DeviceContext::RSSetScissorRects()
void Real_ID3D11DeviceContext_RSSetScissorRects(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumRects, const D3D11_RECT* pRects);

/// Real function wrapper for ID3D11DeviceContext::CopySubresourceRegion
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pDstResource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstSubresource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstX Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstY Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param DstZ Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pSrcResource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param SrcSubresource Parameter for ID3D11DeviceContext::CopySubresourceRegion()
/// \param pSrcBox Parameter for ID3D11DeviceContext::CopySubresourceRegion()
void Real_ID3D11DeviceContext_CopySubresourceRegion(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox);

/// Real function wrapper for ID3D11DeviceContext::CopyResource
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CopyResource()
/// \param pDstResource Parameter for ID3D11DeviceContext::CopyResource()
/// \param pSrcResource Parameter for ID3D11DeviceContext::CopyResource()
void Real_ID3D11DeviceContext_CopyResource(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource);

/// Real function wrapper for ID3D11DeviceContext::UpdateSubresource
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param pDstResource Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param DstSubresource Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param pDstBox Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param pSrcData Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param SrcRowPitch Parameter for ID3D11DeviceContext::UpdateSubresource()
/// \param SrcDepthPitch Parameter for ID3D11DeviceContext::UpdateSubresource()
void Real_ID3D11DeviceContext_UpdateSubresource(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch);

/// Real function wrapper for ID3D11DeviceContext::CopyStructureCount
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CopyStructureCount()
/// \param pDstBuffer Parameter for ID3D11DeviceContext::CopyStructureCount()
/// \param DstAlignedByteOffset Parameter for ID3D11DeviceContext::CopyStructureCount()
/// \param pSrcView Parameter for ID3D11DeviceContext::CopyStructureCount()
void Real_ID3D11DeviceContext_CopyStructureCount(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer* pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView* pSrcView);

/// Real function wrapper for ID3D11DeviceContext::ClearRenderTargetView
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ClearRenderTargetView()
/// \param pRenderTargetView Parameter for ID3D11DeviceContext::ClearRenderTargetView()
/// \param ColorRGBA Parameter for ID3D11DeviceContext::ClearRenderTargetView()
void Real_ID3D11DeviceContext_ClearRenderTargetView(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[ 4 ]);

/// Real function wrapper for ID3D11DeviceContext::ClearUnorderedAccessViewUint
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
/// \param pUnorderedAccessView Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
/// \param Values Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
void Real_ID3D11DeviceContext_ClearUnorderedAccessViewUint(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[ 4 ]);

/// Real function wrapper for ID3D11DeviceContext::ClearUnorderedAccessViewFloat
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
/// \param pUnorderedAccessView Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
/// \param Values Parameter for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
void Real_ID3D11DeviceContext_ClearUnorderedAccessViewFloat(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const FLOAT Values[ 4 ]);

/// Real function wrapper for ID3D11DeviceContext::ClearDepthStencilView
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param pDepthStencilView Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param ClearFlags Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param Depth Parameter for ID3D11DeviceContext::ClearDepthStencilView()
/// \param Stencil Parameter for ID3D11DeviceContext::ClearDepthStencilView()
void Real_ID3D11DeviceContext_ClearDepthStencilView(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil);

/// Real function wrapper for ID3D11DeviceContext::GenerateMips
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GenerateMips()
/// \param pShaderResourceView Parameter for ID3D11DeviceContext::GenerateMips()
void Real_ID3D11DeviceContext_GenerateMips(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11ShaderResourceView* pShaderResourceView);

/// Real function wrapper for ID3D11DeviceContext::SetResourceMinLOD
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::SetResourceMinLOD()
/// \param pResource Parameter for ID3D11DeviceContext::SetResourceMinLOD()
/// \param MinLOD Parameter for ID3D11DeviceContext::SetResourceMinLOD()
void Real_ID3D11DeviceContext_SetResourceMinLOD(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource, FLOAT MinLOD);

/// Real function wrapper for ID3D11DeviceContext::GetResourceMinLOD
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GetResourceMinLOD()
/// \param pResource Parameter for ID3D11DeviceContext::GetResourceMinLOD()
FLOAT Real_ID3D11DeviceContext_GetResourceMinLOD(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pResource);

/// Real function wrapper for ID3D11DeviceContext::ResolveSubresource
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param pDstResource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param DstSubresource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param pSrcResource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param SrcSubresource Parameter for ID3D11DeviceContext::ResolveSubresource()
/// \param Format Parameter for ID3D11DeviceContext::ResolveSubresource()
void Real_ID3D11DeviceContext_ResolveSubresource(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format);

/// Real function wrapper for ID3D11DeviceContext::ExecuteCommandList
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ExecuteCommandList()
/// \param pCommandList Parameter for ID3D11DeviceContext::ExecuteCommandList()
/// \param RestoreContextState Parameter for ID3D11DeviceContext::ExecuteCommandList()
void Real_ID3D11DeviceContext_ExecuteCommandList(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11CommandList* pCommandList, BOOL RestoreContextState);

/// Real function wrapper for ID3D11DeviceContext::HSSetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::HSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::HSSetShaderResources()
void Real_ID3D11DeviceContext_HSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::HSSetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSSetShader()
/// \param pHullShader Parameter for ID3D11DeviceContext::HSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::HSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::HSSetShader()
void Real_ID3D11DeviceContext_HSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::HSSetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::HSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::HSSetSamplers()
void Real_ID3D11DeviceContext_HSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::HSSetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::HSSetConstantBuffers()
void Real_ID3D11DeviceContext_HSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::DSSetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::DSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::DSSetShaderResources()
void Real_ID3D11DeviceContext_DSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::DSSetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSSetShader()
/// \param pDomainShader Parameter for ID3D11DeviceContext::DSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::DSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::DSSetShader()
void Real_ID3D11DeviceContext_DSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::DSSetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::DSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::DSSetSamplers()
void Real_ID3D11DeviceContext_DSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::DSSetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::DSSetConstantBuffers()
void Real_ID3D11DeviceContext_DSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::CSSetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSSetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::CSSetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::CSSetShaderResources()
void Real_ID3D11DeviceContext_CSSetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::CSSetUnorderedAccessViews
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
/// \param pUAVInitialCounts Parameter for ID3D11DeviceContext::CSSetUnorderedAccessViews()
void Real_ID3D11DeviceContext_CSSetUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts);

/// Real function wrapper for ID3D11DeviceContext::CSSetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSSetShader()
/// \param pComputeShader Parameter for ID3D11DeviceContext::CSSetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::CSSetShader()
/// \param NumClassInstances Parameter for ID3D11DeviceContext::CSSetShader()
void Real_ID3D11DeviceContext_CSSetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::CSSetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSSetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::CSSetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::CSSetSamplers()
void Real_ID3D11DeviceContext_CSSetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::CSSetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::CSSetConstantBuffers()
void Real_ID3D11DeviceContext_CSSetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::VSGetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::VSGetConstantBuffers()
void Real_ID3D11DeviceContext_VSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::PSGetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::PSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::PSGetShaderResources()
void Real_ID3D11DeviceContext_PSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::PSGetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSGetShader()
/// \param ppPixelShader Parameter for ID3D11DeviceContext::PSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::PSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::PSGetShader()
void Real_ID3D11DeviceContext_PSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::PSGetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::PSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::PSGetSamplers()
void Real_ID3D11DeviceContext_PSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::VSGetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSGetShader()
/// \param ppVertexShader Parameter for ID3D11DeviceContext::VSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::VSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::VSGetShader()
void Real_ID3D11DeviceContext_VSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::PSGetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::PSGetConstantBuffers()
void Real_ID3D11DeviceContext_PSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::IAGetInputLayout
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IAGetInputLayout()
/// \param ppInputLayout Parameter for ID3D11DeviceContext::IAGetInputLayout()
void Real_ID3D11DeviceContext_IAGetInputLayout(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11InputLayout** ppInputLayout);

/// Real function wrapper for ID3D11DeviceContext::IAGetVertexBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param ppVertexBuffers Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param pStrides Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
/// \param pOffsets Parameter for ID3D11DeviceContext::IAGetVertexBuffers()
void Real_ID3D11DeviceContext_IAGetVertexBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets);

/// Real function wrapper for ID3D11DeviceContext::IAGetIndexBuffer
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param pIndexBuffer Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param Format Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
/// \param Offset Parameter for ID3D11DeviceContext::IAGetIndexBuffer()
void Real_ID3D11DeviceContext_IAGetIndexBuffer(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset);

/// Real function wrapper for ID3D11DeviceContext::GSGetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::GSGetConstantBuffers()
void Real_ID3D11DeviceContext_GSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::GSGetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSGetShader()
/// \param ppGeometryShader Parameter for ID3D11DeviceContext::GSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::GSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::GSGetShader()
void Real_ID3D11DeviceContext_GSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11GeometryShader** ppGeometryShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::IAGetPrimitiveTopology
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::IAGetPrimitiveTopology()
/// \param pTopology Parameter for ID3D11DeviceContext::IAGetPrimitiveTopology()
void Real_ID3D11DeviceContext_IAGetPrimitiveTopology(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY* pTopology);

/// Real function wrapper for ID3D11DeviceContext::VSGetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::VSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::VSGetShaderResources()
void Real_ID3D11DeviceContext_VSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::VSGetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::VSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::VSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::VSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::VSGetSamplers()
void Real_ID3D11DeviceContext_VSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::GetPredication
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GetPredication()
/// \param ppPredicate Parameter for ID3D11DeviceContext::GetPredication()
/// \param pPredicateValue Parameter for ID3D11DeviceContext::GetPredication()
void Real_ID3D11DeviceContext_GetPredication(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11Predicate** ppPredicate, BOOL* pPredicateValue);

/// Real function wrapper for ID3D11DeviceContext::GSGetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::GSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::GSGetShaderResources()
void Real_ID3D11DeviceContext_GSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::GSGetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::GSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::GSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::GSGetSamplers()
void Real_ID3D11DeviceContext_GSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::OMGetRenderTargets
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMGetRenderTargets()
/// \param NumViews Parameter for ID3D11DeviceContext::OMGetRenderTargets()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMGetRenderTargets()
/// \param ppDepthStencilView Parameter for ID3D11DeviceContext::OMGetRenderTargets()
void Real_ID3D11DeviceContext_OMGetRenderTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView);

/// Real function wrapper for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param NumRTVs Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param ppRenderTargetViews Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param ppDepthStencilView Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param UAVStartSlot Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
void Real_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews);

/// Real function wrapper for ID3D11DeviceContext::OMGetBlendState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMGetBlendState()
/// \param ppBlendState Parameter for ID3D11DeviceContext::OMGetBlendState()
/// \param BlendFactor Parameter for ID3D11DeviceContext::OMGetBlendState()
/// \param pSampleMask Parameter for ID3D11DeviceContext::OMGetBlendState()
void Real_ID3D11DeviceContext_OMGetBlendState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11BlendState** ppBlendState, FLOAT BlendFactor[ 4 ], UINT* pSampleMask);

/// Real function wrapper for ID3D11DeviceContext::OMGetDepthStencilState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::OMGetDepthStencilState()
/// \param ppDepthStencilState Parameter for ID3D11DeviceContext::OMGetDepthStencilState()
/// \param pStencilRef Parameter for ID3D11DeviceContext::OMGetDepthStencilState()
void Real_ID3D11DeviceContext_OMGetDepthStencilState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef);

/// Real function wrapper for ID3D11DeviceContext::SOGetTargets
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::SOGetTargets()
/// \param NumBuffers Parameter for ID3D11DeviceContext::SOGetTargets()
/// \param ppSOTargets Parameter for ID3D11DeviceContext::SOGetTargets()
void Real_ID3D11DeviceContext_SOGetTargets(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer** ppSOTargets);

/// Real function wrapper for ID3D11DeviceContext::RSGetState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::RSGetState()
/// \param ppRasterizerState Parameter for ID3D11DeviceContext::RSGetState()
void Real_ID3D11DeviceContext_RSGetState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11RasterizerState** ppRasterizerState);

/// Real function wrapper for ID3D11DeviceContext::RSGetViewports
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::RSGetViewports()
/// \param pNumViewports Parameter for ID3D11DeviceContext::RSGetViewports()
/// \param pViewports Parameter for ID3D11DeviceContext::RSGetViewports()
void Real_ID3D11DeviceContext_RSGetViewports(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT* pNumViewports, D3D11_VIEWPORT* pViewports);

/// Real function wrapper for ID3D11DeviceContext::RSGetScissorRects
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::RSGetScissorRects()
/// \param pNumRects Parameter for ID3D11DeviceContext::RSGetScissorRects()
/// \param pRects Parameter for ID3D11DeviceContext::RSGetScissorRects()
void Real_ID3D11DeviceContext_RSGetScissorRects(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT* pNumRects, D3D11_RECT* pRects);

/// Real function wrapper for ID3D11DeviceContext::HSGetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::HSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::HSGetShaderResources()
void Real_ID3D11DeviceContext_HSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::HSGetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSGetShader()
/// \param ppHullShader Parameter for ID3D11DeviceContext::HSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::HSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::HSGetShader()
void Real_ID3D11DeviceContext_HSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11HullShader** ppHullShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::HSGetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::HSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::HSGetSamplers()
void Real_ID3D11DeviceContext_HSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::HSGetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::HSGetConstantBuffers()
void Real_ID3D11DeviceContext_HSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::DSGetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::DSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::DSGetShaderResources()
void Real_ID3D11DeviceContext_DSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::DSGetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSGetShader()
/// \param ppDomainShader Parameter for ID3D11DeviceContext::DSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::DSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::DSGetShader()
void Real_ID3D11DeviceContext_DSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11DomainShader** ppDomainShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::DSGetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::DSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::DSGetSamplers()
void Real_ID3D11DeviceContext_DSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::DSGetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::DSGetConstantBuffers()
void Real_ID3D11DeviceContext_DSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::CSGetShaderResources
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSGetShaderResources()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetShaderResources()
/// \param NumViews Parameter for ID3D11DeviceContext::CSGetShaderResources()
/// \param ppShaderResourceViews Parameter for ID3D11DeviceContext::CSGetShaderResources()
void Real_ID3D11DeviceContext_CSGetShaderResources(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews);

/// Real function wrapper for ID3D11DeviceContext::CSGetUnorderedAccessViews
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param NumUAVs Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
/// \param ppUnorderedAccessViews Parameter for ID3D11DeviceContext::CSGetUnorderedAccessViews()
void Real_ID3D11DeviceContext_CSGetUnorderedAccessViews(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews);

/// Real function wrapper for ID3D11DeviceContext::CSGetShader
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSGetShader()
/// \param ppComputeShader Parameter for ID3D11DeviceContext::CSGetShader()
/// \param ppClassInstances Parameter for ID3D11DeviceContext::CSGetShader()
/// \param pNumClassInstances Parameter for ID3D11DeviceContext::CSGetShader()
void Real_ID3D11DeviceContext_CSGetShader(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, ID3D11ComputeShader** ppComputeShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances);

/// Real function wrapper for ID3D11DeviceContext::CSGetSamplers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSGetSamplers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetSamplers()
/// \param NumSamplers Parameter for ID3D11DeviceContext::CSGetSamplers()
/// \param ppSamplers Parameter for ID3D11DeviceContext::CSGetSamplers()
void Real_ID3D11DeviceContext_CSGetSamplers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers);

/// Real function wrapper for ID3D11DeviceContext::CSGetConstantBuffers
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param StartSlot Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param NumBuffers Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
/// \param ppConstantBuffers Parameter for ID3D11DeviceContext::CSGetConstantBuffers()
void Real_ID3D11DeviceContext_CSGetConstantBuffers(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers);

/// Real function wrapper for ID3D11DeviceContext::ClearState
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::ClearState()
void Real_ID3D11DeviceContext_ClearState(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj);

/// Real function wrapper for ID3D11DeviceContext::Flush
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::Flush()
void Real_ID3D11DeviceContext_Flush(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj);

/// Real function wrapper for ID3D11DeviceContext::GetContextFlags
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::GetContextFlags()
UINT Real_ID3D11DeviceContext_GetContextFlags(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj);

/// Real function wrapper for ID3D11DeviceContext::FinishCommandList
/// \param pVTMgr ID3D11DeviceContext VTable Manager
/// \param pObj Parameter for ID3D11DeviceContext::FinishCommandList()
/// \param RestoreDeferredContextState Parameter for ID3D11DeviceContext::FinishCommandList()
/// \param ppCommandList Parameter for ID3D11DeviceContext::FinishCommandList()
HRESULT Real_ID3D11DeviceContext_FinishCommandList(ID3D11DeviceContextVTableManager* pVTMgr, ID3D11DeviceContext* pObj, BOOL RestoreDeferredContextState, ID3D11CommandList** ppCommandList);

/// Real function wrapper for ID3D11Device::CreateBuffer
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateBuffer()
/// \param pDesc Parameter for ID3D11Device::CreateBuffer()
/// \param pInitialData Parameter for ID3D11Device::CreateBuffer()
/// \param ppBuffer Parameter for ID3D11Device::CreateBuffer()
HRESULT Real_ID3D11Device_CreateBuffer(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer);

/// Real function wrapper for ID3D11Device::CreateTexture1D
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateTexture1D()
/// \param pDesc Parameter for ID3D11Device::CreateTexture1D()
/// \param pInitialData Parameter for ID3D11Device::CreateTexture1D()
/// \param ppTexture1D Parameter for ID3D11Device::CreateTexture1D()
HRESULT Real_ID3D11Device_CreateTexture1D(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D);

/// Real function wrapper for ID3D11Device::CreateTexture2D
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateTexture2D()
/// \param pDesc Parameter for ID3D11Device::CreateTexture2D()
/// \param pInitialData Parameter for ID3D11Device::CreateTexture2D()
/// \param ppTexture2D Parameter for ID3D11Device::CreateTexture2D()
HRESULT Real_ID3D11Device_CreateTexture2D(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D);

/// Real function wrapper for ID3D11Device::CreateTexture3D
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateTexture3D()
/// \param pDesc Parameter for ID3D11Device::CreateTexture3D()
/// \param pInitialData Parameter for ID3D11Device::CreateTexture3D()
/// \param ppTexture3D Parameter for ID3D11Device::CreateTexture3D()
HRESULT Real_ID3D11Device_CreateTexture3D(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D);

/// Real function wrapper for ID3D11Device::CreateUnorderedAccessView
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateUnorderedAccessView()
/// \param pResource Parameter for ID3D11Device::CreateUnorderedAccessView()
/// \param pDesc Parameter for ID3D11Device::CreateUnorderedAccessView()
/// \param ppUAView Parameter for ID3D11Device::CreateUnorderedAccessView()
HRESULT Real_ID3D11Device_CreateUnorderedAccessView(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView);

/// Real function wrapper for ID3D11Device::CreateDeferredContext
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateDeferredContext()
/// \param ContextFlags Parameter for ID3D11Device::CreateDeferredContext()
/// \param ppDeferredContext Parameter for ID3D11Device::CreateDeferredContext()
HRESULT Real_ID3D11Device_CreateDeferredContext(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, UINT ContextFlags, ID3D11DeviceContext** ppDeferredContext);

/// Real function wrapper for ID3D11Device::GetImmediateContext
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::GetImmediateContext()
/// \param ppImmediateContext Parameter for ID3D11Device::GetImmediateContext()
void Real_ID3D11Device_GetImmediateContext(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, ID3D11DeviceContext** ppImmediateContext);

/// Real function wrapper for ID3D11Device::CreateComputeShader
/// \param pDeviceVTMgr ID3D11Device VTable Manager
/// \param pObj Parameter for ID3D11Device::CreateComputeShader()
/// \param pShaderBytecode Parameter for ID3D11Device::CreateComputeShader()
/// \param BytecodeLength Parameter for ID3D11Device::CreateComputeShader()
/// \param pClassLinkage Parameter for ID3D11Device::CreateComputeShader()
/// \param ppComputeShader Parameter for ID3D11Device::CreateComputeShader()
HRESULT Real_ID3D11Device_CreateComputeShader(DCID3D11DeviceVTManager* pDeviceVTMgr, ID3D11Device* pObj, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader);

// @}

#endif // _DC_FUNC_DEFS_H_

