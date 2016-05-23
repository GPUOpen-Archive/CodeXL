//------------------------------ DXProfileSessionDataContainer.h ------------------------------

#ifndef _GPTRACEDATACONTAINER_H_
#define _GPTRACEDATACONTAINER_H_



#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Qt:
#include <QList>
#include <QMap>
#include <QStack>

// Backend:
#include "CLAPIInfo.h"
#include "HSAAPIInfo.h"
#include "DX12APIInfo.h"
#include "PerfMarkerAtpFile.h"


// Local:
#include <AMDTGpuProfiling/OccupancyInfo.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>

class ProfileSessionDataItem;
class SymbolInfo;

// ----------------------------------------------------------------------------------
// Class Name:          gpTraceDataContainer
// General Description: A DX profiled session data container
// ----------------------------------------------------------------------------------
class gpTraceDataContainer
{
public:
    friend class gpTraceDataParser;
    /// Constructor
    gpTraceDataContainer();

    /// Destructor
    ~gpTraceDataContainer();

    class CommandListInstanceData
    {
    public:

        /// Constructor
        CommandListInstanceData();

        /// The queue for which the command list belongs to
        QString m_commandListQueueName;

        /// A list of API calls indices
        QList<int> m_apiIndices;

        /// The command list start time
        quint64 m_startTime;

        /// The command list end time
        quint64 m_endTime;

        /// The index of this command list instance execution
        int m_instanceIndex;

        /// The command list pointer
        QString m_commandListPtr;

        /// List of sample ids for this instance
        QList<int> m_sampleIds;
    };

    /// Sets the session API calls count
    virtual void SetAPINum(osThreadId threadId, unsigned int apiNum);

    /// Add an OpenCL API item:
    /// \param the CLAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddCLItem(CLAPIInfo* pAPIInfo);

    /// pThreadItem the thread new item
    void AddItemToThread(ProfileSessionDataItem* pThreadItem);

    /// Add an HSA API item:
    /// \param the HSAAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddHSAItem(HSAAPIInfo* pAPIInfo);

    /// Add a DX12 API item:
    /// \param the DX12APIInfo class describing the DX12 item to add
    /// \return the created API item
    ProfileSessionDataItem* AddDX12APIItem(DX12APIInfo* pAPIInfo);

    /// Add a DX12 API item:
    /// \param the DX12APIInfo class describing the DX12 item to add
    /// \return the created API item
    ProfileSessionDataItem* AddDX12GPUTraceItem(DX12GPUTraceInfo* pAPIInfo);

    /// Add a DX12 API item:
    /// \param the VKAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddVKAPIItem(VKAPIInfo* pAPIInfo);

    /// Add a DX12 API item:
    /// \param the VKAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddVKGPUTraceItem(VKGPUTraceInfo* pAPIInfo);

    /// Add an OpenCL GPU item:
    /// \param pAPIOwnerItem the item representing the owner API call
    void AddCLGPUItem(ProfileSessionDataItem* pAPIOwnerItem);

    /// Add an HSA GPU item:
    /// \param pAPIInfo the API info for the HSA dispatch API call
    void AddHSAGPUItem(HSAAPIInfo* pAPIInfo);

    /// Add a performance marker object to the data container
    /// \param pPerfMarkerEntry the performance marker information
    void AddPerformanceMarker(PerfMarkerEntry* pPerfMarkerEntry);

    /// Find the occupancy info for the requested API
    /// \param pAPIInfo the API info for which the occupancy is needed
    /// \return matching OccupancyInfo or null
    OccupancyInfo* FindOccupancyInfo(CLAPIInfo* pAPIInfo);

    /// Finds the symbol info for the API specified by threadId and callIndex
    /// \param threadId the thread Id of the API
    /// \param callIndex the call index of the API
    /// \return the symbol info for the API specified by threadId and callIndex or NULL if not found
    SymbolInfo* GetSymbolInfo(int threadId, int callIndex);

    /// Accessor
    /// \return a reference to the symbol map table
    const QMap<uint, QList<SymbolInfo*>>& SymbolTableMap() const { return m_symbolTableMap; };

    // Return the API monitored for the requested thread
    /// \param tid the monitored thread ID
    /// \return the API type monitored for this thread
    ProfileSessionDataItem::ProfileItemAPIType APITypeForThread(osThreadId tid);

    /// Return the threads count for the session
    int ThreadsCount() const;

    /// Return the DX12 queues or Vulkan command buffers count for this session
    int GPUCallsContainersCount() const;

