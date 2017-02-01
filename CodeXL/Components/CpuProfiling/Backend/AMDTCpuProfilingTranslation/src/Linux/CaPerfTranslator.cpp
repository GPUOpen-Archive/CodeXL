//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfTranslator.cpp
/// \brief This is the interface for the CAPERF file translation.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/Linux/CaPerfTranslator.cpp#40 $
// Last checkin:   $DateTime: 2016/04/14 01:44:54 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569055 $
//=====================================================================


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif // HAVE_CONFIG_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cwchar>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>

#ifdef HAVE_LIBELF_GELF_H
    #include <libelf/gelf.h>
#else
    #include <gelf.h>
#endif // HAVE_LIBELF_GELF_H

#include <linux/perf_event.h>
#include <AMDTCpuProfilingBackendUtils/3rdParty/linux/perfStruct.h>

#define HAS_LIBCSS 1

#if HAS_LIBCSS
    #include <AMDTCpuCallstackSampling/inc/CallStackBuilder.h>
#if ENABLE_OLD_PROFILE_WRITER
    #include <AMDTCpuCallstackSampling/inc/CssWriter.h>
#endif
#endif

#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTCpuProfilingRawData/inc/Linux/CaPerfDataReader.h>
#include <AMDTCpuProfilingRawData/inc/Linux/PerfData.h>
#include <AMDTCpuProfilingRawData/inc/RunInfo.h>

#include "CaPerfTranslator.h"

#ifdef HAVE_CALOG
    #include "calog.h"
#endif

#include "../ExecutableAnalyzer.h"

wchar_t smoduleName[OS_MAX_PATH] = { L'\0' };
wchar_t sfunctionName[OS_MAX_PATH] = { L'\0' };
wchar_t sjncName[OS_MAX_PATH] = { L'\0' };
wchar_t sjavaSrcFileName[OS_MAX_PATH] = { L'\0' };
wchar_t sSessionDir[OS_MAX_PATH] = { L'\0' };

TiModuleInfo    gJavaJclModInfo;
bool            gInlineMode = true;



//TODO: [Suravee]: List of todos for data translation
// - Interface for supporting generic PMU supports
// - IBS + DCMISS LOG
// - IBS + BTA LOG


/* NOTE: [Suravee]
 * 1: Enable module aggregation caching
 * 0: Disable
 */
#define _ENABLE_MOD_AGG_CACHED_ 1
#define CSS_DEPTH_MAX 200

// Maximum number of *inlined* functions will be pushed to the call stack
// (Along with these, 1 more non-inlined function will also be pushed to call stack
#define MAX_INLINED_FUNCS 3

class SimpleProcessWorkingSetQuery : public ProcessWorkingSetQuery
{
public:
    SimpleProcessWorkingSetQuery(ProcessWorkingSet& workingSet) : m_workingSet(workingSet) {}

    virtual ExecutableFile* FindModule(gtVAddr va)
    {
        return m_workingSet.FindModule(va);
    }

    virtual unsigned ForeachModule(void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext)
    {
        for (ProcessWorkingSet::const_iterator it = m_workingSet.begin(), itEnd = m_workingSet.end(); it != itEnd; ++it)
        {
            pfnProcessModule(*it->second, pContext);
        }

        return m_workingSet.GetModulesCount();
    }

private:
    ProcessWorkingSet& m_workingSet;
};

CaPerfTranslator::ProcessInfo::ProcessInfo(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath) :
    m_workingSet(true, pSearchPath, pServerList, pCachePath)
{
    m_exeAnalyzers.reserve(64);
}

CaPerfTranslator::ProcessInfo::~ProcessInfo()
{
    for (ExecutableAnalyzersMap::iterator it = m_exeAnalyzers.begin(), itEnd = m_exeAnalyzers.end(); it != itEnd; ++it)
    {
        ExecutableAnalyzer* pExeAnalyzer = it->second;

        if (nullptr != pExeAnalyzer)
        {
            delete pExeAnalyzer;
        }
    }
}

ExecutableAnalyzer* CaPerfTranslator::ProcessInfo::AcquireExecutableAnalyzer(gtVAddr va)
{
    ExecutableAnalyzer* pExeAnalyzer = nullptr;
    VAddrRange range = {va, va};

    ExecutableAnalyzersMap::iterator it = m_exeAnalyzers.find(range);

    if (it != m_exeAnalyzers.end())
    {
        pExeAnalyzer = it->second;
    }
    else
    {
        ExecutableFile* pExe = m_workingSet.FindModule(va);

        if (nullptr != pExe)
        {
            range.m_min = pExe->GetLoadAddress();
            range.m_max = range.m_min + static_cast<gtVAddr>(pExe->GetImageSize() - 1);

            ExecutableAnalyzer*& pMapExeAnalyzer = m_exeAnalyzers[range];

            if (nullptr == pMapExeAnalyzer)
            {
                pMapExeAnalyzer = new ExecutableAnalyzer(*pExe);
            }

            pExeAnalyzer = pMapExeAnalyzer;
        }
    }

    return pExeAnalyzer;
}

CaPerfTranslator::ProcessInfo* CaPerfTranslator::FindProcessInfo(ProcessIdType pid) const
{
    gtMap<ProcessIdType, ProcessInfo*>::const_iterator it = m_processInfos.find(pid);
    return (m_processInfos.end() != it) ? it->second : nullptr;
}

CaPerfTranslator::ProcessInfo& CaPerfTranslator::AcquireProcessInfo(ProcessIdType pid)
{
    ProcessInfo* pProcessInfo = FindProcessInfo(pid);

    if (nullptr == pProcessInfo)
    {
        pProcessInfo = new ProcessInfo(m_pSearchPath, m_pServerList, m_pCachePath);
        m_processInfos.insert(std::pair<ProcessIdType, ProcessInfo*>(pid, pProcessInfo));
    }

    return *pProcessInfo;
}

CaPerfTranslator::CaPerfTranslator()
{
    _init();
}


CaPerfTranslator::CaPerfTranslator(const std::string& perfDataPath)
{
    _init();
    m_inputFile = perfDataPath;
}

void CaPerfTranslator::_init()
{
    m_curProcTs = 0;
    m_pCurProc = nullptr;
    m_pPerfDataRdr = nullptr;
    m_cachedMod = m_modLoadInfoMap.rend();
    m_cachedPid = 0;
    m_bVerb = false;

    _clearAllHandlers();

    m_pLogFile = nullptr;
    m_pCalogCss = nullptr;

    m_numFork = 0;
    m_numComm = 0;
    m_numMmap = 0;
    m_numExit = 0;
    m_numSamples = 0;
    m_numUnknownSamples = 0;
    m_numUnknownKernSamples = 0;
    timerclear(&m_pass1Start);
    timerclear(&m_pass1Stop);
    timerclear(&m_pass2Start);
    timerclear(&m_pass2Stop);
    timerclear(&m_pass2Css);

    m_family = 0;
    m_model = 0;

    m_isPerProcess = false;

    m_numSampleBlocks = 0;
    m_curEvBlkOffset = 0;
    m_lastEvBlkId = 0;
    m_lastEvBlkRecIndex = 0;
    m_lastProcessedEvId = 0;
    m_lastEvBlkNumEntries = 0;
    m_lastEvBlkStartTs = 0;
    m_lastEvBlkStopTs = 0;
    m_pass2Mode = PASS2_MODE_SERIAL;
    m_lastSortTs = 0;

#ifdef ENABLE_FAKETIMER
    //Initialize the arrays to NULL
    m_aFakeFlags = nullptr;
    m_fakeInfo.timerFds = nullptr;
    m_fakeInfo.fakeTimerFds = nullptr;
#endif

    m_pSearchPath = nullptr;
    m_pServerList = nullptr;
    m_pCachePath = nullptr;
}

// This function adds an individual IBS event identifier to
// m_ibsEventMap.
void CaPerfTranslator::_addIbsEvent(unsigned int event)
{
    if (m_ibsEventMap.end() == m_ibsEventMap.find(event))
    {
        m_ibsEventMap.insert(gtMap<gtUInt32, gtUInt32>::value_type(event, 0));
    }

}

// Add the IBS Fetch events
void CaPerfTranslator::_addIbsFetchEventsToMap()
{
    unsigned int event;

    for (event = IBS_FETCH_BASE; event <= IBS_FETCH_END; event++)
    {
        switch (event)
        {
            case DE_IBS_FETCH_1G_PAGE:
            case DE_IBS_FETCH_XX_PAGE:
            {
                // ignore the reserved events
                break;
            }

            case DE_IBS_FETCH_L2C_MISS:
            case DE_IBS_ITLB_REFILL_LAT:
            {
                int cpuFamily = m_family;
                int cpuModels = m_model >> 4;

                // Only supported by family:0x15, models:0x60-0x6F
                if (FAMILY_OR == cpuFamily && 0x6 == cpuModels)
                {
                    _addIbsEvent(event);
                }

                break;
            }

            default:
            {
                _addIbsEvent(event);
                break;
            }
        }
    }
}

// Add the IBS Op events
void CaPerfTranslator::_addIbsOpEventsToMap(bool addBr, bool addLS, bool addNB)
{
    unsigned int event;

    if (addBr)
    {
        for (event = IBS_OP_BASE; event <= IBS_OP_END; event++)
        {
            switch (event)
            {
                case DE_IBS_BR_ADDR:
                {
                    // ignore the reserved events
                    break;
                }

                default:
                {
                    _addIbsEvent(event);
                    break;
                }
            }
        }
    }

    if (addLS)
    {
        for (event = IBS_OP_LS_BASE; event <= IBS_OP_LS_END; event++)
        {
            switch (event)
            {
                case DE_IBS_LS_L1_DTLB_RES:
                case DE_IBS_LS_L2_DTLB_RES2:
                case DE_IBS_LS_DC_LIN_ADDR:
                case DE_IBS_LS_DC_PHY_ADDR:
                {
                    // ignore the reserved events
                    break;
                }

                case DE_IBS_LS_DC_LD_RESYNC:
                {
                    int cpuFamily = m_family;
                    int cpuModels = m_model >> 4;

                    // Only supported by family:0x15, models:0x60-0x6F
                    if (FAMILY_OR == cpuFamily && 0x6 == cpuModels)
                    {
                        _addIbsEvent(event);
                    }

                    break;
                }

                default:
                {
                    _addIbsEvent(event);
                    break;
                }
            }
        }
    }

    if (addNB)
    {
        for (event = IBS_OP_NB_BASE; event <= IBS_OP_NB_END; event++)
        {
            _addIbsEvent(event);
        }
    }
}

// If ip belongs to an inlined function, then the vector will be filled
// with any nested inlined caller function addresses in the calling order
bool CaPerfTranslator::_getInlinedFuncInfoListByVa(const ProcessIdType pid, gtVAddr ip, gtVector<gtVAddr>& funcList)
{
    bool result = false;
    ProcessInfo* pProcessInfo = FindProcessInfo(pid);

    if (nullptr != pProcessInfo)
    {
        ExecutableFile* pExecutable = pProcessInfo->m_workingSet.FindModule(ip);

        if (nullptr != pExecutable)
        {
            SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

            if (nullptr != pSymbolEngine)
            {
                gtRVAddr rva = pExecutable->VaToRva(ip);
                gtVector<gtRVAddr> funcRvaList = pSymbolEngine->FindNestedInlineFunctions(rva);

                for (auto& it : funcRvaList)
                {
                    funcList.push_back(pExecutable->RvaToVa(it));
                }

                result = true;
            }
        }
    }

    return result;
}

void CaPerfTranslator::SetDebugSymbolsSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath)
{
    // We use 'free' instead of 'delete' because these strings were created by 'wcsdup'
    //
    if (nullptr != m_pSearchPath)
    {
        free(m_pSearchPath);
    }

    if (nullptr != m_pServerList)
    {
        free(m_pServerList);
    }

    if (nullptr != m_pCachePath)
    {
        free(m_pCachePath);
    }

    m_pSearchPath = (nullptr != pSearchPath) ? wcsdup(pSearchPath) : nullptr;
    m_pServerList = (nullptr != pServerList) ? wcsdup(pServerList) : nullptr;
    m_pCachePath  = (nullptr != pCachePath)  ? wcsdup(pCachePath)  : nullptr;

    // Update the working sets, if there are any.
    for (gtMap<ProcessIdType, ProcessInfo*>::iterator it = m_processInfos.begin(), itEnd = m_processInfos.end(); it != itEnd; ++it)
    {
        it->second->m_workingSet.SetSymbolsSearchPath(pSearchPath, pServerList, pCachePath);
    }
}

inline void CaPerfTranslator::_clearAllHandlers()
{
    // Initialize the record handler dispatcher
    memset(&m_handlers, 0, sizeof(PerfRecordHandler_t) * PERF_RECORD_MAX);
}


CaPerfTranslator::~CaPerfTranslator()
{
    if (m_pPerfDataRdr)
    {
        m_pPerfDataRdr->deinit();
        delete m_pPerfDataRdr;
        m_pPerfDataRdr = nullptr;
    }

    if (m_pLogFile)
    {
        fclose(m_pLogFile);
        m_pLogFile = nullptr;
    }

#ifdef HAVE_CALOG

    if (m_pCalogCss)
    {
        calog_close(&m_pCalogCss);
        m_pCalogCss = nullptr;
    }

#endif

#ifdef ENABLE_FAKETIMER

    if (m_aFakeFlags)
    {
        delete [] m_aFakeFlags;
        m_aFakeFlags = nullptr;
    }

#endif

    for (gtMap<ProcessIdType, ProcessInfo*>::iterator it = m_processInfos.begin(), itEnd = m_processInfos.end(); it != itEnd; ++it)
    {
        ProcessInfo* pProcessInfo = it->second;

        if (nullptr != pProcessInfo)
        {
            delete pProcessInfo;
        }
    }

    // We use 'free' instead of 'delete' because these strings were created by 'wcsdup'
    //
    if (nullptr != m_pSearchPath)
    {
        free(m_pSearchPath);
    }

    if (nullptr != m_pServerList)
    {
        free(m_pServerList);
    }

    if (nullptr != m_pCachePath)
    {
        free(m_pCachePath);
    }
}


HRESULT CaPerfTranslator::dumpPerfData(const std::string& perfDataPath)
{
    HRESULT retVal = S_OK;

    // Sanity check
    if (perfDataPath.empty())
    {
        return E_INVALIDPATH;
    }

    // Initialize the reader
    if (!m_pPerfDataRdr
        &&  S_OK != (retVal = _setupReader(perfDataPath)))
    {
        return retVal;
    }

    m_pPerfDataRdr->dumpHeaderSections();
    m_pPerfDataRdr->dumpData();

    m_pPerfDataRdr->deinit();

    return S_OK;
}


bool
CaPerfTranslator::_removeJavaJncTmpDir(const gtString& directory)
{
    osDirectory tmpDir;
    tmpDir.setDirectoryFullPathFromString(directory);

    if (tmpDir.exists())
    {
        gtList<osFilePath> di_list;
        tmpDir.getSubDirectoriesPaths(osDirectory::SORT_BY_NAME_ASCENDING, di_list);

        // for each process id directory in the given directory
        for (auto& jncDirPath : di_list)
        {
            gtString jncDir;
            bool rc = jncDirPath.getFileName(jncDir);

            if (rc)
            {
                pid_t pid = 0;
                jncDir.toIntNumber(pid);

                if (0 == pid)
                {
                    continue;
                }

                PidProcessMap::iterator it = m_procMap.find(pid);

                if (it == m_procMap.end())
                {
                    // This pid does not belong this profile run, ignore
                    continue;
                }

                // If the Java process is still running, don't remove the tmp JNC dir
                char filename[PATH_MAX];
                memset(filename, 0, sizeof(filename));
                snprintf(filename, sizeof(filename), "/proc/%d/status", pid);

                if (0 == (access(filename, F_OK)))
                {
                    continue;
                }

                if (jncDirPath.exists())
                {
                    osDirectory aJncDir(jncDirPath);
                    aJncDir.deleteRecursively();
                }
            }
        }
    }

    return true;
}


HRESULT CaPerfTranslator::translatePerfDataToCaData(const std::string& outPath, const std::string& perfDataPath, bool bVerb)
{
    HRESULT retVal = S_OK;

    // Sanity check
    if (perfDataPath.empty())
    {
        if (m_inputFile.empty())
        {
            return E_INVALIDPATH;
        }
    }
    else
    {
        m_inputFile = perfDataPath;
    }

    // Read the Java JIT data
    wchar_t javaJncTmpDir[OS_MAX_PATH] = L"/tmp/.codexl-java";
    gtString outDir;

    if (outPath.empty())
    {
        // outPath is empty, consider base directory of
        // CAPERF file as output directory
        gtString inputFile;
        inputFile.fromUtf8String(m_inputFile);
        osFilePath tmpPath(inputFile);
        outDir = tmpPath.fileDirectoryAsString();
    }
    else
    {
        // outPath is non-empty, use it as output directory
        outDir.fromUtf8String(outPath);
    }

    m_javaModInfo.ReadJavaJitInformation(javaJncTmpDir, outDir.asCharArray());

    // Initialize the reader
    if (!m_pPerfDataRdr
        &&  S_OK != (retVal = _setupReader(m_inputFile)))
    {
        return E_INVALIDDATA;
    }

    if (S_OK != m_pPerfDataRdr->getCpuInfo(&m_family, &m_model))
    {
        return E_FAIL;
    }

    // Fill-in the m_ibsEventMap with the derived IBS events
    _addIbsFetchEventsToMap();
    _addIbsOpEventsToMap(true, true, true);

    m_bVerb = bVerb;

    if (m_bVerb)
    {
        m_pPerfDataRdr->dumpHeaderSections();
    }

    m_sampleType = m_pPerfDataRdr->getAttrSampleType();

    if (m_pPerfDataRdr->getNumCpus() == 0)
    {
        return E_FAIL;
    }

    if (m_pLogFile)
    {
        fprintf(m_pLogFile, "Translating                : %s\n",
                m_inputFile.c_str());
    }

    // Setup the process for storing vmlinux and kernel modules
    CpuProfileProcess* pProc = getProcess(-1);

    if (!pProc)
    {
        return E_FAIL;
    }

    // Get target pids
    if (S_OK != _getTargetPids())
    {
        return E_FAIL;
    }

    // Read the data from the RI file
    gtString riFileStr;
    riFileStr.fromUtf8String(m_inputFile);
    osFilePath riFilePath(riFileStr);
    riFilePath.setFileExtension(L"ri");

    RunInfo runInfo;
    int hr = fnReadRIFile(riFilePath.asString().asCharArray(), &runInfo);

    if ((hr == S_OK) &&
        (0 == runInfo.m_profScope.compareNoCase(L"Single Application")))
    {
        m_isPerProcess = true;
    }

    _clearAllHandlers();

    retVal = _translate_pass1();

    if (S_OK != retVal)
    {
        return retVal;
    }

    //-------------------------------------------------------
    // NOTE [Suravee]
    // Perf doesn't guarantee the order of fork records. At this point
    // we should have most of the FORK records in place.  Therefore, we
    // are trying to resolve the orpharn processes here.
    PidProcInfoMap::iterator zit = m_pidProcInfoOrpharnMap.begin(), zend = m_pidProcInfoOrpharnMap.end();

    for (; zit != zend; ++zit)
    {
        PidProcInfoMap::iterator pit = m_pidProcInfoMap.find(zit->second.ppid);

        if (pit != m_pidProcInfoMap.end())
        {
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Resolved Orpharn pid : %u, ppid : %u", zit->first, pit->first);
            }

            m_pidProcInfoMap.insert(PidProcInfoMap::value_type(
                                        zit->first, zit->second));
        }
    }

    //-------------------------------------------------------

    _dumpModLoadInfoMap();

    _dumpEvBlkInfo();

    //-------------------------------------------------------

    _clearAllHandlers();

