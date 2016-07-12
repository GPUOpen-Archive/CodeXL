//------------------------------ kaCliLauncher.cpp ------------------------------

// C++.
#include <string>
#include <set>
#include <sstream>
#include <stdint.h>

// Infra.
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Framework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local.
#include <kaCliLauncher.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Definitions.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    #if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

        #if AMDT_BUILD_ACCESS == AMDT_PUBLIC_ACCESS
            const std::string ANALYZER_CLI_WIN_X86 = "CodeXLAnalyzer-d.exe";
            const std::string ANALYZER_CLI_WIN_X64 = "CodeXLAnalyzer-x64-d.exe";
        #elif AMDT_BUILD_ACCESS == AMDT_NDA_ACCESS
            const std::string ANALYZER_CLI_WIN_X86 = "CodeXLAnalyzer-d-nda.exe";
            const std::string ANALYZER_CLI_WIN_X64 = "CodeXLAnalyzer-x64-d-nda.exe";
        #elif AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
            const std::string ANALYZER_CLI_WIN_X86 = "CodeXLAnalyzer-d-internal.exe";
            const std::string ANALYZER_CLI_WIN_X64 = "CodeXLAnalyzer-x64-d-internal.exe";
        #else
            #error Unknown build access
        #endif

    #else

        #if AMDT_BUILD_ACCESS == AMDT_PUBLIC_ACCESS
            const std::string ANALYZER_CLI_WIN_X86 = "CodeXLAnalyzer.exe";
            const std::string ANALYZER_CLI_WIN_X64 = "CodeXLAnalyzer-x64.exe";
        #elif AMDT_BUILD_ACCESS == AMDT_NDA_ACCESS
            const std::string ANALYZER_CLI_WIN_X86 = "CodeXLAnalyzer-nda.exe";
            const std::string ANALYZER_CLI_WIN_X64 = "CodeXLAnalyzer-x64-nda.exe";
        #elif AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
            const std::string ANALYZER_CLI_WIN_X86 = "CodeXLAnalyzer-internal.exe";
            const std::string ANALYZER_CLI_WIN_X64 = "CodeXLAnalyzer-x64-internal.exe";
        #else
            #error Unknown build access
        #endif
    #endif // AMDT_DEBUG_BUILD
#else
    const std::string ANALYZER_CLI_LINUX_X64 = "CodeXLAnalyzer-bin";
#endif

const std::string KA_STR_CL_CLI_CMD_PREFIX = "-s cl";
const std::string KA_STR_VK_CLI_CMD_PREFIX = "-s vulkan";
const std::string KA_STR_GLShaderCommandLineOption = "-s opengl";
const std::string KA_STR_DXShaderCommandLineOption =  "-s hlsl";
const std::string KA_STR_D3D_FXC_ShaderCommandLineOption = "-s DXAsm";
const std::string KA_STR_DX_LOCATION = "--DXLocation";
const std::string KA_STR_FXC = "--FXC";

// **********************************************
// INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN


static std::string GenerateGLShaderOptions(const std::string& shaderType)
{
    std::stringstream outputStream;
    outputStream << KA_STR_GLShaderCommandLineOption << " " << "-p " << " " << shaderType << " ";
    return outputStream.str();
}

static std::string GenerateOptions(const BuildType buildType, kaPipelineShaders shadersPaths)
{
    std::stringstream outputStream;

    switch (buildType)
    {
        case VK_BUILD :
            outputStream << KA_STR_VK_CLI_CMD_PREFIX << " ";
            break;

        case GL_BUILD:
            outputStream << KA_STR_GLShaderCommandLineOption << " ";
            break;

        default:
            GT_ASSERT_EX(false, (L"Unknown Build Type given : " + to_wstring(buildType)).c_str());
            break;
    }


    if (!shadersPaths.m_vertexShader.isEmpty())
    {
        outputStream << " --vert " << "\"" <<  shadersPaths.m_vertexShader.asASCIICharArray() << "\"";
    }

    if (!shadersPaths.m_tessControlShader.isEmpty())
    {
        outputStream << " --tesc " << "\"" <<  shadersPaths.m_tessControlShader.asASCIICharArray() << "\"";
    }

    if (!shadersPaths.m_tessEvaluationShader.isEmpty())
    {
        outputStream << " --tese " << "\"" <<  shadersPaths.m_tessEvaluationShader.asASCIICharArray() << "\"";
    }

    if (!shadersPaths.m_geometryShader.isEmpty())
    {
        outputStream << " --geom " << "\"" <<  shadersPaths.m_geometryShader.asASCIICharArray() << "\"";
    }

    if (!shadersPaths.m_fragmentShader.isEmpty())
    {
        outputStream << " --frag " << "\"" <<  shadersPaths.m_fragmentShader.asASCIICharArray() << "\"";
    }

    if (!shadersPaths.m_computeShader.isEmpty())
    {
        outputStream << " --comp " << "\"" <<  shadersPaths.m_computeShader.asASCIICharArray() << "\"";
    }

    return outputStream.str();
}

