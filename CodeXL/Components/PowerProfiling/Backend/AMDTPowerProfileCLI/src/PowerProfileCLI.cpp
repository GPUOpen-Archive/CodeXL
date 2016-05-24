//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileCLI.cpp
///
//==================================================================================

// system
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

#ifdef LINUX
    #include <signal.h>
#endif

// infra
#include <AMDTOSWrappers/Include/osThread.h>  // for osSleep()
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osTime.h>

// project
#include <AMDTPowerProfileApi.h>
#include <ppParseArgs.h>
#include <ppCollect.h>
#include <ppReporter.h>

//
//      Macros
//

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define TEMP_PATH    "c:\\Temp\\"
    #define DEFAULT_TEMP_PATH    "%Temp%\\"
    #define CODEXL_POWERPROFILE_CLI    "CodeXLPowerProfiler.exe"
#else
    #define TEMP_PATH    "/tmp/"
    #define DEFAULT_TEMP_PATH   TEMP_PATH
    #define CODEXL_POWERPROFILE_CLI    "CodeXLPowerProfiler"
#endif

#define HISTOGRAM_COUNTER_MAX_CNT (10)

//
//      Globals
//

static ppCollect* g_pCollectHandle = nullptr;
static ppReporter* g_pReporterHandle = nullptr;

#ifdef LINUX
#include <unistd.h>
#include <sys/time.h>
void Sleep(unsigned int msecs)
{
    usleep(msecs * 1000);
}
void SSleep(unsigned int secs)
{
    sleep(secs);
}

AMDTUInt64 GetTickCount(void)
{
    AMDTUInt64 milliSec = 0;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    milliSec = tv.tv_sec * 1000;
    milliSec += (tv.tv_usec / 1000);
    return milliSec;
}

#else
void SSleep(unsigned int secs)
{

    Sleep(secs * 1000);
}
#endif

// PrintVersion()
// helper functions to print the help
static int PrintVersion()
{
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);

    fprintf(stderr, "%s Version %s.\n", CODEXL_POWERPROFILE_CLI, appVersion.toString().asASCIICharArray());
    return 0;
} // PrintVersion

// Print Examples
int PrintExamples()
{
    fprintf(stderr, "\nExamples:\n\n");

    fprintf(stderr, " 1. Collect all the supported counter values for the duration of 10 seconds with sampling interval of 100 milliseconds:\n");
    fprintf(stderr, "     %s -e all -T 100 -d 10\n\n", CODEXL_POWERPROFILE_CLI);

    fprintf(stderr, " 2. Collect all the supported counter values for the duration of 300 seconds with sampling interval of 100 milliseconds"
            "and output the data to a binary file that can be imported into the graphic client:\n");
    fprintf(stderr, "     %s  -P all -C 0x3 -z %s -T 100 -d 300\n\n", CODEXL_POWERPROFILE_CLI, DEFAULT_TEMP_PATH);

    fprintf(stderr, " 3. Collect all power counter values for 10 seconds, sampling them every 500 milliseconds and dumping the results to a csv file:\n");
    fprintf(stderr, "     %s  -P power -o %sPowerOutput -T 500 -d 10\n\n", CODEXL_POWERPROFILE_CLI, DEFAULT_TEMP_PATH);

    fprintf(stderr, " 4. Collect all frequency counter values for 10 seconds, sampling them every 500 milliseconds and dumping the results to a text file:\n");
    fprintf(stderr, "     %s  -P frequency -o %sPowerOutput -T 500 -d 10 -F txt\n\n", CODEXL_POWERPROFILE_CLI, DEFAULT_TEMP_PATH);

    fprintf(stderr, " 5. Collect process power consumption values for 100 seconds, sampling them every 10 milliseconds and dumping the results to a csv file:\n");
    fprintf(stderr, "     %s  -M process -o %sPowerOutput -T 10 -d 100\n\n", CODEXL_POWERPROFILE_CLI, DEFAULT_TEMP_PATH);

    fprintf(stderr, " 6. Display help:\n");
    fprintf(stderr, "     %s -h\n\n", CODEXL_POWERPROFILE_CLI);

    return 0;
}

