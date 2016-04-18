//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdReader.h
/// \brief Interface for the PrdReader class.
///
//==================================================================================

#ifndef _PRDREADER_H_
#define _PRDREADER_H_

#pragma warning(push)
#pragma warning(disable: 4251 4201)

#include <qstring.h>

#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include <WinIoCtl.h>

#include <Driver/Windows/CpuProf/inc/UserAccess/CpuProfDriver.h>
#pragma warning(push)
#pragma warning(disable:4091) //'typedef ': ignored on left of '' when no variable is declared
#include <Driver/Windows/CpuProf/inc/UserAccess/PrdRecords.h>
#pragma warning(pop)


#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfilingRawDataDLLBuild.h>
#include "PrdOldRecords.h"

#pragma warning(pop)

#define ERBT_713_NON_CANONICAL_MASK 0x0000FFFFFFFFFFFF

#define PAGE_BUFFER_SIZE ( 4 * 1024 )
#define PROF_REC_EOF  0x0fe

#define DATA_RECORD      0
#define USR_RECORD       1
#define IGNORED_RECORD   2
#define END_OF_PAGE      3
#define END_OF_FILE      4

#define EVENT_ID_MASK 0xFFFF
#define EVENT_COUNT_MASK 0xFFFFFFFFFFFF0000
#define EVENT_COUNT_SHIFT 16

#define CAPRDRECORDSIZE     40

#define PROF_INVALIDTYPE    0x00
#define PROF_TBP            0x01
#define PROF_EBP            0x02
#define PROF_IBS            0x04

struct EventKey
{
    unsigned int core;
    PERF_CTL ctl;
    EventKey()
    {
        core = 0;
        ctl.perf_ctl = 0;

    }
    EventKey(unsigned int c, PERF_CTL eventSel)
    {
        core = c;
        ctl.perf_ctl = eventSel.perf_ctl;
    }

    bool operator< (const EventKey& other) const
    {
        return (this->core < other.core) ? true
               : (this->core > other.core) ? false
               : (this->ctl.perf_ctl < other.ctl.perf_ctl);
    }
};

struct EventData
{
    unsigned int numApperance;
    gtUInt64 ctr;
};

typedef gtMap<EventKey, EventData> PrdRDEventMap;

#define APIC_WEIGHT_BASE        0x100
#define PMC_WEIGHT_BASE         0x200
#define NB_PMC_WEIGHT_BASE      0x300
#define L2I_PMC_WEIGHT_BASE     0x400
#define IBS_FETCH_WEIGHT_BASE   0x500
#define IBS_OP_WEIGHT_BASE      0x600

struct WeightKey
{
    unsigned int core;

    // res_index is resource weight base + offset;
    // for APIC, IBS fetch and IBS op, the offset is 0
    // for PMC and NB_PMC, the offset will be the counter index;
    unsigned int res_index;

    WeightKey() : core(0U), res_index(0U) {}
    WeightKey(unsigned int c, unsigned int res) : core(c), res_index(res) {}

    bool operator< (const WeightKey& other) const
    {
        return core < other.core || (core == other.core && res_index < other.res_index);
    }
};

// the data is the current weight;
typedef gtMap<WeightKey, unsigned int> WeightMap;

// the data is process id;
typedef gtList<gtUInt64> PidFilterList;

struct CORE_TOPOLOGY
{
    gtUInt16 processor;
    gtUInt16 numaNode;
};

enum ERROR_CODES
{
    VALID_RECORD_FOUND = 0,
    NO_VALID_RECORD_FOUND,
    NO_MORE_DATA_IN_FILE,
    GET_THE_HINT,
    GO_AWAY,
    I_MEAN_IT,
    IBS_PAIR_NOT_MATCH,
    NO_IBS_EXT_FOUND
};

struct RecordDataStruct
{
    gtUInt64 m_PID;
    gtUInt64 m_ThreadHandle;
    gtUInt64 m_RIP;
    gtUInt64 m_DeltaTick;   // number of milliseconds since profile start;
    // 0xFFFF to indicate timer based. Otherwise, it will be 12bits event select Mask
    gtUInt16   m_EventType;
    gtUByte    m_ProcessorID;
    gtUByte  m_EventUnitMask;
    gtUInt16   m_weight;
    gtUByte m_eventBitMask;
};

