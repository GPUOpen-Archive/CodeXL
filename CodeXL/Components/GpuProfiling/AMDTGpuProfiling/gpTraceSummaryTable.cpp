
//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryTable.h>
#include <AMDTGpuProfiling/gpTraceSummaryWidget.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/gpTraceView.h>

#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

const int SUMMARY_INFO_ARRAY_SIZE = 500; // 130 DX12 call types, X Vulcan call types...


void gpSummaryTable::Refresh(bool useTimelineScope, quint64 min, quint64 max)
{
    RebuildSummaryMap(useTimelineScope, min, max);
    FillTable();
}

gpTraceSummaryTable::gpTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, eCallType callType)
    : gpSummaryTable(pDataContainer, pSessionView, callType), m_callType(callType)
{
    m_pSessionDataContainer = pDataContainer;

    QStringList columnCaptions;
    columnCaptions << GP_STR_SummaryTableColumnCall;
    columnCaptions << GP_STR_SummaryTableColumnMaxTime;
    columnCaptions << GP_STR_SummaryTableColumnMinTime;
    columnCaptions << GP_STR_SummaryTableColumnAvgTime;
    columnCaptions << GP_STR_SummaryTableColumnCumulativeTime;
    columnCaptions << GP_STR_SummaryTableColumnPercentageOfTotalTime;
    columnCaptions << GP_STR_SummaryTableColumnNumberOfCalls;
    initHeaders(columnCaptions, false);
    setShowGrid(true);

    Init(m_callType, m_pSessionDataContainer, pSessionView);

    // fill Table widget
    FillTable();
    setSortingEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::NoContextMenu);

    // Connect to the cell entered signal
    setMouseTracking(true);
    bool rc = connect(this, SIGNAL(cellEntered(int, int)), this, SLOT(OnCellEntered(int, int)));
    GT_ASSERT(rc);
}

gpTraceSummaryTable::~gpTraceSummaryTable()
{
    Cleanup();
}

void gpTraceSummaryTable::SelectRowByCallName(const QString& callName)
{
    GT_UNREFERENCED_PARAMETER(callName);

    int rowCount = QTableWidget::rowCount();

    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
    {
        QString itemName;
        QTableWidgetItem* pItemInterface = item(rowIndex, /*TraceSummaryColumnIndex::COLUMN_CALL_NAME*/0);

        if (pItemInterface != nullptr)
        {
            itemName = pItemInterface->text();
        }
        
        if (callName == itemName)
        {
            selectRow(rowIndex);
            m_lastSelectedRowIndex = rowIndex;
            break;
        }
    }
}

ProfileSessionDataItem* gpTraceSummaryTable::GetRelatedItem(int rowIndex, int colIndex)
{
    ProfileSessionDataItem* pItem = nullptr;
    CallIndexId apiCall;
    QString callName;

    if (GetItemCallIndex(rowIndex, apiCall, callName))
    {
        APISummaryTraceInfo info = GetSummaryInfo(apiCall);

        if (colIndex == COLUMN_MIN_TIME)
        {
            pItem = info.m_pMinTimeItem;
        }
        else if (colIndex == COLUMN_MAX_TIME)
        {
            pItem = info.m_pMaxTimeItem;
        }
    }

    return pItem;
}



