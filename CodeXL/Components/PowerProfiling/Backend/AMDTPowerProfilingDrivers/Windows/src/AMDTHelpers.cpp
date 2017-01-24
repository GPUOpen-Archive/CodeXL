//=============================================================
// (c) 2013 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief  Power profile control APIs
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================
#include <ntifs.h>
#include <winerror.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTDriverInternal.h>
#include <AMDTHelpers.h>
#include <AMDTCounterAccessInterface.h>
#include <WinDriverUtils\Include\Cpuid.h>
#include <WinDriverUtils\Include\Pci.hpp>
#include <WinDriverUtils\Include\Msr.h>
#define ALIGN_16_BYTES 16

// Memory pool for the profile session
// Pool is created before every configuration for the session
// and deleted once session is ended
MemoryPool* g_pSessionPool = NULL;
uint32 g_pPoolBufferCnt = 0;
static HANDLE g_hResArbitration = NULL;

extern "C" {

    uint64 ReadMsrReg(uint reg)
    {
        return __readmsr(static_cast<ulong>(reg));
    }

    void WriteMsrReg(uint reg, uint64 val)
    {
        __writemsr(static_cast<ulong>(reg), val);
    }

} // extern "C"

extern CoreData* g_pCoreCfg;
extern PPWRPROF_DEV_EXTENSION gpPwrDevExt;
static uint32 g_computeUnitCnt = INVALID_UINT32_VALUE;

//SMU NB Spec
#define SMU_INDEX_ADDR        0x800000B8
#define SMU_INDEX_DATA        0x800000BC

#define PciAddr ((PULONG)0xCF8) // IO-Space Configuration Address Register
#define PciData ((PULONG)0xCFC) // IO-Space Configuration Data Port

// IsStoping: Is stop is in progress
bool IsStoping()
{
    uint32 ulProfilerState = gpPwrDevExt->m_pClient[0].m_profileState;
    return (ulProfilerState & (STATE_PROFILING | STATE_STOPPING)) == (STATE_PROFILING | STATE_STOPPING);
}

// IsStarted: Is profile started
bool IsStarted()
{
    uint32 ulProfilerState = gpPwrDevExt->m_pClient[0].m_profileState;
    return (0 != (ulProfilerState & (STATE_PROFILING | STATE_PAUSED | STATE_STOPPING)));
}


// GetCurrentCoreId: Core id of the current execution
uint32 GetCurrentCoreId(void)
{
    return KeGetCurrentProcessorNumberEx(NULL);
}

// EncodeExtendedConfigSpaceAddress: Encode PCI address to extended PCI config speace.
uint32 HelpEncodeExtendedConfigSpaceAddress(uint32 bus, uint32 device, uint32  func, uint32 reg)
{
    _ExtendedPciAddrSpace pciAddr;
    pciAddr.address = 0U;

    pciAddr.element.configEn = 1U;
    pciAddr.element.extRegNo = (reg >> 8) & 0xFU;
    pciAddr.element.regNo = reg & 0x00FCU;
    pciAddr.element.busNo = bus;
    pciAddr.element.device = device;
    pciAddr.element.function = func;
    return pciAddr.address;
}

