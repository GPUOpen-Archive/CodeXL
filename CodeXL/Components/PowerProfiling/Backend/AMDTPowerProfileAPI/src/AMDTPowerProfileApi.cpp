//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileApi.cpp
///
//==================================================================================

#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTPowerProfileApi.h>
#include <AMDTPowerProfileControl.h>
#include <AMDTPowerProfileDataAccess.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTPowerProfileApiInternal.h>
#include <AMDTHistogram.h>
#include <PowerProfileHelper.h>

#include <algorithm>
#include <atomic>
#include <list>
#include <mutex>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <thread>

#ifdef LINUX
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
void __attribute__((constructor)) initPowerProfileDriverInterface(void);
void __attribute__((destructor))  finiPowerProfileDriverInterface(void);

void __attribute__((constructor)) initPowerProfileDriverInterface(void)
{
    //TODO: Setup a signal handler which will do a cleanup in case of
    // any segfaults so that the next operations will go on smoothly.
    return;
}

void __attribute__((destructor))  finiPowerProfileDriverInterface(void)
{
    // Ideally when we reach here, the  Profiling Should have stopped.
    //  If not Make sure we pass the stop timer and unregister calls to the driver
    AMDTPwrStopProfiling();
    AMDTPwrProfileClose();

}
#endif

bool g_internalCounters = false;
bool g_isSVI2Supported = false;
// Create API layer pool.
static MemoryPool g_apiMemoryPool;
#define API_POOL_SIZE 1048576 // 1MB

static AMDTUInt32 g_samplingPeriod = 0;
static AMDTPwrProfileMode g_profileMode;

//Default option for output data is AMDT_PWR_SAMPLE_VALUE_INSTANTANEOUS
static AMDTSampleValueOption g_outputOption = AMDT_PWR_SAMPLE_VALUE_INSTANTANEOUS;

// Cofigured counter list from client
static std::vector<AMDTUInt32> g_activeCounters;

// Filter counter list from output list
static std::vector <AMDTUInt32> g_filterCounters;

// Supported counter count
AMDTUInt32 g_counterCnt = 0;
static CounterInfo g_counterList[MAX_SUPPORTED_COUNTERS];
static AMDTPwrCounterDesc g_desc[MAX_SUPPORTED_COUNTERS];
AMDTInt32 g_histcounterCnt = 0;

// Dynamic list prepared for client request per device
static AMDTPwrCounterDesc* g_pClientList = nullptr;

// Output result list
AMDTPwrSample* g_pResult = nullptr;

// Topology: dynamic list created based of the platform used
static AMDTPwrDevice* g_pTopology;
// Memory pool created to avoid complexity during cleanup
static AMDTPwrDevice* g_pDevPool;

// Total device count
static AMDTUInt32 g_deviceCnt = 0;


// Profile type
AMDTUInt32 g_profileType = PROFILE_TYPE_TIMELINE;

// Cummulative counter list
AMDTFloat32 g_cumulativeCounterList[MAX_COUNTER_CNT];
AMDTPwrHistogram g_histogramCounterList[MAX_COUNTER_CNT];


static CounterMapping g_counterMapping[] = { \
                                             // SMU7 APU specific counters
{ COUNTER_SMU7_APU_PWR_CU, COUNTERID_SMU7_APU_PWR_CU},
{ COUNTER_SMU7_APU_TEMP_CU, COUNTERID_SMU7_APU_TEMP_CU},
{ COUNTER_SMU7_APU_PWR_PCIE, COUNTERID_SMU7_APU_PWR_PCIE},
{ COUNTER_SMU7_APU_PWR_DDR, COUNTERID_SMU7_APU_PWR_DDR},
{ COUNTER_SMU7_APU_PWR_PACKAGE, COUNTERID_SMU7_APU_PWR_PACKAGE},
{ COUNTER_SMU7_APU_PWR_DISPLAY, COUNTERID_SMU7_APU_PWR_DISPLAY},
{ COUNTER_SMU7_APU_PWR_IGPU, COUNTERID_SMU7_APU_PWR_IGPU},
{ COUNTER_SMU7_APU_TEMP_IGPU, COUNTERID_SMU7_APU_TEMP_IGPU},
{ COUNTER_SMU7_APU_FREQ_IGPU, COUNTERID_SMU7_APU_FREQ_IGPU},

// SMU8 APU specific counters
{ COUNTER_SMU8_APU_PWR_CU, COUNTERID_SMU8_APU_PWR_CU},
{ COUNTER_SMU8_APU_TEMP_CU, COUNTERID_SMU8_APU_TEMP_CU},
{ COUNTER_SMU8_APU_PWR_APU, COUNTERID_SMU8_APU_PWR_APU},
{ COUNTER_SMU8_APU_PWR_VDDGFX, COUNTERID_SMU8_APU_PWR_VDDGFX},
{ COUNTER_SMU8_APU_TEMP_VDDFFX, COUNTERID_SMU8_APU_TEMP_VDDGFX},
{ COUNTER_SMU8_APU_FREQ_IGPU, COUNTERID_SMU8_APU_FREQ_IGPU},
{ COUNTER_SMU8_APU_C0_RES, COUNTERID_SMU8_APU_C0STATE_RES},
{ COUNTER_SMU8_APU_C1_RES, COUNTERID_SMU8_APU_C1STATE_RES},
{ COUNTER_SMU8_APU_CC6_RES, COUNTERID_SMU8_APU_CC6_RES},
{ COUNTER_SMU8_APU_PC6_RES, COUNTERID_SMU8_APU_PC6_RES},
{ COUNTER_SMU8_APU_PWR_VDDIO, COUNTERID_SMU8_APU_PWR_VDDIO},
{ COUNTER_SMU8_APU_PWR_VDDNB, COUNTERID_SMU8_APU_PWR_VDDNB},
{ COUNTER_SMU8_APU_PWR_VDDP, COUNTERID_SMU8_APU_PWR_VDDP},
{ COUNTER_SMU8_APU_PWR_UVD, COUNTERID_SMU8_APU_PWR_UVD},
{ COUNTER_SMU8_APU_PWR_VCE, COUNTERID_SMU8_APU_PWR_VCE},
{ COUNTER_SMU8_APU_PWR_ACP, COUNTERID_SMU8_APU_PWR_ACP},
{ COUNTER_SMU8_APU_PWR_UNB, COUNTERID_SMU8_APU_PWR_UNB},
{ COUNTER_SMU8_APU_PWR_SMU, COUNTERID_SMU8_APU_PWR_SMU},
{ COUNTER_SMU8_APU_PWR_ROC, COUNTERID_SMU8_APU_PWR_ROC},
{ COUNTER_SMU8_APU_FREQ_ACLK, COUNTERID_SMU8_APU_FREQ_ACLK},

// Non SMU counters accessed by MSR/PCIe interface
{ COUNTER_CORE_APU_PID, COUNTERID_PID},
{ COUNTER_CORE_APU_TID, COUNTERID_TID},
{ COUNTER_CORE_EFFECTIVE_FREQUENCY, COUNTERID_CEF},
{ COUNTER_CORE_PSTATE, COUNTERID_PSTATE},
{ COUNTER_NODE_TCTL_TEMPERATURE, COUNTERID_NODE_TCTL_TEPERATURE},
{ COUNTER_NCORE_SVI2_CORE_TELEMETRY, COUNTERID_SVI2_CORE_TELEMETRY},
{ COUNTER_NCORE_SVI2_NB_TELEMETRY, COUNTERID_SVI2_NB_TELEMETRY},
{ COUNTER_MAX, COUNTERID_MAX_CNT}
                                           };
static AMDTUInt32 g_smu7PackagePwr[] =
{
    COUNTER_SMU7_APU_PWR_CU,
    COUNTER_SMU7_APU_PWR_PCIE,
    COUNTER_SMU7_APU_PWR_DDR,
    COUNTER_SMU7_APU_PWR_DISPLAY,
    COUNTER_SMU7_APU_PWR_IGPU
};

static AMDTUInt32 g_smu8ApuPwr[] =
{
    COUNTER_SMU8_APU_PWR_CU,
    COUNTER_SMU8_APU_PWR_VDDNB,
    COUNTER_SMU8_APU_PWR_VDDGFX,
    COUNTER_SMU8_APU_PWR_VDDIO,
    COUNTER_SMU8_APU_PWR_VDDP,
    COUNTER_SMU8_APU_PWR_ROC,

};
static AMDTUInt32 g_smu8VddNbPwr[] =
{
    COUNTER_SMU8_APU_PWR_UVD,
    COUNTER_SMU8_APU_PWR_VCE,
    COUNTER_SMU8_APU_PWR_ACP,
    COUNTER_SMU8_APU_PWR_UNB,
    COUNTER_SMU8_APU_PWR_SMU

};

static AMDTPwrCounterHierarchy g_InternalCounterHirarchy[] =
{
    {COUNTER_SMU7_APU_PWR_CU, COUNTER_SMU7_APU_PWR_PACKAGE, 0, nullptr},
    {COUNTER_SMU7_APU_PWR_PCIE, COUNTER_SMU7_APU_PWR_PACKAGE, 0, nullptr},
    {COUNTER_SMU7_APU_PWR_DDR, COUNTER_SMU7_APU_PWR_PACKAGE, 0, nullptr},
    { COUNTER_SMU7_APU_PWR_DISPLAY, COUNTER_SMU7_APU_PWR_PACKAGE, 0, nullptr },
    { COUNTER_SMU7_APU_PWR_IGPU, COUNTER_SMU7_APU_PWR_PACKAGE, 0, nullptr },
    { COUNTER_SMU7_APU_PWR_PACKAGE, COUNTER_SMU7_APU_PWR_PACKAGE, sizeof(g_smu7PackagePwr) / sizeof(AMDTUInt32), g_smu7PackagePwr },
    { COUNTER_SMU8_APU_PWR_CU, COUNTER_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_APU, COUNTER_SMU8_APU_PWR_APU, sizeof(g_smu8ApuPwr) / sizeof(AMDTUInt32), g_smu8ApuPwr },
    { COUNTER_SMU8_APU_PWR_VDDGFX, COUNTER_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_VDDIO, COUNTER_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_VDDNB, COUNTER_SMU8_APU_PWR_APU, sizeof(g_smu8VddNbPwr) / sizeof(AMDTUInt32), g_smu8VddNbPwr },
    { COUNTER_SMU8_APU_PWR_VDDP, COUNTER_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_UVD, COUNTER_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_VCE, COUNTER_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_ACP, COUNTER_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_UNB, COUNTER_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_SMU, COUNTER_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTER_SMU8_APU_PWR_ROC, COUNTER_SMU8_APU_PWR_APU, 0, nullptr },
};

static AMDTUInt32 g_ChildCounterList[COUNTERID_MAX_CNT];

// Macros
#define IS_PROFILE_MODE_OFFLINE (g_profileMode == AMDT_PWR_PROFILE_MODE_OFFLINE)
#define IS_PROFILE_MODE_ONLINE (g_profileMode == AMDT_PWR_PROFILE_MODE_ONLINE)

#define PP_INVALID_COUNTER_ID    0xFFFF

static bool g_histogramEnabled;
static CounterInfo  g_histogramCounters[MAX_SUPPORTED_COUNTERS];
static AMDTUInt32   g_histogramCounterCount = 0;
typedef std::pair<AMDTUInt8, bool> DgpuCountPair;
//#define PWR_API_STUB_ENABLED
#ifdef PWR_API_STUB_ENABLED
#include <AMDTPowerProfileApiStub.cpp>
#else

// AllocateBuffers: Allocate required memory for power profiling
AMDTResult AllocateBuffers()
{
    AMDTResult ret = AMDT_STATUS_OK;

    //Allocate buffers for client device specific counter list
    if (nullptr == g_pClientList)
    {
        g_pClientList = (AMDTPwrCounterDesc*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                                                 sizeof(AMDTPwrCounterDesc) * MAX_SUPPORTED_COUNTERS);
    }

    if (nullptr == g_pClientList)
    {
        ret = AMDT_ERROR_OUTOFMEMORY;
        PwrTrace("g_pClientList memory allocation failed");
    }

    // Allocation for result buffer
    if (AMDT_STATUS_OK == ret)
    {
        if (nullptr != g_pResult)
        {
            ret = AMDT_ERROR_UNEXPECTED;
            PwrTrace("g_pResult is corrupted");
        }

        if (AMDT_STATUS_OK == ret)
        {
            g_pResult = (AMDTPwrSample*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                                            sizeof(AMDTPwrSample) * MAX_SAMPLES_PER_QUERY);

            if (nullptr != g_pResult)
            {
                ret = AMDT_STATUS_OK;
                memset(g_pResult, 0, sizeof(AMDTPwrSample) * MAX_SAMPLES_PER_QUERY);

                for (AMDTUInt32 i = 0; i < MAX_SAMPLES_PER_QUERY; i++)
                {
                    AMDTPwrCounterValue* ptr = nullptr;
                    ptr = (AMDTPwrCounterValue*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                                                    sizeof(AMDTPwrCounterValue) * MAX_SUPPORTED_COUNTERS);

                    if (nullptr != ptr)
                    {
                        memset(ptr, 0, sizeof(AMDTPwrCounterValue) * MAX_SUPPORTED_COUNTERS);
                        g_pResult[i].m_counterValues = ptr;
                    }
                    else
                    {
                        ret = AMDT_ERROR_OUTOFMEMORY;
                        PwrTrace("g_pResult memory allocation failed");
                    }
                }
            }
        }
    }

    // reserve memory for max supported counters
    g_activeCounters.reserve(MAX_SUPPORTED_COUNTERS);

    memset(g_desc, 0, sizeof(AMDTPwrCounterDesc)* MAX_SUPPORTED_COUNTERS);

    //Allocate memory for topology tree
    g_pDevPool = (AMDTPwrDevice*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                                     sizeof(AMDTPwrDevice) * MAX_DEVICE_CNT);

    if (nullptr != g_pDevPool)
    {
        AMDTUInt32 cnt = 0;

        for (cnt = 0; cnt < MAX_DEVICE_CNT; cnt++)
        {
            (g_pDevPool + cnt)->m_pName = (char*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                                                     sizeof(char) * PWR_MAX_NAME_LEN);
            (g_pDevPool + cnt)->m_pDescription = (char*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                                                            sizeof(char) * PWR_MAX_DESC_LEN);
            (g_pDevPool + cnt)->m_pFirstChild = nullptr;
            (g_pDevPool + cnt)->m_pNextDevice = nullptr;
        }
    }
    else
    {
        ret = AMDT_ERROR_OUTOFMEMORY;
    }

    g_counterCnt = 0;
    g_histogramCounterCount = 0;
    g_histogramEnabled = false;
    g_histcounterCnt = 0;

    memset(g_counterList, 0, sizeof(CounterInfo)* MAX_SUPPORTED_COUNTERS);
    memset(g_histogramCounters, 0, sizeof(CounterInfo)* MAX_SUPPORTED_COUNTERS);

    return ret;
}

