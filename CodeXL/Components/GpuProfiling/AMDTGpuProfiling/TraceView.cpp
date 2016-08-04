//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/TraceView.cpp $
/// \version $Revision: #144 $
/// \brief :  This file contains TraceView
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/TraceView.cpp#144 $
// Last checkin:   $DateTime: 2016/03/30 04:04:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 566240 $
//==============

#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

#include <CL/cl.h>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/TraceView.h>
#include <AMDTGpuProfiling/TraceTable.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/CLTimelineItems.h>
#include <AMDTGpuProfiling/HSATimelineItems.h>
#include <AMDTGpuProfiling/KernelOccupancyWindow.h>
#include <AMDTGpuProfiling/SymbolInfo.h>
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/SessionViewTabWidget.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>


static QList<TraceTableModel::TraceTableColIndex> s_hsaHiddenColumns = { TraceTableModel::TRACE_DEVICE_BLOCK_COLUMN, TraceTableModel::TRACE_OCCUPANCY_COLUMN, TraceTableModel::TRACE_DEVICE_TIME_COLUMN };

static const unsigned int s_UI_REFRESH_RATE = 1000;
static const unsigned int s_MAX_TRACE_ENTRIES = 200000;
static const int PROGRESS_STAGES = 6;

TraceView::TraceView(QWidget* parent) : gpBaseSessionView(parent),
    m_pCurrentSession(nullptr),
    m_pMainSplitter(nullptr),
    m_pTimeline(nullptr),
    m_pTraceTabView(nullptr),
    m_pSummaryView(nullptr),
    m_pSymbolInfo(nullptr),
    m_pSummarizer(nullptr),
    m_pOpenCLBranch(nullptr),
    m_pHSABranch(nullptr),
    m_perfMarkersAdded(false)
#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
    , m_lastDeviceItem(nullptr),
    m_lastDeviceItemIdx(-1)
#endif
    , m_areTimelinePropertiesSet(false),
    m_parseCallsCounter(0),
    m_alreadyDisplayedAPILimitMessage(false),
    m_shouldStopParsing(false),
    m_maxTimestampWhenParsingStopped(0),
    m_isProgressRangeSet(false)
{
    BuildWindowLayout();

    // Add the actions to the table:
    m_pCopyAction = m_pTraceTableContextMenu->addAction(AF_STR_CopyA, this, SLOT(OnEditCopy()));
    m_pSelectAllAction = m_pTraceTableContextMenu->addAction(AF_STR_SelectAllA, this, SLOT(OnEditSelectAll()));
    m_pTraceTableContextMenu->addSeparator();

    m_pExpandAllAction = m_pTraceTableContextMenu->addAction(GPU_STR_TraceViewExpandAll, this, SLOT(OnExpandAll()));
    m_pCollapseAllAction = m_pTraceTableContextMenu->addAction(GPU_STR_TraceViewCollapseAll, this, SLOT(OnCollapseAll()));
    m_pTraceTableContextMenu->addSeparator();

    m_pGotoSourceAction = m_pTraceTableContextMenu->addAction(GPU_STR_TraceViewGoToSource, this, SLOT(OnGotoSource()));
    m_pZoomInTimelineAction = m_pTraceTableContextMenu->addAction(GPU_STR_TraceViewZoomTimeline, this, SLOT(OnZoomItemInTimeline()));
    m_pTraceTableContextMenu->addSeparator();

    m_pExportToCSVAction = m_pTraceTableContextMenu->addAction(AF_STR_ExportToCSV, this, SLOT(OnExportToCSV()));


    bool rc = connect(m_pTimeline, SIGNAL(itemClicked(acTimelineItem*)), this, SLOT(TimelineItemClickedHandler(acTimelineItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pTimeline, SIGNAL(branchClicked(acTimelineBranch*)), this, SLOT(TimelineBranchClickedHandler(acTimelineBranch*)));
    GT_ASSERT(rc);

    rc = connect(afApplicationCommands::instance()->applicationTree()->treeControl(), SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnApplicationTreeSelection()));
    GT_ASSERT(rc);

}

TraceView::~TraceView()
{
    SAFE_DELETE(m_pSummarizer);

    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);
}


bool TraceView::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    bool retVal = true;

    // If this is a new session file, load the session
    if (m_sessionFilePath != sessionFilePath)
    {
        // Call the base class implementation
        retVal = SharedSessionWindow::DisplaySession(sessionFilePath, sessionInnerPage, errorMessage);

        // Initialize the session file path:
        m_sessionFilePath = sessionFilePath;

        m_pCurrentSession = nullptr;
        m_alreadyDisplayedAPILimitMessage = false;
        m_shouldStopParsing = false;
        m_maxTimestampWhenParsingStopped = 0;
        m_pCurrentSession = qobject_cast <TraceSession*> (m_pSessionData);


        // Sanity check:
        GT_IF_WITH_ASSERT((m_pCurrentSession != nullptr) && (m_pCurrentSession->m_pParentData != nullptr))
        {
            // Reset the flag stating the the occupancy file was loaded:
            m_pCurrentSession->ResetOccupancyFileLoad();

            if (m_pCurrentSession->m_pParentData->m_filePath.exists())
            {
                int thisSessionMajor = m_pCurrentSession->GetVersionMajor();
                int thisSessionMinor = m_pCurrentSession->GetVersionMinor();

                if ((thisSessionMajor > GPUPROFILER_BACKEND_MAJOR_VERSION) || (thisSessionMajor == GPUPROFILER_BACKEND_MAJOR_VERSION && thisSessionMinor > GPUPROFILER_BACKEND_MINOR_VERSION))
                {
                    // Output a message to the user
                    errorMessage = GP_Str_NewerTraceSession;

                    // Hide the progress dialog
                    afProgressBarWrapper::instance().hideProgressBar();

                    retVal = false;
                }
                else
                {
                    LoadSessionUsingBackendParser(m_pCurrentSession->m_pParentData->m_filePath);

                    // if we didn't load symbol info from .atp file, try loading it from .st file
                    if (m_symbolTableMap.isEmpty())
                    {
                        osFilePath stFilePath = m_pCurrentSession->m_pParentData->m_filePath;
                        stFilePath.setFileExtension(L"st");

                        if (stFilePath.exists())
                        {
                            LoadSessionUsingBackendParser(stFilePath);
                        }
                    }

                    // if we didn't load CL perfmarker info from .atp file, try loading it from .clperfmarker file
                    if (!m_perfMarkersAdded)
                    {
                        osFilePath perfMarkerFilePath = m_pCurrentSession->m_pParentData->m_filePath;
                        perfMarkerFilePath.setFileExtension(L"clperfmarker");

                        if (perfMarkerFilePath.exists())
                        {
                            LoadSessionUsingBackendParser(perfMarkerFilePath);
                        }
                    }

                    DoneParsingATPFile();

                    // Load the summary view
                    LoadSummary(sessionInnerPage);

                    afProgressBarWrapper::instance().hideProgressBar();

                }
            }
        }
    }

    if ((m_pSummaryView != nullptr) && (m_pTraceTabView != nullptr) && (m_pSessionTabWidget != nullptr))
    {
        m_pSummaryView->DisplaySummaryPageType(sessionInnerPage);

        // Find the summary view tab index
        int summaryTabIndex = -1;

        for (int i = 0; i < m_pTraceTabView->count(); i++)
        {
            if (m_pTraceTabView->tabText(i) == GPU_STR_TraceViewSummary)
            {
                summaryTabIndex = i;
                break;
            }
        }

        if (summaryTabIndex >= 0)
        {
            m_pTraceTabView->setCurrentIndex(summaryTabIndex);
        }

        // Set the timeline as the current index
        m_pSessionTabWidget->setCurrentIndex(0);
    }

    return retVal;
}

void TraceView::LoadSummary(afTreeItemType summaryItemType)
{
    if (m_pSummarizer == nullptr)
    {
        m_pSummarizer = new CLSummarizer(m_pCurrentSession);
    }

    m_pSummarizer->CreateSummaryPages();

    if (m_pTraceTabView != nullptr && m_pSummarizer->GetSummaryPagesMap().count() != 0)
    {
        if (m_pSummaryView == nullptr)
        {
            m_pSummaryView = new SummaryView(this);

            connect(m_pSummaryView, SIGNAL(LinkClicked(const QString&, unsigned int, unsigned int, AnalyzerHTMLViewType)),
                    this, SLOT(SummaryPageLinkClickedHandler(const QString&, unsigned int, unsigned int, AnalyzerHTMLViewType)));
        }

        GT_IF_WITH_ASSERT(m_pSummaryView->LoadSession(m_pCurrentSession, m_pSummarizer))
        {
            int indexOfSummary = m_pTraceTabView->addTab(m_pSummaryView, GPU_STR_TraceViewSummary);
            m_pTraceTabView->setCurrentIndex(indexOfSummary);
            m_pSummaryView->DisplaySummaryPageType(summaryItemType);
        }
    }
}

