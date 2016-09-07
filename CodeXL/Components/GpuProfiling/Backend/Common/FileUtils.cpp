//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File Utils functions
//==============================================================================

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <direct.h>
    #include <tchar.h>
    #define GetCurrentDir _getcwd
#else
    #include <cstdlib> // getenv
    #include <sys/types.h> // opendir
    #include <dirent.h>
    #include <unistd.h>
    #include <errno.h>
    #define GetCurrentDir getcwd
#endif
#include <time.h>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <locale>
#include "FileUtils.h"
#include "StringUtils.h"
#include "Logger.h"
#include "LocaleSetting.h"
#include "OSUtils.h"
#include "Defs.h"

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>

using std::stringstream;
using std::ofstream;
using std::ifstream;
using std::string;
using std::vector;
using std::endl;

using namespace GPULogger;


#ifdef _DEBUG
//
// Pause application to allow debugger to attach
// This functionality only happens once - on the first call to
// LoadLibrary.
//
void FileUtils::CheckForDebuggerAttach(void)
{
    static const gtString checkForDebuggerAttachEnvVarName = L"GPUProfilerCheckForDebuggerAttach";
    gtString checkForDebuggerAttachEnvVarValue = L"";

    osGetCurrentProcessEnvVariableValue(checkForDebuggerAttachEnvVarName, checkForDebuggerAttachEnvVarValue);

    bool shouldStop = L"true" == checkForDebuggerAttachEnvVarValue;  // only pause application once.

    if (shouldStop)
    {
        osEnvironmentVariable checkForDebuggerAttachEnvVar(checkForDebuggerAttachEnvVarName, L"false");
        osSetCurrentProcessEnvVariable(checkForDebuggerAttachEnvVar);

        std::stringstream ss;

        ss << "The application has been paused to allow a debugger to be attached to the process." << endl << endl;
        ss << "Process = " << FileUtils::GetExeFullPath() << endl << endl;
        ss << "ProcessID = " << osGetCurrentProcessId() << endl << endl;
#ifdef _WIN32
        ss << "Press OK to continue";
        string errorMessage = ss.str().c_str();
#ifdef UNICODE
        std::wstring errorMessageConverted;
        StringUtils::Utf8StringToWideString(errorMessage, errorMessageConverted);
#else
        std::string errorMessageConverted = errorMessage;
#endif
        MessageBox(NULL, errorMessageConverted.c_str(), _T("CodeXL GPU Profiler"),
                   MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_TASKMODAL);
#else
        ss << "Press 'y' to continue...";
        std::cout << ss.str();
        char ch = '\0';

        do
        {
            std::cin >> ch;
        }
        while (ch != 'y');

#endif
    }
}
#endif


std::string FileUtils::GetTempFile()
{
    std::string strTmpPath;
#ifdef _WIN32
    strTmpPath = OSUtils::Instance()->GetEnvVar("TEMP");
    strTmpPath.append("\\tmp.spdata");
#else //_LINUX || LINUX
    strTmpPath = OSUtils::Instance()->GetEnvVar("HOME");
    strTmpPath.append("/.spdata");
#endif
    return strTmpPath;
}

std::string FileUtils::GetTempFragFilePath()
{
    std::string strTmpPath;
#ifdef _WIN32
    strTmpPath = OSUtils::Instance()->GetEnvVar("TEMP");
    strTmpPath.append("\\");
#else //_LINUX || LINUX
    strTmpPath = OSUtils::Instance()->GetEnvVar("HOME");
    strTmpPath.append("/");
#endif
    return strTmpPath;
}

gtString FileUtils::GetTempFragFilePathAsUnicode()
{
    gtString retVal;
#ifdef _WIN32
    osGetCurrentProcessEnvVariableValue(L"TEMP", retVal);
    retVal.append(L"\\");
#else //_LINUX || LINUX
    osGetCurrentProcessEnvVariableValue(L"HOME", retVal);
    retVal.append(L"/");
#endif
    return retVal;
}

