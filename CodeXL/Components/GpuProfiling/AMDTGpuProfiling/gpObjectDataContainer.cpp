//------------------------------ gpObjectDataContainer.cpp ------------------------------

// TODO Note: the file is created to match Object data container to trace data container.  Most functionality is not required, need more cleanup.

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

#include <CL/cl.h>

// afProgressBarWrapper:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/gpObjectDataContainer.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>
#include <AMDTGpuProfiling/ProjectSettings.h>
#include <AMDTGpuProfiling/SymbolInfo.h>
#include <AMDTGpuProfiling/Util.h>

#ifdef GP_OBJECT_VIEW_ENABLE

gpObjectDataContainer::gpObjectDataContainer() :
    m_apiCount(0)
{
    m_sessionTimeRange.first = std::numeric_limits<quint64>::max();
    m_sessionTimeRange.second = std::numeric_limits<quint64>::min();
}


gpObjectDataContainer::~gpObjectDataContainer()
{
    auto iter = m_threadToRootItemMap.begin();

    for (; iter != m_threadToRootItemMap.end(); iter++)
    {
        delete(*iter);
    }

    foreach (ProfileSessionDataItem* pItem, m_sessionGPUDataItems)
    {
        delete pItem;
    }

    m_sessionCPUDataItems.clear();
    m_sessionGPUDataItems.clear();
}

void gpObjectDataContainer::SetAPINum(osThreadId threadId, unsigned int apiNum)
{
    m_apiCountMap[threadId] = apiNum;
}

