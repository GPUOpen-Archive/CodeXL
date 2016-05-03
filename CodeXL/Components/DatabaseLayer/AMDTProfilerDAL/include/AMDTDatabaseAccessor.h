//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTDatabaseAccessor.h
///
//==================================================================================

#ifndef _AMDTDATABASEACCESSOR_H_
#define _AMDTDATABASEACCESSOR_H_

#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTProfilerDAL/include/AMDTProfileDALDataTypes.h>
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

#if defined(_WIN32)
    #if defined(AMDTPROFILEDAL_EXPORTS)
        #define AMDTPROFILEDAL_API __declspec(dllexport)
    #else
        #define AMDTPROFILEDAL_API __declspec(dllimport)
    #endif
#else
    #define AMDTPROFILEDAL_API
#endif

#define AMDT_CURRENT_PROFILE_DB_VERSION     1
#define AMDT_PROFILE_ALL_CALLPATHS  0xFFFFFFFFUL

namespace AMDTProfilerDAL
{

class AMDTPROFILEDAL_API AmdtDatabaseAccessor
{
public:
    AmdtDatabaseAccessor();
    ~AmdtDatabaseAccessor();

    // Prevent copy, assignment
    AmdtDatabaseAccessor(const AmdtDatabaseAccessor& other) = delete;
    AmdtDatabaseAccessor& operator=(const AmdtDatabaseAccessor& other) = delete;

    //
    // DB Control APIs
    //

    // Creates a DB for CPU/Power/both profiler.
    // profileType is a bit mask to create tables for CPU, Power  or both profilers
    bool CreateProfilingDatabase(const gtString& dbName, gtUInt64 profileType);

    // This function opens a database for read.
    // It must be called before any other of the following function is called.
    // Arguments: const gtString& dbName - a name of an already created database.
    // Note: if this function fails, the behavior of any of the following functions is undefined.
    bool OpenProfilingDatabase(const gtString& dbName, gtUInt64 profileType, bool isReadOnly = true);

    // Used to migrate the DB generated by CodeXL 1.9 (db user_version 0).
    // This renames the tables and add columns etc
    bool MigrateProfilingDatabase(const gtString& dbName);

    // Commits all pending transactions to the DB.
    bool FlushData(void);

    // Commits all pending transactions to the DB asynchronously.
    bool FlushDataAsync(void);

    // Closes all the DB connections.
    bool CloseAllConnections(void);

    bool GetDbVersion(int& version);

    //
    // DB Update/Insert APIs
    //

    // Inserts session info to the DB.
    bool InsertSessionInfoKeyValue(const gtString& keyStr, const gtString& valueStr);
    bool InsertSessionInfo(const gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec);

    // Inserts a batch of power profiling samples to the DB.
    bool InsertSamples(const gtVector<PPSampleData>& samples);

    // Inserts a device to the DB.
    bool InsertDevice(const AMDTProfileDevice& device);

    // Inserts a power profiling counter as meta-data to the DB.
    bool InsertCounter(const AMDTProfileCounterDesc& counterDescription);

    // Sets the counter represented by counterId as enabled as of quantizedTime.
    bool InsertCounterControl(int counterId, int quantizedTime, std::string& action);

    // Sets the session sampling interval to samplingIntervalMs as of quantizedTime.
    bool InsertSamplingInterval(unsigned samplingIntervalMs, unsigned quantizedTime);

    bool InsertCoreInfo(gtUInt32 coreId, gtUInt32 processorId, gtUInt32 numaNodeId);
    bool InsertSamplingCounter(gtUInt32 eventId, gtString name, gtString description);
    bool InsertSamplingConfig(gtUInt32 id, gtUInt16 counterId, gtUInt64 samplingInterval, gtUInt16 unitMask, bool isUserMode, bool isOsMode, bool edge);
    bool InsertCoreSamplingConfig(gtUInt64 id, gtUInt16 coreId, gtUInt32 samplingConfigId);
    bool InsertProcessInfo(gtUInt64 pid, const gtString& path, bool is32Bit);
    bool InsertModuleInfo(gtUInt32 id, const gtString& path, bool isSystemModule, bool is32Bit, gtUInt32 type, gtUInt32 size, bool foundDebugInfo);
    bool InsertModuleInstanceInfo(gtUInt32 moduleInstanceId, gtUInt32 moduleId, gtUInt64 pid, gtUInt64 loadAddr);
    bool InsertProcessThreadInfo(gtUInt64 id, gtUInt64 pid, gtUInt64 threadId);
    bool InsertSamples(CPSampleData& sampleData);
    bool InsertFunction(gtUInt32 funcId, gtUInt32 modId, const gtString& funcName, gtUInt64 offset, gtUInt64 size);

