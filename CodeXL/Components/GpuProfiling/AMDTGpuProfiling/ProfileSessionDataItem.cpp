//------------------------------ ProfileSessionDataItem.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/CLTimelineItems.h>

// Static members initialization

/// True iff InitStaticMembers was called
bool ProfileSessionDataItem::m_sAreStaticMembersInitialized = false;
QMap<ProfileSessionDataItem::ProfileItemAPIType, QVector<ProfileSessionDataItem::ProfileSessionDataColumnIndex>> ProfileSessionDataItem::m_sItemTypesColumnsMap;
QMap<ProfileSessionDataItem::ProfileSessionDataColumnIndex, QString> ProfileSessionDataItem::m_sItemTypesToTitleMap;

/// Use this define to add start and end time columns (for debug purposes)
#define DEBUG_TABLE_DATA 1

// The DX12 timestamps are double number. The data structures expect long long numbers, so we multiply the double timestamp by a GP_DX_TIMESTAMP_FACTOR
// to make sure that we get integer value. In the front-end, we will perform the opposite operation
#define GP_NANOSECONDS_TO_MILLISECOND 1000000
#define GP_MILLISECONDS_TO_SECONDS 1000

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, ProfileItemAPIType itemType) :
    m_pParent(nullptr),
    m_itemType(itemType, 0),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(nullptr),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{

}


ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, CLAPIInfo* pApiInfo, OccupancyInfo* pOccupancyInfo) :
    m_pParent(nullptr),
    m_itemType(API_PROFILE_ITEM_UNKNOWN, 0),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_OPENCL),
    m_pOccupancyInfo(pOccupancyInfo),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(nullptr),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{
    // This is an OpenCL API function, since we get a CLAPIInfo*:
    m_api = APIToTrace_OPENCL;

    // Sanity check:
    if (pApiInfo != nullptr)
    {
        m_itemType.m_itemMainType = CL_API_PROFILE_ITEM;
        m_itemType.m_itemSubType = pApiInfo->m_Type;

        // Fill the data structure with empty strings:
        m_data.reserve(ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT);

        for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
        {
            m_data << "";
        }

        if (pApiInfo != nullptr)
        {
            if (pApiInfo->m_bHasDisplayableSeqId)
            {
                m_itemIndex = (int)pApiInfo->m_uiDisplaySeqID;
            }

            if (pApiInfo->m_uiAPIID < CL_FUNC_TYPE_Unknown)
            {
                m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = CLAPIDefs::Instance()->GetOpenCLAPIString(CL_FUNC_TYPE(pApiInfo->m_uiAPIID));
            }
            else
            {
                m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = pApiInfo->m_strName.c_str();
            }

            m_data[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
            m_data[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);

            // Set the start and end time
            m_startTime = pApiInfo->m_ullStart;
            m_endTime = pApiInfo->m_ullEnd;

            // removed calc of m_data[ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN] because it will be calulated on GetColumnData()

            // Set the thread id
            m_tid = pApiInfo->m_tid;

            m_pApiInfo = pApiInfo;

            // Set the device block text
            bool isEnququeMem = (m_itemType.m_itemSubType & CL_ENQUEUE_MEM) == CL_ENQUEUE_MEM;
            bool isEnququeKernel = (m_itemType.m_itemSubType & CL_ENQUEUE_KERNEL) == CL_ENQUEUE_KERNEL;
            bool isEnququeOther = (m_itemType.m_itemSubType & CL_ENQUEUE_OTHER_OPERATIONS) == CL_ENQUEUE_OTHER_OPERATIONS;

            // This is an enqueue API
            if (isEnququeKernel)
            {
                CLKernelAPIInfo* pKernelApiInfo = dynamic_cast<CLKernelAPIInfo*>(pApiInfo);

                if (pKernelApiInfo != nullptr)
                {
                    m_data[ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN] = QString::fromStdString(pKernelApiInfo->m_strKernelName);
                }
            }
            else if (isEnququeMem)
            {
                CLMemAPIInfo* memApiInfo = dynamic_cast<CLMemAPIInfo*>(pApiInfo);
                quint64 transferSize = memApiInfo->m_uiTransferSize;
                QString strCmdType = QString::fromStdString(memApiInfo->m_strCMDType);
                m_data[ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN] = CLMemTimelineItem::getDataSizeString(transferSize, 1) + " " + strCmdType.mid(11);
            }
            else if (isEnququeOther)
            {
                CLOtherEnqueueAPIInfo* pOtherEnqueueOperationsInfo = dynamic_cast<CLOtherEnqueueAPIInfo*>(pApiInfo);
                GT_IF_WITH_ASSERT(pOtherEnqueueOperationsInfo != nullptr)
                {
                    QString strCmdType = QString::fromStdString(pOtherEnqueueOperationsInfo->m_strCMDType);

                    QString commandName = strCmdType.replace("CL_COMMAND_", "");

                    if ((m_itemType.m_itemSubType & CL_ENQUEUE_DATA_OPERATIONS) == CL_ENQUEUE_DATA_OPERATIONS)
                    {
                        CLDataEnqueueAPIInfo* pDataEnqueueOperationsInfo = dynamic_cast<CLDataEnqueueAPIInfo*>(pApiInfo);
                        GT_IF_WITH_ASSERT(pDataEnqueueOperationsInfo != nullptr)
                        {
                            quint64 dataSize = pDataEnqueueOperationsInfo->m_uiDataSize;
                            commandName.prepend(CLMemTimelineItem::getDataSizeString(dataSize, 1) + " ");
                        }
                    }

                    m_data[ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN] = commandName;

                }
            }
        }
    }
}

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, HSAAPIInfo* pApiInfo) :
    m_pParent(nullptr),
    m_itemType(HSA_API_PROFILE_ITEM, 0),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_HSA),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(nullptr),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{
    // Sanity check:
    if (pApiInfo != nullptr)
    {
        // Fill the data structure with empty strings:
        m_data.reserve(ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT);

        for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
        {
            m_data << "";
        }

        if (pApiInfo != nullptr)
        {
            if (pApiInfo->m_bHasDisplayableSeqId)
            {
                m_itemIndex = (int)pApiInfo->m_uiDisplaySeqID;
            }

            m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = pApiInfo->m_strName.c_str();
            m_data[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
            m_data[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);

            // Set the start and end time
            m_startTime = pApiInfo->m_ullStart;
            m_endTime = pApiInfo->m_ullEnd;

            // removed calc of m_data[ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN] because it will be calulated on GetColumnData()

            // Set the thread id
            m_tid = pApiInfo->m_tid;

            m_pApiInfo = pApiInfo;
        }
    }
}



ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, ProfileSessionDataItem* pOwnerCPUAPIItem, ProfileItemType itemType) :
    m_pParent(nullptr),
    m_itemType(itemType),
    m_pOwnerCPUAPIItem(pOwnerCPUAPIItem),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(nullptr),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{

    // Sanity check:
    if ((pOwnerCPUAPIItem != nullptr) && (pOwnerCPUAPIItem->m_pApiInfo != nullptr))
    {
        m_pApiInfo = pOwnerCPUAPIItem->m_pApiInfo;

        m_startTime = m_pApiInfo->m_ullStart;
        m_endTime = m_pApiInfo->m_ullEnd;

        if (pOwnerCPUAPIItem->m_itemType.m_itemMainType == CL_API_PROFILE_ITEM)
        {
            // Downcast the API info class
            CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(m_pApiInfo);

            if (m_itemType.m_itemMainType == CL_GPU_PROFILE_ITEM)
            {
                bool isEnqueueMem = (pCLApiInfo->m_Type & CL_ENQUEUE_MEM) == CL_ENQUEUE_MEM;
                bool isEnququeKernel = (pCLApiInfo->m_Type & CL_ENQUEUE_KERNEL) == CL_ENQUEUE_KERNEL;
                bool isEnququeOther = (pCLApiInfo->m_Type & CL_ENQUEUE_OTHER_OPERATIONS) == CL_ENQUEUE_OTHER_OPERATIONS;
                bool isEnququeData = (pCLApiInfo->m_Type & CL_ENQUEUE_DATA_OPERATIONS) == CL_ENQUEUE_DATA_OPERATIONS;

                // Set the GPU item type according to the owner API call type
                if (isEnqueueMem)
                {
                    m_itemType.m_itemSubType = (unsigned int)GPU_MEMORY_ITEM;
                }
                else if (isEnququeData)
                {
                    m_itemType.m_itemSubType = (unsigned int)GPU_DATA_ITEM;
                }
                else if (isEnququeKernel)
                {
                    m_itemType.m_itemSubType = (unsigned int)GPU_KERNEL_ITEM;
                }
                else if (isEnququeOther)
                {
                    m_itemType.m_itemSubType = (unsigned int)GPU_OTHER_ITEM;
                }
                else
                {
                    GT_ASSERT_EX(false, L"Unknown GPU item type");
                }
            }
        }
    }
}

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, HSAAPIInfo* pAPIInfo, ProfileItemType itemType) :
    m_pParent(nullptr),
    m_itemType(itemType),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(pAPIInfo),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{
    for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
    {
        m_data << "";
    }
}

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, DX12APIInfo* pApiInfo) :
    m_pParent(nullptr),
    m_itemType(DX12_API_PROFILE_ITEM, 0),
    m_pOwnerCPUAPIItem(nullptr),
