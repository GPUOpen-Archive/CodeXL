//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaPersistentDataManager.h
///
//==================================================================================

//------------------------------ gaPersistentDataManager.h ------------------------------

#ifndef __GAPERSISTENTDATAMANAGER
#define __GAPERSISTENTDATAMANAGER

// Forward deceleration:
class apApiConnectionEstablishedEvent;
class apOpenCLProgramCreatedEvent;
class apExceptionEvent;
class apModuleLoadedEvent;
class apDebuggedProcessRunStartedExternallyEvent;
class apDebuggedProcessIsDuringTerminationEvent;
class apContextDataSnapshotWasUpdatedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/apRasterMode.h>

// std
#include <mutex>


// ----------------------------------------------------------------------------------
// Class Name:           gaPersistentDataManager : public apIEventsObserver
// General Description:
//   Manages data that should remain persistent between debug sessions.
// Author:               Yaki Tebeka
// Creation Date:        15/2/2004
// ----------------------------------------------------------------------------------
class gaPersistentDataManager : public apIEventsObserver
{
public:
    static gaPersistentDataManager& instance();

    // Debugged process:
    void setDebuggedExecutablePath(const osFilePath& exePath) { _debuggedProcessExePath = exePath; };
    bool suspendDebuggedProcessExecution();
    bool isDuringDebuggedProcessTermination() { return _isDuringDebuggedProcessTermination; };

    void setTerminatingDebuggedProcessThroughAPI() {_terminatingDebuggedProcessThroughAPI = true;};

    // API:
    void setAPIInitializationData(const apApiFunctionsInitializationData& apiInitData) { _apiInitData = apiInitData; };
    bool setProcessDebuggerPersistentDataValues();

    // Context data snapshot:
    bool updateContextDataSnapshot(apContextID contextId);
    bool markContextAsDeleted(apContextID contextId);
    bool wasContextDeleted(const apContextID& contextId);

    // Breakpoints data:
    bool setBreakpoint(const apBreakPoint& breakpoint);
    bool setBreakpointState(int breakpointId, const apBreakPoint::State state);
    bool setKernelSourceCodeBreakpointProgramHandle(int index, oaCLProgramHandle programHandle);
    bool setBreakpointHitCount(int breakpointIndex, int hitCount);
    bool removeBreakpoint(int breakpointIndex);
    bool removeGenericBreakpoint(apGenericBreakpointType breakpointType);
    int amountOfBreakpoints() const;
    bool getBreakpoint(int breakPointId, gtAutoPtr<apBreakPoint>& aptrBreakpoint) const;
    bool getBreakpointIndex(const apBreakPoint& breakpoint, int& breakpointId) const;
    bool removeAllBreakpoints();

    /// Removes all the debugged process breakpoints whose state equals the parameter state
    bool removeAllBreakpointsByState(const apBreakPoint::State state);

    /// Change the state of all the debugged process enabled breakpoints to 'temporarily disabled' state
    bool temporarilyDisableAllBreakpoints();

    /// Change the state of any debugged process breakpoint whose state equals the parameter state to 'Enabled'
    bool enableAllBreakpointsByState(const apBreakPoint::State state);

    // Break on next monitored function call:
    bool setBreakPointOnNextMonitoredFunctionCall();

    // Break on next draw function call:
    bool setBreakPointOnNextDrawFunctionCall();

    // Break on next frame terminator:
    bool setBreakPointOnNextFrame();

    // Break inside the current function:
    bool setBreakPointInMonitoredFunctionCall();

    // Clear the step flags:
    bool clearAllStepFlags();

    // Log files directories:
    void deleteLogFilesWhenDebuggedProcessTerminates(bool deleteLogFiles) { _deleteLogFilesWhenDebuggedProcessTerminates = deleteLogFiles; };
    const osFilePath& currentDebugSessionLogFilesSubDirectory() const {return _apiInitData.logFilesDirectoryPath();};

    // Function call caching:
    bool getCachedFunctionCall(const apContextID& contextID, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall) const;
    void cacheFunctionCall(const apContextID& contextID, int callIndex, const gtAutoPtr<apFunctionCall>& aptrFunctionCall);