#ifdef _WIN32

// Splits a given string to sub-elements according to the given delimiter.
static void split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    elems.clear();
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
}

// Splits the user-defined list of macros into pairs of key-value strings.
static bool ExtractDxCompilerDefines(const QString& userDefines,
                                     std::vector<std::pair<std::string, std::string>>& extractedDefines)
{
    bool ret = false;
    extractedDefines.clear();
    const char DELIMITER = ';';
    const char EQUALS_SYMBOL = '=';
    const std::string& definesStr = userDefines.toStdString();

    std::vector<std::string> defines;
    split(definesStr, DELIMITER, defines);

    for (size_t i = 0; i < defines.size(); ++i)
    {
        const std::string& currDefinition = defines[i];

        // Split the definition to key and value.
        size_t equalsSymbolIndex = currDefinition.find(EQUALS_SYMBOL);

        if (equalsSymbolIndex != std::string::npos)
        {
            // We have both key and value.
            std::string key = currDefinition.substr(0, equalsSymbolIndex);
            std::string value = currDefinition.substr(equalsSymbolIndex + 1);

            // Add the key value definition to our container.
            std::pair<std::string, std::string> pair(key, value);
            extractedDefines.push_back(pair);
        }
        else
        {
            // We only have a single definition (not a key-value pair).
            // Add the key value definition to our container.
            std::pair<std::string, std::string> pair(currDefinition, " ");
            extractedDefines.push_back(pair);
        }
    }

    ret = !extractedDefines.empty();
    return ret;
}

// Splits the user-defined list of include paths into a list of path strings.
static bool ExtractDxCompilerIncludes(const QString& userIncludes,
                                      std::vector<std::string>& extractedIncludes)
{
    bool ret = false;
    extractedIncludes.clear();
    const char DELIMITER = ';';
    const std::string& definesStr = userIncludes.toStdString();

    if (!userIncludes.contains(DELIMITER))
    {
        // Edge case: only a single include directory, without a delimiter.
        const QString& includeDir = userIncludes.trimmed();

        if (!includeDir.isEmpty())
        {
            extractedIncludes.push_back(includeDir.toStdString());
        }
    }
    else
    {
        std::vector<std::string> paths;
        split(definesStr, DELIMITER, paths);

        // Copy the paths.
        if (!paths.empty())
        {
            std::copy(paths.begin(), paths.end(), std::back_inserter(extractedIncludes));
        }
    }

    ret = !extractedIncludes.empty();
    return ret;
}

#endif

static std::string GenerateDeviceArgList(const std::set<std::string>& devices)
{
    std::stringstream outputStream;

    for (const std::string& device : devices)
    {
        outputStream << "-c " << "\"" << device << "\"" << " ";
    }

    return outputStream.str();
}


static std::string GenerateDeviceArg(const std::string& device)
{
    std::stringstream outputStream;
    outputStream << "-c " << "\"" << device << "\"" << " ";
    return outputStream.str();
}

static std::string GenerateKernelOptions()
{
    std::stringstream outputStream;
    outputStream << KA_STR_CL_CLI_CMD_PREFIX << " " << "--kernel all ";
    return outputStream.str();
}

