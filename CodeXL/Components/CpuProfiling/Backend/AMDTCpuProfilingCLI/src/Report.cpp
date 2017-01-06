//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Report.cpp
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

// Backend:
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Project:
#include <Report.h>
#include <CommonUtils.h>
#include <algorithm>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <time.h>
#endif

#define TP_ALL_PIDS                0xFFFFFFFFUL

// External function.
extern bool reportError(bool appendDriverError, const wchar_t* pFormatString, ...);

// Local functions
static AMDTResult printThreadSummary(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile, bool reportHdr);
static AMDTResult printThreadSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile);


HRESULT CpuProfileReport::Initialize()
{
    // Setup the Environment
    SetupEnvironment();

    // Validate Profile Type and other options
    ValidateOptions();

    return m_error;
}

bool CpuProfileReport::IsRawInputFile()
{
    return m_isRawInputFile;
}

HRESULT CpuProfileReport::Translate()
{
    HRESULT hr = S_OK;

    if (isOK() && IsRawInputFile())
    {
        // Debug search path
        gtString searchPath = m_args.GetDebugSymbolPath();
        gtString serverList = m_args.GetSymbolServerPath();
        gtString cachePath = m_args.GetSymbolCachePath();

        const wchar_t* pSearchPath = (!searchPath.isEmpty()) ? searchPath.asCharArray() : nullptr;
        const wchar_t* pServerList = (!serverList.isEmpty()) ? serverList.asCharArray() : nullptr;
        const wchar_t* pCachePath = (nullptr != pServerList && !cachePath.isEmpty()) ? cachePath.asCharArray() : nullptr;

        // TODO: Initialize Symbol engine - AuxInitializeSymbolEngine

        gtString inputFilePath = m_args.GetInputFile();
        ReaderHandle* pHandle = nullptr;
        hr = fnOpenProfile(inputFilePath.asCharArray(), &pHandle);

        if (!SUCCEEDED(hr))
        {
            reportError(false, L"The raw profile (%ls) open failed. (error code 0x%lx).\n",
                        inputFilePath.asCharArray(), hr);
            return hr;
        }

        osFilePath outputFilePath = GetDBFilePath();
        //TODO: remove this line once EBP file support is removed.
        outputFilePath.setFileExtension(L"ebp");

        osDirectory outDir;
        outputFilePath.getFileDirectory(outDir);

        if (! outDir.exists())
        {
            if (!outDir.create())
            {
                reportError(false, L"Failed to create output directory " STR_FORMAT L".\n", outputFilePath.asString().asCharArray());
            }
        }

        hr = fnCpuProfileDataTranslate(pHandle,
                                       outputFilePath.asString().asCharArray(),
                                       nullptr,
                                       pSearchPath,
                                       pServerList,
                                       pCachePath);

        if (S_OK != hr)
        {
            gtString msg;
            gtString additional;

            if ((int)E_NODATA == hr)
            {
                additional = L"There were no data records, was the profiling paused the entire duration?";
            }
            else if ((int)E_INVALIDDATA == hr)
            {
                additional = L"The raw data file may be corrupted.";
            }

            msg.appendFormattedString(L"Could not write the profile data file (%ls).\n%ls\n",
                                      outputFilePath.asString().asCharArray(), additional.asCharArray());

            reportError(false, STR_FORMAT, msg.asCharArray());
        }

        if (S_OK != fnCloseProfile(&pHandle))
        {
            reportError(false, L"Failed to close the profile file.\n");
            hr = E_FAIL;
        }
    }

    return hr;
} // Translate

HRESULT CpuProfileReport::Migrate()
{
    HRESULT hr = S_OK;

    if (isOK() && m_isEbpInputFile)
    {
        hr = fnMigrateEBPToDB(m_inputFilePath);
    }

    return hr;
}

HRESULT CpuProfileReport::Report()
{
    HRESULT hr = S_OK;

    // Initialize the report file
    bool retVal = InitializeReportFile();

    if (retVal)
    {
        // Input file - Processed profile data file
        osFilePath dbFilePath = GetDBFilePath();
        dbFilePath.setFileExtension(L"cxlcpdb");

        // Set the symbol directory and symbol server path
        gtString searchPath = m_args.GetDebugSymbolPath();
        gtString serverList = m_args.GetSymbolServerPath();
        gtString cachePath = m_args.GetSymbolCachePath();
        m_profileDbReader.SetDebugInfoPaths(searchPath, serverList, cachePath);

        m_profileDbReader.OpenProfileData(dbFilePath.asString());

        AMDTProfileSessionInfo sessionInfo;
        m_profileDbReader.GetProfileSessionInfo(sessionInfo);

#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS

        // Disable CLU on Linux
        //if (m_profileReader.getProfileInfo()->m_isProfilingCLU)
        {
            //reportError(false, L"CLU reporting is not yet supported!\n");
            //return E_FAIL;
        }

#endif

        if (!m_args.GetSectionsToReport().isEmpty()
            && IsReportCallGraph()
            && sessionInfo.m_cssEnabled)
        {
            reportError(false, L"Callgraph details are not available in the profile data file - " STR_FORMAT L".\n",
                        dbFilePath.asString().asCharArray());
        }

        if (retVal)
        {
            if (m_args.IsReportSampleCount())
            {
                ReportSampleCount(m_args.IsReportByCore());
            }
            else
            {
                ReportExecution(sessionInfo);
                ReportProfileDetails(sessionInfo);
                ReportMonitoredEventDetails();

                ReportOverviewData(sessionInfo);
                ReportProcessData();

#ifdef AMDT_CPCLI_ENABLE_IMIX
                ReportImixData();
#endif
            }
        }
    }

    return hr = (retVal) ? S_OK : E_FAIL;
} // Report

static gtString printTS(AMDTUInt64 timeStamp)
{
    // Time stamp for when the event occurred.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    FILETIME ft;
    LARGE_INTEGER ts;
    SYSTEMTIME st;
    SYSTEMTIME stLocal;
    AMDTUInt64 nanoSeconds;

    ts.QuadPart = timeStamp;
    ft.dwHighDateTime = ts.HighPart;
    ft.dwLowDateTime = ts.LowPart;

    FileTimeToSystemTime(&ft, &st);
    SystemTimeToTzSpecificLocalTime(nullptr, &st, &stLocal);

    nanoSeconds = (ts.QuadPart % 10000000) * 100;

    gtString tsStr;
    tsStr.appendFormattedString(L" %02d:%02d:%02d.%I64u , ", stLocal.wHour, stLocal.wMinute, stLocal.wSecond, nanoSeconds);
    return tsStr;
#else
    gtString tsStr;
    // tsStr.appendFormattedString(L" %016lu , ", timeStamp);

    struct timeval ts;
    struct tm time;

    ts.tv_sec = timeStamp / 1000000000;
    ts.tv_usec = (timeStamp - (ts.tv_sec * 1000000000)) / 1000;

    tzset();
    localtime_r(&(ts.tv_sec), &time);
    tsStr.appendFormattedString(L" %d:%d:%d:%06lu , ", time.tm_hour, time.tm_min, time.tm_sec, ts.tv_usec);
    return tsStr;
#endif // WINDOWS
}

