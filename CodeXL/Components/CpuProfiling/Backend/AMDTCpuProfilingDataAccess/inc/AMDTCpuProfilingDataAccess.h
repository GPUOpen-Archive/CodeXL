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
    bool GetProcessProfileData(AMDTProcessId procId, AMDTProfileDataVec& processProfileData);
    bool GetModuleProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& moduleProfileData);
    bool GetFunctionProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& funcProfileData);

    bool GetFunctionDetailedProfileData(AMDTFunctionId            funcId,
                                        AMDTProcessId             processId,
                                        AMDTThreadId              threadId,
                                        AMDTProfileFunctionData&  functionData);

    bool GetFunctionSourceAndDisasmInfo(AMDTFunctionId funcId,
                                        gtString& srcFilePath,
                                        AMDTSourceAndDisasmInfoVec& srcInfo);

    bool GetDisassembly(AMDTModuleId moduleId,
                        AMDTUInt32 offset,
                        AMDTUInt32 size,
                        AMDTSourceAndDisasmInfoVec& srcInfo);

private:
    class Impl;
    Impl* m_pImpl;
};

#endif // _AMDTPROFILEDATAACCESS_H_