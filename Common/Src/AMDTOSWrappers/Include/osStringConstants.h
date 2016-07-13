//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osStringConstants.h
///
//=====================================================================

//------------------------------ osStringConstants.h ------------------------------

#ifndef __OSSTRINGCONSTANTS
#define __OSSTRINGCONSTANTS

// Log special character:
#define OS_STR_DebugLogDelimiter L"\t#"
#define OS_STR_OptionsAdvancedDebugLogLevelUnknown L"Unknown"
#define OS_STR_OptionsAdvancedDebugLogLevelError L"Error"
#define OS_STR_OptionsAdvancedDebugLogLevelInfo L"Info"
#define OS_STR_OptionsAdvancedDebugLogLevelDebug L"Debug"
#define OS_STR_OptionsAdvancedDebugLogLevelExtensive L"Extensive"
#define OS_STR_DebugLogNewLine L"\n"

// Constants:
#define OS_32_BIT_ADDRESS_SPACE_AS_STR L"32"
#define OS_64_BIT_ADDRESS_SPACE_AS_STR L"64"
#define OS_ITANIUM_64_BIT_ADDRESS_SPACE_AS_STR L"IA64"

// Linux variant names:
#define OS_STR_linuxVariantUbuntu L"Ubuntu"

// Debug log:
#define OS_STR_DebugLogIsReInitialized L"Debug log is re-initialized. New log file path is: "
#define OS_STR_DebugLogIsTerminated L"Debug log is terminated"
#define OS_STR_DebugLogHeaderBorder L"////////////////////////////////////////////////////////////\n"
#define OS_STR_DebugLogHeader L"This File contains debug printouts for CodeXL. http://gpuopen.com/"
#define OS_STR_UnknownApplication L"Unknown Application"
#define OS_STR_FailedToDeleteSharedMemFile L"Failed to delete shared memory file: "
#define OS_STR_FailedToSetEnvVariable L"Failed to set environment variable: "
#define OS_STR_FailedToRemoveEnvVariable L"Failed to remove environment variable: "
#define OS_STR_ReadOperationTimeOut L"Read operation timeout was reached. Are you debugging in DevStudio?"
#define OS_STR_FailedToLoadModule L"Failed to load module. OS error is: "
#define OS_STR_FailedToLaunch L"Failed to launch file: "
#define OS_STR_bindError L"Bind error"
#define OS_STR_readError L"Read error (pipe type : %ls)"
#define OS_STR_writeError L"Write error"
#define OS_STR_host L" Host: "
#define OS_STR_port L" Port: "
#define OS_STR_timeoutReached L"Timeout reached"
#define OS_STR_pipeReadError L"Pipe read error (pipe type: %ls)"
#define OS_STR_pipeReadMoreDataThenAskedFor L"Pipe read error - we read more data then requested (pipe type: %ls)"
#define OS_STR_pipeReadErrorLastMessage L"Pipe read error. This is the last logged error message (pipe type: %ls)"
#define OS_STR_pipeWriteError L"Pipe write error (pipe type: %ls)"
#define OS_STR_pipeCannotClose L"Cannot close pipe (pipe type: %ls)"
#define OS_STR_pipeWriteErrorLastMessage L"Pipe write error. This is the last logged error message (pipe type: %ls)"
#define OS_STR_pipeException L"Pipe exception"
#define OS_STR_osReportedErrorIs L"  OS reported error is: "
#define OS_STR_socketException L"Socket exception"
#define OS_STR_noSystemError L"No system error was recorded"
#define OS_STR_unknownSystemError L"Unknown system error"
#define OS_STR_unknown L"Unknown"
#define OS_STR_desktopTypeIs L"Desktop type is: "
#define OS_STR_cannotGetDesktopType L"Unable to retrieve desktop type"
#define OS_STR_NotAvailable L"N/A"

