//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApiConnectionEndedEvent.cpp
///
//==================================================================================

//------------------------------ apApiConnectionEndedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::apApiConnectionEndedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
apApiConnectionEndedEvent::apApiConnectionEndedEvent(apAPIConnectionType apiConnectionType)
    : _apiConnectionType(apiConnectionType)
{
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::~apApiConnectionEndedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
apApiConnectionEndedEvent::~apApiConnectionEndedEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apApiConnectionEndedEvent::type() const
{
    return OS_TOBJ_ID_API_CONNECTION_ENDED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
bool apApiConnectionEndedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt32)_apiConnectionType;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
bool apApiConnectionEndedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    gtUInt32 APIConnectionType = AP_SPIES_UTILITIES_API_CONNECTION;
    ipcChannel >> APIConnectionType;
    _apiConnectionType = (apAPIConnectionType)APIConnectionType;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::eventType
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
apEvent::EventType apApiConnectionEndedEvent::eventType() const
{
    return apEvent::AP_API_CONNECTION_ENDED;
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEndedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
apEvent* apApiConnectionEndedEvent::clone() const
{
    apApiConnectionEndedEvent* pEventCopy = new apApiConnectionEndedEvent(_apiConnectionType);
    return pEventCopy;
}

