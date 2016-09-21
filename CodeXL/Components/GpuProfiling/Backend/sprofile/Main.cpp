//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This is the main command line application that will launch
///         all the agents and DX server.
//==============================================================================

#ifdef _WIN32
    #include <windows.h>
    #include "Interceptor.h"
    #include <tchar.h>
    #include <vector>
    #include <string>
#else // LINUX
    #include <sys/wait.h>
    #include <cstdlib>
    #include <cstdio>
    #include <cstring>
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <signal.h>
#include "GPUPerfAPIRegistry.h"
#include "ParseCmdLine.h"
#include "Analyze.h"
#include "OccupancyChart.h"
#include "OccupancyUtils.h"
#include "PerfMarkerAtpFile.h"
#include "../Common/OSUtils.h"
#include "../Common/StringUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/FileUtilsDefs.h"
#include "../Common/StackTraceAtpFile.h"
#include "../CLTraceAgent/CLAtpFile.h"
#include "../HSAFdnTrace/HSAAtpFile.h"
#include "../CLOccupancyAgent/CLOccupancyFile.h"
#include "../Common/Version.h"
#include "../Common/BinFileHeader.h"

#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osOutOfMemoryHandling.h>

using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using namespace std;

static Parameters params;
static Config config;
static osProcessId processId;
static gtString strTmpFilePath;
static bool bDoneMerging = false;

static void MergeFragFiles(int sig);
static void MergeTraceFile(int sig);
static void MergeOccupancyFile(int sig);
static bool SetAgent(const gtString& strDirPath);

#if defined (_LINUX) || defined (LINUX)
    static bool SetPreLoadLibs();
#endif

static bool CheckIsAppValid(const gtString& strAppName, const int iProfilerNbrBits);

static int  GetNbrAppBits(const gtString& strProfiler);

#ifdef _WIN32
    typedef std::wstring EnvSysBlockString;  ///< type of the system env block: std::wstring on Windows std::string on Linux
    #define ENVBLOCKDELIMITER L'\0'
    #define ENVVARSEPARATOR L'='
#elif defined (_LINUX) || defined (LINUX)
    typedef std::string EnvSysBlockString;   ///< type of the system env block: std::wstring on Windows std::string on Linux
    #define ENVBLOCKDELIMITER '\0'
    #define ENVVARSEPARATOR '='
#endif

/// Gets the environment block to be passed to the profiled program
/// \param mapUserBlock a map of the user-specified environment variables
/// \param bIncludeSystemEnv true if mapUserBlock should augment the system environment block, false if mapUserBlock should replace the system environment block
/// \return a string that can be passed to CreateProcess (or the Linux equivalent)
EnvSysBlockString GetEnvironmentBlock(EnvVarMap mapUserBlock, bool bIncludeSystemEnv);

static bool SetHSAServer(const gtString& strDirPath);

/// Set a maximum number of agents to be supported
const unsigned int MAX_NBR_AGENTS = 5;

///Constant strings
#define CL_AGENT_OCCUPANCY  GPU_PROFILER_LIBRARY_NAME_PREFIX "CLOccupancyAgent"
#define CL_AGENT_TRACE      GPU_PROFILER_LIBRARY_NAME_PREFIX "CLTraceAgent"
#define CL_AGENT_PERF_CTR   GPU_PROFILER_LIBRARY_NAME_PREFIX "CLProfileAgent"
#define CL_AGENT_SUB_KRNL   GPU_PROFILER_LIBRARY_NAME_PREFIX "CLSubKernelProfileAgent"
#define LOG_FILE_NAME   "codexlgpuprofiler"
#define LOG_FILE_EXTENSION ".log"

#ifdef _WIN32

// print last error from the system
static bool PrintLastError(wchar_t* szPre)
{
    wchar_t szError[1024];
    DWORD dwError = GetLastError();

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  dwError,
                  0,
                  szError,
                  1024,
                  NULL);

    cout << szPre << ": "
         << dwError << ": " << szError << endl;

    return true;
}

