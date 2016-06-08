//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGDBListenerThreadWasSuspendedEvent.h
///
//==================================================================================

//------------------------------ apGDBListenerThreadWasSuspendedEvent.h ------------------------------

#ifndef __APGDBLISTENERTHREADWASSUSPENDEDEVENT_H
#define __APGDBLISTENERTHREADWASSUSPENDEDEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGDBListenerThreadWasSuspendedEvent
// General Description:
//   Is thrown when the gdb listener thread is suspended.
//   I.E: When the debugged process is suspended by and the gdb listener thread
//        finished reading all gdb outputs printed before and at the debugged process suspension.
//
// Author:  AMD Developer Tools Team
// Creation Date:        2/1/2008
// ----------------------------------------------------------------------------------
class AP_API apGDBListenerThreadWasSuspendedEvent : public apEvent
{
public:
    apGDBListenerThreadWasSuspendedEvent();
    virtual ~apGDBListenerThreadWasSuspendedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};


#endif //__APGDBLISTENERTHREADWASSUSPENDEDEVENT_H

