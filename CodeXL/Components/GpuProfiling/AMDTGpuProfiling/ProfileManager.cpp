//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileManager.cpp $
/// \version $Revision: #182 $
/// \brief :  This file contains ProfileManager
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileManager.cpp#182 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <string>
#include <sstream>

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>


#include <AMDTOSWrappers/Include/osDirectorySerializer.h>



// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// SharedProfileManager:
#include <SharedProfileManager.h>


// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerPlugin.h>
#include <AMDTGpuProfiling/gpSessionView.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/OpenCLTraceSettingPage.h>
#include <AMDTGpuProfiling/CounterSelectionSettingPage.h>
#include <AMDTGpuProfiling/SessionWindow.h>
#include <AMDTGpuProfiling/TraceView.h>
#include <AMDTGpuProfiling/CLAPIFilterManager.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <Version.h>

#define GPU_PROFILER_TERMINATION_TIMEOUT 2000

/// Static list of file extensions that should be renamed when the session is renamed
QList<gtString> ProfileManager::m_sAdditionalFileExtensionsToRename = { GP_HTML_FileExtensionW, GP_Occupancy_FileExtensionW };


///< Static list of file extensions that should be renamed when the session is renamed

ProfileManager::ProfileManager() : m_pSessionExplorer(nullptr), m_pCurrentProjectSettings(nullptr), m_pProfileProcessMonitor(nullptr), m_pPagesGenerationProcessMonitor(nullptr),
    m_pRemoteProfilingTask(nullptr), m_tempEnvVarFile(nullptr), m_pSpecificKernelsFile(nullptr), m_pFrameAnalysisMode(nullptr)
{
    // Register as an events observer
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
    m_GPUProfilerProcessId = osProcessId(-1);
}

ProfileManager::~ProfileManager()
{
    SAFE_DELETE(m_pCurrentProjectSettings);
    SAFE_DELETE(m_tempEnvVarFile);
    SAFE_DELETE(m_pProfileProcessMonitor);
    SAFE_DELETE(m_pPagesGenerationProcessMonitor);
    SAFE_DELETE(m_pRemoteProfilingTask);
    SAFE_DELETE(m_pSpecificKernelsFile);

    // Unregister as an events observer
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

void ProfileManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();


    // handle the Global var changed event
    if (eventType == apEvent::APP_GLOBAL_VARIABLE_CHANGED)
    {
        const afGlobalVariableChangedEvent& globalVarChangedEvent = dynamic_cast<const afGlobalVariableChangedEvent&>(eve);
        // Get id of the global variable that was changed
        afGlobalVariableChangedEvent::GlobalVariableId variableId = globalVarChangedEvent.changedVariableId();

        // If the project file path was changed
        if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
        {
            ProjectOpened();
        }
    }
    else if (eventType == apEvent::AP_PROFILE_PROGRESS_EVENT)
    {
        const apProfileProgressEvent& progressEvent = dynamic_cast<const apProfileProgressEvent&>(eve);

        if (progressEvent.profileName() == GPU_STR_GENERAL_SETTINGS)
        {
            int progressValue = progressEvent.value();

            if (progressValue != 0)
            {
                if (progressValue != 100)
                {
                    afProgressBarWrapper::instance().updateProgressBar(progressValue);
                }
                else
                {
                    afProgressBarWrapper::instance().hideProgressBar();
                }
            }

            gtString strProgress = progressEvent.progress();

            if (!strProgress.isEmpty())
            {
                afProgressBarWrapper::instance().setProgressText(strProgress);
            }

            vetoEvent = true;
        }
    }
    else if (eventType == apEvent::AP_PROFILE_PROCESS_TERMINATED)
    {
        const apProfileProcessTerminatedEvent& processTermEvent = dynamic_cast<const apProfileProcessTerminatedEvent&>(eve);

        if (processTermEvent.profilerName() == GPU_STR_GENERAL_SETTINGS)
        {

            ProfileProcessMonitor::ProfileServerRunType runType = static_cast<ProfileProcessMonitor::ProfileServerRunType>(processTermEvent.ProfileType());

            bool isRemoteSession = afProjectManager::instance().currentProjectSettings().isRemoteTarget();

            if (isRemoteSession && runType == ProfileProcessMonitor::ProfileServerRunType_Unknown)
            {
                runType = ProfileProcessMonitor::ProfileServerRunType_Profile;
            }

            ProfilingFinishedHandler(processTermEvent.processExitCode(), runType);
            vetoEvent = true;
        }
    }

    else if (eventType == apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT)
    {
        // Get the activation event:
        const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

        // Call the activation event handler:
        OnTreeItemActivatedEvent(activationEvent);
    }
    else if (eventType == apEvent::AP_EXECUTION_MODE_CHANGED_EVENT)
    {
        GT_IF_WITH_ASSERT(m_pFrameAnalysisMode != nullptr)
        {
            m_pFrameAnalysisMode->onExecutionModeChanged();
        }
    }
}

const wchar_t* ProfileManager::eventObserverName() const
{
    return L"GPUProfilerManagerEventsObserver";
}

void ProfileManager::SetupGPUProfiling()
{
    // Create the Frame Analysis Mode
    m_pFrameAnalysisMode = new gpExecutionMode;
    m_pFrameAnalysisMode->Initialize();

    SharedProfileManager::instance().registerProfileType(acQStringToGTString(GP_profileTypePerformanceCountersWithPrefix), this, GPU_STR_TRACE_PROJECT_TREE_PATH_STR);
    SharedProfileManager::instance().registerProfileType(acQStringToGTString(GP_profileTypeApplicationTraceWithPrefix), this, GPU_STR_APP_TRACE_PROJECT_TREE_PATH_STR);

    // Register the tree handlers
    ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(PM_profileTypeApplicationTrace, &gpTreeHandler::Instance());
    ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(PM_profileTypePerformanceCounters, &gpTreeHandler::Instance());

    afExecutionModeManager::instance().registerExecutionMode(m_pFrameAnalysisMode);
    afPluginConnectionManager::instance().registerRunModeManager(m_pFrameAnalysisMode);

    // Add here the tree handler for the Frame Analysis registration.
    ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(PM_profileTypeFrameAnalysis, &gpTreeHandler::Instance());

    connect(&(SharedProfileManager::instance()), SIGNAL(profileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)), this, SLOT(ProfileStartedHandler(const gtString&, const spISharedProfilerPlugin * const, osProcessId)));
    connect(&(SharedProfileManager::instance()), SIGNAL(profileStopped(const spISharedProfilerPlugin * const, bool)), this, SLOT(onProfileStopped(const spISharedProfilerPlugin * const, bool)));
    connect(this, SIGNAL(ProfilingFinished(bool, QString, GPUSessionTreeItemData*)), &(SharedProfileManager::instance()), SLOT(onProfileEnded()));

    // if the session explorer has been created, then hook ourselves up to it.  If not, then connect to the signal that will notify us of it being created
    m_pSessionExplorer = ProfileApplicationTreeHandler::instance();

    HookupSessionExplorer();

    //Create current project settings with default values.
    m_pCurrentProjectSettings = new(std::nothrow) ProjectSettings();

}

void ProfileManager::HookupSessionExplorer()
{
    if (m_pSessionExplorer != nullptr)
    {
        connect(m_pSessionExplorer, SIGNAL(SessionDeleted(ExplorerSessionId, SessionExplorerDeleteType, bool&)), this, SLOT(SessionDeletedHandler(ExplorerSessionId, SessionExplorerDeleteType, bool&)));
        connect(m_pSessionExplorer, SIGNAL(SessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)), this, SLOT(SessionRenamedHandler(SessionTreeNodeData*, const osFilePath&, const osDirectory&)));
        connect(m_pSessionExplorer, SIGNAL(FileImported(const QString&, bool&)), this, SLOT(OnImportSession(const QString&, bool&)));
        connect(m_pSessionExplorer, SIGNAL(BeforeSessionRename(SessionTreeNodeData*, bool&, QString&)), this, SLOT(OnBeforeSessionRename(SessionTreeNodeData*, bool&, QString&)));

        m_pSessionExplorer->AddImportFileFilter("GPU Performance Counter Output File", "*.csv", PM_STR_PROFILE_MODE);
        m_pSessionExplorer->AddImportFileFilter("Application Trace Output File", "*.atp", PM_STR_PROFILE_MODE);
        m_pSessionExplorer->AddImportFileFilter("Frame Analysis archived session", "*." + acGTStringToQString(AF_STR_frameAnalysisArchivedFileExtension), PM_STR_FrameAnalysisMode);

        // if there are sessions that were added/created before the explorer was created, add them to the explorer now
        for (QList<GPUSessionTreeItemData*>::iterator i = m_deferredSessionList.begin(); i != m_deferredSessionList.end(); ++i)
        {
            AddSessionToExplorer(*i, false, false);
        }

        m_deferredSessionList.clear();
    }
}

void ProfileManager::AddSessionToExplorer(GPUSessionTreeItemData* pSessionData, bool doShow, bool isNewSession)
{
    (void)(isNewSession); // Unused variable

    GT_IF_WITH_ASSERT(pSessionData != nullptr)
    {
        QString sessionProjectName = pSessionData->m_projectName;

        if (m_pSessionExplorer != nullptr)
        {
            // Add this session to the sessions tree, with the session data:
            ExplorerSessionId sessionId = m_pSessionExplorer->AddSession(pSessionData, doShow);
            GT_ASSERT(sessionId != SESSION_ID_ERROR);
        }
        else
        {
            // explorer is not available; show the session and add the session to the deferred session list to be added to the explorer later
            // Also, notify the naming helper of the session being added (since the explorer won't do it for us here)
            m_deferredSessionList.append(pSessionData);
        }
    }
}

