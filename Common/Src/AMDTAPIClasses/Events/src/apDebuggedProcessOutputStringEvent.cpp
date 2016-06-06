//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessOutputStringEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessOutputStringEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::apDebuggedProcessOutputStringEvent
// Description: Constructor
// Arguments:  outputString - The debugged process outputted string.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
apDebuggedProcessOutputStringEvent::apDebuggedProcessOutputStringEvent(const gtString& outputString)
    : apEvent(OS_NO_THREAD_ID), _debuggedProcessOutputString(outputString)
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessOutputStringEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_OUTPUT_STRING_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessOutputStringEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the debugged process output string:
    ipcChannel << _debuggedProcessOutputString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessOutputStringEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the debugged process output string:
    ipcChannel >> _debuggedProcessOutputString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessOutputStringEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_OUTPUT_STRING;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessOutputStringEvent::clone() const
{
    apDebuggedProcessOutputStringEvent* pEventCopy = new apDebuggedProcessOutputStringEvent(_debuggedProcessOutputString);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessOutputStringEvent::apDebuggedProcessOutputStringEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apDebuggedProcessOutputStringEvent::apDebuggedProcessOutputStringEvent()
{

}

