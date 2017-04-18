

// Local:
#include "DX12FunctionDefs.h"


//--------------------------------------------------------------------------
/// Retrieve the API Type that an API call has been classified into.
/// \param inAPIFuncId The FunctionId of an API call to retrieve the group for.
/// \returns An API Type that a call has been classified as being part of.
//--------------------------------------------------------------------------
eAPIType DX12FunctionDefs::GetAPIGroupFromAPI(FuncId inAPIFuncId)
{
    eAPIType apiType = kAPIType_Unknown;

    switch (inAPIFuncId)
    {
        case FuncId_ID3D12GraphicsCommandList_SetDescriptorHeaps:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootSignature:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootSignature:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootDescriptorTable:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstant:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstants:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootConstantBufferView:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootShaderResourceView:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView:
            apiType = kAPIType_BindingCommand;
            break;

        case FuncId_ID3D12GraphicsCommandList_ClearState:
        case FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView:
        case FuncId_ID3D12GraphicsCommandList_ClearRenderTargetView:
        case FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint:
        case FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat:
            apiType = kAPIType_ClearCommand;
            break;

        case FuncId_ID3D12GraphicsCommandList_Close:
        case FuncId_ID3D12GraphicsCommandList_Reset:
        case FuncId_ID3D12GraphicsCommandList_ResolveSubresource:
        case FuncId_ID3D12GraphicsCommandList_SetPipelineState:
        case FuncId_ID3D12GraphicsCommandList_ExecuteBundle:
        case FuncId_ID3D12GraphicsCommandList_DiscardResource:
        case FuncId_ID3D12GraphicsCommandList_BeginQuery:
        case FuncId_ID3D12GraphicsCommandList_EndQuery:
        case FuncId_ID3D12GraphicsCommandList_ResolveQueryData:
        case FuncId_ID3D12GraphicsCommandList_SetPredication:
        case FuncId_ID3D12GraphicsCommandList_ExecuteIndirect:
            apiType = kAPIType_Command;
            break;

        case FuncId_ID3D12CommandQueue_CopyTileMappings:
        case FuncId_ID3D12Device_CopyDescriptors:
        case FuncId_ID3D12Device_CopyDescriptorsSimple:
            apiType = kAPIType_Copy;
            break;

        case FuncId_ID3D12Device_CreateCommandQueue:
        case FuncId_ID3D12Device_CreateCommandAllocator:
        case FuncId_ID3D12Device_CreateGraphicsPipelineState:
        case FuncId_ID3D12Device_CreateComputePipelineState:
        case FuncId_ID3D12Device_CreateCommandList:
        case FuncId_ID3D12Device_CreateDescriptorHeap:
        case FuncId_ID3D12Device_CreateRootSignature:
        case FuncId_ID3D12Device_CreateConstantBufferView:
        case FuncId_ID3D12Device_CreateShaderResourceView:
        case FuncId_ID3D12Device_CreateUnorderedAccessView:
        case FuncId_ID3D12Device_CreateRenderTargetView:
        case FuncId_ID3D12Device_CreateDepthStencilView:
        case FuncId_ID3D12Device_CreateSampler:
        case FuncId_ID3D12Device_CreateHeap:
        case FuncId_ID3D12Device_CreatePlacedResource:
        case FuncId_ID3D12Device_CreateReservedResource:
        case FuncId_ID3D12Device_CreateSharedHandle:
        case FuncId_ID3D12Device_CreateFence:
        case FuncId_ID3D12Device_CreateQueryHeap:
        case FuncId_ID3D12Device_CreateCommandSignature:
            apiType = kAPIType_Create;
            break;

        case FuncId_ID3D12Object_GetPrivateData:
        case FuncId_ID3D12Object_SetPrivateData:
        case FuncId_ID3D12Object_SetPrivateDataInterface:
        case FuncId_ID3D12Object_SetName:
        case FuncId_ID3D12GraphicsCommandList_SetMarker:
        case FuncId_ID3D12GraphicsCommandList_BeginEvent:
        case FuncId_ID3D12GraphicsCommandList_EndEvent:
        case FuncId_ID3D12CommandQueue_BeginEvent:
        case FuncId_ID3D12CommandQueue_EndEvent:
        case FuncId_ID3D12CommandQueue_SetMarker:
            apiType = kAPIType_Debug;
            break;

        case FuncId_ID3D12GraphicsCommandList_DrawInstanced:
        case FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced:
        case FuncId_ID3D12GraphicsCommandList_Dispatch:
            apiType = kAPIType_DrawCommand;
            break;

        case FuncId_IUnknown_QueryInterface:
        case FuncId_IUnknown_AddRef:
        case FuncId_IUnknown_Release:
        case FuncId_ID3D12DeviceChild_GetDevice:
        case FuncId_ID3D12Heap_GetDesc:
        case FuncId_ID3D12RootSignatureDeserializer_GetRootSignatureDesc:
        case FuncId_ID3D12Resource_GetHeapProperties:
        case FuncId_ID3D12CommandAllocator_Reset:
        case FuncId_ID3D12PipelineState_GetCachedBlob:
        case FuncId_ID3D12DescriptorHeap_GetDesc:
        case FuncId_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart:
        case FuncId_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart:
        case FuncId_ID3D12CommandList_GetType:
        case FuncId_ID3D12CommandQueue_ExecuteCommandLists:
        case FuncId_ID3D12CommandQueue_GetTimestampFrequency:
        case FuncId_ID3D12CommandQueue_GetClockCalibration:
        case FuncId_ID3D12CommandQueue_GetDesc:
        case FuncId_ID3D12Device_GetNodeCount:
        case FuncId_ID3D12Device_CheckFeatureSupport:
        case FuncId_ID3D12Device_GetDescriptorHandleIncrementSize:
        case FuncId_ID3D12Device_GetResourceAllocationInfo:
        case FuncId_ID3D12Device_GetCustomHeapProperties:
        case FuncId_ID3D12Device_OpenSharedHandle:
        case FuncId_ID3D12Device_OpenSharedHandleByName:
        case FuncId_ID3D12Device_GetDeviceRemovedReason:
        case FuncId_ID3D12Device_GetCopyableFootprints:
        case FuncId_ID3D12Device_SetStablePowerState:
        case FuncId_ID3D12Device_GetResourceTiling:
        case FuncId_ID3D12Device_GetAdapterLuid:
            apiType = kAPIType_General;
            break;

        case FuncId_ID3D12CommandQueue_UpdateTileMappings:
        case FuncId_ID3D12Device_MakeResident:
        case FuncId_ID3D12Device_Evict:
            apiType = kAPIType_Paging;
            break;

        case FuncId_ID3D12Resource_Map:
        case FuncId_ID3D12Resource_Unmap:
        case FuncId_ID3D12Resource_GetDesc:
        case FuncId_ID3D12Resource_GetGPUVirtualAddress:
        case FuncId_ID3D12Resource_ReadFromSubresource:
            apiType = kAPIType_Resource;
            break;

        case FuncId_ID3D12GraphicsCommandList_IASetPrimitiveTopology:
        case FuncId_ID3D12GraphicsCommandList_RSSetViewports:
        case FuncId_ID3D12GraphicsCommandList_RSSetScissorRects:
        case FuncId_ID3D12GraphicsCommandList_OMSetBlendFactor:
        case FuncId_ID3D12GraphicsCommandList_OMSetStencilRef:
        case FuncId_ID3D12GraphicsCommandList_IASetIndexBuffer:
        case FuncId_ID3D12GraphicsCommandList_IASetVertexBuffers:
        case FuncId_ID3D12GraphicsCommandList_SOSetTargets:
        case FuncId_ID3D12GraphicsCommandList_OMSetRenderTargets:
            apiType = kAPIType_StageCommand;
            break;

        case FuncId_ID3D12Fence_GetCompletedValue:
        case FuncId_ID3D12Fence_SetEventOnCompletion:
        case FuncId_ID3D12Fence_Signal:
        case FuncId_ID3D12GraphicsCommandList_ResourceBarrier:
        case FuncId_ID3D12CommandQueue_Signal:
        case FuncId_ID3D12CommandQueue_Wait:
            apiType = kAPIType_Synchronization;
            break;


        default:
            apiType = kAPIType_Unknown;
    }

    return apiType;
}