//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdTranslator.h
/// \brief This is the interface for the PRD file translation.
///
//==================================================================================

#ifndef _PRDTRANSLATOR_H_
#define _PRDTRANSLATOR_H_

#include <memory>

#include "MemoryMap.h"
#include "CluInfo.h"
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>
#include <AMDTCpuProfilingRawData/inc/Windows/PrdReader.h>
#include <AMDTCpuProfilingRawData/inc/RunInfo.h>
#include <CXLTaskInfo/inc/TaskInfoInterface.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>
#include <AMDTProfilingAgentsData/inc/JavaJncReader.h>
#include <AMDTCpuCallstackSampling/inc/CallGraph.h>
#include <AMDTCpuCallstackSampling/inc/VirtualStackWalker.h>
#include <AMDTOSWrappers/Include/osReadWriteLock.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osSynchronizationObject.h>
#include <AMDTExecutableFormat/inc/ProcessWorkingSet.h>
#include <AMDTBaseTools/Include/gtFlatMap.h>
#include <AMDTCpuProfilingRawData/inc/ProfilerDataDBWriter.h>

// Enable this macro to enable .ebp/.imd files generation, only for debugging purpose
#define ENABLE_OLD_PROFILE_WRITER  0


#if ENABLE_OLD_PROFILE_WRITER
#include <AMDTCpuProfilingRawData/inc/CpuProfileWriter.h>
#endif

class ExecutableAnalyzer;

typedef gtMap<EventMaskType, float> EventNormValueMap;

typedef gtMap<EventMaskType, unsigned int> EventMap;

struct MissedInfoType
{
    gtUInt64 missedCount;
    int worstEvent;

    MissedInfoType() : missedCount(0), worstEvent(0) {}
};


class PidModaddrKey
{
public:
    PidModaddrKey() : pid(0), modLdAddr(0) {}
    PidModaddrKey(ProcessIdType p, gtVAddr m) : pid(p), modLdAddr(m) {}

    ~PidModaddrKey() {}

    bool operator< (const PidModaddrKey& m) const { return (pid < m.pid) || (pid == m.pid && modLdAddr < m.modLdAddr); }

    ProcessIdType pid;
    gtVAddr modLdAddr;
};

typedef gtMap<PidModaddrKey, NameModuleMap::iterator> PidModaddrItrMap;
typedef gtList<PidProcessMap*> PidProcessList;
typedef gtList<NameModuleMap*> NameModuleList;

using ModInstanceMap = gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>;
using ModInstanceList = gtList<ModInstanceMap*>;

struct PrdTranslationStats
{
    struct StatValue
    {
        unsigned int value;
        unsigned int count;

        StatValue() : value(0U), count(0U) {}

        StatValue& operator+=(const StatValue& stat)
        {
            Add(stat);
            return *this;
        }

        void Add(const StatValue& stat)
        {
            value += stat.value;
            count += stat.count;
        }

        void Add(unsigned int val)
        {
            value += val;
            count++;
        }

        void AtomicAdd(const StatValue& stat)
        {
            InterlockedExchangeAdd(reinterpret_cast<long*>(&value), static_cast<long>(stat.value));
            InterlockedExchangeAdd(reinterpret_cast<long*>(&count), static_cast<long>(stat.count));
        }

        void AtomicAdd(unsigned int val)
        {
            InterlockedExchangeAdd(reinterpret_cast<long*>(&value), static_cast<long>(val));
            InterlockedIncrement(reinterpret_cast<long*>(&count));
        }
    };


    enum
    {
        findModuleInfo,
        addWorkingSetModule,
        querySymbolEngine,

        analyzeCss,
        analyzeUserCss,
        buildCss,
        traverseCss,

        finalizeKernelCss,
        finalizeUserCss,
        finalizePartialUserCss,

        disassemble,

        mapMemoryUserCss,
        unmapMemoryUserCss,

        COUNT_VALUES
    };

    StatValue m_values[COUNT_VALUES];


    PrdTranslationStats& operator+=(const PrdTranslationStats& stats)
    {
        for (int i = 0; i < COUNT_VALUES; ++i)
        {
            m_values[i] += stats.m_values[i];
        }

        return *this;
    }
};

class PrdTranslator
{
public:
    //Uses taskinfo dataFile(.prd->.ti),
    PrdTranslator(gtString dataFile, bool collectStat = false);
    ~PrdTranslator();

