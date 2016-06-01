//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileControl.cpp
///
//==================================================================================

#if ( defined (_WIN32) || defined (_WIN64) )
    #include <windows.h>
    #include <AMDTDriverControl/inc/DriverControl.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <ppCountersStringConstants.h>
#include <AMDTRawDataFileHeader.h>
#include <PowerProfileHelper.h>
#include <PowerProfileDriverInterface.h>
#include <AMDTPowerProfileControl.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTPowerProfileDataTypes.h>
#include <AMDTSmu8Interface.h>
#include <SupportedDeviceList.h>
#include <OsFileWrapper.h>

#define SMU_RETRY_COUNT_MAX       1024
#define SMU_LOG_MSG_RETRY_MAX       32
#define DEVICE_TRACE_STR "Device:B%d/D%d/F%d dev-id:0x%x, hw-type:%d, dev-type:%d, model:%s, sname:%s, ip-ver: %d, access:%d"
// This macro needs to be removed in release branch
#define CONTROL_POOL_SIZE 1048576 // 1MB

// Globals
extern AMDTUInt8* g_pSharedBuffer;
extern bool g_internalCounters;
extern bool g_isSVI2Supported;

PwrInternalAddr g_internalCounterAddr;
// Create control layer pool.
static MemoryPool g_controlMemoryPool;
wchar_t g_rawFileName[MAX_PATH_LEN];
static bool g_isOnline = false;
static bool g_isSvi2Selected = false;

AMDTPwrProfilingState g_powerProfileState = PowerProfilingUnavailable;
static AMDTPwrTargetSystemInfo* g_pTargetSystemInfo = NULL;

PciDeviceInfo g_activePciDevice[PLATFORM_SMU_CNT + 2];
PciPortAddress  g_activePciPortList[PLATFORM_SMU_CNT + 2 ];

static AMDTPwrApuPstateList g_pStateInfo;

static bool g_smuRestore = false;
bool GetAvailableSmuList(SmuList* pList);

// Basic Counters- Sample id, record-id, timestamp
AMDTPwrAttributeTypeInfo g_basicCounters[] =
{
#include "BasicCounterList.h"
};

// Node specific Counters- PID, TID, pstate, cef, node temperature
AMDTPwrAttributeTypeInfo g_nodeCounters[] =
{
#include "NodeCounterList.h"
};


// Smu7.0- Kaveri
AMDTPwrAttributeTypeInfo g_smu7Counters[] =
{
#include "Smu7CounterList.h"
};

// Smu7.5 -Mullins
AMDTPwrAttributeTypeInfo g_smu75Counters[] =
{
#include "Smu75CounterList.h"
};

// Smu8 - Carrizo
AMDTPwrAttributeTypeInfo g_smu8Counters[] =
{
#include "Smu8CounterList.h"
};

AMDTPwrAttributeTypeInfo g_CounterDgpuSmu7_0[] =
{
#include "Smu7DgpuCounterList.h"
};

/** \typedef ProfileControlHandle
    \brief A handle to identify which profiles were opened for driver work.
    \ingroup datafiles

    The handle is necessary for all functions related to retrieving data from
    the opened profiles.
*/
typedef void* ProfileControlHandle;


//AMDTPwrProfileInitialize-This will set up the power profiling driver for your profile .
AMDTResult PwrProfileInitialize(
    AMDTPwrProfileInitParam* pParam)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (!pParam)
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("PwrProfileInitialize failed as pParam == NULL");
    }

    g_isOnline = (bool)pParam->isOnline;

    if (AMDT_STATUS_OK == ret)
    {
        //Driver initialization
        ret = OpenPowerProfileDriver();
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Create memory pool for API layer
        ret = CreateMemoryPool(&g_controlMemoryPool, CONTROL_POOL_SIZE);
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_powerProfileState = PowerProfilingInitialized;
        memset(&g_attributeList, 0, sizeof(AMDTPwrProfileAttributeList));
        g_attributeList.pAttrList = (AMDTPwrAttributeTypeInfo*) GetMemoryPoolBuffer(&g_controlMemoryPool, COUNTERID_MAX_CNT * sizeof(AMDTPwrAttributeTypeInfo));

        if (NULL == g_attributeList.pAttrList)
        {
            ret = AMDT_ERROR_OUTOFMEMORY;
        }
    }

    // Initialize p state structure
    g_pStateInfo.m_cnt = 0;

    return ret;
}

// PwrProfileUnregisterClient: Unregister client from driver's client list
// Release all internel memory allocated by driver and state the state
// machine to PowerProfilingUnavailable.
AMDTResult PwrProfileUnregisterClient()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 res = 0;
    AMDTUInt32 clientId = 0;

    //Check if profiler is not event initialized
    if (g_powerProfileState == PowerProfilingUnavailable)
    {
        ret = AMDT_ERROR_ACCESSDENIED;
    }

    if ((g_powerProfileState == PowerProfiling) ||
        (g_powerProfileState == PowerProfilingPaused))
    {
        ret = AMDT_STATUS_PENDING;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = GetPowerDriverClientId(&clientId);
    }


    if (AMDT_STATUS_OK == ret)
    {
        //unregister driver
        ret = CommandPowerDriver(DRV_CMD_TYPE_IN,
                                 IOCTL_UNREGISTER_CLIENT,
                                 &clientId,
                                 sizeof(AMDTUInt32),
                                 0,
                                 0,
                                 &res);
    }

    // Set The profiling state to uninitialised
    g_powerProfileState = PowerProfilingStopped;

    if (AMDT_STATUS_OK != ret)
    {
        PwrTrace("PwrProfileUnregisterClient failed:");
    }

    return ret;
}

// PwrProfileClose: Close the power profiler seession.
// Cleanup all memory allocated by backend
AMDTResult PwrProfileClose()
{
    AMDTResult ret = AMDT_STATUS_OK;

    // Check the status of the power profiling
    if (PowerProfiling == g_powerProfileState)
    {
        PwrStopProfiling();
    }

    g_powerProfileState = PowerProfilingUnavailable;

    // Release the memory pool buffer
    ret = ReleaseMemoryPool(&g_controlMemoryPool);

    // Relase g_pTargetSystemInfo memory
    if (NULL != g_pTargetSystemInfo)
    {
        free(g_pTargetSystemInfo);
        g_pTargetSystemInfo = NULL;
    }

    return ret;
}

// PwrGetSupportedAttributeList: This function prepares the list of supported counters
// including APU and dGPU which are connected to the platform
// This function needs to be modified whenever there is a new addion of SMU version
// for APU and dGPU
AMDTResult PwrGetSupportedAttributeList(
    /*out*/AMDTPwrProfileAttributeList* pList)

