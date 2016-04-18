//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Declarations of structures designed to hold all the parameters used to create interfaces.
//=============================================================================

#ifndef DX12CREATEINFOSTRUCTS_H
#define DX12CREATEINFOSTRUCTS_H

#include "DX12Defines.h"
#include <AMDTBaseTools/Include/gtASCIIString.h>

class Wrapped_ID3D12Heap;
class Wrapped_ID3D12RootSignature;
class Wrapped_ID3D12CommandAllocator;
class Wrapped_ID3D12PipelineState;

//-----------------------------------------------------------------------------
/// A baseclass for all CreateInfo types.
//-----------------------------------------------------------------------------
class Wrapped_DX12CreateInfoBase
{
public:
    Wrapped_DX12CreateInfoBase() {}
    virtual ~Wrapped_DX12CreateInfoBase() {}

    //-----------------------------------------------------------------------------
    /// Append the Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const = 0;
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Root Signature instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12RootSignatureCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12RootSignatureCreateInfo.
    /// \param inNodeMask The GPU node mask.
    /// \param inBlobWithRootSignature The serialized Root Signature blob.
    /// \param inBlobLengthInBytes The total blob length in bytes.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12RootSignatureCreateInfo(UINT inNodeMask, const void* inBlobWithRootSignature, SIZE_T inBlobLengthInBytes);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12RootSignatureCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12RootSignatureCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Root Signature Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    UINT mNodeMask;                 ///< The incoming node mask.
    char* mBlobWithRootSignature;   ///< The incoming root signature blob.
    SIZE_T mBlobLengthInBytes;      ///< The incoming blob size in bytes.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Root Signature instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12RootSignatureDeserializerCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12RootSignatureDeserializerCreateInfo.
    /// \param pSrcData The incoming deserialized Root Signature source data.
    /// \param SrcDataSizeInBytes The total source data size in bytes.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12RootSignatureDeserializerCreateInfo(LPCVOID pSrcData, SIZE_T SrcDataSizeInBytes);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12RootSignatureDeserializerCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12RootSignatureDeserializerCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Root Signature Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    char* mSrcData;                 ///< The incoming Root Signature source data.
    SIZE_T mSrcDataSizeInBytes;     ///< The incoming Root Signature data size in bytes.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Heap instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12HeapCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12HeapCreateInfo.
    /// \param inDescription The description structure used to create the instance.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12HeapCreateInfo(const D3D12_HEAP_DESC* inDescription);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12HeapCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12HeapCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Heap Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_HEAP_DESC* mDescription;      ///< The incoming heap description.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Resource instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12ResourceCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12ResourceCreateInfo.
    /// \param inResourceDescription The description of the resource being created.
    /// \param inInitialState The initial state of the resource being created.
    /// \param inOptimizedClearValue The initial clear value for the resource being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12ResourceCreateInfo(const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12ResourceCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12ResourceCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Resource Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

protected:
    D3D12_RESOURCE_DESC* mResourceDescription;      ///< The incoming resource description.
    D3D12_RESOURCE_STATES mInitialState;            ///< The incoming initial resource state.
    D3D12_CLEAR_VALUE* mOptimizedClearValue;        ///< The incoming clear value if provided.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Resource instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12ComittedResourceCreateInfo : public Wrapped_ID3D12ResourceCreateInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12ComittedResourceCreateInfo.
    /// \param pHeapProperties The properties for the heap that this resource will use.
    /// \param inHeapFlags The flags for the heap that will be created.
    /// \param inResourceDescription The description for the resource to be created.
    /// \param inInitialState The initial state for the resource being created.
    /// \param inOptimizedClearValue The clear value for the resource being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12ComittedResourceCreateInfo(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS inHeapFlags, const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12ComittedResourceCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12ComittedResourceCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Comitted Resource Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_HEAP_PROPERTIES* mHeapProperties;     ///< Incoming heap creation properties.
    D3D12_HEAP_FLAGS mHeapFlags;                ///< Incoming heap creation flags.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Resource instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12PlacedResourceCreateInfo : public Wrapped_ID3D12ResourceCreateInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12PlacedResourceCreateInfo.
    /// \param inHeap The heap used for the new resource.
    /// \param inHeapOffset The heap offset for this resource.
    /// \param inResourceDescription The description for the resource being created.
    /// \param inInitialState The initial state for the resource being created.
    /// \param inOptimizedClearValue The clear value for the resource being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12PlacedResourceCreateInfo(Wrapped_ID3D12Heap* inHeap, UINT64 inHeapOffset, const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12PlacedResourceCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12PlacedResourceCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Placed Resource Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    Wrapped_ID3D12Heap* mHeap;  ///< The incoming heap pointer.
    UINT64 mHeapOffset;         ///< The incoming placed heap offset.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Resource instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12ReservedResourceCreateInfo : public Wrapped_ID3D12ResourceCreateInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12ReservedResourceCreateInfo.
    /// \param inResourceDescription The description for the resource being created.
    /// \param inInitialState The initial state for the resource being created.
    /// \param inOptimizedClearValue The clear value for the resource being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12ReservedResourceCreateInfo(const D3D12_RESOURCE_DESC* inResourceDescription, D3D12_RESOURCE_STATES inInitialState, const D3D12_CLEAR_VALUE* inOptimizedClearValue);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12ReservedResourceCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12ReservedResourceCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Reserved Resource Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new CommandAllocator instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12CommandAllocatorCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12CommandAllocatorCreateInfo.
    /// \param inCommandListType The type of Command List being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12CommandAllocatorCreateInfo(D3D12_COMMAND_LIST_TYPE inCommandListType);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12CommandAllocatorCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12CommandAllocatorCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Command Allocator Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_COMMAND_LIST_TYPE mCommandListType;   ///< The incoming Command List type.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Fence instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12FenceCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12FenceCreateInfo.
    /// \param inInitialValue The initial value for the fence being created.
    /// \param inFlags The flags for the fence being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12FenceCreateInfo(UINT64 inInitialValue, D3D12_FENCE_FLAGS inFlags);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12FenceCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12FenceCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Fence Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    UINT64 mInitialValue;           ///< The incoming initial value for the fence.
    D3D12_FENCE_FLAGS mFlags;       ///< The incoming Fence creation flags.
};