m_itemIndex(-1),
m_endIndex(-1),
m_api(APIToTrace_Unknown),
m_pOccupancyInfo(nullptr),
m_startTime(0),
m_endTime(0),
m_tid(0),
m_queueName(""),
m_pApiInfo(pApiInfo),
m_sampleId(0),
m_pSessionDataContainer(pSessionDataContainer)
{
    for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
    {
        if (pApiInfo->m_bHasDisplayableSeqId)
        {
            m_itemIndex = (int)pApiInfo->m_uiDisplaySeqID;
        }

        m_itemType.m_itemSubType = pApiInfo->m_apiId;

        // Split the function name to an interface and call
        QString functionName = QString::fromStdString(pApiInfo->m_strName);
        int pos = functionName.indexOf('_');
        GT_IF_WITH_ASSERT(pos >= 0)
        {
            m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = functionName.mid(0, pos);
            m_data[ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN] = functionName.mid(pos + 1, functionName.length() - 1);
        }
        m_data[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
        m_data[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);

        // Set the start and end time. DX12 timestamps are stored in Nanoseconds
        m_startTime = pApiInfo->m_ullStart;
        m_endTime = pApiInfo->m_ullEnd;

        // removed calc of m_data[ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN] because it will be calulated on GetColumnData()

        m_tid = pApiInfo->m_tid;

        m_sampleId = pApiInfo->m_sampleId;
    }
}

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, DX12GPUTraceInfo* pApiInfo) :
    m_pParent(nullptr),
    m_itemType(DX12_GPU_PROFILE_ITEM, 0),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(pApiInfo),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{
    for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
    {
        if (pApiInfo->m_bHasDisplayableSeqId)
        {
            m_itemIndex = (int)pApiInfo->m_uiDisplaySeqID;
        }

        m_queueName = QString::fromStdString(pApiInfo->m_commandQueuePtrStr);

        // Split the function name to an interface and call
        QString functionName = QString::fromStdString(pApiInfo->m_strName);
        int pos = functionName.indexOf('_');
        GT_IF_WITH_ASSERT(pos >= 0)
        {
            m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = functionName.mid(0, pos);
            m_data[ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN] = functionName.mid(pos + 1, functionName.length() - 1);
        }
        m_itemType.m_itemSubType = pApiInfo->m_apiId;

        m_data[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
        m_data[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);


        // Set the start and end time. DX12 timestamps are stored in Nanoseconds
        m_startTime = pApiInfo->m_ullStart;
        m_endTime = pApiInfo->m_ullEnd;

        m_tid = pApiInfo->m_tid;
        m_sampleId = pApiInfo->m_sampleId;
    }
}

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, VKAPIInfo* pApiInfo) :
    m_pParent(nullptr),
    m_itemType(VK_API_PROFILE_ITEM, 0),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(pApiInfo),
    m_pSessionDataContainer(pSessionDataContainer)
{
    for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
    {
        if (pApiInfo->m_bHasDisplayableSeqId)
        {
            m_itemIndex = (int)pApiInfo->m_uiDisplaySeqID;
        }

        std::string interfaceName = pApiInfo->m_strName;
        m_itemType.m_itemSubType = pApiInfo->m_apiId;

        // Separate the interface string to interface_call
        size_t pos = pApiInfo->m_strName.find('_');
        GT_IF_WITH_ASSERT(pos != string::npos)
        {
            m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = QString::fromStdString(interfaceName.substr(0, pos));
            m_data[ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN] = QString::fromStdString(interfaceName.substr(pos + 1, interfaceName.size() - pos));

        }

        m_data[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
        m_data[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);

        // Set the start and end time. VK timestamps are stored in Nanoseconds
        m_startTime = pApiInfo->m_ullStart;
        m_endTime = pApiInfo->m_ullEnd;

        m_tid = pApiInfo->m_tid;
        m_sampleId = pApiInfo->m_sampleId;
    }
}

ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, VKGPUTraceInfo* pApiInfo) :
    m_pParent(nullptr),
    m_itemType(VK_GPU_PROFILE_ITEM, 0),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(pApiInfo),
    m_pSessionDataContainer(pSessionDataContainer)
{
    for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApiInfo != nullptr) && (m_pSessionDataContainer != nullptr))
    {
        if (pApiInfo->m_bHasDisplayableSeqId)
        {
            m_itemIndex = (int)pApiInfo->m_uiDisplaySeqID;
        }

        m_queueName = QString::fromStdString(pApiInfo->m_queueIndexStr);

        std::string interfaceName = pApiInfo->m_strName;
        m_itemType.m_itemSubType = pApiInfo->m_apiId;

        // Separate the interface string to interface_call
        size_t pos = interfaceName.find('_');
        GT_IF_WITH_ASSERT(pos != string::npos)
        {
            m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = QString::fromStdString(interfaceName.substr(0, pos));
            m_data[ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN] = QString::fromStdString(interfaceName.substr(pos + 1, interfaceName.size() - pos));

        }

        m_data[ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN] = QString::fromStdString(pApiInfo->m_ArgList);
        m_data[ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN] = QString::fromStdString(pApiInfo->m_strRet);
        m_data[ProfileSessionDataItem::SESSION_ITEM_COMMAND_BUFFER_COLUMN] = m_pSessionDataContainer->GetContainingCommandList(pApiInfo);

        // Set the start and end time. DX12 timestamps are stored in Nanoseconds
        m_startTime = pApiInfo->m_ullStart;
        m_endTime = pApiInfo->m_ullEnd;

        m_tid = pApiInfo->m_tid;
        m_sampleId = pApiInfo->m_sampleId;
    }
}