    //
    //  Update Queries
    //  Used to migrate the database generated by CodeXL 1.9 in CodeXL 2.0 (db version 1)
    //

    bool UpdateDeviceTypeId(const gtMap<gtString, int>& deviceInfo);
    bool UpdateCounterCategoryId(const gtMap<gtString, int>& counterInfo);
    bool UpdateCounterAggregationId(const gtMap<gtString, int>& counterInfo);
    bool UpdateCounterUnitId(const gtMap<gtString, int>& counterInfo);

    //
    // DB Query APIs
    //

    // Retrieves the sessioninfo
    bool GetSessionInfoValue(const gtString& key, gtString& infoValue);
    bool GetAllSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec);

    // Utility function that retrieves the AMDTDeviceType for the device with id deviceId.
    bool GetDeviceType(int deviceId, std::string& deviceType);
    bool GetDeviceType(int deviceId, int& deviceType);

    bool GetDeviceTypeByCounterId(int counterId, std::string& deviceType);
    bool GetDeviceTypeByCounterId(int counterId, int& deviceType);

    // Fetch the counter names.
    bool GetCounterNames(gtMap<gtString, int>& counterNames);

    // Utility function that retrieves the details of all of the counters which were enabled during the session.
    // TODO: map may not be reqd..
    bool GetCountersDescription(gtMap<int, AMDTProfileCounterDesc*>& counterDetails);

    // Fetch the name of the counter based on counter id
    bool GetCounterIdByName(std::string& counterName, int& counterId);

    // Utility function that retrieves the counters which were enabled during the session and that
    // have category of type counterCategory.
    bool GetCountersByCategory(std::string& counterCategoryStr, gtVector<int>& counterIds);
    bool GetCountersByCategory(int& counterCategoryId, gtVector<int>& counterIds);

    // Utility function that retrieves the counters which were enabled during the session and that
    // are linked to devices of type deviceType and have category of type counterCategory.
    bool GetCountersByDeviceAndCategory(std::string& deviceType, std::string& counterCategory, gtVector<int>& counterIds);
    bool GetCountersByDeviceAndCategory(int deviceTypeId, int counterCategoryId, gtVector<int>& counterIds);

    // Retrieves the sampling interval for the current session.
    bool GetSamplingInterval(unsigned& samplingInterval);

    // Retrieves the time points when the session began and ended.
    bool GetSamplesTimeRange(SamplingTimeRange& samplingTimeRange);

    // Retrieves the samples grouped by counter ids
    // Output args: gtMap<int, double>& samplesMap.
    bool GetSamplesGroupByCounterId(const gtVector<int>& counterIds, gtMap<int, double>& samplesMap);

    // Retrieves the samples grouped by counter id
    // TODO: not yet implemented
    // bool GetSamplesGroupByCounterId(const int counterId, double& samplesMap);

    // Calculates a bucketed samples for the specified counters from the beginning of the session.
    // TODO: revisit this function
    bool GetBucketizedSamplesByCounterId(unsigned int bucketWidth,
                                         const gtVector<int>& counterIds,
                                         gtVector<int>& cIds, gtVector<double>& dbBucketBottoms,
                                         gtVector<int>& dbBucketCount);

    // TODO: not yet implemented
    // bool GetSamplesByCounterIdAndRange(int counterId, SamplingTimeRange& samplingTimeRange,
    //                                   gtVector<SampledValue>& sampledValuesPerCounter);

    // Retrieves the sampled values for each counter in the specified time range.
    // Output args: gtVector<double>>& sampledValuesPerCounter.
    bool GetSamplesByCounterIdAndRange(const gtVector<int>& counterIds,
                                       SamplingTimeRange& samplingTimeRange,
                                       gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter);

    // Retrieves the global maximum and minimum of sampled values in the specified range.
    // Output args: double& minValue, double& maxValue.
    bool GetMinMaxSampleByCounterId(const gtVector<int>& counterIds,
                                    SamplingTimeRange& samplingTimeRange,
                                    double& minValue,
                                    double& maxValue);

