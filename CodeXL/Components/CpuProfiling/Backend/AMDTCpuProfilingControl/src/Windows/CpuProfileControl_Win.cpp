//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileControl_Win.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingControl/src/Windows/CpuProfileControl_Win.cpp#21 $
// Last checkin:   $DateTime: 2016/04/14 02:12:20 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569057 $
//=====================================================================

#include "CpuProfileControl_Win.h"
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <CXLTaskInfo/inc/TaskInfoInterface.h>
#include <AMDTCpuProfilingControl/inc/Windows/CpuUtilization.h>
#include <AMDTDriverControl/inc/DriverControl.h>

#include <Sddl.h>
#include <objbase.h>
#include <Psapi.h>
#include <Ntsecapi.h>


#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #define INSTALL_DIR_REG_KEY L"SOFTWARE\\Wow6432Node\\AMD\\CodeAnalyst\\Key\0"
#else
    #define INSTALL_DIR_REG_KEY L"SOFTWARE\\AMD\\CodeAnalyst\\Key\0"
#endif

typedef enum
{
    ///The current client is not configured for anything
    PROFILE_STATE_NOT_CONFIGURED = 0x0000,
    ///At least one event configuration has been added for the next
    /// profile
    PROFILE_STATE_EBP_SET       = 0x0001,
    ///The timer configuration has been set for the next profile
    PROFILE_STATE_TBP_SET       = 0x0002,
    ///The ibs configuration has been set for the next profile
    PROFILE_STATE_IBS_SET       = 0x0004,
    ///The CPU utilization configuration has been set for the next profile
    PROFILE_STATE_CU_SET        = 0x0008
} CAPROF_CONTROL_STATE;

// Configuration Mode
enum EventCfgMode
{
    EVENTCFG_NOTSET = 0,
    EVENTCFG_SAMPLE_MODE = 0x1,
    EVENTCFG_COUNT_MODE = 0x2,
};

#define IS_PMC_SAMPLE_MODE(clientId_)    ((EVENTCFG_SAMPLE_MODE & g_EventCfgMode[clientId_]) == EVENTCFG_SAMPLE_MODE)
#define IS_PMC_COUNT_MODE(clientId_)    ((EVENTCFG_COUNT_MODE & g_EventCfgMode[clientId_]) == EVENTCFG_COUNT_MODE)

#define IS_SAMPLING_MODE(clientId_)     (   (0 != (g_profileType[clientId_] & PROFILE_STATE_IBS_SET))    \
                                         || (0 != (g_profileType[clientId_] & PROFILE_STATE_TBP_SET)) \
                                         || (IS_PMC_SAMPLE_MODE(clientId_)))

//Will unlock state mutex when it goes out of scope
class MutexLocker
{
private:
    HANDLE mutex;
public:
    inline explicit MutexLocker(HANDLE m) : mutex(m)
    {
        if (NULL != mutex)
        {
            WaitForSingleObject(mutex, INFINITE);
        }
    }
    inline ~MutexLocker()
    {
        if (NULL != mutex)
        {
            ReleaseMutex(mutex);
        }
    }
};

gtUInt32 helpGetClientId();

//state of the driver
extern ProfileState g_profilingState[MAX_CLIENT_COUNT];
HANDLE g_stateMutex[MAX_CLIENT_COUNT];

//Driver Client id, index is id, value is the process id
extern osProcessId g_clientId[MAX_CLIENT_COUNT];
extern gtUInt32 INVALID_CLIENT;
//last error string, overwritten with every error
extern gtString g_errorString[MAX_CLIENT_COUNT];
extern gtString g_invalidClientErr;

//profile configuration info
extern unsigned long g_profileType[MAX_CLIENT_COUNT];
//shared memory interacting with the driver
HANDLE g_sharedMapFile = NULL;
SHARED_CLIENT* g_aSharedPause = NULL;
HANDLE g_hAbortEvent[MAX_CLIENT_COUNT];

//driver name used
wchar_t g_pcoreDriverFile [OS_MAX_PATH + 1];
HANDLE g_hPcoreDevice[MAX_CLIENT_COUNT];
wchar_t g_cpuProfDriverFile [OS_MAX_PATH + 1];
HANDLE g_hCpuProfDevice[MAX_CLIENT_COUNT];

//profile configuration info
extern unsigned long g_profileType[MAX_CLIENT_COUNT];
wchar_t g_prdFileName[MAX_CLIENT_COUNT][OS_MAX_PATH];
wchar_t g_outputDirectory[MAX_CLIENT_COUNT][OS_MAX_PATH];
wchar_t g_dynamicTiFileName[MAX_CLIENT_COUNT][OS_MAX_PATH];

// CPU Utilization
wchar_t g_cutFileName[OS_MAX_PATH];
CpuUtilization g_CPUUtil;

TIMER_PROPERTIES g_timerCfg[MAX_CLIENT_COUNT];
IBS_PROPERTIES g_ibsCfg[MAX_CLIENT_COUNT];
PID_PROPERTIES g_pidCfg[MAX_CLIENT_COUNT];

gtUInt32 g_EventCfgMode[MAX_CLIENT_COUNT];
gtList<EVENT_PROPERTIES> g_eventCfgs;

// Baskar: TBD: Should this is be in Windows Specific Implementation ?
extern EventsFile* gp_eventsFile;
RESOURCE_AVAILABILITY g_Availability;

//Defines and constants
extern unsigned int g_maxCore;
extern gtUInt64 g_SystemCoreMask;

extern gtUInt64* g_pSystemCoreMask;
extern gtUInt32 g_SystemCoreMaskCount;
extern gtUInt32 g_SystemCoreMaskSize;

extern unsigned int g_maxEventConfigs;

const gtUInt64 MSR_PERF_EVT_SEL_I = 0xC0010000ULL;
const gtUInt64 MSR_PER_FEVT_CTR_I = 0xC0010004ULL;

//
// Helper Functions
//

static void PrintDriverError(int errorCode, gtUInt32 clientId, const wchar_t* pIoctl = NULL, bool success = false)
{
    GT_UNREFERENCED_PARAMETER(success);

    switch (errorCode)
    {
        case PROF_SUCCESS:
            //no need for an error string here...
            break;

        case ERROR_ACCESS_DENIED:
            g_errorString[clientId] = L"The client id was not valid, no client id was available, or the configuration was already set";
            break;

        case ERROR_INVALID_PARAMETER:
            g_errorString[clientId] = L"One of the given configuration values was invalid";
            break;

        case ERROR_GEN_FAILURE:
            g_errorString[clientId] = L"The profiler was not started";
            break;

        case ERROR_INSUFFICIENT_BUFFER:
            g_errorString[clientId] = L"The output buffer was too small";
            break;

        case ERROR_BAD_LENGTH:
            g_errorString[clientId] = L"The input size was wrong";
            break;

        case ERROR_INVALID_USER_BUFFER:
            g_errorString[clientId] = L"The buffer size provided was insufficient";
            break;

        case ERROR_NOT_READY:
            g_errorString[clientId] = L"The profile client state was not ready";
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            g_errorString[clientId] = L"There is not enough available kernel memory";
            break;

        case ERROR_NO_SYSTEM_RESOURCES:
            g_errorString[clientId] = L"There were not enough resources";
            break;

        case ERROR_BUSY:
            g_errorString[clientId] = L"The profiler was already started";
            break;

        case ERROR_RESOURCE_TYPE_NOT_FOUND:
            g_errorString[clientId] = L"No configurations were added";
            break;

        case ERROR_FILE_NOT_FOUND:
            g_errorString[clientId] = L"No output files were set";
            break;

        case ERROR_NOT_SUPPORTED:
            g_errorString[clientId] = L"The IOCTL was not supported";
            break;

        case ERROR_FILE_INVALID:
            g_errorString[clientId] = L"The output file could not be written";
            break;

        case ERROR_LOCK_VIOLATION:
            g_errorString[clientId] = L"Some other process or service has locked the profiling hardware";
            break;

        default:
        {
            wchar_t buffer[50];
            wsprintf(buffer, L"There was an undefined (0x%x) driver error",
                     errorCode);
            g_errorString[clientId] = buffer;
        }
        break;
    }

    if ((PROF_SUCCESS != errorCode) && (NULL != pIoctl))
    {
        g_errorString[clientId].append(pIoctl);
    }

}


gtUInt32 helpGetPauseKeyClientId(const wchar_t* pauseKey)
{
    gtUInt32 clientId = INVALID_CLIENT;
    gtUInt32 i;

    for (i = 0; i < MAX_CLIENT_COUNT; i++)
    {
        if (L'\0' == g_aSharedPause[i].pauseKey[0])
        {
            continue;
        }

        if (0 == wcscmp(pauseKey, g_aSharedPause[i].pauseKey))
        {
            break;
        }
    }

    if (MAX_CLIENT_COUNT == i)
    {
        clientId = INVALID_CLIENT;
    }
    else
    {
        clientId = i;
    }

    return clientId;
}

bool helpVerifyEventFileAvailable(gtUInt32 clientId)
{
    if (NULL == gp_eventsFile)
    {
        gtString pathVar;
        osGetCurrentProcessEnvVariableValue(L"CPUPerfAPIDataPath", pathVar);

        EventEngine eventEngine;
        osDirectory fileDirectory;
        fileDirectory.setDirectoryFullPathFromString(pathVar);

        if (eventEngine.Initialize(fileDirectory))
        {
            osCpuid cpuid;
            gp_eventsFile = eventEngine.GetEventFile(cpuid.getFamily(), cpuid.getModel());
        }

        if (NULL == gp_eventsFile)
        {
            wchar_t buffer[65];
            wsprintf(buffer, L"The event file was not available");

            if (INVALID_CLIENT != clientId)
            {
                g_errorString[clientId] = buffer;
            }
        }
    }

    return (NULL != gp_eventsFile);
}

void InitializeProfAPISharedObj()
{
    g_sharedMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS,  // read/write access
                                       FALSE,                      // do not inherit the name
                                       CPU_PROF_SHARED_OBJ);   // name of mapping object

    if (!g_sharedMapFile)
    {
        SECURITY_ATTRIBUTES secAttr;
        PSECURITY_DESCRIPTOR pSD;
        PACL pSacl = NULL;  // not allocated
        BOOL fSaclPresent = FALSE;
        BOOL fSaclDefaulted = FALSE;
        char secDesc[ SECURITY_DESCRIPTOR_MIN_LENGTH ];

        secAttr.nLength = sizeof(secAttr);
        secAttr.bInheritHandle = FALSE;
        secAttr.lpSecurityDescriptor = &secDesc;

        bool bHasSD = false;
        OSVERSIONINFO osVersionInfo;
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (GetVersionEx(&osVersionInfo))
        {
            if (osVersionInfo.dwMajorVersion >= 6)
            {
                // Vista, Longhorn or later;
                bHasSD = true;
            }
        }

        if (bHasSD)
        {
            ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)",   // this means "low integrity"
                                                                 SDDL_REVISION_1, &pSD, NULL);

            GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl,
                                      &fSaclDefaulted);

            SetSecurityDescriptorSacl(secAttr.lpSecurityDescriptor, TRUE,
                                      pSacl, FALSE);
        }

        InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor,
                                     SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0,
                                  FALSE);

        g_sharedMapFile =
            CreateFileMappingW(INVALID_HANDLE_VALUE,
                               &secAttr,           // default security
                               PAGE_READWRITE,         // read/write access
                               0,              // max. object size
                               CPU_PROF_SHARED_MEM_SIZE,           // buffer size
                               CPU_PROF_SHARED_OBJ);   // name of mapping object
    }

    if (g_sharedMapFile)
    {
        //Mapview of file to get the memory location to write 1 to pause the
        // sample collection and write 0 to resume sample collection
        g_aSharedPause =
            (SHARED_CLIENT*) MapViewOfFile(g_sharedMapFile,   // handle to mapping object
                                           FILE_MAP_READ | FILE_MAP_WRITE, // read/write permission
                                           0, 0, CPU_PROF_SHARED_MEM_SIZE);
    }
}


