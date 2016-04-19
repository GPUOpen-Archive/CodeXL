//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csStringConstants.h
///
//==================================================================================

//------------------------------ csStringConstants.h ------------------------------

#ifndef __CSSTRINGCONSTANTS_H
#define __CSSTRINGCONSTANTS_H

// Debug log printouts:
#define CS_STR_DebugLog_OpenCLServerInitializing L"The OpenCL Server is initializing"
#define CS_STR_DebugLog_OpenCLServerInitialionEnded L"The OpenCL Server initialization ended"
#define CS_STR_DebugLog_OpenCLServerInitializationFailed L"CodeXL's OpenCL Server initialization has failed. The debugged application will now exit"
#define CS_STR_DebugLog_loadingSystemOCLICDModule L"Loading system's OpenCL ICD module: "
#define CS_STR_DebugLog_systemOCLICDModuleLoadFailed L"Error: Failed to load the system's OpenCL ICD module"
#define CS_STR_DebugLog_systemOCLICDModuleLoaded L"The System's OpenCL ICD module was loaded successfully"
#define CS_STR_DebugLog_wrappingSystemOCLFunctions L"Wrapping system's OpenCL ICD module functions"
#define CS_STR_DebugLog_wrappingSystemOCLFunctionsEnded L"Ended wrapping system's OpenCL ICD module functions"
#define CS_STR_DebugLog_cannotGetOCLFuncPtr L"Error: Cannot retrieve OpenCL function pointer: "
#define CS_STR_DebugLog_startedHandlingAPIInitCalls L"Started handling OpenCL Server API initialization calls"
#define CS_STR_DebugLog_endedHandlingAPIInitCalls L"Ended handling OpenCL Server API initialization calls"
#define CS_STR_DebugLog_MainThreadWaitsForAPIThreadToHandleOCLAPIInitialization L"The main thread waits for OpenCL Server API initililization, handled by the API thread"
#define CS_STR_DebugLog_MainThreadFinishedWaitingForAPIThreadToHandleOCLAPIInitialization L"The main thread finished waiting for OpenCL Server API initililization, handled by the API thread"
#define CS_STR_DebugLog_UpdateOpenCLContextDataSnapshotStart L"gaUpdateOpenCLContextDataSnapshotStub (ran by API thread) started"
#define CS_STR_DebugLog_UpdateOpenCLContextDataSnapshotEnd L"gaUpdateOpenCLContextDataSnapshotStub (ran by API thread) ended"
#define CS_STR_DebugLog_clWaitForEventsTerminatingFrameOnlyForFirstContextInevents_list L"clWaitForEvents called with events from mixed contexts. Terminating frame only context related to first event in list."
#define CS_STR_DebugLog_clWaitForEventsWithBadParametersIgnoredAsTerminator L"clWaitForEvents with bad parameters (num_events == 0 or invalid events) ignored as frame terminator."
#define CS_STR_DebugLog_aboutToReadGLObject L"About to read OpenGL-OpenCL interop object data."
#define CS_STR_DebugLog_detectedAMDOpenCLPlatformVersion L"Detected AMD OpenCL Platform runtime version as %d.%d. System string: "
#define CS_STR_retrievingFunctionPointer L"Retrieving OpenCL function pointer: "
#define CS_STR_unsupportedExtensionUse L"Warning: Using an OpenCL extension function that is not supported by CodeXL's OpenCL server"
#define CS_STR_computeContextWasCreated L"Compute Context %d was created"
#define CS_STR_computeContextWasDeleted L"Compute Context %d was deleted"

// File names:
#define CS_STR_callsLogFilePath L"OpenCLContext%d-CallsLog"
#define CS_STR_bufferFilePath L"OpenCLContext%d-Buffer%d"
#define CS_STR_textureFilePath L"OpenCLContext%d-Texture%d"
#define CS_STR_programFilePath L"%ls-OpenCLContext%d-CLProgram%.3u"

// Detected error descriptions:
#define CS_STR_queueEventsOverflow L"Maximal amount of events (%d) was exceeded in command queue %d in OpenCL context %d.\nUse clReleaseEvent once you have finished using the event object\nor use CodeXL's 'check for memory leaks' mechanism."

// Compute Context Calls History logger:
#define CS_STR_ComputeContextCallsHistoryLoggerMessagesLabelFormat L"OpenCL Compute Context %d: "

// ATI Performance counters integration:
#define CS_STR_initializingATICounters L"Initializing ATI performance counters support..."
#define CS_STR_ATICountersAreNotSupported L"The installed hardware does not support the ATI performance counters"
#define CS_STR_ATICountersInitialized L"ATI performance counters support was initialized successfully"
#define CS_STR_ATICountersInitializationFailed L"Failed to initialize the ATI performance counters support"
#define CS_STR_ATICountersSamplingUnknownState L"ATI performance counters sampling failure: unknown state"
#define CS_STR_ATICountersSamplingUnknownDataType L"ATI performance counters sampling failure: unknown counter data type"
#define CS_STR_ATICountersSamplingCounterValueFailure L"ATI performance counters sampling failure: could not get counter value"
#define CS_STR_ATICountersSamplingNotStartFailure L"ATI performance counters sampling failure: Trying to end pass and sampling did not start"

// Kernel debugging errors:
#define CS_STR_CouldNotDebugKernelInvalidArgsDetails L"Tried to enqueue a kernel that uses unsupported argument types.\nDebugging kernels that use the following argument types is not supported:\n- SVM buffers (using clSetKernelArgSVMPointer)\n- SVM pointers (using clSetKernelExecInfo with CL_KERNEL_EXEC_INFO_SVM_PTRS)\n- Fine-grained system SVM (using clSetKernelExecInfo with CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM)"

// Other:
#define CS_STR_OpenCLServerInitializedSuccessfully L"CodeXL OpenCL Server was initialized"
#define CS_STR_OpenCLServerInitializationFailureMessage L"Error: CodeXL's OpenCL Server failed to initialize\nThe debugged application (%ls) will now exit"
#define CS_STR_VariableWorkItemNotValid L"Work item is invalid"

#endif //__CSSTRINGCONSTANTS_H

