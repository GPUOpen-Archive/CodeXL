//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atMainFunction.cpp
///
//==================================================================================

//------------------------------ atMainFunction.cpp ------------------------------

// Standard C++:
#include <iostream>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>


// Local:
#include <inc/atEventObserver.h>
#include <inc/atInitializeInfrastructureCommand.h>
#include <inc/atGpuDebuggingTest.h>
#include <inc/atStringConstants.h>
#include <inc/atTestsDataVector.h>
#include <inc/atUtils.h>

//TinyXml
#include <tinyxml.h>

// Need a global container to hold the tests data so the INSTANTIATE_TEST_CASE_P macro can use it
atTestsDataVector* g_pTestsDataVector = NULL;

// ---------------------------------------------------------------------------
// Name:        main
// Description: Main function for the automatic tester executable
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    gtVector<osFilePath> testsXMLPaths;

    gtVector<gtString> xmlPathsBasePaths;
    atGenerateBaseFilePathList(AT_STR_XML_FOLDER_ENV_VAR_NAME, false, xmlPathsBasePaths);

    for (int i = 1; i < argc; i++)
    {
        gtString currentArg;
        currentArg.fromASCIIString((const char*)argv[i]);

        if ((currentArg == L"/h") || (currentArg == L"/?") || (currentArg == L"/help"))
        {
            std::wcout << L"Runs an automatic tests of CodeXL debugger component\n";
            std::wcout << L"The tests data XML file " << TESTS_DATA_XML_FILE_NAME << L" must be present in the same folder as the test executable\n";
            return -1;
        }
        else if (!currentArg.startsWith(L"-"))
        {
            // Consider any other parameter as an XML file path:
            osFilePath currentArgAsPath;
            bool rcMatched = atMatchFilePathToBasePaths(currentArg, xmlPathsBasePaths, currentArgAsPath);

            if (rcMatched)
            {
                testsXMLPaths.push_back(currentArgAsPath);
            }
        }
    }

    // Set up the tests from XML file(s) (if none are specified, use the default):
    int numberOfTestXMLs = (int)testsXMLPaths.size();

    if (1 > numberOfTestXMLs)
    {
        g_pTestsDataVector = new atTestsDataVector;
    }
    else
    {
        g_pTestsDataVector = new atTestsDataVector(testsXMLPaths[0].asString());

        for (int i = 1; i < numberOfTestXMLs; i++)
        {
            g_pTestsDataVector->addTestsFromXMLFile(testsXMLPaths[i].asString());
        }
    }

    ::testing::InitGoogleTest(&argc, argv);

    // Initialize events observer:
    atEventObserver::instance();

    // Initialize the infrastructure:
    atInitializeInfrastructureCommand initInfraCmd;
    bool isInitInfraOk = initInfraCmd.execute();
    ASSERT_TRUE(isInitInfraOk);

    // Add the XML install folder to PATH:
    bool isModifiedPathEnvVarOK = atGpuDebuggingTest::addInstallFolderToPathEnvVariable();
    ASSERT_TRUE(isModifiedPathEnvVarOK);

    // Set the log level to DEBUG
    osDebugLog::instance().setLoggedSeverity(OS_DEBUG_LOG_DEBUG);

    // Run the test cases
    return RUN_ALL_TESTS();
}

INSTANTIATE_TEST_CASE_P(CodeXL,
                        atGpuDebuggingTest,
                        ::testing::ValuesIn(*g_pTestsDataVector));


