//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppStringConstants.h
///
//==================================================================================

//------------------------------ ppStringConstants.h ------------------------------

#ifndef __PPSTRINGCONSTANTS_H
#define __PPSTRINGCONSTANTS_H


#include <Backend/AMDTPowerProfileAPI/src/ppCountersStringConstants.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Driver:
#define PP_STR_DriverPath L"\\drivers\\AMDTPWRPROF"

// Project setting
#define PP_STR_projectSettingExtensionName L"PowerProfile"
#define PP_STR_projectSettingExtensionNameASCII "PowerProfile"
#define PP_STR_projectSettingExtensionDisplayName L"Profile, Power Profile"

// Global settings xml file
#define PP_STR_projectSettingEnabledCounters L"EnabledCounters"
#define PP_STR_projectSettingSamplingInterval L"SamplingInterval"

// Project settings:
#define PP_STR_projectSettingsSamplingInterval "Sampling Interval"
#define PP_STR_projectSettingsSampleEvery "Counters sampling interval (ms)"

// Profiling info
#define PP_STR_OnlineProfileName L"Power Profiling"

// Power Graph yAxis units:
#define PP_STR_UnitsWatts "Watts"
#define PP_STR_UnitsJoules "Joules"
#define PP_STR_UnitsKiloJoules "Kilojoules"
#define PP_STR_UnitsMegaJoules "Megajoules"
#define PP_STR_UnitsGigaJoules "Gigajoules"
#define PP_STR_UnitsTeraJoules "Terajoules"
#define PP_STR_UnitsPostfixVolt "[V]"
#define PP_STR_UnitsPostfixWatt "[W]"
#define PP_STR_UnitsPostfixMHz "[MHz]"
#define PP_STR_UnitsPostfixPercentage "%"
#define PP_STR_UnitsPostfixCelsius "[C]"
#define PP_STR_UnitsPostfixmA "[mA]"

// file info
#define PP_STR_dbFileExt L"cxldb"
#define PP_STR_dbFileExtSearchString "*.cxldb"
#define PP_STR_PowerProfileFileExtensionCaption "Power Profile"


// Tree data
#define PP_STR_TreeNodeSummary L"Summary"
#define PP_STR_TreeNodeTimeline L"Timeline"

#define PP_STR_sessionViewCaption L" (Power)"

// Summary view:
#define PP_STR_SummaryDurationLabel "Session Duration: <b>%02d:%02d:%02d.%03d</b>"
#define PP_STR_SummaryCumulativeEnergyCaption "Cumulative Energy Consumption [Joules]"
#define PP_STR_SummaryAveragePowerCaption "Average APU Power Consumption [Watts]"
#define PP_STR_SummaryCPUFrequencyCaption "CPU Avg Frequency [MHz]"
#define PP_STR_SummaryGPUFrequencyCaption "GPU Avg Frequency [MHz]"
#define PP_STR_SummaryCPUFrequencyYAxisLabel "Duration [Sec]"
#define PP_STR_SummaryViewPowerCaptionWithUnits "Power [%1]"
#define PP_STR_SummaryViewEnergyCaptionWithUnits "Energy [%1]"
#define PP_STR_SummaryCPUFrequencyCounterPrefix "CPU"
#define PP_STR_SummaryGPUFrequencyCounterPrefix "GPU"
#define PP_STR_SummaryiGPUFrequencyCounterPrefix "iGPU"
#define PP_STR_SummaryFrequencyCounterPostfix " " PP_STR_FrequencyCounterPostfix
#define PP_STR_SummaryCPUPowerCounterPostfix " Power"
#define PP_STR_SummaryViewPowerGraphTypeAverage "Total Energy Consumption of all supported devices: %1 [%2]"
#define PP_STR_SummaryViewEnergyGraphTypeCumulative "Average Power Consumption of all supported devices: %1 [%2]"

// Timeline view:
#define PP_STR_TimeLineDgpuCounterPart "dGPU"