static AMDTResult printAllProcessesSummary(AMDTThreadProfileDataHandle& tpReaderHandle, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTUInt32 nbrProcesses = 0;
    AMDTProcessId* pProcesses = nullptr;
    AMDTUInt32 nbrProcessors = 0;

    hr = AMDTGetNumOfProcessors(tpReaderHandle, &nbrProcessors);
    gtString s(L"Number of Processors : ");
    s.appendFormattedString(L" %u \n", nbrProcessors);
    reportFile.writeString(s);

    hr = AMDTGetProcessIds(tpReaderHandle, &nbrProcesses, 0, nullptr);

    if (nbrProcesses > 0)
    {
        pProcesses = (AMDTProcessId*)malloc(sizeof(AMDTProcessId) * nbrProcesses);

        hr = AMDTGetProcessIds(tpReaderHandle, nullptr, nbrProcesses, pProcesses);
    }

    reportFile.writeString(L"\nPROCESSES :\n");
    reportFile.writeString(L"\nPID , ImageName");

    for (AMDTUInt32 i = 0; i < nbrProcesses; i++)
    {
        gtString data(L"\n");
        data.appendFormattedString(L" %lu , ", pProcesses[i]);

        AMDTProcessData procData;
        hr = AMDTGetProcessData(tpReaderHandle, pProcesses[i], &procData);

        gtString image;
        data.append(image.fromASCIIString(procData.m_pCommand));
        reportFile.writeString(data);
    }

    reportFile.writeString(L"\n");

    if (nullptr != pProcesses)
    {
        free(pProcesses);
    }

    return hr;
}

static AMDTResult printProcessSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTProcessId pid, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTUInt32 nbrThreads = 0;
    AMDTThreadId* pThreads = nullptr;

    hr = AMDTGetThreadIds(tpReaderHandle, pid, &nbrThreads, 0, nullptr);

    if (nbrThreads > 0)
    {
        pThreads = (AMDTThreadId*)malloc(sizeof(AMDTThreadId) * nbrThreads);

        hr = AMDTGetThreadIds(tpReaderHandle, pid, nullptr, nbrThreads, pThreads);
    }

    gtString data;
    data.appendFormattedString(L"PID - %d\n", pid);
    reportFile.writeString(data);

    for (AMDTUInt32 i = 0; i < nbrThreads; i++)
    {
        AMDTThreadId tid = pThreads[i];

        // print the thread samples
        printThreadSampleData(tpReaderHandle, tid, reportFile);

        // print thread meta data
        data.makeEmpty();
        data.appendFormattedString(L"\n THREAD SUMMARY for TID - %d\n", tid);
        reportFile.writeString(data);
        printThreadSummary(tpReaderHandle, tid, reportFile, true);
    }

    if (nullptr != pThreads)
    {
        free(pThreads);
    }

    return hr;
}

static AMDTResult printAllProcessesSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTUInt32 nbrProcesses = 0;
    AMDTProcessId* pProcesses = nullptr;

    hr = AMDTGetProcessIds(tpReaderHandle, &nbrProcesses, 0, nullptr);

    if (nbrProcesses > 0)
    {
        pProcesses = (AMDTProcessId*)malloc(sizeof(AMDTProcessId) * nbrProcesses);

        hr = AMDTGetProcessIds(tpReaderHandle, nullptr, nbrProcesses, pProcesses);
    }


    for (AMDTUInt32 i = 0; i < nbrProcesses; i++)
    {
        printProcessSampleData(tpReaderHandle, pProcesses[i], reportFile);
    }

    if (nullptr != pProcesses)
    {
        free(pProcesses);
    }

    return hr;
}

static AMDTResult printThreadSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;

    gtString data;
    data.appendFormattedString(L"\nTHREAD ID - %lu:\n", tid);
    reportFile.writeString(data);

    AMDTUInt32 nbrRecords = 0;
    AMDTThreadSample* pThreaSampledData = nullptr;

    hr = AMDTGetThreadSampleData(tpReaderHandle, tid, &nbrRecords, &pThreaSampledData);

    if (nbrRecords > 0)
    {
        reportFile.writeString(L"PID,TID,CORE-ID,START-TS,END-TS,WAIT-TIME,TRANSITION-TIME,EXEC-TIME,THREAD-STATE,WAIT-REASON,WAIT-MODE,NBR-OF-STACKFRAMES,STACKFRAMES\n");
    }
    else
    {
        data.makeEmpty();
        data.appendFormattedString(L"No sample records for TID(%d).\n", tid);
        reportFile.writeString(data);
    }

    for (AMDTUInt32 j = 0; j < nbrRecords; j++)
    {
        data.makeEmpty();

        data.appendFormattedString(L"%d,%d,%d,", pThreaSampledData[j].m_processId,
                                   pThreaSampledData[j].m_threadId,
                                   pThreaSampledData[j].m_coreId);

        data.append(printTS(pThreaSampledData[j].m_startTS));
        data.append(printTS(pThreaSampledData[j].m_endTS));

        data.appendFormattedString(L"%u,%u,%llu ", pThreaSampledData[j].m_waitTime,
                                   pThreaSampledData[j].m_transitionTime,
                                   pThreaSampledData[j].m_execTime);

        char* pStr = nullptr;
        hr = AMDTGetThreadStateString(pThreaSampledData[j].m_threadState, &pStr);

        gtString s;
        s.fromASCIIString(pStr);
        data += L",";
        data.append(s);

        hr = AMDTGetThreadWaitReasonString(pThreaSampledData[j].m_waitReason, &pStr);
        s.makeEmpty();
        s.fromASCIIString(pStr);
        data += L",";
        data += s;

        hr = AMDTGetThreadWaitModeString(pThreaSampledData[j].m_waitMode, &pStr);
        s.makeEmpty();
        s.fromASCIIString(pStr);
        data += L",";
        data += s;

        data.appendFormattedString(L",%d,", pThreaSampledData[j].m_nbrStackFrames);

        for (AMDTUInt32 i = 0; i < pThreaSampledData[j].m_nbrStackFrames; i++)
        {
            //data.appendFormattedString(L" , 0x%llx ", pThreaSampledData[j].m_pStackFrames[i]);
            char* pfuncName;
            AMDTGetFunctionName(tpReaderHandle, pThreaSampledData[j].m_processId, pThreaSampledData[j].m_pStackFrames[i], &pfuncName);
            s.makeEmpty();
            s.fromASCIIString(pfuncName);
            data += s;
            data += L",";
            free(pfuncName);
        }

        data.append(L"\n");
        reportFile.writeString(data);
    }

    // free the pThreaSampledData
    if (nullptr != pThreaSampledData)
    {
        free(pThreaSampledData);
        pThreaSampledData = nullptr;
    }

    return hr;
}

static AMDTResult printAllThreadsSummary(AMDTThreadProfileDataHandle& tpReaderHandle, osFile& reportFile)
{
    AMDTResult hr;

    AMDTUInt32 nbrThreads = 0;
    AMDTThreadId* pThreads = nullptr;
    AMDTProcessId pid = static_cast<AMDTProcessId>(-1);

    hr = AMDTGetThreadIds(tpReaderHandle, pid, &nbrThreads, 0, nullptr);

    if (nbrThreads > 0)
    {
        pThreads = (AMDTThreadId*)malloc(sizeof(AMDTThreadId) * nbrThreads);
        hr = AMDTGetThreadIds(tpReaderHandle, pid, nullptr, nbrThreads, pThreads);
    }

    reportFile.writeString(L"\nALL THREADS SUMMARY\n");

    for (AMDTUInt32 i = 0; i < nbrThreads; i++)
    {
        bool reportHdr = (0 == i) ? true : false;
        printThreadSummary(tpReaderHandle, pThreads[i], reportFile, reportHdr);
    }

    reportFile.writeString(L"\n");

    if (nullptr != pThreads)
    {
        free(pThreads);
    }

    return hr;
}

