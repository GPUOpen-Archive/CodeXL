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
CounterMap g_basicCounterList[PLATFORM_SMU_CNT];
PwrSupportedCounterMap g_supportedCounterMap;

fpFillSmuInternal g_fpFillSmuInternal = nullptr;
fpGetCountersInternal g_fpGetSmuCountersInternal = nullptr;

bool GetAvailableSmuList(SmuList* pList);

// Basic Counters- Sample id, record-id, timestamp
AMDTPwrCounterBasicInfo g_basicCounters[] =
{
#include "BasicCounterList.h"
};

// Node specific Counters- PID, TID, pstate, cef, node temperature
AMDTPwrCounterBasicInfo g_nodeCounters[] =
{
#include "NodeCounterList.h"
};

// Smu7.0- Kaveri
AMDTPwrCounterBasicInfo g_smu7Counters[] =
{
#include "Smu7CounterList.h"
};

// Smu7.5 -Mullins
AMDTPwrCounterBasicInfo g_smu75Counters[] =
{
#include "Smu75CounterList.h"
};

// Smu8 - Carrizo
AMDTPwrCounterBasicInfo g_smu8Counters[] =
{
#include "Smu8CounterList.h"
};

AMDTPwrCounterBasicInfo g_CounterDgpuSmu7_0[] =
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


// Zeppelin Node specific Counters- PID, TID, pstate, cef, node temperature
AMDTPwrCounterBasicInfo g_zeppelinNodeCounters[] =
{
#include "ZeppelinNodeCounterList.h"
};

