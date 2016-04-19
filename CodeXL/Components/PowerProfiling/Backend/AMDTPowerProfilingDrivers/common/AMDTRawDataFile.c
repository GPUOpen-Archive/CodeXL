//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================

#include <AMDTCounterAccessInterface.h>
#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTPwrProfAttributes.h>
#include <AMDTPwrProfInternal.h>
#include <AMDTSharedBufferConfig.h>
#include <AMDTSmu8Interface.h>
#include <AMDTSmu7Interface.h>

#define ONLINE_SECTION_HEADER_CNT (2)
#define MAX_SMU_VALUE_CNT 64
#define MAX_REPEAT_CNT 10

#ifndef LINUX
    extern PPWRPROF_DEV_EXTENSION gpPwrDevExt;
#endif

extern CoreData* g_pCoreCfg;
extern uint8* pSharedBuffer;

//WriteRawBufferHeader: Prepare and write header to the header buffer
int32 WriteRawBufferHeader(RawFileHeader* pHeader, uint16 sectionCnt)
{
    int32 ret = STATUS_SUCCESS;

    if (pHeader == NULL)
    {
        ret = STATUS_INVALID_PARAMETER;
    }
    else
    {
        pHeader->m_magicNum = RAWFILE_MAGIC;
        pHeader->m_rawFileVersion = RAW_FILE_VERSION;
        pHeader->m_sectionTabCnt = SECTION_HDR_CNT;
        pHeader->m_versionNum = PROFILE_VERSION;

        // High resolution relative timestamp
        pHeader->m_startPerfCounter = 0;
        pHeader->m_perfFreq = 0;

        //Start session should be updated when start profiling is called
        pHeader->m_sessionStart = 0;

        //End session should be updated when stop profiling is called
        pHeader->m_sessionEnd = 0;

        //Total number of core in the target system
        pHeader->m_targetCoreCuCnt = (uint16)(GetTargetCoreCount() << 8 | GetComputeUnitCntPerNode());

        //Get the family and model of the CPU
        GetCpuModelFamily((uint32*)&pHeader->m_family, (uint32*)&pHeader->m_model);

        //Record count can not be provided in  case of online profiling
        pHeader->m_rawRecordCount = 0;

        //Only one section header table is used at this moment.
        pHeader->m_tabInfo[0].m_sectionTabOff = sizeof(RawFileHeader);
        pHeader->m_tabInfo[0].m_sectionTabSize = sectionCnt * sizeof(SectionHdrInfo);
    }

    return ret;
}

//WriteSectionHeaders: Prepare and write section headers based on the section header mask
int32 WriteSectionHeaders(SectionHdrInfo* secHeader, uint64 secHeaderMask)
{
    int32 ret = STATUS_SUCCESS;
    uint32 cnt = 1;

    if (secHeader == NULL)
    {
        ret = STATUS_INVALID_PARAMETER;
    }
    else
    {
        while (cnt < RAW_FILE_SECTION_MAX - 1)
        {
            if (secHeaderMask & cnt)
            {
                secHeader->m_sectionType = cnt;
                secHeader->m_sectionOffset = 0;
                secHeader->m_sectionSize = 0;

                secHeader++;
            }

            cnt = cnt << 1;
        }
    }

    return ret;
}

//UpdateBufferHeader: Update header buffer for session start/end timestamp, record count
//field = 0 for start time update
//field = 1 for end time update
//filed =2 for record count
int32 UpdateBufferHeader(ClientData* pClient, uint16 field)
{
    int32 ret = STATUS_SUCCESS;
    PageBuffer* pBuffer;
    RawFileHeader* pHeader = NULL;

    if (NULL == pClient->m_header.m_pBuffer)
    {
        ret = STATUS_INVALID_PARAMETER;
    }
    else
    {
        pBuffer = &pClient->m_header;
        pHeader = (RawFileHeader*)pBuffer->m_pBuffer;
    }

    if ((STATUS_SUCCESS == ret) && (NULL == pHeader))
    {
        ret = STATUS_INVALID_PARAMETER;
    }
    else
    {
        switch (field)
        {

            case 0:
            {
                //update start session timestamp
                //Retrive the current time stamp when session was started
                GetTimeStamp(&pHeader->m_sessionStart);

                // Also get the high resolution performance counter and frequency
                // values for relative timestamp
                GetPerformanceCounter(&pHeader->m_startPerfCounter, &pHeader->m_perfFreq);
                break;
            }

            case 1:
            {
                //update end session timestamp
                //Retrive the current time stamp when session was stopped
                GetTimeStamp(&pHeader->m_sessionEnd);
                break;
            }

            case 2:
            {
                //update total record count
                PageBuffer* pCoreBuffer = g_pCoreCfg->m_pCoreBuffer;

                if (NULL != pCoreBuffer)
                {
                    pHeader->m_rawRecordCount = pCoreBuffer->m_recCnt;
                }
                else
                {
                    DRVPRINT("pCoreBuffer in valid");
                }

                break;
            }

            default:
            {
                ret = STATUS_INVALID_PARAMETER;
            }
        }
    }

    return ret;

}

