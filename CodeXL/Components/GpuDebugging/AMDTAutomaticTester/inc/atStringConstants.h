//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atStringConstants.h
///
//==================================================================================
#ifndef __ATSTRINGCONSTANTS_H
#define __ATSTRINGCONSTANTS_H

#define TESTS_DATA_XML_FILE_NAME L"CodeXLGpuDebuggingTest.xml"

// General strings:
#define AT_AUTOMATIC_TESTER_PRODUCT_NAME L"AMDTAutomaticTester"
#define AT_AUTOMATIC_TESTER_STARTS L"AMDTAutomaticTester starts...\n"
#define AT_STR_LOGMSG_APPINITBEGIN L"Automatic tester initialization started"
#define AT_STR_LOGMSG_APPINITENDED L"Automatic tester initialization ended"
#define AT_STR_LOGMSG_FAILEDTOINITAPI L"Failed to initialize the API package"
#define AT_STR_LOGMSG_APPINITCMDSUCCEEDED L"Application Initialization command succeeded"
#define AT_STR_MAINT_THREAD_NAME "AMDTAutomaticTester - main thread"

// Environment variables:
#define AT_STR_XML_FOLDER_ENV_VAR_NAME L"AT_TEST_XML_FOLDER"
#define AT_STR_TEST_EXECUTABLE_FOLDER_ENV_VAR_NAME L"AT_TEST_EXEC_FOLDER"
#define AT_STR_GOLD_MASTER_FOLDER_ENV_VARNAME L"AT_TEST_GOLD_MASTER_FOLDER"

// Kernel step test:
#define AT_STR_KERNEL_STEP_TEST_BEGINS L"Kernel step test begins"
#define AT_STR_KERNEL_STEP_TEST_ENDED L"Kernel step test ended"
#define AT_STR_KERNEL_STEP_TEST_SUCCEEDED L"Kernel step test succeeded"
#define AT_STR_KERNEL_STEP_TEST_FAILED L"Kernel step test failed"

// Tests XML file:
#define AT_STR_loadTestsfilePath L"C:\\Users\\salgrana\\AppData\\Roaming\\CodeXL\\sampleAutoTest.xml"
#define AT_STR_loadTestsCodeXLNode "CodeXL"
#define AT_STR_loadTestsTestsNode "Tests"
#define AT_STR_loadTestsTestResultsDirectoryNode "TestResultsDirectory"
#define AT_STR_loadTestsTestsOutputNode "TestsOutput"
#define AT_STR_loadTestsTestNode "Test"
#define AT_STR_loadTestsTestName "TestName"
#define AT_STR_loadTestsTestType "TestType"
#define AT_STR_loadTestsExecutablePathNode "ExecutablePath"
#define AT_STR_loadTestsKernelsNode "Kernels"
#define AT_STR_loadTestsKernelNode "Kernel"
#define AT_STR_loadTestsKernelName "KernelName"
#define AT_STR_loadTestsKernelVariable "Variable"
#define AT_STR_loadTestsKernelVariables "KernelVariables"
#define AT_STR_loadTestsKernelLocals "KernelLocals"
#define AT_STR_loadTestsKernelLocalsInLine "LocalsInLine"
#define AT_STR_loadTestsKernelStepIntoLine "StepIntoLine"
#define AT_STR_loadTestsKernelExpectedLine "ExpectedLine"
#define AT_STR_loadTestsKernelWorkItemIdNode "WorkItemId"
#define AT_STR_loadTestsWorkingDirectoryNode "WorkingDirectory"
#define AT_STR_loadTestsLogFilesDirectoryNode "LogFilesDirectory"
#define AT_STR_temporaryLogFilesFolderName L"AMDAutomaticTestResultsTemp"
#endif //__ATSTRINGCONSTANTS_H

