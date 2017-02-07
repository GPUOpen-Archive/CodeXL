//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: #130 $
/// \brief  The string constants used in the CodeAnalyst component
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/StringConstants.h#130 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CASTRINGCONSTANSTS_H
#define _CASTRINGCONSTANSTS_H

#include <QObject>

/// \TODO Handle overlapped framework menus

// Keyboard shortcuts (Platform specific). Note that each has a menu version and a string version.
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    //edit menu
    #define CA_STR_SHORT_COPY L"Ctrl+C"
    #define CA_STR_SHORT_FIND L"Ctrl+F"
    #define CA_STR_SHORT_FIND_NEXT L"F3"
    #define CA_STR_SHORT_SELECT_ALL L"Ctrl+A"
    #define CA_STR_SHORT_NEXT_MARK L"Ctrl+Shift+N"
    #define CA_STR_SHORT_PREV_MARK L"Ctrl+Shift+P"
    //Profile menu
    #define CA_STR_SHORT_PAUSE L"Alt+Ctrl+P"
    #define CA_STR_SHORT_STOP L"Ctrl+Shift+Enter"
    //Tools menu
    //Help menu
#else
    //edit menu
    #define CA_STR_SHORT_COPY L"Ctrl+C"
    #define CA_STR_SHORT_FIND L"Ctrl+F"
    #define CA_STR_SHORT_FIND_NEXT L"F3"
    #define CA_STR_SHORT_SELECT_ALL L"Ctrl+A"
    #define CA_STR_SHORT_NEXT_MARK L"Ctrl+Shift+N"
    #define CA_STR_SHORT_PREV_MARK L"Ctrl+Shift+P"
    //Profile menu
    #define CA_STR_SHORT_PAUSE L"Shift+F5"
    #define CA_STR_SHORT_STOP L"F6"
    //Tools menu
    //Help menu
#endif

#define CA_STR_MENU_SEPARATOR L"/"

//Edit menu:
#define CA_STR_MENU_EDIT L"&Edit"
#define CA_STR_MENU_VIEW L"&View"
#define CA_STR_MENU_COPY L"&Copy\t" CA_STR_SHORT_COPY
#define CA_STR_STATUS_COPY L"Copy the selected text to the Clipboard"
#define CA_STR_MENU_FIND L"&Find\t" CA_STR_SHORT_FIND
#define CA_STR_STATUS_FIND L"Find the specified text"
#define CA_STR_MENU_FIND_NEXT L"Find &Next\t" CA_STR_SHORT_FIND_NEXT
#define CA_STR_STATUS_FIND_NEXT L"Find the specified text again"
#define CA_STR_MENU_SELECT_ALL L"Select &All\t" CA_STR_SHORT_SELECT_ALL
#define CA_STR_STATUS_SELECT_ALL L"Select all items in the view"
#define CA_STR_MENU_SHOW_HIDE_INFO L"Show / Hide &information bar\t"
#define CA_STR_STATUS_SHOW_HIDE_INFO L"Show / Hide bottom session window information bar"
#define CA_STR_MENU_NEXT_MARK L"Go to Next Marker\t" CA_STR_SHORT_NEXT_MARK
#define CA_STR_STATUS_NEXT_MARK L"Next: Scroll the Calls History View to the next string marker function call"
#define CA_STR_MENU_PREV_MARK L"Go to Previous Marker\t" CA_STR_SHORT_PREV_MARK
#define CA_STR_STATUS_PREV_MARK L"Previous: Scroll the Calls History View to the previous string marker function call"
#define CA_STR_MENU_DISPLAY_IN_FUNCTIONS_VIEW "Display in Functions View"
#define CA_STR_MENU_DISPLAY_IN_MODULES_VIEW "Display in Modules View"
#define CA_STR_MENU_DISPLAY_IN_CALL_GRAPH_VIEW "Display in Call Graph View"
#define CA_STR_MENU_DISPLAY_IN_SOURCE_CODE_VIEW "Open Source Code"
#define CA_STR_MENU_DISPLAY_BY "Display by"
#define CA_STR_MENU_DISPLAY_BY_PROCESS_ARG "Display Modules created by Process %1"
#define CA_STR_MENU_DISPLAY_BY_MODULE_ARG "Select %1 in Modules Table"
#define CA_STR_MENU_SHOW_CLU_NOTE "Show CLU Notes"
#define CA_STR_MENU_HIDE_CLU_NOTE "Hide CLU Notes"

