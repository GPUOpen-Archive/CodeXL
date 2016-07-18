//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12WrappedObjectDatabase.h
/// \brief  The DX12WrappedObjectDatabase is responsible for tracking
///         wrapped and destroyed objects.
//=============================================================================

#ifndef DX12WRAPPEDOBJECTDATABASE_H
#define DX12WRAPPEDOBJECTDATABASE_H

#include <unordered_map>
#include "../../Common/WrappedObjectDatabase.h"

class IInstanceBase;
class IDX12InstanceBase;

/// A map used to associate an application's D3D12 interface with a IDXInstanceBase metadata object.
typedef std::unordered_map<IUnknown*, IDX12InstanceBase*> DXInterfaceToWrapperMetadata;

//-----------------------------------------------------------------------------
/// The DX12WrappedObjectDatabase is responsible for tracking wrapped and destroyed objects.
//-----------------------------------------------------------------------------
class DX12WrappedObjectDatabase : public WrappedObjectDatabase
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor initializes a new instance of the DX12WrappedObjectDatabase class.
    //-----------------------------------------------------------------------------
    DX12WrappedObjectDatabase() {}

    //-----------------------------------------------------------------------------
    /// Destructor destroys the DX12WrappedObjectDatabase instance.
    //-----------------------------------------------------------------------------
    virtual ~DX12WrappedObjectDatabase() {}

    //-----------------------------------------------------------------------------
    /// Retrieve a vector of objects with a given type.
    /// \param inObjectType The type of wrapped object to retrieve a vector of.
    /// \param outObjectInstancesOfGivenType The vector of all objects found to have the given type.
    /// \param inbOnlyActiveObjects A flag to specify if all instances should be returned, or only active instances.
    //-----------------------------------------------------------------------------
    virtual void GetObjectsByType(eObjectType inObjectType, WrappedInstanceVector& outObjectInstancesOfGivenType, bool inbOnlyActiveObjects = false) const;

    //-----------------------------------------------------------------------------
    /// Retrieve an pointer to a wrapped instance of a DX12 object created in the application.
    /// \param inInstanceHandle A handle to a Mantle object that was created in the application.
    /// \returns A wrapped instance of the given Mantle object.
    //-----------------------------------------------------------------------------
    virtual IInstanceBase* GetWrappedInstance(void* inInstanceHandle) const;

    //-----------------------------------------------------------------------------
    /// A handler that's invoked when a device is destroyed. Responsible for cleaning up wrappers.
    /// \param inDeviceInstance A wrapper instance for the device being destroyed.
    //-----------------------------------------------------------------------------
    virtual void OnDeviceDestroyed(IInstanceBase* inDeviceInstance);

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the wrapper instance when given a pointer to the real instance.
    /// Note: When true, ioInstance is reassigned to the wrapper instance.
    /// \param ioInstance A pointer to the real ID3D12 interface instance.
    /// \returns True if the object is already in the DB, false if it's not.
    //-----------------------------------------------------------------------------
    bool WrappedObject(IUnknown** inRealInstance) const;

    //-----------------------------------------------------------------------------
    /// Add a wrapper metadata object to the object database.
    /// \param inWrapperMetadata A pointer to the IDX12InstanceBase wrapper metadata instance.
    //-----------------------------------------------------------------------------
    void Add(IDX12InstanceBase* inWrapperMetadata);

    //-----------------------------------------------------------------------------
    /// Retrieve the IDX12InstanceBase metadata object corresponding to the given input wrapper.
    /// \param inWrapperInstance A wrapped D3D12 interface.
    /// \returns The IDX12InstanceBase metadata object associated with the given wrapper, or nullptr if it doesn't exist.
    //-----------------------------------------------------------------------------
    IDX12InstanceBase* GetMetadataObject(IUnknown* inWrapperInstance);

private:
    /// Maintain a map containing the object database. The keys are DX12 interface pointers, with values set to their corresponding wrapper instance.
    DXInterfaceToWrapperMetadata mRealInstanceToWrapperMetadata;

    /// Maintain an std::map wrapper database. The keys are wrapper class instances, and the values are their corresponding DX12 wrapped interfaces.
    DXInterfaceToWrapperMetadata mWrapperInstanceToWrapperMetadata;
};

#endif // DX12WRAPPEDOBJECTDATABASE_H
