//------------------------------ kaStringConstants.h ------------------------------

#ifndef __KASTRINGCONSTANTS_H
#define __KASTRINGCONSTANTS_H

// Need to include AMDTDefinitions.h so AMDT_BUILD_TARGET macros will be recognized correctly.
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#define KA_STR_ERROR_TITLE "Error"
#define KA_STR_openclBuild L"&Build"
#define KA_STR_CancelBuild L"&Cancel Build"
#define KA_STR_openFolder L"Open &Containing Folder"
#define KA_STR_deviceOptions L"Analyze &Options..."
#define KA_STR_analyzeSettingsMenu L"Analyze &Settings..."
#define KA_STR_gotoSourceCode L"&Goto Source Code"
#define KA_STR_removeFromProject L"Remove from Project"
#define KA_STR_openFolderStatusbarString L"Open containing folder"
#define KA_STR_analyzeSettingsMenuStatusbarString L"Open Analyze settings dialog"
#define KA_STR_emptyDeviceListError "No target device selected. Please select target devices using the Analyze tab of  the CodeXL Options dialog."
#define KA_STR_buildOptionRemovedNoParenthesis "'-o' build option ignored. Parenthesis are not supported.\n"
#define KA_STR_buildOptionRemovedNoWritableDir "'-o' build option ignored. No valid writable directory was provided.\n"
#define KA_STR_OpenCL2NotSupported "Building OpenCL 2.0 kernels is not supported.\nBuild is cancelled."
#define KA_STR_PropertiesExecutionInformationSA L"To build and analyze your file, select 'Build' from the Analyze menu,<br> or click the 'Build' toolbar button."
#define KA_STR_PropertiesExecutionInformationVS L"To build and analyze your file, select 'Build' from the CodeXL menu,<br> or click the 'Build' toolbar button."
#define KA_STR_DX_FXC_BUILD_TYPE "FXC"
#define KA_STR_DX_FXC_INVOKING "Invoking FXC with the following command:\n>"
#define KA_STR_DX_FXC_FAILED_TO_EXECUTE_WITH_OUTPUT "FXC failed to execute with the following output:\n"
#define KA_STR_DX_FXC_FAILED_TO_EXECUTE_NO_OUTPUT "FXC failed to execute. Please make sure that FXC is installed on your machine, and that its directory is listed in the PATH environment variable.\n"
#define KA_STR_DX_FXC_FAILED_TO_EXECUTE_CUSTOM_PATH_NO_OUTPUT "FXC failed to execute.\n"
#define KA_STR_DX_FXC_EXECUTED_SUCCSESSFULLY "FXC executed with the following output:\n"
#define KA_STR_DX_UNABLE_TO_LOAD_D3D_COMPILER_MODULE "Unable to load the D3D compiler module: "
#define KA_STR_DX_INCOMPATIBLE_D3D_COMPILER_MODULE_A "The D3D compiler module: "
#define KA_STR_DX_INCOMPATIBLE_D3D_COMPILER_MODULE_B " is not supported. Use D3DCompiler_43.dll and above."
#define KA_STR_DX_SHADER_BUILD_ABORTED " Shader build is aborted."
#define KA_STR_DX_USE_PROJECT_SETTINGS "\nUse 'HLSL Build Options' node in CodeXL Project Settings to set the D3D Compiler path."
#define KA_STR_DX_UNABLE_TO_INIT "Unable to initialize the static analyzer."
#define KA_STR_DirectXNoFxcBuilderSelectedError "Invalid FXC.exe path. Use Project Settings to specify the location of FXC.exe"
#define KA_STR_DirectXNoD3dBuilderSelectedError "Invalid d3dcompiler*.dll path. Use Project Settings to specify the location of d3dcompiler*.dll"
#define KA_STR_DX_EmptyEntryPointError "Please select an entry point function from toolbar drop-list."
#define KA_STR_DX_EmptyEntryPointErrorTitle "Error - no entry point function selected"
#define KA_STR_DX_OutputMsg_IgnoringStrs "Ignoring invalid build options: "
#define KA_STR_GL_UNSUPPORTED_SHADERS_ERROR "The following shaders are currently supported for OpenGL: fragment, vertex, compute, geometry, tessellation evaluation."
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #define KA_STR_GL_BUILD_WARNING_OLDDRIVER L"Offline build of GLSL shaders is fully supported as of Catalyst driver version 15.20.\nInstalled driver release: %ls. It is recommended to update to the latest driver version."
#else // Building for Linux
    #define KA_STR_GL_BUILD_WARNING_OLDDRIVER L"Offline build of GLSL shaders is fully supported as of Catalyst driver version 15.30.\nInstalled driver release: %ls. It is recommended to update to the latest driver version."
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)


// Family names:
#define KA_STR_familyNameSICards " (HD7000 / HD8000 series)"
#define KA_STR_familyNameCICards " (HD8000 / Rx 200 / 300 series)"
#define KA_STR_familyNameVICards " (Rx 200 / 300 / Fury series)"

// Analyze mode:
#define KA_STR_executionMode L"Analyze Mode"
#define KA_STR_executionModeMenu L"&Analyze Mode"
#define KS_STR_SwitchToAnalyzeMode L"Switch to &Analyze Mode"
#define KA_STR_executionModeAction L"Analyzing"
#define KA_STR_executionModeVerb   L"analyze"
#define KA_STR_executionSesionType L"Analyze"
#define KA_STR_executionModeStatusbarString L"Analyze Mode - Switch to Analyze Mode"

// Context menu commands
#define KA_STR_openclBuildASCII "&Build"
#define KA_STR_CancelBuildASCII "&Cancel Build"
#define KA_STR_openclOpenFile "&Open"
#define KA_STR_SelectedFiles "selected files"


#define KA_STR_openclBuildStatusbarString L"Build the source file"
#define KA_STR_CancelBuildStatusbarString L"Cancel the build"
#define KA_STR_addFileStatusbarString L"Add a source file to CodeXL Explorer"
#define KA_STR_newFileStatusbarString L"Create a new source file for analysis, and add it to CodeXL Explorer"
#define KA_STR_addFile L"A&dd existing source files to project..."
#define KA_STR_newFile L"&Create new source file"
#define KA_STR_newProgramASCII "Create new &program/folder"


// Create program/folder dialog
#define KA_STR_CreateNewProgramDlgTitle "Create a New Program/Folder"
#define KA_STR_SelectProgramType "<b>Select Program/Folder type:"
#define KA_STR_ProgramTypeVulkan "<b>Vulkan"
#define KA_STR_ProgramTypeOpenGL "<b>OpenGL"
#define KA_STR_ProgramTypeRendering "Rendering Program"
#define KA_STR_ProgramTypeCompute "Compute Program"
#define KA_STR_ProgramTypeDirectX "<b>DirectX"
#define KA_STR_ProgramTypeDirectXFolder "DirectX Folder"
#define KA_STR_ProgramTypeOpenCL "<b>OpenCL"
#define KA_STR_ProgramTypeOpenCL1_2 "OpenCL Folder"
#define KA_STR_DescriptionTitle "<b>Description:</b>"
#define KA_STR_VulkanRenderingProgramDescription "A Vulkan Rendering Program is a graphics pipeline that can have a single Vulkan glsl shader attached to each of its stages: Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment.\nWhen built, all of the program's shaders are being compiled and linked together."
#define KA_STR_VulkanComputeProgramDescription "A Vulkan Compute Program is a single-stage pipeline that can have a single Vulkan glsl compute shader attached to it."
#define KA_STR_OpenGLComputeProgramDescription "An OpenGL Compute Program is a a single-stage pipeline that can have a single glsl compute shader attached to it."
#define KA_STR_OpenGLRenderingProgramDescription "An OpenGL Rendering Program is a graphics pipeline that can have a single glsl shader attached to each of its stages: Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment.\nWhen built, all of the program's shaders are being compiled and linked together."
#define KA_STR_OpenCLFolderDescription "An OpenCL Folder is a logical container that can have one or more OpenCL source files attached to it.\nWhen built, the source files in the folder are being compiled independently, one after the other. Unlike the case with Rendering Programs, there is no interdependcy between a folder's source files during the build process."
#define KA_STR_DirectXFolderDescription "A DirectX Folder is a logical container that can have one or more hlsl source files attached to it.\nWhen built, the source files in the folder are being compiled independently, one after the other. Unlike the case with Rendering Programs, there is no interdependcy between a folder's source files during the build process."



#define KA_STR_BuildProgram L"&Build"
#define KA_STR_BuildProgramASCII "&Build"

#define KA_STR_RebuildProgram L"&Rebuild"
#define KA_STR_RebuildProgramASCII "&Rebuild"

#define KA_STR_CleanProgram L"&Clean"
#define KA_STR_CleanProgramASCII "&Clean"

#define KA_STR_addFileASCII "A&dd existing source file"
#define KA_STR_newFileASCII "&Create a new source file"
#define KA_STR_executionModeDescription L"Perform static analysis of kernels and shaders<br>to estimate and improve performance."

#define KA_STR_openContainingFolderASCII "Open &Containing Folder"
#define KA_STR_openOutputFolderASCII "Open &Output Folder"
#define KA_STR_gotoSourceCodeASCII "&Goto Source Code"
#define KA_STR_removeFromProjectASCII "&Remove from Project"
#define KA_STR_removeFromProgramASCII "&Remove from Program"
#define KA_STR_removeProgramFromProjectASCII "Remove %1 &from Project"
#define KA_STR_removeShaderFromProgramASCII "Remove &shader from program"
#define KA_STR_removeShadersFromProgramASCII "Remove &from %1"

#define KA_STR_addShaderFromProgramASCII "Add &file for this shader"
#define KA_STR_attachStageShaderToASCII "Attach a %1 shader"
#define KA_STR_createShaderFromProgramASCII "Create new shader"
#define KA_STR_exportBinariesASCII "&Export Binaries..."
#define KA_STR_renameFileStatusbarStringASCII "R&ename file"
#define KA_STR_renameProgramStatusbarStringASCII "R&ename program"

//Export Binaries dialog strings

