// C++.
#include <iterator>

// Backend.
#include <DeviceInfo.h>
#include <AMDTBackEnd/Include/beBackend.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcCLICommanderOpenGL.h>
#include <AMDTKernelAnalyzerCLI/src/kcCliStringConstants.h>
#include <AMDTKernelAnalyzerCLI/src/kcOpenGLStatisticsParser.h>
#include <AMDTKernelAnalyzerCLI/src/kcUtils.h>

struct kcCLICommanderOpenGL::OpenGLDeviceInfo
{
    OpenGLDeviceInfo() : m_deviceFamilyId(0), m_deviceId(0) {}

    OpenGLDeviceInfo(size_t chipFamily, size_t chipRevision) :
        m_deviceFamilyId(chipFamily), m_deviceId(chipRevision) {}

    static bool HwGenToFamilyId(GDT_HW_GENERATION hwGen, size_t& familyId)
    {
        bool ret = true;
        familyId = 0;

        switch (hwGen)
        {
            case GDT_HW_GENERATION_SOUTHERNISLAND:
                familyId = 110;
                break;

            case GDT_HW_GENERATION_SEAISLAND:
                familyId = 120;
                break;

            case GDT_HW_GENERATION_VOLCANICISLAND:
                familyId = 130;
                break;

            case GDT_HW_GENERATION_NONE:
            case GDT_HW_GENERATION_NVIDIA:
            case GDT_HW_GENERATION_LAST:
            default:
                ret = false;
                GT_ASSERT_EX(false, L"Unsupported HW generation.");
                break;
        }

        return ret;
    }

    // HW family id.
    size_t m_deviceFamilyId;

    // Chip id
    size_t m_deviceId;
};


kcCLICommanderOpenGL::kcCLICommanderOpenGL() : m_pOglBuilder(new beProgramBuilderOpenGL)
{
}


kcCLICommanderOpenGL::~kcCLICommanderOpenGL()
{
    delete m_pOglBuilder;
}

bool kcCLICommanderOpenGL::GetSupportedDevices()
{
    if (m_pOglBuilder != nullptr)
    {
        std::vector<GDT_GfxCardInfo> availableDevices;

        if (m_supportedDevicesCache.empty())
        {
            beKA::beStatus status = m_pOglBuilder->GetDeviceTable(availableDevices);

            if (status == beKA::beStatus_SUCCESS && !availableDevices.empty())
            {
                // Filter out the duplicates.
                for (const GDT_GfxCardInfo& device : availableDevices)
                {
                    if (device.m_szCALName != nullptr && strlen(device.m_szCALName) > 1)
                    {
                        // Cache device name.
                        std::string deviceName = device.m_szCALName;
                        m_supportedDevicesCache.insert(deviceName);

                        // Cache device info if needed.
                        OpenGLDeviceInfo deviceInfo;

                        // Fetch the family and revision IDs from the backend.
                        bool isSupportedDevice = m_pOglBuilder->GetDeviceGLInfo(deviceName, deviceInfo.m_deviceFamilyId, deviceInfo.m_deviceId);

                        if (isSupportedDevice && (m_deviceInfo.find(deviceName) == m_deviceInfo.end()))
                        {
                            m_deviceInfo[deviceName] = deviceInfo;
                        }
                    }
                }
            }
        }
    }

    return (!m_supportedDevicesCache.empty() && !m_deviceInfo.empty());
}


void kcCLICommanderOpenGL::ListAsics(Config& config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);

    // Output message.
    std::stringstream logMsg;

    // Todo: handle the verbose part.
    if (m_supportedDevicesCache.empty())
    {
        if (m_pOglBuilder != nullptr)
        {
            bool isDeviceListExtracted = GetSupportedDevices();

            if (!isDeviceListExtracted)
            {
                logMsg << STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST << std::endl;
            }
        }
    }

    // Print the list of unique device names.
    for (const std::string& device : m_supportedDevicesCache)
    {
        logMsg << device << std::endl;
    }

    // Print the output messages.
    if (callback != nullptr)
    {
        callback(logMsg.str());
    }

}

