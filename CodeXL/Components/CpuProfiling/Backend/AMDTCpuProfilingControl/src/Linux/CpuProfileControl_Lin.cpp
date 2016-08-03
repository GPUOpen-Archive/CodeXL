//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileControl_Lin.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingControl/src/Linux/CpuProfileControl_Lin.cpp#18 $
// Last checkin:   $DateTime: 2016/04/14 02:12:20 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569057 $
//=====================================================================

//
// This is the Linux Specific  CpuProfileControl Layer
//

//
// Headers
//
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

// project headers
#include "CpuProfileControl_Lin.h"
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>

//
// typedefs
//
enum CPU_PROFILE_TYPE
{
    PROFILE_TYPE_NOT_CONFIGURED      = 0x00,
    PROFILE_TYPE_TIME_SAMPLING       = 0x01,
    PROFILE_TYPE_EVENT_SAMPLING      = 0x02,
    PROFILE_TYPE_EVENT_COUNTING      = 0x04,
    PROFILE_TYPE_IBS                 = 0x08,
    PROFILE_TYPE_PERF_EVENT_SAMPLING = 0x10,
    PROFILE_TYPE_PERF_EVENT_COUNTING = 0x20,
    PROFILE_TYPE_CPU_UTILIZATION     = 0x40
};

//
// Globals
//
// Whether to use PERF or OProfile
bool g_usePERF = true;

/// Holds one client's shared information
typedef struct
{
    /// Whether the client is paused
    bool paused;
    /// Process ID of the client
    pid_t clientPid;
} SHARED_CLIENT;

/// File descriptor to shared object returned by smh_open()
int g_shm_fd = -1;

/// Address of shared object returned by mmap()
SHARED_CLIENT* g_sharedObj = (SHARED_CLIENT*) - 1;

// CaPerf layer specific global objects
PerfPmuSession* g_pProfileSession;
PerfProfiler* g_pProfiler;
PerfConfig* g_pProfileConfig;
PerfEvent* g_pEvent; // UNUSED ?
PerfPmuTarget* g_pTarget;

gtList< std::pair<PerfEvent, gtUInt64> >   gSamplingEventList;
gtUInt32 gNumEbpEvents = 0;

#ifdef ENABLE_FAKETIMER
    // The timer hack index is the event index of the SW timer that is immediately followed by a non-user specified
    // clock unhalted event, for kernels < 3.0, where the SW timer event doesn't give real call stacks.
    int g_timerHackIndex = -1;
    gtUInt64 g_timerHackNsPeriod = 0;

    // globals to hold the fake-timer event and its sampling period.
    PerfEvent g_fakeTimerEvent;
#endif

// Profile State
//ProfileState g_profileState = ProfilingUnavailable;

#ifdef CODEXL_LIBCPUPERFEVENT_AVBL
    EventEngine g_eventEngine;
    gtUInt64 g_pmcAvailable; // counter mask; depends on the family
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

// Output directory & file name
wchar_t g_outputDir[PATH_MAX] = { 0 };
wchar_t g_outputFile[PATH_MAX] = { 0 };

// System Wide Profile
volatile bool g_isSWP = true;
volatile bool g_callStack = false;
bool g_validSystemCoreMask = true;
volatile bool g_isIbs = false;

// Target details
size_t g_numCpus = 0;
size_t g_pCpuSize = 0; // this is used to maintain the size of g_pCpus;
int* g_pCpus = NULL;
gtList<pid_t>* g_pPidList;

// extern variables; These are defined in LibCPUProfileControl.cpp
extern gtUInt32 INVALID_CLIENT;
extern osProcessId g_clientId[MAX_CLIENT_COUNT];
extern ProfileState g_profilingState[MAX_CLIENT_COUNT];
extern unsigned long g_profileType[MAX_CLIENT_COUNT];

extern gtString g_errorString[MAX_CLIENT_COUNT];
extern gtString g_invalidClientErr;

extern unsigned int g_maxCore; // active cores
extern gtUInt64 g_SystemCoreMask;

extern gtUInt64* g_pSystemCoreMask;
extern gtUInt32 g_SystemCoreMaskCount;
extern gtUInt32 g_SystemCoreMaskSize;

extern int g_maxEventConfigs;

gtUInt32 helpGetClientId();
bool isProfilingNow(gtUInt32 clientId);

//
// Macros
//
#define CPU_PERF_COUNTING_MODE(__clientId) \
    (   (0 != (g_profileType[__clientId] & PROFILE_TYPE_EVENT_COUNTING)) \
        || (0 != (g_profileType[__clientId] & PROFILE_TYPE_PERF_EVENT_COUNTING)) )

#define CPU_PERF_SAMPLING_MODE(__clientId) \
    (   (0 != (g_profileType[__clientId] & PROFILE_TYPE_TIME_SAMPLING))        \
        || (0 != (g_profileType[__clientId] & PROFILE_TYPE_EVENT_SAMPLING))       \
        || (0 != (g_profileType[__clientId] & PROFILE_TYPE_PERF_EVENT_SAMPLING))  \
        || (0 != (g_profileType[__clientId] & PROFILE_TYPE_IBS)) )

#define CPU_PERF_TBP_ONLY_MODE(__clientId) \
    (   (0 != (g_profileType[__clientId] & PROFILE_TYPE_TIME_SAMPLING))        \
        && (0 == (g_profileType[__clientId] & PROFILE_TYPE_EVENT_SAMPLING))       \
        && (0 == (g_profileType[__clientId] & PROFILE_TYPE_PERF_EVENT_SAMPLING))  \
        && (0 == (g_profileType[__clientId] & PROFILE_TYPE_IBS)) )

/// The size of the shared memory
#define CPU_PROF_SHARED_MEM_SIZE (sizeof(SHARED_CLIENT))

//
// Local Classes
//
class ProfileControlMonitor : public osThread
{
public:
    ProfileControlMonitor(osProcessId launchedProcessId, bool puased);
    // Disallow default constructtion
    ProfileControlMonitor() = delete;
    ~ProfileControlMonitor() {};

    // Overrides osThread
    virtual int entryPoint();

private:
    // test and set the control state
    bool isControlRequested(void);

    // The process Id of the launcher process:
    osProcessId m_launcherProcessId;

    // save the last known state
    bool m_paused;
};

ProfileControlMonitor::ProfileControlMonitor(osProcessId launchedProcessId, bool paused):
    osThread(L"ProfileControlMonitor"), m_launcherProcessId(launchedProcessId), m_paused(paused)
{
}

bool ProfileControlMonitor::isControlRequested(void)
{
    bool oldPaused = m_paused;
    m_paused = (g_sharedObj != MAP_FAILED) ? (g_sharedObj->paused) : m_paused;
    return oldPaused != m_paused;
}

int ProfileControlMonitor::entryPoint()
{
    gtUInt32 clientId = helpGetClientId();

    while (isProfilingNow(clientId))
    {
        // if state toggled, process the new request
        if (isControlRequested())
        {
            if (m_paused)
            {
                // pause profiling
                CpuPerfPauseProfiling(nullptr);
            }
            else
            {
                // resume profiling
                CpuPerfResumeProfiling(nullptr);
            }
        }
    }

    return 0;
}

/// Thread to monitor Profile Control pause/resume request
ProfileControlMonitor* g_pcMonitor = nullptr;


//
// Helper functions
//


// This helper function returns the g_pProfileSession object;
// !!! pauseKey is UNUSED in Linux as of now. !!!
// In Linux PERF, you can NOT pause/resume a profiling session from another
// application; Hence there is NO need for this pauseKey in Linux.
gtUInt32 helpGetPauseKeyClientId(const wchar_t* pauseKey __attribute__((unused)))
{
    // just return the clientId;
    gtUInt32 clientId = helpGetClientId();

    return clientId;
}


//This function will test whether the application(codeanalyst) is currently profiling
//If the application is currently profiling, this will return true;
bool isProfilingNow(gtUInt32 clientId)
{
    // In Linux PERF, multiple clients can profile the same process
    // What about ?
    //         SWP Vs SWP
    //         Per-Process Vs SWP
    //        TODO: Is there a way to querry PERF to check whether profiling is on currently ?
    //
    bool bRet = false;

    if (Profiling == g_profilingState[clientId] || ProfilingPaused == g_profilingState[clientId])
    {
        bRet = true;
    }

    return bRet;
}


