//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileDataAccess.cpp
///
//==================================================================================

#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>

#include <RawDataReader.h>
#include "PowerProfileTranslate.h"
#include <AMDTPowerProfileDataAccess.h>
#include <PowerProfileHelper.h>
#include <PowerProfileDriverInterface.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>

static bool g_isOnline = false;
static PowerProfileTranslate* g_pTranslate = nullptr;
static AMDTPwrProfileConfig g_currentCfg;
static AMDTPwrProcessInfo g_pidInfo[MAX_PID_CNT];

osCriticalSection driverCriticalSection;

static AMDTPwrProcessedDataRecord* pProcData = nullptr;
static AMDTPwrProcessedDataRecord* pprevProcData = nullptr;
std::unordered_map<AMDTUInt32, AMDTPwrProcessInfo> g_lastAggrPidPwrMap;

extern PowerData   g_aggrPidPowerList;

// DriverDataMonitor: Function to monitor the driver events and process the raw data
void DriverDataMonitor(DriverSignalInfo* pInfo)
{
    // Handle the signal
    pInfo->m_fill = 0;

    if (nullptr != g_pTranslate)
    {
        g_pTranslate->TranslateRawData();
    }
}

//AMDTPwrOpenOnlineDataAccess:
AMDTResult AMDTPwrOpenOnlineDataAccess(void* pParam)
{
    pParam = pParam;

    AMDTUInt32 ret = AMDT_STATUS_OK;
    AMDTUInt64 flag = 0x01; //To indicate online

    g_isOnline = true;
    g_pTranslate = new PowerProfileTranslate();

    memset(&g_currentCfg, 0, sizeof(AMDTPwrProfileConfig));

    if (nullptr == g_pTranslate)
    {
        ret = AMDT_ERROR_OUTOFMEMORY;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = g_pTranslate->InitPowerTranslate(nullptr, flag);
    }

    if (AMDT_STATUS_OK == ret)
    {
        pProcData = nullptr;
        pprevProcData = nullptr;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = DriverSignalInitialize(&DriverDataMonitor, g_pTranslate->GetSamplingPeriod());
    }

    return ret;
}

//AMDTPwrOpenOfflineDataAccess:
AMDTResult AMDTPwrOpenOfflineDataAccess(
    /*in*/ const wchar_t* pFileName)
{
    pFileName = pFileName;

    int ret = AMDT_STATUS_OK;

    //Offline mode will be supported in future.
    return ret;
}

//AMDTPwrCloseDataAccess:
AMDTResult AMDTPwrCloseDataAccess()
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    FlushDriverSignal();

    g_pTranslate->ClosePowerProfileTranslate();

    //Release all allocated memory

    if (g_pTranslate)
    {
        delete g_pTranslate;
        g_pTranslate = nullptr;
    }

    pProcData = nullptr;
    pprevProcData = nullptr;

    return ret;
}


//AMDTPwrGetProfileDuration:
AMDTResult AMDTPwrGetProfileTimeStamps(
    /*in*/ AMDTUInt64* pStartTime, /*in*/ AMDTUInt64* pEndTime)
{
    AMDTResult ret = AMDT_ERROR_FAIL;

    if ((nullptr != pStartTime) || (nullptr != pEndTime))
    {
        AMDTUInt64 startTs;
        AMDTUInt64 endTs;

        ret = g_pTranslate->GetSessionTimeStamps(startTs, endTs);

        if (nullptr != pStartTime)
        {
            *pStartTime = startTs;
        }

        if (nullptr != pEndTime)
        {
            *pEndTime = endTs;
        }
    }

    return ret;
}

// AMDTGetCounterValues: Get profile time line counter values.
AMDTResult AMDTGetCounterValues(AMDTPwrProcessedDataRecord* pData)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (nullptr != g_pTranslate)
    {
        ret = g_pTranslate->GetProcessedList(pData);
    }
    else
    {
        ret = AMDT_ERROR_INTERNAL;
        PwrTrace("g_translation = NULL");
    }

    return ret;
}

// GetCummulativePidProfDataFromStart: Get power data for process
AMDTResult GetCummulativePidProfDataFromStart(AMDTUInt32* pPIDCount,
                                              AMDTPwrProcessInfo** ppData,
                                              AMDTUInt32 pidVal)
{
    AMDTResult ret = AMDT_ERROR_NODATA;
    AMDTPwrProcessInfo* pInfo = nullptr;
    AMDTUInt32 entries = 0;
    AMDTFloat32 power = 0;
#ifndef LINUX

    if (nullptr != g_pTranslate)
    {
        g_pTranslate->PwrGetProfileData(PROCESS_PROFILE, (void**)&pInfo, &entries, &power);
        (void)power;
    }

#else
    entries = g_aggrPidPowerList.m_numberOfPids;
    pInfo = &g_aggrPidPowerList.m_process[0]

#endif

    if (entries > 0)
    {
        if (AMD_PWR_ALL_PIDS == pidVal)
        {
            *pPIDCount = entries;
            *ppData = pInfo;
            ret = AMDT_STATUS_OK;
        }
        else
        {
            for (AMDTUInt32 idx = 0; idx < entries; ++idx)
            {
                if (pidVal == pInfo[idx].m_pid)
                {
                    *pPIDCount = 1;
                    *ppData = &pInfo[idx];
                    ret = AMDT_STATUS_OK;
                    break;
                }
            }
        }
    }

    return ret;
}

