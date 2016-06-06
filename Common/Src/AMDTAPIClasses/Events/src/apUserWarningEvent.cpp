//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apUserWarningEvent.cpp
///
//==================================================================================

//------------------------------ apUserWarningEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apUserWarningEvent.h>



// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::apUserWarningEvent
// Description: Constructor
// Arguments:   const gtString& userWarningString
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apUserWarningEvent::apUserWarningEvent(const gtString& userWarningString)
    : apEvent(), _userWarningString(userWarningString)
{
}

// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::type
// Description: Return the event object type
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apUserWarningEvent::type() const
{
    return OS_TOBJ_ID_USER_WARNING_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
bool apUserWarningEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the string:
    ipcChannel << _userWarningString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
bool apUserWarningEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the string:
    ipcChannel >> _userWarningString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apEvent::EventType apUserWarningEvent::eventType() const
{
    return apEvent::AP_USER_WARNING;
}


// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apEvent* apUserWarningEvent::clone() const
{
    apUserWarningEvent* pEventCopy = new apUserWarningEvent(_userWarningString);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apUserWarningEvent::apUserWarningEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apUserWarningEvent::apUserWarningEvent()
{

}