// SetSmu7ApuPkgPwrCounter: In case of SMU 7 total APU power is sum of
// CU, PCIE, DDR, DISPLAY and IGPU power values.These counters value are added
// to handle the delta observed
void SetSmu7ApuPkgPwrCounter(const AMDTPwrProfileAttributeList& supportedList, AMDTUInt16* counterList,
                             AMDTUInt32& cnt)
{
    // back end IDs for associative  counters
    const AMDTUInt32 attributeIds[6] = { COUNTERID_SMU7_APU_PWR_CU,
                                         COUNTERID_SMU7_APU_PWR_PCIE,
                                         COUNTERID_SMU7_APU_PWR_DDR,
                                         COUNTERID_SMU7_APU_PWR_DISPLAY,
                                         COUNTERID_SMU7_APU_PWR_IGPU
                                       };

    for (AMDTUInt32 attrId : attributeIds)
    {
        for (AMDTUInt16 itr = 0; itr < supportedList.attrCnt; itr++)
        {
            if (attrId == supportedList.pAttrList[itr].m_attrId)
            {
                // enabling  counter to fetch there value,
                // which will be summed to get total apu power
                counterList[cnt++] = itr;
                break;
            }
        }
    }
}

// CleanBuffers: Clean all buffers which are allocated during
// initialization of the profile session
AMDTResult CleanBuffers()
{
    AMDTResult ret = AMDT_STATUS_OK;

    g_pClientList = nullptr;
    g_pResult = nullptr;
    g_pDevPool = nullptr;

    // clear the active counters
    g_activeCounters.clear();

    g_counterCnt = 0;
    g_histcounterCnt = 0;

    // Clear all the histogram counters
    g_histogramCounterCount = 0;
    g_histogramEnabled = false;

    memset(g_histogramCounters, 0, sizeof(CounterInfo)* MAX_SUPPORTED_COUNTERS);
    memset(g_counterList, 0, sizeof(CounterInfo)* MAX_SUPPORTED_COUNTERS);

    g_deviceCnt = 0;

    return ret;
}

// ConvertTimeStamp: Conver Epoc time to seconds and micro seconds
void ConvertTimeStamp(AMDTPwrSystemTime* pTimeStamp , AMDTUInt64 data)
{
    // The Windows ticks are in 100 nanoseconds (10^-7).
#define WINDOWS_TICK_PER_SEC 10000000
    // The windows epoch starts 1601-01-01 00:00:00.
    // It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01 00:00:00).
#define SEC_TO_UNIX_EPOCH 11644473600LL

#ifdef LINUX
    const int millToSec = 1000;
    // recieved data is in milli-seconds convert it in seconds
    pTimeStamp->m_second = data / millToSec;
    // remaing mill-sec
    pTimeStamp->m_microSecond = (data % millToSec) * 1000;
#else
    pTimeStamp->m_second = (AMDTUInt64)(data / WINDOWS_TICK_PER_SEC);
    pTimeStamp->m_microSecond = (AMDTUInt64)((data - (pTimeStamp->m_second * WINDOWS_TICK_PER_SEC)) / 10);

#endif

}

//GetCoreCount: get number of cores from mask.
AMDTUInt8 GetCoreCount(AMDTInt64 n)
{
    AMDTUInt8 c;

    for (c = 0; n; c++)
    {
        n &= n - 1;
    }

    return c;
}


// GetClientCounterId: Get basic counter id from backend id
static BasicCounters GetBaseCounterIdFromBackendId(AMDTPwrProfileAttrType id)
{
    AMDTUInt32 cnt = 0;
    BasicCounters clientId = COUNTER_MAX;

    for (cnt = 0; cnt < COUNTER_MAX; cnt++)
    {
        if ((AMDTUInt32)id == g_counterMapping[cnt].m_backendId)
        {
            clientId = (BasicCounters)g_counterMapping[cnt].m_clientId;
            break;
        }
    }

    return clientId;
}

// GetBackendCounterId: get backend counter id from basic counter id
static AMDTPwrProfileAttrType GetBackendCounterIdFromBaseId(BasicCounters id)
{
    AMDTUInt32 cnt = 0;
    AMDTPwrProfileAttrType clientId = COUNTERID_APU_MAX_CNT;

    for (cnt = 0; cnt < COUNTER_MAX; cnt++)
    {
        if ((AMDTUInt32)id == g_counterMapping[cnt].m_clientId)
        {
            clientId = (AMDTPwrProfileAttrType)g_counterMapping[cnt].m_backendId;
            break;
        }
    }

    return clientId;
}

// GetBaseCounterInfoFromClientId: Comple base counter information from a
// client space counter id
static CounterInfo* GetBaseCounterInfoFromClientId(AMDTUInt32 ClientId)
{
    AMDTUInt32 cnt = 0;

    for (cnt = 0; cnt < g_counterCnt; cnt++)
    {
        if (ClientId == g_counterList[cnt].m_pDesc->m_counterID)
        {
            return &g_counterList[cnt];
        }
    }

    return nullptr;
}

// GetBaseCounterInfoFromClientId: Comple base counter information from a
// client space counter id
static AMDTUInt32 GetClientIdFromBaseCounterId(AMDTUInt32 baseId, AMDTUInt32 instId)
{
    AMDTUInt32 cnt = 0;

    for (cnt = 0; cnt < g_counterCnt; cnt++)
    {
        if ((baseId == (AMDTUInt32)g_counterList[cnt].m_basicCounterId) && (instId == g_counterList[cnt].m_instanceId))
        {
            return g_counterList[cnt].m_pDesc->m_counterID;
        }
    }

    return AMDT_STATUS_OK;
}

// AddInstanceString: Add instance type string with the base counter name
void AddInstanceString(char* pName, char* pDesc, AMDTPwrAttributeTypeInfo* pInfo, AMDTUInt32 instId)
{
    // Description length is greater than name length so we use it as the size of the temp buffer becasue it supports both cases.
    char tempString[PWR_MAX_DESC_LEN];

    if (nullptr != pInfo)
    {
        if (INSTANCE_TYPE_NONCORE_SINGLE == pInfo->m_instanceType)
        {
            //No need to add anything
        }

        if (INSTANCE_TYPE_PER_CU == pInfo->m_instanceType)
        {
            sprintf(tempString, "CPU CU%d %s", instId, pName);
            strcpy(pName, tempString);
        }
        else if (INSTANCE_TYPE_PER_CORE == pInfo->m_instanceType)
        {
            sprintf(tempString, "CPU Core%d %s", instId, pName);
            strcpy(pName, tempString);
        }
        else if (INSTANCE_TYPE_NONCORE_MULTIVALUE == pInfo->m_instanceType)
        {
            if (g_internalCounters && g_isSVI2Supported)
            {
                if (COUNTERID_SVI2_CORE_TELEMETRY == pInfo->m_attrId)
                {
                    memset(pName, 0, PWR_MAX_NAME_LEN);
                    memset(pDesc, 0, PWR_MAX_DESC_LEN);

                    if (0 == instId)
                    {
                        sprintf(pName, "Measured CPU Core Voltage - SVI2");
                        sprintf(pDesc, "CPU Core Voltage measured by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry, reported in Volts.");
                    }
                    else
                    {
                        sprintf(pName, "Measured CPU Core Current - SVI2");
                        sprintf(pDesc, "CPU Core Current measured by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry, reported in Milli-Amperes.");
                    }
                }
                else if (COUNTERID_SVI2_NB_TELEMETRY == pInfo->m_attrId)
                {
                    memset(pName, 0, PWR_MAX_NAME_LEN);
                    memset(pDesc, 0, PWR_MAX_DESC_LEN);

                    if (0 == instId)
                    {
                        sprintf(pName, "Measured NB Voltage - SVI2");
                        sprintf(pDesc, "North-Bridge Voltage measured by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry, reported in Volts.");
                    }
                    else
                    {
                        sprintf(pName, "Measured NB Current - SVI2");
                        sprintf(pDesc, "North-Bridge Current measured by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry, reported in Milli-Amperes.");
                    }
                }
            }
        }
    }
}

// AllocateDevice: allocate device from free pool
AMDTPwrDevice* AllocateDevice(AMDTUInt32* usedCnt)
{
    AMDTPwrDevice* dev = g_pDevPool + *usedCnt;

    if (nullptr != dev)
    {
        *usedCnt += 1;
    }

    dev->m_isAccessible = false;

    return (nullptr != dev) ? dev : nullptr;
}
#define DGPU_AGGREGATED_COUNTER_OFFSET 7
// FillCounterDetails: Populate the counter details
void FillCounterDetails(AMDTUInt32 deviceId,
                        AMDTUInt32 instId,
                        AMDTUInt32 baseId,
                        AMDTPwrAttributeInstanceType instType,
                        AMDTPwrAggregation aggr,
                        AMDTPwrAttributeTypeInfo* pData)
{
    AMDTUInt32 repeatCnt = 1;
    AMDTUInt32 loop = 0;
    char* desc;
    char* name;

    if ((COUNTERID_SVI2_CORE_TELEMETRY == pData->m_attrId)
        || (COUNTERID_SVI2_NB_TELEMETRY == pData->m_attrId))
    {
        if (g_isSVI2Supported)
        {
            //Voltage & current
            repeatCnt = 2;
        }
        else
        {
            // CARRIZO doesn't support SVI2 access via PCIe
            repeatCnt = 0;
        }
    }


    while (loop < repeatCnt)
    {
        if ((COUNTERID_SVI2_CORE_TELEMETRY == pData->m_attrId)
            || (COUNTERID_SVI2_NB_TELEMETRY == pData->m_attrId))
        {
            instId = loop;
        }

        g_counterList[g_counterCnt].m_instanceId = instId;
        g_counterList[g_counterCnt].m_pDesc = &g_desc[g_counterCnt];
        //Found client id
        g_counterList[g_counterCnt].m_instanceType = instType;
        g_counterList[g_counterCnt].m_basicCounterId = (BasicCounters)baseId;
        g_counterList[g_counterCnt].m_pDesc->m_counterID
            = (DGPU_COUNTER_BASE_ID > baseId) ? g_counterCnt : baseId + DGPU_AGGREGATED_COUNTER_OFFSET;

        g_counterList[g_counterCnt].m_pDesc->m_aggregation = aggr;
        desc = (char*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                          sizeof(char) * PWR_MAX_DESC_LEN);
        memset(desc, '\0', sizeof(char)* PWR_MAX_DESC_LEN);

        strncpy(desc, pData->m_description, PWR_MAX_DESC_LEN - 1);
        desc[PWR_MAX_DESC_LEN - 1] = '\0';

        name = (char*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                          sizeof(char) * PWR_MAX_NAME_LEN);
        memset(name, '\0', sizeof(char)* PWR_MAX_NAME_LEN);

        strncpy(name, pData->m_name, PWR_MAX_NAME_LEN - 1);
        name[PWR_MAX_NAME_LEN - 1] = '\0';

        AddInstanceString(name, desc, pData, instId);

        g_counterList[g_counterCnt].m_pDesc->m_name = name;
        g_counterList[g_counterCnt].m_pDesc->m_description = desc;
        g_counterList[g_counterCnt].m_pDesc->m_deviceId = deviceId;
        g_counterList[g_counterCnt].m_pDesc->m_units = (AMDTPwrUnit)pData->m_unitType;
        g_counterList[g_counterCnt].m_pDesc->m_category = (AMDTPwrCategory)pData->m_category;

        if ((COUNTERID_SVI2_CORE_TELEMETRY == pData->m_attrId) || (COUNTERID_SVI2_NB_TELEMETRY == pData->m_attrId))
        {
            if (1 == instId)
            {
                g_counterList[g_counterCnt].m_pDesc->m_units = AMDT_PWR_UNIT_TYPE_MILLI_AMPERE;
                g_counterList[g_counterCnt].m_pDesc->m_category = AMDT_PWR_CATEGORY_CURRENT;
            }
            else
            {
                g_counterList[g_counterCnt].m_pDesc->m_units = AMDT_PWR_UNIT_TYPE_VOLT;
                g_counterList[g_counterCnt].m_pDesc->m_category = AMDT_PWR_CATEGORY_VOLTAGE;
            }
        }

        //backend id needs to be stored
        g_counterList[g_counterCnt].m_backendId = pData->m_attrId;
        g_counterCnt++;

        if (AMDT_PWR_VALUE_CUMULATIVE == aggr || AMDT_PWR_VALUE_HISTOGRAM == aggr)
        {
            g_histcounterCnt++;
        }

        loop++;
    }
}

