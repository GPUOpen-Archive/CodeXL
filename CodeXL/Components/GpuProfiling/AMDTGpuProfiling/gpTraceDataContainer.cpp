//------------------------------ gpTraceDataContainer.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>
// boost
#include <boost/icl/split_interval_map.hpp>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

#include <CL/cl.h>

// afProgressBarWrapper:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>
#include <AMDTGpuProfiling/ProjectSettings.h>
#include <AMDTGpuProfiling/SymbolInfo.h>
#include <AMDTGpuProfiling/Util.h>

namespace boosticl = boost::icl;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include <d3d12.h>
#endif

gpTraceDataContainer::gpTraceDataContainer() :
    m_apiCount(0),
    m_sessionAPIType(ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
{
    m_sessionTimeRange.first = std::numeric_limits<quint64>::max();
    m_sessionTimeRange.second = std::numeric_limits<quint64>::min();

    m_lastFindResultStartTime = 0;
}


gpTraceDataContainer::~gpTraceDataContainer()
{
    auto cpuIter = m_threadToRootItemMap.begin();

    for (; cpuIter != m_threadToRootItemMap.end(); cpuIter++)
    {
        delete(*cpuIter);
    }

    auto gpuIter = m_sessionQueuesToCallsMap.begin();

    for (; gpuIter != m_sessionQueuesToCallsMap.end(); gpuIter++)
    {
        const QList<ProfileSessionDataItem*>& list = (*gpuIter);

        foreach (ProfileSessionDataItem* pItem, list)
        {
            delete pItem;
        }
    }

    foreach (ProfileSessionDataItem* pItem, m_sessionGPUDataItems)
    {
        delete pItem;
    }

    m_sessionCPUDataItems.clear();
    m_sessionGPUDataItems.clear();
    m_sessionQueuesToCallsMap.clear();
    m_threadToRootItemMap.clear();
}

void gpTraceDataContainer::SetAPINum(osThreadId threadId, unsigned int apiNum)
{
    m_apiCountMap[threadId] = apiNum;
}

ProfileSessionDataItem* gpTraceDataContainer::AddCLItem(CLAPIInfo* pAPIInfo)
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

        // Find the occupancy info for this API
        OccupancyInfo* pOccupancyInfo = FindOccupancyInfo(pAPIInfo);

        // Add the requested item to the thread root
        pRetVal = new ProfileSessionDataItem(this, pAPIInfo, pOccupancyInfo);
        AddItemToThread(pRetVal);

        // Add the item to the session items map
        m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

        // Initialize the container API type
        m_sessionAPIType = ProfileSessionDataItem::CL_API_PROFILE_ITEM;

    }

    return pRetVal;
}

ProfileSessionDataItem* gpTraceDataContainer::AddHSAItem(HSAAPIInfo* pAPIInfo)
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

        // Create an HSA profile item
        pRetVal = new ProfileSessionDataItem(this, pAPIInfo);

        // Add the item to the session items map
        m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

        // Add the item to the thread's root
        AddItemToThread(pRetVal);

        // Initialize the container API type
        m_sessionAPIType = ProfileSessionDataItem::HSA_API_PROFILE_ITEM;
    }

    return pRetVal;
}


ProfileSessionDataItem* gpTraceDataContainer::AddDX12APIItem(DX12APIInfo* pAPIInfo)
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

        // Create an DX12 profile item
        pRetVal = new ProfileSessionDataItem(this, pAPIInfo);

        // Add the item to the thread's root
        AddItemToThread(pRetVal);

        if (pAPIInfo->m_sampleId > 0)
        {
            m_sampleIdToCPUItemMap.insertMulti(pAPIInfo->m_sampleId, pRetVal);
        }

        // Add the item to the session items map
        m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

        // Initialize the container API type
        m_sessionAPIType = ProfileSessionDataItem::DX12_API_PROFILE_ITEM;

        // If this call has a sample id, add it to the list of calls
        if (pAPIInfo->m_sampleId > 0)
        {
            m_commandListUnAttachedCalls << pRetVal;
        }

        // Analyze the command list close API call
        if (pAPIInfo->m_apiId == FuncId_ID3D12GraphicsCommandList_Close)
        {
            // Close the command list
            CloseCommandList(pAPIInfo);
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpTraceDataContainer::AddDX12GPUTraceItem(DX12GPUTraceInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        // Create an HSA profile item
        pRetVal = new ProfileSessionDataItem(this, pAPIInfo);

        // Add the item to the queues map
        m_sessionQueuesToCallsMap[pAPIInfo->m_commandQueuePtrStr] << pRetVal;

        // Add a map from the queue name to the queue type
        if (m_sessionQueueNameToCommandListType.contains(pAPIInfo->m_commandQueuePtrStr))
        {
            GT_ASSERT(m_sessionQueueNameToCommandListType[pAPIInfo->m_commandQueuePtrStr] == pAPIInfo->m_commandListType);
        }
        else
        {
            m_sessionQueueNameToCommandListType[pAPIInfo->m_commandQueuePtrStr] = pAPIInfo->m_commandListType;
        }

        if (pAPIInfo->m_sampleId > 0)
        {
            m_sampleIdToGPUItemMap.insertMulti(pAPIInfo->m_sampleId, pRetVal);
        }

        // Add the item to the session items map
        m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

        // Add this GPU call to the relevant command list instance (according to it's sample id)
        QString commandListInstanceName = AddGPUCallToCommandList(pAPIInfo);
        pRetVal->SetColumnData(ProfileSessionDataItem::SESSION_ITEM_COMMAND_LIST_COLUMN, commandListInstanceName);

    }

    return pRetVal;
}

