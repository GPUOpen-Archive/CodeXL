//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpAppController.cpp
///
//==================================================================================

//------------------------------ ppEventObserver.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SharedProfileProcessMonitor.h>
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// AMDTCommonHeaders:
#include <AMDTCommonHeaders/AMDTDefinitions.h>

// Backend:
#include <AMDTThreadProfileDataTypes.h>
#include <AMDTThreadProfileApi.h>

// Local:
#include <inc/tpAppController.h>
#include <inc/StringConstants.h>
#include <inc/tpMDIViewCreator.h>
#include <inc/tpTreeHandler.h>

#include <AMDTCpuProfilingRawData/inc/RunInfo.h>

tpAppController* tpAppController::m_spMySingleInstance = nullptr;

tpAppController::tpAppController() :
    m_pApplicationTree(nullptr), m_pCurrentlyRunningSessionData(nullptr), m_pMonitorProcessThread(nullptr)
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

    SharedProfileManager::instance().registerProfileType(acQStringToGTString(CP_STR_ThreadProfileTypeName), this, CP_STR_projectSettingExtensionDisplayName, SPM_ALLOW_STOP);

    // Add the power profile sessions extension to import list:
    ProfileApplicationTreeHandler::instance()->AddImportFileFilter(CP_STR_ThreadProfileExtension, CP_STR_ThreadProfileExtensionSearchString, PM_STR_PROFILE_MODE);

}

tpAppController& tpAppController::Instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == nullptr)
    {
        // Create it:
        m_spMySingleInstance = new tpAppController;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

tpAppController::~tpAppController()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

void tpAppController::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // handle the Global var changed event
    switch (eventType)
    {
        case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
        {
            // Get the activation event:
            const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

            // Get the item data;
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

            if (pItemData != nullptr)
            {
                // Check if the file is of offline/online profiling. if it is Activate the item:
                tpSessionTreeNodeData* pExtenedData = qobject_cast<tpSessionTreeNodeData*>(pItemData->extendedItemData());

                if (pExtenedData != nullptr)
                {
                    // Display the item (is this is a power profile tree node):
                    bool rc = ActivateItem(pItemData->m_pTreeWidgetItem);
                    GT_ASSERT(rc);
                }
            }
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            const afGlobalVariableChangedEvent& globalVarChangedEvent = dynamic_cast<const afGlobalVariableChangedEvent&>(eve);
            // Get id of the global variable that was changed
            afGlobalVariableChangedEvent::GlobalVariableId variableId = globalVarChangedEvent.changedVariableId();

            // If the project file path was changed
            if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                // Initialize the sessions for this project:
                ProjectOpened();
            }
        }
        break;

        case apEvent::AP_PROFILE_PROCESS_TERMINATED:
        {
            const apProfileProcessTerminatedEvent& profileProcessTerminateEvent = dynamic_cast<const apProfileProcessTerminatedEvent&>(eve);

            if (profileProcessTerminateEvent.profilerName() == CP_STR_ThreadProfileExtensionW)
            {
                SharedProfileManager::instance().stopCurrentRun();
            }
        }
        break;

        default:
            break;
    }
}


afRunModes tpAppController::getCurrentRunModeMask()
{
    afRunModes retVal = 0;
    return retVal;
}

bool tpAppController::canStopCurrentRun()
{
    return true;
}

bool tpAppController::stopCurrentRun()
{
    return true;
}

bool tpAppController::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(&exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    return true;
}

