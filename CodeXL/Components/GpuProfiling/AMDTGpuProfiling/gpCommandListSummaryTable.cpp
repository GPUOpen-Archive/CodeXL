
//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpCommandListSummaryTable.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/gpTraceView.h>

#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

const int SUMMARY_INFO_ARRAY_SIZE = 500; // 130 DX12 call types, X Vulcan call types...

gpCommandListSummaryTable::gpCommandListSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart)
    : gpSummaryTable(pDataContainer, pSessionView, timelineAbsoluteStart)
{
    m_pSessionDataContainer = pDataContainer;
    bool isDx12 = (pDataContainer != nullptr && pDataContainer->SessionAPIType() == ProfileSessionDataItem::ProfileItemAPIType::DX12_API_PROFILE_ITEM);

    QStringList columnCaptions;
    if (isDx12)
    {
        columnCaptions << GP_STR_SummaryTableCommandListType;
    }
    else
    { 
        columnCaptions << GP_STR_SummaryTableCommandBufferType;
    }

    columnCaptions << GP_STR_SummaryTableCommandListExecutionTime;
    columnCaptions << GP_STR_SummaryTableCommandListStartTime;
    columnCaptions << GP_STR_SummaryTableCommandListEndTime;
    columnCaptions << GP_STR_SummaryTableCommandListNumCommands;
    columnCaptions << GP_STR_SummaryTableCommandListGPUQueue;

    if (isDx12)
    {
        columnCaptions << GP_STR_SummaryTableCommandListAddress;
    }
    else
    {
        columnCaptions << GP_STR_SummaryTableCommandListHandle;
    }

    initHeaders(columnCaptions, false);
    setShowGrid(true);
}

gpCommandListSummaryTable::~gpCommandListSummaryTable()
{

}

void gpCommandListSummaryTable::CollateAllItemsIntoSummaryMap()
{
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        m_apiCallInfoSummaryMap.clear();

        // insert items into map collated by call name
        QList<QString> keys = m_allInfoSummaryMap.uniqueKeys();

        foreach(QString key, keys)
        {
            APISummaryCommandListInfo info;
            QList<APISummaryCommandListInfo> values = m_allInfoSummaryMap.values(key);

            foreach(APISummaryCommandListInfo info, values)
            {
                 m_apiCallInfoSummaryMap.insert(key, info);
            }
        }
    }
}
bool gpCommandListSummaryTable::InitItems()
{
    bool rc = false;
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        APISummaryCommandListInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};
        const QVector<gpTraceDataContainer::CommandListInstanceData>& commandListsData = m_pSessionDataContainer->CommandListsData();

        int commandListIndex = 0;
        auto iter = commandListsData.begin();
        auto iterEnd = commandListsData.end();
        for (; iter != iterEnd; iter++)
        {
            gpTraceDataContainer::CommandListInstanceData commandListData = *iter;
            if (!commandListData.m_commandListQueueName.isEmpty())
            {
                afProgressBarWrapper::instance().incrementProgressBar();
                QString commandListName = m_pSessionDataContainer->CommandListNameFromPointer(commandListData.m_commandListPtr, commandListData.m_instanceIndex);

                APISummaryCommandListInfo& info = infoArray[commandListIndex];

                info.m_index = commandListName;

                info.m_gpuQueue = m_pSessionDataContainer->QueueDisplayName(commandListData.m_commandListQueueName);
                info.m_gpuQueueAddress = commandListData.m_commandListQueueName;

                info.m_address = commandListData.m_commandListPtr;
                info.m_minTimeMs = commandListData.m_startTime;
                info.m_maxTimeMs = commandListData.m_endTime;

                info.m_numCalls = commandListData.m_apiIndices.size();
                info.m_executionTimeMS = (commandListData.m_endTime - commandListData.m_startTime);
                info.m_typeColor = APIColorMap::Instance()->GetCommandListColor(commandListIndex);


                for (auto apiIndex : commandListData.m_apiIndices)
                {
                    //ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItemByItemCallIndex(commandListData.m_commandListQueueName, apiIndex + 1);
                    ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItem(commandListData.m_commandListQueueName, apiIndex);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        if (pItem->StartTime() == info.m_minTimeMs)
                        {
                            info.m_pMinTimeItem = pItem;
                        }
                        if (pItem->EndTime() == info.m_maxTimeMs)
                        {
                            info.m_pMaxTimeItem = pItem;
                        }

                        if (info.m_pMinTimeItem != nullptr && info.m_pMaxTimeItem != nullptr)
                        {
                            break;
                        }
                    }
                }
                // normalize to timeline start
                info.m_minTimeMs = (info.m_minTimeMs - m_timelineAbsoluteStart);
                info.m_maxTimeMs = (info.m_maxTimeMs - m_timelineAbsoluteStart);

                commandListIndex++;

                m_allInfoSummaryMap.insert(commandListName, info);
                m_apiCallInfoSummaryMap.insert(commandListName, info);
            }
        }
        rc = (commandListIndex > 0);
    }
    return rc;
}

