//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppParseArgs.h
///
//==================================================================================

#ifndef _PP_PARSE_ARGS_H_
#define _PP_PARSE_ARGS_H_
#pragma once

// Project
#include <PowerProfileCLI.h>


class ppParseArgs
{
public:
    ppParseArgs();
    ~ppParseArgs() { };

    bool Initialize(int nbrArgs, char* args[]);

    bool IsPrintUsage() const { return m_isPrintHelp; }
    bool IsPrintVersion() const { return m_isPrintVersion; }
    bool IsListCounters() const { return m_isListCounters; }
    bool IsProfileCounters() const { return m_hasProfileCounters; }
    AMDTPwrProfileMode GetProfileMode() const { return m_profileMode;}

    int GetProfileType() const { return m_profileType; }
    bool IsProfileOnline() const { return (AMDT_PWR_PROFILE_MODE_ONLINE == m_profileMode); }

    // TODO: Once Offline feature support  is added m_profileMode need to compare
    // with AMDT_PWR_PROFILE_MODE_OFFLINE to know state.
    bool IsProfileOffline() const { return false; }

    int GetSamplingInterval() const { return m_samplingInterval; }
    int GetProfileDuration() const { return m_profileDuration; }

    AMDTUInt64 GetCoreMask() const { return m_coreMask; }
    void SetCoreMask(AMDTUInt64 coreMask) { m_coreMask = coreMask; }

    // Get the device id list
    gtList<AMDTUInt32> GetDeviceIDList() const { return m_deviceIDList; }

    gtList<AMDTUInt32> GetCounterGroupList() const { return m_counterGroupIDList; } // -P 0x1f
    gtList<gtString> GetCounterGroupNameList() const { return m_counterGroupNameList; } // -P cu_power,gpu_power

    gtList<gtString> GetCounterNameList() const { return m_counterNameList; }
    gtList<AMDTUInt32> GetCounterIDList() const { return m_counterIDList; }

    // Target application related functions
    gtString& GetLaunchApp() { return m_launchApp; }
    gtString GetLaunchAppArgs() { return m_launchAppArgs; }

    // Used to set the affinity for the launched application
    AMDTUInt64 GetCoreAffinityMask() const { return m_coreAffinityMask; }
    void SetCoreAffinityMask(AMDTUInt64 coreMask) { m_coreAffinityMask = coreMask; }

    gtString GetWorkingDir() const { return m_workingDir; }
    bool  IsTerminateApp() const { return m_terminateLaunchApp; }

    // Report specific functions
    bool GetOutputFilePath(osFilePath& filePath) { return GetReportFilePath(filePath); } // FIXME

    bool GetReportFilePath(osFilePath& filePath);
    gtString GetOutputFileFormat() const { return m_reportFileFormat; }

    bool ShouldExportToDb() const { return m_exportToDb; }
    gtString GetDbFileOutDir() const { return m_dbFileOutDir; }

    AMDTUInt32 GetReportType() const { return m_reportType; }

    bool IsReportGroupByDevice() const { return m_isGroupByDevice; }
    bool IsSimulateGUI() const { return m_simulateGui; }
    bool IsReportPathSet() const { return m_isReportPathSet; }

private:
    gtString  m_commandLineArgs;

    AMDTPwrProfileMode  m_profileMode;        // ONLINE / OFFLINE mode

    gtList<AMDTUInt32>  m_deviceIDList;    // device id list
    gtList<gtString>    m_deviceNameList;

    gtList<AMDTUInt32>  m_counterGroupIDList;       // HEX mask for predefined counter groups 1,2,4,8..
    gtList<gtString>    m_counterGroupNameList;     // Predefined counter group name list
    gtList<gtString>    m_counterNameList;
    gtList<AMDTUInt32>  m_counterIDList;

    int         m_profileType;
    AMDTUInt64  m_coreMask;  // Core mask for "core" specific attributes
    AMDTUInt64  m_coreAffinityMask;

    int         m_samplingInterval;      // Sampling interval in milli-secons
    int         m_profileDuration;       // Profile Duration in Seconds
    gtList<gtString> m_profileTypeList;

    gtString    m_launchApp;
    gtString    m_launchAppArgs;
    gtString    m_workingDir;

    bool        m_isListCounters;
    bool        m_hasProfileCounters;
    bool        m_terminateLaunchApp;
    bool        m_isPrintHelp;
    bool        m_isPrintVersion;
    bool        m_isGroupByDevice;

    // undocumented option
    bool        m_simulateGui;

    // Output Report Options
    gtString    m_dbFileOutDir;
    gtString    m_reportFile;
    gtString    m_reportFileFormat; // csv of text
    AMDTUInt32  m_reportType;
    bool        m_exportToDb;
    bool        m_isReportPathSet;

    osFilePath  m_reportFilePath;

    bool InitializeArgs(int nbrArgs, char* args[]);
};

#endif // _PP_PARSE_ARGS_H_