ProfileSessionDataItem* gpTraceDataContainer::AddVKAPIItem(VKAPIInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        osThreadId tid = pAPIInfo->m_tid;

        // Set this thread API type
        if (!m_threadAPIType.contains(tid))
        {
            m_threadAPIType[tid] = ProfileSessionDataItem::VK_API_PROFILE_ITEM;
        }

        // Create an HSA profile item
        pRetVal = new ProfileSessionDataItem(this, pAPIInfo);

        // Add the item to the thread's root
        AddItemToThread(pRetVal);

        // Add the item to the session items map
        m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

        // Initialize the container API type
        m_sessionAPIType = ProfileSessionDataItem::VK_API_PROFILE_ITEM;

        // If this call has a sample id, add it to the list of calls
        if (pAPIInfo->m_sampleId > 0)
        {
            m_commandListUnAttachedCalls << pRetVal;
        }

        // Analyze the command list close API call
        if (pAPIInfo->m_apiId == FuncId_vkEndCommandBuffer)
        {
            // Close the command list
            CloseCommandList(pAPIInfo);
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpTraceDataContainer::AddVKGPUTraceItem(VKGPUTraceInfo* pAPIInfo)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        if (pAPIInfo->m_strName == GPU_STR_TraceViewWholeBufferTraceStr)
        {
            // Do not add WholeBuffer commands as GPU api calls
        }
        else
        {
            // Create a Vulkan profile item
            pRetVal = new ProfileSessionDataItem(this, pAPIInfo);

            // Add the item to the session items map
            m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

            // Add the item to the queues map
            m_sessionQueuesToCallsMap[pAPIInfo->m_queueIndexStr] << pRetVal;

            // Add a map from the queue name to the queue type
            if (m_sessionQueueNameToCommandListType.contains(pAPIInfo->m_queueIndexStr))
            {
                GT_ASSERT(m_sessionQueueNameToCommandListType[pAPIInfo->m_queueIndexStr] == pAPIInfo->m_commandListType);
            }
            else
            {
                m_sessionQueueNameToCommandListType[pAPIInfo->m_queueIndexStr] = pAPIInfo->m_commandListType;
            }

            // Add this GPU call to the relevant command list instance (according to it's sample id)
            QString commandListInstanceName = AddGPUCallToCommandList(pAPIInfo);
            pRetVal->SetColumnData(ProfileSessionDataItem::SESSION_ITEM_COMMAND_LIST_COLUMN, commandListInstanceName);

        }
    }

    return pRetVal;
}


void gpTraceDataContainer::AddPerformanceMarker(PerfMarkerEntry* pPerfMarkerEntry)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pPerfMarkerEntry != nullptr)
    {
        osThreadId tid = pPerfMarkerEntry->m_tid;

        if (pPerfMarkerEntry->m_markerType == PerfMarkerEntry::PerfMarkerType_Begin)
        {
            pRetVal = new ProfileSessionDataItem(this, pPerfMarkerEntry);

            // Add the item to the session items map
            m_sessionItemsSortedByStartTime.insertMulti(pRetVal->StartTime(), pRetVal);

            m_sessionPerformanceMarkers[tid] << pRetVal;

            m_openedMarkers.push(pRetVal);
        }
        else if (pPerfMarkerEntry->m_markerType == PerfMarkerEntry::PerfMarkerType_End)
        {
            // Sanity check - we should only get here with opened markers
            GT_IF_WITH_ASSERT(m_openedMarkers.top() != nullptr)
            {
                ProfileSessionDataItem* pMarkerProfileItem = m_openedMarkers.pop();

                // Sanity check:
                GT_IF_WITH_ASSERT(pMarkerProfileItem != nullptr)
                {
                    pMarkerProfileItem->SetEndTime(pPerfMarkerEntry->m_timestamp);

                    QPair<quint64, quint64> range;
                    range.first = pMarkerProfileItem->StartTime();
                    range.second = pMarkerProfileItem->EndTime();
                    m_perfMarkersTraceItemsMap[range] = pMarkerProfileItem;

                    // Update the session start + end times
                    m_sessionTimeRange.first = qMin(m_sessionTimeRange.first, pMarkerProfileItem->StartTime());
                    m_sessionTimeRange.second = qMax(m_sessionTimeRange.second, pMarkerProfileItem->EndTime());
                }
            }
        }
    }
}

