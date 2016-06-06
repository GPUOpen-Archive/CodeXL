//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLMemObject.cpp
///
//==================================================================================

//------------------------------ apCLMemObject.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apCLMemObject.h>


// ---------------------------------------------------------------------------
// Name:        apCLMemObject::apCLMemObject
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        17/2/2010
// ---------------------------------------------------------------------------
apCLMemObject::apCLMemObject()
    : _memHandle(OA_CL_NULL_HANDLE), _wasMarkedForDeletion(false), _referenceCount(0), _memoryFlags(0), _memObjectName(L""),
      m_destructorPfnNotify(OS_NULL_PROCEDURE_ADDRESS_64), m_destructorUserData(OS_NULL_PROCEDURE_ADDRESS_64), _wasFirstWriteOperationPerformed(0)
{

}


// ---------------------------------------------------------------------------
// Name:        apCLMemObject::~apCLMemObject
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        17/2/2010
// ---------------------------------------------------------------------------
apCLMemObject::~apCLMemObject()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMemObject::writeSelfIntoChannel
// Description: Writes this object into an IPC Channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/2/2010
// ---------------------------------------------------------------------------
bool apCLMemObject::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the object handle:
    ipcChannel << (gtUInt64)_memHandle;

    // Write the deletion status:
    ipcChannel << _wasMarkedForDeletion;

    // Write the reference count:
    ipcChannel << _referenceCount;

    // Write the memory flags:
    retVal = _memoryFlags.writeSelfIntoChannel(ipcChannel);

    // Write the object name:
    ipcChannel << _memObjectName;

    // Write the destructor info:
    ipcChannel << (gtUInt64)m_destructorPfnNotify;
    ipcChannel << (gtUInt64)m_destructorUserData;

    // Write the first write operation status:
    ipcChannel << _wasFirstWriteOperationPerformed;

    // Write the allocated object Info:
    retVal = apAllocatedObject::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemObject::readSelfFromChannel
// Description: Reads this object from an IPC Channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/2/2010
// ---------------------------------------------------------------------------
bool apCLMemObject::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the object handle:
    gtUInt64 memHandleAsUInt64 = 0;
    ipcChannel >> memHandleAsUInt64;
    _memHandle = (oaCLMemHandle)memHandleAsUInt64;

    // Read the deletion status:
    ipcChannel >> _wasMarkedForDeletion;

    // Write the reference count:
    ipcChannel >> _referenceCount;

    // Read the memory flags:
    retVal = _memoryFlags.readSelfFromChannel(ipcChannel);

    // Read the object name:
    ipcChannel >> _memObjectName;

    // Read the destructor info:
    gtUInt64 pfnNotifyAsUInt64 = 0;
    ipcChannel >> pfnNotifyAsUInt64;
    m_destructorPfnNotify = (osProcedureAddress64)pfnNotifyAsUInt64;
    gtUInt64 userDataAsUInt64 = 0;
    ipcChannel >> userDataAsUInt64;
    m_destructorUserData = (osProcedureAddress64)userDataAsUInt64;

    // Read the first write operation status:
    ipcChannel >> _wasFirstWriteOperationPerformed;

    // Read the allocated object Info:
    retVal = apAllocatedObject::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

