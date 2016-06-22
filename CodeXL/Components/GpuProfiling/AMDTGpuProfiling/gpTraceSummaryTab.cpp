//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryTab.h>
#include <AMDTGpuProfiling/gpTraceView.h>


#include <AMDTApplicationComponents/Include/acColours.h>

const int MAX_ITEMS_IN_TABLE = 20;


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

