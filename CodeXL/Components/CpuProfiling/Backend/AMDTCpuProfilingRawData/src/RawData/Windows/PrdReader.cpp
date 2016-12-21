//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdReader.cpp
/// \brief Implementation of the PrdReader class.
///
//==================================================================================

#pragma warning( disable : 4127 )

#include <Windows/PrdReader.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>


enum physicalCtrs
{
    NBR_OF_PHY_CTR              = 4,
    NBR_OF_PHY_NB_CTR           = 0,
    NBR_OF_PHY_L2I_CTR          = 0,
    NBR_OF_PHY_CTR_FAMILY15     = 6,
    NBR_OF_PHY_NB_CTR_FAMILY15  = 4,
    NBR_OF_PHY_L2I_CTR_FAMILY16 = 4
};

#define CA_MAX_RES_TYPE  ((unsigned int)(sizeof(((sTrcCaWeightRecord*)0)->m_indexes) / sizeof(*(((sTrcCaWeightRecord*)0)->m_indexes))))
#define CXL_MAX_RES_TYPE ((unsigned int)(sizeof(((PRD_WEIGHT_RECORD*)0)->m_indexes) / sizeof(*(((PRD_WEIGHT_RECORD*)0)->m_indexes))))

#define CA_NBR_OF_WEIGHTS  ((unsigned int)(sizeof(((sTrcCaWeightRecord*)0)->m_Weights) / sizeof(*(((sTrcCaWeightRecord*)0)->m_Weights))))
#define CXL_NBR_OF_WEIGHTS ((unsigned int)(sizeof(((PRD_WEIGHT_RECORD*)0)->m_Weights) / sizeof(*(((PRD_WEIGHT_RECORD*)0)->m_Weights))))

// This is grabbed from pcore-interface.h
// We cannot include pcore-interface.h, since it contains kernel specific types like "KEVENT"
//
// This enum represents the different types of available configurations
typedef enum tagResourceTypes
{
    /// Advanced Programmable Interrupt Controller for timer profiling
    APIC = 0,
    /// Performance events counter (PMC)
    EVENT_CTR,
    /// Northbridge events counter
    NB_CTR,
    /// Instruction based sampling fetch events
    IBS_FETCH,
    /// Instruction based sampling operation events
    IBS_OP,
    /// L2I events counter
    L2I_CTR, // This should stay here to not break compatibility with CodeAnalyst
    /// The maximum number of types available
    MAX_RESOURCE_TYPE
} PCORERESOURCETYPES;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PrdReader::PrdReader()
{
    m_aTopology = NULL;
    m_hPRDFile = NULL;
    m_TimerResolution = 0;
    m_hrFreq = 0;
    m_EMTicks = 0;
    m_EventGroupCount = 0;

    m_firstWeightRecOffset = 0;
    m_weightRecHasBufferCount = false;

    m_IbsFetchCtl = 0;
    m_IbsOpCtl = 0;
    m_profType = 0;
    m_version = 0;

    m_startMark.dwHighDateTime = 0;
    m_startMark.dwLowDateTime = 0;
    m_endMark.dwHighDateTime = 0;
    m_endMark.dwLowDateTime = 0;

    m_startTick = 0;
    m_endTick = 0;

    m_EventMap.clear();
    m_L1DcSize = 0;
    m_L1DcAssoc = 0;
    m_L1DcLinesPerTag = 0;
    m_L1DcLineSize = 0;
}

PrdReader::~PrdReader()
{
    Close();
}

unsigned int PrdReader::GetProfileVersion()
{
    return m_version;
}

//===========================================================================
//Initialize

//@mfunc Opens the file specified and validates the header information

//In:
//  @parm const char * | pFileName | Name of the PRD file to open.
// If processId != 0, then we need to open up another temp prd file, and
//  write only samples with that pid into it.

//@rdesc Returns whether an error was encountered
//@flag S_OK | No error was encountered
//@flag E_FAIL | An error was encountered

//===========================================================================
/*
HRESULT PrdReader::InitializeOldPrd (const wchar_t *pFileName)
{
    HRESULT         hr_ret_val = S_OK;
    sFileHeaderV2   file_header;
    size_t          bytes_read;

    m_profType = 0;
    m_cpuFamily = 0;
    m_cpuModel = 0;
    m_coreCount = 0;

    //  Open the file
    m_hPRDFile = _wfopen( pFileName, L"rb" );
    if (m_hPRDFile == NULL)
        return E_FAIL;

    // Read in the first entry
    bytes_read = fread( &file_header, 1, PRD_RECORD_SIZE, m_hPRDFile);

    // Check header signature and version
    if (( file_header.m_Signature != PRD_HDR_SIGNATURE )
        || ( file_header.m_Version > V2_HDR_VERSION ))
    {
        Close();
        return E_FAIL;
    }

    m_version = file_header.m_Version;
    RawPRDRecord tRawRecord;

    m_startMark = file_header.m_StartMark;
    m_startTick = file_header.m_TSC;

    if (V2_HDR_VERSION != m_version)
    {
        //Get the time marks for the profile
        HANDLE timeHandle = CreateFile (pFileName, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        //note that create time resolution is 10mS and write time resolution is 1S
        GetFileTime (timeHandle, &m_startMark, NULL, NULL);
        CloseHandle (timeHandle);
    } else {
        //get the end tsc from the last record
        long length = sizeof(sTrcMissedCountRecord);
        fseek (m_hPRDFile, -1 * (long) length, SEEK_END );
        sTrcMissedCountRecord missedRec;
        fread( &missedRec, 1, length, m_hPRDFile );

        if (PROF_REC_MISSED == missedRec.m_RecordType )
        {
            m_endTick = missedRec.m_TSC;
        }

        fseek (m_hPRDFile, PRD_RECORD_SIZE, SEEK_SET  );
    }

    bool bSampleRecord = false;
    // Get first record entry and trying to look for
    // the CONFIG record
    while (!bSampleRecord && GetNextRawRecord(&tRawRecord) == S_OK)
    {
        gtUByte recordType = tRawRecord.rawRecordsData[0];

        switch(recordType)
        {
        case PROF_REC_TIMER :
        case PROF_REC_EVENT :
        case PROF_REC_IBS_FETCH_BASIC:
        case PROF_REC_IBS_FETCH_EXT:
        case PROF_REC_IBS_OP_BASIC:
        case PROF_REC_IBS_OP_EXT:
            bSampleRecord = true;
            break;

        case PROF_REC_EVTCFG:
            {
                m_profType |= PROF_EBP;

                sTrcEvtConfRecord *pEvtConfRecord = (sTrcEvtConfRecord*) (tRawRecord.rawRecordsData);
                if (m_EventGroupCount <= pEvtConfRecord->m_GroupIndex)
                {
                    m_EventGroupCount = pEvtConfRecord->m_GroupIndex + 1;
                }

                EventKey eKey;
                EventData eData;

                for (unsigned int core = 0; core < m_coreCount; core++)
                {
                    eKey.core = core;
                    eKey.ctl.perf_ctl = pEvtConfRecord->m_EventSelReg;

                    EventMap::iterator it = m_EventMap.find(eKey);
                    if (it == m_EventMap.end()) {
                        eData.ctr = pEvtConfRecord->m_EventCount;
                        eData.numApperance = 1;
                        m_EventMap.insert(EventMap::value_type(eKey, eData));
                    } else {
                        it->second.numApperance++;
                    }
                }
                // all event multiplex will share the same event mulitplexing period clock ticks
                m_EMTicks = pEvtConfRecord->m_EvtMultiplexTicks;
            }
            break;

        case PROF_REC_TIMERCFG:
            {
                m_profType |= PROF_TBP;
                sTrcTimerConfRecord *pTimerCfg = (sTrcTimerConfRecord*) (tRawRecord.rawRecordsData);
                m_TimerResolution = pTimerCfg->m_TimerGranularity;
            }
            break;

        case PROF_REC_IBSCFG:
            {
                // IBS Configuration.
                m_profType |= PROF_IBS;
                sTrcIbsConfRecord *pIbsCfg = (sTrcIbsConfRecord*) (tRawRecord.rawRecordsData);

                if (pIbsCfg->m_IbsFetchMaxCnt)
                    m_IbsFetchCtl = pIbsCfg->m_IbsFetchMaxCnt >> 4;
                if (pIbsCfg->m_IbsOpMaxCnt)
                    m_IbsOpCtl = pIbsCfg->m_IbsOpMaxCnt >> 4;
            }
            break;

        case PROF_REC_CPUID:
            {
                sTrcCPUInfoRecord *pCpuRec = (sTrcCPUInfoRecord *) (tRawRecord.rawRecordsData);
                ConvertCpuFamily (pCpuRec->socket[0].m_FamilyModel);

                // since  pCpuRec->socket[0].m_ClockSpeed is Clock speed of 100MHZ);
                m_ClockSpeed = pCpuRec->socket[0].m_ClockSpeed * 100;

                m_coreCount = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (0x80 != pCpuRec->socket[i].m_NumCPUs)
                        m_coreCount += pCpuRec->socket[i].m_NumCPUs;
                }
            }
            break;

        case PROF_REC_PIDCFG:
            break;

        case PROF_REC_USER :
        case PROF_REC_CONFIG:
        case PROF_REC_DEBUG:
        case PROF_REC_CSS:

        default:
            break;
        }
    }

    if (bSampleRecord) {
        long offset = ftell(m_hPRDFile);
        fseek(m_hPRDFile, offset - sizeof(sTrcTimerRecord), SEEK_SET);
    }

    return hr_ret_val;
} // PrdReader::Initialize
*/


