//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIFunctionsStubs.h
///
//==================================================================================

//------------------------------ suAPIFunctionsStubs.h ------------------------------

#ifndef __SUAPIFUNCTIONSSTUBS_H
#define __SUAPIFUNCTIONSSTUBS_H

// Forward decelerations:
class osSocket;

// API management:
void suRegisterAPIStubFunctions();

// API package:
void gaIntializeAPIStub(osSocket& apiSocket);
void gaIntializeAPIEndedStub(osSocket& apiSocket);
void gaGetAPIThreadIdStub(osSocket& apiSocket);

// Debugged process:
void gaTerminateDebuggedProcessStub(osSocket& apiSocket);
void gaSuspendDebuggedProcessStub(osSocket& apiSocket);
void gaResumeDebuggedProcessStub(osSocket& apiSocket);

// Debugged process threads:
void gaMakeThreadGetCallStackStub(osSocket& apiSocket);
void gaSuspendThreadsStub(osSocket& apiSocket);
void gaResumeThreadsStub(osSocket& apiSocket);
bool gaGetCurrentThreadCallStackInternalStub();

// Debugged process execution mode:
void gaSetDebuggedProcessExecutionModeStub(osSocket& apiSocket);

// Breakpoints
void gaSetBreakpointStub(osSocket& apiSocket);
void gaRemoveBreakpointStub(osSocket& apiSocket);
void gaRemoveAllBreakpointsStub(osSocket& apiSocket);
void gaBreakOnNextMonitoredFunctionCallStub(osSocket& apiSocket);
void gaBreakOnNextDrawFunctionCallStub(osSocket& apiSocket);
void gaBreakOnNextFrameStub(osSocket& apiSocket);
void gaBreakInMonitoredFunctionCallStub(osSocket& apiSocket);
void gaClearAllStepFlagsStub(osSocket& apiSocket);
void gaGetDetectedErrorParametersStub(osSocket& apiSocket);
void gaGetBreakReasonStub(osSocket& apiSocket);
void gaGetBreakpointTriggeringContextIdStub(osSocket& apiSocket);

// Event forwarding:
void gaCreateEventForwardingTCPConnectionStub(osSocket& apiSocket);
void gaCreateEventForwardingPipeConnectionStub(osSocket& apiSocket);

// Allocated objects:
void gaGetAmountOfRegisteredAllocatedObjectsStub(osSocket& apiSocket);
void gaGetAllocatedObjectCreationStackStub(osSocket& apiSocket);
void gaCollectAllocatedObjectsCreationCallsStacksStub(osSocket& apiSocket);

// Sending files through the API pipe:
void gaReadFileStub(osSocket& apiSocket);
void gaWriteFileStub(osSocket& apiSocket);

// Slow motion:
void gaSetSlowMotionDelayStub(osSocket& apiSocket);

// Log file recording:
void gaStartMonitoredFunctionsCallsLogFileRecordingStub(osSocket& apiSocket);
void gaStopMonitoredFunctionsCallsLogFileRecordingStub(osSocket& apiSocket);
void gaIsMonitoredFunctionsCallsLogFileRecordingActiveStub(osSocket& apiSocket);

// Textures:
void gaEnableImagesDataLoggingStub(osSocket& apiSocket);

// Float parameters display precision:
void gaSetFloatParametersDisplayPrecisionStub(osSocket& apiSocket);

// Enable FlushLogFileAfterEachFunctionCall:
void gaFlushLogFileAfterEachFunctionCallStub(osSocket& apiSocket);


#endif //__SUAPIFUNCTIONSSTUBS_H

