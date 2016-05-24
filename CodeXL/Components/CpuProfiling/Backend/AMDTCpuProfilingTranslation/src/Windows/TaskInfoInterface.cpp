//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TaskInfoInterface.cpp
/// \brief Implementation file for Windows Task Info exported APIs.
///
//==================================================================================

#include <AMDTCpuProfilingTranslation/inc/Windows/TaskInfoInterface.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include "WinTaskInfo.h"

// Suppress Qt header warnings
#pragma warning(push)
#pragma warning(disable : 4800)
#include <QDir>
#pragma warning(pop)

// TODO: Finalize the value of this key
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #define CXL_INSTALL_DIR_REG_KEY     L"SOFTWARE\\Wow6432Node\\AMD\\CodeXL\\Key\0"
#else
    #define CXL_INSTALL_DIR_REG_KEY     L"SOFTWARE\\AMD\\CodeXL\\Key\0"
#endif

#define AMDTPROCESSENUM32_NAME  L"\\CXLProcessEnum" GDT_DEBUG_SUFFIX_W GDT_BUILD_SUFFIX_W
#define AMDTPROCESSENUM64_NAME  L"\\CXLProcessEnum-x64" GDT_DEBUG_SUFFIX_W GDT_BUILD_SUFFIX_W

// global task info variable
WinTaskInfo g_TaskInfo;


////////////////////////////////////////////////////////////////////////////////////
// fnStartCapture()
//  Start task info capturing.
//
HRESULT fnStartCapture(gtUInt64 startCount, const wchar_t* binDirectory)
{
    HRESULT hr = g_TaskInfo.StartCapture(startCount, binDirectory);

    return hr;
}

//////////////////////////////////////////////////////////////////////////////////////
// fnStopCapture()
//  Stop the task info capturing.
//
HRESULT fnStopCapture(bool bConvertFlag,
                      wchar_t* tempTiFileName)
{
    HRESULT hr = g_TaskInfo.StopCapture(bConvertFlag, tempTiFileName);

    return hr;
}


HRESULT fnReadJitInformation(const wchar_t* sessionDir)
{
    wchar_t pidDirName[OS_MAX_PATH];
    GetTempPath(OS_MAX_PATH, pidDirName);
    HRESULT hr = g_TaskInfo.ReadJavaJitInformation(pidDirName, sessionDir);

    return hr;
}


HRESULT fnReadOldJitInformation(/*[in]*/ const wchar_t* directory)
{
    HRESULT hr = g_TaskInfo.ReadOldJitInfo(directory);
    return hr;
}


HRESULT fnReadCLRJitInformation(const wchar_t* sessionDir)
{
    wchar_t pidDirName[OS_MAX_PATH];
    memset(pidDirName, 0, sizeof(pidDirName));
    GetTempPathW(OS_MAX_PATH, pidDirName);

    HRESULT hr = g_TaskInfo.ReadCLRJitInformation(pidDirName, sessionDir);

    return hr;
}

