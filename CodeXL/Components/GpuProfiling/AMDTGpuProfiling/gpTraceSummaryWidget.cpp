//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryWidget.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

const int DEFAULT_PROGRESS_FACTOR = 10000;
static const char* tab_captions[] = { GPU_STR_API_Summary, GPU_STR_GPU_Summary, GPU_STR_Command_List_Summary };

/////////////////////////////////////////////////////////////////////////////////////
// gpTraceSummaryWidget

gpTraceSummaryWidget::gpTraceSummaryWidget(QWidget* pParent)
    : acTabWidget(pParent), m_pTraceView(nullptr), m_useTimelineSelectionScope(false), m_timelineAbsoluteStart(0), m_timelineStart(0), m_timelineEnd(0)
{
    for (int nTab = 0; nTab < eCallType::MAX_TYPE; nTab++)
    {
        m_tabs[nTab] = nullptr;
    }
}

gpTraceSummaryWidget::~gpTraceSummaryWidget()
{
}

void gpTraceSummaryWidget::Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStartTime, quint64 timelineRange)
{
    m_pTraceView = pSessionView;

    int numItems = pDataContainer->GPUItemsCount() + pDataContainer->ThreadsCount();

    afProgressBarWrapper::instance().ShowProgressDialog(L"Loading Summary", numItems * DEFAULT_PROGRESS_FACTOR);
    afProgressBarWrapper::instance().incrementProgressBar();

    m_timelineAbsoluteStart = timelineStartTime;
    m_timelineStart = timelineStartTime;
    m_timelineEnd = m_timelineStart + timelineRange;

    for (int i = 0; i < eCallType::MAX_TYPE; i++)
    {
        if (i == eCallType::API_CALL || i == eCallType::GPU_CALL)
        {
            m_tabs[i] = new gpTraceSummaryTab((eCallType)i);
        }
        else
        {
            m_tabs[i] = new gpCommandListSummaryTab((eCallType)i);
        }

        bool rc = m_tabs[i]->Init(pDataContainer, pSessionView, timelineStartTime, timelineRange);
        GT_ASSERT(rc);
        addTab(m_tabs[i], QIcon(), tab_captions[i]);

        rc = connect(this, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));
        GT_ASSERT(rc);
        rc = connect(m_tabs[i], SIGNAL(TabUseTimelineSelectionScopeChanged(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
        GT_ASSERT(rc);
        rc = connect(m_tabs[i], SIGNAL(TabSummaryItemClicked(ProfileSessionDataItem*)), this, SLOT(OnTabSummaryItemClicked(ProfileSessionDataItem*)));
        GT_ASSERT(rc);


        afProgressBarWrapper::instance().incrementProgressBar();
    }

    afProgressBarWrapper::instance().hideProgressBar();

    setCurrentIndex(GPU_CALL);
}

void gpTraceSummaryWidget::SelectAPIRowByCallName(const QString& callName)
{
    GT_IF_WITH_ASSERT(m_tabs[API_CALL] != nullptr)
    {
        gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_tabs[API_CALL]->m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);
        pSummaryTable->SelectRowByCallName(callName);
    }
}

void gpTraceSummaryWidget::SelectGPURowByCallName(const QString& callName)
{
    GT_IF_WITH_ASSERT(m_tabs[GPU_CALL] != nullptr)
    {
        gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_tabs[GPU_CALL]->m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);
        pSummaryTable->SelectRowByCallName(callName);
    }
}

void gpTraceSummaryWidget::ClearAPISelection()
{
    GT_IF_WITH_ASSERT(m_tabs[API_CALL] != nullptr)
    {
        m_tabs[API_CALL]->m_pSummaryTable->clearSelection();
    }
}

void gpTraceSummaryWidget::ClearGPUSelection()
{
    GT_IF_WITH_ASSERT(m_tabs[GPU_CALL] != nullptr)
    {
        m_tabs[GPU_CALL]->m_pSummaryTable->clearSelection();
    }
}