#ifdef ENABLE_FAKETIMER

    // Get Fake timer info. If we are using
    if (S_OK == _getFakeTimerInfo())
    {
        m_pass2Mode = PASS2_MODE_SORT;
    }

#endif

    // In case there is only one block, we just use serial
    // translation
    if (m_evBlkIdMap.size() == 1)
    {
        m_pass2Mode = PASS2_MODE_SERIAL;
    }

    switch (m_pass2Mode)
    {
        case PASS2_MODE_SERIAL:
            retVal = _translate_pass2_serialize();
            break;

        case PASS2_MODE_SORT:
            retVal = _translate_pass2_sort();
            break;

        default:
            OS_OUTPUT_DEBUG_LOG(L"Unknown pass2 translation mode", OS_DEBUG_LOG_ERROR);
    }

    // Fix for BUG400722: Removed the residual JNC files created from
    // Java profiling on temp dir.
    _removeJavaJncTmpDir(javaJncTmpDir);

    // If there are no samples collected, return appropriate
    // error code.
    if (!m_numSamples)
    {
        retVal = E_NODATA;
    }

    return retVal;
}


HRESULT CaPerfTranslator::_translate_pass1()
{
    HRESULT retVal = S_OK;
    struct perf_event_header hdr;
    const void* pBuf = nullptr;

    if (m_bVerb)
    {
        fprintf(stdout, "============== PASS1 ==============\n");
    }

    m_handlers[PERF_RECORD_MMAP] =
        (PerfRecordHandler_t) &CaPerfTranslator::process_PERF_RECORD_MMAP;

    m_handlers[PERF_RECORD_COMM] =
        (PerfRecordHandler_t) &CaPerfTranslator::process_PERF_RECORD_COMM;

    m_handlers[PERF_RECORD_FORK] =
        (PerfRecordHandler_t) &CaPerfTranslator::process_PERF_RECORD_FORK;

    m_handlers[PERF_RECORD_EXIT] =
        (PerfRecordHandler_t) &CaPerfTranslator::process_PERF_RECORD_EXIT;

    m_handlers[PERF_RECORD_SAMPLE] =
        (PerfRecordHandler_t) &CaPerfTranslator::preprocess_PERF_RECORD_SAMPLE_into_block;

    gettimeofday(&m_pass1Start, nullptr);

    //-------------------------------------------------------------------------

    gtUInt32 recIndx = 0;
    gtUInt32 offset = 0;

    if (m_pPerfDataRdr->getFirstRecord(&hdr, &pBuf, &offset) != S_OK)
    {
        return E_FAIL;
    }

    m_curEvBlkOffset = offset;

    do
    {
        if (hdr.size == 0 || !pBuf)
        {
            retVal = E_UNEXPECTED;
            break;
        }

        // Check for invalide type
        if (hdr.type >= PERF_RECORD_MAX)
        {
            if (m_bVerb)
                fprintf(stdout, "Error: Unknown PERF record type (%x)\n",
                        hdr.type);

            continue;
        }

        if (m_handlers[hdr.type] != nullptr)
        {
            (this->*(m_handlers[hdr.type]))(&hdr, pBuf, offset, recIndx);
        }

        recIndx++;

    }
    while (m_pPerfDataRdr->getNextRecord(&hdr, &pBuf, &offset) == S_OK);

    // Insert last EvBlk into EvBlkMap
    if (0 != m_lastEvBlkId)
    {
        EvBlkKey key(m_lastEvBlkStartTs, m_lastEvBlkId);
        EvBlkInfo data(
            m_numSampleBlocks - 1 /*index*/,
            m_lastEvBlkId /*evId*/,
            m_curEvBlkOffset /*offset*/,
            offset - m_curEvBlkOffset /*bytes*/,
            m_lastEvBlkNumEntries /*num*/,
            m_lastEvBlkStartTs /*startTs*/,
            (m_lastEvBlkStopTs - m_lastEvBlkStartTs) /*period*/);

        m_evBlkIdMap[m_lastEvBlkId][m_lastEvBlkStartTs] = data;
    }

    //-------------------------------------------------------
    // Stop PASS1 timer and log timing in logfile
    gettimeofday(&m_pass1Stop, nullptr);

    if (m_pLogFile)
    {
        struct timeval diff;
        timersub(&m_pass1Stop, &m_pass1Start, &diff);
        fprintf(m_pLogFile, "Pass1 Time                 : %lu sec, %lu usec\n",
                diff.tv_sec, diff.tv_usec);
    }

    return retVal;
}


// In this mode, we just go through PERF_EVENT_SAMPLE records
// sequentially.
HRESULT CaPerfTranslator::_translate_pass2_serialize()
{
    HRESULT retVal = S_OK;
    struct perf_event_header hdr;
    const void* pBuf = nullptr;
    gtUInt32 offset = 0;
    gtUInt32 recIndx = 0;

    // Re-initialize some stuff
    m_numSampleBlocks = 0;
    m_lastEvBlkId = 0;
    m_lastEvBlkNumEntries = 0;
    m_lastEvBlkStartTs = 0;
    m_lastEvBlkStopTs = 0;

    // Clear the cache
    m_cachedMod = m_modLoadInfoMap.rend();
    m_cachedPid = 0;


    if (m_bVerb)
    {
        OS_OUTPUT_DEBUG_LOG(L"============== PASS2 SERIALIZE ==============", OS_DEBUG_LOG_INFO);
    }

    gettimeofday(&m_pass2Start, nullptr);

    m_handlers[PERF_RECORD_SAMPLE] =
        (PerfRecordHandler_t) &CaPerfTranslator::process_PERF_RECORD_SAMPLE;

    if (m_pPerfDataRdr->getFirstRecord(&hdr, &pBuf, &offset) != S_OK)
    {
        return E_FAIL;
    }

    do
    {
        if (hdr.size == 0 || !pBuf)
        {
            retVal = E_UNEXPECTED;
            break;
        }

        // Check for invalide type
        if (hdr.type >= PERF_RECORD_MAX)
        {
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error: Unknown PERF record type (%x)", hdr.type);
            }

            continue;
        }

        if (m_handlers[hdr.type] != nullptr)
        {
            (this->*(m_handlers[hdr.type]))(&hdr, pBuf, offset, recIndx);
        }

        recIndx++;

    }
    while (m_pPerfDataRdr->getNextRecord(&hdr, &pBuf, &offset) == S_OK);

    //-------------------------------------------------------

    // Check if the translation finished prematurely
    if (!m_pPerfDataRdr->isEndOfFile())
    {
        std::stringstream ss;
        ss << "Error: Data translation ended prematurely. ";
        ss << "Processed " << m_pPerfDataRdr->getCurrentDataSize() << " out of ";
        ss << m_pPerfDataRdr->getDataSectionSize() << " bytes (";
        ss << (double)(m_pPerfDataRdr->getCurrentDataSize() * 100) / m_pPerfDataRdr->getDataSectionSize();
        ss << "%).";

        m_errorList.push_back(ss.str());
    }

    //-------------------------------------------------------
    // Print last EvBlk into EvBlkMap
    if (m_bVerb && 0 != m_lastEvBlkId)
    {
        std::stringstream pre;

        pre << " ----------- Sample Block " << std::setw(5) << m_numSampleBlocks - 1;
        pre << ", rec:[" << std::setw(5) << std::dec <<  m_lastEvBlkRecIndex << " to " << recIndx << "]";
        pre << ", size:" << std::setw(5) << std::dec << offset - m_curEvBlkOffset;
        pre << ", num:" << std::setw(5) << std::dec << m_lastEvBlkNumEntries;
        pre << ": id:0x" <<  std::hex << std::setw(16) << std::setfill('0') << m_lastEvBlkId;
        pre << ": time:" <<  std::hex << "(0x"
            << std::setw(16) << std::setfill('0') << m_lastEvBlkStartTs
            << " to 0x"
            << std::setw(16) << std::setfill('0') << m_lastEvBlkStopTs
            << ") = 0x"
            << std::setw(16) << std::setfill('0') << (m_lastEvBlkStopTs - m_lastEvBlkStartTs);
        pre << " -----------" ;

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"%hs", pre.str().c_str());
    }

    //-------------------------------------------------------
    gettimeofday(&m_pass2Stop, nullptr);

    _printPass2Log();

    return retVal;
}


// In this mode, use the block information (from pass1) to sort
// samples in timestamp order.
//
// Raw Data Characteristics (Assumptions)
// - Each sample block is corresponded to a particular EvId
// - Sample blocks from the same EvId maintian time ordering in the data file.
// - Samples in each block are already sorted by time.
//
// Algorithm
// 1. Sort sample blocks into buckets according to EvId (EvBlkIdMap).
// 2. Within each bucket, the blocks are sorted by time. (TsEvBlkMap)
//    For each bucket, the first block is consider as "Active" block.
// 3. For each active block, go through each sample record,
//    and put in them in the "m_tsRecMap" which use timestamp
//    as key.
// 4. Take the first item of m_tsRecMap and process them normally.
// 5. Once processed, check if the active block where the sample is
//    comming from is already exhaust.  If so, bring in the next
//    active block.
// 6. If the we exhaust all block from an EvId, remove the EvId from the
//    EvBlkIdMap.
HRESULT CaPerfTranslator::_translate_pass2_sort()
{
    HRESULT retVal = S_OK;

    // Re-initialize block stuff which was used in pass1
    m_numSampleBlocks = 0;
    m_lastEvBlkId = 0;
    m_lastEvBlkNumEntries = 0;
    m_lastEvBlkStartTs = 0;
    m_lastEvBlkStopTs = 0;

    // Clear the cache
    m_cachedMod = m_modLoadInfoMap.rend();
    m_cachedPid = 0;

    if (m_bVerb)
    {
        OS_OUTPUT_DEBUG_LOG(L"============== PASS2 SORT ==============", OS_DEBUG_LOG_INFO);
    }

    gettimeofday(&m_pass2Start, nullptr);
    //-------------------------------------------------------
    // Initializing
    EvBlkIdMap::iterator eit  = m_evBlkIdMap.begin();
    EvBlkIdMap::iterator eend = m_evBlkIdMap.end();

    for (int j = 0; eit != eend; j++, eit++)
    {
        // For each EvId, get the first block
        TsEvBlkMap::iterator tit  = eit->second.begin();

        // Pull-in data of the first block from each EvId
        if (S_OK != _getSampleBlockFromOffset(
                tit->second.offset,
                tit->second.bytes,
                eit->first,
                &(tit->second.pData)))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Couldn't get block. EvId=0x%016llx, offset=%u, size=%u",
                                       eit->first, tit->second.offset, tit->second.bytes);
            return E_FAIL;
        }

        // The put all records in the block into sorting map
        if (S_OK != _prepareBlkForSorting(
                tit->second.bytes,
                tit->second.pData,
                tit->second.numUnProcessed))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to prepare block for evID 0x%016llx", eit->first);
            return E_FAIL;
        }
    }

    //-------------------------------------------------------
    // Processing
    TimeStampRecordMap::iterator it =  m_tsRecMap.begin();

    // Processing sorted PERF_SAMPLE_RECORDs
    while (it != m_tsRecMap.end())
    {
        // Since samples should be sorted in this map, we just take the first one
        char* pTmp = (char*) it->second;

        // Sanity checking to make sure we are not seeing the reverse time
        if (m_lastSortTs > it->first)
        {
            std::stringstream ss;
            ss << "Error : Sort translatoin error. ";
            ss << "(LastTs = 0x" << std::hex << std::setw(16) << std::setfill('0') << m_lastSortTs;
            ss << ",CurTs = 0x" << std::hex << std::setw(16) << std::setfill('0') << it->first;
            m_errorList.push_back(ss.str());
        }
        else
        {
            m_lastSortTs = it->first;
        }

        struct perf_event_header* pHdr = (struct perf_event_header*) pTmp;

        void* ptr = (void*)(pTmp + sizeof(struct perf_event_header));

        process_PERF_RECORD_SAMPLE(pHdr, ptr, 0, 0);

        // Remove the already processed record
        m_tsRecMap.erase(it);

        // Decrement the sample counters in each active block
        TsEvBlkMap::iterator tit  = m_evBlkIdMap[m_lastProcessedEvId].begin();

        if (0 == tit->second.numUnProcessed)
        {
            std::stringstream ss;
            ss << "Error: Sort Error. Block for EvID (0x";
            ss << std::hex << std::setw(16) << std::setfill('0') << m_lastProcessedEvId;
            ss << ") is already empty";
            m_errorList.push_back(ss.str());
        }

        tit->second.numUnProcessed--;

        // Check to see if we need to bring in a new block
        if (0 == tit->second.numUnProcessed)
        {
            // Remove the block (constant time)
            m_evBlkIdMap[m_lastProcessedEvId].erase(tit);

            // Check if still have more blocks for the EvId just processes
            if (0 != m_evBlkIdMap[m_lastProcessedEvId].size())
            {
                //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Bring in new block for evID 0x%016llx", m_lastProcessedEvId);

                // Bring in the next block
                tit = m_evBlkIdMap[m_lastProcessedEvId].begin();

                // Pull-in data of the first block from each EvId
                if (S_OK != _getSampleBlockFromOffset(
                        tit->second.offset,
                        tit->second.bytes,
                        m_lastProcessedEvId,
                        &(tit->second.pData)))
                {
                    std::stringstream ss;
                    ss << "Error: Sort failed to pull in new block for ";
                    ss << "EvId=0x" << std::hex << std::setw(16) << std::setfill('0') << m_lastProcessedEvId;
                    ss << ", offset=" << tit->second.offset;
                    ss << ", size=" << tit->second.bytes;
                    m_errorList.push_back(ss.str());
                    return E_FAIL;
                }

                // Then put all records in the block into sorting map
                if (S_OK != _prepareBlkForSorting(
                        tit->second.bytes,
                        tit->second.pData,
                        tit->second.numUnProcessed))
                {
                    std::stringstream ss;
                    ss << "Error: Sort failed to prepare block for evID 0x";
                    ss << std::hex << std::setw(16) << std::setfill('0') << m_lastProcessedEvId;
                    m_errorList.push_back(ss.str());
                    return E_FAIL;
                }
            }
            else
            {
                //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"No more block for evID 0x%016llx", m_lastProcessedEvId);

                // Remove the EvID
                m_evBlkIdMap.erase(m_lastProcessedEvId);

                //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Now we have only %d EvId left", m_evBlkIdMap.size());
            }
        } // if

        // Next
        it =  m_tsRecMap.begin();
    } // while

    //-------------------------------------------------------
    gettimeofday(&m_pass2Stop, nullptr);


    //-------------------------------------------------------
    // Sanity check
    if (m_tsRecMap.size() != 0)
    {
        std::stringstream ss;
        ss << "Error: Left over samples in merge map. (" << m_tsRecMap.size() << ")";
        m_errorList.push_back(ss.str());
    }

    if (m_evBlkIdMap.size() != 0)
    {
        std::stringstream ss;
        ss << "Error: Left over sample blocks. ( " << m_evBlkIdMap.size() << ")";
        m_errorList.push_back(ss.str());

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Left over blocks (%d) :", m_evBlkIdMap.size());
            EvBlkIdMap::iterator eit  = m_evBlkIdMap.begin();
            EvBlkIdMap::iterator eend = m_evBlkIdMap.end();

            for (int j = 0; eit != eend; j++, eit++)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"evId=0x%016llx, numBlock=%d", eit->first, eit->second.size());
                TsEvBlkMap::iterator tit  = eit->second.begin();
                TsEvBlkMap::iterator tend = eit->second.end();

                for (int k = 0; tit != tend; tit++, k++)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    Index:%5u, numUnProcessed:%5u",
                                               tit->second.index, tit->second.numUnProcessed);
                }
            }
        }
    }

    //-------------------------------------------------------
    // Check if the translation finished prematurely
    if (!m_pPerfDataRdr->isEndOfFile())
    {
        std::stringstream ss;
        ss << "Error: Data translation ended prematurely. ";
        ss << "Processed " << m_pPerfDataRdr->getCurrentDataSize() << " bytes out of ";
        ss << m_pPerfDataRdr->getDataSectionSize() << "bytes.(";
        ss << (double)(m_pPerfDataRdr->getCurrentDataSize() * 100) / m_pPerfDataRdr->getDataSectionSize();
        ss << ")";

        m_errorList.push_back(ss.str());
    }

    _printPass2Log();

    return retVal;
}


// This function go to the specified offset in the caperf data file
// which should be the beginning of the sample block and read in all
// sample record from the block
HRESULT CaPerfTranslator::_getSampleBlockFromOffset(
    gtUInt32 blkOffset,
    gtUInt32 blkSize,
    gtUInt64 evId,
    void** ppData)
{
    (void)(evId); // unused
    int retVal = S_OK;
    struct perf_event_header hdr;
    const void* pBuf = nullptr;
    char* pTmp = nullptr;
    gtUInt32 totSize = 0;

#if 0
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"_getSampleBlockFromOffset: evId=0x%016llx, offset=%u, size=%u", evId, blkOffset, blkSize);
#endif

    *ppData = calloc(1, blkSize);

    if ((*ppData) == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // TODO: [Suravee]: We probably can read the whole size in at once.

    // Get the first record of the block
    if (S_OK != m_pPerfDataRdr->getRecordFromOffset(blkOffset, &hdr, &pBuf))
    {
        return E_FAIL;
    }

    pTmp = (char*) *ppData;

    do
    {
        // Store into the buffer
        gtUInt32 hdrSize = sizeof(struct perf_event_header);
        memcpy(pTmp, &hdr, hdrSize);
        pTmp += hdrSize;

        memcpy(pTmp, pBuf, hdr.size - hdrSize);
        pTmp += hdr.size - hdrSize;

        totSize += (hdr.size);

        if (totSize >= blkSize)
        {
            break;
        }

    }
    while (m_pPerfDataRdr->getNextRecord(&hdr, &pBuf) == S_OK);

    return retVal;
}