#define KA_STR_ExportBinariesDialogIncludeLable                      "Sections Included:"
#define KA_STR_ExportBinariesDialogBittnessLabel                     "Bitness:"
#define KA_STR_ExportBinariesDialogBaseFaleNameLabel                 "Base file &name:"
#define KA_STR_ExportBinariesDialogFileNameLabel                     "File name:"
#define KA_STR_ExportBinariesDialogTitle                             "Export Binaries"
#define KA_STR_ExportBinariesDialogAcceptButtonText                  "&Export"
#define KA_STR_ExportBinariesDialogDebugInfoCheckboxText             "&Debug info"
#define KA_STR_ExportBinariesDialogILCheckboxText                    "&IL"
#define KA_STR_ExportBinariesDialogISACheckboxText                   "IS&A"
#define KA_STR_ExportBinariesDialogBittness32CheckboxText            "&32-bit"
#define KA_STR_ExportBinariesDialogBittness64CheckboxText            "&64-bit"
#define KA_STR_ExportBinariesDialogLLVMIRCheckboxText                "&LLVM IR"
#define KA_STR_ExportBinariesDialogSourceCheckboxText                "&Source"
#define KA_STR_ExportBinariesDialogDebugInfoCheckboxTooltip          "Include the associated debug info"
#define KA_STR_ExportBinariesDialogILCheckboxTooltip                 "Include the generated AMD Intermediate Language text"
#define KA_STR_ExportBinariesDialogISACheckboxTooltip                "Include the ISA in binary form"
#define KA_STR_ExportBinariesDialogBittness32CheckboxTooltip         "Export 32 bit biniries"
#define KA_STR_ExportBinariesDialogBittness64CheckboxTooltip         "Export 64 bit biniries"
#define KA_STR_ExportBinariesDialogLLVMIRCheckboxTooltip             "Include the compiler's Internal Representation in binary form"
#define KA_STR_ExportBinariesDialogSourceCheckboxTooltip             "Include the OpenCL source code"
#define KA_STR_ExportBinariesDialogExportToLabelText                 "Export to:"
#define KA_STR_ExportBinariesDialogBaseNameDevicesPart               "-{DEVICE_NAME}-{BITNESS}.bin"
#define KA_STR_ExportBinariesDialogBaseNameProgramDevicesPart        "-{FILE_NAME}" KA_STR_ExportBinariesDialogBaseNameDevicesPart
#define KA_STR_ExportBinariesDialogBaseNameDevicesSatgePart          "-{DEVICE_NAME}-{STAGE}-{BITNESS}.bin"
#define KA_STR_ExportBinariesDialogBaseNameProgramDevicesStagePart   "-{FILE_NAME}" KA_STR_ExportBinariesDialogBaseNameDevicesSatgePart

#define KA_STR_ExportBinariesOutPutFile32BitnessSuffix               L"-32"
#define KA_STR_ExportBinariesOutPutFile64BitnessSuffix               L"-64"
#define KA_STR_NA                                                    "N/A"

#define KA_STR_ExportBinariesDialogHyphenPart                         "-"
#define KA_STR_ExportBinariesDialogBinPart                            ".bin"
#define KA_STR_ExportBinariesDialogEmptyBaseNameWarning               "Please enter a base name for exported binaries."
#define KA_STR_QFileDialogLookInCombo                                 "lookInCombo"
#define KA_STR_QFileDialogLookInLabel                                 "lookInLabel"
#define KA_STR_QFileDialogToolButtonBack                              "backButton"
#define KA_STR_QFileDialogToolButtonForward                           "forwardButton"
#define KA_STR_QFileDialogToolButtonDetailMode                        "detailModeButton"
#define KA_STR_QFileDialogToolButtonParent                            "toParentButton"
#define KA_STR_QFileDialogToolButtonNewFolder                         "newFolderButton"
#define KA_STR_QFileDialogToolButtonListMode                          "listModeButton"
#define KA_STR_QFileDialogComboBoxFileType                            "fileTypeCombo"
#define KA_STR_QFileDialogLabelFileName                               "fileNameLabel"
#define KA_STR_QFileDialogLabelFileType                               "fileTypeLabel"
#define KA_STR_QFileDialogLineEditFileName                            "fileNameEdit"
#define KA_STR_QFileDialogLookInComboLineEdit                         "lookInComboLineEdit"


// tables headers names:
#define KA_STR_tableKernelNameColumn "Kernel Name"
#define KA_STR_tableGlobalXColumn "Global X"
#define KA_STR_tableGlobalYColumn "Global Y"
#define KA_STR_tableGlobalZColumn "Global Z"
#define KA_STR_tableLocalXColumn "Local X"
#define KA_STR_tableLocalYColumn "Local Y"
#define KA_STR_tableLocalZColumn "Local Z"
#define KA_STR_tableLoopsColumn "Loop Iterations"

// analysis settings page:
#define KA_STR_analyzeSettingsPageTitle L"&Analyze"
#define KA_STR_analsisSettingGeneralInfo "Default kernel analysis work dimensions"
#define KA_STR_analsisSettingKernelColumn "Kernel"
#define KA_STR_analsisSettingDefaultGlobal "64"
#define KA_STR_analsisSettingDefaultLocal "4"
#define KA_STR_analsisSettingDefaultLoop "100"
#define KA_STR_analsisSettingInvalidDataMsg "Invalid Analyze data:\n1: Global work items must be multiplication of local work items.\n2: Not all Global or local work item can be zero.\n3: Local work size X * Y * Z must be <= 256.\n4: Global work size X * Y * Z must be <= 16777216"
#define KA_STR_analsisSettingInvalidLdsDataMsg "Invalid Analyze data:\n: Dynamic LDS usage must be a positive number of bytes."
#define KA_STR_targetDeviceSettingsPageGeneralInfo "Select devices to include them as build targets<br> The displayed devices are devices supported by the installed driver"
#define KA_STR_driverOutOfData "Note: For CodeXL to be able to target the most recent hardware, please make sure that the latest driver is installed."
#define KA_STR_applyUpdateLoops "Apply changes to all kernels in project"
#define KA_STR_mustSelectDeviceError "At least one device must be selected"

// Global settings xml file
#define KA_STR_analysisSettingsXMLSectionPageTitle L"Analyze"
#define KA_STR_analsisSettingDefaultExecutionValuesNode L"DefaultExecutionValues"
#define KA_STR_targetDeviceSettingsXMLSectionPageTitle L"KATargetDevice"
#define KA_STR_targetDeviceSettingTargetDeviceNode L"TargetDevice"


// Tree Strings
#define KA_STR_treeFileNode L"Files"
#define KA_STR_treeOverviewNode L"Overview"
#define KA_STR_treeSourceCodeNode L"Source Code"

#define KA_STR_treeAddFileNode L"Add existing source file..."
#define KA_STR_treeAddFileNodeTooltip "Add an existing kernel/shader source file..."
#define KA_STR_treeAddFileNodeSourcesTooltip "<font>Add an existing source file to the Source Files pool of the project. Note that files <u>cannot</u> be built directly from the Source Files pool. In order to build the files, you first need to create a new Program or Folder of the relevant type, and drag the file from the pool to the Program or Folder.</font>"
#define KA_STR_treeNewFileNode L"Create new source file..."
#define KA_STR_treeNewFileNodeTooltip "Create a new kernel/shader source file..."
#define KA_STR_treeNewProgramNode L"Create new program/folder..."
#define KA_STR_treeSourcesNode L"Source Files"
#define KA_STR_treeRemoveFileWarning "This file is referenced in some of the project programs. Removing it will also remove it from the related programs.\n"
#define KA_STR_treeRemoveFileWarningMultiple "One or more of the selected files are referenced in some of the project programs. Removing it will also remove it from the related programs.\n"
#define KA_STR_treeRemoveProgramWarning "This will remove the build outputs of the selected programs from the disk. Do you want to continue?"
#define KA_STR_treeRenameQuestion "Windows related to this file will be closed before renaming. Do you want to continue?"
#define KA_STR_treeRenameProgramQuestion "Renaming a program will remove the selected programs binary output from disk. Are you sure you want to continue?"
#define KA_STR_treeRenameProgramWithOpenedQuestion "Renaming a program will remove the selected programs binary output from disk, and will close all the windows related to this program. Are you sure you want to continue?"


#define KA_STR_treeProjectFileExists "Source file already exists in the project."
#define KA_STR_treeProgramFileExists "Source file already attached to the program."
#define KA_STR_kernelPrefix L"Kernel - "
#define KA_STR_kernelPrefixASCII "Kernel - "
#define KA_STR_ShaderPrefix L"Shader - "
#define KA_STR_ShaderPrefixASCII "Shader - "
#define KA_STR_kernelFileExtension L"cl"
#define KA_STR_selectExportDirectory L"Select Export Directory"
#define KA_STR_ERR_CANNOT_ADD_SOURCE_FILE_DURING_BUILD "Cannot add source files while a build is in progress."
#define KA_STR_ERR_CANNOT_CREATE_SOURCE_FILE_DURING_BUILD "Cannot create source files while a build is in progress."
#define KA_STR_ERR_CANNOT_CREATE_PROGRAM_DURING_BUILD "Cannot create programs while a build is in progress."

#define KA_STR_mdiKernelNamePlaceHolder "Kernel Name"
#define KA_STR_mdiCaptionFormat L"%ls (%ls) - %ls"
#define KA_STR_buildCommandFormat "&Build %1"
#define KA_STR_buildCommandFormatDirectX "&Build %1 (%2)"
#define KA_STR_refTypeNode "%1 (%2)"

#define KA_STR_programSortOrder "vert,tesc,tese,geom,frag"

// Project setting
#define KA_STR_projectSettingExtensionName L"KernelAnalyzer"
#define KA_STR_projectSettingExtensionNameASCII "KernelAnalyzer"

#define KA_STR_projectSettingExtensionDisplayName L"Analyze, Kernel/Shader Build Options, General & Optimization"
#define KA_STR_projectSettingExtensionOtherDisplayName L"Analyze, Kernel/Shader Build Options, Other"

#define KA_STR_projectSettingExtensionNameOther L"KernelAnalyzerOther"
#define KA_STR_projectSettingExtensionNameOtherASCII "KernelAnalyzerOther"
#define KA_STR_projectSettingShaderExtensionDisplayName L"Analyze, HLSL Build Options"
#define KA_STR_projectSettingShaderExtensionName L"ShaderAnalyzer"
#define KA_STR_projectSettingShaderExtensionNameASCII "ShaderAnalyzer"

