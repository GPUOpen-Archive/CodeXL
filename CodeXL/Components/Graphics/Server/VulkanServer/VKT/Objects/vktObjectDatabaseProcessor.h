//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktObjectDatabaseProcessor.h
/// \brief  Header file for a Vulkan object database processor.
///         This class maintains handles to object content and queries their
///         instances to retrieve information.
//==============================================================================

#ifndef __VKT_OBJECT_DATABASE_PROCESSOR_H__
#define __VKT_OBJECT_DATABASE_PROCESSOR_H__

#include "../../../Common/ObjectDatabaseProcessor.h"
#include "../../../Common/TSingleton.h"
#include "../Util/vktUtil.h"

#include "vktWrappedObjectDatabase.h"

//-----------------------------------------------------------------------------
/// The VktObjectDatabaseProcessor contains a Vulkan object database and
/// helps respond to ObjectInspector requests under Vulkan.
//-----------------------------------------------------------------------------
class VktObjectDatabaseProcessor : public ObjectDatabaseProcessor, public TSingleton< VktObjectDatabaseProcessor >
{
    // TSingleton needs to be able to use our constructor.
    friend class TSingleton < VktObjectDatabaseProcessor >;

public:
    VktObjectDatabaseProcessor() {}
    virtual ~VktObjectDatabaseProcessor() {}
    virtual ModernAPILayerManager* GetParentLayerManager();

    /// Return a reference to the object database
    virtual WrappedObjectDatabase* GetObjectDatabase() { return &m_objectDatabase; }

protected:
    virtual int GetObjectTypeFromString(const gtASCIIString& inObjectTypeString) const;

    /// Return the first object type
    virtual int GetFirstObjectType() const { return 0; }

    /// Return the last object type
    virtual int GetLastObjectType() const { return 0; }

    /// Return the device type
    virtual int GetDeviceType() const { return 0; }

private:
    /// The structure backing our object database
    VktWrappedObjectDatabase m_objectDatabase;
};

#endif // __VKT_OBJECT_DATABASE_PROCESSOR_H__