void gpTraceDataContainer::AddCLGPUItem(ProfileSessionDataItem* pAPIOwnerItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIOwnerItem != nullptr)
    {
        ProfileSessionDataItem* pNewItem = new ProfileSessionDataItem(this, pAPIOwnerItem, ProfileSessionDataItem::CL_GPU_PROFILE_ITEM);
        m_sessionGPUDataItems << pNewItem;

        // Add the item to the session items map
        m_sessionItemsSortedByStartTime.insertMulti(pNewItem->StartTime(), pNewItem);

        // Update the session start + end times
        m_sessionTimeRange.first = qMin(m_sessionTimeRange.first, pNewItem->StartTime());
        m_sessionTimeRange.second = qMax(m_sessionTimeRange.second, pNewItem->EndTime());
    }
}

void gpTraceDataContainer::AddHSAGPUItem(HSAAPIInfo* pAPIInfo)
{
    ProfileSessionDataItem::ProfileItemType itemType(ProfileSessionDataItem::HSA_GPU_PROFILE_ITEM);
    ProfileSessionDataItem* pNewItem = new ProfileSessionDataItem(this, pAPIInfo, itemType);
    m_sessionGPUDataItems << pNewItem;

    // Add the item to the session items map
    m_sessionItemsSortedByStartTime.insertMulti(pNewItem->StartTime(), pNewItem);

    // Update the session start + end times
    m_sessionTimeRange.first = qMin(m_sessionTimeRange.first, pNewItem->StartTime());
    m_sessionTimeRange.second = qMax(m_sessionTimeRange.second, pNewItem->EndTime());
}


SymbolInfo* gpTraceDataContainer::GetSymbolInfo(int threadId, int callIndex)
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
OccupancyInfo* gpTraceDataContainer::FindOccupancyInfo(CLAPIInfo* pAPIInfo)
{
    OccupancyInfo* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        // This is the index of the occupancy item found. If we do find the occupancy item,
        // we increase this variable, so that the next time the user will ask of occupancy data,
        // we will give him the next item
        int occupancyIndex = 0;

        if (!m_occupancyInfoMap.isEmpty())
        {
            const QList<OccupancyInfo*> tempInfosList = m_occupancyInfoMap[pAPIInfo->m_tid];

            if (m_oclThreadOccIndexMap.contains(pAPIInfo->m_tid))
            {
                occupancyIndex = m_oclThreadOccIndexMap[pAPIInfo->m_tid];
            }

            QString deviceNameStr;

            // If this is an enqueue api
            if ((pAPIInfo->m_Type & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API)
            {
                unsigned int apiID = pAPIInfo->m_uiAPIID;
                CLEnqueueAPI* pEnqueueApiInfo = dynamic_cast<CLEnqueueAPI*>(pAPIInfo);
                GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
                {
                    quint64 gpuStart = pEnqueueApiInfo->m_ullRunning;
                    quint64 gpuEnd = pEnqueueApiInfo->m_ullComplete;
                    quint64 gpuQueued = pEnqueueApiInfo->m_ullQueue;
                    quint64 gpuSubmit = pEnqueueApiInfo->m_ullSubmit;

                    deviceNameStr = QString::fromStdString(pEnqueueApiInfo->m_strDevice);

                    if ((gpuEnd < gpuStart && (apiID < CL_FUNC_TYPE_clEnqueueMapBuffer || apiID > CL_FUNC_TYPE_clEnqueueUnmapMemObject)) || gpuStart < gpuSubmit || gpuSubmit < gpuQueued)
                    {
                        if (pEnqueueApiInfo->m_uiCMDType <= CL_COMMAND_TASK && (deviceNameStr != GPU_STR_TraceViewCpuDevice))
                        {
                            // if this is a kernel dispatch without valid timestamps, on a non-CPU device, then bump the
                            // occIndex so that subsequent dispatches are matched up with the correct occupancy info
                            // This fixes BUG355468
                            occupancyIndex++;
                        }
                    }

                    if ((pEnqueueApiInfo->m_Type & CL_ENQUEUE_KERNEL) == CL_ENQUEUE_KERNEL)
                    {
                        if ((occupancyIndex < tempInfosList.count()) && Util::CheckOccupancyDeviceName(tempInfosList[occupancyIndex]->GetDeviceName(), deviceNameStr))
                        {
                            pRetVal = tempInfosList[occupancyIndex];
                            occupancyIndex++;
                        }
                    }

                    // Set the thread last taken occupancy index
                    m_oclThreadOccIndexMap[pAPIInfo->m_tid] = occupancyIndex;
                }
            }

        }
    }

    return pRetVal;
}

ProfileSessionDataItem::ProfileItemAPIType gpTraceDataContainer::APITypeForThread(osThreadId tid)
{
    ProfileSessionDataItem::ProfileItemAPIType retVal = ProfileSessionDataItem::API_PROFILE_ITEM_UNKNOWN;

    if (m_threadAPIType.contains(tid))
    {
        retVal = m_threadAPIType[tid];
    }

    return retVal;
}


int gpTraceDataContainer::ThreadsCount() const
{
    int retVal = 0;

    retVal = m_apiCountMap.size();

    return retVal;
}

int gpTraceDataContainer::GPUCallsContainersCount() const
{
    return m_sessionQueuesToCallsMap.size();
}

osThreadId gpTraceDataContainer::ThreadID(int threadIndex) const
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