//CheckIfValidOperation: Check if the current state of profiler is right to carry out this IOCTL
//operation. It doesn't validate the paramters passed by IOCTL buffer
NTSTATUS CheckIfValidOperation(ULONG* aResourceCounts,
                               ULONG ulProfilerState,
                               ULONG ulIoctlOperation)
{
    NTSTATUS ret = STATUS_SUCCESS;

    //Supress warning
    (void)aResourceCounts;

    switch (ulIoctlOperation)
    {
        case (IOCTL_START_PROFILER):

            //Profile can not start if it is already in running state or
            //paused state
            if (0 != (ulProfilerState & STATE_PROFILING))
            {
                ret = STATUS_DEVICE_BUSY;
            }
            else if (0 == (ulProfilerState & STATE_TBP_SET))
            {
                //Can profile only if a configuration is set
                KdPrint(("PWRPROF: STARTING NOT VALID: Client profiler state 0x%lx\n", ulProfilerState));
                ret = STATUS_RESOURCE_TYPE_NOT_FOUND;
            }

            break;

        case (IOCTL_PAUSE_PROFILER):
        case (IOCTL_RESUME_PROFILER):

            //Not currently profiling
            if (0 == (ulProfilerState & STATE_PROFILING))
            {
                ret = STATUS_UNSUCCESSFUL;
            }

            break;

        case (IOCTL_SET_OUTPUT_FILE):

            //Currently profiling
            if (0 != (ulProfilerState & STATE_PROFILING))
            {
                ret = STATUS_DEVICE_BUSY;
            }
            else if (0 != (ulProfilerState & STATE_OUTPUT_FILE_SET))
            {
                //Output file already set
                ret = STATUS_ALREADY_COMMITTED;
            }

            break;

        case (IOCTL_STOP_PROFILER):

            //Not currently profiling
            if (0 != (ulProfilerState & STATE_PROFILING))
            {
                ret = STATUS_UNSUCCESSFUL;
            }

            break;

        case IOCTL_GET_DATA_BUFFER:

            // profile is not running
            if (0 == (ulProfilerState & STATE_PROFILING))
            {
                ret = STATUS_UNSUCCESSFUL;
            }

            break;

        default:
            break;

    }

    return ret;
}

extern NTSTATUS DeleteSharedMap();
//HelpUnregisterClient: Stop any ongoing profile, clear and re-intialize the client.
NTSTATUS HelpUnregisterClient(ClientData* pClient)
{
    NTSTATUS ret = STATUS_SUCCESS;

    //stop the profile if it is running
    if (0 != (pClient->m_profileState & STATE_PROFILING))
    {
        //Since the client is un-registering, I don't expect this reason to
        // be read, but we might do more with the value in the future.
        DRVPRINT("Aborting the profile due to unexpected unregistration.");

        HelpStopProfile(pClient);
    }

    HelpClearClient(pClient);

    // Delete the shared map
    DeleteSharedMap();

    if (!SUCCEEDED(PcoreUnregister(pClient->m_osClientCfg.m_pcoreReg)))
    {
        ret = STATUS_ACCESS_DENIED;
    }

    if (STATUS_SUCCESS == ret)
    {
        RtlZeroMemory(pClient, sizeof(ClientData));
    }

    //Release memory pool
    ReleaseMemoryPool();
    g_pCoreCfg = NULL;

    //allow the client to be claimed by some other personality
    InterlockedExchange((LONG*) & (pClient->m_validClient), SYNCH_AVAILABLE);

    return ret;
}

