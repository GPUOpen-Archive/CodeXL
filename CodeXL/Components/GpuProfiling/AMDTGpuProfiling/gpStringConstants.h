//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __GPSTRINGCONSTANTS_H
#define __GPSTRINGCONSTANTS_H

#define GP_Str_ProfileSessionGPUPrefix "GPU: "
#define GP_Str_ProfileSessionHTMLHeading L"GPU Profile Session"
#define GP_Str_FrameAnalysisSessionHTMLHeading L"Frame Analysis Session"
#define GP_Str_HSA "HSA"
#define GP_Str_CL "CL"
#define GP_Str_LocalHost "Local host"


// Session and other file name
#define GP_Str_SessionFileNameFormat "Profile %1:%2:%3"
#define GPU_STR_FrameTraceFileNameFormat "%1_Frame_%2"
#define GPU_STR_FrameTraceFileNameFormatW L"%ls_Frame_%d"
#define GPU_STR_FrameFileNameFormat "%1_Frame_%2.%3"
#define GPU_STR_FrameFileNameFormat "%1_Frame_%2.%3"
#define GPU_STR_FramePerfCountersFileNameFormat "%1_Frame_%2_Profile_%3_%4_%5"
#define GPU_STR_FrameSubFolderNameFormat L"Frame%d"
#define GPU_STR_FrameSubFolderNameFormatA "Frame%1"
#define GPU_STR_FrameSubFolderNamePrefix "Frame"
#define GPU_STR_FullObjectDatabase L"\\FullObjectDatabase.xml"

// Session views names
#define GP_Str_PerfCountersViewName "Performance Counter View"
#define GP_Str_AppTraceViewName "Application Trace View"
#define GP_Str_DXAppTraceViewName "DX Application Trace View"
#define GP_Str_OccupancyViewName "Kernel Occupancy Viewer"
#define GP_Str_OccupancyCodeViewerName "Code Viewer"
#define GP_Str_OccupancyWindowCaption "Kernel Occupancy (%1)"

/// Atp file properties
#define GP_Str_ATPPropertyDisplayName "DisplayName"

/// Command list types
#define GP_Str_CommandListTypeDirect "Direct"
#define GP_Str_CommandListTypeBundle "Bundle"
#define GP_Str_CommandListTypeCompute "Compute"
#define GP_Str_CommandListTypeCopy "Copy"

// Counter selection settings
#define GP_Str_CounterSelectionMainCaptionGeneral "General"
#define GP_Str_CounterSelectionPreformanceCountersSelection "Performance Counters Selection"
#define GP_Str_CounterSelectionProfileSpecificKernels "Profile Specific Kernels"
#define GP_Str_CounterSelectionAdvancedOptions   "Advanced Options"
#define GP_Str_CounterSelectionLoadSelection "Load Selection"
#define GP_Str_CounterSelectionSaveSelection "Save Selection"
#define GP_Str_CounterSelectionGenerateOccupancyDetails "Generate occupancy information for each OpenCL or HSA kernel profiled"
#define GP_str_GpuTimeCoolect "Measure kernel execution time (requires an additional pass)"
#define GP_str_GpuTimeToolTip "Perform 1 additional pass to measure the kernel execution time with high precision."
#define GP_str_CollectingMultiCountersLabel "Collecting Multiple performance counters may require executing the kernel more than once. If multiple passes are required, the profiler \nstores the OpenCL read/write buffers prior to kernel execution and restores them afterwards."
#define GP_str_MultiPassNote "      Note: Profiling OpenCL kernels that take Shared Virtual Memory pointers or pipes as arguments is disabled during multi-pass collection"
#define GP_str_PerfCounterNotAvailable "GPU Performance Counters are not available on the host platform. GPU Performance Counters are available only for AMD devices with a Catalyst driver installed"
#define GP_str_ProfileSpecificKernelsDesc  "Profile only these kernels (names are case-sensitive, separated by semi-colons). \nLeave empty to profile all kernels."
#define GP_Str_CounterSelectionAPITypeDesc "Collect counters for:"
#define GP_Str_CounterSelectionXInitThreadsDesc "Init the threading support before beginning a profile. Applicable only to multi-threaded applications."
#define GP_Str_CounterSelectionHSAPassesWarning "You selected a counter combination that requires %1 passes. Only a single pass can be used to collect HSA counters. Some counters will be removed from the selection."

// Performance counters data view
#define GP_Str_CountersDataSummaryAPICalls "# of API calls %1"
#define GP_Str_CountersDataSummaryDrawCalls "# of Draw calls %1"
#define GP_Str_CountersDataSummaryCpuTime "CPU Time: %1"
#define GP_Str_CountersDataSummaryGpuTime "GPU Time: %1"
#define GP_Str_CountersDataSummaryCpuTimePercentageDrawCalls "% CPU time in draw calls %1"
#define GP_Str_CountersDataSummaryGpuBusy "% GPU busy %1"

// Error and status messages
#define GP_Str_ErrorUnableToDelete "Unable to delete"
#define GP_Str_ErrorInsufficientMemory "Insufficient memory"
#define GP_Str_ErrorUnableToLoad "Unable to load %1."
#define GP_Str_ErrorWhileLoadingSession "Failed to load the session data"
#define GP_Str_ErrorWhileLoadingFrame "Failed to load the frame data"
#define GP_Str_MessageDeleted "Deleted"
#define GP_Str_ErrorFailedToConnectToGraphicServer "Failed to connect to the graphic server"
#define GP_Str_ErrorFailedToOpenFrameAnalysisArchive L"Failed to open frame analysis archive %1"
#define GP_Str_ErrorTimelineCannotBeRetrievedWhileProcessIsRunning L"Timeline data cannot be retrieved from the server while profiling is in progress."
#define GP_Str_WarningRaptrProcessIsRunning "%1 is running and may conflict with CodeXL graphics profiling. Raptr is not required for system operation. Would you like to kill raptr.exe?"
#define GP_Str_WarningProjectProcessIsRunning "Project executable %1 is already running and may conflict with CodeXL graphics profiling. Would you like to kill it?"
#define GP_Str_ErrorCannotRenameServerUnavailable "The session cannot be renamed. CodeXL remote server must be available when a session is renamed. Please make sure that CodeXL remote agent is running on %1."
#define GP_Str_WarningMaxItems "Max items limitation reached, Some data may be missing, try closing unnecessary sessions."
#define GP_Str_WarningCantDisplayDetailedRibbon "Number of API calls exceeds detailed ribbon capacity"

