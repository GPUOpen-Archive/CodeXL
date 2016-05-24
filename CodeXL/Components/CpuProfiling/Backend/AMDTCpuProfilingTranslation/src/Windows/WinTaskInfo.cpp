//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file WinTaskInfo.cpp
/// \brief Windows task information interface implementation.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/Windows/WinTaskInfo.cpp#30 $
// Last checkin:   $DateTime: 2016/04/14 02:42:23 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569060 $
//=====================================================================

#include "WinTaskInfo.h"

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

#include <AMDTExecutableFormat/inc/PeFile.h>
#include <AMDTProfilingAgentsData/inc/JclReader.h>
#include <AMDTProfilingAgentsData/inc/JclWriter.h>
#include <AMDTProfilingAgentsData/inc/Windows/CelReader.h>
#include <AMDTProfilingAgentsData/inc/Windows/CelWriter.h>

#include <tlhelp32.h>

// Suppress Qt header warnings
#pragma warning(push)
#pragma warning(disable : 4800)
#include <QDir>
#pragma warning(pop)

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (addValue) )

#define AMDTPROCESSENUM32_NAME  L"\\CXLProcessEnum" GDT_DEBUG_SUFFIX_W GDT_BUILD_SUFFIX_W
#define AMDTPROCESSENUM64_NAME  L"\\CXLProcessEnum-x64" GDT_DEBUG_SUFFIX_W GDT_BUILD_SUFFIX_W

// canonical linear address
#define ERBT_713_NON_CANONICAL_MASK 0x0000FFFFFFFFFFFF

bool CopyFilesToDestionDir(QString srcDirName, QString destDirName, QStringList nameFilter = QStringList());

// This is the constructor of a class
WinTaskInfo::WinTaskInfo()
{
    memset(m_tiFileName, 0, sizeof(m_tiFileName));
    memset(m_tiSnapshotFileName, 0, sizeof(m_tiSnapshotFileName));

    // at least 1 CPU
    m_affinity = 1;

    m_lpMaxAppAddr = 0x0;
    m_lpMinAppAddr = 0x0;

    m_bPartitionInfoflag = false;
    m_bMapBuilt = false;
    m_bLoadKernelPeFiles = false;
    m_jnc_counter = 0;
    m_JitModCount = 0;

    m_pSearchPath = NULL;
    m_pServerList = NULL;
    m_pCachePath  = NULL;
}

// destructor
WinTaskInfo::~WinTaskInfo()
{
    Cleanup();
}

////////////////////////////////////////////////////////////////////////
// WinTaskInfo::Cleanup()
//  Clean up the process map, user mode module map, and kernel mode module
//  map. Clean up the time marks.
//
//  Param: void.
//  Redsc: void
//
void WinTaskInfo::Cleanup()
{
    JitTaskInfo::Cleanup();

    m_allModulesMap.clear();
    m_interestingPidMap.clear();

    for (KernelModMap::iterator itMod = m_tiKeModMap.begin(), itModEnd = m_tiKeModMap.end(); itMod != itModEnd; ++itMod)
    {
        PeFile* pPeFile = itMod->second.pPeFile;

        if (NULL != pPeFile)
        {
            delete pPeFile;
        }
    }

    for (auto itMod = m_modLoadAddrPEMap.begin(), itModEnd = m_modLoadAddrPEMap.end(); itMod != itModEnd; ++itMod)
    {
        PeFile* pPeFile = itMod->second;

        if (NULL != pPeFile)
        {
            delete pPeFile;
        }
    }

    m_modLoadAddrPEMap.clear();

    m_tiKeModMap.clear();
    m_tiProcMap.clear();
    m_ThreadMap.clear();
    m_tiDriveMap.clear();
    m_PJSMap.clear();
    m_bPartitionInfoflag = false;
    m_bMapBuilt = false;
    m_bLoadKernelPeFiles = false;

    // We use 'free' instead of 'delete' because these strings were created by 'wcsdup'
    //
    if (NULL != m_pSearchPath)
    {
        free(m_pSearchPath);
        m_pSearchPath = NULL;
    }

    if (NULL != m_pServerList)
    {
        free(m_pServerList);
        m_pServerList = NULL;
    }

    if (NULL != m_pCachePath)
    {
        free(m_pCachePath);
        m_pCachePath = NULL;
    }

    m_moduleIdMap.clear();
}


HRESULT WinTaskInfo:: SetPrivilege(
    HANDLE hToken,  // token handle
    LPCTSTR Privilege,  // Privilege to enable/disable
    BOOL bEnablePrivilege  // TRUE to enable. FALSE to disable
)
{
    TOKEN_PRIVILEGES tp = { 0 };
    // Initialize everything to zero
    LUID luid;
    DWORD cb = sizeof(TOKEN_PRIVILEGES);

    if (!LookupPrivilegeValue(NULL, Privilege, &luid))
    {
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;

    if (bEnablePrivilege)
    {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else
    {
        tp.Privileges[0].Attributes = 0;
    }

    AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL);

    if (GetLastError() != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}


HRESULT WinTaskInfo::EnumProcessThreads()
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Take a snapshot of all running threads
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (hThreadSnap == INVALID_HANDLE_VALUE)
    {
        return S_FALSE;
    }

    // Fill in the size of the structure before using it.
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(hThreadSnap, &te32))
    {
        CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
        return S_FALSE;
    }

    // Walk the thread list of the system, and
    // collect information about each thread associated with the specified process
    do
    {
        ThreadInfoKey t_threadKey((gtUInt64) te32.th32OwnerProcessID, (DWORD) te32.th32ThreadID);
        // Note: We don't know the thread creation time.
        ThreadInfoValue t_threadValue(te32.th32ThreadID, -1, -1, 0, 0);
        m_ThreadMap.insert(ThreadInfoMap::value_type(t_threadKey, t_threadValue));
    }
    while (Thread32Next(hThreadSnap, &te32));

    // Clean up the snapshot object.
    CloseHandle(hThreadSnap);
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// WinTaskInfo::ProcModSnapShot()
//  Enumerate process on 32-bit OS, and gather the module info in those processes.
//
// Param: void.
//
// Rdesc: HRESULT: On success, return S_OK. Otherwise return E_FAIL or S_FALSE.
//
HRESULT WinTaskInfo::ProcModSnapShot()
{
    HRESULT hr = S_OK;

    // Enumerate processes and modules in those processes.
    HANDLE  hToken = NULL;
    HRESULT tpSetFlag = 0;

    // allocate space for processes and module info
    // Since we don't know exact number of processes and modules yet.
    // Just allocate certain amount.
    // If it's not enough, we will reallocate again.
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        tpSetFlag = SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
    }
    else
    {
        CloseHandle(hToken);
        hToken = NULL;
    }

    // Take a snapshot of all processes in the system.
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        hr = E_FAIL;
    }

    PROCESSENTRY32W pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    // Retrieve information about the first process
    if (S_OK == hr && !Process32FirstW(hProcessSnap, &pe32))
    {
        hr = E_FAIL;
    }

    if (S_OK == hr)
    {
        BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        isSys64 = true;
#else
        IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

        do
        {
            ProcessValue t_procValue;
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

            if (NULL == hProcess)
            {
                wcsncpy(t_procValue.processName,
                        ((0 == pe32.th32ProcessID) ? L"System Idle Process" : pe32.szExeFile), OS_MAX_PATH);

                t_procValue.bNameConverted = true;
                m_tiProcMap.insert(ProcessMap::value_type(pe32.th32ProcessID, t_procValue));
                continue;
            }

            BOOL wow64 = true;

            //Only check whether the processes are 64-bit on a 64-bit system
            if (isSys64)
            {
                IsWow64Process(hProcess, &wow64);
            }

            m_bitnessMap[pe32.th32ProcessID] = (wow64 ? true : false);

            CloseHandle(hProcess);

            // Take a snapshot of all modules in the specified process.
            HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);

            if (hModuleSnap == INVALID_HANDLE_VALUE)
            {
                continue;
            }

            MODULEENTRY32W me32;
            me32.dwSize = sizeof(MODULEENTRY32W);

            // Retrieve information about the first module
            if (!Module32FirstW(hModuleSnap, &me32))
            {
                CloseHandle(hModuleSnap);
                continue;
            }

            if (wow64)
            {
                HANDLE hFile = CreateFileW(me32.szExePath,
                                           GENERIC_READ,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                           NULL,
                                           OPEN_EXISTING,
                                           0,
                                           NULL);

                if (INVALID_HANDLE_VALUE != hFile)
                {
                    GetFinalPathNameByHandleW(hFile,
                                              me32.szExePath,
                                              sizeof(me32.szExePath) / sizeof(*me32.szExePath),
                                              VOLUME_NAME_DOS | FILE_NAME_NORMALIZED);

                    CloseHandle(hFile);
                }
            }

            // convert name
            ConvertName(me32.szExePath, pe32.th32ProcessID);
            bool bNameConverted = true;

            // the executable module is always the first module from the process.
            // save the process info into process map
            t_procValue.dProcStartTime = 0;
            wcsncpy(t_procValue.processName, me32.szExePath, OS_MAX_PATH);
            t_procValue.bNameConverted = true;
            m_tiProcMap.insert(ProcessMap::value_type(pe32.th32ProcessID, t_procValue));

            do
            {
                // convert name
                if (!bNameConverted)
                {
                    if (wow64)
                    {
                        HANDLE hFile = CreateFileW(me32.szExePath,
                                                   GENERIC_READ,
                                                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                                   NULL,
                                                   OPEN_EXISTING,
                                                   0,
                                                   NULL);

                        if (INVALID_HANDLE_VALUE != hFile)
                        {
                            GetFinalPathNameByHandleW(hFile,
                                                      me32.szExePath,
                                                      sizeof(me32.szExePath) / sizeof(*me32.szExePath),
                                                      VOLUME_NAME_DOS | FILE_NAME_NORMALIZED);

                            CloseHandle(hFile);
                        }
                    }

                    ConvertName(me32.szExePath, pe32.th32ProcessID);
                }
                else
                {
                    bNameConverted = false;
                }

                // save the module info into module map
                ModuleKey t_modKey(pe32.th32ProcessID, reinterpret_cast<gtUInt64>(me32.modBaseAddr), 0);
                ModuleValue t_modValue(0, static_cast<gtUInt64>(me32.modBaseSize), TI_TIMETYPE_MAX, me32.szExePath, evPEModule);
                t_modValue.bNameConverted = true;
                m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
            }
            while (Module32NextW(hModuleSnap, &me32));

            CloseHandle(hModuleSnap);
        }
        while (Process32NextW(hProcessSnap, &pe32));
    }

    if (hProcessSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hProcessSnap);
    }

    if (S_OK == hr)
    {
        // Take a snapshot of all running threads.
        hr = EnumProcessThreads();
    }

    // clean up mess.
    if (tpSetFlag)
    {
        SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);
    }

    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::DriverSnapShot()
//  Enumerate the kernel mode drivers and gather driver loading info.
//
HRESULT WinTaskInfo::DriverSnapShot()
{
    HRESULT hr = S_OK;

    // Enumerate driver info
    DWORD t_DriverArraySize = 0;
    LPVOID* pDriverAddrArray = NULL;
    DWORD cbNeeded;

    EnumDeviceDrivers(NULL, 0, &cbNeeded);

    // calculate driver numbers;
    t_DriverArraySize = cbNeeded / sizeof(LPVOID);

    // allocate space for the driver load addresses.
    pDriverAddrArray = (LPVOID*) malloc(sizeof(LPVOID) * t_DriverArraySize);

    if (NULL != pDriverAddrArray)
    {
        // enumerate driver again to fill in the driver array,
        if (EnumDeviceDrivers(pDriverAddrArray, sizeof(LPVOID) *t_DriverArraySize, &cbNeeded))
        {
            wchar_t szDriverName[1024];

            // for each driver
            for (DWORD i = 0; i < t_DriverArraySize; i++)
            {
                // get driver file name
                if (GetDeviceDriverFileNameW(pDriverAddrArray[i], szDriverName, sizeof(szDriverName)))
                {
                    // create key
                    KernelModKey t_keModKey((DWORD) pDriverAddrArray[i], 0);

                    // convert driver module name
                    //string t_str = ConvertName(szDriverName);
                    ConvertName(szDriverName, 0);

                    // create driver mulde value, with base address (0), size(0), and unload time (0).
                    KernelModValue t_keModValue(/*end addr*/    0,
                                                                /* base addr*/  0,
                                                                /*unload time*/ 0,
                                                                szDriverName);
                    t_keModValue.bNameConverted = true;
                    t_keModValue.keModImageSize = 0;

                    // add into kernel module map.
                    m_tiKeModMap.insert(KernelModMap::value_type(t_keModKey, t_keModValue));
                }
            }
        }   // if (EnumDeviceDrivers(pDriverAddrArray, t_DriverArraySize, &cbNeeded))
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    // clean up mess
    if (pDriverAddrArray != NULL)
    {
        free(pDriverAddrArray);
        pDriverAddrArray = NULL;
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::LaunchModuleEnumerator(bool b32bit)
//  Launch a helper process to enumerate the process and modules in the system
//  space and save the module info into a file.
//
// Note:
//  Since the 64-bit EnumProcesses() and EnumProcessModules() does not gather the
//  right module info for the 32-bit application running on 64-bit OS.We need to
//  launch 32-bit and 64-bit Module Enumerator processes separately.
//
//  Param: bool | b32bit | indicate to launch 32-bit module enumerator or 64-bit
//
//  Redsc: return S_OK if successful, otherwise return E_FAIL.
//
HRESULT WinTaskInfo::LaunchModuleEnumerator(bool b32bit, const wchar_t* binDirectory)
{
    HRESULT hr = E_FAIL;
    wchar_t    exepath[OS_MAX_PATH + 1];
    DWORD   exitcode;
    STARTUPINFO         si;
    PROCESS_INFORMATION pinfo;

    // initialize to success
    exitcode = 0;
    // launch the helper process to enumerate modules
    // and dump the module info into a file.

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pinfo, sizeof(pinfo));

    wchar_t BinFileName [OS_MAX_PATH + 1];
    BinFileName[0] = L'\0';
    exepath[0] = L'\0';

    if (NULL == binDirectory)
    {
        // find the current executable file
        if (GetModuleFileNameW(NULL, BinFileName, sizeof(BinFileName)))
        {
            wchar_t drive[_MAX_DRIVE];
            wchar_t dir[_MAX_DIR];

            // split the current executable file name into drive, path etc.
            _wsplitpath(BinFileName, drive, dir, NULL, NULL);
            wcscpy(exepath, L"\"");
            wcscat(exepath, drive);
            wcscat(exepath, dir);
            hr = S_OK;
        }
    }
    else
    {
        wcscpy(exepath, L"\"");
        wcscat(exepath, binDirectory);
        hr = S_OK;
    }

    if (b32bit)
    {
        // build up path, name and argument for the 32-bit module enumerator
        wcscat(exepath, AMDTPROCESSENUM32_NAME L".exe\" ");
        wcscat(exepath, m_tiSnapshotFileName);
    }
    else
    {
        // build up path, name and argument for the 64-bit module enumerator
        wcscat(exepath, AMDTPROCESSENUM64_NAME L".exe\" ");
        wcscat(exepath, m_tiSnapshotFileName);
    }

    char msg[255];

    if (S_OK == hr)
    {
        // create the helper process
        if (!CreateProcess(NULL, exepath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pinfo))
        {
            unsigned int terr = GetLastError();
            sprintf(msg, "Unable to create module enumerator process.  Error %u.", terr);

            m_ErrorMsg = msg;
            hr = E_ACCESSDENIED;
        }
    }

    if (S_OK == hr)
    {
        // wait the helper process terminates.
        WaitForSingleObject(pinfo.hProcess, INFINITE);

        // get the exit code of helper process
        GetExitCodeProcess(pinfo.hProcess, &exitcode);

        // close the process and thread
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);

        if (exitcode != 0)
        {
            unsigned int terr = GetLastError();

            if (b32bit)
            {
                sprintf(msg, "Unable to enumerate 32-bit modules.  Error %u.", terr);
            }
            else
            {
                sprintf(msg, "Unable to enumerate 64-bit modules.  Error %u.", terr);
            }

            m_ErrorMsg = msg;
            hr = E_FAIL;
        }
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::ProcessSnapShotFile(char *pfilename, BOOL b32bitSnapShotFlag)
//  Read the snapshot file for the user mode module info.
//
//  Note:
//  The 64-bit snap shot file contain both 32-bit and 64-bit module info. And the info
//  about 32-bit module is not right. Therefore, we need to filter out the 32-bit module
//  in the 64-bit snapshot file.
//
//  Param: char* | pfilename | snapshot file name
//  Param: BOOL  | b32bitSnapShotFlag | Indicate it's a 32-bit or 64-bit snap shot file.
//
//  Redsc: Returns S_OK if successful, otherwise returns E_FAIL.
//
HRESULT WinTaskInfo::ProcessSnapShotFile(const wchar_t* pfilename, BOOL b32bitSnapShotFlag, bool bConvertFlag)
{
    GT_UNREFERENCED_PARAMETER(b32bitSnapShotFlag);

    HRESULT hr = S_OK;

    gtUInt64  tProcessID, tModLoadAddr, tModSize, t_u64, previous_pid = static_cast<gtUInt64>(-1);
    DWORD   tLoadTime, tUnloadTime, usermodulecount, kernelmodcount, t_d32;
    BOOL    bWow64 = FALSE;
    wchar_t szModFullName[OS_MAX_PATH + 1];

    usermodulecount = 0;
    kernelmodcount = 0;

    FILE* p_tifile = NULL;
    _wfopen_s(&p_tifile, pfilename, L"r+b");

    if (p_tifile == NULL)
    {
        hr = E_FAIL;
    }

    if (S_OK == hr)
    {
        // move to the begin of the file
        rewind(p_tifile);

        // module number is stored at the begin here
        fread(&usermodulecount, sizeof(DWORD), 1, p_tifile);

        // kernel module number is stored just after user module count
        fread(&kernelmodcount, sizeof(DWORD), 1, p_tifile);

        // MAX app address is stored here
        fread(&m_lpMaxAppAddr, sizeof(gtUInt64), 1, p_tifile);
        fread(&m_lpMinAppAddr, sizeof(gtUInt64), 1, p_tifile);
        int i;

        for (i = 0; i < (int) usermodulecount; i++)
        {

            // read Process ID
            fread(&tProcessID, sizeof(gtUInt64), 1, p_tifile);

            // read whether process is running under WOW64
            fread(&bWow64, sizeof(BOOL), 1, p_tifile);

            // read module load address
            fread(&tModLoadAddr, sizeof(gtUInt64), 1, p_tifile);

            // read module load time
            fread(&tLoadTime, sizeof(DWORD), 1, p_tifile);

            // read image base address
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);

            // read module size
            fread(&tModSize, sizeof(gtUInt64), 1, p_tifile);

            // module unload time
            fread(&tUnloadTime, sizeof(DWORD), 1, p_tifile);

            // length of the module name string
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            // module name string
            ::ZeroMemory(szModFullName, sizeof(szModFullName));
            fread((void*) szModFullName, sizeof(wchar_t), t_d32, p_tifile);


            if (0 == i)
            {
                previous_pid = tProcessID + 1; // make it different
            }

            if (previous_pid != tProcessID)
            {
                previous_pid = tProcessID;

                // this is from a snap shot file, we need to filter out known processes
                ProcessMap::iterator procMapIter = m_tiProcMap.find(tProcessID);

                //If we already have the process, keep going.
                if (m_tiProcMap.end() == procMapIter)
                {
                    // this is a new process
                    ProcessValue t_procValue;
                    t_procValue.dProcStartTime = tLoadTime;
                    t_procValue.bNameConverted = bConvertFlag;
                    m_bitnessMap[tProcessID] = (bWow64 ? true : false);

                    // convert module name
                    if (bConvertFlag)
                    {
                        //t_procValue.processName = ConvertName(szModFullName);
                        ConvertName(szModFullName, tProcessID);
                        wcsncpy(t_procValue.processName, szModFullName, OS_MAX_PATH);
                    }
                    else
                    {
                        wcsncpy(t_procValue.processName, szModFullName, OS_MAX_PATH);
                    }

                    // we did not it find from current process map, add to the list
                    m_tiProcMap.insert(ProcessMap::value_type(tProcessID, t_procValue));
                }
            }

            ModuleKey t_modKey(tProcessID, tModLoadAddr, tLoadTime);
            ModuleValue t_modValue(0, tModSize, TI_TIMETYPE_MAX, szModFullName, evPEModule);
            t_modValue.bNameConverted = bConvertFlag;

            m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
        }

        // we will read the device driver info.
        for (i = 0; i < (int) kernelmodcount; i++)
        {
            // write kernel module load address
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);

            // write kernel module load time
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            // create key
            KernelModKey t_keModKey(t_u64, t_d32);

            // read length of kernel module name
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            // read kernel module name
            ::ZeroMemory(szModFullName, sizeof(szModFullName));
            fread((void*) szModFullName, sizeof(wchar_t), t_d32, p_tifile);

            // convert driver module name
            if (bConvertFlag)
            {
                ConvertName(szModFullName, 0);
            }

            // create driver module value, with base address (0), size(0), and unload time (0).
            KernelModValue t_keModValue(0, 0, 0, szModFullName);
            t_keModValue.bNameConverted = bConvertFlag;

            // add into kernel module map.
            m_tiKeModMap.insert(KernelModMap::value_type(t_keModKey, t_keModValue));
        }
    }

    // close file
    if (NULL != p_tifile)
    {
        fclose(p_tifile);
        p_tifile = NULL;

        // remove the temp file.
        ::DeleteFile(pfilename);

    }

    return hr;
}



//We do the calculation this way because that's how MS says to obtain relative
// times in the SYSTEMTIME help article
//"Do not add and subtract values from the SYSTEMTIME structure to obtain
// relative times. Instead, do the following:
//Convert the SYSTEMTIME structure to a FILETIME structure.
//Copy the resulting FILETIME structure to a ULARGE_INTEGER structure.
//Use ordinary 64-bit arithmetic on the ULARGE_INTEGER value."

//if the sample is before the first time mark, return what the filetime should be for
//  the first time mark
FILETIME WinTaskInfo::Synchronize(FILETIME start, gtUInt64 deltaTick, int core, unsigned int extraMs)
{
    GT_UNREFERENCED_PARAMETER(core);

    FILETIME relative;
    ULARGE_INTEGER addition;// = relative;
    addition.LowPart = 10000; //1 mS = this much filetime
    addition.HighPart = 0;

    //figure out the relative mS of the timeStamp since the start

    gtUInt64 time_ms = ((deltaTick * 1000) / m_hrFreq) + extraMs;
    //one milisecond of time * so many miliseconds
    addition.QuadPart  *= time_ms;
    ULARGE_INTEGER base;
    base.HighPart = start.dwHighDateTime;
    base.LowPart = start.dwLowDateTime;
    //  + the start FILETIME
    addition.QuadPart += base.QuadPart;
    relative.dwLowDateTime = addition.LowPart;
    relative.dwHighDateTime = addition.HighPart;

    return relative;
}

///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::ConvertName( const char *modulename )
//  Convert the module name.
//
// Param: const char* | modulename | module name
//
// Redsc: string | the converted module name string.
//
void WinTaskInfo::ConvertName(wchar_t* pModulename, gtUInt64 procId) const
{
    wchar_t pathstring[OS_MAX_PATH + 1];
    memset(pathstring, 0x00, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));
    wcsncpy(pathstring, pModulename, OS_MAX_PATH);

    wchar_t tempstr[OS_MAX_PATH + 1];

    wchar_t windowsDir[OS_MAX_PATH + 1];
    wchar_t systemDir[OS_MAX_PATH + 1];
    memset(windowsDir, 0x00, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));
    memset(systemDir, 0x00, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));

    BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    isSys64 = true;
