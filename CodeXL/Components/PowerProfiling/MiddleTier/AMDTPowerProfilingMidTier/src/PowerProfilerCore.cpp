//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerCore.cpp
///
//==================================================================================

#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfilingMidTier/include/PPPollingThread.h>

// Power Profiling Backend
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileApi.h>

// Local Backend Adapter.
#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>
#include <AMDTPowerProfilingMidTier/include/LocalBackendAdapter.h>
#include <AMDTPowerProfilingMidTier/include/BackendDataConvertor.h>

// Remote Backend Adapter.
#include <AMDTPowerProfilingMidTier/include/RemoteBackendAdapter.h>

// Framework.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>
#include <AMDTBaseTools/Include/gtSet.h>

// C++.
#include <algorithm>

#ifndef NULL
    #define NULL 0
#endif


class PowerProfilerCore::Impl
{
public:

    // By default we go with the local adapter.
    // When remote profiling will be supported, we will be able to
    // pass a RemoteBackendAdapter (to be implemented).
    Impl()
        : m_pBEAdapter(new LocalBackendAdapter())
        , m_pPollingThread(NULL)
        , m_pDataAdapter(new amdtProfileDbAdapter())
        , m_currentSamplingIntervalMs(0)
        , m_powerProfilingSupportLevel(POWER_PROFILING_LEVEL_UNKNOWN)
    {}

    ~Impl()
    {
        delete m_pBEAdapter;
        m_pBEAdapter = NULL;

        delete m_pPollingThread;
        m_pPollingThread = NULL;

        delete m_pDataAdapter;
        m_pDataAdapter = NULL;
    }

    void ClearSystemTopologyCache()
    {
        for (PPDevice*& pDevice : m_sysTopologyCache)
        {
            delete pDevice;
            pDevice = NULL;
        }

        m_sysTopologyCache.clear();
    }

    void ClearEnabledCountersCache()
    {
        m_enabledCountersCache.clear();
    }

    // Initialization.
    PPResult InitPowerProfiler(const PowerProfilerCoreInitDetails& initDetails)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;

        m_initDetails = initDetails;

        // Clear the system topology cache.
        ClearSystemTopologyCache();

        // Clear the enabled counters cache.
        ClearEnabledCountersCache();

        // Reset the sampling interval.
        m_currentSamplingIntervalMs = 0;

        // Create a new backend adapter.
        IPowerProfilerBackendAdapter* pBeAdapter = NULL;

        if (m_initDetails.m_isRemoteProfiling)
        {
            // Create a remote backend adapter.
            RemoteBackendAdapter* pRemoteAdapter = new RemoteBackendAdapter();

            // Set the remote target.
            pRemoteAdapter->SetRemoteTarget(initDetails.m_remoteHostAddr, initDetails.m_remotePortNumber);

            // Set the current backend adapter to the remote one.
            pBeAdapter = pRemoteAdapter;
        }
        else
        {
            // Create a local backend adapter.
            pBeAdapter = new LocalBackendAdapter;
        }

        // Replace the existing adapter with the new one.
        delete m_pBEAdapter;
        m_pBEAdapter = pBeAdapter;

