//=============================================================
// (c) 2013 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief  Power profile internal common driver interface APIs
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $Rajeeb Barman
// Change list:    $Change: $
//=============================================================

// OSWRAPPER INCLUDES
#include <AMDTOSWrappers/Include/osCondition.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>

// PROJECT INCLUDES
#include <AMDTDefinitions.h>
#include <AMDTPwrProfAttributes.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTPowerProfileDataTypes.h>
#include <AMDTPowerProfileApi.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTSharedBufferConfig.h>
#include <PowerProfileDriverInterface.h>
#include <PowerProfileHelper.h>
#include <ppStringConstants.h>
#include <AMDTDriverTypedefs.h>

// SYSTEM INCLUDE
#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <poll.h>
#include <thread>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PCORE_DEVICE_FILE_NAME      "/dev/amdtPwrProf"
#define OLD_PCORE_DEVICE_FILE_NAME  "/dev/pcore"

// static defines
static int g_power_drv_handle       = INVALID_HANDLE_VALUE;
static AMDTUInt32 g_clientId        = 0xFFFFFFFF;
static int g_sharedFd               = -1;
static DRV_SIGINFO_CB g_eventCb     = nullptr;

// global define
AMDTUInt8* g_pSharedBuffer          = nullptr;

//Function pointers for internal API
fpSmuActivate g_fnSmuActivate = nullptr;

namespace PwrProfDrvInterface
{
// thread synchronisation variable
osCondition         g_closeFdCondition;
AMDTUInt32          g_stopProfile;

class ProcessDataThread : private osThread
{
public:
    ProcessDataThread(AMDTUInt32 samplingPeriod);

private:
    virtual int entryPoint();
    virtual void beforeTermination();

    // data members
    AMDTUInt32 m_samplingPeriod;
};

ProcessDataThread::ProcessDataThread(AMDTUInt32 samplingPeriod)
    : osThread(L"ProcessDataThread"), m_samplingPeriod(samplingPeriod)
{
    osThread::execute();
}

// entry point for thread
// Poll and collect data from the driver
int ProcessDataThread::entryPoint()
{
    int ret = -1;
    struct pollfd pollFd;
    pollFd.fd = g_sharedFd;
    pollFd.events = POLLIN;

    DriverSignalInfo CbInfo;
    memset(&CbInfo, 0 , sizeof(DriverSignalInfo));

    // to be in sync with main thread acquire conditional
    // lock when profile starts triggers polling.
    g_closeFdCondition.lockCondition();

    while (false == g_stopProfile)
    {
        // poll waits for POLLIN event or
        // expire after 1000 msec if smapling period is
        // less than 1 sec, otherwise sampling period + 1 sec
        AMDTUInt32 waitMS = 1000;

        if (m_samplingPeriod >= 1000)
        {
            waitMS = m_samplingPeriod + 1000;
        }

        ret = poll(&pollFd, 1, waitMS);

        if (ret > 0)
        {
            if (POLLIN & pollFd.revents)
            {
                g_eventCb(&CbInfo);
            }
        }
        else
        {
            PwrTrace("Poll failed : 0x%x \n", ret);
            break;
        }
    }

    // profile stopped, release conditional lock.
    if (ret == g_closeFdCondition.unlockCondition())
    {
        PwrTrace("Unlocking g_closeFdCondition fails.");
    }

    // signal main thread to close file descriptor
    if (ret == (g_closeFdCondition.signalAllThreads() && ret))
    {
        PwrTrace("Signalling all thread fails.");
    }

    return ret;
}

void ProcessDataThread::beforeTermination()
{
    // nothing to clean
}

ProcessDataThread* pProcessDataThread = nullptr;
} // PwrProfDrvInterface


AMDTResult OpenPowerProfileDriver()
{
    AMDTInt32 ret           = AMDT_STATUS_OK;

    // get the device file handle for /dev/amdtPwrProf
    g_power_drv_handle = open(PCORE_DEVICE_FILE_NAME, O_RDWR);

    // file descriptor is invalid
    if (INVALID_HANDLE_VALUE == g_power_drv_handle)
    {
        // old dev file named pcore might be installed
        g_power_drv_handle = open(OLD_PCORE_DEVICE_FILE_NAME, O_RDWR);

        if (INVALID_HANDLE_VALUE == g_power_drv_handle)
        {
            ret = AMDT_ERROR_FAIL;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        uint64_t version;

        if (ioctl(g_power_drv_handle, IOCTL_GET_VERSION, &version) < 0)
        {
            ret = AMDT_ERROR_FAIL;
            return ret;
        }

        uint32_t version_major = (version >> 32) & 0xFFFF ;
        uint32_t version_minor = version & 0xFFFF;

        if ((version_major != LINUX_PWR_DRV_MAJOR) ||
            (version_minor != LINUX_PWR_DRV_MINOR)
           )
        {
            printf(PP_STR_IncomptibleDriverMessageHeader);
            printf(PP_STR_IncomptibleDriverMessageLine1, version_major, version_minor);
            printf(PP_STR_IncomptibleDriverMessageLine2, LINUX_PWR_DRV_MAJOR, LINUX_PWR_DRV_MINOR);
            printf(PP_STR_IncomptibleDriverMessageLine3);
            ret = AMDT_DRIVER_VERSION_MISMATCH;
        }

    }

    return ret;
}

AMDTResult RegisterClientWithDriver(AMDTUInt32* pClientId)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (INVALID_HANDLE_VALUE == g_power_drv_handle)
    {
        ret = AMDT_ERROR_FAIL;
        return ret;
    }

    // Register the client
    if (ioctl(g_power_drv_handle, IOCTL_REGISTER_CLIENT, pClientId) == -1)
    {
        ret = AMDT_ERROR_FAIL;
        return ret;
    }

    g_clientId = *pClientId;
    return ret;
}

