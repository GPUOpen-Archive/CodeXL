
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFindParameters.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acRibbonManager.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/acColours.h>


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpNavigationRibbon.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/gpTimeline.h>
#include <AMDTGpuProfiling/gpTraceTable.h>
#include <AMDTGpuProfiling/gpTraceTree.h>
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpTraceDataModel.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/SessionViewTabWidget.h>
#include <AMDTGpuProfiling/gpTraceSummaryWidget.h>
#include <AMDTGpuProfiling/gpDetailedDataRibbon.h>
#include <AMDTGpuProfiling/gpRibbonDataCalculator.h>
#include <AMDTGpuProfiling/APITimelineItems.h>


static const unsigned int s_UI_REFRESH_RATE = 1000;
static const unsigned int s_MAX_TRACE_ENTRIES = 200000;

#define MAX_TIME_LINE_SHOWN_LINES 7
#define TIME_LINE_SCROLL_HEIGHT 20
#define TABLE_HEIGHT -300

APICallId::APICallId() : m_tid(0), m_queueName(""), m_callIndex(-1)
{

}

bool APICallId::operator<(const APICallId& other) const
{

    bool retVal = false;

    if ((int)m_tid < (int)other.m_tid)
    {
        retVal = true;
    }
    else if ((int)m_tid == (int)other.m_tid)
    {
        if ((int)m_callIndex < (int)other.m_callIndex)
        {
            retVal = true;
        }
    }

    return retVal;
}


gpTraceView::gpTraceView(QWidget* parent) : gpBaseSessionView(parent),
    m_pRibbonManager(nullptr),
    m_pNavigationRibbon(nullptr),
    m_pDetailedDataRibbon(nullptr),
    m_pFrameDataCalculator(nullptr),
    m_pTimeline(nullptr),
    m_pCPUTraceTablesTabWidget(nullptr),
    m_pGPUTraceTablesTabWidget(nullptr),
    m_pSummaryTableTabWidget(nullptr),
    m_pSessionDataContainer(nullptr),
    m_frameIndex(-1),
    m_isSessionLoaded(false),
    m_isCpuAPISelectionChangeInProgress(false),
    m_isGpuAPISelectionChangeInProgress(false)
{
    // Set the background color to white
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);

}

gpTraceView::~gpTraceView()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);

    delete m_pFrameDataCalculator;
}

