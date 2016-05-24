//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileHelper.cpp
///
//==================================================================================

//#ifndef _POWERPROFILEHELPER_CPP_
//#define _POWERPROFILEHELPER_CPP_
//Compile only once: Either with control module or Data access module
#include "PowerProfileHelper.h"
#include <AMDTDefinitions.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTPwrProfAttributes.h>
#include <PowerProfileDriverInterface.h>
#include <stdarg.h>

#if ( defined (_WIN32) || defined (_WIN64) )
    #include <windows.h>
    #include <intrin.h>
#endif
#include <string.h>
#include <stdio.h>

AMDTPwrProfileAttributeList g_attributeList;

//SetRawAttributeMask:
AMDTResult SetRawAttributeMask(AMDTUInt16 attributeId, AMDTUInt64* pEventMask)
{
    if (attributeId > COUNTERID_MAX_CNT)
    {
        return AMDT_ERROR_FAIL;
    }

    pEventMask[attributeId / 64] |= (1ULL << (attributeId % 64));

    return AMDT_STATUS_OK;
}

// ReadPciAddress : PCIe Device address read
bool ReadPciAddress(AMDTUInt32 bus,
                    AMDTUInt32 dev,
                    AMDTUInt32 func,
                    AMDTUInt32 reg,
                    AMDTUInt32* pData)
{
    ACCESS_PCI pci;
    ExtendedPciAddressSpace p;
    p.address = 0U;

    p.element.configEn = 1U;
    p.element.extRegNo = (reg >> 8) & 0xFU;
    p.element.regNo = reg & 0x00FCU;
    p.element.busNo = bus;
    p.element.device = dev;
    p.element.function = func;
    pci.address = p.address;
    pci.isReadAccess = true;
    AccessPciAddress(&pci);
    *pData = pci.data;
    return true;
}

// AccessPciAddress: Access PCI address
AMDTResult AccessPciAddress(PACCESS_PCI pData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                             IOCTL_ACCESS_PCI_DEVICE,
                             pData,
                             sizeof(ACCESS_PCI),
                             pData,
                             sizeof(ACCESS_PCI),
                             &ret);
    return ret;
}

// AccessMSRAddress: Access MSR address
AMDTResult AccessMSRAddress(PACCESS_MSR pData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                             IOCTL_ACCESS_MSR,
                             pData,
                             sizeof(ACCESS_MSR),
                             pData,
                             sizeof(ACCESS_MSR),
                             &ret);
    return ret;
}


// ReadMMIOSpace: Read from the MMIO space
AMDTResult ReadMMIOSpace(AMDTUInt64 address, AMDTUInt32* pData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    ACCESS_MMIO mmio;

    mmio.m_isReadAccess = true;
    mmio.m_addr = address;
    ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                             IOCTL_ACCESS_MMIO,
                             &mmio,
                             sizeof(ACCESS_MMIO),
                             &mmio,
                             sizeof(ACCESS_MMIO),
                             &ret);

    if (AMDT_STATUS_OK == ret)
    {
        *pData = mmio.m_data;
    }

    return ret;
}

// WriteMMIOSpace: write to the MMIO space
AMDTResult WriteMMIOSpace(AMDTUInt64 address, AMDTUInt32 data)
{
    AMDTResult ret = AMDT_STATUS_OK;
    ACCESS_MMIO mmio;

    mmio.m_isReadAccess = false;
    mmio.m_addr = address;
    mmio.m_data = data;

    ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                             IOCTL_ACCESS_MMIO,
                             &mmio,
                             sizeof(ACCESS_MMIO),
                             &mmio,
                             sizeof(ACCESS_MMIO),
                             &ret);
    return ret;
}

// GetSupportedTargetPlatformId: Provide the target system platform id
// if it is supported by power profiler. Otherwise return PLATFORM_INVALID
AMDTUInt32 GetSupportedTargetPlatformId()
{
    AMDTUInt32 family = 0;
    AMDTUInt32 model = 0;
    bool isAmd = 0;
    AMDTUInt32 idx = 0;

    GetCpuFamilyDetails(&family, &model, &isAmd);

    if ((0x15 == family) && (model >= 0x30 && model <= 0x3F))
    {
        // Kaveri : 0x15 30 to 3F
        idx = PLATFORM_KAVERI;
    }
    else if ((0x15 == family) && (model >= 0x60 && model <= 0x6F))
    {
        // Carrizo: 0x15 60 to 6F
        idx  = PLATFORM_CARRIZO;
    }
    else if ((0x16 == family) && (model >= 0x30 && model <= 0x3F))
    {
        // Mullins : 0x16 30 to 3F
        idx = PLATFORM_MULLINS;
    }
    else
    {
        idx = PLATFORM_INVALID;
    }

    return idx;
}