static AMDTResult printThreadSummary(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile, bool reportHdr)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTThreadData threadData;

    hr = AMDTGetThreadData(tpReaderHandle, tid, &threadData);

    if (reportHdr)
    {
        reportFile.writeString(L"PID,TID,AFFINITY,CREATE-TS,TERMINATE-TS,TOTAL-WAIT-TIME,TOTAL-TRANSITION-TIME,TOTAL-EXEC-TIME,NBR-CS,NBR-CORE-SWITCHES\n");
    }

    gtString data;
    data.appendFormattedString(L"%d,%d,0x%lx,", threadData.m_processId, threadData.m_threadId, threadData.m_affinity);

    data.append(printTS(threadData.m_threadCreateTS));
    data.append(printTS(threadData.m_threadTerminateTS));

    data.appendFormattedString(L"%llu,%llu,%llu,%llu,%llu", threadData.m_totalWaitTime,
                               threadData.m_totalTransitionTime,
                               threadData.m_totalExeTime,
                               threadData.m_nbrOfContextSwitches,
                               threadData.m_nbrOfCoreSwitches);

    reportFile.writeString(data);
    reportFile.writeString(L"\n");

    return hr;
}

HRESULT CpuProfileReport::ReportTP()
{
    HRESULT retVal = S_OK;

    if (IsTpInputFile())
    {
        osFilePath reportFilePath = GetOutputFilePath();
        reportFilePath.setFileExtension(CPUPROFILE_CSV_REPORT_EXTENSION);
        osFile reportFile;

        if (!reportFilePath.isEmpty())
        {
            retVal = reportFile.open(reportFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        }

        AMDTResult hr;
        AMDTThreadProfileDataHandle tpReaderHandle;
        AMDTProcessId interestingPids[64];    // max 64 pids
        AMDTProcessId interestingPid = static_cast<AMDTProcessId>(-1);

        // input file
        const char* pLogFilePath = m_inputFilePath.asString().asASCIICharArray();

        hr = AMDTOpenThreadProfile(pLogFilePath, &tpReaderHandle);

        if (!m_args.GetPidsList().empty())
        {
            // just support one now
            for (AMDTUInt32 i = 0; i < m_args.GetPidsList().size(); i++)
            {
                interestingPid = m_args.GetPidsList().at(i);
                interestingPids[i] = interestingPid;
            }

            hr = AMDTSetFilterProcesses(tpReaderHandle, 1, interestingPids);
        }

        hr = AMDTProcessThreadProfileData(tpReaderHandle);

        // print the all processes summary
        printAllProcessesSummary(tpReaderHandle, reportFile);

        // Set the symbol server path
        gtString symServer(L"http://msdl.microsoft.com/download/symbols");
        hr = AMDTSetSymbolSearchPath(tpReaderHandle, nullptr, symServer.asASCIICharArray(), nullptr);

        // for the interesting processes and its child processs print the thread sampled data
        if (TP_ALL_PIDS == interestingPid)
        {
            printAllProcessesSampleData(tpReaderHandle, reportFile);
        }
        else
        {
            for (AMDTUInt32 i = 0; i < m_args.GetPidsList().size(); i++)
            {
                interestingPid = m_args.GetPidsList().at(i);

                printProcessSampleData(tpReaderHandle, interestingPid, reportFile);

                // report child processes - only the first level
                AMDTProcessData procData;
                hr = AMDTGetProcessData(tpReaderHandle, interestingPid, &procData);

                for (AMDTUInt32 j = 0; j < procData.m_nbrChildProcesses; j++)
                {
                    interestingPid = procData.m_pChildProcesses[j];
                    printProcessSampleData(tpReaderHandle, interestingPid, reportFile);
                }
            }
        }

        reportFile.writeString(L"\n");

        // print the all threads summary
        if (TP_ALL_PIDS == interestingPid)
        {
            printAllThreadsSummary(tpReaderHandle, reportFile);
        }

        hr = AMDTCloseThreadProfile(tpReaderHandle);

        retVal = (AMDT_STATUS_OK == hr) ? S_OK : E_FAIL;
    }

    return retVal;
}

//      Private Member Functions

void CpuProfileReport::SetupEnvironment()
{
    // Set up the environmental variable so the event files can be found.
    osEnvironmentVariable eventDataPath;
    eventDataPath._name = L"CPUPerfAPIDataPath";
    osFilePath eventFilePath;

    if (osGetCurrentApplicationDllsPath(eventFilePath) || osGetCurrentApplicationPath(eventFilePath))
    {
        eventFilePath.clearFileName();
        eventFilePath.clearFileExtension();

        eventFilePath.appendSubDirectory(L"Data");
        eventFilePath.appendSubDirectory(L"Events");

        gtString eventFilePathStr = eventFilePath.fileDirectoryAsString();

        eventDataPath._value.appendFormattedString(L"%ls/", eventFilePathStr.asCharArray());
        osSetCurrentProcessEnvVariable(eventDataPath);

        m_error = S_OK;
    }

    return;
}

void CpuProfileReport::ValidateOptions()
{
    // REPORT Options
    //  -i <input file - PRD/EBP/CXLDB>
    //  -o <output path dir> output dir, in which processed or report files will be generated
    //  -F csv | text        file format - only CSV format is supported (Optional)
    //  -V <view xmL>        by default only the raw data will be reported
    //
    //  -R <overview|process|module|callgraph|imix|all> Sections to be reported
    //                       process & module are mutually exclusive
    //                       module & callgraph - NOT possible
    //
    //  -e <event-index>     Use this to sort the OverView and select the CallGraph
    //
    //  -I                   ignore system modules
    //  -N                   seperate by Numa; By defualt it is core - CXL-1.6 ?
    //  -O                   seperate by Core; By defualt it is core - CXL-1.6 ?
    //  -P                   Show Percentage
    //  -c <core mask>       core affinity mask
    //  -D <debug symbol paths>
    //  -S <symbol server directories>
    //  -X                  enable cache in default path (output-dir/cache), if it already exists, error...
    //  -L                  Limit - 1.6, percent-cutoff,cumulative-cutoff,minimum-count

    m_error = E_FAIL;
    osCpuid cpuInfo;

    // validate for valid input dir
    if (! m_args.GetInputFile().isEmpty())
    {
        m_inputFilePath.setFullPathFromString(m_args.GetInputFile());

        if (m_inputFilePath.isDirectory())
        {
            reportError(false, L"Input file (" STR_FORMAT L") is not specified.\n", m_inputFilePath.asString(true).asCharArray());
            return;
        }

        if (! m_inputFilePath.exists())
        {
            reportError(false, L"Input file (" STR_FORMAT L") does not exist.\n", m_inputFilePath.asString(true).asCharArray());
            return;
        }

        gtString fileExtension;
        m_inputFilePath.getFileExtension(fileExtension);
        m_isRawInputFile = (0 == fileExtension.compareNoCase(CPUPROFILE_RAWFILE_EXTENSION)) ? true : false;
        m_isEbpInputFile = (0 == fileExtension.compareNoCase(CPUPROFILE_EBP_REPORT_EXTENSION)) ? true : false;
        m_isTpInputFile = (0 == fileExtension.compareNoCase(THREAD_PROFILE_DATAFILE_EXTENSION)) ? true : false;
        m_isDbInputFile = (0 == fileExtension.compareNoCase(CPUPROFILE_DBFILE_EXTENSION)) ? true : false;

        if (!m_isRawInputFile && !m_isEbpInputFile && !m_isTpInputFile && !m_isDbInputFile)
        {
            reportError(false, L"Invalid input file (" STR_FORMAT L").\n", m_inputFilePath.asString(true).asCharArray());
            return;
        }
    }
    else
    {
        reportError(false, L"Input file is missing. Please use option(-i) to specify input file.\n");
        return;
    }

    // validate the output file path
    if (! m_args.GetOutputFile().isEmpty())
    {
        osFilePath outputPath(m_args.GetOutputFile());
        outputPath.reinterpretAsDirectory();

        if (! outputPath.exists())
        {
            reportError(false, L"Output path (" STR_FORMAT L") does not exists.\n", m_args.GetOutputFile().asCharArray());
            return;
        }

        if (! outputPath.isDirectory())
        {
            reportError(false, L"Output path (" STR_FORMAT L") is not a directory.\n", m_args.GetOutputFile().asCharArray());
            return;
        }
    }
    else
    {
        if (!m_args.IsReportSampleCount())
        {
            reportError(false, L"Output file path is missing. Please use option(-o) to specify the output file path.\n");
            return;
        }
    }

    // Check the output file format
    if (! m_args.GetOutputFileFormat().isEmpty())
    {
        // Only CSV is supported
        if (! m_args.GetOutputFileFormat().toLowerCase().compare(L"csv"))
        {
            m_reportType = CPU_PROFILE_REPORT_TYPE_CSV;
        }
        else
        {
            reportError(false, L"Unsupported output file format(" STR_FORMAT L") specified with option(-F).\n", m_args.GetOutputFileFormat().asCharArray());
            return;
        }
    }

    // Validate the report sections requested to be printed
    gtString reportSections = m_args.GetSectionsToReport().isEmpty() ? CPU_PROFILE_REPORT_ALL
                              : m_args.GetSectionsToReport();

    if (!reportSections.isEmpty())
    {
        int pos;
        int startPosition = 0;
        bool invalidReportSection = false;

        do
        {
            pos = reportSections.find(L",", startPosition);
            int endPosition = (-1 != pos) ? (pos - 1) : reportSections.length();

            gtString tmpStr;
            reportSections.getSubString(startPosition, endPosition, tmpStr);

            if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_ALL))
            {
                m_isReportOverview = true;
                m_isReportAggregateByProcess = true;
                m_isReportAggregateByModule = false;
                m_isReportCallGraph = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_OVERVIEW))
            {
                m_isReportOverview = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_PROCESS))
            {
                m_isReportAggregateByProcess = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_MODULE))
            {
                m_isReportAggregateByModule = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_CALLGRAPH))
            {
                m_isReportCallGraph = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_IMIX))
            {
                m_isReportImix = true;
            }
            else
            {
                invalidReportSection = true;
            }

            startPosition = pos + 1;
        }
        while (-1 != pos);

        if (invalidReportSection)
        {
            reportError(false, L"Invalid report section (" STR_FORMAT L") specified with option(-R).\n",
                        reportSections.asCharArray());
            return;
        }

        // TODO:
        if (m_isReportAggregateByModule)
        {
            reportError(false, L"Aggregate by Module is not yet supported!!\n");
            return;
        }

        if (m_isReportAggregateByProcess && m_isReportAggregateByModule)
        {
            reportError(false, L"Aggregate by Process and Aggregate by Module together not supported.\n");
            return;
        }

        if (m_isReportAggregateByModule && m_isReportCallGraph)
        {
            reportError(false, L"Aggregate by Module and Callgraph together not supported.\n");
            return;
        }
    }

    // Validate the core affinity mask
    // TODO: Currently this supports only upto 64 cores
    if (GT_UINT64_MAX != m_args.GetCoreAffinityMask())
    {
        int nbrCores = 0;
        gtUInt64 maxAffinity = 0;

        // TODO: this should be read from DB
        osGetAmountOfLocalMachineCPUs(nbrCores);

        if (nbrCores < 64)
        {
            maxAffinity = (1ULL << nbrCores) - 1;
        }
        else
        {
            maxAffinity = GT_UINT64_MAX;
        }

        if (maxAffinity < m_args.GetCoreAffinityMask())
        {
            reportError(false, L"Invalid core affinity mask (0x%lx) specified with option(-c).\n", m_args.GetCoreAffinityMask());
            return;
        }
    }

    // Fill-in the m_coresList from the coreAffinity mask
    // If separate-by-core and core-affinity-mask not used, then report for all the cores
    if (m_args.IsReportByCore())
    {
        gtUInt64 coreMask = m_args.GetCoreAffinityMask();

        if (GT_UINT64_MAX == coreMask)
        {
            int nbrCores = 0;

            // TODO: this should be read from profile reader
            osGetAmountOfLocalMachineCPUs(nbrCores);

            if (nbrCores < 64)
            {
                coreMask = (1ULL << nbrCores) - 1;
            }
            else
            {
                coreMask = GT_UINT64_MAX;
            }
        }

        gtUInt32 coreId = 0;

        while (coreMask)
        {
            if (coreMask & 0x1)
            {
                m_coresList.push_back(coreId);

                coreMask >>= 1;
                coreId++;
            }
        }
    }

    // For predefined profile configs, construct the profile XML file path
    osFilePath profileFilePath;
    gtString reportConfig = m_args.GetViewConfigName();

    if (!reportConfig.isEmpty())
    {
        // TODO: call GetReportConfigurations() and check whether the user provided View config name 
        // m_args.GetViewConfigName() is in the list returned by the API?
        // returned 
        reportError(false, L"-V Option is not yet supported.\n");
        return;
    }

    m_error = S_OK;
    return;
}