//Project settings extension label
#define CPU_STR_PROJECT_SETTINGS L"CPU Profile"
#define CPU_STR_PROJECT_SETTINGS_TAB_NAME L"&CPU Profile"
#define CPU_STR_PROJECT_EXTENSION L"CpuProfile"
#define CPU_STR_PROJECT_EXTENSION_CUSTOM L"CpuProfileCustom"

//Profile prefix
const wchar_t CPU_PREFIX[] = L"CPU: ";
const char CPU_PREFIX_A[] = "CPU: ";

// PRD raw data file extension
const wchar_t PRD_EXT[] = L"prd";

// CAPerf raw data file extension
const wchar_t CAPERF_EXT[] = L"caperf";

// Raw module data file extension
const wchar_t TI_EXT[] = L"ti";

// Raw run info file extension
const wchar_t RI_EXT[] = L"ri";

// Aggregated old data file extension
const wchar_t OLD_DATA_EXT[] = L"ebp";

// Aggregated data file extension
const wchar_t DATA_EXT[] = L"cxlcpdb";


#define CP_STR_OverviewTabTitle "Profile Overview"
#define CP_STR_ModulesTabTitle "Modules"
#define CP_STR_CallGraphTabTitle "Call Graph"
#define CP_STR_FunctionsTabTitle "Functions"


//Pause/resume key
const wchar_t AMDT_CPU_PROFILING_PAUSE_KEY[] = L"AMD Cpu Profiling";

const QString CPU_PROF_MESSAGE(QObject::tr("CPU Profiling Error"));

// General views strings:
#define CP_profileAllProcesses "All Processes"
#define CP_profileAllThreads "All Threads"

// General CPU profile strings:
#define CP_strProcesses "Processes"
#define CP_strModules "Modules"
#define CP_strModulesFiltered "<b>Modules</b> filtered by selected processes"
#define CP_strProcessesFiltered "<b>Processes</b> filtered by selected modules"
#define CP_strFunctions "Functions"
#define CP_strProcess "Process"
#define CP_strModule "Module"
#define CP_strFunction "Function"
#define CP_strDisplayBy "Display by:"
#define CP_strDisplayFilterButton "Display Filter..."
#define CP_strDisplayFilterColumnsToDisplay "Columns to Display"
#define CP_strDisplayFilterSelectModules "Select Modules"
#define CP_strCPUProfileToolbar "CPU profile Display filter toolbar"
#define CP_strCPUProfilePerCPU "Data Per Core"
#define CP_strCPUProfilePerNuma "Data Per NUMA"
#define CP_strCPUProfileToolbarBase "Display"
#define CP_strCPUProfileDisplayFilterAllData "All Data"
#define CP_strRemoteCPUProfileNotSupported L"Remote CPU profiling is not currently supported."
#define CP_strLoaded "Loaded"
#define CP_strNotLoaded "Not Loaded"
#define CP_strUnknownModule "This module has no functions data. It may be a dynamically created module that was generated during run-time by unpacking binary code to memory pages and executing them. No data is available for modules generated this way."
#define CP_strOther "other"
#define CP_strFailedToOpenRawProfileFile L"Failed to open raw profile (%ls). Error: %s (error code 0x%lx)"

// Profile process error messages:
#define CP_STR_FailedToLoadDriver L"Failed to load driver.\nIf you have recently upgraded or installed CodeXL, please reboot and try again.\n"
#define CP_STR_FailedToCreateOutputFolder L"Failed to create profile output directory : %ls"
#define CP_STR_FailedToCreateOutputFolder2 L"The profile output directory wasn't available: %ls"
#define CP_STR_FailedToSetupSession L"Failed to initialize the profile session."
#define CP_STR_FailedToLaunchSession L"Failed to execute the profile session."
#define CP_STR_FailedToLaunchTarget L"Could not launch the target:"
#define CP_STR_FailedToFindEvent L"The configuration erronously contains an event (%#x) which is not\n"\
    L"available on this system.  Please choose a different\n"\
    L"configuration."

#define CP_STR_FailedToFindNBEvent L"The configuration contains a northbridge event:\n"\
    L"[%#x] %ls\n which is not a valid sampling event. "\
    L"Please fix the configuration."

