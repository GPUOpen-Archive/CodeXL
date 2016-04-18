//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csMemoryMonitor.cpp
///
//==================================================================================

//------------------------------ csMemoryMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apContextDataSnapshotWasUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessIsDuringTerminationEvent.h>
#include <AMDTAPIClasses/Include/Events/apSearchingForMemoryLeaksEvent.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/csOpenCLMonitor.h>
#include <src/csMemoryMonitor.h>

// Static members initializations:
csMemoryMonitor* csMemoryMonitor::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::instance
// Description: Returns the single instance of the gsOpenGLMonitor class
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
csMemoryMonitor& csMemoryMonitor::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new csMemoryMonitor;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::csMemoryMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
csMemoryMonitor::csMemoryMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::~csMemoryMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
csMemoryMonitor::~csMemoryMonitor()
{
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::beforeContextDeletion
// Description: Perform a memory leaks check before a context is deleted
// Arguments:   int contextID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
bool csMemoryMonitor::beforeContextDeletion(const apContextID& deleteContextID)
{
    bool retVal = true;

    // Check for memory leaks for the OpenCL context:
    checkForMemoryLeaksOnOpenCLContextReleased(deleteContextID);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::beforeComputationProgramReleased
// Description: Perform a memory leak check before a program is deleted
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool csMemoryMonitor::beforeComputationProgramReleased(const apContextID& contextID, int programIndex)
{
    bool retVal = true;

    checkForMemoryLeaksOnOpenCLProgramReleased(contextID, programIndex);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::beforeSpyTermination
// Description: Handle Spy termination
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
bool csMemoryMonitor::beforeSpyTermination()
{
    bool retVal = true;

    // Check for global memory leaks:
    checkForMemoryLeaksOnOpenCLUnloaded();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::checkForMemoryLeaksOnOpenCLUnloaded
// Description: Check for memory leaks when the OpenCL driver is unloaded.
//              Here we check for OpenCL context-independent items (contexts...)
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void csMemoryMonitor::checkForMemoryLeaksOnOpenCLUnloaded()
{
    // Send a "Searching for memory leaks" event:
    apSearchingForMemoryLeaksEvent searchingEvent(SU_STR_MemoryLeakSearchingForCLMemoryLeaksOnApplicationExit);
    bool rcEve = suForwardEventToClient(searchingEvent);
    GT_ASSERT(rcEve);

    // Define a memory leak event:
    apMemoryLeakEvent memoryLeakEvent;

    // Set the return event type:
    memoryLeakEvent.setLeakType(apMemoryLeakEvent::AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK);

    // Get the render contexts amount:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    int amountOfCtxs = theOpenCLMonitor.amountOfContexts();
    unsigned int numberOfLeakedCtxs = 0;
    int firstLeakedContext = -1;

    // Iterate the render context and search for not deleted contexts:
    for (int contextID = 1; contextID < amountOfCtxs; contextID++)
    {
        // Get the queried render context monitor:
        const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(contextID);
        GT_IF_WITH_ASSERT(nullptr != pContextMonitor)
        {
            // Check if the render context was deleted from the OS:
            const apCLContext& ctxProps = pContextMonitor->contextInformation();
            bool wasContextDeleted = ctxProps.wasMarkedForDeletion();

            if (!wasContextDeleted)
            {
                // See if the context was released by now:
                theOpenCLMonitor.checkIfContextWasDeleted((cl_context)ctxProps.contextHandle());
            }

            wasContextDeleted = ctxProps.wasMarkedForDeletion();

            if (!wasContextDeleted)
            {
                // Count the still existing contexts:
                numberOfLeakedCtxs++;

                if (-1 < firstLeakedContext)
                {
                    firstLeakedContext = contextID;
                }

                // Note this in the event:
                memoryLeakEvent.addLeakingComputeContext(contextID);

                // Calculate the memory leak for this context:
                gtUInt64 contextMemoryLeak = 0;
                bool rcCalculateMemoryLeakSize = calculateMemoryLeakForContext(pContextMonitor, contextMemoryLeak);
                GT_IF_WITH_ASSERT(rcCalculateMemoryLeakSize)
                {
                    // Add the context memory leak to the event:
                    memoryLeakEvent.addMemoryLeakSize(contextMemoryLeak);
                }
            }
        }
    }

    if (memoryLeakEvent.memoryLeakExists())
    {
        bool rcEve2 = suForwardEventToClient(memoryLeakEvent);
        GT_ASSERT(rcEve2);

        // Check if we should break on memory leaks:
        bool shouldBreakOnMemoryLeaks = su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_MEMORY_LEAK);

        // If a memory leak was detected by a spy update function, we do not want to trigger a breakpoint, to avoid hangs in the client:
        bool isAPIThread = suIsSpiesAPIThreadId(osGetCurrentThreadId());

        if (shouldBreakOnMemoryLeaks && !isAPIThread)
        {
            // Report that process is during termination:
            apDebuggedProcessIsDuringTerminationEvent duringTerminationEvent;
            bool rcEve3 = suForwardEventToClient(duringTerminationEvent);
            GT_ASSERT(rcEve3);

            // Set the current break reason:
            su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
            su_stat_theBreakpointsManager.setBreakReason(AP_MEMORY_LEAK_BREAKPOINT_HIT);

            // Get the current context id:
            apContextID triggeringContextID(AP_OPENCL_CONTEXT, firstLeakedContext);

            // Uri, 20/12/09: We check if the API thread exists, since this function could be called either when the
            // OpenGL spy is unloaded or when the app terminates. If the latter, the API thread should already be dead...
            bool apiThreadExists = suIsAPIThreadRunning();

            if (!apiThreadExists)
            {
                // Let the debugger know we are about to enter the termination loop:
                suBeforeEnteringTerminationAPILoop();
            }

            // Trigger the breakpoint exception:
            su_stat_theBreakpointsManager.triggerBreakpointException(triggeringContextID, apMonitoredFunctionsAmount);
            su_stat_theBreakpointsManager.afterTriggeringBreakpoint();

            if (!apiThreadExists)
            {
                suRunTerminationAPILoop();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::checkForMemoryLeaksOnOpenCLContextReleased
// Description: Check for memory leaks when an OpenCL context is released.
//              Here we check for OpenCL context-dependent items (textures, buffers...)
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void csMemoryMonitor::checkForMemoryLeaksOnOpenCLContextReleased(const apContextID& deletedContextID)
{
    // Sanity check:
    GT_ASSERT(deletedContextID.isOpenCLContext());
    int deletedContextIndex = deletedContextID._contextId;

    // Send a "Searching for memory leaks" event:
    gtString searchingMessage;
    searchingMessage.appendFormattedString(SU_STR_MemoryLeakSearchingForMemoryLeaksOnCLContextDeletion, deletedContextIndex);
    apSearchingForMemoryLeaksEvent searchingEvent(searchingMessage);

    // Write the searching for memory leak event:
    bool rcEve = suForwardEventToClient(searchingEvent);
    GT_ASSERT(rcEve);

    // Initialize a memory leak event:
    apMemoryLeakEvent memoryLeakEvent;
    memoryLeakEvent.setLeakType(apMemoryLeakEvent::AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK);
    bool rcRC = memoryLeakEvent.setLeakingObjectsComputeContextID(deletedContextIndex);
    GT_ASSERT(rcRC);

    // Get the OpenCL monitor instance:
    const csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();

    // Get the context monitor:
    const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(deletedContextIndex);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // This functions is currently only called from the Context check, so there is no need to perform
        // an update here, since the context should already be updated by csOpenCLMonitor.
        // csContextMonitor* pMutableContext = ((csContextMonitor*)pContextMonitor);
        // pMutableContext->updateContextDataSnapshot(false, false);

        // Report that process is during termination:
        apContextDataSnapshotWasUpdatedEvent contextUpdatedEvent(deletedContextID, true);
        bool rcEve2 = suForwardEventToClient(contextUpdatedEvent);
        GT_ASSERT(rcEve2);

        // Check the number of not deleted command queues:
        const csCommandQueuesMonitor& commandQueuesMonitor = pContextMonitor->commandQueuesMonitor();
        int leakingCommandQueues = commandQueuesMonitor.amountOfNotDeletedQueues();

        // Events:
        const csEventsMonitor& eventsMonitor = pContextMonitor->eventsMonitor();
        int leakingEvents = eventsMonitor.amountOfEvents();

        // Buffers & Textures:
        const csImagesAndBuffersMonitor& textureAndBuffersMonitor = pContextMonitor->imagesAndBuffersMonitor();
        int leakingBuffers = textureAndBuffersMonitor.amountOfBuffers();
        int leakingImages = textureAndBuffersMonitor.amountOfImages();

        // Samplers:
        const csSamplersMonitor& samplersMonitor = pContextMonitor->samplersMonitor();
        int leakingSamplers = samplersMonitor.amountOfSamplers();

        // Programs:
        const csProgramsAndKernelsMonitor& programsMonitor = pContextMonitor->programsAndKernelsMonitor();
        int leakingPrograms = programsMonitor.amountOfNotDeletedPrograms();

        // Set the values into the event:
        memoryLeakEvent.setLeakingCLAllocatedObjects(leakingCommandQueues, leakingEvents, leakingBuffers, leakingImages, leakingSamplers, leakingPrograms);

        // Calculate the memory leak size:
        gtUInt64 contextMemoryLeak = 0;
        bool rcCalculateMemoryLeakSize = calculateMemoryLeakForContext(pContextMonitor, contextMemoryLeak);
        GT_IF_WITH_ASSERT(rcCalculateMemoryLeakSize)
        {
            // Add the context memory leak to the event:
            memoryLeakEvent.addMemoryLeakSize(contextMemoryLeak);
        }

        // Check if memory leaks was found:
        if (memoryLeakEvent.memoryLeakExists())
        {
            // Write the memory leak event:
            bool rcEve3 = suForwardEventToClient(memoryLeakEvent);
            GT_ASSERT(rcEve3);

            // Check if we should break on memory leaks:
            bool shouldBreakOnMemoryLeaks = su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_MEMORY_LEAK);

            // If a memory leak was detected by a spy update function, we do not want to trigger a breakpoint, to avoid hangs in the client:
            bool isAPIThread = suIsSpiesAPIThreadId(osGetCurrentThreadId());

            if (shouldBreakOnMemoryLeaks && !isAPIThread)
            {
                su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
                su_stat_theBreakpointsManager.setBreakReason(AP_MEMORY_LEAK_BREAKPOINT_HIT);

                // Get the release function Id:
                apMonitoredFunctionId monitoredFunctionId = apMonitoredFunctionsAmount;
                const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
                GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
                {
                    monitoredFunctionId = pCallsHistoryLogger->lastCalledFunctionId();
                }

                // Trigger the breakpoint exception:
                su_stat_theBreakpointsManager.triggerBreakpointException(deletedContextID, monitoredFunctionId);
                su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::checkForMemoryLeaksOnOpenCLProgramReleased
// Description: Check for memory leaks when an OpenCL program is released.
//              Here we check for OpenCL program-dependent items (kernels...)
// Author:      Uri Shomroni
// Date:        28/7/2010
// ---------------------------------------------------------------------------
void csMemoryMonitor::checkForMemoryLeaksOnOpenCLProgramReleased(const apContextID& deletedProgramContextID, int deletedProgramIndex)
{
    // Sanity check:
    GT_ASSERT(deletedProgramContextID.isOpenCLContext());
    int deletedContextIndex = deletedProgramContextID._contextId;

    // Send a "Searching for memory leaks" event:
    gtString searchingMessage;
    searchingMessage.appendFormattedString(SU_STR_MemoryLeakSearchingForMemoryLeaksOnCLProgramDeletion, deletedContextIndex, deletedProgramIndex + 1);
    apSearchingForMemoryLeaksEvent searchingEvent(searchingMessage);

    // Write the searching for memory leak event:
    bool rcEve = suForwardEventToClient(searchingEvent);
    GT_ASSERT(rcEve);

    // Initialize a memory leak event:
    apMemoryLeakEvent memoryLeakEvent;
    memoryLeakEvent.setLeakType(apMemoryLeakEvent::AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK);
    bool rcProg = memoryLeakEvent.setLeakingObjectsComputationProgramID(deletedContextIndex, deletedProgramIndex);
    GT_ASSERT(rcProg);

    // Get the OpenCL monitor instance:
    const csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();

    // Get the context monitor:
    const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(deletedContextIndex);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Programs:
        const csProgramsAndKernelsMonitor& programsMonitor = pContextMonitor->programsAndKernelsMonitor();

        // Set the leaked programs:
        int leakingKernels = programsMonitor.amountOfNotDeletedKernelsInProgram(deletedProgramIndex);
        bool rcObj = memoryLeakEvent.setLeakingProgramAllocatedCLObjects(leakingKernels);
        GT_ASSERT(rcObj);

        // Calculate the memory leak size:
        gtUInt64 programMemoryLeakSize = 0;
        bool rcSize = calculateMemoryLeakForProgram(pContextMonitor, deletedProgramIndex, programMemoryLeakSize);
        GT_IF_WITH_ASSERT(rcSize)
        {
            // Add the context memory leak to the event:
            memoryLeakEvent.addMemoryLeakSize(programMemoryLeakSize);
        }

        // Check if memory leaks was found:
        if (memoryLeakEvent.memoryLeakExists())
        {
            // Write the memory leak event:
            bool rcEve2 = suForwardEventToClient(memoryLeakEvent);
            GT_ASSERT(rcEve2);

            // Check if we should break on memory leaks:
            bool shouldBreakOnMemoryLeaks = su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_MEMORY_LEAK);

            // If a memory leak was detected by a spy update function, we do not want to trigger a breakpoint, to avoid hangs in the client:
            bool isAPIThread = suIsSpiesAPIThreadId(osGetCurrentThreadId());

            if (shouldBreakOnMemoryLeaks && !isAPIThread)
            {
                su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
                su_stat_theBreakpointsManager.setBreakReason(AP_MEMORY_LEAK_BREAKPOINT_HIT);

                // Get the release function Id:
                apMonitoredFunctionId monitoredFunctionId = apMonitoredFunctionsAmount;
                const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
                GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
                {
                    monitoredFunctionId = pCallsHistoryLogger->lastCalledFunctionId();
                }

                // Trigger the breakpoint exception:
                su_stat_theBreakpointsManager.triggerBreakpointException(deletedProgramContextID, monitoredFunctionId);
                su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::calculateMemoryLeakForContext
// Description: Calculate a memory leak for a context
// Arguments:   pContextMonitor - the context monitor
//              gtUInt64& contextMemorySize - output. the context memory size
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool csMemoryMonitor::calculateMemoryLeakForContext(const csContextMonitor* pContextMonitor, gtUInt64& contextMemorySize)
{
    bool retVal = true;

    contextMemorySize = 0;
    bool rcTexturesMemory = false;
    bool rcBuffersMemory = false;

    // Buffers & Textures:
    const csImagesAndBuffersMonitor& textureAndBuffersMonitor = pContextMonitor->imagesAndBuffersMonitor();

    // Textures memory size:
    gtUInt64 texturesLeakMemorySize = 0;
    rcTexturesMemory = textureAndBuffersMonitor.calculateImagesMemorySize(texturesLeakMemorySize);

    // Buffers memory size:
    gtUInt64 buffersLeakMemorySize = 0;
    rcBuffersMemory = textureAndBuffersMonitor.calculateBuffersMemorySize(buffersLeakMemorySize);

    retVal = rcTexturesMemory && rcBuffersMemory;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csMemoryMonitor::calculateMemoryLeakForProgram
// Description: Get the memory leak size for a program
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/7/2010
// ---------------------------------------------------------------------------
bool csMemoryMonitor::calculateMemoryLeakForProgram(const csContextMonitor* pContextMonitor, int deletedProgramIndex, gtUInt64& contextMemorySize)
{
    (void)(pContextMonitor); // unused
    (void)(deletedProgramIndex); // unused
    bool retVal = true;

    // TO_DO: A memory leak of a program is the total size of the undeleted kernels' arguments.
    // Right now, we treat them as insignificant, like GL programs:
    contextMemorySize = 0;

    return retVal;
}


