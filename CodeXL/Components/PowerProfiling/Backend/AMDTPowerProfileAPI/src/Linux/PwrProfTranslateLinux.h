//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileTranslate.h
///
//==================================================================================

#ifndef _LINUX_PWR_PROF_TRANS_H_
#define _LINUX_PWR_PROF_TRANS_H_

// Project include
#include <PowerProfileTranslate.h>

class PwrProfTranslateLinux : public PowerProfileTranslate
{
public:
    // InitializeModuleMap: initialize the data structure for process/module profiling
    void InitializeModuleMap();

    // CleanupModuleMap: cleanup data structure for process/module profiling
    void CleanupModuleMap();

    // PwrGetProfileData: Get the profile data
    AMDTResult PwrGetProfileData(CXLContextProfileType type,
                                 void** pData,
                                 AMDTUInt32* pCnt,
                                 AMDTFloat32* pPower);

private:
    // PwrGetCountsPerSecs:  get the TS count per sec
    AMDTFloat32 PwrGetCountsPerSecs() const;

    // SetElapsedTime: Get the Elapsed time
    void SetElapsedTime(AMDTUInt64 raw, AMDTUInt64* pResult);

    // AttributePowerToSample : Collect the power/ipc for each sample
    AMDTResult AttributePowerToSample();

    // ProcessSample: Process the collected data for each sample
    AMDTResult ProcessSample(ContextData* pCtx, AMDTUInt32 coreId, AMDTUInt32 componentIdx);

    //ReadProcPidMap : Read the /proc/$pid/maps file and populate process ==> module map
    //                  for each process
    AMDTResult ReadProcPidMap(ContextData* pCtx, ProcPidModInfo& info);

    // UpdateModuleSampleMap: Update the moduleId ==> moduleInfo map for each sample collected
    AMDTResult UpdateModuleSampleMap(const std::vector<ModInfo>& modInfoTable,
                                     ContextData* pCtx,
                                     AMDTUInt32 componentIdx,
                                     AMDTFloat32 ipc);

    // GetProcessName: Get the process name
    void GetProcessName(AMDTPwrProcessInfo* pInfo);

    //ExtractNameAndPath : Extract module name and path from full path
    void ExtractNameAndPath(char* pFullPath, char* pName, char* pPath);

    // DATA MEMBERS

    // Each process id maps to a vector of module contained by that process
    // process_id ==> module info maps
    ProcPidModTable m_procPidModTable;

    // Maps all modules of their respective components
    // component number ==> module id ==> sample data of module
    ComponentModSampleTable m_modSampleDataTable;

    // Aggregation of module power/ipc mapped with module index
    // module id ==> aggregiation of sample data of module
    ModuleSampleDataMap   m_aggrModSampleDataMap;

    // module id ==> module name, start address, end address, ..
    ModuleIdInfoMap m_moduleIdInfoMap;

    // total number of modules during profing
    AMDTUInt64 m_moduleCnt = 0;

    // set once when module inserted for swapper process (pid = 0)
    bool m_isSwapperPid = false;
};

#endif
