//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdStringConstants.h
///
//==================================================================================

//------------------------------ pdStringConstants.h ------------------------------

#ifndef __PDSTRINGCONSTANTS
#define __PDSTRINGCONSTANTS

// Debug log messages:
#define PD_STR_unexpectedEventDuringFunctionExecution L"Error: Got unexpected event during function execution. Event id: "
#define PD_STR_exceptionDuringFunctionExecution L"Error: Got exception during function execution. Exception type:"
#define PD_STR_gdbErrorDuringFunctionExecution L"Error: Got GDB error during function execution."
#define PD_STR_FailedToAddInstallDirToPath L"Failed to add the installation directory to the path"
#define PD_STR_FailedToGetTheCurrentProcessPath L"Failed to get the current process path"
#define PD_STR_FailedToGetLoadedModuleSize L"Failed to get loaded module size: "
#define PD_STR_makeThreadExecuteFunctionStarted L"pdProcessDebugger::makeThreadExecuteFunction started working"
#define PD_STR_makeThreadExecuteFunctionEnded L"pdProcessDebugger::makeThreadExecuteFunction ended"
#define PD_STR_remoteFuncFailed L"Remote thread executed function failed"
#define PD_STR_win32DebuggerError L"A Win32 debugger error occurred"
#define PD_STR_win32DebuggerFatalError L"The Win32 debugger error will fail the debugged process"
#define PD_STR_UnknownDebugEventError L"Error: Recieved an unknown debug event"
#define PD_STR_cannotGetNVIDIADriverSize L"Error: Cannot get the NVIDIA driver size"
#define PD_STR_cannotGetATIDriverSize L"Error: Cannot get the ATI driver size"
#define PD_STR_addressInsideLoadedModule L"Address %p found inside debugged process module "
#define PD_STR_addressIsNotInsideLoadedModule L"Cannot find a module containing the address %p"
#define PD_STR_cannotGetSpySize L"Error: Cannot get the CodeXL OpenGL server size"
#define PD_STR_driverThreadCreation L"A driver thread was created"
#define PD_STR_failedToLaunchGDB L"Error: Failed to launch the gdb debugger"
#define PD_STR_failedToLaunchGDBASCII "Error: Failed to launch the gdb debugger"
#define PD_STR_processRunWasSuspended L"Debugged process run was suspended or breakpoint was hit"
#define PD_STR_afterGDBProcessSuspensionActionsStarts L"afterGDBProcessSuspensionActions starts"
#define PD_STR_afterGDBProcessSuspensionActionsEnds L"afterGDBProcessSuspensionActions ends"
#define PD_STR_resumingDebuggedProcessRunInternally L"Resuming, internally, the debugged process run"
#define PD_STR_brokenPipeSignalReceived L"Broken pipe signal received"
#define PD_STR_unknownSignalReceived L"The debugged process received an unknown signal"
#define PD_STR_gotOGLServerAPIThreadId L"Got OpenGL server API thread id"
#define PD_STR_errorReadingFromGDB L"Error while reading gdb output. OS error is: "
#define PD_STR_debuggedProcessPID L"Debugged process pid is: "
#define PD_STR_debuggedProcessCurrentTID L". current thread id is: %p"
#define PD_STR_parsingGDBOutput L" Parsing GDB output: "
#define PD_STR_endedParsingGDBOutput L"Ended parsing GDB output: "
#define PD_STR_waitingForGDBCommandPrompt L"Waiting for gdb prompt..."
#define PD_STR_gotGDBCommandPrompt L"Got GDB command prompt: "
#define PD_STR_executedGDBCommand L"Executed GDB command: "
#define PD_STR_parsingGDBOutputLine L"Parsing GDB output line: "
#define PD_STR_parsingThreadLine L"Parsing thread line: "
#define PD_STR_parsingCallStack L"Parsing call stack: "
#define PD_STR_parsingCallStackLine L"Parsing call stack line: "
#define PD_STR_failedToParseGDBOutput L"Failed to parse GDB output"
#define PD_STR_waitingForGDBOutputs L"Waiting for GDB outputs ..."
#define PD_STR_endedParsingGDBOutputs L"Ended parsing GDB outputs -  "
#define PD_STR_debuggedProcessWasSuspended L"Debugged process was suspended"
#define PD_STR_debuggedProcessWasTerminated L"Debugged process was terminated"
#define PD_STR_debuggedProcessIsStillRunning L"Debugged process is still running"
#define PD_STR_startedListeningToGDBOutputs L"Started listening to GDB outputs"
#define PD_STR_listenerThreadStartedWaitingForCondition L"pdGDBListenerThread started waiting for condition"
#define PD_STR_listenerThreadEndedWaitingForCondition L"pdGDBListenerThread ended waiting for condition"
#define PD_STR_AskingTheListenerThreadToExit L"Asking pdGDBListenerThread's thread to exit"
#define PD_STR_AskingTheListenerThreadToExitFailed L"Failed to make pdGDBListenerThread's thread to exit"
#define PD_STR_exitingTheListenerThread L"Exiting pdGDBListenerThread's thread"
#define PD_STR_outputReaderThreadOutputFilePrefix L"CXLAppOutput-"
#define PD_STR_outputReaderThreadStartedReading L"Starting to read debugged application's output"
#define PD_STR_gotDebuggedProcessTerminationMessage L"Got debugged process termination message"
#define PD_STR_debuggingAppAddressSpaceSize L"Debugging %ls (%d-bit)"
#define PD_STR_iPhoneOnDeviceSetupCreationScriptInvoked L"Invoked iPhone on-device setup project creation script. Output was: "
#define PD_STR_iPhoneOnDeviceSetupConfigurationScriptInvoked L"Invoked iPhone on-device setup configuration script. Output was: "
#define PD_STR_iPhoneOnDeviceCleanupScriptInvoked L"Invoked iPhone on-device cleanup script. Output was: "
#define PD_STR_iPhoneOnDeviceTemporaryProjectNameSuffix L"-CodeXL-temp"
#define PD_STR_iPhoneOnDeviceProcessCreationFailureScriptFailed L"Automatic configuration failure."
#define PD_STR_RemoteDebuggingStartFailure L"Unable to start remote debugging.\nPlease verify that:\n1. CodeXL and CodeXL Remote Agent are not blocked by a firewall.\n2. The given IP address and ports are valid, and the remote machine is accessible from the local machine.\n3. CodeXL Remote Agent is running on the remote machine."
#define PD_STR_OutputDebugStringMaxPrintoutsReached L"Maximal amount of debug string reached (%d strings). Additional debug strings will be ignored"
#define PD_STR_GDBStringMaxPrintoutsReached L"Maximal amount of GDB strings reached (%d strings). Additional GDB strings will be ignored"
#define PD_STR_DebuggedProcessOutputDebugStringMaxPrintoutsReached L"Maximal amount of debugged process output string reached (%d strings). Additional debugged process output strings will be ignored"
#define PD_STR_AMD_OCL_BUILD_OPTIONS_IgnoredWarning L"AMD_OCL_BUILD_OPTIONS & AMD_OCL_BUILD_OPTIONS_APPEND are ignored"


