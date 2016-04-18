//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLMonitor.h
///
//==================================================================================

//------------------------------ gsOpenGLMonitor.h ------------------------------

#ifndef __GSOPENGLMONITOR
#define __GSOPENGLMONITOR

// Forward deceleration:
class gsContextCreatedEvent;
class gsContextDeletedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apErrorCode.h>
#include <AMDTAPIClasses/Include/apRasterMode.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apCounterType.h>
#include <AMDTServerUtilities/Include/suITechnologyMonitor.h>
#include <AMDTServerUtilities/Include/suContextMonitor.h>

// Local:
#include <src/gsThreadLocalData.h>
#include <src/gsSpyPerformanceCountersManager.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsPBuffersMonitor.h>
#include <src/gsSyncObjectsMonitor.h>
#include <src/gsThreadsMonitor.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <src/gsATIPerformanceCountersManager.h>
#endif


// ----------------------------------------------------------------------------------
// Class Name:          gsOpenGLMonitor : public suITechnologyMonitor
// General Description:
//   The main class of the OpenGL32.dll spy.
//   Responsible for logging and monitoring:
//   - The application activity on top of the OpenGL API.
//   - OpenGL internal states and objects.
//   - etc.
//
// Author:              Yaki Tebeka
// Creation Date:       11/5/2004
// ----------------------------------------------------------------------------------
class gsOpenGLMonitor : public suITechnologyMonitor
{
public:
    static gsOpenGLMonitor& instance();
    virtual ~gsOpenGLMonitor();

    // Overrides suITechnologyMonitor:
    virtual void onDebuggedProcessTerminationAlert();
    virtual void beforeDebuggedProcessSuspended();
    virtual void afterDebuggedProcessResumed();
    virtual void beforeBreakpointException(bool isInOpenGLBeginEndBlock);
    virtual void afterBreakpointException(bool isInOpenGLBeginEndBlock);
    virtual void onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode);
    virtual void onBeforeKernelDebugging();

    virtual void onFrameTerminatorCall();

    // Debugged process threads:
    int threadCurrentRenderContext(const osThreadId& threadId) const;
    osThreadId renderContextCurrentThread(int renderContextId) const;

    // Performance counters:
    const gsSpyPerformanceCountersManager& spyPerformanceCountersManager() const { return _spyPerformanceCountersMgr; };
    gsSpyPerformanceCountersManager& spyPerformanceCountersManager() { return _spyPerformanceCountersMgr; };

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    // ATI Performance counters:
    gsATIPerformanceCountersManager& ATIPerformanceCountersManager() { return _ATIPerformanceCountersMgr; };
