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

#ifndef _PWRPROFIATTRIBUTES_H
#define _PWRPROFIATTRIBUTES_H
#include "AMDTDriverTypedefs.h"

// Driver version
/// \def DRIVER_VERSION The version format is:
/// [31-24]: pwr prof major, [23-16]: pwr prof minor, [15-0]: pwr prof build
#define DRIVER_VERSION 0x01000000
#define PCORE_MAJOR_VERSION  4
#define PCORE_MINOR_VERSION  11
#define PCORE_BUILD_VERSION  0
#define PWRPROF_MAJOR_VERSION  4
#define PWRPROF_MINOR_VERSION  0
#define PWRPROF_BUILD_VERSION  3

// Linux version
// Once version is changed backend and driver needs to be updated
// Major version should be changed after every release
// Minor version should be changed for any change in driver code
// Please update the version number in file located at ../Linux/CodeXLPwrProfVersion
#define LINUX_PWR_DRV_MAJOR 6
#define LINUX_PWR_DRV_MINOR 01

#define POWER_PROFILE_DRIVER_VERSION \
    DRIVER_VERSION | ((uint64)PCORE_MAJOR_VERSION << 56) | ((uint64)PCORE_MINOR_VERSION << 48) | ((uint64)PCORE_BUILD_VERSION << 32) \
                   | ((uint64)PWRPROF_MAJOR_VERSION << 28) | ((uint64)PWRPROF_MINOR_VERSION << 24) | ((uint64)PWRPROF_BUILD_VERSION << 20)

// Supported platform
#define PLATFORM_INVALID    0xFFFFFFUL
#define PLATFORM_KAVERI     0x15303FUL
#define PLATFORM_MULLINS    0x16303FUL
#define PLATFORM_CARRIZO    0x15606FUL
#define PLATFORM_ZEPPELIN   0x170000UL


#define BASIC_ATTRIBUTE_MASK 0xFF
#define SMU_ATTRIBUTE_MASK 0x000001fffffffff0ULL
#define PERCORE_ATTRIBUTE_MASK  (0x3FULL << COUNTERID_CORE_ATTR_BASE)
#define NONCORE_ATTRIBUTE_MASK  (0x3ULL << COUNTERID_NONCORE_ATTR_BASE)

#define DGPU_COUNTER_BASE_ID 64
#define DGPU_COUNTERS_MAX (10)
#define COUNTERID_MAX_CNT  (100)
#define PWR_MAX_MARKER_CNT (10)
#define PWR_INTERNAL_COUNTER_BASE (2 * sizeof(uint32) + PWR_MAX_MARKER_CNT * sizeof(MarkerTag))
#define APU_SMU_ID (1)
typedef enum PwrBasicCounterIds
{
    COUNTERID_SAMPLE_ID,
    COUNTERID_RECORD_ID,
    COUNTERID_SAMPLE_TIME,
    COUNTERID_BASIC_CNT
} PwrBasicCounterIds;

typedef enum PwrNodeCounterIds
{
    COUNTERID_PID,
    COUNTERID_TID,
    COUNTERID_CEF,
    COUNTERID_CSTATE_RES,
    COUNTERID_PSTATE,
    COUNTERID_SOFTWARE_PSTATE,
    COUNTERID_PERCORE_END = COUNTERID_SOFTWARE_PSTATE,
    COUNTERID_CORE_ENERGY,
    COUNTERID_NODE_TCTL_TEPERATURE,
    COUNTERID_SVI2_CORE_TELEMETRY,
    COUNTERID_SVI2_NB_TELEMETRY,
    COUNTERID_PKG_ENERGY,
    COUNTERID_NODE_MAX_CNT
} PwrNodeCounterIds;

#define PWR_PERCORE_COUNTER_MASK ~(~0ULL << (COUNTERID_PERCORE_END +1) )