// PwrGetZeppelinCounters: Get Smu9 counters
AMDTUInt32 PwrGetZeppelinCounters(AMDTPwrCounterBasicInfo** pCouters)
{
    *pCouters = g_zeppelinNodeCounters;
    return sizeof(g_zeppelinNodeCounters) / sizeof(AMDTPwrCounterBasicInfo);
}

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

    g_supportedCounterMap.clear();
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
        g_attributeList.pAttrList = (AMDTPwrCounterBasicInfo*) GetMemoryPoolBuffer(&g_controlMemoryPool, COUNTERID_MAX_CNT * sizeof(AMDTPwrCounterBasicInfo));

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

    HardwareType type = sysInfo.m_nodeInfo.m_hardwareType;

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

    return ret;
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
AMDTResult PwrSetProfileConfiguration(ProfileConfig* pConfig, AMDTUInt32 clientId)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTInt32 res = 0;
    PROF_CONFIGS cfgList;
    memset(&cfgList, 0, sizeof(PROF_CONFIGS));

    if (nullptr == pConfig)
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
        AMDTUInt32 processID = PwrGetProcessId();
        pConfig->m_fill = processID;
        //Prepare the list for the driver
        cfgList.ulStatus = 0;
        cfgList.ulConfigCnt = 1;
        cfgList.uliProfileConfigs = reinterpret_cast<uint64>(pConfig);
        cfgList.ulClientId = clientId;

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
        PwrEnableSmu(false);
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
            PwrEnableInternalCounters(false);
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
                                                                 (AMDTFloat64)(0x1ULL << ((regVal & AMDT_CPUDID_MASK) >> AMDT_CPUDID_BITSHIFT)));

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

            if (AMDT_STATUS_OK == ret)
            {
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
                    && ((true == g_pTargetSystemInfo->m_isPlatformWithSmu)
                        || (true == isNpuCpuPlatform(family, model))))
                {
                    g_pTargetSystemInfo->m_isPlatformSupported = true;
                }
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
            // ONLY FOR SUPPORTED AMD platforms
            if (DEVICE_TYPE_APU == g_pTargetSystemInfo->m_nodeInfo.m_deviceType)
            {
                // Get compute unit count ONLY FOR SUPPORTED AMD APU
                GET_EXTENED_PCICS_ADDRESS(0x0U, 0x18U, 0x5U, 0x80U, pciData.address);
                pciData.isReadAccess = 1;
                // Get PCI Access
                ret = AccessPciAddress(&pciData);

                if (AMDT_STATUS_OK == ret)
                {
                    if (g_pTargetSystemInfo->m_coreCnt != PwrGetCoreCntFromOS())
                    {
                        g_pTargetSystemInfo->m_computeUnitCnt = 1;
                        g_pTargetSystemInfo->m_coresPerCu = 1;
                        g_pTargetSystemInfo->m_coreCnt = 1;
                    }
                    else if (GDT_KALINDI == g_pTargetSystemInfo->m_nodeInfo.m_hardwareType)
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

            if ((PLATFORM_ZEPPELIN != g_pTargetSystemInfo->m_platformId)
                && (PLATFORM_INVALID != g_pTargetSystemInfo->m_platformId))
            {
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
    // un-used variable
    (void)devType;

    // SMU Message Commands used to START & SAMPLE the PM status logging
    AMDTUInt32 tabId = 0x01;
    bool ret = false;

    switch (pSmu->m_smuIpVersion)
    {

        case SMU_IPVERSION_9_0:
            break;

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

            // User must need to run any OpenCL program if dGPU is in BACO state.
            // Otherwise, counters may not be availables
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
        AMDTUInt32 cnt = 0;
        AMDTUInt32 platformCnt = 0;
        // Clear the dgpu short name table
        memset(&g_activePciDevice[0], 0, sizeof(g_activePciDevice));
        memset(&g_activePciPortList[0], 0, sizeof(g_activePciPortList));
        memset(&g_pTargetSystemInfo->m_nodeInfo, 0, sizeof(PlatformInfo));
        g_pTargetSystemInfo->m_isSmtEnabled = false;
        g_pTargetSystemInfo->m_threadCount = 0;
        g_pTargetSystemInfo->m_isPlatformWithSmu = false;

        platformCnt = sizeof(g_platformTable) / sizeof(PlatformInfo);

        for (cnt = 0; cnt < platformCnt; cnt++)
        {
            AMDTUInt32 platformId = (g_platformTable[cnt].m_family << 16)
                                    | (g_platformTable[cnt].m_modelHigh << 8)
                                    | (g_platformTable[cnt].m_modelLow);

            if (platformId == g_pTargetSystemInfo->m_platformId)
            {
                bool smuAccessible = false;
                g_pTargetSystemInfo->m_isPlatformWithSmu = true;
                g_pTargetSystemInfo->m_isPlatformSupported = true;

                if (PLATFORM_ZEPPELIN == GetSupportedTargetPlatformId())
                {
                    g_pTargetSystemInfo->m_threadCount = PwrGetLogicalProcessCount();
                    g_pTargetSystemInfo->m_isSmtEnabled = PwrIsSmtEnabled();
                }

                if (PLATFORM_ZEPPELIN == platformId)
                {
                    pList->m_info[smuCnt].m_smuIpVersion = SMU_IPVERSION_9_0;

                    if (nullptr != g_fpFillSmuInternal)
                    {
                        g_fpFillSmuInternal(&pList->m_info[smuCnt].m_access);
		                smuAccessible = true;
                    }
                }

                memcpy(&g_pTargetSystemInfo->m_nodeInfo, &g_platformTable[cnt], sizeof(PlatformInfo));

                if (smuAccessible)
                {
                    pList->m_info[smuCnt].m_packageId = 1;
                    pList->m_info[smuCnt].m_isAccessible = true;
                    pList->m_info[smuCnt].m_gpuBaseAddress = 0xFFFFFFF0;
                    pList->m_count += 1;
                    smuCnt++;
                }
            }
        }

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
                                        g_pTargetSystemInfo->m_isPlatformWithSmu = true;

                                        // Check if it is KV, if so, check if BAPM and smu feature is enable
                                        if ((GDT_SPECTRE == pDevInfo->m_hardwareType)
                                            || (GDT_SPECTRE_SL == pDevInfo->m_hardwareType)
                                            || (GDT_SPOOKY == pDevInfo->m_hardwareType)
                                            || (GDT_SPECTRE_LITE == pDevInfo->m_hardwareType))
                                        {
                                            apuSmuFeature = IsSMUFeatureEnabled();
                                            PwrTrace("BAPM Status %s", apuSmuFeature ? "ON" : "Fail");

                                            if (false == apuSmuFeature)
                                            {
                                                apuSmuFeature = PwrEnableSmu(true);
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
                                    || (DEVICE_TYPE_CPU_WITH_SMU == pDevInfo->m_deviceType)
                                    || (DEVICE_TYPE_NPU_NO_SMU == pDevInfo->m_deviceType))
                                {
                                    PlatformInfo* pInfo = &g_pTargetSystemInfo->m_nodeInfo;
                                    memcpy(pInfo->m_name, pDevInfo->m_modelName, PWR_MAX_NAME_LEN);
                                    memcpy(pInfo->m_shortName, pDevInfo->m_shortName, PWR_MAX_NAME_LEN);
                                    pInfo->m_family = g_pTargetSystemInfo->m_family;
                                    pInfo->m_modelHigh = (g_pTargetSystemInfo->m_platformId && 0xFF00) >> 8;
                                    pInfo->m_modelHigh = g_pTargetSystemInfo->m_platformId && 0xFF;
                                    pInfo->m_deviceType = pDevInfo->m_deviceType;
                                    pInfo->m_hardwareType = pDevInfo->m_hardwareType;
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
                        }
                    }
                }
            }
        }
    }

    ret = (0 < pList->m_count) ? true : false;
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