void kcCLICommanderOpenGL::Version(Config& config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);

    if (m_pOglBuilder != nullptr)
    {
        gtString glVersion;
        bool rc = m_pOglBuilder->GetOpenGLVersion(glVersion);

        if (rc && !glVersion.isEmpty())
        {
            callback(glVersion.asASCIICharArray());
        }
        else
        {
            std::stringstream logMsg;
            logMsg << STR_ERR_CANNOT_EXTRACT_OPENGL_VERSION << std::endl;
            callback(logMsg.str());
        }
    }
}

// Helper function to remove unnecessary file paths.
static void GenerateRenderingPipelineOutputPaths(const Config& config, const std::string& baseOutputFileName, const std::string& device, beProgramPipeline& pipelineToAdjust)
{
    // Generate the output file paths.
    kcUtils::AdjustRenderingPipelineOutputFileNames(baseOutputFileName, device, pipelineToAdjust);

    // Clear irrelevant paths.
    bool isVertexShaderPresent = (!config.m_VertexShader.empty());
    bool isTessControlShaderPresent = (!config.m_TessControlShader.empty());
    bool isTessEvaluationShaderPresent = (!config.m_TessEvaluationShader.empty());
    bool isGeometryexShaderPresent = (!config.m_GeometryShader.empty());
    bool isFragmentShaderPresent = (!config.m_FragmentShader.empty());
    bool isComputeShaderPresent = (!config.m_ComputeShader.empty());

    if (!isVertexShaderPresent)
    {
        pipelineToAdjust.m_vertexShader.makeEmpty();
    }

    if (!isTessControlShaderPresent)
    {
        pipelineToAdjust.m_tessControlShader.makeEmpty();
    }

    if (!isTessEvaluationShaderPresent)
    {
        pipelineToAdjust.m_tessEvaluationShader.makeEmpty();
    }

    if (!isGeometryexShaderPresent)
    {
        pipelineToAdjust.m_geometryShader.makeEmpty();
    }

    if (!isFragmentShaderPresent)
    {
        pipelineToAdjust.m_fragmentShader.makeEmpty();
    }

    if (!isComputeShaderPresent)
    {
        pipelineToAdjust.m_computeShader.makeEmpty();
    }
}

