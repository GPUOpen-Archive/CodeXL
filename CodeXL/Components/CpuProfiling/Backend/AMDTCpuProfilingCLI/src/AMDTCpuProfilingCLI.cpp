//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTCpuProfilingCLI.cpp
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#ifdef _WIN32
    #include <tchar.h>
#endif

// Backend:
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

// Infra:
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osThread.h>  // for osSleep()
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Project:
#include <AMDTCpuProfilingCLI.h>
#include <Collect.h>
#include <Report.h>

//
// Macros
//

// JVMTI Agent library name for Java Profiling
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define JVMTIAGENT32_NAME  L"CXLJvmtiAgent"        GDT_DEBUG_SUFFIX_W  GDT_BUILD_SUFFIX_W
    #define JVMTIAGENT64_NAME  L"CXLJvmtiAgent-x64"    GDT_DEBUG_SUFFIX_W  GDT_BUILD_SUFFIX_W
#else
    #ifdef NDEBUG
        #define JVMTIAGENT32_NAME  L"libCXLJvmtiAgent"
        #define JVMTIAGENT64_NAME  L"libCXLJvmtiAgent-x64"
    #else
        #define JVMTIAGENT32_NAME  L"libCXLJvmtiAgent-d"
        #define JVMTIAGENT64_NAME  L"libCXLJvmtiAgent-x64-d"
    #endif
#endif

// CLR Profiling
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <ProfilingAgents/AMDTClrProfAgent/inc/ClrProfAgent.h>
#endif // AMDT_WINDOWS_OS

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define TEMP_PATH    "c:\\Temp\\"
    #define DEFAULT_TEMP_PATH    "%Temp%\\"

    #define U_FORMAT "%llu"
    #define D_FORMAT "%lld"
    #define H_FORMAT "%llx"
#else
    #define TEMP_PATH    "/tmp/"
    #define DEFAULT_TEMP_PATH   TEMP_PATH

    #define U_FORMAT "%lu"
    #define D_FORMAT "%ld"
    #define H_FORMAT "%lx"
#endif

// Helper functions
extern bool reportError(bool appendDriverError, const wchar_t* pFormatString, ...);

static int PrintVersion();
static int PrintHelp();

static bool SetupJavaProfiling(gtString&  launchApp,
                               gtString&  javaAgentArg,
                               bool       is64Bit);
static bool SetupCLRProfiling(gtList<osEnvironmentVariable>& envVars);
static bool RestoreEnvironment(gtList<osEnvironmentVariable>& envVars);

static HRESULT HandleCollectCommand(ParseArgs& args);
static HRESULT HandleTranslateCommand(ParseArgs& args);
static HRESULT HandleReportCommand(ParseArgs& args);

// Helper Functions

int PrintVersion()
{
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);

    fprintf(stderr, "%s Version %s.\n", CODEXL_CPUPROFILER_CLI, appVersion.toString().asASCIICharArray());
    return 0;
}