ProfileSessionDataItem::ProfileSessionDataItem(gpTraceDataContainer* pSessionDataContainer, PerfMarkerEntry* pMarkerEntry) :
    m_pParent(nullptr),
    m_itemType(PERF_MARKER_PROFILE_ITEM),
    m_pOwnerCPUAPIItem(nullptr),
    m_itemIndex(-1),
    m_endIndex(-1),
    m_api(APIToTrace_Unknown),
    m_pOccupancyInfo(nullptr),
    m_startTime(0),
    m_endTime(0),
    m_tid(0),
    m_queueName(""),
    m_pApiInfo(nullptr),
    m_sampleId(0),
    m_pSessionDataContainer(pSessionDataContainer)
{
    for (int i = 0; i < ProfileSessionDataItem::SESSION_ITEM_COLUMN_COUNT; i++)
    {
        m_data << "";
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(pMarkerEntry != nullptr)
    {
        // We should not get here with end markers (should not create a new session item, but close the old one)
        GT_IF_WITH_ASSERT(pMarkerEntry->m_markerType == PerfMarkerEntry::PerfMarkerType_Begin)
        {
            PerfMarkerBeginEntry* pBeginMarkerEntry = dynamic_cast<PerfMarkerBeginEntry*>(pMarkerEntry);
            GT_IF_WITH_ASSERT(pBeginMarkerEntry != nullptr)
            {
                m_data[ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN] = pBeginMarkerEntry->m_strName.c_str();
                m_startTime = pBeginMarkerEntry->m_timestamp;
            }
        }
    }
}

bool ProfileSessionDataItem::DoesStringMatch(const QString& findExpr, bool isCaseSensitive)
{
    bool retVal = false;

    Qt::CaseSensitivity cs = isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (m_sItemTypesColumnsMap.contains(m_itemType.m_itemMainType))
    {
        for (int i = (int)SESSION_ITEM_INDEX_COLUMN; i < (int)SESSION_ITEM_COLUMN_COUNT; i++)
        {
            ProfileSessionDataColumnIndex index = (ProfileSessionDataColumnIndex)i;

            if (m_sItemTypesColumnsMap[m_itemType.m_itemMainType].contains(index))
            {
                if (GetColumnData(i).toString().contains(findExpr, cs))
                {
                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
}

void ProfileSessionDataItem::RemoveAllChildren()
{
    foreach (ProfileSessionDataItem* pChild, m_children)
    {
        if (pChild != nullptr)
        {
            pChild->m_pParent = nullptr;
        }
    }

    m_children.clear();
}

bool ProfileSessionDataItem::GetDX12APIType(eAPIType& apiType)
{
    bool retVal = false;

    if ((m_itemType.m_itemMainType == DX12_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == DX12_GPU_PROFILE_ITEM))
    {
        GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
        {
            // Downcast the API info class
            DX12APIInfo* pDX12ApiInfo = dynamic_cast<DX12APIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pDX12ApiInfo)
            {
                // Get the API type from the DX12APIInfo
                apiType = pDX12ApiInfo->m_apiType;
                retVal = true;
            }
        }
    }

    return retVal;
}

bool ProfileSessionDataItem::GetVKAPIType(vkAPIType& apiType)
{
    bool retVal = false;

    if ((m_itemType.m_itemMainType == VK_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == VK_GPU_PROFILE_ITEM))
    {
        GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
        {
            // Downcast the API info class
            VKAPIInfo* pVKAPIInfo = dynamic_cast<VKAPIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pVKAPIInfo)
            {
                // Get the API type from the VKAPIInfo
                apiType = pVKAPIInfo->m_apiType;
                retVal = true;
            }
        }
    }

    return retVal;
}

QString ProfileSessionDataItem::InterfacePtr() const
{
    QString retVal;

    if ((m_itemType.m_itemMainType == DX12_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == DX12_GPU_PROFILE_ITEM))
    {
        DX12APIInfo* pDXInfo = (DX12APIInfo*)m_pApiInfo;
        if (pDXInfo != nullptr)
        {
            retVal = QString::fromStdString(pDXInfo->m_interfacePtrStr);
        }
    }
    return retVal;
}

ProfileSessionDataItem::~ProfileSessionDataItem()
{
    qDeleteAll(m_children);

    delete m_pApiInfo;
    delete m_pOccupancyInfo;
}

void ProfileSessionDataItem::AppendChild(ProfileSessionDataItem* pChild)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pChild != nullptr)
    {
        ProfileSessionDataItem* pCurrentParent = pChild->GetParent();

        if (pCurrentParent != this)
        {
            if (pCurrentParent != nullptr)
            {
                pCurrentParent->m_children.removeAt(pChild->GetRow());
            }

            m_children.append(pChild);
            pChild->m_pParent = this;
        }
    }
}

void ProfileSessionDataItem::InsertChild(int index, ProfileSessionDataItem* pChild)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pChild != nullptr)
    {
        ProfileSessionDataItem* pCurrentParent = pChild->GetParent();

        if (pCurrentParent != this)
        {
            if (pCurrentParent != nullptr)
            {
                pCurrentParent->m_children.removeAt(pChild->GetRow());
            }

            m_children.insert(index, pChild);
            pChild->m_pParent = this;
        }
    }
}

