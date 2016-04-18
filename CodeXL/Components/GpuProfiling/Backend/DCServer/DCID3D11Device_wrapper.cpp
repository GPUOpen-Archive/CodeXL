//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include <map>
#include <d3d11.h>
#include "DCID3D11Device_wrapper.h"
#include "..\Common\Logger.h"

class ID3D11DeviceWrapper;
static std::map<ID3D11Device*, ID3D11DeviceWrapper*> DevicesBase;     ///< Map from real to wrapper, we maintain this table so that we can look up wrapper through real Device

//------------------------------------------------------------------------------------
/// This class wraps ID3D11Device, we patch the vtable of this wrapper
/// instead of ID3D11Device
//------------------------------------------------------------------------------------
class ID3D11DeviceWrapper: public ID3D11Device
{
public:
    ID3D11Device* m_pReal;  ///< Real pointer

    /// QueryInterface ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        return m_pReal->QueryInterface(riid, ppvObject);
    }

    /// AddRef ID3D11Deivce interface
    ULONG STDMETHODCALLTYPE AddRef()
    {
        return m_pReal->AddRef();
    }

    /// Release ID3D11Deivce interface
    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG refs = m_pReal->Release();

        if (refs == 0)
        {
            DevicesBase.erase(m_pReal);
            delete this; //from this point on we cannot access member variables nor call member functions!
        }

        return refs;
    }

    /// CreateBuffer ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer)
    {
        return m_pReal->CreateBuffer(pDesc, pInitialData, ppBuffer);
    }

    /// CreateTexture1D ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D)
    {
        return m_pReal->CreateTexture1D(pDesc, pInitialData, ppTexture1D);
    }

    /// CreateTexture2D ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
    {
        return m_pReal->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
    }

    /// CreateTexture3D ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D)
    {
        return m_pReal->CreateTexture3D(pDesc, pInitialData, ppTexture3D);
    }

    /// CreateShaderResourceView ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView)
    {
        return m_pReal->CreateShaderResourceView(pResource, pDesc, ppSRView);
    }

    /// CreateUnorderedAccessView ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
    {
        return m_pReal->CreateUnorderedAccessView(pResource, pDesc, ppUAView);
    }

    /// CreateRenderTargetView ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView)
    {
        return m_pReal->CreateRenderTargetView(pResource, pDesc, ppRTView);
    }

    /// CreateDepthStencilView ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView)
    {
        return m_pReal->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
    }

    /// CreateInputLayout ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout)
    {
        return m_pReal->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);
    }

    /// CreateVertexShader ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
    {
        return m_pReal->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
    }

    /// CreateGeometryShader ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateGeometryShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
    {
        return m_pReal->CreateGeometryShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppGeometryShader);
    }

    /// CreateGeometryShaderWithStreamOutput ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput(const void* pShaderBytecode, SIZE_T BytecodeLength, const D3D11_SO_DECLARATION_ENTRY* pSODeclaration, UINT NumEntries, const UINT* pBufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
    {
        return m_pReal->CreateGeometryShaderWithStreamOutput(pShaderBytecode, BytecodeLength, pSODeclaration, NumEntries, pBufferStrides, NumStrides, RasterizedStream, pClassLinkage, ppGeometryShader);
    }

    /// CreatePixelShader ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader)
    {
        return m_pReal->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
    }

    /// CreateHullShader ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateHullShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader)
    {
        return m_pReal->CreateHullShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppHullShader);
    }

    /// CreateDomainShader ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateDomainShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader)
    {
        return m_pReal->CreateDomainShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppDomainShader);
    }

    /// CreateComputeShader ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateComputeShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
    {
        return m_pReal->CreateComputeShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader);
    }

    /// CreateClassLinkage ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateClassLinkage(ID3D11ClassLinkage** ppLinkage)
    {
        return m_pReal->CreateClassLinkage(ppLinkage);
    }

    /// CreateBlendState ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState)
    {
        return m_pReal->CreateBlendState(pBlendStateDesc, ppBlendState);
    }

    /// CreateDepthStencilState ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState)
    {
        return m_pReal->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState);
    }

    /// CreateRasterizerState ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState)
    {
        return m_pReal->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
    }

    /// CreateSamplerState ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState)
    {
        return m_pReal->CreateSamplerState(pSamplerDesc, ppSamplerState);
    }

    /// CreateQuery ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateQuery(const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery)
    {
        return m_pReal->CreateQuery(pQueryDesc, ppQuery);
    }

    /// CreatePredicate ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreatePredicate(const D3D11_QUERY_DESC* pPredicateDesc, ID3D11Predicate** ppPredicate)
    {
        return m_pReal->CreatePredicate(pPredicateDesc, ppPredicate);
    }

    /// CreateCounter ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateCounter(const D3D11_COUNTER_DESC* pCounterDesc, ID3D11Counter** ppCounter)
    {
        return m_pReal->CreateCounter(pCounterDesc, ppCounter);
    }

    /// CreateDeferredContext ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CreateDeferredContext(UINT ContextFlags, ID3D11DeviceContext** ppDeferredContext)
    {
        return m_pReal->CreateDeferredContext(ContextFlags, ppDeferredContext);
    }

    /// OpenSharedResource ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE OpenSharedResource(HANDLE hResource, REFIID ReturnedInterface, void** ppResource)
    {
        return m_pReal->OpenSharedResource(hResource, ReturnedInterface, ppResource);
    }

    /// CheckFormatSupport ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CheckFormatSupport(DXGI_FORMAT Format, UINT* pFormatSupport)
    {
        return m_pReal->CheckFormatSupport(Format, pFormatSupport);
    }

    /// CheckMultisampleQualityLevels ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CheckMultisampleQualityLevels(DXGI_FORMAT Format, UINT SampleCount, UINT* pNumQualityLevels)
    {
        return m_pReal->CheckMultisampleQualityLevels(Format, SampleCount, pNumQualityLevels);
    }

    /// CheckCounterInfo ID3D11Deivce interface
    void STDMETHODCALLTYPE CheckCounterInfo(D3D11_COUNTER_INFO* pCounterInfo)
    {
        m_pReal->CheckCounterInfo(pCounterInfo);
    }

    /// CheckCounter ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CheckCounter(const D3D11_COUNTER_DESC* pDesc, D3D11_COUNTER_TYPE* pType, UINT* pActiveCounters, LPSTR szName, UINT* pNameLength, LPSTR szUnits, UINT* pUnitsLength, LPSTR szDescription, UINT* pDescriptionLength)
    {
        return m_pReal->CheckCounter(pDesc, pType, pActiveCounters, szName, pNameLength, szUnits, pUnitsLength, szDescription, pDescriptionLength);
    }

    /// CheckFeatureSupport ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE CheckFeatureSupport(D3D11_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize)
    {
        return m_pReal->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
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

    /// GetFeatureLevel ID3D11Deivce interface
    D3D_FEATURE_LEVEL STDMETHODCALLTYPE GetFeatureLevel(void)
    {
        return m_pReal->GetFeatureLevel();
    }

    /// GetCreationFlags ID3D11Deivce interface
    UINT STDMETHODCALLTYPE GetCreationFlags(void)
    {
        return m_pReal->GetCreationFlags();
    }

    /// GetDeviceRemovedReason ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason(void)
    {
        return m_pReal->GetDeviceRemovedReason();
    }

    /// GetImmediateContext ID3D11Deivce interface
    void STDMETHODCALLTYPE GetImmediateContext(ID3D11DeviceContext** ppImmediateContext)
    {
        m_pReal->GetImmediateContext(ppImmediateContext);
    }

    /// SetExceptionMode ID3D11Deivce interface
    HRESULT STDMETHODCALLTYPE SetExceptionMode(UINT RaiseFlags)
    {
        return m_pReal->SetExceptionMode(RaiseFlags);
    }

    /// GetExceptionMode ID3D11Deivce interface
    UINT STDMETHODCALLTYPE GetExceptionMode(void)
    {
        return m_pReal->GetExceptionMode();
    }
};