/// Handle invalid project settings
void tpAppController::HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId)
{
#pragma message ("TODO: TP :")
    // Check if the project is set:
    bool isProjectSet = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();
    bool isExeExist = !afProjectManager::instance().currentProjectSettings().executablePath().isEmpty();
    ProfileSessionScope scope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;

    // Open the project settings in the following cases:
    QString infoMessage = PM_STR_StartProfilingNoProjectIsLoaded;

    if (isProjectSet && (scope != PM_PROFILE_SCOPE_SYS_WIDE) && (processId == 0))
    {
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        isProfileSettingsOK = true;
        processId = PROCESS_ID_UNBOUND;
    }
    // Attach to process:
    else if (isProjectSet && (scope == PM_PROFILE_SCOPE_SINGLE_EXE) && (processId != 0))
    {
        isProfileSettingsOK = true;
    }
    else if (isProjectSet && !isExeExist && (scope == PM_PROFILE_SCOPE_SYS_WIDE))
    {
        isProfileSettingsOK = true;
        processId = PROCESS_ID_UNBOUND;
    }
    else if (!isProjectSet && (processId != 0) && !isExeExist)
    {
        // Attach to process:
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
        isProfileSettingsOK = true;
    }
    else if (!isProjectSet && !isExeExist && (scope != PM_PROFILE_SCOPE_SYS_WIDE))
    {
        // Attach to process:
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        isProfileSettingsOK = true;
    }
}


void tpAppController::AddSessionToTree()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCurrentlyRunningSessionData != nullptr)
    {
        m_pCurrentlyRunningSessionData->m_isImported = false;

        m_pCurrentlyRunningSessionData->m_commandArguments.clear();

        const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

        gtString workDirAsStr = projectSettings.workDirectory().asString();
        m_pCurrentlyRunningSessionData->m_workingDirectory = acGTStringToQString(workDirAsStr);
        gtString exeName;
        projectSettings.executablePath().getFileNameAndExtension(exeName);
        m_pCurrentlyRunningSessionData->m_exeName = acGTStringToQString(exeName);
        gtString executablePathAsStr = projectSettings.executablePath().asString();
        m_pCurrentlyRunningSessionData->m_exeFullPath = acGTStringToQString(executablePathAsStr);

        if (m_pCurrentlyRunningSessionData->m_exeFullPath.isEmpty())
        {
            m_pCurrentlyRunningSessionData->m_exeFullPath = acGTStringToQString(projectSettings.windowsStoreAppUserModelID());
        }

        m_pCurrentlyRunningSessionData->m_profileScope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;

        if (m_pCurrentlyRunningSessionData->m_profileScope == PM_PROFILE_SCOPE_SINGLE_EXE)
        {
            // Single exe scope is not supported for this profile type:
            m_pCurrentlyRunningSessionData->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        }

        m_pCurrentlyRunningSessionData->m_shouldProfileEntireDuration = true;

        // Add the created session to the tree:
        ProfileApplicationTreeHandler::instance()->AddSession(m_pCurrentlyRunningSessionData, true);
    }

}