void CaPerfTranslator::_printPass2Log()
{
    if (m_pLogFile)
    {
        struct timeval diff;
        timersub(&m_pass2Stop, &m_pass2Start, &diff);
        fprintf(m_pLogFile, "Pass2 Time                 : %lu sec, %lu usec\n",
                diff.tv_sec, diff.tv_usec);
        fprintf(m_pLogFile, "    - CSS time             : %lu sec, %lu usec\n",
                m_pass2Css.tv_sec, m_pass2Css.tv_usec);
        fprintf(m_pLogFile, "Num Fork                   : %u\n", m_numFork);
        fprintf(m_pLogFile, "Num Comm                   : %u\n", m_numComm);
        fprintf(m_pLogFile, "Num Mmap                   : %u\n", m_numMmap);
        fprintf(m_pLogFile, "Num Exit                   : %u\n", m_numExit);
        fprintf(m_pLogFile, "Num Samples                : %u\n", m_numSamples);
        fprintf(m_pLogFile, "Num Unknown Samples        : %u\n", m_numUnknownSamples);
        fprintf(m_pLogFile, "Num Unknown Kernel Samples : %u\n", m_numUnknownKernSamples);
        fprintf(m_pLogFile, "Num IBS Samples            : \n");
        fprintf(m_pLogFile, "Translation Mode           : %u\n", m_pass2Mode);

        gtMap<gtUInt32, gtUInt32>::iterator it = m_ibsEventMap.begin();
        gtMap<gtUInt32, gtUInt32>::iterator iend = m_ibsEventMap.end();

        for (; it != iend; it++)
        {
            fprintf(m_pLogFile, " 0x%x : %u\n", it->first, it->second);
        }

        fprintf(m_pLogFile, "Num of Errors              : %u\n", static_cast<unsigned int>(m_errorList.size()));
        gtList<std::string>::iterator eit = m_errorList.begin();
        gtList<std::string>::iterator eEnd = m_errorList.end();

        for (; eit != eEnd; eit++)
        {
            fprintf(m_pLogFile, "    - %s\n", eit->c_str());
        }
    }
}


ModLoadInfoMap::reverse_iterator CaPerfTranslator::_getModuleForSample(gtUInt32 pid, gtUInt64 time, gtUInt64 ip, bool bIsUser)
{
    // Look for module with the corresponded
    // ip and timestamp
    ModLoadInfoMap::reverse_iterator rit = m_modLoadInfoMap.rbegin();
    ModLoadInfoMap::reverse_iterator rend = m_modLoadInfoMap.rend();

    if (!pid || !time || !ip)
    {
        return rend;
    }

    if (!bIsUser)
    {
        ModKey key(time, ip, -1);

        for (; rit != rend; rit++)
        {
            if (-1 != rit->first.pid)
            {
                continue;
            }

            if (rit->first < key || rit->first == key)
            {
                break;
            }
        }
    }
    else
    {
        ModKey key(time, ip, pid);

        for (; rit != rend; rit++)
        {
            if ((int) pid != rit->first.pid)
            {
                continue;
            }

            if (rit->first < key || rit->first == key)
            {
                break;
            }
        }
    }

    if (rit == rend)
    {
        return rend;
    }

    // Also check the len
    gtUInt64 endRng = rit->first.addr + rit->second.len;

    // NOTE: This is a hack to handle the MMAP of kernel.kallsyms stuff;
    //       MMAP: time:0x0000000000000000, addr:0x0000000000000000, pid:-1, tid:0,
    //             len:0xffffffff9fffffff, pgoff:0xffffffff81000000, filename:[kernel.kallsyms]_text
    if (rit->first.pid == -1 && rit->second.len > rit->first.addr)
    {
        endRng = -1ULL;
    }

    if (ip >= endRng)
    {
        return rend;
    }

    CpuProfileModule* pMod = rit->second.pMod;

    if (nullptr != pMod && 0 == pMod->m_base)
    {
        pMod->m_base = rit->first.addr;

        if (-1ULL != endRng && 0x7fffffffULL >= rit->second.len)
        {
            pMod->m_size = static_cast<gtUInt32>(rit->second.len);
        }

        AcquireProcessInfo(pid).m_workingSet.AddModule(pMod->m_base, pMod->m_size, pMod->getPath().asCharArray());
    }

    return rit;
}

const FunctionSymbolInfo* CaPerfTranslator::getFunctionSymbol(ProcessIdType pid, gtVAddr ip, CpuProfileModule* pMod)
{
    const FunctionSymbolInfo* pFuncInfo = nullptr;
    bool handleInline = true;

    ProcessInfo* pProcessInfo = FindProcessInfo(pid);

    if (nullptr != pProcessInfo)
    {
        ExecutableAnalyzer* pExeAnalyzer = pProcessInfo->AcquireExecutableAnalyzer(ip);

        if (nullptr != pExeAnalyzer)
        {
            pFuncInfo = pExeAnalyzer->FindAnalyzedFunction(ip, handleInline);
        }
        else
        {
            ExecutableFile* pExecutable = pProcessInfo->m_workingSet.FindModule(ip);

            if (nullptr != pExecutable)
            {
                if (nullptr != pMod)
                {
                    pMod->m_size = pExecutable->GetImageSize();
                }

                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                if (nullptr != pSymbolEngine)
                {
                    pFuncInfo = pSymbolEngine->LookupFunction(pExecutable->VaToRva(ip), nullptr, handleInline);
                }
            }
        }
    }

    return pFuncInfo;
}


CpuProfileModule* CaPerfTranslator::getModule(const std::string& modName)
{
    CpuProfileModule* pRet = nullptr;
    gtString wModName;
    wModName.fromUtf8String(modName);

    NameModuleMap::iterator it = m_modMap.find(wModName);

    if (it != m_modMap.end())
    {
        pRet = &(it->second);
    }
    else
    {
        CpuProfileModule mod;
        mod.setPath(wModName);

        // Find the bitness of the module
        bool is32Bit = false;
        _getModuleBitness(modName, &is32Bit);
        mod.m_is32Bit = is32Bit;
        mod.m_moduleId = AtomicAdd(m_nextModuleId, 1);

        m_modMap.insert(NameModuleMap::value_type(wModName, mod));

        // Re-find
        it = m_modMap.find(wModName);

        if (it != m_modMap.end())
        {
            pRet = &(it->second);
        }
    }

    return pRet;
}


CpuProfileProcess* CaPerfTranslator::getProcess(ProcessIdType pid)
{
    CpuProfileProcess* pRet = nullptr;

    PidProcessMap::iterator it = m_procMap.find(pid);

    if (it != m_procMap.end())
    {
        pRet = &(it->second);
    }
    else
    {
        CpuProfileProcess proc;
        m_procMap.insert(PidProcessMap::value_type(pid, proc));

        // Re-find
        it = m_procMap.find(pid);

        if (it != m_procMap.end())
        {
            pRet = &(it->second);
        }
    }

    return pRet;
}


int CaPerfTranslator::process_PERF_RECORD_MMAP(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index)
{
    (void)(offset); // unused

    if (nullptr == pHdr || nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    struct CA_PERF_RECORD_MMAP* pRec = (struct CA_PERF_RECORD_MMAP*) ptr;

    // Baskar: BUG378236: Module View does not come up.
    // Some MMAP records generated by PERF, does not contain any module names.
    // This leads to incorrect entries in "[MODDATA]" section in .ebp file.
    // Ignore these MMAP records.
    if (pRec->filename[0] == '\0')
    {
        return E_INVALIDARG;
    }

    if (m_bVerb)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO,
                                   L"rec#:%5u, MMAP: hdr.misc:%d, time:0x%016llx, addr:0x%016llx, pid:%d, tid:%d, "
                                   L"len:0x%016llx, pgoff:0x%016llx, filename:%s",
                                   index, pHdr->misc, 0, pRec->addr, pRec->pid, pRec->tid,
                                   pRec->len, pRec->pgoff, pRec->filename);
    }

    // Store module loading info
    ModKey key(0, pRec->addr + pRec->pgoff, pRec->pid);

    ModInfo info(pRec->len, pRec->pgoff, pRec->filename);

    info.pMod = getModule(pRec->filename);

    if (info.pMod && m_pCurProc)
    {
        // Since module and process must have the same bitness
        m_pCurProc->m_is32Bit = info.pMod->m_is32Bit;
        info.pProc = m_pCurProc;

        // NOTE [Suravee]
        // Process name in COMM record is truncated.
        // Here, we are trying to replace them
        // with the proper name from MMAP record.

        // Check if has process has fullpath
        gtString commName = m_pCurProc->getPath();

        if (commName[0] != L'/')
        {
            // Try to check if the module name contains the truncated COMM name.
            gtString modName = info.pMod->getPath();
            gtString tmp(L"/");
            tmp += commName;

            if (modName.find(tmp) != -1)
            {
                // Replaceing the name
                m_pCurProc->setPath(modName);
            }
        }
    }

    info.pMod->m_modType = CpuProfileModule::UNMANAGEDPE;
    info.instanceId = AtomicAdd(m_nextModInstanceId, 1);

    m_modLoadInfoMap.insert(ModLoadInfoMap::value_type(key, info));

    m_numMmap++;

    return S_OK;
}


int CaPerfTranslator::process_PERF_RECORD_COMM(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index)
{
    (void)(offset); // unused

    if (nullptr == pHdr || nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    struct CA_PERF_RECORD_COMM* pRec = (struct CA_PERF_RECORD_COMM*) ptr;

    if (m_bVerb)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"rec#:%5u, COMM: hdr.misc:%d, time:0x%016llx, pid:%d, tid:%d, comm:%s\n",
                                   index, pHdr->misc, 0, pRec->pid, pRec->tid, pRec->comm);
    }

    CpuProfileProcess* pProc = getProcess(pRec->pid);

    if (nullptr == pProc)
    {
        return E_FAIL;
    }

    // Set process name
    std::string procName(pRec->comm);
    gtString wProcName;
    wProcName.fromUtf8String(procName);
    pProc->setPath(wProcName);

    m_pCurProc = pProc;
    m_numComm++;

    // Set process name in PidProcInfoMap
    PidProcInfoMap::iterator pit = m_pidProcInfoMap.find(pRec->pid);

    if (pit != m_pidProcInfoMap.end())
    {
        // Update procInfo of target PIDs
        pit->second.comm =  procName;
    }
    else
    {
        // Try in orpharn map instead
        PidProcInfoMap::iterator pit = m_pidProcInfoOrpharnMap.find(pRec->pid);

        if (pit != m_pidProcInfoOrpharnMap.end())
        {
            // Update procInfo of target PIDs
            pit->second.comm =  procName;
        }
    }

    return S_OK;
}


ModLoadInfoMap::reverse_iterator CaPerfTranslator::getModuleForSample(struct perf_event_header* pHdr,
                                                                      gtUInt32 pid, gtUInt64 time, gtUInt64 ip,
                                                                      bool bStat, bool bIsUser)
{
    (void)(pHdr); // unused

    ModLoadInfoMap::reverse_iterator modRit = _getModuleForSample(pid, time, ip, bIsUser);

    if (modRit == m_modLoadInfoMap.rend())
    {
        if (bIsUser)
        {
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Warning: Unknown Sample: ip:0x%016lx, time:0x%016lx", ip, time);
            }

            if (bStat)
            {
                m_numUnknownSamples++;
            }
        }
        else
        {
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Warning: Unknown Kernel Sample: ip:0x%016lx, time:0x%016lx", ip, time);
            }

            if (bStat)
            {
                m_numUnknownKernSamples++;
            }
        }
    }

    return modRit;
}

void CaPerfTranslator::_addSampleToProcessAndModule(CpuProfileProcess* pProc,
                                                    gtUInt64 ldAddr, gtUInt32 funcSize, CpuProfileModule* pMod,
                                                    gtUInt64 ip,
                                                    gtUInt32 pid, gtUInt32 tid, gtUInt32 cpu,
                                                    gtUInt32 event, gtUInt32 umask, gtUInt32 os, gtUInt32 usr,
                                                    gtUInt32 count,
                                                    const FunctionSymbolInfo* pFuncInfo)

{
    EventMaskType evMask = _getEvmask(event, umask, os, usr);

    if ((nullptr == pProc) || (nullptr == pMod))
    {
        return;
    }

    // Add sample to process
    if (pProc)
    {
        SampleKey key(cpu, evMask);
        // BUG372074: Here .EBP (PROCESSDATA section) data file is containing wrong value for
        // #samples for a module, whenever more than 4 event are
        // selected for profiling in linux.
        // Since we cannot profile more than 4 events
        // at a time, when more than 4 events are selected
        // we do multiplexing and hence we normalize each event
        // counter value using this parameter "count".
        // This "count" is now used to compute the normalized
        // value in addSamples(...);
        pProc->addSamples(key, count);
    }

    SampleInfo sampInfo(ip, pid, tid, cpu, evMask);

    if (pMod->m_modType == CpuProfileModule::UNMANAGEDPE)
    {
        gtString funcName;
        gtString srcFileName;
        unsigned srcLineNum = 0U;
        bool handleInline = true;
        gtUInt32 functionId = UNKNOWN_FUNCTION_ID;

        if (nullptr != pFuncInfo)
        {

            ProcessInfo* pProcessInfo = FindProcessInfo(pid);

            if (nullptr != pProcessInfo)
            {
                ExecutableFile* pExecutable = pProcessInfo->m_workingSet.FindModule(ip);

                if (nullptr != pExecutable)
                {
                    pMod->m_isDebugInfoAvailable = pExecutable->IsDebugInfoAvailable();

                    gtRVAddr rva = pExecutable->VaToRva(sampInfo.address);
                    SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                    if (nullptr != pSymbolEngine)
                    {
                        gtRVAddr inlineRva = pSymbolEngine->TranslateToInlineeRVA(rva);
                        gtVAddr addr = sampInfo.address - rva + inlineRva;
                        CpuProfileFunction* pFunc = pMod->findFunction(addr);

                        if (nullptr == pFunc || pMod->isUnchartedFunction(*pFunc))
                        {
                            if (nullptr != pFuncInfo->m_pName)
                            {
                                funcName = pFuncInfo->m_pName;
                            }

                            SourceLineInfo sourceLine;

                            if (pSymbolEngine->FindSourceLine(rva, sourceLine, handleInline))
                            {
                                srcFileName = sourceLine.m_filePath;
                                srcLineNum = sourceLine.m_line;
                                sampInfo.address = pExecutable->RvaToVa(sourceLine.m_rva);
                            }

                            functionId = pFuncInfo->m_funcId;
                        }
                        else
                        {
                            if (nullptr != pFuncInfo->m_pName)
                            {
                                funcName = pFuncInfo->m_pName;
                            }

                            sampInfo.address = pExecutable->RvaToVa(inlineRva);
                        }
                    }
                }
            }
        }

        pMod->recordSample(sampInfo, count, ldAddr, funcSize, funcName, gtString(), srcFileName, srcLineNum, functionId);
    }
    else if (pMod->m_modType == CpuProfileModule::JAVAMODULE)
    {
        // Debug info is stored in .jnc files
        pMod->m_isDebugInfoAvailable = true;

        gtString javaFuncName(gJavaJclModInfo.pFunctionName);
        gtString javaSrcFileName(gJavaJclModInfo.pJavaSrcFileName);

        gtString jncNameStr;
        jncNameStr.appendFormattedString(L"%u/%S", pid, gJavaJclModInfo.pJncName);

        // fwprintf(stderr, L"recordSample: java mod name : %S\n", pMod->getPath().c_str());
        // fwprintf(stderr, L"insert java function  : %S\n", javaFuncName.c_str());

        gtUInt32 functionId = (pFuncInfo != nullptr) ? pFuncInfo->m_funcId : 0;

        pMod->recordSample(sampInfo,
                           count,
                           gJavaJclModInfo.ModuleStartAddr,
                           funcSize,
                           javaFuncName,
                           jncNameStr,
                           javaSrcFileName,
                           0,
                           functionId);
    }
}


void CaPerfTranslator::_dumpModLoadInfoMap()
{
    if (!m_bVerb)
    {
        return;
    }

    // Print Module loading info
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"==== Num of modules = %d ====", m_modLoadInfoMap.size());
    ModLoadInfoMap::iterator it = m_modLoadInfoMap.begin(), itEnd = m_modLoadInfoMap.end();

    for (int i = 0; it != itEnd; ++it, ++i)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO,
                                   L"%4d: pid:%08d, time:0x%016llx, addr:0x%016llx, len:0x%08llx, pgoff:0x%08llx, filename:%s (0x%llx)",
                                   i,
                                   it->first.pid, it->first.time, it->first.addr, it->second.len,
                                   it->second.pgoff, it->second.name.c_str(),
                                   it->second.pProc);
    }
}


void CaPerfTranslator::_dumpEvBlkInfo()
{
    if (!m_bVerb)
    {
        return;
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"==== EventId Block Histogram (numBuckets = %d) ====", m_evBlkIdMap.size());
    EvBlkIdMap::iterator eit  = m_evBlkIdMap.begin();
    EvBlkIdMap::iterator eend = m_evBlkIdMap.end();

    for (int j = 0; eit != eend; j++, eit++)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Bucket:%4d | evId:0x%016llx, numEntries:%d", j, eit->first, eit->second.size());

        int cpu = 0;
        gtUInt32 event;
        gtUInt32 umask;
        gtUInt32 os;
        gtUInt32 usr;

        if (S_OK != m_pPerfDataRdr->getEventAndCpuFromSampleId(
                eit->first, &cpu, &event, &umask, &os, &usr))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"( Invalid eventId 0x%016x )", eit->first);
        }
        else
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"( cpu:%3d, event:%4xh, umask:%4xh )", cpu, event, umask);
        }

        TsEvBlkMap::iterator tit  = eit->second.begin(), tend = eit->second.end();

        for (int k = 0; tit != tend; ++tit, ++k)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    Index:%5d | EvBlkIndex:%4u, ts:0x%016llx, offset:%10u, num:%u",
                                       k, tit->second.index, tit->first, tit->second.offset, tit->second.num);
        }
    }
}


