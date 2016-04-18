//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A set of declarations used to internally track D3D12 API call type,
///         function IDs, and API object types.
//=============================================================================

#ifndef D3D12ENUMERATIONS_H
#define D3D12ENUMERATIONS_H

//-----------------------------------------------------------------------------
/// An API Type enumeration that defines the group that the API call is classified into.
//-----------------------------------------------------------------------------
enum eAPIType
{
    kAPIType_Unknown         = 0,
    kAPIType_BindingCommand  = 0x1,
    kAPIType_ClearCommand    = 0x2,
    kAPIType_Command         = 0x4,
    kAPIType_Copy            = 0x8,
    kAPIType_Create          = 0x10,
    kAPIType_Debug           = 0x20,
    kAPIType_DrawCommand     = 0x40,
    kAPIType_General         = 0x80,
    kAPIType_Paging          = 0x100,
    kAPIType_Resource        = 0x200,
    kAPIType_StageCommand    = 0x400,
    kAPIType_Synchronization = 0x800,
};

//-----------------------------------------------------------------------------
/// A FuncId enumeration that defines all DX12 API calls that can possibly be traced.
//-----------------------------------------------------------------------------
enum FuncId : int
{
    FuncId_UNDEFINED = 0,

    FuncId_D3D12GetDebugInterface,
    FuncId_D3D12CreateDevice,
    FuncId_D3D12SerializeRootSignature,
    FuncId_D3D12CreateRootSignatureDeserializer,

    FuncId_IUnknown_QueryInterface,
    FuncId_IUnknown_AddRef,
    FuncId_IUnknown_Release,

    FuncId_ID3D12Object_GetPrivateData,
    FuncId_ID3D12Object_SetPrivateData,
    FuncId_ID3D12Object_SetPrivateDataInterface,
    FuncId_ID3D12Object_SetName,

    FuncId_ID3D12DeviceChild_GetDevice,

    FuncId_ID3D12RootSignatureDeserializer_GetRootSignatureDesc,

    FuncId_ID3D12Heap_GetDesc,

    FuncId_ID3D12Resource_Map,
    FuncId_ID3D12Resource_Unmap,
    FuncId_ID3D12Resource_GetDesc,
    FuncId_ID3D12Resource_GetGPUVirtualAddress,
    FuncId_ID3D12Resource_WriteToSubresource,
    FuncId_ID3D12Resource_ReadFromSubresource,
    FuncId_ID3D12Resource_GetHeapProperties,

    FuncId_ID3D12CommandAllocator_Reset,

    FuncId_ID3D12Fence_GetCompletedValue,
    FuncId_ID3D12Fence_SetEventOnCompletion,
    FuncId_ID3D12Fence_Signal,

    FuncId_ID3D12PipelineState_GetCachedBlob,

    FuncId_ID3D12DescriptorHeap_GetDesc,
    FuncId_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart,
    FuncId_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart,

    FuncId_ID3D12CommandList_GetType,

    FuncId_ID3D12GraphicsCommandList_Close,
    FuncId_ID3D12GraphicsCommandList_Reset,
    FuncId_ID3D12GraphicsCommandList_ClearState,
    FuncId_ID3D12GraphicsCommandList_DrawInstanced,
    FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced,
    FuncId_ID3D12GraphicsCommandList_Dispatch,
    FuncId_ID3D12GraphicsCommandList_CopyBufferRegion,
    FuncId_ID3D12GraphicsCommandList_CopyTextureRegion,
    FuncId_ID3D12GraphicsCommandList_CopyResource,
    FuncId_ID3D12GraphicsCommandList_CopyTiles,
    FuncId_ID3D12GraphicsCommandList_ResolveSubresource,
    FuncId_ID3D12GraphicsCommandList_IASetPrimitiveTopology,
    FuncId_ID3D12GraphicsCommandList_RSSetViewports,
    FuncId_ID3D12GraphicsCommandList_RSSetScissorRects,
    FuncId_ID3D12GraphicsCommandList_OMSetBlendFactor,
    FuncId_ID3D12GraphicsCommandList_OMSetStencilRef,
    FuncId_ID3D12GraphicsCommandList_SetPipelineState,
    FuncId_ID3D12GraphicsCommandList_ResourceBarrier,
    FuncId_ID3D12GraphicsCommandList_ExecuteBundle,
    FuncId_ID3D12GraphicsCommandList_SetDescriptorHeaps,
    FuncId_ID3D12GraphicsCommandList_SetComputeRootSignature,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRootSignature,
    FuncId_ID3D12GraphicsCommandList_SetComputeRootDescriptorTable,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable,
    FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstant,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant,
    FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstants,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants,
    FuncId_ID3D12GraphicsCommandList_SetComputeRootConstantBufferView,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView,
    FuncId_ID3D12GraphicsCommandList_SetComputeRootShaderResourceView,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView,
    FuncId_ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView,
    FuncId_ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView,
    FuncId_ID3D12GraphicsCommandList_IASetIndexBuffer,
    FuncId_ID3D12GraphicsCommandList_IASetVertexBuffers,
    FuncId_ID3D12GraphicsCommandList_SOSetTargets,
    FuncId_ID3D12GraphicsCommandList_OMSetRenderTargets,
    FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView,
    FuncId_ID3D12GraphicsCommandList_ClearRenderTargetView,
    FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint,
    FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat,
    FuncId_ID3D12GraphicsCommandList_DiscardResource,
    FuncId_ID3D12GraphicsCommandList_BeginQuery,
    FuncId_ID3D12GraphicsCommandList_EndQuery,
    FuncId_ID3D12GraphicsCommandList_ResolveQueryData,
    FuncId_ID3D12GraphicsCommandList_SetPredication,
    FuncId_ID3D12GraphicsCommandList_SetMarker,
    FuncId_ID3D12GraphicsCommandList_BeginEvent,
    FuncId_ID3D12GraphicsCommandList_EndEvent,
    FuncId_ID3D12GraphicsCommandList_ExecuteIndirect,