static int CreateProcessWithDetour(gtString& strDirPath, gtString& strAppCommandLine, gtString& strAppWorkingDirectory, bool useDetours)
{
    // set the detoured and microDLL server's path
    string dirPathAsUTF8;
    StringUtils::WideStringToUtf8String(strDirPath.asCharArray(), dirPathAsUTF8);

    char szMicroDllPath[ MAX_PATH ];
    SP_strcpy(szMicroDllPath, MAX_PATH, dirPathAsUTF8.c_str());
    SP_strcat(szMicroDllPath, MAX_PATH, MICRO_DLL);

    // Use Detours to launch the app and load our OpenCL server into the process
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    LPVOID pEnvBlock = NULL;

    EnvSysBlockString strEnvBlock;

    if (!config.mapEnvVars.empty())
    {
        strEnvBlock = GetEnvironmentBlock(config.mapEnvVars, !config.bFullEnvBlock);

        if (!strEnvBlock.empty())
        {
            pEnvBlock = (LPVOID)strEnvBlock.c_str();
        }
    }

    BOOL createProcRetVal = FALSE;

    if (useDetours)
    {
        // Run the app with DCServer enabled so that app with both OpenCL and DirectCompute can run
        createProcRetVal = AMDT::CreateProcessAndInjectDllW(config.strInjectedApp.asCharArray(),
                                                            (LPWSTR)strAppCommandLine.asCharArray(),
                                                            NULL, NULL, TRUE,
                                                            CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
                                                            pEnvBlock,
                                                            strAppWorkingDirectory.asCharArray(),
                                                            &si,
                                                            &pi,
                                                            szMicroDllPath);
    }
    else
    {
        createProcRetVal = CreateProcess(config.strInjectedApp.asCharArray(),
                                         (LPWSTR)strAppCommandLine.asCharArray(),
                                         NULL, NULL, TRUE,
                                         CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
                                         pEnvBlock,
                                         strAppWorkingDirectory.asCharArray(),
                                         &si,
                                         &pi);
    }

    if (!createProcRetVal)
    {
        PrintLastError(L"Failed to start application");
        return -1;
    }

    // On Windows, always set processId > 0 so that we can do merging
    processId = pi.dwProcessId;

    // Resume thread and wait on the process..
    if (ResumeThread(pi.hThread) == (DWORD) - 1)
    {
        PrintLastError(L"Failed to resume thread");
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;

    if (config.bTestMode && GetExitCodeProcess(pi.hProcess, &exitCode))
    {
        // if process returned an error code, return that error code from CodeXLGpuProfiler
        if (exitCode != 0)
        {
            return exitCode;
        }
    }

    return 0;
}

#endif

std::string GetExpectedOutputFile(const std::string& strOutputFileArg, const std::string& strReqdExtension)
{
    std::string strExtension = FileUtils::GetFileExtension(strOutputFileArg);
    std::string strProfileOutputFile("");

    if (strExtension == strReqdExtension)
    {
        strProfileOutputFile = strOutputFileArg;
        return strProfileOutputFile;
    }
    else
    {

        if ((strExtension == TRACE_EXT) ||
            (strExtension == OCCUPANCY_EXT) ||
            (strExtension == PERF_COUNTER_EXT))
        {
            strProfileOutputFile = FileUtils::GetBaseFileName(strOutputFileArg);
            strProfileOutputFile += ".";
            strProfileOutputFile += strReqdExtension;
            return strProfileOutputFile;
        }
        else
        {
            strProfileOutputFile = strOutputFileArg + ".";
            strProfileOutputFile += strReqdExtension;
            return strProfileOutputFile;
        }
    }
}

void CheckOutputFile(const Config& configInner)
{
    std::string strOutputFile("");
    std::string strRequiredExt("");

    if (configInner.bTrace || configInner.bHSATrace || configInner.bMergeMode)
    {
        strRequiredExt.assign(TRACE_EXT);
        strOutputFile = GetExpectedOutputFile(configInner.strOutputFile, strRequiredExt);

        if (FileUtils::FileExist(strOutputFile))
        {
            cout << "Session output path: " << strOutputFile << endl;
        }
        else
        {
            cout << "Failed to generate profile result " << strOutputFile << "." << endl;
        }
    }

    if (configInner.bOccupancy)
    {
        strRequiredExt.assign(OCCUPANCY_EXT);
        strOutputFile = GetExpectedOutputFile(configInner.strOutputFile, strRequiredExt);

        if (FileUtils::FileExist(strOutputFile))
        {
            cout << "Session output path: " << strOutputFile << endl;
        }
        else
        {
            cout << "Failed to generate profile result " << strOutputFile << "." << endl;
        }
    }

    if (configInner.bPerfCounter || configInner.bHSAPMC)
    {
        strRequiredExt.assign(PERF_COUNTER_EXT);
        strOutputFile = GetExpectedOutputFile(configInner.strOutputFile, strRequiredExt);

        if (FileUtils::FileExist(strOutputFile))
        {
            cout << "Session output path: " << strOutputFile << endl;
        }
        else
        {
            cout << "Failed to generate profile result " << strOutputFile << "." << endl;
        }
    }
}

bool DisplayOccupancy(const std::string& strOutputFile)
{
    bool success = false;

    if (!config.strOccupancyParamsFile.empty())
    {
        OccupancyUtils::OccupancyParams paramsInner;

        std::string occupancyError;

        //Generate HTML file
        if (OccupancyUtils::GetOccupancyParamsFromFile(config.strOccupancyParamsFile, paramsInner, occupancyError) &&
            GenerateOccupancyChart(paramsInner, strOutputFile, occupancyError))
        {
            success = true;
        }
        else
        {
            std::cout << "Error generating occupancy display file." << std::endl << occupancyError << std::endl;
            success = false;
        }
    }

    return success;
}

bool ProfileApplication(const std::string& strCounterFile, const int& profilerBits)
{

    std::string counterfile = strCounterFile;

    if ((config.bTrace || config.bHSATrace) && config.bAnalyze && config.analyzeOps.strAtpFile.empty())
    {
        // use trace output as sanalyze module input
        config.analyzeOps.strAtpFile = config.strOutputFile;
    }

    params.m_strOutputFile = config.strOutputFile;
    params.m_strSessionName = config.strSessionName;

    // Note: the following is a fix for CODEXL-50 -- use osDirectory to create the output directory
    //       if it does not already exist.  Some of the code here can be removed when we move to
    //       use osFilePath/gtString in more places in the backend (i.e. like in all members in the
    //       "Config" sturcture declared in ParseCmdLine.h
    gtString gtStringOutputFile;
    gtStringOutputFile.fromUtf8String(params.m_strOutputFile.c_str());
    osFilePath outputFilePath(gtStringOutputFile);
    osDirectory outputDir;
    outputFilePath.getFileDirectory(outputDir);

    if (!outputDir.exists())
    {
        outputDir.create();
    }

    // Note: end fix for CODEXL-50

    //----------------------------------------
    // Merge mode
    //----------------------------------------

    if (config.bMergeMode)
    {
        cout << "--- Merge Mode ---" << endl;
        cout << "Temp files prefix (Process ID): " << config.uiPID << endl;
        processId = config.uiPID;

        if (config.strWorkingDirectory.isEmpty())
        {
            osFilePath tempPath;
            tempPath.setPath(osFilePath::OS_CURRENT_DIRECTORY);
            strTmpFilePath = tempPath.asString();
        }
        else
        {
            strTmpFilePath = config.strWorkingDirectory;
        }

        config.bTrace = true;
        params.m_bTimeOutBasedOutput = true;
        MergeFragFiles(1);

        return true;
    }

    //----------------------------------------
    // Remove all tmp files
    //----------------------------------------
    FileUtils::RemoveFragFiles();

    //----------------------------------------
    // Get CodeXLGpuProfiler.exe's full path
    //----------------------------------------
    gtString strDirPath = FileUtils::GetExePathAsUnicode();

#if defined (_LINUX) || defined (LINUX)

    //----------------------------------------
    // Set replace tilde
    //----------------------------------------
    gtString retVal;
    osGetCurrentProcessEnvVariableValue(L"HOME", retVal);
    string strHomePath;
    StringUtils::WideStringToUtf8String(retVal.asCharArray(), strHomePath);

    FileUtils::ReplaceTilde(strHomePath, config.strOutputFile);
    FileUtils::ReplaceTilde(strHomePath, counterfile);

    // replace tilde using gtString
    if (config.strInjectedApp[0] == '~')
    {
        config.strInjectedApp.extruct(0, 1);
        gtString tempStr = config.strInjectedApp;
        config.strInjectedApp = retVal;
        config.strInjectedApp.appendFormattedString(L"%ls", tempStr.asCharArray());
    }

    // For linux, we need to check file existence before we fork
    osFile fileToCheck(config.strInjectedApp);

    if (!fileToCheck.exists())
    {
        cout << "Process failed to run. Make sure you have specified the correct path." << endl;
        return -1;
    }

#endif

    bool bAnyAgentSet = false;

    //----------------------------------------
    // Set Agent
    //----------------------------------------
    if (config.bHSATrace || config.bHSAPMC)
    {
        bAnyAgentSet |= SetHSAServer(strDirPath);
    }

    bAnyAgentSet |= SetAgent(strDirPath);

    if (!bAnyAgentSet)
    {
        if (!config.bAnalyzeOnly && !config.bMergeMode)
        {
            cout << "No profile mode specified. Nothing will be done." << endl;
        }

        return false;
    }

    //----------------------------------------
    // Pass params
    //----------------------------------------

    params.m_strCmdArgs = config.strInjectedAppArgs;
    params.m_strWorkingDir = config.strWorkingDirectory;
    params.m_strCounterFile = counterfile;
    params.m_strKernelFile = config.strKernelFile;
    params.m_strAPIFilterFile = config.strAPIFilterFile;
    params.m_strDLLPath = strDirPath;
    params.m_cOutputSeparator = config.cOutputSeparator;
    params.m_bVerbose = config.bVerbose;
    params.m_bPerfCounter = config.bPerfCounter;
    params.m_bOutputIL = config.bOutputIL;
    params.m_bOutputISA = config.bOutputISA;
    params.m_bOutputCL = config.bOutputCL;
    params.m_bOutputASM = config.bOutputASM;
    params.m_bOutputHSAIL = config.bOutputHSAIL;
    params.m_bTrace = config.bTrace;
    params.m_bTimeOutBasedOutput = config.bTimeOut;
    params.m_uiTimeOutInterval = config.uiTimeOutInterval;
    params.m_bTestMode = config.bTestMode;
    params.m_bQueryRetStat = config.bQueryRetStat;
    params.m_bCollapseClGetEventInfo = config.bCollapseClGetEventInfo;
    params.m_bUserTimer = config.bUserTimer;
    params.m_strTimerDLLFile = config.strTimerDLLFile;
    params.m_strUserTimerFn = config.strUserTimerFn;
    params.m_strUserTimerInitFn = config.strUserTimerInitFn;
    params.m_strUserTimerDestroyFn = config.strUserTimerDestroyFn;
    params.m_bStackTrace = config.bSym;
    params.m_uiMaxNumOfAPICalls = config.uiMaxNumOfAPICalls;
    params.m_uiMaxKernels = config.uiMaxKernels;
    params.m_bKernelOccupancy = config.bOccupancy;
    params.m_bUserPMC = config.bUserPMCSampler;
    params.m_bCompatibilityMode = config.bCompatibilityMode;
    params.m_strUserPMCLibPath = config.strUserPMCLibPath;
    params.m_bHSATrace = config.bHSATrace;
    params.m_bHSAPMC = config.bHSAPMC;
    params.m_bGMTrace = config.bGMTrace;
    params.m_mapEnvVars = config.mapEnvVars;
    params.m_bFullEnvBlock = config.bFullEnvBlock;
    params.m_bForceSinglePassPMC = config.bForceSinglePassPMC;
    params.m_bGPUTimePMC = config.bGPUTimePMC;
    params.m_bStartDisabled = config.bStartDisabled;
    params.m_delayInMilliseconds = config.m_delayInMilliseconds > 0 ? config.m_delayInMilliseconds : 0;
    params.m_bDelayStartEnabled = config.m_delayInMilliseconds > 0;
    params.m_durationInMilliseconds = config.m_durationInMilliseconds > 0 ? config.m_durationInMilliseconds : 0;
    params.m_bProfilerDurationEnabled = config.m_durationInMilliseconds > 0;

#ifdef GDT_INTERNAL

    if ((params.m_bPerfCounter || params.m_bHSAPMC) && params.m_strCounterFile.empty())
    {
        std::cout << "A counter file must be specified when collecting perf counters in the internal build\n";
        std::cout << "Use --counterfile (or -c) to specify a counter file\n";
        return false;
    }

#endif

    //for debugging
    //cout << strDirPath << endl;
    //cout << config.strInjectedApp << endl;
    //cout << params.m_strOutputFile << endl;

    FileUtils::PassParametersByFile(params);

    //----------------------------------------
    // Get App working dir
    //----------------------------------------

    gtString strAppWorkingDirectory;

    if (config.strWorkingDirectory.isEmpty())
    {
        osFilePath injectedApp(config.strInjectedApp);
        // remove file name and ext:
        injectedApp.setFileName(L"");
        injectedApp.setFileExtension(L"");
        strAppWorkingDirectory = injectedApp.asString();
        // FileUtils::GetWorkingDirectory(config.strInjectedApp, strAppWorkingDirectory);
    }
    else
    {
        strAppWorkingDirectory = config.strWorkingDirectory;
    }


    //----------------------------------------
    // Set signal
    //----------------------------------------
    if (config.bTrace || config.bHSATrace || config.bOccupancy || config.bThreadTrace)
    {
        // set tmp file path
        strTmpFilePath = FileUtils::GetTempFragFilePathAsUnicode();

        signal(SIGABRT, MergeFragFiles);
        signal(SIGTERM, MergeFragFiles);
        signal(SIGINT, MergeFragFiles);
    }

    //----------------------------------------
    // Create process
    //----------------------------------------

    //check that the application to be profiled is valid
    if (!CheckIsAppValid(config.strInjectedApp, profilerBits))
    {
        wcout << config.strInjectedApp.asCharArray() << " is not a valid application" << endl;
        return false;
    }

#ifdef _DEBUG
    bool reportPerfCounterEnablement = true;
#else
    bool reportPerfCounterEnablement = false;
#endif

    if (config.bHSAPMC)
    {
        SetHSASoftCPEnvVar(reportPerfCounterEnablement);
    }

#ifdef _WIN32

    gtString strAppCommandLine;
    strAppCommandLine.appendFormattedString(L"\"%ls\"", config.strInjectedApp.asCharArray());

    // create a command line if the app argument list is not empty
    // put arguments in quotes
    if (!config.strInjectedAppArgs.isEmpty())
    {
        strAppCommandLine.appendFormattedString(L" %ls", config.strInjectedAppArgs.asCharArray());
    }

    // call SetDllDirectory so that the app being profiled can locate and load AMDTBaseTools/AMDTOSWrappers DLLs
    SetDllDirectory(strDirPath.asCharArray());

    int ret = CreateProcessWithDetour(strDirPath, strAppCommandLine, strAppWorkingDirectory, !config.bNoDetours);

    if (ret != 0)
    {
        FileUtils::DeleteTmpFile();
        return false;
    }

#else
    SetPreLoadLibs();

    std::string strAppCommandLine;

    // create a command line if the app argument list is not empty
    // put arguments in quotes
    size_t nCmdlineLength = 0;

    if (!config.strInjectedAppArgs.isEmpty())
    {
        StringUtils::WideStringToUtf8String(config.strInjectedAppArgs.asCharArray(), strAppCommandLine);
        nCmdlineLength = strAppCommandLine.length();
    }

    char* pszCmdline = new(std::nothrow) char[nCmdlineLength + 1];

    if (pszCmdline == NULL)
    {
        cout << "Error processing command line\n";
        return false;
    }

    if (nCmdlineLength > 0)
    {
        strcpy(pszCmdline, strAppCommandLine.c_str());
    }
    else
    {
        pszCmdline[0] = '\0';
    }

    char szExe[SP_MAX_PATH] = { '\0' };
    std::string convertedInjectApp;
    StringUtils::WideStringToUtf8String(config.strInjectedApp.asCharArray(), convertedInjectApp);

    strcpy(szExe, convertedInjectApp.c_str());

    const char* pEnvBlock = NULL;

    EnvSysBlockString strEnvBlock;

    if (!config.mapEnvVars.empty())
    {
        strEnvBlock = GetEnvironmentBlock(config.mapEnvVars, !config.bFullEnvBlock);

        if (!strEnvBlock.empty())
        {
            pEnvBlock = strEnvBlock.c_str();
        }
    }

    std::string convertedWorkingDir;
    StringUtils::WideStringToUtf8String(strAppWorkingDirectory.asCharArray(), convertedWorkingDir);

    processId = OSUtils::Instance()->ExecProcess(szExe, pszCmdline, convertedWorkingDir.c_str(), pEnvBlock);

    if (processId < 0)
    {
        // error
        processId = 0;
        std::cout << "error in fork()\n";
        exit(1);
    }
    else if (processId > 0)
    {
        // parent code
        int status;
        waitpid(processId, &status, 0);

        // if process returned an error, return that error code from CodeXLGpuProfiler
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            retVal = WEXITSTATUS(status);
        }

        // if process was terminated by a signal, return that signal number from CodeXLGpuProfiler
        if (WIFSIGNALED(status) && WTERMSIG(status) != 0)
        {
            retVal = WTERMSIG(status);
        }
    }

    delete[] pszCmdline;
#endif

    if (config.bHSAPMC)
    {
        UnsetHSASoftCPEnvVar(reportPerfCounterEnablement);
    }

    //----------------------------------------
    // Unset agent before calling CLUtils to generate atp file header
    //----------------------------------------
    OSUtils::Instance()->UnsetEnvVar(OCL_ENABLE_PROFILING_ENV_VAR);

    if (config.bHSATrace)
    {
        OSUtils::Instance()->UnsetEnvVar(HSA_ENABLE_PROFILING_ENV_VAR);
    }

    //----------------------------------------
    // Merge result if needed
    //----------------------------------------

    MergeFragFiles(1);
    CheckOutputFile(config);

    return true;
}