bool gpTraceView::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    bool retVal = true;

    if (!m_isSessionLoaded)
    {
        m_isSessionLoaded = true;
        afApplicationCommands::instance()->StartPerformancePrintout("Parsing Trace File");

        // Call the base class implementation
        retVal = SharedSessionWindow::DisplaySession(sessionFilePath, sessionInnerPage, errorMessage);

        GT_IF_WITH_ASSERT(retVal)
        {
            // Make sure that the file exists, and contain data from server, and parse the trace file
            gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
            GT_ASSERT(pModeManager != nullptr);
            retVal = pModeManager->PrepareTraceFile(m_sessionFilePath, m_frameIndex, m_pSessionData, this);


            GT_IF_WITH_ASSERT(retVal && (m_pSessionDataContainer != nullptr) && (m_pCPUTraceTablesTabWidget != nullptr) && (m_pTimeline != nullptr))
            {
                afApplicationCommands::instance()->EndPerformancePrintout("Parsing Trace File");

                afApplicationCommands::instance()->StartPerformancePrintout("Adding CPU items");

                int threadsCount = m_pSessionDataContainer->ThreadsCount();

                for (int i = 0; i < threadsCount; i++)
                {
                    osThreadId threadID = m_pSessionDataContainer->ThreadID(i);

                    // Check if there are performance markers, and create a tree / table accordingly
                    bool hasPerfMarkers = m_pSessionDataContainer->DoesThreadContainPerformanceMarkers(threadID);

                    QAbstractItemView* pContainer = nullptr;

                    if (!hasPerfMarkers)
                    {
                        pContainer = new gpTraceTable(threadID, m_pSessionDataContainer, this);
                        ((gpTraceTable*)pContainer)->SetSelectionBackgroundColor(acQAMD_CYAN_SELECTION_BKG_COLOUR);
                    }
                    else
                    {
                        pContainer = new gpTraceTree(threadID, m_pSessionDataContainer, this);
                    }

                    // Connect the table click signal to a handler
                    bool rc = connect(pContainer, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnTableItemClick(const QModelIndex&)));
                    GT_ASSERT(rc);

                    // Connect the table click signal to a handler
                    rc = connect(pContainer, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnTableItemDoubleClick(const QModelIndex&)));
                    GT_ASSERT(rc);


                    QItemSelectionModel* pSelectionModel = pContainer->selectionModel();
                    GT_ASSERT(pSelectionModel != nullptr);
                    rc = connect(pSelectionModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnCPUTableRowChanged(const QModelIndex&, const QModelIndex&)));
                    GT_ASSERT(rc);


                    QString tableCaption = QString(GPU_STR_timeline_CPU_ThreadBranchName).arg(threadID);
                    m_pCPUTraceTablesTabWidget->addTab(pContainer, QIcon(), tableCaption);

                    m_visibilityFiltersMap.push_back(QPair<QString, QAbstractItemView*>(tableCaption, pContainer));
                }

                afApplicationCommands::instance()->EndPerformancePrintout("Adding CPU items");

                afApplicationCommands::instance()->StartPerformancePrintout("Adding GPU items");

                // Add the GPU items
                int queuesCount = m_pSessionDataContainer->GPUCallsContainersCount();

                if (queuesCount > 0)
                {
                    GT_IF_WITH_ASSERT(m_pGPUTraceTablesTabWidget != nullptr)
                    {
                        for (int i = 0; i < queuesCount; i++)
                        {
                            QString queueName = m_pSessionDataContainer->GPUObjectName(i);

                            // Find the GPU item type according to the container API type
                            gpTraceTable* pTable = new gpTraceTable(queueName, m_pSessionDataContainer, this);
                            pTable->SetSelectionBackgroundColor(acQAMD_CYAN_SELECTION_BKG_COLOUR);

                            // Connect the table click signal to a handler
                            bool rc = connect(pTable, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnTableItemClick(const QModelIndex&)));
                            GT_ASSERT(rc);

                            // Connect the table click signal to a handler
                            rc = connect(pTable, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnTableItemDoubleClick(const QModelIndex&)));
                            GT_ASSERT(rc);



                            QItemSelectionModel* pSelectionModel = pTable->selectionModel();
                            GT_ASSERT(pSelectionModel != nullptr);
                            rc = connect(pSelectionModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnGPUTableRowChanged(const QModelIndex&, const QModelIndex&)));
                            GT_ASSERT(rc);


                            // Get the queue display name for the table caption, and add the tab
                            QString queueDisplayName = m_pSessionDataContainer->QueueDisplayName(queueName);
                            m_pGPUTraceTablesTabWidget->addTab(pTable, QIcon(), queueDisplayName);
                        }
                    }
                }

                afApplicationCommands::instance()->EndPerformancePrintout("Adding GPU items");
                afApplicationCommands::instance()->StartPerformancePrintout("Build the timeline");

                // if there is no time line the navigation chart can't really be set even if we have the data
                // it will not be displayed correctly
                GT_IF_WITH_ASSERT(m_pTimeline != nullptr && m_pNavigationRibbon != nullptr && m_pDetailedDataRibbon != nullptr && m_pFrameDataCalculator != nullptr)
                {
                    // Build the timeline with the parsed data
                    m_pTimeline->BuildTimeline(m_pSessionDataContainer);

                    QVector<double> presentData;
                    m_pFrameDataCalculator->GetPresentData(presentData);

                    m_pTimeline->SetPresentData(presentData);

                    afApplicationCommands::instance()->EndPerformancePrintout("Build the timeline");

                    m_pFrameDataCalculator->CalculateData();

                    m_pNavigationRibbon->SetTimeLine(m_pTimeline);
                    // set the initial navigation data item
                    m_pNavigationRibbon->SetPresentData(presentData);
                    m_pNavigationRibbon->CalculateData();

                    // now we have the data and can update the initial height
                    int timeLineHeight = m_pTimeline->cumulativeBranchHeight();
                    timeLineHeight = timeLineHeight > m_pTimeline->defaultBranchHeight() * MAX_TIME_LINE_SHOWN_LINES ? m_pTimeline->defaultBranchHeight() * MAX_TIME_LINE_SHOWN_LINES : timeLineHeight;
                    timeLineHeight += TIME_LINE_SCROLL_HEIGHT;
                    m_pRibbonManager->ChangeRibbonInitialHeight(m_pTimeline, timeLineHeight);

                    m_pDetailedDataRibbon->SetTimeLine(m_pTimeline);

                    m_pDetailedDataRibbon->CalculateData();

                    // After the data was initialized, and layers were created in navigation and detailed views, emit a signal with the visibility
                    m_pNavigationRibbon->EmitLayersVisibility();
                }

                // API Summary
                afApplicationCommands::instance()->StartPerformancePrintout("Summary");

                m_pSummaryTableTabWidget->Init(m_pSessionDataContainer, this, m_pTimeline->visibleStartTime(), m_pTimeline->visibleRange());
                bool rc = connect(m_pSummaryTableTabWidget, SIGNAL(SummaryItemClicked(ProfileSessionDataItem*)), this, SLOT(OnSummaryItemClicked(ProfileSessionDataItem*)));
                GT_ASSERT(rc);
                rc = connect(m_pSummaryTableTabWidget, SIGNAL(SummaryCmdListClicked(const QString& )), m_pTimeline, SLOT(OnSummaryCmdListClicked(const QString&)));
                GT_ASSERT(rc);

                afApplicationCommands::instance()->EndPerformancePrintout("Summary");
            }
        }
    }

    return retVal;
}

