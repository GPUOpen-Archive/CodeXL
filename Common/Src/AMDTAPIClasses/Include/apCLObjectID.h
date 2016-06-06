//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLObjectID.h
///
//==================================================================================

//------------------------------ apCLObjectID.h ------------------------------

#ifndef __APCLOBJECTID
#define __APCLOBJECTID

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// ----------------------------------------------------------------------------------
// Class Name:           apCLObjectID : public osTransferableObject
// General Description: Holds an OpenCL object id - context ,internal id, and object type
//                      This class is used for representing an OpenCL handle object
// Author:  AMD Developer Tools Team
// Creation Date:        8/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCLObjectID : public osTransferableObject
{
public:
    // Constructor / destructor:
    apCLObjectID(int contextId = -1, int objectId = -1, osTransferableObjectType objType = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES, int _ownerObjectId = -1, int objectDisplayName = -1);
    virtual ~apCLObjectID();

    // The context id:
    int _contextId;

    // The internal object id:
    int _objectId;

    // The object's internal id:
    // For example: OpenCL kernel are identified by their owner program id:
    int _ownerObjectId;

    // The object display name. Memory objects, for example, are released and we want to display the
    // original index:
    int _objectDisplayName;

    // cl_gremedy_object_naming:
    gtString _objectName;

    // The object type:
    osTransferableObjectType _objectType;

public:

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};

#endif  // __APCOUNTERID
