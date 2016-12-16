//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CodeXLDriversLoadService.cpp
///
//==================================================================================
#ifndef UNICODE
    #define UNICODE
#endif

// Disable warning:
#pragma warning( disable : 4996 )


#include <stdio.h>
#include <windows.h>
#include <winbase.h>
#include <winsvc.h>
#include <process.h>
#include <intrin.h>

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
// Macros
#define CPU_FAMILY_EXTENDED    0xF

#define CPU_STEPPING_MASK       0x0000000F
#define CPU_BASE_MODEL_MASK     0x000000F0
#define CPU_BASE_FAMILY_MASK    0x00000F00
#define CPU_EXT_FAMILY_MASK     0x0FF00000
#define CPU_EXT_MODEL_MASK      0x000F0000

#include <sddl.h>

#include <AMDTDriverControl/inc/DriverControl.h>
#include <CodeXLDrivers/CpuProf/inc/UserAccess/CpuProfSharedObj.h>
#include <../PowerProfiling/Backend/AMDTPowerProfilingDrivers/Windows/inc/AMDTSharedObjPath.h>
#include <../PowerProfiling/Backend/AMDTPowerProfilingDrivers/inc/AMDTSharedBufferConfig.h>

const int nBufferSize = 500;
wchar_t pServiceName[nBufferSize + 1];
wchar_t pExeFile[nBufferSize + 1];
//char pInitFile[nBufferSize+1];
wchar_t pLogFile[nBufferSize + 1];
const int nMaxProcCount = 127;
PROCESS_INFORMATION pProcInfo[nMaxProcCount];

SERVICE_STATUS          serviceStatus;
SERVICE_STATUS_HANDLE   hServiceStatusHandle;

HANDLE gDriverHandlePcore = nullptr;
HANDLE gDriverHandleCAProf = nullptr;
HANDLE gDriverHandlePwrProf = nullptr;

VOID WINAPI CodeXLDriversLoadServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID WINAPI CodeXLDriversLoadServiceHandler(DWORD fdwControl);

HANDLE gCAProfAPISharedMapFile = nullptr;
HANDLE gPwrProfSharedMapFile = nullptr;


void WriteLog(wchar_t* pMsg);
void WriteErrorLog(wchar_t* pMsg);

CRITICAL_SECTION gCS;

bool InitializeProfAPISharedObj()
{
    gCAProfAPISharedMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS,          // read/write access
                                               FALSE,                      // do not inherit the name
                                               CPU_PROF_SHARED_OBJ);   // name of mapping object

    if (!gCAProfAPISharedMapFile)
    {
        SECURITY_ATTRIBUTES secAttr;
        char secDesc[ SECURITY_DESCRIPTOR_MIN_LENGTH ] = "";
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
            PSECURITY_DESCRIPTOR pSD;
            PACL pSacl = nullptr;                  // not allocated
            BOOL fSaclPresent = FALSE;
            BOOL fSaclDefaulted = FALSE;

            ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)",  // this means "low integrity"
                                                                 SDDL_REVISION_1,
                                                                 &pSD,
                                                                 nullptr);

            GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl,
                                      &fSaclDefaulted);

            SetSecurityDescriptorSacl(secAttr.lpSecurityDescriptor, TRUE,
                                      pSacl, FALSE);
        }

        InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor,
                                     SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0,
                                  FALSE);

        gCAProfAPISharedMapFile =
            CreateFileMappingW(INVALID_HANDLE_VALUE,
                               &secAttr,           // default security
                               PAGE_READWRITE,         // read/write access
                               0,              // max. object size
                               CPU_PROF_SHARED_MEM_SIZE,           // buffer size
                               CPU_PROF_SHARED_OBJ);   // name of mapping object
    }

    if (gCAProfAPISharedMapFile)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void ReleaseProfAPISharedObj()
{
    if (gCAProfAPISharedMapFile)
    {
        CloseHandle(gCAProfAPISharedMapFile);
        gCAProfAPISharedMapFile = nullptr;
    }
}