ProfileSessionDataItem* ProfileSessionDataItem::GetChild(int childIndex) const
{
    ProfileSessionDataItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((childIndex >= 0) && (childIndex < m_children.size()))
    {
        pRetVal = m_children[childIndex];
    }

    return pRetVal;
}

QVariant ProfileSessionDataItem::GetColumnData(int columnIndex) const
{
    ProfileSessionDataItem::ProfileSessionDataColumnIndex columnIndexId = (ProfileSessionDataItem::ProfileSessionDataColumnIndex)columnIndex;
    int precision = 3;

    switch (columnIndexId)
    {
        case ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_COMMAND_LIST_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_COMMAND_BUFFER_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN:
        {
            if (m_data.count() > columnIndex)
            {
                return m_data[columnIndex];
            }

            return QVariant();
        }

        case ProfileSessionDataItem::SESSION_ITEM_GPU_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_DEVICE_TIME_COLUMN:
        {
            QString retVal;

            if (m_data.count() > columnIndex)
            {
                // calc cpu\gpu time according to start and end time as they appear in the table
                double fnumStart = (double)(m_startTime - m_pSessionDataContainer->SessionTimeRange().first);
                double fnumEnd = (double)(m_endTime - m_pSessionDataContainer->SessionTimeRange().first);
                QString startStr = QString("%1").arg(fnumStart, 0, 'f', precision);
                QString endStr = QString("%1").arg(fnumEnd, 0, 'f', precision);
                double startRounded = startStr.toDouble();
                double endRounded = endStr.toDouble();

                double diff = endRounded - startRounded;
                retVal = NanosecToTimeStringFormatted(diff, true);

            }

            return retVal;
        }

        case ProfileSessionDataItem::SESSION_ITEM_START_TIME_COLUMN:
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
            {
                double diff = (double)(m_startTime - m_pSessionDataContainer->SessionTimeRange().first);
                return NanosecToTimeStringFormatted(diff, true);
            }

            return QVariant();
        }

        case ProfileSessionDataItem::SESSION_ITEM_END_TIME_COLUMN:
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
            {
                double diff = (double)(m_endTime - m_pSessionDataContainer->SessionTimeRange().first);
                return NanosecToTimeStringFormatted(diff, true);

            }
            return QVariant();
        }

        case ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN:
        {
            QString retVal;

            if (m_itemIndex >= 0)
            {
                QString retVal = QString::number(m_itemIndex);

                if (m_endIndex >= 0)
                {
                    retVal.sprintf("%d-%d", m_itemIndex, m_endIndex);
                }

                return retVal;
            }

            return QVariant();
        }

        case ProfileSessionDataItem::SESSION_ITEM_OCCUPANCY_COLUMN:
        {
            if (m_pOccupancyInfo != NULL && m_pOccupancyInfo->GetOccupancy() >= 0)
            {
                return Util::RemoveTrailingZero(QString("%1").number(m_pOccupancyInfo->GetOccupancy(), 'f', 2)).append('%');
            }

            return QVariant();

        }

        default:
        {
            return QVariant();
        }
    }
}


