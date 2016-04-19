//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   IDX12InstanceBase.cpp
/// \brief  An info class that holds information related to a wrapped DX12 interface.
//=============================================================================

#include "IDX12InstanceBase.h"
#include "D3D12Enumerations.h"

//-----------------------------------------------------------------------------
/// Constructor invoked when wrapping a new interface instance.
/// \param inRuntimeInstance The real interface pointer that the runtime gives us.
/// \param inWrapperInterface The Wrapper that we create to surround and hook the runtime interface.
/// \param inObjectType The type of object being wrapped.
/// \param inCreateInfo The CreateInfo instance for the newly-created interface.
//-----------------------------------------------------------------------------
IDX12InstanceBase::IDX12InstanceBase(IUnknown* inRuntimeInstance, IUnknown* inWrapperInterface, eObjectType inObjectType, Wrapped_DX12CreateInfoBase* inCreateInfo)
    : mRealInstance(inRuntimeInstance)
    , mWrapperInstance(inWrapperInterface)
    , mObjectType(inObjectType)
    , mCreateInfo(inCreateInfo)
    , mParentDevice(nullptr)
{
}

//-----------------------------------------------------------------------------
/// Destructor invoked when Wrapped interface instances are destroyed.
//-----------------------------------------------------------------------------
IDX12InstanceBase::~IDX12InstanceBase()
{
    // Destroy the cached CreateInfo structure for this instance.
    SAFE_DELETE(mCreateInfo);
}

//-----------------------------------------------------------------------------
/// Print the formatted application handle (the wrapper pointer) to the incoming string.
/// \param outHandleString A string containing the printed application handle.
//-----------------------------------------------------------------------------
void IDX12InstanceBase::PrintFormattedApplicationHandle(gtASCIIString& outHandleString) const
{
    outHandleString.appendFormattedString("0x%p", mWrapperInstance);
}

//-----------------------------------------------------------------------------
/// Write the create data for the wrapped object as XML.
/// \param outCreateInfoXML The CreateInfo formatted in XML.
//-----------------------------------------------------------------------------
void IDX12InstanceBase::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    PsAssert(mCreateInfo != nullptr);

    if (mCreateInfo != nullptr)
    {
        // Retrieve the CreateInfo XML for the instance, and then surround it with a single "CreateInfo" element.
        gtASCIIString instanceCreateInfo;
        mCreateInfo->AppendCreateInfoXML(instanceCreateInfo);

        outCreateInfoXML = XML("CreateInfo", instanceCreateInfo.asCharArray());
    }
}

//-----------------------------------------------------------------------------
/// Stringify the type of D3D12 interface being wrapped for display in the client.
/// \returns A string containing the interface type.
//-----------------------------------------------------------------------------
const char* IDX12InstanceBase::GetTypeAsString() const
{
    char* objectTypeString = nullptr;

    switch (mObjectType)
    {
    // *INDENT-OFF*
        case kObjectType_IUnknown:                          objectTypeString = "IUnknown";                          break;
        case kObjectType_ID3D12Object:                      objectTypeString = "ID3D12Object";                      break;
        case kObjectType_ID3D12DeviceChild:                 objectTypeString = "ID3D12DeviceChild";                 break;
        case kObjectType_ID3D12RootSignature:               objectTypeString = "ID3D12RootSignature";               break;
        case kObjectType_ID3D12RootSignatureDeserializer:   objectTypeString = "ID3D12RootSignatureDeserializer";   break;
        case kObjectType_ID3D12Pageable:                    objectTypeString = "ID3D12Pageable";                    break;
        case kObjectType_ID3D12Heap:                        objectTypeString = "ID3D12Heap";                        break;
        case kObjectType_ID3D12Resource:                    objectTypeString = "ID3D12Resource";                    break;
        case kObjectType_ID3D12CommandAllocator:            objectTypeString = "ID3D12CommandAllocator";            break;
        case kObjectType_ID3D12Fence:                       objectTypeString = "ID3D12Fence";                       break;
        case kObjectType_ID3D12PipelineState:               objectTypeString = "ID3D12PipelineState";               break;
        case kObjectType_ID3D12DescriptorHeap:              objectTypeString = "ID3D12DescriptorHeap";              break;
        case kObjectType_ID3D12QueryHeap:                   objectTypeString = "ID3D12QueryHeap";                   break;
        case kObjectType_ID3D12CommandSignature:            objectTypeString = "ID3D12CommandSignature";            break;
        case kObjectType_ID3D12CommandList:                 objectTypeString = "ID3D12CommandList";                 break;
        case kObjectType_ID3D12GraphicsCommandList:         objectTypeString = "ID3D12GraphicsCommandList";         break;
        case kObjectType_ID3D12CommandQueue:                objectTypeString = "ID3D12CommandQueue";                break;
        case kObjectType_ID3D12Device:                      objectTypeString = "ID3D12Device";                      break;

        // This should never happen. Make sure we catch it below.
        default:                                            objectTypeString = nullptr;
        // *INDENT-ON*
    }

    PsAssert(objectTypeString != nullptr);

    return objectTypeString;
}
