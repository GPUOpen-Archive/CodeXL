//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12CommandQueue.cpp
/// \brief A class used to wrap D3D12's ID3D12CommandQueue interface.
//=============================================================================

#include "Wrapped_ID3D12CommandQueue.h"
#include "../DX12CreateInfoStructs.h"
#include "../DX12ObjectDatabaseProcessor.h"
#include "../../Interception/DX12Interceptor.h"
#include "../../DX12LayerManager.h"
#include "../../Util/DX12CoreDeepCopy.h"
#include "../../../Common/IUnknownWrapperGUID.h"
#include <pix_win.h>

class Wrapped_ID3D12Device;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealCommandQueue The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12CommandQueue(Wrapped_ID3D12Device* inParentDevice, ID3D12CommandQueue** inRealCommandQueue, Wrapped_ID3D12CommandQueueCreateInfo* inCreateInfo)
{
    return GenericWrapObject<ID3D12CommandQueue, Wrapped_ID3D12CommandQueueCustom>(inParentDevice, inRealCommandQueue, kObjectType_ID3D12CommandQueue, inCreateInfo);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::QueryInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::QueryInterface(REFIID riid, void** ppvObject)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    if (riid == IID_IWrappedObject)
    {
        *ppvObject = mRealCommandQueue;
        result = S_OK;
    }
    else
    {
        DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

        if (interceptor && interceptor->ShouldCollectTrace())
        {
            ParameterEntry parameters[] =
            {
                { PARAMETER_REFIID, &riid },
                { PARAMETER_POINTER, ppvObject },
            };

            int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
            DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_QueryInterface, numParameters, parameters);
            result = mRealCommandQueue->QueryInterface(riid, ppvObject);
            interceptor->PostCall(pNewEntry, result);
        }
        else
        {
            result = mRealCommandQueue->QueryInterface(riid, ppvObject);
        }

