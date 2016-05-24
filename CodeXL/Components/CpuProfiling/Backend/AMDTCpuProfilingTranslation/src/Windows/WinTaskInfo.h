//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file WinTaskInfo.h
/// \brief Windows task information interface.
///
//==================================================================================

#ifndef _WINTASKINFO_H_
#define _WINTASKINFO_H_

// Suppress Qt header warnings
#pragma warning(push)
#pragma warning(disable : 4127 4718)
#include <QMap>
#include <QList>
#pragma warning(pop)

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtHashMap.h>
#include "TaskInfoMapper.h"
#include "JitTaskInfo.h"

#include <psapi.h>
#include <WinIoCtl.h>
#include <Driver/Windows/CpuProf/inc/UserAccess/CpuProfDriver.h>

#define TASK_INFO_FILE_SIGNATURE  FCC('cati')

#define TASK_INFO_FILE_VERSION_4    0x00020004  // major version (hight 16 bits) 2, minor version (low 16 bits) 4);

#define TASK_INFO_FILE_VERSION_3    0x00020003  // major version (hight 16 bits) 2, minor version (low 16 bits) 3);

#define TASK_INFO_FILE_VERSION_2    0x00010002  // major version (hight 16 bits) 1, minor version (low 16 bits) 2);

#define TASK_INFO_FILE_VERSION_1    0x00010001  // major version (hight 16 bits) 1, minor version (low 16 bits) 1);
// previous TASK INFO FILE VERSION
#define TASK_INFO_FILE_VERSION_0    0x00010000  // major version (hight 16 bits) 1, minor version (low 16 bits) 0);

//This is current task info file version
#define TASK_INFO_FILE_VERSION      TASK_INFO_FILE_VERSION_4

// define the Task Info driver name
#define CA_TASKINFO_DRIVER_NAME "CAPROF"

typedef gtMap<gtUInt64, gtUInt64> PidMap;
typedef gtHashMap<std::wstring, gtUInt32> ModuleIdMap;

struct ModNameAddrKey
{
    gtString  m_modName;
    gtVAddr   m_loadAddr;

    ModNameAddrKey(gtString name, gtVAddr addr) : m_modName(name), m_loadAddr(addr)
    { }

    bool operator< (const ModNameAddrKey& other) const
    {
        bool retVal = false;

        if (this->m_loadAddr == other.m_loadAddr)
        {
            retVal = this->m_modName.compareNoCase(other.m_modName) < 0;
        }
        else
        {
            retVal = this->m_loadAddr < other.m_loadAddr;
        }

        return retVal;
    }
};

typedef std::multimap<ModNameAddrKey, PeFile*> ModulePeFileMap;

class PeFile;

class WinTaskInfo : public JitTaskInfo
{
public:
    // constructor
    WinTaskInfo();
    // destructor
    ~WinTaskInfo();

    // clean up the maps and time marks
    void Cleanup();

    // start task info capturing, with optional directory for driver
    HRESULT StartCapture(gtUInt64 startCount, const wchar_t* binDirectory);

    // stop task info capturing
    HRESULT StopCapture(bool bConvertFlag, const wchar_t* tempTiFileName);

    // get number of processes in the process map
    HRESULT GetProcessesNum(/* [out] */ unsigned* procNum);

    // get all process info
    HRESULT GetAllProcesses(/* [out] */ ProcQueryInfo* pProcQueryInfo, /* [in] */  unsigned procNum);

    // Find process name for a given process id.
    //Caller allocates the space for process name.
    HRESULT FindProcessName(/* [in] */ gtUInt64 processID, /* [out] */ wchar_t* processName, /* [in] */ unsigned sizeofname);

    // get number of modules in a given process
    HRESULT GetModNumInProc(/* [in] */ gtUInt64 processID, /* [out] */ unsigned* pNumOfMod);

    // Get nth module in a given process
    //Caller allocates the space for module name.
    HRESULT GetModuleInfoByIndex(/* [in] */ gtUInt64 processID,
                                            /* [in] */ unsigned modIndex,
                                            /* [out] */ gtUInt64* pModuleStartAddr,
                                            /* [out] */ gtUInt64* pModulesize,
                                            /* [out] */ wchar_t* pModulename,
                                            /* [in] */ unsigned namesize);

    // for a given sample record, identify which module the sample is in.
    // Caller allocates the space for module name.
    HRESULT GetModuleInfo(TiModuleInfo* pModInfo);