// GetBaseCounterInfo: provide the backend counter information
// from basic counter
AMDTPwrAttributeTypeInfo* GetBaseCounterInfo(BasicCounters counterId)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileAttributeList list;
    AMDTPwrAttributeTypeInfo* pData = nullptr;
    bool found = false;
    ret = PwrGetSupportedAttributeList(&list);

    if (AMDT_STATUS_OK == ret)
    {
        AMDTUInt32 cnt = 0;

        for (cnt = 0; cnt < list.attrCnt; cnt++)
        {
            BasicCounters basicId;
            pData = (list.pAttrList + cnt);
            basicId = GetBaseCounterIdFromBackendId((AMDTPwrProfileAttrType)pData->m_attrId);

            if (counterId == basicId)
            {
                found = true;
                break;
            }
        }
    }

    return found ? pData : nullptr;
}


// FillAggregatedCounterDetails: Populate the details of aggregated counter infomation
void FillAggregatedCounterDetails(AMDTUInt32 cnt,
                                  AMDTUInt32 deviceId,
                                  AMDTUInt32 instId,
                                  AMDTPwrAttributeInstanceType instType,
                                  AMDTPwrAttributeTypeInfo* pData)
{
    AMDTPwrAttributeTypeInfo tmp;

    switch (cnt)
    {
        case COUNTER_SMU7_APU_FREQ_IGPU:
        {
            strncpy(tmp.m_name, "iGPU Frequency Hist", PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Histogram of Integrated-GPU Frequency.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_MEGA_HERTZ);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_FREQUENCY);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_HISTOGRAM,
                               &tmp);
            break;
        }

        case COUNTER_SMU8_APU_FREQ_IGPU:
        {
            strncpy(tmp.m_name, "GFX Frequency Hist", PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Histogram of Integrated-GPU Frequency.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_MEGA_HERTZ);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_FREQUENCY);
            tmp.m_attrId = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_HISTOGRAM,
                               &tmp);
            break;
        }

        case COUNTER_CORE_EFFECTIVE_FREQUENCY:
        {
            std::string str(pData->m_name);
            str.append(" Hist");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Histogram of CPU Core Effective Frequency.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_MEGA_HERTZ);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_FREQUENCY);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_HISTOGRAM,
                               &tmp);
            break;
        }

        case COUNTER_SMU7_APU_PWR_CU:
        case COUNTER_SMU8_APU_PWR_CU:
        {
            std::string str(pData->m_name);
            str.append(" Cuml");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Cumulative CPU Compute Unit Power, reported in Joules.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_JOULE);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_POWER);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_CUMULATIVE,
                               &tmp);
            break;
        }

        case COUNTER_SMU7_APU_PWR_PACKAGE:
        case COUNTER_SMU8_APU_PWR_APU:
        {
            std::string str(pData->m_name);
            str.append(" Cuml");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Cumulative APU Power, reported in Joules.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_JOULE);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_POWER);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_CUMULATIVE,
                               &tmp);
            break;
        }

        case COUNTER_SMU7_APU_PWR_IGPU:
        {
            std::string str(pData->m_name);
            str.append(" Cuml");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Cumulative Integrated-GPU Power, reported in Joules.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_JOULE);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_POWER);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_CUMULATIVE,
                               &tmp);
            break;
        }

        case COUNTER_SMU8_APU_PWR_VDDGFX:
        {
            std::string str(pData->m_name);
            str.append(" Cuml");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Cumulative VddGfx Power, reported in Joules.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_JOULE);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_POWER);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_CUMULATIVE,
                               &tmp);
            break;
        }

        case DGPU_COUNTER_BASE_ID + COUNTERID_PKG_PWR_DGPU:
        case DGPU_COUNTER_BASE_ID + DGPU_COUNTERS_MAX + COUNTERID_PKG_PWR_DGPU:
        case DGPU_COUNTER_BASE_ID + (DGPU_COUNTERS_MAX * 2) + COUNTERID_PKG_PWR_DGPU:
        case DGPU_COUNTER_BASE_ID + (DGPU_COUNTERS_MAX * 3) + COUNTERID_PKG_PWR_DGPU:
        case DGPU_COUNTER_BASE_ID + (DGPU_COUNTERS_MAX * 4) + COUNTERID_PKG_PWR_DGPU:
        {
            std::string str(pData->m_name);
            str.append(" Cuml");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Cumulative dGPU Power, reported in Joules.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_JOULE);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_POWER);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_CUMULATIVE,
                               &tmp);
            break;
        }

        case DGPU_COUNTER_BASE_ID + COUNTERID_FREQ_DGPU:
        case DGPU_COUNTER_BASE_ID + DGPU_COUNTERS_MAX + COUNTERID_FREQ_DGPU:
        case DGPU_COUNTER_BASE_ID + (DGPU_COUNTERS_MAX * 2) + COUNTERID_FREQ_DGPU:
        case DGPU_COUNTER_BASE_ID + (DGPU_COUNTERS_MAX * 3) + COUNTERID_FREQ_DGPU:
        case DGPU_COUNTER_BASE_ID + (DGPU_COUNTERS_MAX * 4) + COUNTERID_FREQ_DGPU:
        {
            std::string str(pData->m_name);
            str.append(" Hist");
            strncpy(tmp.m_name, str.c_str(), PWR_MAX_NAME_LEN);
            strncpy(tmp.m_description, "Histogram of External-GPU Frequency.", PWR_MAX_DESC_LEN);
            tmp.m_unitType = static_cast<AMDTPwrAttributeUnitType>(AMDT_PWR_UNIT_TYPE_MEGA_HERTZ);
            tmp.m_category = static_cast<PwrCategory>(AMDT_PWR_CATEGORY_FREQUENCY);
            tmp.m_attrId  = pData->m_attrId;
            tmp.m_instanceType = pData->m_instanceType;

            FillCounterDetails(deviceId,
                               instId,
                               cnt,
                               instType,
                               AMDT_PWR_VALUE_HISTOGRAM,
                               &tmp);
            break;
        }

    }

    return;
}

// Is node temprature supported on the platform
bool IsNodeTempSupported(const AMDTPwrTargetSystemInfo& sysInfo)
{
    bool ret = true;

    if (NULL != sysInfo.m_pNodeInfo)
    {
        switch (sysInfo.m_pNodeInfo->m_hardwareType)
        {
            case GDT_CARRIZO:
            case GDT_STONEY:
                ret = false;
                break;

            default:
                break;
        }
    }

    return ret;
}

// InsertDeviceCounters: Create counters for a device
AMDTUInt32 InsertDeviceCounters(AMDTPwrDevice* dev, AMDTUInt32 instId)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 instIdx = 0;
    AMDTUInt32 listCnt = 0;
    AMDTPwrAttributeInstanceType type = INSTANCE_TYPE_NONCORE_SINGLE;
    bool isSmuAvailable = false;
    AMDTPwrTargetSystemInfo sysInfo;
    BasicCounters list[COUNTER_MAX];
    bool isFXSeries = false;

    // Reset the counter list
    memset(list, 0, sizeof(BasicCounters) * COUNTER_MAX);
    memset(&sysInfo, 0, sizeof(AMDTPwrTargetSystemInfo));

    if (AMDT_STATUS_OK == ret)
    {
        // Get the system info
        ret = AMDTPwrGetTargetSystemInfo(&sysInfo);

        if (true == sysInfo.m_isAmdApu)
        {
            // If there is a APU check the APU smu status
            isSmuAvailable = IsIGPUAvailable() && sysInfo.m_smuTable.m_info[0].m_isAccessible;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_DEVICE_PACKAGE == dev->m_type)
        {
            if (true == isSmuAvailable)
            {
                list[listCnt++] = COUNTER_SMU7_APU_PWR_PCIE;
                list[listCnt++] = COUNTER_SMU7_APU_PWR_DDR;
                list[listCnt++] = COUNTER_SMU7_APU_PWR_PACKAGE;
                list[listCnt++] = COUNTER_SMU7_APU_PWR_DISPLAY;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_APU;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_VDDIO;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_VDDNB;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_VDDP;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_UVD;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_VCE;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_ACP;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_UNB;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_SMU;
                list[listCnt++] = COUNTER_SMU8_APU_PWR_ROC;
                list[listCnt++] = COUNTER_SMU8_APU_FREQ_ACLK;
            }

            if (true == sysInfo.m_isPlatformSupported)
            {
                if (true == IsNodeTempSupported(sysInfo))
                {
                    list[listCnt] = COUNTER_NODE_TCTL_TEMPERATURE;
                }
            }


            instIdx = 0;
            type = INSTANCE_TYPE_NONCORE_SINGLE;
        }
        else if ((AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT == dev->m_type)
                 && (true == isSmuAvailable))
        {
            list[listCnt++] = COUNTER_SMU7_APU_PWR_CU;
            list[listCnt++] = COUNTER_SMU7_APU_TEMP_CU;
            list[listCnt++] = COUNTER_SMU8_APU_PWR_CU;
            list[listCnt++] = COUNTER_SMU8_APU_TEMP_CU;
            list[listCnt++] = COUNTER_SMU8_APU_C0_RES;
            list[listCnt++] = COUNTER_SMU8_APU_C1_RES;
            list[listCnt++] = COUNTER_SMU8_APU_CC6_RES;
            list[listCnt] = COUNTER_SMU8_APU_PC6_RES;
            instIdx = instId;
            type = INSTANCE_TYPE_PER_CU;
        }
        else if (AMDT_PWR_DEVICE_CPU_CORE == dev->m_type)
        {
            if ((0x15 == sysInfo.m_family) && (sysInfo.m_model <= 0x0f))
            {
                isFXSeries = true;
            }

            if (false == isFXSeries)
            {
                list[listCnt++] = COUNTER_CORE_APU_PID;
                list[listCnt++] = COUNTER_CORE_APU_TID;
                list[listCnt++] = COUNTER_CORE_EFFECTIVE_FREQUENCY;
                list[listCnt] = COUNTER_CORE_PSTATE;
                instIdx = instId;
                type = INSTANCE_TYPE_PER_CORE;
            }
        }
        else if ((AMDT_PWR_DEVICE_INTERNAL_GPU == dev->m_type)
                 && (true == isSmuAvailable))
        {
            list[listCnt++] = COUNTER_SMU7_APU_PWR_IGPU;
            list[listCnt++] = COUNTER_SMU7_APU_TEMP_IGPU;
            list[listCnt++] = COUNTER_SMU7_APU_FREQ_IGPU;

            if (true == sysInfo.m_isPlatformSupported)
            {
                bool isNodeStoney = ((NULL != sysInfo.m_pNodeInfo) &&
                                     (GDT_STONEY == sysInfo.m_pNodeInfo->m_hardwareType)) ? true : false;

                if (false == isNodeStoney)
                {
                    list[listCnt++] = COUNTER_SMU8_APU_PWR_VDDGFX;
                }
            }

            list[listCnt++] = COUNTER_SMU8_APU_TEMP_VDDFFX;
            list[listCnt] = COUNTER_SMU8_APU_FREQ_IGPU;
            instIdx = instId;
            type = INSTANCE_TYPE_NONCORE_SINGLE;
        }
        else if (g_internalCounters
                 && g_isSVI2Supported
                 && (AMDT_PWR_DEVICE_SVI2 == dev->m_type))
        {
            list[listCnt++] = COUNTER_NCORE_SVI2_CORE_TELEMETRY;
            list[listCnt] = COUNTER_NCORE_SVI2_NB_TELEMETRY;
            instIdx = instId;
            type = INSTANCE_TYPE_NONCORE_MULTIVALUE;
        }
        else
        {
            ret = AMDT_ERROR_FAIL;
            PwrTrace("Unknown device type");
        }

    }

    if (AMDT_STATUS_OK == ret)
    {
        AMDTUInt32 cnt = 0;

        for (cnt = 0; cnt <= listCnt; cnt++)
        {
            AMDTUInt32 idx = instIdx;
            //Get backend attribute info
            AMDTPwrAttributeTypeInfo* pData = nullptr;
            pData = GetBaseCounterInfo(list[cnt]);

            // Skip Core effective frequency if it is not supported
            if (false == IsCefSupported())
            {
                continue;
            }

            if (nullptr != pData)
            {
                FillCounterDetails(dev->m_deviceID,
                                   idx,
                                   list[cnt],
                                   type,
                                   AMDT_PWR_VALUE_SINGLE,
                                   pData);

                FillAggregatedCounterDetails(list[cnt],
                                             dev->m_deviceID,
                                             idx,
                                             type,
                                             pData);

            }
        }
    }

    return ret;
}

