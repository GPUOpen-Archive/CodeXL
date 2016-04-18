//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuUtilization.cpp
///
//==================================================================================

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTCpuProfilingControl/inc/Windows/CpuUtilization.h>
#include <Psapi.h>

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

#define SystemProcessorPerformanceInformation 0x8

static LONG(_stdcall* pfnNtQuerySystemInformation)(int SystemInformationClass,
                                                   PVOID SystemInformation,
                                                   ULONG SystemInformationLength,
                                                   PULONG ReturnLength) = NULL;


#define CPU_UTI_INITIALIZED             0x1
#define CPU_UTI_MONITORING              0x2
#define CPU_UTI_MONITORING_PAUSED       0x4

#define DATA_BUFFER_SIZE                4096


// helper funcitons to handle FILETIME;
static bool IsNULLTIME(const FILETIME& a)
{
    return (a.dwHighDateTime == 0 && a.dwLowDateTime == 0);
}

static void MakeNULLTime(FILETIME& a)
{
    a.dwHighDateTime = 0;
    a.dwLowDateTime = 0;
}

// worker thread start point;
static void CPUUtilizationMonitor(void* pData)
{
    if (!pData)
    {
        return;
    }

    CpuUtilization* pSelf = (CpuUtilization*) pData;
    pSelf->MonitorThreadEntry();

}


// This is the constructor of a class that has been exported.
// see CaCPUUti.h for the class definition
CpuUtilization::CpuUtilization()
{
    m_interval = 500;   // sleep interval of ms;

    m_fileStream = NULL;

    for (unsigned int u = 0; u < MAX_PID_SUPPORT; u++)
    {
        m_procHandles[u] = INVALID_HANDLE_VALUE;
        m_pidArray[u] = 0;
        m_pidMonitorThread[u] = 0;
    }

    // initialize the critical section
    InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0x50000600);

    m_pidCnt = 0;
    m_StatusFlag = 0;

    m_dataBuffer = NULL;
    m_dataSize = 0;
    m_sysPerfData = NULL;

    m_wokerThread = INVALID_HANDLE_VALUE;
    m_ThreadExitEvents = INVALID_HANDLE_VALUE;
}

CpuUtilization::~CpuUtilization()
{
    StopCPUUtilMonitor();

    SetDefaultConfig();

    if (m_fileStream)
    {
        fclose(m_fileStream);
        m_fileStream = NULL;
    }

    // delete critical section
    if (m_StatusFlag & CPU_UTI_INITIALIZED)
    {
        DeleteCriticalSection(&m_CriticalSection);
        m_StatusFlag = 0;
    }

    ReleaseDataBuffer();

    ClearEvents();
}

void CpuUtilization::ClearEvents()
{
    if (m_ThreadExitEvents != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_ThreadExitEvents);
        m_ThreadExitEvents = INVALID_HANDLE_VALUE;
    }
}

bool CpuUtilization::CreateEvents()
{
    bool bRet = true;
    m_ThreadExitEvents = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    if (m_ThreadExitEvents == INVALID_HANDLE_VALUE)
    {
        bRet = false;
    }

    return bRet;
}

unsigned int CpuUtilization::GetCoreNumber()
{
    SYSTEM_INFO sysInfo;

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    BOOL use64bit = true;
#else
    BOOL use64bit = false;
    IsWow64Process(GetCurrentProcess(), &use64bit);
#endif

    if (use64bit)
    {
        GetNativeSystemInfo(&sysInfo);
    }
    else
    {
        GetSystemInfo(&sysInfo);
    }

    return sysInfo.dwNumberOfProcessors;
}

bool CpuUtilization::AllocateDataBuffer()
{
    bool bRet = true;

    m_dataSize = 0;
    // allocate data buffer;
    m_dataBuffer = (unsigned char*) malloc(DATA_BUFFER_SIZE);

    m_sysPerfData = (unsigned char*) malloc(
                        sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * GetCoreNumber() * 2);

    if (!m_dataBuffer || !m_sysPerfData)
    {
        bRet = false;
    }

    return bRet;
}

