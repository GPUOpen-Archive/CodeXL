//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apThreadTerminatedEvent.cpp
///
//==================================================================================

//------------------------------ apThreadTerminatedEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::apThreadTerminatedEvent
// Description: Constructor
// Arguments:   threadOSId - The terminated thread OS id.
//              threadExitCode - The thread exit code.
//              threadTerminationTime - The thread termination time.
// Author:  AMD Developer Tools Team
// Date:        8/5/2005
// ---------------------------------------------------------------------------
apThreadTerminatedEvent::apThreadTerminatedEvent(const osThreadId& threadOSId,
                                                 long threadExitCode,
                                                 const osTime& threadTerminationTime)
    : _threadOSId(threadOSId), _threadExitCode(threadExitCode),
      _threadTerminationTime(threadTerminationTime)
{
}

// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apThreadTerminatedEvent::type() const
{
    return OS_TOBJ_ID_THREAD_TERMINATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apThreadTerminatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the thread ID:
    ipcChannel << (gtUInt64)_threadOSId;

    // Write the exit code:
    ipcChannel << (gtInt32)_threadExitCode;

    // Write the exit time:
    ipcChannel << _threadTerminationTime;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apThreadTerminatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the thread ID:
    gtUInt64 threadOSIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    ipcChannel >> threadOSIdAsUInt64;
    _threadOSId = (osThreadId)threadOSIdAsUInt64;

    // Read the exit code:
    gtInt32 threadExitCodeAsInt32 = -1;
    ipcChannel >> threadExitCodeAsInt32;
    _threadExitCode = (long)threadExitCodeAsInt32;

    // Read the exit time:
    ipcChannel >> _threadTerminationTime;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        8/5/2005
// ---------------------------------------------------------------------------
apEvent::EventType apThreadTerminatedEvent::eventType() const
{
    return apEvent::AP_THREAD_TERMINATED;
}


// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        8/5/2005
// ---------------------------------------------------------------------------
apEvent* apThreadTerminatedEvent::clone() const
{
    apEvent* retVal = new apThreadTerminatedEvent(_threadOSId, _threadExitCode,
                                                  _threadTerminationTime);

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apThreadTerminatedEvent::apThreadTerminatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apThreadTerminatedEvent::apThreadTerminatedEvent():
    _threadOSId(OS_NO_THREAD_ID), _threadExitCode(-1)
{

}

