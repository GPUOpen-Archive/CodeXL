// System & Standard header files
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <unistd.h>
#include <fcntl.h>

// Local header files
#include <AMDTMutex.h>
#include "AMDTActivityLogger.h"
#include "AMDTCpuProfileControl.h"

// Max path length of /proc/<pid>/stat
#define AMDT_PROC_STAT_PATH_LEN 30

// Number of bytes to read from /proc/<pid>/stat
#define AMDT_PROC_STAT_BUF_SIZE 100

// Max process tree hierarchy to traverse
#define AMDT_MAX_HIERARCHY 10


/// Holds one client's shared information
typedef struct
{
    /// Whether the client is paused
    bool paused;
    /// The PID of the profiler
    pid_t clientPid;
} CPU_PROF_SHARED_CLIENT;

/// The size of the shared memory
#define CPU_PROF_SHARED_MEM_SIZE (sizeof(CPU_PROF_SHARED_CLIENT))

/// Structure to encapsulate CPU Profile specific data
typedef struct
{
    /// Shared memory file descriptor
    int fd;
    /// Shared client data buffer
    CPU_PROF_SHARED_CLIENT* sharedBuf;
} CPU_PROF_SHARED;

/// Cpu Profiler specific shared data
static CPU_PROF_SHARED _gCpuProfShared = { 0, nullptr };

/// Keep note if shared object initialized
static bool _gIsCpuProfControllerInitialised = false;

/// Mutex to lock Cpu Profiler shared data
static AMDTMutex _gCpuProfMutex;


void initAMDTCpuProfileControl(void);
void __attribute__((destructor)) finiAMDTCpuProfileControl(void);


// Read /proc/<pid>/stat file to extract the parent pid
pid_t getParentPid(pid_t pid)
{
    char psPath[AMDT_PROC_STAT_PATH_LEN] = { 0 };
    pid_t ppid = (pid_t)(-1);

    sprintf(psPath, "/proc/%d/stat", pid);
    FILE* fp = fopen(psPath, "r");

    if (nullptr != fp)
    {
        char buf[AMDT_PROC_STAT_BUF_SIZE] = { 0 };

        // read first AMDT_PROC_STAT_BUF_SIZE bytes, sufficient to get first 4 fields
        if (fread(buf, AMDT_PROC_STAT_BUF_SIZE, 1, fp) > 0)
        {
            char* pbuf = nullptr;

            // Format of /proc/<pid>/stat file:
            // pid (command) state ppid ...
            // Skip till beginning of command name
            for (pbuf = buf; *pbuf != '('; ++pbuf) { ; }

            // Skip till max command name end char
            // Max command name length is 15 char + ')'
            pbuf += 16;

            // Skip reverse till actual command name end char
            for (; *pbuf != ')'; --pbuf) { ; }

            // Skip state char + space delimiters
            pbuf += 4;
            // pbuf points to beginning of ppid
            ppid = atoi(pbuf);
        }

        fclose(fp);
    }

    return ppid;
}

bool generateSharedObjName(pid_t pid, char* name)
{
    sprintf(name, "/amd_cxl_cpuprof_%d", pid);
    return true;
}

/// Resume CPU profiling
int AMDTCpuProfileResume(void)
{
    AMDTScopeLock lock(_gCpuProfMutex);

    if (!_gIsCpuProfControllerInitialised)
    {
        initAMDTCpuProfileControl();
    }

    int rc = AL_SUCCESS;

    // check if shared object is available
    if (-1 == _gCpuProfShared.fd || nullptr == _gCpuProfShared.sharedBuf)
    {
        rc = AL_INTERNAL_ERROR;
    }

    // check if profiling already resumed
    if (AL_SUCCESS == rc)
    {
        if (_gCpuProfShared.sharedBuf->paused)
        {
            _gCpuProfShared.sharedBuf->paused = false;
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
    AMDTScopeLock lock(_gCpuProfMutex);

    int rc = AL_SUCCESS;

    if (!_gIsCpuProfControllerInitialised)
    {
        initAMDTCpuProfileControl();
    }

    // check if handle is valid
    if (-1 == _gCpuProfShared.fd || nullptr == _gCpuProfShared.sharedBuf)
    {
        rc = AL_INTERNAL_ERROR;
    }

    // check if profiling already paused
    if (AL_SUCCESS == rc)
    {
        if (_gCpuProfShared.sharedBuf->paused)
        {
            rc = AL_WARN_PROFILE_ALREADY_PAUSED;
        }
        else
        {
            _gCpuProfShared.sharedBuf->paused = true;
        }
    }

    return rc;
}



/// Initialize CPU Profiler control interface
void initAMDTCpuProfileControl(void)
{
    bool rc = true;
    int oflags = O_RDWR;
    mode_t mode = S_IRUSR | S_IWUSR;
    char name[NAME_MAX] = {0};
    size_t size = CPU_PROF_SHARED_MEM_SIZE;
    int fd = -1;
    void* mapAddr = MAP_FAILED;
    pid_t ppid = getppid();

    // Try to open the shared object (up to max 10 hierarchy up)
    for (int i = 0; i < AMDT_MAX_HIERARCHY; ++i)
    {
        // Generate shared object name
        generateSharedObjName(ppid, name);

        fd = shm_open(name, oflags, mode);

        if (fd >= 0)
        {
            // Found and opened the shared object
            break;
        }

        ppid = getParentPid(ppid);

        if (ppid <= 0)
        {
            // Reached the root of the process tree
            break;
        }
    }

    if (fd >= 0)
    {
        int protection = PROT_READ | PROT_WRITE;
        int mflags = MAP_SHARED;
        off_t offset = 0;

        mapAddr = mmap(NULL, size, protection, mflags, fd, offset);

        if (MAP_FAILED == mapAddr)
        {
            shm_unlink(name);
            fd = -1;
            rc = false;
        }
    }
    else
    {
        rc = false;
    }

    if (rc)
    {
        _gCpuProfShared.fd = fd;
        _gCpuProfShared.sharedBuf = static_cast<CPU_PROF_SHARED_CLIENT*>(mapAddr);
    }

    _gIsCpuProfControllerInitialised = true;
}


/// Close CPU Profiler control interface
void __attribute__((destructor)) finiAMDTCpuProfileControl(void)
{
    if (_gIsCpuProfControllerInitialised)
    {
        if (nullptr != _gCpuProfShared.sharedBuf)
        {
            munmap(_gCpuProfShared.sharedBuf, CPU_PROF_SHARED_MEM_SIZE);
            _gCpuProfShared.sharedBuf = nullptr;
        }

        if (_gCpuProfShared.fd >= 0)
        {
            // Do not call shm_unlink(), just set fd to invalid value
            // Shared object will be deleted by the creator - cpu profiler
            _gCpuProfShared.fd = -1;
        }

        _gIsCpuProfControllerInitialised = false;
    }
}

bool AMDTCpuProfileControlClose(void)
{
    finiAMDTCpuProfileControl();

    return true;
}