void TraceView::AddTraceTable(TraceTableModel* pModel, unsigned int threadId)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pModel != nullptr)
    {
        // Check if the table contain API calls. If the table is empty (due to limited API calls that we load),
        // do not add it to the tab view:
        if (!pModel->IsEmpty())
        {
            // Initialize the model:
            bool rc = pModel->InitializeModel();
            GT_ASSERT(rc);

            // Update the UI:
            // handles [BUG438186]
            // don't create trace view tab if model don't have items (only known by reserved items in this point)
            GT_IF_WITH_ASSERT(qApp != nullptr && (pModel->GetReservedApiCallsTraceItems() > 0))
            {

                QString strTabCaption = QString(GPU_STR_TraceViewHostThreadBranchName).arg(threadId);
                TraceTable* pNewTable = new TraceTable(this, threadId);

                // Do not set this delegate item, it causes performance overhead which we cannot afford:
                // pNewTable->setItemDelegate(new acItemDelegate);

                pNewTable->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(pNewTable, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(TraceTableContextMenuHandler(const QPoint&)));

                connect(pNewTable, SIGNAL(clicked(QModelIndex)), this, SLOT(TraceTableMouseClickedHandler(const QModelIndex&)));
                connect(pNewTable, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(TraceTableMouseDoubleClickedHandler(const QModelIndex&)));
                connect(pNewTable, SIGNAL(entered(const QModelIndex&)), this, SLOT(TraceTableMouseEnteredHandler(const QModelIndex&)));

                pNewTable->setModel(pModel);

                if (m_api == APIToTrace_HSA)
                {
                    pNewTable->SetHiddenColumnsList(s_hsaHiddenColumns);
                }

                GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
                {
                    m_pTraceTabView->addTab(pNewTable, strTabCaption);
                    pNewTable->expandAll();
                }
            }
        }
    }
}

void TraceView::Clear()
{
    m_pTimeline->reset();

    if (m_pSummaryView != nullptr)
    {
        m_pSummaryView->Reset();
    }

    SAFE_DELETE(m_pSummarizer);
    m_pTraceTabView->clear();

    if (m_pCurrentSession != nullptr)
    {
        m_pCurrentSession->FlushData();
    }

    m_modelMap.clear();
    m_hostBranchMap.clear();
    m_symbolTableMap.clear();
    m_pOpenCLBranch = nullptr;
    m_pHSABranch = nullptr;
    m_oclCtxMap.clear();
    m_oclQueueMap.clear();
    m_hsaQueueMap.clear();
    m_oclThreadOccIndexMap.clear();
    m_hsaThreadOccIndexMap.clear();
    m_timestampStack.clear();
    m_titleStack.clear();
    m_branchStack.clear();
    m_perfMarkersAdded = false;
#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
    m_lastDeviceItem = nullptr;
    m_lastDeviceItemIdx = -1;
#endif
}

SymbolInfo* TraceView::GetSymbolInfo(int threadId, int callIndex)
{
    SymbolInfo* pRetVal = nullptr;

    if (callIndex >= 0 && m_symbolTableMap.contains(threadId))
    {
        QList<SymbolInfo*> list = m_symbolTableMap[threadId];

        if (list.count() > callIndex)
        {
            pRetVal = list[callIndex];
        }
    }

    return pRetVal;
}

bool TraceView::SessionHasSymbolInformation()
{
    return m_symbolTableMap.isEmpty();
}

void TraceView::TraceTableMouseClickedHandler(const QModelIndex& modelIndex)
{
    TraceTableItem* item = static_cast<TraceTableItem*>(modelIndex.internalPointer());
    QTreeView* table = dynamic_cast<QTreeView*>(sender());

    if (table != nullptr)
    {
        if (modelIndex.column() == TraceTableModel::TRACE_OCCUPANCY_COLUMN)
        {

            OccupancyInfo* occInfo = item->GetOccupancyInfo();

            if (occInfo != nullptr)
            {
                m_strOccupancyKernelName = occInfo->GetKernelName();

                // get the api index from the Index column (column 0)
                QString strCallIndex;
                strCallIndex = item->GetColumnData(TraceTableModel::TRACE_INDEX_COLUMN).toString();

                bool ok;
                int callIndex = strCallIndex.toInt(&ok);

                // if we got a valid call index, then show the occupancy view
                if (ok)
                {
                    m_currentDisplayedOccupancyKernel = occInfo->GetKernelName();

                    QString strErrorMessageOut;
                    connect(ProfileManager::Instance(), SIGNAL(OccupancyFileGenerationFinished(bool, const QString&, const QString&)), this, SLOT(OnOccupancyFileGenerationFinish(bool, const QString&, const QString&)));
                    // Generate occupancy page
                    bool retVal = ProfileManager::Instance()->GenerateOccupancyPage(m_pCurrentSession, occInfo, callIndex, strErrorMessageOut);

                    if (!retVal)
                    {
                        Util::ShowErrorBox(strErrorMessageOut);
                    }
                }
            }
        }
        else if (modelIndex.column() == TraceTableModel::TRACE_DEVICE_BLOCK_COLUMN)
        {
            acTimelineItem* deviceBlockItem = item->GetDeviceBlock();

            if (deviceBlockItem != nullptr)
            {
                // at this point, we know that the user has clicked a device block cell
                m_pTimeline->ZoomToItem(deviceBlockItem, true);
            }
        }
    }
}

void TraceView::TraceTableMouseDoubleClickedHandler(const QModelIndex& modelIndex)
{
    // Get the activated item:
    TraceTableItem* pItem = static_cast<TraceTableItem*>(modelIndex.internalPointer());
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        // Zoom the timeline into the double-clicked item:
        m_pTimeline->ZoomToItem(pItem->GetTimelineItem(), true);

        // Display the item properties:
        DisplayItemInPropertiesView(pItem->GetTimelineItem());
    }

}

void TraceView::TraceTableMouseEnteredHandler(const QModelIndex& modelIndex)
{
    TraceTableItem* item = static_cast<TraceTableItem*>(modelIndex.internalPointer());
    // change the mouse cursor to the hand cursor when hovering over a device block item or a kernel occupancy item
    QTreeView* table = dynamic_cast<QTreeView*>(sender());

    if (table != nullptr)
    {
        OccupancyInfo* occInfo = item->GetOccupancyInfo();
        acTimelineItem* deviceBlockItem = item->GetDeviceBlock();

        if ((occInfo != nullptr && modelIndex.column() == TraceTableModel::TRACE_OCCUPANCY_COLUMN) || (deviceBlockItem != nullptr && modelIndex.column() == TraceTableModel::TRACE_DEVICE_BLOCK_COLUMN))
        {
            // we are over a cell with either a kernel occupancy figure or a device block pItem
            table->setCursor(Qt::PointingHandCursor);
            return;
        }

        table->setCursor(Qt::ArrowCursor);
    }
}

void TraceView::TimelineItemClickedHandler(acTimelineItem* pItem)
{
    // show the corresponding API trace pItem when clicking a timeline pItem
    if (pItem != nullptr)
    {
        // Update properties with the details of this pItem:
        DisplayItemInPropertiesView(pItem);
        m_areTimelinePropertiesSet = true;

        // first get the correct pItem (if user clicks a timeline pItem for a device)
        HostAPITimelineItem* hostApiItem = dynamic_cast<HostAPITimelineItem*>(pItem);
        PerfMarkerTimelineItem* pPerfItem = dynamic_cast<PerfMarkerTimelineItem*>(pItem);
        TraceTableItem* pTableItem = nullptr;

        if (hostApiItem != nullptr)
        {
            pItem = hostApiItem->hostItem();
        }

        APITimelineItem* apiItem = dynamic_cast<APITimelineItem*>(pItem);


        if (apiItem != nullptr)
        {
            pTableItem = apiItem->traceTableItem();
        }

        if (pPerfItem != nullptr)
        {
            pTableItem = pPerfItem->traceTableItem();
        }

        if (pItem && pItem->parentBranch() != nullptr)
        {
            QTreeView* treeview = nullptr;
            acTimelineBranch* branch = pItem->parentBranch();
            int tabIndex = -1;

            while (branch != nullptr && tabIndex == -1)
            {
                QString strTabTextToFind = branch->text();
                tabIndex = GetTabIndex(strTabTextToFind);
                branch = branch->parentBranch();
            }

            if (tabIndex != -1)
            {
                m_pTraceTabView->setCurrentIndex(tabIndex);
                treeview = dynamic_cast<QTreeView*>(m_pTraceTabView->currentWidget());
            }

            if (treeview != nullptr && pTableItem != nullptr)
            {
                // select the api that corresponds to the timeline pItem clicked
                QModelIndexList selectionList = treeview->model()->match(treeview->model()->index(0, 0), Qt::UserRole, pTableItem->GetUniqueId(), 1, Qt::MatchExactly | Qt::MatchRecursive);
                treeview->setCurrentIndex(selectionList.first());
            }
        }
    }
}

void TraceView::TimelineBranchClickedHandler(acTimelineBranch* branch)
{
    // show the corresponding API trace tab when clicking a timeline branch

    int tabIndex = -1;

    while (branch != nullptr && tabIndex == -1)
    {
        QString strTabTextToFind = branch->text();
        tabIndex = GetTabIndex(strTabTextToFind);
        branch = branch->parentBranch();
    }

    if (tabIndex != -1)
    {
        m_pTraceTabView->setCurrentIndex(tabIndex);
    }
}

HSADispatchTimelineItem* CheckBranchForItem(acTimelineBranch* pBranch, unsigned int callIndex, bool recurse = true)
{
    if (nullptr != pBranch)
    {
        for (int i = 0; i < pBranch->itemCount(); i++)
        {
            HSADispatchTimelineItem* hsaDispatchItem = dynamic_cast<HSADispatchTimelineItem*>(pBranch->getTimelineItem(i));

            if (nullptr != hsaDispatchItem && callIndex == (unsigned int)hsaDispatchItem->apiIndex())
            {
                return hsaDispatchItem;
            }
        }

        if (recurse)
        {
            for (int i = 0; i < pBranch->subBranchCount(); i++)
            {
                HSADispatchTimelineItem* item = CheckBranchForItem(pBranch->getSubBranch(i), callIndex, recurse);

                if (nullptr != item)
                {
                    return item;
                }
            }
        }
    }

    return nullptr;
}