void tpAppController::OnProfileStarted(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId)
{
    // Do not start a profile if this is not a power profile session:
    if ((profileTypeStr == acQStringToGTString(PM_profileTypeThreadProfilePrefix)) && (pCallback == static_cast<spISharedProfilerPlugin*>(this)))
    {
        GT_UNREFERENCED_PARAMETER(processId);

        OS_OUTPUT_DEBUG_LOG(CP_STR_logStartProfiling, OS_DEBUG_LOG_INFO);

        gtString strSessionDisplayName;
        osDirectory sessionOsDir;
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
        osFilePath projectFilePath = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();

        // Get the next session name and dir from the session naming helper:
        ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projectName, projectFilePath, strSessionDisplayName, sessionOsDir);

        m_pCurrentlyRunningSessionData = new tpSessionTreeNodeData;
        m_pCurrentlyRunningSessionData->m_pParentData = new afApplicationTreeItemData;

        m_pCurrentlyRunningSessionData->m_pParentData->m_filePath.setFileDirectory(sessionOsDir);
        m_pCurrentlyRunningSessionData->m_pParentData->m_filePath.setFileName(strSessionDisplayName);
        m_pCurrentlyRunningSessionData->m_pParentData->m_filePath.setFileExtension(CP_STR_ThreadProfileExtensionW);

        m_pCurrentlyRunningSessionData->m_name = acGTStringToQString(strSessionDisplayName);
        m_pCurrentlyRunningSessionData->m_displayName = m_pCurrentlyRunningSessionData->m_name;
        m_pCurrentlyRunningSessionData->m_profileTypeStr = PM_profileTypeThreadProfile;
        m_pCurrentlyRunningSessionData->m_projectName = acGTStringToQString(projectName);
        m_pCurrentlyRunningSessionData->m_exeFullPath = acGTStringToQString(afProjectManager::instance().currentProjectSettings().executablePath().asString());
        m_pCurrentlyRunningSessionData->m_workingDirectory = acGTStringToQString(afProjectManager::instance().currentProjectSettings().workDirectory().asString());

        // Launch the profile session application
        bool rc = LaunchProcess();
        GT_IF_WITH_ASSERT(rc)
        {

            QString sessionFilePath = acGTStringToQString(m_pCurrentlyRunningSessionData->m_pParentData->m_filePath.asString());
            QString message = QString("Running thread profile on file: %1").arg(sessionFilePath);

            AMDTResult res = AMDTSetThreadProfileConfiguration(AMDT_TP_EVENT_TRACE_CSWITCH | AMDT_TP_EVENT_TRACE_CALLSTACK, sessionFilePath.toStdString().c_str());
            GT_IF_WITH_ASSERT(res == AMDT_STATUS_OK)
            {
                res = AMDTStartThreadProfile();
                GT_ASSERT(res == AMDT_STATUS_OK);
            }
        }
    }
}
void tpAppController::OnProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit)
{
    GT_UNREFERENCED_PARAMETER(stopAndExit);
    GT_UNREFERENCED_PARAMETER(pCallback);

    OS_OUTPUT_DEBUG_LOG(CP_STR_logStopProfiling, OS_DEBUG_LOG_INFO);

    // Create a dummy file with the session name:
    AddSessionToTree();

    //terminate process monitor thread
    if (nullptr != m_pMonitorProcessThread)
    {
        if (!m_pMonitorProcessThread->processEnded())
        {
            m_pMonitorProcessThread->terminate();
        }

        delete m_pMonitorProcessThread;
        m_pMonitorProcessThread = nullptr;
    }

    SharedProfileManager::instance().onProfileEnded();

    // Stop the thread profile:
    AMDTResult rc = AMDTStopThreadProfile();
    GT_ASSERT(rc == AMDT_STATUS_OK);

    WriteRunInfo();
}

void tpAppController::WriteRunInfo()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCurrentlyRunningSessionData != nullptr)
    {
        // populate RI data
        RunInfo rInfo;

        // target path
        rInfo.m_targetPath = acQStringToGTString(m_pCurrentlyRunningSessionData->m_exeFullPath);

        // working dir
        rInfo.m_wrkDirectory = acQStringToGTString(m_pCurrentlyRunningSessionData->m_workingDirectory);

        // data folder
        rInfo.m_profDirectory = m_pCurrentlyRunningSessionData->SessionDir().directoryPath().asString();

        // command line arguments
        rInfo.m_cmdArguments = acQStringToGTString(m_pCurrentlyRunningSessionData->m_commandArguments);

        // environment variables
        rInfo.m_envVariables = m_pCurrentlyRunningSessionData->m_envVariables;
        // call stack sampling enabled/disabled
        rInfo.m_isCSSEnabled = false; //TO DO
        // stack depth
        rInfo.m_cssUnwindDepth = 1; //TO DO

        // profile session type
        rInfo.m_profType = CP_STR_InfoViewThreadProfiling;
        // profile start time
        rInfo.m_profStartTime = acQStringToGTString(m_pCurrentlyRunningSessionData->m_startTime);
        // profile end time
        rInfo.m_profEndTime = acQStringToGTString(m_pCurrentlyRunningSessionData->m_endTime);

        rInfo.m_executedPID = m_pCurrentlyRunningSessionData->m_pid;

        osFilePath riFilePath(m_pCurrentlyRunningSessionData->m_pParentData->m_filePath);
        riFilePath.setFileExtension(L"ri");

        HRESULT rc = fnWriteRIFile(riFilePath.asString().asCharArray(), &rInfo);
        GT_ASSERT(rc == S_OK);
    }
}


