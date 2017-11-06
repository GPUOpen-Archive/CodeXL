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

#ifndef _RAW_DATA_FILE_HEADER_H_
#define _RAW_DATA_FILE_HEADER_H_
#include <AMDTPwrProfAttributes.h>
#include <AMDTDriverTypedefs.h>
#include <AMDTSmu8Interface.h>
#include <AMDTSmu7Interface.h>
#include <AMDTAccessPmcData.h>
/****************************************************************************/

// A typical raw profile data file will be like  following
//
//   +-------------------------------------+
//   |   Item                                    |  size |offset|
//   +------------------------- +---- +---- +
//   |  RawFileHeader                       | 56    | 0     |
//   +------------------------- +---- +---- +
//   |  SectionHrdTableInfo               | 8      | 48   |
//   +--------------------------+ ----+----+
//   |  SectionHdrInfo-cpu info          | 24    | 80   |
//   +--------------------------+---- +----+
//   |  SectionHdrInfo-run info          | 24    | 104  |
//   +--------------------------+---- +----+
//   |  SectionHdrInfo-topology         | 24    | 128  |
//   +--------------------------+---- +----+
//   |  SectionHdrInfo-sample config  | 24    | 152  |
//   +--------------------------+---- +----+
//   |  SectionHdrInfo-sample Info    | 24    | 176  |
//   +--------------------------+---- +----+
//   |  SectionHdrInfo-Ti info            | 24    | 200  |
//   +------------------------- +---- +---- +
//   |  SectionRunInfo                      | 784  | 984   |
//   +------------------------- +---- +---- +
//   |  SectionCpuInfo                      | 32    | 1016 |
//   +------------------------- +---- +---- +
//   |  SectionCpuTopologyInfo         | 16    | 1032 |
//   +------------------------- +---- +---- +
//   |  RawProfileConfigTable            | 172  | 1204 |
//   +------------------------- +---- +---- +
//   |  SectionSampleInfo                 | 24    | 1228 |
//   +------------------------- +---- +----  +
//   |  SectionTiInfo                         | 24    | 1252 |
//   +------------------------- +---- +---- +
//   |  Chunk_marker HDR               | 8      | 1260 |
//   +------------------------- +---- +---- +
//   |  Chunk marker data                | 24    | 1284 |
//   +------------------------- +---- +---- +
//   |  RawRecordHdr                       | 8      | 1292 |
//   +------------------------- +---- +---- +
//   |  Raw-data                              |Variable size |
//   +------------------------- +---------- +

// Error code for power profile API access
typedef enum
{
    RAW_STATUS_ERROR = -1,
    RAW_STATUS_SUCCESS = 0,
    RAW_READ_ERROR = 1,
    RAW_WRITE_ERROR = 2,
    RAW_UNKNOWN_ERROR = 3,
} RAW_Status;

#define RAWFILE_MAGIC 0
#define MAJOR_VERSION 3
#define MINOR_VERSION 3
#define MICRO_VERSION 1
#define PROFILE_VERSION (MAJOR_VERSION << 24)|(MINOR_VERSION << 16)| MICRO_VERSION
#define RAW_FILE_VERSION 1
#define SECTION_HDR_CNT 1
#define MAX_SECTION_HDR_CNT 12
#define MAX_POWER_CFG 10
#define FILE_BUFFER_SIZE (4*1024)

#define MAX_SAMPLE_ATTRIBUTE 20 //will this be sufficient
#define MAX_RAW_DATA_SIZE (MAX_SAMPLE_ATTRIBUTE * sizeof(uint64))
#define PWR_MARKER_BUFFER_SIZE 32
#define CONTEXT_RECORD_LEN sizeof(RawRecordHdr) + sizeof(uint64) + sizeof(ContextData)
#define MARKER_RECORD_LEN (sizeof(RawRecordHdr) + sizeof(MarkerTag))

// Maker states
#define PWR_MARKER_DISABLE 0
#define PWR_MARKER_ENABLE 1
#define PWR_MARKER_ENABLE_INITIATED 2
#define PWR_MARKER_DISABLE_INITIATED 3

// SectionHrdTableInfo
//
// This structure holds the starting address and size of each section
// Currently there is only one section table.
// There could be more than one section header table in future.
typedef struct SectionHrdTableInfo
{
    uint32   m_sectionTabOff;  // offset of section header table in file;
    uint32   m_sectionTabSize; // size of the section header table in file;
} SectionHrdTableInfo;

// RawFileHeader
//
// We can also use MAJOR and MINOR VERSION NUMBER - 16 bits each and
// consturct a 32-bit VERSION from these MAJOR and MINOR version numbers
#define RAW_FILE_VERSION      1

#define MAX_SECTION_HDR_TABLE 2
typedef struct RawFileHeader
{
    uint64   m_magicNum;
    uint32   m_versionNum;
    uint32   m_rawFileVersion;
    uint16   m_sectionTabCnt;
    uint16   m_targetCoreCuCnt;
    uint32   m_rawDataOffset;
    uint32   m_family;
    uint32   m_model;
    uint64   m_rawRecordCount;
    uint64   m_sessionStart;
    uint64   m_sessionEnd;
    uint64   m_startPerfCounter;
    uint64   m_perfFreq;
    SectionHrdTableInfo m_tabInfo[1];
} RawFileHeader;