#else
    IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

    int once = 1;

    while (once)
    {
        // case: windows dir
        // case like \WINNT or \WINDOWS
        if (GetWindowsDirectoryW(windowsDir, sizeof(windowsDir)))
        {
            unsigned int winDirLen = wcslen(windowsDir);

            if (!_wcsnicmp(pModulename, windowsDir + 2, winDirLen - 2))
            {
                swprintf(pathstring, OS_MAX_PATH, L"%s%s", windowsDir,
                         pModulename + winDirLen - 2);
                break;
            }
        }

        // case like \Windows\SysWOW64\ntdll.dll, only on 64-bit systems
        if ((isSys64) && (GetSystemWow64Directory(systemDir, OS_MAX_PATH)))
        {
            unsigned int sysDirLen = wcslen(systemDir);

            if (!_wcsnicmp(pModulename, systemDir + 2, sysDirLen - 2))
            {
                swprintf(pathstring, OS_MAX_PATH, L"%s%s", windowsDir,
                         pModulename + sysDirLen - 2);
                break;
            }
        }

        // case: system dir
        // case like \WINNT\System32\ntoskrnl.exe
        if (GetSystemDirectory(systemDir, OS_MAX_PATH))
        {
            unsigned int sysDirLen = wcslen(systemDir);

            if (!_wcsnicmp(pModulename, systemDir + 2, sysDirLen - 2))
            {
                //check for incorrect 32-bit path given
                if ((isSys64) && (m_bitnessMap.at(procId)))
                {
                    //It is 32-bit!  It should use sysWow64 path instead
                    GetSystemWow64Directory(systemDir, OS_MAX_PATH);
                }

                swprintf(pathstring, OS_MAX_PATH, L"%s%s", systemDir,
                         pModulename + sysDirLen - 2);
                break;
            }
        }

        // case : the module name starts with "\SystemRoot\"
        wcscpy(tempstr, L"\\SystemRoot\\");

        if (!_wcsnicmp(pModulename, tempstr, wcslen(tempstr)))
        {
            wchar_t systemroot[OS_MAX_PATH + 1];
            GetWindowsDirectoryW(systemroot, OS_MAX_PATH);
            int strlength = (int) wcslen(systemroot);

            // in case the system root with path separator
            if ((strlength > 0) && (L'\\' == systemroot[strlength - 1]))
            {
                systemroot[strlength - 1] = L'\0';
            }

            // replace the system root with the real path
            //pathstring.replace(0, tempstr.length(), systemroot);
            wsprintf(pathstring, L"%s\\%s", systemroot, pModulename + wcslen(tempstr));
            break;
        }

        // case like \??\C:\WINNT\system32\winlogon.exe
        wcscpy(tempstr, L"\\??\\");

        if (!_wcsnicmp(pModulename, tempstr, wcslen(tempstr)))
        {
            if (pModulename[5] == L':')
            {
                // this is local storage device
                //pathstring.replace(0, tempstr.length(), "");
                wsprintf(pathstring, L"%s", pModulename + wcslen(tempstr));
            }
            else
            {
                // this is UNC
                //pathstring.replace(0, tempstr.length(), "\\\\");
                wsprintf(pathstring, L"\\\\%s", pModulename + wcslen(tempstr));
            }

            break;
        }

        // case like \\?\C:\WINNT\system32\winlogon.exe
        wcscpy(tempstr, L"\\\\?\\");

        if (!_wcsnicmp(pModulename, tempstr, wcslen(tempstr)))
        {
            if (pModulename[5] == L':')
            {
                // this is local storage device
                //pathstring.replace(0, tempstr.length(), "");
                wsprintf(pathstring, L"%s", pModulename + wcslen(tempstr));
            }
            else
            {
                // this is UNC
                //pathstring.replace(0, tempstr.length(), "\\\\");
                wsprintf(pathstring, L"\\\\%s", pModulename + wcslen(tempstr));
            }

            break;
        }

        // case like \Device\LanmanRedirector\networkname\dir\winlogon.exe
        wcscpy(tempstr, L"\\Device\\LanmanRedirector\\");

        if (!_wcsnicmp(pModulename, tempstr, wcslen(tempstr)))
        {
            // this is network storage device
            wsprintf(pathstring, L"\\\\%s", pModulename + wcslen(tempstr));
            break;
        }

        // case like \Device\Mup\networkname\dir\winlogon.exe
        wcscpy(tempstr, L"\\Device\\Mup\\");

        if (!_wcsnicmp(pModulename, tempstr, wcslen(tempstr)))
        {
            // this is network storage device
            wsprintf(pathstring, L"\\\\%s", pModulename + wcslen(tempstr));
            break;
        }

        // no path, just file name
        // case like pci.sys
        // this is the last thing we check. If there is a path (with "\" or ":"),
        wchar_t* pSlash = NULL;
        wchar_t* pColon = NULL;
        const wchar_t pathSeparator[] = { osFilePath::osPathSeparator, '\0' };
        pSlash = wcsstr(pathstring, pathSeparator);
        pColon = wcsstr(pathstring, L":");

        if (!pSlash && !pColon)
        {

            wchar_t drive[_MAX_DRIVE];
            wchar_t dir[_MAX_DIR];
            wchar_t fname[OS_MAX_FNAME];
            wchar_t ext[_MAX_EXT];
            drive[0] = L'\0';
            dir[0] = L'\0';
            fname[0] = L'\0';
            ext[0] = L'\0';

            _wsplitpath(pathstring, drive, dir, fname, ext);

            if (0 == wcslen(drive) && 0 == wcslen(dir))
            {
                if (!_wcsicmp(ext, L".sys"))
                {
                    wsprintf(tempstr, L"%s\\drivers\\%s%s",
                             systemDir, fname, ext);
                    //string tempstr = systemDir;
                    //tempstr += "\\drivers\\";
                    //tempstr += fname;
                    //tempstr += ext;

                    if (!m_is32on64Sys)
                    {
                        // No redirection needed
                        if (0xFFFFFFFF != GetFileAttributesW(tempstr))
                        {
                            wcsncpy(pathstring, tempstr, OS_MAX_PATH);
                            break;
                        }
                    }
                    else
                    {
                        PVOID oldValue;

                        // 64-bit OS
                        if (Wow64DisableWow64FsRedirection(&oldValue))
                        {
                            DWORD DRet = GetFileAttributesW(tempstr);
                            // Enable WOW64 file system redirection.
                            Wow64RevertWow64FsRedirection(oldValue);

                            if (0xFFFFFFFF != DRet)
                            {
                                wcsncpy(pathstring, tempstr, OS_MAX_PATH);
                                break;
                            }
                        }
                    }
                }
            }
        }

        // case like \kingkong\temp\32bit\hwdll.dll (across network)
        // case like \temp\32bit\hwdll.dll (on C: drive)
        if (L'\\' == pModulename[0])
        {
            //case like \v\develop\project\... which can be achieved through subst
            if (L'\\' == pModulename[2])
            {
                //skip the first '\'
                wsprintf(tempstr, L"%c:%s", pModulename[1], (pModulename + 2));

                if (FileExists(tempstr))
                {
                    wcsncpy(pathstring, tempstr, OS_MAX_PATH);
                    break;
                }

            }

            // case: logical drive in path
            DriveMap::const_iterator driveIt = m_tiDriveMap.begin(), driveEnd = m_tiDriveMap.end();

            for (; driveIt != driveEnd; ++driveIt)
            {
                const DriveMap::value_type& driveItem = *driveIt;

                //check each in drive map for device match
                if (0 == _wcsnicmp(driveItem.first.wString, pModulename, wcslen(driveItem.first.wString)))
                {
                    wsprintf(pathstring, L"%s%s", driveItem.second.wString, pModulename + wcslen(driveItem.first.wString));
                    break;
                }
            }

            if (driveIt == driveEnd)
            {
                if (S_OK == AddDriveLabel(pModulename, tempstr))
                {
                    wcsncpy(pathstring, tempstr, OS_MAX_PATH);
                    break;
                }
            }
            else
            {
                //found it in drive list, break
                break;
            }
        }

        // either the path is right or we don't know how to convert it.
        break;
    }

    wcsncpy(pModulename, pathstring, OS_MAX_PATH);

    //return pathstring;
}


bool WinTaskInfo::FileExists(const wchar_t* pModulename) const
{
    bool retVal;
    UINT old = SetErrorMode(SEM_FAILCRITICALERRORS);

    if (!m_is32on64Sys)
    {
        // 32-bit OS or no redirect on a 64-bit OS
        retVal = (0xFFFFFFFF != GetFileAttributesW(pModulename));
    }
    else
    {
        DWORD DRet;
        PVOID oldValue;

        // 64-bit OS
        if (Wow64DisableWow64FsRedirection(&oldValue))
        {
            DRet = GetFileAttributesW(pModulename);
            // Enable WOW64 file system redirection.
            Wow64RevertWow64FsRedirection(oldValue);

        }
        else
        {
            DRet = GetFileAttributesW(pModulename);
        }

        retVal = (0xFFFFFFFF != DRet);
    }

    SetErrorMode(old);
    return retVal;
}

///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::AddDriveLabel( const char *modulename, string *pString)
//  Convert the module name.
//
// Param: const wchar_t* | modulename | module name
// Param: wchar_t *    | pString    | module name string with drive label if successful.
//
// Redsc: HRESULT | Returns S_OK if find the only one valid drive, otherwise return S_FALSE.
//

HRESULT WinTaskInfo::AddDriveLabel(const wchar_t* modulename, wchar_t* pString) const
{
    HRESULT hr = S_FALSE;

    wchar_t returnstr[OS_MAX_PATH + 1];
    int count = 0;
    //initialize string with initial module name - even if not found, at least
    // something comprehensible will be shown
    wcsncpy(returnstr, modulename, OS_MAX_PATH);

    // try the map label in device drive map
    for (DriveMap::const_iterator driveMapIter = m_tiDriveMap.begin(), driveEnd = m_tiDriveMap.end();
         driveMapIter != driveEnd; ++driveMapIter)
    {
        const DriveMap::value_type& driveItem = *driveMapIter;

        //ignore the floppy drive
        if (0 == _wcsicmp(driveItem.second.wString, L"A:"))
        {
            continue;
        }

        // check if the current drive is a hard disk
        // if the drive is not a hard disk, skip it
        UINT driveType = GetDriveType(driveItem.second.wString);

        if ((DRIVE_FIXED != driveType) && (DRIVE_REMOVABLE != driveType))
        {
            continue;
        }

        //Check to see if the file exists on each mounted drive...
        if (wcslen(driveItem.second.wString) > 0)
        {
            wchar_t tempstr[OS_MAX_PATH + 1];

            if ('\\' == modulename[1])
            {
                //skip the first '\'
                wsprintf(tempstr, L"%s%s", driveItem.second.wString,
                         (modulename + 1));
            }
            else
            {
                wsprintf(tempstr, L"%s%s", driveItem.second.wString,
                         modulename);
            }

            if (FileExists(tempstr))
            {
                wcsncpy(returnstr, tempstr, OS_MAX_PATH);
                count++;
            }
        }
    }

    if (1 >= count)
    {
        //use the last one found, if more than one found
        wcsncpy(pString, returnstr, OS_MAX_PATH);
        hr = S_OK;
    }
    else
    {
        //assume it's on a network drive
        if (0 != wcsncmp(modulename, L"\\\\", 2))
        {
            wsprintf(pString, L"\\%s", modulename);
        }
        else
        {
            wcscpy(pString, modulename);
        }

        hr = S_OK;
    }

    return hr;
}


/////////////////////////////////////////////////////////////////////
// WinTaskInfo::GenerateTempFiles()
//  Generate the temp file name for driver to write task info. Generate
//  temp file name for snap shot file.
//
HRESULT WinTaskInfo::GenerateTempFiles()
{
    wchar_t tempPath[OS_MAX_PATH + 1];
    DWORD len = GetTempPathW(OS_MAX_PATH + 1, tempPath);

    if (0 == len)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT hr = S_OK;
    // define the temp file name for driver with prefix "tiD".
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    {
        // define the temp file name for snapshot 32 with prefix 32S
        UINT ordinal = GetTempFileNameW(tempPath, L"32S", 0, m_tiSnapshotFileName);

        if (0U == ordinal)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            memset(m_tiSnapshotFileName, 0x00, sizeof(m_tiSnapshotFileName));
        }
    }
#else

    if (m_is32on64Sys)
    {
        // define the temp file name for snapshot 64 with prefix 64S
        UINT ordinal = GetTempFileNameW(tempPath, L"64S", 0, m_tiSnapshotFileName);

        if (0U == ordinal)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            memset(m_tiSnapshotFileName, 0x00, sizeof(m_tiSnapshotFileName));
        }
    }

#endif

    return hr;
}