bool InitializePwrProfSharedObj()
{
    gPwrProfSharedMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS,          // read/write access
                                             FALSE,                      // do not inherit the name
                                             PWR_PROF_SHARED_OBJ);   // name of mapping object

    if (!gPwrProfSharedMapFile)
    {
        SECURITY_ATTRIBUTES secAttr;
        char secDesc[ SECURITY_DESCRIPTOR_MIN_LENGTH ] = "";
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
            PSECURITY_DESCRIPTOR pSD;
            PACL pSacl = nullptr;                  // not allocated
            BOOL fSaclPresent = FALSE;
            BOOL fSaclDefaulted = FALSE;

            ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)",  // this means "low integrity"
                                                                 SDDL_REVISION_1,
                                                                 &pSD,
                                                                 nullptr);

            GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl,
                                      &fSaclDefaulted);

            SetSecurityDescriptorSacl(secAttr.lpSecurityDescriptor, TRUE,
                                      pSacl, FALSE);
        }

        InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor,
                                     SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0,
                                  FALSE);

        gPwrProfSharedMapFile =
            CreateFileMappingW(INVALID_HANDLE_VALUE,
                               &secAttr,           // default security
                               PAGE_READWRITE,         // read/write access
                               0,              // max. object size
                               PWRPROF_SHARED_BUFFER_SIZE,           // buffer size
                               PWR_PROF_SHARED_OBJ);   // name of mapping object
    }

    if (gPwrProfSharedMapFile)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ReleasePwrProfSharedObj()
{
    if (gPwrProfSharedMapFile)
    {
        CloseHandle(gPwrProfSharedMapFile);
        gPwrProfSharedMapFile = nullptr;
    }
}

void WriteLog(wchar_t* pMsg)
{
#ifdef _DEBUG
    // write error or other information into log file
    ::EnterCriticalSection(&gCS);

    try
    {
        SYSTEMTIME oT;
        ::GetLocalTime(&oT);
        //FILE* pLog = _wfopen(pLogFile, L"a");
        FILE* pLog = nullptr;
        _wfopen_s(&pLog, pLogFile, L"a");

        if (pLog)
        {
            fwprintf(pLog, L"%02d/%02d/%04d, %02d:%02d:%02d\n    %s\n",
                     oT.wMonth, oT.wDay, oT.wYear, oT.wHour, oT.wMinute, oT.wSecond, pMsg);
            fclose(pLog);
        }
        else
        {
            wprintf(L"Unable to open log file: %s\n", pLogFile);
            throw;
        }
    }
    catch (...)
    {
        wprintf(L"Exception when trying to open or write log file: %s\n", pLogFile);
    }

    ::LeaveCriticalSection(&gCS);
#else
    (void)(pMsg);
#endif
}

void WriteErrorLog(wchar_t* pMsg)
{
#ifdef _DEBUG
    long nError = GetLastError();
    wchar_t pTemp[121];
    swprintf(pTemp, 120, L"%s, error code = %ld", pMsg, nError);
    WriteLog(pTemp);
#else
    (void)(pMsg);
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Configuration Data and Tables
//

SERVICE_TABLE_ENTRY DispatchTable[] =
{
    {pServiceName, CodeXLDriversLoadServiceMain},
    {nullptr, nullptr}
};

void LoadDrivers()
{
    if (!gCAProfAPISharedMapFile)
    {
        InitializeProfAPISharedObj();
    }

    if (!gPwrProfSharedMapFile)
    {
        InitializePwrProfSharedObj();
    }

    if (!gDriverHandlePcore)
    {
        wchar_t drivername[nBufferSize + 1];
        wchar_t systemDir[MAX_PATH];
        systemDir[0] = '\0';
        GetSystemDirectory(systemDir, MAX_PATH);
        PVOID oldValue = nullptr;
        BOOL isSys64;
        IsWow64Process(GetCurrentProcess(), &isSys64);

        if (isSys64)
        {
            isSys64 = Wow64DisableWow64FsRedirection(&oldValue);
        }

        swprintf(drivername, nBufferSize, L"%s%s", systemDir, L"\\drivers\\PCORE");
        OpenAmdDriver((LPCTSTR)drivername, &gDriverHandlePcore);

        swprintf(drivername, nBufferSize, L"%s%s", systemDir, L"\\drivers\\CpuProf");
        OpenAmdDriver((LPCTSTR)drivername, &gDriverHandleCAProf);

        // Install the Power Profiler driver only on AMD supported platforms
        swprintf(drivername, nBufferSize, L"%s%s", systemDir, L"\\drivers\\AMDTPwrProf");
        OpenAmdDriver((LPCTSTR)drivername, &gDriverHandlePwrProf);

        if (isSys64)
        {
            Wow64RevertWow64FsRedirection(oldValue);
        }
    }
}

void UnloadDrivers()
{
    wchar_t drivername[nBufferSize + 1];
    wchar_t systemDir[MAX_PATH];
    systemDir[0] = '\0';
    GetSystemDirectory(systemDir, MAX_PATH);

    if (gCAProfAPISharedMapFile)
    {
        ReleaseProfAPISharedObj();
    }

    if (gPwrProfSharedMapFile)
    {
        ReleasePwrProfSharedObj();
    }

    if (gDriverHandlePcore)
    {
        swprintf(drivername, nBufferSize, L"%s%s", systemDir, L"\\drivers\\PCORE");
        CloseAmdDriver((LPCTSTR)drivername, gDriverHandlePcore);
        gDriverHandlePcore = nullptr;
    }

    if (gDriverHandleCAProf)
    {
        swprintf(drivername, nBufferSize, L"%s%s", systemDir, L"\\drivers\\CpuProf");
        CloseAmdDriver((LPCTSTR)drivername, gDriverHandleCAProf);
        gDriverHandleCAProf = nullptr;
    }

    if (gDriverHandlePwrProf)
    {
        swprintf(drivername, nBufferSize, L"%s%s", systemDir, L"\\drivers\\AMDTPwrProf");
        CloseAmdDriver((LPCTSTR)drivername, gDriverHandlePwrProf);
        gDriverHandlePwrProf = nullptr;
    }
}

BOOL StopService(wchar_t* pName)
{
    // stop service with given name
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == 0)
    {
        WriteErrorLog(L"OpenSCManager failed");
    }
    else
    {
        // open the service
        SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);

        if (schService == 0)
        {
            WriteErrorLog(L"OpenService failed");
        }
        else
        {
            // call ControlService to stop the given service
            SERVICE_STATUS status;

            if (ControlService(schService, SERVICE_CONTROL_STOP, &status))
            {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return TRUE;
            }
            else
            {
                WriteErrorLog(L"ControlService failed");
            }

            CloseServiceHandle(schService);
        }

        CloseServiceHandle(schSCManager);
    }

    return FALSE;
}

