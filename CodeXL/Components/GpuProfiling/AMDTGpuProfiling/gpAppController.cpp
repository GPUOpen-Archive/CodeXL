//------------------------------ gpAppController.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Local:
#include <AMDTGpuProfiling/gpAppController.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerViewCreator.h>
#include <AMDTGpuProfiling/ProfileManager.h>


//#pragma message ("TODO: DX: remove when really implemented")
static QProgressDialog* m_sProgressDialog = nullptr;

gpAppController* gpAppController::m_spMySingleInstance = nullptr;


gpAppController::gpAppController() : m_pApplicationCommands(nullptr), m_pApplicationTree(nullptr), m_pCurrentlyRunningSessionData(nullptr)
{
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        // Register as an events observer:
        apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
    }

    // 'Register' to the shared profiling manager
    //Connect to the shared profile manager
    bool rcConnect = connect(&(SharedProfileManager::instance()), SIGNAL(profileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)),
                             this, SLOT(OnProfileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(&(SharedProfileManager::instance()), SIGNAL(profileBreak(const bool&, const spISharedProfilerPlugin * const)),
                        this, SLOT(OnProfilePaused(const bool&, const spISharedProfilerPlugin * const)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(&(SharedProfileManager::instance()), SIGNAL(profileStopped(const spISharedProfilerPlugin * const, bool)),
                        this, SLOT(OnProfileStopped(const spISharedProfilerPlugin * const, bool)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)),
                        this, SLOT(OnSessionRename(SessionTreeNodeData*, const osFilePath&, const osDirectory&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(BeforeSessionRename(SessionTreeNodeData*, bool&, QString&)),
                        this, SLOT(OnBeforeSessionRename(SessionTreeNodeData*, bool&, QString&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(FileImported(const QString&, bool&)), this, SLOT(OnImportSession(const QString&, bool&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionDeleted(ExplorerSessionId, SessionExplorerDeleteType, bool&)), this, SLOT(OnSessionDelete(ExplorerSessionId, SessionExplorerDeleteType, bool&)));
    GT_ASSERT(rcConnect);

    // Add the power profile sessions extension to import list:
    ProfileApplicationTreeHandler::instance()->AddImportFileFilter(GPU_STR_profileFileExtExtensionCaption, GPU_STR_profileFileExtSearchString);

}

gpAppController& gpAppController::Instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == nullptr)
    {
        // Create it:
        m_spMySingleInstance = new gpAppController;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

gpAppController::~gpAppController()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

void gpAppController::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // handle the Global var changed event
    switch (eventType)
    {
        case apEvent::AP_PROFILE_PROCESS_TERMINATED:
        {
            const apProfileProcessTerminatedEvent& profileProcessTerminateEvent = dynamic_cast<const apProfileProcessTerminatedEvent&>(eve);

            if (profileProcessTerminateEvent.profilerName() == GPU_STR_profileFileExt)
            {
                SharedProfileManager::instance().stopCurrentRun();
            }
        }
        break;

        default:
            break;
    }
}

afRunModes gpAppController::getCurrentRunModeMask()
{
    afRunModes retVal = 0;
    return retVal;
}

bool gpAppController::canStopCurrentRun()
{
    return true;
}

bool gpAppController::stopCurrentRun()
{
    return true;
}

bool gpAppController::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(&exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    return true;
}

/// Handle invalid project settings
void gpAppController::HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId)
{
    GT_UNREFERENCED_PARAMETER(isProfileSettingsOK);
    GT_UNREFERENCED_PARAMETER(processId);
    GT_ASSERT_EX(false, L"Not implemented yet");
}

gpSessionTreeNodeData* gpAppController::CreateSession(gtString& projectName, osFilePath& projectDirPath, gtString& strSessionDisplayName, osDirectory& sessionOsDir)
{
    GT_UNREFERENCED_PARAMETER(projectDirPath);

    gpSessionTreeNodeData* pRetVal = nullptr;

    // Create the session data:
    gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
    gtString workDirAsStr = projectSettings.workDirectory().asString();

    osFilePath outputFilePath(sessionOsDir.directoryPath());
    outputFilePath.setFileName(strSessionDisplayName);
    outputFilePath.setFileExtension(GPU_STR_profileFileExt);

    pRetVal = new gpSessionTreeNodeData(acGTStringToQString(strSessionDisplayName), acGTStringToQString(workDirAsStr), acGTStringToQString(sessionOsDir.directoryPath().asString()), acGTStringToQString(outputFilePath.asString()), acGTStringToQString(projectName), false);

    pRetVal->m_profileTypeStr = PM_profileTypeDXProfile;
    pRetVal->m_sessionDir = sessionOsDir;

    // get the last dir path this is our session name:
    QString sessionName = ProfileApplicationTreeHandler::GetProjectNameFromSessionDir(sessionOsDir);

    pRetVal->m_commandArguments.clear();

    gtString exeName;
    projectSettings.executablePath().getFileNameAndExtension(exeName);
    pRetVal->m_exeName = acGTStringToQString(exeName);
    gtString executablePathAsStr = projectSettings.executablePath().asString();
    pRetVal->m_exeFullPath = acGTStringToQString(executablePathAsStr);

    if (pRetVal->m_exeFullPath.isEmpty())
    {
        pRetVal->m_exeFullPath = acGTStringToQString(projectSettings.windowsStoreAppUserModelID());
    }

    pRetVal->m_profileScope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;
    GT_ASSERT(pRetVal->m_profileScope == PM_PROFILE_SCOPE_SINGLE_EXE);

    pRetVal->m_shouldProfileEntireDuration = true;

    return pRetVal;
}

void gpAppController::OnProfileStarted(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId)
{
    GT_UNREFERENCED_PARAMETER(processId);
    GT_UNREFERENCED_PARAMETER(profileTypeStr);

    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // call new functions
        gtASCIIString strWebRtn, strPid, strQueryUrl;
        string strPage;        // only used for easier debug string inspection

        m_pGraphicsServerCommunication = new GraphicsServerCommunication("127.0.0.1", 80);

        strWebRtn.makeEmpty();

        bool rc = m_pGraphicsServerCommunication->GetProcessID(strWebRtn, false);

        if (rc)
        {
            rc = m_pGraphicsServerCommunication->ConnectProcess("");
        }

#endif
    }
}

void gpAppController::OnProfilePaused(const bool& toggled, const spISharedProfilerPlugin* const pCallback)
{
    GT_UNREFERENCED_PARAMETER(toggled);
    GT_UNREFERENCED_PARAMETER(pCallback);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    gtASCIIString strWebRtn, strPid, strQueryUrl;
    string strPage;        // only used for easier debug string inspection

    // no need to call this currently for DX12
    //    gpsComm.Pause(gtASCIIString& strLinkedTrace)

    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        bool rc = m_pGraphicsServerCommunication->PushLogger(strWebRtn);

        if (rc)
        {
            rc = m_pGraphicsServerCommunication->GetLinkedTrace(strWebRtn);
        }

        const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
        gtString projectName = projectSettings.projectName();
        gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

        gtString strSessionDisplayName;
        osDirectory sessionOsDir;
        osFilePath projectDirPath(projectDir);

        if (rc)
        {
            acMessageBox::instance().information("Working correctly", strWebRtn.asCharArray(), QMessageBox::Ok);
            // Get the place where the file needs to be written to:
            // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
            ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projectName, projectDirPath, strSessionDisplayName, sessionOsDir);

            // Create the file and write it:
            osFilePath apiTraceFilePath = sessionOsDir.directoryPath();
            apiTraceFilePath.setFileName(L"APITrace");
            apiTraceFilePath.setFileExtension(L"txt");
            osFile apiTraceFile;
            bool rcFile = apiTraceFile.open(apiTraceFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

            if (rcFile)
            {
                gtString dataAsStr;
                dataAsStr.fromASCIIString(strWebRtn.asCharArray());
                apiTraceFile.writeString(dataAsStr);
                apiTraceFile.close();
            }
        }
        else
        {
            acMessageBox::instance().information("ERROR", strWebRtn.asCharArray(), QMessageBox::Ok);
        }

        if (rc)
        {
            rc = m_pGraphicsServerCommunication->GetTimingLog(strWebRtn);
        }

        if (rc)
        {
            acMessageBox::instance().information("Working correctly", strWebRtn.asCharArray(), QMessageBox::Ok);
            // Get the place where the file needs to be written to:
            // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
            // Create the file and write it:
            osFilePath apiTraceFilePath = sessionOsDir.directoryPath();
            apiTraceFilePath.setFileName(L"APITraceTiming");
            apiTraceFilePath.setFileExtension(L"txt");
            osFile apiTraceFile;
            bool rcFile = apiTraceFile.open(apiTraceFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

            if (rcFile)
            {
                gtString dataAsStr;
                dataAsStr.fromASCIIString(strWebRtn.asCharArray());
                apiTraceFile.writeString(dataAsStr);
                apiTraceFile.close();
            }
        }
        else
        {
            acMessageBox::instance().information("ERROR", strWebRtn.asCharArray(), QMessageBox::Ok);
        }

        if (rc)
        {
            m_pCurrentlyRunningSessionData = CreateSession(projectName, projectDirPath, strSessionDisplayName, sessionOsDir);
        }

    }

#endif
}

void gpAppController::OnProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit)
{
    GT_UNREFERENCED_PARAMETER(stopAndExit);
    GT_UNREFERENCED_PARAMETER(pCallback);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        // Call the shared profile manager profile end function:
        SharedProfileManager::instance().onProfileEnded();

        acMessageBox::instance().critical("CodeXL", "not implemented yet");
        /*
        // disconnect GPS server
        if (false == m_pGraphicsServerCommunication->Disconnect())
        {
            return;
        }

        GT_IF_WITH_ASSERT(m_pCurrentlyRunningSessionData != nullptr)
        {
            // Add the created session to the tree:
            ProfileApplicationTreeHandler::instance()->AddSession(m_pCurrentlyRunningSessionData, true);

        //#pragma message ("TODO: DX: remove this code. This is only until we have a server generated atr")
            static int rand = 0;
            rand++;

            QString tempFilePath;

            if (rand % 3 == 0)
            {
                tempFilePath = "C:\\Temp\\TestFrameProfiler2.atr";
            }
            else if (rand % 3 == 1)
            {
                tempFilePath = "C:\\Temp\\TestFrameProfiler1.atr";
            }
            else if (rand % 3 == 2)
            {
                tempFilePath = "C:\\Temp\\TestFrameProfiler3.atr";
            }
            QFile file(tempFilePath);
            if (file.exists() && !m_pCurrentlyRunningSessionData->m_profileOutputFilePath.isEmpty())
            {
                QString sessionFilePath = acGTStringToQString(m_pCurrentlyRunningSessionData->m_profileOutputFilePath.asString());
                bool rc = QFile::copy(tempFilePath, sessionFilePath);
                if (rand % 3 == 2)
                {
                    // Copy occupancy file:
                    QString tempFilePath = "C:\\Temp\\TestFrameProfiler3.occupancy";
                    osFilePath occPath = m_pCurrentlyRunningSessionData->m_profileOutputFilePath;
                    occPath.setFileExtension(L"occupancy");
                    QString fileName = acGTStringToQString(occPath.asString());
                    rc = rc && QFile::copy(tempFilePath, fileName);

                }
                GT_ASSERT(rc);
            }

            // Open the session file:
            afApplicationCommands::instance()->OpenFileAtLine(m_pCurrentlyRunningSessionData->m_profileOutputFilePath, 0);

            m_pCurrentlyRunningSessionData = nullptr;

            if (m_sProgressDialog != nullptr)
            {
                m_sProgressDialog->hide();
            }

        }*/
    }

#endif
}

void gpAppController::GetFrameProfilingSessionsList(gtList<gtString>& sessionsPathAsStrList)
{
    // Get the project profile sessions directory:
    osFilePath projectFilePath;
    afGetUserDataFolderPath(projectFilePath);

    // Add the 'ProjectName_ProfileOutput' to the folder:
    gtString projectProfilesLocation = afProjectManager::instance().currentProjectSettings().projectName();
    projectProfilesLocation += AF_STR_ProfileDirExtension;
    projectFilePath.appendSubDirectory(projectProfilesLocation);

    gtString projectFolderString = projectFilePath.fileDirectoryAsString();
    projectFolderString.append(osFilePath::osPathSeparator);

    // Get all the session folders for this project:
    osDirectory projectDirectory;
    gtList<osFilePath> sessionsDirectoriesList;
    projectDirectory.setDirectoryFullPathFromString(projectFolderString);
    bool isExistingProjectDirectory  = projectDirectory.exists();

    if (isExistingProjectDirectory  && projectDirectory.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_ASCENDING, sessionsDirectoriesList))
    {
        // check each path if it has a '.pro' file that shows it is a power profiling session:
        gtList<osFilePath>::iterator sessionsIt = sessionsDirectoriesList.begin();

        for (; sessionsIt != sessionsDirectoriesList.end(); sessionsIt++)
        {
            gtString currentSessionDir = (*sessionsIt).asString();

            osFilePath currentSessionFilePath;
            currentSessionFilePath.setFileDirectory(currentSessionDir);

            // Get the directory name (should have the same name as the session file):
            QDir dir(acGTStringToQString(currentSessionDir));
            gtString sessionFileName = acQStringToGTString(dir.dirName());
            currentSessionFilePath.setFileName(sessionFileName);
            currentSessionFilePath.setFileExtension(GPU_STR_profileFileExt);

            // check if the file exits and if it does add it to the list:
            if (currentSessionFilePath.exists())
            {
                sessionsPathAsStrList.push_back(currentSessionFilePath.asString());
            }
        }
    }
}