void gpTraceSummaryWidget::OnTimelineChanged(const QPointF& rangePoint, bool isRelativeRangeStartTime)
{
    int activeTabIndex = currentIndex();

    if (isRelativeRangeStartTime)
    {
        m_timelineStart = m_timelineAbsoluteStart + rangePoint.x() ;
        m_timelineEnd = m_timelineAbsoluteStart + rangePoint.y();
    }
    else
    {
        m_timelineStart = rangePoint.x();
        m_timelineEnd = m_timelineStart + rangePoint.y();
    }

    for (auto tab : m_tabs)
    {
        if (tab != nullptr)
        {
            tab->OnTimelineChanged(m_timelineStart, m_timelineEnd);
        }
    }

    if (m_useTimelineSelectionScope == true)
    {
        GT_IF_WITH_ASSERT(m_tabs[activeTabIndex] != nullptr && m_tabs[activeTabIndex])
        {
            m_tabs[activeTabIndex]->RefreshAndMaintainSelection(true);
        }
    }
}
void gpTraceSummaryWidget::OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap)
{
    GT_UNREFERENCED_PARAMETER(threadNameVisibilityMap);
    int activeTabIndex = currentIndex();
    GT_IF_WITH_ASSERT(m_tabs[activeTabIndex] != nullptr && m_tabs[activeTabIndex])
    {
        m_tabs[activeTabIndex]->OnTimelineFilterChanged();
    }
}
void gpTraceSummaryWidget::OnUseTimelineSelectionScopeChanged(bool check)
{
    m_useTimelineSelectionScope = check;
}

void gpTraceSummaryWidget::OnFind()
{
    // Get the current tab and find the requested text in it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnFind();
    }
}

void gpTraceSummaryWidget::OnEditSelectAll()
{
    // Get the current tab and apply the select all command on it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnEditSelectAll();
    }
}

void gpTraceSummaryWidget::OnEditCopy()
{
    // Get the current tab and apply the copy command on it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnEditCopy();
    }
}

void gpTraceSummaryWidget::OnCurrentChanged(int activeTabIndex)
{
    GT_IF_WITH_ASSERT(m_tabs[activeTabIndex] != nullptr && m_tabs[activeTabIndex])
    {
        m_tabs[activeTabIndex]->SetTimelineScope(m_useTimelineSelectionScope, m_timelineStart, m_timelineEnd);
        m_tabs[activeTabIndex]->RefreshAndMaintainSelection(m_useTimelineSelectionScope);
    }
}

void gpTraceSummaryWidget::OnTabSummaryItemClicked(ProfileSessionDataItem* pItem)
{
    emit SummaryItemClicked(pItem);
}
void gpTraceSummaryWidget::SelectCommandList(const QString& commandListName)
{
    gpCommandListSummaryTab* pCurrentTab = qobject_cast<gpCommandListSummaryTab*>(currentWidget());
    if (pCurrentTab != nullptr)
    {
        pCurrentTab->SelectCommandList(commandListName);
    }
}
=======
//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryWidget.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/gpTraceSummaryTable.h>

#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

#include <AMDTApplicationComponents/Include/acColours.h>

static const char* tab_captions[]        = { GPU_STR_API_Summary, GPU_STR_GPU_Summary };
static const char* table_captions[]      = { GPU_STR_API_Call_Summary, GPU_STR_GPU_Call_Summary };

const int DEFAULT_PROGRESS_FACTOR = 10000;
const int MAX_ITEMS_IN_TABLE = 20;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
gpSummaryTab::gpSummaryTab(eCallType callType)
    : m_callType(callType), m_pTraceView(nullptr), m_pSummaryTable(nullptr), m_pTop5Table(nullptr), m_pTop5Caption(nullptr), m_pChkboxUseScope(nullptr), m_useTimelineSelectionScope(false), m_timelineStart(0), m_timelineEnd(0), m_currentCallIndex(0)
{
}