void PwrFillSupportedCounters(AMDTPwrCounterBasicInfo* pCounter,
                              AMDTUInt32 count,
                              AMDTPwrTargetSystemInfo* pSys,
                              AMDTUInt32 pkgId,
                              AMDTUInt32* pClientId,
                              AMDTUInt32 packageId)
{
    AMDTUInt32 idx = 0;
    CounterMap* pCounterList = nullptr;
    AMDTUInt32 clientId = *pClientId;
    bool internalSupported = false;

    pCounterList = &g_basicCounterList[pkgId];
    AMDTPwrTargetSystemInfo sysInfo;
    AMDTPwrGetTargetSystemInfo(&sysInfo);
    AMDTUInt32 instanceId = 0;

    for (idx = 0; idx < count; idx++)
    {
        CounterInstance inst;
        AMDTUInt32 repeat = 1;
        AMDTUInt32 loop = 0;
        AMDTPwrCounterBasicInfo* pInfo = pCounter + idx;

        if (INSTANCE_TYPE_PER_CU == pCounter[idx].m_instanceType)
        {
            repeat = pSys->m_computeUnitCnt;
        }
        else if ((INSTANCE_TYPE_PER_CORE == pCounter[idx].m_instanceType)
                 || (INSTANCE_TYPE_SMU_PER_CORE == pCounter[idx].m_instanceType))
        {
            repeat = pSys->m_coreCnt;
        }
        else if (INSTANCE_TYPE_PER_PHYSICAL_CORE == pCounter[idx].m_instanceType)
        {
            if (PwrIsSmtEnabled())
            {
                repeat = pSys->m_coreCnt / 2;
            }
            else
            {
                repeat = pSys->m_coreCnt;
            }
        }

        inst.m_instanceId = loop;

        // Filter Stoney VDDGFX counter as it is not available
        bool isVddGfxNotSupported = (sysInfo.m_isPlatformWithSmu
                                     && (SMU_IPVERSION_8_1 == sysInfo.m_smuTable.m_info[0].m_smuIpVersion)
                                     && (COUNTERID_SMU8_APU_PWR_VDDGFX == pInfo->m_attrId));

        if (isVddGfxNotSupported)
        {
            continue;
        }

        // Filter Measured/calculated temperature
        if ((PLATFORM_KAVERI == sysInfo.m_platformId)
            || (PLATFORM_MULLINS == sysInfo.m_platformId))
        {
            if (APU_SMU_ID == packageId)
            {
                // Filter Measured / Calculated counters
                if (PLATFORM_KAVERI == sysInfo.m_platformId)
                {
                    if ((COUNTERID_SMU7_APU_TEMP_MEAS_CU == pInfo->m_attrId)
                        || (COUNTERID_SMU7_APU_TEMP_MEAS_IGPU == pInfo->m_attrId))
                    {
                        continue;
                    }
                }
                else
                {
                    if ((COUNTERID_SMU7_APU_TEMP_CU == pInfo->m_attrId)
                        || (COUNTERID_SMU7_APU_TEMP_IGPU == pInfo->m_attrId))
                    {
                        continue;
                    }
                }
            }
        }

        // Filter SVI2/CSTATE counters
        if (0 == packageId)
        {
            // Filter internal counters
            if (internalSupported)
            {
                if (!PwrIsSVISupported(sysInfo) && ((COUNTERID_SVI2_CORE_TELEMETRY == pInfo->m_attrId)
                                                    || (COUNTERID_SVI2_NB_TELEMETRY == pInfo->m_attrId)))
                {
                    continue;
                }
            }
            else
            {
                if ((COUNTERID_SVI2_CORE_TELEMETRY == pInfo->m_attrId)
                    || (COUNTERID_SVI2_NB_TELEMETRY == pInfo->m_attrId)
                    || (COUNTERID_CSTATE_RES == pInfo->m_attrId))
                {
                    continue;
                }
            }
        }

        instanceId = 0;

        for (loop = 0; loop < repeat; loop++)
        {
            memcpy(&inst.m_counter, pInfo, sizeof(AMDTPwrCounterBasicInfo));
            inst.m_instanceId = instanceId++;
            inst.m_packageId = packageId;
            inst.m_counter.m_aggr = AMDT_PWR_VALUE_SINGLE;
            pCounterList->insert(CounterMap::value_type(clientId++, inst));

            if ((1 << AMDT_PWR_VALUE_HISTOGRAM) & pInfo->m_aggr)
            {
                char str[PWR_MAX_DESC_LEN];
                memcpy(str, inst.m_counter.m_name, PWR_MAX_NAME_LEN);
                sprintf(inst.m_counter.m_name, "Histogram-%s", str);
                memcpy(str, inst.m_counter.m_description, PWR_MAX_DESC_LEN);
                sprintf(inst.m_counter.m_description, "Histogram-%s", str);
                inst.m_counter.m_aggr = AMDT_PWR_VALUE_HISTOGRAM;
                pCounterList->insert(CounterMap::value_type(clientId++, inst));
            }
            else if ((1 << AMDT_PWR_VALUE_CUMULATIVE) & pInfo->m_aggr)
            {
                char str[PWR_MAX_DESC_LEN];
                memcpy(str, inst.m_counter.m_name, PWR_MAX_NAME_LEN);
                sprintf(inst.m_counter.m_name, "Cummulative-%s", str);
                memcpy(str, inst.m_counter.m_description, PWR_MAX_DESC_LEN);
                sprintf(inst.m_counter.m_description, "Cummulative-%s", str);
                inst.m_counter.m_aggr = AMDT_PWR_VALUE_CUMULATIVE;
                pCounterList->insert(CounterMap::value_type(clientId++, inst));
            }

            if ((INSTANCE_TYPE_PER_PHYSICAL_CORE == pInfo->m_instanceType) && PwrIsSmtEnabled())
            {
                instanceId++;
            }

        }

    }

    *pClientId = clientId;
}