        // Initialize the backend.
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->InitializeBackend();
        }

        if (ret == PPR_NOT_SUPPORTED)
        {
            m_powerProfilingSupportLevel = POWER_PROFILING_LEVEL_PROFILING_NOT_SUPPORTED;
        }
        else
        {
            m_powerProfilingSupportLevel = POWER_PROFILING_LEVEL_PROFILING_SUPPORTED;
        }

        return ret;
    }

    // Shutdown.
    PPResult ShutdownPowerProfiler()
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->CloseProfileSession();
        }
        return ret;
    }

    // Control.
    PPResult StartProfiling(const AMDTProfileSessionInfo& sessionInfo, const ApplicationLaunchDetails& targetAppDetails,
                            AppLaunchStatus& targetAppLaunchStatus, PPSamplesDataHandler profileOnlineDataHandler, void* pDataHandlerParams,
                            PPFatalErrorHandler fatalErrorHandler, void* pErrorHandlerParams)
    {
        // Reset the output variable.
        targetAppLaunchStatus = rasUnknown;

        unsigned samplingIntervalMs = 0;
        PPResult ret =  GetCurrentSamplingIntervalMS(samplingIntervalMs);
        GT_IF_WITH_ASSERT(ret < PPR_FIRST_ERROR)
        {
            unsigned minSamplingIntervalMs = 0;
            ret = GetMinSamplingIntervalMs(minSamplingIntervalMs);
            GT_IF_WITH_ASSERT(ret < PPR_FIRST_ERROR)
            {
                GT_IF_WITH_ASSERT(samplingIntervalMs >= minSamplingIntervalMs)
                {
                    // Create a new data adapter.
                    delete m_pDataAdapter;
                    m_pDataAdapter = new amdtProfileDbAdapter();

                    gtList<PPDevice*>* systemTopology;
                    GetSystemTopology(systemTopology);

                    gtVector<int> enabledCounters;
                    GetEnabledCounters(enabledCounters);

                    // Create the session DB.
                    bool isDbCreated = m_pDataAdapter->CreateDb(sessionInfo.m_sessionDbFullPath, AMDT_PROFILE_MODE_TIMELINE);

                    GT_IF_WITH_ASSERT(isDbCreated)
                    {
                        // This is the beginning of the power profiling session.
                        // Save the session configuration to the db.
                        //int quantizedTime = 0;

                        // Get the list of devices
                        gtVector<AMDTProfileDevice*> profileDeviceVec;
                        bool rc = BackendDataConvertor::ConvertToProfileDevice(*systemTopology, profileDeviceVec);

                        gtList<AMDTPwrCounterDesc*> pwrCounterDescList;
                        gtVector<AMDTProfileCounterDesc*> profileCounterVec;

                        if (rc)
                        {
                            m_pBEAdapter->GetDeviceCounters(AMDT_PWR_ALL_DEVICES, pwrCounterDescList);
                            rc = BackendDataConvertor::ConvertToProfileCounterDescVec(pwrCounterDescList, profileCounterVec);
                        }

                        if (rc)
                        {
                            rc = m_pDataAdapter->InsertAllSessionInfo(sessionInfo,
                                                                      samplingIntervalMs,
                                                                      profileDeviceVec,
                                                                      profileCounterVec,
                                                                      enabledCounters);
                        }

                        GT_IF_WITH_ASSERT(rc)
                        {
                            GT_IF_WITH_ASSERT(m_pPollingThread == NULL)
                            {
                                // Set the application launch details.
                                m_pBEAdapter->SetApplicationLaunchDetails(targetAppDetails);

                                // Create the polling thread.
                                m_pPollingThread = new PPPollingThread(samplingIntervalMs, profileOnlineDataHandler,
                                                                       pDataHandlerParams, fatalErrorHandler, pErrorHandlerParams, m_pBEAdapter, m_pDataAdapter);

                                // Launch the polling thread.
                                bool isPollingThreadDispatchSuccess = m_pPollingThread->execute();
                                GT_ASSERT_EX(isPollingThreadDispatchSuccess, L"Dispatch of PP polling thread");

                                if (isPollingThreadDispatchSuccess)
                                {
                                    // Wait for the signal from the polling thread.
                                    ret = m_pPollingThread->GetSessionStartedStatus();
                                    const unsigned MAX_NUM_OF_ITERATIONS = 800;
                                    unsigned currIteration = 0;

                                    while (ret == PPR_COMMUNICATION_FAILURE && (currIteration++ < MAX_NUM_OF_ITERATIONS))
                                    {
                                        osSleep(50);
                                        ret = m_pPollingThread->GetSessionStartedStatus();
                                    }

                                    // Retrieve the status of launching the target application.
                                    targetAppLaunchStatus = m_pPollingThread->GetApplicationLaunchStatus();
                                }
                            }
                        }
                        else
                        {
                            ret = PPR_POLLING_THREAD_ALREADY_RUNNING;
                        }
                    }
                    else
                    {
                        ret = PPR_DB_CREATION_FAILURE;
                    }
                }
                else
                {
                    ret = PPR_INVALID_SAMPLING_INTERVAL;
                }
            }
        }
        return ret;
    }


    PPResult StopProfiling()
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;

        // Stop the polling activity.
        GT_IF_WITH_ASSERT(m_pPollingThread != NULL)
        {
            m_pPollingThread->requestExit();
            osTimeInterval timeout;
            timeout.setAsMilliSeconds(100);
            m_pPollingThread->waitForThreadEnd(timeout);
            m_pPollingThread->terminate();
            delete m_pPollingThread;
            m_pPollingThread = NULL;
        }

        // Stop the backend.
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->StopProfiling();
        }

        GT_IF_WITH_ASSERT(m_pDataAdapter != NULL)
        {
            // Flush any pending data, and close all DB connections.
            m_pDataAdapter->FlushDb();
            m_pDataAdapter->CloseDb();
        }
        return ret;
    }

    PPResult PauseProfiling()
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->PauseProfiling();
        }
        return ret;
    }

    PPResult ResumeProfiling()
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->ResumeProfiling();
        }
        return ret;
    }

    PPResult EnableCounter(int counterId)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;

        GT_IF_WITH_ASSERT(m_pDataAdapter != NULL)
        {
            GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
            {
                // Enable the counter.
                ret = m_pBEAdapter->EnableCounter(counterId);
                GT_IF_WITH_ASSERT(ret < PPR_FIRST_ERROR)
                {
                    // Update the cache.
                    m_enabledCountersCache.insert(counterId);

                    // If the session has already started, document this action in the DB.
                    // Otherwise, this info will be documented together with all other session
                    // configurations just before the session starts running.
                    unsigned currentQuantizedTime = 0;

                    if (m_pPollingThread != NULL)
                    {
                        currentQuantizedTime = m_pPollingThread->GetCurrentQuantizedTime();

                        if (currentQuantizedTime > 0)
                        {
                            m_pDataAdapter->InsertCounterEnabled(counterId, currentQuantizedTime);
                        }
                    }
                }
            }
        }
        return ret;
    }

    PPResult DisableCounter(int counterId)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;

        GT_IF_WITH_ASSERT(m_pDataAdapter != NULL)
        {
            GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
            {
                // Disable the counter.
                ret = m_pBEAdapter->DisableCounter(counterId);
                GT_IF_WITH_ASSERT(ret < PPR_FIRST_ERROR)
                {
                    // Update the cache.
                    auto iter = m_enabledCountersCache.find(counterId);

                    if (iter != m_enabledCountersCache.end())
                    {
                        m_enabledCountersCache.erase(iter);
                    }

                    // If the session has already started, document this action in the DB.
                    // Otherwise, this info will be documented together with all other session
                    // configurations just before the session starts running.
                    unsigned currentQuantizedTime = 0;

                    if (m_pPollingThread != NULL)
                    {
                        currentQuantizedTime = m_pPollingThread->GetCurrentQuantizedTime();

                        if (currentQuantizedTime > 0)
                        {
                            m_pDataAdapter->InsertCounterDisabled(counterId, currentQuantizedTime);
                        }
                    }
                }
            }
        }
        return ret;
    }

    // Returns PPR_NO_ERROR if and only if ALL counters were enabled successfully.
    PPResult EnableCounter(const gtList<int>& counterIds)
    {
        PPResult ret = PPR_NO_ERROR;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            PPResult tmpRc = PPR_NO_ERROR;

            for (const int& counterId : counterIds)
            {
                tmpRc = this->EnableCounter(counterId);

                if (tmpRc != PPR_NO_ERROR && ret < PPR_FIRST_ERROR)
                {
                    ret = tmpRc;
                }
            }
        }
        return ret;
    }

    PPResult GetMinSamplingIntervalMs(unsigned& minSamplingInterval)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->GetMinTimerSamplingPeriodMS(minSamplingInterval);
        }
        return ret;
    }

    PPResult GetCurrentSamplingIntervalMS(unsigned& samplingInterval)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            if (m_currentSamplingIntervalMs == 0)
            {
                ret = m_pBEAdapter->GetCurrentSamplingInterval(samplingInterval);
            }
            else
            {
                samplingInterval = m_currentSamplingIntervalMs;
                ret = PPR_NO_ERROR;
            }
        }
        return ret;
    }

    PPResult SetSamplingIntervalMS(unsigned int samplingInterval)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            ret = m_pBEAdapter->SetTimerSamplingInterval(samplingInterval);

            if (ret == AMDT_STATUS_OK)
            {
                m_currentSamplingIntervalMs = samplingInterval;
            }
        }
        return ret;
    }

    PPResult IsCounterEnabled(int counterId, bool& isEnabled)
    {
        PPResult ret = PPR_UNKNOWN_FAILURE;
        GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
        {
            isEnabled = (m_enabledCountersCache.find(counterId) != m_enabledCountersCache.end());
            ret = PPR_NO_ERROR;
        }
        return ret;
    }

    PPResult GetSystemTopology(gtList<PPDevice*>*& pSysTopology)
    {
        PPResult ret = PPR_NO_ERROR;

        if (m_sysTopologyCache.empty())
        {
            GT_IF_WITH_ASSERT(m_pBEAdapter != NULL)
            {
                ret = m_pBEAdapter->GetSystemTopology(m_sysTopologyCache);
            }
        }

        // Assign the pointer to point to the cached structure.
        if (!m_sysTopologyCache.empty())
        {
            pSysTopology = &m_sysTopologyCache;
        }
        else
        {
            pSysTopology = NULL;
        }

        return ret;
    }

    PPResult GetEnabledCounters(gtVector<int>& enabledCounters)
    {
        PPResult ret = PPR_NO_ERROR;
        bool isEnabled = false;

        // Build the enabled counters map.
        gtMap<int, AMDTPwrCounterDesc*> countersMap;
        ret = GetAllCountersDetails(countersMap);

        GT_IF_WITH_ASSERT(ret < PPR_FIRST_ERROR)
        {
            for (auto& counterDetailsPair : countersMap)
            {
                int counterId = counterDetailsPair.first;

                // If the counter is enabled, and it hasn't been added yet, add its id to our container.
                if ((IsCounterEnabled(counterId, isEnabled) < PPR_FIRST_ERROR) && isEnabled &&
                    (std::find(enabledCounters.begin(), enabledCounters.end(), counterId) == enabledCounters.end()))
                {
                    enabledCounters.push_back(counterId);
                }

                // We can delete the counter details structure, as it won't be needed.
                delete counterDetailsPair.second;
                counterDetailsPair.second = NULL;
            }
        }

        return ret;
    }

    void UpdateCountersMap(const PPDevice* pDevice, gtMap<int, AMDTPwrCounterDesc*>& countersMap)
    {
        GT_IF_WITH_ASSERT(pDevice != NULL)
        {
            for (AMDTPwrCounterDesc* pCounterDesc : pDevice->m_supportedCounters)
            {
                GT_IF_WITH_ASSERT(pCounterDesc != NULL)
                {
                    // Copy the required counter's value.
                    countersMap[pCounterDesc->m_counterID] =
                        BackendDataConvertor::CopyPwrCounterDesc(*pCounterDesc);
                }
            }

            for (const PPDevice* pSubDevice : pDevice->m_subDevices)
            {
                UpdateCountersMap(pSubDevice, countersMap);
            }
        }
    }

    void BuildCountersMap(const gtList<PPDevice*>& systemTopology, gtMap<int, AMDTPwrCounterDesc*>& countersMap)
    {
        for (const PPDevice* pDevice : systemTopology)
        {
            UpdateCountersMap(pDevice, countersMap);
        }
    }

    PPResult GetAllCountersDetails(gtMap<int, AMDTPwrCounterDesc*>& counterDetailsPerCounter)
    {
        // This implementation can be optimized in the future by caching the system topology.
        gtList<PPDevice*>* pSystemTopology;

        // Get the system topology.
        PPResult ret = GetSystemTopology(pSystemTopology);

        GT_IF_WITH_ASSERT(ret < PPR_FIRST_ERROR)
        {
            GT_IF_WITH_ASSERT(pSystemTopology != NULL)
            {
                // Build the counters map.
                BuildCountersMap(*pSystemTopology, counterDetailsPerCounter);
            }
        }

        return ret;
    }

    void GetLastErrorMessage(gtString& msg) const
    {
        if (m_pBEAdapter != nullptr)
        {
            m_pBEAdapter->GetLastErrorMessage(msg);
        }
    }

    PowerProfilingSupportLevel GetPowerProfilingSupportLevel() const
    {
        return m_powerProfilingSupportLevel;
    }

    bool IsRemotePP() const
    {
        return m_initDetails.m_isRemoteProfiling;
    }


    // This object connects us to the backend.
    // It might be a local session or remote session.
    IPowerProfilerBackendAdapter* m_pBEAdapter;
    PPPollingThread* m_pPollingThread;
    amdtProfileDbAdapter* m_pDataAdapter;
    PowerProfilerCoreInitDetails m_initDetails;
    gtList<PPDevice*> m_sysTopologyCache;
    unsigned m_currentSamplingIntervalMs;
    gtSet<int> m_enabledCountersCache;


    /// Indicates whether power profiling is supported with the latest configuration that
    /// was fed to the profiler middle tier.
    /// This value is refreshed each time the middle tier is initialized.
    /// This value is cached and can be queried even after calling shutdown on the middle tier.
    PowerProfilingSupportLevel m_powerProfilingSupportLevel;
};

