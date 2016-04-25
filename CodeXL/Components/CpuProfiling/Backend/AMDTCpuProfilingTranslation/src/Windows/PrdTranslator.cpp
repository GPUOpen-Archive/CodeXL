//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdTranslator.cpp
/// \brief This is the interface for the PRD file translation.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/Windows/PrdTranslator.cpp#85 $
// Last checkin:   $DateTime: 2016/04/18 05:31:05 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569611 $
//=====================================================================

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>

#include "ThreadPool.h"
#include "PrdTranslator.h"

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTExecutableFormat/inc/PeFile.h>
#include <AMDTCpuCallstackSampling/inc/CallStackBuilder.h>
#include <AMDTCpuCallstackSampling/inc/CssWriter.h>
#include <AMDTCpuProfilingRawData/inc/ProfilerDataDBWriter.h>

#include <psapi.h>
#include <sstream>

#define ENABLE_PRD_DEBUG_OUTPUT 0

// canonical linear address
#define ERBT_713_NON_CANONICAL_MASK 0x0000FFFFFFFFFFFF

#if SUPPORT_CLU
    #include "CluWriter.h"
#endif

#include "../ExecutableAnalyzer.h"
#include "PrdUserCss.h"

bool  gInlineMode = true;
bool  gNestedJavaInline = true;
FILE*  pPRDDebugFP = NULL;

static void PrintMemoryUsage(const wchar_t* header);


enum HardcodedEventType
{
    NON_LOCAL_EVENT = 0xe9
};


#define KERNEL64_SPACE_START    0xfffff80000000000
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #define KERNEL_SPACE_START  KERNEL64_SPACE_START
#else
    #define KERNEL_SPACE_START  0x80000000
#endif

const QString UNKNOWN_MODULE_STR = "Unknown Module";
#define UNKNOWN_KERNEL_SAMPLES  L"Unknown Kernel Samples"
#define UNKNOWN_MODULE_PID_BUF_LEN 30

const QString TI_FILE_EXT = ".ti";
const QString PRD_FILE_EXT = ".prd";
const QString TBS_FILE_EXT = ".tbp";
const QString EBS_FILE_EXT = ".ebp";
const QString RI_FILE_EXT = ".ri";

#define MAX_CPUS            256

//
//  Macros
//

// CXL_PRD_MIN_SIZE_FOR_THREADPOOL
//
// Macro to define the minimum file size required to create the thread pool to process the PRD file
//

// #define CXL_PRD_MIN_SIZE_FOR_THREADPOOL       1048576     // 1 MB
#define CXL_PRD_MIN_SIZE_FOR_THREADPOOL      20971520        // 20 MB

// CXL_PRD_MAPVIEW_GRANULARITY
//
//      If the file size is too huge, we cannot map the entire file into in a single file
// mapview. Hence we need to create multiple file map views and process them sequential.
// This macro CXL_PRD_MAPVIEW_GRANULARITY defines the minimum granularity.
//      While creating a mapview of the file, the offset should be multiples of File
// Allocation Granularuty. Windows API GetSystemInfo() returns the file allocation
// granularity in SYSTEM_INFO::dwAllocationGranularity field. Which is normally 64K.
//
// CodeAnalyst's PRD record size 40 bytes. Ensure that the size of the file map view is
// multiples of 64K and 40 bytes.
//

#define CXL_PRD_MAPVIEW_GRANULARITY      131072000       // 125 MB
// #define CXL_PRD_MAPVIEW_GRANULARITY       983040          // 960 K

// There is also a possibility the final buffer in the file-map-view can exceed the mapview size.
// Hence, to complete the processing of final buffer in a map-view, map another 4096 bytes.
// (we can have maximum of 4040bytes in a PRD buffer).
//
//
//  Lets take an example..
//      mapview granularity = 960K
//      start of mapview-1  = 0
//      end of mapview-1 = 960K + 4K = 987136
//
//      (start of mapview-1) offset 0,  |--------------------|
//                                      |                    |
//                                      |                    |
//                                      |                    |
//                                      |++ (WEIGHT record)  |
//                          960 k       |--------------------|
//                  960 K + 1200 bytes  |+++ (PRD records extends upto this point for the buffer)
//      (end of mapview-1) offset(964k) |--------------------|
//                                      |                    |
//
// the second file mapview will be from 960K to 1920K.
// As the first 1200 bytes are already processed, the processing of PRD records for the second
// mapview will start after that.
//

#define CXL_PRD_MAPVIEW_SIZE             CXL_PRD_MAPVIEW_GRANULARITY + 4096

#define CXL_3GB_VM  3221225472

using namespace std;

__declspec(thread) static wchar_t smoduleName[OS_MAX_PATH + 1];
__declspec(thread) static wchar_t sfunctionName [OS_MAX_PATH + 1];
__declspec(thread) static wchar_t sjncName [OS_MAX_PATH + 1];
__declspec(thread) static wchar_t sjavaSrcFileName [OS_MAX_PATH + 1];
__declspec(thread) static gtUInt64 threadBytesReadSoFar = 0;

__declspec(thread) static wchar_t system32Dir[OS_MAX_PATH + 1];
__declspec(thread) static bool bGetSystemDir = true;

#define BEGIN_TICK_COUNT()      if (NULL != pStats) startTime = GetTickCount()
#define END_TICK_COUNT(field)   if (NULL != pStats) pStats->m_values[PrdTranslationStats::field].Add(GetTickCount() - startTime)

unsigned int PrdTranslator::GetKernelCallStackAdditionalRecordsCount(unsigned int callersCount, bool is64Bit)
{
    const unsigned int VALUE_SIZE = is64Bit ? sizeof(ULONG64) : sizeof(ULONG32);
    unsigned int totalBytesCount = offsetof(PRD_KERNEL_CSS_DATA_RECORD, m_CallStack32) + (callersCount * VALUE_SIZE);
    return (totalBytesCount - 1) / PRD_RECORD_SIZE;
}

unsigned int PrdTranslator::GetUserCallStackAdditionalRecordsCount(unsigned int callersCount, bool is64Bit)
{
    const unsigned int VALUE_SIZE = is64Bit ? sizeof(ULONG64) : sizeof(ULONG32);
    unsigned int totalBytesCount = offsetof(PRD_USER_CSS_DATA_EXT_RECORD, m_CallStack32) + (callersCount * VALUE_SIZE);
    return (totalBytesCount - 1) / PRD_RECORD_SIZE + 1;
}

unsigned int PrdTranslator::GetVirtualStackAdditionalRecordsCount(unsigned int valuesCount)
{
    unsigned int totalBytesCount = offsetof(PRD_VIRTUAL_STACK_RECORD, m_Values) + valuesCount * (sizeof(ULONG32) + sizeof(USHORT));
    return (totalBytesCount - 1) / PRD_RECORD_SIZE;
}


static inline bool AtomicCyclicSub(volatile gtUInt64& addend, gtUInt64 value, gtUInt64 initial)
{
    bool ret;

    gtInt64 oldValue, newValue;

    do
    {
        oldValue = static_cast<gtInt64>(addend);

        newValue = oldValue - static_cast<gtInt64>(value);
        ret = (0LL >= newValue);

        if (ret)
        {
            newValue += static_cast<gtInt64>(initial);
        }

    }
    while (!AtomicCompareAndSwap(reinterpret_cast<volatile gtInt64&>(addend), oldValue, newValue));

    return ret;
}

// GetSysInfo
//
void GetSysInfo(DWORD* NbrProcs, DWORD* FileAllocGranularity)
{
    if (!NbrProcs || !FileAllocGranularity)
    {
        return;
    }

    // Get the system information from the OS.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    SYSTEM_INFO SysInfo;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    BOOL use64bit = true;
#else
    BOOL use64bit = false;
    IsWow64Process(GetCurrentProcess(), &use64bit);
#endif

    if (use64bit)
    {
        GetNativeSystemInfo(&SysInfo);
    }
    else
    {
        GetSystemInfo(&SysInfo);
    }

    *NbrProcs = SysInfo.dwNumberOfProcessors;
    *FileAllocGranularity = SysInfo.dwAllocationGranularity;
#endif
}

// GetSysInfo
//
void GetFreeVM(gtUInt64& pFreeVM)
{
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    GlobalMemoryStatusEx(&memStatus);

    pFreeVM = memStatus.ullAvailVirtual; // in bytes

    return;
}

#ifdef AMDT_ENABLE_CPUPROF_DB
static gtUInt32 generateFuncId(gtUInt16 moduleId, gtUInt16 funcSeq)
{
    gtUInt32 funcId = moduleId;
    funcId = (funcId << 16) | funcSeq;
    return funcId;
}
#endif

PrdTranslator::ProcessInfo::ProcessInfo(ProcessIdType processId) : m_processId(processId)
{
    m_exeAnalyzers.reserve(64);
    m_userCallStacks.reserve(32);
}

PrdTranslator::ProcessInfo::~ProcessInfo()
{
    for (ThreadUserCallStacksMap::iterator it = m_userCallStacks.begin(), itEnd = m_userCallStacks.end(); it != itEnd; ++it)
    {
        PeriodicUserCallStackMap* pPeriodicCallStackMap = it->second;

        if (NULL != pPeriodicCallStackMap)
        {
            delete pPeriodicCallStackMap;
        }
    }

    for (ExecutableAnalyzersMap::iterator it = m_exeAnalyzers.begin(), itEnd = m_exeAnalyzers.end(); it != itEnd; ++it)
    {
        ExecutableAnalyzer* pExeAnalyzer = it->second;

        if (NULL != pExeAnalyzer)
        {
            delete pExeAnalyzer;
        }
    }
}

ExecutableAnalyzer* PrdTranslator::ProcessInfo::AcquireExecutableAnalyzer(gtVAddr va)
{
    ExecutableAnalyzer* pExeAnalyzer = NULL;
    VAddrRange range = { va, va };

    m_lockAnalyzers.lockRead();
    ExecutableAnalyzersMap::iterator it = m_exeAnalyzers.find(range);

    if (it != m_exeAnalyzers.end())
    {
        pExeAnalyzer = it->second;
        m_lockAnalyzers.unlockRead();
    }
    else
    {
        m_lockAnalyzers.unlockRead();
        ExecutableFile* pExe = fnFindExecutableFile(m_processId, va);

        if (NULL != pExe)
        {
            range.m_min = pExe->GetLoadAddress();
            range.m_max = range.m_min + static_cast<gtVAddr>(pExe->GetImageSize() - 1);

            m_lockAnalyzers.lockWrite();
            ExecutableAnalyzer*& pMapExeAnalyzer = m_exeAnalyzers[range];

            if (NULL == pMapExeAnalyzer)
            {
                pMapExeAnalyzer = new ExecutableAnalyzer(*pExe);
            }

            pExeAnalyzer = pMapExeAnalyzer;
            m_lockAnalyzers.unlockWrite();
        }
    }

    return pExeAnalyzer;
}

PrdTranslator::ProcessInfo* PrdTranslator::FindProcessInfo(ProcessIdType pid) const
{
    ProcessInfo* pProcessInfo;

    m_processInfosLock.lockRead();
    gtMap<ProcessIdType, ProcessInfo*>::const_iterator it = m_processInfos.find(pid);
    pProcessInfo = (m_processInfos.end() != it) ? it->second : NULL;
    m_processInfosLock.unlockRead();

    return pProcessInfo;
}

PrdTranslator::ProcessInfo& PrdTranslator::AcquireProcessInfo(ProcessIdType pid)
{
    ProcessInfo* pProcessInfo = FindProcessInfo(pid);

    if (NULL == pProcessInfo)
    {
        m_processInfosLock.lockWrite();
        ProcessInfo*& pMapProcessInfo = m_processInfos[pid];

        if (NULL == pMapProcessInfo)
        {
            pMapProcessInfo = new ProcessInfo(pid);
        }

        m_processInfosLock.unlockWrite();

        pProcessInfo = pMapProcessInfo;
    }

    return *pProcessInfo;
}

//Uses taskinfo dataFile(.prd->.ti),
PrdTranslator::PrdTranslator(QString dataFile, bool collectStat) : m_pfnProgressBarCallback(NULL),
    m_progressEvent(L"CpuProfile", L"Preparing raw data translation...", 0),
    m_pProfilingDrivers(NULL),
    m_countProfilingDrivers(0U)
{
    m_dataFile = dataFile;
    CssMapCleanup();
    m_eventMap.clear();
    m_ibsFetchCount = 0;
    m_ibsOpCount = 0;
    m_collectStat = collectStat;
    m_pCluInfo = NULL;
    m_runInfo = NULL;
    m_hrFreq = 0;
    m_numWorkerThreads = 0;

    m_is64Sys = false;
#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
    IsWow64Process(GetCurrentProcess(), &m_is64Sys);
#endif

    m_progressAsync = 0;
    m_useProgressSyncObject = false;

    m_pSearchPath = NULL;
    m_pServerList = NULL;
    m_pCachePath = NULL;

#ifdef AMDT_ENABLE_CPUPROF_DB
    m_nextModuleId = 1;
#endif
}

PrdTranslator::~PrdTranslator()
{
    CssMapCleanup();
    m_eventMap.clear();

    if (NULL != m_pCluInfo)
    {
        delete m_pCluInfo;
    }

    if (NULL != m_runInfo)
    {
        delete m_runInfo;
    }

    for (gtMap<ProcessIdType, ProcessInfo*>::iterator it = m_processInfos.begin(), itEnd = m_processInfos.end(); it != itEnd; ++it)
    {
        ProcessInfo* pProcessInfo = it->second;

        if (NULL != pProcessInfo)
        {
            delete pProcessInfo;
        }
    }

    // We use 'free' instead of 'delete' because these strings were created by 'wcsdup'
    //
    if (NULL != m_pSearchPath)
    {
        free(m_pSearchPath);
    }

    if (NULL != m_pServerList)
    {
        free(m_pServerList);
    }

    if (NULL != m_pCachePath)
    {
        free(m_pCachePath);
    }

    if (NULL != m_pProfilingDrivers)
    {
        delete [] m_pProfilingDrivers;
    }

    fnCleanupMaps();
}

void PrdTranslator::InitializeProgressBar(const gtString& caption, bool incremental)
{
    m_progressEvent.setProgress(caption);
    m_progressEvent.setIncrement(false);
    m_progressEvent.setValue(0);
    UpdateProgressBar();
    m_progressEvent.setIncrement(incremental);
    m_progressEvent.setValue(1);
}

void PrdTranslator::IncrementProgressBar(int value)
{
    m_progressEvent.setIncrement(true);
    m_progressEvent.setValue(value);
    UpdateProgressBar();
}

void PrdTranslator::UpdateProgressBar()
{
    if (NULL != m_pfnProgressBarCallback)
    {
        m_pfnProgressBarCallback(m_progressEvent);
    }
}

void PrdTranslator::UpdateProgressBar(gtUInt64 bytesReadSoFar, gtUInt64 totalBytes)
{
    bool incremental = m_progressEvent.increment();
    m_progressEvent.setIncrement(false);
    m_progressEvent.setValue(static_cast<int>((bytesReadSoFar * 100ULL) / totalBytes));
    UpdateProgressBar();
    m_progressEvent.setIncrement(incremental);
}

void PrdTranslator::CompleteProgressBar()
{
    UpdateProgressBar(1ULL, 1ULL);
}

void PrdTranslator::AddBytesToProgressBar(gtUInt64 bytes)
{
    if (AtomicCyclicSub(m_progressThreshold, bytes, m_progressStride))
    {
        UpdateProgressBar();
    }
}

void PrdTranslator::AsyncAddBytesToProgressBar(gtUInt64 bytes)
{
    if (AtomicCyclicSub(m_progressThreshold, bytes, m_progressStride))
    {
        AtomicAdd(m_progressAsync, 1);

        if (m_useProgressSyncObject)
        {
            m_progressSyncObject.unlock();
        }
    }
}

void PrdTranslator::SetDebugSymbolsSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath)
{
    // We use 'free' instead of 'delete' because these strings were created by 'wcsdup'
    //
    if (NULL != m_pSearchPath)
    {
        free(m_pSearchPath);
    }

    if (NULL != m_pServerList)
    {
        free(m_pServerList);
    }

    if (NULL != m_pCachePath)
    {
        free(m_pCachePath);
    }

    m_pSearchPath = (NULL != pSearchPath) ? wcsdup(pSearchPath) : NULL;
    m_pServerList = (NULL != pServerList) ? wcsdup(pServerList) : NULL;
    m_pCachePath  = (NULL != pCachePath)  ? wcsdup(pCachePath)  : NULL;
}


void PrdTranslator::CssMapCleanup()
{
}

unsigned int PrdTranslator::GetProfileType()
{
    PrdReader prdReader;

    if (S_OK != prdReader.Initialize(m_dataFile.toStdWString().c_str()))
    {
        return PROF_INVALIDTYPE;
    }

    unsigned int types = prdReader.GetProfileType();

    if (0 == (types & PROF_TBP) && 0 == (types & PROF_EBP) && 0 == (types & PROF_IBS))
    {
        types = PROF_INVALIDTYPE;
    }

    return types;
}

//FIXME [Suravee]: Should not need this
#if 0
bool PrdTranslator::GetProfileEvents(DcEventConfig* pEventConfig, unsigned int numOfEvents, EventNormValueMap* pNorms)
{
    bool bRet = false;

    if (NULL == pEventConfig)
    {
        return false;
    }

    if (NULL != pNorms)
    {
        pNorms->clear();
    }

    PrdReader prdReader;

    if (S_OK != prdReader.Initialize(m_dataFile.toStdWString().c_str()))
    {
        return false;
    }

    unsigned int groupNum;
    unsigned int cfgCnt = 0;

    groupNum = prdReader.GetEventGroupCount();
    cfgCnt = prdReader.GetEventCount();

    if (numOfEvents >= cfgCnt)
    {
        EventCfgInfo* pEvtCfg = new EventCfgInfo[cfgCnt];

        if (NULL != pEvtCfg)
        {
            HRESULT hr = prdReader.GetEventInfo(pEvtCfg, cfgCnt);

            if (S_OK == hr)
            {
                DcEventConfig* pE = pEventConfig;

                for (unsigned int i = 0; i < cfgCnt; i++)
                {
                    pE[i].pmc.perf_ctl = pEvtCfg[i].ctl.perf_ctl;
                    pE[i].eventCount = pEvtCfg[i].ctr;

                    if (NULL != pNorms)
                    {
                        EventMaskType encodedEvent = EncodeEvent(pEvtCfg[i].ctl);
                        (*pNorms) [encodedEvent] = pEvtCfg[i].ctr * groupNum / pEvtCfg[i].numApperance;
                    }
                }

                prdReader.Close();
            }

            delete [] pEvtCfg;
            pEvtCfg = NULL;
        }
    }

    return true;
}
#endif