void ReleaseProfAPISharedObj()
{
    //Unmap the shared memory area for the user kernel mode
    if (g_aSharedPause)
    {
        UnmapViewOfFile(g_aSharedPause);
        g_aSharedPause = NULL;
    }

    if (g_sharedMapFile)
    {
        CloseHandle(g_sharedMapFile);
        g_sharedMapFile = NULL;
    }
}


//This function will test whether the driver is currently profiling
//If the driver is currently profiling, this will return true;
bool isProfilingNow(gtUInt32 clientId)
{
    bool bRet = false;

    if (g_hCpuProfDevice == INVALID_HANDLE_VALUE)
    {
        return bRet;
    }

    PROFILER_PROPERTIES profProp;
    profProp.ulClientId = clientId;
    profProp.ulStatus = 0;
    DWORD dwReturned;

    //check this every time in case the profile aborted
    if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_GET_PROFILER_PROPERTIES, &profProp,
                    sizeof(PROFILER_PROPERTIES), &profProp, sizeof(PROFILER_PROPERTIES), &dwReturned))
    {
        if ((profProp.ulProfilerState & STATE_PROFILING) ||
            (profProp.ulProfilerState & STATE_STOPPING))
        {
            bRet = true;
        }
    }
    else
    {
        //If the IOCTL failed, they shouldn't be profiling anyway
        bRet = true;
    }

    return bRet;
}


HRESULT mutexFreePause(gtUInt32 clientId)
{
    gtUInt32 myClientId = helpGetClientId();

    if (ProfilingPaused == g_profilingState[clientId])
    {
        return S_FALSE;
    }

    if ((Profiling != g_profilingState[clientId]) && (myClientId == clientId))
    {
        g_errorString[clientId] = L"The profiler was not ready to pause";
        return E_ACCESSDENIED;
    }

    HRESULT hr = S_OK;
    bool sampling = IS_SAMPLING_MODE(clientId);

    //sampling can use the shared data pause
    if ((NULL != g_aSharedPause) && ((sampling) || (myClientId != clientId)))
    {
        g_aSharedPause[clientId].paused = TRUE;
    }

    //counting has to use the full ioctl pause
    if ((NULL == g_aSharedPause) ||
        ((myClientId == clientId) && (IS_PMC_COUNT_MODE(clientId))))
    {
        PROFILER_PROPERTIES profProp;
        DWORD dwReturned;

        profProp.ulClientId = clientId;
        profProp.ulStatus = 0;

        if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_PAUSE_PROFILER, &profProp,
                        sizeof(PROFILER_PROPERTIES), &profProp, sizeof(PROFILER_PROPERTIES), &dwReturned))
        {
            if (profProp.ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(profProp.ulStatus, clientId, L" (IOCTL_PAUSE_PROFILER)");
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = S_OK;
            }
        }
        else
        {
            PrintDriverError(GetLastError(), clientId, L" (IOCTL_PAUSE_PROFILER)", true);
            hr = E_UNEXPECTED;
        }
    }

    if (S_OK == hr)
    {
        g_profilingState[clientId] = ProfilingPaused;
    }

    return hr;
}


/* Return the number of bits with value "1" */
unsigned int count1Bits(gtUInt64 value)
{
    value &= 0xFF;
    unsigned int count = 0;

    while (value)
    {
        count += value & 0x1;
        value >>= 1;
    }

    return count;
}


bool compareMappingOrder(const EVENT_PROPERTIES a, const EVENT_PROPERTIES b)
{
    unsigned int a_bitCount = count1Bits(a.ulCounterIndex);
    unsigned int b_bitCount = count1Bits(b.ulCounterIndex);

    // Check number of bits in the avilable mask
    if (a_bitCount < b_bitCount)
    {
        return true;
    }
    else if (a_bitCount > b_bitCount)
    {
        return false;
    }

    // Check value of the availability mask
    if (a.ulCounterIndex < b.ulCounterIndex)
    {
        return true;
    }
    else if (a.ulCounterIndex > b.ulCounterIndex)
    {
        return false;
    }

    // Check evSelect
    if (a.ullEventCfg < b.ullEventCfg)
    {
        return true;
    }

    return false;
}


void getCounterAllocation()
{
    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);
    int* aAllocCount = new int[maxCounters];
    memset(aAllocCount, 0, (sizeof(int) * maxCounters));

    for (gtList<EVENT_PROPERTIES>::iterator evIt  = g_eventCfgs.begin(), evEnd = g_eventCfgs.end(); evIt != evEnd; ++evIt)
    {
        unsigned int minCounter = static_cast<unsigned int>(-1);
        unsigned int curCounter = 0;
        int minVal = 255;
        unsigned int countMask = (*evIt).ulCounterIndex;

        //check the counter of events on each available counter
        while (countMask > 0)
        {
            if ((countMask & 1) > 0)
            {
                //available counter
                if (aAllocCount[curCounter] < minVal)
                {
                    minVal = aAllocCount[curCounter];
                    minCounter = curCounter;
                }
            }

            countMask >>= 1;
            curCounter++;
        }

        //The distribution is fair across the available counters
        (*evIt).ulCounterIndex = minCounter;
        aAllocCount[minCounter]++;
    }

    delete [] aAllocCount;
}


gtUInt64 GetSystemCoreMask()
{
    gtUInt64 coremask = 0;
    SYSTEM_INFO sysinfo;

    BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    isSys64 = true;
#else
    IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

    if (isSys64)
    {
        GetNativeSystemInfo(&sysinfo);
    }
    else
    {
        GetSystemInfo(&sysinfo);
    }

    for (unsigned int u = 0; u < sysinfo.dwNumberOfProcessors; u++)
    {
        coremask += 1LL << u;
    }

    return coremask;
}


// This should support more than 64 processors
HRESULT GetActiveCoreMask(gtUInt64** pCoreMask, gtUInt32* pCoreMaskSize, gtUInt32* pCoreMaskCount)
{
    typedef DWORD (WINAPI * PGAPC)(WORD);

    if (!pCoreMask || !pCoreMaskCount || !pCoreMaskSize)
    {
        return S_FALSE;
    }

    DWORD numberOfProcessors = 0;
    // Note: To use GetActiveProcessorCount, set _WIN32_WINNT >= 0x0601
    // GetActiveProcessorCount() is available from windows 7 and windows server 2008.
    PGAPC pGAPC;
    pGAPC = (PGAPC) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetActiveProcessorCount");

    if (NULL != pGAPC)
    {
        // numberOfProcessors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
        numberOfProcessors = pGAPC(ALL_PROCESSOR_GROUPS);
    }
    else
    {
        SYSTEM_INFO sysinfo;
        BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        isSys64 = true;
#else
        IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

        if (isSys64)
        {
            GetNativeSystemInfo(&sysinfo);
        }
        else
        {
            GetSystemInfo(&sysinfo);
        }

        numberOfProcessors = sysinfo.dwNumberOfProcessors;
    }

    if (0 == numberOfProcessors)
    {
        return E_UNEXPECTED;
    }

    // alloc memory
    gtUInt32 maskSize = ((numberOfProcessors - 1) / 64) + 1;
    gtUInt64* pTmp  = (gtUInt64*)malloc(sizeof(gtUInt64) * maskSize);

    if (pTmp == NULL)
    {
        return E_UNEXPECTED;
    }

    gtUInt32 coreMaskCount = 0;

    for (unsigned int u = 0; u < numberOfProcessors; u++)
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


unsigned int
GetLargestCoreMasked(const gtUInt64* pCpuCoreMask, unsigned int cpuCoreMaskSize)
{
    unsigned int maskCoreCount = 0;

    if ((0 == cpuCoreMaskSize) || (NULL == pCpuCoreMask))
    {
        return 0;
    }

    // Atleast one core should be set in the pCpuCoreMask array
    bool validCore = false;

    for (unsigned int i = 0; i < cpuCoreMaskSize; i++)
    {
        if (pCpuCoreMask[i])
        {
            validCore = true;
            break;
        }
    }

    if (false == validCore)
    {
        // the array has no core mask set; Lets profile all the cores
        return 0;
    }

    maskCoreCount = (cpuCoreMaskSize - 1) * 64;

    gtUInt64 t64 = pCpuCoreMask[cpuCoreMaskSize - 1];

    while (0 != t64)
    {
        t64 = t64 >> 1;
        maskCoreCount++;
    }

    return maskCoreCount;
}


gtUInt64*
CopyCoreMask(const gtUInt64* pCpuCoreMask, unsigned maskCoreCount)
{
    if ((NULL == pCpuCoreMask) || (0 == maskCoreCount))
    {
        return NULL;
    }

    //allocate core masks
    unsigned int maskIndex = ((maskCoreCount - 1) / 64) + 1;
    gtUInt64* pTmp = (gtUInt64*)malloc((sizeof(gtUInt64) * maskIndex));

    if (NULL == pTmp)
    {
        return NULL;
    }

    //Copy the data
    memcpy(pTmp, pCpuCoreMask, (maskIndex * sizeof(gtUInt64)));

    return pTmp;
}


gtUInt64*
CreateCoreMask(unsigned int core)
{
    // The assumption is core starts from 0
    // allocate core masks
    unsigned int maskSize = (core / 64) + 1;
    gtUInt64* pTmp = (gtUInt64*)malloc((sizeof(gtUInt64) * maskSize));

    if (NULL == pTmp)
    {
        return NULL;
    }

    memset(pTmp, 0, (sizeof(gtUInt64) * maskSize));
    pTmp[maskSize - 1] = 1ULL << core;

    return pTmp;
}


// Check whether the given core is set in the
bool
isCoreSet(gtUInt64* pCoreMaskArray, unsigned int core)
{
    if (NULL == pCoreMaskArray)
    {
        return false;
    }

    unsigned int  maskIndex = core / 64;
    unsigned int maskOffset = core % 64;

    bool coreMasked = (0 != (pCoreMaskArray[maskIndex] & (1ULL << maskOffset)));

    return coreMasked;
}

bool
isProfileAllCores(EVENT_PROPERTIES& eventProp)
{
    return (eventProp.ulCoreMaskCount == 0) ? true : false;
}

bool
isProfileCore(EVENT_PROPERTIES& eventProp, unsigned int coreId)
{
    bool ret = isProfileAllCores(eventProp);

    if (!ret)
    {
        ret = isCoreSet((gtUInt64*)(eventProp.ullCpuMask.QuadPart), coreId);
    }

    return ret;
}

bool GetAppPath(gtString& appPath)
{
    bool retVal = false;

    // Assumption: processEnum[32|64].exe will reside in the dir in which codexl.exe resides;
    osFilePath filePath;

    // First, see if the dll path is set (like VS)
    retVal = osGetCurrentApplicationDllsPath(filePath);

    if (!retVal)
    {
        // Since the dll path is not set, assume the dlls are in the
        // same path as the exe.
        // Get the absolute path for the application - codexl.exe
        retVal = osGetCurrentApplicationPath(filePath);
    }

    if (retVal)
    {
        appPath = filePath.fileDirectoryAsString();
    }

    return retVal;
}


bool isSampleEvent(EVENT_PROPERTIES& eventProp)
{
    PERF_CTL pmc;
    pmc.perf_ctl = eventProp.ullEventCfg;

    return pmc.bitSampleEvents ? true : false;
}


#ifdef _MANAGED
    #pragma managed(push, off)
#endif

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                     )
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_sharedMapFile = NULL;
            g_aSharedPause = NULL;
            memset(&g_clientId, 0, (sizeof(DWORD)*MAX_CLIENT_COUNT));

            for (int i = 0; i < MAX_CLIENT_COUNT; i++)
            {
                g_profilingState[i] = ProfilingUnavailable;
                g_hPcoreDevice[i] = INVALID_HANDLE_VALUE;
                g_hCpuProfDevice[i] = INVALID_HANDLE_VALUE;
                g_prdFileName[i][0] = L'\0';
                g_outputDirectory[i][0] = L'\0';
                g_dynamicTiFileName[i][0] = L'\0';
                g_hAbortEvent[i] = INVALID_HANDLE_VALUE;
                g_profileType[i] = 0;
                g_EventCfgMode[i] = EVENTCFG_NOTSET;
                g_stateMutex[i] = CreateMutex(NULL, FALSE, NULL);
                g_cutFileName[0] = L'\0';
            }

            wchar_t systemDir[OS_MAX_PATH];
            systemDir[0] = L'\0';
            GetSystemDirectory(systemDir, OS_MAX_PATH);

            g_pcoreDriverFile[0] = L'\0';
            g_cpuProfDriverFile[0] = L'\0';

            wcscpy_s(g_pcoreDriverFile, OS_MAX_PATH, systemDir);
            wcscpy_s(g_cpuProfDriverFile, OS_MAX_PATH, systemDir);

            wcscat_s(g_pcoreDriverFile, OS_MAX_PATH, L"\\drivers\\PCORE");
            wcscat_s(g_cpuProfDriverFile, OS_MAX_PATH, L"\\drivers\\CPUPROF");

            InitializeProfAPISharedObj();
            {
                SYSTEM_INFO sysinfo;
                BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                isSys64 = true;
#else
                IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

                if (isSys64)
                {
                    GetNativeSystemInfo(&sysinfo);
                }
                else
                {
                    GetSystemInfo(&sysinfo);
                }

                g_maxCore = sysinfo.dwNumberOfProcessors;
                g_SystemCoreMask = GetSystemCoreMask();

                // Support for 64+ cores
                GetActiveCoreMask(&g_pSystemCoreMask, &g_SystemCoreMaskSize, &g_SystemCoreMaskCount);
            }
            break;
        }

        case DLL_PROCESS_DETACH:
            for (int i = 0; i < MAX_CLIENT_COUNT; i++)
            {
                if (ProfilingUnavailable != g_profilingState[i])
                {
                    fnReleaseProfiling();
                }

                g_profilingState[i] = ProfilingUnavailable;

                if (g_stateMutex[i])
                {
                    CloseHandle(g_stateMutex[i]);
                }
            }

            if (NULL != gp_eventsFile)
            {
                delete gp_eventsFile;
            }

            ReleaseProfAPISharedObj();

            if (g_pSystemCoreMask)
            {
                free(g_pSystemCoreMask);
                g_pSystemCoreMask = NULL;
            }

            break;
    }

    return TRUE;
}

