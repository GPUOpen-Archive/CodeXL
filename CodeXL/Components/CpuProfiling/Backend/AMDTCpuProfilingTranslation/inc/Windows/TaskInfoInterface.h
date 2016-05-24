//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TaskInfoInterface.h
/// \brief Header file that defines the Windows Task Info APIs.
///
//==================================================================================

#ifndef _TASKINFOINTERFACE_H_
#define _TASKINFOINTERFACE_H_

#include <AMDTCpuProfilingTranslation/inc/CpuProfilingTranslationDLLBuild.h>
#include <AMDTCpuProfilingTranslation/inc/JitTaskInfoMapper.h>
#include <AMDTOSWrappers/Include/osSynchronizedQueue.h>

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

// key is module load address, data is size
typedef gtMap<gtUInt64, gtUInt32> CSSMODMAP;

struct TI_THREAD_INFO
{
    DWORD threadID;
    DWORD cpuNumCreated;
    DWORD cpuNumDeleted;
    DWORD threadCreationTime;
    DWORD threadDeletionTime;
};

// ModuleInfo: This structure is use for power profiler
typedef struct LoadModuleInfo
{
	gtUInt32	m_pid;
	gtUInt64	m_moduleStartAddr;
	gtUInt32	m_modulesize;
	ModTypeEnum m_moduleType;
	char		m_pModulename[OS_MAX_PATH];
	bool		m_isKernel;
	gtUInt32	m_moduleId;
	gtUInt32	m_instanceId;
} LoadModuleInfo;

// start task info capturing, with optional directory for driver
CP_TRANS_API HRESULT fnStartCapture(gtUInt64 startCount, const wchar_t* binDirectory = NULL);

// stop task info capturing
CP_TRANS_API HRESULT fnStopCapture(bool bConvertFlag, wchar_t* tempTiFileName);

// removes non-active Java information from the directory in the registry
CP_TRANS_API HRESULT fnCleanupJitInformation();

// read the agent Java information in the directory in the registry
//copy all java process id directories to the session directory
CP_TRANS_API HRESULT fnReadJitInformation(const wchar_t* sessionDir);

// read the agent CLR information in the directory in the registry
//copy all CLR process id directories to the session directory
CP_TRANS_API HRESULT fnReadCLRJitInformation(const wchar_t* sessionDir);

// read the saved jit information in the directory
CP_TRANS_API HRESULT fnReadOldJitInformation(/*[in]*/ const wchar_t* directory);

//Write the sampled JNC files to the session directory
CP_TRANS_API HRESULT fnWriteJncFiles(/*[in]*/ const wchar_t* directory);

// Write the all module info into task info file
CP_TRANS_API HRESULT fnWriteModuleInfoFile(/* [in] */ const wchar_t* filename);

//  read module info from task info file and build up maps
CP_TRANS_API HRESULT fnReadModuleInfoFile(/* [in] */ const wchar_t* filename);

// Clean up maps
// Note: Once maps are cleaned up, all querying will return S_FALSE until maps are rebuilt.
CP_TRANS_API void fnCleanupMaps();

// get the last error message
//CP_TRANS_API string fnGetErrorMsg();

// get number of processes in the process map
CP_TRANS_API HRESULT fnGetProcessNum(/* [out] */ unsigned* procNum);

// Get all process info
// Caller allocates the space for ProcQueryInfo struct.
CP_TRANS_API HRESULT fnGetAllProcesses(/* [out] */ ProcQueryInfo* pProcQueryInfo, /* [in] */  unsigned procNum);

// Find process name for a given process id.
//Caller allocates the space for process name.
CP_TRANS_API HRESULT fnFindProcessName(
    /* [in] */  gtUInt64 processID, /* [out] */ wchar_t* processName, /* [in] */  unsigned sizeofname);

// get number of modules in a given process
CP_TRANS_API HRESULT fnGetModNumInProc(/* [in] */  gtUInt64 processID, /* [out] */ unsigned* pNumOfMod);

