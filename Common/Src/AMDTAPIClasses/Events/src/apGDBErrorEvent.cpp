//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGDBErrorEvent.cpp
///
//==================================================================================

//------------------------------ apGDBErrorEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apGDBErrorEvent.h>


// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::apGDBOutputStringEvent
// Description: Constructor
// Arguments:   gdbErrorString - The GDB outputted string.
// Author:  AMD Developer Tools Team
// Date:        13/01/2009
// ---------------------------------------------------------------------------
apGDBErrorEvent::apGDBErrorEvent(const gtString& gdbErrorString)
    : apEvent(OS_NO_THREAD_ID), _gdbErrorString(gdbErrorString)
{

}

// ---------------------------------------------------------------------------
// Name:        apGDBErrorEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGDBErrorEvent::type() const
{
    return OS_TOBJ_ID_GDB_ERROR_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apGDBErrorEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apGDBErrorEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _gdbErrorString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGDBErrorEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apGDBErrorEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _gdbErrorString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        13/01/2009
// ---------------------------------------------------------------------------
apEvent::EventType apGDBErrorEvent::eventType() const
{
    return apEvent::AP_GDB_ERROR;
}


// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        13/01/2009
// ---------------------------------------------------------------------------
apEvent* apGDBErrorEvent::clone() const
{
    apGDBErrorEvent* pEventCopy = new apGDBErrorEvent(_gdbErrorString);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apGDBErrorEvent::apGDBErrorEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apGDBErrorEvent::apGDBErrorEvent()
{

}