int CaPerfTranslator::preprocess_PERF_RECORD_SAMPLE_into_block(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index)
{
    (void)(pHdr); // unused
    struct CA_PERF_RECORD_SAMPLE rec;
    char* pCur = (char*)ptr;

    memset(&rec, 0, sizeof(struct CA_PERF_RECORD_SAMPLE));

    if (nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    if (m_sampleType & PERF_SAMPLE_IP)
    {
        rec.ip = *((u64*)pCur);
        pCur += 8;
    }

    if (m_sampleType & PERF_SAMPLE_TID)
    {
        rec.pid = *((u32*)pCur);
        pCur += 4;

        rec.tid = *((u32*)pCur);
        pCur += 4;
    }

    if (m_sampleType & PERF_SAMPLE_TIME)
    {
        rec.time = *((u64*)pCur);
        pCur += 8;

        if (0 == m_lastEvBlkStartTs)
        {
            m_lastEvBlkStartTs = rec.time;
        }
    }

    if (m_sampleType & PERF_SAMPLE_ADDR)
    {
        rec.addr = *((u64*)pCur);
        pCur += 8;
    }


    if (m_sampleType & PERF_SAMPLE_ID)
    {
        rec.id = *((u64*)pCur);

        if (m_lastEvBlkId != rec.id)
        {
            // Print event block info
            if (m_bVerb && 0 != m_lastEvBlkId)
            {
                std::stringstream pre;
                pre << " ----------- Sample Block " << std::setw(5) << m_numSampleBlocks - 1;
                pre << std::setw(5) << std::dec << ", rec:[" << m_lastEvBlkRecIndex << " to " << index << "]";
                pre << ", size:" << std::setw(5) << std::dec << offset - m_curEvBlkOffset;
                pre << ", num:" << std::setw(5) << std::dec << m_lastEvBlkNumEntries;
                pre << ": id:0x" <<  std::hex << std::setw(16) << std::setfill('0') << m_lastEvBlkId;
                pre << ": time:" <<  std::hex << "(0x"
                    << std::setw(16) << std::setfill('0') << m_lastEvBlkStartTs
                    << " to 0x"
                    << std::setw(16) << std::setfill('0') << m_lastEvBlkStopTs
                    << ") = 0x"
                    << std::setw(16) << std::setfill('0') << (m_lastEvBlkStopTs - m_lastEvBlkStartTs);
                pre << " -----------" ;
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"%hs\n", pre.str().c_str());
            }


            // Insert EvBlk into EvBlkMap
            if (0 != m_lastEvBlkId)
            {
                EvBlkKey key(m_lastEvBlkStartTs, m_lastEvBlkId);
                EvBlkInfo data(
                    m_numSampleBlocks - 1 /*index*/,
                    m_lastEvBlkId /*evId*/,
                    m_curEvBlkOffset /*offset*/,
                    offset - m_curEvBlkOffset /*bytes*/,
                    m_lastEvBlkNumEntries /*num*/,
                    m_lastEvBlkStartTs /*startTs*/,
                    (m_lastEvBlkStopTs - m_lastEvBlkStartTs) /*period*/);

                m_evBlkIdMap[m_lastEvBlkId][m_lastEvBlkStartTs] = data;
            }

            // Reset EvBlk stuff
            m_curEvBlkOffset = offset;
            m_lastEvBlkId = rec.id;
            m_lastEvBlkRecIndex = index;
            m_numSampleBlocks++;
            m_lastEvBlkNumEntries = 1;
            m_lastEvBlkStartTs = rec.time;
            m_lastEvBlkStopTs = rec.time;
        }
        else
        {
            m_lastEvBlkStopTs = rec.time;
            m_lastEvBlkNumEntries++;
        }

        pCur += 8;
    }

    if (m_bVerb)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"rec#:%5u, SAMP: offset:%x, time:0x%016llx, pid:%d, id:0x%016llx, ip:0x%016llx",
                                   index, offset, rec.time, rec.pid, rec.id, rec.ip);
    }

    return S_OK;
}


HRESULT CaPerfTranslator::_prepareBlkForSorting(gtUInt32 blkSize, void* pBlk, gtUInt32& numEntries)
{
    HRESULT retVal = S_OK;
    struct CA_PERF_RECORD_SAMPLE rec;
    struct perf_event_header* pRec = nullptr;
    char* pCur = nullptr;
    gtUInt32 totSize = 0;

    numEntries = 0;

    if (!pBlk | (0 == blkSize))
    {
        return E_FAIL;
    }

    pRec = (struct perf_event_header*)pBlk;

    while (totSize < blkSize)
    {
        if (pRec->type == PERF_RECORD_SAMPLE)
        {
            memset(&rec, 0, sizeof(struct CA_PERF_RECORD_SAMPLE));

            // Set pCur to the beginning of the record
            pCur = (char*)pRec;

            // Skip the header
            pCur += sizeof(struct perf_event_header);

            if (m_sampleType & PERF_SAMPLE_IP)
            {
                rec.ip = *((u64*)pCur);
                pCur += 8;
            }

            if (m_sampleType & PERF_SAMPLE_TID)
            {
                rec.pid = *((u32*)pCur);
                pCur += 4;

                rec.tid = *((u32*)pCur);
                pCur += 4;
            }

            if (m_sampleType & PERF_SAMPLE_TIME)
            {
                rec.time = *((u64*)pCur);
                pCur += 8;
            }

            // Store record in the buffer
            m_tsRecMap.insert(TimeStampRecordMap::value_type(rec.time, pRec));
            numEntries++;
        }

        // Update size have been processed
        unsigned int curRecSize = pRec->size;
        totSize += curRecSize;

        // Update pRec to the next record
        pRec = (struct perf_event_header*)(((char*)pRec) + curRecSize);
    } // while

    // Sanity check
    if (totSize != blkSize)
    {
        std::stringstream ss;
        ss << "Error, Block size miss match (blkSize = " << blkSize;
        ss << ", readSize = " << totSize;
        m_errorList.push_back(ss.str());
    }

    return retVal;
}


HRESULT CaPerfTranslator::process_PERF_RECORD_SAMPLE(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index)
{
    if (nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    struct CA_PERF_RECORD_SAMPLE rec;

    char* pCur = (char*)ptr;

    float weight = 1.0;

    int cpu = 0;

    gtUInt32 event;

    gtUInt32 umask;

    gtUInt32 os;

    gtUInt32 usr;

    EventMaskType evMask;

    std::stringstream pre;

    std::stringstream ss;

    memset(&rec, 0, sizeof(struct CA_PERF_RECORD_SAMPLE));

    if (m_bVerb)
    {

        ss << std::setw(5) << "rec#:" << index ;

        ss << ",SAMP: ";

        if (offset)
        {
            ss << "offset:" << std::setw(10) << offset;
        }
    }

    if (m_bVerb)
    {
        ss << ", hdr.misc:0x" << std::hex << pHdr->misc;
    }

    if (m_bVerb && m_sampleType & PERF_SAMPLE_TIME)
    {
        ss << ", time:0x" << std::hex << std::setw(16) << std::setfill('0') << *((u64*)(pCur + 8 + 4 + 4));
    }

    if (m_sampleType & PERF_SAMPLE_IP)
    {
        rec.ip = *((u64*)pCur);
        pCur += 8;

        if (m_bVerb)
        {
            ss << ", ip:0x" << std::hex << std::setw(16) << std::setfill('0') << rec.ip;
        }
    }

    if (m_sampleType & PERF_SAMPLE_TID)
    {
        rec.pid = *((u32*)pCur);
        pCur += 4;

        if (m_bVerb)
        {
            ss << ", pid:" << std::dec << std::setw(10) << rec.pid;
        }

        rec.tid = *((u32*)pCur);
        pCur += 4;

        if (m_bVerb)
        {
            ss << ", tid:" << std::dec << std::setw(10) << rec.tid;
        }
    }

    if (m_sampleType & PERF_SAMPLE_TIME)
    {
        rec.time = *((u64*)pCur);
        pCur += 8;

        if (PASS2_MODE_SERIAL == m_pass2Mode)
        {
            if (0 == m_lastEvBlkStartTs)
            {
                m_lastEvBlkStartTs = rec.time;
            }
        }
    }

    if (m_sampleType & PERF_SAMPLE_ADDR)
    {
        rec.addr = *((u64*)pCur);
        pCur += 8;

        if (m_bVerb)
        {
            //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"addr:0x%016llx", rec.ip);
            ss << ", addr:0x" << std::hex << std::setw(16) << std::setfill('0') << rec.ip;
        }
    }

    if (m_sampleType & PERF_SAMPLE_ID)
    {
        rec.id = *((u64*)pCur);
        m_lastProcessedEvId = rec.id;

        if (PASS2_MODE_SERIAL == m_pass2Mode)
        {
            if (m_lastEvBlkId != rec.id)
            {
                // Print event block info
                if (m_bVerb && 0 != m_lastEvBlkId)
                {
                    pre << " ----------- Sample Block " << std::setw(5) << m_numSampleBlocks - 1;
                    pre << std::setw(5) << std::dec << ", rec:[" << m_lastEvBlkRecIndex << " to " << index << "]";
                    pre << ", size:" << std::setw(5) << std::dec << offset - m_curEvBlkOffset;
                    pre << ", num:" << std::setw(5) << std::dec << m_lastEvBlkNumEntries;
                    pre << ": id:0x" <<  std::hex << std::setw(16) << std::setfill('0') << m_lastEvBlkId;
                    pre << ": time:" <<  std::hex << "(0x"
                        << std::setw(16) << std::setfill('0') << m_lastEvBlkStartTs
                        << " to 0x"
                        << std::setw(16) << std::setfill('0') << m_lastEvBlkStopTs
                        << ") = 0x"
                        << std::setw(16) << std::setfill('0') << (m_lastEvBlkStopTs - m_lastEvBlkStartTs);
                    pre << " -----------" ;
                }

                m_lastEvBlkId = rec.id;
                m_lastEvBlkRecIndex = index;
                m_numSampleBlocks++;
                m_lastEvBlkNumEntries = 1;
                m_lastEvBlkStartTs = rec.time;
                m_lastEvBlkStopTs = rec.time;
            }
            else
            {
                m_lastEvBlkStopTs = rec.time;
                m_lastEvBlkNumEntries++;
            }
        }

        pCur += 8;

        if (m_bVerb)
        {
            //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"id:0x%016llx", rec.id);
            ss << ", id:0x" << std::hex << std::setw(16) << std::setfill('0') << rec.id;

        }
    }

    if (m_bVerb)
    {
        if (!pre.str().empty())
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"%hs", pre.str().c_str());
        }

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"%hs", ss.str().c_str());
    }

    if (m_sampleType & PERF_SAMPLE_CPU)
    {
        rec.cpu = *((u32*)pCur);
        pCur += 4;

        // This is for the "res" as list in teh perf_event.h
        pCur += 4;

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"cpu:0x08%x", rec.cpu);
        }
    }

    //-----------------------------------------------------

    if (m_sampleType & PERF_SAMPLE_PERIOD)
    {
        rec.period = *((u64*)pCur);
        pCur += 8;

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"period:0x%016llx", rec.period);
        }
    }

    if (m_sampleType & PERF_SAMPLE_READ)
    {
        gtUInt64 read_format = 0;
        read_format |= PERF_FORMAT_TOTAL_TIME_ENABLED;
        read_format |= PERF_FORMAT_TOTAL_TIME_RUNNING;
        read_format |= PERF_FORMAT_ID;

        // Baskar: PERF_FORMAT_GROUP is not used by CaPerf
        if (read_format & PERF_FORMAT_GROUP)
        {
            return E_NOTSUPPORTED;
        }

        // Read the event count value
        rec.values.value = *((u64*)pCur);
        pCur += 8;

        if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED)
        {
            rec.values.time_enabled = *((u64*)pCur);
            pCur += 8;
        }

        if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING)
        {
            rec.values.time_running = *((u64*)pCur);
            pCur += 8;
        }

        if (read_format && PERF_FORMAT_ID)
        {
            rec.values.id = *((u64*)pCur);
            pCur += 8;
        }

        EvtIdPreviousMap::iterator prevIt =  m_previousMap.find(rec.id);
        PreviousTimes prevTime;

        //If we have a record of the previous times for this event id
        if (m_previousMap.end() != prevIt)
        {
            prevTime = prevIt->second;
        }
        else
        {
            prevTime.previousEnabledTime = 0;
            prevTime.previousRunningTime = 0;
        }

        //Calculate the weight of this sample, based on the running time given by perf, rounded up
        // Baskar: Fix for BUG368631: junk values shown in CSS on RHEL 6.2.
        // Only if we know the previous running and enabled time, we should
        // compute the weight
        if ((!prevTime.previousEnabledTime) && (!prevTime.previousRunningTime))
        {
            weight = double(rec.values.time_enabled - prevTime.previousEnabledTime)
                     / double(rec.values.time_running - prevTime.previousRunningTime) + 0.5;
        }

        //Save the times to get the correct diff for the next sample weight
        prevTime.previousEnabledTime = rec.values.time_enabled;
        prevTime.previousRunningTime = rec.values.time_running;
        m_previousMap[rec.id] = prevTime;

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"read_format:{value:0x%016llx, time_enabled:0x%016llx, "
                                       L"time_running:0x%016llx, id:0x%016llx}, weighted: %0.2f",
                                       rec.values.value, rec.values.time_enabled,
                                       rec.values.time_running, rec.values.id, weight);
        }
    }

    if (m_sampleType & PERF_SAMPLE_STREAM_ID)
    {
        rec.stream_id = *((u64*)pCur);
        pCur += 8;

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"stream_id:0x%016llx", rec.stream_id);
        }
    }

    if (m_sampleType & PERF_SAMPLE_CALLCHAIN)
    {
        rec.callchain = (struct ip_callchain*)pCur;

        // Sanity check
        if (!rec.callchain)
        {
            return E_UNEXPECTED;
        }

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"nr_callchain:%03lu", rec.callchain->nr);
        }

        // Sanity check. The first entry of call chain must be PERF_CONTEXT_xxx enums
        if ((0 < rec.callchain->nr) && (rec.callchain->ips[0] < PERF_CONTEXT_MAX))
        {
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid callchain info (nr = %lu(%lx))",
                                           rec.callchain->nr, rec.callchain->nr);
            }

            return E_FAIL;
        }

        pCur += sizeof(gtUInt64) * (rec.callchain->nr + 1);
    }
    else
    {
        rec.callchain = nullptr;
    }

    if (m_sampleType & PERF_SAMPLE_RAW)
    {
        rec.raw_size = *((u32*)pCur);
        pCur += 4;

        rec.raw_data = (void*)pCur;

        if (m_bVerb)
        {
            gtString outMsg;
            outMsg.appendFormattedString(L"raw_data[%u]: ", rec.raw_size);

            for (size_t i = 0 ; i < rec.raw_size; i++)
            {
                char* tmp = pCur;
                outMsg.appendFormattedString(L"%02x ", 0xff & *tmp);
                pCur += 1; //next character
            }

            OS_OUTPUT_DEBUG_LOG(outMsg.asCharArray(), OS_DEBUG_LOG_INFO);
        }
        else
        {
            // Skip raw records
            pCur += (rec.raw_size);
        }
    }

    ModLoadInfoMap::reverse_iterator modRit;
    bool bCached = false;
    bool bIsUser = ((pHdr->misc & PERF_RECORD_MISC_USER) != 0);

    bool isJavaProcess = false;
    CpuProfileProcess* pJavaProc = nullptr;
    CpuProfileModule* pJavaMod = nullptr;

    gtVAddr funcBaseAddr = 0;
    gtUInt32 funcSize = 0;
    const FunctionSymbolInfo* pFuncInfo = nullptr;

#if _ENABLE_MOD_AGG_CACHED_
    /* MODULE AGGREGATION CACHING:
     * We cache the pid and module that matched the last sample.
     * This greatly help reducing the data processing time.
     */
    // Check if sample belong to the last cached module

    if (_useModuleCaching(rec.pid, rec.ip, rec.time, bIsUser))
    {
        modRit = m_cachedMod;
        bCached = true;

        funcBaseAddr = modRit->first.addr;

        pFuncInfo = getFunctionSymbol(rec.pid, rec.ip, modRit->second.pMod);

        if (nullptr != pFuncInfo)
        {
            funcBaseAddr += pFuncInfo->m_rva;
            funcSize = pFuncInfo->m_size;
        }
    }
    else
#endif // _ENABLE_MOD_AGG_CACHED_
    {
        // Java module samples only falls in user-space
        modRit = getModuleForSample(pHdr, rec.pid, rec.time, rec.ip, true, bIsUser);

        if (modRit == m_modLoadInfoMap.rend())
        {
            // Check for java module,
            pJavaMod = getJavaModuleforSample(&gJavaJclModInfo, rec.pid, rec.time, rec.ip);

            if (nullptr != pJavaMod)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Found Java Module pid(%d), ip(0x%lx)", rec.pid, rec.ip);

                pJavaProc = getProcess(rec.pid);

                if (!pJavaProc)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"failed to get CpuProfileProcess for Pid %d", rec.pid);
                    return E_UNEXPECTED;
                }

                if (!m_pPerfDataRdr)
                {
                    return E_FAIL;
                }

                if (S_OK != m_pPerfDataRdr->getEventAndCpuFromSampleId(rec.id, &cpu, &event, &umask, &os, &usr))
                {
                    return E_INVALIDDATA;
                }

                isJavaProcess = true;

                TiModuleInfo modInfo;
                modInfo.processID = rec.pid;
                modInfo.sampleAddr = rec.ip;
                modInfo.deltaTick = rec.time;
                modInfo.instanceId = 0;
                m_javaModInfo.GetModuleInfo(&modInfo);

                FunctionSymbolInfo funcInfo;
                funcInfo.m_funcId = modInfo.instanceId;

                pFuncInfo = &funcInfo;
            }
            else
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Could not find Module for pid(%d), ip(0x%lx)", rec.pid, rec.ip);
                return E_FAIL;
            }
        }
        else
        {
            funcBaseAddr = modRit->first.addr;

            pFuncInfo = getFunctionSymbol(rec.pid, rec.ip, modRit->second.pMod);

            if (nullptr != pFuncInfo)
            {
                funcBaseAddr += pFuncInfo->m_rva;
                funcSize = pFuncInfo->m_size;
            }
        }

        if (bIsUser)
        {
            m_cachedPid = rec.pid;
        }
        else
        {
            m_cachedPid = -1;
        }

        m_cachedMod = modRit;

#ifdef _DEBUG_MOD_CACHE_

        if (m_cachedMod != m_modLoadInfoMap.rend())
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"-- New cache %d, 0x%016llx, 0x%016llx",
                                       m_cachedPid, m_cachedMod->first.time, m_cachedMod->first.addr);
        }

#endif
    }

    if (m_bVerb)
    {
        gtString wModName = modRit->second.pMod->getPath();
        std::string modName;
        wModName.asUtf8(modName);

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L", MmapTime:0x%016lx, MmapAddr:0x%016lx, MmapLen:0x%016lx, (%hs)%hs",
                                   modRit->first.time, modRit->first.addr, modRit->second.len,
                                   modName.c_str(), (bCached ? " (CACHED)" : ""));

        if (rec.callchain)
        {
            for (size_t i = 0; i < rec.callchain->nr; i++)
            {
                std::string tmp;

                if (rec.callchain->ips[i] >= PERF_CONTEXT_MAX)
                {
                    switch (rec.callchain->ips[i])
                    {
                        case PERF_CONTEXT_HV:
                            tmp = "PERF_CONTEXT_HV";
                            break;

                        case PERF_CONTEXT_KERNEL:
                            tmp = "PERF_CONTEXT_KERNEL";
                            break;

                        case PERF_CONTEXT_USER:
                            tmp = "PERF_CONTEXT_USER";
                            break;

                        case PERF_CONTEXT_GUEST:
                            tmp = "PERF_CONTEXT_GUEST";
                            break;

                        case PERF_CONTEXT_GUEST_KERNEL:
                            tmp = "PERF_CONTEXT_GUEST_KERNEL";
                            break;

                        case PERF_CONTEXT_GUEST_USER:
                            tmp = "PERF_CONTEXT_GUEST_USER";
                            break;

                        default:
                            tmp = "Invalid PERF context";
                    }
                }
                else
                {
                    tmp = "return address";
                }

                if (i >= CSS_DEPTH_MAX)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"CSS depth exceed %d ......", i);
                    break;
                }

                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"callchain %03d: %hs: 0x%016lx", i , tmp.c_str(), rec.callchain->ips[i]);
            }
        }
    }

    if (!m_pPerfDataRdr)
    {
        return E_FAIL;
    }

    if (S_OK != m_pPerfDataRdr->getEventAndCpuFromSampleId(rec.id, &cpu, &event, &umask, &os, &usr))
    {
        return E_INVALIDDATA;
    }

