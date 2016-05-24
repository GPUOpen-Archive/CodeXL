//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCollect.h
///
//==================================================================================

#ifndef _PP_COLLECT_H_
#define _PP_COLLECT_H_
#pragma once

// Project
#include <PowerProfileCLI.h>
#include <ppParseArgs.h>

#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <sys/types.h>
    #include <sys/wait.h>
#endif // AMDT_LINUX_OS

class ppCollect
{
public:
    ppCollect(ppParseArgs& args) : m_args(args),
        m_error(S_OK),
        m_isProfilingEnabled(false),
        m_profileState(AMDT_PWR_PROFILE_STATE_UNINITIALIZED),
        m_launchedPid(0),
        m_pDeviceTopology(nullptr),
        m_nbrSupportedCounters(0),
        m_pSupportedCountersDesc(nullptr),
        m_totalElapsedTime(0),
        m_totalNbrSamples(0),
        m_NbrOfFailedReads(0),
        m_nbrSamples(0),
        m_pSampleData(nullptr),
        m_coreMask(0),
        m_isAppLaunched(false),
        m_isTemperatureSupported(false),
        m_pDbAdapter(nullptr),
        m_isDbExportEnabled(false)
    {
    }

    ~ppCollect()
    {
        Clear();
    };

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    ppCollect(const ppCollect&) = delete;
    ppCollect& operator=(const ppCollect&) = delete;
#endif