{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 size = 0;
    AMDTPwrTargetSystemInfo sysInfo;
    AMDTUInt32 smuCnt = 0;
    AMDTUInt32 counterCnt = 0;

    if (0 == g_attributeList.attrCnt)
    {
        // Get base counter list
        memcpy(g_attributeList.pAttrList, &g_basicCounters[0], sizeof(g_basicCounters));
        counterCnt = sizeof(g_basicCounters) / sizeof(AMDTPwrAttributeTypeInfo);
        g_attributeList.attrCnt = counterCnt;

        ret = AMDTPwrGetTargetSystemInfo(&sysInfo);

        if ((AMDT_STATUS_OK == ret) && (true == sysInfo.m_isPlatformSupported))
        {

            if ((true == sysInfo.m_isAmdApu) && (nullptr != sysInfo.m_pNodeInfo))
            {
                if (SMU_IPVERSION_7_0 == sysInfo.m_pNodeInfo->m_smuIpVersion)

                {
                    memcpy(g_attributeList.pAttrList + g_attributeList.attrCnt,
                           &g_smu7Counters[0],
                           sizeof(g_smu7Counters));

                    counterCnt = sizeof(g_smu7Counters) / sizeof(AMDTPwrAttributeTypeInfo);
                    g_attributeList.attrCnt += counterCnt;
                }
                else if (SMU_IPVERSION_7_5 == sysInfo.m_pNodeInfo->m_smuIpVersion)
                {
                    memcpy(g_attributeList.pAttrList + g_attributeList.attrCnt,
                           &g_smu75Counters[0],
                           sizeof(g_smu75Counters));

                    counterCnt = sizeof(g_smu75Counters) / sizeof(AMDTPwrAttributeTypeInfo);
                    g_attributeList.attrCnt += counterCnt;
                }
                else if ((SMU_IPVERSION_8_0 == sysInfo.m_pNodeInfo->m_smuIpVersion)
                         || (SMU_IPVERSION_8_1 == sysInfo.m_pNodeInfo->m_smuIpVersion))
                {
                    memcpy(g_attributeList.pAttrList + g_attributeList.attrCnt,
                           &g_smu8Counters[0],
                           sizeof(g_smu8Counters));

                    counterCnt = sizeof(g_smu8Counters) / sizeof(AMDTPwrAttributeTypeInfo);
                    g_attributeList.attrCnt += counterCnt;
                }
            }

            // Add AMD node counters like pstate,PID,TID, node temperature
            memcpy(g_attributeList.pAttrList + g_attributeList.attrCnt,
                   &g_nodeCounters[0],
                   sizeof(g_nodeCounters));

            counterCnt = sizeof(g_nodeCounters) / sizeof(AMDTPwrAttributeTypeInfo);
            g_attributeList.attrCnt += counterCnt;
        }

        for (smuCnt = 0; smuCnt < sysInfo.m_smuTable.m_count; smuCnt++)
        {
            AMDTUInt32 id = 0;
            AMDTPwrAttributeTypeInfo* attrInfo = NULL;
            SmuInfo* smuInfo = &sysInfo.m_smuTable.m_info[smuCnt];

            if (APU_SMU_ID == smuInfo->m_packageId)
            {
                continue;
            }

            if ((SMU_IPVERSION_7_0 == smuInfo->m_smuIpVersion)
                || (SMU_IPVERSION_7_1 == smuInfo->m_smuIpVersion)
                || (SMU_IPVERSION_7_2 == smuInfo->m_smuIpVersion))
            {
                size = sizeof(g_CounterDgpuSmu7_0);
                attrInfo = g_attributeList.pAttrList + g_attributeList.attrCnt;
                memcpy(attrInfo, &g_CounterDgpuSmu7_0[0], size);
                g_attributeList.attrCnt += (size / sizeof(AMDTPwrAttributeTypeInfo));

                for (id = 0; id < (size / sizeof(AMDTPwrAttributeTypeInfo)); id++)
                {
                    attrInfo[id].m_attrId = attrInfo[id].m_attrId
                                            + DGPU_COUNTER_BASE_ID
                                            + (smuInfo->m_packageId - APU_SMU_ID - 1)
                                            * DGPU_COUNTERS_MAX;
                }
            }
        }
    }

    pList->pAttrList = g_attributeList.pAttrList;
    pList->attrCnt = g_attributeList.attrCnt;
    return ret;
}

// SetAttributeMask: Prepare and set counter mask for each accessible components
// Line smu, Pcie, MSR access
AMDTResult SetAttributeMask(AMDTUInt16* clientAttrList , AMDTUInt32 attrCnt, ProfileConfig* cfg)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    g_isSvi2Selected = false;
    AMDTUInt64* pMask = nullptr;

    if (!clientAttrList)
    {
        ret = AMDT_ERROR_INVALIDARG;
        PwrTrace("clientAttrList == NULL");
    }

    pMask = &cfg->m_apuCounterMask;

    if (AMDT_STATUS_OK == ret)
    {
        AMDTPwrProfileAttributeList supportedList;

        //Get the actual id from supported list
        PwrGetSupportedAttributeList(&supportedList);

        for (cnt = 0; cnt < cfg->m_activeList.m_count; cnt++)
        {
            cfg->m_activeList.m_info[cnt].m_counterMask = 0;

        }

        for (cnt = 0; cnt < attrCnt; cnt++)
        {
            AMDTUInt16 idx = *(clientAttrList + cnt);

            if (DGPU_COUNTER_BASE_ID <= idx)
            {
                AMDTUInt32 loop = 0;
                AMDTUInt32 counterId = (idx - DGPU_COUNTER_BASE_ID) % DGPU_COUNTERS_MAX;

                //Find the correct package id for this counter
                AMDTUInt32 pkgId = APU_SMU_ID + ((idx - DGPU_COUNTER_BASE_ID) / DGPU_COUNTERS_MAX) + 1;

                for (loop = 0; loop < cfg->m_activeList.m_count; loop++)
                {
                    if (cfg->m_activeList.m_info[loop].m_packageId == pkgId)
                    {
                        cfg->m_activeList.m_info[loop].m_counterMask |= (1ULL << counterId);
                        break;
                    }
                }
            }
            else
            {
                AMDTPwrProfileAttrType id;
                id = (AMDTPwrProfileAttrType)(supportedList.pAttrList + idx)->m_attrId;
                SetRawAttributeMask((AMDTUInt16)id, pMask);

                if ((COUNTERID_SVI2_CORE_TELEMETRY == id)
                    || (COUNTERID_SVI2_NB_TELEMETRY == id))
                {
                    g_isSvi2Selected = true;
                }
            }
        }

        if (*pMask & SMU_ATTRIBUTE_MASK)
        {
            cfg->m_activeList.m_info[0].m_counterMask = (*pMask & SMU_ATTRIBUTE_MASK);
        }
    }

    return ret;
}

// Bios message service registers
#define SMU7_CPU_INT_STATUS                       0xC2100004
#define SMU7_CPU_INT_ARGUMENT                     0xC210003C
#define SMU7_CPU_INT_REQUEST                      0xC2100000
#define SMU7_PWR_FEATURE_MASK                     0x13
#define SMU7_BAPM_ON_BIT                          8
#define SMU7_GPU_CAC_ON_BIT                       15
#define SMU7_PKG_PWR_LIMITING_BIT                 23
#define SMU7_BIOSSMC_MSG_ENABLE_ALL_SMU_FEATURES  0x5F
#define SMU7_BIOSSMC_MSG_DISABLE_ALL_SMU_FEATURES 0x60
#define ENABLE_ALL_SMU7_FEATURES                  0x5F
#define ENABLE_ALL_SMU7_FEAUTRE_MASK              0x13
#define SMU7_FEATURE_STATUS_REGISTER              0x3f818
//NB Spec registers
#define SMU_INDEX_ADDR        0x800000B8
#define SMU_INDEX_DATA        0x800000BC

