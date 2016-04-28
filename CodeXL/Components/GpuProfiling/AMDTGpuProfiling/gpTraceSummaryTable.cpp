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

const int SUMMARY_INFO_ARRAY_SIZE = 500; // 130 DX12 call types, X Vulcan call types...

gpTraceSummaryTable::gpTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, eCallType callType)
    : acListCtrl(nullptr), m_callType(callType), m_pSessionDataContainer(pDataContainer), m_pTraceView(pSessionView), m_lastSelectedRowIndex(-1)
{
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

    m_logic.Init(m_callType, m_pSessionDataContainer, pSessionView);

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

}

void gpTraceSummaryTable::SelectRowByCallName(const QString& callName)
{
    int rowCount = QTableWidget::rowCount();

    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
    {
        QString itemName;
        QTableWidgetItem* pItemInterface = item(rowIndex, COLUMN_CALL_NAME);

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
        APISummaryInfo info = m_logic.GetSummaryInfo(apiCall);

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



void gpTraceSummaryTable::AddSummaryRow(int rowIndex, APISummaryInfo* pInfo)
{
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

const QMap<CallIndexId, ProfileSessionDataItem*>& gpTraceSummaryTable::GetAllApiCallItemsMap()const
{
    return m_logic.GetAllApiCallItemsMap();
}

// This method should be called after m_logic InitAPISummary() or InitGPUSummary() were called, it assumes m_logic.m_apiCallInfoSummaryMap is filled with call data
void gpTraceSummaryTable::FillTable()
{
    clearList();
    QMapIterator<CallIndexId, APISummaryInfo> it(m_logic.GetApiCallInfoSummaryMap());
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

void gpTraceSummaryTable::Refresh(bool useTimelineScope, quint64 min, quint64 max)
{
    m_logic.RebuildSummaryMap(useTimelineScope, min, max);
    FillTable();
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

        QMapIterator<CallIndexId, APISummaryInfo> it(m_logic.GetApiCallInfoSummaryMap());

        while (it.hasNext())
        {
            it.next();
            APISummaryInfo info = it.value();

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

void gpTraceSummaryTable::SaveSelection(int row)
{
    m_lastSelectedRowIndex = row;
}

void gpTraceSummaryTable::RestoreSelection()
{
    if (m_lastSelectedRowIndex != -1)
    {
        selectRow(m_lastSelectedRowIndex);
    }
}

void gpTraceSummaryTable::ClearSelection()
{
    clearSelection();
    m_lastSelectedRowIndex = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// gpSummaryLogic
void gpSummaryLogic::Init(eCallType callType, gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView)
{
    m_callType = callType;
    m_pTraceView = pSessionView;

    if (callType == API_CALL)
    {
        InitAPIItems(pDataContainer);
    }
    else
    {
        InitGPUItems(pDataContainer);
    }
}

void gpSummaryLogic::InitAPIItems(gpTraceDataContainer* pSessionDataContainer)
{
    GT_IF_WITH_ASSERT(pSessionDataContainer != nullptr)
    {
        m_pSessionDataContainer = pSessionDataContainer;
        m_allCallItemsMultiMap.clear();
        m_totalTimeMs = 0;
        m_numTotalCalls = 0;
        int threadCount = pSessionDataContainer->ThreadsCount();

        // fill local array of SummaryInfo items
        APISummaryInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};

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
                    APISummaryInfo& info = infoArray[apiId];
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

void gpSummaryLogic::AddSessionItemToSummaryInfo(APISummaryInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId)
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

            if (duration < info.m_minTimeMs || info.m_minTimeMs == INT_MIN)
            {
                info.m_minTimeMs = duration;
                info.m_pMinTimeItem = pItem;
            }

        }
    }
}


void gpSummaryLogic::InitGPUItems(gpTraceDataContainer* pSessionDataContainer)
{
    GT_IF_WITH_ASSERT(pSessionDataContainer != nullptr)
    {
        m_pSessionDataContainer = pSessionDataContainer;

        m_allCallItemsMultiMap.clear();
        m_totalTimeMs = 0;
        m_numTotalCalls = 0;
        int queuesCount = pSessionDataContainer->DX12QueuesCount();
        // fill local array of SummaryInfo items
        APISummaryInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};

        for (int i = 0; i < queuesCount; i++)
        {
            afProgressBarWrapper::instance().incrementProgressBar();
            QString queueName = pSessionDataContainer->QueueName(i);
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
                    APISummaryInfo& info = infoArray[apiId];
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

void gpSummaryLogic::CollateAllItemsIntoSummaryMap(const QMap<CallIndexId, ProfileSessionDataItem*>& callMap)
{
    // insert items into map collated by call name
    m_apiCallInfoSummaryMap.clear();
    QList<CallIndexId> keys = callMap.uniqueKeys();

    foreach (CallIndexId key, keys)
    {
        APISummaryInfo info;
        QList<ProfileSessionDataItem*> values = callMap.values(key);

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

void gpSummaryLogic::BuildSummaryMapInTimelineScope(quint64 min, quint64 max)
{
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        m_apiCallInfoSummaryMap.clear();
        quint64 totalTime = 0;

        QList<CallIndexId> keys = m_allCallItemsMultiMap.uniqueKeys();

        foreach (CallIndexId key, keys)
        {
            APISummaryInfo info;
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
void gpSummaryLogic::InsertSummaryInfoToMap(APISummaryInfo& info)
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

void gpSummaryLogic::RebuildSummaryMap(bool useScope, quint64 startTime, quint64 endTime)
{
    // at this point m_logic.m_apiItemsMap is already filled with all items,
    // we only need to collate them into
    if (useScope)
    {
        // m_allCallItemsMultiMap already contains all items, we just need to insert items within mintime and maxtime into summary map
        BuildSummaryMapInTimelineScope(startTime, endTime);
    }
    else
    {
        // m_allCallItemsMultiMap already contains all items, we just need to collate m_allCallItemsMap into summary map
        CollateAllItemsIntoSummaryMap(m_allCallItemsMultiMap);
    }
}

APISummaryInfo gpSummaryLogic::GetSummaryInfo(int apiCall)
{
    APISummaryInfo dummyInfo;
    m_apiCallInfoSummaryMapIter = m_apiCallInfoSummaryMap.find(apiCall);

    if (m_apiCallInfoSummaryMapIter != m_apiCallInfoSummaryMap.end())
    {
        dummyInfo = m_apiCallInfoSummaryMapIter.value();
    }

    return dummyInfo;
}

const QMap<CallIndexId, APISummaryInfo>& gpSummaryLogic::GetApiCallInfoSummaryMap()const
{
    return m_apiCallInfoSummaryMap;
}

const QMap<CallIndexId, ProfileSessionDataItem*>& gpSummaryLogic::GetAllApiCallItemsMap()const
{
    return m_allCallItemsMultiMap;
}

void gpSummaryLogic::Cleanup()
{
    m_allCallItemsMultiMap.clear();
    m_apiCallInfoSummaryMap.clear();
}

