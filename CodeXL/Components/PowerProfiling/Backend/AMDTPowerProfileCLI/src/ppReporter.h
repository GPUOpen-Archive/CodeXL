//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppReporter.h
///
//==================================================================================

#ifndef _PP_REPORTER_H_
#define _PP_REPORTER_H_
#pragma once

// Project:
#include <PowerProfileCLI.h>
#include <ppParseArgs.h>
#include <ppCliUtils.h>

#define PP_REPORT_HDR                         "CODEXL POWER PROFILE REPORT"

#define PP_REPORT_EXECUTION_SECTION_HDR        "LAUNCHED APPLICATION DETAILS"
#define PP_REPORT_EXECUTION_TARGET_PATH        "Target Path:"
#define PP_REPORT_EXECUTION_TARGET_ARGS        "Command Line Arguments:"
#define PP_REPORT_EXECUTION_WORK_DIR           "Working Directory:"
#define PP_REPORT_EXECUTION_TARGET_ENV_VARS    "Environment Variables:"
#define PP_REPORT_PROFILE_AFFINITY_MASK        "CPU Core Affinity Mask:"
#define PP_REPORT_PROFILE_CPU_DETAILS          "CPU Details:"
#define PP_REPORT_PROFILE_TAREGET_OS           "Operating System:"

#define PP_REPORT_PROFILE_DETAILS_SECTION_HDR    "PROFILE DETAILS"
#define PP_REPORT_PROFILE_SAMPLING_INTERVAL      "Sampling Interval:"
#define PP_REPORT_PROFILE_CPU_MASK               "CPU Core Mask:"
#define PP_REPORT_PROFILE_START_TIME             "Profile Start Time:"
#define PP_REPORT_PROFILE_DURATION               "Profile Duration:"

#define PP_REPORT_PROFILE_COUNTERS               "PROFILED COUNTERS"

#define PP_REPORT_COUNTER_ID                     "COUNTER ID"
#define PP_REPORT_COUNTER_NAME                   "NAME"
#define PP_REPORT_COUNTER_CATEGORY               "CATEGORY"
#define PP_REPORT_COUNTER_MAX_VALUE              "MAX VALUE"
#define PP_REPORT_COUNTER_MIN_VALUE              "MIN VALUE"
#define PP_REPORT_COUNTER_DESCRIPTION            "DESCRIPTION"
#define PP_REPORT_COUNTER_UNIT                   "UNIT"

#define PP_REPORT_PROFILE_RECORDS_SECTION_HDR    "PROFILE RECORDS"

#define PP_REPORT_PROFILE_RECORD_ID              "RecordId"
#define PP_REPORT_PROFILE_RECORD_TS              "Timestamp"

#define PP_REPORT_UNKNOWN_CATEGORY       "Unknown Category"
#define PP_REPORT_UNKNOWN_UNIT           "Unknown Unit"


// The Windows ticks are in 100 nanoseconds (10^-7).
#define PP_WINDOWS_TICK_PER_SEC 10000000

// The windows epoch starts 1601-01-01 00:00:00.
// It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01 00:00:00).
#define PP_SEC_TO_UNIX_EPOCH 11644473600LL


class ppReporter
{
public:
    ppReporter(ppParseArgs& args, osFilePath& filePath) : m_args(args),
        m_error(AMDT_STATUS_OK),
        m_pDataStr(NULL),
        m_reportFilePath(filePath),
        m_isCounterDataHdrPrinted(false)
    {
        m_pDataStr = new char[m_DataStrSize];
    }

    virtual ~ppReporter() { };

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    ppReporter(const ppReporter&) = delete;
    ppReporter& operator=(const ppReporter&) = delete;
#endif

    bool Open()
    {
        bool retVal = false;

        if (!m_reportFilePath.isEmpty())
        {
            retVal = m_reportFile.open(m_reportFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        }

        m_error = retVal ? AMDT_STATUS_OK : AMDT_ERROR_NOFILE;
        return retVal;
    }

    bool Close()
    {
        m_reportFile.close();
        return true;
    }

    bool IsOpened() const { return m_reportFile.isOpened(); }

    void SetProfileStartTime(string startTime) { m_profStartTime = startTime;  }

    void SetCounterDesc(AMDTPwrCounterIdDescVec& supportedCounterIdDescVec,
                        AMDTPwrCounterIdNameVec& supportedCounterIdNameVec,
                        gtVector<AMDTUInt32>& profiledCounterIdVec)
    {
        m_supportedCounterIdDescVec = supportedCounterIdDescVec;
        m_supportedCounterIdNameVec = supportedCounterIdNameVec;
        m_profiledCounterIdVec = profiledCounterIdVec;
    }

    void ReportHeader();
    bool ReportProfiledCounterDesc();
    bool ReportProfiledCounterDataHdr(AMDTPwrSample*& sample);
    bool ReportProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample);

