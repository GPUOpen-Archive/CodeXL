//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRenderContextCreatedEvent.cpp
///
//==================================================================================

//------------------------------ apRenderContextCreatedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apRenderContextCreatedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::apRenderContextCreatedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that created the render context.
//              contextId - The OpenGL Server's render context id.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apRenderContextCreatedEvent::apRenderContextCreatedEvent(osThreadId triggeringThreadId, int contextId)
    : apEvent(triggeringThreadId), _contextId(contextId)
{
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::~apRenderContextCreatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apRenderContextCreatedEvent::~apRenderContextCreatedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apRenderContextCreatedEvent::type() const
{
    return OS_TOBJ_ID_RENDER_CONTEXT_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
bool apRenderContextCreatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenGL Server context id:
    ipcChannel << _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
bool apRenderContextCreatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenGL Server context id:
    ipcChannel >> _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apEvent::EventType apRenderContextCreatedEvent::eventType() const
{
    return apEvent::AP_RENDER_CONTEXT_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apEvent* apRenderContextCreatedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apRenderContextCreatedEvent* pEventCopy = new apRenderContextCreatedEvent(threadId, _contextId);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apRenderContextCreatedEvent::apRenderContextCreatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apRenderContextCreatedEvent::apRenderContextCreatedEvent()
    : _contextId(0)
{
}

