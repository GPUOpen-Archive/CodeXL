// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Backend.
#include <AMDTBackEnd/Include/beStaticIsaAnalyzer.h>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcUtils.h>
#include <AMDTKernelAnalyzerCLI/src/kcCliStringConstants.h>
#include <AMDTKernelAnalyzerCLI/src/kcCLICommander.h>

using namespace beKA;

bool kcUtils::ValidateShaderFileName(const char* shaderType, const std::string& shaderFileName, std::stringstream& logMsg)
{
    bool isShaderNameValid = true;
    gtString shaderFileNameAsGtStr;
    shaderFileNameAsGtStr << shaderFileName.c_str();
    osFilePath shaderFile(shaderFileNameAsGtStr);

    if (!shaderFile.exists())
    {
        isShaderNameValid = false;
        logMsg << STR_ERR_CANNOT_FIND_SHADER_PREFIX << shaderType << STR_ERR_CANNOT_FIND_SHADER_SUFFIX << shaderFileName << std::endl;
    }

    return isShaderNameValid;
}

bool kcUtils::ValidateShaderOutputDir(const std::string& outputFileName, std::stringstream& logMsg)
{
    bool isShaderOutputDirValid = true;
    gtString shaderFileNameAsGtStr;
    shaderFileNameAsGtStr << outputFileName.c_str();
    osFilePath shaderFile(shaderFileNameAsGtStr);
    osDirectory outputDir;
    shaderFile.getFileDirectory(outputDir);
    isShaderOutputDirValid = outputDir.exists();

    if (!isShaderOutputDirValid)
    {
        logMsg << STR_ERR_CANNOT_FIND_OUTPUT_DIR << outputDir.directoryPath().asString().asASCIICharArray() << std::endl;
    }

    return isShaderOutputDirValid;
}


void kcUtils::AdjustRenderingPipelineOutputFileNames(const std::string& baseOutputFileName, const std::string& device, beProgramPipeline& pipelineFiles)
{
    // Clear the existing pipeline.
    pipelineFiles.ClearAll();

    // Isolate the original file name.
    gtString outputFileAsGtStr;
    outputFileAsGtStr << baseOutputFileName.c_str();
    osFilePath outputFilePath(outputFileAsGtStr);

    // Directory.
    osDirectory outputDir;
    outputFilePath.getFileDirectory(outputDir);

    // File name.
    gtString originalFileName;
    outputFilePath.getFileName(originalFileName);

    // File extension.
    gtString originalFileExtension;
    outputFilePath.getFileExtension(originalFileExtension);

    // Make the adjustments.
    gtString fixedFileName;
    fixedFileName << outputDir.directoryPath().asString(true) << device.c_str() << "_";
    pipelineFiles.m_vertexShader << fixedFileName << KA_CLI_STR_VERTEX_ABBREVIATION;
    pipelineFiles.m_tessControlShader << fixedFileName << KA_CLI_STR_TESS_CTRL_ABBREVIATION;
    pipelineFiles.m_tessEvaluationShader << fixedFileName << KA_CLI_STR_TESS_EVAL_ABBREVIATION;
    pipelineFiles.m_geometryShader << fixedFileName << KA_CLI_STR_GEOMETRY_ABBREVIATION;
    pipelineFiles.m_fragmentShader << fixedFileName << KA_CLI_STR_FRAGMENT_ABBREVIATION;
    pipelineFiles.m_computeShader << fixedFileName << KA_CLI_STR_COMPUTE_ABBREVIATION;

    if (!originalFileName.isEmpty())
    {
        pipelineFiles.m_vertexShader << "_" << originalFileName.asASCIICharArray();
        pipelineFiles.m_tessControlShader << "_" << originalFileName.asASCIICharArray();
        pipelineFiles.m_tessEvaluationShader << "_" << originalFileName.asASCIICharArray();
        pipelineFiles.m_geometryShader << "_" << originalFileName.asASCIICharArray();
        pipelineFiles.m_fragmentShader << "_" << originalFileName.asASCIICharArray();
        pipelineFiles.m_computeShader << "_" << originalFileName.asASCIICharArray();
    }

    if (!originalFileExtension.isEmpty())
    {
        pipelineFiles.m_vertexShader << "." << originalFileExtension.asASCIICharArray();
        pipelineFiles.m_tessControlShader << "." << originalFileExtension.asASCIICharArray();
        pipelineFiles.m_tessEvaluationShader << "." << originalFileExtension.asASCIICharArray();
        pipelineFiles.m_geometryShader << "." << originalFileExtension.asASCIICharArray();
        pipelineFiles.m_fragmentShader << "." << originalFileExtension.asASCIICharArray();
        pipelineFiles.m_computeShader << "." << originalFileExtension.asASCIICharArray();
    }
}

