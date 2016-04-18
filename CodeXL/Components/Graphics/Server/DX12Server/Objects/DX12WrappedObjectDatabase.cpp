//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12WrappedObjectDatabase.cpp
/// \brief  The DX12WrappedObjectDatabase is responsible for tracking
///         wrapped and destroyed objects.
//=============================================================================

#include "DX12WrappedObjectDatabase.h"
#include "../../Common/IInstanceBase.h"
#include "IDX12InstanceBase.h"
#include "DX12ObjectDatabaseProcessor.h"

//-----------------------------------------------------------------------------
/// Retrieve a vector of objects with a given type.
/// \param inObjectType The type of wrapped object to retrieve a vector of.
/// \param outObjectInstancesOfGivenType The vector of all objects found to have the given type.
/// \param inbOnlyActiveObjects A flag to specify if all instances should be returned, or only active instances.
//-----------------------------------------------------------------------------
void DX12WrappedObjectDatabase::GetObjectsByType(eObjectType inObjectType, WrappedInstanceVector& outObjectInstancesOfGivenType, bool inbOnlyActiveObjects) const
{
    // Loop through everything in the object database and look for specific types of objects.
    DXInterfaceToWrapperMetadata::const_iterator wrapperIter;
    DXInterfaceToWrapperMetadata::const_iterator endIter = mWrapperInstanceToWrapperMetadata.end();

    for (wrapperIter = mWrapperInstanceToWrapperMetadata.begin(); wrapperIter != endIter; ++wrapperIter)
    {
        IDX12InstanceBase* wrapperMetadata = wrapperIter->second;

        if (wrapperMetadata->GetObjectType() == inObjectType)
        {
            // If we only care about currently-active objects, don't include it if it has already been destroyed.
            if (inbOnlyActiveObjects && wrapperMetadata->IsDestroyed())
            {
                continue;
            }

            outObjectInstancesOfGivenType.push_back(wrapperMetadata);
        }
    }
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the wrapper instance of the given ID3D12 object.
/// \param inInstanceHandle A handle to the ID3D12 interface instance.
/// \returns A IInstanceBase pointer, which is the wrapper for the given input instance.
//-----------------------------------------------------------------------------
IInstanceBase* DX12WrappedObjectDatabase::GetWrappedInstance(void* inInstanceHandle) const
{
    // Look in the wrapped object database for the incoming pointer.
    IUnknown* wrappedInstance = (IUnknown*)inInstanceHandle;

    DXInterfaceToWrapperMetadata::const_iterator interfaceIter = mRealInstanceToWrapperMetadata.find(wrappedInstance);
    DXInterfaceToWrapperMetadata::const_iterator endIter = mRealInstanceToWrapperMetadata.end();

    if (interfaceIter != endIter)
    {
        return interfaceIter->second;
    }
    else
    {
        // Is the incoming pointer already a wrapped pointer? If so, just return the metadata object.
        DXInterfaceToWrapperMetadata::const_iterator wrapperInterfaceIter = mWrapperInstanceToWrapperMetadata.find(wrappedInstance);
        DXInterfaceToWrapperMetadata::const_iterator wrappersEnditer = mWrapperInstanceToWrapperMetadata.end();

        if (wrapperInterfaceIter != wrappersEnditer)
        {
            return wrapperInterfaceIter->second;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
/// A handler that's invoked when a device is destroyed. Responsible for cleaning up wrappers.
/// \param inDeviceInstance A wrapper instance for the device being destroyed.
//-----------------------------------------------------------------------------
void DX12WrappedObjectDatabase::OnDeviceDestroyed(IInstanceBase* inDeviceInstance)
{
    // @DX12TODO: Clean up wrappers related to the destroyed object.
    (void)inDeviceInstance;
}

//-----------------------------------------------------------------------------
/// Add a wrapper metadata object to the object database.
/// \param inWrapperMetadata A pointer to the IDX12InstanceBase wrapper metadata instance.
//-----------------------------------------------------------------------------
void DX12WrappedObjectDatabase::Add(IDX12InstanceBase* inWrapperMetadata)
{
    ScopeLock mutex(&mCurrentObjectsMapLock);
    IUnknown* wrapperHandle = static_cast<IUnknown*>(inWrapperMetadata->GetApplicationHandle());
    IUnknown* runtimeInstance = inWrapperMetadata->GetRuntimeInstance();

    mWrapperInstanceToWrapperMetadata[wrapperHandle] = inWrapperMetadata;
    mRealInstanceToWrapperMetadata[runtimeInstance] = inWrapperMetadata;
}

//-----------------------------------------------------------------------------
/// Retrieve the IDX12InstanceBase metadata object corresponding to the given input wrapper.
/// \param inWrapperInstance A wrapped D3D12 interface.
/// \returns The IDX12InstanceBase metadata object associated with the given wrapper, or nullptr if it doesn't exist.
//-----------------------------------------------------------------------------
IDX12InstanceBase* DX12WrappedObjectDatabase::GetMetadataObject(IUnknown* inWrapperInstance)
{
    ScopeLock mutex(&mCurrentObjectsMapLock);
    IDX12InstanceBase* resultInstance = nullptr;

    DXInterfaceToWrapperMetadata::const_iterator objectIter = mWrapperInstanceToWrapperMetadata.find(inWrapperInstance);

    if (objectIter != mWrapperInstanceToWrapperMetadata.end())
    {
        resultInstance = objectIter->second;
    }

    return resultInstance;
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the wrapper instance when given a pointer to the real instance.
/// Note: When true, ioInstance is reassigned to the wrapper instance.
/// \param ioInstance A pointer to the real ID3D12 interface instance.
/// \returns True if the object is already in the DB, false if it's not.
//-----------------------------------------------------------------------------
bool DX12WrappedObjectDatabase::WrappedObject(IUnknown** ioInstance) const
{
    ScopeLock mutex(&mCurrentObjectsMapLock);

    IUnknown* pReal = *ioInstance;

    // Check if object has been wrapped
    DXInterfaceToWrapperMetadata::const_iterator objectIter = mRealInstanceToWrapperMetadata.find(pReal);

    if (objectIter == mRealInstanceToWrapperMetadata.end())
    {
        return false;
    }
    else
    {
        // The object already has a wrapper instance in the database. Return the Wrapped_ID3D12[Something] wrapper instance.
        IDX12InstanceBase* wrapperMetaData = static_cast<IDX12InstanceBase*>(objectIter->second);
        *ioInstance = (IUnknown*)wrapperMetaData->GetApplicationHandle();
        return true;
    }
}