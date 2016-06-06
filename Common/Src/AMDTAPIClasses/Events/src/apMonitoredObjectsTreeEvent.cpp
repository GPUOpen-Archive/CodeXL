//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMonitoredObjectsTreeEvent.cpp
///
//==================================================================================

//------------------------------ apMonitoredObjectsTreeSelectedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::apMonitoredObjectsTreeSelectedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apMonitoredObjectsTreeSelectedEvent::apMonitoredObjectsTreeSelectedEvent(const void* pTreeItemData): _pTreeItemData(pTreeItemData)
{
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::~apMonitoredObjectsTreeSelectedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apMonitoredObjectsTreeSelectedEvent::~apMonitoredObjectsTreeSelectedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apMonitoredObjectsTreeSelectedEvent::type() const
{
    return OS_TOBJ_ID_MONITORED_OBJECT_TREE_SELECTED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
bool apMonitoredObjectsTreeSelectedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
bool apMonitoredObjectsTreeSelectedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apEvent::EventType apMonitoredObjectsTreeSelectedEvent::eventType() const
{
    return apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeSelectedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apEvent* apMonitoredObjectsTreeSelectedEvent::clone() const
{
    apMonitoredObjectsTreeSelectedEvent* pClone = new apMonitoredObjectsTreeSelectedEvent(_pTreeItemData);


    return pClone;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::apMonitoredObjectsTreeActivatedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apMonitoredObjectsTreeActivatedEvent::apMonitoredObjectsTreeActivatedEvent(const void* pTreeItemData): _pTreeItemData(pTreeItemData)
{
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::~apMonitoredObjectsTreeActivatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apMonitoredObjectsTreeActivatedEvent::~apMonitoredObjectsTreeActivatedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apMonitoredObjectsTreeActivatedEvent::type() const
{
    return OS_TOBJ_ID_MONITORED_OBJECT_TREE_ACTIVATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
bool apMonitoredObjectsTreeActivatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
bool apMonitoredObjectsTreeActivatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apEvent::EventType apMonitoredObjectsTreeActivatedEvent::eventType() const
{
    return apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredObjectsTreeActivatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
apEvent* apMonitoredObjectsTreeActivatedEvent::clone() const
{
    apMonitoredObjectsTreeActivatedEvent* pClone = new apMonitoredObjectsTreeActivatedEvent(_pTreeItemData);


    return pClone;
}
