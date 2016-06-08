//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAllocatedObject.h
///
//==================================================================================

//------------------------------ apAllocatedObject.h ------------------------------

#ifndef __APALLOCATEDOBJECT_H
#define __APALLOCATEDOBJECT_H

// Infra
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apAllocatedObject: public osTransferableObject
// General Description: Describes an allocated object (an object which takes up graphic
//                      memory such as a texture or a display list) and holds its ID
//                      as represented in the spy's allocated objects manager
// Author:  AMD Developer Tools Team
// Creation Date:        20/10/2008
// ----------------------------------------------------------------------------------
class AP_API apAllocatedObject: public osTransferableObject
{
protected:
    // Do not allow creating changeable allocatedObjects which are not subclass items:
    apAllocatedObject();

public:
    virtual ~apAllocatedObject();

    int getAllocatedObjectId() const {return _allocatedObjectId;};

    // This function should only be used by the spy's allocated objects manager:
    bool setAllocatedObjectId(int allocObjId, bool allowOverwrite = false);

    // Overrides osTransferableObject:
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // (apAllocatedObject is not a type)
    virtual osTransferableObjectType type() const = 0;

private:
    // The position of the allocated object in the allocated objects manager's list.
    int _allocatedObjectId;
};

#endif //__APALLOCATEDOBJECT_H