        if (result == S_OK)
        {
            if (riid == __uuidof(ID3D12CommandQueue))
            {
                WrapD3D12CommandQueue(nullptr, (ID3D12CommandQueue**)ppvObject);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::AddRef
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::AddRef()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_AddRef, 0, nullptr);
        result = mRealCommandQueue->AddRef();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->AddRef();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::Release
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::Release()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_Release, 0, nullptr);
        result = mRealCommandQueue->Release();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->Release();
    }

    if (result == 0)
    {
        DX12WrappedObjectDatabase* objectDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();
        IDX12InstanceBase* objectMetadata = objectDatabase->GetMetadataObject(this);

        if (objectMetadata != nullptr)
        {
            objectMetadata->FlagAsDestroyed();
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::GetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_GUID, &guid },
            { PARAMETER_POINTER, pDataSize },
            { PARAMETER_POINTER, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_GetPrivateData, numParameters, parameters);
        result = mRealCommandQueue->GetPrivateData(guid, pDataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->GetPrivateData(guid, pDataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::SetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_GUID, &guid },
            { PARAMETER_UNSIGNED_INT, &DataSize },
            { PARAMETER_POINTER, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetPrivateData, numParameters, parameters);
        result = mRealCommandQueue->SetPrivateData(guid, DataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->SetPrivateData(guid, DataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::SetPrivateDataInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_GUID, &guid },
            { PARAMETER_POINTER_SPECIAL, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetPrivateDataInterface, numParameters, parameters);
        result = mRealCommandQueue->SetPrivateDataInterface(guid, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->SetPrivateDataInterface(guid, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::SetName
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::SetName(LPCWSTR Name)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_WIDE_STRING, Name },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetName, numParameters, parameters);
        result = mRealCommandQueue->SetName(Name);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->SetName(Name);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::GetDevice
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::GetDevice(REFIID riid, void** ppvDevice)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_REFIID, &riid },
            { PARAMETER_POINTER, ppvDevice },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12DeviceChild_GetDevice, numParameters, parameters);
        result = mRealCommandQueue->GetDevice(riid, ppvDevice);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->GetDevice(riid, ppvDevice);
    }

    if (ppvDevice != nullptr && *ppvDevice != nullptr)
    {
        DX12WrappedObjectDatabase* objectDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();
        objectDatabase->WrappedObject((IUnknown**)ppvDevice);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::UpdateTileMappings
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::UpdateTileMappings(ID3D12Resource* pResource, UINT NumResourceRegions, const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates, const D3D12_TILE_REGION_SIZE* pResourceRegionSizes, ID3D12Heap* pHeap, UINT NumRanges, const D3D12_TILE_RANGE_FLAGS* pRangeFlags, const UINT* pHeapRangeStartOffsets, const UINT* pRangeTileCounts, D3D12_TILE_MAPPING_FLAGS Flags)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));
    ID3D12Heap* pHeapUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pHeap, &(pHeapUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        // print variables with nullptr pointer check
        UINT heapRangeStartOffsets = 0;

        if (pHeapRangeStartOffsets != nullptr)
        {
            heapRangeStartOffsets = *pHeapRangeStartOffsets;
        }

        UINT rangeTileCounts = 0;

        if (pRangeTileCounts != nullptr)
        {
            rangeTileCounts = *pRangeTileCounts;
        }

        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_UNSIGNED_INT, &NumResourceRegions },
            { PARAMETER_POINTER, pResourceRegionStartCoordinates },
            { PARAMETER_POINTER, pResourceRegionSizes },
            { PARAMETER_POINTER_SPECIAL, pHeap },
            { PARAMETER_UNSIGNED_INT, &NumRanges },
            { PARAMETER_POINTER, pRangeFlags },
            { PARAMETER_UNSIGNED_INT, &heapRangeStartOffsets },
            { PARAMETER_UNSIGNED_INT, &rangeTileCounts },
            { PARAMETER_UNSIGNED_INT, &Flags },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_UpdateTileMappings, numParameters, parameters);
        mRealCommandQueue->UpdateTileMappings(pResourceUnwrapped, NumResourceRegions, pResourceRegionStartCoordinates, pResourceRegionSizes, pHeapUnwrapped, NumRanges, pRangeFlags, pHeapRangeStartOffsets, pRangeTileCounts, Flags);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealCommandQueue->UpdateTileMappings(pResourceUnwrapped, NumResourceRegions, pResourceRegionStartCoordinates, pResourceRegionSizes, pHeapUnwrapped, NumRanges, pRangeFlags, pHeapRangeStartOffsets, pRangeTileCounts, Flags);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::CopyTileMappings
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::CopyTileMappings(ID3D12Resource* pDstResource, const D3D12_TILED_RESOURCE_COORDINATE* pDstRegionStartCoordinate, ID3D12Resource* pSrcResource, const D3D12_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* pRegionSize, D3D12_TILE_MAPPING_FLAGS Flags)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12Resource* pDstResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pDstResource, &(pDstResourceUnwrapped));
    ID3D12Resource* pSrcResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pSrcResource, &(pSrcResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pDstResource },
            { PARAMETER_POINTER, pDstRegionStartCoordinate },
            { PARAMETER_POINTER_SPECIAL, pSrcResource },
            { PARAMETER_POINTER, pSrcRegionStartCoordinate },
            { PARAMETER_POINTER, pRegionSize },
            { PARAMETER_UNSIGNED_INT, &Flags },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_CopyTileMappings, numParameters, parameters);
        mRealCommandQueue->CopyTileMappings(pDstResourceUnwrapped, pDstRegionStartCoordinate, pSrcResourceUnwrapped, pSrcRegionStartCoordinate, pRegionSize, Flags);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealCommandQueue->CopyTileMappings(pDstResourceUnwrapped, pDstRegionStartCoordinate, pSrcResourceUnwrapped, pSrcRegionStartCoordinate, pRegionSize, Flags);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::ExecuteCommandLists
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12CommandList** ppCommandListsCopy = nullptr;

    if (NumCommandLists > 0)
    {
        ppCommandListsCopy = new ID3D12CommandList*[NumCommandLists];

        for (UINT index = 0; index < NumCommandLists; index++)
        {
            DX12CoreDeepCopy::UnwrapInterface(ppCommandLists[index], &(ppCommandListsCopy[index]));
        }
    }

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        int numParameters = NumCommandLists + 1;
        ParameterEntry* parameters = new ParameterEntry[numParameters];
        parameters[0].mType = PARAMETER_UNSIGNED_INT;
        parameters[0].mData = &NumCommandLists;

        for (UINT loop = 0; loop < NumCommandLists; loop++)
        {
            parameters[loop + 1].mType = PARAMETER_POINTER;
            parameters[loop + 1].mData = ppCommandLists[loop];
        }

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_ExecuteCommandLists, numParameters, parameters);
        mRealCommandQueue->ExecuteCommandLists(NumCommandLists, ppCommandListsCopy);
        interceptor->PostCall(pNewEntry);
        delete[] parameters;
    }
    else
    {
        mRealCommandQueue->ExecuteCommandLists(NumCommandLists, ppCommandListsCopy);
    }

    SAFE_DELETE_ARRAY(ppCommandListsCopy);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::SetMarker
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::SetMarker(UINT Metadata, const void* pData, UINT Size)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &Metadata },
            { PARAMETER_STRING, pData },
            { PARAMETER_UNSIGNED_INT, &Size },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));

        if (Metadata == DirectX::Detail::PIX_EVENT_UNICODE_VERSION)
        {
            parameters[1].mType = PARAMETER_WIDE_STRING;
        }
        else if (Metadata != DirectX::Detail::PIX_EVENT_ANSI_VERSION)
        {
            numParameters = 0;
        }

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_SetMarker, numParameters, parameters);
        mRealCommandQueue->SetMarker(Metadata, pData, Size);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealCommandQueue->SetMarker(Metadata, pData, Size);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::BeginEvent
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::BeginEvent(UINT Metadata, const void* pData, UINT Size)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &Metadata },
            { PARAMETER_STRING, pData },
            { PARAMETER_UNSIGNED_INT, &Size },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));

        if (Metadata == DirectX::Detail::PIX_EVENT_UNICODE_VERSION)
        {
            parameters[1].mType = PARAMETER_WIDE_STRING;
        }
        else if (Metadata != DirectX::Detail::PIX_EVENT_ANSI_VERSION)
        {
            numParameters = 0;
        }

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_BeginEvent, numParameters, parameters);
        mRealCommandQueue->BeginEvent(Metadata, pData, Size);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealCommandQueue->BeginEvent(Metadata, pData, Size);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::EndEvent
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::EndEvent()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_EndEvent, 0);
        mRealCommandQueue->EndEvent();
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealCommandQueue->EndEvent();
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::Signal
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::Signal(ID3D12Fence* pFence, UINT64 Value)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    ID3D12Fence* pFenceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pFence, &(pFenceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pFence },
            { PARAMETER_UINT64, &Value },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_Signal, numParameters, parameters);
        result = mRealCommandQueue->Signal(pFenceUnwrapped, Value);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->Signal(pFenceUnwrapped, Value);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::Wait
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::Wait(ID3D12Fence* pFence, UINT64 Value)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};
    ID3D12Fence* pFenceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pFence, &(pFenceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pFence },
            { PARAMETER_UINT64, &Value },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_Wait, numParameters, parameters);
        result = mRealCommandQueue->Wait(pFenceUnwrapped, Value);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->Wait(pFenceUnwrapped, Value);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::GetTimestampFrequency
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::GetTimestampFrequency(UINT64* pFrequency)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pFrequency },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_GetTimestampFrequency, numParameters, parameters);
        result = mRealCommandQueue->GetTimestampFrequency(pFrequency);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->GetTimestampFrequency(pFrequency);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::GetClockCalibration
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::GetClockCalibration(UINT64* pGpuTimestamp, UINT64* pCpuTimestamp)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pGpuTimestamp },
            { PARAMETER_POINTER, pCpuTimestamp },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_GetClockCalibration, numParameters, parameters);
        result = mRealCommandQueue->GetClockCalibration(pGpuTimestamp, pCpuTimestamp);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealCommandQueue->GetClockCalibration(pGpuTimestamp, pCpuTimestamp);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12CommandQueue::GetDesc
//-----------------------------------------------------------------------------
D3D12_COMMAND_QUEUE_DESC STDMETHODCALLTYPE Wrapped_ID3D12CommandQueue::GetDesc()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    D3D12_COMMAND_QUEUE_DESC result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandQueue_GetDesc, 0);
        result = mRealCommandQueue->GetDesc();
        // TODO: get return value
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        result = mRealCommandQueue->GetDesc();
    }

    return result;
}