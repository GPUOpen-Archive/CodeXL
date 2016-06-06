//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMDIViewCreateEvent.cpp
///
//==================================================================================

//------------------------------ apMDIViewCreateEvent.cpp ------------------------------

// Infra
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>


// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::apMDIViewCreateEvent
// Description: Constructor
// Arguments:   const osFilePath& filePath
//              int viewIndex - the requested view index
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apMDIViewCreateEvent::apMDIViewCreateEvent(const gtString& createdMDIType, const osFilePath& filePath, const gtString& viewTitle, int viewIndex, int lineNumber, int programCounterIndex)
    : m_filePath1(filePath), m_viewIndex(viewIndex), m_viewTitle(viewTitle), m_lineNumber(lineNumber), m_createdMDIType(createdMDIType), m_programCounterIndex(programCounterIndex), m_pItemData(nullptr)
{
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::type
// Description: Returns my transferable object type.
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apMDIViewCreateEvent::type() const
{
    return OS_TOBJ_ID_MDI_VIEW_CREATION_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool apMDIViewCreateEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the file path into the channel:
    retVal = m_filePath1.writeSelfIntoChannel(ipcChannel);

    // Write the file path into the channel:
    retVal = m_filePath2.writeSelfIntoChannel(ipcChannel);

    // Write the view index into the channel:
    ipcChannel << (gtInt32)m_viewIndex;

    // Write the line number into the channel:
    ipcChannel << (gtInt32)m_lineNumber;

    // Write the title:
    ipcChannel << m_viewTitle;

    // Write the item data:
    ipcChannel << (gtUInt64)(m_pItemData);

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apMDIViewCreateEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the file path from the channel:
    retVal = m_filePath1.readSelfFromChannel(ipcChannel);

    // Read the file path from the channel:
    retVal = m_filePath2.readSelfFromChannel(ipcChannel);

    // Read the view index from the channel:
    gtInt32 viewIndexAsInt32 = 0;
    ipcChannel >> viewIndexAsInt32;
    m_viewIndex = (int)viewIndexAsInt32;

    // Read the view index from the channel:
    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    m_lineNumber = (int)varAsInt32;

    // Read the view title:
    ipcChannel >> m_viewTitle;

    // Write the item data:
    gtInt64 varAsInt64 = 0;
    ipcChannel >> varAsInt64;
    m_pItemData = (void*)varAsInt64;

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::eventType
// Description: Returns my event type. Notice this event should not be instantiated,
//              and handled, only its inherited classes, therefor, there is no
//              real event type for it
// Return Val:  apEvent::EventType
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apEvent::EventType apMDIViewCreateEvent::eventType() const
{
    return apEvent::AP_MDI_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apEvent* apMDIViewCreateEvent::clone() const
{
    apMDIViewCreateEvent* pEventCopy = new apMDIViewCreateEvent(m_createdMDIType, m_filePath1, m_viewTitle, m_viewIndex, m_lineNumber, m_programCounterIndex);
    pEventCopy->SetSecondFilePath(m_filePath2);
    pEventCopy->SetItemData(m_pItemData);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewCreateEvent::apMDIViewCreateEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apMDIViewCreateEvent::apMDIViewCreateEvent()
{

}

