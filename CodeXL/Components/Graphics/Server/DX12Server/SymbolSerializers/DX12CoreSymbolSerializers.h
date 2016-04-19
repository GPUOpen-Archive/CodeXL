//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12CoreSymbolSerializers.h
/// \brief  Serialize D3D12 types to strings, and build nested XML.
//=============================================================================

#ifndef DX12CORESYMBOLSERIALIZERS_H
#define DX12CORESYMBOLSERIALIZERS_H

#include "DX12Defines.h"
#include <AMDTBaseTools/Include/gtASCIIString.h>

namespace DX12CoreSerializers
{
//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCommandListTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteCommandListTypeEnumAsString(DWORD inWriteCommandListTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCommandQueueFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteCommandQueueFlagsEnumAsString(DWORD inWriteCommandQueueFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeCommandQueueFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCommandQueuePriorityEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteCommandQueuePriorityEnumAsString(DWORD inWriteCommandQueuePriorityEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWritePrimitiveTopologyTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WritePrimitiveTopologyTypeEnumAsString(DWORD inWritePrimitiveTopologyTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteInputClassificationEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteInputClassificationEnumAsString(DWORD inWriteInputClassificationEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFillModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFillModeEnumAsString(DWORD inWriteFillModeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCullModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteCullModeEnumAsString(DWORD inWriteCullModeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteComparisonFuncEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteComparisonFuncEnumAsString(DWORD inWriteComparisonFuncEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDepthWriteMaskEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteDepthWriteMaskEnumAsString(DWORD inWriteDepthWriteMaskEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteStencilOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteStencilOpEnumAsString(DWORD inWriteStencilOpEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBlendEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteBlendEnumAsString(DWORD inWriteBlendEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBlendOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteBlendOpEnumAsString(DWORD inWriteBlendOpEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteColorWriteEnableEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteColorWriteEnableEnumAsString(DWORD inWriteColorWriteEnableEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteLogicOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteLogicOpEnumAsString(DWORD inWriteLogicOpEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteConservativeRasterizationModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteConservativeRasterizationModeEnumAsString(DWORD inWriteConservativeRasterizationModeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteIndexBufferStripCutValueEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteIndexBufferStripCutValueEnumAsString(DWORD inWriteIndexBufferStripCutValueEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWritePipelineStateFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WritePipelineStateFlagsEnumAsString(DWORD inWritePipelineStateFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposePipelineStateFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFeatureEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFeatureEnumAsString(DWORD inWriteFeatureEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteShaderMinPrecisionSupportEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteShaderMinPrecisionSupportEnumAsString(DWORD inWriteShaderMinPrecisionSupportEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTiledResourcesTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTiledResourcesTierEnumAsString(DWORD inWriteTiledResourcesTierEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceBindingTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceBindingTierEnumAsString(DWORD inWriteResourceBindingTierEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteConservativeRasterizationTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteConservativeRasterizationTierEnumAsString(DWORD inWriteConservativeRasterizationTierEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFormatSupport1EnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFormatSupport1EnumAsString(DWORD inWriteFormatSupport1EnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFormatSupport2EnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFormatSupport2EnumAsString(DWORD inWriteFormatSupport2EnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteMultisampleQualityLevelFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteMultisampleQualityLevelFlagsEnumAsString(DWORD inWriteMultisampleQualityLevelFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeMultisampleQualityLevelFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCrossNodeSharingTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteCrossNodeSharingTierEnumAsString(DWORD inWriteCrossNodeSharingTierEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceHeapTierEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceHeapTierEnumAsString(DWORD inWriteResourceHeapTierEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteHeapTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteHeapTypeEnumAsString(DWORD inWriteHeapTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteCpuPagePropertyEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteCpuPagePropertyEnumAsString(DWORD inWriteCpuPagePropertyEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteMemoryPoolEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteMemoryPoolEnumAsString(DWORD inWriteMemoryPoolEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteHeapFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteHeapFlagsEnumAsString(DWORD inWriteHeapFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeHeapFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);


//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceDimensionEnumAsString(DWORD inWriteResourceDimensionEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTextureLayoutEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTextureLayoutEnumAsString(DWORD inWriteTextureLayoutEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceFlagsEnumAsString(DWORD inWriteResourceFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeResourceFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTileRangeFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTileRangeFlagsEnumAsString(DWORD inWriteTileRangeFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeTileRangeFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTileMappingFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTileMappingFlagsEnumAsString(DWORD inWriteTileMappingFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeTileMappingFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTileCopyFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTileCopyFlagsEnumAsString(DWORD inWriteTileCopyFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeTileCopyFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceStatesEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceStatesEnumAsString(DWORD inWriteResourceStatesEnumAsString);
//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceBarrierTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceBarrierTypeEnumAsString(DWORD inWriteResourceBarrierTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteResourceBarrierFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteResourceBarrierFlagsEnumAsString(DWORD inWriteResourceBarrierFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeResourceBarrierFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTextureCopyTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTextureCopyTypeEnumAsString(DWORD inWriteTextureCopyTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteShaderComponentMappingEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteShaderComponentMappingEnumAsString(DWORD inWriteShaderComponentMappingEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBufferSrvFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteBufferSrvFlagsEnumAsString(DWORD inWriteBufferSrvFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeBufferSrvFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteSrvDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteSrvDimensionEnumAsString(DWORD inWriteSrvDimensionEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFilterEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFilterEnumAsString(DWORD inWriteFilterEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFilterTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFilterTypeEnumAsString(DWORD inWriteFilterTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFilterReductionTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFilterReductionTypeEnumAsString(DWORD inWriteFilterReductionTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteTextureAddressModeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteTextureAddressModeEnumAsString(DWORD inWriteTextureAddressModeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteBufferUavFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteBufferUavFlagsEnumAsString(DWORD inWriteBufferUavFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeBufferUavFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteUavDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteUavDimensionEnumAsString(DWORD inWriteUavDimensionEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteRtvDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteRtvDimensionEnumAsString(DWORD inWriteRtvDimensionEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDsvFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteDsvFlagsEnumAsString(DWORD inWriteDsvFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeDsvFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDsvDimensionEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteDsvDimensionEnumAsString(DWORD inWriteDsvDimensionEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteClearFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteClearFlagsEnumAsString(DWORD inWriteClearFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeClearFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteFenceFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteFenceFlagsEnumAsString(DWORD inWriteFenceFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeFenceFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDescriptorHeapTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteDescriptorHeapTypeEnumAsString(DWORD inWriteDescriptorHeapTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDescriptorHeapFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteDescriptorHeapFlagsEnumAsString(DWORD inWriteDescriptorHeapFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeDescriptorHeapFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteDescriptorRangeTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteDescriptorRangeTypeEnumAsString(DWORD inWriteDescriptorRangeTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteShaderVisibilityEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteShaderVisibilityEnumAsString(DWORD inWriteShaderVisibilityEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteRootParameterTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteRootParameterTypeEnumAsString(DWORD inWriteRootParameterTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteRootSignatureFlagsEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteRootSignatureFlagsEnumAsString(DWORD inWriteRootSignatureFlagsEnumAsString);

//-----------------------------------------------------------------------------
/// Decompose a packed bit flags variable into a string containing the enabled enum members separated by pipes.
/// \param inFlags The packed bit flags variable to decompose into a string.
/// \param ioFlagsString The string where the output will be dumped.
//-----------------------------------------------------------------------------
void DecomposeRootSignatureFlagsEnumAsString(DWORD inFlags, gtASCIIString& ioFlagsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteStaticBorderColorEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteStaticBorderColorEnumAsString(DWORD inWriteStaticBorderColorEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteQueryHeapTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteQueryHeapTypeEnumAsString(DWORD inWriteQueryHeapTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteQueryTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteQueryTypeEnumAsString(DWORD inWriteQueryTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWritePredicationOpEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WritePredicationOpEnumAsString(DWORD inWritePredicationOpEnumAsString);

//-----------------------------------------------------------------------------
/// Write the incoming enumeration as a string.
/// \param inWriteIndirectArgumentTypeEnumAsString The incoming enumeration.
/// \returns A stringified version of the incoming enumeration.
//-----------------------------------------------------------------------------
const char* WriteIndirectArgumentTypeEnumAsString(DWORD inWriteIndirectArgumentTypeEnumAsString);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteCommandQueueDescStructAsString(const D3D12_COMMAND_QUEUE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteInputElementDescStructAsString(const D3D12_INPUT_ELEMENT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteSoDeclarationEntryStructAsString(const D3D12_SO_DECLARATION_ENTRY inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteViewportStructAsString(const D3D12_VIEWPORT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteBoxStructAsString(const D3D12_BOX inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDepthStencilopDescStructAsString(const D3D12_DEPTH_STENCILOP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDepthStencilDescStructAsString(const D3D12_DEPTH_STENCIL_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRenderTargetBlendDescStructAsString(const D3D12_RENDER_TARGET_BLEND_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteBlendDescStructAsString(const D3D12_BLEND_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRasterizerDescStructAsString(const D3D12_RASTERIZER_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteShaderBytecodeStructAsString(const D3D12_SHADER_BYTECODE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteStreamOutputDescStructAsString(const D3D12_STREAM_OUTPUT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteInputLayoutDescStructAsString(const D3D12_INPUT_LAYOUT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteCachedPipelineStateStructAsString(const D3D12_CACHED_PIPELINE_STATE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteGraphicsPipelineStateDescStructAsString(const D3D12_GRAPHICS_PIPELINE_STATE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteComputePipelineStateDescStructAsString(const D3D12_COMPUTE_PIPELINE_STATE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureData_optionsStructAsString(const D3D12_FEATURE_DATA_D3D12_OPTIONS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureDataArchitectureStructAsString(const D3D12_FEATURE_DATA_ARCHITECTURE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureDataFeatureLevelsStructAsString(const D3D12_FEATURE_DATA_FEATURE_LEVELS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureDataFormatSupportStructAsString(const D3D12_FEATURE_DATA_FORMAT_SUPPORT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureDataMultisampleQualityLevelsStructAsString(const D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureDataFormatInfoStructAsString(const D3D12_FEATURE_DATA_FORMAT_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteFeatureDataGpuVirtualAddressSupportStructAsString(const D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteResourceAllocationInfoStructAsString(const D3D12_RESOURCE_ALLOCATION_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteHeapPropertiesStructAsString(const D3D12_HEAP_PROPERTIES inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteHeapDescStructAsString(const D3D12_HEAP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteResourceDescStructAsString(const D3D12_RESOURCE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDepthStencilValueStructAsString(const D3D12_DEPTH_STENCIL_VALUE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteClearValueStructAsString(const D3D12_CLEAR_VALUE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRangeStructAsString(const D3D12_RANGE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteSubresourceInfoStructAsString(const D3D12_SUBRESOURCE_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTiledResourceCoordinateStructAsString(const D3D12_TILED_RESOURCE_COORDINATE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTileRegionSizeStructAsString(const D3D12_TILE_REGION_SIZE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteSubresourceTilingStructAsString(const D3D12_SUBRESOURCE_TILING inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTileShapeStructAsString(const D3D12_TILE_SHAPE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WritePackedMipInfoStructAsString(const D3D12_PACKED_MIP_INFO inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteResourceTransitionBarrierStructAsString(const D3D12_RESOURCE_TRANSITION_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteResourceAliasingBarrierStructAsString(const D3D12_RESOURCE_ALIASING_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteResourceUavBarrierStructAsString(const D3D12_RESOURCE_UAV_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteResourceBarrierStructAsString(const D3D12_RESOURCE_BARRIER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteSubresourceFootprintStructAsString(const D3D12_SUBRESOURCE_FOOTPRINT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WritePlacedSubresourceFootprintStructAsString(const D3D12_PLACED_SUBRESOURCE_FOOTPRINT inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTextureCopyLocationStructAsString(const D3D12_TEXTURE_COPY_LOCATION inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteBufferSrvStructAsString(const D3D12_BUFFER_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dSrvStructAsString(const D3D12_TEX1D_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dArraySrvStructAsString(const D3D12_TEX1D_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dSrvStructAsString(const D3D12_TEX2D_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dArraySrvStructAsString(const D3D12_TEX2D_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex3dSrvStructAsString(const D3D12_TEX3D_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTexcubeSrvStructAsString(const D3D12_TEXCUBE_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTexcubeArraySrvStructAsString(const D3D12_TEXCUBE_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dmsSrvStructAsString(const D3D12_TEX2DMS_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dmsArraySrvStructAsString(const D3D12_TEX2DMS_ARRAY_SRV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteShaderResourceViewDescStructAsString(const D3D12_SHADER_RESOURCE_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteConstantBufferViewDescStructAsString(const D3D12_CONSTANT_BUFFER_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteSamplerDescStructAsString(const D3D12_SAMPLER_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteBufferUavStructAsString(const D3D12_BUFFER_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dUavStructAsString(const D3D12_TEX1D_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dArrayUavStructAsString(const D3D12_TEX1D_ARRAY_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dUavStructAsString(const D3D12_TEX2D_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dArrayUavStructAsString(const D3D12_TEX2D_ARRAY_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex3dUavStructAsString(const D3D12_TEX3D_UAV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteUnorderedAccessViewDescStructAsString(const D3D12_UNORDERED_ACCESS_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteBufferRtvStructAsString(const D3D12_BUFFER_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dRtvStructAsString(const D3D12_TEX1D_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dArrayRtvStructAsString(const D3D12_TEX1D_ARRAY_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dRtvStructAsString(const D3D12_TEX2D_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dmsRtvStructAsString(const D3D12_TEX2DMS_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dArrayRtvStructAsString(const D3D12_TEX2D_ARRAY_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dmsArrayRtvStructAsString(const D3D12_TEX2DMS_ARRAY_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex3dRtvStructAsString(const D3D12_TEX3D_RTV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRenderTargetViewDescStructAsString(const D3D12_RENDER_TARGET_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dDsvStructAsString(const D3D12_TEX1D_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex1dArrayDsvStructAsString(const D3D12_TEX1D_ARRAY_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dDsvStructAsString(const D3D12_TEX2D_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dArrayDsvStructAsString(const D3D12_TEX2D_ARRAY_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dmsDsvStructAsString(const D3D12_TEX2DMS_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteTex2dmsArrayDsvStructAsString(const D3D12_TEX2DMS_ARRAY_DSV inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDepthStencilViewDescStructAsString(const D3D12_DEPTH_STENCIL_VIEW_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDescriptorHeapDescStructAsString(const D3D12_DESCRIPTOR_HEAP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDescriptorRangeStructAsString(const D3D12_DESCRIPTOR_RANGE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRootDescriptorTableStructAsString(const D3D12_ROOT_DESCRIPTOR_TABLE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRootConstantsStructAsString(const D3D12_ROOT_CONSTANTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRootDescriptorStructAsString(const D3D12_ROOT_DESCRIPTOR inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRootParameterStructAsString(const D3D12_ROOT_PARAMETER inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteStaticSamplerDescStructAsString(const D3D12_STATIC_SAMPLER_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteRootSignatureDescStructAsString(const D3D12_ROOT_SIGNATURE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteCpuDescriptorHandleStructAsString(const D3D12_CPU_DESCRIPTOR_HANDLE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteGpuDescriptorHandleStructAsString(const D3D12_GPU_DESCRIPTOR_HANDLE inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDiscardRegionStructAsString(const D3D12_DISCARD_REGION inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteQueryHeapDescStructAsString(const D3D12_QUERY_HEAP_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteQueryDataPipelineStatisticsStructAsString(const D3D12_QUERY_DATA_PIPELINE_STATISTICS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteQueryDataSoStatisticsStructAsString(const D3D12_QUERY_DATA_SO_STATISTICS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteStreamOutputBufferViewStructAsString(const D3D12_STREAM_OUTPUT_BUFFER_VIEW inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDrawArgumentsStructAsString(const D3D12_DRAW_ARGUMENTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDrawIndexedArgumentsStructAsString(const D3D12_DRAW_INDEXED_ARGUMENTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDispatchArgumentsStructAsString(const D3D12_DISPATCH_ARGUMENTS inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteVertexBufferViewStructAsString(const D3D12_VERTEX_BUFFER_VIEW inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteIndexBufferViewStructAsString(const D3D12_INDEX_BUFFER_VIEW inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteIndirectArgumentDescStructAsString(const D3D12_INDIRECT_ARGUMENT_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteCommandSignatureDescStructAsString(const D3D12_COMMAND_SIGNATURE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteSubresourceDataStructAsString(const D3D12_SUBRESOURCE_DATA inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);

//-----------------------------------------------------------------------------
/// Serialize the incoming structure to a string of XML.
/// \param inStruct The structure to serialize to XML.
/// \param ioSerializedStructure The string to serialize the XML to.
/// \param inOptionalNameAttribute An optional string that will include a "Name=" attribute on the output XML element.
/// \returns The serialized XML string.
//-----------------------------------------------------------------------------
gtASCIIString& WriteMemcpyDestStructAsString(const D3D12_MEMCPY_DEST inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);
};


#endif // DX12CORESYMBOLSERIALIZERS_H