void gpTraceView::onFindClick()
{
    QWidget* pFocusedWidget = focusWidget();

    // If the focused widget belongs to the API tables, find the requested text in the data container, sorted by start time,
    // and select the found item in the tables
    if (acIsChildOf(pFocusedWidget, m_pCPUTraceTablesTabWidget) || acIsChildOf(pFocusedWidget, m_pGPUTraceTablesTabWidget))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
        {
            // If the last find result was successful:
            ProfileSessionDataItem* pFindMatchItem = m_pSessionDataContainer->FindNextItem(acFindParameters::Instance().m_findExpr, acFindParameters::Instance().m_isCaseSensitive);

            if (pFindMatchItem != nullptr)
            {
                SelectItemInTraceTables(pFindMatchItem, true);
                acFindParameters::Instance().m_lastResult = true;
            }
            else
            {
                acFindParameters::Instance().m_lastResult = false;
            }
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pSummaryTableTabWidget))
    {
        // Find the requested text in the summary widget
        GT_IF_WITH_ASSERT(m_pSummaryTableTabWidget != nullptr)
        {
            m_pSummaryTableTabWidget->OnFind();
        }
    }
}

void gpTraceView::onFindNext()
{
}

void gpTraceView::OnTableItemClick(const QModelIndex& clickedItem)
{
    ProfileSessionDataItem* pDataItem = nullptr;
    ProfileSessionDataItem::ProfileSessionDataColumnIndex colIndex = ProfileSessionDataItem::SESSION_ITEM_UNKNOWN;

    // Get the table / tree
    gpTraceTable* pTable = qobject_cast<gpTraceTable*>(sender());

    if (pTable != nullptr)
    {
        pDataItem = pTable->GetItem(clickedItem);
        colIndex = pTable->TableIndexToColumnIndex(clickedItem.column());
    }
    else
    {
        gpTraceTree* pTree = qobject_cast<gpTraceTree*>(sender());

        if (pTree != nullptr)
        {
            pDataItem = pTree->GetItem(clickedItem);
            colIndex = pTree->TableIndexToColumnIndex(clickedItem.column());
        }
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(pDataItem != nullptr && m_pSummaryTableTabWidget != nullptr)
    {
        if (colIndex == ProfileSessionDataItem::SESSION_ITEM_OCCUPANCY_COLUMN)
        {
            OccupancyInfo* pOccupancyInfo = pDataItem->GetOccupancyInfo();

            if (pOccupancyInfo != nullptr)
            {
                m_currentDisplayedOccupancyKernel = pOccupancyInfo->GetKernelName();

                // get the api index from the Index column (column 0)
                QString strCallIndex;
                strCallIndex = pDataItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN).toString();

                bool rc = false;
                int callIndex = strCallIndex.toInt(&rc);

                // if we got a valid call index, then show the occupancy view
                if (rc)
                {
                    QString strErrorMessageOut;
                    rc = connect(ProfileManager::Instance(), SIGNAL(OccupancyFileGenerationFinished(bool, const QString&, const QString&)), this, SLOT(OnOccupancyFileGenerationFinish(bool, const QString&, const QString&)));
                    GT_ASSERT(rc);

                    // Generate occupancy page
                    GPUSessionTreeItemData* pGPUData = qobject_cast<GPUSessionTreeItemData*>(m_pSessionData);
                    bool retVal = ProfileManager::Instance()->GenerateOccupancyPage(pGPUData, pOccupancyInfo, callIndex, strErrorMessageOut);

                    if (!retVal)
                    {
                        Util::ShowErrorBox(strErrorMessageOut);
                    }
                }
            }
        }
        else if (colIndex == ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN)
        {
            APICallId id;
            id.m_tid = pDataItem->ThreadID();
            id.m_callIndex = pDataItem->APICallIndex();

            if (m_apiItemsUIMap.contains(id))
            {
                acTimelineItem* pDeviceBlockItem = m_apiItemsUIMap[id].m_pDeviceBlock;

                if (pDeviceBlockItem != nullptr)
                {
                    // at this point, we know that the user has clicked a device block cell
                    m_pTimeline->ZoomToItem(pDeviceBlockItem, true);
                }
            }
        }

        SyncSelectionInAllTables(pDataItem);

        // Select the item on the timeline and make sure that it is centered
        acTimelineItem* pItem = SessionItemToTimelineItem(pDataItem);

        if (pItem != nullptr)
        {
            m_pTimeline->DisplayItemAtHorizontalCenter(pItem, true);
        }
    }
}