static std::string GetCliExecutableName(AnalyzerBuildArchitecture& bitness)
{
    std::stringstream outputStream;
    const gtString OS_PATH_SEPRATOR(osFilePath::osPathSeparator);
    bool areWrappingQuotesRequired = false;

    // Get the binary directory.
    static gtString s_cxlBinDir;

    if (s_cxlBinDir.isEmpty())
    {
        osFilePath cxlBinPath(osFilePath::OS_CODEXL_BINARIES_PATH);
        s_cxlBinDir = cxlBinPath.fileDirectoryAsString();
    }

    // If the path contains space characters then wrapping quote characters are required at the beginning and the end of the complete Cli executable path
    if (s_cxlBinDir.find(L' ') != -1)
    {
        areWrappingQuotesRequired = true;
    }

    if (areWrappingQuotesRequired)
    {
        outputStream << "\"";
    }

    outputStream << s_cxlBinDir.asASCIICharArray() << OS_PATH_SEPRATOR.asASCIICharArray();

#if _WIN32

    if (bitness == kaBuildArch32_bit)
    {
        outputStream << ANALYZER_CLI_WIN_X86;
    }
    else
    {
        outputStream << ANALYZER_CLI_WIN_X64;
    }

#else
    GT_UNREFERENCED_PARAMETER(bitness);
    outputStream << ANALYZER_CLI_LINUX_X64;
#endif

    if (areWrappingQuotesRequired)
    {
        outputStream << "\"";
    }

    return outputStream.str();
}

// INTERNALLY-LINKED AUXILIARY FUNCTIONS - END
// **********************************************

//-----------------------------------------------------------------------------
/// \brief Name: GenerateCLICommandForDevice
/// \brief Description: Generates CLI Command according to configuration parameters
static std::string GenerateCLICommandForDevice(AnalyzerBuildArchitecture bitness,
                                               const std::string& buildType,
                                               const std::string& device,
                                               const std::string& buildOptions,
                                               const std::string& baseISAFilename,
                                               const std::string& baseILFilename,
                                               const std::string& baseAnalysFilename,
                                               const std::string& baseDxAsmFileName,
                                               const std::string& binaryFileName,
                                               const std::string& sourceCodeFullPathName)
{
    std::stringstream outputStream;

    // The executable name returned by the GetCliExecutableName() function is already wrapped in quotes if it contains spaces
    outputStream << GetCliExecutableName(bitness) << " ";
    outputStream << buildType << " " << GenerateDeviceArg(device) << " ";

    outputStream << buildOptions << " ";

    if (!baseISAFilename.empty())
    {
        outputStream << "--isa " << "\"" << baseISAFilename << "\"" << " ";
    }

    if (!baseILFilename.empty())
    {
        outputStream << "--il " << "\"" << baseILFilename << "\"" << " ";
    }

    if (!baseAnalysFilename.empty())
    {
        outputStream << "-a " << "\"" << baseAnalysFilename << "\"" << " ";
    }

    if (!binaryFileName.empty())
    {
        outputStream << "-b " << "\"" << binaryFileName << "\"" << " ";
    }

    if (!baseDxAsmFileName.empty())
    {
        outputStream << "--DumpMSIntermediate " << "\"" << baseDxAsmFileName << "\"" << " ";
    }

    if (!sourceCodeFullPathName.empty())
    {
        outputStream << "\"" << sourceCodeFullPathName << "\"";
    }

    return outputStream.str();
}

//-----------------------------------------------------------------------------
/// \brief Name: GenerateVulkanCLICommandForDevice
/// \brief Description: Generates Vulkan CLI Command according to configuration parameters
static std::string GenerateCLICommandForDevice(AnalyzerBuildArchitecture bitness,
                                               const std::string& vulkanBuildOptions,
                                               const std::string& device,
                                               const std::string& baseISAFilename,
                                               const std::string& baseILFilename,
                                               const std::string& baseStatisticsFilename,
                                               const std::string& binaryFileName)

{
    std::stringstream outputStream;

    // The executable name returned by the GetCliExecutableName() function is already wrapped in quotes if it contains spaces
    outputStream << GetCliExecutableName(bitness) << " ";
    outputStream << vulkanBuildOptions << " " << GenerateDeviceArg(device) << " ";

    if (!baseISAFilename.empty())
    {
        outputStream << "--isa " << "\"" << baseISAFilename << "\"" << " ";
    }

    if (!baseILFilename.empty())
    {
        outputStream << "--il " << "\"" << baseILFilename << "\"" << " ";
    }

    if (!baseStatisticsFilename.empty())
    {
        outputStream << "-a " << "\"" << baseStatisticsFilename << "\"" << " ";
    }

    if (!binaryFileName.empty())
    {
        outputStream << " -b " << "\"" << binaryFileName << "\"" << " ";
    }

    return outputStream.str();
}


