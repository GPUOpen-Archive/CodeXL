//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLMonitor.h
///
//==================================================================================

//------------------------------ csOpenCLMonitor.h ------------------------------

#ifndef __CSOPENCLMONITOR_H
#define __CSOPENCLMONITOR_H

// Forward declarations:
class osCriticalSectionDelayedLocker;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apErrorCode.h>
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTServerUtilities/Include/suITechnologyMonitor.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>

// Local:
#include <src/csContextMonitor.h>
#include <src/csDevicesMonitor.h>
#include <src/csOpenCLHandleMonitor.h>
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <src/csAMDPerformanceCountersManager.h>
#endif
#include <src/csOpenCLQueuesPerformanceCountersManager.h>
#include <src/csForcedModesManager.h>

// ----------------------------------------------------------------------------------
// Class Name:           csOpenCLMonitor : public suITechnologyMonitor
// General Description:
//   The main class of the OpenCL spy.
//   Responsible for logging and monitoring:
//   - The application activity on top of the OpenCL API
//   - OpenCL contexts
//   - OpenCL allocated objects
//   - etc
//
// Author:               Yaki Tebeka
// Creation Date:        29/10/2009
// ----------------------------------------------------------------------------------
class csOpenCLMonitor : public suITechnologyMonitor
{
public:
    static csOpenCLMonitor& instance();
    virtual ~csOpenCLMonitor();

