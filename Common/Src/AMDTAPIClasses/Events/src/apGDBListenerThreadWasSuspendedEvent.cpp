//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGDBListenerThreadWasSuspendedEvent.cpp
///
//==================================================================================

//------------------------------ apGDBListenerThreadWasSuspendedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apGDBListenerThreadWasSuspendedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::apGDBListenerThreadWasSuspendedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/1/2008
// ---------------------------------------------------------------------------
apGDBListenerThreadWasSuspendedEvent::apGDBListenerThreadWasSuspendedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::~apGDBListenerThreadWasSuspendedEvent
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        2/1/2008
// ---------------------------------------------------------------------------
apGDBListenerThreadWasSuspendedEvent::~apGDBListenerThreadWasSuspendedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGDBListenerThreadWasSuspendedEvent::type() const
{
    return OS_TOBJ_ID_GDB_LISTENER_THREAD_WAS_SUSPENDED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apGDBListenerThreadWasSuspendedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apGDBListenerThreadWasSuspendedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        2/1/2008
// ---------------------------------------------------------------------------
apEvent::EventType apGDBListenerThreadWasSuspendedEvent::eventType() const
{
    return apEvent::AP_GDB_LISTENER_THREAD_WAS_SUSPENDED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apGDBListenerThreadWasSuspendedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        2/1/2008
// ---------------------------------------------------------------------------
apEvent* apGDBListenerThreadWasSuspendedEvent::clone() const
{
    apGDBListenerThreadWasSuspendedEvent* pEventCopy = new apGDBListenerThreadWasSuspendedEvent;
    return pEventCopy;
}


