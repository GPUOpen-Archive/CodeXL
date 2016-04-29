//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12Fence.cpp
/// \brief A class used to wrap D3D12's ID3D12Fence interface.
//=============================================================================

#include "Wrapped_ID3D12Fence.h"
#include "../DX12CreateInfoStructs.h"
#include "../DX12ObjectDatabaseProcessor.h"
#include "../../Interception/DX12Interceptor.h"
#include "../../DX12LayerManager.h"
#include "../../../Common/IUnknownWrapperGUID.h"

class Wrapped_ID3D12Device;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealFence The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12Fence(Wrapped_ID3D12Device* inParentDevice, ID3D12Fence** inRealFence, Wrapped_ID3D12FenceCreateInfo* inCreateInfo)
{
    return GenericWrapObject<ID3D12Fence, Wrapped_ID3D12Fence>(inParentDevice, inRealFence, kObjectType_ID3D12Fence, inCreateInfo);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::QueryInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::QueryInterface(REFIID riid, void** ppvObject)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    if (riid == IID_IWrappedObject)
    {
        *ppvObject = mRealFence;
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
            result = mRealFence->QueryInterface(riid, ppvObject);
            interceptor->PostCall(pNewEntry, result);
        }
        else
        {
            result = mRealFence->QueryInterface(riid, ppvObject);
        }

        if (result == S_OK)
        {
            if (riid == __uuidof(ID3D12Fence))
            {
                WrapD3D12Fence(nullptr, (ID3D12Fence**)ppvObject);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::AddRef
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12Fence::AddRef()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_AddRef, 0, nullptr);
        result = mRealFence->AddRef();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->AddRef();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::Release
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12Fence::Release()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_Release, 0, nullptr);
        result = mRealFence->Release();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->Release();
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
/// Wrapped_ID3D12Fence::GetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
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
        result = mRealFence->GetPrivateData(guid, pDataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->GetPrivateData(guid, pDataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::SetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
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
        result = mRealFence->SetPrivateData(guid, DataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->SetPrivateData(guid, DataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::SetPrivateDataInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
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
        result = mRealFence->SetPrivateDataInterface(guid, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->SetPrivateDataInterface(guid, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::SetName
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::SetName(LPCWSTR Name)
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
        result = mRealFence->SetName(Name);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->SetName(Name);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::GetDevice
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::GetDevice(REFIID riid, void** ppvDevice)
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
        result = mRealFence->GetDevice(riid, ppvDevice);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->GetDevice(riid, ppvDevice);
    }

    if (ppvDevice != nullptr && *ppvDevice != nullptr)
    {
        DX12WrappedObjectDatabase* objectDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();
        objectDatabase->WrappedObject((IUnknown**)ppvDevice);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::GetCompletedValue
//-----------------------------------------------------------------------------
UINT64 STDMETHODCALLTYPE Wrapped_ID3D12Fence::GetCompletedValue()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    UINT64 result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Fence_GetCompletedValue, 0, nullptr);
        result = mRealFence->GetCompletedValue();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->GetCompletedValue();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::SetEventOnCompletion
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::SetEventOnCompletion(UINT64 Value, HANDLE hEvent)
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
            { PARAMETER_UINT64, &Value },
            { PARAMETER_POINTER, hEvent },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Fence_SetEventOnCompletion, numParameters, parameters);
        result = mRealFence->SetEventOnCompletion(Value, hEvent);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->SetEventOnCompletion(Value, hEvent);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12Fence::Signal
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12Fence::Signal(UINT64 Value)
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
            { PARAMETER_UINT64, &Value },
        };

        int numParameters = (sizeof(parameters) / sizeof(parameters[0]));
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_ID3D12Fence_Signal, numParameters, parameters);
        result = mRealFence->Signal(Value);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealFence->Signal(Value);
    }

    return result;
}