bool PrdTranslator::InitPrdReader(PrdReader* pReader, const wchar_t* pFileName, gtUInt64* pLastUserCssRecordOffset, QWidget* pParent)
{
    bool bRet = true;

    if ((NULL == pReader) || (NULL == pFileName))
    {
        return false;
    }

    int nTry = 5;
    HRESULT hr = E_FAIL;

    //Give the file time to be finished writing.
    while (nTry-- > 0)
    {
        hr = pReader->Initialize(pFileName, pLastUserCssRecordOffset);

        if (S_OK == hr) { break; }

        Sleep(500);
    }

    if (S_OK != hr)
    {
        QString msg = "Can't open raw data file. ("
                      + QString::fromWCharArray(pFileName) + ")\nMaybe no samples were taken.";

        if (NULL != pParent)
        {
            QMessageBox::information(pParent, "Notification", msg,
                                     QMessageBox::Ok);
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(msg.toStdWString().c_str(), OS_DEBUG_LOG_ERROR);
        }

        return false;
    }

    // Get the RI file path
    osFilePath riFilePath(pFileName);
    riFilePath.setFileExtension(L"ri");

    if (NULL != m_runInfo)
    {
        delete m_runInfo;
        m_runInfo = NULL;
    }

    m_runInfo = new RunInfo();

    // Read the data from the RI file
    hr = fnReadRIFile(riFilePath.asString().asCharArray(), m_runInfo);

    if (hr == S_OK)
    {
        if (m_runInfo->m_isProfilingClu)
        {
            if (NULL != m_pCluInfo)
            {
                delete m_pCluInfo;
                m_pCluInfo = NULL;
            }

            m_pCluInfo = new CluInfo(pReader->GetL1DcAssoc(),
                                     pReader->GetL1DcLineSize(),
                                     pReader->GetL1DcLinesPerTag(),
                                     pReader->GetL1DcSize());

            if (NULL == m_pCluInfo)
            {
                bRet = false;
            }
            else
            {
                AddCluEventsToMap();
            }
        }
    }

    unsigned int profType = pReader->GetProfileType();

    if ((PROF_TBP & profType) != 0)
    {
        gtUInt64 resolution = 0;
        GetTimerInterval(&resolution);
        m_eventMap.insert(EventMap::value_type(static_cast<EventMaskType>(GetTimerEvent()), (DWORD) resolution));
    }

    if ((PROF_EBP & profType) != 0)
    {
        unsigned int groupNum;
        unsigned int cfgCnt = 0;

        groupNum = pReader->GetEventGroupCount();
        cfgCnt = pReader->GetEventCount();
        EventCfgInfo* pEvtCfg = new EventCfgInfo[cfgCnt];

        if (NULL == pEvtCfg)
        {
            bRet = false;
            OS_OUTPUT_DEBUG_LOG(L"Unable to allocate memory for the events", OS_DEBUG_LOG_ERROR);
        }

        hr = pReader->GetEventInfo(pEvtCfg, cfgCnt);

        if (S_OK != hr)
        {
            bRet = false;
            cfgCnt = 0;
        }

        for (unsigned int i = 0; i < cfgCnt; i++)
        {
            EventMaskType encodedEvent = EncodeEvent(pEvtCfg[i].ctl);
            m_norms[encodedEvent] = pEvtCfg[i].ctr * groupNum / pEvtCfg[i].numApperance;
            m_eventMap.insert(EventMap::value_type(encodedEvent, (gtUInt32) m_norms[encodedEvent]));
        }

        if (NULL != pEvtCfg)
        {
            delete [] pEvtCfg;
        }
    }

    if ((PROF_IBS & profType) != 0)
    {
        pReader->GetIBSConfig(&m_ibsFetchCount, &m_ibsOpCount) ;

        if (m_ibsFetchCount != 0)
        {
            AddIBSFetchEventsToMap(pReader->GetCpuFamily(), pReader->GetCpuModel());
        }

        if (m_runInfo->m_isProfilingIbsOp && (m_ibsOpCount != 0))
        {
            AddIBSOpEventsToMap(pReader->GetCpuFamily(), pReader->GetCpuModel(), true, true, true);
        }
    }

    m_hrFreq = pReader->GetHrFreq();

    return bRet;
}


void PrdTranslator::AggregateKnownModuleSampleData(
    SampleInfo& sampInfo,
    TiModuleInfo* pModInfo,
    NameModuleMap* pMMap,
    PidModaddrItrMap& pidModaddrItrMap,
    bool& b_is32bit,
    unsigned int samplesCount,
    PrdTranslationStats* const pStats)
{
    gtString ModName;
    gtString funcName;
    gtString jncFileName;
    gtString srcFileName;
    gtUInt64 startAddress = pModInfo->ModuleStartAddr;
    gtUInt32 funcSize = 0;
    unsigned srcLineNum = 0U;
    gtUInt32 functionId = UNKNOWN_FUNCTION_ID;

    PidModaddrKey key(pModInfo->processID, pModInfo->ModuleStartAddr);
    PidModaddrItrMap::iterator pmait = pidModaddrItrMap.find(key);

    if (pmait == pidModaddrItrMap.end())
    {
        ModName = gtString(pModInfo->pModulename);

        NameModuleMap::iterator mit = pMMap->find(ModName);

        if (mit == pMMap->end())
        {
            // Init
            CpuProfileModule mod;
            InitNewModule(mod, pModInfo, ModName, funcName,
                          jncFileName, srcFileName, sampInfo.pid);

            // Insert
            pMMap->insert(NameModuleMap::value_type(ModName, mod));

            // Find again
            mit = pMMap->find(ModName);
        }

#ifdef AMDT_ENABLE_CPUPROF_DB
        else
        {
            // Module is already present in the map. Just append the new instance info.
            gtUInt64 pid = pModInfo->processID;
            gtUInt64 loadAddr = pModInfo->ModuleStartAddr;
            gtUInt32 instanceId = pModInfo->instanceId;
            bool isFound = false;

            for (auto& it : mit->second.m_moduleInstanceInfo)
            {
                // Just check for matching PID and module load address
                // We are assuming that only module gets loaded to loadAddr for a PID
                if ((std::get<0>(it) == pid) && (std::get<1>(it) == loadAddr))
                {
                    isFound = true;
                    break;
                }
            }

            if (!isFound)
            {
                mit->second.m_moduleInstanceInfo.emplace_back(pid, loadAddr, instanceId);
            }
        }

#endif

        if ((pModInfo->moduleType == evJavaModule || pModInfo->moduleType == evManaged || pModInfo->moduleType == evOCLModule) &&
            NULL != pModInfo->pFunctionName && L'\0' != pModInfo->pFunctionName[0])
        {
            funcName = pModInfo->pFunctionName;
            wostringstream ss;

            if (pModInfo->moduleType != evOCLModule)
            {
                ss << sampInfo.pid << L"/" << pModInfo->pJncName;
            }
            else
            {
                ss << pModInfo->pJncName;
            }

            funcSize = pModInfo->Modulesize;
            srcFileName = pModInfo->pJavaSrcFileName;
            jncFileName = ss.str().c_str();
        }

        pmait = pidModaddrItrMap.insert(PidModaddrItrMap::value_type(key, mit)).first;
    }

    CpuProfileModule& module = pmait->second->second;

    if (evPEModule == pModInfo->moduleType)
    {
        DWORD startTime = 0L;

        // Baskar: We already have pExecutable in pModInfo;
        BEGIN_TICK_COUNT();
        ExecutableFile* pExecutable = pModInfo->pPeFile;

        if (nullptr == pExecutable)
        {
            pExecutable = fnFindExecutableFile(sampInfo.pid, sampInfo.address);
        }

        END_TICK_COUNT(querySymbolEngine);

        if (NULL != pExecutable)
        {
            SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

            if (NULL != pSymbolEngine)
            {
                gtRVAddr rva = pExecutable->VaToRva(sampInfo.address);
                gtRVAddr inlineRva = pSymbolEngine->TranslateToInlineeRVA(rva);
                gtVAddr addr = sampInfo.address - rva + inlineRva;

                const CpuProfileFunction* pFunc = module.findFunction(addr);

                if (NULL == pFunc || module.isUnchartedFunction(*pFunc))
                {
                    const FunctionSymbolInfo* pFuncInfo;

                    // LookupFunction will discover both the inlined and non-inlined functions
                    pFuncInfo = pSymbolEngine->LookupFunction(pExecutable->VaToRva(sampInfo.address), NULL, true);

                    if (NULL != pFuncInfo)
                    {
                        module.m_base = startAddress;
                        module.m_size = pExecutable->GetImageSize();

                        sampInfo.address = addr;

                        if (NULL != pFuncInfo->m_pName && L'!' != pFuncInfo->m_pName[0])
                        {
                            funcName = pFuncInfo->m_pName;
                        }

                        startAddress += pFuncInfo->m_rva;
                        funcSize = pFuncInfo->m_size;

                        pFunc = module.findFunction(startAddress);

                        if (NULL == pFunc || module.isUnchartedFunction(*pFunc))
                        {
                            SourceLineInfo sourceLine;

                            if (pSymbolEngine->FindSourceLine(pFuncInfo->m_rva, sourceLine))
                            {
                                int srcFileNameLen = static_cast<int>(wcslen(sourceLine.m_filePath));

                                if (!afUtils::ConvertCygwinPath(sourceLine.m_filePath, srcFileNameLen, srcFileName))
                                {
                                    srcFileName.assign(sourceLine.m_filePath, srcFileNameLen);
                                }

                                srcLineNum = sourceLine.m_line;
                            }
                        }

#ifdef AMDT_ENABLE_CPUPROF_DB
                        functionId = generateFuncId(module.m_moduleId, pFuncInfo->m_funcId);
#endif
                    }
                }
                else
                {
                    sampInfo.address = addr;
                    funcName = pFunc->getFuncName();
                    startAddress = pFunc->getBaseAddr();
                    funcSize = pFunc->getSize();
                    // No need to fill functionId, as the function is already present in m_funcMap of CpuProfileModule
                }
            }
        }
    }

    gtUInt16 eventType;
    DecodeEvent(sampInfo.event, &eventType, NULL, NULL, NULL);

    // CLU profiles IBS Op event, record IBS sample only when profiling IBS Op
    if (!IsIbsOpEvent(eventType) || m_runInfo->m_isProfilingIbsOp)
    {
        // update the function node under module
        // to avoid same module are loaded multiple times and loading addresses are different.
        // I will use offset for function nodes here. --Lei
        module.recordSample(sampInfo, samplesCount, startAddress, funcSize, funcName, jncFileName, srcFileName, srcLineNum, functionId);
    }

    b_is32bit = module.m_is32Bit;
}


void PrdTranslator::InitNewModule(CpuProfileModule& mod,
                                  TiModuleInfo* pModInfo,
                                  const gtString& ModName,
                                  const gtString& FuncName,
                                  const gtString& JncName,
                                  const gtString& JavaSrcFileName,
                                  ProcessIdType pid)
{
    GT_UNREFERENCED_PARAMETER(FuncName);
    GT_UNREFERENCED_PARAMETER(JncName);
    GT_UNREFERENCED_PARAMETER(JavaSrcFileName);

    //////////////////////////////////////
    // we did not find the module in global module map
    switch (pModInfo->moduleType)
    {
        case evPEModule:
            mod.m_modType = CpuProfileModule::UNMANAGEDPE;
#ifdef AMDT_ENABLE_CPUPROF_DB

            if (nullptr != pModInfo && nullptr != pModInfo->pPeFile)
            {
                mod.m_checksum = pModInfo->pPeFile->GetChecksum();
                mod.m_isDebugInfoAvailable = pModInfo->pPeFile->IsDebugInfoAvailable();
            }

#endif // AMDT_ENABLE_CPUPROF_DB
            break;

        case evJavaModule:
            mod.m_modType = CpuProfileModule::JAVAMODULE;
            break;

        case evManaged:
            mod.m_modType = CpuProfileModule::MANAGEDPE;
            break;

        case evOCLModule:

        //mod.m_modType = CpuProfileModule::OCLMODULE;
        //break;
        case evInvalidType:
            break;
    }

    mod.setPath(ModName);
    mod.m_base = pModInfo->ModuleStartAddr;
#ifdef AMDT_ENABLE_CPUPROF_DB
    mod.m_moduleId = AtomicAdd(m_nextModuleId, 1);
    mod.m_moduleInstanceInfo.emplace_back(pModInfo->processID, pModInfo->ModuleStartAddr, pModInfo->instanceId);
#endif

    // currently I don't use size and tsc in CA --Lei
    mod.m_size = 0;

    if (m_is64Sys)
    {
        // determines if the module is a 64-bit module
        // it's in kernel space on 64-bit OS, it should be 64-bit module.
        if (pModInfo->ModuleStartAddr > KERNEL64_SPACE_START)
        {
            mod.m_is32Bit = false;
        }
        else
        {
            if (bGetSystemDir)
            {
                system32Dir[0] = L'\0';
                // C:\windows\system32 or C:\winnt\system32
                GetSystemDirectoryW(system32Dir, OS_MAX_PATH);
                bGetSystemDir = false;
            }

            // copy to syswows64
            bool b64bitSystemDir = false;

            if (0 == _wcsnicmp(pModInfo->pModulename, system32Dir, wcslen(system32Dir)))
            {
                b64bitSystemDir = true;
            }

            PVOID oldValue = nullptr;

            if (b64bitSystemDir)
            {
                b64bitSystemDir = Wow64DisableWow64FsRedirection(&oldValue);
            }

            if (nullptr != pModInfo->pPeFile)
            {
                mod.m_is32Bit = !(pModInfo->pPeFile->Is64Bit());
            }
            else
            {
                mod.m_is32Bit = fnIsJITProcess32Bit(pid);
            }

            if (b64bitSystemDir)
            {
                Wow64RevertWow64FsRedirection(oldValue);
            }
        }
    }
    else
    {
        mod.m_is32Bit = fnIsJITProcess32Bit(pid);
    }
}


void PrdTranslator::AggregateUnknownModuleSampleData(
    SampleInfo& sampInfo,
    TiModuleInfo* pModInfo,
    NameModuleMap* pMMap,
    bool& b_is32bit,
    unsigned int samplesCount)
{
    gtString ModName;
    gtString FuncName;
    gtString JncName;
    gtString JavaSrcFileName;

    //////////////////////////////////////
    // Here are unknown samples
    gtString tmp_modName;
    int unknownType = CpuProfileModule::UNKNOWNMODULE;

    gtUInt64 kernelSpace = KERNEL_SPACE_START;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    BOOL on64 = false;
    IsWow64Process(GetCurrentProcess(), &on64);

    if (on64)
    {
        kernelSpace = KERNEL64_SPACE_START;
    }

#endif

    if (pModInfo->sampleAddr > kernelSpace)
    {
        // Unknown kernel module
        tmp_modName = gtString(UNKNOWN_KERNEL_SAMPLES);
        unknownType = CpuProfileModule::UNKNOWNKERNELSAMPLES;
    }
    else
    {
        // unknown user module
        wchar_t buf[UNKNOWN_MODULE_PID_BUF_LEN] = {L'\0'};
        swprintf(buf, sizeof(buf) - 1, L" (PID %d)", sampInfo.pid);
        tmp_modName = UNKNOWN_MODULE_STR.toStdWString().c_str();
        tmp_modName += gtString(buf);
    }

    ModName = tmp_modName;

    NameModuleMap::iterator mit = pMMap->find(ModName);

    gtUInt16 eventType;
    DecodeEvent(sampInfo.event, &eventType, NULL, NULL, NULL);

    if (pMMap->end() == mit)
    {
        // we did not find the module in global module map

        CpuProfileModule mod;
        mod.m_modType = unknownType;
        mod.m_base = 0;
        mod.setPath(ModName);
        mod.m_size = 0;
        mod.m_is32Bit = fnIsJITProcess32Bit(pModInfo->processID);
        b_is32bit = mod.m_is32Bit;
#ifdef AMDT_ENABLE_CPUPROF_DB
        mod.m_moduleId = AtomicAdd(m_nextModuleId, 1);
        mod.m_moduleInstanceInfo.emplace_back(pModInfo->processID, pModInfo->ModuleStartAddr, pModInfo->instanceId);
#endif

        // CLU profiles IBS Op event, record IBS sample only when profiling IBS Op
        if (!IsIbsOpEvent(eventType) || m_runInfo->m_isProfilingIbsOp)
        {
            // update the function node under module
            // to avoid same module are loaded multiple times and loading addresses are different.
            // I will use offset for function nodes here. --Lei
            mod.recordSample(sampInfo, samplesCount, 0, 0,
                             FuncName, JncName, JavaSrcFileName);
        }

        pMMap->insert(NameModuleMap::value_type(ModName, mod));

    }
    else
    {
        b_is32bit = mit->second.m_is32Bit;
#ifdef AMDT_ENABLE_CPUPROF_DB
        // Module is already present in the map. Just append the new instance info.
        gtUInt64 pid = pModInfo->processID;
        gtUInt64 loadAddr = pModInfo->ModuleStartAddr;
        gtUInt32 instanceId = pModInfo->instanceId;
        bool isFound = false;

        for (auto& it : mit->second.m_moduleInstanceInfo)
        {
            // Just check for matching PID and module load address
            // We are assuming that only module gets loaded to loadAddr for a PID
            if ((std::get<0>(it) == pid) && (std::get<1>(it) == loadAddr))
            {
                isFound = true;
                break;
            }
        }

        if (!isFound)
        {
            mit->second.m_moduleInstanceInfo.emplace_back(pid, loadAddr, instanceId);
        }

#endif

        // CLU profiles IBS Op event, record IBS sample only when profiling IBS Op
        if (!IsIbsOpEvent(eventType) || m_runInfo->m_isProfilingIbsOp)
        {
            mit->second.recordSample(sampInfo, samplesCount, 0, 0,
                                     FuncName, JncName, JavaSrcFileName);
        }
    }
}


bool PrdTranslator::AggregateSampleData(RecordDataStruct prdRecord,
                                        TiModuleInfo* pModInfo,
                                        PidProcessMap* pPMap,
                                        NameModuleMap* pMMap,
                                        PidModaddrItrMap* pidModaddrItrMap,
                                        unsigned int samplesCount,
                                        PrdTranslationStats* const pStats)
{
    bool bRet = false;

    if (!pPMap || !pMMap || !pModInfo)
    {
        return bRet;
    }

    if (prdRecord.m_ProcessorID >= MAX_CPUS)
    {
        return bRet;
    }

    bool b_is32bit = true;

    //bit 17 OS, bit 16 USR >> 16 = prdRecord.m_eventBitMask
    SampleInfo sampInfo(prdRecord.m_RIP,
                        prdRecord.m_PID,
                        prdRecord.m_ThreadHandle,
                        prdRecord.m_ProcessorID,
                        EncodeEvent(prdRecord.m_EventType,
                                    prdRecord.m_EventUnitMask,
                                    (prdRecord.m_eventBitMask & 2),
                                    (prdRecord.m_eventBitMask & 1)));

    if ((NULL != pModInfo->pModulename) && (wcslen(pModInfo->pModulename)))
    {
        AggregateKnownModuleSampleData(sampInfo, pModInfo,
                                       pMMap, *pidModaddrItrMap, b_is32bit, samplesCount, pStats);
    }
    else
    {
        AggregateUnknownModuleSampleData(sampInfo, pModInfo,
                                         pMMap, b_is32bit, samplesCount);
    }

    AggregatePidSampleData(prdRecord, pModInfo, pPMap, b_is32bit,
                           samplesCount);

    return true;
}


void PrdTranslator::AggregatePidSampleData(
    RecordDataStruct& prdRecord,
    TiModuleInfo* pModInfo,
    PidProcessMap* pPMap,
    bool b_is32bit,
    unsigned int samplesCount)
{
    // HOTSPOT
    PidProcessMap::iterator p_it = pPMap->find(prdRecord.m_PID);

    if (pPMap->end() == p_it)
    {
        wchar_t processName[OS_MAX_PATH + 1];
        processName[0] = L'\0';

        CpuProfileProcess temp_process;

        if (S_OK != fnFindProcessName(prdRecord.m_PID, processName, OS_MAX_PATH))
        {
            gtString tmp;
            wchar_t buf[UNKNOWN_MODULE_PID_BUF_LEN] = {L'\0'};
            swprintf(buf, sizeof(buf) - 1, L" (PID %lld)", prdRecord.m_PID);
            tmp = UNKNOWN_MODULE_STR.toStdWString().c_str();
            tmp += gtString(buf);
            temp_process.setPath(tmp);
            temp_process.m_is32Bit = b_is32bit;
        }
        else
        {
            temp_process.setPath(gtString(processName));

            if (m_is64Sys)
            {
                if (bGetSystemDir)
                {
                    system32Dir[0] = L'\0';
                    // C:\windows\system32 or C:\winnt\system32
                    GetSystemDirectoryW(system32Dir, OS_MAX_PATH);
                    bGetSystemDir = false;
                }

                bool b64bitSystemDir = false;

                if (0 == _wcsnicmp(processName, system32Dir, wcslen(system32Dir)))
                {
                    b64bitSystemDir = true;
                }

                PVOID oldValue = nullptr;

                if (b64bitSystemDir)
                {
                    b64bitSystemDir = Wow64DisableWow64FsRedirection(&oldValue);
                }

                ExecutableFile* pExecutable = ExecutableFile::Open(processName);

                if (NULL != pExecutable)
                {
                    temp_process.m_is32Bit = !pExecutable->Is64Bit();
                    delete pExecutable;
                    pExecutable = NULL;
                }
                else
                {
                    temp_process.m_is32Bit = fnIsJITProcess32Bit(prdRecord.m_PID);
                }

                if (b64bitSystemDir)
                {
                    Wow64RevertWow64FsRedirection(oldValue);
                }
            }
            else
            {
                temp_process.m_is32Bit = fnIsJITProcess32Bit(prdRecord.m_PID);
            }
        }

        temp_process.m_hasCss = false;
        pPMap->insert(PidProcessMap::value_type(static_cast<ProcessIdType>(prdRecord.m_PID), temp_process));
        p_it = pPMap->find(prdRecord.m_PID);
    }

    // For managed code, overwrite the process name.
    if (evJavaModule == pModInfo->moduleType || evManaged == pModInfo->moduleType)
    {
        QString tmp = QString::fromWCharArray(p_it->second.getPath().asCharArray());

        if (tmp.contains(UNKNOWN_MODULE_STR))
        {
            p_it->second.setPath(gtString(pModInfo->pModulename));
        }
    }

    if (!IsIbsOpEvent(prdRecord.m_EventType) || m_runInfo->m_isProfilingIbsOp)
    {
        //bit 17 OS, bit 16 USR >> 16 = prdRecord.m_eventBitMask
        SampleKey sKey(prdRecord.m_ProcessorID,
                       EncodeEvent(prdRecord.m_EventType,
                                   prdRecord.m_EventUnitMask, (prdRecord.m_eventBitMask & 2), (prdRecord.m_eventBitMask & 1)));

        p_it->second.addSamples(sKey, samplesCount);
    }
}