// PwrGetSupportedCounters: This function prepares the list of supported counters
// including APU and dGPU which are connected to the platform
// This function needs to be modified whenever there is a new addion of SMU version
// for APU and dGPU
AMDTResult PwrGetSupportedCounters(CounterMap** pList)

{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrTargetSystemInfo sysInfo;
    AMDTUInt32 smuCnt = 0;
    AMDTUInt32 counterCnt = 0;
    AMDTUInt32 clientId = 0;
    bool listEmpty = true;

    ret = AMDTPwrGetTargetSystemInfo(&sysInfo);

    for (smuCnt = 0; smuCnt <= sysInfo.m_smuTable.m_count; smuCnt++)
    {
        if (g_basicCounterList[smuCnt].size() > 0)
        {
            listEmpty = false;
            break;
        }
    }

    if (true == listEmpty)
    {
        AMDTUInt32 dGpuPackageId = APU_SMU_ID + 1;

        for (smuCnt = 0; smuCnt < sysInfo.m_smuTable.m_count; smuCnt++)
        {
            SmuInfo* smuInfo = &sysInfo.m_smuTable.m_info[smuCnt];

            if (APU_SMU_ID == smuInfo->m_packageId)
            {
                if ((AMDT_STATUS_OK == ret) && (true == sysInfo.m_isPlatformSupported))
                {
                    if ((true == sysInfo.m_isPlatformWithSmu)
                        && (sysInfo.m_smuTable.m_info[0].m_isAccessible))
                    {
                        AMDTUInt32 ipVersion = sysInfo.m_smuTable.m_info[0].m_smuIpVersion;

                        if (SMU_IPVERSION_7_0 == ipVersion)

                        {
                            counterCnt = sizeof(g_smu7Counters) / sizeof(AMDTPwrCounterBasicInfo);
                            PwrFillSupportedCounters(g_smu7Counters, counterCnt, &sysInfo, smuInfo->m_packageId, &clientId, APU_SMU_ID);
                        }
                        else if (SMU_IPVERSION_7_5 == ipVersion)
                        {
                            counterCnt = sizeof(g_smu75Counters) / sizeof(AMDTPwrCounterBasicInfo);

                            PwrFillSupportedCounters(g_smu75Counters, counterCnt, &sysInfo, smuInfo->m_packageId, &clientId, APU_SMU_ID);

                        }
                        else if ((SMU_IPVERSION_8_0 == ipVersion)
                                 || (SMU_IPVERSION_8_1 == ipVersion))
                        {
                            counterCnt = sizeof(g_smu8Counters) / sizeof(AMDTPwrCounterBasicInfo);
                            PwrFillSupportedCounters(g_smu8Counters, counterCnt, &sysInfo, smuInfo->m_packageId, &clientId, APU_SMU_ID);
                        }
                        else if (SMU_IPVERSION_9_0 == ipVersion)
                        {
                            if (nullptr != g_fpGetSmuCountersInternal)
                            {
                                AMDTPwrCounterBasicInfo* pCounters = nullptr;
                                counterCnt = g_fpGetSmuCountersInternal(&pCounters);;
                                PwrFillSupportedCounters(pCounters, counterCnt, &sysInfo, smuInfo->m_packageId, &clientId, APU_SMU_ID);
                            }
                        }
                    }
                }
            }
            else
            {
                if ((SMU_IPVERSION_7_0 == smuInfo->m_smuIpVersion)
                    || (SMU_IPVERSION_7_1 == smuInfo->m_smuIpVersion)
                    || (SMU_IPVERSION_7_2 == smuInfo->m_smuIpVersion))
                {
                    if (sysInfo.m_smuTable.m_info[smuCnt].m_isAccessible)
                    {
                        counterCnt = sizeof(g_CounterDgpuSmu7_0) / sizeof(AMDTPwrCounterBasicInfo);
                        PwrFillSupportedCounters(g_CounterDgpuSmu7_0,
                                                 counterCnt,
                                                 &sysInfo,
                                                 smuInfo->m_packageId,
                                                 &clientId,
                                                 dGpuPackageId++);
                    }
                }
            }
        }

        if ((PLATFORM_INVALID != sysInfo.m_platformId) && (sysInfo.m_isAmd))
        {
            if (PLATFORM_ZEPPELIN == sysInfo.m_platformId)
            {
                AMDTPwrCounterBasicInfo* pCounters = nullptr;
                counterCnt = PwrGetZeppelinCounters(&pCounters);;
                PwrFillSupportedCounters(pCounters, counterCnt, &sysInfo, 0, &clientId, 0);
            }
            else
            {
                counterCnt = sizeof(g_nodeCounters) / sizeof(AMDTPwrCounterBasicInfo);
                PwrFillSupportedCounters(g_nodeCounters, counterCnt, &sysInfo, 0, &clientId, 0);
            }
        }
    }

    *pList = g_basicCounterList;
    return ret;
}