// Actually this one returns the absolute path with only the base file name.
// with this base name
osFilePath& CpuProfileReport::GetOutputFilePath()
{
    m_error = S_OK;

    if (m_outputFilePath.isEmpty())
    {
        gtString outputFile = m_args.GetOutputFile();

        if (outputFile.isEmpty())
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            wchar_t  tmpPath[OS_MAX_PATH] = { L'\0' };
            GetTempPathW(OS_MAX_PATH, tmpPath);
            outputFile = gtString(tmpPath);
#else
            // TODO: Use P_tmpdir on Linux
            wchar_t  tmpPath[OS_MAX_PATH] = L"/tmp/";
            outputFile = gtString(tmpPath);
#endif // AMDT_BUILD_TARGET
        }

        m_outputFilePath = osFilePath(outputFile);
        m_outputFilePath.reinterpretAsDirectory();

        // Get the basename from the input file and create a output dir
        // in which the output file CXLDB will be created
        osFilePath inputFile(m_args.GetInputFile());
        gtString inputFileName;
        inputFile.getFileName(inputFileName);

        m_outputFilePath.appendSubDirectory(inputFileName);

        // check if the base dir exists
        osDirectory osDir;
        m_outputFilePath.getFileDirectory(osDir);

        if (! osDir.exists())
        {
            osDir.create();
        }
    }

    if (isOK())
    {
        // Add the output file name - which is base of the input file name
        gtString inputBaseName;
        m_inputFilePath.getFileName(inputBaseName);
        m_outputFilePath.setFileName(inputBaseName);
        m_outputFilePath.setFileExtension(L"cxlcpdb");
    }

    return m_outputFilePath;
}

