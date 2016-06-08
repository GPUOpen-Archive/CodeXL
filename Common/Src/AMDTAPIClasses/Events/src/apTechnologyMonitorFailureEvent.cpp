//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apTechnologyMonitorFailureEvent.cpp
///
//==================================================================================

//------------------------------ apTechnologyMonitorFailureEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apTechnologyMonitorFailureEvent.h>


// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::apTechnologyMonitorFailureEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
apTechnologyMonitorFailureEvent::apTechnologyMonitorFailureEvent(const gtString& failInformation, osThreadId triggeringThreadID)
    : apEvent(triggeringThreadID), m_failInformation(failInformation)
{

}

// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::~apTechnologyMonitorFailureEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
apTechnologyMonitorFailureEvent::~apTechnologyMonitorFailureEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apTechnologyMonitorFailureEvent::type() const
{
    return OS_TOBJ_ID_TECHNOLOGY_MONITOR_FAILURE_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
bool apTechnologyMonitorFailureEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    ipcChannel << m_failInformation;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
bool apTechnologyMonitorFailureEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    ipcChannel >> m_failInformation;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::eventType
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
apEvent::EventType apTechnologyMonitorFailureEvent::eventType() const
{
    return apEvent::AP_TECHNOLOGY_MONITOR_FAILURE_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apTechnologyMonitorFailureEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        16/2/2014
// ---------------------------------------------------------------------------
apEvent* apTechnologyMonitorFailureEvent::clone() const
{
    apEvent* retVal = new apTechnologyMonitorFailureEvent(m_failInformation, triggeringThreadId());


    return retVal;
}
