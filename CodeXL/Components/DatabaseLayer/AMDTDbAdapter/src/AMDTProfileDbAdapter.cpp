//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTProfileDbAdapter.cpp
///
//==================================================================================

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local.
#include <AMDTProfileDbAdapter.h>

//
//  Insert APIs
//

bool amdtProfileDbAdapter::InsertAllSessionInfo(
    const AMDTProfileSessionInfo& sessionInfo,
    unsigned profilingIntervalMs,
    const gtVector<AMDTProfileDevice*>& deviceList,
    const gtVector<AMDTProfileCounterDesc*>& counterList,
    const gtVector<int>& enabledCounters)
{
    bool ret = false;
    int quantizedTime = 0;

    if (m_pDbAccessor != nullptr)
    {
        // Begin a new transaction.
        m_pDbAccessor->FlushData();

        ret = InsertSessionInfo(sessionInfo);

        if (ret)
        {
            ret = InsertDevice(deviceList);
        }

        if (ret)
        {
            ret = InsertCounters(counterList);
        }

        GT_IF_WITH_ASSERT(ret)
        {
            // FIXME: Why do we need quantizedTime?
            // Anyways the sampling interval cannot be changed on the fly.. absolutely no need..
            ret = m_pDbAccessor->InsertSamplingInterval(profilingIntervalMs, quantizedTime);
        }

        GT_IF_WITH_ASSERT(ret)
        {
            // Document the enabled counters.
            for (const int counterId : enabledCounters)
            {
                std::string action("E");
                m_pDbAccessor->InsertCounterControl(counterId, quantizedTime, action);
            }
        }

        // Commit the transaction.
        m_pDbAccessor->FlushData();
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertDevice(const AMDTProfileDevice& device)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->InsertDevice(device);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertDevice(const gtVector<AMDTProfileDevice*>& deviceList)
{
    bool ret = false;

    // Insert the devices
    for (const AMDTProfileDevice* pDevice : deviceList)
    {
        if (nullptr != pDevice)
        {
            ret = m_pDbAccessor->InsertDevice(*pDevice);
        }
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertCounters(const gtVector<AMDTProfileCounterDesc*>& countersList)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        for (const AMDTProfileCounterDesc* pCounterDesc : countersList)
        {
            ret = m_pDbAccessor->InsertCounter(*pCounterDesc);
        }
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertCounterEnabled(int counterId, int quantizedTime)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        // Insert the data to the DB.
        std::string action("E");
        ret = m_pDbAccessor->InsertCounterControl(counterId, quantizedTime, action);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertCounterDisabled(int counterId, int quantizedTime)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        // Insert the data to the DB.
        std::string action("D");
        ret = m_pDbAccessor->InsertCounterControl(counterId, quantizedTime, action);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertSamplingInterval(unsigned samplingIntervalMs, unsigned quantizedTime)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->InsertSamplingInterval(samplingIntervalMs, quantizedTime);
    }
    return ret;
}

bool amdtProfileDbAdapter::InsertSamples(const gtVector<AMDTProfileTimelineSample*>& ppSamples)
{
    bool ret = false;
    gtVector<PPSampleData> dbSamples;

    for (AMDTProfileTimelineSample* pSample : ppSamples)
    {
        PrepareTimelineSamplesToInsert(pSample, dbSamples);
    }

    if (m_pDbAccessor != nullptr)
    {
        // Insert the data to the DB.
        ret = m_pDbAccessor->InsertSamples(dbSamples);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertTopology(const CPAdapterTopologyMap& topologyMap)
{
    bool ret = false;

    for (const auto& t : topologyMap)
    {
        ret = m_pDbAccessor->InsertCoreInfo(t.m_coreId, t.m_processor, t.m_numaNode);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertSamplingEvents(AMDTProfileCounterDescVec& events, const AMDTProfileSamplingConfigVec& configVec)
{
    bool ret = false;

    for (const auto& config : configVec)
    {
        bool edge = false;
        ret = m_pDbAccessor->InsertSamplingConfig(config.m_id,
                                                  static_cast<AMDTUInt16>(config.m_hwEventId),
                                                  config.m_samplingInterval,
                                                  config.m_unitMask,
                                                  config.m_userMode,
                                                  config.m_osMode,
                                                  edge);
    }

    for (const auto& event : events)
    {
        ret = m_pDbAccessor->InsertSamplingCounter(event.m_hwEventId, event.m_name, event.m_abbrev, event.m_description);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertProcessInfo(const CPAProcessList& procesList)
{
    bool ret = false;

    for (const auto& p : procesList)
    {
        ret = m_pDbAccessor->InsertProcessInfo(p.m_pid, p.m_name, p.m_is32Bit, p.m_hasCSS);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertModuleInfo(const CPAModuleList& moduleList)
{
    bool ret = false;

    for (const auto& mod : moduleList)
    {
        ret = m_pDbAccessor->InsertModuleInfo(mod.m_id, mod.m_name, mod.m_isSysModule, mod.m_is32Bit, mod.m_type, mod.m_size, mod.m_foundDebugInfo);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertModuleInstanceInfo(const CPAModuleInstanceList& moduleInstanceList)
{
    bool ret = false;

    for (const auto& modInst : moduleInstanceList)
    {
        ret = m_pDbAccessor->InsertModuleInstanceInfo(modInst.m_id, modInst.m_moduleId, modInst.m_pid, modInst.m_loadAddr);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertProcessThreadInfo(const CPAProcessThreadList& procThreadList)
{
    bool ret = false;

    for (const auto& it : procThreadList)
    {
        ret = m_pDbAccessor->InsertProcessThreadInfo(it.m_ptId, it.m_processId, it.m_threadId);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertCoreSamplingConfigInfo(const CPACoreSamplingConfigList& coreConfigList)
{
    bool ret = false;

    for (const auto& it : coreConfigList)
    {
        ret = m_pDbAccessor->InsertCoreSamplingConfig(it.m_id, it.m_coreId, it.m_samplingConfigId);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertFunctionInfo(const CPAFunctionInfoList& funcList)
{
    bool ret = false;

    for (const auto& func : funcList)
    {
        ret = m_pDbAccessor->InsertFunction(func.m_funcId, func.m_modId, func.m_funcName, func.m_funcStartOffset, func.m_funcSize);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertSamples(const CPASampeInfoList& sampleList)
{
    bool ret = false;

    for (const auto& it : sampleList)
    {
        CPSampleData sampleData;

        sampleData.m_processThreadId = it.m_processThreadId;
        sampleData.m_moduleInstanceId = it.m_moduleInstanceId;
        sampleData.m_coreSamplingConfigId = it.m_coreSamplingConfigId;
        sampleData.m_functionId = it.m_functionId;
        sampleData.m_offset = it.m_offset;
        sampleData.m_count = it.m_count;

        ret = m_pDbAccessor->InsertSamples(sampleData);
    }

    m_pDbAccessor->FlushData();

    return ret;
}

//
//  Update APIs
//

bool amdtProfileDbAdapter::UpdateDeviceTypeId(const gtMap<gtString, int>& deviceInfo)
{
    return (m_pDbAccessor != nullptr) ? m_pDbAccessor->UpdateDeviceTypeId(deviceInfo) : false;
}

bool amdtProfileDbAdapter::UpdateCounterCategoryId(const gtMap<gtString, int>& counterInfo)
{
    return (m_pDbAccessor != nullptr) ? m_pDbAccessor->UpdateCounterCategoryId(counterInfo) : false;
}

bool amdtProfileDbAdapter::UpdateCounterAggregationId(const gtMap<gtString, int>& counterInfo)
{
    return (m_pDbAccessor != nullptr) ? m_pDbAccessor->UpdateCounterAggregationId(counterInfo) : false;
}

bool amdtProfileDbAdapter::UpdateCounterUnitId(const gtMap<gtString, int>& counterInfo)
{
    return (m_pDbAccessor != nullptr) ? m_pDbAccessor->UpdateCounterUnitId(counterInfo) : false;
}

//
//  Query APIs
//

bool amdtProfileDbAdapter::GetDeviceType(int deviceId, int& deviceType) const
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetDeviceType(deviceId, deviceType);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetDeviceTypeByCounterId(int counterId, int& deviceType) const
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetDeviceTypeByCounterId(counterId, deviceType);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCounterNames(gtMap<gtString, int>& counterNames)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCounterNames(counterNames);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCounterIdByName(const gtString& counterName, int& counterId)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        std::string counterNameStr(counterName.asASCIICharArray());

        ret = m_pDbAccessor->GetCounterIdByName(counterNameStr, counterId);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCountersDescription(gtMap<int, AMDTProfileCounterDesc>& counterDetails)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        gtMap<int, AMDTProfileCounterDesc*> tempCounterDetails;

        ret = m_pDbAccessor->GetCountersDescription(tempCounterDetails);

        if (ret)
        {
            for (const auto& it : tempCounterDetails)
            {
                AMDTProfileCounterDesc counterDesc = *it.second;
                counterDetails[it.first] = counterDesc;
            }
        }
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSessionCounters(int deviceTypeId, int counterCategoryId, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCountersByDeviceAndCategory(deviceTypeId, counterCategoryId, counterIds);
    }
    return ret;
}

bool amdtProfileDbAdapter::GetSessionCounters(int counterCategory, gtVector<int>& counterIds)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCountersByCategory(counterCategory, counterIds);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSessionCounters(const gtVector<int>& deviceTypes, int counterCategoryId, gtVector<int>& counterIds)
{
    bool ret = true;

    // Clear the output vector.
    counterIds.clear();

    // This set will hold the counter IDs without duplicates.
    gtSet<int> uniqueSet;

    // Create a temporary vector to hold the current counter IDs.
    gtVector<int> tmpCountersVec;

    for (int devType : deviceTypes)
    {
        // Get the session counters.
        tmpCountersVec.clear();
        GetSessionCounters(devType, counterCategoryId, tmpCountersVec);

        // Insert the counters to the set of unique IDs.
        for (int cid : tmpCountersVec)
        {
            uniqueSet.insert(cid);
        }
    }

    // Fill our vector with the unique set of counter IDs.
    for (int cid : uniqueSet)
    {
        counterIds.push_back(cid);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSessionSamplingIntervalMs(unsigned& samplingIntervalMs)
{
    return m_pDbAccessor->GetSamplingInterval(samplingIntervalMs);
}

bool amdtProfileDbAdapter::GetSessionTimeRange(SamplingTimeRange& samplingTimeRange) const
{
    return m_pDbAccessor->GetSamplesTimeRange(samplingTimeRange);
}

bool amdtProfileDbAdapter::GetGlobalMinMaxValuesPerCounters(const gtVector<int> counterIds, SamplingTimeRange& samplingTimeRange, double& minValue, double& maxValue)
{
    return m_pDbAccessor->GetMinMaxSampleByCounterId(counterIds, samplingTimeRange, minValue, maxValue);
}

bool amdtProfileDbAdapter::GetOverallNubmerOfSamples(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter)
{
    return m_pDbAccessor->GetSampleCountByCounterId(counterIds, numberOfSamplesPerCounter);
}

bool amdtProfileDbAdapter::GetSampledValuesByRange(const gtVector<int>& counterIds, SamplingTimeRange& samplingTimeRange,
                                                   gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetSamplesByCounterIdAndRange(counterIds, samplingTimeRange, sampledValuesPerCounter);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSampleCountByCounterId(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetSampleCountByCounterId(counterIds, numberOfSamplesPerCounter);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSamplesGroupByCounterId(const gtVector<int>& counterIds, gtMap<int, double>& samplesMap)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetSamplesGroupByCounterId(counterIds, samplesMap);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCpuTopology(gtVector<AMDTCpuTopology>& cpuTopology)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCpuTopology(cpuTopology);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSampledCountersList(gtVector<AMDTProfileCounterDesc>& counterDesc)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetSampledCountersList(counterDesc);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetSamplingConfiguration(AMDTUInt32 counterId, AMDTProfileSamplingConfig& samplingConfig)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetSamplingConfiguration(counterId, samplingConfig);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetProcessInfo(AMDTUInt32 pid, gtVector<AMDTProfileProcessInfo>& processInfoList)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetProcessInfo(pid, processInfoList);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, gtVector<AMDTProfileModuleInfo>& moduleInfoList)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetModuleInfo(pid, mid, moduleInfoList);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetThreadInfo(AMDTUInt32 pid, AMDTThreadId tid, gtVector<AMDTProfileThreadInfo>& threadInfoList)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetThreadInfo(pid, tid, threadInfoList);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetProcessesWithCallstackSamples(gtVector<AMDTProcessId>& cssProcessVec)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetProcessesWithCallstackSamples(cssProcessVec);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCounterTotals(AMDTProfileDataType   type,
    AMDTProcessId         procId,
    AMDTThreadId          threadId,
    AMDTModuleId          moduleId,
    AMDTFunctionId        funcId,
    gtVector<AMDTUInt32>  counterIdsList,
    AMDTUInt64            coreMask,
    bool                  separateByCore,
    AMDTSampleValueVec&   sampleValueVec)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        switch (type)
        {
        case AMDT_PROFILE_DATA_PROCESS:
            ret = m_pDbAccessor->GetProcessTotals(procId,
                                                  counterIdsList,
                                                  coreMask,
                                                  separateByCore,
                                                  sampleValueVec);
            break;

        case AMDT_PROFILE_DATA_MODULE:
            ret = m_pDbAccessor->GetModuleTotals(moduleId,
                                                 procId,
                                                 counterIdsList,
                                                 coreMask,
                                                 separateByCore,
                                                 sampleValueVec);
            break;

        case AMDT_PROFILE_DATA_THREAD:
            break;

        case AMDT_PROFILE_DATA_FUNCTION:
            ret = m_pDbAccessor->GetFunctionTotals(funcId,
                                                   procId,
                                                   threadId,
                                                   counterIdsList,
                                                   coreMask,
                                                   separateByCore,
                                                   sampleValueVec);
            break;

        default:
            ret = false;
        }
    }

    return ret;
}

bool amdtProfileDbAdapter::GetProfileData(
    AMDTProfileDataType         type,
    AMDTProcessId               processId,
    AMDTModuleId                moduleId,
    AMDTThreadId                threadId,
    gtVector<AMDTUInt32>        counterIdList,
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    bool                        separateByProcess,  // for function summary
    bool                        doSort,
    size_t                      count,
    gtVector<AMDTProfileData>&  dataList)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        switch (type)
        {
            case AMDT_PROFILE_DATA_PROCESS:
                ret = m_pDbAccessor->GetProcessSummaryData(processId,
                                                           moduleId,
                                                           counterIdList,
                                                           coreMask,
                                                           separateByCore,
                                                           doSort,
                                                           count,
                                                           dataList);
                break;

            case AMDT_PROFILE_DATA_MODULE:
                // TODO: should we also add thread
                ret = m_pDbAccessor->GetModuleSummaryData(processId,
                                                          moduleId,
                                                          counterIdList,
                                                          coreMask,
                                                          separateByCore,
                                                          doSort,
                                                          count,
                                                          dataList);
                break;

            case AMDT_PROFILE_DATA_THREAD:
                ret = m_pDbAccessor->GetThreadSummaryData(processId,
                                                          threadId,
                                                          counterIdList,
                                                          coreMask,
                                                          separateByCore,
                                                          doSort,
                                                          count,
                                                          dataList);
                break;

            case AMDT_PROFILE_DATA_FUNCTION:
                ret = m_pDbAccessor->GetFunctionSummaryData(processId,
                                                            threadId,
                                                            moduleId,
                                                            counterIdList,
                                                            coreMask,
                                                            separateByCore,
                                                            separateByProcess,
                                                            doSort,
                                                            count,
                                                            dataList);
                break;

            default:
                ret = false;
        }
    }

    return ret;
}

bool amdtProfileDbAdapter::GetFunctionProfileData(
    AMDTFunctionId              funcId,
    AMDTProcessId               processId,
    AMDTThreadId                threadId,
    gtVector<AMDTUInt32>&       counterIdList,
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    AMDTProfileFunctionData&    functionData)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetFunctionProfileData(funcId,
                                                    processId,
                                                    threadId,
                                                    counterIdList,
                                                    coreMask,
                                                    separateByCore,
                                                    functionData);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetBucketizedSamplesByCounterId(unsigned int bucketWidth,
                                                           const gtVector<int>& counterIds,
                                                           gtVector<int>& cIds, gtVector<double>& dbBucketBottoms,
                                                           gtVector<int>& dbBucketCount)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetBucketizedSamplesByCounterId(bucketWidth, counterIds, cIds, dbBucketBottoms, dbBucketCount);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetUnknownCallstackLeafsByProcessId(AMDTProcessId processId, CallstackFrameVec& leafs)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetUnknownCallstackLeafsByProcessId(processId, leafs);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCallstackLeafData(AMDTProcessId       processId,
                                                AMDTUInt32          counterId,
                                                gtUInt32            callStackId,
                                                CallstackFrameVec&  leafs)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCallstackLeafData(processId, counterId, callStackId, leafs);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCallstackFrameData(AMDTProcessId       processId,
                                                 gtUInt32            callstackId,
                                                 CallstackFrameVec&  frames)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCallstackFrameData(processId, callstackId, frames);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetCallstackIds(AMDTProcessId        processId,
                                           AMDTFunctionId       funcId,
                                           gtVector<gtUInt32>&  csIds)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetCallstackIds(processId, funcId, csIds);
    }

    return ret;
}

bool amdtProfileDbAdapter::UpdateCallstackLeaf(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->UpdateCallstackLeaf(funcInfo);
    }

    return ret;
}

bool amdtProfileDbAdapter::UpdateCallstackFrame(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->UpdateCallstackFrame(funcInfo);
    }

    return ret;
}

bool amdtProfileDbAdapter::InsertFunctionInfo(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->InsertFunctionInfo(funcInfo);
    }

    return ret;
}

bool amdtProfileDbAdapter::GetMaxFunctionId(AMDTModuleId moduleId, gtUInt32& maxFuncId)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        ret = m_pDbAccessor->GetMaxFunctionId(moduleId, maxFuncId);
    }

    return ret;
}

//
//  Helper functions
//

void amdtProfileDbAdapter::PrepareTimelineSamplesToInsert(AMDTProfileTimelineSample* pSample, gtVector<PPSampleData>& dbSamples)
{
    for (const auto& beVal : pSample->m_sampleValues)
    {
        dbSamples.emplace_back(
            static_cast<int>(pSample->m_sampleElapsedTimeMs),
            static_cast<int>(beVal.m_counterId),
            static_cast<double>(beVal.m_counterValue));
    }
}

bool amdtProfileDbAdapter::InsertCallStackFrames(const CPACallStackFrameInfoList& csFrameInfoList)
{
    if (m_pDbAccessor != nullptr)
    {
        for (const auto& frameInfo : csFrameInfoList)
        {
            m_pDbAccessor->InsertCallStackFrame(
                frameInfo.m_callStackId,
                frameInfo.m_processId,
                frameInfo.m_funcId,
                frameInfo.m_offset,
                frameInfo.m_depth);
        }
    }
    return true;
}

bool amdtProfileDbAdapter::InsertCallStackLeafs(const CPACallStackLeafInfoList& csLeafInfoList)
{
    if (m_pDbAccessor != nullptr)
    {
        for (const auto& frameInfo : csLeafInfoList)
        {
            m_pDbAccessor->InsertCallStackLeaf(
                frameInfo.m_callStackId,
                frameInfo.m_processId,
                frameInfo.m_funcId,
                frameInfo.m_offset,
                frameInfo.m_counterId,
                frameInfo.m_selfSamples);
        }
    }
    return true;
}
