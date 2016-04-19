//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfTranslator.h
/// \brief This is the interface for the CAPERF file translation.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/Linux/CaPerfTranslator.h#15 $
// Last checkin:   $DateTime: 2016/04/14 01:44:54 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569055 $
//=====================================================================

#ifndef _CAPERFTRANSLATOR_H_
#define _CAPERFTRANSLATOR_H_

//
// Class CaPerfTranslator
//
// Description:
//

#include <linux/perf_event.h>

#include "CaPerfTranslatorIbs.h"
#include <AMDTCpuProfilingRawData/inc/CpuProfileWriter.h>
#include <AMDTCpuProfilingRawData/inc/Linux/CaPerfHeader.h>
#include <ProfilingAgents/AMDTProfilingAgentsData/inc/JavaJncReader.h>

#include <AMDTExecutableFormat/inc/ProcessWorkingSet.h>
#include <AMDTCpuCallstackSampling/inc/CallGraph.h>
#include <AMDTBaseTools/Include/gtFlatMap.h>
#include <AMDTBaseTools/Include/gtList.h>

#include "../JitTaskInfo.h"

// Aggregate the IBS derived event. Increase the derived event count by one
#define AGG_IBS_EVENT(EV) _log_ibs(pProc, ldAddr, funcSize, pMod, ip, pid, tid, cpu, EV, 0, os, usr, 1, pFuncInfo)

// Aggregate the IBS latency/cycle counts. Increase the derived event count by the specified count value
#define AGG_IBS_COUNT(EV, COUNT) _log_ibs(pProc, ldAddr, funcSize, pMod, ip, pid, tid, cpu, EV, 0, os, usr, COUNT, pFuncInfo)

struct calog;
struct PROCMODAddress;
class PerfDataReader;
class ExecutableAnalyzer;

#include <qstring.h>

class EvBlkKey
{
public:

    EvBlkKey(gtUInt64 time = 0, gtUInt64 evId = 0) : time(time), evId(evId) {}

    virtual ~EvBlkKey() {}

    bool operator< (const EvBlkKey& m) const { return (time < m.time) || (time == m.time && evId < m.evId); }

    bool operator==(const EvBlkKey& m) const { return (time == m.time && evId == m.evId); }

    gtUInt64 time;
    gtUInt64 evId;
};


class EvBlkInfo
{
public:

    EvBlkInfo(gtUInt32 vindex = 0,
              gtUInt32 vevId = 0,
              gtUInt32 voffset = 0,
              gtUInt32 vbytes = 0,
              gtUInt32 vnum = 0,
              gtUInt64 vstartTs = 0,
              gtUInt64 vperiod = 0,
              void* pVData = NULL) : index(vindex),
        evId(vevId),
        offset(voffset),
        bytes(vbytes),
        num(vnum),
        numUnProcessed(vnum),
        startTs(vstartTs),
        period(vperiod),
        pData(pVData)
    {}

    virtual ~EvBlkInfo()
    {
        if (NULL != pData)
        {
            free(pData);
        }
    }

    gtUInt32 index;
    gtUInt32 evId;
    gtUInt32 offset;
    gtUInt32 bytes;
    gtUInt32 num;
    gtUInt32 numUnProcessed;
    gtUInt64 startTs;
    gtUInt64 period;

    // This is populate when the block becomes active
    void* pData;
};

// This is second level map of sample blocks with timestamp as key.
// It is used to maintain sorted list of sample blocks.
typedef gtMap<gtUInt64, EvBlkInfo> TsEvBlkMap;

// This is the top level map from EvId to list of sample blocks
typedef gtMap<gtUInt64, TsEvBlkMap> EvBlkIdMap;

// This multimap is used for sorting timestamp of all active
// sample blocks. "Active blocks" are set of sample blocks
// being sorted.
typedef std::multimap<gtUInt64, void*> TimeStampRecordMap;

//------------------------------------------------------------------------------
class ModKey
{
public:

    ModKey(gtUInt64 vtime = 0, gtUInt64 vaddr = 0, int vpid = 0) : time(vtime), addr(vaddr), pid(vpid) {}

    virtual ~ModKey() {}

