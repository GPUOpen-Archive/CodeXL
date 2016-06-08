//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunSuspendedEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessRunSuspendedEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSRUNSUSPENDEDEVENT
#define __APDEBUGGEDPROCESSRUNSUSPENDEDEVENT

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apDebuggedProcessRunSuspendedEvent
// General Description:
//   Is thrown when the debugged process run was suspended.
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessRunSuspendedEvent : public apEvent
{
public:
    apDebuggedProcessRunSuspendedEvent(osThreadId triggeringThreadId = OS_NO_THREAD_ID, bool IsHostBreakpoint = false);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
    bool IsHostBreakPointReason() const;

private:
    bool _IsHostBreakpointReason;
};


#endif  // __APDEBUGGEDPROCESSRUNSUSPENDEDEVENT
