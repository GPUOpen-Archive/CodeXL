//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunSuspendedEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessRunSuspendedEvent.cpp ------------------------------

// Infra
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunSuspendedEvent::apDebuggedProcessRunSuspendedEvent
// Description: Constructor
// Arguments: triggeringThreadId - The debugged process thread that triggered this event.
// Author:  AMD Developer Tools Team
// Date:        10/6/2004
// ---------------------------------------------------------------------------
apDebuggedProcessRunSuspendedEvent::apDebuggedProcessRunSuspendedEvent(osThreadId triggeringThreadId, bool IsHostBreakPoint)
    : apEvent(triggeringThreadId), _IsHostBreakpointReason(IsHostBreakPoint)
{
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunSuspendedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessRunSuspendedEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_RUN_SUSPENDED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunSuspendedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunSuspendedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    ipcChannel << _IsHostBreakpointReason;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunSuspendedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunSuspendedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    ipcChannel >> _IsHostBreakpointReason;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunSuspendedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        10/6/2004
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessRunSuspendedEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunSuspendedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        10/6/2004
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessRunSuspendedEvent::clone() const
{
    // Get the triggering thread id:
    osThreadId threadId = triggeringThreadId();

    // Create a clone of this event:
    apDebuggedProcessRunSuspendedEvent* pEventCopy = new apDebuggedProcessRunSuspendedEvent(threadId, _IsHostBreakpointReason);
    return pEventCopy;
}

//////////////////////////////////////////////////////////////////////
/// \brief Get breakpoint reason. Host breakpoint or other
///
/// \return true in case host breakpoint process suspended reason
/// \author Vadim Entov
/// \date 05/01/2016
bool apDebuggedProcessRunSuspendedEvent::IsHostBreakPointReason() const
{
    return _IsHostBreakpointReason;
}









