// Application trace settings
#define GP_Str_ProjectSettingsMainCaption       "Application Timeline Trace"
#define GP_Str_ProjectSettingsOpenCLTraceOptions "OpenCL Trace Options"
#define GP_Str_ProjectSettingsAPITraceOptions   "API Trace Options"
#define GP_Str_AppTraceAPIToTrace               "API to trace"
#define GP_Str_AppTraceCollpase                 "Collapse consecutive identical clGetEventInfo calls"
#define GP_Str_AppTraceEnableNavigation         "Enable navigation to source code (high overhead)"
#define GP_Str_AppTraceAlwaysShowAPIErrorCodes  "Always show API error codes"
#define GP_Str_AppTraceGenerateSummary          "Generate summary pages"
#define GP_Str_AppTraceMaxAPI                   "Maximum number of APIs to trace:"
#define GP_Str_AppTraceWriteTraceData           "Write trace data in intervals during program execution (ms):"
#define GP_Str_AppTraceTraceDataInterval        "Interval at which to write trace data during program execution (ms):"
#define GP_Str_MarkersFileExtension             "clperfmarker"
#define GP_Str_NewerTraceSession                "This Application Trace session was generated by a newer version of CodeXL. Please update to a newer version of CodeXL to view this session."
#define GP_Str_InvalidDebugInfo                 "This may be caused by a lack of or invalid debugging information."
#define GP_Str_AppTerminatedUnexpectedly        "\n\nThis may also occur if the application terminated unexpectedly before the profiler wrote out any symbol information.\n%1"
#define GP_Str_NoSourceInfoForSelectedAPI       "No source information is available for the selected API.\n\n%1"
#define GP_Str_CantAccessSourceForSelectedAPI   "Unable to go to source for the selected API\n\nFile not found: %1"
#define GP_Str_AppMadeNoCallsToEnabledAPI       "\n\nIt is possible that the application did not call any of the APIs enabled in the \"%1\" option on the \"%2\" project setting page.\nTry enabling more (or all) APIs and profiling the application again."
#define GP_Str_UnableToLoadTimelineData         "Unable to load timeline data. %1"
#define GP_Str_ProjectSettingsOpenCLAPI         "OpenCL"
#define GP_Str_ProjectSettingsHSAAPI            "HSA"
#define GP_Str_ProjectSettingsAPITypeDesc       "Profile applications that use:"
#define GP_Str_ProjectSettingsOpenCLAPITooltip  "Profile applications that use OpenCL"
#define GP_Str_ProjectSettingsHSAAPITooltip     "Profile applications that use HSA-enabled languages (C++ AMP, OpenMP, etc'). Currently available only on Linux"


#define GP_Str_ProjectSettingsAllCLRulesItemName "All OpenCL Rules"
#define GP_Str_ProjectSettingsAllHSARulesItemName "All HSA Rules"
#define GP_Str_ProjectSettingsAllCLAPIsItemName "All OpenCL APIs"
#define GP_Str_ProjectSettingsAllHSAAPIsItemName "All HSA APIs"

#define GP_profileTypePerformanceCountersWithPrefix         "GPU: Performance Counters"
#define GP_profileTypeApplicationTraceWithPrefix            "Application Timeline Trace"
#define GP_profileTypePerformanceCountersPrefix             L"GPU: "

#define GPU_str_profileTypePerformanceCounters               "Performance Counters"
#define GPU_str_profileTypeApplicationTrace                 "Application Timeline Trace"

#define GPU_STR_COUNTER_PROJECT_SETTINGS                    L"GPUPerformanceCounters"
#define GPU_STR_COUNTER_PROJECT_SETTINGS_DISPLAY            L"GPU Profile: Performance Counters"
#define GPU_STR_COUNTER_PROJECT_SETTINGS_TAB_NAME           L"GPU Pro&file: Performance Counters"
#define GPU_STR_APP_TRACE_PROJECT_TREE_PATH_STR             L"Profile, Application Timeline Trace"
#define GPU_STR_TRACE_PROJECT_SETTINGS                      L"OpenCLAppTrace"
#define GPU_STR_TRACE_PROJECT_SETTINGS_DISPLAY              L"GPU Profile: Application Trace"
#define GPU_STR_TRACE_PROJECT_TREE_PATH_STR                 L"Profile, GPU Profile: Perf. Counters"
#define GPU_NARROW_STR_TRACE_PROJECT_SETTINGS_DISPLAY       "Application Timeline Trace"
#define GPU_STR_TRACE_PROJECT_SETTINGS_TAB_NAME             L"GPU Profile: Application &Trace"
#define GPU_STR_GENERAL_SETTINGS                            L"GPUProfiler"
#define GPU_STR_GENERAL_SETTINGS_DISPLAY                    L"GPU Profile"
#define GPU_STR_STARTING_REMOTE_SESSION                     L"GPU Profile: Attempting to connect to the remote agent..."
#define GPU_STR_REMOTE_HANDSHAKE_FALIURE                    L"Failed executing the handshake with the agent."
#define GPU_STR_REMOTE_HANDSHAKE_UNKNOWN_FAILURE            L"Unknown error on handshake with remote agent."
#define GPU_STR_REMOTE_TARGET_APP_NOT_FOUND                 "The target application does not exist on the remote machine. Session is aborted."
#define GPU_STR_PORT_NNN                                    L"PORT_NNNN"
#define GPU_STR_REMOTE_TARGET_APP_PORT_OCCUPIED             L"Session aborted - the CodeXL graphics server requires port " GPU_STR_PORT_NNN L" but it is already in use by another application.\nPlease close the application that acquired this port or use the CodeXL project settings to specify a different port number"