void kcCLICommanderOpenGL::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    // Output stream.
    std::stringstream logMsg;

    // Input validation.
    bool shouldAbort = false;
    bool isVertexShaderPresent = (!config.m_VertexShader.empty());
    bool isTessControlShaderPresent = (!config.m_TessControlShader.empty());
    bool isTessEvaluationShaderPresent = (!config.m_TessEvaluationShader.empty());
    bool isGeometryexShaderPresent = (!config.m_GeometryShader.empty());
    bool isFragmentShaderPresent = (!config.m_FragmentShader.empty());
    bool isComputeShaderPresent = (!config.m_ComputeShader.empty());
    bool isIsaRequired = (!config.m_ISAFile.empty());
    bool isLiveRegAnalysisRequired = (!config.m_LiveRegisterAnalysisFile.empty());
    bool isCfgRequired = (!config.m_ControlFlowGraphFile.empty());
    bool isIsaBinary = (!config.m_BinaryOutputFile.empty());
    bool isStatisticsRequired = (!config.m_AnalysisFile.empty());

    // Fatal error. This should not happen unless we have an allocation problem.
    if (m_pOglBuilder == nullptr)
    {
        shouldAbort = true;
        logMsg << STR_ERR_MEMORY_ALLOC_FAILURE << std::endl;
    }

    // Cannot mix compute and non-compute shaders.
    if (isComputeShaderPresent && (isVertexShaderPresent || isTessControlShaderPresent ||
                                   isTessEvaluationShaderPresent || isGeometryexShaderPresent || isFragmentShaderPresent))
    {
        logMsg << STR_ERR_RENDER_COMPUTE_MIX << std::endl;
        shouldAbort = true;
    }

    // Options to be passed to the backend.
    OpenGLOptions glOptions;

    // Validate the input shaders.
    if (!shouldAbort && isVertexShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_VERTEX_SHADER, config.m_VertexShader, logMsg);
        glOptions.m_pipelineShaders.m_vertexShader << config.m_VertexShader.c_str();
    }

    if (!shouldAbort && isTessControlShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_TESS_CTRL_SHADER, config.m_TessControlShader, logMsg);
        glOptions.m_pipelineShaders.m_tessControlShader << config.m_TessControlShader.c_str();
    }

    if (!shouldAbort && isTessEvaluationShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_TESS_EVAL_SHADER, config.m_TessEvaluationShader, logMsg);
        glOptions.m_pipelineShaders.m_tessEvaluationShader << config.m_TessEvaluationShader.c_str();
    }

    if (!shouldAbort && isGeometryexShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_GEOMETRY_SHADER, config.m_GeometryShader, logMsg);
        glOptions.m_pipelineShaders.m_geometryShader << config.m_GeometryShader.c_str();
    }

    if (!shouldAbort && isFragmentShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_FRAGMENT_SHADER, config.m_FragmentShader, logMsg);
        glOptions.m_pipelineShaders.m_fragmentShader << config.m_FragmentShader.c_str();
    }

    if (!shouldAbort && isComputeShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_COMPUTE_SHADER, config.m_ComputeShader, logMsg);
        glOptions.m_pipelineShaders.m_computeShader << config.m_ComputeShader.c_str();
    }

    // Validate the output directories.
    if (!shouldAbort && isIsaRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_ISAFile, logMsg);
    }

    if (!shouldAbort && isLiveRegAnalysisRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_LiveRegisterAnalysisFile, logMsg);
    }

    if (!shouldAbort && isCfgRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_ControlFlowGraphFile, logMsg);
    }

    if (!shouldAbort && isStatisticsRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_AnalysisFile, logMsg);
    }

    if (!shouldAbort && isIsaBinary)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_BinaryOutputFile, logMsg);
    }

    if (!shouldAbort)
    {
        // Set the log callback for the backend.
        m_pOglBuilder->SetLog(callback);

        // If the user did not specify any device, we should use all supported devices.
        std::vector<std::string> targetDecives;
        bool shouldUseAlldevices = config.m_ASICs.empty();

        if (m_supportedDevicesCache.empty() || m_deviceInfo.empty())
        {
            // We need to populate the list of supported devices.
            bool isDeviceListExtracted = GetSupportedDevices();

            if (!isDeviceListExtracted)
            {
                std::stringstream errMsg;
                errMsg << STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST << std::endl;
                shouldAbort = true;
            }
        }

        if (!shouldAbort)
        {
            if (!shouldUseAlldevices)
            {
                // If the user specified a device, go with the user's choice.
                targetDecives = config.m_ASICs;
            }
            else
            {
                // Otherwise, use the cached list of all supported devices.
                std::copy(m_supportedDevicesCache.begin(), m_supportedDevicesCache.end(), std::back_inserter(targetDecives));
            }

            for (const std::string& device : targetDecives)
            {
                // Generate the output message.
                logMsg << KA_CLI_STR_COMPILING << device;

                // Set the target device info for the backend.
                auto iter = m_deviceInfo.find(device);

                if (iter != m_deviceInfo.end())
                {
                    OpenGLDeviceInfo& deviceInfo = iter->second;
                    glOptions.m_chipRevision = deviceInfo.m_deviceId;
                    glOptions.m_chipFamily = deviceInfo.m_deviceFamilyId;
                }
                else
                {
                    shouldAbort = true;
                    logMsg << STR_ERR_CANNOT_GET_DEVICE_INFO << device << std::endl;
                }

                if (!shouldAbort)
                {
                    // Adjust the output file names to the device and shader type.
                    if (isIsaRequired)
                    {
                        glOptions.m_isAmdIsaDisassemblyRequired = true;
                        GenerateRenderingPipelineOutputPaths(config, config.m_ISAFile, device, glOptions.m_isaDisassemblyOutputFiles);
                    }

                    if (isLiveRegAnalysisRequired)
                    {
                        glOptions.m_isLiveRegisterAnalysisRequired = true;
                        GenerateRenderingPipelineOutputPaths(config, config.m_LiveRegisterAnalysisFile, device, glOptions.m_liveRegisterAnalysisOutputFiles);
                    }

                    if (isCfgRequired)
                    {
                        glOptions.m_isLiveRegisterAnalysisRequired = true;
                        GenerateRenderingPipelineOutputPaths(config, config.m_ControlFlowGraphFile, device, glOptions.m_controlFlowGraphOutputFiles);
                    }

                    if (isStatisticsRequired)
                    {
                        glOptions.m_isScStatsRequired = true;
                        GenerateRenderingPipelineOutputPaths(config, config.m_AnalysisFile, device, glOptions.m_scStatisticsOutputFiles);
                    }

                    if (isIsaBinary)
                    {
                        glOptions.m_isAmdIsaBinariesRequired = true;
                        kcUtils::ConstructOutputFileName(config.m_BinaryOutputFile, KC_STR_DEFAULT_BIN_SUFFIX, "", device, glOptions.m_programBinaryFile);
                    }

                    // A handle for canceling the build. Currently not in use.
                    bool shouldCancel = false;

                    // Compile.
                    gtString vcOutput;
                    beKA::beStatus buildStatus = m_pOglBuilder->Compile(glOptions, shouldCancel, vcOutput);

                    if (buildStatus == beStatus_SUCCESS)
                    {
                        logMsg << KA_CLI_STR_STATUS_SUCCESS << std::endl;

                        // Parse and replace the statistics files.
                        beKA::AnalysisData statistics;
                        kcOpenGLStatisticsParser statsParser;

                        if (isStatisticsRequired)
                        {
                            if (isVertexShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_vertexShader, config, device, statsParser, callback);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_tessControlShader, config, device, statsParser, callback);
                            }

                            if (isTessEvaluationShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_tessEvaluationShader, config, device, statsParser, callback);
                            }

                            if (isGeometryexShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_geometryShader, config, device, statsParser, callback);
                            }

                            if (isFragmentShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_fragmentShader, config, device, statsParser, callback);
                            }

                            if (isComputeShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_computeShader, config, device, statsParser, callback);
                            }
                        }

                        // Perform live register analysis if required.
                        if (isLiveRegAnalysisRequired)
                        {
                            if (isVertexShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_vertexShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_vertexShader, callback);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_tessControlShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_tessControlShader, callback);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_tessEvaluationShader, callback);
                            }

                            if (isFragmentShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_fragmentShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_fragmentShader, callback);
                            }

                            if (isComputeShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_computeShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_computeShader, callback);
                            }
                        }

                        // Generate control flow graph if required.
                        if (isCfgRequired)
                        {
                            if (isVertexShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_vertexShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_vertexShader, callback);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_tessControlShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_tessControlShader, callback);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_tessEvaluationShader, callback);
                            }

                            if (isFragmentShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_fragmentShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_fragmentShader, callback);
                            }

                            if (isComputeShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_computeShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_computeShader, callback);
                            }
                        }
                    }
                    else
                    {
                        logMsg << KA_CLI_STR_STATUS_FAILURE << std::endl;

                        if (buildStatus == beKA::beStatus_GLOpenGLVirtualContextFailedToLaunch)
                        {
                            logMsg << STR_ERR_CANNOT_INVOKE_COMPILER << std::endl;
                        }
                    }

                    // Notify the user about build errors if any.
                    if (!vcOutput.isEmpty())
                    {
                        logMsg << vcOutput.asASCIICharArray() << std::endl;
                    }

                    // Print the message for the current device.
                    callback(logMsg.str());

                    // Clear the output stream for the next iteration.
                    logMsg.str("");
                }
            }
        }
    }
    else
    {
        logMsg << KA_CLI_STR_ABORTING << std::endl;
    }

    // Print the output message.
    if (callback != nullptr)
    {
        callback(logMsg.str());
    }

}

