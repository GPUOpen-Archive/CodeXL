//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionView.cpp
///
//==================================================================================

//------------------------------ ppSessionView.cpp ------------------------------

#include <AMDTPowerProfiling/src/ppSessionView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppSummaryView.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>

// ---------------------------------------------------------------------------
ppSessionView::ppSessionView(QWidget* pParent, ppSessionController::SessionState state) : SharedSessionWindow(pParent),
    m_pMainLayout(nullptr), m_pTabWidget(nullptr), m_pTimelineView(nullptr), m_pSummaryView(nullptr)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_sessionController.SetState(state);

    if (state == ppSessionController::PP_SESSION_STATE_RUNNING)
    {
        ppAppController::instance().DisplayProcessRunningHTML();
    }
}

// ---------------------------------------------------------------------------
ppSessionView::~ppSessionView()
{

}

// ---------------------------------------------------------------------------
void ppSessionView::SetViewData(const osFilePath& sessionPath)
{
    gtString sessionName;
    gtString dbFullPath;
    UpdateSessionInfo(sessionPath, sessionName, dbFullPath);

    m_sessionController.OpenDB();

    // Create the tab view in a layout and add it.
    m_pMainLayout = new QVBoxLayout(this);

    m_pTabWidget = new acTabWidget;

    m_pTabWidget->setTabsClosable(false);

    m_pMainLayout->addWidget(m_pTabWidget);

    m_pTimelineView = new ppTimeLineView(this, &m_sessionController);
    m_pTabWidget->addTab(m_pTimelineView, acGTStringToQString(PP_STR_TreeNodeTimeline));

    m_pSummaryView = new ppSummaryView(this, &m_sessionController);
    m_pTabWidget->addTab(m_pSummaryView, acGTStringToQString(PP_STR_TreeNodeSummary));

    m_pMainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_pMainLayout);

    // Connect to profile start and stop signals:
    bool rc = QObject::connect(&ppAppController::instance(), SIGNAL(ProfileStopped(const QString&)), this, SLOT(OnProfileStopped(const QString&)));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
void ppSessionView::UpdateSessionInfo(const osFilePath& sessionPath, gtString& sessionName, gtString& dbFullPath)
{
    ppAppController& appController = ppAppController::instance();

    osFilePath sessionDBPath = sessionPath;

    // Set the file path in the controller:
    m_sessionController.SetDBFilePath(sessionDBPath);

    dbFullPath = sessionDBPath.asString();

    // Get the session name from the sessionPath:
    osDirectory sessionDir;
    sessionPath.getFileDirectory(sessionDir);
    QString sessionNameAsQString = appController.GetProjectNameFromSessionDir(sessionDir);
    sessionName = acQStringToGTString(sessionNameAsQString);
}

// ---------------------------------------------------------------------------
void ppSessionView::DisplayTab(int tabIndex)
{
    GT_IF_WITH_ASSERT(tabIndex >= 0 && tabIndex < m_pTabWidget->count())
    {
        m_pTabWidget->setCurrentIndex(tabIndex);
    }
}

void ppSessionView::StartDBConnection()
{
    m_sessionController.OpenDB();
}

void ppSessionView::StopDBConnection()
{
    m_sessionController.CloseDB();
}

void ppSessionView::ActivateSession()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTimelineView != nullptr) && (m_pSummaryView != nullptr))
    {
        // Set the session state:
        m_sessionController.SetState(ppSessionController::PP_SESSION_STATE_RUNNING);

        // Open the database:
        StartDBConnection();

        // Open the view connection to profile events:
        m_pTimelineView->UpdateProfileState();

        // Open the view connection to profile events:
        m_pSummaryView->UpdateSessionInformation();

        ppAppController::instance().DisplayProcessRunningHTML();
    }
}

void ppSessionView::SetSessionFilePath(const osFilePath& sessionPath)
{
    // Set the file path in the controller:
    m_sessionController.SetDBFilePath(sessionPath);
}

void ppSessionView::OnProfileStopped(const QString& sessionName)
{
    GT_UNREFERENCED_PARAMETER(sessionName);

    if (m_sessionController.GetSessionState() == ppSessionController::PP_SESSION_STATE_RUNNING)
    {
        // Change the profile state to complete:
        m_sessionController.SetState(ppSessionController::PP_SESSION_STATE_COMPLETED);

        // Only after the state is updated, make the inner views respond to stop profile:
        GT_IF_WITH_ASSERT(m_pTimelineView != nullptr)
        {
            m_pTimelineView->OnProfileStopped(sessionName);
        }

        GT_IF_WITH_ASSERT(m_pSummaryView != nullptr)
        {
            m_pSummaryView->OnProfileStopped(sessionName);
        }
    }
}

void ppSessionView::OnCloseMdiWidget()
{
    osFilePath filePath = m_sessionController.DBFilePath();
    osDirectory fileDirectory;
    filePath.getFileDirectory(fileDirectory);

    // If the MDI window being closed is the one the contains the session that is currently executing, then stop the session execution.
    if (ppAppController::instance().GetExecutedSessionName() == ppAppController::instance().GetProjectNameFromSessionDir(fileDirectory))
    {
        SharedProfileManager::instance().stopCurrentRun();
    }
}