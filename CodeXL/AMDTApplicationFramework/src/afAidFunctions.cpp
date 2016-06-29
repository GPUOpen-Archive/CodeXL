//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afAidFunctions.cpp
///
//==================================================================================

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>

// AMDTApplicationComponents:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// For remote sessions.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// Local:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>


// ---------------------------------------------------------------------------
// Name:        afLoadCodeXLTitleBarIcon
// Description: helper function which loads the icon to the QDialog
// Author:      Yoni Rabin
// Date:        4/6/2012
// ---------------------------------------------------------------------------
void afLoadTitleBarIcon(QDialog* pDlg)
{
    QPixmap iconPixMap;
    acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
    pDlg->setWindowIcon(iconPixMap);
}

// ---------------------------------------------------------------------------
// Name:        afCalculateCodeXLTitleBarString
// Description: Calculates CodeXL's title bar string.
// Arguments: titleBarString - Will get the title bar string.
// Author:      Yaki Tebeka
// Date:        21/8/2007
// ---------------------------------------------------------------------------
void afCalculateCodeXLTitleBarString(gtString& titleBarString)
{

    afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();

    afGetCodeXLTitleBarString(titleBarString, runModes);

}


void afGetCodeXLTitleBarString(gtString& titleBarString, afRunModes runModes)
{

    titleBarString.makeEmpty();

    // Get the current project file name:
    gtString appName = afProjectManager::instance().currentProjectSettings().projectName();

    // First set the project name:
    titleBarString = appName.asCharArray();

    gtString modeString, sessionTypeName, runModeTitle;

    // Append the mode from the execution mode manager:
    afExecutionModeManager& executionMode = afExecutionModeManager::instance();

    if (executionMode.Initialized())
    {
        afIExecutionMode* pActiveMode = executionMode.activeMode();
        int activeSessionType = executionMode.activeSessionType();

        GT_IF_WITH_ASSERT(pActiveMode != nullptr)
        {
            modeString = pActiveMode->modeName();
            sessionTypeName = pActiveMode->sessionTypeName(activeSessionType);

            // Get the debug mode:

            if (runModes & AF_DEBUGGED_PROCESS_RUNNING)
            {
                runModeTitle = AF_STR_TitleRunModeRunning;
            }
            else if (runModes & AF_DEBUGGED_PROCESS_PAUSED)
            {
                runModeTitle = AF_STR_TitleRunModePaused;
            }
            else if (runModes & AF_DEBUGGED_PROCESS_SUSPENDED)
            {
                runModeTitle = AF_STR_TitleRunModeSuspended;

                if (runModes & AF_DEBUGGED_PROCESS_IN_KERNEL_DEBUGGING)
                {
                    runModeTitle = AF_STR_TitleRunModeKernel;
                }
            }
            else if (runModes & AF_DEBUGGED_PROCESS_DATA_TRANSLATING)
            {
                runModeTitle = AF_STR_TitleRunModeDataTranslating;
            }
        }
    }

    // Once all information is gathered build the title:
    // Add the state of the running mode if there is one:
    if (!appName.isEmpty())
    {
        if (!runModeTitle.isEmpty())
        {
            titleBarString.appendFormattedString(AF_STR_TitleRunningMode, runModeTitle.asCharArray());
        }
    }
    else
    {
        titleBarString.append(AF_STR_TitleNoProjectLoaded);
    }

    // Append the CodeXL window name:
    titleBarString.append(AF_STR_TitleSeparator);
    titleBarString.append(AF_STR_Space);
    titleBarString.append(afGlobalVariablesManager::ProductName());


    gtString codeXLVersion = afGlobalVariablesManager::instance().versionCaption();

    if (!codeXLVersion.isEmpty())
    {
        titleBarString.appendFormattedString(L" (%ls)", codeXLVersion.asCharArray());
    }

    titleBarString.append(AF_STR_Space);

    // check if it is a simple session name: only the word "debug" with out really checking the word since it is unknown here
    if (sessionTypeName.find(' ') == -1 || modeString.find(sessionTypeName) != -1)
    {
        titleBarString.appendFormattedString(AF_STR_TitleModeFormat, modeString.asCharArray());
    }
    else
    {
        titleBarString.appendFormattedString(AF_STR_TitleModeSessionFormat, modeString.asCharArray(), sessionTypeName.asCharArray());
    }
}