bool tpAppController::ActivateItem(QTreeWidgetItem* pItemToActivate)
{
    bool retVal = false;

    if (nullptr == m_pApplicationTree)
    {
        m_pApplicationTree = tpTreeHandler::Instance().GetApplicationTree();
    }

    GT_IF_WITH_ASSERT((nullptr != m_pApplicationTree) && (nullptr != pItemToActivate))
    {
        afApplicationTreeItemData* pActivatedItemData = m_pApplicationTree->getTreeItemData(pItemToActivate);
        GT_IF_WITH_ASSERT(nullptr != pActivatedItemData)
        {
            tpSessionTreeNodeData* pExtendedData = dynamic_cast<tpSessionTreeNodeData*>(pActivatedItemData->extendedItemData());

            if (nullptr != pExtendedData)
            {
                afTreeItemType itemType = pActivatedItemData->m_itemType;

                // If this is the current item in the controller send line 1 as a signal to listen to the events:
                osDirectory sessionDir = pExtendedData->SessionDir();
                QString sessionNameInEvent = ProfileApplicationTreeHandler::GetProjectNameFromSessionDir(sessionDir);

                gtString viewTitle = acQStringToGTString(pExtendedData->m_displayName);

                switch (itemType)
                {
                    case AF_TREE_ITEM_PROFILE_SESSION:
                    case AF_TREE_ITEM_TP_OVERVIEW:
                    {
                        apMDIViewCreateEvent openFileEvent(AF_STR_ThreadProfileViewsCreatorID, pActivatedItemData->m_filePath, viewTitle, 0, 0);
                        apEventsHandler::instance().registerPendingDebugEvent(openFileEvent);
                        retVal = true;
                    }
                    break;

                    case AF_TREE_ITEM_TP_TIMELINE:
                    {
                        apMDIViewCreateEvent openFileEvent(AF_STR_ThreadProfileViewsCreatorID, pActivatedItemData->m_filePath, viewTitle, 1, 0);
                        apEventsHandler::instance().registerPendingDebugEvent(openFileEvent);
                        retVal = true;
                    }
                    break;

                    default:
                        GT_ASSERT(false);
                        break;
                }

                // Select the item in tree:
                m_pApplicationTree->selectItem(pActivatedItemData, false);
            }
        }

    }
    return retVal;

}

void tpAppController::ProjectOpened()
{
    gtList<gtString> sessionsPathAsStrList;
    GetThreadProfilingSessionsList(sessionsPathAsStrList);

    // Create a session for each item:
    gtList<gtString>::iterator sessionsIt = sessionsPathAsStrList.begin();

    for (; sessionsIt != sessionsPathAsStrList.end(); sessionsIt++)
    {
        // Get the current session name:
        gtString currentSession = (*sessionsIt);

        // Add the session:
        CreateLoadedSession(currentSession);
    }
}

void tpAppController::GetThreadProfilingSessionsList(gtList<gtString>& sessionsPathAsStrList)
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
    bool isExsistingProjectDirectory = projectDirectory.exists();

    if (isExsistingProjectDirectory && projectDirectory.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_ASCENDING, sessionsDirectoriesList))
    {
        // check each path if it has a '.cxldb' file that shows it is a power profiling session:
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
            currentSessionFilePath.setFileExtension(CP_STR_ThreadProfileExtensionW);

            // check if the file exits and if it does add it to the list:
            if (currentSessionFilePath.exists())
            {
                sessionsPathAsStrList.push_back(currentSessionFilePath.asString());
            }
        }
    }
}