    // Get intance id of a module from ip address, pid and time stamp
    HRESULT GetModuleInstanceId(gtUInt32 processId, gtUInt64 sampleAddr, gtUInt64 deltaTick, gtUInt32& modInstId);

    // GetModuleInfoFromInstanceId: Get module information for a give instance id
    HRESULT GetLoadModuleInfoByInstanceId(gtUInt32 instanceId, LoadModuleInfo* pModInfo);

    HRESULT GetProcessThreadList(gtVector<std::tuple<gtUInt32, gtUInt32>>& info);

    // Get number of kernel modules
    HRESULT GetKernelModNum(/* [out] */ unsigned* pKeModNum);

    // Get all kernel module info
    // Caller allocates space for the KeModQueryInfo
    HRESULT GetAllKeMod(/* [out] */ KeModQueryInfo* pKeMods, /* [in] */  unsigned keModNum);

    // Get maximum number of AMD CPU Profiling drivers (in Windows this is 2 for the CpuProf.sys and pcore.sys)
    HRESULT GetCpuProfilingDriversMaxCount(/* [out] */ unsigned* pKeModNum) const;

    // Get the AMD CPU Profiling drivers info (in Windows these are the CpuProf.sys and pcore.sys)
    // Caller allocates space for the KeModQueryInfo
    HRESULT GetCpuProfilingDrivers(/* [out] */ KeModQueryInfo* pKeMods, /* [in,out] */  unsigned& keModNum);

    unsigned int GetNumThreadInProcess(gtUInt64 processID);

    HRESULT GetThreadInfoInProcess(gtUInt64 processID, unsigned int sizeOfInfoArray, TI_THREAD_INFO* pThreadInfoArray);

    // Write the all module info into task info file
    HRESULT WriteModuleInfoFile(/* [in] */ const wchar_t* filename);

    //  read module info from task info file and build up maps
    HRESULT ReadModuleInfoFile(/* [in] */ const wchar_t* filename);

    HRESULT ReadCLRJitInformation(/* [in] */ const wchar_t* clrdirectory, const wchar_t* sessionDir);

    HRESULT WriteCLRJncFiles(/*[in]*/ const wchar_t* directory);

#ifdef CXL_SUPPORTS_INTERPRETED_CODE
    //read the jit info after processing
    HRESULT ReadOldJitInfo(/* [in] */ const wchar_t* directory);

    HRESULT WriteJncFiles(/*[in]*/ const wchar_t* directory);
#endif // CXL_SUPPORTS_INTERPRETED_CODE

    // clean up maps
    void CleanupMaps() { Cleanup(); }

    // Get error message.
    const gtASCIIString& GetErrorMsg() const { return m_ErrorMsg; }

    gtUInt64 GetMaxApplicationAddr() const { return m_lpMaxAppAddr; }

    FILETIME Synchronize(FILETIME start, gtUInt64 deltaTick, int core, unsigned int extraMs);

    //void UpdateOclKernelJit(unsigned int pid, gtUInt64* pJitAddress, unsigned int size, const wchar_t* pJitOutputDir);

    void SetExecutableFilesSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath);
    void LoadProcessExecutableFiles(gtUInt64 processId, osSynchronizedQueue<gtString>& statusesQueue);

    void AddLoadModules(gtUInt64 processId);

    PeFile* FindExecutableFile(gtUInt64 processId, gtUInt64 addr) const;

    unsigned int ForeachExecutableFile(gtUInt64 processId, bool kernel, void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext) const;

