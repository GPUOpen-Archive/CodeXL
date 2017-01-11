//==================================================================================
// Copyright (c) 2013-2017, Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TaskInfoInterface.h
/// \brief Header file that defines the Windows Task Info APIs.
///
//==================================================================================

#pragma once

#include <AMDTOSWrappers/Include/osSynchronizedQueue.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <inc/CXLTaskInfoDLLBuild.h>
#include <inc/JitTaskInfoMapper.h>


// key is module load address, data is size
typedef gtMap<gtUInt64, gtUInt32> CSSMODMAP;

struct ProcQueryInfo
{
    gtUInt64 pqiProcessID;
    wchar_t pqiProcessName[OS_MAX_PATH + 1];
};

// kernel module info
struct KeModQueryInfo
{
    gtUInt64 keModStartAddr;
    gtUInt64 keModEndAddr;
    wchar_t keModName[OS_MAX_PATH + 1];
};

struct TI_THREAD_INFO
{
    DWORD threadID;
    DWORD cpuNumCreated;
    DWORD cpuNumDeleted;
    DWORD threadCreationTime;
    DWORD threadDeletionTime;
};

// ModuleInfo: This structure is use for power profiler
struct LoadModuleInfo
{
    gtUInt32	m_pid = 0;
    gtUInt64	m_moduleStartAddr = 0;
    gtUInt32	m_modulesize = 0;
    ModTypeEnum m_moduleType = evInvalidType;
    char		m_pModulename[OS_MAX_PATH];
    bool		m_isKernel = false;
    gtUInt32	m_moduleId = 0;
    gtUInt32	m_instanceId = 0;
};

// start task info capturing, with optional directory for driver
TASKINFO_API HRESULT fnStartCapture(gtUInt64 startCount, const wchar_t* binDirectory = nullptr);

// stop task info capturing
TASKINFO_API HRESULT fnStopCapture(bool bConvertFlag, wchar_t* tempTiFileName);

// removes non-active Java information from the directory in the registry
TASKINFO_API HRESULT fnCleanupJitInformation();

// read the agent Java information in the directory in the registry
// copy all Java process id directories to the session directory
TASKINFO_API HRESULT fnReadJitInformation(const wchar_t* sessionDir);

// read the agent CLR information in the directory in the registry
// copy all CLR process id directories to the session directory
TASKINFO_API HRESULT fnReadCLRJitInformation(const wchar_t* sessionDir);

// read the saved JIT information in the directory
TASKINFO_API HRESULT fnReadOldJitInformation(/*[in]*/ const wchar_t* directory);

// Write the sampled JNC files to the session directory
TASKINFO_API HRESULT fnWriteJncFiles(/*[in]*/ const wchar_t* directory);

// Write the all module info into task info file
TASKINFO_API HRESULT fnWriteModuleInfoFile(/* [in] */ const wchar_t* filename);

// read module info from task info file and build up maps
TASKINFO_API HRESULT fnReadModuleInfoFile(/* [in] */ const wchar_t* filename);

// Clean up maps
// Note: Once maps are cleaned up, all querying will return S_FALSE until maps are rebuilt.
TASKINFO_API void fnCleanupMaps();

// get the last error message
//TASKINFO_API string fnGetErrorMsg();

// get number of processes in the process map
TASKINFO_API HRESULT fnGetProcessNum(/* [out] */ unsigned* procNum);

// Get all process info
// Caller allocates the space for ProcQueryInfo struct.
TASKINFO_API HRESULT fnGetAllProcesses(/* [out] */ ProcQueryInfo* pProcQueryInfo, /* [in] */  unsigned procNum);

// Find process name for a given process id.
// Caller allocates the space for process name.
TASKINFO_API HRESULT fnFindProcessName(
    /* [in] */  gtUInt64 processID, /* [out] */ wchar_t* processName, /* [in] */  unsigned sizeofname);

// get number of modules in a given process
TASKINFO_API HRESULT fnGetModNumInProc(/* [in] */  gtUInt64 processID, /* [out] */ unsigned* pNumOfMod);