int PrintCollectOptions()
{
    fprintf(stderr, "\nCollect Options:\n");
    fprintf(stderr, "    -m <profile type>          Predefined profile type to be used to collect samples.\n");
    fprintf(stderr, "                               Supported profile types are:-\n");
    fprintf(stderr, "                                   tbp         - Time-based Sampling\n");
    fprintf(stderr, "                                   assess      - Assess Performance\n");
    fprintf(stderr, "                                   branch      - Investigate Branching\n");
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    fprintf(stderr, "                                   clu         - Cache Line Utilization\n");
#endif
    fprintf(stderr, "                                   data_access - Investigate Data Access\n");
    fprintf(stderr, "                                   ibs         - Instruction-based Sampling\n");
    fprintf(stderr, "                                   inst_access - Investigate Instruction Access\n");
    fprintf(stderr, "                                   l2_access   - Investigate L2 Cache Access\n");

    fprintf(stderr, "\n    -T <n>                     Custom Time based profiling.\n");
    fprintf(stderr, "                               (sampling interval n in milli-seconds).\n");

    fprintf(stderr, "\n    -d <n>                     Profile duration in seconds.\n");

    fprintf(stderr, "\n    -o <file name>             Base name of the output file.\n");
    fprintf(stderr, "                               Default path will be %sCodexl-CpuProfile-<time>.\n", DEFAULT_TEMP_PATH);

    fprintf(stderr, "\n    -p <PID,PID,..>            Profile existing processes (processes to attach to).\n");
    fprintf(stderr, "                               Process IDs are separated by comma.\n");

    fprintf(stderr, "\n    -a                         System Wide Profile (SWP).\n");
    fprintf(stderr, "                               Otherwise, profile only the launched application or the\n");
    fprintf(stderr, "                               PIDs attached.\n");

    fprintf(stderr, "\n    -G                         Enable Callstack sampling with default Unwind Interval\n");
    fprintf(stderr, "                               and Unwind Depth. The default values are:\n");
    fprintf(stderr, "                                   Unwind Interval - %d\n", CP_CSS_DEFAULT_UNWIND_INTERVAL);
    fprintf(stderr, "                                   Unwind Depth    - %d\n", CP_CSS_DEFAULT_UNWIND_DEPTH);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    fprintf(stderr, "                                   Scope           - User\n");
    fprintf(stderr, "                                   Collect missing frames due to Frame Pointer omission - No\n");
    fprintf(stderr, "\n    -g <Interval:Depth:Scope:Fpo>\n");
    fprintf(stderr, "                                 Enable Callstack Sampling.\n");
    fprintf(stderr, "                               Specify the Unwind Interval and Unwind Depth values.\n");
    fprintf(stderr, "                               Scope should contain one of these options:\n");
    fprintf(stderr, "                                   user   - Collect Callstack only for code executed in user space.\n");
    fprintf(stderr, "                                   kernel - Collect Callstack only for code executed in kernel space.\n");
    fprintf(stderr, "                                   all    - Collect Callstack for code executed in user space and kernel space.\n");
    fprintf(stderr, "                               Specify to collect missing frames due to omission of frame pointers by compiler.\n");
    fprintf(stderr, "                                   fpo    - Collect missing callstack frames.\n");
    fprintf(stderr, "                                   nofpo  - Do not collect missing callstack frames.\n");
#else
    fprintf(stderr, "\n    -g <Interval:Depth>        Enable Callstack Sampling.\n");
    fprintf(stderr, "                               Specify the Unwind Interval and Unwind Depth values.\n");
#endif

    fprintf(stderr, "\n    -c                         Core Affinity Mask.\n");
    fprintf(stderr, "                               Default affinity is all the available cores.\n");
    fprintf(stderr, "                               In System-wide profiling, samples are collected only\n");
    fprintf(stderr, "                               from these cores. In Per-Process profile, processor\n");
    fprintf(stderr, "                               affinity is set for the launched application.\n");
    fprintf(stderr, "\n    -f                         Profile the children of the launched application.\n");
    fprintf(stderr, "\n    -b                         Terminate the launched application after profile\n");
    fprintf(stderr, "                               collection.\n");
    fprintf(stderr, "\n    -s <n>                     Start Delay (n in seconds).\n");
    fprintf(stderr, "                               Start profiling after the specified duration.\n");
    fprintf(stderr, "                               If n is 0, then wait indefinitely, used for profile control.\n");
    fprintf(stderr, "\n    -v                         Print version string.\n");
    fprintf(stderr, "\n    -w                         Specify the working directory.\n");
    fprintf(stderr, "                               Default will be the path of the launched application.\n");
    fprintf(stderr, "\n    -h                         Displays this help information.\n");
    fprintf(stderr, "\n    -C <Custom profile>        Path to the custom profile XML file.\n");

    fprintf(stderr, "\n    -L <n>                     Specify debug log messaging level. Valid values are 1 to 3.\n");
    fprintf(stderr, "                                   1 - INFO, 2 - DEBUG, 3 - EXTENSIVE\n");

    return 0;
}