// Returns the coremask of the active processors
HRESULT GetSystemCoreMask(gtUInt64* pCoreMask, gtUInt32* pNumCpus)
{
    gtUInt64 coremask = 0;

    if (!pCoreMask || !pNumCpus)
    {
        return E_INVALIDARG;
    }

    long numCpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (0 == numCpus)
    {
        return E_UNEXPECTED;
    }

    for (long u = 0; u < numCpus; u++)
    {
        coremask += 1LL << u;
    }

    if (pNumCpus)
    {
        *pNumCpus = numCpus;
    }

    if (pCoreMask)
    {
        *pCoreMask = coremask;
    }

    return S_OK;
}


// Returns the coremask of the active processors;
// This should support more than 64 processors
HRESULT GetActiveCoreMask(gtUInt64** pCoreMask, gtUInt32* pCoreMaskSize, gtUInt32* pCoreMaskCount)
{
    if (!pCoreMask || !pCoreMaskCount || !pCoreMaskSize)
    {
        return E_INVALIDARG;
    }

    long numCpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (0 == numCpus)
    {
        return E_UNEXPECTED;
    }

    // alloc memory
    gtUInt32 maskSize = static_cast<gtUInt32>((numCpus - 1) / 64) + 1;
    gtUInt64* pTmp  = (gtUInt64*)malloc(sizeof(gtUInt64) * maskSize);

    if (NULL == pTmp)
    {
        return E_UNEXPECTED;
    }

    memset(pTmp, 0, sizeof(gtUInt64) * maskSize);

    gtUInt32 coreMaskCount = 0;

    for (long u = 0; u < numCpus; u++)
    {
        gtUInt32 maskIndex = u / 64;
        gtUInt32 maskOffset = u % 64;

        pTmp[maskIndex] += 1ULL << maskOffset;
        coreMaskCount++;
    }

    *pCoreMask      = pTmp;
    *pCoreMaskSize  = maskSize;
    *pCoreMaskCount = coreMaskCount;

    return S_OK;
}


gtUInt64 getPerfSampleAttr(bool is_multiplexing)
{
    if (false == g_usePERF)
    {
        return 0;
    }

    gtUInt64 sampleAttr = static_cast<gtUInt64>(PERF_SAMPLE_IP | PERF_SAMPLE_TID  | PERF_SAMPLE_TIME | PERF_SAMPLE_ID);

    // Baskar: Only if multiplexing is enabled, set PERF_SAMPLE_READ
    if (is_multiplexing)
    {
        sampleAttr |= static_cast<gtUInt64>(PERF_SAMPLE_READ);
    }

    if (g_isSWP)
    {
        sampleAttr |= static_cast<gtUInt64>(PERF_SAMPLE_CPU);
    }

    if (g_callStack)
    {
        sampleAttr |= static_cast<gtUInt64>(PERF_SAMPLE_CALLCHAIN);
    }

    if (g_isIbs)
    {
        // for IBS, we need to set PERF_SAMPLE_RAW (FIXME: also PERF_SAMPLE_CPU ?)
        sampleAttr |= static_cast<gtUInt64>(PERF_SAMPLE_RAW);
    }

    return sampleAttr;
}


gtUInt64 getPerfReadFormat()
{
    gtUInt64 rdFormat = 0;

    if (g_usePERF)
    {
        rdFormat = static_cast<gtUInt64>(PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_ID);
    }

    return rdFormat;
}


gtUInt32 getPerfPmcFlags(bool excludeUser, bool excludeKernel)
{
    if (false == g_usePERF)
    {
        return 0;
    }

    gtUInt32 pmuFlags = (gtUInt32)(PerfEvent::PERF_PMU_FLAG_INCLUDE_MMAP_DATA
                                   | PerfEvent::PERF_PMU_FLAG_INCLUDE_COMM_DATA);

    // Baskar: BUG379336: IBS is not working in Linux kernel version 3.5.5+
    // After digging into kernel source (arch/x86/kernel/cpu/perf_event_amd_ibs.c)
    // found out that, to enable IBS profiling, kernel version 3.5.5+ expects
    // certain flags in "perf_event_attr" structure to be set to 0. The flags
    // are - exclude_user, exclude_kernel, exclude_hv, exclude_idle,
    // exclude_host & exclude_guest.
    if (! g_isIbs)
    {
        pmuFlags |= (gtUInt32)(PerfEvent::PERF_PMU_FLAG_EXCLUDE_IDLE);

        // If EBP performed within Linux guest OS, it leads to crash in VMware driver on host
        // Exclude HYPERVISOR only on host OS
        osCpuid cpuid;

        if (!cpuid.hasHypervisor())
        {
            pmuFlags |= (gtUInt32)(PerfEvent::PERF_PMU_FLAG_EXCLUDE_HYPERVISOR);
        }
    }

    if (excludeUser)
    {
        pmuFlags = ((gtUInt32)pmuFlags | (gtUInt32)(PerfEvent::PERF_PMU_FLAG_EXCLUDE_USER));
    }

    if (excludeKernel)
    {
        pmuFlags = ((gtUInt32)pmuFlags | (gtUInt32)(PerfEvent::PERF_PMU_FLAG_EXCLUDE_KERNEL));
    }

    return pmuFlags;
}


gtUInt32 getPerfTaskFlags()
{
    gtUInt32 taskFlags = 0;

    // NOTE: Inherit is not allowed on per-task event
    // If we set enable_on_exec, "profile paused" case on CodeXL won't work;
    // enable_on_exec doesn't have to be set, since we explicilty enable
    // the profiling in fnStartProfiling
    if (g_usePERF && !g_isSWP)
    {
        taskFlags = (gtUInt32)(PerfEvent::PERF_TASK_FLAG_INHERIT);
    }

    return taskFlags;
}


// just a linear search - n is small and hence ok
static bool isCoreSet(int core)
{
    for (size_t i = 0; i < g_numCpus; i++)
    {
        if (core == g_pCpus[i])
        {
            return true;
        }
    }

    return false;
}


HRESULT addCore(unsigned int core)
{
    if (NULL == g_pCpus)
    {
        g_pCpuSize = 32;
        g_pCpus = (int*)malloc(sizeof(int) * g_pCpuSize);

        if (NULL == g_pCpus)
        {
            return E_UNEXPECTED;
        }
    }

    if ((g_numCpus + 1) > g_pCpuSize)
    {
        g_pCpuSize += 32;

        int* pCpusOrig = g_pCpus;
        g_pCpus = (int*)realloc(pCpusOrig, sizeof(int) * g_pCpuSize);

        if (NULL == g_pCpus)
        {
            free(pCpusOrig);
            return E_UNEXPECTED;
        }
    }

    // check if the cpu is set already; Linear search - n is small and hence ok.
    if (isCoreSet(core))
    {
        return S_OK;
    }

    g_pCpus[g_numCpus++] = core;

    return S_OK;
}


int getCoreCount(const gtUInt64* pCoreMask, int coreMaskSize)
{
    if ((NULL == pCoreMask) || (0 == coreMaskSize))
    {
        return -1;
    }

    gtUInt32 count = 0;

    for (int i = 0; i < coreMaskSize; i++)
    {
        gtUInt64 value = pCoreMask[i];

        while (value)
        {
            count += value & 0x1;
            value >>= 1;
        }
    }

    return count;
}


HRESULT addCoreMask(const gtUInt64* pCoreMask, int coreMaskSize)
{
    if ((NULL == pCoreMask) || (0 == coreMaskSize))
    {
        return E_UNEXPECTED;
    }

    int coreCount = getCoreCount(pCoreMask, coreMaskSize);

    if (coreCount <= 0)
    {
        return E_UNEXPECTED;
    }

    if (NULL == g_pCpus)
    {
        g_pCpuSize = coreCount;
        g_pCpus = (int*)malloc(sizeof(int) * g_pCpuSize);

        if (NULL == g_pCpus)
        {
            return E_UNEXPECTED;
        }
    }
    else if ((g_numCpus + coreCount) > g_pCpuSize)
    {
        g_pCpuSize += coreCount;

        int* pCpusOrig = g_pCpus;
        g_pCpus = (int*)realloc(pCpusOrig, (sizeof(int) * g_pCpuSize));

        if (NULL == g_pCpus)
        {
            free(pCpusOrig);
            return E_UNEXPECTED;
        }
    }

    for (int i = 0; i < coreMaskSize; i++)
    {
        gtUInt64 value = pCoreMask[i];
        int core = 0;

        while (value)
        {
            if (value & 0x1)
            {
                int tcore = core + (i * 64);

                if (!isCoreSet(tcore))
                {
                    g_pCpus[g_numCpus++] = tcore;
                }
            }

            value >>= 1;
            core++;
        }
    }

    return S_OK;
}