// *** Obsolate API ***
// This API was not updated for JAVA profile and CLR profile.
//
// Get nth module in a given process
// Caller allocates the space for module name.
TASKINFO_API HRESULT fnGetModuleInfoByIndex(/* [in] */  gtUInt64 processID,
                                           /* [in] */  unsigned modIndex,
                                           /* [out] */ gtUInt64* pModuleStartAddr,
                                           /* [out] */ gtUInt64* pModulesize,
                                           /* [out] */ wchar_t* pModulename,
                                           /* [in] */  unsigned namesize);

// for a given sample record, identify which module the sample is in.
// Caller allocates the space for module name.
TASKINFO_API HRESULT fnGetModuleInfo(TiModuleInfo* info);

// Get module instance id for a given samples process id, time offset and ip address
TASKINFO_API HRESULT fnGetModuleInstanceId(gtUInt32 processId, gtUInt64 sampleAddr, gtUInt64 deltaTick, gtUInt32& modInstId);

// Get module information for a give module instance id
TASKINFO_API HRESULT fnGetModuleInfoByInstanceId(gtUInt32 instanceId, LoadModuleInfo* pModInfo);

// Get the module instance info as a vector of <instanceId, modName, pid, loadAddr>
TASKINFO_API HRESULT fnGetProcessThreadList(gtVector<std::tuple<gtUInt32, gtUInt32>>& info);

// Get number of kernel modules
TASKINFO_API HRESULT fnGetKernelModNum(/* [out] */ unsigned* pNumOfKeMod);

// Get all kernel module info
// caller allocates the space for KeModQueryInfo structure
TASKINFO_API HRESULT fnGetAllKeMod(/* [out] */ KeModQueryInfo* pKeMods, /* [in] */  unsigned keModNum);

// Get maximum number of AMD CPU Profiling drivers (in Windows this is 2 for the CpuProf.sys and pcore.sys)
TASKINFO_API HRESULT fnGetCpuProfilingDriversMaxCount(/* [out] */ unsigned* pNumOfKeMod);

// Get the AMD CPU Profiling drivers info (in Windows these are the CpuProf.sys and pcore.sys)
// caller allocates the space for KeModQueryInfo structure
TASKINFO_API HRESULT fnGetCpuProfilingDrivers(/* [out] */ KeModQueryInfo* pKeMods, /* [in,out] */  unsigned& keModNum);

// Get Number of Thread in a given process ID.
TASKINFO_API unsigned int fnGetNumThreadInProcess(/* [in] */ gtUInt64 processID);

// Get thread info for a given process. Caller allocates the space for
// the TI_THREAD_INFO.
TASKINFO_API HRESULT fnGetThreadInfoInProcess(
    /* [in] */ gtUInt64 processID, /* [in] */ unsigned int sizeOfInfoArray, /* [out] */ TI_THREAD_INFO* pThreadInfoArray);

// Get JIT process bitness for a given process ID.
TASKINFO_API bool fnIsJITProcess32Bit(gtUInt64 jitProcID);

// This will return the FILETIME for a given timestamp
TASKINFO_API FILETIME fnSynchronize(FILETIME start, gtUInt64 deltaTick, int core, unsigned int extraMs = 0);

// This will return a list of module load info for a given pid;
TASKINFO_API void fnGetCSSModules(unsigned int pid, CSSMODMAP* pModMap, BOOL* pIs32);

// This will clear the memory allocated in fnGetCSSModules
TASKINFO_API void fnClearCSSModules(CSSMODMAP* pModMap);

TASKINFO_API void fnSetExecutableFilesSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath);

TASKINFO_API void fnLoadProcessExecutableFiles(gtUInt64 processId, osSynchronizedQueue<gtString>& statusesQueue);

TASKINFO_API PeFile* fnFindExecutableFile(gtUInt64 processId, gtUInt64 addr);

TASKINFO_API unsigned int fnForeachExecutableFile(gtUInt64 processId, bool kernel, void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext);

TASKINFO_API void fnGetJavaJitBlockInfo(gtVector<std::tuple<gtUInt32, gtString, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& jitBlockInfo);

TASKINFO_API void fnGetJavaJncInfo(gtVector<std::tuple<gtUInt32, gtString, gtString>>& jncInfoList);

TASKINFO_API void fnGetClrJitBlockInfo(gtVector<std::tuple<gtUInt32, gtString, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& jitBlockInfo);

TASKINFO_API void fnGetClrJncInfo(gtVector<std::tuple<gtUInt32, gtString, gtString>>& jncInfoList);