typedef enum PwrSmu7CounterIds
{
    COUNTERID_SMU7_APU_PWR_CU,
    COUNTERID_SMU7_APU_TEMP_CU,
    COUNTERID_SMU7_APU_TEMP_MEAS_CU,
    COUNTERID_SMU7_APU_PWR_IGPU,
    COUNTERID_SMU7_APU_PWR_PCIE,
    COUNTERID_SMU7_APU_PWR_DDR,
    COUNTERID_SMU7_APU_PWR_DISPLAY,
    COUNTERID_SMU7_APU_PWR_PACKAGE,
    COUNTERID_SMU7_APU_TEMP_IGPU,
    COUNTERID_SMU7_APU_TEMP_MEAS_IGPU,
    COUNTERID_SMU7_APU_FREQ_IGPU,
    COUNTERID_SMU7_CNT
} PwrSmu7CounterIds;

typedef enum PwrSmu8CounterIds
{
    COUNTERID_SMU8_APU_PWR_CU,
    COUNTERID_SMU8_APU_TEMP_CU,
    COUNTERID_SMU8_APU_C0STATE_RES,
    COUNTERID_SMU8_APU_C1STATE_RES,
    COUNTERID_SMU8_APU_CC6_RES,
    COUNTERID_SMU8_APU_PWR_VDDGFX,
    COUNTERID_SMU8_APU_PWR_APU,
    COUNTERID_SMU8_APU_TEMP_VDDGFX,
    COUNTERID_SMU8_APU_FREQ_IGPU,
    COUNTERID_SMU8_APU_PWR_VDDIO,
    COUNTERID_SMU8_APU_PWR_VDDNB,
    COUNTERID_SMU8_APU_PWR_VDDP,
    COUNTERID_SMU8_APU_PWR_UVD,
    COUNTERID_SMU8_APU_PWR_VCE,
    COUNTERID_SMU8_APU_PWR_ACP,
    COUNTERID_SMU8_APU_PWR_UNB,
    COUNTERID_SMU8_APU_PWR_SMU,
    COUNTERID_SMU8_APU_PWR_ROC,
    COUNTERID_SMU8_APU_FREQ_ACLK,
    COUNTERID_SMU8_CNT
} PwrSmu8CounterIds;

#define SMU_IPVERSION_INVALID 255
#define SMU_IPVERSION_7_0     70
#define SMU_IPVERSION_7_1     71
#define SMU_IPVERSION_7_2     72
#define SMU_IPVERSION_7_5     75
#define SMU_IPVERSION_8_0     80
#define SMU_IPVERSION_8_1     81
#define SMU_IPVERSION_9_0     90
typedef enum
{
    COUNTERID_PKG_PWR_DGPU,
    COUNTERID_TEMP_MEAS_DGPU,
    COUNTERID_FREQ_DGPU,
    COUNTERID_VOLT_VDDC_LOAD_DGPU,
    COUNTERID_CURR_VDDC_DGPU,
    COUNTERID_DGPU_MAX_CNT
} AMDTPwrProfileDgpuAttribute;

///The maximum number of clients that PwrProf supports
#define MAX_CLIENT_COUNT 1

//  PwrProf's error codes
typedef enum
{
    /// An error occurred
    PROF_ERROR = -1,
    /// No errors occurred
    PROF_SUCCESS = 0x00,
    /// An argument was invalid
    PROF_INVALID_ARG = 0x01,
    /// Unable to create a file with that name
    PROF_INVALID_FILENAME_FORMAT = 0x03,
    /// Unable to write to the file
    PROF_FILE_WRITE_ERROR = 0x05,
    /// The configuration is not available
    PROF_INVALID_OPERATION = 0x06,
    /// Memory is not available for buffer allocation
    PROF_BUFFER_NOT_ALLOCATED = 0x08,
    /// Smu configuration error
    PROF_ERROR_SMU_CONGIGURATION = 0x10,
    /// Smu access failure error
    PROF_ERROR_SMU_ACCESS_FAILED = 0x11,
    /// The client application crashed during a profile
    PROF_CRITICAL_ERROR = 0xDEAD
} PWRPROF_ERROR_CODES;


