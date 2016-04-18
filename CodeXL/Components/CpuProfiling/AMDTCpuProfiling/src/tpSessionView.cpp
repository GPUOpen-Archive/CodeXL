//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpSessionView.cpp
///
//==================================================================================

//------------------------------ tpSessionView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// AMDTCommonHeaders:
#include <AMDTCommonHeaders/AMDTDefinitions.h>

// Shared profile:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>


// Local:
#include <inc/StringConstants.h>
#include <inc/tpAppController.h>
#include <inc/tpSessionView.h>
#include <inc/tpOverview.h>
#include <inc/tpTreeHandler.h>
#include <inc/tpThreadsView.h>


tpSessionView::tpSessionView(QWidget* pParent, const osFilePath& filePath) : QWidget(pParent), afBaseView(&afProgressBarWrapper::instance()),
    m_pMainLayout(nullptr), m_pTabWidget(nullptr), m_pOverview(nullptr), m_pThreadsTimelineWidget(nullptr), m_pSessionData(nullptr)
{
    // Find the application data related to this session:
    tpSessionTreeNodeData* pSessionTreeData = nullptr;


    GT_IF_WITH_ASSERT(!filePath.isEmpty())
    {
        afApplicationTreeItemData* pData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(filePath);
        GT_IF_WITH_ASSERT(pData != nullptr)
        {
            pSessionTreeData = qobject_cast<tpSessionTreeNodeData*>(pData->extendedItemData());
            GT_IF_WITH_ASSERT(pSessionTreeData != nullptr)
            {

                // Show a progress bar + dialog (the following actions are time consuming):
                afProgressBarWrapper::instance().ShowProgressDialog(L"Processing the session data...", 0);

                m_pSessionData = new tpSessionData(filePath, pSessionTreeData);

                // Hide the progress bar:
                afProgressBarWrapper::instance().hideProgressBar();
            }
        }
    }

    // Create the tab view in a layout and add it.
    m_pMainLayout = new QVBoxLayout(this);

    m_pTabWidget = new acTabWidget;

    bool rc = connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));
    GT_ASSERT(rc);

    m_pTabWidget->setTabsClosable(false);

    m_pMainLayout->addWidget(m_pTabWidget);

    // Look for the icon for this tab:
    QPixmap* pOVerviewPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_TP_OVERVIEW);
    QPixmap* pTimelinePixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_TP_TIMELINE);

    QIcon overviewIcon;
    QIcon timelineIcon;

    if ((pOVerviewPixmap != nullptr) && (pTimelinePixmap != nullptr))
    {
        timelineIcon = *pTimelinePixmap;
        overviewIcon = *pOVerviewPixmap;
    }

    // NOTICE: The timeline should be created before the overview, since the total execution, wait time etc'
    // are only extracted when the samples are extracted:
    m_pThreadsTimelineWidget = new tpThreadsView(nullptr, m_pSessionData, pSessionTreeData);

    // After the samples are extracted, the session data can analyze the thread data:
    m_pSessionData->AnalyzeThreadsData();

    m_pOverview = new tpOverview(nullptr, m_pSessionData, pSessionTreeData);

    m_pTabWidget->addTab(m_pOverview, overviewIcon, CP_STR_OverviewTabTitle);
    m_pTabWidget->addTab(m_pThreadsTimelineWidget, timelineIcon, CP_STR_SessionTimelineTitle);

    m_pMainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_pMainLayout);

    m_pSessionData->CloseThreadProfile();
}

tpSessionView::~tpSessionView()
{

}

void tpSessionView::DisplayTab(int tabIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        m_pTabWidget->setCurrentIndex(tabIndex);
    }
}

void tpSessionView::OnCurrentChanged(int index)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pTabWidget->widget(index) != nullptr)
        {
            m_pTabWidget->widget(index)->show();
        }
    }
}
