//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include <map>
#include "windows.h"
#include "d3d11.h"
#include "DCID3D11DeviceContext_wrapper.h"
#include "..\Common\Logger.h"

class ID3D11DeviceContextWrapper;
static std::map<ID3D11DeviceContext*, ID3D11DeviceContextWrapper*> DevicesBase;     ///< Map from real to wrapper, we maintain this table so that we can look up wrapper through real DeviceContext

//------------------------------------------------------------------------------------
/// This class wraps ID3D11DeviceContext, we patch the vtable of this wrapper
/// instead of ID3D11DeviceContext
//------------------------------------------------------------------------------------
class ID3D11DeviceContextWrapper: public ID3D11DeviceContext
{
public:
    ID3D11DeviceContext* m_pReal;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        return m_pReal->QueryInterface(riid, ppvObject);
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return m_pReal->AddRef();
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG refs = m_pReal->Release();

        // NOTE: We don't delete wrapper even if ref==0, because DX11 runtime allows immediate context to have 0 ref counter
        //if ( refs == 0 )
        //{
        //   DevicesBase.erase( m_pReal );
        //   delete this; //from this point on we cannot access member variables nor call member functions!
        //}

        return refs;
    }

    //-----------------------------------------------------------
    // GetDevice
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice)
    {
        m_pReal->GetDevice(ppDevice);
    }

    //-----------------------------------------------------------
    // GetPrivateData
    //-----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
    {
        return m_pReal->GetPrivateData(guid, pDataSize, pData);
    }

    //-----------------------------------------------------------
    // SetPrivateData
    //-----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
    {
        return m_pReal->SetPrivateData(guid, DataSize, pData);
    }

    //-----------------------------------------------------------
    // SetPrivateDataInterface
    //-----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
    {
        return m_pReal->SetPrivateDataInterface(guid, pData);
    }

    //-----------------------------------------------------------
    // VSSetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pReal->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // PSSetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pReal->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // PSSetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSSetShader(ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pReal->PSSetShader(pPixelShader, ppClassInstances, NumClassInstances);
    }

    //-----------------------------------------------------------
    // PSSetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pReal->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // VSSetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSSetShader(ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pReal->VSSetShader(pVertexShader, ppClassInstances, NumClassInstances);
    }

    //-----------------------------------------------------------
    // DrawIndexed
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
    {
        m_pReal->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
    }

    //-----------------------------------------------------------
    // Draw
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE Draw(UINT VertexCount, UINT StartVertexLocation)
    {
        m_pReal->Draw(VertexCount, StartVertexLocation);
    }

    //-----------------------------------------------------------
    // Map
    //-----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE Map(ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource)
    {
        return m_pReal->Map(pResource, Subresource, MapType, MapFlags, pMappedResource);
    }

    //-----------------------------------------------------------
    // Unmap
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE Unmap(ID3D11Resource* pResource, UINT Subresource)
    {
        m_pReal->Unmap(pResource, Subresource);
    }

    //-----------------------------------------------------------
    // PSSetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pReal->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // IASetInputLayout
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IASetInputLayout(ID3D11InputLayout* pInputLayout)
    {
        m_pReal->IASetInputLayout(pInputLayout);
    }

    //-----------------------------------------------------------
    // IASetVertexBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets)
    {
        m_pReal->IASetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
    }

    //-----------------------------------------------------------
    // IASetIndexBuffer
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IASetIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
    {
        m_pReal->IASetIndexBuffer(pIndexBuffer, Format, Offset);
    }

    //-----------------------------------------------------------
    // DrawIndexedInstanced
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        m_pReal->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }

    //-----------------------------------------------------------
    // DrawInstanced
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
    {
        m_pReal->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    }

    //-----------------------------------------------------------
    // GSSetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pReal->GSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // GSSetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSSetShader(ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pReal->GSSetShader(pShader, ppClassInstances, NumClassInstances);
    }

    //-----------------------------------------------------------
    // IASetPrimitiveTopology
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology)
    {
        m_pReal->IASetPrimitiveTopology(Topology);
    }

    //-----------------------------------------------------------
    // VSSetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pReal->VSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // VSSetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pReal->VSSetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // Begin
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE Begin(ID3D11Asynchronous* pAsync)
    {
        m_pReal->Begin(pAsync);
    }

    //-----------------------------------------------------------
    // End
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE End(ID3D11Asynchronous* pAsync)
    {
        m_pReal->End(pAsync);
    }

    //-----------------------------------------------------------
    // GetData
    //-----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE GetData(ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags)
    {
        return m_pReal->GetData(pAsync, pData, DataSize, GetDataFlags);
    }

    //-----------------------------------------------------------
    // SetPredication
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE SetPredication(ID3D11Predicate* pPredicate, BOOL PredicateValue)
    {
        m_pReal->SetPredication(pPredicate, PredicateValue);
    }

    //-----------------------------------------------------------
    // GSSetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pReal->GSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // GSSetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pReal->GSSetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // OMSetRenderTargets
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMSetRenderTargets(UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView)
    {
        m_pReal->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
    }

    //-----------------------------------------------------------
    // OMSetRenderTargetsAndUnorderedAccessViews
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMSetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
    {
        m_pReal->OMSetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews, pDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
    }

    //-----------------------------------------------------------
    // OMSetBlendState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMSetBlendState(ID3D11BlendState* pBlendState, const FLOAT BlendFactor[ 4 ], UINT SampleMask)
    {
        m_pReal->OMSetBlendState(pBlendState, BlendFactor, SampleMask);
    }

    //-----------------------------------------------------------
    // OMSetDepthStencilState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMSetDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef)
    {
        m_pReal->OMSetDepthStencilState(pDepthStencilState, StencilRef);
    }

    //-----------------------------------------------------------
    // SOSetTargets
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE SOSetTargets(UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets)
    {
        m_pReal->SOSetTargets(NumBuffers, ppSOTargets, pOffsets);
    }

    //-----------------------------------------------------------
    // DrawAuto
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DrawAuto()
    {
        m_pReal->DrawAuto();
    }

    //-----------------------------------------------------------
    // DrawIndexedInstancedIndirect
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DrawIndexedInstancedIndirect(ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        m_pReal->DrawIndexedInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
    }

    //-----------------------------------------------------------
    // DrawInstancedIndirect
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DrawInstancedIndirect(ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        m_pReal->DrawInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
    }

    //-----------------------------------------------------------
    // Dispatch
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
        m_pReal->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    //-----------------------------------------------------------
    // DispatchIndirect
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DispatchIndirect(ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        m_pReal->DispatchIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
    }

    //-----------------------------------------------------------
    // RSSetState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE RSSetState(ID3D11RasterizerState* pRasterizerState)
    {
        m_pReal->RSSetState(pRasterizerState);
    }

    //-----------------------------------------------------------
    // RSSetViewports
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports)
    {
        m_pReal->RSSetViewports(NumViewports, pViewports);
    }

    //-----------------------------------------------------------
    // RSSetScissorRects
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE RSSetScissorRects(UINT NumRects, const D3D11_RECT* pRects)
    {
        m_pReal->RSSetScissorRects(NumRects, pRects);
    }

    //-----------------------------------------------------------
    // CopySubresourceRegion
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CopySubresourceRegion(ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox)
    {
        m_pReal->CopySubresourceRegion(pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
    }

    //-----------------------------------------------------------
    // CopyResource
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource)
    {
        m_pReal->CopyResource(pDstResource, pSrcResource);
    }

    //-----------------------------------------------------------
    // UpdateSubresource
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE UpdateSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
    {
        m_pReal->UpdateSubresource(pDstResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
    }

    //-----------------------------------------------------------
    // CopyStructureCount
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CopyStructureCount(ID3D11Buffer* pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView* pSrcView)
    {
        m_pReal->CopyStructureCount(pDstBuffer, DstAlignedByteOffset, pSrcView);
    }

    //-----------------------------------------------------------
    // ClearRenderTargetView
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[ 4 ])
    {
        m_pReal->ClearRenderTargetView(pRenderTargetView, ColorRGBA);
    }

    //-----------------------------------------------------------
    // ClearUnorderedAccessViewUint
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[ 4 ])
    {
        m_pReal->ClearUnorderedAccessViewUint(pUnorderedAccessView, Values);
    }

    //-----------------------------------------------------------
    // ClearUnorderedAccessViewFloat
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView* pUnorderedAccessView, const FLOAT Values[ 4 ])
    {
        m_pReal->ClearUnorderedAccessViewFloat(pUnorderedAccessView, Values);
    }

    //-----------------------------------------------------------
    // ClearDepthStencilView
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
    {
        m_pReal->ClearDepthStencilView(pDepthStencilView, ClearFlags, Depth, Stencil);
    }

    //-----------------------------------------------------------
    // GenerateMips
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GenerateMips(ID3D11ShaderResourceView* pShaderResourceView)
    {
        m_pReal->GenerateMips(pShaderResourceView);
    }

    //-----------------------------------------------------------
    // SetResourceMinLOD
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE SetResourceMinLOD(ID3D11Resource* pResource, FLOAT MinLOD)
    {
        m_pReal->SetResourceMinLOD(pResource, MinLOD);
    }

    //-----------------------------------------------------------
    // GetResourceMinLOD
    //-----------------------------------------------------------
    FLOAT STDMETHODCALLTYPE GetResourceMinLOD(ID3D11Resource* pResource)
    {
        return m_pReal->GetResourceMinLOD(pResource);
    }

    //-----------------------------------------------------------
    // ResolveSubresource
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ResolveSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
    {
        m_pReal->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
    }

    //-----------------------------------------------------------
    // ExecuteCommandList
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ExecuteCommandList(ID3D11CommandList* pCommandList, BOOL RestoreContextState)
    {
        m_pReal->ExecuteCommandList(pCommandList, RestoreContextState);
    }

    //-----------------------------------------------------------
    // HSSetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pReal->HSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // HSSetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSSetShader(ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pReal->HSSetShader(pHullShader, ppClassInstances, NumClassInstances);
    }

    //-----------------------------------------------------------
    // HSSetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pReal->HSSetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // HSSetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pReal->HSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // DSSetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pReal->DSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // DSSetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSSetShader(ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pReal->DSSetShader(pDomainShader, ppClassInstances, NumClassInstances);
    }

    //-----------------------------------------------------------
    // DSSetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pReal->DSSetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // DSSetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pReal->DSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // CSSetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pReal->CSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // CSSetUnorderedAccessViews
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSSetUnorderedAccessViews(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
    {
        m_pReal->CSSetUnorderedAccessViews(StartSlot, NumUAVs, ppUnorderedAccessViews, pUAVInitialCounts);
    }

    //-----------------------------------------------------------
    // CSSetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSSetShader(ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pReal->CSSetShader(pComputeShader, ppClassInstances, NumClassInstances);
    }

    //-----------------------------------------------------------
    // CSSetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pReal->CSSetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // CSSetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pReal->CSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // VSGetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pReal->VSGetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // PSGetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pReal->PSGetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // PSGetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSGetShader(ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pReal->PSGetShader(ppPixelShader, ppClassInstances, pNumClassInstances);
    }

    //-----------------------------------------------------------
    // PSGetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pReal->PSGetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // VSGetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSGetShader(ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pReal->VSGetShader(ppVertexShader, ppClassInstances, pNumClassInstances);
    }

    //-----------------------------------------------------------
    // PSGetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE PSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pReal->PSGetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // IAGetInputLayout
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IAGetInputLayout(ID3D11InputLayout** ppInputLayout)
    {
        m_pReal->IAGetInputLayout(ppInputLayout);
    }

    //-----------------------------------------------------------
    // IAGetVertexBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets)
    {
        m_pReal->IAGetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
    }

    //-----------------------------------------------------------
    // IAGetIndexBuffer
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IAGetIndexBuffer(ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset)
    {
        m_pReal->IAGetIndexBuffer(pIndexBuffer, Format, Offset);
    }

    //-----------------------------------------------------------
    // GSGetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pReal->GSGetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // GSGetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSGetShader(ID3D11GeometryShader** ppGeometryShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pReal->GSGetShader(ppGeometryShader, ppClassInstances, pNumClassInstances);
    }

    //-----------------------------------------------------------
    // IAGetPrimitiveTopology
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY* pTopology)
    {
        m_pReal->IAGetPrimitiveTopology(pTopology);
    }

    //-----------------------------------------------------------
    // VSGetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pReal->VSGetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // VSGetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE VSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pReal->VSGetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // GetPredication
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GetPredication(ID3D11Predicate** ppPredicate, BOOL* pPredicateValue)
    {
        m_pReal->GetPredication(ppPredicate, pPredicateValue);
    }

    //-----------------------------------------------------------
    // GSGetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pReal->GSGetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // GSGetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE GSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pReal->GSGetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // OMGetRenderTargets
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMGetRenderTargets(UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView)
    {
        m_pReal->OMGetRenderTargets(NumViews, ppRenderTargetViews, ppDepthStencilView);
    }

    //-----------------------------------------------------------
    // OMGetRenderTargetsAndUnorderedAccessViews
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMGetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
    {
        m_pReal->OMGetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews, ppDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews);
    }

    //-----------------------------------------------------------
    // OMGetBlendState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMGetBlendState(ID3D11BlendState** ppBlendState, FLOAT BlendFactor[ 4 ], UINT* pSampleMask)
    {
        m_pReal->OMGetBlendState(ppBlendState, BlendFactor, pSampleMask);
    }

    //-----------------------------------------------------------
    // OMGetDepthStencilState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE OMGetDepthStencilState(ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef)
    {
        m_pReal->OMGetDepthStencilState(ppDepthStencilState, pStencilRef);
    }

    //-----------------------------------------------------------
    // SOGetTargets
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE SOGetTargets(UINT NumBuffers, ID3D11Buffer** ppSOTargets)
    {
        m_pReal->SOGetTargets(NumBuffers, ppSOTargets);
    }

    //-----------------------------------------------------------
    // RSGetState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE RSGetState(ID3D11RasterizerState** ppRasterizerState)
    {
        m_pReal->RSGetState(ppRasterizerState);
    }

    //-----------------------------------------------------------
    // RSGetViewports
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE RSGetViewports(UINT* pNumViewports, D3D11_VIEWPORT* pViewports)
    {
        m_pReal->RSGetViewports(pNumViewports, pViewports);
    }

    //-----------------------------------------------------------
    // RSGetScissorRects
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE RSGetScissorRects(UINT* pNumRects, D3D11_RECT* pRects)
    {
        m_pReal->RSGetScissorRects(pNumRects, pRects);
    }

    //-----------------------------------------------------------
    // HSGetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pReal->HSGetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // HSGetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSGetShader(ID3D11HullShader** ppHullShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pReal->HSGetShader(ppHullShader, ppClassInstances, pNumClassInstances);
    }

    //-----------------------------------------------------------
    // HSGetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pReal->HSGetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // HSGetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE HSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pReal->HSGetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // DSGetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pReal->DSGetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // DSGetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSGetShader(ID3D11DomainShader** ppDomainShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pReal->DSGetShader(ppDomainShader, ppClassInstances, pNumClassInstances);
    }

    //-----------------------------------------------------------
    // DSGetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pReal->DSGetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // DSGetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE DSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pReal->DSGetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // CSGetShaderResources
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pReal->CSGetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
    }

    //-----------------------------------------------------------
    // CSGetUnorderedAccessViews
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSGetUnorderedAccessViews(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
    {
        m_pReal->CSGetUnorderedAccessViews(StartSlot, NumUAVs, ppUnorderedAccessViews);
    }

    //-----------------------------------------------------------
    // CSGetShader
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSGetShader(ID3D11ComputeShader** ppComputeShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pReal->CSGetShader(ppComputeShader, ppClassInstances, pNumClassInstances);
    }

    //-----------------------------------------------------------
    // CSGetSamplers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pReal->CSGetSamplers(StartSlot, NumSamplers, ppSamplers);
    }

    //-----------------------------------------------------------
    // CSGetConstantBuffers
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE CSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pReal->CSGetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
    }

    //-----------------------------------------------------------
    // ClearState
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE ClearState()
    {
        m_pReal->ClearState();
    }

    //-----------------------------------------------------------
    // Flush
    //-----------------------------------------------------------
    void STDMETHODCALLTYPE Flush()
    {
        m_pReal->Flush();
    }

    //-----------------------------------------------------------
    // GetType
    //-----------------------------------------------------------
    D3D11_DEVICE_CONTEXT_TYPE STDMETHODCALLTYPE GetType()
    {
        return m_pReal->GetType();
    }

    //-----------------------------------------------------------
    // GetContextFlags
    //-----------------------------------------------------------
    UINT STDMETHODCALLTYPE GetContextFlags()
    {
        return m_pReal->GetContextFlags();
    }

    //-----------------------------------------------------------
    // FinishCommandList
    //-----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE FinishCommandList(BOOL RestoreDeferredContextState, ID3D11CommandList** ppCommandList)
    {
        return m_pReal->FinishCommandList(RestoreDeferredContextState, ppCommandList);
    }

};

