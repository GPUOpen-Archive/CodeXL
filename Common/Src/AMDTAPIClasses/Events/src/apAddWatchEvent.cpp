//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAddWatchEvent.cpp
///
//==================================================================================

// Infra
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

/// Local:
#include <AMDTAPIClasses/Include/Events/apAddWatchEvent.h>


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        apAddWatchEvent
/// \brief Description: Default constructor, only allowed for use by osTransferableObjectCreator
/// \return
/// -----------------------------------------------------------------------------------------------
apAddWatchEvent::apAddWatchEvent()
{

}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        apAddWatchEvent
/// \brief Description: Constructor
/// \param[in]          watchExpression
/// \param[in]          isMultiwatch
/// \return
/// -----------------------------------------------------------------------------------------------
apAddWatchEvent::apAddWatchEvent(const gtString& watchExpression, bool isMultiwatch) :
    m_watchExpression(watchExpression), m_isMultiwatch(isMultiwatch)
{
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        type
/// \brief Description: Returns my transferable object type.
/// \return osTransferableObjectType
/// -----------------------------------------------------------------------------------------------
osTransferableObjectType apAddWatchEvent::type() const
{
    return OS_TOBJ_ID_MDI_ADD_WATCH_EVENT;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        writeSelfIntoChannel
/// \brief Description: Write the event into the channel
/// \param[in]          ipcChannel
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool apAddWatchEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the watch expression:
    ipcChannel << m_watchExpression;

    // Write the debug flag:
    ipcChannel << m_isMultiwatch;


    return retVal;
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        readSelfFromChannel
/// \brief Description: Reads this class data from a communication channel
/// \param[in]          ipcChannel
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool apAddWatchEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the watch expression:
    ipcChannel >> m_watchExpression;

    // Read the multi watch flag:
    ipcChannel >> m_isMultiwatch;

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        eventType
/// \brief Description: Return my event type
/// \return apEvent::EventType
/// -----------------------------------------------------------------------------------------------
apEvent::EventType apAddWatchEvent::eventType() const
{
    return apEvent::AP_ADD_WATCH_EVENT;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        clone
/// \brief Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
/// \return apEvent*
/// -----------------------------------------------------------------------------------------------
apEvent* apAddWatchEvent::clone() const
{
    apAddWatchEvent* pEventCopy = new apAddWatchEvent(m_watchExpression, m_isMultiwatch);
    return pEventCopy;
}