void TraceView::SummaryPageLinkClickedHandler(const QString& strPageName, unsigned int threadID, unsigned int callIndex, AnalyzerHTMLViewType viewType)
{
    QString strHostThread = QString(GPU_STR_TraceViewHostThreadBranchName).arg(threadID);

    // determine if the page containing the link is an HSA page (by looking at the prefix of the page)
    QString strPrefix = GPU_STR_TraceViewOpenCL;
    QStringList pageNameTokens = strPageName.split(' ');

    if (pageNameTokens.count() > 0)
    {
        if (pageNameTokens[0] == GPU_STR_TraceViewHSA)
        {
            strPrefix = GPU_STR_TraceViewHSA;
        }
    }

    switch (viewType)
    {

        case AnalyzerHTMLViewType_TimelineHost:
        case AnalyzerHTMLViewType_TimelineDevice:
        {
            acTimelineBranch* pBranch = m_pTimeline->getBranchFromText(strHostThread, false);

            if (nullptr != pBranch)
            {
                // find subbranch with the specified prefix name
                for (int i = 0; i < pBranch->subBranchCount(); i++)
                {
                    if (pBranch->getSubBranch(i)->text() == strPrefix)
                    {
                        pBranch = pBranch->getSubBranch(i);
                        break;
                    }
                }
            }

            if (nullptr != pBranch)
            {
                // get the specified timeline item
                acTimelineItem* pItem = pBranch->getTimelineItem(callIndex);

                if (nullptr != pItem)
                {
                    // if the item has a device item, then show that
                    DispatchAPITimelineItem* pEnqueueItem = dynamic_cast<DispatchAPITimelineItem*>(pItem);

                    if ((AnalyzerHTMLViewType_TimelineDevice == viewType) && (nullptr != pEnqueueItem) && (nullptr != pEnqueueItem->deviceItem()))
                    {
                        m_pTimeline->ZoomToItem(pEnqueueItem->deviceItem(), true);
                    }
                    else
                    {
                        m_pTimeline->ZoomToItem(pItem, true);
                    }
                }
            }

            break;
        }

        case AnalyzerHTMLViewType_TimelineDeviceNoAPI:
        {
            // there is not a host-side API for this -- most likely an HSA kernel dispatch
            acTimelineBranch* pBranch = m_pTimeline->getBranchFromText(GPU_STR_TraceViewHSA, false, false);

            if (nullptr != pBranch)
            {
                HSADispatchTimelineItem* pItem = CheckBranchForItem(pBranch, callIndex);

                if (nullptr != pItem)
                {
                    m_pTimeline->ZoomToItem(pItem, true);
                }
            }

            break;
        }

        case AnalyzerHTMLViewType_Trace:
        {
            QTreeView* treeview = nullptr;

            for (int tabIndex = 0; tabIndex < m_pTraceTabView->count(); tabIndex++)
            {
                if (m_pTraceTabView->tabText(tabIndex) == strHostThread)
                {
                    m_pTraceTabView->setCurrentIndex(tabIndex);
                    treeview = dynamic_cast<QTreeView*>(m_pTraceTabView->currentWidget());
                    break;
                }
            }

            if (treeview != nullptr)
            {
                QString strUniqueId = strPrefix;
                strUniqueId.append('.').append(QString::number(callIndex));
                QModelIndexList selectionList = treeview->model()->match(treeview->model()->index(0, 0), Qt::UserRole, QVariant::fromValue(strUniqueId), 1, Qt::MatchExactly | Qt::MatchRecursive);

                if (!selectionList.empty())
                {
                    treeview->setCurrentIndex(selectionList.first());
                }
                else
                {
                    // TODO: display a message that you can't navigate to the api because it wasnt loaded (because it was an API after the 200k limit)
                }
            }

            break;
        }

        default:
            break;
    }
}

void TraceView::TraceTableContextMenuHandler(const QPoint& pt)
{
    TraceTable* table = dynamic_cast<TraceTable*>(sender());

    if (table != nullptr)
    {
        // get the thread and callindex
        int threadId = table->GetThreadId();
        QString strMenuText(GPU_STR_TraceViewGoToSource);
        bool isSourceCodeEnabled = false;
        bool isCopyEnabled = false;
        bool isSelectAllEnabled = false;
        bool isExpandEnabled = false;

        QModelIndex index = table->indexAt(pt);
        QString strUniqueId = table->model()->data(index, Qt::UserRole).toString();
        QStringList uniqueIdTokens = strUniqueId.split(".");
        int row = -1;
        AGP_TODO("need to revisit if symbol info becomes available for HSA APIs")
        isExpandEnabled = table->ShouldExpandBeEnabled();

        if (uniqueIdTokens.count() > 1 && (uniqueIdTokens[0] == GPU_STR_TraceViewOpenCL || uniqueIdTokens[0] == GPU_STR_TraceViewHSA))
        {
            bool ok;
            row = uniqueIdTokens[1].toInt(&ok);

            if (!ok)
            {
                row = -1;
            }
        }

        // Check if copy and select all actions are enabled:
        isCopyEnabled = !table->selectionModel()->selectedIndexes().isEmpty();
        isSelectAllEnabled = (table->model()->rowCount() > 0);

        if (row >= 0)
        {
            // get the symbol info for that call, store it in m_pSymbolInfo for use in the menu trigger handler
            m_pSymbolInfo = GetSymbolInfo(threadId, row);
        }

        if (m_pSymbolInfo != nullptr)
        {
            isSourceCodeEnabled = true;
        }
        else
        {
            // change the caption if the option to allow source code navigation is not enabled (gives a hint to the user)
            if (!SessionHasSymbolInformation())
            {
                strMenuText.append(QString(" (Enable navigation to source code on the \"%1\" project setting page.)").arg(Util::ms_APP_TRACE_OPTIONS_PAGE));
            }

            isSourceCodeEnabled = false;
        }

        // disable context menu actions on selection of more then 1 row (not copy and select all)
        bool isMultiRowSelection = (table->NumOfSelectedRows() > 1);

        m_pGotoSourceAction->setText(strMenuText);
        m_pGotoSourceAction->setEnabled(isSourceCodeEnabled && !isMultiRowSelection);
        m_pExpandAllAction->setEnabled(isExpandEnabled);
        m_pCollapseAllAction->setEnabled(isExpandEnabled);
        m_pZoomInTimelineAction->setEnabled(isCopyEnabled && !isMultiRowSelection);
        m_pCopyAction->setEnabled(isCopyEnabled);
        m_pSelectAllAction->setEnabled(isSelectAllEnabled);
        m_pTraceTableContextMenu->exec(acMapToGlobal(table, pt));
    }
}

void TraceView::OnGotoSource()
{
    if (m_pSymbolInfo != nullptr)
    {
        osFilePath filePath(acQStringToGTString(m_pSymbolInfo->FileName()));

        if (filePath.isEmpty())
        {
            QString strExtraInfo;

            strExtraInfo = GP_Str_InvalidDebugInfo;

            // if file name, api name, symbol name, and line number are all empty, then that is an indication that there is no entry in the .st file for the selected API
            if (m_pSymbolInfo->ApiName().isEmpty() && m_pSymbolInfo->SymbolName().isEmpty() && m_pSymbolInfo->LineNumber() == 0)
            {
                strExtraInfo += QString(GP_Str_AppTerminatedUnexpectedly).arg(Util::ms_ENABLE_TIMEOUT_OPTION);
            }

            Util::ShowWarningBox(QString(GP_Str_NoSourceInfoForSelectedAPI).arg(strExtraInfo));
        }
        else if (filePath.exists() && filePath.isRegularFile())
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                pApplicationCommands->OpenFileAtLine(filePath, m_pSymbolInfo->LineNumber(), -1);
            }
        }
        else
        {
            // check to see if this file is a file from an installed sample file (in this case, the path in the debug info might not match the path to the file on disk)
            filePath = Util::GetInstalledPathForSampleFile(filePath);

            if (filePath.exists() && filePath.isRegularFile())
            {
                afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
                {
                    pApplicationCommands->OpenFileAtLine(filePath, m_pSymbolInfo->LineNumber(), -1);
                }
            }
            else
            {
                Util::ShowWarningBox(QString(GP_Str_CantAccessSourceForSelectedAPI).arg(acGTStringToQString(filePath.asString())));
            }
        }

        m_pSymbolInfo = nullptr;
    }
}

void TraceView::SetAPINum(osThreadId threadId, unsigned int apiNum)
{
    TraceTableModel* pTableModel = nullptr;

    if (m_modelMap.contains(threadId))
    {
        pTableModel = m_modelMap[threadId];
    }
    else
    {
        pTableModel = new TraceTableModel(this);

        pTableModel->SetVisualProperties(palette().color(QPalette::Text), palette().color(QPalette::Link), font());
        m_modelMap.insert(threadId, pTableModel);
    }

    GT_IF_WITH_ASSERT(pTableModel != nullptr)
    {
        pTableModel->SetAPICallsNumber(apiNum);
    }
}

bool TraceView::CheckStopParsing(quint64 curEndTime)
{
    bool stopParsing = false;

    // On windows, we are limited with a 2GB memory size, so we limit the API calls load to 200K.
    // On linux this limitation is not relevant, so we do not stop parse:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    stopParsing = m_parseCallsCounter >= s_MAX_TRACE_ENTRIES;

    if (stopParsing)
    {
        // CODEXL-3550: If the API limit was reached, continue to parse additional items (HSA kernel timestamps, perf markers, etc.), up to the point (in time) where api parsing stopped.
        // This ensures that the partial timeline shown will at least be complete (in the sense that all items in the given timespan will be shown):
        stopParsing = ((0 == m_maxTimestampWhenParsingStopped || curEndTime != std::numeric_limits<quint64>::max()) && curEndTime > m_maxTimestampWhenParsingStopped);
    }

    if (stopParsing && !m_alreadyDisplayedAPILimitMessage)
    {
        // We ask the user if they want to continue loading the data at their own risk:
        QString userMessage = QString(GPU_STR_APICallsAmountExceedsTheLimitQuestion).arg(s_MAX_TRACE_ENTRIES);
        int userAnswer = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), userMessage, QMessageBox::No | QMessageBox::Yes);
        m_shouldStopParsing = stopParsing = (userAnswer == QMessageBox::Yes);

        m_alreadyDisplayedAPILimitMessage = true;

        if (m_shouldStopParsing)
        {
            m_maxTimestampWhenParsingStopped = curEndTime;
        }
    }
    else if (stopParsing && m_alreadyDisplayedAPILimitMessage)
    {
        stopParsing = m_shouldStopParsing;
    }