#define GPU_STR_REMOTE_TARGET_APP_ALREADY_RUNNING           "Session aborted. It is not possible to run 2 frame analysis sessions concurrently on the same host. Please stop the first session and run again."
#define GPU_STR_REMOTE_AGENT_VERSION_NOT_RETRIEVED          L"Unable to retrieve CodeXL version of CodeXL Remote Agent."
#define GPU_STR_REMOTE_AGENT_INIT_FAILURE                   L"Failed to initialize CodeXL Remote Agent."
#define GPU_STR_REMOTE_AGENT_INIT_FAILURE_WITH_CTX          L"Failed to initialize CodeXL Remote Agent from GPU profiling context."
#define GPU_STR_REMOTE_SESSION_TERMINATION_FAILURE_WITH_CTX L"Unable to terminate the whole remote session (from GPU profiling context)."
#define GPU_STR_REMOTE_SESSION_TERMINATION_FAILURE          L"Failed to terminate the current seession before another attempt."
#define GPU_STR_REMOTE_CONNECTION_FAILURE                   L"Unable to connect to CodeXL Remote Agent.\nPlease verify that:\n1. CodeXL and CodeXL Remote Agent are not blocked by a firewall.\n2. The given IP address and ports are valid, and the remote machine is accessible from the local machine.\n3. CodeXL Remote Agent is running on the remote machine."
#define GPU_STR_LOCAL_REMOTE_CONNECTION_FAILURE             L"Unable to establish a connection with the CodeXL Remote Agent.\nPlease verify that firewall settings allow CodeXL and CodeXLRemoteAgent to communicate over the network, even for local scenarios."

#define GPU_STR_REMOTE_COMMUNICATION_FAILURE                L"Communication failure: Please verify that both client and remote machines are connected to the network."
#define GPU_STR_REMOTE_PROFILE_IN_PROGRESS_PREFIX           L"Remote GPU profile in progress on "
#define GPU_STR_REMOTE_PROFILING_TASK_THREAD_NAME           L"AsyncProgressUpdater"

/// String constants for the project settings XML:
#define GPU_STR_ProjectSettingsGenerateOccupancy L"GenerateOccupancyInfo"
#define GPU_STR_ProjectSettingsShowErrorCode L"AlwaysShowAPIErrorCode"
#define GPU_STR_ProjectSettingsCollapseClGetEventInfo L"CollapseAllclGetEventInfoCalls"
#define GPU_STR_ProjectSettingsEnableNavigation L"EnableNavigationToSourceCode"
#define GPU_STR_ProjectSettingsGenerateSummaryPage L"GenerateSummaryPage"
#define GPU_STR_ProjectSettingsAPIsToFilter L"APIsToTrace"
#define GPU_STR_ProjectSettingsMaxAPIs L"MaximumNumberOfAPIs"
#define GPU_STR_ProjectSettingsRulesTree L"RuleTree"
#define GPU_STR_ProjectSettingsAPIsFilterTree L"APIsFilterTree"
#define GPU_STR_ProjectSettingsWriteDataTimeOut L"WriteDataTimeOut"
#define GPU_STR_ProjectSettingsTimeOutInterval L"TimeOutInterval"
#define GPU_STR_ProjectSettingsNumberOfCountersSelected L"NumberOfCountersSelected"
#define GPU_STR_ProjectSettingsLoadSelection L"LoadSelection"
#define GPU_STR_ProjectSettingsSaveSelection L"SaveSelection"
#define GPU_STR_ProjectSettingsCounterTree L"CounterTree"
#define GPU_STR_ProjectSettingsGpuTimeCollect L"GpuTimeCollect"
#define GPU_STR_ProjectSettingsCallXInitThreads L"CallXInitThreads"
#define GPU_STR_ProjectSettingsSpecificKernels L"ProfileSpecificKernels"
#define GPU_STR_ProjectSettingsIsRemoteSession L"IsRemoteSession"
#define GPU_STR_ProjectSettingsAPIType L"APIType"
#define GPU_STR_ProjectSettingsAPITypeOpenCL L"OpenCL"
#define GPU_STR_ProjectSettingsAPITypeHSA L"HSA"

#define GPU_STR_ProjectSettingsSpecificKernelErrMsg L"Profile Specific Kernels contains invalid characters.\nKernel name should consist of characters, digits and underscore and can not start with a digit.\nKernels should also be semi-colon delimited."

// Project settings rules strings:
#define GPU_STR_ProjectSettingsRulesWarnings "Warnings"
#define GPU_STR_ProjectSettingsRulesErrors "Errors"
#define GPU_STR_ProjectSettingsRulesBestPractices "Best Practices"


#define GPU_STR_ProjectSettingsRulesWarning1Name "RefTracker"
#define GPU_STR_ProjectSettingsRulesWarning1DisplayName "Detect resource leaks"
#define GPU_STR_ProjectSettingsRulesWarning1Description "Track the reference count for all OpenCL objects, and report any objects which are never released."

#define GPU_STR_ProjectSettingsRulesBestPractice1Name "BlockingWrite"
#define GPU_STR_ProjectSettingsRulesBestPractice1DisplayName "Detect unnecessary blocking writes"
#define GPU_STR_ProjectSettingsRulesBestPractice1Description "Detect unnecessary blocking write operations."

#define GPU_STR_ProjectSettingsRulesBestPractice2Name "BadWorkGroupSize"
#define GPU_STR_ProjectSettingsRulesBestPractice2DisplayName "Detect non-optimized work size"
#define GPU_STR_ProjectSettingsRulesBestPractice2Description "Detect clEnqueueNDRangeKernel calls which specify a global or local workgroup size which is non-optimal for AMD Hardware."

#define GPU_STR_ProjectSettingsRulesError1Name "RetCodeAnalyzer"
#define GPU_STR_ProjectSettingsRulesError1DisplayName "Detect failed API calls"
#define GPU_STR_ProjectSettingsRulesError1Description "Detect OpenCL API calls that do not return CL_SUCCESS.\n Some of the return codes may not be detected unless \"Always show API error codes\" option is checked."

#define GPU_STR_ProjectSettingsRulesBestPractice3Name "DataTransferAnalyzer"
#define GPU_STR_ProjectSettingsRulesBestPractice3DisplayName "Detect non-optimized data transfer"
#define GPU_STR_ProjectSettingsRulesBestPractice3Description "1. Detect Non-Fusion APU access to Device-Visible Host Memory directly.\n2. Detect Host-Visible Device Memory read back to CPU directly."

#define GPU_STR_ProjectSettingsRulesBestPractice4Name "SyncAnalyzer"
#define GPU_STR_ProjectSettingsRulesBestPractice4DisplayName "Detect redundant synchronization"
#define GPU_STR_ProjectSettingsRulesBestPractice4Description "Detect redundant synchronization which results in low host and device utilization."