    FuncId_ID3D12CommandQueue_UpdateTileMappings,
    FuncId_ID3D12CommandQueue_CopyTileMappings,
    FuncId_ID3D12CommandQueue_ExecuteCommandLists,
    FuncId_ID3D12CommandQueue_SetMarker,
    FuncId_ID3D12CommandQueue_BeginEvent,
    FuncId_ID3D12CommandQueue_EndEvent,
    FuncId_ID3D12CommandQueue_Signal,
    FuncId_ID3D12CommandQueue_Wait,
    FuncId_ID3D12CommandQueue_GetTimestampFrequency,
    FuncId_ID3D12CommandQueue_GetClockCalibration,
    FuncId_ID3D12CommandQueue_GetDesc,

    FuncId_ID3D12Device_GetNodeCount,
    FuncId_ID3D12Device_CreateCommandQueue,
    FuncId_ID3D12Device_CreateCommandAllocator,
    FuncId_ID3D12Device_CreateGraphicsPipelineState,
    FuncId_ID3D12Device_CreateComputePipelineState,
    FuncId_ID3D12Device_CreateCommandList,
    FuncId_ID3D12Device_CheckFeatureSupport,
    FuncId_ID3D12Device_CreateDescriptorHeap,
    FuncId_ID3D12Device_GetDescriptorHandleIncrementSize,
    FuncId_ID3D12Device_CreateRootSignature,
    FuncId_ID3D12Device_CreateConstantBufferView,
    FuncId_ID3D12Device_CreateShaderResourceView,
    FuncId_ID3D12Device_CreateUnorderedAccessView,
    FuncId_ID3D12Device_CreateRenderTargetView,
    FuncId_ID3D12Device_CreateDepthStencilView,
    FuncId_ID3D12Device_CreateSampler,
    FuncId_ID3D12Device_CopyDescriptors,
    FuncId_ID3D12Device_CopyDescriptorsSimple,
    FuncId_ID3D12Device_GetResourceAllocationInfo,
    FuncId_ID3D12Device_GetCustomHeapProperties,
    FuncId_ID3D12Device_CreateCommittedResource,
    FuncId_ID3D12Device_CreateHeap,
    FuncId_ID3D12Device_CreatePlacedResource,
    FuncId_ID3D12Device_CreateReservedResource,
    FuncId_ID3D12Device_CreateSharedHandle,
    FuncId_ID3D12Device_OpenSharedHandle,
    FuncId_ID3D12Device_OpenSharedHandleByName,
    FuncId_ID3D12Device_MakeResident,
    FuncId_ID3D12Device_Evict,
    FuncId_ID3D12Device_CreateFence,
    FuncId_ID3D12Device_GetDeviceRemovedReason,
    FuncId_ID3D12Device_GetCopyableFootprints,
    FuncId_ID3D12Device_CreateQueryHeap,
    FuncId_ID3D12Device_SetStablePowerState,
    FuncId_ID3D12Device_CreateCommandSignature,
    FuncId_ID3D12Device_GetResourceTiling,
    FuncId_ID3D12Device_GetAdapterLuid,

    FuncId_IDXGISwapChain_Present,

    FuncId_MAX,
};

//-----------------------------------------------------------------------------
/// An object type enumeration that defines the types of DX12 interfaces that GPS can wrap.
//-----------------------------------------------------------------------------
enum eObjectType
{
    kObjectType_Undefined = -1,

    kObjectType_IUnknown,
    kObjectType_ID3D12Object,
    kObjectType_ID3D12DeviceChild,

    kObjectType_ID3D12RootSignature,
    kObjectType_ID3D12RootSignatureDeserializer,
    kObjectType_ID3D12Pageable,
    kObjectType_ID3D12Heap,
    kObjectType_ID3D12Resource,
    kObjectType_ID3D12CommandAllocator,
    kObjectType_ID3D12Fence,
    kObjectType_ID3D12PipelineState,
    kObjectType_ID3D12DescriptorHeap,
    kObjectType_ID3D12QueryHeap,
    kObjectType_ID3D12CommandSignature,
    kObjectType_ID3D12CommandList,
    kObjectType_ID3D12GraphicsCommandList,
    kObjectType_ID3D12CommandQueue,
    kObjectType_ID3D12Device,

    kObjectType_Begin_Range = kObjectType_ID3D12RootSignature,
    kObjectType_End_Range = kObjectType_ID3D12Device,
    kObjectType_Count = (kObjectType_End_Range - kObjectType_Begin_Range + 1)
};

#endif // D3D12ENUMERATIONS_H