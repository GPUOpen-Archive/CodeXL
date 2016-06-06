//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBeforeDebuggedProcessRunResumedEvent.cpp
///
//==================================================================================

//------------------------------ apBeforeDebuggedProcessRunResumedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apBeforeDebuggedProcessRunResumedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apBeforeDebuggedProcessRunResumedEvent::apBeforeDebuggedProcessRunResumedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        17/11/2005
// ---------------------------------------------------------------------------
apBeforeDebuggedProcessRunResumedEvent::apBeforeDebuggedProcessRunResumedEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apBeforeDebuggedProcessRunResumedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apBeforeDebuggedProcessRunResumedEvent::type() const
{
    return OS_TOBJ_ID_BEFORE_DEBUGGED_PROCESS_RUN_RESUMED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apBeforeDebuggedProcessRunResumedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apBeforeDebuggedProcessRunResumedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apBeforeDebuggedProcessRunResumedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apBeforeDebuggedProcessRunResumedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBeforeDebuggedProcessRunResumedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        17/11/2005
// ---------------------------------------------------------------------------
apEvent::EventType apBeforeDebuggedProcessRunResumedEvent::eventType() const
{
    return apEvent::AP_BEFORE_DEBUGGED_PROCESS_RUN_RESUMED;
}


// ---------------------------------------------------------------------------
// Name:        apBeforeDebuggedProcessRunResumedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        17/11/2005
// ---------------------------------------------------------------------------
apEvent* apBeforeDebuggedProcessRunResumedEvent::clone() const
{
    apBeforeDebuggedProcessRunResumedEvent* pEventCopy = new apBeforeDebuggedProcessRunResumedEvent;
    return pEventCopy;
}