int PrintReportOptions()
{
    // REPORT Options
    fprintf(stderr, "\nReport Options:");

    fprintf(stderr, "\n    -i <file name>             Input file name.\n");
    fprintf(stderr, "                               Either the raw profile data file (%s) or the \n", CPUPROFILE_RAWFILE_EXT_STR);
    fprintf(stderr, "                               processed data file (.ebp or .cxlcpdb) can be specified.\n");

    fprintf(stderr, "\n    -o <output dir>            Output dir in which .cxlcpdb file will be created.\n");
    fprintf(stderr, "                               Default path will be %s<base-name-of-input-file>.\n", DEFAULT_TEMP_PATH);

    // TBD: Currently only CSV file format is supported
    // fprintf(stderr, "\n    -F <csv|xml|text>          Output file format.\n");
    // fprintf(stderr, "                               Default file format is CSV.\n");

    fprintf(stderr, "\n    -V <view xml>              Specify the View Config XML file.\n");
    fprintf(stderr, "                               All the raw data will be reported.\n");

    fprintf(stderr, "\n    -R <section,..>            Specify the report sections to be generated.\n");
    fprintf(stderr, "                               Supported Report sections are:\n");
    fprintf(stderr, "                                   all       - Report all the sections (except imix).\n");
    fprintf(stderr, "                                   overview  - Report Overview section.\n");
    fprintf(stderr, "                                   process   - Report process details.\n");
    fprintf(stderr, "                                   module    - Report module details.\n");
    fprintf(stderr, "                                   callgraph - Report callgraph.\n");
    fprintf(stderr, "                                   imix      - Report imix details.\n");
    fprintf(stderr, "                               process and module together are not allowed.\n");
    fprintf(stderr, "                               module and callgraph together are not allowed.\n");

    fprintf(stderr, "\n    -e                         Specify the event index for which callgraph will be\n");
    fprintf(stderr, "                               generated. This event is also used to find the hot\n");
    fprintf(stderr, "                               functions in the Overview section.\n");

    fprintf(stderr, "\n    -I                         Ignore samples from System Modules.\n");
    fprintf(stderr, "\n    -P                         Show Percentage.\n");

    fprintf(stderr, "\n    -D <path1;path2..>         Debug Symbol paths.\n");
    fprintf(stderr, "\n    -S <path1;path2..>         Symbol Server directories.\n");
    fprintf(stderr, "\n    -X <path>                  Path to store the symbols downloaded from");
    fprintf(stderr, "\n                               the Symbol Servers.\n");
#if 0
    fprintf(stderr, "\n    -O                        Report by separate cores.\n");
    fprintf(stderr, "\n    -N                        Report by NUMA.\n");

    fprintf(stderr, "\n    -c <core mask>            Core Mask for which samples to be reported.\n");
    fprintf(stderr, "                              By default report samples collected from all the cores.\n");

    fprintf(stderr, "\n    -L <n1,n2,n3>             Cutoff to limit the process/modules/functions reported.\n");
    fprintf(stderr, "                              Percent-cutoff, Cumulative-cutoff, Minimum-count.\n");
#endif //0

    fprintf(stderr, "\n    -L <n>                     Specify debug log messaging level. Valid values are 1 to 3.\n");
    fprintf(stderr, "                                   1 - INFO, 2 - DEBUG, 3 - EXTENSIVE\n");

    return 0;
}

