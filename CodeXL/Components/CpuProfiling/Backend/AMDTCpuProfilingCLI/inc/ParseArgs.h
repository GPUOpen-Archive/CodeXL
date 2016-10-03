//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ParseArgs.h
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#ifndef _PARSE_ARGS_H_
#define _PARSE_ARGS_H_
#pragma once

#include <cwchar>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTCpuProfilingBackendUtils/Include/CpuProfileDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

//
// codexl-cli  // print usage & exit
// codexl-cli -v // print version & exit
// codexl-cli -h // print help & exit
// codexl-cli classic.exe // report error and exit
// codexl-cli collect -m tbp classic.exe // launch app
// codexl-cli collect -m access -p PID1 // Per Process
// codexl-cli collect -m ibs -a // SWP
//
class ParseArgs
{
public:
    ParseArgs();
    ~ParseArgs() { };

    bool Initialize(int nbrArgs, wchar_t* args[]);
    bool Initialize(int nbrArgs, char* args[]);

    bool IsPrintUsage() const { return m_isPrintHelp; }
    bool IsPrintVersion() const { return m_isPrintVersion; }

    // collect / analyze / report
    gtString GetCommand() const { return m_command; }

    // profile types
    // Is it assess | branch | clu | data_access | ibs |inst_access|l2_access| tbp | custom
    gtString GetProfileConfig() const { return m_profileConfig; }
    bool SetProfileConfig(gtString config);

    gtString GetCustomFile() const { return m_customFile; }
    gtString GetOutputFile();

    gtString GetWorkingDir() const { return m_workingDir; }

    int   GetDebugLogLevel() const { return m_debugLogLevel; }
    bool  IsSystemWide() const { return m_isSWP; }
    bool  IsPerProcess() const { return ! m_isSWP; }
    bool  IsAttach() const { return m_isAttach; }
    bool  IsProfileChildren() const { return m_profileChildren; }
    bool  IsTerminateApp() const { return m_terminateLaunchApp; }

    bool  IsCSSEnabled() const { return m_isCSSEnabled; }
    bool  IsCSSWithDefaultValues() const { return m_cssWithDefaultValues; }
    int   GetUnwindInterval() const { return m_unwindInterval; }
    int   GetUnwindDepth() const { return m_unwindDepth; }
    CpuProfileCssScope GetCSSScope() const { return m_cssScope; }
    int   IsCSSWithFpoSupport() const { return m_cssSupportFpo; }
    void  DisableCSS() { m_isCSSEnabled = m_cssWithDefaultValues = false; }

    int GetTbpSamplingInterval() const { return m_tbpSamplingInterval; }
    int GetProfileDuration() const { return m_profileDuration; }
    int GetStartDelay() const { return m_startDelay; }
    gtVector<gtString> GetRawEventString() const { return m_rawEventsStringVec; }

    gtUInt64 GetCoreAffinityMask() const { return m_coreAffinityMask; }

    gtString& GetLaunchApp() { return m_launchApp; }
    gtString GetLaunchAppArgs() { return m_launchAppArgs; }

    gtVector<int> GetPidsList() const { return m_pidsList; }

    //
    // REPORT options specific member functions
    //

    gtString GetSectionsToReport() const { return m_sectionsToReport; }
    gtString GetInputFile();
    gtString GetViewConfigName() const { return m_viewConfigName; }
    gtString GetOutputFileFormat() const { return m_outputFileFormat; }
    gtString GetDebugSymbolPath() const { return m_debugSymbolPaths; }
    gtString GetSymbolServerPath() const { return m_symbolServerDirs; }
    gtString GetSymbolCachePath() const { return m_cachePath; }

    int GetSortEventIndex() const { return m_sortEventIndex; }

    bool IsIgnoreSystemModules() const { return m_ignoreSystemModules; }
    bool IsReportByNuma() const { return m_reportByNuma; }
    bool IsReportByCore() const { return m_reportByCore; }
    bool IsShowPercentage() const { return m_showPercentage; }
    bool IsEnableCache() const { return m_enableCache; }
    bool IsReportSampleCount() const { return m_printSampleCount; }

private:
    gtString  m_commandLineArgs;
    gtString  m_command;
    gtString  m_launchApp;
    gtString  m_launchAppArgs;

    gtString  m_profileConfig;
    gtString  m_customFile;

    gtString  m_outputFile;
    gtVector<gtString>  m_rawEventsStringVec;

    gtString  m_workingDir;

    gtVector<int>       m_pidsList;

    gtUInt64  m_coreAffinityMask = static_cast<gtUInt64>(-1);

    bool      m_printSampleCount = false;
    int       m_debugLogLevel = 0;
    int       m_tbpSamplingInterval = 0;  // TBP sampling interval in milli-seconds
    int       m_profileDuration = 0;      // Profile Duration in Seconds
    int       m_startDelay = -1;          // start delay duration in Seconds

    int       m_unwindInterval = CP_CSS_DEFAULT_UNWIND_INTERVAL;
    int       m_unwindDepth = CP_CSS_DEFAULT_UNWIND_DEPTH;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    CpuProfileCssScope m_cssScope = CP_CSS_SCOPE_USER;
#else
    CpuProfileCssScope m_cssScope = CP_CSS_SCOPE_UNKNOWN;
#endif

    bool      m_cssSupportFpo = false;

    bool      m_isPrintHelp = false;
    bool      m_isPrintVersion = false;
    bool      m_isSWP = false;
    bool      m_isAttach = false;
    bool      m_isCSSEnabled = false;
    bool      m_cssWithDefaultValues = false;
    bool      m_isPredefinedProfile = false;

    bool      m_profileChildren = false;
    bool      m_terminateLaunchApp = false;

    // Report Options
    gtString  m_sectionsToReport;
    gtString  m_inputFile;
    gtString  m_outputFileFormat = L"csv";
    gtString  m_viewConfigName;
    gtString  m_debugSymbolPaths;
    gtString  m_symbolServerDirs;
    gtString  m_cachePath;

    int       m_sortEventIndex = 0;

    bool      m_ignoreSystemModules = false;
    bool      m_showPercentage = false;
    bool      m_reportByNuma = false;
    bool      m_reportByCore = false;
    bool      m_enableCache = false;

    bool InitializeArgs(int nbrArgs, wchar_t* args[]);
};

#endif // _PARSE_ARGS_H_