void FileUtils::PassParametersByFile(Parameters params)
{
    std::ofstream fout;

    fout.open(GetTempFile().c_str());
    fout << "VersionMajor=" << params.m_uiVersionMajor << endl;
    fout << "VersionMinor=" << params.m_uiVersionMinor << endl;
    fout << "CmdArgs=" << params.m_strCmdArgs.asUTF8CharArray() << endl;
    fout << "OutputFile=" << params.m_strOutputFile.c_str() << endl;
    fout << "WorkingDir=" << params.m_strWorkingDir.asUTF8CharArray() << endl;
    fout << "SessionName=" << params.m_strSessionName.c_str() << endl;
    fout << "DLLPath=" << params.m_strDLLPath.asUTF8CharArray() << endl;
    fout << "CounterFile=" << params.m_strCounterFile.c_str() << endl;
    fout << "KernelFile=" << params.m_strKernelFile.c_str() << endl;
    fout << "APIFilterFile=" << params.m_strAPIFilterFile.c_str() << endl;
    fout << "Verbose=" << (params.m_bVerbose ? "True" : "False") << endl;
    fout << "StartDisabled=" << (params.m_bStartDisabled ? "True" : "False") << endl;
    fout << "OutputIL=" << (params.m_bOutputIL ? "True" : "False") << endl;
    fout << "OutputHSAIL=" << (params.m_bOutputHSAIL ? "True" : "False") << endl;
    fout << "OutputISA=" << (params.m_bOutputISA ? "True" : "False") << endl;
    fout << "OutputCL=" << (params.m_bOutputCL ? "True" : "False") << endl;
    fout << "OutputASM=" << (params.m_bOutputASM ? "True" : "False") << endl;
    fout << "PerfCounter=" << (params.m_bPerfCounter ? "True" : "False") << endl;
    fout << "Trace=" << (params.m_bTrace ? "True" : "False") << endl;
    fout << "HSATrace=" << (params.m_bHSATrace ? "True" : "False") << endl;
    fout << "HSAPMC=" << (params.m_bHSAPMC ? "True" : "False") << endl;
    fout << "SinglePassPMC=" << (params.m_bForceSinglePassPMC ? "True" : "False") << endl;
    fout << "GPUTimePMC=" << (params.m_bGPUTimePMC ? "True" : "False") << endl;
    fout << "TimeOut=" << (params.m_bTimeOutBasedOutput ? "True" : "False") << endl;
    fout << "QueryRetStat=" << (params.m_bQueryRetStat ? "True" : "False") << endl;
    fout << "CollapseClGetEventInfo=" << (params.m_bCollapseClGetEventInfo ? "True" : "False") << endl;
    fout << "Interval=" << params.m_uiTimeOutInterval << endl;
    fout << "Separator=" << params.m_cOutputSeparator << endl;
    fout << "UserTimerIsUsed=" << (params.m_bUserTimer ? "True" : "False") << endl;
    fout << "UserTimerDLLFileName=" << params.m_strTimerDLLFile.c_str() << endl;
    fout << "UserTimerFunctionName=" << params.m_strUserTimerFn.c_str() << endl;
    fout << "UserTimerInitFunctionName=" << params.m_strUserTimerInitFn.c_str() << endl;
    fout << "UserTimerDestroyFunctionName=" << params.m_strUserTimerDestroyFn.c_str() << endl;
    fout << "UserPMC=" << (params.m_bUserPMC ? "True" : "False") << endl;
    fout << "CompatibilityMode=" << (params.m_bCompatibilityMode ? "True" : "False") << endl;
    fout << "UserPMCLibPath=" << params.m_strUserPMCLibPath.c_str() << endl;
    fout << "StackTrace=" << (params.m_bStackTrace ? "True" : "False") << endl;
    fout << "MaxNumOfAPICalls=" << params.m_uiMaxNumOfAPICalls << endl;
    fout << "MaxKernels=" << params.m_uiMaxKernels << endl;
    fout << "KernelOccupancy=" << (params.m_bKernelOccupancy ? "True" : "False") << endl;
    fout << "GMTrace=" << (params.m_bGMTrace ? "True" : "False") << endl;
    fout << "FullEnvBlock=" << (params.m_bFullEnvBlock ? "True" : "False") << endl;
    fout << "ForceSingleGPU=" << (params.m_bForceSingleGPU ? "True" : "False") << endl;
    fout << "ForcedGpuIndex=" << params.m_uiForcedGpuIndex << endl;

    for (EnvVarMap::const_iterator it = params.m_mapEnvVars.begin(); it != params.m_mapEnvVars.end(); ++it)
    {
        fout << "EnvVar=" << (it->first).asUTF8CharArray() << "=" << (it->second).asUTF8CharArray() << endl;
    }

    if (params.m_bTestMode)
    {
        fout << "TestMode=" << (params.m_bTestMode ? "True" : "False") << endl;
    }

    fout.close();
}

bool FileUtils::GetProfilerBinaryPath(gtString& outPath, bool& is64bit)
{
    gtString profileAgent;
    gtString traceAgent;
    gtString kernelAgent;
    profileAgent.fromASCIIString(CL_PROFILE_AGENT_DLL);
    traceAgent.fromASCIIString(CL_TRACE_AGENT_DLL);
    kernelAgent.fromASCIIString(CL_SUB_KERNEL_PROFILE_AGENT_DLL);

    gtString agentValue;
    osGetCurrentProcessEnvVariableValue(L"CL_AGENT", agentValue);

    if (!agentValue.isEmpty())
    {
        // we could have multiple agents, find the right one.
        gtString agentToken;
        gtStringTokenizer agentNameTokenizer(agentValue, L",");

        while (agentNameTokenizer.getNextToken(agentToken))
        {
            // find any of our agents
            if (agentToken.find(profileAgent) != -1 ||
                agentToken.find(traceAgent)   != -1 ||
                agentToken.find(kernelAgent)  != -1)
            {
                osFilePath filePath;
                filePath.setFullPathFromString(agentToken);
                // extract path from CL_AGENT
                outPath = filePath.fileDirectoryAsString();
                outPath.append(osFilePath::osPathSeparator);

                if (agentToken.find(L"x64") != -1)
                {
                    is64bit = true;
                }
                else
                {
                    is64bit = false;
                }

                return true;
            }
        }
    }

    outPath = L"";
    return false;
}