#define CP_STR_FailedToSetCounterRange L"The configuration has too many events specified for the event counter range."\
    L"\nPlease choose a different configuration."

// Profile overview page:
#define CP_overviewPageGeneralHeader "General"
#define CP_overviewPageExecutionHeader L"Execution"
#define CP_overviewPageTargetPath L"Target Path"
#define CP_overviewPageWorkingDirectory L"Working Directory"
#define CP_overviewPageDataFolder L"Profile Session Directory"
#define CP_overviewPageCommandLineArgs L"Command Line Arguments"
#define CP_overviewPageEnvVars L"Environment Variables"
#define CP_overviewPageProfileScope L"Profile Scope"
#define CP_overviewPageProfileDetailsHeader L"Profile Details"
#define CP_overviewPageSessionType L"Profile Session Type"
#define CP_overviewPageProfileStartTime L"Profile Start Time"
#define CP_overviewPageProfileEndTime L"Profile End Time"
#define CP_overviewPageProfileDuration L"Profile Duration"
#define CP_overviewPageProfileCPUAffinity L"CPU Affinity"
#define CP_overviewPageProfileCPUDetails L"CPU Details"
#define CP_overviewPageProfileCPUDetailsStr L"Family 0x%x, Model 0x%x, %d core(s)"
#define CP_overviewPageTotalProcesses L"Total Processes in Profile"
#define CP_overviewPageTotalThreads L"Total Threads in Profile"
#define CP_overviewCallStackSampling L"Call Stack Sampling"
#define CP_overviewCallStackInformationScopeSubstr L"Scope - %ls, "
#define CP_overviewCallStackInformationFpoSubstr L", FPO - %ls"
#define CP_overviewCallStackInformation L"Call Stack Information: %lsDepth - %d, Frequency - per %d samples%ls"
#define CP_overviewCallStackInformationBold L"<b>Call Stack Information:</b> %lsDepth - %d, Frequency - per %d samples%ls"
#define CP_overviewPageMonitoredEventsHeader L"Monitored Events"
#define CP_overviewPageHotspotIndicatorHeader "Hotspot Indicator"
#define CP_overviewPageHottestFunctionsHeader "5 Hottest Functions"
#define CP_overviewPageHottestProcessesHeader "5 Hottest Processes"
#define CP_overviewPageHottestModulesHeader "5 Hottest Modules"
#define CP_overviewEmptySamples "No samples were collected during the profile session that fit the display settings. This may be caused by various reasons:\n - Profiling only cores that were idle\n - Profiling only events that did not occur\n - Display filter settings that hide the modules with collected samples\nTry changing the hotspot indicator or choosing different profile session settings."
#define CP_overviewHotspotWarning "Hotspot Indicator is disabled when 'Display Data Per Core/Node' is selected. Use Display Filter to un-select this option and enable Hotspot Indicator."

// Functions table:
#define CP_functionsTableProgress L"Loading session data..."

// Overview page hints:
#define CP_overviewInformationHint "The 5 Hottest Functions usually indicate the most significant performance bottlenecks of your application. Select a different Hotspot Indicator to display hottest functions based on different criteria"
#define CP_functionsInformationHint "Functions with a high sample count usually indicate performance bottlenecks. Sort the table according to a specific metric to highlight potential bottleneck functions"
#define CP_modulesInformationHint "Modules with a high sample count usually indicate performance bottlenecks. Sort the table according to a specific metric to highlight potential bottleneck modules"
#define CP_callgraphInformationHint "Drill down through call paths to find potential bottleneck functions downstream from your top level code"
#define CP_sourceCodeViewInformationHint "Select a function from the drop-down list to display its source/disassembly lines with sample counts"

// CLU specific
#define CP_overviewInformationHintForCLU "The 5 Hottest Functions indicate the functions having lowest utilization (read/write) of L1 data cache lines between evictions. Select a different Hotspot Indicator to display hottest functions based on different criteria"
#define CP_functionsInformationHintForCLU "Functions with a low cache line utilization usually indicate performance bottlenecks. Sort the table according to a specific metric to highlight potential bottleneck functions"
#define CP_modulesInformationHintForCLU "Modules with a low cache line utilization usually indicate performance bottlenecks. Sort the table according to a specific metric to highlight potential bottleneck modules"
#define CP_sourceCodeViewInformationHintForCLU "Select a function to find source/disassembly lines with low cache line utilization"