ID3D11Device* GetWrappedDevice(ID3D11Device* pReal)
{
    // if the deviceContext is not in our database, return the one that was passed in.
    // otherwise NULL will get returned and could cause the app to crash (BattleForge)
    if (DevicesBase.find(pReal) == DevicesBase.end())
    {
        return pReal;
    }

    return DevicesBase[pReal];
}


ID3D11Device* WrapDevice(ID3D11Device* pReal)
{
    ID3D11DeviceWrapper* pWrapper = new ID3D11DeviceWrapper;
    pWrapper->m_pReal = pReal;

    //SpAssert( DevicesBase[pReal] == NULL );
    std::map<ID3D11Device*, ID3D11DeviceWrapper*>::iterator it = DevicesBase.find(pReal);

    if (it != DevicesBase.end())
    {
        delete it->second;
        DevicesBase.erase(it);
    }

    DevicesBase[pReal] = pWrapper;

    return pWrapper;
}

ID3D11Device* GetRealDevice11(ID3D11Device* pWrapped)
{
    for (std::map<ID3D11Device*, ID3D11DeviceWrapper*>::iterator it = DevicesBase.begin(); it != DevicesBase.end(); it++)
    {
        if (it->second == pWrapped)
        {
            return it->first;
        }
    }

    // not found -> not wrapper
    return pWrapped;
}