bool FileUtils::GetParametersFromFile(Parameters& params)
{
    std::ifstream fin;
    fin.open(GetTempFile().c_str());

    bool bSave_default = false;

    if (fin.is_open())
    {
        try
        {
            while (!fin.eof())
            {
                std::string utf8str;
                getline(fin, utf8str);

                size_t idx = utf8str.find("=");
                std::string opStr = utf8str.substr(0, idx);
                std::string valStr = utf8str.substr(idx + 1);

                if (opStr.find("CmdArgs") != std::string::npos)
                {
                    gtString realStr;
                    realStr.fromUtf8String(valStr);
                    params.m_strCmdArgs = realStr;
                }
                else if (opStr.find("WorkingDir") != std::string::npos)
                {
                    gtString realStr;
                    realStr.fromUtf8String(valStr);
                    params.m_strWorkingDir = realStr;
                }
                else if (opStr.find("VersionMajor") != std::string::npos)
                {
                    StringUtils::Parse(valStr, params.m_uiVersionMajor);
                }
                else if (opStr.find("VersionMinor") != std::string::npos)
                {
                    StringUtils::Parse(valStr, params.m_uiVersionMinor);
                }
                else if (opStr.find("OutputFile") != std::string::npos)
                {
                    params.m_strOutputFile = valStr;

                    // Allow the string ___PID___ in the output file name to be replaced by a string
                    // containing the PID and a unique timestamp. This allows for the following scenario
                    // used by the server BU:  a server farm is executing many individual processes, all of which
                    // use OpenCL. In order to profile all of these processes easily, they will use the CL_AGENT approach.
                    // The one drawback of the CL_AGENT approach is that all processes by default will use the same output
                    // file.  With this change each output file can be unique (using pid and a timestamp).  They can later
                    // process the many (tens/hundreds/thousands/???) output files to analyze the overall workload.  To use
                    // this, you can modify the default .spdata/tmp.spdata data file (the "OutputFile=" line) that is used
                    // when profiling using CL_AGENT.  You can also use this by manually including the ___PID___ string in
                    // the argument passed to the -o (--outputfile) switch in CodeXLGpuProfiler.
                    size_t pidPos = params.m_strOutputFile.find("___PID___");

                    if (pidPos != string::npos)
                    {
                        osProcessId pid = osGetCurrentProcessId();
                        time_t timeElapsed;
                        time(&timeElapsed);
                        params.m_strOutputFile.replace(pidPos, 9, "%d_%d");
                        params.m_strOutputFile = StringUtils::FormatString(params.m_strOutputFile.c_str(), pid, timeElapsed);
                    }
                }
                else if (opStr.find("SessionName") != std::string::npos)
                {
                    params.m_strSessionName = valStr;
                }
                else if (opStr.find("DLLPath") != std::string::npos)
                {
                    gtString realStr;
                    realStr.fromUtf8String(valStr);
                    params.m_strDLLPath = realStr;
                }
                else if (opStr.find("CounterFile") != std::string::npos)
                {
                    params.m_strCounterFile = valStr;
                }
                else if (opStr.find("KernelFile") != std::string::npos)
                {
                    params.m_strKernelFile = valStr;
                }
                else if (opStr.find("APIFilterFile") != std::string::npos)
                {
                    params.m_strAPIFilterFile = valStr;
                }
                else if (opStr.find("Verbose") != std::string::npos)
                {
                    params.m_bVerbose = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("StartDisabled") != std::string::npos)
                {
                    params.m_bStartDisabled = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("OutputIL") != std::string::npos)
                {
                    params.m_bOutputIL = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("OutputHSAIL") != std::string::npos)
                {
                    params.m_bOutputHSAIL = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("OutputISA") != std::string::npos)
                {
                    params.m_bOutputISA = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("OutputASM") != std::string::npos)
                {
                    params.m_bOutputASM = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("OutputCL") != std::string::npos)
                {
                    params.m_bOutputCL = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "PerfCounter")
                {
                    params.m_bPerfCounter = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "Trace")
                {
                    params.m_bTrace = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "HSATrace")
                {
                    params.m_bHSATrace = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "HSAPMC")
                {
                    params.m_bHSAPMC = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "SinglePassPMC")
                {
                    params.m_bForceSinglePassPMC = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "GPUTimePMC")
                {
                    params.m_bGPUTimePMC = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("TimeOut") != std::string::npos)
                {
                    params.m_bTimeOutBasedOutput = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("QueryRetStat") != std::string::npos)
                {
                    params.m_bQueryRetStat = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("CollapseClGetEventInfo") != std::string::npos)
                {
                    params.m_bCollapseClGetEventInfo = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("Interval") != std::string::npos)
                {
                    bool ret = StringUtils::Parse(valStr, params.m_uiTimeOutInterval);

                    if (!ret)
                    {
                        // failed to parse, assign to default interval
                        Log(logWARNING, "Failed to parse parameter file.\n");
                        params.m_uiTimeOutInterval = DEFAULT_TIMEOUT_INTERVAL;
                    }
                }
                else if (opStr.find("Separator") != std::string::npos)
                {
                    params.m_cOutputSeparator = valStr[0];
                }
                else if (opStr.find("TestMode") != std::string::npos)
                {
                    params.m_bTestMode = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("UserTimerIsUsed") != std::string::npos)
                {
                    params.m_bUserTimer = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("UserTimerDLLFileName") != std::string::npos)
                {
                    params.m_strTimerDLLFile = valStr;
                }
                else if (opStr.find("UserTimerFunctionName") != std::string::npos)
                {
                    params.m_strUserTimerFn = valStr;
                }
                else if (opStr.find("UserTimerInitFunctionName") != std::string::npos)
                {
                    params.m_strUserTimerInitFn = valStr;
                }
                else if (opStr.find("UserTimerDestroyFunctionName") != std::string::npos)
                {
                    params.m_strUserTimerDestroyFn = valStr;
                }
                else if (opStr.find("UserPMCLibPath") != std::string::npos)
                {
                    params.m_strUserPMCLibPath = valStr;
                }
                else if (opStr.find("UserPMC") != std::string::npos)
                {
                    params.m_bUserPMC = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("CompatibilityMode") != std::string::npos)
                {
                    params.m_bCompatibilityMode = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("StackTrace") != std::string::npos)
                {
                    params.m_bStackTrace = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("Occupancy") != std::string::npos)
                {
                    params.m_bKernelOccupancy = (valStr.find("True") != std::string::npos);
                }
                else if (opStr == "MaxNumOfAPICalls")
                {
                    bool ret = StringUtils::Parse(valStr, params.m_uiMaxNumOfAPICalls);

                    if (!ret)
                    {
                        // failed to retrieve max num of API calls from params file, generate a default value
                        break;
                    }
                }
                else if (opStr == "MaxKernels")
                {
                    bool ret = StringUtils::Parse(valStr, params.m_uiMaxKernels);

                    if (!ret)
                    {
                        // failed to retrieve max num of kernels from params file, generate a default value
                        break;
                    }
                }
                else if (opStr.find("GMTrace") != std::string::npos)
                {
                    params.m_bGMTrace = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("FullEnvBlock") != std::string::npos)
                {
                    params.m_bFullEnvBlock = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("EnvVar") != std::string::npos)
                {
                    idx = valStr.find("=");

                    if (idx != std::string::npos)
                    {
                        std::string envVarNameStr = valStr.substr(0, idx);
                        std::string envVarValStr = valStr.substr(idx + 1);
                        gtString realNameStr;
                        realNameStr.fromUtf8String(envVarNameStr);
                        gtString realValStr;
                        realValStr.fromUtf8String(envVarValStr);
                        params.m_mapEnvVars[realNameStr] = realValStr;
                    }
                    else
                    {
                        gtString realStr;
                        realStr.fromUtf8String(valStr);
                        params.m_mapEnvVars[realStr] = L"";
                    }
                }
                else if (opStr.find("ForceSingleGPU") != std::string::npos)
                {
                    params.m_bForceSingleGPU = (valStr.find("True") != std::string::npos);
                }
                else if (opStr.find("ForcedGpuIndex") != std::string::npos)
                {
                    bool ret = StringUtils::Parse(valStr, params.m_uiForcedGpuIndex);

                    if (!ret)
                    {
                        // failed to parse, don't force single GPU
                        Log(logWARNING, "Failed to parse parameter file.\n");
                        params.m_bForceSingleGPU = false;
                    }
                }
            }
        }
        catch (...)
        {
            // use default value
            Log(logWARNING, "Error reading tmp file\n");

            params.m_strOutputFile.clear();
            bool is64bit;
            GetProfilerBinaryPath(params.m_strDLLPath, is64bit);
            params.m_strCounterFile = "";
            params.m_bVerbose = false;
            params.m_bOutputIL = false;
            params.m_bOutputISA = false;
            params.m_bOutputCL = false;
            params.m_bOutputHSAIL = false;
            params.m_bOutputASM = false;
            params.m_cOutputSeparator = LocaleSetting::GetListSeparator();
        }

        fin.close();
    }
    else
    {
        bSave_default = true;
    }

    if (bSave_default)
    {
        params.m_strOutputFile.clear();
        bool is64bit;
        GetProfilerBinaryPath(params.m_strDLLPath, is64bit);
        params.m_strCounterFile = "";
        params.m_bVerbose = false;
        params.m_bOutputIL = false;
        params.m_bOutputISA = false;
        params.m_bOutputHSAIL = false;
        params.m_bOutputCL = false;
        params.m_bOutputASM = false;
        params.m_cOutputSeparator = LocaleSetting::GetListSeparator();

        // Write config out
        // It's helpful when user use agent without CodeXLGpuProfiler,
        // they can config agent by modifying the file.
        PassParametersByFile(params);
        return false;
    }

    return true;
}