PowerProfilerCore::PowerProfilerCore() : m_pImpl(new PowerProfilerCore::Impl())
{
}


PowerProfilerCore::~PowerProfilerCore()
{
    delete m_pImpl;
    m_pImpl = nullptr;
}

PPResult PowerProfilerCore::InitPowerProfiler(const PowerProfilerCoreInitDetails& initDetails)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->InitPowerProfiler(initDetails);
        GT_ASSERT_EX(PPR_EXPECTED_RESULT(ret), L"PP Initialization");
    }
    return ret;
}

PPResult PowerProfilerCore::StartProfiling(const AMDTProfileSessionInfo& sessionInfo, const ApplicationLaunchDetails& targetAppDetails, AppLaunchStatus& targetAppLaunchStatus,
                                           PPSamplesDataHandler profileOnlineDataHandler, void* pDataHandlerParams, PPFatalErrorHandler profileOnlineFatalErrorHandler, void* pErrorHandlerParams)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->StartProfiling(sessionInfo, targetAppDetails, targetAppLaunchStatus,
                                      profileOnlineDataHandler, pDataHandlerParams, profileOnlineFatalErrorHandler, pErrorHandlerParams);
    }
    return ret;
}

PPResult PowerProfilerCore::StopProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->StopProfiling();
    }
    return ret;
}

