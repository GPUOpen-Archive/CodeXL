//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atGpuDebuggingTest.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>

/// Local:
#include <inc/atGpuDebuggingTest.h>
#include <inc/atKernelDebuggingStepTest.h>

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        SetUp
/// \brief Description:
/// \return void
/// -----------------------------------------------------------------------------------------------
void atGpuDebuggingTest::SetUp()
{

}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        TearDown
/// \brief Description:
/// \return void
/// -----------------------------------------------------------------------------------------------
void atGpuDebuggingTest::TearDown()
{

}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        addInstallFolderToPathEnvVariable
/// \brief Description: Add the XML install directory to the path environment variable
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool atGpuDebuggingTest::addInstallFolderToPathEnvVariable()
{
    bool retVal = false;
    // Get the current process path:
    gtString currentProcessPath;
    bool rc = osGetCurrentProcessEnvVariableValue(L"PATH", currentProcessPath);
    GT_IF_WITH_ASSERT(rc)
    {
        // Get the current executable path:
        osFilePath currentExePath;
        bool rcExePath = osGetCurrentApplicationPath(currentExePath);
        GT_IF_WITH_ASSERT(rcExePath)
        {
            // Calculate the "debugger" path:
            osDirectory debuggerPath;
            bool rcDebuggerPath = currentExePath.getFileDirectory(debuggerPath);
            GT_IF_WITH_ASSERT(rcDebuggerPath)
            {
                // Build a new path that contains the installation directory:
                gtString newProcessPath = debuggerPath.directoryPath().asString();
                newProcessPath += L"\\;";
                newProcessPath += currentProcessPath;

                // Set the current process path to be the new path:
                osEnvironmentVariable pathEnvVariable;
                pathEnvVariable._name = L"PATH";
                pathEnvVariable._value = newProcessPath;
                retVal = osSetCurrentProcessEnvVariable(pathEnvVariable);
            }
        }
    }
    return retVal;
}

// Define a parameterized test that takes a parameter containing the test data
TEST_P(atGpuDebuggingTest, KernelDebugging)
{
    atPrintableTestData printableTestData = GetParam();
    const atTestData* pTestData = printableTestData.GetData();

    // Log the test name and type to the XML report that GoogleTest generates
    //RecordProperty("TestName", pTestData->_testName.asASCIICharArray());
    //RecordProperty("TestType", pTestData->_testType.asASCIICharArray());

    // Perform the test case
    atKernelDebuggingStepTest stepTest(pTestData);
    stepTest.execute();
}