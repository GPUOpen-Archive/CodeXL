//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLMonitor.cpp
///
//==================================================================================

//------------------------------ csOpenCLMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildFailedWithDebugFlagsEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>

// Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include <AMDTHsaDebugging/Include/hdGlobalVariables.h>
    #include <AMDTHsaDebugging/Include/hdHSAHardwareBasedDebuggingManager.h>
#endif

// AMD CL Debugging API:
#include <AMDOpenCLDebug.h>

// Local:
#include <Include/csPublicStringConstants.h>
#include <src/csAPIFunctionsImplementations.h>
#include <src/csGlobalVariables.h>
#include <src/csMemoryMonitor.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLMonitor.h>
#include <src/csOpenCLServerInitialization.h>
#include <src/csStringConstants.h>

// Static members initializations:
csOpenCLMonitor* csOpenCLMonitor::_pMySingleInstance = NULL;
#define CS_AMOUNT_OF_CALLS 5

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::instance
// Description: Returns the single instance of the csOpenCLMonitor class
// Author:      Yaki Tebeka
// Date:        29/10/2009
// ---------------------------------------------------------------------------
csOpenCLMonitor& csOpenCLMonitor::instance()
{
    // If this class single instance was not already created:
    if (_pMySingleInstance == NULL)
    {
        // Create it:
        _pMySingleInstance = new csOpenCLMonitor;
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::csOpenCLMonitor
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        29/10/2009
// ---------------------------------------------------------------------------
csOpenCLMonitor::csOpenCLMonitor():
    suITechnologyMonitor(), _wasOpenCLServerInitialized(false), _isCommandQueuesProfileModeForced(true), _breakOnOpenCLErrors(false), _functionsCounter(0), _lastFunctionContextId(-1)
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    , _AMDPerformanceCountersMgr(_shouldInitializePerformanceCounters)
#endif
#endif
{
    // Initialize genetic breakpoints - generic breakpoints are initialized before the monitors
    // are initialized. At this stage the monitor should check the status of each generic breakpoint
    // and set it. This must be done here, and not in base class since onGenericBreakpointSet is a virtual function:
    for (int i = (int)AP_BREAK_ON_GL_ERROR; i < (int)AP_AMOUNT_OF_GENERIC_BREAKPOINT_TYPES; i++)
    {
        // Check if the generic type is on:
        apGenericBreakpointType genericType = (apGenericBreakpointType)i;
        bool isOn = suBreakpointsManager::instance().breakOnGenericBreakpoint(genericType);
        onGenericBreakpointSet(genericType, isOn);
    }
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::~csOpenCLMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        29/10/2009
// ---------------------------------------------------------------------------
csOpenCLMonitor::~csOpenCLMonitor()
{
    if (_shouldInitializePerformanceCounters)
    {
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            bool rc = _AMDPerformanceCountersMgr.terminate();
            GT_ASSERT(rc);
        }
#endif
#endif
    }

    // The OpenCL monitor is only destroyed during the OpenCL spy termination:
    bool rcTer = csPerformOpenCLMonitorTerminationActions();
    GT_ASSERT(rcTer);
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onDebuggedProcessTerminationAlert
// Description: Called before the debugged process is terminated.
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onDebuggedProcessTerminationAlert()
{
    // Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    hd_stat_hsaHardwareBasedDebuggingManager.terminate();
#endif
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::beforeDebuggedProcessSuspended
// Description: Called before the debugged process is suspended.
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::beforeDebuggedProcessSuspended()
{
    suIKernelDebuggingManager::synchronizeWithKernelDebuggingCallback();

    int numberOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < numberOfContexts; i++)
    {
        suContextMonitor* pCurrentContext = _contextsMonitors[i];

        if (pCurrentContext != NULL)
        {
            // Sanity check:
            if (pCurrentContext->contextID().isOpenCLContext())
            {
                // Notify the Context monitor:
                ((csContextMonitor*)pCurrentContext)->onDebuggedProcessSuspended();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::afterDebuggedProcessResumed
// Description: Called once the debugged process is resumed from suspension.
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::afterDebuggedProcessResumed()
{
    int numberOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < numberOfContexts; i++)
    {
        suContextMonitor* pCurrentContext = _contextsMonitors[i];

        if (pCurrentContext != NULL)
        {
            // Sanity check:
            if (pCurrentContext->contextID().isOpenCLContext())
            {
                // Notify the Context monitor:
                ((csContextMonitor*)pCurrentContext)->onDebuggedProcessResumed();
            }
        }
    }

    // Release the kernel debugging condition:
    // The pointer can be null when the application is closing
    if (cs_stat_pIKernelDebuggingManager != NULL)
    {
        cs_stat_pIKernelDebuggingManager->releaseCallbackToNextCommand();
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::beforeBreakpointException
// Description: Called by a thread before a breakpoint exception is thrown by it
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::beforeBreakpointException(bool isInOpenGLBeginEndBlock)
{
    (void)(isInOpenGLBeginEndBlock); // unused
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::afterBreakpointException
// Description: Called by a thread after a breakpoint exception it threw is passed
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::afterBreakpointException(bool isInOpenGLBeginEndBlock)
{
    (void)(isInOpenGLBeginEndBlock); // unused
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onDebuggedProcessExecutionModeChanged
// Description: Called when the debugged process execution mode is changed to newExecutionMode
// Author:      Uri Shomroni
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode)
{
    (void)(newExecutionMode); // unused
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onContextCreation
// Description: Is called when an OpenCL context is created.
//              The function is NOT CALLED for the null context!
// Arguments:   contextHandle - Handle to the created context.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onContextCreation(cl_context contextHandle, apMonitoredFunctionId creationFunc)
{
    // Create a context monitor that will represent this OpenCL context:
    int contextId = (int)_contextsMonitors.size();
    oaCLContextHandle hContext = OA_CL_NULL_HANDLE;

    if (contextHandle != NULL)
    {
        hContext = (oaCLContextHandle)contextHandle;
    }

    // Check if the current handle is reused:
    csContextMonitor* pExistingContextMonitor = clContextMonitor((oaCLContextHandle) contextHandle);

    if (pExistingContextMonitor != NULL)
    {
        pExistingContextMonitor->markHandleAsReused();
    }

    // Create the new context monitor:
    csContextMonitor* pOpenCLContextMonitor = new csContextMonitor(hContext, contextId, creationFunc);

    // Calculate the compute context's spy id:
    int contextSpyId = (int)_contextsMonitors.size();

    // Log the existence of this context:
    _contextsMonitors.push_back(pOpenCLContextMonitor);

    // If the text log files are active:
    bool areHTMLLogFilesActive = suAreHTMLLogFilesActive();

    if (areHTMLLogFilesActive)
    {
        // Start log file recording:
        suCallsHistoryLogger* pCallsLogger = pOpenCLContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            pCallsLogger->startHTMLLogFileRecording();
        }
    }

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = openCLHandleMonitor();
    int objectIndex = (int)_contextsMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)contextHandle, contextId, objectIndex, OS_TOBJ_ID_CL_CONTEXT);

    // Output debug message:
    gtString debugMsg;
    debugMsg.appendFormattedString(CS_STR_computeContextWasCreated, contextSpyId);
    GT_IF_WITH_ASSERT(apMonitoredFunctionsAmount > creationFunc)
    {
        debugMsg.append(L" (").append(apMonitoredFunctionsManager::instance().monitoredFunctionName(creationFunc)).append(')');
    }
    OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);

    // Notify the debugger about the context creation:
    osThreadId currentThreadId = osGetCurrentThreadId();
    apComputeContextCreatedEvent computeContextCreatedEvent(currentThreadId, contextSpyId);
    bool rcEve = suForwardEventToClient(computeContextCreatedEvent);
    GT_ASSERT(rcEve);
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onProgramCreation
// Description: Is called when an OpenCL program is created from source code.
// Arguments: contextHandle - Handle to the context in which the program resides.
//            programHandle - Handle to the newly created program.
//            count - strings amount.
//            strings - source code strings array.
//            lengths - An array with the number of chars in each string.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onProgramCreation(cl_context contextHandle, cl_program programHandle, cl_uint count, const char** strings, const size_t* lengths)
{
    // Look for the context in which the program was created:
    csContextMonitor* pContextMonitor = clContextMonitor((oaCLContextHandle)contextHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the program's source code as a single string:
        gtASCIIString programSourceCode;
        apCLMultiStringParameter sourceCodeAsMultiString(count, strings, lengths);
        sourceCodeAsMultiString.valueAsString(programSourceCode);

        // Log the program creation at the appropriate context monitor:
        pContextMonitor->onProgramCreation(programHandle, programSourceCode);

        // Notify the debugger about the program creation:
        // Get the program details:
        const csProgramsAndKernelsMonitor& contextProgramsAndKernelsMonitor = pContextMonitor->programsAndKernelsMonitor();
        const apCLProgram* pProgramDetails = contextProgramsAndKernelsMonitor.programMonitor((oaCLProgramHandle)programHandle);
        GT_IF_WITH_ASSERT(pProgramDetails != NULL)
        {
            // Get the current thread id:
            osThreadId currentThreadId = osGetCurrentThreadId();

            // Send a creation event:
            apOpenCLProgramCreatedEvent programCreatedEvent(currentThreadId, pContextMonitor->spyId(), contextProgramsAndKernelsMonitor.programIndexFromHandle((oaCLProgramHandle)programHandle), pProgramDetails->sourceCodeFilePath());
            bool rcEve = suForwardEventToClient(programCreatedEvent);
            GT_ASSERT(rcEve);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onKernelCreation
// Description: Is handling kernel creation
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onKernelCreation(cl_program program, cl_kernel kernel, const gtString& kernelName)
{
    // Look for contexts containing the program object:
    csContextMonitor* pContextMonitor = contextContainingProgram((oaCLProgramHandle)program);

    // If we found the context containing the program object:
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        pContextMonitor->onKernelCreation(program, kernel, kernelName);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onKernelArgumentSet
// Description: Called when kernel's arg_index-th argument is set
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onKernelArgumentSet(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value, bool isSVMPointer)
{
    csContextMonitor* pContextMonitor = contextContainingKernel((oaCLKernelHandle)kernel);

    // If we found the context containing the kernel:
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        pContextMonitor->onKernelArgumentSet(kernel, arg_index, arg_size, arg_value, isSVMPointer);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onSamplerCreation
// Description: Called when a sampler is created
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onSamplerCreation(cl_context contextHandle, cl_sampler samplerHandle, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode)
{
    csContextMonitor* pContextMonitor = clContextMonitor((oaCLContextHandle)contextHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        pContextMonitor->onSamplerCreation(samplerHandle, normalizedCoords, addressingMode, filterMode);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onSamplerCreationWithProperties
// Description: Called when a sampler is created
// Author:      Uri Shomroni
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onSamplerCreationWithProperties(cl_context contextHandle, cl_sampler samplerHandle, const cl_sampler_properties* properties)
{
    csContextMonitor* pContextMonitor = clContextMonitor((oaCLContextHandle)contextHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        pContextMonitor->onSamplerCreationWithProperties(samplerHandle, properties);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onFrameTerminatorCall
// Description: Is called when a frame terminator function is called for hContext.
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onFrameTerminatorCall(oaCLContextHandle hContext)
{
    // In profile mode, we do the checks here instead of in addFunctionCall()
    // to slow down execution less often:
    su_stat_theBreakpointsManager.waitOnDebuggedProcessSuspension();

    // Get the current thread render context monitor:
    csContextMonitor* pContextMonitor = clContextMonitor(hContext);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // If we are supposed to break on the next frame terminator:
        if (su_stat_theBreakpointsManager.breakAtTheNextFrame())
        {
            // Clear the flag:
            su_stat_theBreakpointsManager.setBreakAtTheNextFrame(false);

            // Set the break reason as frame step breakpoint:
            su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
            su_stat_theBreakpointsManager.setBreakReason(AP_FRAME_BREAKPOINT_HIT);

            // Get the frame terminator function Id:
            apMonitoredFunctionId monitoredFunctionId = apMonitoredFunctionsAmount;
            const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
            GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
            {
                monitoredFunctionId = pCallsHistoryLogger->lastCalledFunctionId();
            }

            // Break the execution:
            apContextID contextID(AP_OPENCL_CONTEXT, pContextMonitor->spyId());
            su_stat_theBreakpointsManager.triggerBreakpointException(contextID, monitoredFunctionId);
            su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
        }

        // Notify the render context monitor:
        pContextMonitor->onFrameTerminatorCall();

        // If we are in profiling mode:
        apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

        if (debuggedProcessExecutionMode == AP_PROFILING_MODE)
        {
            // If we should break the debugged process execution:
            if (su_stat_theBreakpointsManager.isBreakDebuggedProcessExecutionFlagOn())
            {
                // Break the debugged process execution:
                apContextID contextId(AP_OPENCL_CONTEXT, pContextMonitor->spyId());
                su_stat_theBreakpointsManager.testAndTriggerBeforeFuncExecutionBreakpoints(ap_clFlush, contextId);
            }
        }
        else // debuggedProcessExecutionMode != AP_PROFILING_MODE
        {
            // Get the context monitor:
            csContextMonitor* pContextMonitorInner = clContextMonitor(hContext);
            GT_IF_WITH_ASSERT(pContextMonitorInner != NULL)
            {
                // Some monitors should only be notified in modes other than profiling mode:
                csCallsHistoryLogger* pCallsHistoryLogger = (csCallsHistoryLogger*)pContextMonitorInner->callsHistoryLogger();
                GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
                {
                    pCallsHistoryLogger->onFrameTerminatorCall();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::beforeMonitoredFunctionExecutionActions
// Description: Perform OpenGL monitor actions that should be executed BEFORE
//              the monitored function is executed.
// Arguments: apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Yaki Tebeka
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::beforeMonitoredFunctionExecutionActions(apMonitoredFunctionId monitoredFunctionId)
{
    // TO_DO: OpenCL - decide how to choose the right context:
    apContextID contextId(AP_OPENCL_CONTEXT, _lastFunctionContextId);

    // Do not perform the below actions while in profiling mode:
    apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

    if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
    {
        // Test and trigger BEFORE function execution breakpoints:
        su_stat_theBreakpointsManager.testAndTriggerBeforeFuncExecutionBreakpoints(monitoredFunctionId, contextId);
    }
    else
    {
        // Test and trigger profiling mode breakpoints:
        su_stat_theBreakpointsManager.testAndTriggerProfileModeBreakpoints(monitoredFunctionId, contextId);
    }
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::afterMonitoredFunctionExecutionActions
// Description: Perform OpenGL monitor actions that should be executed AFTER
//              the monitored function is executed.
// Author:      Yaki Tebeka
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId)
{
    // Do not perform the below actions in profiling mode:
    apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

    if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
    {
        GT_IF_WITH_ASSERT((_lastFunctionContextId >= 0) && (_lastFunctionContextId < (int)_contextsMonitors.size()))
        {
            // Get current function's context monitor:
            suContextMonitor* pContextMonitor = _contextsMonitors[_lastFunctionContextId];
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                // Will get true iff we are during context data snapshot update:
                bool isDuringContextDataUpdate = false;

                // If the render context is not during context data update:
                isDuringContextDataUpdate = pContextMonitor->isDuringContextDataUpdate();

                if (!isDuringContextDataUpdate)
                {
                    // Update state change function status:
                    pContextMonitor->afterMonitoredFunctionExecutionActions(calledFunctionId);
                }
            }
        }
    }

    // Leave the function call CS:
    _functionCallCS.leave();
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::addFunctionCall
// Description:
//   Logs a monitored function call into the active context call history logger.
//   Triggers monitored functions breakpoint events.
// Arguments: oaCLHandle objectCLHandle - contain the object that the function is 'working on'
//            This object is used to find out the context that the function belongs to.
//            apMonitoredFunctionId calledFunctionId - the logged function id.
//            argumentsAmount - The amount of function arguments.
//            ... - Function arguments in va_list style.
// Author:      Yaki Tebeka
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::addFunctionCall(oaCLHandle objectCLHandle, apMonitoredFunctionId calledFunctionId, int argumentsAmount, ...)
{
    // Enter the function CS:
    _functionCallCS.enter();

    // If the OpenGL Server was not initialized yet, initialize it:
    if (!_wasOpenCLServerInitialized)
    {
        csInitializeOpenCLServer();
        _wasOpenCLServerInitialized = true;
    }

    // If the process is suspended, wait here on platforms which do not allow
    // suspending threads by the debugger (iPhone Device, Linux):
    su_stat_theBreakpointsManager.waitOnDebuggedProcessSuspension();

    // Get the function context:
    _lastFunctionContextId = 0;

    if (objectCLHandle != OA_CL_NULL_HANDLE)
    {
        // Get the object id for this handle:
        apCLObjectID* pObjectID = _openCLHandlesMonitor.getCLHandleObjectDetails(objectCLHandle);

        if (pObjectID != NULL)
        {
            _lastFunctionContextId = pObjectID->_contextId;
        }
    }

    GT_IF_WITH_ASSERT((_lastFunctionContextId >= 0) && (_lastFunctionContextId < (int)_contextsMonitors.size()))
    {
        suContextMonitor* pContextMonitor = _contextsMonitors[_lastFunctionContextId];
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // Get a pointer to the arguments list:
            va_list pCurrentArgument;
            va_start(pCurrentArgument, argumentsAmount);

            // In OpenCL 1.1 there is one deprecated functions, so we check it here:
            apFunctionDeprecationStatus deprectionStatus = functionDeprecationStatus(calledFunctionId);

            // Add the function call to OpenCL calls logger:
            pContextMonitor->addFunctionCall(calledFunctionId, argumentsAmount, pCurrentArgument, deprectionStatus);
            va_end(pCurrentArgument);
        }
    }

    // Debugging section (don't remove me)
    // -----------------------------------
    bool foo1 = false;

    if (foo1)
    {
        gtVector<int> images;
        images.push_back(7);
        gaUpdateOpenCLImageRawDataImpl(2, images);

        //gaUpdateOpenCLContextDataSnapshotImpl(1);
    }

    // Execute before monitored function execution actions:
    beforeMonitoredFunctionExecutionActions(calledFunctionId);
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::shouldDebugKernel
// Description: Returns true iff we want to call the debug API for kernel instead
//              of the real clEnqueueNDRangeKernel function.
// Author:      Uri Shomroni
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::shouldDebugKernel(oaCLKernelHandle kernel, suIKernelDebuggingManager::KernelDebuggingSessionReason& reason, osCriticalSectionDelayedLocker& dispatchLock)
{
    //////////////////////////////////////////////////////////////////////////
    // Uri, 6/6/11 - note that this logic dictates that if we step in we won't
    // hit the kernel function name breakpoint, but if there are kernel source
    // breakpoints we will hit it. That is consistent with the Visual Studio
    // debug model. Don't change the order of the checks.
    //////////////////////////////////////////////////////////////////////////

    // First, wait for any existing dispatches, if needed.
    OS_OUTPUT_DEBUG_LOG(L"Calling beforeCheckingKernelDebuggingDisptach... ", OS_DEBUG_LOG_EXTENSIVE);
    bool rcAllowMult = suIKernelDebuggingManager::beforeCheckingKernelDebuggingDisptach(dispatchLock);
    OS_OUTPUT_DEBUG_LOG(L"... beforeCheckingKernelDebuggingDisptach ended.", OS_DEBUG_LOG_EXTENSIVE);

    // Stepping into a kernel makes us debug it:
    bool retVal = su_stat_theBreakpointsManager.breakInMonitoredFunctionCall();

    if (retVal)
    {
        // We're stepping in, clear the flag:
        reason = suIKernelDebuggingManager::STEP_IN_COMMAND;
        bool dummyBool = false;
        su_stat_theBreakpointsManager.setBreakInMonitoredFunctionCall(false, dummyBool);
    }
    else
    {
        // Check for a breakpoint on the kernel function name:
        // Get the containing context:
        const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingKernel(kernel);

        if (pContextMonitor != NULL)
        {
            // Get the kernel object:
            const csCLKernel* pKernelDetails = pContextMonitor->programsAndKernelsMonitor().kernelMonitor(kernel);

            if (pKernelDetails != NULL)
            {
                // Check if there's a breakpoint on its name:
                const gtString& kernelFuncName = pKernelDetails->kernelFunctionName();
                retVal = su_stat_theBreakpointsManager.shouldBreakAtKernelFunction(kernelFuncName);

                // If there is a breakpoint:
                if (retVal && (!kernelFuncName.isEmpty()))
                {
                    // We want to break on the kernel's first line:
                    reason = suIKernelDebuggingManager::KERNEL_FUNCTION_NAME_BREAKPOINT;
                }
            }
        }
    }

    // Check for breakpoints:
    if (!retVal)
    {
        // Get the containing program:
        oaCLProgramHandle containingProgramHandle = cs_stat_openCLMonitorInstance.programContainingKernel(kernel);

        if (containingProgramHandle != OA_CL_NULL_HANDLE)
        {
            // Get the breakpoints:
            gtVector<int> breakpointLineNumbers;
            su_stat_theBreakpointsManager.getBreakpointsInProgram(containingProgramHandle, breakpointLineNumbers);

            // If there are breakpoints on this program:
            if (breakpointLineNumbers.size() > 0)
            {
                retVal = true;
                reason = suIKernelDebuggingManager::KERNEL_SOURCE_CODE_BREAKPOINT;
            }
        }
    }

    // If we're blocking this debugging, return false (but perform the previous checks to clear any flags)
    if (!rcAllowMult)
    {
        retVal = false;
    }

    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg;

        if (retVal)
        {
            logMsg.appendFormattedString(L"csOpenCLMonitor::shouldDebugKernel determined kernel should be debugged. Reason = %#x", reason);
        }
        else
        {
            logMsg = L"csOpenCLMonitor::shouldDebugKernel determined kernel should not be debugged.";
        }

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::beforeKernelDebugging
// Description: Called before the debugging API is invoked for the kernel.
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::beforeKernelDebuggingEnqueued()
{
    OS_OUTPUT_DEBUG_LOG(L"csOpenCLMonitor::beforeKernelDebuggingEnqueued", OS_DEBUG_LOG_EXTENSIVE);

    // Notify the technology monitors manager that the kernel debugging session is about to start:
    suTechnologyMonitorsManager::instance().notifyMonitorsBeforeKernelDebugging();
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::afterKernelDebugging
// Description: Called after the debugging API is invoked for the kernel.
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::afterKernelDebuggingEnqueued()
{
    // We currently don't do anything after debug kernel enqueuement
    OS_OUTPUT_DEBUG_LOG(L"csOpenCLMonitor::afterKernelDebuggingEnqueued", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onKernelDebuggingFailure
// Description: Called when kernel debugging fails, to clear up the changes done in beforeKernelDebugging
// Author:      Uri Shomroni
// Date:        10/4/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onKernelDebuggingEnqueueFailure()
{
    // We currently don't do anything after debug kernel enqueuement fails
    OS_OUTPUT_DEBUG_LOG(L"csOpenCLMonitor::onKernelDebuggingEnqueueFailure", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::reportKernelDebuggingFailure
// Description: Reports kernel debugging failure to the client app.
// Author:      Uri Shomroni
// Date:        9/3/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::reportKernelDebuggingFailure(cl_int originalOpenCLError, cl_int openCLError, gtString& errorString)
{
    // Determine the failure reason:
    apKernelDebuggingFailedEvent::apKernelDebuggingFailureReason failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_UNKNOWN_FAILURE;

    if (openCLError != CL_SUCCESS)
    {
        // There was a cl error with enqueueing the kernel, send an event to the client:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_ENQUEUE_ERROR;
    }
    else if (originalOpenCLError == CL_SUCCESS)
    {
        // There was no OpenCL or debug API error, this is a debug failure or error after debugging started:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_DEBUG_FAILURE;
    }
    else if (originalOpenCLError == CL_DEBUG_ERROR_AMD)
    {
        // The problem was only with the debug API, the kernel is OK otherwise:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_NOT_SUPPORTED;
    }
    else if (originalOpenCLError == CL_UNSUPPORTED_PLATFORM_AMD)
    {
        // Attempting to debug a kernel on a non-AMD platform:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_UNSUPPORTED_PLATFORM;
    }
    else if (originalOpenCLError == CL_DEVICE_NOT_GPU_AMD)
    {
        // Attempting to debug a kernel on a non-GPU device:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_NON_GPU_DEVICE;
    }
    else if (originalOpenCLError == CL_KERNEL_NOT_DEBUGGABLE_AMD)
    {
        // Attempting to debug a non-debuggable kernel:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_NOT_DEBUGGABLE;
    }
    else if (originalOpenCLError == CL_COMMAND_QUEUE_NOT_INTERCEPTED_AMD)
    {
        // The command queue was not intercepted correctly:
        failureReason = apKernelDebuggingFailedEvent::AP_KERNEL_QUEUE_NOT_INTERCEPTED;
    }

    // Send the event:
    suIKernelDebuggingManager::reportKernelDebuggingFailure(openCLError, failureReason, errorString);
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::clContextMonitor
// Description: Returns a context monitor.
// Arguments: contextId - The queried context id.
// Return Val: The context monitor or NULL if a context of this id does not exist.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::clContextMonitor(int contextId) const
{
    const csContextMonitor* retVal = NULL;

    // Context id range:
    int contextsAmount = (int)_contextsMonitors.size();
    GT_IF_WITH_ASSERT((0 < contextId) && (contextId < contextsAmount))
    {
        retVal = (const csContextMonitor*)_contextsMonitors[contextId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::clContextMonitor
// Description: Returns a context monitor.
// Arguments: contextId - The queried context id.
// Return Val: The context monitor or NULL if a context of this id does not exist.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::clContextMonitor(int contextId)
{
    csContextMonitor* retVal = NULL;

    // Context id range:
    int contextsAmount = (int)_contextsMonitors.size();
    GT_IF_WITH_ASSERT((0 < contextId) && (contextId < contextsAmount))
    {
        retVal = (csContextMonitor*)_contextsMonitors[contextId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextMonitor
// Description:
//  Inputs a context handle and returns the context monitor that represents it
//  (or NULL if such context monitor does not exist).
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::clContextMonitor(oaCLContextHandle contextHandle) const
{
    const csContextMonitor* retVal = NULL;

    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                oaCLContextHandle hCurrentContext = pCurrContextMonitor->contextHandle();

                if (hCurrentContext == contextHandle)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextMonitor
// Description:
//  Inputs a context handle and returns the context monitor that represents it
//  (or NULL if such context monitor does not exist).
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::clContextMonitor(oaCLContextHandle contextHandle)
{
    csContextMonitor* retVal = NULL;

    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                oaCLContextHandle hCurrentContext = pCurrContextMonitor->contextHandle();

                if (hCurrentContext == contextHandle)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingMemObject
// Description:
//  Inputs a mem handle and returns the context in which it was created,
//  or NULL if such a buffer / texture was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::contextContainingMemObject(oaCLMemHandle memObjectHandle) const
{
    const csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                const csImagesAndBuffersMonitor& memObjectsMonitor = pCurrContextMonitor->imagesAndBuffersMonitor();
                const apCLMemObject* pMemObj = memObjectsMonitor.getMemObjectDetails(memObjectHandle);

                if (pMemObj != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living object.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csImagesAndBuffersMonitor::getMemObjectDetails prefers living objects to dead ones:
                        if (!pMemObj->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingMemObject
// Description:
//  Inputs a mem handle and returns the context in which it was created,
//  or NULL if such a buffer / texture was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::contextContainingMemObject(oaCLMemHandle memObjectHandle)
{
    csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                const csImagesAndBuffersMonitor& memObjectsMonitor = pCurrContextMonitor->imagesAndBuffersMonitor();
                const apCLMemObject* pMemObj = memObjectsMonitor.getMemObjectDetails(memObjectHandle);

                if (pMemObj != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living object.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csImagesAndBuffersMonitor::getMemObjectDetails prefers living objects to dead ones:
                        if (!pMemObj->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingMemObject
// Description:
//  Inputs a mem handle and returns the context in which it was created,
//  or NULL if such a buffer / texture was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
apCLMemObject* csOpenCLMonitor::getMemryObjectDetails(oaCLMemHandle memObjectHandle)
{
    apCLMemObject* pRetVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                csImagesAndBuffersMonitor& memObjectsMonitor = pCurrContextMonitor->imagesAndBuffersMonitor();
                apCLMemObject* pMemObj = memObjectsMonitor.getMemObjectDetails(memObjectHandle);

                if (pMemObj != NULL)
                {
                    pRetVal = pMemObj;

                    // Only stop if we found a living context with a living object.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csImagesAndBuffersMonitor::getMemObjectDetails prefers living objects to dead ones:
                        if (!pMemObj->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingMemObject
// Description:
//  Inputs a mem handle and returns the context in which it was created,
//  or NULL if such a buffer / texture was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
const apCLMemObject* csOpenCLMonitor::getMemryObjectDetails(oaCLMemHandle memObjectHandle) const
{
    const apCLMemObject* pRetVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                const csImagesAndBuffersMonitor& memObjectsMonitor = pCurrContextMonitor->imagesAndBuffersMonitor();
                const apCLMemObject* pMemObj = memObjectsMonitor.getMemObjectDetails(memObjectHandle);

                if (pMemObj != NULL)
                {
                    pRetVal = pMemObj;

                    // Only stop if we found a living context with a living object.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csImagesAndBuffersMonitor::getMemObjectDetails prefers living objects to dead ones:
                        if (!pMemObj->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingProgram
// Description:
//  Inputs a program handle and returns the context in which it was created,
//  or NULL if such program was not logged by our OpenCL Server.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::contextContainingProgram(oaCLProgramHandle programHandle) const
{
    const csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                const csProgramsAndKernelsMonitor& programsMonitor = pCurrContextMonitor->programsAndKernelsMonitor();
                const apCLProgram* pProgramMonitor = programsMonitor.programMonitor(programHandle);

                if (pProgramMonitor != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living program.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csProgramsAndKernelsMonitor::programMonitor prefers living programs to dead ones:
                        if (!pProgramMonitor->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingProgram
// Description:
//  Inputs a program handle and returns the context in which it was created,
//  or NULL if such program was not logged by our OpenCL Server.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::contextContainingProgram(oaCLProgramHandle programHandle)
{
    csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                csProgramsAndKernelsMonitor& programMonitor = pCurrContextMonitor->programsAndKernelsMonitor();
                apCLProgram* pProgramMonitor = programMonitor.programMonitor(programHandle);

                if (pProgramMonitor != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living program.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csProgramsAndKernelsMonitor::programMonitor prefers living programs to dead ones:
                        if (!pProgramMonitor->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingKernel
// Description:
//  Inputs a kernel handle and returns the context in which it was created,
//  or NULL if such kernel was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::contextContainingKernel(oaCLKernelHandle kernelHandle) const
{
    const csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                const csProgramsAndKernelsMonitor& programMonitor = pCurrContextMonitor->programsAndKernelsMonitor();
                const csCLKernel* pKernelMonitor = programMonitor.kernelMonitor(kernelHandle);

                if (pKernelMonitor != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context (kernels are removed from our vectors when marked for deletion).
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingKernel
// Description:
//  Inputs a kernel handle and returns the context in which it was created,
//  or NULL if such kernel was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::contextContainingKernel(oaCLKernelHandle kernelHandle)
{
    csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();
    GT_ASSERT(iter != endIter);

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;
        GT_ASSERT(pContextMonitor != NULL);

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                csProgramsAndKernelsMonitor& programMonitor = pCurrContextMonitor->programsAndKernelsMonitor();
                csCLKernel* pKernelMonitor = programMonitor.kernelMonitor(kernelHandle);

                if (pKernelMonitor != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context (kernels are removed from our vectors when marked for deletion).
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingSampler
// Description:
//  Inputs a sampler handle and returns the context in which it was created,
//  or NULL if this sampler was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::contextContainingSampler(oaCLSamplerHandle samplerHandle) const
{
    const csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the sampler was created in this context:
                const csSamplersMonitor& currentSamplersMonitor = pCurrContextMonitor->samplersMonitor();
                const apCLSampler* pSampler = currentSamplersMonitor.getSamplerDetails(samplerHandle);

                if (pSampler != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living sampler.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csSamplersMonitor::getSamplerDetails prefers living samplers to dead ones:
                        if (!pSampler->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingSampler
// Description:
//  Inputs a sampler handle and returns the context in which it was created,
//  or NULL if this sampler was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::contextContainingSampler(oaCLSamplerHandle samplerHandle)
{
    csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the sampler was created in this context:
                const csSamplersMonitor& currentSamplersMonitor = pCurrContextMonitor->samplersMonitor();
                const apCLSampler* pSampler = currentSamplersMonitor.getSamplerDetails(samplerHandle);

                if (pSampler != NULL)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living sampler.
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        // Note that csSamplersMonitor::getSamplerDetails prefers living samplers to dead ones:
                        if (!pSampler->wasMarkedForDeletion())
                        {
                            break;
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingQueue
// Description:
//  Inputs a queue handle and returns the context in which it was created,
//  or NULL if this queue was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::contextContainingQueue(oaCLCommandQueueHandle queueHandle) const
{
    const csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::const_iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::const_iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        const suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                const csContextMonitor* pCurrContextMonitor = (const csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the queue was created in this context:
                const csCommandQueuesMonitor& currentQueuesMonitor = pCurrContextMonitor->commandQueuesMonitor();
                int queueIndex = currentQueuesMonitor.commandQueueIndex(queueHandle);

                if (-1 != queueIndex)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if the context wasn't marked for deletion:
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingQueue
// Description:
//  Inputs a queue handle and returns the context in which it was created,
//  or NULL if this queue was not logged by our OpenCL Server.
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::contextContainingQueue(oaCLCommandQueueHandle queueHandle)
{
    csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the queue was created in this context:
                const csCommandQueuesMonitor& currentQueuesMonitor = pCurrContextMonitor->commandQueuesMonitor();
                int queueIndex = currentQueuesMonitor.commandQueueIndex(queueHandle);

                if (-1 != queueIndex)
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if the context was not marked for deletion:
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::commandQueueMonitor
// Description: Inputs a command queue's OpenCL handle and returns its monitor.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
const csCommandQueueMonitor* csOpenCLMonitor::commandQueueMonitor(oaCLCommandQueueHandle queueHandle) const
{
    const csCommandQueueMonitor* retVal = NULL;

    // Use my none-const version:
    csCommandQueueMonitor* pQueueMtr = ((csOpenCLMonitor*)this)->commandQueueMonitor(queueHandle);
    retVal = pQueueMtr;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::commandQueueMonitor
// Description: Inputs a command queue's OpenCL handle and returns its monitor.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
csCommandQueueMonitor* csOpenCLMonitor::commandQueueMonitor(oaCLCommandQueueHandle queueHandle)
{
    csCommandQueueMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // If the input program was created in this context:
                csCommandQueuesMonitor& commandQueuesMtr = pCurrContextMonitor->commandQueuesMonitor();
                int queueIndex = commandQueuesMtr.commandQueueIndex(queueHandle);

                if (0 <= queueIndex)
                {
                    retVal = commandQueuesMtr.commandQueueMonitor(queueIndex);

                    if (retVal != NULL)
                    {
                        // Only stop if we found a living context with a living queue.
                        // This has to be done to avoid problems from handle re-use.
                        if (isLivingContext)
                        {
                            // Note that csCommandQueuesMonitor::commandQueueMonitor prefers living queues to dead ones:
                            if (!retVal->commandQueueInfo().wasMarkedForDeletion())
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingEvent
// Description: Inputs an OpenCL handle and returns the context containing
//              this event.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
const csContextMonitor* csOpenCLMonitor::contextContainingEvent(oaCLEventHandle eventHandle) const
{
    const csContextMonitor* retVal = NULL;

    // Use my none-const version:
    csContextMonitor* pQueueMtr = ((csOpenCLMonitor*)this)->contextContainingEvent(eventHandle);
    retVal = pQueueMtr;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextContainingEvent
// Description: Inputs an OpenCL handle and returns the context containing
//              this event.
// Author:      Yaki Tebeka
// Date:        4/3/2010
// ---------------------------------------------------------------------------
csContextMonitor* csOpenCLMonitor::contextContainingEvent(oaCLEventHandle eventHandle)
{
    csContextMonitor* retVal = NULL;

    // Iterate the OpenCL contexts:
    gtPtrVector<suContextMonitor*>::iterator iter = _contextsMonitors.begin();
    gtPtrVector<suContextMonitor*>::iterator endIter = _contextsMonitors.end();

    while (iter != endIter)
    {
        // Get the current context monitor:
        suContextMonitor* pContextMonitor = *iter;

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->contextID().isOpenCLContext())
            {
                // Get the current context monitor:
                csContextMonitor* pCurrContextMonitor = (csContextMonitor*)pContextMonitor;

                // Is this context alive?
                bool isLivingContext = !pCurrContextMonitor->contextInformation().wasMarkedForDeletion();

                // Does it contain the requested event?
                if (NULL != pCurrContextMonitor->eventsMonitor().eventDetails(eventHandle))
                {
                    retVal = pCurrContextMonitor;

                    // Only stop if we found a living context with a living event (events are removed from our vectors when they are marked for deletion).
                    // This has to be done to avoid problems from handle re-use.
                    if (isLivingContext)
                    {
                        break;
                    }
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::programContainingKernel
// Description: Returns the handle to the program that contains kernelHandle,
//              or OA_CL_NULL_HANDLE if we cannot find it.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
oaCLProgramHandle csOpenCLMonitor::programContainingKernel(oaCLKernelHandle kernelHandle) const
{
    oaCLProgramHandle retVal = OA_CL_NULL_HANDLE;

    const csContextMonitor* pContext = contextContainingKernel(kernelHandle);

    if (pContext != NULL)
    {
        const csCLKernel* pKernelDetails = pContext->programsAndKernelsMonitor().kernelMonitor(kernelHandle);
        GT_IF_WITH_ASSERT(pKernelDetails != NULL)
        {
            retVal = pKernelDetails->programHandle();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::programHandleFromSourcePath
// Description: If sourceFilePath is the path to a program's source, returns that
//              program's handle. If the program file path is a temporary file created by CodeXL,
//              compare only the file name, and return the new path
//              Otherwise, returns OA_CL_NULL_HANDLE.
// Arguments:   const osFilePath& sourceFilePath - the source code file path
//              osFilePath& newTempSourceFilePath - the new temporary source code file path
//              that represent the same OpenCL program in the new application execution
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
oaCLProgramHandle csOpenCLMonitor::programHandleFromSourcePath(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath) const
{
    oaCLProgramHandle retVal = OA_CL_NULL_HANDLE;

    // Get the log files directory:
    const osFilePath logFilesPath = suCurrentSessionLogFilesDirectory();

    // Check if the source code file path is in CodeXL temporary files folder:
    bool isTempFile = false;
    osDirectory sourceCodeDirectory;
    bool rc = sourceFilePath.getFileDirectory(sourceCodeDirectory);
    GT_IF_WITH_ASSERT(rc)
    {
        // Get the directory from the log files path:
        osDirectory logFilesDirectory;
        logFilesPath.getFileDirectory(logFilesDirectory);
        isTempFile = (logFilesDirectory.getParentDirectory().directoryPath() == sourceCodeDirectory.getParentDirectory().directoryPath());
    }

    // Iterate all the contexts:
    int numberOfContexts = amountOfContexts();

    for (int i = 1; i <= numberOfContexts; i++)
    {
        // Get each context:
        const csContextMonitor* pCurrentContext = clContextMonitor(i);
        GT_IF_WITH_ASSERT(pCurrentContext != NULL)
        {
            // Iterate the programs:
            const csProgramsAndKernelsMonitor& currentProgramsMonitor = pCurrentContext->programsAndKernelsMonitor();
            int numberOfPrograms = currentProgramsMonitor.amountOfPrograms();

            for (int j = 0; j < numberOfPrograms; j++)
            {
                // Get each program:
                const apCLProgram* pCurrentProgram = currentProgramsMonitor.programMonitor(j);
                GT_IF_WITH_ASSERT(pCurrentProgram != NULL)
                {
                    // Sanity check:
                    oaCLProgramHandle currentProgramHandle = pCurrentProgram->programHandle();
                    GT_IF_WITH_ASSERT(currentProgramHandle != OA_CL_NULL_HANDLE)
                    {
                        // Compare the paths:
                        if (pCurrentProgram->sourceCodeFilePath() == sourceFilePath)
                        {
                            // We found the program, stop looking in this context:
                            retVal = currentProgramHandle;
                            break;
                        }
                        else
                        {
                            // If this is a temporary cl file, compare only the file name:
                            if (isTempFile)
                            {
                                // Get the file name from the current program:
                                gtString programFileName;
                                pCurrentProgram->sourceCodeFilePath().getFileNameAndExtension(programFileName);

                                // Get the file name from the requested file path:
                                gtString requestedProgramFileNameAndExt, requestedProgramFileName;
                                sourceFilePath.getFileNameAndExtension(requestedProgramFileNameAndExt);
                                sourceFilePath.getFileName(requestedProgramFileName);

                                if (requestedProgramFileNameAndExt == programFileName)
                                {
                                    retVal = currentProgramHandle;
                                    newTempSourceFilePath.setFileDirectory(logFilesPath.asString());
                                    newTempSourceFilePath.setFileName(requestedProgramFileName);
                                    newTempSourceFilePath.setFileExtension(CS_STR_kernelSourceFileExtension);
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // If we found the program, stop looking in this context:
            if (retVal != OA_CL_NULL_HANDLE)
            {
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::setKernelSourceFilePath
// Description: store the list of programs source path
// Author:      Gilad Yarnitzky
// Date:        21/4/2011
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::setKernelSourceFilePath(gtVector<osFilePath>& programsFilePath)
{
    bool retVal = true;

    // Clear old list of program paths:
    _programsFilePath.clear();

    // copy new list:
    _programsFilePath = programsFilePath;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfDeviceWasDeleted
// Description: When a device might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfDeviceWasDeleted(cl_device_id device)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)device, OS_TOBJ_ID_CL_DEVICE);

    if (isValid)
    {
        cl_uint externalCount = deviceExternalReferenceCount(device);

        if (0 == externalCount)
        {
            _devicesMonitor.onDeviceMarkedForDeletion((oaCLDeviceID)device);

            // Release the actual device:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseDevice);
            cs_stat_realFunctionPointers.clReleaseDevice(device);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseDevice);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::deviceExternalReferenceCount
// Description: Returns a device's external reference count (subtracting the
//              1 that the debugger adds for generated devices)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::deviceExternalReferenceCount(cl_device_id device)
{
    cl_uint retVal = 0;

    if (NULL != device)
    {
        retVal = 1;

        cl_device_id parentDevice = NULL;

        // We can ignore error values here, since any error would mean this is not a parented device:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);
        cs_stat_realFunctionPointers.clGetDeviceInfo(device, CL_DEVICE_PARENT_DEVICE, sizeof(cl_device_id), &parentDevice, NULL);

        // If this a parented device, we can get its reference count:
        if (NULL != parentDevice)
        {
            cl_uint refCount = 1;
            cl_int rcInfo = cs_stat_realFunctionPointers.clGetDeviceInfo(device, CL_DEVICE_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);

            if (CL_SUCCESS == rcInfo)
            {
                // There should be at least one reference, which the debugger is holding:
                GT_IF_WITH_ASSERT(refCount > 0)
                {
                    retVal = refCount - 1;
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfContextWasDeleted
// Description: When a context might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfContextWasDeleted(cl_context context)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)context, OS_TOBJ_ID_CL_CONTEXT);

    if (isValid)
    {
        cl_uint externalCount = contextExternalReferenceCount(context);

        // Get the context monitor:
        csContextMonitor* pContext = clContextMonitor((oaCLContextHandle)context);

        // If the context is alive, update it to see if any of its subordinates have been
        // release since we last checked:
        if (0 < externalCount)
        {
            GT_IF_WITH_ASSERT(NULL != pContext)
            {
                // Update it:
                pContext->updateContextDataSnapshot(false, false);

                // If the external count was greater than 0, it might have changed now - check again:
                externalCount = contextExternalReferenceCount(context);
            }
        }

        // If the context should be marked for deletion, but wasn't yet:
        if ((0 == externalCount) && (NULL != context) && (!pContext->contextInformation().wasMarkedForDeletion()))
        {
            // If we are not in profile mode:
            apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

            // Check for memory leaks:
            if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
            {
                // If we are running with a debugger:
                bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();

                if (!isRunningInStandaloneMode)
                {
                    // Get the containing context ID:
                    apContextID contextId = pContext->contextID();

                    // Handle memory leak check:
                    csMemoryMonitor::instance().beforeContextDeletion(contextId);
                }
            }

            // Release the actual context:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseContext);
            cs_stat_realFunctionPointers.clReleaseContext(context);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseContext);

            // Kill the context object:
            GT_IF_WITH_ASSERT(NULL != pContext)
            {
                pContext->onContextMarkedForDeletion();

                // Output debug message:
                gtString debugMsg;
                debugMsg.appendFormattedString(CS_STR_computeContextWasDeleted, pContext->spyId());
                OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);

                // Notify the debugger about the context deletion:
                osThreadId currentThreadId = osGetCurrentThreadId();
                apComputeContextDeletedEvent computeContextDeletedEvent(currentThreadId, pContext->spyId());
                bool rcEve = suForwardEventToClient(computeContextDeletedEvent);
                GT_ASSERT(rcEve);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::contextExternalReferenceCount
// Description: Returns a context's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::contextExternalReferenceCount(cl_context context)
{
    cl_uint retVal = 0;

    if (NULL != context)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount - 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfCommandQueueWasDeleted
// Description: When a queue might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfCommandQueueWasDeleted(cl_command_queue command_queue, bool checkParentObjects)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)command_queue, OS_TOBJ_ID_CL_COMMAND_QUEUE);

    if (isValid)
    {
        cl_uint externalCount = commandQueueExternalReferenceCount(command_queue);

        if (0 == externalCount)
        {
            // Get the queue's context:
            cl_context queueContext = NULL;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);
            cs_stat_realFunctionPointers.clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &queueContext, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);

            // Release the actual queue:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseCommandQueue);
            cs_stat_realFunctionPointers.clReleaseCommandQueue(command_queue);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseCommandQueue);

            // Notify its queue monitor:
            csCommandQueueMonitor* pCommandQueueMonitor = commandQueueMonitor((oaCLCommandQueueHandle)command_queue);
            GT_IF_WITH_ASSERT(pCommandQueueMonitor != NULL)
            {
                pCommandQueueMonitor->onCommandQueueMarkedForDeletion();
            }

            // Check if its container expired:
            if (checkParentObjects && (NULL != queueContext))
            {
                checkIfContextWasDeleted(queueContext);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::commandQueueExternalReferenceCount
// Description: Returns a queue's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::commandQueueExternalReferenceCount(cl_command_queue command_queue)
{
    cl_uint retVal = 0;

    if (NULL != command_queue)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetCommandQueueInfo(command_queue, CL_QUEUE_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetCommandQueueInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount - 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfMemObjectWasDeleted
// Description: When a mem object might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfMemObjectWasDeleted(cl_mem memobj, bool checkParentObjects)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)memobj, OS_TOBJ_ID_CL_BUFFER) ||
                   _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)memobj, OS_TOBJ_ID_CL_SUB_BUFFER) ||
                   _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)memobj, OS_TOBJ_ID_CL_IMAGE) ||
                   _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)memobj, OS_TOBJ_ID_CL_PIPE);

    if (isValid)
    {
        cl_uint externalCount = memObjectExternalReferenceCount(memobj);

        if (0 == externalCount)
        {
            // Get the mem object's context:
            cl_context memContext = NULL;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);
            cs_stat_realFunctionPointers.clGetMemObjectInfo(memobj, CL_MEM_CONTEXT, sizeof(cl_context), &memContext, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);

            // Release the actual mem object:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseMemObject);
            cs_stat_realFunctionPointers.clReleaseMemObject(memobj);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseMemObject);

            // Notify its context monitor:
            csContextMonitor* pContextMonitor = contextContainingMemObject((oaCLMemHandle)memobj);
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                pContextMonitor->onMemObjectMarkedForDeletion(memobj);
            }

            // Check if its container expired:
            if (checkParentObjects && (NULL != memContext))
            {
                checkIfContextWasDeleted(memContext);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::memObjectExternalReferenceCount
// Description: Returns a mem object's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::memObjectExternalReferenceCount(cl_mem memobj)
{
    cl_uint retVal = 0;

    if (NULL != memobj)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetMemObjectInfo(memobj, CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount - 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfSamplerWasDeleted
// Description: When a sampler might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfSamplerWasDeleted(cl_sampler sampler, bool checkParentObjects)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)sampler, OS_TOBJ_ID_CL_SAMPLER);

    if (isValid)
    {
        cl_uint externalCount = samplerExternalReferenceCount(sampler);

        if (0 == externalCount)
        {
            // Get the sampler's context:
            cl_context samplerContext = NULL;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetSamplerInfo);
            cs_stat_realFunctionPointers.clGetSamplerInfo(sampler, CL_SAMPLER_CONTEXT, sizeof(cl_context), &samplerContext, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetSamplerInfo);

            // Release the actual sampler:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseSampler);
            cs_stat_realFunctionPointers.clReleaseSampler(sampler);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseSampler);

            // Notify its context monitor:
            csContextMonitor* pContextMonitor = contextContainingSampler((oaCLSamplerHandle)sampler);
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                pContextMonitor->onSamplerMarkedForDeletion(sampler);
            }

            // Check if its container expired:
            if (checkParentObjects && (NULL != samplerContext))
            {
                checkIfContextWasDeleted(samplerContext);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::samplerExternalReferenceCount
// Description: Returns a sampler's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::samplerExternalReferenceCount(cl_sampler sampler)
{
    cl_uint retVal = 0;

    if (NULL != sampler)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetSamplerInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetSamplerInfo(sampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetSamplerInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount - 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfProgramWasDeleted
// Description: When a program might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfProgramWasDeleted(cl_program program, bool checkParentObjects)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)program, OS_TOBJ_ID_CL_PROGRAM);

    if (isValid)
    {
        cl_uint externalCount = programExternalReferenceCount(program);

        if (0 == externalCount)
        {
            // Get the program's context:
            cl_context programContext = NULL;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
            cs_stat_realFunctionPointers.clGetProgramInfo(program, CL_PROGRAM_CONTEXT, sizeof(cl_context), &programContext, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);

            // Get our containing context monitor:
            oaCLProgramHandle progHandle = (oaCLProgramHandle)program;
            csContextMonitor* pContext = contextContainingProgram(progHandle);

            // If we are not in profile mode:
            apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

            if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
            {
                // If we are running with a debugger:
                bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();

                if (!isRunningInStandaloneMode)
                {
                    // Get the program index:
                    int programIndex = pContext->programsAndKernelsMonitor().programIndexFromHandle(progHandle);
                    GT_IF_WITH_ASSERT(programIndex > -1)
                    {
                        // Get the containing context ID:
                        apContextID contextId = pContext->contextID();

                        // Handle memory leak check:
                        csMemoryMonitor::instance().beforeComputationProgramReleased(contextId, programIndex);
                    }
                }
            }

            // Release the actual program:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);
            cs_stat_realFunctionPointers.clReleaseProgram(program);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);

            // Destroy the program monitor:
            GT_IF_WITH_ASSERT(NULL != pContext)
            {
                pContext->onProgramMarkedForDeletion(program);
            }

            // Check if its container expired:
            if (checkParentObjects && (NULL != programContext))
            {
                checkIfContextWasDeleted(programContext);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::programExternalReferenceCount
// Description: Returns a program's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::programExternalReferenceCount(cl_program program)
{
    cl_uint retVal = 0;

    if (NULL != program)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetProgramInfo(program, CL_PROGRAM_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount;

                // Unless the program is currently being built, we should decrease the reference count by 1, for the retain
                // we cause at its creation:
                bool shouldDecrease = true;
                gtMap<cl_program, bool>::const_iterator findIter = m_isProgramRefCountDecreasedForBuild.find(program);
                gtMap<cl_program, bool>::const_iterator endIter = m_isProgramRefCountDecreasedForBuild.end();

                if (endIter != findIter)
                {
                    shouldDecrease = !findIter->second;
                }

                if (shouldDecrease)
                {
                    retVal--;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfKernelWasDeleted
// Description: When a kernel might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfKernelWasDeleted(cl_kernel kernel, bool checkParentObjects)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)kernel, OS_TOBJ_ID_CL_KERNEL);

    if (isValid)
    {
        cl_uint externalCount = kernelExternalReferenceCount(kernel);

        if (0 == externalCount)
        {
            // Get the kernel's program:
            cl_program kernelProgram = NULL;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);
            cs_stat_realFunctionPointers.clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(cl_program), &kernelProgram, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);

            // Release the actual kernel:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);
            cs_stat_realFunctionPointers.clReleaseKernel(kernel);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);

            // Get the context monitor:
            csContextMonitor* pContextMonitor = contextContainingKernel((oaCLKernelHandle)kernel);
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                // Mark the kernel for deletion in the programs and kernels monitor:
                pContextMonitor->onKernelMarkedForDeletion(kernel);
            }

            // Check if its container expired:
            if (checkParentObjects && (NULL != kernelProgram))
            {
                checkIfProgramWasDeleted(kernelProgram, checkParentObjects);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::kernelExternalReferenceCount
// Description: Returns a kernel's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::kernelExternalReferenceCount(cl_kernel kernel)
{
    cl_uint retVal = 0;

    if (NULL != kernel)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetKernelInfo(kernel, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount - 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkIfEventWasDeleted
// Description: When an Event might have been released, checks if it was supposed
//              to have been released (i.e. its reference count is the 1 we added)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkIfEventWasDeleted(cl_event event, bool checkParentObjects)
{
    osCriticalSectionDelayedLocker objectDeletionCSL;
    enterDeletionCheckerFunction(objectDeletionCSL);

    bool isValid = _openCLHandlesMonitor.validateLivingHandle((oaCLHandle)event, OS_TOBJ_ID_CL_EVENT);

    if (isValid)
    {
        cl_uint externalCount = eventExternalReferenceCount(event);

        if (0 == externalCount)
        {
            // Get the event's queue and context:
            cl_context eventContext = NULL;
            cl_command_queue eventQueue = NULL;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetEventInfo);
            cs_stat_realFunctionPointers.clGetEventInfo(event, CL_EVENT_CONTEXT, sizeof(cl_context), &eventContext, NULL);
            cs_stat_realFunctionPointers.clGetEventInfo(event, CL_EVENT_COMMAND_QUEUE, sizeof(cl_command_queue), &eventQueue, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetEventInfo);

            // Get the context monitor:
            oaCLEventHandle hEvent = (oaCLEventHandle)event;
            csContextMonitor* pContextMonitor = contextContainingEvent(hEvent);
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                // Mark the event for deletion in the events monitor:
                pContextMonitor->eventsMonitor().onEventMarkedForDeletion(hEvent);
            }

            // Release the actual event:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseEvent);
            cs_stat_realFunctionPointers.clReleaseEvent(event);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseEvent);

            // Check if its container expired:
            if (checkParentObjects && (NULL != eventQueue))
            {
                // The event object does not necessarily retain the queue object.
                // Thus, the queue might have already been released.
                bool checkQueue = true;
                const csCommandQueueMonitor* pQueueMonitor = commandQueueMonitor((oaCLCommandQueueHandle)eventQueue);

                if (NULL != pQueueMonitor)
                {
                    // If the queue was marked for deletion, its handle is volatile and might not be safe to call.
                    checkQueue = !pQueueMonitor->commandQueueInfo().wasMarkedForDeletion();
                }

                if (checkQueue)
                {
                    checkIfCommandQueueWasDeleted(eventQueue, checkParentObjects);
                }
            }
            else if (checkParentObjects && (NULL != eventContext))
            {
                // We only need to specifically check the context if we didn't check the queue
                checkIfContextWasDeleted(eventContext);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::eventExternalReferenceCount
// Description: Returns a event's external reference count (subtracting the
//              1 that the debugger adds)
// Author:      Uri Shomroni
// Date:        13/8/2013
// ---------------------------------------------------------------------------
cl_uint csOpenCLMonitor::eventExternalReferenceCount(cl_event event)
{
    cl_uint retVal = 0;

    if (NULL != event)
    {
        retVal = 1;

        cl_uint refCount = 1;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetEventInfo);
        cl_int rcInfo = cs_stat_realFunctionPointers.clGetEventInfo(event, CL_EVENT_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetEventInfo);

        if (CL_SUCCESS == rcInfo)
        {
            // There should be at least one reference, which the debugger is holding:
            GT_IF_WITH_ASSERT(refCount > 0)
            {
                retVal = refCount - 1;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::isObjectOnAMDPlatform
// Description: Returns true if the object is monitored and on an AMD platform.
//              if expectedObjectType is not OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES,
//              that value is also validated to be the object's type.
// Author:      Uri Shomroni
// Date:        29/7/2015
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::isObjectOnAMDPlatform(oaCLHandle objectHandle, osTransferableObjectType expectedObjectType) const
{
    bool retVal = false;

    apCLObjectID* pCLObjectId = _openCLHandlesMonitor.getCLHandleObjectDetails(objectHandle);

    if (nullptr != pCLObjectId)
    {
        // TO_DO handle devices?
        int contextId = pCLObjectId->_contextId;

        if (0 < contextId)
        {
            // Get the context:
            const csContextMonitor* pContext = clContextMonitor(contextId);
            GT_IF_WITH_ASSERT(nullptr != pContext)
            {
                // If the context is on an AMD platform:
                retVal = pContext->contextInformation().isAMDPlatform();

                if (retVal && (OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES != expectedObjectType))
                {
                    // Validate type:
                    retVal = (expectedObjectType == pCLObjectId->_objectType);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::isObjectOnAMDPlatform
// Description: Returns true if the program object is monitored and on an AMD platform.
// Author:      Uri Shomroni
// Date:        29/7/2015
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::isProgramOnAMDPlatform(oaCLProgramHandle programHandle) const
{
    // Option 1: Try the handles monitor:
    bool retVal = isObjectOnAMDPlatform((oaCLHandle)programHandle, OS_TOBJ_ID_CL_PROGRAM);

    if (!retVal && (OA_CL_NULL_HANDLE != programHandle))
    {
        // Option 2: Try the program and kernels monitor:
        const csContextMonitor* pContext = contextContainingProgram(programHandle);
        GT_IF_WITH_ASSERT(nullptr != pContext)
        {
            retVal = pContext->contextInformation().isAMDPlatform();
        }

        if (!retVal)
        {
            // Option 3: directly query the platform:
            cl_context hContext = nullptr;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
            cl_int rc = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)programHandle, CL_PROGRAM_CONTEXT, sizeof(cl_context), &hContext, nullptr);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
            GT_ASSERT((CL_SUCCESS == rc) && (nullptr != hContext));

            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);
            size_t devicesArraySize = 0;
            rc = cs_stat_realFunctionPointers.clGetContextInfo(hContext, CL_CONTEXT_DEVICES, 0, nullptr, &devicesArraySize);
            GT_IF_WITH_ASSERT((CL_SUCCESS == rc) && (0 == (devicesArraySize % sizeof(cl_device_id)) && (0 < devicesArraySize)))
            {
                cl_device_id* devicesArray = new cl_device_id[devicesArraySize / sizeof(cl_device_id)];
                rc = cs_stat_realFunctionPointers.clGetContextInfo(hContext, CL_CONTEXT_DEVICES, devicesArraySize, devicesArray, nullptr);
                GT_IF_WITH_ASSERT(CL_SUCCESS == rc)
                {
                    retVal = _devicesMonitor.isAMDDevice((oaCLDeviceID)devicesArray[0]);
                }

                delete[] devicesArray;
            }
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::reportDetectedError
// Description: Reports a detected error to the breakpoints manager.
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::reportDetectedError(int contextIndex, apErrorCode errorCode, const gtString& errorDescription, apMonitoredFunctionId associatedFuncId)
{
    apContextID contextId(AP_OPENCL_CONTEXT, contextIndex);

    su_stat_theBreakpointsManager.reportDetectedError(contextId, errorCode, errorDescription, associatedFuncId);
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::internalProgramHandleFromExternalHandle
// Description: Returns the internal handle appropriate for this program
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
cl_program csOpenCLMonitor::internalProgramHandleFromExternalHandle(cl_program externalHandle) const
{
    cl_program retVal = externalHandle;

    oaCLProgramHandle progExternalHandle = (oaCLProgramHandle)externalHandle;
    const csContextMonitor* pContextMonitor = contextContainingProgram(progExternalHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        oaCLProgramHandle progInternalHandle = pContextMonitor->programsAndKernelsMonitor().getInternalProgramHandle(progExternalHandle);
        GT_IF_WITH_ASSERT(progInternalHandle != OA_CL_NULL_HANDLE)
        {
            retVal = (cl_program)progInternalHandle;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::internalKernelHandleFromExternalHandle
// Description: Returns the internal handle appropriate for this kernel
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
cl_kernel csOpenCLMonitor::internalKernelHandleFromExternalHandle(cl_kernel externalHandle) const
{
    cl_kernel retVal = externalHandle;

    oaCLKernelHandle kernExternalHandle = (oaCLKernelHandle)externalHandle;
    const csContextMonitor* pContextMonitor = contextContainingKernel(kernExternalHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        oaCLKernelHandle kernInternalHandle = pContextMonitor->programsAndKernelsMonitor().getInternalKernelHandle(kernExternalHandle);
        GT_IF_WITH_ASSERT(kernInternalHandle != OA_CL_NULL_HANDLE)
        {
            retVal = (cl_kernel)kernInternalHandle;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::isCommandQueueProfileModeForcedForQueue
// Description: Returns true iff command queues profile mode is on and queueHandle
//              supports profile mode.
// Author:      Uri Shomroni
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::isCommandQueueProfileModeForcedForQueue(oaCLCommandQueueHandle queueHandle) const
{
    bool retVal = _isCommandQueuesProfileModeForced;

    // If the mode isn't off, check if the queue supports this:
    if (retVal)
    {
        // Get the command queue monitor:
        const csCommandQueueMonitor* pCommandQueueMtr = commandQueueMonitor(queueHandle);
        GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
        {
            // Get the queue's associated device:
            int queueDeviceIndex = pCommandQueueMtr->commandQueueInfo().deviceIndex();

            // See if our device supports this:
            retVal = isCommandQueueProfileModeForcedForDevice(queueDeviceIndex);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::isCommandQueueProfileModeForcedForDevice
// Description: Returns true iff command queues profile mode is on and deviceId
//              supports profile mode.
// Author:      Uri Shomroni
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::isCommandQueueProfileModeForcedForDevice(int deviceIndex) const
{
    bool retVal = _isCommandQueuesProfileModeForced;

    // If the mode isn't off, check if the device supports this:
    if (retVal)
    {
        const apCLDevice* pDevice = _devicesMonitor.getDeviceObjectDetailsByIndex(deviceIndex);
        GT_IF_WITH_ASSERT(pDevice != NULL)
        {
            retVal = ((pDevice->deviceQueueProperties() & CL_QUEUE_PROFILING_ENABLE) != 0);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onCommandQueueCreation
// Description: Handles command queue creation
// Arguments: cl_context context
//            cl_device_id device
//            cl_command_queue_properties properties
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onCommandQueueCreation(cl_command_queue commandQueueHandle, cl_context context, cl_device_id device, cl_command_queue_properties properties)
{
    csContextMonitor* pContextMonitor = clContextMonitor((oaCLContextHandle)context);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the command queue monitor:
        csCommandQueuesMonitor& commandQueuesMonitor = pContextMonitor->commandQueuesMonitor();
        commandQueuesMonitor.onCommandQueueCreation(commandQueueHandle, context, device, properties);

        // Notify the debugger about the queue creation:
        // Get the current thread id:
        osThreadId currentThreadId = osGetCurrentThreadId();

        // Get the queue index:
        int queueIndex = commandQueuesMonitor.commandQueueIndex((oaCLCommandQueueHandle)commandQueueHandle);

        apOpenCLQueueCreatedEvent queueCreatedEvent(currentThreadId, pContextMonitor->spyId(), queueIndex);
        bool rcEve = suForwardEventToClient(queueCreatedEvent);
        GT_ASSERT(rcEve);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onCommandQueueCreationWithProperties
// Description: Handles command queue creation
// Author:      Uri Shomroni
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onCommandQueueCreationWithProperties(cl_command_queue commandQueueHandle, cl_context context, cl_device_id device, const cl_queue_properties* properties)
{
    csContextMonitor* pContextMonitor = clContextMonitor((oaCLContextHandle)context);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the command queue monitor:
        csCommandQueuesMonitor& commandQueuesMonitor = pContextMonitor->commandQueuesMonitor();
        commandQueuesMonitor.onCommandQueueCreationWithProperties(commandQueueHandle, context, device, properties);

        // Notify the debugger about the queue creation:
        // Get the current thread id:
        osThreadId currentThreadId = osGetCurrentThreadId();

        // Get the queue index:
        int queueIndex = commandQueuesMonitor.commandQueueIndex((oaCLCommandQueueHandle)commandQueueHandle);

        apOpenCLQueueCreatedEvent queueCreatedEvent(currentThreadId, pContextMonitor->spyId(), queueIndex);
        bool rcEve = suForwardEventToClient(queueCreatedEvent);
        GT_ASSERT(rcEve);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onCommandQueuePropertiesSet
// Description: Called when clSetCommandQueueProperty is used to set properties
//              on or off on a queue.
// Author:      Uri Shomroni
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onCommandQueuePropertiesSet(cl_command_queue commandQueueHandle, cl_command_queue_properties properties, cl_bool enable)
{
    // Get the command queue monitor:
    csCommandQueueMonitor* pCommandQueueMtr = commandQueueMonitor((oaCLCommandQueueHandle)commandQueueHandle);
    GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
    {
        pCommandQueueMtr->onCommandQueuePropertiesSet(properties, enable);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onOpenCLError
// Description: Handle OpenCL errors
// Arguments: apMonitoredFunctionId openCLFunctionID - the OpenCL function ID
//            int openCLErrorCode - the OpenCL error code
// Return Val: void
// Author:      Sigal Algranaty
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onOpenCLError(int openCLErrorCode, oaCLContextHandle hContext)
{
    (void)(hContext); // unused
    // Notify the debugged about the OpenCL error:
    GT_IF_WITH_ASSERT((_lastFunctionContextId >= 0) && (_lastFunctionContextId < (int)_contextsMonitors.size()))
    {
        apMonitoredFunctionId errorFuncId = apMonitoredFunctionsAmount;
        suContextMonitor* pContextMonitor = _contextsMonitors[_lastFunctionContextId];
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // Get the calls logger for this context:
            suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
            GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
            {
                // Get the last function call pointer:
                int amountOfFuncCalls = pCallsHistoryLogger->amountOfFunctionCalls();

                // Get the last function call:
                apOpenCLErrorParameters openCLErrorParams;
                bool rc = pCallsHistoryLogger->getFunctionCall(amountOfFuncCalls - 1, openCLErrorParams._aptrBreakedOnFunctionCall);
                GT_ASSERT(rc);

                // Get the function Id:
                if (openCLErrorParams._aptrBreakedOnFunctionCall.pointedObject() != NULL)
                {
                    errorFuncId = openCLErrorParams._aptrBreakedOnFunctionCall->functionId();
                }

                // Create an OpenCL error event:
                osThreadId currentThreadId = osGetCurrentThreadId();
                openCLErrorParams._openCLErrorCode = openCLErrorCode;
                apOpenCLErrorEvent openCLErrorEvent(currentThreadId, openCLErrorParams, _breakOnOpenCLErrors);
                bool rcEve = suForwardEventToClient(openCLErrorEvent);
                GT_ASSERT(rcEve);
            }
        }

        // If the user asked to break on OpenCL errors:
        if (_breakOnOpenCLErrors)
        {
            // Set the breakpoint reason:
            su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
            su_stat_theBreakpointsManager.setBreakReason(AP_OPENCL_ERROR_BREAKPOINT_HIT);

            // Trigger an "OpenCL error" breakpoint exception:
            apContextID contextId(AP_OPENCL_CONTEXT, _lastFunctionContextId);
            su_stat_theBreakpointsManager.triggerBreakpointException(contextId, errorFuncId);
            su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::setBreakOnOpenCLErrorsMode
// Description: Sets the "Break on OpenCL errors" flag
// Arguments: bool breakOnCLErrors
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/2/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::setBreakOnOpenCLErrorsMode(bool breakOnCLErrors)
{
    _breakOnOpenCLErrors = breakOnCLErrors;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::stubKernelFromRealKernel
// Description: Gets the stub kernel which can be used to replace realKernel
//              in execution commands (i.e. belongs to the right context, etc.)
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
oaCLKernelHandle csOpenCLMonitor::stubKernelFromRealKernel(oaCLKernelHandle realKernel)
{
    oaCLKernelHandle retVal = OA_CL_NULL_HANDLE;

    // Get the context containing this kernel:
    csContextMonitor* pContextMonitor = contextContainingKernel(realKernel);

    if (pContextMonitor != NULL)
    {
        // Get the context's stub kernel:
        retVal = pContextMonitor->stubKernelHandle();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::stubImageFromRealImage
// Description: Gets the stub image which can be used to replace realImage
//              in write / copy commands (i.e. belongs to the right context, has
//              same format etc.)
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
oaCLMemHandle csOpenCLMonitor::stubImageFromRealImage(oaCLMemHandle realImage)
{
    oaCLMemHandle retVal = OA_CL_NULL_HANDLE;

    // Get the context containing this image:
    csContextMonitor* pContextMonitor = contextContainingMemObject(realImage);

    if (pContextMonitor != NULL)
    {
        // Get the image's format:
        cl_image_format realImageFormat;
        memset((void*)&realImageFormat, 0, sizeof(cl_image_format));

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);
        cl_int rcFor = cs_stat_realFunctionPointers.clGetImageInfo((cl_mem)realImage, CL_IMAGE_FORMAT, sizeof(cl_image_format), &realImageFormat, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);

        if (rcFor)
        {
            // Get the context's stub image for this format:
            retVal = pContextMonitor->stubImageHandle(realImageFormat);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::stubBufferFromRealBuffer
// Description: Gets the stub buffer which can be used to replace realBuffer
//              in write / copy commands (i.e. belongs to the right context etc.)
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
oaCLMemHandle csOpenCLMonitor::stubBufferFromRealBuffer(oaCLMemHandle realBuffer)
{
    oaCLMemHandle retVal = OA_CL_NULL_HANDLE;

    // Get the context containing this buffer:
    csContextMonitor* pContextMonitor = contextContainingMemObject(realBuffer);

    if (pContextMonitor != NULL)
    {
        // Get the context's stub buffer:
        retVal = pContextMonitor->stubBufferHandle();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::stubNativeKernel
// Description: A function that is used as the stub native kernel.
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
void CL_CALLBACK csOpenCLMonitor::stubNativeKernel(void* ignored)
{
    (void)(ignored); // unused
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onEnqueueNDRangeKernel
// Description: Handles enqueue range kernel command
// Arguments: oaCLCommandQueueHandle commandQueueHandle
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onEnqueueNDRangeKernel(oaCLCommandQueueHandle commandQueueHandle)
{
    // Get the command queue monitor:
    csCommandQueueMonitor* pCommandQueueMtr = commandQueueMonitor((oaCLCommandQueueHandle)commandQueueHandle);
    GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
    {
        pCommandQueueMtr->onEnqueueNDRangeKernel();
    }
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onMemoryObjectWriteCommand
// Description: Handles memory object write command
// Arguments:   oaCLCommandQueueHandle memObjectHandle
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        12/5/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onMemoryObjectWriteCommand(oaCLMemHandle memObjectHandle)
{
    // Get the memory object details:
    apCLMemObject* pMemoryObject = getMemryObjectDetails(memObjectHandle);
    GT_IF_WITH_ASSERT(pMemoryObject != NULL)
    {
        pMemoryObject->markWriteOperationPerform();
    }
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::isOpenCLWriteOperationExecutionOn
// Description: Check if a write operation can currently be executed for the
//              requested buffer object
//              The function first check if write operations are enabled, and if
//              they are not, we check if this buffer was already written.
//              This function is used to make sure that the buffers first write
//              operation is not disabled, since the graphic card uses this first
//              call to allocate the buffer.
// Arguments:   cl_mem memobj
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/5/2010
// ---------------------------------------------------------------------------
bool csOpenCLMonitor::isOpenCLWriteOperationExecutionOn(cl_mem memobj) const
{
    // Check if write operation is enabled:
    bool retVal = _forceModesManager.isOperationExecutionOn(AP_OPENCL_WRITE_OPERATION_EXECUTION);

    if (!retVal)
    {
        // In case of error, perform the write operation:
        retVal = true;
        // Get the memory object details:
        const apCLMemObject* pMemoryObjectDetails = getMemryObjectDetails((oaCLMemHandle)memobj);
        GT_IF_WITH_ASSERT(pMemoryObjectDetails != NULL)
        {
            // Perform the operation if no write operation performed yet:
            retVal = !pMemoryObjectDetails->wasFirstWriteOperationPerformed();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::checkForProcessMemoryLeaks
// Description: Check memory leaks for the process before termination
// Author:      Sigal Algranaty
// Date:        18/7/2010
// ---------------------------------------------------------------------------
void csOpenCLMonitor::checkForProcessMemoryLeaks()
{
    //Check memory leaks when OpenGL is unloaded:
    csMemoryMonitor::instance().beforeSpyTermination();
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onBeforeKernelDebugging
// Description: Do nothing
// Author:      Sigal Algranaty
// Date:        5/4/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onBeforeKernelDebugging()
{

}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onBuildProgram
// Description: Send event on program build with log info
// Arguments:   cl_program program
//              cl_uint num_devices
//              const cl_device_id * device_list
//              const char* pCompileOptions
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* pCompileOptions, bool programDebuggable, const gtString& programNotDebuggableReason, bool canUpdateProgramKernels)
{
    // Look for the context in which the program was created:
    csContextMonitor* pContextMonitor = contextContainingProgram((oaCLProgramHandle)program);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the program details:
        csProgramsAndKernelsMonitor& contextProgramsAndKernelsMonitor = pContextMonitor->programsAndKernelsMonitor();
        contextProgramsAndKernelsMonitor.onProgramBuilt(program, num_devices, device_list, pCompileOptions, programDebuggable, programNotDebuggableReason, canUpdateProgramKernels);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onBeforeBuildProgram
// Description: Send event that a program build is about to start
// Arguments:   cl_program program - the program handle
//              cl_uint num_devices - the amount of devices
//              const cl_device_id * device_list - the list of devices to build the program on
//              const char* pCompileOptions - compile options
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onBeforeBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* pCompileOptions)
{
    // Look for the context in which the program was created:
    csContextMonitor* pContextMonitor = contextContainingProgram((oaCLProgramHandle)program);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the program details:
        csProgramsAndKernelsMonitor& contextProgramsAndKernelsMonitor = pContextMonitor->programsAndKernelsMonitor();
        contextProgramsAndKernelsMonitor.onBeforeProgramBuild(program, num_devices, device_list, pCompileOptions);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onProgramsLinked
// Description: Called when a program is linked with other programs by:
//              clLinkProgram - input programs
//              clCompileProgram - include headers
//              Marks the programs as related to the "main" program.
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onProgramsLinked(cl_program program, cl_uint num_related_programs, const cl_program* related_programs)
{
    csContextMonitor* pContextMonitor = contextContainingProgram((oaCLProgramHandle)program);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Update the program details:
        csProgramsAndKernelsMonitor& contextProgramsAndKernelsMonitor = pContextMonitor->programsAndKernelsMonitor();
        contextProgramsAndKernelsMonitor.onProgramsLinked(program, num_related_programs, related_programs);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onBuildProgramFailedWithDebugFlags
// Description: Called when a program build fails due to the debug flags added
//              (i.e. succeeded without them)
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onBuildProgramFailedWithDebugFlags(cl_program program, cl_int buildErrorCode)
{
    // Send an event to the client:
    if (!suIsRunningInStandaloneMode())
    {
        // Notify the debugger about the program build failure:
        // Get the program Id:
        int contextIndex = -1;
        int programIndex = -1;
        oaCLProgramHandle progHandle = (oaCLProgramHandle)program;
        const csContextMonitor* pContext = contextContainingProgram(progHandle);
        GT_IF_WITH_ASSERT(pContext != NULL)
        {
            contextIndex = pContext->contextID()._contextId;
            programIndex = pContext->programsAndKernelsMonitor().programIndexFromHandle(progHandle);
        }

        // Create the event:
        apOpenCLProgramBuildFailedWithDebugFlagsEvent buildFailedEvent(osGetCurrentThreadId(), contextIndex, programIndex, buildErrorCode);

        // Send the event:
        bool rcEve = suForwardEventToClient(buildFailedEvent);
        GT_ASSERT(rcEve);
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::releaseProgramForBuild
// Description: Called before a program is built, reducing its reference count.
//
//              Until Catalyst 13.8 (and possibly later), the AMD OpenCL runtime
//              uses reference counting to determine if an OpenCL program has
//              subordinate objects. This causes a sequence of:
//              clCreateProgram*-clRetainProgram-clBuildProgram/clCompileProgram
//              to fail with CL_INVALID_OPERATION.
//              Thus, before building, we temporarily disable the reference count
//              spoofing for the program (and restore it immediately after).
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::releaseProgramForBuild(cl_program program)
{
    cs_stat_realFunctionPointers.clReleaseProgram(program);

    m_isProgramRefCountDecreasedForBuild[program] = true;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::restoreProgramFromBuild
// Description: Called after a program is built, restoring its increased reference count.
//
//              Until Catalyst 13.8 (and possibly later), the AMD OpenCL runtime
//              uses reference counting to determine if an OpenCL program has
//              subordinate objects. This causes a sequence of:
//              clCreateProgram*-clRetainProgram-clBuildProgram/clCompileProgram
//              to fail with CL_INVALID_OPERATION.
//              Thus, before building, we temporarily disable the reference count
//              spoofing for the program (and restore it immediately after).
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csOpenCLMonitor::restoreProgramFromBuild(cl_program program)
{
    cs_stat_realFunctionPointers.clRetainProgram(program);

    m_isProgramRefCountDecreasedForBuild[program] = false;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::onGenericBreakpointSet
// Description: Set / Unset a generic breakpoint
// Arguments:   apGenericBreakpointType breakpointType
//              bool isOn
// Author:      Sigal Algranaty
// Date:        11/10/2011
// ---------------------------------------------------------------------------
void csOpenCLMonitor::onGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn)
{
    if (breakpointType == AP_BREAK_ON_CL_ERROR)
    {
        setBreakOnOpenCLErrorsMode(isOn);
    }
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::functionDeprecationStatus
// Description: Return the function deprecation status
// Arguments:   apMonitoredFunctionId calledFunctionId
// Return Val:  apFunctionDeprecationStatus
// Author:      Sigal Algranaty
// Date:        16/1/2012
// ---------------------------------------------------------------------------
apFunctionDeprecationStatus csOpenCLMonitor::functionDeprecationStatus(apMonitoredFunctionId calledFunctionId)
{
    apFunctionDeprecationStatus retVal = AP_DEPRECATION_NONE;

    if ((calledFunctionId == ap_clEnqueueMarker) || (calledFunctionId == ap_clEnqueueBarrier) ||
        (calledFunctionId == ap_clEnqueueWaitForEvents) || (calledFunctionId == ap_clCreateImage2D) ||
        (calledFunctionId == ap_clCreateImage3D) || (calledFunctionId == ap_clUnloadCompiler) || (calledFunctionId == ap_clGetExtensionFunctionAddress) ||
        (calledFunctionId == ap_clCreateFromGLTexture2D) || (calledFunctionId == ap_clCreateFromGLTexture3D) || (calledFunctionId == ap_clSetCommandQueueProperty))
    {
        retVal = AP_DEPRECATION_FULL;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLMonitor::enterDeletionCheckerFunction
// Description: Locks the supplied locker to the object deletion CS, unless this
//              function is called in the spy API thread.
// Author:      Uri Shomroni
// Date:        2/11/2015
// ---------------------------------------------------------------------------
void csOpenCLMonitor::enterDeletionCheckerFunction(osCriticalSectionDelayedLocker& objectDeletionCSL)
{
    // If the current thread is the API thread, don't enter:
    if (!suIsSpiesAPIThreadId(osGetCurrentThreadId()) || suIsSpiesAPIThreadId(OS_NO_THREAD_ID))
    {
        objectDeletionCSL.attachToCriticalSection(m_objectDeletionCS);
    }
}

