//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Report.h
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#ifndef _CPUPROFILE_REPORT_H_
#define _CPUPROFILE_REPORT_H_
#pragma once

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Backend:
#include <AMDTCpuProfilingDataAccess/inc/AMDTCpuProfilingDataAccess.h>

// Project:
#include <ParseArgs.h>
#include <AMDTThreadProfileApi.h>
#include <Reporter.h>
#include <Utils.h>

// Macros
#define CODEXL_DEFAULT_OUTPUTFILE_NAME    L"CodeXL-CpuProfile"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define CPUPROFILE_RAWFILE_EXTENSION    L"prd"
#else
    #define CPUPROFILE_RAWFILE_EXTENSION    L"caperf"
#endif

#define CPUPROFILE_EBP_REPORT_EXTENSION     L"ebp"
#define THREAD_PROFILE_DATAFILE_EXTENSION   L"cxltp"
#define CPUPROFILE_CSV_REPORT_EXTENSION     L"csv"
#define CPUPROFILE_TEXT_REPORT_EXTENSION    L"txt"
#define CPUPROFILE_XML_REPORT_EXTENSION     L"xml"

#define CPUPROFILE_DBFILE_EXTENSION         L"cxlcpdb"

#define CODEXL_REPORT_HDR            L"CODEXL CPU-PROFILE REPORT\n"

#define EXECUTION_SECTION_HDR        L"EXECUTION"
#define EXECUTION_TARGET_PATH        L"Target Path:"
#define EXECUTION_WORK_DIR           L"Working Directory:"
#define EXECUTION_TARGET_ARGS        L"Command Line Arguments:"
#define EXECUTION_TARGET_ENV_VARS    L"Environment Variables:"
#define PROFILE_CPU_DETAILS          L"CPU Details:"
#define PROFILE_TAREGET_OS           L"Operating System:"

#define PROFILE_DETAILS_SECTION_HDR    L"PROFILE DETAILS"
#define PROFILE_SESSION_TYPE           L"Profile Session Type:"
#define PROFILE_SCOPE                  L"Profile Scope:"
#define PROFILE_CPU_AFFINITY           L"CPU Affinity Mask:"
#define PROFILE_START_TIME             L"Profile Start Time:"
#define PROFILE_END_TIME               L"Profile End Time:"
#define PROFILE_DURATION               L"Profile Duration:"
#define PROFILE_DATA_FOLDER            L"Data Folder:"
#define PROFILE_CSS                    L"Call Stack Sampling:"
#define PROFILE_CSS_DEPTH              L"Call Stack Unwind Depth:"
#define PROFILE_CSS_FREQUENCY          L"Call Stack Unwind Frequency:"
#define PROFILE_CSS_SCOPE              L"Call Stack Scope:"
#define PROFILE_CSS_FPO                L"Call Stack Overcome frame-pointer omission:"

#define PROFILE_SAMPLING_EVENTS        L"Monitored Events:,Name,Interval,Unitmask,User,OS"

// OVERVIEW sections
#define OVERVIEW_FUNCTIONS_SECTION_HDR          L"5 HOTTEST FUNCTIONS"
#define OVERVIEW_PROCESSES_SECTION_HDR          L"5 HOTTEST PROCESSES"
#define OVERVIEW_MODULES_SECTION_HDR            L"5 HOTTEST MODULES"

// Overview section headers
#define OVERVIEW_SECTION_FUNCTIONS       L"FUNCTION"
#define OVERVIEW_SECTION_MODULE          L"MODULE"
#define OVERVIEW_SECTION_PROCESS         L"PROCESS"
#define OVERVIEW_SECTION_PID             L"PID"
#define OVERVIEW_SECTION_SAMPLES         L"SAMPLES"
#define OVERVIEW_SECTION_PERC            L"% OF HOTSPOT SAMPLES"

// Process sections
#define PROCESS_PROFILE_HDR                   L"PROFILE REPORT FOR PROCESS - "
#define MODULE_SUMMARY_SECTION_HDR            L"MODULE SUMMARY"
#define FUNCTION_SUMMARY_SECTION_HDR          L"FUNCTION SUMMARY"