void gpTraceView::SyncSelectionInSummary(ProfileSessionDataItem* pDataItem)
{
    GT_IF_WITH_ASSERT(pDataItem != nullptr)
    {
        // sync selection in summary table
        QString call = pDataItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN).toString();

        if (pDataItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
        {
            m_pSummaryTableTabWidget->SelectAPIRowByCallName(call);
        }
        else
        {
            m_pSummaryTableTabWidget->SelectGPURowByCallName(call);
        }
    }
}

void gpTraceView::OnTableItemDoubleClick(const QModelIndex& clickedItem)
{
    ProfileSessionDataItem* pDataItem = nullptr;

    // Get the table / tree
    gpTraceTable* pTable = qobject_cast<gpTraceTable*>(sender());

    if (pTable != nullptr)
    {
        pDataItem = pTable->GetItem(clickedItem);
    }
    else
    {
        gpTraceTree* pTree = qobject_cast<gpTraceTree*>(sender());

        if (pTree != nullptr)
        {
            pDataItem = pTree->GetItem(clickedItem);
        }
    }

    ZoomToItem(pDataItem);
}

void gpTraceView::SetAPICallTimelineItem(APICallId apiId, acAPITimelineItem* pTimelineItem)
{
    m_apiItemsUIMap[apiId].m_pAPITimelineItem = pTimelineItem;
}

void gpTraceView::SetGPUTimelineItem(APICallId apiId, acTimelineItem* pGpuTimelineItem)
{
    m_apiItemsUIMap[apiId].m_pDeviceBlock = pGpuTimelineItem;
}

acAPITimelineItem* gpTraceView::GetAPITimelineItem(APICallId apiId)
{
    acAPITimelineItem* pRetVal = nullptr;

    if (m_apiItemsUIMap.contains(apiId))
    {
        pRetVal = m_apiItemsUIMap[apiId].m_pAPITimelineItem;
    }

    return pRetVal;
}

void gpTraceView::ZoomToItem(ProfileSessionDataItem* pDataItem)
{
    acTimelineItem* pTimelineItem = SessionItemToTimelineItem(pDataItem);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTimeline != nullptr && pTimelineItem != nullptr)
    {
        m_pTimeline->ZoomToItem(pTimelineItem, true);
    }
}