// Project xml file info
#define KA_STR_projectSettingSectionNode L"FilesSection"
#define KA_STR_projectSettingSectionNodeASCII "FilesSection"
#define KA_STR_projectSettingFilesInfoNode L"FileInfo"
#define KA_STR_projectSettingFilesInfoNodeASCII "FileInfo"
#define KA_STR_projectSettingFilePathNode L"FilePath"
#define KA_STR_projectSettingFileType     L"FileType"
#define KA_STR_projectSettingFileExecutionPathNode L"ExecutionPath"
#define KA_STR_projectSettingDevicesNode L"ExecutionDevices"
#define KA_STR_projectSettingAnalyzeSectionNode L"AnalyzeSection"
#define KA_STR_projectSettingAnalyzeSectionNodeASCII "AnalyzeSection"
#define KA_STR_projectSettingKernelAnalsisNode L"KernelAnalysis"
#define KA_STR_projectSettingBuildOptionsNode L"BuildOptions"
#define KA_STR_projectSettingShaderBuildOptionsNode L"ShaderBuildOptions"
#define KA_STR_projectSettingShaderCompileTypeNode L"ShaderCompileType"
#define KA_STR_projectSettingShaderD3dBuilderPathNode L"ShaderD3dBuilderPath"
#define KA_STR_projectSettingShaderFxcBuilderPathNode L"ShaderFxcBuilderPath"
#define KA_STR_projectSettingShaderMacrosNode L"ShaderMacros"
#define KA_STR_projectSettingKernelMacrosNode L"KernelMacros"
#define KA_STR_projectSettingShaderIncludesNode L"ShaderIncludes"
#define KA_STR_projectSettingShaderPlatform L"ShaderPlatform"
#define KA_STR_projectSettingShaderProfile L"ShaderProfile"
#define KA_STR_projectSettingShaderEntryPoint L"ShaderEntryPoint"
#define KA_STR_projectSettingShaderGLType L"ShaderGLType"
#define KA_STR_fileGLGeom L"Geom"
#define KA_STR_fileGLFrag L"Frag"
#define KA_STR_fileGLTesc L"Tesc"
#define KA_STR_fileGLTese L"Tese"
#define KA_STR_fileGLVert L"Vert"
#define KA_STR_fileGLComp L"Comp"
#define KA_STR_fileGLShader L"GLSL"
#define KA_STR_fileCLKernel L"OpenCL"
#define KA_STR_fileDXShader L"HLSL"
#define KA_STR_fileUnknown L"Unknown"
#define KA_STR_programName   L"ProgramName"
#define KA_STR_program       L"Program"
#define KA_STR_programDirectory L"ProgramDirectory"
#define KA_STR_programType L"ProgramType"
#define KA_STR_projectArchitecture L"ProjectArchitecture"
#define KA_STR_programSection  L"ProgramSection"
#define KA_STR_sourceFileId L"SourceFileId"
#define KA_STR_sourceFile L"SourceFile"
#define KA_STR_sourceFileStage L"stage"
#define KA_STR_sourceFileStageVertex L"vertex"
#define KA_STR_sourceFileStageTesc L"tesc"
#define KA_STR_sourceFileStageTese L"tese"
#define KA_STR_sourceFileStageGeom L"geom"
#define KA_STR_sourceFileStageShader L"shader"
#define KA_STR_sourceFileStageComputeShader L"comp"
#define KA_STR_sourceFileStageFrag L"frag"

// Stage abbreviations for output file names.
#define KA_STR_CLI_VERTEX_ABBREVIATION    L"vert"
#define KA_STR_CLI_TESS_CTRL_ABBREVIATION L"tesc"
#define KA_STR_CLI_TESS_EVAL_ABBREVIATION L"tese"
#define KA_STR_CLI_GEOMETRY_ABBREVIATION  L"geom"
#define KA_STR_CLI_FRAGMENT_ABBREVIATION  L"frag"
#define KA_STR_CLI_COMP_ABBREVIATION    L"comp"

#define KA_STR_CLI_VERTEX_LONG    L"Vertex"
#define KA_STR_CLI_TESS_CTRL_LONG L"Tessellation Control"
#define KA_STR_CLI_TESS_EVAL_LONG L"Tessellation Evaluation"
#define KA_STR_CLI_GEOMETRY_LONG  L"Geometry"
#define KA_STR_CLI_FRAGMENT_LONG  L"Fragment"
#define KA_STR_CLI_COMPUTE_LONG  L"Compute"



#define KA_STR_sourceFilesSection L"SourceFiles"
#define KA_STR_VertexId            L"VertexId"
#define KA_STR_TeseId              L"TeseId"
#define KA_STR_TescId              L"TescId"
#define KA_STR_GeomId              L"GeometryId"
#define KA_STR_FragId              L"FragId"
#define KA_STR_ComputeId           L"ComputeId"
#define KA_STR_shaderIdSection     L"ShaderFileIdSection"

#define KA_STR_filePropertiesModel L"Model"
#define KA_STR_filePropertiesEntryPoint L"EntryPoint"
#define KA_STR_filePropertiesType L"Type"

// Commands
#define KA_STR_addFileSelectionTitle "Add Existing Source Files"
// Linux does not support DirectX so only OpenGL shader files are supported

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define KA_STR_addFileSelectionFileDetails "All Kernel/Shader Files (*.cl *.glsl *.vs *.vert *.fs *.frag *.cs *.comp *.gs *.geom *.tesc *.tese);OpenCL Kernel Files (*.cl);OpenGL Shader Files (*.glsl *.vs *.vert *.fs *.frag *.cs *.comp *.gs *.geom *.tesc *.tese);All Files (*.*)"
#else
    #define KA_STR_addFileSelectionFileDetails "All Kernel/Shader Files (*.cl *.hlsl *.glsl *.vs *.vert *.fs *.frag *.cs *.comp *.gs *.geom *.hs *.ds *.ps *.tesc *.tese);;OpenCL Kernel Files (*.cl);;DirectX Shader Files (*.hlsl *.vs *.vert *.fs *.frag *.cs *.comp *.gs *.geom *.hs *.ds *.ps);;OpenGL Shader Files (*.glsl *.vs *.vert *.fs *.frag *.cs *.comp *.gs *.geom *.tesc *.tese);;All Files (*.*)"
#endif

#define KA_STR_ComputeShaderProfilePrefix L"cs"
#define KA_STR_DirectXShaderExtensions "vs,hs,ds,gs,ps,cs,hlsl"
#define KA_STR_OpenGLShaderExtensions "fs,frag,vs,vert,cs,comp,gs,geom,tesc,tese,glsl"
#define KA_STR_DXIndicator L"dx"
#define KA_STR_GLIndicator L"gl"
#define KA_STR_DXShaderLastVersionSuffix L"_5_0"
#define CSV_HEADER "Address, Opcode, Operands, Cycles, Functional Unit, Hex\n"


// Overview View
#define KA_STR_overViewTitle L" - Overview"
#define KA_STR_overviewName L"Overview"
#define KA_STR_overviewNameASCII "Overview"
#define KA_STR_overviewExtension L"cxlovr"
#define KA_STR_overviewRefreshButton "Refresh Kernels List"
#define KA_STR_overviewLastModifed "Last modified: "
#define KA_STR_overviewTableInformation "Work dimensions used in kernel analysis to estimate running time"
#define KA_STR_overviewTableCaption "Emulation Dimensions"
#define KA_STR_overviewEmptyKernelList "No kernels found in the file."

// Build
#define KA_STR_buildMainDirectoryName L"_AnalyzerOutput"
#define KA_STR_buildMainProgramOutput L"_Output"
#define KA_STR_buildMainILFileName L"IL"
#define KA_STR_buildMainISAFileName L"ISA"
#define KA_STR_buildMainILFileNameASCII "IL"
#define KA_STR_buildMainISAFileNameASCII "ISA"
#define KA_STR_buildMainStatisticsFileName L"Statistics"
#define KA_STR_buildMainAnalysisFileName L"Analysis"
#define KA_STR_buildMainBinaryFileName L"bin"
#define KA_STR_buildMainBinaryFileNameASCII "bin"
#define KA_STR_buildDXEntryPoint "main"

// Progress notification
#define KA_STR_postProcessing "Post-processing for"
#define KA_STR_postProcessingDone "Done!"
#define KA_STR_analyzing "Analyzing"
#define KA_STR_generatingStatistics "\nGenerating statistics:\n----------------------"

#define KA_STR_buildMainILType "0"
#define KA_STR_buildMainISAType "1"
#define KA_STR_buildMainISAandILType "2"
#define KA_STR_buildMainAnalysisType "3"
#define KA_STR_buildMainStatisticType "4"

#define KA_STR_SPLIT_CHAR "$"

// kernels views
#define KA_STR_fileSectionSeperator "_"
#define KA_STR_fileSectionSeperatorW L"_"
#define KA_STR_kernelViewFile L"devices"
#define KA_STR_kernelViewExtension L"cxltxt"
#define KA_STR_kernelViewILExtension L"cxlil"
#define KA_STR_kernelViewILExtensionASCII "cxlil"
#define KA_STR_kernelViewISAExtension L"cxlisa"
#define KA_STR_kernelViewBinExtension L"bin"
#define KA_STR_analysisTableInfo "<b>Analysis generated by emulated execution</b>"
#define KA_STR_analysisTotalCyclesRowName "Total clock cycles"
#define KA_STR_analysisCodeLen "Code Length"
#define KA_STR_analysisTableRows "Device,ISA branches executed,Clock cycles per wavefront,Total clock cycles,SALU instructions,SFetch instructions,VALU instructions,VFetch instructions,VWrite instructions,LDS instructions,GDS instructions,Atomic instructions,SGPRs,VGPRs,Wavefronts,Code Length"
#define KA_STR_analysisTableRowsTooltip "The name of OpenCL device being analyzed#Branches Evaluated to True:\nFirst Column: All instructions evaluated to True\nSecond column: Some instructions evaluated to True\nThird column: none instructions evaluated to True\
                                        #The estimated number of cycles the kernel execution is expected to require per wavefront.\
                                        #The estimated number of cycles the kernel execution is expected to require.\
                                        #The estimated average number of scalar ALU instructions executed per work-item (affected by flow control).\
                                        #The estimated average number of scalar fetch instructions from the video memory executed per work-item (affected by flow control).\
                                        #The estimated average number of vector ALU instructions executed per work-item (affected by flow control).\
                                        #The estimated average number of vector fetch instructions from the video memory executed per work-item (affected by flow control).\
                                        #The estimated average number of vector write instructions  to the video memory executed per work-item (affected by flow control).\
                                        #The estimated average number of LDS read or LDS write instructions executed per work-item (affected by flow control).\
                                        #The estimated average number of instructions to/from GDS executed per work-item (affected by flow control).\nThis counter is a subset of VALUInsts counter.\
                                        #The estimated average number of atomic instructions executed per work-item (affected by flow control).\
                                        #The number of general purpose scalar registers used by kernel.\
                                        #The number of general purpose vector registers used by kernel.\
                                        #The total number of wavefronts.\
#The size of the kernel in binary form. Performance may degrade if this size exceeds the size of the device instruction cache."
#define KA_STR_analysisTableMainColumn "Family"
#define KA_STR_analysisTableSecondColumn "Southern Island"

