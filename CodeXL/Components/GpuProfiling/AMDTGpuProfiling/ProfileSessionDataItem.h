//------------------------------ ProfileSessionDataItem.h ------------------------------

#ifndef _PROFILESESSIONDATAITEM_H_
#define _PROFILESESSIONDATAITEM_H_

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include "CLAPIInfo.h"
#include "HSAAPIInfo.h"
#include "DX12APIInfo.h"
#include "VulkanAPIInfo.h"
#include "PerfMarkerAtpFile.h"
#include <AMDTGpuProfiling/ProjectSettings.h>

class OccupancyInfo;
class gpTraceDataContainer;
typedef QPair<quint64, quint64> TimeRange;

// ----------------------------------------------------------------------------------
// Class Name:          ProfileSessionDataItem
// General Description: Represents an item in a GPU profile session
// ----------------------------------------------------------------------------------
class ProfileSessionDataItem
{
public:


    enum ProfileItemAPIType
    {
        API_PROFILE_ITEM_UNKNOWN = 0,
        CL_API_PROFILE_ITEM,
        CL_GPU_PROFILE_ITEM,
        HSA_API_PROFILE_ITEM,
        HSA_GPU_PROFILE_ITEM,
        PERF_MARKER_PROFILE_ITEM,
        DX12_API_PROFILE_ITEM,
        DX12_GPU_PROFILE_ITEM,
        VK_API_PROFILE_ITEM,
        VK_GPU_PROFILE_ITEM,
    };

    enum GPUItemType
    {
        GPU_MEMORY_ITEM,
        GPU_KERNEL_ITEM,
        GPU_OTHER_ITEM,
        GPU_DATA_ITEM
    };

    class ProfileItemType
    {
    public:

        /// Constructors
        ProfileItemType() : m_itemMainType(API_PROFILE_ITEM_UNKNOWN), m_itemSubType(0) {}
        ProfileItemType(ProfileItemAPIType apiType) : m_itemMainType(apiType), m_itemSubType(0) {}
        ProfileItemType(ProfileItemAPIType apiType, unsigned int itemType) : m_itemMainType(apiType), m_itemSubType(itemType) {}
        ProfileItemType(const ProfileItemType& other) : m_itemMainType(other.m_itemMainType), m_itemSubType(other.m_itemSubType) {}

        /// Return true iff this is a type of a dispatch command (CL / HSA)
        bool IsDispatchCommand() const;

        /// The item main type (CL/DX/GPU item etc')
        ProfileItemAPIType m_itemMainType;

        /// m_itemSubType contains the specific enumeration for this type.
        /// For example: CLAPIType for CL API functions
        unsigned int m_itemSubType;

        // override operator< specific for CPUProfileDataTableItem items
        // this operator handles differently items from "othr" row (6th row from top 5 table - hotspot)
        bool operator<(const ProfileItemType& other) const;
    };

    enum ProfileSessionDataColumnIndex
    {
        SESSION_ITEM_UNKNOWN = -1,
        SESSION_ITEM_INDEX_COLUMN = 0,
        SESSION_ITEM_INTERFACE_COLUMN,
        SESSION_ITEM_COMMAND_LIST_COLUMN,
        SESSION_ITEM_COMMAND_BUFFER_COLUMN,
        SESSION_ITEM_CALL_COLUMN,
        SESSION_ITEM_PARAMETERS_COLUMN,
        SESSION_ITEM_RESULT_COLUMN,
        SESSION_ITEM_DEVICE_BLOCK_COLUMN,
        SESSION_ITEM_OCCUPANCY_COLUMN,
        SESSION_ITEM_CPU_TIME_COLUMN,
        SESSION_ITEM_GPU_TIME_COLUMN,
        SESSION_ITEM_DEVICE_TIME_COLUMN,
        SESSION_ITEM_START_TIME_COLUMN,
        SESSION_ITEM_END_TIME_COLUMN,
        SESSION_ITEM_COLUMN_COUNT = SESSION_ITEM_END_TIME_COLUMN + 1
    };