void ProfileManager::ProfileStartedHandler(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processID)
{
    GT_UNREFERENCED_PARAMETER(processID);
    QString strError;
    bool retVal = true;

    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        if (profileTypeStr == acQStringToGTString(GP_profileTypeApplicationTraceWithPrefix))
        {
            retVal = ProfileProject(API_TRACE, strError);
        }
        else if (profileTypeStr == acQStringToGTString(GP_profileTypePerformanceCountersWithPrefix))
        {
            retVal = ProfileProject(PERFORMANCE, strError);
        }

        if (!retVal)
        {
            if (!strError.trimmed().isEmpty())
            {
                Util::ShowErrorBox(strError);
            }

            emit ProfilingFinished(retVal, strError, nullptr);
        }
    }
}

void ProfileManager::onProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit)
{
    GT_UNREFERENCED_PARAMETER(stopAndExit);

    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        if (m_GPUProfilerProcessId != osProcessId(-1))
        {
            // Kill the children of the GPU profiler. then let it finish its tasks and terminate normally
            // if it did not end in a defined period of time, kill it. This is a brute force way of killing GPU profiler and is used
            // only when exiting CodeXL. later when Stop profiling is implemented
            // we'll need to check if this is a regular stop, or is it a stop and exit
            osTerminateChildren(m_GPUProfilerProcessId);

            // wait a bit for the GPU profiler to terminate normally after its child terminated.
            // if it does not terminate then kill it
            bool rcTerminated = osWaitForProcessToTerminate(m_GPUProfilerProcessId, GPU_PROFILER_TERMINATION_TIMEOUT);

            if (!rcTerminated)
            {
                osTerminateProcess(m_GPUProfilerProcessId);
            }

            m_GPUProfilerProcessId = osProcessId(-1);

            if (m_pProfileProcessMonitor != nullptr)
            {
                // delete the monitor also terminates it
                delete m_pProfileProcessMonitor;
                m_pProfileProcessMonitor = nullptr;
            }
        }
    }
}

void ProfileManager::ProfilingFinishedHandler(int exitCode, ProfileProcessMonitor::ProfileServerRunType runType)
{
    SAFE_DELETE(m_tempEnvVarFile);
    SAFE_DELETE(m_pSpecificKernelsFile);

    // End the profile session:
    osDebugLog::instance().EndSession();

    afProgressBarWrapper::instance().hideProgressBar();

    switch (runType)
    {
        // Note:  this SAFE_DELETE call is not done here, since there were some rare
        // cases where deleting the object there caused the osThread object to be deleted before its
        // threadEntryPoint function was completed, leading to heap corruption (see TT BUG384639).
        // Moving it here ensures that the thread is really complete before we delete the object.

        case ProfileProcessMonitor::ProfileServerRunType_Profile:
            HandleProfileFinished(exitCode);
            break;

        case ProfileProcessMonitor::ProfileServerRunType_GenSummary:
        {
            HandleGenSummaryFinished(exitCode);
            SAFE_DELETE(m_pPagesGenerationProcessMonitor);
            break;
        }

        case ProfileProcessMonitor::ProfileServerRunType_GenOccupancy:
        {
            HandleGenOccupancyFinished(exitCode);
            SAFE_DELETE(m_pPagesGenerationProcessMonitor);
            break;
        }

        case ProfileProcessMonitor::PerfStudioServerRunType_Application:
            if (m_pFrameAnalysisMode != nullptr)
            {
                m_pFrameAnalysisMode->ApplicationEnded();
            }

            break;

        default:
            GT_ASSERT(false);
            break;
    }
}

void ProfileManager::SummaryPagesGenerationFinishedHandler(bool success, const QString& strError)
{
    disconnect(this, SIGNAL(SummaryPagesGenerationFinished(bool, const QString&)), this, SLOT(SummaryPagesGenerationFinishedHandler(bool, const QString&)));

    if (success)
    {
        AddImportedSession(m_strImportedSessionName, API_TRACE, m_strImportedFileName);
        m_strImportedSessionName.clear();
        m_strImportedFileName.clear();
    }
    else
    {
        Util::ShowErrorBox(strError);
    }
}

void ProfileManager::OnTreeItemActivatedEvent(const apMonitoredObjectsTreeActivatedEvent& activationEvent)
{
    // Get the pItem data;
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

    if (pItemData != nullptr)
    {
        // See if the item type if of GPU session:
        GPUSessionTreeItemData* pGPUSessionData = qobject_cast<GPUSessionTreeItemData*>(pItemData->extendedItemData());

        if (pGPUSessionData != nullptr)
        {
            gpViewsCreator::Instance()->ShowSession(pGPUSessionData, pItemData->m_itemType);
        }
    }
}

void ProfileManager::SessionDeletedHandler(ExplorerSessionId sessionId, SessionExplorerDeleteType deleteType, bool& canDelete)
{
    GPUSessionTreeItemData* pSession = GetSessionFromSessionId(sessionId);

    if ((pSession != nullptr) && (pSession->m_pParentData != nullptr))
    {
        // Close the session MDI window:
        gpViewsCreator::Instance()->HideSession(pSession->m_pParentData->m_filePath);

        // Close all the frames related to the deleted session if this is a frame analysis session
        gpUIManager::Instance()->CloseAllSessionWindows(pSession);

        // Remove session entries in the session manager:
        QString strError;
        QMessageBox::StandardButton dlgResult = QMessageBox::No;
        bool deleteSessionFiles = false;

        if (deleteType == SESSION_EXPLORER_REMOVE_FILES)
        {
            deleteSessionFiles = true;
        }
        else
        {
            if (GlobalSettings::Instance()->m_generalOpt.m_delOption == ASK)
            {
                dlgResult = acMessageBox::instance().question(QString("Delete %1 GPUSessionTreeItemData Files").arg(afGlobalVariablesManager::ProductNameA()),
                                                              "Do you want to delete the session files?",
                                                              QMessageBox::Yes | QMessageBox::No,
                                                              QMessageBox::Yes);
            }

            if ((GlobalSettings::Instance()->m_generalOpt.m_delOption == ALWAYS) ||
                (dlgResult == QMessageBox::Yes))
            {
                deleteSessionFiles = true;
            }
        }

        if (!canDelete)
        {
            if (!strError.trimmed().isEmpty())
            {
                QString strErrorPrefix = "An error occurred when deleting session \'";
                strErrorPrefix.append(pSession->m_name).append("\':\n\n");
                strError.prepend(strErrorPrefix);
                Util::ShowWarningBox(strError);
            }
        }

        // Remove the session from the sessions manager:
        SessionManager::Instance()->RemoveSession(pSession, deleteSessionFiles);
        canDelete = true;
    }
}

void ProfileManager::OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    // Close the session file, and all the frames files if it's opened
    GT_IF_WITH_ASSERT((pAboutToRenameSessionData != nullptr) && (pAboutToRenameSessionData->m_pParentData != nullptr))
    {
        afApplicationCommands::instance()->closeFile(pAboutToRenameSessionData->m_pParentData->m_filePath);
        gpSessionTreeNodeData* pGPSessionData = qobject_cast<gpSessionTreeNodeData*>(pAboutToRenameSessionData);

        if (pGPSessionData != nullptr)
        {
            QList<FrameIndex> framesIndices;
            gpUIManager::Instance()->GetListOfFrameFolders(pAboutToRenameSessionData->m_pParentData->m_filePath, framesIndices);

            for (int i = 0; i < framesIndices.size(); i++)
            {
                osFilePath traceFilePath = gpTreeHandler::BuildFrameChildFilePath(pGPSessionData, AF_TREE_ITEM_GP_FRAME_TIMELINE, framesIndices[i]);
                afApplicationCommands::instance()->closeFile(traceFilePath);
            }
        }
    }

    // Get all trace files from server
    bool rc = gpTreeHandler::Instance().PrepareTraceForSessionFrames(pAboutToRenameSessionData->SessionDir(), pAboutToRenameSessionData);
    isRenameEnabled = rc;

    if (!isRenameEnabled)
    {
        QString remoteTargetName = acGTStringToQString(afProjectManager::instance().currentProjectSettings().remoteTargetName());

        if (!afProjectManager::instance().currentProjectSettings().isRemoteTarget())
        {
            remoteTargetName = GP_Str_LocalHost;
        }

        renameDisableMessage = QString(GP_Str_ErrorCannotRenameServerUnavailable).arg(remoteTargetName);
    }
}
void ProfileManager::SessionRenamedHandler(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDirectory)
{
    GT_UNREFERENCED_PARAMETER(oldSessionDirectory);

    if (pRenamedSessionData != nullptr)
    {
        bool isFASession = (pRenamedSessionData->m_profileTypeStr == PM_profileTypeFrameAnalysis);

        if (isFASession)
        {
            // This is a frame analysis session. Handle the specific file rename for FA sessions
            HandleFASessionRename(pRenamedSessionData, oldSessionFilePath);
        }
        else
        {
            // This is a GPU session. Handle the specific file rename for GPU sessions
            HandleGPUSessionRename(pRenamedSessionData, oldSessionFilePath);

            // Update the MDI window title:
            gpViewsCreator::Instance()->UpdateTitleString(oldSessionFilePath, pRenamedSessionData->m_pParentData->m_filePath);
        }
    }
}

