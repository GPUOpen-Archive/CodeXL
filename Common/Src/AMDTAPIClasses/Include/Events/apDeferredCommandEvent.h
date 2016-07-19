//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDeferredCommandEvent.h
///
//==================================================================================

//------------------------------ apDeferredCommandEvent.h ------------------------------

#ifndef __apDeferredCommandEvent_H
#define __apDeferredCommandEvent_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apDeferredCommandEvent : public apEvent
// General Description: An event used to enqueue a command from inside another event handler
//                      This event is used when kernel debugging is interrupted. If the user
//                      elects to skip all breakpoints and continue execution then this event
//                      is used to enqueue the 'Resume Debugged Process' command.
// Author:  AMD Developer Tools Team
// Creation Date:       Jan-29, 2015
// ----------------------------------------------------------------------------------
class AP_API apDeferredCommandEvent : public apEvent
{
public:
    enum apDeferredCommand
    {
        AP_DEFERRED_COMMAND_RESUME_DEBUGGED_PROCESS,
        AP_DEFERRED_COMMAND_INTERNAL_HOST_STEP,
        AP_DEFERRED_COMMAND_INVALID_COMMAND,
        AP_DEFERRED_COMMAND_LOCK_THREAD_CONTROL,
        AP_DEFERRED_COMMAND_UNLOCK_THREAD_CONTROL,
    };

    enum apDeferredCommandTarget
    {
        AP_GW_EVENT_OBSERVER,
        AP_VSP_EVENT_OBSERVER,
        AP_VSD_PROCESS_DEBUGGER,
        AP_UNKNOWN_COMMAND_TARGET
    };

    typedef void (*apDeferredCommandDataReleaser)(void* pData);
    typedef void* (*apDeferredCommandDataCloner)(const void* pData);

public:
    apDeferredCommandEvent(apDeferredCommand command = AP_DEFERRED_COMMAND_INVALID_COMMAND, apDeferredCommandTarget target = AP_UNKNOWN_COMMAND_TARGET);
    ~apDeferredCommandEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    apDeferredCommand command() const { return m_command; };
    apDeferredCommandTarget commandTarget() const { return m_target; };

    const void* getData() const { return m_pvData; };
    void clearData();
    void setData(const void* pvData, apDeferredCommandDataReleaser pfnReleaser = nullptr, apDeferredCommandDataCloner pfnCloner = nullptr);

private:
    apDeferredCommandEvent& operator=(const apDeferredCommandEvent& other) = delete;
    apDeferredCommandEvent& operator=(apDeferredCommandEvent&& other) = delete;
    apDeferredCommandEvent(const apDeferredCommandEvent& other) = delete;
    apDeferredCommandEvent(apDeferredCommandEvent&& other) = delete;

    apDeferredCommand m_command;
    apDeferredCommandTarget m_target;
    void* m_pvData;
    apDeferredCommandDataCloner m_pfnDataCloner;
    apDeferredCommandDataReleaser m_pfnDataReleaser;
};

#endif //__apDeferredCommandEvent_H

