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

    void handleUserDecision(bool shouldSkip) const { m_userChoice = (shouldSkip ? AP_USER_CHOSE_SKIP : AP_USER_CHOSE_STOP); };
    bool userDecided() const { return (AP_USER_NOT_DECIDED != m_userChoice); };
    bool shouldSkip() const { return (AP_USER_CHOSE_SKIP == m_userChoice); };

    enum apKernelDebuggingInterruptionStatus
    {
        AP_USER_NOT_DECIDED,
        AP_USER_CHOSE_SKIP,
        AP_USER_CHOSE_STOP,
    };

private:
    mutable apKernelDebuggingInterruptionStatus m_userChoice;
};



#endif //__APKERNELDEBUGGINGINTERRUPTEDEVENT_H

