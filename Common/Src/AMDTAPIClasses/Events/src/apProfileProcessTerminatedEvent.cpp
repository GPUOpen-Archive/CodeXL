//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apProfileProcessTerminatedEvent.cpp
///
//==================================================================================

//------------------------------ apProfileProcessTerminatedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::apProfileProcessTerminatedEvent
// Description: Constructor
// Arguments:   profilerName - The name of the profiler triggering the event.
//              exitCode - The debugged process exit code.
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
apProfileProcessTerminatedEvent::apProfileProcessTerminatedEvent(const gtString& profilerName, long exitCode, int profileType)
    : m_profilerName(profilerName), m_exitCode(exitCode), m_profileType(profileType)
{
    // Set the current time as the process termination time.
    m_processTerminationTime.setFromCurrentTime();
}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::~apProfileProcessTerminatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
apProfileProcessTerminatedEvent::~apProfileProcessTerminatedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
osTransferableObjectType apProfileProcessTerminatedEvent::type() const
{
    return OS_TOBJ_ID_PROFILE_PROCESS_TERMINATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
bool apProfileProcessTerminatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the profiler name:
    ipcChannel << m_profilerName;

    // Write the exit code:
    ipcChannel << (gtInt32)m_exitCode;

    // Write the termination time:
    ipcChannel << m_processTerminationTime;

    // Write the profile type:
    ipcChannel << m_profileType;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
bool apProfileProcessTerminatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the profiler name:
    ipcChannel >> m_profilerName;

    // Read the exit code:
    gtInt32 valueAsInt32 = -1;
    ipcChannel >> valueAsInt32;
    m_exitCode = (long)valueAsInt32;

    // Read the termination time:
    ipcChannel >> m_processTerminationTime;

    // Read the exit code:
    valueAsInt32 = -1;
    ipcChannel >> valueAsInt32;
    m_profileType = (long)valueAsInt32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
apEvent::EventType apProfileProcessTerminatedEvent::eventType() const
{
    return apEvent::AP_PROFILE_PROCESS_TERMINATED;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
apEvent* apProfileProcessTerminatedEvent::clone() const
{
    apProfileProcessTerminatedEvent* pEventCopy = new apProfileProcessTerminatedEvent(m_profilerName, m_exitCode, m_profileType);
    pEventCopy->m_processTerminationTime = m_processTerminationTime;
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProcessTerminatedEvent::apProfileProcessTerminatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        5/23/2012
// ---------------------------------------------------------------------------
apProfileProcessTerminatedEvent::apProfileProcessTerminatedEvent()
{

}
