//------------------------------ gpObjectDataContainer.h ------------------------------

#ifndef _GPObjectDATACONTAINER_H_
#define _GPObjectDATACONTAINER_H_

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
// Class Name:          gpObjectDataContainer
// General Description: A DX profiled session object data container, functionality not actually used, included to match trace data implementation
// ----------------------------------------------------------------------------------
class gpObjectDataContainer
{
public:
    friend class gpObjectDataParser;
    /// Constructor
    gpObjectDataContainer();

    /// Destructor
    ~gpObjectDataContainer();

    /// Sets the session API calls count
    virtual void SetAPINum(osThreadId threadId, unsigned int apiNum);

    /// Add an OpenCL API item:
    /// \param the CLAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddCLItem(CLAPIInfo* pAPIInfo);

    /// Add an HSA API item:
    /// \param the HSAAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddHSAItem(HSAAPIInfo* pAPIInfo);

    /// Add a DX12 API item:
    /// \param the DX12APIInfo class describing the DX12 item to add
    /// \return the created API item
    ProfileSessionDataItem* AddDX12APIItem(DX12APIInfo* pAPIInfo);

    /// Add a DX12 API item:
    /// \param the VKAPIInfo class describing the OpenCL item to add
    /// \return the created API item
    ProfileSessionDataItem* AddVKAPIItem(VKAPIInfo* pAPIInfo);

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

    /// Return the DX12 queues count for the session
    int DX12QueuesCount() const;

    /// Return the thread ID for the thread in index threadIndex
    osThreadId ThreadID(int threadIndex) const;

    /// Return the API count for the thread
    unsigned int ThreadAPICount(osThreadId threadID);

    /// Return the performance markers count for the thread
    int ThreadPerfMarkersCount(osThreadId threadID) const;

    /// Return the GPU items count
    int GPUItemsCount() const;

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

    /// Return the name of the queue in the queueIndex place in the map
    QString QueueName(int queueIndex);

    /// Return the items count for the requested queue
    /// \param queueName the queue name
    /// \return the items count for the requested queue
    int QueueItemsCount(const QString& queueName);

    /// Return the API item in apiItemIndex for the thread in threadID
    ProfileSessionDataItem* QueueItem(const QString& queueName, int apiItemIndex) const;
private:

    /// Map from thread id to API count
    QMap<osThreadId, unsigned int> m_apiCountMap;

    /// List of the CPU profile session data items
    QMap<osThreadId, QList<ProfileSessionDataItem*>> m_sessionCPUDataItems;

    /// List of the GPU profile session data items
    QMap<std::string, QList<ProfileSessionDataItem*>> m_sessionQueueToGPUDataItems;

    /// List of the performance markers for each thread
    QMap<osThreadId, QList<ProfileSessionDataItem*>> m_sessionPerformanceMarkers;

    /// Map from the performance markers items, to their time ranges
    QMap<TimeRange, ProfileSessionDataItem*> m_perfMarkersObjectItemsMap;

    /// Stack of opened markers. Is used while building the data structure
    QStack<ProfileSessionDataItem*> m_openedMarkers;

    /// List of the GPU profile session data items
    QList<ProfileSessionDataItem*> m_sessionGPUDataItems;

    /// Stack Object table map. From thread ID to symbol list
    QMap<uint, QList<SymbolInfo*>> m_symbolTableMap;

    /// The root of the tree
    QMap<osThreadId, ProfileSessionDataItem*> m_threadToRootItemMap;

    /// Gets occupancy table map. From thread ID to list of occupancy info
    OccupancyTable m_occupancyInfoMap;

    /// Map from thread id to the current occupancy index for that thread (for OCL)
    QMap<osThreadId, int> m_oclThreadOccIndexMap;

    /// Map from thread id to the monitored API type
    QMap<osThreadId, ProfileSessionDataItem::ProfileItemAPIType> m_threadAPIType;

    /// Counts the API function calls
    int m_apiCount;

    /// Contain the start and end time of the session
    TimeRange m_sessionTimeRange;
};

#endif // _GPObjectDATACONTAINER_H_
