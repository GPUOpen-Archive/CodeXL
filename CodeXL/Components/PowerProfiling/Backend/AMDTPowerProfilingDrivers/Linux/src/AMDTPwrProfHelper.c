//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================

// LOCAL INCLUDES
#include <AMDTCounterAccessInterface.h>
#include <AMDTHelpers.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTPwrProfInternal.h>
#include <AMDTPwrProfHwaccess.h>
#include <AMDTPwrProfCpuid.h>
#include <AMDTPwrProfCoreUtils.h>

// Memory pool for the profile session
// Pool is created before every configuration for the session
// and deleted once session is ended
MemoryPool g_sessionPool;

DEFINE_PER_CPU(CefInfo, g_prevROCefData);

//SMU NB Spec
#define SMU_INDEX_ADDR          0x800000B8
#define SMU_INDEX_DATA          0x800000BC
#define PCI_ADDR_PORT       0xCF8
#define PCI_DATA_PORT       0xCFC

// STATIC VARIABLES
static uint32 g_computeUnitCnt = INVALID_UINT32_VALUE;
static bool IsCpuSigRead = false;

// struct for cpu signature
static CpuSignature cpu_sig =
{
    .m_value = 0,
    .m_isHypervisor = false,
};

// LOCAL FUNCTIONS
//
// Read CPU Signature.
void ReadCpuSignature(CpuSignature* cpu)
{

    uint32 dwEax, dwEbx, dwEcx, dwEdx;
    char vendorId[13];

    dwEax = dwEbx = dwEcx = dwEdx = 0;

    ReadCPUID(0, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);

    DRVPRINT("cpuid1 a=%u, b=%u, c=%u,d =%u \n", dwEax, dwEbx, dwEcx, dwEdx);
    memcpy(vendorId, &dwEbx, 4);
    memcpy(vendorId + 4, &dwEdx , 4);
    memcpy(vendorId + 8, &dwEcx, 4);
    vendorId[12] = '\0';

    if (0 != strcmp(vendorId , "AuthenticAMD"))
    {
        printk(KERN_WARNING "pcore: NON AMD CPU found \n");
        cpu->m_value = 0;
        return;
    }

    ReadCPUID(CPUID_FnBasicFeatures, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);

    cpu->m_value = dwEax;
    cpu->m_isHypervisor = false;

    if ((dwEcx & CPUID_FnBasicFeatures_ECX_Hypervisor) != 0)
    {
        cpu->m_isHypervisor = true;
        printk(KERN_WARNING "pcore: Hypervisor Detected\n");
    }
}

// Get CPU signature.
CpuSignature* GetCpuSignature(void)
{

    if (!IsCpuSigRead)
    {
        ReadCpuSignature(&cpu_sig);
        IsCpuSigRead = true;
    }

    return &cpu_sig;
}

// Check for Amd platform
bool IsAmd(CpuSignature* cpu)
{
    return cpu->m_value != 0U;
}

// Get Cpu family
uint GetFamilyValue(CpuSignature* cpu)
{
    return ((cpu->m_value & CpuBaseFamily_MASK) >> 8) + ((cpu->m_value & CpuExtFamily_MASK) >> 20);
}

// Get Cpu model
uint GetModelValue(CpuSignature* cpu)
{
    return ((cpu->m_value & CpuBaseModel_MASK) >> 4) | ((cpu->m_value & CpuExtModel_MASK) >> (16 - 4));
}

// Encode PCI address to extended PCI config speace.
uint32 HelpEncodeExtendedConfigSpaceAddress(uint32 bus, uint32 device, uint32  func, uint32 reg)
{
    ExtendedPciAddrSpace pciAddr;
    pciAddr.address = 0U;

    pciAddr.element.configEn = 1U;
    pciAddr.element.extRegNo = (reg >> 8) & 0xFU;
    pciAddr.element.regNo = reg & 0x00FCU;
    pciAddr.element.busNo = bus;
    pciAddr.element.device = device;
    pciAddr.element.function = func;
    return pciAddr.address;
}

// Get number of compute units per node.
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

        ReadPCI32(bus, device, function, reg, &cuStatus);

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

// Get Cpu model and family
void GetCpuModelFamily(uint32* family, uint32* model)
{
    CpuSignature* sig = GetCpuSignature();
    *family = GetFamilyValue(sig);
    *model  = GetModelValue(sig);
}

// Check if core effective Frequency supported.
bool HelpIsCefSupported()
{
    uint32 dwEax, dwEbx, dwEcx, dwEdx;

    dwEax = dwEbx = dwEcx = dwEdx = 0;

    ReadCPUID(CPUID_FnThermalAndPowerManagement, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);

    return (dwEcx & CPUID_FnThermalAndPowerManagement_ECX_EffFreq) != 0;
}

