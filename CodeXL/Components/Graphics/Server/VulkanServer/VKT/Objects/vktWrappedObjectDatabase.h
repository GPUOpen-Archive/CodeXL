//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedObjectDatabase.h
/// \brief  Header file for a Vulkan object database.
///         This class holds C++ wrappers for live Vulkan objects.
//==============================================================================

#ifndef __VKT_WRAPPED_OBJECT_DATABASE_H__
#define __VKT_WRAPPED_OBJECT_DATABASE_H__

class IInstanceBase;
class VktInstanceBase;

#include "../../../Common/WrappedObjectDatabase.h"
#include <unordered_map>

/// Associate an application object handle with a PerfStudio-wrapped instance.
typedef std::unordered_map<uint64, VktInstanceBase*> AppObjectToWrappedInstance;

/// A type of vector used to hold object instance handles.
typedef std::vector<uint64> AppObjectHandleVector;

//--------------------------------------------------------------------------
/// A specialized database to hold Vulkan object instances.
//--------------------------------------------------------------------------
class VktWrappedObjectDatabase : public WrappedObjectDatabase
{
public:
    VktWrappedObjectDatabase() {}
    virtual ~VktWrappedObjectDatabase() {}

    virtual void GetObjectsByType(eObjectType inObjectType, WrappedInstanceVector& outObjectInstancesOfGivenType, bool inbOnlyCurrentObjects = false) const;
    virtual IInstanceBase* GetWrappedInstance(void* inInstanceHandle) const;
    virtual void StoreWrappedInstance(IInstanceBase* pWrappedInstance);
    virtual void OnDeviceDestroyed(IInstanceBase* pDeviceInstance);

private:
    /// Hold an association of app handles to wrapped objects
    AppObjectToWrappedInstance m_currentObjects;
};

#endif // __VKT_WRAPPED_OBJECT_DATABASE_H__