static std::string GenerateCLICommandForMultipleDevices(AnalyzerBuildArchitecture bitness,
                                                        const std::string& buildType,
                                                        const std::set<std::string>& deviceList,
                                                        const std::string& baseISAFilename,
                                                        const std::string& baseILFilename,
                                                        const std::string& baseAnalysFilename, const std::string& baseDxAsmFileName,
                                                        const std::string& binaryFileName,
                                                        const std::string& sourceCodeFullPathName)
{
    std::stringstream outputStream;

    // The executable name returned by the GetCliExecutableName() function is already wrapped in quotes if it contains spaces
    outputStream << GetCliExecutableName(bitness) << " ";
    outputStream << buildType << " " << GenerateDeviceArgList(deviceList) << " ";

    if (!baseISAFilename.empty())
    {
        outputStream << "--isa " << "\"" << baseISAFilename << "\"" << " ";
    }

    if (!baseILFilename.empty())
    {
        outputStream << "--il " << "\"" << baseILFilename << "\"" << " ";
    }

    if (!baseAnalysFilename.empty())
    {
        outputStream << "-a " << "\"" << baseAnalysFilename << "\"" << " ";
    }

    if (!baseDxAsmFileName.empty())
    {
        outputStream << "--DumpMSIntermediate " << "\"" << baseDxAsmFileName << "\"" << " ";
    }

    if (!binaryFileName.empty())
    {
        outputStream << "-b " << "\"" << binaryFileName << "\"" << " ";
    }

    outputStream << "\"" << sourceCodeFullPathName << "\"";

    return outputStream.str();
}


static bool ExecAndGrabOutput(const char* cmd, const bool& shouldBeCancelled, std::string& cmdOutput)
{
    bool ret = true;
    bool isBuildCancelled = kaBackendManager::instance().IsBuildCancelled();

    if (!isBuildCancelled)
    {
        gtString cmdOutputAsGtStr;
        ret = osExecAndGrabOutput(cmd, shouldBeCancelled, cmdOutputAsGtStr);

        if (ret && !cmdOutputAsGtStr.isEmpty())
        {
            cmdOutput = cmdOutputAsGtStr.asASCIICharArray();
        }
    }

    return ret;
}


//-----------------------------------------------------------------------------
bool LaunchOpenGLSession(AnalyzerBuildArchitecture bitness, const std::string& shaderType,
                         const std::string& baseISAFilename, const std::string& baseILFilename,
                         const std::string& baseAnalysFilename, const std::set<std::string>& SelectedDevices,
                         const std::string& sourceCodeFullPathName, const bool& shouldBeCancelled, std::string& cliOutput)
{
    bool retVal = false;
    std::string GLShaderOptions = GenerateGLShaderOptions(shaderType);

    // Generate the CLI command.
    std::string commandLine = GenerateCLICommandForMultipleDevices(bitness, GLShaderOptions, SelectedDevices, baseISAFilename,
                                                                   baseILFilename, baseAnalysFilename, "", "", sourceCodeFullPathName);

    // Execute the CLI.
    ExecAndGrabOutput(commandLine.c_str(), shouldBeCancelled, cliOutput);

    retVal = !cliOutput.empty();
    return retVal;
}

//-----------------------------------------------------------------------------
bool LaunchOpenGLSessionForDevice(AnalyzerBuildArchitecture bitness, const std::string& shaderType,
                                  const std::string& baseISAFilename, const std::string& baseILFilename,
                                  const std::string& baseAnalysFilename, const std::string& SelectedDevice,
                                  const std::string& sourceCodeFullPathName, const bool& shouldBeCancelled, std::string& cliOutput)
{
    bool retVal = false;
    std::string GLShaderOptions = GenerateGLShaderOptions(shaderType);

    // Build options are always empty for OpenGL build
    std::string buildOptions;

    // Generate the CLI command.
    std::string commandLine = GenerateCLICommandForDevice(bitness, GLShaderOptions, SelectedDevice, buildOptions, baseISAFilename,
                                                          baseILFilename, baseAnalysFilename, "", "", sourceCodeFullPathName);

    // Execute the CLI.
    ExecAndGrabOutput(commandLine.c_str(), shouldBeCancelled, cliOutput);

    retVal = !cliOutput.empty();
    return retVal;
}