//WriteSampleCfgInfo: Prepare and write the Sample configuration to the buffer
int32 WriteSampleCfgInfo(ClientData* pClient, ProfileConfig* pSrcCfg)
{
    int32 ret = STATUS_SUCCESS;
    SectionHdrInfo* hdrInfo = NULL;
    ProfileConfig* profConfig = NULL;
    PageBuffer* pBuffer = &pClient->m_header;
    uint32 offset = 0;
    uint32 currentOffset = 0;
    uint32 cnt = ONLINE_SECTION_HEADER_CNT;

    if (NULL == pBuffer->m_pBuffer)
    {
        ret = STATUS_INVALID_PARAMETER;
    }
    else
    {
        ATOMIC_GET(&offset, pBuffer->m_currentOffset);
        hdrInfo = (SectionHdrInfo*) & (pBuffer->m_pBuffer[sizeof(RawFileHeader)]);
        profConfig = (ProfileConfig*) & (pBuffer->m_pBuffer[offset]);
        memcpy(profConfig, pSrcCfg, sizeof(ProfileConfig));
        ATOMIC_SET(&pBuffer->m_currentOffset, offset + sizeof(ProfileConfig));

        //Update section header
        while (cnt--)
        {
            if (hdrInfo->m_sectionType == RAW_FILE_SECTION_SAMPLE_CONFIG)
            {
                hdrInfo->m_sectionOffset = offset;
                hdrInfo->m_sectionSize = currentOffset - offset;
                break;
            }

            hdrInfo++;
        }
    }

    return ret;
}

//WriteSampleInfo: This section tells the information regarding the
//first chunk offset and other details. Using the next chunk offset information
//it is easy to walk through similar type chunk records.
int32 WriteSampleInfo(ClientData* pClient)
{
    int32 ret = STATUS_SUCCESS;
    SectionHdrInfo* hdrInfo = NULL;
    SectionSampleInfo* sampleInfo = NULL;
    uint32 cnt = ONLINE_SECTION_HEADER_CNT;
    PageBuffer* pBuffer = &pClient->m_header;
    uint32 offset = 0;

    if (NULL == pBuffer)
    {
        ret = STATUS_INVALID_PARAMETER;
    }
    else
    {
        ATOMIC_GET(&offset, pBuffer->m_currentOffset);

        hdrInfo = (SectionHdrInfo*) & (pBuffer->m_pBuffer[sizeof(RawFileHeader)]);
        sampleInfo = (SectionSampleInfo*) & (pBuffer->m_pBuffer[offset]);

        sampleInfo->m_firstChunkOffset = 0;
        sampleInfo->m_recordCount = 0;
        sampleInfo->m_size = 0;
        ATOMIC_SET(&pBuffer->m_currentOffset, offset + sizeof(SectionSampleInfo));

        //Update section header
        while (cnt--)
        {
            if (hdrInfo->m_sectionType == RAW_FILE_SECTION_SAMPLE_REC_INFO)
            {
                hdrInfo->m_sectionOffset = offset;
                hdrInfo->m_sectionSize = sizeof(SectionSampleInfo);
                break;
            }

            hdrInfo++;
        }
    }

    return ret;
}

