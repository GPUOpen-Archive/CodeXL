//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Utils.h
///
//==================================================================================

#ifndef __CPUPROFILE_CLI_UTILS_H
#define __CPUPROFILE_CLI_UTILS_H

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

// AMDTCpuPerfEventUtils:
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>
#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuPerfEventUtils/inc/IbsEvents.h>

// Data Translation
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/PeFile.h>
#endif // AMDT_WINDOWS_OS

#define ALL_PROCESS_IDS    (-1)
#define ALL_THREAD_IDS     (-1)

// FLAGS
#define SAMPLE_IGNORE_SYSTEM_MODULES    0x1
#define SAMPLE_GROUP_BY_THREAD          0x2
#define SAMPLE_GROUP_BY_MODULE          0x4
#define SAMPLE_SEPARATE_BY_CORE         0x8

typedef struct SamplingEvent
{
    EventMaskType  m_event;
    gtString       m_eventName;
    gtUInt64       m_interval;
    gtUInt16       m_unitMask;
    bool           m_user;
    bool           m_os;
    bool           edge;
} SamplingEvent;

// Event to index Map; This index is used in EBP/IMD file
typedef gtMap<EventMaskType, int> EventToIndexMap;

typedef struct ProcessInfo
{
public:
    gtString            m_processName;
    ProcessIdType       m_pid;

    gtVector<gtUInt64>  m_dataVector;       // total samples for every sampling event

    bool                m_is32Bit;
    bool                m_hasCSS;
} ProcessInfo;


typedef struct ModuleInfo
{
public:
    gtString            m_moduleName;
    gtString            m_modulePath;

    gtVector<gtUInt64>  m_dataVector;   // total aggregate across all the PIDs
    gtMap<ProcessIdType, gtVector<gtUInt64>>  m_pidDataVector;

    bool                m_isSystemModule;
} ModuleInfo;


class FunctionInfo
{
public:
    gtString                 m_functionName;
    gtVAddr                  m_baseAddress;

    gtVector<gtUInt64>       m_dataVector;

    ProcessIdType            m_pid;
    ThreadIdType             m_tid;

    const CpuProfileModule*  m_pModule;
};

struct CGFunctionInfo;

struct CGParentChildInfo
{
    CGFunctionInfo*  m_pFuncInfo;
    gtUInt64         m_selfCount;
    gtUInt64         m_deepCount;

    CGParentChildInfo(CGFunctionInfo* pFuncInfo)
    {
        m_pFuncInfo = pFuncInfo;
        m_selfCount = 0;
        m_deepCount = 0;
    }
};

struct CGFunctionInfo
{
    CpuProfileModule*    m_pModule;
    CpuProfileFunction*  m_pFunction;

    gtVAddr              m_baseAddress;
    gtUInt64             m_selfCount;
    gtUInt64             m_deepCount;
    gtUInt32             m_pathCount;

    gtString             m_funcName;
    gtUInt64             m_index;   // function index in callgraph

    gtVector<CGParentChildInfo>  m_parents;
    gtVector<CGParentChildInfo>  m_children;

    CGFunctionInfo(gtString funcName)
        : m_pModule(NULL), m_pFunction(NULL), m_baseAddress(0),
          m_selfCount(0), m_deepCount(0), m_pathCount(0),
          m_funcName(funcName), m_index(0)
    {
    }

    CGFunctionInfo(gtString funcName, CpuProfileModule* pMod, CpuProfileFunction* pFunction)
        : m_pModule(pMod), m_pFunction(pFunction), m_baseAddress(0),
          m_selfCount(0), m_deepCount(0), m_pathCount(0),
          m_funcName(funcName), m_index(0)
    {
        m_baseAddress = (NULL != m_pModule) ? m_pModule->getBaseAddr() : 0;
    }
};

// Instruction and samples count map
using InstructionSamplesMap = gtHashMap<string, gtUInt64>;
struct ModuleImixInfo
{
    InstructionSamplesMap m_InstMap;
    const CpuProfileModule*  m_pModule;
    gtUInt64 m_samplesCount;
};

using ModuleImixInfoList = gtVector<ModuleImixInfo>;
using ImixSummaryMap = gtHashMap<string, gtUInt64>;

typedef gtMap<gtString, CGFunctionInfo*> CGFunctionInfoMap; // Function Name & Function Info - TODO can be based on address?
typedef std::multimap<gtUInt64, CGFunctionInfo*> CGSampleFunctionMap; // Sample & Function Info

typedef gtVector<ProcessInfo> ProcessInfoList;
typedef gtMap<ProcessIdType, ProcessInfo> PidProcessInfoMap;
typedef std::multimap<double, ProcessInfo> ProcessInfoMap; // Sample & Process Info

typedef gtVector<ModuleInfo> ModuleInfoList;
typedef std::multimap<double, ModuleInfo> ModuleInfoMap; // Sample & Module Info

typedef gtVector<FunctionInfo> FunctionInfoList;
typedef std::multimap<double, FunctionInfo> FunctionInfoMap;
typedef std::unordered_multimap<gtVAddr, gtUInt32> FunctionIdxMap;

typedef gtVector<gtString> ProfileEventNameVec;


// These are meant to be APIs to access the processes data

