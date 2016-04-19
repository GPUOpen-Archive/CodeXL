//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileControl_Lin.h
///
//==================================================================================

#ifndef _CPUPROFILECONTROL_LIN_H_
#define _CPUPROFILECONTROL_LIN_H_

#include <stdio.h>
#include <errno.h>
#include <wchar.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>

//  TODO: The AMDTOSWrappers library is not yet available in Linux. Hence the osCpuid cannot
//  be used in Linux.
//
// #define  CODEXL_OSWRAPPERS__AVBL     1

// TODO: The LibCpuPerfEvent library is not yet available in Linux. Hence the eventId cannot
//  be validated in Linux.
//
// #define  CODEXL_LIBCPUPERFEVENT_AVBL 1

//  TODO: The PERF kernel layer is yet to support IBS. Till then disable this
//
// #define  CPU_PERF_PERF_SUPPORTS_IBS      1

#include <CpuProfileControl.h>

#ifdef CODEXL_OSWRAPPERS_AVBL
    #include <AMDTOSWrappers/Include/osCpuid.h>
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

#ifdef CODEXL_LIBCPUPERFEVENT_AVBL
    #include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>

// CaPerf headers
#include "CaPerfEvent.h"
#include "CaPerfConfig.h"
#include "PerfPmuTarget.h"
#include "CaPerfProfiler.h"
#include "PerfPmuSession.h"

//
// Macros
//

#define MAX_CLIENT_COUNT    1
#define MAX_PID_COUNT       256


// IBS related Macros
//
// MIN_IBS_CYCLE_COUNT Experimentally defined minimal cycle count.  With
// a lower count, it will crash due to excessive interrupts.  (10,000)
#define MIN_IBS_CYCLE_COUNT 0x2710

// MAX_IBS_CYCLE_COUNT BKDG defined maximum cycle count - 20 bit counter
#define MAX_IBS_CYCLE_COUNT 0xFFFFF

// MAX_IBS_EXT_COUNT BKDG defined maximum cycle count, when the extended
// IBS Op count is enabled.
#define MAX_IBS_EXT_COUNT 0x7FFFFFF

//
// Linux Specific API functions
//
HRESULT CpuPerfEnableProfiling();

HRESULT CpuPerfEnableProfiling(bool usePERF);

HRESULT CpuPerfReleaseProfiling();

HRESULT CpuPerfGetDriverVersion(unsigned int* pMajor, unsigned int* pMinor, unsigned int* pBuild);

HRESULT CpuPerfMakeProfileEvent(
    /*in*/ unsigned int eventSelect,
    /*in*/ unsigned int unitMask,
    /*in*/ bool edgeDetect,
    /*in*/ bool usrEvents,
    /*in*/ bool osEvents,
    /*in*/ bool guestOnlyEvents,
    /*in*/ bool hostOnlyEvents,
    /*in*/ bool countingEvent,
    /*out*/ gtUInt64* pPerformanceEvent);

// This function retrieves the availability of event counters per core.
HRESULT CpuPerfGetEventCounterAvailability(
    /*in*/ gtUInt64 performanceEvent,
    /*out*/ unsigned int* pAvailabilityMask,
    /*in*/  PmcType type);

HRESULT CpuPerfGetProfilerState(
    /*out*/ ProfileState* pProfileState);

HRESULT CpuPerfStartProfiling(
    /*in*/ bool startPaused,
    /*in*/ const wchar_t* pauseKey,
    /*out*/ ProfileState* pProfileState);

CaProfileCtrlKey CpuPerfGetProfileControlKey(const wchar_t* pauseKey);

HRESULT CpuPerfPauseProfiling(CaProfileCtrlKey profileKey);

HRESULT CpuPerfResumeProfiling(CaProfileCtrlKey profileKey);

HRESULT CpuPerfStopSamplingProfile(gtUInt32 clientId);

HRESULT CpuPerfStopCPUUtilMonitoring(gtUInt32 clientId);

HRESULT CpuPerfSetCountingEvent(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*in*/ EventConfiguration performanceEvent);

HRESULT CpuPerfGetEventCount(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*out*/ gtUInt64* pEventCount);

HRESULT CpuPerfSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ bool profileAllCores);

HRESULT CpuPerfSetCountingConfiguration(
    /*in*/ gtUInt32 clientId,
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores);

HRESULT CpuPerfGetCountingEventCount(
    unsigned int core,
    unsigned int* pCount);

HRESULT CpuPerfGetAllEventCounts(
    /*in*/ unsigned int core,
    /*in*/ unsigned int size,
    /*out*/ gtUInt64* pCounts);

HRESULT CpuPerfSetProfileOutputFile(/*in*/ const wchar_t* pFileName);

HRESULT CpuPerfSetProfileOutputDirectory(/*in*/ const wchar_t* pDirectoryName);

HRESULT CpuPerfGetCurrentTimeMark(CPA_TIME* pTimeMark);

// UNUSED
// HRESULT CpuPerfGetSampleCount (gtUInt32 *pCount);

HRESULT CpuPerfSetTimerConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ bool profileAllCores = false);

HRESULT CpuPerfSetTimerConfiguration(
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores = false);

HRESULT CpuPerfSetThreadProfileConfiguration(bool isCSS);

HRESULT CpuPerfSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ bool profileAllCores = false);

HRESULT CpuPerfSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores = false);

HRESULT CpuPerfSetIbsConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples,
    /*in*/ bool useDispatchOps,
    /*in*/ bool profileAllCores = false);

HRESULT CpuPerfSetIbsConfiguration(
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples,
    /*in*/ bool useDispatchOps,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores = false);

HRESULT CpuPerfSetFilterProcesses(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ bool systemWideProfile,
    /*in*/ bool ignoreChildren);

HRESULT CpuPerfSetCallStackSampling(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ unsigned int unwindLevel,
    /*in*/ unsigned int samplePeriod,
    /*in*/ CpuProfileCssScope scope,
    /*in*/ bool captureVirtualStack);

HRESULT CpuPerfClearConfigurations(gtUInt32 clientId);

HRESULT CpuPerfEnableCPUUtilization(
    unsigned int utilization_interval,
    unsigned int monitorFlag);

HRESULT CpuPerfAddCPUUtilizationProcessId(unsigned pid);

#endif // _CPUPROFILECONTROL_LIN_H_
