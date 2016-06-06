//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apComputeContextCreatedEvent.cpp
///
//==================================================================================

//------------------------------ apComputeContextCreatedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::apComputeContextCreatedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that created the compute context.
//              contextId - The OpenCL Server's compute context id.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apComputeContextCreatedEvent::apComputeContextCreatedEvent(osThreadId triggeringThreadId, int contextId)
    : apEvent(triggeringThreadId), _contextId(contextId)
{
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::~apComputeContextCreatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apComputeContextCreatedEvent::~apComputeContextCreatedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apComputeContextCreatedEvent::type() const
{
    return OS_TOBJ_ID_COMPUTE_CONTEXT_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
bool apComputeContextCreatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenGL Server context id:
    ipcChannel << _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
bool apComputeContextCreatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenGL Server context id:
    ipcChannel >> _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apEvent::EventType apComputeContextCreatedEvent::eventType() const
{
    return apEvent::AP_COMPUTE_CONTEXT_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apEvent* apComputeContextCreatedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apComputeContextCreatedEvent* pEventCopy = new apComputeContextCreatedEvent(threadId, _contextId);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apComputeContextCreatedEvent::apComputeContextCreatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
apComputeContextCreatedEvent::apComputeContextCreatedEvent()
    : _contextId(0)
{
}