//HelpAddPcoreCfgs Add the configurations to the applicable pcore cores and resources.
// This is where the actual profiling occurs, from the viewpoint of PwrProf
NTSTATUS HelpAddPcoreCfgs(ClientData* pClient, CoreData* pCoreCfg)
{
    NTSTATUS ret = STATUS_SUCCESS;
    uint32 cnt = 0;

    //For each configuration in the list. One cofiguration per core
    for (cnt = 0 ; cnt < pClient->m_configCount; cnt++)
    {
        HRESULT hr;
        OsCoreCfgData* pCfg = NULL;

        if (NULL == (pCoreCfg + cnt))
        {
            DRVPRINT(" STATUS_ACCESS_DENIED: pCoreCfg + cnt");
            ret = STATUS_ACCESS_DENIED;
            break;
        }

        pCfg = (pCoreCfg + cnt)->m_pOsData;

        if ((NULL == pCfg) || (pCfg->m_pcoreCfg.m_coreId > (gpPwrDevExt->coreCount - 1)))
        {
            DRVPRINT("Invalid pCfg or pCfg->m_pcoreCfg.m_coreId > max core");
            ret = STATUS_ACCESS_DENIED;
            break;
        }

        hr = PcoreAddConfiguration(pClient->m_osClientCfg.m_pcoreReg,
                                   pCfg->m_pcoreCfg.m_coreId,
                                   APIC,
                                   0,
                                   &pCfg->m_pcoreCfg.m_cfg);

        if (S_OK != hr)
        {
            DRVPRINT("pcore config res: 0x%lx, client %ld, core %ld failed",
                     hr, pClient->m_clientId, pCfg->m_pcoreCfg.m_coreId);

            //If there was trouble adding any of the configurations,
            //abort the entire profile!
            PcoreRemoveAllConfigurations(pClient->m_osClientCfg.m_pcoreReg);

            if (E_ACCESSDENIED != hr)
            {
                ret = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            else
            {
                ret = STATUS_LOCK_NOT_GRANTED;
                break;
            }
        }
    }

    return ret;
} //HelpAddPcoreCfgs


///HelpRemovePcoreCfgs: Remove the configurations from pcore, saving the counts. We could use
/// PcoreRemoveAllConfigurations, but that won't save the counts when this is
/// called for pausing
NTSTATUS HelpRemovePcoreCfgs(ClientData* pClient)
{
    NTSTATUS ret = STATUS_SUCCESS;

    //pull configs out of pcore, so the counters don't advance
    if (S_OK != PcoreRemoveAllConfigurations(pClient->m_osClientCfg.m_pcoreReg))
    {
        ret = STATUS_UNSUCCESSFUL;
    }

    return ret;
} //HelpRemovePcoreCfgs


//HelpStopProfile: Stop profiling, flush any partial buffers to the files, and clear the
//previous profile's settings.
NTSTATUS HelpStopProfile(ClientData* pClient)
{
    NTSTATUS ret = STATUS_SUCCESS;

    //if it's aborted already, the FlushAndFree functions would try to abort it
    // again, so this check prevents that
    if (0 != (pClient->m_profileState & STATE_STOPPING))
    {
        DRVPRINT("HelpStopProfile exited, due to stopping already");
        ret = STATUS_SUCCESS;
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient->m_profileState |= STATE_STOPPING;

        //If it's not actually profiling, it will act the same as the
        // IoctlClearProfiler
        if (0 == (pClient->m_profileState & STATE_PROFILING))
        {
            KdPrint(("PWRPROF: HelpStopProfile exited, due to not profiling state\n"));
            ret =  HelpClearClient(pClient);
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        // Terminate any stuff going on for us on that core
        if (S_OK != PcoreRemoveAllConfigurations(pClient->m_osClientCfg.m_pcoreReg))
        {
            ret = STATUS_UNSUCCESSFUL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        ret = HelpClearClient(pClient);
    }

    return ret;
} //HelpStopProfile

//HelpClearClient: Clear the profile settings, except for abort reason, overhead data, and
//profile record count
NTSTATUS HelpClearClient(ClientData* pClient)
{
    pClient->m_configCount = 0;
    pClient->m_profileState = STATE_NOT_CONFIGURED;
    return STATUS_SUCCESS;
} //HelpClearClient

//HelpCheckClient: Determine if the client id is to a valid client
bool HelpCheckClient(IN PPWRPROF_DEV_EXTENSION pDevExt, uint32 clientId)
{
    if (clientId >= MAX_CLIENT_COUNT)
    {
        return FALSE;
    }

    if (CLIENT_REGISTERED != pDevExt->m_pClient[clientId].m_validClient)
    {
        return FALSE;
    }

    return TRUE;
}

// GetTargetCoreCount: Get the total number of core in the cpu
uint32 GetTargetCoreCount(void)
{
    uint32 cores = 0;

    if (NULL != gpPwrDevExt)
    {
        cores = gpPwrDevExt->coreCount;
    }

    return cores;
}

void ReadPci32Reg(uint bus, uint device, uint func, uint reg, uint& data)
{
    uint32 address = HelpEncodeExtendedConfigSpaceAddress(bus, device, func, reg);

    // Set new address
    WRITE_PORT_ULONG(PciAddr, static_cast<ULONG>(address));

    // Read current data
    ULONG currentData = READ_PORT_ULONG(PciData);

    data = static_cast<uint>(currentData);
}

//GetComputeUnitCntPerNode
uint32 GetComputeUnitCntPerNode()
{
    if (0xFFFFFFFF == g_computeUnitCnt)
    {
        // D18F5x80 gives the CU count
        uint32 bus = 0U;
        uint32 device = 0x18U;
        uint32 function = 0x5U;
        uint32 reg = 0x80U;
        uint32 cuStatus = 0U;
        uint32 corePerCu = 0;

        ReadPci32Reg(bus, device, function, reg, cuStatus);

        if (PLATFORM_MULLINS == HelpPwrGetTargetPlatformId())
        {
            g_computeUnitCnt = 1;
        }
        else
        {
            DecodeCURegisterStatus(cuStatus, &g_computeUnitCnt, &corePerCu);
        }
    }

    return g_computeUnitCnt;
}

// GetCpuModelFamily: Get family and model number of the cpu
void GetCpuModelFamily(uint32* family, uint32* model)
{
    CpuInfo info;
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };
    __cpuid(aCPUInfo, CPUID_FnBasicFeatures);

    info.eax = aCPUInfo[0];
    *family = info.info.family;
    *model = info.info.model;

    if (FAMILY_EXTENDED == *family)
    {
        *family += info.info.extFamily;
    }

    //From the Bkdg: If ExtendedModel[3:0]=Eh and BaseModel[3:0]=8h, then Model[7:0] = E8h
    *model += (info.info.extModel << 4);
}

// HelpIsCefSupported: Check if core effective frequency feature is available
bool HelpIsCefSupported()
{
    bool result = false;
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

    __cpuid(aCPUInfo, CPUID_FnThermalAndPowerManagement);

    // Effective Frequency is supported
    result = (aCPUInfo[ECX_OFFSET] & CPUID_FnThermalAndPowerManagement_ECX_EffFreq) != 0;
    return result;
}

// HelpIsROCefAvailable
bool HelpIsROCefAvailable()
{
    bool result = false;
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

    __cpuid(aCPUInfo, CPUID_FnAdvancePowerManagementInformation);

    // Effective Frequency is supported
    result = (aCPUInfo[EDX_OFFSET] & CPUID_FnAdvancePowerManagementInformation_EDX_EffFreqRO) != 0;
    return result;
}

// HelpIsPMCCounterAvailable
bool HelpIsPMCCounterAvailable()
{
    bool result = false;
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

    __cpuid(aCPUInfo, CPUID_FnAmdExtendedFeatures);

    // Effective Frequency is supported
    result = (aCPUInfo[ECX_OFFSET] & CPUID_FnAmdExtendedFeatures_ECX_PerfCtrExtCore) != 0;
    return result;

}

// HelpAccessPciAddress: Read/Write PCI address space
bool HelpAccessPciAddress(PACCESS_PCI pData)
{
    bool ret = true;

    if (NULL == pData)
    {
        ret = false;
    }

    if (true == ret)
    {
        // Enable Access to the PCI Extended Configuration Space
        // It is not done by default for Pci write
        // TODO: need to check if we are really usingEnableExtendedPCIConfigSpace();

        if (pData->isReadAccess)
        {
            // Set new address
            WRITE_PORT_ULONG(PciAddr, static_cast<ULONG>(pData->address));

            // Read current data
            ULONG currentData = READ_PORT_ULONG(PciData);
            pData->data = static_cast<uint>(currentData);
        }
        else
        {
            // Get the address to write to
            // Set new address
            WRITE_PORT_ULONG(PciAddr, static_cast<ULONG>(pData->address));

            // Write data
            WRITE_PORT_ULONG(PciData, static_cast<ULONG>(pData->data));
        }

        ret = true;
    }

    return ret;
}

// HelpAccessMSRAddress: Read/Write MSR
bool HelpAccessMSRAddress(PACCESS_MSR pData)
{
    bool ret = false;

    if (NULL != pData)
    {
        if (pData->isReadAccess)
        {
            pData->data = static_cast<ulong>(ReadMsrReg(pData->regId));

        }
        else
        {
            WriteMsrReg(pData->regId, static_cast <uint64>(pData->data));
        }

        ret = true;
    }

    return ret;
}

// HelpReadMsr64: Read 64 bit MSR address
uint64 HelpReadMsr64(uint32 reg)
{
    return ReadMsrReg(reg);

}

// HelpGetBitsCount: Get the set bit count
void HelpGetBitsCount(uint64 mask, uint32* pCount)
{
    uint32 count = 0;
    uint32 loop = 8 * sizeof(uint64);

    while (loop--)
    {
        if (mask & (1ULL << loop))
        {
            count++;
        }
    }

    *pCount = count;
}

// HelpMapMMIOSpace: Map MMIO space
bool HelpMapMMIOSpace(
    uint64  address,         // IN
    size_t  size,            // IN
    uint64* mappedAddress,   // OUT
    uint64* mappedSize)      // OUT
{
    bool result = false;
    void* pLinearAddress = NULL;
    PHYSICAL_ADDRESS physicalAddress;

    ResetPoolMemory(&physicalAddress, sizeof(PHYSICAL_ADDRESS));
    physicalAddress.QuadPart = address;

    pLinearAddress = static_cast<PUCHAR>(MmMapIoSpace(physicalAddress, size, MmNonCached));

    if (NULL != pLinearAddress)
    {
        if (MmIsAddressValid(pLinearAddress))
        {
            *mappedAddress = reinterpret_cast<uint64>(pLinearAddress);
            *mappedSize = size;
            result = true;
        }
        else
        {
            MmUnmapIoSpace(pLinearAddress, size);
        }
    }

    return result;
}

// HelpUnmapMMIOSpace: Unmapping MMIO space
bool HelpUnmapMMIOSpace(uint64 mappedAddress, uint64 mappedSize)
{
    bool result = false;
    void* pLinearAddress = reinterpret_cast<void*>(mappedAddress);

    if (NULL != pLinearAddress)
    {
        MmUnmapIoSpace(pLinearAddress, static_cast<size_t>(mappedSize));
        result = true;
    }

    return result;
}

// HelpGmmxGetBaseAddress: Get the base address of gMMx space
uint64 HelpGmmxGetBaseAddress(uint32 gpuAddr)
{

    ACCESS_PCI pci;
    uint32 baseGPUAddressLow = 0;

    pci.address = gpuAddr;
    pci.isReadAccess = true;

    HelpAccessPciAddress(&pci);
    baseGPUAddressLow = pci.data;

    // set bits 0:3 to 0
    baseGPUAddressLow = baseGPUAddressLow & 0xFFFFFFF0;

    return baseGPUAddressLow;
}

//GetTimeStamp
void GetTimeStamp(uint64* pTime)
{
    LARGE_INTEGER systemTime;
    KeQuerySystemTime(&systemTime);

    LARGE_INTEGER localTime;
    ExSystemTimeToLocalTime(&systemTime, &localTime);

    *pTime = (uint64)localTime.QuadPart;
}

// GetPerformanceCounter:
void GetPerformanceCounter(uint64* pPerfCounter, uint64* pFrequency)
{
    LARGE_INTEGER perfCounter;
    LARGE_INTEGER perfFreq;

    perfCounter = KeQueryPerformanceCounter(&perfFreq);

    if (NULL != pPerfCounter)
    {
        *pPerfCounter = (uint64)perfCounter.QuadPart;
    }

    if (NULL != pFrequency)
    {
        *pFrequency = (uint64)perfFreq.QuadPart;
    }
}

// CreateMemoryPool: Create memory pool
bool CreateMemoryPool()
{
    bool ret = false;
    uint32 cnt = 0;


    if (NULL == g_pSessionPool)
    {
        g_pPoolBufferCnt = POOL_BASE_SIZE + KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

        g_pSessionPool = (MemoryPool*)ExAllocatePoolWithTag(NonPagedPool, g_pPoolBufferCnt * sizeof(MemoryPool), 'PLM');

        if (NULL != g_pSessionPool)
        {
            for (cnt = 0; cnt < g_pPoolBufferCnt; cnt++)
            {
                MemoryPool* pPool = g_pSessionPool + cnt;

                if (NULL != pPool)
                {
                    uint8* pMem = (uint8*)ExAllocatePoolWithTag(NonPagedPool, DATA_PAGE_BUFFER_SIZE, 'PLB');

                    if (NULL != pMem)
                    {
                        pPool->m_pBase = pMem;
                        pPool->m_offset = 0;
                        pPool->m_size = DATA_PAGE_BUFFER_SIZE;
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}

// GetMemoryPoolBuffer: Get buffer from the pool
void* GetMemoryPoolBuffer(uint32 size, bool resetMem)
{
    void* pBuffer = NULL;
    uint32 cnt = 0;

    if ((NULL != g_pSessionPool) && (size > 0) && (size <= DATA_PAGE_BUFFER_SIZE))
    {
        for (cnt = 0; cnt < g_pPoolBufferCnt; cnt++)
        {
            MemoryPool* pPool = g_pSessionPool + cnt;

            if ((NULL != pPool) && (NULL != pPool->m_pBase))
            {
                if ((pPool->m_offset + size) <= pPool->m_size)
                {
                    pBuffer = (uint8*)pPool->m_pBase + pPool->m_offset;
                    pPool->m_offset += size;
                    uint32 alignBytes = (pPool->m_offset % ALIGN_16_BYTES);
                    pPool->m_offset += alignBytes ? (ALIGN_16_BYTES - alignBytes) : 0;

                    if (true == resetMem)
                    {
                        ResetPoolMemory(pBuffer, size);
                    }

                    break;
                }
                else
                {
                    // Current buffer doesn't have available memory.
                    // Check in the next chunk
                    continue;
                }
            }
        }
    }
    else
    {
        DRVPRINT("Memory pool not created/ invalid");
        pBuffer = NULL;
    }

    if (NULL == pBuffer)
    {
        DRVPRINT("Memory Allocation Failed");
    }

    return pBuffer;
}

// ReleaseMemoryPool Delete the memory pool
bool ReleaseMemoryPool()
{
    uint32 cnt = 0;
    bool ret = false;

    if (NULL != g_pSessionPool)
    {
        for (cnt = 0; cnt < g_pPoolBufferCnt; cnt++)
        {
            MemoryPool* pPool = g_pSessionPool + cnt;

            if ((NULL != pPool) && (NULL != pPool->m_pBase))
            {
                ExFreePoolWithTag(pPool->m_pBase, 'PLB');
                pPool->m_pBase = NULL;
                pPool->m_offset = 0;
                pPool->m_size = 0;
                ret = true;
            }
        }

        ExFreePoolWithTag(g_pSessionPool, 'PLM');
        g_pSessionPool = NULL;
        g_pPoolBufferCnt = 0;
    }

    return ret;
}

// ResetPoolMemory: Set the memory to 0
void ResetPoolMemory(void* pBuffer, uint32 size)
{
    if (NULL != pBuffer)
    {
        RtlZeroMemory(pBuffer, size);
    }
    else
    {
        DRVPRINT("Trying to access Invalid memory");
    }
}

// AcquirePCMCountersLock: Check if PMC counters are available
bool AcquirePCMCountersLock()
{
    bool res = true;

    if (HalAllocateHardwareCounters(NULL, 0, NULL, &g_hResArbitration) != STATUS_SUCCESS)
    {
        res = false;
        PrintError("HAL Arbitration failed!");
    }

    return res;
}

// ReleasePCMCountersLock: Release if PMC counters are acquired
bool ReleasePCMCountersLock()
{
    bool res = false;

    if (g_hResArbitration != NULL)
    {
        HalFreeHardwareCounters(g_hResArbitration);
        res = true;
    }

    return res;
}

// PwrGetLogicalProcessCount:  Get the number of logical cores
uint32 PwrGetLogicalProcessCount(void)
{
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };
    __cpuid(aCPUInfo, CPUID_FnFeatureId);

    uint32 numOfThreads = (aCPUInfo[EBX_OFFSET] & CPUID_FeatureId_EBX_LogicalProcessorCount) >> 16;
    return numOfThreads;
}

// HelpPwrIsSmtEnabled: Check if thread per core is more than 1
bool HelpPwrIsSmtEnabled()
{
    uint32 numOfThreads = 0;

    bool result = false;
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };
    __cpuid(aCPUInfo, CPUID_FnIdentifiers);

    numOfThreads = (aCPUInfo[EBX_OFFSET] & CPUID_NodeIdentifiers_EBX_ThreadsPerCore) + 1;

    result = (numOfThreads > 1) ? true : false;
    return result;
}

