//==============================================================================
// Copyright (c) 2014-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Class supports a databse of wrapped object
//==============================================================================

#ifndef WRAPPEDOBJECTDATABASE_H
#define WRAPPEDOBJECTDATABASE_H

#include <map>
#include "../Common/misc.h" // For mutex

/// A macro
//#define CASE(res, enumCase) case enumCase: res = #enumCase; break;

enum eObjectType : int;
    class IInstanceBase;

    //--------------------------------------------------------------------------
    /// A type of vector used to hold wrapped object instances.
    //--------------------------------------------------------------------------
    typedef std::vector<IInstanceBase*> WrappedInstanceVector;

    //////////////////////////////////////////////////////////////////////////
    /// Database of all DX12 objects created and wrapped.
    class WrappedObjectDatabase
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor initializes internal database state.
    //--------------------------------------------------------------------------
    WrappedObjectDatabase();

    //--------------------------------------------------------------------------
    /// Destructor is responsible for cleaning up the database before destruction.
    //--------------------------------------------------------------------------
    virtual ~WrappedObjectDatabase();

    //--------------------------------------------------------------------------
    /// Retrieve a vector of objects with a given type.
    /// \param inObjectType The type of wrapped object to retrieve a vector of.
    /// \param outObjectInstancesOfGivenType The vector of all objects found to have the given type.
    /// \param inbOnlyCurrentObjects Flag to control if only current objects are to be returned.
    //--------------------------------------------------------------------------
    virtual void GetObjectsByType(eObjectType inObjectType, WrappedInstanceVector& outObjectInstancesOfGivenType, bool inbOnlyCurrentObjects = false) const = 0;

    //--------------------------------------------------------------------------
    /// Retrieve an pointer to a wrapped instance of a Mantle object created in the application.
    /// \param inInstanceHandle A handle to a Mantle object that was created in the application.
    /// \returns A wrapped instance of the given Mantle object.
    //--------------------------------------------------------------------------
    virtual IInstanceBase* GetWrappedInstance(void* inInstanceHandle) const = 0;

    //--------------------------------------------------------------------------
    /// A handler that's invoked when a device is destroyed. Responsible for cleaning up wrappers.
    /// \param inDeviceInstance A wrapper instance for the device being destroyed.
    //--------------------------------------------------------------------------
    virtual void OnDeviceDestroyed(IInstanceBase* inDeviceInstance) = 0;

    //--------------------------------------------------------------------------
    /// Store the wrapped metadata instance for an object.
    /// \param inWrappedInstance A handle to the metadata object instance.
    //--------------------------------------------------------------------------
    virtual void StoreWrappedInstance(IInstanceBase* inWrappedInstance) { (void)inWrappedInstance; };

    //--------------------------------------------------------------------------
    /// Enable/disable object tracking.
    /// \param inbTrackingObjects A boolean indicating whether or not to wrap newly-created Mantle object instances.
    //--------------------------------------------------------------------------
    inline void TrackObjectLifetime(bool inbTrackingObjects) {mbTrackingInstances = inbTrackingObjects; }

protected:
    //--------------------------------------------------------------------------
    /// Lock the object map whenever we add or remove an object.
    //--------------------------------------------------------------------------
    mutable mutex mCurrentObjectsMapLock;

    //--------------------------------------------------------------------------
    /// A boolean member to determine if we're tracking object creation/destruction.
    //--------------------------------------------------------------------------
    bool mbTrackingInstances;

    //--------------------------------------------------------------------------
    /// Lock the vector containing all previously created+destroyed objects.
    //--------------------------------------------------------------------------
    mutable mutex mAllObjectsVectorLock;

    //--------------------------------------------------------------------------
    /// A vector containing all object instances that ever existed. This includes
    /// instances that use the same handle over the history of the application run.
    //--------------------------------------------------------------------------
    WrappedInstanceVector mAllObjectInstances;
};

#endif // WRAPPEDOBJECTDATABASE_H