#if _WIN32

static std::string GenerateDXShaderOptions(const std::string& dxTarget, const std::string& dxEntryPoint, bool isFxc, bool isIntrinsicsEnabled)
{
    std::stringstream outputStream;

    if (!isFxc)
    {
        outputStream << KA_STR_DXShaderCommandLineOption << " " << "-p" << " " << dxTarget << " " << "-f" << " " << dxEntryPoint;
    }
    else
    {
        outputStream << KA_STR_D3D_FXC_ShaderCommandLineOption << " " << "-p " << dxTarget << " " << "-f " << dxEntryPoint << " ";
    }

    if (isIntrinsicsEnabled)
    {
        outputStream << " --intrinsics ";
    }

    return outputStream.str();
}

static std::string GenerateDXBuilderOptions(const std::string& buildOptions, const DXAdditionalBuildOptions& additionalBuildOptions)
{
    std::stringstream outputStream;

    if (!buildOptions.empty())
    {
        std::vector<std::string> buildOptionsElems;
        split(buildOptions, ' ', buildOptionsElems);

        for (const std::string& buildOptElem : buildOptionsElems)
        {
            outputStream << "-D " << buildOptElem << " ";
        }
    }

    if (!additionalBuildOptions.m_builderPath.isEmpty())
    {
        outputStream << " " << KA_STR_DX_LOCATION << " \"" << additionalBuildOptions.m_builderPath.toStdString() << "\" ";
    }

    // Handle custom include paths.
    std::vector<std::string> includePaths;
    bool isInlucdePathsHandled = ExtractDxCompilerIncludes(additionalBuildOptions.m_additionalIncludes, includePaths);

    if (isInlucdePathsHandled)
    {
        for (const std::string& incPath : includePaths)
        {
            outputStream << "-I \"" << incPath << "\" ";
        }
    }

    // Handle user-defined macro definitions.
    std::vector<std::pair<std::string, std::string>> userMacros;
    bool isUserMacrosHandled = ExtractDxCompilerDefines(additionalBuildOptions.m_additionalMacros, userMacros);

    if (isUserMacrosHandled)
    {
        for (const auto& definedMacro : userMacros)
        {
            outputStream << "-D " << definedMacro.first;

            if (!definedMacro.second.empty())
            {
                outputStream << "=" << definedMacro.second << " ";
            }
        }
    }

    if (additionalBuildOptions.m_buildOptionsMask != 0)
    {
        outputStream << "--DXFlags " << additionalBuildOptions.m_buildOptionsMask << " ";
    }

    return outputStream.str();
}