static wchar_t sSessionDir [OS_MAX_PATH + 1];
enum eRecordType
{
    evTBPEBPRecord      = 0,        // timer based profile or event based profile record
    evIBSFetchRecord    = 1,        // IBS fetch record
    evIBSOpRecord       = 2,        // IBS Op record
    evCLURecord         = 3,
    evInvalidRecord     = 4
};


// This is helper function to get initialize module info structure and get module
// information via task info interface
HRESULT GetModuleInfoHelper(void* pVoid, TiModuleInfo* pModInfo, eRecordType dataType, QString modPath)
{
    if (!pVoid || !pModInfo)
    {
        return E_INVALIDARG;
    }

    if (dataType >= evInvalidRecord)
    {
        return E_INVALIDARG;
    }

    smoduleName[0] = L'\0';
    sfunctionName[0] = L'\0';
    sjncName[0] = L'\0';
    sjavaSrcFileName[0] = L'\0';
    sSessionDir[0] = L'\0';

    switch (dataType)
    {
        case evTBPEBPRecord:
        {
            RecordDataStruct* prdRecord = (RecordDataStruct*) pVoid;
            pModInfo->processID = prdRecord->m_PID;
            pModInfo->sampleAddr = prdRecord->m_RIP & ERBT_713_NON_CANONICAL_MASK;
            pModInfo->cpuIndex = prdRecord->m_ProcessorID;
            pModInfo->deltaTick = prdRecord->m_DeltaTick;
        }
        break;

        case evIBSFetchRecord:
        {
            IBSFetchRecordData* pIBSFetchRec = (IBSFetchRecordData*) pVoid;
            pModInfo->processID = pIBSFetchRec->m_PID;
            pModInfo->sampleAddr = pIBSFetchRec->m_RIP & ERBT_713_NON_CANONICAL_MASK;
            pModInfo->cpuIndex = pIBSFetchRec->m_ProcessorID;
            pModInfo->deltaTick = pIBSFetchRec->m_DeltaTick;
        }
        break;

        case evIBSOpRecord:
        {
            IBSOpRecordData* pIBSOpRec = (IBSOpRecordData*) pVoid;
            pModInfo->processID = pIBSOpRec->m_PID;
            pModInfo->sampleAddr = pIBSOpRec->m_RIP & ERBT_713_NON_CANONICAL_MASK;
            pModInfo->cpuIndex = pIBSOpRec->m_ProcessorID;
            pModInfo->deltaTick = pIBSOpRec->m_DeltaTick;
        }
        break;

        case evCLURecord:
        {
            // Keeping deltaTick as 0, will correct it in subsequent changes
            CLUKey* key = (CLUKey*) pVoid;
            pModInfo->processID = key->ProcessID;
            pModInfo->sampleAddr = key->RIP & ERBT_713_NON_CANONICAL_MASK;
            pModInfo->cpuIndex = key->core;
            pModInfo->deltaTick = 0;
        }
        break;

        case evInvalidRecord:
        {
            pModInfo->processID = 0;
            pModInfo->sampleAddr = 0;
            pModInfo->cpuIndex = 0;
            pModInfo->deltaTick = 0;
        }
        break;
    }

    pModInfo->funNameSize = pModInfo->jncNameSize = pModInfo->namesize = pModInfo->sesdirsize = OS_MAX_PATH;
    pModInfo->pModulename = smoduleName;
    pModInfo->pFunctionName = sfunctionName;
    pModInfo->pJncName = sjncName;
    pModInfo->srcfilesize = OS_MAX_PATH;
    pModInfo->pJavaSrcFileName = sjavaSrcFileName;
    pModInfo->moduleType = evPEModule;
    pModInfo->ModuleStartAddr = 0;
    pModInfo->Modulesize = 0;
    pModInfo->pSessionDir = sSessionDir;
    pModInfo->pSessionDir[0] = L'\0';
    pModInfo->pPeFile = nullptr;
    pModInfo->CSvalue = 0;
    pModInfo->FunStartAddr = 0;
    pModInfo->kernel = false;

    int pos = modPath.lastIndexOf('\\');

    if (-1 != pos && OS_MAX_PATH >= pos)
    {
        int sz = modPath.size();

        if (OS_MAX_PATH < sz)
        {
            if ((OS_MAX_PATH * 2) > sz)
            {
                wchar_t tmp[OS_MAX_PATH * 2];
                modPath.toWCharArray(tmp);

                memcpy(pModInfo->pSessionDir, tmp, pos * sizeof(wchar_t));
                pModInfo->pSessionDir[pos] = L'\0';
            }
        }
        else
        {
            modPath.toWCharArray(pModInfo->pSessionDir);
            pModInfo->pSessionDir[pos] = L'\0';
        }
    }

    HRESULT hr = fnGetModuleInfo(pModInfo);

    if (S_OK != hr)
    {
        smoduleName[0] = L'\0';
    }

    return hr;
}


//S_OK - everything worked ok
//E_UNEXPECTED - Could not write the new file
//E_INVALIDARG - pMissedInfo was not passed in
//E_ACCESSDENIED - Could not read raw data file
//E_ABORT - No samples for the process filters provided
//Writes the data to the file in proFile, heh heh
HRESULT PrdTranslator::TranslateData(QString proFile,
                                     MissedInfoType* pMissedInfo,
                                     QStringList processFilters,
                                     QStringList targetPidList,
                                     QWidget* pApp,
                                     bool bThread,
                                     bool bCLUtil,
                                     bool bLdStCollect,
                                     PfnProgressBarCallback pfnProgressBarCallback)
{
    GT_UNREFERENCED_PARAMETER(bLdStCollect);

    if (NULL == pMissedInfo)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    DWORD startTime;

    QString sessionDir = proFile.section('\\', 0, -2);
    QString sessionName = proFile.section('\\', -1);

    if (m_dataFile.toLower().endsWith(".prd"))
    {
        startTime = GetTickCount();
        hr = OpenTaskInfoFile();

        if (hr != S_OK)
        {
            return hr;
        }

        if (m_collectStat)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Processing TaskInfo file (%d ms)", (GetTickCount() - startTime));
        }
    }

    m_pfnProgressBarCallback = pfnProgressBarCallback;
    UpdateProgressBar(10ULL, 100ULL);

    if (S_OK == hr)
    {
        //Fix for BUG314524 - the idea is not to show progress bar in command line tools.
        QProgressDialog* pProgDlg = NULL;

        if (NULL != pApp)
        {
            pProgDlg = new QProgressDialog(
                "",
                QString::null,
                0, 0, pApp,
                Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            if (!pProgDlg)
            {
                return -1;
            }

            pProgDlg->setWindowModality(Qt::WindowModal);
            pProgDlg->show();
        }

        if (NULL != pProgDlg)
        {
            delete pProgDlg;
        }
    }

    UpdateProgressBar(20ULL, 100ULL);

    if (m_dataFile.toLower().endsWith(".prd"))
    {
        startTime = GetTickCount();

        if (S_OK == hr)
        {
            hr = TranslateDataPrdFile(proFile, pMissedInfo, processFilters, pApp, bThread, bCLUtil, false);
        }

        if (m_collectStat)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: TranslateDataPrdFile (%d ms)", (GetTickCount() - startTime));
        }
    }

#if SUPPORT_CLU

    if ((NULL != m_pCLU) && bCLUtil && (S_OK == hr))
    {
        startTime = GetTickCount();
        m_pCLU->CacheLineCleanup();
        wchar_t sessDir[MAX_PATH];
        wchar_t sessName[MAX_PATH];
        sessionDir.toWCharArray(sessDir);
        sessDir[sessionDir.size()] = L'\0';
        sessionName.toWCharArray(sessName);
        sessName[sessionName.size()] = L'\0';
        hr = m_pCLU->generateCluFile(sessDir, sessName, pApp);

        if (m_collectStat)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Generating CLU file (%d ms)", (GetTickCount() - startTime));
        }
    }

#endif

#if SUPPORT_CLU

    if (bCLUtil)
    {
        delete m_pCLU;
        m_pCLU = NULL;
    }

#endif

    return hr;
}


HRESULT PrdTranslator::OpenTaskInfoFile()
{
    HRESULT hr = S_OK;
    QString tiFile;

    tiFile = m_dataFile;
    tiFile.replace(PRD_FILE_EXT, TI_FILE_EXT);

    //get the info from the ti file
    hr = fnReadModuleInfoFile(tiFile.toStdWString().c_str());

    if (S_OK != hr)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to read the module information file (0x%lx)", hr);
        return E_ACCESSDENIED;
    }

    return hr;
}


HRESULT PrdTranslator::ThreadTranslateDataPrdFile(QString proFile,
                                                  MissedInfoType* pMissedInfo,
                                                  QStringList& processFilters,
                                                  MemoryMap& mapAddress,
                                                  PrdReader& tPrdReader,
                                                  PrdReaderThread& threadPRDReader,
                                                  bool bThread,
                                                  bool bMainThread,
                                                  gtUInt64* pByteRead,
                                                  gtUInt64 totalBytes,
                                                  gtUInt64 baseAddress,
                                                  PidProcessMap& processMap,
                                                  NameModuleMap& moduleMap,
                                                  PidModaddrItrMap* pidModaddrItrMap,
                                                  ModInstanceMap& modInstanceMap,
                                                  bool bDoCLU,
                                                  bool bLdStCollect,
                                                  UINT8 L1DcAssoc,
                                                  UINT8 L1DcLineSize,
                                                  UINT8 L1DcLinesPerTag,
                                                  UINT8 L1DcSize,
                                                  gtUByte* pCssBuffer,
                                                  PrdTranslationStats* pStats)
{
    GT_UNREFERENCED_PARAMETER(baseAddress);

    m_ThreadHR = S_OK;

    // Check if the global PrdReader object tPrdReader has mapped the file
    if (! mapAddress.isMapped())
    {
        OS_OUTPUT_DEBUG_LOG(L"PRD file is not memory mapped.", OS_DEBUG_LOG_ERROR);
        m_ThreadHR = E_ABORT;
        return m_ThreadHR;
    }

#if ENABLE_PRD_DEBUG_OUTPUT
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Thread(%d), totalBytes(%d), baseAddress(0x%x)",
                               GetCurrentThreadId(), totalBytes, baseAddress);
#endif

    DWORD startTime = 0L;

    if (!m_collectStat)
    {
        pStats = NULL;
    }

    pMissedInfo->missedCount = 0;

    TiModuleInfo modInfo;
    memset(&modInfo, 0, sizeof(TiModuleInfo));
    struct _stati64 fileStat;
    _wstati64(m_dataFile.toStdWString().c_str(), &fileStat);

    //if applicable, get the list of filtered processes
    QList<DWORD> filters;

    if (!processFilters.isEmpty())
    {
        //TODO: error checking
        unsigned int procCount = 0;
        fnGetProcessNum(&procCount);
        ProcQueryInfo* allProcArray = new ProcQueryInfo[procCount];
        fnGetAllProcesses(allProcArray, procCount);

        //TODO: verify for java
        for (unsigned int i = 0; i < procCount; i++)
        {
            if (processFilters.contains(QString::fromWCharArray(allProcArray[i].pqiProcessName),
                                        Qt::CaseInsensitive))
            {
                filters.append(allProcArray[i].pqiProcessID);
            }
        }

        delete [] allProcArray;

        if (filters.isEmpty())
        {
            m_ThreadHR = E_ABORT;
            return m_ThreadHR;
        }
    }

    system32Dir[0] = L'\0';

    // C:\windows\system32 or C:\winnt\system32
    GetSystemDirectoryW(system32Dir, OS_MAX_PATH);

    //try to get the cpu from the reader, in case it was before the config
    int cpuFamily = tPrdReader.GetCpuFamily();
    int cpuModel = tPrdReader.GetCpuModel();

    if (0 == cpuFamily)
    {
        osCpuid cpuid;
        cpuFamily = cpuid.getFamily();
        cpuModel = cpuid.getModel();
    }

    unsigned int cpuCount = tPrdReader.GetCoreCount();

    ProcessInfo* pProcessInfo = NULL;
    ProcessIdType processInfoId = 0;

    gtUInt64 iterBytes;
    gtUInt64 bytes = 0;

    while (bytes <= totalBytes)
    {
        iterBytes = bytes;
        bytes = threadPRDReader.GetBytesRead();
        *pByteRead  = threadBytesReadSoFar + bytes;
        HRESULT hr = E_FAIL;

        iterBytes = bytes - iterBytes;

        if (bMainThread)
        {
            AddBytesToProgressBar(iterBytes);
        }
        else
        {
            AsyncAddBytesToProgressBar(iterBytes);
        }

        RawPRDRecord tRawRec, tRawRec2;
        RecordDataStruct prdRecord;
        unsigned int recNum = 0;

        if (S_OK != threadPRDReader.GetNextRawRecords(&tRawRec, &tRawRec2, &recNum))
        {
            break;
        }

        switch (tRawRec.rawRecordsData[0])
        {
            case PROF_REC_CSS:
                break;

            case PROF_REC_USER_CSS:
            {
                PRD_USER_CSS_DATA_RECORD* pUserCssRec = reinterpret_cast<PRD_USER_CSS_DATA_RECORD*>(&tRawRec);
                bool is64Bit = FALSE != pUserCssRec->m_Is64Bit;
                unsigned int depth = pUserCssRec->m_Depth;

                unsigned int additionalRecordsCount = GetUserCallStackAdditionalRecordsCount(depth, is64Bit);
                threadPRDReader.SkipRawRecords(additionalRecordsCount);
            }
            break;

            case PROF_REC_KERNEL_CSS:
            {
                PRD_KERNEL_CSS_DATA_RECORD* pKernelCssRec = reinterpret_cast<PRD_KERNEL_CSS_DATA_RECORD*>(&tRawRec);
                bool is64Bit = FALSE != pKernelCssRec->m_Is64Bit;
                unsigned int depth = pKernelCssRec->m_Depth;

                unsigned int additionalRecordsCount = GetKernelCallStackAdditionalRecordsCount(depth, is64Bit);

                if (0U != additionalRecordsCount)
                {
                    threadPRDReader.SkipRawRecords(additionalRecordsCount);
                }
            }
            break;

            case PROF_REC_VIRTUAL_STACK:
            {
                PRD_VIRTUAL_STACK_RECORD* pVirtualStaclRec = reinterpret_cast<PRD_VIRTUAL_STACK_RECORD*>(&tRawRec);
                unsigned int valuesCount = pVirtualStaclRec->m_ValuesCount;

                unsigned int additionalRecordsCount = GetVirtualStackAdditionalRecordsCount(valuesCount);
                threadPRDReader.SkipRawRecords(additionalRecordsCount);
            }
            break;

            case PROF_REC_EVENT:
            case PROF_REC_TIMER:
            {
                threadPRDReader.ConvertSampleData(tRawRec, &prdRecord);

                if ((!filters.isEmpty()) && (!filters.contains(prdRecord.m_PID)))
                {
                    continue;
                }

                if (IsProfilingDriver(prdRecord.m_RIP))
                {
                    continue;
                }

                ProcessIdType processId = static_cast<ProcessIdType>(prdRecord.m_PID);
                ThreadIdType threadId = static_cast<ThreadIdType>(prdRecord.m_ThreadHandle);
                gtUInt64 timeStamp = prdRecord.m_DeltaTick;
                gtUInt64 sampleAddr = prdRecord.m_RIP;
                unsigned int core = prdRecord.m_ProcessorID;
                EventMaskType eventType = EncodeEvent(prdRecord.m_EventType,
                                                      prdRecord.m_EventUnitMask,
                                                      (prdRecord.m_eventBitMask & 2),
                                                      (prdRecord.m_eventBitMask & 1));

                //If we are saving the thread info,
                if (bThread)
                {
                    //ignore timer events and pretend the non local event is the first
                    //  event
                    if (NON_LOCAL_EVENT != prdRecord.m_EventType)
                    {
                        continue;
                    }
                }

                //keep track of the highest core sampled
                if (cpuCount <= core)
                {
                    cpuCount = (core + 1);
                }

                // We try to process the Kernel CSS here, to detect cases of call-stacks containing the CPU Profiling drivers,
                // For which we need to drop this sample.
                if (PROF_REC_KERNEL_CSS == threadPRDReader.PeekNextRecordType())
                {
                    GT_IF_WITH_ASSERT(S_OK == threadPRDReader.GetNextRawRecords(&tRawRec, &tRawRec2, &recNum))
                    {
                        HRESULT hrCss = TranslateKernelCallStack(*reinterpret_cast<PRD_KERNEL_CSS_DATA_RECORD*>(&tRawRec),
                                                                 threadPRDReader,
                                                                 pProcessInfo,
                                                                 processInfoId,
                                                                 filters,
                                                                 processId,
                                                                 threadId,
                                                                 timeStamp,
                                                                 eventType,
                                                                 core,
                                                                 pCssBuffer,
                                                                 pStats);

                        if (S_FALSE == hrCss)
                        {
                            continue;
                        }
                    }
                }

                BEGIN_TICK_COUNT();
                hr = GetModuleInfoHelper((void*) &prdRecord, &modInfo, evTBPEBPRecord, proFile);
                END_TICK_COUNT(findModuleInfo);

#ifdef AMDT_ENABLE_CPUPROF_DB
                if (modInstanceMap.end() == modInstanceMap.find(modInfo.instanceId))
                {
                    modInstanceMap.emplace(modInfo.instanceId, std::make_tuple(gtString(modInfo.pModulename), modInfo.processID, modInfo.ModuleStartAddr));
                }
#else
                GT_UNREFERENCED_PARAMETER(modInstanceMap);
#endif

                AggregateSampleData(prdRecord, &modInfo, &processMap, &moduleMap, pidModaddrItrMap, 1U, pStats);

                // We only try to finalize a User call-stack if the sampled instruction is in user space.
                if ((S_OK == hr) && !modInfo.kernel)
                {
                    // Ignore CSS data that is not in the filters (if applicable).
                    if (0 != processId && 4 != processId && 8 != processId)
                    {
                        BEGIN_TICK_COUNT();

                        if (processId != processInfoId)
                        {
                            processInfoId = processId;
                            pProcessInfo = &AcquireProcessInfo(processInfoId);
                        }

                        FinalizeUserCallStack(*pProcessInfo,
                                              modInfo.pPeFile,
                                              threadId,
                                              timeStamp,
                                              eventType,
                                              sampleAddr,
                                              pStats);
                        END_TICK_COUNT(analyzeCss);
                    }
                }
            }
            break;

            case PROF_REC_IBS_FETCH_BASIC:
            case PROF_REC_IBS_FETCH_EXT:
            {
                IBSFetchRecordData ibsFetch;

                if (S_OK == threadPRDReader.ConvertIBSFetchData(&tRawRec, &tRawRec2, &ibsFetch))
                {
                    if (IsProfilingDriver(ibsFetch.m_RIP))
                    {
                        continue;
                    }

                    if ((!filters.isEmpty()) && (!filters.contains(ibsFetch.m_PID)))
                    {
                        continue;
                    }

                    ProcessIdType processId = static_cast<ProcessIdType>(ibsFetch.m_PID);
                    ThreadIdType threadId = static_cast<ThreadIdType>(ibsFetch.m_ThreadHandle);
                    gtUInt64 timeStamp = ibsFetch.m_DeltaTick;
                    gtUInt64 sampleAddr = ibsFetch.m_RIP;
                    unsigned int core = ibsFetch.m_ProcessorID;

                    // Probably, we should process the next CSS record for all IBS_FETCH events enabled in
                    // this record; will do it later
                    EventMaskType eventType = DE_IBS_FETCH_ALL;

                    //keep track of the highest core sampled
                    if (cpuCount <= core)
                    {
                        cpuCount = (core + 1);
                    }

                    // We try to process the Kernel CSS here, to detect cases of call-stacks containing the CPU Profiling drivers,
                    // For which we need to drop this sample.
                    if (PROF_REC_KERNEL_CSS == threadPRDReader.PeekNextRecordType())
                    {
                        GT_IF_WITH_ASSERT(S_OK == threadPRDReader.GetNextRawRecords(&tRawRec, &tRawRec2, &recNum))
                        {
                            HRESULT hrCss = TranslateKernelCallStack(*reinterpret_cast<PRD_KERNEL_CSS_DATA_RECORD*>(&tRawRec),
                                                                     threadPRDReader,
                                                                     pProcessInfo,
                                                                     processInfoId,
                                                                     filters,
                                                                     processId,
                                                                     threadId,
                                                                     timeStamp,
                                                                     eventType,
                                                                     core,
                                                                     pCssBuffer,
                                                                     pStats);

                            if (S_FALSE == hrCss)
                            {
                                continue;
                            }
                        }
                    }

                    BEGIN_TICK_COUNT();
                    hr = GetModuleInfoHelper((void*)&ibsFetch, &modInfo, evIBSFetchRecord, proFile);
                    END_TICK_COUNT(findModuleInfo);

                    ProcessIbsFetchRecord(ibsFetch, &modInfo, &processMap, &moduleMap, pidModaddrItrMap, pStats);

                    // We only try to finalize a User call-stack if the sampled instruction is in user space.
                    if ((S_OK == hr) && !modInfo.kernel)
                    {
                        // Ignore CSS data that is not in the filters (if applicable).
                        if (0 != processId && 4 != processId && 8 != processId)
                        {
                            BEGIN_TICK_COUNT();

                            if (processId != processInfoId)
                            {
                                processInfoId = processId;
                                pProcessInfo = &AcquireProcessInfo(processInfoId);
                            }

                            FinalizeUserCallStack(*pProcessInfo,
                                                  modInfo.pPeFile,
                                                  threadId,
                                                  timeStamp,
                                                  eventType,
                                                  sampleAddr,
                                                  pStats);
                            END_TICK_COUNT(analyzeCss);
                        }
                    }
                }

            }
            break;

            case PROF_REC_IBS_OP_BASIC:
            case PROF_REC_IBS_OP_EXT:
            {
                IBSOpRecordData ibsOp;

                if (S_OK == threadPRDReader.ConvertIBSOpData(&tRawRec, &tRawRec2, &ibsOp))
                {
                    if (IsProfilingDriver(ibsOp.m_RIP))
                    {
                        continue;
                    }

                    if ((!filters.isEmpty()) && (!filters.contains(ibsOp.m_PID)))
                    {
                        continue;
                    }

                    ProcessIdType processId = static_cast<ProcessIdType>(ibsOp.m_PID);
                    ThreadIdType threadId = static_cast<ThreadIdType>(ibsOp.m_ThreadHandle);
                    gtUInt64 timeStamp = ibsOp.m_DeltaTick;
                    gtUInt64 sampleAddr = ibsOp.m_RIP;
                    unsigned int core = ibsOp.m_ProcessorID;

                    // Probably, we should process the next CSS record for all IBS_OP events enabled in
                    // this record; will do it later
                    EventMaskType eventType = DE_IBS_OP_ALL;

                    //keep track of the highest core sampled
                    if (cpuCount <= core)
                    {
                        cpuCount = (core + 1);
                    }

                    // We try to process the Kernel CSS here, to detect cases of call-stacks containing the CPU Profiling drivers,
                    // For which we need to drop this sample.
                    if (PROF_REC_KERNEL_CSS == threadPRDReader.PeekNextRecordType())
                    {
                        GT_IF_WITH_ASSERT(S_OK == threadPRDReader.GetNextRawRecords(&tRawRec, &tRawRec2, &recNum))
                        {
                            HRESULT hrCss = TranslateKernelCallStack(*reinterpret_cast<PRD_KERNEL_CSS_DATA_RECORD*>(&tRawRec),
                                                                     threadPRDReader,
                                                                     pProcessInfo,
                                                                     processInfoId,
                                                                     filters,
                                                                     processId,
                                                                     threadId,
                                                                     timeStamp,
                                                                     eventType,
                                                                     core,
                                                                     pCssBuffer,
                                                                     pStats);

                            if (S_FALSE == hrCss)
                            {
                                continue;
                            }
                        }
                    }

                    BEGIN_TICK_COUNT();
                    hr = GetModuleInfoHelper((void*)&ibsOp, &modInfo, evIBSOpRecord, proFile);
                    END_TICK_COUNT(findModuleInfo);

                    ProcessIbsOpRecord(ibsOp, &modInfo, &processMap,
                                       &moduleMap, pidModaddrItrMap,
                                       bDoCLU, bLdStCollect,
                                       L1DcAssoc, L1DcLineSize,
                                       L1DcLinesPerTag, L1DcSize,
                                       pStats);

                    // We only try to finalize a User call-stack if the sampled instruction is in user space.
                    if ((S_OK == hr) && !modInfo.kernel)
                    {
                        // Ignore CSS data that is not in the filters (if applicable) or is in the System process
                        if (0 != processId && 4 != processId && 8 != processId)
                        {
                            BEGIN_TICK_COUNT();

                            if (processId != processInfoId)
                            {
                                processInfoId = processId;
                                pProcessInfo = &AcquireProcessInfo(processInfoId);
                            }

                            // Process CSS data for IBS sample only when profiling IBS Op
                            // ignoring for CLU Profile
                            if (m_runInfo->m_isProfilingIbsOp)
                            {
                                FinalizeUserCallStack(*pProcessInfo,
                                                      modInfo.pPeFile,
                                                      threadId,
                                                      timeStamp,
                                                      eventType,
                                                      sampleAddr,
                                                      pStats);
                            }

                            END_TICK_COUNT(analyzeCss);
                        }
                    }
                }
            }
            break;

            default:
                // Continue to process next record
                break;
        }
    } //while reading data from PRD file

    threadBytesReadSoFar += bytes;


    m_processInfosLock.lockRead();

    for (gtMap<ProcessIdType, ProcessInfo*>::const_iterator it = m_processInfos.begin(), itEnd = m_processInfos.end(); it != itEnd; ++it)
    {
        PidProcessMap::iterator p_it = processMap.find(it->first);

        if (processMap.end() != p_it)
        {
            p_it->second.m_hasCss = true;
        }
    }

    m_processInfosLock.unlockRead();


    // Writing the processed profile data into .tbp/.ebp/.imd files should be
    // done after processing all the records. This is done by the main thread

    return m_ThreadHR;
}


