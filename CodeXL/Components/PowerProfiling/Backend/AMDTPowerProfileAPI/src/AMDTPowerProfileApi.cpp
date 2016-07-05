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
#include <PowerProfileDriverInterface.h>

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

bool g_isSVI2Supported = false;
// Create API layer pool.
static MemoryPool g_apiMemoryPool;
#define API_POOL_SIZE 10485760 // 10MB

static AMDTUInt32 g_samplingPeriod = 0;
static AMDTPwrProfileMode g_profileMode;
AMDTUInt32 g_profileType = PROFILE_TYPE_TIMELINE;


//Default option for output data is AMDT_PWR_SAMPLE_VALUE_INSTANTANEOUS
static AMDTSampleValueOption g_outputOption = AMDT_PWR_SAMPLE_VALUE_INSTANTANEOUS;

// Filter counter list from output list
static std::vector <AMDTUInt32> g_filterCounters;

// Single counter Output list
gtVector<AMDTPwrSample> g_result;
AMDTPwrCounterValue*     g_pCounterStorage = nullptr;
#define PWR_COUNTER_STORAGE_POOL (1024*1024)

// Cummulative counter Output list
gtVector <AMDTFloat32> g_cummulativeResult;

// Histogram counter Output list
gtVector <AMDTPwrHistogram> g_histogramResult;

// Topology: dynamic list created based of the platform used
static AMDTPwrDevice* g_pTopology;
// Memory pool created to avoid complexity during cleanup
static AMDTPwrDevice* g_pDevPool;

static std::vector<AMDTPwrCounterDesc> g_clientCounters;
static AMDTPwrTargetSystemInfo g_sysInfo;

static AMDTUInt32 g_smu7PackagePwr[] =
{
    COUNTERID_SMU7_APU_PWR_CU,
    COUNTERID_SMU7_APU_PWR_PCIE,
    COUNTERID_SMU7_APU_PWR_DDR,
    COUNTERID_SMU7_APU_PWR_DISPLAY,
    COUNTERID_SMU7_APU_PWR_IGPU
};

static AMDTUInt32 g_smu8ApuPwr[] =
{
    COUNTERID_SMU8_APU_PWR_CU,
    COUNTERID_SMU8_APU_PWR_VDDNB,
    COUNTERID_SMU8_APU_PWR_VDDGFX,
    COUNTERID_SMU8_APU_PWR_VDDIO,
    COUNTERID_SMU8_APU_PWR_VDDP,
    COUNTERID_SMU8_APU_PWR_ROC,

};
static AMDTUInt32 g_smu8VddNbPwr[] =
{
    COUNTERID_SMU8_APU_PWR_UVD,
    COUNTERID_SMU8_APU_PWR_VCE,
    COUNTERID_SMU8_APU_PWR_ACP,
    COUNTERID_SMU8_APU_PWR_UNB,
    COUNTERID_SMU8_APU_PWR_SMU

};

static AMDTPwrCounterHierarchy g_InternalCounterHirarchy[] =
{
    {COUNTERID_SMU7_APU_PWR_CU, COUNTERID_SMU7_APU_PWR_PACKAGE, 0, nullptr},
    {COUNTERID_SMU7_APU_PWR_PCIE, COUNTERID_SMU7_APU_PWR_PACKAGE, 0, nullptr},
    {COUNTERID_SMU7_APU_PWR_DDR, COUNTERID_SMU7_APU_PWR_PACKAGE, 0, nullptr},
    { COUNTERID_SMU7_APU_PWR_DISPLAY, COUNTERID_SMU7_APU_PWR_PACKAGE, 0, nullptr },
    { COUNTERID_SMU7_APU_PWR_IGPU, COUNTERID_SMU7_APU_PWR_PACKAGE, 0, nullptr },
    { COUNTERID_SMU7_APU_PWR_PACKAGE, COUNTERID_SMU7_APU_PWR_PACKAGE, sizeof(g_smu7PackagePwr) / sizeof(AMDTUInt32), g_smu7PackagePwr },
    { COUNTERID_SMU8_APU_PWR_CU, COUNTERID_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_APU, COUNTERID_SMU8_APU_PWR_APU, sizeof(g_smu8ApuPwr) / sizeof(AMDTUInt32), g_smu8ApuPwr },
    { COUNTERID_SMU8_APU_PWR_VDDGFX, COUNTERID_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_VDDIO, COUNTERID_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_VDDNB, COUNTERID_SMU8_APU_PWR_APU, sizeof(g_smu8VddNbPwr) / sizeof(AMDTUInt32), g_smu8VddNbPwr },
    { COUNTERID_SMU8_APU_PWR_VDDP, COUNTERID_SMU8_APU_PWR_APU, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_UVD, COUNTERID_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_VCE, COUNTERID_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_ACP, COUNTERID_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_UNB, COUNTERID_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_SMU, COUNTERID_SMU8_APU_PWR_VDDNB, 0, nullptr },
    { COUNTERID_SMU8_APU_PWR_ROC, COUNTERID_SMU8_APU_PWR_APU, 0, nullptr },
};