    /// Return the thread ID for the thread in index threadIndex
    osThreadId ThreadID(int threadIndex) const;

    /// Return the API count for the thread
    unsigned int ThreadAPICount(osThreadId threadID);

    /// Return the performance markers count for the thread
    int ThreadPerfMarkersCount(osThreadId threadID) const;

    /// Return the GPU items count
    int QueueItemsCount() const;

    /// Return the API item in apiItemIndex for the thread in threadID
    ProfileSessionDataItem* APIItem(osThreadId threadID, int apiItemIndex) const;

    /// Return the performance marker item in markerIndex for the thread in threadID
    ProfileSessionDataItem* PerfMarkerItem(osThreadId threadID, int markerIndex) const;

    /// Return the GPU item
    ProfileSessionDataItem* GpuItem(int gpuItemIndex) const;

    /// Return the root item for the requested thread
    ProfileSessionDataItem* GetRootItem(osThreadId threadID);

    /// Return true iff the requested thread contain performance markers
    bool DoesThreadContainPerformanceMarkers(osThreadId threadID);

    /// Finalize the data collection. This function is creating the structure of the data, and connecting profile data items.
    /// Some of the relations between the items can only be determined as post process, so this is done as a post process to the data collection
    void FinalizeDataCollection();

    /// Go over the API functions and performance markers for a thread, and merge them into a single tree
    void MergePerformanceCountersForThread(osThreadId tid);

    /// Return the API calls count across all thread
    int APICount() { return m_apiCount; };

    /// Returns the session start and end time
    TimeRange SessionTimeRange() const { return m_sessionTimeRange; }

    /// Return the name of the queue / command buffer in the index place in the map
    QString GPUObjectName(int index);

    /// Return an integer number of describing the queue / command buffer type
    int QueueType(const QString& queueName);

    /// Return the items count for the requested queue / command buffer
    /// \param gpuItemName the queue / command buffer name
    /// \return the items count for the requested queue
    int QueueItemsCount(const QString& gpuItemName);

    /// Returns the item in m_sessionQueueToGPUDataItems which has the requested queue name and given index (n'th place in the list of items with the same queue name)
    /// \param gpuObjectName the requested queue / command buffer name
    /// \param apiItemIndex the requested index (n'th place in the list of items with the same queue name)
    ProfileSessionDataItem* QueueItem(const QString& gpuObjectName, int apiItemIndex) const;

    /// Returns the item in m_sessionQueueToGPUDataItems which has the requested queue name and call index. This method is required when there is more than 1 queue, and the n'th place does not match the call index 
    /// \param queueName the requested queue name
    /// \param apiItemIndex the requested call index
    ProfileSessionDataItem* QueueItemByItemCallIndex(const QString& queueName, int apiItemIndex) const;

    /// Return all API items with the given sampleId
    /// \param sampleId the requested sampleId
    /// \param items list, to be filled in this method (output)
    void GetCPUItemsBySampleId(int sampleId, QList<ProfileSessionDataItem*>& items)const;

    /// Return all GPU items with the given sampleId
    void GetGPUItemsBySampleId(int sampleId, QList<ProfileSessionDataItem*>& items)const;

    /// Convert the command list type to a string
    static QString CommandListTypeAsString(int commandListType);

    /// Find the first item that contain the find string in one of it's columns
    /// \param findStr the string to search for
    /// \param isCaseSensitive should the search be case sensitive?
    /// \return the item in which the string appears or null if the string was not found in the container
    ProfileSessionDataItem* FindItem(const QString& findStr, bool isCaseSensitive);

    /// Find the next item that contain the find string in one of it's columns
    /// \param findStr the string to search for
    /// \param isCaseSensitive should the search be case sensitive?
    /// \return the item in which the string appears or null if the string was not found in the container
    ProfileSessionDataItem* FindNextItem(const QString& findStr, bool isCaseSensitive);

    /// Get the data collected for the frame command lists
    const QVector<CommandListInstanceData>& CommandListsData() const { return m_commandListInstancesVector; }

    /// Is called when a command list / buffer close / end function is added. 
    /// The function will analyze the command list API execution 
    /// \param pAPIInfo the api info is expected to be of type DX12APIInfo or VKAPIInfo
    void CloseCommandList(APIInfo* pAPIInfo);

    /// Is called when a GPU call is added. The function is adding the GPU call to the matching command list
    /// \param pApiInfo the structure describing the API info of the GPU call
    /// \return the string describing the command list instance that executes this call
    QString AddGPUCallToCommandList(APIInfo* pAPIInfo);