#endif

    return stopParsing;
}

void TraceView::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Save the API type for later use:
    m_api = APIToTrace_OPENCL;

    stopParsing = CheckStopParsing(pAPIInfo->m_ullEnd);
    m_parseCallsCounter++;

    if (!stopParsing)
    {
        HandleCLAPIInfo(pAPIInfo);
    }
}

void TraceView::OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing)
{
    // Save the API type for later use:
    m_api = APIToTrace_HSA;

    stopParsing = CheckStopParsing(pAPIInfo->m_ullEnd);
    m_parseCallsCounter++;

    if (!stopParsing)
    {
        HandleHSAAPIInfo(pAPIInfo);
    }
}

void TraceView::OnParse(SymbolFileEntry* pSymFileEntry, bool& stopParsing)
{
    AGP_TODO("should check the module of pSymFileEntry and match it up to that module's APIs. This will be needed to properly support multi-module traces (i.e. traces that contain both HSA and OCL)")
    m_parseCallsCounter++;
    stopParsing = CheckStopParsing(std::numeric_limits<quint64>::max());

    if (!stopParsing)
    {
        HandleSymFileEntry(pSymFileEntry);
    }
}

void TraceView::OnParse(PerfMarkerEntry* pPerfMarkerEntry, bool& stopParsing)
{
    stopParsing = CheckStopParsing(pPerfMarkerEntry->m_timestamp);
    m_parseCallsCounter++;

    if (!stopParsing)
    {
        HandlePerfMarkerEntry(pPerfMarkerEntry);
    }
}

void TraceView::OnParserProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    gtString localProgressMsg;
    afProgressBarWrapper& theProgressBarWrapper = afProgressBarWrapper::instance();

    localProgressMsg.fromASCIIString(strProgressMessage.c_str());

    // If this is the first item we're reporting to the progress indicator, or if the
    // progress dialog is not shown (was hidden by some other stage of the load?)
    if (uiCurItem == 0 || theProgressBarWrapper.IsDlgShown() == false)
    {
        // Make sure the progress dialog is displayed
        theProgressBarWrapper.ShowProgressDialog(uiCurItem, uiTotalItems);
        theProgressBarWrapper.setProgressText(localProgressMsg);

        // Store the message displayed on the progress dialog for later comparison to avoid unnecessary updates
        m_currentProgressMessage = strProgressMessage;
    }

    // Update the progress message only if it has changed
    if (m_currentProgressMessage != strProgressMessage)
    {
        theProgressBarWrapper.setProgressText(localProgressMsg);
        m_currentProgressMessage = strProgressMessage;
    }

    theProgressBarWrapper.updateProgressBar(uiCurItem);
}

acTimelineBranch* TraceView::GetHostBranchForAPI(osThreadId threadId, const QString& branchText)
{
    acTimelineBranch* retVal = nullptr;

    // first add the root branch, if necessary
    QString hostThreadLabel = QString(GPU_STR_TraceViewHostThreadBranchName).arg(threadId);

    acTimelineBranch* rootBranch = nullptr;

    if (m_hostBranchMap.contains(threadId))
    {
        rootBranch = m_hostBranchMap[threadId];
    }

    if (rootBranch == nullptr)
    {
        rootBranch = new acTimelineBranch();
        rootBranch->SetBGColor(QColor::fromRgb(230, 230, 230));
        rootBranch->setText(hostThreadLabel);
        m_hostBranchMap.insert(threadId, rootBranch);
    }

    retVal = rootBranch;

    if (!branchText.isEmpty())
    {
        // now add the API branch, if necessary
        retVal = rootBranch->getSubBranchFromText(branchText, false);

        if (retVal == nullptr)
        {
            retVal = new acTimelineBranch();
            retVal->SetBGColor(QColor::fromRgb(230, 230, 230));
            retVal->setText(branchText);
            rootBranch->addSubBranch(retVal);
        }
    }

    return retVal;
}

bool TraceView::LoadSessionUsingBackendParser(const osFilePath& sessionFile)
{
    bool retVal = false;

    Config config;
    AtpFileParser parser;

    // add part parser for OpenCL API Trace and Timestamp sections
    CLAtpFilePart clAtpPart(config);
    clAtpPart.AddProgressMonitor(this);
    parser.AddAtpFilePart(&clAtpPart);

    // add part parser for HSA API Trace and Timestamp sections
    HSAAtpFilePart hsaAtpPart(config);
    hsaAtpPart.AddProgressMonitor(this);
    parser.AddAtpFilePart(&hsaAtpPart);

    // add part parser for cl Stack Trace section
    StackTraceAtpFilePart clStackPart("ocl", config);
    clStackPart.AddProgressMonitor(this);
    parser.AddAtpFilePart(&clStackPart);

    // add part parser for hsa Stack Trace section
    StackTraceAtpFilePart hsaStackPart("hsa", config);
    hsaStackPart.AddProgressMonitor(this);
    parser.AddAtpFilePart(&hsaStackPart);

    PerfMarkerAtpFilePart perfMarkerPart(config);
    perfMarkerPart.AddProgressMonitor(this);
    parser.AddAtpFilePart(&perfMarkerPart);

    clAtpPart.AddListener(this);
    hsaAtpPart.AddListener(this);
    clStackPart.AddListener(this);
    hsaStackPart.AddListener(this);
    perfMarkerPart.AddListener(this);

    afApplicationCommands::instance()->StartPerformancePrintout("Parsing trace file");
    retVal = parser.LoadFile(sessionFile.asString().asASCIICharArray());
    retVal &= parser.Parse();
    bool parseWarning;
    std::string parseWarningMsg;
    parser.GetParseWarning(parseWarning, parseWarningMsg);

    if (false == retVal || true == parseWarning)
    {
        QString parseError = QString("Error parsing %1").arg(sessionFile.asString().asASCIICharArray());

        if (parseWarning)
        {
            parseError.append("\n").append(QString::fromStdString(parseWarningMsg));
        }

        Util::ShowWarningBox(parseError);
    }

    return retVal;
}

