//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file LocalBackendAdapter.h
///
//==================================================================================

#ifndef LocalPowerProfiler_h__
#define LocalPowerProfiler_h__

#ifdef _WIN32
    #define MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
#endif

#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>

// Forward declarations.
struct ApplicationLaunchDetails;

class LocalBackendAdapter :
    public IPowerProfilerBackendAdapter
{
public:
    LocalBackendAdapter();
    virtual ~LocalBackendAdapter();

    virtual PPResult InitializeBackend();

    virtual PPResult GetSystemTopology(gtList<PPDevice*>& systemDevices);

    virtual PPResult GetDeviceCounters(int deviceID, gtList<AMDTPwrCounterDesc*>& countersListBuffer);

    virtual PPResult GetMinTimerSamplingPeriodMS(unsigned int& samplingPeriodBufferMs);

    virtual PPResult GetProfilingState(AMDTPwrProfileState& stateBuffer);

    virtual PPResult GetCurrentSamplingInterval(unsigned int& samplingIntervalMs);

    virtual PPResult IsCounterEnabled(unsigned int counterID, bool& isEnabled);

    virtual PPResult GetNumOfEnabledCounters(int& numOfAvailableCounters);

    virtual PPResult StartProfiling();

    virtual PPResult StopProfiling();

    virtual PPResult PauseProfiling();

    virtual PPResult ResumeProfiling();

    virtual PPResult EnableCounter(int counterId);

    virtual PPResult DisableCounter(int counterId);

    virtual PPResult SetTimerSamplingInterval(unsigned int interval);

    virtual PPResult CloseProfileSession();

    virtual PPResult ReadAllEnabledCounters(gtVector<AMDTProfileTimelineSample*>& buffer);

    virtual PPResult SetApplicationLaunchDetails(const ApplicationLaunchDetails& appLaunchDetails);

    virtual AppLaunchStatus GetApplicationLaunchStatus();

    virtual void GetLastErrorMessage(gtString& msg);

    // This is a temporary change. Will be replaced with a built-in profiler at a later stage.
#ifdef MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS
    LONGLONG m_stopwatchFrequency, m_startStopwatch, m_endStopwatch;
#endif // MEASURE_TIME_BETWEEN_CALLS_ON_WINDOWS

};


#endif // LocalPowerProfiler_h__
