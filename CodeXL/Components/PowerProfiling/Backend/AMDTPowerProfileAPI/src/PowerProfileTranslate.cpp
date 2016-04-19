//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileTranslate.cpp
///
//==================================================================================

#include "PowerProfileTranslate.h"
#include <RawDataReader.h>
#include <PowerProfileHelper.h>
#include <AMDTPowerProfileControl.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTPwrProfAttributes.h>
#include <AMDTSharedBufferConfig.h>
#include <AMDTHistogram.h>
#include <cstring>
#include <AMDTDriverTypedefs.h>
#include <math.h>
#include <stdarg.h>
#include <list>
struct ContextData;

AMDTUInt32 g_tracePids = 0;

PowerData   g_aggrPidPowerList;

AMDTUInt32 g_markerCnt = 0;

PwrInstrumentedPowerData* g_pInstrumentedPowerData = nullptr;

static std::list <ProcessName> g_processNames;

// Memory pool for translation layer
static MemoryPool g_transPool;

static AMDTFloat32 g_totalApuPower = 0;
#define MICROSEC_PER_SEC 1000000
#define MILLISEC_PER_SEC 1000

#ifdef LINUX
    // Linux driver return TS is micro seconds
    const AMDTFloat32 g_countPerSeconds = static_cast<AMDTFloat32>(MICROSEC_PER_SEC);
#else
    // Win driver return TS is milli seconds
    const AMDTFloat32 g_countPerSeconds = static_cast<AMDTFloat32>(MILLISEC_PER_SEC);
#endif

extern AMDTUInt8* g_pSharedBuffer;
PowerProfileTranslate::PowerProfileTranslate()
{
    m_sampleOffset = 0;
    m_configList.m_cfgCnt = 0;
    m_consumedRecId = 0;

    // Platform id
    m_platformId = PLATFORM_INVALID;
}
PowerProfileTranslate::~PowerProfileTranslate()
{
    //Close the raw file and cleanup some memory. We don't need any more
    //TODO: need to check  m_rawFileHld->Close();
}

void PowerProfileTranslate::ClosePowerProfileTranslate()
{
    m_isRunning =  false;

    PwrTrace("PID Collected %d", g_tracePids);
    //TRACE//
    {
        AMDTUInt32 cnt = 0;

        for (cnt = 0; cnt < g_markerCnt; cnt++)
        {
            PwrTrace("markerid %d\n\
                          pid %d \n\
                          name %s\n\
                          buffer %s\n\
                          pwr %f\n\
                          name %s",
                     g_pInstrumentedPowerData[cnt].m_markerId,
                     g_pInstrumentedPowerData[cnt].m_data.m_pidInfo.m_pid,
                     g_pInstrumentedPowerData[cnt].m_data.m_name,
                     g_pInstrumentedPowerData[cnt].m_data.m_userBuffer,
                     g_pInstrumentedPowerData[cnt].m_data.m_pidInfo.m_power,
                     g_pInstrumentedPowerData[cnt].m_data.m_pidInfo.m_name);
        }
    }

    ReleaseMemoryPool(&g_transPool);

    m_configList.m_pCfg = NULL;

    // Close the m_rawFileHld
    if (NULL != m_rawFileHld)
    {
        // Cleanup the buffer list from previous run
        m_rawFileHld->ReleaseBufferList();
        delete m_rawFileHld; // Destructor calls close
    }

    m_rawFileHld = NULL;
}

//InitPowerProfiler - Initialize power profiler
AMDTResult PowerProfileTranslate::InitPowerTranslate(const wchar_t* pFileName, AMDTUInt64 flag)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    SectionSampleInfo sampleInfo;
    bool isOnline = (flag & 0x01) ? true : false;

    // Create the memory pool for translation layer
    ret = CreateMemoryPool(&g_transPool, TRANSLATION_POOL_SIZE);

    if (AMDT_STATUS_OK == ret)
    {
        m_rawFileHld = new RawDataReader();

        if (!m_rawFileHld)
        {
            ret = AMDT_ERROR_FAIL;
        }
    }

    //Initialize raw file
    if (AMDT_STATUS_OK == ret)
    {
        ret = m_rawFileHld->RawDataInitialize(pFileName, isOnline);
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetTargetSystemInfo(&m_sysInfo);

        if (AMDT_STATUS_OK == ret)
        {
            m_targetCoreCnt = m_sysInfo.m_coreCnt;

            if (m_sysInfo.m_isAmdApu)
            {
                m_targetCuCnt = m_sysInfo.m_computeUnitCnt;
                m_coresPerCu = m_targetCoreCnt / m_targetCuCnt;
            }
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = PrepareAttrList();
    }

    //Allocate the processed list for sample records
    if (AMDT_STATUS_OK == ret)
    {
        ret = m_rawFileHld->GetSampleSectionInfo(&sampleInfo);

        m_platformId = m_sysInfo.m_platformId;
    }

    if (AMDT_STATUS_OK == ret)
    {
        m_pSahredBuffer = (SharedBuffer*)GetMemoryPoolBuffer(&g_transPool, m_targetCoreCnt * sizeof(SharedBuffer));

        // Reset PID power
        memset(&g_aggrPidPowerList, 0, sizeof(PowerData));
        memset(m_samplePower, 0, sizeof(m_samplePower));

        // Initialize histogram counters
        InitializeHistogram();

        // Initialize TS parameters
        m_prevTs = 0.0;
        m_currentTs = 0.0;

        // Instrumented power data
        g_pInstrumentedPowerData = (PwrInstrumentedPowerData*)GetMemoryPoolBuffer(&g_transPool, PWR_MAX_MARKER_CNT * sizeof(PwrInstrumentedPowerData));
        memset(g_pInstrumentedPowerData, 0, PWR_MAX_MARKER_CNT * sizeof(PwrInstrumentedPowerData));
        g_markerCnt = 0;
    }

    m_isRunning =  true;

    // Reset previous Smu sample value list
    memset(m_prevSmuSampleData, 0, MAX_PREV_COUNTERS * sizeof(AMDTPwrAttributeInfo));
    //allocate the buffer list again
    m_rawFileHld->PrepareRawBufferList((AMDTUInt16)m_targetCoreCnt);
    return ret;
}

// GetConfigCoreMask: get the core mask set for profiling
AMDTResult PowerProfileTranslate::GetConfigCoreMask(AMDTUInt64* pMask)
{
    AMDTResult ret = AMDT_STATUS_OK;
    ProfileConfigList cfgTab;
    ProfileConfig cfg[MAX_CONFIG_CNT];
    cfgTab.m_profileConfig = &cfg[0];
    AMDTUInt32 cnt = 0;
    AMDTUInt64 cfgMask = 0;

    m_rawFileHld->GetProfileCfgInfo(&cfgTab);

    for (cnt = 0; cnt < cfgTab.m_configCnt; cnt++)
    {
        ProfileConfig* pCfg = cfgTab.m_profileConfig + cnt;

        if (NULL == pCfg)
        {
            ret = AMDT_ERROR_FAIL;
            break;
        }

        cfgMask |= pCfg->m_samplingSpec.m_mask;
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pMask = cfgMask;
    }

    return ret;
}