bool ProcessCommandLine(const std::string& strCounterFile)
{
    //If the occupancy switch is set, open the occupancy parameters file and parse
    //Then, generate the HTML output.
    if (config.bOccupancyDisplay)
    {
        if (!config.strOccupancyParamsFile.empty())
        {
            return DisplayOccupancy(config.strOutputFile);
        }
    }

    bool success = false;

    if (!config.bAnalyzeOnly)
    {
        //Get the number of bits of the profiler
        int iProfilerNbrBits = FileUtils::FILE_BITS_UNKNOWN;

        gtString strProfiler = FileUtils::GetExeFullPathAsUnicode();

        iProfilerNbrBits = GetNbrAppBits(strProfiler);
        success = ProfileApplication(strCounterFile, iProfilerNbrBits);
    }


    //----------------------------------------
    // Summary
    //----------------------------------------
    if (config.bAnalyze)
    {
        if (!APITraceAnalyze(config))
        {
            cout << "\nFailed to generate summary pages\n";
            success &= false;
        }
    }

    //----------------------------------------
    // Cleanup
    //----------------------------------------

    FileUtils::DeleteTmpFile();

    return success;
}

#ifdef _WIN32
    int _tmain(int argc, wchar_t* argv[])
#else
    int main(int argc, char* argv[])
