//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atTestData.h
///
//==================================================================================

//------------------------------ atTestData.h ------------------------------

#ifndef __ATTESTDATA_H
#define __ATTESTDATA_H

// Infra:
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>

// Local:
#include <inc/atStringConstants.h>

class atTestData
{
public:

    class atKernelTest
    {
    public:
        atKernelTest() {m_workItemCoord[0] = -1; m_workItemCoord[1] = -1; m_workItemCoord[2] = -1; m_stepIntoLine = -1;};
        ~atKernelTest() {};
    public:
        gtString _kernelName;
        int m_workItemCoord[3];

        // For the Variables tests
        struct atVarData
        {
            gtString varName;
            int lineNumber;
            int workItem[3];
        };
        gtVector<atVarData> _variablesToRead;

        // For the Locals tests
        gtVector<int> _localsInLine;

        // For the Step into test
        int m_stepIntoLine;
    };

    atTestData() = default;
    virtual ~atTestData() = default;

    void getTestOutputPath(osFilePath& outputPath) const
    {
        // Set the base dir:
        outputPath.clear();
        outputPath.setPath(osFilePath::OS_TEMP_DIRECTORY);

        if (outputPath.asString().isEmpty())
        {
            outputPath.setPath(osFilePath::OS_CURRENT_DIRECTORY);
        }

        outputPath.appendSubDirectory(AT_STR_temporaryLogFilesFolderName);

        // Create the log files directory if it doesn't exist:
        osDirectory dir(outputPath.fileDirectoryAsString());
        dir.create();

        // Build the file name:
        outputPath.setFileName(_testName);
        outputPath.setFileExtension(L"log");
    }

    osFilePath m_xmlFilePath;
    osFilePath m_xmlFileFolder;
    osFilePath _executablePath;
    gtString _testName;
    gtString _testType;
    gtPtrVector<atKernelTest*> _testedKernels;
    gtString _commandLineArguments;
    osFilePath _workingDirectory;
    osFilePath _logFilesDirectoryPath;
    osFilePath _GoldMastersFolder;
};

#endif //__ATTESTDATA_H