// PrintHelp
// helper functions to print the help
static int PrintHelp()
{
    fprintf(stderr, "\n%s is a command-line tool for CodeXL's Power Profiler.\n", CODEXL_POWERPROFILE_CLI);
    fprintf(stderr, "\nUsage: %s <Options> [<Application>] [<Application Args>]\n", CODEXL_POWERPROFILE_CLI);

    fprintf(stderr, "\n    -P  <profile options>      Specify profile options. Following options are supported.\n");
    fprintf(stderr, "                                   power - collect all the available power counters\n");
    fprintf(stderr, "                                   temperature - collect all the available temperature counters\n");
    fprintf(stderr, "                                   frequency - collect all the available frequency counters\n");
    fprintf(stderr, "                                   cu_power - collect cpu compute-unit power counters\n");
    fprintf(stderr, "                                   cu_temperature - collect cpu compute-unit temperature counters\n");
    fprintf(stderr, "                                   gpu_power - collect gpu power counters\n");
    fprintf(stderr, "                                   gpu_temperature - collect gpu temperature counters\n");
    fprintf(stderr, "                                   core - collect core specific attributes\n");
#ifdef SVI2_COUNTERS_SUPPORTED
    fprintf(stderr, "                                   SVI2 - collect all SVI2 counters\n");
#endif
    fprintf(stderr, "                                   dvfs - collect all the available dvfs counters\n");
    fprintf(stderr, "                                   all - collect all the supported counters\n");
    fprintf(stderr, "\n    -M  <profile mode>         Specify profile mode. Following profile modes are supported.\n");
    fprintf(stderr, "                                   process - process profiling\n");
#ifndef LINUX
    fprintf(stderr, "                                   module - process profiling\n");
#endif
    fprintf(stderr, "\n    -l                         List all the supported counters.\n");

    fprintf(stderr, "\n    -e <counter,...>           Specify the comma separated list of counter names to be collected.\n");
    fprintf(stderr, "                               Use option (-l) to get the supported counter names.\n");
    fprintf(stderr, "                               Note: use any one of the options -P or -e.\n");

    fprintf(stderr, "\n    -D <counter,...>           Specify the comma separated list of device ids. All the counters\n");
    fprintf(stderr, "                               of these devices will be profiled and collected.\n");
    fprintf(stderr, "                               Use option (-l) to get the supported devices.\n");

    //fprintf(stderr, "\n    -c <core mask>             Specify core mask for 'core' mode in option '-e'\n");
    //fprintf(stderr, "                               Default mask is all the available cores.\n");
#if defined(GDT_INTERNAL) || defined(GDT_NDA)
    fprintf(stderr, "\n    -T <sampling period>       Sampling interval in milli seconds. The minimum value is 1ms.\n");
#else
    fprintf(stderr, "\n    -T <sampling period>       Sampling interval in milli seconds. The minimum value is 10ms.\n");
#endif

    fprintf(stderr, "\n    -d <duration>              Profile duration in seconds.\n");

    //fprintf(stderr, "\n    -O                         Enable offline profile mode. The profile records are stored in\n");
    //fprintf(stderr, "                               raw profile data file. The default mode is online.\n");

    fprintf(stderr, "\n    -o <path>                  Output file\n");
    fprintf(stderr, "                               Default path will be %sCodexl-PowerProfile-<time>.\n", DEFAULT_TEMP_PATH);

    fprintf(stderr, "\n    -F <csv|txt>               Output file format.\n");
    fprintf(stderr, "                               Default file format is csv\n");

    //fprintf(stderr, "\n    -g                         Group the counters by device while reporting.\n");

    fprintf(stderr, "\n    -C <core mask>             Specify core affinity mask for the application to be launched\n");
    fprintf(stderr, "                               Default affinity mask is all the available cores.\n");

    fprintf(stderr, "\n    -b                         Terminate the launched application after the specified profile\n");
    fprintf(stderr, "                               duration.\n");

    fprintf(stderr, "\n    -w                         Specify the working directory.\n");
    fprintf(stderr, "                               Default will be the path of the launched application.\n");

    fprintf(stderr, "\n    -h                         Display this help information.\n");
    fprintf(stderr, "\n    -v                         Print version string.\n");
    fprintf(stderr, "\n    -z <db file output dir>    Export results to a *.cxldb file which can be imported to CodeXL GUI\n");
    fprintf(stderr, "                               application.\n");

    PrintExamples();

    fflush(stderr);
    return 0;
} // PrintHelp

