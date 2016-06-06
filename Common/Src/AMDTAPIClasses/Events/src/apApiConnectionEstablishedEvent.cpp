//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApiConnectionEstablishedEvent.cpp
///
//==================================================================================

//------------------------------ apApiConnectionEstablishedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::apApiConnectionEstablishedEvent
// Description: Constructor
// Arguments: APIConnectionType - The type of the API connection that was established.
// Author:  AMD Developer Tools Team
// Date:        4/10/2004
// ---------------------------------------------------------------------------
apApiConnectionEstablishedEvent::apApiConnectionEstablishedEvent(apAPIConnectionType APIConnectionType)
    : _APIConnectionType(APIConnectionType)
{
}


// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::~apApiConnectionEstablishedEvent
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        4/10/2004
// ---------------------------------------------------------------------------
apApiConnectionEstablishedEvent::~apApiConnectionEstablishedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apApiConnectionEstablishedEvent::type() const
{
    return OS_TOBJ_ID_API_CONNECTION_ESTABLISHED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apApiConnectionEstablishedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt32)_APIConnectionType;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apApiConnectionEstablishedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    gtUInt32 APIConnectionType = AP_SPIES_UTILITIES_API_CONNECTION;
    ipcChannel >> APIConnectionType;
    _APIConnectionType = (apAPIConnectionType)APIConnectionType;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::eventType
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        4/10/2004
// ---------------------------------------------------------------------------
apEvent::EventType apApiConnectionEstablishedEvent::eventType() const
{
    return apEvent::AP_API_CONNECTION_ESTABLISHED;
}

// ---------------------------------------------------------------------------
// Name:        apApiConnectionEstablishedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        4/10/2004
// ---------------------------------------------------------------------------
apEvent* apApiConnectionEstablishedEvent::clone() const
{
    apApiConnectionEstablishedEvent* pEventCopy = new apApiConnectionEstablishedEvent(_APIConnectionType);
    return pEventCopy;
}