#endif
#endif

    // OpenGL global events:
    //("per context" events are handled by gsRenderContextMonitor)
    void onContextCreation(oaDeviceContextHandle pDeviceContext, oaOpenGLRenderContextHandle pRenderContext, apMonitoredFunctionId creationFunc, const int* attribList = NULL, bool isDebugFlagForced = false);
    void beforeContextDeletion(oaOpenGLRenderContextHandle pRenderContext);
    void afterContextDeletion(oaOpenGLRenderContextHandle pRenderContext);
    void beforeContextMadeCurrent(oaOpenGLRenderContextHandle hRC);
    void onContextMadeCurrent(oaDeviceContextHandle hDC, oaDrawableHandle draw, oaDrawableHandle read, oaOpenGLRenderContextHandle hRC);

    // Render Contexts:
    int currentThreadRenderContextSpyId() const;
    void* renderContextDeviceContext(int renderContextId);
    oaPixelFormatId renderContextPixelFormat(int renderContextId);

    // Inter-context list sharing
    void onShareLists(oaOpenGLRenderContextHandle a, oaOpenGLRenderContextHandle b);
    void replaceContextIDWithListHolder(int& renderContextId) const;

    // Render contexts monitors:
    const gsRenderContextMonitor* renderContextMonitor(int renderContextId) const;
    gsRenderContextMonitor* renderContextMonitor(int renderContextId);
    const gsRenderContextMonitor* currentThreadRenderContextMonitor() const;
    gsRenderContextMonitor* currentThreadRenderContextMonitor();
    const suContextMonitor* currentThreadContextMonitor() const;
    suContextMonitor* currentThreadContextMonitor();

    // PBuffers monitor:
    const gsPBuffersMonitor& pbuffersMonitor() const { return _pbuffersMonitor; };
    gsPBuffersMonitor& pbuffersMonitor() { return _pbuffersMonitor; };

    // Sync objects monitor:
    const gsSyncObjectsMonitor& syncObjectsMonitor() const { return _syncObjectsMonitor; };
    gsSyncObjectsMonitor& syncObjectsMonitor() { return _syncObjectsMonitor; };

    // Textures:
    GLuint boundTexture(GLenum bindTarget) const;
    GLuint forcedStubTextureName(GLenum bindTarget) const;
    bool isInteractiveBreakOn() const { return _isInteractiveBreakOn; };

    // Programs and Shaders:
    GLuint activeProgram() const;

    // Monitored function calls events:
    void beforeMonitoredFunctionExecutionActions(apMonitoredFunctionId monitoredFunctionId);
    void afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId);
    void addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, ...);

    // Breakpoints:
    virtual void getCurrentOpenGLError(GLenum& opeglError) const { opeglError = _openGLError; };
    virtual void triggerBreakpointException(apBreakReason breakReason, GLenum opeglError, bool isInGLBeginEndBlock);
    virtual void onGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn);

    // Detected Errors:
    void reportDetectedError(apErrorCode errorCode, const gtString& errorDescription, apMonitoredFunctionId associatedFuncId = apMonitoredFunctionsAmount);

    // Forced modes:
    void forceOpenGLFlush(bool isOpenGLFlushForced) { _isOpenGLFlushForced = isOpenGLFlushForced; };
    void setInteractiveBreakMode(bool isInteractiveBreakOn) { _isInteractiveBreakOn = isInteractiveBreakOn; };
    void forceOpenGLPolygonRasterMode(apRasterMode rasterMode);
    void cancelOpenGLPolygonRasterModeForcing();
    void forceStubMode(apOpenGLForcedModeType stubType, bool isStubForced);

    // OpenGL debug output:
    void setGLDebugOutputLoggingEnabled(bool enabled);
    void setGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled);
    void setGLDebugOutputKindMask(const gtUInt64& mask);

    // Monitored function wrappers Helper functions:
    void handleForeignContextExtensionFuncCall(const wchar_t* extensionFunctionName, bool& extensionFuncPtrFound);
    void handleForeignContextExtensionFuncCall(const char* extensionFunctionName, bool& extensionFuncPtrFound);

    // Get the context spy id:
    int renderContextSpyId(oaOpenGLRenderContextHandle pRenderContext);

    // Forced debug context:
    bool doesDebugForcedContextExist(bool isDebugContext);

    // Memory leaks:
    void checkForProcessMemoryLeaks();

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsOpenGLMonitor& operator=(const gsOpenGLMonitor& otherMonitor);
    gsOpenGLMonitor(const gsOpenGLMonitor& otherMonitor);

    // Breakpoints:
    void testAndTriggerAfterFuncExecutionBreakpoints(apMonitoredFunctionId monitoredFunctionId);
    void testAndTriggerBeforeFuncExecutionBreakpoints(apMonitoredFunctionId monitoredFunctionId);
    void updateCurrentContextDataSnapshot();
    void testOpenGLBreakpoints(apMonitoredFunctionId monitoredFunctionId, apBreakReason& breakReason);

    // Misc:
    void initializeRenderContextMonitor(gsRenderContextMonitor& renderContextMonitor);
    bool displayCurrentRenderContextContent();

    // Only my instance() method should create me:
    gsOpenGLMonitor();
    void initialize();

private:
    // Contains true after the OpenGL Server is initialized:
    bool _wasOpenGLServerInitialized;

    // The spy performance counters manager:
    gsSpyPerformanceCountersManager _spyPerformanceCountersMgr;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    // The ATI spy performance counters manager:
    gsATIPerformanceCountersManager _ATIPerformanceCountersMgr;
#endif
#endif

    // The threads monitor:
    gsThreadsMonitor _threadsMonitor;

    // Critical section object that control the access to _renderContextsMonitors:
    osCriticalSection _renderContextsMonitorsCS;

    // PBuffers monitor:
    gsPBuffersMonitor _pbuffersMonitor;

    // Sync objects monitor:
    gsSyncObjectsMonitor _syncObjectsMonitor;

    // The current OpenGL error:
    GLenum _openGLError;

    // Contains true iff we force OpenGL buffers flush after each OpenGL call:
    bool _isOpenGLFlushForced;

    // Contains true iff "Interactive break" mode is on:
    bool _isInteractiveBreakOn;

    // Contains true iff we started the debugged process termination:
    bool _isDuringDebuggedProcessTermination;

    // Force mode manager that hold the force mode flags when no context exists:
    gsForcedModesManager _initForceModesManager;

private:
    // A pointer to this class single instance:
    static gsOpenGLMonitor* _pMySingleInstance;

    // Allow gsSingletonsDelete to clean up the single instance of this class.
    friend class gsSingletonsDelete;
};

#endif  // __GSOPENGLMONITOR