/// These enumerated masks cover the possible range of states the driver is in
typedef enum
{
    /// The current client is not configured for anything
    STATE_NOT_CONFIGURED = 0x0000,
    /// The output file has been set for the next profile
    STATE_OUTPUT_FILE_SET = 0x0001,
    /// The call-stack sampling has been set for the next profile
    STATE_CSS_SET = 0x0002,
    /// The process id filter has been set for the next profile
    STATE_PID_FILTER_SET = 0x0004,
    /// At least one event configuration has been added for the next
    /// profile
    STATE_TBP_SET = 0x0008,
    /// The profile was started and is in process
    STATE_PROFILING = 0x0010,
    /// The profile was started and is currently paused
    STATE_PAUSED = 0x0020,
    /// The profile is currently stopping
    STATE_STOPPING = 0x0040
} PWRPROF_STATE;


typedef enum PMCEvents
{
    PMC_EVENT_CPU_CYCLE_NOT_HALTED,
    PMC_EVENT_RETIRED_MICRO_OPS,
    PMC_EVENT_MAX_CNT
} PMCEvents;

/// \struct OUTPUT_FILE_DESCRIPTOR Holds the file names for the next profile
typedef struct
{
    /// The registered client id
    uint32 ulClientId;
    /// Size of path\filename string
    uint32 ulPathSize;
    /// Pointer to path\filename of output raw file, a NULL-terminated WCHAR_T string
    uint64 uliPathName;
    /// The return status
    uint32 ulStatus;
} OUTPUT_FILE_DESCRIPTOR, *POUTPUT_FILE_DESCRIPTOR;


#define PROFILE_MASK_LEN (3)

// PROF_CONFIGS
typedef struct
{
    /// The registered client id
    uint32 ulClientId;
    /// number of configurations
    uint32 ulConfigCnt;
    /// list of configurations
    uint64 uliProfileConfigs;
    /// The return status
    uint32 ulStatus;
} PROF_CONFIGS, *PPROF_CONFIGS;



/// Profiling modes
typedef enum
{
    /// Offline profiling mode
    PROF_MODE_OFFLINE   = 0x0000,
    /// Online profiling mode
    PROF_MODE_ONLINE    = 0x0001,
} PROF_MODE;

/// \struct PROFILER_PROPERTIES Holds information about the current profile
typedef struct
{
    /// The registered client id
    uint32 ulClientId;
    /// The profiling mode
    uint32 ulProfileMode;
    /// A handle to the event created with CreateEvent for notification if the
    /// profile has to abort, for example: running out of disk space to write
    uint64 hAbort;
    /// The return status
    uint32 ulStatus;
} PROFILER_PROPERTIES, *PPROFILER_PROPERTIES;


/// \struct ACCESS_PCI to access Pci device directly from user space
typedef struct
{
    /// Access mode- read /write
    bool isReadAccess;
    uint32   address;
    uint32   data;
} ACCESS_PCI, *PACCESS_PCI;

/// \struct ACCESS_MSR to access MSR directly from user space
typedef struct
{
    /// Access mode- read /write
    bool isReadAccess;
    uint32   regId;
    uint32   data;
} ACCESS_MSR, *PACCESS_MSR;

/// \struct ACCESS_MMIO to access Memory Mapped space
typedef struct
{
    /// Access mode- read /write
    uint64   m_addr;
    uint32   m_isReadAccess;
    uint32   m_data;
    uint32   m_status;
} ACCESS_MMIO, *PACCESS_MMIO;

/// \struct FILE_HEADER
/// Mainly used for online profiling mode
typedef struct
{
    /// The registered client id
    uint32 ulClientId;
    /// Buffer id, used to identify requested buffer in case header spans
    /// in more than 1 buffer, first buffer id is 0 and so on
    uint32 ulBufferId;
    /// Buffer will contain file header data, the size of buffer
    /// should be >= FILE_HEADER_BUFFER_SIZE
    uint64 uliBuffer;
    /// Number of buffers containing file header
    uint32 ulNoOfBuffer;
    /// The return status
    uint32 ulStatus;
} FILE_HEADER, *PFILE_HEADER;

