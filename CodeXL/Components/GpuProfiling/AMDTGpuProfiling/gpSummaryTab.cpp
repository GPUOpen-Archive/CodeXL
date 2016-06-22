//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpSummaryTab.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/gpTraceSummaryTable.h>
#include <AMDTGpuProfiling/gpCommandListSummaryTable.h>


#include <AMDTApplicationComponents/Include/acColours.h>

const int MAX_ITEMS_IN_TABLE = 20;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
gpSummaryTab::gpSummaryTab(eCallType callType, quint64 timelineAbsoluteStart)
    : m_callType(callType), m_pTraceView(nullptr), m_pSummaryTable(nullptr), m_pTop20Table(nullptr), m_pTop20Caption(nullptr), m_pChkboxUseScope(nullptr), m_useTimelineSelectionScope(false), m_timelineStart(0), m_timelineEnd(0), m_currentCallIndex(0), m_timelineAbsoluteStart(timelineAbsoluteStart)
{
}

// gpSummaryTab
bool gpSummaryTab::Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStart, quint64 timelineEnd)
{
    m_pTraceView = pSessionView;
    m_pSessionDataContainer = pDataContainer;
    if (m_callType == API_CALL)
    {
        m_pSummaryTable = new gpCPUTraceSummaryTable(pDataContainer, pSessionView, m_timelineAbsoluteStart);
    }
    else if (m_callType == GPU_CALL)
    {
        m_pSummaryTable = new gpGPUTraceSummaryTable(pDataContainer, pSessionView, m_timelineAbsoluteStart);
    }
    else
    {
        m_pSummaryTable = new gpCommandListSummaryTable(pDataContainer, pSessionView, m_timelineAbsoluteStart);
    }
    bool rc = m_pSummaryTable->Init();
    if (rc == true)
    {
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

        QLabel* pLblCaption = new QLabel(getCaption());
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

        rc = connect(m_pChkboxUseScope, SIGNAL(toggled(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
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
        OnSummaryTableSelectionChanged(); // NZ for some reason the signal isn't enough
    }
    else
    {
        delete m_pSummaryTable;
        m_pSummaryTable = nullptr;
    }

    return rc;
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
    if (m_pChkboxUseScope != nullptr)
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


