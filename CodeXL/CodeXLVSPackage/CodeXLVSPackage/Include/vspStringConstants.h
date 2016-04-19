//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspStringConstants.h
///
//==================================================================================

//------------------------------ vspStringConstants.h ------------------------------

#ifndef __VSPSTRINGCONSTANTS_H
#define __VSPSTRINGCONSTANTS_H

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// General strings:
#define VS_STR_NewLine L"\n"
#define VS_STR_NewLineA "\n"
#define VS_STR_VS L"VS"
#define VS_STR_CodeXL L"CodeXL"

#define VSP_STR_StartPrefix L"Start CodeXL "
#define VSP_STR_StartGeneric L"Start CodeXL"
#define VSP_STR_Continue L"Continue"
#define VSP_STR_StopDebugging L"Stop Debugging"
#define VSP_STR_StopProfiling L"Stop Profiling"
#define VSP_STR_StopFrameAnalysis L"Stop Frame Analysis"
#define VSP_STR_BreakDebugging L"Break Debugging"
#define VSP_STR_BreakProfiling L"Break Profiling"
#define VSP_STR_PauseProfiling L"Pause Data Collection"
#define VSP_STR_AttachProfiling L"Attach to Process..."
#define VSP_STR_ModuleLoadedDebugMessage L"Loaded \'"
#define VSP_STR_ModuleUnloadedDebugMessage L"Unloaded \'"
#define VSP_STR_BuildPrefix L"Build "

#define VSP_STR_VisualStudioRegistryRootPathPrefix L"Software\\Microsoft\\VisualStudio\\"
#ifdef VSP_VS11BUILD
    #define VSP_STR_VisualStudioRegistryRootVersion L"11.0"
#elif defined VSP_VS12BUILD
    #define VSP_STR_VisualStudioRegistryRootVersion L"12.0"
#elif defined VSP_VS14BUILD
    #define VSP_STR_VisualStudioRegistryRootVersion L"14.0"
#else
    #define VSP_STR_VisualStudioRegistryRootVersion L"10.0"
#endif
#define VSP_STR_VisualStudioRegistryRootConfigSuffix L"_Config"
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define VSP_STR_VisualStudioDefaultRegistryRootPath VSP_STR_VisualStudioRegistryRootPathPrefix VSP_STR_VisualStudioRegistryRootVersion L"Exp"
    #define VSP_STR_VisualStudioDefaultRegistryRootPath2 VSP_STR_VisualStudioRegistryRootPathPrefix VSP_STR_VisualStudioRegistryRootVersion L"Exp" VSP_STR_VisualStudioRegistryRootConfigSuffix
#elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
    #define VSP_STR_VisualStudioDefaultRegistryRootPath VSP_STR_VisualStudioRegistryRootPathPrefix VSP_STR_VisualStudioRegistryRootVersion
    #define VSP_STR_VisualStudioDefaultRegistryRootPath2 VSP_STR_VisualStudioRegistryRootPathPrefix VSP_STR_VisualStudioRegistryRootVersion VSP_STR_VisualStudioRegistryRootConfigSuffix
#else
    #error Unknown configuration
#endif

#define VSP_STR_DebugEngineName L"AMD Inc. OpenCL Debugger"
#define VSP_STR_CallStackC_CppLanguage L"C / C++"
#define VSP_STR_CallStackSourceLineFormat L" Line %d"
#define VSP_STR_CallStackFunctionOffsetAsHexString L" + %#llx bytes"
#define VSP_STR_CallStackFunctionOffsetAsDecString L" + %llu bytes"
#define VSP_STR_ModuleDebugSymbolsLoaded L"Debug symbols loaded"
#define VSP_STR_ModuleDebugSymbolsNotLoaded L"Debug symbols not loaded"
#define VSP_STR_ThreadNormalThreadName L"Thread %d"
#define VSP_STR_ThreadKernelDebuggingWavefrontThreadName L"Wavefront %d"
#define VSP_STR_ThreadMainThreadName L"Main Thread"
#define VSP_STR_ThreadLocationUnknown L"Unknown location"
#define VSP_STR_ForceVariableRefreshPseudoVariable L"Refresh kernel variables"
#define VSP_STR_BreakpointErrorCannotBeBound L"Breakpoint could not be bound."
#define VSP_STR_BreakpointErrorKernelBreakpointsDisabled L"Kernel source breakpoints are disabled.\nTo enable them, stop debugging, and change the setting under CodeXL > Options > Advanced."
#define VSP_STR_BreakpointErrorConditionIsNotSupported L"Conditional breakpoints are not currently supported by CodeXL."
#define VSP_STR_BreakpointErrorC_CPPCodeIsNotSupported L"C / C++ source code breakpoints are not currently supported by CodeXL."
#define VSP_STR_BreakpointErrorC_CPPFunctionsAreNotSupported L"C / C++ function breakpoints are not currently supported by CodeXL."