// gpSummaryTab
bool gpSummaryTab::Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStart, quint64 timelineEnd)
{
    m_pTraceView = pSessionView;
    m_pSummaryTable = new gpTraceSummaryTable(pDataContainer, pSessionView, m_callType);
    m_pTop5Table = new acListCtrl(this);

    m_pTop5Caption = new QLabel(this);

    QHBoxLayout* pHLayout = new QHBoxLayout;
    QVBoxLayout* pLeftVLayout = new QVBoxLayout;
    QHBoxLayout* pLeftHLayout = new QHBoxLayout;
    QVBoxLayout* pRightVLayout = new QVBoxLayout;
    QSplitter* pVMainSplitter = new QSplitter(Qt::Horizontal);

    // set the margins to all layout to zero
    pHLayout->setContentsMargins(0, 0, 0, 0);
    pLeftVLayout->setContentsMargins(0, 0, 0, 0);
    pLeftHLayout->setContentsMargins(0, 0, 0, 0);
    pRightVLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* pWidgetLeft = new QWidget;
    QWidget* pWidgetRight = new QWidget;

    QLabel* pLblCaption = new QLabel(QString(table_captions[m_callType]));
    m_pChkboxUseScope = new QCheckBox(GPU_STR_Use_Scope_Summary);

    pLeftHLayout->addWidget(pLblCaption, 0, Qt::AlignTop);

    bool rc = connect(m_pChkboxUseScope, SIGNAL(toggled(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
    GT_ASSERT(rc);
    rc = connect(m_pSummaryTable, SIGNAL(itemSelectionChanged()), this, SLOT(OnSummaryTableSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pSummaryTable, SIGNAL(cellClicked(int, int)), this, SLOT(OnSummaryTableCellClicked(int, int)));
    GT_ASSERT(rc);
    rc = connect(m_pTop5Table, SIGNAL(cellClicked(int, int)), this, SLOT(OnTop5TableCellClicked(int, int)));
    GT_ASSERT(rc);
    rc = connect(m_pTop5Table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnTop5TableCellDoubleClicked(int, int)));
    GT_ASSERT(rc);

    QString styleSheetStr = QString("QDialog{border:1px solid gray;}");
    pLeftHLayout->addWidget(m_pChkboxUseScope, 0, Qt::AlignHCenter);
    pLeftVLayout->addLayout(pLeftHLayout);
    pWidgetLeft->setLayout(pLeftVLayout);
    pWidgetLeft->setStyleSheet(styleSheetStr);

    m_pSummaryTable->resizeColumnsToContents();
    pLeftVLayout->addWidget(m_pSummaryTable);
    m_pSummaryTable->SetSelectionBackgroundColor(acQAMD_CYAN_SELECTION_BKG_COLOUR);

    pLeftVLayout->setSpacing(0);
    pLeftVLayout->setMargin(1);

    QFont qFont = m_pTop5Caption->font();
    qFont.setBold(true);
    qFont.setPointSize(10);
    m_pTop5Caption->setFont(qFont);

    pRightVLayout->addWidget(m_pTop5Caption);
    QStringList columnCaptions;
    columnCaptions << "#";

    if (m_callType == API_CALL)
    {
        columnCaptions << GP_STR_SummaryTop5TableColumnThreadId;
    }

    columnCaptions << GP_STR_SummaryTop5TableColumnCallIndex;
    columnCaptions << GP_STR_SummaryTop5TableColumnTime;
    m_pTop5Table->initHeaders(columnCaptions, false);
    m_pTop5Table->setContextMenuPolicy(Qt::NoContextMenu);
    m_pTop5Table->resizeColumnsToContents();
    m_pTop5Table->setShowGrid(true);
    pRightVLayout->addWidget(m_pTop5Table);
    pRightVLayout->setSpacing(1);
    pRightVLayout->setMargin(1);
    pWidgetRight->setLayout(pRightVLayout);
    pWidgetRight->setStyleSheet(styleSheetStr);


    pVMainSplitter->addWidget(pWidgetLeft);
    pVMainSplitter->addWidget(pWidgetRight);

    int widgetWidth = geometry().right() - geometry().left();
    QList<int> sizes;
    sizes << (int)(widgetWidth * 0.75);
    sizes << (int)(widgetWidth * 0.25);
    pVMainSplitter->setSizes(sizes);
    pHLayout->addWidget(pVMainSplitter);

    setLayout(pHLayout);

    SetTimelineScope(false, timelineStart, timelineEnd);

    m_pSummaryTable->selectRow(0);

    return true;
}

// Called when the user changes line selection -> top5 table is updated with the top 5 calls
// If the min \ max column is selected -> zoom to item in timeline
void gpSummaryTab::OnSummaryTableSelectionChanged()
{
    GT_IF_WITH_ASSERT((m_pTraceView != nullptr) && (m_pSummaryTable != nullptr))
    {
        ProfileSessionDataItem* pItem = nullptr;

        // Get first of all selections
        QModelIndexList indices = m_pSummaryTable->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex index = indices.at(0);
            QString callName;
            CallIndexId apiCall;
            int rowIndex = index.row();

            if (m_pSummaryTable->GetItemCallIndex(rowIndex, apiCall, callName))
            {
                FillTop5Table(apiCall, callName);
                pItem = m_pSummaryTable->GetRelatedItem(rowIndex, index.column());
                SyncSelectionInOtherControls(pItem);
            }

            m_pSummaryTable->SaveSelection(rowIndex);
        }
    }
}

void gpSummaryTab::FillTop5Table(CallIndexId apiCall, const QString& callName, bool showAll)
{
    GT_IF_WITH_ASSERT(m_pTop5Table != nullptr && m_pSummaryTable != nullptr)
    {
        m_currentCallName = callName;
        m_currentCallIndex = apiCall;

        m_pTop5Table->clearList();
        m_top5ItemList.empty();

        // add items to m_pTop5TableItems for double-click
        const QMap<CallIndexId, ProfileSessionDataItem*>& map = m_pSummaryTable->GetAllApiCallItemsMap();
        m_top5ItemList = map.values(apiCall);
        std::sort(m_top5ItemList.begin(), m_top5ItemList.end(), [](ProfileSessionDataItem * a, ProfileSessionDataItem * b)
        {
            return (a->EndTime() - a->StartTime()) > (b->EndTime() - b->StartTime());
        });
        int numItems = m_top5ItemList.count();

        if (numItems < MAX_ITEMS_IN_TABLE)
        {
            m_pTop5Caption->setText(QString(GPU_STR_Top_Calls_Summary).arg(callName));
        }
        else
        {
            m_pTop5Caption->setText(QString(GPU_STR_Top_5_Summary).arg(MAX_ITEMS_IN_TABLE).arg(callName));
        }

        for (int iVal = 0; (((showAll == false && m_pTop5Table->rowCount() < MAX_ITEMS_IN_TABLE && iVal < numItems)) || (showAll == true && iVal < numItems)); iVal++)
        {
            ProfileSessionDataItem* pItem = m_top5ItemList.at(iVal);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                quint64 startTime = pItem->StartTime();
                quint64 endTime = pItem->EndTime() ;

                if (m_useTimelineSelectionScope == false || (m_timelineStart <= startTime && endTime <= m_timelineEnd))
                {
                    quint64 duration = endTime - startTime;
                    QString durationStr = NanosecToTimeStringFormatted(duration, true);
                    QStringList rowStrings;
                    rowStrings << QString::number(iVal + 1);

                    if (m_callType == API_CALL)
                    {
                        rowStrings << QString::number(pItem->ThreadID());
                    }

                    rowStrings << QString::number(pItem->APICallIndex());
                    rowStrings << durationStr;
                    bool retVal = m_pTop5Table->addRow(rowStrings, nullptr, Qt::AlignVCenter | Qt::AlignLeft);
                    GT_ASSERT(retVal);
                    // Get the duration item to set its data (so the table will be sorted)
                    int lastRowIndex = m_pTop5Table->rowCount() - 1;
                    QTableWidgetItem* pDurationItem = nullptr;

                    if (m_callType == API_CALL)
                    {
                        pDurationItem = m_pTop5Table->item(lastRowIndex, 3);
                    }
                    else
                    {
                        pDurationItem = m_pTop5Table->item(lastRowIndex, 2);
                    }

                    GT_IF_WITH_ASSERT(pDurationItem != nullptr)
                    {
                        pDurationItem->setData(Qt::EditRole, duration);
                        pDurationItem->setData(Qt::DisplayRole, durationStr);

                        // Get the item brush to color in blue:
                        QBrush brush = pDurationItem->foreground();
                        brush.setColor(Qt::blue);
                        pDurationItem->setForeground(brush);

                        QFont font = pDurationItem->font();
                        font.setUnderline(true);
                        pDurationItem->setFont(font);


                    }
                }
            }
        }

        if (showAll == false && MAX_ITEMS_IN_TABLE < numItems)
        {
            m_pTop5Table->addRow(QString("      " GP_STR_SummaryTop5TableShowAll));
            int rowCount = m_pTop5Table->rowCount();
            m_pTop5Table->setSpan(rowCount - 1, 0, 1, 3);
        }
    }
}

// Called when the user changes cell selection in the selected row
// If the min \ max coloumn is selected -> zoom to item in timeline
void gpSummaryTab::OnSummaryTableCellClicked(int row, int col)
{
    GT_IF_WITH_ASSERT(m_pTraceView != nullptr && m_pSummaryTable != nullptr)
    {
        ProfileSessionDataItem* pItem = m_pSummaryTable->GetRelatedItem(row, col);
        SyncSelectionInOtherControls(pItem);
    }
}



void gpSummaryTab::OnTimelineChanged(quint64 startTime, quint64 timelineEnd)
{
    m_timelineStart = startTime;
    m_timelineEnd = timelineEnd;
}

void gpSummaryTab::OnTimelineFilterChanged()
{
    GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
    {
        m_pSummaryTable->Refresh(m_useTimelineSelectionScope, m_timelineStart, m_timelineEnd);
    }
}

void gpSummaryTab::OnFind()
{
    // Check if the table or the top 20 table is in focus
    QWidget* pFocusedWidget = focusWidget();

    if (acIsChildOf(pFocusedWidget, m_pSummaryTable))
    {
        GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
        {
            m_pSummaryTable->onFindClick();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pTop5Table))
    {
        GT_IF_WITH_ASSERT(m_pTop5Table != nullptr)
        {
            m_pTop5Table->onFindClick();
        }
    }
}

void gpSummaryTab::OnEditSelectAll()
{
    // Check if the table or the top 20 table is in focus
    QWidget* pFocusedWidget = focusWidget();

    if (acIsChildOf(pFocusedWidget, m_pSummaryTable))
    {
        GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
        {
            m_pSummaryTable->onEditSelectAll();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pTop5Table))
    {
        GT_IF_WITH_ASSERT(m_pTop5Table != nullptr)
        {
            m_pTop5Table->onEditSelectAll();
        }
    }
}

void gpSummaryTab::OnEditCopy()
{
    // Check if the table or the top 20 table is in focus
    QWidget* pFocusedWidget = focusWidget();

    if (acIsChildOf(pFocusedWidget, m_pSummaryTable))
    {
        GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
        {
            m_pSummaryTable->onEditCopy();
        }
    }
    else if (acIsChildOf(pFocusedWidget, m_pTop5Table))
    {
        GT_IF_WITH_ASSERT(m_pTop5Table != nullptr)
        {
            m_pTop5Table->onEditCopy();
        }
    }
}

bool gpSummaryTab::GetSelectedCallIndex(CallIndexId& apiCall, QString& callName)
{
    GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
    {
        QModelIndexList indices = m_pSummaryTable->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex index = indices.at(0);
            return m_pSummaryTable->GetItemCallIndex(index.row(), apiCall, callName);
        }
    }
    return false;
}