//GetPowerDriverClientId:
AMDTResult GetPowerDriverClientId(AMDTUInt32* pClientId)
{
    *pClientId = g_clientId;
    return (g_clientId <= MAX_CLIENT_COUNT) ? AMDT_STATUS_OK : S_FALSE;
}


bool IsPowerDriverOpened(void)
{
    if (INVALID_HANDLE_VALUE == g_power_drv_handle)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// FlushDriverSignal:
// Send the signal to collect remaining data from driver
AMDTResult FlushDriverSignal()
{
    AMDTResult ret = AMDT_STATUS_OK;

    // to sync with ProcessDataThread
    PwrProfDrvInterface::g_stopProfile = true;

    if (PwrProfDrvInterface::g_closeFdCondition.isConditionLocked())
    {
        // Make this thread wait:
        PwrProfDrvInterface::g_closeFdCondition.waitForCondition();
    }

    // g_closeFdCondition is unlocked free to close the g_sharedFd
    if (nullptr != g_pSharedBuffer)
    {
        int rc = munmap(g_pSharedBuffer, PWRPROF_SHARED_BUFFER_SIZE);

        if ((g_sharedFd >= 0) && (rc == 0))
        {
            rc = close(g_sharedFd);
        }
        else
        {
            ret = AMDT_ERROR_FAIL;
        }

        g_pSharedBuffer = nullptr;

        // delete thread
        delete PwrProfDrvInterface::pProcessDataThread;
        PwrProfDrvInterface::pProcessDataThread = nullptr;
    }

    return ret;
}


// GetSharedMemoryBuffer :
// Return mmaped address of the shared buffer.
AMDTUInt8* GetSharedMemoryBuffer(int devFd)
{
    int fileDescriptor = -1;
    AMDTUInt8* address = NULL;

    if (-1 != ioctl(devFd, IOCTL_SET_AND_GET_FD, &fileDescriptor))
    {
        address = (AMDTUInt8*)mmap(NULL,
                                   PWRPROF_SHARED_BUFFER_SIZE,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   fileDescriptor,
                                   0);

        if (address == MAP_FAILED)
        {
            PwrTrace("Failed to memory map shared buffer.");
            address = NULL;
        }

        g_sharedFd = fileDescriptor;
    }
    else
    {
        PwrTrace("Power Profiler: IOCTL call IOCTL_SET_AND_GET_FD ");
    }

    return address;
}

// InitializeSharedBuffer:
// Create and initialize shared buffer between driver and user spce
AMDTResult InitializeSharedBuffer()
{
    AMDTResult ret = AMDT_STATUS_OK;

    // assign g_pSharedBuffer buffer with mmap address
    g_pSharedBuffer = GetSharedMemoryBuffer(g_power_drv_handle);

    if (NULL == g_pSharedBuffer)
    {
        PwrTrace("InitializeSharedBuffer: Failed to map shared buffer.");
        ret = AMDT_ERROR_OUTOFMEMORY;
    }

    return ret;
}

// DriverSignalInitialize: Driver sends signal based on data buffer availability
AMDTResult DriverSignalInitialize(DRV_SIGINFO_CB fnDrvSignalInfo, AMDTUInt32 samplingPeriod)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == fnDrvSignalInfo)
    {
        PwrTrace("fnDrvSignalInfo is NULL");
        ret = AMDT_ERROR_FAIL;
    }
    else
    {
        g_eventCb = fnDrvSignalInfo;
        // start profiling, unset g_stopProfile
        PwrProfDrvInterface::g_stopProfile = false;

        // create process data thread.
        PwrProfDrvInterface::pProcessDataThread = new PwrProfDrvInterface::ProcessDataThread(samplingPeriod);

        if (nullptr == PwrProfDrvInterface::pProcessDataThread)
        {
            PwrTrace("Not able to allocate memory for ProcessDataThread.");
        }
    }

    return ret;
}