#define KA_STR_statisticsScratchReg "ScratchRegs"
#define KA_STR_statisticsTableInfo "<b>OpenCL Build Statisistics:</b>"
#define KA_STR_statisticsTableRows "Device,ScratchRegs,ThreadsPerWorkGroup,WavefrontSize,MaxLDSSize,LDSSize,MaxSGPRs,SGPRs,MaxVGPRs,VGPRs,ReqdWorkGroupX,ReqdWorkGroupY,ReqdWorkGroupZ,Code Length"
#define KA_STR_statisticsTableRowsTooltip   "The name of the OpenCL device being analyzed.\
                                            #The number of scratch registers used by the kernel.\nTo improve performance, decrease this number to zero by reducing\nthe number of GPRs used by the kernel.\
                                            #The number of work-items in a work-group.\
                                            #The number of work-items in a wavefront for the device.\
                                            #The maximum amount of LDS (Local Data Share) in bytes per kernel supported by the device.\
                                            #The amount of LDS (Local Data Share) in bytes being used by the kernel.\
                                            #The maximum number of scalar General Purpose Registers per kernel supported by the device\nif running on an AMD Radeon HD 7000 series or newer.\
                                            #The number of scalar General Purpose Registers used by the kernel\nif running on an AMD Radeon HD 7000 series or newer.\
                                            #The maximum number of vector General Purpose Registers per kernel supported by the device.\
                                            #The number of vector General Purpose Registers used by the kernel.\
                                            #Required workgroup X size specified for the kernel. Defined by optional __attribute__((reqd_work_group_size(X, Y, Z))).\
                                            #Required workgroup Y size specified for the kernel. Defined by optional __attribute__((reqd_work_group_size(X, Y, Z))).\
                                            #Required workgroup Z size specified for the kernel. Defined by optional __attribute__((reqd_work_group_size(X, Y, Z))).\
#The size of the kernel in binary form. Performance may degrade if this size exceeds the size of the device instruction cache."
#define KA_STR_statisticsTableMainColumn "Family"
#define KA_STR_CAL_NA_Value_64 "18446744073709551615"
#define KA_STR_statisticsTableComboGeneralDeviceLine "Evergreen and Northern Island devices"

#define KA_STR_statisticsTableComboText "<b>%1:</b> Statistics generated during build for"

#define KA_STR_statisticsTableTipReferenceTableCaptionTab1 "Performance Reference Tables"
#define KA_STR_statisticsTableTipSCSourceDumpCaptionTab2 "SC SRCSHADER Dump"
#define KA_STR_statisticsTableTipComputeShaderDataCaptionTab3 "Compute Shader Data"
#define KA_STR_statisticsTableTipGroupDescription "The effect of resource usage on the number of concurrent waves"
#define KA_STR_statisticsTableMaxWavesForSIMD "Max waves/SIMD:"
#define KA_STR_statisticsTableSGPRsWaves "Num of SGPRs used:"
#define KA_STR_statisticsTableVGPRsWaves "Num of VGPRs used:"
#define KA_STR_statisticsTableLDSWaves "Amount of LDS used (bytes):"
#define KA_STR_statisticsTableLDSUsage "%1 (Static: %2, Dynamic: %3)"
#define KA_STR_statisticsTableLDSUsedInfo "Applies to local workgroup dimensions of (%1,%2,%3) and Dynamic LDS Usage (%4 bytes) as set above"
#define KA_STR_statisticsTableLDSDynamicaAvailable "Amount of LDS available without reducing concurrency: %1 bytes"
#define KA_STR_statisticsTableAdvice "Performance advice: To increase the number of waves in flight, %1"
#define KA_STR_statisticsTableAdviceOneToTwo "replace some of your use of %1 with %2 and %3"
#define KA_STR_statisticsTableAdviceTwoToOne "replace some of your use of %1 and %2 with %3"
#define KA_STR_statisticsTableFinalTip "It is generally recommended to increase the number of waves in-flight. However, this is not the only factor to consider. Increasing the number of waves in-flight does not always translate\nto increased kernel performance.\nThe performance of kernels that do a lot of memory transactions is likely to benefit from having more waves in-flight.\nThis is because the device can switch to process instructions from a different wavefront instead of stalling to wait for the memory result.\nKernels that have few memory operations may benefit from a smaller number of wavefronts in-flight due to better cache and memory utilization by the compiler(less memory trashing)."
#define KA_STR_statisticsNoStatisticsToShow "<b>Statistics are not available for the selected devices.<b>"

// Statistics table information
#define KA_STR_statisticsTableResource "Resource"
#define KA_STR_statisticsTableLocalWorkGroup "<b>Local Workgroup:</b>"
#define KA_STR_statisticsTableLocalWorkGroupX "X:"
#define KA_STR_statisticsTableLocalWorkGroupY "Y:"
#define KA_STR_statisticsTableLocalWorkGroupZ "Z:"
#define KA_STR_statisticsTableDynamicLDS "<b>Dynamic LDS Usage:</b>"
#define KA_STR_statisticsTableDynamicBytes "bytes"
#define KA_STR_statisticsTableUsage "Usage"
#define KA_STR_statisticsTableConstraint "Constraint on Max Waves per SIMD (1-10)"
#define KA_STR_statisticsTableSGPRsRange "SGPRs (0-102)"
#define KA_STR_statisticsTableVGPRsRange "VGPRs (0-256)"
#define KA_STR_statisticsTableLDSRange "LDS size (0-32,768)"
#define KA_STR_statisticsTableMaxWave "Effective concurrency constraint (Max waves per SIMD):"
#define KA_STR_statisticsTableRegisters "Registers"
#define KA_STR_statisticsTableRecommendedUsage "Recommended Usage"
#define KA_STR_statisticsTablePerformanceImpact "Performance Impact"
#define KA_STR_statisticsTableISASize "ISA Size"
#define KA_STR_statisticsTableISASizeToolTip "GCN GPUs are equipped with 32KB of instruction cache per 4 ComputeUnits.\nRunning a kernel larger than 32KB may degrade performance due to instruction fetch latency.\nConsider splitting your kernel to several shorter ones"
#define KA_STR_statisticsTableScratchRegisters "Scratch Registers"
#define KA_STR_statisticsTableScratchRegistersToolTip "The GPU has a fixed size register file. If the kernel requires more General Purpose Registers (GPRs) \nthan available in the GPU, the compiler will use global memory as a scratch pad for registers.\nSince global memory is an order of magnitude slower than the GPU register file, this behavior may degrade performance.\nConsider splitting your kernel to several shorter kernels"
#define KA_STR_statisticsTable32KB "<=32KB"
#define KA_STR_statisticsTable0 "0"
#define KA_STR_statisticsTableKB "KB"
#define KA_STR_statisticsTableMeetsRecommended "Meets recommended usage"
#define KA_STR_statisticsTableMayDegrade "May degrade performance"
#define KA_STR_statisticsTableSGPRs "SGPRs"
#define KA_STR_statisticsTableVGPRs "VGPRs"
#define KA_STR_statisticsTableLDS "LDS"

// Static analysis output
#define KA_STR_outputPaneCaption L"Output"
#define KA_STR_outputPaneDockWithDebugProcessView L"Debugged Process Events"

// toolbar
#define KA_STR_toolbarName "Static Analyzer Toolbar"

#define KA_STR_toolbarBuildButton "Kernel/Shader Build"
#define KA_STR_toolbarInfoLabelToolTip "Build options for the active source file"

#define KA_STR_toolbarInfoLabel "Build Options:"
#define KA_STR_toolbarOptionsButton "Select Devices..."
#define KA_STR_toolbarOptionsButtonTooltip "Select devices used for Kernel Analysis"
#define KA_STR_toolbarKernelNamesTooltip "Kernel names"
#define KA_STR_toolbarDXShaderTypesTooltip "Shader types"
#define KA_STR_toolbarSettingsButton "..."
#define KA_STR_toolbarSettingsButtonTooltip "Edit build options"
#define KA_STR_toolbarPlatformComboTooltip "Select Build Platform"
#define KA_STR_toolbarBuildArchitectureComboTooltip "Select Build Architecture"
#define KA_STR_toolbarDXShaderProfileComboTooltip "Build profile"
#define KA_STR_toolbarEntryPointLabel "Entry point: "
#define KA_STR_toolbarEntryPointComboTooltip "Entry point names"
#define KA_STR_toolbarKernelLabel "Kernel: "
#define KA_STR_platformOpenCL "OpenCL"
#define KA_STR_platformOpenCL_GT L"OpenCL"
#define KA_STR_platformDirectX "DirectX"
#define KA_STR_platformDirectX_GT L"DirectX"
#define KA_STR_toolbarDXShaderProfileData "cs_4_0 cs_4_1 cs_5_0 ds_5_0 gs_4_0 gs_4_1 gs_5_0 hs_5_0 ps_4_0 ps_4_1 ps_5_0 vs_4_0 vs_4_1 vs_5_0"
#define KA_STR_VS_toolbarDXShaderProfileData L"cs_4_0", L"cs_4_1", L"cs_5_0", L"ds_5_0", L"gs_4_0", L"gs_4_1", L"gs_5_0", L"hs_5_0", L"ps_4_0", L"ps_4_1", L"ps_5_0", L"vs_4_0", L"vs_4_1", L"vs_5_0"
#define KA_STR_toolbarDXShaderModel "5_0 4_1 4_0"
#define KA_STR_VS_toolbarDXShaderModel L"5_0", L"4_1", L"4_0"
#define KA_STR_toolbarDXShaderModelLabel "DX Shader Model"
#define KA_STR_VS_toolbarDXShaderModelLabel L"DX Shader Model"

