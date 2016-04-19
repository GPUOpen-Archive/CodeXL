//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StringConstants.h
///
//==================================================================================

#ifndef __STRINGCONSTANTS_H
#define __STRINGCONSTANTS_H


// Profile menu
#define PM_STR_SHORT_RUN L"F5"
#define PM_STR_SHORT_RUN_A "F5"
#define PM_STR_SHORT_PAUSE L"F6"
#define PM_STR_SHORT_STOP L"Shift+F5"
#define PM_STR_MENU_SEPARATOR L"/"

// General error messages:
#define PM_STR_UnsupportedProfileTypeError L"Unsupported profile type"

// Session names postfixes:
#define PM_STR_ImportedSessionPostfix "Imported"
#define PM_STR_AttachedSessionPostfix "Attached"
#define PM_STR_SystemWideSessionPostfix "System-wide"

// Profile scopes:
#define PM_STR_ProfileScopeSystemWide L"System-Wide"
#define PM_STR_ProfileScopeSystemWideWithFocus L"System-Wide with focus on application"
#define PM_STR_ProfileScopeSingleApplication L"Single Application"

// Profile sessions HTML description:
#define PM_Str_HTMLProfileSessionCaption L"Profile Session"
#define PM_Str_HTMLProfileType L"Profile Type"
#define PM_Str_HTMLSessionName L"Session Name"
#define PM_Str_HTMLProfileCPUPrefix L"CPU"
#define PM_Str_HTMLProfileGPUPrefix L"GPU"
#define PM_Str_HTMLProfilePowerPrefix L"Power"
#define PM_Str_HTMLProfileThreadPrefix L"Thread"
#define PM_Str_HTMLProfileFrameAnalysisPrefix L"Frame Analysis"
#define PM_Str_HTMLProfileScope L"Profile Scope"
#define PM_Str_HTMLExePathTitle L"Executable Path: "
#define PM_Str_HTMLArgumentsTitle L"Arguments: "
#define PM_Str_HTMLWorkDirTitle L"Working Directory: "
#define PM_Str_HTMLEnvVarTitle L"Environment Variables: "
#define PM_Str_HTMLProfileStartTime L"Profile Start Time"
#define PM_Str_HTMLProfileEndTime L"Profile End Time"
#define PM_Str_HTMLProfileDuration L"Profile Duration"
#define PM_Str_ViewHTMLLineStructure L"<b>%ls:</b> %ls"

// Profile menu:
#define PM_STR_MENU_START L"&Start Profiling\t" PM_STR_SHORT_RUN
#define PM_STR_MENU_START_WITH_PARAMS "&Start Profiling (%1)\t" PM_STR_SHORT_RUN_A
#define PM_STR_MENU_START_NO_PARAMS "&Start Profiling\t" PM_STR_SHORT_RUN_A
#define PM_STR_SYSTEM_WIDE "System-wide"
#define PM_STR_STATUS_START L"Start profiling the target"
#define PM_STR_StartProfilingNoProjectIsLoaded "No project loaded.\nClick OK to create a new project."
#define PM_STR_StartProfilingNoExeNoSystemWide "No executable selected.\nWould you like to perform system-wide profiling?"

#define PM_STR_StartProfilingNoExeIsSet "No executable selected.\nPlease select an executable in the Project Settings dialog."


//Pause & Stop menu:
#define PM_STR_MENU_PAUSE L"&Pause Profiling\t" PM_STR_SHORT_PAUSE
#define PM_STR_STATUS_PAUSE L"Pause or resume profiling the target"
#define PM_STR_MENU_STOP L"St&op Profiling\t" PM_STR_SHORT_STOP
#define PM_STR_STATUS_STOP L"Stop profiling the target"
#define PM_STR_MENU_PAUSE_DATA L"&Pause Data Collection\t" PM_STR_SHORT_PAUSE
#define PM_STR_STATUS_PAUSE_DATA "Pause data collection of the application"
#define PM_STR_MENU_PAUSE_DATA_TOOLBAR "Pause Data Collection (" AF_STR_PauseDebuggingShortcut ")"

#define PM_STR_MENU_ATTACH L"A&ttach to Process...\t"
#define PM_STR_STATUS_ATTACH L"Attach to process and start profiling"

#define PM_STR_SWITCH_TO_PROFILE_MODE_MENU_COMMAND_PREFIX L"Switch to &Profile Mode - "
#define PM_STR_PROFILE_MODE_MENU_COMMAND_PREFIX L"&Profile Mode - "
#define PM_STR_STATUS_SELECT L"Currently selected profile - Move to Profile Mode"
#define PM_STR_SELECT_EMPTY L"No profile selected"

