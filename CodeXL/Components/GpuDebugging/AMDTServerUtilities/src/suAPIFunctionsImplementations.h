//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIFunctionsImplementations.h
///
//==================================================================================

//------------------------------ suAPIFunctionsImplementations.h ------------------------------

#ifndef __SUAPIFUNCTIONSIMPLEMENTATIONS_H
#define __SUAPIFUNCTIONSIMPLEMENTATIONS_H

// Forward decelerations:
class osCallStack;
class osPortAddress;
class osRawMemoryBuffer;
class apApiFunctionsInitializationData;
class apContextID;
struct apDetectedErrorParameters;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTOSWrappers/Include/osThread.h>

// API package:
bool gaIntializeAPIImpl(const apApiFunctionsInitializationData& initData);
osThreadId gaGetAPIThreadIdImpl();

// Debugged process:
void gaBeforeTerminateDebuggedProcessImpl();
void gaTerminateDebuggedProcessImpl();
bool gaSuspendDebuggedProcessImpl();
bool gaResumeDebuggedProcessImpl();
bool gaSuspendThreadsImpl(const std::vector<osThreadId>& thrds);
bool gaResumeThreadsImpl();

// Debugged process threads:
bool gaGetCurrentThreadCallStackImpl(osCallStack& threadCallStack, bool hideSpyFunctions);

// Debugged process execution mode:
bool gaSetDebuggedProcessExecutionModeImpl(apExecutionMode executionMode);

// Breakpoints:
bool gaSetBreakpointImpl(const apBreakPoint& breakpoint);
bool gaRemoveBreakpointImpl(const apBreakPoint& breakpoint);
bool gaRemoveAllBreakpointsImpl();
bool gaBreakOnNextMonitoredFunctionCallImpl();
bool gaBreakOnNextDrawFunctionCallImpl();
bool gaBreakOnNextFrameImpl();
bool gaBreakInMonitoredFunctionCallImpl(bool& replacedWithStepOver);
bool gaClearAllStepFlagsImpl();
bool gaGetDetectedErrorParametersImpl(apDetectedErrorParameters& detectedErrorParameters);
bool gaGetBreakReasonImpl(apBreakReason& breakReason);
bool gaGetBreakpointTriggeringContextIdImpl(apContextID& contextId);

// Event forwarding:
bool gaCreateEventForwardingTCPConnectionImpl(const osPortAddress& portAddress);
bool gaCreateEventForwardingPipeConnectionImpl(const gtString& eventsPipeName);

// Allocated objects
bool gaGetAmountOfRegisteredAllocatedObjectsImpl(unsigned int& amountOfAllocatedObjects);
bool gaGetAllocatedObjectCreationStackImpl(int allocatedObjectId, osCallStack& callsStack);
bool gaCollectAllocatedObjectsCreationCallsStacksImpl(bool collectCreationStacks);

// Sending files through the API pipe:
bool gaReadFileImpl(const osFilePath& filePath, osRawMemoryBuffer& memoryBuffer);
bool gaWriteFileImpl(const osFilePath& filePath, const osRawMemoryBuffer& memoryBuffer);

// Slow motion:
bool gaSetSlowMotionDelayImpl(int delayTimeUnits);

// Log file recording:
bool gaStartMonitoredFunctionsCallsLogFileRecordingImpl();
bool gaStopMonitoredFunctionsCallsLogFileRecordingImpl();
bool gaIsMonitoredFunctionsCallsLogFileRecordingActiveImpl(bool& isActive);
bool gaFlushLogFileAfterEachFunctionCallImpl(bool flushAfterEachFunctionCall);

// Textures:
bool gaEnableImagesDataLoggingImpl(bool isTexturesImageDataLogged);

#endif //__SUAPIFUNCTIONSIMPLEMENTATIONS_H