struct EventCfgInfo
{
    PERF_CTL ctl;
    gtUInt64 ctr;
    // NumApperance = number of appearance that same event appears in the profile group.
    // for new CA 3.0 and CodeXL, this is always 1
    unsigned int numApperance;
};

struct IBSFetchRecordData
{
    gtUInt64    m_PID;
    gtUInt64    m_ThreadHandle;
    gtUInt64    m_RIP;
    gtUInt64    m_DeltaTick;    // num of ms since profile starts;
    gtUInt64    m_PhysicalAddr;
    unsigned int m_weight;
    gtUInt16    m_FetchLatency;
    gtUInt16    m_TLBPageSize;
    gtUInt16    m_ITLBRefillLatency;
    gtUByte     m_ProcessorID;
    bool        m_PhysicalAddrValid;
    bool        m_L2TLBMiss;
    bool        m_L1TLBMiss;
    bool        m_InstCacheMiss;
    bool        m_InstCacheHit;
    bool        m_FetchCompletion;
    bool        m_L1TLBHit;
    bool        m_ITLB_L1M_L2H;
    bool        m_ITLB_L1M_L2M;
    bool        m_Killed;
    bool        m_L2CacheMiss;
};

struct IBSOpRecordData
{
    gtUInt64    m_PID;
    gtUInt64    m_ThreadHandle;
    gtUInt64    m_DeltaTick;    // num of ms since profile starts;
    gtUInt64    m_RIP;
    gtUInt64    m_IbsDcLinAd;
    gtUInt64    m_IbsDcPhysAd;
    gtUInt64    m_BranchTarget;
    unsigned int m_weight;
    gtUInt16    m_TagToRetireCycles;
    gtUInt16    m_CompToRetireCycles;
    gtUInt16    m_IbsDcMissLat;
    gtUByte     m_ProcessorID;
    gtUByte     m_NbIbsReqSrc;
    bool        m_NbIbsCacheHitSt;
    bool        m_NbIbsReqDstProc;
    bool        m_OpBranchRetired;
    bool        m_OpBranchMispredicted;
    bool        m_OpBranchTaken;
    bool        m_OpMispredictedReturn;
    bool        m_OpBranchResync;
    bool        m_OpReturn;
    bool        m_IbsLdOp;
    bool        m_IbsStOp;
    bool        m_IbsDcLinAddrValid;
    bool        m_IbsDcPhyAddrValid;
    bool        m_IbsDcL1tlbMiss;
    bool        m_IbsDcL2tlbMiss;
    bool        m_IbsDcL1tlbHit2M;
    bool        m_IbsDcL1tlbHit1G;
    bool        m_IbsDcL2tlbHit2M;
    bool        m_IbsDcL2tlbHit1G;
    bool        m_IbsDcMiss;
    bool        m_IbsDcMisAcc;
    bool        m_IbsDcLdBnkCon;
    bool        m_IbsDcStBnkCon;
    bool        m_IbsDcStToLdFwd;
    bool        m_IbsDcStToLdCan;
    bool        m_IbsDcUcMemAcc;
    bool        m_IbsDcWcMemAcc;
    bool        m_IbsDcLockedOp;
    bool        m_IbsDcMabHit;
    bool        m_IbsOpLdResync;
};



struct RawPRDRecord
{
    gtUByte rawRecordsData[PRD_RECORD_SIZE];
};


class CP_RAWDATA_API PrdReader
{
public:
    PrdReader();
    virtual ~PrdReader();

    // Initialize() will open the prd file and read the configuration records.
    HRESULT Initialize(const wchar_t* pFileName, gtUInt64* pLastUserCssRecordOffset = NULL);
    HRESULT Close();

    unsigned int GetProfileVersion();

    // GetProfileType
    unsigned int GetProfileType() const;

    // The resolution unit is 0.1 millisec
    gtUInt64 GetTimerResultion() const { return m_TimerResolution; }

    // Return number of event groups for old prd file (CA 2.9 and before).
    // FOr CA 3.0, this will always return 1;
    unsigned int GetEventGroupCount();

