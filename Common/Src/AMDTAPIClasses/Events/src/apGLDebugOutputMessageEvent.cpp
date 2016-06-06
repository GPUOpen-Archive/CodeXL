//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDebugOutputMessageEvent.cpp
///
//==================================================================================

//------------------------------ apGLDebugOutputMessageEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apGLDebugOutputMessageEvent.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::apGLDebugOutputMessageEvent
// Description: Constructor
// Arguments:   const gtString& debugOutputCategory
//              const gtString& debugOutputMessageContent
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
apGLDebugOutputMessageEvent::apGLDebugOutputMessageEvent(osThreadId triggeringThreadId, const gtString& debugOutputSource, const gtString& debugOutputType, const gtString& debugOutputSeverity, const gtString& debugOutputMessageContent, int contextId, int messageId)
    : apEvent(triggeringThreadId), m_contextId(contextId), m_messageId(messageId),
      m_debugOutputSource(debugOutputSource), m_debugOutputType(debugOutputType), m_debugOutputSeverity(debugOutputSeverity), m_debugOutputMessageContent(debugOutputMessageContent)
{
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::~apGLDebugOutputMessageEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
apGLDebugOutputMessageEvent::~apGLDebugOutputMessageEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apGLDebugOutputMessageEvent::type() const
{
    return OS_TOBJ_ID_GL_DEBUG_OUTPUT_MESSAGE_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool apGLDebugOutputMessageEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << m_debugOutputSource;
    ipcChannel << m_debugOutputType;
    ipcChannel << m_debugOutputSeverity;
    ipcChannel << m_debugOutputMessageContent;
    ipcChannel << (gtInt32)m_contextId;
    ipcChannel << (gtInt32)m_messageId;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool apGLDebugOutputMessageEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> m_debugOutputSource;
    ipcChannel >> m_debugOutputType;
    ipcChannel >> m_debugOutputSeverity;
    ipcChannel >> m_debugOutputMessageContent;
    int idAsInt32 = 0;
    ipcChannel >> idAsInt32;
    m_contextId = (int)idAsInt32;

    ipcChannel >> idAsInt32;
    m_messageId = (int)idAsInt32;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
apEvent::EventType apGLDebugOutputMessageEvent::eventType() const
{
    return apEvent::AP_GL_DEBUG_OUTPUT_MESSAGE;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
apEvent* apGLDebugOutputMessageEvent::clone() const
{
    apGLDebugOutputMessageEvent* pEventCopy = new apGLDebugOutputMessageEvent(triggeringThreadId(), m_debugOutputSource, m_debugOutputType, m_debugOutputSeverity, m_debugOutputMessageContent, m_contextId, m_messageId);

    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputMessageEvent::apGLDebugOutputMessageEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
apGLDebugOutputMessageEvent::apGLDebugOutputMessageEvent()
    : m_contextId(0), m_messageId(-1)
{
}