// Cpu profile table:
#define CP_colCaptionModuleName "Module"
#define CP_colCaptionModuleNameTooltip "Module file name"
#define CP_colCaptionProcessName "Process"
#define CP_colCaptionProcessNameTooltip "Process file name"
#define CP_colCaptionFunctionName "Function"
#define CP_colCaptionFunctionNameTooltip "Function name"
#define CP_colCaptionPID "PID"
#define CP_colCaptionPIDTooltip "Process ID"
#define CP_colCaptionTID "TID"
#define CP_colCaptionTIDTooltip "Thread ID"
#define CP_colCaptionSamples "Samples"
#define CP_colCaptionHotSpotSamples "Hotspot Samples"
#define CP_colCaptionSamplesTooltip "Samples collected for the Hotspot Indicator (%1)"
#define CP_colCaptionSamplesPercent "% of Hotspot Samples"
#define CP_colCaptionSamplesPercentTooltip "Percent of samples collected for the Hotspot Indicator (%1)"
#define CP_colCaptionModuleSymbolsLoaded "Symbols"
#define CP_colCaptionModuleSymbolsLoadedTooltip "Are the symbols for the modules loaded / not loaded?"
#define CP_colCaptionAddress "Address"
#define CP_colCaptionAddressTooltip "Source/disassembly address"
#define CP_colCaptionLineNumber "Line"
#define CP_colCaptionLineNumberTooltip "Source line number"
#define CP_colCaptionLineSourceCode "Source Code"
#define CP_colCaptionLineSourceCodeTooltip "Source/disassembly code of the profiled function"
#define CP_colCaptionCodeBytes "Code Bytes"
#define CP_colCaptionCodeBytesTooltip "Bytes representation of the actual machine instructions"
#define CP_colCaptionSourceFile "Source file"
#define CP_colCaptionSourceFileTooltip "Source file full path"
#define CP_colCaptionModuleId "Module Id"
#define CP_colCaptionModuleIdTooltip "Module Id Hidden"
#define CP_colCaptionFuncId "Function Id"
#define CP_colCaptionFuncIdTooltip "Function Id Hidden"

#define CP_emptyTableMessage "No samples to display. Consider changing the display settings or selecting a different hotspot indicator."
#define CP_emptyCallGraphTableMessage "No callstack data was collected. Callstack collection settings are: %1. See the CPU Profiler node in the CodeXL Project Settings"

// Source code view:
#define CP_sourceCodeViewExportString "&Export source data..."
#define CP_sourceCodeViewFunctionPrefix "Function:"
#define CP_sourceCodeViewProcessPrefix "Process:"
#define CP_sourceCodeViewTIDPrefix "TID:"
#define CP_sourceCodeExpandAll "Expand All"
#define CP_sourceCodeCollapseAll "Collapse All"
#define CP_sourceCodeShowCodeBytes "Show Code Bytes"
#define CP_sourceCodeShowAddress "Show Address"
#define CP_sourceCodeShowCluNotes CA_STR_MENU_SHOW_CLU_NOTE
#define CP_sourceCodeHideCluNotes CA_STR_MENU_HIDE_CLU_NOTE
#define CP_sourceCodeErrorCouldNotOpenFile "Could not open %1.\nDisassembly will not be available."
#define CP_sourceCodeErrorSourceNotFound "The source file was not found. Cannot annotate source. Only disassembly will be available."
#define CP_sourceCodeErrorFailedToAnnotateSource "Failed to annotate source. Only disassembly will be available."
#define CP_sourceCodeViewDoubleClickText "Scroll down to display the next block of disassembly instructions."
#define CP_sourceCodeViewDoubleClickTooltip "Disassembly size is too large to display. Scroll down to display the next block of disassembly instructions."
#define CP_sourceCodeViewBreak "----- break -----"
#define CP_sourceCodeViewFunctionsPercentageTooltip "%2.2f%% of functions samples, %2.2f%% of total samples"
#define CP_sourceCodeViewDisassemblyChunkSizeExtractionFailure L"Failed to extract disassembly instructions chunk size from the global settings."
#define CP_sourceCodeViewProgressDisassemblyUpdate L"Building Disassembly Instructions..."
#define CP_sourceCodeViewProgressSamplesUpdate L"Updating Source Code Sample Values..."
#define CP_sourceCodeViewProgressViewUpdate L"Updating Source Code View..."