osFilePath& CpuProfileReport::GetDBFilePath()
{
    if (IsRawInputFile())
    {
        return GetOutputFilePath();
    }
    else
    {
        return m_inputFilePath;
    }
}

// Actually this one returns the absolute path with only the base file name.
// with this base name
bool CpuProfileReport::InitializeReportFile()
{
    m_error = S_OK;

    // CSV Reporter
    osFilePath reportFilePath = GetOutputFilePath();

    if (CPU_PROFILE_REPORT_TYPE_CSV == m_reportType)
    {
        reportFilePath.setFileExtension(CPUPROFILE_CSV_REPORT_EXTENSION);
    }
    else if (CPU_PROFILE_REPORT_TYPE_TEXT == m_reportType)
    {
        reportFilePath.setFileExtension(CPUPROFILE_TEXT_REPORT_EXTENSION);
    }
    else if (CPU_PROFILE_REPORT_TYPE_XML == m_reportType)
    {
        reportFilePath.setFileExtension(CPUPROFILE_XML_REPORT_EXTENSION);
    }
    else
    {
        return false;
    }

    m_pReporter = new CSVReporter(reportFilePath);
    m_pReporter->Open();

    return true;
}

void CpuProfileReport::ReportSampleCount(bool sepByCore)
{
    AMDTProfileCounterDescVec counterDesc;
    AMDTProfileSamplingConfigVec samplingConfigVec;
    AMDTProfileSessionInfo sessionInfo;
    m_profileDbReader.GetProfileSessionInfo(sessionInfo);

    m_profileDbReader.GetSampledCountersList(counterDesc);

    for (const auto& counter : counterDesc)
    {
        AMDTProfileSamplingConfig sampleConfig;
        m_profileDbReader.GetSamplingConfiguration(counter.m_id, sampleConfig);
        samplingConfigVec.push_back(sampleConfig);
    }

    AMDTSampleValueVec sampleValueVec;
    m_profileDbReader.GetSampleCount(sepByCore, sampleValueVec);

    fprintf(stderr, "\nCPCLI>>> coreMask  counterId   counterName      samplingInterval   nbrSamples\n");

    for (const auto& value : sampleValueVec)
    {
        AMDTCounterId cid = value.m_counterId;

        auto counterInfo = std::find_if(samplingConfigVec.begin(), samplingConfigVec.end(),
            [&cid](AMDTProfileSamplingConfig const& aConfig) { return aConfig.m_id == cid; });

        AMDTUInt32 eventId = counterInfo->m_hwEventId;
        auto aCounterDesc = std::find_if(counterDesc.begin(), counterDesc.end(),
            [&eventId](AMDTProfileCounterDesc const& aCounter) { return aCounter.m_hwEventId == eventId; });

        gtString printStr;
        aCounterDesc->m_abbrev.replace(L" ", L"-");
        printStr.appendFormattedString(L"    0x%llx %10x    %-20s %8llu  %12llu",
            sessionInfo.m_coreAffinity, counterInfo->m_hwEventId, aCounterDesc->m_abbrev.asCharArray(), counterInfo->m_samplingInterval, static_cast<AMDTUInt64>(value.m_sampleCount));

        fprintf(stderr, "CPCLI>>> %s\n", printStr.asASCIICharArray());
    }

    return;
}

void CpuProfileReport::ReportExecution(AMDTProfileSessionInfo& sessionInfo)
{
    gtVector<gtString> sectionHdrs;
    gtList<std::pair<gtString, gtString>> sectionDataList;

    // EXECUTION
    //    Target Path:,classic.exe
    //    Working Directory:,c:\temp
    //    Command Line Arguments:,123 -i o
    //    Environment Variables:
    //    Cpu Details:
    //    Operating System:

    sectionHdrs.push_back(CODEXL_REPORT_HDR);
    sectionHdrs.push_back(EXECUTION_SECTION_HDR);

    std::pair<gtString, gtString> keyValuePair;

    // Target Path
    keyValuePair.first = EXECUTION_TARGET_PATH;
    keyValuePair.second = sessionInfo.m_targetAppPath;
    sectionDataList.push_back(keyValuePair);

    // Target Args
    keyValuePair.first = EXECUTION_TARGET_ARGS;
    keyValuePair.second = sessionInfo.m_targetAppCmdLineArgs;
    sectionDataList.push_back(keyValuePair);

    // Working Directory
    keyValuePair.first = EXECUTION_WORK_DIR;
    keyValuePair.second = sessionInfo.m_targetAppWorkingDir;
    sectionDataList.push_back(keyValuePair);

    // Environment Variables
    keyValuePair.first = EXECUTION_TARGET_ENV_VARS;
    keyValuePair.second = sessionInfo.m_targetAppEnvVars;
    sectionDataList.push_back(keyValuePair);

    // Cpu Details
    keyValuePair.first = PROFILE_CPU_DETAILS;
    gtString tmpStr;
    tmpStr.appendFormattedString(L"Family(0x%lx), Model(0x%lx), Number of Cores(%u)",
        sessionInfo.m_cpuFamily, sessionInfo.m_cpuModel, sessionInfo.m_coreCount);
    keyValuePair.second = tmpStr;
    sectionDataList.push_back(keyValuePair);

    // Target Operating System
    keyValuePair.first = PROFILE_TAREGET_OS;
    tmpStr = sessionInfo.m_targetMachineName;

    if (tmpStr.isEmpty())
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        tmpStr = L"Windows";
#else
        tmpStr = L"Linux";
#endif
    }

    keyValuePair.second = tmpStr;
    sectionDataList.push_back(keyValuePair);

    if ((nullptr != m_pReporter) && m_pReporter->IsOpened())
    {
        m_pReporter->ReportExecution(sectionHdrs, sectionDataList);
    }
}