// ---------------------------------------------------------------------------
// Name:        afGetApplicationImagesPath
// Description: Outputs the path of the application images directory.
// Arguments: imagesDirPathAsString - Will get the output path, converted into a string.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/3/2008
// ---------------------------------------------------------------------------
bool afGetApplicationImagesPath(gtString& imagesDirPathAsString)
{
    bool retVal = false;

    // Get the CodeXL application path:
    osFilePath CodeXLExePath;
    bool rc = CodeXLExePath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    GT_IF_WITH_ASSERT(rc)
    {
        // In Windows / Linux:
        // + ... /CodeXL <- application binary location = application path
        // |- Images <- images dir

        // In Mac:
        // + CodeXL.app (application bundle) <- application path
        // |+ Contents
        //  |- MacOS <- application binary location
        //  |- Resources <- images dir

        // Calculate the application images directory path:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        gtString CodeXLImagesPathAsString;
        gtString CodeXLExecutableExtension;

        // We do not assert this function's return value as it is false if the executable
        // is the binary file (which has no extnesion)
        CodeXLExePath.getFileExtension(CodeXLExecutableExtension);

        if (CodeXLExecutableExtension == GD_STR_CodeXLMacBundleExtension)
        {
            // The executable "file" is acutally the bundle directory, so we need to go inside it:
            CodeXLImagesPathAsString = CodeXLExePath.asString();
            CodeXLImagesPathAsString.append(osFilePath::osPathSeparator);
            CodeXLImagesPathAsString.append(OS_STR_CodeXLMacBundleContentsDirName);
            CodeXLImagesPathAsString.append(osFilePath::osPathSeparator);
            CodeXLImagesPathAsString.append(OS_STR_CodeXLMacBundleResourcesDirName);
            retVal = true;
        }
        else
        {
            GT_ASSERT(CodeXLExecutableExtension.isEmpty());

            // We are running the executable straight from the binary, so we need to go up one level:
            osDirectory CodeXLBundleContentsDir;
            bool rc2 = CodeXLExePath.getFileDirectory(CodeXLBundleContentsDir);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Note: This code is here in the unlikely event someone runs CodeXL from the terminal
                // directly by using the the binaries. In any other case (eg running the binary by using the
                // "Show package contents" option then opening it in the finder) the above option is called.
                CodeXLBundleContentsDir.upOneLevel();
                gtString CodeXLImagesPathAsString = CodeXLBundleContentsDir.directoryPath().asString();
                CodeXLImagesPathAsString.append(osFilePath::osPathSeparator);
                CodeXLImagesPathAsString.append(OS_STR_CodeXLMacBundleResourcesDirName);
                retVal = true;
            }
        }

        if (retVal)
        {
            imagesDirPathAsString = CodeXLImagesPathAsString;
        }

#else // !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        // Get the application installation directory:
        osDirectory CodeXLInstallDir;
        bool rc2 = CodeXLExePath.getFileDirectory(CodeXLInstallDir);
        GT_IF_WITH_ASSERT(rc2)
        {
            gtString CodeXLImagesPathAsString = CodeXLInstallDir.directoryPath().asString();
            CodeXLImagesPathAsString.append(osFilePath::osPathSeparator);
            CodeXLImagesPathAsString.append(AF_STR_CodeXLImagesDirName);

            imagesDirPathAsString = CodeXLImagesPathAsString;

            retVal = true;
        }
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afCanAllowDifferentSystemPath
// Description: Returns true iff the target application can have a different system path.
//              This is true if:
//              * The target and client bitness is different
//              * We are targetting a remote machine
// Author:      Uri Shomroni
// Date:        1/10/2013
// ---------------------------------------------------------------------------
bool afCanAllowDifferentSystemPath()
{
    bool retVal = false;

    const apProjectSettings& currentProjSettings = afProjectManager::instance().currentProjectSettings();

    if (currentProjSettings.isRemoteTarget())
    {
        // Remote targets can have different system paths:
        retVal = true;
    }
    else // !currentProjSettings.isRemoteTarget()
    {
        // If the client is a different address space size than the target, also allow it:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        bool isClient64Bit = false;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        bool isClient64Bit = true;
#else
#error Unknown Address Space Type!
#endif

        bool isTarget64Bit = !isClient64Bit;

        // Get the target executable:
        const osFilePath& targetExecutable = currentProjSettings.executablePath();

        // Since this is local debugging, we can assume that the executable path is on the same machine as us:
        if (targetExecutable.exists())
        {
            bool rc64 = osIs64BitModule(targetExecutable, isTarget64Bit);

            // If the operation failed, reset to default:
            if (!rc64)
            {
                GT_ASSERT(rc64);
                isTarget64Bit = !isClient64Bit;
            }
        }

        // Return the answer:
        retVal = (isTarget64Bit != isClient64Bit);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afDefaultProjectFilePath
// Description: Creates the default file path for a project file, given the executable
//              and the project type and execution target.
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        25/4/2010
// ---------------------------------------------------------------------------
bool afDefaultProjectFilePath(const osFilePath& executableFilePath, osFilePath& projectFilePath)
{
    bool retVal = false;

    // Base the name off the executable file name:
    gtString exeFileName;
    bool rc1 = executableFilePath.getFileName(exeFileName);

    GT_IF_WITH_ASSERT(rc1)
    {
        // Project files are saved in the CodeXL app data dir:
        afGetUserDataFolderPath(projectFilePath);

        // Apply the name and file extension:
        projectFilePath.setFileName(exeFileName);
        projectFilePath.setFileExtension(AF_STR_projectFileExtension);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGetVisualStudioProjectFilePath
// Description: Gets the path to the visual studio project file from a given
//              executable path.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/1/2011
// ---------------------------------------------------------------------------
void afGetVisualStudioProjectFilePath(const osFilePath& executablePath, const gtString& projectName, osFilePath& vsProjectFilePath)
{
    afGetUserDataFolderPath(vsProjectFilePath);

    if (!executablePath.isEmpty())
    {
        vsProjectFilePath.setFromOtherPath(executablePath, false, true, false);
    }
    else
    {
        GT_IF_WITH_ASSERT(!projectName.isEmpty())
        {
            vsProjectFilePath.setFileName(projectName);
        }
    }

    vsProjectFilePath.setFileExtension(AF_STR_visualStudioProjectFileExtension);
}

// ---------------------------------------------------------------------------
// Name:        afGetVersionDetails
// Description: Parses the version number and output the numbers of the version
//              The information is parsed from STRPRODUCTVER, which its structure
//              should be: "[MAJOR],[MINOR],[BUILD],[UPDATE]"
// Arguments:   int& buildVersion
//              int& majorVersion
//              int& minorVersion
//              int& year
//              int& month
//              int& day
// Return Val:  GD_API bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/2/2012
// ---------------------------------------------------------------------------
bool afGetVersionDetails(int& buildVersion, int& majorVersion, int& minorVersion, int& year, int& month, int& day)
{
    bool retVal = true;
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);

    // Initialize return values:
    majorVersion = appVersion._majorVersion;
    minorVersion = appVersion._minorVersion;
    buildVersion = appVersion._patchNumber;
    year = -1;
    month = -1;
    day = -1;

    // Get the current date:
    osTime now;
    now.setFromCurrentTime();
    struct tm timeStruct;
    osTime::TimeZone timeZone = osTime::LOCAL;
    now.timeAsTmStruct(timeStruct, timeZone);
    day = timeStruct.tm_mday;
    month = timeStruct.tm_mon;
    year = timeStruct.tm_year + 1900;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afGetUserDataFolderPath
// Description: Get user data folder path
// Arguments:   osFilePath& userDataPath
// Author:      Sigal Algranaty
// Date:        16/5/2012
// ---------------------------------------------------------------------------
void afGetUserDataFolderPath(osFilePath& userDataPath)
{
    userDataPath.setPath(osFilePath::OS_USER_APPLICATION_DATA);
    userDataPath.appendSubDirectory(afGlobalVariablesManager::ProductName());
}


void IsApplicationPathsValid(const afIsValidApplicationInfo& isValidApplicationInfo, bool &isAppValid, bool &isWorkingFolderValid) 
{
    if (isValidApplicationInfo.isRemoteSession && isValidApplicationInfo.portAddress)
    {
        bool isExecutionSuccessfull = CXLDaemonClient::ValidateAppPaths(*isValidApplicationInfo.portAddress, isValidApplicationInfo.appFilePath, 
                                                                        isValidApplicationInfo.workingFolderPath, isAppValid, isWorkingFolderValid);
        GT_ASSERT_EX(isExecutionSuccessfull, L"Failed to check application path for validity");
    }
    else
    {
        if (isValidApplicationInfo.isWInStoreAppRadioButtonChecked)
        {
            isAppValid = !isValidApplicationInfo.appFilePath.isEmpty();
        }
        else if (!isValidApplicationInfo.appFilePath.isEmpty())
        {
            const osFile file(isValidApplicationInfo.appFilePath);
            //for local files: if windows store application just check if file exists, for regular binaries check if it's an executable
            isAppValid = isValidApplicationInfo.isWInStoreAppRadioButtonChecked ? file.exists() : file.IsExecutable();
        }

        const osFile  dir(isValidApplicationInfo.workingFolderPath);
        isWorkingFolderValid = dir.exists() || isValidApplicationInfo.workingFolderPath.isEmpty();
    }
}

void afGetStartButtonText(gtString& buttonText, bool addKeyboardShortcut /*= false*/, bool fullString /* = true */)
{
    buttonText = L"";
    // Get the current active mode
    afIExecutionMode* pExecMode = afExecutionModeManager::instance().activeMode();

    bool processExists = (0 != ((AF_DEBUGGED_PROCESS_EXISTS)& afPluginConnectionManager::instance().getCurrentRunModeMask()));

    if (processExists)
    {
        buttonText = AF_STR_playButtonContinue;
    }
    else // !processExists
    {
        buttonText = AF_STR_playButtonStartGeneric;

        GT_IF_WITH_ASSERT(NULL != pExecMode)
        {
            pExecMode->GetToolbarStartButtonText(buttonText, fullString);

            // Add the "Start Code if is in the menu area and not tool bar
            if (fullString)
            {
                buttonText.prepend(AF_STR_playButtonStartPrefix);
            }
        }
    }

    // Add the remote location
    bool isRemoteEnabled = afProjectManager::instance().currentProjectSettings().isRemoteTarget();
    if (isRemoteEnabled)
    {
        bool isRemoteHost = !afProjectManager::instance().currentProjectSettings().remoteTargetName().isEmpty();

        if (isRemoteHost)
        {
            buttonText.appendFormattedString(L" @%ls", afProjectManager::instance().currentProjectSettings().remoteTargetName().asCharArray());
        }
    }

    if (addKeyboardShortcut)
    {
        buttonText.prepend('&');
    }
}