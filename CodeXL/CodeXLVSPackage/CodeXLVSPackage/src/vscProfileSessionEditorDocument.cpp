//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscProfileSessionEditorDocument.cpp
///
//==================================================================================

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTGpuProfiling:
#include <AMDTGpuProfiling/ProfileManager.h>

// Local:
#include "stdafx.h"
#include <Include/vscProfileSessionEditorDocumentQt.h>
#include <Include/Public/vscProfileSessionEditorDocument.h>

// Power profiling
#include <AMDTPowerProfiling/src/ppMDIViewCreator.h>

ProfileMDIWindowHelper* ProfileMDIWindowHelper::m_spMySingleInstance = nullptr;

void ProfileMDIWindowHelper::initialize()
{
    bool rc = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionAddedToTree()), &ProfileMDIWindowHelper::instance(), SLOT(OnSessionAddedToTree()));
    GT_ASSERT(rc);

    // Handle session rename:
    rc = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(BeforeSessionRename(SessionTreeNodeData*, bool&, QString&)), this, SLOT(OnBeforeSessionRenamed(SessionTreeNodeData*, bool&, QString&)));
    GT_ASSERT(rc);

    rc = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)), this, SLOT(OnAfterSessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)));
    GT_ASSERT(rc);

    // Handle session deletion:
    rc = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionDeleted(ExplorerSessionId, SessionExplorerDeleteType, bool&)), this, SLOT(OnSessionDelete(ExplorerSessionId, SessionExplorerDeleteType, bool&)));
    GT_ASSERT(rc);
}


vscProfileSessionEditorDocument::vscProfileSessionEditorDocument() : vscEditorDocument(), m_pOwner(nullptr)
{
    ProfileMDIWindowHelper::instance().m_documents.push_back(this);
}

void vscProfileSessionEditorDocument::SetEditorCaption(const wchar_t* filePathStr, wchar_t*& pOutBuffer)
{
    m_filePath.setFullPathFromString(filePathStr);
    GT_IF_WITH_ASSERT(!m_filePath.isEmpty())
    {
        gtString sessionName;
        gtString fileExtension;
        GT_ASSERT(m_filePath.getFileExtension(fileExtension));

        // CPU Profile sessions:
        if (fileExtension == AF_STR_CpuProfileFileExtension)
        {
            SessionViewCreator* pCreator = AmdtCpuProfiling::sessionViewCreator();

            if (nullptr != pCreator)
            {
                gtString commandName;
                pCreator->titleString(pCreator->viewIndexByPath(m_filePath), sessionName, commandName);
            }
            else
            {
                m_filePath.getFileNameAndExtension(sessionName);
            }
        }

        // GPU Profile sessions:
        else if ((fileExtension == AF_STR_GpuProfileTraceFileExtension) || (fileExtension == AF_STR_GpuProfileSessionFileExtension))
        {
            gpViewsCreator* pGpuProfilerMDIViewsCreator = gpViewsCreator::Instance();

            if (pGpuProfilerMDIViewsCreator != nullptr)
            {
                sessionName = pGpuProfilerMDIViewsCreator->GetViewTitleForFile(m_filePath);
            }
            else
            {
                m_filePath.getFileNameAndExtension(sessionName);
            }
        }

        // Power Profile sessions:
        else if (fileExtension == AF_STR_PowerProfileSessionFileExtension)
        {
            osDirectory sessionDir;
            bool rc = m_filePath.getFileDirectory(sessionDir);
            GT_IF_WITH_ASSERT(rc)
            {
                sessionName = acQStringToGTString(ppAppController::instance().GetProjectNameFromSessionDir(sessionDir));
            }
        }

        // Allocate a new string and copy the contents.
        std::wstring tmp(sessionName.asCharArray());
        size_t sz = tmp.size() + 1;
        GT_IF_WITH_ASSERT(sz > 0)
        {
            pOutBuffer = new wchar_t[sz];
            std::copy(tmp.begin(), tmp.end(), pOutBuffer);
            pOutBuffer[sz - 1] = L'\0';
        }
    }

}

