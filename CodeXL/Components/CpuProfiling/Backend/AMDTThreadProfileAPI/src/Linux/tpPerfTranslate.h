//=====================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief Data Translate layer for Thread Profiler
//
//=====================================================================

#ifndef _TPETLTRANSLATE_H_
#define _TPETLTRANSLATE_H_

// Project Headers
#include <tpInternalDataTypes.h>
#include <tpTranslateDataTypes.h>
#include <tpTranslate.h>
#include <AMDTThreadProfileDataTypes.h>

// #include <AMDTCpuProfilingRawData/inc/Linux/CaPerfHeader.h>
#include <AMDTCpuProfilingRawData/inc/Linux/CaPerfDataReader.h>
#include <AMDTCpuProfilingRawData/inc/Linux/PerfData.h>


// FIXME
// The below structures should match with structs mentioned linux/perf_event.h
//

#define PERF_COMMAND_LEN 16

#define PERF_EV_BASE 0xE000

struct perfRecordMmap
{
    uint32_t pid;
    uint32_t tid;
    uint64_t addr;
    uint64_t len;
    uint64_t pgoff;
    char     filename[CA_PERF_RECORD_PATH_MAX];
};

struct perfRecordComm
{
    uint32_t  pid;
    uint32_t  tid;
    char      commandStr[PERF_COMMAND_LEN];
};

struct perfIpCallchain
{
    uint64_t nbrStackFrames;
    uint64_t stackFrames[0];
};

struct perfReadFormat
{
    uint64_t value;
    uint64_t time_enabled;  // PERF_FORMAT_TOTAL_TIME_ENABLED
    uint64_t time_running;  // PERF_FORMAT_TOTAL_TIME_RUNNING
    uint64_t id;            //PERF_FORMAT_ID
};

struct perfRecordSample
{
    uint64_t          ip;
    uint32_t          pid;
    uint32_t          tid;
    uint64_t          time;
    uint64_t          addr;
    uint64_t          id;
    uint64_t          streamId;
    uint32_t          cpu;
    uint32_t          res;
    uint64_t          period;
    perfReadFormat    values;
    perfIpCallchain*  pCallchain;

    // RAW PERF_SAMPLE_RAW ??
};

struct perfRecordFork
{
    uint32_t pid;
    uint32_t ppid;
    uint32_t tid;
    uint32_t ptid;
    uint64_t time;
};


struct perfRecordExit
{
    uint32_t pid;
    uint32_t ppid;
    uint32_t tid;
    uint32_t ptid;
    uint64_t time;
};

struct perfRecordRead
{
    uint32_t               pid;
    uint32_t               tid;
    perfReadFormat  values;
};



//
// Data Structures
//

#define CXL_TP_MAX_CORES    64
#define CXL_TP_ANY_TID      (AMDTUInt32)-1

// class PerfDataReader;

class tpPerfTranslate : public tpTranslate
{
public:
    tpPerfTranslate()
    {
        m_pCaperfReader = nullptr;

        int i;

        for (i = 0; i < CXL_TP_MAX_CORES; i++)
        {
            m_prevThreadId[i] = CXL_TP_ANY_TID;
        }

        memset(m_prevTimestamp, 0, sizeof(m_prevTimestamp));

        m_addedPid0 = false;
        m_isPassOneComplete = false;
    };

    ~tpPerfTranslate() { };

    AMDTResult OpenThreadProfileData(bool isPassOne);
    AMDTResult ProcessThreadProfileData();
    AMDTResult CloseThreadProfileData();

private:
    AMDTResult AddGenericData();

    AMDTResult ProcessPerfRecordsPass1();
    AMDTResult ProcessPerfRecordsPass2();

    AMDTResult ParsePerfRecordSample(void* pData, perfRecordSample* pSample);
    AMDTResult ProcessMmapRecord(struct perf_event_header* pHdr, const void* pData, AMDTUInt32 offset, AMDTUInt32 index);
    AMDTResult ProcessCommRecord(struct perf_event_header* pHdr, const void* pData, AMDTUInt32 offset, AMDTUInt32 index);
    AMDTResult ProcessForkRecord(struct perf_event_header* pHdr, const void* pData, AMDTUInt32 offset, AMDTUInt32 index);
    AMDTResult ProcessExitRecord(struct perf_event_header* pHdr, const void* pData, AMDTUInt32 offset, AMDTUInt32 index);
    AMDTResult ProcessSampleRecord(struct perf_event_header* pHdr, const void* pData, AMDTUInt32 offset, AMDTUInt32 index);

    AMDTResult AddCSRecord(perfRecordSample& sample);

private:
    PerfDataReader* m_pCaperfReader;

    AMDTUInt32  m_prevThreadId[CXL_TP_MAX_CORES];
    AMDTUInt64  m_prevTimestamp[CXL_TP_MAX_CORES];

    bool m_addedPid0;
    bool m_isPassOneComplete;
};

#endif // _TPETLTRANSLATE_H_