BOOL StartService(wchar_t* pName, int nArg, wchar_t** pArg)
{
    // start service with given name
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == 0)
    {
        WriteErrorLog(L"OpenSCManager failed");
    }
    else
    {
        // open the service
        SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);

        if (schService == 0)
        {
            WriteErrorLog(L"OpenService failed");
        }
        else
        {
            // Start The Service
            if (StartService(schService, nArg, (const wchar_t**)pArg))
            {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return TRUE;
            }
            else
            {
                WriteErrorLog(L"StartService failed");
            }

            CloseServiceHandle(schService);
        }

        CloseServiceHandle(schSCManager);
    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////
//
// This routine gets used to start your service
//
VOID WINAPI CodeXLDriversLoadServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    (void)(lpszArgv);
    (void)(dwArgc);

    serviceStatus.dwServiceType        = SERVICE_WIN32;
    serviceStatus.dwCurrentState       = SERVICE_START_PENDING;
    serviceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    serviceStatus.dwWin32ExitCode      = 0;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCheckPoint         = 0;
    serviceStatus.dwWaitHint           = 0;

    hServiceStatusHandle = RegisterServiceCtrlHandler(pServiceName, CodeXLDriversLoadServiceHandler);

    if (hServiceStatusHandle == 0)
    {
        WriteErrorLog(L"RegisterServiceCtrlHandler failed");
        return;
    }

    // Initialization complete - report running status
    serviceStatus.dwCurrentState       = SERVICE_RUNNING;
    serviceStatus.dwCheckPoint         = 0;
    serviceStatus.dwWaitHint           = 0;

    if (!SetServiceStatus(hServiceStatusHandle, &serviceStatus))
    {
        WriteErrorLog(L"SetServiceStatus failed");
    }
}

//////////////////////////////////////////////////////////////////////
//
// This routine responds to events concerning your service, like start/stop
//
VOID WINAPI CodeXLDriversLoadServiceHandler(DWORD fdwControl)
{
    switch (fdwControl)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            serviceStatus.dwWin32ExitCode = 0;
            serviceStatus.dwCurrentState  = SERVICE_STOPPED;
            serviceStatus.dwCheckPoint    = 0;
            serviceStatus.dwWaitHint      = 0;
            // terminate all processes started by this service before shutdown
            UnloadDrivers();
            {
                SetServiceStatus(hServiceStatusHandle, &serviceStatus);
            }
            return;

        case SERVICE_CONTROL_PAUSE:
            serviceStatus.dwCurrentState = SERVICE_PAUSED;
            break;

        case SERVICE_CONTROL_CONTINUE:
            serviceStatus.dwCurrentState = SERVICE_RUNNING;
            break;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        default:
            break;
    }

    SetServiceStatus(hServiceStatusHandle,  &serviceStatus);
}