// AccessBCB8Pairs: Access SMU BC B8 pairs
bool AccessBCB8Pairs(AMDTUInt32 reg, AMDTUInt32* pData, bool isRead)
{
    bool ret = false;
    AMDTResult res = AMDT_STATUS_OK;
    ACCESS_PCI pciData;
    pciData.address = SMU_INDEX_ADDR;
    pciData.data = reg;
    pciData.isReadAccess = false;

    // Write index register
    res = AccessPciAddress(&pciData);

    if (AMDT_STATUS_OK == res)
    {
        // Read SMU soft register
        pciData.address = SMU_INDEX_DATA;
        pciData.isReadAccess = isRead;
        AccessPciAddress(&pciData);
        *pData = pciData.data;
        ret = true;
    }

    return ret;
}

// PwrIsSVISupported: Check if SVI2 is supported
bool PwrIsSVISupported(const AMDTPwrTargetSystemInfo& sysInfo)
{
    bool ret = false;

    if (nullptr != sysInfo.m_pNodeInfo)
    {
        HardwareType type = sysInfo.m_pNodeInfo->m_hardwareType;

        if ((GDT_SPECTRE == type)
            || (GDT_SPECTRE_LITE == type)
            || (GDT_SPECTRE_SL == type)
            || (GDT_KALINDI == type))
        {
            ret = true;
        }
        else
        {
            PwrTrace("SVI2 counters are not supported in this platform");
        }
    }

    return ret;
}

// PwrIsInternalCounterAvailable: Check if internal counters are available
bool PwrIsInternalCounterAvailable()
{
    bool ret = false;
    FILE* pFile = nullptr;

    memset(&g_internalCounterAddr, 0 , sizeof(PwrInternalAddr));
    // For writing the file
    //ret = PwrOpenFile(&pFile, "AMDTPwrInternalCounter.bin", "wb");
    //ret = PwrWriteFile(pFile, (AMDTUInt8*)&g_internalCounterAddr, sizeof(PwrInternalAddr));

    // Read internal counter address file
    ret = PwrOpenFile(&pFile, "Data\\AMDTPwrInternalCounter.bin", "r");

    // If internal file is available update the address from internal file
    // else reset all address to 0
    if (ret)
    {
        // Read the file into the buffer:
        ret = PwrReadFile(pFile, (AMDTUInt8*)&g_internalCounterAddr, sizeof(PwrInternalAddr));
    }
    else
    {
        PwrTrace("Internal Counters are not available");
    }

    return ret;
}

// PwrWriteInternalCounterAddres: Write internal counter information to shared buffer
void PwrWriteInternalCounterAddres()
{
    if (nullptr != g_pSharedBuffer)
    {
        memcpy((PwrInternalAddr*)&g_pSharedBuffer[PWR_INTERNAL_COUNTER_BASE], &g_internalCounterAddr, sizeof(PwrInternalAddr));
    }
}

// IsSMUFeatureEnabled:  Check if BAPM/ PKG power limiting etc are enabled
bool IsSMUFeatureEnabled()
{

    AMDTUInt32 statusReg = 0;
    bool ret = false;
    AMDTResult result = false;
    ACCESS_PCI pciData;
    pciData.address = SMU_INDEX_ADDR;
    pciData.data = SMU7_FEATURE_STATUS_REGISTER;
    pciData.isReadAccess = false;

    // Write index register
    result = AccessPciAddress(&pciData);

    if (AMDT_STATUS_OK == result)
    {
        // Read SMU soft register
        pciData.address = SMU_INDEX_DATA;
        pciData.isReadAccess = true;
        AccessPciAddress(&pciData);
        statusReg = pciData.data;
    }

    ret = (statusReg & (1 << SMU7_BAPM_ON_BIT)) ? 1 : 0;
    ret &= (statusReg & (1 << SMU7_GPU_CAC_ON_BIT)) ? 1 : 0;
    ret &= (statusReg & (1 << SMU7_PKG_PWR_LIMITING_BIT)) ? 1 : 0;
    return ret;
}

// IsIGPUAvailable: Check if iGPU is available with B0D1F0x18 register
// refer to the BKDG for this register
bool IsIGPUAvailable()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    ACCESS_PCI pciData;

    GET_EXTENED_PCICS_ADDRESS(0x0U, 0x1U, 0x0U, 0x18U, pciData.address)
    pciData.isReadAccess = 1;

    //Get PCI Access
    ret = AccessPciAddress(&pciData);

    if (AMDT_STATUS_OK != ret)
    {
        PwrTrace("IsIGPUAvailable failed for PCIe access");
    }

    return (pciData.data < 0xFFFFFFFF ? true : false);
}

// PwrRegisterClient: Register client with the driver's client list and get a client id.
// Only one client is allowed at a time at this moment.
AMDTResult PwrRegisterClient(AMDTUInt32* clientId)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTInt32 res = 0;
    bool result = false;

    if (PowerProfilingInitialized != g_powerProfileState)
    {
        ret = AMDT_ERROR_ACCESSDENIED;
        PwrTrace("PwrRegisterClient failed as state not eq PowerProfilingInitialized");
    }

    if (AMDT_STATUS_OK == ret)
    {
        result = 0;
        //Check if driver is opened
        result = IsPowerDriverOpened();
    }

    if (!result)
    {
        ret = AMDT_ERROR_ACCESSDENIED;
        PwrTrace("IsPowerDriverOpened failed");
    }

    if (AMDT_STATUS_OK == ret)
    {
        // Register the client
        res = RegisterClientWithDriver(clientId);
    }

    if ((AMDT_STATUS_OK != res) || *clientId > (MAX_CLIENT_COUNT - 1))
    {
        ret = AMDT_ERROR_ACCESSDENIED;
        PwrTrace("RegisterClientWithDriver failed");
    }

    return ret;
}