void tpAppController::CreateLoadedSession(gtString& sessionFilePath)
{
    // Create the session data:
    osFilePath sessionPath(sessionFilePath);
    osDirectory sessionDirectory;
    sessionPath.getFileDirectory(sessionDirectory);
    gtString projectName = acQStringToGTString(ProfileApplicationTreeHandler::GetProjectNameFromSessionDir(sessionDirectory));

    // Create the session item data:
    tpSessionTreeNodeData* pRetVal = new tpSessionTreeNodeData();
    pRetVal->m_pParentData = new afApplicationTreeItemData;

    gtString projectDir = sessionDirectory.directoryPath().fileDirectoryAsString();

    pRetVal->m_name = acGTStringToQString(projectName);
    pRetVal->m_displayName = pRetVal->m_name;
    pRetVal->m_profileTypeStr = PM_profileTypeThreadProfile;

    pRetVal->m_isImported = false;
    pRetVal->m_projectName = acGTStringToQString(projectName);

    pRetVal->m_commandArguments.clear();

    pRetVal->m_pParentData->m_filePath = sessionFilePath;
    pRetVal->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
    pRetVal->m_shouldProfileEntireDuration = true;

    ProfileApplicationTreeHandler::instance()->AddSession(pRetVal, false);

}

void tpAppController::OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir)
{
    // Get the MDI creator:
    tpMDIViewCreator::Instance().OnSessionRename(pRenamedSessionData, oldSessionFilePath, oldSessionDir);
}

void tpAppController::OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    tpMDIViewCreator::Instance().OnBeforeSessionRename(pAboutToRenameSessionData, isRenameEnabled, renameDisableMessage);
}

void tpAppController::OnImportSession(const QString& strSessionFilePath, bool& wasImported)
{
    wasImported = false;

    // Extract the import file extension, and check if this is a profile session:
    osFilePath importedSessionFilePath(acQStringToGTString(strSessionFilePath));
    gtString importedFileExtension;
    importedSessionFilePath.getFileExtension(importedFileExtension);

    if (importedFileExtension.compareNoCase(CP_STR_ThreadProfileExtensionW) == 0)
    {
        osDirectory sessionOldDirectory;
        importedSessionFilePath.getFileDirectory(sessionOldDirectory);

        gtString importProfile;
        importedSessionFilePath.getFileName(importProfile);
        osDirectory projectPath;
        afProjectManager::instance().currentProjectFilePath().getFileDirectory(projectPath);
        gtString projName = afProjectManager::instance().currentProjectSettings().projectName();

        // Allocate a new session data:
        tpSessionTreeNodeData* pImportSessionData = new tpSessionTreeNodeData;
        pImportSessionData->m_pParentData = new afApplicationTreeItemData;

        // Set the imported session data parameters:
        gtString profileFileName;
        importedSessionFilePath.getFileName(profileFileName);
        QString sessionName = acGTStringToQString(profileFileName);

        pImportSessionData->m_displayName = QString("%1 (%2)").arg(sessionName).arg(PM_STR_ImportedSessionPostfix);
        pImportSessionData->m_profileTypeStr = PM_profileTypeThreadProfile;
        pImportSessionData->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;


        osDirectory baseDir;
        ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projName, projectPath, profileFileName, baseDir);

        if (sessionOldDirectory.directoryPath() != baseDir.directoryPath())
        {
            // Update the path for the imported profile file path:
            pImportSessionData->m_pParentData->m_filePath = baseDir.directoryPath();
            pImportSessionData->m_pParentData->m_filePath.setFileExtension(CP_STR_ThreadProfileExtensionW);
            pImportSessionData->m_pParentData->m_filePath.setFileName(profileFileName);

            // Copy the file to the new directory:
            bool rc = osCopyFile(importedSessionFilePath, pImportSessionData->m_pParentData->m_filePath, false);
            GT_ASSERT(rc);

            pImportSessionData->m_isImported = true;

            ReadRunInfo(strSessionFilePath, pImportSessionData);


            // Add the imported session to the tree:
            ProfileApplicationTreeHandler::instance()->AddSession(pImportSessionData, true);

            //Save the session list to the project
            afApplicationCommands::instance()->OnFileSaveProject();

            wasImported = true;
        }
    }
}