void gpTraceSummaryTable::AddSummaryRow(int rowIndex, APISummaryInfo* pSummaryInfo)
{
    APISummaryTraceInfo* pInfo = dynamic_cast<APISummaryTraceInfo*>(pSummaryInfo);
    GT_IF_WITH_ASSERT(pInfo != nullptr && rowIndex == rowCount())
    {

        QStringList rowStrings;
        pInfo->TableItemsAsString(rowStrings);

        insertRow(rowIndex);

        for (int i = 0; i < TraceSummaryColumnIndex::COLUMN_COUNT; i++)
        {
            QTableWidgetItem* pItem = nullptr;

            bool shouldSetValue = true;

            switch (i)
            {
                case COLUMN_CALL_NAME:
                {
                    pItem = allocateNewWidgetItem(rowStrings[i]);
                    setItem(rowIndex, i, pItem);
                    initItem(*pItem, rowStrings[i], nullptr, false, Qt::Unchecked, nullptr);
                    setItemTextColor(rowIndex, i, pInfo->m_typeColor);
                    pItem->setData(Qt::UserRole, pInfo->m_interface);
                    shouldSetValue = false;
                }
                break;

                case COLUMN_CUMULATIVE_TIME:
                {
                    pItem = new FormattedTimeItem();
                }
                break;

                case COLUMN_PERCENTAGE_OF_TOTAL_TIME:
                {
                    pItem = new PercentageItem();
                }
                break;

                case COLUMN_NUM_OF_CALLS:
                {
                    pItem = new QTableWidgetItem();
                }
                break;

                case COLUMN_AVG_TIME:
                {
                    pItem = new FormattedTimeItem();
                }
                break;

                case COLUMN_MAX_TIME:
                case COLUMN_MIN_TIME:
                {
                    pItem = new FormattedTimeItem();
                    ((FormattedTimeItem*)pItem)->SetAsLink(true);
                }
                break;
            }

            if (shouldSetValue)
            {
                setItem(rowIndex, i, pItem);
                QVariant dataVariant;
                dataVariant.setValue(rowStrings[i].toDouble());
                pItem->setData(Qt::DisplayRole, dataVariant);
                pItem->setToolTip(pItem->text());
            }

            pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }
    }
}

// This method should be called after InitAPISummary() or InitGPUSummary() were called, it assumes m_apiCallInfoSummaryMap is filled with call data
void gpTraceSummaryTable::FillTable()
{
    clearList();
    QMapIterator<CallIndexId, APISummaryTraceInfo> it(GetApiCallInfoSummaryMap());
    int rowIndex = 0;

    setSortingEnabled(false);

    while (it.hasNext())
    {
        it.next();
        AddSummaryRow(rowIndex, (APISummaryInfo*)&it.value());

        rowIndex++;
    }

    setSortingEnabled(true);

    sortByColumn(COLUMN_MAX_TIME);
    horizontalHeader()->setSortIndicator(COLUMN_MAX_TIME, Qt::DescendingOrder);
    horizontalHeader()->setSortIndicatorShown(true);
    selectRow(0);
}


bool gpTraceSummaryTable::GetItemCallIndex(int row, CallIndexId& callIndex, QString& callName)const
{
    // Get the table widget item:
    QTableWidgetItem* pItemInterface = item(row, COLUMN_CALL_NAME);

    if (pItemInterface != nullptr)
    {
        callName = pItemInterface->text();
        QVariant interfaceAsVar = pItemInterface->data(Qt::UserRole).toString();
        QString textInterface = interfaceAsVar.toString();

        QMapIterator<CallIndexId, APISummaryTraceInfo> it(GetApiCallInfoSummaryMap());

        while (it.hasNext())
        {
            it.next();
            APISummaryTraceInfo info = it.value();

            if (textInterface == info.m_interface && callName == info.m_call)
            {
                callIndex = it.key();
                return true;
            }
        }
    }

    return false;
}