// Functions view:
#define CP_functionsViewModuleDoesntExist "The module %1 does not exist. Source code cannot be displayed."

// Call Graph view:
#define CP_callGraphViewFailedToReadCss "This profile session has no call graph samples. No data to display."

// Global settings:
#define C_STR_cpuProfileSymbolsServerDirectories "Symbol server download directory"

// Project settings:
#define CP_STR_cpuProfileTreePathString L"Profile, CPU Profile"
#define CP_STR_cpuProfileCustomTreePathString L"Profile, CPU Profile, Custom"
#define CP_STR_cpuProfileProjectSettingsDepthMinimal "Minimal (%1 levels)"
#define CP_STR_cpuProfileProjectSettingsDepthLow "Low (%1 levels)"
#define CP_STR_cpuProfileProjectSettingsDepthMedium "Medium (%1 levels)"
#define CP_STR_cpuProfileProjectSettingsDepthHigh "High (%1 levels)"
#define CP_STR_cpuProfileProjectSettingsDepthTooltip "Click to select the call stack depth for time based profiling. A low depth will improve performance."
#define CP_STR_cpuProfileProjectSettingsDepthOtherTooltip "Click to select the call stack depth. A low depth will improve performance."
#define CP_STR_cpuProfileProjectSettingsDepthMaximal "Maximal (%1 levels)"
#define CP_STR_cpuProfileProjectSettingsCallStackCollectionCaption "Call Stack Collection"
#define CP_STR_cpuProfileProjectSettingsCallStackCollection "Collect call stack details"
#define CP_STR_cpuProfileProjectSettingsCallStackCollectionTooltip "Click to collect call stack details while profiling. Notice, this selection will cause a performance overhead."
#define CP_STR_cpuProfileProjectSettingsCallStackFpo "Reproduce missing call stack info"
#define CP_STR_cpuProfileProjectSettingsCallStackFpoTooltip "Perform additional analysis to overcome frame-pointer omission (FPO) \nin 32-bit apps and lack of unwind info in 64-bit. The profiler will store \nadditional data during the profile session and require more time during \npost-session processing."
#define CP_STR_cpuProfileProjectSettingsCallStackCodeExecutedIn "Collect for code executed in:"
#define CP_STR_cpuProfileProjectSettingsCallStackModeTooltip "Click to select the operating system modes for which the call stack will be collected"
#define CP_STR_cpuProfileProjectSettingsCallStackUserSpace "User mode"
#define CP_STR_cpuProfileProjectSettingsCallStackKernelSpace "Kernel mode"
#define CP_STR_cpuProfileProjectSettingsCallStackUserKernelSpaces "User mode and Kernel mode"
#define CP_STR_cpuProfileProjectSettingsCallStackCollectionEvery "Collect call stack every:"
#define CP_STR_cpuProfileProjectSettingsCallStackCollectionEveryTooltip "Click or type the amount of samples that will have collected call stacks.\nIncreasing this number will improve the profiling process performance overhead"
#define CP_STR_cpuProfileProjectSettingsUnwindDepth "Call stack collection depth"
#define CP_STR_cpuProfileProjectSettingsHWScopeCaption "Profile Hardware Scope"
#define CP_STR_cpuProfileProjectSettingsHWScopeDescription "Collect profiling data from the following CPUs:"
#define CP_STR_cpuProfileProjectSettingsCPUAffinityMask "CPU Affinity Mask:"
#define CP_STR_cpuProfileProjectSettingsCPUAffinityMaskTooltip "A binary mask describing the selected CPU cores"
#define CP_STR_cpuProfileProjectSettingTimeBasedSampling "       Time-Based Sampling:"
#define CP_STR_cpuProfileProjectSettingOtherCpuProfiling "       Other CPU Profiling session types:"
#define CP_STR_cpuProfileProjectSettingsCPUPrefix "CPU "
#define CP_STR_cpuProfileProjectSettingsCorePrefix "Core "
#define CP_STR_cpuProfileProjectSettingsCallStackDepthDetails "Sample the call stack with a maximal depth of <b>%1 levels</b>"
#define CP_STR_cpuProfileProjectSettingsCaption "Custom CPU Profile Monitored Events"
#define CP_STR_cpuProfileProjectSettingsMonitoredEvents "Monitored Events"
#define CP_STR_cpuProfileProjectSettingsAvailableEvents "Available Events"
#define CP_STR_cpuProfileProjectSettingsEvents "Custom CPU Profile Monitored Events"
#define CP_STR_cpuProfileProjectSettingsMonitoredEventsDetails "Add an Event to the 'Monitored Events' list to include it in the Custom Profile session"
#define CP_STR_cpuProfileProjectSettingsNameColumn "Name\n"
#define CP_STR_cpuProfileProjectSettingsNameColumnTooltip "Event name"
#define CP_STR_cpuProfileProjectSettingsIntervalColumn "Interval\n"
#define CP_STR_cpuProfileProjectSettingsIntervalColumnTooltip "The interval in which the event is monitored"
#define CP_STR_cpuProfileProjectSettingsUnitMaskColumn "Unit\nMask"
#define CP_STR_cpuProfileProjectSettingsUnitMaskColumnTooltip "Unit Mask"
#define CP_STR_cpuProfileProjectSettingsUsrColumn "User\nMode"
#define CP_STR_cpuProfileProjectSettingsUsrColumnTooltip "Count events occuring in User Mode"
#define CP_STR_cpuProfileProjectSettingsOSColumn "Kernel\nMode"
#define CP_STR_cpuProfileProjectSettingsOSColumnTooltip "Count events occuring in  Kernel Mode"
#define CP_STR_cpuProfileProjectSettingsCustomTypeWarning "*** Note: Multiple events profiling may not work properly on this processor. ***\n\n"
#define CP_STR_cpuProfileProjectSettingsCustomTypeChangeQuestion "Profile custom settings were changed.\nDo you want to change the current profile type to 'Custom Profile'?"
#define CP_STR_cpuProfileProjectSettingsCurrentProfileType "Collect call stack details for current session type - %1"
#define CP_STR_cpuProfileProjectSettingsAddButtonTooltip "Click to add the selected event to the list of monitored events"
#define CP_STR_cpuProfileProjectSettingsRemoveButtonTooltip "Click to remove the selected event to the list of monitored events"
#define CP_STR_cpuProfileProjectSettingsRemoveAllButtonTooltip "Click to clear the list of monitored evetns"