#define GPU_STR_ProjectSettingsRulesWarning2Name "DeprecatedFunctionAnalyzer"
#define GPU_STR_ProjectSettingsRulesWarning2DisplayName "Detect deprecated API calls"
#define GPU_STR_ProjectSettingsRulesWarning2Description "Detect calls to OpenCL API functions that have been deprecated in recent versions of OpenCL"

// String constants for the GeneratedFileHeader namespace
#define GPU_STR_FileHeader_ProfileFileVersion   "ProfileFileVersion"
#define GPU_STR_FileHeader_TraceFileVersion     "TraceFileVersion"
#define GPU_STR_FileHeader_ProfilerVersion      "ProfilerVersion"
#define GPU_STR_FileHeader_GpSessionVersion     "GpSessionVersion"
#define GPU_STR_FileHeader_Application          "Application"
#define GPU_STR_FileHeader_ApplicationArgs      "ApplicationArgs"
#define GPU_STR_FileHeader_UserTimer            "UserTimer"
#define GPU_STR_FileHeader_PlatformVendor       "Platform Vendor"
#define GPU_STR_FileHeader_PlatformName         "Platform Name"
#define GPU_STR_FileHeader_PlatformVersion      "Platform Version"
#define GPU_STR_FileHeader_CLDriverVersion      "CLDriver Version"
#define GPU_STR_FileHeader_CLRuntimeVersion     "CLRuntime Version"
#define GPU_STR_FileHeader_NumberAppAddressBits "NumberAppAddressBits"
#define GPU_STR_FileHeader_OSVersion            "OS Version"

// Trace View:
#define GPU_STR_TraceViewclGetEventInfo "clGetEventInfo"
#define GPU_STR_TraceViewHostThreadBranchName "Host Thread %1"
#define GPU_STR_TraceViewSummary "Summary"
#define GPU_STR_TraceViewGoToSource "Go to &source code"
#define GPU_STR_TraceViewZoomTimeline "&Show in timeline"
#define GPU_STR_TraceViewExpandAll "&Expand all"
#define GPU_STR_TraceViewCollapseAll "Co&llapse all"
#define GPU_STR_TraceViewCpuDevice "CPU_Device"
#define GPU_STR_TraceViewOpenCL "OpenCL"
#define GPU_STR_TraceViewHSA "HSA"
#define GPU_STR_DX12Api "DX12"
#define GPU_STR_VulkanApi "Vulkan"
#define GPU_STR_TraceViewGPU "GPU Queues"
#define GPU_STR_TraceViewCPU "CPU"
#define GPU_STR_TraceViewThread "Thread"
#define GPU_STR_TraceViewHSAPerfMarker "HSAPerfMarker"
#define GPU_STR_TraceViewQueueRow "Queue %1 - %2 (%3)"
#define GPU_STR_TraceViewCPUThreads "CPU Threads (%1 of %2)"
#define GPU_STR_TraceViewCPULaterFull "CPU PROCESSING LATER FRAME/S"
#define GPU_STR_TraceViewCPULaterShort "LATER FRAME/S"
#define GPU_STR_TraceViewGPUEarlierFull "GPU PROCESSING EARLIER FRAME/S"
#define GPU_STR_TraceViewGPUEarlierShort "EARLIER FRAME/S"

// WholeCmdBuf
#define GPU_STR_TraceViewWholeBufferTraceStr "Vulkan_WholeCmdBuf"

// Trace table captions
#define GP_STR_TraceTableColumnIndex "Index"
#define GP_STR_TraceTableColumnInterface "Interface"
#define GP_STR_TraceTableColumnCommandList "CmdList #"
#define GP_STR_TraceTableColumnCommandBuffer "CmdBuffer #"
#define GP_STR_TraceTableColumnCall "Call"
#define GP_STR_TraceTableColumnParameters "Parameters"
#define GP_STR_TraceTableColumnResult "Result"
#define GP_STR_TraceTableColumnDeviceBlock "Device Block"
#define GP_STR_TraceTableColumnKernelOccupancy "Kernel Occupancy"
#define GP_STR_TraceTableColumnCPUTime "CPU Time"
#define GP_STR_TraceTableColumnGPUTime "GPU Time"
#define GP_STR_TraceTableColumnDeviceTime "Device Time"
#define GP_STR_TraceTableColumnStartTime "Start Time"
#define GP_STR_TraceTableColumnEndTime "End Time"

/// Progress messages
#define GPU_STR_TraceViewLoadingTraceProgress L"Loading trace data..."
#define GPU_STR_TraceViewLoadingTraceTableItemsProgress L"Loading items..."
#define GPU_STR_TraceViewLoadingOccupancyProgress L"Loading occupancy data..."
#define GPU_STR_TraceViewLoadingAPITimelineItems L"Loading API items to timeline..."
#define GPU_STR_TraceViewLoadingGPUTimelineItems L"Loading GPU items to timeline..."
#define GPU_STR_TraceViewLoadingPerfMarkersTimelineItems L"Loading Performance Markers to timeline..."
#define GPU_STR_TraceViewLoadingFASession L"Loading Frame analysis session..."
#define GPU_STR_TraceViewLoadingThreadsConcurrency L"Calculating threads concurrency..."

// Profile Manager error messages
#define GPU_STR_ERR_NoCountersSelected "Unable to profile. At least one counter needs to be selected.\nCounter selection can be modified in the Project Settings dialog."
#define GPU_STR_ERR_FileNotSpecified   "Unable to profile.  No %1 file specified"
#define GPU_STR_ERR_InvalidExecutable  "Unable to profile. The specified executable is not a valid %1 file: \n%2"
#define GPU_STR_ERR_FileMissing        "Unable to profile. The specified %1 does not exist: \n%2"

// User messages:
#define GPU_STR_APICallsAmountExceedsTheLimitQuestion "The session contains more than %1 API calls.\n" \
    "Do you want to analyze only the first %1 API calls (recommended for performance reasons)?\n Selecting 'No' will analyze all API calls."
#define GPU_STR_SessionStopConfirm "Closing this window will stop the running frame analysis session. Click 'Yes' to close the window and end the session, or 'No' to keep the session running."