    void SetNumWorkerThreads(DWORD numThreads) { m_numWorkerThreads = numThreads; }

    DWORD GetNumWorkerThreads()  const { return m_numWorkerThreads; }

    HRESULT TranslateData(const osFilePath& proFile,
                          MissedInfoType* pMissedInfo,
                          gtList<gtString>& processFilters,
                          gtString& errorString,
                          bool bThread = false,
                          bool bCLUtil = false,
                          bool bLdStCollect = false,
                          PfnProgressBarCallback pfnProgressBarCallback = NULL);

    HRESULT ThreadTranslateDataPrdFile(const osFilePath& proFile,
                                       MissedInfoType* pMissedInfo,
                                       gtList<gtString>& processFilters,
                                       MemoryMap& mapAddress,
                                       PrdReader& tPrdReader,
                                       PrdReaderThread& threadPrdReader,
                                       bool bThread,
                                       bool bMainThread,
                                       gtUInt64* pByteRead,
                                       gtUInt64 totalBytes,
                                       gtUInt64 baseAddress,
                                       PidProcessMap& processMap,
                                       NameModuleMap& moduleMap,
                                       PidModaddrItrMap* pidModaddrItrMap,
                                       ModInstanceMap& ModInstanceMap,
                                       bool bCLUUtil,
                                       bool bLdStCollect,
                                       UINT8 L1DcAssoc,
                                       UINT8 L1DcLineSize,
                                       UINT8 L1DcLinesPerTag,
                                       UINT8 L1DcSize,
                                       gtUByte* pCssBuffer,
                                       PrdTranslationStats* pStats);

    unsigned int GetProfileType();

    bool GetProfileEventCount(int* pEventCount);

    bool GetTimerInterval(gtUInt64* resolution);
    void addJavaInlinedMethods(CpuProfileModule&  mod,
        gtVector<std::tuple<gtUInt32, gtUInt32, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& inlinedJitInfo);

    void SetDebugSymbolsSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath);