#define CP_STR_cpuProfileProjectSettingsCustomAllEvents "All Events"
#define CP_STR_cpuProfileProjectSettingsCustomHardwareParent "Events by Hardware Source"
#define CP_STR_cpuProfileProjectSettingsCustomTimerEvent "Timer Event"
#define CP_STR_cpuProfileProjectSettingsCustomMilliseconds "milliseconds"
#define CP_STR_cpuProfileProjectSettingsCustomCLUEvent PM_profileTypeCLU
#define CP_STR_cpuProfileProjectSettingsCustomHardwareParentDescription "The complete list of available events for each hardware component"
#define CP_STR_cpuProfileProjectSettingsCustomAllEventsDescription "The complete list of available events"

// Cache Line Utilization:
#define CP_CacheLineUtilizationNotes "<b>Cache Line Utilization Notes</b>"


// Rename disabled message:
#define CP_sessionCannotBeRenamed "Cannot rename session while source code view is open. Please close all source code view tabs and try again."

// Messages:
#define CP_MSG_cpuProfileProjectSettingsTypeChangeToDefaultInfo "The default Call Stack Collection settings of the selected session type are different from the current settings."
#define CP_MSG_cpuProfileProjectSettingsTypeChangeToDefaultQuestion "Do you want to change the settings to the default values?"

// CLU notes
#define CP_CLU_NOTE_SPAN "At least one data access spans multiple cache lines. " \
    "This is usually caused by an access to a data element which is not " \
    "aligned to its natural alignment (i.e., an integer access from an address " \
    "with the lower 2 bits non-zero). It can also occur when accessing an " \
    "element of a packed structure."
#define CP_CLU_NOTE_SPAN_OVER_THRESHOLD "Data accesses span multiple cache lines. " \
    "This is usually caused by an access to a data element which is not " \
    "aligned to its natural alignment (i.e., an integer access from an address " \
    "with the lower 2 bits non-zero). It can also occur when accessing an " \
    "element of a packed structure."