unsigned int gpTraceDataContainer::ThreadAPICount(osThreadId threadID)
{
    unsigned int retVal = 0;

    if (m_apiCountMap.find(threadID) != m_apiCountMap.end())
    {
        retVal = m_apiCountMap[threadID];
    }

    return retVal;
}

int gpTraceDataContainer::ThreadPerfMarkersCount(osThreadId threadID) const
{
    int retVal = 0;

    if (m_sessionPerformanceMarkers.find(threadID) != m_sessionPerformanceMarkers.end())
    {
        retVal = m_sessionPerformanceMarkers[threadID].size();
    }

    return retVal;
}


int gpTraceDataContainer::QueueItemsCount() const
{
    int retVal = 0;

    retVal = m_sessionGPUDataItems.size();

    return retVal;
}

ProfileSessionDataItem* gpTraceDataContainer::APIItem(osThreadId threadID, int apiItemIndex) const
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


ProfileSessionDataItem* gpTraceDataContainer::PerfMarkerItem(osThreadId threadID, int markerIndex) const
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


ProfileSessionDataItem* gpTraceDataContainer::GpuItem(int gpuItemIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT((gpuItemIndex >= 0) && (gpuItemIndex < (int)m_sessionGPUDataItems.size()))
    {
        pRetVal = m_sessionGPUDataItems[gpuItemIndex];
    }

    return pRetVal;
}

ProfileSessionDataItem* gpTraceDataContainer::GetRootItem(osThreadId threadID)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    if (m_threadToRootItemMap.contains(threadID))
    {
        pRetVal = m_threadToRootItemMap[threadID];
    }

    return pRetVal;
}

void gpTraceDataContainer::AddItemToThread(ProfileSessionDataItem* pThreadItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pThreadItem != nullptr)
    {
        m_sessionCPUDataItems[pThreadItem->ThreadID()] << pThreadItem;

        ProfileSessionDataItem* pThreadRootItem = GetRootItem(pThreadItem->ThreadID());

        if (pThreadRootItem == nullptr)
        {
            pThreadRootItem = new ProfileSessionDataItem(this, pThreadItem->ItemType().m_itemMainType);

            // Add the root item to the map
            m_threadToRootItemMap[pThreadItem->ThreadID()] = pThreadRootItem;
        }

        // Update the API count
        m_apiCount++;

        // Sanity check:
        GT_IF_WITH_ASSERT(pThreadRootItem != nullptr)
        {
            pThreadRootItem->AppendChild(pThreadItem);
        }

        // Update the session start + end times
        m_sessionTimeRange.first = qMin(m_sessionTimeRange.first, pThreadItem->StartTime());
        m_sessionTimeRange.second = qMax(m_sessionTimeRange.second, pThreadItem->EndTime());
    }
}

bool gpTraceDataContainer::DoesThreadContainPerformanceMarkers(osThreadId threadID)
{
    bool retVal = false;

    if (m_sessionPerformanceMarkers.contains(threadID))
    {
        retVal = !m_sessionPerformanceMarkers[threadID].isEmpty();
    }

    return retVal;
}

void gpTraceDataContainer::FinalizeDataCollection()
{
    /// Map from thread id to API count
    auto threadsIter = m_apiCountMap.begin();

    for (; threadsIter != m_apiCountMap.end(); threadsIter++)
    {
        osThreadId tid = threadsIter.key();

        if (m_sessionPerformanceMarkers.contains(tid))
        {
            // The data structure is a tree, and we should merge the API functions and performance markers to a single tree
            MergePerformanceCountersForThread(tid);
        }
    }
}