    // Get number of unique event configurations
    unsigned int GetEventCount();

    // get all EBP event configuration,
    HRESULT GetEventInfo(EventCfgInfo* pEventCfgInfo, unsigned size);

    gtUInt64 GetEventMultiplexPeriod();                     // clock ticks

    HRESULT GetIBSConfig(unsigned int* pIBSFetchCount, unsigned int* pIBSOpCount) const;

    // Get the offset of the first PROF_REC_WEIGHT record in the PRD file
    gtUInt32 GetFirstWeightRecOffset() const { return m_firstWeightRecOffset; }
    HRESULT GetBufferWeightRecOffset(void* baseAddress, gtUInt32 length, gtUInt32* pOffset);

    // Get the start tick
    gtUInt64 GetStartTick() const { return m_startTick; }

    // Get the end tick
    gtUInt64 GetEndTick() const { return m_endTick; }

    int ReadMappedRecord(const void* baseAddress, RawPRDRecord* pRawRec1)
    {
        if (pRawRec1)
        {
            memcpy(pRawRec1->rawRecordsData, baseAddress, CAPRDRECORDSIZE);
            return CAPRDRECORDSIZE;
        }

        return 0;
    }

    bool isWeightRecHasBufferCount() const { return m_weightRecHasBufferCount; }

    // The following calls will get filtered records.
    // HRESULT GetNextSampleRecord(RecordDataStruct *pRecordData);

    // The following calls will get raw records.  The thread profile needs this.
    HRESULT GetNextRawRecord(RawPRDRecord* pRawDataRec);
    HRESULT GetNextRawRecords(RawPRDRecord* pRawRec1, RawPRDRecord* pRawRec2, unsigned int* pRecNum);

    // Convert the raw sample (timer/event) into a RecordDataStruct
    HRESULT ConvertSampleData(const RawPRDRecord& rawRecord, RecordDataStruct* pRecData);

    // Convert the raw sample data into an IBS fetch record
    HRESULT ConvertIBSFetchData(RawPRDRecord* pRawRecord1,
                                RawPRDRecord* pRawRecord2, IBSFetchRecordData* pIBSFetch);

    // Convert the raw sample data into an IBS op record
    HRESULT ConvertIBSOpData(RawPRDRecord* pRawRecord1,
                             RawPRDRecord* pRawRecord2, IBSOpRecordData* pIBSOp);

    gtUInt64 ConvertMissedCount(RawPRDRecord rawRecord);

    int GetCpuFamily() const;  //in case the cpu record is before the config
    int GetCpuModel() const;
    int GetCoreCount() const;
    gtUInt32 GetClockSpeed() const { return m_ClockSpeed; }
    gtUInt64 GetHrFreq() const { return m_hrFreq; }

    // reutrn number of seconds;
    unsigned int GetProfileDuration() const;

    //  void GetStartMarks (FILETIME *pStartMark, gtUInt64 * pStartTick,
    //      gtUByte *pStartCpu);
    //  void GetEndTick (gtUInt64 * pEndTick, gtUByte *pEndCpu);

    long GetBytesRead();

    bool GetTopology(unsigned int core, gtUInt16* pProcessor, gtUInt16* pNode) const;

    gtUInt64 ConvertToDeltaTick(gtUInt64 timestamp);

    gtUByte GetL1DcSize() const { return m_L1DcSize; }
    gtUByte GetL1DcAssoc() const { return m_L1DcAssoc; }
    gtUByte GetL1DcLinesPerTag() const { return m_L1DcLinesPerTag; }
    gtUByte GetL1DcLineSize() const { return m_L1DcLineSize; }
    const PidFilterList& GetPidFilterList() const { return m_pidList; }

private:
    HRESULT InitializeOldPrd(const wchar_t* pFileName);
    HRESULT ConvertCpuFamily(gtUInt32 Fn0000_0001_EAX);
    unsigned int GetWeight(unsigned int core, unsigned int res_index);
    void UpdateResWeight(RawPRDRecord* pRawRecord);
    void UpdatePIDFilter(RawPRDRecord* pRawRecord);
    void UpdateIBSConfigRec(RawPRDRecord* pRawRecord);
    void UpdateTimerConfigRec(RawPRDRecord* pRawRecord);
    void UpdateEventConfigRec(RawPRDRecord* pRawRecord);
    void UpdateCPUIDRec(RawPRDRecord* pRawRecord);
    void AdjustOldEventConfig();
    void UpdateTopologyRec(RawPRDRecord* pRawRecord);

protected:
    FILE* m_hPRDFile;

private:
    gtUInt64    m_TimerResolution;      // in microseconds
    gtUInt64 m_IbsFetchCtl;
    gtUInt64 m_IbsOpCtl;
    unsigned int m_EventGroupCount;

