//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief Common Data Types used by DB Adapter and Profiler Report layers
//
//=============================================================

#ifndef _AMDTCOMMONPROFILEDATATYPES_H_
#define _AMDTCOMMONPROFILEDATATYPES_H_

// Base headers
#include <AMDTCommonHeaders/AMDTDefinitions.h>
#include <AMDTCommonHeaders/AMDTCommonDataTypes.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

//
//      Macros
//

#define AMDT_PROFILE_MAX_VALUE              0xFFFFFFFFUL
#define AMDT_PROFILE_HW_EVENT_ID_MAX        AMDT_PROFILE_MAX_VALUE
#define AMDT_PROFILE_ALL_PROCESSES          AMDT_PROFILE_MAX_VALUE
#define AMDT_PROFILE_ALL_MODULES            AMDT_PROFILE_MAX_VALUE
#define AMDT_PROFILE_ALL_THREADS            AMDT_PROFILE_MAX_VALUE
#define AMDT_PROFILE_ALL_COUNTERS           AMDT_PROFILE_MAX_VALUE
#define AMDT_PROFILE_ALL_CORES              AMDT_PROFILE_MAX_VALUE
#define AMDT_PROFILE_MAX_COUNT              AMDT_PROFILE_MAX_VALUE

#define AMDT_PROFILE_INVALID_ADDR           ULLONG_MAX

#define UNKNOWN_FUNCTION_ID                 0
#define UNKNOWN_MODULE_ID                   0

typedef AMDTUInt32       AMDTModuleId;
typedef AMDTUInt32       AMDTProcessId;
typedef AMDTUInt32       AMDTThreadId;
typedef AMDTUInt32       AMDTCounterId;
typedef AMDTUInt32       AMDTFunctionId;

//
//  Enums
//

enum AMDTProfileMode
{
    AMDT_PROFILE_MODE_NONE        = 0,
    AMDT_PROFILE_MODE_TIMELINE    = 1,
    AMDT_PROFILE_MODE_AGGREGATION = 2,
};

enum AMDTProfileDataType
{
    AMDT_PROFILE_DATA_PROCESS  = 1,
    AMDT_PROFILE_DATA_THREAD   = 2,
    AMDT_PROFILE_DATA_MODULE   = 3,
    AMDT_PROFILE_DATA_FUNCTION = 4,
};

enum AMDTProfileCounterType
{
    AMDT_PROFILE_COUNTER_TYPE_RAW      = 1,
    AMDT_PROFILE_COUNTER_TYPE_COMPUTED = 2,
};

enum AMDTProfileCounterUnit
{
    AMDT_PROFILE_COUNTER_UNIT_COUNT   = 1,
    AMDT_PROFILE_COUNTER_UNIT_RATIO   = 2,
    AMDT_PROFILE_COUNTER_UNIT_PERCENT = 3,
};

enum AMDTModuleType
{
    AMDT_MODULE_TYPE_NONE           = 0,
    AMDT_MODULE_TYPE_NATIVE         = 1,
    AMDT_MODULE_TYPE_JAVA           = 2,
    AMDT_MODULE_TYPE_MANAGEDDPE     = 3,
    AMDT_MODULE_TYPE_OCL            = 4,
    AMDT_MODULE_TYPE_UNKNOWN        = 5,
    AMDT_MODULE_TYPE_UNKNOWN_KERNEL = 6,
};

enum AMDTReportOptionType
{
    AMDT_REPORT_OPTION_COREMASK                = 1,
    AMDT_REPORT_OPTION_AGGREGATE_BY_CORE       = 2,
    AMDT_REPORT_OPTION_AGGREGATE_BY_NUMA       = 3,
    AMDT_REPORT_OPTION_IGNORE_SYSTEM_MODULE    = 4,
    AMDT_REPORT_OPTION_SORT_PROFILE_DATA       = 5,    // TBD: Is this required ?
    AMDT_REPORT_OPTION_OTHERS_ENTRY_IN_SUMMARY = 6,
    AMDT_REPORT_OPTION_SUMMARY_COUNT           = 7,
    AMDT_REPORT_OPTION_MAX_BYTES_TO_DISASM     = 8,
};

//
//  Structs
//

struct AMDTProfileDevice
{
    AMDTUInt32              m_deviceId;
    AMDTUInt32              m_deviceType;
    gtString                m_deviceTypeStr;
    gtString                m_name;
    gtString                m_description;
    gtVector<AMDTUInt32>    m_subDeviceIds;
};

struct AMDTCpuTopology
{
    AMDTUInt32  m_coreId      = AMDT_PROFILE_ALL_CORES;
    AMDTUInt16  m_processorId = 0;
    AMDTUInt16  m_numaNodeId  = 0;

    AMDTCpuTopology(AMDTUInt32 coreId, AMDTUInt16 processorId, AMDTUInt16 numaNodeId) :
        m_coreId(coreId), m_processorId(processorId), m_numaNodeId(numaNodeId) {}
};

struct AMDTProfileSystemTime
{
    AMDTUInt64   m_second = 0;
    AMDTUInt64   m_microSecond = 0;
};