void CpuProfileReport::ReportProfileDetails(AMDTProfileSessionInfo& sessionInfo)
{
    gtVector<gtString> sectionHdrs;
    gtList<std::pair<gtString, gtString>> sectionDataList;

    // PROFILE DETAILS
    // Profile Session Type:,TBP
    // Profile Scope:,SWP
    // CPU Affinity:,0xf
    // Call Stack Sampling:,True
    // Profile Start Time:,10-11-2014
    // Profile End Time:,11-11-2014
    // Profile Duration:,10 seconds
    // Data Folder:,c:\temp
    // SamplingEvents,
    //  ,cpu-cycles
    //  ,retired-instructions

    sectionHdrs.push_back(PROFILE_DETAILS_SECTION_HDR);
    std::pair<gtString, gtString> keyValuePair;

    // Profile Session Type
    keyValuePair.first = PROFILE_SESSION_TYPE;
    keyValuePair.second = sessionInfo.m_sessionType;
    sectionDataList.push_back(keyValuePair);

    // Profile Scope
    keyValuePair.first = PROFILE_SCOPE;
    keyValuePair.second = sessionInfo.m_sessionScope;
    sectionDataList.push_back(keyValuePair);

    // Core Affinity Mask
    keyValuePair.first = PROFILE_CPU_AFFINITY;
    gtString tmpStr;
    tmpStr.appendFormattedString(L"0x%lx", sessionInfo.m_coreAffinity);
    keyValuePair.second = tmpStr;
    sectionDataList.push_back(keyValuePair);

    // Profile Start Time
    keyValuePair.first = PROFILE_START_TIME;
    keyValuePair.second = sessionInfo.m_sessionStartTime;
    sectionDataList.push_back(keyValuePair);

    // Profile End Time
    keyValuePair.first = PROFILE_END_TIME;
    keyValuePair.second = sessionInfo.m_sessionEndTime;
    sectionDataList.push_back(keyValuePair);

    // Profile Duration
    keyValuePair.first = PROFILE_DURATION;
    osTime startTime, endTime;
    startTime.setFromDateTimeString(osTime::LOCAL, sessionInfo.m_sessionStartTime, osTime::NAME_SCHEME_FILE);
    endTime.setFromDateTimeString(osTime::LOCAL, sessionInfo.m_sessionEndTime, osTime::NAME_SCHEME_FILE);
    gtUInt64 profileDuration = endTime.secondsFrom1970() - startTime.secondsFrom1970();

    tmpStr.makeEmpty();
    tmpStr.appendFormattedString(L"%llu seconds", profileDuration);
    keyValuePair.second = tmpStr;
    sectionDataList.push_back(keyValuePair);

    // Profile Data Folder
    keyValuePair.first = PROFILE_DATA_FOLDER;
    keyValuePair.second = sessionInfo.m_sessionDir; // TBD: Do we need to report this ?
    sectionDataList.push_back(keyValuePair);

    keyValuePair.first = PROFILE_CSS;
    keyValuePair.second = sessionInfo.m_cssEnabled ? L"True" : L"False";
    sectionDataList.push_back(keyValuePair);

    keyValuePair.first = PROFILE_CSS_DEPTH;
    tmpStr.makeEmpty();
    tmpStr.appendFormattedString(L"%ld", sessionInfo.m_unwindDepth);
    keyValuePair.second = tmpStr;
    sectionDataList.push_back(keyValuePair);

    const wchar_t* pCssScopeStr = nullptr;

    switch (sessionInfo.m_unwindScope)
    {
    case CP_CSS_SCOPE_USER:
        pCssScopeStr = L"User space";
        break;

    case CP_CSS_SCOPE_KERNEL:
        pCssScopeStr = L"Kernel space";
        break;

    case CP_CSS_SCOPE_ALL:
        pCssScopeStr = L"User space and Kernel space";
        break;

    default:
        break;
    }

    if (nullptr != pCssScopeStr)
    {
        keyValuePair.first = PROFILE_CSS_SCOPE;
        keyValuePair.second = pCssScopeStr;
        sectionDataList.push_back(keyValuePair);
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    keyValuePair.first = PROFILE_CSS_FPO;
    keyValuePair.second = sessionInfo.m_cssFPOEnabled ? L"True" : L"False";
    sectionDataList.push_back(keyValuePair);
#endif

    keyValuePair.first = PROFILE_CSS_FREQUENCY;
    tmpStr.makeEmpty();
    tmpStr.appendFormattedString(L"%u", sessionInfo.m_cssInterval);
    keyValuePair.second = tmpStr;
    sectionDataList.push_back(keyValuePair);

    if ((nullptr != m_pReporter) && m_pReporter->IsOpened())
    {
        m_pReporter->ReportProfileData(sectionHdrs, sectionDataList);
    }
}

void CpuProfileReport::ReportMonitoredEventDetails()
{
    gtVector<gtString> sectionHdrs;
    sectionHdrs.push_back(PROFILE_SAMPLING_EVENTS);

    AMDTProfileCounterDescVec counterDesc;
    AMDTProfileSamplingConfigVec samplingConfigVec;

    m_profileDbReader.GetSampledCountersList(counterDesc);

    for (const auto& counter : counterDesc)
    {
        AMDTProfileSamplingConfig sampleConfig;
        m_profileDbReader.GetSamplingConfiguration(counter.m_id, sampleConfig);
        samplingConfigVec.push_back(sampleConfig);
    }

    m_pReporter->ReportSamplingSpec(sectionHdrs, counterDesc, samplingConfigVec);

    // set the sort event id, name
    int sortEventIndex = m_args.GetSortEventIndex();

    if (samplingConfigVec.size() < 1)
    {
        sortEventIndex = -1;
    }
    else if (sortEventIndex >= 0 && static_cast<unsigned int>(sortEventIndex) >= samplingConfigVec.size())
    {
        reportError(false, L"Invalid event index(%d) specified with option(-e).\n", sortEventIndex);
        sortEventIndex = 0;
    }

    m_sortEventId = (-1 == sortEventIndex) ? -1 : samplingConfigVec[sortEventIndex].m_hwEventId;
    m_sortCounterId = (-1 == sortEventIndex) ? -1 : samplingConfigVec[sortEventIndex].m_id;

    if (m_sortEventId == 0xFFFFFFFF)
    {
        m_sortEventName = L"All Events";
    }
    else
    {
        m_sortEventName = L"???";
    }

    for (const auto& counter : counterDesc)
    {
        if (counter.m_hwEventId == m_sortEventId)
        {
            m_sortEventName = counter.m_name;
            break;
        }
    }

    m_profileEventsNameVec.clear();
    for (const auto& configInfo : samplingConfigVec)
    {
        gtUInt32 eventId = configInfo.m_hwEventId;
        gtString eventName(L"???");

        for (const auto& counterInfo : counterDesc)
        {
            if (eventId == counterInfo.m_hwEventId)
            {
                eventName = counterInfo.m_name;
                break;
            }
        }

        m_profileEventsNameVec.push_back(eventName);
    }
}

void CpuProfileReport::ReportOverviewData(AMDTProfileSessionInfo& sessionInfo)
{
    // Report Overview only if requested
    if (IsReportOverview())
    {
        gtVector<gtString> sectionHdrs;
        gtString tmpStr;

        bool sepByCore = m_args.IsReportByCore();
        bool ignoreSysModules = m_args.IsIgnoreSystemModules();
        gtUInt64 coreMask = sessionInfo.m_coreAffinity;

        gtVector<AMDTProfileReportConfig> reportConfigs;
        m_profileDbReader.GetReportConfigurations(reportConfigs);

        AMDTProfileDataOptions options;
        options.m_coreMask = coreMask;
        options.m_ignoreSystemModules = ignoreSysModules;
        options.m_doSort = true;
        options.m_summaryCount = 5;
        options.m_isSeperateByCore = sepByCore;
        options.m_othersEntryInSummary = false;

        AMDTProfileReportConfig* pReportConfig = &reportConfigs[0]; // all-data report view
        gtString reportConfigName = m_args.GetViewConfigName();

        if (!reportConfigName.isEmpty())
        {
            auto aReportConfig = std::find_if(reportConfigs.begin(), reportConfigs.end(),
                [&reportConfigName](AMDTProfileReportConfig const& aConfig) { return aConfig.m_name.compareNoCase(reportConfigName) == 0; });

            pReportConfig = &(*aReportConfig);
        }

        for (const auto& counter : pReportConfig->m_counterDescs)
        {
            options.m_counters.push_back(counter.m_id);
        }

        m_profileDbReader.SetReportOption(options);

        // Overview FUNCTIONS
        tmpStr = OVERVIEW_FUNCTIONS_SECTION_HDR;
        tmpStr.appendFormattedString(L" (Sort Event - ");
        tmpStr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
        tmpStr.appendFormattedString(L")");
        sectionHdrs.push_back(tmpStr);

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_FUNCTIONS);

        // Add the profiled event's names
        for (gtUInt32 i = 0; i < m_profileEventsNameVec.size(); i++)
        {
            tmpStr.appendFormattedString(L"," STR_FORMAT, m_profileEventsNameVec[i].asCharArray());
        }

        tmpStr.appendFormattedString(L"," STR_FORMAT, OVERVIEW_SECTION_MODULE);
        sectionHdrs.push_back(tmpStr);

        AMDTProfileDataVec funcProfileData;
        m_profileDbReader.GetFunctionSummary(m_sortCounterId, funcProfileData);
        m_pReporter->WriteOverviewFunction(sectionHdrs, funcProfileData, m_args.IsShowPercentage());
        funcProfileData.clear();

        // Overview PROCESS
        sectionHdrs.clear();
        tmpStr.makeEmpty();

        tmpStr = OVERVIEW_PROCESSES_SECTION_HDR;
        tmpStr.appendFormattedString(L" (Sort Event - ");
        tmpStr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
        tmpStr.appendFormattedString(L")");
        sectionHdrs.push_back(tmpStr);

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(STR_FORMAT L"," STR_FORMAT, OVERVIEW_SECTION_PROCESS, OVERVIEW_SECTION_PID);

        // Add the profiled event's names
        for (gtUInt32 i = 0; i < m_profileEventsNameVec.size(); i++)
        {
            tmpStr.appendFormattedString(L"," STR_FORMAT, m_profileEventsNameVec[i].asCharArray());
        }

        sectionHdrs.push_back(tmpStr);

        AMDTProfileDataVec processProfileData;
        m_profileDbReader.GetProcessSummary(m_sortCounterId, processProfileData);
        // Don't print process overview if there is only 1 process.
        if (processProfileData.size() > 1)
        {
            m_pReporter->WriteOverviewProcess(sectionHdrs, processProfileData, m_args.IsShowPercentage());
        }
        processProfileData.clear();

        // Overview - MODULE
        sectionHdrs.clear();
        tmpStr.makeEmpty();

        tmpStr.makeEmpty();
        tmpStr = OVERVIEW_MODULES_SECTION_HDR;
        tmpStr.appendFormattedString(L" (Sort Event - ");
        tmpStr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
        tmpStr.appendFormattedString(L")");
        sectionHdrs.push_back(tmpStr);

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_MODULE);

        // Add the profiled event's names
        for (gtUInt32 i = 0; i < m_profileEventsNameVec.size(); i++)
        {
            tmpStr.appendFormattedString(L"," STR_FORMAT, m_profileEventsNameVec[i].asCharArray());
        }

        sectionHdrs.push_back(tmpStr);

        AMDTProfileDataVec moduleProfileData;
        m_profileDbReader.GetModuleSummary(m_sortCounterId, moduleProfileData);
        m_pReporter->WriteOverviewModule(sectionHdrs, moduleProfileData, m_args.IsShowPercentage());
        moduleProfileData.clear();
    }
}