#ifdef ENABLE_FAKETIMER
    // Handling fake timer evId
    int f_cpu = 0;
    gtUInt32 f_event;
    gtUInt32 f_umask;
    gtUInt32 f_os;
    gtUInt32 f_usr;

    //If there is a fake timer that should generate real css data for the software timer
    if ((nullptr != m_fakeInfo.timerFds) && (m_fakeInfo.timerFds[cpu] == rec.id))
    {
        //Set the flag so the next sample will also be saved as software timer data
        m_aFakeFlags[cpu] = true;
        //Don't record the software timer event
        return S_FALSE;
    }

#endif

    // IBS Fetch event
    if (event == IBS_FETCH_BASE)
    {
        // Populate trans_fetch
        struct ibs_fetch_sample trans_fetch;
        gtUInt32* pTmp = (gtUInt32*) rec.raw_data;
        pTmp++; // Skipping the first 4 bytes

        trans_fetch.ibs_fetch_ctl_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_fetch_ctl_low        = 0x%08x", *pTmp); }

        pTmp++;
        trans_fetch.ibs_fetch_ctl_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_fetch_ctl_high       = 0x%08x", *pTmp); }

        pTmp++;
        trans_fetch.ibs_fetch_lin_addr_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_fetch_lin_addr_low   = 0x%08x", *pTmp); }

        pTmp++;
        trans_fetch.ibs_fetch_lin_addr_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_fetch_lin_addr_high  = 0x%08x", *pTmp); }

        pTmp++;
        trans_fetch.ibs_fetch_phys_addr_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_fetch_phys_addr_low  = 0x%08x", *pTmp); }

        pTmp++;
        trans_fetch.ibs_fetch_phys_addr_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_fetch_phys_addr_high = 0x%08x", *pTmp); }

        gtUInt32 ibs_fetch_selected_flag = 0xffffffff;

        gtUInt64 ibsOffset = trans_fetch.ibs_fetch_lin_addr_high;
        ibsOffset = ibsOffset << 32;
        ibsOffset += trans_fetch.ibs_fetch_lin_addr_low;

        if (isJavaProcess)
        {
            trans_ibs_fetch(
                &trans_fetch, ibs_fetch_selected_flag,
                pJavaProc,
                gJavaJclModInfo.ModuleStartAddr,
                gJavaJclModInfo.Modulesize,
                pJavaMod,
                ibsOffset,
                rec.pid, rec.tid, cpu,
                os, usr, weight,
                pFuncInfo);
        }
        else
        {
            trans_ibs_fetch(
                &trans_fetch, ibs_fetch_selected_flag,
                modRit->second.pProc,
                funcBaseAddr,
                funcSize,
                modRit->second.pMod,
                ibsOffset,
                rec.pid, rec.tid, cpu,
                os, usr, weight,
                pFuncInfo);
        }
    }
    // IBS OP events
    else if (event == IBS_OP_BASE)
    {
        // Populate trans_op
        struct ibs_op_sample trans_op;
        gtUInt32* pTmp = (gtUInt32*) rec.raw_data;
        pTmp++; // Skipping the first 4 bytes

        trans_op.ibs_op_ctrl_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_ctrl_low       = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_ctrl_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_ctrl_high      = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_lin_addr_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_lin_addr_low   = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_lin_addr_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_lin_addr_high  = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_data1_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_data1_low      = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_data1_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_data1_high     = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_data2_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_data2_low      = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_data2_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_data2_high     = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_data3_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_data3_low      = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_data3_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_data3_high     = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_ldst_linaddr_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_linaddr_low    = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_ldst_linaddr_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_linaddr_high   = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_phys_addr_low = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_phys_addr_low  = 0x%08x", *pTmp); }

        pTmp++;
        trans_op.ibs_op_phys_addr_high = *pTmp;

        if (m_bVerb) { OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"ibs_op_phys_addr_high = 0x%08x", *pTmp); }

        trans_op.ibs_op_brtgt_addr = 0;

        trans_ibs_op_mask_reserved(m_family, &trans_op);

        if (trans_ibs_op_rip_invalid(&trans_op) == 0)
        {

            gtUInt32 ibs_op_selected_flag = 0xffffffff;
            gtUInt32 ibs_op_ls_selected_flag = 0xffffffff;
            gtUInt32 ibs_op_nb_selected_flag = 0xffffffff;

            gtUInt64 ibsOffset = trans_op.ibs_op_lin_addr_high;
            ibsOffset = ibsOffset << 32;
            ibsOffset += trans_op.ibs_op_lin_addr_low;

            if (isJavaProcess) {
                trans_ibs_op(
                    &trans_op, ibs_op_selected_flag,
                    pJavaProc,
                    gJavaJclModInfo.ModuleStartAddr,
                    gJavaJclModInfo.Modulesize,
                    pJavaMod,
                    ibsOffset, rec.pid, rec.tid, cpu,
                    os, usr, weight,
                    pFuncInfo);

                trans_ibs_op_ls(
                    &trans_op, ibs_op_ls_selected_flag,
                    pJavaProc,
                    gJavaJclModInfo.ModuleStartAddr,
                    gJavaJclModInfo.Modulesize,
                    pJavaMod,
                    ibsOffset, rec.pid, rec.tid, cpu,
                    os, usr, weight,
                    pFuncInfo);

                trans_ibs_op_nb(
                    &trans_op, ibs_op_nb_selected_flag,
                    pJavaProc,
                    gJavaJclModInfo.ModuleStartAddr,
                    gJavaJclModInfo.Modulesize,
                    pJavaMod,
                    ibsOffset, rec.pid, rec.tid, cpu,
                    os, usr, weight,
                    pFuncInfo);
            }
            else
            {
                trans_ibs_op(
                    &trans_op, ibs_op_selected_flag,
                    modRit->second.pProc,
                    funcBaseAddr, funcSize, modRit->second.pMod,
                    ibsOffset, rec.pid, rec.tid, cpu,
                    os, usr, weight,
                    pFuncInfo);

                trans_ibs_op_ls(
                    &trans_op, ibs_op_ls_selected_flag,
                    modRit->second.pProc,
                    funcBaseAddr, funcSize, modRit->second.pMod,
                    ibsOffset, rec.pid, rec.tid, cpu,
                    os, usr, weight,
                    pFuncInfo);

                trans_ibs_op_nb(
                    &trans_op, ibs_op_nb_selected_flag,
                    modRit->second.pProc,
                    funcBaseAddr, funcSize, modRit->second.pMod,
                    ibsOffset, rec.pid, rec.tid, cpu,
                    os, usr, weight,
                    pFuncInfo);
            }

#ifdef HAS_DCMISS
            trans_ibs_op_ls_dcmiss(trans_op);
#endif
#ifdef HAS_BTA
            trans_ibs_op_bta(trans_op);
#endif
        }

    }
    // Non IBS events
    else
    {
#ifdef ENABLE_FAKETIMER
        // Algorithm:
        // Fake timer mode assume the samples are sorted by time.
        // If the fake timer flag for this cpu is set in the previous sample,
        // 1. We will also process this event as timer event along along with
        //    the normal event.
        // 2. If this is the fake-timer and the m_aFakeFlags[cpu] is not set, throw
        //    this sample away.

        //If there is a fake timer in this profile
        if ((nullptr != m_aFakeFlags))
        {
            // If the flag wasn't set, just ignore any fake timer samples
            if ((m_fakeInfo.fakeTimerFds[cpu] == rec.id) && (!m_aFakeFlags[cpu]))
            {
                return S_FALSE;
            }

            //The first sample after a software timer event should be recorded as data for it
            if (m_aFakeFlags[cpu])
            {
                // Handling normal event as timer by grabing the software timer event from
                // the timer given
                if (S_OK != m_pPerfDataRdr->getEventAndCpuFromSampleId(m_fakeInfo.timerFds[cpu], &f_cpu, &f_event, &f_umask, &f_os, &f_usr))
                {
                    return E_INVALIDDATA;
                }

                //Since we're using this fake timer sample, reset the flag
                m_aFakeFlags[cpu] = false;

                // Add fake timer sample
                if (false == isJavaProcess)
                {
                    _addSampleToProcessAndModule(
                        modRit->second.pProc,
                        funcBaseAddr, funcSize, modRit->second.pMod,
                        rec.ip, rec.pid, rec.tid, cpu,
                        f_event, f_umask, f_os, f_usr,
                        weight,
                        pFuncInfo);

                    evMask = _getEvmask(f_event, f_umask, f_os, f_usr);

                    // Process css for fake timer
                    _processCSS(pHdr, rec, evMask, weight, modRit);
                }
                else
                {
                    _addSampleToProcessAndModule(
                        pJavaProc,
                        gJavaJclModInfo.ModuleStartAddr,
                        gJavaJclModInfo.Modulesize,
                        pJavaMod,
                        rec.ip, rec.pid, rec.tid, cpu,
                        f_event, f_umask, f_os, f_usr,
                        weight,
                        pFuncInfo);

                }

                m_numSamples += weight;

                // If this is the fake timer, don't record the sample
                if (m_fakeInfo.fakeTimerFds[cpu] == rec.id)
                {
                    return S_OK;
                }
            }
        }

#endif

        // Process sample for normal events
        if (isJavaProcess)
        {
            _addSampleToProcessAndModule(
                pJavaProc,
                gJavaJclModInfo.ModuleStartAddr,
                gJavaJclModInfo.Modulesize,
                pJavaMod,
                rec.ip, rec.pid, rec.tid, cpu,
                event, umask, os, usr,
                weight,
                pFuncInfo);
        }
        else
        {
            _addSampleToProcessAndModule(
                modRit->second.pProc,
                funcBaseAddr, funcSize, modRit->second.pMod,
                rec.ip, rec.pid, rec.tid, cpu,
                event, umask, os, usr,
                weight,
                pFuncInfo);
        }
    }

    if (false == isJavaProcess)
    {
        evMask = _getEvmask(event, umask, os, usr);

        _processCSS(pHdr, rec, evMask, weight, modRit);
    }

    m_numSamples += weight;

    return S_OK;

}


int CaPerfTranslator::process_PERF_RECORD_FORK(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index)
{
    (void)(offset); // unused
    struct CA_PERF_RECORD_FORK_EXIT rec;

    if (nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    memcpy(&rec, ptr, sizeof(struct CA_PERF_RECORD_FORK_EXIT));

    if (m_bVerb)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"rec#:%5u, FORK: hdr.misc:%d, time:0x%016llx, pid:%d, ppid:%d, tid:%d, ptid:%d",
                                   index, pHdr->misc, rec.time, rec.pid, rec.ppid, rec.tid, rec.ptid);
    }

    //////////////////////////////////////////////
    // Update ProcInfo of target processes tree
    PidProcInfoMap::iterator pit = m_pidProcInfoMap.find(rec.pid);

    if (pit != m_pidProcInfoMap.end())
    {
        // This is the case when:
        // 1. Handle the FORK record for the first time
        //    after getting the target PIDs from header.
        // 2. Handling different threads being forks/clone
        //    of the same process

        // Update procInfo of target PIDs
        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L" --- Found target pid : %u", rec.pid);
        }

        pit->second.ppid = rec.ppid;
        pit->second.tidList.push_back(rec.tid);
    }
    else
    {

        ProcInfo childInfo;
        childInfo.ppid = rec.ppid;
        childInfo.tidList.push_back(rec.tid);

        // Try search in m_procMap in case COMM record
        // is before FORK record.
        PidProcessMap::iterator it = m_procMap.find(rec.pid);

        if (it != m_procMap.end())
        {
            it->second.getPath().asUtf8(childInfo.comm);
        }

        // Try looking for parent processes
        pit = m_pidProcInfoMap.find(rec.ppid);

        if (pit != m_pidProcInfoMap.end())
        {
            // Handling by adding this PID to the list
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L" --- Add child pid %u", rec.pid);
            }

            m_pidProcInfoMap.insert(PidProcInfoMap::value_type(
                                        rec.pid, childInfo));
        }
        else
        {
            // Cannot find parent process, add to orpharn list for now,
            // and we will try to find parent later (at the end of pass1).
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L" --- Add Orpharn pid : %u", rec.pid);
            }

            m_pidProcInfoOrpharnMap.insert(PidProcInfoMap::value_type(
                                               rec.pid, childInfo));
        }
    }

    m_numFork++;

    return S_OK;
}


HRESULT CaPerfTranslator::process_PERF_RECORD_EXIT(struct perf_event_header* pHdr, void* ptr, gtUInt32 offset, gtUInt32 index)
{
    (void)(offset); // unused
    struct CA_PERF_RECORD_FORK_EXIT rec;

    if (nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    memcpy(&rec, ptr, sizeof(struct CA_PERF_RECORD_FORK_EXIT));

    if (m_bVerb)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"rec#:%5u, EXIT: hdr.misc:%d, time:0x%016llx, pid:%d, ppid:%d, tid:%d, ptid:%d",
                                   index, pHdr->misc, rec.time, rec.pid, rec.ppid, rec.tid, rec.ptid);
    }

    m_numExit++;

    return S_OK;
}

//TODO: rename the function name to writeDBOutput
int CaPerfTranslator::writeEbpOutput(const std::string& outputFile)
{
    bool bRet = false;
#if ENABLE_OLD_PROFILE_WRITER
    CpuProfileWriter      profWriter;
#endif
    CpuProfileInfo        profInfo;
    int numMod = 0;
    const PerfEventAttrVec* pAttrVec;
    struct timeval ebp_timerStart;
    struct timeval ebp_timerStop;
    struct timeval css_timerStart;
    struct timeval css_timerStop;

    gettimeofday(&ebp_timerStart, nullptr);

    if (!m_pPerfDataRdr || (pAttrVec = m_pPerfDataRdr->getPerfEventAttrVec()) == nullptr)
    {
        return E_FAIL;
    }

#if 0
    // Useful for debugging process info
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"writeEbpOutput: Num of processes = %d", m_procMap.size());

    for (PidProcessMap::iterator pIt = m_procMap.begin(), pEnd = m_procMap.end(); pIt != pEnd; ++pIt)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"pid:%d comm:%ls numSamples:%llu",
                                   pIt->first, pIt->second.getPath().asCharArray(), pIt->second.getTotal());
    }

#endif

    // We need to count the number of modules which contains samples
    NameModuleMap::iterator it = m_modMap.begin();
    NameModuleMap::iterator itEnd = m_modMap.end();

    for (; it != itEnd; it++)
    {
        if (it->second.getTotal() != 0)
        {
            numMod++;
        }
    }

    profInfo.m_tbpVersion = TBPVER_BEFORE_RI;
    profInfo.m_numCpus    = m_pPerfDataRdr->getNumCpus();
    profInfo.m_cpuFamily  = m_family;
    profInfo.m_cpuModel   = m_model;
    profInfo.m_numEvents  = 0;
    profInfo.m_numModules = numMod;

    ////////////////////////////////
    // Get total number of sample
    profInfo.m_numSamples = 0;
    PidProcessMap::iterator pit  = m_procMap.begin();
    PidProcessMap::iterator pend = m_procMap.end();

    for (; pit != pend; pit++)
    {
        profInfo.m_numSamples += pit->second.getTotal();
    }

    profInfo.m_numMisses = 0; // This is not available

    ////////////////////////////////
    // Get time.
    time_t wrTime = time(nullptr);
    std::string str(ctime(&wrTime));
    profInfo.m_timeStamp.fromUtf8String(str);

    // Remove the newline at the end
    if (profInfo.m_timeStamp[profInfo.m_timeStamp.length() - 1] == L'\n')
    {
        profInfo.m_timeStamp.resize(profInfo.m_timeStamp.length() - 1);
    }

    /////////////////////////////////
    // Get Events

#ifdef ENABLE_FAKETIMER
    bool flagIgnoreNext = false;