// Session HTML summary strings:
#define PP_STR_SummaryExecutionHeader L"Execution Information"
#define PP_STR_SummaryTargetPath L"Target Path"
#define PP_STR_SummaryWorkingDirectory L"Working Directory"
#define PP_STR_SummarySessionDirectory L"Profile Session Directory"
#define PP_STR_SummaryCommandLineArgs L"Command Line Arguments"
#define PP_STR_SummaryEnvVars L"Environment Variables"
#define PP_STR_SummaryProfileScope L"Profile Scope"
#define PP_STR_SummaryProfileDuration L"Profile Duration"
#define PP_STR_SummaryProfileScopeSysWide L"System-wide"
#define PP_STR_SummaryProfileDetailsHeader L"Profile Details"
#define PP_STR_SummarySessionType L"Profile Session Type"
#define PP_STR_SummaryProfileStartTime L"Profile Start Time"
#define PP_STR_SummaryProfileEndTime L"Profile End Time"
#define PP_STR_SummaryProfilePower L"Power"
#define PP_STR_SummaryRunningMessage L"Session is currently running"
#define PP_STR_SummaryNewCreatedMessage L"Click play to start profiling"

// Navigation chart strings:

#define PP_STR_NavChartYAxisLabel PP_STR_Counter_Power_TotalAPU

// Timeline view strings:
#define PP_StrTimelineRibbonButtonIndexPropertyName "RibbonIndex"
#define PP_StrTimelineTimeLabelPrefix "Time:&nbsp;"
#define PP_StrTimelineCurrentGraphName "Current"
#define PP_StrTimelineAPUPowerGraphName "Node Power"
#define PP_StrTimelineScaledTemperatureGraphName "Scaled Temp."
#define PP_StrTimelineVoltageGraphName "Voltage"
#define PP_StrTimelineCPUCoreStateGraphName "CPU Core State (DVFS)"
#define PP_StrTimelineCStateGraphName "C - State Residency"
#define PP_StrTimelineEnergyGraphName "Energy"
#define PP_StrTimelineProgressBarWrapperLabel L"Profile duration: "
#define PP_StrTimelineNavigationCounterSelectionLabel "Use <b>%1</b> for session navigation.<br><a href='change_counter'>Click to change</a>"
#define PP_StrTimelineRibbonButtonDownTooltip "Move \"%1\" graph down"
#define PP_StrTimelineRibbonButtonUpTooltip "Move \"%1\" graph up"
#define PP_StrTimelineRibbonButtonStyle "QPushButton { "\
    "border: none;" \
    "background-color: transparent;" \
    "}" \
    "QPushButton:pressed{ " \
    "border: solid gray 1;" \
    "background-color: transparent;" \
    "border: none;" \
    "}" \
    "QPushButton:hover{ " \
    "border: 1px  solid gray;" \
    "background-color: transparent;" \
    "}"

// log strings
#define PP_STR_logMiddleTierNotInit L"Power Profiler middle tier not initialized. Can't start profiling"
#define PP_STR_logStartProfiling L"Power Profiling started"
#define PP_STR_logStopProfiling L"Power Profiling stopped"

// Main menu
#define PP_STR_CountersSelection                   L"S&elect Power Profiling Counters..."
#define PP_STR_CountersSelectionStatusbarString    L"Select which counters will be sampled"

// Counter selection dialog
#define PP_STR_DialogTitle                      "Counters Selection"
#define PP_STR_DialogDescription                "Add a performance counters to the 'Active Counters' list to view it's usage"
#define PP_STR_DialogGroupBoxTitle              "Performance Counters"
#define PP_STR_AvailableCountersTitle           "Available Counters"
#define PP_STR_ActiveCountersTitle              "Active Counters"
#define PP_STR_MultipleCountersSelectedHeading  "Multiple Counters Selected"
#define PP_STR_MultipleCountersSelectedMsg      "Select a single item to see its description"
#define PP_STR_CounterDescriptionCaption        "<font color=blue>%1</font><br>%2"
#define PP_STR_CounterSelectionSwitchToPowerProfileQuestion "Profile configuration settings were changed.\nDo you want to change the current profile type to 'Power Profile'?"

#define PP_STR_CounterSelectionRemoveErrorPrefix "Failed to remove counter"
#define PP_STR_CounterSelectionRemoveErrorPostfix " cannot be removed"
#define PP_STR_CounterSelectionProcessIdPrefix "Process Id"

