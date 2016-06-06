//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSearchingForMemoryLeaksEvent.h
///
//==================================================================================

//------------------------------ apSearchingForMemoryLeaksEvent.h ------------------------------

#ifndef __APSEARCHINGFORMEMORYLEAKSEVENT_H
#define __APSEARCHINGFORMEMORYLEAKSEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apSearchingForMemoryLeaksEvent
// General Description: This event is sent from the memory viewer to the debugged
//                      process events view when a memory leak occurs
// Author:  AMD Developer Tools Team
// Creation Date:        2/11/2008
// ----------------------------------------------------------------------------------
class AP_API apSearchingForMemoryLeaksEvent : public apEvent
{
public:
    apSearchingForMemoryLeaksEvent(const gtString& message = L"");
    ~apSearchingForMemoryLeaksEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    const gtString& message() const {return _message;};

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    gtString _message;

};

#endif //__APSEARCHINGFORMEMORYLEAKSEVENT_H