    /// Constructor for root items
    /// \param pApiInfo structure containing info about this CL API function(params, return val, timestamps, etc...)
    /// \param pOccupancyInfo the occupancy info for this item
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, ProfileItemAPIType itemType);

    /// Constructor
    /// \param pApiInfo structure containing info about this CL API function(params, return val, timestamps, etc...)
    /// \param pOccupancyInfo the occupancy info for this item
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, CLAPIInfo* pApiInfo, OccupancyInfo* pOccupancyInfo);

    /// Constructor
    /// \param pApiInfo structure containing info about this HSA API function (params, return val, timestamps, etc...)
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, HSAAPIInfo* pApiInfo);

    /// Constructor
    /// \param pApiInfo structure containing info about this DX12 API function (params, return val, timestamps, etc...)
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, DX12APIInfo* pApiInfo);

    /// Constructor
    /// \param pApiInfo structure containing info about this DX12 API function (params, return val, timestamps, etc...)
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, DX12GPUTraceInfo* pApiInfo);

    /// Constructor
    /// \param pApiInfo structure containing info about this Vulkan API function (params, return val, timestamps, etc...)
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, VKAPIInfo* pApiInfo);

    /// Constructor
    /// \param pApiInfo structure containing info about this Vulkan API function (params, return val, timestamps, etc...)
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, VKGPUTraceInfo* pApiInfo);

    /// Constructor for GPU item
    /// \param pOwnerCPUAPIItem the data item representing the API call
    /// \param ProfileItemType itemType the item type
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, ProfileSessionDataItem* pOwnerCPUAPIItem, ProfileItemType itemType);

    /// Constructor
    /// \param pApiInfo structure containing info about this HSA API function (params, return val, timestamps, etc...)
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, HSAAPIInfo* pApiInfo, ProfileItemType itemType);

    /// Constructor
    /// \param pMarkerEntry the object describing the performance marker
    ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, PerfMarkerEntry* pMarkerEntry);

    /// Return true if one of the columns of the item matches the input string
    /// \param findExpr the string to search for
    /// \param isCaseSensitive should the search be case sensitive?
    /// \return true iff the item contain the string in one of it's columns
    bool DoesStringMatch(const QString& findExpr, bool isCaseSensitive);

    /// Destructor
    virtual ~ProfileSessionDataItem();

    /// Appends a child to the end of the child list of this item
    /// \param child the child item to append
    void AppendChild(ProfileSessionDataItem* pChild);

    /// Inserts a child at the specified index in the child list of this item
    /// \param index the index at which to insert the child
    /// \param child the child item to append
    void InsertChild(int index, ProfileSessionDataItem* pChild);

    /// Gets the parent of this item
    /// \return the parent of this item
    ProfileSessionDataItem* GetParent() const { return m_pParent; }

    /// Gets the number of children this node has
    /// \return the number of children this node has
    int GetChildCount() const { return m_children.count(); }

    /// Reserves a count places of children:
    void ReserveChildrenCount(int count) { m_children.reserve(count); }

    /// Gets the specified child
    /// \param childIndex the index of the child requested
    /// \return the specified child
    ProfileSessionDataItem* GetChild(int childIndex) const;

    /// Gets the data for the specified column
    /// \param columnIndex the index of the column whose data is needed
    /// \return the data for the specified column
    QVariant GetColumnData(int columnIndex) const;

    /// Gets the tooltip for the specified column
    /// \param columnIndex the index of the column whose data is needed
    /// \return the tooltip for the specified column
    QString GetColumnTooltip(int columnIndex) const;

    /// Gets the data for the specified column
    /// \param columnIndex the index of the column whose data should be set
    /// \param data the data for the specified column
    void SetColumnData(int columnIndex, QVariant data);

    /// Gets the index of this item within its parent
    /// \return the index of this item within its parent
    int GetRow() const;

    /// Return a pointer to the occupancy data
    OccupancyInfo* GetOccupancyInfo() const { return m_pOccupancyInfo; };

    /// ----------- Data attributes common to all profile session items:

    /// Item start time
    quint64 StartTime() const { return m_startTime; }

    /// Item end time
    quint64 EndTime() const { return m_endTime; }

    /// Item start time
    quint64 StartTimeMilliseconds() const;

    /// Item end time
    quint64 EndTimeMilliseconds() const;

    /// time to milliseconds
    quint64 TimeMilliseconds(quint64 time) const;

    /// Set the end time
    void SetEndTime(quint64 endTime) { m_endTime = endTime; }

    /// Thread ID
    osThreadId ThreadID() const { return m_tid; }

    /// API call index
    int APICallIndex() const { return m_itemIndex; }

    /// Queue name
    QString QueueName() const;

    /// Command list pointer (only applicable for DX12 GPU items)
    QString CommandListPointer() const;

    /// The queue name, without the leading zeros
    static QString QueueDisplayName(const QString& queueName) ;

    /// If this is not a API object type (GPU, for instance), and it has an owner, return it
    ProfileSessionDataItem* OwnerAPIItem() const { return m_pOwnerCPUAPIItem; };

    /// The item type
    ProfileItemType ItemType() const { return m_itemType; }

    /// Is this a CPU API item?
    bool IsCPUItem() const;

    /// Is this a GPU API item?
    bool IsGPUItem() const;

    /// Return an API call sequence ID
    /// \param seqID[output] the sequence ID
    /// \return true when the data found
    bool GetDisplaySeqID(unsigned int& seqID) const;

    /// ----------- Data attributes that are are only related to some of the item types
    /// CL_API_PROFILE_ITEM data attributes
    bool GetAPIFunctionID(unsigned int& apiID) const;

    /// CL_API_PROFILE_ITEM, CL_ENQUEUE_BASE_API data attributes
    bool GetEnqueueCommandType(QString& commandType) const;

    /// Get the enqueue command GPU start and end times
    bool GetEnqueueCommandGPUTimes(quint64& gpuStart, quint64& gpuEnd, quint64& gpuQueued, quint64& gpuSubmit);
    bool GetEnqueueCommandQueueID(unsigned int& contextID, unsigned int& queueID);

    /// CL_API_PROFILE_ITEM, CL_ENQUEUE_OTHER_OPERATIONS data attributes
    bool GetEnqueueOtherDataSize(quint64& dataSize);

    /// CL_API_PROFILE_ITEM, CL_ENQUEUE_MEM data attributes
    bool GetEnqueueMemTransferSize(quint64& transferSize);

    /// HSA_API_Type_Non_API_Dispatch data attributes
    bool GetHSACommandGPUTimes(quint64& gpuStart, quint64& gpuEnd);

    /// CL_API_PROFILE_ITEM, CL_ENQUEUE_KERNEL and HSA_API_Type_Non_API_Dispatch data attributes
    bool GetDispatchCommandWorkSizes(QString& globalWorkSize, QString& groupWorkSize);
    bool GetDispatchCommandDeviceNames(QString& strQueueHandle, QString& strContextHandle, QString& deviceNameStr);

    /// CL_API_PROFILE_ITEM, CL_ENQUEUE_KERNEL and HSA_API_Type_Non_API_Dispatch
    bool GetDispatchCommandKernelName(QString& kernelNameStr);

    /// Initializes the mapping from item type to columns list
    static void InitStaticMembers();

    /// Get the list of columns that should be displayed for the requested item
    /// \param apiItemType the profiled session item type
    /// \param [out] columns vector with the matching column indices
    /// \return true if the item type was found and columns were added correctly to the output vector
    static bool GetListOfColumnIndices(ProfileItemAPIType apiItemType, QVector<ProfileSessionDataColumnIndex>& columns);

    /// Return the string describing the column header to the requested column index
    /// \param ProfileSessionDataColumnIndex the column index enumeration
    /// \return string with the column title
    static QString ColumnIndexTitle(ProfileSessionDataColumnIndex colIndex);

    /// Update the start & end indices with the current added child index:
    /// \param childStartIndex start updating from childStartIndex
    /// \param childEndIndex end updating at childEndIndex
    void UpdateIndices(int childStartIndex, int childEndIndex);

    /// Gets the item end index
    /// \return the item end index
    int EndIndex() const { return m_endIndex; }

    /// Remove all the children of this item
    void RemoveAllChildren();

    /// Return the DX12 api type
    /// \param apiType[out] the api type
    /// \return true for success (false if this is not a DX12 item)
    bool GetDX12APIType(eAPIType& apiType);

    /// Return the Vulkan api type
    /// \param apiType[out] the api type
    /// \return true for success (false if this is not a DX12 item)
    bool GetVKAPIType(vkAPIType& apiType);

    /// Get the item sample id
    int SampleId()const { return m_sampleId; }

    /// Return the session item interface pointer. Only relevant for DX12
    /// \return string with the interface pointer for DX12 function, or an empty string for other APIs
    QString InterfacePtr()const;

