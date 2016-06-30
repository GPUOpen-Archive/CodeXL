//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStartDebuggingCommand.cpp
///
//==================================================================================

//------------------------------ gdStartDebuggingCommand.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>


// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osPipeExecutor.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdStartDebuggingCommand.h>

// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::gdStartDebuggingCommand
// Description: Constructor
// Arguments:   processCreationData - The debugged process creation data.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
gdStartDebuggingCommand::gdStartDebuggingCommand(const apDebugProjectSettings& processCreationData)
    : _processCreationData(processCreationData)
{
}

// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::~gdStartDebuggingCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
gdStartDebuggingCommand::~gdStartDebuggingCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::canExecuteSpecificCommand
// Description: Answers the question - can we start debugging a process.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// Implementation Notes:
//   We can start debugging a process iff there is currently no debugged process.
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::canExecuteSpecificCommand()
{
    bool retVal = false;

    // Verify that we are not currently debugging a process:
    bool debuggedProcessExists = gaDebuggedProcessExists();

    if (!debuggedProcessExists)
    {
        retVal = true;
    }

    return retVal;
}

bool gdStartDebuggingCommand::AreDebugSettingsValid(bool& exeFileExist, bool& workingFolderExist, bool& logDirExistAndAccessible, bool& arePortsLegal, bool& arePortsDistinct, bool& requiredSystemLibrariesExists)
{
    bool retVal = false;

    afGlobalVariablesManager& theGlobalVarsMgr = afGlobalVariablesManager::instance();

    // Get the debugged process executable path:
    const osFilePath& exePath = _processCreationData.executablePath();
    exeFileExist = exePath.isExecutable();
    bool isExeTypeSupported = false;
    requiredSystemLibrariesExists = true;

    // Log files folder:
    osFilePath logFilesDirectory = theGlobalVarsMgr.logFilesDirectoryPath();
    logFilesDirectory.reinterpretAsDirectory();
    osDirectory logFilesDir(logFilesDirectory);
    logDirExistAndAccessible = logFilesDirectory.isDirectory() && logFilesDir.isWriteAccessible();

    // Working directory:
    const osDirectory& workingDirectory = _processCreationData.workDirectory();
    workingFolderExist = workingDirectory.exists();

    // If the log files directory does not exist, create it:
    if (!logDirExistAndAccessible)
    {
        osDirectory logFilesDir1(logFilesDirectory.asString());
        logFilesDir1.create();

        logDirExistAndAccessible = logFilesDirectory.isDirectory() && logFilesDir1.isWriteAccessible();
    }

    // Set the log files folder:
    _processCreationData.setLogFilesFolder(logFilesDirectory);

    // Get the frame terminators:
    unsigned int frameTerminators = _processCreationData.frameTerminatorsMask();

    bool oglFrameTerminatorsExist = false;

    if ((frameTerminators & (AP_SWAP_BUFFERS_TERMINATOR | AP_GL_FLUSH_TERMINATOR | AP_GL_FINISH_TERMINATOR | AP_SWAP_LAYER_BUFFERS_TERMINATOR |
                             AP_MAKE_CURRENT_TERMINATOR | AP_GL_CLEAR_TERMINATOR | AP_GL_FRAME_TERMINATOR_GREMEDY)) != 0)
    {
        oglFrameTerminatorsExist = true;
    }
    else
    {
        // Add default frame terminator for GL:
        frameTerminators |= AP_GL_FINISH_TERMINATOR;
        _processCreationData.setFrameTerminators(frameTerminators);
        oglFrameTerminatorsExist = true;
    }

    // Get the remote target settings:
    bool isRemoteTarget = _processCreationData.isRemoteTarget();
    unsigned short rdsPort = theGlobalVarsMgr.getRdsPort();
    unsigned short rdsEvePort = theGlobalVarsMgr.getRdsEventsPort();
    unsigned short spyPort = theGlobalVarsMgr.getSpyPort();
    unsigned short spyEvePort = theGlobalVarsMgr.getSpyEventsPort();

    // Remote Settings:
    arePortsLegal = (0 != rdsPort) && (0 != rdsEvePort) && (0 != spyPort) && (0 != spyEvePort);
    arePortsDistinct = ((rdsPort != rdsEvePort) && (rdsPort != spyPort) && (rdsPort != spyEvePort) && (rdsEvePort != spyPort) && (rdsEvePort != spyEvePort) && (spyPort != spyEvePort));

    if (arePortsLegal && arePortsDistinct)
    {
        _processCreationData.setRemoteTargetConnectionPort(rdsPort);
        _processCreationData.setRemoteTargetEventsPort(rdsEvePort);
        _processCreationData.setRemoteConnectionAPIPort(spyPort);
        _processCreationData.setRemoteConnectionSpiesEventsPort(spyEvePort);
    }

    // If the project type doesn't use OpenGL [ES], we can ignore the case where no OGL frame terminators are defined:
    retVal = oglFrameTerminatorsExist && logDirExistAndAccessible;

    if (!isRemoteTarget)
    {
        // Check if the executable type is supported:
        isExeTypeSupported = exeFileExist && isExecutableTypeSupported(exePath);

        // Check the existence of system libraries required by the debugged process:
        bool testForMissingSystemLibraries = false;

        if (testForMissingSystemLibraries)
        {
            requiredSystemLibrariesExists = checkRequiredSystemLibraries();
        }

        // Verify local settings:
        retVal = retVal && exeFileExist && workingFolderExist && isExeTypeSupported && requiredSystemLibrariesExists;
    }
    else // isRemoteTarget
    {
        // Verify remote settings:
        retVal = retVal && arePortsLegal && arePortsDistinct;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::executeSpecificCommand
// Description: Starts debugging my input process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Check if the current settings are valid:
    bool exeFileExist = false, workingFolderExist = false, logDirExistAndAccessible = false, arePortsLegal = false, arePortsDistinct = false, requiredSystemLibrariesExists = false;
    bool isValidSettings = AreDebugSettingsValid(exeFileExist, workingFolderExist, logDirExistAndAccessible, arePortsLegal, arePortsDistinct, requiredSystemLibrariesExists);

    // Verify that it exists on disk:
    if (isValidSettings)
    {
        // Start the debugging:
        retVal = StartDebugging();
    }
    else
    {
        bool failedOnGlobalOption = false;

        // Handle invalid settings, and check if the failure is for global settings:
        HandleInvalidSettings(exeFileExist, workingFolderExist, logDirExistAndAccessible, arePortsLegal, arePortsDistinct, requiredSystemLibrariesExists, failedOnGlobalOption);

        retVal = true;
    }

    // Stop recording
    // Get the new recorder status:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    bool recording = globalVarsManager.recording();

    if (recording)
    {
        // Start recording:
        gaStartMonitoredFunctionsCallsLogFileRecording();

        // Change the recorder variable to false
        gdGDebuggerGlobalVariablesManager& globalVarsMgr = gdGDebuggerGlobalVariablesManager::instance();
        globalVarsMgr.startRecording();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::isHostMachineConfigurationSupported
// Description:
//  Checks if the host machine's configuration is supported by our process debugger.
//  If not, displays appropriate error messages to the user.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/6/2009
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::isHostMachineConfigurationSupported() const
{
    bool retVal = true;

    // If we are running on Mac:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_VARIANT) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        // If the debugged process is an OpenGL ES process:
        apProjectType debuggedProcessType = _processCreationData.projectType();

        if (apDoesProjectTypeSupportOpenGLES(debuggedProcessType))
        {
            // If the iPhone Simulator is not installed on the local host:
            osFilePath iPhoneSimulatorFilePath(OS_IPHONE_SIMULATOR_APP_PATH);
            bool doesSimulatorExist = iPhoneSimulatorFilePath.isDirectory();

            if (!doesSimulatorExist)
            {
                // Display an error message to the user:
                acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_iPhoneSimulatorIsNotInstalled, QMessageBox::Ok);

                // Fail the application's launch:
                retVal = false;
            }
        }
    }
#endif

    // Local hardware and driver validation are irrelevant for remote debugging:
    if (!_processCreationData.isRemoteTarget())
    {
        gdApplicationCommands* pTheApplicationCommands = gdApplicationCommands::gdInstance();
        GT_IF_WITH_ASSERT(pTheApplicationCommands != NULL)
        {
            // Validate the driver and GPU support AMD and kernel debugging
            pTheApplicationCommands->validateDriverAndGPU();
        }
    }
    else // _processCreationData.isRemoteTarget()
    {
        // Enable kernel debugging if it was disabled before (e.g by a local debugging session:
        bool rcRKD = gaSetKernelDebuggingEnable(true);
        GT_ASSERT(rcRKD);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::adjustSpiesDirectoryToBinaryType
// Description: Adjust the spies directory to match the (Windows) address space size
//              to be either spies or spies64.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        9/8/2009
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::adjustSpiesDirectoryToBinaryType()
{
    bool retVal = false;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    {
        // On windows, we need to check if the debugged application is 64-bit or 32-bit.
        gtString executableFullPath = _processCreationData.executablePath().asString();

        // Get the executable type:
        DWORD executableBinaryType = SCS_32BIT_BINARY;
        BOOL retCode = ::GetBinaryType(executableFullPath.asCharArray(), &executableBinaryType);
        GT_IF_WITH_ASSERT(retCode == (BOOL)TRUE)
        {
            gtString binaryTypeAsString = AF_STR_Unknown;

            switch (executableBinaryType)
            {
                case SCS_32BIT_BINARY:
                {
                    // This is a 32-bit binary. The 32-bit spy is the default, do nothing.
                    retVal = true;
                }
                break;

                case SCS_64BIT_BINARY:
                {
                    // This is a 64-bit binary, change the spies directory:
                    osDirectory spiesDir;
                    retVal = _processCreationData.spiesDirectory().getFileDirectory(spiesDir);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        osFilePath spies64DirAsFilePath = spiesDir.upOneLevel().directoryPath();
                        spies64DirAsFilePath.appendSubDirectory(OS_SPIES_64_SUB_DIR_NAME);
                        _processCreationData.setSpiesDirectory(spies64DirAsFilePath);
                    }

                    retVal = true;
                }
                break;

                case SCS_DOS_BINARY:
                {
                    binaryTypeAsString = L"MS-DOS";
                }
                break;

                case SCS_WOW_BINARY:
                {
                    binaryTypeAsString = L"Windows 16-bit";
                }
                break;

                case SCS_PIF_BINARY:
                {
                    binaryTypeAsString = L"MS-DOS .PIF";
                }
                break;

                case SCS_POSIX_BINARY:
                {
                    binaryTypeAsString = L"POSIX";
                }
                break;

                case SCS_OS216_BINARY:
                {
                    binaryTypeAsString = L"OS/2 16-bit";
                }
                break;

                default:
                {
                    GT_ASSERT_EX(false, L"Unidentified binary type");
                }
                break;
            }

            if (!retVal)
            {
                gtASCIIString errMsg;
                errMsg.appendFormattedString(GD_STR_WindowsSupportedBinaryTypes, binaryTypeAsString.asCharArray());
                acMessageBox::instance().critical(AF_STR_ErrorA, errMsg.asCharArray(), QMessageBox::Ok);
            }
        }
    }
#else
    {
        // Nothing to do...
        retVal = true;
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::isExecutableTypeSupported
// Description: Checks if the input executable debugging is supported.
// Arguments: exePath - The input executable.
// Return Val: bool  - true iff the input executable debugging is supported.
// Author:      Yaki Tebeka
// Date:        5/8/2010
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::isExecutableTypeSupported(const osFilePath& exePath)
{
    GT_UNREFERENCED_PARAMETER(exePath);

    bool retVal = true;

    // If we are on generic Linux:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    {
        // Get the executable architecture:
        gtVector<osModuleArchitecture> moduleArchitectures;
        bool rcArchitecture = osGetModuleArchitectures(exePath, moduleArchitectures);
        GT_IF_WITH_ASSERT(rcArchitecture)
        {
            // Generic Linux supports only one architecture per module:
            GT_IF_WITH_ASSERT(moduleArchitectures.size() == 1)
            {
                osModuleArchitecture moduleArchitecture = moduleArchitectures[0];

#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE)
                {
                    // Debugging 64 bit exe using 32 bit CodeXL is not supported:
                    if (moduleArchitecture == OS_X86_64_ARCHITECTURE)
                    {
                        acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_debugging64bitAppsOnLinuxError, QMessageBox::Ok);
                        retVal = false;
                    }
                }
#elif (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
                {
                    // Debugging 32 bit exe using 64 bit CodeXL is not supported:
                    if (moduleArchitecture == OS_I386_ARCHITECTURE)
                    {
                        acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_debugging32bitAppsOnLinuxError, QMessageBox::Ok);
                        retVal = false;
                    }
                }
#endif
            }
        }
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::checkRequiredSystemLibraries
// Description: Check the existence of system libraries required by the debugged process
// Return Val:  bool  - true - iff the required system libraries (OpenGL/OpenCL/etc) exist.
// Author:      Yaki Tebeka
// Date:        4/8/2010
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::checkRequiredSystemLibraries() const
{
    bool retVal = true;

    gtString missingSystemLibraries;
    bool sysLibsExist = gaSystemLibrariesExists(_processCreationData, missingSystemLibraries);

    if (!sysLibsExist)
    {

        gtASCIIString errMsg = GD_STR_missingSystemLibrariesPrefix;

        if (missingSystemLibraries.find(L"\n") >= 0)
        {
            errMsg += AF_STR_NewLineA;
        }

        errMsg += missingSystemLibraries.asASCIICharArray();
        errMsg += AF_STR_NewLineA;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

        if (missingSystemLibraries.find(GD_STR_missingSystemLibrariesLibOpenCL) != -1 || missingSystemLibraries.find(GD_STR_missingSystemLibrariesLibOpenGL) != -1)
        {
            errMsg += GD_STR_missingSystemLibrariesAdditionalInfo;
        }
        else
        {
            errMsg += GD_STR_missingSystemLibrariesLinuxSuffix;
        }

#else
        {
            errMsg += GD_STR_missingSystemLibrariesDefaultSuffix;
        }
#endif

        // Output an error message:
        acMessageBox::instance().critical(AF_STR_ErrorA, errMsg.asCharArray(), QMessageBox::Ok);

        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStartDebuggingCommand::setKernelSourceFilePaths
// Description: Set the kernel source code file paths
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
bool gdStartDebuggingCommand::setKernelSourceFilePaths()
{
    bool retVal = true;

    if (!_processCreationData.SourceFilesDirectories().isEmpty())
    {
        gtString sourceDirsStr = _processCreationData.SourceFilesDirectories();
        gtStringTokenizer sourceDirTokenizer(sourceDirsStr, L";");
        gtString dirStr;

        while (sourceDirTokenizer.getNextToken(dirStr))
        {
            // Skip empty entries:
            if (dirStr.isEmpty()) { continue; }

            // Get the cl files within the specified folder in the process creation data:
            gtList<osFilePath> filePathsList;
            osDirectory dir(dirStr);
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

bool gdStartDebuggingCommand::StartDebugging()
{
    bool retVal = false;

    static bool stat_needToTestDigitalSignature = true;
    static bool stat_isDigitalSignatureValid = false;

    // If we are in debug build - do not test the digital signature
    // (The signature generation and the signature test are annoying when debugging)
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    stat_needToTestDigitalSignature = false;
    stat_isDigitalSignatureValid = true;
#endif

    // If we need to test the digital signature:
    if (stat_needToTestDigitalSignature)
    {
        // If we are running under a debugger - do not test the digital signature:
        // (Make the life of the people who try to reverse engineer us using a debugger
        //  a bit harder)
        stat_isDigitalSignatureValid = true;

        stat_needToTestDigitalSignature = false;
    }

    // If the digital signature is valid:
    if (stat_isDigitalSignatureValid)
    {
        // Check if the host machine's configuration is supported by our process debugger:
        bool isHostSupported = isHostMachineConfigurationSupported();

        if (isHostSupported)
        {
            // The AMD driver on Linux can sometimes block signals.
            // This causes gdb and CodeXL to behave problematically when the signals
            // are released.
            // See ./aticonfig --help about the --sb option:
            bool areLinuxSignalsBlocked = false;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            {
                // Only perform this check if the AMD driver is present:
                bool hasAMDOpenCLDevice = gdApplicationCommands::gdInstance()->isAMDOpenCLDevicePresent();
                bool isHSASystem = false;
#if GD_ALLOW_HSA_DEBUGGING

                if (0 != (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_HSA_COMPONENT))
                {
                    isHSASystem = true;
                }

#endif // GD_ALLOW_HSA_DEBUGGING

                if (hasAMDOpenCLDevice)
                {
                    // The default for the driver is blocking signals:
                    areLinuxSignalsBlocked = true;

                    // Call the driver configuration utility and check whether the "block signals" flag is on:
                    osPipeExecutor pipeExec;
                    gtString commandOutput;
                    bool rcExec = pipeExec.executeCommand(GD_STR_LinuxBlockSignalsQueryCommand, commandOutput);

                    if (rcExec)
                    {
                        // The values for true in xorg.conf are "true" "yes" "on" or "1", false is "false" "no" "off" or "0", all case-insensitive:
                        commandOutput.toLowerCase();
                        static const gtString errorString = L"error";
                        static const gtString trueString1 = L"true";
                        static const gtString trueString2 = L"yes";
                        static const gtString trueString3 = L"on";
                        static const gtString falseString1 = L"false";
                        static const gtString falseString2 = L"no";
                        static const gtString falseString3 = L"off";

                        if (commandOutput.isEmpty() || (-1 != commandOutput.find(errorString)))
                        {
                            // On any error, we assume default behavior, which is blocking:
                            areLinuxSignalsBlocked = true;
                            GT_IF_WITH_ASSERT(isHSASystem && commandOutput.isEmpty())
                            {
                                // On HSA systems where aticonfig is not found, there is no need to disable signal blocking:
                                areLinuxSignalsBlocked = false;
                            }
                            else
                            {
                                gtString logMsg = commandOutput;
                                logMsg.prepend(L"Unexpected output from aticonfig: ");
                                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                            }
                        }
                        else if ((commandOutput.find(trueString1) != -1) || (commandOutput.find(trueString2) != -1) || (commandOutput.find(trueString3) != -1) || (commandOutput.find('1') != -1))
                        {
                            // Linux Signals are blocked:
                            areLinuxSignalsBlocked = true;
                        }
                        else if ((commandOutput.find(falseString1) != -1) || (commandOutput.find(falseString2) != -1) || (commandOutput.find(falseString3) != -1) || (commandOutput.find('0') != -1))
                        {
                            // Linux Signals are not blocked:
                            areLinuxSignalsBlocked = false;
                        }
                        else
                        {
                            // Neither value was found, assume default (blocking):
                            areLinuxSignalsBlocked = true;
                            GT_ASSERT(false);
                        }
                    }

                    // If signals are blocked:
                    if (areLinuxSignalsBlocked)
                    {
                        // Ask the user if they want to continue:
                        int userAnswer = acMessageBox::instance().warning(GD_STR_LinuxBlockSignalsMessageTitle, GD_STR_LinuxBlockSignalsMessage, QMessageBox::Yes | QMessageBox::No);

                        if (userAnswer == QMessageBox::Yes)
                        {
                            // The user wants to continue:
                            areLinuxSignalsBlocked = false;
                        }
                    }
                }
            }
#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS

            // On remote targets, spies dir adjustments are done by the remote debugging server
            // (See rdDebuggerCommandExecutor::processDebugProjectSettingsForLocalDebugger)
            bool adjustedSpiesDir = true;

            // Get the remote target settings:
            bool isRemoteTarget = _processCreationData.isRemoteTarget();

            if (!isRemoteTarget)
            {
                OS_OUTPUT_DEBUG_LOG(_processCreationData.workDirectory().asString().asCharArray(), OS_DEBUG_LOG_DEBUG);
                adjustedSpiesDir = adjustSpiesDirectoryToBinaryType();
            }

            if (adjustedSpiesDir && (!areLinuxSignalsBlocked))
            {
                // Saves the user answer:
                int userAnswer = afGlobalVariablesManager::instance().askRestoreLogSeverity();


                // If the user confirmed the process execution:
                if (userAnswer == QMessageBox::Ok)
                {
                    // Clear output screen:
                    afApplicationCommands::instance()->ClearInformationView();

                    // Launch the debugged process:
                    OS_OUTPUT_DEBUG_LOG(_processCreationData.workDirectory().asString().asCharArray(), OS_DEBUG_LOG_DEBUG);
                    retVal = gaLaunchDebuggedProcess(_processCreationData, false);

                    // After launching the debugged process, set the kernel source code file paths:
                    (void) setKernelSourceFilePaths();
                }
                else
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

void gdStartDebuggingCommand::HandleInvalidSettings(bool exeFileExist, bool workingFolderExist, bool logDirExistAndAccessible, bool arePortsLegal, bool arePortsDistinct,
                                                    bool requiredSystemLibrariesExists, bool& failedOnGlobalOption)
{
    failedOnGlobalOption = false;

    // Check if this is a remote target:
    bool isRemoteTarget = _processCreationData.isRemoteTarget();

    bool shouldDisplaySettings = !isRemoteTarget && requiredSystemLibrariesExists;

    if (!isRemoteTarget && !exeFileExist)
    {
        bool isProjectLoaded = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();
        QString userMessage = isProjectLoaded ? AF_STR_NoExeIsSelected : AF_STR_NoProjectIsLoaded;

        // The exe file does not exist on disk - display an appropriate message:
        QMessageBox::StandardButton userAnswer = acMessageBox::instance().information(AF_STR_ErrorA, userMessage, QMessageBox::Ok | QMessageBox::Cancel);
        shouldDisplaySettings = (userAnswer == QMessageBox::Ok);
    }
    else if (!isRemoteTarget && !workingFolderExist)
    {
        // The exe file does not exist on disk - display an appropriate message:
        acMessageBox::instance().information(AF_STR_InformationA, GD_STR_StartDebuggingWorkingFolderDoesNotExist, QMessageBox::Ok);
    }
    else if (!isRemoteTarget && !logDirExistAndAccessible)
    {
        // The log file does not exist on disk - display an appropriate message:
        acMessageBox::instance().information(AF_STR_InformationA, GD_STR_StartDebuggingLogDirDoesNotExist, QMessageBox::Ok);
        failedOnGlobalOption = true;
    }
    else if (isRemoteTarget && !arePortsLegal)
    {
        // One or more of the remote connection ports is missing:
        acMessageBox::instance().information(AF_STR_InformationA, GD_STR_StartDebuggingRemotePortsIllegal, QMessageBox::Ok);
    }
    else if (isRemoteTarget && !arePortsDistinct)
    {
        // At least two of the remote connection ports are identical:
        acMessageBox::instance().information(AF_STR_InformationA, GD_STR_StartDebuggingRemotePortsNotDistinct, QMessageBox::Ok);
    }

    // If we need to display the debug settings dialog again:
    if (shouldDisplaySettings)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            if (failedOnGlobalOption)
            {
                // Open the global settings dialog:
                pApplicationCommands->onToolsOptions();
            }
            else
            {
                // Open the debug settings dialog:
                pApplicationCommands->OnProjectSettings(AF_globalSettingsGeneralHeaderUnicode);
            }
        }
    }
}

