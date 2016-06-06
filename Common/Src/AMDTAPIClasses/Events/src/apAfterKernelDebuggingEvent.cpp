//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAfterKernelDebuggingEvent.cpp
///
//==================================================================================

//------------------------------ apAfterKernelDebuggingEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apAfterKernelDebuggingEvent.h>

// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::apAfterKernelDebuggingEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apAfterKernelDebuggingEvent::apAfterKernelDebuggingEvent(osThreadId triggeringThreadID)
    : apEvent(triggeringThreadID)
{
}

// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::~apAfterKernelDebuggingEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apAfterKernelDebuggingEvent::~apAfterKernelDebuggingEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apAfterKernelDebuggingEvent::type() const
{
    return OS_TOBJ_ID_AFTER_KERNEL_DEBUGGING_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool apAfterKernelDebuggingEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    //////////////////////////////////////////////////////////////////////////
    // Uri, 30/5/11:
    // We READ from the pipe here to synchronize the Server and Client.
    // The place where this event is written is immediately followed by
    // triggering a breakpoint exception. If we do not synchronize here, the
    // event registration will race with the breakpoint handling, sometimes
    // causing an unexpected event to reach the package event handlers, which
    // messes up the timing.
    //////////////////////////////////////////////////////////////////////////
    gtInt32 dummyValue = -1;
    ipcChannel >> dummyValue;
    GT_ASSERT(dummyValue == 0x840011);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool apAfterKernelDebuggingEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    //////////////////////////////////////////////////////////////////////////
    // Uri, 30/5/11:
    // We WRITE into the pipe here to synchronize the Server and Client.
    // The place where this event is written is immediately followed by
    // triggering a breakpoint exception. If we do not synchronize here, the
    // event registration will race with the breakpoint handling, sometimes
    // causing an unexpected event to reach the package event handlers, which
    // messes up the timing.
    //////////////////////////////////////////////////////////////////////////
    gtInt32 dummyValue = 0x840011;
    ipcChannel << dummyValue;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apEvent::EventType apAfterKernelDebuggingEvent::eventType() const
{
    return apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apAfterKernelDebuggingEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apEvent* apAfterKernelDebuggingEvent::clone() const
{
    apAfterKernelDebuggingEvent* pClone = new apAfterKernelDebuggingEvent();


    return pClone;
}