private:
    struct TimeRange
    {
        gtUInt64 m_begin;
        gtUInt64 m_end;

        bool operator<(const TimeRange& range) const
        {
            return m_end < range.m_begin;
        }

        bool operator==(const TimeRange& range) const
        {
            return 0 == memcmp(this, &range, sizeof(TimeRange));
        }

        bool operator!=(const TimeRange& range) const
        {
            return !operator==(range);
        }
    };

    struct UserCallStack
    {
        enum KernelTransaction
        {
            KERNEL_TRANSACTION_UNKNOWN,
            KERNEL_TRANSACTION_FALSE,
            KERNEL_TRANSACTION_TRUE,
        };

        CallSite* m_pSampleSite;
        CallStack* m_pCallStack;
        unsigned m_callStackIndex;
        KernelTransaction m_kernelTransaction;
    };

    typedef gtMap<TimeRange, UserCallStack> PeriodicUserCallStackMap;
    typedef gtFlatMap<ThreadIdType, PeriodicUserCallStackMap*> ThreadUserCallStacksMap;

    typedef gtFlatMap<VAddrRange, ExecutableAnalyzer*> ExecutableAnalyzersMap;

    struct ProcessInfo
    {
        ProcessIdType m_processId;
        osCriticalSection m_criticalSection;
        ExecutableAnalyzersMap m_exeAnalyzers;
        osReadWriteLock m_lockAnalyzers;

        CallGraph m_callGraph;

        ThreadUserCallStacksMap m_userCallStacks;

        ProcessInfo(ProcessIdType processId);
        ~ProcessInfo();

        ExecutableAnalyzer* AcquireExecutableAnalyzer(gtVAddr va);
    };

    void InitializeProgressBar(const gtString& caption, bool incremental = false);
    void IncrementProgressBar(int value);
    void UpdateProgressBar();
    void UpdateProgressBar(gtUInt64 bytesReadSoFar, gtUInt64 totalBytes);
    void CompleteProgressBar();

    void AddBytesToProgressBar(gtUInt64 bytes);
    void AsyncAddBytesToProgressBar(gtUInt64 bytes);

    bool InitPrdReader(PrdReader* pReader, const wchar_t* pFileName, gtUInt64* pLastUserCssRecordOffset, gtString& errorString);

    unsigned int GetCpuCount() const;

    bool IsProfilingDriver(gtVAddr va) const;

    static unsigned int GetKernelCallStackAdditionalRecordsCount(unsigned int callersCount, bool is64Bit);
    static unsigned int GetUserCallStackAdditionalRecordsCount(unsigned int callersCount, bool is64Bit);
    static unsigned int GetVirtualStackAdditionalRecordsCount(unsigned int valuesCount);

    static bool BuildCallStack(ProcessInfo& processInfo,
                               ProcessIdType processID,
                               gtUInt64 tsc,
                               unsigned core,
                               gtUInt32* pValues32,
                               gtUInt64* pValues64,
                               unsigned depth,
                               gtUInt64& sampleAddr,
                               CallStackBuilder& callStackBuilder,
                               PrdTranslationStats* const pStats);

    void FinalizeKernelCallStack(ProcessInfo& processInfo,
                                 gtUInt64 threadID,
                                 gtUInt64 tsc,
                                 gtUInt64 sampleAddr,
                                 EventMaskType eventType,
                                 CallStackBuilder& callStackBuilder,
                                 PrdTranslationStats* const pStats);

    static void FinalizeUserCallStack(ProcessInfo& processInfo,
                                      ExecutableFile* pPeFile,
                                      gtUInt64 threadID,
                                      gtUInt64 tsc,
                                      EventMaskType eventType,
                                      gtUInt64 instructionPtr,
                                      PrdTranslationStats* const pStats);

    static void FinalizePartialUserCallStack(ProcessInfo& processInfo,
                                             gtUInt64 threadID,
                                             const TimeRange& tscRange,
                                             gtUInt64 sampleAddr,
                                             CallStackBuilder& callStackBuilder,
                                             PrdTranslationStats* const pStats);

    HRESULT TranslateKernelCallStack(PRD_KERNEL_CSS_DATA_RECORD& kernelCssRec,
                                     PrdReaderThread& threadPRDReader,
                                     ProcessInfo*& pProcessInfo,
                                     ProcessIdType& processInfoId,
                                     const gtList<DWORD>& filters,
                                     ProcessIdType processId,
                                     ThreadIdType threadId,
                                     gtUInt64 timeStamp,
                                     EventMaskType eventType,
                                     unsigned int core,
                                     gtUByte* pBuffer,
                                     PrdTranslationStats* const pStats);

    void CssMapCleanup();

    HRESULT TranslateDataPrdFile(const osFilePath& proFile,
                                 MissedInfoType* pMissedInfo,
                                 gtList<gtString>& processFilters,
                                 gtString& errorString,
                                 bool bThread = false,
                                 bool bCLUtil = false,
                                 bool bLdStCollect = false);

    bool AggregateSampleData(
        RecordDataStruct prdRecord,
        TiModuleInfo* pModInfo,
        PidProcessMap* pPMap,
        NameModuleMap* pMMap,
        PidModaddrItrMap* pidModaddrItrMap,
        unsigned int samplesCount,
        PrdTranslationStats* const pStats);

    void AggregateKnownModuleSampleData(
        SampleInfo& sampInfo,
        TiModuleInfo* pModInfo,
        NameModuleMap* pMMap,
        PidModaddrItrMap& pidModaddrItrMap,
        bool& b_is32bit,
        unsigned int samplesCount,
        PrdTranslationStats* const pStats);

    void AggregateUnknownModuleSampleData(
        SampleInfo& sampInfo,
        TiModuleInfo* pModInfo,
        NameModuleMap* pMMap,
        bool& b_is32bit,
        unsigned int samplesCount);

    void AggregatePidSampleData(
        RecordDataStruct& prdRecord,
        TiModuleInfo* pModInfo,
        PidProcessMap* pPMap,
        bool b_is32bit,
        unsigned int samplesCount);

    void InitNewModule(
        CpuProfileModule& mod,
        TiModuleInfo* pModInfo,
        const gtString& ModName,
        const gtString& FuncName,
        const gtString& JncName,
        const gtString& JavaSrcFileName,
        ProcessIdType pid);

    bool ProcessIbsFetchRecord(
        const IBSFetchRecordData& ibsFetchRec,
        TiModuleInfo* pModInfo,
        PidProcessMap* pPMap,
        NameModuleMap* pMMap,
        PidModaddrItrMap* pidModaddrItrMap,
        PrdTranslationStats* const pStats);

    bool ProcessIbsOpRecord(
        const IBSOpRecordData& ibsOpRec,
        TiModuleInfo* pModInfo,
        PidProcessMap* pPMap,
        NameModuleMap* pMMap,
        PidModaddrItrMap* pidModaddrItrMap,
        bool bDoCLU,
        bool bLdStCollect,
        UINT8 L1DcAssoc,
        UINT8 L1DcLineSize,
        UINT8 L1DcLinesPerTag,
        UINT8 L1DcSize,
        PrdTranslationStats* const pStats);

    void AggregateCluData(PidProcessMap* pPMap,
                          NameModuleMap* pMMap,
                          const osFilePath& proFile,
                          PrdTranslationStats* const pStats);

    static HRESULT CreatePRDView(PrdReader& tPrdReader,
                                 gtUInt64 offset,
                                 gtUInt32 length,
                                 MemoryMap& mapAddress,
                                 gtUInt32* pfirstWeightRecOffset);

    HRESULT GetBufferRecordCount(
        PrdReader& tPrdReader,
        LPVOID baseAddress,
        gtUInt32* pRecType,
        gtUInt32* pCnt);

    bool AggregateThreadMaps(PidProcessList& procList, NameModuleList& modList, ModInstanceList& modInstanceList);

    HRESULT WriteProfile(const osFilePath& proFile,
                         PrdReader& tPrdReader,
                         const MissedInfoType* pMissedInfo,
                         const PidProcessMap& processMap,
                         NameModuleMap& moduleMap);

