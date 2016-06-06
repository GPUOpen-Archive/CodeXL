//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInfrastructureEndsBeingBusyEvent.cpp
///
//==================================================================================

//------------------------------ apInfrastructureEndsBeingBusyEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apInfrastructureEndsBeingBusyEvent.h>



// ---------------------------------------------------------------------------
// Name:        apInfrastructureEndsBeingBusyEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apInfrastructureEndsBeingBusyEvent::type() const
{
    return OS_TOBJ_ID_INFRASTRUCTURE_ENDS_BEING_BUSY_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apInfrastructureEndsBeingBusyEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apInfrastructureEndsBeingBusyEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureEndsBeingBusyEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apInfrastructureEndsBeingBusyEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureEndsBeingBusyEvent::eventType
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        16/7/2006
// ---------------------------------------------------------------------------
apEvent::EventType apInfrastructureEndsBeingBusyEvent::eventType() const
{
    return apEvent::AP_INFRASTRUCTURE_ENDS_BEING_BUSY_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureEndsBeingBusyEvent::clone
// Description: Returns a copy of self.
// Author:  AMD Developer Tools Team
// Date:        16/7/2006
// ---------------------------------------------------------------------------
apEvent* apInfrastructureEndsBeingBusyEvent::clone() const
{
    apEvent* retVal = new apInfrastructureEndsBeingBusyEvent;
    GT_RETURN_WITH_ASSERT(retVal);
}
