//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLQueueCreatedEvent.cpp
///
//==================================================================================

//------------------------------ apOpenCLQueueCreatedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::apOpenCLQueueCreatedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that deleted the render context.
//              contextId - The OpenCL Server context id.
//              queueId - The OpenCL queue id.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apOpenCLQueueCreatedEvent::apOpenCLQueueCreatedEvent(osThreadId triggeringThreadId, int contextID, int queueID)
    : apEvent(triggeringThreadId), _contextID(contextID), _queueID(queueID)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::~apOpenCLQueueCreatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apOpenCLQueueCreatedEvent::~apOpenCLQueueCreatedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLQueueCreatedEvent::type() const
{
    return OS_TOBJ_ID_COMMAND_QUEUE_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
bool apOpenCLQueueCreatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenCL Server context id:
    ipcChannel << (gtInt32)_contextID;

    // Write the OpenCL Server queue id:
    ipcChannel << (gtInt32)_queueID;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
bool apOpenCLQueueCreatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenCL Server context id:
    gtInt32 int32Var = 0;
    ipcChannel >> int32Var;
    _contextID = (int)int32Var;

    // Read the OpenCL queue id:
    int32Var = 0;
    ipcChannel >> int32Var;
    _queueID = (int)int32Var;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apEvent::EventType apOpenCLQueueCreatedEvent::eventType() const
{
    return apEvent::AP_OPENCL_QUEUE_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apEvent* apOpenCLQueueCreatedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apOpenCLQueueCreatedEvent* pEventCopy = new apOpenCLQueueCreatedEvent(threadId, _contextID, _queueID);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCreatedEvent::apOpenCLQueueCreatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apOpenCLQueueCreatedEvent::apOpenCLQueueCreatedEvent()
    : _queueID(0)
{
}