int PrintExamples()
{
    fprintf(stderr, "\nExamples:\n");

    fprintf(stderr, "  1. Print help:\n");
    fprintf(stderr, "     %s -h\n\n", CODEXL_CPUPROFILER_CLI);

    fprintf(stderr, "  2. Print version:\n");
    fprintf(stderr, "     %s -v\n\n", CODEXL_CPUPROFILER_CLI);

    fprintf(stderr, "  3. Launch Classic.exe and collect 'TBP' samples:\n");
    fprintf(stderr, "     %s collect -m tbp -o %scpuprof-tbp %s\n\n", CODEXL_CPUPROFILER_CLI, TEMP_PATH, CLASSIC_EXAMPLE);

    fprintf(stderr, "  4. Launch Classic.exe and do 'Assess Performance' profile for 10 seconds:\n");
    fprintf(stderr, "     %s collect -m assess -o %scpuprof-assess -d 10 %s\n\n", CODEXL_CPUPROFILER_CLI, TEMP_PATH, CLASSIC_EXAMPLE);

    fprintf(stderr, "  5. Launch Classic.exe and collect 'IBS' samples in SWP mode:\n");
    fprintf(stderr, "     %s collect -m ibs -a -o %scpuprof-ibs-swp %s\n\n", CODEXL_CPUPROFILER_CLI, TEMP_PATH, CLASSIC_EXAMPLE);

    fprintf(stderr, "  6. Collect 'TBP' samples in SWP mode for 10 seconds:\n");
    fprintf(stderr, "     %s collect -m tbp -a -o %scpuprof-TBP-swp -d 10\n\n", CODEXL_CPUPROFILER_CLI, TEMP_PATH);

    fprintf(stderr, "  7. Launch Classic.exe and collect 'TBP' with Callstack sampling:\n");
    fprintf(stderr, "     %s collect -m tbp -G -o %scpuprof-tbp %s\n\n", CODEXL_CPUPROFILER_CLI, TEMP_PATH, CLASSIC_EXAMPLE);

    fprintf(stderr, "  8. Generate report from the raw datafile:\n");
    fprintf(stderr, "     %s report -i %scpuprof-tbp.%s -o %scpuprof-tbp-out\n\n", CODEXL_CPUPROFILER_CLI,
            TEMP_PATH, CPUPROFILE_RAWFILE_EXT_STR, TEMP_PATH);

    fprintf(stderr, "  9. Generate IMIX report from the raw datafile:\n");
    fprintf(stderr, "     %s report -R imix -i %scpuprof-tbp.%s -o %scpuprof-tbp-out\n\n", CODEXL_CPUPROFILER_CLI, TEMP_PATH, CPUPROFILE_RAWFILE_EXT_STR, TEMP_PATH);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    fprintf(stderr, "  10. Generate report with Symbol Server paths:\n");
    fprintf(stderr, "      %s report -D C:\\AppSymbols;C:\\DriverSymbols -S http://msdl.microsoft.com/download/symbols -X C:\\symbols -i %scpuprof-tbp.%s -o %scpuprof-tbp-out\n\n",
            CODEXL_CPUPROFILER_CLI, TEMP_PATH, CPUPROFILE_RAWFILE_EXT_STR, TEMP_PATH);
#endif

    return 0;
}

int PrintHelp()
{
    fprintf(stderr, "\n%s is a command-line tool for CodeXL's CPU Profiler.\n", CODEXL_CPUPROFILER_CLI);

    fprintf(stderr, "\nUsage: %s COMMAND [<Options>] <PROGRAM> [<ARGS>]\n", CODEXL_CPUPROFILER_CLI);

    fprintf(stderr, "\nCOMMAND - Following CPU Profile commands are supported:\n");
    fprintf(stderr, "  collect       Run the given launch application and collect the\n");
    fprintf(stderr, "                cpu profile samples.\n\n");
    fprintf(stderr, "  report        Process the given profile-data file and generates\n");
    fprintf(stderr, "                a cpu profile report.\n\n");

    fprintf(stderr, "\nPROGRAM\n");
    fprintf(stderr, "  The launch application to be profiled and its arguments.\n");

    PrintCollectOptions();

    PrintReportOptions();

    PrintExamples();

    fflush(stderr);
    return 0;
}

