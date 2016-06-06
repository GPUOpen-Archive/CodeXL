//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apModuleUnloadedEvent.cpp
///
//==================================================================================

//------------------------------ apModuleUnloadedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::apModuleUnloadedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that unloaded the module.
//              modulePath - The unloaded module path.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apModuleUnloadedEvent::apModuleUnloadedEvent(osThreadId triggeringThreadId, const gtString& modulePath)
    : apEvent(triggeringThreadId), _modulePath(modulePath)
{
}

// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apModuleUnloadedEvent::type() const
{
    return OS_TOBJ_ID_MODULE_UNLOADED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apModuleUnloadedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the module path:
    ipcChannel << _modulePath;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apModuleUnloadedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the module path:
    ipcChannel >> _modulePath;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apModuleUnloadedEvent::eventType() const
{
    return apEvent::AP_MODULE_UNLOADED;
}


// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apModuleUnloadedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apModuleUnloadedEvent* pEventCopy = new apModuleUnloadedEvent(threadId, _modulePath);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apModuleUnloadedEvent::apModuleUnloadedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apModuleUnloadedEvent::apModuleUnloadedEvent()
{

}

