//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLMonitor.cpp
///
//==================================================================================

//------------------------------ gsOpenGLMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/Events/apContextDataSnapshotWasUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suSlowMotionDelay.h>

// Local:
#include <src/gsGLDebugOutputManager.h>
#include <src/gsExtensionsManager.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsOpenGLSpyInitFuncs.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#include <src/gsMemoryMonitor.h>

// For debugging purposes:
#include <src/gsAPIFunctionsImplementations.h>

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // Linux generic only stuff:
    #include <X11/Xlib.h>
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // According to Kent Miller from apple, we should use these instead of the regular ones
    // since they ignore the dispatch and run faster. The equivalents are 310 and 311
    // respectively (same names, without the "NoDispatch").
    #define kCGLCPGPUVertexProcessingNoDispatch     ((CGLContextParameter)1310)
    #define kCGLCPGPUFragmentProcessingNoDispatch   ((CGLContextParameter)1311)

    #ifdef _GR_IPHONE_BUILD
        #include <src/gsEAGLWrappers.h>
        #ifdef _GR_IPHONE_DEVICE_BUILD
            #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
        #endif
    #endif
#elif (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        #include <intrin.h>
    #endif
#endif

// Static members initializations:
gsOpenGLMonitor* gsOpenGLMonitor::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::instance
// Description: Returns the single instance of the gsOpenGLMonitor class
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
gsOpenGLMonitor& gsOpenGLMonitor::instance()
{
    if (_pMySingleInstance == NULL)
    {
        // Make sure we're not creating ourselves twice:
        static bool onlyOnce = true;
        GT_ASSERT(onlyOnce);
        onlyOnce = false;

        _pMySingleInstance = new gsOpenGLMonitor;
        GT_ASSERT(_pMySingleInstance);
        _pMySingleInstance->initialize();
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::gsOpenGLMonitor
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        11/5/2004
// ---------------------------------------------------------------------------
gsOpenGLMonitor::gsOpenGLMonitor()
    : suITechnologyMonitor(),
      _wasOpenGLServerInitialized(false),
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
      _ATIPerformanceCountersMgr(_shouldInitializePerformanceCounters),
#endif
#endif
      _openGLError(GL_NO_ERROR),
      _isOpenGLFlushForced(false),
      _isInteractiveBreakOn(true),
      _isDuringDebuggedProcessTermination(false)
{
}
// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::initialize
// Description: Initialize function - this is called by the instance() function after
//              creating the single instance. Any initializations that requires
//              gsOpenGLMonitor::instance() should be done here (or the assert inside
//              instance() will trigger).
// Author:      Uri Shomroni
// Date:        4/9/2012
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::initialize()
{
    // Initialize generic breakpoints - generic breakpoints are initialized before the monitors
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
// Name:        gsOpenGLMonitor::~gsOpenGLMonitor
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        11/5/2004
// ---------------------------------------------------------------------------
gsOpenGLMonitor::~gsOpenGLMonitor()
{
    if (_shouldInitializePerformanceCounters)
    {
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            bool rc = _ATIPerformanceCountersMgr.terminate();
            GT_ASSERT(rc);
        }
#endif
#endif
    }

    // The OpenGL monitor is only destroyed during the OpenGL spy termination:
    bool rcTer = gsPerformOpenGLMonitorTerminationActions();
    GT_ASSERT(rcTer);
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onDebuggedProcessTerminationAlert
// Description: Is called before the debugged process or this DLL is terminated.
// Author:      Yaki Tebeka
// Date:        25/1/2005
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onDebuggedProcessTerminationAlert()
{
    // Only perform these operations once:
    if (!_isDuringDebuggedProcessTermination)
    {
        // Mark the debugged process is being terminated:
        _isDuringDebuggedProcessTermination = true;

        // Notify the render context monitors:
        int noOfRenderContexts = (int)_contextsMonitors.size();

        for (int i = 0; i < noOfRenderContexts; i++)
        {
            _contextsMonitors[i]->onDebuggedProcessTerminationAlert();
        }

        // Notify the OpenGL context monitors:
        for (int i = 1; i < noOfRenderContexts; i++)
        {
            // Mark the render context texture as not updated:
            gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);

            if (pContextMonitor != NULL)
            {
                pContextMonitor->markTextureParametersAsNonUpdated();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::beforeDebuggedProcessSuspended
// Description: Called before the debugged process is suspended
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::beforeDebuggedProcessSuspended()
{

}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::afterDebuggedProcessResumed
// Description: Called once the debugged process is resumed from suspension.
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::afterDebuggedProcessResumed()
{

}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::beforeBreakpointException
// Description: Called by a thread before a breakpoint exception is thrown by it
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::beforeBreakpointException(bool isInOpenGLBeginEndBlock)
{
    // If "Interactive break" mode is on (and we are not in a glBegin-glEnd block or the process termination):
    if (_isInteractiveBreakOn && !isInOpenGLBeginEndBlock && !_isDuringDebuggedProcessTermination)
    {
        // Display the current render context content to the user:
        displayCurrentRenderContextContent();
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::afterBreakpointException
// Description: Called by a thread after a breakpoint exception it threw is passed
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::afterBreakpointException(bool isInOpenGLBeginEndBlock)
{
    // If "Interactive break" mode is on (and we are not in a glBegin-glEnd block or the process termination):
    if (_isInteractiveBreakOn && !isInOpenGLBeginEndBlock && !_isDuringDebuggedProcessTermination)
    {
        // After getting back from the breakpoint, we need to call displayCurrentRenderContextContent() again,
        // since when running on graphic boards that implement "real SwapBuffers" (not implementing SwapBuffers
        // by copy), we need to return the buffers to the state they were before the breakpoint exception:
        displayCurrentRenderContextContent();
    }

    // Clear OpenGL error:
    // (If we are here, it means that the debugger resumed the debugged process run).
    _openGLError = GL_NO_ERROR;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onDebuggedProcessExecutionModeChanged
// Description: Called when the debugged process execution mode is changed to newExecutionMode
// Author:      Uri Shomroni
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode)
{
    if (newExecutionMode == AP_PROFILING_MODE)
    {
        // We just changed to Profile Mode, clear the calls statistics:
        int amountOfContexts = (int)_contextsMonitors.size();

        for (int i = 0; i < amountOfContexts; i++)
        {
            gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);

            if (pContextMonitor != NULL)
            {
                pContextMonitor->callsStatisticsLogger().clearFunctionCallsStatistics();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onBeforeKernelDebugging
// Description: When we're within a kernel debugging session, we cannot update
//              context data snapshot. Therefore, we go through all the existing
//              contexts before the kernel debugging, and update them.
// Author:      Sigal Algranaty
// Date:        5/4/2011
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onBeforeKernelDebugging()
{
    // Go through each of the context monitors:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // If the context was not deleted, update its data snapshot:
            if (!pContextMonitor->wasDeleted())
            {
                // Make the input render context the API thread's current render context:
                (void) gs_stat_openGLMonitorInstance.currentThreadRenderContextSpyId();
                bool rcMakeCurrent = gsMakeRenderContextCurrent(i);
                GT_IF_WITH_ASSERT(rcMakeCurrent)
                {
                    // Update the context's data snapshot:
                    bool rcUpdateSnapshot = pContextMonitor->updateContextDataSnapshot();
                    GT_ASSERT(rcUpdateSnapshot);

                    // Get its static buffers monitor, and update the static buffer:
                    gsStaticBuffersMonitor& buffersMtr = pContextMonitor->buffersMonitor();
                    bool rcUpdateDims = buffersMtr.updateStaticBuffersDimensions();
                    GT_ASSERT(rcUpdateDims);

                    // Report the context snapshot:
                    apContextDataSnapshotWasUpdatedEvent contextUpdatedEvent(apContextID(AP_OPENGL_CONTEXT, i), false);
                    bool rcEve = suForwardEventToClient(contextUpdatedEvent);
                    GT_ASSERT(rcEve);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::threadCurrentRenderContext
// Description: Inputs a thread id and returns its current render context id.
// Arguments - threadId - The input thread id.
// Return Val:  int - The output render context id, or 0 if the input thread
//                    does not have a current render context.
// Author:      Yaki Tebeka
// Date:        5/5/2005
// ---------------------------------------------------------------------------
int gsOpenGLMonitor::threadCurrentRenderContext(const osThreadId& threadId) const
{
    int retVal = _threadsMonitor.threadCurrentRenderContext(threadId);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::renderContextCurrentThread
// Description: Inputs a render context and outputs the thread that uses this
//              render context as "current render context".
// Arguments: renderContextId - The input render context spy id.
// Return Val: osThreadId - The output thread id, or OS_NO_THREAD_ID if no thread
//                          uses the input render context as "current render context".
// Author:      Yaki Tebeka
// Date:        31/1/2008
// ---------------------------------------------------------------------------
osThreadId gsOpenGLMonitor::renderContextCurrentThread(int renderContextId) const
{
    osThreadId retVal = _threadsMonitor.renderContextCurrentThread(renderContextId);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onContextCreation
// Description:
//   Is called when a render context is created.
//   Adds a render context to the list of OpenGL render contexts created by the
//   debugged application.
// Arguments:
//            pDeviceContext - The render context device context.
//            pRenderContext - The render context handle.
//            const int* attribList - the render context attributes list
//            bool isDebugFlagForced - true iff CodeXL forced a debug bit on / off
// Author:      Yaki Tebeka
// Date:        12/5/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onContextCreation(oaDeviceContextHandle pDeviceContext, oaOpenGLRenderContextHandle pRenderContext, apMonitoredFunctionId creationFunc, const int* attribList, bool isDebugFlagForced)
{
    // Lock the critical section that controls the access to _contextsMonitors
    osCriticalSectionLocker criticalSectionLocker(_renderContextsMonitorsCS);

    // Calculate the context Spy id:
    int contextSpyId = (int)_contextsMonitors.size();

    // Create a render context monitor:
    gsRenderContextMonitor* pNewRenderContextMonitor = new gsRenderContextMonitor(contextSpyId, pDeviceContext, pRenderContext, _shouldInitializePerformanceCounters, creationFunc, attribList, isDebugFlagForced);

    // Add the new context data to the _contextsMonitors vector:
    _contextsMonitors.push_back(pNewRenderContextMonitor);

    // Initialize the render context monitor:
    initializeRenderContextMonitor(*pNewRenderContextMonitor);

    // Leave the critical section:
    criticalSectionLocker.leaveCriticalSection();

    // Output debug message:
    gtString debugMsg;
    debugMsg.appendFormattedString(GS_STR_renderContextWasCreated, contextSpyId);
    GT_IF_WITH_ASSERT(apMonitoredFunctionsAmount > creationFunc)
    {
        debugMsg.append(L" (").append(apMonitoredFunctionsManager::instance().monitoredFunctionName(creationFunc)).append(')');
    }
    OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);

    if (_shouldInitializePerformanceCounters)
    {
        // Notify other classes about the context creation event:
        _spyPerformanceCountersMgr.onContextCreatedEvent(contextSpyId);
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            _ATIPerformanceCountersMgr.onContextCreatedEvent(contextSpyId);
        }
#endif
#endif
    }

    gsExtensionsManager& theExtensionsMgr = gsExtensionsManager::instance();
    theExtensionsMgr.onContextCreatedEvent(contextSpyId);

    // Notify the debugger about the context creation:
    osThreadId currentThreadId = osGetCurrentThreadId();
    apRenderContextCreatedEvent renderContextCreatedEvent(currentThreadId, contextSpyId);
    bool rcEve = suForwardEventToClient(renderContextCreatedEvent);
    GT_ASSERT(rcEve);
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::beforeContextDeletion
// Description: Is called when a render context is about to be deleted. Note
//              that this function comes before the actual call to gl..DeleteContext
// Author:      Uri Shomroni
// Date:        11/11/2008
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::beforeContextDeletion(oaOpenGLRenderContextHandle pRenderContext)
{
    // Search for the deleted context spy id:
    int deletedContextSpyId = renderContextSpyId(pRenderContext);

    // If we didn't find the input context:
    GT_IF_WITH_ASSERT_EX((deletedContextSpyId > 0), GS_STR_UnkownContextHandleUsed)
    {
        // Get the current execution mode:
        apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

        if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
        {
            // Check if this is the last context using allocated objects:
            int contextsAmount = amountOfContexts();
            unsigned int numberOfContextsWithSameLists = 0;

            int sharedContext = deletedContextSpyId;
            replaceContextIDWithListHolder(sharedContext);

            const gsRenderContextMonitor* pDeletedContextMonitor = NULL;

            for (int i = 1; i < contextsAmount; i++)
            {
                int currentContextShared = i;
                replaceContextIDWithListHolder(currentContextShared);

                const gsRenderContextMonitor* pCurrentRC = renderContextMonitor(i);
                GT_IF_WITH_ASSERT(pCurrentRC != NULL)
                {
                    if (currentContextShared == sharedContext)
                    {
                        // Only count RCs that weren't already deleted:
                        if (!pCurrentRC->wasDeleted())
                        {
                            numberOfContextsWithSameLists++;
                        }
                    }

                    // While we are running through the OpenGL Monitors, we need to notify
                    // the one about to be deleted that it is about to be deleted:
                    if (i == deletedContextSpyId)
                    {
                        // Perform before context deletion actions:
                        pDeletedContextMonitor = pCurrentRC;
                    }
                }
            }

            // We should have at least one context sharing the same lists - the context we deleted.
            GT_ASSERT(numberOfContextsWithSameLists > 0);

            GT_IF_WITH_ASSERT(pDeletedContextMonitor != NULL)
            {
                pDeletedContextMonitor->beforeContextDeletion();
            }

            // If this is the last context holding these lists:
            if (numberOfContextsWithSameLists == 1)
            {
                // Handle memory leak check:
                apContextID deleteContextID(AP_OPENGL_CONTEXT, deletedContextSpyId);
                gsMemoryMonitor::instance().beforeContextDeletion(deleteContextID);
            }
        }

        // Output debug message:
        gtString debugMsg;
        debugMsg.appendFormattedString(GS_STR_renderContextAboutToBeDeleted, deletedContextSpyId);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);

        // Notify the debugger about the context deletion:
        osThreadId currentThreadId = osGetCurrentThreadId();
        apRenderContextDeletedEvent renderContextDeletedEvent(currentThreadId, deletedContextSpyId);
        bool rcEve = suForwardEventToClient(renderContextDeletedEvent);
        GT_ASSERT(rcEve);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::afterContextDeletion
// Description: Is called after a render context is deleted. Note that in windows,
//              this function is called only if the call to wglDeleteContext is
//              successful.
// Arguments:   pRenderContext - The render context win32 HGLRC
// Author:      Yaki Tebeka
// Date:        27/7/2005
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::afterContextDeletion(oaOpenGLRenderContextHandle pRenderContext)
{
    // Search for the deleted context spy id:
    int deletedContextSpyId = renderContextSpyId(pRenderContext);

    // If we didn't find the input context:
    GT_IF_WITH_ASSERT_EX((deletedContextSpyId > 0), GS_STR_UnkownContextHandleUsed)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // Remove the context from being a current context of any thread:
        _threadsMonitor.removeRenderContextFromAllThreads(pRenderContext);
#endif

        if (_shouldInitializePerformanceCounters)
        {
            // Notify the spy performance counters manager about the context deletion event:
            _spyPerformanceCountersMgr.onContextDeletedEvent(deletedContextSpyId);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
#ifdef OA_DEBUGGER_USE_AMD_GPA
            {
                // Notify the ATI performance counters manager about the context deletion event:
                _ATIPerformanceCountersMgr.onContextDeletedEvent(deletedContextSpyId);
            }
#endif
#endif
        }

        // Notify the render context monitor about the context deletion event:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(deletedContextSpyId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            pContextMonitor->onContextDeletion();
        }

        // Output debug message:
        gtString debugMsg;
        debugMsg.appendFormattedString(GS_STR_renderContextWasDeleted, deletedContextSpyId);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);

        // Notice:
        // We don't delete the render context monitor since we would like
        // to keep its data (calls history log, etc)).
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::currentThreadRenderContextSpyId
// Description: Returns the current render context of the calling thread.
// Return Val:  int - Will get the spy id of the render context that is the
//                    "current render context" of the thread that called this function.
// Author:      Yaki Tebeka
// Date:        6/11/2005
// ---------------------------------------------------------------------------
int gsOpenGLMonitor::currentThreadRenderContextSpyId() const
{
    int retVal = _threadsMonitor.currentThreadRenderContextSpyId();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::beforeContextMadeCurrent
// Description: Is called before a new context is made the current context
//              of the calling thread.
// Arguments: pRenderContext - The context that is going to be the new
//                             "current context" of the calling thread.
// Author:      Yaki Tebeka
// Date:        3/11/2005
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::beforeContextMadeCurrent(oaOpenGLRenderContextHandle hRC)
{
    // Get the "old" render context of the calling thread:
    int oldContextId = currentThreadRenderContextSpyId();

    // If this is a real context:
    if (0 < oldContextId)
    {
        // Will get the input context spy id:
        int newContextId = 0;

        // if this input context is the NULL context:
        void* phRC = (void*)hRC;

        if (phRC == NULL)
        {
            newContextId = 0;
        }
        else
        {
            // Get the input context spy id:
            newContextId = renderContextSpyId(hRC);

            if (newContextId == -1)
            {
                // Error: Unrecognized context:
                newContextId = 0;
                GT_ASSERT_EX(0, GS_STR_UnkownContextHandleUsed);
            }
        }

        // It this is a "real" context change:
        if (oldContextId != newContextId)
        {
            gsRenderContextMonitor* pContextMonitor = renderContextMonitor(oldContextId);
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                // Notify the context that it was removed from being the current context of a thread:
                pContextMonitor->onContextRemovedFromBeingCurrent();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onContextMadeCurrent
// Description:
//  Is called when the debugged application makes a context the active context
//  of the current thread.
// Arguments:   hDC - OS handle of the device context of the draw surface.
//                     draw - The drawable into which OpenGL will draw.
//                     read - The drawable from which OpenGL will read.
//                      hRC - The render context OS handle.
// Author:      Yaki Tebeka
// Date:        12/5/2004
// Implementation Notes:
//   We expect:
//   - The amount of contexts to be low.
//   - The amount of active context switches to be low.
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onContextMadeCurrent(oaDeviceContextHandle hDC, oaDrawableHandle draw, oaDrawableHandle read, oaOpenGLRenderContextHandle hRC)
{
    // Will get the input context spy id:
    int contextId = 0;

    // If this is a real context:
    void* phRC = (void*)hRC;

    if (phRC != NULL)
    {
        // Get the context spy id:
        contextId = renderContextSpyId(hRC);
    }

    // If this is not a registered context:
    if (contextId == -1)
    {
        // Mark that this thread uses an unknown context:
        contextId = AP_NULL_CONTEXT_ID;

        // Trigger an assertion:
        GT_ASSERT_EX(0, GS_STR_UnkownContextHandleUsed);
    }

    // Log the current thread context id:
    _threadsMonitor.setCurrentThreadCurrentContext(hDC, draw, read, contextId);

    if (contextId != AP_NULL_CONTEXT_ID)
    {
        // Notify the context monitor that its context became the active context:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            pContextMonitor->onContextMadeCurrent(hDC);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onFrameTerminatorCall
// Description: Is called when a frame terminator function is called.
// Author:      Yaki Tebeka
// Date:        13/3/2006
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onFrameTerminatorCall()
{
    // In profile mode, we do the checks here instead of in addFunctionCall()
    // to slow down execution less often:
    su_stat_theBreakpointsManager.waitOnDebuggedProcessSuspension();

    // Get the current thread render context monitor:
    suContextMonitor* pCurrentThreadRenderContextMonitor = currentThreadContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // If we are supposed to break on the next frame terminator:
        if (su_stat_theBreakpointsManager.breakAtTheNextFrame())
        {
            // clear the flag:
            su_stat_theBreakpointsManager.setBreakAtTheNextFrame(false);

            // break the execution (frame terminators will never be in a begin-end block):
            triggerBreakpointException(AP_FRAME_BREAKPOINT_HIT, GL_NO_ERROR, false);
        }

        // Notify the render context monitor:
        pCurrentThreadRenderContextMonitor->onFrameTerminatorCall();

        // If we are in profiling mode:
        apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

        if (debuggedProcessExecutionMode == AP_PROFILING_MODE)
        {
            // If we should break the debugged process execution:
            if (su_stat_theBreakpointsManager.isBreakDebuggedProcessExecutionFlagOn())
            {
                // Break the debugged process execution:
                apContextID contextId(AP_OPENGL_CONTEXT, currentThreadRenderContextSpyId());
                su_stat_theBreakpointsManager.testAndTriggerBeforeFuncExecutionBreakpoints(ap_glClear, contextId);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::verifyOpenGLServerInitialized
// Description: Makes sure the OpenGL server is initialized.
// Author:      Uri Shomroni
// Date:        29/6/2016
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::verifyOpenGLServerInitialized()
{
    // If the OpenGL Server was not initialized yet, initialize it:
    if (!_wasOpenGLServerInitialized)
    {
        bool rcSpyInit = gsOpenGLSpyInit();
        GT_ASSERT(rcSpyInit);
        _wasOpenGLServerInitialized = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::renderContextDeviceContext
// Description: Returns the device context of a given render context.
// Author:      Yaki Tebeka
// Date:        6/9/2004
// ---------------------------------------------------------------------------
void* gsOpenGLMonitor::renderContextDeviceContext(int renderContextId)
{
    void* retVal = NULL;

    // Sanity test:
    int renderContextsAmount = amountOfContexts();

    if ((0 < renderContextId) && (renderContextId < renderContextsAmount))
    {
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(renderContextId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            pContextMonitor->deviceContextOSHandle();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::renderContextPixelFormat
// Description: Inputs a render context and returns its selected pixel format.
// Arguments:   renderContextId - The render context spy id.
// Return Val:  int - Will get the pixel format OS id.
//                    (Or -1 in case of failure)
// Author:      Yaki Tebeka
// Date:        6/9/2004
// ---------------------------------------------------------------------------
oaPixelFormatId gsOpenGLMonitor::renderContextPixelFormat(int renderContextId)
{
    oaPixelFormatId retVal = OA_NO_PIXEL_FORMAT_ID;

    // Sanity test:
    int renderContextsAmount = amountOfContexts();

    if ((0 < renderContextId) && (renderContextId < renderContextsAmount))
    {
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(renderContextId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            retVal = pContextMonitor->pixelFormatId();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onShareLists
// Description:
// Arguments: a - the context "giving" the list
//            b - the context "taking" the list
//              This is supposed to share display lists, VBOs/IBOs, programs + shaders,
//              textures, FBOs, PBOs
// Author:      Uri Shomroni
// Date:        11/6/2008
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onShareLists(oaOpenGLRenderContextHandle firstContextHandle, oaOpenGLRenderContextHandle secondContextHandle)
{
    // Get render context spy ids:
    int firstContextSpyId = renderContextSpyId(firstContextHandle);
    int secondContextSpyId = renderContextSpyId(secondContextHandle);

    // Get the first and second context monitors:
    gsRenderContextMonitor* pFirstContextMonitor = renderContextMonitor(firstContextSpyId);
    gsRenderContextMonitor* pSecondContextMonitor = renderContextMonitor(secondContextSpyId);

    GT_IF_WITH_ASSERT((firstContextSpyId > 0) && (secondContextSpyId > 0) && (pSecondContextMonitor != NULL) && (pFirstContextMonitor != NULL))
    {
        // Get the second context sharing object id:
        int secondObjectSharingContextID = pSecondContextMonitor->getObjectSharingContextID();
        gsRenderContextMonitor* pSecondObjectSharingContextIDMonitor = renderContextMonitor(secondObjectSharingContextID);

        // If we try to get the list from a context which itself shares a list, we go directly to the
        // source rather than have a chain.
        replaceContextIDWithListHolder(secondContextSpyId);

        // If we're using circular logic (eg 1->2 and then 2->1), ignore the command as it's ineffective.
        if (firstContextSpyId != secondContextSpyId)
        {
            // Set this render context to point at the other one
            pSecondContextMonitor->setObjectSharingContext(firstContextSpyId, pFirstContextMonitor, true);

            // If the we pointed to another context before, update it on  the change:
            if (pSecondObjectSharingContextIDMonitor != NULL)
            {
                pSecondObjectSharingContextIDMonitor->setObjectSharingContext(firstContextSpyId, pFirstContextMonitor);
            }

            // Make sure that render contexts pointing to us or the context we pointed to are notified:
            int numberOfRCs = (int)_contextsMonitors.size();

            for (int l = 1; l < numberOfRCs; l++)
            {
                // Get curret render context monitor:
                gsRenderContextMonitor* pCurrentContextMonitor = renderContextMonitor(l);

                int lp = pCurrentContextMonitor->getObjectSharingContextID();

                if ((lp == secondContextSpyId) || ((secondObjectSharingContextID != -1) && (lp == secondObjectSharingContextID)))
                {
                    pCurrentContextMonitor->setObjectSharingContext(firstContextSpyId, pFirstContextMonitor);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        replaceContextIDWithListHolder
// Description: If the renderContext number renderContextId is sharing another
//              context's lists, change that variable to represent the context
//              which holds the information.
// Author:      Uri Shomroni
// Date:        11/6/2008
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::replaceContextIDWithListHolder(int& renderContextId) const
{
    const gsRenderContextMonitor* pContextMonitor = renderContextMonitor(renderContextId);

    if (pContextMonitor != NULL)
    {
        int shareListsHolder = pContextMonitor->getObjectSharingContextID();

        if (shareListsHolder != -1)
        {
            renderContextId = shareListsHolder;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::renderContextMonitor
// Description: Inputs a render context id and returns its render context monitor.
//              (Or null, if a context of that id does not exist)
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
const gsRenderContextMonitor* gsOpenGLMonitor::renderContextMonitor(int renderContextId) const
{
    const gsRenderContextMonitor* retVal = NULL;

    // Index range test:
    int renderContextsAmount = amountOfContexts();
    GT_IF_WITH_ASSERT((0 < renderContextId) && (renderContextId < renderContextsAmount))
    {
        retVal = (const gsRenderContextMonitor*)_contextsMonitors[renderContextId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::renderContextMonitor
// Description: Inputs a render context id and returns its render context monitor.
//              (Or null, if a context of that id does not exist)
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
gsRenderContextMonitor* gsOpenGLMonitor::renderContextMonitor(int renderContextId)
{
    gsRenderContextMonitor* retVal = NULL;

    // Index range test:
    int renderContextsAmount = amountOfContexts();
    GT_IF_WITH_ASSERT((0 < renderContextId) && (renderContextId < renderContextsAmount))
    {
        retVal = (gsRenderContextMonitor*)_contextsMonitors[renderContextId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::currentThreadRenderContextMonitor
// Description: Returns const access to the current thread's render context monitor.
//              or NULL if the current thread does not have a current render context.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
const gsRenderContextMonitor* gsOpenGLMonitor::currentThreadRenderContextMonitor() const
{
    const gsRenderContextMonitor* pRetVal = NULL;
    int currentContextId = currentThreadRenderContextSpyId();

    if (currentContextId > 0)
    {
        pRetVal = renderContextMonitor(currentContextId);
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::currentThreadRenderContextMonitor
// Description: Returns the current thread's render context monitor.
//              or NULL if the current thread does not have a current render
//              context.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
gsRenderContextMonitor* gsOpenGLMonitor::currentThreadRenderContextMonitor()
{
    gsRenderContextMonitor* pRetVal = NULL;
    int currentContextId = currentThreadRenderContextSpyId();

    // If this is not a NULL context:
    if (currentContextId > 0)
    {
        pRetVal = renderContextMonitor(currentContextId);
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::currentThreadContextMonitor
// Description: Returns const access to the current thread's context monitor.
//              When the current thread's context is NULL, return a pointer to the
//              none context monitor
//              Notice: Use this function when the object needed is a pointer to an
//              suContextMonitor. Only when gsRenderContextMonitor functionality is
//              requested, use currentThreadRenderContextMonitor
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
const suContextMonitor* gsOpenGLMonitor::currentThreadContextMonitor() const
{
    const suContextMonitor* pRetVal = NULL;
    int currentContextId = currentThreadRenderContextSpyId();

    if (currentContextId >= 0)
    {
        pRetVal = _contextsMonitors[currentContextId];
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::currentThreadContextMonitor
// Description: Returns const access to the current thread's context monitor.
//              When the current thread's context is NULL, return a pointer to the
//              none context monitor
//              Notice: Use this function when the object needed is a pointer to an
//              suContextMonitor. Only when gsRenderContextMonitor functionality is
//              requested, use currentThreadRenderContextMonitor
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
suContextMonitor* gsOpenGLMonitor::currentThreadContextMonitor()
{
    suContextMonitor* pRetVal = NULL;
    int currentContextId = currentThreadRenderContextSpyId();

    // If this is not a NULL context:
    if (currentContextId >= 0)
    {
        pRetVal = _contextsMonitors[currentContextId];
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::boundTexture
// Description: Inputs a bind target and returns the name of the texture that is
//              bind to this target.
// Arguments:   bindTarget - The queried bind target.
// Return Val:  GLuint - The bounded texture, or 0 if there is no texture bounded
//                       to this target.
// Author:      Yaki Tebeka
// Date:        12/1/2005
// ---------------------------------------------------------------------------
GLuint gsOpenGLMonitor::boundTexture(GLenum bindTarget) const
{
    GLuint retVal = 0;
    apTextureType bindTargetAsTextureType = apTextureBindTargetToTextureType(bindTarget);

    if (bindTargetAsTextureType != AP_UNKNOWN_TEXTURE_TYPE)
    {
        // Get the current render context monitor:
        const gsRenderContextMonitor* pRenderContextMonitor = currentThreadRenderContextMonitor();
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            pRenderContextMonitor->activeTextureUnitIndex();
            int activeTexUnitIndex = pRenderContextMonitor->activeTextureUnitIndex();
            retVal = pRenderContextMonitor->bindTextureName(activeTexUnitIndex, bindTargetAsTextureType);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::forcedStubTextureName
// Description: Returns the forced stub texture name for an input bind target.
// Author:      Yaki Tebeka
// Date:        4/3/2005
// ---------------------------------------------------------------------------
GLuint gsOpenGLMonitor::forcedStubTextureName(GLenum bindTarget) const
{
    GLuint retVal = 0;
    // Get the current render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        retVal = texturesMtr->forcedStubTextureObjectName(bindTarget);
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::activeProgram
// Description: Returns the currently active program for this thread.
//              This is to be used in function wrappers.
// Author:      Uri Shomroni
// Date:        9/9/2013
// ---------------------------------------------------------------------------
GLuint gsOpenGLMonitor::activeProgram() const
{
    GLuint retVal = 0;

    const gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (NULL != pCurrentThreadRenderContextMonitor)
    {
        retVal = pCurrentThreadRenderContextMonitor->activeProgramName();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::addFunctionCall
// Description:
//   Logs a monitored function call into the active render context call history logger.
//   Triggers monitored functions breakpoint events.
// Arguments: apMonitoredFunctionId calledFunctionId - the logged function id.
//            argumentsAmount - The amount of function arguments.
//            ... - Function arguments in va_list style.
// Author:      Yaki Tebeka
// Date:        11/5/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, ...)
{
    // If the OpenGL Server was not initialized yet, initialize it:
    if (!_wasOpenGLServerInitialized)
    {
        bool rcSpyInit = gsOpenGLSpyInit();
        GT_ASSERT(rcSpyInit);
        _wasOpenGLServerInitialized = true;
    }

    // Get the render context that is current to the calling thread:
    suContextMonitor* pRenderContextMonitor = currentThreadContextMonitor();
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Notify the render context monitor that a monitored function was called:
        pRenderContextMonitor->onMonitoredFunctionCall();

        // Do not perform the below actions while in profiling mode:
        apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

        if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
        {
            // If the render context is not during context data update:
            bool isDuringContextDataUpdate = pRenderContextMonitor->isDuringContextDataUpdate();

            if (!isDuringContextDataUpdate)
            {
                // Get a pointer to the arguments list:
                va_list pCurrentArgument;
                va_start(pCurrentArgument, argumentsAmount);

                su_stat_theBreakpointsManager.waitOnDebuggedProcessSuspension();

                // Add the function call to the render context loggers:
                pRenderContextMonitor->addFunctionCall(calledFunctionId, argumentsAmount, pCurrentArgument, AP_DEPRECATION_NONE);

                // Execute before monitored function execution actions:
                beforeMonitoredFunctionExecutionActions(calledFunctionId);

                // Free the arguments list:
                va_end(pCurrentArgument);
            }
        }
    }

    // ------------ Spy debugging code --------------

    // Notice: The below section is used for spy debugging.
    //         Please leave it commented out.


    bool foo1 = false;

    if (foo1)
    {
        gtVector<GLuint> vbos;
        vbos.push_back(1);
        gaUpdateVBORawDataImpl(1, vbos);
    }

    /*
    bool foo2 = false;
    if (foo2)
    {
    int foundIndex = -1;
    bool rc = gaFindCurrentFrameFunctionCallImpl(1, AP_SEARCH_INDICES_DOWN,0, "glcolor", false,foundIndex);
    }*/

    /*
    bool foo3 = false;
    if (foo3)
    {
    gtPtrVector<apFunctionCallStatistics*> vecvec;
    bool rc = gaGetLastFrameFunctionCallsStatisticsImpl(1, vecvec);

    int i = 3;
    i++;
    }
    */
    /*
    bool foo4 = false;
    if (foo4)
    {
    gtVector<GLuint> textureNames;
    textureNames.push_back(1);
    bool rc = gaUpdateCurrentThreadTextureRawDataImpl(textureNames);

    int i = 3;
    i++;
    }
    */
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::beforeMonitoredFunctionExecutionActions
// Description: Perform OpenGL monitor actions that should be executed BEFORE
//              the monitored function is executed.
// Arguments: apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::beforeMonitoredFunctionExecutionActions(apMonitoredFunctionId monitoredFunctionId)
{
    // Test and trigger BEFORE function execution breakpoints:
    testAndTriggerBeforeFuncExecutionBreakpoints(monitoredFunctionId);

    // If slow motion mode is on:
    int slowMotionDelayTimeUnits = suSlowMotionDelayTimeUnits();

    if (slowMotionDelayTimeUnits > 0)
    {
        // Perform the slow motion delay:
        suPerformSlowMotionDelay(slowMotionDelayTimeUnits);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::afterMonitoredFunctionExecutionActions
// Description: Perform OpenGL monitor actions that should be executed AFTER
//              the monitored function is executed.
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId)
{
    // Do not perform the below actions in profiling mode:
    apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

    if (debuggedProcessExecutionMode != AP_PROFILING_MODE)
    {
        // Will get true iff we are during context data snapshot update:
        bool isDuringContextDataUpdate = false;

        // Get current thread's render context monitor:
        suContextMonitor* pContextMonitor = currentThreadContextMonitor();
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // If the render context is not during context data update:
            isDuringContextDataUpdate = pContextMonitor->isDuringContextDataUpdate();

            if (!isDuringContextDataUpdate)
            {
                // Get the function calls monitor:
                gsCallsHistoryLogger* pActiveContextLogger = (gsCallsHistoryLogger*)pContextMonitor->callsHistoryLogger();
                GT_IF_WITH_ASSERT(pActiveContextLogger != NULL)
                {
                    // If OpenGL flush is forced:
                    if (_isOpenGLFlushForced)
                    {
#ifdef _GR_IPHONE_BUILD
                        // The iPhone cannot draw directly into the front buffer, so we simply swap the buffers:
                        gsRenderContextMonitor* pRenderContextMonitor = (gsRenderContextMonitor*)pContextMonitor;

                        if (pRenderContextMonitor != NULL)
                        {
                            gsSwapEAGLContextBuffers(pRenderContextMonitor->renderContextOSHandle());
                        }

#else

                        // If we are not in a glBegin - glEnd block:
                        if (!(pActiveContextLogger->isInOpenGLBeginEndBlock()))
                        {
                            // Flush the graphics pipeline:
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFlush);
                            gs_stat_realFunctionPointers.glFlush();
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFlush);
                        }

#endif
                    }

                }

                // Update state change function status:
                pContextMonitor->afterMonitoredFunctionExecutionActions(calledFunctionId);
            }
        }

        if (!isDuringContextDataUpdate)
        {
            // Test and trigger AFTER function execution breakpoints:
            testAndTriggerAfterFuncExecutionBreakpoints(calledFunctionId);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::onGenericBreakpointSet
// Description: Set / Unset a generic breakpoint
// Arguments:    apGenericBreakpointType breakpointType
//              bool isOn
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        7/7/2011
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::onGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn)
{
    if (breakpointType == AP_BREAK_ON_GL_ERROR)
    {
        // Notify the contexts forced modes managers:
        int amountOfContexts = (int)_contextsMonitors.size();

        for (int i = 1; i < amountOfContexts; i++)
        {
            // Get the render context monitor:
            gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
            GT_IF_WITH_ASSERT(pContextMonitor != NULL)
            {
                pContextMonitor->onBreakOnOpenGLErrorModeChanged(isOn);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::forceOpenGLPolygonRasterMode
// Description:
//  Force an input raster mode on OpenGL.
//  I.E: OpenGL API calls will not be able to change the raster mode that this
//       function sets.
//
// Arguments:   rasterMode - The forced raster mode.
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::forceOpenGLPolygonRasterMode(apRasterMode rasterMode)
{
    // Notify the initialize force mode manager with the change:
    _initForceModesManager.forcePolygonRasterMode(rasterMode);

    // Notify all the active render contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        // Get the render context monitor:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            gsForcedModesManager& contextForcedModesMgr = pContextMonitor->forcedModesManager();
            contextForcedModesMgr.forcePolygonRasterMode(rasterMode);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::cancelOpenGLPolygonRasterModeForcing
// Description: Cancels the "Force OpenGL render mode" mode.
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::cancelOpenGLPolygonRasterModeForcing()
{
    // Notify the initialize force mode manager with the change:
    _initForceModesManager.cancelPolygonRasterModeForcing();

    // Notify all the active render contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        // Get the render context monitor:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            gsForcedModesManager& contextForcedModesMgr = pContextMonitor->forcedModesManager();
            contextForcedModesMgr.cancelPolygonRasterModeForcing();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::forceStubMode
// Description: Forces a stub mode.
// Arguments:   apOpenGLForcedModeType stubType - the stub type
//              bool isStubForced
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        9/5/2010
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::forceStubMode(apOpenGLForcedModeType stubType, bool isStubForced)
{
    // Notify the initialize force mode manager with the change:
    _initForceModesManager.forceStub(stubType, isStubForced);

    // Notify all the active render contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        // Get the render context monitor:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            gsForcedModesManager& contextForcedModesMgr = pContextMonitor->forcedModesManager();
            contextForcedModesMgr.forceStub(stubType, isStubForced);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::setGLDebugOutputLoggingEnabled
// Description: Applies debug output logging parameter to all contexts
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::setGLDebugOutputLoggingEnabled(bool enabled)
{
    // Notify the initialize force mode manager with the change:
    _initForceModesManager.setGLDebugOutputLoggingEnabled(enabled);

    // Notify all the active render contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        // Get the render context monitor:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            gsForcedModesManager& contextForcedModesMgr = pContextMonitor->forcedModesManager();
            contextForcedModesMgr.setGLDebugOutputLoggingEnabled(enabled);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::setGLDebugOutputSeverityEnabled
// Description: Applies debug output logging parameter to all contexts
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::setGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled)
{
    // Notify the initialize force mode manager with the change:
    _initForceModesManager.setGLDebugOutputSeverityEnabled(severity, enabled);

    // Notify all the active render contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        // Get the render context monitor:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            gsForcedModesManager& contextForcedModesMgr = pContextMonitor->forcedModesManager();
            contextForcedModesMgr.setGLDebugOutputSeverityEnabled(severity, enabled);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::setGLDebugOutputKindMask
// Description: Applies debug output logging parameter to all contexts
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::setGLDebugOutputKindMask(const gtUInt64& mask)
{
    // Notify the initialize force mode manager with the change:
    _initForceModesManager.setGLDebugOutputKindMask(mask);

    // Notify all the active render contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        // Get the render context monitor:
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            gsForcedModesManager& contextForcedModesMgr = pContextMonitor->forcedModesManager();
            contextForcedModesMgr.setGLDebugOutputKindMask(mask);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::handleForeignContextExtensionFuncCall
// Description:
//   Handles a case in which the debugged process calls an extgension function
//   who's pointer was not retrieved into the current context.
//
//   - On OpenGL + WGL, this is happens when the debugged process asks for an extension
//     function pointer at context A, but calls the function pointer while context B is
//     the active context. This is illegal in WGL terminology, therefore we rasize an
//     AP_FOREIGN_CONTEXT_EXTENSION_FUNC_CALL_ERROR error.
//
//   - In OpenGL ES + EGL this is a legal operation. Therefore, no error is triggered.
//
// Arguments:   extensionFunctionName - The name of the called extension function.
// Author:      Yaki Tebeka
// Date:        22/3/2005
// Implementation details:
//   - To imitate current drivers action at this case: look for this function pointer
//     at other contexts, and copy the function pointer to this context extension
//     function pointers structure.
//   - If the function pointer is not found in other contexts, try getting it using
//     wglGetProcAddress (this happends when using OpenGL ES + EGL, where some extension
//     functions are now OpenGL ES core functions).
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::handleForeignContextExtensionFuncCall(const wchar_t* extensionFunctionName,
                                                            bool& extensionFuncPtrFound)
{
    extensionFuncPtrFound = false;

    // Get the associated function id:
    apMonitoredFunctionId associatedFuncId = su_stat_theMonitoredFunMgr.monitoredFunctionId(extensionFunctionName);

    // Sanity check:
    GT_IF_WITH_ASSERT(associatedFuncId != -1)
    {
        // Try to copy the function pointer from another context:
        gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();
        extensionFuncPtrFound = theExtensionsManager.copyExtensionPointerFromOtherContexts(associatedFuncId);

        // If we didn't find the extension function pointer in other contexts:
        // (This can happen when calling OpenGL ES core functions that are OpenGL extension functions)
        if (!extensionFuncPtrFound)
        {
            // Try to get the extension function pointer from the system:
            extensionFuncPtrFound = theExtensionsManager.getExtensionPointerFromSystem(associatedFuncId);
            GT_ASSERT(extensionFuncPtrFound);
        }

        // Getting extension function pointer from one render context and called this function pointer in another render context
        // is not legal under WGL. However, it is legal under GLX and EGL.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        {
#ifndef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
            {
                // Generate a detected process event:
                int currentContextId = currentThreadRenderContextSpyId();
                gtString errorDescription = GS_STR_foreignExtensionCallError1;
                errorDescription.appendFormattedString(L"(%ls)", extensionFunctionName);
                errorDescription += GS_STR_foreignExtensionCallError2;
                errorDescription.appendFormattedString(L" (context #%d) ", currentContextId);

                if (!extensionFuncPtrFound)
                {
                    errorDescription += GS_STR_extensionCallIgnored;
                }

                // Yaki 12/10/2005:
                // A lot of users that use GLEW (or other OpenGL extensions wrapping libraries) complained that they
                // get this error a lot of times. This force them to disable the "Break on detected errors" test and they
                // cannot use it to catch other errors.
                // So, Instead of reporting a detected error and breaking the debugged process run, we only generate a
                // debug string (for more details, see Case 470).
                gtString dbgString = L"CodeXL warning: ";
                dbgString += errorDescription;
                osOutputDebugString(dbgString);

                // reportDetectedError(AP_FOREIGN_CONTEXT_EXTENSION_FUNC_CALL_ERROR, errorDescription, associatedFuncId);
            }
#endif
        }
#endif
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::handleForeignContextExtensionFuncCall
// Description: For Linux builds, converts the first parameter of
//              handleForeignContextExtensionFuncCall from an ASCII string
// Author:      Uri Shomroni
// Date:        19/7/2011
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::handleForeignContextExtensionFuncCall(const char* extensionFunctionName,
                                                            bool& extensionFuncPtrFound)
{
    // Convert to unicode:
    gtString extensionFunctionNameAsUnicodeString;
    extensionFunctionNameAsUnicodeString.fromASCIIString(extensionFunctionName);

    // Send to the real function:
    handleForeignContextExtensionFuncCall(extensionFunctionNameAsUnicodeString.asCharArray(), extensionFuncPtrFound);
}

// -------------------- private functions ------------------


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::testOpenGLBreakpoints
// Description: The function checks for specific OpenGL breakpoints, and set the
//              break reason if there is a breakpoint.
// Author:      Sigal Algranaty
// Date:        26/11/2009
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::testOpenGLBreakpoints(apMonitoredFunctionId monitoredFunctionId, apBreakReason& breakReason)
{
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    if (su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_SOFTWARE_FALLBACK))
    {
        // Check if the function is a draw function:
        bool isDrawFunction = ((apMonitoredFunctionsManager::instance().monitoredFunctionType(monitoredFunctionId) & AP_DRAW_FUNC) != 0);

        if (isDrawFunction)
        {
            // Check for software fallbacks:
            bool vertexSFB = false;
            bool fragmentSFB = false;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            {
                // This only works with CGL:
#ifndef _GR_IPHONE_BUILD
                // In Mac OS X, we check for software fallbacks by checking after each
                // draw call if the GPU is used for vertex operations and fragment ones:
                GLint paramValue = 0;
                CGLContextObj currentContext = currentThreadRenderContextMonitor()->renderContextOSHandle();

                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
                CGLError errCode = gs_stat_realFunctionPointers.CGLGetParameter(currentContext , kCGLCPGPUVertexProcessingNoDispatch, &paramValue);
                GT_IF_WITH_ASSERT(errCode == kCGLNoError)
                {
                    vertexSFB = (paramValue == GL_FALSE);
                }

                paramValue = 0;
                errCode = gs_stat_realFunctionPointers.CGLGetParameter(currentContext , kCGLCPGPUFragmentProcessingNoDispatch, &paramValue);
                GT_IF_WITH_ASSERT(errCode == kCGLNoError)
                {
                    fragmentSFB = (paramValue == GL_FALSE);
                }

                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
#endif
            }
#endif

            if (vertexSFB || fragmentSFB)
            {
                // We had a Software Fallback, report it!
                breakReason = AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT;
            }
        }
    }

#else
    // Resolve the compiler warning for the Linux variant
    (void)(monitoredFunctionId);
    (void)(breakReason);
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::testAndTriggerBeforeFuncExecutionBreakpoints
// Description:
//  Tests if there is a breakpoint at the input monitored function.
//  If there is - triggers a breakpoint event.
//
//  This function tests for breakpoints that should be raised BEFORE the
//  monitored function is executed.
// Arguments: apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Yaki Tebeka
// Date:        16/6/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::testAndTriggerBeforeFuncExecutionBreakpoints(apMonitoredFunctionId monitoredFunctionId)
{
    // Initialize a break reason:
    apBreakReason breakReason = AP_FOREIGN_BREAK_HIT;

    // First test OpenGL specific breakpoints reason:
    testOpenGLBreakpoints(monitoredFunctionId, breakReason);

    // Check if we are in a begin-end block:
    int currentContextId = currentThreadRenderContextSpyId();
    bool isInGLBeginEndBlock = false;

    // Get the render context wrapper object:
    if (currentContextId != AP_NULL_CONTEXT_ID)
    {
        gsRenderContextMonitor* pRenderContextMonitor = renderContextMonitor(currentContextId);

        if (pRenderContextMonitor != NULL)
        {
            isInGLBeginEndBlock = pRenderContextMonitor->isInOpenGLBeginEndBlock();

            // If this context needs to test for undetected OpenGL errors, do so now:
            if (su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_GL_ERROR))
            {
                if (pRenderContextMonitor->isOpenGLErrorCheckNeededOnModeTurnedOn())
                {
                    pRenderContextMonitor->checkForOpenGLErrorAfterModeChange();
                }
            }
        }
    }

    if (breakReason == AP_FOREIGN_BREAK_HIT)
    {
        // Check the 'common' (spy utilities) breakpoints reasons:
        apContextID contextId(AP_OPENGL_CONTEXT, currentContextId);
        su_stat_theBreakpointsManager.testAndTriggerBeforeFuncExecutionBreakpoints(monitoredFunctionId, contextId, isInGLBeginEndBlock);
    }
    else // (breakReason != AP_FOREIGN_BREAK_HIT)
    {
        // Trigger a breakpoint exception:
        triggerBreakpointException(breakReason, GL_NO_ERROR, isInGLBeginEndBlock);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::triggerBreakpointException
// Description: Allows clients to trigger a breakpoint exception.
// Arguments: breakReason - The reason why we triggered the breakpoint exception.
//            opeglError - OpenGL error associated with the triggered breakpoint,
//                         or GL_NO_ERROR if there is no associated error.
//            isInGLBeginEndBlock - true iff we are in a glBegin-glEnd block.
// Author:      Yaki Tebeka
// Date:        8/3/2006
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::triggerBreakpointException(apBreakReason breakReason, GLenum opeglError, bool isInGLBeginEndBlock)
{
    // Log the break reason and error (they are retrieved by the debugger when it
    // catch the breakpoint exception):
    _openGLError = opeglError;

    // Set the breakpoint reason:
    su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
    su_stat_theBreakpointsManager.setBreakReason(breakReason);

    // Get the breakpoint function Id:
    apMonitoredFunctionId monitoredFunctionId = apMonitoredFunctionsAmount;
    const gsRenderContextMonitor* pCurrentThreadContextMonitor = currentThreadRenderContextMonitor();

    if (pCurrentThreadContextMonitor != NULL)
    {
        const suCallsHistoryLogger* pCallsHistoryLogger = pCurrentThreadContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            monitoredFunctionId = pCallsHistoryLogger->lastCalledFunctionId();
        }
    }

    // Trigger breakpoint exception:
    int currentContextId = currentThreadRenderContextSpyId();
    apContextID contextId(AP_OPENGL_CONTEXT, currentContextId);
    su_stat_theBreakpointsManager.triggerBreakpointException(contextId, monitoredFunctionId, isInGLBeginEndBlock);
    su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::reportDetectedError
// Description: Reports a detected error from the current OpenGL context
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::reportDetectedError(apErrorCode errorCode, const gtString& errorDescription, apMonitoredFunctionId associatedFuncId)
{
    int currentThreadGLContext = currentThreadRenderContextSpyId();
    apContextID contextId(AP_OPENGL_CONTEXT, currentThreadGLContext);

    su_stat_theBreakpointsManager.reportDetectedError(contextId, errorCode, errorDescription, associatedFuncId);
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::testAndTriggerAfterFuncExecutionBreakpoints
// Description:
//  Tests if there is a breakpoint caused by input monitored function.
//  If there is - triggers a breakpoint event.
//
//  This function tests for breakpoints that should be raised AFTER the
//  monitored function is executed.
// Arguments: apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Yaki Tebeka
// Date:        16/6/2004
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::testAndTriggerAfterFuncExecutionBreakpoints(apMonitoredFunctionId monitoredFunctionId)
{
    // If we need to test for OpenGL errors:
    if (su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_GL_ERROR))
    {
        // If this is not the NULL context:
        int currentContextId = currentThreadRenderContextSpyId();

        if (currentContextId != AP_NULL_CONTEXT_ID)
        {
            const gsRenderContextMonitor* pRenderContextMonitor = renderContextMonitor(currentContextId);
            GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
            {
                // If the mode was just turned on, we need to wait for one more function call, so that the context won't
                // report former OpenGL errors as belonging to this function:

                if (!pRenderContextMonitor->isOpenGLErrorCheckNeededOnModeTurnedOn())
                {
                    bool isInOpenGLBeginEndBlock = pRenderContextMonitor->isInOpenGLBeginEndBlock();

                    // This is AFTER the function call, so we need to reverse the value for glBegin and glEnd:
                    if (monitoredFunctionId == ap_glBegin)
                    {
                        isInOpenGLBeginEndBlock = true;
                    }
                    else if (monitoredFunctionId == ap_glEnd)
                    {
                        isInOpenGLBeginEndBlock = false;
                    }

                    // If we are not at a glBegin glEnd block:
                    // (We are not allowed to call glGetError inside a glBegin glEnd block)
                    if (!isInOpenGLBeginEndBlock)
                    {
                        // If there is an OpenGL error:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
                        GLenum openGLError = gs_stat_realFunctionPointers.glGetError();
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

                        if (openGLError != GL_NO_ERROR)
                        {
                            // Trigger an "OpenGL error" breakpoint exception:
                            triggerBreakpointException(AP_OPENGL_ERROR_BREAKPOINT_HIT, openGLError, false);
                        }
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::updateCurrentContextDataSnapshot
// Description: Updates the data snapshot of the current thread's
//                    current render context.
// Author:      Yaki Tebeka
// Date:        28/1/2007
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::updateCurrentContextDataSnapshot()
{
    // Get the current thread render context:
    gsRenderContextMonitor* pRenderContextMon = currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pRenderContextMon != NULL)
    {
        // Get the context id:
        int renderContextId = pRenderContextMon->spyId();

        // If this is a real context:
        if (renderContextId > 0)
        {
            // Update the context data:
            bool rc = pRenderContextMon->updateContextDataSnapshot();
            GT_ASSERT(rc);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::initializeRenderContextMonitor
// Description: Initializes a render context monitor.
// Arguments: renderContextMonitor - The render context monitor to be initialized.
// Author:      Yaki Tebeka
// Date:        3/11/2005
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::initializeRenderContextMonitor(gsRenderContextMonitor& renderContextMonitor)
{
    // Get the context spy id:
    int contextSpyId = renderContextMonitor.spyId();

    // If the created context is not the NULL context:
    if ((contextSpyId != 0))
    {
        // Set the render context pixel format:
        // Get the context device context handle:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        oaDeviceContextHandle hDeviceContext = renderContextMonitor.deviceContextOSHandle();

        int pixelFormatId = ::GetPixelFormat(hDeviceContext);
        renderContextMonitor.setPixelFormatId(pixelFormatId);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
        // This is done when we create the context with glXCreateContext / CGLCreateContext
#else
#error Unsupported platform
#endif

        // Get the new context forced modes manager:
        gsForcedModesManager& newContextForcedModeMgr = renderContextMonitor.forcedModesManager();

        // Copy the force mode settings from the init force modes manager:
        newContextForcedModeMgr.copyForcedModesFromOtherManager(_initForceModesManager);
    }

    // If the text log files are active:
    bool areHTMLLogFilesActive = suAreHTMLLogFilesActive();

    if (areHTMLLogFilesActive)
    {
        // Start log file recording:
        suCallsHistoryLogger* pCallsLogger = renderContextMonitor.callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            pCallsLogger->startHTMLLogFileRecording();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::renderContextSpyId
// Description: Inputs a render context OS handle (casted into void*) and returns
//              the render context spy id.
//              (Or -1 if this context is not known to this class)
// Author:      Yaki Tebeka
// Date:        14/11/2005
// Implementation notes:
//   We assume that the amount of render context is low (up to 20), so a linear
//   search is a fast solution.
// ---------------------------------------------------------------------------
int gsOpenGLMonitor::renderContextSpyId(oaOpenGLRenderContextHandle pRenderContext)
{
    int retVal = -1;

    // Iterate spy monitored context (last context to first, assuming that the last
    // contexts are more active):
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = (amountOfContexts - 1); 0 < i; i--)
    {
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // We found the input context
            oaOpenGLRenderContextHandle currentRenderContextOSHandle = pContextMonitor->renderContextOSHandle();

            if (currentRenderContextOSHandle == pRenderContext)
            {
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::displayCurrentRenderContextContent
// Description:
//   Display the current render context content to the user.
//   I.E:
//   - For single buffered contexts - calls glFlush.
//   - For double buffered contexts - calls SwapBuffers.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gsOpenGLMonitor::displayCurrentRenderContextContent()
{
    bool retVal = true;

    // Get the render context wrapper:
    gsRenderContextMonitor* pActiveRenderCtxMtr = currentThreadRenderContextMonitor();

    if (pActiveRenderCtxMtr != NULL)
    {
        // Flush its content to its draw surface (usually the screen):
        retVal = pActiveRenderCtxMtr->flushContentToDrawSurface();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::doesDebugForcedContextExist
// Description: Check if there is a debug context with debug flag forced
// Arguments:   bool isDebugContext - is the debug flag on
// Author:      Sigal Algranaty
// Date:        7/7/2010
// ---------------------------------------------------------------------------
bool gsOpenGLMonitor::doesDebugForcedContextExist(bool isDebugContext)
{
    bool retVal = false;

    // Iterate the render contexts and search for a forced debug context:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < amountOfContexts; i++)
    {
        gsRenderContextMonitor* pContextMonitor = renderContextMonitor(i);

        if (pContextMonitor != NULL)
        {
            if (pContextMonitor->isDebugContextFlagForced() && pContextMonitor->isDebugContext() == isDebugContext)
            {
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::checkForProcessMemoryLeaks
// Description: Check for process termination memory leaks
// Author:      Sigal Algranaty
// Date:        18/7/2010
// ---------------------------------------------------------------------------
void gsOpenGLMonitor::checkForProcessMemoryLeaks()
{
    //Check memory leaks when OpenGL is unloaded:
    gsMemoryMonitor::instance().beforeSpyTermination();
}