// InsertDgpuCounters: Insert dgpu counters in the supported counter list
AMDTUInt32 InsertDgpuCounters(AMDTPwrDevice* dev,
                              AMDTPwrProfileAttributeList* pDgpu,
                              AMDTUInt32 instId)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    AMDTUInt32 dgpuCounterId = 0;
    AMDTUInt32 startIdx = DGPU_COUNTER_BASE_ID
                          + (instId - APU_SMU_ID - 1)
                          * DGPU_COUNTERS_MAX;

    if (nullptr == pDgpu)
    {
        ret = AMDT_ERROR_INVALIDARG;
        PwrTrace("pDGPU invalid");
    }
    else
    {
        for (cnt = 0; cnt < pDgpu->attrCnt; cnt++)
        {
            char* desc = nullptr;
            char* name = nullptr;
            AMDTUInt32 idx = 0;
            AMDTPwrAttributeTypeInfo* pData = &pDgpu->pAttrList[cnt];

            if (pData->m_attrId < startIdx)
            {
                continue;
            }

            if (pData->m_attrId >= (startIdx + DGPU_COUNTERS_MAX))
            {
                break;
            }

            idx = pData->m_attrId;
            dgpuCounterId = (idx - DGPU_COUNTER_BASE_ID) % DGPU_COUNTERS_MAX;
            g_counterList[g_counterCnt].m_backendId = idx;
            g_counterList[g_counterCnt].m_instanceId = instId;
            g_counterList[g_counterCnt].m_pDesc = &g_desc[g_counterCnt];
            g_counterList[g_counterCnt].m_instanceType = pData->m_instanceType;
            g_counterList[g_counterCnt].m_pDesc->m_counterID = idx;
            desc = (char*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                              sizeof(char) * PWR_MAX_DESC_LEN);
            strncpy(desc, pData->m_description, PWR_MAX_DESC_LEN - 1);
            name = (char*)GetMemoryPoolBuffer(&g_apiMemoryPool,
                                              sizeof(char) * PWR_MAX_NAME_LEN);

            sprintf(name, "%s ", dev->m_pName);
            strncpy(&name[strlen(name)], pData->m_name, PWR_MAX_NAME_LEN - 1);
            g_counterList[g_counterCnt].m_pDesc->m_counterID = idx;
            g_counterList[g_counterCnt].m_pDesc->m_name = name;
            g_counterList[g_counterCnt].m_pDesc->m_description = desc;
            g_counterList[g_counterCnt].m_pDesc->m_deviceId = dev->m_deviceID;
            g_counterList[g_counterCnt].m_pDesc->m_units    = (AMDTPwrUnit)pData->m_unitType;
            g_counterList[g_counterCnt].m_pDesc->m_category = (AMDTPwrCategory)pData->m_category;
            g_counterCnt++;

            // Insert Dgpu histogram and cummulative counters
            if ((COUNTERID_PKG_PWR_DGPU == dgpuCounterId)
                || (COUNTERID_FREQ_DGPU == dgpuCounterId))
            {
                strncpy(pData->m_name, name, PWR_MAX_NAME_LEN - 1);
                FillAggregatedCounterDetails(idx,
                                             dev->m_deviceID,
                                             0,
                                             INSTANCE_TYPE_NONCORE_SINGLE,
                                             pData);
            }

        }

    }

    return ret;
}

// GetDeviceInfo: get the device information with respect to device id
bool GetDeviceInfo(AMDTUInt32 deviceId, AMDTPwrDevice* pDev)
{
    bool ret = false;
    AMDTUInt32 cnt = 0;

    for (cnt = 0; cnt < MAX_DEVICE_CNT; cnt++)
    {
        if (deviceId == g_pDevPool[cnt].m_deviceID)
        {
            memcpy(pDev, &g_pDevPool[cnt], sizeof(AMDTPwrDevice));
            ret = true;
            break;
        }
    }

    return ret;
}


// PrepareDgpuInstanceTable: Maintain a list of dGPU with the deviceId, enumeration count
// and duplicate flag
void PrepareDgpuInstanceTable(const AMDTPwrTargetSystemInfo& sysInfo,
                              std::unordered_map<AMDTUInt32, DgpuCountPair>& dGpuCntMap)
{
    for (AMDTUInt32 cnt = 0; cnt < sysInfo.m_smuTable.m_count; cnt++)
    {
        const SmuInfo* smuInfo = &sysInfo.m_smuTable.m_info[cnt];

        if (APU_SMU_ID == smuInfo->m_packageId)
        {
            continue;
        }

        PciDeviceInfo* pDevInfo = nullptr;
        GetPciDeviceInfo(smuInfo->m_packageId, &pDevInfo, nullptr);

        if (nullptr != pDevInfo)
        {
            // get the device Id
            AMDTUInt32 deviceId { pDevInfo->m_deviceId };
            // if entry exists in map
            auto itr = dGpuCntMap.find(deviceId);

            if (itr != dGpuCntMap.end())
            {
                // entry does exists, set the duplicate flag to true
                itr->second.second = true;
            }
            else
            {
                // first entry for deviceID, set duplicate flag to false
                // and device count to 1
                dGpuCntMap.insert(std::pair< AMDTUInt32, DgpuCountPair>(deviceId, std::pair<int, bool>(1, false)));
            }
        }
    }
}

// PrepareSystemTopologyInfo: Prepare the system topology
AMDTResult PrepareSystemTopologyInfo()
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 corePerCuCnt = 0;
    AMDTUInt32 cuCnt = 0;
    AMDTPwrDevice* system;
    AMDTPwrTargetSystemInfo sysInfo;
    AMDTUInt32 devCnt = 0;
    AMDTPwrProfileAttributeList dgpuCounters;
    SmuInfo* smuInfo = &sysInfo.m_smuTable.m_info[0];
    bool smuAccesible = false;

    ret = AMDTPwrGetTargetSystemInfo(&sysInfo);

    if ((AMDT_STATUS_OK == ret) && (true == sysInfo.m_isAmdApu))
    {
        cuCnt = sysInfo.m_computeUnitCnt;
        corePerCuCnt = sysInfo.m_coresPerCu;
    }

    // Create the root node as system
    system = AllocateDevice(&devCnt);
    system->m_deviceID = g_deviceCnt++;
    system->m_type = AMDT_PWR_DEVICE_SYSTEM;
    sprintf(system->m_pName, "System");
    sprintf(system->m_pDescription, "Root device");
    system->m_pNextDevice = nullptr;
    g_pTopology = system;

    // Considering only one package at this moment
    AMDTPwrDevice* pkg0 = AllocateDevice(&devCnt);
    system->m_pFirstChild = pkg0;
    pkg0->m_deviceID = g_deviceCnt++;
    pkg0->m_type = AMDT_PWR_DEVICE_PACKAGE;
    sprintf(pkg0->m_pName, "Node-0");
    sprintf(pkg0->m_pDescription, "First node");

    // ONLY if supported AMD platform
    if ((true == sysInfo.m_isAmd) && (true == sysInfo.m_isPlatformSupported))
    {
        InsertDeviceCounters(pkg0, 0);
    }

    AMDTUInt32 cnt = 0;
    AMDTPwrDevice* cuHead = nullptr;
    AMDTUInt32 coreId = 0;

    // Check if APU smu is accesible
    if (1 == smuInfo->m_isAccessible)
    {
        smuAccesible = true;
    }
    else
    {
        // APU SMU is not accessible. Throw and warning message
        if (NULL != sysInfo.m_pNodeInfo && DEVICE_TYPE_APU == sysInfo.m_pNodeInfo->m_deviceType)
        {
            // smu is not available emit warning
            ret = AMDT_WARN_SMU_DISABLED;
        }

        smuAccesible = false;
        PwrTrace(" AMDT_WARN_SMU_DISABLED Apu Smu not accessible");
    }

    for (cnt = 0; cnt < cuCnt; cnt++)
    {
        AMDTUInt32 cnt1 = 0;
        AMDTPwrDevice* newCu = AllocateDevice(&devCnt);
        newCu->m_deviceID = g_deviceCnt++;
        newCu->m_type = AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT;
        sprintf(newCu->m_pName, "CU-%d", cnt);
        sprintf(newCu->m_pDescription, "Compute Unit %d", cnt);

        if (true == smuAccesible)
        {
            InsertDeviceCounters(newCu, cnt);
            newCu->m_isAccessible = true;
        }

        AMDTPwrDevice* coreHead = nullptr;

        for (cnt1 = 0; cnt1 < corePerCuCnt; cnt1++)
        {
            AMDTPwrDevice* newCore = AllocateDevice(&devCnt);
            newCore->m_deviceID = g_deviceCnt++;
            newCore->m_type = AMDT_PWR_DEVICE_CPU_CORE;
            sprintf(newCore->m_pName, "Core-%d", coreId);
            sprintf(newCore->m_pDescription, "Core %d", coreId);
            newCore->m_pFirstChild = nullptr;
            InsertDeviceCounters(newCore, coreId);
            newCore->m_isAccessible = true;
            coreId++;

            if (nullptr == coreHead)
            {
                coreHead = newCore;
            }
            else
            {
                AMDTPwrDevice* temp = coreHead;

                while (nullptr != temp->m_pNextDevice)
                {
                    temp = temp->m_pNextDevice;
                }

                temp->m_pNextDevice = newCore;
            }
        }

        newCu->m_pFirstChild = coreHead;

        if (nullptr == cuHead)
        {
            cuHead = newCu;
        }
        else
        {
            AMDTPwrDevice* temp = cuHead;

            while (nullptr != temp->m_pNextDevice)
            {
                temp = temp->m_pNextDevice;
            }

            temp->m_pNextDevice = newCu;
        }
    }

    // If there no compute units
    if ((0 == cuCnt) && (true == sysInfo.m_isPlatformSupported))
    {
        AMDTPwrDevice* coreHead = nullptr;

        for (cnt = 0; cnt < sysInfo.m_coreCnt; cnt++)
        {
            AMDTPwrDevice* newCore = AllocateDevice(&devCnt);
            newCore->m_deviceID = g_deviceCnt++;
            newCore->m_type = AMDT_PWR_DEVICE_CPU_CORE;
            sprintf(newCore->m_pName, "Core-%d", coreId);
            sprintf(newCore->m_pDescription, "Core %d", coreId);
            InsertDeviceCounters(newCore, coreId);
            newCore->m_isAccessible = true;
            coreId++;
            newCore->m_pFirstChild = nullptr;

            if (nullptr == coreHead)
            {
                coreHead = newCore;
            }
            else
            {
                AMDTPwrDevice* temp = coreHead;

                while (nullptr != temp->m_pNextDevice)
                {
                    temp = temp->m_pNextDevice;
                }

                temp->m_pNextDevice = newCore;
            }
        }

        cuHead = coreHead;
    }

    if (true == sysInfo.m_isAmdApu)
    {
        // check if Igpu is present
        for (cnt = 0; cnt < sysInfo.m_igpuCnt; cnt++)
        {
            AMDTPwrDevice* igpu = AllocateDevice(&devCnt);
            igpu->m_deviceID = g_deviceCnt++;
            igpu->m_type = AMDT_PWR_DEVICE_INTERNAL_GPU;
            sprintf(igpu->m_pName, "Igpu");
            sprintf(igpu->m_pDescription, "Integrated GPU");
            igpu->m_pFirstChild = nullptr;

            // Show iGPU counters only if SMU is accessible
            if (true == smuAccesible)
            {
                InsertDeviceCounters(igpu, 0);
                igpu->m_isAccessible = true;
            }

            if (nullptr == cuHead)
            {
                cuHead = igpu;
            }
            else
            {
                AMDTPwrDevice* temp = cuHead;

                while (nullptr != temp->m_pNextDevice)
                {
                    temp = temp->m_pNextDevice;
                }

                temp->m_pNextDevice = igpu;
            }

        }

        if (g_internalCounters && g_isSVI2Supported)
        {
            //Fill discrete SVI2
            for (cnt = 0; cnt < sysInfo.m_svi2Cnt; cnt++)
            {
                AMDTPwrDevice* svi2 = AllocateDevice(&devCnt);
                svi2->m_deviceID = g_deviceCnt++;
                svi2->m_type = AMDT_PWR_DEVICE_SVI2;
                sprintf(svi2->m_pName, "svi2 telemetry");
                sprintf(svi2->m_pDescription, "Serial Voltage Interface");
                svi2->m_pFirstChild = nullptr;
                InsertDeviceCounters(svi2, 0);
                svi2->m_isAccessible = true;

                if (nullptr == cuHead)
                {
                    cuHead = svi2;
                }
                else
                {
                    AMDTPwrDevice* temp = cuHead;

                    while (nullptr != temp->m_pNextDevice)
                    {
                        temp = temp->m_pNextDevice;
                    }

                    temp->m_pNextDevice = svi2;
                }
            }
        }
    }

    pkg0->m_pFirstChild = cuHead;

    // map contain deviceID and a pair of <device count> and <bool>.
    // device count keep track of device name enumeration while iterating
    // the SMU and a bool will be set if more then one instance
    // of device found.
    std::unordered_map<AMDTUInt32, DgpuCountPair> dGpuCntMap;
    PrepareDgpuInstanceTable(sysInfo, dGpuCntMap);

    //Fill discrete GPUs
    for (cnt = 0; cnt < sysInfo.m_smuTable.m_count; cnt++)
    {
        smuInfo = &sysInfo.m_smuTable.m_info[cnt];

        if (APU_SMU_ID == smuInfo->m_packageId)
        {
            continue;
        }

        PciDeviceInfo* pDevInfo = nullptr;
        AMDTPwrDevice* dgpu = AllocateDevice(&devCnt);
        PwrGetSupportedAttributeList(&dgpuCounters);

        dgpu->m_deviceID = g_deviceCnt++;
        dgpu->m_type = AMDT_PWR_DEVICE_EXTERNAL_GPU;

        //Get the device information
        GetPciDeviceInfo(smuInfo->m_packageId, &pDevInfo, nullptr);

        if (nullptr != pDevInfo)
        {
            auto itr = dGpuCntMap.find(pDevInfo->m_deviceId);

            if (itr->second.second == false)
            {
                strncpy(dgpu->m_pName, pDevInfo->m_shortName, PWR_MAX_NAME_LEN - 1);
            }
            else
            {
                sprintf(dgpu->m_pName, "%s-%d", pDevInfo->m_shortName, itr->second.first);
                // increment the count for next device
                itr->second.first++;
            }

            strncpy(dgpu->m_pDescription, pDevInfo->m_name, PWR_MAX_NAME_LEN - 1);

            dgpu->m_pFirstChild = nullptr;

            if (nullptr == pkg0)
            {
                pkg0 = dgpu;
            }
            else
            {
                AMDTPwrDevice* temp = pkg0;

                while (nullptr != temp->m_pNextDevice)
                {
                    temp = temp->m_pNextDevice;
                }

                temp->m_pNextDevice = dgpu;

                if (1 == smuInfo->m_isAccessible)
                {
                    //Insert counters only if SMU logging is accessible
                    InsertDgpuCounters(dgpu, &dgpuCounters, smuInfo->m_packageId);
                    dgpu->m_isAccessible = true;
                }
                else
                {
                    PwrTrace(" AMDT_WARN_SMU_DISABLED for dGPU-%d", smuInfo->m_packageId - 1);
                    ret = AMDT_WARN_SMU_DISABLED;
                }
            }
        }
    }

    return ret;
}

