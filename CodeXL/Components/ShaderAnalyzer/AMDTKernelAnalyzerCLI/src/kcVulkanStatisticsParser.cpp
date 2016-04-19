// C++.
#include <iosfwd>
#include <streambuf>
#include <vector>

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcVulkanStatisticsParser.h>
#include <AMDTKernelAnalyzerCLI/src/kcUtils.h>

// Constants.
const char* ISA_SIZE_TOKEN = "codeLenInByte";
const char* USED_VGPRS_TOKEN = "NumVgprs";
const char* USED_SGPRS_TOKEN = "NumSgprs";
const char* END_OF_LINE_DELIMITER = ";";

kcVulkanStatisticsParser::kcVulkanStatisticsParser()
{
}

kcVulkanStatisticsParser::~kcVulkanStatisticsParser()
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
            size_t valueEndIndex = fileContent.find(END_OF_LINE_DELIMITER, valueBeginIndex) - 1;

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
                    extractedValue = std::stoi(value);
                    ret = true;
                }
            }
        }
    }

    return ret;
}

// Extracts the ISA size in bytes.
static bool ExtractIsaSize(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, ISA_SIZE_TOKEN, isaSizeInBytes);
}

// Extracts the number of used SGPRs.
static bool ExtractUsedSgprs(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, USED_SGPRS_TOKEN, isaSizeInBytes);
}

// Extracts the number of used VGPRs.
static bool ExtractUsedVgprs(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, USED_VGPRS_TOKEN, isaSizeInBytes);
}

bool kcVulkanStatisticsParser::ParseStatistics(const gtString& satisticsFilePath, beKA::AnalysisData& parsedStatistics)
{
    bool ret = false;
    parsedStatistics.ISASize = 0;
    parsedStatistics.numSGPRsUsed = 0;
    parsedStatistics.numVGPRsUsed = 0;

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
                bool isSgprsExtracted = ExtractUsedSgprs(fileContent, usedSgprs);

                if (isSgprsExtracted)
                {
                    parsedStatistics.numSGPRsUsed = usedSgprs;
                }

                // Extract the number of used VGPRs.
                size_t usedVgprs = 0;
                bool isVgprsExtracted = ExtractUsedVgprs(fileContent, usedVgprs);

                if (isVgprsExtracted)
                {
                    parsedStatistics.numVGPRsUsed = usedVgprs;
                }

                // We succeeded if all data was extracted successfully.
                ret = (isIsaSizeExtracted && isSgprsExtracted && isVgprsExtracted);
            }
        }
    }

    return ret;
}