QWidget* vscProfileSessionEditorDocument::CreateView()
{
    QWidget* pRetVal = nullptr;
    return pRetVal;
}

void vscProfileSessionEditorDocument::LoadDocData(const wchar_t* filePathStr)
{
    // Set the file path:
    m_filePath.setFullPathFromString(filePathStr);

    // Print the loaded document file path to log:
    gtString logMessage;
    logMessage.appendFormattedString(L"Trying to load VS project document: %ls", m_filePath.asString().asCharArray());
    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);

    // For performance counters files, get the temporary file created for VS (gpsession file):
    QString sessionFilePathStr;
    osFilePath sessionFilePath;
    gpViewsCreator::GetSessionFileFromTempPCFile(m_filePath, sessionFilePathStr);
    sessionFilePath = acQStringToGTString(sessionFilePathStr);

    // Do not try to load this session if it was not yet added to the CodeXL explorer
    SessionTreeNodeData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(m_filePath);

    if (pSessionItemData == nullptr)
    {
        pSessionItemData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(sessionFilePath);
    }

    if (pSessionItemData != nullptr)
    {
        // Get the requested profile session file extension:
        gtString fileExtension;
        m_filePath.getFileExtension(fileExtension);

        // If the profile session view was not created yet:
        if (m_pInnerView == nullptr)
        {
            // Load the session according to the session type:
            if (fileExtension == AF_STR_CpuProfileFileExtension)
            {
                // CPU Profile session:
                // Create the sub window from the view file:
                SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
                GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
                {
                    m_pInnerView = pSessionViewCreator->openMdiWidget(nullptr, m_filePath.asString());
                }
            }
            else if ((fileExtension == AF_STR_GpuProfileTraceFileExtension) || (fileExtension == AF_STR_GpuProfileSessionFileExtension))
            {
                // GPU Profile session:
                gpViewsCreator* pGpuProfilerMDIViewsCreator = gpViewsCreator::Instance();
                GT_IF_WITH_ASSERT(pGpuProfilerMDIViewsCreator != nullptr)
                {
                    m_pInnerView = pGpuProfilerMDIViewsCreator->CreateMDIWidget(nullptr, m_filePath);
                }
            }
            else if (fileExtension == AF_STR_PowerProfileSessionFileExtension)
            {
                // Power Profile session:

                // Create the sub window from the view file:
                ppSessionView* pSessionView = new ppSessionView(nullptr, ppSessionController::PP_SESSION_STATE_NEW);

                // Set the new created window:
                m_pInnerView = pSessionView;

                // Add the view to the created view
                ppMDIViewCreator* pCreator = ppMDIViewCreator::Instance();
                gtString filePathAsString = m_filePath.asString();

                pCreator->GetCreatedViews()[filePathAsString] = pSessionView;

                // Load the session:
                const apEvent* pEvent = ppMDIViewCreator::Instance()->CreationEvent();
                const apMDIViewCreateEvent* pCreationEvent = dynamic_cast<const apMDIViewCreateEvent*>(pEvent);
                int viewIndex = AF_TREE_ITEM_PP_TIMELINE;

                if (pCreationEvent != nullptr)
                {
                    if (pCreationEvent->viewIndex() == AF_TREE_ITEM_PP_SUMMARY)
                    {
                        viewIndex = AF_TREE_ITEM_PP_SUMMARY;
                    }
                }

                ProfileMDIWindowHelper::instance().LoadPowerProfileSessionContent(m_pInnerView, m_filePath, viewIndex);
            }
            else if (fileExtension == AF_STR_frameAnalysisOverviewFileExtension || fileExtension == AF_STR_FrameAnalysisTraceFileExtension || fileExtension == AF_STR_frameAnalysisDashboardFileExtension || fileExtension == AF_STR_FrameAnalysisPerfCountersFileExtension)
            {
                gpViewsCreator* pGpuProfilerMDIViewsCreator = gpViewsCreator::Instance();
                GT_IF_WITH_ASSERT(pGpuProfilerMDIViewsCreator != nullptr)
                {
                    m_pInnerView = pGpuProfilerMDIViewsCreator->CreateMDIWidget(nullptr, m_filePath);
                }
            }

            // Add the inner view to the background widget layout
            // Get the created wrapper from vspQTWindowPaneImpl:
            if (m_pImpl != nullptr)
            {
                QWidget* pVSBackgroundPane = m_pImpl->widget();

                // Sanity check:
                GT_IF_WITH_ASSERT(pVSBackgroundPane != nullptr)
                {
                    QHBoxLayout* pLayout = new QHBoxLayout;
                    pLayout->addWidget(m_pInnerView);
                    pVSBackgroundPane->setLayout(pLayout);
                }
            }
            else
            {
                delete m_pInnerView;
                m_pInnerView = nullptr;
            }

        }
    }
}