// Setup the environment for profiling Java applications
bool SetupJavaProfiling(
    gtString&  launchApp,
    gtString&  javaAgentArg,
    bool       is64Bit
)
{
    // Check if the launched application is java
    if (launchApp.endsWith(L"java.exe") || launchApp.endsWith(L"java"))
    {
        // Get the current application path
        osFilePath jvmtiAgentPath;

        if (osGetCurrentApplicationDllsPath(jvmtiAgentPath) || osGetCurrentApplicationPath(jvmtiAgentPath))
        {
            // Set the agentpath stuff
            jvmtiAgentPath.setFileName(is64Bit ? JVMTIAGENT64_NAME : JVMTIAGENT32_NAME);
            jvmtiAgentPath.setFileExtension(OS_MODULE_EXTENSION);

            if (jvmtiAgentPath.exists())
            {
                javaAgentArg = L" -agentpath:";

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                javaAgentArg += L'\"';
#endif
                javaAgentArg += jvmtiAgentPath.asString();
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                javaAgentArg += L'\"';
#endif
                javaAgentArg += L' ';
            }
        }
    }

    return true;
}

bool SetupCLRProfiling(gtList<osEnvironmentVariable>& envVars)
{
    bool retVal = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Set Cor_Enable_Profiling
    osEnvironmentVariable corEnProf;
    corEnProf._name = gtString(L"Cor_Enable_Profiling");
    osGetCurrentProcessEnvVariableValue(corEnProf._name, corEnProf._value);
    envVars.push_back(corEnProf);
    osEnvironmentVariable setCorEnProf;
    setCorEnProf._name = corEnProf._name;
    setCorEnProf._value = gtString(L"0x01");
    osSetCurrentProcessEnvVariable(setCorEnProf);

    // set COMPLUS_ProfAPI_ProfilerCompatibilitySetting
    osEnvironmentVariable profCompSet;
    profCompSet._name = gtString(L"COMPLUS_ProfAPI_ProfilerCompatibilitySetting");
    osGetCurrentProcessEnvVariableValue(profCompSet._name, profCompSet._value);
    envVars.push_back(profCompSet);
    osEnvironmentVariable setProfCompSet;
    setProfCompSet._name = profCompSet._name;
    setProfCompSet._value = gtString(L"EnableV2Profiler");
    osSetCurrentProcessEnvVariable(setProfCompSet);

    // set COR_PROFILER
    osEnvironmentVariable corProfiler;
    corProfiler._name = gtString(L"COR_PROFILER");
    osGetCurrentProcessEnvVariableValue(corProfiler._name, corProfiler._value);
    envVars.push_back(corProfiler);
    osEnvironmentVariable setCorProfiler;
    setCorProfiler._name = corProfiler._name;
    setCorProfiler._value = L"{" _T(CLSID_AMDTClrProfAgent) L"}";
    osSetCurrentProcessEnvVariable(setCorProfiler);

    retVal = true;
#else
    GT_UNREFERENCED_PARAMETER(envVars);

    retVal = false;
#endif // AMDT_WINDOWS_OS

    return retVal;
}

bool RestoreEnvironment(gtList<osEnvironmentVariable>& envVars)
{
    // Restore the environmental variables set for CLR profiling
    for (gtList<osEnvironmentVariable>::const_iterator it = envVars.begin(), itEnd = envVars.end(); it != itEnd; ++it)
    {
        // Set the current environment value in this process environment block:
        osSetCurrentProcessEnvVariable(*it);
    }

    return true;
}

