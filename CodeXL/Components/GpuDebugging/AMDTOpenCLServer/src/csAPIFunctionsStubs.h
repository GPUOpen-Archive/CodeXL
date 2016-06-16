//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAPIFunctionsStubs.h
///
//==================================================================================

//------------------------------ csAPIFunctionsStubs.h ------------------------------

#ifndef __CSAPIFUNCTIONSSTUBS_H
#define __CSAPIFUNCTIONSSTUBS_H

// Forward decelerations:
class osSocket;

// API management:
void csRegisterAPIStubFunctions();
void csHandleAPIInitializationCalls();
void gaOpenCLServerInitializationEndedStub(osSocket& apiSocket);

// Function calls:
void gaGetAmountOfOpenCLFunctionCallsStub(osSocket& apiSocket);
void gaGetOpenCLFunctionCallStub(osSocket& apiSocket);
void gaGetLastOpenCLFunctionCallStub(osSocket& apiSocket);
void gaFindOpenCLFunctionCallStub(osSocket& apiSocket);
void gaGetOpenCLHandleObjectDetailsStub(osSocket& apiSocket);

// Statistics:
void gaGetCurrentOpenCLStatisticsStub(osSocket& apiSocket);
void gaClearOpenCLFunctionCallsStatisticsStub(osSocket& apiSocket);

// Contexts:
void gaGetAmountOfOpenCLContextsStub(osSocket& apiSocket);
void gaUpdateOpenCLContextDataSnapshotStub(osSocket& apiSocket);
void gaGetOpenCLContextDetailsStub(osSocket& apiSocket);

// Log files recording:
void gaGetCLContextLogFilePathStub(osSocket& apiSocket);

// Programs & kernels:
void gaGetAmountOfOpenCLProgramObjectsStub(osSocket& apiSocket);
void gaGetOpenCLProgramObjectDetailsStub(osSocket& apiSocket);
void gaSetOpenCLProgramCodeStub(osSocket& apiSocket);
void gaBuildOpenCLProgramStub(osSocket& apiSocket);
void gaGetOpenCLProgramHandleFromSourceFilePathStub(osSocket& apiSocket);
void gaGetOpenCLKernelObjectDetailsStub(osSocket& apiSocket);
void gaSetKernelSourceFilePathStub(osSocket& apiSocket);

// Kernel debugging:
void gaGetKernelDebuggingLocationStub(osSocket& apiSocket);
void gaGetCurrentlyDebuggedKernelDetailsStub(osSocket& apiSocket);
void gaGetCurrentlyDebuggedKernelCallStackStub(osSocket& apiSocket);
void gaSetKernelDebuggingCommandStub(osSocket& apiSocket);
void gaSetKernelSteppingWorkItemStub(osSocket& apiSocket);
void gaIsWorkItemValidStub(osSocket& apiSocket);
void gaGetFirstValidWorkItemStub(osSocket& apiSocket);
void gaCanGetKernelVariableValueStub(osSocket& apiSocket);
void gaGetKernelDebuggingVariableValueStringStub(osSocket& apiSocket);
void gaGetKernelDebuggingVariableMembersStub(osSocket& apiSocket);
void gaGetKernelDebuggingAvailableVariablesStub(osSocket& apiSocket);
void gaGetKernelDebuggingAmountOfActiveWavefrontsStub(osSocket& apiSocket);
void gaGetKernelDebuggingActiveWavefrontIDStub(osSocket& apiSocket);
void gaGetKernelDebuggingWavefrontIndexStub(osSocket& apiSocket);
void gaUpdateKernelVariableValueRawDataStub(osSocket& apiSocket);
void gaGetKernelSourceCodeBreakpointResolutionStub(osSocket& apiSocket);
void gaSetKernelDebuggingEnableStub(osSocket& apiSocket);
void gaSetMultipleKernelDebugDispatchModeStub(osSocket& apiSocket);

// Devices:
void gaGetOpenCLDeviceObjectDetailsStub(osSocket& apiSocket);

// Platforms:
void gaGetOpenCLPlatformAPIIDStub(osSocket& apiSocket);

// Buffers:
void gaGetAmountOfOpenCLBufferObjectsStub(osSocket& apiSocket);
void gaGetOpenCLBufferObjectDetailsStub(osSocket& apiSocket);
void gaUpdateOpenCLBufferRawDataStub(osSocket& apiSocket);
void gaSetCLBufferDisplayPropertiesStub(osSocket& apiSocket);

// Sub Buffers:
void gaGetOpenCLSubBufferObjectDetailsStub(osSocket& apiSocket);
void gaSetCLSubBufferDisplayPropertiesStub(osSocket& apiSocket);
void gaUpdateOpenCLSubBufferRawDataStub(osSocket& apiSocket);

// Textures:
void gaGetAmountOfOpenCLImageObjectsStub(osSocket& apiSocket);
void gaGetOpenCLImageObjectDetailsStub(osSocket& apiSocket);
void gaUpdateOpenCLImageRawDataStub(osSocket& apiSocket);

// Pipes:
void gaGetAmountOfOpenCLPipeObjectsStub(osSocket& apiSocket);
void gaGetOpenCLPipeObjectDetailsStub(osSocket& apiSocket);

// Command queues:
void gaGetAmountOfCommandQueuesStub(osSocket& apiSocket);
void gaGetCommandQueueDetailsStub(osSocket& apiSocket);
void gaGetAmountOfCommandsInQueueStub(osSocket& apiSocket);
void gaGetAmountOfEventsInQueueStub(osSocket& apiSocket);
void gaGetEnqueuedCommandDetailsStub(osSocket& apiSocket);

// Samplers:
void gaGetAmountOfOpenCLSamplersStub(osSocket& apiSocket);
void gaGetOpenCLSamplerObjectDetailsStub(osSocket& apiSocket);

// Events:
void gaGetAmountOfOpenCLEventsStub(osSocket& apiSocket);
void gaGetOpenCLEventObjectDetailsStub(osSocket& apiSocket);

// OpenCL execution mode:
void gaSetOpenCLOperationExecutionStub(osSocket& apiSocket);

// AMD Performance counters:
void gaGetAMDOpenCLPerformanceCountersValuesStub(osSocket& apiSocket);
void gaActivateAMDOpenCLPerformanceCountersStub(osSocket& apiSocket);
void gaGetOpenCLQueuePerformanceCountersValuesStub(osSocket& apiSocket);

#endif //__CSAPIFUNCTIONSSTUBS_H