// Remote process debugger:
#define PD_STR_remoteProcessDebuggerSharedMemoryObject L"AMDTPDSharedMemObj"
#define PD_STR_remoteProcessDebuggerEventsSharedMemoryObject L"AMDTPDEventsSharedMemObj"
#define PD_STR_remoteDebuggingServer64ExecutableFileName L"CXLRemoteDebuggingServer-x64" AMDT_DEBUG_SUFFIX_W AMDT_BUILD_SUFFIX_W

// ProcessDebuggerESLauncher (iPhone launcher):
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define PD_ES_LAUNCHER_MODULE_NAME L"libGRProcessDebuggerESLauncher_d.dylib"
#elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
    #define PD_ES_LAUNCHER_MODULE_NAME L"libGRProcessDebuggerESLauncher.dylib"
#else
    #error Unknown build configuration
#endif
#define PD_LAUNCH_IPHONE_SIMULATOR_WITH_APP_FUNCTION_NAME L"pdLaunchiPhoneSimulatorWithApplication"
// Note: this should be identical to GD_STR_iPhoneSimulatorIsNotInstalled
#define PD_STR_iPhoneSimulatorIsNotInstalled L"The iPhone Simulator application is not installed on the local computer.\nPlease install the iPhone SDK on the local computer to enable iPhone applications debugging."

// Environment variables:
#define PD_STR_AMD_OCL_BUILD_OPTIONS L"AMD_OCL_BUILD_OPTIONS"
#define PD_STR_AMD_OCL_BUILD_OPTIONS_APPEND L"PD_STR_AMD_OCL_BUILD_OPTIONS_APPEND"

#endif  // __PDSTRINGCONSTANTS