void gpSummaryTab::OnUseTimelineSelectionScopeChanged(bool check)
{
    m_useTimelineSelectionScope = check;

    GT_IF_WITH_ASSERT(m_pTop5Table != nullptr && m_pSummaryTable != nullptr)
    {
        RefreshAndMaintainSelection(check);
    }
    emit TabUseTimelineSelectionScopeChanged(check);
}

void gpSummaryTab::RefreshAndMaintainSelection(bool check)
{
    // save current selection in summary table to restore after m_pSummaryTable is refreshed
    int selectionIndex = -1;

    CallIndexId selectedInterface = 0;
    QString selectedApiCall;

    QItemSelectionModel* pSelection = m_pSummaryTable->selectionModel();

    if (pSelection != nullptr && pSelection->hasSelection())
    {
        QModelIndexList selectedRows = pSelection->selectedRows();
        selectionIndex = selectedRows[0].row();
        m_pSummaryTable->GetItemCallIndex(selectionIndex, selectedInterface, selectedApiCall);
    }

    m_pSummaryTable->Refresh(check, m_timelineStart, m_timelineEnd);
    int rowCount = m_pSummaryTable->rowCount();

    for (int i = 0; i < rowCount; i++)
    {
        CallIndexId rowInterface;
        QString rowName;
        m_pSummaryTable->GetItemCallIndex(i, rowInterface, rowName);

        if (rowName == selectedApiCall && rowInterface == selectedInterface)
        {
            selectionIndex = i;
            break;
        }
    }

    if (selectionIndex > -1 && selectionIndex < m_pSummaryTable->rowCount())
    {
        m_pSummaryTable->selectRow(selectionIndex);

        QModelIndex modelIdx = m_pSummaryTable->model()->index(selectionIndex, 0);
        m_pSummaryTable->scrollTo(modelIdx);
    }
    else
    {
        m_pTop5Table->clearList();
    }
}

