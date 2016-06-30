//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12CoreDeepCopy.cpp
/// \brief  This file contains DeepCopy utility function implementations.
//=============================================================================

#include "DX12CoreDeepCopy.h"
#include "../DX12Defines.h"
#include "../Objects/IDX12InstanceBase.h"
#include "../Objects/DX12ObjectDatabaseProcessor.h"

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_COMMAND_QUEUE_DESC* inStruct, D3D12_COMMAND_QUEUE_DESC* outDestination)
{
    outDestination->Type = inStruct->Type;
    memcpy(&(outDestination->Priority), &(inStruct->Priority), sizeof(INT));
    outDestination->Flags = inStruct->Flags;
    memcpy(&(outDestination->NodeMask), &(inStruct->NodeMask), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_INPUT_ELEMENT_DESC* inStruct, D3D12_INPUT_ELEMENT_DESC* outDestination)
{
    memcpy(&(outDestination->SemanticName), &(inStruct->SemanticName), sizeof(LPCSTR));
    memcpy(&(outDestination->SemanticIndex), &(inStruct->SemanticIndex), sizeof(UINT));
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    memcpy(&(outDestination->InputSlot), &(inStruct->InputSlot), sizeof(UINT));
    memcpy(&(outDestination->AlignedByteOffset), &(inStruct->AlignedByteOffset), sizeof(UINT));
    outDestination->InputSlotClass = inStruct->InputSlotClass;
    memcpy(&(outDestination->InstanceDataStepRate), &(inStruct->InstanceDataStepRate), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SO_DECLARATION_ENTRY* inStruct, D3D12_SO_DECLARATION_ENTRY* outDestination)
{
    memcpy(&(outDestination->Stream), &(inStruct->Stream), sizeof(UINT));
    memcpy(&(outDestination->SemanticName), &(inStruct->SemanticName), sizeof(LPCSTR));
    memcpy(&(outDestination->SemanticIndex), &(inStruct->SemanticIndex), sizeof(UINT));
    memcpy(&(outDestination->StartComponent), &(inStruct->StartComponent), sizeof(BYTE));
    memcpy(&(outDestination->ComponentCount), &(inStruct->ComponentCount), sizeof(BYTE));
    memcpy(&(outDestination->OutputSlot), &(inStruct->OutputSlot), sizeof(BYTE));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_VIEWPORT* inStruct, D3D12_VIEWPORT* outDestination)
{
    memcpy(&(outDestination->TopLeftX), &(inStruct->TopLeftX), sizeof(FLOAT));
    memcpy(&(outDestination->TopLeftY), &(inStruct->TopLeftY), sizeof(FLOAT));
    memcpy(&(outDestination->Width), &(inStruct->Width), sizeof(FLOAT));
    memcpy(&(outDestination->Height), &(inStruct->Height), sizeof(FLOAT));
    memcpy(&(outDestination->MinDepth), &(inStruct->MinDepth), sizeof(FLOAT));
    memcpy(&(outDestination->MaxDepth), &(inStruct->MaxDepth), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_BOX* inStruct, D3D12_BOX* outDestination)
{
    memcpy(&(outDestination->left), &(inStruct->left), sizeof(UINT));
    memcpy(&(outDestination->top), &(inStruct->top), sizeof(UINT));
    memcpy(&(outDestination->front), &(inStruct->front), sizeof(UINT));
    memcpy(&(outDestination->right), &(inStruct->right), sizeof(UINT));
    memcpy(&(outDestination->bottom), &(inStruct->bottom), sizeof(UINT));
    memcpy(&(outDestination->back), &(inStruct->back), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DEPTH_STENCILOP_DESC* inStruct, D3D12_DEPTH_STENCILOP_DESC* outDestination)
{
    outDestination->StencilFailOp = inStruct->StencilFailOp;
    outDestination->StencilDepthFailOp = inStruct->StencilDepthFailOp;
    outDestination->StencilPassOp = inStruct->StencilPassOp;
    outDestination->StencilFunc = inStruct->StencilFunc;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DEPTH_STENCIL_DESC* inStruct, D3D12_DEPTH_STENCIL_DESC* outDestination)
{
    memcpy(&(outDestination->DepthEnable), &(inStruct->DepthEnable), sizeof(BOOL));
    outDestination->DepthWriteMask = inStruct->DepthWriteMask;
    outDestination->DepthFunc = inStruct->DepthFunc;
    memcpy(&(outDestination->StencilEnable), &(inStruct->StencilEnable), sizeof(BOOL));
    memcpy(&(outDestination->StencilReadMask), &(inStruct->StencilReadMask), sizeof(UINT8));
    memcpy(&(outDestination->StencilWriteMask), &(inStruct->StencilWriteMask), sizeof(UINT8));
    DeepCopy(&(inStruct->FrontFace), &(outDestination->FrontFace));
    DeepCopy(&(inStruct->BackFace), &(outDestination->BackFace));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RENDER_TARGET_BLEND_DESC* inStruct, D3D12_RENDER_TARGET_BLEND_DESC* outDestination)
{
    memcpy(&(outDestination->BlendEnable), &(inStruct->BlendEnable), sizeof(BOOL));
    memcpy(&(outDestination->LogicOpEnable), &(inStruct->LogicOpEnable), sizeof(BOOL));
    outDestination->SrcBlend = inStruct->SrcBlend;
    outDestination->DestBlend = inStruct->DestBlend;
    outDestination->BlendOp = inStruct->BlendOp;
    outDestination->SrcBlendAlpha = inStruct->SrcBlendAlpha;
    outDestination->DestBlendAlpha = inStruct->DestBlendAlpha;
    outDestination->BlendOpAlpha = inStruct->BlendOpAlpha;
    outDestination->LogicOp = inStruct->LogicOp;
    memcpy(&(outDestination->RenderTargetWriteMask), &(inStruct->RenderTargetWriteMask), sizeof(UINT8));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_BLEND_DESC* inStruct, D3D12_BLEND_DESC* outDestination)
{
    memcpy(&(outDestination->AlphaToCoverageEnable), &(inStruct->AlphaToCoverageEnable), sizeof(BOOL));
    memcpy(&(outDestination->IndependentBlendEnable), &(inStruct->IndependentBlendEnable), sizeof(BOOL));

    for (UINT index = 0; index < 8; index++)
    {
        DX12CoreDeepCopy::DeepCopy(&(inStruct->RenderTarget[index]), &(outDestination->RenderTarget[index]));
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RASTERIZER_DESC* inStruct, D3D12_RASTERIZER_DESC* outDestination)
{
    outDestination->FillMode = inStruct->FillMode;
    outDestination->CullMode = inStruct->CullMode;
    memcpy(&(outDestination->FrontCounterClockwise), &(inStruct->FrontCounterClockwise), sizeof(BOOL));
    memcpy(&(outDestination->DepthBias), &(inStruct->DepthBias), sizeof(INT));
    memcpy(&(outDestination->DepthBiasClamp), &(inStruct->DepthBiasClamp), sizeof(FLOAT));
    memcpy(&(outDestination->SlopeScaledDepthBias), &(inStruct->SlopeScaledDepthBias), sizeof(FLOAT));
    memcpy(&(outDestination->DepthClipEnable), &(inStruct->DepthClipEnable), sizeof(BOOL));
    memcpy(&(outDestination->MultisampleEnable), &(inStruct->MultisampleEnable), sizeof(BOOL));
    memcpy(&(outDestination->AntialiasedLineEnable), &(inStruct->AntialiasedLineEnable), sizeof(BOOL));
    memcpy(&(outDestination->ForcedSampleCount), &(inStruct->ForcedSampleCount), sizeof(UINT));
    outDestination->ConservativeRaster = inStruct->ConservativeRaster;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SHADER_BYTECODE* inStruct, D3D12_SHADER_BYTECODE* outDestination)
{
    char* byteCodeCopy = nullptr;

    if (inStruct->BytecodeLength > 0)
    {
        byteCodeCopy = new char[inStruct->BytecodeLength];
        memcpy(byteCodeCopy, inStruct->pShaderBytecode, sizeof(char) * inStruct->BytecodeLength);
    }

    outDestination->pShaderBytecode = byteCodeCopy;
    outDestination->BytecodeLength = inStruct->BytecodeLength;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_STREAM_OUTPUT_DESC* inStruct, D3D12_STREAM_OUTPUT_DESC* outDestination)
{
    D3D12_SO_DECLARATION_ENTRY* pSODeclarationCopy = nullptr;

    if (inStruct->NumEntries > 0)
    {
        pSODeclarationCopy = new D3D12_SO_DECLARATION_ENTRY[inStruct->NumEntries];

        for (UINT index = 0; index < inStruct->NumEntries; index++)
        {
            DeepCopy(&(inStruct->pSODeclaration[index]), &(pSODeclarationCopy[index]));
        }
    }

    outDestination->pSODeclaration = pSODeclarationCopy;
    memcpy(&(outDestination->NumEntries), &(inStruct->NumEntries), sizeof(UINT));
    UINT* pBufferStridesCopy = nullptr;

    if (inStruct->NumStrides > 0)
    {
        pBufferStridesCopy = new UINT[inStruct->NumStrides];

        for (UINT index = 0; index < inStruct->NumStrides; index++)
        {
            pBufferStridesCopy[index] = inStruct->pBufferStrides[index];
        }
    }

    outDestination->pBufferStrides = pBufferStridesCopy;
    memcpy(&(outDestination->NumStrides), &(inStruct->NumStrides), sizeof(UINT));
    memcpy(&(outDestination->RasterizedStream), &(inStruct->RasterizedStream), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_INPUT_LAYOUT_DESC* inStruct, D3D12_INPUT_LAYOUT_DESC* outDestination)
{
    D3D12_INPUT_ELEMENT_DESC* pInputElementDescsCopy = nullptr;

    if (inStruct->NumElements > 0)
    {
        pInputElementDescsCopy = new D3D12_INPUT_ELEMENT_DESC[inStruct->NumElements];

        for (UINT index = 0; index < inStruct->NumElements; index++)
        {
            DeepCopy(&(inStruct->pInputElementDescs[index]), &(pInputElementDescsCopy[index]));
        }
    }

    outDestination->pInputElementDescs = pInputElementDescsCopy;
    memcpy(&(outDestination->NumElements), &(inStruct->NumElements), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_CACHED_PIPELINE_STATE* inStruct, D3D12_CACHED_PIPELINE_STATE* outDestination)
{
    char* blobCopy = nullptr;

    if (inStruct->CachedBlobSizeInBytes > 0)
    {
        blobCopy = new char[inStruct->CachedBlobSizeInBytes];
        memcpy(blobCopy, inStruct->pCachedBlob, sizeof(char) * inStruct->CachedBlobSizeInBytes);
    }

    outDestination->pCachedBlob = blobCopy;
    memcpy(&(outDestination->CachedBlobSizeInBytes), &(inStruct->CachedBlobSizeInBytes), sizeof(SIZE_T));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* inStruct, D3D12_GRAPHICS_PIPELINE_STATE_DESC* outDestination)
{
    UnwrapInterface(inStruct->pRootSignature, &(outDestination->pRootSignature));
    DeepCopy(&(inStruct->VS), &(outDestination->VS));
    DeepCopy(&(inStruct->PS), &(outDestination->PS));
    DeepCopy(&(inStruct->DS), &(outDestination->DS));
    DeepCopy(&(inStruct->HS), &(outDestination->HS));
    DeepCopy(&(inStruct->GS), &(outDestination->GS));
    DeepCopy(&(inStruct->StreamOutput), &(outDestination->StreamOutput));
    DeepCopy(&(inStruct->BlendState), &(outDestination->BlendState));
    memcpy(&(outDestination->SampleMask), &(inStruct->SampleMask), sizeof(UINT));
    DeepCopy(&(inStruct->RasterizerState), &(outDestination->RasterizerState));
    DeepCopy(&(inStruct->DepthStencilState), &(outDestination->DepthStencilState));
    DeepCopy(&(inStruct->InputLayout), &(outDestination->InputLayout));
    outDestination->IBStripCutValue = inStruct->IBStripCutValue;
    outDestination->PrimitiveTopologyType = inStruct->PrimitiveTopologyType;
    memcpy(&(outDestination->NumRenderTargets), &(inStruct->NumRenderTargets), sizeof(UINT));
    memcpy(&(outDestination->RTVFormats), &(inStruct->RTVFormats), sizeof(DXGI_FORMAT) * 8);
    memcpy(&(outDestination->DSVFormat), &(inStruct->DSVFormat), sizeof(DXGI_FORMAT));
    memcpy(&(outDestination->SampleDesc), &(inStruct->SampleDesc), sizeof(DXGI_SAMPLE_DESC));
    memcpy(&(outDestination->NodeMask), &(inStruct->NodeMask), sizeof(UINT));
    DeepCopy(&(inStruct->CachedPSO), &(outDestination->CachedPSO));
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_COMPUTE_PIPELINE_STATE_DESC* inStruct, D3D12_COMPUTE_PIPELINE_STATE_DESC* outDestination)
{
    DX12CoreDeepCopy::UnwrapInterface(inStruct->pRootSignature, &(outDestination->pRootSignature));
    DeepCopy(&(inStruct->CS), &(outDestination->CS));
    memcpy(&(outDestination->NodeMask), &(inStruct->NodeMask), sizeof(UINT));
    DeepCopy(&(inStruct->CachedPSO), &(outDestination->CachedPSO));
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_D3D12_OPTIONS* inStruct, D3D12_FEATURE_DATA_D3D12_OPTIONS* outDestination)
{
    memcpy(&(outDestination->DoublePrecisionFloatShaderOps), &(inStruct->DoublePrecisionFloatShaderOps), sizeof(BOOL));
    memcpy(&(outDestination->OutputMergerLogicOp), &(inStruct->OutputMergerLogicOp), sizeof(BOOL));
    outDestination->MinPrecisionSupport = inStruct->MinPrecisionSupport;
    outDestination->TiledResourcesTier = inStruct->TiledResourcesTier;
    outDestination->ResourceBindingTier = inStruct->ResourceBindingTier;
    memcpy(&(outDestination->PSSpecifiedStencilRefSupported), &(inStruct->PSSpecifiedStencilRefSupported), sizeof(BOOL));
    memcpy(&(outDestination->TypedUAVLoadAdditionalFormats), &(inStruct->TypedUAVLoadAdditionalFormats), sizeof(BOOL));
    memcpy(&(outDestination->ROVsSupported), &(inStruct->ROVsSupported), sizeof(BOOL));
    outDestination->ConservativeRasterizationTier = inStruct->ConservativeRasterizationTier;
    memcpy(&(outDestination->MaxGPUVirtualAddressBitsPerResource), &(inStruct->MaxGPUVirtualAddressBitsPerResource), sizeof(UINT));
    memcpy(&(outDestination->StandardSwizzle64KBSupported), &(inStruct->StandardSwizzle64KBSupported), sizeof(BOOL));
    outDestination->CrossNodeSharingTier = inStruct->CrossNodeSharingTier;
    memcpy(&(outDestination->CrossAdapterRowMajorTextureSupported), &(inStruct->CrossAdapterRowMajorTextureSupported), sizeof(BOOL));
    memcpy(&(outDestination->VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation), &(inStruct->VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation), sizeof(BOOL));
    outDestination->ResourceHeapTier = inStruct->ResourceHeapTier;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_ARCHITECTURE* inStruct, D3D12_FEATURE_DATA_ARCHITECTURE* outDestination)
{
    memcpy(&(outDestination->NodeIndex), &(inStruct->NodeIndex), sizeof(UINT));
    memcpy(&(outDestination->TileBasedRenderer), &(inStruct->TileBasedRenderer), sizeof(BOOL));
    memcpy(&(outDestination->UMA), &(inStruct->UMA), sizeof(BOOL));
    memcpy(&(outDestination->CacheCoherentUMA), &(inStruct->CacheCoherentUMA), sizeof(BOOL));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_FEATURE_LEVELS* inStruct, D3D12_FEATURE_DATA_FEATURE_LEVELS* outDestination)
{
    memcpy(&(outDestination->NumFeatureLevels), &(inStruct->NumFeatureLevels), sizeof(UINT));
    D3D_FEATURE_LEVEL* copies = nullptr;

    if (inStruct->NumFeatureLevels > 0)
    {
        copies = new D3D_FEATURE_LEVEL[inStruct->NumFeatureLevels];

        for (UINT index = 0; index < inStruct->NumFeatureLevels; index++)
        {
            copies[index] = inStruct->pFeatureLevelsRequested[index];
        }
    }

    outDestination->pFeatureLevelsRequested = copies;
    outDestination->MaxSupportedFeatureLevel = inStruct->MaxSupportedFeatureLevel;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_FORMAT_SUPPORT* inStruct, D3D12_FEATURE_DATA_FORMAT_SUPPORT* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    outDestination->Support1 = inStruct->Support1;
    outDestination->Support2 = inStruct->Support2;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS* inStruct, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    memcpy(&(outDestination->SampleCount), &(inStruct->SampleCount), sizeof(UINT));
    outDestination->Flags = inStruct->Flags;
    memcpy(&(outDestination->NumQualityLevels), &(inStruct->NumQualityLevels), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_FORMAT_INFO* inStruct, D3D12_FEATURE_DATA_FORMAT_INFO* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    memcpy(&(outDestination->PlaneCount), &(inStruct->PlaneCount), sizeof(UINT8));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT* inStruct, D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT* outDestination)
{
    memcpy(&(outDestination->MaxGPUVirtualAddressBitsPerResource), &(inStruct->MaxGPUVirtualAddressBitsPerResource), sizeof(UINT));
    memcpy(&(outDestination->MaxGPUVirtualAddressBitsPerProcess), &(inStruct->MaxGPUVirtualAddressBitsPerProcess), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RESOURCE_ALLOCATION_INFO* inStruct, D3D12_RESOURCE_ALLOCATION_INFO* outDestination)
{
    memcpy(&(outDestination->SizeInBytes), &(inStruct->SizeInBytes), sizeof(UINT64));
    memcpy(&(outDestination->Alignment), &(inStruct->Alignment), sizeof(UINT64));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_HEAP_PROPERTIES* inStruct, D3D12_HEAP_PROPERTIES* outDestination)
{
    outDestination->Type = inStruct->Type;
    outDestination->CPUPageProperty = inStruct->CPUPageProperty;
    outDestination->MemoryPoolPreference = inStruct->MemoryPoolPreference;
    memcpy(&(outDestination->CreationNodeMask), &(inStruct->CreationNodeMask), sizeof(UINT));
    memcpy(&(outDestination->VisibleNodeMask), &(inStruct->VisibleNodeMask), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_HEAP_DESC* inStruct, D3D12_HEAP_DESC* outDestination)
{
    memcpy(&(outDestination->SizeInBytes), &(inStruct->SizeInBytes), sizeof(UINT64));
    DeepCopy(&(inStruct->Properties), &(outDestination->Properties));
    memcpy(&(outDestination->Alignment), &(inStruct->Alignment), sizeof(UINT64));
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RESOURCE_DESC* inStruct, D3D12_RESOURCE_DESC* outDestination)
{
    outDestination->Dimension = inStruct->Dimension;
    memcpy(&(outDestination->Alignment), &(inStruct->Alignment), sizeof(UINT64));
    memcpy(&(outDestination->Width), &(inStruct->Width), sizeof(UINT64));
    memcpy(&(outDestination->Height), &(inStruct->Height), sizeof(UINT));
    memcpy(&(outDestination->DepthOrArraySize), &(inStruct->DepthOrArraySize), sizeof(UINT16));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT16));
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    memcpy(&(outDestination->SampleDesc), &(inStruct->SampleDesc), sizeof(DXGI_SAMPLE_DESC));
    outDestination->Layout = inStruct->Layout;
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DEPTH_STENCIL_VALUE* inStruct, D3D12_DEPTH_STENCIL_VALUE* outDestination)
{
    memcpy(&(outDestination->Depth), &(inStruct->Depth), sizeof(FLOAT));
    memcpy(&(outDestination->Stencil), &(inStruct->Stencil), sizeof(UINT8));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_CLEAR_VALUE* inStruct, D3D12_CLEAR_VALUE* outDestination)
{
    D3D12_CLEAR_VALUE* clearCopy = nullptr;

    if (inStruct != nullptr)
    {
        clearCopy = new D3D12_CLEAR_VALUE;
        memcpy(clearCopy, inStruct, sizeof(D3D12_CLEAR_VALUE));
    }

    //TODO refactor below :  Assignment of function parameter has no effect outside the function. Did you forget dereferencing it?
    outDestination = clearCopy;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RANGE* inStruct, D3D12_RANGE* outDestination)
{
    memcpy(&(outDestination->Begin), &(inStruct->Begin), sizeof(SIZE_T));
    memcpy(&(outDestination->End), &(inStruct->End), sizeof(SIZE_T));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SUBRESOURCE_INFO* inStruct, D3D12_SUBRESOURCE_INFO* outDestination)
{
    memcpy(&(outDestination->Offset), &(inStruct->Offset), sizeof(UINT64));
    memcpy(&(outDestination->RowPitch), &(inStruct->RowPitch), sizeof(UINT));
    memcpy(&(outDestination->DepthPitch), &(inStruct->DepthPitch), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TILED_RESOURCE_COORDINATE* inStruct, D3D12_TILED_RESOURCE_COORDINATE* outDestination)
{
    memcpy(&(outDestination->X), &(inStruct->X), sizeof(UINT));
    memcpy(&(outDestination->Y), &(inStruct->Y), sizeof(UINT));
    memcpy(&(outDestination->Z), &(inStruct->Z), sizeof(UINT));
    memcpy(&(outDestination->Subresource), &(inStruct->Subresource), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TILE_REGION_SIZE* inStruct, D3D12_TILE_REGION_SIZE* outDestination)
{
    memcpy(&(outDestination->NumTiles), &(inStruct->NumTiles), sizeof(UINT));
    memcpy(&(outDestination->UseBox), &(inStruct->UseBox), sizeof(BOOL));
    memcpy(&(outDestination->Width), &(inStruct->Width), sizeof(UINT));
    memcpy(&(outDestination->Height), &(inStruct->Height), sizeof(UINT16));
    memcpy(&(outDestination->Depth), &(inStruct->Depth), sizeof(UINT16));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SUBRESOURCE_TILING* inStruct, D3D12_SUBRESOURCE_TILING* outDestination)
{
    memcpy(&(outDestination->WidthInTiles), &(inStruct->WidthInTiles), sizeof(UINT));
    memcpy(&(outDestination->HeightInTiles), &(inStruct->HeightInTiles), sizeof(UINT16));
    memcpy(&(outDestination->DepthInTiles), &(inStruct->DepthInTiles), sizeof(UINT16));
    memcpy(&(outDestination->StartTileIndexInOverallResource), &(inStruct->StartTileIndexInOverallResource), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TILE_SHAPE* inStruct, D3D12_TILE_SHAPE* outDestination)
{
    memcpy(&(outDestination->WidthInTexels), &(inStruct->WidthInTexels), sizeof(UINT));
    memcpy(&(outDestination->HeightInTexels), &(inStruct->HeightInTexels), sizeof(UINT));
    memcpy(&(outDestination->DepthInTexels), &(inStruct->DepthInTexels), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_PACKED_MIP_INFO* inStruct, D3D12_PACKED_MIP_INFO* outDestination)
{
    memcpy(&(outDestination->NumStandardMips), &(inStruct->NumStandardMips), sizeof(UINT8));
    memcpy(&(outDestination->NumPackedMips), &(inStruct->NumPackedMips), sizeof(UINT8));
    memcpy(&(outDestination->NumTilesForPackedMips), &(inStruct->NumTilesForPackedMips), sizeof(UINT));
    memcpy(&(outDestination->StartTileIndexInOverallResource), &(inStruct->StartTileIndexInOverallResource), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RESOURCE_TRANSITION_BARRIER* inStruct, D3D12_RESOURCE_TRANSITION_BARRIER* outDestination)
{
    DX12CoreDeepCopy::UnwrapInterface(inStruct->pResource, &(outDestination->pResource));
    memcpy(&(outDestination->Subresource), &(inStruct->Subresource), sizeof(UINT));
    outDestination->StateBefore = inStruct->StateBefore;
    outDestination->StateAfter = inStruct->StateAfter;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RESOURCE_ALIASING_BARRIER* inStruct, D3D12_RESOURCE_ALIASING_BARRIER* outDestination)
{
    DX12CoreDeepCopy::UnwrapInterface(inStruct->pResourceBefore, &(outDestination->pResourceBefore));
    DX12CoreDeepCopy::UnwrapInterface(inStruct->pResourceAfter, &(outDestination->pResourceAfter));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RESOURCE_UAV_BARRIER* inStruct, D3D12_RESOURCE_UAV_BARRIER* outDestination)
{
    DX12CoreDeepCopy::UnwrapInterface(inStruct->pResource, &(outDestination->pResource));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RESOURCE_BARRIER* inStruct, D3D12_RESOURCE_BARRIER* outDestination)
{
    memcpy(outDestination, inStruct, sizeof(D3D12_RESOURCE_BARRIER));

    outDestination->Type = inStruct->Type;
    outDestination->Flags = inStruct->Flags;

    if (inStruct->Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
    {
        DX12CoreDeepCopy::UnwrapInterface(inStruct->Transition.pResource, &(outDestination->Transition.pResource));
    }
    else if (inStruct->Type == D3D12_RESOURCE_BARRIER_TYPE_ALIASING)
    {
        DX12CoreDeepCopy::UnwrapInterface(inStruct->Aliasing.pResourceBefore, &(outDestination->Aliasing.pResourceBefore));
        DX12CoreDeepCopy::UnwrapInterface(inStruct->Aliasing.pResourceAfter, &(outDestination->Aliasing.pResourceAfter));
    }
    else if (inStruct->Type == D3D12_RESOURCE_BARRIER_TYPE_UAV)
    {
        DX12CoreDeepCopy::UnwrapInterface(inStruct->UAV.pResource, &(outDestination->UAV.pResource));
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SUBRESOURCE_FOOTPRINT* inStruct, D3D12_SUBRESOURCE_FOOTPRINT* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    memcpy(&(outDestination->Width), &(inStruct->Width), sizeof(UINT));
    memcpy(&(outDestination->Height), &(inStruct->Height), sizeof(UINT));
    memcpy(&(outDestination->Depth), &(inStruct->Depth), sizeof(UINT));
    memcpy(&(outDestination->RowPitch), &(inStruct->RowPitch), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* inStruct, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* outDestination)
{
    memcpy(&(outDestination->Offset), &(inStruct->Offset), sizeof(UINT64));
    DeepCopy(&(inStruct->Footprint), &(outDestination->Footprint));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEXTURE_COPY_LOCATION* inStruct, D3D12_TEXTURE_COPY_LOCATION* outDestination)
{
    DX12CoreDeepCopy::UnwrapInterface(inStruct->pResource, &(outDestination->pResource));
    outDestination->Type = inStruct->Type;

    if (inStruct->Type == D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX)
    {
        outDestination->SubresourceIndex = inStruct->SubresourceIndex;
    }
    else if (inStruct->Type == D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT)
    {
        outDestination->PlacedFootprint = inStruct->PlacedFootprint;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_BUFFER_SRV* inStruct, D3D12_BUFFER_SRV* outDestination)
{
    memcpy(&(outDestination->FirstElement), &(inStruct->FirstElement), sizeof(UINT64));
    memcpy(&(outDestination->NumElements), &(inStruct->NumElements), sizeof(UINT));
    memcpy(&(outDestination->StructureByteStride), &(inStruct->StructureByteStride), sizeof(UINT));
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_SRV* inStruct, D3D12_TEX1D_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_ARRAY_SRV* inStruct, D3D12_TEX1D_ARRAY_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_SRV* inStruct, D3D12_TEX2D_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->PlaneSlice), &(inStruct->PlaneSlice), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_ARRAY_SRV* inStruct, D3D12_TEX2D_ARRAY_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
    memcpy(&(outDestination->PlaneSlice), &(inStruct->PlaneSlice), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX3D_SRV* inStruct, D3D12_TEX3D_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEXCUBE_SRV* inStruct, D3D12_TEXCUBE_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEXCUBE_ARRAY_SRV* inStruct, D3D12_TEXCUBE_ARRAY_SRV* outDestination)
{
    memcpy(&(outDestination->MostDetailedMip), &(inStruct->MostDetailedMip), sizeof(UINT));
    memcpy(&(outDestination->MipLevels), &(inStruct->MipLevels), sizeof(UINT));
    memcpy(&(outDestination->First2DArrayFace), &(inStruct->First2DArrayFace), sizeof(UINT));
    memcpy(&(outDestination->NumCubes), &(inStruct->NumCubes), sizeof(UINT));
    memcpy(&(outDestination->ResourceMinLODClamp), &(inStruct->ResourceMinLODClamp), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2DMS_SRV* inStruct, D3D12_TEX2DMS_SRV* outDestination)
{
    memcpy(&(outDestination->UnusedField_NothingToDefine), &(inStruct->UnusedField_NothingToDefine), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2DMS_ARRAY_SRV* inStruct, D3D12_TEX2DMS_ARRAY_SRV* outDestination)
{
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SHADER_RESOURCE_VIEW_DESC* inStruct, D3D12_SHADER_RESOURCE_VIEW_DESC* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    outDestination->ViewDimension = inStruct->ViewDimension;
    memcpy(&(outDestination->Shader4ComponentMapping), &(inStruct->Shader4ComponentMapping), sizeof(UINT));

    switch (inStruct->ViewDimension)
    {
        case D3D12_SRV_DIMENSION_BUFFER:
            memcpy(&(outDestination->Buffer), &(inStruct->Buffer), sizeof(D3D12_BUFFER_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE1D:
            memcpy(&(outDestination->Texture1D), &(inStruct->Texture1D), sizeof(D3D12_TEX1D_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
            memcpy(&(outDestination->Texture1DArray), &(inStruct->Texture1DArray), sizeof(D3D12_TEX1D_ARRAY_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2D:
            memcpy(&(outDestination->Texture2D), &(inStruct->Texture2D), sizeof(D3D12_TEX2D_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
            memcpy(&(outDestination->Texture2DArray), &(inStruct->Texture2DArray), sizeof(D3D12_TEX2D_ARRAY_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2DMS:
            memcpy(&(outDestination->Texture2DMS), &(inStruct->Texture2DMS), sizeof(D3D12_TEX2DMS_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
            memcpy(&(outDestination->Texture2DMSArray), &(inStruct->Texture2DMSArray), sizeof(D3D12_TEX2DMS_ARRAY_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURE3D:
            memcpy(&(outDestination->Texture3D), &(inStruct->Texture3D), sizeof(D3D12_TEX3D_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURECUBE:
            memcpy(&(outDestination->TextureCube), &(inStruct->TextureCube), sizeof(D3D12_TEXCUBE_SRV));
            break;

        case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
            memcpy((&outDestination->TextureCubeArray), &(inStruct->TextureCubeArray), sizeof(D3D12_TEXCUBE_ARRAY_SRV));
            break;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_CONSTANT_BUFFER_VIEW_DESC* inStruct, D3D12_CONSTANT_BUFFER_VIEW_DESC* outDestination)
{
    memcpy(&(outDestination->BufferLocation), &(inStruct->BufferLocation), sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
    memcpy(&(outDestination->SizeInBytes), &(inStruct->SizeInBytes), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SAMPLER_DESC* inStruct, D3D12_SAMPLER_DESC* outDestination)
{
    outDestination->Filter = inStruct->Filter;
    outDestination->AddressU = inStruct->AddressU;
    outDestination->AddressV = inStruct->AddressV;
    outDestination->AddressW = inStruct->AddressW;
    memcpy(&(outDestination->MipLODBias), &(inStruct->MipLODBias), sizeof(FLOAT));
    memcpy(&(outDestination->MaxAnisotropy), &(inStruct->MaxAnisotropy), sizeof(UINT));
    outDestination->ComparisonFunc = inStruct->ComparisonFunc;
    memcpy(&(outDestination->BorderColor), &(inStruct->BorderColor), sizeof(FLOAT) * 4);
    memcpy(&(outDestination->MinLOD), &(inStruct->MinLOD), sizeof(FLOAT));
    memcpy(&(outDestination->MaxLOD), &(inStruct->MaxLOD), sizeof(FLOAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_BUFFER_UAV* inStruct, D3D12_BUFFER_UAV* outDestination)
{
    memcpy(&(outDestination->FirstElement), &(inStruct->FirstElement), sizeof(UINT64));
    memcpy(&(outDestination->NumElements), &(inStruct->NumElements), sizeof(UINT));
    memcpy(&(outDestination->StructureByteStride), &(inStruct->StructureByteStride), sizeof(UINT));
    memcpy(&(outDestination->CounterOffsetInBytes), &(inStruct->CounterOffsetInBytes), sizeof(UINT64));
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_UAV* inStruct, D3D12_TEX1D_UAV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_ARRAY_UAV* inStruct, D3D12_TEX1D_ARRAY_UAV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_UAV* inStruct, D3D12_TEX2D_UAV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->PlaneSlice), &(inStruct->PlaneSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_ARRAY_UAV* inStruct, D3D12_TEX2D_ARRAY_UAV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
    memcpy(&(outDestination->PlaneSlice), &(inStruct->PlaneSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX3D_UAV* inStruct, D3D12_TEX3D_UAV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstWSlice), &(inStruct->FirstWSlice), sizeof(UINT));
    memcpy(&(outDestination->WSize), &(inStruct->WSize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_UNORDERED_ACCESS_VIEW_DESC* inStruct, D3D12_UNORDERED_ACCESS_VIEW_DESC* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    outDestination->ViewDimension = inStruct->ViewDimension;

    switch (inStruct->ViewDimension)
    {
        case D3D12_UAV_DIMENSION_BUFFER:
            memcpy(&(outDestination->Buffer), &(inStruct->Buffer), sizeof(D3D12_BUFFER_UAV));
            break;

        case D3D12_UAV_DIMENSION_TEXTURE1D:
            memcpy(&(outDestination->Texture1D), &(inStruct->Texture1D), sizeof(D3D12_TEX1D_UAV));
            break;

        case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
            memcpy(&(outDestination->Texture1DArray), &(inStruct->Texture1DArray), sizeof(D3D12_TEX1D_ARRAY_UAV));
            break;

        case D3D12_UAV_DIMENSION_TEXTURE2D:
            memcpy(&(outDestination->Texture2D), &(inStruct->Texture2D), sizeof(D3D12_TEX2D_UAV));
            break;

        case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
            memcpy(&(outDestination->Texture2DArray), &(inStruct->Texture2DArray), sizeof(D3D12_TEX2D_ARRAY_UAV));
            break;

        case D3D12_UAV_DIMENSION_TEXTURE3D:
            memcpy(&(outDestination->Texture3D), &(inStruct->Texture3D), sizeof(D3D12_TEX3D_UAV));
            break;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_BUFFER_RTV* inStruct, D3D12_BUFFER_RTV* outDestination)
{
    memcpy(&(outDestination->FirstElement), &(inStruct->FirstElement), sizeof(UINT64));
    memcpy(&(outDestination->NumElements), &(inStruct->NumElements), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_RTV* inStruct, D3D12_TEX1D_RTV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_ARRAY_RTV* inStruct, D3D12_TEX1D_ARRAY_RTV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_RTV* inStruct, D3D12_TEX2D_RTV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->PlaneSlice), &(inStruct->PlaneSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2DMS_RTV* inStruct, D3D12_TEX2DMS_RTV* outDestination)
{
    memcpy(&(outDestination->UnusedField_NothingToDefine), &(inStruct->UnusedField_NothingToDefine), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_ARRAY_RTV* inStruct, D3D12_TEX2D_ARRAY_RTV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
    memcpy(&(outDestination->PlaneSlice), &(inStruct->PlaneSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2DMS_ARRAY_RTV* inStruct, D3D12_TEX2DMS_ARRAY_RTV* outDestination)
{
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX3D_RTV* inStruct, D3D12_TEX3D_RTV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstWSlice), &(inStruct->FirstWSlice), sizeof(UINT));
    memcpy(&(outDestination->WSize), &(inStruct->WSize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_RENDER_TARGET_VIEW_DESC* inStruct, D3D12_RENDER_TARGET_VIEW_DESC* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    outDestination->ViewDimension = inStruct->ViewDimension;

    switch (inStruct->ViewDimension)
    {
        case D3D12_RTV_DIMENSION_BUFFER:
            memcpy(&(outDestination->Buffer), &(inStruct->Buffer), sizeof(D3D12_BUFFER_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE1D:
            memcpy(&(outDestination->Texture1D), &(inStruct->Texture1D), sizeof(D3D12_TEX1D_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
            memcpy(&(outDestination->Texture1DArray), &(inStruct->Texture1DArray), sizeof(D3D12_TEX1D_ARRAY_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE2D:
            memcpy(&(outDestination->Texture2D), &(inStruct->Texture2D), sizeof(D3D12_TEX2D_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
            memcpy(&(outDestination->Texture2DArray), &(inStruct->Texture2DArray), sizeof(D3D12_TEX2D_ARRAY_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE2DMS:
            memcpy(&(outDestination->Texture2DMS), &(inStruct->Texture2DMS), sizeof(D3D12_TEX2DMS_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
            memcpy(&(outDestination->Texture2DMSArray), &(inStruct->Texture2DMSArray), sizeof(D3D12_TEX2DMS_ARRAY_RTV));
            break;

        case D3D12_RTV_DIMENSION_TEXTURE3D:
            memcpy(&(outDestination->Texture3D), &(inStruct->Texture3D), sizeof(D3D12_TEX3D_RTV));
            break;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_DSV* inStruct, D3D12_TEX1D_DSV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX1D_ARRAY_DSV* inStruct, D3D12_TEX1D_ARRAY_DSV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_DSV* inStruct, D3D12_TEX2D_DSV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2D_ARRAY_DSV* inStruct, D3D12_TEX2D_ARRAY_DSV* outDestination)
{
    memcpy(&(outDestination->MipSlice), &(inStruct->MipSlice), sizeof(UINT));
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2DMS_DSV* inStruct, D3D12_TEX2DMS_DSV* outDestination)
{
    memcpy(&(outDestination->UnusedField_NothingToDefine), &(inStruct->UnusedField_NothingToDefine), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_TEX2DMS_ARRAY_DSV* inStruct, D3D12_TEX2DMS_ARRAY_DSV* outDestination)
{
    memcpy(&(outDestination->FirstArraySlice), &(inStruct->FirstArraySlice), sizeof(UINT));
    memcpy(&(outDestination->ArraySize), &(inStruct->ArraySize), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DEPTH_STENCIL_VIEW_DESC* inStruct, D3D12_DEPTH_STENCIL_VIEW_DESC* outDestination)
{
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
    outDestination->ViewDimension = inStruct->ViewDimension;
    outDestination->Flags = inStruct->Flags;

    switch (inStruct->ViewDimension)
    {
        case D3D12_DSV_DIMENSION_TEXTURE1D:
            memcpy(&(outDestination->Texture1D), &(inStruct->Texture1D), sizeof(D3D12_TEX1D_DSV));
            break;

        case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
            memcpy(&(outDestination->Texture1DArray), &(inStruct->Texture1DArray), sizeof(D3D12_TEX1D_ARRAY_DSV));
            break;

        case D3D12_DSV_DIMENSION_TEXTURE2D:
            memcpy(&(outDestination->Texture2D), &(inStruct->Texture2D), sizeof(D3D12_TEX2D_DSV));
            break;

        case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
            memcpy(&(outDestination->Texture2DArray), &(inStruct->Texture2DArray), sizeof(D3D12_TEX2D_ARRAY_DSV));
            break;

        case D3D12_DSV_DIMENSION_TEXTURE2DMS:
            memcpy(&(outDestination->Texture2DMS), &(inStruct->Texture2DMS), sizeof(D3D12_TEX2DMS_DSV));
            break;

        case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
            memcpy(&(outDestination->Texture2DMSArray), &(inStruct->Texture2DMSArray), sizeof(D3D12_TEX2DMS_ARRAY_DSV));
            break;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DESCRIPTOR_HEAP_DESC* inStruct, D3D12_DESCRIPTOR_HEAP_DESC* outDestination)
{
    outDestination->Type = inStruct->Type;
    memcpy(&(outDestination->NumDescriptors), &(inStruct->NumDescriptors), sizeof(UINT));
    outDestination->Flags = inStruct->Flags;
    memcpy(&(outDestination->NodeMask), &(inStruct->NodeMask), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DESCRIPTOR_RANGE* inStruct, D3D12_DESCRIPTOR_RANGE* outDestination)
{
    outDestination->RangeType = inStruct->RangeType;
    memcpy(&(outDestination->NumDescriptors), &(inStruct->NumDescriptors), sizeof(UINT));
    memcpy(&(outDestination->BaseShaderRegister), &(inStruct->BaseShaderRegister), sizeof(UINT));
    memcpy(&(outDestination->RegisterSpace), &(inStruct->RegisterSpace), sizeof(UINT));
    memcpy(&(outDestination->OffsetInDescriptorsFromTableStart), &(inStruct->OffsetInDescriptorsFromTableStart), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_ROOT_DESCRIPTOR_TABLE* inStruct, D3D12_ROOT_DESCRIPTOR_TABLE* outDestination)
{
    memcpy(&(outDestination->NumDescriptorRanges), &(inStruct->NumDescriptorRanges), sizeof(UINT));
    D3D12_DESCRIPTOR_RANGE* pDescriptorRangesCopy = nullptr;

    if (inStruct->NumDescriptorRanges > 0)
    {
        pDescriptorRangesCopy = new D3D12_DESCRIPTOR_RANGE[inStruct->NumDescriptorRanges];

        for (UINT index = 0; index < inStruct->NumDescriptorRanges; index++)
        {
            DeepCopy(&(inStruct->pDescriptorRanges[index]), &(pDescriptorRangesCopy[index]));
        }
    }

    outDestination->pDescriptorRanges = pDescriptorRangesCopy;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_ROOT_CONSTANTS* inStruct, D3D12_ROOT_CONSTANTS* outDestination)
{
    memcpy(&(outDestination->ShaderRegister), &(inStruct->ShaderRegister), sizeof(UINT));
    memcpy(&(outDestination->RegisterSpace), &(inStruct->RegisterSpace), sizeof(UINT));
    memcpy(&(outDestination->Num32BitValues), &(inStruct->Num32BitValues), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_ROOT_DESCRIPTOR* inStruct, D3D12_ROOT_DESCRIPTOR* outDestination)
{
    memcpy(&(outDestination->ShaderRegister), &(inStruct->ShaderRegister), sizeof(UINT));
    memcpy(&(outDestination->RegisterSpace), &(inStruct->RegisterSpace), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_ROOT_PARAMETER* inStruct, D3D12_ROOT_PARAMETER* outDestination)
{
    outDestination->ParameterType = inStruct->ParameterType;
    outDestination->ShaderVisibility = inStruct->ShaderVisibility;

    switch (inStruct->ParameterType)
    {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            DeepCopy(&(inStruct->DescriptorTable), &(outDestination->DescriptorTable));
            break;

        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            DeepCopy(&(inStruct->Constants), &(outDestination->Constants));
            break;

        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            DeepCopy(&(inStruct->Descriptor), &(outDestination->Descriptor));
            break;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_STATIC_SAMPLER_DESC* inStruct, D3D12_STATIC_SAMPLER_DESC* outDestination)
{
    outDestination->Filter = inStruct->Filter;
    outDestination->AddressU = inStruct->AddressU;
    outDestination->AddressV = inStruct->AddressV;
    outDestination->AddressW = inStruct->AddressW;
    memcpy(&(outDestination->MipLODBias), &(inStruct->MipLODBias), sizeof(FLOAT));
    memcpy(&(outDestination->MaxAnisotropy), &(inStruct->MaxAnisotropy), sizeof(UINT));
    outDestination->ComparisonFunc = inStruct->ComparisonFunc;
    outDestination->BorderColor = inStruct->BorderColor;
    memcpy(&(outDestination->MinLOD), &(inStruct->MinLOD), sizeof(FLOAT));
    memcpy(&(outDestination->MaxLOD), &(inStruct->MaxLOD), sizeof(FLOAT));
    memcpy(&(outDestination->ShaderRegister), &(inStruct->ShaderRegister), sizeof(UINT));
    memcpy(&(outDestination->RegisterSpace), &(inStruct->RegisterSpace), sizeof(UINT));
    outDestination->ShaderVisibility = inStruct->ShaderVisibility;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_ROOT_SIGNATURE_DESC* inStruct, D3D12_ROOT_SIGNATURE_DESC* outDestination)
{
    memcpy(&(outDestination->NumParameters), &(inStruct->NumParameters), sizeof(UINT));
    D3D12_ROOT_PARAMETER* pParametersCopy = nullptr;

    if (inStruct->NumParameters > 0)
    {
        pParametersCopy = new D3D12_ROOT_PARAMETER[inStruct->NumParameters];

        for (UINT index = 0; index < inStruct->NumParameters; index++)
        {
            DeepCopy(&(inStruct->pParameters[index]), &(pParametersCopy[index]));
        }
    }

    outDestination->pParameters = pParametersCopy;
    memcpy(&(outDestination->NumStaticSamplers), &(inStruct->NumStaticSamplers), sizeof(UINT));
    D3D12_STATIC_SAMPLER_DESC* pStaticSamplersCopy = nullptr;

    if (inStruct->NumStaticSamplers > 0)
    {
        pStaticSamplersCopy = new D3D12_STATIC_SAMPLER_DESC[inStruct->NumStaticSamplers];

        for (UINT index = 0; index < inStruct->NumStaticSamplers; index++)
        {
            DeepCopy(&(inStruct->pStaticSamplers[index]), &(pStaticSamplersCopy[index]));
        }
    }

    outDestination->pStaticSamplers = pStaticSamplersCopy;
    outDestination->Flags = inStruct->Flags;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_CPU_DESCRIPTOR_HANDLE* inStruct, D3D12_CPU_DESCRIPTOR_HANDLE* outDestination)
{
    memcpy(&(outDestination->ptr), &(inStruct->ptr), sizeof(SIZE_T));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_GPU_DESCRIPTOR_HANDLE* inStruct, D3D12_GPU_DESCRIPTOR_HANDLE* outDestination)
{
    memcpy(&(outDestination->ptr), &(inStruct->ptr), sizeof(UINT64));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DISCARD_REGION* inStruct, D3D12_DISCARD_REGION* outDestination)
{
    memcpy(&(outDestination->NumRects), &(inStruct->NumRects), sizeof(UINT));
    D3D12_RECT* pRectsCopy = nullptr;

    if (inStruct->NumRects > 0)
    {
        pRectsCopy = new D3D12_RECT[inStruct->NumRects];

        for (UINT index = 0; index < inStruct->NumRects; index++)
        {
            pRectsCopy[index] = inStruct->pRects[index];
        }
    }

    outDestination->pRects = pRectsCopy;
    memcpy(&(outDestination->FirstSubresource), &(inStruct->FirstSubresource), sizeof(UINT));
    memcpy(&(outDestination->NumSubresources), &(inStruct->NumSubresources), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_QUERY_HEAP_DESC* inStruct, D3D12_QUERY_HEAP_DESC* outDestination)
{
    outDestination->Type = inStruct->Type;
    memcpy(&(outDestination->Count), &(inStruct->Count), sizeof(UINT));
    memcpy(&(outDestination->NodeMask), &(inStruct->NodeMask), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_QUERY_DATA_PIPELINE_STATISTICS* inStruct, D3D12_QUERY_DATA_PIPELINE_STATISTICS* outDestination)
{
    memcpy(&(outDestination->IAVertices), &(inStruct->IAVertices), sizeof(UINT64));
    memcpy(&(outDestination->IAPrimitives), &(inStruct->IAPrimitives), sizeof(UINT64));
    memcpy(&(outDestination->VSInvocations), &(inStruct->VSInvocations), sizeof(UINT64));
    memcpy(&(outDestination->GSInvocations), &(inStruct->GSInvocations), sizeof(UINT64));
    memcpy(&(outDestination->GSPrimitives), &(inStruct->GSPrimitives), sizeof(UINT64));
    memcpy(&(outDestination->CInvocations), &(inStruct->CInvocations), sizeof(UINT64));
    memcpy(&(outDestination->CPrimitives), &(inStruct->CPrimitives), sizeof(UINT64));
    memcpy(&(outDestination->PSInvocations), &(inStruct->PSInvocations), sizeof(UINT64));
    memcpy(&(outDestination->HSInvocations), &(inStruct->HSInvocations), sizeof(UINT64));
    memcpy(&(outDestination->DSInvocations), &(inStruct->DSInvocations), sizeof(UINT64));
    memcpy(&(outDestination->CSInvocations), &(inStruct->CSInvocations), sizeof(UINT64));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_QUERY_DATA_SO_STATISTICS* inStruct, D3D12_QUERY_DATA_SO_STATISTICS* outDestination)
{
    memcpy(&(outDestination->NumPrimitivesWritten), &(inStruct->NumPrimitivesWritten), sizeof(UINT64));
    memcpy(&(outDestination->PrimitivesStorageNeeded), &(inStruct->PrimitivesStorageNeeded), sizeof(UINT64));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_STREAM_OUTPUT_BUFFER_VIEW* inStruct, D3D12_STREAM_OUTPUT_BUFFER_VIEW* outDestination)
{
    memcpy(&(outDestination->BufferLocation), &(inStruct->BufferLocation), sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
    memcpy(&(outDestination->SizeInBytes), &(inStruct->SizeInBytes), sizeof(UINT64));
    memcpy(&(outDestination->BufferFilledSizeLocation), &(inStruct->BufferFilledSizeLocation), sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DRAW_ARGUMENTS* inStruct, D3D12_DRAW_ARGUMENTS* outDestination)
{
    memcpy(&(outDestination->VertexCountPerInstance), &(inStruct->VertexCountPerInstance), sizeof(UINT));
    memcpy(&(outDestination->InstanceCount), &(inStruct->InstanceCount), sizeof(UINT));
    memcpy(&(outDestination->StartVertexLocation), &(inStruct->StartVertexLocation), sizeof(UINT));
    memcpy(&(outDestination->StartInstanceLocation), &(inStruct->StartInstanceLocation), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DRAW_INDEXED_ARGUMENTS* inStruct, D3D12_DRAW_INDEXED_ARGUMENTS* outDestination)
{
    memcpy(&(outDestination->IndexCountPerInstance), &(inStruct->IndexCountPerInstance), sizeof(UINT));
    memcpy(&(outDestination->InstanceCount), &(inStruct->InstanceCount), sizeof(UINT));
    memcpy(&(outDestination->StartIndexLocation), &(inStruct->StartIndexLocation), sizeof(UINT));
    memcpy(&(outDestination->BaseVertexLocation), &(inStruct->BaseVertexLocation), sizeof(INT));
    memcpy(&(outDestination->StartInstanceLocation), &(inStruct->StartInstanceLocation), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_DISPATCH_ARGUMENTS* inStruct, D3D12_DISPATCH_ARGUMENTS* outDestination)
{
    memcpy(&(outDestination->ThreadGroupCountX), &(inStruct->ThreadGroupCountX), sizeof(UINT));
    memcpy(&(outDestination->ThreadGroupCountY), &(inStruct->ThreadGroupCountY), sizeof(UINT));
    memcpy(&(outDestination->ThreadGroupCountZ), &(inStruct->ThreadGroupCountZ), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_VERTEX_BUFFER_VIEW* inStruct, D3D12_VERTEX_BUFFER_VIEW* outDestination)
{
    memcpy(&(outDestination->BufferLocation), &(inStruct->BufferLocation), sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
    memcpy(&(outDestination->SizeInBytes), &(inStruct->SizeInBytes), sizeof(UINT));
    memcpy(&(outDestination->StrideInBytes), &(inStruct->StrideInBytes), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_INDEX_BUFFER_VIEW* inStruct, D3D12_INDEX_BUFFER_VIEW* outDestination)
{
    memcpy(&(outDestination->BufferLocation), &(inStruct->BufferLocation), sizeof(D3D12_GPU_VIRTUAL_ADDRESS));
    memcpy(&(outDestination->SizeInBytes), &(inStruct->SizeInBytes), sizeof(UINT));
    memcpy(&(outDestination->Format), &(inStruct->Format), sizeof(DXGI_FORMAT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_INDIRECT_ARGUMENT_DESC* inStruct, D3D12_INDIRECT_ARGUMENT_DESC* outDestination)
{
    outDestination->Type = inStruct->Type;

    if (outDestination->Type == D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW)
    {
        outDestination->VertexBuffer.Slot = inStruct->VertexBuffer.Slot;
    }
    else if (outDestination->Type == D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT)
    {
        outDestination->Constant.RootParameterIndex = inStruct->Constant.RootParameterIndex;
        outDestination->Constant.DestOffsetIn32BitValues = inStruct->Constant.DestOffsetIn32BitValues;
        outDestination->Constant.Num32BitValuesToSet = inStruct->Constant.Num32BitValuesToSet;
    }
    else if (outDestination->Type == D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW)
    {
        outDestination->ConstantBufferView.RootParameterIndex = inStruct->ConstantBufferView.RootParameterIndex;
    }
    else if (outDestination->Type == D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW)
    {
        outDestination->ShaderResourceView.RootParameterIndex = inStruct->ShaderResourceView.RootParameterIndex;
    }
    else if (outDestination->Type == D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW)
    {
        outDestination->UnorderedAccessView.RootParameterIndex = inStruct->UnorderedAccessView.RootParameterIndex;
    }
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_COMMAND_SIGNATURE_DESC* inStruct, D3D12_COMMAND_SIGNATURE_DESC* outDestination)
{
    memcpy(&(outDestination->ByteStride), &(inStruct->ByteStride), sizeof(UINT));
    memcpy(&(outDestination->NumArgumentDescs), &(inStruct->NumArgumentDescs), sizeof(UINT));
    D3D12_INDIRECT_ARGUMENT_DESC* pArgumentDescsCopy = nullptr;

    if (inStruct->NumArgumentDescs > 0)
    {
        pArgumentDescsCopy = new D3D12_INDIRECT_ARGUMENT_DESC[inStruct->NumArgumentDescs];

        for (UINT index = 0; index < inStruct->NumArgumentDescs; index++)
        {
            DeepCopy(&(inStruct->pArgumentDescs[index]), &(pArgumentDescsCopy[index]));
        }
    }

    outDestination->pArgumentDescs = pArgumentDescsCopy;
    memcpy(&(outDestination->NodeMask), &(inStruct->NodeMask), sizeof(UINT));
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_SUBRESOURCE_DATA* inStruct, D3D12_SUBRESOURCE_DATA* outDestination)
{
    outDestination->pData = inStruct->pData;
    outDestination->RowPitch = inStruct->RowPitch;
    outDestination->SlicePitch = inStruct->SlicePitch;
}

//-----------------------------------------------------------------------------
/// Deep copy the incoming structure into the destination structure.
/// \param inStruct The original structure to copy.
/// \param outDestination The destination that the structure will be deep copied to.
//-----------------------------------------------------------------------------
void DX12CoreDeepCopy::DeepCopy(const D3D12_MEMCPY_DEST* inStruct, D3D12_MEMCPY_DEST* outDestination)
{
    memcpy(&(outDestination->RowPitch), &(inStruct->RowPitch), sizeof(SIZE_T));
    memcpy(&(outDestination->SlicePitch), &(inStruct->SlicePitch), sizeof(SIZE_T));
}