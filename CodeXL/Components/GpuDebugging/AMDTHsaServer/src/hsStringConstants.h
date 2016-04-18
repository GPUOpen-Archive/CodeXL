//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsStringConstants.h
///
//==================================================================================

#ifndef __HSSTRINGCONSTANTS_H
#define __HSSTRINGCONSTANTS_H

// Debug log printouts:
#define HS_STR_debugLogInitModuleStart L"HSA Server module initialization started"
#define HS_STR_debugLogInitModuleEnd L"HSA Server module initialization ended"
#define HS_STR_debugLogUninitModuleStart L"HSA Server module deinitialization started"
#define HS_STR_debugLogUninitModuleEnd L"HSA Server module deinitialization ended"
#define HS_STR_debugLogInitHSAMonitor L"Initializing HSA monitor"
#define HS_STR_debugLogHSAMonitorTerminationAlertStart L"hsHSAMonitor::onDebuggedProcessTerminationAlert: Stopping kernel run"
#define HS_STR_debugLogHSAMonitorTerminationAlertEnd L"hsHSAMonitor::onDebuggedProcessTerminationAlert completed"
#define HS_STR_debugLogComparingISAAndAQL L"Comparing HSAIL kernel ISA code with AQL packet: "
#define HS_STR_debugLogCheckingELFSym L"Checking HSAIL code object ELF symbol for ISA binaries: "
#define HS_STR_debugLogFoundISAMatch L"Found HSAIL kernel ISA match!"
#define HS_STR_debugLogUsingFirstKernel L"Using first HSAIL kernel name: "
#define HS_STR_debugLogEnteredPredispatch L"hsPreDispatchCallback() started"
#define HS_STR_debugLogStartingDebugging L"Starting debugging HSAIL kernel "
#define HS_STR_debugLogNotStartingDebugging L"Not debugging HSAIL kernel "
#define HS_STR_debugLogExitedPredispatch L"hsPreDispatchCallback() ended"
#define HS_STR_debugLogEnteredPostdispatch L"hsPostDispatchCallback() started"
#define HS_STR_debugLogExitedPostdispatch L"hsPostDispatchCallback() ended"
#define HS_STR_debugLogInterceptingQueueDispatch L"Intercepting Queue %p dispatch. Success: %c"
#define HS_STR_debugLogSpecifiedQueueSizeTooSmall L"Specified queue size (%#x) is too small for debugging. Resizing to %#x."
#define HS_STR_debugLogFailQueueCreationWithModifiedSize L"Queue creation with modified size failed. Queue will not be intercepted."
#define HS_STR_debugLogApplyingBuildFlags L"Applying debug build flags to program %p. Original options: \"%ls\". Modified options: \"%ls\""
#define HS_STR_debugLogFailProgramBuildWithFlags L"Program build with debug flags failed. Program will not be debuggable."
#define HS_STR_debugLogHSAOnLoad L"HSA OnLoad callback function called"
#define HS_STR_debugLogHSAOnUnload L"HSA OnUnload callback function called"
#define HS_STR_debugLogStartingDebuggingStart L"hsDebuggingManager::StartDebugging started"
#define HS_STR_debugLogStartingDebuggingEntered L"hsDebuggingManager::StartDebugging critical section entered"
#define HS_STR_debugLogStartingDebuggingEnd L"hsDebuggingManager::StartDebugging ended"
#define HS_STR_debugLogInitDebuggingMgr L"Initializing HSAIL debugging manager"
#define HS_STR_debugLogDebugInfoVarsList L"Listing HSAIL variables: "
#define HS_STR_debugLogDebugInfoVarValue L" HSAIL variable value evaluated. Success: %c, value: "
#define HS_STR_debugLogDebugEventThreadCreated L"hsDebuggingManager::hsDebugEventThread created"
#define HS_STR_debugLogDebugEventThreadDestroyed L"hsDebuggingManager::hsDebugEventThread destroyed"
#define HS_STR_debugLogDebugEventThreadStartedRunning L"hsDebuggingManager::hsDebugEventThread started running"
#define HS_STR_debugLogDebugEventThreadFinishedRunning L"hsDebuggingManager::hsDebugEventThread finished running"
#define HS_STR_debugLogDebugEventThreadBeforeTermination L"hsDebuggingManager::hsDebugEventThread before termination"
#define HS_STR_debugLogDebugEventThreadStopDebuggingKillAllFailed L"hsDebuggingManager::hsDebugEventThread dispatch waves kill attempt %d/%d failed. Error: %#x"
#define HS_STR_debugLogDebugEventThreadWaitingForEvent L"hsDebuggingManager::hsDebugEventThread waiting for debug event"
#define HS_STR_debugLogDebugEventThreadGotEvent L"hsDebuggingManager::hsDebugEventThread got debug event"
#define HS_STR_debugLogDebugEventThreadContinueEvent L"hsDebuggingManager::hsDebugEventThread continuing debug event"
#define HS_STR_debugLogDebugEventThreadWaitingForCommand L"hsDebuggingManager::hsDebugEventThread before breakpoint"
#define HS_STR_debugLogDebugEventThreadGotCommand L"hsDebuggingManager::hsDebugEventThread after breakpoint"
#define HS_STR_debugLogDebugEventThreadWaitingForTermination0 L"hsDebuggingManager::Cleanup waiting for debugging thread"
#define HS_STR_debugLogDebugEventThreadWaitingForTermination1 L"hsDebuggingManager::Cleanup stopping thread"
#define HS_STR_debugLogDebugEventThreadWaitingForTermination2 L"hsDebuggingManager::Cleanup forcing thread termination"
#define HS_STR_debugLogCouldNotGetPrivateMemory L"Error getting HSAIL kernel scratch memory: %#x"

// Source files
#define HS_STR_hsailSourceFileExtension L"hsail"
#define HS_STR_hsailSourceFileExtensionWildcard L"*." HS_STR_hsailSourceFileExtension
#define HS_STR_hsailSourceFileNameFormat L"HSAProgram_%04u"

// Debugging errors:
#define HS_STR_debuggingErrorUnknown L"HSAIL Kernel Debugging Error: Unexpected error"
#define HS_STR_debuggingErrorMemory L"HSAIL Kernel Debugging Error: Out of memory"
#define HS_STR_debuggingErrorBinary L"HSAIL Kernel Debugging Error: Could not get kernel binary"
#define HS_STR_debuggingErrorConcurrent L"HSAIL Kernel Debugging Error: Can not debug more than one kernel at a time"
#define HS_STR_debuggingErrorDBE L"HSAIL Kernel Debugging Error: Could not initialize debugger back-end\nMake sure that the latest HSA driver is installed."
#define HS_STR_debuggingErrorDebugInfo L"HSAIL Kernel Debugging Error: Debug information not found\nMake sure that HSAIL compilation to BRIG is done with the \"-g -include-source\" flags."

#define HS_STR_variableInactiveWIPrivateMemory L"Variable is not accessible (inactive work-item private memory)"
#define HS_STR_variableNotFound L"Variable not found"
#define HS_STR_variableEvalError L"Variable evaluation error"

#endif // __HSSTRINGCONSTANTS_H