std::string kcUtils::DeviceStatisticsToCsvString(const Config& config, const std::string& device, const beKA::AnalysisData& statistics)
{
    std::stringstream output;

    // Device name.
    char csvSeparator = GetCsvSeparator(config);
    output << device << csvSeparator;

    // Scratch registers.
    output << statistics.maxScratchRegsNeeded << csvSeparator;

    // Work-items per work-group.
    output << statistics.numThreadPerGroup << csvSeparator;

    // Wavefront size.
    output << statistics.wavefrontSize << csvSeparator;

    // LDS available bytes.
    output << statistics.LDSSizeAvailable << csvSeparator;

    // LDS actual bytes.
    output << statistics.LDSSizeUsed << csvSeparator;

    // Available SGPRs.
    output << statistics.numSGPRsAvailable << csvSeparator;

    // Used SGPRs.
    output << statistics.numSGPRsUsed << csvSeparator;

    // Available VGPRs.
    output << statistics.numVGPRsAvailable << csvSeparator;

    // Used VGPRs.
    output << statistics.numVGPRsUsed << csvSeparator;

    // CL Work-group dimensions (for a unified format, to be revisited).
    output << statistics.numThreadPerGroupX << csvSeparator;
    output << statistics.numThreadPerGroupY << csvSeparator;
    output << statistics.numThreadPerGroupZ << csvSeparator;

    // ISA size.
    output << statistics.ISASize;

    output << std::endl;

    return output.str().c_str();
}

bool kcUtils::CreateStatisticsFile(const gtString& fileName, const Config& config,
                                   const std::map<std::string, beKA::AnalysisData>& analysisData, LoggingCallBackFunc_t pLogCallback)
{
    bool ret = false;

    // Get the separator for CSV list items.
    char csvSeparator = GetCsvSeparator(config);

    // Open output file.
    std::ofstream output;
    output.open(fileName.asASCIICharArray());

    if (output.is_open())
    {
        // Write the header.
        output << GetStatisticsCsvHeaderString(csvSeparator) << std::endl;

        // Write the device data.
        for (const auto& deviceStatsPair : analysisData)
        {
            // Write a line of CSV.
            output << DeviceStatisticsToCsvString(config, deviceStatsPair.first, deviceStatsPair.second);
        }

        output.close();
        ret = true;
    }
    else if (pLogCallback != nullptr)
    {
        std::stringstream s_Log;
        s_Log << STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_A << fileName.asASCIICharArray() <<
              STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_B << std::endl;
        pLogCallback(s_Log.str());
    }

    return ret;
}


std::string kcUtils::GetStatisticsCsvHeaderString(char csvSeparator)
{
    std::stringstream output;
    output << STR_CSV_HEADER_DEVICE << csvSeparator;
    output << STR_CSV_HEADER_SCRATCH_REGS << csvSeparator;
    output << STR_CSV_HEADER_THREADS_PER_WG << csvSeparator;
    output << STR_CSV_HEADER_WAVEFRONT_SIZE << csvSeparator;
    output << STR_CSV_HEADER_LDS_BYTES_MAX << csvSeparator;
    output << STR_CSV_HEADER_LDS_BYTES_ACTUAL << csvSeparator;
    output << STR_CSV_HEADER_SGPR_AVAILABLE << csvSeparator;
    output << STR_CSV_HEADER_SGPR_USED << csvSeparator;
    output << STR_CSV_HEADER_VGPR_AVAILABLE << csvSeparator;
    output << STR_CSV_HEADER_VGPR_USED << csvSeparator;
    output << STR_CSV_HEADER_CL_WORKGROUP_DIM_X << csvSeparator;
    output << STR_CSV_HEADER_CL_WORKGROUP_DIM_Y << csvSeparator;
    output << STR_CSV_HEADER_CL_WORKGROUP_DIM_Z << csvSeparator;
    output << STR_CSV_HEADER_ISA_SIZE_BYTES;
    return output.str().c_str();
}

void kcUtils::CreateStatisticsFile(const gtString& fileName, const Config& config, const std::string& device,
                                   const beKA::AnalysisData& deviceStatistics, LoggingCallBackFunc_t pLogCallback)
{
    // Create a temporary map and invoke the general routine.
    std::map<std::string, beKA::AnalysisData> tmpMap;
    tmpMap[device] = deviceStatistics;
    CreateStatisticsFile(fileName, config, tmpMap, pLogCallback);
}

char kcUtils::GetCsvSeparator(const Config& config)
{
    char csvSeparator;

    if (!config.m_CSVSeparator.empty())
    {
        csvSeparator = config.m_CSVSeparator[0];

        if (config.m_CSVSeparator[0] == '\\' && config.m_CSVSeparator.size() > 1)
        {
            switch (config.m_CSVSeparator[1])
            {
                case 'a': csvSeparator = '\a'; break;

                case 'b': csvSeparator = '\b'; break;

                case 'f': csvSeparator = '\f'; break;

                case 'n': csvSeparator = '\n'; break;

                case 'r': csvSeparator = '\r'; break;

                case 't': csvSeparator = '\t'; break;

                case 'v': csvSeparator = '\v'; break;

                default:
                    csvSeparator = config.m_CSVSeparator[1];
                    break;
            }
        }
    }
    else
    {
        // The default separator.
        csvSeparator = ',';
    }

    return csvSeparator;
}

