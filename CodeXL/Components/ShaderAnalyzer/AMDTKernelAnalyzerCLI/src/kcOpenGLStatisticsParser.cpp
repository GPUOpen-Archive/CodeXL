// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local.
#include<AMDTKernelAnalyzerCLI/src/kcOpenGLStatisticsParser.h>

// Constants.
const char* GL_ISA_SIZE_TOKEN = "ISA_SIZE";
const char* GL_USED_VGPRS_TOKEN = "USED_VGPRS";
const char* GL_USED_SGPRS_TOKEN = "USED_SGPRS";
const char* GL_USED_SCRATCH_REGS_TOKEN = "SCRATCH_REGS";
const char* GL_END_OF_LINE_DELIMITER = "\n";

kcOpenGLStatisticsParser::kcOpenGLStatisticsParser()
{
}


kcOpenGLStatisticsParser::~kcOpenGLStatisticsParser()
{
}
/// Extracts a numeric value from the SC's textual statistics for Vulkan.
/// Params:
///     fileContent: the content of the SC statistics file.
///     attributeToken: the attribute whose value is to be extracted.
//      numericValue: the extracted value.
// Returns: true for success, false otherwise.
static bool ExtractNumericStatistic(const std::string& fileContent, const char* attributeToken, size_t& extractedValue)
{

    bool ret = false;
    size_t valueBeginIndex = fileContent.find(attributeToken);

    if (valueBeginIndex != std::string::npos)
    {
        valueBeginIndex += strlen(attributeToken) + 1;

        if (valueBeginIndex < fileContent.size())
        {
            size_t valueEndIndex = fileContent.find(GL_END_OF_LINE_DELIMITER, valueBeginIndex) - 1;

            if (valueEndIndex != std::string::npos)
            {
                size_t valueLength = valueEndIndex - valueBeginIndex + 1;

                if (valueLength > 0)
                {
                    // Extract the value.
                    std::string value = fileContent.substr(valueBeginIndex, valueLength);
                    std::string::iterator end_pos = std::remove_if(value.begin(),
                    value.end(), [&value](char c) { return (c == ' ' || !std::isdigit(c)); });
                    value.erase(end_pos, value.end());

                    if (value.empty() == false)
                    {
                        extractedValue = std::stoi(value);
                        ret = true;
                    }

                }
            }
        }
    }

    return ret;
}

// Extracts the ISA size in bytes.
static bool ExtractIsaSize(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, GL_ISA_SIZE_TOKEN, isaSizeInBytes);
}

// Extracts the number of used SGPRs.
static bool ExtractUsedSgprsGL(const std::string& fileContent, size_t& usedSgprs)
{
    return ExtractNumericStatistic(fileContent, GL_USED_SGPRS_TOKEN, usedSgprs);
}

// Extracts the number of used VGPRs.
static bool ExtractUsedVgprsGL(const std::string& fileContent, size_t& usedVgprs)
{
    return ExtractNumericStatistic(fileContent, GL_USED_VGPRS_TOKEN, usedVgprs);
}

// Extracts the scratch registers attributes.
static bool ExtractScratchRegsGL(const std::string& fileContent, size_t& scratchRegs)
{
    return ExtractNumericStatistic(fileContent, GL_USED_SCRATCH_REGS_TOKEN, scratchRegs);
}

bool kcOpenGLStatisticsParser::ParseStatistics(const gtString& satisticsFilePath, beKA::AnalysisData& parsedStatistics)
{
    bool ret = false;
    parsedStatistics.ISASize = 0;
    parsedStatistics.numSGPRsUsed = 0;
    parsedStatistics.numVGPRsUsed = 0;
    parsedStatistics.maxScratchRegsNeeded = 0;

    // Check if the file exists.
    if (!satisticsFilePath.isEmpty())
    {
        osFilePath filePath(satisticsFilePath);

        if (filePath.exists())
        {
            std::ifstream file(satisticsFilePath.asASCIICharArray());
            std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            if (!fileContent.empty())
            {
                // Extract the ISA size in bytes.
                size_t isaSizeInBytes = 0;
                bool isIsaSizeExtracted = ExtractIsaSize(fileContent, isaSizeInBytes);

                if (isIsaSizeExtracted)
                {
                    parsedStatistics.ISASize = isaSizeInBytes;
                }

                // Extract the number of used SGPRs.
                size_t usedSgprs = 0;
                bool isSgprsExtracted = ExtractUsedSgprsGL(fileContent, usedSgprs);

                if (isSgprsExtracted)
                {
                    parsedStatistics.numSGPRsUsed = usedSgprs;
                }

                // Extract the number of used VGPRs.
                size_t usedVgprs = 0;
                bool isVgprsExtracted = ExtractUsedVgprsGL(fileContent, usedVgprs);

                if (isVgprsExtracted)
                {
                    parsedStatistics.numVGPRsUsed = usedVgprs;
                }

                // Extract the scratch registers size.
                size_t scratchRegs = 0;
                bool isScratchRegsExtracted = ExtractScratchRegsGL(fileContent, scratchRegs);

                if (isScratchRegsExtracted)
                {
                    parsedStatistics.maxScratchRegsNeeded = scratchRegs;
                }

                // We succeeded if all data was extracted successfully.
                ret = (isIsaSizeExtracted && isSgprsExtracted && isVgprsExtracted && isScratchRegsExtracted);
            }
        }
    }

    return ret;
}