void gpTraceDataContainer::MergePerformanceCountersForThread(osThreadId tid)
{
    // Get the list of API functions for this thread
    QList<ProfileSessionDataItem*> apisList = m_sessionCPUDataItems[tid];

    // Go through the marker items, and the API items in parallel, and add it to the table:
    auto apiIter = apisList.begin();
    auto apiIterEnd = apisList.end();

    auto markersIter = m_perfMarkersTraceItemsMap.begin();
    auto markersIterEnd = m_perfMarkersTraceItemsMap.end();

    afProgressBarWrapper::instance().setProgressText(GPU_STR_TraceViewLoadingTraceTableItemsProgress);

    QStack<ProfileSessionDataItem*> openedMarkersItems;

    // Get the root item for this thread
    ProfileSessionDataItem* pRootItem = GetRootItem(tid);

    // Sanity check:
    GT_IF_WITH_ASSERT(pRootItem != nullptr)
    {
        // Clear the list of children, and build it again with timestamps
        pRootItem->RemoveAllChildren();

        // Go over the API items and the markers item, and add them to the table:
        while ((apiIter != apiIterEnd) || (markersIter != markersIterEnd))
        {
            bool isAddingMarker = false;
            ProfileSessionDataItem* pNextItemToAdd = nullptr;
            ProfileSessionDataItem* pCurrentAPI = (apiIter != apiIterEnd) ? (*apiIter) : nullptr;
            ProfileSessionDataItem* pCurrentMarker = (markersIter != markersIterEnd) ? markersIter.value() : nullptr;

            // Save the current item start time, for later decision of the item's parent:
            quint64 currentStart = 0;
            quint64 currentEnd = 0;

            if ((pCurrentAPI != nullptr) && (pCurrentMarker == nullptr))
            {
                pNextItemToAdd = pCurrentAPI;
                currentStart = pCurrentAPI->StartTime();
                currentEnd = pCurrentAPI->EndTime();
                apiIter++;
            }
            else if ((pCurrentAPI == nullptr) && (pCurrentMarker != nullptr))
            {
                pNextItemToAdd = pCurrentMarker;
                currentStart = markersIter.key().first;
                currentEnd = markersIter.key().second;
                markersIter++;
                isAddingMarker = true;
            }
            else
            {
                // We have both next API and markers. Compare the start time to decide which of them should be added next:
                quint64 apiStart = pCurrentAPI->StartTime();
                quint64 apiEnd = pCurrentAPI->EndTime();
                quint64 markerStart = markersIter.key().first;
                quint64 markerEnd = markersIter.key().second;

                // Next item should be an API item:
                if (apiStart <= markerStart)
                {
                    pNextItemToAdd = pCurrentAPI;
                    currentStart = apiStart;
                    currentEnd = apiEnd;
                    apiIter++;
                }

                // Next one should be a marker item:
                else
                {
                    pNextItemToAdd = pCurrentMarker;
                    currentStart = markerStart;
                    currentEnd = markerEnd;
                    markersIter++;
                    isAddingMarker = true;
                }
            }

            // Look for the right parent for this item:
            ProfileSessionDataItem* pParent = pRootItem;

            if (!openedMarkersItems.empty())
            {
                QStack<ProfileSessionDataItem*>::iterator iter = openedMarkersItems.begin();
                int itemsToPop = 0;

                while (iter != openedMarkersItems.end())
                {
                    quint64 currentOpenedMarkerEndTime = (*iter)->EndTime();

                    if (currentOpenedMarkerEndTime < currentStart)
                    {
                        itemsToPop++;
                    }

                    iter++;
                }

                // Pop the markers items that should be closed, and update it's API indices:
                for (int i = 0; i < itemsToPop; i++)
                {
                    // Pop the item from the parents stack:
                    openedMarkersItems.pop();
                }

                if (!openedMarkersItems.empty())
                {
                    pParent = openedMarkersItems.top();
                }
            }

            if (isAddingMarker)
            {
                openedMarkersItems.push(pNextItemToAdd);
            }

            GT_IF_WITH_ASSERT((pParent != nullptr) && (pNextItemToAdd != nullptr))
            {
                // Add the child to the appropriate parent:
                pParent->AppendChild(pNextItemToAdd);

                if (pParent != pRootItem)
                {
                    pParent->UpdateIndices(pNextItemToAdd->APICallIndex(), pNextItemToAdd->EndIndex());
                }

                afProgressBarWrapper::instance().incrementProgressBar();
            }
        }
    }
}

QString gpTraceDataContainer::GPUObjectName(int gpuObjectIndex)
{
    QString retVal;

    // Iterate the map and find the queue with the requested index
    auto iter = m_sessionQueuesToCallsMap.begin();

    for (int index = 0; iter != m_sessionQueuesToCallsMap.end(); iter++, index++)
    {
        if (index == gpuObjectIndex)
        {
            std::string queue = iter.key();
            retVal = QString::fromStdString(queue);
            break;
        }
    }

    return retVal;
}


int gpTraceDataContainer::QueueType(const QString& queueName)
{
    int retVal = -1;

    if (m_sessionQueueNameToCommandListType.contains(queueName.toStdString()))
    {
        retVal = m_sessionQueueNameToCommandListType[queueName.toStdString()];
    }

    return retVal;
}

int gpTraceDataContainer::QueueItemsCount(const QString& queueName)
{
    int retVal = 0;
    std::string queueNameStr = queueName.toUtf8().constData();

    if (m_sessionQueuesToCallsMap.contains(queueNameStr))
    {
        retVal = m_sessionQueuesToCallsMap[queueNameStr].size();
    }

    return retVal;
}

ProfileSessionDataItem* gpTraceDataContainer::QueueItem(const QString& queueName, int apiItemIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;
    std::string gpuObjectNameStr = queueName.toUtf8().constData();

    // Sanity check:
    GT_IF_WITH_ASSERT(m_sessionQueuesToCallsMap.contains(gpuObjectNameStr))
    {
        const QList<ProfileSessionDataItem*>& apisList = m_sessionQueuesToCallsMap[gpuObjectNameStr];
        GT_IF_WITH_ASSERT((apiItemIndex >= 0) && (apiItemIndex < (int)apisList.size()))
        {
            pRetVal = apisList[apiItemIndex];
        }
    }

    return pRetVal;

}