#ifdef _MANAGED
    #pragma managed(pop)
#endif


static HRESULT OpenCpuProfilingDriver(const wchar_t* pName, HANDLE& handle)
{
    HRESULT res;

    OpenAmdDriver(pName, &handle);

    if (INVALID_HANDLE_VALUE == handle)
    {
        DWORD err = GetLastError();

        osCpuid cpuid;

        if (true == cpuid.hasHypervisor() && false == cpuid.isSupportedHypervisor())
        {
            g_invalidClientErr = L"An unsupported hypervisor has been detected.\n"
                                 L"The CodeXL CPU Profiling driver does not support running on this hypervisor guest operating system.\nCPU Profiling is unavailable.";
            res = E_NOTSUPPORTED;
        }
        else if (ERROR_ACCESS_DENIED == err)
        {
            g_invalidClientErr = L"No handle to the driver could be accessed.";
            res = E_ACCESSDENIED;
        }
        else
        {
            g_invalidClientErr.makeEmpty();
            g_invalidClientErr.appendFormattedString(L"Trying to open the driver (%ls) returned error %u.", pName, err);
            res = E_UNEXPECTED;
        }
    }
    else
    {
        res = S_OK;
    }

    return res;
}


//
// Windows Specific API implementation
//

HRESULT CpuPerfEnableProfiling()
{
    g_invalidClientErr = L"No error yet";
    gtUInt32 clientId = helpGetClientId();

    PVOID oldValue = nullptr;
    BOOL isWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &isWow64);

    if (isWow64)
    {
        isWow64 = Wow64DisableWow64FsRedirection(&oldValue);
    }

    HANDLE hPcore = INVALID_HANDLE_VALUE;
    HANDLE hCpuProf = INVALID_HANDLE_VALUE;

    HRESULT res = OpenCpuProfilingDriver(g_pcoreDriverFile, hPcore);

    if (S_OK == res)
    {
        res = OpenCpuProfilingDriver(g_cpuProfDriverFile, hCpuProf);
    }

    //If the path to the event files has not yet been initialized, do it now.
    helpVerifyEventFileAvailable(INVALID_CLIENT);

    if (isWow64)
    {
        Wow64RevertWow64FsRedirection(oldValue);
    }


    DWORD dwReturned;

    if (S_OK == res)
    {
        gtUInt64 version = 0;
        HRESULT hr = DeviceIoControl(hCpuProf, IOCTL_GET_VERSION, NULL, 0, &version, sizeof(gtUInt64), &dwReturned, NULL);

        //Check that the CpuProf version is current
        if ((!SUCCEEDED(hr)) || ((version & 0xFFFFFFFF) < MIN_DRIVER_VERSION))
        {
            // Log the result
            gtString errorMsg;

            if (SUCCEEDED(hr))
            {
                errorMsg.appendFormattedString(L"Failed to get a valid CPU Profiling driver version. "
                                               L"DeviceIoControl return code = 0x%08X, returned driver version = %I64X.", hr, version);
            }
            else
            {
                // We need to get the error code first, so it won't get polluted from the next operations.
                DWORD err = GetLastError();

                errorMsg.appendFormattedString(L"Failed to get CPU Profiling driver version. "
                                               L"DeviceIoControl return code = 0x%08X, GetLastError return code = %u.", hr, err);
            }

            OS_OUTPUT_DEBUG_LOG(errorMsg.asCharArray(), OS_DEBUG_LOG_ERROR);

            g_invalidClientErr = L"The previous CPU Profiling driver version was still loaded, please reboot.";
            res = E_FAIL;
        }
    }

    if (S_OK == res)
    {
        if (!InvokeOut(hCpuProf, IOCTL_REGISTER_CLIENT, &clientId, sizeof(gtUInt32), &dwReturned))
        {
            // We need to get the error code first, so it won't get polluted from the next operations.
            DWORD err = GetLastError();

            g_invalidClientErr.makeEmpty();
            g_invalidClientErr.appendFormattedString(L"Unable to register with the driver (%u).", err);
            res = E_FAIL;
        }
    }

    if (S_OK == res)
    {
        if (!InvokeOut(hCpuProf, IOCTL_GET_AVAILABILITY, &g_Availability, sizeof(RESOURCE_AVAILABILITY), &dwReturned))
        {
            // We need to get the error code first, so it won't get polluted from the next operations.
            DWORD err = GetLastError();

            g_invalidClientErr.makeEmpty();
            g_invalidClientErr.appendFormattedString(L"Unable to get resource availability (%u).", err);

            InvokeIn(hCpuProf, IOCTL_UNREGISTER_CLIENT, &clientId, sizeof(gtUInt32), &dwReturned);
            res = E_FAIL;
        }
    }

    fnGetEventCounters(&g_maxEventConfigs);
    g_maxEventConfigs *= 8;

    if (S_OK == res)
    {
        g_clientId[clientId] = GetCurrentProcessId();

        MutexLocker ml(g_stateMutex[clientId]);

        g_profilingState[clientId] = ProfilingStopped;

        g_hPcoreDevice[clientId] = hPcore;
        g_hCpuProfDevice[clientId] = hCpuProf;
        g_prdFileName[clientId][0] = L'\0';
        g_outputDirectory[clientId][0] = L'\0';
        g_dynamicTiFileName[clientId][0] = L'\0';
        g_hAbortEvent[clientId] = INVALID_HANDLE_VALUE;
        g_cutFileName[0] = L'\0';
    }
    else
    {
        if (INVALID_HANDLE_VALUE != hCpuProf)
        {
            CloseAmdDriver(g_cpuProfDriverFile, hCpuProf);
        }

        if (INVALID_HANDLE_VALUE != hPcore)
        {
            CloseAmdDriver(g_pcoreDriverFile, hPcore);
        }
    }

    return res;
}


HRESULT CpuPerfReleaseProfiling()
{
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (ProfilingStopped != g_profilingState[clientId])
    {
        fnStopProfiling();
        hr = S_FALSE;
    }

    fnClearConfigurations();

    //TODO clear only configurations specific to the clientId
    g_eventCfgs.clear();

    g_prdFileName[clientId][0] = L'\0';
    g_outputDirectory[clientId][0] = L'\0';
    g_dynamicTiFileName[clientId][0] = L'\0';
    g_hAbortEvent[clientId] = INVALID_HANDLE_VALUE;
    g_cutFileName[0] = L'\0';

    if (INVALID_HANDLE_VALUE != g_hCpuProfDevice[clientId])
    {
        DWORD dwReturned;
        InvokeIn(g_hCpuProfDevice[clientId], IOCTL_UNREGISTER_CLIENT,
                 &clientId, sizeof(gtUInt32), &dwReturned);

        //If the handle doesn't close, like a Vista system, it's expected.
        CloseAmdDriver(g_cpuProfDriverFile, g_hCpuProfDevice[clientId]);
        g_hCpuProfDevice[clientId] = INVALID_HANDLE_VALUE;
    }

    if (INVALID_HANDLE_VALUE != g_hPcoreDevice[clientId])
    {
        CloseAmdDriver(g_pcoreDriverFile, g_hPcoreDevice[clientId]);
        g_hPcoreDevice[clientId] = INVALID_HANDLE_VALUE;
    }

    MutexLocker ml(g_stateMutex[clientId]);
    g_profilingState[clientId] = ProfilingUnavailable;
    g_clientId[clientId] = 0;
    return hr;
}


HRESULT CpuPerfGetDriverVersion(unsigned int* pMajor,
                                unsigned int* pMinor,
                                unsigned int* pBuild)

{
    HRESULT hr = E_ACCESSDENIED;
    DWORD d_temp;
    wchar_t fileName[OS_MAX_PATH];
    wcscpy_s(fileName, OS_MAX_PATH, g_cpuProfDriverFile);
    wcsncat_s(fileName, OS_MAX_PATH, L".sys", 4);
    DWORD d_version_size = GetFileVersionInfoSize(fileName, & d_temp);

    if (d_version_size > 0)
    {
        char* cp_buffer = new char[d_version_size + 1];

        if (NULL != cp_buffer)
        {
            memset(cp_buffer, 0, d_version_size + 1);

            if (GetFileVersionInfo(fileName, d_temp, d_version_size,
                                   cp_buffer))
            {
                VS_FIXEDFILEINFO* p_ffinfo = NULL;
                UINT ui_temp;

                if (::VerQueryValue((LPVOID) cp_buffer, _T("\\"),
                                    & (LPVOID&) p_ffinfo, & ui_temp))
                {
                    *pMajor = unsigned int (HIWORD(p_ffinfo->dwProductVersionMS));
                    *pMinor = unsigned int (LOWORD(p_ffinfo->dwProductVersionMS));
                    *pBuild = unsigned int (HIWORD(p_ffinfo->dwProductVersionLS));
                    hr = S_OK;
                }
            }

            delete [] cp_buffer;
        }
    }

    return hr;
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


//This function retrieves the availability of event counters per core.
HRESULT CpuPerfGetEventCounterAvailability(
    /*in*/ gtUInt64 performanceEvent,
    /*out*/ unsigned int* pAvailabilityMask,
    /*in*/  PmcType type)
{
    GT_UNREFERENCED_PARAMETER(type);

    PERF_CTL oneEvent;
    oneEvent.perf_ctl = performanceEvent;
    gtUInt32 clientId = helpGetClientId();

    // Baskar: TODO: Handle PmcType - PMC_CORE, PMC_NORTHBRIDGE, PMC_L2I
    if (!helpVerifyEventFileAvailable(clientId))
    {
        return E_FAIL;
    }

    //counter event has to be set
    unsigned int evSelect = GetEvent12BitSelect(oneEvent);
    CpuEvent eventData;

    if (!gp_eventsFile->FindEventByValue(evSelect, eventData))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"The event (0x%x) was not valid",
                 performanceEvent);

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = buffer;
        }

        return E_INVALIDARG;
    }

    if ((eventData.m_source == "NB") && (0 != g_Availability.nbAvailable))
    {
        *pAvailabilityMask = eventData.m_counters & g_Availability.nbAvailable;
    }
    else if ((eventData.m_source == "L2I") && (0 != g_Availability.l2iAvailable))
    {
        *pAvailabilityMask = eventData.m_counters & g_Availability.l2iAvailable;
    }
    else
    {
        *pAvailabilityMask = eventData.m_counters & g_Availability.pmcAvailable;
    }

    return S_OK;
}