HRESULT PrdReader::Initialize(const wchar_t* pFileName, gtUInt64* pLastUserCssRecordOffset)
{
    HRESULT         hr_ret_val = S_OK;
    PRD_FILE_HEADER_V3  file_header;
    PRD_HEADER_EXT_RECORD* pExt_header;
    size_t          bytes_read;
    RawPRDRecord tRawRecord;

    m_profType = 0;
    m_cpuFamily = 0;
    m_cpuModel = 0;
    m_coreCount = 0;

    if (NULL != pLastUserCssRecordOffset)
    {
        *pLastUserCssRecordOffset = 0ULL;
    }

    //
    //  Open the file
    //
    if (0 != _wfopen_s(&m_hPRDFile, pFileName, L"rb"))
    {
        return E_FAIL;
    }

    // Read in the first entry
    bytes_read = fread(&file_header, 1, PRD_RECORD_SIZE, m_hPRDFile);

    // Check header signature and version
    if (file_header.m_Signature != PRD_HDR_SIGNATURE)
    {
        Close();
        return E_FAIL;
    }

    if (file_header.m_Version >= V3_HDR_VERSION)
    {
        // CA 3.0
        m_version = file_header.m_Version;

        m_startMark = file_header.m_StartMark;
        m_coreCount = file_header.m_coreCount;
        m_ClockSpeed = file_header.m_speed;
    }
    else
    {
        sFileHeaderV2*   pOldFileHdr = (sFileHeaderV2*)(&file_header);
        m_startMark = pOldFileHdr->m_StartMark;
        m_startTick = pOldFileHdr->m_TSC;
    }

    HANDLE timeHandle = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ,
                                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //note that create time resolution is 10mS and write time resolution is 1S
    GetFileTime(timeHandle, NULL, NULL, &m_endMark);
    CloseHandle(timeHandle);

    bool bSampleRecord = false;

    // Get first record entry and trying to look for
    // the CONFIG record
    while (!bSampleRecord && GetNextRawRecord(&tRawRecord) == S_OK)
    {
        gtUByte recordType = tRawRecord.rawRecordsData[0];

        switch (recordType)
        {
            case PROF_REC_TIMER :
            case PROF_REC_EVENT :
            case PROF_REC_IBS_FETCH_BASIC:
            case PROF_REC_IBS_FETCH_EXT:
            case PROF_REC_IBS_OP_BASIC:
            case PROF_REC_IBS_OP_EXT:
                bSampleRecord = true;
                break;

            case PROF_REC_EVTCFG:
                UpdateEventConfigRec(&tRawRecord);
                break;

            case PROF_REC_TIMERCFG:
                UpdateTimerConfigRec(&tRawRecord);
                break;

            case PROF_REC_IBSCFG:
                UpdateIBSConfigRec(&tRawRecord);
                break;

            case PROF_REC_CPUID:
            case PROF_REC_CPUINFO:
                UpdateCPUIDRec(&tRawRecord);
                break;

            case PROF_REC_TOPOLOGY:
                UpdateTopologyRec(&tRawRecord);
                break;

            case PROF_REC_WEIGHT:
                UpdateResWeight(&tRawRecord);
                // Save the offset of the first weight record
                m_firstWeightRecOffset = ftell(m_hPRDFile) - CAPRDRECORDSIZE;
                break;

            case PROF_REC_PIDCFG:
                UpdatePIDFilter(&tRawRecord);
                break;

            case PROF_REC_EXT_HEADER:
                pExt_header = (PRD_HEADER_EXT_RECORD*)&tRawRecord;
                m_hrFreq = pExt_header->m_HrFrequency;

                if (NULL != pLastUserCssRecordOffset)
                {
                    *pLastUserCssRecordOffset = pExt_header->m_LastUserCssRecordOffset;
                }

                break;

            case PROF_REC_USER :
            case PROF_REC_CONFIG:
            case PROF_REC_DEBUG:
            case PROF_REC_CSS:
            default:
                break;
        }
    }

    if (m_version < V3_HDR_VERSION)
    {
        AdjustOldEventConfig();
    }

    if (bSampleRecord)
    {
        long offset = ftell(m_hPRDFile);
        fseek(m_hPRDFile, offset - sizeof(tRawRecord), SEEK_SET);
    }

    return hr_ret_val;
} // PrdReader::Initialize


gtUInt64 PrdReader::GetEventMultiplexPeriod()
{
    return m_EMTicks;
}

int PrdReader::GetCpuFamily() const
{
    return m_cpuFamily;
}

int PrdReader::GetCpuModel() const
{
    return m_cpuModel;
}
//===========================================================================
//Close

//@mfunc Closes the file

//@rdesc Returns whether an error was encountered
//@flag S_OK | No error was encountered

//===========================================================================
HRESULT PrdReader::Close()
{
    HRESULT hr_ret_val = S_OK;

    if (m_hPRDFile)
    {
        //  Close the file handle
        fclose(m_hPRDFile);
        m_hPRDFile = NULL;
    }

    if (NULL != m_aTopology)
    {
        delete [] m_aTopology;
    }

    m_aTopology = NULL;
    return hr_ret_val;
}

unsigned int PrdReader::GetProfileType() const
{
    return m_profType;
}

// The following calls will get raw records.
HRESULT PrdReader::GetNextRawRecord(RawPRDRecord* pRawDataRec)
{
    HRESULT hr = E_FAIL;
    unsigned int bytes_read;

    if (!pRawDataRec || !m_hPRDFile)
    {
        return hr;
    }

    bytes_read = fread(pRawDataRec->rawRecordsData, 1, CAPRDRECORDSIZE, m_hPRDFile);

    if (CAPRDRECORDSIZE == bytes_read)
    {
        hr = S_OK;

    }

    return hr;
}

HRESULT PrdReader::GetNextRawRecords(RawPRDRecord* pRawRec1, RawPRDRecord* pRawRec2, unsigned int* pRecNum)
{
    HRESULT hr = E_FAIL;
    unsigned int bytes_read;

    if (!pRawRec1 || !pRawRec2 || !pRecNum || !m_hPRDFile)
    {
        return hr;
    }

    bytes_read = fread(pRawRec1->rawRecordsData, 1, CAPRDRECORDSIZE, m_hPRDFile);

    if (CAPRDRECORDSIZE == bytes_read)
    {
        *pRecNum = 1;
        hr = S_OK;

        // read first record correctly
        switch (pRawRec1->rawRecordsData[0])
        {
            case PROF_REC_IBS_OP_EXT:
            {
                PRD_IBS_OP_DATA_BASIC_RECORD* pOp = (PRD_IBS_OP_DATA_BASIC_RECORD*)
                                                    pRawRec1->rawRecordsData;

                if (m_endTick < pOp->m_TickStamp)
                {
                    m_endTick = pOp->m_TickStamp;
                }

                bytes_read = fread(pRawRec2->rawRecordsData, 1, CAPRDRECORDSIZE, m_hPRDFile);

                if (CAPRDRECORDSIZE != bytes_read)
                {
                    hr = E_FAIL;
                }

                *pRecNum = 2;
            }
            break;

            case PROF_REC_IBS_FETCH_EXT:
            {
                PRD_IBS_FETCH_DATA_BASIC_RECORD* pF = (PRD_IBS_FETCH_DATA_BASIC_RECORD*)
                                                      pRawRec1->rawRecordsData;

                if (m_endTick < pF->m_TickStamp)
                {
                    m_endTick = pF->m_TickStamp;
                }

                bytes_read = fread(pRawRec2->rawRecordsData, 1, CAPRDRECORDSIZE, m_hPRDFile);

                if (CAPRDRECORDSIZE != bytes_read)
                {
                    hr = E_FAIL;
                }

                *pRecNum = 2;
            }
            break;

            case PROF_REC_EVTCFG:
                UpdateEventConfigRec(pRawRec1);
                break;

            case PROF_REC_TIMERCFG:
                UpdateTimerConfigRec(pRawRec1);
                break;

            case PROF_REC_IBSCFG:
                UpdateIBSConfigRec(pRawRec1);
                break;

            case PROF_REC_CPUID:
                UpdateCPUIDRec(pRawRec1);
                break;

            case PROF_REC_WEIGHT:
                UpdateResWeight(pRawRec1);
                break;

            case PROF_REC_PIDCFG:
                UpdatePIDFilter(pRawRec1);

            case PROF_REC_USER :
            case PROF_REC_CONFIG:
            case PROF_REC_DEBUG:
            case PROF_REC_CSS:
            case PROF_REC_TIMER :
            case PROF_REC_EVENT :
            case PROF_REC_IBS_FETCH_BASIC:
            case PROF_REC_IBS_OP_BASIC:
            default:
                break;
        }
    }

    return hr;
}

HRESULT PrdReader::GetBufferWeightRecOffset(void* baseAddress, gtUInt32 length, gtUInt32* pOffset)
{
    RawPRDRecord rec;
    int          ret;
    bool         foundWeightRec = false;
    void*        address = baseAddress;
    gtUInt32     cnt;

    while (! foundWeightRec)
    {
        ret = ReadMappedRecord(address, &rec);

        if (CAPRDRECORDSIZE != ret)
        {
            return E_FAIL;
        }

        if (PROF_REC_WEIGHT == rec.rawRecordsData[0])
        {
            if (m_version >= CXL_HDR_VERSION)
            {
                PRD_WEIGHT_RECORD* pW = (PRD_WEIGHT_RECORD*) rec.rawRecordsData;
                cnt = static_cast<gtUInt32>(pW->m_BufferRecordCount);
            }
            else
            {
                sTrcCaWeightRecord* pW = (sTrcCaWeightRecord*) rec.rawRecordsData;
                cnt = static_cast<gtUInt32>(pW->m_BufferRecordCount);
            }

            if (0 != cnt)
            {
                // found a valid weight record with nonzero buffer record count;
                *pOffset = static_cast<gtUInt32>(static_cast<gtByte*>(address) - static_cast<gtByte*>(baseAddress));
                return S_OK;
            }

            // PRD from CodeAnalyst 3.2, just return, if we see the weight rec..
            if (false == m_weightRecHasBufferCount)
            {
                return S_OK;
            }
        }

        address = static_cast<gtByte*>(address) + CAPRDRECORDSIZE;

        //If there were no records...
        if (reinterpret_cast<gtUIntPtr>(address) >= (reinterpret_cast<gtUIntPtr>(baseAddress) + static_cast<gtUIntPtr>(length)))
        {
            return E_NODATA;
        }
    }

    return S_OK;
}