void ProfileManager::HandleGPUSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath)
{
    // Downcast the item data to a GPU profile item data
    GPUSessionTreeItemData* pGPUData = qobject_cast<GPUSessionTreeItemData*>(pRenamedSessionData);

    if (pGPUData != nullptr)
    {
        // Get the before rename occupancy file path:
        osFilePath occupancyFilePath(acQStringToGTString(pGPUData->OccupancyFile()));
        occupancyFilePath.setFileDirectory(pRenamedSessionData->SessionDir());

        // Now occupancyFilePath contains the current location of the occupancy file. RENAMED_FOLDER_NAME\OLD_NAME.occupancy.
        // We want to rename it on disk to: RENAMED_FOLDER_NAME\RENAMED_FILE_NAME.occupancy:
        osFilePath renamedOccupancyFilePath = occupancyFilePath;
        renamedOccupancyFilePath.setFileName(acQStringToGTString(pRenamedSessionData->m_displayName));

        // Rename the file on disk:
        bool rc = QFile::rename(acGTStringToQString(occupancyFilePath.asString()), acGTStringToQString(renamedOccupancyFilePath.asString()));
        GT_ASSERT(rc);

        // Now set the new occupancy file on the item data:
        pGPUData->SetOccupancyFile(acGTStringToQString(renamedOccupancyFilePath.asString()));

        // Rename the additional files:
        QStringList additionalFiles = pGPUData->GetAdditionalFiles();
        QStringList newAdditionalFiles;

        foreach (QString fileStr, additionalFiles)
        {
            // Rename the additional file string
            osFilePath oldAdditionalFilePath(acQStringToGTString(fileStr));
            oldAdditionalFilePath.setFileDirectory(pRenamedSessionData->SessionDir());
            osFilePath newAdditionalFilePath = oldAdditionalFilePath;

            // The additional file has multiple file extension (OLD_NAME.CLBestPractices.html for example)
            // We should replace OLD_NAME by NEW_NAME
            gtString oldFileName, currentFileName, fileExtension;
            oldSessionFilePath.getFileName(oldFileName);
            oldAdditionalFilePath.getFileName(currentFileName);
            oldAdditionalFilePath.getFileExtension(fileExtension);

            // Do not change the rules file name
            if (m_sAdditionalFileExtensionsToRename.contains(fileExtension))
            {
                currentFileName.replace(oldFileName, acQStringToGTString(pRenamedSessionData->m_displayName));

                newAdditionalFilePath.setFileName(currentFileName);

                // Rename the file on disk (if it wasn't already moved - the occupancy file is copied before)
                if (oldAdditionalFilePath.exists())
                {
                    bool rc = QFile::rename(acGTStringToQString(oldAdditionalFilePath.asString()), acGTStringToQString(newAdditionalFilePath.asString()));
                    GT_ASSERT(rc);
                }
            }

            newAdditionalFiles << acGTStringToQString(newAdditionalFilePath.asString());
        }

        // Set the renamed additional files list:
        pGPUData->SetAdditionalFiles(newAdditionalFiles);

        // Rename all children's file path
        for (int i = AF_TREE_ITEM_PROFILE_GPU_SUMMARY; i < AF_TREE_ITEM_PROFILE_GPU_LAST_SUMMARY_ITEM_TYPE; i++)
        {
            // Get the item data for the parent session
            afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(pRenamedSessionData->m_pParentData);

            if (pSessionItemData != nullptr)
            {
                afApplicationTreeItemData* pChildData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, (afTreeItemType)i);

                if (pChildData != nullptr)
                {
                    pChildData->m_filePath = pRenamedSessionData->m_pParentData->m_filePath;
                    pChildData->m_filePathLineNumber = i;
                }
            }
        }
    }
}

void ProfileManager::HandleFASessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath)
{
    // Downcast the item data to a FA item data
    gpSessionTreeNodeData* pGPData = qobject_cast<gpSessionTreeNodeData*>(pRenamedSessionData);

    if (pGPData != nullptr)
    {
        // Sanity check
        GT_IF_WITH_ASSERT(pRenamedSessionData->m_pParentData != nullptr)
        {
            // Find the frame folders for the renamed session
            QList<FrameIndex> framesForSession;
            gpUIManager::Instance()->GetListOfFrameFolders(pRenamedSessionData->m_pParentData->m_filePath, framesForSession);

            for (FrameIndex frameIndex : framesForSession)
            {
                // Calculate the frame paths for the frames (do not create the frame folder!)
                QDir frameDir;
                QString overviewFilePathStr, thumbFilePathStr;
                bool rc = gpUIManager::Instance()->GetPathsForFrame(oldSessionFilePath, frameIndex, frameDir, overviewFilePathStr, thumbFilePathStr, false);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Set the old file paths
                    osFilePath oldOverviewFilePath(acQStringToGTString(overviewFilePathStr));
                    osFilePath oldThumbFilePath(acQStringToGTString(thumbFilePathStr));
                    osFilePath oldltrFilePath = oldOverviewFilePath;
                    oldltrFilePath.setFileExtension(GP_LTR_FileExtensionW);

                    gtString frameFolderName;
                    if (frameIndex.first == frameIndex.second)
                    {
                        frameFolderName.appendFormattedString(GPU_STR_FrameSubFolderNameSingleFormat, frameIndex.first);
                    }
                    else
                    {
                        frameFolderName.appendFormattedString(GPU_STR_FrameSubFolderNameMultipleFormat, frameIndex.first, frameIndex.second);
                    }

                    // The files to rename should be in the already renamed folder
                    oldOverviewFilePath.setFileDirectory(pRenamedSessionData->SessionDir());
                    oldOverviewFilePath.appendSubDirectory(frameFolderName);
                    oldThumbFilePath.setFileDirectory(pRenamedSessionData->SessionDir());
                    oldThumbFilePath.appendSubDirectory(frameFolderName);
                    oldltrFilePath.setFileDirectory(pRenamedSessionData->SessionDir());
                    oldltrFilePath.appendSubDirectory(frameFolderName);

                    // The new files should be in the same folder, with the new display name of the sessions as file name
                    osFilePath newOverviewFilePath = oldOverviewFilePath;
                    osFilePath newThumbFilePath = oldThumbFilePath;
                    osFilePath newltrFilePath = oldltrFilePath;

                    gtString newFileName;

                    if (frameIndex.first == frameIndex.second)
                    {
                        newFileName = acQStringToGTString(QString(GPU_STR_FrameTraceFileNameFormatSingle).arg(pRenamedSessionData->m_displayName).arg(frameIndex.first));
                    }
                    else
                    {
                        newFileName = acQStringToGTString(QString(GPU_STR_FrameTraceFileNameFormatMulti).arg(pRenamedSessionData->m_displayName).arg(frameIndex.first).arg(frameIndex.second));
                    }

                    newOverviewFilePath.setFileName(newFileName);
                    newThumbFilePath.setFileName(newFileName);
                    newltrFilePath.setFileName(newFileName);

                    rc = oldOverviewFilePath.Rename(newOverviewFilePath.asString());
                    GT_ASSERT(rc);
                    rc = oldThumbFilePath.Rename(newThumbFilePath.asString());
                    GT_ASSERT(rc);
                    rc = oldltrFilePath.Rename(newltrFilePath.asString());
                    GT_ASSERT(rc);

                    rc = gpTreeHandler::UpdateFrameFilePath(pGPData, frameIndex, newltrFilePath);
                    GT_ASSERT(rc);
                }
            }
        }
    }
}

void ProfileManager::OnImportSession(const QString& strSessionFilePath, bool& imported)
{
    imported = false;

    if (!strSessionFilePath.trimmed().isEmpty())
    {
        // Extract the profile type from the session file name:
        GPUProfileType profileType = SessionManager::Instance()->GetProfileType(strSessionFilePath);
        osFilePath mainFilePath = acQStringToGTString(strSessionFilePath);

        // If this session is a GPU session:
        if (profileType != NA_PROFILE_TYPE)
        {

            QFileInfo fileInfo(strSessionFilePath);

            // Get the imported file folder:
            QString strOutputDirectory = fileInfo.path();

            if (!strOutputDirectory.endsWith(QDir::separator()))
            {
                strOutputDirectory.append(QDir::separator());
            }

            // Get the current project name:
            gtString strProjName = afProjectManager::instance().currentProjectSettings().projectName();

            // Get the current project directory:
            osDirectory sessionOSDir;
            bool rc = afProjectManager::instance().currentProjectFilePath().getFileDirectory(sessionOSDir);
            GT_ASSERT(rc);

            // Get the session display name from the session file name:
            gtString strSessionDisplayName = acQStringToGTString(fileInfo.baseName());

            // Get the name and location for the new session:
            ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(strProjName, sessionOSDir, strSessionDisplayName, sessionOSDir);

            // Copy session files to sessionOSDir and then add the session from there.
            // create a temp session so we can get the list of files associated with it
            GPUSessionTreeItemData* pImportedSessionItemData = nullptr;

            if (profileType == API_TRACE)
            {
                pImportedSessionItemData = new TraceSession(acGTStringToQString(strSessionDisplayName), QString(), strSessionFilePath, acGTStringToQString(strProjName), false);
            }
            else if (profileType == PERFORMANCE)
            {
                pImportedSessionItemData = new PerformanceCounterSession(acGTStringToQString(strSessionDisplayName), QString(), strSessionFilePath, acGTStringToQString(strProjName), false);
            }
            else if (profileType == FRAME_ANALYSIS)
            {
                gtString strSessionMainDashboardFilePath;

                if (ExtractArchivedSession(strSessionFilePath, sessionOSDir, acGTStringToQString(strProjName), strSessionMainDashboardFilePath, strSessionDisplayName))
                {
                    pImportedSessionItemData = new gpSessionTreeNodeData(acGTStringToQString(strSessionDisplayName), QString(), acGTStringToQString(strSessionMainDashboardFilePath), acGTStringToQString(strProjName), true);
                }

                mainFilePath = strSessionMainDashboardFilePath;
                osFilePath path(strSessionMainDashboardFilePath);
                path.getFileDirectory(sessionOSDir);
            }
            else
            {
                pImportedSessionItemData = new gpSessionTreeNodeData(acGTStringToQString(strSessionDisplayName), QString(), strSessionFilePath, acGTStringToQString(strProjName), false);
            }

            if (pImportedSessionItemData != nullptr)
            {

                pImportedSessionItemData->m_pParentData = new afApplicationTreeItemData;
                pImportedSessionItemData->m_pParentData->m_filePath = mainFilePath;

                // Set the profile session directory:
                pImportedSessionItemData->m_profileTypeStr = Util::GetProfileTypeName(profileType);

                // Import the file:
                imported = DoImport(pImportedSessionItemData, sessionOSDir);
            }
        }
    }
}