void gpTraceSummaryTable::OnCellEntered(int row, int column)
{
    GT_UNREFERENCED_PARAMETER(row);

    if ((column == COLUMN_MAX_TIME) || (column == COLUMN_MIN_TIME))
    {
        setCursor(Qt::PointingHandCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}

gpSummaryTable::gpSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, eCallType callType)
    :acListCtrl(nullptr), m_pSessionDataContainer(pDataContainer), m_pTraceView(pSessionView), m_lastSelectedRowIndex(-1)
{
    GT_UNREFERENCED_PARAMETER(callType);
}

gpSummaryTable::~gpSummaryTable()
{

}

void gpSummaryTable::SaveSelection(int row)
{
    m_lastSelectedRowIndex = row;
}

void gpSummaryTable::RestoreSelection()
{
    if (m_lastSelectedRowIndex != -1)
    {
        selectRow(m_lastSelectedRowIndex);
    }
}

void gpSummaryTable::ClearSelection()
{
    clearSelection();
    m_lastSelectedRowIndex = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// gpSummaryTable
void gpTraceSummaryTable::Init(eCallType callType, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView)
{
    m_callType = callType;
    m_pTraceView = pSessionView;

    switch (callType)
    {
    case API_CALL:
        InitAPIItems(pDataContainer);
        break;
    case GPU_CALL:
        InitGPUItems(pDataContainer);
        break;
    default:
        GT_ASSERT(0);
    }
}

void gpTraceSummaryTable::InitAPIItems(gpTraceDataContainer* pSessionDataContainer)
{
    GT_IF_WITH_ASSERT(pSessionDataContainer != nullptr)
    {
        m_pSessionDataContainer = pSessionDataContainer;
        m_allCallItemsMultiMap.clear();
        m_totalTimeMs = 0;
        m_numTotalCalls = 0;
        int threadCount = pSessionDataContainer->ThreadsCount();

        // fill local array of SummaryInfo items
        APISummaryTraceInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};

        for (int i = 0; i < threadCount; i++)
        {
            afProgressBarWrapper::instance().incrementProgressBar();
            osThreadId threadID = pSessionDataContainer->ThreadID(i);

            int apiCount = pSessionDataContainer->ThreadAPICount(threadID);

            for (int i = 0; i < apiCount; i++)
            {
                // Get the current API item
                ProfileSessionDataItem* pItem = pSessionDataContainer->APIItem(threadID, i);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    unsigned int apiId;
                    pItem->GetAPIFunctionID(apiId);
                    m_allCallItemsMultiMap.insertMulti(apiId, pItem);
                    quint64 startTime = pItem->StartTime();
                    quint64 endTime = pItem->EndTime();
                    m_totalTimeMs += (endTime - startTime);
                    m_numTotalCalls++;
                    APISummaryTraceInfo& info = infoArray[apiId];
                    AddSessionItemToSummaryInfo(info, pItem, apiId);
                }
            }
        }

        // insert array items to summary map
        for (auto info : infoArray)
        {
            InsertSummaryInfoToMap(info);
        }
    }
}

void gpTraceSummaryTable::AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId)
{
    GT_IF_WITH_ASSERT(pItem != nullptr && m_pTraceView != nullptr)
    {
        APICallId callId;
        callId.m_callIndex = apiId;
        callId.m_tid = pItem->ThreadID();

        bool showItem = true;

        if (m_callType == API_CALL) // API items may be filtered out
        {
            acAPITimelineItem* pTimelineItem = m_pTraceView->GetAPITimelineItem(callId);

            if (pTimelineItem != nullptr)
            {
                acTimelineBranch* pBranch = pTimelineItem->parentBranch();

                if (pBranch && pBranch->IsVisible() == false)
                {
                    showItem = false;
                }
            }
        }

        if (showItem == true)
        {
            quint64 startTime = pItem->StartTime();
            quint64 endTime = pItem->EndTime();
            quint64 duration = endTime - startTime;

            if (info.m_numCalls == 0)
            {
                info.m_call = pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN).toString();
                info.m_interface = pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN).toString();
                info.m_typeColor = APIColorMap::Instance()->GetAPIColor(pItem->ItemType(), apiId, acQGREY_TEXT_COLOUR);
            }

            info.m_numCalls++;
            info.m_callIndex = apiId;
            info.m_cumulativeTime += duration;

            if (duration > info.m_maxTimeMs)
            {
                info.m_maxTimeMs = duration;
                info.m_pMaxTimeItem = pItem;
            }

            if (duration < info.m_minTimeMs || info.m_minTimeMs == std::numeric_limits<quint64>::max())
            {
                info.m_minTimeMs = duration;
                info.m_pMinTimeItem = pItem;
            }
        }
    }
}