/// CSV export file names:
#define GPU_CSV_FileNameFormat "CodeXL%1_%2"
#define GPU_CSV_FileNameErrorsWarnings "ErrorsWarnings"
#define GPU_CSV_FileNameAPISummary "APISummary"
#define GPU_CSV_FileNameContextSummary "ContextSummary"
#define GPU_CSV_FileNameTop10KernelSummary "Top10KernelSummary"
#define GPU_CSV_FileNameKernelSummary "KernelSummary"
#define GPU_CSV_FileNameTop10DataSummary "Top10DataSummary"
#define GPU_CSV_FileNameTraceView "Trace"
#define GPU_RulesFullFileName "Rules.rls"
#define GPU_PerformanceCountersFullFileName ".CounterFile.csl"
#define GPU_RulesFileName L"Rules"
#define GPU_SummaryFileName L"*Summary.html"
#define GPU_APISummaryFileName L"*APISummary.html"
#define GPU_HSAAPISummaryFileName L"*HSAAPISummary.html"
#define GPU_CLAPISummaryFileName L"*CLAPISummary.html"
#define GPU_ContextSummaryFileName L"*ContextSummary.html"
#define GPU_KernelSummaryFileName L"*KernelSummary.html"
#define GPU_Top10DataTransferSummaryFileName L"*Top10DataTransferSummary.html"
#define GPU_Top10KernelSummaryFileName L"*Top10KernelSummary.html"
#define GPU_BestPracticesFileName L"*BestPractices.html"

/// GPU Profile file extensions
#define GP_CSV_FileExtension ".csv"
#define GP_ATP_FileExtension ".atp"
#define GP_LTR_FileExtension ".ltr"
#define GP_AOR_FileExtension ".aor"
#define GP_OVR_FileExtension ".cxlfovr"
#define GP_HTML_FileExtension ".html"
#define GP_Occupancy_FileExtension ".occupancy"
#define GP_Dashbord_FileExtension ".cxldsh"
#define GP_HTML_FileExtensionW L"html"
#define GP_Occupancy_FileExtensionW L"occupancy"
#define GP_PerformanceCounters_FileExtension "cxlfc"
#define GP_thumbnailImageExtension "png"

#define GP_CSV_FileExtensionW L"csv"
#define GP_ATP_FileExtensionW L"atp"
#define GP_LTR_FileExtensionW L"ltr"
#define GP_AOR_FileExtensionW L"aor"
#define GP_Dashbord_FileExtensionW L"cxldsh"
#define GP_Overview_FileExtensionW L"cxlfovr"
#define GP_Overview_FileExtension "cxlfovr"
#define GP_PerformanceCounters_FileExtensionW L"cxlfc"
#define GP_Image_FileExtensionW L"jpg"
#define GP_ThumbnailFileExtension "png"
#define GP_ThumbnailFileExtensionW L"png"

// Process Monitor Types
#define GPU_STR_ProcessMonitorRunType_Profile L"GPU profile in progress"
#define GPU_STR_ProcessMonitorRunType_GenSummary L"Generating Summary Pages"
#define GPU_STR_ProcessMonitorRunType_GenOccupancy L"Generating Occupancy Page"
#define GPU_STR_ProcessMonitorRunType_Geneneric L"Executing Application"

// Frame Analysis Mode
#define GPU_STR_executionMode L"Frame Analysis Mode"
#define GPU_STR_executionModeMenu L"&Frame Analysis Mode"
#define GPU_STR_SwitchToAnalyzeMode L"Switch to &Frame Analysis Mode"
#define GPU_STR_executionModeAction L"Frame Analysis"
#define GPU_STR_executionModeVerb   L"Frame Analysis"
#define GPU_STR_executionSesionType L"Frame Analysis"
#define GPU_STR_executionModeStatusbarString L"Frame Analysis Mode - Switch to Frame Analysis Mode"
#define GPU_STR_executionModeDescription L"Perform frame analysis of an image."
#define GPU_STR_executionModeStart L"&Start Frame Analysis"
#define GPU_STR_executionModeStartStatus L"Start an application and capture a frame for analysis"
#define GPU_STR_executionModeCapture L"&Capture a Frame"
#define GPU_STR_executionModeCaptureStatus L"&Capture a frame for analysis"
#define GPU_STR_executionModeStop L"Sto&p Frame Analysis"
#define GPU_STR_executionModeStopStatus L"Stop the executed application"
#define GPU_STR_executionModeRefreshFromServer L"&Refresh sessions from server"
#define GPU_STR_executionModeRefreshFromServerStatus L"Refresh sessions from server"
#define GPU_STR_executionModeSetting L"Frame Analysis Se&ttings"
#define GPU_STR_executionModeSettingsStatus L"Open the Frame Analysis settings dialog"

/// Tree item text:
#define GPU_STR_TreeNodeDashboard L"Dashboard"
#define GPU_STR_TreeNodeTimeline L"Timeline"
#define GPU_STR_TreeNodeOverview L"Overview"
#define GPU_STR_TreeNodeFrame "Frame # %1"
#define GPU_STR_TreeNodePerformanceProfile L"Performance Profile"
#define GPU_STR_TreeNodeImage L"Image"
#define GPU_STR_TreeNodeObjectInspector L"Object Inspector"

/// Mode settings settings
#define GPU_STR_projectSettingExtensionDisplayName L"Frame Analysis"

/// Project settings:
#define GPU_STR_projectSettingExtensionName L"FrameAnalysis"
#define GPU_STR_projectSettingExtensionNameASCII "FrameAnalysis"
#define GPU_STR_projectSettingsAutomaticXMLField L"AutoSelect"
#define GPU_STR_projectSettingsConnectionXMLField L"Connection"
#define GPU_STR_projectSettingsPortNumberXMLField L"PortNumber"
#define GPU_STR_projectSettingsProcessNumberXMLField L"ProcessNumber"
#define GPU_STR_projectSettingsProcessNameXMLField L"ProcessName"
#define GPU_STR_projectSettingsNumberFramesToCaptureXMLField L"NumberFrames"
#define GPU_STR_projectSettingsAPISelection "API Selection"
#define GPU_STR_projectSettingsAutomaticConnect "Automatically connect to"
#define GPU_STR_projectSettingNumberOfFramesToCapture "Number of frames to Capture at once"
#define GPU_STR_projectSettingsServerConnectionPort "Server connection port"
#define GPU_STR_projectSettingsComboProcessNumberOption "Launched Process #"
#define GPU_STR_projectSettingsComboAPIInProcessOption "First active API in [ProcessName]"
#define GPU_STR_projectSettingsComboAPIOption "First active [API]"