QString ProfileSessionDataItem::GetColumnTooltip(int columnIndex) const
{
    QString retVal;

    switch (columnIndex)
    {
        case ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_GPU_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_DEVICE_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_START_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_END_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_COMMAND_LIST_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_COMMAND_BUFFER_COLUMN:
        {
            retVal = GetColumnData(columnIndex).toString();
            break;
        }

        default:
        {

            break;
        }
    }

    return retVal;
}

void ProfileSessionDataItem::SetColumnData(int columnIndex, QVariant data)
{
    ProfileSessionDataItem::ProfileSessionDataColumnIndex columnIndexId = (ProfileSessionDataItem::ProfileSessionDataColumnIndex)columnIndex;

    switch (columnIndexId)
    {
        case ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_COMMAND_LIST_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_COMMAND_BUFFER_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_PARAMETERS_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_RESULT_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_CPU_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_GPU_TIME_COLUMN:
        case ProfileSessionDataItem::SESSION_ITEM_DEVICE_TIME_COLUMN:
        {
            if (m_data.count() > columnIndex)
            {
                m_data[columnIndex] = data;
            }

            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"Invalid data column index");
            break;
        }
    }
}


int ProfileSessionDataItem::GetRow() const
{
    int retVal = 0;

    if (m_pParent != nullptr)
    {
        retVal = m_pParent->m_children.indexOf(const_cast<ProfileSessionDataItem*>(this));
    }

    return retVal;
}

quint64 ProfileSessionDataItem::StartTimeMilliseconds() const
{
    return TimeMilliseconds(m_startTime);
}

quint64 ProfileSessionDataItem::EndTimeMilliseconds() const
{
    return TimeMilliseconds(m_endTime);
}

quint64 ProfileSessionDataItem::TimeMilliseconds(quint64 time) const
{
    quint64 retVal = time;

    if (m_itemType.m_itemMainType == DX12_API_PROFILE_ITEM || m_itemType.m_itemMainType == DX12_GPU_PROFILE_ITEM)
    {
        retVal = time / GP_NANOSECONDS_TO_MILLISECOND;
    }

    return retVal;
}

bool ProfileSessionDataItem::GetAPIFunctionID(unsigned int& apiID) const
{
    bool retVal = false;
    apiID = 0;

    if (m_itemType.m_itemMainType == CL_API_PROFILE_ITEM)
    {
        GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
        {
            // Downcast the API info class
            CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pCLApiInfo)
            {
                // Get the API id from the CLAPIInfo
                apiID = pCLApiInfo->m_uiAPIID;
                retVal = true;
            }
        }
    }
    else if ((m_itemType.m_itemMainType == HSA_GPU_PROFILE_ITEM) || (m_itemType.m_itemMainType == HSA_API_PROFILE_ITEM))
    {
        GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
        {
            // Downcast the API info class
            HSAAPIInfo* pHSAApiInfo = dynamic_cast<HSAAPIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pHSAApiInfo)
            {
                // Get the API id from the HSAApiInfo
                apiID = pHSAApiInfo->m_apiID;
                retVal = true;
            }
        }
    }

    else if ((m_itemType.m_itemMainType == DX12_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == DX12_GPU_PROFILE_ITEM))
    {
        GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
        {
            // Downcast the API info class
            DX12APIInfo* pDX12ApiInfo = dynamic_cast<DX12APIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pDX12ApiInfo)
            {
                // Get the API id from the DX12APIInfo
                apiID = pDX12ApiInfo->m_apiId;
                retVal = true;
            }
        }
    }

    else if ((m_itemType.m_itemMainType == VK_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == VK_GPU_PROFILE_ITEM))
    {
        GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
        {
            // Downcast the API info class
            VKAPIInfo* pVKApiInfo = dynamic_cast<VKAPIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pVKApiInfo)
            {
                // Get the API id from the DX12APIInfo
                apiID = pVKApiInfo->m_apiId;
                retVal = true;
            }
        }
    }

    return retVal;
}

bool ProfileSessionDataItem::GetEnqueueCommandType(QString& commandType) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApiInfo != nullptr) && ((m_itemType.m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API))
    {
        CLEnqueueAPI* pEnqueueApiInfo = dynamic_cast<CLEnqueueAPI*>(m_pApiInfo);
        GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
        {
            commandType = QString::fromStdString(pEnqueueApiInfo->m_strCMDType);
            retVal = true;
        }
    }
    return retVal;
}

bool ProfileSessionDataItem::GetEnqueueCommandGPUTimes(quint64& gpuStart, quint64& gpuEnd, quint64& gpuQueued, quint64& gpuSubmit)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApiInfo != nullptr) && ((m_itemType.m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API))
    {
        CLEnqueueAPI* pEnqueueApiInfo = dynamic_cast<CLEnqueueAPI*>(m_pApiInfo);
        GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
        {
            gpuStart = pEnqueueApiInfo->m_ullRunning;
            gpuEnd = pEnqueueApiInfo->m_ullComplete;
            gpuQueued = pEnqueueApiInfo->m_ullQueue;
            gpuSubmit = pEnqueueApiInfo->m_ullSubmit;
            retVal = true;
        }
    }
    return retVal;
}

bool ProfileSessionDataItem::GetEnqueueCommandQueueID(unsigned int& contextID, unsigned int& queueID)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApiInfo != nullptr) && ((m_itemType.m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API))
    {
        CLEnqueueAPI* pEnqueueApiInfo = dynamic_cast<CLEnqueueAPI*>(m_pApiInfo);
        GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
        {
            queueID = pEnqueueApiInfo->m_uiQueueID;
            contextID = pEnqueueApiInfo->m_uiContextID;
            retVal = true;
        }
    }
    return retVal;
}

