//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaDebuggedProcessEventsFiller.h
///
//==================================================================================

//------------------------------ gaDebuggedProcessEventsFiller.h ------------------------------

#ifndef __GADEBUGGEDPROCESSEVENTSFILLER
#define __GADEBUGGEDPROCESSEVENTSFILLER

// Forward deceleration:
class apEvent;
class apBreakpointHitEvent;
class apThreadCreatedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/Events/apIEventsFiller.h>


// ----------------------------------------------------------------------------------
// Class Name:           gaDebuggedProcessEventsFiller : public apIEventsFiller
// General Description:
//   Adds data to debugged process events that is not accessible from GRProcessDebugger.dll.
//
// Author:               Yaki Tebeka
// Creation Date:        30/9/2004
// ----------------------------------------------------------------------------------
class gaDebuggedProcessEventsFiller : public apIEventsFiller
{
public:
    // Overrides apIEventsFiller:
    virtual void fillEvent(apEvent& event);

private:
    void fillBreakpointHitEvent(apBreakpointHitEvent& event);
    void fillThreadCreatedEvent(apThreadCreatedEvent& event);
    void createAndTriggerDetectedErrorEvent(const osThreadId& breakedOnThreadId);

    bool getBreakedOnFunctionDetails(gtAutoPtr<apFunctionCall>& aptrBreakedOnFunctionCall) const;
};


#endif  // __GADEBUGGEDPROCESSEVENTSFILLER