void gpAppController::OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir)
{
    // Get the MDI creator:
    GpuProfilerMDIViewsCreator* pViewsCreator = GpuProfilerMDIViewsCreator::Instance();

    if (pViewsCreator != nullptr)
    {
        pViewsCreator->OnSessionRename(pRenamedSessionData, oldSessionFilePath, oldSessionDir);
    }
}

void gpAppController::OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    // Get the MDI creator:
    GpuProfilerMDIViewsCreator* pViewsCreator = GpuProfilerMDIViewsCreator::Instance();

    if (pViewsCreator != nullptr)
    {
        pViewsCreator->OnBeforeSessionRename(pAboutToRenameSessionData, isRenameEnabled, renameDisableMessage);
    }
}

void gpAppController::ActivateExistingSession(SessionTreeNodeData* pSessionData)
{
    GT_UNREFERENCED_PARAMETER(pSessionData);
    acMessageBox::instance().critical("CodeXL", "not implemented yet");
}

void gpAppController::OnImportSession(const QString& strSessionFilePath, bool& wasImported)
{
    wasImported = false;

    // Extract the import file extension, and check if this is a profile session:
    osFilePath importedSessionFilePath(acQStringToGTString(strSessionFilePath));
    gtString importedFileExtension;
    importedSessionFilePath.getFileExtension(importedFileExtension);

    if (importedFileExtension.compareNoCase(GPU_STR_profileFileExt) == 0)
    {
        osDirectory sessionOldDirectory;
        importedSessionFilePath.getFileDirectory(sessionOldDirectory);

        gtString importProfile;
        importedSessionFilePath.getFileName(importProfile);
        osDirectory projectPath;
        afProjectManager::instance().currentProjectFilePath().getFileDirectory(projectPath);
        gtString projName = afProjectManager::instance().currentProjectSettings().projectName();

        // Allocate a new session data:
        gpSessionTreeNodeData* pImportSessionData = new gpSessionTreeNodeData("", "", "", "", "", true);

        // Set the imported session data parameters:
        gtString profileFileName;
        importedSessionFilePath.getFileName(profileFileName);
        QString sessionName = acGTStringToQString(profileFileName);

        pImportSessionData->m_displayName = QString("%1 (%2)").arg(sessionName).arg(PM_STR_ImportedSessionPostfix);
        pImportSessionData->m_profileTypeStr = PM_profileTypeDXProfile;
        pImportSessionData->m_profileScope = PM_PROFILE_SCOPE_SINGLE_EXE;


        osDirectory baseDir;
        ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projName, projectPath, profileFileName, baseDir);

        if (sessionOldDirectory.directoryPath() != baseDir.directoryPath())
        {
            // Update the path for the imported profile file path:
            pImportSessionData->m_profileOutputFilePath = baseDir.directoryPath();
            pImportSessionData->m_profileOutputFilePath.setFileExtension(GPU_STR_profileFileExt);
            pImportSessionData->m_profileOutputFilePath.setFileName(profileFileName);

            // Copy the file to the new directory:
            bool rc = osCopyFile(importedSessionFilePath, pImportSessionData->m_profileOutputFilePath, false);
            GT_ASSERT(rc);

            pImportSessionData->m_isImported = true;
            pImportSessionData->m_sessionDir = baseDir;

            // Add the imported session to the tree:
            ProfileApplicationTreeHandler::instance()->AddSession(pImportSessionData, true);

            //Save the session list to the project
            afApplicationCommands::instance()->OnFileSaveProject();

            wasImported = true;
        }
    }
}

