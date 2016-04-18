//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atKernelDebuggingStepTest.cpp
///
//==================================================================================

//------------------------------ atKernelDebuggingStepTest.cpp ------------------------------

// Standard C++:
#include <iostream>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <inc/atEventObserver.h>
#include <inc/atKernelDebuggingStepTest.h>
#include <inc/atStringConstants.h>

#define AT_TESTER_PROCESS_WAIT_TIME_MS 5000


// ---------------------------------------------------------------------------
// Name:        atKernelDebuggingStepTest::atKernelDebuggingStepTest
// Description: Constructor.
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
atKernelDebuggingStepTest::atKernelDebuggingStepTest(const atTestData* pTestData) : m_pTestData(pTestData)
{
}


// ---------------------------------------------------------------------------
// Name:        atKernelDebuggingStepTest::~atKernelDebuggingStepTest
// Description: Destructor.
// Author:      Merav Zanany
// Date:        29/11/2011
// ---------------------------------------------------------------------------
atKernelDebuggingStepTest::~atKernelDebuggingStepTest()
{
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        execute
/// \brief Description: Performs the kernel stepping test.
/// \return void
/// -----------------------------------------------------------------------------------------------
void atKernelDebuggingStepTest::execute()
{
    apEventsHandler& theEventsHandler = apEventsHandler::instance();
    atEventObserver& theATEventObserver = atEventObserver::instance();

    theATEventObserver.resetProcessFlags();

    // Put a breakpoint on the desired kernel:
    putBreakpointsOnKernels();

    // Launch the debugged process:
    launchExecutableForDebugSession();

    // The above function is expected to abort the test if failed, so we can assume the launch succeeded.
    // Now wait for the process to be created:
    bool processExists = osWaitForFlagToTurnOn(theATEventObserver.wasProcessCreatedFlag(), AT_TESTER_PROCESS_WAIT_TIME_MS);
    ASSERT_TRUE(gaDebuggedProcessExists());
    ASSERT_TRUE(processExists);

    while (processExists)
    {
        osWaitForFlagToTurnOff(theATEventObserver.waitingForEvents(), ULONG_MAX);

        theATEventObserver.beforeAccessingEventsFlag();

        bool rcEventsHandled = theEventsHandler.handlePendingDebugEvent();

        if (rcEventsHandled && theEventsHandler.areNoEventsPending())
        {
            atEventObserver::instance().setPendingDebugEvents(false);
        }

        theATEventObserver.afterAccessingEventsFlag();
        processExists = !theATEventObserver.hasProcessEndedFlag();
    }

    GT_ASSERT(!gaDebuggedProcessExists());

    // Write the test output to the output file:
    writeBreakpointsIntoResultsFile();

    // After the test was done, compare the gold master (pre-defined test result) to the actual result:
    checkTestResults();
}

// ---------------------------------------------------------------------------
// Name:        atKernelDebuggingStepTest::writeBreakpointsIntoResultsFile
// Description: Insert the lines hit into the XML log file
// Return Val:  void
// Author:      Merav Zanany
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void atKernelDebuggingStepTest::writeBreakpointsIntoResultsFile()
{
    atEventObserver& theATEventObserver = atEventObserver::instance();

    // Sanity check
    GT_IF_WITH_ASSERT(m_pTestData != NULL)
    {
        // Open the Log file:
        osFilePath logFile;
        m_pTestData->getTestOutputPath(logFile);

        osDirectory dir;
        logFile.getFileDirectory(dir);

        // If the file exists, remove it:
        if (logFile.exists())
        {
            std::cout << "Deleting old results file: " << logFile.asString().asASCIICharArray() << std::endl;
            gtString fileName;
            logFile.getFileNameAndExtension(fileName);
            dir.deleteFile(fileName);
        }

        atTestsLogCommand testsLog(logFile);

        // Add the breakpoints lines to the log
        std::cout << "Writing results to file:   " << logFile.asString().asASCIICharArray() << std::endl;

        const gtVector<gtString>& testLogStrings = theATEventObserver.getTestLogStrings();
        testsLog.writeToLogFile(testLogStrings);

        // Clear the BPs vector
        theATEventObserver.clearTestLogStrings();
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        putBreakpointsOnKernels
/// \brief Description: Puts breakpoints on the kernels to be stepped
/// \return void
/// -----------------------------------------------------------------------------------------------
void atKernelDebuggingStepTest::putBreakpointsOnKernels()
{
    // Create breakpoints:
    for (int i = 0 ; i < (int) m_pTestData->_testedKernels.size(); i++)
    {
        // Get the current kernel test:
        atTestData::atKernelTest* pCurrentKernelTest = m_pTestData->_testedKernels[i];

        if (pCurrentKernelTest != NULL)
        {
            gtString kernelFunctionName(pCurrentKernelTest->_kernelName);
            apKernelFunctionNameBreakpoint funcNameBreakpoint(kernelFunctionName);
            bool setBreakpointSuccessfully = gaSetBreakpoint(funcNameBreakpoint);
            ASSERT_TRUE(setBreakpointSuccessfully);
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        launchExecutableForDebugSession
/// \brief Description: Launches the executable for a debug session
/// \return void
/// -----------------------------------------------------------------------------------------------
void atKernelDebuggingStepTest::launchExecutableForDebugSession()
{
    // Get the current executable path:
    osFilePath currentExePath;
    bool isValidCurrentPath = osGetCurrentApplicationPath(currentExePath);
    ASSERT_TRUE(isValidCurrentPath);

    // Calculate the "debugger" path:
    osDirectory debuggerPath;
    bool isValidDebuggerPath = currentExePath.getFileDirectory(debuggerPath);
    ASSERT_TRUE(isValidDebuggerPath);

    // Let the event observer know the current debugged executable name:
    gtString exeName;
    osFilePath executablePath = m_pTestData->_executablePath;
    osFilePath workingDirectory = m_pTestData->_workingDirectory;
    executablePath.getFileName(exeName);

    bool isExePathExist = executablePath.exists();
    EXPECT_TRUE(isExePathExist) << L"Test executable was not found: ", executablePath.asString().asCharArray();

    bool isWorkingDirExist = workingDirectory.exists();
    EXPECT_TRUE(isWorkingDirExist) << L"Test working directory was not found: " << workingDirectory.fileDirectoryAsString().asCharArray();

    atEventObserver::instance().setCurrentTest(m_pTestData);

    // Deduce the spies directory from the installation path:
    osFilePath spiesDirectory = debuggerPath.directoryPath();
    bool is64bit = false;
    bool isIdentifiedArchitectureOK = osIs64BitModule(executablePath, is64bit);
    ASSERT_TRUE(isIdentifiedArchitectureOK);

    if (is64bit)
    {
        spiesDirectory.appendSubDirectory(OS_SPIES_64_SUB_DIR_NAME);
    }
    else
    {
        spiesDirectory.appendSubDirectory(OS_SPIES_SUB_DIR_NAME);
    }

    int frameTerminators = 65537;
    apDebugProjectSettings processCreationData(
        m_pTestData->_testName,
        executablePath,
        m_pTestData->_commandLineArguments,
        debuggerPath.directoryPath(),
        workingDirectory,
        spiesDirectory,
        frameTerminators,
        AP_PNG_FILE,
        4000000,
        50000,
        2000,
        false,
        false);

    // Set the log folder:
    osFilePath logDir = m_pTestData->_logFilesDirectoryPath;

    if (logDir.asString().isEmpty())
    {
        logDir.setPath(osFilePath::OS_TEMP_DIRECTORY);

        if (logDir.asString().isEmpty())
        {
            logDir.setPath(osFilePath::OS_CURRENT_DIRECTORY);
        }
    }

    processCreationData.setLogFilesFolder(logDir);

    // Get the execution target:
    //apProjectExecutionTarget projExecTarget = processCreationData.projectExecutionTarget();

    wprintf(L"Launching executable for debugging session: %s\n", processCreationData.executablePath().asString().asCharArray());
    // Launch the debugged process:
    bool isDebuggedProcessLaunchedOK = gaLaunchDebuggedProcess(processCreationData, false);
    ASSERT_TRUE(isDebuggedProcessLaunchedOK);

    // After launching the debugged process, set the kernel source code file paths:
    bool isSetKernelSourceFilePathsOK = setKernelSourceFilePaths(processCreationData);
    ASSERT_TRUE(isSetKernelSourceFilePathsOK);
}

// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::setKernelSourceFilePaths
// Description: Set the kernel source code file paths
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
bool atKernelDebuggingStepTest::setKernelSourceFilePaths(const apDebugProjectSettings& processCreationData)
{
    bool retVal = true;

    if (!processCreationData.SourceFilesDirectories().isEmpty())
    {
        gtString currentDir;
        gtStringTokenizer tokenizer(processCreationData.SourceFilesDirectories(), L";");

        while (tokenizer.getNextToken(currentDir))
        {
            // Get the cl files within the specified folder in the process creation data:
            gtList<osFilePath> filePathsList;
            osDirectory dir(currentDir);
            bool rc = dir.getContainedFilePaths(L"*.cl", osDirectory::SORT_BY_NAME_ASCENDING, filePathsList);

            if (rc)
            {
                // Copy the paths to a vector:
                gtVector<osFilePath> kernelSources;
                gtList<osFilePath>::const_iterator iter = filePathsList.begin();
                gtList<osFilePath>::const_iterator iterEnd = filePathsList.end();

                for (; iter != iterEnd; iter++)
                {
                    kernelSources.push_back(*iter);
                }

                // Update the spy with the source file paths:
                rc = gaSetKernelSourceFilePath(kernelSources);
                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        atKernelDebuggingStepTest::checkTestResults
// Description: Compare the test result to the base one
// Arguments:   int testIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
void atKernelDebuggingStepTest::checkTestResults()
{
    // Build the file paths for the base result file and for the actual log file:
    osFilePath goldMaster;
    osFilePath actualLogFile;
    m_pTestData->getTestOutputPath(actualLogFile);

    goldMaster.setFileDirectory(m_pTestData->_GoldMastersFolder);
    goldMaster.setFileName(m_pTestData->_testName);

    goldMaster.setFileExtension(L"log");

    // Make sure reference file exists
    bool isGoldMasterExist = goldMaster.exists();
    ASSERT_TRUE(isGoldMasterExist) << "Gold Master file is missing: " << goldMaster.asString().asASCIICharArray() << "\n";

    // Run a Diff process to compare both files:
    bool isFileCompareOK = diffFiles(actualLogFile.asString(), goldMaster.asString());
    ASSERT_TRUE(isFileCompareOK);
}


// ---------------------------------------------------------------------------
// Name:        atKernelDebuggingStepTest::diffFiles
// Description: Uses the fc command to compare two files it is given
// Arguments:   gtString newLogFile
//              gtString referenceLogFile
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        4/1/2012
// ---------------------------------------------------------------------------
bool atKernelDebuggingStepTest::diffFiles(const gtString& newLogFile, const gtString& referenceLogFile)
{
    bool retVal = false;

    // Create the fc command to execute. Surround the parameter file paths with quotes to correctly handle paths with spaces in them
    gtString command;
    command.appendFormattedString(L"fc /U /W \"%ls\" \"%ls\"", newLogFile.asCharArray(), referenceLogFile.asCharArray());

    // Compare the log file to the ref file
    int sysVal;
    sysVal = system(command.asASCIICharArray());

    if (sysVal == 0)
    {
        retVal = true;
    }

    return retVal;
}