private:
    bool GetUserPeModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick, ModuleMap::value_type& item);
    bool GetUserOclModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick, ModuleMap::value_type& item);

    // generate temp file name for driver, snapshot file names
    HRESULT GenerateTempFiles();

    // process the task info records written by driver
    HRESULT ProcessTiTempFile(bool bConvertFlag, const wchar_t* tempTiFileName);

    // Update the process unload time when process is terminated
    void OnTerminateProcess(gtUInt64 processID, DWORD endtime);

    // update the module info
    void OnLoadImage(const TASK_INFO_RECORD& tiRecord, bool bConvertFlag);

    // update thread info
    void OnThreadCreation(const TASK_INFO_RECORD& tiRecord);
    void OnThreadDeletion(const TASK_INFO_RECORD& tiRecord);

    // Calculate the mdoule unload time.
    void CalculateEndTime();

    // calculate kernel module size.
    void CalculateDriverSize();

    // get hard disk partition info
    void GetDrivePartitionInfo();

    // attempt to add drive label front the path and validate if the file exists.
    HRESULT AddDriveLabel(const wchar_t* pModulename, wchar_t* pString) const;
    bool FileExists(const wchar_t* pModulename) const;

    // enumerate device driver info
    HRESULT DriverSnapShot();

    //Set Privilege to get the handles of system process
    //caller must get Token handle first
    HRESULT SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege);

    // convert module name
    void ConvertName(wchar_t* pModulename, gtUInt64 procId) const;
    gtUInt64 GetModuleSize(const wchar_t* pModuleName) const;

    HRESULT GetUserModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick);
    HRESULT GetKernelModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick);

    // enumerate user mode threads associated with the process id
    HRESULT EnumProcessThreads();

    // enumerate user mode processes and module info
    HRESULT ProcModSnapShot();

    // Read the process and module snap shot file
    HRESULT ProcessSnapShotFile(const wchar_t* pfilename, BOOL b32bitFlag, bool bConvertFlag);

    // launch 32-bit or 64-bit process to enumerate process and module info
    HRESULT LaunchModuleEnumerator(bool b32bit = true, const wchar_t* binDirectory = NULL);

    // Find the module id for the given module name
    HRESULT FindModuleId(const wchar_t* pModuleName, gtInt32& moduleId);

    // the snapshot file name for 32-bit process
    wchar_t m_tiSnapshotFileName[OS_MAX_PATH + 1];

    // driver task info file name.
    wchar_t m_tiFileName[OS_MAX_PATH + 1];

    // structure of time mark
    struct TimeMark
    {
        DWORD     sysTime_ms;     // system time in milliseconds
        gtUInt64  cpuTimeStamp;   // cpu time stamp (no longer used)

        // constructor for time mark
        TimeMark() : sysTime_ms(0), cpuTimeStamp(0) {}
    };

    // enumeration of time stamp measurement type
    enum EnumMeasureType
    {
        evInvalid,
        evStartMark,
        evStopMark
    };

    // the current Process index
    int m_CurrentProcessorNum;

    // measurement type
    EnumMeasureType m_Mtype;

    // max address for user application
    gtUInt64  m_lpMaxAppAddr;
    gtUInt64  m_lpMinAppAddr;

    // process map
    ProcessMap  m_tiProcMap;

    // ModuleName/LoadAddress and PeFile map, to avoid crearting
    // duplicate PeFile objects for the same DLL
    ModulePeFileMap m_modLoadAddrPEMap;

    // Baskar: During profile data collection, the TI contains information about all the processes
    // (and their load-modules) that are running/stopped during profile time (1000's of entries).
    // Where as typically samples will be attributed to only few processes and their loadmodules.
    // m_tiModMap contains info of all the load modules that are discovered during profile durtion.
    // And for each sample (IP and CSS RIPs) we need to discover the corresponding load-modules.
    // This upper/lower operations on such a huge set ("S) for reasonably big "n" (IP + CSS RIPs samples)
    // is time consuming. We cannot reduce the "n", but "S" can be greatly simplified by maintaining
    // only the modules details about the relavant/interesting processes.
    //
    // m_allModulesMap -> will be constructed while processing TI file and this will have details
    // about all the load modules identified during data collection.
    // m_tiModMap -> will only contain the loadmodule details of the interesting processes.

    ModuleMap m_allModulesMap;
    PidMap m_interestingPidMap;
    ModuleIdMap m_moduleIdMap;

    // kernel module map
    KernelModMap m_tiKeModMap;

    // drive map
    DriveMap    m_tiDriveMap;

    // Thread Info Map
    ThreadInfoMap m_ThreadMap;

    // prejit module symbol file map
    PreJitModSymbolMap m_PJSMap;

    // error message.
    gtASCIIString m_ErrorMsg;

    // flag to indicate if the partition info had been measured.
    bool m_bPartitionInfoflag;

    // map status
    bool m_bMapBuilt;

    bool m_bLoadKernelPeFiles;

    wchar_t* m_pSearchPath;
    wchar_t* m_pServerList;
    wchar_t* m_pCachePath;

    osCriticalSection m_TIMutex;
    osCriticalSection m_TIMutexKE;
    osCriticalSection m_TIMutexModule;

    gtUInt32 m_nextModInstanceId = 1;
    gtInt32 m_nextModuleId = 1;
};

#endif // _WINTASKINFO_H_