// worker thread start point;
//
static void workerPRDReaderThread(void* pData, void* workData)
{
    if (!pData)
    {
        return;
    }

    if (!workData)
    {
        return;
    }

    // Input Parameters
    ThreadPrdData* pThreadData = reinterpret_cast<ThreadPrdData*>(pData);
    PrdWorkUnit* work = reinterpret_cast<PrdWorkUnit*>(workData);

    // Update the thread specific PrdReaderThread object with baseAddress, etc.
    gtUInt64 startTick = (pThreadData->pPrdReader)->GetStartTick();
    gtUInt64 endTick = (pThreadData->pPrdReader)->GetEndTick();

    (pThreadData->threadPrdReader)->SetPRDBufferValues(
        work->baseAddress,
        work->numberRecords,
        startTick,
        endTick);


#if ENABLE_PRD_DEBUG_OUTPUT
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"baseAddress(0x%llx), totalBytes(%llu), total records(%u).",
                               work->baseAddress, work->totalBytes, work->numberRecords);
#endif

    pThreadData->pCDXptr->ThreadTranslateDataPrdFile(
        pThreadData->sessionPath,
        pThreadData->pMissedInfo,
        *(pThreadData->pProcessFilters),
        *(pThreadData->pMapAddress),
        *(pThreadData->pPrdReader),
        *(pThreadData->threadPrdReader),
        pThreadData->bThread,
        pThreadData->bMainThread,
        pThreadData->pBytesRead,
        work->totalBytes,
        work->baseAddress,
        *(pThreadData->processMap),
        *(pThreadData->moduleMap),
        pThreadData->pidModaddrItrMap,
        *(pThreadData->modInstanceMap),
        pThreadData->bCLUtil,
        pThreadData->bLdStCollect,
        pThreadData->L1DcAssoc,
        pThreadData->L1DcLineSize,
        pThreadData->L1DcLinesPerTag,
        pThreadData->L1DcSize,
        pThreadData->pCssBuffer,
        &pThreadData->stats);
}


// CreatePRDView
//
//  Create file map view for the given file offset and length. Also find the first weight record
//  in the mapped file.
HRESULT PrdTranslator::CreatePRDView(PrdReader& tPrdReader,
                                     gtUInt64 offset,
                                     gtUInt32 length,
                                     MemoryMap& mapAddress,
                                     gtUInt32* pfirstWeightRecOffset)
{
    HRESULT res;
    gtUInt32  firstWeightRecOffset = 0;

    res = mapAddress.CreateView(offset, length);

    if (S_OK == res)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"mapAddress(0x%p)", mapAddress.GetMappedAddress());

        res = tPrdReader.GetBufferWeightRecOffset(mapAddress.GetMappedAddress(), length, &firstWeightRecOffset);

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"GetBufferWeightRecOffset returns(%x), firstSampleRecOffset(%u)",
                                   res, firstWeightRecOffset);
    }

    *pfirstWeightRecOffset = firstWeightRecOffset;

    return res;
}


// GetBufferRecordCount
//
// Get the number of records in the buffer
HRESULT PrdTranslator::GetBufferRecordCount(PrdReader& tPrdReader, LPVOID baseAddress, gtUInt32* pRecType, gtUInt32* pCnt)
{
    HRESULT res = S_OK;
    RawPRDRecord pRawRec;
    gtUInt32 cnt = 0;

    int ret = tPrdReader.ReadMappedRecord(baseAddress, &pRawRec);

    if (CAPRDRECORDSIZE != ret)
    {
        return E_FAIL;
    }

#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Record type : %d", pRawRec.rawRecordsData[0]);
#endif

    switch (pRawRec.rawRecordsData[0])
    {
        case PROF_REC_WEIGHT:
            if (tPrdReader.GetProfileVersion() >= CXL_HDR_VERSION)
            {
                PRD_WEIGHT_RECORD* pW = (PRD_WEIGHT_RECORD*) pRawRec.rawRecordsData;
                cnt = static_cast<gtUInt32>(pW->m_BufferRecordCount);
            }
            else
            {
                sTrcCaWeightRecord* pW = (sTrcCaWeightRecord*) pRawRec.rawRecordsData;
                cnt = static_cast<gtUInt32>(pW->m_BufferRecordCount);
            }

#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Found WEIGHT record... cnt(%d)", cnt);
#endif
            break;

        case PROF_REC_MISSED:
            // These missed records will only be written at the end ?
            // Missed Sample counts record.
            // cnt = (totalMapViewBytes - mapViewBytesRead) / CAPRDRECORDSIZE;
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Found MISSED record... cnt(%d)", cnt);
            break;

        default:
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Found invalid record(%d)", pRawRec.rawRecordsData[0]);
            res = E_FAIL;
            break;
    }

    *pCnt = cnt;
    *pRecType = pRawRec.rawRecordsData[0];

    return res;
}


