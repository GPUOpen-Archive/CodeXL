//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessCreationFailureEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessCreationFailureEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::apDebuggedProcessCreationFailureEvent
// Description: Constructor
// Arguments:   exitCode - The debugged process exit code.
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
apDebuggedProcessCreationFailureEvent::apDebuggedProcessCreationFailureEvent(ProcessCreationFailureReason processCreationFailureReason, const gtString& createdProcessCommandLine, const gtString& createdProcessWorkDir, const gtString& processCreationError)
    : _failureReason(processCreationFailureReason), _createdProcessCommandLine(createdProcessCommandLine), _createdProcessWorkDir(createdProcessWorkDir), _processCreationError(processCreationError)
{
    // Set the current time as the process creation time.
    _processCreationTime.setFromCurrentTime();
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessCreationFailureEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_CREATION_FAILURE_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
bool apDebuggedProcessCreationFailureEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the failure reason:
    ipcChannel << (gtInt32)_failureReason;

    // Write the termination time:
    ipcChannel << _processCreationTime;

    // Write the command line:
    ipcChannel << _createdProcessCommandLine;

    // Write the working folder:
    ipcChannel << _createdProcessWorkDir;

    // Write the process creation error:
    ipcChannel << _processCreationError;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
bool apDebuggedProcessCreationFailureEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the failure reason:
    gtInt32 failureReasonAsInt32 = 0;
    ipcChannel >> failureReasonAsInt32;
    _failureReason = (ProcessCreationFailureReason)failureReasonAsInt32;

    // Read the termination time:
    ipcChannel >> _processCreationTime;

    // Read the command line:
    ipcChannel >> _createdProcessCommandLine;

    // Read the working folder:
    ipcChannel >> _createdProcessWorkDir;

    // Read the process creation error:
    ipcChannel >> _processCreationError;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessCreationFailureEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessCreationFailureEvent::clone() const
{
    apDebuggedProcessCreationFailureEvent* pEventCopy = new apDebuggedProcessCreationFailureEvent(_failureReason, _createdProcessCommandLine, _createdProcessWorkDir, _processCreationError);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreationFailureEvent::apDebuggedProcessCreationFailureEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        11/2/2010
// ---------------------------------------------------------------------------
apDebuggedProcessCreationFailureEvent::apDebuggedProcessCreationFailureEvent()
{

}