#endif

    for (size_t i = 0 ; i < m_pPerfDataRdr->getNumEvents(); i++)
    {
        gtUInt32 event;
        gtUInt32 umask;

#ifdef ENABLE_FAKETIMER

        //If we need to ignore the fake timer event (by not displaying it as a separate column)
        if (flagIgnoreNext)
        {
            flagIgnoreNext = false;
            continue;
        }

#endif

        m_pPerfDataRdr->getEventInfoFromEvTypeAndConfig(
            (*pAttrVec)[i].type, (*pAttrVec)[i].config,
            &event, &umask);

        if (event == IBS_FETCH_BASE)
        {
            gtMap<gtUInt32, gtUInt32>::iterator it = m_ibsEventMap.begin();
            gtMap<gtUInt32, gtUInt32>::iterator iend = m_ibsEventMap.end();

            for (; it != iend; it++)
            {
                if (!IsIbsFetchEvent(it->first))
                {
                    continue;
                }

                profInfo.m_numEvents++;
                EventMaskType evMask = _getEvmask(
                                           it->first,
                                           umask,
                                           !(*pAttrVec)[i].exclude_kernel,
                                           !(*pAttrVec)[i].exclude_user);
                EventEncodeType ev = { evMask , (*pAttrVec)[i].sample_period, 0 };
                profInfo.m_eventVec.push_back(ev);
            }

        }
        else if (event == IBS_OP_BASE)
        {
            gtMap<gtUInt32, gtUInt32>::iterator it = m_ibsEventMap.begin(), iend = m_ibsEventMap.end();

            for (; it != iend; ++it)
            {
                if (!IsIbsOpEvent(it->first))
                {
                    continue;
                }

                profInfo.m_numEvents++;
                EventMaskType evMask = _getEvmask(
                                           it->first,
                                           umask,
                                           !(*pAttrVec)[i].exclude_kernel,
                                           !(*pAttrVec)[i].exclude_user);
                EventEncodeType ev = { evMask , (*pAttrVec)[i].sample_period, 0 };
                profInfo.m_eventVec.push_back(ev);
            }
        }
        else     // NON-IBS
        {
            profInfo.m_numEvents++;
            EventMaskType evMask = _getEvmask(event,
                                              umask,
                                              !(*pAttrVec)[i].exclude_kernel,
                                              !(*pAttrVec)[i].exclude_user);

            EventEncodeType ev = { evMask , (*pAttrVec)[i].sample_period, 0 };
            profInfo.m_eventVec.push_back(ev);

#ifdef ENABLE_FAKETIMER

            //If the current event is the software timer, and we're using the fake timer
            if ((event == TIMER_PERF) && ((*pAttrVec)[i].sample_period == m_fakeInfo.timerNanosec))
            {
                //Ignore the next event, since it will be the fake timer.
                flagIgnoreNext = true;
            }

#endif
        }
    }

    // Get the RI file path
    gtString woutputFile;
    woutputFile.fromUtf8String(outputFile);

    gtString riFileStr;
    riFileStr.fromUtf8String(m_inputFile);
    osFilePath riFilePath(riFileStr);
    riFilePath.setFileExtension(L"ri");

    // Read the data from the RI file
    RunInfo runInfo;
    int hr = fnReadRIFile(riFilePath.asString().asCharArray(), &runInfo);

    if (hr == S_OK)
    {
        // We have the RI data, set current TBP version
        profInfo.m_tbpVersion     = TBPVER_DEFAULT;

        // populate RI info
        profInfo.m_targetPath      = runInfo.m_targetPath;
        profInfo.m_wrkDirectory    = runInfo.m_wrkDirectory;
        profInfo.m_cmdArguments    = runInfo.m_cmdArguments;
        profInfo.m_envVariables    = runInfo.m_envVariables;
        profInfo.m_profType        = runInfo.m_profType;
        profInfo.m_profDirectory   = runInfo.m_profDirectory;
        profInfo.m_profStartTime   = runInfo.m_profStartTime;
        profInfo.m_profEndTime     = runInfo.m_profEndTime;
        profInfo.m_isCSSEnabled    = runInfo.m_isCSSEnabled;
        profInfo.m_cssUnwindDepth  = runInfo.m_cssUnwindDepth;
        profInfo.m_cssScope        = runInfo.m_cssScope;
        profInfo.m_isCssSupportFpo = runInfo.m_isCssSupportFpo;
        profInfo.m_osName          = runInfo.m_osName;
        profInfo.m_profScope       = runInfo.m_profScope;
    }

    // Build up the core topology map.
    unsigned int cpuCount = m_pPerfDataRdr->getNumCpus();
    CoreTopologyMap topMap;

    for (unsigned int j = 0; j < cpuCount; j++)
    {
        CoreTopology topTemp;
        int ret;

        ret = m_pPerfDataRdr->getTopology(j, &(topTemp.processor), &(topTemp.numaNode));

        if (S_OK == ret)
        {
            topMap.insert(CoreTopologyMap::value_type(j, topTemp));
        }
    }

    // write the JCL/JNC files
    // Get the profile session dir
    osFilePath tmpPath(woutputFile);
    m_javaModInfo.WriteJavaJncFiles(tmpPath.fileDirectoryAsString().asCharArray());

    // <jitId, moduleId, instanceId, pid, loadAddr, size>
    gtVector<std::tuple<gtUInt32, gtUInt32, gtUInt32, gtUInt64, gtUInt64, gtUInt64>> inlinedJitInfo;

    // Handle the java inlined methods
    if (gInlineMode)
    {
        for (auto& modit : m_modMap)
        {
            if (CpuProfileModule::JAVAMODULE == modit.second.getModType())
            {
                addJavaInlinedMethods(modit.second, inlinedJitInfo);
            }
        }
    }

    bRet = true;
#if ENABLE_OLD_PROFILE_WRITER
    bRet = profWriter.Write(woutputFile, &profInfo, &m_procMap, &m_modMap, &topMap);
#endif

    gettimeofday(&ebp_timerStop, nullptr);
    memcpy(&css_timerStart, &ebp_timerStop, sizeof(struct timeval));

    m_dbWriter.reset(new ProfilerDataDBWriter);

    if (m_dbWriter)
    {
        if (!m_dbWriter->Initialize(woutputFile))
        {
            m_dbWriter.reset(nullptr);
        }
    }

    if (m_dbWriter)
    {
        // Populate profile session info
        AMDTProfileSessionInfo *info = new (std::nothrow) AMDTProfileSessionInfo;

        if (nullptr != info)
        {
            info->m_cpuFamily = m_family;
            info->m_cpuModel = m_model;

            // populate RI info
            info->m_targetAppPath = runInfo.m_targetPath;
            info->m_targetAppWorkingDir = runInfo.m_wrkDirectory;
            info->m_targetAppCmdLineArgs = runInfo.m_cmdArguments;
            info->m_targetAppEnvVars = runInfo.m_envVariables;
            info->m_sessionType = runInfo.m_profType;
            info->m_sessionDir = runInfo.m_profDirectory;
            info->m_sessionStartTime = runInfo.m_profStartTime;
            info->m_sessionEndTime = runInfo.m_profEndTime;
            info->m_cssEnabled = runInfo.m_isCSSEnabled;
            info->m_unwindDepth = runInfo.m_cssUnwindDepth;
            info->m_unwindScope = runInfo.m_cssScope;
            info->m_cssFPOEnabled = runInfo.m_isCssSupportFpo;
            info->m_systemDetails = runInfo.m_osName;
            info->m_sessionScope = runInfo.m_profScope;
            info->m_coreAffinity = runInfo.m_cpuAffinity;
            info->m_coreCount = runInfo.m_cpuCount;
            info->m_cssInterval = runInfo.m_cssInterval;
            info->m_codexlCollectorVer = runInfo.m_codexlVersion;

            // Get CodeXL version
            osProductVersion cxlVersion;
            osGetApplicationVersion(cxlVersion);
            info->m_codexlTranslatorVer = cxlVersion.toString();

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_SESSION_INFO, (void*)info });
            info = nullptr;
        }

        // Populate topology info
        CPAdapterTopologyMap *topologyInfo = new (std::nothrow) CPAdapterTopologyMap;

        if (nullptr != topologyInfo)
        {
            unsigned int cpuCount = m_pPerfDataRdr->getNumCpus();
            topologyInfo->reserve(cpuCount);

            for (unsigned int j = 0; j < cpuCount; j++)
            {
                gtUInt16 processor = 0;
                gtUInt16 numaNode = 0;

                if (S_OK == m_pPerfDataRdr->getTopology(j, &processor, &numaNode))
                {
                    topologyInfo->emplace_back(j, processor, numaNode);
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_TOPOLOGY_INFO, (void*)topologyInfo });
            topologyInfo = nullptr;
        }

        // Populate sampling config info
        gtVector<std::pair<EventMaskType, gtUInt32>> *samplingConfigInfo = new (std::nothrow) gtVector<std::pair<EventMaskType, gtUInt32>>;

        if (nullptr != samplingConfigInfo)
        {
            samplingConfigInfo->reserve(profInfo.m_numEvents);

            for (const auto& event : profInfo.m_eventVec)
            {
                samplingConfigInfo->emplace_back(event.eventMask, static_cast<gtUInt32>(event.eventCount));
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_SAMPLINGCONFIG_INFO, (void*)samplingConfigInfo });
            samplingConfigInfo = nullptr;
        }

        // Populate core sampling config info
        CPACoreSamplingConfigList *coreSamplingConfigList = new (std::nothrow) CPACoreSamplingConfigList;

        if (nullptr != coreSamplingConfigList)
        {
            gtUInt64 coreAffinity = static_cast<gtUInt64>(runInfo.m_cpuAffinity);
            gtUInt32 numProfiledCores = 0;

            numProfiledCores =  __builtin_popcountll(coreAffinity);
            coreSamplingConfigList->reserve(numProfiledCores * profInfo.m_numEvents);

            const gtUInt32 unusedBitsMask = 0x3FFFFFF;
            gtUInt32 coreIndex = 0;

            while (numProfiledCores)
            {
                if (coreAffinity & (1ull << coreIndex))
                {
                    for (const auto& event : profInfo.m_eventVec)
                    {
                        gtUInt64 id = coreIndex;
                        id = (id << 32) | (event.eventMask & unusedBitsMask);
                        coreSamplingConfigList->emplace_back(id, coreIndex, event.eventMask);
                    }

                    --numProfiledCores;
                }

                ++coreIndex;
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_CORECONFIG_INFO, (void*)coreSamplingConfigList });
            coreSamplingConfigList = nullptr;
        }

        // Populate event info
        gtVector<EventMaskType> *eventInfo = new (std::nothrow) gtVector<EventMaskType>;

        if (nullptr != eventInfo)
        {
            eventInfo->reserve(profInfo.m_numEvents);

            for (const auto& event : profInfo.m_eventVec)
            {
                eventInfo->emplace_back(event.eventMask);
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_EVENT_INFO, (void*)eventInfo });
            eventInfo = nullptr;
        }

        // Populate "unknown" function info
        CPAFunctionInfoList *funcInfoList = new (std::nothrow) CPAFunctionInfoList;

        if (nullptr != funcInfoList)
        {
            // Insert a dummy function as "Unknown Function"
            gtString unknownFuncName = L"Unknown Function";
            funcInfoList->emplace_back(UNKNOWN_FUNCTION_ID, UNKNOWN_MODULE_ID, unknownFuncName, 0, 0);

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_FUNCTION_INFO, (void*)funcInfoList });
            funcInfoList = nullptr;
        }

        // Populate process info and insert into DB.
        CPAProcessList *processList = new (std::nothrow) CPAProcessList;

        if (nullptr != processList)
        {
            processList->reserve(m_procMap.size());

            for (auto procIt : m_procMap)
            {
                if (procIt.second.getTotal())
                {
                    processList->emplace_back(procIt.first, procIt.second.getPath(), procIt.second.m_is32Bit, procIt.second.m_hasCss);
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_PROCESS_INFO, (void*)processList });
            processList = nullptr;
        }

        gtSet<gtUInt64> processThreadList;

        for (const auto& module : m_modMap)
        {
            if (module.second.getTotal())
            {
                for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
                {
                    for (auto aptIt = fit->second.getBeginSample(); aptIt != fit->second.getEndSample(); ++aptIt)
                    {
                        gtUInt32 pid = aptIt->first.m_pid;
                        gtUInt32 threadId = aptIt->first.m_tid;

                        gtUInt64 ptId = pid;
                        ptId = (ptId << 32) | threadId;

                        processThreadList.insert(ptId);
                    }
                }
            }
        }

        CPAProcessThreadList *procThreadIdList = new (std::nothrow) CPAProcessThreadList;

        if (nullptr != procThreadIdList)
        {
            procThreadIdList->reserve(processThreadList.size());

            for (auto& ptId : processThreadList)
            {
                gtUInt32 pid = ptId >> 32;
                gtUInt32 threadId = ptId & 0xFFFFFFFF;

                procThreadIdList->emplace_back(ptId, static_cast<gtUInt64>(pid), threadId);
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_THREAD_INFO, (void*)procThreadIdList });
            procThreadIdList = nullptr;
        }

        processThreadList.clear();

        // Populate module info
        CPAModuleList *moduleList = new (std::nothrow) CPAModuleList;

        if (nullptr != moduleList)
        {
            moduleList->reserve(m_modMap.size());

            for (const auto& m : m_modMap)
            {
                //if (m.second.getTotal())
                {
                    moduleList->emplace_back(m.second.m_moduleId,
                        m.first,
                        m.second.m_size,
                        m.second.m_modType,
                        osIsSystemModule(m.first),
                        m.second.m_is32Bit,
                        m.second.m_isDebugInfoAvailable);
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_MODULE_INFO, (void*)moduleList });
            moduleList = nullptr;
        }

        // Populate module instance info
        // modInstanceInfoMap : Map of < instanceId, tuple of <modName, pid, loadAddr> >
        CPAModuleInstanceList *moduleInstanceList = new (std::nothrow) CPAModuleInstanceList;

        if (nullptr != moduleInstanceList)
        {
            moduleInstanceList->reserve(m_modLoadInfoMap.size());

            for (const auto& modIns : m_modLoadInfoMap)
            {
                // This can be optimized further. Instead of passing module name, we can pass moduleId.
                gtString modName;
                modName.fromUtf8String(modIns.second.name);

                // Update module, so that it can be used while inserting samples
                if (nullptr != modIns.second.pMod)
                {
                    modIns.second.pMod->m_moduleInstanceInfo.emplace_back(modIns.first.pid, modIns.first.addr, modIns.second.instanceId);
                }

                gtUInt32 moduleId = 0;
                const auto& it = m_modMap.find(modName);

                //if (it != m_modMap.end() && it->second.getTotal())
                if (it != m_modMap.end())
                {
                    // Insert into DB only if the module has samples
                    moduleId = it->second.m_moduleId;
                    moduleInstanceList->emplace_back(
                            modIns.second.instanceId,
                            moduleId,
                            static_cast<gtUInt64>(modIns.first.pid),
                            modIns.first.addr);
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_MODINSTANCE_INFO, (void*)moduleInstanceList });
            moduleInstanceList = nullptr;
        }

        // Populate function info
        funcInfoList = new (std::nothrow) CPAFunctionInfoList;

        if (nullptr != funcInfoList)
        {
            // Let us assume one function from each module
            funcInfoList->reserve(m_modMap.size());

            for (auto& module : m_modMap)
            {
                if (module.second.getTotal() > 0)
                {
                    gtUInt32 modId = module.second.m_moduleId;
                    gtUInt64 modLoadAddr = module.second.getBaseAddr();

                    for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
                    {
                        gtString funcName = fit->second.getFuncName();
                        gtUInt64 startOffset = fit->second.getBaseAddr() - modLoadAddr;
                        gtUInt64 size = fit->second.getSize();
                        gtUInt32 funcId = modId;
                        funcId = ((funcId << 16) | fit->second.m_functionId);

                        bool doInsert = funcName.isEmpty() ? false : true;

                        if ((!doInsert) && ((funcId & 0x0000ffff) > 0))
                        {
                            osFilePath aPath(module.second.getPath());
                            aPath.getFileNameAndExtension(funcName);
                            funcName.appendFormattedString(L"!0x%lx", startOffset + modLoadAddr);
                            doInsert = true;
                        }

                        if (doInsert)
                        {
                            funcInfoList->emplace_back(funcId, modId, funcName, startOffset, size);
                        }
                    }
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_FUNCTION_INFO, (void*)funcInfoList });
            funcInfoList = nullptr;
        }

        // Populate sample info
        CPASampeInfoList *sampleList = new (std::nothrow) CPASampeInfoList;

        if (nullptr != sampleList)
        {
            const gtUInt32 unusedBitsMask = 0x3FFFFFF;

            // Assume minimum 1000 samples
            sampleList->reserve(1000);

            for (const auto& module : m_modMap)
            {
                gtUInt32 modSize = module.second.m_size;

                bool isJitModule = ((CpuProfileModule::JAVAMODULE == module.second.getModType())
                    || (CpuProfileModule::MANAGEDPE == module.second.getModType())) ? true : false;

                for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
                {
                    gtUInt32 funcId = module.second.m_moduleId;
                    funcId = (funcId << 16) | fit->second.m_functionId;

                    for (auto aptIt = fit->second.getBeginSample(); aptIt != fit->second.getEndSample(); ++aptIt)
                    {
                        gtUInt64 pid = aptIt->first.m_pid;
                        gtUInt32 threadId = aptIt->first.m_tid;
                        gtVAddr  sampleAddr = aptIt->first.m_addr;
                        gtUInt32 moduleInstanceid = 0ULL;
                        gtVAddr  modLoadAddr = 0ULL;

                        if (!isJitModule)
                        {
                            for (const auto& it : module.second.m_moduleInstanceInfo)
                            {
                                modLoadAddr = std::get<1>(it);
                                gtVAddr modEndAddr = modLoadAddr + modSize;

                                if ((std::get<0>(it) == pid))
                                {
                                    // 1. sampleAddr falls within module size limit.
                                    // 2. module size is 0, pick the first match.
                                    if ((modLoadAddr <= sampleAddr) && ((sampleAddr < modEndAddr) || (0 == modSize)))
                                    {
                                        moduleInstanceid = std::get<2>(it);
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            modLoadAddr = fit->second.getBaseAddr();

                            if (!module.second.m_moduleInstanceInfo.empty())
                            {
                                moduleInstanceid = std::get<2>(module.second.m_moduleInstanceInfo.at(0));
                            }
                        }

                        // sample address offset is w.r.t. module load address
                        gtUInt64 sampleOffset = sampleAddr - modLoadAddr;
                        gtUInt64 processThreadId = pid;
                        processThreadId = (processThreadId << 32) | threadId;

                        for (auto skIt = aptIt->second.getBeginSample(); skIt != aptIt->second.getEndSample(); ++skIt)
                        {
                            gtUInt64 sampleCount = skIt->second;
                            gtUInt64 coreSamplingConfigId = skIt->first.cpu;
                            coreSamplingConfigId = (coreSamplingConfigId << 32) | (skIt->first.event & unusedBitsMask);

                            sampleList->emplace_back(processThreadId, moduleInstanceid, coreSamplingConfigId, funcId, sampleOffset, sampleCount);
                        }
                    }
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_SAMPLE_INFO, (void*)sampleList });
            sampleList = nullptr;
        }
    }

#if ENABLE_OLD_PROFILE_WRITER
    // Write CSS too
    if (!m_cssFileDir.empty())
    {
        for (gtMap<ProcessIdType, ProcessInfo*>::iterator it = m_processInfos.begin(), itEnd = m_processInfos.end(); it != itEnd; ++it)
        {
            ProcessInfo* pProcessInfo = it->second;

            if (nullptr != pProcessInfo && 0U != pProcessInfo->m_callGraph.GetOrder())
            {
                ProcessIdType pid = it->first;

                gtString cssFile(m_cssFileDir.c_str(), static_cast<int>(m_cssFileDir.size()));
                cssFile.append(L'/');
                cssFile.appendUnsignedIntNumber(pid);
                cssFile.append(L".css");

                SimpleProcessWorkingSetQuery workingSetQuery(pProcessInfo->m_workingSet);
                CssWriter cssWriter;
                cssWriter.Open(cssFile.asCharArray());
                cssWriter.Write(pProcessInfo->m_callGraph, workingSetQuery, pid);
            }
        }
    }
#endif

    // Write the callstack info to DB
    if (m_dbWriter)
    {
        for (const auto& procIter : m_processInfos)
        {
            ProcessInfo* pProcessInfo = procIter.second;

            if (nullptr != pProcessInfo && 0U != pProcessInfo->m_callGraph.GetOrder())
            {
                ProcessIdType pid = procIter.first;
                CallGraph& callGraph = pProcessInfo->m_callGraph;
                SimpleProcessWorkingSetQuery workingSet(pProcessInfo->m_workingSet);

                gtUInt32 callStackId = 0;
                CPACallStackFrameInfoList *csFrameInfoList = new (std::nothrow) CPACallStackFrameInfoList;
                CPACallStackLeafInfoList  *csLeafInfoList = new (std::nothrow) CPACallStackLeafInfoList;

                if (nullptr != csFrameInfoList && nullptr != csLeafInfoList)
                {
                    // This is just a safe random space reservation. Check if we can reserve more accurate space.
                    csFrameInfoList->reserve(500);
                    csLeafInfoList->reserve(100);

                    for (auto stackIt = callGraph.GetBeginCallStack(), stackItEnd = callGraph.GetEndCallStack(); stackIt != stackItEnd; ++stackIt)
                    {
                        // **stackIt is a reference in callGraph, hence null check not required.
                        CallStack& callStack = **stackIt;
                        gtUInt16 callSiteDepth = 0;
                        ++callStackId;

                        for (auto siteIt = callStack.begin(), siteItEnd = callStack.end(); siteIt != siteItEnd; ++siteIt)
                        {
                            CallSite& callSite = *siteIt;
                            ++callSiteDepth;
                            gtUInt64 funcId = 0;

                            ExecutableFile* pExe = workingSet.FindModule(callSite.m_traverseAddr);
                            gtUInt64 loadAddr = (nullptr != pExe) ? pExe->GetLoadAddress() : 0ULL;
                            gtUInt64 offset = callSite.m_traverseAddr - loadAddr;

                            if (pExe != nullptr)
                            {
                                gtString moduleName = pExe->GetFilePath();
                                auto modIt = m_modMap.find(moduleName);

                                if (modIt != m_modMap.end())
                                {
                                    funcId = modIt->second.m_moduleId << 16;
                                    auto pFunc = modIt->second.findFunction(callSite.m_traverseAddr);

                                    if (pFunc != nullptr)
                                    {
                                        funcId |= pFunc->m_functionId;
                                    }
                                }
                                else
                                {
                                    // TODO: sometime few modules observed by CSS are not present in module map
                                    // This happens on Windows, check if this happens on Linux or not.
                                }
                            }

                            csFrameInfoList->emplace_back(callStackId, pid, funcId, offset, callSiteDepth);
                        }

                        const EventSampleList& samples = callStack.GetEventSampleList();

                        for (const auto& sampleInfo : samples)
                        {
                            gtUInt64 funcId = 0;

                            ExecutableFile* pExe = workingSet.FindModule(sampleInfo.m_pSite->m_traverseAddr);
                            gtUInt64 loadAddr = (nullptr != pExe) ? pExe->GetLoadAddress() : 0ULL;
                            gtUInt64 offset = sampleInfo.m_pSite->m_traverseAddr - loadAddr;

                            if (pExe != nullptr)
                            {
                                gtString moduleName = pExe->GetFilePath();
                                auto modIt = m_modMap.find(moduleName);

                                if (modIt != m_modMap.end())
                                {
                                    funcId = modIt->second.m_moduleId << 16;
                                    auto pFunc = modIt->second.findFunction(sampleInfo.m_pSite->m_traverseAddr);

                                    if (pFunc != nullptr)
                                    {
                                        funcId |= pFunc->m_functionId;
                                    }
                                }
                            }

                            csLeafInfoList->emplace_back(
                                callStackId, pid, funcId, offset, sampleInfo.m_eventId, sampleInfo.m_count);
                        }
                    }

                    m_dbWriter->Push({ TRANSLATED_DATA_TYPE_CSS_FRAME_INFO, (void*)csFrameInfoList });
                    csFrameInfoList = nullptr;

                    m_dbWriter->Push({ TRANSLATED_DATA_TYPE_CSS_LEAF_INFO, (void*)csLeafInfoList });
                    csLeafInfoList = nullptr;
                }
                else
                {
                    // At least one of the memory allocations failed.
                    delete csFrameInfoList;
                    delete csLeafInfoList;
                    break;
                }
            }
        }

        // Populate Java JIT instance info
        CPAJitInstanceInfoList *jitInstanceInfoList = new (std::nothrow) CPAJitInstanceInfoList;

        if (jitInstanceInfoList != nullptr)
        {
            // <jncIdx, moduleName, instanceId, pid, loadAddr, size>
            gtVector<std::tuple<gtUInt32, gtString, gtUInt32, gtUInt64, gtUInt64, gtUInt64>> jitBlockInfo;

            m_javaModInfo.GetJavaJitBlockInfo(jitBlockInfo);

            jitInstanceInfoList->reserve(jitBlockInfo.size() + inlinedJitInfo.size());

            for (const auto& it : jitBlockInfo)
            {
                gtUInt32 jitId = std::get<0>(it);

                // Compute function id.
                gtUInt64 funcId = 0;
                gtString modName = std::get<1>(it);
                auto modIt = m_modMap.find(modName);

                if (m_modMap.end() != modIt)
                {
                    funcId = modIt->second.m_moduleId;
                }

                funcId = (funcId << 16) | std::get<2>(it);

                gtUInt64 pid = std::get<3>(it);
                gtUInt64 loadAddr = std::get<4>(it);
                gtUInt32 size = static_cast<gtUInt32>(std::get<5>(it));

                jitInstanceInfoList->emplace_back(jitId, funcId, pid, loadAddr, size);
            }

            jitBlockInfo.clear();

            for (const auto& it : inlinedJitInfo)
            {
                gtUInt32 jitId = std::get<0>(it);

                // Compute function id.
                gtUInt64 funcId = std::get<1>(it);
                funcId = (funcId << 16) | std::get<2>(it);

                gtUInt64 pid = std::get<3>(it);
                gtUInt64 loadAddr = std::get<4>(it);
                gtUInt32 size = static_cast<gtUInt32>(std::get<5>(it));

                jitInstanceInfoList->emplace_back(jitId, funcId, pid, loadAddr, size);
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_JITINSTANCE_INFO, (void*)jitInstanceInfoList });
            jitInstanceInfoList = nullptr;
            inlinedJitInfo.clear();
        }

        // Populate Java Jnc info.
        CPAJitCodeBlobInfoList *jitCodeBlobInfoList = new CPAJitCodeBlobInfoList;

        if (jitCodeBlobInfoList != nullptr)
        {
            // <jncIdx, srcFilePath, jncFilePath>
            gtVector<std::tuple<gtUInt32, gtString, gtString>> jncInfoList;

            m_javaModInfo.GetJavaJncInfo(jncInfoList);

            for (const auto& it : jncInfoList)
            {
                gtUInt32 jitId = std::get<0>(it);
                gtString srcFilePath = std::get<1>(it);
                gtString jncFilePath = std::get<2>(it);

                jitCodeBlobInfoList->emplace_back(jitId, srcFilePath, jncFilePath);
            }

            jncInfoList.clear();
            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_JITCODEBLOB_INFO, (void*)jitCodeBlobInfoList });
            jitCodeBlobInfoList = nullptr;
        }

        // Finish writing profile data
        m_dbWriter->Push({ TRANSLATED_DATA_TYPE_UNKNOWN_INFO, nullptr });
    }

    gettimeofday(&css_timerStop, nullptr);

    if (m_pLogFile)
    {
        struct timeval diff;
        timersub(&ebp_timerStop, &ebp_timerStart, &diff);
        fprintf(m_pLogFile, "Write EBP time             : %lu sec, %lu usec\n", diff.tv_sec, diff.tv_usec);
        timersub(&css_timerStop, &css_timerStart, &diff);
        fprintf(m_pLogFile, "Write CSS time             : %lu sec, %lu usec\n", diff.tv_sec, diff.tv_usec);
    }

    return bRet ? 0 : 1;
}