//S_OK - everything worked ok
//E_UNEXPECTED - Could not write the new file
//E_INVALIDARG - pMissedInfo was not passed in
//E_ACCESSDENIED - Could not read raw data file
//E_ABORT - No samples for the process filters provided
//Writes the data to the file in proFile,
HRESULT PrdTranslator::TranslateDataPrdFile(QString proFile,
                                            MissedInfoType* pMissedInfo,
                                            QStringList processFilters,
                                            QWidget* pApp,
                                            bool bThread,
                                            bool bCLUtil,
                                            bool bLdStCollect)
{
    HRESULT     res;
    m_ThreadHR = S_OK;
    DWORD       startTime;

    fnSetExecutableFilesSearchPath(m_pSearchPath, m_pServerList, m_pCachePath);

    pMissedInfo->missedCount = 0;
    //if (INVALID_HANDLE_VALUE == m_ThreadCompleteEvent) {
    //  m_ThreadCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    //  if (INVALID_HANDLE_VALUE == m_ThreadCompleteEvent)
    //      return E_ABORT;
    //}

    struct _stati64 fileStat;
    _wstati64(m_dataFile.toStdWString().c_str(), &fileStat);


    startTime = GetTickCount();
    fnReadCLRJitInformation(proFile.section("\\", 0, -2).toStdWString().c_str());

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Processing CLR JIT (%d ms)", (GetTickCount() - startTime));
    }

    UpdateProgressBar(40ULL, 100ULL);

    startTime = GetTickCount();
    fnReadJitInformation(proFile.section("\\", 0, -2).toStdWString().c_str());

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Processing JIT (%d ms)", (GetTickCount() - startTime));
    }

    UpdateProgressBar(50ULL, 100ULL);

    // Measure the time taken to process the PRD records..
    startTime = GetTickCount();

    if (m_collectStat)
    {
        PrintMemoryUsage(L"Memory Usage: Before processing PRD records.");
    }

    PrdReader tPrdReader;
    MemoryMap mapAddress;
    gtUInt64 lastUserCssRecordOffset = 0ULL;

    if (!InitPrdReader(&tPrdReader, m_dataFile.toStdWString().c_str(), &lastUserCssRecordOffset, pApp))
    {
        return E_ACCESSDENIED;
    }

    UpdateProgressBar(60ULL, 100ULL);

    if (static_cast<gtInt64>(lastUserCssRecordOffset * PRD_RECORD_SIZE) >= fileStat.st_size)
    {
        return E_FAIL;
    }

    m_durationSec = tPrdReader.GetProfileDuration();

    // Memory map the file..
    res = mapAddress.Map(m_dataFile.toStdWString().c_str());

    if (S_OK != res)
    {
        return E_ACCESSDENIED; // Well, i should add a new error code, and handle it all the respective places..
    }

    UpdateProgressBar(70ULL, 100ULL);

    //
    // Create a thread pool to process the PRD records...
    //
    unsigned long   nbrThreads = 1;
    bool            oneChunk = false;
    DWORD           FileAllocGranularity = 0;

    // Check if the file size is greater than CXL_PRD_MIN_SIZE_FOR_THREADPOOL, if so
    // create the threadpool; The number of threads should be equal to the number of
    // processors available in the system.
    //
    // The PRD file generated by CA3.2 and earlier does not have the BufferCount in
    // Weight Record. Hence we cannot process those PRD files in threaded manner.
    // Hence process them in one single chunk. One potential issue here is, we
    // memory map the entire file. If the PRD generated by CA3.2 is too huge (>1GB),
    // memory map will fail.
    if (CXL_PRD_MIN_SIZE_FOR_THREADPOOL >= fileStat.st_size || !tPrdReader.isWeightRecHasBufferCount() || m_runInfo->m_isProfilingClu)
    {
        nbrThreads = 1;
        // oneChunk = true;
    }
    else
    {
        // If the user has specified the number of worker threads to be created for processing
        // the PRD records, create as many threads.
        if (m_numWorkerThreads)
        {
            nbrThreads = m_numWorkerThreads;
        }
        else
        {
            GetSysInfo(&nbrThreads, &FileAllocGranularity);

            // Baskar:
            //
            // Due to the huge memory footprint issues, currently I am setting the number
            // of threads to 2 (irrespective of number of cores available)
            //
            // The internal data structures used for data translation CpuProfileProcess and
            // CpuProfileModule maps are HiGhLy MeMoRy CoNsUmInG (particularly for IBS profile runs).
            // Having them per-thread is making the problem much more worser. Hence till
            // we refactor those data structures, we will just create only 2 worker threads..
            //
            // How does this affect MT translation ?
            //  Lets say there are 10000 unique (address/pid/tid) samples. If these samples
            //  are hot, they will be spread across multiple PRD buffers. These buffers
            //  will be processed by multiple threads and each thread has its own CA_module
            //  object. Eventually, even if 3000 unique samples are seen by just 2 threads,
            //  the memory consumption will approximately be increased by 30% percent.
            //
            // Refactoring of these data structures itself should be separate task.
            //
            gtUInt64 freeVM = 0;
            GetFreeVM(freeVM);

            if (freeVM < CXL_3GB_VM)
            {
                nbrThreads = (nbrThreads > 1) ? 2 : 1;
            }
        }
    }

    // If the nbrThreads is 1 and the PRD file size less than the mapping granularity (125MB)
    // process them in one chunk; No need to create multiple work units..
    if (((1 == nbrThreads) && (fileStat.st_size < CXL_PRD_MAPVIEW_GRANULARITY))
        || (false == tPrdReader.isWeightRecHasBufferCount()))
    {
        oneChunk = true;
    }


    ThreadPool* workerTP = NULL;

    if (!oneChunk)
    {
        workerTP = new ThreadPool(nbrThreads);

        if (NULL == workerTP)
        {
            oneChunk = true;
            nbrThreads = 1;
        }
    }

    ThreadParamList plist;
    PidProcessList procList;
    NameModuleList modList;
    ModInstanceList modInstanceList;

    gtUInt64 totalBytes;
    gtUInt64* bytesRead;

    bytesRead = (gtUInt64*)malloc(sizeof(gtUInt64) * nbrThreads);

    memset((void*)bytesRead, 0, sizeof(gtUInt64) * nbrThreads);

    UpdateProgressBar(80ULL, 100ULL);

    m_progressStride = (20 * 128) / nbrThreads;

    // Create the thread parameters for the worker threads
    for (unsigned int i = 0; i < nbrThreads; i++)
    {
        ThreadPrdData* pThreadData = new ThreadPrdData;


        pThreadData->bThread           = bThread;
        pThreadData->bMainThread       = false;
        pThreadData->pBytesRead        = &bytesRead[i];
        pThreadData->pMissedInfo       = pMissedInfo;
        pThreadData->pMapAddress       = &mapAddress;
        pThreadData->pPrdReader        = &tPrdReader;
        pThreadData->pProcessFilters   = &processFilters;
        pThreadData->sessionPath       = proFile;
        pThreadData->pCDXptr           = this;
        pThreadData->bCLUtil           = bCLUtil;
        pThreadData->bLdStCollect      = bLdStCollect;
        pThreadData->L1DcSize          = tPrdReader.GetL1DcSize();
        pThreadData->L1DcAssoc         = tPrdReader.GetL1DcAssoc();
        pThreadData->L1DcLineSize      = tPrdReader.GetL1DcLineSize();
        pThreadData->L1DcLinesPerTag   = tPrdReader.GetL1DcLinesPerTag();

        pThreadData->pCssBuffer = new gtUByte[CallStackBuilder::CalcRequiredBufferSize(MAX_CSS_VALUES)];

        PrdReaderThread*  threadPRDReader  = new PrdReaderThread(tPrdReader);
        PidProcessMap*    processMap       = new PidProcessMap;
        NameModuleMap*    moduleMap        = new NameModuleMap;
        PidModaddrItrMap* pidModaddrItrMap = new PidModaddrItrMap;
        ModInstanceMap*   modInstanceMap   = new ModInstanceMap;

        pThreadData->threadPrdReader     = threadPRDReader;
        pThreadData->processMap          = processMap;
        pThreadData->moduleMap           = moduleMap;
        pThreadData->pidModaddrItrMap    = pidModaddrItrMap;
        pThreadData->modInstanceMap      = modInstanceMap;

        // Put the processMap and moduleMap in the respective lists.
        procList.push_back(processMap);
        modList.push_back(moduleMap);
        plist.push_back(pThreadData);
        modInstanceList.push_back(modInstanceMap);

        UpdateProgressBar(80ULL + static_cast<gtUInt64>(((i + 1) * static_cast<unsigned int>(m_progressStride)) / 128), 100ULL);
    }

    totalBytes = fileStat.st_size - tPrdReader.GetFirstWeightRecOffset();
    QProgressDialog* pProgDlg = NULL;

    if (NULL != pApp)
    {
        // Show the PRD processing bar
        pProgDlg = new QProgressDialog("Processing Raw sample Data...", QString::null,
                                       0, (totalBytes), pApp, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

        pProgDlg->setModal(true);
        pProgDlg->show();
    }

    if (NULL != m_pProfilingDrivers)
    {
        delete [] m_pProfilingDrivers;
        m_pProfilingDrivers = NULL;
    }

    m_countProfilingDrivers = 0U;
    fnGetCpuProfilingDriversMaxCount(&m_countProfilingDrivers);

    if (0U != m_countProfilingDrivers)
    {
        m_pProfilingDrivers = new KeModQueryInfo[m_countProfilingDrivers];
        fnGetCpuProfilingDrivers(m_pProfilingDrivers, m_countProfilingDrivers);
    }

    CompleteProgressBar();

    PrdTranslationStats globalStats;

    if (0ULL != lastUserCssRecordOffset)
    {
        const wchar_t CPU_PROFILING_STR_TranslatingRawCSSData[] = L"Translating raw CSS data...";
        m_useProgressSyncObject = false;
        m_progressAsync = 0;
        m_progressStride = (lastUserCssRecordOffset + 99ULL) / 100ULL;
        m_progressThreshold = m_progressStride;
        InitializeProgressBar(CPU_PROFILING_STR_TranslatingRawCSSData, true);

        PrdTranslationStats* pStats = (m_collectStat) ? &globalStats : NULL;

        BEGIN_TICK_COUNT();
        PrdUserCssRcuHandlerPool* pUserCssHandlerPool = new PrdUserCssRcuHandlerPool(*this,
                tPrdReader,
                mapAddress,
                fileStat.st_size,
                lastUserCssRecordOffset);
        RcuScheduler userCssScheduler(*pUserCssHandlerPool, USER_CSS_RCU_DATA_SIZE);
        userCssScheduler.Start(false);

        if (NULL != m_pfnProgressBarCallback)
        {
            unsigned int waitMilliseconds = 0U;
            m_progressEvent.setValue(0);

            while (-1 == userCssScheduler.WaitForCompletion(waitMilliseconds))
            {
                if (m_useProgressSyncObject)
                {
                    m_progressSyncObject.lock();
                }
                else
                {
                    //waitMilliseconds = INFINITE;
                    osSleep(250);
                }

                if (!pUserCssHandlerPool->GetStatusesQueue().isEmpty())
                {
                    // Get updates to the status bar caption that the UserCssRcuHandler threads pushed into the queue
                    gtQueue<gtString> statusesQueue;
                    pUserCssHandlerPool->GetStatusesQueue().popAll(statusesQueue);

                    // If any updates were pushed into the queue
                    if (!statusesQueue.empty())
                    {
                        // Update the status bar with the last caption pushed onto the queue
                        m_progressEvent.setProgress(statusesQueue.back());
                        m_pfnProgressBarCallback(m_progressEvent);
                    }
                }

                gtInt32 progress = AtomicSwap(m_progressAsync, 0);

                if (0 != progress)
                {
                    m_progressEvent.setValue(progress);
                    m_pfnProgressBarCallback(m_progressEvent);
                }
            }
        }
        else
        {
            userCssScheduler.WaitForCompletion();
        }

        mapAddress.DestroyView();

        END_TICK_COUNT(analyzeUserCss);
        m_progressEvent.setProgress(CPU_PROFILING_STR_TranslatingRawCSSData);
        CompleteProgressBar();
    }

    m_progressAsync = 0;
    m_progressStride = (static_cast<gtUInt64>(fileStat.st_size) + 99ULL) / 100ULL;
    m_progressThreshold = m_progressStride;
    InitializeProgressBar(L"Translating raw profile data...", true);

    PrdWorkUnit prdwork;
    unsigned int cnt = 0U;
    gtUInt64 bufSize = 0ULL;
    gtUInt32 firstWeightRecOffset;

    LPVOID baseAddress;

    // If the PRD file size < CXL_PRD_MIN_SIZE_FOR_THREADPOOL (20MB ?), do not create any thread pool.
    if (oneChunk)
    {
        res = CreatePRDView(tPrdReader, 0, fileStat.st_size, mapAddress, &firstWeightRecOffset);

        if (S_OK != res)
        {
            OS_OUTPUT_DEBUG_LOG(L"Could not create a mapped file view", OS_DEBUG_LOG_ERROR);
            return res;
        }

        baseAddress = static_cast<gtUByte*>(mapAddress.GetMappedAddress()) + firstWeightRecOffset;
        cnt = (fileStat.st_size - firstWeightRecOffset) / CAPRDRECORDSIZE;

#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"baseAddress(0x%p), firstWeightRecOffset(%u), Number of Records(%u)",
                                   baseAddress, firstWeightRecOffset, cnt);
#endif

        // Create the workunit...
        bufSize                 = cnt * CAPRDRECORDSIZE;
        prdwork.baseAddress     = (gtUInt64)baseAddress;
        prdwork.totalBytes      = bufSize;
        prdwork.numberRecords   = cnt;

        ThreadPrdData* pThreadData = static_cast<ThreadPrdData*>(plist.front());
        pThreadData->bMainThread = true;
        workerPRDReaderThread(pThreadData, &prdwork);
    }
    else
    {
        // Initialize the ThreadPool with the thread-work-function and the threadparams
        workerTP->InitThreadPool((THREAD_WORK_ROUTINE)workerPRDReaderThread, plist);

        //
        // Here we handle multiple work chunks...
        //
        gtUInt64 mapviewGranularity  = CXL_PRD_MAPVIEW_GRANULARITY;
        gtUInt64 mapviewSize         = CXL_PRD_MAPVIEW_SIZE;
        gtUInt64 bytesProcessedSoFar = 0;

        // create mapviews and process them
        while (true)
        {
            bool     endOfMap = false;
            gtUInt32 length;

            // Calculate the length of the file-map-view
            gtUInt64 bytesToBeProcessed = fileStat.st_size - bytesProcessedSoFar;

            // Create a View and get the offset of the first weight record...
            while (true)
            {
                length = (bytesToBeProcessed > mapviewSize) ? mapviewSize : bytesToBeProcessed;
                res = CreatePRDView(tPrdReader, bytesProcessedSoFar, length, mapAddress, &firstWeightRecOffset);

                if (S_OK == res)
                {
                    break;
                }

                // If we did not have enough memory and the granularity is bigger than 2MB.
                if (E_OUTOFMEMORY == res && (2 * 1024 * 1024) <= mapviewGranularity)
                {
                    // Retry with 1MB less.
                    mapviewGranularity -= 1024 * 1024;
                    mapviewGranularity = gtAlignDown(mapviewGranularity, FileAllocGranularity);

                    // Rounded down to a multiplication of PRD_RECORD_SIZE.
                    while (0 != (mapviewGranularity % PRD_RECORD_SIZE))
                    {
                        mapviewGranularity -= FileAllocGranularity;
                    }

                    if (0 == mapviewGranularity)
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Could not create a mapped file view", OS_DEBUG_LOG_ERROR);
                        return res;
                    }

                    mapviewSize = mapviewGranularity + 4096;
                }
                else
                {
                    OS_OUTPUT_DEBUG_LOG(L"Could not create a mapped file view", OS_DEBUG_LOG_ERROR);
                    return res;
                }
            }

            baseAddress = static_cast<gtUByte*>(mapAddress.GetMappedAddress()) + firstWeightRecOffset;

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"baseAddress(0x%p)", baseAddress);
#endif

            gtUInt64  totalMapViewBytes = length;
            gtUInt64  mapViewBytesRead = firstWeightRecOffset;

            while (! endOfMap)
            {
                gtUInt32  recType;
                res = GetBufferRecordCount(tPrdReader, baseAddress, &recType, &cnt);

                if (S_OK != res)
                {
                    // error.. invalid record
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid PRD record type (%d) while processing...", recType);
                    endOfMap = true;
                    break;
                }

                // we need to figure out the number of missed records - PROF_REC_MISSED
                if (PROF_REC_MISSED == recType)
                {
                    cnt = (totalMapViewBytes - mapViewBytesRead) / PRD_RECORD_SIZE;
                }

                // Create the workunit...
                bufSize                 = cnt * PRD_RECORD_SIZE;
                prdwork.baseAddress     = (gtUInt64)baseAddress;
                prdwork.totalBytes      = bufSize;
                prdwork.numberRecords   = cnt;
                // prdwork.status           = 0;

                // Assign the workunit to the threadpool
                workerTP->AssignWork(&prdwork);

                mapViewBytesRead += bufSize;

                if (mapViewBytesRead >= totalMapViewBytes || mapViewBytesRead >= mapviewGranularity)
                {
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"End of loop.. mapViewBytesRead(%llu) totalMapViewBytes(%llu)",
                                               mapViewBytesRead, totalMapViewBytes);
#endif
                    endOfMap = true;
                }
                else
                {
                    baseAddress = (LPVOID)((gtUInt64)baseAddress + bufSize);
                }

                if (NULL != m_pfnProgressBarCallback)
                {
                    gtInt32 progress = AtomicSwap(m_progressAsync, 0);

                    if (0 != progress)
                    {
                        m_progressEvent.setValue(progress);
                        m_pfnProgressBarCallback(m_progressEvent);
                    }
                }

            } // while (! endOfMap)


            //
            // Now..
            //  - Wait for the jobs to finish
            //  - unmap the view..
            // don't do this for last file map view...
            //

            bytesProcessedSoFar += mapviewGranularity;

            if (bytesProcessedSoFar >= static_cast<gtUInt64>(fileStat.st_size))
            {
                break;
            }

            HRESULT waitHr;

            do
            {
                //Update 4 times a second
                waitHr = workerTP->WaitForWorkCompletion(250);

                if (NULL != m_pfnProgressBarCallback)
                {
                    gtInt32 progress = AtomicSwap(m_progressAsync, 0);

                    if (0 != progress)
                    {
                        m_progressEvent.setValue(progress);
                        m_pfnProgressBarCallback(m_progressEvent);
                    }
                }
            }
            while (E_TIMEOUT == waitHr);

            mapAddress.DestroyView();
        }
    } // multiple work chunks

    while (1)
    {
        bool bComplete = true;

        if (NULL != workerTP)
        {
            //Update 4 times a second
            HRESULT status = workerTP->DestroyThreadPool(250);

            switch (status)
            {
                case S_OK:
                    // thread terminates
                    break;

                case E_FAIL:
                    // We can't do anything here.. just bail out..
                    OS_OUTPUT_DEBUG_LOG(L"Failed to DestroyThreadPool", OS_DEBUG_LOG_ERROR);
                    break;

                case E_TIMEOUT:

                //Fall through, update percentage and keep waiting
                default:
                    bComplete = false;
                    break;
            }

            if (NULL != m_pfnProgressBarCallback)
            {
                gtInt32 progress = AtomicSwap(m_progressAsync, 0);

                if (0 != progress)
                {
                    m_progressEvent.setValue(progress);
                    m_pfnProgressBarCallback(m_progressEvent);
                }
            }
        }

        if (bComplete)
        {
            CompleteProgressBar();
            break;
        }
    }

    mapAddress.UnMap();

    // make sure the progress bar is complete
    if (NULL != pApp)
    {
        pProgDlg->setValue(totalBytes);
    }
    else
    {
        printf("\rProfile analysis complete.\n");
    }

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Processing PRD records (%d ms)", (GetTickCount() - startTime));
        PrintMemoryUsage(L"Memory Usage: After processing PRD records.");
    }

    bool hasCluData = (m_runInfo->m_isProfilingClu && (NULL != m_pCluInfo));

    InitializeProgressBar(L"Writing translated profile data...", false);

    // Aggregate all the processmaps and module maps
    startTime = GetTickCount();
    AggregateThreadMaps(procList, modList, modInstanceList);

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Aggregate CpuProfileProcess and CpuProfileModule Maps: (%d ms)",
                                   (GetTickCount() - startTime));
        PrintMemoryUsage(L"Memory Usage: After Aggregating CpuProfileProcess and CpuProfileModule maps.");
    }

    UpdateProgressBar((hasCluData ? 15ULL : 30ULL), 100ULL);

    PidProcessMap*  procMap = *(PidProcessList::iterator)procList.begin();
    NameModuleMap*  modMap = *(NameModuleList::iterator)modList.begin();
    ModInstanceMap* modInstanceMap = *(modInstanceList.begin());

    if (hasCluData)
    {
        m_pCluInfo->CacheLineCleanup();

        // Aggregate and populate the CLU data
        AggregateCluData(procMap, modMap, proFile, (m_collectStat ? &globalStats : NULL));

        UpdateProgressBar(30ULL, 100ULL);
    }

    // Now that we have processes all the PRD...
    // write into files...
    startTime = GetTickCount();

    m_ThreadHR = WriteProfile(proFile,
                              tPrdReader,
                              pMissedInfo,
                              *procMap,
                              *modMap,
                              *modInstanceMap);

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Write profile files: (%d ms)", (GetTickCount() - startTime));
    }

    // Store pid filter list before closing the reader
    m_pidFilterList = tPrdReader.GetPidFilterList();

    // Clean up..
    tPrdReader.Close();

    // Delete the workers thread pool
    if (NULL != workerTP)
    {
        delete workerTP;
    }

    for (PidProcessList::iterator i = procList.begin(); i != procList.end(); i++)
    {
        delete *i;
    }

    procList.clear();

    for (NameModuleList::iterator i = modList.begin(); i != modList.end(); i++)
    {
        delete *i;
    }

    modList.clear();

    for (ThreadParamList::iterator i = plist.begin(); i != plist.end(); i++)
    {
        ThreadPrdData* pThreadData = reinterpret_cast<ThreadPrdData*>(*i);

        if (m_collectStat)
        {
            globalStats += pThreadData->stats;
        }

        delete pThreadData->threadPrdReader;
        delete pThreadData->pidModaddrItrMap;

        delete [] pThreadData->pCssBuffer;

        delete pThreadData;
    }

    CompleteProgressBar();

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Find module info (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::findModuleInfo]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Add Working Set module (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::addWorkingSetModule]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Query Symbol Engine (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::querySymbolEngine]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Analyze CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::analyzeCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Analyze User CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::analyzeUserCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Build CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::buildCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Traverse CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::traverseCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Finalize Kernel CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::finalizeKernelCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Finalize User CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::finalizeUserCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Finalize Partial User CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::finalizePartialUserCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Disassemble (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::disassemble]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Map memory for User CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::mapMemoryUserCss]);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: Unmap memory for User CSS (%u ms, count: %u)", globalStats.m_values[PrdTranslationStats::unmapMemoryUserCss]);
    }

    if (m_collectStat)
    {
        PrintMemoryUsage(L"Memory Usage: After destroying CpuProfileProcess and CpuProfileModule objects.");
    }

    if (NULL != pProgDlg)
    {
        delete pProgDlg;
    }

    if (NULL != m_pProfilingDrivers)
    {
        delete [] m_pProfilingDrivers;
        m_pProfilingDrivers = NULL;
        m_countProfilingDrivers = 0U;
    }

    return m_ThreadHR;
}


bool PrdTranslator::IsProfilingDriver(gtVAddr va) const
{
    bool ret = false;

    for (unsigned i = 0U; i < m_countProfilingDrivers; ++i)
    {
        if (m_pProfilingDrivers[i].keModStartAddr <= va && va < m_pProfilingDrivers[i].keModEndAddr)
        {
            ret = true;
            break;
        }
    }

    return ret;
}


// Aggregate all the processmaps and module maps
//
// The aggregated values will be in the first list entry..
//
bool PrdTranslator::AggregateThreadMaps(PidProcessList& procList, NameModuleList& modList, ModInstanceList& modInstanceList)
{
    PidProcessMap*   procMap;
    NameModuleMap*   modMap;
    ModInstanceMap*  modInstanceMap;

#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Thread(%d) got the work from workqueue", GetCurrentThreadId());
#endif

    // Everything will get merged with the first map entry
    PidProcessList::iterator piter = procList.begin();
    procMap = *piter;

    for (++piter; piter != procList.end();)
    {
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
        OS_OUTPUT_DEBUG_LOG(L"processing PidProcessMap entries...", OS_DEBUG_LOG_EXTENSIVE);
#endif

        PidProcessMap* pMap = *piter;
        // Iterate over the pids in pMap
        // Check if the pid is in procMap
        // if so,
        //      update the pid entry in procMap
        // else
        //      insert a  new pid entry in procMap

        PidProcessMap::iterator it = pMap->begin();
        PidProcessMap::iterator it_end = pMap->end();

        for (; it != it_end;)
        {
            PidProcessMap::iterator p_it = procMap->find(it->first);

            if (procMap->end() == p_it)
            {
                // Not found
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"inserting pid(%d)...", it->first);
#endif
                procMap->insert(PidProcessMap::value_type(it->first, it->second));
            }
            else
            {
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Pid(%d) is already in procMap", it->first);
#endif
                // Update this into procMap
                // CpuProfileProcess *cap = &(*it).second;
                p_it->second.addSamples((const AggregatedSample*)(&it->second));
            }

            // Clear the CpuProfileprocess - this will clear the AggregatedSample::m_sampleMap (CpuProfileSampleMap)
            it->second.clear();
            it = pMap->erase(it);
        }

        piter = procList.erase(piter);
        delete pMap;
    }

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    OS_OUTPUT_DEBUG_LOG(L"processing PidProcessMap done...", OS_DEBUG_LOG_EXTENSIVE);

    OS_OUTPUT_DEBUG_LOG(L"processing NAmeModuleMap...", OS_DEBUG_LOG_EXTENSIVE);
#endif

    // Everything will get merged with the first map entry
    NameModuleList::iterator miter = modList.begin();
    modMap = *miter;

    for (++miter; miter != modList.end();)
    {
        NameModuleMap* mMap = *miter;

        NameModuleMap::iterator mit = mMap->begin(), mit_end = mMap->end();

        while (mit != mit_end)
        {
            NameModuleMap::iterator m_it = modMap->find(mit->first);

            if (modMap->end() == m_it)
            {
                // Not Found..
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"inserting module.. (%ls)..", mit->first.asCharArray());
#endif
                modMap->insert(NameModuleMap::value_type(mit->first, mit->second));
            }
            else
            {
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Found Module(%ls): in modMap", mit->first.asCharArray());
#endif

                PidAggregatedSampleMap::const_iterator ag_it = mit->second.getBeginSample(), ag_end = mit->second.getEndSample();

                for (; ag_it != ag_end; ++ag_it)
                {
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Found Module(%ls): in modMap. Pid(%u)",
                                               mit->first.asCharArray(), ag_it->first);
#endif
                    m_it->second.recordSample(ag_it->first, &(ag_it->second));
                }

                // Update Function (IMD) Map...
                AddrFunctionMultMap::const_iterator fit = mit->second.getBeginFunction(), fit_end = mit->second.getEndFunction();

                for (; fit != fit_end; ++fit)
                {
#if ENABLE_PRD_DEBUG_OUTPUT && (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Function map.. VAddr(0x%llx)", fit->first);
#endif
                    m_it->second.recordSample(fit->first, &(fit->second));
                }
            }

            mit->second.clear();
            mit = mMap->erase(mit);
        }

        miter = modList.erase(miter);
        delete mMap;
    }

    auto miIter = modInstanceList.begin();
    modInstanceMap = *miIter;
    ++miIter;
    while (miIter != modInstanceList.end())
    {
        ModInstanceMap* miMap = *miIter;

        for (const auto& it : *miMap)
        {
            if (modInstanceMap->end() == modInstanceMap->find(it.first))
            {
                modInstanceMap->emplace(it.first, it.second);
            }
        }

        miMap->clear();
        delete miMap;
        miIter = modInstanceList.erase(miIter);
    }

    return true;
}


HRESULT PrdTranslator::WriteProfile(const QString& proFile,
                                    PrdReader& tPrdReader,
                                    const MissedInfoType* pMissedInfo,
                                    const PidProcessMap& processMap,
                                    NameModuleMap& moduleMap,
                                    const ModInstanceMap& modInstanceMap)
{
    HRESULT  res = S_OK;

    // Write the CSS files to the same directory as the profile file
    for (gtMap<ProcessIdType, ProcessInfo*>::iterator it = m_processInfos.begin(), itEnd = m_processInfos.end(); it != itEnd; ++it)
    {
        ProcessInfo* pProcessInfo = it->second;

        if (NULL != pProcessInfo && 0U != pProcessInfo->m_callGraph.GetOrder())
        {
            ProcessIdType pid = it->first;

            QString cssFile = proFile.section("\\", 0, -2) + '\\' + QString::number(pid) + ".css";

            TiProcessWorkingSetQuery workingSet(pid);

            CssWriter cssWriter;
            cssWriter.Open(cssFile.toStdWString().c_str());
            cssWriter.Write(pProcessInfo->m_callGraph, workingSet, pid);
        }
    }

    UpdateProgressBar(60ULL, 100ULL);

    int cpuFamily = tPrdReader.GetCpuFamily();
    int cpuModel = tPrdReader.GetCpuModel();

    if (0 == cpuFamily)
    {
        osCpuid cpuid;
        cpuFamily = cpuid.getFamily();
        cpuModel = cpuid.getModel();
    }

    unsigned int cpuCount = tPrdReader.GetCoreCount();

    // Build up the core topology map.
    CoreTopologyMap topMap;

    for (unsigned int j = 0; j < cpuCount; j++)
    {
        CoreTopology topTemp;

        if (tPrdReader.GetTopology(j, &(topTemp.processor), &(topTemp.numaNode)))
        {
            topMap.insert(CoreTopologyMap::value_type(j, topTemp));
        }
    }

    //  Write out all the jit jnc that were sampled during the profile
    //  to the <session name>.dir directory, assumes that the output file is
    //  being written to the same directory
    fnWriteJncFiles(proFile.section('\\', 0, -2).toStdWString().c_str());

    if (gInlineMode)
    {
        //Inline addition
        NameModuleMap::iterator modit = moduleMap.begin();
        NameModuleMap::iterator modit_end = moduleMap.end();

        for (; modit != modit_end; modit++)
        {
            if (CpuProfileModule::JAVAMODULE == modit->second.getModType())
            {
                addJavaInlinedMethods(modit->second);
            }
        }
    }

    UpdateProgressBar(70ULL, 100ULL);

    if (!WriteProfileFile(proFile.toStdWString().c_str(),
                          &processMap,
                          &moduleMap,
                          &topMap,
                          &modInstanceMap,
                          cpuCount,
                          pMissedInfo->missedCount,
                          cpuFamily,
                          cpuModel))
    {
        res = E_UNEXPECTED;
        return res;
    }

    return res;
}