void CpuProfileReport::ReportProcessData()
{
    if (!IsReportAggregateByProcess() && !IsReportCallGraph())
    {
        return;
    }

    gtVector<gtString> sectionHdrs;
    gtString tmpStr;

    AMDTProfileDataVec processProfileData;
    m_profileDbReader.GetProcessSummary(m_sortCounterId, processProfileData);

    // TBD - just print the top 5 processes
    for (const auto& proc : processProfileData)
    {
        AMDTProcessId pid = static_cast<AMDTProcessId>(proc.m_id);

        if (IsReportAggregateByProcess())
        {
            // PROCESS HEADER
            gtString procHdr(PROCESS_PROFILE_HDR);
            procHdr.appendFormattedString(STR_FORMAT, proc.m_name.asCharArray());
            procHdr.appendFormattedString(L" (PID - %ld)", pid);

            sectionHdrs.clear();
            sectionHdrs.push_back(procHdr);
            tmpStr.makeEmpty();
            tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_PROCESS);

            for (const auto& evName : m_profileEventsNameVec)
            {
                tmpStr.appendFormattedString(L"," STR_FORMAT, evName.asCharArray());
            }

            sectionHdrs.push_back(tmpStr);

            // REPORT the process
            m_pReporter->WritePidSummary(sectionHdrs, proc, m_args.IsShowPercentage(), m_args.IsReportByCore());

            //// Get the module data for this pid
            AMDTProfileDataVec moduleProfileData;
            m_profileDbReader.GetModuleProfileData(static_cast<AMDTProcessId>(proc.m_id), AMDT_PROFILE_ALL_MODULES, moduleProfileData);

            // MODULE section Hdrs
            sectionHdrs.clear();
            sectionHdrs.push_back(MODULE_SUMMARY_SECTION_HDR);
            tmpStr.makeEmpty();
            tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_MODULE);

            for (const auto& evName : m_profileEventsNameVec)
            {
                tmpStr.appendFormattedString(L"," STR_FORMAT, evName.asCharArray());
            }

            sectionHdrs.push_back(tmpStr);

            if (moduleProfileData.empty())
            {
                tmpStr.makeEmpty();
                tmpStr.appendFormattedString(STR_FORMAT, NO_SAMPLES_TO_DISPLAY);
                sectionHdrs.push_back(tmpStr);
            }

            m_pReporter->WritePidModuleSummary(sectionHdrs, moduleProfileData, m_args.IsShowPercentage(), m_args.IsReportByCore());
            moduleProfileData.clear();

            //  FUNCTIONS  SUMMARY
            AMDTProfileDataVec functionProfileData;
            m_profileDbReader.GetFunctionProfileData(static_cast<AMDTProcessId>(proc.m_id), AMDT_PROFILE_ALL_MODULES, functionProfileData);

            sectionHdrs.clear();
            sectionHdrs.push_back(FUNCTION_SUMMARY_SECTION_HDR);
            tmpStr.makeEmpty();
            tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_FUNCTIONS);

            for (const auto& evName : m_profileEventsNameVec)
            {
                tmpStr.appendFormattedString(L"," STR_FORMAT, evName.asCharArray());
            }

            tmpStr.appendFormattedString(L"," STR_FORMAT, OVERVIEW_SECTION_MODULE);

            sectionHdrs.push_back(tmpStr);

            if (functionProfileData.empty())
            {
                tmpStr.makeEmpty();
                tmpStr.appendFormattedString(STR_FORMAT, NO_SAMPLES_TO_DISPLAY);
                sectionHdrs.push_back(tmpStr);
            }

            m_pReporter->WritePidFunctionSummary(sectionHdrs, functionProfileData, m_args.IsShowPercentage(), m_args.IsReportByCore());
            functionProfileData.clear();
        }

        // Report CallGraph
        if (IsReportCallGraph() && m_profileDbReader.IsProcessHasCssSamples(pid))
        {
            ReportCSSData(static_cast<AMDTProcessId>(proc.m_id));
        }
    } // iterate over the process list
}

