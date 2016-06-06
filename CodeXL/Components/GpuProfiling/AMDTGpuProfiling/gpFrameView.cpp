//------------------------------ gpFrameView.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpFrameView.h>
#include <AMDTGpuProfiling/gpOverview.h>
#include <AMDTGpuProfiling/gpPerformanceCountersDataView.h>
#include <AMDTGpuProfiling/gpTraceDataModel.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/gpObjectView.h>
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/ProfileManager.h>

gpFrameView::gpFrameView(QWidget* pParent) : gpBaseSessionView(pParent),
    m_pTabWidget(nullptr), m_pOverview(nullptr), m_pTraceView(nullptr), m_pTraceModel(nullptr), m_frameIndex(-1, -1)
{
    m_pTabWidget = new QTabWidget;
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pTabWidget, 1, 0);
    setLayout(pMainLayout);

    // Initialize the overview and add it to the tab widget
    m_pOverview = new gpOverview(m_pTabWidget);
    m_pTabWidget->addTab(m_pOverview, GPU_STR_FrameViewOverview);

    // Don't allow to close "Overview" tab
    QTabBar* pTabBar = m_pTabWidget->findChild<QTabBar*>();

    if (nullptr != pTabBar)
    {
        pTabBar->setTabButton(0, QTabBar::RightSide, 0);
    }

    setFocusProxy(m_pTabWidget);
    m_pTabWidget->setTabsClosable(true);

    // Set white background
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);
}

gpFrameView::~gpFrameView()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);
}

bool gpFrameView::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    bool retVal = true;

    // Call the base class
    gpBaseSessionView::DisplaySession(sessionFilePath, sessionInnerPage, errorMessage);

    // Extract the frame index from the file path
    ExtractFrameIndex();

    switch (sessionInnerPage)
    {
        case AF_TREE_ITEM_GP_FRAME:
        case AF_TREE_ITEM_GP_FRAME_OVERVIEW:
        case AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILES:
        {
            DisplayOverview(sessionFilePath);
        }
        break;

        case AF_TREE_ITEM_GP_FRAME_TIMELINE:
        {
            DisplayTimeline();
        }
        break;

        case AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE:
        {
            DisplayProfile(sessionFilePath);
        }
        break;

#ifdef GP_OBJECT_VIEW_ENABLE

        case AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR:
        {
            DisplayObjectInspector();
        }
        break;
#endif

        default:
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}

void gpFrameView::SetProfileDataModel(gpTraceDataModel* pTraceDataModel)
{
    m_pTraceModel = pTraceDataModel;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pOverview != nullptr)
    {
        m_pOverview->SetProfileDataModel(pTraceDataModel);
    }

    if ((m_pTraceView != nullptr) && (pTraceDataModel != nullptr))
    {
        m_pTraceView->SetProfileDataModel(m_pTraceModel);
    }

#ifdef GP_OBJECT_VIEW_ENABLE

    if ((m_pObjectView != nullptr) && (m_pObjectModel != nullptr))
    {
        m_pObjectView->SetProfileObjectDataModel(m_pObjectModel);
    }

#endif
}

void gpFrameView::DisplayOverview(const osFilePath& sessionFilePath)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTabWidget != nullptr) && (m_pOverview != nullptr))
    {
        m_pTabWidget->setCurrentIndex(0);
        m_pOverview->DisplaySession(sessionFilePath, AF_TREE_ITEM_GP_FRAME_OVERVIEW);
    }
}

void gpFrameView::DisplayTimeline()
{
    GT_IF_WITH_ASSERT((m_pTabWidget != nullptr) && (m_pSessionData != nullptr))
    {
        // Get the timeline item data
        osFilePath traceFilePath = gpTreeHandler::Instance().GetFrameChildFilePath(m_sessionFilePath, m_frameIndex, AF_TREE_ITEM_GP_FRAME_TIMELINE);
        GT_IF_WITH_ASSERT(!traceFilePath.isEmpty())
        {
            // Make sure that the frame trace is written to the trace file
            gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
            GT_IF_WITH_ASSERT(pModeManager != nullptr)
            {
                // We should look for the frame owner session file path (needed to calculate the frame file paths)
                afApplicationTreeItemData* pSessionData = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(m_pSessionData->m_pParentData);
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    osFilePath sessionFilePath = pSessionData->m_filePath;
                    gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
                    gtString sessionName = acQStringToGTString(m_pSessionData->m_displayName);
                    gtASCIIString traceAsText;
                    bool rc = pModeManager->GetFrameTraceFromServer(sessionFilePath, m_frameIndex, traceFilePath);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gpUIManager::Instance()->PrepareTraceData(traceFilePath, this, qobject_cast<GPUSessionTreeItemData*>(m_pSessionData));

                        if (m_pTraceView == nullptr)
                        {
                            m_pTraceView = new gpTraceView(m_pTabWidget);
                            m_pTabWidget->addTab(m_pTraceView, GPU_STR_FrameViewTimeline);

                            if (m_pTraceModel != nullptr)
                            {
                                m_pTraceView->SetProfileDataModel(m_pTraceModel);
                            }

                            QString errorMessage;
                            m_pTraceView->DisplaySession(traceFilePath, AF_TREE_ITEM_GP_FRAME_TIMELINE, errorMessage);

                            // If the window failed to open, let the user know what was the problem
                            if (!errorMessage.isEmpty())
                            {
                                acMessageBox::instance().critical(afGlobalVariablesManager::instance().ProductNameA(), errorMessage);
                            }

                        }

                        int traceTabIndex = m_pTabWidget->indexOf(m_pTraceView);
                        m_pTabWidget->setCurrentIndex(traceTabIndex);
                    }
                }
            }
        }
    }
}