ProfileSessionDataItem* gpObjectDataContainer::AddCLItem(CLAPIInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        osThreadId tid = pAPIInfo->m_tid;

        // Set this thread API type
        if (!m_threadAPIType.contains(tid))
        {
            m_threadAPIType[tid] = ProfileSessionDataItem::CL_API_PROFILE_ITEM;
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpObjectDataContainer::AddHSAItem(HSAAPIInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        osThreadId tid = pAPIInfo->m_tid;

        // Set this thread API type
        if (!m_threadAPIType.contains(tid))
        {
            m_threadAPIType[tid] = ProfileSessionDataItem::HSA_API_PROFILE_ITEM;
        }
    }

    return pRetVal;
}


ProfileSessionDataItem* gpObjectDataContainer::AddDX12APIItem(DX12APIInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        osThreadId tid = pAPIInfo->m_tid;

        // Set this thread API type
        if (!m_threadAPIType.contains(tid))
        {
            m_threadAPIType[tid] = ProfileSessionDataItem::DX12_API_PROFILE_ITEM;
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpObjectDataContainer::AddVKAPIItem(VKAPIInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        osThreadId tid = pAPIInfo->m_tid;

        // Set this thread API type
        if (!m_threadAPIType.contains(tid))
        {
            m_threadAPIType[tid] = ProfileSessionDataItem::DX12_API_PROFILE_ITEM;
        }
    }

    return pRetVal;
}

SymbolInfo* gpObjectDataContainer::GetSymbolInfo(int threadId, int callIndex)
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

ProfileSessionDataItem::ProfileItemAPIType gpObjectDataContainer::APITypeForThread(osThreadId tid)
{
    ProfileSessionDataItem::ProfileItemAPIType retVal = ProfileSessionDataItem::API_PROFILE_ITEM_UNKNOWN;

    if (m_threadAPIType.contains(tid))
    {
        retVal = m_threadAPIType[tid];
    }

    return retVal;
}


int gpObjectDataContainer::ThreadsCount() const
{
    int retVal = 0;

    retVal = m_apiCountMap.size();

    return retVal;
}

int gpObjectDataContainer::DX12QueuesCount() const
{
    return m_sessionQueueToGPUDataItems.size();
}

osThreadId gpObjectDataContainer::ThreadID(int threadIndex) const
{
    osThreadId retVal = 0;

    auto iter = m_apiCountMap.begin();

    for (int i = 0; i < threadIndex; i++)
    {
        iter++;
    }

    if (iter != m_apiCountMap.end())
    {
        retVal = iter.key();
    }

    return retVal;
}

unsigned int gpObjectDataContainer::ThreadAPICount(osThreadId threadID)
{
    unsigned int retVal = 0;

    if (m_apiCountMap.find(threadID) != m_apiCountMap.end())
    {
        retVal = m_apiCountMap[threadID];
    }

    return retVal;
}

int gpObjectDataContainer::ThreadPerfMarkersCount(osThreadId threadID) const
{
    int retVal = 0;

    if (m_sessionPerformanceMarkers.find(threadID) != m_sessionPerformanceMarkers.end())
    {
        retVal = m_sessionPerformanceMarkers[threadID].size();
    }

    return retVal;
}


int gpObjectDataContainer::GPUItemsCount() const
{
    int retVal = 0;

    retVal = m_sessionGPUDataItems.size();

    return retVal;
}

ProfileSessionDataItem* gpObjectDataContainer::APIItem(osThreadId threadID, int apiItemIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_sessionCPUDataItems.contains(threadID))
    {
        const QList<ProfileSessionDataItem*>& apisList = m_sessionCPUDataItems[threadID];
        GT_IF_WITH_ASSERT((apiItemIndex >= 0) && (apiItemIndex < (int)apisList.size()))
        {
            pRetVal = apisList[apiItemIndex];
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpObjectDataContainer::PerfMarkerItem(osThreadId threadID, int markerIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT(m_sessionPerformanceMarkers.contains(threadID))
    {
        const QList<ProfileSessionDataItem*>& markersList = m_sessionPerformanceMarkers[threadID];
        GT_IF_WITH_ASSERT((markerIndex >= 0) && (markerIndex < (int)markersList.size()))
        {
            pRetVal = markersList[markerIndex];
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpObjectDataContainer::GpuItem(int gpuItemIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((gpuItemIndex >= 0) && (gpuItemIndex < (int)m_sessionGPUDataItems.size()))
    {
        pRetVal = m_sessionGPUDataItems[gpuItemIndex];
    }

    return pRetVal;
}

ProfileSessionDataItem* gpObjectDataContainer::GetRootItem(osThreadId threadID)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    if (m_threadToRootItemMap.contains(threadID))
    {
        pRetVal = m_threadToRootItemMap[threadID];
    }

    return pRetVal;
}

bool gpObjectDataContainer::DoesThreadContainPerformanceMarkers(osThreadId threadID)
{
    bool retVal = false;

    if (m_sessionPerformanceMarkers.contains(threadID))
    {
        retVal = !m_sessionPerformanceMarkers[threadID].isEmpty();
    }

    return retVal;
}

void gpObjectDataContainer::FinalizeDataCollection()
{
    /// Map from thread id to API count
    auto threadsIter = m_apiCountMap.begin();

    for (; threadsIter != m_apiCountMap.end(); threadsIter++)
    {
        osThreadId tid = threadsIter.key();

        if (m_sessionPerformanceMarkers.contains(tid))
        {
            // The data structure is a tree, and we should merge the API functions and performance markers to a single tree
            //MergePerformanceCountersForThread(tid);
        }
    }
}

QString gpObjectDataContainer::QueueName(int queueIndex)
{
    QString retVal;
    int index = 0;

    auto iter = m_sessionQueueToGPUDataItems.begin();

    for (; iter != m_sessionQueueToGPUDataItems.end(); iter++)
    {
        if (index == queueIndex)
        {
            std::string queue = iter.key();
            retVal = QString::fromStdString(queue);
            break;
        }
    }

    return retVal;
}

int gpObjectDataContainer::QueueItemsCount(const QString& queueName)
{
    int retVal = 0;
    std::string queueNameStr = queueName.toUtf8().constData();

    if (m_sessionQueueToGPUDataItems.contains(queueNameStr))
    {
        retVal = m_sessionQueueToGPUDataItems[queueNameStr].size();
    }

    return retVal;
}

ProfileSessionDataItem* gpObjectDataContainer::QueueItem(const QString& queueName, int apiItemIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;
    std::string queueNameStr = queueName.toUtf8().constData();

    // Sanity check:
    GT_IF_WITH_ASSERT(m_sessionQueueToGPUDataItems.contains(queueNameStr))
    {
        const QList<ProfileSessionDataItem*>& apisList = m_sessionQueueToGPUDataItems[queueNameStr];
        GT_IF_WITH_ASSERT((apiItemIndex >= 0) && (apiItemIndex < (int)apisList.size()))
        {
            pRetVal = apisList[apiItemIndex];
        }
    }

    return pRetVal;

}

#endif
