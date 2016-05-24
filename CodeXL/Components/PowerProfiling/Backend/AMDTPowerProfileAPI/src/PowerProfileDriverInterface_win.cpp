//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileDriverInterface_win.cpp
///
//==================================================================================

#include <PowerProfileHelper.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTDefinitions.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTPowerProfileDataTypes.h>
#include <AMDTRawDataFileHeader.h>
#include <PowerProfileDriverInterface.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTPwrProfAttributes.h>
#include <iostream>
#include <thread>
#include <Sddl.h>
#include <objbase.h>
#include <Psapi.h>
#include <Ntsecapi.h>
#include <qstring.h>
#include <AMDTSharedBufferConfig.h>
#include <AMDTSharedObjPath.h>
#include <list>
#include <windows.h>
#include <tlhelp32.h>
static bool isFlashedSignal = false;
static EventCfg g_eventCfg;
static HANDLE g_sharedFile = nullptr;
static HANDLE g_process;
AMDTUInt8* g_pSharedBuffer = nullptr;

#define POWER_DRIVER_IN InvokeIn
#define POWER_DRIVER_OUT InvokeOut
#define POWER_DRIVER_IN_OUT InvokeInOut
static HANDLE g_powerDrvHld = INVALID_HANDLE_VALUE;

static wchar_t g_drvPath[OS_MAX_PATH + 1];
static AMDTUInt32 g_clientIdx = 0;
AMDTResult OpenPowerProfileDriver()
{
    wchar_t systemDir[OS_MAX_PATH];
    systemDir[0] = '\0';
    wchar_t g_pcorePath[OS_MAX_PATH + 1];
    HANDLE g_pcoreDrvHld = INVALID_HANDLE_VALUE;
    //printf("\n OpenPowerProfileDriver inside\n");
    GetSystemDirectoryW(systemDir, OS_MAX_PATH);
    swprintf_s(g_pcorePath, OS_MAX_PATH, L"%s%s", systemDir, L"\\drivers\\PCORE");
    swprintf_s(g_drvPath, OS_MAX_PATH, L"%s%s", systemDir, L"\\drivers\\AMDTPWRPROF");
    DWORD err;
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt64 version = 0;
    DWORD dwReturned;
    g_eventCfg.m_isActive = false;

    OpenAmdDriver((LPCTSTR)g_pcorePath, &g_pcoreDrvHld);
    OpenAmdDriver((LPCTSTR)g_drvPath, &g_powerDrvHld);

    if ((INVALID_HANDLE_VALUE == g_powerDrvHld) || (INVALID_HANDLE_VALUE == g_pcoreDrvHld))
    {
        err = GetLastError();

        if (ERROR_ACCESS_DENIED == err)
        {
            ret = AMDT_ERROR_ACCESSDENIED;
        }
        else if (ERROR_FILE_NOT_FOUND == err)
        {
            ret = AMDT_ERROR_DRIVER_UNAVAILABLE;
        }
        else
        {
            ret = AMDT_ERROR_UNEXPECTED;
        }

        PwrTrace("OpenPowerProfileDriver failed as INVALID_HANDLE_VALUE failed");
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Check driver version for compatibility.
        if (POWER_DRIVER_OUT(g_powerDrvHld, IOCTL_GET_VERSION, &version, sizeof(AMDTUInt64), &dwReturned))
        {
            uint64 currVersion = POWER_PROFILE_DRIVER_VERSION;

            //Check supported version.
            if ((currVersion >> 48)  != (version >> 48))
            {
                ret = AMDT_DRIVER_VERSION_MISMATCH;
            }
        }
        else
        {
            ret = AMDT_ERROR_FAIL;
            err = GetLastError();

            PwrTrace("OpenPowerProfileDriver failed as IOCTL_GET_VERSION failed");
        }
    }

    return ret;
}

//OpenPowerProfileDriver: Register client with driver's client list
AMDTResult RegisterClientWithDriver(AMDTUInt32* pClientId)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 clientIdx = 0xFF;
    DWORD dwReturned;

    //Opend driver succesfully. Register the driver now
    if (!POWER_DRIVER_OUT(g_powerDrvHld, IOCTL_REGISTER_CLIENT, &clientIdx, sizeof(AMDTUInt32), &dwReturned))
    {
        ret = AMDT_ERROR_FAIL;
        PwrTrace("RegisterClientWithDriver failed as IOCTL_REGISTER_CLIENT failed");
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pClientId = clientIdx;
        g_clientIdx = clientIdx;
    }

    return ret;
}

//GetPowerDriverClientId: get the cliend id of the driver
AMDTResult GetPowerDriverClientId(AMDTUInt32* pClientId)
{
    *pClientId = g_clientIdx;
    return (g_clientIdx <= MAX_CLIENT_COUNT) ? AMDT_STATUS_OK : S_FALSE;
}
bool InitializeTaskInfo();

