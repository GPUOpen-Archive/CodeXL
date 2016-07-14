//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpCommandListSummaryTab.h>
#include <AMDTGpuProfiling/gpCommandListSummaryTable.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/gpStringConstants.h>

#include <AMDTApplicationComponents/Include/acColours.h>


const int MAX_ITEMS_IN_TABLE = 20;

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
                    //ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItem(commandListsData[i].m_commandListQueueName, apiIndex) ;
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
        if (pSummaryTable->GetItemCommandListAddress(row, commandListName))
        {
            emit TabSummaryCmdListDoubleClicked(commandListName);
        }
    }
}

QString gpCommandListSummaryTab::getCaption()const 
{
    QString retVall = GPU_STR_Command_Lists_Summary;

    GT_IF_WITH_ASSERT(m_pSessionDataContainer)
    {
        if (m_pSessionDataContainer->SessionAPIType() == ProfileSessionDataItem::ProfileItemAPIType::VK_GPU_PROFILE_ITEM)
        {
            retVall = GPU_STR_Command_Buffers_Summary;
        }
    }
    return retVall;
}
