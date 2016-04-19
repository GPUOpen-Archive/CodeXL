//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLWrappersAidMacros.h
///
//==================================================================================

//------------------------------ csOpenCLWrappersAidMacros.h ------------------------------

#ifndef __CSOPENCLWRAPPERSAIDMACROS_H
#define __CSOPENCLWRAPPERSAIDMACROS_H

// Uri, 22/8/13 - disabling the command monitoring mechanism, since we currently do not show event profiling information
// for enqueued commands in the debugger.

// Leaving this code re-accssible in here in case it becomes relevant
// #define CS_USE_ENQUEUED_COMMANDS_MONITORING 1

#ifdef CS_USE_ENQUEUED_COMMANDS_MONITORING

// ---------------------------------------------------------------------------
// Name:        CS_BEFORE_ENQUEUEING_COMMAND
// Description: Between the AddFunctionCall() and the real function call of
//              clEnqueueXXXXX
// Arguments:   out pEventVarName - will be defined as cl_event* and will get assigned
//                  eventVarName if it non-null, or a new cl_event if it isn't.
//              in eventVarName - the "event" parameter of the command, as specified
//                  by the user (of type cl_event).
//              out wasEventCreatedBySpyVarName - will be defined as a bool and get true
//                  if we assigned pEventVarName ourselves.
//              out pContextMonitorVarName - will be defined as csContextMonitor*
//                  and get the context holding the queue.
//              in queueHandle - the queue handle (of type cl_command_queue)
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
#define CS_BEFORE_ENQUEUEING_COMMAND(pEventVarName, eventVarName, wasEventCreatedBySpyVarName, pContextMonitorVarName, queueHandle)                 \
    csContextMonitor* pContextMonitorVarName = NULL;                                                                                                \
    cl_event* pEventVarName = eventVarName;                                                                                                         \
    bool wasEventCreatedBySpyVarName = false;                                                                                                       \
    csCommandQueueMonitor* pCommandQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)queueHandle);               \
    GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)                                                                                                     \
    {                                                                                                                                               \
        pCommandQueueMtr->beforeCommandAddedToQueue();                                                                                              \
        \
        int contextId = pCommandQueueMtr->contextSpyId();                                                                                           \
        pContextMonitorVarName = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);                                                         \
        \
        /* Make sure we get an event if the user didn't request one.*/                                                                              \
        if ((pEventVarName == NULL) && cs_stat_openCLMonitorInstance.isCommandQueueProfileModeForcedForQueue((oaCLCommandQueueHandle)queueHandle))  \
        {                                                                                                                                           \
            pEventVarName = new cl_event;                                                                                                           \
            *pEventVarName = (cl_event)OA_CL_NULL_HANDLE;                                                                                           \
            wasEventCreatedBySpyVarName = true;                                                                                                     \
        }                                                                                                                                           \
    }


// ---------------------------------------------------------------------------
// Name:        CS_BEFORE_ENQUEUEING_COMMAND_NO_EVENT
// Description: Between the AddFunctionCall() and the real function call of
//              clEnqueueXXXXX
//              out pContextMonitorVarName - will be defined as csContextMonitor*
//                  and get the context holding the queue.
//              in queueHandle - the queue handle (of type cl_command_queue)
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
#define CS_BEFORE_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitorVarName, queueHandle)                                              \
    csContextMonitor* pContextMonitorVarName = NULL;                                                                                \
    csCommandQueueMonitor* pQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)queueHandle);      \
    GT_IF_WITH_ASSERT(pQueueMtr != NULL)                                                                                            \
    {                                                                                                                               \
        pQueueMtr->beforeCommandAddedToQueue();                                                                                     \
        \
        int contextId = pQueueMtr->contextSpyId();                                                                                  \
        pContextMonitorVarName = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);                                         \
    }


// ---------------------------------------------------------------------------
// Name:        CS_AFTER_ENQUEUEING_COMMAND
// Description: After the real function call of clEnqueueXXXXX
// Arguments:   in pEventVarName - the event parameter passed to the real function
//                  (of type cl_event)
//              in wasEventCreatedBySpyVarName - was the pEventVarName assigned by us?
//                  (of type bool)
//              in pContextMonitorVarName - the context holding the queue.
//                  (of type csContextMonitor*)
//              in queueHandle - the queue handle (of type cl_command_queue)
//              newCommandAssignment - an expression of format
//                  new [apCLEnqueuedCommand subclass](param list) to assign
//                  the new command object if needed.
//              retValVarName - the return value of the function or the error code ret
//                  for functions that return a handle (of type cl_int).
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
#define CS_AFTER_ENQUEUEING_COMMAND(pEventVarName, wasEventCreatedBySpyVarName, pContextMonitorVarName, queueHandle, retValVarName, newCommandAssignment)   \
    if (pContextMonitorVarName != NULL)                                                                                                     \
    {                                                                                                                                       \
        /* Add the command to the queue:*/                                                                                                  \
        gtAutoPtr<apCLEnqueuedCommand> aptrCommand = (newCommandAssignment);                                                                \
        \
        if (retValVarName == CL_SUCCESS)                                                                                                    \
        {                                                                                                                                   \
            /* Set the event handle: */                                                                                                     \
            aptrCommand->setCommandEventHandle((oaCLEventHandle)*pEventVarName, wasEventCreatedBySpyVarName);                               \
            \
            csCommandQueueMonitor* pQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)queueHandle);      \
            GT_IF_WITH_ASSERT(pQueueMtr != NULL)                                                                                            \
            {                                                                                                                               \
                pQueueMtr->onCommandAddedToQueue(aptrCommand);                                                                              \
                pQueueMtr->afterCommandAddedToQueue();                                                                                      \
            }                                                                                                                               \
        }                                                                                                                                   \
        \
    }                                                                                                                                       \
    if (wasEventCreatedBySpyVarName)                                                                                                        \
    {                                                                                                                                       \
        /* Release the handle container, as don't need it anymore: */                                                                       \
        if (NULL != pEventVarName)                                                                                                          \
        {                                                                                                                                   \
            if ((cl_event)NULL != *pEventVarName)                                                                                           \
            {                                                                                                                               \
                cs_stat_realFunctionPointers.clReleaseEvent(*pEventVarName);                                                                    \
            }                                                                                                                               \
        }                                                                                                                                   \
        delete pEventVarName;                                                                                                               \
    }