PPResult PowerProfilerCore::PauseProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->PauseProfiling();
    }
    return ret;
}

PPResult PowerProfilerCore::ResumeProfiling()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->ResumeProfiling();
    }
    return ret;
}

PPResult PowerProfilerCore::GetEnabledCounters(gtVector<int>& enabledCounters)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetEnabledCounters(enabledCounters);
    }
    return ret;
}

PPResult PowerProfilerCore::EnableCounter(int counterId)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->EnableCounter(counterId);
    }
    return ret;
}

PPResult PowerProfilerCore::EnableCounter(const gtList<int>& counterIdsList)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->EnableCounter(counterIdsList);
    }
    return ret;
}

PPResult PowerProfilerCore::DisableCounter(int counterId)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->DisableCounter(counterId);
    }
    return ret;
}

PPResult PowerProfilerCore::GetMinSamplingIntervalMS(unsigned int& samplingInterval)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetMinSamplingIntervalMs(samplingInterval);
    }
    return ret;
}

PPResult PowerProfilerCore::GetCurrentSamplingIntervalMS(unsigned int& samplingInterval)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetCurrentSamplingIntervalMS(samplingInterval);
    }
    return ret;
}

PPResult PowerProfilerCore::SetSamplingIntervalMS(unsigned int samplingInterval)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->SetSamplingIntervalMS(samplingInterval);
    }
    return ret;
}

