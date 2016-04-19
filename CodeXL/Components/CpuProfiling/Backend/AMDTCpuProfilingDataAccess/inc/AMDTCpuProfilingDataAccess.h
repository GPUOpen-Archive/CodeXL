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

typedef AMDTUInt64 AMDTProfileReaderHandle;

//
//  APIs
//

// profile file can either be raw PRD file or processed DB file
AMDTResult AMDTOpenProfileData(gtString profileFilePath, AMDTProfileReaderHandle& readerHandle);

AMDTResult AMDTCloseProfileData(AMDTProfileReaderHandle readerHandle);

AMDTResult AMDTGetProfileSessionInfo(AMDTProfileReaderHandle readerHandle, AMDTProfileSessionInfo& sessionInfo);

extern AMDTResult AMDTGetCpuTopology(AMDTProfileReaderHandle readerHandle, gtVector<AMDTCpuTopology>& cpuToplogy);

// Returns the information about the counters used for sampling to collect profile data.
// Does NoT return the calculated counetrs like CPI, IPC etc
extern AMDTResult AMDTGetSampledCountersList(AMDTProfileReaderHandle readerHandle,
                                             gtVector<AMDTProfileCounterDesc>& counterDesc);

extern AMDTResult AMDTGetSamplingConfiguration(AMDTProfileReaderHandle readerHandle,
                                               AMDTUInt32 counterId,
                                               AMDTProfileSamplingConfig& pCounterDesc);

//extern AMDTResult AMDTGetReportConfigurations(AMDTProfileReaderHandle readerHandle,
//                                       gtVector<AMDTProfileReportConfig>& viewConfigDesc);

extern AMDTResult AMDTSetDebugInfoPaths(AMDTProfileReaderHandle readerHandle,
                                        gtVector<gtString>& symbolServer,
                                        gtVector<gtString>& symbolDirectory);

extern AMDTResult AMDTSetSourcePaths(AMDTProfileReaderHandle readerHandle,
                                     gtVector<gtString>& sourceDirPath);

extern AMDTResult AMDTSetReportOptions(AMDTProfileReaderHandle readerHandle,
                                       AMDTProfileDataOptions& options);

extern AMDTResult AMDTSetReportOption(AMDTProfileReaderHandle readerHandle,
                                      AMDTReportOptionType type,
                                      gtUInt64 value);

extern AMDTResult AMDTSetReportCounters(AMDTProfileReaderHandle readerHandle,
                                        gtVector<AMDTUInt32> countersList);

// If counter list is empty all the counters pertaining to to the view-config will be selected
//extern AMDTResult AMDTSetReportConfiguration(AMDTProfileReaderHandle readerHandle,
//                                             AMDTUInt64 coreMask,
//                                             bool seperateByCore,
//                                             gtVector<AMDTUInt32> countersList); // NOT REQD?

extern AMDTResult AMDTGetProcessInfo(AMDTProfileReaderHandle readerHandle,
                                     gtVector<AMDTProfileProcessInfo>& procInfo);

extern AMDTResult AMDTGetModuleInfo(AMDTProfileReaderHandle readerHandle,
                                    gtVector<AMDTProfileModuleInfo>& procInfo);

extern AMDTResult AMDTGetThreadInfo(AMDTProfileReaderHandle readerHandle,
                                    gtVector<AMDTProfileThreadInfo>& procInfo);

extern AMDTResult AMDTGetProcessSummary(AMDTProfileReaderHandle readerHandle,
                                        AMDTUInt32 counterId,
                                        size_t count,
                                        gtVector<AMDTProfileData>& processProfileData);

extern AMDTResult AMDTGetThreadSummary(AMDTProfileReaderHandle readerHandle,
                                       AMDTUInt32 counterId,      // This sampleConfigId
                                       size_t count,
                                       gtVector<AMDTProfileData>& processProfileData);

extern AMDTResult AMDTGetModuleSummary(AMDTProfileReaderHandle readerHandle,
                                       AMDTUInt32 counterId,      // This sampleConfigId
                                       size_t count,
                                       gtVector<AMDTProfileData>& processProfileData);

extern AMDTResult AMDTGetFunctionSummary(AMDTProfileReaderHandle readerHandle,
                                         AMDTUInt32 counterId,      // This sampleConfigId
                                         size_t count,
                                         gtVector<AMDTProfileData>& processProfileData);

//extern AMDTResult AMDTGetProcessInfo(AMDTProfileReaderHandle readerHandle,
//                              AMDTUInt32 pid,
//                              gtVector<AMDTProfileProcessInfo>& procInfo);
//
//extern AMDTResult AMDTGetModuleInfo(AMDTProfileReaderHandle readerHandle,
//                             AMDTUInt32 pid,
//                             AMDTModuleId mid,
//                             gtVector<AMDTProfileModuleInfo>& moduleInfo);
//
//extern AMDTResult AMDTGetThreadInfo(AMDTProfileReaderHandle readerHandle,
//                             AMDTUInt32 pid,
//                             AMDTThreadId tid,
//                             gtVector<AMDTProfileThreadInfo>& threadInfo);

// Sorting will be on the first counter
//extern AMDTResult AMDTGetProfileData(AMDTProfileReaderHandle readerHandle,
//                              AMDTProfileDataType type,
//                              AMDTProcessId processId,  // can be AMDT_CP_ALL_PROCESSES for process summary
//                              AMDTModuleId moduleId,  // can be AMDT_CP_ALL_MODULES
//                              AMDTThreadId threadId,  // can be AMDT_CP_ALL_THREAD
//                              // AMDTUInt64 coreMask,   // can be AMDT_CP_ALL_CORES
//                              AMDTUInt32 counterId,  // if AMDT_CP_ALL_COUNTERS, by default all raw counters
//                                                     // otherwise provides all counters set by AMDTSetViewCounters
//                              size_t count,     // AMDT_CP_MAX_COUNT - returns all the entries
//                              gtVector<AMDTProfileData>& profileData);

extern AMDTResult AMDTGetFunctionProfileData(AMDTProfileReaderHandle   readerHandle,
                                             AMDTFunctionId            funcId,
                                             AMDTProcessId             processId,
                                             AMDTThreadId              threadId,
                                             AMDTProfileFunctionData&  functionData);

extern AMDTResult AMDTGetFunctionDisassembly(AMDTProfileReaderHandle readerHandle,
                                             AMDTFunctionId functionId,
                                             AMDTUInt32 lineNbr,
                                             gtVector<gtString>& disasmString);

#endif // _AMDTPROFILEDATAACCESS_H_