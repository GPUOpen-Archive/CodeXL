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

#ifndef _HELPERS_H
#define _HELPERS_H
#include <AMDTDriverTypedefs.h>
#include <AMDTPwrProfAttributes.h>
#include <AMDTDriverInternal.h>
#include <AMDTRawDataFileHeader.h>

#ifdef _WIN32
    #include <ntifs.h>
    #define DRVPRINT(fmt, ...)    DbgPrint( "PwrProf: %s, "fmt "\n",__FUNCTION__, __VA_ARGS__)
#endif
#ifdef _LINUX
    #ifdef DEBUG
        #define DRVPRINT(fmt, ...)   printk("%s %d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
    #else
        #define DRVPRINT(fmt, ...) {}
    #endif
#endif

#define FAMILY_EXTENDED 0x0f
#define INVALID_UINT32_VALUE 0xFFFFFFFF
#define CPUID_FnAdvancePowerManagementInformation 0x80000007
#define CPUID_FnAdvancePowerManagementInformation_EDX_EffFreqRO (1 << 10)
#define POOL_BASE_SIZE  16
#define CPUID_FnFeatureId 1
#define CPUID_NodeIdentifiers_EBX_ThreadsPerCore (0xFF << 8)
#define CPUID_FeatureId_EBX_LogicalProcessorCount 0xFF << 16
#define CPUID_FnThermalAndPowerManagement_ECX_EffFreq  (1 << 0)

union CpuInfo
{
    struct _info
    {
        /// Bits[3:0] The Cpu stepping
        uint32 stepping : 4;
        /// Bits[7:4] The Cpu base model (model bits [3:0])
        uint32 model : 4;
        /// Bits[11:8] The Cpu base family
        uint32 family : 4;
        /// Bits[15:12] Reserved
        uint32 unknown1 : 4;
        /// Bits[19:16] The Cpu extended model (model bits[7:4])
        uint32 extModel : 4;
        /// Bits[27:20] The Cpu extended family (family = family + extFamily)
        uint32 extFamily : 8;
        /// Bits[31:28] Reserverd
        uint32 unknown2 : 4;
    } info;
    /// The value of the EAX register
    uint32 eax;
};

//MemoryPool: create memory pool for internal use
typedef struct
{
    void* m_pBase;
    uint32 m_offset;
    uint32 m_size;
} MemoryPool;

#ifdef LINUX
    extern MemoryPool g_sessionPool;
    #define SESSION_POOL_SIZE 1024

    // Create memory pool
    bool CreateMemoryPool(MemoryPool* pPool, uint32 size);

    // Get buffer from the pool
    uint8* GetMemoryPoolBuffer(MemoryPool* pPool, uint32 size);

    // Delete the memory pool
    bool ReleaseMemoryPool(MemoryPool* pPool);
#else
    // CreateMemoryPool: Create memory pool
    bool CreateMemoryPool();

    // GetMemoryPoolBuffer: Get buffer from the pool
    void* GetMemoryPoolBuffer(uint32 size, bool resetMem);

    // ReleaseMemoryPoo: lDelete the memory pool
    bool ReleaseMemoryPool();

    // ResetPoolMemory: Set the memory to 0
    void ResetPoolMemory(void* pBuffer, uint32 size);
#endif
// GetTargetCoreCount
uint32 GetTargetCoreCount(void);

//GetComputeUnitCntPerNode - get compute unit count and core count for a node
uint32 GetComputeUnitCntPerNode(void);

//GetCpuModelFamily - get CPU family id and model id
void GetCpuModelFamily(uint32* family, uint32* model);

//Is profile stopping
bool IsStoping(void);

//HelpAccessPciAddress: Read or Write PCI address
bool HelpAccessPciAddress(PACCESS_PCI pData);

//HelpAccessMSRAddress: Read or write generic MSR
bool HelpAccessMSRAddress(PACCESS_MSR pData);

// HelpReadMsr64: Read 64 bit MSR address
uint64 HelpReadMsr64(uint32 reg);

// HelpWriteMsr64: Write 64 bit MSR address
uint64 HelpWriteMsr64(uint32 reg, uint64 value);

// EncodeExtendedConfigSpaceAddress: Encode PCI address to extended PCI config speace.
uint32 HelpEncodeExtendedConfigSpaceAddress(uint32 bus,
                                            uint32 device,
                                            uint32  func,
                                            uint32 reg);

// HelpPwrGetTargetPlatform: Return the current target platform id
uint32 HelpPwrGetTargetPlatformId(void);

// HelpGetBitsCount: Count number of bits set
void HelpGetBitsCount(uint64 mask, uint32* pCount);

// HelpMapMMIOSpace: Map MMIO space
bool HelpMapMMIOSpace(uint64  address,
                      size_t  size,
                      uint64* pMappedAddress,
                      uint64* pMappedSize);

// HelpUnmapMMIOSpace: Unmapping MMIO space
bool HelpUnmapMMIOSpace(uint64 mappedAddress, uint64 mappedSize);

//Is profile stopping
bool IsStarted(void);

// HelpIsCefSupported
bool HelpIsCefSupported(void);

// HelpIsROCefAvailable
bool HelpIsROCefAvailable(void);

uint64 HelpGmmxGetBaseAddress(uint32 gpuAddr);

uint32 GetCurrentCoreId(void);

//GetTimeStamp
void GetTimeStamp(uint64* pTime);

// Raw data file functions
bool GetRequiredBufferLength(CoreData* cfg, uint32* pLength);

int32 WriteSampleData(CoreData* cfg);

int32 WriteRawBufferHeader(RawFileHeader* pHeader, uint16 sectionCnt);

int32 WriteSectionHeaders(SectionHdrInfo* secHeader, uint64 secHeaderMask);

int32 UpdateBufferHeader(ClientData* pClient, uint16 field);

int32 WriteSampleCfgInfo(ClientData* pData, ProfileConfig* pSrcCfg);

int32 WriteSampleInfo(ClientData* pClient);

int32 WriteSections(ClientData* pClient, ProfileConfig* pSrcCfg, uint64 secMask);

int32 WriteHeader(ClientData* pClient, ProfileConfig* pSrcCfg);

bool HelpIsPMCCounterAvailable(void);

void GetPerformanceCounter(uint64* pPerfCounter, uint64* pFrequency);

bool AcquirePCMCountersLock(void);

bool ReleasePCMCountersLock(void);

uint32 PwrGetLogicalProcessCount(void);

bool HelpPwrIsSmtEnabled(void);

// HelpPwrEnablePerf: Enable perf bit
void HelpPwrEnablePerf(bool enable);

uint32 HelpPwrGetPhysicalCoreCount(void);

#endif //_HELPERS_H