// AMDTPwrGetNodeTemperature: This API provides the note temperature in tctl
// scale or internally add any offset provided by the end user.
AMDTResult AMDTPwrGetNodeTemperature(AMDTFloat32* pNodeTemp)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrTargetSystemInfo sysInfo;
    ret = AMDTPwrGetProfilingState(&state);
    AMDTUInt32 data = 0;
    AMDTFloat32 temp = 0.0;
    bool isSupported = false;

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    // Check if it is Orochi family
    ret = AMDTPwrGetTargetSystemInfo(&sysInfo);

    // Orochi
    isSupported = ((0x15 == sysInfo.m_family)
                   && ((0x00 == sysInfo.m_model) || ((0x00 < sysInfo.m_model) && (sysInfo.m_model <= 0x0f))));

    isSupported |= ((0x15 == sysInfo.m_family) && ((0x30 <= sysInfo.m_model) && (sysInfo.m_model <= 0x3f))); // KV

    if (false == isSupported)
    {
        ret = AMDT_ERROR_NOTSUPPORTED;
    }

    if (AMDT_STATUS_OK == ret)
    {
        temp = 0;
        ReadPciAddress(0, 0x18, 0x3, 0xA4, &data);
        DecodeTctlTemperature(data, &temp);
        *pNodeTemp = (AMDTFloat32)temp;
    }

    return ret;
}

// isSupportedHypervisor : If hypervisior is enabled power profiling
// is only supported on root partition.
bool isSupportedHypervisor()
{
    bool ret = false;
    osCpuid cpuInfo;

    if (false == cpuInfo.hasHypervisor())
    {
        ret = true;
    }
    else if ((HV_VENDOR_MICROSOFT == cpuInfo.getHypervisorVendorId()) &&
             (true == cpuInfo.isHypervisorRootPartition()))
    {
        ret = true;
    }

    return ret;
}

// AMDTPwrProfileInitialize: This function loads and initializes the AMDT Power Profile drivers.
// This function should be the first one to be called.
AMDTResult AMDTPwrProfileInitialize(AMDTPwrProfileMode profileMode)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileInitParam param;
    AMDTPwrTargetSystemInfo sysInfo;
    AMDTResult result = AMDT_STATUS_OK;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

    memset(&param, 0, sizeof(AMDTPwrProfileInitParam));
    memset(&sysInfo, 0, sizeof(AMDTPwrTargetSystemInfo));

    if ((AMDT_PWR_PROFILE_MODE_ONLINE != profileMode)
        && (AMDT_PWR_PROFILE_MODE_OFFLINE != profileMode))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_STOPPED == state)
        {
            ret = AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED;
            PwrTrace("Failed:AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED");
        }
        else if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED != state)
        {
            ret = AMDT_ERROR_DRIVER_ALREADY_INITIALIZED;
            PwrTrace("Failed:AMDT_ERROR_DRIVER_ALREADY_INITIALIZED");
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (!isSupportedHypervisor())
        {
            ret = AMDT_ERROR_HYPERVISOR_NOT_SUPPORTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Create memory pool for API layer
        ret = CreateMemoryPool(&g_apiMemoryPool, API_POOL_SIZE);
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_profileMode = profileMode;
        param.isOnline = g_profileMode;
        ret = PwrProfileInitialize(&param);

        if (AMDT_STATUS_OK != ret)
        {
            PwrTrace(" PwrProfileInitialize failed, ret = 0x%x", ret);
        }
    }

    if (AMDT_ERROR_NOTSUPPORTED == ret)
    {
        // Platform not supported, close the power profile
        PwrProfileClose();
        ret = AMDT_ERROR_PLATFORM_NOT_SUPPORTED;
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Get the system info
        ret = AMDTPwrGetTargetSystemInfo(&sysInfo);
        PwrTrace("AMDTPwrGetTargetSystemInfo res 0x%x", ret);
        g_internalCounters = PwrIsInternalCounterAvailable();
        g_isSVI2Supported = PwrIsSVISupported(sysInfo);
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Unsupported platforms are detected inside the call to AMDTPwrGetTargetSystemInfo(), so we need
        // to check for them only after the call to AMDTPwrGetTargetSystemInfo().
        if (AMDT_ERROR_PLATFORM_NOT_SUPPORTED == ret)
        {
            // Platform not supported, close the power profile
            PwrProfileClose();
        }

        if (AMDT_STATUS_OK == ret)
        {
            // Allocate internal memory
            ret = AllocateBuffers();
        }

        if (AMDT_STATUS_OK == ret)
        {
            // Overwrite temerature to Measured in case of Godavari, Mulllins and Temash platform
            // Mullins doesn't support Calculated temperature
            bool isGodavari = false;

            if ((0x15 == sysInfo.m_family) && (0x38 == sysInfo.m_model))
            {
                isGodavari = true;
            }

            if ((true == isGodavari) || (PLATFORM_MULLINS == sysInfo.m_platformId))
            {
                AMDTUInt32 cnt = 0;
                AMDTUInt32 listSize = sizeof(g_counterMapping) / sizeof(CounterMapping);

                // Find CU temperature
                for (cnt = 0; cnt < listSize; cnt++)
                {
                    if (COUNTER_SMU7_APU_TEMP_CU == g_counterMapping[cnt].m_clientId)
                    {
                        g_counterMapping[cnt].m_backendId = COUNTERID_SMU7_APU_TEMP_MEAS_CU;
                        break;
                    }
                }

                // Find GPU temperature
                for (cnt = 0; cnt < listSize; cnt++)
                {
                    if (COUNTER_SMU7_APU_TEMP_IGPU == g_counterMapping[cnt].m_clientId)
                    {
                        g_counterMapping[cnt].m_backendId = COUNTERID_SMU7_APU_TEMP_MEAS_IGPU;
                        break;
                    }
                }
            }

            //Prepare the counter list
            result = PrepareSystemTopologyInfo();
        }

        if ((AMDT_STATUS_OK == ret) && (0 == g_counterCnt))
        {
            // No counters area available for profiler
            ret = AMDT_ERROR_PLATFORM_NOT_SUPPORTED;
        }

        if (AMDT_STATUS_OK == ret)
        {
            // set the profile state to initialized
            ret = PwrSetProfilingState(PowerProfilingInitialized);
        }

        if (AMDT_STATUS_OK == ret)
        {
            //Check if SMU is available
            if ((true == sysInfo.m_isPlatformSupported)
                && (true == sysInfo.m_isAmdApu)
                && !IsIGPUAvailable())
            {
                ret = AMDT_WARN_IGPU_DISABLED;
                PwrTrace(" AMDT_WARN_IGPU_DISABLED");
            }

            if (AMDT_WARN_SMU_DISABLED == result)
            {
                ret = AMDT_WARN_SMU_DISABLED;
                PwrTrace(" AMDT_WARN_SMU_DISABLED");
            }
        }

        PwrTrace("Return code: 0x%x counters %d ", ret, g_counterCnt);
    }

    return ret;
}

// AMDTPwrGetSystemTopology: This function provides device tree which represents
// the current system topology Here node as well as components inside the nodes
// are also considered as devices. Each device in the tree has none or more next
// devices in the same hierarchy and none or more sub devices.
AMDTResult AMDTPwrGetSystemTopology(AMDTPwrDevice** ppTopology)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == ppTopology)
    {
        ret  = AMDT_ERROR_INVALIDARG;
    }
    else
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (nullptr != g_pTopology)
        {
            *ppTopology = g_pTopology;
        }
        else
        {
            ret = AMDT_ERROR_OUTOFMEMORY;
        }
    }

    return ret;
}

// GetSubCompDevID: This function will provide a vector which contains deviceId of all the
// subcomponents of given deviceId. Function traverse over device topology tree, if device found
// capture id of the devices and its sub components (firstChild )in a vector.
void GetSubCompDevID(AMDTPwrDeviceId deviceID, AMDTPwrDevice* node,
                     std::vector<AMDTPwrDeviceId>& subCompDevIdVec, bool flag)
{

    if (nullptr != node)
    {

        if (node->m_deviceID == deviceID)
        {
            // Device found with the req. device Id
            flag = true;
        }

        if (nullptr != node->m_pFirstChild)
        {
            // recursively called the last child of the topology tree.
            GetSubCompDevID(deviceID, node->m_pFirstChild, subCompDevIdVec, flag);
        }

        if (true == flag)
        {
            // flag is set, means the devices encountered
            // is either a device itself or its subcomponent
            // push the device id in the vector
            subCompDevIdVec.push_back(node->m_deviceID);
        }

        // iterated the the sub component of device, set the flag to false
        if (node->m_deviceID == deviceID)
        {
            flag = false;
        }

        if (nullptr != node->m_pNextDevice)
        {
            // recursively call the right sibling of the device.
            GetSubCompDevID(deviceID, node->m_pNextDevice, subCompDevIdVec, flag);
        }
    }
}

// AMDTPwrGetDeviceCounters: Function for retrieving the counters supported by
// a specific device. The user specifies which device he is interested in.If
// the deviceID is 0xFFFFFFFF, then counters for all the available devices will
// be returned. The pointer returned will be valid till the client closes the
// profile-run using AMDTPwrProfileClose().
AMDTResult AMDTPwrGetDeviceCounters(AMDTPwrDeviceId deviceID,
                                    AMDTUInt32* pNumCounters,
                                    AMDTPwrCounterDesc** ppCounterDescs)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else
        {
            if ((nullptr == pNumCounters) || (nullptr == ppCounterDescs))
            {
                ret = AMDT_ERROR_INVALIDARG;
            }
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_ALL_DEVICES == deviceID)
        {
            *ppCounterDescs = &g_desc[0];
            *pNumCounters = g_counterCnt;
        }
        else
        {
            //Create a list based on the number of counters for the device
            *pNumCounters = 0;
            std::vector<AMDTPwrDeviceId> supportedDeviceIDList;

            // code for traversing tree
            if (nullptr != g_pTopology)
            {
                supportedDeviceIDList.reserve(MAX_DEVICE_CNT);
                GetSubCompDevID(deviceID, g_pTopology, supportedDeviceIDList, false);

                //Check if the device is valid
                if (0 == supportedDeviceIDList.size() && (AMDT_PWR_ALL_DEVICES != deviceID))
                {
                    ret = AMDT_ERROR_INVALID_DEVICEID;
                }
            }

            if (AMDT_STATUS_OK == ret)
            {
                for (const auto& deviceId : supportedDeviceIDList)
                {
                    for (cnt = 0; cnt < g_counterCnt; cnt++)
                    {
                        if (deviceId == g_desc[cnt].m_deviceId)
                        {
                            memcpy(&g_pClientList[*pNumCounters], &g_desc[cnt], sizeof(AMDTPwrCounterDesc));
                            (*pNumCounters)++;
                        }
                    }
                }

                *ppCounterDescs = &g_pClientList[0];
            }
        }
    }

    return ret;
}

// AMDTPwrGetCounterDesc: This API provides the description of the counter requested
// by the client with counter index
AMDTResult AMDTPwrGetCounterDesc(AMDTUInt32 counterID,
                                 AMDTPwrCounterDesc* pCounterDesc)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (nullptr == pCounterDesc)
        {
            ret = AMDT_ERROR_INVALIDARG;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (g_counterCnt < counterID)
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pCounterDesc = g_desc[counterID];
    }

    return ret;
}

