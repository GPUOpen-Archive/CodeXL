//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute VTable Offsets
//==============================================================================

#ifndef _DC_VTABLE_OFFSETS_H_
#define _DC_VTABLE_OFFSETS_H_

/// \addtogroup DCVirtualTablePatching
// @{

// Enumerations for the DX11 virtual table offsets

// This information is generated from the struct ID3D11DeviceChildVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11DeviceChild
{
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_AddRef,
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_Release,
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DeviceChild_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11DepthStencilStateVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11DepthStencilState
{
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_AddRef,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_Release,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilState_GetDesc,
};

// This information is generated from the struct ID3D11BlendStateVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11BlendState
{
    DX11_VTABLE_OFFSET_ID3D11BlendState_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11BlendState_AddRef,
    DX11_VTABLE_OFFSET_ID3D11BlendState_Release,
    DX11_VTABLE_OFFSET_ID3D11BlendState_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11BlendState_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11BlendState_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11BlendState_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11BlendState_GetDesc,
};

// This information is generated from the struct ID3D11RasterizerStateVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11RasterizerState
{
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_AddRef,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_Release,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11RasterizerState_GetDesc,
};

// This information is generated from the struct ID3D11ResourceVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Resource
{
    DX11_VTABLE_OFFSET_ID3D11Resource_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Resource_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Resource_Release,
    DX11_VTABLE_OFFSET_ID3D11Resource_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Resource_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Resource_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Resource_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Resource_GetType,
    DX11_VTABLE_OFFSET_ID3D11Resource_SetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Resource_GetEvictionPriority,
};

// This information is generated from the struct ID3D11BufferVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Buffer
{
    DX11_VTABLE_OFFSET_ID3D11Buffer_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Buffer_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Buffer_Release,
    DX11_VTABLE_OFFSET_ID3D11Buffer_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Buffer_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Buffer_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Buffer_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Buffer_GetType,
    DX11_VTABLE_OFFSET_ID3D11Buffer_SetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Buffer_GetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Buffer_GetDesc,
};

// This information is generated from the struct ID3D11Texture1DVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Texture1D
{
    DX11_VTABLE_OFFSET_ID3D11Texture1D_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_Release,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_GetType,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_SetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_GetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Texture1D_GetDesc,
};

// This information is generated from the struct ID3D11Texture2DVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Texture2D
{
    DX11_VTABLE_OFFSET_ID3D11Texture2D_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_Release,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_GetType,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_SetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_GetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Texture2D_GetDesc,
};

// This information is generated from the struct ID3D11Texture3DVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Texture3D
{
    DX11_VTABLE_OFFSET_ID3D11Texture3D_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_Release,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_GetType,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_SetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_GetEvictionPriority,
    DX11_VTABLE_OFFSET_ID3D11Texture3D_GetDesc,
};

// This information is generated from the struct ID3D11ViewVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11View
{
    DX11_VTABLE_OFFSET_ID3D11View_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11View_AddRef,
    DX11_VTABLE_OFFSET_ID3D11View_Release,
    DX11_VTABLE_OFFSET_ID3D11View_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11View_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11View_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11View_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11View_GetResource,
};

// This information is generated from the struct ID3D11ShaderResourceViewVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11ShaderResourceView
{
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_AddRef,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_Release,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_GetResource,
    DX11_VTABLE_OFFSET_ID3D11ShaderResourceView_GetDesc,
};

// This information is generated from the struct ID3D11RenderTargetViewVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11RenderTargetView
{
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_AddRef,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_Release,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_GetResource,
    DX11_VTABLE_OFFSET_ID3D11RenderTargetView_GetDesc,
};

// This information is generated from the struct ID3D11DepthStencilViewVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11DepthStencilView
{
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_AddRef,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_Release,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_GetResource,
    DX11_VTABLE_OFFSET_ID3D11DepthStencilView_GetDesc,
};

// This information is generated from the struct ID3D11UnorderedAccessViewVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView
{
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_AddRef,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_Release,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_GetResource,
    DX11_VTABLE_OFFSET_ID3D11UnorderedAccessView_GetDesc,
};

// This information is generated from the struct ID3D11VertexShaderVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11VertexShader
{
    DX11_VTABLE_OFFSET_ID3D11VertexShader_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11VertexShader_AddRef,
    DX11_VTABLE_OFFSET_ID3D11VertexShader_Release,
    DX11_VTABLE_OFFSET_ID3D11VertexShader_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11VertexShader_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11VertexShader_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11VertexShader_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11HullShaderVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11HullShader
{
    DX11_VTABLE_OFFSET_ID3D11HullShader_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11HullShader_AddRef,
    DX11_VTABLE_OFFSET_ID3D11HullShader_Release,
    DX11_VTABLE_OFFSET_ID3D11HullShader_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11HullShader_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11HullShader_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11HullShader_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11DomainShaderVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11DomainShader
{
    DX11_VTABLE_OFFSET_ID3D11DomainShader_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11DomainShader_AddRef,
    DX11_VTABLE_OFFSET_ID3D11DomainShader_Release,
    DX11_VTABLE_OFFSET_ID3D11DomainShader_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11DomainShader_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DomainShader_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DomainShader_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11GeometryShaderVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11GeometryShader
{
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_AddRef,
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_Release,
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11GeometryShader_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11PixelShaderVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11PixelShader
{
    DX11_VTABLE_OFFSET_ID3D11PixelShader_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11PixelShader_AddRef,
    DX11_VTABLE_OFFSET_ID3D11PixelShader_Release,
    DX11_VTABLE_OFFSET_ID3D11PixelShader_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11PixelShader_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11PixelShader_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11PixelShader_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11ComputeShaderVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11ComputeShader
{
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_AddRef,
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_Release,
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ComputeShader_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11InputLayoutVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11InputLayout
{
    DX11_VTABLE_OFFSET_ID3D11InputLayout_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11InputLayout_AddRef,
    DX11_VTABLE_OFFSET_ID3D11InputLayout_Release,
    DX11_VTABLE_OFFSET_ID3D11InputLayout_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11InputLayout_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11InputLayout_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11InputLayout_SetPrivateDataInterface,
};

// This information is generated from the struct ID3D11SamplerStateVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11SamplerState
{
    DX11_VTABLE_OFFSET_ID3D11SamplerState_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_AddRef,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_Release,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11SamplerState_GetDesc,
};

// This information is generated from the struct ID3D11AsynchronousVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Asynchronous
{
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_Release,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Asynchronous_GetDataSize,
};

// This information is generated from the struct ID3D11QueryVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Query
{
    DX11_VTABLE_OFFSET_ID3D11Query_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Query_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Query_Release,
    DX11_VTABLE_OFFSET_ID3D11Query_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Query_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Query_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Query_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Query_GetDataSize,
    DX11_VTABLE_OFFSET_ID3D11Query_GetDesc,
};

// This information is generated from the struct ID3D11PredicateVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Predicate
{
    DX11_VTABLE_OFFSET_ID3D11Predicate_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Predicate_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Predicate_Release,
    DX11_VTABLE_OFFSET_ID3D11Predicate_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Predicate_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Predicate_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Predicate_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Predicate_GetDataSize,
    DX11_VTABLE_OFFSET_ID3D11Predicate_GetDesc,
};

// This information is generated from the struct ID3D11CounterVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Counter
{
    DX11_VTABLE_OFFSET_ID3D11Counter_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Counter_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Counter_Release,
    DX11_VTABLE_OFFSET_ID3D11Counter_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11Counter_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Counter_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Counter_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Counter_GetDataSize,
    DX11_VTABLE_OFFSET_ID3D11Counter_GetDesc,
};

// This information is generated from the struct ID3D11ClassInstanceVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11ClassInstance
{
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_AddRef,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_Release,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_GetClassLinkage,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_GetDesc,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_GetInstanceName,
    DX11_VTABLE_OFFSET_ID3D11ClassInstance_GetTypeName,
};

// This information is generated from the struct ID3D11ClassLinkageVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11ClassLinkage
{
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_AddRef,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_Release,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_GetClassInstance,
    DX11_VTABLE_OFFSET_ID3D11ClassLinkage_CreateClassInstance,
};

// This information is generated from the struct ID3D11CommandListVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11CommandList
{
    DX11_VTABLE_OFFSET_ID3D11CommandList_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11CommandList_AddRef,
    DX11_VTABLE_OFFSET_ID3D11CommandList_Release,
    DX11_VTABLE_OFFSET_ID3D11CommandList_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11CommandList_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11CommandList_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11CommandList_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11CommandList_GetContextFlags,
};

// This information is generated from the struct ID3D11DeviceContextVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11DeviceContext
{
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_AddRef,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Release,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetDevice,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawIndexed,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Draw,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Map,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Unmap,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSSetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetInputLayout,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetVertexBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetIndexBuffer,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawIndexedInstanced,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawInstanced,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IASetPrimitiveTopology,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSSetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Begin,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_End,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetData,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_SetPredication,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSSetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetRenderTargets,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetBlendState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMSetDepthStencilState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_SOSetTargets,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawAuto,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawIndexedInstancedIndirect,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DrawInstancedIndirect,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Dispatch,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DispatchIndirect,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSSetState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSSetViewports,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSSetScissorRects,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CopySubresourceRegion,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CopyResource,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_UpdateSubresource,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CopyStructureCount,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearRenderTargetView,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearUnorderedAccessViewUint,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearUnorderedAccessViewFloat,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearDepthStencilView,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GenerateMips,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_SetResourceMinLOD,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetResourceMinLOD,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ResolveSubresource,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ExecuteCommandList,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSSetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSSetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetUnorderedAccessViews,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSSetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_PSGetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetInputLayout,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetVertexBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetIndexBuffer,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_IAGetPrimitiveTopology,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_VSGetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetPredication,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GSGetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetRenderTargets,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetBlendState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_OMGetDepthStencilState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_SOGetTargets,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSGetState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSGetViewports,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_RSGetScissorRects,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_HSGetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_DSGetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetShaderResources,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetUnorderedAccessViews,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetShader,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetSamplers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_CSGetConstantBuffers,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_ClearState,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_Flush,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetType,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_GetContextFlags,
    DX11_VTABLE_OFFSET_ID3D11DeviceContext_FinishCommandList,
};

// This information is generated from the struct ID3D11DeviceVtbl in D3D11.h
enum DX11_VTABLE_OFFSET_ID3D11Device
{
    DX11_VTABLE_OFFSET_ID3D11Device_QueryInterface,
    DX11_VTABLE_OFFSET_ID3D11Device_AddRef,
    DX11_VTABLE_OFFSET_ID3D11Device_Release,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateBuffer,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture1D,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture2D,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture3D,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateShaderResourceView,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateUnorderedAccessView,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateRenderTargetView,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateDepthStencilView,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateInputLayout,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateVertexShader,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateGeometryShader,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateGeometryShaderWithStreamOutput,
    DX11_VTABLE_OFFSET_ID3D11Device_CreatePixelShader,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateHullShader,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateDomainShader,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateComputeShader,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateClassLinkage,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateBlendState,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateDepthStencilState,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateRasterizerState,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateSamplerState,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateQuery,
    DX11_VTABLE_OFFSET_ID3D11Device_CreatePredicate,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateCounter,
    DX11_VTABLE_OFFSET_ID3D11Device_CreateDeferredContext,
    DX11_VTABLE_OFFSET_ID3D11Device_OpenSharedResource,
    DX11_VTABLE_OFFSET_ID3D11Device_CheckFormatSupport,
    DX11_VTABLE_OFFSET_ID3D11Device_CheckMultisampleQualityLevels,
    DX11_VTABLE_OFFSET_ID3D11Device_CheckCounterInfo,
    DX11_VTABLE_OFFSET_ID3D11Device_CheckCounter,
    DX11_VTABLE_OFFSET_ID3D11Device_CheckFeatureSupport,
    DX11_VTABLE_OFFSET_ID3D11Device_GetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Device_SetPrivateData,
    DX11_VTABLE_OFFSET_ID3D11Device_SetPrivateDataInterface,
    DX11_VTABLE_OFFSET_ID3D11Device_GetFeatureLevel,
    DX11_VTABLE_OFFSET_ID3D11Device_GetCreationFlags,
    DX11_VTABLE_OFFSET_ID3D11Device_GetDeviceRemovedReason,
    DX11_VTABLE_OFFSET_ID3D11Device_GetImmediateContext,
    DX11_VTABLE_OFFSET_ID3D11Device_SetExceptionMode,
    DX11_VTABLE_OFFSET_ID3D11Device_GetExceptionMode,
};

// @}

#endif // _DC_VTABLE_OFFSETS_H_