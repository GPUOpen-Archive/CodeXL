//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12CoreDeepCopy.h
/// \brief  This file contains DeepCopy utility function implementations.
//=============================================================================

#ifndef DX12COREDEEPCOPY_H
#define DX12COREDEEPCOPY_H

#include "../DX12Defines.h"
#include "../../Common/IUnknownWrapperGUID.h"

//-----------------------------------------------------------------------------
/// A set of utility functions used to create a copy of the incoming structure.
//-----------------------------------------------------------------------------
namespace DX12CoreDeepCopy
{
//-----------------------------------------------------------------------------
/// Unwrap the incoming interface by returning the runtime instance wrapped inside.
/// \param inInterface The wrapped interface to unwrap.
/// \param outInterface The "real" unwrapped runtime interface instance.
//-----------------------------------------------------------------------------
template <class T>
void UnwrapInterface(T* inInterface, T** outInterface)
{
    // Only attempt to unwrap valid interfaces.
    if (inInterface != nullptr)
    {
        T* unwrappedInterface = nullptr;
        HRESULT unwrapResult = reinterpret_cast<IUnknown*>(inInterface)->QueryInterface(IID_IWrappedObject, (void**)&unwrappedInterface);

        if (unwrapResult == S_OK)
        {
            *outInterface = unwrappedInterface;
        }
        else
        {
            // Don't do anything. Just assign the output to the original incoming pointer. It's impossible to unwrap it.
            *outInterface = inInterface;
        }
    }
    else
    {
        // The incoming pointer was null, so don't attempt to unwrap it.
        *outInterface = inInterface;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_COMMAND_QUEUE_DESC* inStruct, D3D12_COMMAND_QUEUE_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_INPUT_ELEMENT_DESC* inStruct, D3D12_INPUT_ELEMENT_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SO_DECLARATION_ENTRY* inStruct, D3D12_SO_DECLARATION_ENTRY* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_VIEWPORT* inStruct, D3D12_VIEWPORT* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_BOX* inStruct, D3D12_BOX* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DEPTH_STENCILOP_DESC* inStruct, D3D12_DEPTH_STENCILOP_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DEPTH_STENCIL_DESC* inStruct, D3D12_DEPTH_STENCIL_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RENDER_TARGET_BLEND_DESC* inStruct, D3D12_RENDER_TARGET_BLEND_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_BLEND_DESC* inStruct, D3D12_BLEND_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RASTERIZER_DESC* inStruct, D3D12_RASTERIZER_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SHADER_BYTECODE* inStruct, D3D12_SHADER_BYTECODE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_STREAM_OUTPUT_DESC* inStruct, D3D12_STREAM_OUTPUT_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_INPUT_LAYOUT_DESC* inStruct, D3D12_INPUT_LAYOUT_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_CACHED_PIPELINE_STATE* inStruct, D3D12_CACHED_PIPELINE_STATE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* inStruct, D3D12_GRAPHICS_PIPELINE_STATE_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_COMPUTE_PIPELINE_STATE_DESC* inStruct, D3D12_COMPUTE_PIPELINE_STATE_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_D3D12_OPTIONS* inStruct, D3D12_FEATURE_DATA_D3D12_OPTIONS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_ARCHITECTURE* inStruct, D3D12_FEATURE_DATA_ARCHITECTURE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_FEATURE_LEVELS* inStruct, D3D12_FEATURE_DATA_FEATURE_LEVELS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_FORMAT_SUPPORT* inStruct, D3D12_FEATURE_DATA_FORMAT_SUPPORT* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS* inStruct, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_FORMAT_INFO* inStruct, D3D12_FEATURE_DATA_FORMAT_INFO* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT* inStruct, D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RESOURCE_ALLOCATION_INFO* inStruct, D3D12_RESOURCE_ALLOCATION_INFO* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_HEAP_PROPERTIES* inStruct, D3D12_HEAP_PROPERTIES* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_HEAP_DESC* inStruct, D3D12_HEAP_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RESOURCE_DESC* inStruct, D3D12_RESOURCE_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DEPTH_STENCIL_VALUE* inStruct, D3D12_DEPTH_STENCIL_VALUE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_CLEAR_VALUE* inStruct, D3D12_CLEAR_VALUE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RANGE* inStruct, D3D12_RANGE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SUBRESOURCE_INFO* inStruct, D3D12_SUBRESOURCE_INFO* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TILED_RESOURCE_COORDINATE* inStruct, D3D12_TILED_RESOURCE_COORDINATE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TILE_REGION_SIZE* inStruct, D3D12_TILE_REGION_SIZE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SUBRESOURCE_TILING* inStruct, D3D12_SUBRESOURCE_TILING* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TILE_SHAPE* inStruct, D3D12_TILE_SHAPE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_PACKED_MIP_INFO* inStruct, D3D12_PACKED_MIP_INFO* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RESOURCE_TRANSITION_BARRIER* inStruct, D3D12_RESOURCE_TRANSITION_BARRIER* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RESOURCE_ALIASING_BARRIER* inStruct, D3D12_RESOURCE_ALIASING_BARRIER* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RESOURCE_UAV_BARRIER* inStruct, D3D12_RESOURCE_UAV_BARRIER* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RESOURCE_BARRIER* inStruct, D3D12_RESOURCE_BARRIER* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SUBRESOURCE_FOOTPRINT* inStruct, D3D12_SUBRESOURCE_FOOTPRINT* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* inStruct, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEXTURE_COPY_LOCATION* inStruct, D3D12_TEXTURE_COPY_LOCATION* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_BUFFER_SRV* inStruct, D3D12_BUFFER_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_SRV* inStruct, D3D12_TEX1D_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_ARRAY_SRV* inStruct, D3D12_TEX1D_ARRAY_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_SRV* inStruct, D3D12_TEX2D_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_ARRAY_SRV* inStruct, D3D12_TEX2D_ARRAY_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX3D_SRV* inStruct, D3D12_TEX3D_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEXCUBE_SRV* inStruct, D3D12_TEXCUBE_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEXCUBE_ARRAY_SRV* inStruct, D3D12_TEXCUBE_ARRAY_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2DMS_SRV* inStruct, D3D12_TEX2DMS_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2DMS_ARRAY_SRV* inStruct, D3D12_TEX2DMS_ARRAY_SRV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SHADER_RESOURCE_VIEW_DESC* inStruct, D3D12_SHADER_RESOURCE_VIEW_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_CONSTANT_BUFFER_VIEW_DESC* inStruct, D3D12_CONSTANT_BUFFER_VIEW_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SAMPLER_DESC* inStruct, D3D12_SAMPLER_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_BUFFER_UAV* inStruct, D3D12_BUFFER_UAV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_UAV* inStruct, D3D12_TEX1D_UAV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_ARRAY_UAV* inStruct, D3D12_TEX1D_ARRAY_UAV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_UAV* inStruct, D3D12_TEX2D_UAV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_ARRAY_UAV* inStruct, D3D12_TEX2D_ARRAY_UAV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX3D_UAV* inStruct, D3D12_TEX3D_UAV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_UNORDERED_ACCESS_VIEW_DESC* inStruct, D3D12_UNORDERED_ACCESS_VIEW_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_BUFFER_RTV* inStruct, D3D12_BUFFER_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_RTV* inStruct, D3D12_TEX1D_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_ARRAY_RTV* inStruct, D3D12_TEX1D_ARRAY_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_RTV* inStruct, D3D12_TEX2D_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2DMS_RTV* inStruct, D3D12_TEX2DMS_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_ARRAY_RTV* inStruct, D3D12_TEX2D_ARRAY_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2DMS_ARRAY_RTV* inStruct, D3D12_TEX2DMS_ARRAY_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX3D_RTV* inStruct, D3D12_TEX3D_RTV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_RENDER_TARGET_VIEW_DESC* inStruct, D3D12_RENDER_TARGET_VIEW_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_DSV* inStruct, D3D12_TEX1D_DSV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX1D_ARRAY_DSV* inStruct, D3D12_TEX1D_ARRAY_DSV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_DSV* inStruct, D3D12_TEX2D_DSV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2D_ARRAY_DSV* inStruct, D3D12_TEX2D_ARRAY_DSV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2DMS_DSV* inStruct, D3D12_TEX2DMS_DSV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_TEX2DMS_ARRAY_DSV* inStruct, D3D12_TEX2DMS_ARRAY_DSV* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DEPTH_STENCIL_VIEW_DESC* inStruct, D3D12_DEPTH_STENCIL_VIEW_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DESCRIPTOR_HEAP_DESC* inStruct, D3D12_DESCRIPTOR_HEAP_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DESCRIPTOR_RANGE* inStruct, D3D12_DESCRIPTOR_RANGE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_ROOT_DESCRIPTOR_TABLE* inStruct, D3D12_ROOT_DESCRIPTOR_TABLE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_ROOT_CONSTANTS* inStruct, D3D12_ROOT_CONSTANTS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_ROOT_DESCRIPTOR* inStruct, D3D12_ROOT_DESCRIPTOR* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_ROOT_PARAMETER* inStruct, D3D12_ROOT_PARAMETER* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_STATIC_SAMPLER_DESC* inStruct, D3D12_STATIC_SAMPLER_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_ROOT_SIGNATURE_DESC* inStruct, D3D12_ROOT_SIGNATURE_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_CPU_DESCRIPTOR_HANDLE* inStruct, D3D12_CPU_DESCRIPTOR_HANDLE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_GPU_DESCRIPTOR_HANDLE* inStruct, D3D12_GPU_DESCRIPTOR_HANDLE* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DISCARD_REGION* inStruct, D3D12_DISCARD_REGION* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_QUERY_HEAP_DESC* inStruct, D3D12_QUERY_HEAP_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_QUERY_DATA_PIPELINE_STATISTICS* inStruct, D3D12_QUERY_DATA_PIPELINE_STATISTICS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_QUERY_DATA_SO_STATISTICS* inStruct, D3D12_QUERY_DATA_SO_STATISTICS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_STREAM_OUTPUT_BUFFER_VIEW* inStruct, D3D12_STREAM_OUTPUT_BUFFER_VIEW* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DRAW_ARGUMENTS* inStruct, D3D12_DRAW_ARGUMENTS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DRAW_INDEXED_ARGUMENTS* inStruct, D3D12_DRAW_INDEXED_ARGUMENTS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_DISPATCH_ARGUMENTS* inStruct, D3D12_DISPATCH_ARGUMENTS* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_VERTEX_BUFFER_VIEW* inStruct, D3D12_VERTEX_BUFFER_VIEW* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_INDEX_BUFFER_VIEW* inStruct, D3D12_INDEX_BUFFER_VIEW* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_INDIRECT_ARGUMENT_DESC* inStruct, D3D12_INDIRECT_ARGUMENT_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_COMMAND_SIGNATURE_DESC* inStruct, D3D12_COMMAND_SIGNATURE_DESC* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_SUBRESOURCE_DATA* inStruct, D3D12_SUBRESOURCE_DATA* outDestination);

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DeepCopy(const D3D12_MEMCPY_DEST* inStruct, D3D12_MEMCPY_DEST* outDestination);
};


#endif // DX12COREDEEPCOPY_H