    AMDTResult Initialize();
    AMDTResult Configure();
    AMDTResult GetProcessData(AMDTUInt32* pPIDCount, AMDTPwrProcessInfo** ppData);
    AMDTResult GetModuleData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCnt, AMDTFloat32* pPower);

    AMDTResult EnableProcessProfiling();

    unsigned int GetLaunchedPid() const { return m_launchedPid; }

    AMDTResult GetDeviceNameDescMap(AMDTPwrDeviceNameDescMap& deviceNameDescMap)
    {
        AMDTResult retVal = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            deviceNameDescMap = m_deviceNameDescMap;
            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult GetDeviceIdDescVec(AMDTPwrDeviceIdDescVec& deviceIdDescVec)
    {
        AMDTResult retVal = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            deviceIdDescVec = m_deviceIdDescVec;
            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult GetDeviceIdNamecVec(AMDTPwrDeviceIdNameVec& deviceIdNameVec)
    {
        AMDTResult retVal = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            deviceIdNameVec = m_deviceIdNameVec;
            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult GetSupportedCounterNameDescMap(AMDTPwrCounterNameDescMap& counterDescMap)
    {
        AMDTResult retVal = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            counterDescMap = m_supportedCounterNameDescMap;
            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult GetSupportedCounterIdDescVec(AMDTPwrCounterIdDescVec& counterDescVec)
    {
        m_error = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            counterDescVec = m_supportedCounterIdDescVec;
            m_error = AMDT_STATUS_OK;
        }

        return m_error;
    }

    AMDTResult GetSupportedCounterIdNameVec(AMDTPwrCounterIdNameVec& counterIdNameVec)
    {
        m_error = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            counterIdNameVec = m_supportedCounterIdNameVec;
            m_error = AMDT_STATUS_OK;
        }

        return m_error;
    }

    AMDTResult GetProfiledCounterIdVec(gtVector<AMDTUInt32>& counterIdVec)
    {
        counterIdVec = m_profiledCounterIdVec;
        return AMDT_STATUS_OK;
    }

    AMDTResult GetCounterId(gtString& counterName, AMDTUInt32& counterID)
    {
        m_error = AMDT_ERROR_FAIL;

        if (!isProfileStateUninitialized())
        {
            AMDTPwrCounterNameDescMap::const_iterator iter = m_supportedCounterNameDescMap.find(counterName);

            if (iter != m_supportedCounterNameDescMap.end())
            {
                counterID = (*iter).second->m_counterID;
                m_error = AMDT_STATUS_OK;
            }
        }

        return m_error;
    }

    bool IsProfilingSimpleCounters() const { return (m_simpleCounterIdVec.size() > 0) ? true : false; }
    bool IsProfilingCumulativeCounters() const { return (m_cumulativeCounterIdVec.size() > 0) ? true : false; }
    bool IsProfilingHistogramCounters() const { return (m_histogramCounterIdVec.size() > 0) ? true : false; }

    AMDTUInt32 GetNumberOfSimpleCounters() const { return m_simpleCounterIdVec.size(); }
    AMDTUInt32 GetNumberOfCumulativeCounters() const { return m_cumulativeCounterIdVec.size(); }
    AMDTUInt32 GetNumberOfHistogramCounters() const { return m_histogramCounterIdVec.size(); }

    bool IsSimpleCounter(AMDTPwrCounterDesc& counterDesc)
    {
        return IsCounterAggregationType(counterDesc, AMDT_PWR_VALUE_SINGLE);
    }

    bool IsCumulativeCounter(AMDTPwrCounterDesc& counterDesc)
    {
        return IsCounterAggregationType(counterDesc, AMDT_PWR_VALUE_CUMULATIVE);
    }

    bool IsHistogramCounter(AMDTPwrCounterDesc& counterDesc)
    {
        return IsCounterAggregationType(counterDesc, AMDT_PWR_VALUE_HISTOGRAM);
    }

    AMDTResult StartProfiling();
    AMDTResult StopProfiling();
    AMDTResult PauseProfiling();
    AMDTResult ResumeProfiling();

    AMDTResult LaunchTargetApp();
    bool ResumeLaunchedApp()
    {
        bool retVal = false;

        if (IsAppLaunched())
        {
            retVal = osResumeSuspendedProcess(m_launchedProcessId, m_launchedProcessHandle, m_launchedProcessThreadHandle, true);
        }

        return retVal;
    }
    bool IsAppLaunched() const { return m_isAppLaunched; }
    bool IsLaunchedAppAlive()
    {
        bool isAlive = false;

        if (IsAppLaunched())
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            osIsProcessAlive(m_launchedProcessId, isAlive);
#else
            int status = 0;
            pid_t waitPid;

            waitPid = waitpid(m_launchedProcessId, &status, WNOHANG);

            bool isExited = (-1 == waitPid) || ((m_launchedProcessId == waitPid) && (WIFEXITED(status)));
            isAlive = !isExited;
#endif
        }

        return isAlive;
    }
    void TerminateLaunchedApp() { osTerminateProcess(m_launchedProcessId); }

    AMDTResult GetSampleData(AMDTUInt32* pNbrSamples, AMDTPwrSample** ppSampleData);

    AMDTResult GetHistogramCounters(AMDTUInt32* pNumHistograms, AMDTPwrHistogram* ppHistData);

    AMDTResult GetCumulativeCounters(AMDTUInt32* pNumHistograms, AMDTFloat32* pHistData, AMDTUInt32* pCounterId);

    AMDTUInt32 GetTotalNumberOfSamples() { return m_totalNbrSamples; }

    AMDTUInt32 GetNumberOfFailedReads() { return m_NbrOfFailedReads; }

    AMDTUInt64 GetTotalElapsedTime() { return m_totalElapsedTime; }

    void Clear()
    {
        if (m_isProfilingEnabled)
        {
            StopProfiling();

            AMDTPwrProfileClose();

            // FIXME: this should be part of API: Linux ONLY
            //RemovePcoreModule();

            m_isProfilingEnabled = false;
        }

        for (auto pDevice : m_dbDeviceVec)
        {
            delete pDevice;
        }

        m_dbDeviceVec.clear();

        for (auto pCounter : m_dbCounterDescVec)
        {
            delete pCounter;
        }

        m_dbCounterDescVec.clear();
        m_dbEnabledCountersVec.clear();

        if (nullptr != m_pDbAdapter)
        {
            // Flush database
            m_pDbAdapter->FlushDb();

            delete m_pDbAdapter;
            m_pDbAdapter = nullptr;
        }
    }

    gtString GetProfileStartTime() const { return m_profStartTime; }
    gtString GetProfileEndTime() const { return m_profEndTime; }

    // DB specific functions
    bool InitDb(AMDTProfileSessionInfo& sessionInfo);
    bool WriteSamplesIntoDb();

private:
    ppParseArgs&              m_args;
    AMDTResult                m_error;

    bool                      m_isProfilingEnabled;
    AMDTPwrProfileState       m_profileState;

    unsigned int              m_launchedPid;

    gtString                  m_profStartTime;
    gtString                  m_profEndTime;

    // Device details
    AMDTPwrDevice*            m_pDeviceTopology;
    AMDTPwrDeviceNameDescMap  m_deviceNameDescMap;   // Device Name - Device Desc Map
    AMDTPwrDeviceIdDescVec    m_deviceIdDescVec;     // Device ID-Desc vector
    AMDTPwrDeviceIdNameVec    m_deviceIdNameVec;     // Device ID-Name vector

    // Counter details
    AMDTUInt32                m_nbrSupportedCounters;
    AMDTPwrCounterDesc*       m_pSupportedCountersDesc;

    AMDTPwrCounterIdDescVec   m_supportedCounterIdDescVec;    // ID-Desc vector
    AMDTPwrCounterIdNameVec   m_supportedCounterIdNameVec;    // ID-Name vector
    AMDTPwrCounterNameDescMap m_supportedCounterNameDescMap;  // Name-Desc map

    AMDTUInt64                m_totalElapsedTime;
    AMDTUInt32                m_totalNbrSamples;
    AMDTUInt32                m_NbrOfFailedReads;
    AMDTUInt32                m_nbrSamples;
    AMDTPwrSample*            m_pSampleData;

    // list of counters to be profiled
    gtVector<AMDTUInt32>      m_simpleCounterIdVec;
    gtVector<AMDTUInt32>      m_cumulativeCounterIdVec;
    gtVector<AMDTUInt32>      m_histogramCounterIdVec;
    gtVector<AMDTUInt32>      m_profiledCounterIdVec; // vector of all profiled counter-ids

    osFilePath                m_outputFilePath;   // constructed outputfile path either user-provided or default path + timestamp

    // core-mask of available cores in target system
    AMDTUInt64                m_coreMask;

    // launched target app
    osProcessId              m_launchedProcessId;
    osProcessHandle          m_launchedProcessHandle;
    osThreadHandle           m_launchedProcessThreadHandle;
    bool                     m_isAppLaunched;
    bool                     m_isTemperatureSupported;

    // DB specific members
    amdtProfileDbAdapter*               m_pDbAdapter;
    gtVector<AMDTProfileCounterDesc*>   m_dbCounterDescVec;
    gtVector<AMDTProfileDevice*>        m_dbDeviceVec;
    gtVector<int>                       m_dbEnabledCountersVec;
    bool                                m_isDbExportEnabled;

    void ValidateProfileOptions();
    AMDTResult EnableProfiling();
    AMDTResult ValidateCounters();
    AMDTResult ConfigureProfile();

    bool IsValidCoreMask(gtUInt64 coreMask);

    gtString GetTimeStr()
    {
        gtString profTime;
        osTime currentTime;
        currentTime.setFromCurrentTime();
        currentTime.dateAsString(profTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);

        return profTime;
    }

    bool isProfilingEnabled() const { return m_isProfilingEnabled; }

    bool isProfileState(AMDTPwrProfileState profState)
    {
        AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
        AMDTPwrGetProfilingState(&state);

        // m_error = (state == profState) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        return (state == profState) ? true : false;
    }

    bool IsCounterAggregationType(AMDTPwrCounterDesc& counterDesc, AMDTPwrAggregation aggregationType)
    {
        return (aggregationType == counterDesc.m_aggregation) ? true : false;
    }

    // FIMXE: This should be part of API
    AMDTResult LoadPcoreModule();
    void RemovePcoreModule();

    bool IsValidCounterId(AMDTUInt32 counterId);
    bool AddValidCounterId(AMDTUInt32 counterId);
    bool GetCounterAggregationType(AMDTUInt32 counterId, AMDTPwrAggregation& aggregationType);

    bool AddCounterGroups();
    bool AddCounters(AMDTDeviceType deviceId, AMDTPwrCategory category);

    bool isProfileStateUninitialized() { return isProfileState(AMDT_PWR_PROFILE_STATE_UNINITIALIZED); }
    bool isProfileStateReady()  { return isProfileState(AMDT_PWR_PROFILE_STATE_IDLE); }
    bool isProfileStateRunning()  { return isProfileState(AMDT_PWR_PROFILE_STATE_RUNNING); }
    bool isProfileStatePaused() { return isProfileState(AMDT_PWR_PROFILE_STATE_PAUSED); }

    bool isOK() const { return (AMDT_STATUS_OK == m_error) ? true : false; }
};

#endif // #ifndef _PP_COLLECT_H_