HRESULT CpuPerfGetProfilerState(
    /*out*/ ProfileState* pProfileState)
{
    gtUInt32 clientId = helpGetClientId();

    MutexLocker ml(g_stateMutex[clientId]);

    //quick check to see if the profile was aborted
    if ((NULL != g_hAbortEvent[clientId]) && (WAIT_OBJECT_0 ==  WaitForSingleObject(g_hAbortEvent[clientId], 0)))
    {
        //Currently, the only reason this abort happens is due to a file write error
        g_errorString[clientId] = L"Unable to write to the file";
        g_profilingState[clientId] = ProfilingAborted;
    }

    bool sampling = IS_SAMPLING_MODE(clientId);

    switch (g_profilingState[clientId])
    {
        case Profiling:
        case ProfilingPaused:

            //check that it hasn't aborted, paused, or resumed
            if ((NULL != g_aSharedPause) && (sampling))
            {
                if (g_aSharedPause[clientId].paused)
                {
                    g_profilingState[clientId] = ProfilingPaused;
                }
                else
                {
                    g_profilingState[clientId] = Profiling;
                }
            }

            if (!isProfilingNow(clientId))
            {
                g_profilingState[clientId] = ProfilingAborted;
            }

        //deliberate fall through
        case ProfilingUnavailable:
        case ProfilingStopped:
        case ProfilingAborted:
        default:
            *pProfileState = g_profilingState[clientId];
            break;
    }

    return S_OK;
}


HRESULT CpuPerfStartProfiling(
    /*in*/ bool startPaused,
    /*in*/ bool pauseIndefinite,
    /*in*/ const wchar_t* pauseKey,
    /*out*/ ProfileState* pProfileState)
{
    GT_UNREFERENCED_PARAMETER(pauseIndefinite);
    HRESULT hr = S_OK;
    DWORD dwReturned;
    gtUInt32 clientId = helpGetClientId();

    MutexLocker ml(g_stateMutex[clientId]);

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"The profiler is already in use";
        return E_ACCESSDENIED;
    }

    bool sampling = IS_SAMPLING_MODE(clientId);

    if (sampling)
    {
        //verify output file
        if (L'\0' == g_prdFileName[clientId][0])
        {
            g_errorString[clientId] = L"fnSetProfileOutputFile has not been called";
            return E_PENDING;
        }

        //Do the actual driver file setting here
        OUTPUT_FILE_DESCRIPTOR  sOutputFile;
        memset(&sOutputFile, 0, sizeof(OUTPUT_FILE_DESCRIPTOR));
        sOutputFile.ulClientId = clientId;

        sOutputFile.uliPathName.QuadPart = reinterpret_cast<ULONGLONG>(g_prdFileName[clientId]);
        sOutputFile.ulPathSize = static_cast<ULONG>(wcslen(g_prdFileName[clientId]));

        wchar_t tempPath [OS_MAX_PATH + 1];
        GetTempPathW(OS_MAX_PATH + 1, tempPath);
        // define the temp file name for driver with prefix "tiD".
        GetTempFileNameW(tempPath, L"tiD", 0, g_dynamicTiFileName[clientId]);
        sOutputFile.uliTempTiPathName.QuadPart = reinterpret_cast<ULONGLONG>(g_dynamicTiFileName[clientId]);
        sOutputFile.ulTempTiSize = static_cast<ULONG>(wcslen(g_dynamicTiFileName[clientId]));

        // The driver saves its own time stamp when called with IOCTL_SET_OUTPUT_FILE. Therefore, we need to have the actual start
        // time stamp, which must be before the driver's, as it is used as a reference to every sample.
        LARGE_INTEGER startCount;
        QueryPerformanceCounter(&startCount);

        if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_SET_OUTPUT_FILE, &sOutputFile,
                        sizeof(OUTPUT_FILE_DESCRIPTOR), &sOutputFile, sizeof(OUTPUT_FILE_DESCRIPTOR), &dwReturned))
        {
            if (sOutputFile.ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(sOutputFile.ulStatus, clientId, L" (IOCTL_SET_OUTPUT_FILE)");
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            PrintDriverError(GetLastError(), clientId, L" (IOCTL_SET_OUTPUT_FILE)", true);
            hr = E_UNEXPECTED;
        }

        if (FAILED(hr))
        {
            return hr;
        }

        //clean up any previous temp Just-In-Time files
        fnCleanupJitInformation();

        //Start the ti capture
        gtString appPath;
        GetAppPath(appPath);
        hr = fnStartCapture(startCount.QuadPart, appPath.asCharArray());

        if (FAILED(hr))
        {
            wchar_t buffer[200];
            wsprintf(buffer, L"Failed (0x%lx) to start the task information capture",
                     hr);
            g_errorString[clientId] = buffer;
            return E_UNEXPECTED;
        }

        //If there are PIDs to filter for
        if (0 != g_pidCfg[clientId].ullPidArray[0])
        {
            if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_SET_PID_PROPERTIES, &(g_pidCfg[clientId]),
                            sizeof(PID_PROPERTIES), &(g_pidCfg[clientId]), sizeof(PID_PROPERTIES),
                            &dwReturned))
            {
                if (g_pidCfg[clientId].ulStatus != PROF_SUCCESS)
                {
                    PrintDriverError(g_pidCfg[clientId].ulStatus, clientId, L" (IOCTL_SET_PID_PROPERTIES)");
                    return E_UNEXPECTED;
                }
            }
            else
            {
                PrintDriverError(GetLastError(), clientId, L" (IOCTL_SET_PID_PROPERTIES)", true);
                return E_UNEXPECTED;
            }
        }
    }

    if ((SUCCEEDED(hr)) && (EVENTCFG_NOTSET != g_EventCfgMode[clientId]))
    {
        for (gtList<EVENT_PROPERTIES>::iterator evIt  = g_eventCfgs.begin(), evEnd = g_eventCfgs.end(); evIt != evEnd; ++evIt)
        {
            EVENT_PROPERTIES& eventCfg = (*evIt);

            DWORD dwReturned1;

            if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_ADD_EVENT_PROPERTIES, &eventCfg,
                            sizeof(EVENT_PROPERTIES), &eventCfg, sizeof(EVENT_PROPERTIES), &dwReturned1))
            {
                if (eventCfg.ulStatus != PROF_SUCCESS)
                {
                    PrintDriverError(eventCfg.ulStatus, clientId, L" (IOCTL_ADD_EVENT_PROPERTIES)");
                    hr = E_UNEXPECTED;
                    break;
                }
            }
            else
            {
                PrintDriverError(GetLastError(), clientId, L" (IOCTL_ADD_EVENT_PROPERTIES)", true);
                hr = E_UNEXPECTED;
                break;
            }
        }
    }

    //timer profiling
    if ((SUCCEEDED(hr)) && (0 != (g_profileType[clientId] & PROFILE_STATE_TBP_SET)))
    {
        if ((InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_SET_TIMER_PROPERTIES, &(g_timerCfg[clientId]),
                         sizeof(TIMER_PROPERTIES), &(g_timerCfg[clientId]), sizeof(TIMER_PROPERTIES), &dwReturned))
            || (g_timerCfg[clientId].ulStatus != PROF_SUCCESS))
        {
            if (g_timerCfg[clientId].ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(g_timerCfg[clientId].ulStatus, clientId, L" (IOCTL_SET_TIMER_PROPERTIES)");
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            PrintDriverError(GetLastError(), clientId, L" (IOCTL_SET_TIMER_PROPERTIES)", true);
            hr = E_UNEXPECTED;
        }
    }

    //ibs profiling
    if ((SUCCEEDED(hr)) && (0 != (g_profileType[clientId] & PROFILE_STATE_IBS_SET)))
    {
        if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_SET_IBS_PROPERTIES, &(g_ibsCfg[clientId]),
                        sizeof(IBS_PROPERTIES), &(g_ibsCfg[clientId]), sizeof(IBS_PROPERTIES), &dwReturned))
        {
            if (g_ibsCfg[clientId].ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(g_ibsCfg[clientId].ulStatus, clientId, L" (IOCTL_SET_IBS_PROPERTIES)");
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            PrintDriverError(GetLastError(), clientId, L" (IOCTL_SET_IBS_PROPERTIES)", true);
            hr = E_UNEXPECTED;
        }
    }

    PROFILER_PROPERTIES profProp;

    //Todo
    if (SUCCEEDED(hr))
    {
        profProp.ulClientId = clientId;
        g_hAbortEvent[clientId] = CreateEvent(NULL, FALSE, FALSE, NULL);
        profProp.hAbort.QuadPart = reinterpret_cast<ULONGLONG>(g_hAbortEvent[clientId]);

        if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_START_PROFILER,
                        &profProp, sizeof(PROFILER_PROPERTIES), &profProp, sizeof(PROFILER_PROPERTIES), &dwReturned))
        {
            if (profProp.ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(profProp.ulStatus, clientId, L" (IOCTL_START_PROFILER)");
                hr = E_UNEXPECTED;

                if (sampling)
                {
                    fnStopCapture(true, g_dynamicTiFileName[clientId]);
                }
            }
            else
            {
                hr = S_OK;
                g_profilingState[clientId] = Profiling;
            }
        }
        else
        {
            int errorCode = GetLastError();
            PrintDriverError(errorCode, clientId, L" (IOCTL_START_PROFILER)", true);

            if (ERROR_LOCK_VIOLATION != errorCode)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = E_LOCKED;
            }

            if (sampling)
            {
                fnStopCapture(true, g_dynamicTiFileName[clientId]);
            }
        }
    }

    // CPU Utilization Monitoring
    if (SUCCEEDED(hr) && (0 != (g_profileType[clientId] & PROFILE_STATE_CU_SET)))
    {
        if (g_CPUUtil.StartCPUUtilMonitor(g_cutFileName) != CpuUtilization::evCU_OK)
        {
            hr = S_FALSE;
        }
    }


    if (NULL != pauseKey)
    {
        wcscpy_s(g_aSharedPause[clientId].pauseKey, (OS_MAX_PATH - 1), pauseKey);
        g_aSharedPause[clientId].clientPid = GetCurrentProcessId();
    }

    if (SUCCEEDED(hr) && startPaused)
    {
        mutexFreePause(clientId);
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
    fnStartProfiling to control the profile.  The pause key used by the
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
        profileKey = (CaProfileCtrlKey) & (g_aSharedPause[clientId].paused);
    }

    return profileKey;
}

