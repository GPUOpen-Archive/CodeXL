//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelSourceBreakpointsUpdatedEvent.h
///
//==================================================================================

//------------------------------ apKernelSourceBreakpointsUpdatedEvent.h ------------------------------

#ifndef __APKERNELSOURCEBREAKPOINTSUPDATEDEVENT_H
#define __APKERNELSOURCEBREAKPOINTSUPDATEDEVENT_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apKernelSourceBreakpointsUpdatedEvent : public apEvent
// General Description: Sent when kernel breakpoints are moved around
// Author:  AMD Developer Tools Team
// Creation Date:       8/5/2011
// ----------------------------------------------------------------------------------
class AP_API apKernelSourceBreakpointsUpdatedEvent : public apEvent
{
private:
    struct apKernelSourceBreakpointBinding
    {
        int _requestedLineNumber;
        int _boundLineNumber;
    };

public:
    apKernelSourceBreakpointsUpdatedEvent(osThreadId triggeringThreadID = OS_NO_THREAD_ID, oaCLProgramHandle debuggedProgramHandle = OA_CL_NULL_HANDLE);
    virtual ~apKernelSourceBreakpointsUpdatedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Accessors:
    oaCLProgramHandle debuggedProgramHandle() const {return _debuggedProgramHandle;};
    void addBreakpointBinding(int requestedLineNumber, int boundLineNumber);
    int getBreakpointBoundLineNumber(int requestedLineNumber) const;

private:
    oaCLProgramHandle _debuggedProgramHandle;
    gtVector<apKernelSourceBreakpointBinding> _updatedBreakpoints;
};

#endif //__APKERNELSOURCEBREAKPOINTSUPDATEDEVENT_H

