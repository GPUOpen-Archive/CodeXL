//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12Serializers.cpp
/// \brief  Provides singleton access to DX12 symbol serialization functions.
//=============================================================================

#include "DX12Serializers.h"
#include "DX12Defines.h"
#include "../Util/DX12Utilities.h"
#include "../Common/xml.h"

//-----------------------------------------------------------------------------
/// An enumeration printer for DXGI_FORMAT types.
/// \param inDxgiFormat A DXGI_FORMAT enum to stringify.
/// \returns A string containing a human-readable DXGI_FORMAT type.
//-----------------------------------------------------------------------------
const char* DX12CustomSerializers::WriteDXGIFormat(DXGI_FORMAT inDxgiFormat)
{
    const char* resultString = nullptr;

    switch (inDxgiFormat)
    {
            PRINTENUMCASE(DXGI_FORMAT_UNKNOWN, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32A32_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32A32_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32A32_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32A32_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32B32_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16B16A16_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16B16A16_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16B16A16_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16B16A16_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16B16A16_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16B16A16_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G32_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32G8X24_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R10G10B10A2_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R10G10B10A2_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R10G10B10A2_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R11G11B10_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8B8A8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8B8A8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8B8A8_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8B8A8_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8B8A8_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16G16_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_D32_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R32_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R24G8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_D24_UNORM_S8_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_X24_TYPELESS_G8_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16_FLOAT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_D16_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R16_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8_UINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8_SINT, resultString);
            PRINTENUMCASE(DXGI_FORMAT_A8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R1_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R9G9B9E5_SHAREDEXP, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R8G8_B8G8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_G8R8_G8B8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC1_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC1_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC1_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC2_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC2_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC2_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC3_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC3_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC3_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC4_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC4_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC4_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC5_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC5_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC5_SNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B5G6R5_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B5G5R5A1_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B8G8R8A8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B8G8R8X8_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B8G8R8A8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B8G8R8X8_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B8G8R8X8_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC6H_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC6H_UF16, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC6H_SF16, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC7_TYPELESS, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC7_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_BC7_UNORM_SRGB, resultString);
            PRINTENUMCASE(DXGI_FORMAT_AYUV, resultString);
            PRINTENUMCASE(DXGI_FORMAT_Y410, resultString);
            PRINTENUMCASE(DXGI_FORMAT_Y416, resultString);
            PRINTENUMCASE(DXGI_FORMAT_NV12, resultString);
            PRINTENUMCASE(DXGI_FORMAT_P010, resultString);
            PRINTENUMCASE(DXGI_FORMAT_P016, resultString);
            PRINTENUMCASE(DXGI_FORMAT_420_OPAQUE, resultString);
            PRINTENUMCASE(DXGI_FORMAT_YUY2, resultString);
            PRINTENUMCASE(DXGI_FORMAT_Y210, resultString);
            PRINTENUMCASE(DXGI_FORMAT_Y216, resultString);
            PRINTENUMCASE(DXGI_FORMAT_NV11, resultString);
            PRINTENUMCASE(DXGI_FORMAT_AI44, resultString);
            PRINTENUMCASE(DXGI_FORMAT_IA44, resultString);
            PRINTENUMCASE(DXGI_FORMAT_P8, resultString);
            PRINTENUMCASE(DXGI_FORMAT_A8P8, resultString);
            PRINTENUMCASE(DXGI_FORMAT_B4G4R4A4_UNORM, resultString);
            PRINTENUMCASE(DXGI_FORMAT_P208, resultString);
            PRINTENUMCASE(DXGI_FORMAT_V208, resultString);
            PRINTENUMCASE(DXGI_FORMAT_V408, resultString);
            PRINTENUMCASE(DXGI_FORMAT_FORCE_UINT, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// An enumeration printer for D3D12_PRIMITIVE_TOPOLOGY types.
/// \param inPrimitiveTopology A D3D12_PRIMITIVE_TOPOLOGY enum to stringify.
/// \returns A string containing a human-readable D3D12_PRIMITIVE_TOPOLOGY type.
//-----------------------------------------------------------------------------
const char* DX12CustomSerializers::WritePrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY inPrimitiveTopology)
{
    const char* resultString = nullptr;

    switch (inPrimitiveTopology)
    {
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_POINTLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_LINELIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST, resultString);
            PRINTENUMCASE(D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// An enumeration printer for D3D_FEATURE_LEVEL types.
/// \param inFeatureLevel A D3D_FEATURE_LEVEL enum to stringify.
/// \returns A string containing a human-readable D3D_FEATURE_LEVEL type.
//-----------------------------------------------------------------------------
const char* DX12CustomSerializers::WriteD3DFeatureLevel(D3D_FEATURE_LEVEL inFeatureLevel)
{
    const char* resultString = nullptr;

    switch (inFeatureLevel)
    {
            PRINTENUMCASE(D3D_FEATURE_LEVEL_9_1, resultString);
            PRINTENUMCASE(D3D_FEATURE_LEVEL_9_2, resultString);
            PRINTENUMCASE(D3D_FEATURE_LEVEL_9_3, resultString);
            PRINTENUMCASE(D3D_FEATURE_LEVEL_10_0, resultString);
            PRINTENUMCASE(D3D_FEATURE_LEVEL_10_1, resultString);
            PRINTENUMCASE(D3D_FEATURE_LEVEL_11_0, resultString);
            PRINTENUMCASE(D3D_FEATURE_LEVEL_11_1, resultString);
    }

    return resultString;
}

//-----------------------------------------------------------------------------
/// An structure printer for DXGI_SAMPLE_DESC types.
/// \param inStruct A DXGI_SAMPLE_DESC structure to stringify.
/// \param ioSerializedStructure A string that the serialized structure will be printed into.
/// \param inOptionalNameAttribute An optional string containing a name to be applied to the output as an XML attribute.
/// \returns A string containing a human-readable DXGI_SAMPLE_DESC type.
//-----------------------------------------------------------------------------
gtASCIIString& DX12CustomSerializers::WriteDXGISampleDesc(const DXGI_SAMPLE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute)
{
    ioSerializedStructure.appendFormattedString("<UINT name=\"Count\">%u</UINT>", inStruct.Count);
    ioSerializedStructure.appendFormattedString("<UINT name=\"Quality\">%u</UINT>", inStruct.Quality);
    ioSerializedStructure = DX12Util::SurroundWithNamedElement("DXGI_SAMPLE_DESC", ioSerializedStructure, inOptionalNameAttribute);
    return ioSerializedStructure;
}