bool PrdTranslator::WriteProfileFile(const gtString& path,
                                     const PidProcessMap* procMap,
                                     const NameModuleMap* modMap,
                                     const CoreTopologyMap* pTopMap,
                                     const ModInstanceMap* pModInstanceMap,
                                     gtUInt32 num_cpus,
                                     gtUInt64 missedCount,
                                     int cpuFamily,
                                     int cpuModel)
{
    CpuProfileWriter writer;
    CpuProfileInfo info;

    // Populate info
    info.m_numCpus      = num_cpus;
    info.m_numSamples   = 0;
    info.m_numMisses    = missedCount;
    info.m_numModules   = modMap->size();
    info.m_tbpVersion   = TBPVER_BEFORE_RI;
    info.m_cpuFamily    = cpuFamily;
    info.m_cpuModel     = cpuModel;
    QString tmp = QDateTime::currentDateTime().toString();
    info.setTimeStamp(tmp.toStdWString().c_str());

    EventMap::const_iterator iter = m_eventMap.begin(), iterEnd = m_eventMap.end();

    for (; iter != iterEnd; ++iter)
    {
        info.addEvent(iter->first, iter->second);
    }

    PidProcessMap::const_iterator pit = procMap->begin(), pend = procMap->end();

    for (; pit != pend; ++pit)
    {
        info.m_numSamples += pit->second.getTotal();
    }

    if (NULL != m_runInfo)
    {
        // We have the RI data, set current TBP version
        info.m_tbpVersion = TBPVER_DEFAULT;

        // populate RI info
        info.m_targetPath = m_runInfo->m_targetPath;
        info.m_wrkDirectory = m_runInfo->m_wrkDirectory;
        info.m_cmdArguments = m_runInfo->m_cmdArguments;
        info.m_envVariables = m_runInfo->m_envVariables;
        info.m_profType = m_runInfo->m_profType;
        info.m_profDirectory = m_runInfo->m_profDirectory;
        info.m_profStartTime = m_runInfo->m_profStartTime;
        info.m_profEndTime = m_runInfo->m_profEndTime;
        info.m_isCSSEnabled = m_runInfo->m_isCSSEnabled;
        info.m_cssUnwindDepth = m_runInfo->m_cssUnwindDepth;
        info.m_cssScope = m_runInfo->m_cssScope;
        info.m_isCssSupportFpo = m_runInfo->m_isCssSupportFpo;
        info.m_isProfilingCLU = m_runInfo->m_isProfilingClu;
        info.m_osName = m_runInfo->m_osName;
        info.m_profScope = m_runInfo->m_profScope;
    }

    UpdateProgressBar(80ULL, 100ULL);

#ifdef AMDT_ENABLE_CPUPROF_DB
    gtUInt64 startTime = GetTickCount();

    gtVector<std::tuple<gtUInt32, gtUInt32>> processThreadList;
    fnGetProcessThreadList(processThreadList);

    ProfilerDataDBWriter DBWriter;
    //TODO: cpu affinity should be part of CpuProfileInfo i.e. info parameter
    DBWriter.Write(path, info, m_runInfo->m_cpuAffinity, *procMap, processThreadList, *modMap, *pModInstanceMap, pTopMap);

    if (m_collectStat)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: DB Write: (%d ms)", (GetTickCount() - startTime));
    }
#else
    GT_UNREFERENCED_PARAMETER(pModInstanceMap);
#endif

    if (!writer.Write(path, &info, procMap, modMap, pTopMap))
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to write profile file '%ls'", path.asCharArray());
        return false;
    }

    return true;
}

unsigned int PrdTranslator::GetCpuCount() const
{
    SYSTEM_INFO sysinfo;
#ifdef CODEANALYST64
    GetNativeSystemInfo(&sysinfo);
#else
    GetSystemInfo(&sysinfo);
#endif
    return sysinfo.dwNumberOfProcessors;
}


bool PrdTranslator::GetProfileEventCount(int* pEventCount)
{
    if (NULL == pEventCount)
    {
        return false;
    }

    PrdReader prdReader;

    if (S_OK != prdReader.Initialize(m_dataFile.toStdWString().c_str()))
    {
        return false;
    }

    *pEventCount = prdReader.GetEventCount();

    prdReader.Close();
    return true;
}

void PrdTranslator::FinalizeUserCallStack(ProcessInfo& processInfo,
                                          ExecutableFile* pPeFile,
                                          gtUInt64 threadID,
                                          gtUInt64 tsc,
                                          EventMaskType eventType,
                                          gtUInt64 instructionPtr,
                                          PrdTranslationStats* const pStats)
{
    DWORD startTime = 0L;
    BEGIN_TICK_COUNT();
    ExecutableFile* pExecutable = pPeFile;

    if (nullptr == pExecutable)
    {
        pExecutable = fnFindExecutableFile(processInfo.m_processId, instructionPtr);
    }

    // if fn is inlined, instruction ptr needs to be manipulated
    // to store VA of stanby inlined fn
    if (NULL != pExecutable)
    {
        SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

        if (NULL != pSymbolEngine)
        {
            gtRVAddr rva = pExecutable->VaToRva(instructionPtr);
            rva = pSymbolEngine->TranslateToInlineeRVA(rva);
            instructionPtr = pExecutable->RvaToVa(rva);
        }
    }

    osCriticalSectionLocker lock(processInfo.m_criticalSection);

    if (!processInfo.m_userCallStacks.empty())
    {
        ThreadUserCallStacksMap::iterator itThread = processInfo.m_userCallStacks.find(static_cast<ThreadIdType>(threadID));

        if (itThread != processInfo.m_userCallStacks.end())
        {
            PeriodicUserCallStackMap* pPeriodicCallStackMap = itThread->second;

            TimeRange timeKey = { tsc, tsc };
            PeriodicUserCallStackMap::iterator itTime = pPeriodicCallStackMap->find(timeKey);

            if (itTime != pPeriodicCallStackMap->end() && tsc <= itTime->first.m_end)
            {
                UserCallStack& userCallStack = itTime->second;

                if (instructionPtr == userCallStack.m_pSampleSite->m_traverseAddr ||
                    IsIbsFetchEvent(eventType) || IsIbsOpEvent(eventType) || IsIbsCluEvent(eventType))
                {
                    EventSampleInfo eventSample;
                    eventSample.m_pSite = userCallStack.m_pSampleSite;
                    eventSample.m_eventId = eventType;
                    eventSample.m_threadId = static_cast<gtUInt32>(threadID);
                    eventSample.m_count = 1ULL;

                    userCallStack.m_pCallStack->AddEventSample(eventSample);
                    userCallStack.m_pSampleSite->m_callStackIndices.Add(userCallStack.m_callStackIndex);
                }
            }
        }
    }

    END_TICK_COUNT(finalizeUserCss);
}

void PrdTranslator::FinalizePartialUserCallStack(ProcessInfo& processInfo,
                                                 gtUInt64 threadID,
                                                 const TimeRange& tscRange,
                                                 gtUInt64 sampleAddr,
                                                 CallStackBuilder& callStackBuilder,
                                                 PrdTranslationStats* const pStats)
{
    GT_UNREFERENCED_PARAMETER(pStats);

    UserCallStack userCallStack;
    userCallStack.m_pSampleSite = callStackBuilder.GetCallGraph().AcquireCallSite(sampleAddr);
    userCallStack.m_pCallStack = callStackBuilder.Finalize(&userCallStack.m_callStackIndex);
    userCallStack.m_kernelTransaction = UserCallStack::KERNEL_TRANSACTION_UNKNOWN;

    PeriodicUserCallStackMap*& pPeriodicCallStackMap = processInfo.m_userCallStacks[static_cast<ThreadIdType>(threadID)];

    if (NULL == pPeriodicCallStackMap)
    {
        pPeriodicCallStackMap = new PeriodicUserCallStackMap();
    }

    bool inserted;
    inserted = pPeriodicCallStackMap->insert(PeriodicUserCallStackMap::value_type(tscRange, userCallStack)).second;
}

void PrdTranslator::FinalizeKernelCallStack(ProcessInfo& processInfo,
                                            gtUInt64 threadID,
                                            gtUInt64 tsc,
                                            gtUInt64 sampleAddr,
                                            EventMaskType eventType,
                                            CallStackBuilder& callStackBuilder,
                                            PrdTranslationStats* const pStats)
{
    DWORD startTime = 0L;
    BEGIN_TICK_COUNT();

    osCriticalSectionLocker lock(processInfo.m_criticalSection);
    EventSampleInfo eventSample;
    eventSample.m_pSite = processInfo.m_callGraph.AcquireCallSite(sampleAddr);
    eventSample.m_eventId = eventType;
    eventSample.m_threadId = static_cast<gtUInt32>(threadID);
    eventSample.m_count = 1ULL;

    ThreadUserCallStacksMap::iterator itThread = processInfo.m_userCallStacks.find(static_cast<ThreadIdType>(threadID));

    if (itThread != processInfo.m_userCallStacks.end())
    {
        PeriodicUserCallStackMap* pPeriodicCallStackMap = itThread->second;

        TimeRange timeKey = { tsc, tsc };
        PeriodicUserCallStackMap::iterator itTime = pPeriodicCallStackMap->find(timeKey);

        if (itTime != pPeriodicCallStackMap->end() && tsc <= itTime->first.m_end)
        {
            UserCallStack& userCallStack = itTime->second;


            if (UserCallStack::KERNEL_TRANSACTION_UNKNOWN == userCallStack.m_kernelTransaction)
            {

                BEGIN_TICK_COUNT();
                ExecutableAnalyzer* pExeAnalyzer = processInfo.AcquireExecutableAnalyzer(userCallStack.m_pSampleSite->m_traverseAddr);

                if (NULL != pExeAnalyzer && pExeAnalyzer->AnalyzeIsSystemCall(userCallStack.m_pSampleSite->m_traverseAddr - 1))
                {
                    userCallStack.m_kernelTransaction = UserCallStack::KERNEL_TRANSACTION_TRUE;
                }
                else
                {
                    userCallStack.m_kernelTransaction = UserCallStack::KERNEL_TRANSACTION_FALSE;
                }

                END_TICK_COUNT(disassemble);
            }

            if (UserCallStack::KERNEL_TRANSACTION_TRUE == userCallStack.m_kernelTransaction)
            {
                if (callStackBuilder.IsEmpty())
                {
                    callStackBuilder.Initialize(userCallStack.m_pSampleSite->m_traverseAddr, 0ULL, 0ULL);
                }
                else
                {
                    callStackBuilder.Push(userCallStack.m_pSampleSite);
                }

                callStackBuilder.Push(*userCallStack.m_pCallStack);
            }
        }
    }

    callStackBuilder.Finalize(eventSample);

    END_TICK_COUNT(finalizeKernelCss);
}

HRESULT PrdTranslator::TranslateKernelCallStack(PRD_KERNEL_CSS_DATA_RECORD& kernelCssRec,
                                                PrdReaderThread& threadPRDReader,
                                                ProcessInfo*& pProcessInfo,
                                                ProcessIdType& processInfoId,
                                                const QList<DWORD>& filters,
                                                ProcessIdType processId,
                                                ThreadIdType threadId,
                                                gtUInt64 timeStamp,
                                                EventMaskType eventType,
                                                unsigned int core,
                                                gtUByte* pBuffer,
                                                PrdTranslationStats* const pStats)
{
    HRESULT hr = S_OK;
    DWORD startTime = 0L;

    bool is64Bit = FALSE != kernelCssRec.m_Is64Bit;
    unsigned int depth = kernelCssRec.m_Depth;

    gtUInt32* pKernelCssValues32 = kernelCssRec.m_CallStack32;
    gtUInt64* pKernelCssValues64 = kernelCssRec.m_CallStack64;

    unsigned int additionalRecordsCount = GetKernelCallStackAdditionalRecordsCount(depth, is64Bit);

    if (0U != additionalRecordsCount)
    {
        PRD_KERNEL_CSS_DATA_RECORD* pKernlCssRecRef = reinterpret_cast<PRD_KERNEL_CSS_DATA_RECORD*>(threadPRDReader.GetLastRecord());
        pKernelCssValues32 = pKernlCssRecRef->m_CallStack32;
        pKernelCssValues64 = pKernlCssRecRef->m_CallStack64;

        threadPRDReader.SkipRawRecords(additionalRecordsCount);
    }

    // Ignore CSS data that is not in the filters (if applicable) or is in the System process
    // CLU profiles IBS Op event, process CSS data for IBS sample only when profiling IBS Op.
    if ((filters.isEmpty() || filters.contains(processId)) &&
        (0 != processId) && (4 != processId) && (8 != processId) &&
        (!IsIbsOpEvent(eventType) || m_runInfo->m_isProfilingIbsOp))
    {
#if ENABLE_PRD_DEBUG_OUTPUT
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Calling BuildCallStack... thread(%d)", GetCurrentThreadId());
#endif
        BEGIN_TICK_COUNT();

        if (is64Bit)
        {
            pKernelCssValues32 = NULL;

            for (unsigned int i = 0U; i < depth; ++i)
            {
                if (IsProfilingDriver(pKernelCssValues64[i]))
                {
                    hr = S_FALSE;
                    break;
                }
            }
        }
        else
        {
            pKernelCssValues64 = NULL;

            for (unsigned int i = 0U; i < depth; ++i)
            {
                if (IsProfilingDriver(pKernelCssValues32[i]))
                {
                    hr = S_FALSE;
                    break;
                }
            }
        }

        if (S_OK == hr)
        {
            if (processId != processInfoId)
            {
                processInfoId = processId;
                pProcessInfo = &AcquireProcessInfo(processInfoId);
            }

            gtUInt64 sampleAddr;
            CallStackBuilder callStackBuilder(pProcessInfo->m_callGraph,
                                              pBuffer,
                                              CallStackBuilder::CalcRequiredBufferSize(MAX_CSS_VALUES));

            if (BuildCallStack(*pProcessInfo,
                               processId,
                               timeStamp,
                               core,
                               pKernelCssValues32,
                               pKernelCssValues64,
                               depth,
                               sampleAddr,
                               callStackBuilder,
                               pStats))
            {
                FinalizeKernelCallStack(*pProcessInfo,
                                        threadId,
                                        timeStamp,
                                        sampleAddr,
                                        eventType,
                                        callStackBuilder,
                                        pStats);
            }
        }

        END_TICK_COUNT(analyzeCss);

#if ENABLE_PRD_DEBUG_OUTPUT
        OS_OUTPUT_DEBUG_LOG(L"Finished calling ProcessCSSData", OS_DEBUG_LOG_EXTENSIVE);
#endif
    }

    return hr;
}

bool PrdTranslator::BuildCallStack(ProcessInfo& processInfo,
                                   ProcessIdType processID,
                                   gtUInt64 tsc,
                                   unsigned core,
                                   gtUInt32* pValues32,
                                   gtUInt64* pValues64,
                                   unsigned depth,
                                   gtUInt64& sampleAddr,
                                   CallStackBuilder& callStackBuilder,
                                   PrdTranslationStats* const pStats)
{
    bool ret = false;

    if (0U != depth && (NULL != pValues32 || NULL != pValues64))
    {
        DWORD startTime = 0L;

        *smoduleName      = L'\0';
        *sfunctionName    = L'\0';
        *sjncName         = L'\0';
        *sjavaSrcFileName = L'\0';

        TiModuleInfo modInfo = { 0 };
        modInfo.processID        = static_cast<gtUInt64>(processID);
        modInfo.cpuIndex         = core;
        modInfo.deltaTick        = tsc;
        modInfo.pModulename      = smoduleName;
        modInfo.pFunctionName    = sfunctionName;
        modInfo.pJncName         = sjncName;
        modInfo.pJavaSrcFileName = sjavaSrcFileName;
        modInfo.moduleType       = evPEModule;

        modInfo.funNameSize      = OS_MAX_PATH;
        modInfo.jncNameSize      = OS_MAX_PATH;
        modInfo.namesize         = OS_MAX_PATH;
        modInfo.srcfilesize      = OS_MAX_PATH;
        modInfo.ModuleStartAddr  = 0;

        if (NULL != pValues32)
        {
            modInfo.sampleAddr = pValues32[0];
        }
        else
        {
            modInfo.sampleAddr = pValues64[0] & ERBT_713_NON_CANONICAL_MASK;
        }


        BEGIN_TICK_COUNT();
        HRESULT hr = fnGetModuleInfo(&modInfo);
        END_TICK_COUNT(findModuleInfo);

        if (S_OK == hr && evPEModule == modInfo.moduleType)
        {
            ret = true;
            sampleAddr = modInfo.sampleAddr;

            if (1U < depth)
            {
                modInfo.funNameSize      = OS_MAX_PATH;
                modInfo.jncNameSize      = OS_MAX_PATH;
                modInfo.namesize         = OS_MAX_PATH;
                modInfo.srcfilesize      = OS_MAX_PATH;
                modInfo.ModuleStartAddr  = 0;

                if (NULL != pValues32)
                {
                    modInfo.sampleAddr = pValues32[1];
                }
                else
                {
                    modInfo.sampleAddr = pValues64[1] & ERBT_713_NON_CANONICAL_MASK;
                }

                BEGIN_TICK_COUNT();
                hr = fnGetModuleInfo(&modInfo);
                END_TICK_COUNT(findModuleInfo);

                if (S_OK == hr && evPEModule == modInfo.moduleType)
                {
                    osCriticalSectionLocker lock(processInfo.m_criticalSection);
                    callStackBuilder.Initialize(modInfo.sampleAddr, 0ULL, 0ULL);

                    if (NULL != pValues32)
                    {
                        for (unsigned int i = 2U; i < depth; ++i)
                        {
                            *smoduleName      = L'\0';
                            *sfunctionName    = L'\0';
                            *sjncName         = L'\0';
                            *sjavaSrcFileName = L'\0';

                            modInfo.funNameSize = OS_MAX_PATH;
                            modInfo.jncNameSize = OS_MAX_PATH;
                            modInfo.namesize    = OS_MAX_PATH;
                            modInfo.srcfilesize = OS_MAX_PATH;
                            modInfo.ModuleStartAddr = 0;
                            modInfo.sampleAddr = pValues32[i];

                            BEGIN_TICK_COUNT();
                            hr = fnGetModuleInfo(&modInfo);
                            END_TICK_COUNT(findModuleInfo);

                            if (S_OK != hr || evPEModule != modInfo.moduleType)
                            {
                                break;
                            }

                            BEGIN_TICK_COUNT();
                            callStackBuilder.Push(modInfo.sampleAddr);

                            if (NULL != pStats) { pStats->m_values[PrdTranslationStats::buildCss].value += GetTickCount() - startTime; }
                        }
                    }
                    else
                    {
                        for (unsigned int i = 2U; i < depth; ++i)
                        {
                            *smoduleName      = L'\0';
                            *sfunctionName    = L'\0';
                            *sjncName         = L'\0';
                            *sjavaSrcFileName = L'\0';

                            modInfo.funNameSize = OS_MAX_PATH;
                            modInfo.jncNameSize = OS_MAX_PATH;
                            modInfo.namesize    = OS_MAX_PATH;
                            modInfo.srcfilesize = OS_MAX_PATH;
                            modInfo.ModuleStartAddr = 0;
                            modInfo.sampleAddr = pValues64[i] & ERBT_713_NON_CANONICAL_MASK;

                            BEGIN_TICK_COUNT();
                            hr = fnGetModuleInfo(&modInfo);
                            END_TICK_COUNT(findModuleInfo);

                            if (S_OK != hr || evPEModule != modInfo.moduleType)
                            {
                                break;
                            }

                            BEGIN_TICK_COUNT();
                            callStackBuilder.Push(modInfo.sampleAddr);

                            if (NULL != pStats) { pStats->m_values[PrdTranslationStats::buildCss].value += GetTickCount() - startTime; }
                        }
                    }
                }
            }

            if (NULL != pStats) { pStats->m_values[PrdTranslationStats::buildCss].count++; }
        }
    }

    return ret;
}

//
// This function adds an individual event identifier to
// the m_eventMap list.
//
void PrdTranslator::AddIbsFetchEvent(unsigned int eventSelect)
{
    EventMap::iterator iter = m_eventMap.find(eventSelect);

    if (m_eventMap.end() == iter)
    {
        m_eventMap.insert(EventMap::value_type(eventSelect, m_ibsFetchCount));
    }
}

void PrdTranslator::AddIbsOpEvent(unsigned int eventSelect)
{
    EventMap::iterator iter = m_eventMap.find(eventSelect);

    if (m_eventMap.end() == iter)
    {
        m_eventMap.insert(EventMap::value_type(eventSelect, m_ibsOpCount));
    }
}