void CpuUtilization::ReleaseDataBuffer()
{
    if (m_dataBuffer)
    {
        free(m_dataBuffer);
    }

    if (m_sysPerfData)
    {
        free(m_sysPerfData);
    }

    m_sysPerfData = NULL;
    m_dataBuffer = NULL;
}

unsigned int CpuUtilization::Initialize(unsigned int monitorFlag)
{
    unsigned int ret_val = evCU_FAILED_INITIALIZE;
    bool bCriticalSectionOK = true;

    if (m_StatusFlag & CPU_UTI_INITIALIZED)
    {
        m_monitorFlag = monitorFlag;
        return evCU_OK;
    }

    // initialize the NT API for system performance query.
    if (NULL == pfnNtQuerySystemInformation)
    {
        *(FARPROC*) & pfnNtQuerySystemInformation = GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
    }

    if (NULL != pfnNtQuerySystemInformation && AllocateDataBuffer() && CreateEvents())
    {
        ret_val = evCU_OK;
        m_StatusFlag |= CPU_UTI_INITIALIZED;
    }
    else
    {
        if (bCriticalSectionOK)
        {
            DeleteCriticalSection(&m_CriticalSection);
        }

        ClearEvents();

        ReleaseDataBuffer();
    }

    m_monitorFlag = monitorFlag;

    return ret_val;
}

unsigned int CpuUtilization::AddPid(unsigned int newPid, bool bMonitorThread)
{
    if (m_pidCnt >= MAX_PID_SUPPORT)
    {
        return evCU_MAX_PID_ERROR;
    }

    HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, newPid);

    if (NULL == h)
    {
        return evCU_FAILED_OPEN_PROCESS_HANDLE;
    }

    EnterCriticalSection(&m_CriticalSection);
    m_pidArray[m_pidCnt] = newPid;
    m_procHandles[m_pidCnt] = h;

    if (bMonitorThread)
    {
        m_pidMonitorThread[m_pidCnt] = 1;
    }

    MakeNULLTime(m_pidftKernelTime[m_pidCnt]);
    MakeNULLTime(m_pidftUserTime[m_pidCnt]);
    m_pidCnt++;
    LeaveCriticalSection(&m_CriticalSection);

    return evCU_OK;
}

void CpuUtilization::InitUtilHeader()
{
    LARGE_INTEGER ctr;

    // write header
    memcpy(m_hdr.signature, CUT_HEADER_SIGNATURE, 8);
    m_hdr.version = CPUUTILIZATION_VERSION;
    m_hdr.num_cores = GetCoreNumber();
    m_hdr.num_Process = 0;
    m_hdr.num_Records = 0;

    QueryPerformanceCounter(&ctr);
    m_hdr.systemTick = GetTickCount();
    m_hdr.highResTime = ctr.QuadPart;

    QueryPerformanceFrequency(&ctr);
    m_hdr.highResFreq = ctr.QuadPart;

    // get memory info
    MEMORYSTATUS ms;
    memset(&ms, sizeof(MEMORYSTATUS), 0);
    ms.dwLength = sizeof(MEMORYSTATUS);
    ::GlobalMemoryStatus(&ms);
    m_hdr.phy_memory = ms.dwTotalPhys;
    m_hdr.vir_memroy = ms.dwTotalVirtual;
}

