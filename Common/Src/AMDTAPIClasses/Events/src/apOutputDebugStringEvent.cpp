//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOutputDebugStringEvent.cpp
///
//==================================================================================

//------------------------------ apOutputDebugStringEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>


// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::apOutputDebugStringEvent
// Description: Constructor
// Arguments:  triggeringThreadId - The id of the thread that output the debug string.
//             debugString - The outputted debug string.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apOutputDebugStringEvent::apOutputDebugStringEvent(osThreadId triggeringThreadId, const gtString& debugString, const int targetView)
    : apEvent(triggeringThreadId), _debugString(debugString), m_targetView(targetView)
{
}

// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apOutputDebugStringEvent::type() const
{
    return OS_TOBJ_ID_OUTPUT_DEBUG_STRING_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apOutputDebugStringEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the string:
    ipcChannel << _debugString;

    // write the target view:
    ipcChannel << m_targetView;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apOutputDebugStringEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the string:
    ipcChannel >> _debugString;

    // Read the target View
    ipcChannel >> m_targetView;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apOutputDebugStringEvent::eventType() const
{
    return apEvent::AP_OUTPUT_DEBUG_STRING;
}


// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apOutputDebugStringEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apOutputDebugStringEvent* pEventCopy = new apOutputDebugStringEvent(threadId, _debugString, m_targetView);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apOutputDebugStringEvent::apOutputDebugStringEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apOutputDebugStringEvent::apOutputDebugStringEvent()
{

}