void TraceView::HandleCLAPIInfo(CLAPIInfo* pApiInfo)
{
    osThreadId threadId = pApiInfo->m_tid;

    TraceTableModel* tableModel = nullptr;
    acTimelineItem* deviceBlockItem = nullptr;
    OccupancyInfo* occupancyInfo = nullptr;

    const QList<OccupancyInfo*>* pOccupancyInfosList = nullptr;
    int nOccIndex = 0;

    if (m_pCurrentSession->LoadAndGetOccupancyTable().contains(threadId))
    {
        // temp member is needed to prevent class rvalue used as lvalue warning
        const QList<OccupancyInfo*> tempInfosList = m_pCurrentSession->GetOccupancyTable()[threadId];
        pOccupancyInfosList = &tempInfosList;

        if (m_oclThreadOccIndexMap.contains(threadId))
        {
            nOccIndex = m_oclThreadOccIndexMap[threadId];
        }
    }

    if (m_modelMap.contains(threadId))
    {
        tableModel = m_modelMap[threadId];
    }
    else
    {
        tableModel = new TraceTableModel(this);

        // Show all columns for CL API:
        tableModel->SetVisualProperties(palette().color(QPalette::Text), palette().color(QPalette::Link), font());
        m_modelMap.insert(threadId, tableModel);
    }

    acTimelineBranch* hostBranch = GetHostBranchForAPI(threadId, GPU_STR_TraceViewOpenCL);
    GT_ASSERT(hostBranch != nullptr);

    unsigned int apiID = pApiInfo->m_uiAPIID;

    if (!pApiInfo->m_strComment.empty())
    {
        pApiInfo->m_ArgList.append(" /* ").append(pApiInfo->m_strComment).append(" */");
    }

    QString apiName;

    if (apiID < CL_FUNC_TYPE_Unknown)
    {
        apiName = CLAPIDefs::Instance()->GetOpenCLAPIString(CL_FUNC_TYPE(apiID));
    }
    else
    {
        apiName = QString::fromStdString(pApiInfo->m_strName);
    }

    quint64 itemStartTime = pApiInfo->m_ullStart;
    quint64 itemEndTime = pApiInfo->m_ullEnd;

    // check for reasonable timestamps
    GT_ASSERT((itemEndTime >= itemStartTime) && itemStartTime != 0);

    APITimelineItem* pAPITimelineItem = nullptr;

#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY

    if (m_lastDeviceItemIdx != -1 && (apiID == CL_FUNC_TYPE_clFinish || apiID == CL_FUNC_TYPE_clWaitForEvents || apiID == CL_FUNC_TYPE_clGetEventInfo))
    {
        QString syncParams = QString::fromStdString(pApiInfo->m_ArgList);
        QString enqueueParams = tableModel->data(tableModel->index(m_lastDeviceItemIdx, TraceTableModel::TRACE_PARAMETERS_COLUMN), Qt::DisplayRole).toString();

        if (apiID == CL_FUNC_TYPE_clFinish)
        {
            // if the cmdQ of the clFinish call matches the cmdQ of the last enqueue call, then set the completion latency for the enqueue call
            QStringList syncParamList = syncParams.split(';');
            QStringList enqParamList = enqueueParams.split(';');

            if (syncParamList.first().trimmed() == enqParamList.first().trimmed())
            {
                m_lastDeviceItem->setCompletionLatencyTime(itemEndTime);
                m_lastDeviceItem = nullptr;
            }
        }
        else if (apiID == CL_FUNC_TYPE_clWaitForEvents)
        {
            // if the event list of the clWaitForEvents call contains the event of the last enqueue call, then set the completion latency for the enqueue call
            QStringList syncParamList = syncParams.split(';');
            QStringList enqParamList = enqueueParams.split(';');
            enqueueParams = enqParamList.last().trimmed();
            int beginIndex = enqueueParams.indexOf('[');
            int endIndex = enqueueParams.indexOf(']');
            enqueueParams = enqueueParams.mid(beginIndex + 1, endIndex - beginIndex - 1);

            syncParams = syncParamList.last().trimmed();
            beginIndex = syncParams.indexOf('[');
            endIndex = syncParams.indexOf(']');
            syncParams = syncParams.mid(beginIndex + 1, endIndex - beginIndex - 1);
            syncParamList = syncParams.split(',');

            if (syncParamList.contains(enqueueParams.trimmed()))
            {
                m_lastDeviceItem->setCompletionLatencyTime(itemEndTime);
                m_lastDeviceItem = nullptr;
            }
        }
        else if (apiID == CL_FUNC_TYPE_clGetEventInfo)
        {
            // if clGetEventInfo indicates COMPLETE and the event matches the event of the last enqueue call, then set the completion latency for the enqueue call
            if (syncParams.contains("[CL_COMPLETE]"))
            {
                // check if the event here matches the event of the last enqueue call
                QStringList syncParamList = syncParams.split(';');
                QStringList enqParamList = enqueueParams.split(';');
                syncParams = syncParamList[0].trimmed();
                enqueueParams = enqParamList.last().trimmed();

                if (enqueueParams.contains(syncParams))
                {
                    m_lastDeviceItem->setCompletionLatencyTime(itemEndTime);
                    m_lastDeviceItem = nullptr;
                }
            }
        }
    }

#endif

    if (apiID == CL_FUNC_TYPE_clGetEventInfo)
    {
        // don't assign an apiIndex to clGetEventInfo
        pAPITimelineItem = new CLGetEventInfoTimelineItem(itemStartTime, itemEndTime, pApiInfo->m_uiDisplaySeqID); //TODO verify that the tooltip show the index as "after index XXX" correctly
    }
    else
    {
        pAPITimelineItem = new APITimelineItem(itemStartTime, itemEndTime, pApiInfo->m_uiDisplaySeqID);
    }

    pAPITimelineItem->setText(apiName);
    pAPITimelineItem->setBackgroundColor(APIColorMap::Instance()->GetAPIColor(apiName, QColor(90, 90, 90)));
    pAPITimelineItem->setForegroundColor(Qt::white);

    if ((pApiInfo->m_Type & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API) // this is an enqueue api
    {
        CLEnqueueAPI* enqueueApiInfo = dynamic_cast<CLEnqueueAPI*>(pApiInfo);
        GT_IF_WITH_ASSERT(enqueueApiInfo != nullptr)
        {
            unsigned int cmdType = enqueueApiInfo->m_uiCMDType;
            QString strCmdType = QString::fromStdString(enqueueApiInfo->m_strCMDType);
            quint64 gpuStart = enqueueApiInfo->m_ullRunning;
            quint64 gpuEnd = enqueueApiInfo->m_ullComplete;
            quint64 gpuQueued = enqueueApiInfo->m_ullQueue;
            quint64 gpuSubmit = enqueueApiInfo->m_ullSubmit;

            QString strQueueHandle = QString::fromStdString(enqueueApiInfo->m_strCmdQHandle);
            unsigned int queueId = enqueueApiInfo->m_uiQueueID;

            QString strContextHandle = QString::fromStdString(enqueueApiInfo->m_strCntxHandle);
            unsigned int contextId = enqueueApiInfo->m_uiContextID;

            QString deviceNameStr = QString::fromStdString(enqueueApiInfo->m_strDevice);

            if ((gpuEnd < gpuStart && (apiID < CL_FUNC_TYPE_clEnqueueMapBuffer || apiID > CL_FUNC_TYPE_clEnqueueUnmapMemObject)) ||  gpuStart < gpuSubmit || gpuSubmit < gpuQueued)
            {
                if (cmdType <= CL_COMMAND_TASK && (deviceNameStr != GPU_STR_TraceViewCpuDevice))
                {
                    // if this is a kernel dispatch without valid timestamps, on a non-CPU device, then bump the
                    // occIndex so that subsequent dispatches are matched up with the correct occupancy info
                    // This fixes BUG355468
                    nOccIndex++;
                }
            }
            else
            {
                // Get or create the branch for this queue:
                OCLQueueBranchInfo* pBranchInfo = GetBranchInfo(contextId, queueId, strContextHandle, deviceNameStr, strQueueHandle);

                if ((pApiInfo->m_Type & CL_ENQUEUE_KERNEL) == CL_ENQUEUE_KERNEL)  // TODO does CL_COMMAND_NATIVE_KERNEL need special handling here????
                {
                    CLKernelAPIInfo* kernelApiInfo = dynamic_cast<CLKernelAPIInfo*>(pApiInfo);
                    GT_IF_WITH_ASSERT((kernelApiInfo != nullptr) && (pBranchInfo != nullptr) && (pBranchInfo->m_pQueueBranch != nullptr))
                    {
                        CLKernelTimelineItem* gpuItem = new CLKernelTimelineItem(gpuStart, gpuEnd, kernelApiInfo->m_uiDisplaySeqID);

#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
                        m_lastDeviceItem = gpuItem;
                        m_lastDeviceItemIdx = pApiInfo->m_uiSeqID;
#endif

                        APITimelineItem* newItem = new DispatchAPITimelineItem(gpuItem, pAPITimelineItem);

                        SAFE_DELETE(pAPITimelineItem);
                        pAPITimelineItem = newItem;

                        gpuItem->setText(QString::fromStdString(kernelApiInfo->m_strKernelName));
                        gpuItem->setGlobalWorkSize(QString::fromStdString(kernelApiInfo->m_strGlobalWorkSize));
                        gpuItem->setLocalWorkSize(QString::fromStdString(kernelApiInfo->m_strGroupWorkSize));
                        gpuItem->setBackgroundColor(pAPITimelineItem->backgroundColor());
                        gpuItem->setForegroundColor(Qt::white);
                        gpuItem->setQueueTime(gpuQueued);
                        gpuItem->setSubmitTime(gpuSubmit);
                        gpuItem->setDeviceType(deviceNameStr);
                        gpuItem->setCommandType(strCmdType);
                        gpuItem->setHostItem(pAPITimelineItem);

                        deviceBlockItem = gpuItem;

                        if (pOccupancyInfosList != nullptr)
                        {
                            if ((nOccIndex < pOccupancyInfosList->count()) && Util::CheckOccupancyDeviceName((*pOccupancyInfosList)[nOccIndex]->GetDeviceName(), deviceNameStr))
                            {
                                occupancyInfo = (*pOccupancyInfosList)[nOccIndex];
                                gpuItem->setOccupancyInfo(occupancyInfo);
                                nOccIndex++;
                            }
                        }

                        m_oclThreadOccIndexMap[threadId] = nOccIndex;

                        pBranchInfo->m_pKernelBranch->addTimelineItem(gpuItem);

                    }
                }
                else if ((pApiInfo->m_Type & CL_ENQUEUE_MEM) == CL_ENQUEUE_MEM)
                {
#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
                    m_lastDeviceItem = nullptr;
                    m_lastDeviceItemIdx = -1;
#endif
                    CLMemAPIInfo* memApiInfo = dynamic_cast<CLMemAPIInfo*>(pApiInfo);
                    GT_IF_WITH_ASSERT((memApiInfo != nullptr) && (pBranchInfo != nullptr) && (pBranchInfo->m_pMemoryBranch != nullptr))
                    {
                        CLAPITimelineItem* gpuItem = nullptr;

                        //                   if ((cmdType >= CL_COMMAND_READ_BUFFER && cmdType <= CL_COMMAND_MAP_IMAGE) || (cmdType >= CL_COMMAND_READ_BUFFER_RECT && cmdType <= CL_COMMAND_COPY_BUFFER_RECT))
                        gpuItem = new CLMemTimelineItem(gpuStart, gpuEnd, memApiInfo->m_uiDisplaySeqID);

                        quint64 transferSize = memApiInfo->m_uiTransferSize;
                        ((CLMemTimelineItem*)gpuItem)->setDataTransferSize(transferSize);
                        gpuItem->setText(CLMemTimelineItem::getDataSizeString(transferSize, 1) + " " + strCmdType.mid(11));

                        APITimelineItem* newItem = new DispatchAPITimelineItem(gpuItem, pAPITimelineItem);

                        SAFE_DELETE(pAPITimelineItem);
                        pAPITimelineItem = newItem;

                        gpuItem->setBackgroundColor(pAPITimelineItem->backgroundColor());
                        gpuItem->setForegroundColor(Qt::white);
                        gpuItem->setQueueTime(gpuQueued);
                        gpuItem->setSubmitTime(gpuSubmit);
                        gpuItem->setDeviceType(deviceNameStr);
                        gpuItem->setCommandType(strCmdType);
                        gpuItem->setHostItem(pAPITimelineItem);

                        pBranchInfo->m_pMemoryBranch->addTimelineItem(gpuItem);

                        deviceBlockItem = gpuItem;
                    }
                }
                else if ((pApiInfo->m_Type & CL_ENQUEUE_OTHER_OPERATIONS) == CL_ENQUEUE_OTHER_OPERATIONS)
                {
#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
                    m_lastDeviceItem = nullptr;
                    m_lastDeviceItemIdx = -1;
#endif
                    CLOtherEnqueueAPIInfo* pOtherEnqueueOperationsInfo = dynamic_cast<CLOtherEnqueueAPIInfo*>(pApiInfo);
                    GT_IF_WITH_ASSERT((pOtherEnqueueOperationsInfo != nullptr) && (pBranchInfo != nullptr) && (pBranchInfo->m_pQueueBranch != nullptr))
                    {
                        CLAPITimelineItem* gpuItem = nullptr;

                        quint64 startTime = pOtherEnqueueOperationsInfo->m_ullRunning;
                        quint64 endTime = pOtherEnqueueOperationsInfo->m_ullComplete;

                        // Prepare the command name:
                        QString commandName = strCmdType.replace("CL_COMMAND_", "");

                        if ((pApiInfo->m_Type & CL_ENQUEUE_DATA_OPERATIONS) == CL_ENQUEUE_DATA_OPERATIONS)
                        {
                            // if (cmdType == CL_COMMAND_FILL_IMAGE) || (cmdType == CL_COMMAND_FILL_BUFFER)) || (cmdType == CL_COMMAND_SVM_MAP) || cmdType == CL_COMMAND_SVM_UNMAP)
                            gpuItem = new CLDataEnqueueOperationsTimelineItem(startTime, endTime, pOtherEnqueueOperationsInfo->m_uiDisplaySeqID);

                            CLDataEnqueueAPIInfo* pDataEnqueueOperationsInfo = dynamic_cast<CLDataEnqueueAPIInfo*>(pApiInfo);
                            GT_IF_WITH_ASSERT((pDataEnqueueOperationsInfo != nullptr) && (pBranchInfo != nullptr) && (pBranchInfo->m_pQueueBranch != nullptr))
                            {
                                quint64 dataSize = pDataEnqueueOperationsInfo->m_uiDataSize;
                                ((CLDataEnqueueOperationsTimelineItem*)gpuItem)->setDataSize(dataSize);

                                commandName.prepend(CLMemTimelineItem::getDataSizeString(dataSize, 1) + " ");
                            }
                        }
                        else
                        {
                            gpuItem = new CLOtherEnqueueOperationsTimelineItem(startTime, endTime, pOtherEnqueueOperationsInfo->m_uiDisplaySeqID);
                        }

                        APITimelineItem* newItem = new DispatchAPITimelineItem(gpuItem, pAPITimelineItem);

                        gpuItem->setText(commandName);

                        SAFE_DELETE(pAPITimelineItem);
                        pAPITimelineItem = newItem;

                        gpuItem->setBackgroundColor(pAPITimelineItem->backgroundColor());
                        gpuItem->setForegroundColor(Qt::white);
                        gpuItem->setQueueTime(gpuQueued);
                        gpuItem->setSubmitTime(gpuSubmit);
                        gpuItem->setDeviceType(deviceNameStr);
                        gpuItem->setCommandType(strCmdType);
                        gpuItem->setHostItem(pAPITimelineItem);

                        if (pBranchInfo->m_pFillOperationBranch == nullptr)
                        {
                            pBranchInfo->m_pFillOperationBranch = new acTimelineBranch();
                            pBranchInfo->m_pFillOperationBranch->setText(tr("Other Enqueue Operations"));
                            pBranchInfo->m_pFillOperationBranch->SetBGColor(QColor::fromRgb(230, 230, 230));
                            pBranchInfo->m_pQueueBranch->addSubBranch(pBranchInfo->m_pFillOperationBranch);

                        }


                        GT_IF_WITH_ASSERT(pBranchInfo->m_pFillOperationBranch != nullptr)
                        {
                            pBranchInfo->m_pFillOperationBranch->addTimelineItem(gpuItem);
                        }

                        deviceBlockItem = gpuItem;
                    }
                }
            }
        }

    }

    if (pAPITimelineItem != nullptr)
    {
        hostBranch->addTimelineItem(pAPITimelineItem);
        pAPITimelineItem->setTraceTableItem(tableModel->AddTopLevelTraceItem(GPU_STR_TraceViewOpenCL, apiName, pApiInfo, pAPITimelineItem, deviceBlockItem, occupancyInfo));
    }

}

void TraceView::HandleHSAAPIInfo(HSAAPIInfo* pApiInfo)
{
    if (pApiInfo->m_bIsAPI)
    {
        osThreadId threadId = pApiInfo->m_tid;

        TraceTableModel* tableModel = nullptr;
        acTimelineItem* deviceBlockItem = nullptr;
        OccupancyInfo* occupancyInfo = nullptr;

        AGP_TODO("hook up occupancy info for HSA");
        //QList<OccupancyInfo*> occInfo;
        // int nOccIndex = 0;

        //if (m_pCurrentSession->GetOccupancyTable().contains(threadId))
        //{
        //    occInfo = m_pCurrentSession->GetOccupancyTable()[threadId];
        //    if (m_hsaThreadOccIndexMap.contains(threadId))
        //    {
        //        nOccIndex = m_hsaThreadOccIndexMap[threadId];
        //    }
        //}

        // if we already have CL data for this thread, then insert the HSA data as sub-nodes under the containing CL node, otherwise create a table map for HSA
        if (m_modelMap.contains(threadId))
        {
            tableModel = m_modelMap[threadId];
        }
        else
        {
            tableModel = new TraceTableModel(this);

            tableModel->SetVisualProperties(palette().color(QPalette::Text), palette().color(QPalette::Link), font());
            m_modelMap.insert(threadId, tableModel);
        }

        acTimelineBranch* hostBranch = GetHostBranchForAPI(threadId, GPU_STR_TraceViewHSA);
        GT_ASSERT(hostBranch != nullptr);

        unsigned int apiID = pApiInfo->m_apiID;
        QString apiName;

        if (apiID < HSA_API_Type_Init)
        {
            //apiName = CLAPIDefs::Instance()->GetOpenCLAPIString(CL_FUNC_TYPE(apiID));  //TODO : add a HSA version....
            apiName = QString::fromStdString(pApiInfo->m_strName);
        }
        else
        {
            apiName = QString::fromStdString(pApiInfo->m_strName);
        }

        quint64 itemStartTime = pApiInfo->m_ullStart;
        quint64 itemEndTime = pApiInfo->m_ullEnd;

        // check for reasonable timestamps
        GT_ASSERT((itemEndTime >= itemStartTime) && itemStartTime != 0);

        APITimelineItem* item = nullptr;

        HSAMemoryAPIInfo* pHsaMemoryAPIInfo = dynamic_cast<HSAMemoryAPIInfo*>(pApiInfo);

        if (nullptr != pHsaMemoryAPIInfo)
        {
            item = new HSAMemoryTimelineItem(itemStartTime, itemEndTime, pApiInfo->m_uiDisplaySeqID, pHsaMemoryAPIInfo->m_size, pHsaMemoryAPIInfo->m_shouldShowBandwidth);
        }
        else
        {
            item = new APITimelineItem(itemStartTime, itemEndTime, pApiInfo->m_uiDisplaySeqID);
        }

        if (nullptr != item)
        {
            item->setText(apiName);
            item->setBackgroundColor(APIColorMap::Instance()->GetAPIColor(apiName, QColor(90, 90, 90)));
            item->setForegroundColor(Qt::white);
            hostBranch->addTimelineItem(item);
        }

        item->setTraceTableItem(tableModel->AddTraceItem(GPU_STR_TraceViewHSA, apiName, pApiInfo, item, deviceBlockItem, occupancyInfo));
    }
    else if (HSA_API_Type_Non_API_Dispatch == pApiInfo->m_apiID)
    {
        HSADispatchInfo* dispatchInfo = dynamic_cast<HSADispatchInfo*>(pApiInfo);

        if (dispatchInfo != nullptr)
        {
            quint64 gpuStart = dispatchInfo->m_ullStart;
            quint64 gpuEnd = dispatchInfo->m_ullEnd;

            QString kernelNameStr = QString::fromStdString(dispatchInfo->m_strKernelName);
            QString deviceNameStr = QString::fromStdString(dispatchInfo->m_strDeviceName);

            if ((gpuEnd < gpuStart))
            {
                AGP_TODO("FIXME the commented out bit in the if statement")
                //if (cmdType <= CL_COMMAND_TASK && (deviceNameStr != GPU_STR_TraceViewCpuDevice))
                {
                    // if this is a kernel dispatch without valid timestamps, on a non-CPU device, then bump the
                    // occIndex so that subsequent dispatches are matched up with the correct occupancy info
                    // This fixes BUG355468
                    //nOccIndex++;
                }
            }
            else
            {
                // setup a device row based on deviceNameStr OR m_strQueueHandle ??
                acTimelineBranch* deviceBranch = nullptr;

                // Create HSA branch if it does not yet exist
                if (m_pHSABranch == nullptr)
                {
                    m_pHSABranch = new acTimelineBranch();
                    m_pHSABranch->SetBGColor(QColor::fromRgb(230, 230, 230));
                    m_pHSABranch->setText(GPU_STR_TraceViewHSA);
                }

                QString handleStr = QString::fromStdString(dispatchInfo->m_strQueueHandle);

                if (m_hsaQueueMap.contains(handleStr))
                {
                    deviceBranch = m_hsaQueueMap[handleStr];
                }
                else
                {
                    deviceBranch = new acTimelineBranch();
                    deviceBranch->SetBGColor(QColor::fromRgb(230, 230, 230));
                    QString queueBranchText;

                    if (handleStr == "<UnknownQueue>")
                    {
                        queueBranchText = deviceNameStr;
                    }
                    else
                    {
                        queueBranchText = QString(tr(GPU_STR_TraceViewQueueRow)).arg(dispatchInfo->m_queueIndex).arg(deviceNameStr).arg(handleStr);
                    }

                    deviceBranch->setText(queueBranchText);
                    m_hsaQueueMap[handleStr] = deviceBranch;
                }

                HSADispatchTimelineItem* dispatchItem = new HSADispatchTimelineItem(gpuStart, gpuEnd, dispatchInfo->m_uiSeqID);

                dispatchItem->setText(kernelNameStr);

                dispatchItem->setGlobalWorkSize(QString::fromStdString(dispatchInfo->m_strGlobalWorkSize));
                dispatchItem->setLocalWorkSize(QString::fromStdString(dispatchInfo->m_strGroupWorkSize));
                //dispatchItem->setOffset(QString::fromStdString(dispatchInfo->m_strOffset));
                dispatchItem->setDeviceType(deviceNameStr);
                dispatchItem->setQueueHandle(QString::fromStdString(dispatchInfo->m_strQueueHandle));

                dispatchItem->setBackgroundColor(Qt::darkGreen);
                dispatchItem->setForegroundColor(Qt::white);
                //dispatchItem->setHostItem(item);
                dispatchItem->setHostItem(nullptr);


                AGP_TODO("hook up occupancy info for HSA");
                //if ((nOccIndex < occInfo.count()) && Util::CheckOccupancyDeviceName(occInfo[nOccIndex]->GetDeviceName(), deviceNameStr))
                //{
                //    occupancyInfo = occInfo[nOccIndex];
                //    dispatchItem->setOccupancyInfo(occupancyInfo);
                //    nOccIndex++;
                //}
                //m_hsaThreadOccIndexMap[threadId] = nOccIndex;

                deviceBranch->addTimelineItem(dispatchItem);
            }
        }
    }
}

void TraceView::HandleSymFileEntry(SymbolFileEntry* pSymFileEntry)
{
    GT_IF_WITH_ASSERT(pSymFileEntry != nullptr)
    {
        osThreadId threadId = pSymFileEntry->m_tid;

        SymbolInfo* entry = nullptr;

        if ((pSymFileEntry->m_pStackEntry != nullptr) && (pSymFileEntry->m_pStackEntry->m_dwLineNum != (LineNum)(-1)) && (!pSymFileEntry->m_pStackEntry->m_strFile.empty()))
        {
            entry = new SymbolInfo(QString::fromStdString(pSymFileEntry->m_strAPIName),
                                   QString::fromStdString(pSymFileEntry->m_pStackEntry->m_strSymName),
                                   QString::fromStdString(pSymFileEntry->m_pStackEntry->m_strFile),
                                   pSymFileEntry->m_pStackEntry->m_dwLineNum);
        }
        else
        {
            entry = new SymbolInfo;
        }



        if (m_symbolTableMap.contains(threadId))
        {
            m_symbolTableMap[threadId].append(entry);
        }
        else
        {
            QList<SymbolInfo*> list;
            list.append(entry);
            m_symbolTableMap.insert(threadId, list);
        }
    }
}

acTimelineBranch* TraceView::GetPerfMarkerSubBranchHelper(const QString& name, acTimelineBranch* pParent)
{
    acTimelineBranch* ret = nullptr;
    GT_IF_WITH_ASSERT(pParent)
    {
        for (int i = 0; i < pParent->subBranchCount(); ++i)
        {
            acTimelineBranch* pb = pParent->getSubBranch(i);

            if (pb->text() == name)
            {
                ret = pb;
            }
        }

        if (ret == nullptr)
        {
            // Create a new branch for the added perf marker
            ret = new(std::nothrow) acTimelineBranch();

            ret->setParent(pParent);
            ret->setText(name);
            pParent->addSubBranch(ret);
        }
    }

    return ret;
}

void TraceView::HandlePerfMarkerEntry(PerfMarkerEntry* pPerfMarkerEntry)
{
    m_perfMarkersAdded = true;
    osThreadId threadId = pPerfMarkerEntry->m_tid;
    TraceTableModel* pTableModel = nullptr;

    acTimelineBranch* hostBranch = GetHostBranchForAPI(threadId, "");
    GT_ASSERT(hostBranch != nullptr);

    if (m_modelMap.contains(threadId))
    {
        pTableModel = m_modelMap[threadId];
    }
    else
    {
        pTableModel = new TraceTableModel(this);
        pTableModel->SetVisualProperties(palette().color(QPalette::Text), palette().color(QPalette::Link), font());
        m_modelMap.insert(threadId, pTableModel);
    }

    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (pPerfMarkerEntry->m_markerType == PerfMarkerEntry::PerfMarkerType_Begin)
    {
        PerfMarkerBeginEntry* pBeginMarkerEntry = dynamic_cast<PerfMarkerBeginEntry*>(pPerfMarkerEntry);
        GT_IF_WITH_ASSERT(pBeginMarkerEntry != nullptr)
        {
            if (m_branchStack.count() > 0)
            {
                hostBranch = m_branchStack.top();
            }

            acTimelineBranch* branchToUse = GetPerfMarkerSubBranchHelper(QString::fromStdString(pBeginMarkerEntry->m_strGroup), hostBranch);

            if (branchToUse != hostBranch)
            {
                m_branchStack.push(branchToUse);
            }

            m_titleStack.push(QString::fromStdString(pBeginMarkerEntry->m_strName));
            m_timestampStack.push(pBeginMarkerEntry->m_timestamp);

            // Add an item to the table:
            TraceTableItem* pTableItem = pTableModel->AddTraceItem("Perf Marker", QString::fromStdString(pBeginMarkerEntry->m_strName), pPerfMarkerEntry);
            GT_ASSERT(pTableItem != nullptr);
        }
    }
    else // PerfMarkerEntry::PerfMarkerType_End
    {
        if (m_timestampStack.isEmpty())
        {
            Util::LogError("Invalid input perfmarker file");
            return;
        }

        startTime = m_timestampStack.pop();
        endTime = pPerfMarkerEntry->m_timestamp;

        // Create the new time line item:
        PerfMarkerTimelineItem* pNewItem = new PerfMarkerTimelineItem(startTime, endTime);

        // Set the font and color for the perf marker item:
        pNewItem->setBackgroundColor(APIColorMap::Instance()->GetPerfMarkersColor());
        pNewItem->setForegroundColor(Qt::black);
        pNewItem->setText(m_titleStack.pop());

        // Add the timeline item to the branch:
        GT_IF_WITH_ASSERT(m_branchStack.count() > 0)
        {
            acTimelineBranch* currentBranch = m_branchStack.pop();
            currentBranch->addTimelineItem(pNewItem);
        }

        // Add an item to the table:
        TraceTableItem* pTableItem = pTableModel->CloseLastOpenedPerfMarker(pNewItem);
        GT_IF_WITH_ASSERT(pTableItem != nullptr)
        {
            pNewItem->setTraceTableItem(pTableItem);
        }
    }
}

void TraceView::DoneParsingATPFile()
{
    bool timelineDataLoaded = false;
    bool traceDataLoaded = false;
    m_parseCallsCounter = 0;

    afApplicationCommands::instance()->EndPerformancePrintout("Parsing trace file");

    if (!m_hostBranchMap.isEmpty())
    {
        timelineDataLoaded = true;
        acTimelineBranch* hostBranch = new acTimelineBranch();

        hostBranch->setText(tr("Host"));

        for (QMap<osThreadId, acTimelineBranch*>::const_iterator i = m_hostBranchMap.begin(); i != m_hostBranchMap.end(); ++i)
        {
            hostBranch->addSubBranch(*i);
        }

        m_pTimeline->addBranch(hostBranch);

        m_hostBranchMap.clear();
    }

    if (!m_oclCtxMap.isEmpty())
    {
        timelineDataLoaded = true;

        for (QMap<unsigned int, acTimelineBranch*>::const_iterator i = m_oclCtxMap.begin(); i != m_oclCtxMap.end(); ++i)
        {
            m_pOpenCLBranch->addSubBranch(*i);
        }

        m_pTimeline->addBranch(m_pOpenCLBranch);
        m_oclCtxMap.clear();
    }

    if (!m_oclQueueMap.isEmpty())
    {
        timelineDataLoaded = true;

        for (QMap<unsigned int, OCLQueueBranchInfo*>::iterator i = m_oclQueueMap.begin(); i != m_oclQueueMap.end(); ++i)
        {
            SAFE_DELETE(*i);
        }

        m_oclQueueMap.clear();
    }

    if (!m_hsaQueueMap.isEmpty())
    {
        timelineDataLoaded = true;

        for (QMap<QString, acTimelineBranch*>::const_iterator i = m_hsaQueueMap.begin(); i != m_hsaQueueMap.end(); ++i)
        {
            m_pHSABranch->addSubBranch(*i);
        }

        m_pTimeline->addBranch(m_pHSABranch);
        m_hsaQueueMap.clear();
    }

    if (!m_modelMap.isEmpty())
    {
        traceDataLoaded = true;

        for (QMap<osThreadId, TraceTableModel*>::const_iterator i = m_modelMap.begin(); i != m_modelMap.end(); ++i)
        {
            AddTraceTable(*i, i.key());
        }

        m_modelMap.clear();
    }

    if (!timelineDataLoaded)
    {
        if (!traceDataLoaded)
        {
            SAFE_DELETE(m_pSessionTabWidget);
            //by deleting m_pSessionTabWidget dependant widgets are erased as well, thus we zero them
            m_pMainSplitter = nullptr;
            m_pTraceTabView = nullptr;
            m_pTimeline = nullptr;
            QString strError = "An error occurred when loading the Application Trace.";
            QStringList excludedAPIs;

            if (m_pCurrentSession->GetExcludedAPIs(excludedAPIs))
            {
                strError += QString(GP_Str_AppMadeNoCallsToEnabledAPI).arg(GP_Str_AppTraceAPIToTrace).arg(Util::ms_APP_TRACE_OPTIONS_PAGE);
            }

            QLabel* badDataLabel = new QLabel(strError);
            GT_ASSERT(badDataLabel);
            badDataLabel->setAlignment(Qt::AlignCenter);
            m_pMainLayout->addWidget(badDataLabel);
            m_pMainLayout->setContentsMargins(0, 0, 0, 0);
        }
        else
        {
            Util::ShowErrorBox(QString(GP_Str_UnableToLoadTimelineData).arg(Util::ms_ENABLE_TIMEOUT_OPTION));
        }
    }
}

int TraceView::GetTabIndex(const QString& strTabText)
{
    int retVal = -1;

    for (int tabIndex = 0; tabIndex < m_pTraceTabView->count(); tabIndex++)
    {
        if (m_pTraceTabView->tabText(tabIndex) == strTabText)
        {
            retVal = tabIndex;
            break;
        }
    }

    return retVal;
}


void TraceView::DisplaySummaryPageType(int selectedIndex)
{
    GT_IF_WITH_ASSERT((m_pSummaryView != nullptr) && (m_pTraceTabView != nullptr) && (m_pSessionTabWidget != nullptr))
    {
        m_pSummaryView->DisplaySummaryPageType(selectedIndex);
        int indexOfSummary = m_pTraceTabView->indexOf(m_pSummaryView);
        m_pTraceTabView->setCurrentIndex(indexOfSummary);

        // Set the summarizer as current:
        m_pSessionTabWidget->setCurrentIndex(0);
    }
}

void TraceView::UpdateRenamedSession(const osFilePath& oldSessionFileName, const osFilePath& newSessionFileName)
{
    // Call the base class implementation
    SharedSessionWindow::UpdateRenamedSession(oldSessionFileName, newSessionFileName);
}

void TraceView::OnEditCopy()
{
    GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
    {
        QWidget* pCurrent = m_pTraceTabView->currentWidget();
        SummaryView* pSummaryView = qobject_cast<SummaryView*>(pCurrent);

        if (pSummaryView != nullptr)
        {
            pSummaryView->OnEditCopy();
        }
        else
        {
            TraceTable* pTraceTable = qobject_cast<TraceTable*>(pCurrent);

            if (pTraceTable != nullptr)
            {
                pTraceTable->OnEditCopy();
            }
        }
    }
}

void TraceView::OnEditSelectAll()
{
    GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
    {
        QWidget* pCurrent = m_pTraceTabView->currentWidget();
        SummaryView* pSummaryView = qobject_cast<SummaryView*>(pCurrent);

        if (pSummaryView != nullptr)
        {
            pSummaryView->OnEditSelectAll();
        }
        else
        {
            TraceTable* pTraceTable = qobject_cast<TraceTable*>(pCurrent);

            if (pTraceTable != nullptr)
            {
                pTraceTable->OnEditSelectAll();
            }
        }
    }
}

void TraceView::DisplayItemInPropertiesView(acTimelineItem* pItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        acTimelineItemToolTip itemTooltip;
        pItem->tooltipItems(itemTooltip);

        // Create an HTML content item:
        afHTMLContent htmlContent(acQStringToGTString(pItem->text()));

        for (int i = 0 ; i < itemTooltip.count(); i++)
        {
            // Get the current name and value:
            QString name = itemTooltip.getName(i);
            QString val = itemTooltip.getValue(i);

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, acQStringToGTString(name), acQStringToGTString(val));
        }

        gtString htmlText;
        htmlContent.toString(htmlText);

        afApplicationCommands::instance()->propertiesView()->setHTMLText(acGTStringToQString(htmlText), nullptr);
    }
}