//
unsigned int CpuUtilization::StartCPUUtilMonitor(wchar_t* logFileName)
{
    unsigned int ret_val = evCU_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (m_StatusFlag & CPU_UTI_MONITORING)
    {
        ret_val = evCU_ALREADY_IN_MONITORING_STATUS;
    }

    if (m_StatusFlag & ~CPU_UTI_INITIALIZED)
    {
        ret_val = evCU_NOT_INITIALIZED;
    }

    // NOTE: [Suravee]
    // This is a sanity check. From the callstack shown in debugger,
    // I suspect the memset below was causing a random crash in
    // BUG261017: CA GUI crashes when user checks "CPU Utilization" option
    if (!m_sysPerfData)
    {
        ret_val = evCU_NOT_INITIALIZED;
    }

    LeaveCriticalSection(&m_CriticalSection);

    if (ret_val != evCU_OK)
    {
        return ret_val;
    }

    MakeNULLTime(m_prevFTSysKernel);
    MakeNULLTime(m_prevFTSysUser);

    // clean up the previous data in the system performance information.
    // allocate one chunk of buffer and use it for both previous data an current data;
    memset(m_sysPerfData, 0, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * GetCoreNumber() * 2);
    m_perfDataPrev = m_sysPerfData;
    m_perfDataCurr = m_sysPerfData + GetCoreNumber() * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);
    pfnNtQuerySystemInformation(SystemProcessorPerformanceInformation, m_perfDataPrev,
                                (sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)*m_hdr.num_cores), NULL);

    // open the log file
    _wfopen_s(&m_fileStream, logFileName, L"wb");

    if (!m_fileStream)
    {
        return evCU_FILE_OPEN_FAILURE;
    }

    // initialize the cpu utilization file header
    InitUtilHeader();

    // write the header into file;
    fwrite(&m_hdr, sizeof(CpuUtilHeader), 1, m_fileStream);

    // thread is not created yet, it's safe to just update status without
    // critical section.
    m_StatusFlag |= CPU_UTI_MONITORING;

    // log first data;
    CollectData();

    // Create thread to flush the log file regularly
    m_wokerThread = CreateThread(NULL, 0,
                                 (LPTHREAD_START_ROUTINE) CPUUtilizationMonitor, this, 0, NULL);

    if (INVALID_HANDLE_VALUE == m_wokerThread)
    {
        ret_val = evCU_FAILED_INITIALIZE;
    }

    if (ret_val != evCU_OK)
    {
        if (m_fileStream)
        {
            fclose(m_fileStream);
            m_fileStream = NULL;

            // thread is not created yet, it's safe to just update status without
            // critical section.
            m_StatusFlag &= ~CPU_UTI_MONITORING;
        }
    }

    return ret_val;
}

unsigned int CpuUtilization::StopCPUUtilMonitor()
{
    unsigned int ret_val = evCU_OK;

    // check if it's in monitoring mode;
    if (~m_StatusFlag & CPU_UTI_MONITORING)
    {
        ret_val = evCU_NOT_IN_MONITORING;
    }

    if (ret_val != evCU_OK)
    {
        return ret_val;
    }

    // set event to terminate the worker threads;
    SetEvent(m_ThreadExitEvents);
    ::WaitForSingleObject(m_wokerThread, INFINITE);

    // log the last data();
    CollectData();

    // Stop data collection;
    EnterCriticalSection(&m_CriticalSection);
    m_StatusFlag &= ~CPU_UTI_MONITORING;
    LeaveCriticalSection(&m_CriticalSection);

    long pos = ftell(m_fileStream);

    m_hdr.num_Process = m_pidCnt;
    // rewind file descriptor to beginning position;
    rewind(m_fileStream);
    // update the header into file;
    fwrite(&m_hdr, sizeof(CpuUtilHeader), 1, m_fileStream);
    // move back to end of file;
    fseek(m_fileStream, pos, SEEK_SET);

    fclose(m_fileStream);
    m_fileStream = NULL;

    // final clean up for next session;
    m_dataSize = 0;
    m_wokerThread = INVALID_HANDLE_VALUE;
    m_StatusFlag = CPU_UTI_INITIALIZED;
    SetDefaultConfig();

    return ret_val;
}

void CpuUtilization::SetDefaultConfig()
{
    m_StatusFlag &= CPU_UTI_INITIALIZED; //clear out any flags set during profile

    for (unsigned int u = 0; u < m_pidCnt; u++)
    {
        if (INVALID_HANDLE_VALUE != m_procHandles[u])
        {
            CloseHandle(m_procHandles[u]);
        }

        m_procHandles[u] = INVALID_HANDLE_VALUE;
        m_pidArray[u] = 0;
        m_pidMonitorThread[u] = 0;
        MakeNULLTime(m_pidftKernelTime[u]);
        MakeNULLTime(m_pidftUserTime[u]);
    }

    m_pidCnt = 0;

    m_dataSize = 0;

}