void tpAppController::OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete)
{
    GT_UNREFERENCED_PARAMETER(deleteType);

    // Get the data related to session that is about to be deleted:
    tpSessionTreeNodeData* pSessionData = qobject_cast<tpSessionTreeNodeData*>(ProfileApplicationTreeHandler::instance()->GetSessionTreeNodeData(deletedSessionId));

    if ((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        canDelete = true;

        if (canDelete)
        {
            // Get the MDI views creator:
            tpMDIViewCreator::Instance().OnSessionDelete(pSessionData->m_pParentData->m_filePath.asString());
        }
    }
}

bool tpAppController::IsSpecialExetableCaseSet()
{
    return !afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID().isEmpty();
}

void tpAppController::DisplayProcessRunningHTML()
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

bool tpAppController::LaunchProcess()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(nullptr != m_pCurrentlyRunningSessionData)
    {
#pragma message ("TODO: TP : unite this code for CPU, power, and thread profile")

        osProcessHandle processHandle;
        osThreadHandle processThreadHandle;
        osProcessId processId;
        bool bCreateWindow = true;
        bool isWindowsStoreApp = !afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID().isEmpty();
        gtString projectCmdlineArguments = afProjectManager::instance().currentProjectSettings().commandLineArguments();
        gtString exePath = acQStringToGTString(m_pCurrentlyRunningSessionData->m_exeFullPath);

        if (isWindowsStoreApp)
        {
            // Execute the windows store app:
            osFilePath filePath;
            retVal = osLaunchSuspendedWindowsStoreApp(exePath, projectCmdlineArguments, processId, processHandle, filePath);
        }

        else
        {
            // execute the application
            retVal = osLaunchSuspendedProcess(exePath, projectCmdlineArguments, acQStringToGTString(m_pCurrentlyRunningSessionData->m_workingDirectory),
                                              processId, processHandle, processThreadHandle, bCreateWindow, true);
        }

        if (retVal)
        {
            if (isWindowsStoreApp)
            {
                osResumeSuspendedWindowsStoreApp(processHandle, true);
            }
            else
            {
                osResumeSuspendedProcess(processId, processHandle, processThreadHandle, true);
            }

            // Create a thread to monitor the application:
            m_pMonitorProcessThread = new SharedProfileProcessMonitor(processId, CP_STR_ThreadProfileExtensionW);
            m_pMonitorProcessThread->execute();
        }

        m_pCurrentlyRunningSessionData->m_pid = (AMDTProcessId)processId;
    }
    return retVal;
}

void tpAppController::ReadRunInfo(const QString& strSessionFilePath, tpSessionTreeNodeData* pSessionData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionData != nullptr)
    {
        RunInfo runInfo;
        osFilePath riFilePath = acQStringToGTString(strSessionFilePath);
        riFilePath.setFileExtension(L"ri");
        HRESULT hr = fnReadRIFile(riFilePath.asString().asCharArray(), &runInfo);
        GT_ASSERT(hr == S_OK);

        // Initialize the session data from runINfo
        pSessionData->m_exeFullPath = acGTStringToQString(runInfo.m_targetPath);
        pSessionData->m_workingDirectory = acGTStringToQString(runInfo.m_wrkDirectory);
        pSessionData->m_commandArguments = acGTStringToQString(runInfo.m_cmdArguments);
        pSessionData->m_envVariables = runInfo.m_envVariables;
        pSessionData->m_startTime = acGTStringToQString(runInfo.m_profStartTime);
        pSessionData->m_endTime = acGTStringToQString(runInfo.m_profEndTime);
        pSessionData->m_pid = runInfo.m_executedPID;
    }
}