bool FileUtils::ReadKernelListFile(Parameters& params, bool doOutputError)
{
    bool retVal = false;

    if (!params.m_strKernelFile.empty())
    {
        retVal = FileUtils::ReadFile(params.m_strKernelFile, params.m_kernelFilterList, true, false);

        if (!retVal && doOutputError)
        {
            std::cout << "Unable to read kernel list file: " << params.m_strKernelFile << ". All kernels will be profiled." << endl;
        }
    }

    return retVal;
}

void FileUtils::DeleteTmpFile()
{
    remove(GetTempFile().c_str());
}


bool FileUtils::GetWorkingDirectory(const std::string& strFilename, std::string& strOutputDir)
{
    strOutputDir.clear();

    if (strFilename.empty())
    {
        return false;
    }

    if (strFilename.find('\\') == string::npos && strFilename.find('/') == string::npos)
    {
        // use current directory
        strOutputDir = "./";
        return true;
    }

    size_t length = strFilename.length();
    strOutputDir = strFilename;
    std::replace(strOutputDir.begin(), strOutputDir.end(), '\\', '/');

    if (length > 0)
    {
        for (size_t i = length - 1; i > 0; i--)
        {
            if (strOutputDir[i] == '/')
            {
                strOutputDir.erase(strOutputDir.begin() + i + 1, strOutputDir.end());

                return true;
            }
        }

        return false;
    }

    return false;
}

bool FileUtils::WriteFile(const std::string& strFilename, const std::string& strMessage)
{
    std::wstring convertedFileName;
    StringUtils::Utf8StringToWideString(strFilename, convertedFileName);

    return WriteFile(convertedFileName, strMessage);
}

bool FileUtils::WriteFile(const std::wstring& strFilename, const std::string& strMessage)
{
    std::ofstream oFile;

#ifdef _WIN32
    oFile.open(strFilename.c_str());
#else
    std::string convertedName;
    StringUtils::WideStringToUtf8String(strFilename, convertedName);
    oFile.open(convertedName.c_str());
#endif

    if (oFile.fail())
    {
        // failed to open the file
        std::wcout << "Failed to write file: " << strFilename << std::endl;
        std::wcout << "Please make sure you have write permission in the path you specified.\n";
        return false;
    }

    oFile << strMessage << std::endl;

    oFile.close();

    std::wcout << "Writing to file: " << strFilename << std::endl;

    return true;
}

