//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suGlobalVariables.h
///
//==================================================================================

//------------------------------ suGlobalVariables.h ------------------------------

#ifndef __SUGLOBALVARIABLES_H
#define __SUGLOBALVARIABLES_H

// Forward decelerations:
class osFilePath;

// Mac OS X-only forward declaration, see suMacOSInterception.cpp
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    struct suFunctionInterceptionInformation;
    #ifdef _GR_IPHONE_DEVICE_BUILD
        struct suARMv6InterceptionIsland;
        class suSpyBreakpointImplementation;
    #endif
#endif

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>

// Local:
#include <AMDTServerUtilities/Include/suAllocatedObjectsMonitor.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suInteroperabilityHelper.h>
#include <AMDTServerUtilities/Include/suMemoryAllocationMonitor.h>
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          SU_API suGlobalServerEnvironmentSettings
// General Description: Holds global environment settings, mainly values settable by
//                      environment variables that allow tweaking certain behaviors.
// Author:             Uri Shomroni
// Creation Date:      16/10/2013
// ----------------------------------------------------------------------------------
struct SU_API suGlobalServerEnvironmentSettings
{
    suGlobalServerEnvironmentSettings()
        : m_gsDontForceOpenGLDebugContexts(false),
          m_csDontAddDebuggingBuildFlags(false),
          m_hdForceResetOfKernelDebugging(false),
          m_suDontFixCRInSourceStrings(false)
    {};
    ~suGlobalServerEnvironmentSettings() {};

public:
    ////////////////////
    // OpenCL server: //
    ////////////////////

    // gsDontForceOpenGLDebugContexts - true = glCreateContextAttribs* will not force the debug flag in GL_CONTEXT_FLAGS
    bool m_gsDontForceOpenGLDebugContexts;

    // csDontAddDebuggingBuildFlags - true = clBuildProgram, clCompileProgram and clLinkProgram will not add "-g -O0 -cl-opt-disable"
    // to the program build flags.
    bool m_csDontAddDebuggingBuildFlags;

    // hdForceResetOfKernelDebugging - true = hdHSAHardwareBasedDebuggingManager will force a cleanup of the debug state, overriding any
    // existing debug session ***ON THE ENTIRE MACHINE***. This is a very powerful flag, do not expose it to users.
    bool m_hdForceResetOfKernelDebugging;

    // suDontFixCRInSourceStrings - true = clCreateProgramWithSource, glShaderSource, glShaderSourceARB will not replace CR-s not followed by a LF with CRLF.
    bool m_suDontFixCRInSourceStrings;
};


// ------------------ Global variables access functions ------------------------

// API Thread:
void suSetSpiesAPIThreadId(osThreadId spiesAPIThreadId);
osThreadId suSpiesAPIThreadId();
SU_API bool suIsSpiesAPIThreadId(osThreadId threadId); // Note that this function is expressly the only one that's exported from the module.

// Execution mode:
SU_API void suSetDebuggedProcessExecutionMode(apExecutionMode executionMode);
SU_API apExecutionMode suDebuggedProcessExecutionMode();

// Standalone mode:
SU_API void suSetStandaloneModeStatus(bool isRunningInStandaloneMode);
SU_API bool suIsRunningInStandaloneMode();

// Debugged process termination:
SU_API void suOnDebuggedProcessTermination();
SU_API bool suIsDuringDebuggedProcessTermination();

// Project Log files:
SU_API void suSetCurrentProjectLogFilesDirectory(const osFilePath& directoryPath);
SU_API const osFilePath& suCurrentProjectLogFilesDirectory();

// Session Log files:
SU_API void suSetCurrentSessionLogFilesDirectory(const osFilePath& directoryPath);
SU_API const osFilePath& suCurrentSessionLogFilesDirectory();

// Textures data logging:
SU_API void suEnableImagesDataLogging(bool isEnabled);
SU_API bool suIsTexturesDataLoggingEnabled();
SU_API void suSetLoggedTexturesFileType(apFileType fileType);
SU_API apFileType suLoggedTexturesFileType();

// Calls and commands logging:
SU_API void suSetLoggingLimits(unsigned int maxOpenGLCallsPerContext, unsigned int maxOpenCLCallsPerContext, unsigned int maxOpenCLCommandPerQueue);
SU_API unsigned int suMaxLoggedOpenGLCallsPerContext();
SU_API unsigned int suMaxLoggedOpenCLCalls();
SU_API unsigned int suMaxLoggedOpenCLCommandPerQueue();

// Global server environment settings:
SU_API const suGlobalServerEnvironmentSettings& suGetGlobalServerEnvironmentSettings();
SU_API suGlobalServerEnvironmentSettings& suGetMutableGlobalServerEnvironmentSettings();

// Debugger install directory:
SU_API void suSetDebuggerInstallDir(const osFilePath& installDirectory);
SU_API const osFilePath& suDebuggerInstallDir();

// Calls history log file:
SU_API void suFlushLogFileAfterEachFunctionCall(bool shouldFlush);
SU_API bool suShouldFlushLogFileAfterEachFunctionCall();

// OpenGL frame terminators:
SU_API void suSetFrameTerminatorsMask(unsigned int frameTerminatorsMask);
SU_API unsigned int suFrameTerminatorsMask();

// "Slow motion" delay:
SU_API void suSetSlowMotionDelayTimeUnits(unsigned int slowMotionDelayTimeUnits);
SU_API int suSlowMotionDelayTimeUnits();

// Log files:
SU_API void suStartHTMLLogFileRecording();
SU_API void suStopHTMLLogFileRecording();
SU_API bool suAreHTMLLogFilesActive();
SU_API bool suWereHTMLLogFilesActive();

// References to singletons instances:
SU_API extern suMemoryAllocationMonitor& su_stat_memoryAllocationMonitor;
SU_API extern apMonitoredFunctionsManager& su_stat_theMonitoredFunMgr;
SU_API extern suBreakpointsManager& su_stat_theBreakpointsManager;
SU_API extern suAllocatedObjectsMonitor& su_stat_theAllocatedObjectsMonitor;
SU_API extern suInteroperabilityHelper& su_stat_interoperabilityHelper;


// Initialization / termination:
void suTerminateGlobalVariables();

// ------------------ Global variables direct access ------------------------
// (Use with care, only for performance critical access)

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // See gsMacOSInterception.cpp
    extern suFunctionInterceptionInformation* su_stat_functionInterceptionInfo;
    #ifdef _GR_IPHONE_DEVICE_BUILD
        extern suARMv6InterceptionIsland* su_stat_armv6InterceptionIslands;
        extern suSpyBreakpointImplementation& su_stat_spyBreakpointsImplemenation;
    #endif
#endif


#endif //__SUGLOBALVARIABLES_H

