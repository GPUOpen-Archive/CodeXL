//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IPowerProfilerBackendAdapter.h
///
//==================================================================================

#ifndef __IPowerProfilerBackendAdapter_h
#define __IPowerProfilerBackendAdapter_h

// Local.
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>

// Infrastructure.
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Common DB related structures
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

// Currently include the API header through a hard-coded path.
// Should be updated.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

class IPowerProfilerBackendAdapter
{
public:
    virtual ~IPowerProfilerBackendAdapter() {}

    // Initialization.
    virtual PPResult InitializeBackend() = 0;

    // Meta Data.
    virtual PPResult GetSystemTopology(gtList<PPDevice*>& systemDevices) = 0;
    virtual PPResult GetDeviceCounters(int deviceID, gtList<AMDTPwrCounterDesc*>& countersListBuffer) = 0;
    virtual PPResult GetMinTimerSamplingPeriodMS(unsigned int& samplingPeriodBufferMs) = 0;
    virtual PPResult GetProfilingState(AMDTPwrProfileState& stateBuffer) = 0;
    virtual PPResult GetCurrentSamplingInterval(unsigned int& samplingIntervalMs) = 0;
    virtual PPResult IsCounterEnabled(unsigned int counterID, bool& isEnabled) = 0;
    virtual PPResult GetNumOfEnabledCounters(int& numOfAvailableCounters) = 0;
    virtual PPResult SetApplicationLaunchDetails(const ApplicationLaunchDetails& appLaunchDetails) = 0;
    virtual AppLaunchStatus GetApplicationLaunchStatus() = 0;

    // Control.
    virtual PPResult StartProfiling() = 0;
    virtual PPResult StopProfiling() = 0;
    virtual PPResult PauseProfiling() = 0;
    virtual PPResult ResumeProfiling() = 0;
    virtual PPResult EnableCounter(int counterId) = 0;
    virtual PPResult DisableCounter(int counterId) = 0;
    virtual PPResult SetTimerSamplingInterval(unsigned int interval) = 0;
    virtual PPResult CloseProfileSession() = 0;
    virtual PPResult ReadAllEnabledCounters(gtVector<AMDTProfileTimelineSample*>& buffer) = 0;

    // Error handling.
    virtual void GetLastErrorMessage(gtString& msg) = 0;
};
#endif // IPowerProfilerBackendAdapter_h__