ProfileSessionDataItem* gpTraceDataContainer::QueueItemByItemCallIndex(const QString& queueName, int apiItemIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;
    std::string queueNameStr = queueName.toUtf8().constData();

    // Sanity check:
    GT_IF_WITH_ASSERT(m_sessionQueuesToCallsMap.contains(queueNameStr))
    {
        const QList<ProfileSessionDataItem*>& apisList = m_sessionQueuesToCallsMap[queueNameStr];
        GT_IF_WITH_ASSERT(apiItemIndex >= 0)
        {
            for (auto pItem : apisList)
            {
                if (pItem->APICallIndex() == apiItemIndex)
                {
                    pRetVal = pItem;
                    break;
                }
            }
        }
    }
    return pRetVal;
}

void gpTraceDataContainer::GetCPUItemsBySampleId(int sampleId, QList<ProfileSessionDataItem*>& items)const
{
    items = m_sampleIdToCPUItemMap.values(sampleId);
}

void gpTraceDataContainer::GetGPUItemsBySampleId(int sampleId, QList<ProfileSessionDataItem*>& items)const
{
    items = m_sampleIdToGPUItemMap.values(sampleId);
}

QString gpTraceDataContainer::CommandListTypeAsString(int commandListType)
{
    QString retVal;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    D3D12_COMMAND_LIST_TYPE type = (D3D12_COMMAND_LIST_TYPE)commandListType;

    switch (type)
    {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
        {
            retVal = GP_Str_CommandListTypeDirect;
            break;
        }

        case D3D12_COMMAND_LIST_TYPE_BUNDLE:
        {
            retVal = GP_Str_CommandListTypeBundle;
            break;
        }

        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        {
            retVal = GP_Str_CommandListTypeCompute;
            break;
        }

        case D3D12_COMMAND_LIST_TYPE_COPY:
        {
            retVal = GP_Str_CommandListTypeCopy;
            break;
        }

        default:
            GT_ASSERT_EX(false, L"Unsupported list type");
            break;
    }

#endif

    return retVal;
}

ProfileSessionDataItem* gpTraceDataContainer::FindItem(const QString& findStr, bool isCaseSensitive)
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Find the first item to start with
    auto iter = m_sessionItemsSortedByStartTime.begin();
    auto iterEnd = m_sessionItemsSortedByStartTime.end();
    m_lastFindResultStartTime = 0;
    for (; iter != iterEnd; iter++)
    {
        if (iter.value()->DoesStringMatch(findStr, isCaseSensitive))
        {
            m_lastFindResultStartTime = iter.key();
            pRetVal = iter.value();
            break;
        }
    }

    return pRetVal;
}

ProfileSessionDataItem* gpTraceDataContainer::FindNextItem(const QString& findStr, bool isCaseSensitive)
{
    ProfileSessionDataItem* pRetVal = nullptr;
    if (m_lastStringSearched != findStr)
    {
        m_lastStringSearched = findStr;
        m_lastFindResultStartTime = 0;
    }
    // Find the first item to start with
    auto iter = m_sessionItemsSortedByStartTime.upperBound(m_lastFindResultStartTime);
    auto iterEnd = m_sessionItemsSortedByStartTime.end();

    for (; iter != iterEnd; iter++)
    {
        if (iter.value()->DoesStringMatch(findStr, isCaseSensitive))
        {
            m_lastFindResultStartTime = iter.key();
            pRetVal = iter.value();
            break;
        }
    }
    //failed to find, try from beginning next time
    if (pRetVal == nullptr)
    {
        m_lastFindResultStartTime = 0;
    }

    return pRetVal;
}

void gpTraceDataContainer::CloseCommandList(APIInfo* pAPIInfo)
{
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        bool isVK = (m_sessionAPIType == ProfileSessionDataItem::VK_API_PROFILE_ITEM);
        bool isDX = (m_sessionAPIType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM);
        if (isVK || isDX)
        {
            // Get the command list pointer
            QString commandListPtr;
            if (isVK)
            {
                VKGPUTraceInfo* pVKAPIInfo = (VKGPUTraceInfo*)pAPIInfo;

                // In Vulkan, we expect the command buffer pointer to be the end command list function argument
                commandListPtr = QString::fromStdString(pVKAPIInfo->m_ArgList);
            }
            else if (isDX)
            {
                DX12GPUTraceInfo* pDXAPIInfo = (DX12GPUTraceInfo*)pAPIInfo;

                // In DX12 the command list pointer is the function interface pointer
                commandListPtr = QString::fromStdString(pDXAPIInfo->m_interfacePtrStr);
            }

            // Count other command list instance with the same command list pointer
            int newCommandListIndex = -1;
            for (int i = 0; i < m_commandListInstancesVector.size(); i++)
            {
                if (m_commandListInstancesVector[i].m_commandListPtr == commandListPtr)
                {
                    // If there only was one instance so far, make sure that the already existing command list is indexed
                    if (m_commandListInstancesVector[i].m_instanceIndex < 0)
                    {
                        m_commandListInstancesVector[i].m_instanceIndex = 1;
                    }

                    // Find the next available index for the new instance
                    newCommandListIndex = qMax(newCommandListIndex, m_commandListInstancesVector[i].m_instanceIndex + 1);
                }
            }

            // Add new command list instance to the vector
            CommandListInstanceData currentInstanceData;
            currentInstanceData.m_instanceIndex = newCommandListIndex;
            currentInstanceData.m_commandListPtr = commandListPtr;

            // This list accumulates all items that should be removed from m_commandListUnAttachedCalls after the iteration
            QList<ProfileSessionDataItem*> itemsToRemove;

            // Go through all the calls, and check if the command list pointer is in it's parameter's list
            for (auto iter = m_commandListUnAttachedCalls.begin(); iter != m_commandListUnAttachedCalls.end(); iter++)
            {
                ProfileSessionDataItem* pCurrentItem = *iter;
                if (pCurrentItem != nullptr)
                {
                    QString params = pCurrentItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN).toString();
                    QString interfacePtr = pCurrentItem->InterfacePtr();
                    if ((interfacePtr == commandListPtr) || params.contains(commandListPtr))
                    {
                        // Add this sample id to the list of samples for this instance, and remove the item from the unattached list
                        currentInstanceData.m_sampleIds << pCurrentItem->SampleId();
                        itemsToRemove << pCurrentItem;
                        currentInstanceData.m_commandListQueueName = m_commandListToQueueMap[commandListPtr];
                    }
                }
            }

            foreach(ProfileSessionDataItem* pItem, itemsToRemove)
            {
                m_commandListUnAttachedCalls.removeOne(pItem);
            }

            m_commandListInstancesVector << currentInstanceData;
        }
    }
}