bool ProfileManager::ExtractArchivedSession(const QString& strSessionFilePath, osDirectory sessionOSDir, const QString& strProjName, gtString& xmlFileNewName, gtString& strSessionDisplayName)
{
    GT_UNREFERENCED_PARAMETER(strProjName);

    osDirectorySerializer dirSerailzer;
    gtString archiveFullPath = acQStringToGTString(strSessionFilePath);
    gtString archiveRootDirStr;
    bool rc = dirSerailzer.DecompressDir(archiveFullPath, sessionOSDir.directoryPath().fileDirectoryAsString(), archiveRootDirStr);

    if (rc)
    {
        // make sure name restrictions are maintained:
        // 1. session dir name should be the same as dashboard filename
        // 2. session dir name should have the following structure: Xxx ( imported - session name)
        osDirectory archiveRootDir;
        archiveRootDir.setDirectoryFullPathFromString(archiveRootDirStr);
        gtString strSessionMainDashboardFilePath;
        strSessionMainDashboardFilePath = archiveRootDir.FindFile(L"*." AF_STR_frameAnalysisDashboardFileExtension);
        GT_ASSERT(strSessionMainDashboardFilePath.isEmpty() == false);

        // make sure directory name is the same as dashboard file name
        gtString sessionName;
        gtString sessionFolderFullPath;
        gtString sessionFolderName;

        osFilePath projectPath(strSessionMainDashboardFilePath);
        gtString newDirNameStr;
        projectPath.getFileName(sessionName);
        QString sessionFolderNameCheckToRemve = QString("%1 (%2)").arg(acGTStringToQString(sessionName)).arg(PM_STR_ImportedSessionPostfix);
        sessionFolderName.appendFormattedString(L"%ls (%ls)", sessionName.asCharArray(), acQStringToGTString(PM_STR_ImportedSessionPostfix).asCharArray());

        sessionFolderFullPath = (sessionFolderName);
        strSessionDisplayName = (sessionFolderName);
        sessionFolderFullPath.prepend(osFilePath::osPathSeparator);
        sessionFolderFullPath.prepend(sessionOSDir.getParentDirectory().directoryPath().fileDirectoryAsString());
        osDirectory newDir(sessionFolderFullPath);

        if (newDir.exists())
        {
            acMessageBox::instance().information(AF_STR_InformationA, PM_STR_PROFILE_TREE_SESSION_EXIST);
            rc = false;
        }
        else
        {
            // name the session dir
            rc = archiveRootDir.rename(sessionFolderFullPath);
            // session dir and inner files must have the same name
            GT_IF_WITH_ASSERT(rc)
            {
                rc = RenameSessionFiles(archiveRootDir, sessionName, sessionFolderName);
                xmlFileNewName = archiveRootDir.FindFile(L"*." AF_STR_frameAnalysisDashboardFileExtension);
            }
        }

    }
    else
    {
        gtString osErrorMsg;
        osErrorMsg.appendFormattedString(GP_Str_ErrorFailedToOpenFrameAnalysisArchive, archiveFullPath.asCharArray());
        OS_OUTPUT_DEBUG_LOG(osErrorMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        acMessageBox::instance().critical(AF_STR_ErrorA, acGTStringToQString(osErrorMsg), QMessageBox::Ok);
    }


    return rc;
}

bool ProfileManager::RenameSessionFiles(const osDirectory& sessionDir, const gtString& stringToReplace, const gtString& newString)
{
    gtList<osFilePath> filePaths;
    // Obtaining all the
    bool rc = sessionDir.getContainedFilePaths(gtString(L"*.*"), osDirectory::SORT_BY_DATE_DESCENDING, filePaths);

    if (rc)
    {
        // Iterate over the list and add the entires (up to MAX_NUMBER_OF_RECENT_PROJECTS_TO_SHOW)
        gtList<osFilePath>::iterator headIterator = filePaths.begin();
        gtList<osFilePath>::iterator tailIterator = filePaths.end();

        // Iterating over the recent projects paths while not exceeding the maximal number of recent projects allowed
        for (; headIterator != tailIterator; headIterator++)
        {
            // Extracting the file name
            osFilePath newFilePath = *headIterator;

            gtString sessionFileName;
            newFilePath.getFileName(sessionFileName);
            sessionFileName.replace(stringToReplace, newString, false);
            newFilePath.setFileName(sessionFileName);

            rc &= headIterator->Rename(newFilePath.asString());
        }

    }

    gtList<osFilePath> subDirPaths;
    rc = sessionDir.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_DESCENDING, subDirPaths);

    if (rc)
    {
        // Iterate over the list and add the entires (up to MAX_NUMBER_OF_RECENT_PROJECTS_TO_SHOW)
        gtList<osFilePath>::iterator headIterator = subDirPaths.begin();
        gtList<osFilePath>::iterator tailIterator = subDirPaths.end();

        // Iterating over the recent projects paths while not exceeding the maximal number of recent projects allowed
        while (headIterator != tailIterator)
        {
            osDirectory childDir(*headIterator);
            rc &= RenameSessionFiles(childDir, stringToReplace, newString);
            headIterator++;
        }
    }

    return rc;

}

bool ProfileManager::DoImport(GPUSessionTreeItemData* pImportedSessionItemData, const osDirectory& importedSessionDir)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((pImportedSessionItemData != nullptr) && (pImportedSessionItemData->m_pParentData != nullptr))
    {
        // Add the project name to the session name:
        QString postfix;
        QFileInfo exeFileInfo(pImportedSessionItemData->m_exeName);
        pImportedSessionItemData->m_projectName = exeFileInfo.baseName();

        if (pImportedSessionItemData->m_projectName == afProjectManager::instance().currentProjectSettings().projectName().asASCIICharArray())
        {
            postfix.sprintf(" (%s)", PM_STR_ImportedSessionPostfix);
        }
        else if (pImportedSessionItemData->GetProfileType() != FRAME_ANALYSIS)
        {
            postfix.sprintf(" (%s - %s)", PM_STR_ImportedSessionPostfix, pImportedSessionItemData->m_projectName.toLatin1().data());
        }

        // Create a name with import postfix:
        QString strSessionNameWithImportPostfix = pImportedSessionItemData->m_displayName;
        strSessionNameWithImportPostfix += postfix;

        QString strImportedFileName;
        bool doImport = true;
        bool importedSessionHasSummaryPage = false;

        if (!pImportedSessionItemData->IsSessionFileValid())
        {
            doImport = acMessageBox::instance().question("Import GPUSessionTreeItemData",
                                                         QString("%1 does not appear to be a valid session file. Do you want to import it anyway?").arg(acGTStringToQString(pImportedSessionItemData->m_pParentData->m_filePath.asString())),
                                                         QMessageBox::Yes | QMessageBox::No,
                                                         QMessageBox::No) == QMessageBox::Yes;
        }

        if (doImport)
        {
            // Set the old sessions directory:
            pImportedSessionItemData->m_displayName = strSessionNameWithImportPostfix;

            // Copy all files needed for the imported session:
            pImportedSessionItemData->SearchForAdditionalFiles();
            QStringList additionalSessionFiles = pImportedSessionItemData->GetAdditionalFiles();
            pImportedSessionItemData->m_pParentData->m_filePath.setFileDirectory(pImportedSessionItemData->SessionDir());
            strImportedFileName = acGTStringToQString(pImportedSessionItemData->m_pParentData->m_filePath.asString());
            additionalSessionFiles.prepend(strImportedFileName);

            osFilePath sourcePath, destinationPath;
            gtString fileName, fileExt;
            destinationPath.setFileDirectory(importedSessionDir);

            foreach (QString additionalFile, additionalSessionFiles)
            {
                sourcePath.setFullPathFromString(acQStringToGTString(additionalFile));

                if (sourcePath.getFileName(fileName) && sourcePath.getFileExtension(fileExt))
                {
                    if (!importedSessionHasSummaryPage && additionalFile.endsWith(Util::ms_APISUMFILE))
                    {
                        importedSessionHasSummaryPage = true;
                    }

                    destinationPath.setFileName(fileName);
                    destinationPath.setFileExtension(fileExt);
                    osCopyFile(sourcePath, destinationPath, true);
                }
            }
        }

        if (doImport)
        {
            // Relocate the files list in the item data:
            pImportedSessionItemData->SetFilesFolder(importedSessionDir.asString());
            retVal = true;

            // The imported file was relocated, it should get the new path (in the imported path):
            strImportedFileName = acGTStringToQString(pImportedSessionItemData->m_pParentData->m_filePath.asString());

            //GPUProfileType profileType = API_TRACE;
            GPUProfileType profileType = SessionManager::Instance()->GetProfileType(strImportedFileName);

            if (qobject_cast<PerformanceCounterSession*>(pImportedSessionItemData) != nullptr)
            {
                profileType = PERFORMANCE;
            }

            // Generate summary page here (so that imported trace sessions always have summary)
            //    NOTE:  there is no guarantee that the trace is complete enough for summary pages (if APIs are filtered)

            if (profileType == API_TRACE && !importedSessionHasSummaryPage)
            {
                connect(this, SIGNAL(SummaryPagesGenerationFinished(bool, const QString&)), this, SLOT(SummaryPagesGenerationFinishedHandler(bool, const QString&)));

                QString strError;

                if (!GenerateSummaryPages(strImportedFileName, strSessionNameWithImportPostfix, strError))
                {
                    retVal = false;
                }
            }

            // If necessary, create a temp performance counters file
            gpViewsCreator::Instance()->CreateTempPCFile(pImportedSessionItemData);

            // Add the session to the tree:
            AddSessionToExplorer(pImportedSessionItemData, true, false);
        }
    }

    return retVal;
}