// *** Obsolate API ***
// This API was not updated for JAVA profile and CLR profile.
// --Lei 03/07/05
//
// Get nth module in a given process
//Caller allocates the space for module name.
CP_TRANS_API HRESULT fnGetModuleInfoByIndex(/* [in] */  gtUInt64 processID,
                                                        /* [in] */  unsigned modIndex,
                                                        /* [out] */ gtUInt64* pModuleStartAddr,
                                                        /* [out] */ gtUInt64* pModulesize,
                                                        /* [out] */ wchar_t* pModulename,
                                                        /* [in] */  unsigned namesize);

// for a given sample record, identify which module the sample is in.
//Caller allocates the space for module name.
CP_TRANS_API HRESULT fnGetModuleInfo(TiModuleInfo* info);

// Get module instance id for a given samples process id, time offset and ip address
CP_TRANS_API HRESULT fnGetModuleInstanceId(gtUInt32 processId, gtUInt64 sampleAddr, gtUInt64 deltaTick, gtUInt32& modInstId);

// Get module information for a give module instance id
CP_TRANS_API HRESULT fnGetModuleInfoByInstanceId(gtUInt32 instanceId, LoadModuleInfo* pModInfo);


// Get the module instance info as a vector of <instanceId, modName, pid, loadAddr>
CP_TRANS_API HRESULT fnGetProcessThreadList(gtVector<std::tuple<gtUInt32, gtUInt32>>& info);

// Get number of kernel modules
CP_TRANS_API HRESULT fnGetKernelModNum(/* [out] */ unsigned* pNumOfKeMod);

// Get all kernel module info
// caller allocates the space for KeModQueryInfo structure
CP_TRANS_API HRESULT fnGetAllKeMod(/* [out] */ KeModQueryInfo* pKeMods, /* [in] */  unsigned keModNum);

// Get maximum number of AMD CPU Profiling drivers (in Windows this is 2 for the CpuProf.sys and pcore.sys)
CP_TRANS_API HRESULT fnGetCpuProfilingDriversMaxCount(/* [out] */ unsigned* pNumOfKeMod);

// Get the AMD CPU Profiling drivers info (in Windows these are the CpuProf.sys and pcore.sys)
// caller allocates the space for KeModQueryInfo structure
CP_TRANS_API HRESULT fnGetCpuProfilingDrivers(/* [out] */ KeModQueryInfo* pKeMods, /* [in,out] */  unsigned& keModNum);

// Get Number of Thread in a given process ID.
CP_TRANS_API unsigned int fnGetNumThreadInProcess(/* [in] */ gtUInt64 processID);

// Get thread info for a given process. Caller allocates the space for
// the TI_THREAD_INFO.
CP_TRANS_API HRESULT fnGetThreadInfoInProcess(
    /* [in] */ gtUInt64 processID, /* [in] */ unsigned int sizeOfInfoArray, /* [out] */ TI_THREAD_INFO* pThreadInfoArray);

// Get JIT process bitness for a given process ID.
CP_TRANS_API bool fnIsJITProcess32Bit(gtUInt64 jitProcID);

//This will return the FILETIME for a given timestamp
CP_TRANS_API FILETIME fnSynchronize(FILETIME start, gtUInt64 deltaTick, int core, unsigned int extraMs = 0);

//This will return a list of module load info for a given pid;
CP_TRANS_API void fnGetCSSModules(unsigned int pid, CSSMODMAP* pModMap, BOOL* pIs32);

//This will clear the memory allocated in fnGetCSSModules
CP_TRANS_API void fnClearCSSModules(CSSMODMAP* pModMap);

// This will update the module map to convert the OCL temporary JIT dll string into a JIT dll that is logged into session dir.
//CP_TRANS_API void fnUpdateOclKernelJit(unsigned int pid, gtUInt64* pJitAddress, unsigned int size, wchar_t* pJitPath);

void fnSetExecutableFilesSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath);

void fnLoadProcessExecutableFiles(gtUInt64 processId, osSynchronizedQueue<gtString>& statusesQueue);

PeFile* fnFindExecutableFile(gtUInt64 processId, gtUInt64 addr);

unsigned int fnForeachExecutableFile(gtUInt64 processId, bool kernel, void (*pfnProcessModule)(ExecutableFile&, void*), void* pContext);

#endif // _TASKINFOINTERFACE_H_
