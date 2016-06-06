//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCallStackFrameSelectedEvent.cpp
///
//==================================================================================

//------------------------------ apCallStackFrameSelectedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apCallStackFrameSelectedEvent.h>



// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::apCallStackFrameSelectedEvent
// Description: Constructor
// Arguments:   const gtString& userWarningString
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
apCallStackFrameSelectedEvent::apCallStackFrameSelectedEvent(int frameIndex)
    : apEvent(), m_frameIndex(frameIndex)
{
}

// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::type
// Description: Return the event object type
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
osTransferableObjectType apCallStackFrameSelectedEvent::type() const
{
    return OS_TOBJ_ID_CALL_STACK_FRAME_SELECTED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
bool apCallStackFrameSelectedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the string:
    ipcChannel << (gtInt32)m_frameIndex;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
bool apCallStackFrameSelectedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the string:
    gtInt32 frameIndexAsInt32 = -1;
    ipcChannel >> frameIndexAsInt32;
    m_frameIndex = (int)frameIndexAsInt32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
apEvent::EventType apCallStackFrameSelectedEvent::eventType() const
{
    return apEvent::AP_CALL_STACK_FRAME_SELECTED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
apEvent* apCallStackFrameSelectedEvent::clone() const
{
    apCallStackFrameSelectedEvent* pEventCopy = new apCallStackFrameSelectedEvent(m_frameIndex);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apCallStackFrameSelectedEvent::apCallStackFrameSelectedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        7/19/2012
// ---------------------------------------------------------------------------
apCallStackFrameSelectedEvent::apCallStackFrameSelectedEvent()
{

}