HRESULT fnWriteJncFiles(/*[in]*/ const wchar_t* directory)
{
    HRESULT hr =  S_OK;

    HRESULT hr_java = g_TaskInfo.WriteJavaJncFiles(directory);
    HRESULT hr_clr = g_TaskInfo.WriteCLRJncFiles(directory);

    // Probably, we can have seperate APIs for Java and CLR
    // For now, returning E_FAIL if either function fail
    if (hr_java != S_OK || hr_clr != S_OK)
    {
        hr = E_FAIL;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////////////
// fnWriteModuleInfoFile(char *filename)
//  Write process, module, kernel module info into task info file.
//
//  Param: char | *filename  | task info file name.
//
HRESULT fnWriteModuleInfoFile(/* [in] */ const wchar_t* filename)
{
    HRESULT hr = g_TaskInfo.WriteModuleInfoFile(filename);

    return hr;
}


////////////////////////////////////////////////////////////////////////////////////////
// fnReadModuleInfoFile(char *filename)
//  Read process, module and kernel module info from task info file, and build up maps.
//
//  Param: [in] char |*filename | task info file name
//
HRESULT fnReadModuleInfoFile(/* [in] */ const wchar_t* filename)
{
    HRESULT hr = g_TaskInfo.ReadModuleInfoFile(filename);

    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////
// fnCleanupMaps()
//  Clean up maps
// Note: Once maps are cleaned up, all querying will return S_FALSE until maps are rebuilt.
void fnCleanupMaps()
{
    g_TaskInfo.CleanupMaps();
}

//////////////////////////////////////////////////////////////////////////////////////
// fnGetErrorMsg()
//  Get the last error message from Task Info.
//
const gtASCIIString& fnGetErrorMsg()
{
    return g_TaskInfo.GetErrorMsg();
}

//////////////////////////////////////////////////////////////////////////////////////
// fnGetProcessNum(unsigned *pProcNum)
//  Get number of processes in the process map.
//
//  Param: [out] unsigned * | pProcNum | number of processes
//
HRESULT fnGetProcessNum(/* [out] */ unsigned* procNum)
{
    HRESULT hr = g_TaskInfo.GetProcessesNum(procNum);

    return hr;
}

//////////////////////////////////////////////////////////////////////////
// fnGetAllProcesses(ProcQueryInfo *pProcQueryInfo, unsigned procNum)
//  Get all processes info.
//
//  Note: Caller allocates the space for ProcQueryInfo struct.
//
//  Param: [out] ProcQueryInfo | *pProcQueryInfo | reference pointer of process info.
//  Param: [in]  unsigned      | procNum         | number of processes
//
HRESULT fnGetAllProcesses(
    /* [out] */ ProcQueryInfo* pProcQueryInfo,
    /* [in] */  unsigned procNum)
{
    HRESULT hr = S_OK;

    if (NULL == pProcQueryInfo)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetAllProcesses(pProcQueryInfo, procNum);

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// fnFindProcessName(gtUInt64 processID, char* processName, unsigned sizeofname)
//  Find the process name for a given process id.
//
//  Note: Caller allocates the space for process name.
//
//  Param: [in]  gtUInt64   | processID   | process id
//  Param: [out] char*    | processName | process name
//  Param: [in]  unsigned | sizeofname  | length of the process name string
//
HRESULT fnFindProcessName(
    /* [in] */  gtUInt64 processID,
    /* [out] */ wchar_t* processName,
    /* [in] */  unsigned sizeofname)
{
    HRESULT hr = S_OK;

    if (NULL == processName)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.FindProcessName(processID, processName, sizeofname);

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// fnGetModNumInProc(gtUInt64 processID, unsigned *pNumOfMod)
//  Get number of modules in a process.
//
//  Param: [in]  gtUInt64   | processID | process ID
//  Param: [out] unsigned | *pNumOfMod| number of modules in a given process.
//
HRESULT fnGetModNumInProc(
    /* [in] */  gtUInt64 processID,
    /* [out] */ unsigned* pNumOfMod)
{
    HRESULT hr = S_OK;

    if (NULL == pNumOfMod)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetModNumInProc(processID, pNumOfMod);

    return hr;
}

//////////////////////////////////////////////////////////////////////////////////
// fnGetModuleInfoByIndex( )
//  Get the module info for the nth module in the process.
//
//  Param: [in] gtUInt64  | process ID | process id
//  Param: [in] unsigned| modIndex   | module index in the process
//  Param: [out] gtUInt64 | *pModuleStartAddr | module start address
//  Param: [out] gtUInt64 | *pModuleSize | module image size
//  Param: [out] char*  | pModulename | module full name
//  Param: [in] unsigned| namesize | length of the module name string.
//
HRESULT fnGetModuleInfoByIndex(
    /* [in] */  gtUInt64 processID,
    /* [in] */  unsigned modIndex,
    /* [out] */ gtUInt64* pModuleStartAddr,
    /* [out] */ gtUInt64* pModulesize,
    /* [out] */ wchar_t* pModulename,
    /* [in] */  unsigned namesize)
{
    HRESULT hr = S_OK;

    if (NULL == pModulename || NULL == pModulesize || NULL == pModuleStartAddr)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetModuleInfoByIndex(processID, modIndex, pModuleStartAddr, pModulesize, pModulename, namesize);

    return hr;
}

//////////////////////////////////////////////////////////////////////////////////////
// fnGetModuleInfo( )
//  Get module info for a given sample record.
//
//  Param: [in]     gtUInt64      processID,          | process id
//  Param: [in]     unsigned    CSvalue,            | CS value
//  Param: [in]     gtUInt64      sampleAddr,         | sample address
//  Param: [in]     unsigned    cpuIndex,           | cpu number
//  Param: [in]     gtUInt64      timestamp,          | sample time stamp
//  Param: [out]    gtUInt64      *pModuleStartAddr,  | module start address
//  Param: [out]    gtUInt64      *pModulesize,       | module image size
//  Param: [out]    char        *pModulename,       | module full name
//  Param: [in]     unsigned    namesize            | length of module name string
//
HRESULT fnGetModuleInfo(TiModuleInfo* info)
{
    HRESULT hr = S_OK;

    if (NULL == info->pModulename)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetModuleInfo(info);

    return hr;
}

HRESULT fnGetModuleInstanceId(gtUInt32 processId, gtUInt64 sampleAddr, gtUInt64 deltaTick, gtUInt32& modInstId)
{
    HRESULT hr = S_OK;

    hr = g_TaskInfo.GetModuleInstanceId(processId, sampleAddr, deltaTick, modInstId);

    return hr;
}

HRESULT fnGetModuleInfoByInstanceId(gtUInt32 instanceId, LoadModuleInfo* pModInfo)
{
    HRESULT hr = S_OK;

    if (NULL == pModInfo)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetLoadModuleInfoByInstanceId(instanceId, pModInfo);

    return hr;
}


HRESULT fnGetProcessThreadList(gtVector<std::tuple<gtUInt32, gtUInt32>>& info)
{
    HRESULT hr = g_TaskInfo.GetProcessThreadList(info);

    return hr;
}

///////////////////////////////////////////////////////////////////////////
// fnGetKernelModNum(unsigned *pKeModNum)
//  Get number of kernel modules.
//
//  Param: [out] unsigned | *pKeModNum | reference pointer of kernel module number
//
HRESULT fnGetKernelModNum(
    /* [out] */ unsigned* pNumOfKeMod)
{
    HRESULT hr = S_OK;

    if (NULL == pNumOfKeMod)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetKernelModNum(pNumOfKeMod);

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// fnGetAllKeMod(KeModQueryInfo *pKeMods, unsigned keModNum)
//  Get all kernel module info.
//
//  Param: [out] KeModQueryInfo | *pKeMods | reference pointer of kernel module info structure.
//  Param: [in]  unsigned       | keModNum | number of kernel modules
//
HRESULT fnGetAllKeMod(
    /* [out] */ KeModQueryInfo* pKeMods,
    /* [in] */  unsigned keModNum)
{
    HRESULT hr = S_OK;

    if (NULL == pKeMods)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetAllKeMod(pKeMods, keModNum);

    return hr;
}


HRESULT fnGetCpuProfilingDriversMaxCount(
    /* [out] */ unsigned* pNumOfKeMod)
{
    HRESULT hr = S_OK;

    if (NULL == pNumOfKeMod)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetCpuProfilingDriversMaxCount(pNumOfKeMod);

    return hr;
}


HRESULT fnGetCpuProfilingDrivers(
    /* [out] */ KeModQueryInfo* pKeMods,
    /* [in,out] */  unsigned& keModNum)
{
    HRESULT hr = S_OK;

    if (NULL == pKeMods)
    {
        return S_FALSE;
    }

    hr = g_TaskInfo.GetCpuProfilingDrivers(pKeMods, keModNum);

    return hr;
}

////////////////////////////////////////////////////////////////////////////////////////
// fnGetNumThreadInProcess()
//  Get number of thread in the process for a given process ID.
//      Note: The whole thread map only contains thread info for process that is launched
//              after profiling starts.
//
//  Param: [in] gtUInt64 |processID | process ID
//
unsigned int fnGetNumThreadInProcess(/* [in] */ gtUInt64 processID)
{
    return g_TaskInfo.GetNumThreadInProcess(processID);
}


////////////////////////////////////////////////////////////////////////////////////////
// fnGetThreadInfoInProcess()
//  Get thread info for a given process ID.
//
//  Param: [in] gtUInt64 |processID | process ID
//  Param: [in] UINT   |entriesOfThreadInfo | count of entries in thread info array
//  Param: [out] TI_THREAD_INFO* | pThreadInfoArray | thread info array
//
HRESULT fnGetThreadInfoInProcess(
    /* [in] */ gtUInt64 processID,
    /* [in] */ unsigned int sizeOfInfoArray,
    /* [out] */ TI_THREAD_INFO* pThreadInfoArray)
{
    HRESULT hr = S_FALSE;

    if (pThreadInfoArray)
    {
        hr = g_TaskInfo.GetThreadInfoInProcess(processID, sizeOfInfoArray, pThreadInfoArray);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////////////////////
// fnGetJITProcessBitness()
//  Get JIT process bitness for a given process ID.
//
//  Param: [in] gtUInt64 |processID | process ID
//  Ret: return true if it's 32bit process, otherwise false.
bool fnIsJITProcess32Bit(gtUInt64 jitProcID)
{
    return g_TaskInfo.IsJitProcess32Bit(jitProcID);
}
//This will return the FILETIME for a given timestamp
FILETIME fnSynchronize(FILETIME start, gtUInt64 deltaTick, int core,
                       unsigned int extraMs)
{
    return g_TaskInfo.Synchronize(start, deltaTick, core, extraMs);
}


//We clear out all jit directories where the process id is not currently in
//  use.
void helpClearDir(const wchar_t* pDirName, QList<unsigned long>* pPidList)
{
    //the jit dir may contain a bunch of directories with the pid as the name
    QDir searchDir(QString::fromWCharArray(pDirName));

    if (searchDir.exists())
    {
        QFileInfoList di_list = searchDir.entryInfoList(QDir::AllDirs);

        //while directories are left
        for (int i = 0; i < di_list.size(); i++)
        {
            unsigned long pid = di_list.at(i).fileName().toULong();

            //check if pid is on the snapshot list, and therefore running
            if ((0 != pid) && (-1 == pPidList->indexOf(pid)))
            {
                QString pidDir(QString::fromWCharArray(pDirName));
                pidDir += QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();

                //if it's not running, delete the dir
                QDir dir(pidDir);

                if (dir.exists())
                {
                    QFileInfoList fi_list = dir.entryInfoList();

                    for (int j = 0; j < fi_list.size(); j++)
                    {
                        dir.remove(fi_list.at(j).fileName());
                    }

                    dir.rmdir(pidDir);
                }
            }
        }

        di_list = searchDir.entryInfoList(QStringList("*.css"), QDir::Files);

        for (int j = 0; j < di_list.size(); j++)
        {
            searchDir.remove(di_list.at(j).fileName());
        }
    }
}

//This functions cleans up data about jitted instructions from previous runs,
//  so as not to fill up the user's computer.
HRESULT fnCleanupJitInformation()
{
    DWORD* procArray = NULL;
    uint procCount = 0;
    uint i;
    QList <unsigned long> pidList;

    DWORD modNum = 1024;
    bool bRepeat = true;

    //Allocate space for processes
    //since we don't know the #s, if it's not enough,
    //  we'll re-allocate later
    while (bRepeat)
    {
        procArray = (DWORD*)malloc(sizeof(DWORD) * modNum);

        if (NULL == procArray)
        {
            break;
        }

        DWORD cbNeeded;

        if (!EnumProcesses(procArray, (sizeof(DWORD) * modNum), &cbNeeded))
        {
            free(procArray);
            procArray = NULL;
            break;
        }

        procCount = cbNeeded / sizeof(DWORD);

        if (procCount > modNum)
        {
            free(procArray);
            procArray = NULL;
            modNum = procCount;
            procCount = 0;
        }
        else
        {
            bRepeat = false;
        }

    }

    //walk the snapshow, for each proc, save the id
    for (i = 0; i < procCount; i++)
    {
        pidList.append(procArray[i]);
    }

    //cleanup snapshot
    if (procArray)
    {
        free(procArray);
        procArray = NULL;
    }

    wchar_t pidDirName[OS_MAX_PATH + 1];
    pidDirName[0] = L'\0';
    GetTempPathW(OS_MAX_PATH, pidDirName);

    helpClearDir(pidDirName, &pidList);
    pidList.clear();
    return 0;
}


void fnClearCSSModules(CSSMODMAP* pModMap)
{
    if (!pModMap)
    {
        return;
    }

    pModMap->clear();
}


void fnGetCSSModules(unsigned int pid, CSSMODMAP* pModMap, BOOL* pIs32)
{
    if (!pModMap)
    {
        return;
    }

    BOOL isSys64;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    isSys64 = true;
#else
    IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

    pModMap->clear();

    //snapshot list of modules start addresses and sizes for pid's
    DWORD modNum = 1024;
    HMODULE* phModArray = (HMODULE*) malloc(sizeof(HMODULE) * modNum);

    if (NULL == phModArray)
    {
        return ;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, (BOOL) FALSE, pid);

    //If this failed, the number of modules passed in will be 0, so it's ok
    if (NULL != hProcess)
    {
        if (isSys64)
        {
            IsWow64Process(hProcess, pIs32);
        }
        else
        {
            *pIs32 = true;
        }

        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, phModArray, (sizeof(HMODULE) * modNum),
                               &cbNeeded))
        {
            DWORD cModules = cbNeeded / sizeof(HMODULE);

            for (unsigned int j = 0; j < cModules && j < modNum; j++)
            {
                MODULEINFO modinfo;

                if (GetModuleInformation(hProcess, phModArray[j], &modinfo,
                                         sizeof(MODULEINFO)))
                {
                    pModMap->insert(CSSMODMAP::value_type(
                                        (gtUInt64)modinfo.lpBaseOfDll, modinfo.SizeOfImage));
                }
            }
        }

        CloseHandle(hProcess);
    }

    if (phModArray != NULL)
    {
        free(phModArray);
        phModArray = NULL;
    }

    //Don't need the rest on a 32-bit system
    if (!isSys64)
    {
        return;
    }

    wchar_t tempPath[OS_MAX_PATH + 1];
    wchar_t snapShotFile[OS_MAX_PATH + 1];
    wchar_t pidStr[OS_MAX_PATH + 1];

    ::GetTempPathW(OS_MAX_PATH, tempPath);
    ::GetTempFileNameW(tempPath, L"64S", 0, snapShotFile);

    wchar_t exePath[OS_MAX_PATH + 1];
    //Get the install path to /bin from the registry

    exePath[0] = '\0';
    wchar_t BinFileName[OS_MAX_PATH + 1];
    BinFileName[0] = L'\0';

    // find the current executable file
    if (GetModuleFileNameW(NULL, BinFileName, sizeof(BinFileName)))
    {
        wchar_t drive[_MAX_DRIVE];
        wchar_t dir[_MAX_DIR];

        // split the current executable file name into drive, path etc.
        _wsplitpath(BinFileName, drive, dir, NULL, NULL);
        wcscpy(exePath, L"\"");
        wcscat(exePath, drive);
        wcscat(exePath, dir);
    }

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    // this is to correctly enumerate the 32-bit processes for the 64-bit application
    wcscat_s(exePath, AMDTPROCESSENUM32_NAME L".exe\" ");
#else
    // this is correctly enumerate the 64-bit processes for the 32-bit application
    wcscat_s(exePath, AMDTPROCESSENUM64_NAME L".exe\" ");
#endif

    wcscat_s(exePath, snapShotFile);
    swprintf(pidStr, sizeof(pidStr) - 1, L" %u", pid);
    wcscat_s(exePath, pidStr);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    //launch the helper process to enum modules and dump
    // the info to a file
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, exePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        return;
    }

    //wait for helper process to terminate
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitcode = 0;
    GetExitCodeProcess(pi.hProcess, &exitcode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitcode != 0)
    {
        return;
    }

    DWORD usermodCount = 0;
    FILE* pTiFile;

    if (0 != _wfopen_s(&pTiFile, snapShotFile, L"rb"))
    {
        return;
    }

    //module number is stored at the beginning
    fread(&usermodCount, sizeof(DWORD), 1, pTiFile);
    gtUInt64 temp64, loadAddr;
    DWORD dtemp, modSize;

    //skip stuff
    fread(&dtemp, sizeof(DWORD), 1, pTiFile);
    fread(&temp64, sizeof(gtUInt64), 1, pTiFile);
    fread(&temp64, sizeof(gtUInt64), 1, pTiFile);

    for (DWORD i = 0; i < usermodCount; i++)
    {
        //read process id; skip
        fread(&temp64, sizeof(gtUInt64), 1, pTiFile);

        //read WOW64 (32-bit app)
        fread(&temp64, sizeof(BOOL), 1, pTiFile);

        // module load address
        fread(&loadAddr, sizeof(gtUInt64), 1, pTiFile);

        // load time; skip
        fread(&dtemp, sizeof(DWORD), 1, pTiFile);
        // imagebase; skip
        fread(&temp64, sizeof(gtUInt64), 1, pTiFile);

        // module size
        fread(&temp64, sizeof(gtUInt64), 1, pTiFile);
        modSize = (gtUInt32) temp64;

        // unload time; skip
        fread(&dtemp, sizeof(DWORD), 1, pTiFile);
        // length of name string; skip

        fread(&dtemp, sizeof(DWORD), 1, pTiFile);
        // name; skip
        fread((void*)tempPath, sizeof(wchar_t), dtemp, pTiFile);

        pModMap->insert(CSSMODMAP::value_type((gtUInt64)loadAddr, modSize));
    }

    fclose(pTiFile);
    pTiFile = NULL;
    ::DeleteFile(snapShotFile);
}

#if 0
void fnUpdateOclKernelJit(unsigned int pid, gtUInt64* pJitAddress, unsigned int size, wchar_t* pJitPath)
{
    g_TaskInfo.UpdateOclKernelJit(pid, pJitAddress, size, pJitPath);
}
#endif

void fnSetExecutableFilesSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath)
{
    g_TaskInfo.SetExecutableFilesSearchPath(pSearchPath, pServerList, pCachePath);
}

void fnLoadProcessExecutableFiles(gtUInt64 processId, osSynchronizedQueue<gtString>& statusesQueue)
{
    g_TaskInfo.LoadProcessExecutableFiles(processId, statusesQueue);
}

PeFile* fnFindExecutableFile(gtUInt64 processId, gtUInt64 addr)
{
    return g_TaskInfo.FindExecutableFile(processId, addr);
}

unsigned int fnForeachExecutableFile(gtUInt64 processId, bool kernel, void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext)
{
    return g_TaskInfo.ForeachExecutableFile(processId, kernel, pfnProcessModule, pContext);
}