bool kcUtils::DeleteFile(const gtString& fileFullPath)
{
    bool ret = false;
    osFilePath path(fileFullPath);

    if (path.exists())
    {
        osFile file(path);
        ret = file.deleteFile();
    }

    return ret;
}

void kcUtils::ReplaceStatisticsFile(const gtString& statisticsFile, const Config& config,
                                    const std::string& device, IStatisticsParser& statsParser, LoggingCallBackFunc_t logCb)
{
    // Parse the backend statistics.
    beKA::AnalysisData statistics;
    statsParser.ParseStatistics(statisticsFile, statistics);

    // Delete the older statistics file.
    kcUtils::DeleteFile(statisticsFile);

    // Create a new statistics file in the CLI format.
    kcUtils::CreateStatisticsFile(statisticsFile, config, device, statistics, logCb);
}

void kcUtils::PerformLiveRegisterAnalysis(const gtString& isaFileName,
                                          const gtString& outputFileName, LoggingCallBackFunc_t pCallback)
{
    // Call the backend.
    beStatus rc = beStaticIsaAnalyzer::PerformLiveRegisterAnalysis(isaFileName, outputFileName);

    if (rc != beStatus_SUCCESS && pCallback != nullptr)
    {
        // Inform the user in case of an error.
        std::stringstream msg;

        switch (rc)
        {
            case beKA::beStatus_shaeCannotLocateAnalyzer:
                // Failed to locate the ISA analyzer.
                msg << STR_ERR_CANNOT_LOCATE_LIVE_REG_ANALYZER << std::endl;
                break;

            case beKA::beStatus_shaeIsaFileNotFound:
                // ISA file not found.
                msg << STR_ERR_CANNOT_FIND_ISA_FILE << std::endl;
                break;

            case beKA::beStatus_shaeFailedToLaunch:
                // Failed to launch the ISA analyzer.
                msg << STR_ERR_CANNOT_LAUNCH_LIVE_REG_ANALYZER << std::endl;
                break;

            case beKA::beStatus_General_FAILED:
            default:
                // Generic error message.
                msg << STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS << std::endl;
                break;
        }

        const std::string& errMsg = msg.str();

        if (!errMsg.empty() && pCallback != nullptr)
        {
            pCallback(errMsg);
        }
    }
}

void kcUtils::GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName, LoggingCallBackFunc_t pCallback)
{
    // Call the backend.
    beStatus rc = beStaticIsaAnalyzer::GenerateControlFlowGraph(isaFileName, outputFileName);

    if (rc != beStatus_SUCCESS && pCallback != nullptr)
    {
        // Inform the user in case of an error.
        std::stringstream msg;

        switch (rc)
        {
            case beKA::beStatus_shaeCannotLocateAnalyzer:
                // Failed to locate the ISA analyzer.
                msg << STR_ERR_CANNOT_LOCATE_LIVE_REG_ANALYZER << std::endl;
                break;

            case beKA::beStatus_shaeIsaFileNotFound:
                // ISA file not found.
                msg << STR_ERR_CANNOT_FIND_ISA_FILE << std::endl;
                break;

            case beKA::beStatus_shaeFailedToLaunch:
                // Failed to launch the ISA analyzer.
                msg << STR_ERR_CANNOT_LAUNCH_CFG_ANALYZER << std::endl;
                break;

            case beKA::beStatus_General_FAILED:
            default:
                // Generic error message.
                msg << STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS << std::endl;
                break;
        }

        const std::string& errMsg = msg.str();

        if (!errMsg.empty() && pCallback != nullptr)
        {
            pCallback(errMsg);
        }
    }

}

void kcUtils::ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultExtension, const std::string& kernelName, const std::string& deviceName, gtString& generatedFileName)
{
    // Convert the base output file name to gtString.
    gtString baseOutputFileNameAsGtStr;
    baseOutputFileNameAsGtStr << baseOutputFileName.c_str();
    osFilePath outputFilePath(baseOutputFileNameAsGtStr);

    // Extract the user's file name and extension.
    gtString fileName;
    outputFilePath.getFileName(fileName);

    // Fix the user's file name to generate a unique output file name in the Analyzer CLI format.
    gtString fixedFileName;
    fixedFileName << deviceName.c_str();

    if (!kernelName.empty())
    {
        if (!fixedFileName.isEmpty())
        {
            fixedFileName << "_";
        }

        fixedFileName << kernelName.c_str();
    }

    if (!fileName.isEmpty())
    {
        if (!fixedFileName.isEmpty())
        {
            fixedFileName << "_";
        }

        fixedFileName << fileName;
    }

    outputFilePath.setFileName(fixedFileName);

    // Handle the default extension (unless the user specified an extension).
    gtString outputFileExtension;
    outputFilePath.getFileExtension(outputFileExtension);

    if (outputFileExtension.isEmpty())
    {
        outputFileExtension.fromASCIIString(defaultExtension.c_str());
        outputFilePath.setFileExtension(outputFileExtension);
    }

    // Set the output string.
    generatedFileName = outputFilePath.asString();
}

kcUtils::kcUtils()
{
}


kcUtils::~kcUtils()
{
}