void gpTraceSummaryTable::InitGPUItems(gpTraceDataContainer* pSessionDataContainer)
{
    GT_IF_WITH_ASSERT(pSessionDataContainer != nullptr)
    {
        m_pSessionDataContainer = pSessionDataContainer;

        m_allCallItemsMultiMap.clear();
        m_totalTimeMs = 0;
        m_numTotalCalls = 0;
        int queuesCount = pSessionDataContainer->GPUCallsContainersCount();
        // fill local array of SummaryInfo items
        APISummaryTraceInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};

        for (int i = 0; i < queuesCount; i++)
        {
            afProgressBarWrapper::instance().incrementProgressBar();
            QString queueName = pSessionDataContainer->GPUObjectName(i);
            int apiCount = pSessionDataContainer->QueueItemsCount(queueName);

            for (int i = 0; i < apiCount; i++)
            {
                // Get the current API item
                ProfileSessionDataItem* pItem = pSessionDataContainer->QueueItem(queueName, i);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    unsigned int apiId;
                    pItem->GetAPIFunctionID(apiId);
                    m_allCallItemsMultiMap.insertMulti(apiId, pItem);
                    quint64 startTime = pItem->StartTime();
                    quint64 endTime = pItem->EndTime();
                    m_totalTimeMs += (endTime - startTime);
                    m_numTotalCalls++;
                    APISummaryTraceInfo& info = infoArray[apiId];
                    AddSessionItemToSummaryInfo(info, pItem, apiId);
                }
            }
        }

        // insert array items to summary map
        for (auto info : infoArray)
        {
            InsertSummaryInfoToMap(info);
        }
    }
}



void gpTraceSummaryTable::CollateAllItemsIntoSummaryMap()
{
    // insert items into map collated by call name
    m_apiCallInfoSummaryMap.clear();
    QList<CallIndexId> keys = m_allCallItemsMultiMap.uniqueKeys();

    foreach (CallIndexId key, keys)
    {
        APISummaryTraceInfo info;
        QList<ProfileSessionDataItem*> values = m_allCallItemsMultiMap.values(key);

        foreach (ProfileSessionDataItem* pItem, values)
        {
            if (pItem != nullptr)
            {
                unsigned int apiId;
                pItem->GetAPIFunctionID(apiId);
                AddSessionItemToSummaryInfo(info, pItem, apiId);
            }
        }

        if (info.m_numCalls > 0)
        {
            m_apiCallInfoSummaryMap.insert(info.m_callIndex, info);
        }

        InsertSummaryInfoToMap(info);
    }
}

void gpTraceSummaryTable::BuildSummaryMapInTimelineScope(quint64 min, quint64 max)
{
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        m_apiCallInfoSummaryMap.clear();
        quint64 totalTime = 0;

        QList<CallIndexId> keys = m_allCallItemsMultiMap.uniqueKeys();

        foreach (CallIndexId key, keys)
        {
            APISummaryTraceInfo info;
            QList<ProfileSessionDataItem*> values = m_allCallItemsMultiMap.values(key);

            foreach (ProfileSessionDataItem* pItem, values)
            {
                if (pItem != nullptr)
                {

                    quint64 startTime = pItem->StartTime();
                    quint64 endTime = pItem->EndTime();

                    if (min <= startTime && max >= endTime)
                    {
                        unsigned int apiId;
                        pItem->GetAPIFunctionID(apiId);
                        AddSessionItemToSummaryInfo(info, pItem, apiId);
                        totalTime += (endTime - startTime);
                    }
                }
            }

            InsertSummaryInfoToMap(info);
        }
    }
}

// avg and percentage are calculated with regards to all calls
void gpTraceSummaryTable::InsertSummaryInfoToMap(APISummaryTraceInfo& info)
{
    if (m_numTotalCalls > 0 && info.m_numCalls > 0 && m_totalTimeMs > 0)
    {
        // can't calculate  m_avgTimeMs yet, because not all samples were gathered
        float cumulativeTimeF = static_cast<float>(info.m_cumulativeTime);
        float totalTimeF = static_cast<float>(m_totalTimeMs);
        info.m_percentageOfTotalTime = cumulativeTimeF / totalTimeF * 100.f;
        m_apiCallInfoSummaryMap.insert(info.m_callIndex, info);
    }
}

void gpSummaryTable::RebuildSummaryMap(bool useScope, quint64 startTime, quint64 endTime)
{
    // at this point m_apiItemsMap is already filled with all items,
    // we only need to collate them into
    if (useScope)
    {
        // m_allCallItemsMultiMap already contains all items, we just need to insert items within mintime and maxtime into summary map
        BuildSummaryMapInTimelineScope(startTime, endTime);
    }
    else
    {
        // m_allCallItemsMultiMap already contains all items, we just need to collate m_allCallItemsMap into summary map
        CollateAllItemsIntoSummaryMap();
    }
}