// SectionType
//
// Not all sections are mandatory...
// Following sections are optional
//    1.RAW_FILE_SECTION_RUN_INFO
//    2.RAW_FILE_SECTION_CPU_INFO
//    3.RAW_FILE_SECTION_CPU_TOPOLOGY
// For CPU profile following could be necessary
//      1.Sampling mode requires - EVENT_ATTRIBUTE, SAMPLE_ID & SAMPLE_DATA sections
//      2.Counting mode requires - EVENT_ATTRIBUTE & COUNTER_DATA sections
//      3.Sampling and Sampling mode requires -
//        EVENT_ATTRIBUTE, SAMPLE_ID, SAMPLE_DATA & COUNTER_DATA sections
//  For Power profile following could be necessary
//        RAW_FILE_SECTION_POWER_CONFIG, SAMPLE_ID, SAMPLE_DATA sections

#define RAW_FILE_SECTION_RUN_INFO (1 << 0)
#define RAW_FILE_SECTION_CPU_INFO (1 << 1)
#define RAW_FILE_SECTION_CPU_TOPOLOGY (1<<2)
#define RAW_FILE_SECTION_SAMPLE_CONFIG (1<<3)
#define RAW_FILE_SECTION_SAMPLE_REC_INFO (1<<4)
#define RAW_FILE_SECTION_TI_REC_INFO (1<<5)
#define RAW_FILE_SECTION_MAX ((uint32)1<<31)

//SectionHdrInfo
//SectionType could be one of SectionType
typedef struct SectionHdrInfo
{
    uint32   m_sectionType;
    uint32   m_misc;            // _UNUSED_ field
    uint64   m_sectionOffset;
    uint64   m_sectionSize;
} SectionHdrInfo;

// SectionRunInfo Section :Optional
//
//   contains details about the machine, processor, OS
//   name/path of the application launched
//   start and end time of the profile
//
//
//
#define MAX_NAME_SIZE    64
typedef struct SectionRunInfo
{
    uint64   m_startTime;
    uint64   m_endTime;
    // This can contain null terminated 64 byte length strings (65 chars)
    wchar_t   m_sysName[MAX_NAME_SIZE];    // OS implementation - Linux
    wchar_t   m_releases[MAX_NAME_SIZE];   // Release - 2.6.38-8-generic
    wchar_t   m_version[MAX_NAME_SIZE];    // version - Ubuntu
    wchar_t   m_nodeName[MAX_NAME_SIZE];   // machine name - capike01
    wchar_t   m_machine[MAX_NAME_SIZE];    // hw type - x86_64
    wchar_t   m_fileName[MAX_NAME_SIZE];   // launched application - should be the last field
} SectionRunInfo;

//SectionCpuInfo: This is an optional section to hold cpu details.
//
// TBD: Do we need vendor info ?
//
#define MAX_CPUID_VALUE    4

typedef struct SectionCpuInfo
{
    uint32     m_function;
    uint32     m_value[MAX_CPUID_VALUE]; // when CA_CPUID_VALUE_MAX gets modified, ensure
    uint32     m_filler[3];
} SectionCpuInfo;

//SectionCpuTopologyInfo: This is an optional section to hold cpu topology info.
typedef struct SectionCpuTopologyInfo
{
    uint32     m_coreId;
    uint32     m_processorId;
    uint32     m_numaNodId;
    uint32     m_filler;
} SectionCpuTopologyInfo;

//SectionSampleInfo: This structure define the first record chunck offset.
typedef struct SectionSampleInfo
{
    uint64   m_firstChunkOffset;
    uint64   m_recordCount;
    uint64   m_size;
} SectionSampleInfo;

//SectionTiInfo: This structure define the first TI chunck offset.
typedef struct SectionTiInfo
{
    uint64  m_firstChunkOffset;
    uint64  m_recordCount;
    uint64  m_size;
} SectionTiInfo;

//Chunk type: describe the types of records followed by chunk marker
typedef enum
{
    CHUNK_TYPE_SAMPLE,
    CHUNK_TYPE_TI
} ChunkType;

// ProfileRecordType
//
// Type of the profile
typedef enum
{
    REC_TYPE_INVALID = 0,
    REC_TYPE_SAMPLE_DATA,
    REC_TYPE_CHUNK_MARKER,
    REC_TYPE_CONTEXT_DATA,
    REC_TYPE_MARKER_DATA,
    REC_TYPE_MAX_CNT
} ProfileRecordType;

// RawRecordHrd
//
// RawRecordHrd holds the information about raw record followed
// by this header
typedef struct RawRecordHdr
{
    uint16  m_recordType;
    uint16  m_recordLen;
    uint32  m_fill;
} RawRecordHdr;