//////////////////////////////////////////////////////////////////////
//
// Uninstall
//
VOID UnInstall(wchar_t* pName)
{
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (schSCManager != 0)
    {
        SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);

        if (schService != 0)
        {
            DeleteService(schService);
            CloseServiceHandle(schService);
        }

        CloseServiceHandle(schSCManager);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Install
//
VOID Install(wchar_t* pPath, wchar_t* pName)
{
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

    if (schSCManager != 0)
    {
        SC_HANDLE schService = CreateServiceW
                               (
                                   schSCManager,   /* SCManager database      */
                                   pName,          /* name of service         */
                                   pName,          /* service name to display */
                                   SERVICE_ALL_ACCESS,        /* desired access          */
                                   SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS , /* service type            */
                                   SERVICE_AUTO_START,      /* start type              */
                                   SERVICE_ERROR_NORMAL,      /* error control type      */
                                   pPath,          /* service's binary        */
                                   nullptr,                      /* no load ordering group  */
                                   nullptr,                      /* no tag identifier       */
                                   nullptr,                      /* no dependencies         */
                                   nullptr,                      /* LocalSystem account     */
                                   nullptr
                               );                     /* no password             */

        if (schService != 0)
        {
            CloseServiceHandle(schService);
        }

        CloseServiceHandle(schSCManager);
    }
}

void WorkerProc(void* pParam)
{
    (void)(pParam);
    LoadDrivers();
}


void main(int argc, char* argv[])
{
    // error message
    wchar_t pTemp[121];
    unsigned int errStrSize = 120;

    // initialize global critical section
    ::InitializeCriticalSection(&gCS);

    // initialize variables for .exe, .ini, and .log file names
    wchar_t pModuleFile[nBufferSize + 1];
    DWORD dwSize = GetModuleFileName(nullptr, (LPTSTR)pModuleFile, nBufferSize);
    pModuleFile[dwSize] = 0;

    if (dwSize > 4 && pModuleFile[dwSize - 4] == '.')
    {
        swprintf(pExeFile, nBufferSize, L"%s", pModuleFile);
        pModuleFile[dwSize - 4] = 0;
        swprintf(pLogFile, nBufferSize, L"%s.log", pModuleFile);
    }
    else
    {
        printf("Invalid module file name: %ws\r\n", pModuleFile);
        return;
    }

    WriteLog(pExeFile);
    WriteLog(pLogFile);
    wcscpy_s(pServiceName, 500, L"CodeXLDriversLoadService");
    WriteLog(pServiceName);

    // uninstall service if switch is "-u"
    if (argc == 2 && _stricmp("-uninstall", argv[1]) == 0)
    {
        UnInstall(pServiceName);
    }
    // install service if switch is "-i"
    else if (argc == 2 && _stricmp("-install", argv[1]) == 0)
    {
        Install(pExeFile, pServiceName);
    }
    // stop a service with given name
    else if (argc == 2 && _stricmp("-stop", argv[1]) == 0)
    {
        if (StopService(pServiceName))
        {
            swprintf(pTemp, errStrSize, L"Stopped service %s", pServiceName);
            WriteLog(pTemp);
        }
        else
        {
            swprintf(pTemp, errStrSize, L"Failed to stop service %s", pServiceName);
            WriteLog(pTemp);
        }
    }
    // run a service with given name
    else if (argc == 2 && _stricmp("-start", argv[1]) == 0)
    {
        WriteLog(L"StartService");

        if (StartService(pServiceName, 0, nullptr))
        {
            swprintf(pTemp, errStrSize, L"Ran service %s", pServiceName);
            WriteLog(pTemp);
        }
        else
        {
            swprintf(pTemp, errStrSize, L"Failed to run service %s", pServiceName);
            WriteLog(pTemp);
        }
    }
    // assume user is starting this service
    else
    {
        // start a worker thread to load driver
        if (_beginthread(WorkerProc, 0, nullptr) == -1)
        {
            WriteErrorLog(L"_beginthread failed");
        }

        // pass dispatch table to service controller
        if (!StartServiceCtrlDispatcher(DispatchTable))
        {
            WriteErrorLog(L"StartServiceCtrlDispatcher failed");
        }

        // you don't get here unless the service is shutdown
    }

    ::DeleteCriticalSection(&gCS);
}
