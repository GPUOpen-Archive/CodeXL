//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryTab.h>
#include <AMDTGpuProfiling/gpTraceView.h>


#include <AMDTApplicationComponents/Include/acColours.h>

static const char* table_captions[] = { GPU_STR_API_Call_Summary, GPU_STR_GPU_Call_Summary, GPU_STR_Command_List_Call_Summary , GPU_STR_Command_Buffers_Call_Summary };

const int MAX_ITEMS_IN_TABLE = 20;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
gpSummaryTab::gpSummaryTab(eCallType callType)
    : m_callType(callType), m_pTraceView(nullptr), m_pSummaryTable(nullptr), m_pTop20Table(nullptr), m_pTop20Caption(nullptr), m_pChkboxUseScope(nullptr), m_useTimelineSelectionScope(false), m_timelineStart(0), m_timelineEnd(0), m_currentCallIndex(0)
{
}

// gpSummaryTab
bool gpSummaryTab::Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStart, quint64 timelineEnd)
{
    m_pTraceView = pSessionView;
    m_pSessionDataContainer = pDataContainer;
    if (m_callType == API_CALL || m_callType == GPU_CALL)
    {
        m_pSummaryTable = new gpTraceSummaryTable(pDataContainer, pSessionView, m_callType);
    }
    else
    {
        m_pSummaryTable = new gpCommandListSummaryTable(pDataContainer, pSessionView);
    }

    m_pTop20Table = new acListCtrl(this);

    m_pTop20Caption = new QLabel(this);

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

    QString caption;
    if (m_callType == eCallType::API_CALL || m_callType == eCallType::GPU_CALL)
    {
        caption = table_captions[m_callType];
    }
    else
    { 
        int commandCount = m_pSessionDataContainer->CommandListsData().size();
        if (pDataContainer->SessionAPIType() == ProfileSessionDataItem::ProfileItemAPIType::DX12_API_PROFILE_ITEM)
        {
            caption = QString(table_captions[m_callType]).arg(commandCount);
        }
        else
        {
            caption = QString(table_captions[m_callType + 1]).arg(commandCount);
        }
    }
    
    QLabel* pLblCaption = new QLabel(caption);
    m_pChkboxUseScope = new QCheckBox(GPU_STR_Use_Scope_Summary);

    pLeftHLayout->addWidget(pLblCaption, 0, Qt::AlignTop);


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

    QFont qFont = m_pTop20Caption->font();
    qFont.setBold(true);
    qFont.setPointSize(10);
    m_pTop20Caption->setFont(qFont);

    pRightVLayout->addWidget(m_pTop20Caption);
    QStringList columnCaptions;
    columnCaptions << "#";

    if (m_callType == API_CALL)
    {
        columnCaptions << GP_STR_SummaryTop20TableColumnThreadId;
    }

    columnCaptions << GP_STR_SummaryTop20TableColumnCallIndex;
    columnCaptions << GP_STR_SummaryTop20TableColumnTime;
    m_pTop20Table->initHeaders(columnCaptions, false);
    m_pTop20Table->setContextMenuPolicy(Qt::NoContextMenu);
    m_pTop20Table->resizeColumnsToContents();
    m_pTop20Table->setShowGrid(true);
    pRightVLayout->addWidget(m_pTop20Table);
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

    bool rc = connect(m_pChkboxUseScope, SIGNAL(toggled(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
    GT_ASSERT(rc);
    rc = connect(m_pSummaryTable, SIGNAL(itemSelectionChanged()), this, SLOT(OnSummaryTableSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pSummaryTable, SIGNAL(cellClicked(int, int)), this, SLOT(OnSummaryTableCellClicked(int, int)));
    GT_ASSERT(rc);
    rc = connect(m_pSummaryTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnSummaryTableCellDoubleClicked(int, int)));
    GT_ASSERT(rc);
    rc = connect(m_pTop20Table, SIGNAL(cellClicked(int, int)), this, SLOT(OnTop20TableCellClicked(int, int)));
    GT_ASSERT(rc);
    rc = connect(m_pTop20Table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnTop20TableCellDoubleClicked(int, int)));
    GT_ASSERT(rc);

    m_pSummaryTable->selectRow(0);



    return true;
}

void gpSummaryTab::FillTop20Table(CallIndexId callIndex, const QString& callName, bool showAll)
{
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {


        GT_IF_WITH_ASSERT(m_pTop20Table != nullptr && m_pSummaryTable != nullptr)
        {
            m_currentCallName = callName;

            m_pTop20Table->clearList();
            m_top20ItemList.clear();

            FillTop20List(callIndex, callName);

            std::sort(m_top20ItemList.begin(), m_top20ItemList.end(), [](ProfileSessionDataItem * a, ProfileSessionDataItem * b)
            {
                return (a->EndTime() - a->StartTime()) > (b->EndTime() - b->StartTime());
            });

            SetTop20TableCaption(callName);

            int numItems = m_top20ItemList.count();
            for (int iVal = 0; (((showAll == false && m_pTop20Table->rowCount() < MAX_ITEMS_IN_TABLE && iVal < numItems)) || (showAll == true && iVal < numItems)); iVal++)
            {
                ProfileSessionDataItem* pItem = m_top20ItemList.at(iVal);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    quint64 startTime = pItem->StartTime();
                    quint64 endTime = pItem->EndTime();

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
                        bool retVal = m_pTop20Table->addRow(rowStrings, nullptr, Qt::AlignVCenter | Qt::AlignLeft);
                        GT_ASSERT(retVal);
                        // Get the duration item to set its data (so the table will be sorted)
                        int lastRowIndex = m_pTop20Table->rowCount() - 1;
                        QTableWidgetItem* pDurationItem = nullptr;

                        if (m_callType == API_CALL)
                        {
                            pDurationItem = m_pTop20Table->item(lastRowIndex, 3);
                        }
                        else
                        {
                            pDurationItem = m_pTop20Table->item(lastRowIndex, 2);
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
                m_pTop20Table->addRow(QString("      " GP_STR_SummaryTop20TableShowAll));
                int rowCount = m_pTop20Table->rowCount();
                m_pTop20Table->setSpan(rowCount - 1, 0, 1, 3);
            }
        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Called when the user changes line selection -> top20 table is updated with the top 5 calls
// If the min \ max column is selected -> zoom to item in timeline
void gpTraceSummaryTab::OnSummaryTableSelectionChanged()
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

            gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_pSummaryTable);
            GT_ASSERT(pSummaryTable != nullptr);
            if (pSummaryTable->GetItemCallIndex(rowIndex, apiCall, callName))
            {
                m_currentCallIndex = apiCall;
                FillTop20Table(apiCall, callName);
                pItem = pSummaryTable->GetRelatedItem(rowIndex, index.column());
                SyncSelectionInOtherControls(pItem);
            }

            pSummaryTable->SaveSelection(rowIndex);
        }
    }
}

// Called when the user changes cell selection in the selected row
// If the min \ max coloumn is selected -> zoom to item in timeline
void gpTraceSummaryTab::OnSummaryTableCellClicked(int row, int col)
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
    else if (acIsChildOf(pFocusedWidget, m_pTop20Table))
    {
        GT_IF_WITH_ASSERT(m_pTop20Table != nullptr)
        {
            m_pTop20Table->onFindClick();
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
    else if (acIsChildOf(pFocusedWidget, m_pTop20Table))
    {
        GT_IF_WITH_ASSERT(m_pTop20Table != nullptr)
        {
            m_pTop20Table->onEditSelectAll();
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
    else if (acIsChildOf(pFocusedWidget, m_pTop20Table))
    {
        GT_IF_WITH_ASSERT(m_pTop20Table != nullptr)
        {
            m_pTop20Table->onEditCopy();
        }
    }
}

void gpSummaryTab::OnUseTimelineSelectionScopeChanged(bool check)
{
    m_useTimelineSelectionScope = check;

    GT_IF_WITH_ASSERT(m_pTop20Table != nullptr && m_pSummaryTable != nullptr)
    {
        RefreshAndMaintainSelection(check);
    }
    emit TabUseTimelineSelectionScopeChanged(check);
}

void gpTraceSummaryTab::RefreshAndMaintainSelection(bool check)
{
    // save current selection in summary table to restore after m_pSummaryTable is refreshed
    int selectionIndex = -1;

    CallIndexId selectedInterface = 0;
    QString selectedApiCall;

    gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_pSummaryTable);
    GT_ASSERT(pSummaryTable != nullptr);

    QItemSelectionModel* pSelection = pSummaryTable->selectionModel();

    if (pSelection != nullptr && pSelection->hasSelection())
    {
        QModelIndexList selectedRows = pSelection->selectedRows();
        selectionIndex = selectedRows[0].row();
        pSummaryTable->GetItemCallIndex(selectionIndex, selectedInterface, selectedApiCall);
    }

    pSummaryTable->Refresh(check, m_timelineStart, m_timelineEnd);
    int rowCount = pSummaryTable->rowCount();

    for (int i = 0; i < rowCount; i++)
    {
        CallIndexId rowInterface;
        QString rowName;
        pSummaryTable->GetItemCallIndex(i, rowInterface, rowName);

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
        pSummaryTable->scrollTo(modelIdx);
    }
    else
    {
        m_pTop20Table->clearList();
    }
}

// check if <show more...> button was pressed
void gpSummaryTab::OnTop20TableCellClicked(int row, int col)
{
    GT_IF_WITH_ASSERT(m_pTop20Table != nullptr)
    {
        QModelIndexList indices = m_pTop20Table->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex index = indices.at(0);
            QTableWidgetItem* pItemCall = m_pTop20Table->item(row, col);

            if (pItemCall != nullptr && pItemCall->text().contains(GP_STR_SummaryTop20TableShowAll))
            {
                FillTop20Table(m_currentCallIndex, m_currentCallName, true);
            }
            else if ((m_callType == API_CALL && col == 3) || (m_callType == GPU_CALL && col == 2) || (m_callType == COMMAND_LIST && col == 2) )
            {
                ProfileSessionDataItem* pItem = m_top20ItemList.at(index.row());
                SyncSelectionInOtherControls(pItem);
            }

        }
    }
}

// select item in timeline
void gpTraceSummaryTab::OnTop20TableCellDoubleClicked(int row, int col)
{
    GT_IF_WITH_ASSERT(m_pTop20Table != nullptr)
    {
        QModelIndexList indices = m_pTop20Table->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex indexInTop20Table = indices.at(0);
            QTableWidgetItem* pItemCall = m_pTop20Table->item(row, col);

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

                    gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_pSummaryTable);
                    GT_ASSERT(pSummaryTable != nullptr);
                    if (pSummaryTable->GetItemCallIndex(selectedRowInSummaryTableIndex, apiCall, callName))
                    {
                        ProfileSessionDataItem* pItem = m_top20ItemList.at(indexInTop20Table.row());
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
void gpTraceSummaryTab::FillTop20List(CallIndexId apiCall, const QString& callName)
{
    GT_UNREFERENCED_PARAMETER(callName);

    gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_pSummaryTable);
    GT_ASSERT(pSummaryTable != nullptr);
    const QMap<CallIndexId, ProfileSessionDataItem*>& map = pSummaryTable->GetAllApiCallItemsMap();
    m_top20ItemList = map.values(apiCall);
}


void gpTraceSummaryTab::SetTop20TableCaption(const QString& callName)
{
    GT_IF_WITH_ASSERT(m_pTop20Caption != nullptr)
    {
        int numItems = m_top20ItemList.count();
        if (numItems < MAX_ITEMS_IN_TABLE)
        {
            if (m_callType == API_CALL)
            {
                m_pTop20Caption->setText(QString(GPU_STR_Top_Cpu_Calls_Summary).arg(callName));
            }
            else
            {
                m_pTop20Caption->setText(QString(GPU_STR_Top_Gpu_Calls_Summary).arg(callName));
            }
        }
        else
        {
            if (m_callType == API_CALL)
            {
                m_pTop20Caption->setText(QString(GPU_STR_Top_20_Cpu_Calls_Summary).arg(MAX_ITEMS_IN_TABLE).arg(callName));
            }
            else
            {
                m_pTop20Caption->setText(QString(GPU_STR_Top_20_Gpu_Calls_Summary).arg(MAX_ITEMS_IN_TABLE).arg(callName));
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
void gpCommandListSummaryTab::OnSummaryTableSelectionChanged()
{
    GT_IF_WITH_ASSERT((m_pTraceView != nullptr) && (m_pSummaryTable != nullptr))
    {

        // Get first of all selections
        QModelIndexList indices = m_pSummaryTable->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex index = indices.at(0);
            QString callName;
            int rowIndex = index.row();

            gpCommandListSummaryTable* pSummaryTable = dynamic_cast<gpCommandListSummaryTable*>(m_pSummaryTable);
            GT_ASSERT(pSummaryTable != nullptr);

            if (pSummaryTable->GetItemCommandList(rowIndex, callName))
            {
                FillTop20Table(0, callName);
                ProfileSessionDataItem* pItem = m_pSummaryTable->GetRelatedItem(rowIndex, index.column());
                SyncSelectionInOtherControls(pItem);
            }

            m_pSummaryTable->SaveSelection(rowIndex);
        }
    }
}
void gpCommandListSummaryTab::OnSummaryTableCellClicked(int row, int col)
{
    GT_IF_WITH_ASSERT(m_pTraceView != nullptr && m_pSummaryTable != nullptr)
    {
        ProfileSessionDataItem* pItem = m_pSummaryTable->GetRelatedItem(row, col);
        SyncSelectionInOtherControls(pItem);
    }
}

void gpCommandListSummaryTab::RefreshAndMaintainSelection(bool check)
{
    // save current selection in summary table to restore after m_pSummaryTable is refreshed
    int selectionIndex = -1;

    QString selectedApiCall;
    gpCommandListSummaryTable* pSummaryTable = dynamic_cast<gpCommandListSummaryTable*>(m_pSummaryTable);
    GT_ASSERT(pSummaryTable != nullptr);

    QItemSelectionModel* pSelection = m_pSummaryTable->selectionModel();

    if (pSelection != nullptr && pSelection->hasSelection())
    {
        QModelIndexList selectedRows = pSelection->selectedRows();
        selectionIndex = selectedRows[0].row();
        
        pSummaryTable->GetItemCommandList(selectionIndex, selectedApiCall);
    }

    m_pSummaryTable->Refresh(check, m_timelineStart, m_timelineEnd);
    int rowCount = m_pSummaryTable->rowCount();

    for (int i = 0; i < rowCount; i++)
    {
        QString rowName;
        pSummaryTable->GetItemCommandList(i, rowName);

        if (rowName == selectedApiCall)
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
        m_pTop20Table->clearList();
    }
}
void gpCommandListSummaryTab::OnTop20TableCellDoubleClicked(int row, int col)
{
    GT_UNREFERENCED_PARAMETER(row);
    GT_UNREFERENCED_PARAMETER(col);
    GT_IF_WITH_ASSERT(m_pTop20Table != nullptr)
    {
        QModelIndexList indices = m_pTop20Table->selectionModel()->selection().indexes();

        if (indices.size() > 0)
        {
            QModelIndex indexInTop20Table = indices.at(0);

            ProfileSessionDataItem* pItem = m_top20ItemList.at(indexInTop20Table.row());
            SyncSelectionInOtherControls(pItem);

        }
    }
}

void gpCommandListSummaryTab::FillTop20List(CallIndexId apiCall, const QString& callName)
{
    GT_UNREFERENCED_PARAMETER(apiCall);

    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr && m_pSummaryTable != nullptr)
    {
        // add items to m_pTop20TableItems for double-click
        gpCommandListSummaryTable* pSummaryTable = dynamic_cast<gpCommandListSummaryTable*>(m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);

        const QVector<gpTraceDataContainer::CommandListInstanceData>& commandListsData = m_pSessionDataContainer->CommandListsData();
        for (int i = 0; i < commandListsData.size(); i++)
        {
            QString cmdListPtr = commandListsData[i].m_commandListPtr;
            int instanceIndex = commandListsData[i].m_instanceIndex;
            QString currentCmdListName = m_pSessionDataContainer->CommandListNameFromPointer(cmdListPtr, instanceIndex);
            if (callName == currentCmdListName)
            {
                // Add all the API indices for this command list instance
                for (auto apiIndex : commandListsData[i].m_apiIndices)
                {
                    ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItemByItemCallIndex(commandListsData[i].m_commandListQueueName, apiIndex + 1);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        m_top20ItemList.push_back(pItem);
                    }
                }
                break;
            }
        }

        
    }
}

void gpCommandListSummaryTab::SelectCommandList(const QString& commandListName)
{
    GT_IF_WITH_ASSERT(m_pSummaryTable != nullptr)
    {
        gpCommandListSummaryTable* pSummaryTable = dynamic_cast<gpCommandListSummaryTable*>(m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);
        pSummaryTable->SelectCommandList(commandListName);
    }
}

void gpCommandListSummaryTab::SetTop20TableCaption(const QString& callName)
{
    GT_IF_WITH_ASSERT(m_pTop20Caption != nullptr)
    {
        int numItems = m_top20ItemList.count();
        if (numItems < MAX_ITEMS_IN_TABLE)
        {
            m_pTop20Caption->setText(QString(GPU_STR_Top_CommandLists_Summary).arg(callName));
        }
        else
        {
            m_pTop20Caption->setText(QString(GPU_STR_Top_20_CommandLists_Summary).arg(MAX_ITEMS_IN_TABLE).arg(callName));
        }
    }
}

void gpCommandListSummaryTab::OnSummaryTableCellDoubleClicked(int row, int col) 
{
    GT_UNREFERENCED_PARAMETER(col);
    GT_IF_WITH_ASSERT(m_pTraceView != nullptr && m_pSummaryTable != nullptr)
    {
        gpCommandListSummaryTable* pSummaryTable = dynamic_cast<gpCommandListSummaryTable*>(m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);

        QString commandListName;
        if (pSummaryTable->GetItemCommandList(row, commandListName))
        {
            emit TabSummaryCmdListClicked(commandListName);
        }
    }
}

