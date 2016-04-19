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

private:
    gtString  m_commandLineArgs;
    gtString  m_command;
    gtString  m_launchApp;
    gtString  m_launchAppArgs;

    gtString  m_profileConfig;
    gtString  m_customFile;

    gtString  m_outputFile;
    gtString  m_rawEventsString;

    gtString  m_workingDir;

    gtList<gtUInt64>    m_rawEventsList; // UNUSED. will be used with -e option
    gtVector<int>       m_pidsList;

    gtUInt64  m_coreAffinityMask;

    int       m_debugLogLevel;
    int       m_tbpSamplingInterval;      // TBP sampling interval in milli-secons
    int       m_profileDuration;          // Profile Duration in Seconds
    int       m_startDelay;               // start delay duration in Seconds

    int       m_unwindInterval;
    int       m_unwindDepth;
    CpuProfileCssScope  m_cssScope;
    bool      m_cssSupportFpo;

    bool      m_isPrintHelp;
    bool      m_isPrintVersion;
    bool      m_isSWP;
    bool      m_isAttach;
    bool      m_isCSSEnabled;
    bool      m_cssWithDefaultValues;
    bool      m_isPredefinedProfile;

    bool      m_profileChildren;
    bool      m_terminateLaunchApp;

    // Report Options
    gtString  m_sectionsToReport;
    gtString  m_inputFile;
    gtString  m_outputFileFormat; // csv of text
    gtString  m_viewConfigName;
    gtString  m_debugSymbolPaths;
    gtString  m_symbolServerDirs;
    gtString  m_cachePath;

    int       m_sortEventIndex;

    bool      m_ignoreSystemModules;
    bool      m_showPercentage;
    bool      m_reportByNuma;
    bool      m_reportByCore;
    bool      m_enableCache;

    bool InitializeArgs(int nbrArgs, wchar_t* args[]);
};

#endif // _PARSE_ARGS_H_