AMDTResult GetCummulativePidProfDataInstatant(AMDTUInt32* pPIDCount,
                                              AMDTPwrProcessInfo** ppData,
                                              AMDTUInt32 pidVal)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProcessInfo* pInfo = nullptr;
    AMDTUInt32 entries = 0;
    AMDTFloat32 power = 0;
#ifndef LINUX

    if (nullptr != g_pTranslate)
    {
        g_pTranslate->PwrGetProfileData(PROCESS_PROFILE, (void**)&pInfo, &entries, &power);
        (void)power;
    }

#else
    entries = g_aggrPidPowerList.m_numberOfPids;
    pInfo = &g_aggrPidPowerList.m_process[0]

#endif

    // first call to this function
    if (g_lastAggrPidPwrMap.empty())
    {
        if (entries > 0)
        {
            // Copy g_aggrPidPowerData to an unordered-map for later processing
            for (AMDTUInt32 idx = 0; idx < entries; ++idx)
            {
                g_lastAggrPidPwrMap.insert({ pInfo[idx].m_pid, pInfo[idx] });
            }

            if (AMD_PWR_ALL_PIDS == pidVal)
            {
                *pPIDCount = entries;
                *ppData = &pInfo[0];
            }
            else
            {
                // find pid in map
                auto pidFindItr = g_lastAggrPidPwrMap.find(pidVal);

                if (g_lastAggrPidPwrMap.end() != pidFindItr)
                {
                    *pPIDCount = 1;
                    *ppData = &pidFindItr->second;
                }
                else
                {
                    // error
                    ret = AMDT_ERROR_NODATA;
                }
            }
        }
        else
        {
            ret = AMDT_ERROR_NODATA;
        }
    }
    else
    {
        AMDTUInt32 count = 0;
        memset(g_pidInfo, 0, sizeof(AMDTUInt32) * MAX_PID_CNT);

        for (AMDTUInt32 idx = 0; idx < entries; ++idx)
        {
            auto itr = g_lastAggrPidPwrMap.find(pInfo[idx].m_pid);

            if (g_lastAggrPidPwrMap.end() != itr)
            {
                // entry found
                g_pidInfo[count].m_ipc = pInfo[idx].m_ipc - itr->second.m_ipc;
                g_pidInfo[count].m_power = pInfo[idx].m_power - itr->second.m_power;
                g_pidInfo[count].m_sampleCnt = pInfo[idx].m_sampleCnt - itr->second.m_sampleCnt;
            }
            else
            {
                // entry not found
                g_pidInfo[count].m_ipc = pInfo[idx].m_ipc;
                g_pidInfo[count].m_power = pInfo[idx].m_power;
                g_pidInfo[count].m_sampleCnt = pInfo[idx].m_sampleCnt;
            }

            strcpy(g_pidInfo[count].m_name, pInfo[idx].m_name);
            strcpy(g_pidInfo[count].m_path, pInfo[idx].m_path);
            g_pidInfo[count].m_pid = pInfo[idx].m_pid;
            ++count;
        }

        if (AMD_PWR_ALL_PIDS == pidVal)
        {
            *pPIDCount = count;
            *ppData = &g_pidInfo[0];
        }
        else
        {
            for (AMDTUInt32 pid = 0; pid < count; ++pid)
            {
                if (pidVal == g_pidInfo[pid].m_pid)
                {
                    *pPIDCount = 1;
                    *ppData = &g_pidInfo[pid];
                    break;
                }
            }
        }

        if (entries > 0)
        {
            g_lastAggrPidPwrMap.clear();

            // Copy g_aggrPidPowerData
            for (AMDTUInt32 idx = 0; idx < entries; ++idx)
            {
                g_lastAggrPidPwrMap.insert({ g_aggrPidPowerList.m_process[idx].m_pid, g_aggrPidPowerList.m_process[idx] });
            }
        }
    }

    return ret;
}

// AMDTGetCummulativePidProfData: Get Process profile infornation list.
// This is a list of PIDs and their corresponding power indicators
AMDTResult AMDTGetCummulativePidProfData(AMDTUInt32* pPIDCount,
                                         AMDTPwrProcessInfo** ppData,
                                         AMDTUInt32 pidVal,
                                         bool reset)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (false == reset)
    {
        ret = GetCummulativePidProfDataFromStart(pPIDCount, ppData, pidVal);
    }
    else
    {
        ret = GetCummulativePidProfDataInstatant(pPIDCount, ppData, pidVal);
    }

    return ret;
}

// AMDTGetInstrumentedData: Get the accumulated instrumented data
AMDTResult AMDTGetInstrumentedData(AMDTUInt32 markerId, PwrInstrumentedPowerData** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if ((nullptr != g_pTranslate)
        && (nullptr != ppData)
        && (0 != markerId))
    {
        ret = g_pTranslate->GetInstrumentedData(markerId, ppData);
    }
    else
    {
        ret = AMDT_ERROR_INTERNAL;
        PwrTrace("Invalid pointer g_translation or pInfo");
    }

    return ret;
}

// PwrGetModuleProfileData: module level profiling data.
// this api can be called at any point of profile state from start to end of the session
AMDTResult PwrGetModuleProfileData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCount, AMDTFloat32* pPower)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if ((nullptr != g_pTranslate)
        && (nullptr != ppData)
        && (nullptr != pModuleCount))
    {
        ret = g_pTranslate->PwrGetProfileData(MODULE_PROFILE, (void**)ppData, pModuleCount, pPower);
    }
    else
    {
        ret = AMDT_ERROR_INTERNAL;
        PwrTrace("Invalid pointer g_translation or pInfo");
    }

    return ret;
}
