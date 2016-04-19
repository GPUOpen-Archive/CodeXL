//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12ObjectDatabaseProcessor.cpp
/// \brief  The DX12ObjectDatabaseProcessor is responsible for extending the
///         ObjectDatabaseProcessor to work with DX12.
//=============================================================================

#include "DX12ObjectDatabaseProcessor.h"
#include "../DX12LayerManager.h"
#include "Interception/DX12Interceptor.h"

//-----------------------------------------------------------------------------
/// Default constructor for the DX12ObjectDatabaseProcessor class.
//-----------------------------------------------------------------------------
DX12ObjectDatabaseProcessor::DX12ObjectDatabaseProcessor()
    : ObjectDatabaseProcessor()
{
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* DX12ObjectDatabaseProcessor::GetParentLayerManager()
{
    return DX12LayerManager::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve the object type enumeration value from a type string.
/// \param inObjectTypeString A string containing the type of object to get the value for.
/// \returns The enumeration value for the incoming object type string.
//-----------------------------------------------------------------------------
int DX12ObjectDatabaseProcessor::GetObjectTypeFromString(const gtASCIIString& inObjectTypeString) const
{
    int typeValue = kObjectType_Undefined;

    static ObjectTypeNameToValueMap typeStringToValueMap;

    if (typeStringToValueMap.empty())
    {
        typeStringToValueMap["ID3D12RootSignature"]             = kObjectType_ID3D12RootSignature;
        typeStringToValueMap["ID3D12RootSignatureDeserializer"] = kObjectType_ID3D12RootSignatureDeserializer;
        typeStringToValueMap["ID3D12Pageable"]                  = kObjectType_ID3D12Pageable;
        typeStringToValueMap["ID3D12Heap"]                      = kObjectType_ID3D12Heap;
        typeStringToValueMap["ID3D12Resource"]                  = kObjectType_ID3D12Resource;
        typeStringToValueMap["ID3D12CommandAllocator"]          = kObjectType_ID3D12CommandAllocator;
        typeStringToValueMap["ID3D12Fence"]                     = kObjectType_ID3D12Fence;
        typeStringToValueMap["ID3D12PipelineState"]             = kObjectType_ID3D12PipelineState;
        typeStringToValueMap["ID3D12DescriptorHeap"]            = kObjectType_ID3D12DescriptorHeap;
        typeStringToValueMap["ID3D12QueryHeap"]                 = kObjectType_ID3D12QueryHeap;
        typeStringToValueMap["ID3D12CommandSignature"]          = kObjectType_ID3D12CommandSignature;
        typeStringToValueMap["ID3D12CommandList"]               = kObjectType_ID3D12CommandList;
        typeStringToValueMap["ID3D12GraphicsCommandList"]       = kObjectType_ID3D12GraphicsCommandList;
        typeStringToValueMap["ID3D12CommandQueue"]              = kObjectType_ID3D12CommandQueue;
        typeStringToValueMap["ID3D12Device"]                    = kObjectType_ID3D12Device;
    }

    ObjectTypeNameToValueMap::const_iterator typeIter = typeStringToValueMap.find(inObjectTypeString);

    if (typeIter != typeStringToValueMap.end())
    {
        typeValue = typeIter->second;
    }

    return typeValue;
}