    // Overrides suITechnologyMonitor:
    virtual void onDebuggedProcessTerminationAlert();
    virtual void beforeDebuggedProcessSuspended();
    virtual void afterDebuggedProcessResumed();
    virtual void beforeBreakpointException(bool isInOpenGLBeginEndBlock);
    virtual void afterBreakpointException(bool isInOpenGLBeginEndBlock);
    virtual void onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode);
    virtual void onBeforeKernelDebugging();
    virtual void onGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn);

    // Events:
    void onContextCreation(cl_context contextHandle, apMonitoredFunctionId creationFunc);
    void onCommandQueueCreation(cl_command_queue commandQueueHandle, cl_context context, cl_device_id device, cl_command_queue_properties properties);
    void onCommandQueueCreationWithProperties(cl_command_queue commandQueueHandle, cl_context context, cl_device_id device, const cl_queue_properties*  properties);
    void onCommandQueuePropertiesSet(cl_command_queue commandQueueHandle, cl_command_queue_properties properties, cl_bool enable);
    void onProgramCreation(cl_context contextHandle, cl_program programHandle, cl_uint count, const char** strings, const size_t* lengths);
    void onKernelCreation(cl_program program, cl_kernel kernel, const gtString& kernelName);
    void onKernelArgumentSet(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value, bool isSVMPointer);
    void onSamplerCreation(cl_context contextHandle, cl_sampler samplerHandle, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode);
    void onSamplerCreationWithProperties(cl_context contextHandle, cl_sampler samplerHandle, const cl_sampler_properties* properties);
    void onFrameTerminatorCall(oaCLContextHandle hContext);
    void onEnqueueNDRangeKernel(oaCLCommandQueueHandle commandQueueHandle);
    void onMemoryObjectWriteCommand(oaCLMemHandle memObjectHandle);
    void onBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* pCompileOptions, bool programDebuggable, const gtString& programNotDebuggableReason, bool canUpdateProgramKernels);
    void onBeforeBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* pCompileOptions);
    void onProgramsLinked(cl_program program, cl_uint num_related_programs, const cl_program* related_programs);
    void onBuildProgramFailedWithDebugFlags(cl_program program, cl_int buildErrorCode);
    void releaseProgramForBuild(cl_program program);
    void restoreProgramFromBuild(cl_program program);


    void beforeMonitoredFunctionExecutionActions(apMonitoredFunctionId monitoredFunctionId);
    void afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId);
    void addFunctionCall(oaCLHandle objectCLHandle, apMonitoredFunctionId calledFunctionId, int argumentsAmount, ...);

    // Kernel debugging:
    // ---------------------------------------------------------------------------
    // Name:        csAMDKernelDebuggingManager::shouldDebugKernel
    // Description: Returns true iff we want to call the debug API for kernel instead
    //              of the real clEnqueueNDRangeKernel function.
    // Author:      Uri Shomroni
    // Date:        26/10/2010
    // ---------------------------------------------------------------------------
    bool shouldDebugKernel(oaCLKernelHandle kernel, suIKernelDebuggingManager::KernelDebuggingSessionReason& reason, osCriticalSectionDelayedLocker& dispatchLock);
    void beforeKernelDebuggingEnqueued();
    void afterKernelDebuggingEnqueued();
    void onKernelDebuggingEnqueueFailure();
    void reportKernelDebuggingFailure(cl_int originalOpenCLError, cl_int openCLError, gtString& errorString);

    // Context monitor by id:
    const csContextMonitor* clContextMonitor(int contextId) const;
    csContextMonitor* clContextMonitor(int contextId);

    // Context monitor by context handle:
    const csContextMonitor* clContextMonitor(oaCLContextHandle contextHandle) const;
    csContextMonitor* clContextMonitor(oaCLContextHandle contextHandle);

    // Context containing objects:
    const csContextMonitor* contextContainingMemObject(oaCLMemHandle memObjectHandle) const;
    csContextMonitor* contextContainingMemObject(oaCLMemHandle memObjectHandle);
    const csContextMonitor* contextContainingProgram(oaCLProgramHandle programHandle) const;
    csContextMonitor* contextContainingProgram(oaCLProgramHandle programHandle);
    const csContextMonitor* contextContainingKernel(oaCLKernelHandle kernelHandle) const;
    csContextMonitor* contextContainingKernel(oaCLKernelHandle kernelHandle);
    const csContextMonitor* contextContainingSampler(oaCLSamplerHandle samplerHandle) const;
    csContextMonitor* contextContainingSampler(oaCLSamplerHandle samplerHandle);
    const csContextMonitor* contextContainingQueue(oaCLCommandQueueHandle queueHandle) const;
    csContextMonitor* contextContainingQueue(oaCLCommandQueueHandle queueHandle);
    const csContextMonitor* contextContainingEvent(oaCLEventHandle eventHandle) const;
    csContextMonitor* contextContainingEvent(oaCLEventHandle eventHandle);
    apCLMemObject* getMemryObjectDetails(oaCLMemHandle memObjectHandle);
    const apCLMemObject* getMemryObjectDetails(oaCLMemHandle memObjectHandle) const ;

    // Command Queues and event objects:
    const csCommandQueueMonitor* commandQueueMonitor(oaCLCommandQueueHandle queueHandle) const;
    csCommandQueueMonitor* commandQueueMonitor(oaCLCommandQueueHandle queueHandle);

    // Programs and kernels:
    oaCLProgramHandle programContainingKernel(oaCLKernelHandle kernelHandle) const;
    oaCLProgramHandle programHandleFromSourcePath(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath) const;
    bool setKernelSourceFilePath(gtVector<osFilePath>& programsFilePath);

    // Object reference counts:
    void checkIfDeviceWasDeleted(cl_device_id device);
    cl_uint deviceExternalReferenceCount(cl_device_id device);
    void checkIfContextWasDeleted(cl_context context);
    cl_uint contextExternalReferenceCount(cl_context context);
    void checkIfCommandQueueWasDeleted(cl_command_queue command_queue, bool checkParentObjects);
    cl_uint commandQueueExternalReferenceCount(cl_command_queue command_queue);
    void checkIfMemObjectWasDeleted(cl_mem memobj, bool checkParentObjects);
    cl_uint memObjectExternalReferenceCount(cl_mem memobj);
    void checkIfSamplerWasDeleted(cl_sampler sampler, bool checkParentObjects);
    cl_uint samplerExternalReferenceCount(cl_sampler sampler);
    void checkIfProgramWasDeleted(cl_program program, bool checkParentObjects);
    cl_uint programExternalReferenceCount(cl_program program);
    void checkIfKernelWasDeleted(cl_kernel kernel, bool checkParentObjects);
    cl_uint kernelExternalReferenceCount(cl_kernel kernel);
    void checkIfEventWasDeleted(cl_event event, bool checkParentObjects);
    cl_uint eventExternalReferenceCount(cl_event event);

    // Object platform association:
    bool isObjectOnAMDPlatform(oaCLHandle objectHandle, osTransferableObjectType expectedObjectType = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES) const;
    bool isProgramOnAMDPlatform(oaCLProgramHandle programHandle) const;

    // Detected errors:
    void reportDetectedError(int contextIndex, apErrorCode errorCode, const gtString& errorDescription, apMonitoredFunctionId associatedFuncId = apMonitoredFunctionsAmount);

    // OpenCL handles monitor:
    const csOpenCLHandleMonitor& openCLHandleMonitor() const { return _openCLHandlesMonitor; };
    csOpenCLHandleMonitor& openCLHandleMonitor() { return _openCLHandlesMonitor; };

    // Devices monitor:
    const csDevicesMonitor& devicesMonitor() const {return _devicesMonitor;};
    csDevicesMonitor& devicesMonitor() {return _devicesMonitor;};

    // Programs and Kernels:
    cl_program internalProgramHandleFromExternalHandle(cl_program externalHandle) const;
    cl_kernel internalKernelHandleFromExternalHandle(cl_kernel externalHandle) const;

    // Command Queues:
    bool isCommandQueueProfileModeForcedForQueue(oaCLCommandQueueHandle queueHandle) const;
    bool isCommandQueueProfileModeForcedForDevice(int deviceIndex) const;

    // OpenCL error handling:
    void onOpenCLError(int openCLErrorCode, oaCLContextHandle hContext = OA_CL_NULL_HANDLE);
    void setBreakOnOpenCLErrorsMode(bool breakOnCLErrors);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    // AMD Performance counters:
    csAMDPerformanceCountersManager& AMDPerformanceCountersManager() { return _AMDPerformanceCountersMgr;};
