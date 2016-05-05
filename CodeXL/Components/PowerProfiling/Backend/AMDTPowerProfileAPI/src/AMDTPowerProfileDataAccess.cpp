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

static bool g_isOnline = false;
static PowerProfileTranslate* g_pTranslate = nullptr;
static AMDTPwrProfileConfig g_currentCfg;

osCriticalSection driverCriticalSection;

static AMDTPwrProcessedDataRecord* pProcData = nullptr;
static AMDTPwrProcessedDataRecord* pprevProcData = nullptr;

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

// AMDTGetProcessProfileData: Get Process profile infornation list.
// This is a list of PIDs and their corresponding power indicators
AMDTResult AMDTGetProcessProfileData(AMDTUInt32* pPIDCount, AMDTPwrProcessInfo** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (g_aggrPidPowerList.m_numberOfPids > 0)
    {
        *pPIDCount = g_aggrPidPowerList.m_numberOfPids;
        *ppData = &g_aggrPidPowerList.m_process[0];
    }
    else
    {
        ret = AMDT_ERROR_NODATA;
        PwrTrace("data not avaiolable");
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