// AMDTPwrEnableCounter: This API will enable the counter to be sampled. This
// API can be used even after the profile run is started.
AMDTResult AMDTPwrEnableCounter(AMDTUInt32 counterID)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_ERROR_FAIL;
    AMDTUInt32 cnt = 0;
    bool validCounter = false;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_STOPPED == state)
        {
            ret = AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED;
        }
        else if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            // In offline, don't support enabling countes on the fly
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    // If dGPU counter, check if it is still accessible
    if ((AMDT_STATUS_OK == ret) && (counterID >= DGPU_COUNTER_BASE_ID))
    {
        AMDTPwrTargetSystemInfo sysInfo;
        AMDTUInt32 pkgId = 0;
        PciPortAddress* pAddress = nullptr;
        bool isAccessible = false;

        // Find the package id
        // dGpu pkgId will always start from index 2 irrespective of AMD or Intel platform
        pkgId = 2 + (counterID % DGPU_COUNTER_BASE_ID) / DGPU_COUNTERS_MAX;

        // Check if there platform has a APU, add package id if it is APU
        AMDTPwrGetTargetSystemInfo(&sysInfo);

        // Check if this dGPU package is accessible
        GetPciDeviceInfo(pkgId, nullptr, &pAddress);
        isAccessible = IsDgpuMMIOAccessible(pAddress->m_bus, pAddress->m_dev, pAddress->m_func);

        if (false == isAccessible)
        {
            ret = AMDT_ERROR_COUNTER_NOT_ACCESSIBLE;
            PwrTrace("B%d:D%d:F%dCounter: %d AMDT_ERROR_COUNTER_NOT_ACCESSIBLE",
                     pAddress->m_bus,
                     pAddress->m_dev,
                     pAddress->m_func,
                     counterID);
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Check for valid counter id
        for (cnt = 0; cnt < MAX_SUPPORTED_COUNTERS; cnt++)
        {
            if (counterID == g_desc[cnt].m_counterID)
            {
                validCounter = true;
                break;
            }
        }

        if (false == validCounter)
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrIsCounterEnabled(counterID);

        if (AMDT_ERROR_COUNTER_NOT_ENABLED == ret)
        {
            // add it to the g_ActiveCounters list
            if (AMDT_PWR_VALUE_SINGLE == g_desc[cnt].m_aggregation)
            {
                // Set the counter only in case of simple counters
                g_activeCounters.push_back(counterID);
            }

            if ((AMDT_PWR_VALUE_HISTOGRAM == g_desc[cnt].m_aggregation)
                || (AMDT_PWR_VALUE_CUMULATIVE == g_desc[cnt].m_aggregation))
            {
                //printf("\n Enabling Histogram for %s counterId = %d , backendId = %d\n", g_counterList[cnt].m_pDesc->m_name, g_counterList[cnt].m_pDesc->m_counterID,g_counterList[cnt].m_backendId);
                g_histogramEnabled = true;
                g_histogramCounters[g_histogramCounterCount++] = g_counterList[cnt];
            }

            ret = AMDT_STATUS_OK;
        }
        else if (AMDT_STATUS_OK == ret)
        {
            ret = AMDT_ERROR_COUNTER_ALREADY_ENABLED;
        }
    }

    return ret;
}

// AMDTPwrDisableCounter: This API will disable the counter to be sampled from the
// active counter list. This API can be used even after the profile run is started.
AMDTResult AMDTPwrDisableCounter(AMDTUInt32 counterID)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    bool validCounter = false;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_STOPPED == state)
        {
            ret = AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED;
        }
        else if ((AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state))
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if ((AMDT_PWR_PROFILE_STATE_RUNNING == state))
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrIsCounterEnabled(counterID);
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Check for valid counter id
        for (cnt = 0; cnt < MAX_SUPPORTED_COUNTERS; cnt++)
        {
            if (counterID == g_desc[cnt].m_counterID)
            {
                validCounter = true;
                break;
            }
        }

        if (! validCounter)
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        bool foundCounter = false;
        auto first = g_activeCounters.begin();
        auto last = g_activeCounters.end();

        if (last != std::find(first, last, counterID))
        {
            g_activeCounters.erase(remove(first, last, counterID), first);
            foundCounter = true;
        }

        // Check if it is a histogram counter
        if (false == foundCounter)
        {
            for (cnt = 0; cnt < MAX_SUPPORTED_COUNTERS; cnt++)
            {
                if ((nullptr != g_histogramCounters[cnt].m_pDesc) && (counterID == g_histogramCounters[cnt].m_pDesc->m_counterID))
                {
                    foundCounter = true;

                    // Delete the counter from the active list
                    size_t size = (g_histogramCounterCount - (cnt + 1)) * sizeof(CounterInfo);

                    memmove(&g_histogramCounters[cnt], &g_histogramCounters[cnt + 1], size);

                    memset(&g_histogramCounters[g_histogramCounterCount - 1], 0, sizeof(CounterInfo));
                    g_histogramCounters[g_histogramCounterCount].m_pDesc = nullptr;
                    g_histogramCounterCount--;

                    break;
                }
            }
        }

        if (! foundCounter)
        {
            ret = AMDT_ERROR_COUNTER_NOT_ENABLED;
        }
    }

    return ret;
}

// AMDTPwrEnableAllCounters: This API will enable all the available counters.
// This API cannot be used once a profile run is started.
AMDTResult AMDTPwrEnableAllCounters()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_STOPPED == state)
        {
            ret = AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED;
        }
        else if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Check if total supported counter is same as
        // active counters
        if (g_activeCounters.size() == (g_counterCnt - g_histcounterCnt))
        {
            ret = AMDT_ERROR_COUNTER_ALREADY_ENABLED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Erase the current counters
        g_activeCounters.clear();

        // Set all supported counters
        for (cnt = 0; cnt < g_counterCnt; cnt++)
        {
            if (AMDT_PWR_VALUE_SINGLE == g_desc[cnt].m_aggregation)
            {
                g_activeCounters.push_back(g_desc[cnt].m_counterID);
            }
        }
    }

    return ret;
}

// AMDTPwrGetMinimalTimerSamplingPeriod: This API provide the minimum sampling
// time which can be set by the client.
AMDTResult AMDTPwrGetMinimalTimerSamplingPeriod(AMDTUInt32* pIntervalMilliSec)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pIntervalMilliSec)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if ((AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state))
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pIntervalMilliSec = MINIMAL_SAMPLING_PERIOD;
    }

    return ret;
}

// AMDTPwrSetTimerSamplingPeriod: This API will set the driver to periodically
// sample the counter values and store them in a buffer. This cannot be called
// once the profile run is started.
AMDTResult AMDTPwrSetTimerSamplingPeriod(AMDTUInt32 interval)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (interval < MINIMAL_SAMPLING_PERIOD)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_STOPPED == state)
        {
            ret = AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED;
        }
        else if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_samplingPeriod = interval;
    }

    return ret;
}

// AMDTPwrSetProfileDataFile: This API specifies the path to a file in which
// the raw profile data records will be stored. This API should be called
// before starting a profile run. This is an OFFLINE mode ONLY API. This
// cannot be called once the profile run in started.
AMDTResult AMDTPwrSetProfileDataFile(const char* pFilePath, AMDTUInt32 len)
{
    AMDTResult ret = AMDT_STATUS_OK;
    pFilePath = pFilePath;
    (void)len;
    return ret;
}

// AMDTPwrStartProfiling: If the profiler is not running, this will start the
// profiler.
AMDTResult AMDTPwrStartProfiling()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileConfig cfg;
    SamplingSpec spec;
    AMDTUInt16 counterList[MAX_SUPPORTED_COUNTERS];
    AMDTUInt32 counterListCnt = 0;
    AMDTUInt16 cnt = 0;
    AMDTUInt32 cuMask = 0;
    AMDTUInt32 coreMask = 0;
    AMDTUInt32 clientId = 0;
    AMDTPwrTargetSystemInfo sysInfo;
    AMDTUInt32 loop = 0;
    bool found = false;

    memset(&sysInfo, 0, sizeof(AMDTPwrTargetSystemInfo));
    memset(&spec, 0, sizeof(SamplingSpec));

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
        else if (AMDT_PWR_PROFILE_STATE_STOPPED == state)
        {
            ret = AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (0 == g_samplingPeriod)
        {
            ret = AMDT_ERROR_TIMER_NOT_SET;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        AMDTPwrGetTargetSystemInfo(&sysInfo);

        // Set the CU power counters if the profile mode is process type
        if (PROFILE_TYPE_PROCESS_PROFILING == g_profileType)
        {
            for (cnt = 0; cnt < g_counterCnt; cnt++)
            {
                AMDTUInt32 cuCnt = 0;

                for (cuCnt = 0; cuCnt < sysInfo.m_computeUnitCnt; cuCnt++)
                {
                    if ((g_counterList[cnt].m_backendId == COUNTERID_SMU7_APU_PWR_CU)
                        || (g_counterList[cnt].m_backendId == COUNTERID_SMU8_APU_PWR_CU))
                    {
                        if ((AMDT_PWR_VALUE_SINGLE == g_counterList[cnt].m_pDesc->m_aggregation)
                            && (g_counterList[cnt].m_instanceId == cuCnt))
                        {
                            if (AMDT_ERROR_COUNTER_ALREADY_ENABLED !=
                                AMDTPwrEnableCounter(g_counterList[cnt].m_pDesc->m_counterID))
                            {
                                g_filterCounters.push_back(g_counterList[cnt].m_pDesc->m_counterID);
                            }
                        }
                    }
                }
            }
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (g_activeCounters.empty() && !g_histogramEnabled)
        {
            ret = AMDT_ERROR_COUNTERS_NOT_ENABLED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Configure the profile
        cfg.m_specCnt = 1;
        spec.m_samplingPeriod = g_samplingPeriod;

        // Prepare the attribute list
        memset(counterList, 0, MAX_SUPPORTED_COUNTERS);

        // Set first three backend attributes (rec-id, sample id, time stamp
        // by default
        counterList[counterListCnt++] = 0;
        counterList[counterListCnt++] = 1;
        counterList[counterListCnt++] = 2;

        AMDTPwrProfileAttributeList supportedList;
        PwrGetSupportedAttributeList(&supportedList);
        bool isSmu7PkgPower {false};

        // check how platform measure total apu power
        for (auto itr : g_activeCounters)
        {
            for (AMDTUInt32 k = 0; k < g_counterCnt; ++k)
            {
                if (itr == g_counterList[k].m_pDesc->m_counterID &&
                    COUNTERID_SMU7_APU_PWR_PACKAGE == g_counterList[k].m_backendId)
                {
                    isSmu7PkgPower = true;
                    break;
                }
            }
        }


        if (true == g_histogramEnabled)
        {
            for (AMDTUInt32 histCnt = 0; histCnt < g_histogramCounterCount; ++histCnt)
            {
                AMDTUInt32 backendId = g_histogramCounters[histCnt].m_backendId;

                for (cnt = 0; cnt < supportedList.attrCnt; cnt++)
                {
                    if (backendId == supportedList.pAttrList[cnt].m_attrId)
                    {
                        if (COUNTERID_CEF == backendId)
                        {
                            coreMask |= (1 << g_histogramCounters[histCnt].m_instanceId);
                        }

                        counterList[counterListCnt++] = (DGPU_COUNTER_BASE_ID > backendId) ? cnt : (AMDTUInt16)backendId;

                        if (COUNTERID_SMU7_APU_PWR_PACKAGE == backendId)
                        {
                            isSmu7PkgPower = true;
                        }

                        break;
                    }
                }
            }
        }

        if (true == isSmu7PkgPower)
        {
            SetSmu7ApuPkgPwrCounter(supportedList, counterList, counterListCnt);
        }

        for (auto itr : g_activeCounters)
        {
            AMDTPwrProfileAttrType id = COUNTERID_MUST_BASE;
            AMDTPwrProfileAttributeList list;
            AMDTUInt16 cnt1 = 0;
            CounterInfo* pInfo = nullptr;
            memset(&list, 0, sizeof(AMDTPwrProfileAttributeList));

            // Get the counter from activated list
            AMDTUInt32 cnt2 = 0;
            found = false;

            for (cnt2 = 0; cnt2 < g_counterCnt; cnt2++)
            {
                if (itr == g_counterList[cnt2].m_pDesc->m_counterID)
                {
                    found = true;
                    break;
                }
            }

            if (true == found)
            {
                pInfo = &g_counterList[cnt2];
                ret = AMDT_STATUS_OK;
            }
            else
            {
                pInfo = nullptr;
                ret = AMDT_ERROR_UNEXPECTED;
                PwrTrace("Requested counter not found");
            }

            // If requested counter is supported prepare the counterList for preparing attribute mask
            // Histogram counter will be ommited from the counterList
            if (AMDT_STATUS_OK == ret)
            {
                AMDTPwrDevice dev;
                GetDeviceInfo(pInfo->m_pDesc->m_deviceId, &dev);

                if (AMDT_PWR_DEVICE_EXTERNAL_GPU == dev.m_type)
                {
                    // Check if this counter id is already added
                    loop = 0;
                    found = false;

                    //Check if this counter is already set
                    for (loop = 0; loop <= counterListCnt; loop++)
                    {
                        if (pInfo->m_pDesc->m_counterID == counterList[loop])
                        {
                            found = true;
                            break;
                        }
                    }

                    // This counter is not yet set
                    if (false == found)
                    {
                        counterList[counterListCnt++] = (AMDTUInt16)pInfo->m_pDesc->m_counterID;
                    }

                }
                else
                {
                    if (AMDT_STATUS_OK == ret)
                    {
                        // 1. Get the correspomding backend id
                        id = GetBackendCounterIdFromBaseId(pInfo->m_basicCounterId);

                        if (COUNTERID_MAX_CNT == id)
                        {
                            ret = AMDT_ERROR_UNEXPECTED;
                            PwrTrace("COUNTERID_MAX_CNT == id");
                        }
                    }

                    if (AMDT_STATUS_OK == ret)
                    {
                        // Find the core mask and CU mask

                        if (INSTANCE_TYPE_PER_CU == pInfo->m_instanceType)
                        {
                            // CU mask may not be useful at this moment as
                            // specific CUs can not be selected in the driver as this moement
                            cuMask |= (1 << pInfo->m_instanceId);
                        }
                        else if (INSTANCE_TYPE_PER_CORE == pInfo->m_instanceType)
                        {
                            AMDTUInt32 coreId = pInfo->m_instanceId;
                            coreMask |= (1 << coreId);
                        }

                        ret = PwrGetSupportedAttributeList(&list);
                    }

                    if (AMDT_STATUS_OK == ret)
                    {
                        for (cnt1 = 0; cnt1 < list.attrCnt; cnt1++)
                        {
                            AMDTPwrAttributeTypeInfo* info = list.pAttrList + cnt1;

                            if ((nullptr != info) && (info->m_attrId == (AMDTUInt32)id))
                            {
                                loop = 0;
                                found = false;

                                //Check if this counter is already set
                                for (loop = 0; loop <= counterListCnt; loop++)
                                {
                                    if (cnt1 == counterList[loop])
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                // This counter is not yet set
                                if (false == found)
                                {
                                    counterList[counterListCnt++] = cnt1;
                                }

                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        cfg.m_attrCnt = counterListCnt;
        spec.m_mask = coreMask;
        spec.m_maskCnt = GetCoreCount(coreMask);
        spec.m_profileType = g_profileType;

        if (PROFILE_TYPE_PROCESS_PROFILING == g_profileType)
        {
            spec.m_maskCnt = sysInfo.m_coreCnt;
            spec.m_mask = ~0 ^ (~0 << sysInfo.m_coreCnt);
        }
        else if (0 == spec.m_maskCnt)
        {
            spec.m_maskCnt = 1;
            spec.m_mask = 1;
        }

        cfg.m_pSpecList = &spec;
        cfg.m_pAttrList = &counterList[0];

        if (AMDT_STATUS_OK == ret)
        {
            ret = PwrRegisterClient(&clientId);
            cfg.m_clientId = clientId;
        }

        if (AMDT_STATUS_OK == ret)
        {
            ret = PwrSetProfilingConfiguration(&cfg);
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = PwrStartProfiling();
    }

    // Profile is started successfully. Create the data access service

    if (AMDT_STATUS_OK == ret)
    {
        // Open data access
        ret = AMDTPwrOpenOnlineDataAccess(nullptr);
    }

    return ret;
}

// AMDTPwrStopProfiling: If the profiler is running, this will stop the profile
AMDTResult AMDTPwrStopProfiling()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if ((AMDT_PWR_PROFILE_STATE_RUNNING != state) && (AMDT_PWR_PROFILE_STATE_PAUSED != state))
        {
            ret = AMDT_ERROR_PROFILE_NOT_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = PwrStopProfiling();
    }

    return ret;
}

// AMDTPwrPauseProfiling: This API will pause the profiling. The driver and the
// backend will retain the profile configuration details provided by the client.
AMDTResult AMDTPwrPauseProfiling()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    bool isPaused = false;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_PAUSED == state)
        {
            // Already in paused state
            isPaused = true;
        }
        else if (AMDT_PWR_PROFILE_STATE_RUNNING != state)
        {
            ret = AMDT_ERROR_PROFILE_NOT_STARTED;
        }
    }

    if (true == isPaused)
    {
        // Already in paused state
        ret = AMDT_STATUS_OK;
    }
    else if ((false == isPaused) && (AMDT_STATUS_OK == ret))
    {
        ret = PwrPauseProfiling();
    }

    return ret;
}

// AMDTPwrResumeProfiling This API will resume the profiling which is in paused
// state.
AMDTResult AMDTPwrResumeProfiling()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_PAUSED != state)
        {
            ret = AMDT_ERROR_PROFILE_NOT_PAUSED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = PwrResumeProfiling();
    }

    return ret;
}

// AMDTPwrGetProfilingState: This API provides the current state of the profile.
AMDTResult  AMDTPwrGetProfilingState(AMDTPwrProfileState* pState)
{
    AMDTPwrProfilingState state = PowerProfilingUnavailable;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pState)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = PwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        switch (state)
        {
            case PowerProfilingUnavailable:
                *pState = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
                break;

            case PowerProfilingInitialized:
                *pState = AMDT_PWR_PROFILE_STATE_IDLE;
                break;

            case PowerProfilingStopped:
                *pState = AMDT_PWR_PROFILE_STATE_STOPPED;
                break;

            case PowerProfiling:
                *pState = AMDT_PWR_PROFILE_STATE_RUNNING;
                break;

            case PowerProfilingPaused:
                *pState = AMDT_PWR_PROFILE_STATE_PAUSED;
                break;

            case PowerProfilingAborted:
                *pState = AMDT_PWR_PROFILE_STATE_ABORTED;
                break;

            default:
                ret = AMDT_ERROR_FAIL;
                PwrTrace("Unexpected state");
                break;
        }
    }

    return ret;
}

// AMDTPwrProfileClose** This API will close the power profiler and unregister
// driver and cleanup all memory allocated during AMDTPwrProfileInitialize.
AMDTResult AMDTPwrProfileClose()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = PwrProfileClose();
    }

    if (AMDT_STATUS_OK == ret)
    {
        AMDTPwrCloseDataAccess();
        // release the memory pool created
        ret = ReleaseMemoryPool(&g_apiMemoryPool);

        if (AMDT_STATUS_OK == ret)
        {
            CleanBuffers();
            PwrSetProfilingState(PowerProfilingUnavailable);
        }
    }

    return ret;
}