void CpuPerfCleanUpConfiguration(void)
{
    if (NULL != g_pProfiler)
    {
        delete g_pProfiler;
        g_pProfiler =  NULL;
    }

    if (NULL != g_pProfileConfig)
    {
        g_pProfileConfig->clear();
    }

    if (NULL != g_pTarget)
    {
        delete g_pTarget;
        g_pTarget =  NULL;
    }

    if (NULL != g_pCpus)
    {
        free(g_pCpus);
        g_pCpus = NULL;
        g_numCpus = 0;
        g_pCpuSize = 0;
    }

    if (NULL != g_pPidList)
    {
        g_pPidList->clear();
        delete g_pPidList;
        g_pPidList = NULL;
    }

    if (NULL != g_pProfileSession)
    {
        g_pProfileSession->clear();
    }

    return;
}


static HRESULT _addTargetPids(
    /*in*/ unsigned int count,
    /*in*/ unsigned int* pProcessIds)
{
    if (0 == count || !pProcessIds)
    {
        return E_INVALIDARG;
    }

    if (NULL == g_pPidList)
    {
        g_pPidList = new gtList<pid_t>;

        if (NULL == g_pPidList)
        {
            return E_UNEXPECTED;
        }
    }

    for (size_t j = 0; j < count; j++)
    {
        g_pPidList->push_back(pProcessIds[j]);
    }

    g_pPidList->unique();

    return S_OK;
}


// Shared object name is concatenation of
// "/amd_cxl_cpuprof_" and pid of cpu profiler
bool generateSharedObjName(char* name)
{
    if (nullptr != name)
    {
        pid_t pid = getpid();
        sprintf(name, "/amd_cxl_cpuprof_%d", pid);
    }

    return true;
}

HRESULT initializeSharedObj(void)
{
    HRESULT hr = S_OK;
    int oflags = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    char name[NAME_MAX] = {0};
    size_t size = CPU_PROF_SHARED_MEM_SIZE;

    // Generate an unique name for new shared object
    generateSharedObjName(name);

    // Try to open/create the shared object
    g_shm_fd = shm_open(name, oflags, mode);

    if (g_shm_fd >= 0 && 0 != ftruncate(g_shm_fd, size))
    {
        // Unable to truncate (set size) the shared object
        shm_unlink(name);
        g_shm_fd = -1;
        hr = E_FAIL;
    }

    if (g_shm_fd >= 0)
    {
        int protection = PROT_READ | PROT_WRITE;
        int mflags = MAP_SHARED;
        off_t offset = 0;

        g_sharedObj = (SHARED_CLIENT*)mmap(NULL, size, protection, mflags, g_shm_fd, offset);

        if (MAP_FAILED == g_sharedObj)
        {
            shm_unlink(name);
            g_shm_fd = -1;
            hr = E_FAIL;
        }
        else
        {
            g_sharedObj->paused = false;
            g_sharedObj->clientPid = getpid();
        }
    }

    return hr;
}

void releaseSharedObj(void)
{
    if (MAP_FAILED != g_sharedObj)
    {
        munmap(g_sharedObj, CPU_PROF_SHARED_MEM_SIZE);
        g_sharedObj = (SHARED_CLIENT*)MAP_FAILED;
    }

    if (g_shm_fd >= 0)
    {
        char name[NAME_MAX] = {0};
        generateSharedObjName(name);
        shm_unlink(name);
        g_shm_fd = -1;
    }
}


void CpuPerfCleanUp(void)
{
    // Clean all the objects;
    CpuPerfCleanUpConfiguration();

    if (NULL != g_pProfileSession)
    {
        delete g_pProfileSession;
        g_pProfileSession =  NULL;
    }

    if (NULL != g_pProfileConfig)
    {
        delete g_pProfileConfig;
        g_pProfileConfig =  NULL;
    }

    if (NULL != g_pEvent)
    {
        delete g_pEvent;
        g_pEvent =  NULL;
    }
}


//
// Linux Specific API implementation
//

void __attribute__((constructor)) initCpuProfileControl(void);
void __attribute__((destructor)) finiCpuProfileControl(void);

// init/constructor routine which gets executed when the library gets loaded
void __attribute__((constructor)) initCpuProfileControl(void)
{
    HRESULT hr = S_OK;
    gtUInt32 clientId = 0;
    g_clientId[clientId] = getpid();
    g_pPidList = NULL;

    hr = GetSystemCoreMask(&g_SystemCoreMask, &g_maxCore);

    if ((S_OK != hr) || (0 == g_maxCore) || (0 == g_SystemCoreMask))
    {
        // Log the error somewhere ?
        g_validSystemCoreMask = false;
    }

    // Support for 64+ core
    hr = GetActiveCoreMask(&g_pSystemCoreMask,
                           &g_SystemCoreMaskSize,
                           &g_SystemCoreMaskCount);

    if (S_OK != hr)
    {
        // Log the error somewhere ?
        g_validSystemCoreMask = false;
    }

    // Initialize the g_eventsFile
#ifdef CODEXL_OSWRAPPERS_AVBL
    gtUInt32 family = 0;
    gtUInt32 model = 0;
    // in helperAPI.cpp, helpGetEventFile() is defined; use it to get the eventFile Path
    osCpuid cpuid;
    family = cpuid.getFamily();
    model = cpuid.getModel();
#endif // CODEXL_OSWRAPPERS_AVBL

#ifdef CODEXL_LIBCPUPERFEVENT_AVBL
    // in helperAPI.cpp, helpGetEventFile() is defined; use it to get the eventFile Path
    gtString gtEventFile = g_eventEngine.GetEventFilePath(family, model);
    QString eventFile = QString::fromWCharArray(gtEventFile.asCharArray());

    if (!eventFile.isEmpty())
    {
        g_eventsFile.Open(eventFile);
    }

    fnGetEventCounters(&g_pmcAvailable);
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

    g_profilingState[clientId] = ProfilingUnavailable;
    g_profileType[clientId] = PROFILE_TYPE_NOT_CONFIGURED;

    initializeSharedObj();
}


// fini/destructor routine which gets executed when the library gets unloaded
void __attribute__((destructor)) finiCpuProfileControl(void)
{
    CpuPerfCleanUp();

#ifdef CODEXL_LIBCPUPERFEVENT_AVBL
    g_eventsFile.Close()
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

    releaseSharedObj();
}


HRESULT CpuPerfUsePERF()
{
    HRESULT hr = S_OK;

    g_usePERF = true;

    return hr;
}


HRESULT CpuPerfUseOProfile()
{
    HRESULT hr = S_OK;

    g_usePERF = false;

    return hr;
}


HRESULT CpuPerfEnableProfiling()
{
    return CpuPerfEnableProfiling(g_usePERF);
}


HRESULT CpuPerfEnableProfiling(bool usePerf)
{
    HRESULT hr = S_OK;

    osCpuid cpuid;

    if (true == cpuid.hasHypervisor() && false == cpuid.isSupportedHypervisor())
    {
        g_invalidClientErr = L"An unsupported hypervisor has been detected. "
                             L"The CodeXL CPU Profiling driver does not support running on "
                             L"this hypervisor guest operating system. CPU Profiling is unavailable.";
        return E_NOTSUPPORTED;
    }

    if (usePerf)
    {
        hr = CpuPerfUsePERF();
    }
    else
    {
        hr = CpuPerfUseOProfile();
    }

    if (S_OK != hr)
    {
        return hr;
    }

    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        return S_FALSE;
    }

    // Checking the profile state
    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"The profiler is already in use";
        return S_FALSE;
    }

    // Create all the required objects
    g_pProfileSession = new PerfPmuSession;
    g_pProfileConfig = new PerfConfig;
    // g_pEvent = new PerfEvent;        // UNUSED

    // Set the profilingstate
    g_profilingState[clientId] = ProfilingStopped;

    // Set the profile type
    g_profileType[clientId] = PROFILE_TYPE_NOT_CONFIGURED;

    g_clientId[clientId] = getpid();

    fnGetEventCounters((unsigned int*)&g_maxEventConfigs);

    // Multiplexing is supported in PERF:
    // TBD: What is it the scale factor - 8?
    gtUInt32 multiplexScale = 8;
    g_maxEventConfigs *= multiplexScale;

    return hr;
}