//CommandPowerDriver:
AMDTResult CommandPowerDriver(DriverCommandType cmdType,
                              AMDTUInt32 cmd,
                              void* pInData,
                              AMDTUInt32 inLen,
                              void* pOutData,
                              AMDTUInt32 outlen,
                              void* pResult)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    // To avoid compiler warning
    // using un-used variable
    cmdType = cmdType;
    inLen++;
    pOutData = pOutData;
    outlen++;
    pResult = pResult;

    switch (cmd)
    {
        case IOCTL_ADD_PROF_CONFIGS:
            ret = ioctl(g_power_drv_handle, IOCTL_ADD_PROF_CONFIGS , PPROF_CONFIGS(pInData));
            break;

        case IOCTL_START_PROFILER:
            ret = ioctl(g_power_drv_handle, IOCTL_START_PROFILER, PPROFILER_PROPERTIES(pInData));
            break;

        case IOCTL_STOP_PROFILER:
            ret = ioctl(g_power_drv_handle, IOCTL_STOP_PROFILER, PPROFILER_PROPERTIES(pInData));
            break;

        case IOCTL_PAUSE_PROFILER:
            ret = ioctl(g_power_drv_handle, IOCTL_PAUSE_PROFILER, PPROFILER_PROPERTIES(pInData));
            break;

        case IOCTL_RESUME_PROFILER:
            ret = ioctl(g_power_drv_handle, IOCTL_RESUME_PROFILER, PPROFILER_PROPERTIES(pInData));
            break;

        case IOCTL_GET_FILE_HEADER_BUFFER:
            ret = ioctl(g_power_drv_handle, IOCTL_GET_FILE_HEADER_BUFFER, PFILE_HEADER(pInData));
            break;

        case IOCTL_GET_DATA_BUFFER:
            ret = ioctl(g_power_drv_handle, IOCTL_GET_DATA_BUFFER, PDATA_BUFFER(pInData));
            break;

        case IOCTL_UNREGISTER_CLIENT:
            ret = ioctl(g_power_drv_handle, IOCTL_UNREGISTER_CLIENT, (AMDTUInt32*)pInData);
            break;

        case IOCTL_ACCESS_MSR:
            ret = ioctl(g_power_drv_handle, IOCTL_ACCESS_MSR, (PACCESS_MSR)pInData);
            break;

        case IOCTL_ACCESS_PCI_DEVICE:
            ret = ioctl(g_power_drv_handle, IOCTL_ACCESS_PCI_DEVICE, (PACCESS_PCI)pInData);
            break;

        case IOCTL_ACCESS_MMIO:
            ret = ioctl(g_power_drv_handle, IOCTL_ACCESS_MMIO, (PACCESS_MMIO)pInData);
            break;

        default:
            // HANDLE ERROR CASE
            ret = AMDT_ERROR_FAIL;
            break;
    }

    return ret;
}

// GetProcessNameFromPid: Get process name from process id
bool GetProcessNameFromPid(AMDTPwrProcessInfo* pInfo)
{
    bool found = false;
    pid_t pid           = (pid_t)pInfo->m_pid;
    gtSize_t maxNameLen     = AMDT_PWR_EXE_NAME_LENGTH - 1;
    gtString path;
    osModuleArchitecture moduleArchitecture;
    osRuntimePlatform currentPlatform;
    gtString commandLine;
    gtString workingDirectory;

    if (nullptr != pInfo)
    {
        if (0 == pid)
        {
            memcpy(pInfo->m_name, AMDT_PWR_STR_SYSTEM_IDLE, strlen(AMDT_PWR_STR_SYSTEM_IDLE) + 1);
            memcpy(pInfo->m_path, "", strlen("") + 1);
            found = true;
        }
        else
        {
            if (osGetProcessIdentificationInfo(pid, NULL, NULL, pInfo->m_name, &maxNameLen))
            {
                if (true == osGetProcessLaunchInfo(pid, moduleArchitecture,
                                                   currentPlatform, path,
                                                   commandLine, workingDirectory))
                {
                    memcpy(pInfo->m_path, path.asASCIICharArray(), path.length());
                }
                else
                {
                    memcpy(pInfo->m_path, ERROR_READING_PROCESS_PATH, strlen(ERROR_READING_PROCESS_PATH) + 1);
                }

                found = true;
            }
            else
            {
                found = false;
            }
        }
    }

    return found;
}

// PrepareInitialProcessList: Prepare the initial process list before the
// profile session
AMDTResult PrepareInitialProcessList(list<ProcessName>& list)
{
    AMDTResult ret = AMDT_STATUS_OK;
    list.clear();

#if 0
    osProcessesEnumerator processEnum;


    if (processEnum.initialize())
    {
        osProcessId processId;
        gtString executableName;
        ProcessName process;

        while (processEnum.next(processId, &executableName))
        {
            process.m_pid = processId;
            strncpy(process.m_name, executableName.asASCIICharArray(), AMDT_PWR_PID_MAX_NAME_LENGTH);
            list.push_back(process);
        }
    }

#endif

    return ret;
}

// EnableSmu: enable Smu features
bool EnableSmu(bool activate)
{
    bool retVal = false;

    if (nullptr != g_fnSmuActivate)
    {
        retVal = g_fnSmuActivate(activate);
    }

    return retVal;
}

// PwrApiCleanUp: Cleaning up Apis in case of unexpected abort
bool PwrApiCleanUp(void)
{
    return EnableSmu(false);
}