// ReportError
//
bool ReportError(bool appendDriverError, const char* pFormatString, ...)
{
    // Get a pointer to the variable list of arguments:
    va_list argptr;
    va_start(argptr, pFormatString);

    const unsigned int sizeErrBuffer = 512U;
    char errBuffer[sizeErrBuffer];

    // Write the formatted string into the buffer:
    int size = vsnprintf(errBuffer, sizeErrBuffer - 1U, pFormatString, argptr);

    // The buffer was big enough to contains the formatted string:
    if (0 < size)
    {
        // Terminate the string manually:
        errBuffer[size] = L'\0';

        if (appendDriverError && static_cast<unsigned>(size) < sizeErrBuffer)
        {
            // TODO: We need PP API to get the last profile error
            //fnGetLastProfileError(sizeErrBuffer - static_cast<unsigned>(size), &errBuffer[size]);
        }

        pFormatString = errBuffer;
    }

    fprintf(stderr, "%s\n", pFormatString);
    fflush(stderr);

    // Terminate the argptr pointer:
    va_end(argptr);

    return 0 < size;
} // ReportError

// ErrorExit
//
void ErrorExit(const char* errorString, int exitCode)
{
    if (nullptr != errorString)
    {
        ReportError(false, errorString);
    }

    exit(exitCode);
} // ErrorExit

bool ReportSupportedCounterDesc(AMDTPwrCounterIdNameVec& counterIdNameVec, AMDTPwrCounterIdDescVec& counterIdDescVec)
{
    bool retVal = true;

    fprintf(stderr, "\nList of supported counters (to be used in -e):\n");

    // Print the heading
    for (AMDTUInt32 i = 0; i < counterIdNameVec.size(); i++)
    {
        const AMDTPwrCounterDesc* pDesc = counterIdDescVec[i];

        if (nullptr != pDesc)
        {
            gtString desc;
            desc.fromASCIIString(pDesc->m_description);

            int pos = desc.find(L".", 0);
            int endPosition = (-1 != pos) ? (pos - 1) : desc.length();

            gtString shortDesc;
            desc.getSubString(0, endPosition, shortDesc);

            fprintf(stderr, " %3d. %-25.25s        [%s]\n", pDesc->m_counterID,
                    counterIdNameVec[i].asASCIICharArray(), shortDesc.asASCIICharArray());
        }
    }

    return retVal;
} // ReportSupportedCounterDesc

bool ReportDeviceDesc(AMDTPwrDeviceIdNameVec& deviceIdNameVec, AMDTPwrDeviceIdDescVec& deviceIdDescVec)
{
    bool retVal = true;

    fprintf(stderr, "\nList of supported devices (to be used in -D):\n");

    // Print the heading
    for (AMDTUInt32 i = 0; i < deviceIdNameVec.size(); i++)
    {
        const AMDTPwrDevice* pDevice = deviceIdDescVec[i];

        if (nullptr != pDevice)
        {
            AMDTUInt32 numOfSupportedCounters = 0;
            AMDTPwrCounterDesc* pBeSupportedCounters;
            AMDTResult ret = AMDTPwrGetDeviceCounters(pDevice->m_deviceID, &numOfSupportedCounters, &pBeSupportedCounters);

            if ((AMDT_STATUS_OK == ret) && (numOfSupportedCounters > 0))
            {
                fprintf(stderr, " %3d. %-20.20s        [%s]\n", pDevice->m_deviceID,
                        deviceIdNameVec[i].asASCIICharArray(), pDevice->m_pDescription);
            }
        }
    }

    return retVal;
} // ReportDeviceDesc