    // OpenCL handle caching:
    bool getCacheOpenCLObjectID(oaCLHandle hOpenCLObject, apCLObjectID& o_objectId) const;
    void cacheOpenCLObjectID(oaCLHandle hOpenCLObject, const apCLObjectID& objectId);

    // OpenGL context <-> thread correlation caching:
    bool getThreadCurrentOpenGLContextFromCache(osThreadId threadId, int& o_contextId) const;
    bool getOpenGLContextCurrentThreadFromCache(int contextId, osThreadId& o_threadId) const;
    void cacheThreadToOpenGLContextCorrelation(osThreadId threadId, int contextId);

    // HTML log file recording status:
    bool setHTMLLogFileRecordingMode(bool isHTMLLogFileRecordingOn);
    bool isHTMLLogFileRecordingOn() const { return _isHTMLLogFileRecordingOn; };
    bool wasOpenGLDataRecordedInDebugSession();

    // Slow motion delay:
    bool setSlowMotionDelay(int delayTimeUnits);
    int slowMotionDelay() const { return _slowMotionDelayTimeUnits; };

    // Force OpenGL flush:
    bool forceOpenGLFlush(bool isOpenGLFlushForced);
    bool isOpenGLFlushForced() const { return _isOpenGLFlushForced; };

    // "Interactive break" mode:
    bool setInteractiveBreakMode(bool isInteractiveBreakOn);
    bool isInteractiveBreakOn() const { return _isInteractiveBreakOn; };

    // Debugged process execution mode:
    bool setDebuggedProcessExecutionMode(apExecutionMode executionMode);
    apExecutionMode getDeubggedProcessExecutionMode() {return _debuggedProcessExecutionMode; };
    bool setSpyDebuggedProcessExecutionMode(apExecutionMode executionMode);

    // Kernel source paths:
    bool setKernelSourceCodePath(const gtVector<osFilePath>& kernelSourceCodePaths);
    bool setSpyKernelSourceCodePath();

    bool forceOpenGLPolygonRasterMode(apRasterMode rasterMode);
    bool cancelOpenGLPolygonRasterModeForcing();