void gpAppController::OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete)
{
    GT_UNREFERENCED_PARAMETER(deleteType);

    // Get the data related to session that is about to be deleted:
    gpSessionTreeNodeData* pSessionData = qobject_cast<gpSessionTreeNodeData*>(ProfileApplicationTreeHandler::instance()->GetSessionTreeNodeData(deletedSessionId));

    if (pSessionData != nullptr)
    {
        canDelete = true;

        // If this is the running session:
        if (pSessionData->m_isSessionRunning)
        {
            // Ask the user if he wants to close the profile session:
            int userAnswer = acMessageBox::instance().question(AF_STR_QuestionA, GPU_STR_SessionStopConfirm, QMessageBox::Yes | QMessageBox::No);

            if (userAnswer == QMessageBox::No)
            {
                canDelete = false;
            }
            else
            {
                // Stop session before deleting it
                SharedProfileManager::instance().stopCurrentRun();
            }
        }

        if (canDelete)
        {
            // Get the MDI views creator:
            GpuProfilerMDIViewsCreator* pViewsCreator = GpuProfilerMDIViewsCreator::Instance();

            if (pViewsCreator != nullptr)
            {
                pViewsCreator->OnSessionDelete(pSessionData->m_profileOutputFilePath.asString());
            }
        }
    }
}

void gpAppController::DisplayProcessRunningHTML()
{
    gtString propertiesHTMLMessage;
    afHTMLContent htmlContent(AF_STR_PropertiesProcessRunning);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, PM_STR_PROCESS_IS_RUNNING_MESSAGE);
    htmlContent.toString(propertiesHTMLMessage);
    afPropertiesView* pPropertiesView = afApplicationCommands::instance()->propertiesView();
    GT_IF_WITH_ASSERT(pPropertiesView != nullptr)
    {
        pPropertiesView->setHTMLText(acGTStringToQString(propertiesHTMLMessage), nullptr);
    }
}

