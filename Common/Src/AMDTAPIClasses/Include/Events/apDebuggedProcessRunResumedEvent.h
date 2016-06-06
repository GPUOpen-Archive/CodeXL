//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunResumedEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessRunResumedEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSRUNRESUMEDEVENT
#define __APDEBUGGEDPROCESSRUNRESUMEDEVENT

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API apDebuggedProcessRunResumedEvent
// General Description:
//   Is thrown when the debugged process run was resumed.
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessRunResumedEvent : public apEvent
{
public:
    apDebuggedProcessRunResumedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};


#endif  // __APDEBUGGEDPROCESSRUNRESUMEDEVENT
