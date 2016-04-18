#ifndef kcUtils_h__
#define kcUtils_h__

// C++.
#include <string>
#include <sstream>
#include <map>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>

// Backend.
#include <AMDTBackEnd/Include/beDataTypes.h>
#include <AMDTBackEnd/Include/beInclude.h>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcDataTypes.h>
#include <AMDTKernelAnalyzerCLI/src/kcConfig.h>
#include <AMDTKernelAnalyzerCLI/src/kcIStatisticsParser.h>

class kcUtils
{
public:
    // Helper function to validate a shader's file name and generate the appropriate output message.
    // shaderType: the stage of the shader (vertex, tessellation control, tessellation evaluation, geometry, fragment or compute).
    // shaderFileName: the file name to be validated.
    // logMsg: the stream to which the output should be directed.
    // Returns true if the file name is valid, and false otherwise.
    static bool ValidateShaderFileName(const char* shaderType, const std::string& shaderFileName, std::stringstream& logMsg);

    // Helper function to validate an output file's directory and generate the appropriate output message.
    // outputFileName: the file name to be validated.
    // logMsg: the stream to which the output should be directed.
    // Returns true if the directory is valid, and false otherwise.
    static bool ValidateShaderOutputDir(const std::string& outputFileName, std::stringstream& logMsg);

    // Helper function to construct the output file name in Analyzer CLI's output format, which combines
    // the base output file name, the target device name and the rendering pipeline stage.
    static void AdjustRenderingPipelineOutputFileNames(const std::string& baseOutputFileName,
                                                       const std::string& device, beProgramPipeline& pipelineFiles);

    // Creates the statistics file according to the user's configuration.
    // config: the user's configuration
    // analysisData: a map that contains each device's statistics data
    // // logCallback: the log callback
    static bool CreateStatisticsFile(const gtString& fileName, const Config& config,
                                     const std::map<std::string, beKA::AnalysisData>& analysisData, LoggingCallBackFunc_t logCallback);

    // Creates the statistics file according to the user's configuration.
    // fileName: the target statistics file name
    // config: the user's configuration.
    // device: the target device
    // analysisData: the device's statistics data.
    // logCallback: the log callback
    static void CreateStatisticsFile(const gtString& fileName, const Config& config,
                                     const std::string& device, const beKA::AnalysisData& analysisData, LoggingCallBackFunc_t logCallback);

    // Generates the CLI statistics file header.
    // csvSeparator - the character that is being used
    static std::string GetStatisticsCsvHeaderString(char csvSeparator);

    // Returns the CLI statistics CSV separator, according to the user's configuration.
    static char GetCsvSeparator(const Config& config);

    // Converts the device statistics to a CSV string.
    static std::string DeviceStatisticsToCsvString(const Config& config, const std::string& device, const beKA::AnalysisData& statistics);

    // Deletes the a file from the file system.
    // fileFullPath - the full path to the file to be deleted.
    static bool DeleteFile(const gtString& fileFullPath);

    // Replaces a backend statistics file with a CLI statistics file.
    // statisticsFile - full path to the file to be replaced
    // config - user configuration
    // device - the name of the device for which the statistics where generated
    // statsParser - a parser to be used to parse the backend raw statistics file
    // logCb - a log callback
    static void ReplaceStatisticsFile(const gtString& statisticsFile, const Config& config, const std::string& device,
                                      IStatisticsParser& statsParser, LoggingCallBackFunc_t logCb);

    // Performs live register analysis for the ISA in the given file, and dumps
    // the output to the given output file name.
    // isaFileName - the disassembled ISA file name
    // outputFileName - the output file name
    // pCallback - callback to log messages
    static void PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName, LoggingCallBackFunc_t pCallback);

    // Generates control flow graph for the given ISA.
    // isaFileName - the disassembled ISA file name
    // outputFileName - the output file name
    // pCallback - callback to log messages
    static void GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName, LoggingCallBackFunc_t pCallback);

    // Generates an output file name in the Analyzer CLI format.
    // baseOutputFileName - the base output file name as configured by the user's command
    // defaultExtension - default extension to use if user did not specify an extension for the output file
    // entryPointName - the name of the entry point to which the output file refers (can be empty if not relevant)
    // deviceName - the name of the target device to which the output file refers (can be empty if not relevant)
    // generatedFileName - an output variable to hold the generated file name
    static void ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultExtension,
                                        const std::string& entryPointName, const std::string& deviceName, gtString& generatedFileName);

private:
    // This is a static class (no instances).
    kcUtils(const kcUtils& other);
    kcUtils();
    ~kcUtils();
};

#endif // kcUtils_h__
