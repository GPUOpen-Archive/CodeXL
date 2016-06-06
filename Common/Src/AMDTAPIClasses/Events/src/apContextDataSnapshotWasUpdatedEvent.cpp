//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apContextDataSnapshotWasUpdatedEvent.cpp
///
//==================================================================================

//------------------------------ apContextDataSnapshotWasUpdatedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apContextDataSnapshotWasUpdatedEvent.h>

// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::apContextDataSnapshotWasUpdatedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apContextDataSnapshotWasUpdatedEvent::apContextDataSnapshotWasUpdatedEvent(apContextID updatedContextID, bool isContextBeingDeleted):
    _updatedContextID(updatedContextID), _isContextBeingDeleted(isContextBeingDeleted)
{
}


// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::~apContextDataSnapshotWasUpdatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apContextDataSnapshotWasUpdatedEvent::~apContextDataSnapshotWasUpdatedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apContextDataSnapshotWasUpdatedEvent::type() const
{
    return OS_TOBJ_ID_CONTEXT_UPDATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool apContextDataSnapshotWasUpdatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the context ID:
    _updatedContextID.writeSelfIntoChannel(ipcChannel);

    ipcChannel << _isContextBeingDeleted;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool apContextDataSnapshotWasUpdatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the context ID:
    _updatedContextID.readSelfFromChannel(ipcChannel);

    ipcChannel >> _isContextBeingDeleted;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apEvent::EventType apContextDataSnapshotWasUpdatedEvent::eventType() const
{
    return apEvent::AP_CONTEXT_UPDATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apContextDataSnapshotWasUpdatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        19/7/2010
// ---------------------------------------------------------------------------
apEvent* apContextDataSnapshotWasUpdatedEvent::clone() const
{
    // Create a new event:
    apContextDataSnapshotWasUpdatedEvent* pEventCopy = new apContextDataSnapshotWasUpdatedEvent(_updatedContextID, _isContextBeingDeleted);

    return pEventCopy;
}