void gpCommandListSummaryTable::FillTable()
{
    clearList();
    int rowIndex = 0;

    setSortingEnabled(false);

    QMapIterator<QString, APISummaryCommandListInfo> it(m_apiCallInfoSummaryMap);
    while (it.hasNext())
    {
        it.next();
        AddSummaryRow(rowIndex, (APISummaryCommandListInfo*)&it.value());

        rowIndex++;
    }

    setSortingEnabled(true);

    sortByColumn(CommandListSummaryColumnIndex::COLUMN_EXECUTION_TIME);
    horizontalHeader()->setSortIndicator(CommandListSummaryColumnIndex::COLUMN_EXECUTION_TIME, Qt::DescendingOrder);
    horizontalHeader()->setSortIndicatorShown(true);
    selectRow(0);
}

void gpCommandListSummaryTable::AddSummaryRow(int rowIndex, APISummaryInfo* pInfo)
{
    int rowCount = QTableWidget::rowCount();
    GT_IF_WITH_ASSERT(pInfo != nullptr && rowIndex == rowCount)
    {

        QStringList rowStrings;
        pInfo->TableItemsAsString(rowStrings);

        insertRow(rowIndex);


        // Sanity check: make sure that the string are fully built
        GT_IF_WITH_ASSERT(rowStrings.size() >= CommandListSummaryColumnIndex::COLUMN_COUNT - 1)
        {
            for (int i = 0; i < CommandListSummaryColumnIndex::COLUMN_COUNT; i++)
            {
                QTableWidgetItem* pItem = nullptr;

                bool shouldSetValue = true;
                bool shouldSetCmdBufferTooltip = false;

                switch (i)
                {
                case COLUMN_ADDRESS:
                case COLUMN_COMMAND_INDEX:
                {
                    pItem = allocateNewWidgetItem(rowStrings[i]);
                    setItem(rowIndex, i, pItem);
                    initItem(*pItem, rowStrings[i], nullptr, false, Qt::Unchecked, nullptr);
                    setItemTextColor(rowIndex, i, pInfo->m_typeColor);
                    shouldSetValue = false;
                }
                break;
                case COLUMN_GPU_QUEUE:
                {
                    pItem = allocateNewWidgetItem(rowStrings[i]);
                    setItem(rowIndex, i, pItem);
                    initItem(*pItem, rowStrings[i], nullptr, false, Qt::Unchecked, nullptr);
                    shouldSetValue = false;
                }
                break;

                case COLUMN_START_TIME:
                case COLUMN_END_TIME:
                {
                    pItem = new FormattedTimeItem();
                    ((FormattedTimeItem*)pItem)->SetAsLink(true);

                }
                break;
                case COLUMN_EXECUTION_TIME:
                {
                    pItem = new FormattedTimeItem();
                }
                break;
                case COLUMN_NUM_OF_COMMANDS:
                {
                    pItem = new QTableWidgetItem();

                    // For number of commands, set both text and value, to make sure that the column is sortable, and enable
                    // display of N/A string for 0 calls 
                    setItem(rowIndex, i, pItem);

                    if (pInfo->m_numCalls == 0)
                    {
                        shouldSetCmdBufferTooltip = true;
                    }
                    QVariant dataVariant;
                    dataVariant.setValue(rowStrings[i].toDouble());
                    pItem->setData(Qt::EditRole, dataVariant);
                    pItem->setData(Qt::DisplayRole, rowStrings[i]);
                    shouldSetValue = false;
                }
                break;
                }

                if (shouldSetValue)
                {
                    setItem(rowIndex, i, pItem);
                    QVariant dataVariant;
                    dataVariant.setValue(rowStrings[i].toDouble());
                    pItem->setData(Qt::DisplayRole, dataVariant);
                }
                if (shouldSetCmdBufferTooltip)
                {
                    pItem->setToolTip(GP_STR_SummaryCmdBufferToolTip);
                }
                else
                {
                    pItem->setToolTip(pItem->text());
                }
                pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
        }
    }
}

bool gpCommandListSummaryTable::GetItemCommandList(int row, QString& callName)const
{
    // Get the table widget item:
    QTableWidgetItem* pItemInterface = item(row, COLUMN_COMMAND_INDEX);

    if (pItemInterface != nullptr)
    {
        callName = pItemInterface->text();
        return true;
    }
    return false;
}

bool gpCommandListSummaryTable::GetItemCommandListAddress(int row, QString& address)const
{
    // Get the table widget item:
    QTableWidgetItem* pItemInterface = item(row, COLUMN_ADDRESS);

    if (pItemInterface != nullptr)
    {
        address = pItemInterface->text();
        return true;
    }
    return false;
}

bool gpCommandListSummaryTable::GetItemQueueName(int row, QString& queueName)const
{
    // Get the table widget item:
    QTableWidgetItem* pItemInterface = item(row, COLUMN_GPU_QUEUE);

    if (pItemInterface != nullptr)
    {
        queueName = pItemInterface->text();
        return true;
    }

    return false;
}

void gpCommandListSummaryTable::OnCellEntered(int row, int column)
{
    GT_UNREFERENCED_PARAMETER(row);
    GT_UNREFERENCED_PARAMETER(column);

    setCursor(Qt::ArrowCursor);
}

void gpCommandListSummaryTable::BuildSummaryMapInTimelineScope(quint64 min, quint64 max)
{
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        m_apiCallInfoSummaryMap.clear();
                    
        // insert items into map collated by call name
        QList<QString> keys = m_allInfoSummaryMap.uniqueKeys();

        foreach(QString key, keys)
        {
            APISummaryCommandListInfo info;
            QList<APISummaryCommandListInfo> values = m_allInfoSummaryMap.values(key);

            foreach(APISummaryCommandListInfo info, values)
            {
                if (info.m_minTimeMs >= min && info.m_maxTimeMs <= max)
                {
                    m_apiCallInfoSummaryMap.insert(key, info);
                }
            }
        }
    }
}