// convert the raw sample (timer/event) into RecordDataStruct
// returns E_INVALIDARG if the record type didn't have config info
HRESULT PrdReader::ConvertSampleData(const RawPRDRecord& rawRecord, RecordDataStruct* pRecData)
{
    HRESULT hr_ret_val = S_OK;

    switch (rawRecord.rawRecordsData[0])
    {
        case (PROF_REC_EVENT):

            if (0 == (m_profType & PROF_EBP))
            {
                return E_INVALIDARG;
            }

            if (m_version >= V3_HDR_VERSION)
            {
                PRD_EVENT_CTR_DATA_RECORD* pEvtRec  = (PRD_EVENT_CTR_DATA_RECORD*) rawRecord.rawRecordsData;

                bool isL2IEvent = false;

                if ((pEvtRec->m_EventSelectHigh & (FAKE_L2I_EVENT_ID_PREFIX << 4)) ==
                    (FAKE_L2I_EVENT_ID_PREFIX << 4))
                {
                    isL2IEvent = true;
                }

                if (isL2IEvent)
                {
                    // For L2I events - Do not mask fake value in m_EventSelectHigh bits[7:4]
                    pRecData->m_EventType = (pEvtRec->m_EventSelectHigh << 8) + (pEvtRec->m_EventCtl & 0xFF);
                }
                else
                {
                    pRecData->m_EventType = ((pEvtRec->m_EventSelectHigh & 0xF) << 8) + (pEvtRec->m_EventCtl & 0xFF);
                }

                pRecData->m_EventUnitMask = (pEvtRec->m_EventCtl >> 8) & 0xFF;
                pRecData->m_eventBitMask = (pEvtRec->m_EventCtl >> 16) & 0xf;
                pRecData->m_PID = pEvtRec->m_ProcessHandle;
                pRecData->m_ProcessorID = pEvtRec->m_Core;
                pRecData->m_RIP = pEvtRec->m_InstructionPointer;
                pRecData->m_ThreadHandle = pEvtRec->m_ThreadHandle;
                pRecData->m_DeltaTick = pEvtRec->m_TickStamp;

                gtUInt32 weight = 0;

                if (isL2IEvent)
                {
                    weight = GetWeight(pEvtRec->m_Core, L2I_PMC_WEIGHT_BASE + pEvtRec->m_EventCounter);
                }
                else
                {
                    weight = GetWeight(pEvtRec->m_Core, PMC_WEIGHT_BASE + pEvtRec->m_EventCounter);
                }

                pRecData->m_weight = static_cast<gtUInt16>(weight);
            }
            else    // ca 2.9 and earlier
            {
                sTrcPerfCountRecord* pEvtRec = (sTrcPerfCountRecord*) rawRecord.rawRecordsData;
                pRecData->m_EventType = pEvtRec->m_EventSelect;
                pRecData->m_EventUnitMask = pEvtRec->m_UnitMask;
                pRecData->m_eventBitMask = 0;
                pRecData->m_PID = pEvtRec->m_ProcessHandle;
                pRecData->m_ProcessorID = pEvtRec->m_ProcessorNumber;
                pRecData->m_RIP = pEvtRec->m_InstructionPointer;
                pRecData->m_ThreadHandle = pEvtRec->m_ThreadHandle;
                pRecData->m_DeltaTick = ConvertToDeltaTick(pEvtRec->m_TimeStampCounter);
                pRecData->m_weight = 1;
            }

            break;

        case (PROF_REC_TIMER):
        {
            if (0 == (m_profType & PROF_TBP))
            {
                return E_INVALIDARG;
            }

            if (m_version >= V3_HDR_VERSION)
            {
                // for prd in CA 3.0 and later;
                PRD_APIC_DATA_RECORD* pRec  = (PRD_APIC_DATA_RECORD*) rawRecord.rawRecordsData;
                pRecData->m_EventType = GetTimerEvent();
                pRecData->m_EventUnitMask = 0;
                pRecData->m_eventBitMask = 0;
                pRecData->m_PID = pRec->m_ProcessHandle;
                pRecData->m_ProcessorID = pRec->m_Core;
                pRecData->m_RIP = pRec->m_InstructionPointer;
                pRecData->m_ThreadHandle = pRec->m_ThreadHandle;
                pRecData->m_DeltaTick = pRec->m_TickStamp;
                auto weight = GetWeight(pRec->m_Core, APIC_WEIGHT_BASE);
                pRecData->m_weight = static_cast<gtUInt16>(weight);
            }
            else
            {
                // for old prd file in ca 2.9 and ealier;
                sTrcTimerRecord* pTimerRec      = (sTrcTimerRecord*) rawRecord.rawRecordsData;
                pRecData->m_EventType           = GetTimerEvent();
                pRecData->m_EventUnitMask = 0;
                pRecData->m_eventBitMask = 0;
                pRecData->m_PID                 = pTimerRec->m_ProcessHandle;
                pRecData->m_ProcessorID         = pTimerRec->m_ProcessorNumber;
                pRecData->m_RIP                 = pTimerRec->m_InstructionPointer;
                pRecData->m_ThreadHandle        = pTimerRec->m_ThreadHandle;
                pRecData->m_DeltaTick           = ConvertToDeltaTick(pTimerRec->m_TimeStampCounter);
                pRecData->m_weight = 1;
            }
        }
        break;

        default:
            hr_ret_val = E_FAIL;
            break;
    }

    return hr_ret_val;
}


HRESULT PrdReader::GetEventInfo(EventCfgInfo* pEventCfgInfo, unsigned size)
{
    if (!pEventCfgInfo)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    // since all core are using same configuration, I will pickup one to report;
    PrdRDEventMap::const_iterator it = m_EventMap.begin();
    PrdRDEventMap::const_iterator it_end = m_EventMap.end();
    unsigned int core = MAXUINT;
    unsigned int cfgCnt = 0;

    while (it != it_end)
    {
        if ((core != -1) && (core != it->first.core))
        {
            break;
        }

        if (cfgCnt >= size)
        {
            hr = E_FAIL;
            break;
        }

        pEventCfgInfo->ctl = it->first.ctl;
        pEventCfgInfo->ctr = it->second.ctr;
        pEventCfgInfo->numApperance = it->second.numApperance;
        core = it->first.core;

        cfgCnt++;
        it++;
        pEventCfgInfo++;
    }

    return hr;
}



unsigned int PrdReader::GetEventGroupCount()
{
    return m_EventGroupCount;
}

unsigned int PrdReader::GetEventCount()
{
    unsigned int evtCnt = 0;

    // since all core are using same configuration, I will pickup one to report;
    PrdRDEventMap::const_iterator it = m_EventMap.begin();
    PrdRDEventMap::const_iterator it_end = m_EventMap.end();
    unsigned int core = MAXUINT;

    while (it != it_end)
    {
        if ((core != -1) && (core != it->first.core))
        {
            break;
        }

        core = it->first.core;

        evtCnt++;
        it++;
    }

    return evtCnt;
}


HRESULT PrdReader::GetIBSConfig(unsigned int* pIBSFetchCount, unsigned int* pIBSOpCount) const
{
    HRESULT hr = E_FAIL;

    if (!pIBSFetchCount || !pIBSOpCount)
    {
        return hr;
    }

    if (m_IbsFetchCtl)
    {
        *pIBSFetchCount = (m_IbsFetchCtl & 0xFFFF) << 4;
    }


    if (m_IbsOpCtl)
    {
        *pIBSOpCount = m_IbsOpCtl & (0x7FF << 20);
        *pIBSOpCount += (m_IbsOpCtl & 0xFFFF) << 4;
    }


    return S_OK;
}

// Convert raw sample data into an IBS fetch record
//
// typedef struct _TrcIbsFetchRecord
// {
//   gtUByte      m_RecordType;           /* PROF_REC_IBS_FETCH_BASIC or PROF_REC_IBS_FETCH_EXT   */
//   gtUByte      m_ProcessorNumber;      /* Processor ID                                         */
//   gtUInt16     m_Reserved;             /* Reserved space                                       */
//   gtUInt32      m_IbsFetchCtlHigh;      /* IbsFetchCtl<63:32> (Fetch event flags/cycle counts)  */
//   gtUInt64    m_InstructionPointer;   /* IbsFetchLinAd<63:0>                                  */
//   gtUInt64    m_TimeStampCounter;     /* Time stamp counter                                   */
//   gtUInt64    m_ProcessHandle;        /* Process ID                                           */
//   gtUInt64    m_ThreadHandle;         /* Thread ID                                            */
// } sTrcIbsFetchRecord;
// typedef struct _TrcIbsFetchRecordExt
// {
//   gtUInt64    m_IbsFetchPhysAd ;      /* IbsFetchPhysAd<63:0>                                 */
//   gtUInt64    m_Reserved1 ;
//   gtUInt64    m_Reserved2 ;
//   gtUInt64    m_Reserved3 ;
//   gtUInt64    m_Reserved4 ;
// } sTrcIbsFetchRecordExt ;

// This is helper function to convert the IBS fetch data record in old prd file;
//
HRESULT ConvertOldIBSFetchData(RawPRDRecord* pRawRecord1,
                               RawPRDRecord* pRawRecord2,
                               IBSFetchRecordData* pIBSFetch)
{
    HRESULT hr = E_FAIL;

    sTrcIbsFetchRecord* pPRDIBSRec = (sTrcIbsFetchRecord*) pRawRecord1;
    sTrcIbsFetchRecordExt* pPRDIBSRecExt = (sTrcIbsFetchRecordExt*) pRawRecord2;

    switch (pPRDIBSRec->m_RecordType)
    {
        case PROF_REC_IBS_FETCH_BASIC:
        case PROF_REC_IBS_FETCH_EXT:
            // the common data including PID, thread ID, RIP, timestamp etc.
            pIBSFetch->m_PID = pPRDIBSRec->m_ProcessHandle;
            pIBSFetch->m_ProcessorID = pPRDIBSRec->m_ProcessorNumber;
            pIBSFetch->m_ThreadHandle = pPRDIBSRec->m_ThreadHandle;
            //linear address may be present in canonical or non-canonical form!
            pIBSFetch->m_RIP = (pPRDIBSRec->m_InstructionPointer & ERBT_713_NON_CANONICAL_MASK);

            //Start converting IBS fetch bits
            // m_IbsFetchCtlHigh stores IbsFetchCtl<63:32> (Fetch event flags/cycle counts)  */

            //  Bits 47:32  IbsFetchLat: instruction fetch latency.
            pIBSFetch->m_FetchLatency = (pPRDIBSRec->m_IbsFetchCtlHigh & 0xFFFF);

            //  Bit 48      IbsFetchEn: instruction fetch enable.
            //  Bit 49      IbsFetchVal: instruction fetch valid.
            // No use here

            //  Bit 50      IbsFetchComp: instruction fetch complete.
            pIBSFetch->m_FetchCompletion = (pPRDIBSRec->m_IbsFetchCtlHigh & FETCH_MASK_COMPLETE) ? true : false;

            //  Bit 51      IbsIcMiss: instruction cache miss.
            pIBSFetch->m_InstCacheMiss = (pPRDIBSRec->m_IbsFetchCtlHigh & FETCH_MASK_IC_MISS) ? true : false;

            //  Bit 52      IbsPhyAddrValid: instruction fetch physical address valid.
            pIBSFetch->m_PhysicalAddrValid = (pPRDIBSRec->m_IbsFetchCtlHigh & FETCH_MASK_PHY_ADDR) ? true : false;

            if (pIBSFetch->m_PhysicalAddrValid)
            {
                // Save the physical address from the extended part of the record
                if (pPRDIBSRecExt != NULL)
                {
                    // Baskar: FIXME: Should we handle canonical address form for physical address here?
                    pIBSFetch->m_PhysicalAddr = pPRDIBSRecExt->m_IbsFetchPhysAd & ERBT_713_NON_CANONICAL_MASK;
                }
                else
                {
                    pIBSFetch->m_PhysicalAddr = 0x0ULL;
                }
            }
            else
            {
                pIBSFetch->m_PhysicalAddr = 0xdeadbeefdeadbeefULL;
            }

            //  Bits 54:53  IbsL1TlbPgSz: instruction cache L1TLB page size.
            pIBSFetch->m_TLBPageSize = ((pPRDIBSRec->m_IbsFetchCtlHigh >> 21) & 0x3);

            //  Bit 55      IbsL1TlbMiss: instruction cache L1TLB miss.
            pIBSFetch->m_L1TLBMiss = (pPRDIBSRec->m_IbsFetchCtlHigh & FETCH_MASK_L1_MISS) ? true : false;

            //  Bit 56      IbsL2TlbMiss: instruction cache L2TLB miss.
            pIBSFetch->m_L2TLBMiss = (pPRDIBSRec->m_IbsFetchCtlHigh & FETCH_MASK_L2_MISS) ? true : false;

            pIBSFetch->m_Killed = (pPRDIBSRec->m_IbsFetchCtlHigh & FETCH_MASK_KILLED) ? false : true;

            pIBSFetch->m_InstCacheHit = (pIBSFetch->m_FetchCompletion && !pIBSFetch->m_InstCacheMiss);

            pIBSFetch->m_L1TLBHit = (!pIBSFetch->m_L1TLBMiss && pIBSFetch->m_PhysicalAddrValid);

            pIBSFetch->m_ITLB_L1M_L2H = (pIBSFetch->m_L1TLBMiss && !pIBSFetch->m_L2TLBMiss);

            pIBSFetch->m_ITLB_L1M_L2M = (pIBSFetch->m_L1TLBMiss && pIBSFetch->m_L2TLBMiss);

            pIBSFetch->m_weight = 1;
            hr = S_OK;

            break;

        default:
            break;
    }

    return hr;
}

