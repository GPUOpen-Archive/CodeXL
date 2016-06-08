//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apProfileProgressEvent.cpp
///
//==================================================================================

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>

// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::apProfileProgressEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
apProfileProgressEvent::apProfileProgressEvent(const gtString& profilerName, const gtString& progress, const int value):
    m_profilerName(profilerName), m_progress(progress), m_value(value), m_aborted(false), m_increment(false)
{
}

// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::~apProfileProgressEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
apProfileProgressEvent::~apProfileProgressEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
osTransferableObjectType apProfileProgressEvent::type() const
{
    return OS_TOBJ_ID_PROFILE_PROGRESS_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
bool apProfileProgressEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write my progress:
    ipcChannel << m_profilerName;
    ipcChannel << m_progress;
    ipcChannel << (gtUInt32)m_value;
    ipcChannel << m_aborted;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
bool apProfileProgressEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read my progress:
    gtString tempStr;
    ipcChannel >> tempStr;
    m_profilerName = tempStr;

    ipcChannel >> tempStr;
    m_progress = tempStr;

    gtUInt32 varAsUInt32 = 0;
    ipcChannel >> varAsUInt32;
    m_value = varAsUInt32;

    bool aborted;
    ipcChannel >> aborted;
    m_aborted = aborted;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
apEvent::EventType apProfileProgressEvent::eventType() const
{
    return apEvent::AP_PROFILE_PROGRESS_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apProfileProgressEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        5/29/2012
// ---------------------------------------------------------------------------
apEvent* apProfileProgressEvent::clone() const
{
    apProfileProgressEvent* pClone = new apProfileProgressEvent(m_profilerName, m_progress, m_value);

    pClone->setAborted(m_aborted);
    pClone->setIncrement(m_increment);

    return pClone;
}