void ConstructProfileSessionInfo(ppParseArgs& args, AMDTProfileSessionInfo& sessionInfo)
{
    osGetLocalMachineName(sessionInfo.m_targetMachineName);

    sessionInfo.m_sessionDir = args.GetWorkingDir();
    sessionInfo.m_sessionType = L"Power Profiling";
    sessionInfo.m_sessionScope = L"System Wide";
    sessionInfo.m_targetAppCmdLineArgs = args.GetLaunchAppArgs();
    sessionInfo.m_targetAppPath = args.GetLaunchApp();

    // Session start time.
    osTime timing;
    timing.setFromCurrentTime();
    timing.dateAsString(sessionInfo.m_sessionStartTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);

    // Build the DB file name.
    gtString dbFileName = sessionInfo.m_sessionStartTime;
    sessionInfo.m_sessionDbFullPath = args.GetDbFileOutDir().append(osFilePath::osPathSeparator).append(dbFileName);
}

void StopProf()
{
    if (nullptr != g_pCollectHandle)
    {
        g_pCollectHandle->Clear();
    }

    if (nullptr != g_pReporterHandle)
    {
        // close reporter file.
        g_pReporterHandle->Close();
    }
}

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
void ppSighandler(int signum)
{
    StopProf();

    // generate a core file
    signal(signum, SIG_DFL);
    kill(getpid(), signum);

} // ppSighandler
#else
BOOL ppWinSigHandler(DWORD fdwCtrlType)
{
    BOOL result = TRUE;

    switch (fdwCtrlType)
    {
        case CTRL_C_EVENT:
            StopProf();
            ErrorExit(nullptr, -1);
            break;

        default:
            result = FALSE;
            break;
    }

    return result;
}//ppWinSigHandler
#endif

