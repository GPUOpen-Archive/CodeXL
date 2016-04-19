//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atTestsLogCommand.h
///
//==================================================================================

//------------------------------ atTestsLogCommand.h ------------------------------

#ifndef __ATTESTSLOGCOMMAND
#define __ATTESTSLOGCOMMAND

// Forward declarations:
class gtASCIIString;

// infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <inc/atStringConstants.h>

//TinyXml
#include <tinyxml.h>

// ----------------------------------------------------------------------------------
// Class Name:          atTestsLogCommand
// General Description:
// Log tests Output - Log the tests output into a .xml file.
// Author:              Merav Zanany
// Creation Date:       25/12/2011
// ----------------------------------------------------------------------------------
class atTestsLogCommand
{
public:
    atTestsLogCommand(const osFilePath& fileName);
    ~atTestsLogCommand();

    bool startTestLog(gtString execName, gtString kernelName);
    bool logTestInfo(gtString testInfo);
    bool writeToLogFile(const gtVector<gtString>& logMessages);

private:
    osFilePath _filePath;

    bool initFileStructure(TiXmlDocument& xmlDoc);
    bool setTestData(TiXmlHandle& docHandle, gtASCIIString executablePath, gtASCIIString kernelName);
};

#endif  // __ATTESTSLOGCOMMAND