////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::StartCapture()
//  Start task info capturing.
//
HRESULT WinTaskInfo::StartCapture(gtUInt64 startCount, const wchar_t* binDirectory)
{
    HRESULT hr = S_OK;

    Cleanup();

    if (S_OK == hr)
    {
        if (!m_bPartitionInfoflag)
        {
            GetDrivePartitionInfo();
        }

        // generate temp files
        hr = GenerateTempFiles();
    }

    int prevThreadPriority = GetThreadPriority(GetCurrentThread());
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    if (S_OK == hr)
    {
        // enumerate process and gather the module info for
        hr = ProcModSnapShot();
    }

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE

    if (S_OK == hr)
    {
        // gather 32-bit kernel module info
        hr = DriverSnapShot();
    }

    if (S_OK == hr)
    {
        // CodeAnalyst is running in 32-bit Mode on 64bit OS

        // enumerate process and gather the module info for 32-bit apps
        // and driver info or for 64-bit OS drivers.
        if (m_is32on64Sys)
        {
            hr = LaunchModuleEnumerator(false, binDirectory);
        }
    }

#else // AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

    // CodeAnalyst is running in 64-bit Mode on 64bit OS
    if (S_OK == hr)
    {
        // correctly enumerate process and gather the module info for 32-bit processes
        hr = LaunchModuleEnumerator(true, binDirectory);
    }

    // The WinTaskInfo is running in native 64bit mode
    if (S_OK == hr)
    {
        // gather kernel module info
        hr = DriverSnapShot();
    }

#endif
    SetThreadPriority(GetCurrentThread(), prevThreadPriority);

    SYSTEM_INFO SystemInfo;
    BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    isSys64 = true;
#else
    IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

    if (isSys64)
    {
        GetNativeSystemInfo(&SystemInfo);
    }
    else
    {
        GetSystemInfo(&SystemInfo);
    }

    m_affinity = SystemInfo.dwNumberOfProcessors;

    // get the max address that user mode application can reach.
    m_lpMaxAppAddr = (gtUInt64) SystemInfo.lpMaximumApplicationAddress;
    m_lpMinAppAddr = (gtUInt64) SystemInfo.lpMinimumApplicationAddress;

    m_startHr = startCount;
    return hr;
}


