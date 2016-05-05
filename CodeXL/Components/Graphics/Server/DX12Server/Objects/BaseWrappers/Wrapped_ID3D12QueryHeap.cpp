//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12QueryHeap.cpp
/// \brief A class used to wrap D3D12's ID3D12QueryHeap interface.
//=============================================================================

#include "Wrapped_ID3D12QueryHeap.h"
#include "../DX12CreateInfoStructs.h"
#include "../DX12ObjectDatabaseProcessor.h"
#include "../../Interception/DX12Interceptor.h"
#include "../../DX12LayerManager.h"
#include "../../../Common/IUnknownWrapperGUID.h"

class Wrapped_ID3D12Device;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealQueryHeap The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12QueryHeap(Wrapped_ID3D12Device* inParentDevice, ID3D12QueryHeap** inRealQueryHeap, Wrapped_ID3D12QueryHeapCreateInfo* inCreateInfo)
{
    return GenericWrapObject<ID3D12QueryHeap, Wrapped_ID3D12QueryHeap>(inParentDevice, inRealQueryHeap, kObjectType_ID3D12QueryHeap, inCreateInfo);
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::QueryInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::QueryInterface(REFIID riid, void** ppvObject)
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    HRESULT result = {};

    if (riid == IID_IWrappedObject)
    {
        *ppvObject = mRealQueryHeap;
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
            result = mRealQueryHeap->QueryInterface(riid, ppvObject);
            interceptor->PostCall(pNewEntry, result);
        }
        else
        {
            result = mRealQueryHeap->QueryInterface(riid, ppvObject);
        }

        if (result == S_OK)
        {
            if (riid == __uuidof(ID3D12QueryHeap))
            {
                WrapD3D12QueryHeap(nullptr, (ID3D12QueryHeap**)ppvObject);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::AddRef
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::AddRef()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_AddRef, 0, nullptr);
        result = mRealQueryHeap->AddRef();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->AddRef();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::Release
//-----------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::Release()
{
#if SERIALIZE_DX12_ENTRY_POINTS
    ScopeLock lock(&s_mutex);
#endif

    ULONG result = {};

    DX12Interceptor* interceptor = GetDX12LayerManager()->GetInterceptor();

    if (interceptor && interceptor->ShouldCollectTrace())
    {
        DX12APIEntry* pNewEntry = interceptor->PreCall(this, FuncId_IUnknown_Release, 0, nullptr);
        result = mRealQueryHeap->Release();
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->Release();
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
/// Wrapped_ID3D12QueryHeap::GetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
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
        result = mRealQueryHeap->GetPrivateData(guid, pDataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->GetPrivateData(guid, pDataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::SetPrivateData
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
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
        result = mRealQueryHeap->SetPrivateData(guid, DataSize, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->SetPrivateData(guid, DataSize, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::SetPrivateDataInterface
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
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
        result = mRealQueryHeap->SetPrivateDataInterface(guid, pData);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->SetPrivateDataInterface(guid, pData);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::SetName
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::SetName(LPCWSTR Name)
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
        result = mRealQueryHeap->SetName(Name);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->SetName(Name);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Wrapped_ID3D12QueryHeap::GetDevice
//-----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE Wrapped_ID3D12QueryHeap::GetDevice(REFIID riid, void** ppvDevice)
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
        result = mRealQueryHeap->GetDevice(riid, ppvDevice);
        interceptor->PostCall(pNewEntry, result);
    }
    else
    {
        result = mRealQueryHeap->GetDevice(riid, ppvDevice);
    }

    if (ppvDevice != nullptr && *ppvDevice != nullptr)
    {
        DX12WrappedObjectDatabase* objectDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();
        objectDatabase->WrappedObject((IUnknown**)ppvDevice);
    }

    return result;
}