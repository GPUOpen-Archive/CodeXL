//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief AMDTProfilerDataAccess.h - APIs used to access the profile data stored in the db.
//
//=============================================================

#ifndef _AMDTPROFILEDATAACCESS_H_
#define _AMDTPROFILEDATAACCESS_H_

// Base headers
#include <AMDTCommonHeaders/AMDTDefinitions.h>
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>

//
//  typedefs
//

#if defined(_WIN32)
#if defined(CXLPROFILEDATA_READER_EXPORTS)
#define CXLPROFILEDATA_READER_API __declspec(dllexport)
#else
#define CXLPROFILEDATA_READER_API __declspec(dllimport)
#endif
#else
#define CXLPROFILEDATA_READER_API
#endif

#define CXL_DATAACCESS_SUCCESS                               0
#define CXL_DATAACCESS_ERROR_INTERNAL                       0xF0000001
#define CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE           0xF0000002   // debug info not available
#define CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE         0xF0000003   // binary not available

struct cgEdge
{
    AMDTProfileFunctionInfo     m_funcInfo;
    gtVAddr                     m_moduleBaseAddr = 0;

    AMDTSampleValue             m_selfSamples;
    AMDTSampleValue             m_deepSamples;
};

using cgEdgeVec = gtVector<cgEdge>;

struct cgNode
{
    AMDTProfileFunctionInfo     m_funcInfo;
    gtVAddr                     m_moduleBaseAddr = 0;

    cgEdgeVec                   m_callerVec;
    cgEdgeVec                   m_calleeVec;

    AMDTSampleValue             m_totalSelfSamples;
    AMDTSampleValue             m_totalDeepSamples;
    gtUInt32                    m_pathCount = 0;
};

using functionIdcgNodeMap = gtMap<AMDTFunctionId, cgNode>;

class CXLPROFILEDATA_READER_API cxlProfileDataReader
{
public:
    cxlProfileDataReader();
    ~cxlProfileDataReader();

    // Prevent copy, assignment
    cxlProfileDataReader(const cxlProfileDataReader& other) = delete;
    cxlProfileDataReader& operator=(const cxlProfileDataReader& other) = delete;

    bool OpenProfileData(gtString profileFilePath);
    bool CloseProfileData();

    bool GetProfileSessionInfo(AMDTProfileSessionInfo& sessionInfo);
    bool GetCpuTopology(AMDTCpuTopologyVec& cpuToplogy);

    // Returns the information about the counters used for sampling to collect profile data.
    // Does not return the calculated counetrs like CPI, IPC etc
    bool GetSampledCountersList(AMDTProfileCounterDescVec& counterDesc);
    bool GetSamplingConfiguration(AMDTUInt32 counterId, AMDTProfileSamplingConfig& counterDesc);
    bool GetReportConfigurations(AMDTProfileReportConfigVec& reportConfigs);

    bool SetDebugInfoPaths(gtVector<gtString>& symbolServer, gtVector<gtString>& symbolDirectory);
    bool SetSourcePaths(gtVector<gtString>& sourceDirPath);
    bool SetBinaryPaths(gtVector<gtString>& binaryDirPath);
    bool SetReportOption(AMDTProfileDataOptions& options);
    bool SetReportOption(AMDTReportOptionType type, gtUInt64 value);
    bool SetReportCounters(gtVector<AMDTUInt32>& countersList);

    bool GetProcessInfo(AMDTUInt32 pid, AMDTProfileProcessInfoVec& procInfo);
    bool GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, AMDTProfileModuleInfoVec& procInfo);
    bool GetThreadInfo(AMDTUInt32 pid, AMDTThreadId tid, AMDTProfileThreadInfoVec& procInfo);

    //  Summary APIs
    bool GetProcessSummary(AMDTUInt32 counterId, AMDTProfileDataVec& processSummaryData);
    bool GetThreadSummary(AMDTUInt32 counterId, AMDTProfileDataVec& threadSummaryData);
    bool GetModuleSummary(AMDTUInt32 counterId, AMDTProfileDataVec& moduleSummaryData);
    bool GetFunctionSummary(AMDTUInt32 counterId, AMDTProfileDataVec& funcSummaryData);

    // Process/Module/Funtion View APIs
    bool GetProcessProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& processProfileData);
    bool GetModuleProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& moduleProfileData);
    bool GetFunctionProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& funcProfileData);

    int GetFunctionDetailedProfileData(AMDTFunctionId            funcId,
                                       AMDTProcessId             processId,
                                       AMDTThreadId              threadId,
                                       AMDTProfileFunctionData&  functionData);

    int GetFunctionSourceAndDisasmInfo(AMDTFunctionId funcId,
                                       gtString& srcFilePath,
                                       AMDTSourceAndDisasmInfoVec& srcInfo);

    int GetDisassembly(AMDTModuleId moduleId,
                       AMDTUInt32 offset,
                       AMDTUInt32 size,
                       AMDTSourceAndDisasmInfoVec& srcInfo);

    bool GetCallGraphProcesses(gtVector<AMDTProcessId>& cssProcesses);
    bool IsProcessHasCssSamples(AMDTProcessId pid);

    bool GetCallGraphFunctions(AMDTProcessId pid, AMDTCounterId counterId, AMDTCallGraphFunctionVec& cgFuncsVec);

    bool GetCallGraphFunctionInfo(AMDTProcessId pid, AMDTFunctionId funcId, AMDTCallGraphFunctionVec& caller, AMDTCallGraphFunctionVec& callee);

    bool GetCallGraphPaths(AMDTProcessId processId, AMDTFunctionId funcId, gtVector<AMDTCallGraphPath>& paths);

private:
    class Impl;
    Impl* m_pImpl;
};

#endif // _AMDTPROFILEDATAACCESS_H_