#define GPU_STR_PropertiesExecutionInformationSA L"To start Frame Analysis, select 'Start Frame Analysis' from the Frame Analysis menu, or click the 'Start Frame Analysis' toolbar button."
#define GPU_STR_PropertiesExecutionInformationVS L"To start Frame Analysis, select 'Start Frame Analysis' from CodeXL menu, or click the 'Start Frame Analysis' toolbar button."

#define GPU_STR_atrHeader L"//API:DX12\n=====CodeXL dx12 API Trace Output=====\n"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define GPU_STR_perfStudioServer64 L"CXLGraphicsServer-x64" GDT_DEBUG_SUFFIX_W
    #define GPU_STR_CodeXLAgent L"CodeXLRemoteAgent" GDT_DEBUG_SUFFIX_W   L"." AF_STR_exeFileExtension
    #define GPU_STR_GraphicsCapturePlayer64 L"CXLGraphicsServerPlayer-x64" GDT_DEBUG_SUFFIX_W
#else
    #define GPU_STR_perfStudioServer64 L"CXLGraphicsServer-x64"
    #define GPU_STR_CodeXLAgent L"CodeXLRemoteAgent"
    #define GPU_STR_GraphicsCapturePlayer64 L"CXLGraphicsServerPlayer-x64"
#endif
#define GPU_STR_CodeXLAgentHomeIP L"127.0.0.1"
#define GPU_STR_RaptrExeName L"raptr.exe"
#define GPU_STR_FrapsExeName L"fraps.exe"
#define GPU_STR_RaptrExeNameA "raptr.exe"
#define GPU_STR_FrapsExeNameA "fraps.exe"

// Dashboard
#define GPU_STR_dashboard_FrameAnalysisCaption "Frame Analysis"
#define GPU_STR_dashboard_CapturedFramesCaption "Captured Frames"
#define GPU_STR_dashboard_CaptureTooltip "Click to capture and analyze a frame"
#define GPU_STR_dashboard_StopTooltip "Stop the analyzed application and review any captured frames"
#define GPU_STR_dashboard_OpenTimelineTooltip "Open the selected captured frame timeline for review"
#define GPU_STR_dashboard_MainImageCaptionRunning "Running Frame"
#define GPU_STR_dashboard_MainImageCaptionStopped "Selected Frame"
#define GPU_STR_dashboard_ExecutionCaption "Execution"
#define GPU_STR_dashboard_CaptureButton "&Capture"
#define GPU_STR_dashboard_StopButton "&Stop"
#define GPU_STR_dashboard_OpenTimelineButton "&Open\nTimeline"
#define GPU_STR_dashboard_FrameDetailsLabel "Frame #%1       Elapsed: %2        %3 FPS"
#define GPU_STR_dashboard_FrameNumber "Frame #%1"
#define GPU_STR_dashboard_CapturedAt "Captured: %1"
#define GPU_STR_dashboard_FPS "FPS: %1"
#define GPU_STR_dashboard_Duration "Duration: %1"
#define GPU_STR_dashboard_APICalls "API Calls: %1"
#define GPU_STR_dashboard_DrawCalls "Draw Calls: %1"
#define GPU_STR_dashboard_RunTimeDoubleClickMessage "Frame trace can be opened only after the session stops.\nDo you want to stop the session and open the timeline?"
#define GPU_STR_dashboard_ItemTooltip "Double click to review frame data"
#define GPU_STR_dashboard_serverdisconnectedError "The session was stopped because:\n- The application being analyzed was closed.\n- The analyzed application needs to be raised into focus."
#define GPU_STR_dashboard_failedToLaunchError "Application failed to launch:\n- Please validate application exists at specified location\n- Please validate application can be launched."
#define GPU_STR_dashboard_failedToConnectError "Application failed to launch:\nServer failed to connect to application, no DX12 component or profiled application need to be raised into focus."
#define GPU_STR_dashboard_CapturedFramesCaptionNumFrames " (%1 Frames)"

// Dashboard HTML description
#define GPU_STR_dashboard_HTMLHost L"Host:"
#define GPU_STR_dashboard_HTMLHostLocal L"Local"
#define GPU_STR_dashboard_HTMLTargetPath L"Target Path:"
#define GPU_STR_dashboard_HTMLWorkingDirectory L"Working Directory:"
#define GPU_STR_dashboard_HTMLCommandLineArguments L"Command Line Arguments:"

// Frame view
#define GPU_STR_FrameViewOverview "Overview"
#define GPU_STR_FrameViewTimeline "Timeline"
#define GPU_STR_FrameViewProfile "Profile %1"
#define GPU_STR_FrameViewImage "Image"
#define GPU_STR_FrameViewObject "Objects"

// Overview
#define GPU_STR_overview_FrameAnalysisDescriptionCaption "Frame Analysis - frame #%1, %2"
#define GPU_STR_overviewRenderAgain "&Render Again"
#define GPU_STR_overviewTimelineLabel "<a href=timeline>Timeline</a>"
#define GPU_STR_overviewObjectLabel "<a href=object>Object</a>"
#define GPU_STR_overviewProfileLabel "<a href=profile>Profile %1</a>"
#define GPU_STR_overview_PerfCounterProfileCaption "Performance Counters Profile"
#define GPU_STR_overview_PerfCounterCheckBox "Performance Counters Profile - %1 passes %2"
#define GPU_STR_overview_PerfCounterCheckBoxTime "(up to 1 minute)"
#define GPU_STR_overview_PresetLabel "Use preset selection: "
#define GPU_STR_overview_CountersSelectedLabel "%1 counters selected (%2 available counters), %3 passes required"
#define GPU_STR_overview_AddRemoveButton "Add/Remove Counters..."
#define GPU_STR_overview_FrameStatCaption "Frame Statistics"

/// Overview frame statistics HTML
#define GPU_STR_overview_HTMLCPUTime L"CPU Time"
#define GPU_STR_overview_HTMLGPUTime L"GPU Time"
#define GPU_STR_overview_HTMLGPUTimeBusy L"% GPU Time busy"
#define GPU_STR_overview_HTMLCPUTimeDrawCalls L"% CPU time in Draw calls"
#define GPU_STR_overview_HTMLAPICallsCount L"# of API calls"
#define GPU_STR_overview_HTMLDrawCallsCount L"# of Draw calls"
#define GPU_STR_overview_FramDetailsLabel L"Frame #%1   Time: %2   %3 FPS"

