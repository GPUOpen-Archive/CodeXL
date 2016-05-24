//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnDefinitions.h
///
//==================================================================================

#ifndef __dmnDefinitions_h
#define __dmnDefinitions_h

#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// *************************
// TYPE DECLARATIONS - BEGIN
// *************************

enum REMOTE_OPERATION_MODE
{
    romUNKNOWN = 0x00,
    romDEBUG,
    romPROFILE,
    romGRAPHICS,
    romModeCount
};

enum DaemonOpCode
{
    docUnknown = 0x00,
    docLaunchRds,
    docLaunchProfiler,
    docDebuggingSessionStatusQuery,
    docProfilingSessionStatusQuery,
    docTerminateDebuggingSession,
    docTerminateProfilingSession,
    docTerminateWholeSession,
    docGetRemoteFile,
    docConsumeRemoteFile,
    docGetDaemonCXLVersion,
    docGetDaemonPlatform,

    // RT power profiling.
    docPowerInit,
    docPowerSetSamplingConfig,
    docPowerSetSamplingInterval,
    docPowerSetSamplingPeriod,
    docPowerGetCurrentSamplingPeriod,
    docPowerGetMinimumSamplingInterval,
    docPowerGetSystemTopology,
    docPowerStartProfiling,
    docPowerStopProfiling,
    docPowerPauseProfiling,
    docPowerResumeProfiling,
    docPowerEnableCounter,
    docPowerDisableCounter,
    docPowerIsCounterEnabled,
    docPowerClose,
    docPowerGetSamplesBatch,
    docPowerGetDeviceCounters,
    docPowerDisconnectWithoutClosing,

    // Frame analysis
    docLaunchGraphicsBeckendServer,
    docTerminateGraphicsBeckendServerSession,
    docGetCapturedFrames,
    docGetCapturedFramesByTime,
    docCapturedFrameData,
    docDeleteFrameAnalysisSession,

    // General utilities
    docIsProcessRunning,
    docKillRunningProcess,
    docIsHSAEnabled,
    docValidateAppPaths,

    docOpCodeCount
};

enum DaemonPlatform
{
    dpUnknown = 0x00,
    dpWindows,
    dpLinux,
    dpPlatformCount
};

enum DaemonOpStatus
{
    // Unknown.
    dosUnknown        = 0x00,

    // Success.
    dosSuccess        = 0x01,

    // Failure.
    dosFailure        = 0x02,

    // The remote agent terminated the session itself.
    dosSelfTerminated = 0x03,

    // Count.
    dosOpStatusCount
};

enum DaemonSessionStatus
{
    dssUknown = 0x00,
    dssAlive,
    dssTerminated,
    dssSessionStatusCount
};

enum DaemonFileType
{
    dftUnknown,

    // Mentions that a file is missing.
    dftMissingFile,

    dftProfilerCounterFile,
    dftProfilerEnvVarsFile,
    dftProfilerApiFiltersFile,
    dftProfilerApiRulesFile,
};

// ***********************
// TYPE DECLARATIONS - END
// ***********************


// *****************
// CONSTANTS - BEGIN
// *****************

const int DMN_MAX_TIMEOUT_VAL = -1;

// If transmitted, this code signals that the
// remote agent terminated itself.
const unsigned int DMN_SELF_TERMINATION_CODE = 0X7B;

// ***************
// CONSTANTS - END
// ***************

#endif // __dmnDefinitions_h
