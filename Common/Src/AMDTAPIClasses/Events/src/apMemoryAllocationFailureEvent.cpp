//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMemoryAllocationFailureEvent.cpp
///
//==================================================================================

//------------------------------ apMemoryAllocationFailureEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apMemoryAllocationFailureEvent.h>

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::apMemoryAllocationFailureEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
apMemoryAllocationFailureEvent::apMemoryAllocationFailureEvent(osCallStack callStack) : m_callStack(callStack)
{
}

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::apMemoryAllocationFailureEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
apMemoryAllocationFailureEvent::apMemoryAllocationFailureEvent()
{
}

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::~apMemoryAllocationFailureEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
apMemoryAllocationFailureEvent::~apMemoryAllocationFailureEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
osTransferableObjectType apMemoryAllocationFailureEvent::type() const
{
    return OS_TOBJ_ID_MEMORY_ALLOCATION_FAILURE_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
bool apMemoryAllocationFailureEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write my progress:
    bool retVal = m_callStack.writeSelfIntoChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal &= apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
bool apMemoryAllocationFailureEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read my progress:
    bool retVal = m_callStack.readSelfFromChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal &= apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
apEvent::EventType apMemoryAllocationFailureEvent::eventType() const
{
    return apEvent::AP_MEMORY_ALLOCATION_FAILURE_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryAllocationFailureEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/5/2015
// ---------------------------------------------------------------------------
apEvent* apMemoryAllocationFailureEvent::clone() const
{
    apMemoryAllocationFailureEvent* pClone = new apMemoryAllocationFailureEvent(m_callStack);

    return pClone;
}