// check if <show more...> button was pressed
void gpSummaryTab::OnTop5TableCellClicked(int row, int col)
{
    GT_IF_WITH_ASSERT(m_pTop5Table != nullptr)
    {
        QModelIndexList indices = m_pTop5Table->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex index = indices.at(0);
            QTableWidgetItem* pItemCall = m_pTop5Table->item(row, col);

            if (pItemCall != nullptr && pItemCall->text().contains(GP_STR_SummaryTop5TableShowAll))
            {
                FillTop5Table(m_currentCallIndex, m_currentCallName, true);
            }
            else if ((m_callType == API_CALL && col == 3) || (m_callType == GPU_CALL && col == 2))
            {
                ProfileSessionDataItem* pItem = m_top5ItemList.at(index.row());
                SyncSelectionInOtherControls(pItem);
            }

        }
    }
}

// select item in timeline
void gpSummaryTab::OnTop5TableCellDoubleClicked(int row, int col)
{
    GT_IF_WITH_ASSERT(m_pTop5Table != nullptr)
    {
        QModelIndexList indices = m_pTop5Table->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex indexInTop5Table = indices.at(0);
            QTableWidgetItem* pItemCall = m_pTop5Table->item(row, col);

            if (pItemCall != nullptr)
            {
                // get the selected call from the summary table
                QModelIndexList indicesSummary = m_pSummaryTable->selectionModel()->selection().indexes();

                if (indicesSummary.size() > 0)
                {
                    QModelIndex indexSummary = indicesSummary.at(0);
                    QString callName;
                    CallIndexId apiCall;
                    int selectedRowInSummaryTableIndex = indexSummary.row();

                    if (m_pSummaryTable->GetItemCallIndex(selectedRowInSummaryTableIndex, apiCall, callName))
                    {
                        ProfileSessionDataItem* pItem = m_top5ItemList.at(indexInTop5Table.row());
                        SyncSelectionInOtherControls(pItem);
                    }
                }
            }
        }
    }
}