#define PM_STR_MENU_SETTINGS L"P&rofile Settings...\t"
#define PM_STR_STATUS_SETTINGS L"Open the profile project settings"

// Profile mode
#define PM_STR_FrameAnalysisMode L"Frame Analysis Mode"
#define PM_STR_SwitchToFrameAnalysisMode L"Switch to Frame Analysis Mode"
#define PM_STR_PROFILE_MODE L"Profile Mode"
#define PM_STR_PROFILE_MODE_ACTION L"Profiling"
#define PM_STR_PROFILE_MODE_VERB L"profile"
#define PM_STR_PROFILE_MODE_DESCRIPTION L"Perform profiling of various CPU and GPU metrics<br>to assess performance and locate bottlenecks."

#define PM_STR_PROCESS_IS_RUNNING_MESSAGE L"Profiled session is running.<br> Profiled session results will be displayed after the profile session will finish executing."
#define PM_STR_PROCESS_IS_NOT_RUNNING_MESSAGE L"The profiled session has completed executing.<br> Navigate CodeXL explorer for the last session results."

#define PM_STR_PROFILE_TREE_SESSION_EXIST "This session already exists in this project"

// Rename error messages:
#define PM_STR_RENAME_ONLY_WHITESPACES "Session name should not contain only whitespace character(s)."
#define PM_STR_RENAME_LEADING_OR_TRAILING "Session name should not contain leading or trailing whitespace."
#define PM_STR_RENAME_EMPTY "Session name should not be empty."
#define PM_STR_RENAME_SPECIAL "A session name can't contain any of the following characters:\n   \\ / : * ? \" < > | %"
#define PM_STR_RENAME_SESSION_EXISTS "Session \"%1\" already exists."
#define PM_STR_RENAME_FILE_CANNOT_BE_RENAMED "Session file could not be renamed. Check if the file is used by another process and retry."

// Tree context menu:
// Initialize the context menu for tree items:
#define PM_STR_TREE_OPEN_ITEM "Open Item"
#define PM_STR_TREE_OPEN_ITEM_PREFIX "Open "
#define PM_STR_TREE_DELETE_ALL_TYPE "Delete All %1 Sessions"
#define PM_STR_TREE_DELETE_SESSION "Delete Session"
#define PM_STR_TREE_DELETE_MULTIPLE_SESSION "Delete Selected Sessions"
#define PM_STR_TREE_DELETE_ALL_SESSIONS "Delete All Sessions"
#define PM_STR_TREE_RENAME_SESSION "Rename Session"
#define PM_STR_TREE_OPEN_CONTAINING_FOLDER "Open Containing Folder"
#define PM_STR_TREE_IMPORT_SESSION "Import Session..."
#define PM_STR_TREE_EXPORT_SESSION "Export Session..."
#define PM_STR_TREE_REFRESH_FROM_SERVER "Refresh sessions from server"
#define PS_STR_TREE_DELETE_QUESTION "Are you sure you want to delete %1?"
#define PS_STR_TREE_DELETE_MULTIPLE_QUESTION "Are you sure you want to delete the selected sessions?"
#define PS_STR_TREE_DELETE_ALL_QUESTION "Are you sure you want to delete all sessions?"
#define PS_STR_TREE_DELETE_ALL_FROM_TYPE_QUESTION "Are you sure you want to delete all %1 sessions?"
#define PS_STR_TREE_DELETE_FAILED_REPORT "The following session files could not be deleted:"

// New session:
#define PM_STR_NewSessionCreationOtherProfileTypeQuestion "Do you want to switch to Power Profiling?"
#define PM_STR_NewSessionCreationErrorProcessRunning L"A new session cannot be created while a process is running"
#define PM_STR_NewSessionCreationErrorOtherProfileType L"New session can be created only for power profile sessions. Select 'Power Profile' and retry"

#define PM_STR_OnlineProfileName "Power Profiling"
#define PM_STR_OnlineProfileNameW L"Power Profiling"
#define PM_STR_NewSessionNodeName L"New Power Session..."
#define PM_STR_NewSessionName L"New Power Session"


#define PM_STR_ImportDialogTitle "Import Profile Session"
#define PM_STR_ImportWarningHeader "Import Error"
#define PM_STR_ImportWarning "Import is allowed only in Profile Mode and when profiling is not in progress"
#define PM_STR_ImportWarningProcessing "A profile session processing is being executed. Please wait until the processing is done."
#define PM_STR_ImportWarningImporting "Another importing is in progress. Import is allowed one at a time."
#define PM_STR_ImportNoProjectWarning "Please create a new project or open an existing one before importing session data."
#define PM_STR_ExportDialogTitle "Export Frame Analysis Session"
#define PM_STR_ImportFrameAnalysisDialogTitle "Import Frame Analysis Session"