// PwrSetProfilingConfiguration: Set profile configuration to the driver data structure
// so that driver can initiliaze and create required data structure and memories.
AMDTResult PwrSetProfilingConfiguration(
    /*in*/ AMDTPwrProfileConfig* pConfig)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTInt32 res = 0;
    PROF_CONFIGS cfgList;
    ProfileConfig cfg;
    memset(&cfgList, 0, sizeof(PROF_CONFIGS));

    if ((AMDT_STATUS_OK == ret) &&
        (!pConfig ||
         !pConfig->m_pAttrList ||
         !pConfig->m_pSpecList ||
         !pConfig->m_attrCnt ||
         //Number of spec is restricted to 1 at this moment
         pConfig->m_specCnt != 1))
    {
        ret = AMDT_ERROR_INVALIDARG;
        PwrTrace("PwrSetProfilingConfiguration failed: as one of the arg is invalid");
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = InitializeSharedBuffer();
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Prepare the list for the driver
        AMDTPwrTargetSystemInfo sysInfo;
        AMDTPwrGetTargetSystemInfo(&sysInfo);
        memset(&cfg, 0, sizeof(cfg));
        memcpy(&cfg.m_activeList, &sysInfo.m_smuTable, sizeof(SmuList));
        cfgList.ulStatus = 0;
        cfgList.ulConfigCnt = 1;
        cfgList.uliProfileConfigs = reinterpret_cast<uint64>(&cfg);
        cfg.m_attrCnt = (AMDTUInt16)pConfig->m_attrCnt;
        cfg.m_samplingSpec.m_profileType = pConfig->m_pSpecList->m_profileType;
        cfg.m_apuCounterMask = 0;
        SetAttributeMask(pConfig->m_pAttrList, pConfig->m_attrCnt, &cfg);
        cfg.m_samplingSpec.m_samplingPeriod = (uint32)pConfig->m_pSpecList->m_samplingPeriod;

        //Set core mask count & mask
        cfg.m_samplingSpec.m_maskCnt = pConfig->m_pSpecList->m_maskCnt;
        cfg.m_samplingSpec.m_mask = pConfig->m_pSpecList->m_mask;

        cfgList.ulClientId = pConfig->m_clientId;

        // Check if internal counter file is present
        PwrWriteInternalCounterAddres();

        //Instruct driver for the configuration.
        ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                                 IOCTL_ADD_PROF_CONFIGS,
                                 &cfgList,
                                 sizeof(PROF_CONFIGS),
                                 &cfgList,
                                 sizeof(PROF_CONFIGS),
                                 &res);
    }

    if (PROF_ERROR_SMU_CONGIGURATION == cfgList.ulStatus)
    {
        ret = AMDT_ERROR_BIOS_VERSION_NOT_SUPPORTED;
    }

    if (AMDT_STATUS_OK != ret)
    {
        PwrTrace("failed as IOCTL_ADD_PROF_CONFIGS failed");
    }

    return ret;
}

// EnableSVI2Telemetry: SVI2 telemetry voltage and current is supported
// only for internal version of Kavery & Mullins platform
bool EnableSVI2Telemetry(bool isEnable)
{
    AMDTResult ret = false;
    // Wait for the VOFT complete indication from the voltage regulator
    // before macking additional voltage change request
    ACCESS_PCI pci;
    pci.address = g_internalCounterAddr.m_timingControl4;
    pci.isReadAccess = false;
    pci.data = (1 << 30);
    ret = AccessPciAddress(&pci);

    if (AMDT_STATUS_OK == ret)
    {
        // Enable SVI2 now. Telemetry enable in Voltage & Current mode
        memset(&pci, 0, sizeof(ACCESS_PCI));
        pci.address = g_internalCounterAddr.m_timingControl6;
        pci.isReadAccess = false;

        if (true == isEnable)
        {
            pci.data = 0x01;
        }
        else
        {
            pci.data = 0x00;
        }

        ret = AccessPciAddress(&pci);
    }

    return (AMDT_STATUS_OK == ret) ? true : false;
}

//AMDTPwrStartProfiling- If the profiler is not running, this will start the profiler, and cause the
AMDTResult PwrStartProfiling()

{
    AMDTInt32 ret = AMDT_STATUS_OK;

    PROFILER_PROPERTIES param;
    AMDTUInt32 res = 0;
    AMDTUInt32 clientId = 0;

    memset(&param, 0, sizeof(PROFILER_PROPERTIES));

    //Check power profile state
    if ((PowerProfiling == g_powerProfileState) ||
        (PowerProfilingUnavailable == g_powerProfileState))
    {
        ret = AMDT_STATUS_PENDING;
        PwrTrace("Failed as state not correct %d", g_powerProfileState);
    }

    //Check if profiler is initialized
    if (PowerProfilingInitialized != g_powerProfileState)
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("Failed: state %d", g_powerProfileState);
    }

    memset(&param, 0, sizeof(PROFILER_PROPERTIES));

    if (AMDT_STATUS_OK == ret)
    {
        //Check driver initialization is done
        ret = GetPowerDriverClientId(&clientId);
    }

    if (AMDT_STATUS_OK != ret)
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("Failed: as GetPowerDriverClientId failed");
    }

    if (AMDT_STATUS_OK == ret)
    {

        if (true == g_isSvi2Selected)
        {
            // Enable SVI2
            // SVI2 is supported only on internal & CodeXL build and
            // for Kaveri & Mullins platforms
            if (g_internalCounters
                && g_isSVI2Supported)
            {
                EnableSVI2Telemetry(true);
            }
            else
            {
                g_isSvi2Selected = false;
            }
        }

        memset(&param, 0, sizeof(PROFILER_PROPERTIES));
        //IOCTL command to set the output file
        param.ulClientId = clientId;
        param.ulProfileMode = g_isOnline;

        param.hAbort = 0;
        ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                                 IOCTL_START_PROFILER,
                                 &param,
                                 sizeof(PROFILER_PROPERTIES),
                                 &param,
                                 sizeof(PROFILER_PROPERTIES),
                                 &res);
    }

    if (PROF_ERROR_SMU_CONGIGURATION == param.ulStatus)
    {
        ret = AMDT_ERROR_BIOS_VERSION_NOT_SUPPORTED;
    }

    g_powerProfileState = PowerProfiling;
    return ret;
}


//AMDTPwrStopProfiling- If the profiler is running, this will stop the profiler, and cause the
//state to go to ProfilingStopped.  If sampling, no further data will
//be written to the profile files.
AMDTResult PwrStopProfiling()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTInt32 res = 0;
    AMDTInt32 status = 0;
    AMDTUInt32 clientId = 0;

    //Check the status of the power profiling
    if ((PowerProfiling != g_powerProfileState) && (PowerProfilingPaused != g_powerProfileState))
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("PwrStopProfiling failed: as state not correct %d", g_powerProfileState);
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = GetPowerDriverClientId(&clientId);
    }

    if (g_smuRestore)
    {
        EnableSmu(false);
        g_smuRestore = false;
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Stop the driver
        ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                                 IOCTL_STOP_PROFILER,
                                 &clientId,
                                 sizeof(AMDTUInt64),
                                 &status,
                                 sizeof(AMDTUInt64),
                                 &res);
    }


    if (AMDT_STATUS_OK == ret)
    {
        if (true == g_isSvi2Selected)
        {
            // SVI2 votlage and current is enables. So need to disable now
            EnableSVI2Telemetry(false);
        }

        g_powerProfileState = PowerProfilingStopped;
        PwrProfileUnregisterClient();
    }
    else
    {
        PwrTrace("PwrStopProfiling failed: as IOCTL_STOP_PROFILER failed");
    }

    return ret;
}

//AMDTPwrPauseProfiling- If the profiler is running, this will pause the profiler
AMDTResult PwrPauseProfiling()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTInt32 res = 0;
    AMDTInt32 status = 0;
    AMDTUInt32 clientId = 0;

    //Check the status of the power profiling
    if (PowerProfiling != g_powerProfileState)
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("PwrPauseProfiling failed: as state not correct %d", g_powerProfileState);
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = GetPowerDriverClientId(&clientId);
    }

    if (AMDT_STATUS_OK == ret)
    {

        //Stop the driver
        ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                                 IOCTL_PAUSE_PROFILER,
                                 &clientId,
                                 sizeof(AMDTUInt32),
                                 &status,
                                 sizeof(AMDTUInt32),
                                 &res);

        if (AMDT_STATUS_OK != ret)
        {
            PwrTrace("PwrPauseProfiling failed: as IOCTL_PAUSE_PROFILER failed");
        }
    }

    g_powerProfileState = PowerProfilingPaused;

    return ret;
}