HRESULT LaunchTargetApp(
    ParseArgs&        args,
    osProcessId&      processId,
    osProcessHandle&  processHandle,
    osThreadHandle&   processThreadHandle,
    bool&             isAppLaunched
)
{
    HRESULT hr = S_OK;
    bool createWindow = true;
    bool redirectFiles = true;
    bool appLaunched = false;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    createWindow = false;
#endif // AMDT_BUILD_TARGET

    if (! args.GetLaunchApp().isEmpty())
    {
        // Launch the target application, if any
        osFilePath exePath(args.GetLaunchApp());

        ExecutableFile* pExec = ExecutableFile::Open(args.GetLaunchApp().asCharArray());
        bool is64Bit = false;

        if (nullptr != pExec)
        {
            is64Bit = pExec->Is64Bit();
            // Delete the Executable object
            delete pExec;
        }

        // Setup the environment for profiling Java applications
        gtString javaAgentArg;
        SetupJavaProfiling(args.GetLaunchApp(),
                           javaAgentArg,
                           is64Bit);

        // Set up the arguments for the launched app
        gtString LaunchAppArgs = javaAgentArg;
        LaunchAppArgs += args.GetLaunchAppArgs();

        // Set the working directory for the launched process
        gtString workDir = args.GetWorkingDir();

        if (workDir.isEmpty())
        {
            if (! exePath.fileDirectoryAsString().startsWith(L"."))
            {
                workDir = exePath.fileDirectoryAsString();
            }
            else
            {
                osFilePath currentWorkDir;
                currentWorkDir.setPath(osFilePath::OS_CURRENT_DIRECTORY);
                workDir = currentWorkDir.fileDirectoryAsString();
            }
        }

        // Set the environment variables for CLR apps
        gtList<osEnvironmentVariable> clrEnvVars;
        SetupCLRProfiling(clrEnvVars);

        appLaunched = osLaunchSuspendedProcess(exePath,
                                               LaunchAppArgs,
                                               workDir,
                                               processId,
                                               processHandle,
                                               processThreadHandle,
                                               createWindow,
                                               redirectFiles);

        if (appLaunched)
        {
            // Set processor affinity
            osSetProcessAffinityMask(processId, processHandle, args.GetCoreAffinityMask());

            // Restore the environment vars...
            RestoreEnvironment(clrEnvVars);
        }
        else
        {
            reportError(false, L"Could not launch the target: %ls", args.GetLaunchApp().asCharArray());
            hr = E_FAIL;
        }
    }

    isAppLaunched = appLaunched;
    return hr;
}