int main(int argc, char* argv[])
{
    AMDTResult hResult = AMDT_STATUS_OK;
    int retVal = 0;

    // Register a signal handler to handle crashes/^C
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    signal(SIGSEGV, ppSighandler);
    signal(SIGINT, ppSighandler);
#else // handling ^C event in Winodws
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ppWinSigHandler, TRUE);
#endif

    // handle the arguments passed.
    ppParseArgs args;
    retVal = args.Initialize(argc, argv);

    if (!retVal)
    {
        ErrorExit(nullptr, -1);
    }

    // Print the Version info
    if (args.IsPrintVersion())
    {
        PrintVersion();
    }

    // Print the Help
    if (args.IsPrintUsage())
    {
        PrintHelp();
    }

    if (args.IsListCounters() || args.IsProfileCounters())
    {
        // Process the collection mode options and initialize the ppCollect object
        ppCollect collect(args);
        g_pCollectHandle = &collect;

        // Enable Profiler
        // Get the supported counters
        // Enable all the counter
        // Set the timer configuration
        hResult = collect.Initialize();

        if (AMDT_STATUS_OK != hResult)
        {
            if (AMDT_ERROR_HYPERVISOR_NOT_SUPPORTED == hResult)
            {
                ErrorExit("Hypervisor detected. Power Profiling is not supported on this guest OS.",
                          AMDT_ERROR_HYPERVISOR_NOT_SUPPORTED);
            }
            else
            {
                ErrorExit(nullptr, -1);
            }
        }

        // List the supported counters -- "-l" option
        if (args.IsListCounters())
        {
            // Report devices
            AMDTPwrDeviceIdNameVec deviceIdNameVec;
            AMDTPwrDeviceIdDescVec deviceIdDescVec;
            collect.GetDeviceIdDescVec(deviceIdDescVec);
            // TODO error handling

            collect.GetDeviceIdNamecVec(deviceIdNameVec);
            // TODO error handling

            ReportDeviceDesc(deviceIdNameVec, deviceIdDescVec);

            // Report counters
            AMDTPwrCounterIdDescVec counterIdDescVec;
            hResult = collect.GetSupportedCounterIdDescVec(counterIdDescVec);

            if (AMDT_STATUS_OK == hResult)
            {
                // print values
                AMDTPwrCounterIdNameVec counterIdNameVec;
                hResult = collect.GetSupportedCounterIdNameVec(counterIdNameVec);
                // TODO handle error

                ReportSupportedCounterDesc(counterIdNameVec, counterIdDescVec);
            }
            else
            {
                // error exit
                ErrorExit("Failed to get the supported counter descriptors", -1);
            }
        }

        // configure the profile run
        if (args.IsProfileCounters())
        {
            // configure the profile run
            hResult = collect.Configure();

            if (AMDT_STATUS_OK != hResult)
            {
                ErrorExit("Failed to configure the power profile run.", -1);
            }

            if ((1 == args.GetProfileType()) || (2 == args.GetProfileType()))
            {
                hResult = collect.EnableProcessProfiling();

                if (AMDT_STATUS_OK != hResult)
                {
                    ErrorExit("Failed to configure the process/module profiling run.", -1);
                }
            }

            // Contruct the DB
            bool isDbExportEnabled = args.ShouldExportToDb();

            if (isDbExportEnabled)
            {
                AMDTProfileSessionInfo sessionInfo;
                ConstructProfileSessionInfo(args, sessionInfo);
                collect.InitDb(sessionInfo);
            }

            // Initialize the reporter
            osFilePath reportFilePath;
            bool isReportEnabled = args.GetReportFilePath(reportFilePath);
            ppReporter* pReporter = nullptr;

            if (isReportEnabled)
            {
                if (PP_REPORT_TYPE_TEXT == args.GetReportType())
                {
                    pReporter = new ppReporterText(args, reportFilePath);
                }
                else if (PP_REPORT_TYPE_CSV == args.GetReportType())
                {
                    pReporter = new ppReporterCsv(args, reportFilePath);
                }

                // set the global report handle for signal handling
                g_pReporterHandle = pReporter;

                if ((nullptr != pReporter) && (!pReporter->IsOpened()))
                {
                    if (pReporter->Open())
                    {
                        pReporter->SetProfileStartTime(collect.GetProfileStartTime().asASCIICharArray());

                        AMDTPwrCounterIdDescVec supportedCounterIdDescVec;
                        collect.GetSupportedCounterIdDescVec(supportedCounterIdDescVec);

                        AMDTPwrCounterIdNameVec supportedCounterIdNameVec;
                        collect.GetSupportedCounterIdNameVec(supportedCounterIdNameVec);

                        gtVector<AMDTUInt32> profiledCounterIdVec;
                        collect.GetProfiledCounterIdVec(profiledCounterIdVec);

                        pReporter->SetCounterDesc(supportedCounterIdDescVec, supportedCounterIdNameVec, profiledCounterIdVec);

                        pReporter->ReportHeader();
                    }
                }
            }

            // Start the launched application
            hResult = collect.LaunchTargetApp();

            // Start profile run
            hResult = collect.StartProfiling();

            if (AMDT_STATUS_OK != hResult)
            {
                // FIXME.. reporter is not cleaned up
                ErrorExit(nullptr, -1);
            }

            // Resume launched application
            if (collect.IsAppLaunched())
            {
                collect.ResumeLaunchedApp();
            }

            // Collect and report the counter values
            //
            // Periodically Read the counter values and Report them
            int samplingInterval = args.GetSamplingInterval();
            int sleepDuration = samplingInterval;
            AMDTUInt32 profileDuration = (args.GetProfileDuration() * 1000);
            int sleepCount = (profileDuration > 0) ? (profileDuration / sleepDuration) : 0;
            bool stopProfile = false;

            if (collect.IsProfilingSimpleCounters())
            {
                AMDTUInt32  nbrSamples = 0;
                AMDTPwrSample* pSampleData = nullptr;

                // testing purpose
                bool simulateProfileStopped = false;
                bool simulateProfilePaused = false;

                while (!stopProfile)
                {
#if 0
                    double x = rand() / static_cast<double>(RAND_MAX + 1);
                    // [0,1[ * (max - min) + min is in [min,max]
                    double multiplier = 0.5 + (x * (3.5));
                    double n_sleepDuration = sleepDuration * multiplier;
                    Sleep((int)n_sleepDuration);
#else
                    Sleep(sleepDuration);
#endif

                    if (args.IsSimulateGUI())
                    {
                        if (simulateProfileStopped)
                        {
                            fprintf(stderr, "restart profiling..\n");
                            collect.StartProfiling();
                            simulateProfileStopped = false;
                        }
                        else if (!simulateProfilePaused && (0 == sleepCount % 11))
                        {
                            fprintf(stderr, "stop profiling..\n");
                            collect.StopProfiling();
                            simulateProfileStopped = true;
                        }

                        if (simulateProfilePaused)
                        {
                            fprintf(stderr, "resume profiling..\n");
                            collect.ResumeProfiling();
                            simulateProfilePaused = false;
                        }
                        else if (!simulateProfileStopped && (0 == sleepCount % 23))
                        {
                            fprintf(stderr, "pause profiling..\n");
                            collect.PauseProfiling();
                            simulateProfilePaused = true;
                        }
                    } // simulate GUI

                    nbrSamples = 0;
                    pSampleData = nullptr;
                    hResult = collect.GetSampleData(&nbrSamples, &pSampleData);

                    //printf(" Got %d values\n", nbrSamples);
                    if (AMDT_ERROR_SMU_ACCESS_FAILED == hResult)
                    {
                        // One or all SMU access error. We should stop the profile.
                        fprintf(stderr, "SMU access error for one of the required device.\n");
                        stopProfile = true;
                        continue;
                    }

                    if ((AMDT_STATUS_OK == hResult) && (nbrSamples > 0) && (nullptr != pSampleData))
                    {
                        if (isDbExportEnabled)
                        {
                            collect.WriteSamplesIntoDb();
                        }

                        if (nullptr != pReporter)
                        {
                            pReporter->ReportProfiledCounterData(nbrSamples, pSampleData);
                        }

                        // check if we exceeded the profile duration
                        if ((profileDuration > 0)
                            && (pSampleData[nbrSamples - 1].m_elapsedTimeMs >= profileDuration))
                        {
                            stopProfile = true;
                        }
                    }

                    //else
                    //{
                    //    fprintf(stderr, "Error while reading sampled counters. errorcode(0x%x).\n", hResult);
                    //}

                    // the profile is stopped either if the duration expires or the launched
                    // application is terminated
                    if (collect.IsAppLaunched())
                    {
                        stopProfile = collect.IsLaunchedAppAlive() ? false : true;
                    }

                    if ((!stopProfile) && profileDuration && (--sleepCount == 0))
                    {
                        stopProfile = true;
                    }
                } // while

                // If there are any missing reads report a warning
                AMDTUInt32 expectedSamples = (AMDTUInt32)(collect.GetTotalElapsedTime() / samplingInterval);

                if (collect.GetNumberOfFailedReads()
                    && collect.GetTotalNumberOfSamples() < expectedSamples)
                {
                    fprintf(stderr, "WARNING: Number of missing records: (%d).\n",
                            (expectedSamples - collect.GetTotalNumberOfSamples()));
                }
            }
            else if (collect.IsProfilingCumulativeCounters()
                     || collect.IsProfilingHistogramCounters()
                     || (1 == args.GetProfileType())
                     || (2 == args.GetProfileType()))
            {
                AMDTUInt64 startTs = GetTickCount();

                // Only the cumulative counters are enabled
                while (!stopProfile)
                {
                    Sleep(sleepDuration);

                    if (collect.IsAppLaunched())
                    {
                        stopProfile = collect.IsLaunchedAppAlive() ? false : true;
                    }

                    if ((!stopProfile)
                        && ((profileDuration > 0) && ((--sleepCount == 0) ||
                                                      (GetTickCount() - startTs) > profileDuration)))
                    {
                        stopProfile = true;
                    }
                }
            }

            // stop profiling
            hResult = collect.StopProfiling();

            // if requested, terminate the launched app
            if (args.IsTerminateApp() && collect.IsLaunchedAppAlive())
            {
                collect.TerminateLaunchedApp();
            }

            // Now read cumulative counters
            if ((AMDT_STATUS_OK == hResult) && collect.IsProfilingCumulativeCounters() &&
                nullptr != pReporter)
            {
                AMDTFloat32 hist[HISTOGRAM_COUNTER_MAX_CNT] = { 0, };
                AMDTUInt32 histCounter[HISTOGRAM_COUNTER_MAX_CNT] = { 0, };
                AMDTUInt32 numOfHist = 0;

                // Read the cumulative counters and report..
                hResult = collect.GetCumulativeCounters(&numOfHist, hist, histCounter);

                if ((AMDT_STATUS_OK == hResult) && (numOfHist > 0))
                {
                    pReporter->ReportCumulativeCounters(numOfHist, hist, histCounter);
                }
            }

            // Now read histogram counters
            if ((AMDT_STATUS_OK == hResult) && collect.IsProfilingHistogramCounters() &&
                nullptr != pReporter)
            {
                AMDTPwrHistogram hist[HISTOGRAM_COUNTER_MAX_CNT];
                AMDTUInt32 numOfHist1 = 0;

                // Read the histograms and report
                memset(hist, 0, sizeof(AMDTPwrHistogram) * HISTOGRAM_COUNTER_MAX_CNT);
                hResult = collect.GetHistogramCounters(&numOfHist1, hist);

                if ((AMDT_STATUS_OK == hResult) && (numOfHist1 > 0))
                {
                    pReporter->ReportHistogramCounters(numOfHist1, hist);

                    for (AMDTUInt32 i = 0; i < numOfHist1; i++)
                    {
                        // free the contents of the histogram pointers
                        free(hist[i].m_pRange);
                        free(hist[i].m_pBins);
                    }
                }
            }

            // print process profiling data now
            if (1 ==  args.GetProfileType() && (nullptr != pReporter))
            {
                AMDTUInt32 pidCnt = 0;
                AMDTPwrProcessInfo* pProcessData = nullptr;

                //Get the aggregated process data
                hResult = collect.GetProcessData(&pidCnt, &pProcessData);
                pReporter->WriteProcessData(pidCnt, pProcessData);
            }
            else if (2 ==  args.GetProfileType() && (nullptr != pReporter))
            {
                AMDTUInt32 moduleCnt = 0;
                AMDTFloat32 power = 0;
                AMDTPwrModuleData* pModuleData = nullptr;

                //Get the aggregated process data
                hResult = collect.GetModuleData(&pModuleData, &moduleCnt, &power);
                pReporter->WriteModuleData(moduleCnt, pModuleData, power);
            }

            if (AMDT_STATUS_OK == hResult)
            {
                collect.Clear();
            }

            // Clear
            if (nullptr != pReporter)
            {
                pReporter->Close();
                delete pReporter;
                pReporter = nullptr;
            }
        }
    }

    return hResult;
} // main