unsigned int CpuUtilization::SetInterval(unsigned int interval)
{
    unsigned int ret_val = evCU_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (m_StatusFlag & CPU_UTI_MONITORING)
    {
        ret_val = evCU_ALREADY_IN_MONITORING_STATUS;
    }

    LeaveCriticalSection(&m_CriticalSection);

    if (ret_val != evCU_OK)
    {
        return ret_val;
    }

    // if it's not in monitoring mode; there is only one thread; it's safe to modify interval;
    m_interval = interval;

    return ret_val;
}


void CpuUtilization::MonitorThreadEntry()
{
    DWORD dwThreadStatus = 0;
    bool bComplete = false;

    for (;;)
    {
        dwThreadStatus = ::WaitForSingleObject(m_ThreadExitEvents, m_interval);

        switch (dwThreadStatus)
        {
            case WAIT_OBJECT_0:
                // Stop is triggered; Handle here
                bComplete = true;

            case WAIT_TIMEOUT:
                CollectData();
                break;

            // Return value is invalid.
            default:
                break;
        }

        // if stop data collection, break the loop and terminate the thread;
        if (bComplete)
        {
            break;
        }
    }

    ExitThread(0);
}

unsigned int CpuUtilization::Pause()
{
    unsigned int ret_val = evCU_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (~m_StatusFlag & CPU_UTI_MONITORING)
    {
        ret_val = evCU_NOT_IN_MONITORING;
    }
    else
    {
        m_StatusFlag |= CPU_UTI_MONITORING_PAUSED;
    }

    LeaveCriticalSection(&m_CriticalSection);

    return ret_val;
}

unsigned int CpuUtilization::Resume()
{
    unsigned int ret_val = evCU_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (~m_StatusFlag & CPU_UTI_MONITORING)
    {
        ret_val = evCU_NOT_IN_MONITORING;
    }
    else
    {
        m_StatusFlag &= ~CPU_UTI_MONITORING_PAUSED;
    }

    LeaveCriticalSection(&m_CriticalSection);

    return ret_val;
}


// the routine to collect data;
//
unsigned int CpuUtilization::CollectData()
{
    unsigned int ret_val = evCU_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (~m_StatusFlag & CPU_UTI_MONITORING)
    {
        ret_val = evCU_NOT_IN_MONITORING;
    }

    if (m_StatusFlag & CPU_UTI_MONITORING_PAUSED)
    {
        ret_val = evCU_NOT_IN_MONITORING;
    }

    LeaveCriticalSection(&m_CriticalSection);

    if (ret_val != evCU_OK)
    {
        return ret_val;
    }

    m_dataSize = 0;

    // time stamp record is always the first one;
    LogTimeStampRecord();

    if (m_monitorFlag & MONITOR_MEM)
    {
        LogSysMemoryUsage();
    }

    if (m_monitorFlag & MONITOR_CPU)
    {
        LogCoreCPUUtilization();
        LogProcessCPUUtilization();
    }

    if (m_monitorFlag & MONITOR_MEM)
    {
        LogProcessMemoryConsumption();
    }

    if (m_dataSize)
    {
        EnterCriticalSection(&m_CriticalSection);
        fwrite(m_dataBuffer, 1, m_dataSize, m_fileStream);
        LeaveCriticalSection(&m_CriticalSection);
    }

    return ret_val;
}

unsigned int CpuUtilization::LogTimeStampRecord()
{
    unsigned int retVal = 0;
    unsigned sz = sizeof(CU_TimestampRecord);
    EnsureDataBufferAvailable(sz);

    CU_TimestampRecord* pTSRec;
    pTSRec = (CU_TimestampRecord*)(m_dataBuffer + m_dataSize);
    LARGE_INTEGER ctr;
    QueryPerformanceCounter(&ctr);
    pTSRec->recordType = CU_TIMESTAMP_RECORD;
    pTSRec->hrTimeStamp = ctr.QuadPart;
    m_dataSize += sz;

    m_hdr.num_Records++;
    m_hdr.lastHrTime = ctr.QuadPart;
    return retVal;

}

unsigned int CpuUtilization::LogSysMemoryUsage()
{
    unsigned int retVal = 0;
    unsigned sz = sizeof(SysMemoryUsage);
    EnsureDataBufferAvailable(sz);

    SysMemoryUsage* pSysMemUsage = reinterpret_cast<SysMemoryUsage*>(m_dataBuffer + m_dataSize);

    // get memory info
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(MEMORYSTATUS);
    ::GlobalMemoryStatus(&ms);
    pSysMemUsage->recordType = SYSTEM_MEMORY_USAGE;
    pSysMemUsage->memoryUsage = static_cast<gtUInt16>(ms.dwMemoryLoad);
    m_dataSize += sz;

    m_hdr.num_Records++;

    return retVal;
}