QString gpTraceDataContainer::AddGPUCallToCommandList(APIInfo* pAPIInfo)
{
    QString retVal;

    // Sanity check:
    GT_IF_WITH_ASSERT(pAPIInfo != nullptr)
    {
        // Find the relevant information for this call
        int sampleID = -1;
        QString commandListName, queueName;

        bool isVK = (m_sessionAPIType == ProfileSessionDataItem::VK_API_PROFILE_ITEM);
        bool isDX = (m_sessionAPIType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM);
        if (isVK || isDX)
        {
            if (isVK)
            {
                VKGPUTraceInfo* pVKAPIInfo = (VKGPUTraceInfo*)pAPIInfo;
                sampleID = pVKAPIInfo->m_sampleId;
                commandListName = QString::fromStdString(pVKAPIInfo->m_commandBufferHandleStr);
                queueName = QString::fromStdString(pVKAPIInfo->m_queueIndexStr);
            }
            else if (isDX)
            {
                DX12GPUTraceInfo* pDXAPIInfo = (DX12GPUTraceInfo*)pAPIInfo;
                sampleID = pDXAPIInfo->m_sampleId;
                commandListName = QString::fromStdString(pDXAPIInfo->m_commandListPtrStr);
                queueName = QString::fromStdString(pDXAPIInfo->m_commandQueuePtrStr);
            }


            // Add this command list to the queue names map. Make sure that the command list was not added with a different name before
            if (m_commandListToQueueMap.contains(commandListName) && !m_commandListToQueueMap[commandListName].isEmpty())
            {
                GT_ASSERT_EX(m_commandListToQueueMap[commandListName] == queueName, L"This command list was already added with another queue name");
            }
            else
            {
                m_commandListToQueueMap.insertMulti(commandListName, queueName);
            }

            // Find the command list data for which this GPU call should be added
            // The right command list instance is the one containing this sample id
            int commandListInstnaceIndex = -1;
            for (int i = 0; i < (int)m_commandListInstancesVector.size(); i++)
            {
                if (m_commandListInstancesVector[i].m_sampleIds.contains(sampleID))
                {
                    commandListInstnaceIndex = i;
                    break;
                }
            }

            int commandListIndex = -1;
            QString commandListPtr = commandListName;

            // The sample id should be contained in one of the lists
            if ((commandListInstnaceIndex >= 0) && (commandListInstnaceIndex < m_commandListInstancesVector.size()))
            {
                // Update the existing command list data with the current API call data
                m_commandListInstancesVector[commandListInstnaceIndex].m_commandListQueueName = queueName;
                m_commandListInstancesVector[commandListInstnaceIndex].m_apiIndices.push_back(pAPIInfo->m_uiSeqID);

                if ((m_commandListInstancesVector[commandListInstnaceIndex].m_startTime == std::numeric_limits<quint64>::max()) || (m_commandListInstancesVector[commandListInstnaceIndex].m_startTime > pAPIInfo->m_ullStart))
                {
                    m_commandListInstancesVector[commandListInstnaceIndex].m_startTime = pAPIInfo->m_ullStart;
                }

                if ((m_commandListInstancesVector[commandListInstnaceIndex].m_endTime == std::numeric_limits<quint64>::min()) || (m_commandListInstancesVector[commandListInstnaceIndex].m_endTime < pAPIInfo->m_ullEnd))
                {
                    m_commandListInstancesVector[commandListInstnaceIndex].m_endTime = pAPIInfo->m_ullEnd;
                }

                commandListIndex = m_commandListInstancesVector[commandListInstnaceIndex].m_instanceIndex;
                commandListPtr = m_commandListInstancesVector[commandListInstnaceIndex].m_commandListPtr;
            }

            // Build containing command list instance name
            retVal = CommandListNameFromPointer(commandListPtr, commandListIndex);
        }
    }

    return retVal;
}