// Timeline strings
#define GPU_STR_timeline_CPU_ThreadBranchName "Thread %1"
#define GPU_STR_timeline_OtherEnqueueBranchName "Other Enqueue Operations"
#define GPU_STR_timeline_UnknownQueueBranchName "<UnknownQueue>"
#define GPU_STR_timeline_QueueBranchNameWithParam "Queue (%1) - %2"
#define GPU_STR_timeline_QueueBranchNameWithParam "Queue (%1) - %2"
#define GPU_STR_timeline_ContextBranchName "Context %1 (%2)"
#define GPU_STR_timeline_QueueBranchName "Queue %1"
#define GPU_STR_timeline_CmdListBranchName "CmdList%1"
#define GPU_STR_timeline_CmdBufferBranchName "CmdBuffer%1"
#define GPU_STR_timeline_CmdListIsntanceBranchName "CmdList%1_%2"
#define GPU_STR_timeline_CmdBufferInstanceBranchName "CmdBuffer%1_%2"
#define GPU_STR_timeline_QueueAPICallsBranchName "API Calls"
#define GPU_STR_timeline_CmdListsBranchName "Command Lists"
#define GPU_STR_timeline_CmdBuffersBranchName "Command Buffers"
#define GPU_STR_timeline_ContextBranchNameWithParam "Queue %1 - %2 (%3)"

#define GPU_STR_DXAPITimeline_tooltipLine1 "Call #%1<br>%2"
#define GPU_STR_APITimeline_TimeTooltipLine "Time: %3 - %4 (%5ms)"

// Frame info xml
#define GPU_STR_frameInfoXMLLocation "Location"
#define GPU_STR_frameInfoXMLFrameNumber "FrameNumber"
#define GPU_STR_frameInfoXMLServerXMLFullPath "ServerXMLFullPath"
#define GPU_STR_frameInfoXMLContents "Contents"
#define GPU_STR_frameInfoXMLLinkedTrace "LinkedTrace"
#define GPU_STR_frameInfoXMLFrameBufferImage "FrameBufferImage"
#define GPU_STR_frameInfoXMLFrameElapsedTime "ElapsedTime"
#define GPU_STR_frameInfoXMLFrameFPS "FPS"
#define GPU_STR_frameInfoXMLFrameCPUFrameDuration "CPUFrameDuration"
#define GPU_STR_frameInfoXMLFrameAPICallCount "APICallCount"
#define GPU_STR_frameInfoXMLFrameDrawCallCount "DrawCallCount"
#define GPU_STR_frameInfoXMLObjectTree "ObjectTree"
#define GPU_STR_frameInfoXMLObjectDBase "ObjectDBase"

// Session frames XML
#define GPU_STR_sessionInfoXMLSessions "Sessions"
#define GPU_STR_sessionInfoXMLSession "Session"
#define GPU_STR_sessionInfoXMLFrame "Frame"
#define GPU_STR_sessionInfoXMLIndex "index"
#define GPU_STR_sessionInfoXMLLoadProgressMessage L"Updating the project sessions from the graphics server"

#define GPU_STR_frameInfoXMLFileFormat "<Root>\n<Location>%1</Location>\n"\
    "<FrameNumber>%2</FrameNumber><Contents>\n"\
    "<LinkedTrace>%3</LinkedTrace>\n"\
    "<ServerXMLFullPath>%4</ServerXMLFullPath>\n"\
    "<FrameBufferImage></FrameBufferImage>\n"\
    "<ElapsedTime>%5</ElapsedTime>\n"\
    "<FPS>%6</FPS>\n"\
    "<CPUFrameDuration>%7</CPUFrameDuration>\n"\
    "<APICallCount>%8</APICallCount>\n"\
    "<DrawCallCount>%9</DrawCallCount>\n"\
    "</Contents></Root>"

// Perf Studio xml file
#define GPU_STR_perfXMLCounterSetNode "counterset"
#define GPU_STR_perfXMLCounterNode "counter"
#define GPU_STR_perfXMLNameNode "name"
#define GPU_STR_perfXMLDescriptionNode "description"
#define GPU_STR_perfXMLDatatypeNode "datatype"
#define GPU_STR_perfXMLUsageNode "usage"

// Performance counters xml file
#define GPU_STR_perfCountersXMLFrameNode "frame"
#define GPU_STR_perfCountersXMLDrawCallNode "DrawCall"
#define GPU_STR_perfCountersXMLDrawCallNodeSmall "drawcall"
#define GPU_STR_perfCountersXMLIndexNode "Index"
#define GPU_STR_perfCountersXMLIndexNodeSmall "index"
#define GPU_STR_perfCountersXMLCallNodeSmall "call"
#define GPU_STR_perfCountersXMLGPUTimeNode "GPUTime"
#define GPU_STR_perfCountersXMLPSBusyNode "PSBusy"

// Counters data tree strings
#define GPU_STR_perfCountersTreeStateBucket "State bucket"
#define GPU_STR_perfCountersTreeDrawCallIndex "Draw call #"

#define GPU_STR_presetFileName L"FrameAnalysisDefaultCounterSets"
#define GPU_STR_presetFileExt L"cfg"

// Counter selection dialog
#define GPU_STR_counterSelectionDialogTitle                           "Counters Selection"
#define GPU_STR_counterSelectionDialogDescription                     "Add a performance counter to the 'Selected Counters' list to view it's usage"
#define GPU_STR_counterSelectionDialogAvailableCountersTitle          "Available Counters"
#define GPU_STR_counterSelectionDialogActiveCountersTitle             "Selected Counters"
#define GPU_STR_counterSelectionDialogGroupBoxTitle                   "Performance Counters"
#define GPU_STR_counterSelectionDialogMultipleCountersSelectedHeading "Multiple Counters Selected"
#define GPU_STR_counterSelectionDialogMultipleCountersSelectedMsg     "Select a single item to see its description"
#define GPU_STR_counterSelectionDialogCounterDescriptionCaption        "<font color=blue>%1</font><br>%2"
#define GPU_STR_counterSelectionDialogPresetLabel                      "New preset name"
#define GPU_STR_counterSelectionDialogMustEnterNameError               "Please enter preset name"
#define GPU_STR_counterSelectionDialogMustEnterNewNameError            "Preset name already exists, please select a new one."