//WriteSections: Write various sections in the header buffer.
//Only two sections are added in case of online profiling at this moment
int32 WriteSections(ClientData* pClient, ProfileConfig* pSrcCfg, uint64 secHeaderMask)
{
    int32 ret = STATUS_SUCCESS;

    uint64 cnt = 1;

    while (cnt < RAW_FILE_SECTION_MAX)
    {
        switch (secHeaderMask & cnt)
        {
            case RAW_FILE_SECTION_RUN_INFO:
                break;

            case RAW_FILE_SECTION_CPU_INFO:
                break;

            case RAW_FILE_SECTION_CPU_TOPOLOGY:
                break;

            case RAW_FILE_SECTION_SAMPLE_CONFIG:
                ret = WriteSampleCfgInfo(pClient, pSrcCfg);
                break;

            case RAW_FILE_SECTION_SAMPLE_REC_INFO:
                ret = WriteSampleInfo(pClient);
                break;

            case RAW_FILE_SECTION_TI_REC_INFO:
                break;

            default:
                //Do nothing and check for next optional section
                break;
        }

        cnt = cnt << 1;
    }

    return ret;
}

//WriteHeader: Prepare and write data header
int32 WriteHeader(ClientData* pClient, ProfileConfig* pSrcCfg)
{
    int32 ret = STATUS_SUCCESS;
    PageBuffer* pBuffer = &pClient->m_header;
    RawFileHeader* pHeader = NULL;
    uint32 offset = 0;

    //Only two sections are written to the header for online profiling
    uint16 sectionCnt = ONLINE_SECTION_HEADER_CNT;
    uint64 secMask = RAW_FILE_SECTION_SAMPLE_CONFIG |
                     RAW_FILE_SECTION_SAMPLE_REC_INFO;

    if (NULL == pBuffer || NULL == pSrcCfg)
    {
        ret = STATUS_INVALID_PARAMETER;
    }

    if (STATUS_SUCCESS == ret)
    {
        // Write raw file header
        ret = WriteRawBufferHeader((RawFileHeader*)pBuffer->m_pBuffer, sectionCnt);
    }

    if (STATUS_SUCCESS == ret)
    {
        ATOMIC_GET(&offset, pBuffer->m_currentOffset);
        ATOMIC_SET(&pBuffer->m_currentOffset, offset + sizeof(RawFileHeader));
        ATOMIC_GET(&offset, pBuffer->m_currentOffset);
        // Write section headers
        ret = WriteSectionHeaders((SectionHdrInfo*) & (pBuffer->m_pBuffer[offset]), secMask);
    }

    if (STATUS_SUCCESS == ret)
    {
        ATOMIC_GET(&offset, pBuffer->m_currentOffset);
        ATOMIC_SET(&pBuffer->m_currentOffset, offset + (sectionCnt * sizeof(SectionHdrInfo)));

        pHeader = (RawFileHeader*)pBuffer->m_pBuffer;
        pHeader->m_tabInfo[0].m_sectionTabSize = ONLINE_SECTION_HEADER_CNT * sizeof(SectionHdrInfo);

        // Write sections
        ret = WriteSections(pClient, pSrcCfg, secMask);
    }

    return ret;
}

