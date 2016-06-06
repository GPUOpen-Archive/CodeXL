//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRenderContextDeletedEvent.cpp
///
//==================================================================================

//------------------------------ apRenderContextDeletedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::apRenderContextDeletedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that deleted the render context.
//              contextId - The OpenGL Server context id.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apRenderContextDeletedEvent::apRenderContextDeletedEvent(osThreadId triggeringThreadId, int contextId)
    : apEvent(triggeringThreadId), _contextId(contextId)
{
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::~apRenderContextDeletedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apRenderContextDeletedEvent::~apRenderContextDeletedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apRenderContextDeletedEvent::type() const
{
    return OS_TOBJ_ID_RENDER_CONTEXT_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
bool apRenderContextDeletedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenGL Server context id:
    ipcChannel << _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
bool apRenderContextDeletedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenGL Server context id:
    ipcChannel >> _contextId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apEvent::EventType apRenderContextDeletedEvent::eventType() const
{
    return apEvent::AP_RENDER_CONTEXT_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apEvent* apRenderContextDeletedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apRenderContextDeletedEvent* pEventCopy = new apRenderContextDeletedEvent(threadId, _contextId);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apRenderContextDeletedEvent::apRenderContextDeletedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        27/12/2009
// ---------------------------------------------------------------------------
apRenderContextDeletedEvent::apRenderContextDeletedEvent()
    : _contextId(0)
{
}