// Debug and error messages:
#define VSP_STR_UnsupportedGDWindowCommandID L"Unsupported GD window command id"
#define VSP_STR_NoDebugEnginesSelected L"At least one debug engine must be enabled in order to debug."
#define VSP_STR_UnknownModuleUnloaded L"Unknown module unloaded: "
#define VSP_STR_FailedToLaunchDebuggedProcess "Failed to launch debugged process"
#define VSP_STR_NoExecutableSelectedForDebugging "No executable selected for debugging."
#define VSP_STR_DebuggedExecutableDoesNotExist "Debugged executable does not exist:\n"
#define VSP_STR_WorkingDirectoryDoesNotExist "Working directory does not exist:\n"
#define VSP_STR_PleaseCheckValuesInProjectSettings "\n\nPlease verify the settings in the startup project's \"Debugging\" properties tab."
#define VSP_STR_NonSupportedProjectType "Non-supported project type or no executable specified.\nWhen debugging .NET projects, select the \"External Program\" option for debugging.\nUse C / C++ projects if this problem persists."
#define VSP_STR_OpenCLNotFound "OpenCL driver not found.\nInstall an OpenCL driver or disable the OpenCL debug engine to continue."
#define VSP_STR_OpenCLInSameDirectoryPrefix "The debugged executable's directory "
#define VSP_STR_OpenCLInSameDirectorySuffix " contains an OpenCL.dll file. Please remove or rename this file before starting to debug your application."
#define VSP_STR_OpenGLInSameDirectoryPrefix "The debugged executable's directory "
#define VSP_STR_OpenGLInSameDirectorySuffix " contains an opengl32.dll file. Please remove or rename this file before starting to debug your application."
#define VSP_STR_OpenGLNotFound "OpenGL driver not found.\nInstall an OpenGL driver or disable the OpenGL debug engine to continue."
#define VSP_STR_EditAndContinueNotSupported "Edit and Continue is not supported. The source code has changed and no longer matches the file version used to build the application."

#define VSP_STR_FileOperationPointerNotInitialized L"File operation do not initialized"

// Files and folders:
#define VSP_STR_VSCacheFolderName "VS_Cache"

// Dialogs strings
#define VSP_STR_BuildListDialogCaption "Microsoft Visual Studio"
#define VSP_STR_BuildListDialogUpperText "These projects are out of date:"
#define VSP_STR_BuildListDialogLowerText "Would you like to build them?"

#define VSP_STR_BuildFailedCaption L"Microsoft Visual Studio"
#define VSP_STR_BuildFailedBody L"There were build errors. Whould you like to continue and run the last successful build?"

#define VSP_STR_SaveListDialogCaption "Microsoft Visual Studio"
#define VSP_STR_SaveListDialogUpperText "Save changes to the following items?"

// Output pane:
#define VS_STR_CodeXLPrefix L"CodeXL - "
#define VS_STR_CodeXLPrefixIndentation L"         "

// HTML properties:
#define VSP_STR_keyboardShortcutRunString L"(Shift + Alt + F5)"
#define VSP_STR_PropertiesViewStartDebuggingPackageComment L"To %ls your application with CodeXL, press the \"%ls\" command from the CodeXL menu "

// Teapot sample:
#define VSP_STR_SampleLoadErrorMessage "%1 sample project cannot be loaded"
#define VSP_STR_SampleNotFoundLogError L"The sample solution could not be found: %ls"
#define VSP_STR_D3DMT_VS13_IDEWarning "D3DMultiThreading sample can only be opened in Visual Studio 2015 or higher."
#define VSP_STR_TeapotSampleName L"AMDTTeaPot.vcxproj"
#define VSP_STR_MatMulSampleName L"AMDTClassicMatMul.vcxproj"

// gDEBugger installation
#define VSP_STR_gDEBuggerName L"gDEBugger"
#define VSP_STR_gDEBuggerPackage L"gDEBuggerVSPackage.dll"
#define VSP_STR_gDEBuggerInstallerTitle "gDEBugger Installed"
#define VSP_STR_gDEBuggerInstallerError "gDEBugger extension for Visual Studio detected. Having both CodeXL and gDEBugger installed in Visual Studio may hinder mouse activation of CodeXL dialog items.\nAs a possible workaround, please use keyboard navigation."


#endif //__VSPSTRINGCONSTANTS_H

