//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apHexChangedEvent.cpp
///
//==================================================================================

// Infra
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

/// Local:
#include <AMDTAPIClasses/Include/Events/apHexChangedEvent.h>


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        apHexChangedEvent
/// \brief Description: Default constructor, only allowed for use by osTransferableObjectCreator
/// \return
/// -----------------------------------------------------------------------------------------------
apHexChangedEvent::apHexChangedEvent()
{

}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        apHexChangedEvent
/// \brief Description: Default constructor, only allowed for use by osTransferableObjectCreator
/// \return
/// -----------------------------------------------------------------------------------------------
apHexChangedEvent::apHexChangedEvent(bool displayHex) : m_displayHex(displayHex)
{

}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        apHexChangedEvent
/// \brief Description: Destructor
/// \return
/// -----------------------------------------------------------------------------------------------
apHexChangedEvent::~apHexChangedEvent()
{
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        type
/// \brief Description: Returns my transferable object type.
/// \return osTransferableObjectType
/// -----------------------------------------------------------------------------------------------
osTransferableObjectType apHexChangedEvent::type() const
{
    return OS_TOBJ_ID_SHOW_HEX_CHANGED_EVENT;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        writeSelfIntoChannel
/// \brief Description: Write the event into the channel
/// \param[in]          ipcChannel
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool apHexChangedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    ipcChannel << m_displayHex;

    return retVal;
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        readSelfFromChannel
/// \brief Description: Reads this class data from a communication channel
/// \param[in]          ipcChannel
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool apHexChangedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    ipcChannel >> m_displayHex;

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        eventType
/// \brief Description: Return my event type
/// \return apEvent::EventType
/// -----------------------------------------------------------------------------------------------
apEvent::EventType apHexChangedEvent::eventType() const
{
    return apEvent::AP_HEX_CHANGED_EVENT;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        clone
/// \brief Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
/// \return apEvent*
/// -----------------------------------------------------------------------------------------------
apEvent* apHexChangedEvent::clone() const
{
    apHexChangedEvent* pEventCopy = new apHexChangedEvent(m_displayHex);
    return pEventCopy;
}

