// -*- C++ -*-
//=====================================================================
// Copyright 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcConfig.h $
/// \version $Revision: #2 $
/// \brief  Config options to the KernelAnalyzer backend.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcConfig.h#2 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>
#include <vector>
#include <AMDTBackEnd/Include/beBackend.h>

/// A place to collect command line options.
/// This is a POD with a dump method.
class Config
{
public:

    static std::string sourceKindHLSL;
    static std::string sourceKindDXAsm;
    static std::string sourceKindDXAsmT;
    static std::string sourceKindOpenCL;
    static std::string sourceKindGLSL;
    static std::string sourceKindOpenGL;
    static std::string sourceKindVulkan;

    enum ConfigCommand
    {
        ccInvalid = 0,
        ccCompile,
        ccListKernels,
        ccHelp,
        ccListAsics,
        ccVersion,
    };

    Config();
    void dump(std::ostream&) const;              ///< Method for debugging and maybe testing.

    beKA::SourceLanguage    m_SourceLanguage;    ///<What language we want to work on. currently we support cl/hlsl
    ConfigCommand           m_RequestedCommand;  ///<What the user requested to do
    std::string              m_InputFile;        ///< Source file for processing.
    std::string              m_AnalysisFile;     ///< Output analysis file.
    std::string              m_ILFile;           ///< Output IL Text file template.
    std::string              m_ISAFile;          ///< Output ISA Text file template.
    std::string              m_LiveRegisterAnalysisFile;///< Live register analysis output file.
    std::string              m_ControlFlowGraphFile;///< Control flow graph output file.
    std::string              m_BinaryOutputFile; ///< Output binary file template.
    std::string              m_Function;         ///< Kernel/Function of interest in analysis.
    std::string              m_CSVSeparator;     ///< Override for CSV list separator.
    std::string              m_DebugILFile;      ///< Output .debugil Text file template.
    std::string              m_MetadataFile;     ///< Output .metadata Text file template.
    std::vector<std::string> m_ASICs;            ///< Target GPUs for compilation.
    std::vector<std::string> m_SuppressSection;  ///< List of sections to suppress in generated binary files.
    std::vector<std::string> m_OpenCLOptions;    ///< Options to be added to OpenCL compile.
    bool                     m_bVerbose;         ///< Be verbose.  (For now only adds to ListASICs.)
    std::vector<std::string> m_Defines;          ///< Macros to be added to compile.
    std::vector<std::string> m_IncludePath;       ///< Additional Include paths
    bool                     m_isRetainUserBinaryPath; ///< If true then CLI will not add the asic name to the generated binary output file

    // DX/GL
    std::string              m_SourceKind;       ///< Kind of source HLSL or GLSL (maybe more later like ASM kinds).
    std::string              m_Profile;          ///< Profile used with GSA compilations. Target in DX
    unsigned int             m_DXFlags;          ///< Flags to pass to D3DCompile.
    std::string              m_DXLocation;       ///< D3DCompiler dll location
    std::string              m_FXC;               ///< FXC path and arguments
    std::string              m_DumpMSIntermediate; /// the location where to save the ms blob as text
    bool                     m_EnableShaderIntrinsics; /// true to enable DX shader intrinsics


    // Vulkan.
    std::string              m_programOutputDir;     ///< Output directory for the compiler.
    std::string              m_VertexShader;         ///< Vertex shader full path
    std::string              m_TessControlShader;    ///< Tessellation control shader full path
    std::string              m_TessEvaluationShader; ///< Tessellation evaluation shader full path
    std::string              m_GeometryShader;       ///< Geometry shader full path
    std::string              m_FragmentShader;       ///< Fragment shader full path
    std::string              m_ComputeShader;        ///< Compute shader full path

    bool                     m_isSpirvBinariesRequired;          ///< True to generate SPIR-V binaries
    bool                     m_isAmdPalIlBinariesRequired;       ///< True to generate AMD PAL IL binaries
    bool                     m_isAmdPalIlDisassemblyRequired;    ///< True to generate AMD PAL IL disassembly
    bool                     m_isAmdIsaBinariesRequired;         ///< True to generate AMD ISA binaries
    bool                     m_isAmdIsaDisassemblyRequired;      ///<True to generate AMD ISA binaries
    bool                     m_isScStatsRequired;                ///< True to generate shader compiler statistics

private:
    // Disable copy
    Config(const Config&);
    // Disable assign
    Config& operator= (const Config&);

};

std::ostream& operator<<(std::ostream& ostr, const Config& config);

#endif
