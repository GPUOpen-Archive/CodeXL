//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileControl_Win.h
///
//==================================================================================

#ifndef _CPUPROFILECONTROL_WIN_H_
#define _CPUPROFILECONTROL_WIN_H_

#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTCpuPerfEventUtils/inc/EventsFile.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>
#include <WinIoCtl.h>
#include <Driver/Windows/CpuProf/inc/UserAccess/CpuProfDriver.h>
#include <CpuProfileControl.h>


#define MIN_DRIVER_VERSION 0x01000000


HRESULT CpuPerfEnableProfiling();

HRESULT CpuPerfReleaseProfiling();

HRESULT CpuPerfGetDriverVersion(unsigned int* pMajor,
                                unsigned int* pMinor,
                                unsigned int* pBuild);

HRESULT CpuPerfMakeProfileEvent(/*in*/ unsigned int eventSelect,
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
    /*in*/ bool pauseIndefinite,
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

HRESULT CpuPerfGetEventCount(/*in*/ unsigned int core,
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

HRESULT CpuPerfEnableCPUUtilization(unsigned int utilization_interval, unsigned int monitorFlag);

HRESULT CpuPerfAddCPUUtilizationProcessId(unsigned pid);

#endif // _CPUPROFILECONTROL_WIN_H_
