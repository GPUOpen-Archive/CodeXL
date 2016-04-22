//=====================================================================
// Copyright (c) 2013-2016 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief Interface for the ProfilerDataDBWriter class.
//
//=====================================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=====================================================================

#ifndef _PROFILEDATADBWRITER_H_
#define _PROFILEDATADBWRITER_H_

#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include "CpuProfilingRawDataDLLBuild.h"
#include "CpuProfileInfo.h"
#include "CpuProfileModule.h"
#include "CpuProfileProcess.h"

class CP_RAWDATA_API ProfilerDataDBWriter
{
public:
    ProfilerDataDBWriter() : m_pCpuProfDbAdapter(new amdtProfileDbAdapter) {};
    ~ProfilerDataDBWriter()
    {
        if (m_pCpuProfDbAdapter != nullptr)
        {
            m_pCpuProfDbAdapter->CloseDb();
            delete m_pCpuProfDbAdapter;
            m_pCpuProfDbAdapter = nullptr;
        }
    }

    bool Write(const gtString& path,
               CpuProfileInfo& profileInfo,
               gtUInt64 cpuAffinity,
               const PidProcessMap& procMap,
               gtVector<std::tuple<gtUInt32, gtUInt32>>& processThreadList,
               const NameModuleMap& modMap,
               const gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>& modInstanceInfo,
               const CoreTopologyMap* topMap = nullptr);

private:
    void PackSessionInfo(const CpuProfileInfo& profileInfo, gtUInt64 cpuAffinity, AMDTProfileSessionInfo& sessionInfo);
    void PackCoreTopology(const CoreTopologyMap& coreTopology, CPAdapterTopologyMap& cpaTopology);
    void PackSamplingEvents(const CpuProfileInfo& profileInfo,
                            AMDTProfileCounterDescVec& events,
                            AMDTProfileSamplingConfigVec& samplingConfigs);
    void PackProcessInfo(const PidProcessMap& processMap, CPAProcessList& processList);
    void PackModuleInfo(const NameModuleMap& modMap, CPAModuleList& moduleList);
    void PackModuleInstanceInfo(const NameModuleMap& modMap, const gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>& modInstanceInfoMap, CPAModuleInstanceList& moduleInstanceList);
    void PackProcessThreadInfo(const gtVector<std::tuple<gtUInt32, gtUInt32>>& processThreadList, CPAProcessThreadList& procThreadIdList);
    void PackCoreSamplingConfigInfo(const NameModuleMap& modMap, CPACoreSamplingConfigList& coreConfigList);
    void PackSampleInfo(const NameModuleMap& modMap, CPASampeInfoList& sampleList);
    void PackFunctionInfo(const NameModuleMap& modMap, CPAFunctionInfoList& funcInfoList);

    // TODO: These helper functions should be in utils
    gtString ConvertQtToGTString(const QString& inputStr);
    void DecodeSamplingEvent(EventMaskType encoded, gtUInt16& event, gtUByte& unitMask, bool& bitOs, bool& bitUsr);
    bool InitializeEventsXMLFile(gtUInt32 cpuFamily, gtUInt32 cpuModel, EventsFile& eventsFile);
    bool IsWindowsSystemModuleNoExt(const gtString& absolutePath);
    bool AuxIsLinuxSystemModule(const gtString& absolutePath);
    bool IsSystemModule(const gtString& absolutePath);

    amdtProfileDbAdapter* m_pCpuProfDbAdapter = nullptr;
};

#endif //_CPUPROFILEDATADBWRITER_H_