#define KA_STR_toolbarTypeLabel "Type: "
#define KA_STR_platformOpenGL "OpenGL"
#define KA_STR_platformOpenGL_GT L"OpenGL"
#define KA_STR_platformVulkan "Vulkan"
#define KA_STR_platformVulkan_GT L"Vulkan"
#define KA_STR_toolbarTypesTooltip "OpenGL shader types"
#define KA_STR_toolbarTypeData "Fragment Vertex Compute Geometry TessCont TessEval"
#define KA_STR_VS_toolbarTypeData L"Fragment", L"Vertex", L"Compute", L"Geometry", L"TessCont" L"TessEval"
#define KA_STR_VS_DXShaderExtensions L"vs hs ds gs ps cs hlsl"
#define KA_STR_VS_GLShaderExtensions L"vs vert fs frag cs comp gs geom tesc tese glsl"
#define KA_STR_VS_CLKernelExtensions L"cl"
#define KA_STR_VS_toolbarPlatformData L"OpenCL", L"OpenGL", L"Vulkan", L"DirectX"
#define KA_STR_ColorFormatting "Use Color Formatting"
#define KA_STR_LabelIndicator "label_"
#define KA_STR_LABEL_HREF "<a href='%1'>%1</a>"
#define KA_STR_SourceTableViewColumnsTooltips "Instruction offset within the program|The operation to be performed|The data on which the operation should act|The number of clock cycles which are required by a Compute Unit in order to process the instruction for a 64-thread Wavefront, while neglecting the system load and any other runtime-related factor|The category of instructions to which the instruction belongs: Scalar Memory Read, Scalar Memory Write, Scalar Arithmetics, Vector Memory Read, Vector Memory Write, Vector Arithmetics, LDS, GDS, Export, Atomics, Flow Control, Flow Control Branch|Binary representation of the instruction, in hexadecimal format"
#define KA_STR_BranchInstructionTooltip "Branch-not-taken takes 4 clock cycles.\nBranch-taken takes 16 clock cycles (assuming that the branch address is found in the instruction cache)"
#define KA_STR_NonGCNVersions "v4 v5"
#define KA_STR_CommonDXShaderExtension "hlsl"
#define KA_STR_CommonGLShaderExtension "glsl"
#define KA_STR_VertexShaderIndicators "vert vs"
#define KA_STR_FragmentShaderIndicators "frag fs"
#define KA_STR_ComputeShaderIndicators "comp cs"
#define KA_STR_GeometryShaderIndicators "geom gs"
#define KA_STR_TessEvalShaderIndicators "tesseval tese teval"
#define KA_STR_TessContShaderIndicators "tesscont tesc tcont"
#define KA_STR_VertexShaderExtension "vs"
#define KA_STR_FragmentShaderExtension "fs"
#define KA_STR_ComputeShaderExtension "cs"
#define KA_STR_GeometryShaderExtension "gs"
#define KA_STR_TessellationEvaluationShaderExtension "tese"
#define KA_STR_TessellationControlShaderExtension "tesc"
#define KA_STR_toolbarDXShaderTypes "Vertex Hull Domain Geometry Pixel Compute"
#define KA_STR_toolbarDXShaderTypesGT L"Vertex", L"Hull", L"Domain", L"Geometry", L"Pixel", L"Compute"
#define KA_STR_toolbarDXShaderTypesShort "vs hs ds gs ps cs"
#define KA_STR_toolbarDXShaderTypesShortGT L"vs", L"hs", L"ds", L"gs", L"ps", L"cs"
// HTML
#define KA_STR_htmlInfoNumOfLines L"Number of Lines:"
#define KA_STR_htmlInfoCLFileCaption L"OpenCL Kernel Source File"
#define KA_STR_htmlInfoShaderFileCaption L"Shader Source File"
#define KA_STR_htmlInfoCLFileFullPath L"Full Path:"
#define KA_STR_htmlInfoKernels L"Kernels:"
#define KA_STR_htmlInfoShaders L"Shaders:"
#define KA_STR_htmlInfoMainTreeNode "<p>Static analyzer information:</p>"
#define KA_STR_htmlInfoEmptyKernelList L"No kernels found in the file."

// Multi view
#define KA_STR_sourceSectionOpenCL "<b> OpenCL Source Code"
#define KA_STR_ILSectionOpenCL "<b> IL Code"
#define KA_STR_ISASection "<b> ISA Code"
#define KA_STR_sourceSectionHLSL "<b> HLSL Source Code"
#define KA_STR_sourceSectionGLSL "<b> GLSL Source Code"
#define KA_STR_ILSectionHLSL "<b> D3D ASM Code"
#define KA_STR_ILSectionGLSL "<b> GLSL IL Code"
#define KA_STR_ISA_FileNamePostfix L"_ISA"
#define KA_STR_menuShow "Show "
#define KA_STR_sourceTab "Code"
#define KA_STR_ILNotAvailable "IL not available for target device"
#define KA_STR_ISANotAvailable "ISA not available for target device"
#define KA_STR_viewNotUpdated " <b>Not up to date</b>"
#define KA_STR_updatedLabel "updatedLabelObject"
#define KA_STR_ISA_DASMStart "; -------- Disassembly --------------------"
#define KA_STR_ISA_DASMStart2 "; --------  Disassembly --------------------"
#define KA_STR_ISA_CSDataStart "; ----------------- CS Data ------------------------"
#define KA_STR_ISA_SCShaderStart "; ------------- SC_SRCSHADER Dump ------------------"

// Options dialog
#define KA_STR_optionsDialogOptionColumnCaption "Build Option"
#define KA_STR_optionsDialogValueColumnCaption "Value"
#define KA_STR_optionsDialogEditCaption "OpenCL Build Command Line"
#define KA_STR_optionsDialogGeneralSection "General"
#define KA_STR_optionsDialogPredefinedText "Predefined macros#-D"
#define KA_STR_optionsDialogPredefinedTextTooltip "Predefine macros should be separated by ';'. <br>If the Predefined macro needs to include a space, surround the macro with parentheses."
#define KA_STR_optionsDialogAdditionalText "Additional include directories#-I"
#define KA_STR_optionsDialogAdditionalTextTooltip "Additional include directories should be separated by ';'. <br>If the directory path includes a space, surround the macro with parentheses."
#define KA_STR_optionsDialogOpenCLFormatCombo "OpenCL format#Default,-x clc,-x clc++"
#define KA_STR_optionsDialogOpenCLFormatComboTooltip ""
#define KA_STR_optionsDialogDisableWarningCheck "Disable all warnings#-w"
#define KA_STR_optionsDialogDisableWarningCheckTooltip "Inhibit all warning messages."
#define KA_STR_optionsDialogTreatAsErrorCheck "Treat any warning as an error#-Werror"
#define KA_STR_optionsDialogTreatAsErrorCheckTooltip "Make all warnings into errors."

#define KA_STR_optionsDialogOptimizationSection "Optimization"
#define KA_STR_optionsDialogOptimizationCombo "Optimization level#Default,0 (-O0),1 (-O1),2 (-O2),3 (-O3),4 (-O4),5 (-O5)"
#define KA_STR_optionsDialogOptimizationComboTooltip "Specifies to the compiler not to optimize. This is equivalent to the OpenCL standard option -cl-opt-disable."
#define KA_STR_optionsDialogTreatDoubleCheck "Treat double float-point constant as single one#-cl-single-precision-constant"
#define KA_STR_optionsDialogTreatDoubleCheckTooltip "Treat double precision floating-point constant as single precision constant"
#define KA_STR_optionsDialogFlushCheck "Flush denormalized floating point numbers as zeros#-cl-denorms-are-zero"
#define KA_STR_optionsDialogFlushCheckTooltip "This option controls how single precision and double precision denormalized numbers are handled. If specified as a build option, the single precision denormalized numbers may be flushed to zero and if the optional extension for double precision is supported, double precision denormalized numbers may also be flushed to zero. This is intended to be a performance hint and the OpenCL compiler can choose not to flush denorms to zero if the device supports single precision (or double precision) denormalized numbers.\nThis option is ignored for single precision numbers if the device does not support single precision denormalized numbers i.e. CL_FP_DENORM bit is not set in CL_DEVICE_SINGLE_FP_CONFIG.\nThis option is ignored for double precision numbers if the device does not support double precision or if it does support double precison but CL_FP_DENORM bit is not set in CL_DEVICE_DOUBLE_FP_CONFIG.\nThis flag only applies for scalar and vector single precision floating-point variables and computations on these floating-point variables inside a program. It does not apply to reading from or writing to image objects."
#define KA_STR_optionsDialogCompilerAssumesCheck "Compiler assumes the strict aliasing rules#-cl-strict-aliasing"
#define KA_STR_optionsDialogCompilerAssumesCheckTooltip "This option allows the compiler to assume the strictest aliasing rules."
#define KA_STR_optionsDialogEnableMADCheck "Enable MAD#-cl-mad-enable"
#define KA_STR_optionsDialogEnableMADCheckTooltip "Allow a * b + c to be replaced by a mad. The mad computes a * b + c with reduced accuracy. For example, some OpenCL devices implement mad as truncate the result of a * b before adding it to c."
#define KA_STR_optionsDialogIgnoreSignednessCheck "Ignore the signedness of zero#-cl-no-signed-zeros"
#define KA_STR_optionsDialogIgnoreSignednessCheckTooltip "Allow optimizations for floating-point arithmetic that ignore the signedness of zero. IEEE 754 arithmetic specifies the behavior of distinct +0.0 and -0.0 values, which then prohibits simplification of expressions such as x+0.0 or 0.0*x (even with -clfinite-math only). This option implies that the sign of a zero result isn't significant."
#define KA_STR_optionsDialogAllowUnsafeCheck "Allow unsafe optimization#-cl-unsafe-math-optimizations"
#define KA_STR_optionsDialogAllowUnsafeCheckTooltip "Allow optimizations for floating-point arithmetic that (a) assume that arguments and results are valid, (b) may violate IEEE 754 standard and (c) may violate the OpenCL numerical compliance requirements as defined in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5. This option includes the -cl-no-signed-zeros and -cl-mad-enable options."
#define KA_STR_optionsDialogAssumeNaNCheck "Assume no NaN nor infinite#-cl-finite-math-only"
#define KA_STR_optionsDialogAssumeNaNCheckTooltip "Allow optimizations for floating-point arithmetic that assume that arguments and results are not NaNs or +/-?. This option may violate the OpenCL numerical compliance requirements defined in in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5."
#define KA_STR_optionsDialogAggressiveMathCheck "Do aggressive Math Optimization#-cl-fast-relaxed-math"
#define KA_STR_optionsDialogAggressiveMathCheckTooltip "Sets the optimization options -cl-finite-math-only and -cl-unsafe-math-optimizations. This allows optimizations for floating-point arithmetic that may violate the IEEE 754 standard and the OpenCL numerical compliance requirements defined in the specification in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5. This option causes the preprocessor macro __FAST_RELAXED_MATH__ to be defined in the OpenCL program."
#define KA_STR_optionsDialogCorrectlyRoundCheck "Correctly round single-precision FP divide & sqrt#-cl-fp32-correctly-rounded-divide-sqrt"
#define KA_STR_optionsDialogCorrectlyRoundCheckTooltip "The -cl-fp32-correctly-rounded-divide-sqrt build option to clBuildProgram or clCompileProgram allows an application to specify that single precision floating-point divide (x/y and 1/x) and sqrt used in the program source are correctly rounded. If this build option is not specified, the minimum numerical accuracy of single precision floating-point divide and sqrt are as defined in section 7.4 of the OpenCL specification.\nThis build option can only be specified if the CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT is set in CL_DEVICE_SINGLE_FP_CONFIG (as defined in in the table of allowed values for param_name for clGetDeviceInfo) for devices that the program is being build. clBuildProgram or clCompileProgram will fail to compile the program for a device if the -cl-fp32-correctly-rounded-divide-sqrt option is specified and CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT is not set for the device."