bool IsPowerDriverOpened(void)
{
    InitializeTaskInfo();

    if (INVALID_HANDLE_VALUE == g_powerDrvHld)
    {
        return false;
    }
    else
    {
        return true;
    }
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
    BOOL status = false;

    if (cmdType == DRV_CMD_TYPE_OUT)
    {
        status = POWER_DRIVER_OUT(g_powerDrvHld, cmd, pOutData, (DWORD)outlen, (DWORD*)pResult);
    }
    else if (cmdType == DRV_CMD_TYPE_IN)
    {
        status = POWER_DRIVER_IN(g_powerDrvHld, cmd, pInData, inLen, (DWORD*)pResult);
    }
    else if (cmdType == DRV_CMD_TYPE_IN_OUT)
    {
        status = POWER_DRIVER_IN_OUT(g_powerDrvHld, cmd, pInData, inLen, pOutData, outlen, (DWORD*)pResult);
    }

    if (status == false)
    {
        ret = AMDT_ERROR_FAIL;
    }

    return ret;
}

// ProcessDriverMessage: Wait for driver event and process the callback
DWORD WINAPI ProcessDriverMessage(LPVOID iVal)
{
    DWORD res = 0;

    DriverSignalInfo info;
    (void)iVal;
    info.m_fill = 10;

    while (g_eventCfg.m_isActive)
    {
        res = WaitForSingleObject(g_eventCfg.m_hdl, INFINITE);

        if (WAIT_OBJECT_0 == res)
        {
            g_eventCfg.m_eventCb(&info);
        }

        if (true == isFlashedSignal)
        {
            break;
        }
    }

    return res;
}
// DriverSignalInitialize: Driver sends signal based on data buffer availability
AMDTResult DriverSignalInitialize(DRV_SIGINFO_CB fnDrvSignalInfo, AMDTUInt32 samplingPeriod)
{
    BOOL status = false;
    AMDTResult ret = AMDT_STATUS_OK;
    DWORD param;
    AMDTUInt64 out = 0;

    samplingPeriod = samplingPeriod;

    isFlashedSignal = false;
    g_eventCfg.m_eventCb = fnDrvSignalInfo;
    g_eventCfg.m_hdl = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (!g_eventCfg.m_hdl)
    {
        PwrTrace("CreateEvent error = %d\n", GetLastError());
        ret = AMDT_ERROR_ACCESSDENIED;
    }
    else
    {
        AMDTUInt64 hdl = (AMDTUInt64)g_eventCfg.m_hdl;
        g_eventCfg.m_isActive = true;
        // Send the event information to driver
        status = POWER_DRIVER_IN_OUT(g_powerDrvHld, IOCTL_SET_EVENT, &hdl, sizeof(AMDTUInt64), &out, sizeof(AMDTUInt64), (DWORD*)&ret);
        g_process = CreateThread(nullptr, 0, ProcessDriverMessage, nullptr, 0, &param);
    }

    return 0;
}

// FlushDriverSignal: Send the signal to collect remaining data from driver
AMDTResult FlushDriverSignal()
{
    //    DWORD res = 0;

    if (nullptr != g_eventCfg.m_hdl)
    {
        SetEvent(g_eventCfg.m_hdl);
        isFlashedSignal = true;

#if 0
        //TODO: do we need to wait till thread is completed
        res = WaitForSingleObject(g_process, INFINITE);

        if (WAIT_OBJECT_0 == res)
        {
            DriverSignalClose();
        }

#endif
    }

    return 0;
}

// DriverSignalClose: Close the signal handler
AMDTResult DriverSignalClose()
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr != g_eventCfg.m_hdl)
    {
        CloseHandle(g_eventCfg.m_hdl);
        g_eventCfg.m_hdl = nullptr;
        g_eventCfg.m_isActive = false;
    }

    return ret;
}

