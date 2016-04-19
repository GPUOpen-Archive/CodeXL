//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12Serializers.h
/// \brief  Provides singleton access to DX12 symbol serialization functions.
//=============================================================================

#ifndef DX12SERIALIZERS_H
#define DX12SERIALIZERS_H

#include "../DX12Defines.h"
#include "../Common/TSingleton.h"
#include <AMDTBaseTools/Include/gtASCIIString.h>

//-----------------------------------------------------------------------------
/// A namespace that provides implementations for serializing types that weren't
/// included from parsed headers.
//-----------------------------------------------------------------------------
namespace DX12CustomSerializers
{
//-----------------------------------------------------------------------------
/// An enumeration printer for DXGI_FORMAT types.
/// \param inDxgiFormat A DXGI_FORMAT enum to stringify.
/// \returns A string containing a human-readable DXGI_FORMAT type.
//-----------------------------------------------------------------------------
const char* WriteDXGIFormat(DXGI_FORMAT inDxgiFormat);

//-----------------------------------------------------------------------------
/// An enumeration printer for D3D12_PRIMITIVE_TOPOLOGY types.
/// \param inPrimitiveTopology A D3D12_PRIMITIVE_TOPOLOGY enum to stringify.
/// \returns A string containing a human-readable D3D12_PRIMITIVE_TOPOLOGY type.
//-----------------------------------------------------------------------------
const char* WritePrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY inPrimitiveTopology);

//-----------------------------------------------------------------------------
/// An enumeration printer for D3D_FEATURE_LEVEL types.
/// \param inFeatureLevel A D3D_FEATURE_LEVEL enum to stringify.
/// \returns A string containing a human-readable D3D_FEATURE_LEVEL type.
//-----------------------------------------------------------------------------
const char* WriteD3DFeatureLevel(D3D_FEATURE_LEVEL inFeatureLevel);

//-----------------------------------------------------------------------------
/// An structure printer for DXGI_SAMPLE_DESC types.
/// \param inStruct A DXGI_SAMPLE_DESC structure to stringify.
/// \param ioSerializedStructure A string that the serialized structure will be printed into.
/// \param inOptionalNameAttribute An optional string containing a name to be applied to the output as an XML attribute.
/// \returns A string containing a human-readable DXGI_SAMPLE_DESC type.
//-----------------------------------------------------------------------------
gtASCIIString& WriteDXGISampleDesc(const DXGI_SAMPLE_DESC inStruct, gtASCIIString& ioSerializedStructure, const char* inOptionalNameAttribute = nullptr);
};

#endif // DX12SERIALIZERS_H
