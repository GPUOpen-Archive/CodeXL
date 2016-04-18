//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RemoteBackendAdapter.h
///
//==================================================================================

#ifndef __RemoteBackendAdapter_h
#define __RemoteBackendAdapter_h

// Local.
#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Common DB related structures
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

// Remote client.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

class RemoteBackendAdapter :
    public IPowerProfilerBackendAdapter
{
public:
    RemoteBackendAdapter() {};
    virtual ~RemoteBackendAdapter();

    // Set the address of the remote target.
    bool SetRemoteTarget(const gtString& remoteTargetHostName, unsigned short remoteTargetPortNumber);

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

private:
    gtString m_remoteHostName;
    unsigned short m_remoteTargetPort;
    ApplicationLaunchDetails m_appLaunchDetails;
    AppLaunchStatus m_appLaunchStatus;
    gtString m_lastErrorMsg;
};

#endif // __RemoteBackendAdapter_h
