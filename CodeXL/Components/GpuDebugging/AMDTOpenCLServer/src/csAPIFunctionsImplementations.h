//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAPIFunctionsImplementations.h
///
//==================================================================================

//------------------------------ csAPIFunctionsImplementations.h ------------------------------

#ifndef __CSAPIFUNCTIONSIMPLEMENTATIONS_H
#define __CSAPIFUNCTIONSIMPLEMENTATIONS_H

// Forward decelerations:
class osCallStack;
class apCLBuffer;
class apCLSubBuffer;
class apCLCommandQueue;
class apCLContext;
class apCLDevice;
class apCLEnqueuedCommand;
class apCLEvent;
class apCLKernel;
class apCLObjectID;
class apCLPipe;
class apCLProgram;
class apCLSampler;
class apCLImage;
class apKernelSourceCodeBreakpoint;
class apStatistics;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/apSearchDirection.h>

// Function calls:
bool gaGetAmountOfOpenCLFunctionCallsImpl(int contextId, int& amountOfFunctionCalls);
bool gaGetOpenCLFunctionCallImpl(int contextId, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
bool gaGetLastOpenCLFunctionCallImpl(int contextId, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
bool gaFindOpenCLFunctionCallImpl(int contextId, apSearchDirection searchDirection, int searchStartIndex, const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex);
bool gaGetOpenCLHandleObjectDetailsImpl(oaCLHandle handle, const apCLObjectID*& pCLOjbectIDDetails);

// Statistics:
bool gaGetCurrentOpenCLStatisticsImpl(int contextId, apStatistics* pStatistics);
bool gaClearOpenCLFunctionCallsStatisticsImpl();

// Contexts:
bool gaGetAmountOfOpenCLContextsImpl(int& amountOfOpenCLContexts);
bool gaUpdateOpenCLContextDataSnapshotImpl(int contextId);

// Log file recording:
bool gaGetCLContextLogFilePathImpl(int contextId, bool& logFileExists, osFilePath& filePath);

// Programs:
bool gaGetAmountOfOpenCLProgramObjectsImpl(int contextId, int& amountOfPrograms);
bool gaGetOpenCLProgramObjectDetailsImpl(int contextId, int programId, const apCLProgram*& pCLProgramDetails);
bool gaSetOpenCLProgramCodeImpl(oaCLProgramHandle programHandle, const osFilePath& newSourcePath);
bool gaBuildOpenCLProgramImpl(oaCLProgramHandle programHandle, apCLProgram*& pFailedProgramData);
bool gaGetOpenCLProgramHandleFromSourceFilePathImpl(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath, oaCLProgramHandle& programHandle);
bool gaSetKernelSourceFilePathImpl(gtVector<osFilePath>& programsFilePath);

// Kernels:
bool gaGetOpenCLKernelObjectDetailsImpl(oaCLKernelHandle kernelHandle, const apCLKernel*& pCLKernelDetails);

// Kernel debugging:
bool gaGetKernelDebuggingLocationImpl(oaCLProgramHandle& debuggedProgramHandle, int& currentLineNumber);
bool gaGetCurrentlyDebuggedKernelDetailsImpl(const apCLKernel*& pKernelDetails);
bool gaGetCurrentlyDebuggedKernelCallStackImpl(const int coordinate[3], osCallStack& kernelStack);
bool gaSetKernelDebuggingCommandImpl(apKernelDebuggingCommand command);
bool gaSetKernelSteppingWorkItemImpl(const int coordinate[3]);
bool gaIsWorkItemValidImpl(const int coordinate[3]);
bool gaGetFirstValidWorkItemImpl(int wavefrontIndex, int coordinate[3]);
bool gaCanGetKernelVariableValueImpl(const gtString& variableName, const int coordinate[3]);
bool gaGetKernelDebuggingVariableValueStringImpl(const gtString& variableName, const int workItem[3], gtString& variableValue, gtString& variableValueHex, gtString& variableType);
bool gaGetKernelDebuggingVariableMembersImpl(const gtString& variableName, const int coordinate[3], gtVector<gtString>& memberNames);
bool gaGetKernelDebuggingAvailableVariablesImpl(const int coordinate[3], gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth);
bool gaGetKernelDebuggingAmountOfActiveWavefrontsImpl(int& amountOfWavefronts);
bool gaGetKernelDebuggingActiveWavefrontIDImpl(int wavefrontIndex, int& wavefrontId);
bool gaGetKernelDebuggingWavefrontIndexImpl(const int coordinate[3], int& wavefrontIndex);
bool gaUpdateKernelVariableValueRawDataImpl(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableRawDataFilePath);
bool gaGetKernelSourceCodeBreakpointResolutionImpl(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber);
bool gaSetKernelDebuggingEnableImpl(bool kernelEnabled);
bool gaSetMultipleKernelDebugDispatchModeImpl(apMultipleKernelDebuggingDispatchMode mode);

// Devices:
bool gaGetOpenCLDeviceObjectDetailsImpl(int deviceId, const apCLDevice*& pCLDeviceDetails);

// Platforms:
bool gaGetOpenCLPlatformAPIIDImpl(oaCLPlatformID platformId, int& platformName);

// Buffers:
bool gaGetAmountOfOpenCLBufferObjectsImpl(int contextId, int& amountOfBuffer);
bool gaGetOpenCLBufferObjectDetailsImpl(int contextId, int bufferId, const apCLBuffer*& pCLBufferDetails);
bool gaUpdateOpenCLBufferRawDataImpl(int contextId, const gtVector<int>& buffers);
bool gaSetCLBufferDisplayPropertiesImpl(int contextId, int bufferId, oaTexelDataFormat bufferDisplayFormat, int offset, gtSize_t stride);

// Sub Buffers:
bool gaGetOpenCLSubBufferObjectDetailsImpl(int contextId, int subBufferName, const apCLSubBuffer*& pCLSubBufferDetails);
bool gaSetCLSubBufferDisplayPropertiesImpl(int contextId, int subBufferId, oaTexelDataFormat bufferDisplayFormat, int offset, gtSize_t stride);
bool gaUpdateOpenCLSubBufferRawDataImpl(int contextId, const gtVector<int>& subBuffers);

// Texture:
bool gaGetAmountOfOpenCLImageObjectsImpl(int contextId, int& amountOfTexture);
bool gaGetOpenCLImageObjectDetailsImpl(int contextId, int textureId, const apCLImage*& pCLTextureDetails);
bool gaUpdateOpenCLImageRawDataImpl(int contextId, const gtVector<int>& textures);

// Pipes:
bool gaGetAmountOfOpenCLPipeObjectsImpl(int contextId, int& amountOfPipes);
bool gaGetOpenCLPipeObjectDetailsImpl(int contextId, int pipeId, const apCLPipe*& pCLPipeDetails);

// Command Queues:
bool gaGetAmountOfCommandQueuesImpl(int contextId, int& amountOfQueues);
bool gaGetCommandQueueDetailsImpl(int contextId, int queueIndex, const apCLCommandQueue*& pCLCommandQueue);
bool gaGetAmountOfCommandsInQueueImpl(oaCLCommandQueueHandle queueHandle, int& amountOfCommands);
bool gaGetAmountOfEventsInQueueImpl(oaCLCommandQueueHandle queueHandle, int& amountOfEvents);
bool gaGetEnqueuedCommandDetailsImpl(oaCLCommandQueueHandle queueHandle, int commandIndex, const apCLEnqueuedCommand*& pCommand);

// Samplers:
bool gaGetAmountOfOpenCLSamplersImpl(int contextId, int& amountOfSamplers);
bool gaGetOpenCLSamplerObjectDetailsImpl(int contextId, int queueIndex, const apCLSampler*& pCLSampler);

// Events:
bool gaGetAmountOfOpenCLEventsImpl(int contextId, int& amountOfEvents);
bool gaGetOpenCLEventObjectDetailsImpl(int contextId, int queueIndex, const apCLEvent*& pCLEvent);

// Context:
bool gaGetOpenCLContextDetailsImpl(int contextId, apCLContext& contextInfo);

// OpenCL execution mode:
bool gaSetOpenCLOperationExecutionImpl(apOpenCLExecutionType executionType, bool isExecutionOn);

// Performance counters:
bool gaGetAMDOpenCLPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues);
bool gaActivateAMDOpenCLPerformanceCountersImpl(const gtVector<apCounterActivationInfo>& countersActivationInfosVec);
bool gaGetOpenCLQueuePerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues);


#endif //__CSAPIFUNCTIONSIMPLEMENTATIONS_H