    // Retrieves the overall number of samples which were taken throughout the session for each of the specified counters.
    bool GetSampleCountByCounterId(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter);

    bool GetCpuTopology(gtVector<AMDTCpuTopology>& cpuTopology);
    bool GetSampledCountersList(gtVector<AMDTProfileCounterDesc>& counterDesc);
    bool GetSamplingConfiguration(AMDTUInt32 counterId, AMDTProfileSamplingConfig& samplingConfig);
    bool GetProcessInfo(AMDTUInt32 pid, gtVector<AMDTProfileProcessInfo>& processInfoList);
    bool GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, gtVector<AMDTProfileModuleInfo>& moduleInfoList);
    bool GetThreadInfo(AMDTUInt32 pid, gtUInt32 tid, gtVector<AMDTProfileThreadInfo>& threadInfoList);

    bool GetProcessTotals(AMDTProcessId               procId,
        gtVector<AMDTUInt32>        counterIdsList,
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        AMDTSampleValueVec&         sampleValueVec);

    bool GetModuleTotals(AMDTModuleId                moduleId,
        AMDTProcessId               processId,
        gtVector<AMDTUInt32>        counterIdsList,
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        AMDTSampleValueVec&         sampleValueVec);

    bool GetFunctionTotals(AMDTFunctionId         funcId,
        AMDTProcessId          processId,
        AMDTThreadId           threadId,
        gtVector<AMDTUInt32>&  counterIdsList,
        AMDTUInt64             coreMask,
        bool                   separateByCore,
        AMDTSampleValueVec&    sampleValueVec);

    bool GetProcessSummaryData(AMDTProcessId               processId,
                               gtVector<AMDTUInt32>        counterIdsList,      // samplingConfigId
                               AMDTUInt64                  coreMask,
                               bool                        separateByCore,
                               bool                        doSort,
                               size_t                      count,
                               gtVector<AMDTProfileData>&  dataList);

    bool GetModuleSummaryData(AMDTProcessId               processId,           // for a given process or for all processes
                              AMDTModuleId                moduleId,
                              gtVector<AMDTUInt32>        counterIdsList,      // samplingConfigId
                              AMDTUInt64                  coreMask,
                              bool                        separateByCore,
                              bool                        doSort,
                              size_t                      count,
                              gtVector<AMDTProfileData>&  dataList);

    bool GetThreadSummaryData(AMDTProcessId               processId,           // for a given process or for all processes
                              AMDTThreadId                threadId,
                              gtVector<AMDTUInt32>        counterIdsList,      // samplingConfigId
                              AMDTUInt64                  coreMask,
                              bool                        separateByCore,
                              bool                        doSort,
                              size_t                      count,
                              gtVector<AMDTProfileData>&  dataList);

    bool GetFunctionSummaryData(AMDTProcessId               processId,           // for a given process or for all processes
                                AMDTThreadId                threadId,
                                AMDTModuleId                moduleId,
                                gtVector<AMDTUInt32>        counterIdsList,      // samplingConfigId
                                AMDTUInt64                  coreMask,
                                bool                        separateByCore,
                                bool                        separateByProcess,
                                bool                        doSort,
                                size_t                      count,
                                gtVector<AMDTProfileData>&  dataList);

    bool GetFunctionProfileData(AMDTFunctionId              funcId,
                                AMDTProcessId               processId,
                                AMDTThreadId                threadId,
                                gtVector<AMDTUInt32>        counterIdsList,
                                AMDTUInt64                  coreMask,
                                bool                        separateByCore,
                                AMDTProfileFunctionData&    functionData);

    bool GetCallstackLeafData(AMDTProcessId       processId,
                              AMDTUInt32          counterId,
                              gtUInt32            callStackId,
                              CallstackFrameVec&  leafs);

    bool GetCallstackFrameData(AMDTProcessId       processId,
                               gtUInt32            callstackId,
                               CallstackFrameVec&  frames);

    bool GetCallstackIds(AMDTProcessId        processId,
                         AMDTFunctionId       funcId,
                         gtVector<gtUInt32>&  csIds);

private:
    class Impl;
    Impl* m_pImpl;
};

};
#endif // _AMDTDATABASEACCESSOR_H_