HRESULT HandleCollectCommand(ParseArgs& args)
{
    HRESULT hr = E_FAIL;

    CpuProfileCollect collect(args);

    // Validate the collect options and initialize the internal fields
    hr = collect.Initialize();

    if (S_OK == hr)
    {
        osProcessId processId;
        osProcessHandle processHandle;
        osThreadHandle processThreadHandle;
        bool  appLaunched = false;

        // Launch the target application (if any) provided by the user
        hr = LaunchTargetApp(args,
                             processId,
                             processHandle,
                             processThreadHandle,
                             appLaunched);

        if (S_OK == hr)
        {
            // Add the launched process Id to CpuProfileCollect
            if (appLaunched)
            {
                collect.SetLaunchedPid(static_cast<unsigned int>(processId));
            }

            // Start the profiling
            hr = collect.StartProfiling();
        }

        bool terminateLaunchedApp = true;

        if (S_OK == hr)
        {
            if (appLaunched)
            {
                // if start profiling fails, we need to terminate the launched app
                // Resume the suspended process
                osResumeSuspendedProcess(processId, processHandle, processThreadHandle, true);
                terminateLaunchedApp = false;
            }

            // Check if this is a delayed start
            // if delay > 0, then profiling gets resumed after the specified delay
            // if delay == 0, then profiling paused indefinitely, target application resumes the profiling using control APIs
            // if delay < 0, then delay value is not provided by user
            if (0 < args.GetStartDelay())
            {
                // Sleep for that much duration..
                osSleep(args.GetStartDelay() * 1000);

                bool resumeProfile = false;
                osProcessId targetPid = (appLaunched) ? processId : (! args.GetPidsList().empty()) ? args.GetPidsList().at(0) : -1;

                if (-1 != targetPid)
                {
                    // Launch app specified or Attach PIDs;
                    // On Attach PIDs scenario, consider only the first PID
                    osIsProcessAlive(targetPid, resumeProfile);
                }
                else
                {
                    // typical SWP without Launch App or Attach PIDs
                    resumeProfile = (0 < (args.GetProfileDuration() - args.GetStartDelay())) ? true : false;
                }

                if (resumeProfile)
                {
                    fnResumeProfiling(nullptr);
                }
            } // Start Delay

            // Wait for the profile duration to expire or wait till the launched app expires
            int profileDuration = 0;

            if (0 < args.GetStartDelay())
            {
                profileDuration = args.GetProfileDuration() - args.GetStartDelay();
            }
            else
            {
                profileDuration = args.GetProfileDuration();
            }

            int sleepCount = (profileDuration > 0) ? profileDuration : 0;
            bool stopProfile = false;

            if (args.GetProfileDuration() > 0)
            {
                stopProfile = (sleepCount > 0) ? false : true;
            }

            gtUInt32 sleepDuration = (appLaunched && (!args.IsAttach())) ? 500 : 1000;

            while (! stopProfile)
            {
                // osProcessId targetPid = (appLaunched) ? processId : (! args.GetPidsList().empty()) ? args.GetPidsList().at(0) : -1;

                // In attach case, profile till the specified profile-duration, even if the specified
                // attached process is terminated
                if (appLaunched && (!args.IsAttach()))
                {
                    // This is not a attach case and also we have a launch program
                    stopProfile = osWaitForProcessToTerminate(processId, sleepDuration, nullptr, appLaunched);
                }

                if (! stopProfile && (sleepCount-- > 0))
                {
                    osSleep(sleepDuration);

                    if (0 == sleepCount)
                    {
                        stopProfile = true;
                    }
                }
            }

            // Stop profiling
            hr = collect.StopProfiling();
        }

        if (appLaunched && (terminateLaunchedApp || args.IsTerminateApp()))
        {
            // Terminate the launched process if the user has requested or StartProfiling fails
            osTerminateProcess(processId);
        }

        // Print the count mode event values
        if (collect.hasCountModeEvents())
        {
            gtVector<CpuProfilePmcEventCount> countValues;
            collect.GetCountEventValues(countValues);
            
            fprintf(stderr, "\nCPCLI>>> Core     Event       Count-Value\n");

            for (const auto& aValue : countValues)
            {
                for (gtUInt32 i = 0; i < aValue.m_nbrEvents; i++)
                {
                    fprintf(stderr, "CPCLI>>> %d     0x" H_FORMAT "      " U_FORMAT"\n", aValue.m_coreId, aValue.m_eventConfig[i], aValue.m_eventCountValue[i]);
                }
            }
        }
    }

    return hr;
}

HRESULT HandleTranslateCommand(ParseArgs& args)
{
    HRESULT hr = E_FAIL;

    CpuProfileReport report(args);

    // Validate the report options and initialize the internal fields
    hr = report.Initialize();

    // If raw data-file, translate
    if (S_OK == hr)
    {
        hr = report.Translate();
    }

    return hr;
}

HRESULT HandleReportCommand(ParseArgs& args)
{
    HRESULT hr = E_FAIL;

    CpuProfileReport report(args);

    // Validate the report options and initialize the internal fields
    hr = report.Initialize();

    if (report.IsTpInputFile())
    {
        report.ReportTP();
    }
    else
    {
        // If CAPERF/PRD raw data-file , then translate.
        if (S_OK == hr)
        {
            hr = report.Translate();
        }

        // If EBP/IMD file, then migrate to DB format.
        if (S_OK == hr)
        {
            hr = report.Migrate();
        }

        // Read CXLDB file and generate Report.
        if (S_OK == hr)
        {
            hr = report.Report();
        }
    }

    return hr;
}

