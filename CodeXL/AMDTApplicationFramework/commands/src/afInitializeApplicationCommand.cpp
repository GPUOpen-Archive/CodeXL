//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afInitializeApplicationCommand.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>
#include <AMDTApplicationFramework/Include/afUnhandledExceptionHandler.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/Include/commands/afInitializeApplicationCommand.h>
#include <AMDTApplicationFramework/Include/dialogs/afEulaDialog.h>
#include <src/afProcessDebuggerEventHandler.h>

#include <AMDTApplicationComponents/inc/acStringConstants.h>

#define AF_SYSTEM_INFO_TIME_OUT 10000
// Transforms days to seconds
time_t stat_daysToSeconds = (24 * 60 * 60);

bool gdCheckIsEulaLicenseVaild();
bool gdUpdateEulaAcceptence();
bool gdDisplayEULADialog();

afSystemInformationCommandThread* afInitializeApplicationCommand::m_spSysCommandThread = nullptr;

// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::afInitializeApplicationCommand
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
afInitializeApplicationCommand::afInitializeApplicationCommand(const gtString& productName, const gtString& productDescriptionString)
    : m_productName(productName), m_productDescriptionString(productDescriptionString)
{
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::~afInitializeApplicationCommand
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
afInitializeApplicationCommand::~afInitializeApplicationCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::canExecuteSpecificCommand
// Description: Always returns true.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
bool afInitializeApplicationCommand::canExecuteSpecificCommand()
{
    return true;
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::executeSpecificCommand
// Description: Implements this class work - Performs required application initializations.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
bool afInitializeApplicationCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Set the global product name:
    afGlobalVariablesManager::SetProductName(m_productName);

    // Initialize the un-handled exceptions handler:
    initializeUnhandledExceptionHandler();


    // Initialize the debug log file:
    initializeDebugLogFile();

    // Output an "Application init begin" log printout:
    OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_AppInitBegin, OS_DEBUG_LOG_INFO);

    // Create the process debugger event handler:
    afProcessDebuggerEventHandler::instance();

    // Name the application's main thread in Visual Studio's thread's list:
    nameMainThreadInDebugger();

    // Read the Options file:
    loadOptionsFile();

    // Check the OS version:
    bool rc3 = verifyOSVersion();
    GT_IF_WITH_ASSERT(rc3)
    {
        OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_AppInitCmdSucceeded, OS_DEBUG_LOG_DEBUG);
    }

    bool rc4 = true;
    // On Linux / Mac only (where we don't have a setup in which the user accepts the EULA):
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_BUILD_CONFIGURATION != AMDT_DEBUG_BUILD))
    {
        // --------------- EULA ---------------

        // If needed, display EULA dialog:
        rc4 = gdDisplayEULADialog();
    }
#endif

    if (rc4)
    {
#if (AMDT_BUILD_CONFIGURATION != AMDT_DEBUG_BUILD)
        /// NOTICE: Currently, in VS10, web page cannot be initialized when debugging the VS package.
        /// We skip this line as a workaround, to enable the debugging.
        // Initiate Updater to check automatic check for update
        //afSoftwareUpdaterWindow dlg;
        //dlg.performAutoCheckForUpdate();
#endif
    }

    retVal = rc4;

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::verifyOSVersion
// Description: Verifies that we are running on a supported OS version.
// Return Val: bool  - true iff we are running on a supported OS version.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
bool afInitializeApplicationCommand::verifyOSVersion()
{
    bool retVal = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_VerifyWinVersion, OS_DEBUG_LOG_DEBUG);

        // Get the OS Version:
        int majorVersion = 0;
        int minorVersion = 0;
        int buildNumber = 0;

        osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber);

        // If we are running on Windows version that is smaller than XP (5.1), display an error message:
        if ((majorVersion < 5) || ((majorVersion == 5) && (minorVersion < 1)))
        {
            gtString osErrorMsg;
            osErrorMsg.appendFormattedString(AF_STR_OperatingSystemNotSupported, majorVersion, minorVersion, buildNumber);
            OS_OUTPUT_DEBUG_LOG(osErrorMsg.asCharArray(), OS_DEBUG_LOG_ERROR);

            acMessageBox::instance().critical(AF_STR_ErrorA, acGTStringToQString(osErrorMsg), QMessageBox::Ok);
        }
        else
        {
            retVal = true;
        }
    }
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    {
        // We currently don't have a list of unsupported Linux OS versions:
        retVal = true;
    }