bool ProfileManager::HasValidProfileSettingData(GPUProfileType profileType, const apProjectSettings& projectSettings, QString& strErrorMessageOut)
{
    bool retVal = true;

    // All these validations should not be applied when in remote session.
    bool isRemoteSession = afProjectManager::instance().currentProjectSettings().isRemoteTarget();

    if (!isRemoteSession)
    {
        QString strErrorFormat = GPU_STR_ERR_FileMissing;
        QString strErrorExtraPart = "executable";
        // check that the executable exists and that it is a valid executable
        osFilePath exePathOrWorkDir = projectSettings.executablePath();

        retVal = exePathOrWorkDir.exists();

        if (retVal)
        {
            retVal = exePathOrWorkDir.isExecutable();

            if (!retVal)
            {
                strErrorFormat = GPU_STR_ERR_InvalidExecutable;
            }
            else
            {
                // check that the working directory is valid
                exePathOrWorkDir = projectSettings.workDirectory().asFilePath();
                retVal = exePathOrWorkDir.exists();

                if (!retVal)
                {
                    strErrorExtraPart = "working directory";
                }
            }
        }

        if (!retVal)
        {
            if (exePathOrWorkDir.isEmpty())
            {
                strErrorMessageOut = QString(GPU_STR_ERR_FileNotSpecified).arg(strErrorExtraPart);
            }
            else
            {
                strErrorMessageOut = strErrorFormat.arg(strErrorExtraPart).arg(acGTStringToQString(exePathOrWorkDir.asString()));
            }
        }
        else
        {
            CounterSelectionSettingWindow* counterSelectionSetting = CounterSelectionSettingWindow::Instance();

            GT_IF_WITH_ASSERT(nullptr != counterSelectionSetting)
            {
                if (profileType == PERFORMANCE && !counterSelectionSetting->IsCountersSelected())
                {
                    strErrorMessageOut = QString(GPU_STR_ERR_NoCountersSelected);
                    retVal = false;
                }
            }
        }
    }

    return retVal;
}

bool ProfileManager::GetProfilerServer(osFilePath& strServer, QString& strErrorMessageOut)
{
    strErrorMessageOut.clear();
    bool retVal = Util::GetInstallDirectory(strServer);
    GT_ASSERT(retVal);

    apProjectSettings projectSettings = afProjectManager::instance().currentProjectSettings();

    // check the bitness of the project executable -- default to 32-bit if unable to determine correct bitness
    bool is64Bit = false;

    osFilePath exePath = projectSettings.executablePath();

    if (exePath.isExecutable())
    {
        retVal = osIs64BitModule(exePath, is64Bit);
        GT_ASSERT(retVal);
    }

    // If we are on remote session on Linux, force using the 64-bit version of CodeXLGpuProfiler.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    bool isRemoteSession = projectSettings.isRemoteTarget();

    if (isRemoteSession)
    {
        is64Bit = true;
    }

#endif

    gtString profilerFileName;
    profilerFileName = profilerFileName.fromASCIIString(RCP_EXE_BASE_NAME);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

    if (!is64Bit)
    {
        profilerFileName.append(L"32");
    }

#if (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
//    profilerFileName.append(L"-d");
#endif

    if (Util::IsInternalBuild())
    {
        profilerFileName.append(L"-Internal");
    }

#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (is64Bit)
    {
        profilerFileName.append(L"-x64");
    }

//    profilerFileName.append(AMDT_DEBUG_SUFFIX_W);

#if AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
    profilerFileName.append(AMDT_BUILD_SUFFIX_W);
#endif

    strServer.setFileExtension(L"exe");
#else
#error Unknown build target!
#endif
    strServer.setFileName(profilerFileName);
    gtString profileServerAsStr = strServer.asString();

    // check whether the server component exists
    retVal = strServer.exists();

    if (!retVal)
    {
        strErrorMessageOut = QString("%1 does not exist.").arg(acGTStringToQString(strServer.asString()));
    }

    return retVal;
}