bool CpuProfileReport::ReportCSSData(AMDTProcessId pid)
{
    bool retVal = false;
    bool showPerc = true;
    AMDTProfileCounterDescVec counterDesc;
    int sortEventIndex = m_args.GetSortEventIndex();

    m_profileDbReader.GetSampledCountersList(counterDesc);

    AMDTProfileCounterDesc& sortEvent = ((sortEventIndex != -1) && static_cast<size_t>(sortEventIndex) < counterDesc.size()) 
                                            ? counterDesc[sortEventIndex] : counterDesc[0];

    // Get the callgraph function list
    AMDTCallGraphFunctionVec cgFuncsVec;
    retVal = m_profileDbReader.GetCallGraphFunctions(pid, sortEvent.m_id, cgFuncsVec);

    // Headers
    gtVector<gtString> sectionHdrs;

    gtString cgHdr(CALLGRAPH_HDR);
    cgHdr.appendFormattedString(L" (PID - %ld)", pid);
    cgHdr.appendFormattedString(L" (Sort Event - ");
    cgHdr.appendFormattedString(STR_FORMAT, sortEvent.m_name.asCharArray());
    cgHdr.appendFormattedString(L")");
    sectionHdrs.push_back(cgHdr);

    sectionHdrs.push_back(CALLGRAPH_FUNCTION_SUMMARY_HDR);

    gtString tmpStr(CALLGRAPH_FUNCTION);
    tmpStr += L",";
    tmpStr += CALLGRAPH_SELF_SAMPLES;
    tmpStr += L",";
    tmpStr += CALLGRAPH_DEEP_SAMPLES;

    if (showPerc)
    {
        tmpStr += L",";
        tmpStr += CALLGRAPH_PERC_DEEP_SAMPLES;
    }

    tmpStr += L",";
    tmpStr += CALLGRAPH_PATH_COUNT;
    tmpStr += L",";
    tmpStr += CALLGRAPH_SOURCE_FILE;
    tmpStr += L",";
    tmpStr += CALLGRAPH_MODULE;
    sectionHdrs.push_back(tmpStr);

    // write the callgraph function table
    // TODO: Index for the functions?

    m_pReporter->WriteCallGraphFunctionSummary(sectionHdrs, cgFuncsVec, showPerc);

    // Write the callgraph
    sectionHdrs.clear();
    sectionHdrs.push_back(CALLGRAPH_SECTION_HDR);

    tmpStr.makeEmpty();
    tmpStr += L",,";
    tmpStr += L"                SAMPLES UNDER PARENTS";
    tmpStr += L",";
    tmpStr += L"                ";
    tmpStr += CALLGRAPH_PARENTS;

    sectionHdrs.push_back(tmpStr);

    tmpStr.makeEmpty();
    tmpStr += CALLGRAPH_INDEX;
    tmpStr += L",";
    tmpStr += CALLGRAPH_DEEP_SAMPLES;
    tmpStr += L",";
    tmpStr += L"SAMPLES IN FUNCTION";
    tmpStr += L",";
    tmpStr += CALLGRAPH_FUNCTION;
    tmpStr += L",";
    tmpStr += CALLGRAPH_MODULE;
    tmpStr += L",";
    tmpStr += CALLGRAPH_SOURCE_FILE;
    sectionHdrs.push_back(tmpStr);

    tmpStr.makeEmpty();
    tmpStr += L",,";
    tmpStr += L"                SAMPLES IN CHILDREN";
    tmpStr += L",";
    tmpStr += L"                ";
    tmpStr += CALLGRAPH_CHILDREN;

    sectionHdrs.push_back(tmpStr);

    m_pReporter->WriteCallGraphHdr(sectionHdrs);

    // Get the callgraph function list
    for (const auto& cgFunc : cgFuncsVec)
    {
        AMDTCallGraphFunctionVec caller;
        AMDTCallGraphFunctionVec callee;

        retVal = m_profileDbReader.GetCallGraphFunctionInfo(pid, cgFunc.m_functionInfo.m_functionId, caller, callee);

        m_pReporter->WriteCallGraph(cgFunc, caller, callee, true);
    }

    return retVal;
}

bool CpuProfileReport::InitCoresList()
{
    // Fill-in the m_coresList from the coreAffinity mask
    // If separate-by-core and core-affinity-mask not used, then report for all the cores
    if (m_args.IsReportByCore())
    {
        AMDTProfileSessionInfo sessionInfo;
        m_profileDbReader.GetProfileSessionInfo(sessionInfo);

        gtUInt64 coreMask = m_args.GetCoreAffinityMask();

        if (static_cast<gtUInt64>(-1) == coreMask)
        {
            gtUInt32 nbrCores = sessionInfo.m_coreCount;

            for (gtUInt32 core = 0; core < nbrCores; core++)
            {
                coreMask <<= 1;
                coreMask |= 1;
            }
        }

        gtUInt32 coreId = 0;

        while (coreMask)
        {
            if (coreMask & 0x1)
            {
                m_coresList.push_back(coreId);

                coreMask >>= 1;
                coreId++;
            }
        }
    }

    return true;
}

#if AMDT_CPCLI_ENABLE_IMIX
// Report is aggregated by Module
// TODO: Add support for event filtering (option -e)
void CpuProfileReport::ReportImixData()
{
    if (!IsReportImix())
    {
        return;
    }

    gtVector<gtString> sectionHdrs;
    gtUInt64 totalSamples = 0;
    gtUInt64 coreMask = m_args.GetCoreAffinityMask();
    gtUInt64 flags = 0;

    flags = m_args.IsIgnoreSystemModules() ? SAMPLE_IGNORE_SYSTEM_MODULES : 0;
    // Only "group-by-module" is supported now
    flags |= SAMPLE_GROUP_BY_MODULE;

    ProcessIdType pid = static_cast<ProcessIdType>(ALL_PROCESS_IDS);
    ThreadIdType tid = static_cast<ThreadIdType>(ALL_THREAD_IDS);

    ModuleImixInfoList modImixInfoList;

    bool rc = GetImixInfoList(m_profileReader, pid, tid, flags, coreMask, modImixInfoList, totalSamples);

    if (rc)
    {
        ImixSummaryMap imixSummaryMap;
        rc = GetImixSummaryMap(m_profileReader, modImixInfoList, imixSummaryMap, totalSamples);

        if (rc)
        {
            gtString tempStr(L"IMIX SUMMARY REPORT\n");
            tempStr.appendFormattedString(L"Total Samples Taken = %llu\n", totalSamples);
            sectionHdrs.push_back(tempStr);

            tempStr = L"Disassembly";
            tempStr.appendFormattedString(L",Samples Percentage");
            tempStr.appendFormattedString(L",Samples Count\n");
            sectionHdrs.push_back(tempStr);

            m_pReporter->WriteImixSummaryInfo(sectionHdrs, imixSummaryMap, totalSamples);
        }
    }

    if (rc)
    {
        gtString tempStr = L"IMIX MODULE-WISE REPORT\n\n";
        sectionHdrs.clear();
        sectionHdrs.push_back(tempStr);

        m_pReporter->WriteImixInfo(sectionHdrs, modImixInfoList, totalSamples);
    }
}
#endif // AMDT_CPCLI_ENABLE_IMIX