#if ENABLE_OLD_PROFILE_WRITER
    bool WriteProfileFile(const gtString& path,
                          const PidProcessMap* procMap,
                          const NameModuleMap* modMap,
                          const CoreTopologyMap* pTopMap,
                          gtUInt32 num_cpus,
                          gtUInt64 missedCount = 0,
                          int cpuFamily = 0, //FAMILY_UNKNOWN
                          int cpuModel = 0);
#endif

    bool WriteMetaProfileDataIntoDB(PidProcessMap& processMap,
                                    NameModuleMap& moduleMap,
                                    ModInstanceMap& modInstanceMap);

    bool WriteMetaProfileDataIntoDB(PidProcessList& processMapList,
                                    NameModuleList& moduleMapList,
                                    ModInstanceList& modInstanceMapList);

    void AddIBSFetchEventsToMap(int cpuFamily, int cpuModel);

    void AddIBSOpEventsToMap(int cpuFamily, int cpuModel, bool addBr, bool addLS, bool addNB);

    void AddCluEventsToMap();

    HRESULT OpenTaskInfoFile();

    bool CheckForNestedJavaInlinedFunction(CpuProfileFunction&   func,
                                           JavaInlineMap*        jilMap,
                                           AddrFunctionMultMap&  inlinedFuncMap,
                                           gtUInt32&             nextFuncId,
                                           gtUInt32              moduleId,
        gtVector<std::tuple<gtUInt32, gtUInt32, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& inlinedJitInfo);

    bool CheckForJavaInlinedFunction(CpuProfileFunction&  func,
                                     JavaInlineMap*        jilMap,
                                     AddrFunctionMultMap&  inlinedFuncMap,
                                     gtUInt32&             nextFuncId,
                                     gtUInt32              moduleId,
        gtVector<std::tuple<gtUInt32, gtUInt32, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& inlinedJitInfo);

    gtString GetJavaNestedFunctionParentName(JNCInlineMap& jilMap, gtString javaInlinedFunc);

private:
    void AddNewModule(ProcessIdType pid, TiModuleInfo& pModInfo, NameModuleMap& pMMap);
    void AddIbsFetchEvent(unsigned int eventSelect);
    void AddIbsOpEvent(unsigned int eventSelect);
    void AddCluEvent(unsigned int eventSelect);

    bool WriteProcessInfoIntoDB(PidProcessMap& processMap);
    bool WriteThreadInfoIntoDB(PidProcessMap& processMap);
    bool WriteModuleInfoIntoDB(NameModuleMap& moduleMap);
    bool WriteModuleInstanceInfoIntoDB(NameModuleMap& moduleMap, ModInstanceMap& modInstanceMap);
    bool WriteJavaInlineModuleInstanceInfoIntoDB(const NameModuleMap& moduleMap);
    bool WriteFunctionInfoIntoDB(NameModuleMap& moduleMap);
    bool WriteSampleProfileDataIntoDB(const NameModuleMap& moduleMap);
    bool WriteCallgraphProfileDataIntoDB(const NameModuleMap& modMap);
    bool WriteJitInfoIntoDB(const NameModuleMap& modMap,
        gtVector<std::tuple<gtUInt32, gtUInt32, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& inlinedJitInfo);

    bool IsFilteredProcess(gtList<DWORD>& filteredPids, DWORD pid);

    ProcessInfo* FindProcessInfo(ProcessIdType pid) const;
    ProcessInfo& AcquireProcessInfo(ProcessIdType pid);

    friend class PrdUserCssRcuHandler;
    friend class PrdUserCssRcuHandlerPool;

