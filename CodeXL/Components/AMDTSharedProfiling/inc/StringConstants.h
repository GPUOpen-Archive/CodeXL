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
#define PM_STR_MENU_SEPARATOR L"/"

// General error messages:
#define PM_STR_UnsupportedProfileTypeError L"Unsupported profile type"

// Session names postfixes:
#define PM_STR_ImportedSessionPostfix "Imported"


// Profile sessions HTML description:
#define PM_Str_HTMLProfileSessionCaption L"Profile Session"
#define PM_Str_HTMLProfileType L"Profile Type"
#define PM_Str_HTMLSessionName L"Session Name"
#define PM_Str_HTMLProfileGPUPrefix L"GPU"
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
#define PM_STR_STATUS_START L"Start profiling the target"
#define PM_STR_StartProfilingNoProjectIsLoaded "No project loaded.\nClick OK to create a new project."

#define PM_STR_StartProfilingNoExeIsSet "No executable selected.\nPlease select an executable in the Project Settings dialog."


#define PM_STR_SWITCH_TO_PROFILE_MODE_MENU_COMMAND_PREFIX L"Switch to &Profile Mode - "
#define PM_STR_PROFILE_MODE_MENU_COMMAND_PREFIX L"&Profile Mode - "
#define PM_STR_STATUS_SELECT L"Currently selected profile - Move to Profile Mode"
#define PM_STR_SELECT_EMPTY L"No profile selected"

#define PM_STR_MENU_SETTINGS L"P&rofile Settings...\t"
#define PM_STR_STATUS_SETTINGS L"Open the profile project settings"

// Profile mode
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
#define PM_STR_NewSessionCreationErrorProcessRunning L"A new session cannot be created while a process is running"


#define PM_STR_ImportDialogTitle "Import Profile Session"
#define PM_STR_ImportWarningHeader "Import Error"
#define PM_STR_ImportWarning "Import is allowed only in Profile Mode and when profiling is not in progress"
#define PM_STR_ImportWarningProcessing "A profile session processing is being executed. Please wait until the processing is done."
#define PM_STR_ImportWarningImporting "Another importing is in progress. Import is allowed one at a time."
#define PM_STR_ImportNoProjectWarning "Please create a new project or open an existing one before importing session data."

// Profile type strings:
#define PM_profileTypePerformanceCounters "Performance Counters"
#define PM_profileTypeApplicationTrace "Application Timeline Trace"
#define PM_profileTypeApplicationTraceWide L"Application Timeline Trace"

// Profile type strings with prefix:
#define PM_profileTypePerformanceCountersPrefix "GPU: Performance Counters"
#define PM_profileTypeApplicationTracePrefix "Application Timeline Trace"

// Genetic session types strings
#define PM_profileSessions L"Profile sessions"

// Profile types menu items with accelerators
#define PM_profileTypePerformanceCountersPrefixWithAccelerator "GPU: Performa&nce Counters"
#define PM_profileTypeApplicationTracePrefixWithAccelerator "Application Timeline Trac&e"

#define PM_profileTypeGPUPerformanceCountersDescription "Collects performance counters from the GPU or APU for each kernel dispatched to the device"
#define PM_profileTypeApplicationTraceDescription "Show a timeline and table tracing API and performance marker calls"

#define PM_STR_SharedProfileExtensionName L"SharedProfile"
#define PM_STR_SharedProfileExtensionNameA "SharedProfile"
#define PM_STR_SharedProfileExtensionTreePathStr L"Profile"

/// XML Strings:
#define PM_STR_xmlProfileEntireDuration L"ProfileEntireDuration"
#define PM_STR_xmlProfilePaused L"ProfilePaused"
#define PM_STR_xmlProfileStartDelay L"ProfileStartDelay"
#define PM_STR_xmlProfileEndAfter L"ProfileEndAfter"

// Widgets:
#define PM_STR_sharedProfileSettingsProfileType "Profile Type"
#define PM_STR_sharedProfileSettingsProfileSessionType "Profile Session Type:"
#define PM_STR_sharedProfileSettingsProfileSessionTypeTooltip "Click to select the profile type. Change the profile types to display a short description for the current selected profile type."


// properties view
#define PM_STR_PropertiesExecutionInformationSA L"To start profiling, select 'Start Profiling' from the Profile menu,<br> or click the 'Start Profiling' toolbar button."
#define PM_STR_PropertiesExecutionInformationVS L"To start profiling, select 'Start Profiling' from the CodeXL menu,<br> or click the 'Start Profiling' toolbar button."

/// Last browse location strings:
#define PM_STR_lastBrowsedImportFolder "LastBrowsedImportFolder"

/// toolbar start button
#define PM_STR_startButtonProfileMode L"Profile"
#define PM_STR_startButtonProfileGPUMode L"GPU Profile"

#endif //__STRINGCONSTANTS_H

