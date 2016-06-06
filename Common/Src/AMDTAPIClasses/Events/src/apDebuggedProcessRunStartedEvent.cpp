//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunStartedEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessRunStartedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::apDebuggedProcessRunStartedEvent
// Description: Constructor
// Arguments:   processId - The process OS id.
//              processRunStartedTime - The time in which the debugged process run
//                                      started.
// Author:  AMD Developer Tools Team
// Date:        22/6/2004
// ---------------------------------------------------------------------------
apDebuggedProcessRunStartedEvent::apDebuggedProcessRunStartedEvent(osProcessId processId, const osTime& processRunStartedTime)
    : _processId(processId), _processRunStartedTime(processRunStartedTime)
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::~apDebuggedProcessRunStartedEvent
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        22/6/2004
// ---------------------------------------------------------------------------
apDebuggedProcessRunStartedEvent::~apDebuggedProcessRunStartedEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessRunStartedEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_RUN_STARTED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunStartedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the process ID:
    ipcChannel << (gtUInt64)_processId;

    // Read the start time:
    ipcChannel << _processRunStartedTime;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunStartedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the process ID:
    gtUInt64 processIdAsUInt64 = 0;
    ipcChannel >> processIdAsUInt64;
    _processId = (osProcessId)processIdAsUInt64;

    // Read the start time:
    ipcChannel >> _processRunStartedTime;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessRunStartedEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessRunStartedEvent::clone() const
{
    apDebuggedProcessRunStartedEvent* pEventCopy = new apDebuggedProcessRunStartedEvent(_processId, _processRunStartedTime);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedEvent::apDebuggedProcessRunStartedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apDebuggedProcessRunStartedEvent::apDebuggedProcessRunStartedEvent():
    _processId(0)
{

}