HRESULT CpuPerfPauseProfiling(CaProfileCtrlKey profileKey)
{
    if (NULL != profileKey)
    {
        BOOLEAN* pPause = (BOOLEAN*) profileKey;
        *pPause = TRUE;
        return S_OK;
    }
    else
    {
        gtUInt32 clientId = helpGetClientId();

        if (INVALID_CLIENT == clientId)
        {
            g_invalidClientErr = L"fnEnableProfiling was not called";
            return E_ACCESSDENIED;
        }

        if (g_profileType[clientId] & PROFILE_STATE_CU_SET)
        {
            g_CPUUtil.Pause();
        }

        MutexLocker ml(g_stateMutex[clientId]);
        return mutexFreePause(clientId);
    }
}


HRESULT CpuPerfResumeProfiling(CaProfileCtrlKey profileKey)
{
    HRESULT hr = S_OK;

    if (NULL != profileKey)
    {
        BOOLEAN* pPause = (BOOLEAN*) profileKey;
        *pPause = FALSE;
    }
    else
    {
        gtUInt32 clientId = helpGetClientId();

        if (INVALID_CLIENT == clientId)
        {
            g_invalidClientErr = L"fnEnableProfiling was not called";
            return E_ACCESSDENIED;
        }

        MutexLocker ml(g_stateMutex[clientId]);

        if (Profiling == g_profilingState[clientId])
        {
            return S_FALSE;
        }

        if (ProfilingPaused != g_profilingState[clientId])
        {
            g_errorString[clientId] = L"The profiler was not ready to resume";
            return E_ACCESSDENIED;
        }

        bool sampling = IS_SAMPLING_MODE(clientId);

        //sampling can use the shared data resume
        if ((NULL != g_aSharedPause) && (sampling))
        {
            g_aSharedPause[clientId].paused = FALSE;
        }

        //counting has to use the full ioctl resume
        if ((NULL == g_aSharedPause) || IS_PMC_COUNT_MODE(clientId))
        {
            PROFILER_PROPERTIES profProp;
            DWORD dwReturned;

            profProp.ulClientId = clientId;
            profProp.ulStatus = 0;

            if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_RESUME_PROFILER, &profProp,
                            sizeof(PROFILER_PROPERTIES), &profProp, sizeof(PROFILER_PROPERTIES), &dwReturned))
            {
                if (profProp.ulStatus != PROF_SUCCESS)
                {
                    PrintDriverError(profProp.ulStatus, clientId, L" (IOCTL_RESUME_PROFILER)");
                    hr = E_UNEXPECTED;
                }
                else
                {
                    hr = S_OK;
                }
            }
            else
            {
                PrintDriverError(GetLastError(), clientId, L" (IOCTL_RESUME_PROFILER)", true);
                hr = E_UNEXPECTED;
            }
        }

        if (S_OK == hr)
        {
            g_profilingState[clientId] = Profiling;
        }

        if (g_profileType[clientId] & PROFILE_STATE_CU_SET)
        {
            g_CPUUtil.Resume();
        }
    }

    return hr;
}


HRESULT CpuPerfStopSamplingProfile(gtUInt32 clientId)
{
    if ((Profiling != g_profilingState[clientId])
        && (ProfilingPaused != g_profilingState[clientId]))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"The profiler was not ready to stop";
        }

        return E_ACCESSDENIED;
    }

    MutexLocker ml(g_stateMutex[clientId]);

    //If the shared memory is available, set it to a known state.
    if (NULL != g_aSharedPause)
    {
        g_aSharedPause[clientId].paused = FALSE;
    }

    HRESULT hr = S_OK;

    PROFILER_PROPERTIES profProp;
    DWORD dwReturned;

    profProp.ulClientId = clientId;
    profProp.ulStatus = 0;

    if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_STOP_PROFILER,
                    &clientId, sizeof(gtUInt32), &profProp, sizeof(PROFILER_PROPERTIES), &dwReturned))
    {
        if (profProp.ulStatus != PROF_SUCCESS)
        {
            PrintDriverError(profProp.ulStatus, clientId, L" (IOCTL_STOP_PROFILER)");
            hr = E_UNEXPECTED;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        PrintDriverError(GetLastError(), clientId, L" (IOCTL_STOP_PROFILER)", true);
        hr = E_UNEXPECTED;
    }

    if (S_OK == hr)
    {
        g_profilingState[clientId] = ProfilingStopped;

        bool sampling = IS_SAMPLING_MODE(clientId);

        if (sampling)
        {
            hr = fnStopCapture(true, g_dynamicTiFileName[clientId]);

            if (S_OK == hr)
            {
                //use the prd file name to write ti file
                wchar_t tiFileName[OS_MAX_PATH];
                wcscpy_s(tiFileName, OS_MAX_PATH, g_prdFileName[clientId]);

                //replace the prd extension with ti
                tiFileName[wcslen(g_prdFileName[clientId]) - 3] = L'\0';
                wcscat_s(tiFileName, OS_MAX_PATH, L"ti");

                fnWriteModuleInfoFile(tiFileName);
            }
            else
            {
                wchar_t buffer[70];
                wsprintf(buffer, L"Failed (0x%x) to stop capturing task information",
                         hr);
                g_errorString[clientId] = buffer;
                hr = E_UNEXPECTED;
            }
        }
    }

    if (NULL != g_hAbortEvent[clientId])
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(g_hAbortEvent[clientId], 0))
        {
            wchar_t buffer[70];
            wsprintf(buffer, L"Unable to write to the file(s)\n");
            g_errorString[clientId] = buffer;
            g_profilingState[clientId] = ProfilingAborted;
            hr = E_ABORT;
        }

        CloseHandle(g_hAbortEvent[clientId]);
        g_hAbortEvent[clientId] = NULL;
    }

    return hr;
}


HRESULT CpuPerfStopCPUUtilMonitoring(gtUInt32 clientId)
{
    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"The profiler was not ready to stop";
        return E_ACCESSDENIED;
    }

    // Stop CPU utilization monitoring;
    if (g_profileType[clientId] & PROFILE_STATE_CU_SET)
    {
        g_CPUUtil.StopCPUUtilMonitor();
    }

    return S_OK;
}


HRESULT CpuPerfSetCountingEvent(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*in*/ EventConfiguration performanceEvent)
{
    gtUInt32 clientId = helpGetClientId();

    if ((g_hCpuProfDevice == INVALID_HANDLE_VALUE))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);

    if (eventCounterIndex >= maxCounters)
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"The counter index (%d) was not valid, the max is %d", eventCounterIndex, maxCounters - 1);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    if (0 == (g_Availability.pmcAvailable & (1UL << eventCounterIndex)))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"The counter index (%d) was not available, please choose another",
                 eventCounterIndex);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
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
        wsprintf(buffer, L"The event (0x%x) was not a counting event",
                 evSelect, oneEvent.ucUnitMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    if (!helpVerifyEventFileAvailable(clientId))
    {
        return E_FAIL;
    }

    //are the event and unit mask valid?
    if (!gp_eventsFile->ValidateEvent(evSelect, oneEvent.ucUnitMask))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"The event (0x%x) and unit mask (0x%x) were not validated",
                 evSelect, oneEvent.ucUnitMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //check previous configurations for the core/counter combo
    for (gtList<EVENT_PROPERTIES>::iterator evIt  = g_eventCfgs.begin(), evEnd = g_eventCfgs.end(); evIt != evEnd; ++evIt)
    {
        if (((*evIt).ulCounterIndex == eventCounterIndex)
            && (isCoreSet((gtUInt64*)((*evIt).ullCpuMask.QuadPart), core)))
        {
            return E_ACCESSDENIED;
        }
    }

    g_EventCfgMode[clientId] |= EVENTCFG_COUNT_MODE;

    EVENT_PROPERTIES eventCfg;
    eventCfg.ullEventCfg = performanceEvent.performanceEvent;
    eventCfg.ullEventCount = performanceEvent.value;
    eventCfg.ulCounterIndex = eventCounterIndex;

    eventCfg.ulCoreMaskCount = core; // Largest core masked;
    // Create the coremask array
    eventCfg.ullCpuMask.QuadPart = (gtUInt64)CreateCoreMask(core);

    eventCfg.ulClientId = helpGetClientId();
    g_eventCfgs.push_back(eventCfg);

    g_profileType[clientId] |= PROFILE_STATE_EBP_SET;

    return S_OK;
}


HRESULT CpuPerfGetEventCount(/*in*/ unsigned int core,
                                    /*in*/ unsigned int eventCounterIndex,
                                    /*out*/ gtUInt64* pEventCount)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);

    if (eventCounterIndex >= maxCounters)
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"The counter index (%d) was not valid, the max is %d",
                 eventCounterIndex, maxCounters - 1);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    if (!IS_PMC_COUNT_MODE(clientId))
    {
        g_errorString[clientId] = L"The profiler was not configured for counting";
        return E_FAIL;
    }

    if ((Profiling != g_profilingState[clientId]) &&
        (ProfilingPaused != g_profilingState[clientId]))
    {
        g_errorString[clientId] = L"The profiler was not currently profiling";
        return E_PENDING;
    }

    gtList<EVENT_PROPERTIES>::iterator evIt  = g_eventCfgs.begin(), evEnd = g_eventCfgs.end();

    for (; evIt != evEnd; ++evIt)
    {
        if (((*evIt).ulCounterIndex == eventCounterIndex)
            && (isProfileCore((*evIt), core)))
        {
            break;
        }
    }

    if (evIt == evEnd)
    {
        wchar_t buffer[75];
        wsprintf(buffer, L"The counter index %d on core %d was not configured for counting",
                 eventCounterIndex, core);
        g_errorString[clientId] = buffer;
        return S_FALSE;
    }

    COUNT_PROPERTIES countProperties;
    countProperties.ulStatus = 0;
    countProperties.ullCore = core;
    countProperties.ulCounterIndex = eventCounterIndex;
    countProperties.ullEventCfg = (*evIt).ullEventCfg;
    countProperties.ulClientId = clientId;

    HRESULT hr = S_OK;
    DWORD dwReturned;

    if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_GET_EVENT_COUNT,
                    &countProperties, sizeof(COUNT_PROPERTIES), &countProperties, sizeof(COUNT_PROPERTIES), &dwReturned))
    {
        if (countProperties.ulStatus != PROF_SUCCESS)
        {
            PrintDriverError(countProperties.ulStatus, clientId, L" (IOCTL_GET_EVENT_COUNT)");
            hr = E_UNEXPECTED;
        }
    }
    else
    {
        PrintDriverError(GetLastError(), clientId, L" (IOCTL_GET_EVENT_COUNT)", true);
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        *pEventCount = countProperties.ullEventCount;
    }

    return hr;
}