void vscProfileSessionEditorDocument::ClosePane()
{
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ppSessionView* pPowerProfileSessionView = qobject_cast<ppSessionView*>(m_pInnerView);

        if (nullptr != pPowerProfileSessionView)
        {
            pPowerProfileSessionView->OnCloseMdiWidget();
        }

        GT_IF_WITH_ASSERT(m_pOwner != nullptr)
        {
            // Get the requested profile session file extension:
            gtString fileExtension;
            m_filePath.getFileExtension(fileExtension);

            // Close and finalize the session window according to its file extension:
            if (fileExtension == AF_STR_CpuProfileFileExtension)
            {
                // If this is a CPU session window, remove it's window from the creator's list:
                SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
                GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
                {
                    pSessionViewCreator->removeSessionWindow(m_filePath, true);
                }
            }
            else if ((fileExtension == AF_STR_GpuProfileTraceFileExtension) || (fileExtension == AF_STR_GpuProfileSessionFileExtension))
            {
                delete m_pInnerView;
                m_pInnerView = nullptr;
            }
            else if (fileExtension == AF_STR_PowerProfileSessionFileExtension)
            {
                // remove the view from the map first
                ppMDIViewCreator* pCreator = ppMDIViewCreator::Instance();
                gtString filePathAsString = m_filePath.asString();

                pCreator->GetCreatedViews().erase(filePathAsString);
                delete m_pInnerView;
                m_pInnerView = nullptr;
            }
        }
    }

    // Print the loaded document file path to log:
    gtString logMessage;
    logMessage.appendFormattedString(L"Closing VS project document: %ls", m_filePath.asString().asCharArray());
    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);

    // Set the session view pointer to nullptr:
    m_pInnerView = nullptr;

    // Remove the document pointer from the vector:
    int documentIndex = -1;

    for (int i = 0; i < (int)ProfileMDIWindowHelper::instance().m_documents.size(); i++)
    {
        if (this == ProfileMDIWindowHelper::instance().m_documents[i])
        {
            documentIndex = i;
            break;
        }
    }

    if ((documentIndex >= 0) && (documentIndex < (int)ProfileMDIWindowHelper::instance().m_documents.size()))
    {
        ProfileMDIWindowHelper::instance().m_documents.removeItem(documentIndex);
    }

}