#elif (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    {
        OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_VerifyMacVersion, OS_DEBUG_LOG_DEBUG);

        // Get the OS Version:
        int majorVersion = 0;
        int minorVersion = 0;
        int buildNumber = 0;

        osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber);

        // CodeXL Mac is currently supported on Mac OS X 10.5 (Leopard) and 10.6 (Snow Leopard) only.
        // The appropriate Darwin (kernel) version for this is 9.Y.0 where the OS X version is 10.5.Y
        // and 10.Z.0 where the OS X version is 10.6.Y.
        // We don't limit older version (8 and under) since that is limited in the Info.plist (and
        // currently doesn't run anyway because of mismatching libraries).
        if (majorVersion > 10)
        {
            gtString osErrorMsg;
            osErrorMsg.appendFormattedString(AF_STR_OperatingSystemNotSupported, majorVersion, minorVersion, buildNumber);
            OS_OUTPUT_DEBUG_LOG(osErrorMsg.asCharArray(), OS_DEBUG_LOG_ERROR);

            acMessageBox::instance().critical(AF_STR_ErrorA, acGTStringToQString(osErrorMsg), QMessageBox::Ok);
        }
        else
        {
            retVal = true;
        }
    }
#else
#error Unknown Linux variant!
#endif
#else
#error Error: Unsupported OS platform!!
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::initializeDebugLogFile
// Description: Initialize the debug log file.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/8/2005
// ---------------------------------------------------------------------------
bool afInitializeApplicationCommand::initializeDebugLogFile()
{
    bool retVal = false;

    // Set the thread naming prefix:
    osThread::setThreadNamingPrefix(afGlobalVariablesManager::ProductNameCharArray());

    // Initialize the OS description string:
    initializeSystemInformationData();

    // Initialize the log file:
    osDebugLog& theDebugLog = osDebugLog::instance();
    retVal = theDebugLog.initialize(afGlobalVariablesManager::ProductName(), m_productDescriptionString.asCharArray(), m_systemInformationStr.asCharArray());

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::initializeUnhandledExceptionHandler
// Description:
//  Initializes the unhandled exception handler. This handler will be called when
//  this application crashes.
//
// Author:      Yaki Tebeka
// Date:        21/4/2009
// ---------------------------------------------------------------------------
void afInitializeApplicationCommand::initializeUnhandledExceptionHandler()
{
    // afUnhandledExceptionHandler is currently supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Create and initialize the unhandled exceptions handler:
        afUnhandledExceptionHandler::instance();
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::initSystemInformationData
// Description: Initializes system information data. This data is saved as string
//              within the application debug log object
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/12/2010
// ---------------------------------------------------------------------------
bool afInitializeApplicationCommand::initializeSystemInformationData()
{
    bool retVal = false;

    // create a system information command thread
    m_spSysCommandThread = new afSystemInformationCommandThread;
    m_spSysCommandThread->execute();

    if (!osWaitForFlagToTurnOff(m_spSysCommandThread->isGatheringData(), AF_SYSTEM_INFO_TIME_OUT))
    {
        m_systemInformationStr = AF_STR_SystemInformationCommandTimeOut;
        // Don't kill the thread if it did not finish since it is gathering other data
    }
    else
    {
        // Collect the system information data and append to a string:
        m_systemInformationStr = m_spSysCommandThread->m_systemInformationStr;
    }

    // We cannot assert for this return value, since there is no log initialized yet:
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::loadOptionsFile
// Description: Load data from the Options file.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
void afInitializeApplicationCommand::loadOptionsFile()
{
    // Debug log printout:
    OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_LoadingOptionsFile, OS_DEBUG_LOG_DEBUG);

    // Get the gdGDebuggerGlobalVariablesManager instance
    afGlobalVariablesManager& theGlobalVarsManager = afGlobalVariablesManager::instance();

    // Load the Options file data:
    theGlobalVarsManager.loadGlobalSettingsFromXMLFile();
    /* Uri, 10/5/12 - The loading mechanism does not currently have a return value (due to depending on extensions). Should it ever have a return value, the below code should be called on failure:
    if (!rc)
    {
    // We failed to load the options from the options file:

    // Add log file message:
    OS_OUTPUT_DEBUG_LOG(AF_STR_ErrorMessageFailedToLoadTheSettingsUnicode, OS_DEBUG_LOG_ERROR);

    // Display a message box:
    acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_ErrorMessageFailedToLoadTheSettings, QMessageBox::Ok);
    }*/

    OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_FinishedLoadingOptionsFile, OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        afInitializeApplicationCommand::nameThreadInDebugger
// Description:
//   Name the application's main thread in Visual Studio's thread's list.
//   (We assume that the main application's thread is the thread that uses afInitializeApplicationCommand)
// Author:      Yaki Tebeka
// Date:        25/11/2010
// ---------------------------------------------------------------------------
void afInitializeApplicationCommand::nameMainThreadInDebugger()
{
    // Naming threads in a debugger is only supported on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // Do not rename the main thread in Visual Studio:
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        // Name the application's main thread in Visual Studio's thread's list:
        osThreadId mainThreadId = osGetCurrentThreadId();
        gtASCIIString mainThreadName("CodeXL - main thread");
        osNameThreadInDebugger(mainThreadId, mainThreadName);
    }

#endif
}

void afInitializeApplicationCommand::EndSysCommandThread()
{
    if (m_spSysCommandThread != nullptr)
    {
        if (m_spSysCommandThread->isAlive())
        {
            m_spSysCommandThread->terminate();
        }

        delete m_spSysCommandThread;
        m_spSysCommandThread = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDisplayEULADialog
// Description: If needed, displays the EULA dialog.
// Arguments: pMainAppWindow - The Applications main window.
// Author:      Yaki Tebeka
// Date:        5/5/2010
// ---------------------------------------------------------------------------
bool gdDisplayEULADialog()
{
    bool retVal = true;
    // Check if the EULA License was already accepted for the current revision
    bool isEULAValid = gdCheckIsEulaLicenseVaild();

    if (!isEULAValid)
    {
        // Display the EULA dialog:
        bool wasEULAAccepted = gdUpdateEulaAcceptence();

        // If the EULA was not accepted - exit!
        if (!wasEULAAccepted)
        {
            OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_EULANotAcceptedExiting, OS_DEBUG_LOG_INFO);

            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdCheckIsEulaLicenseVaild()
// Description: Checks whether the EULA License was accepted by the user
//              for the current application's revision number
// Return Val: bool  - Was accepted / Was not.
// Author:      Guy Ilany
// Date:        2007/12/22
// ---------------------------------------------------------------------------
bool gdCheckIsEulaLicenseVaild()
{
    bool retVal = false;

    // Get the most updated revision number which the user has already accepted
    int lastAcceptedRevisionNuber = afGlobalVariablesManager::instance().getEULRevisionNumber();

    // Get the current application version:
    osProductVersion currentAppVersion;
    osGetApplicationVersion(currentAppVersion);

    // Get the current application's revision number
    int currentApplicationRevisionNuber = currentAppVersion._patchNumber;

    // Check if the current application's revision number is bigger than
    // the most updated revision that was accepted
    if (currentApplicationRevisionNuber <= lastAcceptedRevisionNuber)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdUpdateEulaAcceptence()
// Description: Shows the EULA dialog and updates the user's revision number if the user accepted.
//              In case the user didn't accept the EULA License terms - a proper dialog pops.
// Return Val: bool  - User accepted / Didn't accept.
// Author:      Guy Ilany
// Date:        2007/12/22
// ---------------------------------------------------------------------------
bool gdUpdateEulaAcceptence()
{
    bool retVal = false;

    // Show the EULA Dialog
    afEulaDialog eulaDialog(nullptr);

    // Call the show modal function for the eula dialog:
    int rc1 = eulaDialog.execute();

    // If the user accepted the EULA License terms then we can continue
    if (QDialog::Accepted == rc1)
    {
        retVal = true;
    }
    else
    {
        // Pop a Message Box telling the user he/she must accept the terms
        acMessageBox::instance().warning(AF_STR_ErrorA, AC_STR_EULANotAcceptedMessage, QMessageBox::Ok);
    }

    return retVal;
}