HRESULT PrdReader::ConvertIBSFetchData(RawPRDRecord* pRawRecord1,
                                       RawPRDRecord* pRawRecord2,
                                       IBSFetchRecordData* pIBSFetch)

{
    HRESULT hr = E_INVALIDARG;

    if (0 == (m_profType & PROF_IBS))
    {
        return hr;
    }

    if (!pRawRecord1 || !pIBSFetch)
    {
        return hr;
    }

    // since PROF_REC_IBS_FETCH_BASIC or PROF_REC_IBS_FETCH_EXT records are identical for both
    // CA 2.9 and CA 3.0 (except name of member variables), I will convert it once here.
    hr = ConvertOldIBSFetchData(pRawRecord1, pRawRecord2, pIBSFetch);

    if (S_OK == hr)
    {
        if (m_version < V3_HDR_VERSION)
        {
            sTrcIbsFetchRecord* pFetch = (sTrcIbsFetchRecord*) pRawRecord1;
            // for CA 2.9 old records, convert timestamp to delta tick;
            pIBSFetch->m_DeltaTick = ConvertToDeltaTick(pFetch->m_TimeStampCounter);
            pIBSFetch->m_weight = 1;
        }
        else
        {
            PRD_IBS_FETCH_DATA_BASIC_RECORD* pFetch = (PRD_IBS_FETCH_DATA_BASIC_RECORD*) pRawRecord1;
            pIBSFetch->m_DeltaTick = pFetch->m_TickStamp;
            pIBSFetch->m_weight = GetWeight(pFetch->m_Core, IBS_FETCH_WEIGHT_BASE);

            // Values of new variables might not be available on all CPUs, reset them
            pIBSFetch->m_L2CacheMiss = false;
            pIBSFetch->m_ITLBRefillLatency = 0;

            if (FAMILY_OR == m_cpuFamily && 0x6 == (m_cpuModel >> 4))
            {
                pIBSFetch->m_L2CacheMiss = (pFetch->m_IbsFetchCtlHigh & FETCH_MASK_L2_CACHE_MISS) != 0;
            }

            if (PROF_REC_IBS_FETCH_EXT == pFetch->m_RecordType)
            {
                PRD_IBS_FETCH_DATA_EXT_RECORD* pFetchExt = (PRD_IBS_FETCH_DATA_EXT_RECORD*)pRawRecord2;

                // MSRC001_103C IBS Fetch Control Extended (IbsFetchCtlExtd)
                if (FAMILY_OR == m_cpuFamily && 0x6 == (m_cpuModel >> 4))
                {
                    pIBSFetch->m_ITLBRefillLatency = pFetchExt->m_IbsFetchCtlExtd;
                }
            }
        }
    }

    return hr;
}

// Convert raw sample data into an IBS op record
//
// typedef struct _TrcIbsOpRecord
// {
//   gtUByte      m_RecordType;           /* PROF_REC_IBS_OP_BASIC or PROF_REC_OP_EXT             */
//   gtUByte      m_ProcessorNumber;      /* Processor ID                                         */
//   gtUInt16     m_IbsOpDataHigh;        /* IbsOpdata <37:32> Branch/return/resync flags         */
//   gtUInt32      m_IbsOpDataLow;         /* IbsOpData<31:0> Macro-op retire cycle counts         */
//   gtUInt64    m_InstructionPointer;   /* IbsOpRip<63:0> Macro-op linear address               */
//   gtUInt64    m_TimeStampCounter;     /* Time stamp counter                                   */
//   gtUInt64    m_ProcessHandle;        /* Process ID                                           */
//   gtUInt64    m_ThreadHandle;         /* Thread ID                                            */
// } sTrcIbsOpRecord ;

// typedef struct _TrcIbsOpRecordExt
// {
//   gtUInt64    m_IbsOpData3;           /* IbsOpData3<63:0> Load/store flags/latency            */
//   gtUInt64    m_IbsDcLinAd;           /* IbsDcLinAd<63:0> IBS DC Linear Data Address          */
//   gtUInt64    m_IbsDcPhyAd;           /* IbsDcPhysAd<63:0> IBS DC Physical Data Address       */
//   gtUInt32      m_Reserved3;
//   gtUInt16     m_Reserved4;
//   gtUByte      m_Reserved5;
//   gtUByte      m_IbsOpData2;           /* IbsOpData3<5:0> Northbridge data                     */
//   gtUInt64    m_Reserved6;            /* Reserved for 64-bit branch target address            */
// } sTrcIbsOpRecordExt ;

HRESULT ConvertOldIBSOpData(RawPRDRecord* pRawRecord1,
                            RawPRDRecord* pRawRecord2, IBSOpRecordData* pIBSOp)
{
    HRESULT hr = E_FAIL;

    sTrcIbsOpRecord* pOpRaw = (sTrcIbsOpRecord*) pRawRecord1 ;
    sTrcIbsOpRecordExt* pOpRawExt = (sTrcIbsOpRecordExt*) pRawRecord2 ;

    // Function should only be called for op record conversion
    if (!((pOpRaw->m_RecordType == PROF_REC_IBS_OP_BASIC) ||
          (pOpRaw->m_RecordType == PROF_REC_IBS_OP_EXT)))
    {
        return (hr) ;
    }

    // Convert raw basic op data
    pIBSOp->m_PID          = pOpRaw->m_ProcessHandle ;
    pIBSOp->m_ThreadHandle = pOpRaw->m_ThreadHandle ;
    //pIBSOp->m_DeltaTick    = (pOpRaw->m_TimeStampCounter ;

    // Address may be present in canonical or non-canonical form!
    pIBSOp->m_RIP = pOpRaw->m_InstructionPointer & ERBT_713_NON_CANONICAL_MASK;

    pIBSOp->m_ProcessorID  = pOpRaw->m_ProcessorNumber ;

    //  Field m_IbsOpDataHigh contains IbsOpdata <37:32>
    //  Field m_IbsOpDataLow contains IbsOpData<31:0> (Macro-op retire cycle counts)
    //
    //  MSRC001_1035 IBS Op Data Register (IbsOpData)
    //  15:0    IbsCompToRetCtr: macro-op completion to retire count. This field returns
    //          the number of cycles from when the macro-op was completed to when the macro-op
    //          was retired.
    pIBSOp->m_CompToRetireCycles = (pOpRaw->m_IbsOpDataLow & BR_MASK_RETIRE);

    //  31:16   IbsTagToRetCtr: macro-op tag to retire count. This field returns the number of
    //          cycles from when the macro-op was tagged to when the macro-op was retired.
    //          This field will be equal to IbsCompToRetCtr when the tagged macro-op is a NOP.
    pIBSOp->m_TagToRetireCycles = (pOpRaw->m_IbsOpDataLow >> 16) & BR_MASK_RETIRE;

    //  32      IbsOpBrnResync: resync macro-op.
    pIBSOp->m_OpBranchResync = (pOpRaw->m_IbsOpDataHigh & BR_MASK_BRN_RESYNC) != 0 ;
    //  33      IbsOpMispReturn: mispredicted return macro-op.
    pIBSOp->m_OpMispredictedReturn = (pOpRaw->m_IbsOpDataHigh & BR_MASK_MISP_RETURN) != 0 ;
    //  34      IbsOpReturn: return macro-op.
    pIBSOp->m_OpReturn = (pOpRaw->m_IbsOpDataHigh & BR_MASK_RETURN) != 0 ;
    //  35      IbsOpBrnTaken: taken branch macro-op.
    pIBSOp->m_OpBranchTaken = (pOpRaw->m_IbsOpDataHigh & BR_MASK_BRN_TAKEN) != 0 ;
    //  36      IbsOpBrnMisp: mispredicted branch macro-op.
    pIBSOp->m_OpBranchMispredicted = (pOpRaw->m_IbsOpDataHigh & BR_MASK_BRN_MISP) != 0 ;
    //  37      IbsOpBrnRet: branch macro-op retired.
    pIBSOp->m_OpBranchRetired = (pOpRaw->m_IbsOpDataHigh & BR_MASK_BRN_RET) != 0 ;

    // Return status should be good for both basic and extended IBS records
    hr = S_OK ;

    // Clear remaining fields and take an early return if the
    // the raw record is an IBS op basic record
    if (pOpRaw->m_RecordType == PROF_REC_IBS_OP_BASIC)
    {
        // Clear key flags to disable interpretation of fields with
        // invalid IBS data in them
        pIBSOp->m_NbIbsReqSrc       = 0 ;
        pIBSOp->m_IbsLdOp           = false ;
        pIBSOp->m_IbsStOp           = false ;
        pIBSOp->m_IbsDcLinAddrValid = false ;
        pIBSOp->m_IbsDcPhyAddrValid = false ;
        return (hr) ;
    }

    // Linear address may be present in canonical or non-canonical form
    //  MSRC001_1038 IBS DC Linear Address Register (IbsDcLinAd)
    pIBSOp->m_IbsDcLinAd = pOpRawExt->m_IbsDcLinAd & ERBT_713_NON_CANONICAL_MASK;

    //  MSRC001_1039 IBS DC Physical Address Register (IbsDcPhysAd)
    pIBSOp->m_IbsDcPhysAd = pOpRawExt->m_IbsDcPhyAd;

    //  MSRC001_1036 IBS Op Data 2 Register (IbsOpData2)
    //  5       NbIbsReqCacheHitSt: IBS L3 cache state
    pIBSOp->m_NbIbsCacheHitSt = ((pOpRawExt->m_IbsOpData2 & NB_MASK_L3_STATE) != 0) ;
    //  4       NbIbsReqDstProc: IBS request destination processor
    pIBSOp->m_NbIbsReqDstProc = ((pOpRawExt->m_IbsOpData2 & NB_MASK_REQ_DST_PROC) != 0) ;

    //  2:0     NbIbsReqSrc: Northbridge IBS request data source
    pIBSOp->m_NbIbsReqSrc = (pOpRawExt->m_IbsOpData2 & NB_MASK_REQ_DATA_SRC) ;

    //  MSRC001_1037 IBS Op Data3 Register (IbsOpData3)
    //  48:32   IbsDcMissLat
    pIBSOp->m_IbsDcMissLat = ((pOpRawExt->m_IbsOpData3 >> 32) & 0xFFFF) ;
    //  0       IbsLdOp: Load op
    pIBSOp->m_IbsLdOp = ((pOpRawExt->m_IbsOpData3 & DC_MASK_LOAD_OP) != 0) ;
    //  1       IbsStOp: Store op
    pIBSOp->m_IbsStOp = ((pOpRawExt->m_IbsOpData3 & DC_MASK_STORE_OP) != 0) ;
    //  2       IbsDcL1TlbMiss: Data cache L1TLB miss
    pIBSOp->m_IbsDcL1tlbMiss = ((pOpRawExt->m_IbsOpData3 & DC_MASK_L1_TLB_MISS) != 0) ;
    //  3       IbsDcL2tlbMiss: Data cache L2TLB miss
    pIBSOp->m_IbsDcL2tlbMiss = ((pOpRawExt->m_IbsOpData3 & DC_MASK_L2_TLB_MISS) != 0) ;
    //  4       IbsDcL1tlbHit2M: Data cache L1TLB hit in 2M page
    pIBSOp->m_IbsDcL1tlbHit2M = ((pOpRawExt->m_IbsOpData3 & DC_MASK_L1_HIT_2M) != 0) ;
    //  5       IbsDcL1tlbHit1G: Data cache L1TLB hit in 1G page
    pIBSOp->m_IbsDcL1tlbHit1G = ((pOpRawExt->m_IbsOpData3 & DC_MASK_L1_HIT_1G) != 0) ;
    //  6       IbsDcL2tlbHit2M: Data cache L2TLB hit in 2M page
    pIBSOp->m_IbsDcL2tlbHit2M = ((pOpRawExt->m_IbsOpData3 & DC_MASK_L2_HIT_2M) != 0) ;
    //  7       IbsDcMiss: Data cache miss
    pIBSOp->m_IbsDcMiss = ((pOpRawExt->m_IbsOpData3 & DC_MASK_DC_MISS) != 0) ;
    //  8       IbsDcMisAcc: Misaligned access
    pIBSOp->m_IbsDcMisAcc = ((pOpRawExt->m_IbsOpData3 & DC_MASK_MISALIGN_ACCESS) != 0) ;
    //  9       IbsDcLdBnkCon: Bank conflict on load operation
    pIBSOp->m_IbsDcLdBnkCon = ((pOpRawExt->m_IbsOpData3 & DC_MASK_LD_BANK_CONFLICT) != 0) ;
    //  10      IbsDcStBnkCon: Bank conflict on store operation
    pIBSOp->m_IbsDcStBnkCon = ((pOpRawExt->m_IbsOpData3 & DC_MASK_ST_BANK_CONFLICT) != 0) ;
    //  11      IbsDcStToLdFwd: Data forwarded from store to load operation
    pIBSOp->m_IbsDcStToLdFwd = ((pOpRawExt->m_IbsOpData3 & DC_MASK_ST_TO_LD_FOR) != 0) ;
    //  12      IbsDcDcStToLdCan: Data forwarding from store to load operation cancelled
    pIBSOp->m_IbsDcStToLdCan = ((pOpRawExt->m_IbsOpData3 & DC_MASK_ST_TO_LD_CANCEL) != 0) ;
    //  13      IbsDcWcMemAcc: WC memory access
    pIBSOp->m_IbsDcWcMemAcc = ((pOpRawExt->m_IbsOpData3 & DC_MASK_WC_MEM_ACCESS) != 0) ;
    //  14      IbsDcDcUcMemAcc: UC memory access
    pIBSOp->m_IbsDcUcMemAcc = ((pOpRawExt->m_IbsOpData3 & DC_MASK_UC_MEM_ACCESS) != 0) ;
    //  15      IbsDcLockedOp: Locked operation
    pIBSOp->m_IbsDcLockedOp = ((pOpRawExt->m_IbsOpData3 & DC_MASK_LOCKED_OP) != 0) ;
    //  16      IbsDcMabHit: MAB hit
    pIBSOp->m_IbsDcMabHit = ((pOpRawExt->m_IbsOpData3 & DC_MASK_MAB_HIT) != 0) ;
    //  17      IbsDcLinAddrValid: Data cache linear address valid
    pIBSOp->m_IbsDcLinAddrValid = ((pOpRawExt->m_IbsOpData3 & DC_MASK_LIN_ADDR_VALID) != 0) ;
    //  18      IbsDcPhyAddrValid: Data cache physical address valid
    pIBSOp->m_IbsDcPhyAddrValid = ((pOpRawExt->m_IbsOpData3 & DC_MASK_PHY_ADDR_VALID) != 0) ;
    //  19      IbsDcL2tlbHit1G: Data cache L2TLB hit in 1G page
    pIBSOp->m_IbsDcL2tlbHit1G = ((pOpRawExt->m_IbsOpData3 & DC_MASK_L2_HIT_1G) != 0) ;

    return (hr) ;
}