HRESULT CpuPerfSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ bool profileAllCores)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    if (!helpVerifyEventFileAvailable(clientId))
    {
        return E_FAIL;
    }

    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);

    for (unsigned int i = 0; i < count; i++)
    {
        //transform the event select from the event
        PERF_CTL oneEvent;
        oneEvent.perf_ctl = pPerformanceEvents[i].performanceEvent;
        unsigned int evSelect = GetEvent12BitSelect(oneEvent);

        if (maxCounters <= pPerformanceEvents[i].eventCounter)
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"The counter %d for event 0x%x was not supported",
                     pPerformanceEvents[i].eventCounter, evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        if (0 == (g_Availability.pmcAvailable & (1UL << pPerformanceEvents[i].eventCounter)))
        {
            wchar_t buffer[65];
            wsprintf(buffer, L"The counter index (%d) was not available, please choose another",
                     pPerformanceEvents[i].eventCounter);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //counter event has to be set
        if (1 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"Event 0x%x was not configured for counting",
                     evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //are the event and unit mask valid?
        if (!gp_eventsFile->ValidateEvent(evSelect, oneEvent.ucUnitMask))
        {
            wchar_t buffer[65];
            wsprintf(buffer, L"The event (0x%x) and unit mask (0x%x) were not validated",
                     evSelect, oneEvent.ucUnitMask);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }
    }

    g_EventCfgMode[clientId] |= EVENTCFG_COUNT_MODE;

    for (unsigned int i = 0; i < count; i++)
    {
        EVENT_PROPERTIES eventCfg;

        eventCfg.ullEventCfg = pPerformanceEvents[i].performanceEvent;
        eventCfg.ullEventCount = pPerformanceEvents[i].value;
        eventCfg.ulCounterIndex = pPerformanceEvents[i].eventCounter;

        if (true == profileAllCores)
        {
            // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
            eventCfg.ulCoreMaskCount = 0;
            eventCfg.ullCpuMask.QuadPart = NULL;
        }
        else
        {
            // Calculate the largest core masked and set the core count here
            unsigned int maskCoreCount = GetLargestCoreMasked(&cpuCoreMask, 1);
            eventCfg.ulCoreMaskCount = maskCoreCount;

            // Copy the coremask array
            eventCfg.ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(&cpuCoreMask, maskCoreCount);
        }

        eventCfg.ulClientId = clientId;
        g_eventCfgs.push_back(eventCfg);
    }

    g_profileType[clientId] |= PROFILE_STATE_EBP_SET;

    return S_OK;
}


HRESULT CpuPerfSetCountingConfiguration(
    /*in*/ gtUInt32 clientId,
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores)
{
    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    if (!helpVerifyEventFileAvailable(clientId))
    {
        return E_FAIL;
    }

    unsigned int maxCounters = 0;
    fnGetEventCounters(&maxCounters);

    for (unsigned int i = 0; i < count; i++)
    {
        //transform the event select from the event
        PERF_CTL oneEvent;
        oneEvent.perf_ctl = pPerformanceEvents[i].performanceEvent;
        unsigned int evSelect = GetEvent12BitSelect(oneEvent);

        if (maxCounters <= pPerformanceEvents[i].eventCounter)
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"The counter %d for event 0x%x was not supported",
                     pPerformanceEvents[i].eventCounter, evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        if (0 == (g_Availability.pmcAvailable & (1UL << pPerformanceEvents[i].eventCounter)))
        {
            wchar_t buffer[65];
            wsprintf(buffer, L"The counter index (%d) was not available, please choose another",
                     pPerformanceEvents[i].eventCounter);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //counter event has to be set
        if (1 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"Event 0x%x was not configured for counting",
                     evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //are the event and unit mask valid?
        if (!gp_eventsFile->ValidateEvent(evSelect, oneEvent.ucUnitMask))
        {
            wchar_t buffer[65];
            wsprintf(buffer, L"The event (0x%x) and unit mask (0x%x) were not validated",
                     evSelect, oneEvent.ucUnitMask);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }
    }

    g_EventCfgMode[clientId] |= EVENTCFG_COUNT_MODE;

    for (unsigned int i = 0; i < count; i++)
    {
        EVENT_PROPERTIES eventCfg;

        eventCfg.ullEventCfg = pPerformanceEvents[i].performanceEvent;
        eventCfg.ullEventCount = pPerformanceEvents[i].value;
        eventCfg.ulCounterIndex = pPerformanceEvents[i].eventCounter;

        if (true == profileAllCores)
        {
            // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
            eventCfg.ulCoreMaskCount = 0;
            eventCfg.ullCpuMask.QuadPart = NULL;
        }
        else
        {
            // Calculate the largest core masked and set the core count here
            unsigned int maskCoreCount = GetLargestCoreMasked(pCpuCoreMask, cpuCoreMaskSize);
            eventCfg.ulCoreMaskCount = maskCoreCount;

            // Copy the coremask array
            eventCfg.ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(pCpuCoreMask, maskCoreCount);
        }

        eventCfg.ulClientId = clientId;
        g_eventCfgs.push_back(eventCfg);
    }

    g_profileType[clientId] |= PROFILE_STATE_EBP_SET;

    return S_OK;
}  //CpuPerfSetCountingConfiguration


HRESULT CpuPerfGetCountingEventCount(
    unsigned int core,
    unsigned int* pCount)
{
    gtUInt32 clientId = helpGetClientId();
    *pCount = 0;
    HRESULT ret = E_FAIL;

    if (IS_PMC_COUNT_MODE(clientId))
    {
        //count each counting event with the core in the core mask
        for (gtList<EVENT_PROPERTIES>::iterator evIt = g_eventCfgs.begin(), evEnd = g_eventCfgs.end(); evIt != evEnd; ++evIt)
        {
            if ((clientId == (*evIt).ulClientId) && (!isSampleEvent((*evIt))) && isProfileCore((*evIt), core))
            {
                (*pCount)++;
            }
        }

        ret = S_OK;
    }

    return ret;
}


HRESULT CpuPerfGetAllEventCounts(
    /*in*/ unsigned int core,
    /*in*/ unsigned int size,
    /*out*/ gtUInt64* pCounts)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (! IS_PMC_COUNT_MODE(clientId))
    {
        return E_FAIL;
    }

    memset(pCounts, 0, size * sizeof(gtUInt64));

    int countIndex = 0;
    HRESULT hr = S_OK;

    //count each counting event with the core in the core mask
    for (gtList<EVENT_PROPERTIES>::iterator evIt  = g_eventCfgs.begin(), evEnd = g_eventCfgs.end(); evIt != evEnd; ++evIt)
    {
        if (isSampleEvent((*evIt)) || (! isProfileCore((*evIt), core)))
        {
            continue;
        }

        COUNT_PROPERTIES countProperties;
        countProperties.ulClientId = clientId;
        countProperties.ullCore = core;
        countProperties.ulCounterIndex = (*evIt).ulCounterIndex;
        countProperties.ullEventCfg = (*evIt).ullEventCfg;

        DWORD dwReturned;

        if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_GET_EVENT_COUNT, &countProperties,
                        sizeof(COUNT_PROPERTIES), &countProperties, sizeof(COUNT_PROPERTIES), &dwReturned))
        {
            if (countProperties.ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(countProperties.ulStatus, clientId, L" (IOCTL_GET_EVENT_COUNT)");
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            PrintDriverError(GetLastError(), clientId, L" (IOCTL_GET_EVENT_COUNT)", true);
            hr = E_UNEXPECTED;
        }

        if (!SUCCEEDED(hr))
        {
            break;
        }

        pCounts[countIndex] = countProperties.ullEventCount;
        countIndex++;
    }

    return hr;
}


HRESULT CpuPerfSetProfileOutputFile(
    /*in*/ const wchar_t* pFileName)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    //verify path exists and is writable
    wchar_t tempPathName[OS_MAX_PATH];
    tempPathName[OS_MAX_PATH - 1] = L'\0';

    wcsncpy_s(tempPathName, OS_MAX_PATH, pFileName, (OS_MAX_PATH - 1));
    wchar_t* finding = wcsrchr(tempPathName, L'\\');

    //remove the file name
    if (NULL != finding)
    {
        finding[1] = L'\0';
    }

    if ((NULL == finding) || (L'\0' == tempPathName[0]))
    {
        wcscpy_s(tempPathName, OS_MAX_PATH, L".");
    }

    //2 means checking for write access
    if (-1 == _waccess(tempPathName, 2))
    {
        g_errorString[clientId] = L"Did not have write access to the given path";
        return E_ACCESSDENIED;
    }

    wchar_t tempName[OS_MAX_PATH];
    //test the write only quality by writing a temporary file

    if (0 == GetTempFileName(tempPathName, L"CAW", 0, (LPWSTR)&tempName))
    {
        g_errorString[clientId] = L"Could not write to the given file name";
        return E_ACCESSDENIED;
    }
    else
    {
        DeleteFile(tempName);
    }

    g_prdFileName[clientId][0] = L'\0';
    g_cutFileName[0] = L'\0';

    if (NULL == finding)
    {
        GetCurrentDirectory(OS_MAX_PATH, g_prdFileName[clientId]);
        GetCurrentDirectory(OS_MAX_PATH, g_cutFileName);
        wcscat_s(g_prdFileName[clientId], OS_MAX_PATH, L"\\");
        wcscat_s(g_cutFileName, OS_MAX_PATH, L"\\");
    }

    wcsncat_s(g_prdFileName[clientId], OS_MAX_PATH, pFileName, (OS_MAX_PATH - 1));

    if (0 != wcscmp(&(pFileName[wcslen(pFileName) - 4]), L".prd"))
    {
        //If the prd file name doesn't have a .prd extension and needs it,
        if (wcslen(pFileName) < (OS_MAX_PATH - 4))
        {
            wcscat_s(g_prdFileName[clientId], OS_MAX_PATH, L".prd");    //add it
        }
        else
        {
            g_errorString[clientId] = L"The path string was too long";
            return E_UNEXPECTED;    //no room for ".prd" in the path string
        }
    }

    wcsncat_s(g_cutFileName, OS_MAX_PATH, pFileName, (OS_MAX_PATH - 1));
    wcscat_s(g_cutFileName, OS_MAX_PATH, L".cut");    //add it

    return S_OK;
}


HRESULT CpuPerfGetCurrentTimeMark(CPA_TIME* pTimeMark)
{
    FILETIME fileTime;
    ULARGE_INTEGER time;

    CoFileTimeNow(&fileTime);

    time.LowPart = fileTime.dwLowDateTime;
    time.HighPart = fileTime.dwHighDateTime;

    // Now fill-in CPA_TIME struct - which contains the time since epoch
    // Epoch is defined as the time 00:00:00 +0000 (UTC) on 1970-01-01.
    //
    // The windows epoch starts 1601-01-01 00:00:00.
    // It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01 00:00:00).
    // The Windows ticks are in 100 nanoseconds.

#define WINDOWS_TICK_PER_SEC 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL

    pTimeMark->second = (gtUInt32)(time.QuadPart / WINDOWS_TICK_PER_SEC - SEC_TO_UNIX_EPOCH);
    pTimeMark->microsec = (gtUInt32)((time.QuadPart - (time.QuadPart / WINDOWS_TICK_PER_SEC)) / 10);

    return S_OK;
}


#if 0
// UNUSED
HRESULT CpuPerfGetSampleCount(gtUInt32* pCount)
{
    gtUInt32 clientId = helpGetClientId();

    HRESULT hr = S_OK;
    DWORD dwReturned;

    if (! InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_GET_RECORD_COUNT, &clientId, sizeof(gtUInt32), pCount,
                      sizeof(gtUInt32), &dwReturned))
    {
        PrintDriverError(GetLastError(), clientId, L" (IOCTL_GET_RECORD_COUNT)", true);
        hr = E_UNEXPECTED;
    }

    return hr;
}
#endif // 0


HRESULT CpuPerfSetTimerConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ bool profileAllCores)
{
    const int APIC_TIMER_MINIMUM = 100;

    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    HRESULT hr = S_OK;

    //verify that period is not too small
    if (*puSPeriod < APIC_TIMER_MINIMUM)
    {
        hr = S_FALSE;
        *puSPeriod = APIC_TIMER_MINIMUM;
    }

    g_timerCfg[clientId].ulClientId = clientId;
    // The driver uses 0.1 mS intervals
    g_timerCfg[clientId].ulGranularity = *puSPeriod / APIC_TIMER_MINIMUM;

    if (true == profileAllCores)
    {
        // Set TIMER_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
        g_timerCfg[clientId].ulCoreMaskCount = 0;
        g_timerCfg[clientId].ullCpuMask.QuadPart = NULL;
    }
    else
    {
        // Calculate the largest core masked and set the core count here
        unsigned int maskCoreCount = GetLargestCoreMasked(&cpuCoreMask, 1);
        g_timerCfg[clientId].ulCoreMaskCount = maskCoreCount;

        // Copy the coremask array
        g_timerCfg[clientId].ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(&cpuCoreMask, maskCoreCount);
    }

    if (SUCCEEDED(hr))
    {
        g_profileType[clientId] |= PROFILE_STATE_TBP_SET;
    }

    return hr;
}


