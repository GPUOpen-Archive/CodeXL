//------------------------------ kaDataAnalyzerFunctions.h ------------------------------

#ifndef __KADATAANALYZERFUNCTIONS_H
#define __KADATAANALYZERFUNCTIONS_H

struct kaKernelExecutionDataStruct;
class acListCtrl;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

//Local
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>


void kaConvertTableRowToKernelExeuctionData(acListCtrl* ipListCtrl, int row, bool hasKernelName, kaKernelExecutionDataStruct* pKernelExecutionData);
bool kaValidKernelTableRow(acListCtrl* ipListCtrl, bool hasKernelName);
bool kaReadFileAsQString(const osFilePath& filePath, QString& fileAsQString);
void kaBuildHTMLFileInfo(const osFilePath& filePath, afHTMLContent& htmlContent);
// Find the family of the device name:
bool kaFindFamilyName(QString& deviceName, QString& familyName);

/// kaParseBuildFile returns the parts of the build file that comes in a format:
/// kernel device_ISA:1 or kernel device_IL:0
/// This function return the parts in the string
void kaParseBuildFile(const osFilePath& buildFile, gtString& kernelName, gtString& device, gtString& codeRep);
void kaNormelizeDeviceName(const std::string& sOrigin, std::string& sDest);
struct  PreProcessedToken
{
    std::string value;
    std::string tokenId;
    std::string  filePath;
    size_t line;
    size_t column;
};
/// Pre-processes given file and replaces all macros definitions
/// \param rawSourceCodeFileName file path to source code file
/// \param preproceesedSourceCodeResult pre-processed result
/// \param additionalMacros holds additional macros to be defined ,PAY ATTENTION:  the format of additional macro must be <name>=<value>
/// \param  tokens output tokens information
KA_API bool ExpandMacros(const std::wstring& rawSourceCodeFileName, const std::vector<std::string>& additionalMacros, std::vector<PreProcessedToken>& tokens);
/// Pre-processes given file and replaces all macros definitions
/// \param inSourceCodeString  input string with source codes
/// \param preproceesedSourceCodeResult pre-processed result
/// \param additionalMacros holds additional macros to be defined ,PAY ATTENTION:  the format of additional macro must be <name>=<value>
/// \param  tokens output tokens information
KA_API bool  ExpandMacros(std::string& inSourceCodeString, const std::wstring& rawSourceCodeFileName, const std::vector<std::string>& additionalMacros, std::vector<PreProcessedToken>& tokens);

#endif // __KADATAANALYZERFUNCTIONS_H