/////////////////////////////////////////////////////////////////////
// WinTaskInfo::ProcessTiTempFile()
//  Read the task info file that generated by driver.
//
HRESULT WinTaskInfo::ProcessTiTempFile(bool bConvertFlag, const wchar_t* tempTiFileName)
{
    HRESULT hr = E_FAIL;
    FILE* tFile = NULL;
    TASK_INFO_RECORD tiRecord;

    if ((tFile = _wfopen(tempTiFileName, L"r+b")) != NULL)
    {
        while (!feof(tFile))
        {
            memset(tiRecord.m_PathName, 0x00, sizeof(tiRecord.m_PathName));

            fread(&tiRecord, sizeof(TASK_INFO_RECORD), 1, tFile);

            switch (tiRecord.m_RecordType)
            {
                case MONITOR_PROC_CREATE:
                    // ignore it here.
                {
                    //bitness is true for 32-bit
                    bool bitness = tiRecord.m_Size != 64;
                    m_bitnessMap[tiRecord.m_ProcessHandle] = bitness;
                }
                break;

                case MONITOR_PROC_DELETE:
                    // process terminates, update the unload time.
                    OnTerminateProcess(tiRecord.m_ProcessHandle, (DWORD) tiRecord.m_TickStamp);
                    break;

                case MONITOR_IMAGE_LOAD:
                    // module load, update process map, module map, or kernel module map.
                    OnLoadImage(tiRecord, bConvertFlag);
                    break;

                case MONITOR_THREAD_CREATE:
                    OnThreadCreation(tiRecord);
                    break;

                case MONITOR_THREAD_DELETE:
                    OnThreadDeletion(tiRecord);
                    break;
            }
        }

        hr = S_OK;
    }

    if (tFile != NULL)
    {
        fclose(tFile);
        tFile = NULL;

        // remove the temporary ti file from the driver.
        ::DeleteFile(tempTiFileName);
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////
// WinTaskInfo::OnLoadImage(TASK_INFO_RECORD tiRecord)
//  Update the process map, module map or kernel module if a module is loaded.
//
//  Param: TASK_INFO_RECORD | tiRecord | task info record
//
void WinTaskInfo::OnLoadImage(const TASK_INFO_RECORD& tiRecord, bool bConvertFlag)
{
    // convert module name;
    wchar_t t_str[OS_MAX_PATH + _MAX_DRIVE_NAME + 1];
    memset(t_str , 0 , ((OS_MAX_PATH + _MAX_DRIVE_NAME + 1) * sizeof(wchar_t)));
    wcsncpy(t_str, tiRecord.m_PathName, (OS_MAX_PATH + _MAX_DRIVE_NAME));

    if (bConvertFlag)
    {
        ConvertName(t_str, tiRecord.m_ProcessHandle);
    }

    // If this is the first image loaded for a process
    ProcessMap::iterator procMapIter = m_tiProcMap.find(tiRecord.m_ProcessHandle);

    if (procMapIter == m_tiProcMap.end())
    {
        // we did not find it from process map, that means this is a new process
        ProcessValue t_procValue;
        t_procValue.dProcStartTime = (DWORD) tiRecord.m_TickStamp;

        //Since the driver change, the first loaded image is not necessarily the executable
        wcscpy(t_procValue.processName, t_str);
        t_procValue.bNameConverted = bConvertFlag;

        m_tiProcMap.insert(ProcessMap::value_type(tiRecord.m_ProcessHandle, t_procValue));
        procMapIter = m_tiProcMap.find(tiRecord.m_ProcessHandle);
    }

    //Name the process after the first exe image loaded
    size_t nameSize = wcslen(t_str);
    size_t curNameSize = wcslen(procMapIter->second.processName);

    if ((nameSize > 4) && (0 == wcscmp(&(t_str[nameSize - 4]), L".exe"))
        && (0 != wcscmp(&(procMapIter->second.processName[curNameSize - 4]), L".exe")))
    {
        wcscpy(procMapIter->second.processName, t_str);
        procMapIter->second.bNameConverted = bConvertFlag;
    }

    GT_ASSERT(m_lpMaxAppAddr);

    if (tiRecord.m_StartAddress < m_lpMaxAppAddr && tiRecord.m_StartAddress > m_lpMinAppAddr)
    {
        // this is in the user mode application or dll

        ModuleKey t_modKey(tiRecord.m_ProcessHandle, tiRecord.m_StartAddress, static_cast<TiTimeType>(tiRecord.m_TickStamp));

        ModuleValue t_modValue(0, tiRecord.m_Size, TI_TIMETYPE_MAX, t_str, evPEModule);
        t_modValue.bNameConverted = bConvertFlag;

        m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
    }
    else
    {
        // this is in the kernel mode module
        KernelModKey t_keModKey((gtUInt64) tiRecord.m_StartAddress, (DWORD) tiRecord.m_TickStamp);

        KernelModValue t_keModValue(tiRecord.m_StartAddress + tiRecord.m_Size, 0, 0, t_str);
        t_keModValue.keModImageSize = tiRecord.m_Size;
        t_keModValue.bNameConverted = bConvertFlag;

        m_tiKeModMap.insert(KernelModMap::value_type(t_keModKey, t_keModValue));

    }
}


///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::OnTerminateProcess(gtUInt64 processID, DWORD endtime)
//  Update the process end time in the process map when the process is terminated.
//
//  Param: gtUInt64 | processID | process ID
//  Param: DWORD  | endtime   | process terminate time.
//
void WinTaskInfo::OnTerminateProcess(gtUInt64 processID, DWORD endtime)
{
    // update the end time of the process in the process map.
    ProcessMap::iterator i;

    for (i = m_tiProcMap.lower_bound(processID); i != m_tiProcMap.end(); ++i)
    {
        ProcessMap::value_type& item = *i;

        if (item.first != processID)
        {
            break;
        }

        if (0 != item.second.dProcEndTime)
        {
            continue;
        }

        item.second.dProcEndTime = endtime;
    }

    // update the end time for all module in the process
    for (ModuleMap::iterator j = m_tiModMap.lower_bound(ModuleKey(processID, GT_UINT64_MAX, 0)), jEnd = m_tiModMap.end(); j != jEnd; ++j)
    {
        ModuleMap::value_type& item = *j;

        if (item.first.processId != processID)
        {
            break;
        }

        if (0 == item.second.moduleUnloadTime || TI_TIMETYPE_MAX == item.second.moduleUnloadTime)
        {
            item.second.moduleUnloadTime = endtime;
        }
    }
}

void WinTaskInfo::OnThreadCreation(const TASK_INFO_RECORD& tiRecord)
{

    // verify this thread creation type
    if (MONITOR_THREAD_CREATE != tiRecord.m_RecordType)
    {
        return;
    }

    // NOTE: for thread creation/deletion record, tiRecord.m_StartAddress is used for thread id
    ThreadInfoKey t_threadKey((gtUInt64) tiRecord.m_ProcessHandle, (DWORD) tiRecord.m_StartAddress);

    // Note: We don't know the thread deletion time yet.
    ThreadInfoValue t_threadValue((DWORD)tiRecord.m_StartAddress,
                                  tiRecord.m_Core, -1, (DWORD)tiRecord.m_TickStamp, 0);
    m_ThreadMap.insert(ThreadInfoMap::value_type(t_threadKey, t_threadValue));
}


void WinTaskInfo::OnThreadDeletion(const TASK_INFO_RECORD& tiRecord)
{
    // verify the record type
    if (MONITOR_THREAD_DELETE != tiRecord.m_RecordType)
    {
        return;
    }

    // update the end time of the thread delete time in the process map.
    ThreadInfoMap::iterator iter;
    ThreadInfoKey t_threadKey((gtUInt64) tiRecord.m_ProcessHandle, (DWORD) tiRecord.m_StartAddress);

    iter = m_ThreadMap.find(t_threadKey);

    if (iter == m_ThreadMap.end())
    {
        // CANNOT find
        ThreadInfoValue t_threadValue((DWORD)tiRecord.m_StartAddress, tiRecord.m_Core, tiRecord.m_Core, 0, (DWORD)tiRecord.m_TickStamp);
        m_ThreadMap.insert(ThreadInfoMap::value_type(t_threadKey, t_threadValue));
    }
    else
    {
        // Found, update deletion time nad cpuNum
        ThreadInfoMap::value_type& item = *iter;

        if ((item.first.processID == tiRecord.m_ProcessHandle) && (item.first.threadID == (DWORD) tiRecord.m_StartAddress))
        {
            item.second.cpuNumDeleted = tiRecord.m_Core;
            item.second.threadDeletionTime = (DWORD)tiRecord.m_TickStamp;
        }
    }
}


////////////////////////////////////////////////////////////////////////
// WinTaskInfo::StopCapture()
//  Stop the task info capturing.
//
HRESULT WinTaskInfo::StopCapture(bool bConvertFlag, const wchar_t* tempTiFileName)
{
    HRESULT hr = S_OK;

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

    if (S_OK == hr)
    {
        // read the 32-bit snap shot file which contains app module info.
        hr = ProcessSnapShotFile(m_tiSnapshotFileName, TRUE, bConvertFlag);
    }

#else

    if (m_is32on64Sys)
    {
        // read the 64-bit snap shot file which contains app module info and
        // device driver info.
        hr = ProcessSnapShotFile(m_tiSnapshotFileName, FALSE, bConvertFlag);
    }

#endif

    if (S_OK == hr)
    {
        // read the driver task info file
        hr = ProcessTiTempFile(bConvertFlag, tempTiFileName);
    }

    if (S_OK == hr)
    {
        // so far maps for snap shot already build up
        m_bMapBuilt = true;

        // calculate the end time for all user mode module
        CalculateEndTime();

        // calculate the size for all kernel module
        CalculateDriverSize();
    }

    // Reset after capture and before translation of each session
    m_nextModInstanceId = 1;
    m_nextModuleId = 1;

    return hr;
}


////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::CalculateEndTime()
//  Calculate the end time for user mode modules.
//
void WinTaskInfo::CalculateEndTime()
{
    LARGE_INTEGER ctr;
    QueryPerformanceCounter(&ctr);
    m_EndHr = ctr.QuadPart;
    gtUInt64 endDeltaTick = m_EndHr - m_startHr;

    ProcessMap::iterator i;

    for (i = m_tiProcMap.begin(); i != m_tiProcMap.end(); ++i)
    {
        ProcessMap::value_type& item = *i;

        if (0 != item.second.dProcEndTime)
        {
            continue;
        }

        item.second.dProcEndTime = (DWORD)endDeltaTick;
    }

    ModuleMap::iterator secondIter = m_tiModMap.begin();
    ModuleMap::iterator procModBeginIter = m_tiModMap.begin();

    for (ModuleMap::iterator firstIter = m_tiModMap.begin(), endIter = m_tiModMap.end(); firstIter != endIter; ++firstIter)
    {
        if (firstIter->first.processId != procModBeginIter->first.processId)
        {
            procModBeginIter = firstIter;
        }

        ModuleMap::value_type& item1 = *firstIter;

        TiTimeType tempEndtime = item1.second.moduleUnloadTime;

        if (0 == tempEndtime || TI_TIMETYPE_MAX == tempEndtime)
        {
            tempEndtime = static_cast<TiTimeType>(endDeltaTick);
        }

        for (secondIter = procModBeginIter; secondIter != endIter; ++secondIter)
        {
            ModuleMap::value_type& item2 = *secondIter;

            if (item1.first.processId != item2.first.processId)
            {
                break;
            }

            if (item1.first.moduleLoadTime >= item2.first.moduleLoadTime)
            {
                continue;
            }

            if (item1.first.moduleLoadAddr >= (item2.first.moduleLoadAddr + item2.second.moduleSize) ||
                item2.first.moduleLoadAddr >= (item1.first.moduleLoadAddr + item1.second.moduleSize))
            {
                continue;
            }

            if (tempEndtime > item2.first.moduleLoadTime)
            {
                tempEndtime = item2.first.moduleLoadTime;
            }
        }

        item1.second.moduleUnloadTime = tempEndtime;
    }

    for (ThreadInfoMap::iterator iter = m_ThreadMap.begin(), iterEnd = m_ThreadMap.end(); iter != iterEnd; ++iter)
    {
        ThreadInfoMap::value_type& item = *iter;

        if (0 != item.second.threadDeletionTime)
        {
            continue;
        }

        item.second.threadDeletionTime = (DWORD)endDeltaTick;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetDrivePartitionInfo()
//  Get hard disk partition info and save them into drive map.
//
#ifdef CODEANALSYT_FOR_WINNT_WIN2K
void WinTaskInfo::GetDrivePartitionInfo()
{
    DWORD drivemask = 0;
    // clean the drive map first. In case drive map has data from reading task info file.
    m_tiDriveMap.clear();
    m_bPartitionInfoflag = false;

    // get drive mask
    drivemask = GetLogicalDrives();

    if (0 == drivemask)
    {
        // it does not have any logical drives.
        return;
    }

    // initial drive label
    wchar_t rawDiskName[3] = L"A:";
    wchar_t buffer[1024];   // this is big enough;
    DWORD bufSize = 1024;

    for (int i = 0; i < 26; ++i)
    {
        // if logical drive exists
        if (drivemask & (1 << i))
        {
            // Open the raw disk

            memset(buffer, 0x00, bufSize);

            if (QueryDosDevice(rawDiskName, buffer, bufSize))
            {
                // add it into drive map.
                m_tiDriveMap.insert(DriveMap::value_type(buffer, rawDiskName));
            }

        }

        rawDiskName[0]++;
    }

    m_bPartitionInfoflag = true;
}
#else
void WinTaskInfo::GetDrivePartitionInfo()
{
    DWORD size = OS_MAX_PATH;
    wchar_t* buffer = new wchar_t[OS_MAX_PATH];
    wchar_t volumeName[OS_MAX_PATH];
    wchar_t deviceName[OS_MAX_PATH];
    DWORD checkSize;

    //If we can't allocate space for inital call, we have more pressing things
    //  to worry about than getting the file names correct.
    if (NULL == buffer) { return; }

    buffer[0] = L'\0';

    // clean the drive map first. In case drive map has data from reading task
    //  info file.
    m_tiDriveMap.clear();
    m_bPartitionInfoflag = false;

    //We have to do this in two parts, part one will get all local partitions,
    //  even if they are mounted within another drive, like C:\mnt\hda2

    // Open a scan for volumes
    HANDLE  hVol = FindFirstVolume(buffer, size);

    //this shouldn't happen, because if there are no volumes, there's a
    //  serious problem with the system.
    if (INVALID_HANDLE_VALUE != hVol)
    {

        // for each volume; process it.
        do
        {
            //If we can't get the user-readable name
            if (!GetVolumePathNamesForVolumeName(buffer, volumeName, OS_MAX_PATH,
                                                 &checkSize))
            {
                continue;
            }

            if (0 == wcslen(volumeName))
            {
                continue;
            }

            //this removes the trailing backslash for QueryDosDevice
            buffer[wcslen(buffer) - 1] = L'\0';
            volumeName[wcslen(volumeName) - 1] = L'\0';

            //This gets the device mapping
            if (QueryDosDevice((buffer + 4), deviceName, OS_MAX_PATH))
            {
                // add it into drive map.
                m_tiDriveMap.insert(DriveMap::value_type(deviceName, volumeName));
            }
        }
        while (FindNextVolume(hVol, buffer, size));

        // Close out the volume scan.
        FindVolumeClose(hVol);
    }

    //This is the second part.  It will also get networked drives that part one
    //  won't, but there is some overlap that needs to be ignored.

    buffer[0] = L'\0';
    //returns the number of bytes used, or needed.
    checkSize = GetLogicalDriveStrings(size, buffer);

    if (checkSize > size)
    {
        //this is possible, but unlikely.  If it happens, I just re-allocate
        //  the required amount, so it's cool.
        delete [] buffer;
        buffer = new wchar_t[checkSize + 1];

        //if the alloc fails, at least we have some info
        if (NULL == buffer) { return; }

        buffer[0] = L'\0';
        GetLogicalDriveStrings(checkSize, buffer);
    }

    wchar_t* temp = buffer;

    //for each null-terminated drive string in the list
    while (temp[0] != 0)
    {
        //we strip off the trailing backslash for the QueryDosDevice call
        int curDriveSize = wcslen(temp);

        if ((temp[curDriveSize - 1] == L'/') ||
            (temp[curDriveSize - 1] == L'\\'))
        {
            temp[curDriveSize - 1] = L'\0';
        }

        if (QueryDosDevice(temp, deviceName, OS_MAX_PATH))
        {
            // add it into drive map.
            m_tiDriveMap.insert(DriveMap::value_type(deviceName, temp));
        }

        temp += curDriveSize + 1;
    }

    if (buffer)
    {
        delete [] buffer;
    }

    m_bPartitionInfoflag = true;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::CalculateDriverSize()
//  Calculate the driver size.
//  Note:
//      The driver load info obtained from EnumDeviceDrivers() does not have
//      driver size. To judge the sample is in which kernel module, we got to
//      calculate the driver size by kludge -- assume the next kernel module
//      load address is the end address of previous kernel module.
//
void WinTaskInfo::CalculateDriverSize()
{
    KernelModMap::iterator i;

    if (m_tiKeModMap.size() > 0)
    {
        i = m_tiKeModMap.begin();
        KernelModMap::value_type& predItem = *i;

        if (predItem.second.keModEndAddr == 0)
        {
            predItem.second.keModEndAddr = 0xFFFFFFFFFFFFFFFF;
        }

        gtUInt64 predStartAddr = predItem.first.keModLoadAddr;
        gtUInt64 predEndAddr = predItem.second.keModEndAddr;

        i++;

        while (i != m_tiKeModMap.end())
        {
            KernelModMap::value_type& currentItem = *i;

            if (currentItem.first.keModLoadAddr < predStartAddr)
            {
                currentItem.second.keModEndAddr = predStartAddr;
            }
            else if (currentItem.first.keModLoadAddr == predStartAddr)
            {
                currentItem.second.keModEndAddr = predEndAddr;
            }

            predStartAddr = currentItem.first.keModLoadAddr;
            predEndAddr = currentItem.second.keModEndAddr;

            i++;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetProcessesNum(unsigned *pProcNum)
//  Get number of processes in the process map.
//
//  Param: [out] unsigned * | pProcNum | number of processes
//
HRESULT WinTaskInfo::GetProcessesNum(unsigned* pProcNum)
{
    HRESULT hr = S_OK;

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        hr = S_FALSE;
    }
    else
    {
        *pProcNum = (unsigned) m_tiProcMap.size();
    }

    return hr;
}


//////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetAllProcesses(ProcQueryInfo *pProcQueryInfo, unsigned procNum)
//  Get all processes info.
//
//  Param: [out] ProcQueryInfo | *pProcQueryInfo | reference pointer of process info.
//  Param: [in]  unsigned      | procNum         | number of processes
//
HRESULT WinTaskInfo::GetAllProcesses(
    ProcQueryInfo* pProcQueryInfo,
    unsigned procNum)
{
    HRESULT hr = S_OK;

    if (NULL == pProcQueryInfo)
    {
        return S_FALSE;
    }

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    ProcessMap::iterator procMapIter;

    unsigned count = 0;

    for (procMapIter = m_tiProcMap.begin(); procMapIter != m_tiProcMap.end(); procMapIter++)
    {
        if (count >= procNum)
        {
            hr = E_FAIL;
            break;
        }

        ProcessMap::value_type& procItem = *procMapIter;
        pProcQueryInfo[count].pqiProcessID = procItem.first;

        // if the name is not converted yet, convert it and update the item name in map.
        if (!procItem.second.bNameConverted)
        {
            wchar_t tStr[OS_MAX_PATH + 1];
            wcscpy(tStr, procItem.second.processName);
            ConvertName(tStr, procItem.first);
            wcscpy(procItem.second.processName, tStr);
            procItem.second.bNameConverted = true;
        }

        wcscpy(pProcQueryInfo[count].pqiProcessName, procItem.second.processName);

        count++;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::FindProcessName(gtUInt64 processID, char* processName, unsigned sizeofname)
//  Find the process name for a given process id.
//
//  Param: [in]  gtUInt64   | processID   | process id
//  Param: [out] char*    | processName | process name
//  Param: [in]  unsigned | sizeofname  | length of the process name string
//
HRESULT WinTaskInfo::FindProcessName(gtUInt64 processID, wchar_t* processName, unsigned sizeofname)
{
    HRESULT hr = E_FAIL;

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    ProcessMap::iterator procMapIter = m_tiProcMap.find(processID);

    if (procMapIter != m_tiProcMap.end())
    {
        ProcessMap::value_type& procItem = *procMapIter;

        if (!procItem.second.bNameConverted)
        {
            wchar_t tStr[OS_MAX_PATH];
            wcscpy(tStr, procItem.second.processName);
            ConvertName(tStr, processID);
            wcscpy(procItem.second.processName, tStr);
            procItem.second.bNameConverted = true;
        }

        if (wcslen(procItem.second.processName) < sizeofname)
        {
            if (0 != processID)
            {
                wcscpy(processName, procItem.second.processName);
            }
            else
            {
                wcscpy(processName, L"System Idle");
            }

            hr = S_OK;
        }
    }

    return hr;

}

///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetModNumInProc(gtUInt64 processID, unsigned *pNumOfMod)
//  Get number of modules in a process.
//
//  Param: [in]  gtUInt64   | processID | process ID
//  Param: [out] unsigned | *pNumOfMod| number of modules in a given process.
//
HRESULT WinTaskInfo::GetModNumInProc(gtUInt64 processID, unsigned* pNumOfMod)
{
    HRESULT hr = S_OK;

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    ModuleMap::iterator iter;
    *pNumOfMod = 0;
    int count = 0;

    // Note: since ModuleKey is sorted by PID (ascending), module address (descending) and load time (descending),
    // Searching (pid-1, 0, 0) will bring up the first module in process of pid
    for (iter = m_tiModMap.upper_bound(ModuleKey(processID - 1, 0, 0)); iter != m_tiModMap.end(); ++iter)
    {
        if (iter->first.processId != processID)
        {
            break;
        }

        count++;
    }

    *pNumOfMod = count;

    return hr;
}


//////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetModuleInfoByIndex( )
//  Get the module info for the nth module in the process.
//
//  Param: [in] gtUInt64  | process ID | process id
//  Param: [in] unsigned| modIndex   | module index in the process
//  Param: [out] gtUInt64 | *pModuleStartAddr | module start address
//  Param: [out] gtUInt64 | *pModuleSize | module image size
//  Param: [out] char*  | pModulename | module full name
//  Param: [in] unsigned| namesize | length of the module name string.
//
HRESULT WinTaskInfo::GetModuleInfoByIndex(gtUInt64 processID, unsigned modIndex,
                                          gtUInt64* pModuleStartAddr, gtUInt64* pModuleSize, wchar_t* pModulename, unsigned namesize)
{
    HRESULT hr = E_FAIL;

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    unsigned count = 0;

    for (ModuleMap::iterator i = m_tiModMap.upper_bound(ModuleKey(processID - 1, 0, 0)); i != m_tiModMap.end(); ++i)
    {
        ModuleMap::value_type& item = *i;

        if (item.first.processId != processID)
        {
            break;
        }

        if (count == modIndex)
        {
            *pModuleStartAddr = item.first.moduleLoadAddr;
            *pModuleSize = item.second.moduleSize;

            // if the module name is not converted yet, convert it and update it in the map.
            if (!item.second.bNameConverted)
            {
                wchar_t tStr[OS_MAX_PATH + 1];
                wcscpy(tStr, item.second.moduleName);
                wcscpy(item.second.moduleName, tStr);
                item.second.bNameConverted = true;
            }

            if (wcslen(item.second.moduleName) < namesize)
            {
                wcscpy(pModulename, item.second.moduleName);
                hr = S_OK;
            }

            break;
        }

        count++;
    }

    return hr;

}

bool WinTaskInfo::GetUserPeModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick, ModuleMap::value_type& item)
{
    //convert the timestamp to the system mS count
    if (systemTimeTick)
    {
        pModInfo->deltaTick = systemTimeTick;
    }

    pModInfo->ModuleStartAddr = item.first.moduleLoadAddr;
    pModInfo->Modulesize = item.second.moduleSize;
    pModInfo->pPeFile = item.second.pPeFile;
    pModInfo->instanceId = item.second.instanceId;

    if (!item.second.bNameConverted)
    {
        wchar_t tStr[OS_MAX_PATH];
        wcsncpy(tStr, item.second.moduleName, OS_MAX_PATH);
        ConvertName(tStr, pModInfo->processID);

        osCriticalSectionLocker lock(m_TIMutex);

        if (!item.second.bNameConverted)
        {
            wcsncpy(item.second.moduleName, tStr, OS_MAX_PATH);
            item.second.bNameConverted = true;
        }
    }

    if (!item.second.bLoadedPeFile)
    {
        osCriticalSectionLocker lock(m_TIMutex);

        if (!item.second.bLoadedPeFile)
        {
            if (NULL != item.second.pPeFile)
            {
                delete item.second.pPeFile;
            }

            PeFile* pPeFile = nullptr;
            auto it = m_modLoadAddrPEMap.find(ModNameAddrKey(item.second.moduleName, item.first.moduleLoadAddr));

            if (it == m_modLoadAddrPEMap.end())
            {
                pPeFile = new PeFile(item.second.moduleName);

                if (NULL != pPeFile)
                {
                    if (pPeFile->Open(item.first.moduleLoadAddr))
                    {
                        pPeFile->InitializeSymbolEngine(m_pSearchPath, m_pServerList, m_pCachePath);
                    }
                    else
                    {
                        delete pPeFile;
                        pPeFile = NULL;
                    }
                }

                // Note: pPeFile can be null
                ModNameAddrKey mkey(item.second.moduleName, item.first.moduleLoadAddr);
                m_modLoadAddrPEMap.insert(ModulePeFileMap::value_type(mkey, pPeFile));
            }
            else
            {
                pPeFile = (*it).second;
            }

            pModInfo->pPeFile = pPeFile;
            item.second.pPeFile = pPeFile;
            item.second.bLoadedPeFile = true;
        }
    }

    wcsncpy(pModInfo->pModulename, item.second.moduleName, pModInfo->namesize);

    wchar_t tPreJITModName[OS_MAX_PATH + 1];
    wcsncpy(tPreJITModName, item.second.moduleName, OS_MAX_PATH);

    if (wcsstr(tPreJITModName, L"assembly\\NativeImages"))
    {
        if (wcsstr(tPreJITModName, L".ni."))
        {
            wchar_t t_fileName[OS_MAX_PATH];
            wchar_t t_Ext[OS_MAX_PATH];

            // split the current executable file name into drive, path etc.
            _wsplitpath(tPreJITModName, NULL, NULL, t_fileName, t_Ext);
            wsprintf(tPreJITModName, L"%s%s", t_fileName, t_Ext);

            PreJitModSymbolMap::iterator pjsIter;
            pjsIter = m_PJSMap.find(tPreJITModName);

            if (pjsIter != m_PJSMap.end())
            {
                PreJitModSymbolMap::value_type& pjsFileItem = *pjsIter;

                if (!pjsFileItem.second.bHasSample)
                {
                    pjsFileItem.second.bHasSample = true;
                }
            }
        }
    }

    return true;
}

HRESULT WinTaskInfo::GetUserModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick)
{
    HRESULT hr = S_FALSE;

    AddLoadModules(pModInfo->processID);

    // this is user space, check module map.
    ModuleMap::iterator i = m_tiModMap.lower_bound(ModuleKey(pModInfo->processID, pModInfo->sampleAddr, TI_TIMETYPE_MAX));

    for (ModuleMap::iterator iEnd = m_tiModMap.end(); i != iEnd; ++i)
    {
        ModuleMap::value_type& item = *i;

        // different process
        if (item.first.processId != pModInfo->processID)
        {
            break;
        }

        // since the module map is sorted by the process id, module address and time.
        // if module load address is greater than sample address, we don't need
        // go farther.
        if ((item.first.moduleLoadAddr + item.second.moduleSize) < pModInfo->sampleAddr)
        {
            break;
        }

        // if load time is not in module load frame, try next module
        //
        // Baskar: The Java Runtime does not provide the loadtime for the compiled methods. It only
        // provides the load address, code size, code blob, method name. CodeXL's JVMTI agent queries
        // the system to get the current timestamp in the CompiledMethodLoad callback. This timestamp
        // is used as module loadtime.
        //     But, it seems, the compiled method starts executing even before our registered callback
        // gets called. So the load time we calculate in our callback is lagged. And eventually
        // good amount of samples are attributed to "unknown module" instead of respective
        // compiled methods.
        //     Hence, for java modules avoid checking the timestamp till we figure out a better approach.
        // This has a side effect - if the same address range is used by two different compiled methods,
        // then all the samples will be attributed only to the compiled method which was loaded first.
        // This behavior is still OK then attributing those many samples to "unknown module"
        //
        if (0 != systemTimeTick && (item.first.moduleLoadTime >= systemTimeTick || item.second.moduleUnloadTime < systemTimeTick))
        {
            if (evJavaModule != item.second.moduleType)
            {
                //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Unknown Module : systemTimeTick(%d) IP(0x%lx)", systemTimeTick, pModInfo->sampleAddr);
                continue;
            }
        }

        pModInfo->moduleType = item.second.moduleType;

        switch (item.second.moduleType)
        {
            case evPEModule:
                if (GetUserPeModInfo(pModInfo, systemTimeTick, item))
                {
                    hr = S_OK;
                }

                break;

            case evManaged:
            case evJavaModule:
                if (GetUserJitModInfo(pModInfo, systemTimeTick, item))
                {
                    hr = S_OK;
                }

                break;

            case evOCLModule:
            case evInvalidType:
                break;
        }

        if (S_OK == hr)
        {
            break;
        }
    }

    return hr;
}

HRESULT WinTaskInfo::GetKernelModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick)
{
    HRESULT hr = S_FALSE;
    KernelModMap::iterator i;

    KernelModKey t_value(pModInfo->sampleAddr, TI_TIMETYPE_MAX);

    for (i = m_tiKeModMap.lower_bound(t_value); i != m_tiKeModMap.end(); ++i)
    {
        KernelModMap::value_type& kemoditem = *i;

        // Since the module with greater start address will stay on the bottom of map.
        // if module end address is less than sample address, we don't need to go farther
        //
        if (kemoditem.second.keModEndAddr < pModInfo->sampleAddr)
        {
            break;
        }

        if (systemTimeTick && kemoditem.first.keModLoadTime > systemTimeTick)
        {
            continue;
        }

        if (kemoditem.first.keModLoadAddr <= pModInfo->sampleAddr && kemoditem.second.keModEndAddr > pModInfo->sampleAddr)
        {
            pModInfo->ModuleStartAddr = kemoditem.first.keModLoadAddr;
            pModInfo->Modulesize = kemoditem.second.keModEndAddr - kemoditem.first.keModLoadAddr;
            pModInfo->pPeFile = kemoditem.second.pPeFile;
            pModInfo->instanceId = kemoditem.second.instanceId;

            if (!kemoditem.second.bNameConverted)
            {
                wchar_t tStr[OS_MAX_PATH + 1];
                wcsncpy(tStr, kemoditem.second.keModName, OS_MAX_PATH + 1);
                ConvertName(tStr, 0);

                osCriticalSectionLocker lock(m_TIMutexKE);

                if (!kemoditem.second.bNameConverted)
                {
                    wcsncpy(kemoditem.second.keModName, tStr, OS_MAX_PATH + 1);
                    kemoditem.second.bNameConverted = true;
                }
            }

            if (!kemoditem.second.bLoadedPeFile)
            {
                osCriticalSectionLocker lock(m_TIMutexKE);

                if (!kemoditem.second.bLoadedPeFile)
                {
                    if (NULL != kemoditem.second.pPeFile)
                    {
                        delete kemoditem.second.pPeFile;
                    }

                    PeFile* pPeFile = new PeFile(kemoditem.second.keModName);

                    if (NULL != pPeFile)
                    {
                        if (pPeFile->Open(kemoditem.first.keModLoadAddr))
                        {
                            pPeFile->InitializeSymbolEngine(m_pSearchPath, m_pServerList, m_pCachePath);
                        }
                        else
                        {
                            delete pPeFile;
                            pPeFile = NULL;
                        }
                    }

                    pModInfo->pPeFile = pPeFile;
                    kemoditem.second.pPeFile = pPeFile;
                    kemoditem.second.bLoadedPeFile = true;
                    m_bLoadKernelPeFiles = true;
                }
            }

            // if the module image size is not available, then update the module image size;
            if (0ULL == kemoditem.second.keModImageSize)
            {
                gtUInt64 modImageSize = 0;

                if (NULL != kemoditem.second.pPeFile)
                {
                    modImageSize = kemoditem.second.pPeFile->GetImageSize();
                }

                osCriticalSectionLocker lock(m_TIMutexKE);

                if (0ULL == kemoditem.second.keModImageSize)
                {
                    kemoditem.second.keModImageSize = modImageSize;
                }
            }

            if (kemoditem.second.keModImageSize != 0)
            {
                pModInfo->Modulesize = kemoditem.second.keModImageSize;
            }

            // else
            // {
            //  kemoditem.second.keModImageSize = GetModuleSize(kemoditem.second.keModName);
            //  pModInfo->Modulesize = kemoditem.second.keModImageSize;
            // }

            wcsncpy(pModInfo->pModulename, kemoditem.second.keModName,
                    pModInfo->namesize);
            hr = S_OK;

            if (kemoditem.second.keModImageSize != 0)
            {
                if (pModInfo->sampleAddr > kemoditem.first.keModLoadAddr + kemoditem.second.keModImageSize)
                {
                    hr = S_FALSE;
                }
            }

            break;
        }
    }

    return hr;
}

HRESULT WinTaskInfo::FindModuleId(const wchar_t* pModuleName, gtInt32& moduleId)
{
    HRESULT hr = S_OK;

    auto iter = m_moduleIdMap.find(pModuleName);

    if (m_moduleIdMap.end() == iter)
    {
        auto ret = m_moduleIdMap.emplace(pModuleName, AtomicAdd(m_nextModuleId, 1));
        moduleId = ret.second;
        //hr = S_OK;
    }

    return hr;
}

HRESULT ConstructModuleName(ModuleValue& modValue, gtUInt64 pid, std::wstring& modName)
{
    if (modValue.moduleName[0] == L'\0')
    {
        // Module name is unknown
        modName = to_wstring(pid);
    }
    else
    {
        // Module name is known
        modName = modValue.moduleName;
    }

    return S_OK;
}

// GetModuleInstanceId
// Get module instance id for a given sample record .
HRESULT WinTaskInfo::GetModuleInstanceId(gtUInt32 processId, gtUInt64 sampleAddr, gtUInt64 deltaTick, gtUInt32& modInstId)
{
    HRESULT hr = S_FALSE;
    bool moduleFound = false;

    AddLoadModules(processId);

    // this is user space, check module map.
    ModuleMap::iterator i = m_tiModMap.lower_bound(ModuleKey(processId, sampleAddr, TI_TIMETYPE_MAX));

    for (ModuleMap::iterator iEnd = m_tiModMap.end(); i != iEnd; ++i)
    {
        ModuleMap::value_type& item = *i;

        // different process
        if (item.first.processId != processId)
        {
            break;
        }

        // since the module map is sorted by the process id, module address and time.
        // if module load address is greater than sample address, we don't need
        // go farther.
        if ((item.first.moduleLoadAddr + item.second.moduleSize) < sampleAddr)
        {
            break;
        }

        if (0 != deltaTick && (item.first.moduleLoadTime >= deltaTick || item.second.moduleUnloadTime < deltaTick))
        {
            if (evJavaModule != item.second.moduleType)
            {
                //OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Unknown Module : systemTimeTick(%d) IP(0x%lx)", systemTimeTick, pModInfo->sampleAddr);
                continue;
            }
        }

        modInstId = item.second.instanceId;
        hr = S_OK;
        std::wstring modName;
        ConstructModuleName(item.second, item.first.processId, modName);

        FindModuleId(modName.c_str(), item.second.moduleId);
        moduleFound = true;
        break;
    }

    if (false == moduleFound)
    {
        KernelModMap::iterator iter;

        KernelModKey t_value(sampleAddr, TI_TIMETYPE_MAX);

        for (iter = m_tiKeModMap.lower_bound(t_value); iter != m_tiKeModMap.end(); ++iter)
        {
            KernelModMap::value_type& kemoditem = *iter;

            // Since the module with greater start address will stay on the bottom of map.
            // if module end address is less than sample address, we don't need to go farther
            //
            if (kemoditem.second.keModEndAddr < sampleAddr)
            {
                break;
            }

            if (deltaTick && kemoditem.first.keModLoadTime > deltaTick)
            {
                continue;
            }

            if (kemoditem.first.keModLoadAddr <= sampleAddr && kemoditem.second.keModEndAddr > sampleAddr)
            {
                modInstId = kemoditem.second.instanceId;
                hr = S_OK;

                if (!kemoditem.second.bNameConverted)
                {
                    wchar_t tStr[OS_MAX_PATH + 1];
                    wcsncpy(tStr, kemoditem.second.keModName, OS_MAX_PATH + 1);
                    ConvertName(tStr, 0);

                    osCriticalSectionLocker lock(m_TIMutexKE);

                    if (!kemoditem.second.bNameConverted)
                    {
                        wcsncpy(kemoditem.second.keModName, tStr, OS_MAX_PATH + 1);
                        kemoditem.second.bNameConverted = true;
                    }
                }

                if (!kemoditem.second.bLoadedPeFile)
                {
                    osCriticalSectionLocker lock(m_TIMutexKE);

                    if (!kemoditem.second.bLoadedPeFile)
                    {
                        if (NULL != kemoditem.second.pPeFile)
                        {
                            delete kemoditem.second.pPeFile;
                        }

                        PeFile* pPeFile = new PeFile(kemoditem.second.keModName);

                        if (NULL != pPeFile)
                        {
                            if (pPeFile->Open(kemoditem.first.keModLoadAddr))
                            {
                                pPeFile->InitializeSymbolEngine(m_pSearchPath, m_pServerList, m_pCachePath);
                            }
                            else
                            {
                                delete pPeFile;
                                pPeFile = NULL;
                            }
                        }

                        kemoditem.second.pPeFile = pPeFile;
                        kemoditem.second.bLoadedPeFile = true;
                        m_bLoadKernelPeFiles = true;
                    }
                }

                // if the module image size is not available, then update the module image size;
                if (0ULL == kemoditem.second.keModImageSize)
                {
                    gtUInt64 modImageSize = 0;

                    if (NULL != kemoditem.second.pPeFile)
                    {
                        modImageSize = kemoditem.second.pPeFile->GetImageSize();
                    }

                    osCriticalSectionLocker lock(m_TIMutexKE);

                    if (0ULL == kemoditem.second.keModImageSize)
                    {
                        kemoditem.second.keModImageSize = modImageSize;
                    }
                }

                if (kemoditem.second.keModImageSize != 0)
                {
                    if (sampleAddr > kemoditem.first.keModLoadAddr + kemoditem.second.keModImageSize)
                    {
                        //TODO: hr = S_FALSE;
                        modInstId = 0xFF000000 + processId;
                    }
                }

                break;
            }
        }
    }

    return hr;
}

// GetModuleInfoFromInstanceId: Get module information for a give instance id
HRESULT WinTaskInfo::GetLoadModuleInfoByInstanceId(gtUInt32 instanceId, LoadModuleInfo* pModInfo)
{
    HRESULT hr = S_FALSE;
    bool found = false;

    if (NULL == pModInfo)
    {
        return hr;
    }

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    for (auto moditer : m_tiModMap)
    {
        if (instanceId == moditer.second.instanceId)
        {
            pModInfo->m_pid = (gtUInt32)moditer.first.processId;
            pModInfo->m_instanceId = instanceId;
            pModInfo->m_moduleStartAddr = moditer.first.moduleLoadAddr;
            pModInfo->m_modulesize = (gtUInt32)moditer.second.moduleSize;
            pModInfo->m_moduleType = moditer.second.moduleType;
            wcstombs(pModInfo->m_pModulename, moditer.second.moduleName, OS_MAX_PATH);
            pModInfo->m_moduleId = moditer.second.moduleId;
            pModInfo->m_isKernel = false;
            found = true;
            break;
        }
    }

    if (false == found)
    {
        for (auto moditer : m_tiKeModMap)
        {
            if (instanceId == moditer.second.instanceId)
            {
                pModInfo->m_instanceId = instanceId;
                pModInfo->m_moduleStartAddr = moditer.first.keModLoadAddr;
                pModInfo->m_modulesize = (gtUInt32)(moditer.second.keModEndAddr - moditer.first.keModLoadAddr);
                pModInfo->m_isKernel = true;
                wcstombs(pModInfo->m_pModulename, moditer.second.keModName, OS_MAX_PATH);
                //pModInfo->m_moduleId = moditer.second.;
                found = true;
                break;
            }
        }
    }

    return (true == found) ? S_OK : E_FAIL;
}
//////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetModuleInfo( )
//  Get module info for a given sample record.
//
//  Param: [in]     gtUInt64      processID,          | process id
//  Param: [in]     unsigned    CSvalue,            | CS value
//  Param: [in]     gtUInt64      sampleAddr,         | sample address
//  Param: [in]     unsigned    cpuIndex,           | cpu number
//  Param: [in/out] gtUInt64      timestamp,          | sample time stamp
//  Param: [out]    gtUInt64      *pModuleStartAddr,  | module start address
//  Param: [out]    gtUInt64      *pModulesize,       | module image size
//  Param: [out]    char        *pModulename,       | module full name
//  Param: [in]     unsigned    namesize            | length of module name string
//
//Note that it will convert the timestamp to the system mS count
HRESULT WinTaskInfo::GetModuleInfo(TiModuleInfo* pModInfo)
{
    HRESULT hr = S_FALSE;

    if (NULL == pModInfo)
    {
        return hr;
    }

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    // check the cpu index
    if (static_cast<int>(pModInfo->cpuIndex) >= m_affinity)
    {
        return hr;
    }

    TiTimeType t_time = static_cast<TiTimeType>(pModInfo->deltaTick);

    if (pModInfo->sampleAddr < m_lpMaxAppAddr && pModInfo->sampleAddr > m_lpMinAppAddr)
    {
        pModInfo->kernel = false;
        hr = GetUserModInfo(pModInfo, t_time);
    }
    else
    {
        pModInfo->kernel = true;
        // sample is in the kernel space

        hr = GetKernelModInfo(pModInfo, t_time);
    }

    std::wstring modName;

    if (pModInfo->pModulename[0] == L'\0')
    {
        // Module name is unknown
        modName = to_wstring(pModInfo->processID);
    }
    else
    {
        // Module name is known
        modName = pModInfo->pModulename;
    }

    osCriticalSectionLocker lock(m_TIMutexModule);

    auto iter = m_moduleIdMap.find(modName);

    if (m_moduleIdMap.end() == iter)
    {
        auto ret = m_moduleIdMap.emplace(modName, AtomicAdd(m_nextModuleId, 1));
        iter = ret.first;
    }

    pModInfo->moduleId = static_cast<gtUInt32>(iter->second);

    return hr;
}

HRESULT WinTaskInfo::GetProcessThreadList(gtVector<std::tuple<gtUInt32, gtUInt32>>& info)
{
    HRESULT hr = S_OK;

    for (const auto& it : m_interestingPidMap)
    {
        ThreadInfoKey t_threadKey(static_cast<gtUInt64>(it.first), 0xFFFFFFFF);
        auto iter = m_ThreadMap.upper_bound(t_threadKey);

        if (m_ThreadMap.end() != iter)
        {
            --iter;

            while (iter->first.processID == it.first)
            {
                info.emplace_back(static_cast<gtUInt32>(iter->first.processID), static_cast<gtUInt32>(iter->first.threadID));

                if (m_ThreadMap.begin() == iter)
                {
                    break;
                }

                --iter;
            }
        }
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetKernelModNum(unsigned *pKeModNum)
//  Get number of kernel modules.
//
//  Param: [out] unsigned | *pKeModNum | reference pointer of kernel module number
//
HRESULT WinTaskInfo::GetKernelModNum(unsigned* pKeModNum)
{
    HRESULT hr = S_OK;

    if (NULL == pKeModNum)
    {
        return S_FALSE;
    }

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    *pKeModNum = (unsigned) m_tiKeModMap.size();

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetAllKeMod(KeModQueryInfo *pKeMods, unsigned keModNum)
//  Get all kernel module info.
//
//  Param: [out] KeModQueryInfo | *pKeMods | reference pointer of kernel module info structure.
//  Param: [in]  unsigned       | keModNum | number of kernel modules
//
HRESULT WinTaskInfo::GetAllKeMod(KeModQueryInfo* pKeMods, unsigned keModNum)
{
    HRESULT hr = S_OK;

    if (NULL == pKeMods)
    {
        return S_FALSE;
    }

    // if maps are build
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    unsigned count = 0;

    for (KernelModMap::iterator it = m_tiKeModMap.begin(), itEnd = m_tiKeModMap.end(); it != itEnd; ++it)
    {
        if (count >= keModNum)
        {
            hr = E_FAIL;
            break;
        }

        KernelModMap::value_type& keModItem = *it;

        pKeMods[count].keModStartAddr = keModItem.first.keModLoadAddr;
        pKeMods[count].keModEndAddr = keModItem.second.keModEndAddr;

        if (!keModItem.second.bNameConverted)
        {
            ConvertName(keModItem.second.keModName, 0);
            keModItem.second.bNameConverted = true;
        }

        wcscpy(pKeMods[count].keModName, keModItem.second.keModName);
        count++;
    }

    return hr;
}


HRESULT WinTaskInfo::GetCpuProfilingDriversMaxCount(unsigned* pKeModNum) const
{
    HRESULT hr;

    if (NULL != pKeModNum)
    {
        *pKeModNum = 2U;
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}


// Returns true if the path is of the AMD CPU Profiling driver.
static bool IsCpuProfilingDriver(const gtString& absolutePath)
{
    bool ret = false;

    // The smallest approved string is "C:\windows\system32\drivers\pcore.sys", which length is 37.
    int len = absolutePath.length();

    if (37 <= len)
    {
        gtString lowerAbsolutePath = absolutePath;
        lowerAbsolutePath.toLowerCase();

        // Search for the string "\windows\system32\drivers\", which length is 26.
        int pos = lowerAbsolutePath.find(L"\\windows\\system32\\drivers\\");

        if (-1 != pos)
        {
            pos += 26;
            int fileLen = len - pos;

            // The length of "pcore.sys" is 9.
            if (9 == fileLen)
            {
                ret = (0 == lowerAbsolutePath.compare(pos, 9, L"pcore.sys"));
            }
            // The length of "cpuprof.sys" is 11.
            else if (11 == fileLen)
            {
                ret = (0 == lowerAbsolutePath.compare(pos, 11, L"cpuprof.sys"));
            }
        }
    }

    return ret;
}


HRESULT WinTaskInfo::GetCpuProfilingDrivers(KeModQueryInfo* pKeMods, unsigned& keModNum)
{
    HRESULT hr = S_OK;

    if (NULL != pKeMods)
    {
        unsigned countLeft = keModNum;

        for (KernelModMap::iterator it = m_tiKeModMap.begin(), itEnd = m_tiKeModMap.end(); it != itEnd; ++it)
        {
            if (0U == countLeft)
            {
                break;
            }

            KernelModMap::value_type& keModItem = *it;

            if (!keModItem.second.bNameConverted)
            {
                ConvertName(keModItem.second.keModName, 0);
                keModItem.second.bNameConverted = true;
            }

            if (IsCpuProfilingDriver(keModItem.second.keModName))
            {
                pKeMods->keModStartAddr = keModItem.first.keModLoadAddr;
                pKeMods->keModEndAddr = keModItem.second.keModEndAddr;
                wcscpy(pKeMods->keModName, keModItem.second.keModName);

                pKeMods++;
                countLeft--;
            }
        }

        keModNum -= countLeft;
    }
    else
    {
        if (0U != keModNum)
        {
            hr = S_FALSE;
        }
    }

    return hr;
}


////////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetNumThreadInProcess()
//  Get number of thread in the process for a given process ID.
//      Note: The whole thread map only contains thread info for process that is launched
//              after profiling starts.
//
//  Param: [in] gtUInt64 |processID | process ID
//
unsigned int WinTaskInfo::GetNumThreadInProcess(gtUInt64 processID)
{
    unsigned int count = 0;

    ThreadInfoMap::iterator threadIter;

    for (threadIter = m_ThreadMap.begin(); threadIter != m_ThreadMap.end(); ++threadIter)
    {

        ThreadInfoMap::value_type& threadItem = *threadIter;

        if (threadItem.first.processID == processID)
        {
            count++;
        }
    }

    return count;
}

////////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::GetThreadInfoInProcess()
//  Get thread info for a given process ID.
//
//  Param: [in] gtUInt64 |processID | process ID
//  Param: [in] UINT   |entriesOfThreadInfo | count of entries in thread info array
//  Param: [out] TI_THREAD_INFO* | pThreadInfoArray | thread info array
//
HRESULT WinTaskInfo::GetThreadInfoInProcess(gtUInt64 processID,
                                            unsigned int sizeOfInfoArray, TI_THREAD_INFO* pThreadInfoArray)
{
    HRESULT hr = S_OK;
    unsigned int count = 0;

    if (!pThreadInfoArray)
    {
        return E_FAIL;
    }

    ThreadInfoMap::iterator threadIter;

    for (threadIter = m_ThreadMap.begin(); threadIter != m_ThreadMap.end(); ++threadIter)
    {
        ThreadInfoMap::value_type& threadItem = *threadIter;

        if (threadItem.first.processID == processID)
        {
            if (count < sizeOfInfoArray)
            {
                pThreadInfoArray[count].threadID = threadItem.first.threadID;
                pThreadInfoArray[count].cpuNumCreated = threadItem.second.cpuNumCreated;
                pThreadInfoArray[count].cpuNumDeleted = threadItem.second.cpuNumDeleted;

                pThreadInfoArray[count].threadCreationTime = threadItem.second.threadCreationTime;

                pThreadInfoArray[count].threadDeletionTime = threadItem.second.threadDeletionTime;

                count++;
            }
            else
            {
                hr = E_FAIL;
                break;
            }
        }
    }

    return hr;
}



////////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::ReadModuleInfoFile(char *filename)
//  Read process, module and kernel module info from task info file, and build up maps.
//
//  Param: [in] char |*filename | task info file name
//
HRESULT WinTaskInfo::ReadModuleInfoFile(const wchar_t* filename)
{
    HRESULT hr = S_OK;

    DWORD versionNumber = 0x0;

    // clean up the previous process, module, kernel module map.
    Cleanup();

    if (!m_bPartitionInfoflag)
    {
        // clean the drive map first. In case drive map has data from reading task info file.
        GetDrivePartitionInfo();
    }

    // open task info file.
    FILE* p_tifile = _wfopen(filename, L"r+b");
    TimeMark* pTimes = NULL;

    bool readConvert = false;
    bool read64bitTS = false;

    if (p_tifile == NULL)
    {
        hr = E_INVALIDPATH;
    }

    if (S_OK == hr)
    {
        // move to the begin of the file
        rewind(p_tifile);

        // read the taskinfo file signature;
        DWORD t_d32;
        fread(&t_d32, sizeof(DWORD), 1, p_tifile);

        if (t_d32 != TASK_INFO_FILE_SIGNATURE)
        {
            hr = S_FALSE;
            m_ErrorMsg = "Obsolete task info file format";
        }

        // read the task info file format version;
        // Read more description below.
        fread(&versionNumber, sizeof(versionNumber), 1, p_tifile);

        if (versionNumber < TASK_INFO_FILE_VERSION)
        {
            if (versionNumber == TASK_INFO_FILE_VERSION_1)
            {
                //Version 1 file names were written in ascii, uncommon
                readConvert = true;
            }
            else if (versionNumber < TASK_INFO_FILE_VERSION_1)
            {
                //Files before version one are no longer supported, probably not an issue
                hr = S_FALSE;
                m_ErrorMsg = "Obsolete task info file format";
            }

            //Version 2+ file names were written using wchar_t, as currently expected.
        }

        if (versionNumber >= TASK_INFO_FILE_VERSION_4)
        {
            read64bitTS = true;
        }

    }

    if (S_OK == hr)
    {
        // read affinity
        m_affinity = 1;
        fread(&m_affinity, sizeof(m_affinity), 1, p_tifile);

        if (m_affinity > 0)
        {
            pTimes = new TimeMark[m_affinity];

            if (NULL == pTimes)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if (S_OK == hr)
    {
        if (!read64bitTS)
        {
            fread(pTimes, sizeof(TimeMark), m_affinity, p_tifile);
            m_startHr = pTimes[0].sysTime_ms;
            fread(pTimes, sizeof(TimeMark), m_affinity, p_tifile);
            m_EndHr = pTimes[0].sysTime_ms;
        }
        else
        {
            fread(pTimes, sizeof(TimeMark), m_affinity, p_tifile);
            m_startHr = pTimes[0].cpuTimeStamp;
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"start profile timestamp (%lu) ", m_startHr);
            fread(pTimes, sizeof(TimeMark), m_affinity, p_tifile);
            m_EndHr = pTimes[0].cpuTimeStamp;
        }

        fread(&m_lpMaxAppAddr, sizeof(gtUInt64), 1, p_tifile);
        fread(&m_lpMinAppAddr, sizeof(gtUInt64), 1, p_tifile);

        // read the process info
        DWORD count = (DWORD) m_tiProcMap.size();
        fread(&count, sizeof(DWORD), 1, p_tifile);

        wchar_t buffer[OS_MAX_PATH + 1];
        char buffChar[OS_MAX_PATH + 1];
        gtUInt64 t_pid = 0;
        gtUInt64 t_u64 = 0;
        DWORD t_d32 = 0;
        unsigned int i;

        // Kludge:
        // CodeAnalyst 2.57.xx is using WinTaskInfo file format 0x00010002. It does not have UNICODE support.
        // CodeAnalyst 2.69.xx - 2.69.271 update file format format to support UNICODE. However, the file
        // format version was not updated in those builds.
        // We update version number to match up UNICODE support since CodeAnalyst 2.69.279.
        // To distinguish WinTaskInfo file format in 2.57.xx and 2.69.xx - 2.69.271, I add the following kludge.
        //
        // Since at here, we are going to read process info, process 0 is alway on the top.
        // The string name of process 0 is always "System Idle process".
        // If the file has UNICODE support, the second byte of process 0 string will be 0x00. This is task info
        // file generated in [2.69.xx - 2.69.271]. We don't need convert file for it.
        // Otherwise, it's taskinfo file generate in 2.57.xx. We need the conversion.
        //
        // --Lei
        //
        long processBeginPos = ftell(p_tifile);

        if (readConvert)
        {
            // 20 = sizeof(process id) + sizeof(dProcStartTIme) + sizeof(dProcEndTime) + sizeof(string length);
            fseek(p_tifile, 20, SEEK_CUR);
            char tchar[2];
            memset(tchar, 0x00, 2);
            fread(&tchar, sizeof(char), 2, p_tifile);

            if (tchar[1] == 0x00)
            {
                readConvert = false;
            }

            fseek(p_tifile, processBeginPos, SEEK_SET);
        }

        for (i = 0; i < count; ++i)
        {
            ProcessValue t_procValue;

            // read process id;
            fread(&t_pid, sizeof(gtUInt64), 1, p_tifile);

            // read dProcStartTime
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            //convert to deltaTick, if necessary
            if (m_startHr < t_d32)
            {
                t_d32 = CalculateDeltaTick(t_d32);
            }

            t_procValue.dProcStartTime = t_d32;

            // read dProcEndTime
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            //convert to deltaTick, if necessary
            if ((((DWORD) - 1) != t_d32) && (m_startHr < t_d32))
            {
                t_d32 = CalculateDeltaTick(t_d32);
            }

            t_procValue.dProcEndTime = t_d32;

            // read string length
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);
            memset((void*)buffer, 0x00, sizeof(buffer));

            //if reading wchar
            if (!readConvert)
            {
                fread(&buffer, sizeof(wchar_t), t_d32, p_tifile);
            }
            else
            {
                memset((void*)buffChar, 0x00, sizeof(buffChar));
                fread(&buffChar, sizeof(char), t_d32, p_tifile);
                mbstowcs(buffer, buffChar, t_d32);
            }

            wcscpy(t_procValue.processName, buffer);
            t_procValue.bNameConverted = false;
            m_tiProcMap.insert(ProcessMap::value_type(t_pid, t_procValue));
        }


        // read module number
        fread(&count, sizeof(DWORD), 1, p_tifile);

        for (i = 0; i < count; i++)
        {
            // read process id
            fread(&t_pid, sizeof(gtUInt64), 1, p_tifile);

            // read loading addr
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);

            // read loading time
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            //convert to deltaTick, if necessary
            if (m_startHr < t_d32)
            {
                t_d32 = CalculateDeltaTick(t_d32);
            }

            ModuleKey t_modKey(t_pid, t_u64, t_d32);
            ModuleValue t_modValue;

            // read module base
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);
            t_modValue.moduleBaseAddr = t_u64;

            // read module size
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);
            t_modValue.moduleSize = t_u64;

            // read unload time
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            //convert to deltaTick, if necessary
            if ((((DWORD) - 1) != t_d32) && (m_startHr < t_d32))
            {
                t_d32 = CalculateDeltaTick(t_d32);
            }

            t_modValue.moduleUnloadTime = t_d32;

            // read string length
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);
            memset(buffer, 0x00, sizeof(buffer));

            //if reading wchar
            if (!readConvert)
            {
                fread(&buffer, sizeof(wchar_t), t_d32, p_tifile);
            }
            else
            {
                memset((void*)buffChar, 0x00, sizeof(buffChar));
                fread(&buffChar, sizeof(char), t_d32, p_tifile);
                mbstowcs(buffer, buffChar, t_d32);
            }

            wcscpy(t_modValue.moduleName, buffer);
            t_modValue.bNameConverted = false;

            //m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
            m_allModulesMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
        }

        // read kernel module number
        fread(&count, sizeof(DWORD), 1, p_tifile);

        for (i = 0; i < count; i++)
        {
            // read module load addr
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);

            // read module load time
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            //convert to deltaTick, if necessary
            if (m_startHr < t_d32)
            {
                t_d32 = CalculateDeltaTick(t_d32);
            }

            KernelModKey t_keModKey(t_u64, t_d32);
            KernelModValue t_keModValue;

            // read kernel module end address
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);
            t_keModValue.keModEndAddr = t_u64;

            // read kernel module base
            fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);
            t_keModValue.keModBase = t_u64;

            // read kernel module unload time
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);

            //convert to deltaTick, if necessary
            if ((((DWORD) - 1) != t_d32) && (m_startHr < t_d32))
            {
                t_d32 = CalculateDeltaTick(t_d32);
            }

            t_keModValue.keModUnloadTime = t_d32;

            // read string length
            fread(&t_d32, sizeof(DWORD), 1, p_tifile);
            memset(buffer, 0x00, sizeof(buffer));

            //if reading wchar
            if (!readConvert)
            {
                fread(&buffer, sizeof(wchar_t), t_d32, p_tifile);
            }
            else
            {
                memset((void*)buffChar, 0x00, sizeof(buffChar));
                fread(&buffChar, sizeof(char), t_d32, p_tifile);
                mbstowcs(buffer, buffChar, t_d32);
            }

            wcscpy(t_keModValue.keModName, buffer);
            t_keModValue.bNameConverted = false;
            t_keModValue.keModImageSize = 0;
            t_keModValue.instanceId = m_nextModInstanceId++;

            m_tiKeModMap.insert(KernelModMap::value_type(t_keModKey, t_keModValue));

            std::wstring modName(t_keModValue.keModName);
            auto iter = m_moduleIdMap.find(modName);

            if (m_moduleIdMap.end() == iter)
            {
                m_moduleIdMap.emplace(modName, AtomicAdd(m_nextModuleId, 1));
            }
        }

        if (TASK_INFO_FILE_VERSION_2 <= versionNumber)
        {
            // read Thread Info count
            fread(&count, sizeof(DWORD), 1, p_tifile);

            for (i = 0; i < count; i++)
            {
                // read process ID
                fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);
                // read thread id
                fread(&t_d32, sizeof(DWORD), 1, p_tifile);

                ThreadInfoKey t_threadKey(t_u64, t_d32);
                ThreadInfoValue t_threadValue;
                t_threadValue.threadID = t_d32;

                // processor the number the thread was created on
                fread(&t_d32, sizeof(DWORD), 1, p_tifile);
                t_threadValue.cpuNumCreated = t_d32;

                // processor the number the thread was deleted on
                fread(&t_d32, sizeof(DWORD), 1, p_tifile);
                t_threadValue.cpuNumDeleted = t_d32;

                // thread creation time.
                fread(&t_d32, sizeof(DWORD), 1, p_tifile);

                //convert to deltaTick, if necessary
                if (m_startHr < t_d32)
                {
                    t_d32 = CalculateDeltaTick(t_d32);
                }

                t_threadValue.threadCreationTime = t_d32;

                // thread deletion time.
                fread(&t_d32, sizeof(DWORD), 1, p_tifile);

                //convert to deltaTick, if necessary
                if ((((DWORD) - 1) != t_d32) && (m_startHr < t_d32))
                {
                    t_d32 = CalculateDeltaTick(t_d32);
                }

                t_threadValue.threadDeletionTime = t_d32;

                m_ThreadMap.insert(ThreadInfoMap::value_type(t_threadKey, t_threadValue));
            }
        }

        if (TASK_INFO_FILE_VERSION_3 <= versionNumber)
        {
            // read Bitness Map count
            fread(&count, sizeof(DWORD), 1, p_tifile);

            for (i = 0; i < count; i++)
            {
                // read process ID
                fread(&t_u64, sizeof(gtUInt64), 1, p_tifile);
                // read bitness
                fread(&t_d32, sizeof(DWORD), 1, p_tifile);
                m_bitnessMap[t_u64] = (t_d32 ? true : false);
            }
        }
    }

    if (NULL != pTimes)
    {
        delete [] pTimes;
    }

    if (S_OK == hr)
    {
        m_bMapBuilt = true;
    }

    // close task info file.
    if (p_tifile != NULL)
    {
        fclose(p_tifile);
        p_tifile = NULL;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////////////
// WinTaskInfo::WriteModuleInfoFile(char *filename)
//  Write process, module, kernel module info into task info file.
//
//  Param: char | *filename  | task info file name.
//
HRESULT WinTaskInfo::WriteModuleInfoFile(const wchar_t* filename)
{
    HRESULT hr = S_OK;
    gtUInt64 t_u64;
    DWORD t_d32;

    // check map status
    if (! m_bMapBuilt)
    {
        m_ErrorMsg = "Maps are not built up yet.";
        return S_FALSE;
    }

    FILE* p_tifile = _wfopen(filename, L"w+b");

    if (p_tifile == NULL)
    {
        return E_ACCESSDENIED;
    }

    TimeMark* pTimes = new TimeMark[m_affinity];

    if (pTimes == NULL)
    {
        fclose(p_tifile);
        return E_OUTOFMEMORY;
    }

    // move to the begin of the file
    rewind(p_tifile);

    // write the taskinfo file signature;
    t_d32 = (DWORD) TASK_INFO_FILE_SIGNATURE;
    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

    // write the task info file signature;
    t_d32 = (DWORD) TASK_INFO_FILE_VERSION;
    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

    // write the variables
    fwrite(&m_affinity, sizeof(m_affinity), 1, p_tifile);
    pTimes[0].cpuTimeStamp = m_startHr;
    fwrite(pTimes, sizeof(TimeMark), m_affinity, p_tifile);
    pTimes[0].cpuTimeStamp = m_EndHr;
    fwrite(pTimes, sizeof(TimeMark), m_affinity, p_tifile);
    delete [] pTimes;
    fwrite(&m_lpMaxAppAddr, sizeof(gtUInt64), 1, p_tifile);
    fwrite(&m_lpMinAppAddr, sizeof(gtUInt64), 1, p_tifile);

    // write the process info
    DWORD count = (DWORD) m_tiProcMap.size();

    // write number of processes
    fwrite(&count, sizeof(DWORD), 1, p_tifile);

    ProcessMap::iterator i;

    for (i = m_tiProcMap.begin(); i != m_tiProcMap.end(); ++i)
    {
        ProcessMap::value_type& procItem = *i;

        // write process id
        gtUInt64 pid = procItem.first;
        fwrite(&pid, sizeof(gtUInt64), 1, p_tifile);

        // write process start time
        DWORD t_data = procItem.second.dProcStartTime;
        fwrite(&t_data, sizeof(DWORD), 1, p_tifile);

        // write process end time
        t_data = procItem.second.dProcEndTime;
        fwrite(&t_data, sizeof(DWORD), 1, p_tifile);

        // write length of process name
        t_data = (DWORD) wcslen(procItem.second.processName);
        fwrite(&t_data, sizeof(DWORD), 1, p_tifile);

        // write process name
        fwrite((void*) procItem.second.processName, sizeof(wchar_t), t_data, p_tifile);
    }

    // write module info
    ModuleMap::iterator moditer;

    // write number of modules
    t_d32 = (DWORD) m_tiModMap.size() - m_JitModCount;
    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

    for (moditer = m_tiModMap.begin(); moditer != m_tiModMap.end(); ++moditer)
    {
        ModuleMap::value_type& item = *moditer;

        if ((item.second.moduleType == evJavaModule) ||
            (item.second.moduleType == evManaged))
        {
            continue;
        }

        // write process id
        t_u64 = item.first.processId;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write module load address
        t_u64 = item.first.moduleLoadAddr;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write module load time
        t_d32 = item.first.moduleLoadTime;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // write module image base
        t_u64 = item.second.moduleBaseAddr;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write module size
        t_u64 = item.second.moduleSize;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write module unload time
        t_d32 = item.second.moduleUnloadTime;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // write length of module full name
        t_d32 = (DWORD) wcslen(item.second.moduleName);
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // write module full name
        fwrite((void*) item.second.moduleName, sizeof(wchar_t), t_d32, p_tifile);
    }

    // Write kernel module number
    KernelModMap::iterator keIter;
    t_d32 = (DWORD) m_tiKeModMap.size();
    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

    for (keIter = m_tiKeModMap.begin(); keIter != m_tiKeModMap.end(); ++keIter)
    {
        KernelModMap::value_type& kemoditem = *keIter;

        // write kernel module load address
        t_u64 = kemoditem.first.keModLoadAddr & ERBT_713_NON_CANONICAL_MASK;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write kernel module load time
        t_d32 = kemoditem.first.keModLoadTime;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // write kernel module end address
        t_u64 = kemoditem.second.keModEndAddr & ERBT_713_NON_CANONICAL_MASK;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write kernel module image base
        t_u64 = kemoditem.second.keModBase & ERBT_713_NON_CANONICAL_MASK;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write kernel module unload time
        t_d32 = kemoditem.second.keModUnloadTime;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // write length of kernel module name
        t_d32 = (DWORD) wcslen(kemoditem.second.keModName);
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // write kernel module name
        fwrite((void*) kemoditem.second.keModName, sizeof(wchar_t), t_d32, p_tifile);
    }

    // Write Thread Info count
    t_d32 = (DWORD) m_ThreadMap.size();
    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

    ThreadInfoMap::iterator threadIter;

    for (threadIter = m_ThreadMap.begin(); threadIter != m_ThreadMap.end(); ++threadIter)
    {
        ThreadInfoMap::value_type& threadItem = *threadIter;

        // write process ID
        t_u64 = threadItem.first.processID;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write thread id
        t_d32 = threadItem.first.threadID;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // processor the number the thread was created on
        t_d32 = threadItem.second.cpuNumCreated;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // processor the number the thread was deleted on
        t_d32 = threadItem.second.cpuNumDeleted;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // thread creation time.
        t_d32 = threadItem.second.threadCreationTime;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

        // thread deletion time.
        t_d32 = threadItem.second.threadDeletionTime;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);
    }

    //The bitness map should be part of the process map, but to be compatible
    // with previous versions of the .ti file, it's written separately here

    // Write Bitness map count
    t_d32 = (DWORD) m_bitnessMap.size();
    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

    BitnessMap::iterator bitIt;

    for (bitIt = m_bitnessMap.begin(); bitIt != m_bitnessMap.end(); ++bitIt)
    {
        // write process ID
        t_u64 = bitIt->first;
        fwrite(&t_u64, sizeof(gtUInt64), 1, p_tifile);

        // write bitness
        t_d32 = bitIt->second;
        fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);
    }

    if (p_tifile != NULL)
    {
        fclose(p_tifile);
        p_tifile = NULL;
    }

    return hr;
}

typedef QMap<gtUInt64, JclWriter*> JclMap;  //process id, jcl...
typedef QMap<ModuleKey, int> ModIdMap;

#ifndef CXL_SUPPORTS_INTERPRETED_CODE

// key (unsigned __64) is moduleID from CLR, QString is CLR module name.
typedef gtMap<gtUInt64, QString> CLRModMap;
//Function id is key, load addr is value
typedef QMap<FunctionID, gtUInt64> FunIdToAddrMap;
//Note that the directory string should not end in a "\"
HRESULT WinTaskInfo::ReadCLRJitInformation(/* [in] */ const wchar_t* clrdirectory,
                                                      const wchar_t* sessionDir)
{
    HRESULT hr = S_OK;
    CLRModMap clrModMap;
    FunIdToAddrMap clrFunMap;
    unsigned long clrPid = 0;

    //for each process id directory in the given directory
    QDir search_dir(QString::fromWCharArray(clrdirectory));

    if (search_dir.exists())
    {
        QFileInfoList di_list = search_dir.entryInfoList(QDir::AllDirs);

        //while there are directories left,
        for (int i = 0; i < di_list.size(); i++)
        {
            QString tempPath = QString::fromWCharArray(clrdirectory)
                               + di_list.at(i).fileName();

            QString celFile = tempPath;
            celFile += QString(osFilePath::osPathSeparator) + di_list.at(i).fileName() + ".cel";

            if (!QFile::exists(celFile))
            {
                continue;
            }

            clrPid = di_list.at(i).fileName().toULong();

            if (0 == clrPid)
            {
                continue;
            }

            QString sess_pid = QString::fromWCharArray(sessionDir) + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
            QDir sess_pid_dir;
            sess_pid_dir.mkpath(sess_pid);

            // copy file from temp dir to session dir;
            CopyFilesToDestionDir(tempPath, sess_pid);
            QDir tCurrentDir(sess_pid);

            QFileInfoList pjsFile_list = tCurrentDir.entryInfoList(QStringList("*.pjs"));

            //while there are files left,
            for (int j = 0; j < pjsFile_list.size(); j++)
            {
                QString pjsFileName = pjsFile_list.at(j).fileName();

                QString pjsFilePath = sess_pid + QString(osFilePath::osPathSeparator) + pjsFileName;

                pjsFileName.remove(pjsFileName.length() - 4, 4);
                PreJitModSymbolFile tSymFile(pjsFilePath.toStdWString().c_str());
                m_PJSMap.insert(PreJitModSymbolMap::value_type(pjsFileName.toStdWString().c_str(), tSymFile));
            }

            clrModMap.clear();
            clrFunMap.clear();

            CelReader tClrReader;
            celFile = sess_pid + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName() + ".cel";

            if (!tClrReader.Open(celFile.toStdWString().c_str()))
            {
                continue;
            }

            bool bitness  = tClrReader.Is32BitProcess();
            gtUInt64 jitProcessID = di_list.at(i).fileName().toULongLong();
            m_bitnessMap[jitProcessID] = bitness;

            DWORD recType = tClrReader.GetNextRecordType();

            while (recType != evInvalidCLREvent)
            {
                HRESULT hr1 = S_OK;

                switch (recType)
                {
                    case evAppDomainCreationFinished:
                    {
                        AppDomainRecord domainRec;
                        hr1 = tClrReader.GetAppDomainCreationRecord(&domainRec);
                    }
                    break;

                    case evAppDomainShutdownStarted:
                    {
                        AppDomainID domainId;
                        gtUInt64 systime;
                        hr1 = tClrReader.GetAppDomainShutdownRecord(&domainId, &systime);
                    }
                    break;

                    case evAssemblyLoadFinished:
                    {
                        AssemblyRecord asmRec;
                        hr1 = tClrReader.GetAssemblyLoadRecord(&asmRec);
                    }
                    break;

                    case evAssemblyUnloadStarted:
                    {
                        AssemblyID assemblyId;
                        gtUInt64 systime;
                        hr1 = tClrReader.GetAssemblyUnloadRecord(&assemblyId, &systime);
                    }
                    break;

                    case evModuleLoadFinished:
                    {
                        ModuleRecord modRec;
                        hr1 = tClrReader.GetModuleLoadRecord(&modRec);

                        if (S_OK == hr1)
                        {
                            clrModMap[modRec.modId] = QString::fromWCharArray(modRec.modName);
                        }
                    }
                    break;

                    case evModuleUnloadStarted:
                    {
                        ModuleID modId;
                        gtUInt64 systime;
                        hr1 = tClrReader.GetModuleUnloadRecord(&modId, &systime);
                    }
                    break;

                    case evModuleAttachedToAssembly:
                    {
                        ModuleID moduleId;
                        AssemblyID assemblyId;
                        gtUInt64 assemblyLoadAddr = 0;
                        wchar_t assemblyPath[OS_MAX_PATH];
                        hr1 = tClrReader.GetModuleAttachedToAssemblyRec(&moduleId, &assemblyId,
                                                                        &assemblyLoadAddr, assemblyPath, OS_MAX_PATH);

                        if (hr1 == S_OK && assemblyLoadAddr)
                        {
                            // Due to OS had truncated assembly for some ngened assembly;
                            // need to update Ngened assembly full path.
                            // Lei 05/04/2010
                            ModuleMap::iterator j;
                            j = m_tiModMap.lower_bound(ModuleKey(clrPid, assemblyLoadAddr, TI_TIMETYPE_MAX));

                            for (; j != m_tiModMap.end(); ++j)
                            {
                                ModuleMap::value_type& item = *j;

                                if (item.first.processId != clrPid)
                                {
                                    break;
                                }

                                if (item.first.moduleLoadAddr != assemblyLoadAddr)
                                {
                                    break;
                                }

                                wcscpy_s(item.second.moduleName, OS_MAX_PATH, assemblyPath);
                            }
                        }
                    }
                    break;

                    case evClassLoadFinished:
                    {
                        ClassRecord classRec;
                        hr1 = tClrReader.GetClassLoadRecord(&classRec);
                    }
                    break;

                    case evClassUnloadStarted:
                    {
                        ClassID classId;
                        gtUInt64 systime;
                        hr1 = tClrReader.GetClassUnloadRec(&classId, &systime);
                    }
                    break;

                    case evJITCompilationFinished:
                    {
                        FunctionRecord funcRec;
                        funcRec.loadTime = 0;
                        funcRec.jitLoadAddr = 0;
                        hr1 = tClrReader.GetJITCompilationFinished(&funcRec);

                        if (S_OK == hr1)
                        {
                            gtUInt64 tModuleId = funcRec.modId;
                            QString tModuleName = clrModMap[tModuleId];
                            QString tClassFunc;

                            gtUInt64 tLoadAddress = funcRec.jitLoadAddr;
                            clrFunMap[funcRec.funcId] = tLoadAddress;
                            funcRec.loadTime = CalculateDeltaTick(funcRec.loadTime);
                            ModuleKey t_modKey(clrPid, tLoadAddress, (TiTimeType)funcRec.loadTime);

                            tClassFunc = QString::fromWCharArray(funcRec.className) +
                                         "::" + QString::fromWCharArray(funcRec.funcName);

                            ModuleValue t_modValue(0, static_cast<gtUInt64>(funcRec.codeSize),
                                                   TI_TIMETYPE_MAX, tClassFunc.toStdWString().c_str(), evManaged);
                            t_modValue.instanceId = m_nextModInstanceId++;

                            m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
                            m_JitModCount++;

                            std::wstring modName(t_modValue.moduleName);
                            auto iter = m_moduleIdMap.find(modName);

                            if (m_moduleIdMap.end() == iter)
                            {
                                m_moduleIdMap.emplace(modName, AtomicAdd(m_nextModuleId, 1));
                            }

                            tModuleName.append(".jit");
                            QString repPath = QString::fromWCharArray(funcRec.jncFileName);
                            repPath.replace(tempPath, sess_pid);
                            wcscpy(funcRec.jncFileName, repPath.toStdWString().c_str());

                            JitBlockValue tJitBlockValue(tModuleName.toStdWString().c_str(), funcRec.jncFileName, L"UnknownJITSource");
                            m_JitClrMap.insert(JitBlockInfoMap::value_type(t_modKey, tJitBlockValue));
                        }
                    }
                    break;

                    case evFunctionUnloadStarted:
                    {
                        FunctionID funcId;
                        gtUInt64 systime;
                        hr1 = tClrReader.GetFunctionUnloadStarted(&funcId, &systime);
                        ModuleMap::iterator it;

                        for (it = m_tiModMap.begin(); it != m_tiModMap.end(); ++it)
                        {
                            ModuleMap::value_type& item1 = *it;

                            //note that item.first is the key, and item.second is the value
                            if (item1.first.processId != clrPid)
                            {
                                continue;
                            }

                            if (item1.first.moduleLoadAddr != clrFunMap[funcId])
                            {
                                continue;
                            }

                            if (item1.first.moduleLoadTime >= systime)
                            {
                                continue;
                            }

                            if (TI_TIMETYPE_MAX != item1.second.moduleUnloadTime)
                            {
                                continue;
                            }

                            item1.second.moduleUnloadTime = CalculateDeltaTick(systime);
                        }
                    }
                }

                if (S_OK != hr1)
                {
                    break;
                }

                recType = tClrReader.GetNextRecordType();
            }

            tClrReader.Close();
        }
    }

    clrModMap.clear();
    clrFunMap.clear();

    return hr;
}

#endif // #ifndef CXL_SUPPORTS_INTERPRETED_CODE

typedef QMap<gtUInt64, CelWriter*> CelMap;  //process id, cel...

//Note that the directory string should not end in a "\"
HRESULT WinTaskInfo::WriteCLRJncFiles(/*[in]*/ const wchar_t* directory)
{
    HRESULT hr = S_OK;

    QString qtstrDir = QString::fromWCharArray(directory);

    if (qtstrDir.isEmpty())
    {
        qtstrDir = ".";    //set as local directory
    }

    JitBlockInfoMap::iterator map_it;

    CelMap cel_map;
    CelMap::iterator cel_it;
    ModIdMap modIdMap;
    int modId = 0;
    int funcId = 0;

    for (map_it = m_JitClrMap.begin(); map_it != m_JitClrMap.end(); ++map_it)
    {
        //If the block wasn't used, go to the next one.
        JitBlockInfoMap::value_type& item = *map_it;

        if (!wcslen(item.second.movedJncFileName))
        {
            QFile::remove(QString::fromWCharArray(item.second.jncFileName));
            continue;
        }

        QString procDir = qtstrDir + "/" + QString::number(item.first.processId) + "/";

        cel_it = cel_map.find(item.first.processId);

        //If there is not a cel file for this jnc, add it
        if (cel_map.end() == cel_it)
        {
            QString new_cel;
            new_cel = procDir + QString(osFilePath::osPathSeparator) + QString::number(item.first.processId) + ".cel";

            CelWriter* pWriter = new CelWriter();

            if (NULL == pWriter)
            {
                return E_FAIL;
            }

            int bitness = 32;

            if (!m_bitnessMap[item.first.processId])
            {
                bitness = 64;
            }

            if (!pWriter->Initialize(new_cel.toStdWString().c_str(), bitness))
            {
                delete pWriter;
                return E_FAIL;
            }

            cel_it = cel_map.insert(item.first.processId, pWriter);
        }

        if (!modIdMap.contains(map_it->first))
        {
            //write the module name to the jcl file
            cel_it.value()->WriteModuleLoadFinished(modId, item.first.moduleLoadTime,
                                                    static_cast<unsigned int>(item.second.categoryName.length()),
                                                    item.second.categoryName.asCharArray());
            modIdMap[map_it->first] = modId++;
        }

        //rename from temp->jncFile to temp->translatedJncFile
        QString new_jnc = procDir + QString(osFilePath::osPathSeparator) + QString::fromWCharArray(item.second.movedJncFileName);
        QString oldFile = QString::fromWCharArray(item.second.jncFileName);
        QFile::rename(oldFile, new_jnc);

        QString classFun = QString::fromWCharArray(m_tiModMap [map_it->first].moduleName);
        cel_it.value()->ReWriteJITCompilation(modIdMap[map_it->first],
                                              funcId, classFun.section("::", 0, 0).toStdWString().c_str(),
                                              classFun.section("::", 1, 1).toStdWString().c_str(),
                                              item.second.movedJncFileName, item.first.moduleLoadTime,
                                              item.first.moduleLoadAddr,
                                              (unsigned int)m_tiModMap[item.first].moduleSize);

        if (TI_TIMETYPE_MAX != m_tiModMap [map_it->first].moduleUnloadTime)
        {
            cel_it.value()->WriteFunctionUnloadStarted(funcId++, m_tiModMap [map_it->first].moduleUnloadTime);
        }
    }

    //delete and close the CEL writer(s)
    for (cel_it = cel_map.begin(); cel_it != cel_map.end(); cel_it++)
    {
        delete cel_it.value();
    }

    cel_map.clear();

    // PreJITed Module symbol .pjs file
    PreJitModSymbolMap::iterator symbolFileIter;

    for (symbolFileIter = m_PJSMap.begin(); symbolFileIter != m_PJSMap.end(); ++symbolFileIter)
    {
        PreJitModSymbolMap::value_type& symFile = *symbolFileIter;

        if (symFile.second.bHasSample)
        {
            QString pjsFileName = QString::fromWCharArray(directory);
            pjsFileName.append(osFilePath::osPathSeparator);
            pjsFileName.append(QString::fromWCharArray(symFile.first.wString));
            pjsFileName.append(QString::fromWCharArray(L".pjs"));
            QFile::rename(QString::fromWCharArray(symFile.second.symbolFilePath), pjsFileName);
        }
    }

    return hr;
}


#ifdef CXL_SUPPORTS_INTERPRETED_CODE

//Note that the directory string should not end in a "\"
HRESULT WinTaskInfo::WriteJncFiles(/*[in]*/ const wchar_t* directory)
{
    HRESULT hr = S_OK;

    QString qtstrDir = QString::fromWCharArray(directory);

    if (qtstrDir.isEmpty())
    {
        qtstrDir = ".";    //set as local directory
    }

    JitBlockInfoMap::iterator map_it;
    JclMap jcl_map;
    JclMap::iterator jcl_it;

    for (map_it = m_JitInfoMap.begin(); map_it != m_JitInfoMap.end(); ++map_it)
    {
        //If the block wasn't used, go to the next one.
        JitBlockInfoMap::value_type& item = *map_it;

        if (!wcslen(item.second.movedJncFileName))
        {
            //delete the jit file, if not used
            QFile::remove(QString::fromWCharArray(item.second.jncFileName));
            continue;
        }

        // Make directory for Jnc/Jcl if not already exist
        QString procDir = qtstrDir + "/" + QString::number(item.first.processId) + "/";

        jcl_it = jcl_map.find(item.first.processId);

        //If there is not a jcl file for this jnc, add it
        if (jcl_map.end() == jcl_it)
        {
            QString new_jcl;
            new_jcl = procDir + QString::number(item.first.processId) + ".jcl";

            JclWriter* pWriter = new JclWriter(new_jcl.toStdWString().c_str(),
                                               item.second.categoryName.asCharArray(), item.first.processId);

            if (NULL == pWriter)
            {
                return E_FAIL;
            }

            if (!pWriter->Initialize())
            {
                delete pWriter;
                return E_FAIL;
            }

            jcl_it = jcl_map.insert(item.first.processId, pWriter);
        }

        //write the jit block to the jcl file
        JitLoadRecord element;
        element.blockStartAddr = item.first.moduleLoadAddr;
        element.blockEndAddr = element.blockStartAddr + m_tiModMap [item.first].moduleSize;
        element.loadTimestamp = item.first.moduleLoadTime;
        wcscpy(element.classFunctionName, m_tiModMap [map_it->first].moduleName);
        wcscpy(element.jncFileName, item.second.movedJncFileName);
        wcscpy(element.srcFileName, item.second.srcFileName);
        jcl_it.value()->WriteLoadRecord(&element);

        //rename from temp->jncFile to temp->translatedJncFile
        QString new_jnc = procDir + osFilePath::osPathSeparator + QString::fromWCharArray(item.second.movedJncFileName);
        QString oldFile = QString::fromWCharArray(item.second.jncFileName);
        QFile::rename(oldFile, new_jnc);

        //add unload info, if known
        if (TI_TIMETYPE_MAX != m_tiModMap [map_it->first].moduleUnloadTime)
        {
            JitUnloadRecord block;
            block.blockStartAddr = item.first.moduleLoadAddr;
            block.unloadTimestamp = m_tiModMap [map_it->first].moduleUnloadTime;
            jcl_it.value()->WriteUnloadRecord(&block);
        }
    }

    //delete and close the jcl writer(s)
    for (jcl_it = jcl_map.begin(); jcl_it != jcl_map.end(); jcl_it++)
    {
        delete jcl_it.value();
    }

    jcl_map.clear();

    //repeat algorithm for CLR jit files
    CelMap cel_map;
    CelMap::iterator cel_it;
    ModIdMap modIdMap;
    int modId = 0;
    int funcId = 0;

    for (map_it = m_JitClrMap.begin(); map_it != m_JitClrMap.end(); ++map_it)
    {
        //If the block wasn't used, go to the next one.
        JitBlockInfoMap::value_type& item = *map_it;

        if (!wcslen(item.second.movedJncFileName))
        {
            QFile::remove(QString::fromWCharArray(item.second.jncFileName));
            continue;
        }

        QString procDir = qtstrDir + "/" + QString::number(item.first.processId) + "/";

        cel_it = cel_map.find(item.first.processId);

        //If there is not a cel file for this jnc, add it
        if (cel_map.end() == cel_it)
        {
            QString new_cel;
            new_cel = procDir + osFilePath::osPathSeparator + QString::number(item.first.processId) + ".cel";

            CelWriter* pWriter = new CelWriter();

            if (NULL == pWriter)
            {
                return E_FAIL;
            }

            int bitness = 32;

            if (!m_bitnessMap[item.first.processId])
            {
                bitness = 64;
            }

            if (!pWriter->Initialize(new_cel.toStdWString().c_str(), bitness))
            {
                delete pWriter;
                return E_FAIL;
            }

            cel_it = cel_map.insert(item.first.processId, pWriter);
        }

        if (!modIdMap.contains(map_it->first))
        {
            //write the module name to the jcl file
            cel_it.value()->WriteModuleLoadFinished(modId, item.first.moduleLoadTime,
                                                    static_cast<unsigned int>(item.second.categoryName.length()),
                                                    item.second.categoryName.asCharArray());
            modIdMap[map_it->first] = modId++;
        }

        //rename from temp->jncFile to temp->translatedJncFile
        QString new_jnc = procDir + osFilePath::osPathSeparator +
                          QString::fromWCharArray(item.second.movedJncFileName);
        QString oldFile = QString::fromWCharArray(item.second.jncFileName);
        QFile::rename(oldFile, new_jnc);

        QString classFun = QString::fromWCharArray(m_tiModMap [map_it->first].moduleName);
        cel_it.value()->ReWriteJITCompilation(modIdMap[map_it->first],
                                              funcId, classFun.section("::", 0, 0).toStdWString().c_str(),
                                              classFun.section("::", 1, 1).toStdWString().c_str(),
                                              item.second.movedJncFileName, item.first.moduleLoadTime,
                                              item.first.moduleLoadAddr, m_tiModMap [item.first].moduleSize);

        if (TI_TIMETYPE_MAX != m_tiModMap [map_it->first].moduleUnloadTime)
        {
            cel_it.value()->WriteFunctionUnloadStarted(funcId++, m_tiModMap [map_it->first].moduleUnloadTime);
        }
    }

    //delete and close the CEL writer(s)
    for (cel_it = cel_map.begin(); cel_it != cel_map.end(); cel_it++)
    {
        delete cel_it.value();
    }

    cel_map.clear();

    // PreJITed Module symbol .pjs file
    for (PreJitModSymbolMap::iterator symFileIt = m_PJSMap.begin(), symFileEnd = m_PJSMap.end(); symFileIt != symFileEnd; ++symFileIt)
    {
        PreJitModSymbolMap::value_type& symFile = *symFileIt;

        if (symFile.second.bHasSample)
        {
            QString pjsFileName = QString::fromWCharArray(directory);
            pjsFileName.append(osFilePath::osPathSeparator);
            pjsFileName.append(QString::fromWCharArray(symFile.first.wString));
            pjsFileName.append(QString::fromWCharArray(L".pjs"));
            QFile::rename(QString::fromWCharArray(symFile.second.symbolFilePath), pjsFileName);
        }
    }

    return hr;
}

// key (unsigned __64) is moduleID from CLR, QString is CLR module name.
typedef gtMap<gtUInt64, QString> CLRModMap;
//Function id is key, load addr is value
typedef QMap<FunctionID, gtUInt64> FunIdToAddrMap;
//Note that the directory string should not end in a "\"
HRESULT WinTaskInfo::ReadCLRJitInformation(/* [in] */ const wchar_t* clrdirectory,
                                                      const wchar_t* sessionDir)
{
    HRESULT hr = S_OK;
    CLRModMap clrModMap;
    FunIdToAddrMap clrFunMap;
    unsigned long clrPid = 0;

    //for each process id directory in the given directory
    QDir search_dir(QString::fromWCharArray(clrdirectory));

    if (search_dir.exists())
    {
        QFileInfoList di_list = search_dir.entryInfoList(QDir::AllDirs);

        //while there are directories left,
        for (int i = 0; i < di_list.size(); i++)
        {
            QString tempPath = QString::fromWCharArray(clrdirectory)
                               + di_list.at(i).fileName();

            QString celFile = tempPath;
            celFile += osFilePath::osPathSeparator + di_list.at(i).fileName() + ".cel";

            if (!QFile::exists(celFile))
            {
                continue;
            }

            clrPid = di_list.at(i).fileName().toULong();

            if (0 == clrPid)
            {
                continue;
            }

            QString sess_pid = QString::fromWCharArray(sessionDir) + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
            QDir sess_pid_dir;
            sess_pid_dir.mkpath(sess_pid);

            // copy file from temp dir to session dir;
            CopyFilesToDestionDir(tempPath, sess_pid);
            QDir tCurrentDir(sess_pid);

            QFileInfoList pjsFile_list = tCurrentDir.entryInfoList(QStringList("*.pjs"));

            //while there are files left,
            for (int j = 0; j < pjsFile_list.size(); j++)
            {
                QString pjsFileName = pjsFile_list.at(j).fileName();

                QString pjsFilePath = sess_pid + osFilePath::osPathSeparator + pjsFileName;

                pjsFileName.remove(pjsFileName.length() - 4, 4);
                PreJitModSymbolFile tSymFile(pjsFilePath.toStdWString().c_str());
                m_PJSMap.insert(PreJitModSymbolMap::value_type(pjsFileName.toStdWString().c_str(), tSymFile));
            }

            clrModMap.clear();
            clrFunMap.clear();

            CelReader tClrReader;
            celFile = sess_pid + osFilePath::osPathSeparator + di_list.at(i).fileName() + ".cel";

            if (!tClrReader.Open(celFile.toStdWString().c_str()))
            {
                continue;
            }

            bool bitness  = tClrReader.Is32BitProcess();
            gtUInt64 jitProcessID = di_list.at(i).fileName().toULongLong();
            m_bitnessMap[jitProcessID] = bitness;

            DWORD recType = tClrReader.GetNextRecordType();
            DWORD cnt = tClrReader.GetNumRecord();

            while (recType != evInvalidCLREvent)
            {
                HRESULT hr = S_OK;

                switch (recType)
                {
                    case evAppDomainCreationFinished:
                    {
                        AppDomainRecord domainRec;
                        hr = tClrReader.GetAppDomainCreationRecord(&domainRec);
                    }
                    break;

                    case evAppDomainShutdownStarted:
                    {
                        AppDomainID domainId;
                        gtUInt64 systime;
                        hr = tClrReader.GetAppDomainShutdownRecord(&domainId, &systime);
                    }
                    break;

                    case evAssemblyLoadFinished:
                    {
                        AssemblyRecord asmRec;
                        hr = tClrReader.GetAssemblyLoadRecord(&asmRec);
                    }
                    break;

                    case evAssemblyUnloadStarted:
                    {
                        AssemblyID assemblyId;
                        gtUInt64 systime;
                        hr = tClrReader.GetAssemblyUnloadRecord(&assemblyId, &systime);
                    }
                    break;

                    case evModuleLoadFinished:
                    {
                        ModuleRecord modRec;
                        hr = tClrReader.GetModuleLoadRecord(&modRec);

                        if (S_OK == hr)
                        {
                            clrModMap[modRec.modId] = QString::fromWCharArray(modRec.modName);
                        }
                    }
                    break;

                    case evModuleUnloadStarted:
                    {
                        ModuleID modId;
                        gtUInt64 systime;
                        hr = tClrReader.GetModuleUnloadRecord(&modId, &systime);
                    }
                    break;

                    case evModuleAttachedToAssembly:
                    {
                        ModuleID moduleId;
                        AssemblyID assemblyId;
                        gtUInt64 assemblyLoadAddr = 0;
                        wchar_t assemblyPath[OS_MAX_PATH];
                        hr = tClrReader.GetModuleAttachedToAssemblyRec(&moduleId, &assemblyId,
                                                                       &assemblyLoadAddr, assemblyPath, OS_MAX_PATH);

                        if (hr == S_OK && assemblyLoadAddr)
                        {

                            // Due to OS had truncated assembly for some ngened assembly;
                            // need to update Ngened assembly full path.
                            // Lei 05/04/2010
                            ModuleMap::iterator j;
                            j = m_tiModMap.lower_bound(ModuleKey(clrPid, assemblyLoadAddr, TI_TIMETYPE_MAX));

                            for (; j != m_tiModMap.end(); ++j)
                            {
                                ModuleMap::value_type& item = *j;

                                if (item.first.processId != clrPid)
                                {
                                    break;
                                }

                                if (item.first.moduleLoadAddr != assemblyLoadAddr)
                                {
                                    break;
                                }

                                wcscpy_s(item.second.moduleName, OS_MAX_PATH, assemblyPath);
                                break;
                            }
                        }

                    }
                    break;

                    case evClassLoadFinished:
                    {
                        ClassRecord classRec;
                        hr = tClrReader.GetClassLoadRecord(&classRec);
                    }
                    break;

                    case evClassUnloadStarted:
                    {
                        ClassID classId;
                        gtUInt64 systime;
                        hr = tClrReader.GetClassUnloadRec(&classId, &systime);
                    }
                    break;

                    case evJITCompilationFinished:
                    {
                        FunctionRecord funcRec;
                        funcRec.loadTime = 0;
                        funcRec.jitLoadAddr = 0;
                        hr = tClrReader.GetJITCompilationFinished(&funcRec);

                        if (S_OK == hr)
                        {
                            gtUInt64 tModuleId = funcRec.modId;
                            QString tModuleName = clrModMap[tModuleId];
                            QString tClassFunc;

                            gtUInt64 tLoadAddress = funcRec.jitLoadAddr;
                            clrFunMap[funcRec.funcId] = tLoadAddress;
                            funcRec.loadTime = CalculateDeltaTick(funcRec.loadTime);
                            ModuleKey t_modKey(clrPid, tLoadAddress, funcRec.loadTime);

                            tClassFunc = QString::fromWCharArray(funcRec.className) +
                                         "::" + QString::fromWCharArray(funcRec.funcName);

                            ModuleValue t_modValue(0, static_cast<gtUInt64>(funcRec.codeSize),
                                                   TI_TIMETYPE_MAX, tClassFunc.toStdWString().c_str(), evManaged);
                            m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
                            m_JitModCount++;

                            tModuleName.append(".jit");
                            QString repPath = QString::fromWCharArray(funcRec.jncFileName);
                            repPath.replace(tempPath, sess_pid);
                            wcscpy(funcRec.jncFileName, repPath.toStdWString().c_str());

                            JitBlockValue tJitBlockValue(tModuleName.toStdWString().c_str(), funcRec.jncFileName, L"UnknownJITSource");
                            m_JitClrMap.insert(JitBlockInfoMap::value_type(t_modKey, tJitBlockValue));
                        }
                    }
                    break;

                    case evFunctionUnloadStarted:
                    {
                        FunctionID funcId;
                        gtUInt64 systime;
                        hr = tClrReader.GetFunctionUnloadStarted(&funcId, &systime);
                        ModuleMap::iterator it;

                        for (it = m_tiModMap.begin(); it != m_tiModMap.end(); ++it)
                        {
                            ModuleMap::value_type& item1 = *it;

                            //note that item.first is the key, and item.second is the value
                            if (item1.first.processId != clrPid)
                            {
                                continue;
                            }

                            if (item1.first.moduleLoadAddr != clrFunMap[funcId])
                            {
                                continue;
                            }

                            if (item1.first.moduleLoadTime >= systime)
                            {
                                continue;
                            }

                            if (TI_TIMETYPE_MAX != item1.second.moduleUnloadTime)
                            {
                                continue;
                            }

                            item1.second.moduleUnloadTime = CalculateDeltaTick(systime);
                        }
                    }
                }

                if (S_OK != hr)
                {
                    break;
                }

                recType = tClrReader.GetNextRecordType();
            }

            tClrReader.Close();
        }
    }

    clrModMap.clear();
    clrFunMap.clear();

    return hr;
}


//Note that the directory string should not end in a "\"
HRESULT WinTaskInfo::ReadOldJitInfo(/* [in] */ const wchar_t* directory)
{
    HRESULT hr = JitTaskInfo::ReadOldJitInfo(directory);

    QString jitDir = QString::fromWCharArray(directory);
    QDir search_dir(jitDir);
    QFileInfoList pjsFile_list = search_dir.entryInfoList(QStringList("*.pjs"));

    //while there are files left,
    for (int j = 0; j < pjsFile_list.size(); j++)
    {
        QString pjsFileName = pjsFile_list.at(j).fileName();

        //Is this right?
        QString pjsFilePath = jitDir + osFilePath::osPathSeparator + pjsFileName;

        pjsFileName.remove(pjsFileName.length() - 4, 4);
        PreJitModSymbolFile tSymFile(pjsFilePath.toStdWString().c_str());
        m_PJSMap.insert(PreJitModSymbolMap::value_type(pjsFileName.toStdWString().c_str(), tSymFile));
    }

    QFileInfoList di_list = search_dir.entryInfoList(QStringList("*.cel"));
    CLRModMap clrModMap;
    FunIdToAddrMap clrFunMap;

    //while there are cel files left,
    for (int i = 0; i < di_list.size(); i++)
    {
        unsigned long tPid = di_list.at(i).fileName().section(".", 0, 0).toULong();

        if (0 == tPid)
        {
            continue;
        }

        //read the cel file
        QString cel_file = QString::fromWCharArray(directory) + osFilePath::osPathSeparator + di_list.at(i).fileName();
        CelReader tClrReader;

        if (!tClrReader.Open(cel_file.toStdWString().c_str()))
        {
            continue;
        }

        bool bitness  = tClrReader.Is32BitProcess();
        m_bitnessMap[tPid] = bitness;

        DWORD recType = tClrReader.GetNextRecordType();
        DWORD cnt = tClrReader.GetNumRecord();

        while (recType != evInvalidCLREvent)
        {
            HRESULT hr = S_OK;

            switch (recType)
            {
                case evModuleLoadFinished:
                {
                    ModuleRecord modRec;
                    hr = tClrReader.GetModuleLoadRecord(&modRec);

                    if (S_OK == hr)
                    {
                        clrModMap[modRec.modId] = QString::fromWCharArray(modRec.modName);
                        clrModMap[modRec.modId].remove(".jit");
                    }
                }
                break;

                case evJITCompilationFinished:
                {
                    FunctionRecord funcRec;
                    funcRec.loadTime = 0;
                    funcRec.jitLoadAddr = 0;
                    hr = tClrReader.GetJITCompilationFinished(&funcRec);

                    if (S_OK == hr)
                    {
                        gtUInt64 tModuleId = funcRec.modId;
                        QString tModuleName = clrModMap[tModuleId];
                        QString tClassFunc;

                        gtUInt64 tLoadAddress = funcRec.jitLoadAddr;
                        clrFunMap[funcRec.funcId] = tLoadAddress;
                        ModuleKey t_modKey(tPid, tLoadAddress, funcRec.loadTime);

                        tClassFunc = QString::fromWCharArray(funcRec.className) +
                                     "::" + QString::fromWCharArray(funcRec.funcName);

                        ModuleValue t_modValue(0, static_cast<gtUInt64>(funcRec.codeSize),
                                               TI_TIMETYPE_MAX, tClassFunc.toStdWString().c_str(), evManaged);
                        t_modValue.bNameConverted = true;
                        m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
                        m_JitModCount++;

                        tModuleName += ".jit";
                        JitBlockValue tJitBlockValue(tModuleName.toStdWString().c_str(), funcRec.jncFileName, L"UnknownJITSource");
                        wcscpy_s(tJitBlockValue.movedJncFileName, OS_MAX_PATH, funcRec.jncFileName);
                        wcscpy_s(tJitBlockValue.jncFileName, directory);
                        const wchar_t pathSeparator[] = { osFilePath::osPathSeparator, '\0' };
                        wcscat_s(tJitBlockValue.jncFileName, pathSeparator);
                        wcscat_s(tJitBlockValue.jncFileName, tJitBlockValue.movedJncFileName);
                        m_JitClrMap.insert(JitBlockInfoMap::value_type(t_modKey, tJitBlockValue));
                    }
                }
                break;

                case evFunctionUnloadStarted:
                {
                    FunctionID funcId;
                    gtUInt64 systime;
                    hr = tClrReader.GetFunctionUnloadStarted(&funcId, &systime);
                    ModuleMap::iterator it;

                    for (it = m_tiModMap.begin(); it != m_tiModMap.end(); ++it)
                    {
                        ModuleMap::value_type& item1 = *it;

                        //note that item.first is the key, and item.second is the value
                        if (item1.first.processId != tPid)
                        {
                            continue;
                        }

                        if (item1.first.moduleLoadAddr != clrFunMap[funcId])
                        {
                            continue;
                        }

                        if (item1.first.moduleLoadTime >= systime)
                        {
                            continue;
                        }

                        if (TI_TIMETYPE_MAX != item1.second.moduleUnloadTime)
                        {
                            continue;
                        }

                        item1.second.moduleUnloadTime = systime;
                    }
                }
            }

            if (S_OK != hr)
            {
                break;
            }

            recType = tClrReader.GetNextRecordType();
        }

        tClrReader.Close();
    }

    return hr;
}

#endif // CXL_SUPPORTS_INTERPRETED_CODE


gtUInt64 WinTaskInfo::GetModuleSize(const wchar_t* pModuleName) const
{
    gtUInt64 imageSize = 0ULL;
    PeFile exe(pModuleName);

    if (exe.Open())
    {
        imageSize = exe.GetImageSize();
    }

    return imageSize;
}

void WinTaskInfo::AddLoadModules(gtUInt64 processId)
{
    if (m_interestingPidMap.end() == m_interestingPidMap.find(processId))
    {
        osCriticalSectionLocker lock(m_TIMutexModule);

        if (m_interestingPidMap.end() == m_interestingPidMap.find(processId))
        {
            // Add this pid in m_tiModMap from m_allModulesMap
            auto modIter = m_allModulesMap.upper_bound(ModuleKey(processId - 1, 0, 0));

            for (; modIter != m_allModulesMap.end(); ++modIter)
            {
                const ModuleKey& modKey = modIter->first;

                // Different process.
                if (modKey.processId == processId)
                {
                    ModuleValue& modValue = modIter->second;

                    if (0 == modValue.instanceId)
                    {
                        modValue.instanceId = m_nextModInstanceId++;
                    }

                    m_tiModMap.insert(ModuleMap::value_type(modKey, modValue));

                    std::wstring modName(modValue.moduleName);
                    auto iter = m_moduleIdMap.find(modName);

                    if (m_moduleIdMap.end() == iter)
                    {
                        m_moduleIdMap.emplace(modName, AtomicAdd(m_nextModuleId, 1));
                    }
                }
            }

            m_interestingPidMap.insert(PidMap::value_type(processId, processId));
        }
    }

    return;
}

void WinTaskInfo::SetExecutableFilesSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath)
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