// AMDTPwrSetSampleValueOption:  API to set the sample value options to be
// returned by the AMDTPwrReadAlllEnabledCounters().
AMDTResult AMDTPwrSetSampleValueOption(AMDTSampleValueOption opt)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if ((AMDT_PWR_SAMPLE_VALUE_INSTANTANEOUS > opt) || (AMDT_PWR_SAMPLE_VALUE_CNT < opt))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_outputOption = opt;
    }

    return ret;
}

// AMDTPwrGetSampleValueOption: API to get the sample value option current set for
// the profiler
AMDTResult AMDTPwrGetSampleValueOption(AMDTSampleValueOption* pOpt)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pOpt)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pOpt = g_outputOption;
    }

    return ret;
}

// AMDTPwrReadAllEnabledCounters: API to read all the counters that are enabled.
// This will NOT read the histogram counters. This can return an array of
// {CounterID, Float-Value}. If there are no new samples, this API will
// return AMDTResult NO_NEW_DATA and pNumOfSamples will point to value of zero. If there
// are new samples, this API will return AMDTResult SUCCESS and pNumOfSamples will point
// to value greater than zero.
AMDTResult AMDTPwrReadAllEnabledCounters(AMDTUInt32* pNumOfSamples,
                                         AMDTPwrSample** ppData)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 sampleCnt = 0;

    // Check for valid arguments
    if ((nullptr == pNumOfSamples) || (nullptr == ppData))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pNumOfSamples = 0;
        *ppData = nullptr;

        // Check for valid profile state
        ret = AMDTPwrGetProfilingState(&state);
    }

    if ((AMDT_STATUS_OK == ret) && (AMDT_PWR_PROFILE_STATE_RUNNING != state))
    {
        switch (state)
        {
            case AMDT_PWR_PROFILE_STATE_UNINITIALIZED:
            {
                ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
                break;
            }

            case AMDT_PWR_PROFILE_STATE_IDLE:
            case AMDT_PWR_PROFILE_STATE_STOPPED:
            case AMDT_PWR_PROFILE_STATE_PAUSED:
            {
                ret = AMDT_ERROR_PROFILE_NOT_STARTED;
                break;
            }

            case AMDT_PWR_PROFILE_STATE_ABORTED:
            {
                ret = AMDT_ERROR_DRIVER_UNAVAILABLE;
                break;
            }

            default:
                break;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        bool isDataAvailable = false;
        AMDTUInt32 resultCnt = 0;
        AMDTPwrProcessedDataRecord* pData = nullptr;

        AMDTPwrProcessedDataRecord data;

        if ((PROFILE_TYPE_PROCESS_PROFILING == g_profileType) && (!g_activeCounters.empty()))
        {
            for (auto fltr : g_filterCounters)
            {
                g_activeCounters.erase(remove(g_activeCounters.begin(),
                                              g_activeCounters.end(), fltr), g_activeCounters.end());
            }

            g_filterCounters.clear();
        }

        while (AMDT_ERROR_NODATA != AMDTGetCounterValues(&data))
        {
            pData = &data;
            isDataAvailable = true;
            AMDTUInt64 startTs = 0 ;
            AMDTUInt32 cnt = 0;
            AMDTPwrAttributeInfo* info =  nullptr;
            AMDTPwrSample* pResult = g_pResult + resultCnt++;
            pResult->m_numOfValues = 0;
            sampleCnt++;

            // Fill the basic information
            pResult->m_recordId = pData->m_recId;
#ifdef LINUX
            // m_ts is actually micro-seconds on Linux, converting this to
            // milli-seconds and rounding off to nearest milli-second
            double tmpMs = pData->m_ts / 1000.0 ;
            pResult->m_elapsedTimeMs = (AMDTUInt64)nearbyint(tmpMs);
#else
            pResult->m_elapsedTimeMs = pData->m_ts;
#endif
            AMDTPwrGetProfileTimeStamps(&startTs, nullptr);
            ConvertTimeStamp(&pResult->m_systemTime, startTs);

            // Iterate over the counters
            for (auto cntr : g_activeCounters)
            {
                AMDTUInt32 cnt1 = 0;
                CounterInfo* pCounterInfo = GetBaseCounterInfoFromClientId(cntr);

                if (nullptr != pCounterInfo)
                {
                    AMDTUInt32 backendId = pCounterInfo->m_backendId;
                    AMDTUInt32 instanceId = pCounterInfo->m_instanceId;
                    bool foundCounterId = false;

                    // record id and sample id from processed list are ignored
                    for (cnt1 = 0; cnt1 < pData->m_attrCnt; cnt1++)
                    {
                        info = &pData->m_attr[cnt1];

                        // First check the counter id if it is found
                        if ((backendId == info->m_pInfo->m_attrId)
                            && (instanceId == info->m_instanceId))
                        {
                            foundCounterId = true;
                            break;
                        }
                    }

                    if (foundCounterId)
                    {
                        //Prepare the output data now
                        if (PWR_UNIT_TYPE_COUNT == info->m_pInfo->m_unitType)
                        {
                            AMDTUInt64 counterValue = info->u.m_value64;
                            pResult->m_counterValues[cnt].m_counterValue = (AMDTFloat32)counterValue;
                        }
                        else
                        {
                            AMDTFloat32 counterValue = info->u.m_float32;

                            // Make sure that the counter's value does not go beyond the upper bound.
                            pResult->m_counterValues[cnt].m_counterValue = static_cast<AMDTFloat32>(counterValue);
                        }

                        pResult->m_counterValues[cnt].m_counterID = cntr;
                        pResult->m_numOfValues++;
                        cnt++;
                    }
                    else
                    {
                        ret = AMDT_ERROR_INTERNAL;
                    }
                } // Counter info available
                else
                {
                    printf("\n ERROR CounterInfo not available for %d\n", cnt);
                }
            }

            // Check if buffer is full
            if (resultCnt == MAX_SAMPLES_PER_QUERY)
            {
                // We don't have enough space to store
                PwrTrace("MAX_SAMPLES_PER_QUERY exceeded");
                break;
            }

        };

        if (false == isDataAvailable)
        {
            ret = AMDT_ERROR_NODATA;
        }

        if (AMDT_STATUS_OK == ret)
        {
            *pNumOfSamples = sampleCnt;
            *ppData = g_pResult;
        }
    }

    return ret;
}

// AMDTPwrReadCounterHistogram: API to read one of the derived counters generate histograms
// from the raw counter values.
AMDTResult AMDTPwrReadCounterHistogram(AMDTUInt32 counterID,
                                       AMDTUInt32* pNumEntries,
                                       AMDTPwrHistogram** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    CounterInfo* pInfo = nullptr;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

    // Check for valid arguments
    if ((nullptr == pNumEntries) || (nullptr == ppData))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (ret == AMDT_STATUS_OK)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (ret == AMDT_STATUS_OK)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (!((AMDT_PWR_PROFILE_STATE_RUNNING == state)
                   || (AMDT_PWR_PROFILE_STATE_PAUSED == state)
                   || (AMDT_PWR_PROFILE_STATE_STOPPED == state)))
        {
            ret = AMDT_ERROR_PROFILE_NOT_STARTED;
        }
    }

    // Check for valid counter id
    if (ret == AMDT_STATUS_OK)
    {
        pInfo = GetBaseCounterInfoFromClientId(counterID);

        if ((nullptr == pInfo) || (AMDT_PWR_VALUE_HISTOGRAM != pInfo->m_pDesc->m_aggregation))
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }
    }

    if (ret == AMDT_STATUS_OK)
    {
        Histogram* pHistogram = nullptr;
        memset(g_cumulativeCounterList, 0, sizeof(AMDTFloat32) * MAX_COUNTER_CNT);

        if (AMDT_PWR_ALL_COUNTERS != counterID)
        {
            pHistogram = GetHistogramCounter(pInfo->m_backendId, pInfo->m_instanceId);

            if (nullptr != pHistogram)
            {
                g_histogramCounterList[0].m_counterId = counterID;
                g_histogramCounterList[0].m_numOfBins = pHistogram->m_binCnt;
                g_histogramCounterList[0].m_pRange = pHistogram->m_pRangeStartIndex;
                g_histogramCounterList[0].m_pBins = pHistogram->m_pRangeValue;
                *ppData = &g_histogramCounterList[0];
                *pNumEntries = 1;
            }
            else
            {
                ret = AMDT_ERROR_INTERNAL;
                PwrTrace("error: AMDT_ERROR_INTERNAL pHistogram==NULL");
            }
        }
        else
        {
            AMDTUInt32 cnt = 0;
            AMDTUInt32 entryCnt = 0;

            for (cnt = 0; cnt < g_histogramCounterCount; cnt ++)
            {
                pInfo = GetBaseCounterInfoFromClientId(g_histogramCounters[cnt].m_pDesc->m_counterID);

                if ((nullptr != pInfo)
                    && (AMDT_PWR_VALUE_HISTOGRAM == pInfo->m_pDesc->m_aggregation))
                {
                    pHistogram = GetHistogramCounter(pInfo->m_backendId, pInfo->m_instanceId);

                    if (nullptr != pHistogram)
                    {
                        g_histogramCounterList[entryCnt].m_counterId = g_histogramCounters[cnt].m_pDesc->m_counterID;
                        g_histogramCounterList[entryCnt].m_numOfBins = pHistogram->m_binCnt;
                        g_histogramCounterList[entryCnt].m_pRange = pHistogram->m_pRangeStartIndex;
                        g_histogramCounterList[entryCnt].m_pBins = pHistogram->m_pRangeValue;
                        *pNumEntries = entryCnt;
                    }
                    else
                    {
                        ret = AMDT_ERROR_INTERNAL;
                        PwrTrace("error: AMDT_ERROR_INTERNAL pHistogram==NULL for list");
                    }
                }
                else
                {
                    ret = AMDT_ERROR_INTERNAL;
                    PwrTrace("error: AMDT_ERROR_INTERNAL pInfo==NULL");
                }
            }
        }
    }

    return ret;
}