// Call Graph Section
#define CALLGRAPH_HDR                         L"CALLGRAPH FOR PROCESS - "
#define CALLGRAPH_FUNCTION_SUMMARY_HDR        L"FUNCTION TABLE"
#define CALLGRAPH_SECTION_HDR                 L"CALLGRAPH"
#define CALLGRAPH_FUNCTION                    L"FUNCTION"
#define CALLGRAPH_SELF_SAMPLES                L"SELF SAMPLES"
#define CALLGRAPH_PERC_SELF_SAMPLES           L"% SELF SAMPLES"
#define CALLGRAPH_DEEP_SAMPLES                L"DEEP SAMPLES"
#define CALLGRAPH_PERC_DEEP_SAMPLES           L"% DEEP SAMPLES"
#define CALLGRAPH_PATH_COUNT                  L"PATH COUNT"
#define CALLGRAPH_SOURCE_FILE                 L"SOURCE FILE"
#define CALLGRAPH_MODULE                      L"MODULE"
#define CALLGRAPH_PARENTS                     L"PARENTS"
#define CALLGRAPH_CHILDREN                    L"CHILDREN"
#define CALLGRAPH_HOT_CALLPATHS               L"HOT CALLPATHS"
#define CALLGRAPH_INDEX                       L"INDEX"

#define NO_SAMPLES_TO_DISPLAY          L"==== NO SAMPLES TO DISPLAY ===="

// various report sections
#define CPU_PROFILE_REPORT_ALL          L"all"
#define CPU_PROFILE_REPORT_OVERVIEW     L"overview"
#define CPU_PROFILE_REPORT_PROCESS      L"process"
#define CPU_PROFILE_REPORT_MODULE       L"module"
#define CPU_PROFILE_REPORT_CALLGRAPH    L"callgraph"
#define CPU_PROFILE_REPORT_IMIX         L"imix"

typedef enum CpuProfileReportTypes
{
    CPU_PROFILE_REPORT_TYPE_UNKNOW  = 0,
    CPU_PROFILE_REPORT_TYPE_CSV     = 1,
    CPU_PROFILE_REPORT_TYPE_XML     = 2,
    CPU_PROFILE_REPORT_TYPE_TEXT    = 3,
} CpuProfileReportType;

class CpuProfileReport
{
public:
    CpuProfileReport(ParseArgs& args) : m_args(args) {}

    ~CpuProfileReport()
    {
        if ((NULL != m_pReporter) && m_pReporter->IsOpened())
        {
            m_pReporter->Close();
            delete m_pReporter;
            m_pReporter = NULL;
        }
    };

    HRESULT Initialize();
    HRESULT Translate();
    HRESULT Migrate();
    HRESULT Report();

    // Thread profiler
    HRESULT ReportTP();

    // thread profile data file
    bool IsTpInputFile() { return m_isTpInputFile; }

private:
    HRESULT             m_error = S_OK;
    ParseArgs&          m_args;

    int                 m_reportType = CPU_PROFILE_REPORT_TYPE_CSV;

    osFilePath          m_outputFilePath;
    osFilePath          m_inputFilePath;

    Reporter*            m_pReporter = nullptr;
    cxlProfileDataReader m_profileDbReader;

    gtString            m_reportViewName; // TODO: Yet to be used
    gtString            m_sortEventName;
    gtUInt32            m_sortEventId;
    gtUInt32            m_sortCounterId;
    gtVector<gtString>  m_profileEventsNameVec;   // this is used for column headers

    gtList<gtUInt32>    m_coresList; // List of cores for which samples need to be aggregated

    // report sections
    bool                m_isRawInputFile = false;
    bool                m_isEbpInputFile = false;
    bool                m_isTpInputFile = false;
    bool                m_isDbInputFile = false;
    bool                m_isReportOverview = false;
    bool                m_isReportAggregateByProcess = false;
    bool                m_isReportAggregateByModule = false;
    bool                m_isReportCallGraph = false;
    bool                m_isReportImix = false;

    void SetupEnvironment();
    void ValidateOptions();

    osFilePath& GetInputFilePath() { return m_inputFilePath; }
    osFilePath& GetRawFilePath() { return m_inputFilePath; }
    osFilePath& GetDBFilePath();
    osFilePath& GetOutputFilePath();

    bool InitializeReportFile();
    bool InitCoresList();

    bool IsReportOverview() const { return m_isReportOverview; }
    bool IsReportAggregateByProcess() const { return m_isReportAggregateByProcess; }
    bool IsReportAggregateByModule() const { return m_isReportAggregateByModule; }
    bool IsReportCallGraph() const { return m_isReportCallGraph; }
    bool IsReportImix() const { return m_isReportImix; }

    void ReportSampleCount(bool sepByCore);
    void ReportExecution(AMDTProfileSessionInfo& sessionInfo);
    void ReportProfileDetails(AMDTProfileSessionInfo& sessionInfo);
    void ReportMonitoredEventDetails();
    void ReportOverviewData(AMDTProfileSessionInfo& sessionInfo);
    void ReportProcessData();
    bool ReportCSSData(AMDTProcessId pid);
    void ReportImixData();

    bool isOK() const { return (S_OK == m_error) ? true : false; }
    bool IsRawInputFile();
};

#endif // #ifndef _CPUPROFILE_REPORT_H_