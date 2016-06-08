//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBeforeDebuggedProcessRunResumedEvent.h
///
//==================================================================================

//------------------------------ apBeforeDebuggedProcessRunResumedEvent.h ------------------------------

#ifndef __APBEFOREDEBUGGEDPROCESSRUNRESUMEDEVENT_H
#define __APBEFOREDEBUGGEDPROCESSRUNRESUMEDEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API apBeforeDebuggedProcessRunResumedEvent
// General Description:
//   Is thrown before the debugged process run is resumed, enabling clients
//   to perform actions that should happend before the process run is resumed
//   (and before apDebuggedProcessRunResumedEvent is thrown).
//
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apBeforeDebuggedProcessRunResumedEvent : public apEvent
{
public:
    apBeforeDebuggedProcessRunResumedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};


#endif //__APBEFOREDEBUGGEDPROCESSRUNRESUMEDEVENT_H
