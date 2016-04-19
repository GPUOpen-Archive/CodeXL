//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Declarations of structures designed to hold all the parameters used to create interfaces.
//=============================================================================

#include "DX12CreateInfoStructs.h"
#include "../SymbolSerializers/DX12Serializers.h"
#include "../SymbolSerializers/DX12CoreSymbolSerializers.h"
#include "../Util/DX12Utilities.h"
#include "../Common/xml.h"
#include "../Util/DX12CoreDeepCopy.h"

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12RootSignatureCreateInfo.
/// \param inNodeMask The GPU node mask.
/// \param inBlobWithRootSignature The serialized Root Signature blob.
/// \param inBlobLengthInBytes The total blob length in bytes.
//-----------------------------------------------------------------------------
Wrapped_ID3D12RootSignatureCreateInfo::Wrapped_ID3D12RootSignatureCreateInfo(UINT inNodeMask, const void* inBlobWithRootSignature, SIZE_T inBlobLengthInBytes)
    : mNodeMask(inNodeMask)
    , mBlobLengthInBytes(inBlobLengthInBytes)
{
    mBlobWithRootSignature = new char[inBlobLengthInBytes];
    memcpy_s(mBlobWithRootSignature, inBlobLengthInBytes, inBlobWithRootSignature, sizeof(char) * inBlobLengthInBytes);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12RootSignatureCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12RootSignatureCreateInfo::~Wrapped_ID3D12RootSignatureCreateInfo()
{
    SAFE_DELETE_ARRAY(mBlobWithRootSignature);
}

//-----------------------------------------------------------------------------
/// Append the Root Signature Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12RootSignatureCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo.appendFormattedString("<UINT name=\"nodeMask\">%u</UINT>", mNodeMask);
    thisCreateInfo.appendFormattedString("<VOID name=\"pBlobWithRootSignature\">@BUFFERDATA</VOID>");
    thisCreateInfo.appendFormattedString("<UINT name=\"blobLengthInBytes\">%u</UINT>", mBlobLengthInBytes);

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12RootSignature", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12RootSignatureDeserializerCreateInfo.
/// \param pSrcData The incoming deserialized Root Signature source data.
/// \param SrcDataSizeInBytes The total source data size in bytes.
//-----------------------------------------------------------------------------
Wrapped_ID3D12RootSignatureDeserializerCreateInfo::Wrapped_ID3D12RootSignatureDeserializerCreateInfo(LPCVOID pSrcData, SIZE_T SrcDataSizeInBytes)
    : mSrcDataSizeInBytes(SrcDataSizeInBytes)
{
    mSrcData = new char[SrcDataSizeInBytes];
    memcpy_s(mSrcData, SrcDataSizeInBytes, pSrcData, sizeof(char) * SrcDataSizeInBytes);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12RootSignatureDeserializerCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12RootSignatureDeserializerCreateInfo::~Wrapped_ID3D12RootSignatureDeserializerCreateInfo()
{
    SAFE_DELETE_ARRAY(mSrcData);
}

//-----------------------------------------------------------------------------
/// Append the Root Signature Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12RootSignatureDeserializerCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;

    thisCreateInfo.appendFormattedString("<VOID name=\"pSrcData\">@BUFFERDATA</VOID>");
    thisCreateInfo.appendFormattedString("<SIZE_T name=\"SrcDataSizeInBytes\">%llu</SIZE_T>", mSrcDataSizeInBytes);

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12RootSignatureDeserializer", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12HeapCreateInfo.
/// \param inDescription The description structure used to create the instance.
//-----------------------------------------------------------------------------
Wrapped_ID3D12HeapCreateInfo::Wrapped_ID3D12HeapCreateInfo(const D3D12_HEAP_DESC* inDescription)
{
    mDescription = new D3D12_HEAP_DESC;
    DX12CoreDeepCopy::DeepCopy(inDescription, mDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12HeapCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12HeapCreateInfo::~Wrapped_ID3D12HeapCreateInfo()
{
    SAFE_DELETE(mDescription);
}

//-----------------------------------------------------------------------------
/// Append the Heap Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12HeapCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo = DX12CoreSerializers::WriteHeapDescStructAsString(*mDescription, thisCreateInfo, "pDesc");

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12Heap", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12ResourceCreateInfo.
/// \param inResourceDescription The description of the resource being created.
/// \param inInitialState The initial state of the resource being created.
/// \param inOptimizedClearValue The initial clear value for the resource being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ResourceCreateInfo::Wrapped_ID3D12ResourceCreateInfo(const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue)
    : mInitialState(inInitialState)
{
    mResourceDescription = new D3D12_RESOURCE_DESC;
    DX12CoreDeepCopy::DeepCopy(inResourceDescription, mResourceDescription);

    D3D12_CLEAR_VALUE* clearValueCopy = nullptr;

    if (inOptimizedClearValue != nullptr)
    {
        clearValueCopy = new D3D12_CLEAR_VALUE;
        DX12CoreDeepCopy::DeepCopy(inOptimizedClearValue, clearValueCopy);
    }

    mOptimizedClearValue = clearValueCopy;
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12ResourceCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ResourceCreateInfo::~Wrapped_ID3D12ResourceCreateInfo()
{
    SAFE_DELETE(mResourceDescription);
    SAFE_DELETE(mOptimizedClearValue);
}

//-----------------------------------------------------------------------------
/// Append the Resource Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12ResourceCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString createInfo;
    DX12CoreSerializers::WriteResourceDescStructAsString(*mResourceDescription, createInfo, "pDesc");
    createInfo.appendFormattedString("<D3D12_RESOURCE_STATES name=\"InitialResourceState\">%s</D3D12_RESOURCE_STATES>", DX12CoreSerializers::WriteResourceStatesEnumAsString(mInitialState));

    if (mOptimizedClearValue != nullptr)
    {
        gtASCIIString clearValue;
        DX12CoreSerializers::WriteClearValueStructAsString(*mOptimizedClearValue, clearValue, "pOptimizedClearValue");
        createInfo.appendFormattedString("%s", clearValue.asCharArray());
    }

    outCreateInfoXML.append(createInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12ComittedResourceCreateInfo.
/// \param pHeapProperties The properties for the heap that this resource will use.
/// \param inHeapFlags The flags for the heap that will be created.
/// \param inResourceDescription The description for the resource to be created.
/// \param inInitialState The initial state for the resource being created.
/// \param inOptimizedClearValue The clear value for the resource being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ComittedResourceCreateInfo::Wrapped_ID3D12ComittedResourceCreateInfo(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS inHeapFlags, const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue)
    : Wrapped_ID3D12ResourceCreateInfo(inResourceDescription, inInitialState, inOptimizedClearValue)
    , mHeapFlags(inHeapFlags)
{
    mHeapProperties = new D3D12_HEAP_PROPERTIES;
    DX12CoreDeepCopy::DeepCopy(pHeapProperties, mHeapProperties);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12ComittedResourceCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ComittedResourceCreateInfo::~Wrapped_ID3D12ComittedResourceCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Comitted Resource Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12ComittedResourceCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    DX12CoreSerializers::WriteHeapPropertiesStructAsString(*mHeapProperties, thisCreateInfo, "pHeapProperties");
    thisCreateInfo.appendFormattedString("<D3D12_HEAP_FLAGS name=\"HeapFlags\">%s</D3D12_HEAP_FLAGS>", DX12CoreSerializers::WriteHeapFlagsEnumAsString(mHeapFlags));
    Wrapped_ID3D12ResourceCreateInfo::AppendCreateInfoXML(thisCreateInfo);

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12ComittedResource", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12PlacedResourceCreateInfo.
/// \param inHeap The heap used for the new resource.
/// \param inHeapOffset The heap offset for this resource.
/// \param inResourceDescription The description for the resource being created.
/// \param inInitialState The initial state for the resource being created.
/// \param inOptimizedClearValue The clear value for the resource being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12PlacedResourceCreateInfo::Wrapped_ID3D12PlacedResourceCreateInfo(Wrapped_ID3D12Heap* inHeap, UINT64 inHeapOffset, const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue)
    : Wrapped_ID3D12ResourceCreateInfo(inResourceDescription, inInitialState, inOptimizedClearValue)
    , mHeap(inHeap)
    , mHeapOffset(inHeapOffset)
{
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12PlacedResourceCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12PlacedResourceCreateInfo::~Wrapped_ID3D12PlacedResourceCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Placed Resource Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12PlacedResourceCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;

    thisCreateInfo.appendFormattedString("<ID3D12Heap name=\"pHeap\">0x%p</ID3D12Heap>", mHeap);
    thisCreateInfo.appendFormattedString("<UINT64 name=\"HeapOffset\">%llu</UINT64>", mHeapOffset);
    Wrapped_ID3D12ResourceCreateInfo::AppendCreateInfoXML(thisCreateInfo);

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12PlacedResource", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12ReservedResourceCreateInfo.
/// \param inResourceDescription The description for the resource being created.
/// \param inInitialState The initial state for the resource being created.
/// \param inOptimizedClearValue The clear value for the resource being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ReservedResourceCreateInfo::Wrapped_ID3D12ReservedResourceCreateInfo(const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue)
    : Wrapped_ID3D12ResourceCreateInfo(inResourceDescription, inInitialState, inOptimizedClearValue)
{
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12ReservedResourceCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ReservedResourceCreateInfo::~Wrapped_ID3D12ReservedResourceCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Reserved Resource Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12ReservedResourceCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    Wrapped_ID3D12ResourceCreateInfo::AppendCreateInfoXML(thisCreateInfo);
    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12ComittedResource", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12CommandAllocatorCreateInfo.
/// \param inCommandListType The type of Command List being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandAllocatorCreateInfo::Wrapped_ID3D12CommandAllocatorCreateInfo(D3D12_COMMAND_LIST_TYPE inCommandListType)
    : mCommandListType(inCommandListType)
{
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12CommandAllocatorCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandAllocatorCreateInfo::~Wrapped_ID3D12CommandAllocatorCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Command Allocator Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandAllocatorCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo.appendFormattedString("<STRING name=\"type\">%s</STRING>", DX12CoreSerializers::WriteCommandListTypeEnumAsString(mCommandListType));

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12CommandAllocator", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12FenceCreateInfo.
/// \param inInitialValue The initial value for the fence being created.
/// \param inFlags The flags for the fence being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12FenceCreateInfo::Wrapped_ID3D12FenceCreateInfo(UINT64 inInitialValue, D3D12_FENCE_FLAGS inFlags)
    : mInitialValue(inInitialValue)
    , mFlags(inFlags)
{
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12FenceCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12FenceCreateInfo::~Wrapped_ID3D12FenceCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Fence Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12FenceCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo.appendFormattedString("<UINT64 name=\"InitialValue\">%llu</UINT64>", mInitialValue);
    thisCreateInfo.appendFormattedString("<STRING name=\"Flags\">%s</STRING>", DX12CoreSerializers::WriteFenceFlagsEnumAsString(mFlags));

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12Fence", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12GraphicsPipelineStateCreateInfo.
/// \param inGraphicsPipelineDescription The graphics pipeline description for the new graphics pipeline being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12GraphicsPipelineStateCreateInfo::Wrapped_ID3D12GraphicsPipelineStateCreateInfo(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* inGraphicsPipelineDescription)
{
    mGraphicsPipelineDescription = new D3D12_GRAPHICS_PIPELINE_STATE_DESC;
    DX12CoreDeepCopy::DeepCopy(inGraphicsPipelineDescription, mGraphicsPipelineDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12GraphicsPipelineStateCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12GraphicsPipelineStateCreateInfo::~Wrapped_ID3D12GraphicsPipelineStateCreateInfo()
{
    SAFE_DELETE(mGraphicsPipelineDescription);
}

//-----------------------------------------------------------------------------
/// Append the Graphics Pipeline Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12GraphicsPipelineStateCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo = DX12CoreSerializers::WriteGraphicsPipelineStateDescStructAsString(*mGraphicsPipelineDescription, thisCreateInfo, "pDesc");

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12GraphicsPipelineState", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12ComputePipelineStateCreateInfo.
/// \param inComputePipelineDescription The compute pipeline description for the new compute pipeline being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ComputePipelineStateCreateInfo::Wrapped_ID3D12ComputePipelineStateCreateInfo(const D3D12_COMPUTE_PIPELINE_STATE_DESC* inComputePipelineDescription)
{
    mComputePipelineDescription = new D3D12_COMPUTE_PIPELINE_STATE_DESC;
    DX12CoreDeepCopy::DeepCopy(inComputePipelineDescription, mComputePipelineDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12ComputePipelineStateCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12ComputePipelineStateCreateInfo::~Wrapped_ID3D12ComputePipelineStateCreateInfo()
{
    SAFE_DELETE(mComputePipelineDescription);
}

//-----------------------------------------------------------------------------
/// Append the Compute Pipeline Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12ComputePipelineStateCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo = DX12CoreSerializers::WriteComputePipelineStateDescStructAsString(*mComputePipelineDescription, thisCreateInfo, "pDesc");

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12ComputePipelineState", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12DescriptorHeapCreateInfo.
/// \param inDescription The description structure for the new Descriptor Heap being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12DescriptorHeapCreateInfo::Wrapped_ID3D12DescriptorHeapCreateInfo(const D3D12_DESCRIPTOR_HEAP_DESC* inDescription)
{
    mDescription = new D3D12_DESCRIPTOR_HEAP_DESC;
    DX12CoreDeepCopy::DeepCopy(inDescription, mDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12DescriptorHeapCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12DescriptorHeapCreateInfo::~Wrapped_ID3D12DescriptorHeapCreateInfo()
{
    SAFE_DELETE(mDescription);
}

//-----------------------------------------------------------------------------
/// Append the Descriptor Heap Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12DescriptorHeapCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo = DX12CoreSerializers::WriteDescriptorHeapDescStructAsString(*mDescription, thisCreateInfo, "pDesc");

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12DescriptorHeap", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12QueryHeapCreateInfo.
/// \param inDescription The description structure for the new Query Heap being created.
//-----------------------------------------------------------------------------
Wrapped_ID3D12QueryHeapCreateInfo::Wrapped_ID3D12QueryHeapCreateInfo(const D3D12_QUERY_HEAP_DESC* inDescription)
{
    mDescription = new D3D12_QUERY_HEAP_DESC;
    DX12CoreDeepCopy::DeepCopy(inDescription, mDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12QueryHeapCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12QueryHeapCreateInfo::~Wrapped_ID3D12QueryHeapCreateInfo()
{
    SAFE_DELETE(mDescription);
}

//-----------------------------------------------------------------------------
/// Append the Query Heap Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12QueryHeapCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo = DX12CoreSerializers::WriteQueryHeapDescStructAsString(*mDescription, thisCreateInfo, "pDesc");

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12QueryHeap", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12CommandSignatureCreateInfo.
/// \param inDescription The description structure for the new Command Signature being created.
/// \param inRootSignature A pointer to the Root Signature used to create the new Command Signature.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandSignatureCreateInfo::Wrapped_ID3D12CommandSignatureCreateInfo(const D3D12_COMMAND_SIGNATURE_DESC* inDescription, Wrapped_ID3D12RootSignature* inRootSignature)
    : mRootSignature(inRootSignature)
{
    mDescription = new D3D12_COMMAND_SIGNATURE_DESC;
    DX12CoreDeepCopy::DeepCopy(inDescription, mDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12CommandSignatureCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandSignatureCreateInfo::~Wrapped_ID3D12CommandSignatureCreateInfo()
{
    SAFE_DELETE(mDescription);
}

//-----------------------------------------------------------------------------
/// Append the Command Signature Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandSignatureCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo = DX12CoreSerializers::WriteCommandSignatureDescStructAsString(*mDescription, thisCreateInfo, "pDesc");
    thisCreateInfo.appendFormattedString("<ID3D12RootSignature name=\"pRootSignature\">0x%p</ID3D12RootSignature>", mRootSignature);

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12CommandSignature", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12CommandListCreateInfo.
/// \param inNodeMask The nodemask for the new Command List being created.
/// \param inType the type of Command List to create.
/// \param inCommandAllocator The Command Allocator to be used with the new Command List.
/// \param inInitialState The initial state for the new Command List.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandListCreateInfo::Wrapped_ID3D12CommandListCreateInfo(UINT inNodeMask, D3D12_COMMAND_LIST_TYPE inType, Wrapped_ID3D12CommandAllocator* inCommandAllocator, Wrapped_ID3D12PipelineState* inInitialState)
    : mNodeMask(inNodeMask)
    , mType(inType)
    , mCommandAllocator(inCommandAllocator)
    , mInitialState(inInitialState)
{
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12CommandListCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandListCreateInfo::~Wrapped_ID3D12CommandListCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Command List Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandListCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo.appendFormattedString("<UINT name=\"NodeMask\">%d</UINT>", mNodeMask);
    thisCreateInfo.appendFormattedString("<STRING name=\"Type\">%s</STRING>", DX12CoreSerializers::WriteCommandListTypeEnumAsString(mType));
    thisCreateInfo.appendFormattedString("<ID3D12CommandAllocator name=\"pCommandAllocator\">0x%p</ID3D12CommandAllocator>", mCommandAllocator);
    thisCreateInfo.appendFormattedString("<ID3D12PipelineState name=\"pInitialState\">0x%p</ID3D12PipelineState>", mInitialState);

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12CommandList", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12CommandQueueCreateInfo.
/// \param inQueueDescription A structure containing the QueueCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandQueueCreateInfo::Wrapped_ID3D12CommandQueueCreateInfo(const D3D12_COMMAND_QUEUE_DESC* inQueueDescription)
{
    mQueueDescription = new D3D12_COMMAND_QUEUE_DESC;
    DX12CoreDeepCopy::DeepCopy(inQueueDescription, mQueueDescription);
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12CommandQueueCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandQueueCreateInfo::~Wrapped_ID3D12CommandQueueCreateInfo()
{
    // Destroy the copied Queue description structure.
    SAFE_DELETE(mQueueDescription);
}

//-----------------------------------------------------------------------------
/// Append the Command Queue Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.F
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandQueueCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    DX12CoreSerializers::WriteCommandQueueDescStructAsString(*mQueueDescription, thisCreateInfo, "mDesc");

    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12CommandQueue", thisCreateInfo);
}

//-----------------------------------------------------------------------------
/// Default constructor for Wrapped_ID3D12DeviceCreateInfo.
/// \param inAdapter The incoming adapter used to create the device.
/// \param inFeatureLevel The incoming feature level to target when creating the device.
//-----------------------------------------------------------------------------
Wrapped_ID3D12DeviceCreateInfo::Wrapped_ID3D12DeviceCreateInfo(IUnknown* inAdapter, D3D_FEATURE_LEVEL inFeatureLevel)
    : mAdapater(inAdapter)
    , mMinimumFeatureLevel(inFeatureLevel)
{
}

//-----------------------------------------------------------------------------
/// Destructor for Wrapped_ID3D12DeviceCreateInfo.
//-----------------------------------------------------------------------------
Wrapped_ID3D12DeviceCreateInfo::~Wrapped_ID3D12DeviceCreateInfo()
{
}

//-----------------------------------------------------------------------------
/// Append the Device Create Info to the incoming XML string.
/// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12DeviceCreateInfo::AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const
{
    gtASCIIString thisCreateInfo;
    thisCreateInfo.appendFormattedString("<IUNKNOWN name=\"pAdapter\">0x%p</IUNKNOWN>", mAdapater);
    thisCreateInfo.appendFormattedString("<D3D_FEATURE_LEVEL name=\"MinimumFeatureLevel\">%s</D3D_FEATURE_LEVEL>", DX12CustomSerializers::WriteD3DFeatureLevel(mMinimumFeatureLevel));
    outCreateInfoXML = DX12Util::SurroundWithNamedElement("ID3D12Device", thisCreateInfo);
}