//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12CoreSymbolSerializers.cpp
/// \brief  Serialize D3D12 types to strings, and build nested XML.
//=============================================================================

#include "DX12CoreSymbolSerializers.h"
#include "DX12Serializers.h"
#include "../Util/DX12Utilities.h"
#include "../Common/misc.h"

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCommandListTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteCommandListTypeEnumAsString(DWORD inWriteCommandListTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteCommandListTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_COMMAND_LIST_TYPE_DIRECT, resultString);
            PRINTENUMCASE(D3D12_COMMAND_LIST_TYPE_BUNDLE, resultString);
            PRINTENUMCASE(D3D12_COMMAND_LIST_TYPE_COMPUTE, resultString);
            PRINTENUMCASE(D3D12_COMMAND_LIST_TYPE_COPY, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCommandQueueFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteCommandQueueFlagsEnumAsString(DWORD inWriteCommandQueueFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteCommandQueueFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_COMMAND_QUEUE_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeCommandQueueFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteCommandQueueFlagsEnumAsString, D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCommandQueuePriorityEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteCommandQueuePriorityEnumAsString(DWORD inWriteCommandQueuePriorityEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteCommandQueuePriorityEnumAsString)
    {
            PRINTENUMCASE(D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, resultString);
            PRINTENUMCASE(D3D12_COMMAND_QUEUE_PRIORITY_HIGH, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWritePrimitiveTopologyTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WritePrimitiveTopologyTypeEnumAsString(DWORD inWritePrimitiveTopologyTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWritePrimitiveTopologyTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, resultString);
            PRINTENUMCASE(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, resultString);
            PRINTENUMCASE(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, resultString);
            PRINTENUMCASE(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, resultString);
            PRINTENUMCASE(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteInputClassificationEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteInputClassificationEnumAsString(DWORD inWriteInputClassificationEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteInputClassificationEnumAsString)
    {
            PRINTENUMCASE(D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, resultString);
            PRINTENUMCASE(D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFillModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFillModeEnumAsString(DWORD inWriteFillModeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFillModeEnumAsString)
    {
            PRINTENUMCASE(D3D12_FILL_MODE_WIREFRAME, resultString);
            PRINTENUMCASE(D3D12_FILL_MODE_SOLID, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCullModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteCullModeEnumAsString(DWORD inWriteCullModeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteCullModeEnumAsString)
    {
            PRINTENUMCASE(D3D12_CULL_MODE_NONE, resultString);
            PRINTENUMCASE(D3D12_CULL_MODE_FRONT, resultString);
            PRINTENUMCASE(D3D12_CULL_MODE_BACK, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteComparisonFuncEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteComparisonFuncEnumAsString(DWORD inWriteComparisonFuncEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteComparisonFuncEnumAsString)
    {
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_NEVER, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_LESS, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_EQUAL, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_LESS_EQUAL, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_GREATER, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_NOT_EQUAL, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_GREATER_EQUAL, resultString);
            PRINTENUMCASE(D3D12_COMPARISON_FUNC_ALWAYS, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDepthWriteMaskEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteDepthWriteMaskEnumAsString(DWORD inWriteDepthWriteMaskEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteDepthWriteMaskEnumAsString)
    {
            PRINTENUMCASE(D3D12_DEPTH_WRITE_MASK_ZERO, resultString);
            PRINTENUMCASE(D3D12_DEPTH_WRITE_MASK_ALL, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteStencilOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteStencilOpEnumAsString(DWORD inWriteStencilOpEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteStencilOpEnumAsString)
    {
            PRINTENUMCASE(D3D12_STENCIL_OP_KEEP, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_ZERO, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_REPLACE, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_INCR_SAT, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_DECR_SAT, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_INVERT, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_INCR, resultString);
            PRINTENUMCASE(D3D12_STENCIL_OP_DECR, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBlendEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteBlendEnumAsString(DWORD inWriteBlendEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteBlendEnumAsString)
    {
            PRINTENUMCASE(D3D12_BLEND_ZERO, resultString);
            PRINTENUMCASE(D3D12_BLEND_ONE, resultString);
            PRINTENUMCASE(D3D12_BLEND_SRC_COLOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_SRC_COLOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_SRC_ALPHA, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_SRC_ALPHA, resultString);
            PRINTENUMCASE(D3D12_BLEND_DEST_ALPHA, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_DEST_ALPHA, resultString);
            PRINTENUMCASE(D3D12_BLEND_DEST_COLOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_DEST_COLOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_SRC_ALPHA_SAT, resultString);
            PRINTENUMCASE(D3D12_BLEND_BLEND_FACTOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_BLEND_FACTOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_SRC1_COLOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_SRC1_COLOR, resultString);
            PRINTENUMCASE(D3D12_BLEND_SRC1_ALPHA, resultString);
            PRINTENUMCASE(D3D12_BLEND_INV_SRC1_ALPHA, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBlendOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteBlendOpEnumAsString(DWORD inWriteBlendOpEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteBlendOpEnumAsString)
    {
            PRINTENUMCASE(D3D12_BLEND_OP_ADD, resultString);
            PRINTENUMCASE(D3D12_BLEND_OP_SUBTRACT, resultString);
            PRINTENUMCASE(D3D12_BLEND_OP_REV_SUBTRACT, resultString);
            PRINTENUMCASE(D3D12_BLEND_OP_MIN, resultString);
            PRINTENUMCASE(D3D12_BLEND_OP_MAX, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteColorWriteEnableEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteColorWriteEnableEnumAsString(DWORD inWriteColorWriteEnableEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteColorWriteEnableEnumAsString)
    {
            PRINTENUMCASE(D3D12_COLOR_WRITE_ENABLE_RED, resultString);
            PRINTENUMCASE(D3D12_COLOR_WRITE_ENABLE_GREEN, resultString);
            PRINTENUMCASE(D3D12_COLOR_WRITE_ENABLE_BLUE, resultString);
            PRINTENUMCASE(D3D12_COLOR_WRITE_ENABLE_ALPHA, resultString);
            PRINTENUMCASE(D3D12_COLOR_WRITE_ENABLE_ALL, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteLogicOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteLogicOpEnumAsString(DWORD inWriteLogicOpEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteLogicOpEnumAsString)
    {
            PRINTENUMCASE(D3D12_LOGIC_OP_CLEAR, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_SET, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_COPY, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_COPY_INVERTED, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_NOOP, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_INVERT, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_AND, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_NAND, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_OR, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_NOR, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_XOR, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_EQUIV, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_AND_REVERSE, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_AND_INVERTED, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_OR_REVERSE, resultString);
            PRINTENUMCASE(D3D12_LOGIC_OP_OR_INVERTED, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteConservativeRasterizationModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteConservativeRasterizationModeEnumAsString(DWORD inWriteConservativeRasterizationModeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteConservativeRasterizationModeEnumAsString)
    {
            PRINTENUMCASE(D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF, resultString);
            PRINTENUMCASE(D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteIndexBufferStripCutValueEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteIndexBufferStripCutValueEnumAsString(DWORD inWriteIndexBufferStripCutValueEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteIndexBufferStripCutValueEnumAsString)
    {
            PRINTENUMCASE(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED, resultString);
            PRINTENUMCASE(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF, resultString);
            PRINTENUMCASE(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWritePipelineStateFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WritePipelineStateFlagsEnumAsString(DWORD inWritePipelineStateFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWritePipelineStateFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_PIPELINE_STATE_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposePipelineStateFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WritePipelineStateFlagsEnumAsString, D3D12_PIPELINE_STATE_FLAG_NONE, D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFeatureEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFeatureEnumAsString(DWORD inWriteFeatureEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFeatureEnumAsString)
    {
            PRINTENUMCASE(D3D12_FEATURE_D3D12_OPTIONS, resultString);
            PRINTENUMCASE(D3D12_FEATURE_ARCHITECTURE, resultString);
            PRINTENUMCASE(D3D12_FEATURE_FEATURE_LEVELS, resultString);
            PRINTENUMCASE(D3D12_FEATURE_FORMAT_SUPPORT, resultString);
            PRINTENUMCASE(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, resultString);
            PRINTENUMCASE(D3D12_FEATURE_FORMAT_INFO, resultString);
            PRINTENUMCASE(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteShaderMinPrecisionSupportEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteShaderMinPrecisionSupportEnumAsString(DWORD inWriteShaderMinPrecisionSupportEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteShaderMinPrecisionSupportEnumAsString)
    {
            PRINTENUMCASE(D3D12_SHADER_MIN_PRECISION_SUPPORT_NONE, resultString);
            PRINTENUMCASE(D3D12_SHADER_MIN_PRECISION_SUPPORT_10_BIT, resultString);
            PRINTENUMCASE(D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTiledResourcesTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTiledResourcesTierEnumAsString(DWORD inWriteTiledResourcesTierEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTiledResourcesTierEnumAsString)
    {
            PRINTENUMCASE(D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED, resultString);
            PRINTENUMCASE(D3D12_TILED_RESOURCES_TIER_1, resultString);
            PRINTENUMCASE(D3D12_TILED_RESOURCES_TIER_2, resultString);
            PRINTENUMCASE(D3D12_TILED_RESOURCES_TIER_3, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceBindingTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceBindingTierEnumAsString(DWORD inWriteResourceBindingTierEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceBindingTierEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_BINDING_TIER_1, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_BINDING_TIER_2, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_BINDING_TIER_3, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteConservativeRasterizationTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteConservativeRasterizationTierEnumAsString(DWORD inWriteConservativeRasterizationTierEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteConservativeRasterizationTierEnumAsString)
    {
            PRINTENUMCASE(D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED, resultString);
            PRINTENUMCASE(D3D12_CONSERVATIVE_RASTERIZATION_TIER_1, resultString);
            PRINTENUMCASE(D3D12_CONSERVATIVE_RASTERIZATION_TIER_2, resultString);
            PRINTENUMCASE(D3D12_CONSERVATIVE_RASTERIZATION_TIER_3, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFormatSupport1EnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFormatSupport1EnumAsString(DWORD inWriteFormatSupport1EnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFormatSupport1EnumAsString)
    {
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_NONE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_BUFFER, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_IA_INDEX_BUFFER, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SO_BUFFER, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_TEXTURE1D, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_TEXTURE2D, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_TEXTURE3D, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_TEXTURECUBE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SHADER_LOAD, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_MONO_TEXT, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_MIP, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_RENDER_TARGET, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_BLENDABLE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_DISPLAY, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_CAST_WITHIN_BIT_LAYOUT, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SHADER_GATHER, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_BACK_BUFFER_CAST, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_SHADER_GATHER_COMPARISON, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_DECODER_OUTPUT, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_VIDEO_PROCESSOR_OUTPUT, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_VIDEO_PROCESSOR_INPUT, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT1_VIDEO_ENCODER, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFormatSupport2EnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFormatSupport2EnumAsString(DWORD inWriteFormatSupport2EnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFormatSupport2EnumAsString)
    {
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_NONE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_TILED, resultString);
            PRINTENUMCASE(D3D12_FORMAT_SUPPORT2_MULTIPLANE_OVERLAY, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteMultisampleQualityLevelFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteMultisampleQualityLevelFlagsEnumAsString(DWORD inWriteMultisampleQualityLevelFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteMultisampleQualityLevelFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_TILED_RESOURCE, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeMultisampleQualityLevelFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteMultisampleQualityLevelFlagsEnumAsString, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_TILED_RESOURCE);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCrossNodeSharingTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteCrossNodeSharingTierEnumAsString(DWORD inWriteCrossNodeSharingTierEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteCrossNodeSharingTierEnumAsString)
    {
            PRINTENUMCASE(D3D12_CROSS_NODE_SHARING_TIER_NOT_SUPPORTED, resultString);
            PRINTENUMCASE(D3D12_CROSS_NODE_SHARING_TIER_1_EMULATED, resultString);
            PRINTENUMCASE(D3D12_CROSS_NODE_SHARING_TIER_1, resultString);
            PRINTENUMCASE(D3D12_CROSS_NODE_SHARING_TIER_2, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceHeapTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceHeapTierEnumAsString(DWORD inWriteResourceHeapTierEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceHeapTierEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_HEAP_TIER_1, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_HEAP_TIER_2, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteHeapTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteHeapTypeEnumAsString(DWORD inWriteHeapTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteHeapTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_HEAP_TYPE_DEFAULT, resultString);
            PRINTENUMCASE(D3D12_HEAP_TYPE_UPLOAD, resultString);
            PRINTENUMCASE(D3D12_HEAP_TYPE_READBACK, resultString);
            PRINTENUMCASE(D3D12_HEAP_TYPE_CUSTOM, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCpuPagePropertyEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteCpuPagePropertyEnumAsString(DWORD inWriteCpuPagePropertyEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteCpuPagePropertyEnumAsString)
    {
            PRINTENUMCASE(D3D12_CPU_PAGE_PROPERTY_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE, resultString);
            PRINTENUMCASE(D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE, resultString);
            PRINTENUMCASE(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteMemoryPoolEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteMemoryPoolEnumAsString(DWORD inWriteMemoryPoolEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteMemoryPoolEnumAsString)
    {
            PRINTENUMCASE(D3D12_MEMORY_POOL_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_MEMORY_POOL_L0, resultString);
            PRINTENUMCASE(D3D12_MEMORY_POOL_L1, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteHeapFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteHeapFlagsEnumAsString(DWORD inWriteHeapFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteHeapFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_HEAP_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_SHARED, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_DENY_BUFFERS, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_ALLOW_DISPLAY, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES, resultString);
            //PRINTENUMCASE(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES, resultString);
            PRINTENUMCASE(D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeHeapFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteHeapFlagsEnumAsString, D3D12_HEAP_FLAG_NONE, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceDimensionEnumAsString(DWORD inWriteResourceDimensionEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceDimensionEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_DIMENSION_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_DIMENSION_BUFFER, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_DIMENSION_TEXTURE1D, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_DIMENSION_TEXTURE2D, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_DIMENSION_TEXTURE3D, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTextureLayoutEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTextureLayoutEnumAsString(DWORD inWriteTextureLayoutEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTextureLayoutEnumAsString)
    {
            PRINTENUMCASE(D3D12_TEXTURE_LAYOUT_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_LAYOUT_ROW_MAJOR, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceFlagsEnumAsString(DWORD inWriteResourceFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeResourceFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteResourceFlagsEnumAsString, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTileRangeFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTileRangeFlagsEnumAsString(DWORD inWriteTileRangeFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTileRangeFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_TILE_RANGE_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_TILE_RANGE_FLAG_NULL, resultString);
            PRINTENUMCASE(D3D12_TILE_RANGE_FLAG_SKIP, resultString);
            PRINTENUMCASE(D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeTileRangeFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteTileRangeFlagsEnumAsString, D3D12_TILE_RANGE_FLAG_NONE, D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTileMappingFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTileMappingFlagsEnumAsString(DWORD inWriteTileMappingFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTileMappingFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_TILE_MAPPING_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_TILE_MAPPING_FLAG_NO_HAZARD, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeTileMappingFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteTileMappingFlagsEnumAsString, D3D12_TILE_MAPPING_FLAG_NONE, D3D12_TILE_MAPPING_FLAG_NO_HAZARD);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTileCopyFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTileCopyFlagsEnumAsString(DWORD inWriteTileCopyFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTileCopyFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_TILE_COPY_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_TILE_COPY_FLAG_NO_HAZARD, resultString);
            PRINTENUMCASE(D3D12_TILE_COPY_FLAG_LINEAR_BUFFER_TO_SWIZZLED_TILED_RESOURCE, resultString);
            PRINTENUMCASE(D3D12_TILE_COPY_FLAG_SWIZZLED_TILED_RESOURCE_TO_LINEAR_BUFFER, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeTileCopyFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteTileCopyFlagsEnumAsString, D3D12_TILE_COPY_FLAG_NONE, D3D12_TILE_COPY_FLAG_SWIZZLED_TILED_RESOURCE_TO_LINEAR_BUFFER);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceStatesEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceStatesEnumAsString(DWORD inWriteResourceStatesEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceStatesEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_STATE_COMMON, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_INDEX_BUFFER, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_RENDER_TARGET, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_DEPTH_WRITE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_DEPTH_READ, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_STREAM_OUT, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_COPY_DEST, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_COPY_SOURCE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_RESOLVE_DEST, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_STATE_RESOLVE_SOURCE, resultString);

            // @HACK!!! How can we correctly identify "D3D12_RESOURCE_STATE_PRESENT" or "D3D12_RESOURCE_STATE_PREDICATION" if their values match another enumerant?
            //PRINTENUMCASE(D3D12_RESOURCE_STATE_PRESENT, resultString);
            //PRINTENUMCASE(D3D12_RESOURCE_STATE_PREDICATION, resultString);
    }

    // @HACK!!! This is a special case to handle a bitwise or'd set of flags. Find a way to handle these correctly.
    if (inWriteResourceStatesEnumAsString == D3D12_RESOURCE_STATE_GENERIC_READ)
    {
        resultString = "D3D12_RESOURCE_STATE_GENERIC_READ";
    }

    if (resultString == nullptr)
    {
        resultString = "";
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceBarrierTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceBarrierTypeEnumAsString(DWORD inWriteResourceBarrierTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceBarrierTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_BARRIER_TYPE_ALIASING, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_BARRIER_TYPE_UAV, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceBarrierFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteResourceBarrierFlagsEnumAsString(DWORD inWriteResourceBarrierFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteResourceBarrierFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_RESOURCE_BARRIER_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY, resultString);
            PRINTENUMCASE(D3D12_RESOURCE_BARRIER_FLAG_END_ONLY, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeResourceBarrierFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteResourceBarrierFlagsEnumAsString, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTextureCopyTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTextureCopyTypeEnumAsString(DWORD inWriteTextureCopyTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTextureCopyTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteShaderComponentMappingEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteShaderComponentMappingEnumAsString(DWORD inWriteShaderComponentMappingEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteShaderComponentMappingEnumAsString)
    {
            PRINTENUMCASE(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, resultString);
            PRINTENUMCASE(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1, resultString);
            PRINTENUMCASE(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2, resultString);
            PRINTENUMCASE(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3, resultString);
            PRINTENUMCASE(D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0, resultString);
            PRINTENUMCASE(D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBufferSrvFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteBufferSrvFlagsEnumAsString(DWORD inWriteBufferSrvFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteBufferSrvFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_BUFFER_SRV_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_BUFFER_SRV_FLAG_RAW, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeBufferSrvFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteBufferSrvFlagsEnumAsString, D3D12_BUFFER_SRV_FLAG_NONE, D3D12_BUFFER_SRV_FLAG_RAW);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteSrvDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteSrvDimensionEnumAsString(DWORD inWriteSrvDimensionEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteSrvDimensionEnumAsString)
    {
            PRINTENUMCASE(D3D12_SRV_DIMENSION_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_BUFFER, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE1D, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE1DARRAY, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE2D, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE2DARRAY, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE2DMS, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURE3D, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURECUBE, resultString);
            PRINTENUMCASE(D3D12_SRV_DIMENSION_TEXTURECUBEARRAY, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFilterEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFilterEnumAsString(DWORD inWriteFilterEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFilterEnumAsString)
    {
            PRINTENUMCASE(D3D12_FILTER_MIN_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MIN_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_ANISOTROPIC, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_COMPARISON_ANISOTROPIC, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MINIMUM_ANISOTROPIC, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR, resultString);
            PRINTENUMCASE(D3D12_FILTER_MAXIMUM_ANISOTROPIC, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFilterTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFilterTypeEnumAsString(DWORD inWriteFilterTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFilterTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_FILTER_TYPE_POINT, resultString);
            PRINTENUMCASE(D3D12_FILTER_TYPE_LINEAR, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFilterReductionTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFilterReductionTypeEnumAsString(DWORD inWriteFilterReductionTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFilterReductionTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_FILTER_REDUCTION_TYPE_STANDARD, resultString);
            PRINTENUMCASE(D3D12_FILTER_REDUCTION_TYPE_COMPARISON, resultString);
            PRINTENUMCASE(D3D12_FILTER_REDUCTION_TYPE_MINIMUM, resultString);
            PRINTENUMCASE(D3D12_FILTER_REDUCTION_TYPE_MAXIMUM, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTextureAddressModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteTextureAddressModeEnumAsString(DWORD inWriteTextureAddressModeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteTextureAddressModeEnumAsString)
    {
            PRINTENUMCASE(D3D12_TEXTURE_ADDRESS_MODE_WRAP, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_ADDRESS_MODE_MIRROR, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_ADDRESS_MODE_CLAMP, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_ADDRESS_MODE_BORDER, resultString);
            PRINTENUMCASE(D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBufferUavFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteBufferUavFlagsEnumAsString(DWORD inWriteBufferUavFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteBufferUavFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_BUFFER_UAV_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_BUFFER_UAV_FLAG_RAW, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeBufferUavFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteBufferUavFlagsEnumAsString, D3D12_BUFFER_UAV_FLAG_NONE, D3D12_BUFFER_UAV_FLAG_RAW);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteUavDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteUavDimensionEnumAsString(DWORD inWriteUavDimensionEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteUavDimensionEnumAsString)
    {
            PRINTENUMCASE(D3D12_UAV_DIMENSION_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_UAV_DIMENSION_BUFFER, resultString);
            PRINTENUMCASE(D3D12_UAV_DIMENSION_TEXTURE1D, resultString);
            PRINTENUMCASE(D3D12_UAV_DIMENSION_TEXTURE1DARRAY, resultString);
            PRINTENUMCASE(D3D12_UAV_DIMENSION_TEXTURE2D, resultString);
            PRINTENUMCASE(D3D12_UAV_DIMENSION_TEXTURE2DARRAY, resultString);
            PRINTENUMCASE(D3D12_UAV_DIMENSION_TEXTURE3D, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteRtvDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteRtvDimensionEnumAsString(DWORD inWriteRtvDimensionEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteRtvDimensionEnumAsString)
    {
            PRINTENUMCASE(D3D12_RTV_DIMENSION_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_BUFFER, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE1D, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE1DARRAY, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE2D, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE2DARRAY, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE2DMS, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY, resultString);
            PRINTENUMCASE(D3D12_RTV_DIMENSION_TEXTURE3D, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDsvFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteDsvFlagsEnumAsString(DWORD inWriteDsvFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteDsvFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_DSV_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_DSV_FLAG_READ_ONLY_DEPTH, resultString);
            PRINTENUMCASE(D3D12_DSV_FLAG_READ_ONLY_STENCIL, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeDsvFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteDsvFlagsEnumAsString, D3D12_DSV_FLAG_NONE, D3D12_DSV_FLAG_READ_ONLY_STENCIL);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDsvDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteDsvDimensionEnumAsString(DWORD inWriteDsvDimensionEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteDsvDimensionEnumAsString)
    {
            PRINTENUMCASE(D3D12_DSV_DIMENSION_UNKNOWN, resultString);
            PRINTENUMCASE(D3D12_DSV_DIMENSION_TEXTURE1D, resultString);
            PRINTENUMCASE(D3D12_DSV_DIMENSION_TEXTURE1DARRAY, resultString);
            PRINTENUMCASE(D3D12_DSV_DIMENSION_TEXTURE2D, resultString);
            PRINTENUMCASE(D3D12_DSV_DIMENSION_TEXTURE2DARRAY, resultString);
            PRINTENUMCASE(D3D12_DSV_DIMENSION_TEXTURE2DMS, resultString);
            PRINTENUMCASE(D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteClearFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteClearFlagsEnumAsString(DWORD inWriteClearFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteClearFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_CLEAR_FLAG_DEPTH, resultString);
            PRINTENUMCASE(D3D12_CLEAR_FLAG_STENCIL, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeClearFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteClearFlagsEnumAsString, D3D12_CLEAR_FLAG_DEPTH, D3D12_CLEAR_FLAG_STENCIL);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFenceFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteFenceFlagsEnumAsString(DWORD inWriteFenceFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteFenceFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_FENCE_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_FENCE_FLAG_SHARED, resultString);
            PRINTENUMCASE(D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeFenceFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteFenceFlagsEnumAsString, D3D12_FENCE_FLAG_NONE, D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDescriptorHeapTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteDescriptorHeapTypeEnumAsString(DWORD inWriteDescriptorHeapTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteDescriptorHeapTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDescriptorHeapFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteDescriptorHeapFlagsEnumAsString(DWORD inWriteDescriptorHeapFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteDescriptorHeapFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeDescriptorHeapFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteDescriptorHeapFlagsEnumAsString, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDescriptorRangeTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteDescriptorRangeTypeEnumAsString(DWORD inWriteDescriptorRangeTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteDescriptorRangeTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, resultString);
            PRINTENUMCASE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteShaderVisibilityEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteShaderVisibilityEnumAsString(DWORD inWriteShaderVisibilityEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteShaderVisibilityEnumAsString)
    {
            PRINTENUMCASE(D3D12_SHADER_VISIBILITY_ALL, resultString);
            PRINTENUMCASE(D3D12_SHADER_VISIBILITY_VERTEX, resultString);
            PRINTENUMCASE(D3D12_SHADER_VISIBILITY_HULL, resultString);
            PRINTENUMCASE(D3D12_SHADER_VISIBILITY_DOMAIN, resultString);
            PRINTENUMCASE(D3D12_SHADER_VISIBILITY_GEOMETRY, resultString);
            PRINTENUMCASE(D3D12_SHADER_VISIBILITY_PIXEL, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteRootParameterTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteRootParameterTypeEnumAsString(DWORD inWriteRootParameterTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteRootParameterTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, resultString);
            PRINTENUMCASE(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, resultString);
            PRINTENUMCASE(D3D12_ROOT_PARAMETER_TYPE_CBV, resultString);
            PRINTENUMCASE(D3D12_ROOT_PARAMETER_TYPE_SRV, resultString);
            PRINTENUMCASE(D3D12_ROOT_PARAMETER_TYPE_UAV, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteRootSignatureFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteRootSignatureFlagsEnumAsString(DWORD inWriteRootSignatureFlagsEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteRootSignatureFlagsEnumAsString)
    {
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_NONE, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS, resultString);
            PRINTENUMCASE(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DX12CoreSerializers::DecomposeRootSignatureFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString)
{
    DX12Util::DecomposeFlags(inFlags, ioFlagsString, WriteRootSignatureFlagsEnumAsString, D3D12_ROOT_SIGNATURE_FLAG_NONE, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT);
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteStaticBorderColorEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteStaticBorderColorEnumAsString(DWORD inWriteStaticBorderColorEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteStaticBorderColorEnumAsString)
    {
            PRINTENUMCASE(D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, resultString);
            PRINTENUMCASE(D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, resultString);
            PRINTENUMCASE(D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteQueryHeapTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteQueryHeapTypeEnumAsString(DWORD inWriteQueryHeapTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteQueryHeapTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_QUERY_HEAP_TYPE_OCCLUSION, resultString);
            PRINTENUMCASE(D3D12_QUERY_HEAP_TYPE_TIMESTAMP, resultString);
            PRINTENUMCASE(D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS, resultString);
            PRINTENUMCASE(D3D12_QUERY_HEAP_TYPE_SO_STATISTICS, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteQueryTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteQueryTypeEnumAsString(DWORD inWriteQueryTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteQueryTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_QUERY_TYPE_OCCLUSION, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_BINARY_OCCLUSION, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_TIMESTAMP, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_PIPELINE_STATISTICS, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM1, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM2, resultString);
            PRINTENUMCASE(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM3, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWritePredicationOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WritePredicationOpEnumAsString(DWORD inWritePredicationOpEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWritePredicationOpEnumAsString)
    {
            PRINTENUMCASE(D3D12_PREDICATION_OP_EQUAL_ZERO, resultString);
            PRINTENUMCASE(D3D12_PREDICATION_OP_NOT_EQUAL_ZERO, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteIndirectArgumentTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* DX12CoreSerializers::WriteIndirectArgumentTypeEnumAsString(DWORD inWriteIndirectArgumentTypeEnumAsString)
{
    const char* resultString = nullptr;

    switch (inWriteIndirectArgumentTypeEnumAsString)
    {
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW, resultString);
            PRINTENUMCASE(D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteCommandQueueDescStructAsString(const D3D12_COMMAND_QUEUE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_COMMAND_LIST_TYPE name=\"Type\">%s</D3D12_COMMAND_LIST_TYPE>", DX12CoreSerializers::WriteCommandListTypeEnumAsString(inStruct.Type));
    ioSerializedStructure.appendFormattedString("<INT name=\"Priority\">%d</INT>", inStruct.Priority);
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeCommandQueueFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_COMMAND_QUEUE_FLAGS name=\"Flags\">%s</D3D12_COMMAND_QUEUE_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeMask\">%u</UINT>", inStruct.NodeMask);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_COMMAND_QUEUE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteInputElementDescStructAsString(const D3D12_INPUT_ELEMENT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<LPCSTR name=\"SemanticName\">%s</LPCSTR>", inStruct.SemanticName);
    ioSerializedStructure.appendFormattedString("<UINT name=\"SemanticIndex\">%u</UINT>", inStruct.SemanticIndex);
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<UINT name=\"InputSlot\">%u</UINT>", inStruct.InputSlot);
    ioSerializedStructure.appendFormattedString("<UINT name=\"AlignedByteOffset\">%u</UINT>", inStruct.AlignedByteOffset);
    ioSerializedStructure.appendFormattedString("<D3D12_INPUT_CLASSIFICATION name=\"InputSlotClass\">%s</D3D12_INPUT_CLASSIFICATION>", DX12CoreSerializers::WriteInputClassificationEnumAsString(inStruct.InputSlotClass));
    ioSerializedStructure.appendFormattedString("<UINT name=\"InstanceDataStepRate\">%u</UINT>", inStruct.InstanceDataStepRate);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_INPUT_ELEMENT_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteSoDeclarationEntryStructAsString(const D3D12_SO_DECLARATION_ENTRY inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"Stream\">%u</UINT>", inStruct.Stream);
    ioSerializedStructure.appendFormattedString("<LPCSTR name=\"SemanticName\">%s</LPCSTR>", inStruct.SemanticName);
    ioSerializedStructure.appendFormattedString("<UINT name=\"SemanticIndex\">%u</UINT>", inStruct.SemanticIndex);
    ioSerializedStructure.appendFormattedString("<BYTE name=\"StartComponent\">%d</BYTE>", inStruct.StartComponent);
    ioSerializedStructure.appendFormattedString("<BYTE name=\"ComponentCount\">%d</BYTE>", inStruct.ComponentCount);
    ioSerializedStructure.appendFormattedString("<BYTE name=\"OutputSlot\">%d</BYTE>", inStruct.OutputSlot);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SO_DECLARATION_ENTRY", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteViewportStructAsString(const D3D12_VIEWPORT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"TopLeftX\">%f</FLOAT>", inStruct.TopLeftX);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"TopLeftY\">%f</FLOAT>", inStruct.TopLeftY);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"Width\">%f</FLOAT>", inStruct.Width);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"Height\">%f</FLOAT>", inStruct.Height);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MinDepth\">%f</FLOAT>", inStruct.MinDepth);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MaxDepth\">%f</FLOAT>", inStruct.MaxDepth);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_VIEWPORT", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteBoxStructAsString(const D3D12_BOX inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"left\">%u</UINT>", inStruct.left);
    ioSerializedStructure.appendFormattedString("<UINT name=\"top\">%u</UINT>", inStruct.top);
    ioSerializedStructure.appendFormattedString("<UINT name=\"front\">%u</UINT>", inStruct.front);
    ioSerializedStructure.appendFormattedString("<UINT name=\"right\">%u</UINT>", inStruct.right);
    ioSerializedStructure.appendFormattedString("<UINT name=\"bottom\">%u</UINT>", inStruct.bottom);
    ioSerializedStructure.appendFormattedString("<UINT name=\"back\">%u</UINT>", inStruct.back);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_BOX", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDepthStencilopDescStructAsString(const D3D12_DEPTH_STENCILOP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_STENCIL_OP name=\"StencilFailOp\">%s</D3D12_STENCIL_OP>", DX12CoreSerializers::WriteStencilOpEnumAsString(inStruct.StencilFailOp));
    ioSerializedStructure.appendFormattedString("<D3D12_STENCIL_OP name=\"StencilDepthFailOp\">%s</D3D12_STENCIL_OP>", DX12CoreSerializers::WriteStencilOpEnumAsString(inStruct.StencilDepthFailOp));
    ioSerializedStructure.appendFormattedString("<D3D12_STENCIL_OP name=\"StencilPassOp\">%s</D3D12_STENCIL_OP>", DX12CoreSerializers::WriteStencilOpEnumAsString(inStruct.StencilPassOp));
    ioSerializedStructure.appendFormattedString("<D3D12_COMPARISON_FUNC name=\"StencilFunc\">%s</D3D12_COMPARISON_FUNC>", DX12CoreSerializers::WriteComparisonFuncEnumAsString(inStruct.StencilFunc));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DEPTH_STENCILOP_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDepthStencilDescStructAsString(const D3D12_DEPTH_STENCIL_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<BOOL name=\"DepthEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.DepthEnable));
    ioSerializedStructure.appendFormattedString("<D3D12_DEPTH_WRITE_MASK name=\"DepthWriteMask\">%s</D3D12_DEPTH_WRITE_MASK>", DX12CoreSerializers::WriteDepthWriteMaskEnumAsString(inStruct.DepthWriteMask));
    ioSerializedStructure.appendFormattedString("<D3D12_COMPARISON_FUNC name=\"DepthFunc\">%s</D3D12_COMPARISON_FUNC>", DX12CoreSerializers::WriteComparisonFuncEnumAsString(inStruct.DepthFunc));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"StencilEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.StencilEnable));
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"StencilReadMask\">%hhu</UINT8>", inStruct.StencilReadMask);
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"StencilWriteMask\">%hhu</UINT8>", inStruct.StencilWriteMask);
    gtASCIIString FrontFaceString;
    DX12CoreSerializers::WriteDepthStencilopDescStructAsString(inStruct.FrontFace, FrontFaceString, "FrontFace");
    ioSerializedStructure.appendFormattedString("%s", FrontFaceString.asCharArray());
    gtASCIIString BackFaceString;
    DX12CoreSerializers::WriteDepthStencilopDescStructAsString(inStruct.BackFace, BackFaceString, "BackFace");
    ioSerializedStructure.appendFormattedString("%s", BackFaceString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DEPTH_STENCIL_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRenderTargetBlendDescStructAsString(const D3D12_RENDER_TARGET_BLEND_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<BOOL name=\"BlendEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.BlendEnable));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"LogicOpEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.LogicOpEnable));
    ioSerializedStructure.appendFormattedString("<D3D12_BLEND name=\"SrcBlend\">%s</D3D12_BLEND>", DX12CoreSerializers::WriteBlendEnumAsString(inStruct.SrcBlend));
    ioSerializedStructure.appendFormattedString("<D3D12_BLEND name=\"DestBlend\">%s</D3D12_BLEND>", DX12CoreSerializers::WriteBlendEnumAsString(inStruct.DestBlend));
    ioSerializedStructure.appendFormattedString("<D3D12_BLEND_OP name=\"BlendOp\">%s</D3D12_BLEND_OP>", DX12CoreSerializers::WriteBlendOpEnumAsString(inStruct.BlendOp));
    ioSerializedStructure.appendFormattedString("<D3D12_BLEND name=\"SrcBlendAlpha\">%s</D3D12_BLEND>", DX12CoreSerializers::WriteBlendEnumAsString(inStruct.SrcBlendAlpha));
    ioSerializedStructure.appendFormattedString("<D3D12_BLEND name=\"DestBlendAlpha\">%s</D3D12_BLEND>", DX12CoreSerializers::WriteBlendEnumAsString(inStruct.DestBlendAlpha));
    ioSerializedStructure.appendFormattedString("<D3D12_BLEND_OP name=\"BlendOpAlpha\">%s</D3D12_BLEND_OP>", DX12CoreSerializers::WriteBlendOpEnumAsString(inStruct.BlendOpAlpha));
    ioSerializedStructure.appendFormattedString("<D3D12_LOGIC_OP name=\"LogicOp\">%s</D3D12_LOGIC_OP>", DX12CoreSerializers::WriteLogicOpEnumAsString(inStruct.LogicOp));
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"RenderTargetWriteMask\">%hhu</UINT8>", inStruct.RenderTargetWriteMask);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RENDER_TARGET_BLEND_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteBlendDescStructAsString(const D3D12_BLEND_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<BOOL name=\"AlphaToCoverageEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.AlphaToCoverageEnable));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"IndependentBlendEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.IndependentBlendEnable));

    for (UINT index = 0; index < 8; index++)
    {
        gtASCIIString RenderTargetString;
        DX12CoreSerializers::WriteRenderTargetBlendDescStructAsString(inStruct.RenderTarget[index], RenderTargetString, "RenderTarget");
        ioSerializedStructure.appendFormattedString("%s", RenderTargetString.asCharArray());
    }

    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_BLEND_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRasterizerDescStructAsString(const D3D12_RASTERIZER_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_FILL_MODE name=\"FillMode\">%s</D3D12_FILL_MODE>", DX12CoreSerializers::WriteFillModeEnumAsString(inStruct.FillMode));
    ioSerializedStructure.appendFormattedString("<D3D12_CULL_MODE name=\"CullMode\">%s</D3D12_CULL_MODE>", DX12CoreSerializers::WriteCullModeEnumAsString(inStruct.CullMode));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"FrontCounterClockwise\">%s</BOOL>", DX12Util::PrintBool(inStruct.FrontCounterClockwise));
    ioSerializedStructure.appendFormattedString("<INT name=\"DepthBias\">%d</INT>", inStruct.DepthBias);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"DepthBiasClamp\">%f</FLOAT>", inStruct.DepthBiasClamp);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"SlopeScaledDepthBias\">%f</FLOAT>", inStruct.SlopeScaledDepthBias);
    ioSerializedStructure.appendFormattedString("<BOOL name=\"DepthClipEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.DepthClipEnable));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"MultisampleEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.MultisampleEnable));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"AntialiasedLineEnable\">%s</BOOL>", DX12Util::PrintBool(inStruct.AntialiasedLineEnable));
    ioSerializedStructure.appendFormattedString("<UINT name=\"ForcedSampleCount\">%u</UINT>", inStruct.ForcedSampleCount);
    ioSerializedStructure.appendFormattedString("<D3D12_CONSERVATIVE_RASTERIZATION_MODE name=\"ConservativeRaster\">%s</D3D12_CONSERVATIVE_RASTERIZATION_MODE>", DX12CoreSerializers::WriteConservativeRasterizationModeEnumAsString(inStruct.ConservativeRaster));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RASTERIZER_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteShaderBytecodeStructAsString(const D3D12_SHADER_BYTECODE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<void name=\"pShaderBytecode\">0x%p</void>", inStruct.pShaderBytecode);
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"BytecodeLength\">%Iu</SIZE_T>", inStruct.BytecodeLength);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SHADER_BYTECODE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteStreamOutputDescStructAsString(const D3D12_STREAM_OUTPUT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    for (UINT index = 0; index < inStruct.NumEntries; index++)
    {
        gtASCIIString pSODeclarationString;
        DX12CoreSerializers::WriteSoDeclarationEntryStructAsString(inStruct.pSODeclaration[index], pSODeclarationString, "pSODeclaration");
        ioSerializedStructure.appendFormattedString("%s", pSODeclarationString.asCharArray());
    }

    ioSerializedStructure.appendFormattedString("<UINT name=\"NumEntries\">%u</UINT>", inStruct.NumEntries);

    for (UINT index = 0; index < inStruct.NumStrides; index++)
    {
        ioSerializedStructure.appendFormattedString("<UINT name=\"pBufferStrides\">%u</UINT>", inStruct.pBufferStrides[index]);
    }

    ioSerializedStructure.appendFormattedString("<UINT name=\"NumStrides\">%u</UINT>", inStruct.NumStrides);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RasterizedStream\">%u</UINT>", inStruct.RasterizedStream);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_STREAM_OUTPUT_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteInputLayoutDescStructAsString(const D3D12_INPUT_LAYOUT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    for (UINT index = 0; index < inStruct.NumElements; index++)
    {
        gtASCIIString pInputElementDescsString;
        DX12CoreSerializers::WriteInputElementDescStructAsString(inStruct.pInputElementDescs[index], pInputElementDescsString, "pInputElementDescs");
        ioSerializedStructure.appendFormattedString("%s", pInputElementDescsString.asCharArray());
    }

    ioSerializedStructure.appendFormattedString("<UINT name=\"NumElements\">%u</UINT>", inStruct.NumElements);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_INPUT_LAYOUT_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteCachedPipelineStateStructAsString(const D3D12_CACHED_PIPELINE_STATE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<void name=\"pCachedBlob\">0x%p</void>", inStruct.pCachedBlob);
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"CachedBlobSizeInBytes\">%Iu</SIZE_T>", inStruct.CachedBlobSizeInBytes);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_CACHED_PIPELINE_STATE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteGraphicsPipelineStateDescStructAsString(const D3D12_GRAPHICS_PIPELINE_STATE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<ID3D12RootSignature name=\"pRootSignature\">+0x%p</ID3D12RootSignature>", inStruct.pRootSignature);
    gtASCIIString VSString;
    DX12CoreSerializers::WriteShaderBytecodeStructAsString(inStruct.VS, VSString, "VS");
    ioSerializedStructure.appendFormattedString("%s", VSString.asCharArray());
    gtASCIIString PSString;
    DX12CoreSerializers::WriteShaderBytecodeStructAsString(inStruct.PS, PSString, "PS");
    ioSerializedStructure.appendFormattedString("%s", PSString.asCharArray());
    gtASCIIString DSString;
    DX12CoreSerializers::WriteShaderBytecodeStructAsString(inStruct.DS, DSString, "DS");
    ioSerializedStructure.appendFormattedString("%s", DSString.asCharArray());
    gtASCIIString HSString;
    DX12CoreSerializers::WriteShaderBytecodeStructAsString(inStruct.HS, HSString, "HS");
    ioSerializedStructure.appendFormattedString("%s", HSString.asCharArray());
    gtASCIIString GSString;
    DX12CoreSerializers::WriteShaderBytecodeStructAsString(inStruct.GS, GSString, "GS");
    ioSerializedStructure.appendFormattedString("%s", GSString.asCharArray());
    gtASCIIString StreamOutputString;
    DX12CoreSerializers::WriteStreamOutputDescStructAsString(inStruct.StreamOutput, StreamOutputString, "StreamOutput");
    ioSerializedStructure.appendFormattedString("%s", StreamOutputString.asCharArray());
    gtASCIIString BlendStateString;
    DX12CoreSerializers::WriteBlendDescStructAsString(inStruct.BlendState, BlendStateString, "BlendState");
    ioSerializedStructure.appendFormattedString("%s", BlendStateString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT name=\"SampleMask\">%u</UINT>", inStruct.SampleMask);
    gtASCIIString RasterizerStateString;
    DX12CoreSerializers::WriteRasterizerDescStructAsString(inStruct.RasterizerState, RasterizerStateString, "RasterizerState");
    ioSerializedStructure.appendFormattedString("%s", RasterizerStateString.asCharArray());
    gtASCIIString DepthStencilStateString;
    DX12CoreSerializers::WriteDepthStencilDescStructAsString(inStruct.DepthStencilState, DepthStencilStateString, "DepthStencilState");
    ioSerializedStructure.appendFormattedString("%s", DepthStencilStateString.asCharArray());
    gtASCIIString InputLayoutString;
    DX12CoreSerializers::WriteInputLayoutDescStructAsString(inStruct.InputLayout, InputLayoutString, "InputLayout");
    ioSerializedStructure.appendFormattedString("%s", InputLayoutString.asCharArray());
    ioSerializedStructure.appendFormattedString("<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE name=\"IBStripCutValue\">%s</D3D12_INDEX_BUFFER_STRIP_CUT_VALUE>", DX12CoreSerializers::WriteIndexBufferStripCutValueEnumAsString(inStruct.IBStripCutValue));
    ioSerializedStructure.appendFormattedString("<D3D12_PRIMITIVE_TOPOLOGY_TYPE name=\"PrimitiveTopologyType\">%s</D3D12_PRIMITIVE_TOPOLOGY_TYPE>", DX12CoreSerializers::WritePrimitiveTopologyTypeEnumAsString(inStruct.PrimitiveTopologyType));
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumRenderTargets\">%u</UINT>", inStruct.NumRenderTargets);

    for (UINT index = 0; index < 8; index++)
    {
        ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"RTVFormats\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.RTVFormats[index]));
    }

    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"DSVFormat\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.DSVFormat));
    gtASCIIString sampleDescString;
    DX12CustomSerializers::WriteDXGISampleDesc(inStruct.SampleDesc, sampleDescString, "SampleDesc");
    ioSerializedStructure.appendFormattedString("%s", sampleDescString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeMask\">%u</UINT>", inStruct.NodeMask);
    gtASCIIString CachedPSOString;
    DX12CoreSerializers::WriteCachedPipelineStateStructAsString(inStruct.CachedPSO, CachedPSOString, "CachedPSO");
    ioSerializedStructure.appendFormattedString("%s", CachedPSOString.asCharArray());
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposePipelineStateFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_PIPELINE_STATE_FLAGS name=\"Flags\">%s</D3D12_PIPELINE_STATE_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_GRAPHICS_PIPELINE_STATE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteComputePipelineStateDescStructAsString(const D3D12_COMPUTE_PIPELINE_STATE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<ID3D12RootSignature name=\"pRootSignature\">+0x%p</ID3D12RootSignature>", inStruct.pRootSignature);
    gtASCIIString CSString;
    DX12CoreSerializers::WriteShaderBytecodeStructAsString(inStruct.CS, CSString, "CS");
    ioSerializedStructure.appendFormattedString("%s", CSString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeMask\">%u</UINT>", inStruct.NodeMask);
    gtASCIIString CachedPSOString;
    DX12CoreSerializers::WriteCachedPipelineStateStructAsString(inStruct.CachedPSO, CachedPSOString, "CachedPSO");
    ioSerializedStructure.appendFormattedString("%s", CachedPSOString.asCharArray());
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposePipelineStateFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_PIPELINE_STATE_FLAGS name=\"Flags\">%s</D3D12_PIPELINE_STATE_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_COMPUTE_PIPELINE_STATE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureData_optionsStructAsString(const D3D12_FEATURE_DATA_D3D12_OPTIONS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<BOOL name=\"DoublePrecisionFloatShaderOps\">%s</BOOL>", DX12Util::PrintBool(inStruct.DoublePrecisionFloatShaderOps));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"OutputMergerLogicOp\">%s</BOOL>", DX12Util::PrintBool(inStruct.OutputMergerLogicOp));
    ioSerializedStructure.appendFormattedString("<D3D12_SHADER_MIN_PRECISION_SUPPORT name=\"MinPrecisionSupport\">%s</D3D12_SHADER_MIN_PRECISION_SUPPORT>", DX12CoreSerializers::WriteShaderMinPrecisionSupportEnumAsString(inStruct.MinPrecisionSupport));
    ioSerializedStructure.appendFormattedString("<D3D12_TILED_RESOURCES_TIER name=\"TiledResourcesTier\">%s</D3D12_TILED_RESOURCES_TIER>", DX12CoreSerializers::WriteTiledResourcesTierEnumAsString(inStruct.TiledResourcesTier));
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_BINDING_TIER name=\"ResourceBindingTier\">%s</D3D12_RESOURCE_BINDING_TIER>", DX12CoreSerializers::WriteResourceBindingTierEnumAsString(inStruct.ResourceBindingTier));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"PSSpecifiedStencilRefSupported\">%s</BOOL>", DX12Util::PrintBool(inStruct.PSSpecifiedStencilRefSupported));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"TypedUAVLoadAdditionalFormats\">%s</BOOL>", DX12Util::PrintBool(inStruct.TypedUAVLoadAdditionalFormats));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"ROVsSupported\">%s</BOOL>", DX12Util::PrintBool(inStruct.ROVsSupported));
    ioSerializedStructure.appendFormattedString("<D3D12_CONSERVATIVE_RASTERIZATION_TIER name=\"ConservativeRasterizationTier\">%s</D3D12_CONSERVATIVE_RASTERIZATION_TIER>", DX12CoreSerializers::WriteConservativeRasterizationTierEnumAsString(inStruct.ConservativeRasterizationTier));
    ioSerializedStructure.appendFormattedString("<UINT name=\"MaxGPUVirtualAddressBitsPerResource\">%u</UINT>", inStruct.MaxGPUVirtualAddressBitsPerResource);
    ioSerializedStructure.appendFormattedString("<BOOL name=\"StandardSwizzle64KBSupported\">%s</BOOL>", DX12Util::PrintBool(inStruct.StandardSwizzle64KBSupported));
    ioSerializedStructure.appendFormattedString("<D3D12_CROSS_NODE_SHARING_TIER name=\"CrossNodeSharingTier\">%s</D3D12_CROSS_NODE_SHARING_TIER>", DX12CoreSerializers::WriteCrossNodeSharingTierEnumAsString(inStruct.CrossNodeSharingTier));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"CrossAdapterRowMajorTextureSupported\">%s</BOOL>", DX12Util::PrintBool(inStruct.CrossAdapterRowMajorTextureSupported));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation\">%s</BOOL>", DX12Util::PrintBool(inStruct.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation));
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_HEAP_TIER name=\"ResourceHeapTier\">%s</D3D12_RESOURCE_HEAP_TIER>", DX12CoreSerializers::WriteResourceHeapTierEnumAsString(inStruct.ResourceHeapTier));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_D3D12_OPTIONS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureDataArchitectureStructAsString(const D3D12_FEATURE_DATA_ARCHITECTURE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeIndex\">%u</UINT>", inStruct.NodeIndex);
    ioSerializedStructure.appendFormattedString("<BOOL name=\"TileBasedRenderer\">%s</BOOL>", DX12Util::PrintBool(inStruct.TileBasedRenderer));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"UMA\">%s</BOOL>", DX12Util::PrintBool(inStruct.UMA));
    ioSerializedStructure.appendFormattedString("<BOOL name=\"CacheCoherentUMA\">%s</BOOL>", DX12Util::PrintBool(inStruct.CacheCoherentUMA));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_ARCHITECTURE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureDataFeatureLevelsStructAsString(const D3D12_FEATURE_DATA_FEATURE_LEVELS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumFeatureLevels\">%u</UINT>", inStruct.NumFeatureLevels);

    for (UINT index = 0; index < inStruct.NumFeatureLevels; index++)
    {
        ioSerializedStructure.appendFormattedString("<D3D_FEATURE_LEVEL name=\"pFeatureLevelsRequested\">%s</D3D_FEATURE_LEVEL>", DX12CustomSerializers::WriteD3DFeatureLevel(inStruct.pFeatureLevelsRequested[index]));
    }

    ioSerializedStructure.appendFormattedString("<D3D_FEATURE_LEVEL name=\"MaxSupportedFeatureLevel\">%s</D3D_FEATURE_LEVEL>", DX12CustomSerializers::WriteD3DFeatureLevel(inStruct.MaxSupportedFeatureLevel));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_FEATURE_LEVELS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureDataFormatSupportStructAsString(const D3D12_FEATURE_DATA_FORMAT_SUPPORT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<D3D12_FORMAT_SUPPORT1 name=\"Support1\">%s</D3D12_FORMAT_SUPPORT1>", DX12CoreSerializers::WriteFormatSupport1EnumAsString(inStruct.Support1));
    ioSerializedStructure.appendFormattedString("<D3D12_FORMAT_SUPPORT2 name=\"Support2\">%s</D3D12_FORMAT_SUPPORT2>", DX12CoreSerializers::WriteFormatSupport2EnumAsString(inStruct.Support2));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_FORMAT_SUPPORT", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureDataMultisampleQualityLevelsStructAsString(const D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<UINT name=\"SampleCount\">%u</UINT>", inStruct.SampleCount);
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeMultisampleQualityLevelFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS name=\"Flags\">%s</D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumQualityLevels\">%u</UINT>", inStruct.NumQualityLevels);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureDataFormatInfoStructAsString(const D3D12_FEATURE_DATA_FORMAT_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"PlaneCount\">%hhu</UINT8>", inStruct.PlaneCount);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_FORMAT_INFO", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteFeatureDataGpuVirtualAddressSupportStructAsString(const D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MaxGPUVirtualAddressBitsPerResource\">%u</UINT>", inStruct.MaxGPUVirtualAddressBitsPerResource);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MaxGPUVirtualAddressBitsPerProcess\">%u</UINT>", inStruct.MaxGPUVirtualAddressBitsPerProcess);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteResourceAllocationInfoStructAsString(const D3D12_RESOURCE_ALLOCATION_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"SizeInBytes\">%llu</UINT64>", inStruct.SizeInBytes);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"Alignment\">%llu</UINT64>", inStruct.Alignment);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RESOURCE_ALLOCATION_INFO", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteHeapPropertiesStructAsString(const D3D12_HEAP_PROPERTIES inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_HEAP_TYPE name=\"Type\">%s</D3D12_HEAP_TYPE>", DX12CoreSerializers::WriteHeapTypeEnumAsString(inStruct.Type));
    ioSerializedStructure.appendFormattedString("<D3D12_CPU_PAGE_PROPERTY name=\"CPUPageProperty\">%s</D3D12_CPU_PAGE_PROPERTY>", DX12CoreSerializers::WriteCpuPagePropertyEnumAsString(inStruct.CPUPageProperty));
    ioSerializedStructure.appendFormattedString("<D3D12_MEMORY_POOL name=\"MemoryPoolPreference\">%s</D3D12_MEMORY_POOL>", DX12CoreSerializers::WriteMemoryPoolEnumAsString(inStruct.MemoryPoolPreference));
    ioSerializedStructure.appendFormattedString("<UINT name=\"CreationNodeMask\">%u</UINT>", inStruct.CreationNodeMask);
    ioSerializedStructure.appendFormattedString("<UINT name=\"VisibleNodeMask\">%u</UINT>", inStruct.VisibleNodeMask);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_HEAP_PROPERTIES", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteHeapDescStructAsString(const D3D12_HEAP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"SizeInBytes\">%llu</UINT64>", inStruct.SizeInBytes);
    gtASCIIString PropertiesString;
    DX12CoreSerializers::WriteHeapPropertiesStructAsString(inStruct.Properties, PropertiesString, "Properties");
    ioSerializedStructure.appendFormattedString("%s", PropertiesString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"Alignment\">%llu</UINT64>", inStruct.Alignment);
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeHeapFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_HEAP_FLAGS name=\"Flags\">%s</D3D12_HEAP_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_HEAP_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteResourceDescStructAsString(const D3D12_RESOURCE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_DIMENSION name=\"Dimension\">%s</D3D12_RESOURCE_DIMENSION>", DX12CoreSerializers::WriteResourceDimensionEnumAsString(inStruct.Dimension));
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"Alignment\">%llu</UINT64>", inStruct.Alignment);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"Width\">%llu</UINT64>", inStruct.Width);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Height\">%u</UINT>", inStruct.Height);
    ioSerializedStructure.appendFormattedString("<UINT16 name=\"DepthOrArraySize\">%hu</UINT16>", inStruct.DepthOrArraySize);
    ioSerializedStructure.appendFormattedString("<UINT16 name=\"MipLevels\">%hu</UINT16>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    gtASCIIString sampleDescString;
    DX12CustomSerializers::WriteDXGISampleDesc(inStruct.SampleDesc, sampleDescString, "SampleDesc");
    ioSerializedStructure.appendFormattedString("%s", sampleDescString.asCharArray());
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_LAYOUT name=\"Layout\">%s</D3D12_TEXTURE_LAYOUT>", DX12CoreSerializers::WriteTextureLayoutEnumAsString(inStruct.Layout));
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeResourceFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_FLAGS name=\"Flags\">%s</D3D12_RESOURCE_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RESOURCE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDepthStencilValueStructAsString(const D3D12_DEPTH_STENCIL_VALUE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"Depth\">%f</FLOAT>", inStruct.Depth);
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"Stencil\">%hhu</UINT8>", inStruct.Stencil);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DEPTH_STENCIL_VALUE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_CLEAR_VALUE' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteClearValueStructAsString(const D3D12_CLEAR_VALUE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_CLEAR_VALUE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRangeStructAsString(const D3D12_RANGE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"Begin\">%Iu</SIZE_T>", inStruct.Begin);
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"End\">%Iu</SIZE_T>", inStruct.End);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RANGE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteSubresourceInfoStructAsString(const D3D12_SUBRESOURCE_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"Offset\">%llu</UINT64>", inStruct.Offset);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RowPitch\">%u</UINT>", inStruct.RowPitch);
    ioSerializedStructure.appendFormattedString("<UINT name=\"DepthPitch\">%u</UINT>", inStruct.DepthPitch);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SUBRESOURCE_INFO", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTiledResourceCoordinateStructAsString(const D3D12_TILED_RESOURCE_COORDINATE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"X\">%u</UINT>", inStruct.X);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Y\">%u</UINT>", inStruct.Y);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Z\">%u</UINT>", inStruct.Z);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Subresource\">%u</UINT>", inStruct.Subresource);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TILED_RESOURCE_COORDINATE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTileRegionSizeStructAsString(const D3D12_TILE_REGION_SIZE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumTiles\">%u</UINT>", inStruct.NumTiles);
    ioSerializedStructure.appendFormattedString("<BOOL name=\"UseBox\">%s</BOOL>", DX12Util::PrintBool(inStruct.UseBox));
    ioSerializedStructure.appendFormattedString("<UINT name=\"Width\">%u</UINT>", inStruct.Width);
    ioSerializedStructure.appendFormattedString("<UINT16 name=\"Height\">%hu</UINT16>", inStruct.Height);
    ioSerializedStructure.appendFormattedString("<UINT16 name=\"Depth\">%hu</UINT16>", inStruct.Depth);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TILE_REGION_SIZE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteSubresourceTilingStructAsString(const D3D12_SUBRESOURCE_TILING inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"WidthInTiles\">%u</UINT>", inStruct.WidthInTiles);
    ioSerializedStructure.appendFormattedString("<UINT16 name=\"HeightInTiles\">%hu</UINT16>", inStruct.HeightInTiles);
    ioSerializedStructure.appendFormattedString("<UINT16 name=\"DepthInTiles\">%hu</UINT16>", inStruct.DepthInTiles);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StartTileIndexInOverallResource\">%u</UINT>", inStruct.StartTileIndexInOverallResource);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SUBRESOURCE_TILING", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTileShapeStructAsString(const D3D12_TILE_SHAPE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"WidthInTexels\">%u</UINT>", inStruct.WidthInTexels);
    ioSerializedStructure.appendFormattedString("<UINT name=\"HeightInTexels\">%u</UINT>", inStruct.HeightInTexels);
    ioSerializedStructure.appendFormattedString("<UINT name=\"DepthInTexels\">%u</UINT>", inStruct.DepthInTexels);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TILE_SHAPE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WritePackedMipInfoStructAsString(const D3D12_PACKED_MIP_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"NumStandardMips\">%hhu</UINT8>", inStruct.NumStandardMips);
    ioSerializedStructure.appendFormattedString("<UINT8 name=\"NumPackedMips\">%hhu</UINT8>", inStruct.NumPackedMips);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumTilesForPackedMips\">%u</UINT>", inStruct.NumTilesForPackedMips);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StartTileIndexInOverallResource\">%u</UINT>", inStruct.StartTileIndexInOverallResource);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_PACKED_MIP_INFO", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteResourceTransitionBarrierStructAsString(const D3D12_RESOURCE_TRANSITION_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<ID3D12Resource name=\"pResource\">+0x%p</ID3D12Resource>", inStruct.pResource);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Subresource\">%u</UINT>", inStruct.Subresource);
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_STATES name=\"StateBefore\">%s</D3D12_RESOURCE_STATES>", DX12CoreSerializers::WriteResourceStatesEnumAsString(inStruct.StateBefore));
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_STATES name=\"StateAfter\">%s</D3D12_RESOURCE_STATES>", DX12CoreSerializers::WriteResourceStatesEnumAsString(inStruct.StateAfter));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RESOURCE_TRANSITION_BARRIER", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteResourceAliasingBarrierStructAsString(const D3D12_RESOURCE_ALIASING_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<ID3D12Resource name=\"pResourceBefore\">+0x%p</ID3D12Resource>", inStruct.pResourceBefore);
    ioSerializedStructure.appendFormattedString("<ID3D12Resource name=\"pResourceAfter\">+0x%p</ID3D12Resource>", inStruct.pResourceAfter);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RESOURCE_ALIASING_BARRIER", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteResourceUavBarrierStructAsString(const D3D12_RESOURCE_UAV_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<ID3D12Resource name=\"pResource\">+0x%p</ID3D12Resource>", inStruct.pResource);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RESOURCE_UAV_BARRIER", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_RESOURCE_BARRIER' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteResourceBarrierStructAsString(const D3D12_RESOURCE_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_BARRIER_TYPE name=\"Type\">%s</D3D12_RESOURCE_BARRIER_TYPE>", DX12CoreSerializers::WriteResourceBarrierTypeEnumAsString(inStruct.Type));
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeResourceBarrierFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_RESOURCE_BARRIER_FLAGS name=\"Flags\">%s</D3D12_RESOURCE_BARRIER_FLAGS>", FlagsString.asCharArray());
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RESOURCE_BARRIER", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteSubresourceFootprintStructAsString(const D3D12_SUBRESOURCE_FOOTPRINT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<UINT name=\"Width\">%u</UINT>", inStruct.Width);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Height\">%u</UINT>", inStruct.Height);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Depth\">%u</UINT>", inStruct.Depth);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RowPitch\">%u</UINT>", inStruct.RowPitch);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SUBRESOURCE_FOOTPRINT", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WritePlacedSubresourceFootprintStructAsString(const D3D12_PLACED_SUBRESOURCE_FOOTPRINT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"Offset\">%llu</UINT64>", inStruct.Offset);
    gtASCIIString FootprintString;
    DX12CoreSerializers::WriteSubresourceFootprintStructAsString(inStruct.Footprint, FootprintString, "Footprint");
    ioSerializedStructure.appendFormattedString("%s", FootprintString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_PLACED_SUBRESOURCE_FOOTPRINT", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_TEXTURE_COPY_LOCATION' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteTextureCopyLocationStructAsString(const D3D12_TEXTURE_COPY_LOCATION inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<ID3D12Resource name=\"pResource\">+0x%p</ID3D12Resource>", inStruct.pResource);
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_COPY_TYPE name=\"Type\">%s</D3D12_TEXTURE_COPY_TYPE>", DX12CoreSerializers::WriteTextureCopyTypeEnumAsString(inStruct.Type));
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEXTURE_COPY_LOCATION", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteBufferSrvStructAsString(const D3D12_BUFFER_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"FirstElement\">%llu</UINT64>", inStruct.FirstElement);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumElements\">%u</UINT>", inStruct.NumElements);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StructureByteStride\">%u</UINT>", inStruct.StructureByteStride);
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeBufferSrvFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_BUFFER_SRV_FLAGS name=\"Flags\">%s</D3D12_BUFFER_SRV_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_BUFFER_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dSrvStructAsString(const D3D12_TEX1D_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dArraySrvStructAsString(const D3D12_TEX1D_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_ARRAY_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dSrvStructAsString(const D3D12_TEX2D_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<UINT name=\"PlaneSlice\">%u</UINT>", inStruct.PlaneSlice);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dArraySrvStructAsString(const D3D12_TEX2D_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure.appendFormattedString("<UINT name=\"PlaneSlice\">%u</UINT>", inStruct.PlaneSlice);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_ARRAY_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex3dSrvStructAsString(const D3D12_TEX3D_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX3D_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTexcubeSrvStructAsString(const D3D12_TEXCUBE_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEXCUBE_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTexcubeArraySrvStructAsString(const D3D12_TEXCUBE_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MostDetailedMip\">%u</UINT>", inStruct.MostDetailedMip);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipLevels\">%u</UINT>", inStruct.MipLevels);
    ioSerializedStructure.appendFormattedString("<UINT name=\"First2DArrayFace\">%u</UINT>", inStruct.First2DArrayFace);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumCubes\">%u</UINT>", inStruct.NumCubes);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"ResourceMinLODClamp\">%f</FLOAT>", inStruct.ResourceMinLODClamp);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEXCUBE_ARRAY_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dmsSrvStructAsString(const D3D12_TEX2DMS_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"UnusedField_NothingToDefine\">%u</UINT>", inStruct.UnusedField_NothingToDefine);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2DMS_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dmsArraySrvStructAsString(const D3D12_TEX2DMS_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2DMS_ARRAY_SRV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_SHADER_RESOURCE_VIEW_DESC' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteShaderResourceViewDescStructAsString(const D3D12_SHADER_RESOURCE_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<D3D12_SRV_DIMENSION name=\"ViewDimension\">%s</D3D12_SRV_DIMENSION>", DX12CoreSerializers::WriteSrvDimensionEnumAsString(inStruct.ViewDimension));
    ioSerializedStructure.appendFormattedString("<UINT name=\"Shader4ComponentMapping\">%u</UINT>", inStruct.Shader4ComponentMapping);
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SHADER_RESOURCE_VIEW_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteConstantBufferViewDescStructAsString(const D3D12_CONSTANT_BUFFER_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_GPU_VIRTUAL_ADDRESS name=\"BufferLocation\">%llu</D3D12_GPU_VIRTUAL_ADDRESS>", inStruct.BufferLocation);
    ioSerializedStructure.appendFormattedString("<UINT name=\"SizeInBytes\">%u</UINT>", inStruct.SizeInBytes);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_CONSTANT_BUFFER_VIEW_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteSamplerDescStructAsString(const D3D12_SAMPLER_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_FILTER name=\"Filter\">%s</D3D12_FILTER>", DX12CoreSerializers::WriteFilterEnumAsString(inStruct.Filter));
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_ADDRESS_MODE name=\"AddressU\">%s</D3D12_TEXTURE_ADDRESS_MODE>", DX12CoreSerializers::WriteTextureAddressModeEnumAsString(inStruct.AddressU));
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_ADDRESS_MODE name=\"AddressV\">%s</D3D12_TEXTURE_ADDRESS_MODE>", DX12CoreSerializers::WriteTextureAddressModeEnumAsString(inStruct.AddressV));
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_ADDRESS_MODE name=\"AddressW\">%s</D3D12_TEXTURE_ADDRESS_MODE>", DX12CoreSerializers::WriteTextureAddressModeEnumAsString(inStruct.AddressW));
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MipLODBias\">%f</FLOAT>", inStruct.MipLODBias);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MaxAnisotropy\">%u</UINT>", inStruct.MaxAnisotropy);
    ioSerializedStructure.appendFormattedString("<D3D12_COMPARISON_FUNC name=\"ComparisonFunc\">%s</D3D12_COMPARISON_FUNC>", DX12CoreSerializers::WriteComparisonFuncEnumAsString(inStruct.ComparisonFunc));

    for (UINT index = 0; index < 4; index++)
    {
        ioSerializedStructure.appendFormattedString("<FLOAT name=\"BorderColor\">%f</FLOAT>", inStruct.BorderColor[index]);
    }

    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MinLOD\">%f</FLOAT>", inStruct.MinLOD);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MaxLOD\">%f</FLOAT>", inStruct.MaxLOD);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SAMPLER_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteBufferUavStructAsString(const D3D12_BUFFER_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"FirstElement\">%llu</UINT64>", inStruct.FirstElement);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumElements\">%u</UINT>", inStruct.NumElements);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StructureByteStride\">%u</UINT>", inStruct.StructureByteStride);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"CounterOffsetInBytes\">%llu</UINT64>", inStruct.CounterOffsetInBytes);
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeBufferUavFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_BUFFER_UAV_FLAGS name=\"Flags\">%s</D3D12_BUFFER_UAV_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_BUFFER_UAV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dUavStructAsString(const D3D12_TEX1D_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_UAV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dArrayUavStructAsString(const D3D12_TEX1D_ARRAY_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_ARRAY_UAV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dUavStructAsString(const D3D12_TEX2D_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"PlaneSlice\">%u</UINT>", inStruct.PlaneSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_UAV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dArrayUavStructAsString(const D3D12_TEX2D_ARRAY_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure.appendFormattedString("<UINT name=\"PlaneSlice\">%u</UINT>", inStruct.PlaneSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_ARRAY_UAV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex3dUavStructAsString(const D3D12_TEX3D_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstWSlice\">%u</UINT>", inStruct.FirstWSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"WSize\">%u</UINT>", inStruct.WSize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX3D_UAV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_UNORDERED_ACCESS_VIEW_DESC' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteUnorderedAccessViewDescStructAsString(const D3D12_UNORDERED_ACCESS_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<D3D12_UAV_DIMENSION name=\"ViewDimension\">%s</D3D12_UAV_DIMENSION>", DX12CoreSerializers::WriteUavDimensionEnumAsString(inStruct.ViewDimension));
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_UNORDERED_ACCESS_VIEW_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteBufferRtvStructAsString(const D3D12_BUFFER_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"FirstElement\">%llu</UINT64>", inStruct.FirstElement);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumElements\">%u</UINT>", inStruct.NumElements);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_BUFFER_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dRtvStructAsString(const D3D12_TEX1D_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dArrayRtvStructAsString(const D3D12_TEX1D_ARRAY_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_ARRAY_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dRtvStructAsString(const D3D12_TEX2D_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"PlaneSlice\">%u</UINT>", inStruct.PlaneSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dmsRtvStructAsString(const D3D12_TEX2DMS_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"UnusedField_NothingToDefine\">%u</UINT>", inStruct.UnusedField_NothingToDefine);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2DMS_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dArrayRtvStructAsString(const D3D12_TEX2D_ARRAY_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure.appendFormattedString("<UINT name=\"PlaneSlice\">%u</UINT>", inStruct.PlaneSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_ARRAY_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dmsArrayRtvStructAsString(const D3D12_TEX2DMS_ARRAY_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2DMS_ARRAY_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex3dRtvStructAsString(const D3D12_TEX3D_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstWSlice\">%u</UINT>", inStruct.FirstWSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"WSize\">%u</UINT>", inStruct.WSize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX3D_RTV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_RENDER_TARGET_VIEW_DESC' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteRenderTargetViewDescStructAsString(const D3D12_RENDER_TARGET_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<D3D12_RTV_DIMENSION name=\"ViewDimension\">%s</D3D12_RTV_DIMENSION>", DX12CoreSerializers::WriteRtvDimensionEnumAsString(inStruct.ViewDimension));
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_RENDER_TARGET_VIEW_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dDsvStructAsString(const D3D12_TEX1D_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_DSV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex1dArrayDsvStructAsString(const D3D12_TEX1D_ARRAY_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX1D_ARRAY_DSV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dDsvStructAsString(const D3D12_TEX2D_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_DSV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dArrayDsvStructAsString(const D3D12_TEX2D_ARRAY_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"MipSlice\">%u</UINT>", inStruct.MipSlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2D_ARRAY_DSV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dmsDsvStructAsString(const D3D12_TEX2DMS_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"UnusedField_NothingToDefine\">%u</UINT>", inStruct.UnusedField_NothingToDefine);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2DMS_DSV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteTex2dmsArrayDsvStructAsString(const D3D12_TEX2DMS_ARRAY_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstArraySlice\">%u</UINT>", inStruct.FirstArraySlice);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ArraySize\">%u</UINT>", inStruct.ArraySize);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_TEX2DMS_ARRAY_DSV", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_DEPTH_STENCIL_VIEW_DESC' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteDepthStencilViewDescStructAsString(const D3D12_DEPTH_STENCIL_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure.appendFormattedString("<D3D12_DSV_DIMENSION name=\"ViewDimension\">%s</D3D12_DSV_DIMENSION>", DX12CoreSerializers::WriteDsvDimensionEnumAsString(inStruct.ViewDimension));
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeDsvFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_DSV_FLAGS name=\"Flags\">%s</D3D12_DSV_FLAGS>", FlagsString.asCharArray());
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DEPTH_STENCIL_VIEW_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDescriptorHeapDescStructAsString(const D3D12_DESCRIPTOR_HEAP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_DESCRIPTOR_HEAP_TYPE name=\"Type\">%s</D3D12_DESCRIPTOR_HEAP_TYPE>", DX12CoreSerializers::WriteDescriptorHeapTypeEnumAsString(inStruct.Type));
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumDescriptors\">%u</UINT>", inStruct.NumDescriptors);
    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeDescriptorHeapFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_DESCRIPTOR_HEAP_FLAGS name=\"Flags\">%s</D3D12_DESCRIPTOR_HEAP_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeMask\">%u</UINT>", inStruct.NodeMask);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DESCRIPTOR_HEAP_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDescriptorRangeStructAsString(const D3D12_DESCRIPTOR_RANGE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_DESCRIPTOR_RANGE_TYPE name=\"RangeType\">%s</D3D12_DESCRIPTOR_RANGE_TYPE>", DX12CoreSerializers::WriteDescriptorRangeTypeEnumAsString(inStruct.RangeType));
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumDescriptors\">%u</UINT>", inStruct.NumDescriptors);
    ioSerializedStructure.appendFormattedString("<UINT name=\"BaseShaderRegister\">%u</UINT>", inStruct.BaseShaderRegister);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RegisterSpace\">%u</UINT>", inStruct.RegisterSpace);
    ioSerializedStructure.appendFormattedString("<UINT name=\"OffsetInDescriptorsFromTableStart\">%u</UINT>", inStruct.OffsetInDescriptorsFromTableStart);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DESCRIPTOR_RANGE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRootDescriptorTableStructAsString(const D3D12_ROOT_DESCRIPTOR_TABLE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumDescriptorRanges\">%u</UINT>", inStruct.NumDescriptorRanges);

    for (UINT index = 0; index < inStruct.NumDescriptorRanges; index++)
    {
        gtASCIIString pDescriptorRangesString;
        DX12CoreSerializers::WriteDescriptorRangeStructAsString(inStruct.pDescriptorRanges[index], pDescriptorRangesString, "pDescriptorRanges");
        ioSerializedStructure.appendFormattedString("%s", pDescriptorRangesString.asCharArray());
    }

    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_ROOT_DESCRIPTOR_TABLE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRootConstantsStructAsString(const D3D12_ROOT_CONSTANTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"ShaderRegister\">%u</UINT>", inStruct.ShaderRegister);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RegisterSpace\">%u</UINT>", inStruct.RegisterSpace);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Num32BitValues\">%u</UINT>", inStruct.Num32BitValues);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_ROOT_CONSTANTS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRootDescriptorStructAsString(const D3D12_ROOT_DESCRIPTOR inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"ShaderRegister\">%u</UINT>", inStruct.ShaderRegister);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RegisterSpace\">%u</UINT>", inStruct.RegisterSpace);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_ROOT_DESCRIPTOR", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_ROOT_PARAMETER' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteRootParameterStructAsString(const D3D12_ROOT_PARAMETER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_ROOT_PARAMETER_TYPE name=\"ParameterType\">%s</D3D12_ROOT_PARAMETER_TYPE>", DX12CoreSerializers::WriteRootParameterTypeEnumAsString(inStruct.ParameterType));
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure.appendFormattedString("<D3D12_SHADER_VISIBILITY name=\"ShaderVisibility\">%s</D3D12_SHADER_VISIBILITY>", DX12CoreSerializers::WriteShaderVisibilityEnumAsString(inStruct.ShaderVisibility));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_ROOT_PARAMETER", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteStaticSamplerDescStructAsString(const D3D12_STATIC_SAMPLER_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_FILTER name=\"Filter\">%s</D3D12_FILTER>", DX12CoreSerializers::WriteFilterEnumAsString(inStruct.Filter));
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_ADDRESS_MODE name=\"AddressU\">%s</D3D12_TEXTURE_ADDRESS_MODE>", DX12CoreSerializers::WriteTextureAddressModeEnumAsString(inStruct.AddressU));
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_ADDRESS_MODE name=\"AddressV\">%s</D3D12_TEXTURE_ADDRESS_MODE>", DX12CoreSerializers::WriteTextureAddressModeEnumAsString(inStruct.AddressV));
    ioSerializedStructure.appendFormattedString("<D3D12_TEXTURE_ADDRESS_MODE name=\"AddressW\">%s</D3D12_TEXTURE_ADDRESS_MODE>", DX12CoreSerializers::WriteTextureAddressModeEnumAsString(inStruct.AddressW));
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MipLODBias\">%f</FLOAT>", inStruct.MipLODBias);
    ioSerializedStructure.appendFormattedString("<UINT name=\"MaxAnisotropy\">%u</UINT>", inStruct.MaxAnisotropy);
    ioSerializedStructure.appendFormattedString("<D3D12_COMPARISON_FUNC name=\"ComparisonFunc\">%s</D3D12_COMPARISON_FUNC>", DX12CoreSerializers::WriteComparisonFuncEnumAsString(inStruct.ComparisonFunc));
    ioSerializedStructure.appendFormattedString("<D3D12_STATIC_BORDER_COLOR name=\"BorderColor\">%s</D3D12_STATIC_BORDER_COLOR>", DX12CoreSerializers::WriteStaticBorderColorEnumAsString(inStruct.BorderColor));
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MinLOD\">%f</FLOAT>", inStruct.MinLOD);
    ioSerializedStructure.appendFormattedString("<FLOAT name=\"MaxLOD\">%f</FLOAT>", inStruct.MaxLOD);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ShaderRegister\">%u</UINT>", inStruct.ShaderRegister);
    ioSerializedStructure.appendFormattedString("<UINT name=\"RegisterSpace\">%u</UINT>", inStruct.RegisterSpace);
    ioSerializedStructure.appendFormattedString("<D3D12_SHADER_VISIBILITY name=\"ShaderVisibility\">%s</D3D12_SHADER_VISIBILITY>", DX12CoreSerializers::WriteShaderVisibilityEnumAsString(inStruct.ShaderVisibility));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_STATIC_SAMPLER_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteRootSignatureDescStructAsString(const D3D12_ROOT_SIGNATURE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumParameters\">%u</UINT>", inStruct.NumParameters);

    for (UINT index = 0; index < inStruct.NumParameters; index++)
    {
        gtASCIIString pParametersString;
        DX12CoreSerializers::WriteRootParameterStructAsString(inStruct.pParameters[index], pParametersString, "pParameters");
        ioSerializedStructure.appendFormattedString("%s", pParametersString.asCharArray());
    }

    ioSerializedStructure.appendFormattedString("<UINT name=\"NumStaticSamplers\">%u</UINT>", inStruct.NumStaticSamplers);

    for (UINT index = 0; index < inStruct.NumStaticSamplers; index++)
    {
        gtASCIIString pStaticSamplersString;
        DX12CoreSerializers::WriteStaticSamplerDescStructAsString(inStruct.pStaticSamplers[index], pStaticSamplersString, "pStaticSamplers");
        ioSerializedStructure.appendFormattedString("%s", pStaticSamplersString.asCharArray());
    }

    gtASCIIString FlagsString;
    DX12CoreSerializers::DecomposeRootSignatureFlagsEnumAsString(inStruct.Flags, FlagsString);
    ioSerializedStructure.appendFormattedString("<D3D12_ROOT_SIGNATURE_FLAGS name=\"Flags\">%s</D3D12_ROOT_SIGNATURE_FLAGS>", FlagsString.asCharArray());
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_ROOT_SIGNATURE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteCpuDescriptorHandleStructAsString(const D3D12_CPU_DESCRIPTOR_HANDLE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"ptr\">%Iu</SIZE_T>", inStruct.ptr);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_CPU_DESCRIPTOR_HANDLE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteGpuDescriptorHandleStructAsString(const D3D12_GPU_DESCRIPTOR_HANDLE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"ptr\">%llu</UINT64>", inStruct.ptr);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_GPU_DESCRIPTOR_HANDLE", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDiscardRegionStructAsString(const D3D12_DISCARD_REGION inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumRects\">%u</UINT>", inStruct.NumRects);
    ioSerializedStructure.appendFormattedString("<UINT name=\"FirstSubresource\">%u</UINT>", inStruct.FirstSubresource);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumSubresources\">%u</UINT>", inStruct.NumSubresources);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DISCARD_REGION", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteQueryHeapDescStructAsString(const D3D12_QUERY_HEAP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_QUERY_HEAP_TYPE name=\"Type\">%s</D3D12_QUERY_HEAP_TYPE>", DX12CoreSerializers::WriteQueryHeapTypeEnumAsString(inStruct.Type));
    ioSerializedStructure.appendFormattedString("<UINT name=\"Count\">%u</UINT>", inStruct.Count);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeMask\">%u</UINT>", inStruct.NodeMask);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_QUERY_HEAP_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteQueryDataPipelineStatisticsStructAsString(const D3D12_QUERY_DATA_PIPELINE_STATISTICS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"IAVertices\">%llu</UINT64>", inStruct.IAVertices);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"IAPrimitives\">%llu</UINT64>", inStruct.IAPrimitives);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"VSInvocations\">%llu</UINT64>", inStruct.VSInvocations);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"GSInvocations\">%llu</UINT64>", inStruct.GSInvocations);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"GSPrimitives\">%llu</UINT64>", inStruct.GSPrimitives);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"CInvocations\">%llu</UINT64>", inStruct.CInvocations);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"CPrimitives\">%llu</UINT64>", inStruct.CPrimitives);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"PSInvocations\">%llu</UINT64>", inStruct.PSInvocations);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"HSInvocations\">%llu</UINT64>", inStruct.HSInvocations);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"DSInvocations\">%llu</UINT64>", inStruct.DSInvocations);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"CSInvocations\">%llu</UINT64>", inStruct.CSInvocations);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_QUERY_DATA_PIPELINE_STATISTICS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteQueryDataSoStatisticsStructAsString(const D3D12_QUERY_DATA_SO_STATISTICS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"NumPrimitivesWritten\">%llu</UINT64>", inStruct.NumPrimitivesWritten);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"PrimitivesStorageNeeded\">%llu</UINT64>", inStruct.PrimitivesStorageNeeded);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_QUERY_DATA_SO_STATISTICS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteStreamOutputBufferViewStructAsString(const D3D12_STREAM_OUTPUT_BUFFER_VIEW inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_GPU_VIRTUAL_ADDRESS name=\"BufferLocation\">%llu</D3D12_GPU_VIRTUAL_ADDRESS>", inStruct.BufferLocation);
    ioSerializedStructure.appendFormattedString("<UINT64 name=\"SizeInBytes\">%llu</UINT64>", inStruct.SizeInBytes);
    ioSerializedStructure.appendFormattedString("<D3D12_GPU_VIRTUAL_ADDRESS name=\"BufferFilledSizeLocation\">%llu</D3D12_GPU_VIRTUAL_ADDRESS>", inStruct.BufferFilledSizeLocation);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_STREAM_OUTPUT_BUFFER_VIEW", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDrawArgumentsStructAsString(const D3D12_DRAW_ARGUMENTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"VertexCountPerInstance\">%u</UINT>", inStruct.VertexCountPerInstance);
    ioSerializedStructure.appendFormattedString("<UINT name=\"InstanceCount\">%u</UINT>", inStruct.InstanceCount);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StartVertexLocation\">%u</UINT>", inStruct.StartVertexLocation);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StartInstanceLocation\">%u</UINT>", inStruct.StartInstanceLocation);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DRAW_ARGUMENTS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDrawIndexedArgumentsStructAsString(const D3D12_DRAW_INDEXED_ARGUMENTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"IndexCountPerInstance\">%u</UINT>", inStruct.IndexCountPerInstance);
    ioSerializedStructure.appendFormattedString("<UINT name=\"InstanceCount\">%u</UINT>", inStruct.InstanceCount);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StartIndexLocation\">%u</UINT>", inStruct.StartIndexLocation);
    ioSerializedStructure.appendFormattedString("<INT name=\"BaseVertexLocation\">%d</INT>", inStruct.BaseVertexLocation);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StartInstanceLocation\">%u</UINT>", inStruct.StartInstanceLocation);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DRAW_INDEXED_ARGUMENTS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteDispatchArgumentsStructAsString(const D3D12_DISPATCH_ARGUMENTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"ThreadGroupCountX\">%u</UINT>", inStruct.ThreadGroupCountX);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ThreadGroupCountY\">%u</UINT>", inStruct.ThreadGroupCountY);
    ioSerializedStructure.appendFormattedString("<UINT name=\"ThreadGroupCountZ\">%u</UINT>", inStruct.ThreadGroupCountZ);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_DISPATCH_ARGUMENTS", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteVertexBufferViewStructAsString(const D3D12_VERTEX_BUFFER_VIEW inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_GPU_VIRTUAL_ADDRESS name=\"BufferLocation\">%llu</D3D12_GPU_VIRTUAL_ADDRESS>", inStruct.BufferLocation);
    ioSerializedStructure.appendFormattedString("<UINT name=\"SizeInBytes\">%u</UINT>", inStruct.SizeInBytes);
    ioSerializedStructure.appendFormattedString("<UINT name=\"StrideInBytes\">%u</UINT>", inStruct.StrideInBytes);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_VERTEX_BUFFER_VIEW", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteIndexBufferViewStructAsString(const D3D12_INDEX_BUFFER_VIEW inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_GPU_VIRTUAL_ADDRESS name=\"BufferLocation\">%llu</D3D12_GPU_VIRTUAL_ADDRESS>", inStruct.BufferLocation);
    ioSerializedStructure.appendFormattedString("<UINT name=\"SizeInBytes\">%u</UINT>", inStruct.SizeInBytes);
    ioSerializedStructure.appendFormattedString("<DXGI_FORMAT name=\"Format\">%s</DXGI_FORMAT>", DX12CustomSerializers::WriteDXGIFormat(inStruct.Format));
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_INDEX_BUFFER_VIEW", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}


