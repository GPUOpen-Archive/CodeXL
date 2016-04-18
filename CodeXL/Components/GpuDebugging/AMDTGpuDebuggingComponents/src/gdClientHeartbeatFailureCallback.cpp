//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdClientHeartbeatFailureCallback.cpp
///
//==================================================================================

//------------------------------ gdClientHeartbeatFailureCallback.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>




// Local:
#include <AMDTGpuDebuggingComponents/Include/gdProcessDebuggerEventHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdClientHeartbeatFailureEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdClientHeartbeatFailureCallback.h>


// ---------------------------------------------------------------------------
// Name:        gdClientHearbeatFailureHandlingFunc
// Description: Is called when the client heartbeat operation fails.
//              (when this application does not manage to send heartbeat to
//               the license server)
// Arguments: hearbeatFailureCount - The successive heartbeat failures count.
// Author:      Yaki Tebeka
// Date:        3/10/2006
// ---------------------------------------------------------------------------
void gdClientHearbeatFailureHandlingFunc(int hearbeatFailureCount)
{
    // Send an appropriate event to be handled by the application GUI thread:
    // Add this event to the events handler queue:
    gdClientHeartbeatFailureEvent heartbeatFailureEvent(hearbeatFailureCount);
    gdProcessDebuggerEventHandler::instance().AddPendingEvent(heartbeatFailureEvent);
}