PPResult PowerProfilerCore::IsCounterEnabled(int counterId, bool& isEnabled)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    isEnabled = false;

    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->IsCounterEnabled(counterId, isEnabled);
    }
    return ret;
}

PPResult PowerProfilerCore::ShutdownPowerProfiler()
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->ShutdownPowerProfiler();
    }
    return ret;
}

PPResult PowerProfilerCore::GetSystemTopology(gtList<PPDevice*>& systemDevices)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        // Get a reference to the implementation's structure.
        gtList<PPDevice*>* pSysDevices = NULL;
        ret = m_pImpl->GetSystemTopology(pSysDevices);

        // Create a deep copy of the structure, to avoid returning
        // the cached value, which might be cached by the user and
        // become invalidated when this object (PowerProfilerCore)
        // becomes re-initialized.
        GT_IF_WITH_ASSERT(pSysDevices != NULL)
        {
            systemDevices.clear();

            for (PPDevice* pDevice : *pSysDevices)
            {
                if (pDevice != NULL)
                {
                    PPDevice* pDeviceCopy = new PPDevice(*pDevice);
                    systemDevices.push_back(pDeviceCopy);
                }
            }
        }

    }
    return ret;
}

PPResult PowerProfilerCore::GetAllCountersDetails(gtMap<int, AMDTPwrCounterDesc*>& counterDetailsPerCounter)
{
    PPResult ret = PPR_UNKNOWN_FAILURE;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetAllCountersDetails(counterDetailsPerCounter);
    }
    return ret;
}

void PowerProfilerCore::GetLastErrorMessage(gtString& errorMessage) const
{
    errorMessage.makeEmpty();
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        m_pImpl->GetLastErrorMessage(errorMessage);
    }
}

PowerProfilerCore::PowerProfilingSupportLevel PowerProfilerCore::GetPowerProfilingSupportLevel() const
{
    PowerProfilingSupportLevel ret = POWER_PROFILING_LEVEL_UNKNOWN;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetPowerProfilingSupportLevel();
    }
    return ret;
}

bool PowerProfilerCore::isRemotePP() const
{
    bool rc = m_pImpl->IsRemotePP();
    return rc;
}
