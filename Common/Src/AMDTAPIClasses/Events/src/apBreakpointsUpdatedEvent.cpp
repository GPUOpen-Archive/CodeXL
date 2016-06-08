//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBreakpointsUpdatedEvent.cpp
///
//==================================================================================

//------------------------------ apBreakpointsUpdatedEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::apBreakpointsUpdatedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
apBreakpointsUpdatedEvent::apBreakpointsUpdatedEvent(int updatedBreakpointIndex) : _updatedBreakpointIndex(updatedBreakpointIndex)
{

}

// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::~apBreakpointsUpdatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
apBreakpointsUpdatedEvent::~apBreakpointsUpdatedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apBreakpointsUpdatedEvent::type() const
{
    return OS_TOBJ_ID_BREAKPOINTS_UPDATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
bool apBreakpointsUpdatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    // Write the updated breakpoint index:
    ipcChannel << (gtInt32)_updatedBreakpointIndex;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
bool apBreakpointsUpdatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    // Read the updated breakpoint index:
    gtInt32 varAsInt32;
    ipcChannel >> varAsInt32;
    _updatedBreakpointIndex = (int)varAsInt32;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::eventType
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
apEvent::EventType apBreakpointsUpdatedEvent::eventType() const
{
    return apEvent::AP_BREAKPOINTS_UPDATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointsUpdatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        7/9/2011
// ---------------------------------------------------------------------------
apEvent* apBreakpointsUpdatedEvent::clone() const
{
    apBreakpointsUpdatedEvent* pClone = new apBreakpointsUpdatedEvent(_updatedBreakpointIndex);


    return pClone;
}