// Messages:
#define PP_STR_PowerProfilerNotSupportedOnHW L"CodeXL Power Profiler : this station does not have devices that support power profiling. Local Power Profiling will be disabled.\n"
#define PP_STR_PowerProfilerNotInitializedErrorMessage L"Communication to CodeXL Power Profiling driver (%ls) returned error %d"
#define PP_STR_PowerProfilerNotSupportedErrorMessage L"Power profiler is not supported on current hardware"
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define PP_STR_PowerProfilerNotInitializedPrefix L"If you have recently upgraded or installed CodeXL, please reboot and try again.\n"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define PP_STR_PowerProfilerNotInitializedPrefix  L"Please install the Power Profiler driver as detailed in the CodeXL User Guide and then re-run profiling.\n"
#endif

#define PP_STR_SessionStopConfirm "Closing this window will stop the running power profile session. Are you sure you want to continue?"
#define PP_STR_OTHER_GRAPH_DESCRIPTION "Power consumed by other APU sub-components, including leaks and other power losses."

#define PP_STR_TimelineContextMenuShowStackedGraph "Show Stacked Graph..."
#define PP_STR_TimelineContextMenuShowNonStackedGraph "Show Non Stacked Graph..."

// Error messages
#define PP_STR_DriverAlreadyInUseMessageLocal L"A power profiling session is already in progress. Please make sure that no other instance of CodeXL graphic client or command line tool is performing a power profiling session and click the \"Start\" button again."
#define PP_STR_DriverAlreadyInUseMessageRemote L"A power profiling session is already in progress on the remote machine. Please make sure that no other client is connected to the remote agent, and try again."

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define PP_STR_DriverVersionMismatchMessage L"The installed Power Profiling driver's version is incompatible with this instance of CodeXL. Please reinstall CodeXL, or run the AMDTPwrProfDriverInstall.run script."
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define PP_STR_DriverVersionMismatchMessage L"The installed Power Profiling driver's version is incompatible with this instance of CodeXL. Please reinstall CodeXL."
#endif

#define PP_STR_RemoteConnectionErrorMessage L"Unable to connect to CodeXL Remote Agent. Please make sure that:\n1. CodeXL Remote Agent is running on the target machine.\n2. There is network access from the client machine to the remote machine.\n3. CodeXL is not blocked by a firewall on the client machine.\n4. CodeXL Remote Agent is not blocked by a firewall on the remote machine."
#define PP_STR_TargetApplicationNotFoundErrorMessage "The target application: "
#define PP_STR_TargetWorkingDirNotFoundErrorMessage "The target application's working directory: "
#define PP_STR_CouldNotBeLocatedOnRemoteMachineErrorMessage " could not be located on the remote machine."
#define PP_STR_RemoteFailedToLaunchTargetAppErrorMessage "Failed to launch the target application on the remote machine."
#define PP_STR_RemoteFatalCommunicationErrorMessage "The communication with the remote agent was lost.\nPlease make sure that the remote agent is running and that the remote machine is accessible to the client machine via the network."
#define PP_STR_RemoteHandshakeFailureErrorMessage "The remote agent's version does not match the client's version. Session aborted."
#define PP_STRHypervisorNotSupportedErrorMessage  "Failed to launch : Hypervisor machine not supported"
#define PP_STRCountersNotEnabledErrorMessage  "No counters are enabled for collecting profile data"
#define PP_STR_SmuDisabledMsg "SMU is Disabled. Some of the counters will be disabled."
#define PP_STR_iGPUDisabledMsg "iGPU is Disabled. Some of the counters will be disabled."
#define PP_STR_dbMigrateFailureMsg "Unable to migrate the older version of CodeXL profile database."
#define PP_STR_ProjectSettingsPathsInvalid AF_STR_newProjectExeDoesNotExistOrInvalid

// Strings for metrics calculations:
#define PP_STR_LongestCounterName PP_STR_Counter_AvgFreq_Core0

// HTML style definitions:
#define PP_STR_CSS_NavigationChartTimeLabelStyle "QLabel {color: %1; font-size: 10px; font-weight: normal}"
#define PP_STR_CSS_TimelineViewTooltipStyle "border: 1px solid %1; border-radius: 2px; padding: 2px; background-color: %2;"

// Data utils:
#define PP_STR_DebugMessageUnknownGraphType L"Unknown graph type"



#endif //__PPSTRINGCONSTANTS_H