// getCpuid: Rea CPU instruction id
static AMDTResult getCpuid(AMDTUInt32 fn, AMDTInt32 cpuInfo[4])
{
#if ( defined (_WIN32) || defined (_WIN64) )

    __cpuid(cpuInfo, fn);

#else
    // Linux
#if defined(__i386__) && defined(__PIC__)
    /* %ebx may be the PIC register.  */

#define __cpuid(level, a, b, c, d)              \
    __asm__("xchgl\t%%ebx, %1\n\t"             \
            "cpuid\n\t"                 \
            "xchgl\t%%ebx, %1\n\t"              \
            : "=a" (a), "=r" (b), "=c" (c), "=d" (d)    \
            : "0" (level))
#else
#define __cpuid(level, a, b, c, d)          \
    __asm__("cpuid\n\t"                \
            : "=a" (a), "=b" (b), "=c" (c), "=d" (d)    \
            : "0" (level))
#endif

    AMDTUInt32  eax;
    AMDTUInt32  ebx;
    AMDTUInt32  ecx;
    AMDTUInt32  edx;

    /* CPUID Fn0000_0001_EAX Family, Model, Stepping */
    __cpuid(fn, eax, ebx, ecx, edx);

    cpuInfo[0] = eax;
    cpuInfo[1] = ebx;
    cpuInfo[2] = ecx;
    cpuInfo[3] = edx;
#endif

    return AMDT_STATUS_OK;
}

// IsCefSupported: Check if Core Effectiver Frequency is supported by the CPU
bool IsCefSupported()
{
#define CPUID_FnThermalAndPowerManagement 6
#define CPUID_FnThermalAndPowerManagement_ECX_EffFreq  (1 << 0)
#define ECX_OFFSET 2
    bool result = false;
    AMDTInt32 cpuInfo[4] = { -1 };

    getCpuid(CPUID_FnThermalAndPowerManagement, cpuInfo); // vendor information

    // Effective Frequency is supported
    result = (cpuInfo[ECX_OFFSET] & CPUID_FnThermalAndPowerManagement_ECX_EffFreq) != 0;
    return result;
}

// GetCpuFamilyDetails: Read Cpu family, model id and wheather it is a AMD platform from CPU id instruction
AMDTResult GetCpuFamilyDetails(AMDTUInt32* pFamily, AMDTUInt32* pModel, bool* pIsAmd)
{
    AMDTInt32 cpuInfo[4] = { -1 };
    char vendorId[20];
    bool isAmd = false;
    AMDTUInt32 family;
    AMDTUInt32 model;

    if (nullptr == pFamily || nullptr == pModel || nullptr == pIsAmd)
    {
        return AMDT_ERROR_INVALIDARG;
    }

    getCpuid(0, cpuInfo); // vendor information
    memcpy(vendorId, &cpuInfo[1], 4);
    memcpy(vendorId + 4, &cpuInfo[3], 4);
    memcpy(vendorId + 8, &cpuInfo[2], 4);
    vendorId[12] = '\0';

    if (0 == strcmp(vendorId, "AuthenticAMD"))
    {
        isAmd = true;
    }

    // read the family and model details
    memset(cpuInfo, 0, sizeof(cpuInfo));
    getCpuid(1, cpuInfo);

    // Family is an 8-bit value and is defined as:
    // Family[7:0] = ({0000b,BaseFamily[3:0]} + ExtFamily[7:0]).
    family = (cpuInfo[0] & CPU_BASE_FAMILY_MASK) >> 8;

    if (CPU_FAMILY_EXTENDED == family)
    {
        family += (cpuInfo[0] & CPU_EXT_FAMILY_MASK) >> 20;
    }

    // Model is an 8-bit value and is defined as:
    // Model[7:0] = {ExtModel[3:0], BaseModel[3:0]}.
    model = ((cpuInfo[0] & CPU_BASE_MODEL_MASK) >> 4) | ((cpuInfo[0] & CPU_EXT_MODEL_MASK) >> 12);

    *pFamily = family;
    *pModel = model;
    *pIsAmd = isAmd;

    return AMDT_STATUS_OK;
}

// GetBitsCount: Get the number of set bits
AMDTUInt32 GetBitsCount(AMDTUInt64 val)
{
    AMDTUInt32 cnt; // c accumulates the total bits set in v

    for (cnt = 0; val; cnt++)
    {
        val &= val - 1; // clear the least significant bit set
    }

    return cnt;
}