private:

    /// Disable copy constructor
    ProfileSessionDataItem(const ProfileSessionDataItem&);

    /// Disable default assignment operator
    const ProfileSessionDataItem& operator=(const ProfileSessionDataItem& obj);

    /// List of children of this item
    QList<ProfileSessionDataItem*> m_children;

    /// The parent of this item
    ProfileSessionDataItem* m_pParent;

    /// The data for this item
    QList<QVariant> m_data;

    /// The item type (main + sub item type, see the definition for ProfileItemType)
    ProfileItemType m_itemType;

    /// For GPU items - contain a pointer to the CPU API item
    ProfileSessionDataItem* m_pOwnerCPUAPIItem;

    /// The item index within the list
    int m_itemIndex;

    /// The item end index (-1 in case where index is irrelevant)
    int m_endIndex;

    /// The item's API:
    APIToTrace m_api;

    /// Occupancy info
    OccupancyInfo* m_pOccupancyInfo;

    /// The API start time
    /// For DX12 API functions, the units are microseconds.
    /// For CL API functions and performance markers, units are milliseconds
    quint64 m_startTime;

    /// The API end time
    quint64 m_endTime;

    /// The item thread id
    osThreadId m_tid;

    /// The item queue name
    QString m_queueName;

    /// Contain the structure parsed and filled by the backend
    APIInfo* m_pApiInfo;

    /// The item sample Id (0 in case where sampleId is may not be synched)
    int m_sampleId;

    /// True if InitStaticMembers was called
    static bool m_sAreStaticMembersInitialized;

    /// Mapping from the item type to the list of relevant column data displayed
    static QMap<ProfileItemAPIType, QVector<ProfileSessionDataColumnIndex>> m_sItemTypesColumnsMap;

    /// Mapping from the item type to the column caption
    static QMap<ProfileSessionDataColumnIndex, QString> m_sItemTypesToTitleMap;

    /// Contain the session start and end times
    gpTraceDataContainer* m_pSessionDataContainer;
};


#endif // _PROFILESESSIONDATAITEM_H_