HRESULT CaPerfTranslator::_setupReader(const std::string& perfDataPath)
{
    if (m_pPerfDataRdr)
    {
        delete m_pPerfDataRdr;
    }

    // First, try using PerfDataReader
    m_pPerfDataRdr = new PerfDataReader();

    if (!m_pPerfDataRdr)
    {
        return E_OUTOFMEMORY;
    }

    if (S_OK == m_pPerfDataRdr->init(perfDataPath))
    {
        return S_OK;
    }

    delete m_pPerfDataRdr;

    // Retry using CaPerfDataReader
    m_pPerfDataRdr = new CaPerfDataReader();

    if (!m_pPerfDataRdr)
    {
        return E_OUTOFMEMORY;
    }

    if (S_OK == m_pPerfDataRdr->init(perfDataPath))
    {
        return S_OK;
    }

    delete m_pPerfDataRdr;
    m_pPerfDataRdr = nullptr;

    return E_FAIL;
}


HRESULT CaPerfTranslator::setupLogFile(const std::string& logFile)
{
    if (m_pLogFile)
    {
        fclose(m_pLogFile);
        m_pLogFile = nullptr;
    }

    m_pLogFile = fopen(logFile.c_str(), "w");

    if (!m_pLogFile)
    {
        return E_FAIL;
    }

    fprintf(m_pLogFile, "====================\n");
    fprintf(m_pLogFile, "CaPerfTranslator Log\n");
    fprintf(m_pLogFile, "====================\n");
    return S_OK;
}


HRESULT CaPerfTranslator::setupCalogCssFile(const std::string& file)
{
#ifdef HAVE_CALOG

    if (m_pCalogCss)
    {
        calog_close(&m_pCalogCss);
        m_pCalogCss = nullptr;
    }

    m_pCalogCss = calog_init(file.c_str(), CALOG_CACSS);

    if (!m_pCalogCss)
    {
        return E_FAIL;
    }

#else
    (void)(file); // unused
#endif

    return S_OK;
}


int CaPerfTranslator::setupCssFile(const std::string& file)
{
    m_cssFileDir = std::wstring(file.begin(), file.end());

    return S_OK;
}


// Note [Suravee]:
// This can only be used with module name since PERF doesn't
// always return fullpath as process name in COMM
HRESULT CaPerfTranslator::_getModuleBitness(const std::string& modName, bool* pIs32Bit)
{
    int fd;
    int elfClass;
    Elf* e = nullptr;
    HRESULT ret = E_FAIL;

    if (nullptr == pIs32Bit)
    {
        return E_INVALIDARG;
    }

    if (elf_version(EV_CURRENT) != EV_NONE && 0 <= (fd = open(modName.c_str(), O_RDONLY, 0)))
    {
        if ((e = elf_begin(fd, ELF_C_READ, nullptr)) != nullptr)
        {
            elfClass = gelf_getclass(e);
            *pIs32Bit = (ELFCLASS32 == elfClass);
            ret = S_OK;
            elf_end(e);
        }

        close(fd);
    }

    if (ret != S_OK)
    {
        // Note [Suravee]:
        // This is normally when :
        // - The modName is not a fullpath.
        // - The modName has (deleted) appended.
        // - [kernel.kallsyms]

        // Use system bitness as default
        struct utsname buf;

        if (0 != uname(&buf))
        {
            return E_FAIL;
        }

        // Compare
        if (strncmp("x86_64", buf.machine, 6) != 0)
        {
            *pIs32Bit = true;
        }
        else
        {
            *pIs32Bit = false;
        }

        ret = S_OK;
    }

    return ret;
}


HRESULT CaPerfTranslator::_getElfFileType(const std::string& modName, gtUInt32* pElfType)
{
    HRESULT ret = E_FAIL;

    if (nullptr == pElfType)
    {
        return E_INVALIDARG;
    }

    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        return E_FAIL;
    }

    int fd;

    if (0 > (fd = open(modName.c_str(), O_RDONLY, 0)))
    {
        // Note: This is often for [kernel.kallsyms] and deleted stuff.
#if 0
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to open file : %hs", modName.c_str());
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"      : bfd_openr error : %hs", msg.c_str());
#endif
        return E_FAIL;
    }

    Elf* e = elf_begin(fd, ELF_C_READ, nullptr);

    if (e != nullptr)
    {

        int elfClass = gelf_getclass(e);
        bool bIs32Bit = (ELFCLASS32 == elfClass);

        GElf_Ehdr elfHdr;

        if (nullptr != gelf_getehdr(e, &elfHdr))
        {
            if (bIs32Bit)
            {
                Elf32_Ehdr* p32 = (Elf32_Ehdr*) &elfHdr;
                *pElfType = p32->e_type;
            }
            else
            {
                Elf64_Ehdr* p64 = (Elf64_Ehdr*) &elfHdr;
                *pElfType = p64->e_type;
            }

            ret = S_OK;
        }

        elf_end(e);
    }

    close(fd);

    return ret;
}


EventMaskType CaPerfTranslator::_getEvmask(
    gtUInt32 event,
    gtUInt32 umask,
    gtUInt32 kernel,
    gtUInt32 user)
{
    EventMaskTypeEnc evMaskEnc;
    evMaskEnc.encodedEvent = 0;

    evMaskEnc.ucEventSelect = event;

    // No unit mask, os and user bit for IBS and Timer.
    if (event != IBS_FETCH_BASE && event != IBS_OP_BASE && event != TIMER_PERF)
    {
        evMaskEnc.ucUnitMask = umask;
        evMaskEnc.bitOsEvents = kernel;
        evMaskEnc.bitUsrEvents = user;
    }

    EventMaskType evMask = 0;
    memcpy(&evMask, &evMaskEnc, sizeof(EventMaskTypeEnc));

#if 0
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"_getEvmask: event:0x%x, umask:0x%x, kernel:%x, user:%x", event, umask, kernel, user);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"_getEvmask: evMask = 0x%lx", evMask);
#endif
    return evMask;
}


#ifdef ENABLE_FAKETIMER
HRESULT CaPerfTranslator::_getFakeTimerInfo()
{
    if (!m_pPerfDataRdr)
    {
        return E_FAIL;
    }

    HRESULT ret = static_cast<CaPerfDataReader*>(m_pPerfDataRdr)->getFakeTimerInfo(&m_fakeInfo);

    if (S_OK == ret)
    {
        if (m_bVerb)
        {
            OS_OUTPUT_DEBUG_LOG(L"============================", OS_DEBUG_LOG_INFO);
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Num Fake Info : %u", m_fakeInfo.numCpu);
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Timer (nsec)  : %u", m_fakeInfo.timerNanosec);

            for (size_t i = 0 ; i < m_fakeInfo.numCpu; i++)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"%2u: timerFd = %4u, fakeTimerFd = %4u",
                                           i, m_fakeInfo.timerFds[i], m_fakeInfo.fakeTimerFds[i]);
            }
        }

        m_aFakeFlags = new bool[m_fakeInfo.numCpu];

        if (nullptr == m_aFakeFlags)
        {
            return E_OUTOFMEMORY;
        }

        memset(m_aFakeFlags, 0, (sizeof(bool) * m_fakeInfo.numCpu));
    }

    return ret;
}
#endif


HRESULT CaPerfTranslator::_getTargetPids()
{
    if (!m_pPerfDataRdr)
    {
        return E_FAIL;
    }

    size_t numTargetPids = static_cast<CaPerfDataReader*>(m_pPerfDataRdr)->getNumTargetPids();

    if (numTargetPids == 0)
    {
        return S_OK;
    }

    pid_t* pTargetPids = new pid_t[numTargetPids];

    if (!pTargetPids)
    {
        return E_FAIL;
    }

    static_cast<CaPerfDataReader*>(m_pPerfDataRdr)->getTargetPids(numTargetPids * sizeof(pid_t), pTargetPids);

    if (m_bVerb)
    {
        OS_OUTPUT_DEBUG_LOG(L"============================", OS_DEBUG_LOG_INFO);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Num Target Pids = %d", numTargetPids);
    }

    for (size_t i = 0 ; i < numTargetPids; i++)
    {
        // Making sure that target pids are not duplicated
        if (m_pidProcInfoMap.end() != m_pidProcInfoMap.find(pTargetPids[i]))
        {
            if (m_bVerb)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Duplicated target pid %d", pTargetPids[i]);
            }

            continue;
        }

        if (m_bVerb)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"TargetPids [%d] = %d", i, pTargetPids[i]);
        }

        // fprintf(stderr, "TargetPids [%d] = %d", (int)i, (int)pTargetPids[i]);
        m_pidProcInfoMap.insert(PidProcInfoMap::value_type(pTargetPids[i], ProcInfo()));
        m_pidsFilter.push_back(pTargetPids[i]);
    }

    delete [] pTargetPids;

    return S_OK;
}

bool CaPerfTranslator::_isTargetPid(gtUInt32 pid)
{
    bool ret = true;

    if (m_isPerProcess)
    {
        ret = false;

        if (std::find(m_pidsFilter.begin(), m_pidsFilter.end(), pid) != m_pidsFilter.end())
        {
            ret = true;
        }
    }

    return ret;
}

bool CaPerfTranslator::_useModuleCaching(
    size_t pid,
    gtUInt64 ip,
    gtUInt64 time,
    bool bIsUser)
{
    if (m_cachedMod == m_modLoadInfoMap.rend())
    {
#ifdef _DEBUG_MOD_CACHE_
        OS_OUTPUT_DEBUG_LOG(L"Cache not init", OS_DEBUG_LOG_DEBUG);
#endif
        return false;
    }

    if (!bIsUser)
    {
        pid = -1;
    }

#ifdef _DEBUG_MOD_CACHE_
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CACHED pid        =   %016d, REC pid  = %d", m_cachedPid, pid);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CACHED time       = 0x%016llx, REC time = 0x%016llx", m_cachedMod->first.time, time);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CACHED ip start   = 0x%016llx, REC  ip  = 0x%016llx", m_cachedMod->first.addr, ip);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CACHED ip end     = 0x%016llx, REC  ip  = 0x%016llx",
                               m_cachedMod->first.addr + m_cachedMod->second.len, ip);
#endif

    // Match PID
    if (pid  != (size_t) m_cachedPid)
    {
#ifdef _DEBUG_MOD_CACHE_
        OS_OUTPUT_DEBUG_LOG(L"Bad pid", OS_DEBUG_LOG_DEBUG);
#endif
        return false;
    }

    // SAMP time must be later than MMAP time
    if (time < m_cachedMod->first.time)
    {
#ifdef _DEBUG_MOD_CACHE_
        OS_OUTPUT_DEBUG_LOG(L"Bad time", OS_DEBUG_LOG_DEBUG);
#endif
        return false;
    }

    if (ip < m_cachedMod->first.addr)
    {
#ifdef _DEBUG_MOD_CACHE_
        OS_OUTPUT_DEBUG_LOG(L"Bad start addr", OS_DEBUG_LOG_DEBUG);
#endif
        return false;
    }

    // IP must be within the module MMAP range.
    if (ip >= m_cachedMod->first.addr + m_cachedMod->second.len)
    {
#ifdef _DEBUG_MOD_CACHE_
        OS_OUTPUT_DEBUG_LOG(L"Bad range1", OS_DEBUG_LOG_DEBUG);
#endif
        return false;
    }

#ifdef _DEBUG_MOD_CACHE_
    OS_OUTPUT_DEBUG_LOG(L"Cache OK", OS_DEBUG_LOG_DEBUG);
#endif

    return true;
}

int CaPerfTranslator::process_PERF_RECORD_READ(
    struct perf_event_header* pHdr,
    void* ptr,
    gtUInt32 offset,
    gtUInt32 index)
{
    (void)(offset); // unused
    (void)(index); // unused

    if (nullptr == ptr)
    {
        return E_INVALIDARG;
    }

    struct CA_PERF_RECORD_READ rec;

    //char * pCur = (char* )ptr;

    memset(&rec, 0, sizeof(struct CA_PERF_RECORD_READ));

    if (m_bVerb)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"READ: hdr.misc:%d, pid:%d, tid:%d", pHdr->misc, rec.pid, rec.tid);
    }

    // TODO: [Suravee]: Need to process this once we know what this do.

    return S_OK;
}

