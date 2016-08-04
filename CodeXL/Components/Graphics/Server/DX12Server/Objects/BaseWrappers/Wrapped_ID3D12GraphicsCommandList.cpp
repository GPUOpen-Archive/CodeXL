//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12GraphicsCommandList.cpp
/// \brief A class used to wrap D3D12's ID3D12GraphicsCommandList interface.
//=============================================================================

#include "Wrapped_ID3D12GraphicsCommandList.h"
#include "../DX12CreateInfoStructs.h"
#include "../DX12ObjectDatabaseProcessor.h"
#include "../../Interception/DX12Interceptor.h"
#include "../../DX12LayerManager.h"
#include "../../../Common/IUnknownWrapperGUID.h"
#include "../../Util/DX12CoreDeepCopy.h"
#include <pix_win.h>

class Wrapped_ID3D12Device;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealGraphicsCommandList The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12GraphicsCommandList(Wrapped_ID3D12Device* inParentDevice, ID3D12GraphicsCommandList** inRealGraphicsCommandList, Wrapped_ID3D12CommandListCreateInfo* inCreateInfo)
{
    return GenericWrapObject<ID3D12GraphicsCommandList, Wrapped_ID3D12GraphicsCommandListCustom>(inParentDevice, inRealGraphicsCommandList, kObjectType_ID3D12GraphicsCommandList, inCreateInfo);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::QueryInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::QueryInterface(REFIID riid, void** ppvObject)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_IUnknown_QueryInterface);

    HRESULT result = {};

    if (riid == IID_IWrappedObject)
    {
        *ppvObject = mRealGraphicsCommandList;
        result = S_OK;
    }
    else
    {
        DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

        if (interceptor->ShouldCollectTrace())
        {
            ParameterEntry parameters[] =
            {
                { PARAMETER_DX12_REFIID, &riid },
                { PARAMETER_POINTER, ppvObject },
            };

            int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
            DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_QueryInterface, numParameters, parameters);
            result = mRealGraphicsCommandList->QueryInterface(riid, ppvObject);
            interceptor->PostCall(pNewEntry, result);
        }
        else
        {
            result = mRealGraphicsCommandList->QueryInterface(riid, ppvObject);
        }

        if (result == S_OK)
        {
            if (riid == __uuidof(ID3D12GraphicsCommandList))
            {
                WrapD3D12GraphicsCommandList(nullptr, (ID3D12GraphicsCommandList**)ppvObject);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::AddRef
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::AddRef()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_IUnknown_AddRef);

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_AddRef, 0, nullptr);
        result = mRealGraphicsCommandList->AddRef();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->AddRef();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::Release
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::Release()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_IUnknown_Release);

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_Release, 0, nullptr);
        result = mRealGraphicsCommandList->Release();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->Release();
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
/// Wrapped_ID3D12GraphicsCommandList::GetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
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
            { PARAMETER_DX12_GUID, &guid },
            { PARAMETER_POINTER, pDataSize },
            { PARAMETER_POINTER, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_GetPrivateData, numParameters, parameters);
        result = mRealGraphicsCommandList->GetPrivateData(guid, pDataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->GetPrivateData(guid, pDataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12Object_SetPrivateData);

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_DX12_GUID, &guid },
            { PARAMETER_UNSIGNED_INT, &DataSize },
            { PARAMETER_POINTER, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetPrivateData, numParameters, parameters);
        result = mRealGraphicsCommandList->SetPrivateData(guid, DataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->SetPrivateData(guid, DataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetPrivateDataInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12Object_SetPrivateDataInterface);

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_DX12_GUID, &guid },
            { PARAMETER_POINTER_SPECIAL, pData },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Object_SetPrivateDataInterface, numParameters, parameters);
        result = mRealGraphicsCommandList->SetPrivateDataInterface(guid, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->SetPrivateDataInterface(guid, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetName
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetName(LPCWSTR Name)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12Object_SetName);

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
        result = mRealGraphicsCommandList->SetName(Name);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->SetName(Name);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::GetDevice
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::GetDevice(REFIID riid, void** ppvDevice)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12DeviceChild_GetDevice);

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_DX12_REFIID, &riid },
            { PARAMETER_POINTER, ppvDevice },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12DeviceChild_GetDevice, numParameters, parameters);
        result = mRealGraphicsCommandList->GetDevice(riid, ppvDevice);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->GetDevice(riid, ppvDevice);
    }

    if (ppvDevice != nullptr && *ppvDevice != nullptr)
    {
        DX12WrappedObjectDatabase* objectDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();
        objectDatabase->WrappedObject((IUnknown**)ppvDevice);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::GetType
//-----------------------------------------------------------------------------
D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::GetType()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12CommandList_GetType);

    D3D12_COMMAND_LIST_TYPE result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12CommandList_GetType, 0, nullptr);
        result = mRealGraphicsCommandList->GetType();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->GetType();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::Close
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::Close()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_Close);

    HRESULT result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_Close, 0, nullptr);
        result = mRealGraphicsCommandList->Close();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->Close();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::Reset
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::Reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_Reset);

    HRESULT result = {};
    ID3D12CommandAllocator* pAllocatorUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pAllocator, &(pAllocatorUnwrapped));
    ID3D12PipelineState* pInitialStateUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pInitialState, &(pInitialStateUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pAllocator },
            { PARAMETER_POINTER_SPECIAL, pInitialState },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_Reset, numParameters, parameters);
        result = mRealGraphicsCommandList->Reset(pAllocatorUnwrapped, pInitialStateUnwrapped);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealGraphicsCommandList->Reset(pAllocatorUnwrapped, pInitialStateUnwrapped);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ClearState
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ClearState(ID3D12PipelineState* pPipelineState)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ClearState);

    ID3D12PipelineState* pPipelineStateUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pPipelineState, &(pPipelineStateUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pPipelineState },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ClearState, numParameters, parameters);
        mRealGraphicsCommandList->ClearState(pPipelineStateUnwrapped);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ClearState(pPipelineStateUnwrapped);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::DrawInstanced
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_DrawInstanced);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &VertexCountPerInstance },
            { PARAMETER_UNSIGNED_INT, &InstanceCount },
            { PARAMETER_UNSIGNED_INT, &StartVertexLocation },
            { PARAMETER_UNSIGNED_INT, &StartInstanceLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_DrawInstanced, numParameters, parameters);
        mRealGraphicsCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::DrawIndexedInstanced
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &IndexCountPerInstance },
            { PARAMETER_UNSIGNED_INT, &InstanceCount },
            { PARAMETER_UNSIGNED_INT, &StartIndexLocation },
            { PARAMETER_INT,          &BaseVertexLocation },
            { PARAMETER_UNSIGNED_INT, &StartInstanceLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced, numParameters, parameters);
        mRealGraphicsCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::Dispatch
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_Dispatch);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &ThreadGroupCountX },
            { PARAMETER_UNSIGNED_INT, &ThreadGroupCountY },
            { PARAMETER_UNSIGNED_INT, &ThreadGroupCountZ },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_Dispatch, numParameters, parameters);
        mRealGraphicsCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::CopyBufferRegion
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::CopyBufferRegion(ID3D12Resource* pDstBuffer, UINT64 DstOffset, ID3D12Resource* pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_CopyBufferRegion);

    ID3D12Resource* pDstBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pDstBuffer, &(pDstBufferUnwrapped));
    ID3D12Resource* pSrcBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pSrcBuffer, &(pSrcBufferUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pDstBuffer },
            { PARAMETER_UINT64, &DstOffset },
            { PARAMETER_POINTER_SPECIAL, pSrcBuffer },
            { PARAMETER_UINT64, &SrcOffset },
            { PARAMETER_UINT64, &NumBytes },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_CopyBufferRegion, numParameters, parameters);
        mRealGraphicsCommandList->CopyBufferRegion(pDstBufferUnwrapped, DstOffset, pSrcBufferUnwrapped, SrcOffset, NumBytes);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->CopyBufferRegion(pDstBufferUnwrapped, DstOffset, pSrcBufferUnwrapped, SrcOffset, NumBytes);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::CopyTextureRegion
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION* pSrc, const D3D12_BOX* pSrcBox)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_CopyTextureRegion);

    D3D12_TEXTURE_COPY_LOCATION pDstUnwrapped;
    DX12CoreDeepCopy::DeepCopy(pDst, &pDstUnwrapped);
    D3D12_TEXTURE_COPY_LOCATION pSrcUnwrapped;
    DX12CoreDeepCopy::DeepCopy(pSrc, &pSrcUnwrapped);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pDst },
            { PARAMETER_UNSIGNED_INT, &DstX },
            { PARAMETER_UNSIGNED_INT, &DstY },
            { PARAMETER_UNSIGNED_INT, &DstZ },
            { PARAMETER_POINTER, pSrc },
            { PARAMETER_POINTER, pSrcBox },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_CopyTextureRegion, numParameters, parameters);
        mRealGraphicsCommandList->CopyTextureRegion(&pDstUnwrapped, DstX, DstY, DstZ, &pSrcUnwrapped, pSrcBox);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->CopyTextureRegion(&pDstUnwrapped, DstX, DstY, DstZ, &pSrcUnwrapped, pSrcBox);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::CopyResource
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::CopyResource(ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_CopyResource);

    ID3D12Resource* pDstResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pDstResource, &(pDstResourceUnwrapped));
    ID3D12Resource* pSrcResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pSrcResource, &(pSrcResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pDstResource },
            { PARAMETER_POINTER_SPECIAL, pSrcResource },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_CopyResource, numParameters, parameters);
        mRealGraphicsCommandList->CopyResource(pDstResourceUnwrapped, pSrcResourceUnwrapped);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->CopyResource(pDstResourceUnwrapped, pSrcResourceUnwrapped);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::CopyTiles
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::CopyTiles(ID3D12Resource* pTiledResource, const D3D12_TILED_RESOURCE_COORDINATE* pTileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* pTileRegionSize, ID3D12Resource* pBuffer, UINT64 BufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS Flags)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_CopyTiles);

    ID3D12Resource* pTiledResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pTiledResource, &(pTiledResourceUnwrapped));
    ID3D12Resource* pBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pBuffer, &(pBufferUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pTiledResource },
            { PARAMETER_POINTER, pTileRegionStartCoordinate },
            { PARAMETER_POINTER, pTileRegionSize },
            { PARAMETER_POINTER_SPECIAL, pBuffer },
            { PARAMETER_UINT64, &BufferStartOffsetInBytes },
            { PARAMETER_UNSIGNED_INT, &Flags },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_CopyTiles, numParameters, parameters);
        mRealGraphicsCommandList->CopyTiles(pTiledResourceUnwrapped, pTileRegionStartCoordinate, pTileRegionSize, pBufferUnwrapped, BufferStartOffsetInBytes, Flags);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->CopyTiles(pTiledResourceUnwrapped, pTileRegionStartCoordinate, pTileRegionSize, pBufferUnwrapped, BufferStartOffsetInBytes, Flags);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ResolveSubresource
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ResolveSubresource(ID3D12Resource* pDstResource, UINT DstSubresource, ID3D12Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ResolveSubresource);

    ID3D12Resource* pDstResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pDstResource, &(pDstResourceUnwrapped));
    ID3D12Resource* pSrcResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pSrcResource, &(pSrcResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pDstResource },
            { PARAMETER_UNSIGNED_INT, &DstSubresource },
            { PARAMETER_POINTER_SPECIAL, pSrcResource },
            { PARAMETER_UNSIGNED_INT, &SrcSubresource },
            { PARAMETER_DX12_DXGI_FORMAT, &Format },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ResolveSubresource, numParameters, parameters);
        mRealGraphicsCommandList->ResolveSubresource(pDstResourceUnwrapped, DstSubresource, pSrcResourceUnwrapped, SrcSubresource, Format);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ResolveSubresource(pDstResourceUnwrapped, DstSubresource, pSrcResourceUnwrapped, SrcSubresource, Format);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::IASetPrimitiveTopology
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_IASetPrimitiveTopology);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_DX12_PRIMITIVE_TOPOLOGY, &PrimitiveTopology },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_IASetPrimitiveTopology, numParameters, parameters);
        mRealGraphicsCommandList->IASetPrimitiveTopology(PrimitiveTopology);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->IASetPrimitiveTopology(PrimitiveTopology);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::RSSetViewports
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::RSSetViewports(UINT NumViewports, const D3D12_VIEWPORT* pViewports)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_RSSetViewports);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &NumViewports },
            { PARAMETER_POINTER, pViewports }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_RSSetViewports, numParameters, parameters);
        mRealGraphicsCommandList->RSSetViewports(NumViewports, pViewports);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->RSSetViewports(NumViewports, pViewports);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::RSSetScissorRects
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::RSSetScissorRects(UINT NumRects, const D3D12_RECT* pRects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_RSSetScissorRects);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &NumRects },
            { PARAMETER_POINTER, pRects },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_RSSetScissorRects, numParameters, parameters);
        mRealGraphicsCommandList->RSSetScissorRects(NumRects, pRects);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->RSSetScissorRects(NumRects, pRects);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::OMSetBlendFactor
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::OMSetBlendFactor(const FLOAT BlendFactor[4])
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_OMSetBlendFactor);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        static const int arraySize = 4;
        ParameterEntry parameters[] =
        {
            { PARAMETER_ARRAY, &arraySize },
            { PARAMETER_FLOAT, &(BlendFactor[0]) },
            { PARAMETER_FLOAT, &(BlendFactor[1]) },
            { PARAMETER_FLOAT, &(BlendFactor[2]) },
            { PARAMETER_FLOAT, &(BlendFactor[3]) },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_OMSetBlendFactor, numParameters, parameters);
        mRealGraphicsCommandList->OMSetBlendFactor(BlendFactor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->OMSetBlendFactor(BlendFactor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::OMSetStencilRef
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::OMSetStencilRef(UINT StencilRef)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_OMSetStencilRef);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &StencilRef },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_OMSetStencilRef, numParameters, parameters);
        mRealGraphicsCommandList->OMSetStencilRef(StencilRef);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->OMSetStencilRef(StencilRef);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetPipelineState
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetPipelineState(ID3D12PipelineState* pPipelineState)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetPipelineState);

    ID3D12PipelineState* pPipelineStateUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pPipelineState, &(pPipelineStateUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pPipelineState },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetPipelineState, numParameters, parameters);
        mRealGraphicsCommandList->SetPipelineState(pPipelineStateUnwrapped);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetPipelineState(pPipelineStateUnwrapped);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ResourceBarrier
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ResourceBarrier);

    D3D12_RESOURCE_BARRIER* descs = nullptr;

    if (NumBarriers > 0)
    {
        descs = new D3D12_RESOURCE_BARRIER[NumBarriers];

        for (UINT index = 0; index < NumBarriers; index++)
        {
            DX12CoreDeepCopy::DeepCopy(&(pBarriers[index]), &(descs[index]));
        }
    }

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &NumBarriers },
            { PARAMETER_POINTER, pBarriers }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ResourceBarrier, numParameters, parameters);
        mRealGraphicsCommandList->ResourceBarrier(NumBarriers, descs);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ResourceBarrier(NumBarriers, descs);
    }

    SAFE_DELETE_ARRAY(descs);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ExecuteBundle
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ExecuteBundle(ID3D12GraphicsCommandList* pCommandList)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ExecuteBundle);

    ID3D12GraphicsCommandList* pCommandListUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pCommandList, &(pCommandListUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pCommandList },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ExecuteBundle, numParameters, parameters);
        mRealGraphicsCommandList->ExecuteBundle(pCommandListUnwrapped);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ExecuteBundle(pCommandListUnwrapped);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetDescriptorHeaps
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetDescriptorHeaps);

    ID3D12DescriptorHeap** pDescriptorHeapsUnwrapped = nullptr;

    if (NumDescriptorHeaps > 0)
    {
        pDescriptorHeapsUnwrapped = new ID3D12DescriptorHeap*[NumDescriptorHeaps];

        for (UINT index = 0; index < NumDescriptorHeaps; index++)
        {
            DX12CoreDeepCopy::UnwrapInterface(ppDescriptorHeaps[index], &(pDescriptorHeapsUnwrapped[index]));
        }
    }

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        int numParameters = NumDescriptorHeaps + 1;
        ParameterEntry* parameters = new ParameterEntry[numParameters];
        parameters[0].mType = PARAMETER_UNSIGNED_INT;
        parameters[0].mData = &NumDescriptorHeaps;

        for (UINT loop = 0; loop < NumDescriptorHeaps; loop++)
        {
            parameters[loop + 1].mType = PARAMETER_POINTER_SPECIAL;
            parameters[loop + 1].mData = ppDescriptorHeaps[loop];
        }

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetDescriptorHeaps, numParameters, parameters);
        mRealGraphicsCommandList->SetDescriptorHeaps(NumDescriptorHeaps, pDescriptorHeapsUnwrapped);
        interceptor->PostCall(pNewEntry);
        delete[] parameters;
    }
    else
    {
        mRealGraphicsCommandList->SetDescriptorHeaps(NumDescriptorHeaps, pDescriptorHeapsUnwrapped);
    }

    SAFE_DELETE_ARRAY(pDescriptorHeapsUnwrapped);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRootSignature
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRootSignature(ID3D12RootSignature* pRootSignature)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRootSignature);

    ID3D12RootSignature* pRootSignatureUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pRootSignature, &(pRootSignatureUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pRootSignature },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRootSignature, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRootSignature(pRootSignatureUnwrapped);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRootSignature(pRootSignatureUnwrapped);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootSignature
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRootSignature);

    ID3D12RootSignature* pRootSignatureUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pRootSignature, &(pRootSignatureUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pRootSignature }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRootSignature, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRootSignature(pRootSignatureUnwrapped);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRootSignature(pRootSignatureUnwrapped);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRootDescriptorTable);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_POINTER, (void*)BaseDescriptor.ptr },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRootDescriptorTable, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_POINTER, (void*)BaseDescriptor.ptr }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRoot32BitConstant
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstant);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UNSIGNED_INT, &SrcData },
            { PARAMETER_UNSIGNED_INT, &DestOffsetIn32BitValues },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstant, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstant
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UNSIGNED_INT, &SrcData },
            { PARAMETER_UNSIGNED_INT, &DestOffsetIn32BitValues },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRoot32BitConstants
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstants);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UNSIGNED_INT, &Num32BitValuesToSet },
            { PARAMETER_POINTER, pSrcData },
            { PARAMETER_UNSIGNED_INT, &DestOffsetIn32BitValues },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstants, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstants
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UNSIGNED_INT, &Num32BitValuesToSet },
            { PARAMETER_POINTER, pSrcData },
            { PARAMETER_UNSIGNED_INT, &DestOffsetIn32BitValues },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRootConstantBufferView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRootConstantBufferView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UINT64, &BufferLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRootConstantBufferView, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UINT64, &BufferLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRootShaderResourceView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRootShaderResourceView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UINT64, &BufferLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRootShaderResourceView, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootShaderResourceView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UINT64, &BufferLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetComputeRootUnorderedAccessView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetComputeRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UINT64, &BufferLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView, numParameters, parameters);
        mRealGraphicsCommandList->SetComputeRootUnorderedAccessView(RootParameterIndex, BufferLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetComputeRootUnorderedAccessView(RootParameterIndex, BufferLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootUnorderedAccessView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &RootParameterIndex },
            { PARAMETER_UINT64, &BufferLocation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView, numParameters, parameters);
        mRealGraphicsCommandList->SetGraphicsRootUnorderedAccessView(RootParameterIndex, BufferLocation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetGraphicsRootUnorderedAccessView(RootParameterIndex, BufferLocation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::IASetIndexBuffer
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_IASetIndexBuffer);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, pView }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_IASetIndexBuffer, numParameters, parameters);
        mRealGraphicsCommandList->IASetIndexBuffer(pView);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->IASetIndexBuffer(pView);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::IASetVertexBuffers
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_IASetVertexBuffers);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &StartSlot },
            { PARAMETER_UNSIGNED_INT, &NumViews },
            { PARAMETER_POINTER, pViews }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_IASetVertexBuffers, numParameters, parameters);
        mRealGraphicsCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SOSetTargets
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SOSetTargets(UINT StartSlot, UINT NumViews, const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SOSetTargets);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &StartSlot },
            { PARAMETER_UNSIGNED_INT, &NumViews },
            { PARAMETER_POINTER, pViews },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SOSetTargets, numParameters, parameters);
        mRealGraphicsCommandList->SOSetTargets(StartSlot, NumViews, pViews);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SOSetTargets(StartSlot, NumViews, pViews);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::OMSetRenderTargets
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::OMSetRenderTargets(UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_OMSetRenderTargets);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_UNSIGNED_INT, &NumRenderTargetDescriptors },
            { PARAMETER_POINTER, pRenderTargetDescriptors },
            { PARAMETER_BOOL, &RTsSingleHandleToDescriptorRange },
            { PARAMETER_POINTER, pDepthStencilDescriptor }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_OMSetRenderTargets, numParameters, parameters);
        mRealGraphicsCommandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ClearDepthStencilView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT* pRects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, (void*)DepthStencilView.ptr },
            { PARAMETER_UNSIGNED_INT, &ClearFlags },
            { PARAMETER_FLOAT, &Depth },
            { PARAMETER_UNSIGNED_CHAR, &Stencil },
            { PARAMETER_UNSIGNED_INT, &NumRects },
            { PARAMETER_POINTER, pRects }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView, numParameters, parameters);
        mRealGraphicsCommandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ClearRenderTargetView
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[4], UINT NumRects, const D3D12_RECT* pRects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        static const int arraySize = 4;
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, (void*)RenderTargetView.ptr },
            { PARAMETER_ARRAY, &arraySize },
            { PARAMETER_FLOAT, &(ColorRGBA[0]) },
            { PARAMETER_FLOAT, &(ColorRGBA[1]) },
            { PARAMETER_FLOAT, &(ColorRGBA[2]) },
            { PARAMETER_FLOAT, &(ColorRGBA[3]) },
            { PARAMETER_UNSIGNED_INT, &NumRects },
            { PARAMETER_POINTER, pRects }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ClearRenderTargetView, numParameters, parameters);
        mRealGraphicsCommandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ClearUnorderedAccessViewUint
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource* pResource, const UINT Values[4], UINT NumRects, const D3D12_RECT* pRects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint);

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        static const int arraySize = 4;
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, (void*)ViewGPUHandleInCurrentHeap.ptr },
            { PARAMETER_POINTER, (void*)ViewCPUHandle.ptr },
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_ARRAY, &arraySize },
            { PARAMETER_UNSIGNED_INT, &(Values[0]) },
            { PARAMETER_UNSIGNED_INT, &(Values[1]) },
            { PARAMETER_UNSIGNED_INT, &(Values[2]) },
            { PARAMETER_UNSIGNED_INT, &(Values[3]) },
            { PARAMETER_UNSIGNED_INT, &NumRects },
            { PARAMETER_POINTER, pRects }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint, numParameters, parameters);
        mRealGraphicsCommandList->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResourceUnwrapped, Values, NumRects, pRects);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResourceUnwrapped, Values, NumRects, pRects);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ClearUnorderedAccessViewFloat
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource* pResource, const FLOAT Values[4], UINT NumRects, const D3D12_RECT* pRects)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat);

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        static const int arraySize = 4;
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER, (void*)ViewGPUHandleInCurrentHeap.ptr },
            { PARAMETER_POINTER, (void*)ViewCPUHandle.ptr },
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_ARRAY, &arraySize },
            { PARAMETER_FLOAT, &(Values[0]) },
            { PARAMETER_FLOAT, &(Values[1]) },
            { PARAMETER_FLOAT, &(Values[2]) },
            { PARAMETER_FLOAT, &(Values[3]) },
            { PARAMETER_UNSIGNED_INT, &NumRects },
            { PARAMETER_POINTER, pRects }
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat, numParameters, parameters);
        mRealGraphicsCommandList->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResourceUnwrapped, Values, NumRects, pRects);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResourceUnwrapped, Values, NumRects, pRects);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::DiscardResource
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::DiscardResource(ID3D12Resource* pResource, const D3D12_DISCARD_REGION* pRegion)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_DiscardResource);

    ID3D12Resource* pResourceUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pResource, &(pResourceUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pResource },
            { PARAMETER_POINTER, pRegion },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_DiscardResource, numParameters, parameters);
        mRealGraphicsCommandList->DiscardResource(pResourceUnwrapped, pRegion);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->DiscardResource(pResourceUnwrapped, pRegion);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::BeginQuery
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::BeginQuery(ID3D12QueryHeap* pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ID3D12QueryHeap* pQueryHeapUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pQueryHeap, &(pQueryHeapUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pQueryHeap },
            { PARAMETER_DX12_QUERY_TYPE, &Type },
            { PARAMETER_UNSIGNED_INT, &Index },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_BeginQuery, numParameters, parameters);
        mRealGraphicsCommandList->BeginQuery(pQueryHeapUnwrapped, Type, Index);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->BeginQuery(pQueryHeapUnwrapped, Type, Index);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::EndQuery
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::EndQuery(ID3D12QueryHeap* pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_EndQuery);

    ID3D12QueryHeap* pQueryHeapUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pQueryHeap, &(pQueryHeapUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pQueryHeap },
            { PARAMETER_DX12_QUERY_TYPE, &Type },
            { PARAMETER_UNSIGNED_INT, &Index },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_EndQuery, numParameters, parameters);
        mRealGraphicsCommandList->EndQuery(pQueryHeapUnwrapped, Type, Index);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->EndQuery(pQueryHeapUnwrapped, Type, Index);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ResolveQueryData
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ResolveQueryData(ID3D12QueryHeap* pQueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource* pDestinationBuffer, UINT64 AlignedDestinationBufferOffset)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ResolveQueryData);

    ID3D12QueryHeap* pQueryHeapUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pQueryHeap, &(pQueryHeapUnwrapped));
    ID3D12Resource* pDestinationBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pDestinationBuffer, &(pDestinationBufferUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pQueryHeap },
            { PARAMETER_DX12_QUERY_TYPE, &Type },
            { PARAMETER_UNSIGNED_INT, &StartIndex },
            { PARAMETER_UNSIGNED_INT, &NumQueries },
            { PARAMETER_POINTER_SPECIAL, pDestinationBuffer },
            { PARAMETER_UINT64, &AlignedDestinationBufferOffset },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ResolveQueryData, numParameters, parameters);
        mRealGraphicsCommandList->ResolveQueryData(pQueryHeapUnwrapped, Type, StartIndex, NumQueries, pDestinationBufferUnwrapped, AlignedDestinationBufferOffset);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ResolveQueryData(pQueryHeapUnwrapped, Type, StartIndex, NumQueries, pDestinationBufferUnwrapped, AlignedDestinationBufferOffset);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetPredication
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetPredication(ID3D12Resource* pBuffer, UINT64 AlignedBufferOffset, D3D12_PREDICATION_OP Operation)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetPredication);

    ID3D12Resource* pBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pBuffer, &(pBufferUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pBuffer },
            { PARAMETER_UINT64, &AlignedBufferOffset },
            { PARAMETER_DX12_PREDICATION_OP, &Operation },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetPredication, numParameters, parameters);
        mRealGraphicsCommandList->SetPredication(pBufferUnwrapped, AlignedBufferOffset, Operation);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetPredication(pBufferUnwrapped, AlignedBufferOffset, Operation);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::SetMarker
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::SetMarker(UINT Metadata, const void* pData, UINT Size)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_SetMarker);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
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

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_SetMarker, numParameters, parameters);
        mRealGraphicsCommandList->SetMarker(Metadata, pData, Size);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->SetMarker(Metadata, pData, Size);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::BeginEvent
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::BeginEvent(UINT Metadata, const void* pData, UINT Size)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_BeginEvent);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
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

        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_BeginEvent, numParameters, parameters);
        mRealGraphicsCommandList->BeginEvent(Metadata, pData, Size);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->BeginEvent(Metadata, pData, Size);
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::EndEvent
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::EndEvent()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_EndEvent);

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_EndEvent, 0);
        mRealGraphicsCommandList->EndEvent();
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->EndEvent();
    }
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12GraphicsCommandList::ExecuteIndirect
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12GraphicsCommandList::ExecuteIndirect(ID3D12CommandSignature* pCommandSignature, UINT MaxCommandCount, ID3D12Resource* pArgumentBuffer, UINT64 ArgumentBufferOffset, ID3D12Resource* pCountBuffer, UINT64 CountBufferOffset)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    TrackCommandListCall(FuncId_ID3D12GraphicsCommandList_ExecuteIndirect);

    ID3D12CommandSignature* pCommandSignatureUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pCommandSignature, &(pCommandSignatureUnwrapped));
    ID3D12Resource* pArgumentBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pArgumentBuffer, &(pArgumentBufferUnwrapped));
    ID3D12Resource* pCountBufferUnwrapped;
    DX12CoreDeepCopy::UnwrapInterface(pCountBuffer, &(pCountBufferUnwrapped));

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        ParameterEntry parameters[] =
        {
            { PARAMETER_POINTER_SPECIAL, pCommandSignature },
            { PARAMETER_UNSIGNED_INT, &MaxCommandCount },
            { PARAMETER_POINTER_SPECIAL, pArgumentBuffer },
            { PARAMETER_UINT64, &ArgumentBufferOffset },
            { PARAMETER_POINTER_SPECIAL, pCountBuffer },
            { PARAMETER_UINT64, &CountBufferOffset },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12GraphicsCommandList_ExecuteIndirect, numParameters, parameters);
        mRealGraphicsCommandList->ExecuteIndirect(pCommandSignatureUnwrapped, MaxCommandCount, pArgumentBufferUnwrapped, ArgumentBufferOffset, pCountBufferUnwrapped, CountBufferOffset);
        interceptor->PostCall(pNewEntry);
    }
    else
    {
        mRealGraphicsCommandList->ExecuteIndirect(pCommandSignatureUnwrapped, MaxCommandCount, pArgumentBufferUnwrapped, ArgumentBufferOffset, pCountBufferUnwrapped, CountBufferOffset);
    }
}