//int AMDTPwrResumeProfiling()- If the profiler is in paused state, this will resume the profiler
AMDTResult PwrResumeProfiling()

{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTInt32 res = 0;
    AMDTInt32 status = 0;
    AMDTUInt32 clientId = 0;

    //Check the status of the power profiling
    if (PowerProfilingPaused != g_powerProfileState)
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("PwrResumeProfiling failed: as state not correct %d", g_powerProfileState);
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = GetPowerDriverClientId(&clientId);
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Stop the driver
        ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                                 IOCTL_RESUME_PROFILER,
                                 &clientId,
                                 sizeof(AMDTUInt32),
                                 &status,
                                 sizeof(AMDTUInt32),
                                 &res);
    }

    if (AMDT_STATUS_OK != ret)
    {
        PwrTrace("PwrResumeProfiling failed: as IOCTL_RESUME_PROFILER failed");
    }

    g_powerProfileState = PowerProfiling;

    return ret;
}

// PwrGetProfilingState: Provides the current state of power profiling state machine
AMDTResult PwrGetProfilingState(
    /*out*/AMDTPwrProfilingState* pState)
{
    *pState = g_powerProfileState;
    return AMDT_STATUS_OK;
}

// PwrSetProfilingState: Allows to set the profile state
AMDTResult PwrSetProfilingState(
    /*in*/AMDTPwrProfilingState state)
{
    g_powerProfileState = state;
    return AMDT_STATUS_OK;
}

// PrepareApuPstateTable: Prepare the pstate table with harware/boosted pstates
// and their corresponding frequency.
void PrepareApuPstateTable()
{
    AMDTUInt32 boostedStates = 0;
    AMDTUInt64 regVal = 0;
    AMDTUInt32 cnt = 0;
    g_pStateInfo.m_cnt = AMDT_MAX_PSTATES;
    AMDTUInt32 reg = AMDT_PSTATE_BASE_REGISTER;

    // Get the number of boosted pstates from D18F4x15C
    ReadPciAddress(0, 0x18, 0x4, 0x15C, &boostedStates);

    // bits 4:2 are NumBoostStates
    boostedStates = ((boostedStates & 0x1C) >> 2);

    for (cnt = 0; cnt < AMDT_MAX_PSTATES; cnt++)
    {
        ACCESS_MSR msr;
        msr.isReadAccess = true;
        msr.regId = reg;
        AccessMSRAddress(&msr);
        regVal = msr.data;

        g_pStateInfo.m_stateInfo[cnt].m_frequency = (AMDTUInt32)(100.0 * (AMDTFloat64)((regVal & AMDT_CPUFID_MASK) + 0x10) /
                                                                 (AMDTFloat64)(0x1 << ((regVal & AMDT_CPUDID_MASK) >> AMDT_CPUDID_BITSHIFT)));

        if (cnt < boostedStates)
        {
            g_pStateInfo.m_stateInfo[cnt].m_isBoosted = true;
            g_pStateInfo.m_stateInfo[cnt].m_state = (AMDTApuPStates)(AMDT_PWR_PSTATE_PB0 + cnt);
        }
        else
        {
            g_pStateInfo.m_stateInfo[cnt].m_isBoosted = false;
            g_pStateInfo.m_stateInfo[cnt].m_state = (AMDTApuPStates)(AMDT_PWR_PSTATE_P0 + (cnt - boostedStates));
        }

        reg++;
    }

}

// GetApuPStateInfo: Function to access Apu pstate list information
AMDTPwrApuPstateList* GetApuPStateInfo()
{
    return &g_pStateInfo;
}

// check if NPU/CPU with NO SMU is supported
bool isNpuCpuPlatform(const AMDTUInt32& family, const AMDTUInt32& model)
{
    bool ret = false;

    for (PlatformInfo info : g_platformTable)
    {
        if (family == info.m_family)
        {
            if ((info.m_modelLow <= model) &&
                (model <= info.m_modelHigh))
            {
                ret = true;
            }
        }
    }

    return ret;
}

// AMDTPwrGetTargetSystemInfo: Hardware information of the target system where
// profiler was run
AMDTResult AMDTPwrGetTargetSystemInfo(
    /*in*/ AMDTPwrTargetSystemInfo* pSystemInfo)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    ACCESS_PCI pciData;
    AMDTUInt32 family = 0;
    AMDTUInt32 model = 0;
    bool isAmd = false;

    if (NULL == g_pTargetSystemInfo)
    {
        g_pTargetSystemInfo = (AMDTPwrTargetSystemInfo*) malloc(sizeof(AMDTPwrTargetSystemInfo));

        if (NULL == g_pTargetSystemInfo)
        {
            ret = AMDT_ERROR_OUTOFMEMORY;
        }

        if (AMDT_STATUS_OK == ret)
        {
            // Clear the structure first
            memset(g_pTargetSystemInfo, 0, sizeof(AMDTPwrTargetSystemInfo));

            // Get platform id
            g_pTargetSystemInfo->m_platformId = GetSupportedTargetPlatformId();
            ret = GetCpuFamilyDetails(&family, &model, &isAmd);
            g_pTargetSystemInfo->m_family = family;
            g_pTargetSystemInfo->m_model = model;
            g_pTargetSystemInfo->m_isAmd = isAmd;

            //Get core count
            g_pTargetSystemInfo->m_coreCnt = GetActiveCoreCount();
            g_pTargetSystemInfo->m_smuTable.m_count = 0;
            g_pTargetSystemInfo->m_isPlatformSupported = false;

            GetAvailableSmuList(&g_pTargetSystemInfo->m_smuTable);

            // Supported platform
            // APU+SMU , NPU and CPU
            if ((true == isAmd)
                && ((true == g_pTargetSystemInfo->m_isAmdApu)
                    || (true == isNpuCpuPlatform(family, model))))
            {
                g_pTargetSystemInfo->m_isPlatformSupported = true;
            }

            // If node platform is not supported and no valid dgpu connected
            if ((0 == g_pTargetSystemInfo->m_smuTable.m_count)
                && (false == g_pTargetSystemInfo->m_isPlatformSupported))
            {
                ret = AMDT_ERROR_PLATFORM_NOT_SUPPORTED;
            }
        }

        if ((AMDT_STATUS_OK == ret) && (true == isAmd))
        {
            if (NULL != g_pTargetSystemInfo->m_pNodeInfo)
            {
                // ONLY FOR SUPPORTED AMD platforms
                if (DEVICE_TYPE_APU == g_pTargetSystemInfo->m_pNodeInfo->m_deviceType)
                {
                    // Get compute unit count ONLY FOR SUPPORTED AMD APU
                    GET_EXTENED_PCICS_ADDRESS(0x0U, 0x18U, 0x5U, 0x80U, pciData.address);
                    pciData.isReadAccess = 1;
                    // Get PCI Access
                    ret = AccessPciAddress(&pciData);

                    if (AMDT_STATUS_OK == ret)
                    {
                        if (GDT_KALINDI == g_pTargetSystemInfo->m_pNodeInfo->m_hardwareType)
                        {
                            // Mullins has only one compute unit. Refer D1F5x80 Compute Unit Status 1
                            g_pTargetSystemInfo->m_computeUnitCnt = 1;
                            g_pTargetSystemInfo->m_coresPerCu = g_pTargetSystemInfo->m_coreCnt;
                        }
                        else
                        {
                            DecodeCURegisterStatus(pciData.data,
                                                   &g_pTargetSystemInfo->m_computeUnitCnt,
                                                   &g_pTargetSystemInfo->m_coresPerCu);
                        }

                        //Hard coding the remaing as of now
                        g_pTargetSystemInfo->m_igpuCnt = 1;
                        g_pTargetSystemInfo->m_svi2Cnt = 1;
                    }

                }

                // Fill the pState frequency table
                PrepareApuPstateTable();
            }
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        memcpy(pSystemInfo, g_pTargetSystemInfo, sizeof(AMDTPwrTargetSystemInfo));
    }

    return ret;
}