bool FileUtils::WriteFile(const std::string& strFilename, const std::vector<std::string>& vLines)
{
    std::wstring convertedFileName;
    StringUtils::Utf8StringToWideString(strFilename, convertedFileName);

    return WriteFile(convertedFileName, vLines);
}

bool FileUtils::WriteFile(const std::wstring& strFilename, const std::vector<std::string>& vLines)
{
    std::ofstream oFile;

#ifdef _WIN32
    oFile.open(strFilename.c_str());
#else
    std::string convertedName;
    StringUtils::WideStringToUtf8String(strFilename, convertedName);
    oFile.open(convertedName.c_str());
#endif

    if (oFile.fail())
    {
        // failed to open the file
        std::wcout << "Failed to write file: " << strFilename << std::endl;
        std::wcout << "Please make sure you have write permission in the path you specified.\n";
        return false;
    }

    for (std::vector<std::string>::const_iterator it = vLines.begin(); it != vLines.end(); ++it)
    {
        oFile << *it << std::endl;
    }

    oFile.close();
    return true;
}

template<typename InsertionFunc>
bool DoReadFile(const std::wstring& strFilename, InsertionFunc insertionFunc, bool bSkipEmptyLines, bool bOutputError)
{
    std::ifstream iFile;

#ifdef _WIN32
    iFile.open(strFilename.c_str());
#else
    std::string convertedName;
    StringUtils::WideStringToUtf8String(strFilename, convertedName);
    iFile.open(convertedName.c_str());
#endif

    if (iFile.fail())
    {
        // failed to open the file
        if (bOutputError)
        {
            std::wcout << "Failed to open file: " << strFilename << std::endl;
        }

        return false;
    }

    std::string strLine;

    do
    {
        std::getline(iFile, strLine);

        if (bSkipEmptyLines)
        {
            string trimmed = StringUtils::Trim(strLine);

            if (trimmed.empty())
            {
                continue;
            }
        }

        insertionFunc(strLine);
    }
    while (!iFile.eof());

    iFile.close();

    return true;
}

bool FileUtils::ReadFile(const std::string& strFilename, std::string& strOut, bool bOutputError)
{
    std::wstring convertedFileName;
    StringUtils::Utf8StringToWideString(strFilename, convertedFileName);

    return ReadFile(convertedFileName, strOut, bOutputError);
}

bool FileUtils::ReadFile(const std::wstring& strFilename, std::string& strOut, bool bOutputError)
{
    strOut.clear();
    return DoReadFile(strFilename, [&](const std::string & strLine) { strOut += strLine + '\n'; }, true, bOutputError);
}

bool FileUtils::ReadFile(const std::string& strFilename, std::vector<std::string>& lines, bool bSkipEmptyLines, bool bOutputError)
{
    std::wstring convertedFileName;
    StringUtils::Utf8StringToWideString(strFilename, convertedFileName);

    return ReadFile(convertedFileName, lines, bSkipEmptyLines, bOutputError);
}

bool FileUtils::ReadFile(const std::wstring& strFilename, std::vector<std::string>& lines, bool bSkipEmptyLines, bool bOutputError)
{
    lines.clear();
    return DoReadFile(strFilename, [&](const std::string & strLine) { lines.push_back(strLine); }, bSkipEmptyLines, bOutputError);
}

bool FileUtils::ReadFile(const std::string& strFilename, std::unordered_set<std::string>& lines, bool bSkipEmptyLines, bool bOutputError)
{
    std::wstring convertedFileName;
    StringUtils::Utf8StringToWideString(strFilename, convertedFileName);

    return ReadFile(convertedFileName, lines, bSkipEmptyLines, bOutputError);
}

bool FileUtils::ReadFile(const std::wstring& strFilename, std::unordered_set<std::string>& lines, bool bSkipEmptyLines, bool bOutputError)
{
    lines.clear();
    return DoReadFile(strFilename, [&](const std::string & strLine) { lines.insert(strLine); }, bSkipEmptyLines, bOutputError);
}

bool FileUtils::FileExist(const std::string& fileName)
{
    std::fstream fin;

    //this will fail if more capabilities to read the
    //contents of the file is required (e.g. \private\...)
    fin.open(fileName.c_str(), std::ios::in);

    if (fin.is_open())
    {
        fin.close();
        return true;
    }

    fin.close();

    return false;
}

std::string FileUtils::GetExeName()
{
    std::string str = GetExeFullPath();
    string::size_type last = (int)str.find_last_of("/\\");
    return str.substr(last + 1);
}

std::string FileUtils::GetExePath()
{
    std::string str = GetExeFullPath();
    string::size_type last = (int)str.find_last_of("/\\");

    return str.substr(0, last);
}

gtString FileUtils::GetExePathAsUnicode()
{
    gtString str = GetExeFullPathAsUnicode();
    int lastPos = str.reverseFind(L"/");

    if (lastPos != -1)
    {
        str.truncate(0, lastPos);
    }

    return str;
}

gtString FileUtils::GetExeFullPathAsUnicode()
{
    gtString retVal;
#ifdef _WIN32
    wchar_t wideBuffer[SP_MAX_PATH];

    if (GetModuleFileNameW(NULL, wideBuffer, SP_MAX_PATH))
    {
        std::wstring widePath(wideBuffer);

        std::replace(widePath.begin(), widePath.end(), '\\', '/');
        retVal = widePath.c_str();
    }

#else
    char buffer[SP_MAX_PATH];
    ssize_t len;

    if (!((len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1)) == -1))
    {
        buffer[len] = '\0';
        retVal.fromASCIIString(buffer);
    }

#endif
    return retVal;
}

