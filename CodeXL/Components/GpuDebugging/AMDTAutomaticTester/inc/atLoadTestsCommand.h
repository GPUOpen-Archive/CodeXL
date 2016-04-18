//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atLoadTestsCommand.h
///
//==================================================================================

//------------------------------ atLoadTestsCommand.h ------------------------------

#ifndef __ATLOADTESTSCOMMAND
#define __ATLOADTESTSCOMMAND

// Forward declarations:
class gtASCIIString;

// infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <inc/atStringConstants.h>
#include <inc/atPrintableTestData.h>

//TinyXml
#include <tinyxml.h>

// ----------------------------------------------------------------------------------
// Class Name:          atLoadTestsCommand
// General Description:
// Load tests Command - Load the tests configuration from a .xml file.
// Author:              Merav Zanany
// Creation Date:       1/12/2011
// ----------------------------------------------------------------------------------
class atLoadTestsCommand
{
public:
    atLoadTestsCommand(const osFilePath& fileName, gtVector<atPrintableTestData>& testsVector);
    ~atLoadTestsCommand();

    bool execute();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    atLoadTestsCommand() = delete;
    atLoadTestsCommand(const atLoadTestsCommand&) = delete;
    atLoadTestsCommand& operator=(const atLoadTestsCommand&) = delete;

    bool readTestsSettings(TiXmlHandle& docHandle);
    bool readSingleTest(TiXmlNode* pSingleTestNode, const gtVector<gtString>& testExecutablesFolderBasePaths);
    bool readCurrentTestKernels(atTestData* pCurrentXMLCreationData, TiXmlNode* pSingleTestNode);
    bool readKernelName(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest);
    bool readKernelVarsInfo(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest);
    bool readLocalsLinesValue(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest);
    bool readStepIntoTest(TiXmlNode* pKernelAttrNode, atTestData::atKernelTest* pNewKernelTest);

private:

    osFilePath _filePath;
    osFilePath m_fileFolder;
    osFilePath _logFilesDirectory;
    osFilePath _testResultsDirectory;
    gtVector<atPrintableTestData>& _testsVector;

};

#endif  // __ATLOADTESTSCOMMAND