#define OS_STR_cannotAccessSettingsFile L"Cannot access settings file: "
#define OS_STR_FailedToLoadedModuleDebugInfo L"Failed to load module debug info: "
#define OS_STR_FailedToUnloadedModuleDebugInfo L"Failed to unload module debug info: "
#define OS_STR_FailedToAllocateMemory L"Failed to allocate memory"
#define OS_STR_OSThreadWasCreated L"An osThread thread of type %ls was created. Thread id is: "
#define OS_STR_OSThreadRunStarted L"An osThread thread of type %ls started running. Thread id is: "
#define OS_STR_OK L"OK"
#define OS_STR_DELAYED_DEBUG_PRINTOUT_PREFIX L"Delayed debug printout: "
#define OS_STR_DebugLogSeverity L"Debug log severity: %ls"
#define OS_STR_iPhoneConsolePrintoutPrefix L"CodeXL OpenGL ES Server message: "
#define OS_STR_ReleaseVersion L"Release Version"
#define OS_STR_DebugVersion L"Debug Version"
#define OS_STR_Redirection_File_missing L"Redirection token appears but file name missing "

// HTTP Errors (ANSII strings):
#define OS_STR_cannotEndTCPSession "Could not end existing TCP/IP session"
#define OS_STR_cannotOpenTCPSocket "Could not open TCP socket"
#define OS_STR_connectedToSMTPServer L"Connected to server %ls by SMTP over port %d"
#define OS_STR_cannotConnectToTCPServer1 "Could not connect over TCP/IP to server \'"
#define OS_STR_cannotConnectToTCPServer2 "\' over port %u"
#define OS_STR_requestError "Request error"
#define OS_STR_serverRespondError "Server response error"
#define OS_STR_packetLoss "Packet Loss"
#define OS_STR_unknownError "Unknown error"


// Run time strings:
#define OS_STR_debuggedProcessIsTerminating L"Debugged process is terminating. New API thread id is: "

// Perror printouts:
#define OS_STR_FAILED_TO_ADD_DEBUG_MSG_TO_QUEUE L"Failed to add message to the pending debug printouts queue"
#define OS_STR_FAILED_TO_RETRIEVE_DEBUG_MSG_FROM_QUEUE L"Failed to retrieve messages from the pending debug printouts queue"

// Call stack:
#define OS_STR_callStackAsStringHeader L"Function name - File path - Line number - Module path - Function Start address - Module Start address - Instruction counter address\n"

// Environment variables:
#define OS_STR_envVar_spiesDirectory L"SU_SPIES_DIRECTORY"
#define OS_STR_envVar_debugLogSeverity L"SU_DEBUG_LOG_SEVERITY"
#define OS_STR_envVar_debuggedAppName L"SU_DEBUGGED_APPLICATION_NAME"
#define OS_STR_envVar_debuggedAppNameOverride L"SU_DEBUGGED_APPLICATION_NAME_OVERRIDE"
#define OS_STR_envVar_APIPipeName L"SU_API_PIPE_NAME"
#define OS_STR_envVar_stdoutRedirectionFile L"SU_STDOUT_REDIRECT"
#define OS_STR_envVar_serverConnectionIPHostname L"SU_SERVER_CONNECTION_IP_HOSTNAME"
#define OS_STR_envVar_serverConnectionIPPort L"SU_SERVER_CONNECTION_IP_PORT"
#define OS_STR_envVar_stdoutRedirectionFile L"SU_STDOUT_REDIRECT"
#define OS_STR_envVar_initializePerformanceCounters L"SU_INITIALIZE_PERFORMANCE_COUNTERS"
#define OS_STR_envVar_suppressSpyEvents L"SU_SUPPRESS_SPY_EVENTS"
#define OS_STR_envVar_valueTrue L"TRUE"
#define OS_STR_envVar_valueFalse L"FALSE"

