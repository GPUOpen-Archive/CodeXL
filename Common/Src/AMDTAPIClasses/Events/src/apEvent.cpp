//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apEvent.cpp
///
//==================================================================================

//------------------------------ apEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>



// ---------------------------------------------------------------------------
// Name:        apEvent::~apEvent
// Description: Virtual Destructor needed for the crt to call the
//              appropriate sub-classes destructors.
// Author:  AMD Developer Tools Team
// Date:        14/4/2004
// ---------------------------------------------------------------------------
apEvent::~apEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apEvent::isEventObject
// Description: Returns true iff this is a sub-class of apEvent.
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// Implementation notes:
//   By implementing it in apEvent, all sub-classes inherit this implementation
//   that answers "true".
// ---------------------------------------------------------------------------
bool apEvent::isEventObject() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apEvent::writeSelfIntoChannel
// Description: Write into the channel all the members of apEvent. Note that subclasses
//              must still override this function (and use this function in the
//              overriding function).
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/8/2009
// ---------------------------------------------------------------------------
bool apEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_triggeringThreadId;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apEvent::readSelfFromChannel
// Description: Read from the channel all the members of apEvent. Note that subclasses
//              must still override this function (and use this function in the
//              overriding function).
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/8/2009
// ---------------------------------------------------------------------------
bool apEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 triggeringThreadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    ipcChannel >> triggeringThreadIdAsUInt64;
    _triggeringThreadId = (osThreadId)triggeringThreadIdAsUInt64;

    return true;
}

