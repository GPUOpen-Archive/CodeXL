//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apHexChangedEvent.h
///
//==================================================================================
#ifndef __APHEXCHANGEDEVENT_H
#define __APHEXCHANGEDEVENT_H


// Local:
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>

/// This class represents an event created when the show hex value flag had changed
class AP_API apHexChangedEvent : public apEvent
{
public:

    apHexChangedEvent(bool displayHex);
    ~apHexChangedEvent();

    bool displayHex() const {return m_displayHex;};
    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:

    // Do not allow the use of my default constructor:
    apHexChangedEvent();

    bool m_displayHex;
};

#endif //__APHEXCHANGEDEVENT_H

