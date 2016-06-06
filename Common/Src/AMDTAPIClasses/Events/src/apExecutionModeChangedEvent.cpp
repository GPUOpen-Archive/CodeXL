//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apExecutionModeChangedEvent.cpp
///
//==================================================================================

//------------------------------ apExecutionModeChangedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::apExecutionModeChangedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apExecutionModeChangedEvent::apExecutionModeChangedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::apExecutionModeChangedEvent
// Description: Constructor
// Arguments:   const gtString& userWarningString
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apExecutionModeChangedEvent::apExecutionModeChangedEvent(const gtString& modeName, const int sessionTypeIndex, bool updateOnlySessionTypeIndex)
    : apEvent(), m_modeName(modeName), m_sessionTypeName(L""), m_sessionTypeIndex(sessionTypeIndex), m_updateOnlySessionTypeIndex(updateOnlySessionTypeIndex)
{
}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::apExecutionModeChangedEvent
// Description: Constructor
// Arguments:   const gtString& userWarningString
// Author:  AMD Developer Tools Team
// Date:        29/8/2012
// ---------------------------------------------------------------------------
apExecutionModeChangedEvent::apExecutionModeChangedEvent(const gtString& modeName, const gtString& sessionTypeName, const int sessionTypeIndex)
    : apEvent(), m_modeName(modeName), m_sessionTypeName(sessionTypeName), m_sessionTypeIndex(sessionTypeIndex), m_updateOnlySessionTypeIndex(false)
{
}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::apExecutionModeChangedEvent
// Description: Constructor
// Arguments:   const gtString& modeName
//              const gtString& sessionTypeName
// Author:  AMD Developer Tools Team
// Date:        23/5/2012
// ---------------------------------------------------------------------------
apExecutionModeChangedEvent::apExecutionModeChangedEvent(const gtString& modeName, const gtString& sessionTypeName)
    : apEvent(), m_modeName(modeName), m_sessionTypeName(sessionTypeName), m_sessionTypeIndex(-1), m_updateOnlySessionTypeIndex(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::apExecutionModeChangedEvent
// Description: Constructor
// Arguments:   const gtString& modeName
//              const gtString& sessionTypeName
//              const int sessionTypeIndex
//              bool updateOnlySessionTypeIndex
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        5/9/2012
// ---------------------------------------------------------------------------
apExecutionModeChangedEvent::apExecutionModeChangedEvent(const gtString& modeName, const gtString& sessionTypeName, const int sessionTypeIndex, bool updateOnlySessionTypeIndex)
    : apEvent(), m_modeName(modeName), m_sessionTypeName(sessionTypeName), m_sessionTypeIndex(sessionTypeIndex), m_updateOnlySessionTypeIndex(updateOnlySessionTypeIndex)
{

}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::~apExecutionModeChangedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/5/2012
// ---------------------------------------------------------------------------
apExecutionModeChangedEvent::~apExecutionModeChangedEvent()
{

}


// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::type
// Description: Return the event object type
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apExecutionModeChangedEvent::type() const
{
    return OS_TOBJ_ID_EXECUTION_MODE_CHANGED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
bool apExecutionModeChangedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the string:
    ipcChannel << m_modeName;
    ipcChannel << m_sessionTypeName;
    ipcChannel << (gtInt32)m_sessionTypeIndex;
    ipcChannel << m_updateOnlySessionTypeIndex;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
bool apExecutionModeChangedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the string:
    ipcChannel >> m_modeName;
    ipcChannel >> m_sessionTypeName;
    gtInt32 typeIndex;
    ipcChannel >> typeIndex;
    m_sessionTypeIndex = (int)typeIndex;
    ipcChannel >> m_updateOnlySessionTypeIndex;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apEvent::EventType apExecutionModeChangedEvent::eventType() const
{
    return apEvent::AP_EXECUTION_MODE_CHANGED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apExecutionModeChangedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        30/10/2011
// ---------------------------------------------------------------------------
apEvent* apExecutionModeChangedEvent::clone() const
{
    apExecutionModeChangedEvent* pEventCopy = new apExecutionModeChangedEvent(m_modeName, m_sessionTypeName, m_sessionTypeIndex, m_updateOnlySessionTypeIndex);
    return pEventCopy;
}