    AMDTUInt64 ConvertTimeStampToStr(AMDTPwrSystemTime& sampleTime, AMDTUInt64 elapsedMs, char*& pTimeStr);

    virtual void ConstructHeader() = 0;
    virtual void ConstructProfiledCounterDescHdr() = 0;
    virtual void ConstructProfiledCounterDesc(gtString& counterName, const AMDTPwrCounterDesc*& counterDesc) = 0;
    virtual void ConstructProfiledCounterDataHdr(AMDTPwrSample*& sample) = 0;
    virtual void ConstructProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample) = 0;

    virtual void ReportHistogramCounters(AMDTUInt32 num, AMDTPwrHistogram* pHist) = 0;
    virtual void ReportCumulativeCounters(AMDTUInt32 num, AMDTFloat32* pHist, AMDTUInt32* pCounterId) = 0;
    virtual void WriteProcessData(AMDTUInt32 recCnt, AMDTPwrProcessInfo*& pInfo) = 0;
    virtual void WriteModuleData(AMDTUInt32 recCnt, AMDTPwrModuleData*& pInfo, AMDTFloat32 totalPower) = 0;


protected:
    ppParseArgs&  m_args;
    AMDTResult    m_error;

    string        m_profStartTime;
    const int     m_DataStrSize = 1024;
    char*         m_pDataStr;
    string        m_dataStr;

    osFilePath    m_reportFilePath;  // output report
    osFile        m_reportFile;

    bool          m_isCounterDataHdrPrinted;

    // Counter Desc Vector;
    AMDTPwrCounterIdDescVec   m_supportedCounterIdDescVec;
    AMDTPwrCounterIdNameVec   m_supportedCounterIdNameVec;    // ID-Name vector
    gtVector<AMDTUInt32>      m_profiledCounterIdVec; // vector of all profiled counter-ids

};

class ppReporterText : public ppReporter
{
public:

    ppReporterText(ppParseArgs& args, osFilePath& filePath) : ppReporter(args, filePath)
    {
        m_reportFilePath.setFileExtension(L"txt");
    };

    ~ppReporterText() {};

    void ConstructHeader();
    void ConstructProfiledCounterDescHdr();
    void ConstructProfiledCounterDesc(gtString& counterName, const AMDTPwrCounterDesc*& counterDesc);
    void ConstructProfiledCounterDataHdr(AMDTPwrSample*& sample);
    void ConstructProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample);

    void ReportHistogramCounters(AMDTUInt32 num, AMDTPwrHistogram* pHist);
    void ReportCumulativeCounters(AMDTUInt32 num, AMDTFloat32* pHist, AMDTUInt32* pCounterId);
    void WriteProcessData(AMDTUInt32 recCnt, AMDTPwrProcessInfo*& pInfo);
    void WriteModuleData(AMDTUInt32 recCnt, AMDTPwrModuleData*& pInfo, AMDTFloat32 totalPower);
};

class ppReporterCsv : public ppReporter
{
public:

    ppReporterCsv(ppParseArgs& args, osFilePath& filePath) : ppReporter(args, filePath)
    {
        m_reportFilePath.setFileExtension(L"csv");
    };

    ~ppReporterCsv() {};

    void ConstructHeader();
    void ConstructProfiledCounterDescHdr();
    void ConstructProfiledCounterDesc(gtString& counterName, const AMDTPwrCounterDesc*& counterDesc);
    void ConstructProfiledCounterDataHdr(AMDTPwrSample*& sample);
    void ConstructProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample);

    void ReportHistogramCounters(AMDTUInt32 num, AMDTPwrHistogram* pHist);
    void ReportCumulativeCounters(AMDTUInt32 num, AMDTFloat32* pHist, AMDTUInt32* pCounterId);
    void WriteProcessData(AMDTUInt32 recCnt, AMDTPwrProcessInfo*& pInfo);
    void WriteModuleData(AMDTUInt32 recCnt, AMDTPwrModuleData*& pInfo, AMDTFloat32 totalPower);
};

#endif // #ifndef _PP_REPORTER_H_
