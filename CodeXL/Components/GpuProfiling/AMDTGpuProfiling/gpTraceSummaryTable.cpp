
//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryTable.h>
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

gpTraceSummaryTable::gpTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart)
    : gpSummaryTable(pDataContainer, pSessionView, timelineAbsoluteStart)
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

//bool gpTraceSummaryTable::Init()
//{
//    bool rc = InitItems();
//    if (rc == true)
//    {
//        // fill Table widget
//        FillTable();
//        setSortingEnabled(true);
//        setSelectionMode(QAbstractItemView::SingleSelection);
//        setContextMenuPolicy(Qt::NoContextMenu);
//
//        // Connect to the cell entered signal
//        setMouseTracking(true);
//        rc = connect(this, SIGNAL(cellEntered(int, int)), this, SLOT(OnCellEntered(int, int)));
//        GT_ASSERT(rc);
//    }
//
//    return rc;
//}


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

        m_apiCallInfoSummaryMap.insert(info.m_callIndex, info);
        
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

/////////////////////////////////////////////////////
// gpCPUTraceSummaryTable
gpCPUTraceSummaryTable::gpCPUTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart)
    : gpTraceSummaryTable(pDataContainer, pSessionView, timelineAbsoluteStart)
{

}


bool gpCPUTraceSummaryTable::InitItems()
{
    bool rc = false;
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        m_allCallItemsMultiMap.clear();
        m_totalTimeMs = 0;
        m_numTotalCalls = 0;
        int threadCount = m_pSessionDataContainer->ThreadsCount();

        // fill local array of SummaryInfo items
        APISummaryTraceInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};

        for (int i = 0; i < threadCount; i++)
        {
            afProgressBarWrapper::instance().incrementProgressBar();
            osThreadId threadID = m_pSessionDataContainer->ThreadID(i);

            int apiCount = m_pSessionDataContainer->ThreadAPICount(threadID);

            for (int i = 0; i < apiCount; i++)
            {
                // Get the current API item
                ProfileSessionDataItem* pItem = m_pSessionDataContainer->APIItem(threadID, i);
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
        if (threadCount > 0)
        {
            rc = true;
            // insert array items to summary map
            for (auto info : infoArray)
            {
                InsertSummaryInfoToMap(info);
            }
        }
    }
    return rc;
}

void gpCPUTraceSummaryTable::AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId)
{
    GT_IF_WITH_ASSERT(pItem != nullptr && m_pTraceView != nullptr)
    {
        APICallId callId;
        callId.m_callIndex = apiId;
        callId.m_tid = pItem->ThreadID();

        bool showItem = true;


        acAPITimelineItem* pTimelineItem = m_pTraceView->GetAPITimelineItem(callId);

        if (pTimelineItem != nullptr)
        {
            acTimelineBranch* pBranch = pTimelineItem->parentBranch();

            if (pBranch && pBranch->IsVisible() == false)
            {
                showItem = false;
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


/////////////////////////////////////////////////////
// gpGPUTraceSummaryTable
gpGPUTraceSummaryTable::gpGPUTraceSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart)
    : gpTraceSummaryTable(pDataContainer, pSessionView, timelineAbsoluteStart)
{

}

bool gpGPUTraceSummaryTable::InitItems()
{
    bool rc = false;
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        m_allCallItemsMultiMap.clear();
        m_totalTimeMs = 0;
        m_numTotalCalls = 0;
        int queuesCount = m_pSessionDataContainer->GPUCallsContainersCount();
        // fill local array of SummaryInfo items
        APISummaryTraceInfo infoArray[SUMMARY_INFO_ARRAY_SIZE] = {};

        for (int i = 0; i < queuesCount; i++)
        {
            afProgressBarWrapper::instance().incrementProgressBar();
            QString queueName = m_pSessionDataContainer->GPUObjectName(i);
            int apiCount = m_pSessionDataContainer->QueueItemsCount(queueName);

            for (int i = 0; i < apiCount; i++)
            {
                // Get the current API item
                ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItem(queueName, i);
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
        if (queuesCount > 0)
        {
            rc = true;
            for (auto info : infoArray)
            {
                InsertSummaryInfoToMap(info);
            }
        }
    }
    return rc;
}

void gpGPUTraceSummaryTable::AddSessionItemToSummaryInfo(APISummaryTraceInfo& info, ProfileSessionDataItem* pItem, unsigned int apiId)
{
    GT_IF_WITH_ASSERT(pItem != nullptr && m_pTraceView != nullptr)
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


