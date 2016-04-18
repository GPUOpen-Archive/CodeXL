//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCommandQueueMonitor.cpp
///
//==================================================================================

//------------------------------ csCommandQueueMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/csCommandQueueMonitor.h>
#include <src/csContextMonitor.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLHandleMonitor.h>
#include <src/csOpenCLMonitor.h>
#include <src/csStringConstants.h>

// Maximal amount of events per command queue:
#define CS_MAX_EVENTS_PER_QUEUE 500


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::csCommandQueueMonitor
// Description: Constructor.
// Arguments:
//  contextSpyId - The command queue's context spy id.
//  commandQueueIndex - The command queue's index in it's context.
//  commandQueueHandle - The command queue's OpenCL handle.
//  contextHandle - The command queue's OpenCL context handle.
//  deviceId - The command queue's OpenCL device id.
// Author:      Yaki Tebeka
// Date:        3/3/2010
// ---------------------------------------------------------------------------
csCommandQueueMonitor::csCommandQueueMonitor(int contextSpyId, int commandQueueIndex, oaCLCommandQueueHandle commandQueueHandle,
                                             oaCLContextHandle contextHandle, int deviceIndex, bool shouldInitializePerformanceCounters)
    : _contextSpyId(contextSpyId), _commandQueueIndex(commandQueueIndex),
      _commandQueueInfo(commandQueueHandle, contextHandle, deviceIndex),
      _vendorType(OA_VENDOR_UNKNOWN), m_logQueueEvents(false), _maxCommandsPerQueue(0), _wasEventsOverflowErrorIssued(false)
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    , _amdPerformanceCountersReader(contextSpyId, commandQueueIndex)
#endif
#endif
    , _shouldInitializePerformanceCounters(shouldInitializePerformanceCounters), m_logEventProfilingInformation(false)
{
    // Set me as the command queue monitor for the OpenCL queue reader:
    _openCLQueueCountersReader.setCommandQueueMonitor(this);

    // Log the maximal amount of commands per queue:
    _maxCommandsPerQueue = suMaxLoggedOpenCLCommandPerQueue();

    // Update the queue's device vendor type:
    updateVendorType();

    // Log the command queue handle:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    csOpenCLHandleMonitor& handlesMonitor = theOpenCLMonitor.openCLHandleMonitor();
    handlesMonitor.registerOpenCLHandle((oaCLHandle)commandQueueHandle, _contextSpyId, _commandQueueIndex, OS_TOBJ_ID_CL_COMMAND_QUEUE);

    // Register this allocated object:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(_commandQueueInfo);

    if (_shouldInitializePerformanceCounters)
    {

        // Notify the Queue's counters manager:
        theOpenCLMonitor.openCLQueueCountersManager().onQueueCreatedEvent(this);

        // Call the AMD performance counter on queue creation event:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            // Set the AMD performance counter command queue monitor:
            _amdPerformanceCountersReader.setCommandQueueMonitor(this);
            theOpenCLMonitor.AMDPerformanceCountersManager().onQueueCreatedEvent(this);
        }
#endif
#endif
    }

    // Get the context monitor related to this context spy id:
    suContextMonitor* pSUContextMonitor = theOpenCLMonitor.contextMonitor(contextSpyId);
    GT_IF_WITH_ASSERT(pSUContextMonitor != NULL)
    {
        _pContextMonitor = (csContextMonitor*)pSUContextMonitor;
        GT_ASSERT(_pContextMonitor != NULL);
    }
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::~csCommandQueueMonitor
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        3/3/2010
// ---------------------------------------------------------------------------
csCommandQueueMonitor::~csCommandQueueMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::updateContextDataSnapshot
// Description: Updates the command queue information from OpenCL
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::updateContextDataSnapshot()
{
    bool retVal = false;

    // Real updates can only be performed on queues marked for deletion:
    if (!_commandQueueInfo.wasMarkedForDeletion())
    {
        // Don't allow other threads to access the command queue data:
        lockAccessToEnqueuedCommands();

        // Flush the OpenCL queue:
        bool rcFlush = flushOpenCLQueue();

        // Update the queue's reference count:
        bool rcRefCount = updateQueueReferenceCount();
        GT_ASSERT(rcRefCount);

        bool rcTimes = updateQueueTimesIfDeviceAllowsProfiling();

        // TO_DO: OpenCL - do we need to update other things?

        // Re-allow command queue data access:
        unlockAccessToEnqueuedCommands();

        retVal = rcFlush && rcRefCount && rcTimes;
    }
    else // _commandQueueInfo.wasMarkedForDeletion()
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::updateQueueTimesIfDeviceAllowsProfiling
// Description: Updates the queue command times if the device allows profiling.
//              a non-supporting device is considered a failure
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/3/2010
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::updateQueueTimesIfDeviceAllowsProfiling()
{
    // Note that if we somehow failed to get the device data, we still won't call
    // any invalid commands, since the commands will not have events attached to them:
    bool retVal = false;
    bool deviceAllowsProfiling = doesDeviceSupportProfiling();

    if (deviceAllowsProfiling)
    {
        // Don't allow other threads to access the command queue data:
        lockAccessToEnqueuedCommands();

        // Updates the queue command times:
        retVal = updateQueueTimes();

        // Re-allow command queue data access:
        unlockAccessToEnqueuedCommands();
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::onCommandQueueMarkedForDeletion
// Description: Called when the queue is marked for deletion
//              (calling clReleaseCommandQueue with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::onCommandQueueMarkedForDeletion()
{
    // Log the command queue handle:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    csOpenCLHandleMonitor& handlesMonitor = theOpenCLMonitor.openCLHandleMonitor();
    handlesMonitor.registerOpenCLHandle((oaCLHandle)_commandQueueInfo.commandQueueHandle(), _contextSpyId, -1, OS_TOBJ_ID_CL_COMMAND_QUEUE);

    // Mark the queue object as deleted:
    _commandQueueInfo.onCommandQueueMarkedForDeletion();
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::beforeCommandAddedToQueue
// Description:
//  Is called before a clEnqueueXXXX function is used to push a command into the command queue.
// Author:      Yaki Tebeka
// Date:        1/6/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::beforeCommandAddedToQueue()
{

}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::afterCommandAddedToQueue
// Description:
//  Is called after a clEnqueueXXXX function handling is over
//  (see beforeCommandAddedToQueue)
// Author:      Yaki Tebeka
// Date:        1/6/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::afterCommandAddedToQueue()
{

}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::beforeAPIThreadDataAccess
// Description:
//  Is called before an API function is accessing the command queue data.
// Author:      Yaki Tebeka
// Date:        1/6/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::beforeAPIThreadDataAccess()
{
    // Don't allow threads other than the API thread to access the queue data:
    lockAccessToEnqueuedCommands();
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::beforeAPIThreadDataAccess
// Description:
//  Is called after the API function's data access is over.
//  (See beforeAPIThreadDataAccess)
// Author:      Yaki Tebeka
// Date:        1/6/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::afterAPIThreadDataAccess()
{
    // Re-allow command queue data access:
    unlockAccessToEnqueuedCommands();
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::onCommandAddedToQueue
// Description: Is called when a command is added to the queue.
// Arguments: aptrCommand - The added command.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::onCommandAddedToQueue(gtAutoPtr<apCLEnqueuedCommand>& aptrCommand)
{
    if (m_logEventProfilingInformation)
    {
        // Don't allow other threads to read from here while we are adding:
        lockAccessToEnqueuedCommands();

        // Add the command:
        addCommandToQueue(aptrCommand);

        // Add an idle period after the command:
        gtAutoPtr<apCLEnqueuedCommand> aptrIdle = new apCLQueueIdle;
        addCommandToQueue(aptrIdle);

        // Re-allow command queue data access:
        unlockAccessToEnqueuedCommands();
    }
    else // !m_logEventProfilingInformation
    {
        // We still want to log the existence of the event object, if the user created one:
        oaCLEventHandle eventHandle = aptrCommand->commandEventExternalHandle();

        if (OA_CL_NULL_HANDLE != eventHandle)
        {
            bool ignored = false;
            gtString ignored2;
            int ignored3 = -1;
            addEventToQueue(eventHandle, false, true, ignored, ignored2, ignored3);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::onCommandQueuePropertiesSet
// Description: Is called when clSetCommandQueueProperty is used to set properties
//              on or off on a queue.
// Author:      Uri Shomroni
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::onCommandQueuePropertiesSet(cl_command_queue_properties properties, cl_bool enable)
{
    // See if the queue properties are being enabled or disabled:
    bool arePropertiesEnabled = (enable == CL_TRUE);

    // If we are changing the out-of-order execution mode:
    if ((properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0)
    {
        _commandQueueInfo.setOutOfOrderExecutionModeEnable(arePropertiesEnabled);
    }

    // If we are changing the profiling mode:
    if ((properties & CL_QUEUE_PROFILING_ENABLE) != 0)
    {
        _commandQueueInfo.setProfilingModeEnable(arePropertiesEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::onEnqueueNDRangeKernel
// Description: Is called when an NDRange kernel is enqueued.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::onEnqueueNDRangeKernel()
{
    if (_shouldInitializePerformanceCounters)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            if (_vendorType == OS_VENDOR_ATI)
            {
                // TO_DO: OpenCL This does not seems logical !!! (NDRange -> FrameTerminator)
                _amdPerformanceCountersReader.onFrameTerminatorCall();
            }
        }
#endif
#endif
    }
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::onDebuggedProcessResumed
// Description: Called when the debugged process is resumed from being suspended
// Author:      Uri Shomroni
// Date:        12/4/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::onDebuggedProcessResumed()
{
    // Clear the queue's commands list, to avoid the gap that happens when the debugged
    // process is suspended (as the device clock keeps running at this time):
    clearQueue();
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::amountOfCommandsInQueue
// Description: Returns the amount of commands currently logged.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
int csCommandQueueMonitor::amountOfCommandsInQueue() const
{
    // Don't allow other threads to access the command queue data:
    ((csCommandQueueMonitor*)this)->lockAccessToEnqueuedCommands();

    int retVal = (int)(_enqueuedCommands.size());

    // Re-allow command queue data access:
    ((csCommandQueueMonitor*)this)->unlockAccessToEnqueuedCommands();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::getEnqueuedCommandDetails
// Description: Returns the object that represents the enqueuedCommandIndex-th command
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
const apCLEnqueuedCommand* csCommandQueueMonitor::getEnqueuedCommandDetails(int enqueuedCommandIndex) const
{
    const apCLEnqueuedCommand* retVal = NULL;

    // Don't allow other threads to access the command queue data:
    ((csCommandQueueMonitor*)this)->lockAccessToEnqueuedCommands();

    // Validate the command index:
    GT_IF_WITH_ASSERT((enqueuedCommandIndex >= 0) && (enqueuedCommandIndex < (int)_enqueuedCommands.size()))
    {
        retVal = _enqueuedCommands[enqueuedCommandIndex];
    }

    // Re-allow command queue data access:
    ((csCommandQueueMonitor*)this)->unlockAccessToEnqueuedCommands();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::addCommandToQueue
// Description: Called by csCommandQueueTimerThread to add a command to a queue
// Author:      Uri Shomroni
// Date:        7/12/2009
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::addCommandToQueue(gtAutoPtr<apCLEnqueuedCommand>& aptrCommand)
{
    oaCLEventHandle eventHandle = aptrCommand->commandEventHandle();
    bool wasHandleCreatedBySpy = aptrCommand->wasEventCreatedBySpy();
    bool isIdleEvent = (aptrCommand->type() == OS_TOBJ_ID_CL_QUEUE_IDLE_TIME);
    bool commandHasEventParameter = aptrCommand->eventParameterAvailable();

    // Don't allow other threads to access the command queue data:
    lockAccessToEnqueuedCommands();

    // To avoid overflows, we clear the queue whenever it reaches a certain size:
    if (_enqueuedCommands.size() >= _maxCommandsPerQueue)
    {
        clearQueue();
    }

    // Get the command relevant memory size:
    bool rc = getCommandMemorySize(aptrCommand);
    GT_ASSERT(rc);

    // Add the command to the queue:
    _enqueuedCommands.push_back(aptrCommand.releasePointedObjectOwnership());

    // If the event was created by the user, and the command supports time measurements:
    if ((!wasHandleCreatedBySpy) && (!isIdleEvent) && commandHasEventParameter)
    {
        // Make sure we got a valid handle:
        GT_IF_WITH_ASSERT(eventHandle != OA_CL_NULL_HANDLE)
        {
            // Add to the handle's reference count:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainEvent);
            cl_int rcRetain = cs_stat_realFunctionPointers.clRetainEvent((cl_event)eventHandle);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainEvent);

            GT_ASSERT(rcRetain == CL_SUCCESS);
            bool ignored = false;
            gtString ignored2;
            int ignored3 = -1;
            addEventToQueue(eventHandle, (CL_SUCCESS == rcRetain), true, ignored, ignored2, ignored3);
        }
    }

    // Re-allow command queue data access:
    unlockAccessToEnqueuedCommands();
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::amountOfEventsInQueue
// Description: Returns the number of events in a Queue
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
int csCommandQueueMonitor::amountOfEventsInQueue() const
{
    int retVal = 0;

    // Don't allow other threads to access the command queue data:
    ((csCommandQueueMonitor*)this)->lockAccessToEnqueuedCommands();

    retVal = (int)_commandQueueEvents.size();

    // Re-allow command queue data access:
    ((csCommandQueueMonitor*)this)->unlockAccessToEnqueuedCommands();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::flushOpenCLQueue
// Description: Flushes the OpenCL queue.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::flushOpenCLQueue()
{
    bool retVal = false;

    // Don't allow other threads to access the command queue data:
    lockAccessToEnqueuedCommands();

    // Get the queue's OpenCL handle:
    cl_command_queue commandQueueHandle = (cl_command_queue)_commandQueueInfo.commandQueueHandle();
    GT_IF_WITH_ASSERT(commandQueueHandle != NULL)
    {
        // Flush the queue:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clFlush);
        cl_int cRetVal = cs_stat_realFunctionPointers.clFlush(commandQueueHandle);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clFlush);

        retVal = (cRetVal == CL_SUCCESS);
    }

    // Re-allow command queue data access:
    unlockAccessToEnqueuedCommands();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::updateQueueReferenceCount
// Description: Update the command queue reference count.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/2/2010
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::updateQueueReferenceCount()
{
    bool retVal = false;

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);

    // TO_DO: AMD Counters: Do we want to update the rest of the stuff?
    // Get the command queue's reference count:
    cl_command_queue commandQueueHandle = (cl_command_queue)_commandQueueInfo.commandQueueHandle();
    cl_uint referenceCount = 0;
    cl_uint rcGetRefCnt = cs_stat_realFunctionPointers.clGetCommandQueueInfo(commandQueueHandle, CL_QUEUE_REFERENCE_COUNT, sizeof(cl_uint), &referenceCount, NULL);
    GT_IF_WITH_ASSERT(rcGetRefCnt == CL_SUCCESS)
    {
        // Subtract 1 for the reference that the debugger adds:
        _commandQueueInfo.setReferenceCount((gtUInt32)((referenceCount > 0) ? (referenceCount - 1) : referenceCount));
        retVal = true;
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::updateQueueTimes
// Description: Runs the vector in _commandQueuesEnqueuedCommands for queue #i,
//              and updates the command times
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::updateQueueTimes()
{
    bool retVal = true;

    // Iterate the commands in the queue:
    int numberOfCommands = (int)_enqueuedCommands.size();

    if (0 < numberOfCommands)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetEventProfilingInfo);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetEventInfo);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseEvent);

        apCLEnqueuedCommand* pPreviousCommand = NULL;
        apCLEnqueuedCommand* pCurrentCommand = NULL;
        apCLEnqueuedCommand* pNextCommand = _enqueuedCommands[0];

        for (int i = 0; i < numberOfCommands; i++)
        {
            // Get the current command:
            pCurrentCommand = pNextCommand;

            if (i < numberOfCommands - 1)
            {
                pNextCommand = _enqueuedCommands[i + 1];
            }
            else
            {
                pNextCommand = NULL;
            }

            GT_IF_WITH_ASSERT(pCurrentCommand != NULL)
            {
                // Idles' timing is calculated by the items before and after them:
                osTransferableObjectType currentCommandType = pCurrentCommand->type();

                if (currentCommandType != OS_TOBJ_ID_CL_QUEUE_IDLE_TIME)
                {
                    // If we haven't already updated these times:
                    if (!pCurrentCommand->areTimesUpdated())
                    {
                        // Get this command's timing:
                        cl_event currentCommandEventHandle = (cl_event)pCurrentCommand->commandEventHandle();

                        if (currentCommandEventHandle != NULL)
                        {
                            // Only get times for commands that have completed:
                            cl_int eventExecStatus = CL_NONE;
                            cl_int rcStat = cs_stat_realFunctionPointers.clGetEventInfo(currentCommandEventHandle, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &eventExecStatus, NULL);
                            GT_ASSERT(rcStat == CL_SUCCESS);

                            if ((rcStat == CL_SUCCESS) && (eventExecStatus == CL_COMPLETE))
                            {
                                cl_ulong queuedTime = 0;
                                cl_ulong submittedTime = 0;
                                cl_ulong execStartedTime = 0;
                                cl_ulong execEndedTime = 0;
                                cl_ulong execCompletedTime = 0;
                                bool gotTimes = true;

                                cl_int rcTime = cs_stat_realFunctionPointers.clGetEventProfilingInfo(currentCommandEventHandle, CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &queuedTime, NULL);
                                gotTimes = gotTimes && (rcTime == CL_SUCCESS);

                                rcTime = cs_stat_realFunctionPointers.clGetEventProfilingInfo(currentCommandEventHandle, CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &submittedTime, NULL);
                                gotTimes = gotTimes && (rcTime == CL_SUCCESS);

                                rcTime = cs_stat_realFunctionPointers.clGetEventProfilingInfo(currentCommandEventHandle, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &execStartedTime, NULL);
                                gotTimes = gotTimes && (rcTime == CL_SUCCESS);

                                rcTime = cs_stat_realFunctionPointers.clGetEventProfilingInfo(currentCommandEventHandle, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &execEndedTime, NULL);
                                gotTimes = gotTimes && (rcTime == CL_SUCCESS);

                                // Completion time is only relevant in OpenCL 2.0:
                                rcTime = cs_stat_realFunctionPointers.clGetEventProfilingInfo(currentCommandEventHandle, CL_PROFILING_COMMAND_COMPLETE, sizeof(cl_ulong), &execEndedTime, NULL);

                                if ((CL_SUCCESS != rcTime) && (CL_PROFILING_INFO_NOT_AVAILABLE != rcTime))
                                {
                                    execCompletedTime = execEndedTime;
                                }

                                // Some events in some OpenCL implementations sometimes return 0 as the submitted time:
                                if (submittedTime == 0)
                                {
                                    submittedTime = queuedTime;
                                }

                                // Yaki + Uri, 14/4/2010 - On Mac, sometimes the profiling info functions "work" (i.e. returnr CL_SUCCESS) but return 0 values for some fields.
                                // This mostly happens with clEnqueue[Acquire|Release]GLObjects.
                                // While this is not dangerous when it happens with the queued / submitted time, the execution times are used for many calculations, so we do
                                // not consider it updated if we didn't get the values:
                                if (gotTimes && ((execStartedTime == 0) || (execEndedTime == 0)))
                                {
                                    // Copy whatever data we have:
                                    gotTimes = false;
                                    pCurrentCommand->setEventTimes(queuedTime, submittedTime, execStartedTime, execEndedTime, execCompletedTime);
                                }

                                // If we got all the times:
                                if (gotTimes)
                                {
                                    // Update the previous and next commands (we expect both, if they exist, to be Idles):
                                    if (pPreviousCommand != NULL)
                                    {
                                        GT_IF_WITH_ASSERT(pPreviousCommand->type() == OS_TOBJ_ID_CL_QUEUE_IDLE_TIME)
                                        {
                                            // The previous idle ends when this started:
                                            gtUInt64 idleStartedTime = pPreviousCommand->executionStartedTime();
                                            pPreviousCommand->setEventTimes(idleStartedTime, idleStartedTime, idleStartedTime, execStartedTime, execStartedTime);

                                            // If we got both times, this idle is done:
                                            if (idleStartedTime != 0)
                                            {
                                                pPreviousCommand->onTimesUpdated();
                                            }
                                        }
                                    }

                                    if (pNextCommand != NULL)
                                    {
                                        GT_IF_WITH_ASSERT(pNextCommand->type() == OS_TOBJ_ID_CL_QUEUE_IDLE_TIME)
                                        {
                                            // The next idle starts when this command ended:
                                            gtUInt64 idleEndedTime = pNextCommand->executionEndedTime();
                                            pNextCommand->setEventTimes(execEndedTime, execEndedTime, execEndedTime, idleEndedTime, idleEndedTime);

                                            // If we got both times, this idle is done:
                                            if (idleEndedTime != 0)
                                            {
                                                pNextCommand->onTimesUpdated();
                                            }
                                        }
                                    }

                                    // Mark the current command as updated:
                                    pCurrentCommand->setEventTimes(queuedTime, submittedTime, execStartedTime, execEndedTime, execCompletedTime);
                                    pCurrentCommand->onTimesUpdated();

                                    // Release the event (or reduce its ref count if it was created by the user):
                                    cs_stat_realFunctionPointers.clReleaseEvent(currentCommandEventHandle);

                                    // Dispose of the event if we created it:
                                    if (pCurrentCommand->wasEventCreatedBySpy())
                                    {
                                        // Mark we don't have an event anymore:
                                        pCurrentCommand->setCommandEventHandle(OA_CL_NULL_HANDLE, false);
                                    }
                                    else
                                    {
                                        // Find the event object:
                                        const apCLEvent* pEvent = _pContextMonitor->eventsMonitor().eventDetails((oaCLEventHandle)currentCommandEventHandle);

                                        if (pEvent != NULL)
                                        {
                                            // Tell the event it is no longer retained by the spy:
                                            ((apCLEvent*)pEvent)->setRetainedBySpy(false);
                                        }
                                        else
                                        {
                                            // The event should be NULL only if we are after an event overflow warning:
                                            GT_ASSERT(_wasEventsOverflowErrorIssued);
                                        }

                                        // We can ignore this event since we reduced its ref count back:
                                        pCurrentCommand->setCommandEventHandle(OA_CL_NULL_HANDLE, false);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            pPreviousCommand = pCurrentCommand;
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetEventProfilingInfo);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetEventInfo);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseEvent);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::updateVendorType
// Description: Update the command queue's vendor type
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::updateVendorType()
{
    // Get the command queue device index:
    int deviceIndex = _commandQueueInfo.deviceIndex();

    // Get the OpenCL devices monitor:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    const csDevicesMonitor& devicesMonitr = theOpenCLMonitor.devicesMonitor();

    // Get the device object details:
    const apCLDevice* pDevice = devicesMonitr.getDeviceObjectDetailsByIndex(deviceIndex);

    GT_IF_WITH_ASSERT(pDevice != NULL)
    {
        // Get the device id:
        oaCLDeviceID deviceId = pDevice->deviceHandle();

        // Get the device info:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);

        size_t stringLen = 0;
        cl_int clRetVal = cs_stat_realFunctionPointers.clGetDeviceInfo(cl_device_id(deviceId), CL_DEVICE_VENDOR, 0, NULL, &stringLen);
        GT_IF_WITH_ASSERT((clRetVal == CL_SUCCESS) && (stringLen > 0))
        {
            char* pDeviceVendor = new char[stringLen + 1];
            clRetVal = cs_stat_realFunctionPointers.clGetDeviceInfo(cl_device_id(deviceId), CL_DEVICE_VENDOR, stringLen + 1, pDeviceVendor, NULL);
            GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
            {
                GT_IF_WITH_ASSERT(pDeviceVendor != NULL)
                {
                    // Get the device vendor string from the device id:
                    gtString vendorTypeLowerCase;
                    vendorTypeLowerCase.fromASCIIString(pDeviceVendor);
                    delete[] pDeviceVendor;
                    vendorTypeLowerCase.toLowerCase();

                    _vendorType = OA_VENDOR_UNKNOWN;

                    // Find one of the renderer types within the vendor string:
                    if (vendorTypeLowerCase.startsWith(L"advanced micro devices"))
                    {
                        _vendorType = OA_VENDOR_ATI;
                    }
                    else if (vendorTypeLowerCase.startsWith(L"nvidia"))
                    {
                        _vendorType = OA_VENDOR_NVIDIA;
                    }
                    else if (vendorTypeLowerCase.startsWith(L"s3"))
                    {
                        _vendorType = OA_VENDOR_S3;
                    }
                    else if (vendorTypeLowerCase.startsWith(L"intel"))
                    {
                        _vendorType = OA_VENDOR_INTEL;
                    }
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);
    }
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::clearQueue
// Description: Clears the enqueued commands vector
// Author:      Uri Shomroni
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::clearQueue()
{
    // Don't allow other threads to access the command queue data:
    lockAccessToEnqueuedCommands();

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseEvent);

    // Run over the enqueued commands and release any events they still have:
    int numberOfCommands = (int)_enqueuedCommands.size();

    for (int i = 0; i < numberOfCommands; i++)
    {
        // Get the command:
        apCLEnqueuedCommand* pCurrentCommand = _enqueuedCommands[i];
        GT_IF_WITH_ASSERT(pCurrentCommand != NULL)
        {
            // If this command has an event:
            oaCLEventHandle currentEvent = pCurrentCommand->commandEventHandle();

            if (currentEvent != OA_CL_NULL_HANDLE)
            {
                // Lower this event's ref count by 1:
                cl_int rcRel = cs_stat_realFunctionPointers.clReleaseEvent((cl_event)currentEvent);
                GT_ASSERT(rcRel == CL_SUCCESS);

                // Dispose of the event if we created it:
                if (pCurrentCommand->wasEventCreatedBySpy())
                {
                    // Mark we don't have an event anymore:
                    pCurrentCommand->setCommandEventHandle(OA_CL_NULL_HANDLE, false);
                }
                else
                {
                    // Find the event object:
                    const apCLEvent* pEvent = _pContextMonitor->eventsMonitor().eventDetails(currentEvent);

                    if (pEvent != NULL)
                    {
                        // Tell the event it is no longer retained by the spy:
                        ((apCLEvent*)pEvent)->setRetainedBySpy(false);
                    }
                    else
                    {
                        // The event should be NULL only if we are after an event overflow warning:
                        GT_ASSERT(_wasEventsOverflowErrorIssued);
                    }
                }
            }
        }
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseEvent);

    // Clear the vector:
    _enqueuedCommands.deleteElementsAndClear();

    // Re-allow command queue data access:
    unlockAccessToEnqueuedCommands();
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::addEventToQueue
// Description: Adds an event object to the queue
// Author:      Uri Shomroni
// Date:        21/5/2013
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::addEventToQueue(oaCLEventHandle eventHandle, bool retainedBySpy, bool reportEventOverflow, bool& wasEventOverflow, gtString& errorToReport, int& errorContext)
{
    (void)retainedBySpy;

    wasEventOverflow = false;

    GT_IF_WITH_ASSERT(eventHandle != OA_CL_NULL_HANDLE)
    {
        if (m_logQueueEvents)
        {
            lockAccessToEnqueuedCommands();

            // Get the next event index:
            int numberOfEventsInQueue = (int)_commandQueueEvents.size();

            if (numberOfEventsInQueue < CS_MAX_EVENTS_PER_QUEUE)
            {
                // Is this a new event?
                bool newEvent = true;

                for (int i = 0; i < numberOfEventsInQueue; i++)
                {
                    if (_commandQueueEvents[i] == eventHandle)
                    {
                        newEvent = false;
                        break;
                    }
                }

                GT_IF_WITH_ASSERT(newEvent)
                {
                    _commandQueueEvents.push_back(eventHandle);

                    // If this is the last event we will be adding:
                    if (numberOfEventsInQueue == (CS_MAX_EVENTS_PER_QUEUE - 1))
                    {
                        wasEventOverflow = true;
                        errorContext = _contextSpyId;
                        errorToReport.makeEmpty().appendFormattedString(CS_STR_queueEventsOverflow, CS_MAX_EVENTS_PER_QUEUE, _commandQueueIndex + 1, errorContext);

                        if (reportEventOverflow)
                        {
                            // Report a detected error:
                            cs_stat_openCLMonitorInstance.reportDetectedError(errorContext, AP_QUEUE_EVENTS_OVERFLOW, errorToReport);
                        }

                        // Mark that we can no longer take account for events:
                        _wasEventsOverflowErrorIssued = true;
                    }
                }
            }

            unlockAccessToEnqueuedCommands();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::removeEventFromQueue
// Description: Removes an event handle from our vector:
// Author:      Uri Shomroni
// Date:        24/10/2013
// ---------------------------------------------------------------------------
void csCommandQueueMonitor::removeEventFromQueue(oaCLEventHandle eventHandle)
{
    GT_IF_WITH_ASSERT(eventHandle != OA_CL_NULL_HANDLE)
    {
        if (m_logQueueEvents)
        {
            lockAccessToEnqueuedCommands();

            int numberOfEventsInQueue = (int)_commandQueueEvents.size();
            bool foundEvent = false;

            for (int i = 0; i < numberOfEventsInQueue; i++)
            {
                if (foundEvent)
                {
                    // This can only be called starting with the second iteration - so we can access i - 1:
                    _commandQueueEvents[i - 1] = _commandQueueEvents[i];
                }
                else // !foundEvent
                {
                    // Check the current event:
                    foundEvent = (eventHandle == _commandQueueEvents[i]);
                }
            }

            // The last handle we now have is either the found event or a duplicate we created.
            GT_IF_WITH_ASSERT(foundEvent)
            {
                // Remove it:
                _commandQueueEvents.pop_back();
            }

            unlockAccessToEnqueuedCommands();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::doesDeviceSupportProfiling
// Description: Returns true iff the device associated with this command queue
//              supports profiling (CL_QUEUE_PROFILING_ENABLE).
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::doesDeviceSupportProfiling() const
{
    bool retVal = true;

    // Get the device associated with this command queue:
    const apCLDevice* pQueueDevice = cs_stat_openCLMonitorInstance.devicesMonitor().getDeviceObjectDetailsByIndex(_commandQueueInfo.deviceIndex());
    GT_IF_WITH_ASSERT(pQueueDevice != NULL)
    {
        // Check if the device supports profiling:
        retVal = ((pQueueDevice->deviceQueueProperties() & CL_QUEUE_PROFILING_ENABLE) != 0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::getCommandMemorySize
// Description: Get the command memory size according to the command type
// Arguments:   gtAutoPtr<apCLEnqueuedCommand>& aptrCommand
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/5/2010
// ---------------------------------------------------------------------------
bool csCommandQueueMonitor::getCommandMemorySize(gtAutoPtr<apCLEnqueuedCommand>& aptrCommand)
{
    bool retVal = false;

    // Get the command type:
    osTransferableObjectType objectType = aptrCommand->type();

    switch (objectType)
    {
        case OS_TOBJ_ID_CL_WRITE_BUFFER_COMMAND:
        {
            // Add the buffer write command size to the reader:
            apCLWriteBufferCommand* pWriteCommand = (apCLWriteBufferCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pWriteCommand != NULL)
            {
                // Add the buffer write size to the reader:
                _openCLQueueCountersReader.addToWriteSize(pWriteCommand->writtenDataBytes());
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_WRITE_BUFFER_RECT_COMMAND:
        {
            // Add the buffer write command size to the reader:
            apCLWriteBufferRectCommand* pWriteCommand = (apCLWriteBufferRectCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pWriteCommand != NULL)
            {
                // Add the buffer write size to the reader:
                gtUInt64 writtenBytes = pWriteCommand->writtenDataRegionX() * pWriteCommand->writtenDataRegionY() * pWriteCommand->writtenDataRegionZ();
                _openCLQueueCountersReader.addToWriteSize(writtenBytes);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_WRITE_IMAGE_COMMAND:
        {
            // Add the image write command size to the reader:
            apCLWriteImageCommand* pWriteCommand = (apCLWriteImageCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pWriteCommand != NULL)
            {
                // Get amount of pixels written:
                gtUInt32 amountOfPixels = (gtUInt32)pWriteCommand->amountOfPixels();

                // Get the image related memory object:
                gtVector<oaCLMemHandle> dstMemHandles;
                pWriteCommand->getRelatedDstMemHandles(dstMemHandles);

                // Get the pixel size according to image format:
                gtInt32 imagePixelSize = memObjectHandleToPixelSize(dstMemHandles[0]);

                // Calculate the write size:
                gtUInt64 imageWriteSize = imagePixelSize * amountOfPixels;

                // Add the image write size to the reader:
                _openCLQueueCountersReader.addToWriteSize(imageWriteSize);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_FILL_IMAGE_COMMAND:
        {
            // Add the image write command size to the reader:
            apCLFillImageCommand* pFillCommand = (apCLFillImageCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pFillCommand != NULL)
            {
                // Get amount of pixels written:
                gtSize_t rX = pFillCommand->filledRegionX();
                gtSize_t rY = pFillCommand->filledRegionY();
                gtSize_t rZ = pFillCommand->filledRegionZ();
                gtUInt32 amountOfPixels = (gtUInt32)(rX * (rY > 0 ? rY : 1) * (rZ > 0 ? rZ : 1));

                // Get the image related memory object:
                gtVector<oaCLMemHandle> dstMemHandles;
                pFillCommand->getRelatedDstMemHandles(dstMemHandles);

                // Get the pixel size according to image format:
                gtInt32 imagePixelSize = memObjectHandleToPixelSize(dstMemHandles[0]);

                // Calculate the write size:
                gtUInt64 imageWriteSize = imagePixelSize * amountOfPixels;

                // Add the image write size to the reader:
                _openCLQueueCountersReader.addToWriteSize(imageWriteSize);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_READ_BUFFER_COMMAND:
        {
            // Add the buffer read command size to the reader:
            apCLReadBufferCommand* pReadCommand = (apCLReadBufferCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pReadCommand != NULL)
            {
                // Add the buffer read size to the reader:
                _openCLQueueCountersReader.addToReadSize(pReadCommand->readDataBytes());
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_READ_BUFFER_RECT_COMMAND:
        {
            // Add the buffer read command size to the reader:
            apCLReadBufferRectCommand* pReadCommand = (apCLReadBufferRectCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pReadCommand != NULL)
            {
                // Add the buffer read size to the reader:
                gtUInt64 readBytes = pReadCommand->readDataRegionX() * pReadCommand->readDataRegionY() * pReadCommand->readDataRegionZ();
                _openCLQueueCountersReader.addToReadSize(readBytes);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_READ_IMAGE_COMMAND:
        {
            // Add the image read command size to the reader:
            apCLReadImageCommand* pReadCommand = (apCLReadImageCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pReadCommand != NULL)
            {
                // Get amount of pixels written:
                gtUInt32 amountOfPixels = (gtUInt32)pReadCommand->amountOfPixels();

                // Get the image related memory object:
                gtVector<oaCLMemHandle> srcMemHandles;
                pReadCommand->getRelatedSrcMemHandles(srcMemHandles);

                // Get the pixel size according to image format:
                gtInt32 imagePixelSize = memObjectHandleToPixelSize(srcMemHandles[0]);

                // Calculate the write size:
                gtUInt64 imageReadSize = imagePixelSize * amountOfPixels;

                // Add the image read size to the reader:
                _openCLQueueCountersReader.addToReadSize(imageReadSize);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_COMMAND:
        {
            // Add the buffer copy command size to the reader:
            apCLCopyBufferCommand* pCopyCommand = (apCLCopyBufferCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pCopyCommand != NULL)
            {
                // Add the buffer copy size to the reader:
                _openCLQueueCountersReader.addToCopySize(pCopyCommand->copiedBytes());
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_RECT_COMMAND:
        {
            // Add the buffer copy command size to the reader:
            apCLCopyBufferRectCommand* pCopyCommand = (apCLCopyBufferRectCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pCopyCommand != NULL)
            {
                // Add the buffer copy size to the reader:
                gtUInt64 copiedBytes = pCopyCommand->copiedRegionX() * pCopyCommand->copiedRegionY() * pCopyCommand->copiedRegionZ();
                _openCLQueueCountersReader.addToCopySize(copiedBytes);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_TO_IMAGE_COMMAND:
        {
            // Add the buffer copy command size to the reader:
            apCLCopyBufferToImageCommand* pCopyCommand = (apCLCopyBufferToImageCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pCopyCommand != NULL)
            {
                // Get amount of pixels written:
                gtUInt32 amountOfPixels = (gtUInt32)pCopyCommand->amountOfPixels();

                // Get the image related memory object:
                gtVector<oaCLMemHandle> dstMemHandles;
                pCopyCommand->getRelatedDstMemHandles(dstMemHandles);

                // Get the pixel size according to image format:
                gtInt32 imagePixelSize = memObjectHandleToPixelSize(dstMemHandles[0]);

                // Calculate the copy size:
                gtUInt64 imageCopySize = imagePixelSize * amountOfPixels;

                // Add the buffer copy size to the reader:
                _openCLQueueCountersReader.addToCopySize(imageCopySize);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_COPY_IMAGE_COMMAND:
        {
            // Add the image read command size to the reader:
            apCLCopyImageCommand* pCopyCommand = (apCLCopyImageCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pCopyCommand != NULL)
            {
                // Get amount of pixels written:
                gtUInt32 amountOfPixels = (gtUInt32)pCopyCommand->amountOfPixels();

                // Get the image related memory object:
                gtVector<oaCLMemHandle> srcMemHandles;
                pCopyCommand->getRelatedSrcMemHandles(srcMemHandles);

                // Get the pixel size according to image format:
                gtInt32 imagePixelSize = memObjectHandleToPixelSize(srcMemHandles[0]);

                // Calculate the copy size:
                gtUInt64 imageCopySize = imagePixelSize * amountOfPixels;

                // Add the image copy size to the reader:
                _openCLQueueCountersReader.addToCopySize(imageCopySize);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_COPY_IMAGE_TO_BUFFER_COMMAND:
        {
            // Add the image read command size to the reader:
            apCLCopyImageToBufferCommand* pCopyCommand = (apCLCopyImageToBufferCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pCopyCommand != NULL)
            {
                // Get amount of pixels written:
                gtUInt32 amountOfPixels = (gtUInt32)pCopyCommand->amountOfPixels();

                // Get the image related memory object:
                gtVector<oaCLMemHandle> srcMemHandles;
                pCopyCommand->getRelatedSrcMemHandles(srcMemHandles);

                // Get the pixel size according to image format:
                gtInt32 imagePixelSize = memObjectHandleToPixelSize(srcMemHandles[0]);

                // Calculate the copy size:
                gtUInt64 imageCopySize = imagePixelSize * amountOfPixels;

                // Add the image copy size to the reader:
                _openCLQueueCountersReader.addToCopySize(imageCopySize);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_CL_ND_RANGE_KERNEL_COMMAND:
        {
            // Add the work item count to the reader:
            apCLNDRangeKernelCommand* pNDRangeCommand = (apCLNDRangeKernelCommand*)aptrCommand.pointedObject();
            GT_IF_WITH_ASSERT(pNDRangeCommand != NULL)
            {
                // Get the amount of work items:
                gtUInt64 workItemsCount = pNDRangeCommand->amountOfWorkItems();

                // Add the amount of work items to the counter:
                _openCLQueueCountersReader.addToWorkItemsCount(workItemsCount);
                retVal = true;
            }
        }
        break;


        default:
        {
            retVal = true;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueueMonitor::memObjectHandleToPixelSize
// Description:
// Arguments:   cl_mem memobj
// Return Val:  gtInt32
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtInt32 csCommandQueueMonitor::memObjectHandleToPixelSize(oaCLMemHandle memobj)
{
    gtInt32 retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT((memobj != OA_CL_NULL_HANDLE) && (_pContextMonitor != NULL))
    {
        // Get the textures and buffers monitor:
        const csImagesAndBuffersMonitor& texturesAndBuffersMonitor = _pContextMonitor->imagesAndBuffersMonitor();

        // Get the memory object:
        const apCLMemObject* pMemoryObject = texturesAndBuffersMonitor.getMemObjectDetails(memobj);
        GT_IF_WITH_ASSERT(pMemoryObject != NULL)
        {
            // This memory object is supposed to be an image:
            GT_IF_WITH_ASSERT(pMemoryObject->type() == OS_TOBJ_ID_CL_IMAGE)
            {
                // Convert to an image object:
                const apCLImage* pTexture = (const apCLImage*)pMemoryObject;
                GT_IF_WITH_ASSERT(pTexture != NULL)
                {
                    // Get the texture pixel size:
                    retVal = pTexture->pixelSize();
                }
            }
        }
    }
    return retVal;
}

