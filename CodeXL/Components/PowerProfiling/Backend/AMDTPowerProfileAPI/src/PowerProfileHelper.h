//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileHelper.h
///
//==================================================================================

#ifndef _POWERPROFILEHELPER_H_
#define _POWERPROFILEHELPER_H_
#include <AMDTDefinitions.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTRawDataFileHeader.h>

#define _PWR_BACKEND_TRACE_
#define TEMP_PATH_MAX_LEN       260
//SMU Limit registers
#define SMU_INDEX_ADDR        0x800000B8
#define SMU_INDEX_DATA        0x800000BC

#define CU0_PCALC_MAX 0x3FE54
#define CU1_PCALC_MAX 0x3FE84

// definitions for CPU ID instruction
// Macros
#define CPU_FAMILY_EXTENDED    0xF

#define CPU_STEPPING_MASK       0x0000000F
#define CPU_BASE_MODEL_MASK     0x000000F0
#define CPU_BASE_FAMILY_MASK    0x00000F00
#define CPU_EXT_FAMILY_MASK     0x0FF00000
#define CPU_EXT_MODEL_MASK      0x000F0000
typedef union _ExtendedPciAddressSpace
{
    // \brief The elements that make up a PCI address in PCI config space
    struct
    {
        // base register address to access
        unsigned int regNo    : 8;
        // function number to access
        unsigned int function : 3;
        // device number to access
        unsigned int device   : 5;
        // bus number to access
        unsigned int busNo    : 8;
        // extended register number to access
        unsigned int extRegNo : 4;
        // reserved, must be 0
        unsigned int reserved : 3;
        // Configuration space enable, 1 = IO read and write accesses to
        // IOCFC are translated into configuration cycles at the
        // configuration address
        unsigned int configEn : 1;
    } element;
    // The equivalent IO-space configuration address made up of each \ref Element
    AMDTUInt32 address;
} ExtendedPciAddressSpace;


extern AMDTPwrProfileAttributeList g_attributeList;

AMDTResult SetRawAttributeMask(AMDTUInt16 attributeId, AMDTUInt64* pEventMask);

// GetSupportedTargetPlatformId: Provide the target system platform id
// if it is supported by power profiler. Otherwise return PLATFORM_INVALID
AMDTUInt32 GetSupportedTargetPlatformId();

// AccessMSRAddress: Access MSR address
AMDTResult AccessMSRAddress(PACCESS_MSR pData);

// ReadPciAddress : PCIe Device address read
bool ReadPciAddress(AMDTUInt32 bus,
                    AMDTUInt32 dev,
                    AMDTUInt32 func,
                    AMDTUInt32 reg,
                    AMDTUInt32* pData);

// AccessPciAddress: Extended/ decoded PCI address
AMDTResult AccessPciAddress(PACCESS_PCI pData);

// ReadMMIOSpace: Read from the MMIO space
AMDTResult ReadMMIOSpace(AMDTUInt64 address, AMDTUInt32* pData);


// WriteMMIOSpace: write to the MMIO space
AMDTResult WriteMMIOSpace(uint64 address, AMDTUInt32 data);


// GetBitsCount: Get the number of set bits
uint32 GetBitsCount(AMDTUInt64 val);

// GetFirstSetBitIndex: Get the index of first set bit
bool GetFirstSetBitIndex(AMDTUInt32* core_id, AMDTUInt32 mask);


// GetActiveCoreCount: Provides number of active cores
uint32 GetActiveCoreCount();

// IsCefSupported: Check if core effective frequency is supported by the platform
bool IsCefSupported();

// GetCpuFamilyDetails:
AMDTResult GetCpuFamilyDetails(AMDTUInt32* pFamily, AMDTUInt32* pModel, bool* pIsAmd);

#ifdef _PWR_BACKEND_TRACE_
    // PwrTrace: Dumps trace message to a file
    void PowerTrace(const char* func, AMDTUInt32 line, const char* format, ...);
    #ifdef LINUX
        #define PwrTrace(fmt, ...)    PowerTrace(__FUNCTION__, __LINE__, fmt "\n", ##__VA_ARGS__)
    #else
        #define PwrTrace(fmt, ...)    PowerTrace(__FUNCTION__, __LINE__, fmt "\n", __VA_ARGS__)
    #endif
    void PowerTraceFlush(void);
    #define PwrTraceFlush() PowerTraceFlush()
#else
    #define PwrTrace(fmt, ...)
#endif

#if ( defined (_WIN32) || defined (_WIN64) )
    bool GetWindowsVersion(DWORD& major, DWORD& minor);
#endif

// DecodeTctlTemperature: Temperature reporting in Tctl scale
AMDTResult DecodeTctlTemperature(AMDTUInt32 raw, AMDTFloat32* pResult);
#ifdef LINUX
    #include <sys/time.h>
    #include <unistd.h>
    #define SLEEP_IN_MS(x) usleep(x*1000)
#else
    #define SLEEP_IN_MS(x) Sleep(x)
#endif

#endif //_POWERPROFILEHELPER_H_

