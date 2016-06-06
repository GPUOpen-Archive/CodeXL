//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLQueueDeletedEvent.cpp
///
//==================================================================================

//------------------------------ apOpenCLQueueDeletedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueDeletedEvent::apOpenCLQueueDeletedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that deleted the render context.
//              contextId - The OpenCL Server context id.
//              queueId - The OpenCL queue id.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apOpenCLQueueDeletedEvent::apOpenCLQueueDeletedEvent(osThreadId triggeringThreadId, int contextID, int queueID)
    : apEvent(triggeringThreadId), _contextID(contextID), _queueID(queueID)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueDeletedEvent::~apOpenCLQueueDeletedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apOpenCLQueueDeletedEvent::~apOpenCLQueueDeletedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueDeletedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLQueueDeletedEvent::type() const
{
    return OS_TOBJ_ID_COMMAND_QUEUE_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueDeletedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
bool apOpenCLQueueDeletedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
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
// Name:        apOpenCLQueueDeletedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
bool apOpenCLQueueDeletedEvent::readSelfFromChannel(osChannel& ipcChannel)
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
// Name:        apOpenCLQueueDeletedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apEvent::EventType apOpenCLQueueDeletedEvent::eventType() const
{
    return apEvent::AP_OPENCL_QUEUE_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueDeletedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apEvent* apOpenCLQueueDeletedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apOpenCLQueueDeletedEvent* pEventCopy = new apOpenCLQueueDeletedEvent(threadId, _contextID, _queueID);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueDeletedEvent::apOpenCLQueueDeletedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
apOpenCLQueueDeletedEvent::apOpenCLQueueDeletedEvent()
    : _queueID(0)
{
}