#endif
{
    int retVal = 0;

    std::string strLogFileWithPath = FileUtils::GetDefaultOutputPath() + LOG_FILE_NAME + LOG_FILE_EXTENSION;
    LogFileInitialize(strLogFileWithPath.c_str());

    // First, register the out-of-memory event handler.
    std::set_new_handler(osDumpCallStackAndExit);


#ifdef _WIN32

    if (!ParseCmdLine(argc, argv, config))
    {
        return -1;
    }

#else
    std::vector<wchar_t*> convertedArg;
    convertedArg.reserve(argc);
    wstring wargs[argc];

    for (int nArg = 0 ; nArg < argc ; nArg++)
    {
        StringUtils::Utf8StringToWideString(argv[nArg], wargs[nArg]);
        convertedArg.push_back(const_cast<wchar_t*>(wargs[nArg].c_str()));
    }

    if (!ParseCmdLine(argc, convertedArg.data(), config))
    {
        return -1;
    }

#endif

    bool success = false;

    bool isCounterFileMoreThanOne = config.counterFileList.size() > 1 ? true : false;
    std::string defaultOutputFileName = config.strOutputFile;

    if (!config.counterFileList.empty())
    {
        bool isReplaying = false;

        for (unsigned int i = 0; i < config.counterFileList.size(); ++i)
        {
            std::string outputFileName;
            std::string logFile;
            std::string appendString;

            if (!outputFileName.empty())
            {
                outputFileName.clear();
            }

            if (isReplaying)
            {
                config.bTrace = false;
                config.bHSATrace = false;
                config.bMergeMode = false;
                config.bSubKernelProfile = false;
                config.bThreadTrace = false;
                config.bOccupancy = false;
                config.strOutputFile = defaultOutputFileName;
            }

            //Output File
            //----------------------------------------
            // Get output file path
            //----------------------------------------
            if (config.strOutputFile.empty())
            {
                if (isCounterFileMoreThanOne)
                {
                    std::stringstream stringStream;
                    stringStream << "_pass" << (i + 1);
                    appendString = stringStream.str() ;
                }

                if (config.bPerfCounter)
                {
                    outputFileName = FileUtils::GetDefaultProfileOutputFile(appendString);
                }
                else if (config.bTrace || config.bHSATrace || config.bMergeMode)
                {
                    outputFileName = FileUtils::GetDefaultTraceOutputFile();
                }
                else if (config.bSubKernelProfile)
                {
                    outputFileName = FileUtils::GetDefaultSubKernelProfileOutputFile();
                }
                else if (config.bThreadTrace)
                {
                    outputFileName = FileUtils::GetDefaultThreadTraceOutputDir();
                }
            }
            else
            {
                config.strOutputFile = FileUtils::ToAbsPath(config.strOutputFile);
                outputFileName = config.strOutputFile;

                if (isCounterFileMoreThanOne)
                {
                    std::stringstream stringStream;
                    stringStream << "_pass" << (i + 1);
                    outputFileName += stringStream.str();
                }

                std::string strRequiredExt("");

                if (config.bPerfCounter || config.bHSAPMC)
                {
                    strRequiredExt.assign(PERF_COUNTER_EXT);
                    outputFileName = GetExpectedOutputFile(outputFileName, strRequiredExt);
                }
                else if (config.bTrace || config.bHSATrace || config.bMergeMode)
                {
                    strRequiredExt.assign(TRACE_EXT);
                    outputFileName = GetExpectedOutputFile(outputFileName, strRequiredExt);
                }
                else if (config.bOccupancy)
                {
                    strRequiredExt.assign(OCCUPANCY_EXT);
                    outputFileName = GetExpectedOutputFile(outputFileName, strRequiredExt);
                }
            }

            config.strOutputFile = outputFileName;
            success = ProcessCommandLine(config.counterFileList[i]);
            isReplaying = true;
        }
    }
    else
    {
        std::string counterFile;
        std::string outputFileName;

        if (config.strOutputFile.empty())
        {
            if (config.bPerfCounter)
            {
                outputFileName = FileUtils::GetDefaultProfileOutputFile();
            }
            else if (config.bTrace || config.bHSATrace || config.bMergeMode)
            {
                outputFileName = FileUtils::GetDefaultTraceOutputFile();
            }
            else if (config.bSubKernelProfile)
            {
                outputFileName = FileUtils::GetDefaultSubKernelProfileOutputFile();
            }
            else if (config.bThreadTrace)
            {
                outputFileName = FileUtils::GetDefaultThreadTraceOutputDir();
            }
        }
        else
        {
            config.strOutputFile = FileUtils::ToAbsPath(config.strOutputFile);
            outputFileName = config.strOutputFile;

            std::string strRequiredExt("");

            if (config.bPerfCounter || config.bHSAPMC)
            {
                strRequiredExt.assign(PERF_COUNTER_EXT);
                outputFileName = GetExpectedOutputFile(outputFileName, strRequiredExt);
            }
            else if (config.bTrace || config.bHSATrace || config.bMergeMode)
            {
                strRequiredExt.assign(TRACE_EXT);
                outputFileName = GetExpectedOutputFile(outputFileName, strRequiredExt);
            }
            else if (config.bOccupancy)
            {
                strRequiredExt.assign(OCCUPANCY_EXT);
                outputFileName = GetExpectedOutputFile(outputFileName, strRequiredExt);
            }
        }

        config.strOutputFile = outputFileName;
        success = ProcessCommandLine(counterFile);
    }

    return retVal = success ? 0 : -1;
}