#define CP_CLU_NOTE_LOW_UTILIZATION "The cache line utilization is very low. " \
    "This indicates that the data accessed has low spatial locality " \
    "or the address of the accessed data conflicts with another data " \
    "access address such that the cache line is evicted shortly after " \
    "it is loaded. Consider aligning the data or consolidating " \
    "data such that more data gets loaded into a cache line on " \
    "initial load."
#define CP_CLU_NOTE_MEDIUM_UTILIZATION  "The cache line utilization is low. " \
    "This indicates that the data accessed has low spatial locality " \
    "or the address of the accessed data conflicts with another data " \
    "access address such that the cache line is evicted shortly after " \
    "it is loaded. Consider aligning the data or consolidating " \
    "data such that more data gets loaded into a cache line on " \
    "initial load."
#define CP_CLU_NOTE_LOW_CLU_HIGH_ACCESS_RATE "Although the cache line utilization is very low, it appears there were " \
    "a significant number of accesses to the cache line between evictions. " \
    "Consider grouping accesses to this data together with other data " \
    "items within this scope that exhibit a similar access pattern. "
#define CP_CLU_NOTE_MED_CLU_HIGH_ACCESS_RATE "Although the cache line utilization is low, it appears there were " \
    "a significant number of accesses to the cache line between evictions. " \
    "Consider grouping accesses to this data together with other data " \
    "items within this scope that exhibit a similar access pattern. "
#define CP_CLU_NOTE_COMPULSORY "This eviction was compulsory."
#define CP_CLU_NOTE_BAD_DISASM "There was an error disassembling at least one instruction in this block. " \
    "The data may not be reliable."

// Profile type:
#define CP_STR_ThreadProfileName L"CPU: Thread Profiling"
#define CP_STR_ThreadProfileTypeName "CPU: Thread Profiling"

// File extensions:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define CP_STR_ThreadProfileExtension "etw"
    #define CP_STR_ThreadProfileExtensionW L"etw"
    #define CP_STR_ThreadProfileExtensionSearchString "*.etw"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define CP_STR_ThreadProfileExtension "capref"
    #define CP_STR_ThreadProfileExtensionW L"capref"
    #define CP_STR_ThreadProfileExtensionSearchString "*.capref"
#endif
#define CP_STR_ThreadProfileFileExtensionCaption "CPU: Thread Profiling"

/// Project settings:
#define CP_STR_projectSettingExtensionName L"ThreadProfile"
#define CP_STR_projectSettingExtensionNameASCII "ThreadProfile"
#define CP_STR_projectSettingExtensionDisplayName L"Profile, Thread Profile"
#define CP_STR_projectSettingsDummyValueXML L"DummyValue"
#define CP_STR_projectSettingsDummyValue "Dummy Value"
#define CP_STR_projectSettingsDummyValueDefault "Default Dummy Value"


/// Menu actions command ids:
#define CP_STR_menu_command_1 L"Thread Profile menu command 1"
#define CP_STR_menu_command_2 L"Thread Profile menu command 2"
#define CP_STR_menu_command_3 L"Thread Profile menu command 3"
#define CP_STR_menu_caption L"Thread Profiling"

// MDI views:
#define CP_STR_sessionViewCaption L" (Thread)"

// Session view
#define CP_STR_SessionTimelineTitle "Timeline"

#define CP_STR_TreeNodeOverview L"Overview"
#define CP_STR_TreeNodeTimeline L"Timeline"

#define CP_STR_logStartProfiling L"Thread Profiling started"
#define CP_STR_logStopProfiling L"Thread Profiling stopped"

/// Tread states as strings:
#define CP_STR_ThreadStateInitialized "Initialized"
#define CP_STR_ThreadStateReady "Ready"
#define CP_STR_ThreadStateRunning "Running"
#define CP_STR_ThreadStateStandBy "Standby"
#define CP_STR_ThreadStateTerminated "Terminated"
#define CP_STR_ThreadStateWaiting "Waiting"
#define CP_STR_ThreadStateTransition "Transition"
#define CP_STR_ThreadStateDeferredReady "DeferredReady"
#define CP_STR_ThreadStateGatewait "Gatewait"