AMDTResult PowerProfileTranslate::PrepareAttrList()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt16 cnt = 0;
    AMDTUInt32 id = 0;
    AMDTUInt32 bitPos = 0;
    ProfileConfigList cfgTab;
    ProfileConfig cfg[MAX_CONFIG_CNT];
    cfgTab.m_profileConfig = &cfg[0];
    ProfileConfig* pCfg = NULL;
    SampleConfig* pListCfg = NULL;

    m_rawFileHld->GetProfileCfgInfo(&cfgTab);

    m_configList.m_cfgCnt = (AMDTUInt16)cfgTab.m_configCnt;

    m_configList.m_pCfg = (SampleConfig*)GetMemoryPoolBuffer(&g_transPool,
                                                             sizeof(SampleConfig) * (m_targetCoreCnt + 1));

    if (!m_configList.m_cfgCnt || !m_configList.m_pCfg)
    {
        ret = AMDT_ERROR_FAIL;
    }

    if (AMDT_STATUS_OK == ret)
    {
        pCfg = cfgTab.m_profileConfig;
        pListCfg = m_configList.m_pCfg;

        // Store the profile sampling rate and profile type
        m_profileType = (ProfileType)pCfg->m_samplingSpec.m_profileType;
        m_samplingPeriod = (AMDTUInt32)pCfg->m_samplingSpec.m_samplingPeriod;

        if (PROFILE_TYPE_PROCESS_PROFILING == m_profileType)
        {
            m_recIdFactor = m_samplingPeriod;
        }
        else
        {
            m_recIdFactor = 1;
        }

        for (cnt = 0; cnt < cfgTab.m_configCnt; cnt++)
        {
            AMDTUInt32 smuCnt = 0;
            AMDTUInt32 sampleId = pCfg->m_sampleId;
            pListCfg[sampleId].m_configId = cnt;
            pListCfg[sampleId].m_sampleId = pCfg->m_sampleId;
            pListCfg[sampleId].m_attrCnt = pCfg->m_attrCnt;
            memset(pListCfg[sampleId].m_counter, 0, sizeof(AMDTUInt16) * MAX_COUNTER_CNT);

            //prepare the list
            for (bitPos = 0; bitPos < COUNTERID_APU_MAX_CNT; bitPos++)
            {
                if (pCfg->m_apuCounterMask & (1ULL << bitPos))
                {
                    pListCfg[sampleId].m_counter[id] = (AMDTUInt16)bitPos;
                    id = id + 1;

                }
            }

            for (smuCnt = 0; smuCnt < pCfg->m_activeList.m_count; smuCnt++)
            {
                bitPos = 0;
                SmuInfo* pSmu = &pCfg->m_activeList.m_info[smuCnt];

                if ((NULL != pSmu) && (APU_SMU_ID != pSmu->m_packageId))
                {
                    AMDTUInt64 mask = pSmu->m_counterMask;

                    do
                    {
                        if (mask & 0x01)
                        {
                            AMDTUInt32 idx = bitPos
                                             + DGPU_COUNTER_BASE_ID
                                             + (pSmu->m_packageId - APU_SMU_ID - 1)
                                             * DGPU_COUNTERS_MAX;

                            pListCfg[sampleId].m_counter[id] = (AMDTUInt16)idx;
                            id = id + 1;
                        }

                        bitPos++;
                        mask = mask >> 1;
                    }
                    while (mask);
                }
            }

            id = 0;

            pCfg++;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        PrepareInitialProcessList(g_processNames);
    }

    return ret;
}

//GetAttributeDetails: Get the details of a given counter
AMDTResult GetAttributeDetails(AMDTPwrAttributeTypeInfo** pInfo, AMDTUInt32 backendIdx)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    *pInfo = nullptr;

    for (cnt = 0; cnt < g_attributeList.attrCnt; cnt++)
    {
        AMDTPwrAttributeTypeInfo* pData = g_attributeList.pAttrList + cnt;

        if (pData->m_attrId == backendIdx)
        {
            *pInfo = pData;
            ret = AMDT_STATUS_OK;
            break;
        }
    }

    if ((cnt == g_attributeList.attrCnt) || (nullptr == *pInfo))
    {
        //couldn't find the client id
        ret = AMDT_ERROR_FAIL;
    }

    return ret;
}

//GetAttributeLength
AMDTResult PowerProfileTranslate::GetAttributeLength(AMDTUInt32 attrId, AMDTUInt32 coreCnt, AMDTUInt32* pLen)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if ((attrId < COUNTERID_MUST_BASE) || (attrId > COUNTERID_MAX_CNT))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {

        switch (attrId)
        {
            case COUNTERID_SMU7_APU_PWR_CU:
            case COUNTERID_SMU7_APU_TEMP_CU:
            case COUNTERID_SMU7_APU_TEMP_MEAS_CU:
            case COUNTERID_SMU8_APU_PWR_CU:
            case COUNTERID_SMU8_APU_TEMP_CU:
            case COUNTERID_SMU8_APU_TEMP_MEAS_CU:
            case COUNTERID_SMU8_APU_C0STATE_RES:
            case COUNTERID_SMU8_APU_C1STATE_RES:
            case COUNTERID_SMU8_APU_CC6_RES:
            case COUNTERID_SMU8_APU_PC6_RES:
            {
                // Max 2 CUs are considered
                // For kaveri CU = 2
                // For Mullin CU = 1
                // size depends on number of CUs
                *pLen = m_targetCuCnt;
                break;
            }

            case COUNTERID_SVI2_CORE_TELEMETRY:
            case COUNTERID_SVI2_NB_TELEMETRY:
            {
                *pLen = SVI2_ATTR_VALUE_CNT;
                break;
            }

            case COUNTERID_PID:
            case COUNTERID_TID:
            case COUNTERID_CEF:
            case COUNTERID_PSTATE:
            case COUNTERID_CSTATE_RES:
            {
                *pLen = (1 * coreCnt);
                break;
            }

            default:
            {
                //All other attributes are of size 1
                *pLen = 1;
                break;
            }
        }
    }

    return ret;
}

// GetProcessedList: Get the latest processed data from the list
AMDTResult PowerProfileTranslate::GetProcessedList(AMDTPwrProcessedDataRecord* pData)
{
    AMDTResult ret = AMDT_STATUS_OK;

    std::unique_lock<std::mutex> lock(m_lock);

    if (!m_processedData.empty())
    {
        while (m_processedData.empty())
        {
            m_condition.wait(lock);
        }

        if (nullptr != pData)
        {
            memcpy(pData, &m_processedData.front(), sizeof(AMDTPwrProcessedDataRecord));
            m_processedData.pop();
        }
        else
        {
            ret = AMDT_ERROR_INTERNAL;
            PwrTrace("pData = NULL");
        }
    }
    else
    {
        ret = AMDT_ERROR_NODATA;
        //PwrTrace("data not available");
    }

    return ret;
}