void gpTraceView::SetProfileDataModel(gpTraceDataModel* pDataModel)
{
    // Call the base class
    gpBaseSessionView::SetProfileDataModel(pDataModel);

    // Build the view layout according to the content of the session
    // Sanity check:
    GT_IF_WITH_ASSERT(pDataModel != nullptr)
    {
        m_pSessionDataContainer = pDataModel->TraceDataContainer();

        m_pTimeline = new gpTimeline(nullptr, this);

        // Create the calculator that will be used by the navigation and detailed ribbons
        m_pFrameDataCalculator = new gpRibbonDataCalculator(m_pSessionDataContainer, m_pTimeline);

        // Create the ribbon manager and add its ribbons
        m_pRibbonManager = new acRibbonManager(this);

        m_pNavigationRibbon = new gpNavigationRibbon(this, m_pFrameDataCalculator);
        m_pDetailedDataRibbon = new gpDetailedDataRibbon(this, m_pFrameDataCalculator);

        // hide the time grid
        if (m_pTimeline->grid() != nullptr)
        {
            m_pTimeline->grid()->setVisible(false);
        }

        m_pRibbonManager->AddRibbon((QWidget*)m_pNavigationRibbon, "", acScalePixelSizeToDisplayDPI(NAV_HEIGHT), true, false);
        m_pRibbonManager->AddRibbon(m_pDetailedDataRibbon, GPU_STR_ribbonNameDrawCalls, acScalePixelSizeToDisplayDPI(NAV_HEIGHT) * 0.5, false, true, false);
        m_pRibbonManager->AddRibbon(m_pTimeline, GPU_STR_ribbonNameTimeLine, 0);
        m_pRibbonManager->SetBoundFrameControlRibbons(m_pNavigationRibbon->NavigationChart(), m_pNavigationRibbon, m_pTimeline);
        m_pCPUTraceTablesTabWidget = new acTabWidget(nullptr);

        bool isGPUTrace = false;

        // Sanity check:
        GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
        {
            isGPUTrace = m_pSessionDataContainer->GPUCallsContainersCount() > 0;
        }

        // If there is a GPU trace, add an horizontal splitter with the GPU trace table.
        // if there isn't add only the CPU trace tab to the main vertical splitter
        if (isGPUTrace)
        {
            m_pGPUTraceTablesTabWidget = new acTabWidget(nullptr);

            QSplitter* pHSplitter = new QSplitter(Qt::Horizontal);
            QList<int> sizes;
            sizes << 50;
            sizes << 50;
            pHSplitter->addWidget(m_pCPUTraceTablesTabWidget);
            pHSplitter->addWidget(m_pGPUTraceTablesTabWidget);
            pHSplitter->setSizes(sizes);
            m_pRibbonManager->AddRibbon(pHSplitter, GPU_STR_ribbonNameAPICalls, TABLE_HEIGHT);
        }
        else
        {
            m_pRibbonManager->AddRibbon(m_pCPUTraceTablesTabWidget, GPU_STR_ribbonNameAPICalls, TABLE_HEIGHT);
        }

        // API / GPU Summary
        m_pSummaryTableTabWidget = new gpTraceSummaryWidget(nullptr);
        m_pRibbonManager->AddRibbon(m_pSummaryTableTabWidget, GPU_STR_ribbonNameSummary, TABLE_HEIGHT);

        QHBoxLayout* pMainLayout = new QHBoxLayout;
        pMainLayout->addWidget(m_pRibbonManager);
        pMainLayout->setContentsMargins(0, 0, 0, 0);
        setLayout(pMainLayout);

        // connect the events
        bool rc = connect(m_pNavigationRibbon->NavigationChart(), SIGNAL(RangeChangedByUserEnded(const QPointF&)), m_pSummaryTableTabWidget, SLOT(OnTimelineChanged(const QPointF&)));
        GT_ASSERT(rc);
        rc = connect(m_pNavigationRibbon, SIGNAL(LayerVisibilityChanged()), this, SLOT(OnNavigationLayerVisibilityChanged()));
        GT_ASSERT(rc);
        rc = connect(m_pDetailedDataRibbon, SIGNAL(AfterReplot()), this, SLOT(OnAfterReplot()));
        GT_ASSERT(rc);
        rc = connect(m_pTimeline, SIGNAL(VisibilityFilterChanged(QMap<QString, bool>&)), m_pSummaryTableTabWidget, SLOT(OnTimelineFilterChanged(QMap<QString, bool>&)));
        GT_ASSERT(rc);
        rc = connect(m_pTimeline, SIGNAL(VisibilityFilterChanged(QMap<QString, bool>&)), m_pNavigationRibbon, SLOT(OnTimelineFilterChanged(QMap<QString, bool>&)));
        GT_ASSERT(rc);
        rc = connect(m_pTimeline, SIGNAL(VisibilityFilterChanged(QMap<QString, bool>&)), this, SLOT(OnTimelineFilterChanged(QMap<QString, bool>&)));
        GT_ASSERT(rc);
        rc = connect(m_pTimeline, SIGNAL(zoomFactorChanged()), this, SLOT(OnTimelineRangeChanged()));
        GT_ASSERT(rc);
        rc = connect(m_pTimeline, SIGNAL(offsetChanged()), this, SLOT(OnTimelineRangeChanged()));
        GT_ASSERT(rc);
        rc = connect(m_pTimeline, SIGNAL(itemDoubleClicked(acTimelineItem*)), this, SLOT(OnTimelineItemActivated(acTimelineItem*)));
        GT_ASSERT(rc);
        rc = connect(m_pRibbonManager, SIGNAL(ShowTimeLine(bool, double)), m_pNavigationRibbon->NavigationChart(), SLOT(OnShowTimeLine(bool, double)));
        GT_ASSERT(rc);
        rc = connect(m_pNavigationRibbon->NavigationChart(), SIGNAL(ShowTimeLine(bool, double)), m_pRibbonManager, SLOT(OnShowTimeLine(bool, double)));
        GT_ASSERT(rc);
    }
}
void gpTraceView::OnTimelineRangeChanged()
{
    int startTime = m_pTimeline->visibleStartTime();
    QPointF point = QPointF(startTime, m_pTimeline->visibleRange());
    emit m_pSummaryTableTabWidget->OnTimelineChanged(point, false);
}