// Create memory pool: memory pool creation for interel use
AMDTResult CreateMemoryPool(MemoryPool* pPool, AMDTUInt32 size)
{
    AMDTResult ret = AMDT_ERROR_OUTOFMEMORY;

    if (NULL != pPool)
    {
        pPool->m_pBase = (AMDTUInt8*)malloc(size);

        if (NULL != pPool->m_pBase)
        {
            pPool->m_offset = 0;
            pPool->m_size = size;
            ret = AMDT_STATUS_OK;
        }
    }

    return ret;
}

// Get buffer from the pool pointer
AMDTUInt8* GetMemoryPoolBuffer(MemoryPool* pPool, AMDTUInt32 size)
{
    AMDTUInt8* pBuffer = NULL;

    if ((NULL == pPool) || ((pPool->m_offset + size) > pPool->m_size))
    {
        pBuffer = NULL;

        // Serious error -print the error message?
    }
    else
    {
        pBuffer = pPool->m_pBase + pPool->m_offset;
        pPool->m_offset += size;
    }

    return pBuffer;
}

// Delete the memory pool
AMDTResult ReleaseMemoryPool(MemoryPool* pPool)
{
    if (NULL != pPool)
    {
        if (NULL != pPool->m_pBase)
        {
            free(pPool->m_pBase);
            pPool->m_pBase = NULL;
            pPool->m_size = 0;
            pPool->m_offset = 0;
        }
    }

    return AMDT_STATUS_OK;
}

// GetAmdSupportedDevInfo: Get the dgpu information with the device id
bool GetAmdSupportedDevInfo(uint32 deviceId, PciDeviceInfo** pInfo)
{
    bool ret = false;
    AMDTUInt32 cnt = 0;
    AMDTUInt32 tableSize = sizeof(g_deviceTable) / sizeof(PciDeviceInfo);

    for (cnt = 0; cnt < tableSize; cnt++)
    {
        if (deviceId == g_deviceTable[cnt].m_deviceId)
        {
            *pInfo = &g_deviceTable[cnt];
            ret = true;
            break;
        }
    }

    return ret;
}

// GetAmdDeviceInfo: Get all AMD device information
bool GetAmdDeviceInfo(AMDTUInt32 bus,
                      AMDTUInt32 dev,
                      AMDTUInt32 func,
                      AMDTUInt32* pVendorId,
                      AMDTUInt32* pDeviceId)
{
    bool ret = false;
    ACCESS_PCI pci;
    GET_EXTENED_PCICS_ADDRESS(bus, dev, func, 0, pci.address);
    pci.isReadAccess = true;

    if (AMDT_STATUS_OK == AccessPciAddress(&pci))
    {
        *pVendorId = pci.data & 0xFFFF;
        *pDeviceId = pci.data >> 16;
        ret = true;
    }

    return ret;
}

// Smu8SRBMMsg: User spce SRBM message implementation for quick access
static bool Smu8SRBMMsg(SmuInfo* pSmu, uint32 msgId, uint32* argv, uint32* pData, uint32 retry)
{
    bool ret = false;
    uint32 repeatCnt = retry;
    uint64 msg = 0;
    uint64 resp = 0;
    uint32 data = 0;
    uint64 arg = 0;

    msg = pSmu->m_gpuBaseAddress + SMU8_SMC_MESSAGE_0;
    resp = pSmu->m_gpuBaseAddress + SMU8_SMC_RESPONSE_0;
    arg = pSmu->m_gpuBaseAddress + SMU8_SMC_MESSAGE_ARG_0;

    // 1.Driver clears SMU_MP1_SRBM2P_RESP_9 to 0. SMU firmware will write a non-zero response on message completion
    WriteMMIOSpace(resp, data);

    // 2.Driver optionally writes message arguments to SMU_MP1_SRBM2P_ARG_9.
    // SMU firmware may use the arguments for additional information depending on the message
    if (NULL != argv)
    {
        WriteMMIOSpace(arg, *argv);
    }

    // 3.Driver writes message ID to SMU_MP1_SRBM2P_MSG_9.
    WriteMMIOSpace(msg, msgId);

    // 4.Driver polls SMU_MP1_SRBM2P_RESP_9 until it reads back a non-zero response.
    do
    {
        ReadMMIOSpace(resp, &data);

        if (data & 0x01)
        {
            ret = true;
        }
    }
    while ((false == ret) && --repeatCnt);

    // 5.Driver optionally reads message arguments from SMU_MP1_SRBM2P_ARG_9. SMU firmware may
    // have updated this register depending on the message.
    if ((true == ret) && (NULL != pData))
    {
        ReadMMIOSpace(arg, pData);
    }

    return (true == ret) ? true : false;
}

// Smu7DriverMsg: User spce driver message implementation for quick access
bool Smu7DriverMsg(SmuInfo* pSmu, uint32 msgId)
{
    volatile uint32 cnt = 0;
    bool result = false;
    AMDTUInt32 data = 0;

    if (NULL != pSmu)
    {
        uint64 msg = pSmu->m_gpuBaseAddress + SMU7_SMC_MESSAGE_0;
        uint64 resp = pSmu->m_gpuBaseAddress + SMU7_SMC_RESPONSE_0;
        uint64 arg = pSmu->m_gpuBaseAddress + SMU7_SMC_MESSAGE_ARG_0;

        // MSG_RESP register should be non-zero before writing into MSG_ARG or MSG
        // register; If MSG_RESP is non-zero then SMU is ready for the next message.
        for (cnt = 0; cnt < SMU_RETRY_COUNT_MAX && (! result); ++cnt)
        {
            ReadMMIOSpace(resp, &data);

            if (0 != data)
            {
                result = true;
            }
        }

        if (result)
        {
            data = 0;
            WriteMMIOSpace(arg, data);

            // write the message into MSG register;
            WriteMMIOSpace(msg, msgId);

            ReadMMIOSpace(resp, &data);

            // wait for the opration to complete
            for (cnt = 0; cnt < SMU_RETRY_COUNT_MAX; ++cnt)
            {
                if (0 != data)
                {
                    break;
                }
            }

            ReadMMIOSpace(resp, &data);
            result = (1 == data) ? true : false;
        }
    }

    return result;
}