HRESULT CpuPerfSetTimerConfiguration(
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores)
{
    const int APIC_TIMER_MINIMUM = 100;

    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    HRESULT hr = S_OK;

    //verify that period is not too small
    if (*puSPeriod < APIC_TIMER_MINIMUM)
    {
        hr = S_FALSE;
        *puSPeriod = APIC_TIMER_MINIMUM;
    }

    g_timerCfg[clientId].ulClientId = clientId;
    // The driver uses 0.1 mS intervals
    g_timerCfg[clientId].ulGranularity = *puSPeriod / APIC_TIMER_MINIMUM;

    if (true == profileAllCores)
    {
        // Set TIMER_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
        g_timerCfg[clientId].ulCoreMaskCount = 0;
        g_timerCfg[clientId].ullCpuMask.QuadPart = NULL;
    }
    else
    {
        // Calculate the largest core masked and set the core count here
        unsigned int maskCoreCount = GetLargestCoreMasked(pCpuCoreMask, cpuCoreMaskSize);
        g_timerCfg[clientId].ulCoreMaskCount = maskCoreCount;

        // Copy the coremask array
        g_timerCfg[clientId].ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(pCpuCoreMask, maskCoreCount);
    }

    if (SUCCEEDED(hr))
    {
        g_profileType[clientId] |= PROFILE_STATE_TBP_SET;
    }

    return hr;
}


HRESULT CpuPerfSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ bool profileAllCores)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    if (!helpVerifyEventFileAvailable(clientId))
    {
        return E_FAIL;
    }

#if 0
    // Below code is used for debugging VMware BSOD issue on Win7 host
    unsigned int maxPmcCounters = 0;
    fnGetEventCounters(&maxPmcCounters);
    ULONG maxPmcCounterMask = (1 << maxPmcCounters) - 1;

    if ((maxPmcCounterMask & g_Availability.pmcAvailable) != maxPmcCounterMask)
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"All performance counters are not available");
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

#endif

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
            wsprintf(buffer, L"Event 0x%x had a 0 value", evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //counter event should not be set
        if (0 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"Event 0x%x was configured for counting (0x%I64x)",
                     evSelect, pPerformanceEvents[i].performanceEvent);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //are the event and unit mask valid?
        if (!gp_eventsFile->ValidateEvent(evSelect, oneEvent.ucUnitMask))
        {
            wchar_t buffer[120];
            wsprintf(buffer, L"The event (0x%x) and unit mask (0x%x) were not validated%s",
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
                wsprintf(buffer, L"The event (0x%x), unit mask (0x%x), OS (%x) and USR (%x) were duplicated",
                         evSelect, oneEvent.ucUnitMask, oneEvent.bitOsEvents, oneEvent.bitUsrEvents);
                g_errorString[clientId] = buffer;
                return E_INVALIDARG;
            }
        }
    }

     g_EventCfgMode[clientId] |= EVENTCFG_SAMPLE_MODE;

    for (unsigned int j = 0; j < count; j++)
    {
        EVENT_PROPERTIES eventCfg;
        eventCfg.ulClientId = clientId;
        //get the available counters that can be used for this event
        fnGetEventCounterAvailability(pPerformanceEvents[j].performanceEvent,
                                      (unsigned int*) & (pPerformanceEvents[j].eventCounter));
        eventCfg.ulCounterIndex = pPerformanceEvents[j].eventCounter;

        if (true == profileAllCores)
        {
            // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
            eventCfg.ulCoreMaskCount = 0;
            eventCfg.ullCpuMask.QuadPart = NULL;
        }
        else
        {
            // Calculate the largest core masked and set the core count here
            unsigned int maskCoreCount = GetLargestCoreMasked(&cpuCoreMask, 1);
            eventCfg.ulCoreMaskCount = maskCoreCount;

            // Copy the coremask array
            eventCfg.ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(&cpuCoreMask, maskCoreCount);
        }

        eventCfg.ullEventCfg = pPerformanceEvents[j].performanceEvent;
        eventCfg.ullEventCount = pPerformanceEvents[j].value;
        g_eventCfgs.push_back(eventCfg);
    }

    g_eventCfgs.sort(compareMappingOrder);

    getCounterAllocation();

    g_profileType[clientId] |= PROFILE_STATE_EBP_SET;

    return S_OK;
}


HRESULT CpuPerfSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize,
    /*in*/ bool profileAllCores)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling has not been called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    if (!helpVerifyEventFileAvailable(clientId))
    {
        return E_FAIL;
    }

#if 0
    // Below code is used for debugging VMware BSOD issue on Win7 host
    unsigned int maxPmcCounters = 0;
    fnGetEventCounters(&maxPmcCounters);
    ULONG maxPmcCounterMask = (1 << maxPmcCounters) - 1;

    if ((maxPmcCounterMask & g_Availability.pmcAvailable) != maxPmcCounterMask)
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"All performance counters are not available");
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

#endif

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
            wsprintf(buffer, L"Event 0x%x had a 0 value", evSelect);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //counter event should not be set
        if (0 == oneEvent.bitSampleEvents)
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"Event 0x%x was configured for counting (0x%I64x)",
                     evSelect, pPerformanceEvents[i].performanceEvent);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }

        //are the event and unit mask valid?
        if (!gp_eventsFile->ValidateEvent(evSelect, oneEvent.ucUnitMask))
        {
            wchar_t buffer[120];
            wsprintf(buffer, L"The event (0x%x) and unit mask (0x%x) were not validated%s",
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
                wsprintf(buffer, L"The event (0x%x), unit mask (0x%x), OS (%x) and USR (%x) were duplicated",
                         evSelect, oneEvent.ucUnitMask, oneEvent.bitOsEvents, oneEvent.bitUsrEvents);
                g_errorString[clientId] = buffer;
                return E_INVALIDARG;
            }
        }
    }

     g_EventCfgMode[clientId] |= EVENTCFG_SAMPLE_MODE;

    for (unsigned int j = 0; j < count; j++)
    {
        EVENT_PROPERTIES eventCfg;
        eventCfg.ulClientId = clientId;
        //get the available counters that can be used for this event
        fnGetEventCounterAvailability(pPerformanceEvents[j].performanceEvent,
                                      (unsigned int*) & (pPerformanceEvents[j].eventCounter));
        eventCfg.ulCounterIndex = pPerformanceEvents[j].eventCounter;

        if (true == profileAllCores)
        {
            // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
            eventCfg.ulCoreMaskCount = 0;
            eventCfg.ullCpuMask.QuadPart = NULL;
        }
        else
        {
            // Calculate the largest core masked and set the core count here
            unsigned int maskCoreCount = GetLargestCoreMasked(pCpuCoreMask, cpuCoreMaskSize);
            eventCfg.ulCoreMaskCount = maskCoreCount;

            // Copy the coremask array
            eventCfg.ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(pCpuCoreMask, maskCoreCount);
        }

        eventCfg.ullEventCfg = pPerformanceEvents[j].performanceEvent;
        eventCfg.ullEventCount = pPerformanceEvents[j].value;
        g_eventCfgs.push_back(eventCfg);
    }

    g_eventCfgs.sort(compareMappingOrder);

    getCounterAllocation();

    g_profileType[clientId] |= PROFILE_STATE_EBP_SET;

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
    GT_UNREFERENCED_PARAMETER(randomizeFetchSamples);

    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    //verify values
    if ((0 == fetchPeriod) && (0 == opPeriod))
    {
        g_errorString[clientId] = L"There was no period passed in fetchPeriod or opPeriod";
        return E_FAIL;
    }

    if (((fetchPeriod != 0) && (fetchPeriod < 50000)) ||
        (fetchPeriod > MAX_IBS_CYCLE_COUNT))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"fetchPeriod must be between %ld and %ld",
                 50000, MAX_IBS_CYCLE_COUNT);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //Handle extended count and accounting for op count randomization factor, if necessary
    unsigned long maxIbsOps = MAX_IBS_CYCLE_COUNT;
    osCpuid cpuid;

    if (cpuid.isIbsExtCountAvailable())
    {
        maxIbsOps = MAX_IBS_EXT_COUNT;
    }

    if ((opPeriod != 0 && opPeriod < 50000) || (opPeriod > maxIbsOps))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"opPeriod must be between %ld and %ld", 50000, maxIbsOps);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //if the cpu cannot use dispatch sampling and the user directed it
    if ((useDispatchOps) && (! cpuid.isIbsOpsDispatchAvailable()))
    {
        useDispatchOps = false;
        hr = S_FALSE;
    }

    g_ibsCfg[clientId].ulStatus = 0;
    g_ibsCfg[clientId].ulClientId = clientId;

    if (true == profileAllCores)
    {
        // Set IBS_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
        g_ibsCfg[clientId].ulCoreMaskCount = 0;
        g_ibsCfg[clientId].ullCpuMask.QuadPart = NULL;
    }
    else
    {
        // Calculate the largest core masked and set the core count here
        unsigned int maskCoreCount = GetLargestCoreMasked(&cpuCoreMask, 1);
        g_ibsCfg[clientId].ulCoreMaskCount = maskCoreCount;
        // Copy the coremask array
        g_ibsCfg[clientId].ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(&cpuCoreMask, maskCoreCount);
    }

    // Assign the new value.
    if (fetchPeriod > 0)
    {
        // Configure the driver with the value
        g_ibsCfg[clientId].bProfileFetch = TRUE;
        //always try to read the extended information
        g_ibsCfg[clientId].fetchDataMask = (CXL_IBS_FETCH_MASK)(CXL_FETCH_PHYSICAL_ADDR_MASK);

        //check to see if Fetch Control Extended available
        if (cpuid.isIbsFetchCtlExtdAvailable())
        {
            g_ibsCfg[clientId].fetchDataMask = (CXL_IBS_FETCH_MASK)(g_ibsCfg[clientId].fetchDataMask
                                                                    | CXL_FETCH_CONTROL_EXTD_MASK);
        }

        g_ibsCfg[clientId].ulIbsFetchMaxCnt = fetchPeriod;
    }
    else
    {
        g_ibsCfg[clientId].bProfileFetch = FALSE;
        g_ibsCfg[clientId].ulIbsFetchMaxCnt = 0;
        g_ibsCfg[clientId].fetchDataMask = (CXL_IBS_FETCH_MASK)0;
    }

    if (opPeriod > 0)
    {
        // Configure the driver with the value
        g_ibsCfg[clientId].bProfileOp = TRUE;
        g_ibsCfg[clientId].bOpDispatch = useDispatchOps;
        g_ibsCfg[clientId].ulIbsOpMaxCnt = opPeriod;
        //always try to read the extended information
        g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK)(CXL_OP_DATA_2_MASK
                                                          | CXL_OP_DATA_3_MASK | CXL_OP_DC_LINEAR_ADDR_MASK
                                                          | CXL_OP_DC_PHYSICAL_ADDR_MASK);

        //check to see if BR_ADDR available
        if (cpuid.isIbsOpsBrTgtAddrAvailable())
        {
            g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK)(g_ibsCfg[clientId].opDataMask
                                                              | CXL_OP_BR_ADDR_MASK);
        }

        //check to see if Op Data 4 available
        if (cpuid.isIbsOpData4Available())
        {
            g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK)(g_ibsCfg[clientId].opDataMask
                                                              | CXL_OP_DATA_4_MASK);
        }
    }
    else
    {
        g_ibsCfg[clientId].bProfileOp = FALSE;
        g_ibsCfg[clientId].bOpDispatch = FALSE;
        g_ibsCfg[clientId].ulIbsOpMaxCnt = 0;
        g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK) 0;
    }

    if (SUCCEEDED(hr))
    {
        g_profileType[clientId] |= PROFILE_STATE_IBS_SET;
    }

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
    GT_UNREFERENCED_PARAMETER(randomizeFetchSamples);

    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    //verify values
    if ((0 == fetchPeriod) && (0 == opPeriod))
    {
        g_errorString[clientId] = L"There was no period passed in fetchPeriod or opPeriod";
        return E_FAIL;
    }

    if (((fetchPeriod != 0) && (fetchPeriod < 50000)) ||
        (fetchPeriod > MAX_IBS_CYCLE_COUNT))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"fetchPeriod must be between %ld and %ld",
                 50000, MAX_IBS_CYCLE_COUNT);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //Handle extended count and accounting for op count randomization factor, if necessary
    unsigned long maxIbsOps = MAX_IBS_CYCLE_COUNT;
    osCpuid cpuid;

    if (cpuid.isIbsExtCountAvailable())
    {
        maxIbsOps = MAX_IBS_EXT_COUNT;
    }

    if ((opPeriod != 0 && opPeriod < 50000) || (opPeriod > maxIbsOps))
    {
        wchar_t buffer[65];
        wsprintf(buffer, L"opPeriod must be between %ld and %ld", 50000, maxIbsOps);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    //if the cpu cannot use dispatch sampling and the user directed it
    if ((useDispatchOps) && (! cpuid.isIbsOpsDispatchAvailable()))
    {
        useDispatchOps = false;
        hr = S_FALSE;
    }

    g_ibsCfg[clientId].ulStatus = 0;
    g_ibsCfg[clientId].ulClientId = clientId;

    if (true == profileAllCores)
    {
        // Set IBS_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
        g_ibsCfg[clientId].ulCoreMaskCount = 0;
        g_ibsCfg[clientId].ullCpuMask.QuadPart = NULL;
    }
    else
    {
        // Calculate the largest core masked and set the core count here
        unsigned int maskCoreCount = GetLargestCoreMasked(pCpuCoreMask, cpuCoreMaskSize);
        g_ibsCfg[clientId].ulCoreMaskCount = maskCoreCount;
        // Copy the coremask array
        g_ibsCfg[clientId].ullCpuMask.QuadPart = (gtUInt64)CopyCoreMask(pCpuCoreMask, maskCoreCount);
    }

    // Assign the new value.
    if (fetchPeriod > 0)
    {
        // Configure the driver with the value
        g_ibsCfg[clientId].bProfileFetch = TRUE;
        //always try to read the extended information
        g_ibsCfg[clientId].fetchDataMask = (CXL_IBS_FETCH_MASK)(CXL_FETCH_PHYSICAL_ADDR_MASK);

        //check to see if Fetch Control Extended available
        if (cpuid.isIbsFetchCtlExtdAvailable())
        {
            g_ibsCfg[clientId].fetchDataMask = (CXL_IBS_FETCH_MASK)(g_ibsCfg[clientId].fetchDataMask
                                                                    | CXL_FETCH_CONTROL_EXTD_MASK);
        }

        g_ibsCfg[clientId].ulIbsFetchMaxCnt = fetchPeriod;
    }
    else
    {
        g_ibsCfg[clientId].bProfileFetch = FALSE;
        g_ibsCfg[clientId].ulIbsFetchMaxCnt = 0;
        g_ibsCfg[clientId].fetchDataMask = (CXL_IBS_FETCH_MASK)0;
    }

    if (opPeriod > 0)
    {
        // Configure the driver with the value
        g_ibsCfg[clientId].bProfileOp = TRUE;
        g_ibsCfg[clientId].bOpDispatch = useDispatchOps;
        g_ibsCfg[clientId].ulIbsOpMaxCnt = opPeriod;
        //always try to read the extended information
        g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK)(CXL_OP_DATA_2_MASK
                                                          | CXL_OP_DATA_3_MASK | CXL_OP_DC_LINEAR_ADDR_MASK
                                                          | CXL_OP_DC_PHYSICAL_ADDR_MASK);

        //check to see if BR_ADDR available
        if (cpuid.isIbsOpsBrTgtAddrAvailable())
        {
            g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK)(g_ibsCfg[clientId].opDataMask
                                                              | CXL_OP_BR_ADDR_MASK);
        }

        //check to see if Op Data 4 available
        if (cpuid.isIbsOpData4Available())
        {
            g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK)(g_ibsCfg[clientId].opDataMask
                                                              | CXL_OP_DATA_4_MASK);
        }
    }
    else
    {
        g_ibsCfg[clientId].bProfileOp = FALSE;
        g_ibsCfg[clientId].bOpDispatch = FALSE;
        g_ibsCfg[clientId].ulIbsOpMaxCnt = 0;
        g_ibsCfg[clientId].opDataMask = (CXL_IBS_OP_MASK) 0;
    }

    if (SUCCEEDED(hr))
    {
        g_profileType[clientId] |= PROFILE_STATE_IBS_SET;
    }

    return hr;
}