std::string FileUtils::GetExeFullPath()
{
#ifdef _WIN32

    wchar_t wideBuffer[SP_MAX_PATH];

    if (!GetModuleFileNameW(NULL, wideBuffer, SP_MAX_PATH))
    {
        return "";
    }

    std::wstring widePath(wideBuffer);
    std::string ret;
    int errCode = StringUtils::WideStringToUtf8String(widePath, ret);

    if (errCode)
    {
        return "";
    }

    std::replace(ret.begin(), ret.end(), '\\', '/');
    return ret;
#else
    char buffer[SP_MAX_PATH];
    ssize_t len;

    if ((len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1)) == -1)
    {
        return "";
    }

    buffer[len] = '\0';
    return std::string(buffer);
#endif
}

std::string FileUtils::GetDefaultOutputPath()
{
    char szDefaultOutputPath[ SP_MAX_PATH ];
#ifdef _WIN32
    SP_TODO("revisit use of SHGetSpecialFolderPathA for Unicode support")
    SHGetSpecialFolderPathA(0, szDefaultOutputPath, CSIDL_PERSONAL, false);
    strcat_s(szDefaultOutputPath, SP_MAX_PATH, "\\CodeXL\\");

    SP_TODO("revisit use of GetFileAttributesA for Unicode support")
    DWORD dwFileAttr = GetFileAttributesA(szDefaultOutputPath);

    // check if directory exists
    if (dwFileAttr == INVALID_FILE_ATTRIBUTES)
    {
        //if not, try to create it
        SP_TODO("revisit use of CreateDirectoryA for Unicode support")

        if (!CreateDirectoryA(szDefaultOutputPath, NULL))
        {
            std::cout << "Failed to create output directory: " << szDefaultOutputPath << std::endl;
            szDefaultOutputPath[0] = '\0';
        }
    }

#else
    const char* envName = "HOME";
    char* szHomePath;
    szHomePath = getenv(envName);
    strcpy(szDefaultOutputPath, szHomePath);
    strcat(szDefaultOutputPath, "/");
#endif
    return std::string(szDefaultOutputPath);
}

std::string FileUtils::GetDefaultProfileOutputFile()
{
    return GetDefaultOutputPath() + "Session1.csv";
}

std::string FileUtils::GetDefaultOccupancyOutputFile()
{
    return GetDefaultOutputPath() + "Session1.occupancy";
}

std::string FileUtils::GetDefaultPerfMarkerOutputFile()
{
    return GetDefaultOutputPath() + "apitrace" PERFMARKER_EXT;
}

std::string FileUtils::GetDefaultTraceOutputFile()
{
    return GetDefaultOutputPath() + "apitrace.atp";
}

std::string FileUtils::GetDefaultSubKernelProfileOutputFile()
{
    return GetDefaultOutputPath() + "subkernelprofile.csv";
}

std::string FileUtils::GetDefaultThreadTraceOutputDir()
{
    return GetDefaultOutputPath() + "clthreadtrace";
}

bool FileUtils::MergeFiles(const std::wstring& strNewFileName, const std::wstring& strFileName1,
                           const std::wstring& strFileName2, const std::string& strFileHeader)
{

    std::string file1Contents;
    std::string file2Contents;

    if (ReadFile(strFileName1, file1Contents) && ReadFile(strFileName2, file2Contents))
    {
        std::stringstream ss;

        if (strFileHeader != "")
        {
            ss << strFileHeader << std::endl;
        }

        ss << file1Contents << file2Contents;

        return WriteFile(strNewFileName, ss.str());
    }

    return false;
}

bool FileUtils::MergeTmpTraceFiles(const std::string& strOutputFile,
                                   const std::string& strTmpFilesDirPath,
                                   const std::string& strFilePrefix,
                                   const std::string& szFileExt,
                                   const char* szHeader,
                                   MergeSummaryType mergeSummaryType)
{
    std::wstring convertedDirPath;
    std::wstring convertedPrefix;
    std::wstring convertedExt;
    StringUtils::Utf8StringToWideString(strTmpFilesDirPath, convertedDirPath);
    StringUtils::Utf8StringToWideString(strFilePrefix, convertedPrefix);
    StringUtils::Utf8StringToWideString(szFileExt, convertedExt);
    gtString dirPathStr(convertedDirPath.c_str()), prefixStr(convertedPrefix.c_str()), extStr(convertedExt.c_str());

    return MergeTmpTraceFiles(strOutputFile, dirPathStr, prefixStr, extStr, szHeader, mergeSummaryType);
}

bool FileUtils::MergeTmpTraceFiles(SP_outStream& sout,
                                   const std::string& strTmpFilesDirPath,
                                   const std::string& strFilePrefix,
                                   const std::string& szFileExt,
                                   const char* szHeader,
                                   MergeSummaryType mergeSummaryType)
{
    std::wstring convertedDirPath;
    std::wstring convertedPrefix;
    std::wstring convertedExt;
    StringUtils::Utf8StringToWideString(strTmpFilesDirPath, convertedDirPath);
    StringUtils::Utf8StringToWideString(strFilePrefix, convertedPrefix);
    StringUtils::Utf8StringToWideString(szFileExt, convertedExt);
    gtString dirPathStr(convertedDirPath.c_str()), prefixStr(convertedPrefix.c_str()), extStr(convertedExt.c_str());

    return MergeTmpTraceFiles(sout, dirPathStr, prefixStr, extStr, szHeader, mergeSummaryType);

}