void TraceView::OnCollapseAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
    {
        TraceTable* pTraceTable = qobject_cast<TraceTable*>(m_pTraceTabView->currentWidget());

        if (pTraceTable != nullptr)
        {
            pTraceTable->collapseAll();
        }
    }
}

void TraceView::OnExpandAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
    {
        TraceTable* pTraceTable = qobject_cast<TraceTable*>(m_pTraceTabView->currentWidget());

        if (pTraceTable != nullptr)
        {
            pTraceTable->expandAll();
        }
    }
}

void TraceView::OnZoomItemInTimeline()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
    {
        TraceTable* pTraceTable = qobject_cast<TraceTable*>(m_pTraceTabView->currentWidget());

        if ((pTraceTable != nullptr) && (m_pTimeline != nullptr))
        {
            GT_IF_WITH_ASSERT(!pTraceTable->selectionModel()->selectedIndexes().isEmpty())
            {
                QModelIndex firstSelected = pTraceTable->selectionModel()->selectedIndexes().first();
                TraceTableItem* pSelectedItem = static_cast<TraceTableItem*>(firstSelected.internalPointer());
                GT_IF_WITH_ASSERT(pSelectedItem != nullptr)
                {
                    m_pTimeline->ZoomToItem(pSelectedItem->GetTimelineItem(), true);
                }
            }
        }
    }
}