HRESULT CpuPerfReleaseProfiling()
{
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (nullptr != g_pcMonitor)
    {
        g_pcMonitor->terminate();
        delete(g_pcMonitor);
        g_pcMonitor = nullptr;
    }

    if (ProfilingStopped != g_profilingState[clientId])
    {
        fnStopProfiling();
        hr = S_FALSE;
    }

    CpuPerfClearConfigurations(clientId);
    g_maxEventConfigs = 0;

    // Reset the global variablesl
    g_profilingState[clientId] = ProfilingUnavailable;
    g_profileType[clientId] = PROFILE_TYPE_NOT_CONFIGURED;

    // Reset all the globals
    g_isSWP = true;
    g_callStack = false;
    g_isIbs = false;

    return hr;
}


HRESULT CpuPerfGetDriverVersion(unsigned int* pMajor, unsigned int* pMinor, unsigned int* pBuild)
{
    if (g_usePERF)
    {
        // TODO: PERF's version or OS Version?
        *pMajor = 1;
        *pMinor = 0;
        *pBuild = 1;
    }
    else
    {
        // TODO: OProfile driver version
        *pMajor = 1;
        *pMinor = 0;
        *pBuild = 1;
    }

    return S_OK;
}


HRESULT CpuPerfMakeProfileEvent(/*in*/ unsigned int eventSelect,
                                       /*in*/ unsigned int unitMask,
                                       /*in*/ bool edgeDetect,
                                       /*in*/ bool usrEvents,
                                       /*in*/ bool osEvents,
                                       /*in*/ bool guestOnlyEvents,
                                       /*in*/ bool hostOnlyEvents,
                                       /*in*/ bool countingEvent,
                                       /*out*/ gtUInt64* pPerformanceEvent)
{
    PERF_CTL oneEvent;
    oneEvent.perf_ctl = 0;

    //guest & host bits
    if (guestOnlyEvents)
    {
        oneEvent.guestOnly = 1;
    }

    if (hostOnlyEvents)
    {
        oneEvent.hostOnly = 1;
    }

    oneEvent.Reserved = 0;
    oneEvent.ucCounterMask = 0;
    oneEvent.ucEventSelect = (eventSelect & 0xFF);
    oneEvent.ucEventSelectHigh = ((eventSelect >> 8) & 0x0F);

    oneEvent.ucUnitMask = unitMask;
    oneEvent.bitEnabled = 1;

    if (!countingEvent)
    {
        oneEvent.bitSampleEvents = 1;
    }

    if (edgeDetect)
    {
        oneEvent.bitEdgeEvents = 1;
    }

    if (osEvents)
    {
        oneEvent.bitOsEvents = 1;
    }

    if (usrEvents)
    {
        oneEvent.bitUsrEvents = 1;
    }

    *pPerformanceEvent = oneEvent.perf_ctl;

    return S_OK;
}


// This function retrieves the availability of event counters per core.
//        !!!        Not Required for Linux PERF        !!!
HRESULT CpuPerfGetEventCounterAvailability(
    /*in*/ gtUInt64 performanceEvent,
    /*out*/ unsigned int* pAvailabilityMask,
    /*in*/  PmcType type)
{
    (void)(pAvailabilityMask); // unused
    (void)(type); // unused
    // TODO: Handle PmcType - PMC_CORE, PMC_NORTHBRIDGE, PMC_L2I
#ifdef CODEXL_LIBCPUPERFEVENT_AVBL
    PERF_CTL oneEvent;
    oneEvent.perf_ctl = performanceEvent;
    gtUInt32 clientId = helpGetClientId();

    //counter event has to be set
    unsigned int evSelect = GetEvent12BitSelect(oneEvent);

    CpuEvent eventData;

    if (!g_eventsFile.FindEventByValue(evSelect, eventData))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event (0x%x) was not valid",
                 performanceEvent);

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = buffer;
        }

        return E_INVALIDARG;
    }

#else
    (void)(performanceEvent); // unused
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

    // In PERF, we don't have to worry about the pmu counters available; Not sure about
    // Oprofile
    if (g_usePERF)
    {
        return E_NOTSUPPORTED;
    }
    else
    {
        return E_NOTIMPL;
    }

    return S_OK;
}


HRESULT CpuPerfGetProfilerState(
    /*out*/ ProfileState* pProfileState)
{
    gtUInt32 clientId = helpGetClientId();

    // TODO: quick check to see if the profile was aborted
    //        How to check profiling is paused ?

    *pProfileState = g_profilingState[clientId];

    return S_OK;
}


HRESULT CpuPerfStartProfiling(
    /*in*/ bool            startPaused,
    /*in*/ bool            pauseIndefinite,
    /*in*/ const wchar_t*  pauseKey,    // UNUSED in Linux PERF
    /*out*/ ProfileState*  pProfileState)
{
    GT_UNREFERENCED_PARAMETER(pauseKey);
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"The profiler is already in use";
        return S_FALSE;
    }

    if (ProfilingStopped != g_profilingState[clientId])
    {
        g_errorString[clientId] = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    bool sampling = CPU_PERF_SAMPLING_MODE(clientId);

    // In Linux, even the counter values can be written to caperf.data file;
    // TBD: Should we have output file only for sampling mode
    if (sampling)
    {
        //verify output file
        if ('\0' == g_outputFile[0])
        {
            g_errorString[clientId] = L"fnSetProfileOutputFile has not been called";
            return E_PENDING;
        }
    }

    // Set the sampl attributes
    if (g_usePERF)
    {
        // create the profiler and target object;
        g_pProfiler = new CaPerfProfiler;
        g_pTarget = new PerfPmuTarget;

        //----------------------------
        // set the target

        pid_t* pPids = NULL;
        size_t nbrPids = 0;

        if (NULL != g_pPidList)
        {
            nbrPids = g_pPidList->size();
            pPids = (pid_t*) calloc(nbrPids, sizeof(pid_t));

            if (NULL == pPids)
            {
                return E_UNEXPECTED;
            }

            int i = 0;

            for (gtList<pid_t>::iterator it = g_pPidList->begin(), itEnd = g_pPidList->end(); it != itEnd; ++it)
            {
                pPids[i] = *it;
                i++;
            }
        }

        g_pTarget->setPids(nbrPids, pPids, g_numCpus, g_pCpus, g_isSWP);

        if (NULL != pPids)
        {
            free(pPids);
            pPids = NULL;
        }

        // Baskar: BUG361568:  [CPU P] CXL shows very huge number of timer samples for launch app
        // Multiplexing will be enabled by PERF if the number-of-sampling events (EBP) are greater than
        // the number of number of PMCs available.
        //
        // Multiplexing should be enabled only if EBP events exceeds the Max-PMC-available. And
        // when multiplexing is enabled, set the following flag.
        //     PERF_SAMPLE_READ in perf_event_attr.sample_type field
        //
        // Note: Multiplexing should *not* be enabled for TBP & IBS. Can be enabled for custom-profile
        //       only if EBP events > max-PMCs
        //
        unsigned int maxCounters = 0;
        fnGetEventCounters(&maxCounters);
        bool isMultiplexing = (gNumEbpEvents > maxCounters) ? true : false;

        //----------------------------
        // set the sample-attributes in PerfConfig object
        gtUInt64 sampleAttr = getPerfSampleAttr(isMultiplexing);
        HRESULT ret = g_pProfileConfig->setSampleAttribute(sampleAttr);

        if (S_OK != ret)
        {
            return E_INVALIDARG;
        }

        // Set the read format;
        g_pProfileConfig->setReadFormat(getPerfReadFormat());

        // Add the sampling events
        if (0 == gSamplingEventList.size())
        {
            g_errorString[clientId] = L"Profiling Configuration has not been set";
            return E_PENDING;
        }

        // Get task flags
        // Note: This must be done at the moment we start the profile.
        gtUInt32 taskFlags = getPerfTaskFlags();

#ifdef ENABLE_FAKETIMER

        // Baskar: BUG365178
        // The fake-timer logic will only work for the predefined TBP profile.
        // A custom cpu profile with "assess performance" and "TBP" together can throttle
        // the system. Also, the data-translation phase could handle the fake-timer event
        // records generated by the predefined TBP profile. Hence we should not use
        // fake-timer events in custom profiles.
        //
        // Hack to ensure the fake-timer event, is not configured for any custom based events.
        // If this is only the TBP run, then add the fake timer event to gSamplingEventList
        //
        // Baskar: 13/01/2013: Fake timer samples are required only for CSS sampling in TBP mode.
        //
        if ((CPU_PERF_TBP_ONLY_MODE(clientId))
            && g_callStack
            && (g_timerHackNsPeriod > 0))
        {
            gSamplingEventList.push_back(std::pair<PerfEvent, gtUInt64>(g_fakeTimerEvent, g_timerHackNsPeriod));
        }
        else
        {
            g_timerHackIndex = -1;
            g_timerHackNsPeriod = 0;
        }

#endif // ENABLE_FAKETIMER

        gtList< std::pair<PerfEvent, gtUInt64> >::iterator iter;

        for (iter = gSamplingEventList.begin(); iter != gSamplingEventList.end(); ++iter)
        {
            (*iter).first.setTaskFlags(taskFlags);
            g_pProfileConfig->addSamplingEvent((*iter).first, (*iter).second);
        }

#ifdef ENABLE_FAKETIMER

        if (-1 != g_timerHackIndex)
        {
            //Let the CaPerf config know that it will need to resolve data for the SW timer bug
            g_pProfileConfig->setTimerHackInfo(g_timerHackIndex, g_timerHackNsPeriod);
        }

#endif

        // clear the gSamplingEventList
        if (gSamplingEventList.size())
        {
            gSamplingEventList.clear();
        }

        // initialize the PMU Session/profiler
        ret = g_pProfileSession->initialize(g_pProfiler, g_pProfileConfig, g_pTarget);

        if (S_OK != ret)
        {
            std::string tmp = g_pProfileSession->getErrStr();
            std::wstring wtmp(tmp.begin(), tmp.end());
            g_errorString[clientId] = wtmp.c_str();
            return E_UNEXPECTED;
        }

        ret = g_pProfileSession->startProfile(false);

        if (S_OK != ret)
        {
            std::string tmp = g_pProfileSession->getErrStr();
            std::wstring wtmp(tmp.begin(), tmp.end());
            g_errorString[clientId] = wtmp.c_str();
            return E_UNEXPECTED;
        }

        // Enable the profiling only if startPaused is false
        if (! startPaused)
        {
            ret = g_pProfileSession->enableProfile();

            if (S_OK != ret)
            {
                g_errorString[clientId] = L"Error in profileSession::enableProfile";
                return E_UNEXPECTED;
            }

            g_profilingState[clientId] = Profiling;
        }
        else
        {
            g_profilingState[clientId] = ProfilingPaused;
            g_sharedObj->paused = true;

            if (pauseIndefinite)
            {
                g_pcMonitor = new ProfileControlMonitor(getpid(), true);

                if (nullptr != g_pcMonitor)
                {
                    g_pcMonitor->execute();
                }
            }
        }
    }
    else
    {
        return E_NOTIMPL;
    }

    if (NULL != pProfileState)
    {
        *pProfileState = g_profilingState[clientId];
    }

    return hr;
}


