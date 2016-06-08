// System & Standard header files
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>

#include <AMDTMutex.h>
#include "AMDTActivityLogger.h"
#include "AMDTCpuProfileControl.h"


/// Cpu profiler shared object name
#define AMDT_CPU_PROF_SHARED_OBJ_NAME L"Global\\AMD_CPUPROF_SHARED_OBJ"

/// Cpu profiler key to pause
#define AMDT_CPU_PROF_PAUSE_KEY L"AMD Cpu Profiling"

#define AMDT_INVALID_CLIENT ((AMDTClientIdType)(-1))
#define AMDT_INVALID_PID    ((AMDTPidType)(-1))

/// The maximum number of clients that CpuProf supports
#define AMDT_CPU_PROF_MAX_CLIENT_COUNT 8

// Max process tree hierarchy to traverse
#define AMDT_MAX_HIERARCHY 10

typedef uint32_t AMDTClientIdType;
typedef DWORD AMDTPidType;

/// Holds one client's shared information
typedef struct
{
    /// Whether the client is paused
    BOOLEAN paused;
    /// The string 'pause key' for a profile
    wchar_t pauseKey[_MAX_PATH];
    /// The PID of the profiler
    DWORD clientPid;

} CPU_PROF_SHARED_CLIENT;

/// Structure to encapsulate CPU Profile specific data
typedef struct
{
    /// Shared memory map handle
    HANDLE handle;

    /// Shared client data buffer
    CPU_PROF_SHARED_CLIENT* sharedBuf;

} CPU_PROF_SHARED;

/// The size of the shared memory
#define CPU_PROF_SHARED_MEM_SIZE (sizeof(CPU_PROF_SHARED_CLIENT) * AMDT_CPU_PROF_MAX_CLIENT_COUNT)

/// Keep note if shared object initialized
static bool _gIsCpuProfControllerInitialised = false;

/// Cpu Profiler specific shared data
static CPU_PROF_SHARED _gCpuProfShared = { nullptr, nullptr };

/// Profiler process id = PID of CodeXL GUI or CLI
static AMDTPidType _gCpuProfilerPid = AMDT_INVALID_PID;

/// Mutex to lock Cpu Profiler shared data
static AMDTMutex _gCpuProfMutex;


/// Fetch the current process id
AMDTPidType getCurrentPid(void)
{
    AMDTPidType pid = GetCurrentProcessId();
    return pid;
}


/// Extract the parent process id
AMDTPidType getParentPid(AMDTPidType pid)
{
    AMDTPidType ppid = AMDT_INVALID_PID;
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (INVALID_HANDLE_VALUE != h)
    {
        PROCESSENTRY32 pe32 = { 0 };
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(h, &pe32))
        {
            do
            {
                if (pe32.th32ProcessID == pid)
                {
                    ppid = pe32.th32ParentProcessID;
                    break;
                }
            }
            while (Process32Next(h, &pe32));
        }

        CloseHandle(h);
    }

    return ppid;
}


/// Fetch the current client id
AMDTClientIdType getClientId(const wchar_t* pauseKey, AMDTPidType profilerPid)
{
    AMDTClientIdType clientId = AMDT_INVALID_CLIENT;
    uint32_t i;

    for (i = 0; i < AMDT_CPU_PROF_MAX_CLIENT_COUNT; i++)
    {
        if (profilerPid != _gCpuProfShared.sharedBuf[i].clientPid)
        {
            continue;
        }

        if (L'\0' == _gCpuProfShared.sharedBuf[i].pauseKey[0])
        {
            continue;
        }

        if (0 == wcscmp(pauseKey, _gCpuProfShared.sharedBuf[i].pauseKey))
        {
            break;
        }
    }

    if (i < AMDT_CPU_PROF_MAX_CLIENT_COUNT)
    {
        clientId = i;
    }

    return clientId;
}


/// Fetch profiler PID
AMDTPidType getCpuProfilerPid()
{
    AMDTPidType ppid = getParentPid(getCurrentPid());

    // Search profiler PID (up to max 10 hierarchy up)
    for (int i = 0; i < AMDT_MAX_HIERARCHY; ++i)
    {
        if (ppid == AMDT_INVALID_PID)
        {
            // Reached the root of the process tree
            break;
        }

        if (getClientId(AMDT_CPU_PROF_PAUSE_KEY, ppid) != AMDT_INVALID_CLIENT)
        {
            // Found profiler entry in the shared object
            break;
        }

        ppid = getParentPid(ppid);
    }

    // Invalid pid OR profiler pid OR some (grand-)parent pid
    return ppid;
}