HRESULT ConvertRawIBSOpData(RawPRDRecord* pRawRecord1, RawPRDRecord* pRawRecord2,
                            IBSOpRecordData* pIBSOp, int cpuFamily, int cpuModel)
{
    HRESULT hr = E_FAIL;
    hr = ConvertOldIBSOpData(pRawRecord1, pRawRecord2, pIBSOp);

    PRD_IBS_OP_DATA_BASIC_RECORD* pOpRaw = (PRD_IBS_OP_DATA_BASIC_RECORD*) pRawRecord1 ;

    // Value of new variable might not be available on all CPUs, reset it
    pIBSOp->m_IbsOpLdResync = false;

    if (PROF_REC_IBS_OP_EXT == pOpRaw->m_RecordType)
    {
        PRD_IBS_OP_DATA_EXT_RECORD* pOpRawExt = (PRD_IBS_OP_DATA_EXT_RECORD*) pRawRecord2 ;

        //  MSRC001_103B IBS Branch Target Address (IbsBrTarget)
        // Valid when branch tagged op and and !0
        if (pIBSOp->m_OpBranchRetired && (0 != pOpRawExt->m_IbsBrTarget))
        {
            pIBSOp->m_BranchTarget = pOpRawExt->m_IbsBrTarget;
        }

        // MSRC001_103D IBS Op Data 4 (IbsOpData4)
        if (FAMILY_OR == cpuFamily && 0x6 == (cpuModel >> 4))
        {
            pIBSOp->m_IbsOpLdResync = (pOpRawExt->m_IbsOpData4 & DC_MASK_LD_RESYNC) != 0;
        }
    }

    return (hr) ;
}


HRESULT PrdReader::ConvertIBSOpData(RawPRDRecord* pRawRecord1,
                                    RawPRDRecord* pRawRecord2, IBSOpRecordData* pIBSOp)
{
    HRESULT hr = E_FAIL;

    // All pointer arguments should be defined
    if (!pRawRecord1 || !pRawRecord2 || !pIBSOp)
    {
        return (hr) ;
    }

    if (0 == (m_profType & PROF_IBS))
    {
        return E_INVALIDARG;
    }

    // since PROF_REC_IBS_FETCH_BASIC or PROF_REC_IBS_FETCH_EXT records are identical for both
    // CA 2.9 and CA 3.0 (except name of member variables), I will convert it once here.
    if (m_version < V3_HDR_VERSION)
    {
        hr = ConvertOldIBSOpData(pRawRecord1, pRawRecord2, pIBSOp);
        sTrcIbsOpRecord* pOpBasic = (sTrcIbsOpRecord*) pRawRecord1;
        // for CA 2.9 old records, convert timestamp to tick;
        pIBSOp->m_DeltaTick = ConvertToDeltaTick(pOpBasic->m_TimeStampCounter);
        pIBSOp->m_weight = 1;
    }
    else
    {
        hr = ConvertRawIBSOpData(pRawRecord1, pRawRecord2, pIBSOp, GetCpuFamily(), GetCpuModel());
        PRD_IBS_OP_DATA_BASIC_RECORD* pOpBasic = (PRD_IBS_OP_DATA_BASIC_RECORD*) pRawRecord1;
        pIBSOp->m_DeltaTick = pOpBasic->m_TickStamp;
        pIBSOp->m_weight = GetWeight(pOpBasic->m_Core, IBS_OP_WEIGHT_BASE);
    }

    return hr;
}


gtUInt64 PrdReader::ConvertMissedCount(RawPRDRecord rawRecord)
{
    gtUInt64 retVal = 0;

    if (PROF_REC_MISSED != rawRecord.rawRecordsData[0])
    {
        return retVal;
    }

    if (m_version < V3_HDR_VERSION)
    {
        // NOTE: the old prd record, sTrcMissedCountRecord, only count missed samples for EBP.

        sTrcMissedCountRecord* pMissedRec   = (sTrcMissedCountRecord*) rawRecord.rawRecordsData;

        for (unsigned int i = 0; i < MAX_COUNTERS; i++)
        {
            retVal += pMissedRec->m_MissedEvent[i];
        }

        if (m_endTick < pMissedRec->m_TSC)
        {
            m_endTick = pMissedRec->m_TSC;
        }
    }
    else
    {
        PRD_MISSED_DATA_RECORD* pMissedRec = (PRD_MISSED_DATA_RECORD*) rawRecord.rawRecordsData;
        retVal += pMissedRec->m_MissedCount;

        if (pMissedRec->m_IbsFetchMaxCnt)
        {
            retVal += pMissedRec->m_MissedFetchCount;
        }

        if (m_endTick < pMissedRec->m_TickStamp)
        {
            m_endTick = pMissedRec->m_TickStamp;
        }
    }

    return retVal;
}


HRESULT PrdReader::ConvertCpuFamily(gtUInt32  Fn0000_0001_EAX)
{
    //Assume that all cpus are of the same family as the first one.
    m_cpuFamily = 0x0f & (Fn0000_0001_EAX >> 8);

    if (0xf == m_cpuFamily)
    {
        //extended family in bits 27-20
        m_cpuFamily += 0xff & (Fn0000_0001_EAX >> 20);
    }

    //bits 19:16 , bits 7:4
    m_cpuModel = (0xf0 & (Fn0000_0001_EAX >> 12)) | (0x0f & (Fn0000_0001_EAX >>  4));
    return S_OK;
}


int PrdReader::GetCoreCount() const
{
    return m_coreCount;
}


void PrdReader::UpdateResWeight(RawPRDRecord* pRawRecord)
{
    WeightKey k;
    gtUByte* pIndices;
    gtUByte* pWeights;
    unsigned int maxResourceType;
    unsigned int numberOfWeights;

    unsigned int numOfPhyCtr = 4;
    unsigned int numOfPhyNBCtr = 0;
    unsigned int numOfPhyL2ICtr = 0;

    // Baskar: Workaround to check, whether we have the proper PRD file to do MT-PRD Processing.
    unsigned int cnt;

    if (m_version >= CXL_HDR_VERSION)
    {
        PRD_WEIGHT_RECORD* pW = (PRD_WEIGHT_RECORD*) pRawRecord->rawRecordsData;
        k.core = pW->m_Core;
        cnt = static_cast<unsigned int>(pW->m_BufferRecordCount);

        pIndices = pW->m_indexes;
        pWeights = pW->m_Weights;
        maxResourceType = CXL_MAX_RES_TYPE;
        numberOfWeights = CXL_NBR_OF_WEIGHTS;
    }
    else
    {
        sTrcCaWeightRecord* pW = (sTrcCaWeightRecord*) pRawRecord->rawRecordsData;
        k.core = pW->m_Core;
        cnt = static_cast<unsigned int>(pW->m_BufferRecordCount);

        pIndices = pW->m_indexes;
        pWeights = pW->m_Weights;
        maxResourceType = CA_MAX_RES_TYPE;
        numberOfWeights = CA_NBR_OF_WEIGHTS;
    }

    if (0U != cnt)
    {
        m_weightRecHasBufferCount = true;
    }

    if (m_cpuFamily >= 0x15)
    {
        if (m_cpuFamily == 0x16)
        {
            numOfPhyCtr = 4;
            numOfPhyL2ICtr = 4;
        }
        else
        {
            numOfPhyCtr = 6;
        }

        numOfPhyNBCtr = 4;
    }

    for (unsigned int i = 0; i < maxResourceType; i++)
    {
        unsigned int base = 0;
        unsigned int iteration = 1;

        switch (i)
        {
            case APIC:
                base = APIC_WEIGHT_BASE;
                break;

            case EVENT_CTR:
                iteration = numOfPhyCtr;
                base = PMC_WEIGHT_BASE;
                break;

            case NB_CTR:
                iteration = numOfPhyNBCtr;
                base = NB_PMC_WEIGHT_BASE;
                break;

            case L2I_CTR:
                iteration = numOfPhyL2ICtr;
                base = L2I_PMC_WEIGHT_BASE;
                break;

            case IBS_FETCH:
                base = IBS_FETCH_WEIGHT_BASE;
                break;

            case IBS_OP:
                base = IBS_OP_WEIGHT_BASE;
                break;
        }

        for (unsigned int j = 0; j < iteration; j++)
        {
            unsigned int index = pIndices[i] + j;

            if (index < numberOfWeights)
            {
                k.res_index = base + j;
                WeightMap::iterator it = m_WeightMap.find(k);

                if (it != m_WeightMap.end())
                {
                    it->second = pWeights[index];
                }
                else
                {
                    m_WeightMap.insert(WeightMap::value_type(k, pWeights[index]));
                }
            }
        }
    }

}