/** This will provide the profile control key for the pauseKey.  The profile
    control key is used for /ref fnPauseProfiling and /ref fnResumeProfiling

    \ingroup profiling
    @param[in] pauseKey If you want to pause a profile from another
    application, provide the same string as was provided to \ref
    fnStartProfiling to control the profile.  The pause key    used by the
    CodeAnalyst Gui and CaProfile profiles is \ref CPU_PROFILE_PAUSE_KEY.
    \return The profile control key
    \retval NULL There was no active profile with the pause key
*/
CaProfileCtrlKey CpuPerfGetProfileControlKey(const wchar_t* pauseKey)
{
    CaProfileCtrlKey profileKey = NULL;
    gtUInt32 clientId;

    if (NULL != pauseKey)
    {
        clientId = helpGetPauseKeyClientId(pauseKey);
    }
    else
    {
        clientId = helpGetClientId();
    }

    if (INVALID_CLIENT != clientId)
    {
        profileKey = (CaProfileCtrlKey) g_pProfileSession;
    }

    return profileKey;
}


HRESULT CpuPerfPauseProfiling(CaProfileCtrlKey profileKey)
{
    (void)(profileKey); // unused
    // !!!    profileKey is UNUSED in Linux PERF;    !!!
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    // Check if the profilingState is in Profiling state
    if (ProfilingPaused == g_profilingState[clientId])
    {
        g_errorString[clientId] = L"The profiling is already in paused state";
        return S_FALSE;
    }

    // Check if the profilingState is in Profiling state
    if (Profiling != g_profilingState[clientId])
    {
        g_errorString[clientId] = L"fnStartProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (g_usePERF)
    {
        // Pause the profiling; In Linux PERF there is no seperate PaUsE PrOfIlInG.
        HRESULT ret = g_pProfileSession->disableProfile();

        if (S_OK != ret)
        {
            // TODO: anyother cleanup here ?
            g_profilingState[clientId] = ProfilingUnavailable;
            return E_UNEXPECTED;
        }
    }
    else
    {
        // Oprofile
        return E_NOTIMPL;
    }

    g_profilingState[clientId] = ProfilingPaused;
    return S_OK;
}


HRESULT CpuPerfResumeProfiling(CaProfileCtrlKey profileKey)
{
    (void)(profileKey); // unused
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (Profiling == g_profilingState[clientId])
    {
        g_errorString[clientId] = L"The profiler is not in paused state";
        return S_FALSE;
    }

    if (ProfilingPaused != g_profilingState[clientId])
    {
        g_errorString[clientId] = L"The profiler was not ready to resume";
        return E_ACCESSDENIED;
    }

    if (g_usePERF)
    {
        HRESULT ret = g_pProfileSession->enableProfile();

        if (S_OK != ret)
        {
            // TODO: anyother cleanup here ?
            g_profilingState[clientId] = ProfilingUnavailable;
            return E_UNEXPECTED;
        }
    }
    else
    {
        // Oprofile
        return E_NOTIMPL;
    }

    g_profilingState[clientId] = Profiling;

    return hr;
}


HRESULT CpuPerfStopSamplingProfile(gtUInt32 clientId)
{
    HRESULT hr = S_OK;

    if ((Profiling != g_profilingState[clientId])
        && (ProfilingPaused != g_profilingState[clientId]))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"The profiler was not ready to stop";
        }

        return E_ACCESSDENIED;
    }

    if (g_usePERF)
    {
        // Stop the profiling
        HRESULT ret = g_pProfileSession->stopProfile();

        if (S_OK != ret)
        {
            // TODO: anyother cleanup here ?
            g_profilingState[clientId] = ProfilingUnavailable;
            return E_UNEXPECTED;
        }
    }
    else
    {
        // Oprofile
        return E_NOTIMPL;
    }

    g_profilingState[clientId] = ProfilingStopped;

    return hr;
}


HRESULT CpuPerfStopCPUUtilMonitoring(gtUInt32 clientId)
{
    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"The profiler was not ready to stop";
        return E_ACCESSDENIED;
    }

    // The CPU Utilization is not yet supported;
    return E_NOTIMPL;
}


// This API will be used to set the HW PMC Events
HRESULT CpuPerfSetCountingEvent(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex, // UNUSED in Linux PERF
    /*in*/ EventConfiguration performanceEvent)
{
    (void)(eventCounterIndex); // unused
    HRESULT hr = S_OK;

    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    PERF_CTL oneEvent;
    oneEvent.perf_ctl = performanceEvent.performanceEvent ;

    //counter event has to be set
    unsigned int evSelect = GetEvent12BitSelect(oneEvent);

    if (1 == oneEvent.bitSampleEvents)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event (0x%x) was not a counting event",
                 evSelect, oneEvent.ucUnitMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

#if CODEXL_LIBCPUPERFEVENT_AVBL

    // libCpuPerfEvent is not yet available in Linux
    // are the event and unit mask valid?
    if (!g_eventsFile.ValidateEvent(evSelect, oneEvent.ucUnitMask))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event (0x%x) and unit mask (0x%x) were not validated", evSelect, oneEvent.ucUnitMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

#endif // CODEXL_LIBCPUPERFEVENT_AVBL

    // Set the event counter here:
    // In Linux PERF, we don't have to worry about setting
    //    - multiple counters or
    //    - counting + sampling
    if (g_usePERF)
    {
        bool excludeKernel = (oneEvent.bitOsEvents) ? false : true;
        bool excludeUser = (oneEvent.bitUsrEvents) ? false : true;

        gtUInt32 pmcFlags = getPerfPmcFlags(excludeUser, excludeKernel);

        // set the PerfEvent in PerfConfig
        gtUInt32 eventId = GetEvent12BitSelect(oneEvent);
        gtUInt32 umask = oneEvent.ucUnitMask;

        PerfEvent aEvent;
        aEvent.initialize(eventId,
                          umask,
                          pmcFlags,
                          0);

        (void) g_pProfileConfig->addCounterEvent(aEvent);
        // TODO: Error checking

        hr = addCore(core);

        if (S_OK != hr)
        {
            g_errorString[clientId] = L"Unexpected error while setting the core";
            return E_UNEXPECTED;
        }
    }
    else
    {
        // Oprofile Does NOT support Counting mode
        return E_NOTSUPPORTED;
    }

    g_profileType[clientId] |= PROFILE_TYPE_EVENT_COUNTING;

    return hr;
}