static void MergeFragFiles(int sig)
{
    if (!bDoneMerging)
    {
        MergeTraceFile(sig);
        MergeOccupancyFile(sig);
        bDoneMerging = true;
        signal(SIGINT, SIG_DFL);
    }
}

static void MergeTraceFile(int sig)
{
    SP_UNREFERENCED_PARAMETER(sig);

    if ((config.bTrace || config.bHSATrace || config.bMergeMode) && processId > 0)
    {
        std::string pid = StringUtils::ToString(processId);
        AtpFileWriter writer(config, pid);

        CLAtpFilePart* pOclTrace = nullptr;
        StackTraceAtpFilePart* pClStackTrace = nullptr;
        StackTraceAtpFilePart* pHsaStackTrace = nullptr;
        HSAAtpFilePart* pHsaTrace = nullptr;

        if (config.bTrace)
        {
            pOclTrace = new(std::nothrow) CLAtpFilePart(config);

            if (nullptr != pOclTrace)
            {
                writer.AddAtpFilePart(pOclTrace);
            }

            if (config.bSym)
            {
                pClStackTrace = new(std::nothrow) StackTraceAtpFilePart(string("ocl"), config);

                if (nullptr != pClStackTrace)
                {
                    writer.AddAtpFilePart(pClStackTrace);
                }
            }
        }

        if (config.bHSATrace)
        {
            pHsaTrace = new(std::nothrow) HSAAtpFilePart(config);

            if (nullptr != pHsaTrace)
            {
                writer.AddAtpFilePart(pHsaTrace);
            }

            if (config.bSym)
            {
                pHsaStackTrace = new(std::nothrow) StackTraceAtpFilePart(string("hsa"), config);

                if (nullptr != pHsaStackTrace)
                {
                    writer.AddAtpFilePart(pHsaStackTrace);
                }
            }
        }

        PerfMarkerAtpFilePart perfMarker(config);
        writer.AddAtpFilePart(&perfMarker);

        writer.SaveToAtpFile();

        SAFE_DELETE(pOclTrace);
        SAFE_DELETE(pClStackTrace);
        SAFE_DELETE(pHsaStackTrace);
        SAFE_DELETE(pHsaTrace);
    }
}

