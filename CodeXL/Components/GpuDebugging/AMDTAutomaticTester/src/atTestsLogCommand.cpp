//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atTestsLogCommand.cpp
///
//==================================================================================

//------------------------------ atTestsLogCommand.cpp ------------------------------
#include <iostream>

// Infra
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <inc/atTestsLogCommand.h>

// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::atTestsLogCommand
// Description: Constructor
// Arguments:   const osFilePath& fileName
// Author:      Merav Zanany
// Date:        25/12/2011
// ---------------------------------------------------------------------------
atTestsLogCommand::atTestsLogCommand(const osFilePath& fileName):
    _filePath(fileName)
{}

// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::~atTestsLogCommand
// Description: Destructor
// Author:      Merav Zanany
// Date:        25/12/2011
// ---------------------------------------------------------------------------
atTestsLogCommand::~atTestsLogCommand() {}



// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::initFileStructure
// Description: Initialize the Log file structure
// Arguments:   TiXmlDocument& xmlDoc
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        26/12/2011
// ---------------------------------------------------------------------------
bool atTestsLogCommand::initFileStructure(TiXmlDocument& xmlDoc)
{
    bool rc = true;

    const char* pTestsOutput =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<" AT_STR_loadTestsCodeXLNode ">\n"
        "<" AT_STR_loadTestsTestsOutputNode ">\n"
        "</" AT_STR_loadTestsTestsOutputNode ">\n"
        "</" AT_STR_loadTestsCodeXLNode ">\n";

    xmlDoc.Parse(pTestsOutput);

    if (xmlDoc.Error())
    {
        rc = false;
    }

    return rc;
}



// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::startTestLog
// Description: Start the current test's log
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        26/12/2011
// ---------------------------------------------------------------------------
bool atTestsLogCommand::startTestLog(gtString execName, gtString kernelName)
{
    bool rc = false;

    // Create the XML document:
    TiXmlDocument doc(_filePath.asString().asASCIICharArray());

    // Initialize the file structure:
    rc = initFileStructure(doc);

    // Create XML document handle
    TiXmlHandle docHandle(&doc);

    // Add the tests' info into the file, executable name and kernel name
    rc = rc && setTestData(docHandle, execName.asASCIICharArray(), kernelName.asASCIICharArray());

    // Save the file into the disk
    rc = rc && doc.SaveFile();

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::setTestData
// Description: Insert the test's executable path and kernel name into the XML
// Arguments:   TiXmlHandle& docHandle
//              gtASCIIString executablePath
//              gtASCIIString kernelName
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        26/12/2011
// ---------------------------------------------------------------------------
bool atTestsLogCommand::setTestData(TiXmlHandle& docHandle, gtASCIIString executablePath, gtASCIIString kernelName)
{
    bool rc = false;

    TiXmlNode* pXmlNode = NULL;
    pXmlNode = docHandle.FirstChild(AT_STR_loadTestsCodeXLNode).FirstChild(AT_STR_loadTestsTestsOutputNode).Node();

    if (pXmlNode)
    {
        TiXmlElement Test(AT_STR_loadTestsTestNode);

        // Add the Test node
        TiXmlNode* pTestNode = NULL;
        pTestNode = pXmlNode->InsertEndChild(Test);

        if (pTestNode)
        {
            // Add the executable path info
            TiXmlElement execPath(AT_STR_loadTestsExecutablePathNode);
            execPath.LinkEndChild(new TiXmlText(executablePath.asCharArray()));
            pTestNode->InsertEndChild(execPath);

            // Add the kernel name info
            TiXmlElement kernel(AT_STR_loadTestsExecutablePathNode);
            kernel.LinkEndChild(new TiXmlText(kernelName.asCharArray()));
            pTestNode->InsertEndChild(kernel);

            rc = true;
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::logTestInfo
// Description: Insert test's info messages (test started, succeeded/failed)
// Arguments:   gtString testInfo
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        26/12/2011
// ---------------------------------------------------------------------------
bool atTestsLogCommand::logTestInfo(gtString testInfo)
{
    bool rc = false;

    // Create XML document handle
    TiXmlDocument doc;
    doc.LoadFile(_filePath.asString().asASCIICharArray(), TIXML_ENCODING_UTF8);
    TiXmlHandle docHandle(&doc);

    // Get the root node
    TiXmlNode* pXmlNode = NULL;
    pXmlNode = docHandle.FirstChild(AT_STR_loadTestsCodeXLNode).FirstChild(AT_STR_loadTestsTestsOutputNode).Node();

    if (pXmlNode)
    {
        // Get last Test node
        TiXmlNode* pTestNode = NULL;
        pTestNode = pXmlNode->LastChild();

        if (pTestNode)
        {
            TiXmlElement info("TestInfo");

            // Add the TestInfo text
            info.LinkEndChild(new TiXmlText(testInfo.asASCIICharArray()));

            // Insert a TestInfo node
            pTestNode->InsertEndChild(info);
        }
    }

    rc = doc.SaveFile();
    GT_ASSERT(rc);

    return rc;
}



// ---------------------------------------------------------------------------
// Name:        atTestsLogCommand::writeToLogFile
// Description: Insert the log messages into the output file
// Arguments:   const gtVector<gtString>& logMessages
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
bool atTestsLogCommand::writeToLogFile(const gtVector<gtString>& logMessages)
{
    bool rc = false;

    // Write the strings into a file:
    osFile fileForOutput(_filePath);
    bool isFileOpen = fileForOutput.open(osFile::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (!isFileOpen)
    {
        int errNum = GetLastError();
        std::cout << "Failed to open file for results write. Error = " << errNum << ". File path = " << fileForOutput.path().asString().asASCIICharArray() << std::endl;
    }

    // Write the logging line into the file
    for (int i = 0 ; i < (int)logMessages.size(); i++)
    {
        fileForOutput.writeString(logMessages[i]);
        fileForOutput.writeString(L"\n");
    }

    fileForOutput.flush();
    fileForOutput.close();

    return rc;
}