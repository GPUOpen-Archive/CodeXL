//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessTerminatedEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessTerminatedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::apDebuggedProcessTerminatedEvent
// Description: Constructor
// Arguments:   exitCode - The debugged process exit code.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apDebuggedProcessTerminatedEvent::apDebuggedProcessTerminatedEvent(long exitCode)
    : _exitCode(exitCode)
{
    // Set the current time as the process termination time.
    _processTerminationTime.setFromCurrentTime();
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::~apDebuggedProcessTerminatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        6/6/2011
// ---------------------------------------------------------------------------
apDebuggedProcessTerminatedEvent::~apDebuggedProcessTerminatedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessTerminatedEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_TERMINATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessTerminatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the exit code:
    ipcChannel << (gtInt32)_exitCode;

    // Write the termination time:
    ipcChannel << _processTerminationTime;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessTerminatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the exit code:
    gtInt32 exitCodeAsInt32 = -1;
    ipcChannel >> exitCodeAsInt32;
    _exitCode = (long)exitCodeAsInt32;

    // Read the termination time:
    ipcChannel >> _processTerminationTime;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessTerminatedEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_TERMINATED;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessTerminatedEvent::clone() const
{
    apDebuggedProcessTerminatedEvent* pEventCopy = new apDebuggedProcessTerminatedEvent(_exitCode);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessTerminatedEvent::apDebuggedProcessTerminatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apDebuggedProcessTerminatedEvent::apDebuggedProcessTerminatedEvent()
{

}