//-----------------------------------------------------------------------------
bool LaunchDXSessionForDevice(AnalyzerBuildArchitecture& bitness,
                              const::string& dxTarget,
                              const std::string& dxEntryPoint,
                              const std::string& buildOptions,
                              const DXAdditionalBuildOptions& additionalBuildOptions,
                              const std::string& baseISAFilename,
                              const std::string& baseBinFName,
                              const std::string& baseILFilename,
                              const std::string& baseAnalysFilename,
                              const std::string& dxAsmFileName,
                              const std::string& selectedDevice,
                              const std::string& sourceCodeFullPathName,
                              bool isIntrinsicsEnabled,
                              const bool& shouldBeCanceled,
                              std::string& cliOutput)
{
    GT_UNREFERENCED_PARAMETER(baseILFilename);
    bool retVal = true;

    std::stringstream dxOptionsOutputStream;

    // Check if the compilation should go through FXC.
    bool isFxc = (additionalBuildOptions.m_builderType == KA_STR_DX_FXC_BUILD_TYPE);

    dxOptionsOutputStream << GenerateDXShaderOptions(dxTarget, dxEntryPoint, isFxc, isIntrinsicsEnabled);
    std::string compilerBuildOptions;

    if (!isFxc)
    {
        compilerBuildOptions = GenerateDXBuilderOptions(buildOptions, additionalBuildOptions);
    }
    else
    {
        std::string fixedDxAsmFile;

        // Fix the DX ASM file name.
        if (!dxAsmFileName.empty())
        {
            // Convert the original name to a structure that we can work with.
            gtString fileNameAsGtstr;
            fileNameAsGtstr << dxAsmFileName.c_str();
            osFilePath dxAsmFilePath(fileNameAsGtstr);
            gtString originalFileName;
            dxAsmFilePath.getFileName(originalFileName);

            if (!originalFileName.isEmpty())
            {
                // Disassemble the file name, and reconstruct it, this time
                // including the entry point.
                std::vector<std::string> fileNameElems;
                split(originalFileName.asASCIICharArray(), '_', fileNameElems);

                if (fileNameElems.size() > 1)
                {
                    std::stringstream tmpStream;
                    tmpStream << fileNameElems[0] << "_" << dxEntryPoint;

                    for (size_t i = 1; i < fileNameElems.size(); ++i)
                    {
                        tmpStream << "_" << fileNameElems[i];
                    }

                    gtString fixedFilesNameAsGtStr;
                    fixedFilesNameAsGtStr << tmpStream.str().c_str();
                    dxAsmFilePath.setFileName(fixedFilesNameAsGtStr);
                    fixedDxAsmFile = dxAsmFilePath.asString().asASCIICharArray();
                }
            }

        }

        // Wrap FXC's path arguments with this string to make sure that the command
        // is being processed successfully by both Analyzer CLI and FXC.
        const char* PATH_WRAPPER = "\\\"";

        // Get the object file path.
        gtString isaFullPath;
        isaFullPath << baseISAFilename.c_str();
        osFilePath isaOutputDirFullPath;
        isaOutputDirFullPath.setFullPathFromString(isaFullPath);

        // Generate the obj file name.
        std::stringstream objFileName;
        objFileName << isaOutputDirFullPath.fileDirectoryAsString().asASCIICharArray() << "\\";
        objFileName << selectedDevice << "_obj.obj";

        // Add the name of the obj file that is expected to be generated by the FXC.
        dxOptionsOutputStream << "\"" << objFileName.str() << "\"";

        // Build the command string for FXC invocation.
        dxOptionsOutputStream << " --FXC \"\\";
        dxOptionsOutputStream << "\"" << additionalBuildOptions.m_builderPath.toStdString().c_str() << "\\\" ";
        dxOptionsOutputStream << additionalBuildOptions.m_buildOptions.toStdString().c_str() << " ";

        // Handle custom include paths.
        std::vector<std::string> includePaths;
        bool isInlucdePathsHandled = ExtractDxCompilerIncludes(additionalBuildOptions.m_additionalIncludes, includePaths);

        if (isInlucdePathsHandled)
        {
            for (const std::string& incPath : includePaths)
            {
                dxOptionsOutputStream << "/I " << PATH_WRAPPER << incPath << PATH_WRAPPER << " ";
            }
        }

        // Handle user-defined macro definitions.
        std::vector<std::pair<std::string, std::string>> userMacros;
        bool isUserMacrosHandled = ExtractDxCompilerDefines(additionalBuildOptions.m_additionalMacros, userMacros);

        if (isUserMacrosHandled)
        {
            for (const auto& definedMacro : userMacros)
            {
                dxOptionsOutputStream << "/D " << definedMacro.first << "=" << definedMacro.second << " ";
            }
        }

        dxOptionsOutputStream << "/T " << dxTarget << " /E " << dxEntryPoint << " /Fc " << PATH_WRAPPER << fixedDxAsmFile << PATH_WRAPPER << " /Fo " << PATH_WRAPPER <<
                              objFileName.str() << PATH_WRAPPER << " " << PATH_WRAPPER << sourceCodeFullPathName << PATH_WRAPPER << "\"";
    }

    // Generate the CLI command.
    std::string commandLine = GenerateCLICommandForDevice(bitness,
                                                          dxOptionsOutputStream.str(),
                                                          selectedDevice,
                                                          compilerBuildOptions,
                                                          baseISAFilename,
                                                          baseILFilename,
                                                          baseAnalysFilename,
                                                          ((!isFxc) ? dxAsmFileName : ""),
                                                          baseBinFName,
                                                          ((!isFxc) ? sourceCodeFullPathName : ""));

    // Execute the CLI.
    ExecAndGrabOutput(commandLine.c_str(), shouldBeCanceled, cliOutput);
    retVal = retVal && !cliOutput.empty();
    return retVal;
}
#endif