static AMDTUInt32 g_ChildCounterList[COUNTERID_MAX_CNT];

// Macros
#define IS_PROFILE_MODE_OFFLINE (g_profileMode == AMDT_PWR_PROFILE_MODE_OFFLINE)
#define IS_PROFILE_MODE_ONLINE (g_profileMode == AMDT_PWR_PROFILE_MODE_ONLINE)

#define PP_INVALID_COUNTER_ID    0xFFFF

typedef std::pair<AMDTUInt8, bool> DgpuCountPair;
//#define PWR_API_STUB_ENABLED
#ifdef PWR_API_STUB_ENABLED
#include <AMDTPowerProfileApiStub.cpp>
#else

// AllocateBuffers: Allocate required memory for power profiling
AMDTResult AllocateBuffers()
{
    AMDTResult ret = AMDT_STATUS_OK;

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

    return ret;
}

// CleanBuffers: Clean all buffers which are allocated during
// initialization of the profile session
AMDTResult CleanBuffers()
{
    AMDTResult ret = AMDT_STATUS_OK;
    g_clientCounters.clear();
    g_pDevPool = nullptr;

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
    AMDTUInt32 devCnt = 0;
    SmuInfo* smuInfo = &g_sysInfo.m_smuTable.m_info[0];
    bool smuAccesible = false;
    AMDTUInt32 deviceIdx = 0;

    if (true == g_sysInfo.m_isAmdApu)
    {
        cuCnt = g_sysInfo.m_computeUnitCnt;
        corePerCuCnt = g_sysInfo.m_coresPerCu;
    }

    // Create the root node as system
    system = AllocateDevice(&devCnt);
    system->m_deviceID = deviceIdx++;
    system->m_type = AMDT_PWR_DEVICE_SYSTEM;
    sprintf(system->m_pName, "System");
    sprintf(system->m_pDescription, "Root device");
    system->m_pNextDevice = nullptr;
    g_pTopology = system;

    // Considering only one package at this moment
    AMDTPwrDevice* pkg0 = AllocateDevice(&devCnt);
    system->m_pFirstChild = pkg0;
    pkg0->m_deviceID = deviceIdx++;
    pkg0->m_type = AMDT_PWR_DEVICE_PACKAGE;
    PciDeviceInfo* pNodeInfo = nullptr;
    GetPciDeviceInfo(APU_SMU_ID, &pNodeInfo, nullptr);

    if (pNodeInfo && (strlen(pNodeInfo->m_shortName) > 0))
    {
        //TODO: GUI shouldn't check for constant string
        memset(pkg0->m_pName, 0, AMDT_PWR_EXE_NAME_LENGTH);
        sprintf(pkg0->m_pName, "%s", pNodeInfo->m_shortName);
        sprintf(pkg0->m_pDescription, "%s",pNodeInfo->m_shortName);
    }
    else
    {
        memset(pkg0->m_pName, 0, AMDT_PWR_EXE_NAME_LENGTH);
        sprintf(pkg0->m_pName, "%s", "Unsupported Apu");
        sprintf(pkg0->m_pDescription, "%s", "Unsupported Apu device");
    }

    // ONLY if supported AMD platform
    if ((true == g_sysInfo.m_isAmd) && (true == g_sysInfo.m_isPlatformSupported))
    {
        PwrInsertDeviceCounters(pkg0, 0, 0);
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
        if (NULL != g_sysInfo.m_pNodeInfo && DEVICE_TYPE_APU == g_sysInfo.m_pNodeInfo->m_deviceType)
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
        newCu->m_deviceID = deviceIdx++;
        newCu->m_type = AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT;
        //TODO: GUI shouldn't check for constant string
        sprintf(newCu->m_pName, "CPU CU%d", cnt);
        sprintf(newCu->m_pDescription, "Compute Unit %d", cnt);

        if (true == smuAccesible)
        {
            PwrInsertDeviceCounters(newCu, cnt, APU_SMU_ID);
            newCu->m_isAccessible = true;
        }

        AMDTPwrDevice* coreHead = nullptr;

        for (cnt1 = 0; cnt1 < corePerCuCnt; cnt1++)
        {
            AMDTPwrDevice* newCore = AllocateDevice(&devCnt);
            newCore->m_deviceID = deviceIdx++;
            newCore->m_type = AMDT_PWR_DEVICE_CPU_CORE;
            //TODO: GUI shouldn't check for constant string
            sprintf(newCore->m_pName, "CPU Core%d", coreId);
            sprintf(newCore->m_pDescription, "Core%d", coreId);
            newCore->m_pFirstChild = nullptr;
            PwrInsertDeviceCounters(newCore, coreId, 0);
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
    if ((0 == cuCnt) && (true == g_sysInfo.m_isPlatformSupported))
    {
        AMDTPwrDevice* coreHead = nullptr;

        for (cnt = 0; cnt < g_sysInfo.m_coreCnt; cnt++)
        {
            AMDTPwrDevice* newCore = AllocateDevice(&devCnt);
            newCore->m_deviceID = deviceIdx++;
            newCore->m_type = AMDT_PWR_DEVICE_CPU_CORE;
            //TODO: GUI shouldn't check for constant string
            sprintf(newCore->m_pName, "Core%d", coreId);
            sprintf(newCore->m_pDescription, "Core%d", coreId);
            PwrInsertDeviceCounters(newCore, coreId, 0);
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

    if (true == g_sysInfo.m_isAmdApu)
    {
        // check if Igpu is present
        for (cnt = 0; cnt < g_sysInfo.m_igpuCnt; cnt++)
        {
            AMDTPwrDevice* igpu = AllocateDevice(&devCnt);
            igpu->m_deviceID = deviceIdx++;
            igpu->m_type = AMDT_PWR_DEVICE_INTERNAL_GPU;
            //TODO: GUI shouldn't check for constant string
            memset(igpu->m_pName, 0, AMDT_PWR_EXE_NAME_LENGTH);
            sprintf(igpu->m_pName, "Igpu");
            sprintf(igpu->m_pDescription, "Integrated GPU");
            igpu->m_pFirstChild = nullptr;

            // Show iGPU counters only if SMU is accessible
            if (true == smuAccesible)
            {
                PwrInsertDeviceCounters(igpu, 0, APU_SMU_ID);
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

        if (g_isSVI2Supported)
        {
            //Fill discrete SVI2
            for (cnt = 0; cnt < g_sysInfo.m_svi2Cnt; cnt++)
            {
                AMDTPwrDevice* svi2 = AllocateDevice(&devCnt);
                svi2->m_deviceID = deviceIdx++;
                svi2->m_type = AMDT_PWR_DEVICE_SVI2;
                sprintf(svi2->m_pName, "svi2 telemetry");
                sprintf(svi2->m_pDescription, "Serial Voltage Interface");
                svi2->m_pFirstChild = nullptr;
                PwrInsertDeviceCounters(svi2, 0, 0);
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
    PrepareDgpuInstanceTable(g_sysInfo, dGpuCntMap);

    //Fill discrete GPUs
    for (cnt = 0; cnt < g_sysInfo.m_smuTable.m_count; cnt++)
    {
        smuInfo = &g_sysInfo.m_smuTable.m_info[cnt];

        if (APU_SMU_ID == smuInfo->m_packageId)
        {
            continue;
        }

        PciDeviceInfo* pDevInfo = nullptr;
        AMDTPwrDevice* dgpu = AllocateDevice(&devCnt);

        dgpu->m_deviceID = deviceIdx++;
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
                    PwrInsertDeviceCounters(dgpu, 0, smuInfo->m_packageId);
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

// PwrEnableProcessProfileCounters
bool PwrEnableProcessProfileCounters()
{
    bool ret = true;
    AMDTUInt32 pwrCounter = 0;

    if (g_sysInfo.m_isAmdApu)
    {
        AMDTUInt32 smuVersion = g_sysInfo.m_smuTable.m_info[0].m_smuIpVersion;

        if ((SMU_IPVERSION_8_0 == smuVersion) || (SMU_IPVERSION_8_1 == smuVersion))
        {
            pwrCounter = COUNTERID_SMU8_APU_PWR_CU;
        }
        else if ((SMU_IPVERSION_7_0 == smuVersion) || (SMU_IPVERSION_7_5 == smuVersion))
        {
            pwrCounter = COUNTERID_SMU7_APU_PWR_CU;
        }
        else
        {
            ret = false;
            PwrTrace("Smu version %d not supported for process profiling", smuVersion);
        }
    }

    if (ret)
    {
        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

        for (auto iter : *pCounters)
        {
            if ((1 == iter.second.m_pkgId)
                && (pwrCounter == iter.second.m_basicInfo.m_attrId)
                && (AMDT_PWR_VALUE_SINGLE == iter.second.m_basicInfo.m_aggr))
            {
                if (!iter.second.m_isActive)
                {
                    PwrActivateCounter(iter.first, true);
                    g_filterCounters.push_back(iter.first);
                    ret = true;
                }
            }
        }
    }

    return ret;
}

void PwrConfigureHistogramCounters()
{
    PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

    if (pCounters)
    {
        for (auto Iter : *pCounters)
        {
            if (Iter.second.m_isActive
                && ((AMDT_PWR_VALUE_CUMULATIVE == Iter.second.m_desc.m_aggregation)
                    || (AMDT_PWR_VALUE_CUMULATIVE == Iter.second.m_desc.m_aggregation)))
            {

                // Check is prevous basic counter is active
                PwrSupportedCounterMap::iterator tempIter = pCounters->find(Iter.first - 1);

                if (tempIter != pCounters->end())
                {
                    if (!tempIter->second.m_isActive)
                    {
                        g_filterCounters.push_back(Iter.first - 1);
                        g_filterCounters.push_back(Iter.first - 1);
                    }
                }
            }
        }
    }
}

AMDTUInt32 PwrGetCoreMask()
{
    AMDTUInt32 mask = 0;

    if (PROFILE_TYPE_PROCESS_PROFILING == g_profileType)
    {
        mask = ~0 ^ (~0 << g_sysInfo.m_coreCnt);
    }
    else
    {

        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

        for (auto iter : *pCounters)
        {
            if (iter.second.m_isActive)
            {
                if (INSTANCE_TYPE_PER_CORE == iter.second.m_basicInfo.m_instanceType)
                {
                    mask |= 1 << iter.second.m_instanceId;
                }
            }
        }
    }

    if (!mask)
    {
        mask = 0x01;
    }

    return mask;
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

// PwrConfigureProfile
bool PwrConfigureProfile()
{
    bool ret = true;
    AMDTUInt32 clientId = 0;
    ProfileConfig config;
    AMDTUInt32 cnt = 0;
    AMDTUInt64 nodeMask = 0;
    PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();
    memset(&config, 0, sizeof(ProfileConfig));


    // Prepare counter mask for all (APU + SMU + dGPU)
    memcpy(&config.m_activeList, &g_sysInfo.m_smuTable, sizeof(SmuList));
    config.m_samplingSpec.m_profileType = g_profileType;
    config.m_samplingSpec.m_samplingPeriod = g_samplingPeriod;
    config.m_samplingSpec.m_mask = PwrGetCoreMask();
    config.m_samplingSpec.m_maskCnt = GetMaskCount(config.m_samplingSpec.m_mask);
    config.m_sampleId = 0;

    for (auto iter : *pCounters)
    {
        if (iter.second.m_isActive)
        {
            AMDTUInt32 smuIdx = iter.second.m_pkgId - 1;
            if(!g_sysInfo.m_isAmdApu || !g_sysInfo.m_smuTable.m_info[0].m_isAccessible)
            {
                smuIdx = smuIdx - 1;
                //PwrTrace("Apu doesn't have smu smuIdx %d", smuIdx);
            }

            if (iter.second.m_pkgId > 0)
            {
                config.m_activeList.m_info[smuIdx].m_counterMask |= (1ULL << iter.second.m_basicInfo.m_attrId);
            }
            else
            {
                nodeMask |= (1ULL << iter.second.m_basicInfo.m_attrId);
            }
        }
    }


    config.m_apuCounterMask = nodeMask;
    config.m_attrCnt = (AMDTUInt16)GetMaskCount(nodeMask);

    for (cnt = 0; cnt < config.m_activeList.m_count; cnt++)
    {
        config.m_attrCnt += (AMDTUInt16)GetMaskCount(config.m_activeList.m_info[cnt].m_counterMask);
    }

    // Register client for the driver
    if (AMDT_STATUS_OK != PwrRegisterClient(&clientId))
    {
        PwrTrace("PwrRegisterClient failed");
        ret = false;
    }

    if (ret)
    {
        ret = (AMDT_STATUS_OK == PwrSetProfileConfiguration(&config, clientId)) ? true : false;
    }

    return ret;
}

// AMDTPwrProfileInitialize: This function loads and initializes the AMDT Power Profile drivers.
// This function should be the first one to be called.
AMDTResult AMDTPwrProfileInitialize(AMDTPwrProfileMode profileMode)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileInitParam param;
    AMDTResult result = AMDT_STATUS_OK;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

    memset(&param, 0, sizeof(AMDTPwrProfileInitParam));

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
        if(nullptr != g_apiMemoryPool.m_pBase)
        {
            ReleaseMemoryPool(&g_apiMemoryPool);
        }
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
        ret = AMDTPwrGetTargetSystemInfo(&g_sysInfo);
        PwrTrace("AMDTPwrGetTargetSystemInfo res 0x%x", ret);
        PwrEnableInternalCounters(true);
        g_isSVI2Supported = PwrIsSVISupported(g_sysInfo);
    }

    if (AMDT_STATUS_OK == ret)
    {
        AMDTUInt32 totalCounters = 0;

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
            //Prepare the counter list
            result = PrepareSystemTopologyInfo();
        }

        if (AMDT_STATUS_OK == ret)
        {
            PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();
            totalCounters = (AMDTUInt32)pCounters->size();

            if (0 == totalCounters)
            {
                // No counters area available for profiler
                ret = AMDT_ERROR_PLATFORM_NOT_SUPPORTED;
            }
        }

        if (AMDT_STATUS_OK == ret)
        {
            // set the profile state to initialized
            ret = PwrSetProfilingState(PowerProfilingInitialized);
        }

        if (AMDT_STATUS_OK == ret)
        {
            //Check if SMU is available
            if ((true == g_sysInfo.m_isPlatformSupported)
                && (true == g_sysInfo.m_isAmdApu)
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

        PwrTrace("Return code: 0x%x counters %d ", ret, totalCounters);
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
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }
        else
        {
            if ((nullptr == pNumCounters) || (nullptr == ppCounterDescs) || (nullptr == pCounters))
            {
                ret = AMDT_ERROR_INVALIDARG;
            }
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_clientCounters.clear();

        if (AMDT_PWR_ALL_DEVICES == deviceID)
        {
            for (auto Iter : *pCounters)
            {
                g_clientCounters.push_back(Iter.second.m_desc);
            }
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
                    for (auto iter : *pCounters)
                    {
                        if (deviceId == iter.second.m_desc.m_deviceId)
                        {
                            g_clientCounters.push_back(iter.second.m_desc);
                            (*pNumCounters)++;
                        }
                    }
                }
            }
        }

        if (g_clientCounters.size() > 0)
        {
            *ppCounterDescs = &g_clientCounters[0];
            *pNumCounters = (AMDTUInt32)g_clientCounters.size();
        }
        else
        {
            *pNumCounters = 0;
            ret = AMDT_ERROR_INVALID_DEVICEID;
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
    PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

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
        if (pCounters->size() < counterID)
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        PwrSupportedCounterMap::iterator Iter = pCounters->find(counterID);

        if (Iter != pCounters->end())
        {
            *pCounterDesc = Iter->second.m_desc;
        }
    }

    return ret;
}

// AMDTPwrEnableCounter: This API will enable the counter to be sampled. This
// API can be used even after the profile run is started.
AMDTResult AMDTPwrEnableCounter(AMDTUInt32 counterID)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_ERROR_FAIL;

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
        AMDTUInt32 pkgId = 0;
        PciPortAddress* pAddress = nullptr;
        bool isAccessible = false;

        // Find the package id
        // dGpu pkgId will always start from index 2 irrespective of AMD or Intel platform
        pkgId = 2 + (counterID % DGPU_COUNTER_BASE_ID) / DGPU_COUNTERS_MAX;

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
        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

        if (pCounters)
        {

            PwrSupportedCounterMap:: iterator Iter = pCounters->find(counterID);

            if (Iter == pCounters->end())
            {
                ret = AMDT_ERROR_INVALID_COUNTERID;
            }
            else
            {
                if (Iter->second.m_isActive)
                {
                    ret = AMDT_ERROR_COUNTER_ALREADY_ENABLED;
                }
                else
                {
                    Iter->second.m_isActive = true;
                }
            }
        }
        else
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
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
        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

        if (pCounters)
        {

            PwrSupportedCounterMap:: iterator Iter = pCounters->find(counterID);

            if (Iter == pCounters->end())
            {
                ret = AMDT_ERROR_INVALID_COUNTERID;
            }
            else
            {
                if (Iter->second.m_isActive)
                {
                    PwrActivateCounter(Iter->first, false);
                }
                else
                {
                    ret = AMDT_ERROR_COUNTER_NOT_ENABLED;
                }
            }
        }
        else
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
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
        AMDTUInt32 counterCnt = 0;
        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

        if (pCounters)
        {
            for (auto iter : *pCounters)
            {
                if (AMDT_PWR_VALUE_SINGLE == iter.second.m_desc.m_aggregation)
                {
                    if (false == iter.second.m_isActive)
                    {
                        PwrActivateCounter(iter.first, true);
                        counterCnt++;
                    }
                }
            }

            if (0 == counterCnt)
            {
                ret = AMDT_ERROR_COUNTER_ALREADY_ENABLED;
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

// AMDTPwrStartProfiling: If the profiler is not running, this will start the
// profiler.
AMDTResult AMDTPwrStartProfiling()
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

    // Set process profile counters
    if ((AMDT_STATUS_OK == ret) && (PROFILE_TYPE_PROCESS_PROFILING == g_profileType))
    {
        if (false == PwrEnableProcessProfileCounters())
        {
            PwrTrace("EnableProcessProfileCounters failed");
            ret = AMDT_ERROR_NOTSUPPORTED;
        }
    }

    // Configure histogram counters
    PwrConfigureHistogramCounters();

    if (AMDT_STATUS_OK == ret)
    {
        ret = PwrConfigureProfile() ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (nullptr != g_pCounterStorage)
        {
            free(g_pCounterStorage);
        }

        g_pCounterStorage = (AMDTPwrCounterValue*)malloc(sizeof(AMDTPwrCounterValue) * PWR_COUNTER_STORAGE_POOL);

        if(nullptr != g_pCounterStorage)
        {
            ret = PwrStartProfiling();
        }
        else
        {
            ret = AMDT_ERROR_OUTOFMEMORY;
        }
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
    AMDTUInt32 counterPoolCnt = 0;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
    PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

    // Check for valid arguments
    if ((nullptr == pNumOfSamples)
        || (nullptr == ppData)
        || (nullptr == pCounters))
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
        AMDTPwrProcessedDataRecord data;

        // Remove counters which were set internally
        if (g_filterCounters.size() > 0)
        {

            for (auto fltIter : g_filterCounters)
            {
                PwrSupportedCounterMap::iterator Iter = pCounters->find(fltIter);

                if (Iter != pCounters->end())
                {
                    Iter->second.m_isActive = false;
                }
            }

            g_filterCounters.clear();
        }

        g_result.clear();

        while (AMDT_ERROR_NODATA != AMDTGetCounterValues(&data))
        {
            isDataAvailable = true;
            AMDTUInt64 startTs = 0 ;
            AMDTUInt32 cnt = 0;
            AMDTPwrSample result;
            memset(&result, 0, sizeof(AMDTPwrSample));

            // Fill the basic information
            result.m_recordId = data.m_recId;
#ifdef LINUX
            // m_ts is actually micro-seconds on Linux, converting this to
            // milli-seconds and rounding off to nearest milli-second
            double tmpMs = data.m_ts / 1000.0 ;
            result.m_elapsedTimeMs = (AMDTUInt64)nearbyint(tmpMs);
#else
            result.m_elapsedTimeMs = data.m_ts;
#endif
            AMDTPwrGetProfileTimeStamps(&startTs, nullptr);
            ConvertTimeStamp(&result.m_systemTime, startTs);

            result.m_counterValues = &g_pCounterStorage[counterPoolCnt];

            if ((nullptr == result.m_counterValues)
                || ((counterPoolCnt + data.m_counters.size()) > PWR_COUNTER_STORAGE_POOL))
            {
                PwrTrace("memory not available counterPoolCnt %d", counterPoolCnt);
                break;
            }

            for (auto iter : data.m_counters)
            {
                PwrSupportedCounterMap:: iterator supIter = pCounters->find(iter.second.m_counterId);

                AMDTFloat32* value = &result.m_counterValues[cnt].m_counterValue;

                if ((nullptr != value) && supIter != pCounters->end())
                {
                    //Prepare the output data now
                    if (PWR_UNIT_TYPE_COUNT == supIter->second.m_basicInfo.m_unitType)
                    {
                        *value = (AMDTFloat32)iter.second.m_value64;
                    }
                    else
                    {
                        *value = (AMDTFloat32)iter.second.m_float32;
                    }

                    result.m_counterValues[cnt].m_counterID = iter.second.m_counterId;
                    counterPoolCnt++;
                    cnt++;
                }
                else
                {
                    ret = AMDT_ERROR_INTERNAL;
                    PwrTrace("counter not found");
                }
            }

            if (cnt > 0)
            {
                result.m_numOfValues = cnt;
                g_result.push_back(result);
            }
        };

        if (false == isDataAvailable)
        {
            ret = AMDT_ERROR_NODATA;
        }

        if (AMDT_STATUS_OK == ret)
        {
            if (g_result.size() > 0)
            {
                *pNumOfSamples = (AMDTUInt32)g_result.size();
                *ppData = &g_result[0];
            }
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
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTUInt32 instanceId = 0;
    PwrSupportedCounterMap* pCounters = nullptr;

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
        pCounters = PwrGetSupportedCounterList();

        PwrSupportedCounterMap::iterator iter = pCounters->find(counterID);

        if (iter != pCounters->end())
        {
            if (AMDT_PWR_VALUE_HISTOGRAM != iter->second.m_desc.m_aggregation)
            {
                ret = AMDT_ERROR_INVALID_COUNTERID;
            }

            instanceId = iter->second.m_instanceId;
        }
    }

    if (ret == AMDT_STATUS_OK)
    {
        AMDTPwrHistogram* pHistogram = nullptr;

        if (AMDT_PWR_ALL_COUNTERS == counterID)
        {
            for (auto iter : *pCounters)
            {
                if ((AMDT_PWR_VALUE_HISTOGRAM == iter.second.m_desc.m_aggregation)
                    && (instanceId == iter.second.m_instanceId))
                {
                    pHistogram = GetHistogramCounter(iter.first);

                    if (nullptr != pHistogram)
                    {
                        g_histogramResult.push_back(*pHistogram);
                        (*pNumEntries)++;
                    }
                    else
                    {
                        ret = AMDT_ERROR_INTERNAL;
                        PwrTrace("error: AMDT_ERROR_INTERNAL pHistogram==NULL for list");
                    }
                }
            }

        }
        else
        {
            pHistogram = GetHistogramCounter(counterID);

            if (nullptr != pHistogram)
            {
                g_histogramResult.push_back(*pHistogram);
                *ppData = &g_histogramResult[0];
                *pNumEntries = 1;
            }
            else
            {
                ret = AMDT_ERROR_INTERNAL;
                PwrTrace("error: AMDT_ERROR_INTERNAL pHistogram==NULL");
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
    PwrSupportedCounterMap* pCounters = nullptr;

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
        g_cummulativeResult.clear();
        pCounters = PwrGetSupportedCounterList();

        if (AMDT_PWR_ALL_COUNTERS == counterId)
        {
            for (auto iter : *pCounters)
            {
                if (iter.second.m_isActive && (AMDT_PWR_VALUE_CUMULATIVE != iter.second.m_desc.m_aggregation))
                {
                    AMDTFloat32* value = GetCumulativeCounter(counterId);

                    if (value)
                    {
                        g_cummulativeResult.push_back(*value);
                        (*pNumEntries)++;
                    }
                }
            }

            if (g_cummulativeResult.size() > 0)
            {
                *ppData = &g_cummulativeResult[0];
            }
        }
        else
        {

            PwrSupportedCounterMap::iterator iter = pCounters->find(counterId);

            if (iter != pCounters->end())
            {
                AMDTFloat32* value = GetCumulativeCounter(counterId);

                if (value)
                {
                    g_cummulativeResult.push_back(*value);
                    *ppData = &g_cummulativeResult[0];
                    *pNumEntries = 1;
                }

                ret = AMDT_STATUS_OK;
            }
            else
            {
                ret = AMDT_ERROR_COUNTERS_NOT_ENABLED;
            }
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
    AMDTResult ret = AMDT_ERROR_INVALID_COUNTERID;
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

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
        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();
        PwrSupportedCounterMap::iterator iter = pCounters->find(counterID);

        if (iter != pCounters->end())
        {
            ret = iter->second.m_isActive ? AMDT_STATUS_OK : AMDT_ERROR_COUNTERS_NOT_ENABLED;
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
    AMDTUInt32 counter = 0;

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
        PwrSupportedCounterMap* pCounters = PwrGetSupportedCounterList();

        for (auto iter : *pCounters)
        {
            if (iter.second.m_isActive)
            {
                counter++;
            }
        }

        *pCount = counter;
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
    AMDTPwrCounterHierarchy* pRel = nullptr;
    AMDTUInt32 childCnt = 0;
    AMDTUInt32 size = sizeof(g_InternalCounterHirarchy) / sizeof(AMDTPwrCounterHierarchy);
    PwrSupportedCounterMap* pCounters = nullptr;

    if (nullptr != pInfo)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if ((AMDT_STATUS_OK == ret) && (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state))
    {
        ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
    }

    bool foundCounterHierarchy = false;

    if (AMDT_STATUS_OK == ret)
    {

        PwrCounterInfo* pCounterInfo = nullptr;
        pCounters = PwrGetSupportedCounterList();

        PwrSupportedCounterMap::iterator iter = pCounters->find(id);

        if (iter != pCounters->end())
        {
            pCounterInfo = &iter->second;
        }
        else
        {
            ret = AMDT_ERROR_INVALID_COUNTERID;
        }

        pInfo->m_childCnt = 0;

        if (AMDT_STATUS_OK == ret)
        {
            // Find the hierarchy information from the table
            for (cnt = 0; cnt < size; cnt++)
            {
                if (pCounterInfo->m_basicInfo.m_attrId == g_InternalCounterHirarchy[cnt].m_counter)
                {
                    AMDTUInt32 cnt1 = 0;
                    // Prepare Child list
                    pRel = &g_InternalCounterHirarchy[cnt];

                    for (cnt1 = 0; cnt1 < pRel->m_childCnt; cnt1++)
                    {
                        AMDTUInt32 childBaseId = *(pRel->m_pChildList + cnt1);

                        for (auto Iter : *pCounters)
                        {
                            if (Iter.second.m_basicInfo.m_attrId == childBaseId)
                            {
                                g_ChildCounterList[childCnt++] = Iter.first;
                            }
                        }
                    }

                    pInfo->m_childCnt = childCnt;
                    pInfo->m_pChildList = &g_ChildCounterList[0];
                    pInfo->m_counter = id;

                    for (auto iterCtr : *pCounters)
                    {
                        if (iterCtr.second.m_basicInfo.m_attrId == g_InternalCounterHirarchy[cnt].m_parent)
                        {
                            pInfo->m_parent = iterCtr.first;
                        }
                    }

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
        if ((false == g_sysInfo.m_isAmdApu) || (false == g_sysInfo.m_smuTable.m_info[0].m_isAccessible))
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

// AMDTPwrGetNodeTemperature: This API provides the note temperature in tctl
// scale or internally add any offset provided by the end user.
AMDTResult AMDTPwrGetNodeTemperature(AMDTFloat32* pNodeTemp)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;
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

    // Orochi
    isSupported = ((0x15 == g_sysInfo.m_family)
                   && ((0x00 == g_sysInfo.m_model) || ((0x00 < g_sysInfo.m_model) && (g_sysInfo.m_model <= 0x0f))));

    isSupported |= ((0x15 == g_sysInfo.m_family) && ((0x30 <= g_sysInfo.m_model) && (g_sysInfo.m_model <= 0x3f))); // KV

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

// This API will provide the list of running processes/Modules/ip samples collected by the profiler
// so far from the time of profile start or bewteen two consecutive call of this function,
// their agreegated power indicators. This API can be called at any
// point of time from start of the profile to the stop of the profile.
AMDTResult AMDTPwrGetModuleProfileData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCount, AMDTFloat32* pPower)
{

#if (defined(_WIN64) || defined(LINUX))
    (void)ppData;
    (void)pModuleCount;
    (void)pPower;
    return AMDT_ERROR_NOTSUPPORTED;
#else
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

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