bool FileUtils::MergeTmpTraceFiles(const string& strOutputFile,
                                   const gtString& strTmpFilesDirPath,
                                   const gtString& strFilePrefix,
                                   const gtString& szFileExt,
                                   const char* szHeader,
                                   MergeSummaryType mergeSummaryType)
{
    SP_stringStream ss;

    if (!MergeTmpTraceFiles(ss, strTmpFilesDirPath, strFilePrefix, szFileExt, szHeader, mergeSummaryType))
    {
        return false;
    }
    else
    {
        if (ss.str().length() > 0)
        {
            // don't generate a file unless temp files are found and merged
            SP_fileStream fout;
            fout.open(strOutputFile.c_str());

            if (fout.fail())
            {
                // failed to open the file
                std::cout << "Failed to open/create file: " << strOutputFile << std::endl;
                return false;
            }
            else
            {
                fout << ss.str().c_str();
                fout.close();
                return true;
            }
        }

        return true;
    }
}

bool FileUtils::MergeTmpTraceFiles(SP_outStream& sout,
                                   const gtString& strTmpFilesDirPath,
                                   const gtString& strFilePrefix,
                                   const gtString& szFileExt,
                                   const char* szHeader,
                                   MergeSummaryType mergeSummaryType)
{
    gtList<osFilePath> files;
    osDirectory tempFileDirectory(strTmpFilesDirPath);

    gtString finalPrefix = strFilePrefix;
    finalPrefix.append(L"*");

    bool ret = tempFileDirectory.getContainedFilePaths(finalPrefix, osDirectory::SORT_BY_NAME_ASCENDING, files);

    if (files.size() == 0)
    {
        Log(logWARNING, "No temp files found under %s. Nothing will be merged.\n", strTmpFilesDirPath.asUTF8CharArray());
        return false;
    }

    if (ret)
    {
        bool bWriteHeader = false;

        std::wstring strFullFilePath;
        std::string strFileContent;
        osFilePath strFileAsFilePath;
        gtString strFileName;
        std::wstring strFile;
        std::wstring strTid;
        std::wstring strExt;
        strFileContent.empty();

        int cumulativeAPICount = 0;
        std::string cumulativeStrFileContent;

        for (gtList<osFilePath>::iterator it = files.begin(); it != files.end(); it++)
        {
            strFileAsFilePath = (*it);
            strFileAsFilePath.getFileNameAndExtension(strFileName);
            strFile = strFileName.asCharArray();
            strFullFilePath = strFileAsFilePath.asString().asCharArray();

            // extract thread id from file name
            size_t found;
            found = strFile.find_first_of(L".");

            if (found != string::npos)
            {
                std::wstring strPidTid = strFile.substr(0, found);
                strExt = strFile.substr(found);
                found = strPidTid.find_first_of(L"_");

                if (found != string::npos)
                {
                    strTid = strPidTid.substr(found + 1);
                }
                else
                {
                    // wrong file name? ignore this one
                    Log(logWARNING, "Incorrect file name: %s\n", strFile.c_str());
                    continue;
                }
            }
            else
            {
                // wrong file name? ignore this one
                Log(logWARNING, "Incorrect file name: %s\n", strFile.c_str());
                continue;
            }

            if (strExt == szFileExt.asCharArray())
            {
                if (!bWriteHeader)
                {
                    if (szHeader != NULL)
                    {
                        sout << szHeader << endl;
                    }

                    bWriteHeader = true;
                }

                ret = ReadFile(strFullFilePath, strFileContent);

                if (!ret)
                {
                    Log(logWARNING, "Error reading file: %s\n", strFullFilePath.c_str());
                    continue;
                }
                else
                {
                    // remove tmp file
                    gtString fileToRemoveName(strFullFilePath.c_str());
                    osFilePath filePathToRemove(fileToRemoveName);
                    osFile fileToRemove(filePathToRemove);
                    fileToRemove.deleteFile();
                }

                int apiCount = StringUtils::GetNumLines(strFileContent);

                if (MergeSummaryType_CumulativeNumEntries == mergeSummaryType)
                {
                    cumulativeAPICount += apiCount;
                    cumulativeStrFileContent += strFileContent;
                }
                else if (MergeSummaryType_TidAndNumEntries == mergeSummaryType)
                {
                    std::string strConvertedTID;
                    StringUtils::WideStringToUtf8String(strTid, strConvertedTID);
                    sout << strConvertedTID.c_str() << endl << apiCount << endl;
                }

                if (MergeSummaryType_CumulativeNumEntries != mergeSummaryType)
                {
                    sout << strFileContent.c_str();
                }
            }
        }

        if (MergeSummaryType_CumulativeNumEntries == mergeSummaryType && 0 < cumulativeAPICount)
        {
            sout << cumulativeAPICount << endl;
            sout << cumulativeStrFileContent.c_str();
        }

        return true;
    }
    else
    {
        Log(logERROR, "Failed to open directory: %s\n", strTmpFilesDirPath.asCharArray());
        return ret;
    }
}