struct AMDTProfileSamplingConfig
{
    AMDTUInt32      m_id;                   // samplingConfigurationId
    AMDTUInt32      m_hwEventId;            // actual HW PMC event id
    AMDTUInt64      m_samplingInterval;
    AMDTInt8        m_unitMask;
    bool            m_userMode;
    bool            m_osMode;
};

struct AMDTProfileCounterDesc
{
    AMDTUInt32              m_id;           // samplingConfigurationId

    AMDTUInt32              m_hwEventId;    // HW PMC event id for RAW counters or AMDT_PROFILE_ALL_COUNTERS for COMPUTED counters
    AMDTUInt32              m_deviceId;

    gtString                m_name;
    gtString                m_description;
    AMDTUInt32              m_type;         // AMDTProfileCounterType
    AMDTUInt32              m_category;     // UNUSED for CPU Profiler. For Power profiler it is Power/Frequency/Temperature
    AMDTUInt32              m_unit;         // AMDTProfileCounterUnit


    // these are required due to old pp table design
    gtString                m_typeStr;
    gtString                m_categoryStr;
    gtString                m_unitStr;
};
using AMDTProfileCounterDescVec = gtVector<AMDTProfileCounterDesc>;

struct AMDTProfileDataOptions
{
    gtVector<AMDTUInt32>    m_counters;     // counters on which the queries will be selected
    gtUInt64                m_coreMask = 0;
    bool                    m_isSeperateByCore = false;
    bool                    m_isSeperateByNuma = false;
    bool                    m_doSort = true;
    bool                    m_ignoreSystemModules = true;
    bool                    m_othersEntryInSummary = true;
    gtUInt16                m_summaryCount = 5;
    gtUInt16                m_maxBytesToDisassemble = 2048; // bytes

    void Clear(void)
    {
        m_counters.clear();
        m_coreMask = 0;
        m_isSeperateByCore = false;
        m_isSeperateByNuma = false;
        m_doSort = false;
        m_ignoreSystemModules = false;
        m_othersEntryInSummary = false;
        m_summaryCount = 0;
        m_maxBytesToDisassemble = 0;
    };
};

// From a session profile data various reports can be generated using the sampled counters and the calculated counter like
//      - all data (raw counters)
//      - ipc assessment (cpu-ccyles, retured-microops, cpi, ipc)
struct AMDTProfileReportConfig
{
    gtString                    m_name;

    // Counters that are supported for this CPU Profile Data View Configuration
    AMDTProfileCounterDescVec   m_counterDescs;
};

struct AMDTProfileModuleInfo
{
    AMDTModuleId        m_moduleId = 0;
    AMDTModuleType      m_type = AMDT_MODULE_TYPE_NONE;
    gtString            m_name;
    gtString            m_path;
    AMDTUInt64          m_loadAddress = 0;
    AMDTUInt32          m_size = 0;
    AMDTUInt64          m_checksum = 0;
    bool                m_is64Bit = false;
    bool                m_isSystemModule = false;
    bool                m_foundDebugInfo = false;
};

struct AMDTProfileThreadInfo
{
    AMDTThreadId        m_threadId;
    AMDTProcessId       m_pid;
    gtString            m_name;
    AMDTUInt64          m_startTime;
    AMDTUInt64          m_endTime;
};

using AMDTProfileModuleInfoVec = gtVector<AMDTProfileModuleInfo>;
using AMDTProfileThreadInfoVec = gtVector<AMDTProfileThreadInfo>;

struct AMDTProfileProcessInfo
{
    AMDTProcessId               m_pid;
    gtString                    m_name;
    gtString                    m_path;
    bool                        m_is64Bit;

    AMDTUInt64                  m_startTime;
    AMDTUInt64                  m_endTime;

    AMDTProfileModuleInfoVec    m_modulesList;
    AMDTProfileThreadInfoVec    m_threadsList;
};

struct AMDTProfileFunctionInfo
{
    AMDTFunctionId    m_functionId;
    gtString          m_name;
    AMDTModuleId      m_moduleId;
    gtString          m_modulePath; // TBD: Not Reqd
    gtUInt64          m_startOffset;
    gtUInt32          m_size;
};

struct AMDTSampleValue
{
    AMDTCounterId           m_counterId = AMDT_PROFILE_ALL_COUNTERS;
    AMDTUInt32              m_coreId = AMDT_PROFILE_ALL_CORES; // valid only when seperate-by-core is enabled. should this be deviceId?

    AMDTProfileSystemTime   m_sampleTime;
    AMDTUInt64              m_timeSinceProfileStart = 0; // in milli seconds

    double                  m_sampleCount = 0.0;
    double                  m_sampleCountPercentage = 0.0;
};
using AMDTSampleValueVec = gtVector<AMDTSampleValue>;

struct AMDTProfileData
{
    AMDTProfileDataType  m_type;         // process, module, function, thread?
    gtUInt64             m_id;          // Can be Process/Module/Function Id depending on the type
    AMDTModuleId         m_moduleId;    // Valid for Functions,
    gtString             m_name;         // name of the function/thread or path of the process/module
    AMDTSampleValueVec   m_sampleValue;
};

