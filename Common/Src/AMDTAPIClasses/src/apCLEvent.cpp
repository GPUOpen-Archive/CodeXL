//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLEvent.cpp
///
//==================================================================================

//------------------------------ apCLEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apCLEvent.h>


// ---------------------------------------------------------------------------
// Name:        apCLEvent::apCLEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apCLEvent::apCLEvent(oaCLEventHandle eventHandle, oaCLCommandQueueHandle queueHandle, bool retainedBySpy)
    : _eventHandle(eventHandle), _controllingQueueHandle(queueHandle), _referenceCount(1), _isRetainedBySpy(retainedBySpy)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLEvent::~apCLEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apCLEvent::~apCLEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLEvent::type() const
{
    return OS_TOBJ_ID_CL_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apCLEvent::writeSelfIntoChannel
// Description: Writes this class into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
bool apCLEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_eventHandle;
    ipcChannel << (gtUInt64)_controllingQueueHandle;
    ipcChannel << (gtUInt32)_commandType;
    ipcChannel << (gtUInt32)_referenceCount;
    ipcChannel << _isRetainedBySpy;
    ipcChannel << _eventName;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLEvent::readSelfFromChannel
// Description: Reads this class from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
bool apCLEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apAllocatedObject::readSelfFromChannel(ipcChannel);

    gtUInt64 eventHandleAsUInt64 = (gtUInt64)OA_CL_NULL_HANDLE;
    ipcChannel >> eventHandleAsUInt64;
    _eventHandle = (oaCLEventHandle)eventHandleAsUInt64;

    gtUInt64 controllingQueueHandleAsUInt64 = (gtUInt64)OA_CL_NULL_HANDLE;
    ipcChannel >> controllingQueueHandleAsUInt64;
    _controllingQueueHandle = (oaCLCommandQueueHandle)controllingQueueHandleAsUInt64;

    gtUInt32 commandTypeAsUInt32 = (gtUInt32)CL_NONE;
    ipcChannel >> commandTypeAsUInt32;
    _commandType = (cl_command_type)commandTypeAsUInt32;

    gtUInt32 referenceCountAsUInt32 = 1;
    ipcChannel >> referenceCountAsUInt32;
    _referenceCount = (unsigned int)referenceCountAsUInt32;

    ipcChannel >> _isRetainedBySpy;
    ipcChannel >> _eventName;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLEvent::displayReferenceCount
// Description: Returns this event's external reference count (minus the 1 we
//              add for measurement).
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
unsigned int apCLEvent::displayReferenceCount() const
{
    unsigned int retVal = _referenceCount;

    // If we added to this object's reference count, substract 1:
    if (_isRetainedBySpy && (_referenceCount > 0))
    {
        retVal--;
    }

    return retVal;
}

