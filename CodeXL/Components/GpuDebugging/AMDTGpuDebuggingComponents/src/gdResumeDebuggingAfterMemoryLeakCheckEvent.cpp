//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdResumeDebuggingAfterMemoryLeakCheckEvent.cpp
///
//==================================================================================

//------------------------------ gdResumeDebuggingAfterMemoryLeakCheckEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CodeXLAppCode/gdResumeDebuggingAfterMemoryLeakCheckEvent.h>


// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::gdResumeDebuggingAfterMemoryLeakCheckEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
gdResumeDebuggingAfterMemoryLeakCheckEvent::gdResumeDebuggingAfterMemoryLeakCheckEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::~gdResumeDebuggingAfterMemoryLeakCheckEvent
// Description: Destructor
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
gdResumeDebuggingAfterMemoryLeakCheckEvent::~gdResumeDebuggingAfterMemoryLeakCheckEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::type
// Description: Returns my transferable object type
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType gdResumeDebuggingAfterMemoryLeakCheckEvent::type() const
{
    return OS_TOBJ_ID_RESUME_DEBUGGING_AFTER_MEMORY_LEAK_CHECK_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::writeSelfIntoChannel
// Description: Writes this event into a channel
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
bool gdResumeDebuggingAfterMemoryLeakCheckEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::readSelfFromChannel
// Description: Reads this event from a channel
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
bool gdResumeDebuggingAfterMemoryLeakCheckEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::eventType
// Description: Returns my event type
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
apEvent::EventType gdResumeDebuggingAfterMemoryLeakCheckEvent::eventType() const
{
    return apEvent::APP_RESUME_DEBUGGING_AFTER_MEMORY_LEAK_CHECK;
}

// ---------------------------------------------------------------------------
// Name:        gdResumeDebuggingAfterMemoryLeakCheckEvent::clone
// Description: Create a copy of this event
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
apEvent* gdResumeDebuggingAfterMemoryLeakCheckEvent::clone() const
{
    apEvent* pEvent = new gdResumeDebuggingAfterMemoryLeakCheckEvent;
    GT_ASSERT_ALLOCATION(pEvent);
    return pEvent;
}