void PrdTranslator::AddCluEvent(unsigned int eventSelect)
{
    EventMap::iterator iter = m_eventMap.find(eventSelect);

    if (m_eventMap.end() == iter)
    {
        m_eventMap.insert(EventMap::value_type(eventSelect, 1));
    }
}

//
// This function tests the IBS event indicator variables
// and adds the appropriate IBS derived events to the
// m_eventMap, which should contain a list of all sampled
// events. If even one bit in a category such as IBS fetch
// is set, then add all the IBS derived events in the
// category.
//
void PrdTranslator::AddIBSFetchEventsToMap(int cpuFamily, int cpuModel)
{
    AddIbsFetchEvent(DE_IBS_FETCH_ALL) ;
    AddIbsFetchEvent(DE_IBS_FETCH_KILLED) ;
    AddIbsFetchEvent(DE_IBS_FETCH_ATTEMPTED) ;
    AddIbsFetchEvent(DE_IBS_FETCH_COMPLETED) ;
    AddIbsFetchEvent(DE_IBS_FETCH_ABORTED) ;
    AddIbsFetchEvent(DE_IBS_L1_ITLB_HIT) ;
    AddIbsFetchEvent(DE_IBS_ITLB_L1M_L2H) ;
    AddIbsFetchEvent(DE_IBS_ITLB_L1M_L2M) ;
    AddIbsFetchEvent(DE_IBS_IC_MISS) ;
    AddIbsFetchEvent(DE_IBS_IC_HIT) ;
    AddIbsFetchEvent(DE_IBS_FETCH_4K_PAGE) ;
    AddIbsFetchEvent(DE_IBS_FETCH_2M_PAGE) ;
    AddIbsFetchEvent(DE_IBS_FETCH_LATENCY) ;

    int cpuModels = cpuModel >> 4;

    // Only supported by family:0x15, models:0x60-0x6F
    if (FAMILY_OR == cpuFamily && 0x6 == cpuModels)
    {
        AddIbsFetchEvent(DE_IBS_FETCH_L2C_MISS);
        AddIbsFetchEvent(DE_IBS_ITLB_REFILL_LAT);
    }
}

void PrdTranslator::AddIBSOpEventsToMap(int cpuFamily, int cpuModel, bool addBr, bool addLS, bool addNB)
{
    // These derived events are common to all ops:
    // branch/ret, load/store and undifferentiated
    AddIbsOpEvent(DE_IBS_OP_ALL) ;
    AddIbsOpEvent(DE_IBS_OP_TAG_TO_RETIRE) ;
    AddIbsOpEvent(DE_IBS_OP_COMP_TO_RETIRE) ;

    if (addBr)
    {
        AddIbsOpEvent(DE_IBS_BRANCH_RETIRED) ;
        AddIbsOpEvent(DE_IBS_BRANCH_MISP) ;
        AddIbsOpEvent(DE_IBS_BRANCH_TAKEN) ;
        AddIbsOpEvent(DE_IBS_BRANCH_MISP_TAKEN) ;
        AddIbsOpEvent(DE_IBS_RETURN) ;
        AddIbsOpEvent(DE_IBS_RETURN_MISP) ;
        AddIbsOpEvent(DE_IBS_RESYNC) ;
    }

    if (addLS)
    {
        AddIbsOpEvent(DE_IBS_LS_ALL_OP) ;
        AddIbsOpEvent(DE_IBS_LS_LOAD_OP) ;
        AddIbsOpEvent(DE_IBS_LS_STORE_OP) ;
        AddIbsOpEvent(DE_IBS_LS_DTLB_L1H) ;
        AddIbsOpEvent(DE_IBS_LS_DTLB_L1M_L2H) ;
        AddIbsOpEvent(DE_IBS_LS_DTLB_L1M_L2M) ;
        AddIbsOpEvent(DE_IBS_LS_DC_MISS) ;
        AddIbsOpEvent(DE_IBS_LS_DC_HIT) ;
        AddIbsOpEvent(DE_IBS_LS_MISALIGNED) ;
        AddIbsOpEvent(DE_IBS_LS_BNK_CONF_LOAD) ;
        AddIbsOpEvent(DE_IBS_LS_BNK_CONF_STORE) ;
        AddIbsOpEvent(DE_IBS_LS_STL_FORWARDED) ;
        AddIbsOpEvent(DE_IBS_LS_STL_CANCELLED) ;
        AddIbsOpEvent(DE_IBS_LS_UC_MEM_ACCESS) ;
        AddIbsOpEvent(DE_IBS_LS_WC_MEM_ACCESS) ;
        AddIbsOpEvent(DE_IBS_LS_LOCKED_OP) ;
        AddIbsOpEvent(DE_IBS_LS_MAB_HIT) ;
        AddIbsOpEvent(DE_IBS_LS_L1_DTLB_4K) ;
        AddIbsOpEvent(DE_IBS_LS_L1_DTLB_2M) ;
        AddIbsOpEvent(DE_IBS_LS_L1_DTLB_1G) ;
        AddIbsOpEvent(DE_IBS_LS_L2_DTLB_4K) ;
        AddIbsOpEvent(DE_IBS_LS_L2_DTLB_2M) ;
        AddIbsOpEvent(DE_IBS_LS_L2_DTLB_1G) ;
        AddIbsOpEvent(DE_IBS_LS_DC_LOAD_LAT) ;

        int cpuModels = cpuModel >> 4;

        // Only supported by family:0x15, models:0x60-0x6F
        if (FAMILY_OR == cpuFamily && 0x6 == cpuModels)
        {
            AddIbsOpEvent(DE_IBS_LS_DC_LD_RESYNC);
        }
    }

    if (addNB)
    {
        AddIbsOpEvent(DE_IBS_NB_LOCAL) ;
        AddIbsOpEvent(DE_IBS_NB_REMOTE) ;
        AddIbsOpEvent(DE_IBS_NB_LOCAL_L3) ;
        AddIbsOpEvent(DE_IBS_NB_LOCAL_CACHE) ;
        AddIbsOpEvent(DE_IBS_NB_REMOTE_CACHE) ;
        AddIbsOpEvent(DE_IBS_NB_LOCAL_DRAM) ;
        AddIbsOpEvent(DE_IBS_NB_REMOTE_DRAM) ;
        AddIbsOpEvent(DE_IBS_NB_LOCAL_OTHER) ;
        AddIbsOpEvent(DE_IBS_NB_REMOTE_OTHER) ;
        AddIbsOpEvent(DE_IBS_NB_CACHE_STATE_M) ;
        AddIbsOpEvent(DE_IBS_NB_CACHE_STATE_O) ;
        AddIbsOpEvent(DE_IBS_NB_LOCAL_LATENCY) ;
        AddIbsOpEvent(DE_IBS_NB_REMOTE_LATENCY) ;
    }
}

void PrdTranslator::AddCluEventsToMap()
{
    AddCluEvent(DE_IBS_CLU_PERCENTAGE);
    AddCluEvent(DE_IBS_CLU_SPANNING);
    AddCluEvent(DE_IBS_CLU_BYTE_PER_EVICT);
    AddCluEvent(DE_IBS_CLU_ACCESS_PER_EVICT);
    AddCluEvent(DE_IBS_CLU_EVICT_COUNT);
    AddCluEvent(DE_IBS_CLU_ACCESS_COUNT);
    AddCluEvent(DE_IBS_CLU_BYTE_COUNT);
}


// Aggregate the IBS derived event and set the IBS event indicator bit
#define AGG_IBS_EVENT(EV) \
    { \
        prdRecord.m_EventType = EV ; \
        AggregateSampleData(prdRecord, pModInfo, pPMap, pMMap, pidModaddrItrMap, 1U, pStats); \
    }


// Aggregate the IBS latency/cycle counts. Increase the derived event count by the specified count value.
#define AGG_IBS_COUNT(EV, COUNT) \
    { \
        prdRecord.m_EventType = EV ; \
        AggregateSampleData(prdRecord, pModInfo, pPMap, pMMap, pidModaddrItrMap, COUNT, pStats); \
    }


// Aggregate the IBS derived CLU events. Increase the CLU event count by the specified count value.
#define AGG_CLU_EVENT_COUNT(EV, COUNT) \
    { \
        prdRecord.m_EventType = EV ; \
        AggregateSampleData(prdRecord, &modInfo, pPMap, pMMap, pidModaddrItrMap, COUNT, pStats); \
    }


void PrdTranslator::AggregateCluData(PidProcessMap* pPMap, NameModuleMap* pMMap, QString  proFile, PrdTranslationStats* const pStats)
{
    DWORD startTime = 0L;

    PidModaddrItrMap* pidModaddrItrMap = new PidModaddrItrMap;

    CLUMap* cluData = m_pCluInfo->GetCLUData();
    CLUMap::iterator it = cluData->begin();
    CLUMap::iterator itEnd = cluData->end();

    for (; it != itEnd; ++it)
    {
        const CLUKey& key = it->first;
        const CLUData& data = it->second;

        BEGIN_TICK_COUNT();
        TiModuleInfo modInfo;
        GetModuleInfoHelper((void*)&key, &modInfo, evCLURecord, proFile);
        END_TICK_COUNT(findModuleInfo);

        if (IsProfilingDriver(modInfo.sampleAddr))
        {
            continue;
        }

        RecordDataStruct prdRecord;
        prdRecord.m_PID           = key.ProcessID;
        prdRecord.m_ProcessorID   = key.core;
        prdRecord.m_RIP           = key.RIP;
        prdRecord.m_ThreadHandle  = key.ThreadID;
        prdRecord.m_EventUnitMask = 0;
        prdRecord.m_eventBitMask  = 0;

        AGG_CLU_EVENT_COUNT(DE_IBS_CLU_PERCENTAGE, data.sumMax);
        AGG_CLU_EVENT_COUNT(DE_IBS_CLU_SPANNING, data.SpanCount);
        AGG_CLU_EVENT_COUNT(DE_IBS_CLU_EVICT_COUNT, data.tot_evictions);
        AGG_CLU_EVENT_COUNT(DE_IBS_CLU_ACCESS_COUNT, data.num_rw);
        AGG_CLU_EVENT_COUNT(DE_IBS_CLU_BYTE_COUNT, data.tot_rw);
    }

    if (pidModaddrItrMap != NULL)
    {
        delete pidModaddrItrMap;
        pidModaddrItrMap = NULL;
    }
}

//
// Interpret the IBS fetch information in the IBS fetch record.
// Tally the appropriate IBS derived event for a given fetch
// condition (event bit or logical combination of event bits.)
//
bool PrdTranslator::ProcessIbsFetchRecord(const IBSFetchRecordData& ibsFetchRec,
                                          TiModuleInfo* pModInfo,
                                          PidProcessMap* pPMap,
                                          NameModuleMap* pMMap,
                                          PidModaddrItrMap* pidModaddrItrMap,
                                          PrdTranslationStats* const pStats)
{
    if (!pPMap || !pMMap)
    {
        return false;
    }

    RecordDataStruct prdRecord;
    prdRecord.m_PID           = ibsFetchRec.m_PID;
    prdRecord.m_EventUnitMask = 0;
    prdRecord.m_eventBitMask  = 0;
    prdRecord.m_ProcessorID   = ibsFetchRec.m_ProcessorID;
    prdRecord.m_RIP           = ibsFetchRec.m_RIP;
    prdRecord.m_ThreadHandle  = ibsFetchRec.m_ThreadHandle;
    prdRecord.m_DeltaTick     = ibsFetchRec.m_DeltaTick;


    if (ibsFetchRec.m_FetchLatency > 1000)
    {
        //IBS fetch samples with latency >1000 are probably outliers, and
        //  should be ignored in aggregation
        return true;
    }

    // IBS all fetch samples (kills + attempts)
    AGG_IBS_EVENT(DE_IBS_FETCH_ALL) ;

    // IBS killed fetches ("case 0") -- All interesting event
    // flags are clear
    if (ibsFetchRec.m_Killed)
    {
        AGG_IBS_EVENT(DE_IBS_FETCH_KILLED) ;
        // Take an early out with IBS killed fetches, effectively
        // filtering killed fetches out of the other event counts
        return (true) ;
    }

    // Any non-killed fetch is an attempted fetch
    AGG_IBS_EVENT(DE_IBS_FETCH_ATTEMPTED) ;

    if (ibsFetchRec.m_FetchCompletion)
    {
        // IBS Fetch Completed
        AGG_IBS_EVENT(DE_IBS_FETCH_COMPLETED) ;
    }
    else
    {
        // IBS Fetch Aborted
        AGG_IBS_EVENT(DE_IBS_FETCH_ABORTED) ;
    }

    // IBS L1 ITLB hit
    if (ibsFetchRec.m_L1TLBHit)
    {
        AGG_IBS_EVENT(DE_IBS_L1_ITLB_HIT) ;
    }

    // IBS L1 ITLB miss and L2 ITLB hit
    if (ibsFetchRec.m_ITLB_L1M_L2H)
    {
        AGG_IBS_EVENT(DE_IBS_ITLB_L1M_L2H) ;
    }

    // IBS L1 & L2 ITLB miss; complete ITLB miss
    if (ibsFetchRec.m_ITLB_L1M_L2M)
    {
        AGG_IBS_EVENT(DE_IBS_ITLB_L1M_L2M) ;
    }

    // IBS instruction cache miss
    if (ibsFetchRec.m_InstCacheMiss)
    {
        AGG_IBS_EVENT(DE_IBS_IC_MISS) ;
    }

    // IBS instruction cache hit
    if (ibsFetchRec.m_InstCacheHit)
    {
        AGG_IBS_EVENT(DE_IBS_IC_HIT) ;
    }

    // IBS page translations (valid when ibsFetchRec.m_PhysicalAddrValid is set)
    if (ibsFetchRec.m_PhysicalAddrValid)
    {
        switch (ibsFetchRec.m_TLBPageSize)
        {
            case L1TLB4K:
                AGG_IBS_EVENT(DE_IBS_FETCH_4K_PAGE) ;
                break;

            case L1TLB2M:
                AGG_IBS_EVENT(DE_IBS_FETCH_2M_PAGE) ;
                break;

            default:
                // DE_IBS_FETCH_1G_PAGE ;
                // DE_IBS_FETCH_XX_PAGE ;
                break;
        }
    }

    if (ibsFetchRec.m_FetchLatency)
    {
        AGG_IBS_COUNT(DE_IBS_FETCH_LATENCY, ibsFetchRec.m_FetchLatency) ;
    }

    if (ibsFetchRec.m_L2CacheMiss)
    {
        AGG_IBS_EVENT(DE_IBS_FETCH_L2C_MISS);
    }

    if (ibsFetchRec.m_ITLBRefillLatency)
    {
        AGG_IBS_COUNT(DE_IBS_ITLB_REFILL_LAT, ibsFetchRec.m_ITLBRefillLatency);
    }

    return true;
}

bool PrdTranslator::ProcessIbsOpRecord(const IBSOpRecordData& ibsOpRec,
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
                                       PrdTranslationStats* const pStats)
{
    GT_UNREFERENCED_PARAMETER(bDoCLU);
    GT_UNREFERENCED_PARAMETER(bLdStCollect);
    GT_UNREFERENCED_PARAMETER(L1DcAssoc);
    GT_UNREFERENCED_PARAMETER(L1DcLineSize);
    GT_UNREFERENCED_PARAMETER(L1DcLinesPerTag);
    GT_UNREFERENCED_PARAMETER(L1DcSize);

    if (!pPMap || !pMMap)
    {
        return false;
    }

    RecordDataStruct prdRecord;
    prdRecord.m_PID           = ibsOpRec.m_PID;
    prdRecord.m_EventUnitMask = 0;
    prdRecord.m_eventBitMask = 0;
    prdRecord.m_ProcessorID   = ibsOpRec.m_ProcessorID;
    prdRecord.m_RIP           = ibsOpRec.m_RIP;
    prdRecord.m_ThreadHandle  = ibsOpRec.m_ThreadHandle;
    prdRecord.m_DeltaTick     = ibsOpRec.m_DeltaTick;

    // All IBS op samples
    AGG_IBS_EVENT(DE_IBS_OP_ALL) ;

    // Tally retire cycle counts for all sampled macro-ops
    if (ibsOpRec.m_TagToRetireCycles)
    {
        // IBS tagged to retire cycles
        AGG_IBS_COUNT(DE_IBS_OP_TAG_TO_RETIRE, ibsOpRec.m_TagToRetireCycles) ;
    }

    if (ibsOpRec.m_CompToRetireCycles)
    {
        // IBS completion to retire cycles
        AGG_IBS_COUNT(DE_IBS_OP_COMP_TO_RETIRE, ibsOpRec.m_CompToRetireCycles) ;
    }

    // Test for an IBS branch macro-op
    if (ibsOpRec.m_OpBranchRetired)
    {
        // IBS Branch retired op
        AGG_IBS_EVENT(DE_IBS_BRANCH_RETIRED) ;

        // Test branch-specific event flags
        if (ibsOpRec.m_OpBranchMispredicted)
        {
            // IBS mispredicted Branch op
            AGG_IBS_EVENT(DE_IBS_BRANCH_MISP) ;
        }

        if (ibsOpRec.m_OpBranchTaken)
        {
            // IBS taken Branch op
            AGG_IBS_EVENT(DE_IBS_BRANCH_TAKEN) ;
        }

        if (ibsOpRec.m_OpBranchTaken && ibsOpRec.m_OpBranchMispredicted)
        {
            // IBS mispredicted taken branch op
            AGG_IBS_EVENT(DE_IBS_BRANCH_MISP_TAKEN) ;
        }

        if (ibsOpRec.m_OpReturn)
        {
            // IBS return op
            AGG_IBS_EVENT(DE_IBS_RETURN) ;
        }

        if (ibsOpRec.m_OpReturn && ibsOpRec.m_OpBranchMispredicted)
        {
            // IBS mispredicted return op
            AGG_IBS_EVENT(DE_IBS_RETURN_MISP) ;
        }
    } // Branch and return op sample

    // Test for a resync macro-op
    if (ibsOpRec.m_OpBranchResync)
    {
        // IBS resync OP
        AGG_IBS_EVENT(DE_IBS_RESYNC) ;
    }

    if (!ibsOpRec.m_IbsLdOp && !ibsOpRec.m_IbsStOp)
    {
        // If no load or store operation, then take an early return
        // No more derived events need to be tallied
        return (true) ;
    }

    // Count the number of LS op samples
    AGG_IBS_EVENT(DE_IBS_LS_ALL_OP) ;

    // Count and handle load ops
    if (ibsOpRec.m_IbsLdOp)
    {
        // Tally an IBS load derived event
        AGG_IBS_EVENT(DE_IBS_LS_LOAD_OP) ;

        // If the load missed in DC, tally the DC load miss latency
        if (ibsOpRec.m_IbsDcMiss)
        {
            // DC load miss latency is only reliable for load ops
            AGG_IBS_COUNT(DE_IBS_LS_DC_LOAD_LAT, ibsOpRec.m_IbsDcMissLat) ;
        }

        // Data forwarding info are valid only for load ops
        if (ibsOpRec.m_IbsDcStToLdFwd)
        {
            AGG_IBS_EVENT(DE_IBS_LS_STL_FORWARDED) ;
        }

        if (ibsOpRec.m_IbsDcStToLdCan)
        {
            AGG_IBS_EVENT(DE_IBS_LS_STL_CANCELLED) ;
        }

        // NB data is only guaranteed reliable for load operations
        // that miss in L1 and L2 cache. NB data arrives too late
        // to be reliable for store operations
        if (ibsOpRec.m_IbsDcMiss && (ibsOpRec.m_NbIbsReqSrc != 0))
        {
            // NB data is valid, so tally derived NB events
            if (ibsOpRec.m_NbIbsReqDstProc)
            {
                // Request was serviced by remote processor
                AGG_IBS_EVENT(DE_IBS_NB_REMOTE) ;
                AGG_IBS_COUNT(DE_IBS_NB_REMOTE_LATENCY, ibsOpRec.m_IbsDcMissLat) ;

                switch (ibsOpRec.m_NbIbsReqSrc)
                {
                    case 0x2:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_REMOTE_CACHE) ;

                        if (ibsOpRec.m_NbIbsCacheHitSt)
                        {
                            AGG_IBS_EVENT(DE_IBS_NB_CACHE_STATE_O) ;
                        }
                        else
                        {
                            AGG_IBS_EVENT(DE_IBS_NB_CACHE_STATE_M) ;
                        }

                        break ;
                    }

                    case 0x3:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_REMOTE_DRAM) ;
                        break ;
                    }

                    case 0x7:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_REMOTE_OTHER) ;
                        break ;
                    }

                    default:
                    {
                        break ;
                    }
                }
            }
            else
            {
                // Request was serviced by local processor
                AGG_IBS_EVENT(DE_IBS_NB_LOCAL) ;
                AGG_IBS_COUNT(DE_IBS_NB_LOCAL_LATENCY, ibsOpRec.m_IbsDcMissLat) ;

                switch (ibsOpRec.m_NbIbsReqSrc)
                {
                    case 0x1:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_LOCAL_L3) ;
                        break ;
                    }

                    case 0x2:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_LOCAL_CACHE) ;

                        if (ibsOpRec.m_NbIbsCacheHitSt)
                        {
                            AGG_IBS_EVENT(DE_IBS_NB_CACHE_STATE_O) ;
                        }
                        else
                        {
                            AGG_IBS_EVENT(DE_IBS_NB_CACHE_STATE_M) ;
                        }

                        break ;
                    }

                    case 0x3:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_LOCAL_DRAM) ;
                        break ;
                    }

                    case 0x7:
                    {
                        AGG_IBS_EVENT(DE_IBS_NB_LOCAL_OTHER) ;
                        break ;
                    }

                    default:
                    {
                        break ;
                    }
                }
            }
        }
    }

    // Count and handle store ops
    if (ibsOpRec.m_IbsStOp)
    {
        AGG_IBS_EVENT(DE_IBS_LS_STORE_OP) ;
    }

    if (ibsOpRec.m_IbsDcMiss)
    {
        AGG_IBS_EVENT(DE_IBS_LS_DC_MISS) ;
    }
    else
    {
        AGG_IBS_EVENT(DE_IBS_LS_DC_HIT) ;
    }

    if (ibsOpRec.m_IbsDcMisAcc)
    {
        AGG_IBS_EVENT(DE_IBS_LS_MISALIGNED) ;
    }

    if (ibsOpRec.m_IbsDcLdBnkCon)
    {
        AGG_IBS_EVENT(DE_IBS_LS_BNK_CONF_LOAD) ;
    }

    if (ibsOpRec.m_IbsDcStBnkCon)
    {
        AGG_IBS_EVENT(DE_IBS_LS_BNK_CONF_STORE) ;
    }

    if (ibsOpRec.m_IbsDcUcMemAcc)
    {
        AGG_IBS_EVENT(DE_IBS_LS_UC_MEM_ACCESS) ;
    }

    if (ibsOpRec.m_IbsDcWcMemAcc)
    {
        AGG_IBS_EVENT(DE_IBS_LS_WC_MEM_ACCESS) ;
    }

    if (ibsOpRec.m_IbsDcLockedOp)
    {
        AGG_IBS_EVENT(DE_IBS_LS_LOCKED_OP) ;
    }

    if (ibsOpRec.m_IbsDcMabHit)
    {
        AGG_IBS_EVENT(DE_IBS_LS_MAB_HIT) ;
    }

    // IbsDcLinAddrValid is true when address translation was successful.
    // Some macro-ops do not perform an address translation and use only
    // a physical address.
    bool useL2TranslationSize = false ;

    if (ibsOpRec.m_IbsDcLinAddrValid)
    {
        if (! ibsOpRec.m_IbsDcL1tlbMiss)
        {
            // L1 DTLB hit -- This is the most frequent case
            AGG_IBS_EVENT(DE_IBS_LS_DTLB_L1H) ;
        }
        else if (ibsOpRec.m_IbsDcL2tlbMiss)
        {
            // L1 DTLB miss, L2 DTLB miss
            AGG_IBS_EVENT(DE_IBS_LS_DTLB_L1M_L2M) ;
        }
        else
        {
            // L1 DTLB miss, L2 DTLB hit
            AGG_IBS_EVENT(DE_IBS_LS_DTLB_L1M_L2H) ;
            useL2TranslationSize = true ;
        }

        if (useL2TranslationSize)
        {
            // L2 DTLB page translation
            if (ibsOpRec.m_IbsDcL2tlbHit2M)
            {
                // 2M L2 DTLB page translation
                AGG_IBS_EVENT(DE_IBS_LS_L2_DTLB_2M) ;
            }
            else  if (ibsOpRec.m_IbsDcL2tlbHit1G)
            {
                // 1G L2 DTLB page translation
                AGG_IBS_EVENT(DE_IBS_LS_L2_DTLB_1G) ;
            }
            else
            {
                // 4K L2 DTLB page translation
                AGG_IBS_EVENT(DE_IBS_LS_L2_DTLB_4K) ;
            }
        }
        else
        {
            // L1 DTLB page translation
            if (ibsOpRec.m_IbsDcL1tlbHit2M)
            {
                // 2M L1 DTLB page translation
                AGG_IBS_EVENT(DE_IBS_LS_L1_DTLB_2M) ;
            }
            else  if (ibsOpRec.m_IbsDcL1tlbHit1G)
            {
                // 1G L1 DTLB page translation
                AGG_IBS_EVENT(DE_IBS_LS_L1_DTLB_1G) ;
            }
            else
            {
                // This is the most common case, unfortunately
                AGG_IBS_EVENT(DE_IBS_LS_L1_DTLB_4K) ;
            }
        }
    }

    if (m_runInfo->m_isProfilingClu)
    {
        if (NULL != m_pCluInfo)
        {
            m_pCluInfo->RecordCacheLdSt((IBSOpRecordData*)&ibsOpRec, pModInfo, pMMap, ibsOpRec.m_IbsLdOp);
        }
        else
        {
            return false;
        }
    }

    if (ibsOpRec.m_IbsOpLdResync)
    {
        AGG_IBS_EVENT(DE_IBS_LS_DC_LD_RESYNC);
    }

    return (true) ;
}