HRESULT CpuPerfSetFilterProcesses(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ bool systemWideProfile,
    /*in*/ bool ignoreChildren)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    // If system-wide profile, do nothing
    if (systemWideProfile)
    {
        return S_OK;
    }

    //clear any previously set pids
    memset(&g_pidCfg[clientId], 0, sizeof(PID_PROPERTIES));

    for (unsigned int i = 0; i < count; i++)
    {
        g_pidCfg[clientId].ullPidArray[i] = pProcessIds[i];
    }

    g_pidCfg[clientId].ulStatus = 0;
    g_pidCfg[clientId].ulClientId = clientId;
    g_pidCfg[clientId].bAddChildrenToFilter = (ignoreChildren) ? false : true;

    return S_OK;
}


HRESULT CpuPerfSetCallStackSampling(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ unsigned int unwindLevel,
    /*in*/ unsigned int samplePeriod,
    /*in*/ CpuProfileCssScope scope,
    /*in*/ bool captureVirtualStack)
{
    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (CP_CSS_SCOPE_UNKNOWN == scope)
    {
        g_errorString[clientId] = L"Invalid CSS scope";
        return E_INVALIDARG;
    }

    unsigned int i;

    for (i = 0; i < count; i++)
    {
        if ((0 == pProcessIds[i]) || (4 == pProcessIds[i]) || (8 == pProcessIds[i]))
        {
            wchar_t buffer[75];
            wsprintf(buffer, L"processId[%d] cannot be a system kernel processes (%ld)",
                     i, pProcessIds[i]);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    HRESULT hr = S_OK;

    for (i = 0; i < count; i++)
    {
        CSS_PROPERTIES cssProp;
        memset(&cssProp, 0, sizeof(CSS_PROPERTIES));

        cssProp.ulClientId = clientId;
        cssProp.ulCSSDepth = unwindLevel;
        cssProp.ulTargetPid = pProcessIds[i];
        cssProp.ulCSSInterval = samplePeriod;

        switch (scope)
        {
            case CP_CSS_SCOPE_USER:
                cssProp.ucTargetSamplingMode = CSS_USER_MODE;
                break;

            case CP_CSS_SCOPE_KERNEL:
                cssProp.ucTargetSamplingMode = CSS_KERNEL_MODE;
                break;

            case CP_CSS_SCOPE_ALL:
                cssProp.ucTargetSamplingMode = CSS_USER_MODE | CSS_KERNEL_MODE;
                break;

            case CP_CSS_SCOPE_UNKNOWN:
                break;
        }

        cssProp.bCaptureVirtualStack = captureVirtualStack ? TRUE : FALSE;

        CSSMODMAP cssModMap;
        BOOL is32Bit;
        fnGetCSSModules(pProcessIds[i], &cssModMap, &is32Bit);

        unsigned int j = 0;
        unsigned int maxCSSModInDriver = InitialCodeRangeBufferSize / sizeof(CSS_CodeRange) - 1;

        if (is32Bit)
        {
            cssProp.ulAddressLen = SIZE_OF_32_ADDR;    //4 bytes for 32-bit
        }
        else
        {
            cssProp.ulAddressLen = SIZE_OF_32_ADDR * 2;    //8 bytes for 64-bit
        }

        cssProp.ulNumCodeRange = static_cast<ULONG>(cssModMap.size());

        for (CSSMODMAP::const_iterator mapIt = cssModMap.begin(), mapEnd = cssModMap.end(); mapIt != mapEnd; ++mapIt)
        {
            cssProp.aCodeRangeInfo[j].startAddr = mapIt->first;
            cssProp.aCodeRangeInfo[j].codeSize = mapIt->second;

            if (++j > maxCSSModInDriver)
            {
                cssProp.ulNumCodeRange = maxCSSModInDriver;
                break;
            }
        }

        fnClearCSSModules(&cssModMap);

        cssProp.ulStatus = 0;

        DWORD dwReturned;

        if (InvokeInOut(g_hCpuProfDevice[clientId], IOCTL_SET_CSS_PROPERTIES, &cssProp, sizeof(CSS_PROPERTIES),
                        &cssProp, sizeof(CSS_PROPERTIES), &dwReturned))
        {
            if (cssProp.ulStatus != PROF_SUCCESS)
            {
                PrintDriverError(cssProp.ulStatus, clientId, L" (IOCTL_SET_CSS_PROPERTIES)");
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            PrintDriverError(GetLastError(), clientId, L" (IOCTL_SET_CSS_PROPERTIES)", true);
            hr = E_UNEXPECTED;
        }

        if (S_OK != hr)
        {
            break;
        }
    }

    return hr;
}


HRESULT CpuPerfClearConfigurations(gtUInt32 clientId)
{
    HRESULT retVal = S_OK;

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    //clear abort state, if current
    if (ProfilingAborted == g_profilingState[clientId])
    {
        g_profilingState[clientId] = ProfilingStopped;
    }

    DWORD dwReturned;

    //With this IOCTL, the driver should clear all configurations from its
    //internal state
    InvokeIn(g_hCpuProfDevice[clientId], IOCTL_CLEAR_PROFILER,
             &clientId, sizeof(gtUInt32), &dwReturned);
    //I'm not checking a return value, since it'll probably error since the
    //profile's not started yet

    if (0 == g_profileType[clientId])
    {
        retVal = S_FALSE;
    }

    g_profileType[clientId] = 0;
    memset(g_aSharedPause[clientId].pauseKey, 0, (OS_MAX_PATH * sizeof(wchar_t)));

    //if applicable, clear event configuration stuff
    g_EventCfgMode[clientId] = EVENTCFG_NOTSET;
    g_eventCfgs.clear();

    //clear any set pids
    memset(&g_pidCfg[clientId], 0, sizeof(PID_PROPERTIES));
    memset(&g_timerCfg[clientId], 0, sizeof(TIMER_PROPERTIES));
    memset(&g_ibsCfg[clientId], 0, sizeof(IBS_PROPERTIES));

    //ensure that the sampling profile file will need to be defined again
    g_prdFileName[clientId][0] = L'\0';
    g_dynamicTiFileName[clientId][0] = L'\0';

    g_CPUUtil.SetDefaultConfig();

    return retVal;
}


HRESULT CpuPerfEnableCPUUtilization(unsigned int utilization_interval, unsigned int monitorFlag)
{
    if (!(monitorFlag & (MONITOR_CPU  | MONITOR_MEM)))
    {
        return S_FALSE;
    }

    gtUInt32 clientId = helpGetClientId();

    if (g_hCpuProfDevice[clientId] == INVALID_HANDLE_VALUE)
    {
        g_errorString[clientId] = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (isProfilingNow(clientId))
    {
        g_errorString[clientId] = L"Profiling is in progress";
        return E_PENDING;
    }

    if (CpuUtilization::evCU_OK != g_CPUUtil.Initialize(monitorFlag))
    {
        return S_FALSE;
    }

    g_CPUUtil.SetInterval(utilization_interval);

    g_profileType[clientId] |= PROFILE_STATE_CU_SET;

    return S_OK;
}


HRESULT CpuPerfAddCPUUtilizationProcessId(unsigned pid)
{
    if (g_CPUUtil.AddPid(pid, false) == CpuUtilization::evCU_OK)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}