// AMDTPwrReadCumulativeCounter: API to read one of the derived accumulated counters
// values from the raw counter values.
AMDTResult AMDTPwrReadCumulativeCounter(AMDTUInt32 counterId,
                                        AMDTUInt32* pNumEntries,
                                        AMDTFloat32** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    CounterInfo* pCounterInfo = nullptr;

    // Check for valid arguments
    if ((nullptr == pNumEntries) || (nullptr == ppData))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (ret == AMDT_STATUS_OK)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (ret == AMDT_STATUS_OK)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (!((AMDT_PWR_PROFILE_STATE_RUNNING == state)
                   || (AMDT_PWR_PROFILE_STATE_PAUSED == state)
                   || (AMDT_PWR_PROFILE_STATE_STOPPED == state)))
        {
            ret = AMDT_ERROR_PROFILE_NOT_STARTED;
        }
    }

    // Check for valid counter id
    if (ret == AMDT_STATUS_OK)
    {
        memset(g_cumulativeCounterList, 0, sizeof(AMDTFloat32) * MAX_COUNTER_CNT);

        if (AMDT_PWR_ALL_COUNTERS != counterId)
        {
            pCounterInfo = GetBaseCounterInfoFromClientId(counterId);

            if ((nullptr == pCounterInfo) || (AMDT_PWR_VALUE_CUMULATIVE != pCounterInfo->m_pDesc->m_aggregation))
            {
                ret = AMDT_ERROR_INVALID_COUNTERID;
            }

            if (ret == AMDT_STATUS_OK)
            {
                memcpy(g_cumulativeCounterList,
                       GetCumulativeCounter(pCounterInfo->m_backendId, pCounterInfo->m_instanceId),
                       sizeof(AMDTFloat32));
                *ppData = &g_cumulativeCounterList[0];
                *pNumEntries = 1;
            }
        }
        else
        {
            AMDTUInt32 entryCnt = 0;
            AMDTUInt32 cnt = 0;

            for (cnt = 0; cnt < g_histogramCounterCount; cnt ++)
            {
                pCounterInfo = GetBaseCounterInfoFromClientId(g_histogramCounters[cnt].m_pDesc->m_counterID);

                if ((nullptr != pCounterInfo)
                    && (AMDT_PWR_VALUE_CUMULATIVE == pCounterInfo->m_pDesc->m_aggregation))
                {
                    memcpy(&g_cumulativeCounterList[entryCnt++],
                           GetCumulativeCounter(pCounterInfo->m_backendId, pCounterInfo->m_instanceId),
                           sizeof(AMDTFloat32));
                    *pNumEntries = entryCnt;
                }
            }

            *ppData = g_cumulativeCounterList;
        }
    }

    return ret;

}

//Helper functions

// AMDTPwrGetTimerSamplingPeriod: This API will get the timer sampling period at which
// the samples are collected by the driver.
AMDTResult AMDTPwrGetTimerSamplingPeriod(AMDTUInt32* pIntervalMilliSec)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

    if (nullptr == pIntervalMilliSec)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (ret == AMDT_STATUS_OK)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (ret == AMDT_STATUS_OK)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pIntervalMilliSec = g_samplingPeriod;
    }

    return ret;
}

//AMDTPwrIsCounterEnabled: This API is query API to check whether a counter is enabled
AMDTResult AMDTPwrIsCounterEnabled(AMDTUInt32 counterID)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTUInt32 cnt = 0;
    bool validCounter = false;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if ((AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state))
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (IS_PROFILE_MODE_OFFLINE && (AMDT_PWR_PROFILE_STATE_RUNNING == state))
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        std::list<AMDTUInt32>::iterator cntr;

        //Check for valid counter id
        for (cnt = 0; cnt < MAX_SUPPORTED_COUNTERS; cnt++)
        {
            if (counterID == g_desc[cnt].m_counterID)
            {
                validCounter = true;
                break;
            }
        }

        if (validCounter)
        {
            bool counterEnabled = false;
            auto foundCounter = std::find(g_activeCounters.begin(), g_activeCounters.end(), counterID);

            if (foundCounter != g_activeCounters.end())
            {
                counterEnabled = true;
            }

            // Check in the histogram counters as well
            if (!counterEnabled && g_histogramEnabled)
            {
                for (cnt = 0; cnt < g_histogramCounterCount; cnt++)
                {
                    if (counterID == g_histogramCounters[cnt].m_pDesc->m_counterID)
                    {
                        counterEnabled = true;
                        break;
                    }
                }
            }

            ret = (counterEnabled) ? AMDT_STATUS_OK : AMDT_ERROR_COUNTER_NOT_ENABLED;
        }
        else
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }
    }

    return ret;
}

// AMDTPwrGetNumEnabledCounters: This API is query API to check number of counters
// which are enabled
AMDTResult AMDTPwrGetNumEnabledCounters(AMDTUInt32* pCount)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pCount)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pCount = (AMDTUInt32)(g_activeCounters.size() + g_histogramCounterCount);
    }

    return ret;
}

// AMDTPwrGetApuPstateInfo:  API to get the list of pstate supported by the target APU,
// where power profile is running. List contains both hardware / boosted and software
// p states with corresponding frequencies.
AMDTResult AMDTPwrGetApuPstateInfo(AMDTPwrApuPstateList* pList)
{
    AMDTResult ret = AMDT_ERROR_INVALIDARG;

    if (nullptr != pList)
    {
        AMDTPwrProfileState  state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

        ret = AMDTPwrGetProfilingState(&state);

        if (AMDT_STATUS_OK == ret)
        {
            if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED != state)
            {
                AMDTPwrApuPstateList* pData = nullptr;
                pData = GetApuPStateInfo();

                if (0 == pList->m_cnt)
                {
                    // This implies, platform is not supported AMD platform
                    ret = AMDT_ERROR_PLATFORM_NOT_SUPPORTED;
                }

                memcpy(pList, pData, sizeof(AMDTPwrApuPstateList));
            }
            else
            {
                ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
            }
        }
    }

    return ret;
}

// AMDTPwrGetApuPstateInfo:  API to get the list of pstate supported by the target APU,
// where power profile is running. List contains both hardware / boosted and software
// p states with corresponding frequencies.
AMDTResult AMDTPwrGetCounterHierarchy(AMDTUInt32 id, AMDTPwrCounterHierarchy* pInfo)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_ERROR_INVALIDARG;
    AMDTUInt32 cnt = 0;
    CounterInfo* counterInfo = nullptr;
    AMDTPwrCounterHierarchy* pRel = nullptr;
    AMDTUInt32 childCnt = 0;
    AMDTUInt32 size = sizeof(g_InternalCounterHirarchy) / sizeof(AMDTPwrCounterHierarchy);
    AMDTPwrTargetSystemInfo sysInfo;

    memset(&sysInfo, 0, sizeof(sysInfo));

    if (nullptr != pInfo)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if ((AMDT_STATUS_OK == ret) && (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state))
    {
        ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Get the system info
        ret = AMDTPwrGetTargetSystemInfo(&sysInfo);
    }

    bool foundCounterHierarchy = false;

    if (AMDT_STATUS_OK == ret)
    {
        counterInfo = GetBaseCounterInfoFromClientId(id);
        pInfo->m_childCnt = 0;

        if (nullptr == counterInfo)
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }

        if (AMDT_STATUS_OK == ret)
        {
            // Find the hierarchy information from the table
            for (cnt = 0; cnt < size; cnt++)
            {
                if ((AMDTUInt32)counterInfo->m_basicCounterId == g_InternalCounterHirarchy[cnt].m_counter)
                {
                    AMDTUInt32 cnt1 = 0;
                    AMDTUInt32 clientId = 0;
                    // Prepare Child list
                    pRel = &g_InternalCounterHirarchy[cnt];

                    for (cnt1 = 0; cnt1 < pRel->m_childCnt; cnt1++)
                    {
                        AMDTUInt32 childBaseId = *(pRel->m_pChildList + cnt1);
                        clientId = GetClientIdFromBaseCounterId(childBaseId, 0);
                        g_ChildCounterList[childCnt++] = clientId;

                        if ((sysInfo.m_computeUnitCnt > 1) && ((COUNTER_SMU7_APU_PWR_CU == childBaseId) ||
                                                               (COUNTER_SMU8_APU_PWR_CU == childBaseId)))
                        {
                            // we need to add power counters for all othe CUs
                            clientId = GetClientIdFromBaseCounterId(childBaseId, 1);
                            g_ChildCounterList[childCnt++] = clientId;
                        }
                    }

                    pInfo->m_childCnt = childCnt;
                    pInfo->m_pChildList = &g_ChildCounterList[0];
                    pInfo->m_counter = counterInfo->m_pDesc->m_counterID;
                    clientId = GetClientIdFromBaseCounterId(g_InternalCounterHirarchy[cnt].m_parent, 1);
                    pInfo->m_parent = clientId;
                    foundCounterHierarchy = true;

                    break;
                } // if the counter id matches
            } // iterate over the counters in g_InternalCounterHirarchy
        } // valid counter id
    }

    if ((AMDT_STATUS_OK == ret) && (!foundCounterHierarchy))
    {
        // Counter does not have any hierachical relationship
        ret = AMDT_ERROR_COUNTER_NOHIERARCHY;
    }

    return ret;
}

// AMDTEnableProcessProfiling: This API enables process profiling.
// This API will enable backend and driver to collect running PIDs
// at lowest possible granularity and attribute them against the
// power values provided by the SMU.
AMDTResult AMDTEnableProcessProfiling()
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrTargetSystemInfo sysInfo;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Get the system info
        ret = AMDTPwrGetTargetSystemInfo(&sysInfo);

        if ((false == sysInfo.m_isAmdApu) || (false == sysInfo.m_smuTable.m_info[0].m_isAccessible))
        {
            ret = AMDT_WARN_PROCESS_PROFILE_NOT_SUPPORTED;
        }
    }

    if (ret == AMDT_STATUS_OK)
    {
        g_profileType = PROFILE_TYPE_PROCESS_PROFILING;
    }

    return ret;
}

// This API will provide the list of running PIDs so far from the time of
// profile start or bewteen two consecutive call of this function,
// their agreegated power indicators. This API can be called at any
// point of time from start of the profile to the stop of the profile.
AMDTResult AMDTGetProcessProfileData(AMDTUInt32* pPIDCount,
                                     AMDTPwrProcessInfo** ppData,
                                     AMDTUInt32 pidVal,
                                     bool reset)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    // Check for valid arguments
    if ((nullptr == pPIDCount) || (nullptr == ppData))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pPIDCount = 0;
        *ppData = nullptr;

        // Check for valid profile state
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (ret == AMDT_STATUS_OK)
    {
        if ((AMDT_PWR_PROFILE_STATE_STOPPED == state)
            || (AMDT_PWR_PROFILE_STATE_RUNNING == state)
            || (AMDT_PWR_PROFILE_STATE_PAUSED == state))

        {
            ret = AMDTGetCummulativePidProfData(pPIDCount, ppData, pidVal, reset);
        }
    }

    return ret;
}

// This API will provide the list of running processes/Modules/ip samples collected by the profiler
// so far from the time of profile start or bewteen two consecutive call of this function,
// their agreegated power indicators. This API can be called at any
// point of time from start of the profile to the stop of the profile.
AMDTResult AMDTPwrGetModuleProfileData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCount, AMDTFloat32* pPower)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
#ifdef LINUX
    return AMDT_ERROR_NOTSUPPORTED;
#else

    // Check for valid arguments
    if ((nullptr == pModuleCount) || (nullptr == ppData))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pModuleCount = 0;
        *ppData = nullptr;

        // Check for valid profile state
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (ret == AMDT_STATUS_OK)
    {
        if ((AMDT_PWR_PROFILE_STATE_STOPPED == state)
            || (AMDT_PWR_PROFILE_STATE_RUNNING == state)
            || (AMDT_PWR_PROFILE_STATE_PAUSED == state))

        {
            ret = PwrGetModuleProfileData(ppData, pModuleCount, pPower);
        }
    }

    return ret;
#endif
}

#endif