ID3D11DeviceContext* GetWrappedDevice(ID3D11DeviceContext* pReal)
{
    // if the deviceContext is not in our database, return the one that was passed in.
    // otherwise NULL will get returned and could cause the app to crash (BattleForge)
    if (DevicesBase.find(pReal) == DevicesBase.end())
    {
        return pReal;
    }

    return DevicesBase[pReal];
}


ID3D11DeviceContext* WrapDeviceContext(ID3D11DeviceContext* pReal)
{
    ID3D11DeviceContextWrapper* pWrapper = new ID3D11DeviceContextWrapper;
    pWrapper->m_pReal = pReal;

    //SpAssert( DevicesBase[pReal] == NULL );
    std::map<ID3D11DeviceContext*, ID3D11DeviceContextWrapper*>::iterator it = DevicesBase.find(pReal);

    if (it != DevicesBase.end())
    {
        delete it->second;
        DevicesBase.erase(it);
    }

    DevicesBase[pReal] = pWrapper;

    return pWrapper;
}

ID3D11DeviceContext* GetRealDeviceContext11(ID3D11DeviceContext* pWrapped)
{
    for (std::map<ID3D11DeviceContext*, ID3D11DeviceContextWrapper*>::iterator it = DevicesBase.begin(); it != DevicesBase.end(); it++)
    {
        if (it->second == pWrapped)
        {
            return it->first;
        }
    }

    // not found -> not wrapper
    return pWrapped;
}

void CleanupWrappers()
{
    for (std::map<ID3D11DeviceContext*, ID3D11DeviceContextWrapper*>::iterator it = DevicesBase.begin(); it != DevicesBase.end(); it++)
    {
        if (it->second != NULL)
        {
            delete it->second;
        }
    }
}