void vscProfileSessionEditorDocument::LoadSession()
{
    // since there is a chance of deleting the document here (during the load we can cancel) we need to do a retain at the begining and release at the end
    Retain();

    // Print the loaded document file path to log:
    gtString logMessage;
    logMessage.appendFormattedString(L"Trying to load VS project document: %ls", m_filePath.asString().asCharArray());
    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);

    // For performance counters files, get the temporary file created for VS (gpsession file):
    QString sessionFilePathStr;
    osFilePath sessionFilePath;
    gpViewsCreator::GetSessionFileFromTempPCFile(m_filePath, sessionFilePathStr);
    sessionFilePath = acQStringToGTString(sessionFilePathStr);

    // Do not try to load this session if it was not yet added to the CodeXL explorer
    SessionTreeNodeData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(sessionFilePath);

    if (pSessionItemData == nullptr)
    {
        pSessionItemData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(m_filePath);
    }

    if (pSessionItemData != nullptr)
    {
        // Get the requested profile session file extension:
        gtString fileExtension;
        GT_ASSERT(m_filePath.getFileExtension(fileExtension));

        // If the profile session view was not created yet:
        if (m_pInnerView == nullptr)
        {
            // Get the created wrapper from vspQTWindowPaneImpl:
            QWidget* pVSBackgroundWidget = m_pImpl->createdQTWidget();

            // Load the session according to the session type:
            if (fileExtension == AF_STR_CpuProfileFileExtension)
            {
                // CPU Profile session:
                // Create the sub window from the view file:
                SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
                GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
                {
                    m_pInnerView = pSessionViewCreator->openMdiWidget(pVSBackgroundWidget, m_filePath.asString());
                }
            }
            else if ((fileExtension == AF_STR_GpuProfileTraceFileExtension) || (fileExtension == AF_STR_GpuProfileSessionFileExtension))
            {
                // GPU Profile session:
                gpViewsCreator* pGpuProfilerMDIViewsCreator = gpViewsCreator::Instance();
                GT_IF_WITH_ASSERT(pGpuProfilerMDIViewsCreator != nullptr)
                {
                    m_pInnerView = pGpuProfilerMDIViewsCreator->CreateMDIWidget(pVSBackgroundWidget, m_filePath);
                }
            }
            else if (fileExtension == AF_STR_PowerProfileSessionFileExtension)
            {
                // Power Profile session:

                // Create the sub window from the view file:
                ppSessionView* pSessionView = new ppSessionView(pVSBackgroundWidget, ppSessionController::PP_SESSION_STATE_NEW);

                // Set the new created window:
                m_pInnerView = pSessionView;

                // Add the view to the created view
                ppMDIViewCreator* pCreator = ppMDIViewCreator::Instance();
                gtString filePathAsString = m_filePath.asString();

                pCreator->GetCreatedViews()[filePathAsString] = pSessionView;

                // Load the session:
                ProfileMDIWindowHelper::instance().LoadPowerProfileSessionContent(m_pInnerView, m_filePath);
            }
            else if (fileExtension == AF_STR_frameAnalysisOverviewFileExtension || fileExtension == AF_STR_FrameAnalysisTraceFileExtension || fileExtension == AF_STR_frameAnalysisDashboardFileExtension || fileExtension == AF_STR_FrameAnalysisPerfCountersFileExtension)
            {
                gpViewsCreator* pGpuProfilerMDIViewsCreator = gpViewsCreator::Instance();
                GT_IF_WITH_ASSERT(pGpuProfilerMDIViewsCreator != nullptr)
                {
                    m_pInnerView = pGpuProfilerMDIViewsCreator->CreateMDIWidget(nullptr, m_filePath);
                }
            }

            if (m_pInnerView != nullptr && m_pImpl != nullptr)
            {
                // Add the inner view to the background widget layout
                // Get the created wrapper from vspQTWindowPaneImpl:
                QWidget* pVSBackgroundPane = m_pImpl->widget();

                // Sanity check:
                GT_IF_WITH_ASSERT(pVSBackgroundPane != nullptr)
                {
                    QHBoxLayout* pLayout = new QHBoxLayout;
                    pLayout->addWidget(m_pInnerView);
                    pVSBackgroundPane->setLayout(pLayout);
                }
            }
            else if (m_pInnerView != nullptr)
            {
                // the view was created but we can't attach it (probably because the user pressed cancel) then it needs to be deleted
                delete m_pInnerView;
                m_pInnerView = nullptr;
            }
        }
    }

    Release();
}

