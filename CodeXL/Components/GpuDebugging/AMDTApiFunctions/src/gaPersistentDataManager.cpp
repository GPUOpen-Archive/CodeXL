//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaPersistentDataManager.cpp
///
//==================================================================================

//------------------------------ gaPersistentDataManager.cpp ------------------------------

// POSIX:
#include <limits.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apContextDataSnapshotWasUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessIsDuringTerminationEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedExternallyEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingInterruptedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>
// #include <AMDTPerformanceCounters/Include/pcPerformanceCountersManager.h>

// Local:
#include <src/gaStringConstants.h>
#include <src/gaPersistentDataManager.h>
#include <src/gaPrivateAPIFunctions.h>
#include <src/gaAPIToSpyConnector.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// Static members initializations:
gaPersistentDataManager* gaPersistentDataManager::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaPersistentDataManager& gaPersistentDataManager::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gaPersistentDataManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::gaPersistentDataManager
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaPersistentDataManager::gaPersistentDataManager()
    : _suspendDebuggedProcessExecution(false),
      _breakOnNextMonitoredFunctionCall(false),
      _breakOnNextDrawFunctionCall(false),
      _breakOnNextFrame(false),
      _breakInMonitoredFunctionCall(false),
      _deleteLogFilesWhenDebuggedProcessTerminates(true),
      _isHTMLLogFileRecordingOn(false),
      _wasOpenGLRecorderOn(false),
      _slowMotionDelayTimeUnits(0),
      _isFrontDrawBufferForced(false),
      _isOpenGLFlushForced(false),
      _isInteractiveBreakOn(true),
      _isImagesDataLoggingEnabled(true),
      _areAllocatedObjectsCreationCallsStacksCollected(true),
      _isLogFileFlushedAfterEachFunctionCall(false),
      _isOpenGLNullDriverForced(false),
      _forcedOpenGLRasterMode(AP_RASTER_FILL),
      _waitingForKernelDebuggingToStart(false),
      _kernelDebuggingInterruptedWarningIssued(false),
      _isInKernelDebugging(false),
      m_isInKernelDebuggingBreakpoint(false),
      _kernelDebuggingThreadId(OS_NO_THREAD_ID),
      _GLDebugOutputLoggingEnabled(false),
      m_debugOutputKindsMask(0),
      _isAPIConnectionEstablished(false),
      _isSpiesUtilitiesModuleLoaded(false),
      _didDebuggedProcessRunStart(false),
      _isDuringDebuggedProcessTermination(false),
      _terminatingDebuggedProcessThroughAPI(false),
      _debuggedProcessExecutionMode(AP_DEBUGGING_MODE),
      _hexDisplayMode(false),
      _openCLEnglineLoaded(false),
      _openGLEnglineLoaded(false),
      _kernelDebuggingEnteredAtLeastOnce(false),
      _kernelDebuggingEnable(true),
      m_multipleKernelDebugDispatchMode(AP_MULTIPLE_KERNEL_DISPATCH_WAIT)
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_PERSISTENT_DATA_MANAGER_EVENTS_HANDLING_PRIORITY);

    // Initialize the OpenGL stub with false:
    for (int i = 0; i < AP_OPENGL_AMOUNT_OF_FORCED_STUBS ; i++)
    {
        _isOpenGLStubForced[i] = false;
    }

    // Initialize the OpenCL execution modes with false:
    for (int i = 0; i < AP_OPENCL_AMOUNT_OF_EXECUTIONS; i++)
    {
        _isOpenCLExecutionOn[i] = true;
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        m_debugOutputSeverities[i] = false;
    }

    // Initialize the kernel debugging data:
    clearKernelDebuggingData();
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::~gaPersistentDataManager
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
gaPersistentDataManager::~gaPersistentDataManager()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // Clean up:
    removeAllBreakpoints();
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::wasOpenGLDataRecordedInDebugSession
// Description:
//              This function return true iff openGL data recording was
//              done during this debug session;
//
//              meaning either:
//              1. Recording is "on" at the moment - we are recording data.
//              2. During this debug session, recording was "on" for some
//                 time, then we disabled it. In this case we also return
//                 true, since for some time - recording was on.
//
// Return Val:  bool - True / False
// Author:      Eran Zinman
// Date:        29/1/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::wasOpenGLDataRecordedInDebugSession()
{
    bool retVal = (_wasOpenGLRecorderOn || _isHTMLLogFileRecordingOn);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::suspendDebuggedProcessExecution
// Description: Suspends the debugged process execution.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/10/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::suspendDebuggedProcessExecution()
{
    bool retVal = false;

    if (gaCanGetHostDebugging())
    {
        if (gaDebuggedProcessExists())
        {
            std::unique_lock<std::mutex> _lock(_mtxProcessCreating);

            retVal = pdProcessDebugger::instance().suspendHostDebuggedProcess();
        }
    }

    // If we cannot suspend via the debugger, suspend via the spy:
    if (!retVal)
    {
        retVal = true;

        _suspendDebuggedProcessExecution = true;

        // If the API is active - set it to break at the next monitored function call:
        if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
        {
            retVal = makeSpiesSuspendDebuggedProcessExecution();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::updateContextDataSnapshot
// Description: Updates the spy context data snapshot of a given context.
// Arguments:   contextId - The id of the context who's spy snapshot data will be updated.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/5/2005
// Implementation notes:
//   We use caching (_isContextDataSnapshotUpdated vec) to verify that we perform
//   the context data update only once in every debugged process suspension.
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::updateContextDataSnapshot(apContextID contextId)
{
    bool retVal = false;

    // If the _isContextDataSnapshotUpdated vector does not have an entry that represents
    // our context, add the queried context entry (and all its former contexts entries):
    verifyContextUpdateStatusAllocation(contextId);

    // If the queried context snapshot data is not updated in the current debugged process suspension:
    if (_isContextDataSnapshotUpdatedMap[contextId] == GA_CONTEXT_DATA_NOT_UPDATED)
    {
        bool rc = false;

        if (contextId.isOpenGLContext())
        {
            // Update the context data snapshot:
            rc = gaUpdateContextDataSnapshot(contextId._contextId);
        }
        else if (contextId.isOpenCLContext())
        {
            // Update the context data snapshot:
            rc = gaUpdateOpenCLContextDataSnapshot(contextId._contextId);
        }
        else if (contextId.isDefault())
        {
            GT_ASSERT_EX(false, L"implement me please");
            // TO_DO: default context
        }
        else
        {
            // Invalid context ID:
            GT_ASSERT(false);
        }

        if (rc)
        {
            // We mark the context data snapshot as updated:
            _isContextDataSnapshotUpdatedMap[contextId] = GA_CONTEXT_DATA_UPDATED;
        }
        else
        {
            // Mark that the context data update failed:
            _isContextDataSnapshotUpdatedMap[contextId] = GA_CONTEXT_DATA_UPDATE_FAILED;
        }
    }

    // Return true iff the context data was updated successfully:
    retVal = ((_isContextDataSnapshotUpdatedMap[contextId] == GA_CONTEXT_DATA_UPDATED) || (_isContextDataSnapshotUpdatedMap[contextId] == GA_CONTEXT_DATA_UPDATED_AND_DELETED));

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onModuleLoadedEvent
// Description: Is called when a module is loaded into the debugged process address space.
// Arguments: eve - Contains the loaded module data.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onModuleLoadedEvent(const apModuleLoadedEvent& eve)
{
    // If we got the loaded module path:
    gtString loadedModulePath = eve.modulePath();

    if (!loadedModulePath.isEmpty())
    {
        // If this is the Spies utilities module:
        loadedModulePath.toLowerCase();

        if (loadedModulePath.find(OS_SPY_UTILS_FILE_PREFIX) != -1)
        {
            // Mark that the spies utilities module was loaded:
            _isSpiesUtilitiesModuleLoaded = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessRunStartedEvent
// Description: Is called when the debugged process starts running.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessRunStartedEvent()
{
    // If the current debug session log files sub-directory was not created yet, create it:
    createCurrentDebugSessionLogFilesSubDirectory();

    // Mark that the process run started:
    _didDebuggedProcessRunStart = true;

    m_isInKernelDebuggingBreakpoint = false;

    // On Windows, when we catch this event, the debugged process main thread is suspended by the
    // process debugger and THIS CLASS IS RESPONSIBLE FOR RESUMING IT'S RUN !!
    // (See Implementation notes A above)
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // If the spy utilities module is loaded and the API connection was not established yet:
        if (_isSpiesUtilitiesModuleLoaded && !_isAPIConnectionEstablished)
        {
            // Do not resume the debugged process main thread run. It will be resumed by the
            // coming API_CONNECTION_ESTABLISHED event (see Implementation notes B above).
        }
        else
        {
            // Resume the debugged process main thread run:
            resumeDebuggedProcessMainThreadRun();
        }
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onAPIConnectionEstablishedEvent
// Description: Is called when an API connection is established.
// Arguments: eve - Contains the established API connection details.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onAPIConnectionEstablishedEvent(const apApiConnectionEstablishedEvent& eve)
{
    // Act according to the established connection type:
    apAPIConnectionType establishedConnectionType = eve.establishedConnectionType();

    switch (establishedConnectionType)
    {
        case AP_INCOMING_EVENTS_API_CONNECTION:
            // Nothing to be done.
            break;

        case AP_SPIES_UTILITIES_API_CONNECTION:
        {
            std::unique_lock<std::mutex> _lock(_mtxProcessCreating);

            // The Spies Utilities API connection was established:
            onSpiesUtilitiesAPIConnection();
        }
        break;

        case AP_OPENGL_API_CONNECTION:
        {
            std::unique_lock<std::mutex> _lock(_mtxProcessCreating);

            // OpenGL API connection was established:
            onOpenGLServerAPIConnection();
        }
        break;

        case AP_OPENCL_API_CONNECTION:
        {
            std::unique_lock<std::mutex> _lock(_mtxProcessCreating);

            // OpenCL API connection was established:
            onOpenCLServerAPIConnection();
        }
        break;

        case AP_HSA_API_CONNECTION:
        {
            std::unique_lock<std::mutex> _lock(_mtxProcessCreating);

            // HSA API connection was established:
            onHSAServerAPIConnection();
        }
        break;

        default:
            // Unknown connection Type!
            GT_ASSERT(false);
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessCreatedEvent
// Description: Is called when the debugged process is created.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessCreatedEvent()
{
    // Initialize relevant members:
    _isAPIConnectionEstablished = false;
    _isSpiesUtilitiesModuleLoaded = false;
    _didDebuggedProcessRunStart = false;
    _isDuringDebuggedProcessTermination = false;
    _terminatingDebuggedProcessThroughAPI = false;
    _deleteLogFilesWhenDebuggedProcessTerminates = !_isHTMLLogFileRecordingOn;
    _currentDebugSessionLogFilesSubDirPath.setFullPathFromString(L"");
    _isContextDataSnapshotUpdatedMap.clear();
    m_functionCallsCache.clear();
    m_openCLHandlesCache.clear();
    m_threadCurrentOpenGLContextCache.clear();
    m_openGLContextCurrentThreadCache.clear();
    clearKernelDebuggingData();

    // Mark all the contexts as not updated:
    clearIsContextDataSnapshotUpdatedVec();
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessTerminatedEvent
// Description: Is called when the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessTerminatedEvent()
{
    // Notify an update of the breakpoints objects:
    apBreakpointsUpdatedEvent event(-1);
    apEventsHandler::instance().registerPendingDebugEvent(event);

    // We don't want any temporary breakpoints to outlive the process and we want to
    // re-enable any temporarily disabled breakpoint for the next debug session.
    resolveAllTemporaryBreakpoints();

    // Delete the current debug session log files sub directory:
    deleteCurrentDebugSessionLogFilesSubDirectory();

    // Initialize relevant members:
    _isAPIConnectionEstablished = false;
    _isSpiesUtilitiesModuleLoaded = false;
    _didDebuggedProcessRunStart = false;
    _isDuringDebuggedProcessTermination = false;
    _terminatingDebuggedProcessThroughAPI = false;
    _currentDebugSessionLogFilesSubDirPath.setFullPathFromString(L"");
    _suspendDebuggedProcessExecution = false;
    _breakOnNextMonitoredFunctionCall = false;
    _breakOnNextDrawFunctionCall = false;
    _breakOnNextFrame = false;
    _breakInMonitoredFunctionCall = false;
    _isContextDataSnapshotUpdatedMap.clear();
    m_functionCallsCache.clear();
    m_openCLHandlesCache.clear();
    m_threadCurrentOpenGLContextCache.clear();
    m_openGLContextCurrentThreadCache.clear();
    clearKernelDebuggingData();
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessCreationFailureEvent
// Description: Is called when the debugged process creation failed.
// Author:      Sigal Algranaty
// Date:        11/2/2010
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessCreationFailureEvent()
{
    // Delete the current debug session log files sub directory:
    deleteCurrentDebugSessionLogFilesSubDirectory();

    // Initialize relevant members:
    _isAPIConnectionEstablished = false;
    _isSpiesUtilitiesModuleLoaded = false;
    _didDebuggedProcessRunStart = false;
    _isDuringDebuggedProcessTermination = false;
    _terminatingDebuggedProcessThroughAPI = false;
    _currentDebugSessionLogFilesSubDirPath.setFullPathFromString(L"");
    _suspendDebuggedProcessExecution = false;
    _breakOnNextMonitoredFunctionCall = false;
    _breakOnNextDrawFunctionCall = false;
    _breakOnNextFrame = false;
    _breakInMonitoredFunctionCall = false;
    _isContextDataSnapshotUpdatedMap.clear();
    m_functionCallsCache.clear();
    m_openCLHandlesCache.clear();
    m_threadCurrentOpenGLContextCache.clear();
    m_openGLContextCurrentThreadCache.clear();
    clearKernelDebuggingData();
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessRunResumedEvent
// Description: Is called when the debugged process run is resumed.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessRunResumedEvent()
{
    clearIsContextDataSnapshotUpdatedVec();

    // Clear the function call, CL object ID and GL context caches:
    m_functionCallsCache.clear();
    m_openCLHandlesCache.clear();
    m_threadCurrentOpenGLContextCache.clear();
    m_openGLContextCurrentThreadCache.clear();

    m_isInKernelDebuggingBreakpoint = false;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessRunStartedExternallyEvent
// Description: Is called when the debugged process by an external tool
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessRunStartedExternallyEvent(const apDebuggedProcessRunStartedExternallyEvent& eve)
{
    // Notify the API the process was created. This will cause a "real" process run started
    // event to appear, so we don't need to initialize members here:
    const apDebugProjectSettings& creationData = ((const apDebuggedProcessRunStartedExternallyEvent&)eve).getProcessCreationData();

    bool rcInit = pdProcessDebugger::instance().initializeDebugger(creationData);

    GT_IF_WITH_ASSERT(rcInit)
    {
        setProcessDebuggerPersistentDataValues();
    }

    gaLaunchDebuggedProcess(creationData, false);
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onProcessDuringTermination
// Description: Is called when the debugged process is during termination
// Arguments:   const apDebuggedProcessIsDuringTerminationEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onProcessDuringTermination(const apDebuggedProcessIsDuringTerminationEvent& eve)
{
    (void)(eve); // unused
    _isDuringDebuggedProcessTermination = true;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onContextUpdated
// Description: Is called when a context data snapshot was updated
// Arguments:   const apContextDataSnapshotWasUpdatedEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        20/7/2010
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onContextUpdated(const apContextDataSnapshotWasUpdatedEvent& eve)
{
    // If the _isContextDataSnapshotUpdated vector does not have an entry that represents
    // our context, add the queried context entry (and all its former contexts entries):
    verifyContextUpdateStatusAllocation(eve.updatedContextID());

    // Get the context update status:
    ContextDataUpdateStatus status = GA_CONTEXT_DATA_UPDATED;

    if (eve.isContextBeingDeleted())
    {
        status = GA_CONTEXT_DATA_UPDATED_AND_DELETED;
    }

    // If the queried context snapshot data is not updated in the current debugged process suspension:
    _isContextDataSnapshotUpdatedMap[eve.updatedContextID()] = status;

}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onExceptionEvent
// Description: Called when we get an exception event, and vetoes it if it is
//              the SIGKILL created by our gaTerminateDebuggedProcessImpl function
// Return Val: void
// Author:      Uri Shomroni
// Date:        11/1/2010
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onExceptionEvent(const apExceptionEvent& eve, bool& vetoEvent)
{
    osExceptionReason excepReason = eve.exceptionReason();

    if ((excepReason == OS_SIGKILL_SIGNAL) && (_terminatingDebuggedProcessThroughAPI))
    {
        // Don't let the application's other classes get this event, since we caused it:
        vetoEvent = true;

        // Resume the debugged process run. Note that we do not use gaResumeDebuggedProcess(),
        // since the Spies utilities API connection is already dead:
        pdProcessDebugger::instance().resumeDebuggedProcess();
    }
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onSpiesUtilitiesAPIConnection
// Description: Is called when an API connection with the spies utilities is established.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onSpiesUtilitiesAPIConnection()
{
    // If the current debug session log files sub-directory was not created yet, create it:
    createCurrentDebugSessionLogFilesSubDirectory();

    // Initialize the spies API:
    bool rc1 = initializeSpiesAPI();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Mark that the API connection was established:
        _isAPIConnectionEstablished = true;
    }

    // On Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // If the debugged process run started, the main thread is currently suspended, waiting for us
        // to initialize the API (see Implementation notes B above):
        if (_didDebuggedProcessRunStart)
        {
            // Resume the debugged process main thread run:
            resumeDebuggedProcessMainThreadRun();
        }
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onOpenGLServerAPIConnection
// Description: Is called when an API connection with the OpenGL Server is established.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onOpenGLServerAPIConnection()
{
    // Output a message to the debug log:
    OS_OUTPUT_DEBUG_LOG(GA_STR_GotOpenGLServerAPIConnection, OS_DEBUG_LOG_DEBUG);

    // Set the OpenGL Server persistent data values:
    bool rc1 = setOpenGLServerPersistentDataValues();
    GT_ASSERT(rc1);

    // Notify performance counters readers about the API connection established event:
    /*  pcPerformanceCountersManager& thePerformanceCountersManager = pcPerformanceCountersManager::instance();
    apApiConnectionEstablishedEvent connectionEstablishedEvent(AP_OPENGL_API_CONNECTION);
    thePerformanceCountersManager.onAPIConnectionEstablishedEvent(connectionEstablishedEvent);*/

    // Notify the OpenGL Server that the initialization sequence ended,
    // allowing it to continue running:
    osSocket& spiesAPISocket = gaSpiesAPISocket();
    spiesAPISocket << (gtInt32)GA_FID_gaOpenGLServerInitializationEnded;

    // Mark that our GL server was loaded:
    _openGLEnglineLoaded = true;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onOpenCLServerAPIConnection
// Description: Is called when the API connection with the OpenCL Server is established.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onOpenCLServerAPIConnection()
{
    // Output a message to the debug log:
    OS_OUTPUT_DEBUG_LOG(GA_STR_GotOpenCLServerAPIConnection, OS_DEBUG_LOG_DEBUG);

    // Set the OpenCL Server persistent data values:
    bool rc1 = setOpenCLServerPersistentDataValues();
    GT_ASSERT(rc1);

    // Notify performance counters readers about the API connection established event:
    /*  pcPerformanceCountersManager& thePerformanceCountersManager = pcPerformanceCountersManager::instance();
    apApiConnectionEstablishedEvent connectionEstablishedEvent(AP_OPENCL_API_CONNECTION);
    thePerformanceCountersManager.onAPIConnectionEstablishedEvent(connectionEstablishedEvent);*/

    // Notify the OpenGL Server that the initialization sequence ended,
    // allowing it to continue running:
    osSocket& spiesAPISocket = gaSpiesAPISocket();
    spiesAPISocket << (gtInt32)GA_FID_gaOpenCLServerInitializationEnded;


    // Mark that our GL server was loaded:
    _openCLEnglineLoaded = true;
}

void gaPersistentDataManager::onHSAServerAPIConnection()
{

}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::verifyContextUpdateStatusAllocation
// Description:
//  Verifies that _isContextDataSnapshotUpdated contains an entry
//  for the input context id. If it does not exist - allocates the entry.
//
// Author:      Yaki Tebeka
// Date:        28/1/2007
// ---------------------------------------------------------------------------
void gaPersistentDataManager::verifyContextUpdateStatusAllocation(apContextID contextId)
{
    gtMap<apContextID, ContextDataUpdateStatus>::const_iterator findIter = _isContextDataSnapshotUpdatedMap.find(contextId);

    if (findIter == _isContextDataSnapshotUpdatedMap.end())
    {
        _isContextDataSnapshotUpdatedMap[contextId] = GA_CONTEXT_DATA_NOT_UPDATED;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::clearIsContextDataSnapshotUpdatedVec
// Description: Clears the _isContextDataSnapshotUpdated vector data.
// Author:      Yaki Tebeka
// Date:        16/5/2005
// ---------------------------------------------------------------------------
void gaPersistentDataManager::clearIsContextDataSnapshotUpdatedVec()
{
    // Go through the contexts:
    gtMap<apContextID, ContextDataUpdateStatus>::iterator iter = _isContextDataSnapshotUpdatedMap.begin();
    gtMap<apContextID, ContextDataUpdateStatus>::const_iterator iterEnd = _isContextDataSnapshotUpdatedMap.end();

    for (; iter != iterEnd; iter++)
    {
        // Mark only the not deleted contexts, as not updated. Deleted context should not be updated:
        ContextDataUpdateStatus& contextStatus = (*iter).second;

        if ((GA_CONTEXT_DATA_UPDATED_AND_DELETED != contextStatus) && (GA_CONTEXT_DATA_DELETED != contextStatus))
        {
            contextStatus = GA_CONTEXT_DATA_NOT_UPDATED;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::removeBreakpoint
// Description: Removes a debugged process breakpoint.
// Arguments:   breakpoint - The breakpoint to be set.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/6/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::removeBreakpoint(int breakpointIndex)
{
    bool retVal = false;

    int breakpointsAmount =  amountOfBreakpoints();
    GT_IF_WITH_ASSERT((breakpointIndex <= breakpointsAmount) && (breakpointIndex >= 0))
    {
        // Get the breakpoint from the breakpoints list:
        apBreakPoint* pBreakpoint = _activeBreakpoints[breakpointIndex];

        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            // We found the breakpoint we want to remove:
            retVal = true;

            if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == pBreakpoint->type())
            {
                apHostSourceCodeBreakpoint& breakpoint = (apHostSourceCodeBreakpoint&) * pBreakpoint;
                retVal = pdProcessDebugger::instance().deleteHostSourceBreakpoint(breakpoint.filePath(), breakpoint.lineNumber());
            }
            else
            {
                // If the API is active - set the breakpoint also at the spy side:
                if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
                {
                    // Remove the breakpoint from spy:
                    retVal = removeSpyBreakpoint(*pBreakpoint);
                }
            }

            // If we succeeded:
            if (retVal)
            {
                // Remove the breakpoint copy from the _activeBreakpoints vector:
                for (int i = breakpointIndex; i <= (breakpointsAmount - 2); i++)
                {
                    _activeBreakpoints[i] = _activeBreakpoints[i + 1];
                }

                // Pop the breakpoint on top:
                _activeBreakpoints.pop_back();

                // Delete the breakpoint structure:
                delete pBreakpoint;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::removeGenericBreakpoint
// Description: Removes a generic breakpoint
// Arguments:   breakpointType - The breakpoint to remove
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/7/2011
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::removeGenericBreakpoint(apGenericBreakpointType breakpointType)
{
    bool retVal = true;

    // Get the current breakpoint amount:
    int breakpointsAmount =  amountOfBreakpoints();
    int breakpointIndex = -1;

    for (int i = 0 ; i < breakpointsAmount; i++)
    {
        // Get the current breakpoint:
        apBreakPoint* pBreakpoint = _activeBreakpoints[i];
        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            // If this breakpoint is generic:
            if (pBreakpoint->type() == OS_TOBJ_ID_GENERIC_BREAKPOINT)
            {
                // Down cast the breakpoint to a generic breakpoint:
                apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)pBreakpoint;
                GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
                {
                    // If this is the breakpoint, set the id and break the loop:
                    if (pGenericBreakpoint->breakpointType() == breakpointType)
                    {
                        breakpointIndex = i;
                        break;
                    }
                }
            }
        }
    }

    // If the breakpoint exists:
    if (breakpointIndex >= 0)
    {
        retVal = removeBreakpoint(breakpointIndex);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakpointHitCount
// Description: Sets a requested breakpoint hit count
// Arguments:   int breakpointIndex
//              int hitCount
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakpointHitCount(int breakpointIndex, int hitCount)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((breakpointIndex >= 0) && (breakpointIndex < (int)_activeBreakpoints.size()))
    {
        // Get the requested breakpoint:
        apBreakPoint* pBreakpoint = _activeBreakpoints[breakpointIndex];
        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            // Set the breakpoint hit count:
            pBreakpoint->setHitCount(hitCount);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setKernelSourceCodeBreakpointProgramHandle
// Description: Sets a kernel breakpoint program handle
// Arguments:   int index
//              oaCLProgramHandle programHandle
//              bool isResolved
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setKernelSourceCodeBreakpointProgramHandle(int breakpointIndex, oaCLProgramHandle programHandle)
{
    bool retVal = false;

    // Get the real pointer for the breakpoint:
    // breakPointId range test:
    int breakpointsAmount = amountOfBreakpoints();

    if ((0 <= breakpointIndex) && (breakpointIndex < breakpointsAmount))
    {
        // Clone the input breakpoint:
        apBreakPoint* pBreakPoint = _activeBreakpoints[breakpointIndex];
        GT_IF_WITH_ASSERT(pBreakPoint->type() == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
        {
            apKernelSourceCodeBreakpoint* pSourceCodeBreakpoint = (apKernelSourceCodeBreakpoint*)pBreakPoint;
            pSourceCodeBreakpoint->setProgramHandle(programHandle);

            // Set breakpoint is done successfully:
            // If the API is active - set the breakpoint also at the spy side:
            if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
            {
                // Set the spy breakpoint:
                retVal = setSpyBreakpoint(*pSourceCodeBreakpoint);
            }

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakpoint
// Description: Sets a debugged process breakpoint.
// Arguments:   breakpoint - The breakpoint to be set.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = true;

    bool breakpointExist = false;

    // Check if the breakpoint already exist:
    // Get breakpoints amount:
    int breakpointsAmount = amountOfBreakpoints();

    // Iterate the breakpoints, and compare the function id(for monitored functions breakpoints):
    for (int i = 0; i < breakpointsAmount; i++)
    {
        // Get the current breakpoint pointer:
        apBreakPoint* pCurrentBreakpoint = _activeBreakpoints[i];

        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            // Check if this breakpoint is the same as the requested breakpoint:
            if (breakpoint.compareToOther(*pCurrentBreakpoint))
            {
                retVal = setBreakpointState(i, breakpoint.state());
                breakpointExist = true;
                break;
            }
        }
    }

    if (!breakpointExist)
    {
        // Clone the input breakpoint:
        apBreakPoint* pBreakPointCopy = (apBreakPoint*)(breakpoint.clone());
        GT_IF_WITH_ASSERT(pBreakPointCopy != NULL)
        {
            // Insert the breakpoint copy into the _activeBreakpoints vector:
            _activeBreakpoints.push_back(pBreakPointCopy);

            if (breakpoint.isEnabled())
            {
                if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == breakpoint.type())
                {
                    if (gaCanGetHostDebugging())
                    {
                        const apHostSourceCodeBreakpoint& hostSourceBreakpoint = (const apHostSourceCodeBreakpoint&)breakpoint;
                        retVal = gaSetHostBreakpoint(hostSourceBreakpoint.filePath(), hostSourceBreakpoint.lineNumber());
                    }
                }
                else
                {
                    // If the API is active - set the breakpoint also at the spy side:
                    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
                    {
                        // Set the spy breakpoint:
                        retVal = setSpyBreakpoint(breakpoint);
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakpointState
// Description: Update the state of an existing debugged process breakpoint at a
//              specified location in the vector.
// Arguments:   breakpoint - The breakpoint to be set.
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakpointState(int breakpointId, const apBreakPoint::State state)
{
    bool retVal = true;

    // Check if the breakpoint already exist:
    // Get breakpoints amount:
    int breakpointsAmount = amountOfBreakpoints();

    GT_IF_WITH_ASSERT(breakpointId >= 0 && breakpointId < breakpointsAmount)
    {

        // Get the current breakpoint pointer:
        apBreakPoint* pCurrentBreakpoint = _activeBreakpoints[breakpointId];

        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            bool wasPreviouslyEnabled = pCurrentBreakpoint->isEnabled();

            // Set the state of the breakpoint:
            pCurrentBreakpoint->setState(state);
            bool isCurrentlyEnabled = pCurrentBreakpoint->isEnabled();

            // Set breakpoint is done successfully.
            // If breakpoint enabling changed then we need to update the spy as well
            if (wasPreviouslyEnabled != isCurrentlyEnabled)
            {
                // If the API is active - set the breakpoint also at the spy side:
                if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == pCurrentBreakpoint->type())
                {
                    apHostSourceCodeBreakpoint* pHostBP = (apHostSourceCodeBreakpoint*)pCurrentBreakpoint;

                    if (isCurrentlyEnabled)
                    {
                        retVal = gaSetHostBreakpoint(pHostBP->filePath(), pHostBP->lineNumber());
                    }
                    else
                    {
                        retVal = pdProcessDebugger::instance().deleteHostSourceBreakpoint(pHostBP->filePath(), pHostBP->lineNumber());
                    }
                }
                else
                {
                    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
                    {
                        if (isCurrentlyEnabled)
                        {
                            // Set the spy breakpoint:
                            retVal = setSpyBreakpoint(*pCurrentBreakpoint);
                        }
                        else // Breakpoint state is disabled or temporarily disabled
                        {
                            // Remove the breakpoint from spy:
                            retVal = removeSpyBreakpoint(*pCurrentBreakpoint);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::amountOfBreakpoints
// Description: Returns the amount of debugged process breakpoints.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
int gaPersistentDataManager::amountOfBreakpoints() const
{
    return _activeBreakpoints.size();
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getBreakpoint
// Description: Returns a debugged process breakpoint data.
// Arguments:   breakPointId - The id of the queried breakpoint.
//              breakpoint - Output variable - the breakpoint data.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getBreakpoint(int breakPointId, gtAutoPtr<apBreakPoint>& aptrBreakpoint) const
{
    bool retVal = false;

    // breakPointId range test:
    int breakpointsAmount = amountOfBreakpoints();

    if ((0 <= breakPointId) && (breakPointId < breakpointsAmount))
    {
        // Clone the input breakpoint:
        apBreakPoint* pBreakPointCopy = (apBreakPoint*)(_activeBreakpoints[breakPointId]->clone());
        GT_IF_WITH_ASSERT(pBreakPointCopy != NULL)
        {
            aptrBreakpoint = pBreakPointCopy;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getBreakpointIndex
// Description: Tries to find the index of an active breakpoint that matches
//              the supplied breakpoint in type and parameters.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/10/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getBreakpointIndex(const apBreakPoint& breakpoint, int& breakpointId) const
{
    bool retVal = false;

    // breakPointId range test:
    int breakpointsAmount = amountOfBreakpoints();
    breakpointId = -1;

    for (int i = 0; i < breakpointsAmount; i++)
    {
        // Clone the input breakpoint:
        const apBreakPoint* pCurrentBreakpoint = _activeBreakpoints[i];
        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            // If the breakpoint is the same:
            if (breakpoint.compareToOther(*pCurrentBreakpoint))
            {
                retVal = true;
                breakpointId = i;
                break;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::removeAllBreakpoints
// Description: Removes all the debugged process breakpoints.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::removeAllBreakpoints()
{
    bool retVal = true;

    // Remove all the registered debugged process breakpoints:
    int breakpointsAmount = amountOfBreakpoints();

    for (int i = 0; i < breakpointsAmount; i++)
    {
        delete _activeBreakpoints[i];
    }

    _activeBreakpoints.clear();

    // If the API is active - remove the breakpoints also on its side:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = removeAllSpyBreakpoints();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::removeAllBreakpointsByState
// Description: Removes all the debugged process breakpoints whose state
//              equals the parameter state.
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::removeAllBreakpointsByState(const apBreakPoint::State state)
{
    bool retVal = true;
    int breakpointsAmount = amountOfBreakpoints();

    for (int i = 0; i < breakpointsAmount; i++)
    {
        // Get the current breakpoint:
        apBreakPoint* pBreakpoint = _activeBreakpoints[i];
        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            // If the state of this breakpoint matches the parameter:
            if (pBreakpoint->state() == state)
            {
                retVal &= removeBreakpoint(i);

                if (retVal)
                {

                    // Removing the breakpoint pulled the remaining elements of the collection
                    // one index lower, so we need to revisit the i-th element
                    i--;

                    // The total number of elements was reduced by one
                    breakpointsAmount--;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::temporarilyDisableAllBreakpoints
// Description: Change the state of all the debugged process enabled breakpoints
//              to 'temporarily disabled' state
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::temporarilyDisableAllBreakpoints()
{
    bool retVal = true;
    int breakpointsAmount = amountOfBreakpoints();

    for (int i = 0; i < breakpointsAmount; i++)
    {
        // Get the current breakpoint:
        apBreakPoint* pBreakpoint = _activeBreakpoints[i];
        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            if (pBreakpoint->isEnabled())
            {
                // Set the breakpoint state
                retVal = setBreakpointState(i, apBreakPoint::BREAKPOINT_STATE_TEMPORARILY_DISABLED) && retVal;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::enableAllBreakpointsByState
// Description: Change the state of any debugged process breakpoint whose
//              state equals the parameter state to 'Enabled'
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::enableAllBreakpointsByState(const apBreakPoint::State state)
{
    bool retVal = true;
    int breakpointsAmount = amountOfBreakpoints();

    for (int i = 0; i < breakpointsAmount; i++)
    {
        // Get the current breakpoint:
        apBreakPoint* pBreakpoint = _activeBreakpoints[i];
        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            // If the state of this breakpoint matches the parameter:
            if (pBreakpoint->state() == state)
            {
                retVal = setBreakpointState(i, apBreakPoint::BREAKPOINT_STATE_ENABLED) && retVal;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::resolveAllTemporaryBreakpoints
// Description: Enable all temporarily disabled breakpoints and remove all temporary breakpoints
//              This is used in the following scenarios:
//              1. 'Run to Cursor' - we reached the breakpoint (either at the cursor or another breakpoint was hit first)
//                  so remove the temporary breakpoint at the cursor location
//              2. 'Skip breakpoints and step into kernel' - we reached the breakpoint meaning we have stepped into the
//                  kernel, so re-enable all the temporarily disabled breakpoints
//              3. Process is terminated - we don't want any temporary breakpoints to outlive the process
//                  and we want to re-enable any temporarily disabled breakpoint for the next debug session.
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Jan-29, 2015
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::resolveAllTemporaryBreakpoints()
{
    bool retVal = true;
    int breakpointsAmount = amountOfBreakpoints();

    for (int i = breakpointsAmount - 1; i  >= 0; --i)
    {
        // Get the current breakpoint:
        apBreakPoint* pBreakpoint = _activeBreakpoints[i];
        GT_IF_WITH_ASSERT(pBreakpoint != NULL)
        {
            apBreakPoint::State state = pBreakpoint->state();

            // If the state of this breakpoint matches the parameter:
            if (state == apBreakPoint::BREAKPOINT_STATE_TEMPORARILY_DISABLED)
            {
                retVal = setBreakpointState(i, apBreakPoint::BREAKPOINT_STATE_ENABLED) && retVal;
            }
            else if (state == apBreakPoint::BREAKPOINT_STATE_TEMPORARY)
            {
                removeBreakpoint(i);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakPointOnNextMonitoredFunctionCall
// Description:
//   Caused the debugged application to trigger a breakpoint event on the
//   next call a monitored function.
//   This function enables implementing "One step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakPointOnNextMonitoredFunctionCall()
{
    bool retVal = true;

    _breakOnNextMonitoredFunctionCall = true;

    // If the API is active - set it to break at the next monitored function call:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyBreakPointOnNextMonitoredFunctionCall();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakPointOnNextDrawFunctionCall
// Description:
//   Caused the debugged application to trigger a breakpoint event on the
//   next draw function.
//   This function enables implementing "Draw step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        25/5/2006
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakPointOnNextDrawFunctionCall()
{
    bool retVal = true;

    _breakOnNextDrawFunctionCall = true;

    // If the API is active - set it to break at the next monitored function call:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyBreakPointOnNextDrawFunctionCall();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakPointOnNextFrame
// Description:
//   Caused the debugged application to trigger a breakpoint event on the
//   next frame terminator.
//   This function enables implementing "Draw step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/7/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakPointOnNextFrame()
{
    bool retVal = true;

    _breakOnNextFrame = true;

    // If the API is active - set it to break at the next monitored function call:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyBreakPointOnNextFrame();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setBreakPointInMonitoredFunctionCall
// Description: Steps into the monitored function we are currently suspended on.
//              If this function does not support this, sets a breakpoint on the
//              next function call (like gaBreakOnNextMonitoredFunctionCall).
//              This function should be followed by a call to gaResumeDebuggedProcess().
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setBreakPointInMonitoredFunctionCall()
{
    bool retVal = true;

    // Do not allow enqueueing two kernels for debug:
    if (_kernelDebuggingEnable && !(_isInKernelDebugging || _waitingForKernelDebuggingToStart))
    {
        _breakInMonitoredFunctionCall = true;

        // If the API is active - set it to break at the next monitored function call:
        if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
        {
            retVal = setSpyBreakPointInMonitoredFunctionCall();
        }
    }
    else
    {
        retVal = setBreakPointOnNextMonitoredFunctionCall();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::clearAllStepFlags
// Description: Clears the step flags set by setBreak*
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/2/2016
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::clearAllStepFlags()
{
    bool retVal = true;

    bool wasAnyFlagSet = (_suspendDebuggedProcessExecution || _breakOnNextMonitoredFunctionCall || _breakOnNextDrawFunctionCall || _breakOnNextFrame || _breakInMonitoredFunctionCall);

    // Clear the local flags (if the API connection is not active yet):
    _suspendDebuggedProcessExecution = false;
    _breakOnNextMonitoredFunctionCall = false;
    _breakOnNextDrawFunctionCall = false;
    _breakOnNextFrame = false;
    _breakInMonitoredFunctionCall = false;

    // If the API is active - clear the flags there as well:
    if (wasAnyFlagSet && gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = clearAllSpyStepFlags();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getCachedFunctionCall
// Description: Tries to get a function call from the cache. Fails if the
//              call was not cached since the last break.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        3/10/2013
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getCachedFunctionCall(const apContextID& contextID, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall) const
{
    bool retVal = false;

    // Create the call full index:
    FunctionCallIndex callFullIndex;
    callFullIndex.m_context = contextID;
    callFullIndex.m_indexInContext = callIndex;

    // Try and find the call in our cache:
    gtMap<FunctionCallIndex, gtAutoPtr<apFunctionCall> >::const_iterator findIter = m_functionCallsCache.find(callFullIndex);
    gtMap<FunctionCallIndex, gtAutoPtr<apFunctionCall> >::const_iterator endIter = m_functionCallsCache.end();

    if (findIter != endIter)
    {
        // Sanity check:
        const gtAutoPtr<apFunctionCall>& raptrCachedFunctionCall = findIter->second;
        GT_IF_WITH_ASSERT(NULL != raptrCachedFunctionCall.pointedObject())
        {
            // Clone the function call:
            osTransferableObject* pCallClone = raptrCachedFunctionCall->clone();
            GT_IF_WITH_ASSERT(NULL != pCallClone)
            {
                // Output it into the auto pointer:
                retVal = true;
                aptrFunctionCall = (apFunctionCall*)pCallClone;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::cacheFunctionCall
// Description: Adds a function call to the function call cache
// Author:      Uri Shomroni
// Date:        3/10/2013
// ---------------------------------------------------------------------------
void gaPersistentDataManager::cacheFunctionCall(const apContextID& contextID, int callIndex, const gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(NULL != aptrFunctionCall.pointedObject())
    {
        // Create the call full index:
        FunctionCallIndex callFullIndex;
        callFullIndex.m_context = contextID;
        callFullIndex.m_indexInContext = callIndex;

        // We do not expect the same function to be cached twice, this method should only be called from
        // gaGetCurrentFrameFunctionCall itself, and only after cache resolution failed:
        gtMap<FunctionCallIndex, gtAutoPtr<apFunctionCall> >::const_iterator findIter = m_functionCallsCache.find(callFullIndex);
        gtMap<FunctionCallIndex, gtAutoPtr<apFunctionCall> >::const_iterator endIter = m_functionCallsCache.end();

        GT_IF_WITH_ASSERT(findIter == endIter)
        {
            // Clone this function call into the cache, to avoid distrupting the auto pointer:
            gtAutoPtr<apFunctionCall> aptrFunctionCallClone = (apFunctionCall*)aptrFunctionCall->clone();
            m_functionCallsCache[callFullIndex] = aptrFunctionCallClone;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getCacheOpenCLObjectID
// Description: Tries to get an OpenCL object ID from the cache. Fails if the
//              object was not cached since the last break.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getCacheOpenCLObjectID(oaCLHandle hOpenCLObject, apCLObjectID& o_objectId) const
{
    bool retVal = false;

    // Try and find the object in our cache:
    gtMap<oaCLHandle, apCLObjectID>::const_iterator findIter = m_openCLHandlesCache.find(hOpenCLObject);
    gtMap<oaCLHandle, apCLObjectID>::const_iterator endIter = m_openCLHandlesCache.end();

    if (findIter != endIter)
    {
        // Get the cached ID:
        const apCLObjectID& cachedObjectID = findIter->second;

        // Output it into the output parameter:
        retVal = true;
        o_objectId = cachedObjectID;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::cacheFunctionCall
// Description: Adds a function call to the function call cache
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
void gaPersistentDataManager::cacheOpenCLObjectID(oaCLHandle hOpenCLObject, const apCLObjectID& objectId)
{
    // We do not expect the same object to be cached twice, this method should only be called from
    // gaGetOpenCLHandleObjectDetails itself, and only after cache resolution failed:
    gtMap<oaCLHandle, apCLObjectID>::const_iterator findIter = m_openCLHandlesCache.find(hOpenCLObject);
    gtMap<oaCLHandle, apCLObjectID>::const_iterator endIter = m_openCLHandlesCache.end();

    GT_IF_WITH_ASSERT(findIter == endIter)
    {
        // Set the id into the cache:
        m_openCLHandlesCache[hOpenCLObject] = objectId;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getThreadCurrentOpenGLContextFromCache
// Description: Tries to get the cached thread -> OpenGL context correlation. Fails if the
//              context was not cached since the last break.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getThreadCurrentOpenGLContextFromCache(osThreadId threadId, int& o_contextId) const
{
    bool retVal = false;

    // Try and find the context in our cache:
    gtMap<osThreadId, int>::const_iterator findIter = m_threadCurrentOpenGLContextCache.find(threadId);
    gtMap<osThreadId, int>::const_iterator endIter = m_threadCurrentOpenGLContextCache.end();

    if (findIter != endIter)
    {
        // Get the cached ID:
        const int& cachedContextID = findIter->second;

        // Output it into the output parameter:
        retVal = true;
        o_contextId = cachedContextID;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getOpenGLContextCurrentThreadFromCache
// Description: Tries to get the cached OpenGL context -> thread correlation. Fails if the
//              thread was not cached since the last break.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getOpenGLContextCurrentThreadFromCache(int contextId, osThreadId& o_threadId) const
{
    bool retVal = false;

    // Try and find the thread in our cache:
    gtMap<int, osThreadId>::const_iterator findIter = m_openGLContextCurrentThreadCache.find(contextId);
    gtMap<int, osThreadId>::const_iterator endIter = m_openGLContextCurrentThreadCache.end();

    if (findIter != endIter)
    {
        // Get the cached ID:
        const osThreadId& cachedThreadId = findIter->second;

        // Output it into the output parameter:
        retVal = true;
        o_threadId = cachedThreadId;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::cacheThreadToOpenGLContextCorrelation
// Description: Adds a correlation of OpenGL context <-> thread id.
//              Does not cache values when the key is invalid (thread 0 or context 0 / -1)
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
void gaPersistentDataManager::cacheThreadToOpenGLContextCorrelation(osThreadId threadId, int contextId)
{
    // Do not validate this caching, as it may come from either side:
    if (OS_NO_THREAD_ID != threadId)
    {
        m_threadCurrentOpenGLContextCache[threadId] = contextId;
    }

    if (0 < contextId)
    {
        m_openGLContextCurrentThreadCache[contextId] = threadId;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setHTMLLogFileRecordingMode
// Description: Sets the HTML log file recording mode.
// Arguments:   isHTMLLogFileRecordingOn - true - start / resume HTML log file recording.
//                                         false - suspend HTML log file recording.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/8/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setHTMLLogFileRecordingMode(bool isHTMLLogFileRecordingOn)
{
    bool retVal = true;

    _isHTMLLogFileRecordingOn = isHTMLLogFileRecordingOn;

    // If the API is active - set it the text log file recording mode:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyHTMLLogFileRecordingMode(isHTMLLogFileRecordingOn);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSlowMotionDelay
// Description: Sets the "Slow motion" delay.
// Arguments:   delayTimeUnits - The slow motion delay in abstract time units.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSlowMotionDelay(int delayTimeUnits)
{
    bool retVal = true;

    _slowMotionDelayTimeUnits = delayTimeUnits;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpySlowMotionDelay(delayTimeUnits);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::forceOpenGLFlush
// Description: Sets the "Force OpenGL flush" mode.
// Arguments:   isOpenGLFlushForced - true iff "Force OpenGL flush" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::forceOpenGLFlush(bool isOpenGLFlushForced)
{
    bool retVal = true;

    _isOpenGLFlushForced = isOpenGLFlushForced;

    // If the API is active - set the "Force OpenGL flush" mode:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = setSpyForceOpenGLFlush(isOpenGLFlushForced);
    }

    return retVal;
}
//----------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setDebuggedProcessExecutionMode
// Description: Sets the debugged process execution mode (debug / profiling / redundant state)
// Arguments: apExecutionMode executionMode
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setDebuggedProcessExecutionMode(apExecutionMode executionMode)
{
    bool retVal = true;

    _debuggedProcessExecutionMode = executionMode;

    // If the API is active - set the "Interactive break" mode:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyDebuggedProcessExecutionMode(executionMode);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setInteractiveBreakMode
// Description: Sets the "Interactive break" mode.
// Arguments: isInteractiveBreakOn - true to turn on "Interactive break" mode.
//                                   false to rurn off "Interactive break" mode.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setInteractiveBreakMode(bool isInteractiveBreakOn)
{
    bool retVal = true;

    _isInteractiveBreakOn = isInteractiveBreakOn;

    // If the API is active - set the "Interactive break" mode:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = setSpyInteractiveBreakMode(isInteractiveBreakOn);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::flushLogFileAfterEachFunctionCall
// Description: Sets the "flushLogFileAfterEachFunctionCall implementation" mode.
// Arguments:   flushAfterEachFunctionCall - true iff "flushLogFileAfterEachFunctionCall implementation" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::flushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall)
{
    bool retVal = true;

    _isLogFileFlushedAfterEachFunctionCall = flushAfterEachFunctionCall;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyFlushLogFileAfterEachFunctionCall(flushAfterEachFunctionCall);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::forceNullOpenGLDriver
// Description: Sets the "NULL OpenGL implementation" mode.
// Arguments:   isNULLOpenGLImplOn - true iff "the NULL OpenGL implementation" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::forceOpenGLNullDriver(bool isNULLOpenGLImplOn)
{
    bool retVal = true;

    _isOpenGLNullDriverForced = isNULLOpenGLImplOn;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = setSpyForceNullOpenGLDriver(isNULLOpenGLImplOn);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::enableImagesDataLogging
// Description: Sets the "enable Images Data Logging" mode.
// Arguments:   isImagesDataLoggingEnabled - true iff "Logging Enabled" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        15/3/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::enableImagesDataLogging(bool isImagesDataLoggingEnabled)
{
    bool retVal = true;

    _isImagesDataLoggingEnabled = isImagesDataLoggingEnabled;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyEnableImagesDataLoging(isImagesDataLoggingEnabled);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setAllocatedObjectsCreationCallsStacksCollection
// Description: Sets the "Collect allocated objects creation calls stacks" mode
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setAllocatedObjectsCreationCallsStacksCollection(bool collectCreationStacks)
{
    bool retVal = true;

    _areAllocatedObjectsCreationCallsStacksCollected = collectCreationStacks;

    // If the API is active, set the value to it:
    if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        retVal = setSpyCollectAllocatedObjectsCreationCallsStacks(collectCreationStacks);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::forceOpenGLStub
// Description: Forces an OpenGL stub
// Arguments:   apOpenGLForcedModeType openGLStubType - the stub type (textures / geometry / etc')
//              bool isStubForced - is the stub mode forced
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::forceOpenGLStub(apOpenGLForcedModeType openGLStubType, bool isStubForced)
{
    bool retVal = true;

    // Set the value within the persistent data manager:
    _isOpenGLStubForced[openGLStubType] = isStubForced;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = setSpyOpenGLForceStub(openGLStubType, isStubForced);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setOpenCLOperationExecution
// Description: Sets an OpenCL operation execution
// Arguments:   apOpenCLExecutionType openCLExecutionType
//            bool isExecutionOn
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/5/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setOpenCLOperationExecution(apOpenCLExecutionType openCLExecutionType, bool isExecutionOn)
{
    bool retVal = true;

    // Set the value within the persistent data manager:
    _isOpenCLExecutionOn[openCLExecutionType] = isExecutionOn;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        retVal = setSpyOpenCLOperationExecution(openCLExecutionType, isExecutionOn);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::kernelDebuggingGlobalWorkOffset
// Description: Get the current work offset for the nth coordinate.
//              If the work dimension is too low to include it, returns 0.
//              If no kernel is currently debugged, returns -1.
// Author:      Uri Shomroni
// Date:        6/3/2011
// ---------------------------------------------------------------------------
int gaPersistentDataManager::kernelDebuggingGlobalWorkOffset(int coordinate) const
{
    int retVal = -1;

    // If the index is valid:
    if (isInKernelDebugging() && (coordinate > -1) && (coordinate < 3))
    {
        // Return the value:
        retVal = _kernelDebuggingGlobalWorkOffset[coordinate];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::kernelDebuggingGlobalWorkSize
// Description: Get the current work size for the nth coordinate.
//              If the work dimension is too low to include it, returns 0.
//              If no kernel is currently debugged, returns -1.
// Author:      Uri Shomroni
// Date:        4/1/2011
// ---------------------------------------------------------------------------
int gaPersistentDataManager::kernelDebuggingGlobalWorkSize(int coordinate) const
{
    int retVal = -1;

    // If the index is valid:
    if (isInKernelDebugging() && (coordinate > -1) && (coordinate < 3))
    {
        // Return the value:
        retVal = _kernelDebuggingGlobalWorkSize[coordinate];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::kernelDebuggingLocalWorkSize
// Description: Get the current work group size for the nth coordinate.
//              If the work dimension is too low to include it, returns 0.
//              If no kernel is currently debugged, returns -1.
// Author:      Uri Shomroni
// Date:        6/3/2011
// ---------------------------------------------------------------------------
int gaPersistentDataManager::kernelDebuggingLocalWorkSize(int coordinate) const
{
    int retVal = -1;

    // If the index is valid:
    if (isInKernelDebugging() && (coordinate > -1) && (coordinate < 3))
    {
        // Return the value:
        retVal = _kernelDebuggingLocalWorkSize[coordinate];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setKernelDebuggingCurrentWorkItemCoordinate
// Description: Sets the position in the coordinate-th dimension of the current work
//              item to value. e.g. to set the work item to (13, 89, 5), this function
//              should be called with (0,13), (1,89) and (2,5).
//              Will fail if the value is out of range or if the index is out of range.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/1/2011
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int value)
{
    bool retVal = false;

    // If the coordinate index is valid:
    if (isInKernelDebugging() && (coordinate > -1) && (coordinate < 3))
    {
        // Make sure that the value is within the coordinate size:
        bool isValid = (value >= _kernelDebuggingGlobalWorkOffset[coordinate]);
        isValid = isValid && (value < (_kernelDebuggingGlobalWorkOffset[coordinate] + _kernelDebuggingGlobalWorkSize[coordinate]));

        if (isValid)
        {
            // Set the value:
            _kernelDebuggingCurrentWorkItem[coordinate] = value;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setKernelDebuggingEnable
// Description: Sets the kernel debugging enable flag to the spy and locally
// Arguments:   bool kernelDebugging
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setKernelDebuggingEnable(bool kernelDebugging)
{
    bool retVal = true;

    // Set kernel debugging mode
    _kernelDebuggingEnable = kernelDebugging;

    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        retVal = setSpyKernelDebuggingEnable();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyKernelDebuggingEnable
// Description: Sets the spy kernel debugging enable flag
// Arguments:   bool kernelDebugging
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool  gaPersistentDataManager::setSpyKernelDebuggingEnable()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetKernelDebuggingEnable;

    // Send the arguments:
    spyConnectionSocket << _kernelDebuggingEnable;

    // Receive success value:
    spyConnectionSocket >> retVal;

    // If this function failed, kernel debugging is not available:
    if (!retVal)
    {
        _kernelDebuggingEnable = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setKernelDebuggingEnable
// Description: Sets the kernel debugging multiple dispatch handling mode
// Arguments:   apMultipleKernelDebuggingDispatchMode mode
// Author:      Uri Shomroni
// Date:        2/7/2015
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode)
{
    bool retVal = true;

    // Set kernel debugging mode
    m_multipleKernelDebugDispatchMode = mode;

    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        retVal = setSpyMultipleKernelDebugDispatchMode();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyMultipleKernelDebugDispatchMode
// Description: Sets the spy kernel debugging multiple dispatch handling mode
// Author:      Uri Shomroni
// Date:        2/7/2015
// ---------------------------------------------------------------------------
bool  gaPersistentDataManager::setSpyMultipleKernelDebugDispatchMode()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetMultipleKernelDebugDispatchMode;

    // Send the arguments:
    spyConnectionSocket << (gtInt32)m_multipleKernelDebugDispatchMode;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::kernelDebuggingCurrentWorkItemCoordinate
// Description: Gets the value of the coordinate-th dimension of the current work item
//              position (0 = x, 1 = y, 2 = z).
// Author:      Uri Shomroni
// Date:        4/1/2011
// ---------------------------------------------------------------------------
int gaPersistentDataManager::kernelDebuggingCurrentWorkItemCoordinate(int coordinate) const
{
    int retVal = -1;

    // If the coordinate index is valid:
    if (isInKernelDebugging() && (coordinate > -1) && (coordinate < 3))
    {
        // Return the value:
        retVal = _kernelDebuggingCurrentWorkItem[coordinate];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::enableGLDebugOutputLogging
// Description: Tells CodeXL if the the debug output integration is enabled.
// Arguments:   enableGLDebugOutputIntegration - true / false - is integration enabled
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::enableGLDebugOutputLogging(bool enableGLDebugOutputIntegration)
{
    bool retVal = true;

    _GLDebugOutputLoggingEnabled = enableGLDebugOutputIntegration;

    // If the API is active - set it the slow motion delay:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = setSpyGLDebugOutputLoggingStatus(_GLDebugOutputLoggingEnabled);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getGLDebugOutputSeverityEnabled
// Description: Returns the enabled status of the selected severity
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity) const
{
    bool retVal = false;

    // Sanity check:
    if ((0 <= severity) && (AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES > severity))
    {
        retVal = m_debugOutputSeverities[severity];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setGLDebugOutputKindMask
// Description: Sets the debug output category mask
// Arguments:   unsigned long mask
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setGLDebugOutputKindMask(const gtUInt64& mask)
{
    bool retVal = true;

    m_debugOutputKindsMask = mask;

    // If the API is active - set the debug output category mask:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = setSpyGLDebugOutputKindMask(mask);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyGLDebugOutputKindMask
// Description: Set the OpenGL debug output category mask
// Arguments:   bool breakOnGLDebugOutputMessages
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyGLDebugOutputKindMask(const gtUInt64& debugOutputCategoryMask)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetGLDebugOutputKindMask;

    // Send the arguments:
    spyConnectionSocket << debugOutputCategoryMask;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setGLDebugOutputSeverityEnabled
// Description: Sets the debug output severity
// Arguments:   apGLDebugOutputSeverity severity
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/6/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled)
{
    bool retVal = false;

    // Sanity check:
    if ((0 <= severity) && (AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES > severity))
    {
        retVal = true;
        m_debugOutputSeverities[severity] = enabled;

        // If the API is active - set the debug output category mask:
        if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
        {
            retVal = setSpyGLDebugOutputSeverityEnabled(severity, enabled);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyGLDebugOutputSeverityEnabled
// Description: Set the OpenGL debug output severity
// Arguments:   apGLDebugOutputSeverity severity
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/6/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetGLDebugOutputSeverityEnabled;

    // Send the arguments:
    spyConnectionSocket << severity;
    spyConnectionSocket << enabled;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::forceOpenGLPolygonRasterMode
// Description: Force OpenGL polygon raster mode to an input raster mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::forceOpenGLPolygonRasterMode(apRasterMode rasterMode)
{
    bool retVal = true;

    _isOpenGLStubForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] = true;
    _forcedOpenGLRasterMode = rasterMode;

    // If the API is active - set the spy "Force polygon mode" status:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = forceSpyOpenGLPolygonRasterMode(rasterMode);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::cancelOpenGLPolygonRasterModeForcing
// Description: Cancels the "Force OpenGL raster" mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::cancelOpenGLPolygonRasterModeForcing()
{
    bool retVal = true;

    _isOpenGLStubForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] = false;

    // If the API is active - set the spy "Force polygon mode" status:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        retVal = cancelSpyOpenGLPolygonRasterMode();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   eve - A class representing the debugged process event.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// Implementation notes:
//  On Windows:
//  a. When "process run started" event is thrown, the debugged process main thread is suspended by the
//     process debugger and THIS CLASS IS RESPONSIBLE FOR RESUMING ITS RUN !!
//
//  b. In some cases, OpenGL32.dll is loaded before the main thread starts running, but "API connection established"
//     event occurs AFTER the "process run started" event is thrown (The main thread and the Apy API thread runs concurrently).
//     This is bad, because we want the API initialization to occur BEFORE the process starts running.
//     To resolve the problem, if OpenGL32.dll was loaded, in "process run started" we DO NOT resume the main thread
//     run. Instead, we resume it in "API connection established".
// ---------------------------------------------------------------------------
void gaPersistentDataManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Act according to the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_BREAKPOINT_HIT:
        {
            // clear the context update vector when break point hit to gather the data correctly again:
            clearIsContextDataSnapshotUpdatedVec();

            // When a breakpoint is hit there are 2 scenarios:
            // 1. 'Run to Cursor' - we reached the breakpoint (either at the cursor or another breakpoint was hit first)
            //     so remove the temporary breakpoint at the cursor location
            // 2. 'Skip breakpoints and step into kernel' - we reached the breakpoint meaning we have stepped into the
            //    kernel, so re-enable all the temporarily disabled breakpoints
            resolveAllTemporaryBreakpoints();

            // Also clear the step flags, so that if we performed a host step or hit a host BP, API-level steps would be cleared:
            clearAllStepFlags();

            // Clear the function call, CL object ID and OpenGL context caches:
            m_functionCallsCache.clear();
            m_openCLHandlesCache.clear();
            m_threadCurrentOpenGLContextCache.clear();
            m_openGLContextCurrentThreadCache.clear();

            // During kernel debugging, we need to update which thread is the current debugging thread:
            if (_isInKernelDebugging)
            {
                // We don't expect this to change during kernel debugging (unless more than one thread is debugging
                // at the same time):
                osThreadId suspensionThreadId = eve.triggeringThreadId();

                // Do copy the value, to be safe:
                m_isInKernelDebuggingBreakpoint = (_kernelDebuggingThreadId == suspensionThreadId);

                if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
                {
                    gtString logMsg;
                    logMsg.appendFormattedString(L"Breakpoint in kernel debugging from thread %llu (KD thread %llu) identified as KD breakpoint ? %c", (gtUInt64)suspensionThreadId, (gtUInt64)_kernelDebuggingThreadId, m_isInKernelDebuggingBreakpoint ? 'Y' : 'N');
                    OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                }


                // TODO: Uri, 2/9/12 - patchy workaround to allow kernel debugging in the meantime on Linux
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                m_isInKernelDebuggingBreakpoint = true;
                _kernelDebuggingThreadId = suspensionThreadId;
#endif
            }
        }
        break;

        case apEvent::AP_MODULE_LOADED:
            onModuleLoadedEvent((const apModuleLoadedEvent&)eve);
            break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
            onProcessRunStartedEvent();
            break;

        case apEvent::AP_API_CONNECTION_ESTABLISHED:
            onAPIConnectionEstablishedEvent((const apApiConnectionEstablishedEvent&)eve);
            break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
            onProcessCreatedEvent();
            break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            onProcessTerminatedEvent();
            _isInKernelDebugging = false;
            m_isInKernelDebuggingBreakpoint = false;
            _kernelDebuggingThreadId = OS_NO_THREAD_ID;
            _waitingForKernelDebuggingToStart = false;
            _kernelDebuggingInterruptedWarningIssued = false;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
            onProcessCreationFailureEvent();
            break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
            onProcessRunResumedEvent();
            break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED_EXTERNALLY:
            onProcessRunStartedExternallyEvent((const apDebuggedProcessRunStartedExternallyEvent&)eve);
            break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // If we got a suspension while waiting for kernel debugging to start:
            if (_waitingForKernelDebuggingToStart && !_kernelDebuggingInterruptedWarningIssued)
            {
                // Send a "kernel debugging interrupted" event:
                apKernelDebuggingInterruptedEvent kernelDebuggingInterruptedEve;
                apEventsHandler::instance().registerPendingDebugEvent(kernelDebuggingInterruptedEve);
                _kernelDebuggingInterruptedWarningIssued = true;
            }

            // If this is a kernel debugging breakpoint:
            if (m_isInKernelDebuggingBreakpoint)
            {
                // Verify the current work item is valid:
                if (!gaIsWorkItemValid(_kernelDebuggingCurrentWorkItem))
                {
                    // If it isn't, select the first item that is valid, instead:
                    int firstValidItem[3] = { -1, -1, -1};
                    bool rcCoord = gaGetFirstValidWorkItem(-1, firstValidItem);
                    GT_IF_WITH_ASSERT(rcCoord)
                    {
                        bool rcSet = setKernelDebuggingCurrentWorkItemCoordinate(0, firstValidItem[0]);
                        rcSet = ((_kernelDebuggingGlobalWorkSize[1] < 1) || setKernelDebuggingCurrentWorkItemCoordinate(1, firstValidItem[1])) && rcSet;
                        rcSet = ((_kernelDebuggingGlobalWorkSize[2] < 1) || setKernelDebuggingCurrentWorkItemCoordinate(2, firstValidItem[2])) && rcSet;
                        GT_ASSERT(rcSet);
                    }
                }
            }
        }
        break;


        case apEvent::AP_DEBUGGED_PROCESS_IS_DURING_TERMINATION:
            onProcessDuringTermination((const apDebuggedProcessIsDuringTerminationEvent&)eve);
            break;

        case apEvent::AP_CONTEXT_UPDATED_EVENT:
            onContextUpdated((const apContextDataSnapshotWasUpdatedEvent&)eve);
            break;

        case apEvent::AP_EXCEPTION:
            onExceptionEvent((const apExceptionEvent&)eve, vetoEvent);
            break;

        case apEvent::AP_COMPUTE_CONTEXT_DELETED_EVENT:
        {
            const apComputeContextDeletedEvent& computeContextDeletedEvent = (const apComputeContextDeletedEvent&)eve;
            apContextID deletedContextID(AP_OPENCL_CONTEXT, computeContextDeletedEvent.contextId());
            markContextAsDeleted(deletedContextID);
        }
        break;

        case apEvent::AP_RENDER_CONTEXT_DELETED_EVENT:
        {
            const apRenderContextDeletedEvent& renderContextDeletedEvent = (const apRenderContextDeletedEvent&)eve;
            apContextID deletedContextID(AP_OPENGL_CONTEXT, renderContextDeletedEvent.contextId());
            markContextAsDeleted(deletedContextID);
        }
        break;

        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        {
            const apBeforeKernelDebuggingEvent& beforeKernelDebuggingEvent = (const apBeforeKernelDebuggingEvent&)eve;
            _kernelDebuggingGlobalWorkOffset[0] = (int)beforeKernelDebuggingEvent.globalWorkOffsetX();
            _kernelDebuggingGlobalWorkOffset[1] = (int)beforeKernelDebuggingEvent.globalWorkOffsetY();
            _kernelDebuggingGlobalWorkOffset[2] = (int)beforeKernelDebuggingEvent.globalWorkOffsetZ();
            _kernelDebuggingGlobalWorkSize[0] = (int)beforeKernelDebuggingEvent.globalWorkSizeX();
            _kernelDebuggingGlobalWorkSize[1] = (int)beforeKernelDebuggingEvent.globalWorkSizeY();
            _kernelDebuggingGlobalWorkSize[2] = (int)beforeKernelDebuggingEvent.globalWorkSizeZ();
            _kernelDebuggingLocalWorkSize[0] = (int)beforeKernelDebuggingEvent.localWorkSizeX();
            _kernelDebuggingLocalWorkSize[1] = (int)beforeKernelDebuggingEvent.localWorkSizeY();
            _kernelDebuggingLocalWorkSize[2] = (int)beforeKernelDebuggingEvent.localWorkSizeZ();
            _kernelDebuggingCurrentWorkItem[0] = (int)beforeKernelDebuggingEvent.globalWorkOffsetX();
            _kernelDebuggingCurrentWorkItem[1] = (int)beforeKernelDebuggingEvent.globalWorkOffsetY();
            _kernelDebuggingCurrentWorkItem[2] = (int)beforeKernelDebuggingEvent.globalWorkOffsetZ();
            _isInKernelDebugging = true;
            _kernelDebuggingEnteredAtLeastOnce = true;
            _kernelDebuggingThreadId = beforeKernelDebuggingEvent.triggeringThreadId();
            _waitingForKernelDebuggingToStart = false;
            _kernelDebuggingInterruptedWarningIssued = false;
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        case apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT:
        {
            clearKernelDebuggingData();
            _isInKernelDebugging = false;
            m_isInKernelDebuggingBreakpoint = false;
            _kernelDebuggingThreadId = OS_NO_THREAD_ID;
            _waitingForKernelDebuggingToStart = false;
            _kernelDebuggingInterruptedWarningIssued = false;
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT:
        {
            const apOpenCLProgramCreatedEvent& programCreatedEvent = (const apOpenCLProgramCreatedEvent&)eve;

            // Localize the path as needed:
            osFilePath programSourcePath = programCreatedEvent.programSourceFilePath();
            gaRemoteToLocalFile(programSourcePath, true);

            // Bind all the source code breakpoints that are related to this program:
            bindSourceCodeBreakpoints(programSourcePath);
        }
        break;

        default:
            // Any other event is ignored.
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::initializeSpiesAPI
// Description:
//   Initialize the Spies side of the API.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        4/10/2004
// Implementation notes:
// Usually, the initialization is done by talking with the debugged application's main thread
// (an API initialization loop run under the DLLMain(DLL_PROCESS_ATTACH) function).
//   This enables initializing the Spies before the debugged process starts running.
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::initializeSpiesAPI()
{
    bool retVal = false;

    // We need to wait patiently until the Spy Utilities module, running within the debugged process, will
    // be able to talk with us:
    osSocket& spiesAPISocket = gaSpiesAPISocket();
    long storedReadTimeout = spiesAPISocket.readOperationTimeOut();
    spiesAPISocket.setReadOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);

    // Create the spies events forwarding socket:
    bool rcEvents = createSpiesEventsForwardingSocket();
    GT_IF_WITH_ASSERT(rcEvents)
    {
        // Send the API init data:
        bool rcInitData = sendSpiesAPIInitData(spiesAPISocket);
        GT_IF_WITH_ASSERT(rcInitData)
        {
            // Get the spies API thread id:
            bool rcAPIThread = getSpiesAPIThreadId();
            GT_IF_WITH_ASSERT(rcAPIThread)
            {
                // Apply the persistent data values on the Spies Utilities side:
                bool setPData = setSpiesUtilitiesPersistentDataValues();
                GT_ASSERT(setPData);

                retVal = true;
            }
        }
    }

    // Notify the API that the initialization sequence ended:
    spiesAPISocket << (gtInt32)GA_FID_gaIntializeAPIEnded;

    // Restore the original API socket read timeout:
    spiesAPISocket.setReadOperationTimeOut(storedReadTimeout);

    if (!retVal)
    {
        // Output an error line into the debug log file:
        OS_OUTPUT_DEBUG_LOG(GA_STR_FailedToInitOGLSpyAPI, OS_DEBUG_LOG_ERROR);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::createSpiesEventsForwardingSocket
// Description: Creates the spies events forwarding socket.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        22/12/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::createSpiesEventsForwardingSocket()
{
    bool retVal = false;

    // Get the process creation data:
    const apDebugProjectSettings* pProcessCreationData = pdProcessDebugger::instance().debuggedProcessCreationData();
    GT_IF_WITH_ASSERT(pProcessCreationData != NULL)
    {
        if (pProcessCreationData->isRemoteTarget())
        {
            // This is a remote target, create the TCP/IP event forwarding connection:
            osPortAddress spyEventsPort(pProcessCreationData->remoteConnectionSpiesEventsPort(), false);
            retVal = gaCreateEventForwardingTCPConnection(spyEventsPort);
        }
        else // !pProcessCreationData->isRemoteTarget()
        {
            // If this is an iPhone device connection, we do not get debug events from a
            // debugger, so we need to get them from the spy:
            const gtString& eventsPipeName = pProcessCreationData->spiesEventsPipeName();
            retVal = gaCreateEventForwardingPipeConnection(eventsPipeName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setProcessDebuggerPersistentDataValues
// Description: Sets the process debugger related persistent data values.
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        25/1/2016
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setProcessDebuggerPersistentDataValues()
{
    bool retVal = true;

    // Host debugging parameters:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

    if (theProcessDebugger.canPerformHostDebugging())
    {
        // If we have any host breakpoints:
        bool rcHostBP = true;

        for (const apBreakPoint* pBreakpoint : _activeBreakpoints)
        {
            if (nullptr != pBreakpoint)
            {
                osTransferableObjectType bpType = pBreakpoint->type();

                switch (bpType)
                {
                    case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
                    {
                        const apHostSourceCodeBreakpoint& hostSourceBreakpoint = (const apHostSourceCodeBreakpoint&)(*pBreakpoint);

                        if (hostSourceBreakpoint.isEnabled())
                        {
                            bool rcSrc = theProcessDebugger.setHostSourceBreakpoint(hostSourceBreakpoint.filePath(), hostSourceBreakpoint.lineNumber());
                            GT_ASSERT(rcSrc);
                            rcHostBP = rcHostBP && rcSrc;
                        }
                    }
                    break;

                    case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
                        // Unresolved source breakpoints should not exist if the process debugger supports host debugging:
                        GT_ASSERT(false);
                        break;

                    // API debugging breakpoint types:
                    case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
                    case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
                    case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
                    case OS_TOBJ_ID_GENERIC_BREAKPOINT:
                        // Do nothing
                        break;

                    default:
                        // Unexpected object type!
                        GT_ASSERT(false);
                        break;
                }
            }
        }

        retVal = retVal && rcHostBP;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpiesUtilitiesPersistentDataValues
// Description: Sets the spies utilities related persistent data values.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/12/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpiesUtilitiesPersistentDataValues()
{
    bool retVal = true;

    // Set the Spies utilities module execution mode:
    bool rcExecMode = setSpyDebuggedProcessExecutionMode(_debuggedProcessExecutionMode);
    GT_ASSERT(rcExecMode);
    retVal = retVal && rcExecMode;

    // Set all the registered breakpoints into the Spies utilities module:
    int breakpointsAmount = amountOfBreakpoints();

    for (int i = 0; i < breakpointsAmount; i++)
    {
        apBreakPoint* pCurrentBreakpoint = _activeBreakpoints[i];

        if (pCurrentBreakpoint != NULL)
        {
            if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT != pCurrentBreakpoint->type())
            {
                bool rcBreakPoint = setSpyBreakpoint(*pCurrentBreakpoint);
                GT_ASSERT(rcBreakPoint);
                retVal = retVal && rcBreakPoint;
            }
        }
    }

    // Set the Spy "break on next monitored function call" state:
    if (_breakOnNextMonitoredFunctionCall)
    {
        bool rcBreakNextCall = setSpyBreakPointOnNextMonitoredFunctionCall();
        GT_ASSERT(rcBreakNextCall);
        retVal = retVal && rcBreakNextCall;
    }

    // If we need to suspend the debugged process run:
    if (_suspendDebuggedProcessExecution)
    {
        bool rcSuspend = makeSpiesSuspendDebuggedProcessExecution();
        GT_ASSERT(rcSuspend);
        retVal = retVal && rcSuspend;
    }

    // Set the "Collect Allocated Objects Creation Calls Stacks" mode:
    bool rc13 = setSpyCollectAllocatedObjectsCreationCallsStacks(_areAllocatedObjectsCreationCallsStacksCollected);
    GT_ASSERT(rc13);
    retVal = retVal && rc13;

    // Set the "Slow motion" delay:
    bool rc14 = setSpySlowMotionDelay(_slowMotionDelayTimeUnits);
    GT_ASSERT(rc14);
    retVal = retVal && rc14;

    // Set the text log file recording mode:
    bool rc15 = setSpyHTMLLogFileRecordingMode(_isHTMLLogFileRecordingOn);
    GT_ASSERT(rc15);
    retVal = retVal && rc15;

    // Set the "Enable Textures Image Data Logging" mode:
    bool rc16 = setSpyEnableImagesDataLoging(_isImagesDataLoggingEnabled);
    GT_ASSERT(rc16);
    retVal = retVal && rc16;

    // Set the float parameters display precision:
    bool rc17 = setSpyFloatParametersDisplayPrecision();
    GT_ASSERT(rc17);
    retVal = retVal && rc17;

    // Set the "Flush log file after every function call" status:
    bool rc18 = setSpyFlushLogFileAfterEachFunctionCall(_isLogFileFlushedAfterEachFunctionCall);
    GT_ASSERT(rc18);
    retVal = retVal && rc18;

    // Set the Spy "break on next draw function call" state:
    if (_breakOnNextDrawFunctionCall)
    {
        bool rc20 = setSpyBreakPointOnNextDrawFunctionCall();
        GT_ASSERT(rc20);
        retVal = retVal && rc20;
    }

    // Set the Spy "break on next frame terminator" state:
    if (_breakOnNextFrame)
    {
        bool rc21 = setSpyBreakPointOnNextFrame();
        GT_ASSERT(rc21);
        retVal = retVal && rc21;
    }

    if (_breakInMonitoredFunctionCall)
    {
        bool rc22 = setSpyBreakPointInMonitoredFunctionCall();
        GT_ASSERT(rc22);
        retVal = retVal && rc22;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setOpenGLServerPersistentDataValues
// Description: Sets the values of the persistent data (that this class holds)
//              on the spy side.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/10/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setOpenGLServerPersistentDataValues()
{
    bool retVal = true;

    // Set the "Force OpenGL flush" mode:
    bool rc23 = setSpyForceOpenGLFlush(_isOpenGLFlushForced);
    GT_ASSERT(rc23);
    retVal = retVal && rc23;

    // Set the spy "Interactive break" mode status:
    bool rc24 = setSpyInteractiveBreakMode(_isInteractiveBreakOn);
    GT_ASSERT(rc24);
    retVal = retVal && rc24;

    bool rc25 = false;

    if (_isOpenGLStubForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE])
    {
        rc25 = forceSpyOpenGLPolygonRasterMode(_forcedOpenGLRasterMode);
        GT_ASSERT(rc25);
    }
    else
    {
        rc25 = cancelSpyOpenGLPolygonRasterMode();
        GT_ASSERT(rc25);
    }

    retVal = retVal && rc25;

    // Set the "Force Null OpenGL Driver" mode:
    bool rc26 = setSpyForceNullOpenGLDriver(_isOpenGLNullDriverForced);
    GT_ASSERT(rc26);
    retVal = retVal && rc26;

    // Set each of OpenGL the stub modes:
    for (int i = 0; i < AP_OPENGL_AMOUNT_OF_FORCED_STUBS; i++)
    {
        bool rc27 = setSpyOpenGLForceStub((apOpenGLForcedModeType)i, _isOpenGLStubForced[i]);
        GT_ASSERT(rc27);
        retVal = retVal && rc27;
    }

    bool rc28 = setSpyGLDebugOutputLoggingStatus(_GLDebugOutputLoggingEnabled);
    GT_ASSERT(rc28);
    retVal = retVal && rc28;

    bool rc29 = setSpyGLDebugOutputKindMask(m_debugOutputKindsMask);
    GT_ASSERT(rc29);
    retVal = retVal && rc29;

    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; s++)
    {
        bool rc30 = setSpyGLDebugOutputSeverityEnabled((apGLDebugOutputSeverity)s, m_debugOutputSeverities[s]);
        GT_ASSERT(rc30);
        retVal = retVal && rc30;
    }

    // Sanity test:
    GT_ASSERT_EX(retVal, GA_STR_FailedToSetSpyPersistantData);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setOpenCLServerPersistentDataValues
// Description: Sets the values of the persistent data (that this class holds)
//              on the spy side.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setOpenCLServerPersistentDataValues()
{
    bool retVal = true;

    // Set each of OpenCL execution modes:
    for (int i = 0; i < AP_OPENCL_AMOUNT_OF_EXECUTIONS; i++)
    {
        bool rc1 = setSpyOpenCLOperationExecution((apOpenCLExecutionType)i, _isOpenCLExecutionOn[i]);
        GT_ASSERT(rc1);
        retVal = retVal && rc1;
    }

    // Set the kernel source code paths:
    bool rc2 = setSpyKernelSourceCodePath();
    GT_ASSERT(rc2);
    retVal = retVal && rc2;

    // Set the kernel debugging mode enable flag
    bool rc3 = setSpyKernelDebuggingEnable();
    GT_ASSERT(rc3);
    retVal = retVal && rc3;

    // Set the multiple kernel dispatch mode:
    bool rc4 = setSpyMultipleKernelDebugDispatchMode();
    GT_ASSERT(rc4);
    retVal = retVal && rc4;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::removeSpyBreakpoint
// Description: Removes a breakpoint from the OpenGL32.dll spy.
// Arguments: int funcId - the breakpoint to be removed
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/6/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::removeSpyBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = true;

    if (breakpoint.type() != OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT)
    {
        retVal = false;

        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaRemoveBreakpoint;

        // Send the breakpoint:
        spyConnectionSocket << (osTransferableObject&)breakpoint;

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyBreakpoint
// Description: Sets a breakpoint in the OpenGL32.dll spy.
// Arguments:   breakpoint - The breakpoint to be set.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = true;

    osTransferableObjectType bpType = breakpoint.type();

    if (OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT != bpType)
    {
        bool isHSA = false;

        if ((breakpoint.isEnabled()) && (OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT == bpType) && gaIsAPIConnectionActive(AP_HSA_API_CONNECTION))
        {
            const apKernelSourceCodeBreakpoint& kernelSourceBP = (const apKernelSourceCodeBreakpoint&)breakpoint;

            if (kernelSourceBP.isHSAILBreakpoint())
            {
                isHSA = gaHSASetBreakpoint(kernelSourceBP.hsailKernelName(), (gtUInt64)((unsigned int)kernelSourceBP.lineNumber()));

                retVal = isHSA ;
            }
        }

        // If this was not an HSA breakpoint, handle it normally:
        if (!isHSA)
        {
            retVal = false;

            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaSetBreakpoint;

            // Send the breakpoint:
            spyConnectionSocket << (osTransferableObject&)breakpoint;

            // Receive success value:
            spyConnectionSocket >> retVal;
        }
    }
    else
    {
        // Source code breakpoints are not set to spy:
        retVal = true;

        // Resolve the breakpoint if it is new:
        const apSourceCodeBreakpoint& sourceCodeBreakpoint = (const apSourceCodeBreakpoint&)breakpoint;
        bindSourceCodeBreakpoints(sourceCodeBreakpoint.filePath());
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::removeAllSpyBreakpoints
// Description: Removes all the OpenGL32.dll spy breakpoints.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::removeAllSpyBreakpoints()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaRemoveAllBreakpoints;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::makeSpiesSuspendDebuggedProcessExecution
// Description: Makes the spy suspend the debugged process execution.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/10/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::makeSpiesSuspendDebuggedProcessExecution()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSuspendDebuggedProcess;

    // Receive success value:
    spyConnectionSocket >> retVal;

    if (retVal)
    {
        _suspendDebuggedProcessExecution = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getSpiesAPIThreadId
// Description: Receives the OpenGL server API thread id and notifies
//              the process debugger about it.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        4/1/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::getSpiesAPIThreadId()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& OGLSpyConnectionSocket = gaSpiesAPISocket();

    // Get the OpenGL server API thread id:
    OGLSpyConnectionSocket << (gtInt32)GA_FID_gaGetAPIThreadId;

    gtUInt64 spyAPIThreadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    OGLSpyConnectionSocket >> spyAPIThreadIdAsUInt64;
    osThreadId spyAPIThreadId = (osThreadId)spyAPIThreadIdAsUInt64;

    GT_IF_WITH_ASSERT(spyAPIThreadId != OS_NO_THREAD_ID)
    {
        // Set process debugger Spy API thread:
        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
        theProcessDebugger.setSpiesAPIThreadId(spyAPIThreadId);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::sendSpiesAPIInitData
// Description: Sends the API initialization data to the Spy side.
// Return Val: bool  - Success / failure.
// Arguments:  osSocket& spyConnectionSocket - the socket for the Spy
// Author:      Yaki Tebeka
// Date:        4/1/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::sendSpiesAPIInitData(osSocket& spyConnectionSocket)
{
    bool retVal = false;

    // Send the function id:
    spyConnectionSocket << (gtInt32)GA_FID_gaIntializeAPI;

    // Send the API initialization data:
    spyConnectionSocket << _apiInitData;

    // Get a notification that the API at the spy side was initialized ok:
    bool rc1 = false;
    spyConnectionSocket >> rc1;
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyBreakPointOnNextMonitoredFunctionCall
// Description: Sets the spy to trigger a breakpoint at the next call to a monitored
//              function.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/6/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyBreakPointOnNextMonitoredFunctionCall()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaBreakOnNextMonitoredFunctionCall;

    // Receive success value:
    spyConnectionSocket >> retVal;

    if (retVal)
    {
        _breakOnNextMonitoredFunctionCall = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyBreakPointOnNextDrawFunctionCall
// Description: Sets the spy to trigger a breakpoint at the next draw function.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        25/5/2006
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyBreakPointOnNextDrawFunctionCall()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaBreakOnNextDrawFunctionCall;

    // Receive success value:
    spyConnectionSocket >> retVal;

    if (retVal)
    {
        _breakOnNextDrawFunctionCall = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyBreakPointOnNextFrame
// Description: Sets the spy to trigger a breakpoint at the next frame terminator.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/7/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyBreakPointOnNextFrame()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaBreakOnNextFrame;

    // Receive success value:
    spyConnectionSocket >> retVal;

    if (retVal)
    {
        _breakOnNextFrame = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyBreakPointInMonitoredFunctionCall
// Description: Sets the spy to trigger a breakpoint inside the current function call
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyBreakPointInMonitoredFunctionCall()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaBreakInMonitoredFunctionCall;

    // Receive success value:
    spyConnectionSocket >> retVal;

    if (retVal)
    {
        // Get whether we replaced the step in with a step over:
        bool replacedWithStepOver = false;
        spyConnectionSocket >> replacedWithStepOver;

        // If we actually performed a step in, we're waiting for kernel debugging to start:
        if (!replacedWithStepOver)
        {
            _waitingForKernelDebuggingToStart = true;
            _kernelDebuggingInterruptedWarningIssued = false;
        }

        // Clear the flag anyway:
        _breakInMonitoredFunctionCall = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::clearAllSpyStepFlags
// Description: Clears all flags set by setSpyBreak*
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/2/2016
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::clearAllSpyStepFlags()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaClearAllStepFlags;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyHTMLLogFileRecordingMode
// Description: Sets the spy HTML log file recording mode.
// Arguments:   isHTMLLogFileRecordingOn - true - start / resume HTML log file recording.
//                                         false - suspend HTML log file recording.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/8/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyHTMLLogFileRecordingMode(bool isHTMLLogFileRecordingOn)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    if (isHTMLLogFileRecordingOn)
    {
        spyConnectionSocket << (gtInt32)GA_FID_gaStartMonitoredFunctionsCallsLogFileRecording;
    }
    else
    {
        spyConnectionSocket << (gtInt32)GA_FID_gaStopMonitoredFunctionsCallsLogFileRecording;
    }

    // Receive success value:
    spyConnectionSocket >> retVal;

    // Reset the "was opengl recording on" variable
    _wasOpenGLRecorderOn = false;

    // If recording was set to "on"
    if (isHTMLLogFileRecordingOn)
    {
        // Make sure a debug process actually exist
        bool isDebuggedProcessExists = gaDebuggedProcessExists();

        if (isDebuggedProcessExists)
        {
            // Flag that openGL recording was done
            _wasOpenGLRecorderOn = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpySlowMotionDelay
// Description: Set the spy "Slow motion" delay.
// Arguments:   delayTimeUnits - The slow motion delay.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpySlowMotionDelay(int delayTimeUnits)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetSlowMotionDelay;

    // Send the arguments:
    spyConnectionSocket << (gtInt32)delayTimeUnits;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyForceOpenGLFlush
// Description: Sets the Spy "Force OpenGL flush" mode.
// Arguments:   isOpenGLFlushForced - true iff "Force OpenGL flush" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyForceOpenGLFlush(bool isOpenGLFlushForced)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaForceOpenGLFlush;

    // Send the arguments:
    spyConnectionSocket << isOpenGLFlushForced;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyProcessExecutionMode
// Description: Sets the Spy debugged process execution mode.
// Arguments:   executionMode - debug(default)/profiling/redundant state change.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyDebuggedProcessExecutionMode(apExecutionMode executionMode)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetDebuggedProcessExecutionMode;

    // Send the arguments:
    spyConnectionSocket << (gtInt32)executionMode;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyInteractiveBreakMode
// Description: Sets the Spy "Interactive break" mode.
// Arguments:   isInteractiveBreakOn - true iff "Interactive break" mode is on.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyInteractiveBreakMode(bool isInteractiveBreakOn)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetInteractiveBreakMode;

    // Send the arguments:
    spyConnectionSocket << isInteractiveBreakOn;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyFlushLogFileAfterEachFunctionCall
// Description: enable Flush to log file after every OpenGL func call.
// Arguments:   flushAfterEachFunctionCall - true iff "Flush after every OpenGL func call" mode is enabled.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyFlushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaFlushAfterEachMonitoredFunctionCall;

    // Send the arguments:
    spyConnectionSocket << flushAfterEachFunctionCall;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyEnableImagesDataLoging
// Description: enable OpenGL Loging - move from debugging to profiling mode.
// Arguments:   isTexturesImageDataLoggingEnabled - true iff "OpenGL logging" mode is enabled.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        15/3/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyEnableImagesDataLoging(bool isImagesDataLoggingEnabled)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaEnableImagesDataLogging;

    // Send the arguments:
    spyConnectionSocket << isImagesDataLoggingEnabled;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyCollectAllocatedObjectsCreationCallsStacks
// Description: Enables / disables the collection of allocated objects creation calls
//              stacks in the spy
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyCollectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaCollectAllocatedObjectsCreationCallsStacks;

    // Send the arguments:
    spyConnectionSocket << collectCreationStacks;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyForceNullOpenGLDriver
// Description: Sets the Spy "null OpenGL Driver" mode.
// Arguments:   isNULLOpenGLImplOn - true iff "null OpenGL Driver" mode is on.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyForceNullOpenGLDriver(bool isNULLOpenGLImplOn)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetOpenGLNullDriver;

    // Send the arguments:
    spyConnectionSocket << isNULLOpenGLImplOn;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyOpenGLForceStub
// Description: Sets the Spy Stub mode.
// Arguments:   openGLStubType - the stub type.
//              isStubForced - true iff Stub mode is on.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyOpenGLForceStub(apOpenGLForcedModeType openGLStubType, bool isStubForced)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetOpenGLForceStub;

    // Set the stub type:
    spyConnectionSocket << (gtInt32)openGLStubType;

    // Send the arguments:
    spyConnectionSocket << isStubForced;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyOpenCLOperationExecution
// Description: Sets spy OpenCL operation execution mode
// Arguments:   apOpenCLExecutionType openCLExecutionType
//            bool isExecutionOn
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/5/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyOpenCLOperationExecution(apOpenCLExecutionType openCLExecutionType, bool isExecutionOn)
{
    bool retVal = false;
    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetOpenCLOperationExecution;

    // Set the stub type:
    spyConnectionSocket << (gtInt32)openCLExecutionType;

    // Send the arguments:
    spyConnectionSocket << isExecutionOn;

    // Receive success value:
    spyConnectionSocket >> retVal;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::clearKernelDebuggingData
// Description: Resets the kernel debugging data to its status as is when no kernel
//              is debugged
// Author:      Uri Shomroni
// Date:        4/1/2011
// ---------------------------------------------------------------------------
void gaPersistentDataManager::clearKernelDebuggingData()
{
    _kernelDebuggingGlobalWorkOffset[0] = -1;
    _kernelDebuggingGlobalWorkOffset[1] = -1;
    _kernelDebuggingGlobalWorkOffset[2] = -1;
    _kernelDebuggingGlobalWorkSize[0] = -1;
    _kernelDebuggingGlobalWorkSize[1] = -1;
    _kernelDebuggingGlobalWorkSize[2] = -1;
    _kernelDebuggingLocalWorkSize[0] = -1;
    _kernelDebuggingLocalWorkSize[1] = -1;
    _kernelDebuggingLocalWorkSize[2] = -1;
    _kernelDebuggingCurrentWorkItem[0] = -1;
    _kernelDebuggingCurrentWorkItem[1] = -1;
    _kernelDebuggingCurrentWorkItem[2] = -1;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyFloatParametersDisplayPrecision
// Description: Sets the spy float parameters display precision according to
//              the value returned from apGetFloatParamsDisplayPrecision().
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/9/2007
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyFloatParametersDisplayPrecision()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetFloatParametersDisplayPrecision;

    // Send the arguments:
    gtUInt32 maxSignificatDigitsAmount = (gtUInt32)apGetFloatParamsDisplayPrecision();
    spyConnectionSocket << maxSignificatDigitsAmount;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
};


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::forceSpyOpenGLPolygonRasterMode
// Description: Force the Spy OpenGL Raster mode.
// Arguments:   rasterMode - The forced OpenGL raster mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::forceSpyOpenGLPolygonRasterMode(apRasterMode rasterMode)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaForceOpenGLPolygonRasterMode;

    // Send the arguments:
    spyConnectionSocket << (gtInt32)rasterMode;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::cancelSpyOpenGLPolygonRasterMode
// Description: Cancels the spy "Force OpenGL polygon mode" mode
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/11/2004
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::cancelSpyOpenGLPolygonRasterMode()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaCancelOpenGLPolygonRasterModeForcing;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyGLDebugOutputLoggingStatus
// Description:
// Arguments:   bool glDebugOutputLoggingEnabled
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyGLDebugOutputLoggingStatus(bool glDebugOutputLoggingEnabled)
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaEnableGLDebugOutputLogging;

    // Send the arguments:
    spyConnectionSocket << glDebugOutputLoggingEnabled;

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::createCurrentDebugSessionLogFilesSubDirectory
// Description: Creates a sub directory of the log files directory, that will contain
//              the current debug session log files.
// Author:      Yaki Tebeka
// Date:        24/1/2005
// ---------------------------------------------------------------------------
void gaPersistentDataManager::createCurrentDebugSessionLogFilesSubDirectory()
{
    // If the current debug session log files sub-directory was not created yet:
    if (_currentDebugSessionLogFilesSubDirPath.asString().isEmpty())
    {
        // Initialize the sub dir to contain the log files directory:
        osFilePath logFilesSubDirPath = _apiInitData.logFilesDirectoryPath();

        // Get the debugged process name:
        gtString debuggedProcessName;
        bool rc = _debuggedProcessExePath.getFileName(debuggedProcessName);

        if (rc)
        {
            // Get the current data and time as a string:
            osTime currentTime;
            currentTime.setFromCurrentTime();
            gtString currentDateAsString;
            gtString currentTimeAsString;
            currentTime.dateAsString(currentDateAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
            currentTime.timeAsString(currentTimeAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

            // Build the sub directory name:
            // <App name>/<Current date>-<CurrentTime>

            // Append the sub dir name as <App name>:
            gtString subDirProjectName = _apiInitData.projectName();
            logFilesSubDirPath.appendSubDirectory(subDirProjectName);
            // Create the sub directory:
            osDirectory subDir(logFilesSubDirPath);

            // Check if we are remote debugging:
            bool isRemoteDebugging = false;
            pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
            osFilePath localLogPath(osFilePath::OS_TEMP_DIRECTORY);
            const apProjectSettings* pProjectSettings = theProcessDebugger.debuggedProcessCreationData();

            if (NULL != pProjectSettings)
            {
                isRemoteDebugging = pProjectSettings->isRemoteTarget();
                localLogPath = pProjectSettings->logFilesFolder().directoryPath();
            }

            // Only create the directory for local debugging:
            if (!isRemoteDebugging)
            {
                rc = subDir.create();
            }

            if (rc)
            {
                // Replace the API init data log files dir with this project sub-dir:
                _apiInitData.setProjectlogFilesDirectoryPath(logFilesSubDirPath);
            }

            // Append the sub dir name as <Current date>-<CurrentTime>:
            gtString subDirDateTime;
            subDirDateTime = currentDateAsString;
            subDirDateTime += L"-";
            subDirDateTime += currentTimeAsString;
            logFilesSubDirPath.appendSubDirectory(subDirDateTime);
            // Create the sub directory:
            subDir = logFilesSubDirPath;

            // Only create the directory for local debugging:
            if (!isRemoteDebugging)
            {
                rc = subDir.create();
            }

            if (rc)
            {
                // Store the sub directory path:
                _currentDebugSessionLogFilesSubDirPath = logFilesSubDirPath;

                // Replace the API init data log files dir with this session sub-dir:
                _apiInitData.setLogFilesDirectoryPath(_currentDebugSessionLogFilesSubDirPath);
            }

            // Also create the local log path:
            localLogPath.appendSubDirectory(subDirProjectName).appendSubDirectory(subDirDateTime).reinterpretAsDirectory();
            osDirectory localLogPathDir(localLogPath);
            rc = localLogPathDir.create();

            if (rc)
            {
                // Update this folder in the process debugger as well:
                theProcessDebugger.setLocalLogFileDirectory(localLogPath);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::deleteCurrentDebugSessionLogFilesSubDirectory
// Description:
//   Deletes the log files sub directory that contains the current debug session
//   log files.
// Author:      Yaki Tebeka
// Date:        24/1/2005
// ---------------------------------------------------------------------------
void gaPersistentDataManager::deleteCurrentDebugSessionLogFilesSubDirectory()
{
    // Verify that we are allowed to delete the sub dir:
    if (_deleteLogFilesWhenDebuggedProcessTerminates)
    {
        // If the current debug session sub dir was created:
        if (!(_currentDebugSessionLogFilesSubDirPath.asString().isEmpty()))
        {
            // Delete the
            osDirectory subDir(_currentDebugSessionLogFilesSubDirPath);
            bool rc = subDir.deleteRecursively();
            GT_ASSERT(rc);

            // Clean up:
            _currentDebugSessionLogFilesSubDirPath.setFullPathFromString(L"");
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::resumeDebuggedProcessMainThreadRun
// Description: Resumed the run of the debugged process main thread.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/9/2005
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::resumeDebuggedProcessMainThreadRun()
{
    bool retVal = false;

    // Get the debugged process main thread id:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
    osThreadId debuggedProcessMainThreadId = theProcessDebugger.mainThreadId();
    GT_ASSERT(debuggedProcessMainThreadId != OS_NO_THREAD_ID);

    if (debuggedProcessMainThreadId != OS_NO_THREAD_ID)
    {
        // Reset openGL recorder on flag
        _wasOpenGLRecorderOn = false;

        // Resume the main thread run:
        bool rc = theProcessDebugger.resumeDebuggedProcessThread(debuggedProcessMainThreadId);
        GT_ASSERT_EX(rc, GA_STR_FailedToResumeMainThread);

        retVal = rc;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::markContextAsDeleted
// Description: Marks the requested context as deleted.
// Arguments:   apContextID contextId
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/7/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::markContextAsDeleted(apContextID contextId)
{
    bool retVal = false;

    // If the _isContextDataSnapshotUpdated vector does not have an entry that represents
    // our context, add the queried context entry (and all its former contexts entries):
    verifyContextUpdateStatusAllocation(contextId);

    gtMap<apContextID, ContextDataUpdateStatus>::iterator findIter = _isContextDataSnapshotUpdatedMap.find(contextId);

    if (findIter != _isContextDataSnapshotUpdatedMap.end())
    {
        if ((*findIter).second == GA_CONTEXT_DATA_UPDATED)
        {
            (*findIter).second = GA_CONTEXT_DATA_UPDATED_AND_DELETED;
        }
        else if ((*findIter).second == GA_CONTEXT_DATA_UPDATED_AND_DELETED)
        {
            // Do nothing!
        }
        else
        {
            (*findIter).second = GA_CONTEXT_DATA_DELETED;
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::wasContextDeleted
// Description: Was this context deleted
// Arguments:   apContextID contextId
// Return Val:  bool - true iff context was deleted
// Author:      Sigal Algranaty
// Date:        8/8/2010
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::wasContextDeleted(const apContextID& contextId)
{
    bool retVal = false;
    gtMap<apContextID, ContextDataUpdateStatus>::iterator findIter = _isContextDataSnapshotUpdatedMap.find(contextId);

    if (findIter != _isContextDataSnapshotUpdatedMap.end())
    {
        if ((findIter->second == GA_CONTEXT_DATA_UPDATED_AND_DELETED) || (findIter->second == GA_CONTEXT_DATA_DELETED))
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setHexDisplayMode
// Description: set hexadecimal display mode and fire the event
// Author:      Gilad Yarnitzky
// Date:        17/5/2011
// ---------------------------------------------------------------------------
void gaPersistentDataManager::setHexDisplayMode(bool hexMode)
{
    _hexDisplayMode = hexMode;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::getCrashReportAdditionalInformation
// Description: Get the additional information need for the crash report
// Arguments:   bool& openCLEnglineLoaded
//              bool& openGLEnglineLoaded
//              bool &kernelDebuggingEnteredAtLeastOnce
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/7/2011
// ---------------------------------------------------------------------------
void gaPersistentDataManager::getCrashReportAdditionalInformation(bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    openCLEnglineLoaded = _openCLEnglineLoaded;
    openGLEnglineLoaded = _openGLEnglineLoaded;
    kernelDebuggingEnteredAtLeastOnce = _kernelDebuggingEnteredAtLeastOnce;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setSpyKernelSourceCodePath
// Description: Set the kernel source code file paths
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setSpyKernelSourceCodePath()
{
    bool retVal = false;

    // Get the Spy connecting socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the function Id:
    spyConnectionSocket << (gtInt32)GA_FID_gaSetKernelSourceFilePath;

    // Send the number of program file paths:
    int amountOfPaths = _kernelSourceCodePaths.size();
    spyConnectionSocket << (gtInt32)amountOfPaths;

    // Send programs paths
    for (int numProgram = 0 ; numProgram < amountOfPaths; numProgram++)
    {
        // serialize as string since there is no >> operator for osFilePath
        spyConnectionSocket << _kernelSourceCodePaths[numProgram].asString();
    }

    // Receive success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::setKernelSourceCodePath
// Description: Sets the kernel source code file paths
// Arguments:   const gtVector<gtString>& kernelSourceCodePaths
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
bool gaPersistentDataManager::setKernelSourceCodePath(const gtVector<osFilePath>& kernelSourceCodePaths)
{
    bool retVal = true;

    // Clear the kernel source code file paths:
    _kernelSourceCodePaths.clear();

    for (int i = 0; i < (int) kernelSourceCodePaths.size(); i++)
    {
        _kernelSourceCodePaths.push_back(kernelSourceCodePaths[i]);
    }

    // If the API is active - set it to break at the next monitored function call:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        retVal = setSpyKernelSourceCodePath();
    }

    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        gaPersistentDataManager::bindSourceCodeBreakpoints
// Description: When a program is created - bind all the source code breakpoints
//              related to this program source to a kernel source code breakpoints
// Arguments:   const apOpenCLProgramCreatedEvent& programCreatedEvent
// Author:      Sigal Algranaty
// Date:        22/9/2011
// ---------------------------------------------------------------------------
void gaPersistentDataManager::bindSourceCodeBreakpoints(const osFilePath& programFilePath)
{
    // Get the amount of breakpoints:
    int breakpointsAmount = -1;
    bool rc = gaGetAmountOfBreakpoints(breakpointsAmount);
    GT_IF_WITH_ASSERT(rc)
    {
        // Go through each of the source code breakpoints, and collect the following data:
        // Indices of breakpoints to delete, line numbers, enable states for each of the
        // breakpoints related to this program:
        // Collect the breakpoints from the end to the beginning since we do not want to crush while removing the indices:
        struct gaSourceBreakpointData
        {
            int m_index;
            int m_lineNum;
            bool m_isEnabled;
        };

        // Use a structure to avoid vector sync issues:
        gtVector<gaSourceBreakpointData> breakpointsVec;

        for (int i = breakpointsAmount - 1; i >= 0 ; i--)
        {
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            rc = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc)
            {
                // If this is a source code breakpoint, compare the source code:
                if (aptrBreakpoint->type() == OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT)
                {
                    // Down cast to source code breakpoint:
                    apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)aptrBreakpoint.pointedObject();
                    GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != NULL)
                    {
                        // Compare the source code file path to the one related to the created program:
                        if (pSourceCodeBreakpoint->filePath() == programFilePath)
                        {
                            gaSourceBreakpointData bpData = { i, pSourceCodeBreakpoint->lineNumber(), pSourceCodeBreakpoint->isEnabled() };
                            breakpointsVec.push_back(bpData);
                        }
                    }
                }
            }
        }

        // If there are any breakpoints in this file:
        int bpCount = (int)breakpointsVec.size();

        if (0 < bpCount)
        {
            // Get the created program handle:
            oaCLProgramHandle programHandle;
            osFilePath temporaryNewFilePath;
            rc = gaGetOpenCLProgramHandleFromSourceFilePath(programFilePath, temporaryNewFilePath, programHandle);

            if (rc)
            {
                if (programHandle != OA_CL_NULL_HANDLE)
                {
                    for (int i = 0; bpCount > i; i++)
                    {
                        const gaSourceBreakpointData& currentBP = breakpointsVec[i];

                        // Create a new kernel source code breakpoint:
                        apKernelSourceCodeBreakpoint kernelSourceCodeBreakpoint(programHandle, currentBP.m_lineNum);
                        kernelSourceCodeBreakpoint.setEnableStatus(currentBP.m_isEnabled);

                        // Set the breakpoint:
                        rc = gaSetBreakpoint(kernelSourceCodeBreakpoint);
                        GT_ASSERT(rc);
                    }
                }
            }
            else
            {
                // Not OpenCL break point. Create a new host source code breakpoint
                // In case host source breakpoint only one line number exist in the vector:
                if (breakpointsVec.size() == 1)
                {
                    const gaSourceBreakpointData& firstBP = breakpointsVec[0];

                    apHostSourceCodeBreakpoint hostSourceCodeBreakpoint(programFilePath, firstBP.m_lineNum);
                    hostSourceCodeBreakpoint.setEnableStatus(firstBP.m_isEnabled);
                    // Set the breakpoint:
                    rc = gaSetBreakpoint(hostSourceCodeBreakpoint);
                    GT_ASSERT(rc);
                }
            }
        }
    }
}