struct AMDTProfileInstructionData
{
    AMDTUInt32          m_offset;           // offset from the beginning of the module. should this be from beginning of the function
    AMDTSampleValueVec  m_sampleValues;

    ~AMDTProfileInstructionData()
    {
        m_sampleValues.clear();
    }
};
using AMDTProfileInstructionDataVec = gtVector<AMDTProfileInstructionData>;

struct AMDTProfileSourceLineData
{
    AMDTUInt32          m_sourceLineNumber; // Line number in the source
    AMDTSampleValueVec  m_sampleValues;

    ~AMDTProfileSourceLineData()
    {
        m_sampleValues.clear();
    }
};
using AMDTProfileSourceLineDataVec = gtVector<AMDTProfileSourceLineData>;

struct AMDTProfileFunctionData
{
    AMDTProfileFunctionInfo         m_functionInfo;

    // TODO: We need to provide the pids and threads list for which this function has samples
    // AMDTProcessId                   m_pid;
    // AMDTThreadId                    m_threadId;

    gtUInt64                        m_modBaseAddress;  // TBD: Not Reqduired?

    AMDTProfileSourceLineDataVec    m_srcLineDataList;
    AMDTProfileInstructionDataVec   m_instDataList;

    ~AMDTProfileFunctionData()
    {
        m_srcLineDataList.clear();
        m_instDataList.clear();
    }
};

// Source and Disassmebly info
struct AMDTSourceAndDisasmInfo
{
    gtVAddr     m_offset;
    gtUInt16    m_sourceLine;
    gtString    m_disasmStr;
    gtString    m_codeByteStr;
};

using AMDTSourceAndDisasmInfoVec = gtVector<AMDTSourceAndDisasmInfo>;
using AMDTCpuTopologyVec = gtVector<AMDTCpuTopology>;
using AMDTProfileProcessInfoVec = gtVector<AMDTProfileProcessInfo>;
using AMDTProfileSamplingConfigVec = gtVector<AMDTProfileSamplingConfig>;
using AMDTProfileDataVec = gtVector<AMDTProfileData>;
using AMDTProfileReportConfigVec = gtVector<AMDTProfileReportConfig>;


struct AMDTCallGraphFunction
{
    AMDTProfileFunctionInfo m_functionInfo;

    gtString                m_srcFile;
    gtUInt32                m_srcFileLine = 0;
    gtVAddr                 m_moduleBaseAddr = 0;

    gtUInt64                m_totalSelfSamples = 0;
    gtUInt64                m_totalDeepSamples = 0;
    double                  m_deepSamplesPerc = 0.0;

    gtUInt32                m_pathCount = 0;
};

using AMDTCallGraphFunctionVec = gtVector<AMDTCallGraphFunction>;
using AMDTCallGraphPath = gtVector<AMDTCallGraphFunction>;

//
//  !!! This is an internal structure !!!
//
struct CallstackFrame
{
    gtUInt32                m_callstackId;

    AMDTProfileFunctionInfo m_funcInfo;
    gtUInt32                m_depth;
    bool                    m_isLeaf = false;
    gtVAddr                 m_moduleBaseAddr;

    gtUInt32                m_counterId;
    gtUInt32                m_selfSamples = 0;      // Valid only if m_isLeaf is set
};

using CallstackFrameVec = gtVector<CallstackFrame>;

//
//  Temporary structs
//

// This struct is used for sample info insertion into DB
// TODO: this will be removed once we optimize translation for DB
struct CPSampleData
{
    gtUInt64    m_processThreadId = 0;
    gtUInt32    m_moduleInstanceId = 0;
    gtUInt64    m_coreSamplingConfigId = 0;
    gtUInt32    m_functionId = 0;
    gtUInt64    m_offset = 0;
    gtUInt64    m_count = 0;
};

struct SampledValue
{
    SampledValue() : m_sampleValue(0.0), m_sampleTime(0) {}
    SampledValue(int sampleTime, double sampleValue) :
        m_sampleValue(sampleValue), m_sampleTime(sampleTime) {}

    // The sample value.
    double  m_sampleValue;

    // The time when the samples was taken.
    int  m_sampleTime;
};

struct SamplingTimeRange
{
    SamplingTimeRange(int fromQuantizedTime, int toQuantizedTime) :
        m_fromTime(fromQuantizedTime), m_toTime(toQuantizedTime) {}

    int  m_fromTime;
    int  m_toTime;
};

//
//  AMDTProfileTimelineSample & AMDTProfileCounterValue are used to instert/query timeline samples
//

struct AMDTProfileCounterValue
{
    AMDTUInt32      m_counterId;
    AMDTFloat32     m_counterValue;
};

// Used by DbAdapter/MidTier
struct AMDTProfileTimelineSample
{
    AMDTProfileSystemTime               m_sampleSystemTime;
    AMDTUInt64                          m_sampleElapsedTimeMs;
    gtVector<AMDTProfileCounterValue>   m_sampleValues;
};

#endif // _AMDTCOMMONPROFILEDATATYPES_H_