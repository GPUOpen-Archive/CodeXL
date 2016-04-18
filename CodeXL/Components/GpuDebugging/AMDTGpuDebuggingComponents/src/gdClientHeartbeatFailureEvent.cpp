//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdClientHeartbeatFailureEvent.cpp
///
//==================================================================================

//------------------------------ gdClientHeartbeatFailureEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// wxWindows pre-compiled header:


// Local:
#include <AMDTGpuDebuggingComponents/Include/gdClientHeartbeatFailureEvent.h>

// Generate a unique id for this wxWindows event:
wxEventType gdEVT_CLIENT_HEARTBEAT_FAILURE_EVENT = wxNewEventType();


// ---------------------------------------------------------------------------
// Name:        gdClientHeartbeatFailureEvent::gdClientHeartbeatFailureEvent
// Description: Constructor - sets my event type to be gdEVT_CLIENT_HEARTBEAT_FAILURE_EVENT
// Arguments: hearbeatFailureCount - The amount of successive heartbeat failures count.
// Author:      Yaki Tebeka
// Date:        30/10/2006
// ---------------------------------------------------------------------------
gdClientHeartbeatFailureEvent::gdClientHeartbeatFailureEvent(int hearbeatFailureCount)
    : wxEvent(0, gdEVT_CLIENT_HEARTBEAT_FAILURE_EVENT), _hearbeatFailureCount(hearbeatFailureCount)
{
}


// ---------------------------------------------------------------------------
// Name:        gdClientHeartbeatFailureEvent::Clone
// Description: Returns a new copy of self. It is the caller responsibility to
//              delete this copy.
// Author:      Yaki Tebeka
// Date:        30/10/2006
// ---------------------------------------------------------------------------
wxEvent* gdClientHeartbeatFailureEvent::Clone() const
{
    wxEvent* retVal = new gdClientHeartbeatFailureEvent(_hearbeatFailureCount);
    GT_ASSERT(retVal);

    return retVal;
}