static bool SetHSAServer(const gtString& strDirPathUnicode)
{
    SP_TODO("Solve API Loader Issues for installed libraries in unicode directory");
    std::wstring strDirPathWide = strDirPathUnicode.asCharArray();
    std::string strDirPath;
    StringUtils::WideStringToUtf8String(strDirPathWide, strDirPath);

    string strServerPath;

    if (config.bHSATrace)
    {
        strServerPath = strDirPath + HSA_TRACE_AGENT_DLL;

        if (config.bOccupancy)
        {
            std::cout << "Collecting Occupancy data is not supported when performing an Application Trace.  Use Performance Counter mode if you want to collect Occupancy data.\n";
        }
    }
    else if (config.bHSAPMC)
    {
        strServerPath = strDirPath + HSA_PROFILE_AGENT_DLL;
    }
    else
    {
        return false;
    }

    if (!FileUtils::FileExist(strServerPath))
    {
        size_t nSlashPos = strServerPath.find_last_of("/\\", strServerPath.size()) + 1;

        if (nSlashPos != std::string::npos)
        {
            strServerPath = strServerPath.substr(nSlashPos);
        }

        std::cout << strServerPath << " is missing" << std::endl;
        std::cout << "Make sure you have " << strServerPath << " under " << strDirPath.c_str() << std::endl;

        return false;
    }

    if (config.bHSATrace || config.bHSAPMC)
    {
        // need to specify the runtime tools lib as well for both HSA Trace and HSA PMC mode
        string toolsRuntimeLib = HSA_RUNTIME_TOOLS_LIB;
        toolsRuntimeLib.append(" ");
        strServerPath = toolsRuntimeLib + strServerPath;
    }

    OSUtils::Instance()->SetEnvVar(HSA_ENABLE_PROFILING_ENV_VAR, strServerPath.c_str());
    return true;
}