void TraceView::OnExportToCSV()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTraceTabView != nullptr)
    {
        TraceTable* pTraceTable = qobject_cast<TraceTable*>(m_pTraceTabView->currentWidget());

        if (pTraceTable != nullptr)
        {
            // The file path for the saved CSV file:
            QString csvFilePathStr;

            // Build the CSV default file name:
            QString fileName = QString(GPU_CSV_FileNameFormat).arg(m_pCurrentSession->m_displayName).arg(GPU_CSV_FileNameTraceView);
            bool rc = afApplicationCommands::instance()->ShowQTSaveCSVFileDialog(csvFilePathStr, fileName, this);
            GT_IF_WITH_ASSERT(rc)
            {
                // Export the web view table to a CSV file:
                rc = pTraceTable->ExportToCSV(csvFilePathStr);
                GT_ASSERT(rc);
            }
        }
    }
}

TraceView::OCLQueueBranchInfo* TraceView::GetBranchInfo(unsigned int contextId, unsigned int queueId, const QString& strContextHandle, const QString& deviceNameStr, const QString& strQueueHandle)
{
    OCLQueueBranchInfo* pRetVal = nullptr;
    acTimelineBranch* pContextBranch = NULL;

    // Create OpenCL branch if it does not yet exist
    if (m_pOpenCLBranch == nullptr)
    {
        m_pOpenCLBranch = new acTimelineBranch();

        m_pOpenCLBranch->setText(GPU_STR_TraceViewOpenCL);
    }

    if (m_oclCtxMap.contains(contextId))
    {
        pContextBranch = m_oclCtxMap[contextId];
    }
    else
    {
        pContextBranch = new acTimelineBranch();
        pContextBranch->SetBGColor(QColor::fromRgb(230, 230, 230));
        pContextBranch->setText(QString(tr("Context %1 (%2)")).arg(contextId).arg(strContextHandle));
        m_oclCtxMap[contextId] = pContextBranch;
    }

    if (m_oclQueueMap.contains(queueId))
    {
        pRetVal = m_oclQueueMap[queueId];
    }
    else
    {
        pRetVal = new OCLQueueBranchInfo;


        // Create the queue branch
        pRetVal->m_pQueueBranch = new acTimelineBranch();
        pRetVal->m_pQueueBranch->setText(QString(tr(GPU_STR_TraceViewQueueRow)).arg(queueId).arg(deviceNameStr).arg(strQueueHandle));
        pRetVal->m_pQueueBranch->SetBGColor(QColor::fromRgb(230, 230, 230));

        pRetVal->m_pKernelBranch = new acTimelineBranch();
        pRetVal->m_pKernelBranch->setText(tr("Kernel Execution"));
        pRetVal->m_pKernelBranch->SetBGColor(QColor::fromRgb(230, 230, 230));

        pRetVal->m_pMemoryBranch = new acTimelineBranch();
        pRetVal->m_pMemoryBranch->setText(tr("Data Transfer"));
        pRetVal->m_pMemoryBranch->SetBGColor(QColor::fromRgb(230, 230, 230));

        // Notice:
        // we do not initialize the fill operations branch by default. We only do it when there are fill operation functions

        pRetVal->m_pQueueBranch->addSubBranch(pRetVal->m_pMemoryBranch);
        pRetVal->m_pQueueBranch->addSubBranch(pRetVal->m_pKernelBranch);

        pContextBranch->addSubBranch(pRetVal->m_pQueueBranch);

        m_oclQueueMap[queueId] = pRetVal;
    }

    return pRetVal;
}

void TraceView::BuildWindowLayout()
{
    m_pMainSplitter = new QSplitter(this);

    m_pTimeline = new acTimeline(this);

    m_pTraceTabView = new QTabWidget(this);

    //    m_findToolBar = new FindToolBarView(this);
    //
    m_pTraceTableContextMenu = new QMenu(this);

    m_pSessionTabWidget->setTabsClosable(true);
    m_pTraceTabView->setTabsClosable(false);
    m_pMainSplitter->setOrientation(Qt::Vertical);
    m_pMainSplitter->addWidget(m_pTimeline);
    m_pMainSplitter->addWidget(m_pTraceTabView);
    //    addWidget(m_findToolBar);

    // set the initial sizes for the timeline, trace/summary tabs, and find toolbar
    QList<int> sizeList;
    sizeList.append(100); // timeline
    sizeList.append(100); // trace/summary tabs
    //    sizeList.append(10);  // find toolbar
    m_pMainSplitter->setSizes(sizeList);

    m_pMainLayout = new QHBoxLayout(this);
    m_pSessionTabWidget->addTab(m_pMainSplitter, "Application Timeline Trace");
    m_pMainLayout->addWidget(m_pSessionTabWidget);
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pMainLayout);
}