// collect cpu utilization data;
//
unsigned int CpuUtilization::LogCoreCPUUtilization()
{
    unsigned int retVal = 0;

    unsigned long returnlength = 0;
    int status = 0;
    gtUInt64 delta_Idle = 0;
    gtUInt64 delta_Kernel = 0;
    gtUInt64 delta_User = 0;
    unsigned char* pTemp = NULL;

    unsigned int cn = m_hdr.num_cores;
    unsigned sz = sizeof(CoreUtilRecord) + cn * sizeof(unsigned int);

    EnsureDataBufferAvailable(sz);
    CoreUtilRecord* pCoreUtil = reinterpret_cast<CoreUtilRecord*>(m_dataBuffer + m_dataSize);
    pCoreUtil->recordType = CORE_CPU_UTILIZATION;
    pCoreUtil->numBytesFollow = (gtUInt16)cn * sizeof(unsigned int);
    unsigned int* pCU = (unsigned int*)(pCoreUtil + 1);

    status = pfnNtQuerySystemInformation(SystemProcessorPerformanceInformation, m_perfDataCurr,
                                         (sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * cn), &returnlength);

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* pCurInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*) m_perfDataCurr;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* pPreInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*) m_perfDataPrev;

    for (unsigned int cpuCnt = 0; cpuCnt < cn; cpuCnt++)
    {
        delta_Idle = (gtUInt64)(pCurInfo[cpuCnt].IdleTime.QuadPart - pPreInfo[cpuCnt].IdleTime.QuadPart);
        delta_Kernel = (gtUInt64)(pCurInfo[cpuCnt].KernelTime.QuadPart - pPreInfo[cpuCnt].KernelTime.QuadPart);
        delta_User = (gtUInt64)(pCurInfo[cpuCnt].UserTime.QuadPart - pPreInfo[cpuCnt].UserTime.QuadPart);

        if (delta_Kernel + delta_User != 0)
        {
            *pCU = (unsigned int)(100 - (delta_Idle * 100 / (delta_Kernel + delta_User)));
        }
        else
        {
            *pCU = 0;
        }

        pCU++;
    }

    m_dataSize += sz;

    // exchange;
    pTemp = m_perfDataPrev;
    m_perfDataPrev = m_perfDataCurr;
    m_perfDataCurr = pTemp;

    m_hdr.num_Records++;

    return retVal;
}