// Check if Core effective Frequency available.
bool HelpIsROCefAvailable()
{
    uint32 dwEax, dwEbx, dwEcx, dwEdx;

    dwEax = dwEbx = dwEcx = dwEdx = 0;

    ReadCPUID(CPUID_FnAdvancePowerManagementInformation, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);

    return (dwEdx & CPUID_FnAdvancePowerManagementInformation_EDX_EffFreqRO) != 0;
}

// Access PCIe Address space for read/write
bool HelpAccessPciAddress(PACCESS_PCI pData)
{
    bool ret = false;
    uint32 pci_data;

    if (NULL != pData)
    {
        // Enable Access to the PCI Extended Configuration Space
        // It is not done by default for Pci write
        // TODO: need to check if we are really using
        // EnableExtendedPCIConfigSpace();

        if (pData->isReadAccess)
        {
            ReadPCI(pData->address, &pci_data);
            pData->data = (uint32)pci_data;
        }
        else
        {
            WritePCI(pData->address, pData->data);
        }

        ret = true;
    }

    return ret;
}

// Access MSR Address space fore read/write
bool HelpAccessMSRAddress(PACCESS_MSR pData)
{
    bool ret = false;

    uint64 msr_data;

    if (NULL != pData)
    {
        if (pData->isReadAccess)
        {
            ReadMSR(pData->regId, &msr_data);
            pData->data = (uint32)msr_data;
        }
        else
        {
            WriteMSR(pData->regId, pData->data);
        }

        ret = true;
    }

    return ret;
}

// Read MSR
uint64 HelpReadMsr64(uint32 reg)
{
    uint64 data;
    ReadMSR(reg, &data);
    return data;
}

// Read from ndex and SMU soft north bridge specific register.
bool HelpReadSmuNBSpecRegister(uint32 reg, uint32* pData)
{
    bool result = false;
    ACCESS_PCI pciData;
    pciData.address = SMU_INDEX_ADDR;
    pciData.data = reg;
    pciData.isReadAccess = false;

    // Write index register
    result = HelpAccessPciAddress(&pciData);

    // Read SMU soft register
    pciData.address = SMU_INDEX_DATA;
    pciData.isReadAccess = true;
    HelpAccessPciAddress(&pciData);
    *pData = pciData.data;
    return result;
}

// Write to index and SMU soft north bridge specific register.
bool HelpWriteSmuNBSpecRegister(uint32 reg, uint32* pData)
{
    bool result = false;
    ACCESS_PCI pciData;
    pciData.address = SMU_INDEX_ADDR;
    pciData.data = reg;
    pciData.isReadAccess = false;

    // Write index register
    result = HelpAccessPciAddress(&pciData);

    // Write SMU soft register
    pciData.address = SMU_INDEX_DATA;
    pciData.isReadAccess = false;
    pciData.data = *pData;
    HelpAccessPciAddress(&pciData);
    return result;
}

// Return the number of bits set.
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

// Map MMIO space
bool HelpMapMMIOSpace(
    uint64  address,         // IN
    size_t  size,            // IN
    uint64* mappedAddress,   // OUT
    uint64* mappedSize)      // OUT
{

    bool result = true;
    *mappedAddress = (uint64)ioremap(address, size);
    *mappedSize = size;
    return result;
}

// Unmapping MMIO space
bool HelpUnmapMMIOSpace(uint64 mappedAddress, uint64 mappedSize)
{
    bool result = true;
    iounmap((void*)mappedAddress);
    return result;
}

// Get GPU base address
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

// Check if hardware is supported.
long CheckHwSupport(void)
{
    uint family;
    uint model;

    CpuSignature* sig = GetCpuSignature();

    /* Following are not supported:
    1. Non AMD Platforms
    2. Hypervisors
    */
    if (!IsAmd(sig))
    {
        printk(KERN_WARNING "pcore:Non AMD Platform detected. pcore only supports AMD platforms\n");
        return -EACCES;
    }

    if (sig->m_isHypervisor)
    {
        printk(KERN_WARNING "pcore:Hypervisor detected. pcore does not support Hypervisor platforms\n");
        return -EACCES;
    }

    /*Supported AMD platforms
        Kaveri  :  0x15 30 to 3F
        Carrizo :  0x15 60 to 6F
        Mullins :  0x16 30 to 3F
    */
    family = GetFamilyValue(sig);
    model  = GetModelValue(sig);
    DRVPRINT(" family %x , model %x \n", family, model);

    if ((family < 0x15) || (family > 0x16))
    {
        printk(KERN_WARNING "pcore: Unsupported family 0x%x \n", family);
        return -EACCES;
    }

    if (0x15 == family)
    {
        if (!((model >= 0x30 && model <= 0x3F)
              || (model >= 0x60 && model <= 0x6F)))
        {
            printk(KERN_WARNING "pcore: Unsupported model 0x%x for family 0x%x \n", model, family);
            return -EACCES;
        }
    }

    if (0x16 == family)
    {
        if (!(model >= 0x30 && model <= 0x3F))
        {
            printk(KERN_WARNING "pcore: Unsupported model 0x%x for family 0x%x \n", model, family);
            return -EACCES;
        }
    }

    /*
    4. TODO: AMD dGPU on a non AMD platforms SHOULD be supported.
    5. If SMU is not avaliabe support only Core Counters/ MSR's.
    6. If BAPM is disabled - Enable/ Disable through BIOS messages.
    7. If iGPU is disabled same as #5.
    */

    return 0;
}

