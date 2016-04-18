//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCommandQueuesMonitor.cpp
///
//==================================================================================

//------------------------------ csCommandQueuesMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/csCommandQueueMonitor.h>
#include <src/csCommandQueuesMonitor.h>
#include <src/csOpenCLMonitor.h>


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::csCommandQueuesMonitor
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
csCommandQueuesMonitor::csCommandQueuesMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::~csCommandQueuesMonitor
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
csCommandQueuesMonitor::~csCommandQueuesMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::updateContextDataSnapshot
// Description: Updates command queues data snapshot
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool csCommandQueuesMonitor::updateContextDataSnapshot()
{
    bool retVal = true;

    // Iterate the queues:
    int numberOfQueues = (int)_commandQueuesMonitors.size();

    for (int i = 0; i < numberOfQueues; i++)
    {
        csCommandQueueMonitor* pCurrCommandQueue = _commandQueuesMonitors[i];
        GT_IF_WITH_ASSERT(pCurrCommandQueue != NULL)
        {
            bool rc = pCurrCommandQueue->updateContextDataSnapshot();
            retVal = retVal && rc;
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onCommandQueueCreation
// Description: Handles command queue creation
// Arguments: oaCLCommandQueueHandle commandQueueHandle
//            oaCLDeviceID deviceId
//            cl_command_queue_properties properties
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onCommandQueueCreation(cl_command_queue commandQueueHandle, cl_context contextHandle, cl_device_id deviceId, cl_command_queue_properties properties, cl_uint queueSize)
{
    // Handle default queue size:
    bool isOnDevice = 0 != (properties & CL_QUEUE_ON_DEVICE);
    GT_ASSERT(isOnDevice || (0 == queueSize)); // Queue size is only relevant for on-device queues.

    if (isOnDevice)
    {
        if (0 == queueSize)
        {
            const csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
            const apCLDevice* pDevice = theOpenCLMonitor.devicesMonitor().getDeviceObjectDetails((oaCLDeviceID)deviceId);
            GT_IF_WITH_ASSERT(NULL != pDevice)
            {
                queueSize = (cl_uint)pDevice->queueOnDevicePreferredSize();
            }
        }
    }

    // Get the command queue's context id:
    int contextSpyId = -1;
    const csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor((oaCLContextHandle)contextHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the context's spy id:
        contextSpyId = pContextMonitor->spyId();
    }

    // Get the device's API Id:
    int deviceIndex = theOpenCLMonitor.devicesMonitor().getDeviceObjectAPIID((oaCLDeviceID)deviceId);

    // Check if performance counters should be monitored:
    bool shouldInitPerformanceCounters = theOpenCLMonitor.shouldInitializePerformanceCounters();

    // Create a monitor for the new command queue:
    int commandQueueIndex = (int)_commandQueuesMonitors.size();
    csCommandQueueMonitor* pNewQueue = new csCommandQueueMonitor(contextSpyId, commandQueueIndex, (oaCLCommandQueueHandle)commandQueueHandle, (oaCLContextHandle)contextHandle, deviceIndex, shouldInitPerformanceCounters);

    // Set the command queue's info:
    apCLCommandQueue& newQueueInfo = pNewQueue->commandQueueInfo();
    bool outOfOrderModeEnabled = ((properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0);
    newQueueInfo.setOutOfOrderExecutionModeEnable(outOfOrderModeEnabled);
    bool profilingModeEnabled = ((properties & CL_QUEUE_PROFILING_ENABLE) != 0);
    newQueueInfo.setProfilingModeEnable(profilingModeEnabled);
    bool queueIsOnDevice = (0 != (properties & CL_QUEUE_ON_DEVICE));
    newQueueInfo.setQueueOnDevice(queueIsOnDevice);
    bool queueIsOnDeviceDefault = (0 != (properties & CL_QUEUE_ON_DEVICE_DEFAULT));
    newQueueInfo.setIsDefaultOnDeviceQueue(queueIsOnDeviceDefault);
    newQueueInfo.setQueueSize((gtUInt32)queueSize);

    // Add the command queue's monitor to our vector:
    _commandQueuesMonitors.push_back(pNewQueue);
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onCommandQueueCreationWithProperties
// Description: Handles command queue creation
// Author:      Uri Shomroni
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onCommandQueueCreationWithProperties(cl_command_queue commandQueueHandle, cl_context contextHandle, cl_device_id deviceId, const cl_queue_properties* properties)
{
    cl_command_queue_properties queueProps = 0;
    cl_uint queueSize = 0;

    if (NULL != properties)
    {
        const cl_queue_properties* pCurrentProp = properties;

        while (0 != *pCurrentProp)
        {
            cl_queue_properties prop = *pCurrentProp++;
            cl_queue_properties value = *pCurrentProp++;

            switch (prop)
            {
                case CL_QUEUE_PROPERTIES:
                    queueProps = (cl_command_queue_properties)value;
                    break;

                case CL_QUEUE_SIZE:
                    queueSize = (cl_uint)value;
                    break;

                default:
                    // Ignore
                    break;
            }
        }
    }

    // Call the static version of the function:
    onCommandQueueCreation(commandQueueHandle, contextHandle, deviceId, queueProps, queueSize);
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::commandQueueIndex
// Description: Gets the vector index of the command queue pointed by
//              commandQueueHandle, or -1 on failure.
// Author:      Uri Shomroni
// Date:        1/12/2009
// ---------------------------------------------------------------------------
int csCommandQueuesMonitor::commandQueueIndex(oaCLCommandQueueHandle commandQueueHandle) const
{
    int retVal = -1;

    int numberOfQueues = (int)_commandQueuesMonitors.size();

    for (int i = 0; i < numberOfQueues; i++)
    {
        const csCommandQueueMonitor* pCurrentQueueMtr = _commandQueuesMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentQueueMtr != NULL)
        {
            if (pCurrentQueueMtr->commandQueueInfo().commandQueueHandle() == commandQueueHandle)
            {
                retVal = i;

                // If there are multiple queues with the same handle, prefer one that is living (re-used handle)
                // over ones that aren't:
                if (!pCurrentQueueMtr->commandQueueInfo().wasMarkedForDeletion())
                {
                    break;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::commandQueueMonitor
// Description: Inputs a queue index and returns it's monitor.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
const csCommandQueueMonitor* csCommandQueuesMonitor::commandQueueMonitor(int queueIndex) const
{
    const csCommandQueueMonitor* retVal = NULL;

    int numberOfQueues = (int)_commandQueuesMonitors.size();
    GT_IF_WITH_ASSERT((0 <= queueIndex) && (queueIndex < numberOfQueues))
    {
        retVal = _commandQueuesMonitors[queueIndex];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::commandQueueMonitor
// Description: Inputs a queue index and returns it's monitor.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
csCommandQueueMonitor* csCommandQueuesMonitor::commandQueueMonitor(int queueIndex)
{
    csCommandQueueMonitor* retVal = NULL;

    int numberOfQueues = (int)_commandQueuesMonitors.size();
    GT_IF_WITH_ASSERT((0 <= queueIndex) && (queueIndex < numberOfQueues))
    {
        retVal = _commandQueuesMonitors[queueIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onCommandQueuePropertiesSet
// Description: Called when clSetCommandQueueProperty is used to set properties
//              on or off on a queue.
// Author:      Uri Shomroni
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onCommandQueuePropertiesSet(cl_command_queue commandQueueHandle, cl_command_queue_properties properties, cl_bool enable)
{
    // Get the commands queue monitor appropriate to the input command queue handle:
    int queueIndex = commandQueueIndex((oaCLCommandQueueHandle)commandQueueHandle);
    GT_IF_WITH_ASSERT(0 <= queueIndex)
    {
        csCommandQueueMonitor* pCommandQueueMtr = commandQueueMonitor(queueIndex);
        GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
        {
            pCommandQueueMtr->onCommandQueuePropertiesSet(properties, enable);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onDebuggedProcessSuspended
// Description: Called when the debugged process is suspended
// Author:      Uri Shomroni
// Date:        7/12/2009
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onDebuggedProcessSuspended()
{
    // TO_DO: OpenCL - do we need this function?
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onDebuggedProcessResumed
// Description: Called when the debugged process is resumed from being suspended
// Author:      Uri Shomroni
// Date:        7/12/2009
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onDebuggedProcessResumed()
{
    // Notify all queues of this event:
    int numberOfQueues = (int)_commandQueuesMonitors.size();

    for (int i = 0; i < numberOfQueues; i++)
    {
        csCommandQueueMonitor* pCurrentQueueMonitor = _commandQueuesMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentQueueMonitor != NULL)
        {
            pCurrentQueueMonitor->onDebuggedProcessResumed();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onEnqueueNDRangeKernel
// Description: Is called when an NDRange kernel is enqueued.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onEnqueueNDRangeKernel(oaCLCommandQueueHandle commandQueueHandle)
{
    (void)(commandQueueHandle); // unused

    if (csOpenCLMonitor::instance().shouldInitializePerformanceCounters())
    {
        // Currently, AMD's OpenCL performance counters are only available on Windows.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            // If the stop watch is not running:
            bool isStopWatchRunning = _amdCountersSampleWatch.isRunning();

            if (!isStopWatchRunning)
            {
                // Start it's run:
                _amdCountersSampleWatch.start();
            }
            else
            {
                // Check if 100MS have passed since the last check:
                double timePassed = 0;
                _amdCountersSampleWatch.getTimeInterval(timePassed);

                if (0.01 < timePassed)
                {
                    // Get the command queue monitor associated with the handle we got:
                    int queueIndex = commandQueueIndex(commandQueueHandle);
                    GT_IF_WITH_ASSERT(0 < queueIndex)
                    {
                        csCommandQueueMonitor* pCommandQueue = commandQueueMonitor(queueIndex);
                        GT_IF_WITH_ASSERT(pCommandQueue != NULL)
                        {
                            // Notify AMD's performance counters that it should
                            pCommandQueue->AMDPerformanceCountersReader().onFrameTerminatorCall();
                        }
                    }

                    // Restart the stop watch:
                    _amdCountersSampleWatch.stop();
                    _amdCountersSampleWatch.start();
                }
            }
        }
#endif // OA_DEBUGGER_USE_AMD_GPA
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    }
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::amountOfNotDeletedQueues
// Description: Get the amount of not deleted command queues
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        18/7/2010
// ---------------------------------------------------------------------------
int csCommandQueuesMonitor::amountOfNotDeletedQueues() const
{
    int retVal = 0;

    // Iterate the queues:
    for (int i = 0; i < (int) _commandQueuesMonitors.size(); i++)
    {
        // Get the command queue monitor:
        const csCommandQueueMonitor* pQueueMonitor = _commandQueuesMonitors[i];

        if (pQueueMonitor != NULL)
        {
            const apCLCommandQueue& commandQueue = pQueueMonitor->commandQueueInfo();

            if (!commandQueue.wasMarkedForDeletion())
            {
                retVal ++;
            }
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csCommandQueuesMonitor::onFrameTerminatorCall
// Description: Handle frame termination
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/7/2010
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::onFrameTerminatorCall()
{
    // Get the OpenCL monitor:
    // const csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();

    // Iterate the queues:
    for (int i = 0; i < (int) _commandQueuesMonitors.size(); i++)
    {
        // Get the command queue monitor:
        csCommandQueueMonitor* pCommandQueueMonitor = _commandQueuesMonitors[i];

        if (pCommandQueueMonitor != NULL)
        {
            // Get the OpenCL Queue counters reader:
            csOpenCLQueuePerformanceCountersReader& openCLQueueCountersReader = pCommandQueueMonitor->openCLQueueCountersReader();

            // Inform each of the command queues of the frame termination:
            openCLQueueCountersReader.onFrameTerminatorCall();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csCommandQueuessMonitor::checkForReleasedQueues
// Description: Checks if any of the objects monitored by this class have been released
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csCommandQueuesMonitor::checkForReleasedQueues()
{
    // Collect the object handles:
    gtVector<oaCLCommandQueueHandle> queueHandles;
    int numberOfQueues = (int)_commandQueuesMonitors.size();

    for (int i = 0; i < numberOfQueues; i++)
    {
        const csCommandQueueMonitor* pQueue = _commandQueuesMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pQueue)
        {
            // No need to check queues we already know were deleted:
            if (!pQueue->commandQueueInfo().wasMarkedForDeletion())
            {
                queueHandles.push_back(pQueue->commandQueueInfo().commandQueueHandle());
            }
        }
    }

    // Check each one. This is done separately, since finding an object
    // that was marked for deletion will release it:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    int queuesFound = (int)queueHandles.size();

    for (int i = 0; i < queuesFound; i++)
    {
        theOpenCLMonitor.checkIfCommandQueueWasDeleted((cl_command_queue)queueHandles[i], false);
    }
}