#define KA_STR_optionsDialogOtherSection "Other"
#define KA_STR_optionsDialogClVersionText "CL version supported#-cl-std"
#define KA_STR_optionsDialogClVersionTextTooltip "Determine the OpenCL C language version to use. A value for this option must be provided. Valid values are:\nCL1.1 - Support all OpenCL C programs that use the OpenCL C language features defined in section 6 of the OpenCL 1.1 specification.\nCL1.2 – Support all OpenCL C programs that use the OpenCL C language features defined in section 6 of the OpenCL 1.2 specification."
#define KA_STR_optionsDialogKernelArgumentCheck "Kernel argument info#-cl-kernel-arg-info"
#define KA_STR_optionsDialogKernelArgumentCheckTooltip "This option allows the compiler to store information about the arguments of a kernel(s) in the program executable. The argument information stored includes the argument name, its type, the address and access qualifiers used. Refer to description of clGetKernelArgInfo on how to query this information."
#define KA_STR_optionsDialogCreateLibraryCheck "Create library#-create-library"
#define KA_STR_optionsDialogCreateLibraryCheckTooltip "Create a library of compiled binaries specified in input_programs argument to clLinkProgram."
#define KA_STR_optionsDialogEnableLinkCheck "Enable link options#-enable-link-options"
#define KA_STR_optionsDialogEnableLinkCheckTooltip "Allows the linker to modify the library behavior based on one or more link options (described in Program Linking Options, below) when this library is linked with a program executable. This option must be specified with the -create-library option."
#define KA_STR_optionsDialogProduceDebuggingCheck "Produce debugging information#-g"
#define KA_STR_optionsDialogProduceDebuggingCheckTooltip "This is an experimental feature that lets you use the GNU project debugger, GDB, to debug kernels on x86 CPUs running Linux or cygwin/minGW under Windows. For more details, see Chapter 3, \"Debugging OpenCL.\" This option does not affect the default optimization of the OpenCL code."
#define KA_STR_optionsDialogSpecifyUAVCombo "Specify that UAV per pointer should be used\n(HD5XXX and HD6XXX series GPU's only)#Default,Yes (-fper-pointer-uav),No (-fno-per-pointer-uav)"
#define KA_STR_optionsDialogSpecifyUAVComboTooltip ""
#define KA_STR_optionsDialogOpenCLBif3Combo "Allow OpenCL binary to be BIF3.0 format#Default,Yes (-fbin-bif30),No (-fno-bin-bif30)"
#define KA_STR_optionsDialogOpenCLBif3ComboTooltip ""
#define KA_STR_optionsDialogEncryptedOpenCLCombo "Generate an encrypted OpenCL binary (not by default)#Default,Yes (-fbin-encrypt),No (-fno-bin-encrypt)"
#define KA_STR_optionsDialogEncryptedOpenCLComboTooltip ""
#define KA_STR_optionsDialogStoreTempCheck "Store temporary files in current directory#-save-temps"
#define KA_STR_optionsDialogStoreTempCheckTooltip "This option dumps intermediate temporary files, such as IL and ISA code, for each OpenCL kernel. If <prefix> is not given, temporary files are saved in the default temporary directory (the current directory for Linux, C:\\Users\\<user>\\AppData\\Local for Windows). If \\<prefix\\> is given, those temporary files are saved with the given <prefix>. If <prefix> is an absolute path prefix, such as C:\\your\\work\\dir\\mydumpprefix, those temporaries are saved under C:\\your\\work\\dir, with mydumpprefix as prefix to all temporary names. For example, under the default directory"
#define KA_STR_optionsDialogTempPrefixText "    Temporary files prefix#-save-temps"
#define KA_STR_optionsDialogTempPrefixTextTooltip "This option dumps intermediate temporary files, such as IL and ISA code, for each OpenCL kernel. If <prefix> is not given, temporary files are saved in the default temporary directory (the current directory for Linux, C:\\Users\\<user>\\AppData\\Local for Windows). If \\<prefix\\> is given, those temporary files are saved with the given <prefix>. If <prefix> is an absolute path prefix, such as C:\\your\\work\\dir\\mydumpprefix, those temporaries are saved under C:\\your\\work\\dir, with mydumpprefix as prefix to all temporary names. For example, under the default directory"
#define KA_STR_optionsDialogUseJITCombo "Use JIT for CPU target (disable if debugging is enabled#Default,Yes (-fuse-jit),No (-fno-use-jit)"
#define KA_STR_optionsDialogUseJITComboTooltip ""
#define KA_STR_optionsDialogForUseJITCombo "Force use JIT for CPU target (even if debugging is enabled)#Default,Yes (-fforce-jit),No (-fno-force-jit)"
#define KA_STR_optionsDialogForUseJITComboTooltip ""
#define KA_STR_optionsDialogDisableAVXCombo "Disable AVX code generation#Default,Yes (-fdisable-avx),No (-fno-disable-avx)"
#define KA_STR_optionsDialogDisableAVXComboTooltip ""
#define KA_STR_optionsDialogEnablefmaCombo "Enable fma for a*b+c#Default,Yes (-ffma-enable),No (-fno-fma-enable)"
#define KA_STR_optionsDialogEnablefmaComboTooltip ""
#define KA_STR_optionsDialogReplaceMathFunctionCheck "Replace math function calls with native version#-fuse-native"
#define KA_STR_optionsDialogReplaceMathFunctionCheckTooltip ""
#define KA_STR_optionsDialogNativeVersionText "    Native version#-fuse-native"
#define KA_STR_optionsDialogNativeVersionTextTooltip ""

#define KA_STR_captionLabelStyleSheet "background-color:#DDDDDD; text-align:left; font-weight:bold; height:25px; padding-top: 3px; padding-bottom: 3px; padding-left: 0px; padding-right: 0px; "

#define KA_STR_KernelFileStructureInvalid "Invalid file name: file name should be of the form <name>.<extension>"
#define KA_STR_FileNameOnlyWhitespaces "File name should not contain only whitespace character(s)."
#define KA_STR_FileNameLeadingOrTrailing "File name should not contain leading or trailing whitespace character(s)."
#define KA_STR_FileNameEmpty "File name should not be empty."
#define KA_STR_FileNameSpecial "File name should not contain any of the following characters:\n   \\ / : * ? \" < > | %"
#define KA_STR_FileNameExists "A file with the same name already exists."
#define KA_STR_BinariesExist  "The selected directory already contains files with the specified names. Would you like to overwrite them?"
#define KA_STR_NoBinaries "Failed to export binaries. \nNo binaries exist for the selected kernel. Please build the kernel and try again."
#define KA_STR_BinariesExportedSuccessfully "The binaries were exported succefully."
#define KA_STR_BinariesExportOverwriteWarningMessage  R"(%1 already exists in the selected location. Do you want to replace it?)"
#define KA_STR_FileWritePermissionError "Failed to export binaries.\nPlease make sure that you have the required permissions for writing to the destination folder."
#define KA_STR_NoBinariesForDevice L"No binary for device \'%ls\'.\n"
#define KA_STR_ProgramNameEmpty "Program name should not be empty."
#define KA_STR_ProgramNameOnlyWhitespaces "Program name should not contain only whitespace character(s)."
#define KA_STR_ProgramNameLeadingOrTrailing "Program name should not contain leading or trailing whitespace character(s)."
#define KA_STR_ProgramNameSpecial "Program name should not contain any of the following characters:\n   \\ / : * ? \" < > | %"
#define KA_STR_ProgramNameExists "A program with the same name already exists."

// Shader option dialog
#define KA_STR_HLSL_optionsDialogBuildOptionsSection "HLSL Build Options"
#define KA_STR_HLSL_optionsDialogD3dCompiler "D3D Compiler DLL"
#define KA_STR_HLSL_optionsDialogFXCCompiler "FXC Tool"
#define KA_STR_HLSL_optionsDialogPredefMacros "Predefined macros#-D"
#define KA_STR_HLSL_optionsDialogPredefMacrosTooltip "Predefined macros (-D): a semicolon-separated list of key-value pairs in the form of \"key=value\"."
#define KA_STR_HLSL_optionsDialogIncludeDirs "Additional include directories#-I"
#define KA_STR_HLSL_optionsDialogIncludeDirsTooltip "Additional include directories (-I): a semicolon-separated list of directories."
#define KA_STR_HLSL_optionsDialogAvoidFlowControl "Avoid Flow Control#D3DCOMPILE_AVOID_FLOW_CONTROL#/Gfa"
#define KA_STR_HLSL_optionsDialogDebug "Debug#D3DCOMPILE_DEBUG#/Zi"
#define KA_STR_HLSL_optionsDialogEnableBackwordsCompatibility "Enable Backwards Compatibility#D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY#/Gec"
#define KA_STR_HLSL_optionsDialogEnableStrictness "Enable Strictness#D3DCOMPILE_ENABLE_STRICTNESS#/Ges"
#define KA_STR_HLSL_optionsDialogEPixelOptOff "Force Pixel Shader Optimization Off#D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT#"
#define KA_STR_HLSL_optionsDialogVertexOptOff "Force Vertex Shader Optimizations Off#D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT#"
#define KA_STR_HLSL_optionsDialogIeeeStrictness "IEEE Strictness#D3DCOMPILE_IEEE_STRICTNESS#/Gis"
#define KA_STR_HLSL_optionsDialogNoPres "No Preshader#D3DCOMPILE_NO_PRESHADER#/Op"
#define KA_STR_HLSL_optionsDialogOptLevel "Optimization Level#Skip Optimization;D3DCOMPILE_SKIP_OPTIMIZATION;/Od, 0 (Lowest optimization);D3DCOMPILE_OPTIMIZATION_LEVEL0;/O0, 1 (Default);;, 2;D3DCOMPILE_OPTIMIZATION_LEVEL2;/O2, 3 (Highest optimization);D3DCOMPILE_OPTIMIZATION_LEVEL3;/O3"
#define KA_STR_HLSL_optionsDialogPackMatrixColMajor "Pack Matrix Column Major#D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR#/Zpc"
#define KA_STR_HLSL_optionsDialogPackMatrixRowMajor "Pack Matrix Row Major#D3DCOMPILE_PACK_MATRIX_ROW_MAJOR#/Zpr"
#define KA_STR_HLSL_optionsDialogPartialPrecision "Partial Precision#D3DCOMPILE_PARTIAL_PRECISION#/Gpp"
#define KA_STR_HLSL_optionsDialogPreferFlowControl "Prefer Flow Control#D3DCOMPILE_PREFER_FLOW_CONTROL#/Gfp"
#define KA_STR_HLSL_optionsDialogResourcesMatAlias "Resources May Alias#D3DCOMPILE_RESOURCES_MAY_ALIAS#/res_may_alias"
#define KA_STR_HLSL_optionsDialogSkipValidation "Skip Validation#D3DCOMPILE_SKIP_VALIDATION#/Vd"
#define KA_STR_HLSL_optionsDialogWarningsAreErrors "Warnings Are Errors#D3DCOMPILE_WARNINGS_ARE_ERRORS#/WX"
#define KA_STR_HLSL_optionsDialogOutputHexLiterals "Output hexadecimal literals##/Lx"
#define KA_STR_HLSL_optionsDialogNumOfInst "Numbering of instructions in assembly listings##/Ni"
#define KA_STR_HLSL_optionsDialogOutputInstInAsm "Output instruction byte offset in assembly listings##/No"
#define KA_STR_HLSL_optionsDialogStripDebug "Strip debug data from 4.0 + shader bytecode##/Qstrip_debug"
#define KA_STR_HLSL_optionsDialogStripPrivate "Strip private data from 4.0 + shader bytecode##/Qstrip_priv"
#define KA_STR_HLSL_optionsDialogStripReflection "Strip reflection data from 4.0 + shader bytecode##/Qstrip_reflect"
#define KA_STR_HLSL_optionsDialogBuildCommand "HLSL Build Command Line"