bool vscProfileSessionEditorDocument::GetEditorCaption(wchar_t*& pOutBuffer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(!m_filePath.isEmpty())
    {
        gtString sessionName;
        gtString fileExtension;
        GT_ASSERT(m_filePath.getFileExtension(fileExtension));

        // CPU Profile sessions:
        if (fileExtension == AF_STR_CpuProfileFileExtension)
        {
            SessionViewCreator* pCreator = AmdtCpuProfiling::sessionViewCreator();

            if (nullptr != pCreator)
            {
                gtString commandName;
                pCreator->titleString(pCreator->viewIndexByPath(m_filePath), sessionName, commandName);
                retVal = true;
            }
            else
            {
                m_filePath.getFileNameAndExtension(sessionName);
            }
        }

        // GPU Profile sessions:
        else if ((fileExtension == AF_STR_GpuProfileTraceFileExtension) || (fileExtension == AF_STR_GpuProfileSessionFileExtension))
        {
            gpViewsCreator* pGpuProfilerMDIViewsCreator = gpViewsCreator::Instance();

            if (pGpuProfilerMDIViewsCreator != nullptr)
            {
                sessionName = pGpuProfilerMDIViewsCreator->GetViewTitleForFile(m_filePath);
                retVal = true;
            }
            else
            {
                m_filePath.getFileNameAndExtension(sessionName);
            }
        }

        // Power Profile sessions:
        else if (fileExtension == AF_STR_PowerProfileSessionFileExtension)
        {
            osDirectory sessionDir;
            bool rc = m_filePath.getFileDirectory(sessionDir);
            GT_IF_WITH_ASSERT(rc)
            {
                sessionName = acQStringToGTString(ppAppController::instance().GetProjectNameFromSessionDir(sessionDir));
                retVal = true;
            }
        }

        // Allocate a new string and copy the contents.
        std::wstring tmp(sessionName.asCharArray());
        size_t sz = tmp.size() + 1;
        GT_IF_WITH_ASSERT(sz > 0)
        {
            pOutBuffer = new wchar_t[sz];
            std::copy(tmp.begin(), tmp.end(), pOutBuffer);
            pOutBuffer[sz - 1] = L'\0';
        }
    }

    return retVal;
}

void vscProfileSessionEditorDocument::OnShow()
{
    // Call the base class implementation:
    vscEditorDocument::OnShow();

    GT_IF_WITH_ASSERT(!m_filePath.isEmpty())
    {
        gtString sessionName;
        gtString fileExtension;
        GT_ASSERT(m_filePath.getFileExtension(fileExtension));

        // Check if this is a CPU profile session:
        if (fileExtension == AF_STR_CpuProfileFileExtension)
        {
            SessionViewCreator* pCreator = AmdtCpuProfiling::sessionViewCreator();

            if (nullptr != pCreator)
            {
                pCreator->UpdateView(m_filePath);
            }
        }
    }
}

// *********************************************************************
// *** SECTION-BEGIN: ProfileMDIWindowHeler ***
// This section was relocated from vspProfileSessionEditorDocument.cpp.
// *********************************************************************
/// -----------------------------------------------------------------------------------------------
/// \class Name: ProfileMDIWindowHelper : public QObject
/// \brief Description:  This class is used to make sure that all profile session documents are loaded.
///                      When the VS project is opened, the session document is opened, and in this stage
///                      the tree is not ready yet, and therefore we are not ready to load the session.
///                      This class is handling the tree ready for sessions event, and make sure that all
//                       open documents are loaded.
/// -----------------------------------------------------------------------------------------------