// This returns the value of the counting event value
// FIXME:
//        core --> UNUSED in Linux PERF ??
//        eventCounterIndex --> UNUSED in Linux PERF
//    Only the eventConfiguration is required for Linux PERF..
//    should we replace eventCounterIndex with eventConfiguration for Linux.
//
HRESULT CpuPerfGetEventCount(/*in*/ unsigned int core,
                                    /*in*/ unsigned int eventCounterIndex,
                                    /*out*/ gtUInt64* pEventCount)
{
    (void)(core); // unused
    (void)(pEventCount); // unused
    gtUInt32 clientId = helpGetClientId();

    if (NULL == g_pProfileSession)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);

    if (eventCounterIndex >= maxCounters)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The counter index (%d) was not valid, the max is %d",
                 eventCounterIndex, maxCounters - 1);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    bool counting = CPU_PERF_COUNTING_MODE(clientId);

    if (! counting)
    {
        g_errorString[clientId] = L"The profiler was not configured for counting";
        return E_FAIL;
    }

    if ((Profiling != g_profilingState[clientId])
        && (ProfilingPaused != g_profilingState[clientId]))
    {
        g_errorString[clientId] = L"The profiler was not currently profiling";
        return E_PENDING;
    }

    // TBD:
    // From eventCounterIndex, we cannot retrieve the counter values in PERF.

    return E_NOTIMPL;
}


HRESULT CpuPerfSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ bool profileAllCores)
{
    HRESULT hr = S_OK;

    gtUInt32 clientId = helpGetClientId();

    hr = CpuPerfSetCountingConfiguration(clientId,
                                         pPerformanceEvents,
                                         count,
                                         &cpuCoreMask,
                                         1,
                                         profileAllCores);

    return hr;
}