// GetFirstSetBitIndex: Get the index of first set bit
bool GetFirstSetBitIndex(AMDTUInt32* core_id, AMDTUInt32 mask)
{
#ifdef WIN32
    return _BitScanForward((unsigned long*)core_id, mask) ? 1 : 0;
#else
    *core_id = ffsl(mask) - 1;

    return *core_id ? 1 : 0;
#endif
}

//GetActiveCoreCount
uint32 GetActiveCoreCount()
{
    uint32 coreCnt = 0;
#define CPUID_FnSizeIdentifiers 0x80000008
    int aCPUInfo[4] = { -1 };
    getCpuid(CPUID_FnSizeIdentifiers, aCPUInfo);

    coreCnt = ((aCPUInfo[2] & 0xFF) + 1);

    return coreCnt;
}

#ifdef _PWR_BACKEND_TRACE_
FILE* pFile = nullptr;
// PowerTraceFlush: Dump trace buffer to the file
void PowerTraceFlush()
{
    if (nullptr != pFile)
    {
        fflush(pFile);
    }
}
// PowerTrace: This function will write the formated text to the trace file
void PowerTrace(const char* func, AMDTUInt32 line, const char* format, ...)
{
#define TRACE_FILE "PwrBackendTrace.txt"
    char lineBuffer[256];
    char preText[100];
    va_list args;

#ifdef LINUX
#define LINUX_TEMP "/tmp/"
    char filePath[TEMP_PATH_MAX_LEN];
    memset(filePath, '\0', TEMP_PATH_MAX_LEN);
    strncpy(&filePath[0], LINUX_TEMP, strlen(LINUX_TEMP));
#else
    char filePath[TEMP_PATH_MAX_LEN];
    memset(filePath, '\0', TEMP_PATH_MAX_LEN);
    GetTempPath(TEMP_PATH_MAX_LEN, filePath);
#endif

    va_start(args, format);
    vsnprintf(lineBuffer, 255, format, args);

    strncpy(&filePath[strlen(filePath)], TRACE_FILE, strlen(TRACE_FILE));

    if (nullptr == pFile)
    {
        // Create the file if it is not created yet
        pFile = fopen(filePath, "w");
    }

    sprintf(preText, "%s:%d- ", func, line);

    if (nullptr != pFile)
    {
        // If file is created write the trace message
        fwrite(preText, strlen(preText), sizeof(uint8), pFile);
        fwrite(lineBuffer, strlen(lineBuffer), sizeof(uint8), pFile);
        fflush(pFile);
    }

    va_end(args);
}
#endif

#if ( defined (_WIN32) || defined (_WIN64) )
#include <windows.h>
#include <lm.h>
#pragma comment(lib, "netapi32.lib")

bool GetWindowsVersion(DWORD& major, DWORD& minor)
{
    LPBYTE pinfoRawData;

    if (NERR_Success == NetWkstaGetInfo(nullptr, 100, &pinfoRawData))
    {
        WKSTA_INFO_100* pworkstationInfo = (WKSTA_INFO_100*)pinfoRawData;
        major = pworkstationInfo->wki100_ver_major;
        minor = pworkstationInfo->wki100_ver_minor;
        ::NetApiBufferFree(pinfoRawData);
        return true;
    }

    return false;
}
#endif

// DecodeTctlTemperature: Temperature reporting in Tctl scale
AMDTResult DecodeTctlTemperature(AMDTUInt32 raw, AMDTFloat32* pResult)
{
    //temp = 0;
    AMDTUInt32 data = 0;
    AMDTUInt32 tjsel = 0;
    AMDTFloat32  temp = 0;

    // read tclt
    tjsel = (raw >> 16) & 0x3;
    data = raw >> 21;

    if (0x3 != tjsel)
    {
        if (0 == data)
        {
            temp = 0;
        }
        else if (0x01 == data)
        {
            temp = 0.125;
        }
        else if ((data >= 0x02) && (data <= 0xFFE))
        {
            temp = AMDTFloat32(0.125 * (AMDTFloat32)data);
        }
        else if (0xFFF == data)
        {
            temp = 255.875;
        }
    }
    else
    {
        if (0x3 >= data)
        {
            temp = -49;
        }
        else if ((0x4 >= data) && (0x7 >= data))
        {
            temp = -48.5;
        }
        else if ((0x8 >= data) && (0x7FB >= data))
        {
            temp = (AMDTFloat32)((((data >> 2) & 0x3FF) * 0.5) - 49);
        }
        else if ((0x7FC >= data) && (0x7FF >= data))
        {
            temp = 206.5;
        }
    }

    *pResult = temp;
    return AMDT_STATUS_OK;
}