/// Initialize CPU Profiler control interface
bool AMDTCpuProfileControlInit(void)
{
    bool rc = false;
    HANDLE handle = nullptr;
    CPU_PROF_SHARED_CLIENT* sharedBuf = nullptr;

    handle = OpenFileMappingW(FILE_MAP_ALL_ACCESS,    // read/write access
                              FALSE,                  // do not inherit the name
                              AMDT_CPU_PROF_SHARED_OBJ_NAME);   // name of mapping object

    if (nullptr != handle)
    {
        sharedBuf = (CPU_PROF_SHARED_CLIENT*)MapViewOfFile(handle,                         // handle to mapping object
                                                           FILE_MAP_READ | FILE_MAP_WRITE, // read/write permission
                                                           0,
                                                           0,
                                                           CPU_PROF_SHARED_MEM_SIZE);

        if (nullptr == sharedBuf)
        {
            CloseHandle(handle);
            handle = nullptr;
        }
        else
        {
            rc = true;
        }
    }

    // save the information
    _gCpuProfShared.handle = handle;
    _gCpuProfShared.sharedBuf = sharedBuf;

    if (rc)
    {
        _gCpuProfilerPid = getCpuProfilerPid();
    }

    _gIsCpuProfControllerInitialised = true;

    return rc;
}


/// Close CPU Profiler control interface
bool AMDTCpuProfileControlClose()
{
    if (_gIsCpuProfControllerInitialised)
    {
        if (nullptr != _gCpuProfShared.sharedBuf)
        {
            UnmapViewOfFile(_gCpuProfShared.sharedBuf);
        }

        if (nullptr != _gCpuProfShared.handle)
        {
            CloseHandle(_gCpuProfShared.handle);
        }

        // clear the stale information
        _gCpuProfShared.handle = nullptr;
        _gCpuProfShared.sharedBuf = nullptr;

        _gIsCpuProfControllerInitialised = false;
    }

    return true;
}

/// Resume CPU profiling
int AMDTCpuProfileResume(void)
{
    int rc = AL_SUCCESS;
    AMDTClientIdType clientId = AMDT_INVALID_CLIENT;

    AMDTScopeLock lock(_gCpuProfMutex);

    if (!_gIsCpuProfControllerInitialised)
    {
        AMDTCpuProfileControlInit();
    }

    // check if handle is valid
    if (nullptr == _gCpuProfShared.handle || nullptr == _gCpuProfShared.sharedBuf)
    {
        rc = AL_INTERNAL_ERROR;
    }

    if (AL_SUCCESS == rc)
    {
        clientId = getClientId(AMDT_CPU_PROF_PAUSE_KEY, _gCpuProfilerPid);

        if (clientId == AMDT_INVALID_CLIENT)
        {
            rc = AL_INTERNAL_ERROR;
        }
    }

    // check if profiling already resumed
    if (AL_SUCCESS == rc)
    {
        if (_gCpuProfShared.sharedBuf[clientId].paused)
        {
            _gCpuProfShared.sharedBuf[clientId].paused = FALSE;
        }
        else
        {
            rc = AL_WARN_PROFILE_ALREADY_RESUMED;
        }
    }

    return rc;
}


/// Pause CPU profiling
int AMDTCpuProfilePause(void)
{
    int rc = AL_SUCCESS;
    AMDTClientIdType clientId = AMDT_INVALID_CLIENT;

    AMDTScopeLock lock(_gCpuProfMutex);

    if (!_gIsCpuProfControllerInitialised)
    {
        AMDTCpuProfileControlInit();
    }

    // check if handle is valid
    if (nullptr == _gCpuProfShared.handle || nullptr == _gCpuProfShared.sharedBuf)
    {
        rc = AL_INTERNAL_ERROR;
    }

    if (AL_SUCCESS == rc)
    {
        clientId = getClientId(AMDT_CPU_PROF_PAUSE_KEY, _gCpuProfilerPid);

        if (clientId == AMDT_INVALID_CLIENT)
        {
            rc = AL_INTERNAL_ERROR;
        }
    }

    // check if profiling already paused
    if (AL_SUCCESS == rc)
    {
        if (_gCpuProfShared.sharedBuf[clientId].paused)
        {
            rc = AL_WARN_PROFILE_ALREADY_PAUSED;
        }
        else
        {
            _gCpuProfShared.sharedBuf[clientId].paused = TRUE;
        }
    }

    return rc;
}