#endif
#endif

    // OpenCL queue performance counters manager:
    csOpenCLQueuesPerformanceCountersManager& openCLQueueCountersManager() {return _openCLQueueCountersManager;}

    // Set OpenCL operation execution:
    void setOpenCLOperationExecution(apOpenCLExecutionType executionType, bool isExecutionOn) {_forceModesManager.setExecutionMode(executionType, isExecutionOn);};
    bool isOpenCLOperationExecutionOn(apOpenCLExecutionType executionType) const { return _forceModesManager.isOperationExecutionOn(executionType);};
    oaCLKernelHandle stubKernelFromRealKernel(oaCLKernelHandle realKernel);
    oaCLMemHandle stubImageFromRealImage(oaCLMemHandle realImage);
    oaCLMemHandle stubBufferFromRealBuffer(oaCLMemHandle realBuffer);
    static void CL_CALLBACK stubNativeKernel(void* ignored);

    // Checks if write operation should currently be performed for the requested memory object:
    bool isOpenCLWriteOperationExecutionOn(cl_mem memobj) const;

    // Memory leaks:
    void checkForProcessMemoryLeaks();

    // Programs path:
    const gtVector<osFilePath>& programsFilePath() { return _programsFilePath; }

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    csOpenCLMonitor& operator=(const csOpenCLMonitor& otherMonitor) = delete;
    csOpenCLMonitor& operator=(csOpenCLMonitor&& otherMonitor) = delete;
    csOpenCLMonitor(const csOpenCLMonitor& otherMonitor) = delete;
    csOpenCLMonitor(csOpenCLMonitor&& otherMonitor) = delete;

    // Only my instance() method should create me:
    csOpenCLMonitor();

    // Helper functions:
    apFunctionDeprecationStatus functionDeprecationStatus(apMonitoredFunctionId calledFunctionId);
    void enterDeletionCheckerFunction(osCriticalSectionDelayedLocker& objectDeletionCSL);

    // Allow csSingletonsDelete to delete me:
    friend class csSingletonsDelete;

private:
    // Contains true after the OpenCL Server is initialized:
    bool _wasOpenCLServerInitialized;

    // Devices monitor:
    csDevicesMonitor _devicesMonitor;

    // A monitor that handle all the OpenCL handles:
    csOpenCLHandleMonitor _openCLHandlesMonitor;

    // A pointer to this class single instance:
    static csOpenCLMonitor* _pMySingleInstance;

    // Contains true iff we are forcing command queues' CL_QUEUE_PROFILING_ENABLE flag on:
    bool _isCommandQueuesProfileModeForced;

    // A critical section to prevent multiple functions from getting called at the same time (OpenCL 1.1+ is thread-safe,so
    // we're not changing behavior here):
    osCriticalSection _functionCallCS;

    // A critical section to prevent multiple threads (namely the API and application threads) from updating object existence at the same time
    osCriticalSection m_objectDeletionCS;

    // Contains true iff we should break on OpenCL errors:
    bool _breakOnOpenCLErrors;

    // Counter that is being reset each CS_AMOUNT_OF_CALLS times:
    int _functionsCounter;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    // The AMD spy performance counters manager:
    csAMDPerformanceCountersManager _AMDPerformanceCountersMgr;
#endif
#endif

    // OpenCL Queue counters manager:
    csOpenCLQueuesPerformanceCountersManager _openCLQueueCountersManager;

    // Contain the last function context id:
    int _lastFunctionContextId;

    // Force mode manager that hold the force mode flags:
    csForcedModesManager _forceModesManager;

    // Kernel source file paths:
    gtVector<osFilePath> _programsFilePath;

    // A map, used to monitor which programs currently have their reference count lowered
    // to allow program builds. See the documentation for releaseProgramForBuild() and restoreProgramFromBuild().
    gtMap<cl_program, bool> m_isProgramRefCountDecreasedForBuild;
};


#endif //__CSOPENCLMONITOR_H