bool ProfileManager::LaunchProfilerServer(const osFilePath& strServer, const gtString& strOptions, ProfileProcessMonitor::ProfileServerRunType runType, QString& strErrorMessageOut)
{
    bool retVal = false;

    // Allow only one profile server running for
    bool canProfileRun = true;

    if (m_pProfileProcessMonitor != nullptr)
    {
        canProfileRun = (runType != ProfileProcessMonitor::ProfileServerRunType_Profile);
    }

    if (m_pPagesGenerationProcessMonitor != nullptr)
    {
        canProfileRun = (runType == ProfileProcessMonitor::ProfileServerRunType_Profile);
    }

    GT_IF_WITH_ASSERT(canProfileRun)
    {
        osFilePath workDir(osFilePath::OS_CURRENT_DIRECTORY);
        m_GPUProfilerProcessId = (osProcessId)(-1);
        osProcessHandle processHandle;
        osThreadHandle threadHandle;

        gtString strServerLaunchMsg = L"Launching profiler server: ";
        strServerLaunchMsg.append(strServer.asString());
        strServerLaunchMsg.append(L" with arguments: ");
        strServerLaunchMsg.append(strOptions);
        OS_OUTPUT_DEBUG_LOG(strServerLaunchMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

        if (osLaunchSuspendedProcess(strServer, strOptions, workDir, m_GPUProfilerProcessId, processHandle, threadHandle, false))
        {
            // Create a process monitor:
            ProfileProcessMonitor* pNewMonitor = new ProfileProcessMonitor(m_GPUProfilerProcessId, runType);
            pNewMonitor->execute();

            if (runType == ProfileProcessMonitor::ProfileServerRunType_Profile)
            {
                m_pProfileProcessMonitor = pNewMonitor;
            }
            else
            {
                m_pPagesGenerationProcessMonitor = pNewMonitor;
            }

            osResumeSuspendedProcess(m_GPUProfilerProcessId, processHandle, threadHandle, true);
            retVal = true;
        }
        else
        {
            strErrorMessageOut = "Unable to launch profiler";

            gtString osError;
            osGetLastSystemErrorAsString(osError);

            if (!osError.isEmpty())
            {
                strErrorMessageOut.append(".\n\n");
                strErrorMessageOut.append(acGTStringToQString(osError));
            }

            if (m_GPUProfilerProcessId != -1)
            {
                osTerminateProcess(m_GPUProfilerProcessId);
            }
        }
    }

    return retVal;
}

bool ProfileManager::ProfileProject(GPUProfileType profileType, QString& strErrorMessageOut)
{
    bool retVal = false;

    // Store the setting into the trace setting structure:
    OpenCLTraceOptions::Instance()->SetTraceOptions(m_pCurrentProjectSettings->m_traceOptions);

    QString strExecutableFilename;
    //bool projectCommandUsed = false;
    QString strProjectCommandArguments;
    QString strProjectWorkingDir;
    QString strProjectEnvironment;


    // For remote profiling.
    QString apiRulesFile;
    bool isCounterFileRequired   = false;
    bool isEnvVarFileRequired    = false;
    bool isApiFilterFileRequired = false;
    bool isApiRulesFileRequired  = false;

    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

    bool errInSetting = !HasValidProfileSettingData(profileType, projectSettings, strErrorMessageOut);

    if (!errInSetting)
    {
        ProfileSettingData profData(afProjectManager::instance().currentProjectFilePath(), projectSettings);

        m_profileParameters.CurrentProfileData()->Assign(profData);

        //FIXME::add a check whether ProfileParameters is valid
        m_profileParameters.SetProfileTypeValue(profileType);

        // application path
        QFileInfo fileInfo(m_profileParameters.CurrentProfileData()->ApplicationPath());
        m_profileParameters.CurrentProfileData()->SetApplicationPath(fileInfo.filePath());

        //Command line argument
        // add quotes around the command arguments only it is non-empty
        QString strCommandlineArgument;

        if (!(m_profileParameters.CurrentProfileData()->CommandlineArguments().isNull() ||
              m_profileParameters.CurrentProfileData()->CommandlineArguments().trimmed().isEmpty()
             ))
        {
            strCommandlineArgument = m_profileParameters.CurrentProfileData()->CommandlineArguments();
#if AMDT_BUILD_TARGET==GR_WIN_OS
            // On Windows, add \ before " so that cmd line won't remove it
            strCommandlineArgument = Util::ProcessCmdLineArgsStr(strCommandlineArgument);
#endif
        }

        //Working Directory
        QString strWorkingDirectory = " -w \"" + Util::ToQtPath(m_profileParameters.CurrentProfileData()->WorkingDirectory()) + "\"";

        // get the server component
        osFilePath serverFile;

        if (GetProfilerServer(serverFile, strErrorMessageOut))
        {
            QString strProfileTypeArguments;

            //ProfileParameters.ProjectName = GetProjectNameFromFullName(ProfileParameters.m_currentProfileData.ProjectInfo.Path);
            QString projName = m_profileParameters.CurrentProfileData()->ProjectInfo().m_name;
            m_profileParameters.SetProjectName(projName);

            gtString strSessionDisplayName;
            osDirectory sessionOsDir;
            osFilePath projectFilePath(acQStringToGTString(m_profileParameters.CurrentProfileData()->ProjectInfo().m_path));

            // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
            ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(acQStringToGTString(projName), projectFilePath, strSessionDisplayName, sessionOsDir);
            Util::CleanSessionDir(sessionOsDir);
            m_profileParameters.SetSessionName(acGTStringToQString(strSessionDisplayName));
            QString sessionDir = acGTStringToQString(sessionOsDir.directoryPath().asString());

            if (!sessionDir.endsWith(QDir::separator()))
            {
                sessionDir.append(QDir::separator());
            }

            sessionDir = Util::ToQtPath(sessionDir);
            m_profileParameters.SetSessionDir(sessionDir);

            m_profileParameters.SetOutputDirectory(sessionDir);

            // Environment variables file.
            QString strEnvVarsParams;

            if (!m_profileParameters.CurrentProfileData()->MergeEnvironment())
            {
                strEnvVarsParams.append(" -f");
            }

            QStringList envList = m_profileParameters.CurrentProfileData()->EnvVariableList();

            if (envList.count() > 0)
            {
                SAFE_DELETE(m_tempEnvVarFile);
                QString envVarFileName = m_profileParameters.OutputDirectory() + "EnvVarsFile";
                m_tempEnvVarFile = new(std::nothrow) QFile(envVarFileName);


                if (m_tempEnvVarFile->open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    foreach (QString str, envList)
                    {
                        QByteArray b;
                        str.append(QString("\n"));
                        b.append(str);
                        m_tempEnvVarFile->write(b);
                    }

                    m_tempEnvVarFile->flush();
                    m_tempEnvVarFile->close();
                    strEnvVarsParams.append(QString(" -E \"%1\"").arg(Util::ToQtPath(m_tempEnvVarFile->fileName())));

                    // For remote agent.
                    isEnvVarFileRequired = true;
                }
            }

            // Counter file.
            QString strCounterFile;

            QString strSessionFileName = m_profileParameters.OutputDirectory() + acGTStringToQString(strSessionDisplayName);

            if (m_profileParameters.ProfileTypeValue() == API_TRACE)
            {
                GT_IF_WITH_ASSERT(m_pCurrentProjectSettings != nullptr)
                {
                    if ((m_pCurrentProjectSettings->m_traceOptions.m_apiToTrace & APIToTrace_OPENCL) == APIToTrace_OPENCL)
                    {
                        strProfileTypeArguments += " --apitrace ";
                    }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

                    if ((m_pCurrentProjectSettings->m_traceOptions.m_apiToTrace & APIToTrace_HSA) == APIToTrace_HSA)
                    {
                        strProfileTypeArguments += " --hsatrace ";
                    }

#endif

                    if (m_pCurrentProjectSettings->m_traceOptions.m_mode == TIMEOUT)
                    {
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                        strProfileTypeArguments += " --timeout "; // don't pass --timeout option on Linux -- timeout is default, thus CodeXLGpuProfiler doesn't recognize the option on Linux
#endif
                        strProfileTypeArguments.append(QString(" --interval %1 ").arg(m_pCurrentProjectSettings->m_traceOptions.m_timeoutInterval));
                    }

                    strProfileTypeArguments.append(QString(" --maxapicalls %1 ").arg(m_pCurrentProjectSettings->m_traceOptions.m_maxAPICalls));

                    if (m_pCurrentProjectSettings->m_traceOptions.m_generateSummaryPage)
                    {
                        strProfileTypeArguments.append(" --tracesummary ");

                        QString ruleFilePath = m_profileParameters.SessionDir();

                        if (!ruleFilePath.endsWith(QDir::separator()))
                        {
                            ruleFilePath.append(QDir::separator());
                        }

                        ruleFilePath.append(GPU_RulesFullFileName);

                        if (OpenCLTraceOptions::Instance()->GenerateRules(ruleFilePath))
                        {
                            // Save the file name.
                            apiRulesFile = ruleFilePath;
                            isApiRulesFileRequired = true;

                            strProfileTypeArguments.append(QString(" --apirulesfile \"%1\" ").arg(Util::ToQtPath(ruleFilePath)));
                        }
                    }

                    if (m_pCurrentProjectSettings->m_traceOptions.m_queryRetStat)
                    {
                        strProfileTypeArguments.append(" --ret ");
                    }

                    if (m_pCurrentProjectSettings->m_traceOptions.m_generateSymInfo)
                    {
                        strProfileTypeArguments.append(" --sym ");
                    }

                    if (!m_pCurrentProjectSettings->m_traceOptions.m_collapseClGetEventInfo)
                    {
                        strProfileTypeArguments.append(" --nocollapse ");
                    }

                    if (m_pCurrentProjectSettings->m_traceOptions.m_pFilterManager->GetEnabled() && !m_pCurrentProjectSettings->m_traceOptions.m_generateSummaryPage)
                    {
                        // TODO: use StringBuilder for better performance
                        strProfileTypeArguments.append(QString(" --apifilterfile \"%1\" ")
                                                       .arg(Util::ToQtPath(m_pCurrentProjectSettings->m_traceOptions.m_pFilterManager->GetAPIFilterFile())));

                        // For remote daemon.
                        isApiFilterFileRequired = true;
                    }

                    if (m_pCurrentProjectSettings->m_traceOptions.m_generateKernelOccupancy)
                    {
                        if ((m_pCurrentProjectSettings->m_traceOptions.m_apiToTrace & APIToTrace_OPENCL) == APIToTrace_OPENCL)
                        {
                            // Occupancy is not supported for HSA yet.
                            strProfileTypeArguments += " --occupancy ";
                        }
                    }
                }

                strSessionFileName.append(".atp");
            }
            else if (m_profileParameters.ProfileTypeValue() == PERFORMANCE)
            {
                // generate the counter file settings
                strCounterFile = m_profileParameters.SessionDir();

                if (!strCounterFile.endsWith(QDir::separator()))
                {
                    strCounterFile.append(QDir::separator());
                }

                strCounterFile.append(GPU_PerformanceCountersFullFileName);

                QString strCounterFileArgument;
                // The hsapmc switch tells the backend to profile any HSA kernels that are dispatched (except those that are
                // dispatched by OCL - those kernels will continue to be profiled at the OpenCL level, not
                // the HSA level). It's perfectly ok for the front end to always include the --hsapmc switch
                // when it specifies the --perfcounter switch - the back end will do the correct thing if
                // both are specified.
                strProfileTypeArguments += " --perfcounter ";

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

                if ((m_pCurrentProjectSettings->m_counterOptions.m_api & APIToTrace_HSA) == APIToTrace_HSA)
                {
                    strProfileTypeArguments += " --hsapmc ";
                }

#endif

                if (CounterManager::Instance()->GenerateCounterFile(strCounterFile))
                {
                    isCounterFileRequired = true;
                    strCounterFileArgument = " -c \"" + Util::ToQtPath(strCounterFile) + "\"";
                }

                strProfileTypeArguments += (strCounterFileArgument + " -k all");
                strSessionFileName.append(".csv");

                if (m_pCurrentProjectSettings->m_counterOptions.m_generateKernelOccupancy)
                {
                    strProfileTypeArguments += " --occupancy ";
                }

                if (m_pCurrentProjectSettings->m_counterOptions.m_shouldCallXInitThread)
                {
                    strProfileTypeArguments += " --xinitthreads ";
                }

                if (!afProjectManager::instance().currentProjectSettings().isRemoteTarget() &&
                    ExportSpecificKernelsToFile(m_pCurrentProjectSettings->m_counterOptions.m_specificKernels))
                {
                    strProfileTypeArguments += " --kernellistfile \"" + m_pSpecificKernelsFile->fileName() + "\" ";
                }
            }

            m_profileParameters.SetSessionFile(strSessionFileName);

            // The file should not already exist. However, we will try to delete it
            // if it does exist, and return an error if we are unable to do so.
            fileInfo.setFile(m_profileParameters.SessionFile());
            bool shouldProfile = true;

            if (fileInfo.exists())
            {
                QString strMessage;

                if (!Util::RemoveFileOrDirectory(m_profileParameters.SessionFile(), strMessage))
                {
                    strErrorMessageOut = QString("Unable to delete existing session file.\n\n%1").arg(strMessage);
                    shouldProfile = false;
                }
            }

            // Note:  this SAFE_DELETE call used to be done in ProfilingFinishedHandler, but there were some rare
            // cases where deleting the object there caused the osThread object to be deleted before its
            // threadEntryPoint function was completed, leading to heap corruption (see TT BUG384639).
            // Moving it here ensures that the thread is really complete before we delete the object.
            SAFE_DELETE(m_pProfileProcessMonitor);

            GT_IF_WITH_ASSERT(m_pProfileProcessMonitor == nullptr)
            {
                if (shouldProfile)
                {
                    QString strIsSinglePass = "";
                    QString strIsNoGpuTimeCollecting = "";

                    if (CounterSelectionSettingWindow::Instance()->IsSinglePassChecked())
                    {
                        strIsSinglePass.append("--singlepass");
                        strIsNoGpuTimeCollecting.append("--nogputime");
                    }
                    else if (!CounterSelectionSettingWindow::Instance()->IsGpuTimeCollected())
                    {
                        strIsNoGpuTimeCollecting.append("--nogputime");
                    }

                    QString strOptions = QString(" --outputfile \"%1\" --sessionname \"%2\" %3 %4 %5 %6 %7 \"%8\" %9")
                                         .arg(Util::ToQtPath(m_profileParameters.SessionFile()))
                                         .arg(acGTStringToQString(strSessionDisplayName))
                                         .arg(strProfileTypeArguments)
                                         .arg(Util::ToQtPath(strWorkingDirectory))
                                         .arg(strEnvVarsParams)
                                         .arg(strIsSinglePass)
                                         .arg(strIsNoGpuTimeCollecting)
                                         .arg(Util::ToQtPath(m_profileParameters.CurrentProfileData()->ApplicationPath()))
                                         .arg(strCommandlineArgument);

                    gtString strArguments(acQStringToGTString(strOptions));

                    // Check whether we are in a remote session.
                    if (afProjectManager::instance().currentProjectSettings().isRemoteTarget())
                    {
                        // Prepare file names.
                        gtString counterFileName(L"");
                        gtString envVarsFileName(L"");
                        gtString apiFilterFileName(L"");
                        gtString __apiRulesFile(L"");

                        if (isCounterFileRequired)
                        {
                            counterFileName = acQStringToGTString(strCounterFile);
                        }

                        if (isEnvVarFileRequired)
                        {
                            QString qstrEnvVarFileName = m_tempEnvVarFile->fileName();
                            envVarsFileName = acQStringToGTString(qstrEnvVarFileName);
                        }

                        if (isApiFilterFileRequired)
                        {
                            QString qstrApiFilterFileName =
                                m_pCurrentProjectSettings->m_traceOptions.m_pFilterManager->GetAPIFilterFile();
                            apiFilterFileName = acQStringToGTString(qstrApiFilterFileName);
                        }

                        if (isApiRulesFileRequired)
                        {
                            __apiRulesFile = acQStringToGTString(apiRulesFile);
                        }

                        bool isKernelSpecific = false;
                        gtString specificKernels;

                        // Handle remote profiling of specific kernels.
                        if (m_pCurrentProjectSettings != nullptr &&
                            !m_pCurrentProjectSettings->m_counterOptions.m_specificKernels.isEmpty())
                        {
                            isKernelSpecific = true;
                            specificKernels = m_pCurrentProjectSettings->m_counterOptions.m_specificKernels;
                        }

                        // Extract the CodeXLGpuProfiler output dir.
                        QString qstrOutDir = m_profileParameters.OutputDirectory();
                        gtString sprofOutDir = acQStringToGTString(qstrOutDir);

                        // If we have an old task, clean it.
                        SAFE_DELETE(m_pRemoteProfilingTask);

                        // Launch remote GPU profiling asynchronously.
                        // First create the async task object.
                        m_pRemoteProfilingTask = new(std::nothrow)AsyncRemoteGpuProfilingTask(isCounterFileRequired,
                                                                                              counterFileName, isEnvVarFileRequired, isApiRulesFileRequired, isApiFilterFileRequired, __apiRulesFile, strArguments,
                                                                                              envVarsFileName, apiFilterFileName, sprofOutDir, isKernelSpecific, specificKernels);


                        // Register to error message broadcasting event.
                        m_pRemoteProfilingTask->Register(this);


                        // Go.
                        m_pRemoteProfilingTask->execute();
                        retVal = true;
                    }
                    else
                    {
                        retVal = LaunchProfilerServer(serverFile, strArguments, ProfileProcessMonitor::ProfileServerRunType_Profile, strErrorMessageOut);
                    }
                }
            }
        }
    }

    return retVal;
}

bool ProfileManager::GenerateSummaryPages(const QString& strInputAtpFile, const QString& strSessionName, QString& strErrorMessageOut)
{
    bool retVal = false;
    // get the server component
    osFilePath serverFile;

    if (GetProfilerServer(serverFile, strErrorMessageOut))
    {
        gtString strArguments = L"--tracesummary --atpfile ";
        strArguments.appendFormattedString(L"\"%ls\"", acQStringToGTString(strInputAtpFile).asCharArray());

        m_strImportedSessionName = strSessionName;
        m_strImportedFileName = strInputAtpFile;
        retVal = LaunchProfilerServer(serverFile, strArguments, ProfileProcessMonitor::ProfileServerRunType_GenSummary, strErrorMessageOut);
    }

    return retVal;
}

bool ProfileManager::GenerateOccupancyPage(GPUSessionTreeItemData* pSessionData, const IOccupancyInfoDataHandler* pOccInfo, int callIndex, QString& strErrorMessageOut)
{
    bool retVal = false;
    osFilePath serverFile;
    std::string errorMessage;
    strErrorMessageOut = "Error";
    if (pSessionData != nullptr && pOccInfo != nullptr)
    {
        QString sessionDirStr = QString::fromWCharArray(pSessionData->SessionDir().directoryPath().asString().asCharArray());

        // Append a path separator
        wchar_t pathSeparator[2] = { 0 };
        pathSeparator[0] = osFilePath::osPathSeparator;
        sessionDirStr.append(QString::fromWCharArray(pathSeparator));

        // Generate the path of the output file
        m_strOutputOccHTMLPage = QString("%1%2_%3_%4_Occupancy.html")
            .arg(sessionDirStr)
            .arg(QString().fromStdString(pOccInfo->GetKernelName()))
            .arg(pOccInfo->GetThreadId())
            .arg(callIndex);

        std::string outputFile = m_strOutputOccHTMLPage.toStdString();

        bool occChartGenerated = false;

        if (m_kernelOccupancyChartGenerated.find(pSessionData->m_sessionId) != m_kernelOccupancyChartGenerated.end())
        {
            std::vector<unsigned int>::iterator callIndexIter;
            callIndexIter = std::find(m_kernelOccupancyChartGenerated[pSessionData->m_sessionId].begin(), m_kernelOccupancyChartGenerated[pSessionData->m_sessionId].end(), callIndex);
            occChartGenerated = callIndexIter != m_kernelOccupancyChartGenerated[pSessionData->m_sessionId].end();
        }

        if (!occChartGenerated)
        {
            retVal = pOccInfo->GenerateOccupancyChart(outputFile, errorMessage);

            if(retVal)
            {
                m_kernelOccupancyChartGenerated[pSessionData->m_sessionId].push_back(callIndex);
            }
        }
        else
        {
            retVal = true;
        }

        strErrorMessageOut.fromStdString(errorMessage);
        HandleGenOccupancyFinished(retVal ? 0 : -1);
    }

    return retVal;
}

void ProfileManager::SaveSettingOfProject(const QString& strProjFullName, const QString& strPlatformConfig,
                                          const QString& strProjectCommand, bool lastUsedWasOriginal)
{
    try
    {
        foreach (QString str, m_openedProjectList)
        {
            QStringList slist = str.split('\n');

            if (4 != slist.length())
            {
                continue;
            }

            if ((slist[0].trimmed() == strProjFullName.trimmed()) && (slist[1].trimmed() == strPlatformConfig.trimmed()))
            {
                m_openedProjectList.removeOne(str);
                break;
            }
        }

        QString strNewEntry = strProjFullName.trimmed();
        strNewEntry.append(QString("\n"));
        strNewEntry.append(strPlatformConfig.trimmed());
        strNewEntry.append(QString("\n"));
        strNewEntry.append(strProjectCommand.trimmed());
        strNewEntry.append(QString("\n"));
        strNewEntry.append(Util::BoolToQString(lastUsedWasOriginal));

        m_openedProjectList.append(strNewEntry);
    }
    catch (...)
    {
    }
}

bool ProfileManager::GetSettingOfProject(const QString& strProjFullName, const QString& strPlatformConfig,
                                         QString& strProjectCommand, bool& lastUsedWasOriginal)
{
    try
    {
        foreach (QString str, m_openedProjectList)
        {
            QStringList slist = str.split('\n');

            if (4 != slist.length())
            {
                continue;
            }

            if ((slist[0].trimmed() == (strProjFullName.trimmed())) && (slist[1].trimmed() == (strPlatformConfig.trimmed())))
            {
                strProjectCommand = slist[2].trimmed();
                lastUsedWasOriginal = Util::QStringToBool(slist[3].trimmed());
                return true;
            }
        }

        return false;
    }
    catch (...)
    {
        return false;
    }
}

GPUSessionTreeItemData* ProfileManager::GetSessionFromSessionId(ExplorerSessionId sessionId)
{
    GPUSessionTreeItemData* pRetVal = nullptr;

    GT_IF_WITH_ASSERT(m_pSessionExplorer != nullptr)
    {
        SessionTreeNodeData* pNodeData = m_pSessionExplorer->GetSessionTreeNodeData(sessionId);

        if (pNodeData != nullptr)
        {
            pRetVal = qobject_cast<GPUSessionTreeItemData*>(pNodeData);
        }
    }

    return pRetVal;
}

void ProfileManager::LoadCurrentProjectSettings()
{
    if (m_pCurrentProjectSettings == nullptr)
    {
        m_pCurrentProjectSettings = new(std::nothrow) ProjectSettings();

    }

    apProjectSettings projectSettings = afProjectManager::instance().currentProjectSettings();
    gtString projectName = projectSettings.projectName();
    QString projName = acGTStringToQString(projectName);
    m_pCurrentProjectSettings->m_projectName = projName;
    OpenCLTraceOptions::Instance()->SetTraceOptions(m_pCurrentProjectSettings->m_traceOptions);
    CounterSelectionSettingWindow::Instance()->SaveCurrentSettings();
}

void ProfileManager::ProjectOpened()
{
    // First make sure that the current project is closed
    ProjectClosed();

    // Load the project in the application tree
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
        GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
        {
            pApplicationTree->updateTreeRootText();
        }
    }

    // Load current project settings
    LoadCurrentProjectSettings();

    QString strError;
    QList<GPUSessionTreeItemData*> sessionList;
    sessionList = SessionManager::Instance()->LoadProjectProfileSessions(strError);

    if (!strError.trimmed().isEmpty())
    {
        Util::ShowWarningBox(strError);
    }

    for (QList<GPUSessionTreeItemData*>::iterator i = sessionList.begin(); i != sessionList.end(); ++i)
    {
        // Get the current session:
        GPUSessionTreeItemData* pCurrentSession = (*i);

        if (pCurrentSession != nullptr)
        {
            afApplicationTreeItemData* pItemData = pCurrentSession->m_pParentData;

            if (pItemData != nullptr)
            {
                QString fileStr = acGTStringToQString(pItemData->m_filePath.asString());
                QFileInfo fileInfo(fileStr);
                QString fileInfoPath = fileInfo.filePath();

                if (pCurrentSession->m_isImported)
                {
                    QString postfix;
                    QFileInfo exeFileInfo(pCurrentSession->m_exeName);
                    pCurrentSession->m_projectName = exeFileInfo.baseName();

                    if (pCurrentSession->m_projectName == afProjectManager::instance().currentProjectSettings().projectName().asASCIICharArray())
                    {
                        postfix.sprintf(" (%s)", PM_STR_ImportedSessionPostfix);
                    }
                    else
                    {
                        postfix.sprintf(" (%s - %s)", PM_STR_ImportedSessionPostfix, pCurrentSession->m_projectName.toLatin1().data());
                    }

                    // Create a name with import postfix:
                    pCurrentSession->m_displayName += postfix;
                }
            }
        }

        AddSessionToExplorer(pCurrentSession, false, false);
    }
}

