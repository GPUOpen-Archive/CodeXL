//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcParseCmdLine.cpp $
/// \version $Revision: #2 $
/// \brief  This file contains the function to parse the command
///         line using Boost.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzerCLI/src/kcParseCmdLine.cpp#2 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <iostream>
#include <string>
#ifdef _WIN32
    #pragma warning (push, 3) // disable warning level 4 for boost
#endif
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>

#ifdef _WIN32
    #pragma warning (pop)
#endif

#include <AMDTKernelAnalyzerCLI/src/kcConfig.h>
#include <AMDTKernelAnalyzerCLI/src/kcParseCmdLine.h>
#include <VersionInfo/VersionInfo.h>

namespace po = boost::program_options;
using namespace std;

#if GSA_BUILD
    #define KERNEL_OPTION "function"
#else
    #define KERNEL_OPTION "kernel"
#endif

bool ParseCmdLine(int argc, char* argv[], Config& config)
{
    //----- Command Line Options Parsing
    bool doWork = true;

    try
    {
        // Parse command line options using Boost library

        // Generic Options: Valid got CL, DX, GL
        po::options_description genericOpt("Generic options");

        genericOpt.add_options()
        // The next two options here can be repeated, so they are vectors.
        ("csv-separator", po::value< string >(&config.m_CSVSeparator), "Override to default separator for analysis items.")
        ("list-asics,l", "List known ASIC targets. Supported device may vary according to the source you are trying to compile.\n"
         "By default, will show supported ASICS for OpenCL.\n"
         "use '-s HLSL -l' to view supported ASICS for HLSL")
        ("asic,c", po::value< vector<string> >(&config.m_ASICs), "Which ASIC to target.  Repeatable.")
        ("verbose", "List extra information.  (With: --list-asics).")
        ("version", "Print version string.")
        ("help,h", "Produce this help message.")
        ("analysis,a", po::value<string>(&config.m_AnalysisFile), "Path to output analysis file.")
        ("isa", po::value<string>(&config.m_ISAFile), "Path to output ISA disassembly file(s).")
        ("livereg", po::value<string>(&config.m_LiveRegisterAnalysisFile), "Path to live register analysis output file(s).")
        ("cfg", po::value<string>(&config.m_ControlFlowGraphFile), "Path to control flow graph output file(s).")
        ("il", po::value<string>(&config.m_ILFile), "Path to output IL file(s).")
        ("source-kind,s", po::value<string>(&config.m_SourceKind), "Source platform: cl for OpenCL, hlsl for DirectX, opengl for OpenGL and vulkan for Vulkan; cl is set by default")
        ;

        // DX Options
        po::options_description dxOpt("DirectX Shader Analyzer options");
        dxOpt.add_options()
        ("function,f",     po::value<string>(&config.m_Function), "D3D shader to compile, DX ASM shader.")

        ("DXFlags",        po::value<unsigned int>(&config.m_DXFlags),                   "Flags to pass to D3DCompile.")
        ("DXLocation",     po::value<string>(&config.m_DXLocation),                      "Location to the D3DCompiler Dll required for compilation. If none is specified, the default D3D compiler that is bundled with the Analyzer will be used.")
        ("FXC",            po::value<string>(&config.m_FXC),                             "FXC Command Line. Use full path and specify all arguments in \"\". For example:\n"
#ifdef AMDTBASETOOLS_STATIC
         "   GPUPerfStudioAnalyzer.exe  -f VsMain1 -s DXAsm -p vs_5_0 <Path>\\vsBlob.obj  --isa <Path>\\vsTest.isa --FXC \"<Path>\\fxc.exe /E VsMain1 /T vs_5_0 /Fo <Path>\\vsBlob.obj <Path>\\vsTest.fx\"\n "
         "   In order to use it, DXAsm must be specified. /Fo switch must be used and output file must be the same as input file for the GPUPerfStudioAnalyzer.")
#else
         "   CodeXLAnalyzer.exe  -f VsMain1 -s DXAsm -p vs_5_0 <Path>\\vsBlob.obj  --isa <Path>\\vsTest.isa --FXC \"<Path>\\fxc.exe /E VsMain1 /T vs_5_0 /Fo <Path>\\vsBlob.obj <Path>\\vsTest.fx\"\n "
         "   In order to use it, DXAsm must be specified. /Fo switch must be used and output file must be the same as input file for the CodeXLKernelAnalyzer.")
#endif
        ("DumpMSIntermediate", po::value<string>(&config.m_DumpMSIntermediate), "Location to save the MS Blob as text. ")
        ;

        po::options_description macroAndIncludeOpt("Macro and Include paths Options");
        macroAndIncludeOpt.add_options()
        // The next two options here can be repeated, so they are vectors.
        ("define,D", po::value< vector<string> >(&config.m_Defines), "Define symbol or symbol=value. Applicable only to CL and DX files. Repeatable.")
        ("IncludePath,I", po::value< vector<string> >(&config.m_IncludePath), "Additional include path required for compilation.  Repeatable.")
        ;

        // CL Option
#ifdef AMDTBASETOOLS_STATIC
        po::options_description clOpt("GPU PerfStudio Kernel Analyzer options");
#else
        po::options_description clOpt("CodeXL Kernel Analyzer options");
#endif
        clOpt.add_options()
        ("list-kernels",                                                       "List kernel functions.")
        ("debugil",      po::value<string>(&config.m_DebugILFile),             "Path to output Debug IL file(s).")
        ("metadata",     po::value<string>(&config.m_MetadataFile),            "Path to output Metadata file(s).\n"
         "Requires --" KERNEL_OPTION ".")
        ("kernel,k",     po::value<string>(&config.m_Function),                "Kernel to analyze or make IL or ISA.\n")
        ("binary,b",     po::value<string>(&config.m_BinaryOutputFile),        "Path to binary output file(s).")
        ("retain-user-filename",                                               "Retain the output path and name for the generated binary file as specified without adding the target asic name.")
        ("suppress",     po::value<vector<string> >(&config.m_SuppressSection), "Section to omit from binary output.  Repeatable. Available options: .source, .amdil, .debugil, .debug_info, .debug_abbrev, .debug_line, .debug_pubnames, .debug_pubtypes, .debug_loc, .debug_str, .llvmir, .text\nNote: Debug sections are valid only with \"-g\" compile option")
        ("OpenCLoption", po::value< vector<string> >(&config.m_OpenCLOptions), "OpenCL compiler options.  Repeatable.\n"
         "\tTo display a list of options use a command like:\n"
#ifdef AMDTBASETOOLS_STATIC
         "\tGPUPerfStudioAnalyzer k.cl --OpenCLoption=-h --asic Cypress")
#else
         "\tCodeXLKernelAnalyzer k.cl --OpenCLoption=-h --asic Cypress")