bool InitializeEventsXMLFile(EventsFile& eventsFile);

// This returns a vector of EventEncodeType - contains eventmask, count (sampling interval)
// and index used in EBP file
bool GetEventEncodeVec(CpuProfileReader& profileReader, EventEncodeVec& eventVec);

bool GetEventDetailForIndex(EventEncodeVec& eventVec, gtUInt32 index, EventMaskType& eventMask, gtUInt64& eventCount);
bool GetEventDetailForEventMask(EventEncodeVec& eventVec, EventMaskType eventMask, gtUInt32& index, gtUInt64& eventCount);

bool GetNumberOfProcesses(CpuProfileReader& profileReader, gtUInt32& nbrProcs);
bool GetProcessInfoList(CpuProfileReader&    profileReader,
                        bool                 sepByCore,
                        gtUInt64             coreMask,
                        ProcessInfoList&     procList,
                        gtVector<gtUInt64>&  totalDataVector);

bool GetProcessInfoMap(CpuProfileReader&    profileReader,
                       bool                 sepByCore,
                       gtUInt64             coreMask,
                       PidProcessInfoMap&   procInfoMap);

// If pid is -1, get the modules for all the processes
bool GetModuleInfoList(CpuProfileReader&    profileReader,
                       ProcessIdType        pid,
                       gtUInt64             flags,
                       gtUInt64             coreMask,
                       ModuleInfoList&      modList,
                       gtVector<gtUInt64>&  totalDataVector,
                       PidProcessInfoMap*   pProcInfoMap = NULL);

// Get the function info list for the given pid/tid/module
// if pid = -1, tid = -1, module = null, get the entire function list from all the processes
// if pid = PID, tid = -1, module = null, get the entire function list for the given PID
// if pid = PID, tid = TID, module = null, get the entire function list for the given PID & TID
// if pid = PID, tid = TID, module = MOD, get the entire function list for the given PID, TID & MOD
//
CpuProfileModule* GetModuleDetail(CpuProfileReader&  profileReader,
                                  const gtString&    modulePathGt,
                                  ExecutableFile**   ppExe);

bool IsSystemModule(const gtString& absolutePath);

bool GetFunctionInfoList(CpuProfileReader&        profileReader,
                         ProcessIdType            pid,
                         ThreadIdType             tid,
                         gtUInt64                 flags,   // ignore-system-modules, separate-by-core, etc
                         gtUInt64                 coreMask, // UNUSED now
                         FunctionInfoList&        funcInfoList);


bool GetFunctionInfoList(CpuProfileReader&        profileReader,
                         ProcessIdType            pid,
                         ThreadIdType             tid,
                         CpuProfileModule&        module,
                         const gtString&          moduleFilePath,
                         gtUInt64                 flags,   // ignore-system-modules, separate-by-core, etc
                         gtUInt64                 coreMask, // UNUSED now
                         FunctionInfoList&        funcInfoList,
                         FunctionIdxMap&          funcIdxMap);

bool GetImixInfoList(CpuProfileReader&    profileReader,
                     ProcessIdType        pid,
                     ThreadIdType         tid,
                     gtUInt64             flags,
                     gtUInt64             coreMask,
                     ModuleImixInfoList&  modImixInfoList,
                     gtUInt64&            totalSamples);

bool GetImixInfoList(CpuProfileReader&    profileReader,
                     ProcessIdType        pid,
                     ThreadIdType         tid,
                     CpuProfileModule&    module,
                     gtUInt64             flags,
                     gtUInt64             coreMask,
                     ModuleImixInfoList&  modImixInfoList,
                     gtUInt64&            totalSamples);

bool GetImixSummaryMap(CpuProfileReader&    profileReader,
                       ModuleImixInfoList&  modImixInfoList,
                       ImixSummaryMap&      imixSummaryMap,
                       gtUInt64&            totalSamples);

//
// Internal Helper Functions
//

bool GetEventToIndexMap(CpuProfileReader& profileReader, EventToIndexMap& evtToIdexMap);
int GetIndexForEvent(EventToIndexMap& evtToIdexMap, const EventMaskType& eventMask);

gtUInt32 AggregateSamples(CpuProfileReader&        profileReader,
                          const AggregatedSample&  sm,
                          gtVector<gtUInt64>&      dataVector,
                          gtVector<gtUInt64>&      totalVector,
                          EventToIndexMap&         evtToIdxMap,
                          bool                     seperateByCore);


bool GetFunctionName(const CpuProfileModule* pModule, gtVAddr va, gtString& name, CpuProfileFunction** ppFunc);
FunctionInfo* FindFunction(
    FunctionInfoList& funcInfoList,
    FunctionIdxMap& funcIdxMap,
    gtVAddr baseAddr,
    ProcessIdType pid,
    ThreadIdType tid);

inline static gtString convertToGTString(const QString& inputStr)
{
    gtString str;
    str.resize(inputStr.length());

    if (inputStr.length())
    {
        str.resize(inputStr.toWCharArray(&str[0]));
    }

    return str;
}

inline static QString convertToQString(const gtString& inputStr)
{
    return QString::fromWCharArray(inputStr.asCharArray(), inputStr.length());
}

#endif //__CPUPROFILE_CLI_UTILS_H