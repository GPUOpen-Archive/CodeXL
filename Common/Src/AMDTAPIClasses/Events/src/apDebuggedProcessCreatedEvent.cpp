//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessCreatedEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessCreatedEvent.cpp ------------------------------

// Infra
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::apDebuggedProcessCreatedEvent
// Description: Constructor
// Arguments:   pProcessCreationData - The process creation data.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apDebuggedProcessCreatedEvent::apDebuggedProcessCreatedEvent(const apDebugProjectSettings& processCreationData,
                                                             const osTime& processCreationTime, osInstructionPointer loadedAddress)
    : _processCreationData(processCreationData), _processCreationTime(processCreationTime),
      _loadedAddress(loadedAddress), _areDebugSymbolsLoaded(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessCreatedEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessCreatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the process creation data into the channel:
    retVal = _processCreationData.writeSelfIntoChannel(ipcChannel);

    // Write the process creation time into the channel:
    ipcChannel << _processCreationTime;

    // Write the loaded address of the process exe module into the channel:
    ipcChannel << (gtUInt64)_loadedAddress;

    // Write the debug symbol loaded into the channel:
    ipcChannel << _areDebugSymbolsLoaded;

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessCreatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the process creation data from the channel:
    retVal = _processCreationData.readSelfFromChannel(ipcChannel);

    // Read the process creation time from the channel:
    ipcChannel >> _processCreationTime;

    // Read the loaded address of the process exe module from the channel:
    gtUInt64 loadedAddressAsUInt64 = 0;
    ipcChannel >> loadedAddressAsUInt64;
    _loadedAddress = (osInstructionPointer)loadedAddressAsUInt64;

    // Read the debug symbol loaded from the channel:
    ipcChannel >> _areDebugSymbolsLoaded;

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessCreatedEvent::eventType() const
{
    return apEvent::AP_DEBUGGED_PROCESS_CREATED;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessCreatedEvent::clone() const
{
    apDebuggedProcessCreatedEvent* pEventCopy = new apDebuggedProcessCreatedEvent(_processCreationData, _processCreationTime, _loadedAddress);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessCreatedEvent::apDebuggedProcessCreatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apDebuggedProcessCreatedEvent::apDebuggedProcessCreatedEvent()
{

}

