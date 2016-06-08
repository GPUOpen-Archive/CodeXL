//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLSync.cpp
///
//==================================================================================

// -----------------------------   apGLSync.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apGLSync.h>

// ---------------------------------------------------------------------------
// Name:        apGLSync::apGLSync
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
apGLSync::apGLSync()
    : apAllocatedObject(), _syncID(-1), _syncHandle(OA_GL_NULL_HANDLE), _syncCondition(GL_NONE)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLSync::apGLSync
// Description: Copy constructor
// Arguments: other - The other VBO class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
apGLSync::apGLSync(const apGLSync& other)
{
    apGLSync::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLSync::~apGLSync
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
apGLSync::~apGLSync()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLSync::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
apGLSync& apGLSync::operator=(const apGLSync& other)
{
    _syncID = other._syncID;
    _syncHandle = other._syncHandle;
    _syncCondition = other._syncCondition;
    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLSync::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGLSync::type() const
{
    return OS_TOBJ_ID_GL_SYNC;
}

// ---------------------------------------------------------------------------
// Name:        apGLSync::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool apGLSync::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the Sync id:
    ipcChannel << (gtInt32)_syncID;

    // Write the sync handle:
    ipcChannel << (gtUInt64)_syncHandle;

    // Write the sync condition:
    ipcChannel << (gtInt32)_syncCondition;

    // Write the allocated object Info:
    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLSync::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool apGLSync::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the sync id from the channel:
    gtInt32 syncIDAsInt32 = -1;
    ipcChannel >> syncIDAsInt32;
    _syncID = (int)syncIDAsInt32;

    // Read the sync handle from the channel:
    gtUInt64 syncHandleAsUInt64 = 0;
    ipcChannel >> syncHandleAsUInt64;
    _syncHandle = (oaGLSyncHandle)syncHandleAsUInt64;

    // Read the sync condition from the channel:
    gtInt32 syncConditionAsInt32 = 0;
    ipcChannel >> syncConditionAsInt32;
    _syncCondition = (GLenum)syncConditionAsInt32;

    // Read the allocated object Info:
    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