unsigned int PrdReader::GetWeight(unsigned int core, unsigned int res_index)
{
    unsigned int w = 1;
    WeightKey k(core, res_index);
    WeightMap::const_iterator it = m_WeightMap.find(k);

    if (it != m_WeightMap.end())
    {
        w = it->second;
    }

    return w;
}

// return number of millisecond since the profile start (start time is indicated in PRD header)
//
gtUInt64 PrdReader::ConvertToDeltaTick(gtUInt64 timestamp)
{
    gtUInt64 deltaTick = 0;

    if (!m_ClockSpeed)
    {
        return deltaTick;
    }

    if (timestamp > m_startTick)
    {
        //
        gtUInt64 deltaTSC = timestamp - m_startTick;

        if (0 == m_hrFreq)
        {
            //low resolution ticks
            deltaTick = (deltaTSC / (m_ClockSpeed * 1000));
        }
        else
        {
            //high resolution ticks
            deltaTick = (deltaTSC * 1000) / m_hrFreq;
        }
    }

    return deltaTick;
}


void PrdReader::UpdatePIDFilter(RawPRDRecord* pRawRecord)
{
    PRD_PID_CONFIG_RECORD* pRec = (PRD_PID_CONFIG_RECORD*) pRawRecord->rawRecordsData;

    for (unsigned int i = 0; i < PID_DATA_PER_RECORD; i++)
    {
        if (pRec->m_PID_Array[i])
        {
            m_pidList.push_back(pRec->m_PID_Array[i]);
        }
    }
}

void PrdReader::UpdateCPUIDRec(RawPRDRecord* pRawRecord)
{
    if (m_version >= V3_HDR_VERSION)
    {
        PRD_CPUINFO_RECORD* pCpuRec = (PRD_CPUINFO_RECORD*)(pRawRecord->rawRecordsData);

        switch (pCpuRec->m_CpuId_function)
        {
            case 0x00000001:
                // CPUID Fn0000_0001_EAX Family, Model, Stepping Identifiers
                ConvertCpuFamily(pCpuRec->aRegisterInfo[EAX_OFF]);
                break;

            case 0x80000005:
                // CPUID Fn8000_0005_ECX L1 Data Cache Identifiers
                m_L1DcSize = (pCpuRec->aRegisterInfo[ECX_OFF] >> 24) & 0xFF;
                m_L1DcAssoc = (pCpuRec->aRegisterInfo[ECX_OFF] >> 16) & 0xFF;
                m_L1DcLinesPerTag = (pCpuRec->aRegisterInfo[ECX_OFF] >> 8) & 0xFF;
                m_L1DcLineSize = pCpuRec->aRegisterInfo[ECX_OFF] & 0xFF;
                break;

            default:
                //Other cpuid functions not currently used
                break;
        }
    }
    else
    {
        sTrcCPUInfoRecord* pCpuRec = (sTrcCPUInfoRecord*)(pRawRecord->rawRecordsData);
        ConvertCpuFamily(pCpuRec->socket[0].m_FamilyModel);

        // since  pCpuRec->socket[0].m_ClockSpeed is Clock speed of 100MHZ);
        m_ClockSpeed = pCpuRec->socket[0].m_ClockSpeed * 100;

        m_coreCount = 0;

        for (int i = 0; i < 5; i++)
        {
            if (0x80 != pCpuRec->socket[i].m_NumCPUs)
            {
                m_coreCount += pCpuRec->socket[i].m_NumCPUs;
            }
        }
    }
}

void PrdReader::UpdateIBSConfigRec(RawPRDRecord* pRawRecord)
{
    if (m_version >= V3_HDR_VERSION)
    {
        // new in CA 3.0
        /* IBS Configuration. */
        m_profType |= PROF_IBS;
        PRD_IBS_CONFIG_RECORD* pIbsCfg =
            (PRD_IBS_CONFIG_RECORD*)(pRawRecord->rawRecordsData);

        if (0 != pIbsCfg->m_IbsFetchCtl)
        {
            m_IbsFetchCtl = pIbsCfg->m_IbsFetchCtl;
        }

        if (0 != pIbsCfg->m_IbsOpCtl)
        {
            m_IbsOpCtl = pIbsCfg->m_IbsOpCtl;
        }

        if (m_endTick < pIbsCfg->m_TickStamp)
        {
            m_endTick = pIbsCfg->m_TickStamp;
        }
    }
    else
    {
        /* IBS Configuration. */
        m_profType |= PROF_IBS;
        sTrcIbsConfRecord* pIbsCfg = (sTrcIbsConfRecord*)(pRawRecord->rawRecordsData);

        if (pIbsCfg->m_IbsFetchMaxCnt)
        {
            m_IbsFetchCtl = pIbsCfg->m_IbsFetchMaxCnt >> 4;
        }

        if (pIbsCfg->m_IbsOpMaxCnt)
        {
            m_IbsOpCtl = pIbsCfg->m_IbsOpMaxCnt >> 4;
        }

        if (m_endTick < pIbsCfg->m_TimeStampCounter)
        {
            m_endTick = pIbsCfg->m_TimeStampCounter;
        }
    }

}
void PrdReader::UpdateTimerConfigRec(RawPRDRecord* pRawRecord)
{
    m_profType |= PROF_TBP;

    if (m_version >= V3_HDR_VERSION)
    {
        // new format in CA 3.0
        PRD_APIC_CONFIG_RECORD* pTimerCfg =
            (PRD_APIC_CONFIG_RECORD*)(pRawRecord->rawRecordsData);
        m_TimerResolution = pTimerCfg->m_TimerGranularity;

        if (m_endTick < pTimerCfg->m_TickStamp)
        {
            m_endTick = pTimerCfg->m_TickStamp;
        }
    }
    else
    {
        sTrcTimerConfRecord* pTimerCfg = (sTrcTimerConfRecord*)(pRawRecord->rawRecordsData);
        m_TimerResolution = pTimerCfg->m_TimerGranularity;

        if (m_endTick < pTimerCfg->m_TimeStampCounter)
        {
            m_endTick = pTimerCfg->m_TimeStampCounter;
        }
    }
}

void PrdReader::AdjustOldEventConfig()
{
    for (PrdRDEventMap::iterator it = m_EventMap.begin(), itEnd = m_EventMap.end(); it != itEnd; ++it)
    {
        it->second.numApperance = it->second.numApperance / m_coreCount;

        if (0 == it->second.numApperance)
        {
            it->second.numApperance = 1;
        }
    }

}

void PrdReader::UpdateEventConfigRec(RawPRDRecord* pRawRecord)
{
    m_profType |= PROF_EBP;

    if (m_version >= V3_HDR_VERSION)
    {
        // new format in CA 3.0
        PRD_EVENT_CTR_CONFIG_RECORD* pEvtConfRecord =
            (PRD_EVENT_CTR_CONFIG_RECORD*)(pRawRecord->rawRecordsData);
        EventKey eKey;
        EventData eData;

        m_EventGroupCount = 1;

        for (int core = 0; core < m_coreCount; core++)
        {
            if ((0 != pEvtConfRecord->m_CoreMask) &&
                (((pEvtConfRecord->m_CoreMask >> core) & 0x01) == 0x0))
            {
                continue;
            }

            eKey.core = core;
            eKey.ctl.perf_ctl = pEvtConfRecord->m_EventSelReg;

            PrdRDEventMap::iterator it = m_EventMap.find(eKey);

            if (it == m_EventMap.end())
            {
                eData.ctr = pEvtConfRecord->m_EventCount;
                eData.numApperance = 1;
                m_EventMap.insert(PrdRDEventMap::value_type(eKey, eData));
            }
            else
            {
                it->second.numApperance++;
            }
        }

        if (m_endTick < pEvtConfRecord->m_TickStamp)
        {
            m_endTick = pEvtConfRecord->m_TickStamp;
        }

        // all event multiplex will share the same event mulitplexing period clock ticks
        m_EMTicks = 0;
    }
    else
    {
        // old formart

        // kludge:
        // Fact: when user configures 2 group of total 8 events, the CA 2.9 driver will write
        // 8 * numOfCore sTrcEvtConfRecord at the begin of prd file. Notice that the sTrcEvtConfRecord
        // does not contain core info. We will keep all event apperance at here.
        // At the end of configure events, we will divid apperance by numb of cores
        // Lei /3/31/2010
        //
        sTrcEvtConfRecord* pEvtConfRecord = (sTrcEvtConfRecord*)(pRawRecord->rawRecordsData);

        if (m_EventGroupCount <= pEvtConfRecord->m_GroupIndex)
        {
            m_EventGroupCount = pEvtConfRecord->m_GroupIndex + 1;
        }

        EventKey eKey;
        EventData eData;

        for (int core = 0; core < m_coreCount; core++)
        {
            eKey.core = core;
            eKey.ctl.perf_ctl = pEvtConfRecord->m_EventSelReg;

            PrdRDEventMap::iterator it = m_EventMap.find(eKey);

            if (it == m_EventMap.end())
            {
                eData.ctr = pEvtConfRecord->m_EventCount;
                eData.numApperance = 1;
                m_EventMap.insert(PrdRDEventMap::value_type(eKey, eData));
            }
            else
            {
                it->second.numApperance++;
            }
        }

        // all event multiplex will share the same event mulitplexing period clock ticks
        m_EMTicks = pEvtConfRecord->m_EvtMultiplexTicks;

        if (m_endTick < pEvtConfRecord->m_TimeStampCounter)
        {
            m_endTick = pEvtConfRecord->m_TimeStampCounter;
        }
    }

}


// reutrn number of seconds;
unsigned int PrdReader::GetProfileDuration() const
{
    ULARGE_INTEGER start, end;// = relative;
    start.HighPart = m_startMark.dwHighDateTime;
    start.LowPart = m_startMark.dwLowDateTime;
    end.HighPart = m_endMark.dwHighDateTime;
    end.LowPart = m_endMark.dwLowDateTime;

    ULONGLONG diff = end.QuadPart - start.QuadPart;
    diff /= 10000000; //1 S

    return static_cast<unsigned int>(diff);
}


long PrdReader::GetBytesRead()
{

    if (m_hPRDFile)
    {
        return ftell(m_hPRDFile);
    }
    else
    {
        return 0;
    }
}