// Profile type strings:
#define PM_profileTypeTimeBased "Time-based Sampling"
#define PM_profileTypeCustomProfile "Custom Profile"
#define PM_profileTypeCLU "Cache Line Utilization"
#define PM_profileTypeAssesPerformance "Assess Performance"
#define PM_profileTypeInstructionBasedSampling "Instruction-based Sampling"
#define PM_profileTypeInvestigateBranching "Investigate Branching"
#define PM_profileTypeInvestigateDataAccess "Investigate Data Access"
#define PM_profileTypeInvestigateInstructionAccess "Investigate Instruction Access"
#define PM_profileTypeInvestigateInstructionL2CacheAccess "Investigate L2 Cache Access"
#define PM_profileTypePerformanceCounters "Performance Counters"
#define PM_profileTypeApplicationTrace "Application Timeline Trace"
#define PM_profileTypePowerProfile "Power Profiling"
#define PM_profileTypeFrameAnalysis "Frame Analysis"
#define PM_profileTypeThreadProfile "Thread Profiling"

// Profile type strings with prefix:
#define PM_profileTypeTimeBasedPrefix "CPU: Time-based Sampling"
#define PM_profileTypeCustomProfilePrefix "CPU: Custom Profile"
#define PM_profileTypeCLUPrefix "CPU: Cache Line Utilization"
#define PM_profileTypeAssesPerformancePrefix "CPU: Assess Performance"
#define PM_profileTypeInstructionBasedSamplingPrefix "CPU: Instruction-based Sampling"
#define PM_profileTypeInvestigateBranchingPrefix "CPU: Investigate Branching"
#define PM_profileTypeInvestigateDataAccessPrefix "CPU: Investigate Data Access"
#define PM_profileTypeInvestigateInstructionAccessPrefix "CPU: Investigate Instruction Access"
#define PM_profileTypeInvestigateInstructionL2CacheAccessPrefix "CPU: Investigate L2 Cache Access"
#define PM_profileTypePerformanceCountersPrefix "GPU: Performance Counters"
#define PM_profileTypeApplicationTracePrefix "Application Timeline Trace"
#define PM_profileTypePowerProfilePrefix "Power Profiling"
#define PM_profileTypeFrameAnalysisPrefix "Frame Analysis"
#define PM_profileTypeThreadProfilePrefix "CPU: Thread Profiling"

// Genetic session types strings
#define PM_profileSessions L"Profile sessions"
#define PM_frameAnalysisSessions L"Frame Analysis sessions"

// Profile types menu items with accelerators
#define PM_profileTypeTimeBasedPrefixWithAccelerator "CPU: Time-&based Sampling"
#define PM_profileTypeCustomProfilePrefixWithAccelerator "CPU: C&ustom Profile"
#define PM_profileTypeCLUPrefixWithAccelerator "CPU: &Cache Line Utilization"
#define PM_profileTypeAssesPerformancePrefixWithAccelerator "CPU: &Assess Performance"
#define PM_profileTypeInstructionBasedSamplingPrefixWithAccelerator "CPU: &Instruction-based Sampling"
#define PM_profileTypeInvestigateBranchingPrefixWithAccelerator "CPU: In&vestigate Branching"
#define PM_profileTypeInvestigateDataAccessPrefixWithAccelerator "CPU: Investi&gate Data Access"
#define PM_profileTypeInvestigateInstructionAccessPrefixWithAccelerator "CPU: Investigate Inst&ruction Access"
#define PM_profileTypeInvestigateInstructionL2CacheAccessPrefixWithAccelerator "CPU: Investigate &L2 Cache Access"
#define PM_profileTypeThreadProfilePrefixWithAccelerator "CPU: T&hread Profiling"
#define PM_profileTypePerformanceCountersPrefixWithAccelerator "GPU: Performa&nce Counters"
#define PM_profileTypeApplicationTracePrefixWithAccelerator "Application Timeline Trac&e"
#define PM_profileTypePowerProfilePrefixWithAccelerator "Po&wer Profiling"

