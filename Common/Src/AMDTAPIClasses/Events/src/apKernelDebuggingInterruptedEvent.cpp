//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelDebuggingInterruptedEvent.cpp
///
//==================================================================================

//------------------------------ apKernelDebuggingInterruptedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingInterruptedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::apKernelDebuggingInterruptedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
apKernelDebuggingInterruptedEvent::apKernelDebuggingInterruptedEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::~apKernelDebuggingInterruptedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
apKernelDebuggingInterruptedEvent::~apKernelDebuggingInterruptedEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apKernelDebuggingInterruptedEvent::type() const
{
    return OS_TOBJ_ID_KERNEL_DEBUGGING_INTERRUPTED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
bool apKernelDebuggingInterruptedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
bool apKernelDebuggingInterruptedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
apEvent::EventType apKernelDebuggingInterruptedEvent::eventType() const
{
    return apEvent::AP_KERNEL_DEBUGGING_INTERRUPTED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingInterruptedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        31/5/2011
// ---------------------------------------------------------------------------
apEvent* apKernelDebuggingInterruptedEvent::clone() const
{
    apKernelDebuggingInterruptedEvent* pEventCopy = new apKernelDebuggingInterruptedEvent;
    return pEventCopy;
}