//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suStringConstants.h
///
//==================================================================================

//------------------------------ suStringConstants.h ------------------------------

#ifndef __SUSTRINGCONSTANTS_H
#define __SUSTRINGCONSTANTS_H

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Calls history log file related strings:
#define SU_STR_logFileCreated L"Log file created: "
#define SU_STR_startStringMarkerInHTMLLog L"<br><div style=\"background-color: FFFF70;\"><br><b>Marker: "
#define SU_STR_endStringMarkerInHTMLLog L"</b><br><br></div><br>"
#define SU_STR_maxLoggedFunctionsAmountReached L"Maximal amount of logged function calls (%d) was exceeded.\nPlease select suitable Frame Terminator functions (Debug Settings dialog)\n or increase the maximal logged functions amount (Options dialog)."
#define SU_STR_notEnoughMemoryForLoggingFunctions L"There is not enough memory for logging function calls.\nThe debugged program issued %d function calls without calling a function marked as a frame terminator.\nPlease select suitable Frame Terminator functions (Debug Settings dialog)\nor decrease the maximal logged functions amount (Options dialog)."
#define SU_STR_variableRawFileNamePrefix L"VariableMultiWatch-"

// Null context calls history logger:
#define SU_STR_nullContextCallsHistoryLoggerMessagesLabel L"No Context: "

// Breakpoints:
#define SU_STR_triggeringBreakpointException L"Triggering breakpoint exception"
#define SU_STR_passedBreakpointException L"Execution continued after breakpoint exception was handled"

// Mac OS X Interception
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    #define SU_DebugLog_interceptedPointerFormat GT_32_BIT_POINTER_FORMAT_UPPERCASE
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #define SU_DebugLog_interceptedPointerFormat GT_64_BIT_POINTER_FORMAT_UPPERCASE
#else
    #error Unknown address space size!
#endif

// Debug log printouts:
#define SU_STR_FailedToGetDebugLogSeverity L"Failed to get the debug log severity (is spy standalone mode being used?)"
#define SU_STR_DebugLog_APIFunctionCalled L"API function called: "
#define SU_STR_DebugLog_APIFuncHandlingEnded L"API function handling ended: "
#define SU_STR_debugLogFileName L"CodeXLServers"
#define SU_STR_debugLogServersUtilitiesDescription L"CodeXL debugging servers"
#define SU_STR_DebugLogCreatingNullContext L"Creating default context object"
#define SU_STR_DebugLog_replacingFunctionInstructions1 L"Initializing interception of function ";
#define SU_STR_DebugLog_replacingFunctionInstructions2 L" - src: " SU_DebugLog_interceptedPointerFormat L" "    // Source (real function) address
#define SU_STR_DebugLog_replacingFunctionInstructions3 L"dst: " SU_DebugLog_interceptedPointerFormat L". "  // Destination (wrapper function) address
#define SU_STR_DebugLog_replacingFunctionInstructions4 L"org:"                                              // Original instructions
#define SU_STR_DebugLog_replacingFunctionInstructions5 L" new:"                                             // New instructions (JMP/LDR command)
#define SU_STR_DebugLog_functionWrapperAndImplAreSame1 L"Could not initialize interception of function "
#define SU_STR_DebugLog_functionWrapperAndImplAreSame2 L" (at address " SU_DebugLog_interceptedPointerFormat L") due to missing information"
#define SU_STR_DebugLog_couldNotChangeVMAddressRWPermissions L"Problem with interception due to missing access rights"
#define SU_STR_DebugLog_registeringTwoHandlersForAnAPIFunction L"Error: trying to register two handlers for an API function: "
#define SU_STR_DebugLog_cannotFindAPIFunctionStub L"Error: cannot find API function stub (API function "
#define SU_STR_DebugLog_failedToReadAPISharedObjName L"Failed to read API shared memory object name"
#define SU_STR_DebugLog_failedToReadTCPIPEnvVariables L"Failed to read TCP/IP connection environment variables"
#define SU_STR_DebugLog_failedToReadDebuggedApplicationName L"Failed to read the debugged application name"
#define SU_STR_DebugLog_notRunningUnderDebuggedApp L"Notice: This application is not CodeXL's debugged application (debugged application = %ls, this application = %ls)"
#define SU_STR_DebugLog_ServersAreInitializedInStandaloneMode L"CodeXL Servers are initialized in stand-alone mode"
#define SU_STR_DebugLog_ServersAreInitializedInAPIMode L"CodeXL's servers are initialized in API mode"
#define SU_STR_DebugLog_SpyUtilitiesInitializedSuccessfully L"The SpyUtilities module was initializes successfully"
#define SU_STR_DebugLog_SpyUtilitiesInitializationFailed L"The Spies Utilities module initialization has failed. The debugged application will now exit"
#define SU_STR_DebugLog_APIConnectionWasEstablished L"Servers API connection was established"
#define SU_STR_DebugLog_APIThreadId L"CodeXL Servers API thread id is: "
#define SU_STR_DebugLog_raisingDirectFuncExecutionFlag L"Raising direct function execution flag"
#define SU_STR_DebugLog_loweringDirectFuncExecutionFlag L"Lowering thread execution flag"
#define SU_STR_DebugLog_FailedToTerminateAPI L"Failed to terminate the Spies API connection"
#define SU_STR_DebugLog_redirectedStdoutMessage L"Redirected debugged process's stdout to file "
#define SU_STR_DebugLog_reportingDebuggedProcessTermination L"Reporting debugged process termination"
#define SU_STR_DebugLog_enteringDebuggedProcessTerminationAPILoop L"Entering debugged process termination API loop"
#define SU_STR_DebugLog_exitDebuggedProcessTerminationAPILoop L"Exit debugged process termination API loop"
#define SU_STR_DebugLog_startedHandlingAPIInitCalls L"Started handling Spies Utilities API initialization calls"
#define SU_STR_DebugLog_endeddHandlingAPIInitCalls L"Ended handling Spies Utilities API initialization calls"
#define SU_STR_DebugLog_apiThreadStartedListening L"Servers API thread started listening to API calls"
#define SU_STR_DebugLog_apiThreadEndedListening L"Servers API thread ended listening to API calls"
#define SU_STR_DebugLog_waitingForDirectFunctionExecution L"API loop is waiting until a direct function execution is over"
#define SU_STR_DebugLog_finishedWaitingForDirectFunctionExecution L"API loop finished waiting for the direct function execution"
#define SU_STR_DebugLog_apiThreadQuitsDueToProcessTermination L"API Thread is quitting the API loop due to debugged process termination"
#define SU_STR_DebugLog_resumeCommandDuringTermination L"Resume command was received during debugged process termination"
#define SU_STR_DebugLog_apiThreadIsWaitingForServersAPIInitialization L"The API thread is waiting for the Servers API to initialize"
#define SU_STR_DebugLog_apiThreadFinishedWaitingForServersAPIInitialization L"The API thread finished waiting for the Servers API to initialize"
#define SU_STR_DebugLog_apiThreadEnteredAPIInitializationCS L"The API thread entered the API connection initialization critical section"
#define SU_STR_DebugLog_apiThreadLeftAPIInitializationCS L"The API thread left the API connection initialization critical section"
#define SU_STR_DebugLog_mainThreadEnteredAPIInitializationCS L"The main thread entered the API connection initialization critical section"
#define SU_STR_DebugLog_mainThreadLeftAPIInitializationCS L"The main thread left the API connection initialization critical section"
#define SU_STR_DebugLog_APIInitCallsAreHandledByMainThread L"API initialization calls are handled by the main thread"
#define SU_STR_DebugLog_APIInitCallsMainThreadHandlingEnded L"API initialization calls main thread handling ended"
#define SU_STR_DebugLog_exceptionWhileExecutingDirectFunction L"Exception while executing direct function"
#define SU_STR_DebugLog_DebuggedApplicationRanOutOfMemory L"Debugged application ran out of memory"