typedef enum
{
    PROFILE_TYPE_TIMELINE,
    PROFILE_TYPE_PROCESS_PROFILING,
    PROFILE_TYPE_SOURCE_CODE_PROFILING
} ProfileType;

// RawSamplingSpec
typedef struct SamplingSpec
{
    uint32 m_profileType;
    uint32 m_maskCnt;
    uint64 m_mask;
    uint64 m_samplingPeriod;     // in milli seconds
} SamplingSpec;

#define PLATFORM_SMU_CNT (5)
/// Function pointers to initialize SMU
typedef bool (*INIT_SMU_CB)(void* pSmu);

/// Function pointers to Close SMU
typedef bool (*CLOSE_SMU_CB)(void* pSmu);

/// Function pointers to read SMU values
typedef bool (*COLLECT_SMU_CB)(void* pSmu, uint8* pData, uint32* pLength);
/// Function pointers to read SMU values

// Smu9Interface: Access interface for Smu9
typedef struct Smu9Interface
{
    uint32 m_nbSmnIndex;
    uint32 m_nbSmnData;
    uint32 m_testMsgId;
    uint32 m_testMsgArg;
    uint32 m_testMsgResp;
    uint32 m_testMsgPmLogStartId;
    uint32 m_testMsgPmLogReadId;
    uint32 m_testMsgAgmTableVersionId;
    uint32 m_testMsgDramHigh;
    uint32 m_testMsgDramLow;
} Smu9Interface;

typedef struct SmuAccessCb
{
    INIT_SMU_CB    fnSmuInit;
    CLOSE_SMU_CB   fnSmuClose;
    COLLECT_SMU_CB fnSmuReadCb;
} SmuAccessCb;

typedef struct SmuAccess
{
    union
    {
        Smu9Interface m_smu9;
        Smu8Interface m_smu8;
        Smu7Interface m_smu7;
    };
    uint64 m_accessCb;
} SmuAccess;

typedef struct SmuInfo
{
    uint32      m_isAccessible;
    uint32      m_fill;
    uint32      m_packageId;
    uint32      m_smuIpVersion;
    uint64      m_gpuBaseAddress;
    uint64      m_counterMask;
    SmuAccess   m_access;
} SmuInfo;

typedef struct SmuList
{
    uint32  m_count;
    uint32  m_fill;
    SmuInfo m_info[PLATFORM_SMU_CNT];
} SmuList;

// RawPowerConfig
//
// A power session configured for any event type from RAW_PowerEventType.
// Again, for each event type, user can configure one or more than one attributes
// to collect data during the data acquisition.
// For e.g.  if user is interested in RAW_TYPE_HW_PWR and interested in only
// CPU, GPU, and Memory attributes, then following could be the possible mask
// m_eventAttributeMask =  RAW_SMU_POWER_CPU|RAW_SMU_POWER_GPU|RAW_SMU_POWER_MEM
// rawRecLen will calculate the length of the raw sample record for each configuration time
// This format will also support user defined user spac.
typedef struct ProfileConfig
{
    uint16            m_sampleId;
    uint16            m_attrCnt;
    uint32            m_fill;
    SamplingSpec      m_samplingSpec;
    uint64            m_apuCounterMask; // Only node counters
    SmuList           m_activeList;     // m_info[0] will have APU smu
} ProfileConfig;

// RawPowerConfigTable
//
// Table to describe more than one configuration for a profile session
// m_configCnt provides the number of configuration used for the profile session
typedef struct ProfileConfigList
{
    uint32            m_configCnt;
    ProfileConfig* m_profileConfig;
} ProfileConfigList;


//RawBufferInfo
//
//This structure holds the information regarding the buffer comming from driver
typedef struct RawBufferInfo
{
#ifndef _DBG_TOOL_
    ULARGE_INTEGER uliBuffer;
#else
    ULARGE_INT     uliBudder;
#endif
    uint32         ulvalidLength;
} RawBufferInfo;

// Context Data
// Os context data for every sample
typedef struct ContextData
{
    uint32     m_processId;
    uint32     m_threadId;
    uint64     m_timeStamp;
    uint64     m_ip;
    uint32     m_isKernel;
    uint32     m_fill;
    uint64     m_pmcData[PMC_EVENT_MAX_CNT];
} ContextData;

// MarkerTag Data
// Start and stop marker tag
typedef struct MarkerTag
{
    uint32     m_markerId;
    uint32     m_pid;
    uint32     m_state;
    uint32     m_fill;
    uint64     m_ts;
    uint8      m_name[PWR_MARKER_BUFFER_SIZE];
    uint8      m_userBuffer[PWR_MARKER_BUFFER_SIZE];
} MarkerTag;

// PwrInternalAddr: Internal counters PCI address
typedef struct PwrInternalAddr
{
    uint32 m_cstateRes;
    uint32 m_sviTelemetry;
    uint32 m_sviNBTelemetry;
    uint32 m_timingControl4;
    uint32 m_timingControl6;
    uint32 m_coreEnergyMsr;
    uint32 m_packageEnergyMsr;
} PwrInternalAddr;

#endif  // _RAW_DATA_FILE_HEADER_H_