    // Forces an OpenGL stub:
    bool forceOpenGLStub(apOpenGLForcedModeType openGLStubType, bool isStubForced);

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"PersistentDataManager"; };

    // Enable Images Logging:
    bool enableImagesDataLogging(bool isImagesDataLoggingEnabled);
    bool isImagesDataLogged() const { return _isImagesDataLoggingEnabled; };

    // Allocated object creation calls stacks collection:
    bool setAllocatedObjectsCreationCallsStacksCollection(bool collectCreationStacks);
    bool areAllocatedObjectsCreationCallsStacksCollected() const {return _areAllocatedObjectsCreationCallsStacksCollected;};

    // Reset the "Was recording done during last debug session" parameter
    void resetRecordingWasDoneFlag(bool isEnabled) { _wasOpenGLRecorderOn = isEnabled; };

    // Force Flush after every OpenGL Function Call:
    bool flushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall);
    bool isLogFileFlushedAfterEachFunctionCall() const { return _isLogFileFlushedAfterEachFunctionCall; };

    // OpenGL NULL Driver:
    bool forceOpenGLNullDriver(bool isNULLOpenGLImplOn);
    bool isOpenGLNullDriverForced() const { return _isOpenGLNullDriverForced; };

    // OpenGL Stub Operations:
    bool forceOpenGLStubOperation(apOpenGLForcedModeType openGLStubType, bool isStubForced);
    bool isStubOperationForced(apOpenGLForcedModeType openGLStubType) const { return _isOpenGLStubForced[openGLStubType]; };

    // OpenGL stub raster mode:
    apRasterMode forcedOpenGLRasterMode() const { return _forcedOpenGLRasterMode; };

    // OpenCL Operations Cancel:
    bool setOpenCLOperationExecution(apOpenCLExecutionType openCLExecutionType, bool isExecutionOn);
    bool isOpenCLOperationExecutionOn(apOpenCLExecutionType openCLExecutionType) const { return _isOpenCLExecutionOn[openCLExecutionType]; };

    // OpenCL kernel debugging:
    bool isInKernelDebugging() const {return _isInKernelDebugging && m_isInKernelDebuggingBreakpoint;};
    bool isKernelDebuggingOnGoing() const {return _isInKernelDebugging ;};
    int kernelDebuggingGlobalWorkOffset(int coordinate) const;
    int kernelDebuggingGlobalWorkSize(int coordinate) const;
    int kernelDebuggingLocalWorkSize(int coordinate) const;
    bool setKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int value);
    int kernelDebuggingCurrentWorkItemCoordinate(int coordinate) const;
    osThreadId kernelDebuggingThreadId() const {return _kernelDebuggingThreadId;};
    bool setKernelDebuggingEnable(bool kernelDebugging);
    bool setMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode);

    // HSA kernel debugging:
    bool isInHSAKernelDebugging() const { return m_isInHSAKernelDebugging; };

    // OpenGL Debug output setting:
    bool enableGLDebugOutputLogging(bool enableGLDebugOutputLogging);
    void getGLDebugOutputLoggingStatus(bool& glDebugOutputLoggingEnabled) const { glDebugOutputLoggingEnabled = _GLDebugOutputLoggingEnabled; };
    bool getGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity) const;
    const gtUInt64& getGLDebugOutputKindMask() const {return m_debugOutputKindsMask;};
    bool setGLDebugOutputKindMask(const gtUInt64& mask);
    bool setGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled);

    // hexadecimal Display mode:
    void setHexDisplayMode(bool hexMode);
    bool isHexDisplayMode() { return _hexDisplayMode; }

    // Extra crash information:
    void getCrashReportAdditionalInformation(bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

private:
    // Contains a context data update status:
    enum ContextDataUpdateStatus
    {
        GA_CONTEXT_DATA_NOT_UPDATED,            // The data was not updated in this process suspension.
        GA_CONTEXT_DATA_UPDATE_FAILED,          // We tried to update the data in this process suspension, but failed.
        GA_CONTEXT_DATA_UPDATED,                // The data was updated successfully in this process suspension.
        GA_CONTEXT_DATA_DELETED,                // The context was deleted.
        GA_CONTEXT_DATA_UPDATED_AND_DELETED     // The data was updated successfully, and the context was deleted.
    };

    // An index identifying a function call, for caching:
    struct FunctionCallIndex
    {
    public:
        // Needed for being the key for a map:
        bool operator<(const FunctionCallIndex& other) const
        {
            return (m_context < other.m_context) || ((m_context == other.m_context) && (m_indexInContext < other.m_indexInContext));
        };

        apContextID m_context;
        int m_indexInContext;
    };

    // The singletons deleter should be able to delete me:
    friend class gaSingletonsDelete;

    // Only my instance() method should create my single instance:
    gaPersistentDataManager();
    virtual ~gaPersistentDataManager();

    // Events handling:
    void onModuleLoadedEvent(const apModuleLoadedEvent& eve);
    void onProcessRunStartedEvent();
    void onAPIConnectionEstablishedEvent(const apApiConnectionEstablishedEvent& eve);
    void onProcessCreatedEvent();
    void onProcessTerminatedEvent();
    void onProcessCreationFailureEvent();
    void onProcessRunResumedEvent();
    void onProcessRunStartedExternallyEvent(const apDebuggedProcessRunStartedExternallyEvent& eve);
    void onProcessDuringTermination(const apDebuggedProcessIsDuringTerminationEvent& eve);
    void onContextUpdated(const apContextDataSnapshotWasUpdatedEvent& eve);
    void onExceptionEvent(const apExceptionEvent& eve, bool& vetoEvent);
    void onSpiesUtilitiesAPIConnection();
    void onOpenGLServerAPIConnection();
    void onOpenCLServerAPIConnection();
    void onHSAServerAPIConnection();

    // Misc functions:
    bool initializeSpiesAPI();
    bool createSpiesEventsForwardingSocket();
    // bool setProcessDebuggerPersistentDataValues(); is public, since it needs to be accessed from gaGRApiFunctions
    bool setSpiesUtilitiesPersistentDataValues();
    bool setOpenGLServerPersistentDataValues();
    bool setOpenCLServerPersistentDataValues();
    void verifyContextUpdateStatusAllocation(apContextID contextId);
    void clearIsContextDataSnapshotUpdatedVec();
    bool setSpyBreakpoint(const apBreakPoint& breakpoint);
    bool removeSpyBreakpoint(const apBreakPoint& breakpoint);
    bool removeAllSpyBreakpoints();
    bool makeSpiesSuspendDebuggedProcessExecution();
    bool getSpiesAPIThreadId();
    bool sendSpiesAPIInitData(osSocket& spyConnectionSocket);
    bool setSpyBreakPointOnNextMonitoredFunctionCall();
    bool setSpyBreakPointOnNextDrawFunctionCall();
    bool setSpyBreakPointOnNextFrame();
    bool setSpyBreakPointInMonitoredFunctionCall();
    bool clearAllSpyStepFlags();
    bool setSpyHTMLLogFileRecordingMode(bool isHTMLLogFileRecordingOn);
    bool setSpySlowMotionDelay(int delayTimeUnits);
    bool setSpyForceOpenGLFlush(bool isOpenGLFlushForced);
    bool setSpyInteractiveBreakMode(bool isInteractiveBreakOn);
    bool forceSpyOpenGLPolygonRasterMode(apRasterMode rasterMode);
    bool cancelSpyOpenGLPolygonRasterMode();
    bool setSpyFlushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall);
    bool setSpyEnableImagesDataLoging(bool isTexturesImageDataLoggingEnabled);
    bool setSpyCollectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks);
    bool setSpyForceNullOpenGLDriver(bool isNULLOpenGLImplOn);
    bool setSpyOpenGLForceStub(apOpenGLForcedModeType openGLStubType, bool isStubForced);
    bool setSpyOpenCLOperationExecution(apOpenCLExecutionType openCLExecutionType, bool isExecutionOn);
    void clearKernelDebuggingData();
    bool setSpyFloatParametersDisplayPrecision();
    bool setSpyGLDebugOutputLoggingStatus(bool glDebugOutputLoggingEnabled);
    bool setSpyGLDebugOutputKindMask(const gtUInt64& debugOutputKindMask);
    bool setSpyGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled);
    void createCurrentDebugSessionLogFilesSubDirectory();
    void deleteCurrentDebugSessionLogFilesSubDirectory();
    bool resumeDebuggedProcessMainThreadRun();
    void bindSourceCodeBreakpoints(const osFilePath& programFilePath);
    bool setSpyKernelDebuggingEnable();
    bool setSpyMultipleKernelDebugDispatchMode();
    bool resolveAllTemporaryBreakpoints();