#define PM_profileTypeCPUTimeBasedDescription "Identify the hot spots in a program that are consuming the most time"
#define PM_profileTypeCPUAssessPerformanceDescription "Assess overall program performance"
#define PM_profileTypeCPUInvestigateDataAccessDescription "Investigate how well the application uses the data cache (DC)"
#define PM_profileTypeCPUInvestigateInstrctionAccessDescription "Investigate how well the application uses the instruction cache (IC)"
#define PM_profileTypeCPUInvestigateL2CacheDescription "Investigate how well the application uses the unified level (L2) cache"
#define PM_profileTypeCPUInvestigateBranchingDescription "Identify mispredicted branches, taken branches and near returns"
#define PM_profileTypeCPUCustomDescription "Allows user to select sampling-events to measure, set the sampling period and other configuration parameters"
#define PM_profileTypeCPUIBSDescription "Use to precisely attribute events to instructions that caused those events. \nProduces wealth of event data like instruction cache hit/miss, ITLB hit/miss,\ndata cache hit/miss DTLB hit/miss, load/store operations, Mispredicted/taken branch operations in a single profile run."
#define PM_profileTypeCPUCLUDescription "Provides a measure of how efficiently an application utilizes the cache"
#define PM_profileTypeGPUPerformanceCountersDescription "Collects performance counters from the GPU or APU for each kernel dispatched to the device"
#define PM_profileTypeApplicationTraceDescription "Show a timeline and table tracing API and performance marker calls"
#define PM_profileTypePowerProfileDescription "Use this session type to collect power consumption rates for various APU components, \nas well as additional metrics such as Frequency, Voltage and Current levels, Temperature changes, etc."
#define PM_profileTypeDXProfileDescription "DX profiling."

#define PM_STR_SharedProfileExtensionName L"SharedProfile"
#define PM_STR_SharedProfileExtensionNameA "SharedProfile"
#define PM_STR_SharedProfileExtensionTreePathStr L"Profile"

/// XML Strings:
#define PM_STR_xmlProfileScope L"ProfileScope"
#define PM_STR_xmlProfileEntireDuration L"ProfileEntireDuration"
#define PM_STR_xmlProfilePaused L"ProfilePaused"
#define PM_STR_xmlProfileStartDelay L"ProfileStartDelay"
#define PM_STR_xmlProfileEndAfter L"ProfileEndAfter"
#define PM_STR_xmlProfileTerminateAfter L"ProfileTerminateAfter"

// Widgets:
#define PM_STR_sharedProfileSettingsSingleApplication "Single Application Profile"
#define PM_STR_sharedProfileSettingsSystemWideProfile "System-Wide Profile"
#define PM_STR_sharedProfileSettingsSystemWideWithFocusProfile "System-Wide Profile with focus on application"
#define PM_STR_sharedProfileSettingsSingleApplicationTooltip "Launch the project profiled application, and collect profile data only from the launched application"
#define PM_STR_sharedProfileSettingsSystemWideProfileTooltip "Perform system-wide profile, collect profile data from all the current running processes"
#define PM_STR_sharedProfileSettingsSystemWideWithFocusProfileTooltip "Launch the project profiled application, collect profile data from the launched application, and from all other running processes"
#define PM_STR_sharedProfileSettingsDataCollectionSchedule "Data Collection Schedule"
#define PM_STR_sharedProfileSettingsEntireDuration "Throughout entire duration"
#define PM_STR_sharedProfileSettingsProfilePaused "Start profile with data collection paused"
#define PM_STR_sharedProfileSettingsProfileScheduled "Scheduled:"
#define PM_STR_sharedProfileSettingsStartAfter "Start data collection after"
#define PM_STR_sharedProfileSettingsEndAfter "End data collection after"
#define PM_STR_sharedProfileSettingsTerminateAfter "Then, terminate the process"
#define PM_STR_sharedProfileSettingsSeconds "seconds"
#define PM_STR_sharedProfileSettingsAdditionalSeconds "additional seconds"
#define PM_STR_sharedProfileSettingsProfileType "Profile Type"
#define PM_STR_sharedProfileSettingsProfileSessionType "Profile Session Type:"
#define PM_STR_sharedProfileSettingsCollectDataSchedule "Collection Schedule:"
#define PM_STR_sharedProfileSettingsProfileScope "Profile Scope:"

// properties view
#define PM_STR_PropertiesExecutionInformationSA L"To start profiling, select 'Start Profiling' from the Profile menu,<br> or click the 'Start Profiling' toolbar button."
#define PM_STR_PropertiesExecutionInformationVS L"To start profiling, select 'Start Profiling' from the CodeXL menu,<br> or click the 'Start Profiling' toolbar button."

/// Last browse location strings:
#define PM_STR_lastBrowsedImportFolder "LastBrowsedImportFolder"
#define PM_STR_lastBrowsedExportFolder "LastBrowsedExportFolder"

#endif //__STRINGCONSTANTS_H