void WinTaskInfo::LoadProcessExecutableFiles(gtUInt64 processId, osSynchronizedQueue<gtString>& statusesQueue)
{
    ProcessMap::iterator itProc = m_tiProcMap.find(processId);

    if (m_tiProcMap.end() != itProc)
    {
        if (!itProc->second.bLoadedPeFiles)
        {
            osCriticalSectionLocker lock(m_TIMutex);

            // Note: since ModuleKey is sorted by PID (ascending), module address (descending) and load time (descending),
            // Searching (processId - 1, 0, 0) will bring up the first module in process of pid
            ModuleMap::iterator itMod = m_tiModMap.upper_bound(ModuleKey(processId - 1, 0, 0));

            for (ModuleMap::iterator itModEnd = m_tiModMap.end(); itMod != itModEnd; ++itMod)
            {
                const ModuleKey& modKey = itMod->first;

                // Different process.
                if (modKey.processId != processId)
                {
                    break;
                }

                ModuleValue& modValue = itMod->second;

                if (!modValue.bLoadedPeFile)
                {
                    if (NULL != modValue.pPeFile)
                    {
                        delete modValue.pPeFile;
                    }

                    if (!modValue.bNameConverted)
                    {
                        wchar_t tStr[OS_MAX_PATH];
                        wcsncpy(tStr, modValue.moduleName, OS_MAX_PATH);
                        ConvertName(tStr, processId);

                        if (evManaged != modValue.moduleType)
                        {
                            wcsncpy(modValue.moduleName, tStr, OS_MAX_PATH);
                            modValue.bNameConverted = true;
                        }
                    }

                    PeFile* pPeFile = new PeFile(modValue.moduleName);

                    if (NULL != pPeFile)
                    {
                        if (pPeFile->Open(modKey.moduleLoadAddr))
                        {
                            // Update the progress bar with the action of loading the symbols for this module
                            gtString progressCaption(L"Locating and loading symbols for ");
                            progressCaption.append(gtString(modValue.moduleName));
                            statusesQueue.push(progressCaption);

                            pPeFile->InitializeSymbolEngine(m_pSearchPath, m_pServerList, m_pCachePath);
                        }
                        else
                        {
                            delete pPeFile;
                            pPeFile = NULL;
                        }
                    }

                    modValue.pPeFile = pPeFile;

                    // TODO: This is being set even if pPeFile::Open fails, which is not correct
                    // This should be set only for evPEModule modules
                    modValue.bLoadedPeFile = true;
                }
            }

            itProc->second.bLoadedPeFiles = true;
        }
    }
}