//FIXME [Suravee]: Should not need this
#if 0
bool PrdTranslator::GetIbsConfig(IbsConfig* pConfig)
{
    if (NULL == pConfig) { return false; }

    PrdReader prdReader;

    if (S_OK != prdReader.Initialize(m_dataFile.toStdWString().c_str()))
    {
        return false;
    }

    prdReader.GetIBSConfig(&m_ibsFetchCount, &m_ibsOpCount);

    pConfig->fetchMaxCount = m_ibsFetchCount;
    pConfig->fetchSampling = (m_ibsFetchCount != 0);
    pConfig->opMaxCount = m_ibsOpCount;
    pConfig->opSampling = (m_ibsOpCount != 0);
    return true;
}
#endif


bool PrdTranslator::GetTimerInterval(gtUInt64* resolution)
{
    if (NULL == resolution) { return false; }

    PrdReader prdReader;

    if (S_OK != prdReader.Initialize(m_dataFile.toStdWString().c_str()))
    {
        return false;
    }

    *resolution = prdReader.GetTimerResultion();
    return true;
}


void PrdTranslator::addJavaInlinedMethods(CpuProfileModule&  mod)
{
    AddrFunctionMultMap inlinedFuncMap;

    // Iterate over the AddrFunctionMultMap entries
    // Open JNC file
    // Get the JavaInlineMap
    // if the inline entries are available
    //  Iterate over AptAggregatedSampleMap
    //  if the ip falls in Inlined method
    //  create a new CA_Function fo this IP

    if (CpuProfileModule::JAVAMODULE != mod.getModType())
    {
        return;
    }

    AddrFunctionMultMap::iterator it = mod.getBeginFunction();
    AddrFunctionMultMap::iterator itEnd = mod.getEndFunction();

    for (; it != itEnd; ++it)
    {
        const CpuProfileFunction& tmpFunction = (*it).second;
        // Construct the JNC file path and Open the JNCfile
        wchar_t jncName[OS_MAX_PATH] = { L'\0' };
        gtString path(m_dataFile.toStdWString().c_str());
        osFilePath tmpPath(path);
        swprintf(jncName, OS_MAX_PATH, L"%s\\%s",
                 tmpPath.fileDirectoryAsString().asCharArray(),
                 tmpFunction.getJncFileName().asCharArray());
        JavaJncReader javaJncReader;

        // Open the JNC file
        if (! javaJncReader.Open(jncName))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"javaJncReader Open failed : %S", jncName);
            return;
        }

        JavaInlineMap* jilMap = javaJncReader.GetInlineMap();

        // If there is no inlined functions in this JNC file, go to next function
        if (jilMap->begin() == jilMap->end())
        {
            javaJncReader.Close();
            continue;
        }

        if (true == gNestedJavaInline)
        {
            CheckForNestedJavaInlinedFunction(it->second, jilMap, inlinedFuncMap);
        }
        else
        {
            CheckForJavaInlinedFunction(it->second, jilMap, inlinedFuncMap);
        }

        javaJncReader.Close();
    } // AddrFunctionMultMap entries

    // Now add the inlined functions to CpuProfileModule
    AddrFunctionMultMap::const_iterator inlineIt = inlinedFuncMap.begin();
    AddrFunctionMultMap::const_iterator inlineItEnd = inlinedFuncMap.end();

    for (; inlineIt != inlineItEnd; inlineIt++)
    {
        mod.recordFunction(inlineIt->first, &(inlineIt->second));
    }

    return;
} // addJavaInlinedMethods


bool
PrdTranslator::CheckForJavaInlinedFunction(CpuProfileFunction&   func,
                                           JavaInlineMap*        pJilMap,
                                           AddrFunctionMultMap&  inlinedFuncMap)
{
    if (NULL == pJilMap)
    {
        return false;
    }

    // For each sample in CpuProfileFunction
    AptAggregatedSampleMap::iterator ait = func.getBeginSample();
    AptAggregatedSampleMap::iterator aend = func.getEndSample();

    while (ait != aend)
    {
        AggregatedSample aSample = ait->second;
        AptKey aAptKey           = ait->first;

        gtUInt64 ip = ait->first.m_addr;
        bool foundInline = false;

        JavaInlineMap::iterator mit = pJilMap->begin();
        JavaInlineMap::iterator mitEnd = pJilMap->end();

        // DEBUG:
        //if (NULL == pPRDDebugFP) {
        //    pPRDDebugFP = fopen("C:\\Temp\\Java-Translation.txt", "a+");
        //}
        //fprintf(pPRDDebugFP, "==========================================\n", ip);
        //fprintf(pPRDDebugFP, "Vaddr IP = %lu\n", ip);

        for (; mit != mitEnd && (false == foundInline); mit++)
        {
            JNCInlineMap::iterator ilmit = mit->second.begin();
            JNCInlineMap::iterator ilmitEnd = mit->second.end();

            for (; ilmit != ilmitEnd && (false == foundInline); ilmit++)
            {
                // DEBUG:
                //fprintf(pPRDDebugFP, "     methodId = 0x%lx (key=0x%lx)\n",
                //        ilmit->second.methodId, ilmit->first);
                //fprintf(pPRDDebugFP, "      lineNum = %d\n", ilmit->second.lineNum);
                //fprintf(pPRDDebugFP, "   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
                //fprintf(pPRDDebugFP, "      symName = %s\n", ilmit->second.symName.c_str());

                AddressRangeList::iterator listit = ilmit->second.addrs.begin();
                AddressRangeList::iterator listitEnd = ilmit->second.addrs.end();

                gtUInt64 funcAddr = 0;

                for (; listit != listitEnd; listit++)
                {
                    const addrRanges& ar = *listit;
                    gtUInt64 stAddr = ar.startAddr;
                    gtUInt64 stopAddr = ar.stopAddr;

                    if (0 == funcAddr)
                    {
                        funcAddr = stAddr;
                    }

                    //fprintf(pPRDDebugFP, " IP: 0x%lx Address range 0x%lx - 0x%lx\n",
                    //          ip, stAddr, stopAddr);

                    if ((ip >= stAddr) && (ip <= stopAddr))
                    {
                        foundInline = true;

                        //fprintf(pPRDDebugFP, " Found Inlined Method for IP(0x%lx) - range 0x%lx - 0x%lx\n",
                        //   ip, ar.startAddr, ar.stopAddr);
                        //fprintf(pPRDDebugFP, "   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
                        //fprintf(pPRDDebugFP, "      symName = %s\n", ilmit->second.symName.c_str());

                        gtString javaFuncName(L"[inlined] ");
                        javaFuncName.append(func.getFuncName());
                        javaFuncName.append(L"::");
                        gtString inlineName;
                        inlineName.fromUtf8String(ilmit->second.symName);
                        javaFuncName.append(inlineName);

                        gtString javaSrcFileName;
                        javaSrcFileName.fromUtf8String(ilmit->second.sourceFile);

                        AddrFunctionMultMap::iterator fit = inlinedFuncMap.find(funcAddr);

                        if (fit == inlinedFuncMap.end())
                        {
                            CpuProfileFunction func1(javaFuncName, funcAddr, (stopAddr - funcAddr), func.getJncFileName(), javaSrcFileName);
                            fit = inlinedFuncMap.insert(AddrFunctionMultMap::value_type(funcAddr, func1));
                            fit->second.insertSample(aAptKey, aSample);
                        }
                        else
                        {
                            fit->second.addSample(aAptKey, aSample);
                        }
                    }

                    //else
                    //{
                    //     fprintf(pPRDDebugFP, " No Inlined Method for IP(0x%lx) - funcAddr(0x%lx), range 0x%lx - 0x%lx\n",
                    //         ip, funcAddr, ar.startAddr, ar.stopAddr);
                    //}
                    if (foundInline)
                    {
                        break;
                    }
                } // address range in a inlined methos
            } // JNCInlineMap entries
        } // java inline map entries in a CpuProfileFunction

        AptAggregatedSampleMap::iterator tmpait = ait;
        tmpait++;

        if (foundInline)
        {
            func.removeSample(ait);
        }

        ait = tmpait;
    } // AptAggregatedSampleMap entries

    return true;
}

gtString
PrdTranslator::GetJavaNestedFunctionParentName(JNCInlineMap& jilMap, gtString javaInlinedFunc)
{
    gtString parentsName(L"::");
    JNCInlineMap::iterator it = jilMap.begin();

    while (it != jilMap.end())
    {
        gtString name;
        name.fromUtf8String(it->second.symName);

        if (javaInlinedFunc == name)
        {
            break;
        }
        else
        {
            parentsName.append(name);
            parentsName.append(L"::");
        }

        it++;
    }

    return parentsName;
}

bool
PrdTranslator::CheckForNestedJavaInlinedFunction(
    CpuProfileFunction&   func,
    JavaInlineMap*        pJilMap,
    AddrFunctionMultMap&  inlinedFuncMap
)
{
    if (NULL == pJilMap)
    {
        return false;
    }

    // For each sample in CpuProfileFunction
    AptAggregatedSampleMap::iterator ait = func.getBeginSample();
    AptAggregatedSampleMap::iterator aend = func.getEndSample();

    while (ait != aend)
    {
        AggregatedSample aSample = ait->second;
        AptKey aAptKey           = ait->first;

        gtUInt64 ip = ait->first.m_addr;
        bool foundInline = false;

        JavaInlineMap::iterator mit = pJilMap->begin();
        JavaInlineMap::iterator mitEnd = pJilMap->end();

        // DEBUG:
        // if (NULL == pPRDDebugFP) {
        //     pPRDDebugFP = fopen("C:\\Temp\\Java-Translation.txt", "a+");
        // }
        // fprintf(pPRDDebugFP, "==========================================\n", ip);
        // fprintf(pPRDDebugFP, "Vaddr IP = %lu\n", ip);

        for (; mit != mitEnd && (false == foundInline); mit++)
        {

            JNCInlineMap::reverse_iterator ilmit = mit->second.rbegin();
            JNCInlineMap::reverse_iterator ilmitEnd = mit->second.rend();

            for (; ilmit != ilmitEnd && (false == foundInline); ilmit++)
            {
                // DEBUG:
                //fprintf(pPRDDebugFP, "     methodId = 0x%lx (key=0x%lx)\n",
                //        ilmit->second.methodId, ilmit->first);
                //fprintf(pPRDDebugFP, "      lineNum = %d\n", ilmit->second.lineNum);
                //fprintf(pPRDDebugFP, "   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
                //fprintf(pPRDDebugFP, "      symName = %s\n", ilmit->second.symName.c_str());

                AddressRangeList::iterator listit = ilmit->second.addrs.begin();
                AddressRangeList::iterator listitEnd = ilmit->second.addrs.end();

                gtUInt64 funcAddr = 0;

                for (; listit != listitEnd; listit++)
                {
                    const addrRanges& ar = *listit;
                    gtUInt64 stAddr = ar.startAddr;
                    gtUInt64 stopAddr = ar.stopAddr;

                    if (0 == funcAddr)
                    {
                        funcAddr = stAddr;
                    }

                    //fprintf(pPRDDebugFP, " IP: 0x%lx Address range 0x%lx - 0x%lx\n",
                    //          ip, stAddr, stopAddr);

                    if ((ip >= stAddr) && (ip <= stopAddr))
                    {
                        foundInline = true;

                        //fprintf(pPRDDebugFP, " Found Inlined Method for IP(0x%lx) - range 0x%lx - 0x%lx\n",
                        //   ip, ar.startAddr, ar.stopAddr);
                        //fprintf(pPRDDebugFP, "   sourceFile = %s\n", ilmit->second.sourceFile.c_str());
                        //fprintf(pPRDDebugFP, "      symName = %s\n", ilmit->second.symName.c_str());

                        gtString javaFuncName(L"[inlined] ");
                        javaFuncName.append(func.getFuncName());

                        // Now we need to get the name of the parents
                        gtString inlineName;
                        inlineName.fromUtf8String(ilmit->second.symName);

                        javaFuncName.append(GetJavaNestedFunctionParentName(mit->second, inlineName));
                        javaFuncName.append(inlineName);

                        gtString javaSrcFileName;
                        javaSrcFileName.fromUtf8String(ilmit->second.sourceFile);

                        AddrFunctionMultMap::iterator fit = inlinedFuncMap.find(funcAddr);

                        if (fit == inlinedFuncMap.end())
                        {
                            CpuProfileFunction func1(javaFuncName, funcAddr, (stopAddr - funcAddr), func.getJncFileName(), javaSrcFileName);
                            fit = inlinedFuncMap.insert(AddrFunctionMultMap::value_type(funcAddr, func1));
                            fit->second.insertSample(aAptKey, aSample);
                        }
                        else
                        {
                            fit->second.addSample(aAptKey, aSample);
                        }
                    }

                    //else
                    //{
                    //     fprintf(pPRDDebugFP, " No Inlined Method for IP(0x%lx) - funcAddr(0x%lx), range 0x%lx - 0x%lx\n",
                    //         ip, funcAddr, ar.startAddr, ar.stopAddr);
                    //}
                    if (foundInline)
                    {
                        break;
                    }
                } // address range in a inlined methos
            } // JNCInlineMap entries
        } // java inline map entries in a CpuProfileFunction

        AptAggregatedSampleMap::iterator tmpait = ait;
        tmpait++;

        if (foundInline)
        {
            func.removeSample(ait);
        }

        ait = tmpait;
    } // AptAggregatedSampleMap entries

    return true;
}

// PrintMemoryUsage
//
// Note: GetProcessMemoryInfo() API is defined in Psapi.lib. This library needs to linked in.
//
// 1. The resolution of the GetTickCount function is limited to the resolution of the system timer,
//    which is typically in the range of 10 milliseconds to 16 milliseconds.
// 2. The elapsed time is stored as a DWORD value. Therefore, the time will wrap around to zero if
//    the system is run continuously for 49.7 days.
// 3. To avoid this problem, use the GetTickCount64() function. But thus is supported only from
//    Windows Vista / Windows Server 2008.
//
void PrintMemoryUsage(const wchar_t* header)
{
    DWORD                   procId = GetCurrentProcessId();
    HANDLE                  hProc;
    PROCESS_MEMORY_COUNTERS pmc;

    hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                        FALSE, procId);

    if (NULL == hProc)
    {
        return;
    }

    // Print the header, if provided by the caller...
    if (NULL != header)
    {
        OS_OUTPUT_DEBUG_LOG(header, OS_DEBUG_LOG_DEBUG);
    }

    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc)))
    {
#if 0
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tPageFaultCount: %d", pmc.PageFaultCount);
#endif
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tPeakWorkingSetSize: %d bytes", pmc.PeakWorkingSetSize);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tWorkingSetSize: %d bytes", pmc.WorkingSetSize);
#if 0
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tQuotaPeakPagedPoolUsage: 0x%08X", pmc.QuotaPeakPagedPoolUsage);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tQuotaPagedPoolUsage: 0x%08X", pmc.QuotaPagedPoolUsage);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tQuotaPeakNonPagedPoolUsage: 0x%08X", pmc.QuotaPeakNonPagedPoolUsage);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tQuotaNonPagedPoolUsage: 0x%08X", pmc.QuotaNonPagedPoolUsage);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tPagefileUsage: 0x%08X", pmc.PagefileUsage);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"\tPeakPagefileUsage: 0x%08X", pmc.PeakPagefileUsage);
#endif
    }

    CloseHandle(hProc);

    return;
}