void gpTraceView::OnTimelineItemActivated(acTimelineItem* pTimelineItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pTimelineItem != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        // Look for the data base item reflected by this timeline item
        ProfileSessionDataItem* pMatchingItem = nullptr;

        for (auto itemsMapIter = m_apiItemsUIMap.begin(); itemsMapIter != m_apiItemsUIMap.end(); itemsMapIter++)
        {
            if ((*itemsMapIter).m_pAPITimelineItem == pTimelineItem)
            {
                // Get the call id for this timeline item
                APICallId callId = itemsMapIter.key();

                if (!callId.m_queueName.isEmpty())
                {
                    // This is a GPU item, get it from the data container
                    pMatchingItem = m_pSessionDataContainer->QueueItemByItemCallIndex(callId.m_queueName, callId.m_callIndex);
                }
                else
                {
                    // This is a CPU item, get it from the data container
                    pMatchingItem = m_pSessionDataContainer->APIItem(callId.m_tid, callId.m_callIndex - 1);
                }

                break;
            }
        }

        if (pMatchingItem != nullptr)
        {
            // Select the item in the tables
            SelectItemInTraceTables(pMatchingItem, true);
        }
        else // command list
        {
            CommandListTimelineItem* pCmdListItem = dynamic_cast<CommandListTimelineItem*>(pTimelineItem);
            if (pCmdListItem != nullptr)
            {
                m_pSummaryTableTabWidget->SelectCommandList(pCmdListItem->CpommandListPtr());
            }
        }
    }
}

void gpTraceView::OnNavigationLayerVisibilityChanged()
{
    // If concurrency is selected, make sure that the concurrency is calculated
    GT_IF_WITH_ASSERT(m_pNavigationRibbon != nullptr && m_pDetailedDataRibbon != nullptr)
    {
        afApplicationCommands::instance()->StartPerformancePrintout("OnNavigationLayerVisibilityChanged");
        int visibleGroup = 0, visibleLayersByFlag = 0;
        m_pNavigationRibbon->GetCurrentVisibility(visibleGroup, visibleLayersByFlag);

        if (visibleGroup == 2)
        {
            afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingThreadsConcurrency, 3);
            m_pNavigationRibbon->CalculateConcurrency();
        }

        // This function is called again on purpose. CalculateConcurrency changes the visibility flag (it adds
        // few more layers), so we need to set it again
        m_pNavigationRibbon->GetCurrentVisibility(visibleGroup, visibleLayersByFlag);

        m_pNavigationRibbon->OnLayerVisibilityChanged(visibleGroup, visibleLayersByFlag);

        if (visibleGroup == 2)
        {
            afProgressBarWrapper::instance().incrementProgressBar();
        }

        m_pDetailedDataRibbon->OnLayerVisibilityChanged(visibleGroup, visibleLayersByFlag);

        afApplicationCommands::instance()->EndPerformancePrintout("OnNavigationLayerVisibilityChanged");

    }
}

void gpTraceView::OnAfterReplot()
{
    afProgressBarWrapper::instance().hideProgressBar();
}

void gpTraceView::OnSummaryItemClicked(ProfileSessionDataItem* pItem)
{
    SelectItemInTraceTables(pItem, false);
}

void gpTraceView::OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap)
{
    GT_IF_WITH_ASSERT(m_pCPUTraceTablesTabWidget != nullptr)
    {
        int numTabs = m_pCPUTraceTablesTabWidget->count();

        for (int i = numTabs; i >= 0; i--)
        {
            m_pCPUTraceTablesTabWidget->removeTab(i);
        }

        // iterate on tabs vector to maintain the original tab order
        for (auto pair : m_visibilityFiltersMap)
        {
            QStringList tabCaption = pair.first.split(" ");
            GT_IF_WITH_ASSERT(tabCaption.count() == 2)
            {
                QString threadName = tabCaption[1];

                // the keys in threadNameVisibilityMap ("Xxx - DX12") are not the same as the tab captions ("Thread Xxx")
                for (auto key : threadNameVisibilityMap.keys())
                {
                    QString threadNameFromKey = key;

                    if (threadNameVisibilityMap.value(key) == true && threadNameFromKey == pair.first)
                    {
                        QAbstractItemView* pTab = pair.second;

                        if (pTab != nullptr)
                        {
                            m_pCPUTraceTablesTabWidget->addTab(pTab, QIcon(), key);
                        }
                    }
                }
            }
        }
    }
}