void gpSummaryTab::SyncSelectionInOtherControls(ProfileSessionDataItem* pItem)
{
    APICallId apiId;

    if (pItem != nullptr && m_pTraceView != nullptr && m_pSummaryTable != nullptr)
    {
        emit TabSummaryItemClicked(pItem);
        m_pSummaryTable->setFocus();
    }
}

void gpSummaryTab::SetTimelineScope(bool useTimelineSelectionScope, quint64 timelineStart, quint64 timelineRange)
{
    GT_IF_WITH_ASSERT(m_pChkboxUseScope != nullptr)
    {
        m_timelineStart = timelineStart;
        m_timelineEnd = timelineStart + timelineRange;

        if (m_pChkboxUseScope->isChecked() != useTimelineSelectionScope)
        {
            m_pChkboxUseScope->setChecked(useTimelineSelectionScope);
        }
    }
}

void gpSummaryTab::RestoreSelection()
{
    GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
    {
        m_pSummaryTable->RestoreSelection();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// gpTraceSummaryWidget

gpTraceSummaryWidget::gpTraceSummaryWidget(QWidget* pParent)
    : acTabWidget(pParent), m_pTraceView(nullptr), m_useTimelineSelectionScope(false), m_timelineAbsoluteStart(0), m_timelineStart(0), m_timelineEnd(0)
{
    for (int nTab = 0; nTab < eCallType::MAX_TYPE; nTab++)
    {
        m_tabs[nTab] = nullptr;
    }
}

gpTraceSummaryWidget::~gpTraceSummaryWidget()
{
}

void gpTraceSummaryWidget::Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStartTime, quint64 timelineRange)
{
    m_pTraceView = pSessionView;

    int numItems = pDataContainer->QueueItemsCount() + pDataContainer->ThreadsCount();

    afProgressBarWrapper::instance().ShowProgressDialog(L"Loading Summary", numItems * DEFAULT_PROGRESS_FACTOR);
    afProgressBarWrapper::instance().incrementProgressBar();

    m_timelineAbsoluteStart = timelineStartTime;
    m_timelineStart = timelineStartTime;
    m_timelineEnd = m_timelineStart + timelineRange;

    for (int i = 0; i < eCallType::MAX_TYPE; i++)
    {
        m_tabs[i] = new gpSummaryTab((eCallType)i);

        bool rc = m_tabs[i]->Init(pDataContainer, pSessionView, timelineStartTime, timelineRange);
        GT_ASSERT(rc);
        addTab(m_tabs[i], QIcon(), tab_captions[i]);

        rc = connect(this, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));
        GT_ASSERT(rc);
        rc = connect(m_tabs[i], SIGNAL(TabUseTimelineSelectionScopeChanged(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
        GT_ASSERT(rc);
        rc = connect(m_tabs[i], SIGNAL(TabSummaryItemClicked(ProfileSessionDataItem*)), this, SLOT(OnTabSummaryItemClicked(ProfileSessionDataItem*)));
        GT_ASSERT(rc);


        afProgressBarWrapper::instance().incrementProgressBar();
    }

    afProgressBarWrapper::instance().hideProgressBar();

    setCurrentIndex(GPU_CALL);
}

void gpTraceSummaryWidget::SelectAPIRowByCallName(const QString& callName)
{
    GT_IF_WITH_ASSERT(m_tabs[API_CALL] != nullptr)
    {
        m_tabs[API_CALL]->m_pSummaryTable->SelectRowByCallName(callName);
    }
}

void gpTraceSummaryWidget::SelectGPURowByCallName(const QString& callName)
{
    GT_IF_WITH_ASSERT(m_tabs[GPU_CALL] != nullptr)
    {
        m_tabs[GPU_CALL]->m_pSummaryTable->SelectRowByCallName(callName);
    }
}

void gpTraceSummaryWidget::ClearAPISelection()
{
    GT_IF_WITH_ASSERT(m_tabs[API_CALL] != nullptr)
    {
        m_tabs[API_CALL]->m_pSummaryTable->clearSelection();
    }
}

void gpTraceSummaryWidget::ClearGPUSelection()
{
    GT_IF_WITH_ASSERT(m_tabs[GPU_CALL] != nullptr)
    {
        m_tabs[GPU_CALL]->m_pSummaryTable->clearSelection();
    }
}

void gpTraceSummaryWidget::OnTimelineChanged(const QPointF& rangePoint, bool isRelativeRangeStartTime)
{
    int activeTabIndex = currentIndex();

    if (isRelativeRangeStartTime)
    {
        m_timelineStart = m_timelineAbsoluteStart + rangePoint.x() ;
        m_timelineEnd = m_timelineAbsoluteStart + rangePoint.y();
    }
    else
    {
        m_timelineStart = rangePoint.x();
        m_timelineEnd = m_timelineStart + rangePoint.y();
    }

    for (auto tab : m_tabs)
    {
        if (tab != nullptr)
        {
            tab->OnTimelineChanged(m_timelineStart, m_timelineEnd);
        }
    }

    if (m_useTimelineSelectionScope == true)
    {
        GT_IF_WITH_ASSERT(m_tabs[activeTabIndex] != nullptr && m_tabs[activeTabIndex])
        {
            m_tabs[activeTabIndex]->RefreshAndMaintainSelection(true);
        }
    }
}
void gpTraceSummaryWidget::OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap)
{
    GT_UNREFERENCED_PARAMETER(threadNameVisibilityMap);
    int activeTabIndex = currentIndex();
    GT_IF_WITH_ASSERT(m_tabs[activeTabIndex] != nullptr && m_tabs[activeTabIndex])
    {
        m_tabs[activeTabIndex]->OnTimelineFilterChanged();
    }
}
void gpTraceSummaryWidget::OnUseTimelineSelectionScopeChanged(bool check)
{
    m_useTimelineSelectionScope = check;
}

void gpTraceSummaryWidget::OnFind()
{
    // Get the current tab and find the requested text in it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnFind();
    }
}

void gpTraceSummaryWidget::OnEditSelectAll()
{
    // Get the current tab and apply the select all command on it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnEditSelectAll();
    }
}

void gpTraceSummaryWidget::OnEditCopy()
{
    // Get the current tab and apply the copy command on it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnEditCopy();
    }
}

void gpTraceSummaryWidget::OnCurrentChanged(int activeTabIndex)
{
    GT_IF_WITH_ASSERT(m_tabs[activeTabIndex] != nullptr && m_tabs[activeTabIndex])
    {
        m_tabs[activeTabIndex]->SetTimelineScope(m_useTimelineSelectionScope, m_timelineStart, m_timelineEnd);
        m_tabs[activeTabIndex]->RefreshAndMaintainSelection(m_useTimelineSelectionScope);
    }
}

void gpTraceSummaryWidget::OnTabSummaryItemClicked(ProfileSessionDataItem* pItem)
{
    emit SummaryItemClicked(pItem);
}