/// \struct DATA_BUFFERS
/// Mainly used for online profiling mode
typedef struct
{
    /// The registered client id
    uint32 ulClientId;
    /// Buffer id, used to identify requested buffer in case sample data spans
    /// in more than 1 buffer, latest buffer id is 0 then -1, -2...so on
    int32 lBufferId;
    /// Buffer will contain sample data, the size of buffer
    /// should be >= DATA_PAGE_BUFFER_SIZE
    uint64 uliBuffer;
    /// Start offset of the data collected after last call
    uint32 ulStartOffset;
    /// Byte to be consumed after the start offset
    uint32 ulByteToBeConsumed;
    /// Sample buffer collected since last call
    uint32 ulavailableBuffCnt;
    /// The return status
    uint32 ulStatus;
} DATA_BUFFER, *PDATA_BUFFER;

// Common function implementation between driver and backend
// These functions are inteded to have same logic across driver and backend
// to avoid different logic and maintainace purpose

// DecodeCURegisterStatus: Decode the number of compute units and number of active cores
// in that compute unit.
#define AMDT_CU_STATUS1 0x00010001
#define AMDT_CU_STATUS2 0x00030003
#define AMDT_CU_STATUS3 0x00010000
#define AMDT_CU_STATUS4 0x00030000

// PState frequency masks
#define AMDT_CPUFID_MASK        0x3FULL
#define AMDT_CPUDID_MASK        0x1C0ULL
#define AMDT_CPUDID_BITSHIFT    6

// PState
#define AMDT_PSTATE_BASE_REGISTER 0xC0010064


// Page Buffer
// Buffer to hold the profile and header data
// Common structure between driver and backend
typedef struct PageBuffer
{
    uint64   m_recCnt;
    atomic   m_currentOffset;
    atomic   m_consumedOffset;
    union
    {
        uint8*   m_pBuffer;
        uint64   m_dummy;
    };
    atomic   m_maxValidOffset;
    uint32   m_fill;
} PageBuffer;


/*
1h 1h -1 compute unit is enabled; both cores of the compute unit are enabled.\
3h 3h -2 compute units are enabled; both cores of each compute unit are enabled.\
1h 0h -1 compute unit is enabled; core 0 of the compute unit is enabled; core 1 of the\
compute unit is disabled.\
3h 0h 2 compute units are enabled; core 0 of each compute unit is enabled; core 1 of\
each compute unit is disabled.\
*/
#define DecodeCURegisterStatus(reg, pCuCnt, pCorePerCu)\
    if(reg == (reg & AMDT_CU_STATUS1)){*pCuCnt = 1; *pCorePerCu = 2;}\
    else if (reg == (reg & AMDT_CU_STATUS2)){*pCuCnt = 2;*pCorePerCu = 2;}\
    else if (reg == (reg & AMDT_CU_STATUS3)) { *pCuCnt = 1;*pCorePerCu = 1;}\
    else if (reg == (reg & AMDT_CU_STATUS4)) {*pCuCnt = 2; *pCorePerCu = 1; }

// ExtendedPciAddrSpace: PCI config address encoder
typedef union _ExtendedPciAddrSpace
{
    // \brief The elements that make up a PCI address in PCI config space
    struct
    {
        // base register address to access
        uint32 regNo : 8;
        // function number to access
        uint32 function : 3;
        // device number to access
        uint32 device : 5;
        // bus number to access
        uint32 busNo : 8;
        // extended register number to access
        uint32 extRegNo : 4;
        // reserved, must be 0
        uint32 reserved : 3;
        // Configuration space enable, 1 = IO read and write accesses to
        // IOCFC are translated into configuration cycles at the
        // configuration address
        uint32 configEn : 1;
    } element;
    // The equivalent IO-space configuration address made up of each \ref Element
    uint32 address;
} ExtendedPciAddrSpace;

// EncodeExtendedConfigSpaceAddress: Encode PCI address to extended PCI config speace.
#define GET_EXTENED_PCICS_ADDRESS(b, d, f, r, out) {\
        _ExtendedPciAddrSpace p;\
        p.address = 0U;\
        p.element.configEn = 1U;\
        p.element.extRegNo = (r >> 8) & 0xFU;\
        p.element.regNo = r & 0x00FCU;\
        p.element.busNo = b;\
        p.element.device = d;\
        p.element.function = f;\
        out = p.address;   }

#endif //_PWRPROFIATTRIBUTES_H