// ---------------------------------------------------------------------------
// Name:        CS_AFTER_ENQUEUEING_COMMAND_NO_EVENT
// Description: After the real function call of clEnqueueXXXXX which has no "cl_event* event" parameter
// Arguments:   in pContextMonitorVarName - the context holding the queue.
//                  (of type csContextMonitor*)
//              in queueHandle - the queue handle (of type cl_command_queue)
//              newCommandAssignment - an expression of format
//                  new [apCLEnqueuedCommand subclass](param list) to assign
//                  the new command object if needed.
//              retValVarName - the return value of the function or the error code ret
//                  for functions that return a handle (of type cl_int).
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
#define CS_AFTER_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitorVarName, queueHandle, retValVarName, newCommandAssignment)              \
    if ((pContextMonitorVarName != NULL) && (retValVarName == CL_SUCCESS))                                                              \
    {                                                                                                                                   \
        /* Add the command to the queue:*/                                                                                              \
        gtAutoPtr<apCLEnqueuedCommand> aptrCommand = (newCommandAssignment);                                                            \
        \
        csCommandQueueMonitor* pQueueMtr = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)queueHandle);      \
        GT_IF_WITH_ASSERT(pQueueMtr != NULL)                                                                                            \
        {                                                                                                                               \
            pQueueMtr->onCommandAddedToQueue(aptrCommand);                                                                              \
            pQueueMtr->afterCommandAddedToQueue();                                                                                      \
        }                                                                                                                               \
    }

#else // ndef CS_USE_ENQUEUED_COMMANDS_MONITORING

// ---------------------------------------------------------------------------
// Name:        CS_BEFORE_ENQUEUEING_COMMAND
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
#define CS_BEFORE_ENQUEUEING_COMMAND(pEventVarName, eventVarName, wasEventCreatedBySpyVarName, pContextMonitorVarName, queueHandle) \
    cl_event* pEventVarName = eventVarName;                                                                                             \
    csContextMonitor* pContextMonitorVarName = cs_stat_openCLMonitorInstance.contextContainingQueue((oaCLCommandQueueHandle)queueHandle);

// ---------------------------------------------------------------------------
// Name:        CS_BEFORE_ENQUEUEING_COMMAND_NO_EVENT
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
#define CS_BEFORE_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitorVarName, queueHandle)

// ---------------------------------------------------------------------------
// Name:        CS_AFTER_ENQUEUEING_COMMAND
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
#define CS_AFTER_ENQUEUEING_COMMAND(pEventVarName, wasEventCreatedBySpyVarName, pContextMonitorVarName, queueHandle, retValVarName, newCommandAssignment)       \
    if ((pContextMonitorVarName != NULL) && (retValVarName == CL_SUCCESS))                                                                                          \
    {                                                                                                                                                               \
        /* if an event was created */                                                                                                                               \
        if (NULL != pEventVarName)                                                                                                                                  \
        {                                                                                                                                                           \
            if (NULL != *pEventVarName)                                                                                                                             \
            {                                                                                                                                                       \
                /* Retain the event, so it won't get deleted without us knowing:*/                                                                                  \
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainEvent);                                                                                                \
                cs_stat_realFunctionPointers.clRetainEvent(*pEventVarName);                                                                                         \
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainEvent);                                                                                                 \
                \
                gtAutoPtr<apCLEnqueuedCommand> aptrCommand = (newCommandAssignment);                                                                                \
                pContextMonitorVarName->eventsMonitor().onEventCreated((oaCLEventHandle)(*pEventVarName), (oaCLCommandQueueHandle)queueHandle, aptrCommand);        \
            }                                                                                                                                                       \
        }                                                                                                                                                           \
    }

// ---------------------------------------------------------------------------
// Name:        CS_AFTER_ENQUEUEING_COMMAND_NO_EVENT
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
#define CS_AFTER_ENQUEUEING_COMMAND_NO_EVENT(pContextMonitorVarName, queueHandle, retValVarName, newCommandAssignment)

#endif

#endif //__CSOPENCLWRAPPERSAIDMACROS_H