APISummaryTraceInfo gpTraceSummaryTable::GetSummaryInfo(int apiCall)
{
    APISummaryTraceInfo dummyInfo;
    m_apiCallInfoSummaryMapIter = m_apiCallInfoSummaryMap.find(apiCall);

    if (m_apiCallInfoSummaryMapIter != m_apiCallInfoSummaryMap.end())
    {
        dummyInfo = m_apiCallInfoSummaryMapIter.value();
    }

    return dummyInfo;
}

const QMap<CallIndexId, APISummaryTraceInfo>& gpTraceSummaryTable::GetApiCallInfoSummaryMap()const
{
    return m_apiCallInfoSummaryMap;
}

const QMap<CallIndexId, ProfileSessionDataItem*>& gpTraceSummaryTable::GetAllApiCallItemsMap()const
{
    return m_allCallItemsMultiMap;
}

void gpTraceSummaryTable::Cleanup()
{
    m_allCallItemsMultiMap.clear();
    m_apiCallInfoSummaryMap.clear();
}

/////////////////////////////////////////////////////////////////////////////////
gpCommandListSummaryTable::gpCommandListSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView)
    : gpSummaryTable(pDataContainer, pSessionView, (eCallType)3), m_pSessionDataContainer(pDataContainer), m_pTraceView(pSessionView), m_lastSelectedRowIndex(-1)
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
        columnCaptions << GP_STR_SummaryTableCommandListHandle;
    }
    else
    {
        columnCaptions << GP_STR_SummaryTableCommandListAddress;
    }

    initHeaders(columnCaptions, false);
    setShowGrid(true);
    InitCommandListItems();
    // fill Table widget
    FillTable();
    setSortingEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::NoContextMenu);

    // Connect to the cell entered signal
    setMouseTracking(true);
    bool rc = connect(this, SIGNAL(cellEntered(int, int)), this, SLOT(OnCellEntered(int, int)));
    GT_ASSERT(rc);
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
void gpCommandListSummaryTable::InitCommandListItems()
{
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        APISummaryCommandListInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};
        const QVector<gpTraceDataContainer::CommandListInstanceData>& commandListsData = m_pSessionDataContainer->CommandListsData();

        int commandListIndex = 0;
        auto iter = commandListsData.begin();
        auto iterEnd = commandListsData.end();
        for (; iter != iterEnd; iter++)
        {
            if (!(*iter).m_commandListQueueName.isEmpty())
            {
                afProgressBarWrapper::instance().incrementProgressBar();
                QString commandListName = m_pSessionDataContainer->CommandListNameFromPointer((*iter).m_commandListPtr, (*iter).m_instanceIndex);

                APISummaryCommandListInfo& info = infoArray[commandListIndex];

                info.m_index = commandListName;

                info.m_gpuQueue = m_pSessionDataContainer->QueueDisplayName((*iter).m_commandListQueueName);
                info.m_gpuQueueAddress = (*iter).m_commandListQueueName;

                info.m_address = (*iter).m_commandListPtr;
                info.m_minTimeMs = (*iter).m_startTime;
                info.m_maxTimeMs = (*iter).m_endTime;
                info.m_numCalls = (*iter).m_apiIndices.size();
                info.m_executionTimeMS = ((*iter).m_endTime - (*iter).m_startTime);
                info.m_typeColor = APIColorMap::Instance()->GetCommandListColor(commandListIndex);


                for (auto apiIndex : (*iter).m_apiIndices)
                {
                    ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItemByItemCallIndex((*iter).m_commandListQueueName, apiIndex + 1);
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

                commandListIndex++;

                m_allInfoSummaryMap.insert(commandListName, info);
                m_apiCallInfoSummaryMap.insert(commandListName, info);
            }
        }
    }
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
                }
                break;
                }

                if (shouldSetValue)
                {
                    setItem(rowIndex, i, pItem);
                    QVariant dataVariant;
                    dataVariant.setValue(rowStrings[i].toDouble());
                    pItem->setData(Qt::DisplayRole, dataVariant);
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

