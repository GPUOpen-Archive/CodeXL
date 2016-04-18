//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaDebuggedProcessEventsFiller.cpp
///
//==================================================================================

//------------------------------ gaDebuggedProcessEventsFiller.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local:
#include <src/gaPrivateAPIFunctions.h>
#include <src/gaDebuggedProcessEventsFiller.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// ---------------------------------------------------------------------------
// Name:        gaDebuggedProcessEventsFiller::fillEvent
// Description: Inputs a debugged process event, and "fills" it with data that
//              is not accessible from GRProcessDebugger.dll
// Author:      Yaki Tebeka
// Date:        30/9/2004
// ---------------------------------------------------------------------------
void gaDebuggedProcessEventsFiller::fillEvent(apEvent& event)
{
    // Get the event type:
    apEvent::EventType eventType = event.eventType();

    switch (eventType)
    {
        case apEvent::AP_BREAKPOINT_HIT:
            fillBreakpointHitEvent((apBreakpointHitEvent&)event);
            break;

        case apEvent::AP_THREAD_CREATED:
            fillThreadCreatedEvent((apThreadCreatedEvent&)event);
            break;

        default:
            // An events that does not need additional data.
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaDebuggedProcessEventsFiller::fillBreakpointHitEvent
// Description:
//   Adds the following to breakpoint hit events:
//   - Break reason.
//   - Breaked on monitored function id.
//   - OpenGL error GLenum.
// Arguments:   event - The input breakpoint hit event.
// Author:      Yaki Tebeka
// Date:        30/9/2004
// ---------------------------------------------------------------------------
void gaDebuggedProcessEventsFiller::fillBreakpointHitEvent(apBreakpointHitEvent& event)
{
    // Get the break reason:
    apBreakReason breakReason = AP_FOREIGN_BREAK_HIT;
    bool rc = gaGetBreakReason(breakReason);

    if (rc)
    {
        // Fill the break reason:
        event.setBreakReason(breakReason);

        // If this is a CodeXL generated break:
        if ((AP_FOREIGN_BREAK_HIT != breakReason) && (!gaIsHostBreakPoint()))
        {
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
            {
                // Get the id of the thread that triggered the
                const osThreadId& breakedOnThreadId = event.triggeringThreadId();

                // Get the details of the function that triggered the breakpoint event:
                gtAutoPtr<apFunctionCall> aptrBreakedOnFunctionCall;
                rc = getBreakedOnFunctionDetails(aptrBreakedOnFunctionCall);

                if (rc)
                {
                    // Fill the breaked on function details:
                    event.setBreakedOnFunctionCall(aptrBreakedOnFunctionCall);
                }

                // If the break was caused by an OpenGL error:
                if (breakReason == AP_OPENGL_ERROR_BREAKPOINT_HIT)
                {
                    // Get the OpenGL error code:
                    GLenum openGLError = GL_NO_ERROR;
                    rc = gaGetCurrentOpenGLError(openGLError);

                    if (rc)
                    {
                        // Fill the OpenGL error code:
                        event.setOpenGLErrorCode(openGLError);
                    }
                }
                else if (breakReason == AP_DETECTED_ERROR_BREAKPOINT_HIT)
                {
                    // The breakpoint was caused by a detected error:
                    createAndTriggerDetectedErrorEvent(breakedOnThreadId);
                }
                // If the break was caused by an OpenCL error:
                else if (breakReason == AP_OPENCL_ERROR_BREAKPOINT_HIT)
                {
                    // Do nothing (the OpenCL error details are brought up when reporting the OpenCL error itself):
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaDebuggedProcessEventsFiller::fillThreadCreatedEvent
// Description: Fills the details of the name, module, source code, etc
//              of the function that started the thread run.
// Arguments:   event - The event to be filled.
// Author:      Yaki Tebeka
// Date:        9/5/2005
// Implementation Notes:
//   We fill these details here, since we would like to use the debug symbols
//   engine only from the application thread.
// ---------------------------------------------------------------------------
void gaDebuggedProcessEventsFiller::fillThreadCreatedEvent(apThreadCreatedEvent& event)
{
    // Get the process debugger instance:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
    theProcessDebugger.fillThreadCreatedEvent(event);
}


// ---------------------------------------------------------------------------
// Name:        gaDebuggedProcessEventsFiller::getBreakedOnFunctionDetails
// Description: Returns the details of the function that triggered the breakpoint
//              event.
// Arguments - breakedOnThreadId - The id of the thread that caused the breakpoint
//                                 event.
//             aptrBreakedOnFunctionCall - A objects that contains the function call
//                                         details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/10/2004
// ---------------------------------------------------------------------------
bool gaDebuggedProcessEventsFiller::getBreakedOnFunctionDetails(gtAutoPtr<apFunctionCall>& aptrBreakedOnFunctionCall) const
{
    bool retVal = false;

    // Get the context which triggered the process suspension:
    apContextID currentContextId;
    bool rc = gaGetBreakpointTriggeringContextId(currentContextId);

    if (rc)
    {
        // Get the last function call executed in the queried context:
        retVal = gaGetLastFunctionCall(currentContextId, aptrBreakedOnFunctionCall);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaDebuggedProcessEventsFiller::createAndTriggerDetectedErrorEvent
// Description: Creates and triggers a detected error event.
// Arguments: breakedOnThreadId - The id of the thread that generated the
//                                detected error.
// Author:      Yaki Tebeka
// Date:        8/10/2007
// ---------------------------------------------------------------------------
void gaDebuggedProcessEventsFiller::createAndTriggerDetectedErrorEvent(const osThreadId& breakedOnThreadId)
{
    // Get the detected error parameters:
    apDetectedErrorParameters detectedErrorParameters;
    bool rc1 = gaGetDetectedErrorParameters(detectedErrorParameters);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Trigger the debugged process error event:
        apDebuggedProcessDetectedErrorEvent eve(breakedOnThreadId, detectedErrorParameters, true);
        bool rc2 = apEventsHandler::instance().handleDebugEvent(eve);
        GT_ASSERT(rc2);
    }
}