#define KA_STR_HLSL_optionsDialogD3DFileName_GT L"d3dcompiler*.dll"
#define KA_STR_HLSL_optionsDialogFXCFileName_GT L"fxc.exe"
#define KA_STR_HLSL_optionsDialogD3DFileName_QT "d3dcompiler*.dll"
#define KA_STR_HLSL_optionsDialogFXCFileName_QT "fxc.exe"
#define KA_STR_HLSL_optionsDialogVSBuilderFilesRelativeDirPath L"\\Extensions\\AMD\\CodeXL"
#define KA_STR_HLSL_optionsDialogfxcBuilderFileDirPath L"C:\\Program Files (x86)\\Windows Kits\\8.1\\bin"
#define KA_STR_HLSL_optionsDialogBrowse "Browse..."
#define KA_STR_HLSL_optionsDialogDefaultCompiler "D3DCompiler_47.dll (bundled with CodeXL)"
#define KA_STR_HLSL_optionsDialogLocatedAtMessage "Located at "
#define KA_STR_HLSL_optionsDialogFxcLocationErrMessage L"FXC's location not specified."
#define KA_STR_HLSL_optionsDialogD3dLocationErrMessage L"D3D compiler's location not specified."
#define KA_STR_HLSL_optionsDialogFileError "Invalid name of selected DLL. Please select a 32-bit D3DCompiler_*.DLL"

#define KA_STR_HLSL_optionsDialogD3DFileType "Dll files(*.dll)"
#define KA_STR_HLSL_optionsDialogFXCFileType "Fxc exe files(FXC.exe)"
#define KA_STR_HLSL_optionsDialogFXCExeSelectionCaption "Select the FXC executable"
#define KA_STR_HLSL_optionsDialogD3DCompilerSelectionCaption "Select the D3D compiler"

#define KA_STR_HLSL_optionsDialogD3DCompileType "D3D"
#define KA_STR_HLSL_optionsDialogFXCCompileType "FXC"

#define KA_STR_HLSL_optionsDialogAvoidFlowControlToolTip "Directs the compiler to not use flow-control constructs where possible."
#define KA_STR_HLSL_optionsDialogDebugToolTip "Directs the compiler to insert debug file/line/type/symbol information into the output code."
#define KA_STR_HLSL_optionsDialogEnableBackwordsCompatibilityToolTip "Directs the compiler to enable older shaders to compile to 5_0 profiles."
#define KA_STR_HLSL_optionsDialogEnableStrictnessToolTip "Forces strict compile, which might not allow for legacy syntax. By default, the compiler disables strictness on deprecated syntax."
#define KA_STR_HLSL_optionsDialogEPixelOptOffToolTip "Directs the compiler to compile a pixel shader for the next highest shader profile. This constant also turns debugging on and optimizations off."
#define KA_STR_HLSL_optionsDialogVertexOptOffToolTip "Directs the compiler to compile a vertex shader for the next highest shader profile. This constant turns debugging on and optimizations off."
#define KA_STR_HLSL_optionsDialogIeeeStrictnessToolTip "Forces the IEEE strict compile."
#define KA_STR_HLSL_optionsDialogNoPresToolTip "Directs the compiler to disable Preshaders. If you set this constant, the compiler does not pull out static expression for evaluation."
#define KA_STR_HLSL_optionsDialogOptLevelToolTip "Directs the compiler to use the specified level of optimization:"\
    "<br>Skip - skip optimization steps during code generation."\
    "<br>&nbsp; &nbsp; &nbsp; We recommend that you set this constant for debug purposes only."\
    "<br>Lowest level - If you set this constant, the compiler might produce slower code but produces the code more quickly."\
    "<br>&nbsp; &nbsp; &nbsp; Set this constant when you develop the shader iteratively.Second lowest level.Second highest level."\
    "<br>Highest level - If you set this constant, the compiler produces the best possible code but might take significantly longer to do so."\
    "<br>&nbsp; &nbsp; &nbsp; Set this constant for final builds of an application when performance is the most important factor."
#define KA_STR_HLSL_optionsDialogPackMatrixColMajorToolTip "Directs the compiler to pack matrices in column-major order on input and output from the shader. This type of packing is generally more efficient because a series of dot-products can then perform vector-matrix multiplication"
#define KA_STR_HLSL_optionsDialogPackMatrixRowMajorToolTip "Directs the compiler to pack matrices in row-major order on input and output from the shader."
#define KA_STR_HLSL_optionsDialogPartialPrecisionToolTip "Directs the compiler to perform all computations with partial precision. If you set this constant, the compiled code might run faster on some hardware."
#define KA_STR_HLSL_optionsDialogPreferFlowControlToolTip "Directs the compiler to use flow-control constructs where possible."
#define KA_STR_HLSL_optionsDialogResourcesMatAliasToolTip "Directs the compiler to assume that unordered access views (UAVs) and shader resource views (SRVs) may alias for cs_5_0.\nNote  This compiler constant is new starting with the D3dcompiler_47.dll."
#define KA_STR_HLSL_optionsDialogSkipValidationToolTip "Directs the compiler not to validate the generated code against known capabilities and constraints. We recommend that you use this constant only with shaders that have been successfully compiled in the past. DirectX always validates shaders before it sets them to a device."
#define KA_STR_HLSL_optionsDialogWarningsAreErrorsToolTip "Directs the compiler to treat all warnings as errors when it compiles the shader code. We recommend that you use this constant for new shader code, so that you can resolve all warnings and lower the number of hard-to-find code defects."
#define KA_STR_HLSL_optionsDialogOutputHexLiteralsToolTip ""
#define KA_STR_HLSL_optionsDialogNumOfInstToolTip ""
#define KA_STR_HLSL_optionsDialogOutputInstInAsmToolTip ""
#define KA_STR_HLSL_optionsDialogStripDebugToolTip ""
#define KA_STR_HLSL_optionsDialogStripPrivateToolTip ""
#define KA_STR_HLSL_optionsDialogStripReflectionToolTip ""

// Add files dialog
#define KA_STR_addFileTitle "Create a New Source File"
#define KA_STR_addFileCLTypes "OpenCL Kernel (*.cl)"
#define KA_STR_addFileDXTypes "Vertex Shader (*.vs),Hull Shader (*.hs),Domain Shader (*.ds),Geometry Shader (*.gs),Pixel Shader (*.ps),Compute Shader (*.cs),Generic HLSL source file (*.hlsl)"
#define KA_STR_addFileGLTypes "Fragment Shader (*.frag),Vertex Shader (*.vert),Compute Shader (*.comp),Geometry Shader (*.geom),Tessellation Control Shader (*.tesc),Tessellation Evaluation Shader (*.tese),Generic Shader (*.glsl)"
#define KA_STR_addFileGLSLCompute "Compute"
#define KA_STR_addFileGLSLGeneric "Generic"

// Create New Program dialog
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #define KA_STR_programTypesTitles L"glProgram,glProgram,vkProgram,vkProgram,clFolder,dxFolder"
#else // Building for Linux
    #define KA_STR_programTypesTitles L"glProgram,glProgram,vkProgram,vkProgram,clFolder"
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)


#define KA_STR_programTypeGL_Rendering L"ProgramGL_Rendering"
#define KA_STR_programTypeGL_Compute L"ProgramGL_Compute"
#define KA_STR_programTypeVK_Rendering L"ProgramVK_Rendering"
#define KA_STR_programTypeVK_Compute L"ProgramVK_Compute"
#define KA_STR_programTypeCL L"ProgramCL"
#define KA_STR_programTypeDX L"ProgramDX"
#define KA_STR_programTypeUnknown L"Unknown Type"

#define KA_STR_UiOutputDir32 L"Output (32-bit)"
#define KA_STR_UiOutputDir64 L"Output (64-bit)"

#define KA_STR_FileSystemOutputDir32 L"32-bit"
#define KA_STR_FileSystemOutputDir64 L"64-bit"

#define KA_STR_addFileCLDescription "An OpenCL-coded routine that is executed on the device (GPU, CPU or other), usually performed multiple times concurrently."
#define KA_STR_addFileVSDescription "A vertex shader processes vertices from the input assembler, performing per-vertex operations such as transformations, skinning, morphing, and per-vertex lighting. Vertex shaders always operate on a single input vertex and produce a single output vertex."
#define KA_STR_addFileHSDescription "A hull shader is the first of three stages that work together to implement tessellation.The hull - shader outputs drive the tessellator stage, as well as the domain - shader stage."
#define KA_STR_addFileDSDescription "A domain shader is the third of three stages that work together to implement tessellation.The domain shader generates the surface geometry from the transformed control points from a hull shader and the UV coordinates."
#define KA_STR_addFileGSDescription "A geometry shader runs with vertices as input and the ability to generate vertices on output. Unlike vertex shaders, which operate on a single vertex, the geometry shader's inputs are the vertices for a full primitive (two vertices for lines, three vertices for triangles, or single vertex for point). Geometry shaders can also bring in the vertex data for the edge-adjacent primitives as input (an additional two vertices for a line, an additional three for a triangle)."
#define KA_STR_addFilePSDescription "A pixel shader enables rich shading techniques such as per-pixel lighting and post-processing. A pixel shader is a program that combines constant variables, texture data, interpolated per-vertex values, and other data to produce per-pixel outputs."
#define KA_STR_addFileCSDescription "A compute shader provides high-speed general purpose computing and takes advantage of the large numbers of parallel processors on the graphics processing unit (GPU). The compute shader technology is also known as the DirectCompute technology."
#define KA_STR_addFileHLSLDescription "A shader written using the HLSL language.This can be any of the DirectX shader types shown above."
#define KA_STR_addFileFSDescription "A fragment shader is the OpenGL pipeline stage after a primitive is rasterized. For each sample of the pixels covered by a primitive, a 'fragment' is generated. Each fragment has a Window Space position, a few other values, and it contains all of the interpolated per-vertex output values from the last Vertex Processing stage."
#define KA_STR_addFileVertDescription "A vertex shader processes vertices from the input assembler, performing per-vertex operations such as transformations, skinning, morphing, and per-vertex lighting. Vertex shaders always operate on a single input vertex and produce a single output vertex."
#define KA_STR_addFileTESCDescription "A Tessellation Control Shader is a Shader program written in GLSL. It sits between the Vertex Shader and the Tessellation Evaluation Shader. The main purpose of the TCS is to feed the tessellation levels to the Tessellation primitive generator stage, as well as to feed patch data (as its output values) to the Tessellation Evaluation Shader stage."
#define KA_STR_addFileTESEDescription "A Tessellation Evaluation Shader is a Shader program written in GLSL that takes the results of a Tessellation operation and computes the interpolated positions and other per-vertex data from them. These values are passed on to the next stage in the pipeline."
#define KA_STR_addFileGLSLDescription "A shader written using the GLSL language.This can be any of the stage shader types shown above."
#define KA_STR_addFileSelectPlatformTitle "<b>Select Platform:"
#define KA_STR_addFileTypeSelectionTitle "Select the source file's type:"
#define KA_STR_addFileOpenCLTypesTitle "<b>OpenCL Kernels"
#define KA_STR_addFileDirectXTypesTitle "<b>DirectX Shaders"
#define KA_STR_addFileOpenGLTypesTitle "<b>OpenGL Shaders"
#define KA_STR_addFileVulkanTypesTitle "<b>Vulkan Shaders"
#define KA_STR_addFileDescriptionTitle "<b>Description"
#define KA_STR_addFileAssociateProgram "<b>Associate with Program/Folder"
#define KA_STR_addFileProgramComboItemNone "None"
#define KA_STR_addFileProgramComboItemNewProgram "Create New Program/Folder"
// Created files
#define KA_STR_createdFileHeader "/* Created with CodeXL */\n"
#define KA_STR_createdOpenCLFile "__kernel void %s()\n{\n}\n"

