//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSearchingForMemoryLeaksEvent.cpp
///
//==================================================================================

//------------------------------ apSearchingForMemoryLeaksEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apSearchingForMemoryLeaksEvent.h>

// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::apSearchingForMemoryLeaksEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        23/11/2008
// ---------------------------------------------------------------------------
apSearchingForMemoryLeaksEvent::apSearchingForMemoryLeaksEvent(const gtString& message): _message(message)
{
}

// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::~apSearchingForMemoryLeaksEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        23/11/2008
// ---------------------------------------------------------------------------
apSearchingForMemoryLeaksEvent::~apSearchingForMemoryLeaksEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apSearchingForMemoryLeaksEvent::type() const
{
    return OS_TOBJ_ID_SEARCHING_FOR_MEMORY_LEAKS_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apSearchingForMemoryLeaksEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write my message:
    ipcChannel << _message;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apSearchingForMemoryLeaksEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read my message:
    ipcChannel >> _message;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        23/11/2008
// ---------------------------------------------------------------------------
apEvent::EventType apSearchingForMemoryLeaksEvent::eventType() const
{
    return apEvent::AP_SEARCHING_FOR_MEMORY_LEAKS;
}


// ---------------------------------------------------------------------------
// Name:        apSearchingForMemoryLeaksEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        23/11/2008
// ---------------------------------------------------------------------------
apEvent* apSearchingForMemoryLeaksEvent::clone() const
{
    apSearchingForMemoryLeaksEvent* pClone = new apSearchingForMemoryLeaksEvent(_message);


    return pClone;
}