// General strings:
#define SU_STR_unknownApplicationName L"Unknown Application"
#define SU_STR_rawFileExtension L"grw"
#define SU_STR_CodeXLError L"CodeXL error"
#define SU_STR_communicationFailedMessage L"Error: Communication between the debugged application and CodeXL has failed. "
#define GS_STR_SpiesUtilitiesInitializationFailureMessage L"Error: CodeXL failed to attach to the debugged application (%ls)\nThe debugged application will now exit."
#define SU_STR_ServerRunsWithNoDebuggerMessage1 L"Error: A Server supplied with CodeXL (opengl32.dll or OpenCL.dll) was loaded into the \""
#define SU_STR_ServerRunsWithNoDebuggerMessage2 L"\" application process, but the \""
#define SU_STR_ServerRunsWithNoDebuggerMessage3 L"\" application was not launched using CodeXL!\n\nDid you copy a CodeXL Server into the \""
#define SU_STR_ServerRunsWithNoDebuggerMessage4 L"\" application directory?"

// Null context calls history logger:
#define SU_STR_callsLogFilePath L"NoContext-CallsLog"

// Assertion failures:
#define SU_STR_UnsupportedParamIndex L"Error: Trying to retrieve a parameter of index: "

// Memory leaks:
#define SU_STR_MemoryLeakSearchingForMemoryLeaksOnCLContextDeletion L"Checking for memory leaks - OpenCL Context %d deleted"
#define SU_STR_MemoryLeakSearchingForMemoryLeaksOnGLContextDeletion L"Checking for memory leaks - OpenGL Context %d deleted"
#define SU_STR_MemoryLeakSearchingForMemoryLeaksOnCLProgramDeletion L"Checking for memory leaks - OpenCL Context %d Program %d deleted"
#define SU_STR_MemoryLeakSearchingForCLMemoryLeaksOnApplicationExit L"Checking for OpenCL memory leaks - Application exit"
#define SU_STR_MemoryLeakSearchingForGLMemoryLeaksOnApplicationExit L"Checking for OpenGL memory leaks - Application exit"

// Spy progress:
#define SU_STR_SpyIsInProgress L"Spy Is In Progress: %d"

// CL Build configuration:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define SU_STR_kernelDebuggingForcedBuildOptionsASCII "-g -O0 -mem2reg=0 -cl-opt-disable -scopt=3 -fbin-source"
    #define SU_STR_kernelDebuggingForcedBuildOptions L"-g -O0 -mem2reg=0 -cl-opt-disable -scopt=3 -fbin-source"
#elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
    #define SU_STR_kernelDebuggingForcedBuildOptionsASCII "-g -O0 -mem2reg=0 -cl-opt-disable -scopt=3"
    #define SU_STR_kernelDebuggingForcedBuildOptions L"-g -O0 -mem2reg=0 -cl-opt-disable -scopt=3"
#else
    #error unknown build configuration!
#endif

#define SU_STR_kernelDebuggingForcedBuildOptionLegacyASCII " -legacy"
#define SU_STR_kernelDebuggingForcedBuildOptionLegacy L" -legacy"

#endif //__SUSTRINGCONSTANTS_H