// ProcessMarkerRecords: process marker records
void PowerProfileTranslate::ProcessMarkerRecords(AMDTUInt8* pRaw, AMDTUInt32 bufferSize, AMDTUInt32* pOffset)
{
    AMDTUInt32 markerLen = PWR_MARKER_BUFFER_SIZE + sizeof(RawRecordHdr);
    RawRecordHdr hdr;
    MarkerTag marker;
    AMDTFloat32 ts = 0;

    memset(&hdr, 0, sizeof(RawRecordHdr));

    while (1)
    {
        if ((*pOffset + markerLen) <= bufferSize)
        {
            memcpy(&hdr, (RawRecordHdr*)(pRaw + *pOffset), sizeof(RawRecordHdr));
        }

        if (REC_TYPE_MARKER_DATA == hdr.m_recordType)
        {
            // Activate the instrument base profile marker
            *pOffset += sizeof(RawRecordHdr);
            memcpy(&marker, (MarkerTag*)&pRaw[*pOffset], sizeof(MarkerTag));

            *pOffset += sizeof(MarkerTag);

            if ((0 != marker.m_markerId) && (g_markerCnt < PWR_MAX_MARKER_CNT))
            {
                if (PWR_MARKER_ENABLE == marker.m_state)
                {
                    g_pInstrumentedPowerData[g_markerCnt].m_state = PWR_MARKER_ENABLE;
                    g_pInstrumentedPowerData[g_markerCnt].m_markerId = marker.m_markerId;
                    g_pInstrumentedPowerData[g_markerCnt].m_data.m_pidInfo.m_pid = marker.m_pid;
                    SetElapsedTime(marker.m_ts, &ts);
                    g_pInstrumentedPowerData[g_markerCnt].m_data.m_startTs = (AMDTUInt64)ts;
                    memcpy(g_pInstrumentedPowerData[g_markerCnt].m_data.m_name, marker.m_name, PWR_MARKER_BUFFER_SIZE);
                    memcpy(g_pInstrumentedPowerData[g_markerCnt].m_data.m_userBuffer, marker.m_userBuffer, PWR_MARKER_BUFFER_SIZE);
                    g_markerCnt++;
                }
                else if (PWR_MARKER_DISABLE == marker.m_state)
                {
                    AMDTUInt32 loop = 0;

                    for (loop = 0 ; loop < g_markerCnt; loop++)
                    {
                        if (g_pInstrumentedPowerData[loop].m_markerId == marker.m_markerId)
                        {
                            g_pInstrumentedPowerData[loop].m_state = PWR_MARKER_DISABLE;
                            SetElapsedTime(marker.m_ts, &ts);
                            g_pInstrumentedPowerData[g_markerCnt].m_data.m_startTs = (AMDTUInt64)ts;
                        }
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
}

// TranslateRawData: Read the shared buffer and decode the raw data
AMDTResult PowerProfileTranslate::TranslateRawData()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt64 coreMask = 0;
    AMDTUInt32 coreCnt = 0;
    bool dataAvailable = true;
    AMDTUInt32 cnt = 0;
    AMDTUInt64 tempCoreMask = 0;

    if (false == m_isRunning)
    {
        return 0;
    }

    //Get Core mask configured
    ret = GetConfigCoreMask(&coreMask);

    if (AMDT_STATUS_OK == ret)
    {
        coreCnt = GetBitsCount(coreMask);

        if (0 == coreCnt)
        {
            PwrTrace("no configured core");
            ret = AMDT_ERROR_FAIL;
        }
    }


    if (AMDT_STATUS_OK == ret)
    {
        memset(&m_data, 0, sizeof(AMDTPwrProcessedDataRecord));
        ret = ReadSharedBuffer(m_pSahredBuffer, coreMask);
    }

    if (AMDT_STATUS_OK == ret)
    {
        while (dataAvailable & m_isRunning)
        {
            AMDTUInt32 counterIdx = 0;


            if (AMDT_STATUS_OK != ret)
            {
                break;
            }

            tempCoreMask =  coreMask;

            for (cnt = 0; cnt < coreCnt; cnt++)
            {
                AMDTUInt16 sampleId = 0;
                AMDTUInt64 recId = 0;
                AMDTUInt32 attrCnt = 0;
                AMDTUInt32 attrIdx = 0;
                AMDTUInt16* pCounters = nullptr;
                SharedBuffer* pCoreBuffer = nullptr;
                RawRecordHdr hdr;

                AMDTUInt8* pRaw = nullptr;
                AMDTUInt32 offset = 0;
                AMDTUInt32 coreId = 0;
                AMDTUInt64 tsRaw = 0;

                if (true == m_isRunning)
                {
                    GetFirstSetBitIndex(&coreId, (AMDTUInt32)tempCoreMask);
                    tempCoreMask &= ~(1 << coreId);
                    pCoreBuffer = m_pSahredBuffer + coreId;
                    offset = pCoreBuffer->m_processedOffset;
                    pRaw = (AMDTUInt8*)(pCoreBuffer->m_pBuffer);
                    memcpy(&hdr, (RawRecordHdr*)(pRaw + offset), sizeof(RawRecordHdr));
                    offset += sizeof(RawRecordHdr);
                }
                else
                {
                    ret = AMDT_ERROR_FAIL;
                    break;
                }

                if (REC_TYPE_SAMPLE_DATA == hdr.m_recordType)
                {
                    // Read basic information
                    sampleId = *(AMDTUInt16*)(pRaw + offset);
                    offset += sizeof(AMDTUInt16);
                    recId = *(AMDTUInt64*)(pRaw + offset);
                    offset += sizeof(AMDTUInt64);
                    tsRaw = *(AMDTUInt64*)(pRaw + offset);
                    offset += sizeof(AMDTUInt64);

                    // Fill basic information only for master buffer
                    if (0 == cnt)
                    {
                        m_data.m_recId = recId / m_recIdFactor;
                        m_data.m_recordType = (AMDTPwrProcessedDataType)hdr.m_recordType;
                        SetElapsedTime(tsRaw, &m_currentTs);
                        m_data.m_ts = (AMDTUInt64)m_currentTs;
                    }

                    attrCnt = m_configList.m_pCfg[sampleId].m_attrCnt;
                    pCounters = m_configList.m_pCfg[sampleId].m_counter;

                    // Skip sample id, record id and ts
                    pCounters += 3;
                    attrCnt -= 3;

                    // Decode the counters as per sample id
                    for (attrIdx = 0; attrIdx < attrCnt; attrIdx++)
                    {
                        AMDTUInt32 len = 0;
                        AMDTUInt32 dataLen = 0;
                        AMDTPwrAttributeInfo* pInfo = &m_data.m_attr[counterIdx];
                        pInfo->m_instanceId = 0;
                        DecodeData(coreId,
                                   pRaw,
                                   &offset,
                                   *(pCounters + attrIdx),
                                   pInfo,
                                   &dataLen);

                        GetAttributeLength(*(pCounters + attrIdx), 1, &len);
                        counterIdx += len;
                    }

                    m_data.m_attrCnt = counterIdx;

                    if (cnt == (coreCnt - 1))
                    {
                        std::lock_guard<std::mutex> lock(m_lock);
                        m_prevTs = m_currentTs;
                        m_processedData.push(m_data);
                        m_condition.notify_one();
                        memset(&m_data, 0, sizeof(AMDTPwrProcessedDataRecord));
                    }
                }
                else if (REC_TYPE_CONTEXT_DATA == hdr.m_recordType)
                {
                    AMDTFloat32 ipc = 0;
                    ContextData contextData;
                    AMDTUInt32 pidCnt = 0;
                    AMDTUInt32 markerIdx = 0;
                    bool found = false;
                    AMDTUInt32 cuId = coreId / 2;
                    memcpy(&recId, (AMDTUInt64*)&pRaw[offset], sizeof(AMDTUInt64));
                    offset += sizeof(AMDTUInt64);
                    memcpy(&contextData, (ContextData*)&pRaw[offset], sizeof(ContextData));
                    offset += sizeof(ContextData);
                    g_tracePids++;

                    if (MAX_PID_CNT < m_samplePower[cuId].m_numberOfPids)
                    {
                        ret = AMDT_ERROR_UNEXPECTED;
                        PwrTrace("MAX_PID_CNT reached");
                    }

#ifdef LINUX
                    // TODO: PMC support for LINUX is pending
                    contextData.m_pmcData[PMC_EVENT_RETIRED_MICRO_OPS] = 1;
                    contextData.m_pmcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED] = 1;

#endif
                    // ipc = retired micro ops/ cpu cycle not halted
                    ipc = (AMDTFloat32) contextData.m_pmcData[PMC_EVENT_RETIRED_MICRO_OPS]
                          / (AMDTFloat32) contextData.m_pmcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED];

                    for (pidCnt = 0; pidCnt < m_samplePower[cuId].m_numberOfPids; pidCnt++)
                    {
                        if (contextData.m_processId == m_samplePower[cuId].m_process[pidCnt].m_pid)
                        {
                            m_samplePower[cuId].m_process[pidCnt].m_sampleCnt++;
                            m_samplePower[cuId].m_process[pidCnt].m_ipc += ipc;
                            m_samplePower[cuId].m_totalIpc += ipc;
                            m_samplePower[cuId].m_sampleCnt++;
                            found = true;
                            break;
                        }
                    }

                    if (false == found)
                    {
                        m_samplePower[cuId].m_numberOfPids++;
                        m_samplePower[cuId].m_process[pidCnt].m_pid = contextData.m_processId;
                        m_samplePower[cuId].m_process[pidCnt].m_sampleCnt++;
                        m_samplePower[cuId].m_process[pidCnt].m_ipc += ipc;
                        m_samplePower[cuId].m_totalIpc += ipc;
                        m_samplePower[cuId].m_sampleCnt++;
                    }

                    for (markerIdx = 0; markerIdx < g_markerCnt; markerIdx++)
                    {
                        if ((g_pInstrumentedPowerData[markerIdx].m_data.m_pidInfo.m_pid == contextData.m_processId)
                            && ((PWR_MARKER_ENABLE == g_pInstrumentedPowerData[markerIdx].m_state)
                                || (PWR_MARKER_DISABLE_INITIATED == g_pInstrumentedPowerData[markerIdx].m_state)))
                        {
                            g_pInstrumentedPowerData[markerIdx].m_cuPidInst[cuId]++;
                            //PwrTrace("inst %d",g_pInstrumentedPowerData[markerIdx].m_cuPidInst[cuId]);
                        }
                    }
                }

                ProcessMarkerRecords(pRaw, pCoreBuffer->m_size, &offset);

                // Update the process buffer offset
                pCoreBuffer->m_processedOffset = offset;

                if ((pCoreBuffer->m_processedOffset + hdr.m_recordLen) > pCoreBuffer->m_size)
                {
                    // This core buffer doesn't have enough raw data to decode next record
                    dataAvailable = false;
                }
            }
        }

        if (true == m_isRunning)
        {
            // update the shared buffer consumption info and exit
            UpdateSharedBufferInfo(m_pSahredBuffer, coreMask);
        }
    }

    return ret;
}

// GetVDDAPower: This function is specific to SMU7 and it depends of socket types used
AMDTFloat32 GetVDDAPower()
{
    AMDTFloat32 vdda = 0.0;
    return vdda;
}

// SetElapsedTime: Elapse time from the time when first record was collected
void PowerProfileTranslate::SetElapsedTime(AMDTUInt64 raw, AMDTFloat32* pResult)
{
    AMDTUInt64 perfCounter = m_rawFileHld->GetSessionProfileStartPerfCounter();
#ifdef LINUX
    const unsigned int nsec_to_microSec = 1000;
    *pResult = static_cast<AMDTUInt64>(raw - perfCounter) / nsec_to_microSec;
#else
    AMDTUInt64 perfFreq = m_rawFileHld->GetSessionPerfFreq();
    *pResult = static_cast<AMDTFloat32>((AMDTFloat32)((raw - perfCounter) * 1000) / (AMDTFloat32)perfFreq);
#endif
}


//DecodeRegisters -decode register values to meaningful data
AMDTResult PowerProfileTranslate::DecodeDgpuData(AMDTUInt8* pData,
                                                 AMDTUInt32* pOffset,
                                                 AMDTUInt32 counterId,
                                                 AMDTPwrAttributeInfo* pInfo,
                                                 AMDTUInt32* pLen)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTFloat32 res = 0.0;
    PwrCategory category = CATEGORY_POWER;
    AMDTUInt32 counterIdx = counterId - DGPU_COUNTER_BASE_ID;
    counterIdx = counterIdx % DGPU_COUNTERS_MAX;
    AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
    AMDTUInt32 limit = 0;

    if (nullptr != pInfo)
    {
        GetAttributeDetails(&pInfo->m_pInfo, counterId);
    }
    else
    {
        ret = AMDT_ERROR_INVALIDDATA;
        PwrTrace("pInfo is null");
    }

    if (nullptr != pInfo->m_pInfo)
    {
        category = pInfo->m_pInfo->m_category;
    }
    else
    {
        ret = AMDT_ERROR_INVALIDDATA;
        PwrTrace("pInfo->m_pInfo is null");
    }

    if (AMDT_STATUS_OK == ret)
    {
        DECODE_SMU7_RAW_DATA(data, res, category);

        limit = (COUNTERID_FREQ_DGPU == counterIdx) ? MAX_GPU_FREQUENCY : MAX_POWER;

        if (res > limit)
        {
            res = GetPrevSampleData(counterId, 0);
        }

        pInfo->u.m_float32 = res;
        pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
        pInfo->m_instanceId = ((counterId - DGPU_COUNTER_BASE_ID) / DGPU_COUNTERS_MAX) + APU_SMU_ID + 1;
        SaveCurrentSampleData(pInfo);

        if (COUNTERID_PKG_PWR_DGPU == counterIdx)
        {
            AddToCumulativeCounter(counterId, 0, (res * (m_currentTs - m_prevTs) / g_countPerSeconds));
        }
        else if (COUNTERID_FREQ_DGPU == counterIdx)
        {
            AddToHistogram(counterId, 0, res);
        }

        *pLen = sizeof(AMDTFloat32);
        *pOffset += sizeof(AMDTUInt32);
    }

    return ret;
}

// GetProcessName: Get process name
void PowerProfileTranslate:: GetProcessName(AMDTPwrProcessInfo* pInfo)
{
    std::list<ProcessName>::iterator itr;
    bool found = false;
    bool refreshed = false;

    if (nullptr != pInfo)
    {
        memset(pInfo->m_name, 0, AMDT_PWR_EXE_NAME_LENGTH);
        memset(pInfo->m_name, 0, AMDT_PWR_EXE_PATH_LENGTH);

        do
        {
            if (g_processNames.size() > 0)
            {
                for (itr = g_processNames.begin(); itr != g_processNames.end(); ++itr)
                {
                    if (itr->m_pid == pInfo->m_pid)
                    {
                        memcpy(pInfo->m_name, itr->m_name, strlen(itr->m_name));
                        memcpy(pInfo->m_path, itr->m_path, strlen(itr->m_path));
                        found = true;
                        break;
                    }
                }
            }

            if (found == false)
            {
                found = GetProcessNameFromPid(pInfo);

                if (false == found)
                {
                    if (false == refreshed)
                    {
                        // Prepare the list again
                        PrepareInitialProcessList(g_processNames);
                        refreshed = true;
                    }
                    else
                    {
                        memcpy(pInfo->m_name, ERROR_READING_PROCESS_NAME, strlen(ERROR_READING_PROCESS_NAME) + 1);
                        memcpy(pInfo->m_path, ERROR_READING_PROCESS_PATH, strlen(ERROR_READING_PROCESS_PATH) + 1);
                        break;
                    }
                }
            }
        }
        while (false == found);
    }
}

// CalculatePidPower: Calculate power indicators for each PID and agreegate the indicator values
void PowerProfileTranslate::CalculatePidPower(AMDTFloat32* cuPower, AMDTUInt32 cuCnt)
{
    AMDTUInt32 cnt = 0;
    AMDTUInt32 cnt1 = 0;
    AMDTUInt32 cuIdx = 0;
    AMDTUInt32 pidIdx = 0;
    PowerData* pSrc = nullptr;
    AMDTFloat32 energyPerCuSample = 0.0;
    AMDTUInt32 markerIdx = 0;
    AMDTFloat32 timeSpan = 0;
    AMDTFloat32 load = 0;

    for (cuIdx = 0; cuIdx < cuCnt; cuIdx++)
    {
        pSrc = &m_samplePower[cuIdx];

        if (MAX_PID_CNT < pSrc->m_numberOfPids)
        {
            memset(pSrc, 0, sizeof(PowerData) * 2);
            return;
        }

        timeSpan = (m_currentTs - m_prevTs);
        energyPerCuSample = (cuPower[cuIdx] > 0) ? (cuPower[cuIdx] * timeSpan / g_countPerSeconds) : static_cast<AMDTFloat32>(0.0);

        // Calculate power indicator for each PID
        for (pidIdx = 0; pidIdx < pSrc->m_numberOfPids; pidIdx++)
        {
            load = pSrc->m_process[pidIdx].m_ipc / pSrc->m_totalIpc;
            pSrc->m_process[pidIdx].m_power = energyPerCuSample * load;
        }

        for (markerIdx = 0; markerIdx < g_markerCnt; markerIdx++)
        {
            if ((PWR_MARKER_ENABLE == g_pInstrumentedPowerData[markerIdx].m_state)
                || (PWR_MARKER_DISABLE_INITIATED == g_pInstrumentedPowerData[markerIdx].m_state))
            {
                PwrInstrumentedPowerData* pData = &g_pInstrumentedPowerData[markerIdx];

                load = pData->m_ipc[cuIdx] / pData->m_totalIpc[cuIdx];

                pData->m_data.m_pidInfo.m_power += (energyPerCuSample * load);
                pData->m_data.m_pidInfo.m_sampleCnt += pData->m_cuPidInst[cuIdx];

                if (strlen(pData->m_data.m_pidInfo.m_name) == 0)
                {
                    GetProcessName(&pData->m_data.m_pidInfo);
                }

                if (cuCnt == (cuIdx + 1))
                {
                    memset(pData->m_cuPidInst, 0, MAX_CU_CNT * sizeof(AMDTInt32));
                    memset(pData->m_ipc, 0, MAX_CU_CNT * sizeof(AMDTFloat32));
                    memset(pData->m_totalIpc, 0, MAX_CU_CNT * sizeof(AMDTFloat32));

                    if (PWR_MARKER_DISABLE_INITIATED == pData->m_state)
                    {
                        pData->m_state = PWR_MARKER_DISABLE;
                    }
                }
            }
        }
    }

    for (cuIdx = 0; cuIdx < cuCnt; cuIdx++)
    {
        pSrc = &m_samplePower[cuIdx];

        // Agreegate the power with previous samples
        for (cnt = 0; cnt < pSrc->m_numberOfPids; cnt++)
        {
            bool found = false;

            for (cnt1 = 0; cnt1 < g_aggrPidPowerList.m_numberOfPids; cnt1++)
            {
                if (g_aggrPidPowerList.m_process[cnt1].m_pid == pSrc->m_process[cnt].m_pid)
                {
                    g_aggrPidPowerList.m_process[cnt1].m_power += pSrc->m_process[cnt].m_power;
                    g_aggrPidPowerList.m_process[cnt1].m_sampleCnt += pSrc->m_process[cnt].m_sampleCnt;
                    g_aggrPidPowerList.m_sampleCnt += pSrc->m_process[cnt].m_sampleCnt;
                    g_aggrPidPowerList.m_totalIpc += pSrc->m_totalIpc;
                    g_aggrPidPowerList.m_process[cnt1].m_ipc += pSrc->m_process[cnt].m_ipc;
                    found = true;
                    break;
                }
            }

            if (false == found)
            {
                // Add new entry to the list
                g_aggrPidPowerList.m_process[cnt1].m_pid = pSrc->m_process[cnt].m_pid;
                g_aggrPidPowerList.m_process[cnt1].m_power = pSrc->m_process[cnt].m_power;
                g_aggrPidPowerList.m_process[cnt1].m_sampleCnt += pSrc->m_process[cnt].m_sampleCnt;
                g_aggrPidPowerList.m_totalIpc += pSrc->m_totalIpc;
                g_aggrPidPowerList.m_process[cnt1].m_ipc = pSrc->m_process[cnt].m_ipc;
                g_aggrPidPowerList.m_numberOfPids ++;
                g_aggrPidPowerList.m_sampleCnt += pSrc->m_process[cnt].m_sampleCnt;

                // Fill process name
                GetProcessName(&g_aggrPidPowerList.m_process[cnt1]);
            }
        }
    }

    // Reset sample pid list
    memset(&m_samplePower, 0, sizeof(PowerData) * MAX_CU_CNT);
}

// SaveCurrentSampleData: Insert previous valid data. So that it can be used in case
// there is junk value in the next sample
bool PowerProfileTranslate::SaveCurrentSampleData(AMDTPwrAttributeInfo* pData)
{
    AMDTUInt32 cnt = 0;
    bool ret = false;

    for (cnt = 0; cnt < MAX_PREV_COUNTERS; cnt++)
    {
        if ((nullptr == m_prevSmuSampleData[cnt].m_pInfo)
            || ((pData->m_instanceId == m_prevSmuSampleData[cnt].m_instanceId)
                && (pData->m_pInfo->m_attrId == m_prevSmuSampleData[cnt].m_pInfo->m_attrId)))
        {
            memcpy(&m_prevSmuSampleData[cnt], pData, sizeof(AMDTPwrAttributeInfo));
            ret = true;
            break;
        }
    }

    return ret;
}

// GetPrevSampleData: Read previous valid record values
AMDTFloat32 PowerProfileTranslate::GetPrevSampleData(AMDTUInt32 counterId, AMDTUInt32 inst)
{
    AMDTUInt32 cnt = 0;
    AMDTFloat32 res = 0;

    PwrTrace("Spike value received for counter %d ins %d", counterId, inst);

    for (cnt = 0; cnt < MAX_PREV_COUNTERS; cnt++)
    {
        AMDTPwrAttributeInfo* pPrev = &m_prevSmuSampleData[cnt];

        if (nullptr == pPrev->m_pInfo)
        {
            res = (AMDTFloat32)0.0;
            break;
        }

        if ((pPrev->m_instanceId == inst)
            && (pPrev->m_pInfo->m_attrId == counterId))
        {
            res = pPrev->u.m_float32;
            break;
        }
    }

    return res;
}

//DecodeRegisters -decode register values to meaningful data
AMDTResult PowerProfileTranslate::DecodeData(AMDTUInt32 coreId,
                                             AMDTUInt8* pData,
                                             AMDTUInt32* pOffset,
                                             AMDTUInt32 counterId,
                                             AMDTPwrAttributeInfo* pInfo,
                                             AMDTUInt32* pLen)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTFloat32 res = 0;

    if (DGPU_COUNTER_BASE_ID <= counterId)
    {
        DecodeDgpuData(pData, pOffset, counterId, pInfo, pLen);
    }
    else
    {
        switch (counterId)
        {
            case COUNTERID_SMU7_APU_TEMP_CU:
            case COUNTERID_SMU7_APU_TEMP_MEAS_CU:
            {
                AMDTPwrAttributeInfo* info = pInfo;
                AMDTUInt32 repeat = m_targetCuCnt;
                AMDTUInt32 cuId = 0;

                while (repeat--)
                {
                    AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                    res = SMU7_PROCESS_TEPERATURE_DATA(data);

                    if (res > MAX_TEMPERATURE)
                    {
                        res = GetPrevSampleData(counterId, cuId);
                    }

                    info->u.m_float32 = res;
                    GetAttributeDetails(&info->m_pInfo, counterId);
                    info->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CU;
                    info->m_instanceId = cuId++;
                    SaveCurrentSampleData(info);
                    *pLen = sizeof(AMDTFloat32);
                    *pOffset += sizeof(AMDTUInt32);

                    if (repeat)
                    {
                        info++;
                    }
                }

                break;
            }

            case COUNTERID_SMU8_APU_TEMP_CU:
            case COUNTERID_SMU8_APU_TEMP_MEAS_CU:
            case COUNTERID_SMU8_APU_C0STATE_RES:
            case COUNTERID_SMU8_APU_C1STATE_RES:
            case COUNTERID_SMU8_APU_CC6_RES:
            case COUNTERID_SMU8_APU_PC6_RES:
            {
                AMDTPwrAttributeInfo* info = pInfo;
                AMDTUInt32 repeat = m_targetCuCnt;
                AMDTUInt32 cuId = 0;

                while (repeat--)
                {
                    AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                    SMU8_PROCESS_RAWDATA(res, data);
                    info->u.m_float32 = res;
                    GetAttributeDetails(&info->m_pInfo, counterId);
                    info->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CU;
                    info->m_instanceId = cuId++;
                    *pLen = sizeof(AMDTFloat32);
                    *pOffset += sizeof(AMDTUInt32);

                    if (repeat)
                    {
                        info++;
                    }
                }

                break;
            }

            case COUNTERID_SMU7_APU_PWR_CU:
            case COUNTERID_SMU8_APU_PWR_CU:
            {
                AMDTPwrAttributeInfo* info = pInfo;
                AMDTUInt32 repeat = m_targetCuCnt;
                AMDTUInt32 cuId = 0;
                AMDTFloat32 cuPower[MAX_CU_CNT];

                while (repeat--)
                {
                    AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);

                    if (COUNTERID_SMU7_APU_PWR_CU == counterId)
                    {
                        res = SMU7_PROCESS_POWER_DATA(data);
                    }
                    else if (COUNTERID_SMU8_APU_PWR_CU == counterId)
                    {
                        SMU8_PROCESS_RAWDATA(res, data);
                    }

                    if (res > MAX_POWER)
                    {
                        res = GetPrevSampleData(counterId, cuId);
                    }

                    cuPower[cuId] = res;
                    g_totalApuPower += res;
                    info->u.m_float32 = res;
                    GetAttributeDetails(&info->m_pInfo, counterId);
                    info->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CU;
                    info->m_instanceId = cuId++;
                    SaveCurrentSampleData(info);
                    AddToCumulativeCounter(counterId, info->m_instanceId,
                                           (res * (m_currentTs - m_prevTs) / g_countPerSeconds));
                    *pLen = sizeof(AMDTFloat32);
                    *pOffset += sizeof(AMDTUInt32);

                    if (repeat)
                    {
                        info++;
                    }
                }

                if (PROFILE_TYPE_PROCESS_PROFILING == m_profileType)
                {
                    CalculatePidPower(cuPower, cuId);
                }

                break;
            }

            case COUNTERID_SMU8_APU_PWR_VDDGFX:
            case COUNTERID_SMU8_APU_PWR_APU:
            case COUNTERID_SMU8_APU_PWR_VDDIO:
            case COUNTERID_SMU8_APU_PWR_VDDNB:
            case COUNTERID_SMU8_APU_PWR_VDDP:
            case COUNTERID_SMU8_APU_PWR_UVD:
            case COUNTERID_SMU8_APU_PWR_VCE:
            case COUNTERID_SMU8_APU_PWR_ACP:
            case COUNTERID_SMU8_APU_PWR_UNB:
            case COUNTERID_SMU8_APU_PWR_SMU:
            case COUNTERID_SMU8_APU_PWR_ROC:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                SMU8_PROCESS_RAWDATA(res, data);
                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                AddToCumulativeCounter(counterId, 0, (res * (m_currentTs - m_prevTs) / g_countPerSeconds));
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU8_APU_TEMP_VDDGFX:
            case COUNTERID_SMU8_APU_TEMP_MEAS_VDDGFX:
            case COUNTERID_SMU8_APU_FREQ_ACLK:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                SMU8_PROCESS_RAWDATA(res, data);
                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU8_APU_FREQ_IGPU:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                SMU8_PROCESS_RAWDATA(res, data);
                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                AddToHistogram(counterId, pInfo->m_instanceId, res);
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU7_APU_PWR_IGPU:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                res = SMU7_PROCESS_POWER_DATA(data);

                if (res > MAX_POWER)
                {
                    res = GetPrevSampleData(counterId, 0);
                }

                pInfo->u.m_float32 = res;
                g_totalApuPower += res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                SaveCurrentSampleData(pInfo);
                AddToCumulativeCounter(counterId, 0, (res * (m_currentTs - m_prevTs) / g_countPerSeconds));
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU7_APU_PWR_PCIE:
            case COUNTERID_SMU7_APU_PWR_DDR:
            case COUNTERID_SMU7_APU_PWR_DISPLAY:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                res = SMU7_PROCESS_POWER_DATA(data);

                if (res > MAX_POWER)
                {
                    res = GetPrevSampleData(counterId, 0);
                }

                pInfo->u.m_float32 = res;
                g_totalApuPower += res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                SaveCurrentSampleData(pInfo);
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU7_APU_PWR_PACKAGE:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                res = SMU7_PROCESS_POWER_DATA(data);

                // Package power supplied by SMU is discarded for SMU7
                if ((COUNTERID_SMU7_APU_PWR_PACKAGE == counterId) && (PLATFORM_KAVERI == m_platformId))
                {
                    res = g_totalApuPower;
                    g_totalApuPower = 0;
                }

                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                pInfo->m_instanceId = 0;
                AddToCumulativeCounter(counterId,
                                       0,
                                       (res * (m_currentTs - m_prevTs) / g_countPerSeconds));
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU7_APU_TEMP_IGPU:
            {
                //TODO: Need to pack all temperature values
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                res = SMU7_PROCESS_TEPERATURE_DATA(data);

                if (res > MAX_TEMPERATURE)
                {
                    res = GetPrevSampleData(counterId, 0);
                }

                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                SaveCurrentSampleData(pInfo);
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU7_APU_TEMP_MEAS_IGPU:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                res = SMU7_PROCESS_TEPERATURE_DATA(data);

                if (res > MAX_TEMPERATURE)
                {
                    res = GetPrevSampleData(counterId, 0);
                }

                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                pInfo->m_instanceId = 0;
                SaveCurrentSampleData(pInfo);
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SMU7_APU_FREQ_IGPU:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                res = SMU7_PROCESS_FREQUENCY_DATA(data);

                if (res > MAX_GPU_FREQUENCY)
                {
                    res = GetPrevSampleData(counterId, 0);
                }

                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;

                if (counterId >= DGPU_COUNTER_BASE_ID)
                {
                    pInfo->m_instanceId = ((counterId - DGPU_COUNTER_BASE_ID) / DGPU_COUNTERS_MAX) + APU_SMU_ID + 1;
                }

                SaveCurrentSampleData(pInfo);

                AddToHistogram(counterId, pInfo->m_instanceId, res);

                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_PID:
            {
                AMDTUInt64 data = *(AMDTUInt64*)(pData + *pOffset);
                AMDTPwrAttributeInfo* info = pInfo;

                //Get PID
                pInfo->u.m_value64 = data;
                *pLen = sizeof(AMDTUInt64);
                GetAttributeDetails(&info->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE;
                info->m_instanceId = coreId;
                *pOffset += sizeof(AMDTUInt64);
                break;
            }

            case COUNTERID_TID:
            {
                AMDTUInt64 data = *(AMDTUInt64*)(pData + *pOffset);
                AMDTPwrAttributeInfo* info = pInfo;
                //Get TID
                pInfo->u.m_value64 = data;
                *pLen = sizeof(AMDTUInt64);
                GetAttributeDetails(&info->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE;
                info->m_instanceId = coreId;
                *pOffset += sizeof(AMDTUInt64);

                break;
            }

            case COUNTERID_SAMPLE_CALLCHAIN:
            {
                break;
            }

            case COUNTERID_CEF:
            {
                AMDTUInt64 aPerf = 0;
                AMDTUInt64 mPerf = 0;
                AMDTUInt64 pState = 0;
                AMDTFloat64 pStateFreq = 0;
                AMDTFloat32 cef = 0;

                aPerf = *(AMDTUInt64*)(pData + *pOffset);
                *pOffset += sizeof(AMDTUInt64);

                mPerf = *(AMDTUInt64*)(pData + *pOffset);
                *pOffset += sizeof(AMDTUInt64);

                pState = *(AMDTUInt64*)(pData + *pOffset);
                *pOffset += sizeof(AMDTUInt64);

                pStateFreq = (AMDTFloat64)(100.0 * (AMDTFloat64)((pState & AMDT_CPUFID_MASK) + 0x10) /
                                           (AMDTFloat64)(0x1 << ((pState & AMDT_CPUDID_MASK) >> AMDT_CPUDID_BITSHIFT))); // in MHz

                AMDTPwrApuPstateList* pStates = GetApuPStateInfo();
                AMDTFloat64 maxRatio = (AMDTFloat64)pStates->m_stateInfo[0].m_frequency / pStateFreq ;

                if (((AMDTFloat64)aPerf) > ((AMDTFloat64)(maxRatio * mPerf)))
                {
                    aPerf = mPerf;
                }

                cef = (AMDTFloat32)(((AMDTFloat64)aPerf / (AMDTFloat64)mPerf) * pStateFreq);
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE;
                pInfo->m_instanceId = coreId;
                pInfo->u.m_float32 = cef;
                AddToHistogram(counterId, coreId, cef);
                break;
            }

            case COUNTERID_CSTATE_RES:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                *pLen = sizeof(AMDTUInt32);
                pInfo->u.m_value64 = (AMDTUInt64)data;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE;
                pInfo->m_instanceId = coreId;
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_PSTATE:
            {
                res = *(AMDTFloat32*)(pData + *pOffset);
                *pLen = sizeof(AMDTUInt32);
                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE;
                pInfo->m_instanceId = coreId;
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_NODE_TCTL_TEPERATURE:
            {
                AMDTUInt32 data = *(AMDTUInt32*)(pData + *pOffset);
                DecodeTctlTemperature(data, &res);
                pInfo->u.m_float32 = res;
                GetAttributeDetails(&pInfo->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_NONCORE_SINGLE;
                *pLen = sizeof(AMDTFloat32);
                *pOffset += sizeof(AMDTUInt32);
                break;
            }

            case COUNTERID_SVI2_CORE_TELEMETRY:
            case COUNTERID_SVI2_NB_TELEMETRY:
            {
                AMDTPwrAttributeInfo* info = pInfo;
                AMDTUInt32 reg = 0;
                AMDTFloat32 v = 0;
                AMDTFloat32 c = 0;
                AMDTUInt32 idx = 0;
                //Reference Spike current is considered as 120A
                AMDTFloat32 IddSpikeOCP = 120.0;

                //Voltage
                reg = *(AMDTUInt32*)((AMDTUInt8*)pData + *pOffset);
                v = (AMDTFloat32)((reg & 0x1ff0000) >> 16);
                v = (AMDTFloat32)3.15 - v * (AMDTFloat32)0.00625;
                pInfo->u.m_float32 = v;
                GetAttributeDetails(&info->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE_MULTIVALUE;
                info->m_instanceId = idx++;
                info++;

                //Current
                c = (AMDTFloat32)(reg & 0xff);
                c = (IddSpikeOCP * c) / (AMDTFloat32)255.0;
                pInfo->u.m_float32 = c;
                *pLen = sizeof(AMDTFloat32);
                GetAttributeDetails(&info->m_pInfo, counterId);
                pInfo->m_pInfo->m_instanceType = INSTANCE_TYPE_PER_CORE_MULTIVALUE;
                *pOffset += sizeof(AMDTUInt32);
                info->m_instanceId = idx;

                break;
            }

            default:
                ret = AMDT_ERROR_FAIL;
                break;
        }
    }

    return ret;
}


// ReadSharedBuffer: Read shared buffer between driver and backend
// and populate per core buffers for processing.
AMDTResult PowerProfileTranslate::ReadSharedBuffer(SharedBuffer* pBuffer, AMDTUInt64 coreMask)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 sharedOffset = PWRPROF_SHARED_METADATA_SIZE;
    AMDTUInt64 tempCoreMask = coreMask;
    AMDTUInt32 coreId = 0;

    if (AMDT_STATUS_OK == ret)
    {
        while (tempCoreMask)
        {
            GetFirstSetBitIndex(&coreId, (AMDTUInt32)tempCoreMask);
            tempCoreMask &= ~(1 << coreId);
            SharedBuffer* pSBuffer = &pBuffer[coreId];

            AMDTUInt32 bufferBase = sharedOffset + coreId * (sizeof(PageBuffer) + PWRPROF_PERCORE_BUFFER_SIZE);
            AMDTUInt32 rawBufferbase = sharedOffset + (coreId + 1) * sizeof(PageBuffer) + coreId * PWRPROF_PERCORE_BUFFER_SIZE;
            PageBuffer* pCoreBuffer = (PageBuffer*)(g_pSharedBuffer + bufferBase);
            pSBuffer->m_size = 0;

            ATOMIC_SET(&pSBuffer->m_currentOffset, pCoreBuffer->m_currentOffset);
            ATOMIC_SET(&pSBuffer->m_consumedOffset, pCoreBuffer->m_consumedOffset);
            ATOMIC_SET(&pSBuffer->m_maxValidOffset, pCoreBuffer->m_maxValidOffset);

            if (pSBuffer->m_currentOffset > pSBuffer->m_consumedOffset)
            {
                AMDTUInt32 bufferSize = pSBuffer->m_currentOffset - pSBuffer->m_consumedOffset;
                pSBuffer->m_pBuffer = (AMDTUInt8*)&g_pSharedBuffer[rawBufferbase + pSBuffer->m_consumedOffset];
                pSBuffer->m_size = bufferSize;
                pSBuffer->m_processedOffset = 0;
            }
            else if ((pSBuffer->m_currentOffset < pSBuffer->m_consumedOffset)
                     && (pSBuffer->m_consumedOffset < pSBuffer->m_maxValidOffset))
            {
                AMDTUInt32 bufferSize = pSBuffer->m_maxValidOffset - pSBuffer->m_consumedOffset;
                pSBuffer->m_pBuffer = (AMDTUInt8*)&g_pSharedBuffer[rawBufferbase + pSBuffer->m_consumedOffset];
                pSBuffer->m_size = bufferSize;
                pSBuffer->m_processedOffset = 0;
            }
            else if ((pSBuffer->m_currentOffset < pSBuffer->m_consumedOffset)
                     && (pSBuffer->m_consumedOffset == pSBuffer->m_maxValidOffset))
            {
                AMDTUInt32 bufferSize = 0;
                // Processed till max length, start from 0 again
                ATOMIC_SET(&pCoreBuffer->m_maxValidOffset, 0);
                ATOMIC_SET(&pCoreBuffer->m_consumedOffset, 0);
                pSBuffer->m_consumedOffset = 0;
                pCoreBuffer->m_consumedOffset = 0;
                ATOMIC_SET(&pSBuffer->m_maxValidOffset, pCoreBuffer->m_maxValidOffset);
                bufferSize = pSBuffer->m_currentOffset - 0;
                pSBuffer->m_pBuffer = (AMDTUInt8*)&g_pSharedBuffer[rawBufferbase + 0];
                pSBuffer->m_size = bufferSize;
                pSBuffer->m_processedOffset = 0;

            }
            else
            {
                //PwrTrace("else: core %d cons %d curr %d max %d",cnt,  pSBuffer->m_consumedOffset, pSBuffer->m_currentOffset, pSBuffer->m_maxValidOffset);
            }

            if (0 == pSBuffer->m_size)
            {
                ret = AMDT_ERROR_NODATA;
                //PwrTrace("Shared buffer length = 0, core %d ", cnt);
                break;
            }
        }
    }

    return ret;
}

// UpdateSharedBufferInfo: update the buffer meta data once processing is done
AMDTResult PowerProfileTranslate::UpdateSharedBufferInfo(SharedBuffer* pBuffer, AMDTUInt64 coreMask)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 sharedOffset = PWRPROF_SHARED_METADATA_SIZE;
    AMDTUInt64 tempCoreMask = coreMask;
    AMDTUInt32 coreId = 0;

    while (tempCoreMask)
    {
        GetFirstSetBitIndex(&coreId, (AMDTUInt32)tempCoreMask);
        tempCoreMask &= ~(1 << coreId);
        AMDTUInt32 offset = 0;
        SharedBuffer* localBuffer = pBuffer + coreId;


        AMDTUInt32 bufferBase = sharedOffset + coreId * (sizeof(PageBuffer) + PWRPROF_PERCORE_BUFFER_SIZE);
        volatile PageBuffer* pCoreBuffer = (PageBuffer*)(g_pSharedBuffer + bufferBase);

        offset = localBuffer->m_processedOffset + localBuffer->m_consumedOffset;

        if (offset == localBuffer->m_maxValidOffset)
        {
            ATOMIC_SET(&pCoreBuffer->m_maxValidOffset, 0);
            ATOMIC_SET(&pCoreBuffer->m_consumedOffset, 0);
        }
        else if ((offset <= PWRPROF_PERCORE_BUFFER_SIZE))
        {
            ATOMIC_SET(&pCoreBuffer->m_consumedOffset, offset);
        }
        else
        {

            ret = AMDT_ERROR_INTERNAL;
            PwrTrace("Wrong buffer size offset %d, size %d m_processedOffset %d, consumed %d max %d", offset,
                     localBuffer->m_size,
                     localBuffer->m_processedOffset,
                     localBuffer->m_consumedOffset,
                     localBuffer->m_maxValidOffset);
            break;
        }
    }

    return ret;
}

// GetInstrumentedData: Get the intrumented power for the selected marker
AMDTResult PowerProfileTranslate::GetInstrumentedData(AMDTUInt32 markerId, PwrInstrumentedPowerData** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 idx = 0;
    bool found = false;

    if (nullptr != ppData)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    for (idx = 0; idx < g_markerCnt; idx++)
    {
        if (g_pInstrumentedPowerData[idx].m_markerId == markerId)
        {
            *ppData = &g_pInstrumentedPowerData[idx];
            found = true;
            break;
        }
    }

    if (false == found)
    {
        ret = AMDT_ERROR_MARKER_NOT_SET;
    }

    return ret;
}