// D3D default shaders.
#define KA_STR_DEFAULT_DX_VS "void %s_VS(\n    uint vertexIndex : SV_VertexID,\n    float inputPosition : POSITION,\n    out float4 outputPosition : SV_Position)\n{\n    outputPosition = inputPosition;\n}\n"
#define DEFAULT_DX_CS "[numthreads(8, 8, 1)]\nvoid %s_CS(uint3 dispatchThreadID : SV_DispatchThreadID)\n{\n}\n"
#define DEFAULT_DX_GS "struct GSPS_INPUT\n{\n    float4 Pos : SV_POSITION;\n    float3 Norm : TEXCOORD0;\n    float2 Tex : TEXCOORD1;\n    };\n\n[maxvertexcount(12)]\nvoid Geometry12_GS(\n    triangle GSPS_INPUT input[3], \n    inout TriangleStream<GSPS_INPUT> TriStream \n)\n{\n    TriStream.RestartStrip();\n}\n"
#define DEFAULT_DX_HS "struct HullConstants\n{\n    float Edges[3] : SV_TessFactor;\n    float Inside : SV_InsideTessFactor;\n    float4 const1 : SOME_SEMNATIC1;\n    float4 const2 : SOME_SEMNATIC2;\n};\n\nstruct HullInputPatchPoint\n{\n    float4 pos : POSITION;\n};\n\nstruct HullOutputPatchPoint\n{\n    float4 pos : POSITION;\n};\n\nvoid HSConstants(in InputPatch<HullInputPatchPoint,3> patch,\n                     out HullConstants output)\n{\n    output.Edges[0] = 5.0;\n    output.Edges[1] = 5.0;\n    output.Edges[2] = 5.0;\n    output.Inside = 3.5;\n    output.const1 = float4(1.0,1.0,1.0,1.0);\n    output.const2 = float4(2.0,2.0,2.0,2.0);\n}\n\n[domain(\"tri\")]\n[partitioning(\"fractional_odd\")]\n[outputtopology(\"triangle_cw\")]\n[outputcontrolpoints(3)]\n[patchconstantfunc(\"HSConstants\")]\n[maxtessfactor(15.0)]\nHullOutputPatchPoint %s_HS(\n    in InputPatch<HullInputPatchPoint,3> patch ,\n    in uint controlPointID : SV_OutputControlPointID)\n{\n    HullOutputPatchPoint output;\n    output.pos = patch[controlPointID].pos;\n    return output;\n}\n"
#define DEFAULT_DX_DS "struct HullConstants1\n{ \n    float Edges[3] : SV_TessFactor; \n    float Inside : SV_InsideTessFactor; \n    float4 const1 : SOME_SEMNATIC1; \n    float4 const2 : SOME_SEMNATIC2; \n }; \n\nstruct DomainInputPatchPoint\n{ \n    float4 pos : POSITION; \n }; \n\nstruct DomainVertex\n{ \n    float4 pos : SV_POSITION; \n }; \n\n[domain(\"tri\")]\nvoid %s_DS(\n    in HullConstants1 constants,\n    in OutputPatch<DomainInputPatchPoint,3> patch,\n    in float3 barycentricCoordinates : SV_DomainLocation,\n     out DomainVertex output)\n{\n    float3 bc = barycentricCoordinates;\n    output.pos = bc.x * patch[0].pos + bc.y*patch[1].pos + bc.z* patch[2].pos;\n}\n"
#define DEFAULT_DX_PS "void %s_PS(\n    float4 position : SV_Position,\n    out float3 outputSurface : SV_Target0)\n{\n}\n"

// OpenGL default shaders.
#define DEFAULT_GL_COMP "#version 430\n#define LOCAL_SIZE_X 16\n#define LOCAL_SIZE_Y 16\nlayout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y) in;\n\nvoid main(void)\n{\n}\n"
#define DEFAULT_GL_TESE "\nlayout(triangles, equal_spacing, cw) in;\n\nvoid main(void)\n{\n\n}"
#define DEFAULT_GL_TESC "#version 430\nlayout(vertices = 3) out;\n\nvoid main()\n{\n}\n"
#define DEFAULT_GL_GEOM "#version 430\nlayout(points) in;\nlayout(points, max_vertices=1) out;\nvoid main()\n{\n}"
#define KA_STR_createdGLSFile "\nvoid main(void)\n{\n\n}"

// Vulkan default shaders.
#define KA_STR_createVK_VERT "#version 450\nvoid main()\n{\n}"
#define KA_STR_createVK_TESC "#version 450\nlayout(vertices = 3) out;\nvoid main()\n{\n\tgl_out[gl_InvocationID].gl_Position = gl_in[0].gl_Position;\n}"
#define KA_STR_createVK_TESE "#version 450\nlayout(quads, equal_spacing, ccw) in;\nvoid main()\n{\n}"
#define KA_STR_createVK_GEOM "#version 450\nlayout(points) in;\nlayout(points, max_vertices=1) out;\nvoid main()\n{\n}"
#define KA_STR_createVK_FRAG "#version 450\nvoid main()\n{\n}"
#define KA_STR_createVK_COMP "#version 450\nlayout (local_size_x = 1, local_size_y = 1) in;\nvoid main()\n{\n}"


#define KA_STR_createDefaultName "Kernel"
#define KA_STR_createDefaultExt ".cl"

// Build command issues
#define KA_STR_buildCommandInvalidTitle "Build Warning"
#define KA_STR_launchingCLIBuildWithCommand L"Launching CLI build with command line: %ls"
#define KA_STR_CLIPrefixForCompilerOption "--OpenCLoption="
#define KA_STR_RemovedOptionCreateLibrary "Warning: The OpenCL option \"-create-Library\" was omitted because it is currently unsupported with the current CPU device. Please remove this option or uncheck CPU device\n"

// HLSL semantic keywords
#define KA_STR_HLSLSemanticKeyWords "BINORMAL[*],BLENDINDICES[*],BLENDWEIGHT[*],COLOR[*],NORMAL[*],POSITION[*],\
            POSITIONT, PSIZE[*], TANGENT[*], Output, FOG, PSIZE, TESSFACTOR[*], TEXCOORD[*],\
            VFACE, VPOS, DEPTH[*], SV_ClipDistance[*], SV_CullDistance[*], SV_Coverage,\
            SV_Depth, SV_DispatchThreadID, SV_DomainLocation, SV_GroupID, SV_GroupIndex, SV_GroupThreadID, SV_GSInstanceID,\
            SV_InsideTessFactor, SV_IsFrontFace, SV_OutputControlPointID, SV_Position, SV_RenderTargetArrayIndex, SV_SampleIndex,\
SV_Target[*], SV_TessFactor, SV_ViewportArrayIndex, SV_PrimitiveID, SV_VertexID, packoffsetregister"

// Enhanced ISA view.
#define KA_STR_NA_VALUE "Varies"

// Build status.
#define KA_STR_BUILD_STARTED "========== Build started: Building %s for %d devices. ==========\n"
#define KA_STR_BUILD_COMPLETED_PREFIX "\n========== Build completed for %d devices: "
#define KA_STR_BUILD_COMPLETED_SUFFIX ". ========== \n"

#define KA_STR_BUILD_CANCELLED_BY_USER_PREFIX "\n========== Build cancelled: "
#define KA_STR_BUILD_CANCELLED_BY_USER_SUFFIX " devices skipped. ========== \n"
#define KA_STR_BUILD_CANCELLED_BY_USER_NO_SKIPPED "\n========== Build cancelled. ========== \n"


// HTML Strings:
#define KA_Str_HTML_CellFGColorBold "<td style='color:%1;font-weight:bold;'>%2</td>\n"
#define KA_Str_HTML_CellFGColor "<td style='color:%1;'>%2</td>\n"
#define KA_Str_HTML_CellFGColorNoWrap "<td style='color:%1;'><style='white-space:nowrap;'>&nbsp;%2</td>\n"
#define KA_Str_HTML_Cell "<td><style='white-space:nowrap;'>%1</td>\n"
#define KA_Str_HTML_CellWithTooltip "<td title='%1'><style='white-space:nowrap;'>%2</td>\n"
#define KA_Str_HTML_ColoredOpcode "<font color = %1>%2</font>"

// Export To CSV Title
#define KA_STR_exportToCSV "Export ISA to CSV..."
#define KA_STR_htmlExtension L"html"
#define KA_STR_cssExtension L"css"
#define KA_STR_ISAVIEW_CURSOR_cssText "table tr:hover{cursor:default;}"
#define KA_STR_ISATABLE_STYLE "td, th{ font-family:%1;font-size: smaller; overflow:hidden; padding-bottom:2px;} \
                               td, th{padding:2 2px 2 2px;} \
                               th{text-align:left; border-bottom: 1px solid Gray;} \
td{white-spaces:nowrap;}"

#define KA_STR_ISATABLE_OPENTAGS        "<html> \
                                            <body> \
                                                <header> \
                                                    <style>%1</style> \
                                                </header> \
                                                <table cellspacing=0 cellpadding=0 style='width:100%'> \
<tr>"

#define KA_STR_ISATABLE_CLOSETAGS              "</table> \
                                           </body> \
</html>"

#define KA_STR_ISATABLE_FRAME_STYLE "QFrame{ border: 1px solid  #868482; background: white; }"

#define KA_ISA_TABLE_HEXPOS_REGEX "0[xX][0-9a-fA-F]"
#define KA_ISA_TABLE_22DIGITSPOS_REGEX "\\d\\d:\\d\\d"
#define KA_ISA_TABLE_11DIGITSPOS_REGEX "\\d:\\d"
#define KA_ISA_TABLE_12DIGITSPOS_REGEX "\\d:\\d\\d"


#endif //__KASTRINGCONSTANTS_H