    bool operator< (const ModKey& m) const
    {
        return (pid < m.pid) || (pid == m.pid && (addr < m.addr || (addr == m.addr && time < m.time)));
    }

    bool operator==(const ModKey& m) const { return (pid == m.pid && time == m.time && addr == m.addr); }

    gtUInt64 time;
    gtUInt64 addr;
    int pid;
};


class ModInfo
{
public:
    ModInfo() : len(0), pgoff(0), pProc(NULL), pMod(NULL) {}

    ModInfo(gtUInt64 vlen, gtUInt64 vpgoff, char filename[]) : len(vlen), pgoff(vpgoff), name(filename), pProc(NULL), pMod(NULL) {}

    virtual ~ModInfo() {}

    gtUInt64 len;
    gtUInt64 pgoff;
    string name;
    CpuProfileProcess* pProc;
    CpuProfileModule* pMod;
};


// We use multimap here since we can have different
// modules loaded at the same gtUInt64ess. For example,
// when process is fork but not yet executed.
typedef multimap<ModKey, ModInfo> ModLoadInfoMap;

//------------------------------------------------------------------------------

class ProcInfo
{
public:
    ProcInfo() : ppid(0) {}

    pid_t ppid;
    string comm;
    gtList<ThreadIdType> tidList;
};

typedef gtMap<pid_t, ProcInfo> PidProcInfoMap;

struct PreviousTimes
{
    gtUInt64 previousEnabledTime;
    gtUInt64 previousRunningTime;
};

typedef gtMap<gtUInt64, PreviousTimes> EvtIdPreviousMap;

//------------------------------------------------------------------------------
//
// Class CaPerfTranslator
//
// Descriptions:
// This class implement the Perf to CAData translator.
//
class CaPerfTranslator
{
    typedef int (CaPerfTranslator::*PerfRecordHandler_t)(struct perf_event_header* pHdr, const void* ptr, gtUInt32 offset, gtUInt32 index);

public:
    enum Pass2TranslationMode
    {
        PASS2_MODE_SERIAL = 0,
        PASS2_MODE_SORT,
        PASS2_MODE_MAX,
    };

    CaPerfTranslator();
    CaPerfTranslator(const string& perfDataPath);

    virtual ~CaPerfTranslator();

    // Translate perf.data file to EBP/IMD files
    int translatePerfDataToCaData(const string& outPath = "", const string& perfDataPath = "", bool bVerb = false);

    HRESULT dumpPerfData(const string& perfDataPath);

    static int getCurrentPerfDataVersion();

    static int getCurrentCaDataVersion();

    int writeEbpOutput(const string& outputFile);

    HRESULT setupLogFile(const string& logFile);

    int setupCalogCssFile(const string& file);

    int setupCssFile(const string& file);

    void setupTranslateMode(gtUInt32 mode) { m_pass2Mode = mode; }

    PidProcInfoMap* getTargetProcessesAndThreads() { return & m_pidProcInfoMap; }

    gtList<string>* getErrorList() { return & m_errorList; }

    void SetDebugSymbolsSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath);