// collect process cpu utilization data;
//
unsigned int CpuUtilization::LogProcessCPUUtilization()
{
    unsigned int retVal = 0;

    unsigned int pidCnt = 0;
    FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;

    gtUInt64 deltaSysK, deltaSysU, deltaProcK, deltaProcU, totalProc, totalSys;
    deltaSysK = deltaSysU = 0;

    if (GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser))
    {
        if (!IsNULLTIME(m_prevFTSysKernel) && !IsNULLTIME(m_prevFTSysUser))
        {
            deltaSysK = GetFileTimeDelta(m_prevFTSysKernel, ftSysKernel);
            deltaSysU = GetFileTimeDelta(m_prevFTSysUser, ftSysUser);
        }

        // update system kernel and user filetime for future usage;
        m_prevFTSysUser = ftSysUser;
        m_prevFTSysKernel = ftSysKernel;

    }
    else
    {
        MakeNULLTime(m_prevFTSysKernel);
        MakeNULLTime(m_prevFTSysUser);
    }

    totalSys = deltaSysK + deltaSysU;

    HANDLE h;

    EnterCriticalSection(&m_CriticalSection);
    pidCnt = m_pidCnt;
    LeaveCriticalSection(&m_CriticalSection);

    unsigned sz = pidCnt * sizeof(ProcessUtil);
    EnsureDataBufferAvailable(sz);
    ProcessUtil* pProcUtil = (ProcessUtil*)(m_dataBuffer + m_dataSize);

    for (unsigned int p = 0; p < pidCnt; p++)
    {
        pProcUtil->recordType = PROCESS_CPU_UTILIZATION;
        pProcUtil->threadRecCnt = 0;
        pProcUtil->processID = m_pidArray[p];

        deltaProcK = deltaProcU = 0;
        h = m_procHandles[p];

        if (GetProcessTimes(h, &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser) == TRUE)
        {
            if (!IsNULLTIME(m_pidftKernelTime[p]))
            {
                deltaProcK = GetFileTimeDelta(m_pidftKernelTime[p], ftProcKernel);
            }

            if (!IsNULLTIME(m_pidftUserTime[p]))
            {
                deltaProcU = GetFileTimeDelta(m_pidftUserTime[p], ftProcUser);
            }

            m_pidftKernelTime[p].dwHighDateTime = ftProcKernel.dwHighDateTime;
            m_pidftKernelTime[p].dwLowDateTime = ftProcKernel.dwLowDateTime;
            m_pidftUserTime[p].dwHighDateTime = ftProcUser.dwHighDateTime;
            m_pidftUserTime[p].dwLowDateTime = ftProcUser.dwLowDateTime;

        }
        else
        {
            MakeNULLTime(m_pidftKernelTime[p]);
            MakeNULLTime(m_pidftUserTime[p]);
        }

        totalProc = deltaProcK + deltaProcU;

        if (totalSys)
        {
            pProcUtil->procCPUUtil = (unsigned int)((100.0 * totalProc) / totalSys);
        }
        else
        {
            pProcUtil->procCPUUtil = 0;
        }

        pProcUtil++;

        m_hdr.num_Records++;
    }

    m_dataSize += sz;

    return retVal;
}

gtUInt64 CpuUtilization::GetFileTimeDelta(const FILETIME& oldTime, const FILETIME& newTime)
{
    LARGE_INTEGER o, n;

    o.LowPart = oldTime.dwLowDateTime;
    o.HighPart = oldTime.dwHighDateTime;

    n.LowPart = newTime.dwLowDateTime;
    n.HighPart = newTime.dwHighDateTime;

    if (n.QuadPart >= o.QuadPart)
    {
        return n.QuadPart - o.QuadPart;
    }
    else
    {
        return 0;
    }
}


unsigned int CpuUtilization::LogProcessMemoryConsumption()
{
    unsigned int retVal = 0;
#if SUPPORT_CPUUTIL

    unsigned int pidCnt = 0;
    HANDLE h;

    EnterCriticalSection(&m_CriticalSection);
    pidCnt = m_pidCnt;
    LeaveCriticalSection(&m_CriticalSection);

    unsigned sz = pidCnt * sizeof(ProcessMemUsage);
    EnsureDataBufferAvailable(sz);
    ProcessMemUsage* pProcMem = (ProcessMemUsage*)(m_dataBuffer + m_dataSize);

    PROCESS_MEMORY_COUNTERS_EX pmce;

    for (unsigned int p = 0; p < pidCnt; p++)
    {
        pProcMem->recordType = PROCESS_MEMORY_USAGE;
        pProcMem->padding = 0;
        pProcMem->processID = m_pidArray[p];
        pProcMem->workingSetSize = 0;
        pProcMem->privateUsage = 0;

        h = m_procHandles[p];

        if (GetProcessMemoryInfo(h, (PPROCESS_MEMORY_COUNTERS)&pmce, sizeof(pmce)))
        {
            pProcMem->workingSetSize = pmce.WorkingSetSize ;
            pProcMem->privateUsage = pmce.PrivateUsage ;
        }

        pProcMem++;

        m_hdr.num_Records++;
    }

    m_dataSize += sz;
#endif
    return retVal;
}



void CpuUtilization::EnsureDataBufferAvailable(unsigned int dataSizeRequired)
{
    if (dataSizeRequired + m_dataSize >= DATA_BUFFER_SIZE)
    {
        EnterCriticalSection(&m_CriticalSection);
        fwrite(m_dataBuffer, 1, m_dataSize, m_fileStream);
        m_dataSize = 0;
        LeaveCriticalSection(&m_CriticalSection);
    }
}
