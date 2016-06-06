//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMDIViewActivatedEvent.cpp
///
//==================================================================================

//------------------------------ apMDIViewActivatedEvent.cpp ------------------------------

// Infra
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apMDIViewActivatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::apMDIViewActivatedEvent
// Description: Constructor
// Arguments:   const osFilePath& filePath
//              int viewIndex - the requested view index
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apMDIViewActivatedEvent::apMDIViewActivatedEvent(const osFilePath& filePath)
    : m_filePath(filePath)
{
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::type
// Description: Returns my transferable object type.
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apMDIViewActivatedEvent::type() const
{
    return OS_TOBJ_ID_MDI_VIEW_CREATION_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Arguments:   osChannel& ipcChannel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool apMDIViewActivatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the file path into the channel:
    retVal = m_filePath.writeSelfIntoChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apMDIViewActivatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the file path from the channel:
    retVal = m_filePath.readSelfFromChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::eventType
// Description: Returns my event type. Notice this event should not be instantiated,
//              and handled, only its inherited classes, therefor, there is no
//              real event type for it
// Return Val:  apEvent::EventType
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apEvent::EventType apMDIViewActivatedEvent::eventType() const
{
    return apEvent::AP_MDI_ACTIVATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apEvent* apMDIViewActivatedEvent::clone() const
{
    apMDIViewActivatedEvent* pEventCopy = new apMDIViewActivatedEvent(m_filePath);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apMDIViewActivatedEvent::apMDIViewActivatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        23/8/2011
// ---------------------------------------------------------------------------
apMDIViewActivatedEvent::apMDIViewActivatedEvent()
{

}