    /// Contain the api type for this session
    ProfileSessionDataItem::ProfileItemAPIType SessionAPIType() const { return m_sessionAPIType; };

    /// Return an indexed name for the command buffer / list from  it's pointer
    /// \param commandBufferPtrStr the command buffer / list pointer
    /// \param instanceIndex the index of the command buffer / list execution
    /// \return a formatted name with the command buffer / list index
    QString CommandListNameFromPointer(const QString& commandBufferPtrStr, int instanceIndex);

    /// Return an indexed name for the command buffer / list matching this api call. The API call is executed on the GPU,
    /// and should be contained in one of the command lists / buffers
    /// \param pApiInfo* the GPU call api info
    /// \return a formatted name with the command buffer / list index
    QString GetContainingCommandList(APIInfo* pApiInfo);

    /// Return an indexed name for the queue from it's pointer
    /// \param queuePtrStr the queue list pointer
    /// \return a formatted name with the queue index
    QString QueueNameFromPointer(const QString& queuePtrStr);

    /// Return a display name for the queue from it's pointer
    /// \param queuePtrStr the queue list pointer
    /// \return a formatted name with the queue index and type
    QString QueueDisplayName(const QString& queuePtrStr);

private:

    /// Map from thread id to API count
    QMap<osThreadId, unsigned int> m_apiCountMap;

    /// List of the CPU profile session data items
    QMap<osThreadId, QList<ProfileSessionDataItem*>> m_sessionCPUDataItems;

    /// Map from DX12 / Vulkan Queue to the GPU profile session data items
    QMap<std::string, QList<ProfileSessionDataItem*>> m_sessionQueuesToCallsMap;

    /// Map queue name to command list type
    QMap<std::string, int> m_sessionQueueNameToCommandListType;

    /// List of the performance markers for each thread
    QMap<osThreadId, QList<ProfileSessionDataItem*>> m_sessionPerformanceMarkers;

    /// Map from the performance markers items, to their time ranges
    QMap<TimeRange, ProfileSessionDataItem*> m_perfMarkersTraceItemsMap;

    /// Stack of opened markers. Is used while building the data structure
    QStack<ProfileSessionDataItem*> m_openedMarkers;

    /// List of the GPU profile session data items
    QList<ProfileSessionDataItem*> m_sessionGPUDataItems;

    /// Stack trace table map. From thread ID to symbol list
    QMap<uint, QList<SymbolInfo*>> m_symbolTableMap;

    /// The root of the tree
    QMap<osThreadId, ProfileSessionDataItem*> m_threadToRootItemMap;

    /// Gets occupancy table map. From thread ID to list of occupancy info
    OccupancyTable m_occupancyInfoMap;

    /// Map from thread id to the current occupancy index for that thread (for OCL)
    QMap<osThreadId, int> m_oclThreadOccIndexMap;

    /// Map from thread id to the monitored API type
    QMap<osThreadId, ProfileSessionDataItem::ProfileItemAPIType> m_threadAPIType;

    /// Maps the items sorted by start time. Will be used for quick find
    QMap<quint64, ProfileSessionDataItem*> m_sessionItemsSortedByStartTime;

    /// Contain the last find result start time
    quint64 m_lastFindResultStartTime;
    QString m_lastStringSearched;

    /// Counts the API function calls
    int m_apiCount;

    /// Contain the start and end time of the session
    TimeRange m_sessionTimeRange;

    /// map of sampleId to CPU\GPU items, for fast sync
    QMap<int, ProfileSessionDataItem*> m_sampleIdToCPUItemMap;
    QMap<int, ProfileSessionDataItem*> m_sampleIdToGPUItemMap;

    /// A map from command queue pointer to an index
    QMap <QString, int> m_commandQueuePointerToIndexMap;

    /// A map from command list / buffer pointer to queue pointer
    QMap <QString, QString> m_commandListToQueueMap;

    /// A map from command list / buffer pointer to an index
    QMap <QString, int> m_commandListPointerToIndexMap;

    /// Vector of command list instances 
    QVector<CommandListInstanceData> m_commandListInstancesVector;

    /// Contain the api type for this session
    ProfileSessionDataItem::ProfileItemAPIType m_sessionAPIType;

    /// List of calls which has sample id and are not attached to a command list
    QList<ProfileSessionDataItem*> m_commandListUnAttachedCalls;
};

#endif // _GPTRACEDATACONTAINER_H_
