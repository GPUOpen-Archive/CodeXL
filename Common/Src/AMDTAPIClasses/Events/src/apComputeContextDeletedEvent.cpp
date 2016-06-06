//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apComputeContextDeletedEvent.cpp
///
//==================================================================================

//------------------------------ apComputeContextDeletedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::apComputeContextDeletedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that deleted the compute context.
//              contextId - The OpenGL Server's compute context id.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apComputeContextDeletedEvent::apComputeContextDeletedEvent(osThreadId triggeringThreadId, int contextId)
    : apEvent(triggeringThreadId), _contextId(contextId)
{
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::~apComputeContextDeletedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apComputeContextDeletedEvent::~apComputeContextDeletedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apComputeContextDeletedEvent::type() const
{
    return OS_TOBJ_ID_COMPUTE_CONTEXT_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
bool apComputeContextDeletedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenGL Server context id:
    ipcChannel << _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
bool apComputeContextDeletedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenGL Server context id:
    ipcChannel >> _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apEvent::EventType apComputeContextDeletedEvent::eventType() const
{
    return apEvent::AP_COMPUTE_CONTEXT_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apEvent* apComputeContextDeletedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apComputeContextDeletedEvent* pEventCopy = new apComputeContextDeletedEvent(threadId, _contextId);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apComputeContextDeletedEvent::apComputeContextDeletedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apComputeContextDeletedEvent::apComputeContextDeletedEvent()
    : _contextId(0)
{
}