// Additional Environment variables used to control standalone mode:
#define OS_STR_envVar_debuggerInstallDir L"SU_DEBUGGER_INSTALL_DIR"
#define OS_STR_envVar_systemModulesDirs L"SU_SYSTEM_MODULES_DIRS"
#define OS_STR_envVar_suSystemOpenGLModulePath L"SU_SYSTEM_OPENGL_MODULE_PATH"
#define OS_STR_envVar_logFilesPath L"SU_LOG_FILES_PATH"
#define OS_STR_envVar_setAPIFunctionBreakpoint L"SU_SET_API_FUNCTION_BREAKPOINT"
#define OS_STR_envVar_setKernelNameBreakpoint L"SU_SET_KERNEL_NAME_BREAKPOINT"
#define OS_STR_envVar_disableKernelDebugging L"SU_DISABLE_KERNEL_DEBUGGING"
#define OS_STR_envVar_gsDontForceOpenGLDebugContexts L"SU_GS_DONT_FORCE_OPENGL_DEBUG_CONTEXTS"
#define OS_STR_envVar_csDontAddDebuggingBuildFlags L"SU_CS_DONT_ADD_DEBUGGING_BUILD_FLAGS"
#define OS_STR_envVar_hdForceResetOfKernelDebugging L"SU_HD_FORCE_RESET_OF_KERNEL_DEBUGGING"
#define OS_STR_envVar_suDontFixCRInSourceStrings L"SU_DONT_FIX_CARRIAGE_RETURN_IN_SOURCES"
#define OS_STR_envVar_hsAlwaysDebug L"HS_DEBUG_HSA_KERNELS_ALWAYS"
#define OS_STR_envVar_hsNeverDebug L"HS_DEBUG_HSA_KERNELS_NEVER"

// General errors:
#define OS_GEN_ERR_FUNCTION_NOT_IMPLEMENTED L"Function not implemented!"

// TCP-Related errors:
#define OS_TCP_ERR_FAILED_TO_GET_HOSTNAME               L"Failed to get host name."
#define OS_TCP_ERR_FAILED_TO_GET_HOSTENT_FROM_HOSTNAME  L"Failed to get hostent structure from host name."

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #define OS_STR_32BitDirectoryName L"x86"
    #define OS_STR_64BitDirectoryName L"x64"
#else
    #define OS_STR_32BitDirectoryName L"x86"
    #define OS_STR_64BitDirectoryName L"x86_64"
#endif

// Run time directory
#define OS_STR_runTimeDirectory L"RuntimeLibs"

// path related strings
#define OS_STR_CodeXLMacBundleContentsDirName L"Contents"
#define OS_STR_CodeXLMacBundleMacOSDirName L"MacOS"
#define OS_STR_CodeXLExmaplesDirName L"examples"
#define OS_STR_CodeXLCodeXLExmaplesDirName L"CodeXL"
#define OS_STR_CodeXLDataDirName L"Data"
#define OS_STR_CodeXLMacBundleResourcesDirName L"Resources"
#define OS_STR_CodeXLMacTeapotBundleName L"GRTeapot.app"
#define OS_STR_CodeXLLinuxHelpDirName L"webhelp"
#define OS_STR_CodeXLLinuxHelpFilesIndexName L"index"
#define OS_STR_CodeXLLinuxHelpFilesIndexExtension L"html"
#define OS_STR_CodeXLSampleSourcesDirName L"src"
#define OS_STR_CodeXLLinuxTutorialDirName L"tutorial"
#define OS_STR_CodeXLWindowsHelpFilesDirName L"Help"
#define OS_STR_CodeXLWindowsHelpFileNameNoEXT L"CodeXL_User_Guide"
#define OS_STR_CodeXLWindowsHelpFilesExtension L"chm"
#define OS_STR_CodeXLVSPackageWindowsTutorialFileName L"CodeXLVisualStudioTutorial"
#define OS_STR_CodeXLTeapotExampleDirName L"Teapot"
#define OS_STR_CodeXLD3D12MultithreadingExampleDirName L"D3D12Multithreading"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define OS_STR_CodeXLTeapotExampleSourceDirName L"AMDTTeaPot"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define OS_STR_CodeXLTeapotExampleSourceDirName L"AMDTTeaPotLib/src"
#endif
#define OS_STR_CodeXLTeapotExampleLibSourceDirName L"AMDTTeaPotLib"
#define OS_STR_CodeXLMatMulExampleDirName L"ClassicMatMul"
#define OS_STR_CodeXLWindowsQuickStartFileName L"CodeXL_Quick_Start_Guide"
#define OS_STR_CodeXLWindowsQuickStartFileExtension L"pdf"
#endif  // __OSSTRINGCONSTANTS