//-----------------------------------------------------------------------------
/// A baseclass designed of the type used to hold info used to create a PipelineState instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12PipelineStateCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for the PipelineStateCreateInfo class.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12PipelineStateCreateInfo() {}

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12PipelineStateCreateInfo() {}
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Graphics PipelineState instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12GraphicsPipelineStateCreateInfo : public Wrapped_ID3D12PipelineStateCreateInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12GraphicsPipelineStateCreateInfo.
    /// \param inGraphicsPipelineDescription The graphics pipeline description for the new graphics pipeline being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12GraphicsPipelineStateCreateInfo(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* inGraphicsPipelineDescription);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12GraphicsPipelineStateCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12GraphicsPipelineStateCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Graphics Pipeline Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC* mGraphicsPipelineDescription;   ///< The incoming Graphics Pipeline State description.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Compute PipelineState instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12ComputePipelineStateCreateInfo : public Wrapped_ID3D12PipelineStateCreateInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12ComputePipelineStateCreateInfo.
    /// \param inComputePipelineDescription The compute pipeline description for the new compute pipeline being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12ComputePipelineStateCreateInfo(const D3D12_COMPUTE_PIPELINE_STATE_DESC* inComputePipelineDescription);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12ComputePipelineStateCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12ComputePipelineStateCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Compute Pipeline Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_COMPUTE_PIPELINE_STATE_DESC* mComputePipelineDescription; ///< The incoming Compute Pipeline State description.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new DescriptorHeap instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12DescriptorHeapCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12DescriptorHeapCreateInfo.
    /// \param inDescription The description structure for the new Descriptor Heap being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12DescriptorHeapCreateInfo(const D3D12_DESCRIPTOR_HEAP_DESC* inDescription);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12DescriptorHeapCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12DescriptorHeapCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Descriptor Heap Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_DESCRIPTOR_HEAP_DESC* mDescription;   ///< The incoming Descriptor Heap description.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new QueryHeap instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12QueryHeapCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12QueryHeapCreateInfo.
    /// \param inDescription The description structure for the new Query Heap being created.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12QueryHeapCreateInfo(const D3D12_QUERY_HEAP_DESC* inDescription);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12QueryHeapCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12QueryHeapCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Query Heap Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_QUERY_HEAP_DESC* mDescription;        ///< The incoming Query Heap description.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new CommandSignature instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12CommandSignatureCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12CommandSignatureCreateInfo.
    /// \param inDescription The description structure for the new Command Signature being created.
    /// \param inRootSignature A pointer to the Root Signature used to create the new Command Signature.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12CommandSignatureCreateInfo(const D3D12_COMMAND_SIGNATURE_DESC* inDescription, Wrapped_ID3D12RootSignature* inRootSignature);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12CommandSignatureCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12CommandSignatureCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Command Signature Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    D3D12_COMMAND_SIGNATURE_DESC* mDescription;         ///< The incoming Command Signature description.
    Wrapped_ID3D12RootSignature* mRootSignature;        ///< The incoming root signature instance.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new CommandList instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12CommandListCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12CommandListCreateInfo.
    /// \param inNodeMask The nodemask for the new Command List being created.
    /// \param inType the type of Command List to create.
    /// \param inCommandAllocator The Command Allocator to be used with the new Command List.
    /// \param inInitialState The initial state for the new Command List.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12CommandListCreateInfo(UINT inNodeMask, D3D12_COMMAND_LIST_TYPE inType, Wrapped_ID3D12CommandAllocator* inCommandAllocator, Wrapped_ID3D12PipelineState* inInitialState);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12CommandListCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12CommandListCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Command List Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    UINT mNodeMask;                                     ///< The incoming Command List NodeMask.
    D3D12_COMMAND_LIST_TYPE mType;                      ///< The incoming Command List type.
    Wrapped_ID3D12CommandAllocator* mCommandAllocator;  ///< The incoming Command List Allocator instance.
    Wrapped_ID3D12PipelineState* mInitialState;         ///< The incoming Pipeline State object instance.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new CommandQueue instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12CommandQueueCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12CommandQueueCreateInfo.
    /// \param inQueueDescription A structure containing the QueueCreateInfo.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12CommandQueueCreateInfo(const D3D12_COMMAND_QUEUE_DESC* inCommandQueueDesc);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12CommandQueueCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12CommandQueueCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Command Queue Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.F
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

    //-----------------------------------------------------------------------------
    /// Get the Command Queue description structure used to create the instance.
    /// \returns The D3D12_COMMAND_QUEUE_DESC structure used to create the instance.
    //-----------------------------------------------------------------------------
    D3D12_COMMAND_QUEUE_DESC* GetDescription() const { return mQueueDescription; }

private:
    D3D12_COMMAND_QUEUE_DESC* mQueueDescription;    ///< Incoming Queue description.
};

//-----------------------------------------------------------------------------
/// A class designed to hold all info used to create a new Device instance.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12DeviceCreateInfo : public Wrapped_DX12CreateInfoBase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Wrapped_ID3D12DeviceCreateInfo.
    /// \param inAdapter The incoming adapter used to create the device.
    /// \param inFeatureLevel The incoming feature level to target when creating the device.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12DeviceCreateInfo(IUnknown* inAdapater, D3D_FEATURE_LEVEL inFeatureLevel);

    //-----------------------------------------------------------------------------
    /// Destructor for Wrapped_ID3D12DeviceCreateInfo.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12DeviceCreateInfo();

    //-----------------------------------------------------------------------------
    /// Append the Device Create Info to the incoming XML string.
    /// \param outCreateInfoXML An XML string containing the CreateInfo used to create the interface instance.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

private:
    IUnknown* mAdapater;                        ///< Incoming device adapter.
    D3D_FEATURE_LEVEL mMinimumFeatureLevel;     ///< Incoming device feature level.
};

#endif // DX12CREATEINFOSTRUCTS_H