// InsertDeviceCounters: Create counters for a device
void PwrInsertDeviceCounters(AMDTPwrDevice* dev, AMDTUInt32 instId, AMDTUInt32 listId)
{
    AMDTUInt32 listIdx = 0;
    CounterMap* pBasicList;
    AMDTUInt32 loop = (0 == listId) ? (listId + 1) : listId;
    PwrGetSupportedCounters(&pBasicList);

    for (listIdx = listId; listIdx <= loop; listIdx++)
    {
        if (pBasicList[listIdx].size() > 0)
        {
            for (auto iter : pBasicList[listIdx])
            {
                AMDTPwrCounterBasicInfo* pInfo = &iter.second.m_counter;

                if ((nullptr != pInfo) && (pInfo->m_deviceType == dev->m_type) && (iter.second.m_instanceId == instId))
                {
                    PwrCounterInfo counter;
                    memset(&counter, 0, sizeof(PwrCounterInfo));
                    counter.m_instanceId = instId;
                    counter.m_pkgId = iter.second.m_packageId;
                    counter.m_isActive = false;
                    counter.m_instanceType = pInfo->m_instanceType;
                    counter.m_desc.m_counterID = iter.first;
                    counter.m_desc.m_deviceId = dev->m_deviceID;
                    counter.m_desc.m_category = (AMDTPwrCategory)pInfo->m_category;
                    counter.m_desc.m_units = (AMDTPwrUnit)pInfo->m_unitType;
                    counter.m_desc.m_parentCounterId = pInfo->m_parentCounterId;
                    counter.m_desc.m_aggregation = (AMDTPwrAggregation)pInfo->m_aggr;
                    counter.m_desc.m_name = (char*)GetMemoryPoolBuffer(&g_controlMemoryPool,
                                                                       sizeof(char) * PWR_MAX_NAME_LEN);
                    counter.m_desc.m_description = (char*)GetMemoryPoolBuffer(&g_controlMemoryPool,
                                                                              sizeof(char) * PWR_MAX_DESC_LEN);
                    memset(counter.m_desc.m_name, '\0', sizeof(char)* PWR_MAX_DESC_LEN);
                    memset(counter.m_desc.m_description, '\0', sizeof(char)* PWR_MAX_NAME_LEN);


                    // GUI counters are based on name string
                    // GUI needs to be changed before we change the names

                    if ((AMDT_PWR_DEVICE_PACKAGE == dev->m_deviceID)
                        || (AMDT_PWR_DEVICE_INTERNAL_GPU == dev->m_type))
                    {
                        sprintf(counter.m_desc.m_name, "%s", pInfo->m_name);
                        sprintf(counter.m_desc.m_description, "%s-%s", dev->m_pDescription, pInfo->m_description);
                    }
                    else if (strlen(dev->m_pName) > 0)
                    {
                        if (PLATFORM_ZEPPELIN == GetSupportedTargetPlatformId())
                        {
                            AMDTUInt32 inst = counter.m_instanceId;

                            if ((INSTANCE_TYPE_PER_PHYSICAL_CORE == counter.m_instanceType) && PwrIsSmtEnabled())
                            {
                                inst = counter.m_instanceId / 2;
                            }

                            if (INSTANCE_TYPE_PER_CORE == counter.m_instanceType)
                            {
                                if (PwrIsSmtEnabled())
                                {
                                    sprintf(counter.m_desc.m_description, "Logical core%d-%s", inst, pInfo->m_description);
                                    sprintf(counter.m_desc.m_name, "Logical core%d %s", inst, pInfo->m_name);
                                }
                                else
                                {
                                    sprintf(counter.m_desc.m_description, "Core%d-%s", inst, pInfo->m_description);
                                    sprintf(counter.m_desc.m_name, "Core%d %s", inst, pInfo->m_name);
                                }
                            }
                            else if (INSTANCE_TYPE_PER_PHYSICAL_CORE == counter.m_instanceType)
                            {
                                sprintf(counter.m_desc.m_description, "Core%d-%s", inst, pInfo->m_description);
                                sprintf(counter.m_desc.m_name, "Core%d %s", inst, pInfo->m_name);
                            }
                            else
                            {
                                sprintf(counter.m_desc.m_description, "%s%d-%s", dev->m_pName, inst, pInfo->m_description);
                                sprintf(counter.m_desc.m_name, "%s%d %s", dev->m_pName, inst, pInfo->m_name);
                            }
                        }
                        else
                        {
                            sprintf(counter.m_desc.m_description, "%s-%s", dev->m_pName, pInfo->m_description);
                            sprintf(counter.m_desc.m_name, "%s %s", dev->m_pName, pInfo->m_name);
                        }
                    }


                    memcpy(&counter.m_basicInfo, pInfo, sizeof(AMDTPwrCounterBasicInfo));
                    g_supportedCounterMap.insert(PwrSupportedCounterMap::value_type(iter.first, counter));
                }
            }
        }
    }
}

void PwrActivateCounter(AMDTUInt32 counterId, bool isActivate)
{
    PwrSupportedCounterMap:: iterator iter = g_supportedCounterMap.find(counterId);

    if (iter != g_supportedCounterMap.end())
    {
        iter->second.m_isActive = isActivate;
    }

}
PwrSupportedCounterMap* PwrGetSupportedCounterList()
{
    return &g_supportedCounterMap;
}