bool GetRequiredBufferLength(CoreData* pCfg, uint32* pLength)
{
    uint32 attrCnt = 0;
    uint32 bufferLen = 0;
    uint64 mask = pCfg->m_counterMask;
    uint32 smuCnt = 0;
    SmuList* pSmuList = NULL;

    if (NULL != pLength)
    {
        if (0 != mask)
        {
            uint32 cuCnt = 0;
            //HelpGetBitsCount(mask, &attrCnt);

            // Default length calculation for each counter
            bufferLen = sizeof(RawRecordHdr);

            if (mask & SMU_ATTRIBUTE_MASK)
            {
                cuCnt = GetComputeUnitCntPerNode();
            }

            for (attrCnt = 0; attrCnt < COUNTERID_MAX_CNT; attrCnt++)
            {
                if ((mask >> attrCnt) & 0x01)
                {
                    switch (attrCnt)
                    {
                        // 2 byte counters
                        case COUNTERID_SAMPLE_ID:
                        {
                            bufferLen += sizeof(uint16);
                            break;
                        }

                        // 8 byte counters
                        case COUNTERID_RECORD_ID:
                        case COUNTERID_SAMPLE_TIME:
                        case COUNTERID_PID:
                        case COUNTERID_TID:
                        {
                            bufferLen += sizeof(uint64);
                            break;
                        }

                        // per cu counters
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
                            bufferLen += (cuCnt * sizeof(uint32));
                            break;
                        }

                        // 4 byte counters
                        case COUNTERID_SMU7_APU_PWR_IGPU:
                        case COUNTERID_SMU7_APU_PWR_PCIE:
                        case COUNTERID_SMU7_APU_PWR_DDR:
                        case COUNTERID_SMU7_APU_PWR_DISPLAY:
                        case COUNTERID_SMU7_APU_PWR_PACKAGE:
                        case COUNTERID_SMU7_APU_TEMP_IGPU:
                        case COUNTERID_SMU7_APU_TEMP_MEAS_IGPU:
                        case COUNTERID_SMU7_APU_FREQ_IGPU:
                        case COUNTERID_SMU8_APU_PWR_VDDGFX:
                        case COUNTERID_SMU8_APU_PWR_APU:
                        case COUNTERID_SMU8_APU_TEMP_VDDGFX:
                        case COUNTERID_SMU8_APU_TEMP_MEAS_VDDGFX:
                        case COUNTERID_SMU8_APU_FREQ_IGPU:
                        case COUNTERID_SMU8_APU_PWR_VDDIO:
                        case COUNTERID_SMU8_APU_PWR_VDDNB:
                        case COUNTERID_SMU8_APU_PWR_VDDP:
                        case COUNTERID_SMU8_APU_PWR_UVD:
                        case COUNTERID_SMU8_APU_PWR_VCE:
                        case COUNTERID_SMU8_APU_PWR_ACP:
                        case COUNTERID_SMU8_APU_PWR_UNB:
                        case COUNTERID_SMU8_APU_PWR_SMU:
                        case COUNTERID_SMU8_APU_PWR_ROC:
                        case COUNTERID_SMU8_APU_FREQ_ACLK:
                        case COUNTERID_CSTATE_RES:
                        case COUNTERID_PSTATE:
                        case COUNTERID_NODE_TCTL_TEPERATURE:
                        case COUNTERID_SVI2_CORE_TELEMETRY:
                        case COUNTERID_SVI2_NB_TELEMETRY:
                        {
                            bufferLen += sizeof(uint32);
                            break;
                        }

                        // valriable length counter
                        case COUNTERID_SAMPLE_CALLCHAIN:
                        {
                            break;
                        }

                        // 24 bytes counter
                        case COUNTERID_CEF:
                        {
                            bufferLen += (3 * sizeof(uint64));
                            break;
                        }

                        default:
                            break;

                    }
                }
            }

        }

        // If SMU is configured calculeted the Smu counters
        if (NULL != pCfg->m_smuCfg)
        {
            pSmuList = pCfg->m_smuCfg;

            for (smuCnt = 0; smuCnt < pSmuList->m_count; smuCnt++)
            {
                if (APU_SMU_ID == pSmuList->m_info[smuCnt].m_packageId)
                {
                    continue;
                }

                mask = pSmuList->m_info[smuCnt].m_counterMask;

                for (attrCnt = 0; attrCnt < DGPU_COUNTERS_MAX; attrCnt++)
                {
                    if ((mask >> attrCnt) & 0x01)
                    {
                        switch (attrCnt)
                        {
                            case COUNTERID_PKG_PWR_DGPU:
                            case COUNTERID_TEMP_MEAS_DGPU:
                            case COUNTERID_FREQ_DGPU:
                            case COUNTERID_VOLT_VDDC_LOAD_DGPU:
                            case COUNTERID_CURR_VDDC_DGPU:
                                bufferLen += sizeof(uint32);
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

    *pLength = bufferLen;

    return true;
}

// GetMarkerKey: Generate marker id from the given marker name
uint32 GetMarkerKey(uint8* str)
{
    uint32 hash = 5381;
    uint32 c;
    uint32 len = 0;

    while ((c = *str++) && (len++ < PWR_MARKER_BUFFER_SIZE))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

// GetMarkerRecords: Read available markers from shared buffer
void GetMarkerRecords(MarkerTag* pTags, uint32* pCount)
{
    uint32* pMarkerCnt = 0;
    MarkerTag* pBuffer = NULL;
    uint32 prev = 0;
    *pCount = 0;

    if ((NULL != pSharedBuffer)
        && (*(uint32*)&pSharedBuffer[ sizeof(uint32)] > 0)
        && (NULL != pTags))
    {

        // Check if buffer is busy. If not, set the busy flag and read data
        prev = ATOMIC_CMPEXCHANGE(pSharedBuffer, 1, 0);

        // Buffer is not busy, read the content
        if (prev == 0)
        {
            pMarkerCnt = (uint32*)&pSharedBuffer[ sizeof(uint32)];

            if (*pMarkerCnt > 0)
            {
                pBuffer = (MarkerTag*)&pSharedBuffer[2 * sizeof(uint32)];
                memset(pTags, 0, PWR_MAX_MARKER_CNT * sizeof(MarkerTag));
                memcpy(pTags, pBuffer, *pMarkerCnt * sizeof(MarkerTag));
                *pCount = *pMarkerCnt;
                ATOMIC_SET((atomic*)pSharedBuffer, 0);
                ATOMIC_SET((atomic*)pMarkerCnt, 0);
            }
        }
    }
}

//WriteSampleData: Prepare the records from the atrribute values and write into
//the buffer.
int32 WriteSampleData(CoreData* pCoreCfg)
{
    int32 ret = STATUS_SUCCESS;
    uint8* pBuffer = NULL;
    RawRecordHdr* pRecHdr = NULL;
    uint64 attrMask = 0;
    uint32 offset = 0;
    uint32 smuCnt = 0;
    PageBuffer* pCoreBuffer = NULL;
    bool collectSmu = true;
    bool isMarkerTag = false;
    uint32 len = 0;
    uint32 prevOffset = 0;
    uint32 markerCnt = 0;
    uint32 idx = 0;
    bool updateMax = false;
    MarkerTag markerInfo[PWR_MAX_MARKER_CNT];

    // Check valid pointers & buffers
    if ((NULL == pCoreCfg)
        || (NULL == pCoreCfg->m_pCoreBuffer)
        || (NULL == pCoreCfg->m_pCoreBuffer->m_pBuffer))
    {
        DRVPRINT("NULL pointer pCoreCfg %s m_pBuffer %s",
                 (NULL == pCoreCfg) ? "NULL" : "OK",
                 (NULL == pCoreCfg->m_pCoreBuffer->m_pBuffer) ? "NULL" : "OK");
        ret = STATUS_NO_MEMORY;
    }

    // Calculate buffer required
    if (STATUS_SUCCESS == ret)
    {
        pCoreCfg->m_pCoreBuffer->m_recCnt++;

        // Calculate buffer required for context record
        if (pCoreCfg->m_profileType == PROFILE_TYPE_PROCESS_PROFILING)
        {
            collectSmu =  false;
            len = CONTEXT_RECORD_LEN;

            if (!(pCoreCfg->m_pCoreBuffer->m_recCnt % pCoreCfg->m_samplingInterval)
                && (0 != pCoreCfg->m_pCoreBuffer->m_recCnt))
            {
                collectSmu = true;
                len += pCoreCfg->m_recLen;

                GetMarkerRecords(markerInfo, &markerCnt);

                if (markerCnt > 0)
                {
                    len += (markerCnt * MARKER_RECORD_LEN);
                    isMarkerTag = true;
                }
            }
        }
        else
        {
            len = pCoreCfg->m_recLen;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pCoreBuffer = pCoreCfg->m_pCoreBuffer;
        ATOMIC_GET(&offset, pCoreBuffer->m_currentOffset);

        // Check if remaining buffer area is not sufficient to accomodated one more record.
        // If so, start writing from begining
        if (PWRPROF_PERCORE_BUFFER_SIZE <= (offset + len))
        {
            ATOMIC_SET(&pCoreBuffer->m_currentOffset, 0);
            prevOffset = offset;
            updateMax = true;
            offset = 0;
        }

        pBuffer = pCoreBuffer->m_pBuffer;
    }

    if ((STATUS_SUCCESS == ret)
        && isMarkerTag)
    {
        for (idx = 0; idx < markerCnt; idx++)
        {
            pRecHdr = (RawRecordHdr*) &pBuffer[offset];
            pRecHdr->m_recordType = REC_TYPE_MARKER_DATA;
            pRecHdr->m_recordLen = (uint16)MARKER_RECORD_LEN;
            pRecHdr->m_fill = 0;
            offset += sizeof(RawRecordHdr);
            markerInfo[idx].m_ts = pCoreCfg->m_contextData.m_timeStamp;
            markerInfo[idx].m_markerId = GetMarkerKey(markerInfo[idx].m_name);
            memcpy((MarkerTag*)&pBuffer[offset], &markerInfo[idx], sizeof(MarkerTag));
            offset += sizeof(MarkerTag);
            DRVPRINT("marker name %s id %d", markerInfo[idx].m_name, markerInfo[idx].m_markerId);
        }

        markerCnt = 0;
        isMarkerTag = false;
    }

    if ((STATUS_SUCCESS == ret)
        && (pCoreCfg->m_profileType == PROFILE_TYPE_PROCESS_PROFILING))
    {
        // Collect context data @ 1ms
        uint64* pRecId = NULL;
        ContextData* pContextData = NULL;
        pRecHdr = (RawRecordHdr*) &pBuffer[offset];
        pRecHdr->m_recordType = REC_TYPE_CONTEXT_DATA;
        pRecHdr->m_recordLen = (uint16)CONTEXT_RECORD_LEN;
        pRecHdr->m_fill = 0;
        offset += sizeof(RawRecordHdr);

        pRecId = (uint64*)&pBuffer[offset];
        *pRecId =  pCoreCfg->m_pCoreBuffer->m_recCnt;
        offset += sizeof(uint64);

        // Fill the context data
        pContextData = (ContextData*)&pBuffer[offset];
        ReadPmcCounterData(pCoreCfg->m_pmc, pContextData->m_pmcData);
        ResetPMCCounters(pCoreCfg->m_pmc);

        pContextData->m_processId = pCoreCfg->m_contextData.m_processId;
        pContextData->m_threadId = pCoreCfg->m_contextData.m_threadId;
        pContextData->m_timeStamp = pCoreCfg->m_contextData.m_timeStamp;
        pContextData->m_ip = pCoreCfg->m_contextData.m_ip;

        offset += sizeof(ContextData);
    }

    if ((true == collectSmu) && (STATUS_SUCCESS == ret))
    {
        pRecHdr = (RawRecordHdr*) &pBuffer[offset];
        pRecHdr->m_recordType = REC_TYPE_SAMPLE_DATA;
        pRecHdr->m_recordLen = (uint16)pCoreCfg->m_recLen;
        pRecHdr->m_fill = 0;
        offset += sizeof(RawRecordHdr);

        // These attributes are common to all cores
        CollectBasicCounters(pCoreCfg, &offset);

        // Collect SMU counter data
        if ((1 == pCoreCfg->m_sampleId)
            && (NULL != pCoreCfg->m_smuCfg)
            && (APU_SMU_ID == pCoreCfg->m_smuCfg->m_info[0].m_packageId)
            && (0 != pCoreCfg->m_smuCfg->m_info[0].m_counterMask))
        {
            SmuInfo* pSmu = NULL;
            SmuAccessCb* pAccess = NULL;
            pSmu = &pCoreCfg->m_smuCfg->m_info[0];

            if (NULL != pSmu)
            {
                pAccess = (SmuAccessCb*)pSmu->m_access.m_accessCb;

                if (NULL != pAccess->fnSmuReadCb)
                {
                    pAccess->fnSmuReadCb(pSmu,
                                         &pCoreCfg->m_pCoreBuffer->m_pBuffer[offset],
                                         &offset);
                }
            }
        }

        attrMask = pCoreCfg->m_counterMask;

        if (attrMask & PERCORE_ATTRIBUTE_MASK)
        {
            CollectPerCoreCounters(pCoreCfg, &offset);
        }

        if (attrMask & NONCORE_ATTRIBUTE_MASK)
        {
            CollectNonCoreCounters(pCoreCfg, &offset);
        }

        // Check if smu is configured in master core
        if ((1 == pCoreCfg->m_sampleId) && (NULL != pCoreCfg->m_smuCfg))
        {
            for (smuCnt = 0; smuCnt < pCoreCfg->m_smuCfg->m_count; smuCnt++)
            {
                SmuInfo* pSmu = NULL;
                SmuAccessCb* pAccess = NULL;
                pSmu = &pCoreCfg->m_smuCfg->m_info[smuCnt];

                if (APU_SMU_ID == pSmu->m_packageId)
                {
                    continue;
                }
                else if (0 != pSmu->m_counterMask)
                {
                    pAccess = (SmuAccessCb*)pSmu->m_access.m_accessCb;

                    if (NULL != pAccess->fnSmuReadCb)
                    {
                        pAccess->fnSmuReadCb(pSmu,
                                             &pCoreCfg->m_pCoreBuffer->m_pBuffer[offset],
                                             &offset);
                    }
                }
            }
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        //Update the currentOffset now
        ATOMIC_SET(&pCoreBuffer->m_currentOffset, offset);

        if (true == updateMax)
        {
            ATOMIC_SET(&pCoreBuffer->m_maxValidOffset, prevOffset);
        }
    }

    return ret;
}
