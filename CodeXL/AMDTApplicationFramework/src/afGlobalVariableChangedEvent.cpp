//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalVariableChangedEvent.cpp
///
//==================================================================================

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>


// ---------------------------------------------------------------------------
// Name:        afGlobalVariableChangedEvent::afGlobalVariableChangedEvent
// Description: Constructor.
// Arguments:   variableId - The id of the global variable that was changed.
// Author:      Yaki Tebeka
// Date:        1/7/2004
// ---------------------------------------------------------------------------
afGlobalVariableChangedEvent::afGlobalVariableChangedEvent(GlobalVariableId variableId)
    : _changedVariableId(variableId)
{
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariableChangedEvent::type
// Description: Returns my transferable object type.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType afGlobalVariableChangedEvent::type() const
{
    return OS_TOBJ_ID_GLOBAL_VARIABLE_CHANGED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariableChangedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool afGlobalVariableChangedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the variable ID:
    ipcChannel << (gtInt32)_changedVariableId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariableChangedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool afGlobalVariableChangedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the variable ID:
    gtInt32 changedVariableIdAsInt32 = CHOSEN_CONTEXT_ID;
    ipcChannel >> changedVariableIdAsInt32;
    _changedVariableId = (GlobalVariableId)changedVariableIdAsInt32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariableChangedEvent::type
// Description: Returns my event type.
// Author:      Yaki Tebeka
// Date:        1/7/2004
// ---------------------------------------------------------------------------
apEvent::EventType afGlobalVariableChangedEvent::eventType() const
{
    return apEvent::APP_GLOBAL_VARIABLE_CHANGED;
}


// ---------------------------------------------------------------------------
// Name:        afGlobalVariableChangedEvent::clone
// Description: Returns a copy of self.
// Author:      Yaki Tebeka
// Date:        1/7/2004
// ---------------------------------------------------------------------------
apEvent* afGlobalVariableChangedEvent::clone() const
{
    return new afGlobalVariableChangedEvent(_changedVariableId);
}