static bool SetAgent(const gtString& strDirPathUnicode)
{
    SP_TODO("Solve API Loader Issues for installed libraries in unicode directory");
    std::wstring strDirPathWide = strDirPathUnicode.asCharArray();
    std::string strDirPath;
    StringUtils::WideStringToUtf8String(strDirPathWide, strDirPath);

    bool retVal = false;

    std::string strAgentDllPath;
    std::vector<std::string> vAgentDllPath;
    char szAgentDllPath[ SP_MAX_PATH ] = "\0";

    //   const unsigned int AGENT_OCC   = 4;

    if (config.bTrace)
    {
        SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_TRACE_AGENT_DLL);
        strAgentDllPath = strDirPath + string(szAgentDllPath);
        vAgentDllPath.push_back(strAgentDllPath);

        if (config.bOccupancy)
        {
            strAgentDllPath.clear();
            SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_OCCUPANCY_AGENT_DLL);
            strAgentDllPath = strDirPath + string(szAgentDllPath);
            vAgentDllPath.push_back(strAgentDllPath);
        }

        if (config.bSubKernelProfile)
        {
            strAgentDllPath.clear();
            SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_SUB_KERNEL_PROFILE_AGENT_DLL);
            strAgentDllPath = strDirPath + string(szAgentDllPath);
            vAgentDllPath.push_back(strAgentDllPath);
        }

        if (config.bPerfCounter)
        {
            strAgentDllPath.clear();
            SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_PROFILE_AGENT_DLL);
            strAgentDllPath = strDirPath + string(szAgentDllPath);
            vAgentDllPath.push_back(strAgentDllPath);
        }
    }
    else if (config.bSubKernelProfile)
    {

        if (config.bOccupancy)
        {
            strAgentDllPath.clear();
            SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_OCCUPANCY_AGENT_DLL);
            strAgentDllPath = strDirPath + string(szAgentDllPath);
            vAgentDllPath.push_back(strAgentDllPath);
        }

        strAgentDllPath.clear();
        SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_SUB_KERNEL_PROFILE_AGENT_DLL);
        strAgentDllPath = strDirPath + string(szAgentDllPath);
        vAgentDllPath.push_back(strAgentDllPath);

        if (config.bPerfCounter)
        {
            strAgentDllPath.clear();
            SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_PROFILE_AGENT_DLL);
            strAgentDllPath = strDirPath + string(szAgentDllPath);
            vAgentDllPath.push_back(strAgentDllPath);
        }
    }
    else if (config.bPerfCounter)
    {
        if (config.bOccupancy)
        {
            strAgentDllPath.clear();
            SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_OCCUPANCY_AGENT_DLL);
            strAgentDllPath = strDirPath + string(szAgentDllPath);
            vAgentDllPath.push_back(strAgentDllPath);
        }

        strAgentDllPath.clear();
        SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_PROFILE_AGENT_DLL);
        strAgentDllPath = strDirPath + string(szAgentDllPath);
        vAgentDllPath.push_back(strAgentDllPath);
    }
    else if (config.bOccupancy && !config.bHSAPMC  && !config.bHSATrace)
    {
        strAgentDllPath.clear();
        SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_OCCUPANCY_AGENT_DLL);
        strAgentDllPath = strDirPath + string(szAgentDllPath);
        vAgentDllPath.push_back(strAgentDllPath);
    }
    else if (config.bThreadTrace)
    {
        strAgentDllPath.clear();
        SP_strcpy(szAgentDllPath, SP_MAX_PATH, CL_THREAD_TRACE_AGENT_DLL);
        strAgentDllPath = strDirPath + string(szAgentDllPath);
        vAgentDllPath.push_back(strAgentDllPath);
    }

    for (unsigned int i = 0; i < vAgentDllPath.size(); ++i)
    {
        if (!FileUtils::FileExist(vAgentDllPath[i]))
        {
            std::string strAgent = vAgentDllPath[i];
            size_t nSlashPos = strAgent.find_last_of("/\\", strAgent.size()) + 1;

            if (nSlashPos != std::string::npos)
            {
                strAgent = strAgent.substr(nSlashPos);
            }

            std::cout << strAgent << " is missing" << std::endl;
            std::cout << "Make sure you have " << strAgent << " under " << strDirPath.c_str() << std::endl;

            for (unsigned int j = i; j < vAgentDllPath.size() - 1; ++j)
            {
                vAgentDllPath[j] = vAgentDllPath[j + 1];
            }

            vAgentDllPath.pop_back();
            i--;
        }
    }

    // Check existing agents
    string agents = OSUtils::Instance()->GetEnvVar(OCL_ENABLE_PROFILING_ENV_VAR);

    if (!agents.empty())
    {
        // Each token should be ended by a comma to signal the end of the token.
        // If the agents string does not end with a comma, append it

        size_t nAgentStringLength = agents.length();

        if (agents[nAgentStringLength] != ',')
        {
            agents.append(",");
        }

        //If the agents are not empty, parse the values of the environment variable
        string strToken;
        size_t nPos1 = 0;
        size_t nPos2 = agents.find_first_of(",", nPos1);

        while (nPos2 != string::npos)
        {
            strToken = agents.substr(nPos1, nPos2);

            // get ready to extract the next token
            nPos1 = nPos2;
            nPos2 = agents.find_first_of(",", nPos1 + 1);

            // After having extracted the agent strings, check if it agent has already been set
            // via the switches
            if (strToken.find(CL_AGENT_TRACE) != string::npos)
            {
                if (!config.bTrace)
                {
                    vAgentDllPath.insert(vAgentDllPath.begin(), strToken);
                }
            }
            else if (strToken.find(CL_AGENT_OCCUPANCY) != string::npos)
            {
                if (!config.bOccupancy)
                {
                    if (vAgentDllPath[0].find(CL_AGENT_TRACE) != string::npos)
                    {
                        vAgentDllPath.insert(vAgentDllPath.begin() + 1, strToken);
                    }
                    else
                    {
                        vAgentDllPath.insert(vAgentDllPath.begin(), strToken);
                    }
                }
            }
            else if (strToken.find(CL_AGENT_SUB_KRNL) != string::npos)
            {
                if (!config.bSubKernelProfile)
                {
                    if (vAgentDllPath[ vAgentDllPath.size() - 1 ].find(CL_AGENT_PERF_CTR) != string::npos)
                    {
                        vAgentDllPath.insert(vAgentDllPath.begin() + vAgentDllPath.size() - 2, strToken);
                    }
                    else
                    {
                        vAgentDllPath.push_back(strToken);
                    }
                }
            }
            else if (strToken.find(CL_AGENT_PERF_CTR) != string::npos)
            {
                if (!config.bPerfCounter)
                {
                    vAgentDllPath.push_back(strToken);
                }
            }
            else //not a recognized token
            {
                if (!strToken.empty())   //assume a user-specified token
                {
                    if (vAgentDllPath[0].find(CL_AGENT_TRACE) != string::npos)
                    {
                        if (vAgentDllPath.size() > 1)
                        {
                            // Additional token, so need to check if it is occupancy agent or not
                            // If occupancy agent, append after occupancy agent, otherwise, before next
                            // agent
                            if (vAgentDllPath[1].find(CL_AGENT_OCCUPANCY) != string::npos)
                            {
                                vAgentDllPath.insert(vAgentDllPath.begin() + 2, strToken);
                            }
                            else
                            {
                                vAgentDllPath.insert(vAgentDllPath.begin() + 1, strToken);
                            }
                        }
                        else
                        {
                            // At end of agent list, so add user-token to end
                            vAgentDllPath.push_back(strToken);
                        }
                    }
                    else
                    {
                        if (vAgentDllPath[0].find(CL_AGENT_OCCUPANCY) != string::npos)
                        {
                            vAgentDllPath.insert(vAgentDllPath.begin() + 1, strToken);
                        }
                        else
                        {
                            vAgentDllPath.insert(vAgentDllPath.begin(), strToken);
                        }
                    }
                }
            }
        }


        //concatenate the agent dlls into a string (comma-separated and terminated)
        //    agents = szSecondAgentDllPath + ',' + agents + ',';
        agents.clear();

        for (int i = (int)vAgentDllPath.size() - 1; i >= 0; i--)
        {
            agents += vAgentDllPath[i] + ",";
        }
    }
    else
    {
        agents.clear();

        for (int i = (int)vAgentDllPath.size() - 1; i >= 0; i--)
        {
            agents += vAgentDllPath[i] + ",";
        }
    }

    if (!agents.empty())
    {
        // Set environment variable
        OSUtils::Instance()->SetEnvVar(OCL_ENABLE_PROFILING_ENV_VAR, agents.c_str());
        retVal = true;
    }

    return retVal;
}

#if defined (_LINUX) || defined (LINUX)

static bool SetPreLoadLibs()
{
    bool retVal = true;

    if (!config.strPreloadLib.empty())
    {
        static const char* LD_PRELOAD_ENV_VAR_NAME = "LD_PRELOAD";
        std::string curPreLoadPath = OSUtils::Instance()->GetEnvVar(LD_PRELOAD_ENV_VAR_NAME);

        if (!curPreLoadPath.empty())
        {
            curPreLoadPath = ":" + curPreLoadPath;
        }

        curPreLoadPath = config.strPreloadLib + curPreLoadPath;

        retVal = OSUtils::Instance()->SetEnvVar(LD_PRELOAD_ENV_VAR_NAME, curPreLoadPath.c_str());
    }

    return retVal;
}

