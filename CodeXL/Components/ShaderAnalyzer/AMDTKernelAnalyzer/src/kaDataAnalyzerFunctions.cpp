//------------------------------ kaDataAnalyzerFunctions.cpp ------------------------------
#ifdef _WIN32
    //disable std warnings
    #pragma warning(disable : 4996)
#endif

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    //boost warnings disable
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wunused-local-typedefs"
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wformat="
    #pragma GCC diagnostic ignored "-Wformat"
#endif

#include <AMDTBaseTools/Include/gtIgnoreBoostCompilerWarnings.h>
//std
#include <iostream>

//boost
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>    // token class
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp> // lexer class

// Infra
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Framework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeModel.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

using namespace boost::wave;

const unsigned long long MAX_GLOBAL_WG_SIZE = 16777216;

void kaConvertTableRowToKernelExeuctionData(acListCtrl* ipListCtrl, int row, bool hasKernelName, kaKernelExecutionDataStruct* pKernelExecutionData)
{
    GT_IF_WITH_ASSERT(NULL != ipListCtrl)
    {
        // convert the row into the struct:
        int currentColumn = 0;

        if (hasKernelName)
        {
            ipListCtrl->getItemText(row, currentColumn, pKernelExecutionData->m_kernelName);
            currentColumn++;
        }

        for (int nColumn = 0 ; nColumn < 7 ; nColumn++)
        {
            QString currentText;
            ipListCtrl->getItemText(row, currentColumn, currentText);

            switch (nColumn)
            {
                case 0: pKernelExecutionData->m_globalWorkSize[0] = currentText.toInt(); break;

                case 1: pKernelExecutionData->m_globalWorkSize[1] = currentText.toInt(); break;

                case 2: pKernelExecutionData->m_globalWorkSize[2] = currentText.toInt(); break;

                case 3: pKernelExecutionData->m_localWorkSize[0] = currentText.toInt(); break;

                case 4: pKernelExecutionData->m_localWorkSize[1] = currentText.toInt(); break;

                case 5: pKernelExecutionData->m_localWorkSize[2] = currentText.toInt(); break;

                case 6: pKernelExecutionData->m_loopIterations = currentText.toInt(); break;
            }

            currentColumn++;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaValidKernelTableRow
// Description: checks that the data in the table is valid (takes into account if there
//              a column with kernel name (options table does not have, overview table does have)
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
bool kaValidKernelTableRow(acListCtrl* ipListCtrl, bool hasKernelName)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(ipListCtrl != NULL)
    {
        int numRows = ipListCtrl->rowCount();

        for (int nRow = 0 ; nRow < numRows && retVal ; nRow ++)
        {
            kaKernelExecutionDataStruct currentData;
            kaConvertTableRowToKernelExeuctionData(ipListCtrl, nRow, hasKernelName, &currentData);

            // check that not all items (global or local) are zero:
            if (0 == currentData.m_globalWorkSize[0] && 0 == currentData.m_globalWorkSize[1] && 0 == currentData.m_globalWorkSize[2])
            {
                retVal = false;
            }

            if (0 == currentData.m_localWorkSize[0] && 0 == currentData.m_localWorkSize[1] && 0 == currentData.m_localWorkSize[2])
            {
                retVal = false;
            }

            // if values are valid check for multiplication
            if (retVal)
            {
                if ((0 != currentData.m_globalWorkSize[0] % (currentData.m_localWorkSize[0] == 0 ? 1 : currentData.m_localWorkSize[0])) ||
                    (0 != currentData.m_globalWorkSize[1] % (currentData.m_localWorkSize[1] == 0 ? 1 : currentData.m_localWorkSize[1])) ||
                    (0 != currentData.m_globalWorkSize[2] % (currentData.m_localWorkSize[2] == 0 ? 1 : currentData.m_localWorkSize[2])))
                {
                    retVal = false;
                }
            }

            // check that local data x * y * z <= 256
            if (retVal)
            {
                if ((currentData.m_localWorkSize[0] == 0 ? 1 : currentData.m_localWorkSize[0]) *
                    (currentData.m_localWorkSize[1] == 0 ? 1 : currentData.m_localWorkSize[1]) *
                    (currentData.m_localWorkSize[2] == 0 ? 1 : currentData.m_localWorkSize[2])  > 256)
                {
                    retVal = false;
                }
            }

            // check that global data x * y * z <=  MAX_GLOBAL_WG_SIZE
            if (retVal)
            {
                if ((currentData.m_globalWorkSize[0] > (int)MAX_GLOBAL_WG_SIZE) || ((currentData.m_globalWorkSize[0] >= (int)MAX_GLOBAL_WG_SIZE) && ((currentData.m_globalWorkSize[1] > 1) || (currentData.m_globalWorkSize[2] > 1))) ||
                    (currentData.m_globalWorkSize[1] > (int)MAX_GLOBAL_WG_SIZE) || ((currentData.m_globalWorkSize[1] >= (int)MAX_GLOBAL_WG_SIZE) && ((currentData.m_globalWorkSize[0] > 1) || (currentData.m_globalWorkSize[2] > 1))) ||
                    (currentData.m_globalWorkSize[2] > (int)MAX_GLOBAL_WG_SIZE) || ((currentData.m_globalWorkSize[2] >= (int)MAX_GLOBAL_WG_SIZE) && ((currentData.m_globalWorkSize[1] > 1) || (currentData.m_globalWorkSize[0] > 1))))
                {
                    retVal = false;
                }
            }

            if (retVal)
            {
                unsigned long long l0 = (currentData.m_globalWorkSize[0] == 0 ? 1 : currentData.m_globalWorkSize[0]);
                unsigned long long l1 = (currentData.m_globalWorkSize[1] == 0 ? 1 : currentData.m_globalWorkSize[1]);
                unsigned long long l2 = (currentData.m_globalWorkSize[2] == 0 ? 1 : currentData.m_globalWorkSize[2]);

                if (((l0 * l1) > MAX_GLOBAL_WG_SIZE) || ((l0 * l2) > MAX_GLOBAL_WG_SIZE) || ((l1 * l2) > MAX_GLOBAL_WG_SIZE) || ((l0 * l1 * l2) > MAX_GLOBAL_WG_SIZE))
                {
                    retVal = false;
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaReadFileAsQString
// Description: open a file and reads it into QString
// Arguments:   osFilePath& filePath
//              QString& fileAsQString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool kaReadFileAsQString(const osFilePath& filePath, QString& fileAsQString)
{
    osFile kernelFile;
    bool retVal = kernelFile.open(filePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_READ);

    GT_IF_WITH_ASSERT(retVal)
    {
        unsigned long fileSize;
        retVal = kernelFile.getSize(fileSize);

        GT_IF_WITH_ASSERT(retVal)
        {
            char* fileBuffer = (char*)malloc(fileSize + 1);
            gtSize_t fileRead;
            kernelFile.readAvailableData(fileBuffer, fileSize, fileRead);

            // make sure the file is null terminated:
            fileBuffer[fileSize] = 0;

            fileAsQString = fileBuffer;

            free(fileBuffer);
        }
        kernelFile.close();
    }

    return retVal;
}

void kaBuildHTMLFileInfo(const osFilePath& filePath, afHTMLContent& htmlContent)
{
    QString retVal;

    // Add the general information of the overview:
    gtString fileNameAndExtension;
    filePath.getFileNameAndExtension(fileNameAndExtension);

    // Build the HTML content for this object:
    gtString fileExtension;
    gtString infoTitle = KA_STR_htmlInfoKernels;
    filePath.getFileExtension(fileExtension);

    if (fileExtension == KA_STR_kernelFileExtension)
    {
        // if cl file - kernel
        htmlContent.setTitle(KA_STR_htmlInfoCLFileCaption);
    }
    else
    {
        // else - shader
        htmlContent.setTitle(KA_STR_htmlInfoShaderFileCaption);
        infoTitle = KA_STR_htmlInfoShaders;
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, fileNameAndExtension);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, KA_STR_htmlInfoCLFileFullPath, filePath.asString());

    // Add the number of lines:
    QString fileAsString;

    if (kaReadFileAsQString(filePath, fileAsString))
    {
        gtString numLines;
        numLines.appendFormattedString(L"%d", fileAsString.count('\n') + 1);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, KA_STR_htmlInfoNumOfLines, numLines);
    }

    // Add the kernels of the files as a table:
    kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePath);

    // file can be null first time it is loaded in VS
    if (NULL != pCurrentFile)
    {
        int numKernels = pCurrentFile->analyzeVector().size();

        if (0 == numKernels)
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, infoTitle, KA_STR_htmlInfoEmptyKernelList);
        }
        else
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, infoTitle, AF_STR_Empty);

            for (int nKernel = 0 ; nKernel < numKernels ; nKernel ++)
            {
                QString kernelName = pCurrentFile->analyzeVector().at(nKernel).m_kernelName;
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Empty, acQStringToGTString(kernelName));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaFindFamilyName
// Description: Find the family of the device specific name:
// Author:      Gilad Yarnitzky
// Date:        24/8/2013
// ---------------------------------------------------------------------------
bool kaFindFamilyName(QString& deviceName, QString& familyName)
{
    bool retVal = false;
    QStringList& deviceList = kaGlobalVariableManager::instance().currentTreeList();
    int numDevices = deviceList.count();

    for (int nDevice = 0 ; nDevice < numDevices ; nDevice++)
    {
        QString copyDevice = deviceList[nDevice];
        int depthLevel = CheckableTreeItem::removeIndents(copyDevice);

        if (1 == depthLevel)
        {
            familyName = copyDevice;
        }
        else if (2 == depthLevel)
        {
            if (copyDevice.contains(deviceName))
            {
                retVal = true;
                break;
            }
            // this is to overcome a bug in when device name comes with weird characters
            else
            {
                // go over the name and replace funny characters with _ because we later make it as a file name
                std::string tempdeviceName;
                std::string sOrigin = copyDevice.toStdString();
                kaNormelizeDeviceName(sOrigin, tempdeviceName);

                if (tempdeviceName.find(deviceName.toStdString()) != std::string::npos)
                {
                    retVal = true;
                    break;
                }
            }
        }
    }

    if (retVal)
    {
        // Clear the device name a bit:
        int textEnd = familyName.indexOf("(");

        if (textEnd != -1)
        {
            familyName = familyName.left(textEnd - 1);
        }
        else
        {
            // this is a CPU family name
            textEnd = familyName.indexOf(" ");

            if (textEnd != -1)
            {
                familyName = familyName.left(textEnd);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaParseBuildFile(const osFilePath& buildFile, gtString& kernelName, gtString& device, gtString& codeRep)
{
    GT_UNREFERENCED_PARAMETER(kernelName);//TODO get kernel name for specific APIs(not openCL)
    gtString buildFileName;
    buildFile.getFileName(buildFileName);
    buildFile.getFileExtension(codeRep);
    int deviceEndIx = buildFileName.findFirstOf(L"_") - 1;

    if (deviceEndIx > 0)
    {
        buildFileName.getSubString(0, deviceEndIx, device);
    }
    //on GL build  filepaths we have onyl device in file name
    else
    {
        device = buildFileName;
    }
}

void kaNormelizeDeviceName(const std::string& sOrigin, std::string& sDest)
{
    // go over the name and replace funny characters with _ because we later make it as a file name
    sDest = sOrigin;
    std::size_t found = sDest.find("/");

    for (; found != std::string::npos; found = sDest.find("/"))
    {
        sDest.replace(found, 1, "_");
    }

    found = sDest.find(":");

    for (; found != std::string::npos; found = sDest.find(":"))
    {
        sDest.replace(found, 1, "_");
    }
}

//custome directive hook class for boost::wave context
//here can be defined customized hooks of the wave library
class custom_directives_hooks : public context_policies::default_preprocessing_hooks
{
public:
    //override default policy to check include files
    template <typename ContextT>
    bool found_include_directive(ContextT const&, std::string const&,  bool)
    {
        // not ok to include this include file
        return true;
    }
};

bool ExpandMacros(std::string& inSourceCodeString, const std::wstring& rawSourceCodeFileName, const std::vector<std::string>& additionalMacros, std::vector<PreProcessedToken>& tokens)
{
    bool result = false;

    try
    {
        //  This token type is one of the central types used throughout the library.
        //  It is a template parameter to some of the public classes and instances
        //  of this type are returned from the iterators.
        using  token_type = cpplexer::lex_token<>;

        //  The template boost::wave::cpplexer::lex_iterator<> is the lexer type to
        //  to use as the token source for the preprocessing engine. It is
        //  parametrized with the token type.
        using lex_iterator_type = cpplexer::lex_iterator<token_type>;

        //  This is the resulting context type. The first template parameter should
        //  match the iterator type used during construction of the context
        //  instance (see below). It is the type of the underlying input stream.
        using context_type = context<std::string::iterator, lex_iterator_type, iteration_context_policies::load_file_to_string, custom_directives_hooks>;
        //  The preprocessor iterator shouldn't be constructed directly. It is
        //  generated through a wave::context<> object. This wave:context<> object
        //  is additionally used to initialize and define different parameters of
        //  the actual preprocessing (not done here).
        //
        //  The preprocessing of the input stream is done on the fly behind the
        //  scenes during iteration over the range of context_type::iterator_type
        //  instances.
        context_type ctx(inSourceCodeString.begin(), inSourceCodeString.end(), gtString(rawSourceCodeFileName.c_str()).asASCIICharArray());

        language_support languageMask = ctx.get_language();
        languageMask = static_cast <language_support>(languageMask & ~support_option_emit_line_directives);
        languageMask = static_cast<language_support>(languageMask | support_option_single_line);
        ctx.set_language(languageMask);

        for (const auto& val : additionalMacros)
        {
            ctx.add_macro_definition(val, true);
        }

        //  Get the preprocessor iterators and use them to generate the token
        //  sequence.
        context_type::iterator_type first = ctx.begin();
        context_type::iterator_type last = ctx.end();

        ////  The input stream is preprocessed for you while iterating over the range
        ////  [first, last). The dereferenced iterator returns tokens holding
        ////  information about the preprocessed input stream, such as token type,
        ////  token value, and position.
        while (first != last)
        {
            token_id id = token_id(*first);
            PreProcessedToken token{ first->get_value().c_str(),
                                     get_token_name(id).c_str(),
                                     first->get_position().get_file().c_str(),
                                     first->get_position().get_line(),
                                     first->get_position().get_column() };
            tokens.push_back(token);
            ++first;
        }

        result = true;
    }
    catch (cpp_exception const& e)
    {
        // some preprocessing error
        gtString msg = L"Error during parsing :";
        msg.append(rawSourceCodeFileName.c_str()).append(L", ").append(gtString().fromASCIIString(e.description()));
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
    catch (std::exception const& e)
    {
        // some std error
        gtString msg = L"General Error during parsing :";
        msg.append(rawSourceCodeFileName.c_str()).append(L", ").append(gtString().fromASCIIString(e.what()));
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return result;
}

bool ExpandMacros(const std::wstring& rawSourceCodeFileName, const std::vector<std::string>& additionalMacros, std::vector<PreProcessedToken>& tokens)
{
    bool result = false;

    try
    {
        //  The following preprocesses the input file given by rawSourceCodeFileName.
        //  Open and read in the specified input file.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        gtString filename = rawSourceCodeFileName.c_str();
        ifstream instream(filename.asASCIICharArray());
#else
        std::ifstream instream(rawSourceCodeFileName);
#endif

        if (instream.is_open())
        {
            instream.unsetf(std::ios::skipws);
            std::string instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()), std::istreambuf_iterator<char>());
            result = ExpandMacros(instring, rawSourceCodeFileName, additionalMacros, tokens);
        }//if
    }//try
    catch (std::exception const& e)
    {
        // use last recognized token to retrieve the error position
        gtString msg = L"Failed to parse :";
        msg.append(rawSourceCodeFileName.c_str()).append(L", error : ").append(gtString().fromASCIIString(e.what()));
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }
    catch (...)
    {
        gtString msg = L"Failed to parse :";
        msg.append(rawSourceCodeFileName.c_str());
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return result;
}