void ProfileManager::ProjectClosed()
{
    SessionManager::Instance()->CheckAndDeleteSessionFiles();
}

void ProfileManager::AddImportedSession(const QString& strSessionName, GPUProfileType profileType, const QString& strSessionFile)
{
    QString strError;

    GPUSessionTreeItemData* pSessionData = SessionManager::Instance()->AddSessionFromFile(strSessionName, QString(), profileType, strSessionFile, strError);

    if (pSessionData != nullptr)
    {
        AddSessionToExplorer(pSessionData, true, false);
    }
    else
    {
        Util::ShowErrorBox(strError);
    }
}

void ProfileManager::HandleProfileFinished(int exitCode)
{
    bool profileSuccess = false;
    QString strError;
    GPUSessionTreeItemData* pSessionData = nullptr;

    if (exitCode == 0)
    {
        // Do remaining processing
        QFileInfo fileInfo(m_profileParameters.SessionFile());

        if (fileInfo.exists())
        {
            pSessionData = SessionManager::Instance()->AddSession(m_profileParameters.SessionName(),
                                                                  m_profileParameters.CurrentProfileData()->WorkingDirectory(),
                                                                  m_profileParameters.SessionFile(),
                                                                  m_profileParameters.ProjectName(),
                                                                  m_profileParameters.ProfileTypeValue(),
                                                                  false);

            profileSuccess = true;
        }
        else
        {
            HandleMissingProfileOutput(strError);
            profileSuccess = false;
        }
    }
    else
    {
        pSessionData = nullptr;
        profileSuccess = false;

        if (!m_strRemoteProfilingError.isEmpty())
        {
            strError = m_strRemoteProfilingError;
            m_strRemoteProfilingError.clear();
        }
        else
        {
            strError = "Failed to profile.";// FIXME: add more error message from
        }
    }

    emit ProfilingFinished(profileSuccess, strError, pSessionData);

    if (!profileSuccess)
    {
        if (strError.trimmed().isEmpty())
        {
            Util::ShowErrorBox("Failed to profile");
        }
        else
        {
            Util::ShowErrorBox(strError);
        }

    }
    else if (pSessionData != nullptr)
    {
        AddSessionToExplorer(pSessionData, true, true);
    }
    else
    {
        Util::ShowErrorBox(strError);
    }
}