HRESULT CpuPerfSetCountingConfiguration(
    /*in*/ gtUInt32                     clientId,
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int                 count,
    /*in*/ const gtUInt64*                pCpuCoreMask,
    /*in*/ unsigned int                 cpuCoreMaskSize,
    /*in*/ bool                         profileAllCores)
{
    HRESULT hr = S_OK;

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);

    for (unsigned int i = 0; i < count; i++)
    {
        //transform the event select from the event
        PERF_CTL oneEvent;
        oneEvent.perf_ctl = pPerformanceEvents[i].performanceEvent;
        unsigned int evSelect = GetEvent12BitSelect(oneEvent);

        //counter event has to be set
        if (1 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            swprintf(buffer, 75, L"Event 0x%x was not configured for counting",
                     evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

#if CODEXL_LIBCPUPERFEVENT_AVBL
        // LibCpuPerfEvent is not yet availabe in Linux;

        //are the event and unit mask valid?
        if (!g_eventsFile.ValidateEvent(evSelect, oneEvent.ucUnitMask))
        {
            wchar_t buffer[65];
            swprintf(buffer, 65, L"The event (0x%x) and unit mask (0x%x) were not validated",
                     evSelect, oneEvent.ucUnitMask);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

#endif // CODEXL_LIBCPUPERFEVENT_AVBL
    }

    // TODO; Should i check for g_profileType - to see whether it is set already?

    for (unsigned int i = 0; i < count; i++)
    {
        PERF_CTL oneEvent;
        oneEvent.perf_ctl = pPerformanceEvents[i].performanceEvent;
        unsigned int evSelect = GetEvent12BitSelect(oneEvent);

        //does the event have a count? - Not starting count value in Linux PERF
        if (pPerformanceEvents[i].value)
        {
            wchar_t buffer[75];
            swprintf(buffer, 75, L"Event 0x%x had a non 0 value", evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //counter event should not be set
        if (1 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            swprintf(buffer, 75, L"Event 0x%x was configured for sampling (0x%I64x)",
                     evSelect, pPerformanceEvents[i].performanceEvent);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        if (g_usePERF)
        {
            bool excludeKernel = (oneEvent.bitOsEvents) ? false : true;
            bool excludeUser = (oneEvent.bitUsrEvents) ? false : true;

            gtUInt32 pmcFlags = getPerfPmcFlags(excludeUser, excludeKernel);

            // set the PerfEvent in PerfConfig
            PerfEvent aEvent;

            gtUInt32 umask = oneEvent.ucUnitMask;
            gtUInt32 eventId = GetEvent12BitSelect(oneEvent);

            aEvent.initialize(eventId,
                              umask,
                              pmcFlags,
                              0);

            g_pProfileConfig->addCounterEvent(aEvent);

            if (profileAllCores)
            {
                hr = addCoreMask((const gtUInt64*)g_pSystemCoreMask,
                                 g_SystemCoreMaskSize);
            }
            else
            {
                hr = addCoreMask(pCpuCoreMask, cpuCoreMaskSize);
            }

            if (S_OK != hr)
            {
                g_errorString[clientId] = L"Unexpected error while setting the core";
                return E_UNEXPECTED;
            }
        }
        else
        {
            // OProfile
            return E_NOTSUPPORTED;
        }
    }

    g_profileType[clientId] |= PROFILE_TYPE_EVENT_COUNTING;

    return S_OK;
}


HRESULT CpuPerfGetCountingEventCount(
    unsigned int core,
    unsigned int* pCount)
{
    (void)(core); // unused
    gtUInt32 clientId = helpGetClientId();
    *pCount = 0;
    //TODO refactor below always evaluates to true!
    if ((g_profileType[clientId] != PROFILE_TYPE_EVENT_COUNTING)
        || (g_profileType[clientId] != PROFILE_TYPE_PERF_EVENT_COUNTING))
    {
        return S_FALSE;
    }

    // retrieve from g_pProfileConfig

    return E_NOTIMPL;
}


HRESULT CpuPerfGetAllEventCounts(
    /*in*/ unsigned int core,
    /*in*/ unsigned int size,
    /*out*/ gtUInt64* pCounts)
{
    (void)(core); // unused
    gtUInt32 clientId = helpGetClientId();

    if (NULL == g_pProfileSession)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    bool counting = CPU_PERF_COUNTING_MODE(clientId);

    if (! counting)
    {
        return S_FALSE;
    }

    memset(pCounts, 0, size * sizeof(gtUInt64));

    // retrieve from g_pProfileConfig

    return E_NOTIMPL;
}


HRESULT CpuPerfSetProfileOutputFile(/*in*/ const wchar_t* pFileName)
{
    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    wcsncpy(g_outputFile, pFileName, (PATH_MAX - 1));

    char outputFile[PATH_MAX] = { 0 };
    wcstombs(outputFile, g_outputFile, PATH_MAX - 1);

    bool overwrite = false;
    g_pProfileSession->setOutputFile(outputFile, overwrite);

    return S_OK;
}


HRESULT CpuPerfSetProfileOutputDirectory(/*in*/ const wchar_t* pDirectoryName)
{
    (void)(pDirectoryName); // unused
    // Yet to be implemented
    return E_NOTIMPL;

}


HRESULT CpuPerfGetCurrentTimeMark(CPA_TIME* pTimeMark)
{
    time_t seconds;

    // seconds since epoch;
    seconds = time(NULL);

    pTimeMark->second = seconds;
    pTimeMark->microsec = 0;

    return S_OK;
}


HRESULT CpuPerfSetTimerConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ bool profileAllCores)
{
    HRESULT hr = S_OK;

    hr = CpuPerfSetTimerConfiguration(puSPeriod,
                                      &cpuCoreMask,
                                      1,
                                      profileAllCores);

    return hr;
}


HRESULT CpuPerfSetTimerConfiguration(
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores)
{
    const unsigned int APIC_TIMER_MINIMUM = 100; // in micro seconds;

    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    HRESULT hr = S_OK;

    if (g_usePERF)
    {
        //verify that period is not too small
        if (*puSPeriod < APIC_TIMER_MINIMUM)
        {
            hr = S_FALSE;
            *puSPeriod = APIC_TIMER_MINIMUM;
        }

        // TBD: exclude-user and exclude-kernel - from where to get these values ?
        gtUInt64 samplingPeriod = *puSPeriod * 1000; //translate to nanoseconds for perf
        bool excludeUser = false;
        bool excludeKernel = true;

        gtUInt32 pmcFlags = getPerfPmcFlags(excludeUser, excludeKernel);

        // set the PerfEvent in PerfConfig
        gtUInt32 eventType = PerfEvent::PERF_PROFILE_TYPE_SW_CPU_CLOCK;
        PerfEvent aEvent;
        aEvent.initialize(eventType,
                          pmcFlags,
                          0);

        // Add this event to gSamplingEventList
        gSamplingEventList.push_back(std::pair<PerfEvent, gtUInt64>(aEvent, samplingPeriod));

        if (profileAllCores)
        {
            hr = addCoreMask((const gtUInt64*)g_pSystemCoreMask,
                             g_SystemCoreMaskSize);
        }
        else
        {
            hr = addCoreMask(pCpuCoreMask, cpuCoreMaskSize);
        }

        if (S_OK != hr)
        {
            g_errorString[clientId] = L"Unexpected error while setting the core";
            return E_UNEXPECTED;
        }

#ifdef ENABLE_FAKETIMER
        //Check to see if we need to fake the SW timer event to get real call stack data
        int major, minor, build;
        osCpuid cpuid;

        //Do we need to handle the bug affecting kernels < 3.0?
        // Enable fake timer only for AMD CPU's

        if ((cpuid.isCpuAmd()) &&
            (osGetOperatingSystemVersionNumber(major, minor, build)) &&
            (major < 3)
           )
        {
            g_timerHackIndex = gSamplingEventList.size() - 1;
            g_timerHackNsPeriod = samplingPeriod;

            int coreCount = getCoreCount((const gtUInt64*)g_pSystemCoreMask,
                                         g_SystemCoreMaskSize);
            gtUInt64 minFreq = ULLONG_MAX;

            for (int c = 0; c < coreCount; c++)
            {
                //If the core min frequency is lower than the current, use it.
                char minFreqBuff[60];
                snprintf(minFreqBuff, 60, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", c);

                //Read the min freq
                FILE* filePtr;
                char buffer[BUFSIZ];
                filePtr = fopen(minFreqBuff, "r");

                if (! filePtr)
                {
                    continue;
                }

                //Get the ascii representation of cycles/sec
                if (fgets(buffer, sizeof(buffer), filePtr) == NULL)
                {
                    fclose(filePtr);
                    continue;
                }

                gtUInt64 tMin = strtoull(buffer, NULL, 0);

                if (filePtr)
                {
                    fclose(filePtr);
                }

                if (tMin < minFreq)
                {
                    minFreq = tMin;
                }
            }

            //Calculate the period of 0.95 * minimum frequencey / timer period to get cycles/sample
            //The 1666 number helps generate approximately twice the samples for the SW timer sample
            samplingPeriod = ((minFreq * 95) / *puSPeriod) * 1666 / 100;

            // save the fake timer event details for later consumption in CpuPerfStartProfiling
            //0x76 = CPU_CLK_UNHALTED
            g_fakeTimerEvent.initialize(0x76, 0, pmcFlags, 0);
        }

#endif
    }
    else
    {
        // Oprofile
        return E_NOTIMPL;
    }

    // Set the profiletype
    g_profileType[clientId] |= PROFILE_TYPE_TIME_SAMPLING;

    return hr;
}


HRESULT CpuPerfSetThreadProfileConfiguration(bool isCSS)
{
    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    HRESULT hr = S_OK;

    if (g_usePERF)
    {
        bool excludeUser = false;
        bool excludeKernel = false;

        gtUInt32 pmcFlags = getPerfPmcFlags(excludeUser, excludeKernel);

        // set the PerfEvent in PerfConfig
        gtUInt32 eventType = PerfEvent::PERF_PROFILE_TYPE_SW_CONTEXT_SWITCHES;
        PerfEvent aEvent;

        aEvent.initialize(eventType,
                          pmcFlags,
                          0);

        // Add this event to gSamplingEventList
        gtUInt64 samplingPeriod = 1;
        gSamplingEventList.push_back(std::pair<PerfEvent, gtUInt64>(aEvent, samplingPeriod));

        // profile all the cores
        hr = addCoreMask((const gtUInt64*)g_pSystemCoreMask,
                         g_SystemCoreMaskSize);

        if (S_OK != hr)
        {
            g_errorString[clientId] = L"Unexpected error while setting the core";
            return E_UNEXPECTED;
        }

        g_callStack = isCSS;
    }
    else
    {
        // Oprofile
        return E_NOTIMPL;
    }

    // Set the profiletype
    g_profileType[clientId] |= PROFILE_TYPE_TIME_SAMPLING;

    return hr;
} // CpuPerfSetThreadProfileConfiguration


HRESULT CpuPerfSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ bool profileAllCores)
{
    HRESULT hr = S_OK;

    hr = CpuPerfSetEventConfiguration(pPerformanceEvents,
                                      count,
                                      &cpuCoreMask,
                                      1,
                                      profileAllCores);

    return hr;
}


HRESULT CpuPerfSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores)
{
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    for (unsigned int i = 0; i < count; i++)
    {
        //transform the event select from the event
        PERF_CTL oneEvent;
        oneEvent.perf_ctl = pPerformanceEvents[i].performanceEvent;
        unsigned int evSelect = GetEvent12BitSelect(oneEvent);

        //does the event have a count?
        if (0 == pPerformanceEvents[i].value)
        {
            wchar_t buffer[75];
            swprintf(buffer, 75, L"Event 0x%x had a 0 value", evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //counter event should not be set
        if (0 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            swprintf(buffer, 75, L"Event 0x%x was configured for counting (0x%I64x)",
                     evSelect, pPerformanceEvents[i].performanceEvent);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

#ifdef CODEXL_LIBCPUPERFEVENT_AVBL

        // the libCpuPerfEvent is not yet available in Linux
        //are the event and unit mask valid?
        if (!g_eventsFile.ValidateEvent(evSelect, oneEvent.ucUnitMask))
        {
            wchar_t buffer[120];
            swprintf(buffer, 120, L"The event (0x%x) and unit mask (0x%x) were not validated%s",
                     evSelect, oneEvent.ucUnitMask,
                     ((oneEvent.ucUnitMask == 0) ? L"\nAt least one bit needs to be set in the unit mask" : L""));
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //nLogn part to ensure no duplicate events occur later
        for (unsigned int j = i + 1; j < count; j++)
        {
            //transform the event select from the event
            PERF_CTL checkEvent;
            checkEvent.perf_ctl = pPerformanceEvents[j].performanceEvent;

            if ((checkEvent.ucEventSelect == oneEvent.ucEventSelect) &&
                (checkEvent.ucEventSelectHigh == oneEvent.ucEventSelectHigh) &&
                (checkEvent.ucUnitMask == oneEvent.ucUnitMask) &&
                (checkEvent.bitOsEvents == oneEvent.bitOsEvents) &&
                (checkEvent.bitUsrEvents == oneEvent.bitUsrEvents))
            {
                wchar_t buffer[100];
                swprintf(buffer, 100, L"The event (0x%x), unit mask (0x%x), OS (%x) and USR (%x) were duplicated",
                         evSelect, oneEvent.ucUnitMask, oneEvent.bitOsEvents, oneEvent.bitUsrEvents);
                g_errorString[clientId] = buffer;
                return E_INVALIDARG;
            }
        }

#endif // CODEXL_LIBCPUPERFEVENT_AVBL
    }

    // TODO: Should i check for "The profiler was already configured" -- may not be required ?

    for (unsigned int j = 0; j < count; j++)
    {
        if (g_usePERF)
        {
            PERF_CTL oneEvent;
            oneEvent.perf_ctl = pPerformanceEvents[j].performanceEvent;
            (void) GetEvent12BitSelect(oneEvent);

            gtUInt64 samplingPeriod = pPerformanceEvents[j].value; // FIXME

            bool excludeKernel = (oneEvent.bitOsEvents) ? false : true;
            bool excludeUser = (oneEvent.bitUsrEvents) ? false : true;

            gtUInt32 pmcFlags = getPerfPmcFlags(excludeUser, excludeKernel);

            // set the PerfEvent in PerfConfig
            PerfEvent aEvent;

            gtUInt32 umask = oneEvent.ucUnitMask;
            gtUInt32 eventId = GetEvent12BitSelect(oneEvent);

            aEvent.initialize(eventId,
                              umask,
                              pmcFlags,
                              0);

            gSamplingEventList.push_back(std::pair<PerfEvent, gtUInt64>(aEvent, samplingPeriod));
            gNumEbpEvents++;

            if (profileAllCores)
            {
                hr = addCoreMask((const gtUInt64*)g_pSystemCoreMask,
                                 g_SystemCoreMaskSize);
            }
            else
            {
                hr = addCoreMask(pCpuCoreMask, cpuCoreMaskSize);
            }

            if (S_OK != hr)
            {
                g_errorString[clientId] = L"Unexpected error while setting the core";
                return E_UNEXPECTED;
            }
        }
        else
        {
            // OProfile
            return E_NOTIMPL;
        }
    }

    g_profileType[clientId] |= PROFILE_TYPE_EVENT_SAMPLING;

    return S_OK;
}


HRESULT CpuPerfSetIbsConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples,
    /*in*/ bool useDispatchOps,
    /*in*/ bool profileAllCores)
{

    HRESULT hr;

    hr = CpuPerfSetIbsConfiguration(
             fetchPeriod,
             opPeriod,
             randomizeFetchSamples,
             useDispatchOps,
             &cpuCoreMask,
             1,
             profileAllCores);

    return hr;

}


HRESULT CpuPerfSetIbsConfiguration(
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples,
    /*in*/ bool useDispatchOps,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores)
{
    HRESULT hr = S_OK;

    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    // Does the Hardware and OS support IBS based profiling ?
    bool isIbsAvailable = true;
    hr = fnGetIbsAvailable(&isIbsAvailable);

    if (S_OK != hr || (false == isIbsAvailable))
    {
        g_errorString[clientId] = L"IBS profile is not supported";
        return E_NOTIMPL;
    }

    //verify values
    if ((0 == fetchPeriod) && (0 == opPeriod))
    {
        g_errorString[clientId] = L"There was no period passed in fetchPeriod or opPeriod";
        return E_FAIL;
    }

    if (((fetchPeriod != 0) && (fetchPeriod < 50000))
        || (fetchPeriod > MAX_IBS_CYCLE_COUNT))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"fetchPeriod must be between %ld and %ld",
                 50000, MAX_IBS_CYCLE_COUNT);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //Handle extended count and accounting for op count randomization factor, if necessary
    gtUInt32 maxIbsOps = MAX_IBS_CYCLE_COUNT;
    osCpuid cpuid;

    if (cpuid.isIbsExtCountAvailable())
    {
        maxIbsOps = MAX_IBS_EXT_COUNT;
    }

    if (((opPeriod != 0) && (opPeriod < 50000)) ||
        (opPeriod > maxIbsOps))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"opPeriod must be between %ld and %ld",
                 50000, maxIbsOps);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //if the cpu cannot use dispatch sampling and the user directed it
    if ((useDispatchOps) && (! cpuid.isIbsOpsDispatchAvailable()))
    {
        useDispatchOps = false;
        hr = S_FALSE;
    }

    if (g_usePERF)
    {
        // this is used to set the IBS specific sample attributes - PERF_SAMPLE_RAW
        g_isIbs = true;

        gtUInt32 type;
        //
        // Baskar: BUG379336: IBS is not working in Linux kernel version 3.5.5+.
        // To enable IBS profiling, kernel version 3.5.5+ expects certain flags
        // (exclude_user, exclude_kernel, exclude_hv, exclude_idle, exclude_host
        // and exclude_guest) to be zero. Hence need to pass 'false' to
        // getPerfPmcFlags().
        //
        gtUInt32 basePmcFlags = getPerfPmcFlags(false, false);

        // IBS Fetch profile
        if (fetchPeriod)
        {
            gtUInt32 pmcFlags = basePmcFlags;

            if (randomizeFetchSamples)
            {
                pmcFlags |= (gtUInt32)(PerfEvent::PERF_PMU_FLAG_IBS_RAND_INST_FETCH_TAGGING);
            }

            type = PerfEvent::PERF_PROFILE_TYPE_IBS_FETCH;

            // set the PerfEvent in PerfConfig
            PerfEvent aEvent;
            aEvent.initialize(type, pmcFlags, 0);

            gSamplingEventList.push_back(
                std::pair<PerfEvent, gtUInt64>(aEvent, fetchPeriod));
        }

        // IBS OP profile
        if (opPeriod)
        {
            gtUInt32 pmcFlags = basePmcFlags;

            if (useDispatchOps)
            {
                pmcFlags |= (gtUInt32)(PerfEvent::PERF_PMU_FLAG_IBS_COUNT_DISPATCHED_OPS);
            }

            type = PerfEvent::PERF_PROFILE_TYPE_IBS_OP;

            // set the PerfEvent in PerfConfig
            PerfEvent aEvent;
            aEvent.initialize(type, pmcFlags, 0);

            gSamplingEventList.push_back(
                std::pair<PerfEvent, gtUInt64>(aEvent, opPeriod));
        }

        if (profileAllCores)
        {
            hr = addCoreMask((const gtUInt64*)g_pSystemCoreMask,
                             g_SystemCoreMaskSize);
        }
        else
        {
            hr = addCoreMask(pCpuCoreMask, cpuCoreMaskSize);
        }

        if (S_OK != hr)
        {
            g_errorString[clientId] = L"Unexpected error while setting the core";
            return E_UNEXPECTED;
        }
    }
    else
    {
        // Oprofile supports IBS
        return E_NOTIMPL;
    }

    g_profileType[clientId] |= PROFILE_TYPE_IBS;

    return hr;
}


// This is used for Per Process Profiling in PERF
HRESULT CpuPerfSetFilterProcesses(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ bool systemWideMode,
    /*in*/ bool ignoreChildren)
{
    (void)(ignoreChildren); // unused
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    if (g_usePERF)
    {
        if (S_OK != _addTargetPids(count, pProcessIds))
        {
            return E_UNEXPECTED;
        }

        // This is per-process profiling
        g_isSWP = systemWideMode;
    }
    else
    {
        // Handle this in Oprofile ?
        hr = E_NOTIMPL;
    }

    return hr;
}


HRESULT CpuPerfSetCallStackSampling(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ unsigned int unwindLevel,
    /*in*/ unsigned int samplePeriod,
    /*in*/ CpuProfileCssScope scope,
    /*in*/ bool captureVirtualStack)
{
    (void)(unwindLevel); // unused
    (void)(samplePeriod); // unused
    (void)(scope); // unused
    (void)(captureVirtualStack); // unused
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    // set the global flag to true;
    if (g_usePERF)
    {
        g_callStack = true;

        if (S_OK != _addTargetPids(count, pProcessIds))
        {
            return E_UNEXPECTED;
        }
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}


HRESULT CpuPerfClearConfigurations(gtUInt32 clientId)
{
    HRESULT retVal = S_OK;

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    // reset the profilingstate
    g_profilingState[clientId] = ProfilingStopped;

    // Clear the profile configuration objects;
    CpuPerfCleanUpConfiguration();

    if (!gSamplingEventList.empty())
    {
        gSamplingEventList.clear();
    }

    g_profileType[clientId] = PROFILE_TYPE_NOT_CONFIGURED;

    //ensure that the sampling profile file will need to be defined again
    memset(g_outputDir, 0, sizeof(g_outputDir));
    memset(g_outputFile, 0, sizeof(g_outputFile));

    g_isSWP = true;
    g_callStack = false;
    g_isIbs = false;
    gNumEbpEvents = 0;

#ifdef ENABLE_FAKETIMER
    // Baskar;
    // Cleanup the fake timer related hack fields. Otherwise subsequent non-TBP
    // profile measurements will get affected. if we don't reset this g_timerHackIndex
    // to -1, the CaPerfProfile/CaPerfConfig will assume this is a TBP measurement, and
    // ends up adding the fake-timer section (with incorrect data) in the caperf.data file
    //
    g_timerHackIndex = -1;
    g_timerHackNsPeriod = 0;
#endif // ENABLE_FAKETIMER

    return retVal;
}


HRESULT CpuPerfEnableCPUUtilization(
    unsigned int utilization_interval,
    unsigned int monitorFlag)
{
    (void)(utilization_interval); // unused
    (void)(monitorFlag); // unused
    gtUInt32 clientId = helpGetClientId();

    if ((NULL == g_pProfileSession)
        || (INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    g_profileType[clientId] |= PROFILE_TYPE_CPU_UTILIZATION;

    // Yet to be implemented
    return E_NOTIMPL;
}


HRESULT CpuPerfAddCPUUtilizationProcessId(unsigned pid)
{
    (void)(pid); // unused
    //  Yet to be implemented
    return E_NOTIMPL;
}
