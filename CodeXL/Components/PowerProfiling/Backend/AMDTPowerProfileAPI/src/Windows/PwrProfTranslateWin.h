//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PwrProfTranslateWin.h
///
//==================================================================================
#ifndef PWR_PROF_TRANSLATE_WIN_H_
#define PWR_PROF_TRANSLATE_WIN_H_

// Project Include
#include <PowerProfileTranslate.h>

//IpAddressInfo:
typedef struct IpAddressInfo
{
    AMDTUInt32   m_sampleCnt;
    AMDTFloat32  m_power;
} IpAddressInfo;

//Ip map: (ip, ip info)
typedef gtMap<AMDTUInt64, IpAddressInfo> IpAddressMap;

// ModuleIntanceInfo:
typedef struct  ModuleIntanceInfo
{
    AMDTFloat32  m_power;
    AMDTUInt32   m_sampleCnt;
    IpAddressMap m_ipMap;
} ModuleIntanceInfo;

// instance map: (module instance id, instance info)
typedef gtMap<AMDTUInt32, ModuleIntanceInfo> ModuleInstanceMap;

// ProcessInfo:
typedef struct  ProcessInfo
{
    AMDTFloat32       m_power;
    AMDTUInt32        m_sampleCnt;
    ModuleInstanceMap m_modInstMap;
} ProcessInfo;

// SampleData:
typedef struct SampleData
{
    AMDTUInt64     m_ip;
    AMDTUInt32     m_processId;
    AMDTUInt32     m_threadId;
    AMDTUInt64     m_timeStamp;
    AMDTUInt32     m_modInstance;
    AMDTFloat32    m_ipc;
    AMDTUInt32     m_sampleCnt;
} SampleData;

// sample key(ip+instance, sample info)
typedef gtMap<AMDTUInt64, SampleData> SampleMap;

// process map: (process id, process info)
typedef gtMap<AMDTUInt32, ProcessInfo> ProcessTreeMap;

// Interesting module instance list(instance id, module info)
typedef gtMap<AMDTUInt32, LoadModuleInfo> ModuleInfoMap;

class PwrProfTranslateWin : public PowerProfileTranslate
{
public:
    void InitializeModuleMap();
    void CleanupModuleMap();
    AMDTResult PwrGetProfileData(CXLContextProfileType type, void** pData, AMDTUInt32* pCnt, AMDTFloat32* pPower);

private:
    void SetElapsedTime(AMDTUInt64 raw, AMDTUInt64* pResult);
    AMDTResult AttributePowerToSample();
    AMDTResult ProcessSample(ContextData* pCtx, AMDTUInt32 coreId, AMDTUInt32 componentIdx);

    AMDTResult InsertSampleToSystemTree(SampleData* pCtx, AMDTFloat32 power, AMDTUInt32 sampleCnt);
    void ExtractNameAndPath(char* pFullPath, char* pName, char* pPath);

    // DATA MEMBERS
    ModuleInfoMap m_moduleTable;
    AMDTUInt64 m_perfFreq;
    AMDTUInt64 m_prevTs1;
    ProcessTreeMap m_systemTreeMap;
    SampleMap m_sampleMap[MAX_CORE_CNT];
};
#endif // PWR_PROF_TRANSLATE_WIN_H_