    // Memory map related members
    gtUInt32        m_firstWeightRecOffset;
    bool        m_weightRecHasBufferCount;

    PrdRDEventMap m_EventMap;
    WeightMap m_WeightMap;
    CORE_TOPOLOGY* m_aTopology;

    PidFilterList m_pidList;

    gtUInt64 m_EMTicks;             // clock ticks for event multiplexing period
    unsigned int    m_profType;
    unsigned int m_version;

    int m_cpuFamily;
    int m_cpuModel;

    int m_coreCount;
    FILETIME m_startMark;
    FILETIME m_endMark;

    // for old prd, this is timestamp ; system tick for new prd
    gtUInt64 m_startTick;
    gtUInt64 m_endTick;
    gtUInt64 m_hrFreq;
    gtUInt32 m_ClockSpeed;  // mhz;

    gtUByte m_L1DcSize;
    gtUByte m_L1DcAssoc;
    gtUByte m_L1DcLinesPerTag;
    gtUByte m_L1DcLineSize;
};


// PrdReaderThread
//
//  Thread specific PRDReader object
//
class CP_RAWDATA_API PrdReaderThread
{
public:
    PrdReaderThread(PrdReader& prdReader);
    PrdReaderThread(PrdReader& prdReader, gtUInt64 baseAddress, gtUInt32 numRecords, gtUInt64 startTick, gtUInt64 endTick);
    ~PrdReaderThread();

    HRESULT SetPRDBufferValues(gtUInt64 baseAddress, gtUInt32 numRecords, gtUInt64 startTick, gtUInt64 endTick)
    {
        m_baseAddress = baseAddress;
        m_numRecords = numRecords;
        m_recordsRead = 0;

        m_startTick = startTick;
        m_endTick = endTick;

        return S_OK;
    }

    // Handle PROF_REC_WEIGHT records in the PRD file
    unsigned int GetWeight(unsigned int core, unsigned int res_index);
    gtUInt64 ConvertToDeltaTick(gtUInt64 timestamp);

    RawPRDRecord* GetLastRecord();
    PRD_RECORD_TYPE PeekNextRecordType() const;
    bool SkipRawRecords(unsigned int count);
    HRESULT GetNextRawRecord(RawPRDRecord* pRawDataRec);
    HRESULT GetNextRawRecords(RawPRDRecord* pRawRec1, RawPRDRecord* pRawRec2, unsigned int* pRecNum);

    long GetBytesRead() const;
    gtUInt32 GetRecordsRead() const { return m_recordsRead; }

    HRESULT ConvertSampleData(const RawPRDRecord& rawRecord, RecordDataStruct* pRecData);
    HRESULT ConvertIBSFetchData(RawPRDRecord* pRawRecord1, RawPRDRecord* pRawRecord2, IBSFetchRecordData* pIBSFetch);
    HRESULT ConvertIBSOpData(RawPRDRecord* pRawRecord1, RawPRDRecord* pRawRecord2, IBSOpRecordData* pIBSOp);

    // There was a compiler warning that "the assignment operator could not be generated..."
    PrdReaderThread& operator=(const PrdReaderThread&) = delete;

private:
    unsigned int rawRead(RawPRDRecord* pRawRec1);
    void UpdateResWeight(RawPRDRecord* pRawRecord);

    PrdReader&   m_prdReader;

    WeightMap   m_WeightMap;

    gtUInt64        m_baseAddress;
    gtUInt32        m_numRecords;
    gtUInt32        m_recordsRead;

    gtUInt64        m_startTick;
    gtUInt64        m_endTick;
};

#endif // _PRDREADER_H_
