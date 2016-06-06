//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInfrastructureFailureEvent.cpp
///
//==================================================================================

//------------------------------ apInfrastructureFailureEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apInfrastructureFailureEvent.h>



// ---------------------------------------------------------------------------
// Name:        apInfrastructureFailureEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apInfrastructureFailureEvent::type() const
{
    return OS_TOBJ_ID_INFRASTRUCTURE_FAILURE_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureFailureEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apInfrastructureFailureEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the failure reason into the channel:
    ipcChannel << (gtInt32)_failureReason;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureFailureEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apInfrastructureFailureEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the memory leak attributes into the channel:
    gtInt32 failureReasonAsInt = FAILED_TO_INITIALIZE_GDB;
    ipcChannel >> failureReasonAsInt;
    _failureReason = (FailureReason)failureReasonAsInt;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureFailureEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        5/8/2008
// ---------------------------------------------------------------------------
apEvent::EventType apInfrastructureFailureEvent::eventType() const
{
    return apEvent::AP_INFRASTRUCTURE_FAILURE_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apInfrastructureFailureEvent::clone
// Description: Returns a copy of self.
// Author:  AMD Developer Tools Team
// Date:        5/8/2008
// ---------------------------------------------------------------------------
apEvent* apInfrastructureFailureEvent::clone() const
{
    apEvent* retVal = new apInfrastructureFailureEvent(_failureReason);
    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        apInfrastructureFailureEvent::apInfrastructureFailureEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apInfrastructureFailureEvent::apInfrastructureFailureEvent():
    _failureReason(FAILED_TO_INITIALIZE_GDB)
{

}