bool ProfileSessionDataItem::GetDispatchCommandWorkSizes(QString& globalWorkSize, QString& groupWorkSize)
{
    bool retVal = false;

    if (m_itemType.IsDispatchCommand() && (m_pApiInfo != nullptr))
    {
        // Sanity check:
        if ((m_itemType.m_itemMainType == CL_API) && ((m_itemType.m_itemSubType & CL_ENQUEUE_KERNEL) == CL_ENQUEUE_KERNEL))
        {
            CLKernelAPIInfo* pEnqueueApiInfo = dynamic_cast<CLKernelAPIInfo*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
            {
                globalWorkSize = QString::fromStdString(pEnqueueApiInfo->m_strGlobalWorkSize);
                groupWorkSize = QString::fromStdString(pEnqueueApiInfo->m_strGroupWorkSize);
                retVal = true;
            }
        }

        else if (m_itemType.m_itemSubType == HSA_API_PROFILE_ITEM)
        {
            HSAAPIInfo* pHSAInfo = dynamic_cast<HSAAPIInfo*>(m_pApiInfo);

            // Sanity check:
            GT_IF_WITH_ASSERT(pHSAInfo != nullptr)
            {
                if (pHSAInfo->m_apiID == HSA_API_Type_Non_API_Dispatch)
                {
                    HSADispatchInfo* pDispatchInfo = dynamic_cast<HSADispatchInfo*>(pHSAInfo);

                    if (pDispatchInfo != nullptr)
                    {
                        globalWorkSize = QString::fromStdString(pDispatchInfo->m_strGlobalWorkSize);
                        groupWorkSize = QString::fromStdString(pDispatchInfo->m_strGroupWorkSize);
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

bool ProfileSessionDataItem::GetDispatchCommandDeviceNames(QString& strQueueHandle, QString& strContextHandle, QString& deviceNameStr)
{
    bool retVal = false;

    if (m_itemType.IsDispatchCommand() && (m_pApiInfo != nullptr))
    {
        // Sanity check:
        if ((m_itemType.m_itemMainType == CL_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == CL_GPU_PROFILE_ITEM))
        {
            CLEnqueueAPI* pEnqueueApiInfo = dynamic_cast<CLEnqueueAPI*>(m_pApiInfo);
            GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
            {
                strQueueHandle = QString::fromStdString(pEnqueueApiInfo->m_strCmdQHandle);
                strContextHandle = QString::fromStdString(pEnqueueApiInfo->m_strCntxHandle);
                deviceNameStr = QString::fromStdString(pEnqueueApiInfo->m_strDevice);
                retVal = true;
            }
        }
        else if (m_itemType.m_itemMainType == HSA_GPU_PROFILE_ITEM)
        {
            HSAAPIInfo* pHSAInfo = dynamic_cast<HSAAPIInfo*>(m_pApiInfo);

            // Sanity check:
            GT_IF_WITH_ASSERT(pHSAInfo != nullptr)
            {
                if (pHSAInfo->m_apiID == HSA_API_Type_Non_API_Dispatch)
                {
                    HSADispatchInfo* pDispatchInfo = dynamic_cast<HSADispatchInfo*>(m_pApiInfo);

                    if (pDispatchInfo != nullptr)
                    {
                        strQueueHandle = QString::fromStdString(pDispatchInfo->m_strQueueHandle);
                        deviceNameStr = QString::fromStdString(pDispatchInfo->m_strDeviceName);
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

bool ProfileSessionDataItem::GetEnqueueOtherDataSize(quint64& dataSize)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApiInfo != nullptr) && ((m_itemType.m_itemSubType & CL_ENQUEUE_DATA_OPERATIONS) == CL_ENQUEUE_DATA_OPERATIONS))
    {
        CLDataEnqueueAPIInfo* pEnqueueApiInfo = dynamic_cast<CLDataEnqueueAPIInfo*>(m_pApiInfo);
        GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
        {
            dataSize = pEnqueueApiInfo->m_uiDataSize;
            retVal = true;
        }
    }
    return retVal;
}

bool ProfileSessionDataItem::GetEnqueueMemTransferSize(quint64& transferSize)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApiInfo != nullptr) && ((m_itemType.m_itemSubType & CL_ENQUEUE_MEM) == CL_ENQUEUE_MEM))
    {
        CLMemAPIInfo* pEnqueueApiInfo = dynamic_cast<CLMemAPIInfo*>(m_pApiInfo);
        GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
        {
            transferSize = pEnqueueApiInfo->m_uiTransferSize;
            retVal = true;
        }
    }
    return retVal;
}

bool ProfileSessionDataItem::GetHSACommandGPUTimes(quint64& gpuStart, quint64& gpuEnd)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
    {
        HSAAPIInfo* pAPIInfo = dynamic_cast<HSAAPIInfo*>(m_pApiInfo);
        GT_IF_WITH_ASSERT((pAPIInfo != nullptr) && (pAPIInfo->m_apiID == HSA_API_Type_Non_API_Dispatch))
        {
            HSADispatchInfo* pDispatchInfo = dynamic_cast<HSADispatchInfo*>(pAPIInfo);

            GT_IF_WITH_ASSERT(pDispatchInfo != nullptr)
            {
                gpuStart = pDispatchInfo->m_ullStart;
                gpuEnd = pDispatchInfo->m_ullEnd;
                retVal = true;
            }
        }
    }
    return retVal;
}

QString ProfileSessionDataItem::QueueName() const
{
    return m_queueName;
}

QString ProfileSessionDataItem::CommandListPointer() const
{
    QString retVal;
    if (m_itemType.m_itemMainType == DX12_GPU_PROFILE_ITEM)
    {
        if (m_pApiInfo != nullptr)
        {
            DX12GPUTraceInfo* pAPIInfo = (DX12GPUTraceInfo*)m_pApiInfo;
            if (pAPIInfo != nullptr)
            {
                retVal = QString::fromStdString(pAPIInfo->m_commandListPtrStr);
            }
        }
    }

    else if (m_itemType.m_itemMainType == VK_GPU_PROFILE_ITEM)
    {
        if (m_pApiInfo != nullptr)
        {
            VKGPUTraceInfo* pAPIInfo = (VKGPUTraceInfo*)m_pApiInfo;
            if (pAPIInfo != nullptr)
            {
                retVal = QString::fromStdString(pAPIInfo->m_commandBufferHandleStr);
            }
        }
    }

    return retVal;
}

QString ProfileSessionDataItem::QueueDisplayName(const QString& queueName)
{
    QString retVal = queueName;
    bool shouldAppendX = false;

    if (retVal.startsWith("0x"))
    {
        retVal = retVal.right(retVal.size() - 2);
        shouldAppendX = true;
    }

    int numZeros = 0;

    for (int i = 0; i < retVal.size(); i++)
    {
        if (retVal.at(i) != '0')
        {
            break;
        }
        else
        {
            numZeros++;
        }
    }

    retVal = retVal.right(retVal.size() - numZeros);

    if (shouldAppendX)
    {
        retVal.prepend("0x");
    }

    return retVal;
}

bool ProfileSessionDataItem::IsCPUItem() const
{
    bool retVal = false;

    if ((m_itemType.m_itemMainType == CL_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == HSA_API_PROFILE_ITEM)
        || (m_itemType.m_itemMainType == DX12_API_PROFILE_ITEM) || (m_itemType.m_itemMainType == VK_API_PROFILE_ITEM))
    {
        retVal = true;
    }

    return retVal;
}

bool ProfileSessionDataItem::IsGPUItem() const
{
    bool retVal = false;

    if ((m_itemType.m_itemMainType == CL_GPU_PROFILE_ITEM) || (m_itemType.m_itemMainType == HSA_GPU_PROFILE_ITEM)
        || (m_itemType.m_itemMainType == DX12_GPU_PROFILE_ITEM) || (m_itemType.m_itemMainType == VK_GPU_PROFILE_ITEM))
    {
        retVal = true;
    }

    return retVal;
}

bool ProfileSessionDataItem::GetDisplaySeqID(unsigned int& seqID) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApiInfo != nullptr)
    {
        seqID = m_pApiInfo->m_uiDisplaySeqID;
        retVal = true;
    }
    return retVal;
}

bool ProfileSessionDataItem::GetDispatchCommandKernelName(QString& kernelNameStr)
{
    bool retVal = false;

    // Sanity check:
    if ((m_pApiInfo != nullptr) && ((m_itemType.m_itemSubType & CL_ENQUEUE_KERNEL) == CL_ENQUEUE_KERNEL))
    {
        CLKernelAPIInfo* pEnqueueApiInfo = dynamic_cast<CLKernelAPIInfo*>(m_pApiInfo);
        GT_IF_WITH_ASSERT(pEnqueueApiInfo != nullptr)
        {
            kernelNameStr = QString::fromStdString(pEnqueueApiInfo->m_strKernelName);
            retVal = true;
        }
    }

    else if ((m_pApiInfo != nullptr) && (m_itemType.m_itemMainType == HSA_GPU_PROFILE_ITEM))
    {
        HSAAPIInfo* pHSAInfo = dynamic_cast<HSAAPIInfo*>(m_pApiInfo);

        // Sanity check:
        GT_IF_WITH_ASSERT(pHSAInfo != nullptr)
        {
            if (pHSAInfo->m_apiID == HSA_API_Type_Non_API_Dispatch)
            {
                HSADispatchInfo* pDispatchInfo = dynamic_cast<HSADispatchInfo*>(pHSAInfo);

                if (pDispatchInfo != nullptr)
                {
                    kernelNameStr = QString::fromStdString(pDispatchInfo->m_strKernelName);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

bool ProfileSessionDataItem::GetListOfColumnIndices(ProfileItemAPIType apiItemType, QVector<ProfileSessionDataColumnIndex>& columns)
{
    bool retVal = false;

    // Init static members if not initialized yet
    InitStaticMembers();

    if (m_sItemTypesColumnsMap.contains(apiItemType))
    {
        columns = m_sItemTypesColumnsMap[apiItemType];
    }

    return retVal;
}

void ProfileSessionDataItem::InitStaticMembers()
{
    if (!m_sAreStaticMembersInitialized)
    {
        m_sAreStaticMembersInitialized = true;

        // CL API table columns
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_INDEX_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_INTERFACE_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_PARAMETERS_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_RESULT_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_DEVICE_BLOCK_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_OCCUPANCY_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_CPU_TIME_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_DEVICE_TIME_COLUMN;

        // HSA API table columns
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_INDEX_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_INTERFACE_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_PARAMETERS_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_RESULT_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_DEVICE_BLOCK_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_OCCUPANCY_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_CPU_TIME_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_DEVICE_TIME_COLUMN;

        // DX12 API table columns
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_INDEX_COLUMN;
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_INTERFACE_COLUMN;
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_CALL_COLUMN;
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_PARAMETERS_COLUMN;
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_CPU_TIME_COLUMN;
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_RESULT_COLUMN;

        // DX12 GPU table columns
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_INDEX_COLUMN;
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_COMMAND_LIST_COLUMN;
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_CALL_COLUMN;
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_PARAMETERS_COLUMN;
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_GPU_TIME_COLUMN;
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_RESULT_COLUMN;

        // Vulkan API table columns
        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_INDEX_COLUMN;
        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_CALL_COLUMN;
        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_PARAMETERS_COLUMN;
        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_CPU_TIME_COLUMN;
        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_RESULT_COLUMN;

        // Vulkan GPU table columns
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_INDEX_COLUMN;
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_COMMAND_BUFFER_COLUMN;
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_CALL_COLUMN;
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_PARAMETERS_COLUMN;
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_GPU_TIME_COLUMN;
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_RESULT_COLUMN;

#ifdef DEBUG_TABLE_DATA
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_START_TIME_COLUMN;
        m_sItemTypesColumnsMap[CL_API_PROFILE_ITEM] << SESSION_ITEM_END_TIME_COLUMN;

        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_START_TIME_COLUMN;
        m_sItemTypesColumnsMap[HSA_API_PROFILE_ITEM] << SESSION_ITEM_END_TIME_COLUMN;

        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_START_TIME_COLUMN;
        m_sItemTypesColumnsMap[DX12_API_PROFILE_ITEM] << SESSION_ITEM_END_TIME_COLUMN;

        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_START_TIME_COLUMN;
        m_sItemTypesColumnsMap[DX12_GPU_PROFILE_ITEM] << SESSION_ITEM_END_TIME_COLUMN;

        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_START_TIME_COLUMN;
        m_sItemTypesColumnsMap[VK_API_PROFILE_ITEM] << SESSION_ITEM_END_TIME_COLUMN;

        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_START_TIME_COLUMN;
        m_sItemTypesColumnsMap[VK_GPU_PROFILE_ITEM] << SESSION_ITEM_END_TIME_COLUMN;
#endif


        // Initialize the captions map
        m_sItemTypesToTitleMap[SESSION_ITEM_INDEX_COLUMN] = GP_STR_TraceTableColumnIndex;
        m_sItemTypesToTitleMap[SESSION_ITEM_INTERFACE_COLUMN] = GP_STR_TraceTableColumnInterface;
        m_sItemTypesToTitleMap[SESSION_ITEM_COMMAND_LIST_COLUMN] = GP_STR_TraceTableColumnCommandList;
        m_sItemTypesToTitleMap[SESSION_ITEM_COMMAND_BUFFER_COLUMN] = GP_STR_TraceTableColumnCommandBuffer;
        m_sItemTypesToTitleMap[SESSION_ITEM_CALL_COLUMN] = GP_STR_TraceTableColumnCall;
        m_sItemTypesToTitleMap[SESSION_ITEM_PARAMETERS_COLUMN] = GP_STR_TraceTableColumnParameters;
        m_sItemTypesToTitleMap[SESSION_ITEM_RESULT_COLUMN] = GP_STR_TraceTableColumnResult;
        m_sItemTypesToTitleMap[SESSION_ITEM_DEVICE_BLOCK_COLUMN] = GP_STR_TraceTableColumnDeviceBlock;
        m_sItemTypesToTitleMap[SESSION_ITEM_OCCUPANCY_COLUMN] = GP_STR_TraceTableColumnKernelOccupancy;
        m_sItemTypesToTitleMap[SESSION_ITEM_CPU_TIME_COLUMN] = GP_STR_TraceTableColumnCPUTime;
        m_sItemTypesToTitleMap[SESSION_ITEM_GPU_TIME_COLUMN] = GP_STR_TraceTableColumnGPUTime;
        m_sItemTypesToTitleMap[SESSION_ITEM_DEVICE_TIME_COLUMN] = GP_STR_TraceTableColumnDeviceTime;
        m_sItemTypesToTitleMap[SESSION_ITEM_START_TIME_COLUMN] = GP_STR_TraceTableColumnStartTime;
        m_sItemTypesToTitleMap[SESSION_ITEM_END_TIME_COLUMN] = GP_STR_TraceTableColumnEndTime;


    }
}

QString ProfileSessionDataItem::ColumnIndexTitle(ProfileSessionDataColumnIndex colIndex)
{
    QString retVal;
    InitStaticMembers();

    GT_IF_WITH_ASSERT(m_sItemTypesToTitleMap.contains(colIndex))
    {
        retVal = m_sItemTypesToTitleMap[colIndex];
    }

    return retVal;
}

bool ProfileSessionDataItem::ProfileItemType::IsDispatchCommand() const
{
    bool retVal = false;

    if ((m_itemMainType == CL_API_PROFILE_ITEM) && ((m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API))
    {
        retVal = true;
    }
    else if ((m_itemMainType == CL_GPU_PROFILE_ITEM) && ((m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API))
    {
        retVal = true;
    }
    else if (m_itemMainType == HSA_GPU_PROFILE_ITEM)
    {
        retVal = true;
    }

    return retVal;
}

bool ProfileSessionDataItem::ProfileItemType::operator<(const ProfileItemType& other) const
{
    bool retVal = false;

    if (m_itemMainType < other.m_itemMainType)
    {
        retVal = true;
    }
    else if (m_itemSubType < other.m_itemSubType)
    {
        retVal = true;
    }

    return retVal;
}


void ProfileSessionDataItem::UpdateIndices(int childStartIndex, int childEndIndex)
{
    if ((childEndIndex < 0) && (childStartIndex >= 0))
    {
        if (m_itemIndex < 0)
        {
            m_itemIndex = childStartIndex;
        }
        else
        {
            if (m_itemIndex > childStartIndex)
            {
                m_itemIndex = childStartIndex;
            }
        }

        if (m_endIndex < 0)
        {
            m_endIndex = childStartIndex;
        }
        else
        {
            if (m_endIndex < childStartIndex)
            {
                m_endIndex = childStartIndex;
            }
        }
    }

    // Go through the parents, and update its indices, until you get to the root item
    ProfileSessionDataItem* pParent = m_pParent;

    while (pParent != nullptr)
    {
        if (pParent->m_itemIndex < 0)
        {
            pParent->m_itemIndex = m_itemIndex;
        }

        if (pParent->m_endIndex < 0)
        {
            pParent->m_endIndex = m_endIndex;
        }
        else
        {
            if (pParent->m_endIndex < m_endIndex)
            {
                pParent->m_endIndex = m_endIndex;
            }
        }

        pParent = pParent->m_pParent;
    }
}