// Settings dialog:
#define CP_STR_SettingsLinkLAbel "<a href='change_counter'>%1, %2 out of %3 Processes, %4 out of %5 Threads</a>"
#define CP_STR_SettingsLAbelStyleSheet "QLabel { background-color: rgb(236, 236, 236); }"
#define CP_STR_SettingsTitle "<html><body><b>%1</b></body></html>"
#define CP_STR_SettingsTitleCores "Cores"
#define CP_STR_SettingsTitleProcss "Processes & Threads"
#define CP_STR_SettingsOkButtonLabel "Ok"
#define CP_STR_SettingsCancelButtonLabel "Cancel"
#define CP_STR_SettingsAllCoresCbText "All"
#define CP_STR_SettingsAllCoresStr "All Cores"
#define CP_STR_SettingsNoCoresStr "No Cores selected"
#define CP_STR_SettingsWindowTitle "Thread Profile Settings"

/// Overview:
#define CP_STR_ThreadsOverviewTableProcess "Process"
#define CP_STR_ThreadsOverviewTablePID "PID"
#define CP_STR_ThreadsOverviewTableTID "TID"
#define CP_STR_ThreadsOverviewTableExecutionTime "Execution Time"

/// Timeline:
#define CP_STR_ThreadsTimelineProcessesProgress "Extracting data collected for %1 processes"
#define CP_STR_ThreadsTimelineSingleProcessProgress "Extracting data collected for process: %1"
#define CP_STR_ThreadsTimelineThreadProgress "Extracting samples for thread %1"
#define CP_STR_ThreadsTimelineThreadProgressProgress "Processing samples for thread %1 (%2 of %3)"
#define CP_STR_ThreadsTimelineCoreSubBranch "Core %1"
#define CP_STR_ThreadsTimelineThreadSubBranch "Thread %1"

/// Timeline tooltips:
#define CP_STR_ThreadsTimelineSampleTooltipItemFormat "%1"
#define CP_STR_ThreadsTimelineSampleTooltipDuration "Duration:"
#define CP_STR_ThreadsTimelineSampleTooltipStartTime "Start time:"
#define CP_STR_ThreadsTimelineSampleTooltipEndTime "End time:"
#define CP_STR_ThreadsTimelineSampleTooltipThread "Thread:"
#define CP_STR_ThreadsTimelineSampleTooltipCore "Core:"
#define CP_STR_ThreadsTimelineSampleTooltipState "State:"
#define CP_STR_ThreadsTimelineSampleTooltipWaitReason "Wait reason:"

/// Thread view control panel:
#define CP_STR_ControlPanelDisplayTop "Display top"
#define CP_STR_ControlPanelSignificantThreads "significant threads (out of total %1 threads), based on the entire session"


// information view
#define CP_STR_InfoViewExecutionHeader L"Execution Information"
#define CP_STR_InfoViewViewHTMLLineStructure L"<b>%ls:</b> %ls"
#define CP_STR_InfoViewTargetPath L"Target path"
#define CP_STR_InfoViewWorkingDirectory L"Working directory"
#define CP_STR_InfoViewDataFolder L"Data folder"
#define CP_STR_InfoViewCommandLineaArguments L"Command line arguments"
#define CP_STR_InfoViewEnvironmentVariables L"Environment Variables"
#define CP_STR_InfoViewCallStackSampling L"Call stack sampling"
#define CP_STR_InfoViewStackUnwindDepth L"Stack unwind depth"
#define CP_STR_InfoViewProfileDetailsHeader L"Profile Details"
#define CP_STR_InfoViewProfileSessionType L"Profile session type"
#define CP_STR_InfoViewProfileStarted L"Profile started"
#define CP_STR_InfoViewProfileEnded L"Profile ended"
#define CP_STR_InfoViewTotalProcesses L"Total processes in Profile"
#define CP_STR_InfoViewTotalThreads L"Profile threads in profile"
#define CP_STR_InfoViewMonitoredEvent L"Monitored Events"
#define CP_STR_InfoViewThreadProfiling L"Thread profiling"

/// Threads profile legend:
#define CP_STR_ThreadsLegendRunningOn "Running On"
#define CP_STR_ThreadsLegendPrefix "...."
#define CP_STR_ThreadsLegendCaption "Legend"

#define CP_Str_TranslateErrorNoData "The session contains no data records. Please make sure that the profiling is not paused for the entire duration, and run another session"
#define CP_Str_TranslateErrorInvalidData "The session data file may be corrupted."


#endif //_CASTRINGCONSTANSTS_H