PeFile* WinTaskInfo::FindExecutableFile(gtUInt64 processId, gtUInt64 addr) const
{
    PeFile* pPeFile = NULL;

    if (m_lpMinAppAddr < addr && addr < m_lpMaxAppAddr)
    {
        // this is user space, check module map.
        ModuleMap::const_iterator it = m_tiModMap.lower_bound(ModuleKey(processId, addr, TI_TIMETYPE_MAX));

        for (ModuleMap::const_iterator itEnd = m_tiModMap.end(); it != itEnd; ++it)
        {
            const ModuleKey& modKey = it->first;

            // different process
            if (modKey.processId != processId)
            {
                break;
            }

            const ModuleValue& modValue = it->second;

            // since the module map is sorted by the process id, module address and time.
            // if module load address is greater than sample address, we don't need
            // go farther.
            if ((modKey.moduleLoadAddr + modValue.moduleSize) < addr)
            {
                break;
            }

            if (evPEModule == modValue.moduleType)
            {
                pPeFile = modValue.pPeFile;
                break;
            }
        }
    }
    else if (m_bLoadKernelPeFiles)
    {
        for (KernelModMap::const_iterator it = m_tiKeModMap.lower_bound(KernelModKey(addr, TI_TIMETYPE_MAX)), itEnd = m_tiKeModMap.end(); it != itEnd; ++it)
        {
            const KernelModValue& kernelModValue = it->second;

            // Since the module with greater start address will stay on the bottom of map.
            // if module end address is less than sample address, we don't need to go farther
            //
            if (kernelModValue.keModEndAddr < addr)
            {
                break;
            }

            if (it->first.keModLoadAddr <= addr && addr < kernelModValue.keModEndAddr)
            {
                pPeFile = kernelModValue.pPeFile;
                break;
            }
        }
    }

    return pPeFile;
}

unsigned int WinTaskInfo::ForeachExecutableFile(gtUInt64 processId, bool kernel, void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext) const
{
    unsigned int modulesCount = 0U;

    for (ModuleMap::const_iterator it = m_tiModMap.upper_bound(ModuleKey(processId - 1, 0, 0)), itEnd = m_tiModMap.end(); it != itEnd; ++it)
    {
        const ModuleKey& modKey = it->first;

        // different process
        if (modKey.processId != processId)
        {
            break;
        }

        const ModuleValue& modValue = it->second;

        if (NULL != modValue.pPeFile)
        {
            pfnProcessModule(*modValue.pPeFile, pContext);
            modulesCount++;
        }
    }

    if (kernel && m_bLoadKernelPeFiles)
    {
        for (KernelModMap::const_iterator it = m_tiKeModMap.begin(), itEnd = m_tiKeModMap.end(); it != itEnd; ++it)
        {
            const KernelModValue& kernelModValue = it->second;

            if (NULL != kernelModValue.pPeFile)
            {
                pfnProcessModule(*kernelModValue.pPeFile, pContext);
                modulesCount++;
            }
        }
    }

    return modulesCount;
}