bool LaunchOpenCLSession(AnalyzerBuildArchitecture bitness,
                         const std::string& baseISAFilename,
                         const std::string& baseILFilename,
                         const std::string& baseAnalysFilename,
                         const std::string& binaryFilename,
                         const std::set<std::string>& SelectedDevices,
                         const std::string& sourceCodeFullPathName, const bool& shouldBeCancelled,
                         std::string& cliOutput)
{
    bool retVal = false;

    // Generate the kernel options.
    std::string kernelOptions = GenerateKernelOptions();

    // Generate the CLI command.
    std::string commandLine = GenerateCLICommandForMultipleDevices(bitness, kernelOptions, SelectedDevices, baseISAFilename,
                                                                   baseILFilename, baseAnalysFilename, "", binaryFilename, sourceCodeFullPathName);

    // Execute the CLI.
    ExecAndGrabOutput(commandLine.c_str(), shouldBeCancelled, cliOutput);
    retVal = !cliOutput.empty();

    return retVal;
}


//-----------------------------------------------------------------------------
bool LaunchOpenCLSessionForDevice(AnalyzerBuildArchitecture bitness,
                                  const std::string& baseISAFilename,
                                  const std::string& baseILFilename,
                                  const std::string& baseAnalysFilename,
                                  const std::string& binaryFilename,
                                  const std::string& Device,
                                  const std::string& sourceCodeFullPathName,
                                  const std::string& buildOptions,
                                  const bool& shouldBeCancelled,
                                  std::string& cliOutput)
{
    bool retVal = false;
    // Generate the kernel options.
    std::string kernelOptions = GenerateKernelOptions();

    // Generate the CLI command.
    std::string commandLine = GenerateCLICommandForDevice(bitness, kernelOptions, Device, buildOptions, baseISAFilename,
                                                          baseILFilename, baseAnalysFilename, "", binaryFilename, sourceCodeFullPathName);

    // Execute the CLI.
    ExecAndGrabOutput(commandLine.c_str(), shouldBeCancelled, cliOutput);
    retVal = !cliOutput.empty();

    return retVal;
}


bool LaunchVulkanSession(AnalyzerBuildArchitecture bitness, const kaPipelineShaders& inputShaders, const std::string& baseISAFilename,
                         const std::string& baseILFilename, const std::string& baseAnalysFilename, const std::string& binaryFilename,
                         const std::set<std::string>& selectedDevices, const bool& shouldCancel, std::string& cliOutput)
{
    // TODO: implement this function.

    GT_UNREFERENCED_PARAMETER(bitness);
    GT_UNREFERENCED_PARAMETER(inputShaders);
    GT_UNREFERENCED_PARAMETER(baseISAFilename);
    GT_UNREFERENCED_PARAMETER(baseILFilename);
    GT_UNREFERENCED_PARAMETER(baseAnalysFilename);
    GT_UNREFERENCED_PARAMETER(binaryFilename);
    GT_UNREFERENCED_PARAMETER(selectedDevices);
    GT_UNREFERENCED_PARAMETER(shouldCancel);
    GT_UNREFERENCED_PARAMETER(cliOutput);
    return false;
}

bool LaunchRenderSessionForDevice(const BuildType buildType, AnalyzerBuildArchitecture bitness, const kaPipelineShaders& inputShaders,
                                  const std::string& baseISAFilename, const std::string& baseILFilename,
                                  const std::string& baseStatisticsFilename, const std::string& binaryFilename,
                                  const std::string& device, const bool& shouldCancel, std::string& cliOutput)
{
    bool retVal = false;

    std::string buildOptions = GenerateOptions(buildType, inputShaders);

    // Generate the CLI command.
    std::string commandLine = GenerateCLICommandForDevice(bitness, buildOptions,
                                                          device, baseISAFilename,
                                                          baseILFilename, baseStatisticsFilename,
                                                          binaryFilename);
    // Execute the CLI.
    ExecAndGrabOutput(commandLine.c_str(), shouldCancel, cliOutput);

    retVal = !cliOutput.empty();
    return retVal;
}



