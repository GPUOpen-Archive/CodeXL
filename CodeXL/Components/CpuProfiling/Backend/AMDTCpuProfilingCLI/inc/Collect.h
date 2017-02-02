//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Collect.h
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#ifndef _CPUPROFILE_COLLECT_H_
#define _CPUPROFILE_COLLECT_H_
#pragma once

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Backend:
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>
#include <AMDTCpuPerfEventUtils/inc/DcConfig.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/PeFile.h>
#endif // AMDT_WINDOWS_OS

// Project:
#include <ParseArgs.h>

// Macros
#define CODEXL_DEFAULT_OUTPUTFILE_NAME    L"CodeXL-CpuProfile"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define CPUPROFILE_RAWFILE_EXTENSION        L"prd"
    #define CPUPROFILE_RAWFILE_EXT_STR          "prd"
    #define THREAD_PROFILE_RAWFILE_EXTENSION    L"cxltp"
    #define THREAD_PROFILE_RAWFILE_EXT_STR      "cxltp"
#else
    #define CPUPROFILE_RAWFILE_EXTENSION        L"caperf"
    #define CPUPROFILE_RAWFILE_EXT_STR          "caperf"
    #define THREAD_PROFILE_RAWFILE_EXTENSION    L"cxltp"
    #define THREAD_PROFILE_RAWFILE_EXT_STR      "cxltp"
#endif

struct CpuProfilePmcEventCount
{
    gtUInt32    m_coreId = 0;
    gtUInt32    m_nbrEvents = 0;
    gtUInt64    m_eventConfig[6];  // Max six counters
    gtUInt64    m_eventCountValue[6];  // Max six counters
};

typedef enum CpuProfileStates
{
    CPUPROF_STATE_INVALID = 0,
    CPUPROF_STATE_READY,
    CPUPROF_STATE_PROFILING,
    CPUPROF_STATE_PAUSED
} CpuProfileState;


class CpuProfileCollect
{
public:
    CpuProfileCollect(ParseArgs& args) : m_args(args), m_isProfilingEnabled(false), m_profileState(CPUPROF_STATE_INVALID),
        m_launchedPid(0), m_error(S_OK)
    {
    }

    ~CpuProfileCollect()
    {
        StopProfiling();
        fnReleaseProfiling();

        m_pmcEventMsrMap.clear();
    };

    CpuProfileCollect& operator=(const CpuProfileCollect&) = delete;
    HRESULT Initialize();

    void SetLaunchedPid(int pid) { m_launchedPid = pid; }
    unsigned int GetLaunchedPid() const { return m_launchedPid; }

    HRESULT StartProfiling();
    HRESULT StartProfiling(unsigned int* pPidArray, int numPids);
    HRESULT StopProfiling();

    // Thread Profiling
    HRESULT StartTPProfiling();
    HRESULT StopTPProfiling();

    gtString GetProfileTypeStr();

    gtString GetProfileStartTime() const { return m_profStartTime; }
    gtString GetProfileEndTime() const { return m_profEndTime; }

    bool IsTP()
    {
#ifdef CXL_ENABLE_TP
        return (0 == m_args.GetProfileConfig().compareNoCase(L"tp")) ? true : false;
#else
        return false;
#endif // CXL_ENABLE_TP
    }

    bool IsTBP() { return (0 == m_args.GetProfileConfig().compareNoCase(L"tbp")) ? true : false; }
    bool IsCLU() { return (0 == m_args.GetProfileConfig().compareNoCase(L"clu")) ? true : false; }
    bool IsCSSEnabled() { return (m_args.IsCSSEnabled() || m_args.IsCSSWithDefaultValues()); };

    bool IsIbsOPSampling()
    {
        IbsConfig ibsConfig;
        m_profileDcConfig.GetIBSInfo(ibsConfig);
        return ibsConfig.opSampling;
    }

    float GetTbpSamplingInterval() { return m_profileDcConfig.GetTimerInterval(); }

    bool IsClrApp()
    {
        bool retVal = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        PeFile exe(m_args.GetLaunchApp().asCharArray());

        if (exe.Open() && exe.FindClrInfo())
        {
            retVal = true;
            exe.Close();
        }

#endif // AMDT_WINDOWS_OS

        return retVal;
    }

    bool hasCountModeEvents() const { return (m_nbrCountEvents > 0) ? true : false; }
    void GetCountEventValues(gtVector<CpuProfilePmcEventCount>& value) const { value = m_eventCountValuesVec; }

private:
    ParseArgs&          m_args;
    bool                m_isProfilingEnabled;
    int                 m_profileState;

    unsigned int        m_launchedPid;

    HRESULT             m_error;

    gtString            m_profStartTime;
    gtString            m_profEndTime;

    DcConfig            m_profileDcConfig;  // Profile Config

    osFilePath          m_outputFilePath;   // constructed outputfile path either user-provided or default path + timestamp

    gtUInt64            m_nbrSampleEvents = 0;
    gtUInt64            m_nbrCountEvents = 0;
    gtVector<CpuProfilePmcEventCount>  m_eventCountValuesVec;
    gtVector<gtUInt64>  m_countEventVec;
    gtMap<gtUInt64, gtUInt32>   m_pmcEventMsrMap;
    bool                        m_hasPmcEventMsrMap = false;

    void SetupEnvironment();
    void EnableProfiling();
    void ValidateProfile();
    void VerifyAndSetEvents(EventConfiguration** ppDriverSampleEvents, EventConfiguration** ppDriverCountEvents);
    bool ProcessRawEvent(gtVector<DcEventConfig>& eventConfigVec, IbsConfig& ibsConfig, int& timerInterval);
    bool ProcessRawEventId(const gtString& eventStr, gtVector<DcEventConfig>& eventConfigVec, IbsConfig& ibsConfig, int& timerInterval);
    bool ProcessRawEventStr(const gtString& eventStr, gtVector<DcEventConfig>& eventConfigVec, IbsConfig& ibsConfig, int& timerInterval);

    void ConfigureProfile();

    void SetTbpConfig();
    void SetEbpConfig();
    void SetIbsConfig();
    void SetTPConfig();

    void SetOutputFilePath();
    void GetOutputFilePath(osFilePath& outfilePath);

    void WriteRunInfo();

    gtString GetTimeStr()
    {
        gtString profTime;
        osTime currentTime;
        currentTime.setFromCurrentTime();
        currentTime.dateAsString(profTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);

        return profTime;
    }

    bool isProfilingEnabled() const { return m_isProfilingEnabled; }
    bool isStateReady() const { return (m_profileState == CPUPROF_STATE_READY) ? true : false; }
    bool isStateProfiling() const { return (m_profileState == CPUPROF_STATE_PROFILING) ? true : false; }
    bool isStateInvalid() const { return (m_profileState == CPUPROF_STATE_INVALID) ? true : false; }
    bool isOK() const { return (S_OK == m_error) ? true : false; }
};

#endif // #ifndef _CPUPROFILE_COLLECT_H_