void gpTraceView::SelectItemInTraceTables(ProfileSessionDataItem* pItem, bool setFocus)
{
    // Sanity check:
    if (pItem != nullptr)
    {
        // Find the parent table for this item
        gpTraceTable* pTable = nullptr;

        if (pItem->IsCPUItem())
        {
            if (m_pCPUTraceTablesTabWidget != nullptr)
            {
                // Find the tab related to this item's thread ID:
                QString tabName = QString(GPU_STR_timeline_CPU_ThreadBranchName).arg(pItem->ThreadID());

                for (int i = 0; i < m_pCPUTraceTablesTabWidget->count(); i++)
                {
                    if (m_pCPUTraceTablesTabWidget->tabText(i) == tabName)
                    {
                        pTable = qobject_cast<gpTraceTable*>(m_pCPUTraceTablesTabWidget->widget(i));
                        break;
                    }
                }
            }

#pragma message ("TODO: FA: support gpTraceTree in case of perforamnce markers")

        }
        else
        {
            if (m_pGPUTraceTablesTabWidget != nullptr)
            {
                for (int i = 0; i < m_pGPUTraceTablesTabWidget->count(); i++)
                {
                    QString tabText = m_pGPUTraceTablesTabWidget->tabText(i);
                    QString queueDisplayName = m_pSessionDataContainer->QueueNameFromPointer(pItem->QueueName());

                    if (tabText.contains(queueDisplayName))
                    {
                        pTable = qobject_cast<gpTraceTable*>(m_pGPUTraceTablesTabWidget->widget(i));
                        break;
                    }
                }
            }
        }

        // The table should be found. Select the table as current in its owner tab widget
        acTabWidget* pOwnerTabWidget = pItem->IsCPUItem() ? m_pCPUTraceTablesTabWidget : m_pGPUTraceTablesTabWidget;

        if ((pTable != nullptr) && (pOwnerTabWidget != nullptr))
        {
            // Select the tab for this thread
            pOwnerTabWidget->setCurrentWidget(pTable);

            // Select the requested item row
            pTable->blockSignals(true);
            pTable->SelectRowByColValue(0, pItem->APICallIndex());
            pTable->blockSignals(false);

            if (setFocus == true)
            {
                pTable->setFocus();
            }
        }
    }
}

void gpTraceView::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = true;
}

void gpTraceView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = true;

    QWidget* pFocusedWidget = qApp->focusWidget();

    if (acIsChildOf(pFocusedWidget, m_pSummaryTableTabWidget))
    {
        // Summary table only has a single selection option. Therefore, the select all
        // should be disabled when it is focused
        isEnabled = false;
    }
}

void gpTraceView::onUpdateEdit_Find(bool& isEnabled)
{
    isEnabled = true;
}

void gpTraceView::onUpdateEdit_FindNext(bool& isEnabled)
{
    isEnabled = true;
}

void gpTraceView::OnEditCopy()
{
    QWidget* pFocusedWidget = qApp->focusWidget();

    if (acIsChildOf(pFocusedWidget, m_pCPUTraceTablesTabWidget))
    {
        gpTraceTable* pTable = qobject_cast<gpTraceTable*>(m_pCPUTraceTablesTabWidget->currentWidget());

        if (pTable != nullptr)
        {
            pTable->OnEditCopy();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pGPUTraceTablesTabWidget))
    {
        gpTraceTable* pTable = qobject_cast<gpTraceTable*>(m_pGPUTraceTablesTabWidget->currentWidget());

        if (pTable != nullptr)
        {
            pTable->OnEditCopy();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pSummaryTableTabWidget))
    {
        GT_IF_WITH_ASSERT(m_pSummaryTableTabWidget != nullptr)
        {
            m_pSummaryTableTabWidget->OnEditCopy();
        }
    }
}

void gpTraceView::OnEditSelectAll()
{
    QWidget* pFocusedWidget = qApp->focusWidget();

    if (acIsChildOf(pFocusedWidget, m_pCPUTraceTablesTabWidget))
    {
        gpTraceTable* pTable = qobject_cast<gpTraceTable*>(m_pCPUTraceTablesTabWidget->currentWidget());

        if (pTable != nullptr)
        {
            pTable->OnEditSelectAll();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pGPUTraceTablesTabWidget))
    {
        gpTraceTable* pTable = qobject_cast<gpTraceTable*>(m_pGPUTraceTablesTabWidget->currentWidget());

        if (pTable != nullptr)
        {
            pTable->OnEditSelectAll();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pSummaryTableTabWidget))
    {
        GT_IF_WITH_ASSERT(m_pSummaryTableTabWidget != nullptr)
        {
            m_pSummaryTableTabWidget->OnEditSelectAll();
        }
    }
}