void PrdReader::UpdateTopologyRec(RawPRDRecord* pRawRecord)
{
    if (NULL == m_aTopology)
    {
        //allocate on the 1st topology record
        m_aTopology = new CORE_TOPOLOGY[m_coreCount];

        if (NULL == m_aTopology)
        {
            return;
        }

        memset(m_aTopology, 0, (sizeof(CORE_TOPOLOGY) * m_coreCount));
    }

    PRD_TOPOLOGY_RECORD* pTopRec = (PRD_TOPOLOGY_RECORD*)(pRawRecord->rawRecordsData);

    for (int i = 0; i < TOPOLOGY_DATA_PER_RECORD; i++)
    {
        // ToDo: Need to find out how Windows handles core mapping when
        // some cores are offline
        if (((0 != i) && (0 != pTopRec->m_Core[i])) ||
            ((0 == i) && (0 == (pTopRec->m_Core[i] % TOPOLOGY_DATA_PER_RECORD))))
        {
            m_aTopology[pTopRec->m_Core[i]].processor = pTopRec->m_Processor[i];
            m_aTopology[pTopRec->m_Core[i]].numaNode = pTopRec->m_Node[i];
        }
    }
}

bool PrdReader::GetTopology(unsigned int core, gtUInt16* pProcessor, gtUInt16* pNode) const
{
    if (NULL == m_aTopology || core > static_cast<unsigned int>(m_coreCount) || NULL == pProcessor || NULL == pNode)
    {
        return false;
    }

    *pProcessor = m_aTopology[core].processor;
    *pNode = m_aTopology[core].numaNode;
    return true;
}


//
// Class PrdReaderThread
//

// PrdReaderThread - Constructor
//
PrdReaderThread::PrdReaderThread(PrdReader& prdReader) : m_prdReader(prdReader)
{
    SetPRDBufferValues(0, 0, 0, 0);
}


PrdReaderThread::PrdReaderThread(
    PrdReader& prdReader,
    gtUInt64 baseAddress,
    gtUInt32 numRecords,
    gtUInt64 startTick,
    gtUInt64 endTick
) : m_prdReader(prdReader)
{
    SetPRDBufferValues(baseAddress, numRecords, startTick, endTick);
}


// PrdReaderThread - Destructor
//
PrdReaderThread::~PrdReaderThread()
{
    m_WeightMap.clear();
}


// PrdReaderThread - Member function to handle PROC_REF_WEIGHT records in PRD file
//
unsigned int
PrdReaderThread::GetWeight(unsigned int core, unsigned int res_index)
{
    unsigned int w = 1;
    WeightKey k(core, res_index);
    WeightMap::const_iterator it = m_WeightMap.find(k);

    if (it != m_WeightMap.end())
    {
        w = it->second;
    }

    return w;
}


// return number of millisecond since the profile start (start time is indicated in prd header)
//
gtUInt64 PrdReaderThread::ConvertToDeltaTick(gtUInt64 timestamp)
{
    gtUInt64 deltaTick = 0;

    if (!m_prdReader.GetClockSpeed())
    {
        return deltaTick;
    }

    if (timestamp > m_startTick)
    {
        gtUInt64 deltaTSC = timestamp - m_startTick;

        if (0 == m_prdReader.GetHrFreq())
        {
            //low resolution ticks
            deltaTick = (deltaTSC / (m_prdReader.GetClockSpeed() * 1000));
        }
        else
        {
            //high resolution ticks
            deltaTick = (deltaTSC * 1000) / m_prdReader.GetHrFreq();
        }
    }

    return deltaTick;
}


void PrdReaderThread::UpdateResWeight(RawPRDRecord* pRawRecord)
{
    unsigned int numOfPhyCtr = NBR_OF_PHY_CTR;
    unsigned int numOfPhyNBCtr = NBR_OF_PHY_NB_CTR;
    unsigned int numOfPhyL2ICtr = NBR_OF_PHY_L2I_CTR;

    int cpuFamily = m_prdReader.GetCpuFamily();

    if (cpuFamily >= 0x15)
    {
        if (cpuFamily == 0x16)
        {
            numOfPhyCtr =  NBR_OF_PHY_CTR;
            numOfPhyL2ICtr = NBR_OF_PHY_L2I_CTR_FAMILY16;
        }
        else
        {
            numOfPhyCtr = NBR_OF_PHY_CTR_FAMILY15;
        }

        numOfPhyNBCtr = NBR_OF_PHY_NB_CTR_FAMILY15;
    }

    WeightKey k;
    gtUByte* pIndices;
    gtUByte* pWeights;
    unsigned int maxResourceType;
    unsigned int numberOfWeights;

    if (m_prdReader.GetProfileVersion() >= CXL_HDR_VERSION)
    {
        PRD_WEIGHT_RECORD* pW = (PRD_WEIGHT_RECORD*) pRawRecord->rawRecordsData;
        k.core = pW->m_Core;

        pIndices = pW->m_indexes;
        pWeights = pW->m_Weights;
        maxResourceType = CXL_MAX_RES_TYPE;
        numberOfWeights = CXL_NBR_OF_WEIGHTS;
    }
    else
    {
        sTrcCaWeightRecord* pW = (sTrcCaWeightRecord*) pRawRecord->rawRecordsData;
        k.core = pW->m_Core;

        pIndices = pW->m_indexes;
        pWeights = pW->m_Weights;
        maxResourceType = CA_MAX_RES_TYPE;
        numberOfWeights = CA_NBR_OF_WEIGHTS;
    }

    for (unsigned int i = 0; i < maxResourceType; i++)
    {
        unsigned int base = 0;
        unsigned int iteration = 1;

        switch (i)
        {
            case APIC:
                base = APIC_WEIGHT_BASE;
                break;

            case EVENT_CTR:
                iteration = numOfPhyCtr;
                base = PMC_WEIGHT_BASE;
                break;

            case NB_CTR:
                iteration = numOfPhyNBCtr;
                base = NB_PMC_WEIGHT_BASE;
                break;

            case L2I_CTR:
                iteration = numOfPhyL2ICtr;
                base = L2I_PMC_WEIGHT_BASE;
                break;

            case IBS_FETCH:
                base = IBS_FETCH_WEIGHT_BASE;
                break;

            case IBS_OP:
                base = IBS_OP_WEIGHT_BASE;
                break;
        }

        for (unsigned int j = 0; j < iteration; j++)
        {
            unsigned int index = pIndices[i] + j;

            if (index < numberOfWeights)
            {
                k.res_index = base + j;
                WeightMap::iterator it = m_WeightMap.find(k);

                if (it != m_WeightMap.end())
                {
                    it->second = pWeights[index];
                }
                else
                {
                    // As per Frank's suggestion, we don't need to insert 0 weights.
                    if (pWeights[index])
                    {
                        m_WeightMap.insert(WeightMap::value_type(k, pWeights[index]));
                    }
                }
            }
        }
    }
}

unsigned int PrdReaderThread::rawRead(RawPRDRecord* pRawRec1)
{
    if (pRawRec1)
    {
        if (m_recordsRead >= m_numRecords)
        {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Records read(%d) is >= Total Records(%d).", m_recordsRead, m_numRecords);
#endif
            return (unsigned int)E_FAIL;
        }

        memcpy((void*)pRawRec1->rawRecordsData, (const void*)(m_baseAddress + (m_recordsRead * sizeof(RawPRDRecord))), CAPRDRECORDSIZE);

        m_recordsRead++;

        return CAPRDRECORDSIZE;
    }

    return 0;
}


RawPRDRecord* PrdReaderThread::GetLastRecord()
{
    return reinterpret_cast<RawPRDRecord*>(m_baseAddress + ((m_recordsRead - 1) * sizeof(RawPRDRecord)));
}


PRD_RECORD_TYPE PrdReaderThread::PeekNextRecordType() const
{
    return (m_recordsRead < m_numRecords) ?
           static_cast<PRD_RECORD_TYPE>(*reinterpret_cast<const char*>(m_baseAddress + (m_recordsRead * sizeof(RawPRDRecord)))) :
           PROF_REC_INVALID;
}


bool PrdReaderThread::SkipRawRecords(unsigned int count)
{
    bool ret = ((m_recordsRead + count) <= m_numRecords);

    if (ret)
    {
        m_recordsRead += count;
    }

    return ret;
}


HRESULT PrdReaderThread::GetNextRawRecord(RawPRDRecord* pRawDataRec)
{
    HRESULT hr = E_FAIL;
    unsigned int bytes_read;

    if (!pRawDataRec || !m_baseAddress)
    {
        return hr;
    }

    bytes_read = rawRead(pRawDataRec);

    if (CAPRDRECORDSIZE == bytes_read)
    {
        hr = S_OK;
    }

    return hr;
}


HRESULT PrdReaderThread::GetNextRawRecords(RawPRDRecord* pRawRec1, RawPRDRecord* pRawRec2, unsigned int* pRecNum)
{
    HRESULT hr = E_FAIL;
    unsigned int bytes_read;

    if (!pRawRec1 || !pRawRec2 || !pRecNum || !m_baseAddress)
    {
        return hr;
    }

    bytes_read = rawRead(pRawRec1);

    if (CAPRDRECORDSIZE == bytes_read)
    {
        *pRecNum = 1;
        hr = S_OK;

        // read first record correctly
        switch (pRawRec1->rawRecordsData[0])
        {
            case PROF_REC_IBS_OP_EXT:
            {
                PRD_IBS_OP_DATA_BASIC_RECORD* pOp = (PRD_IBS_OP_DATA_BASIC_RECORD*)
                                                    pRawRec1->rawRecordsData;

                if (m_endTick < pOp->m_TickStamp)
                {
                    m_endTick = pOp->m_TickStamp;
                }

                bytes_read = rawRead(pRawRec2);

                if (CAPRDRECORDSIZE != bytes_read)
                {
                    hr = E_FAIL;
                }

                *pRecNum = 2;
            }
            break;

            case PROF_REC_IBS_FETCH_EXT:
            {
                PRD_IBS_FETCH_DATA_BASIC_RECORD* pF = (PRD_IBS_FETCH_DATA_BASIC_RECORD*)
                                                      pRawRec1->rawRecordsData;

                if (m_endTick < pF->m_TickStamp)
                {
                    m_endTick = pF->m_TickStamp;
                }

                bytes_read = rawRead(pRawRec2);

                if (CAPRDRECORDSIZE != bytes_read)
                {
                    hr = E_FAIL;
                }

                *pRecNum = 2;
            }
            break;

            case PROF_REC_EVTCFG:
            case PROF_REC_TIMERCFG:
            case PROF_REC_IBSCFG:
                // There can not be any config records while processing profile sample records.
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: Found Config record(%d) while processing the samples.",
                                           pRawRec1->rawRecordsData[0]);
                break;

            case PROF_REC_CPUID:
                // This should only be written in PRD header;
                // If we see this while processing samples, emit a warning
                OS_OUTPUT_DEBUG_LOG(L"Warning: Found CPUID record while processing profile samples.", OS_DEBUG_LOG_DEBUG);
                break;

            case PROF_REC_WEIGHT:
                // This would update the thread specific object
                UpdateResWeight(pRawRec1);
                break;

            case PROF_REC_PIDCFG:
                // Do nothing here...
                OS_OUTPUT_DEBUG_LOG(L"Warning: Found PIDCFG record while processing profile samples.", OS_DEBUG_LOG_DEBUG);
                break;

            case PROF_REC_USER :
            case PROF_REC_CONFIG:
            case PROF_REC_DEBUG:
            case PROF_REC_CSS:
            case PROF_REC_TIMER :
            case PROF_REC_EVENT :
            case PROF_REC_IBS_FETCH_BASIC:
            case PROF_REC_IBS_OP_BASIC:
            default:
                break;
        }
    }

    return hr;
}