void ProfileMDIWindowHelper::OnBeforeSessionRenamed(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    GT_UNREFERENCED_PARAMETER(renameDisableMessage);

    GT_IF_WITH_ASSERT((pAboutToRenameSessionData != nullptr) && (pAboutToRenameSessionData->m_pParentData != nullptr))
    {
        // Check if the session window is opened:
        isRenameEnabled = true;

        bool isSessionWindowOpen = false;
        vscProfileSessionEditorDocument* pSessionRelatedDocument = nullptr;

        for (int i = 0; i < (int)m_documents.size(); i++)
        {
            vscProfileSessionEditorDocument* pDoc = (vscProfileSessionEditorDocument*)m_documents[i];

            GT_IF_WITH_ASSERT(pDoc != nullptr)
            {
                if (pDoc->m_filePath == pAboutToRenameSessionData->m_pParentData->m_filePath)
                {
                    isSessionWindowOpen = true;
                    pSessionRelatedDocument = pDoc;
                    break;
                }
            }
        }

        if (isSessionWindowOpen)
        {
            // In case of new sessions, in power profile, do not ask the user. Close the window and re-open it:
            bool isEmptySession = true;

            afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->GetSessionNodeItemData(pAboutToRenameSessionData->m_sessionId);

            if (pSessionItemData != nullptr)
            {
                isEmptySession = (pSessionItemData->m_itemType == AF_TREE_ITEM_PROFILE_EMPTY_SESSION);
            }

            if (!isEmptySession)
            {
                int userAnwer = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), VSP_STR_profileSessionRenameQuestion, QMessageBox::Ok | QMessageBox::Cancel);

                if (userAnwer == QMessageBox::Cancel)
                {
                    isRenameEnabled = false;
                }
                else
                {
                    // If this is a power session, stop the db connection:
                    if ((pSessionRelatedDocument != nullptr) && (pSessionRelatedDocument->m_pInnerView != nullptr))
                    {
                        ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pSessionRelatedDocument->m_pInnerView);
                        GT_IF_WITH_ASSERT(pSessionView != nullptr)
                        {
                            pSessionView->SessionController().CloseDBConnections();
                        }
                    }
                }
            }
        }
    }
}

ProfileMDIWindowHelper::ProfileMDIWindowHelper()
{
}


void* vscProfileSessionEditorDocument_CreateInstance()
{
    return new vscProfileSessionEditorDocument();
}


void vscProfileSessionEditorDocument_SetVSCOwner(void* pVscInstance, IProfileSessionEditorDocVsCoreImplOwner* handler)
{
    vscProfileSessionEditorDocument* pInstance =
        (vscProfileSessionEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->vscSetOwner(handler);
    }
}

void vscProfileSessionEditorDocument_LoadSession(void* pVscInstance)
{
    // Convert to vscProfileSessionEditorDocument:
    vscProfileSessionEditorDocument* pInstance = (vscProfileSessionEditorDocument*)pVscInstance;

    // Sanity check:
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->LoadSession();
    }
}

void ProfileMDIWindowHelper::OnSessionAddedToTree()
{
    for (int i = 0; i < (int)m_documents.size(); i++)
    {
        vscProfileSessionEditorDocument* pDocument = (vscProfileSessionEditorDocument*)m_documents[i];

        if (pDocument != nullptr)
        {
            // If when the profile session document was opened, the explorer was not yet ready, load the session now:
            if (pDocument->m_pInnerView == nullptr)
            {
                gtString logMessage;
                logMessage.appendFormattedString(L"Trying to load VS project document: %ls", pDocument->m_filePath.asString().asCharArray());
                OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);

                // Notify the document's owner.
                GT_IF_WITH_ASSERT(pDocument->m_pOwner != nullptr)
                {
                    pDocument->m_pOwner->ceOnExplorerReadyForSessions();
                }
            }
        }
    }
}