#endif

EnvSysBlockString GetEnvironmentBlock(EnvVarMap mapUserBlock, bool bIncludeSystemEnv)
{
    EnvVarMap envBlock;

    if (bIncludeSystemEnv)
    {
        // first, get the entire environment block for the process
        ENVSYSBLOCK envStrs = OSUtils::Instance()->GetSysEnvBlock();

        if (envStrs != NULL)
        {
            ENVSYSBLOCK pCurEnvVar = envStrs;

            EnvSysBlockString strCurEnvVar;
            EnvSysBlockString varName;
            EnvSysBlockString varValue;

            while (*pCurEnvVar != '\0')
            {
                strCurEnvVar = pCurEnvVar;

                size_t equalPos = strCurEnvVar.find('=');

                if (equalPos != 0)
                {
                    if (equalPos != std::string::npos)
                    {
                        varName = strCurEnvVar.substr(0, equalPos);
                        varValue = strCurEnvVar.substr(equalPos + 1);
                    }
                    else
                    {
                        varName = strCurEnvVar;
                        varValue.clear();
                    }

#ifdef _WIN32
                    gtString strVarName(varName.c_str());
                    gtString strVarValue(varValue.c_str());
#elif defined (_LINUX) || defined (LINUX)
                    gtString strVarName;
                    gtString strVarValue;
                    strVarName.fromASCIIString(varName.c_str());
                    strVarValue.fromASCIIString(varValue.c_str());
#endif

                    envBlock[strVarName] = strVarValue;
                }

                pCurEnvVar += strCurEnvVar.length() + 1;
            }

            OSUtils::Instance()->ReleaseSysEnvBlock(envStrs);
        }
    }

#if defined (_LINUX) || defined (LINUX)
    std::string strAgentVar = OSUtils::Instance()->GetEnvVar(OCL_ENABLE_PROFILING_ENV_VAR);

    if (!strAgentVar.empty())
    {
        gtString agentVar;
        gtString agentVarVal;
        agentVar.fromASCIIString(OCL_ENABLE_PROFILING_ENV_VAR);
        agentVarVal.fromASCIIString(strAgentVar.c_str());
        envBlock[agentVar] = agentVarVal;
    }

    strAgentVar = OSUtils::Instance()->GetEnvVar(HSA_ENABLE_PROFILING_ENV_VAR);

    if (!strAgentVar.empty())
    {
        gtString agentVar;
        gtString agentVarVal;
        agentVar.fromASCIIString(HSA_ENABLE_PROFILING_ENV_VAR);
        agentVarVal.fromASCIIString(strAgentVar.c_str());
        envBlock[agentVar] = agentVarVal;
    }

#endif

    // next, replace or add the user-specified env vars
    for (EnvVarMap::const_iterator it = mapUserBlock.begin(); it != mapUserBlock.end(); it++)
    {
        envBlock[(*it).first] = (*it).second;
    }

    // finally, build a zero-separated, double-zero-terminated string that can be passed to CreateProcess
    EnvSysBlockString strEnvVarBlock;

    for (EnvVarMap::const_iterator it = envBlock.begin(); it != envBlock.end(); it++)
    {
#ifdef _WIN32
        EnvSysBlockString strValName((*it).first.asCharArray());
        EnvSysBlockString strValValue((*it).second.asCharArray());
#elif defined (_LINUX) || defined (LINUX)
        EnvSysBlockString strValName((*it).first.asASCIICharArray());
        EnvSysBlockString strValValue((*it).second.asASCIICharArray());
#endif
        strEnvVarBlock += strValName + ENVVARSEPARATOR + strValValue + ENVBLOCKDELIMITER;
    }

    strEnvVarBlock += ENVBLOCKDELIMITER;

    return strEnvVarBlock;
}

bool CheckIsAppValid(const gtString& strAppName, int iProfilerNbrBits)
{
    bool bStatus = true;


    int nFileType = FileUtils::GetBinaryFileType(strAppName);

    if (nFileType < 0)
    {
        cout << "Unable to open the application to be profiled to determine file type" << endl;
    }

    if (nFileType == FileUtils::FTYPE_EXE)
    {
        bStatus = true;
    }
    else
    {
        bStatus = false;
    }

    //Assume that the app to be profiled is a valid executable, next, we check that the bitness
    //is the same as that of the profiler
    int iBinaryFileNbrBits = FileUtils::FILE_BITS_UNKNOWN;

    if (bStatus == true)
    {
        iBinaryFileNbrBits = FileUtils::GetBinaryNbrBits(strAppName);

        if (iBinaryFileNbrBits != iProfilerNbrBits)
        {
            cout << "The profiler is a " << iProfilerNbrBits << " bit application while the application to be profiled is " << iBinaryFileNbrBits << " bits." << std::endl;
            cout << "The number of address bits of the profiler must match the number of the address bits of the application being profiled" << std::endl;
            bStatus = false;
        }
    }

    return bStatus;
}

int GetNbrAppBits(const gtString& strProfiler)
{
    int nBits = FileUtils::FILE_BITS_UNKNOWN;
    nBits = FileUtils::GetBinaryNbrBits(strProfiler);
    return nBits;
}

static void MergeOccupancyFile(int sig)
{
    SP_UNREFERENCED_PARAMETER(sig);

    if (((config.bOccupancy && config.bTimeOut) || config.bMergeMode) && processId > 0)
    {
        CLOccupancyHdr header;
        header.m_iVersionMajor = GPUPROFILER_BACKEND_MAJOR_VERSION;
        header.m_iVersionMinor = GPUPROFILER_BACKEND_MINOR_VERSION;
        header.m_strAppName = config.strInjectedApp;
        header.m_strAppArgs = config.strInjectedAppArgs;
        header.m_listSeparator = config.cOutputSeparator;
        std::string pid = StringUtils::ToString(processId);

        MergeTmpCLOccupancyFile(config.strOutputFile, strTmpFilePath, pid, header);
    }

    signal(SIGINT, SIG_DFL);
}
