//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLObjectID.cpp
///
//==================================================================================

//------------------------------ apCLObjectID.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apCLObjectID.h>


// ---------------------------------------------------------------------------
// Name:        apCLObjectID::apCLObjectID
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
apCLObjectID::apCLObjectID(int contextId, int objectId, osTransferableObjectType objType, int ownerObjectId, int objectDisplayName)
    : _contextId(contextId), _objectId(objectId), _ownerObjectId(ownerObjectId), _objectDisplayName(objectDisplayName), _objectType(objType)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLObjectID::~apCLObjectID
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
apCLObjectID::~apCLObjectID()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLObjectID::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLObjectID::type() const
{
    return OS_TOBJ_ID_CL_OBJECT_ID;
}


// ---------------------------------------------------------------------------
// Name:        apCLObjectID::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool apCLObjectID::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_contextId;
    ipcChannel << (gtInt32)_objectId;
    ipcChannel << (gtInt32)_ownerObjectId;
    ipcChannel << (gtInt32)_objectType;
    ipcChannel << (gtInt32)_objectDisplayName;
    ipcChannel << _objectName;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLObjectID::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool apCLObjectID::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 idAsInt32 = 0;
    ipcChannel >> idAsInt32;
    _contextId = (int)idAsInt32;

    ipcChannel >> idAsInt32;
    _objectId = (int)idAsInt32;

    ipcChannel >> idAsInt32;
    _ownerObjectId = (int)idAsInt32;

    gtInt32 objectTypeAsInt32 = 0;
    ipcChannel >> objectTypeAsInt32;
    _objectType = (osTransferableObjectType)objectTypeAsInt32;

    ipcChannel >> idAsInt32;
    _objectDisplayName = (int)idAsInt32;

    ipcChannel >> _objectName;

    return true;
}