void gpTraceView::ClearSelectionInTraceTables(bool isCPU)
{
    // Find the parent table for this item
    gpTraceTable* pTable = nullptr;

    if (isCPU == true && m_pGPUTraceTablesTabWidget != nullptr && m_pSummaryTableTabWidget != nullptr)
    {
        for (int i = 0; i < m_pGPUTraceTablesTabWidget->count(); i++)
        {
            pTable = qobject_cast<gpTraceTable*>(m_pGPUTraceTablesTabWidget->widget(i));
            pTable->clearSelection();
        }

        m_pSummaryTableTabWidget->ClearGPUSelection();
    }
    else if (isCPU == false && m_pCPUTraceTablesTabWidget != nullptr && m_pSummaryTableTabWidget != nullptr)
    {
        for (int i = 0; i < m_pCPUTraceTablesTabWidget->count(); i++)
        {
            pTable = qobject_cast<gpTraceTable*>(m_pCPUTraceTablesTabWidget->widget(i));
            pTable->clearSelection();
        }

        m_pSummaryTableTabWidget->ClearAPISelection();
    }
}
void gpTraceView::OnCPUTableRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    GT_UNREFERENCED_PARAMETER(previous);

    // Selection in the API table causes selection of the matching item in the GPU table, therefore this method is be called in both cases.
    // if the section "started" in the GPU table, we don't to select the matching item in the timeline, because it will cause flickering
    if (!m_isGpuAPISelectionChangeInProgress)
    {
        m_isCpuAPISelectionChangeInProgress = true;
        int selectedTabIndex = m_pCPUTraceTablesTabWidget->currentIndex();
        gpTraceTable* pTable = qobject_cast<gpTraceTable*>(m_pCPUTraceTablesTabWidget->widget(selectedTabIndex));
        GT_IF_WITH_ASSERT(pTable != nullptr)
        {
            ProfileSessionDataItem* pDataItem = pTable->GetItem(current);

            // Sanity check:
            GT_IF_WITH_ASSERT(pDataItem != nullptr)
            {
                // Sync the GPU and CPU tables selections
                SyncSelectionInAllTables(pDataItem);

                // Select the item on the timeline and make sure that it is centered
                acTimelineItem* pItem = SessionItemToTimelineItem(pDataItem);

                if (pItem != nullptr)
                {
                    m_pTimeline->DisplayItemAtHorizontalCenter(pItem, true);
                }

                // Put the focus back on the table (the function that display the item over the timeline steals the focus)
                pTable->setFocus();
            }
        }
        m_isCpuAPISelectionChangeInProgress = false;
    }
}

void gpTraceView::OnGPUTableRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    GT_UNREFERENCED_PARAMETER(previous);

    // If this event is in response to setting selection in the CPU table then there is no need to
    // cross-sync it with the CPU table
    if (!m_isCpuAPISelectionChangeInProgress)
    {
        m_isGpuAPISelectionChangeInProgress = true;
        int selectedTabIndex = m_pGPUTraceTablesTabWidget->currentIndex();
        gpTraceTable* pTable = qobject_cast<gpTraceTable*>(m_pGPUTraceTablesTabWidget->widget(selectedTabIndex));
        GT_IF_WITH_ASSERT(pTable != nullptr)
        {
            ProfileSessionDataItem* pDataItem = pTable->GetItem(current);

            // Sanity check:
            GT_IF_WITH_ASSERT(pDataItem != nullptr)
            {
                // Sync the GPU and CPU tables selections
                SyncSelectionInAllTables(pDataItem);

                // Select the item on the timeline and make sure that it is centered

                acTimelineItem* pItem = SessionItemToTimelineItem(pDataItem);

                if (pItem != nullptr)
                {
                    m_pTimeline->DisplayItemAtHorizontalCenter(pItem, true);
                }

                // Put the focus back on the table (the function that display the item over the timeline steals the focus)
                pTable->setFocus();
            }
        }
        m_isGpuAPISelectionChangeInProgress = false;
    }
}

void gpTraceView::SyncSelectionInAllTables(ProfileSessionDataItem* pDataItem)
{
    GT_IF_WITH_ASSERT(pDataItem)
    {
        QList<ProfileSessionDataItem*> items;
        // sync selection trace tables
        int itemSampleId = pDataItem->SampleId();
        ClearSelectionInTraceTables(pDataItem->IsCPUItem());

        if (itemSampleId != 0)
        {
            if (pDataItem->IsCPUItem())
            {
                m_pSessionDataContainer->GetGPUItemsBySampleId(itemSampleId, items);
            }
            else
            {
                m_pSessionDataContainer->GetCPUItemsBySampleId(itemSampleId, items);
            }

            for (auto item : items)
            {
                SelectItemInTraceTables(item, false);
            }
        }

        SyncSelectionInSummary(pDataItem);

        for (auto item : items)
        {
            SyncSelectionInSummary(item);
        }
    }
}

acTimelineItem* gpTraceView::SessionItemToTimelineItem(ProfileSessionDataItem* pDataItem)
{
    acTimelineItem* pItem = nullptr;
    GT_IF_WITH_ASSERT(pDataItem != nullptr)
    {
        APICallId id;
        id.m_tid = pDataItem->ThreadID();
        id.m_queueName = pDataItem->QueueName();
        id.m_callIndex = pDataItem->APICallIndex();
        pItem = GetAPITimelineItem(id);
    }
    return pItem;
}