bool reportError(bool appendDriverError, const wchar_t* pFormatString, ...)
{
    // Get a pointer to the variable list of arguments:
    va_list argptr;
    va_start(argptr, pFormatString);

    const unsigned int sizeErrBuffer = 512U;
    wchar_t errBuffer[sizeErrBuffer];

    // Write the formatted string into the buffer:
    int size = vswprintf(errBuffer, sizeErrBuffer - 1U, pFormatString, argptr);

    // The buffer was big enough to contains the formatted string:
    if (0 < size)
    {
        // Terminate the string manually:
        errBuffer[size] = L'\0';

        if (appendDriverError && static_cast<unsigned>(size) < sizeErrBuffer)
        {
            fnGetLastProfileError(sizeErrBuffer - static_cast<unsigned>(size), &errBuffer[size]);
        }

        pFormatString = errBuffer;
    }

    OS_OUTPUT_DEBUG_LOG(pFormatString, OS_DEBUG_LOG_ERROR);
    fprintf(stderr, "%S\n", pFormatString);

    // Terminate the argptr pointer:
    va_end(argptr);

    return 0 < size;
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    int _tmain(int argc, _TCHAR* argv[])
#else
    int main(int argc, char* argv[])
#endif
{
    int retVal = 0;
    bool helpPrinted = false;

    gtString debugLogFile(L"CodeXLCpuProfiler");
    osDebugLog::instance().initialize(debugLogFile, NULL, NULL);

    ParseArgs args;
    retVal = args.Initialize(argc, argv);

    // if there are no args..
    if (! retVal)
    {
        exit(-1);
    }

    osDebugLog::instance().setLoggedSeverity(static_cast<osDebugLogSeverity>(args.GetDebugLogLevel()));

    // Print the Version info
    if (args.IsPrintVersion())
    {
        PrintVersion();
    }

    // Print the Help
    if (args.IsPrintUsage())
    {
        helpPrinted = true;
        PrintHelp();
    }

    // If the user has only requested help/version quit
    CpuProfileCommand commandType = CPUPROF_COMMAND_UNDEFINED;
    gtString command = args.GetCommand();

    if (command.isEmpty())
    {
        commandType = CPUPROF_COMMAND_UNDEFINED;
    }
    else if (command.isEqual("collect"))
    {
        commandType = CPUPROF_COMMAND_COLLECT;
    }
    else if (command.isEqual("translate"))
    {
        commandType = CPUPROF_COMMAND_TRANSLATE;
    }
    else if (command.isEqual("report"))
    {
        commandType = CPUPROF_COMMAND_REPORT;
    }
    else
    {
        commandType = CPUPROF_COMMAND_UNKNOWN;
    }

    int exitCode = 0;

    // User has requested only -v or -h
    if ((args.IsPrintUsage() || args.IsPrintVersion())
        && (commandType == CPUPROF_COMMAND_UNDEFINED))
    {
        exit(exitCode);
    }

    switch (commandType)
    {
        case CPUPROF_COMMAND_UNDEFINED:
        {
            // No command is specified
            reportError(false, L"No Command is specified.\n");
            exitCode = -1;
            exit(exitCode);
        }

        case CPUPROF_COMMAND_COLLECT:
        {
            retVal = HandleCollectCommand(args);
            break;
        }

        case CPUPROF_COMMAND_TRANSLATE:
        {
            retVal = HandleTranslateCommand(args);
            break;
        }

        case CPUPROF_COMMAND_REPORT:
        {
            retVal = HandleReportCommand(args);
            break;
        }

        case CPUPROF_COMMAND_UNKNOWN:
        default:
        {
            // No command is specified
            reportError(false, L"Unknown Command is specified.\n");

            if (! helpPrinted)
            {
                PrintHelp();
                exitCode = -1;
            }

            exit(exitCode);
        }
    }

    return retVal;
}