//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAfterKernelDebuggingEvent.h
///
//==================================================================================

//------------------------------ apAfterKernelDebuggingEvent.h ------------------------------

#ifndef __APAFTERKERNELDEBUGGINGEVENT_H
#define __APAFTERKERNELDEBUGGINGEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apAfterKernelDebuggingEvent : public apEvent
// General Description: An event invoked by the spy after it exits kernel debugging
//                      mode. This event should be followed by triggering a breakpoint.
// Author:  AMD Developer Tools Team
// Creation Date:       28/10/2010
// ----------------------------------------------------------------------------------
class AP_API apAfterKernelDebuggingEvent : public apEvent
{
public:
    apAfterKernelDebuggingEvent(osThreadId triggeringThreadID = OS_NO_THREAD_ID);
    ~apAfterKernelDebuggingEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};

#endif //__APAFTERKERNELDEBUGGINGEVENT_H