void ProfileManager::HandleMissingProfileOutput(QString& strError)
{
    QString strProjectType = (m_pCurrentProjectSettings->m_traceOptions.m_apiToTrace == APIToTrace_OPENCL) ? "an OpenCL program" : "an HSA program";
    QString strExtraReason;
    strExtraReason.clear();

    if (m_profileParameters.ProfileTypeValue() == API_TRACE)
    {
        strProjectType = (m_pCurrentProjectSettings->m_traceOptions.m_apiToTrace == APIToTrace_OPENCL) ? "an OpenCL program" : "an HSA program";

        // in timeout mode, the backend does not produce an .atp file if all APIs called were filtered away
        if (m_pCurrentProjectSettings->m_traceOptions.m_mode == TIMEOUT)
        {
            strExtraReason = QString("<li>The application did not call any of the APIs enabled in the \"%1\" option on the \"%2\" project setting page</li>").arg(GP_Str_AppTraceAPIToTrace).arg(Util::ms_APP_TRACE_OPTIONS_PAGE);
        }

        strExtraReason += QString("<li>The application terminated unexpectedly before the profiler wrote out any data. %1</li>").arg(Util::ms_ENABLE_TIMEOUT_OPTION);
    }
    else
    {
        strProjectType += ".</li><li>The active project is an OpenCL program, but it did not enqueue any kernels.";
        strProjectType += ".</li><li>The active project is an OpenCL program, but it did not enqueue any kernels listed in the Profile Specific Kernels section";
    }

    strError = QString("Unable to gather profile data.  This error can occur for one of several reasons:<ul>");
    strError.append(QString("<li>The active project is not %1.</li>").arg(strProjectType));
    strError.append("<li>The active project does not compile or run properly (try running it manually).</li>");

    bool isRemoteSession = afProjectManager::instance().currentProjectSettings().isRemoteTarget();

    if (isRemoteSession)
    {
        strError.append("<li>Either the profile output directory or the target executable do not exist on the remote machine.</li>");
        strError.append("<li>The remote agent does not have write access to the profile output directory on the remote machine.</li>");
    }
    else
    {
        strError.append("<li>You do not have write access to the profile output directory.</li>");
    }

    if (!strExtraReason.isEmpty())
    {
        strError.append(strExtraReason);
    }

    strError.append("</ul>");
}

void ProfileManager::HandleGenSummaryFinished(int exitCode)
{
    QString strError;

    if (exitCode != 0)
    {
        strError = "Failed to generate Summary Pages";
    }

    emit SummaryPagesGenerationFinished(exitCode == 0, strError);
}

void ProfileManager::HandleGenOccupancyFinished(int exitCode)
{
    QString strError;

    if (exitCode != 0)
    {
        strError = "Failed to generate Occupancy page: ";
        strError.append(m_strOutputOccHTMLPage);
    }

    emit OccupancyFileGenerationFinished(exitCode == 0, strError, m_strOutputOccHTMLPage);
}

void ProfileManager::ConsumeErrorMessage(const QString& errorMsg)
{
    m_strRemoteProfilingError = errorMsg;
}

void ProfileManager::HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId)
{
    GT_UNREFERENCED_PARAMETER(isProfileSettingsOK);
    GT_UNREFERENCED_PARAMETER(processId);

    bool isProjectSet = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();

    // Open the project settings in the following cases:
    QString infoMessage;

    // Check what should be the information message for the user:
    infoMessage = isProjectSet ? PM_STR_StartProfilingNoExeIsSet : PM_STR_StartProfilingNoProjectIsLoaded;

    if (acMessageBox::instance().information(AF_STR_InformationA, infoMessage, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
        // Open the project settings dialog:
        afApplicationCommands::instance()->OnProjectSettings(AF_globalSettingsGeneralHeaderUnicode);
    }
}

bool ProfileManager::ExportSpecificKernelsToFile(gtString& kernels)
{
    bool retVal = false;
    // make sure strings ends with ';'
    kernels += AF_STR_Semicolon;
    gtStringTokenizer strTokenizer(kernels, AF_STR_Semicolon);
    gtString currentKernel;
    gtVector<QString> kernelsVector;

    // Iterate over the kernels strings, and push them into vector
    while (strTokenizer.getNextToken(currentKernel))
    {
        if (!currentKernel.trim().isEmpty())
        {
            kernelsVector.push_back(acGTStringToQString(currentKernel.trim()));
        }
    }

    // Go over valid kernels and write to file
    if (kernelsVector.size() > 0)
    {
        SAFE_DELETE(m_pSpecificKernelsFile);
        QString kernelsFileName = m_profileParameters.OutputDirectory() + "SpecificKernelsFile";
        m_pSpecificKernelsFile = new(std::nothrow) QFile(kernelsFileName);

        if (m_pSpecificKernelsFile->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            foreach (QString str, kernelsVector)
            {
                QByteArray byteArr;
                str.append(QString(AF_STR_NewLineA));
                byteArr.append(str);
                m_pSpecificKernelsFile->write(byteArr);
            }

            retVal = m_pSpecificKernelsFile->flush();
            m_pSpecificKernelsFile->close();
        }
    }

    return retVal;
}

bool ProfileManager::IsProfileEnabled()
{
    bool retVal = true;

    retVal = GpuProfilerPlugin::s_loadEnabled;

    return retVal;
}
