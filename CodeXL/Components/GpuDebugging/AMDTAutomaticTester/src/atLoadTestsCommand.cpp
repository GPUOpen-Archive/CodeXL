//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atLoadTestsCommand.cpp
///
//==================================================================================

//------------------------------ atLoadTestsCommand.cpp ------------------------------

// Standard C++:
#include <iostream>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <inc/atLoadTestsCommand.h>
#include <inc/atUtils.h>


// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::atLoadTestsCommand
// Description: Constructor
// Arguments:   const osFilePath& fileName
// Author:      Merav Zanany
// Date:        1/12/2011
// ---------------------------------------------------------------------------
atLoadTestsCommand::atLoadTestsCommand(const osFilePath& fileName, gtVector<atPrintableTestData>& testsVector):
    _filePath(fileName), _testsVector(testsVector)
{
}

// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::~atLoadTestsCommand
// Description: Destructor
// Author:      Merav Zanany
// Date:        1/12/2011
// ---------------------------------------------------------------------------
atLoadTestsCommand::~atLoadTestsCommand() {}

// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::execute
// Description: Load the XML file and read the tests settings
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        1/12/2011
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::execute()
{
    bool rc = false;

    // Check if the XML file exists:
    rc = _filePath.exists();

    if (!rc)
    {
        std::cout << "Required XML file does not exist: " << _filePath.asString().asASCIICharArray() << std::endl;
    }
    else
    {
        std::cout << "Reading test XML file: " << _filePath.asString().asASCIICharArray() << std::endl;
    }

    GT_IF_WITH_ASSERT(rc)
    {
        // Set the file folder member:
        m_fileFolder = _filePath;
        m_fileFolder.clearFileExtension().clearFileName().reinterpretAsDirectory();

        TiXmlDocument doc(_filePath.asString().asASCIICharArray());

        rc = doc.LoadFile();

        if (rc)
        {
            TiXmlHandle docHandle(&doc);
            rc = readTestsSettings(docHandle);

            // Verify log files directory
            osDirectory logDir(_logFilesDirectory);

            if (logDir.exists())
            {
                std::cout << "Log files directory exists: " << _logFilesDirectory.asString().asASCIICharArray() << std::endl;
            }
            else
            {
                std::cout << "Creating log files directory: " << _logFilesDirectory.asString().asASCIICharArray() << std::endl;

                if (!logDir.create())
                {
                    std::cout << "Failed to create log files directory: " << _logFilesDirectory.asString().asASCIICharArray() << std::endl;
                }
            }

            // Verify log file directory
            osDirectory resultsDir(_testResultsDirectory);

            if (resultsDir.exists())
            {
                std::cout << "Result files directory exists: " << _testResultsDirectory.asString().asASCIICharArray() << std::endl;
            }
            else
            {
                std::cout << "Creating result files directory: " << _testResultsDirectory.asString().asASCIICharArray() << std::endl;

                if (!resultsDir.create())
                {
                    std::cout << "Failed to create result files directory: " << _testResultsDirectory.asString().asASCIICharArray() << std::endl;
                }
            }
        }
    }
    return rc;
}

// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readTestsSettings
// Description: Read the tests settings
// Arguments:   TiXmlHandle& docHandle
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        1/12/2011
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readTestsSettings(TiXmlHandle& docHandle)
{
    // Get the Tests Node
    TiXmlNode* pTestsNode = docHandle.FirstChild(AT_STR_loadTestsCodeXLNode).FirstChild(AT_STR_loadTestsTestsNode).Node();
    TiXmlElement* pElement = NULL;
    gtString currentNodeValue;

    bool retVal = false;

    if (pTestsNode != NULL)
    {
        retVal = true;

        // Base paths for the gold masters folder:
        gtVector<gtString> goldMastersFolderBasePaths;
        atGenerateBaseFilePathList(AT_STR_GOLD_MASTER_FOLDER_ENV_VARNAME, false, goldMastersFolderBasePaths);
        goldMastersFolderBasePaths.push_back(m_fileFolder.asString(true));

        // Base paths for sample executables:
        gtVector<gtString> testExecutablesFolderBasePaths;
        atGenerateBaseFilePathList(AT_STR_TEST_EXECUTABLE_FOLDER_ENV_VAR_NAME, true, testExecutablesFolderBasePaths);
        testExecutablesFolderBasePaths.push_back(m_fileFolder.asString(true));

        // Adding the Tests from the XML file to the:
        for (TiXmlNode* pTestsParameterNode = pTestsNode->FirstChild(); pTestsParameterNode != NULL; pTestsParameterNode = pTestsParameterNode->NextSibling())
        {
            // Get the current test parameter node value:
            currentNodeValue.fromASCIIString(pTestsParameterNode->Value());

            // Get the TestResults value:
            if (currentNodeValue.isEqual(AT_STR_loadTestsTestResultsDirectoryNode))
            {
                pElement = pTestsParameterNode->ToElement();

                if (pElement != NULL)
                {
                    // Try to match the gold masters folder:
                    currentNodeValue.fromASCIIString(pElement->GetText());
                    bool rcMatch = atMatchFilePathToBasePaths(currentNodeValue, goldMastersFolderBasePaths, _testResultsDirectory);
                    _testResultsDirectory.reinterpretAsDirectory();

                    if (rcMatch)
                    {
                        rcMatch = _testResultsDirectory.isDirectory();
                    }

                    if (!rcMatch)
                    {
                        // Just take the string as-is and hope for the best:
                        _testResultsDirectory.setFullPathFromString(currentNodeValue).reinterpretAsDirectory();
                    }
                }
            }

            // Get the LogFilesDirectory value:
            else if (currentNodeValue.isEqual(AT_STR_loadTestsLogFilesDirectoryNode))
            {
                pElement = pTestsParameterNode->ToElement();

                if (pElement != NULL)
                {
                    gtString logPath = currentNodeValue.fromASCIIString(pElement->GetText());

                    // Search for slash and backslash to cover any format of the path in the XML file
                    // I don't use osFilePath::osPathSeparator because the user may place the wrong type
                    // of slash in the XML file.
                    if (-1 == logPath.find(L'\\') && -1 == logPath.find(L'/'))
                    {
                        // Add the current working directory:
                        osFilePath absoluteLogPath(osFilePath::OS_CURRENT_DIRECTORY);
                        absoluteLogPath.appendSubDirectory(logPath);
                        absoluteLogPath.reinterpretAsDirectory();
                        logPath = absoluteLogPath.asString();
                    }

                    _logFilesDirectory.setFullPathFromString(logPath);
                    _logFilesDirectory.reinterpretAsDirectory();
                }
            }

            else if (currentNodeValue.isEqual(AT_STR_loadTestsTestNode))
            {
                // Read the current test:
                bool rc = readSingleTest(pTestsParameterNode, testExecutablesFolderBasePaths);
                GT_ASSERT(rc);

                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readSingleTest
// Description: Read a single test parameters from the XML
// Arguments:   TiXmlNode* pSingleTestNode
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/1/2012
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readSingleTest(TiXmlNode* pSingleTestNode, const gtVector<gtString>& testExecutablesFolderBasePaths)
{
    bool retVal = false;
    TiXmlElement* pElement = NULL;

    if (pSingleTestNode != NULL)
    {
        // Create the current test data object:
        atTestData* pCurrentXMLCreationData = new atTestData;


        // Set the general settings for the current XML data:
        pCurrentXMLCreationData->m_xmlFilePath = _filePath;
        pCurrentXMLCreationData->m_xmlFileFolder = m_fileFolder;
        pCurrentXMLCreationData->_GoldMastersFolder = _testResultsDirectory;
        pCurrentXMLCreationData->_logFilesDirectoryPath = _logFilesDirectory;

        gtString currentNodeValue;

        // Walk on all the Settings and Tests types nodes:
        for (TiXmlNode* pTestParameterNode = pSingleTestNode->FirstChild(); pTestParameterNode != NULL; pTestParameterNode = pTestParameterNode->NextSibling())
        {
            // Get the current test parameter node value:
            currentNodeValue.fromASCIIString(pTestParameterNode->Value());

            // Get the ExecutablePath value
            if (currentNodeValue.isEqual(AT_STR_loadTestsExecutablePathNode))
            {
                pElement = pTestParameterNode->ToElement();

                if (pElement != NULL)
                {
                    currentNodeValue.fromASCIIString(pElement->GetText());

                    bool rcMatch = atMatchFilePathToBasePaths(currentNodeValue, testExecutablesFolderBasePaths, pCurrentXMLCreationData->_executablePath);

                    if (!rcMatch)
                    {
                        // Just take the string as-is and hope for the best:
                        pCurrentXMLCreationData->_executablePath.setFullPathFromString(currentNodeValue);
                    }
                }
            }

            // Get the TestName value
            else if (currentNodeValue.isEqual(AT_STR_loadTestsTestName))
            {
                pElement = pTestParameterNode->ToElement();

                if (pElement != NULL)
                {
                    pCurrentXMLCreationData->_testName.fromASCIIString(pElement->GetText());
                }
            }

            // Get the Test's Type
            else if (currentNodeValue.isEqual(AT_STR_loadTestsTestType))
            {
                pElement = pTestParameterNode->ToElement();

                if (pElement != NULL)
                {
                    pCurrentXMLCreationData->_testType.fromASCIIString(pElement->GetText());
                }
            }

            // Get the KernelName value
            else if (currentNodeValue.isEqual(AT_STR_loadTestsKernelsNode))
            {
                // Read current kernels:
                bool rc = readCurrentTestKernels(pCurrentXMLCreationData, pTestParameterNode);
                GT_ASSERT(rc);
            }

            // Get the WorkingDirectory value
            else if (currentNodeValue.isEqual(AT_STR_loadTestsWorkingDirectoryNode))
            {
                pElement = pTestParameterNode->ToElement();

                if (pElement != NULL)
                {
                    // Try to match the gold masters folder:
                    currentNodeValue.fromASCIIString(pElement->GetText());
                    bool rcMatch = atMatchFilePathToBasePaths(currentNodeValue, testExecutablesFolderBasePaths, pCurrentXMLCreationData->_workingDirectory);
                    pCurrentXMLCreationData->_workingDirectory.reinterpretAsDirectory();

                    if (rcMatch)
                    {
                        rcMatch = pCurrentXMLCreationData->_workingDirectory.isDirectory();
                    }

                    if (!rcMatch)
                    {
                        // Just take the string as-is and hope for the best:
                        pCurrentXMLCreationData->_workingDirectory.setFullPathFromString(currentNodeValue).reinterpretAsDirectory();
                    }
                }
            }
        }

        atPrintableTestData printableTestData(pCurrentXMLCreationData);
        // Add the current data to the vector:
        _testsVector.push_back(printableTestData);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readCurrentTestKernels
// Description: Read the current test kernels data
// Arguments:   atTestData* pCurrentXMLCreationData
//              TiXmlNode* pSingleTestNode
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/1/2012
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readCurrentTestKernels(atTestData* pCurrentXMLCreationData, TiXmlNode* pKernelsNode)
{
    bool retVal = true;
    gtString currentNodeValue;

    // Sanity check
    GT_IF_WITH_ASSERT((pCurrentXMLCreationData != NULL) && (pKernelsNode != NULL))
    {
        for (TiXmlNode* pSingleKernelNode = pKernelsNode->FirstChild(); pSingleKernelNode != NULL; pSingleKernelNode = pSingleKernelNode->NextSibling())
        {
            // Check if this is a kernel node:
            currentNodeValue.fromASCIIString(pSingleKernelNode->Value());

            if (currentNodeValue.isEqual(AT_STR_loadTestsKernelNode))
            {
                // Allocate a new kernel test object, and push to the vector of tests:
                atTestData::atKernelTest* pNewKernelTest = new atTestData::atKernelTest;

                pCurrentXMLCreationData->_testedKernels.push_back(pNewKernelTest);

                for (TiXmlNode* pKernelAttrNode = pSingleKernelNode->FirstChild(); pKernelAttrNode != NULL; pKernelAttrNode = pKernelAttrNode->NextSibling())
                {
                    // Check the current node value:
                    currentNodeValue.fromASCIIString(pKernelAttrNode->Value());

                    // Read the kernel name
                    if (currentNodeValue.isEqual(AT_STR_loadTestsKernelName))
                    {
                        retVal = readKernelName(pKernelAttrNode, pNewKernelTest) && retVal;
                    }

                    // Read the kernel variables
                    else if (currentNodeValue.isEqual(AT_STR_loadTestsKernelVariables))
                    {
                        retVal = readKernelVarsInfo(pKernelAttrNode, pNewKernelTest) && retVal;
                    }

                    // Read the Locals (in specific lines) that need output to the Log file
                    else if (currentNodeValue.isEqual(AT_STR_loadTestsKernelLocals))
                    {
                        retVal = readLocalsLinesValue(pKernelAttrNode, pNewKernelTest) && retVal;
                    }

                    // Read the line that needs to be stepped into
                    else if (currentNodeValue.isEqual(AT_STR_loadTestsKernelStepIntoLine))
                    {
                        retVal = readStepIntoTest(pKernelAttrNode, pNewKernelTest) && retVal;
                    }

                    // Read the active work item coord:
                    else if (currentNodeValue.isEqual(AT_STR_loadTestsKernelWorkItemIdNode))
                    {
                        TiXmlElement* pElement = pKernelAttrNode->ToElement();

                        if (pElement != NULL)
                        {
                            currentNodeValue.fromASCIIString(pElement->GetText());
                            int firstComma = currentNodeValue.find(',');
                            int secondComma = currentNodeValue.find(',', firstComma + 1);
                            int endOfString = currentNodeValue.length() - 1;
                            GT_IF_WITH_ASSERT((0 < firstComma) && (firstComma < secondComma) && (secondComma < endOfString))
                            {
                                gtString xStr;
                                currentNodeValue.getSubString(0, firstComma - 1, xStr);
                                gtString yStr;
                                currentNodeValue.getSubString(firstComma + 1, secondComma - 1, yStr);
                                gtString zStr;
                                currentNodeValue.getSubString(secondComma + 1, endOfString, zStr);

                                retVal = xStr.toIntNumber(pNewKernelTest->m_workItemCoord[0]) && retVal;
                                retVal = yStr.toIntNumber(pNewKernelTest->m_workItemCoord[1]) && retVal;
                                retVal = zStr.toIntNumber(pNewKernelTest->m_workItemCoord[2]) && retVal;
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readKernelName
// Description:
// Arguments:   TiXmlNode* pKernelAttrNode
//              atTestData::atKernelTest* pNewKernelTest
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        12/6/2012
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readKernelName(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest)
{
    bool retVal = false;
    gtString currentNodeValue;

    TiXmlElement* pElement = pKernelAttrNode->ToElement();

    if (pElement != NULL)
    {
        currentNodeValue.fromASCIIString(pElement->GetText());
        pNewKernelTest->_kernelName = currentNodeValue;
        retVal = !pNewKernelTest->_kernelName.isEmpty();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readKernelVarsInfo
// Description:
// Arguments:   TiXmlNode* pKernelAttrNode
//              atTestData::atKernelTest* pNewKernelTest
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        12/6/2012
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readKernelVarsInfo(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest)
{
    bool retVal = false;
    gtString currentNodeValue;

    for (TiXmlNode* pKernelVariableNode = pKernelAttrNode->FirstChild(); pKernelVariableNode != NULL; pKernelVariableNode = pKernelVariableNode->NextSibling())
    {
        // Check the current node value:
        currentNodeValue.fromASCIIString(pKernelVariableNode->Value());

        // Read the kernel name:
        if (currentNodeValue.isEqual(AT_STR_loadTestsKernelVariable))
        {
            atTestData::atKernelTest::atVarData tempVarData;

            // Variable node structure - <Variable name="b" line="108"></Variable>:
            TiXmlElement* pElement = pKernelVariableNode->ToElement();

            if (pElement != NULL)
            {
                // Get the variable name:
                gtString varName, varLineNumber, varWorkItem;
                tempVarData.varName.fromASCIIString(pElement->Attribute("name"));

                // Get the variable line number:
                varLineNumber.fromASCIIString(pElement->Attribute("line"));
                retVal = varLineNumber.toIntNumber(tempVarData.lineNumber);

                // Get the work items values:
                varWorkItem.fromASCIIString(pElement->Attribute("x"));
                retVal = varWorkItem.toIntNumber(tempVarData.workItem[0]);
                varWorkItem.fromASCIIString(pElement->Attribute("y"));
                retVal = varWorkItem.toIntNumber(tempVarData.workItem[1]);
                varWorkItem.fromASCIIString(pElement->Attribute("z"));
                retVal = varWorkItem.toIntNumber(tempVarData.workItem[2]);

                GT_IF_WITH_ASSERT(retVal)
                {
                    pNewKernelTest->_variablesToRead.push_back(tempVarData);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readLocalsLinesValue
// Description:
// Arguments:   TiXmlNode* pKernelAttrNode
//              atTestData::atKernelTest* pNewKernelTest
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        12/6/2012
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readLocalsLinesValue(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest)
{
    bool retVal = false;
    gtString currentNodeValue;

    for (TiXmlNode* pKernelVariableNode = pKernelAttrNode->FirstChild(); pKernelVariableNode != NULL; pKernelVariableNode = pKernelVariableNode->NextSibling())
    {
        // Check the current node value:
        currentNodeValue.fromASCIIString(pKernelVariableNode->Value());

        // Read the kernel name:
        if (currentNodeValue.isEqual(AT_STR_loadTestsKernelLocalsInLine))
        {
            atTestData::atKernelTest::atVarData tempVarData;

            // Locals node structure - <LocalsInLine lineNumber="145"></LocalsInLine>:
            TiXmlElement* pElement = pKernelVariableNode->ToElement();

            if (pElement != NULL)
            {
                // Get the line number:
                gtString varLineNumber;
                varLineNumber.fromASCIIString(pElement->Attribute("lineNumber"));
                retVal = varLineNumber.toIntNumber(tempVarData.lineNumber);
                GT_IF_WITH_ASSERT(retVal)
                {
                    pNewKernelTest->_localsInLine.push_back(tempVarData.lineNumber);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atLoadTestsCommand::readStepIntoTest
// Description:
// Arguments:   TiXmlNode* pKernelAttrNode
//              atTestData::atKernelTest* pNewKernelTest
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        12/6/2012
// ---------------------------------------------------------------------------
bool atLoadTestsCommand::readStepIntoTest(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest)
{
    bool retVal = false;
    gtString currentNodeValue;

    TiXmlElement* pElement = pKernelAttrNode->ToElement();

    if (pElement != NULL)
    {
        currentNodeValue.fromASCIIString(pElement->GetText());
        retVal = currentNodeValue.toIntNumber(pNewKernelTest->m_stepIntoLine);
    }

    return retVal;

}