// IsDgpuMMIOAccessible: For AMD dGPU get the command register and
// check MMIO access bit is set
bool IsDgpuMMIOAccessible(AMDTUInt32 bus, AMDTUInt32 dev, AMDTUInt32 func)
{
    bool ret = false;
    ACCESS_PCI pci;
    GET_EXTENED_PCICS_ADDRESS(bus, dev, func, 4, pci.address);

    pci.isReadAccess = true;

    if (AMDT_STATUS_OK == AccessPciAddress(&pci))
    {
        ret = (pci.data & 0x2) ? true : false;
        PwrTrace("\nCommand reg 0x%x", pci.data);
    }

    return ret;
}

// IsSmuLogAccessible: Check if Smu longing for a particular device is successful
// This function will try for MAX_RETRY_COUNT time to access the smu logging
bool IsSmuLogAccessible(SmuInfo* pSmu, DeviceType devType)
{
    // SMU Message Commands used to START & SAMPLE the PM status logging
    AMDTUInt32 tabId = 0x01;
    bool ret = false;

    switch (pSmu->m_smuIpVersion)
    {
        case SMU_IPVERSION_8_0:
        case SMU_IPVERSION_8_1:
        {
            AMDTUInt32 tableBaseAddr = 0;
            // We can make sure smu is accessible by acquiring the AGM table base
            // address and release the table properly
            ret = Smu8SRBMMsg(pSmu,
                              SMU8_TESTSMC_MSG_RequestDataTable,
                              &tabId,
                              &tableBaseAddr,
                              SMU_RETRY_COUNT_MAX);
            Smu8SRBMMsg(pSmu,
                        0x40,
                        NULL,
                        &pSmu->m_access.m_smu8.m_tableVersion,
                        SMU_RETRY_COUNT_MAX);

            if (true == ret)
            {
                // Stop logging now
                ret = Smu8SRBMMsg(pSmu,
                                  SMU8_TESTSMC_MSG_ReleaseDataTable,
                                  NULL,
                                  NULL,
                                  SMU_RETRY_COUNT_MAX);

                if (false == ret)
                {
                    PwrTrace("IsSmuLogAccessible: SMU8_TESTSMC_MSG_ReleaseDataTable failed");
                }
            }
            else
            {
                PwrTrace("IsSmuLogAccessible: SMU8_TESTSMC_MSG_RequestDataTable failed");
            }

            break;
        }

        default:
        {
            if (DEVICE_TYPE_DGPU == devType)
            {
                ret = true;
            }
            else
            {
                AMDTUInt32 retry = SMU_LOG_MSG_RETRY_MAX;

                do
                {
                    // Initiate smu logging and stop loggin to check if smu is accessible
                    ret = Smu7DriverMsg(pSmu, SMU_PM_STATUS_LOG_START);

                    if (true == ret)
                    {
                        break;
                    }

                    SLEEP_IN_MS(50);
                }
                while (--retry);

                PwrTrace("Smu7DriverMsg: rety cnt %d", SMU_LOG_MSG_RETRY_MAX - retry);

                if (false == ret)
                {
                    PwrTrace("IsSmuLogAccessible: SMU_PM_STATUS_LOG_START failed");
                }
            }

            break;
        }
    }

    return ret;
}

// FillSmu7Details: Fill all register and offset details for smu7.
// Driver is open source. So we can not keep this information in driver
void FillSmu7Details(Smu7Interface* pSmu7Interface, PciDeviceInfo* pDev)
{
    Smu7CounterTable table =
    {
        SMU7_CU0_PWR,
        SMU7_CU1_PWR,
        SMU7_CU0_CALC_TEMP,
        SMU7_CU1_CALC_TEMP,
        SMU7_CU0_MEAS_TEMP,
        SMU7_CU1_MEAS_TEMP,
        SMU7_GPU_PWR,
        SMU7_PCIE_PWR,
        SMU7_DDR_PWR,
        SMU7_DISPLAY_PWR,
        SMU7_PACKAGE_PWR,
        SMU7_GPU_CALC_TEMP,
        SMU7_GPU_MEAS_TEMP,
        SMU7_SCLK,
        SMU7_VOLT_VDDC_LOAD,
        SMU7_CURR_VDDC
    };
    Smu7MessageParam msg = {SMU7_SMC_MESSAGE_0, SMU7_SMC_RESPONSE_0, SMU7_SMC_MESSAGE_ARG_0, 0};
    Smu7Logging logging = {SMU_PM_STATUS_LOG_START, SMU_PM_STATUS_LOG_SAMPLE};
    Smu7IndexData indexData = {SMC_IND_INDEX_2, SMC_IND_DATA_2};

    if (nullptr != pSmu7Interface)
    {
        // Consider calculated voltage & current in case of Bonaire
        if ((nullptr != pDev) && (GDT_BONAIRE == pDev->m_hardwareType))
        {
            table.m_voltVddcLoad = SMU7_VOLT_VDDC_LOAD_CALC;
            table.m_currVddc = SMU7_CURR_VDDC_CALC;
        }

        memcpy(&pSmu7Interface->m_counterTable, &table, sizeof(Smu7CounterTable));
        memcpy(&pSmu7Interface->m_messageParam, &msg, sizeof(Smu7MessageParam));
        memcpy(&pSmu7Interface->m_logging, &logging, sizeof(Smu7Logging));
        memcpy(&pSmu7Interface->m_indexData, &indexData, sizeof(Smu7IndexData));
    }
}

// FillSmu8Details:  Fill all register and offset details for smu8.
// Driver is open source. So we can not keep this information in driver
bool FillSmu8Details(Smu8Interface* pSmu)
{
    bool result = false;
    Smu8SRBMMessage srbmMsgTable = { SMU8_SMC_MESSAGE_0 - MAPPED_BASE_OFFSET,
                                     SMU8_SMC_MESSAGE_ARG_0 - MAPPED_BASE_OFFSET,
                                     SMU8_SMC_RESPONSE_0 - MAPPED_BASE_OFFSET,
                                     SMU8_TESTSMC_MSG_RequestDataTable,
                                     SMU8_TESTSMC_MSG_ReleaseDataTable,
    {0x00}
                                   };

    Smu8MMAccess memAccessTable = {MAPPED_BASE_OFFSET - MAPPED_BASE_OFFSET,
                                   0x60C - MAPPED_BASE_OFFSET
                                  };

    if (NULL != pSmu)
    {
        // Reset the information
        pSmu->m_tableId = 0x01;
        memcpy(&pSmu->m_srbmMsg, &srbmMsgTable, sizeof(Smu8SRBMMessage));
        memcpy(&pSmu->m_gmmxPair, &memAccessTable, sizeof(Smu8MMAccess));

        result = true;
    }

    return result;
}