// InitializeSharedBuffer: Create and initialize shared buffer between driver and user space
AMDTResult InitializeSharedBuffer()
{
    AMDTResult ret = AMDT_ERROR_FAIL;

    g_sharedFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS,  // read/write access
                                    FALSE,                // do not inherit the name
                                    PWR_PROF_SHARED_OBJ); // name of mapping object

    if (nullptr == g_sharedFile)
    {
        SECURITY_ATTRIBUTES secAttr;
        PSECURITY_DESCRIPTOR pSD;
        PACL pSacl = nullptr;  // not allocated
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
                                                                 SDDL_REVISION_1, &pSD, nullptr);

            GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl,
                                      &fSaclDefaulted);

            SetSecurityDescriptorSacl(secAttr.lpSecurityDescriptor, TRUE,
                                      pSacl, FALSE);
        }

        InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor,
                                     SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0,
                                  FALSE);

        g_sharedFile =
            CreateFileMappingW(INVALID_HANDLE_VALUE,
                               &secAttr,                     // default security
                               PAGE_READWRITE,               // read/write access
                               0,                            // max. object size
                               PWRPROF_SHARED_BUFFER_SIZE,   // buffer size
                               PWR_PROF_SHARED_OBJ);         // name of mapping object
    }

    if (nullptr != g_sharedFile)
    {
        g_pSharedBuffer =
            (AMDTUInt8*) MapViewOfFile(g_sharedFile,                   // handle to mapping object
                                       FILE_MAP_READ | FILE_MAP_WRITE, // read/write permission
                                       0, 0, PWRPROF_SHARED_BUFFER_SIZE);

        if (nullptr != g_pSharedBuffer)
        {
            ret = AMDT_STATUS_OK;
        }
    }

    return ret;
}

// ReleaseSharedBuffer: Release and unmap the shared buffer created
// between driver and user spce
AMDTResult ReleaseSharedBuffer()
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr != g_pSharedBuffer)
    {
        UnmapViewOfFile(g_pSharedBuffer);
        g_pSharedBuffer = nullptr;
    }

    if (nullptr != g_sharedFile)
    {
        CloseHandle(g_sharedFile);
        g_sharedFile = nullptr;
    }

    return ret;
}

// GetProcessNameFromPid: Get process name from process id
bool GetProcessNameFromPid(AMDTPwrProcessInfo* pInfo)
{

    bool found = false;

    if (nullptr != pInfo)
    {
        //create process handle
        HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_QUERY_INFORMATION,
                                   false,
                                   pInfo->m_pid);

        if (nullptr != hProc)
        {
            LPSTR path = new CHAR[MAX_PATH];
            DWORD len = MAX_PATH;
            bool res = QueryFullProcessImageName(hProc, 0, path, &len);

            if (0 != res)
            {
                AMDTUInt32 cnt = 0;
                AMDTUInt32 begin = 0;

                while (path[cnt] != '\0')
                {
                    if ('\\' == path[cnt])
                    {
                        begin = cnt;
                    }

                    cnt++;
                }

                memcpy(pInfo->m_name, &path[begin + 1], strlen(&path[begin + 1]) + 1);
                memcpy(pInfo->m_path, &path[0], begin);
                found = true;
            }
        }
        else
        {
            found = false;
        }
    }

    return found;
}

// GetApplicationPath: Retrive the application path
bool GetApplicationPath(wchar_t* appPath)
{
    // Assumption: processEnum[32|64].exe will reside in the dir in which codexl.exe resides;
    osFilePath filePath;
    bool retVal = false;

    // First, see if the dll path is set (like VS)
    retVal = osGetCurrentApplicationDllsPath(filePath);

    if (!retVal)
    {
        //Since the dll path is not set, assume the dlls are in the
        // same path as the exe.
        // Get the absolute path for the application - codexl.exe
        retVal = osGetCurrentApplicationPath(filePath);
    }

    if (false == retVal)
    {
        return retVal;
    }

    memcpy(appPath, filePath.fileDirectoryAsString().asCharArray(), OS_MAX_PATH);
    return retVal;
}

// InitializeTaskInfo: take a snapshot of all running PIDs, modules before we start the profile
bool InitializeTaskInfo()
{
    HRESULT hr = S_OK;
    wchar_t dirPath[OS_MAX_PATH] = { L'\0' };
    LARGE_INTEGER startCount;
    QueryPerformanceCounter(&startCount);

    GetApplicationPath(dirPath);

    hr = fnStartCapture(startCount.QuadPart, dirPath);

    if (S_OK == hr)
    {
        wchar_t tiDynamicFile[OS_MAX_PATH + 1];
        wchar_t tempPath [OS_MAX_PATH + 1];
        GetTempPathW(OS_MAX_PATH + 1, tempPath);
        // define the temp file name for driver with prefix "tiD".
        GetTempFileNameW(tempPath, L"tiD", 0, tiDynamicFile);
        hr = fnStopCapture(true, tiDynamicFile);

        if (S_OK == hr)
        {
            const size_t cSize = OS_MAX_PATH;
            wcscat(tempPath, L"PwrTiFile.ti");
            hr = fnWriteModuleInfoFile(tempPath);

            if (S_OK == hr)
            {
                hr = fnReadModuleInfoFile(tempPath);
            }
        }
    }

    return (S_OK == hr) ? true : false;
}

// PrepareInitialProcessList: Prepare the initial process list before the
// profile session
AMDTResult PrepareInitialProcessList(list<ProcessName>& list)
{
    AMDTResult ret = AMDT_STATUS_OK;

    list.clear();

    //    ret = (true == InitializeTaskInfo()) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return ret;
}