long PrdReaderThread::GetBytesRead() const
{
    if (m_baseAddress)
    {
        return (m_recordsRead * sizeof(RawPRDRecord));
    }
    else
    {
        return 0;
    }
}




// convert the raw sample (timer/event) into RecordDataStruct
// returns E_INVALIDARG if the record type didn't have config info
HRESULT PrdReaderThread::ConvertSampleData(const RawPRDRecord& rawRecord, RecordDataStruct* pRecData)
{
    HRESULT hr_ret_val = S_OK;
    unsigned int profType = m_prdReader.GetProfileType();
    unsigned int profVersion = m_prdReader.GetProfileVersion();

    switch (rawRecord.rawRecordsData[0])
    {
        case (PROF_REC_EVENT):

            if (0 == (profType & PROF_EBP))
            {
                return E_INVALIDARG;
            }

            if (profVersion >= V3_HDR_VERSION)
            {
                PRD_EVENT_CTR_DATA_RECORD* pEvtRec  = (PRD_EVENT_CTR_DATA_RECORD*) rawRecord.rawRecordsData;

                bool isL2IEvent = false;

                if ((pEvtRec->m_EventSelectHigh & (FAKE_L2I_EVENT_ID_PREFIX << 4)) ==
                    (FAKE_L2I_EVENT_ID_PREFIX << 4))
                {
                    isL2IEvent = true;
                }

                if (isL2IEvent)
                {
                    // For L2I events - Do not mask fake value in m_EventSelectHigh bits[7:4]
                    pRecData->m_EventType = (pEvtRec->m_EventSelectHigh << 8) + (pEvtRec->m_EventCtl & 0xFF);
                }
                else
                {
                    pRecData->m_EventType = ((pEvtRec->m_EventSelectHigh & 0xF) << 8) + (pEvtRec->m_EventCtl & 0xFF);
                }

                pRecData->m_EventUnitMask = (pEvtRec->m_EventCtl >> 8) & 0xFF;
                pRecData->m_eventBitMask = (pEvtRec->m_EventCtl >> 16) & 0xf;
                pRecData->m_PID = pEvtRec->m_ProcessHandle;
                pRecData->m_ProcessorID = pEvtRec->m_Core;

                // Address may be present in canonical or non-canonical form
                pRecData->m_RIP = pEvtRec->m_InstructionPointer & ERBT_713_NON_CANONICAL_MASK;

                pRecData->m_ThreadHandle = pEvtRec->m_ThreadHandle;
                pRecData->m_DeltaTick = pEvtRec->m_TickStamp;

                gtUInt32 weight = 0;

                if (isL2IEvent)
                {
                    weight = GetWeight(pEvtRec->m_Core, L2I_PMC_WEIGHT_BASE + pEvtRec->m_EventCounter);
                }
                else
                {
                    weight = GetWeight(pEvtRec->m_Core, PMC_WEIGHT_BASE + pEvtRec->m_EventCounter);
                }

                pRecData->m_weight = static_cast<gtUInt16>(weight);
            }
            else    // ca 2.9 and earlier
            {
                sTrcPerfCountRecord* pEvtRec = (sTrcPerfCountRecord*) rawRecord.rawRecordsData;
                pRecData->m_EventType = pEvtRec->m_EventSelect;
                pRecData->m_EventUnitMask = pEvtRec->m_UnitMask;
                pRecData->m_PID = pEvtRec->m_ProcessHandle;
                pRecData->m_ProcessorID = pEvtRec->m_ProcessorNumber;
                pRecData->m_RIP = pEvtRec->m_InstructionPointer;
                pRecData->m_ThreadHandle = pEvtRec->m_ThreadHandle;
                pRecData->m_DeltaTick = ConvertToDeltaTick(pEvtRec->m_TimeStampCounter);
                pRecData->m_weight = 1;
            }

            break;

        case (PROF_REC_TIMER):
        {
            if (0 == (profType & PROF_TBP))
            {
                return E_INVALIDARG;
            }

            if (profVersion >= V3_HDR_VERSION)
            {
                // for prd in CA 3.0 and later;
                PRD_APIC_DATA_RECORD* pRec  = (PRD_APIC_DATA_RECORD*) rawRecord.rawRecordsData;
                pRecData->m_EventType = GetTimerEvent();
                pRecData->m_EventUnitMask = 0;
                pRecData->m_eventBitMask = 0;
                pRecData->m_PID = pRec->m_ProcessHandle;
                pRecData->m_ProcessorID = pRec->m_Core;

                // Linear address may be present in canonical or non-canonical form.
                // In canonical form, bits 48 to 63 must be the copies of bit 47 - which can either be 1 or 0.
                // For the canonical form address with 0xffff... kernel module discovery will fail.
                pRecData->m_RIP = pRec->m_InstructionPointer & ERBT_713_NON_CANONICAL_MASK;

                pRecData->m_ThreadHandle = pRec->m_ThreadHandle;
                pRecData->m_DeltaTick = pRec->m_TickStamp;
                auto weight = GetWeight(pRec->m_Core, APIC_WEIGHT_BASE);
                pRecData->m_weight = static_cast<gtUInt16>(weight);
            }
            else
            {
                // for old prd file in ca 2.9 and ealier;
                sTrcTimerRecord* pTimerRec      = (sTrcTimerRecord*) rawRecord.rawRecordsData;
                pRecData->m_EventType           = GetTimerEvent();
                pRecData->m_EventUnitMask       = 0;
                pRecData->m_PID                 = pTimerRec->m_ProcessHandle;
                pRecData->m_ProcessorID         = pTimerRec->m_ProcessorNumber;
                pRecData->m_RIP                 = pTimerRec->m_InstructionPointer;
                pRecData->m_ThreadHandle        = pTimerRec->m_ThreadHandle;
                pRecData->m_DeltaTick           = ConvertToDeltaTick(pTimerRec->m_TimeStampCounter);
                pRecData->m_weight = 1;
            }
        }
        break;

        default:
            hr_ret_val = E_FAIL;
            break;
    }

    return hr_ret_val;
}

HRESULT PrdReaderThread::ConvertIBSFetchData(RawPRDRecord* pRawRecord1, RawPRDRecord* pRawRecord2, IBSFetchRecordData* pIBSFetch)
{
    HRESULT hr = E_INVALIDARG;
    unsigned int profType = m_prdReader.GetProfileType();
    unsigned int profVersion = m_prdReader.GetProfileVersion();

    if (0 == (profType & PROF_IBS))
    {
        return hr;
    }

    if (!pRawRecord1 || !pIBSFetch)
    {
        return hr;
    }

    // since PROF_REC_IBS_FETCH_BASIC or PROF_REC_IBS_FETCH_EXT records are identical for both
    // CA 2.9 and CA 3.0 (except name of member variables), I will convert it once here.
    hr = ConvertOldIBSFetchData(pRawRecord1, pRawRecord2, pIBSFetch);

    if (S_OK == hr)
    {
        if (profVersion < V3_HDR_VERSION)
        {
            sTrcIbsFetchRecord* pFetch = (sTrcIbsFetchRecord*) pRawRecord1;
            // for CA 2.9 old records, convert timestamp to delta tick;
            pIBSFetch->m_DeltaTick = ConvertToDeltaTick(pFetch->m_TimeStampCounter);
            pIBSFetch->m_weight = 1;
        }
        else
        {
            PRD_IBS_FETCH_DATA_BASIC_RECORD* pFetch = (PRD_IBS_FETCH_DATA_BASIC_RECORD*) pRawRecord1;
            pIBSFetch->m_DeltaTick = pFetch->m_TickStamp;
            pIBSFetch->m_weight = GetWeight(pFetch->m_Core, IBS_FETCH_WEIGHT_BASE);

            // Values of new variables might not be available on all CPUs, reset them
            pIBSFetch->m_L2CacheMiss = false;
            pIBSFetch->m_ITLBRefillLatency = 0;

            if (FAMILY_OR == m_prdReader.GetCpuFamily() && 0x6 == (m_prdReader.GetCpuModel() >> 4))
            {
                pIBSFetch->m_L2CacheMiss = (pFetch->m_IbsFetchCtlHigh & FETCH_MASK_L2_CACHE_MISS) != 0;
            }

            if (PROF_REC_IBS_FETCH_EXT == pFetch->m_RecordType)
            {
                PRD_IBS_FETCH_DATA_EXT_RECORD* pFetchExt = (PRD_IBS_FETCH_DATA_EXT_RECORD*)pRawRecord2;

                // MSRC001_103C IBS Fetch Control Extended (IbsFetchCtlExtd)
                if (FAMILY_OR == m_prdReader.GetCpuFamily() && 0x6 == (m_prdReader.GetCpuModel() >> 4))
                {
                    pIBSFetch->m_ITLBRefillLatency = pFetchExt->m_IbsFetchCtlExtd;
                }
            }
        }
    }

    return hr;
}


HRESULT PrdReaderThread::ConvertIBSOpData(RawPRDRecord* pRawRecord1, RawPRDRecord* pRawRecord2, IBSOpRecordData* pIBSOp)
{
    HRESULT hr = E_FAIL;
    unsigned int profType = m_prdReader.GetProfileType();
    unsigned int profVersion = m_prdReader.GetProfileVersion();

    // All pointer arguments should be defined
    if (!pRawRecord1 || !pRawRecord2 || !pIBSOp)
    {
        return (hr) ;
    }

    if (0 == (profType & PROF_IBS))
    {
        return E_INVALIDARG;
    }

    // since PROF_REC_IBS_FETCH_BASIC or PROF_REC_IBS_FETCH_EXT records are identical for both
    // CA 2.9 and CA 3.0 (except name of member variables), I will convert it once here.
    if (profVersion < V3_HDR_VERSION)
    {
        hr = ConvertOldIBSOpData(pRawRecord1, pRawRecord2, pIBSOp);
        sTrcIbsOpRecord* pOpBasic = (sTrcIbsOpRecord*) pRawRecord1;
        // for CA 2.9 old records, convert timestamp to tick;
        pIBSOp->m_DeltaTick = ConvertToDeltaTick(pOpBasic->m_TimeStampCounter);
        pIBSOp->m_weight = 1;
    }
    else
    {
        hr = ConvertRawIBSOpData(pRawRecord1, pRawRecord2, pIBSOp,
                                 m_prdReader.GetCpuFamily(),
                                 m_prdReader.GetCpuModel());
        PRD_IBS_OP_DATA_BASIC_RECORD* pOpBasic = (PRD_IBS_OP_DATA_BASIC_RECORD*) pRawRecord1;
        pIBSOp->m_DeltaTick = pOpBasic->m_TickStamp;
        pIBSOp->m_weight = GetWeight(pOpBasic->m_Core, IBS_OP_WEIGHT_BASE);
    }

    return hr;
}
