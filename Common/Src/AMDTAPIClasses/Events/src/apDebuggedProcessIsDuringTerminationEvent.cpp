//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessIsDuringTerminationEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessIsDuringTerminationEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessIsDuringTerminationEvent.h>

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::apDebuggedProcessIsDuringTerminationEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apDebuggedProcessIsDuringTerminationEvent::apDebuggedProcessIsDuringTerminationEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::~apDebuggedProcessIsDuringTerminationEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apDebuggedProcessIsDuringTerminationEvent::~apDebuggedProcessIsDuringTerminationEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessIsDuringTerminationEvent::type() const
{
    return OS_TOBJ_ID_DURING_DEBUGGED_PROCESS_TERMINATION_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool apDebuggedProcessIsDuringTerminationEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool apDebuggedProcessIsDuringTerminationEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessIsDuringTerminationEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_IS_DURING_TERMINATION;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessIsDuringTerminationEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessIsDuringTerminationEvent::clone() const
{
    apDebuggedProcessIsDuringTerminationEvent* pEventCopy = new apDebuggedProcessIsDuringTerminationEvent;
    return pEventCopy;
}