ProfileSessionDataItem* gpCommandListSummaryTable::GetRelatedItem(int rowIndex, int colIndex)
{
    ProfileSessionDataItem* pItem = nullptr;
    QString callName;

    if (GetItemCommandList(rowIndex, callName))
    {
        APISummaryCommandListInfo info = GetSummaryInfo(callName);

        if (colIndex == COLUMN_START_TIME)
        {
            pItem = info.m_pMinTimeItem;
        }
        else if (colIndex == COLUMN_END_TIME)
        {
            pItem = info.m_pMaxTimeItem;
        }
    }

    return pItem;
}

APISummaryCommandListInfo gpCommandListSummaryTable::GetSummaryInfo(const QString& callName)
{
    APISummaryCommandListInfo dummyInfo;
    QMap<QString, APISummaryCommandListInfo>::const_iterator apiCallInfoSummaryMapIter = m_apiCallInfoSummaryMap.find(callName);

    if (apiCallInfoSummaryMapIter != m_apiCallInfoSummaryMap.end())
    {
        dummyInfo = apiCallInfoSummaryMapIter.value();
    }

    return dummyInfo;
}

void gpCommandListSummaryTable::SelectCommandList(const QString& commandListName)
{
    int rowCount = QTableWidget::rowCount();

    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
    {
        QString itemName;
        QTableWidgetItem* pItemInterface = item(rowIndex, CommandListSummaryColumnIndex::COLUMN_ADDRESS);

        if (pItemInterface != nullptr)
        {
            itemName = pItemInterface->text();
        }

        if (commandListName == itemName)
        {
            selectRow(rowIndex);
            m_lastSelectedRowIndex = rowIndex;
            QModelIndex modelIdx = model()->index(rowIndex, 0);
            scrollTo(modelIdx);
            break;
        }
    }
}

