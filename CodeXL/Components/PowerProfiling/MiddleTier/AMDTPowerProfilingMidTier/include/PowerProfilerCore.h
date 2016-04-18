//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerCore.h
///
//==================================================================================

#ifndef PowerProfiler_h__
#define PowerProfiler_h__

// Local.
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>

// Infra.
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>

// Common DB related structures
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

struct AMDTPOWERPROFILINGMIDTIER_API PowerProfilerCoreInitDetails
{
    // By default, we set the state for local profiling.
    PowerProfilerCoreInitDetails() : m_isRemoteProfiling(false) {}

    // If host address and port are specified, the state is set for remote profiling.
    PowerProfilerCoreInitDetails(const gtString& remoteHostAddr,
                                 unsigned short remotePortAddr) : m_isRemoteProfiling(true),
        m_remoteHostAddr(remoteHostAddr), m_remotePortNumber(remotePortAddr) {}

    // Is it remote profiling.
    bool m_isRemoteProfiling;

    // The network address of the remote host.
    gtString m_remoteHostAddr;

    // The port number of the remote host.
    unsigned short m_remotePortNumber = 0;
};

class AMDTPOWERPROFILINGMIDTIER_API PowerProfilerCore
{
public:

    // Use bitfield compatible values in case we want to add details to the support level in the future
    enum PowerProfilingSupportLevel
    {
        POWER_PROFILING_LEVEL_UNKNOWN = 0,
        POWER_PROFILING_LEVEL_PROFILING_NOT_SUPPORTED = 0x01,
        POWER_PROFILING_LEVEL_PROFILING_SUPPORTED = 0x02
    };

    PowerProfilerCore();
    ~PowerProfilerCore();

    /*** Init & Shutdown ***/

    /// Initializes the power profiler.
    PPResult InitPowerProfiler(const PowerProfilerCoreInitDetails& initDetails);

    /// Shuts down the power profiler.
    PPResult ShutdownPowerProfiler();



    /*** Session Control ***/

    /// Starts a profiling session.
    /// @param[in] sessionInfo - the session info to be recorded in the session DB.
    /// @param[in] profileOnlineDataHandler - the callback which handles the data events.
    /// @param[in] pDataHandlerParams - a pointer to data which will be passed to the data callback (for example, you can pass the 'this'
    /// pointer of your class as a parameter).
    /// @param[in] profileOnlineFatalErrorHandler - the callback which handles error events.
    /// @param[in] pErrorHandlerParams - a pointer to data which will be passed to the error events callback (for example, you can pass the 'this' pointer
    /// of your class as a parameter).
    PPResult StartProfiling(const AMDTProfileSessionInfo& sessionInfo, const ApplicationLaunchDetails& targetAppDetails, AppLaunchStatus& targetAppLaunchStatus,
                            PPSamplesDataHandler profileOnlineDataHandler, void* pDataHandlerParams, PPFatalErrorHandler profileOnlineFatalErrorHandler, void* pErrorHandlerParams);

    /// Stops a profiling session.
    PPResult StopProfiling();

    /// Pauses a profiling session.
    PPResult PauseProfiling();

    /// Resumes a profiling session.
    PPResult ResumeProfiling();

    /// Enables a counter.
    /// @param[in] counterId - the id of the counter to be enabled.
    PPResult EnableCounter(int counterId);

    /// Enables counters.
    /// @param[in] counterIdsList - the list of counter ids of the counters to be enabled.
    PPResult EnableCounter(const gtList<int>& counterIdsList);

    /// Disables a counter.
    /// @param[in] counterId - the id of the counter to be disabled.
    PPResult DisableCounter(int counterId);

    /// Disable counter.
    /// @param[in] counterIdsList - the list of counter ids of the counters to be enabled.
    PPResult DisableCounter(const gtList<int>& counterIdsList);



    /*** Meta-Data Access ***/

    /// Extracts the system topology.
    /// @param[out] systemDevices - a list of devices to hold the topology tree.
    PPResult GetSystemTopology(gtList<PPDevice*>& systemDevices);

    /// Extracts the minimum possible sampling interval in [ms].
    /// @param[out] samplingInterval - the minimum possible sampling interval in [ms].
    PPResult GetMinSamplingIntervalMS(unsigned int& samplingInterval);

    /// Extracts the session sampling interval in [ms].
    /// @param[out] samplingInterval - session sampling interval in [ms].
    PPResult GetCurrentSamplingIntervalMS(unsigned int& samplingInterval);

    /// Sets the session sampling interval in [ms].
    /// @param[in] samplingInterval - session sampling interval in [ms].
    PPResult SetSamplingIntervalMS(unsigned int samplingInterval);

    /// Sets true isEnabled to true if counterId represents an enabled counter, otherwise sets isEnabled to false.
    /// @param[in] counterId - the id of the counter to be checked.
    /// @param[out] isEnabled - a buffer to hold the result.
    PPResult IsCounterEnabled(int counterId, bool& isEnabled);

    /// Retrieves a list of the enabled counters ids.
    /// @param[out] enabledCounters - a list of the enabled counters ids to be filled by the function.
    PPResult GetEnabledCounters(gtVector<int>& enabledCounters);

    /// Retrieves the details of all counters.
    /// @param[out] counterDetailsPerCounter - a maps that maps between each counter id and its corresponding description structure.
    PPResult GetAllCountersDetails(gtMap<int, AMDTPwrCounterDesc*>& counterDetailsPerCounter);

    /// Retrieves the most recent error message, if there is any.
    /// @param[out] errorMessage - a buffer to be filled with the error message.
    void GetLastErrorMessage(gtString& errorMessage) const;

    /// Indicates whether power profiling is supported with the latest configuration that
    /// was fed to the profiler middle tier.
    /// This value is refreshed each time the middle tier is initialized.
    /// This value is cached and can be queried even after calling shutdown on the middle tier.
    PowerProfilingSupportLevel GetPowerProfilingSupportLevel() const;
    bool isRemotePP() const;

private:
    class Impl;
    Impl* m_pImpl;

    // No copy.
    PowerProfilerCore(const PowerProfilerCore& other);
    PowerProfilerCore& operator=(const PowerProfilerCore& other);

};

#endif // PowerProfiler_h__
