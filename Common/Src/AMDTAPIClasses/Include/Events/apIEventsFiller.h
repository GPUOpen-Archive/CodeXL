//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apIEventsFiller.h
///
//==================================================================================

//------------------------------ apIEventsFiller.h ------------------------------

#ifndef __APIEVENTSFILLER_H
#define __APIEVENTSFILLER_H


// Pre-decelerations:
class apEvent;

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apIEventsFiller
// General Description:
//   An Interface that can "fill" (add data to) debugged process events.
//   The added data is usually data that is not accessible from GRProcessDebugger.dll.
//   This interface should be inherited and implemented by a sub-class that register
//   itself as the current event filler of the process debugger (pdProcessDebugger).
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apIEventsFiller
{
public:
    virtual ~apIEventsFiller();

    // Must be implemented by sub-classes. Is called after the event was triggered,
    // but before the event reach the event observers.
    virtual void fillEvent(apEvent& event) = 0;
};

#endif //__APIEVENTSFILLER_H