/// Display the object inspector view
#ifdef GP_OBJECT_VIEW_ENABLE
void gpFrameView::DisplayObjectInspector()
{
    GT_IF_WITH_ASSERT((m_pTabWidget != nullptr) && (m_pSessionData != nullptr))
    {
        // Get the object item data
        osFilePath objectFilePath = gpTreeHandler::Instance().GetFrameChildFilePath(m_sessionFilePath, m_frameIndex, AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR);
        GT_IF_WITH_ASSERT(!objectFilePath.isEmpty())
        {
            // Make sure that the frame object is written to the object file
            gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
            GT_IF_WITH_ASSERT(pModeManager != nullptr)
            {
                // We should look for the frame owner session file path (needed to calculate the frame file paths)
                afApplicationTreeItemData* pSessionData = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(m_pSessionData->m_pParentData);
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    osFilePath sessionFilePath = pSessionData->m_filePath;
                    gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
                    gtString sessionName = acQStringToGTString(m_pSessionData->m_displayName);
                    gtASCIIString objectAsText;
                    bool rc = pModeManager->GetFrameObject(sessionFilePath, m_frameIndex, objectFilePath);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gpUIManager::Instance()->PrepareObjectData(objectFilePath, this, qobject_cast<GPUSessionTreeItemData*>(m_pSessionData));

                        if (m_pObjectView == nullptr)
                        {
                            m_pObjectView = new gpObjectView(m_pTabWidget);
                            m_pTabWidget->addTab(m_pObjectView, GPU_STR_FrameViewObject);

                            if (m_pObjectModel != nullptr)
                            {
                                m_pObjectView->SetProfileObjectDataModel(m_pObjectModel);
                            }

                            m_pObjectView->DisplaySession(objectFilePath, AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR);
                        }

                        int objectTabIndex = m_pTabWidget->indexOf(m_pObjectView);
                        m_pTabWidget->setCurrentIndex(objectTabIndex);
                    }
                }
            }
        }
    }
}
#endif  // GP_OBJECT_VIEW_ENABLE


void gpFrameView::DisplayProfile(const osFilePath& profileFilePath)
{
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        int tabIndex = -1;

        // Check if this profile tab already exist
        for (int i = 0; i < m_pTabWidget->count(); i++)
        {
            gpPerformanceCountersDataView* pProfileView = qobject_cast<gpPerformanceCountersDataView*>(m_pTabWidget->widget(i));

            if (pProfileView != nullptr)
            {
                if (pProfileView->SessionFilePath() == profileFilePath)
                {
                    tabIndex = i;
                    break;
                }
            }
        }

        if (tabIndex < 0)
        {
            static int profileIndex = 0;
            gpPerformanceCountersDataView* pNewView = new gpPerformanceCountersDataView(m_pTabWidget);
#pragma message ("TODO: FA: get the right profile time and append to the view name")
            QString name = QString(GPU_STR_FrameViewProfile).arg(profileIndex++);
            tabIndex = m_pTabWidget->addTab(pNewView, name);
            pNewView->DisplaySession(profileFilePath, AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE);
        }

        // Select the profile tab
        m_pTabWidget->setCurrentIndex(tabIndex);

    }
}

void gpFrameView::ExtractFrameIndex()
{
    if (m_frameIndex.first < 0)
    {
        // Extract the frame index from the file path
        gtString fileName;
        m_sessionFilePath.getFileName(fileName);
        int pos = fileName.findLastOf(L"_");

        GT_IF_WITH_ASSERT(pos >= 0)
        {
            gtString frameIndexStr;
            fileName.getSubString(pos + 1, fileName.length(), frameIndexStr);
            m_frameIndex = FrameIndexFromString(acGTStringToQString(frameIndexStr));

            GT_ASSERT(m_frameIndex.first >= 0);
        }
    }
}
