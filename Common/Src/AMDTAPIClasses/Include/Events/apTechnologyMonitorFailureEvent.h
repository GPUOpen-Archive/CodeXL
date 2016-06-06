//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apTechnologyMonitorFailureEvent.h
///
//==================================================================================

//------------------------------ apTechnologyMonitorFailureEvent.h ------------------------------

#ifndef __APTECHNOLOGYMONITORFAILUREEVENT_H
#define __APTECHNOLOGYMONITORFAILUREEVENT_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apTechnologyMonitorFailureEvent : public apEvent
// General Description: An event created by the spy when a technology monitor fails
// Author:  AMD Developer Tools Team
// Creation Date:       16/2/2014
// ----------------------------------------------------------------------------------
class AP_API apTechnologyMonitorFailureEvent : public apEvent
{
public:
    apTechnologyMonitorFailureEvent(const gtString& failInformation = L"", osThreadId triggeringThreadID = OS_NO_THREAD_ID);
    ~apTechnologyMonitorFailureEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    const gtString& failInformation() const { return m_failInformation; };

private:
    gtString m_failInformation;
};

#endif //__APTECHNOLOGYMONITORFAILUREEVENT_H