HRESULT CaPerfTranslator::_processCSS(struct perf_event_header* pHdr,
                                      struct CA_PERF_RECORD_SAMPLE& rec,
                                      EventMaskType evMask,
                                      float weight,
                                      ModLoadInfoMap::reverse_iterator modRit)
{
    HRESULT ret = S_OK;

    bool bIsCssOk = (rec.callchain != nullptr);

    // NOTE [Suravee]:
    // We DO NOT handle the call stack for kernel samples.
    // Current logic assumes that the instruction right before the
    // return address on the call stack is the "call" instruction.
    // However, this is not the case for kernel samples when handling
    // interrupt.
    //
    // To properly handle this, the CSS translator need to track the context
    // in the callchain and handle the case when switching from user-context
    // to kernel context.
    if (pHdr->misc & PERF_RECORD_MISC_KERNEL)
    {
        bIsCssOk = false;
    }

    if (!bIsCssOk)
    {
        return ret;
    }

    if (m_pCalogCss)
    {
#ifdef HAVE_CALOG
        //------------------------------------------
        // Write write out CSS sample records
        calog_data entry;
        cacss_data data;

        /* Setup entry */
        entry.app_cookie = 0; // Not used
        entry.cpu = cpu;
        entry.tgid = rec.pid;
        entry.tid = rec.tid;
        entry.cnt = weight;

        bool bIsUser = true;

        for (int i = 0; i < rec.callchain->nr; i++)
        {
            // We need to setup cookie and offset
            // accordingly
            ModLoadInfoMap::reverse_iterator rit;
            entry.cookie = 0;
            entry.offset = 0;

            gtUInt64 tmpIp = rec.callchain->ips[i];

            if (tmpIp >= PERF_CONTEXT_MAX)
            {
                switch (tmpIp)
                {
                    case PERF_CONTEXT_KERNEL:
                        bIsUser = false;
                        break;

                    case PERF_CONTEXT_USER:
                        bIsUser = true;
                        break;

                    default:
                        break;
                }

                //Context switch, not actual callstack info
                continue;
            }

            if (_useModuleCaching(rec.pid, tmpIp, rec.time, bIsUser))
            {
                rit = modRit;
                // Use MMAP time as app cookie
                entry.cookie = rit->first.time;
                entry.offset = tmpIp - rit->first.addr;
            }
            else
            {
                // NOTE [Suravee]:
                // If we could cache this, it would be much faster.
                rit = getModuleForSample(
                          pHdr, rec.pid, rec.time, tmpIp, false, bIsUser);

                if (rit != m_modLoadInfoMap.rend())
                {
                    // Use MMAP time as app cookie
                    entry.cookie = rit->first.time;
                    entry.offset = tmpIp - rit->first.addr;
                }
                else
                {
                    continue;
                }
            }

            /* Setup data */
            if (i == 1)
            {
                data.cg_type = 1;
            }
            else
            {
                data.cg_type = 0;
            }

            if (0 != calog_add_data(m_pCalogCss,
                                    &entry,
                                    &data,
                                    sizeof(cacss_data),
                                    rit->second.name.c_str(),
                                    nullptr))
            {
                break;
            }
        }

#endif
    }
    else if (!m_cssFileDir.empty())
    {
#if HAS_LIBCSS
        // NOTE [Suravee]: Only generate CSS for specified target pids
        PidProcInfoMap::iterator pit = m_pidProcInfoMap.find(rec.pid);

        if (pit != m_pidProcInfoMap.end())
        {
            // Use libCSS to generate new CSS file.

            struct timeval css_timerStart;
            struct timeval css_timerStop;
            gettimeofday(&css_timerStart, nullptr);

            // Also specify that this process has CSS information
            if (modRit->second.pProc)
            {
                modRit->second.pProc->m_hasCss = true;
            }

            ProcessInfo& processInfo = AcquireProcessInfo(rec.pid);

            const size_t stkSize = rec.callchain->nr;
            // Multiplied by (MAX_INLINED_FUNCS+1) to include inline functions.
            // Later introduce handleInline flag and check it.
            // If true, multiply by (MAX_INLINED_FUNCS+1), else multiply by 1.
            m_cssBuffer.reserve(stkSize * (MAX_INLINED_FUNCS + 1) + 1);
            CallStackBuilder callStackBuilder(processInfo.m_callGraph,
                                              reinterpret_cast<gtUByte*>(m_cssBuffer.data()),
                                              static_cast<unsigned>(m_cssBuffer.capacity() * sizeof(gtUInt64)));

            ModLoadInfoMap::reverse_iterator rit;

            bool bIsUser = true;
            gtUInt64 sampleAddr = 0ULL;
            bool isCSBInitialized = false;

            for (size_t i = 0; i < stkSize; i++)
            {
                // Get current return address
                gtUInt64 tmpIp = rec.callchain->ips[i];

                // Sanity check for valid call stack entries
                if (tmpIp == 0)
                {
                    continue;
                }

                if (tmpIp >= PERF_CONTEXT_MAX)
                {
                    switch (tmpIp)
                    {
                        case PERF_CONTEXT_KERNEL:
                            bIsUser = false;
                            break;

                        case PERF_CONTEXT_USER:
                            bIsUser = true;
                            break;

                        default:
                            break;
                    }

                    //Context switch, not actual callstack info
                    continue;
                }

                // Resolve the name of module of each stack entry.
                // TODO: [Suravee] : If we could cache this, it would be much faster.
                // NOTE [Suravee]
                // If we cannot locate the module, we will just throw
                // the stack record away at this point.
                rit = getModuleForSample(pHdr,
                                         rec.pid,
                                         rec.time,
                                         tmpIp,
                                         false,
                                         bIsUser);

                // Sanity check for valid call stack entries
                if (rit == m_modLoadInfoMap.rend() || 0 == rit->first.addr)
                {
                    continue;
                }

                gtVector<gtVAddr> funcList;
                _getInlinedFuncInfoListByVa(rec.pid, tmpIp, funcList);

                if (funcList.size() > 1)
                {
                    auto it = funcList.rbegin();

                    if (0ULL == sampleAddr)
                    {
                        sampleAddr = *it;
                        it++;
                    }

                    int count = 1;

                    while (count < MAX_INLINED_FUNCS && funcList.rend() != it)
                    {
                        gtVAddr callerVa = *it;

                        if (!isCSBInitialized)
                        {
                            callStackBuilder.Initialize(callerVa, 0ULL, 0ULL);
                            isCSBInitialized = true;
                        }
                        else
                        {
                            callStackBuilder.Push(callerVa);
                        }

                        it++;
                        count++;
                    }

                    if (funcList.rend() != it)
                    {
                        gtVAddr callerVa = funcList.front();
                        callStackBuilder.Push(callerVa);
                    }
                }
                else
                {
                    if (0ULL == sampleAddr)
                    {
                        sampleAddr = tmpIp;
                    }
                    else
                    {
                        if (!isCSBInitialized)
                        {
                            callStackBuilder.Initialize(tmpIp, 0ULL, 0ULL);
                            isCSBInitialized = true;
                        }
                        else
                        {
                            callStackBuilder.Push(tmpIp);
                        }
                    }
                }
            }

            // Store callpath info
            if (0U != callStackBuilder.GetDepth())
            {
                EventSampleInfo eventSample;
                eventSample.m_pSite = processInfo.m_callGraph.AcquireCallSite(sampleAddr);
                eventSample.m_eventId = evMask;
                eventSample.m_threadId = rec.tid;
                eventSample.m_count = static_cast<gtUInt64>(weight);

                callStackBuilder.Finalize(eventSample);
            }

            gettimeofday(&css_timerStop, nullptr);

            struct timeval diff;
            timersub(&css_timerStop, &css_timerStart, &diff);
            timeradd(&m_pass2Css, &diff, &m_pass2Css);
        }

#endif
    }

    return ret;
}

// Function to get the Java module details if the IP is in
// Java module
CpuProfileModule* CaPerfTranslator::getJavaModuleforSample(TiModuleInfo* pModInfo, gtUInt32 pid, gtUInt64 time, gtUInt64 ip)
{
    HRESULT hr = _getJavaModuleforSample(pModInfo, pid, time, ip);

    if (S_OK != hr)
    {
        // fprintf(stderr, "getJavaModuleforSample failed \n");
        return nullptr;
    }

    if (m_pLogFile)
    {
        fwprintf(m_pLogFile, L"java mod name : %S\n", pModInfo->pModulename);
        fwprintf(m_pLogFile, L"java Jnc : %S\n", pModInfo->pJncName);
        fwprintf(m_pLogFile, L"java src file : %S\n", pModInfo->pJavaSrcFileName);
        fwprintf(m_pLogFile, L"java function  : %S\n", pModInfo->pFunctionName);
    }

    // Now, we have a Java Module for the given ip;
    //  - create CpuProfileModule object
    gtString javaModName(pModInfo->pModulename);
    CpuProfileModule javaMod;
    CpuProfileModule* pJavaMod = nullptr;

    // Check whether this java module is available already
    NameModuleMap::iterator it = m_modMap.find(javaModName);

    if (it == m_modMap.end())
    {
        javaMod.setPath(javaModName);

        // TODO: Find the bitness of the module
        javaMod.m_is32Bit = false;
        javaMod.m_base = pModInfo->ModuleStartAddr;
        javaMod.m_size = 0;
        javaMod.m_moduleId = AtomicAdd(m_nextModuleId, 1);

        m_modMap.insert(NameModuleMap::value_type(javaModName, javaMod));

        // Re-find
        it = m_modMap.find(javaModName);

        if (it == m_modMap.end())
        {
            // wprintf(L"Error while adding java module %S\n",
            //      javaModName.c_str());
            return nullptr;
        }

        pJavaMod = &(it->second);

        CpuProfileProcess* pProc = getProcess(pid);

        if (!pProc)
        {
            // wprintf(L"failed to get CA_process for Pid %d\n", pid);
            return nullptr;
        }

        ModKey key(0, pModInfo->ModuleStartAddr, pid);

        // Baskar: moduleSize is used in place of len
        char modName[260] = { '\0' };
        wcstombs(modName, pModInfo->pModulename, 260);
        ModInfo info(pModInfo->Modulesize,
                     pModInfo->ModuleStartAddr,
                     modName);

        info.pMod = pJavaMod;

        if (pJavaMod && pProc)
        {
            info.pProc = pProc;
        }

        // set the module type
        pJavaMod->m_modType = CpuProfileModule::JAVAMODULE;
        info.instanceId = AtomicAdd(m_nextModInstanceId, 1);

        m_modLoadInfoMap.insert(ModLoadInfoMap::value_type(key, info));
    }
    else
    {
        pJavaMod = &(it->second);
    }

    if (nullptr == pJavaMod)
    {
        // wprintf(L"failed to get Java Module for %S\n", javaModName.c_str());
        return nullptr;
    }

    return pJavaMod;
}


// This is helper function to get initialize module info structure
// and get module information via task info interface
HRESULT CaPerfTranslator::_getJavaModuleforSample(TiModuleInfo* pModInfo, gtUInt32 pid, gtUInt64 time, gtUInt64 ip)
{
    if (nullptr == pModInfo)
    {
        return E_FAIL;
    }

    pModInfo->processID = pid;
    pModInfo->sampleAddr = ip;
    pModInfo->cpuIndex = 0;
    pModInfo->deltaTick = time;

    memset(smoduleName, 0, OS_MAX_PATH);
    memset(sfunctionName, 0, OS_MAX_PATH);
    memset(sjncName, 0, OS_MAX_PATH);
    memset(sjavaSrcFileName, 0, OS_MAX_PATH);
    memset(sSessionDir, 0, OS_MAX_PATH);

    pModInfo->namesize = OS_MAX_PATH;
    pModInfo->funNameSize =  OS_MAX_PATH;
    pModInfo->jncNameSize = OS_MAX_PATH;
    pModInfo->srcfilesize = OS_MAX_PATH;
    pModInfo->sesdirsize = OS_MAX_PATH;

    pModInfo->pModulename = smoduleName;
    pModInfo->pFunctionName = sfunctionName;
    pModInfo->pJncName = sjncName;
    pModInfo->pJavaSrcFileName = sjavaSrcFileName;
    pModInfo->pSessionDir = sSessionDir;

    pModInfo->moduleType = evJavaModule;
    pModInfo->ModuleStartAddr = 0;

    // Replace this with CTaskInfo::GetModuleInfo
    int ret = m_javaModInfo.GetModuleInfo(pModInfo);

    return ret;
}

void CaPerfTranslator::addJavaInlinedMethods(CpuProfileModule&  mod,
         gtVector<std::tuple<gtUInt32, gtUInt32, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& inlinedJitInfo)
{
    AddrFunctionMultMap inlinedFuncMap;
    gtUInt32 nextFuncId = 0;

    for (auto funcIt = mod.getBeginFunction(), endIt = mod.getEndFunction(); funcIt != endIt; ++funcIt)
    {
        nextFuncId = std::max(nextFuncId, funcIt->second.m_functionId);
    }

    // Increment it to generate next unused Id.
    ++nextFuncId;

    // Iterate over the AddrFunctionMultMap entries
    // Open JNC file
    // Get the JavaInlineMap
    // if the inline entries are available
    //  Iterate over AptAggregatedSampleMap
    //  if the ip falls in Inlined method
    //  create a new CA_Function for this IP

    if (CpuProfileModule::JAVAMODULE != mod.getModType())
    {
        return;
    }

    AddrFunctionMultMap::iterator it = mod.getBeginFunction();
    AddrFunctionMultMap::iterator itEnd = mod.getEndFunction();

    for (; it != itEnd; ++it)
    {
        // Construct the JNC file path and Open the JNCfile
        wchar_t jncName[OS_MAX_PATH] = { L'\0' };
        osFilePath tmpPath((std::wstring(m_inputFile.begin(),
                                         m_inputFile.end())).c_str());

        swprintf(jncName, OS_MAX_PATH, L"%S/%S",
                 tmpPath.fileDirectoryAsString().asCharArray(),
                 (*it).second.getJncFileName().asCharArray());

        JavaJncReader javaJncReader;

        // Open the JNC file
        if (! javaJncReader.Open(jncName))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"javaJncReader Open failed : %S", jncName);
            return;
        }

        JavaInlineMap* jilMap = javaJncReader.GetInlineMap();

        if (jilMap->begin() == jilMap->end())
        {
            javaJncReader.Close();
            continue;
        }

        // For each sample in CpuProfileFunction
        AptAggregatedSampleMap::iterator ait = it->second.getBeginSample();
        AptAggregatedSampleMap::iterator aend = it->second.getEndSample();

        while (ait != aend)
        {
            AggregatedSample aSample = ait->second;
            AptKey aAptKey           = ait->first;
            uint64_t ip = ait->first.m_addr;
            bool foundInline = false;

            // fwprintf(stderr, L"      Vaddr IP = 0x%lx\n", ip);

            JavaInlineMap::iterator mit = jilMap->begin();
            JavaInlineMap::iterator mitEnd = jilMap->end();

            for (; mit != mitEnd && (false == foundInline); mit++)
            {
                JNCInlineMap::reverse_iterator ilmit = mit->second.rbegin();
                JNCInlineMap::reverse_iterator ilmitEnd = mit->second.rend();

                for (; ilmit != ilmitEnd && (false == foundInline); ++ilmit)
                {
                    // fwprintf(stderr, L"     methodId = 0x%lx (key=0x%lx)\n",
                    //         ilmit->second.methodId, ilmit->first);
                    // fwprintf(stderr, L"      lineNum = %d\n", ilmit->second.lineNum);
                    // fwprintf(stderr, L"   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
                    // fwprintf(stderr, L"      symName = %s\n", ilmit->second.symName.c_str());

                    AddressRangeList::iterator listit = ilmit->second.addrs.begin();
                    AddressRangeList::iterator listitEnd = ilmit->second.addrs.end();

                    uint64_t funcAddr = 0;

                    for (; listit != listitEnd; ++listit)
                    {
                        if (! funcAddr)
                        {
                            funcAddr = listit->startAddr;
                        }

                        if ((ip >= listit->startAddr) && (ip <= listit->stopAddr))
                        {
                            foundInline = true;

                            // fwprintf(stderr, L" Found Inlined Method for IP(0x%lx) - range 0x%lx - 0x%lx\n",
                            //    ip, listit->startAddr, listit->stopAddr);
                            // fwprintf(stderr, L"   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
                            // fwprintf(stderr, L"      symName = %s\n", ilmit->second.symName.c_str());

                            gtString javaFuncName(L"[inlined] ");
                            javaFuncName.append(it->second.getFuncName());
                            javaFuncName.append(L"::");
                            gtString inlineName;
                            inlineName.fromUtf8String(ilmit->second.symName);
                            javaFuncName.append(inlineName);

                            gtString javaSrcFileName;
                            javaSrcFileName.fromUtf8String(ilmit->second.sourceFile);

                            AddrFunctionMultMap::iterator fit = inlinedFuncMap.find(funcAddr);

                            if (fit == inlinedFuncMap.end())
                            {
                                CpuProfileFunction func(javaFuncName,
                                                        funcAddr,
                                                        (*listit).stopAddr - funcAddr,
                                                        it->second.getJncFileName(),
                                                        javaSrcFileName);
                                func.m_functionId = ++nextFuncId;
                                fit = inlinedFuncMap.insert(AddrFunctionMultMap::value_type(funcAddr, func));
                                fit->second.insertSample(aAptKey, aSample);

                                // Extract the jnc index
                                gtUInt32 jncIndex = 0;
                                const wchar_t* jncFileName = func.getJncFileName().asCharArray();

                                while (*jncFileName != L'_' && *jncFileName != L'\0')
                                {
                                    ++jncFileName;
                                }

                                if (*jncFileName == L'_')
                                {
                                    ++jncFileName;
                                    std::wstring str(jncFileName);
                                    jncIndex = std::stoi(str);
                                }

                                inlinedJitInfo.emplace_back(
                                        jncIndex, mod.m_moduleId, func.m_functionId, aAptKey.m_pid, funcAddr, func.getSize());
                            }
                            else
                            {
                                fit->second.addSample(aAptKey, aSample);
                            }
                        }

                        // else {
                        //      fwprintf(stderr, L" No Inlined Method for IP(0x%lx) - range 0x%lx - 0x%lx\n",
                        //          ip, listit->startAddr, listit->stopAddr);
                        // }

                        if (foundInline)
                        {
                            break;
                        }
                    } // address range in a inlined methods
                } // JNCInlineMap entries
            } // java inline map entries in a CpuProfileFunction

            AptAggregatedSampleMap::iterator tmpait = ait;
            ++tmpait;

            if (foundInline)
            {
                it->second.removeSample(ait);
            }

            ait = tmpait;
        } // AptAggregatedSampleMap entries

        javaJncReader.Close();
    } // AddrFunctionMultMap entries

    // Now add the inlined functions to CpuProfileModule
    AddrFunctionMultMap::const_iterator inlineIt = inlinedFuncMap.begin();
    AddrFunctionMultMap::const_iterator inlineItEnd = inlinedFuncMap.end();

    for (; inlineIt != inlineItEnd; ++inlineIt)
    {
        mod.recordFunction(inlineIt->first, &(inlineIt->second));
    }

    return;
} // addJavaInlinedMethods
