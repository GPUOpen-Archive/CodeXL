//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12Device.cpp
/// \brief A class used to wrap D3D12's ID3D12Device interface.
//=============================================================================

#include "Wrapped_ID3D12Device.h"
#include "../DX12CreateInfoStructs.h"
#include "../DX12ObjectDatabaseProcessor.h"
#include "../BaseWrappers/Wrapped_ID3D12CommandAllocator.h"
#include "../BaseWrappers/Wrapped_ID3D12PipelineState.h"
#include "../BaseWrappers/Wrapped_ID3D12CommandList.h"
#include "../BaseWrappers/Wrapped_ID3D12DescriptorHeap.h"
#include "../BaseWrappers/Wrapped_ID3D12RootSignature.h"
#include "../BaseWrappers/Wrapped_ID3D12Resource.h"
#include "../BaseWrappers/Wrapped_ID3D12Heap.h"
#include "../BaseWrappers/Wrapped_ID3D12Fence.h"
#include "../BaseWrappers/Wrapped_ID3D12QueryHeap.h"
#include "../BaseWrappers/Wrapped_ID3D12CommandSignature.h"
#include "../../DX12LayerManager.h"
#include "../../Interception/DX12Interceptor.h"
#include "../../Util/DX12CoreDeepCopy.h"
#include "../../../Common/IUnknownWrapperGUID.h"

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inRealDevice The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12Device(ID3D12Device** inRealDevice, Wrapped_ID3D12DeviceCreateInfo* inCreateInfo)
{
    return GenericWrapObject<ID3D12Device, Wrapped_ID3D12Device>(nullptr, inRealDevice, kObjectType_ID3D12Device, inCreateInfo);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::QueryInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::QueryInterface(REFIID riid, void** ppvObject)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    if (riid == IID_IWrappedObject)
    {
        *ppvObject = mRealDevice;
        result = S_OK;
    }
    else
    {
        DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

        if (interceptor->ShouldCollectTrace())
        {
            ParameterEntry parameters[] =
            {
                { PARAMETER_REFIID, &riid },
                { PARAMETER_POINTER, ppvObject },
            };

            int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
            DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_QueryInterface, numParameters, parameters);
            result = mRealDevice->QueryInterface(riid, ppvObject);
            interceptor->PostCall(pNewEntry, result);
        }
        else
        {
            result = mRealDevice->QueryInterface(riid, ppvObject);
        }

        if (result == S_OK)
        {
            if (riid == __uuidof(ID3D12Device))
            {
                WrapD3D12Device((ID3D12Device**)ppvObject);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::AddRef
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12Device::AddRef()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_AddRef, 0, nullptr);
        result = mRealDevice->AddRef();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->AddRef();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::Release
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12Device::Release()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_Release, 0, nullptr);
        result = mRealDevice->Release();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->Release();
    }

    if (result == 0)
    {
        DX12WrappedObjectDatabase* objectDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();
        IDX12InstanceBase* objectMetadata = objectDatabase->GetMetadataObject(this);

        if (objectMetadata != nullptr)
        {
            objectMetadata->FlagAsDestroyed();
        }

        objectDatabase->OnDeviceDestroyed(objectMetadata);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_GUID, &guid },
            { PARAMETER_POINTER, pDataSize },
            { PARAMETER_POINTER, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_GetPrivateData, numParameters, parameters);
        result = mRealDevice->GetPrivateData(guid, pDataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->GetPrivateData(guid, pDataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::SetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_GUID, &guid },
            { PARAMETER_UNSIGNED_INT, &DataSize },
            { PARAMETER_POINTER, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetPrivateData, numParameters, parameters);
        result = mRealDevice->SetPrivateData(guid, DataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->SetPrivateData(guid, DataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::SetPrivateDataInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_GUID, &guid },
            { PARAMETER_POINTER_SPECIAL, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetPrivateDataInterface, numParameters, parameters);
        result = mRealDevice->SetPrivateDataInterface(guid, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->SetPrivateDataInterface(guid, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::SetName
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::SetName(LPCWSTR Name)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_WIDE_STRING, Name },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetName, numParameters, parameters);
        result = mRealDevice->SetName(Name);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->SetName(Name);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetNodeCount
//-----------------------------------------------------------------------------
UINT STDMETHODCALLTYPE Wrapped_ID3D12Device::GetNodeCount()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    UINT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetNodeCount, 0, nullptr);
        result = mRealDevice->GetNodeCount();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->GetNodeCount();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateCommandQueue
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppCommandQueue },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateCommandQueue, numParameters, parameters);
        result = mRealDevice->CreateCommandQueue(pDesc, riid, ppCommandQueue);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateCommandQueue(pDesc, riid, ppCommandQueue);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12CommandQueue))
        {
            Wrapped_ID3D12CommandQueueCreateInfo* createInfo = new Wrapped_ID3D12CommandQueueCreateInfo(pDesc);
            WrapD3D12CommandQueue(this, (ID3D12CommandQueue**)ppCommandQueue, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateCommandAllocator
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** ppCommandAllocator)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_COMMAND_LIST, &type },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppCommandAllocator },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateCommandAllocator, numParameters, parameters);
        result = mRealDevice->CreateCommandAllocator(type, riid, ppCommandAllocator);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateCommandAllocator(type, riid, ppCommandAllocator);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12CommandAllocator))
        {
            Wrapped_ID3D12CommandAllocatorCreateInfo* createInfo = new Wrapped_ID3D12CommandAllocatorCreateInfo(type);
            WrapD3D12CommandAllocator(this, (ID3D12CommandAllocator**)ppCommandAllocator, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateGraphicsPipelineState
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pDescUnwrapped;
    DX12CoreDeepCopy::DeepCopy(pDesc, &pDescUnwrapped);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppPipelineState },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateGraphicsPipelineState, numParameters, parameters);
        result = mRealDevice->CreateGraphicsPipelineState(&pDescUnwrapped, riid, ppPipelineState);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateGraphicsPipelineState(&pDescUnwrapped, riid, ppPipelineState);
    }

    if (result == S_OK)
    {
        Wrapped_ID3D12GraphicsPipelineStateCreateInfo* createInfo = new Wrapped_ID3D12GraphicsPipelineStateCreateInfo(pDesc);
        WrapD3D12PipelineState(this, (ID3D12PipelineState**)ppPipelineState, createInfo);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateComputePipelineState
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    D3D12_COMPUTE_PIPELINE_STATE_DESC pDescUnwrapped;
    DX12CoreDeepCopy::DeepCopy(pDesc, &pDescUnwrapped);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppPipelineState },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateComputePipelineState, numParameters, parameters);
        result = mRealDevice->CreateComputePipelineState(&pDescUnwrapped, riid, ppPipelineState);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateComputePipelineState(&pDescUnwrapped, riid, ppPipelineState);
    }

    if (result == S_OK)
    {
        Wrapped_ID3D12ComputePipelineStateCreateInfo* createInfo = new Wrapped_ID3D12ComputePipelineStateCreateInfo(pDesc);
        WrapD3D12PipelineState(this, (ID3D12PipelineState**)ppPipelineState, createInfo);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateCommandList
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    ID3D12CommandAllocator* pCommandAllocatorUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pCommandAllocator, &(pCommandAllocatorUnwrapped));
    ID3D12PipelineState* pInitialStateUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pInitialState, &(pInitialStateUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &nodeMask },
            { PARAMETER_COMMAND_LIST, &type },
            { PARAMETER_POINTER_SPECIAL, pCommandAllocator },
            { PARAMETER_POINTER_SPECIAL, pInitialState },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppCommandList },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateCommandList, numParameters, parameters);
        result = mRealDevice->CreateCommandList(nodeMask, type, pCommandAllocatorUnwrapped, pInitialStateUnwrapped, riid, ppCommandList);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateCommandList(nodeMask, type, pCommandAllocatorUnwrapped, pInitialStateUnwrapped, riid, ppCommandList);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12CommandList))
        {
            Log(logWARNING, "Created type 'ID3D12CommandList'. It was wrapped, but couldn't store CreateInfo.\n");
            WrapD3D12CommandList(this, (ID3D12CommandList**)ppCommandList);
        }
        else if (riid == __uuidof(ID3D12GraphicsCommandList))
        {
            Wrapped_ID3D12CommandListCreateInfo* createInfo = new Wrapped_ID3D12CommandListCreateInfo(nodeMask, type, static_cast<Wrapped_ID3D12CommandAllocator*>(pCommandAllocator), static_cast<Wrapped_ID3D12PipelineState*>(pInitialState));
            WrapD3D12GraphicsCommandList(this, (ID3D12GraphicsCommandList**)ppCommandList, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CheckFeatureSupport
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_FEATURE, &Feature },
            { PARAMETER_POINTER, pFeatureSupportData },
            { PARAMETER_UNSIGNED_INT, &FeatureSupportDataSize },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CheckFeatureSupport, numParameters, parameters);
        result = mRealDevice->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateDescriptorHeap
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDescriptorHeapDesc },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvHeap },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateDescriptorHeap, numParameters, parameters);
        result = mRealDevice->CreateDescriptorHeap(pDescriptorHeapDesc, riid, ppvHeap);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateDescriptorHeap(pDescriptorHeapDesc, riid, ppvHeap);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12DescriptorHeap))
        {
            Wrapped_ID3D12DescriptorHeapCreateInfo* createInfo = new Wrapped_ID3D12DescriptorHeapCreateInfo(pDescriptorHeapDesc);
            WrapD3D12DescriptorHeap(this, (ID3D12DescriptorHeap**)ppvHeap, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetDescriptorHandleIncrementSize
//-----------------------------------------------------------------------------
UINT STDMETHODCALLTYPE Wrapped_ID3D12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    UINT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_DESCRIPTOR_HEAP, &DescriptorHeapType },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetDescriptorHandleIncrementSize, numParameters, parameters);
        result = mRealDevice->GetDescriptorHandleIncrementSize(DescriptorHeapType);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->GetDescriptorHandleIncrementSize(DescriptorHeapType);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateRootSignature
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void** ppvRootSignature)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &nodeMask },
            { PARAMETER_POINTER, pBlobWithRootSignature },
            { PARAMETER_SIZE_T, &blobLengthInBytes },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvRootSignature },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateRootSignature, numParameters, parameters);
        result = mRealDevice->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12RootSignature))
        {
            Wrapped_ID3D12RootSignatureCreateInfo* createInfo = new Wrapped_ID3D12RootSignatureCreateInfo(nodeMask, pBlobWithRootSignature, blobLengthInBytes);
            WrapD3D12RootSignature(this, (ID3D12RootSignature**)ppvRootSignature, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateConstantBufferView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_POINTER, (void*)DestDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateConstantBufferView, numParameters, parameters);
        mRealDevice->CreateConstantBufferView(pDesc, DestDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CreateConstantBufferView(pDesc, DestDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateShaderResourceView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_POINTER, (void*)DestDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateShaderResourceView, numParameters, parameters);
        mRealDevice->CreateShaderResourceView(pResourceUnwrapped, pDesc, DestDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CreateShaderResourceView(pResourceUnwrapped, pDesc, DestDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateUnorderedAccessView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));
    ID3D12Resource* pCounterResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pCounterResource, &(pCounterResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_POINTER_SPECIAL, pCounterResource },
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_POINTER, (void*)DestDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateUnorderedAccessView, numParameters, parameters);
        mRealDevice->CreateUnorderedAccessView(pResourceUnwrapped, pCounterResourceUnwrapped, pDesc, DestDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CreateUnorderedAccessView(pResourceUnwrapped, pCounterResourceUnwrapped, pDesc, DestDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateRenderTargetView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_POINTER, (void*)DestDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateRenderTargetView, numParameters, parameters);
        mRealDevice->CreateRenderTargetView(pResourceUnwrapped, pDesc, DestDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CreateRenderTargetView(pResourceUnwrapped, pDesc, DestDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateDepthStencilView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_POINTER, (void*)DestDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateDepthStencilView, numParameters, parameters);
        mRealDevice->CreateDepthStencilView(pResourceUnwrapped, pDesc, DestDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CreateDepthStencilView(pResourceUnwrapped, pDesc, DestDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateSampler
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_POINTER, (void*)DestDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateSampler, numParameters, parameters);
        mRealDevice->CreateSampler(pDesc, DestDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CreateSampler(pDesc, DestDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CopyDescriptors
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CopyDescriptors(UINT NumDestDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, const UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, const UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        int maxParameters = ((NumDestDescriptorRanges + NumSrcDescriptorRanges) * 2) + 3;
        int numParameters = 0;
        ParameterEntry* parameters = new ParameterEntry[maxParameters];

        // Build a string with the array of destination ranges if there are any.
        parameters[numParameters].mType = PARAMETER_UNSIGNED_INT;
        parameters[numParameters].mData = &NumDestDescriptorRanges;
        numParameters++;

        if (NumDestDescriptorRanges > 0)
        {
            for (UINT loop = 0; loop < NumDestDescriptorRanges; loop++)
            {
                parameters[numParameters].mType = PARAMETER_POINTER;
                parameters[numParameters].mData = &pDestDescriptorRangeStarts[loop];
                numParameters++;
            }
        }

        if (pDestDescriptorRangeSizes != nullptr)
        {
            for (UINT loop = 0; loop < NumDestDescriptorRanges; loop++)
            {
                parameters[numParameters].mType = PARAMETER_UNSIGNED_INT;
                parameters[numParameters].mData = &pDestDescriptorRangeSizes[loop];
                numParameters++;
            }
        }

        // Build a string with the array of source ranges if there are any.
        parameters[numParameters].mType = PARAMETER_UNSIGNED_INT;
        parameters[numParameters].mData = &NumSrcDescriptorRanges;
        numParameters++;

        if (NumSrcDescriptorRanges > 0)
        {
            for (UINT loop = 0; loop < NumSrcDescriptorRanges; loop++)
            {
                parameters[numParameters].mType = PARAMETER_POINTER;
                parameters[numParameters].mData = &pSrcDescriptorRangeStarts[loop];
                numParameters++;
            }
        }

        if (pSrcDescriptorRangeSizes != nullptr)
        {
            for (UINT loop = 0; loop < NumSrcDescriptorRanges; loop++)
            {
                parameters[numParameters].mType = PARAMETER_UNSIGNED_INT;
                parameters[numParameters].mData = &pSrcDescriptorRangeSizes[loop];
                numParameters++;
            }
        }

        parameters[numParameters].mType = PARAMETER_DESCRIPTOR_HEAP;
        parameters[numParameters].mData = &DescriptorHeapsType;
        numParameters++;

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CopyDescriptors, numParameters, parameters);
        mRealDevice->CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);
        interceptor->PostCall(pNewEntry);
        delete[] parameters;
    }
    else
    {
        mRealDevice->CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CopyDescriptorsSimple
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::CopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &NumDescriptors },
            { PARAMETER_POINTER, (void*)DestDescriptorRangeStart.ptr },
            { PARAMETER_POINTER, (void*)SrcDescriptorRangeStart.ptr },
            { PARAMETER_DESCRIPTOR_HEAP, &DescriptorHeapsType },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CopyDescriptorsSimple, numParameters, parameters);
        mRealDevice->CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetResourceAllocationInfo
//-----------------------------------------------------------------------------
D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE Wrapped_ID3D12Device::GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    D3D12_RESOURCE_ALLOCATION_INFO result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &visibleMask },
            { PARAMETER_UNSIGNED_INT, &numResourceDescs },
            { PARAMETER_POINTER, pResourceDescs },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetResourceAllocationInfo, numParameters, parameters);
        result = mRealDevice->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);
        // TODO: get return value
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        result = mRealDevice->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetCustomHeapProperties
//-----------------------------------------------------------------------------
D3D12_HEAP_PROPERTIES STDMETHODCALLTYPE Wrapped_ID3D12Device::GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE heapType)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    D3D12_HEAP_PROPERTIES result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &nodeMask },
            { PARAMETER_HEAP_TYPE, &heapType },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetCustomHeapProperties, numParameters, parameters);
        result = mRealDevice->GetCustomHeapProperties(nodeMask, heapType);
        // TODO: get return value
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        result = mRealDevice->GetCustomHeapProperties(nodeMask, heapType);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateCommittedResource
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pHeapProperties },
            { PARAMETER_UNSIGNED_INT, &HeapFlags },
            { PARAMETER_POINTER, pResourceDesc },
            { PARAMETER_RESOURCE_STATES, &InitialResourceState },
            { PARAMETER_POINTER, pOptimizedClearValue },
            { PARAMETER_REFIID, &riidResource },
            { PARAMETER_POINTER, ppvResource },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateCommittedResource, numParameters, parameters);
        result = mRealDevice->CreateCommittedResource(pHeapProperties, HeapFlags, pResourceDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateCommittedResource(pHeapProperties, HeapFlags, pResourceDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

    if (result == S_OK)
    {
        if (riidResource == __uuidof(ID3D12Resource))
        {
            Wrapped_ID3D12ComittedResourceCreateInfo* createInfo = new Wrapped_ID3D12ComittedResourceCreateInfo(pHeapProperties, HeapFlags, pResourceDesc, InitialResourceState, pOptimizedClearValue);
            WrapD3D12Resource(this, (ID3D12Resource**)ppvResource, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateHeap
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateHeap(const D3D12_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvHeap },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateHeap, numParameters, parameters);
        result = mRealDevice->CreateHeap(pDesc, riid, ppvHeap);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateHeap(pDesc, riid, ppvHeap);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12Heap))
        {
            Wrapped_ID3D12HeapCreateInfo* createInfo = new Wrapped_ID3D12HeapCreateInfo(pDesc);
            WrapD3D12Heap(this, (ID3D12Heap**)ppvHeap, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreatePlacedResource
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreatePlacedResource(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    ID3D12Heap* pHeapUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pHeap, &(pHeapUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pHeap },
            { PARAMETER_UINT64, &HeapOffset },
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_RESOURCE_STATES, &InitialState },
            { PARAMETER_POINTER, pOptimizedClearValue },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvResource },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreatePlacedResource, numParameters, parameters);
        result = mRealDevice->CreatePlacedResource(pHeapUnwrapped, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreatePlacedResource(pHeapUnwrapped, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12Resource))
        {
            Wrapped_ID3D12PlacedResourceCreateInfo* createInfo = new Wrapped_ID3D12PlacedResourceCreateInfo(static_cast<Wrapped_ID3D12Heap*>(pHeap), HeapOffset, pDesc, InitialState, pOptimizedClearValue);
            WrapD3D12Resource(this, (ID3D12Resource**)ppvResource, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateReservedResource
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_RESOURCE_STATES, &InitialState },
            { PARAMETER_POINTER, pOptimizedClearValue },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvResource },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateReservedResource, numParameters, parameters);
        result = mRealDevice->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12Resource))
        {
            Wrapped_ID3D12ReservedResourceCreateInfo* createInfo = new Wrapped_ID3D12ReservedResourceCreateInfo(pDesc, InitialState, pOptimizedClearValue);
            WrapD3D12Resource(this, (ID3D12Resource**)ppvResource, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateSharedHandle
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateSharedHandle(ID3D12DeviceChild* pObject, const SECURITY_ATTRIBUTES* pAttributes, DWORD Access, LPCWSTR Name, HANDLE* pHandle)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    ID3D12DeviceChild* pObjectUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pObject, &(pObjectUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pObject },
            { PARAMETER_POINTER, pAttributes },
            { PARAMETER_UNSIGNED_INT, &Access },
            { PARAMETER_WIDE_STRING, Name },
            { PARAMETER_POINTER, pHandle },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateSharedHandle, numParameters, parameters);
        result = mRealDevice->CreateSharedHandle(pObjectUnwrapped, pAttributes, Access, Name, pHandle);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateSharedHandle(pObjectUnwrapped, pAttributes, Access, Name, pHandle);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::OpenSharedHandle
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::OpenSharedHandle(HANDLE NTHandle, REFIID riid, void** ppvObj)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, NTHandle },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvObj },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_OpenSharedHandle, numParameters, parameters);
        result = mRealDevice->OpenSharedHandle(NTHandle, riid, ppvObj);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->OpenSharedHandle(NTHandle, riid, ppvObj);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::OpenSharedHandleByName
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::OpenSharedHandleByName(LPCWSTR Name, DWORD Access, HANDLE* pNTHandle)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_WIDE_STRING, Name },
            { PARAMETER_UNSIGNED_INT, &Access },
            { PARAMETER_POINTER, pNTHandle },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_OpenSharedHandleByName, numParameters, parameters);
        result = mRealDevice->OpenSharedHandleByName(Name, Access, pNTHandle);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->OpenSharedHandleByName(Name, Access, pNTHandle);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::MakeResident
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::MakeResident(UINT NumObjects, ID3D12Pageable* const* ppObjects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    ID3D12Pageable** ppObjectsUnwrapped = nullptr;

    if (NumObjects > 0)
    {
        ppObjectsUnwrapped = new ID3D12Pageable*[NumObjects];

        for (UINT index = 0; index < NumObjects; index++)
        {
            DX12CoreDeepCopy::UnwrapInterface(ppObjects[index], &(ppObjectsUnwrapped[index]));
        }
    }

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        int numParameters = NumObjects + 1;
        ParameterEntry* parameters = new ParameterEntry[numParameters];
        parameters[0].mType = PARAMETER_UNSIGNED_INT;
        parameters[0].mData = &NumObjects;

        for (UINT loop = 0; loop < NumObjects; loop++)
        {
            parameters[loop + 1].mType = PARAMETER_POINTER;
            parameters[loop + 1].mData = ppObjects[loop];
        }

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_MakeResident, numParameters, parameters);
        result = mRealDevice->MakeResident(NumObjects, ppObjectsUnwrapped);
        interceptor->PostCall(pNewEntry, result);
        delete[] parameters;
    }
    else
    {
        result = mRealDevice->MakeResident(NumObjects, ppObjectsUnwrapped);
    }

    SAFE_DELETE_ARRAY(ppObjectsUnwrapped);

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::Evict
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::Evict(UINT NumObjects, ID3D12Pageable* const* ppObjects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    ID3D12Pageable** ppObjectsUnwrapped = nullptr;

    if (NumObjects > 0)
    {
        ppObjectsUnwrapped = new ID3D12Pageable*[NumObjects];

        for (UINT index = 0; index < NumObjects; index++)
        {
            DX12CoreDeepCopy::UnwrapInterface(ppObjects[index], &(ppObjectsUnwrapped[index]));
        }
    }

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        int numParameters = NumObjects + 1;
        ParameterEntry* parameters = new ParameterEntry[numParameters];
        parameters[0].mType = PARAMETER_UNSIGNED_INT;
        parameters[0].mData = &NumObjects;

        for (UINT loop = 0; loop < NumObjects; loop++)
        {
            parameters[loop + 1].mType = PARAMETER_POINTER;
            parameters[loop + 1].mData = ppObjects[loop];
        }

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_Evict, numParameters, parameters);
        result = mRealDevice->Evict(NumObjects, ppObjectsUnwrapped);
        interceptor->PostCall(pNewEntry, result);
        delete[] parameters;
    }
    else
    {
        result = mRealDevice->Evict(NumObjects, ppObjectsUnwrapped);
    }

    SAFE_DELETE_ARRAY(ppObjectsUnwrapped);

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateFence
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateFence(UINT64 InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void** ppFence)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UINT64, &InitialValue },
            { PARAMETER_UNSIGNED_INT, &Flags },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppFence },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateFence, numParameters, parameters);
        result = mRealDevice->CreateFence(InitialValue, Flags, riid, ppFence);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateFence(InitialValue, Flags, riid, ppFence);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12Fence))
        {
            Wrapped_ID3D12FenceCreateInfo* createInfo = new Wrapped_ID3D12FenceCreateInfo(InitialValue, Flags);
            WrapD3D12Fence(this, (ID3D12Fence**)ppFence, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetDeviceRemovedReason
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::GetDeviceRemovedReason()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetDeviceRemovedReason, 0, nullptr);
        result = mRealDevice->GetDeviceRemovedReason();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->GetDeviceRemovedReason();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetCopyableFootprints
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        UINT numRows = pNumRows != nullptr ? *pNumRows : 0;
        UINT64 rowSizeInBytes = pRowSizeInBytes != nullptr ? *pRowSizeInBytes : 0;
        UINT64 totalBytes = pTotalBytes != nullptr ? *pTotalBytes : 0;

        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pResourceDesc },
            { PARAMETER_UNSIGNED_INT, &FirstSubresource },
            { PARAMETER_UNSIGNED_INT, &NumSubresources },
            { PARAMETER_UINT64, &BaseOffset },
            { PARAMETER_POINTER, pLayouts },
            { PARAMETER_UNSIGNED_INT, &numRows },
            { PARAMETER_UINT64, &rowSizeInBytes },
            { PARAMETER_UINT64, &totalBytes },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetCopyableFootprints, numParameters, parameters);
        mRealDevice->GetCopyableFootprints(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->GetCopyableFootprints(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateQueryHeap
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDesc },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvHeap },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateQueryHeap, numParameters, parameters);
        result = mRealDevice->CreateQueryHeap(pDesc, riid, ppvHeap);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateQueryHeap(pDesc, riid, ppvHeap);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12QueryHeap))
        {
            Wrapped_ID3D12QueryHeapCreateInfo* createInfo = new Wrapped_ID3D12QueryHeapCreateInfo(pDesc);
            WrapD3D12QueryHeap(this, (ID3D12QueryHeap**)ppvHeap, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::SetStablePowerState
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::SetStablePowerState(BOOL Enable)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_BOOL, &Enable },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_SetStablePowerState, numParameters, parameters);
        result = mRealDevice->SetStablePowerState(Enable);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->SetStablePowerState(Enable);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::CreateCommandSignature
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Device::CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* pDesc, ID3D12RootSignature* pRootSignature, REFIID riid, void** ppvCommandSignature)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    ID3D12RootSignature* pRootSignatureUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pRootSignature, &(pRootSignatureUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pDesc },
            { PARAMETER_POINTER_SPECIAL, pRootSignature },
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvCommandSignature },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_CreateCommandSignature, numParameters, parameters);
        result = mRealDevice->CreateCommandSignature(pDesc, pRootSignatureUnwrapped, riid, ppvCommandSignature);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealDevice->CreateCommandSignature(pDesc, pRootSignatureUnwrapped, riid, ppvCommandSignature);
    }

    if (result == S_OK)
    {
        if (riid == __uuidof(ID3D12CommandSignature))
        {
            Wrapped_ID3D12CommandSignatureCreateInfo* createInfo = new Wrapped_ID3D12CommandSignatureCreateInfo(pDesc, static_cast<Wrapped_ID3D12RootSignature*>(pRootSignature));
            WrapD3D12CommandSignature(this, (ID3D12CommandSignature**)ppvCommandSignature, createInfo);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetResourceTiling
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12Device::GetResourceTiling(ID3D12Resource* pTiledResource, UINT* pNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* pPackedMipDesc, D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips, UINT* pNumSubresourceTilings, UINT FirstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pTiledResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pTiledResource, &(pTiledResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        // print variables with nullptr pointer check
        UINT numTilesForEntireResource = 0;

        if (pNumTilesForEntireResource != nullptr)
        {
            numTilesForEntireResource = *pNumTilesForEntireResource;
        }

        UINT numSubresourceTilings = 0;

        if (pNumSubresourceTilings != nullptr)
        {
            numSubresourceTilings = *pNumSubresourceTilings;
        }

        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pTiledResource },
            { PARAMETER_UNSIGNED_INT, &numTilesForEntireResource },
            { PARAMETER_POINTER, pPackedMipDesc },
            { PARAMETER_POINTER, pStandardTileShapeForNonPackedMips },
            { PARAMETER_UNSIGNED_INT, &numSubresourceTilings },
            { PARAMETER_UNSIGNED_INT, &FirstSubresourceTilingToGet },
            { PARAMETER_POINTER, pSubresourceTilingsForNonPackedMips },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetResourceTiling, numParameters, parameters);
        mRealDevice->GetResourceTiling(pTiledResourceUnwrapped, pNumTilesForEntireResource, pPackedMipDesc, pStandardTileShapeForNonPackedMips, pNumSubresourceTilings, FirstSubresourceTilingToGet, pSubresourceTilingsForNonPackedMips);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealDevice->GetResourceTiling(pTiledResourceUnwrapped, pNumTilesForEntireResource, pPackedMipDesc, pStandardTileShapeForNonPackedMips, pNumSubresourceTilings, FirstSubresourceTilingToGet, pSubresourceTilingsForNonPackedMips);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Device::GetAdapterLuid
//-----------------------------------------------------------------------------
LUID STDMETHODCALLTYPE Wrapped_ID3D12Device::GetAdapterLuid()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    LUID result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Device_GetAdapterLuid, 0);
        result = mRealDevice->GetAdapterLuid();
        // TODO: get return value
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        result = mRealDevice->GetAdapterLuid();
    }

    return result;
}