//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
// @ERROR! Struct 'D3D12_INDIRECT_ARGUMENT_DESC' fails to serialize member '' correctly.
gtASCIIString& DX12CoreSerializers::WriteIndirectArgumentDescStructAsString(const D3D12_INDIRECT_ARGUMENT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<D3D12_INDIRECT_ARGUMENT_TYPE name=\"Type\">%s</D3D12_INDIRECT_ARGUMENT_TYPE>", DX12CoreSerializers::WriteIndirectArgumentTypeEnumAsString(inStruct.Type));
    (void)inStruct;    // @ERROR!!!
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_INDIRECT_ARGUMENT_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteCommandSignatureDescStructAsString(const D3D12_COMMAND_SIGNATURE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"ByteStride\">%u</UINT>", inStruct.ByteStride);
    ioSerializedStructure.appendFormattedString("<UINT name=\"NumArgumentDescs\">%u</UINT>", inStruct.NumArgumentDescs);

    for (UINT index = 0; index < inStruct.NumArgumentDescs; index++)
    {
        gtASCIIString pArgumentDescsString;
        DX12CoreSerializers::WriteIndirectArgumentDescStructAsString(inStruct.pArgumentDescs[index], pArgumentDescsString, "pArgumentDescs");
        ioSerializedStructure.appendFormattedString("%s", pArgumentDescsString.asCharArray());
    }

    ioSerializedStructure.appendFormattedString("<UINT name=\"NodeMask\">%u</UINT>", inStruct.NodeMask);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_COMMAND_SIGNATURE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteSubresourceDataStructAsString(const D3D12_SUBRESOURCE_DATA inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<void name=\"pData\">0x%p</void>", inStruct.pData);
    ioSerializedStructure.appendFormattedString("<LONG_PTR name=\"RowPitch\">0x%p</LONG_PTR>", inStruct.RowPitch);
    ioSerializedStructure.appendFormattedString("<LONG_PTR name=\"SlicePitch\">0x%p</LONG_PTR>", inStruct.SlicePitch);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_SUBRESOURCE_DATA", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CoreSerializers::WriteMemcpyDestStructAsString(const D3D12_MEMCPY_DEST inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<void name=\"pData\">0x%p</void>", inStruct.pData);
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"RowPitch\">%Iu</SIZE_T>", inStruct.RowPitch);
    ioSerializedStructure.appendFormattedString("<SIZE_T name=\"SlicePitch\">%Iu</SIZE_T>", inStruct.SlicePitch);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("D3D12_MEMCPY_DEST", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}



