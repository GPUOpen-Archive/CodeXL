//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInfrastructureStartsBeingBusyEvent.cpp
///
//==================================================================================

//------------------------------ apInfrastructureStartsBeingBusyEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apInfrastructureStartsBeingBusyEvent.h>

// ---------------------------------------------------------------------------
// Name:        apInfrastructureStartsBeingBusyEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apInfrastructureStartsBeingBusyEvent::type() const
{
    return OS_TOBJ_ID_INFRASTRUCTURE_STARTS_BEING_BUSY_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apInfrastructureStartsBeingBusyEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apInfrastructureStartsBeingBusyEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the reason:
    ipcChannel << (gtInt32)_busyReason;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apInfrastructureStartsBeingBusyEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apInfrastructureStartsBeingBusyEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the reason:
    gtInt32 busyReasonAsInt32 = AP_UPDATING_DEBUGGED_PROCESS_DATA;
    ipcChannel >> busyReasonAsInt32;
    _busyReason = (BusyReason)busyReasonAsInt32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apInfrastructureStartsBeingBusyEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        16/7/2006
// ---------------------------------------------------------------------------
apEvent::EventType apInfrastructureStartsBeingBusyEvent::eventType() const
{
    return apEvent::AP_INFRASTRUCTURE_STARTS_BEING_BUSY_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureStartsBeingBusyEvent::clone
// Description: Returns a copy of self.
// Author:  AMD Developer Tools Team
// Date:        16/7/2006
// ---------------------------------------------------------------------------
apEvent* apInfrastructureStartsBeingBusyEvent::clone() const
{
    apEvent* retVal = new apInfrastructureStartsBeingBusyEvent(_busyReason);
    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        apInfrastructureStartsBeingBusyEvent::apInfrastructureStartsBeingBusyEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apInfrastructureStartsBeingBusyEvent::apInfrastructureStartsBeingBusyEvent():
    _busyReason(AP_UPDATING_DEBUGGED_PROCESS_DATA)
{

}