std::string FileUtils::ToAbsPath(const std::string& input)
{
#ifdef _WIN32
    size_t idx0 = input.find(":\\");
    size_t idx1 = input.find(":/");

    // need to check for // in addition to \\ because boost command line parser will return UNC paths like //BDC-SUPERMAN/c$/foo/bar.csv instead of \\BDC-SUPERMAN\c$\foo\bar.csv
    bool bStartWithSlashslash = (input[0] == '\\' && input[1] == '\\') || (input[0] == '/' && input[1] == '/');  // e.g. \\BDC-SUPERMAN\c$\foo\bar.csv

    if (idx0 != string::npos || idx1 != string::npos || bStartWithSlashslash)
    {
        // it's already an abs path
        return input;
    }
    else
    {
        char slash = '\\';
        char szPath[FILENAME_MAX];
        GetCurrentDir(szPath, FILENAME_MAX);
        return string(szPath) + slash + input;
    }

#else

    if (input[0] == '/')
    {
        return input;
    }
    else
    {
        char slash = '/';
        // check ~ and .
        char* szHomepath;
        const char* envName = "HOME";
        szHomepath = getenv(envName);
        string strHomePath = string(szHomepath);
        string ret = input;
        char szPath[SP_MAX_PATH];
        char* szRet = GetCurrentDir(szPath, SP_MAX_PATH);

        if (szRet == NULL)
        {
            Log(logERROR, "ToAbsPath():Failed to get currect directory.\n");
            return "";
        }

        if (ReplaceTilde(strHomePath, ret))
        {
            return ret;
        }
        else if (ret[0] == '.')
        {
            return string(szPath) + ret.substr(1, ret.length() - 1);
        }
        else
        {
            return string(szPath) + slash + input;
        }
    }

#endif
}

#ifdef _WIN32

bool FileUtils::GetFilesUnderDir(const std::string& strDirPath, std::vector<std::string>& filesOut, std::string filter)
{
    SP_TODO("revisit use of WIN32_FIND_DATAA for Unicode support")
    WIN32_FIND_DATAA ffd;

    string strDir = strDirPath + "\\*";

    SP_TODO("revisit use of FindFirstFileA for Unicode support")
    HANDLE hFind = FindFirstFileA(strDir.c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        return false;
    }

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // it's a sub dir
        }
        else
        {
            string fname = ffd.cFileName;

            if (filter != "")
            {
                if (fname.find(filter) != std::string::npos)
                {
                    // add to output
                    filesOut.push_back(fname);
                }
            }
            else
            {
                filesOut.push_back(fname);
            }
        }
    }

    SP_TODO("revisit use of FindNextFileA for Unicode support")

    while (FindNextFileA(hFind, &ffd) != 0);

    return true;
}

#else

// replace ~ with $HOME
bool FileUtils::ReplaceTilde(string& strHome, string& strInOut)
{
    if (strInOut[0] == '~')
    {
        strInOut = strHome + strInOut.substr(1, strInOut.length() - 1);
        return true;
    }
    else
    {
        return false;
    }
}

bool FileUtils::GetFilesUnderDir(const std::string& strDirPath, std::vector<std::string>& filesOut, std::string filter)
{
    DIR* dp;
    struct dirent* dirp;

    if ((dp = opendir(strDirPath.c_str())) == NULL)
    {
        return false;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        string fname = string(dirp->d_name);

        if (filter != "")
        {
            if (fname.find(filter) != std::string::npos)
            {
                // add to output
                filesOut.push_back(fname);
            }
        }
        else
        {
            filesOut.push_back(fname);
        }
    }

    closedir(dp);
    return true;
}

#endif


void FileUtils::LoadAPIRulesConfig(const string& strFileName, AnalyzeOps& op)
{
    if (!strFileName.empty())
    {
        std::ifstream fin;
        fin.open(strFileName.c_str());

        if (fin.is_open())
        {
            while (!fin.eof())
            {
                char szBuf[SP_MAX_PATH];
                fin.getline(szBuf, SP_MAX_PATH);
                std::string str = szBuf;
                size_t idx = str.find("=");
                std::string opStr = str.substr(0, idx);
                std::string valStr = str.substr(idx + 1);

                size_t idx1 = opStr.find("APITrace.APIRules.");

                if (idx1 != std::string::npos)
                {
                    // 18 = len("APITrace.APIRules.")
                    string ruleName = opStr.substr(18);
                    op.analyzerMap[ruleName] = valStr == "True";
                }
            }

            fin.close();
        }
    }

}

std::string FileUtils::GetBaseFileName(const std::string& strFileName)
{
    size_t found = strFileName.find_last_of(".");

    if (found > 0)
    {
        return strFileName.substr(0, found);
    }
    else if (found == 0)
    {
        // if found == 0, it is a valid name for Linux
#if defined(_LINUX) || defined(LINUX)
        return strFileName;
#else
        return string("");
#endif
    }
    else
    {
        return strFileName;
    }
}

std::string FileUtils::GetFileExtension(const std::string& strFileName)
{
    size_t found = strFileName.find_last_of("/\\");
    string fileNameWithoutPath = strFileName;

    if (found != string::npos)
    {
        if (found == strFileName.length() - 1)
        {
            // Not a valid file name
            return string("");
        }

        fileNameWithoutPath = strFileName.substr(found + 1);
    }

    found = fileNameWithoutPath.find_last_of(".");

    if (found > 0 && found != fileNameWithoutPath.length() - 1)
    {
        return fileNameWithoutPath.substr(found + 1);
    }
    else
    {
        return string("");
    }
}

void FileUtils::RemoveFragFiles(const char* szOutputPath)
{
    // delete files in tmp directory
    string path;

    if (szOutputPath != NULL)
    {
        path.assign(szOutputPath);
    }
    else
    {
        path = GetTempFragFilePath();
    }

    vector<string> files;
    GetFilesUnderDir(path, files);

    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
    {
        if ((*it).find(TMP_TIME_STAMP_EXT)           != string::npos ||
            (*it).find(TMP_GPU_TIME_STAMP_RAW_EXT)   != string::npos ||
            (*it).find(TMP_KERNEL_TIME_STAMP_EXT)    != string::npos ||
            (*it).find(TMP_TRACE_EXT)                != string::npos ||
            (*it).find(TMP_TRACE_STACK_EXT)          != string::npos ||
            (*it).find(TMP_OCCUPANCY_EXT)            != string::npos)
        {
            std::remove((path + *it).c_str());
        }
    }
}