private:
    gtString m_dataFile;
    bool     m_collectStat = false;

    // Number of worker threads to be created for processing the PRD file.
    DWORD   m_numWorkerThreads = 0;

    // The following two variables retain the IBS fetch and op maximum periodic count.
    unsigned int m_ibsFetchCount = 0;
    unsigned int m_ibsOpCount = 0;

    //
    // The event map is a list of the events that were sampled
    // during data collection. The events are identified by their
    // 16-bit event select value. The timer event is identified by
    // GetTimerEvent(). This list is eventually written to the .tbp/.ebp file.
    //
    EventMap m_eventMap;
    EventNormValueMap m_norms;

    // thread sync
    HRESULT m_ThreadHR = S_FALSE;
    gtUInt64 m_hrFreq = 0;
    unsigned int m_durationSec = 0;

    CluInfo* m_pCluInfo = nullptr;
    RunInfo* m_runInfo = nullptr;

    BOOL m_is64Sys = FALSE;
    PidFilterList m_pidFilterList;  // TODO: Seems to be unused

    PfnProgressBarCallback m_pfnProgressBarCallback = nullptr;
    //apProfileProgressEvent m_progressEvent;
    CpuProfilerProgress m_progressEvent;
    gtUInt64 m_progressStride = 0;
    volatile gtUInt64 m_progressThreshold = 0;
    volatile gtInt32 m_progressAsync = 0;
    osSynchronizationObject m_progressSyncObject;
    bool m_useProgressSyncObject = false;

    mutable osReadWriteLock m_processInfosLock;
    gtMap<ProcessIdType, ProcessInfo*> m_processInfos;
    wchar_t* m_pSearchPath = nullptr;
    wchar_t* m_pServerList = nullptr;
    wchar_t* m_pCachePath = nullptr;

    KeModQueryInfo* m_pProfilingDrivers = nullptr;
    unsigned m_countProfilingDrivers = 0U;

    std::unique_ptr<ProfilerDataDBWriter> m_dbWriter;
};

struct ThreadPrdData
{
    PrdTranslator*      pCDXptr;
    osFilePath          sessionPath;
    MissedInfoType*     pMissedInfo;
    gtList<gtString>*   pProcessFilters;
    MemoryMap*          pMapAddress;
    PrdReader*          pPrdReader;        // Global PRD Reader
    PrdReaderThread*    threadPrdReader;   // Thread specific PRD reader

    PidProcessMap*      processMap;
    NameModuleMap*      moduleMap;
    PidModaddrItrMap*   pidModaddrItrMap;
    ModInstanceMap*     modInstanceMap;

    gtUInt64*           pBytesRead;
    gtUInt64            totalTypes;
    bool                bThread;
    bool                bCLUtil;
    bool                bLdStCollect;
    bool                bMainThread;
    UINT8               L1DcSize;
    UINT8               L1DcAssoc;
    UINT8               L1DcLinesPerTag;
    UINT8               L1DcLineSize;

    gtUByte* pCssBuffer;

    PrdTranslationStats stats;
};

class TiProcessWorkingSetQuery : public ProcessWorkingSetQuery
{
public:
    TiProcessWorkingSetQuery(ProcessIdType processId) : m_processId(processId) {}

    virtual ExecutableFile* FindModule(gtVAddr va)
    {
        return fnFindExecutableFile(m_processId, va);
    }

    virtual unsigned ForeachModule(void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext)
    {
        return fnForeachExecutableFile(m_processId, true, pfnProcessModule, pContext);
    }

private:
    gtUInt64 m_processId;
};

#endif // _PRDTRANSLATOR_H_