private:
    // Contains the debugged process executable path:
    osFilePath _debuggedProcessExePath;

    // The API initialization data:
    apApiFunctionsInitializationData _apiInitData;

    // A list of the active breakpoints:
    gtVector<apBreakPoint*> _activeBreakpoints;

    // Contains true iff the debugged process execution should be suspended:
    bool _suspendDebuggedProcessExecution;

    // Contains true iff the Spy should break on the next monitored function call:
    bool _breakOnNextMonitoredFunctionCall;

    // Contains true iff the Spy should break on the next draw function call:
    bool _breakOnNextDrawFunctionCall;

    // Contains true iff the Spy should break on the next frame terminator:
    bool _breakOnNextFrame;

    // Contains true iff the Spy should break in the current monitored function call:
    bool _breakInMonitoredFunctionCall;

    // Contains true iff the log files should be deleted when the debugged
    // process exits:
    bool _deleteLogFilesWhenDebuggedProcessTerminates;

    // Contains the current debug session log files sub directory:
    osFilePath _currentDebugSessionLogFilesSubDirPath;

    // Contains true iff HTML log file recording is on:
    bool _isHTMLLogFileRecordingOn;

    // Contains true iff recording was done during this debug run session
    bool _wasOpenGLRecorderOn;

    // Contains the "Slow motion" delay (in abstract time units):
    int _slowMotionDelayTimeUnits;

    // Contains true iff we force OpenGL front draw buffer:
    bool _isFrontDrawBufferForced;

    // Contains true iff OpenGL flush is forced:
    bool _isOpenGLFlushForced;

    // Contains true iff "Interactive break" mode is on:
    bool _isInteractiveBreakOn;

    // Contains true iff OpenGL Textures logging enabled:
    bool _isImagesDataLoggingEnabled;

    // Contains true iff Allocation objects' creation calls stacks are collected:
    bool _areAllocatedObjectsCreationCallsStacksCollected;

    // Contains true iff flush after every OpenGL function call is enabled:
    bool _isLogFileFlushedAfterEachFunctionCall;

    // Contains true iff OpenGL null Driver is forced:
    bool _isOpenGLNullDriverForced;

    // Contain true iff each OpenGL stub mode is forced:
    bool _isOpenGLStubForced[AP_OPENGL_AMOUNT_OF_FORCED_STUBS];

    // Contain true iff each OpenCL execution operation is on:
    bool _isOpenCLExecutionOn[AP_OPENCL_AMOUNT_OF_EXECUTIONS];

    // Contains the forced OpenGL raster mode:
    // (Active when _isOpenGLRasterModeForced is true):
    apRasterMode _forcedOpenGLRasterMode;

    // OpenCL Kernel debugging:
    bool _waitingForKernelDebuggingToStart;
    bool _kernelDebuggingInterruptedWarningIssued;
    bool _isInKernelDebugging;
    bool m_isInKernelDebuggingBreakpoint;
    int _kernelDebuggingGlobalWorkOffset[3];
    int _kernelDebuggingGlobalWorkSize[3];
    int _kernelDebuggingLocalWorkSize[3];
    int _kernelDebuggingCurrentWorkItem[3];
    osThreadId _kernelDebuggingThreadId;
    gtVector<osFilePath> _kernelSourceCodePaths;

    // HSA kernel debugging:
    bool m_isInHSAKernelDebugging;

    // Contains true iff we the GL Debug Output integration is enabled:
    bool _GLDebugOutputLoggingEnabled;

    // The debug output category mask:
    gtUInt64 m_debugOutputKindsMask;

    // The debug output messages severity:
    bool m_debugOutputSeverities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES];

    // Contains true iff API connection with the spy was established:
    bool _isAPIConnectionEstablished;

    // Contains true iff the Spies Utilities module was loaded:
    bool _isSpiesUtilitiesModuleLoaded;

    // Contains true iff the Spies API was initialized (using the API init data):
    bool _isSpiesAPIInitialized;

    // Contains true iff the debugged process run started:
    bool _didDebuggedProcessRunStart;

    // Contains true iff we are during the debugged process termination:
    bool _isDuringDebuggedProcessTermination;

    // Contains true iff we are during the debugged process getting
    // terminated by gaTerminateDebuggedProcess (i.e. through the API):
    bool _terminatingDebuggedProcessThroughAPI;

    // Item 'apContextId' contains context data snapshot is update status:
    // (For the current debugged application suspension)
    gtMap<apContextID, ContextDataUpdateStatus> _isContextDataSnapshotUpdatedMap;

    // Mapping a context id x function call id to the function call data:
    gtMap<FunctionCallIndex, gtAutoPtr<apFunctionCall> > m_functionCallsCache;

    // Mapping a OpenCL handle x object id:
    gtMap<oaCLHandle, apCLObjectID> m_openCLHandlesCache;

    // Caching the OpenGL context <-> thread ID correlation:
    gtMap<osThreadId, int> m_threadCurrentOpenGLContextCache;
    gtMap<int, osThreadId> m_openGLContextCurrentThreadCache;

    // The single instance of this class:
    static gaPersistentDataManager* _pMySingleInstance;

    // Debugged process execution mode:
    apExecutionMode _debuggedProcessExecutionMode;

    // Variables hexadecimal display mode:
    bool _hexDisplayMode;

    // Additional crash report information:
    bool _openCLEnglineLoaded;
    bool _openGLEnglineLoaded;
    bool _kernelDebuggingEnteredAtLeastOnce;

    // Kernel debugging enable flag
    bool _kernelDebuggingEnable;
    apMultipleKernelDebuggingDispatchMode m_multipleKernelDebugDispatchMode;

    std::mutex  _mtxProcessCreating;

    bool m_apiConnectionInitialized;
    bool m_apiOpenCLConnectionInitialized;
    bool m_apiOpenGLConnectionInitialized;
};


#endif  // __GAPERSISTENTDATAMANAGER