// Debug log messages
#define GPU_STR_Attempting_To_Connect_To_Frame_Analysis_Server L"Attempting to connect to frame analysis server %ls:%d"
// connection dialog
#define GPU_STR_connectionDialogTitle "Frame Analysis API Selection"
#define GPU_STR_connectionDialogHeader "Please select an API to connect to:"
#define GPU_STR_connectionDialogTableHeader "#,Process,PID,API,Status,Time loaded"
#define GPU_STR_connectionDialogMonitoring "Monitoring..."
#define GPU_STR_connectionDialogAutoCheckBox "Automatically connect next session to"
#define GPU_STR_connectionDialogComboOptionsDefault  GPU_STR_projectSettingsComboProcessNumberOption "," GPU_STR_projectSettingsComboAPIInProcessOption "," GPU_STR_projectSettingsComboAPIOption
#define GPU_STR_connectionDialogComboOptionsSelected "Process %1,First active API in %1,First active %1"
#define GPU_STR_connectionDialogOKCaption L"Connect"
#define GPU_STR_connectionDialogStarted "Started"
#define GPU_STR_connectionDialogAttached "Attached"

#define GPU_STR_connectionDialogProcessNode "Process"
#define GPU_STR_connectionDialogPIDNode "PID"
#define GPU_STR_connectionDialogNameNode "Name"
#define GPU_STR_connectionDialogAPINode "API"
#define GPU_STR_connectionDialogAttachedAttribute "attached"
#define GPU_STR_connectionDialogWrongAPI "Cannot connect to Process #%1 (%2) since it does not use the DX12 API.\nChange automatic connection rules."

// Navigation chart
#define GPU_STR_navigationByCaption "Navigate by:"
#define GPU_STR_navigationGroupCount "Count"
#define GPU_STR_navigationGroupDuration "Duration"
#define GPU_STR_navigationGroupThreads "Concurrency"

#define GPU_STR_navigationLayerApiCalls "API Calls Count"
#define GPU_STR_navigationLayerDrawCalls "Draw Calls Count"
#define GPU_STR_navigationLayerCPUApi "All CPU API Calls"
#define GPU_STR_navigationLayerGPUCmds "All GPU Cmds"
#define GPU_STR_navigationLayerTopCPUDrawCalls "Top 20 CPU API Calls"
#define GPU_STR_navigationLayerTopGPUOps "Top 20 GPU Cmds"
#define GPU_STR_navigationLayerMaxSimultaneaus "Max # of busy threads"
#define GPU_STR_navigationLayerAvgSimultaneaus "Avg. # of busy threads"
#define GPU_STR_navigationLayerTotalSimultaneaus "Total # of threads"
#define GPU_STR_navigationLayerDisplayFiltersLink "<a href=display>Display Filters...</a>"

// ribbon names
#define GPU_STR_ribbonNameDrawCalls "CPU Draw Calls"
#define GPU_STR_ribbonNameTimeLine "Timeline"
#define GPU_STR_ribbonNameAPICalls "Call History"
#define GPU_STR_ribbonNameSummary "Hotspot Summary"

// Dashboard Summary
#define GPU_STR_API_Summary "CPU API Summary"
#define GPU_STR_GPU_Summary "GPU Commands Summary"
#define GPU_STR_Command_Lists_Summary "Command Lists Summary"
#define GPU_STR_Command_Buffers_Summary "Command Buffers Summary"
#define GPU_STR_API_Call_Summary "Call History Aggregated Data"
#define GPU_STR_GPU_Call_Summary "GPU Cmds Aggregated Data"
#define GPU_STR_Command_List_Call_Summary "%1 Command Lists"
#define GPU_STR_Command_Buffers_Call_Summary "%1 Command Buffers"
#define GPU_STR_Use_Scope_Summary "Use timeline selection scope"
#define GPU_STR_Top_20_Cpu_Calls_Summary "Top %1 %2 calls (CPU)"
#define GPU_STR_Top_Cpu_Calls_Summary "Top %1 calls (CPU)"
#define GPU_STR_Top_20_Gpu_Calls_Summary "Top %1 %2 calls (GPU)"
#define GPU_STR_Top_Gpu_Calls_Summary "Top %1 calls (GPU)"

#define GPU_STR_Top_20_CommandLists_Summary "Top %1 %2 commands"
#define GPU_STR_Top_CommandLists_Summary "Top %1 commands"


// Summary table captions
#define GP_STR_SummaryTableColumnInterface "Interface"
#define GP_STR_SummaryTableColumnCall "Call"
#define GP_STR_SummaryTableColumnCumulativeTime "Cumulative Time"
#define GP_STR_SummaryTableColumnPercentageOfTotalTime "% of total Time"
#define GP_STR_SummaryTableColumnNumberOfCalls "# of Calls"
#define GP_STR_SummaryTableColumnAvgTime "Average Time"
#define GP_STR_SummaryTableColumnMaxTime "Max Time"
#define GP_STR_SummaryTableColumnMinTime "Min Time"

#define GP_STR_SummaryTableCommandBufferType "Command Buffer"
#define GP_STR_SummaryTableCommandListType "Command List"
#define GP_STR_SummaryTableCommandListStartTime "Start time"
#define GP_STR_SummaryTableCommandListEndTime "End time"
#define GP_STR_SummaryTableCommandListExecutionTime "Execution time"
#define GP_STR_SummaryTableCommandListNumCommands "# of commands"
#define GP_STR_SummaryTableCommandListGPUQueue "GPU queue"
#define GP_STR_SummaryTableCommandListAddress "Address"
#define GP_STR_SummaryTableCommandListHandle "Handle"

#define GP_STR_SummaryTop20TableColumnCallIndex "index"
#define GP_STR_SummaryTop20TableColumnThreadId "Thread Id"
#define GP_STR_SummaryTop20TableColumnTime "Time"
#define GP_STR_SummaryTop20TableShowAll "Show all..."

// draw calls ribbon
#define GP_STR_DrawCallToolTip "%1: %2"
#define GPU_STR_navigationLayerTooltips "API Calls Count,Draw Calls Count,Top GPU Op,Top CPU Api Call,GPU Commands,CPU API Calls,Max busy threads,Avg. busy threads,Total threads"

// Export frame analysis
#define GP_STR_FrameAnalysisExportProgressHeader L"Exporting Frame Analysis Session"
#define GP_STR_FrameAnalysisExportProgressMsg L"Capturing frames..."
#define GP_STR_FrameAnalysisExportCapturingFrameMsg L"Capturing frame %d"

#endif //__GPSTRINGCONSTANTS_H