#endif
        ;

        po::options_description gldxOpt("");
        gldxOpt.add_options()
        ("profile,p", po::value<string>(&config.m_Profile), "Profile to use for compilation.  REQUIRED."
         "\nFor example: vs_5_0, ps_5_0, etc.");

        // Vulkan-specific.
        po::options_description pipelinedOpt("");
        pipelinedOpt.add_options()
        ("vert", po::value<string>(&config.m_VertexShader), "Full path to vertex shader source file.")
        ("tesc", po::value<string>(&config.m_TessControlShader), "Full path to tessellation control shader source file.")
        ("tese", po::value<string>(&config.m_TessEvaluationShader), "Full path to tessellation evaluation shader source file.")
        ("geom", po::value<string>(&config.m_GeometryShader), "Full path to geometry shader source file.")
        ("frag", po::value<string>(&config.m_FragmentShader), "Full path to fragment shader source file")
        ("comp", po::value<string>(&config.m_ComputeShader), "Full path to compute shader source file.");


        po::options_description hiddenOpt("Options that we don't show with --help.");
        hiddenOpt.add_options()
        ("?",                                                 "Produce help message.")
        ("input",     po::value<string>(&config.m_InputFile), "Source program for analysis.");

        // all options available from command line
        po::options_description allOpt;
        allOpt.add(genericOpt).add(macroAndIncludeOpt).add(clOpt).add(hiddenOpt).add(dxOpt).add(gldxOpt).add(pipelinedOpt);

        po::variables_map vm;

        po::positional_options_description positionalOpt;
        positionalOpt.add("input", -1);

        // Parse command line
        store(po::command_line_parser(argc, argv).
              options(allOpt).positional(positionalOpt).run(), vm);

        // Handle Options
        notify(vm);

        if (vm.count("help") || vm.count("?"))
        {
            config.m_RequestedCommand = Config::ccHelp;
            //doWork = false;
        }

        if (vm.count("list-asics"))
        {
            config.m_RequestedCommand = Config::ccListAsics;
        }

        // Verbose only affects list-ASICs right now.
        if (vm.count("verbose"))
        {
            config.m_bVerbose = true;
        }

        if (vm.count("retain-user-filename"))
        {
            config.m_isRetainUserBinaryPath = true;
        }

        if (vm.count("list-kernels"))
        {
            config.m_RequestedCommand = Config::ccListKernels;

        }
        else if (vm.count("version"))
        {
            config.m_RequestedCommand = Config::ccVersion;
        }

        else if (config.m_AnalysisFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }

        else if (config.m_ILFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_ISAFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_LiveRegisterAnalysisFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_BinaryOutputFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_DebugILFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_MetadataFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }

        // what is the right way to do it?
        // Danana- todo: need to decide how to know. not all commands requires file (like -l)
        bool bSourceSpecified = false;

        if (config.m_SourceKind.length() == 0) // set the default and remember it for the help
        {
            config.m_SourceKind = Config::sourceKindOpenCL;
            bSourceSpecified = true;
        }

        if (boost::iequals(config.m_SourceKind, Config::sourceKindHLSL))
        {
            config.m_SourceLanguage = SourceLanguage_HLSL;
        }
        else if (boost::iequals(config.m_SourceKind, Config::sourceKindDXAsm))
        {
            config.m_SourceLanguage = SourceLanguage_DXasm;
        }
        else if (boost::iequals(config.m_SourceKind, Config::sourceKindDXAsmT))
        {
            config.m_SourceLanguage = SourceLanguage_DXasmT;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindOpenCL)))
        {
            config.m_SourceLanguage = SourceLanguage_OpenCL;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindGLSL)))
        {
            config.m_SourceLanguage = SourceLanguage_GLSL;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindOpenGL)))
        {
            config.m_SourceLanguage = SourceLanguage_GLSL_OpenGL;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindVulkan)))
        {
            config.m_SourceLanguage = SourceLanguage_GLSL_Vulkan;
        }
        else
        {
            config.m_SourceLanguage = SourceLanguage_Invalid;
            cout << "Source language: " <<  config.m_SourceKind << " not supported.\n";

        }


        // Require an input file.
        if (doWork && (vm.count("input") == 0 && config.m_RequestedCommand == Config::ccInvalid))
        {
            // TODO: get just the program name here.
            // Maybe use boost file system stuff to just get KernelAnalyzerCLI.
            //           cout << "Usage: " << argv[0] << " [options] source_file" << endl;
            //           cout << visibleOpt << "\n";
            doWork = false;
        }

        // handle the help. I do it here because we need the visibleOpt.
        // TODO: change the function into class and make available.
        string programName(argv[0]);

        // On Linux platforms we use a script file that is what the user should call,
        // and a binary file that the script invokes that has a "-bin" suffix.
        // The binary file is the one that performs the commands so argv[0] equals the binary file name.
        // Force the help text to display the script file name by removing the "-bin" suffix
        // from sProgramName. This is applicable to Linux platforms only.
        string suffixToRemove("-bin");
        size_t pos = programName.rfind(suffixToRemove);

        if (pos != string::npos)
        {
            // Remove the suffix
            programName.erase(pos);
        }

        cout << std::endl;

        if ((config.m_RequestedCommand == Config::ccHelp) && (bSourceSpecified))
        {
            std::wstring versionString = STRPRODUCTVER;
            std::string productVersion(versionString.begin(), versionString.end());
            cout << programName << " version: " << productVersion << std::endl;
            cout << programName << " is an analysis tool for OpenCL";
#if _WIN32
            cout << ", DirectX";
#endif
            cout << ", OpenGL and Vulkan" << std::endl;;
            cout << "To view help for OpenCL: -h -s cl" << std::endl;
            cout << "To view help for OpenGL: -h -s opengl" << endl;
            cout << "To view help for Vulkan: -h -s vulkan" << endl;
#if _WIN32
            cout << "To view help for DirectX: -h -s hlsl" << std::endl;
#endif
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_OpenCL))
        {
            cout << "*** OpenCL Instructions & Options ***" << endl;
            cout << "=====================================" << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl;
            cout << genericOpt << endl;
            cout << macroAndIncludeOpt << endl;
            cout << clOpt << endl;
            cout << "Examples:" << endl;
            cout << "  " << "Extract ISA, IL code and statistics for all devices" << endl;
            cout << "    " << programName << " foo.cl --isa outdir/foo/myIsa.isa --il outdir/foo/myIl.il -a outdir/foo/stats.csv" << endl;
            cout << "  " << "Extract ISA and perform live register analysis for Fiji" << endl;
            cout << "    " << programName << " foo.cl -c Fiji --isa outdir/foo/myIsa.isa --livereg outdir/foo/" << endl;
            cout << "  " << "Create binary files output/foo-ASIC.bin for foo.cl and suppress the .source section" << endl;
            cout << "    " << programName << " foo.cl --bin outdir/foo --suppress .source" << endl;
            cout << "  " << "List the kernels available in foo.cl." << endl;
            cout << "    " << programName << " foo.cl --list-kernels" << endl;
            cout << "  " << "Produce analysis of myKernel in foo.cl.  Write the analysis to foo.csv." << endl;
            cout << "    " << programName << " foo.cl --kernel myKernel --analysis foo.csv" << endl;
            cout << "  " << "List the ASICs that the runtime supports." << endl;
            cout << "    " << programName << " --list-asics" << endl;
            cout << "  " << "Produce foo-Cypress.amdil and foo-Cypress.amdisa files for myKernel compiled for Cypress ASICs." << endl;
            cout << "    " << programName << " foo.cl --kernel myKernel --il foo --isa foo --asic Cypress" << endl;
            cout << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_HLSL))
        {
            cout << "*** DX Instructions & Options (Windows Only) ***" << endl;
            cout << "=================================" << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl;
            cout << genericOpt << endl;
            cout << macroAndIncludeOpt << endl;
            cout << dxOpt << endl;
            cout << gldxOpt << endl;
            cout << "Examples:" << endl;
            cout << " View supported ASICS for DirectX: " << programName << " -s hlsl " << "–" << "l" << endl;
            cout << " Extract the ISA: " << programName << " -s hlsl -f VsMain -p vs_5_0 --isa c:\\files\\myShader.isa c:\\files\\myShader.hlsl" << endl ;
            cout << " Extract the ISA and perform live register analysis: " << programName << " -s hlsl -f VsMain -p vs_5_0 --isa c:\\output\\myShader.isa --livereg c:\\output\\ c:\\files\\myShader.hlsl" << endl;
            cout << " Output analysis: " << programName << " -s hlsl -f VsMain -p vs_5_0  -a c:\\files\\myShader.csv c:\\files\\myShader.hlsl" << endl;
            cout << " Compile using DX Assembly in binary format: " << programName << " -f  VsMain -s DXAsm -p vs_5_0 c:\\files\\myShader.obj  --isa c:\\temp\\dxTest.isa" << endl;
            cout << " Compile using FXC: " << programName << " -s DXAsm -f  VsMain -p vs_5_0 c:\\files\\myShader.obj --isa c:\\files\\myIsa.isa c:\\files\\myShader.hlsl -c tahiti --FXC \"\"C:\\Program Files (x86)\\Windows Kits\\8.1\\bin\\x86\\fxc.exe\" /E VsMain /T vs_5_0  /Fo c:\\files\\myShader.obj c:\\files\\myShader.fx\"" << endl;
            cout << " Compile using DX Assembly in text format: " << programName << " -f  VsMain -s DXAsmTxt -p vs_5_0 c:\\files\\myShaderblob.txt  --isa c:\\temp\\dxTest.isa c:\\files\\myShader.hlsl" << endl;

        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_GLSL_Vulkan))
        {
            cout << "*** Vulkan Instructions & Options ***" << endl;
            cout << "=================================" << endl;
            cout << "Usage: " << programName << " [options]" << endl;
            cout << genericOpt << endl;
            cout << pipelinedOpt << endl;
            cout << "Examples:" << endl;
            cout << " Extract ISA, AMD IL and statistics for a Vulkan program that is comprised of a vertex shader and a fragment shader for all devices: " << programName << " -s vulkan --isa c:\\output\\vulkan_isa.isa --il c:\\output\\vulkan_il.amdil -a c:\\output\\vulkan_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA, AMD IL and statistics for a Vulkan program that is comprised of a vertex shader and a fragment shader for Iceland and Fiji: " << programName << " -s vulkan -c Iceland -c Fiji --isa c:\\output\\vulkan_isa.isa --il c:\\output\\vulkan_il.amdil -a c:\\output\\vulkan_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA and binaries for a Vulkan program that is comprised of a vertex shader and a fragment shader for all devices: " << programName << " -s vulkan --isa c:\\output\\vulkan_isa.isa -b c:\\output\\vulkan_bin.bin -a c:\\output\\vulkan_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA and perform live register analysis for a Vulkan program for all devices: " << programName << " -s vulkan --isa c:\\output\\vulkan_isa.isa --livereg c:\\output\\ --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_GLSL_OpenGL))
        {
            cout << "*** OpenGL Instructions & Options ***" << endl;
            cout << "=================================" << endl;
            cout << "Usage: " << programName << " [options]" << endl;
            cout << genericOpt << endl;
            cout << pipelinedOpt << endl;
            cout << "Examples:" << endl;
            cout << " Extract ISA, binaries and statistics for an OpenGL program that is comprised of a vertex shader and a fragment shader for all devices: " << programName << " -s opengl --isa c:\\output\\opengl_isa.isa -b c:\\output\\opengl_bin.bin -a c:\\output\\opengl_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA and statistics for an OpenGL program that is comprised of a vertex shader and a fragment shader for Tahiti: " << programName << " -s opengl -c Tahiti --isa c:\\output\\opengl_isa.isa -a c:\\output\\opengl_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA and perform live register analysis for an OpenGL program that is comprised of a vertex shader and a fragment shader for Tahiti: " << programName << " -s opengl -c Tahiti --isa c:\\output\\opengl_isa.isa --livereg c:\\output\\ --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
        }

    }
    catch (exception& e)
    {
        // Problem parsing options - report and exit
        std::string sExceptionMsg = e.what();

        // here I replace the blat error msg with a more informative one. BUG429870
        if (sExceptionMsg == "multiple occurrences")
        {
            sExceptionMsg = "Error: Problem parsing arguments. Please check if a non-repeatable argument was used more then once and that all file paths are correct. Note that if a path contain a space, a \"\" should be used.";
        }

        cout << sExceptionMsg << "\n";
        // TODO: Should be able to distinguish problem from --help/--version.
        //return false;
        doWork = false;
    }

    return doWork;
}

