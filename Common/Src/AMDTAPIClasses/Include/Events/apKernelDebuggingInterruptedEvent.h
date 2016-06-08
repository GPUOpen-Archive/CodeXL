//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelDebuggingInterruptedEvent.h
///
//==================================================================================

//------------------------------ apKernelDebuggingInterruptedEvent.h ------------------------------

#ifndef __APKERNELDEBUGGINGINTERRUPTEDEVENT_H
#define __APKERNELDEBUGGINGINTERRUPTEDEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apKernelDebuggingInterruptedEvent
// General Description: Is thrown when entering kernel debugging is interrupted by a
//                      breakpoint or other such event.
// Author:  AMD Developer Tools Team
// Creation Date:       31/5/2011
// ----------------------------------------------------------------------------------
class AP_API apKernelDebuggingInterruptedEvent : public apEvent
{
public:
    apKernelDebuggingInterruptedEvent();
    virtual ~apKernelDebuggingInterruptedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};



#endif //__APKERNELDEBUGGINGINTERRUPTEDEVENT_H