// GetAvailableSmuList: Prepare SMU lists for the all hardware including APU
// and dGPU connected to the platform
bool GetAvailableSmuList(SmuList* pList)
{
    bool ret = false;
    AMDTUInt32 devCnt = 0;
    AMDTUInt32 busCnt = 0;
    AMDTUInt32 funcCnt = 0;
    uint32 smuCnt = 0;
    uint32 dgpuCnt = APU_SMU_ID + 1;

    if (NULL != pList && (0 == pList->m_count))
    {
        // Clear the dgpu short name table
        memset(&g_activePciDevice[0], 0, sizeof(g_activePciDevice));
        memset(&g_activePciPortList[0], 0, sizeof(g_activePciPortList));
        g_pTargetSystemInfo->m_pNodeInfo = NULL;

        for (busCnt = 0; (pList->m_count < PLATFORM_SMU_CNT) && busCnt < MAX_BUS_CNT; busCnt++)
        {
            for (devCnt = 0; (pList->m_count < PLATFORM_SMU_CNT) && devCnt < MAX_PCIE_DEVICE_CNT; devCnt++)
            {
                for (funcCnt = 0; (pList->m_count < PLATFORM_SMU_CNT) && funcCnt < MAX_FUNCTION_CNT; funcCnt++)
                {
                    uint32 vendorId = 0;
                    uint32 deviceId = 0;

                    if (true == GetAmdDeviceInfo(busCnt, devCnt, funcCnt, &vendorId, &deviceId))
                    {
                        if (0xFFFFFFFF == (vendorId | deviceId << 15))
                        {
                            break;
                        }

                        if (AMD_VENDOR_ID == vendorId)
                        {
                            PciDeviceInfo* pDevInfo = NULL;

                            if (true == GetAmdSupportedDevInfo(deviceId, &pDevInfo)
                                && (pList->m_count < PLATFORM_SMU_CNT))
                            {
                                bool smuAccess = false;
                                bool mmioAccess = true;
                                bool apuSmuFeature = true;

                                if ((DEVICE_TYPE_APU == pDevInfo->m_deviceType) || (DEVICE_TYPE_DGPU == pDevInfo->m_deviceType))
                                {
                                    // Fill only if this is an SMU
                                    AMDTUInt32 res = 0x00;
                                    // AMDTUInt32 pkgId = dgpuCnt++;
                                    pList->m_info[smuCnt].m_packageId = (DEVICE_TYPE_APU == pDevInfo->m_deviceType) ? APU_SMU_ID : dgpuCnt++;
                                    pList->m_info[smuCnt].m_smuIpVersion = pDevInfo->m_smuIpVersion;
                                    ReadPciAddress(busCnt, devCnt, funcCnt, 0x24, &res);
                                    pList->m_info[smuCnt].m_gpuBaseAddress = res & 0xFFFFFFF0;
                                    pList->m_count += 1;

                                    // Fill the device information
                                    memcpy(&g_activePciDevice[pList->m_info[smuCnt].m_packageId], pDevInfo, sizeof(PciDeviceInfo));
                                    g_activePciPortList[pList->m_info[smuCnt].m_packageId].m_bus = busCnt;
                                    g_activePciPortList[pList->m_info[smuCnt].m_packageId].m_dev = devCnt;
                                    g_activePciPortList[pList->m_info[smuCnt].m_packageId].m_func = funcCnt;

                                    // Fill SMU values
                                    if ((SMU_IPVERSION_7_0 == pDevInfo->m_smuIpVersion)
                                        || (SMU_IPVERSION_7_1 == pDevInfo->m_smuIpVersion)
                                        || (SMU_IPVERSION_7_5 == pDevInfo->m_smuIpVersion)
                                       )
                                    {
                                        FillSmu7Details(&pList->m_info[smuCnt].m_access.m_smu7, pDevInfo);
                                    }
                                    else if ((SMU_IPVERSION_8_0 == pDevInfo->m_smuIpVersion)
                                             || (SMU_IPVERSION_8_1 == pDevInfo->m_smuIpVersion))
                                    {
                                        FillSmu8Details(&pList->m_info[smuCnt].m_access.m_smu8);
                                    }

                                    if (DEVICE_TYPE_APU == pDevInfo->m_deviceType)
                                    {
                                        g_pTargetSystemInfo->m_isAmdApu = true;

                                        // Check if it is KV, if so, check if BAPM and smu feature is enable
                                        if ((GDT_SPECTRE == pDevInfo->m_hardwareType)
                                            || (GDT_SPECTRE_SL == pDevInfo->m_hardwareType)
                                            || (GDT_SPOOKY == pDevInfo->m_hardwareType)
                                            || (GDT_SPECTRE_LITE == pDevInfo->m_hardwareType))
                                        {
                                            apuSmuFeature = IsSMUFeatureEnabled();

                                            if (false == apuSmuFeature)
                                            {
                                                apuSmuFeature = EnableSmu(true);
                                                PwrTrace("Trying to actiavte SMU status:%s", apuSmuFeature ? "Success" : "Fail");

                                                if (apuSmuFeature)
                                                {
                                                    g_smuRestore = true;
                                                }
                                            }
                                        }
                                    }

                                    // If dGPU first check MMIO is accessible
                                    if (DEVICE_TYPE_DGPU == pDevInfo->m_deviceType)
                                    {
                                        mmioAccess = IsDgpuMMIOAccessible(busCnt, devCnt, funcCnt);
                                        PwrTrace("dGPU mmio access %d", mmioAccess);
                                    }

                                    if ((true == mmioAccess) && (true == apuSmuFeature))
                                    {
                                        smuAccess = IsSmuLogAccessible(&pList->m_info[smuCnt], pDevInfo->m_deviceType);
                                    }

                                    pList->m_info[smuCnt].m_isAccessible = smuAccess;
                                    smuCnt++;
                                }

                                if ((DEVICE_TYPE_APU == pDevInfo->m_deviceType)
                                    || (DEVICE_TYPE_CPU_NO_SMU == pDevInfo->m_deviceType)
                                    || (DEVICE_TYPE_NPU_NO_SMU == pDevInfo->m_deviceType))
                                {
                                    g_pTargetSystemInfo->m_pNodeInfo = pDevInfo;
                                }

                                PwrTrace(DEVICE_TRACE_STR,
                                         busCnt,
                                         devCnt,
                                         funcCnt,
                                         pDevInfo->m_deviceId,
                                         pDevInfo->m_hardwareType,
                                         pDevInfo->m_deviceType,
                                         pDevInfo->m_modelName,
                                         pDevInfo->m_shortName,
                                         pDevInfo->m_smuIpVersion,
                                         smuAccess);
                            }
                            else
                            {
                                PwrTrace("SMU not supported for device Id B%dD%df%d: 0x%x ",
                                         busCnt, devCnt, funcCnt, deviceId);
                            }
                        }
                    }
                }
            }
        }
    }

    ret = ((NULL != g_pTargetSystemInfo->m_pNodeInfo) || (0 < pList->m_count)) ? true : false;
    return ret;
}

// GetPcieDeviceInfo: Get the short name of each device and plaform
// connected
void GetPciDeviceInfo(AMDTUInt32 pkgId, PciDeviceInfo** ppDevInfo, PciPortAddress** ppAddress)
{
    if (nullptr != ppDevInfo)
    {
        *ppDevInfo = &g_activePciDevice[pkgId];
    }

    if (nullptr != ppAddress)
    {
        *ppAddress = &g_activePciPortList[pkgId];
    }
}