QString gpTraceDataContainer::CommandListNameFromPointer(const QString& commandBufferPtrStr, int instanceIndex)
{
    QString retVal;

    int commandListIndex = 1;
    // If the command buffer / list is not mapped to an index yet, allocate an index for the current command list
    if (!m_commandListPointerToIndexMap.contains(commandBufferPtrStr))
    {
        int index = m_commandListPointerToIndexMap.size() + 1;
        m_commandListPointerToIndexMap.insertMulti(commandBufferPtrStr, index);
    }

    // Get the index for this command list / buffer
    GT_IF_WITH_ASSERT(m_commandListPointerToIndexMap.contains(commandBufferPtrStr))
    {
        commandListIndex = m_commandListPointerToIndexMap[commandBufferPtrStr];
    }

    if (m_sessionAPIType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
    {
        if (instanceIndex > 0)
        {
            retVal = QString(GPU_STR_timeline_CmdListIsntanceBranchName).arg(commandListIndex).arg(instanceIndex);
        }
        else
        {
            retVal = QString(GPU_STR_timeline_CmdListBranchName).arg(commandListIndex);
        }
    }
    else
    {
        if (instanceIndex > 0)
        {
            retVal = QString(GPU_STR_timeline_CmdBufferInstanceBranchName).arg(commandListIndex).arg(instanceIndex);
        }
        else
        {
            retVal = QString(GPU_STR_timeline_CmdBufferBranchName).arg(commandListIndex);
        }
    }

    return retVal;
}

QString gpTraceDataContainer::GetContainingCommandList(APIInfo* pApiInfo)
{
    QString retVal;
    // Sanity check:
    GT_IF_WITH_ASSERT(pApiInfo != nullptr)
    {
        QString commandListPtr;
        int commandListIndex = -1;
        int apiIndex = pApiInfo->m_uiSeqID;

        bool isVK = (m_sessionAPIType == ProfileSessionDataItem::VK_API_PROFILE_ITEM);
        bool isDX = (m_sessionAPIType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM);
        if (isVK || isDX)
        {
            if (isVK)
            {
                VKGPUTraceInfo* pVKAPIInfo = (VKGPUTraceInfo*)pApiInfo;
                commandListPtr = QString::fromStdString(pVKAPIInfo->m_commandBufferHandleStr);
            }
            else if (isDX)
            {
                DX12GPUTraceInfo* pDXAPIInfo = (DX12GPUTraceInfo*)pApiInfo;
                commandListPtr = QString::fromStdString(pDXAPIInfo->m_commandListPtrStr);
            }

            int cmdListInstanceIndex = 0;
            for (int i = 0; i < m_commandListInstancesVector.size(); i++)
            {
                if (m_commandListInstancesVector[i].m_commandListPtr == commandListPtr)
                {
                    if (m_commandListInstancesVector[i].m_apiIndices.contains(apiIndex))
                    {
                        cmdListInstanceIndex = m_commandListInstancesVector[i].m_instanceIndex;
                    }
                }
            }
        }

        retVal = CommandListNameFromPointer(commandListPtr, commandListIndex);
    }

    return retVal;
}

QString gpTraceDataContainer::QueueNameFromPointer(const QString& queuePtrStr)
{
    int queueIndex = 1;
    QString retVal;

    if (SessionAPIType() == ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
    {
        // If the command buffer / list is not mapped to an index yet, allocate an index for the current command list
        if (!m_commandQueuePointerToIndexMap.contains(queuePtrStr))
        {
            int index = m_commandQueuePointerToIndexMap.size() + 1;
            m_commandQueuePointerToIndexMap.insertMulti(queuePtrStr, index);
        }

        // Get the index for this command list / buffer
        GT_IF_WITH_ASSERT(m_commandQueuePointerToIndexMap.contains(queuePtrStr))
        {
            queueIndex = m_commandQueuePointerToIndexMap[queuePtrStr];
        }
    }
    else
    {
        queueIndex = queuePtrStr.toInt();
    }

    retVal = QString(GPU_STR_timeline_QueueBranchName).arg(queueIndex);

    return retVal;
}

QString gpTraceDataContainer::QueueDisplayName(const QString& queuePtrStr)
{
    
    QString retVal;

    // Append the queue type to the queue name in DX12 (irrelevant in Vulkan)
    if (SessionAPIType() == ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
    {
        int queueType = QueueType(queuePtrStr);
        QString queueTypeStr = CommandListTypeAsString(queueType);
        retVal.append(queueTypeStr);
        retVal.append(AF_STR_SpaceA);
        retVal.append(QString(GPU_STR_timeline_QueueBranchName).arg(ProfileSessionDataItem::QueueDisplayName(queuePtrStr)));
    }
    else
    {
        // for vulkan: Get the name with the index
        retVal = QueueNameFromPointer(queuePtrStr);
    }

    return retVal;
}

gpTraceDataContainer::CommandListInstanceData::CommandListInstanceData() :
    m_commandListQueueName(""),
    m_startTime(std::numeric_limits<quint64>::max()),
    m_endTime(std::numeric_limits<quint64>::min()),
    m_instanceIndex(-1),
    m_commandListPtr("")
{

}
