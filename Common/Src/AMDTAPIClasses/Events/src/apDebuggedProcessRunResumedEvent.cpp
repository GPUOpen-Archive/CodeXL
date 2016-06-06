//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunResumedEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessRunResumedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunResumedEvent::apDebuggedProcessRunResumedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        10/6/2004
// ---------------------------------------------------------------------------
apDebuggedProcessRunResumedEvent::apDebuggedProcessRunResumedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunResumedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessRunResumedEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_RUN_RESUMED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunResumedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunResumedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunResumedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunResumedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunResumedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        10/6/2004
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessRunResumedEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunResumedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        10/6/2004
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessRunResumedEvent::clone() const
{
    apDebuggedProcessRunResumedEvent* pEventCopy = new apDebuggedProcessRunResumedEvent;
    return pEventCopy;
}