void ProfileMDIWindowHelper::LoadOpenedPowerProfileSessions()
{
    for (int i = 0; i < (int)m_documents.size(); i++)
    {
        vscProfileSessionEditorDocument* pDocument = (vscProfileSessionEditorDocument*)m_documents[i];

        if (pDocument != nullptr)
        {
            // If when the profile session document was opened, the explorer was not yet ready, load the session now:
            if (pDocument->m_pInnerView != nullptr)
            {
                // Check if this is a power profile session, and load it:
                LoadPowerProfileSessionContent(pDocument->m_pInnerView, pDocument->m_filePath);
            }
        }
    }
}
void ProfileMDIWindowHelper::OnAfterSessionRenamed(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir)
{
    GT_UNREFERENCED_PARAMETER(oldSessionDir);

    // Find any valid vscProfileSessionEditorDocument instance.
    vscProfileSessionEditorDocument* pValidInstance = nullptr;

    for (size_t i = 0; i < m_documents.size(); i++)
    {
        if (m_documents[i] != nullptr)
        {
            pValidInstance = (vscProfileSessionEditorDocument*)m_documents[i];
            break;
        }
    }

    // If we didn't manage to find a valid instance, then we're in trouble.
    GT_ASSERT_EX((pValidInstance != nullptr), L"Could not find a valid vscProfileSessionEditorDocument instance");

    if (pValidInstance != nullptr)
    {
        // Create a copy of the path string and return it to the VS code space.
        // VS code will be responsible to release that memory using VSC utility functions.
        gtString tmp(oldSessionFilePath.asString());

        // Get the required buffer size.
        const size_t strSize = (tmp.length() + 1);

        // Allocate the buffer.
        wchar_t* retStr = new wchar_t[strSize];

        // Get a handle to the actual underlying buffer.
        const wchar_t* tmpBuffer = tmp.asCharArray();

        // Copy the string.
        std::copy(tmpBuffer, tmpBuffer + strSize, retStr);
        retStr[strSize - 1] = L'\0';

        // Notify the VSP.
        pValidInstance->m_pOwner->ceOnAfterSessionRenamed(retStr);
    }

    // Remove the session window handler from the session view creator:
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT((pSessionViewCreator != nullptr) && (afApplicationCommands::instance() != nullptr))
    {

        pSessionViewCreator->removeSessionWindow(oldSessionFilePath);

        if ((pRenamedSessionData != nullptr) && (pRenamedSessionData->m_pParentData != nullptr))
        {
            // First close the old file:
            osFilePath fileToClose = oldSessionFilePath;
            gtString fileExtenstion;
            fileToClose.getFileExtension(fileExtenstion);

            // If the profile file is a csv, then we have to "translate" to gpsession file:
            if (fileExtenstion == AF_STR_profileFileExtension4)
            {
                // Get the old session (before rename) gpsession file path, and close it:
                gtString oldFileName;
                oldSessionFilePath.getFileName(oldFileName);

                // For performance counters files, get the temporary file created for VS (gpsession file):
                gpViewsCreator::GetTempPCFile(pRenamedSessionData->m_projectName, acGTStringToQString(oldFileName), fileToClose);

                // Get the new session (after rename) gpsession file path, and close it:
                osFilePath tempPcFile;
                gtString newFileName;
                pRenamedSessionData->m_pParentData->m_filePath.getFileName(newFileName);

                // For performance counters files, get the temporary file created for VS (gpsession file):
                gpViewsCreator::GetTempPCFile(pRenamedSessionData->m_projectName, acGTStringToQString(newFileName), tempPcFile);

                // Write the actual session file name to the temp file
                osFile objectfile;
                bool rc = objectfile.open(tempPcFile, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                GT_IF_WITH_ASSERT(rc)
                {
                    rc = objectfile.writeString(pRenamedSessionData->m_pParentData->m_filePath.asString());
                    GT_IF_WITH_ASSERT(rc)
                    {
                        objectfile.close();
                    }
                }

                pRenamedSessionData->m_pParentData->m_filePath = tempPcFile;
            }

            afApplicationCommands::instance()->closeFile(fileToClose);

            // Now open the renamed session:
            afApplicationTreeItemData* pItemToActivate = pRenamedSessionData->m_pParentData;

            if (pItemToActivate->m_itemType == AF_TREE_ITEM_PP_SUMMARY)
            {
                if (pItemToActivate->m_pTreeWidgetItem != nullptr)
                {
                    QTreeWidgetItem* pParent = pItemToActivate->m_pTreeWidgetItem->parent();
                    pItemToActivate = afApplicationCommands::instance()->applicationTree()->getTreeItemData(pParent);
                }
            }

            afApplicationCommands::instance()->applicationTree()->selectItem(pItemToActivate, true);

        }
    }
}

void ProfileMDIWindowHelper::OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete)
{
    GT_UNREFERENCED_PARAMETER(deleteType);

    // Find the item data related to this session:
    SessionTreeNodeData* pSessionData = ProfileApplicationTreeHandler::instance()->GetSessionTreeNodeData(deletedSessionId);
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        canDelete = true;
        // Look for an opened session with the same path:
        vscProfileSessionEditorDocument* pDeletedSessionDocument = nullptr;

        for (size_t i = 0; i < m_documents.size(); i++)
        {
            if (m_documents[i] != nullptr)
            {
                vscProfileSessionEditorDocument* pCurrentDoc = (vscProfileSessionEditorDocument*)m_documents[i];

                if (pCurrentDoc != nullptr)
                {
                    if (pCurrentDoc->m_filePath == pSessionData->m_pParentData->m_filePath)
                    {
                        pDeletedSessionDocument = pCurrentDoc;
                        break;
                    }
                }
            }
        }

        if (pDeletedSessionDocument != nullptr)
        {
            // If this is a power profile ,close the db connection:
            ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pDeletedSessionDocument->m_pInnerView);

            if (pSessionView != nullptr)
            {
                if (pSessionView->SessionController().DBFilePath() == pSessionData->m_pParentData->m_filePath)
                {
                    // Close all database connections to enable the file deletion from file system:
                    pSessionView->SessionController().CloseDBConnections();
                }
            }

            // Close the VS MDI file:
            bool rc = afApplicationCommands::instance()->closeFile(pSessionData->m_pParentData->m_filePath);
            GT_ASSERT(rc);
        }
    }
}
void ProfileMDIWindowHelper::LoadPowerProfileSessionContent(QWidget* pSessionWidget, const osFilePath& sessionFilePath, int viewIndex)
{
    // The session view was already created, make sure that it is loaded (the load cannot be executed before the tree is ready):
    ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pSessionWidget);

    if (pSessionView != nullptr)
    {
        // Find the state of the requested session:
        afApplicationTreeItemData* pActiveItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(sessionFilePath);

        if (nullptr != pActiveItemData)
        {
            if (pActiveItemData->m_itemType != AF_TREE_ITEM_PROFILE_EMPTY_SESSION)
            {
                ppSessionController::SessionState sessionState = ppSessionController::PP_SESSION_STATE_NEW;
                SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pActiveItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    sessionState = (pSessionData->m_isSessionRunning) ? ppSessionController::PP_SESSION_STATE_RUNNING : ppSessionController::PP_SESSION_STATE_COMPLETED;

                    pSessionView->SessionController().SetState(sessionState);

                    // Initialize the view:
                    pSessionView->SetViewData(sessionFilePath);
                }
            }
            else
            {
                pSessionView->SessionController().SetState(ppSessionController::PP_SESSION_STATE_NEW);

                // Initialize the view:
                pSessionView->SetViewData(sessionFilePath);
            }
        }

        // by default the timeline is displayed but if summery view
        if (AF_TREE_ITEM_PP_SUMMARY == viewIndex)
        {
            pSessionView->DisplayTab(PP_TAB_SUMMARY_INDEX);
        }
    }
}
