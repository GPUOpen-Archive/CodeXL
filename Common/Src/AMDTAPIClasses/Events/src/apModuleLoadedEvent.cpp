//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apModuleLoadedEvent.cpp
///
//==================================================================================

//------------------------------ apModuleLoadedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::apModuleLoadedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that loaded the module.
//              modulePath - The loaded module path.
//              loadAddress - The loaded address of the module, in debugged process address space.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apModuleLoadedEvent::apModuleLoadedEvent(osThreadId triggeringThreadId, const gtString& modulePath, osInstructionPointer loadAddress)
    : apEvent(triggeringThreadId), _modulePath(modulePath), _moduleLoadAddress(loadAddress),
      _areDebugSymbolsLoaded(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apModuleLoadedEvent::type() const
{
    return OS_TOBJ_ID_MODULE_LOADED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apModuleLoadedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the module path into the channel:
    ipcChannel << _modulePath;

    // Write the module load address into the channel:
    ipcChannel << (gtUInt64)_moduleLoadAddress;

    // Write the are symbol loaded bool into the channel:
    ipcChannel << _areDebugSymbolsLoaded;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apModuleLoadedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Write the module path into the channel:
    ipcChannel >> _modulePath;

    // Write the module load address into the channel:
    gtUInt64 moduleLoadAddressAsUInt64 = 0;
    ipcChannel >> moduleLoadAddressAsUInt64;
    _moduleLoadAddress = (osInstructionPointer)moduleLoadAddressAsUInt64;

    // Write the are symbol loaded bool into the channel:
    ipcChannel >> _areDebugSymbolsLoaded;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apModuleLoadedEvent::eventType() const
{
    return apEvent::AP_MODULE_LOADED;
}


// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apModuleLoadedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apModuleLoadedEvent* pEventCopy = new apModuleLoadedEvent(threadId, _modulePath, _moduleLoadAddress);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apModuleLoadedEvent::apModuleLoadedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apModuleLoadedEvent::apModuleLoadedEvent():
    _moduleLoadAddress(NULL), _areDebugSymbolsLoaded(false)
{

}