// Get the current core id
uint32 GetCurrentCoreId(void)
{
    int cpu = get_cpu();
    put_cpu();
    return (uint32)cpu;
}

// Get number of control unit present in a node.
uint32 GetCuCountPerNode(void)
{
    // D18F5x80 gives the CU count
    uint bus = 0U;
    uint device = 0x18U;
    uint function = 0x5U;
    uint reg = 0x80U;

    uint cuStatus = 0U;

    ReadPCI32(bus, device, function, reg, &cuStatus);

    return (cuStatus & 0x1U) + ((cuStatus >> 1) & 0x1U);
}

void GetPerformanceCounter(uint64* perfCounter, uint64* freq)
{
    *perfCounter = ktime_to_ns(ktime_get());
    *freq = 1;
}

// Get current time from thr kernel
void GetTimeStamp(uint64* ts)
{
    struct timespec t = current_kernel_time();
    // return value in mill-seconds
    *ts = (t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

// Get the core oounts.
uint32 GetTargetCoreCount(void)
{
    uint32 i;
    i = num_online_cpus();
    return i;
}

// Create memory pool
bool CreateMemoryPool(MemoryPool* pPool, uint32 size)
{
    bool ret = false;

    if (NULL != pPool)
    {
        pPool->m_pBase = (uint8*)AllocateMemory(size, GFP_KERNEL);

        if (NULL != pPool->m_pBase)
        {
            pPool->m_offset = 0;
            pPool->m_size = size;
            ret = true;
        }
    }

    return ret;
}

// Get buffer from the memory pool
uint8* GetMemoryPoolBuffer(MemoryPool* pPool, uint32 size)
{
    uint8* pBuffer = NULL;

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
bool ReleaseMemoryPool(MemoryPool* pPool)
{
    bool ret = false;

    if (NULL != pPool)
    {
        if (NULL != pPool->m_pBase)
        {
            FreeMemory(pPool->m_pBase);
            pPool->m_pBase = NULL;
            ret = true;
        }
    }

    return ret;
}

bool HelpIsPMCCounterAvailable(void)
{
    uint32 dwEax, dwEbx, dwEcx, dwEdx;

    dwEax = dwEbx = dwEcx = dwEdx = 0;

    ReadCPUID(CPUID_FnAmdExtendedFeatures, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);

    return (dwEdx & CPUID_FnAmdExtendedFeatures_ECX_PerfCtrExtCore) != 0;

}

// AcquirePCMCountersLock: Check if PMC counters are available
bool AcquirePCMCountersLock()
{
    return false;
}

// ReleasePCMCountersLock: Release if PMC counters are acquired
bool ReleasePCMCountersLock()
{
    return false;
}

// PwrGetLogicalProcessCount:  Get the number of logical cores
uint32 PwrGetLogicalProcessCount(void)
{
    uint32 numOfThreads = 0;
    uint32 dwEax, dwEbx, dwEcx, dwEdx;
    dwEax = dwEbx = dwEcx = dwEdx = 0;

    ReadCPUID(CPUID_FnFeatureId, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);
    numOfThreads = (dwEbx & CPUID_FeatureId_EBX_LogicalProcessorCount) >> 16;
    return numOfThreads;
}

// HelpPwrIsSmtEnabled: Check if thread per core is more than 1
bool HelpPwrIsSmtEnabled()
{
    uint32 numOfThreads = 0;

    bool result = false;

    uint32 dwEax = 0;
    uint32 dwEbx = 0;
    uint32 dwEcx = 0;
    uint32 dwEdx = 0;


    ReadCPUID(CPUID_FnIdentifiers, 0, &dwEax, &dwEbx, &dwEcx, &dwEdx);

    numOfThreads = (dwEbx & CPUID_NodeIdentifiers_EBX_ThreadsPerCore) + 1;

    result = (numOfThreads > 1) ? true : false;

    return result;
}