protected:
    ModLoadInfoMap::reverse_iterator getModuleForSample(struct perf_event_header* pHdr,
                                                        gtUInt32 pid, gtUInt64 time, gtUInt64 ip,
                                                        bool bStat, bool bIsUser);

    CpuProfileModule* getModule(const string& modname);

    CpuProfileProcess* getProcess(ProcessIdType pid);

    virtual int process_PERF_RECORD_MMAP(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    //  virtual int process_PERF_RECORD_LOST(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    virtual int process_PERF_RECORD_COMM(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    virtual int process_PERF_RECORD_EXIT(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    //  virtual int process_PERF_RECORD_THROTTLE(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    //  virtual int process_PERF_RECORD_UNTHROTTLE(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    virtual int process_PERF_RECORD_FORK(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    virtual int process_PERF_RECORD_READ(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    virtual int process_PERF_RECORD_SAMPLE(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    virtual int preprocess_PERF_RECORD_SAMPLE_into_block(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index);

    void trans_ibs_fetch(struct ibs_fetch_sample* trans_fetch,
                         gtUInt32 selected_flag,
                         CpuProfileProcess* pProc,
                         gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                         gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                         gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo);

    void trans_ibs_op_mask_reserved(gtUInt32 family, struct ibs_op_sample* trans);

    int  trans_ibs_op_rip_invalid(struct ibs_op_sample* trans);

    void trans_ibs_op(struct ibs_op_sample* trans, gtUInt32 selected_flag,
                      CpuProfileProcess* pProc,
                      gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                      gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                      gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo);

    void trans_ibs_op_ls(struct ibs_op_sample* trans, gtUInt32 selected_flag,
                         CpuProfileProcess* pProc,
                         gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                         gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                         gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo);

    void trans_ibs_op_nb(struct ibs_op_sample* trans, gtUInt32 selected_flag,
                         CpuProfileProcess* pProc,
                         gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                         gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                         gtUInt32 os, gtUInt32 usr, gtUInt32 count, const FunctionSymbolInfo* pFuncInfo);

#ifdef HAS_DCMISS
    void trans_ibs_op_ls_dcmiss(struct ibs_op_sample* trans);
#endif
#ifdef  HAS_BTA
    void trans_ibs_op_bta(struct ibs_op_sample* trans);
#endif

    CpuProfileModule* getJavaModuleforSample(TiModuleInfo* pModInfo, gtUInt32 pid, gtUInt64 time, gtUInt64 ip);

    void addJavaInlinedMethods(CpuProfileModule&  mod);

private:
    void _init();

    inline void _clearAllHandlers();

    HRESULT _setupReader(const string& perfDataPath);

    HRESULT _getModuleBitness(const string& modName, bool* pIs32Bit);

    HRESULT _getElfFileType(const string& modName, gtUInt32* pElfType);

    ModLoadInfoMap::reverse_iterator _getModuleForSample(gtUInt32 pid, gtUInt64 time, gtUInt64 ip, bool bIsUser);

    const FunctionSymbolInfo* getFunctionSymbol(ProcessIdType pid, gtVAddr ip, CpuProfileModule* pMod = NULL);

    void _addSampleToProcessAndModule(CpuProfileProcess* pProc,
                                      gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                      gtUInt64 ip,
                                      gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                      gtUInt32 event, gtUInt32 umask, gtUInt32 os, gtUInt32 usr,
                                      gtUInt32 count,
                                      const FunctionSymbolInfo* pFuncInfo);

    void _log_ibs(CpuProfileProcess* pProc,
                  gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                  gtUInt64 ip, gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                  gtUInt32 event, gtUInt32 umask, gtUInt32 os, gtUInt32 usr,
                  gtUInt32 count,
                  const FunctionSymbolInfo* pFuncInfo);

    EventMaskType _getEvmask(gtUInt32 event, gtUInt32 umask, gtUInt32 kernel, gtUInt32 user);

    HRESULT _getTargetPids();

    HRESULT _getSampleBlockFromOffset(gtUInt32 blkOffset, gtUInt32 blkSize, gtUInt64 evId, void** ppDta);

    bool _useModuleCaching(size_t pid, gtUInt64 ip, gtUInt64 time, bool bIsUser);

    void _dumpModLoadInfoMap();

    void _dumpEvBlkInfo();

    HRESULT _translate_pass1();

    HRESULT _translate_pass2_serialize();

    HRESULT _translate_pass2_sort();

    HRESULT _prepareBlkForSorting(gtUInt32 blkSize, void* pBlk, gtUInt32& numEntries);

    void _printPass2Log();

    HRESULT _processCSS(struct perf_event_header* pHdr,
                        struct CA_PERF_RECORD_SAMPLE& rec,
                        EventMaskType evMask,
                        float weight,
                        ModLoadInfoMap::reverse_iterator modRit);

    HRESULT _getJavaModuleforSample(TiModuleInfo* pModInfo, gtUInt32 pid, gtUInt64 time, gtUInt64 ip);

    bool _removeJavaJncTmpDir(const QString& directory);

    void _addIbsEvent(unsigned int event);
    void _addIbsFetchEventsToMap();
    void _addIbsOpEventsToMap(bool addBr, bool addLS, bool addNB);
    bool _getInlinedFuncInfoListByVa(const ProcessIdType pid, gtVAddr ip, gtVector<gtVAddr>& funcList);

    bool _isTargetPid(gtUInt32 pid);

#ifdef ENABLE_FAKETIMER
    //Read the available fake timer section data
    HRESULT _getFakeTimerInfo();

    //fake timer data
    caperf_section_fake_timer_t m_fakeInfo;
    //Whether the array[core] should substitute the next sample as a software timer sample
    bool* m_aFakeFlags;
#endif

    string m_inputFile;
    gtUInt64 m_sampleType;
    PerfDataReader* m_pPerfDataRdr;
    PidProcessMap m_procMap;
    NameModuleMap m_modMap;
    CpuProfileProcess* m_pCurProc;
    gtUInt64 m_curProcTs;
    PerfRecordHandler_t m_handlers[PERF_RECORD_MAX];
    ModLoadInfoMap m_modLoadInfoMap;
    ModLoadInfoMap::reverse_iterator m_cachedMod;
    int m_cachedPid;
    bool m_bVerb;
    EvtIdPreviousMap m_previousMap;

    // Log file
    FILE* m_pLogFile;

    // CSS stuff
    wstring m_cssFileDir;
    struct calog* m_pCalogCss;
    gtVector<gtUInt64> m_cssBuffer;

    // Statistics
    gtUInt32 m_numFork;
    gtUInt32 m_numComm;
    gtUInt32 m_numMmap;
    gtUInt32 m_numExit;
    gtUInt32 m_numSamples;
    gtUInt32 m_numUnknownSamples;
    gtUInt32 m_numUnknownKernSamples;
    struct timeval m_pass1Start;
    struct timeval m_pass1Stop;
    struct timeval m_pass2Start;
    struct timeval m_pass2Stop;
    struct timeval m_pass2Css;
    gtUInt32 m_family;
    gtUInt32 m_model;
    gtMap<gtUInt32, gtUInt32> m_ibsEventMap;

    // profile scope
    bool m_isPerProcess;

    // Target pid stuff
    PidProcInfoMap m_pidProcInfoMap;
    PidProcInfoMap m_pidProcInfoOrpharnMap;
    gtList<gtUInt32>  m_pidsFilter;

    // EvBlk stuff
    gtUInt64 m_numSampleBlocks;
    gtUInt64 m_lastProcessedEvId;
    gtUInt32 m_curEvBlkOffset;
    gtUInt64 m_lastEvBlkId;
    gtUInt64 m_lastEvBlkRecIndex;
    gtUInt64 m_lastEvBlkNumEntries;
    gtUInt64 m_lastEvBlkStartTs;
    gtUInt64 m_lastEvBlkStopTs;
    gtUInt32 m_pass2Mode;
    gtUInt64 m_lastSortTs;
    EvBlkIdMap m_evBlkIdMap;
    TimeStampRecordMap m_tsRecMap;

    JitTaskInfo   m_javaModInfo;

    // Error/Warning messages
    gtList<string> m_errorList;
    gtList<string> m_warningList;

    typedef gtFlatMap<VAddrRange, ExecutableAnalyzer*> ExecutableAnalyzersMap;

    struct ProcessInfo
    {
        ProcessWorkingSet m_workingSet;
        ExecutableAnalyzersMap m_exeAnalyzers;
        CallGraph m_callGraph;

        ProcessInfo(const wchar_t* pSearchPath = NULL, const wchar_t* pServerList = NULL, const wchar_t* pCachePath = NULL);
        ~ProcessInfo();

        ExecutableAnalyzer* AcquireExecutableAnalyzer(gtVAddr va);
    };

    gtMap<ProcessIdType, ProcessInfo*> m_processInfos;
    wchar_t* m_pSearchPath;
    wchar_t* m_pServerList;
    wchar_t* m_pCachePath;


    ProcessInfo* FindProcessInfo(ProcessIdType pid) const;
    ProcessInfo& AcquireProcessInfo(ProcessIdType pid);
};

#endif //_CAPERFTRANSLATOR_H_
