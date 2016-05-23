//------------------------------ gpTimeline.cpp ------------------------------

#include <CL/cl.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/CLTimelineItems.h>
#include <AMDTGpuProfiling/DXTimelineItems.h>
#include <AMDTGpuProfiling/HSATimelineItems.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpTimeline.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpNavigationRibbon.h>
#include <AMDTGpuProfiling/gpRibbonDataCalculator.h>

#define DASH_SIZE 5

gpTimeline::gpTimeline(QWidget* pParent, gpTraceView* pSessionView) : acTimeline(pParent),
    m_pSessionDataContainer(nullptr),
    m_pSessionView(pSessionView),
    m_pOpenCLBranch(nullptr),
    m_pHSABranch(nullptr),
    m_pCPUTimelineBranch(nullptr),
    m_pGPUTimelineBranch(nullptr)
{
    // Since we deal with a single frame, we want the grid to draw precise time labels
    SetGridLabelsPrecision(5);

    // Set the background color to white
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);

    ShowTimeLineTooltips(false);

    bool rc = connect(this, SIGNAL(VisibilityFilterChanged(QMap<QString, bool>&)), this, SLOT(OnVisibilityFilterChanged(QMap<QString, bool>&)));
    GT_ASSERT(rc);

    m_presentData << -1;
    m_presentData << -1;

    // Draw the children on the parent branches
    m_shouldDisplayChildrenInParentBranch = true;
}

gpTimeline::~gpTimeline()
{
}

void gpTimeline::BuildTimeline(gpTraceDataContainer* pDataContainer)
{
    // Set the data container
    m_pSessionDataContainer = pDataContainer;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        int threadsCount = m_pSessionDataContainer->ThreadsCount();

        for (int i = 0; i < threadsCount; i++)
        {
            // Get the thread id
            osThreadId threadId = m_pSessionDataContainer->ThreadID(i);

            // Add the API for this thread
            AddThreadAPIFunctions(threadId);

            // Add the performance markers to the timeline
            AddPerformanceMarkersToTimeline(threadId);
        }

        int queuesCount = m_pSessionDataContainer->GPUCallsContainersCount();

        for (int i = 0; i < queuesCount; i++)
        {
            // Get the queue name
            QString queueName = m_pSessionDataContainer->GPUObjectName(i);

            // Add the API for this queue
            AddQueueGPUApis(queueName);
        }

        // Add the GPU items for this trace session
        AddGPUItemsToTimeline();

        // Add the frame command lists to the frame timeline
        AddCommandListsToTimeline();

        // Add the created branches and sub-branches. NOTICE: This can only be done at the end, since the start & end time are updated
        // from a branch to its parent
        if (!m_cpuBranchesMap.isEmpty())
        {
            for (auto i = m_cpuBranchesMap.begin(); i != m_cpuBranchesMap.end(); ++i)
            {
                m_pCPUTimelineBranch->addSubBranch(*i);
            }

            addBranch(m_pCPUTimelineBranch);

            m_cpuBranchesMap.clear();
        }

        // Add the created branches and sub-branches. NOTICE: This can only be done at the end, since the start & end time are updated
        // from a branch to its parent
        if (!m_gpuBranchesMap.isEmpty())
        {
            for (auto i = m_gpuBranchesMap.begin(); i != m_gpuBranchesMap.end(); ++i)
            {
                QueueBranches queueBranches = i.value();

                if (queueBranches.m_pQueueRootBranch == nullptr)
                {

                    // Get the queue branch name
                    QString queueDisplayName = m_pSessionDataContainer->QueueDisplayName(i.key());

                    // Create the queue root branch and add the API and command lists / buffers branches
                    queueBranches.m_pQueueRootBranch = new acTimelineBranch();
                    queueBranches.m_pQueueRootBranch->setText(queueDisplayName);

                    queueBranches.m_pQueueRootBranch->addSubBranch(queueBranches.m_pQueueAPIBranch);
                    queueBranches.m_pQueueRootBranch->addSubBranch(queueBranches.m_pQueueCommandListsBranch);

                    m_pGPUTimelineBranch->addSubBranch(queueBranches.m_pQueueRootBranch);
                }
            }

            addBranch(m_pGPUTimelineBranch);

            m_gpuBranchesMap.clear();
        }

        if (!m_oclCtxMap.isEmpty())
        {
            for (QMap<unsigned int, acTimelineBranch*>::const_iterator i = m_oclCtxMap.begin(); i != m_oclCtxMap.end(); ++i)
            {
                m_pOpenCLBranch->addSubBranch(*i);
            }

            addBranch(m_pOpenCLBranch);
            m_oclCtxMap.clear();
        }

        if (!m_oclQueueMap.isEmpty())
        {
            for (QMap<unsigned int, OCLQueueBranchInfo*>::iterator i = m_oclQueueMap.begin(); i != m_oclQueueMap.end(); ++i)
            {
                SAFE_DELETE(*i);
            }

            m_oclQueueMap.clear();
        }

        if (!m_hsaQueueMap.isEmpty())
        {
            for (QMap<QString, acTimelineBranch*>::const_iterator i = m_hsaQueueMap.begin(); i != m_hsaQueueMap.end(); ++i)
            {
                m_pHSABranch->addSubBranch(*i);
            }

            addBranch(m_pHSABranch);
            m_hsaQueueMap.clear();
        }
    }
    UpdateCPUCaption();

    ShouldScrollToEnd(true);
}

void gpTimeline::AddThreadAPIFunctions(osThreadId threadId)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        int apiCount = m_pSessionDataContainer->ThreadAPICount(threadId);
        afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingAPITimelineItems, apiCount);

        for (int i = 0; i < apiCount; i++)
        {
            // Get the current API item
            ProfileSessionDataItem* pItem = m_pSessionDataContainer->APIItem(threadId, i);

            // Sanity check:
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {

                // Create a timeline item according to the API Id
                acAPITimelineItem* pAPITimelineItem = CreateTimelineItem(pItem);

                // Sanity check:
                GT_IF_WITH_ASSERT(m_pSessionView != nullptr)
                {
                    bool isValid = false;
                    int apiIndex = pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN).toInt(&isValid);

                    if (isValid)
                    {
                        APICallId apiId;
                        apiId.m_tid = threadId;
                        apiId.m_callIndex = apiIndex;
                        m_pSessionView->SetAPICallTimelineItem(apiId, pAPITimelineItem);
                    }
                }

                afProgressBarWrapper::instance().incrementProgressBar();
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();
    }
}

void gpTimeline::AddQueueGPUApis(const QString& queueName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        int apiCount = m_pSessionDataContainer->QueueItemsCount(queueName);
        afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingAPITimelineItems, apiCount);

        for (int i = 0; i < apiCount; i++)
        {
            // Get the current API item
            ProfileSessionDataItem* pItem = m_pSessionDataContainer->QueueItem(queueName, i);

            // Sanity check:
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {

                // Create a timeline item according to the API Id
                acAPITimelineItem* pAPITimelineItem = CreateTimelineItem(pItem);

                // Sanity check:
                GT_IF_WITH_ASSERT(m_pSessionView != nullptr)
                {
                    bool isValid = false;
                    int apiIndex = pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_INDEX_COLUMN).toInt(&isValid);

                    if (isValid)
                    {
                        APICallId apiId;
                        apiId.m_queueName = queueName;
                        apiId.m_callIndex = apiIndex;
                        apiId.m_tid = 0;
                        m_pSessionView->SetAPICallTimelineItem(apiId, pAPITimelineItem);
                    }
                }

                afProgressBarWrapper::instance().incrementProgressBar();
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();

    }
}


void gpTimeline::AddCommandListsToTimeline()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        // Get the command lists data from the session data container
        const QVector<gpTraceDataContainer::CommandListInstanceData>& commandListsData = m_pSessionDataContainer->CommandListsData();

        int commandListIndex = 0;
        auto iter = commandListsData.begin();
        auto iterEnd = commandListsData.end();
        for (; iter != iterEnd; iter++)
        {
            QString commandListName = (*iter).m_commandListPtr;
            int instnaceIndex = (*iter).m_instanceIndex;

            // Make sure that the queue name is not empty
            QString queueName = (*iter).m_commandListQueueName;
            if (!queueName.isEmpty())
            {
                acTimelineBranch* pQueueBranch = GetCommandListBranch(queueName);
                GT_IF_WITH_ASSERT(pQueueBranch != nullptr)
                {
                    // Create the command list timeline item
                    CommandListTimelineItem* pNewItem = new CommandListTimelineItem((*iter).m_startTime, (*iter).m_endTime, commandListName);

                    // Set the command list name as the item text
                    QString commandListDisplayName = m_pSessionDataContainer->CommandListNameFromPointer(commandListName, instnaceIndex);
                    pNewItem->setText(commandListDisplayName);

                    // Get the color for this command list by its index
                    QColor commandListColor = APIColorMap::Instance()->GetCommandListColor(commandListIndex);
                    pNewItem->setBackgroundColor(commandListColor);

                    // Add the item to the queue branch
                    pQueueBranch->addTimelineItem(pNewItem);

                    // Index for coloring
                    commandListIndex++;

                    m_cmdListTimelineItemMap.insert(commandListName, pNewItem);
                }
            }
        }
    }
}

void gpTimeline::AddGPUItemsToTimeline()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        int gpuItemsCount = m_pSessionDataContainer->QueueItemsCount();
        afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingGPUTimelineItems, gpuItemsCount);

        for (int i = 0; i < gpuItemsCount; i++)
        {
            // Get the current API item
            ProfileSessionDataItem* pItem = m_pSessionDataContainer->GpuItem(i);

            // Sanity check:
            GT_IF_WITH_ASSERT((pItem != nullptr) && (m_pSessionView != nullptr))
            {
                acAPITimelineItem* pAPITimelineItem = nullptr;

                if (pItem->OwnerAPIItem() != nullptr)
                {
                    // Find the owner API timeline item
                    APICallId apiId;
                    apiId.m_tid = pItem->OwnerAPIItem()->ThreadID();
                    apiId.m_callIndex = pItem->OwnerAPIItem()->APICallIndex();
                    pAPITimelineItem = m_pSessionView->GetAPITimelineItem(apiId);
                }

                acAPITimelineItem* pGPUItem = nullptr;

                if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::CL_GPU_PROFILE_ITEM)
                {
                    // Create a timeline item according to the API Id
                    pGPUItem = CreateCLGPUItem(pItem, pAPITimelineItem);
                }

                if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::HSA_GPU_PROFILE_ITEM)
                {
                    // Create a timeline item according to the API Id
                    pGPUItem = CreateHSAGPUItem(pItem);
                }

                // Sanity check:
                GT_IF_WITH_ASSERT(pGPUItem != nullptr)
                {
                    if (pItem->OwnerAPIItem() != nullptr)
                    {
                        APICallId apiId;
                        apiId.m_tid = pItem->OwnerAPIItem()->ThreadID();
                        apiId.m_callIndex = pItem->OwnerAPIItem()->APICallIndex();
                        m_pSessionView->SetGPUTimelineItem(apiId, pGPUItem);
                    }
                }

                afProgressBarWrapper::instance().incrementProgressBar();
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();

    }
}

void gpTimeline::AddPerformanceMarkersToTimeline(osThreadId threadId)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
    {
        // Get the markers count for this thread
        int markersCount = m_pSessionDataContainer->ThreadPerfMarkersCount(threadId);

        afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingPerfMarkersTimelineItems, markersCount);

        for (int i = 0; i < markersCount; i++)
        {
            // Get the current API item
            ProfileSessionDataItem* pItem = m_pSessionDataContainer->PerfMarkerItem(threadId, i);

            // Create a timeline item for the performance marker Id
            CreatePerfMarkerTimelineItem(pItem);

            afProgressBarWrapper::instance().incrementProgressBar();
        }

        afProgressBarWrapper::instance().hideProgressBar();
    }
}

acAPITimelineItem* gpTimeline::CreateTimelineItem(ProfileSessionDataItem* pItem)
{
    acAPITimelineItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        // Create the API item according to its type
        quint64 startTime = pItem->StartTime();
        quint64 endTime = pItem->EndTime();

        // Create a timeline item
        QString apiName = pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_CALL_COLUMN).toString();

        bool isEnqueue = false, isEnququeMem = false, isEnququeOther = false;

        if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::CL_API_PROFILE_ITEM)
        {
            isEnqueue = (pItem->ItemType().m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API;
            isEnququeMem = (pItem->ItemType().m_itemSubType & CL_ENQUEUE_MEM) == CL_ENQUEUE_MEM;
            isEnququeOther = (pItem->ItemType().m_itemSubType & CL_ENQUEUE_OTHER_OPERATIONS) == CL_ENQUEUE_OTHER_OPERATIONS;
        }

        unsigned int functionId = 0;
        GT_IF_WITH_ASSERT(pItem->GetAPIFunctionID(functionId))
        {
            if (functionId == CL_FUNC_TYPE_clGetEventInfo)
            {
                pRetVal = new CLGetEventInfoTimelineItem(startTime, endTime, pItem->APICallIndex());
            }
            else
            {
                if (isEnqueue || isEnququeMem || isEnququeOther)
                {
                    pRetVal = new DispatchAPITimelineItem(apiName);
                    pRetVal->setStartTime(startTime);
                    pRetVal->setEndTime(endTime);
                    pRetVal->setApiIndex(pItem->APICallIndex());
                }
                else
                {
                    bool isDX = (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM) || (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM);
                    bool isVK = (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::VK_API_PROFILE_ITEM) || (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::VK_GPU_PROFILE_ITEM);

                    if (isDX || isVK)
                    {
                        // Create a new DX API item and set the interface name
                        gpAPITimelineItem* pNewDXItem = new gpAPITimelineItem(startTime, endTime, pItem->APICallIndex());
                        pNewDXItem->SetInterfaceName(pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN).toString());
                        pRetVal = pNewDXItem;
                    }
                    else
                    {
                        pRetVal = new APITimelineItem(startTime, endTime, pItem->APICallIndex());
                    }
                }
            }

            // Set the colors of the item
            QColor color = APIColorMap::Instance()->GetAPIColor(pItem->ItemType(), functionId, QColor(90, 90, 90));

            pRetVal->setBackgroundColor(color);
            pRetVal->setForegroundColor(Qt::white);
            pRetVal->setText(apiName);

            QString gpuObjectName = pItem->QueueName();

            // Create an HSA / OpenCL branch for the API function
            QString branchText = GPU_STR_TraceViewOpenCL;

            if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::HSA_API_PROFILE_ITEM)
            {
                branchText = GPU_STR_TraceViewHSA;
            }
            else if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
            {
                branchText = GPU_STR_TraceViewThread;
            }
            else if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM)
            {
                branchText = GPU_STR_TraceViewGPU;
            }
            else if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::VK_API_PROFILE_ITEM)
            {
                branchText = GPU_STR_TraceViewThread;
            }
            else if (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::VK_GPU_PROFILE_ITEM)
            {
                branchText = GPU_STR_TraceViewGPU;
            }

            acTimelineBranch* pHostBranch = GetBranchForAPI(pItem->ThreadID(), gpuObjectName, branchText, pItem->ItemType().m_itemMainType);
            GT_IF_WITH_ASSERT(pHostBranch != nullptr)
            {
                pHostBranch->addTimelineItem(pRetVal);
            }
        }
    }

    return pRetVal;
}

acTimelineItem* gpTimeline::CreatePerfMarkerTimelineItem(ProfileSessionDataItem* pItem)
{
    acTimelineItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        // Create the API item according to its type
        quint64 startTime = pItem->StartTime();
        quint64 endTime = pItem->EndTime();

        // Create a timeline item
        QString markerName = pItem->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_INTERFACE_COLUMN).toString();
        pRetVal = new PerfMarkerTimelineItem(startTime, endTime);

        // Set the colors of the item
        pRetVal->setBackgroundColor(APIColorMap::Instance()->GetPerfMarkersColor());
        pRetVal->setForegroundColor(Qt::white);
        pRetVal->setText(markerName);

        // Create an HSA / OpenCL branch for the API function
        QString branchText = (pItem->ItemType().m_itemMainType == ProfileSessionDataItem::CL_API_PROFILE_ITEM) ? GPU_STR_TraceViewOpenCL : GPU_STR_TraceViewHSA;
        acTimelineBranch* pHostBranch = GetBranchForAPI(pItem->ThreadID(), pItem->QueueName(), branchText, pItem->ItemType().m_itemMainType);
        GT_IF_WITH_ASSERT(pHostBranch != nullptr)
        {
            pHostBranch->addTimelineItem(pRetVal);
        }
    }

    return pRetVal;
}


gpTimeline::OCLQueueBranchInfo* gpTimeline::GetBranchInfo(unsigned int contextId, unsigned int queueId, const QString& strContextHandle, const QString& deviceNameStr, const QString& strQueueHandle)
{
    OCLQueueBranchInfo* pRetVal = nullptr;
    acTimelineBranch* pContextBranch = nullptr;

    // Create OpenCL branch if it does not yet exist
    if (m_pOpenCLBranch == nullptr)
    {
        m_pOpenCLBranch = new acTimelineBranch;
        m_pOpenCLBranch->setText(GPU_STR_TraceViewOpenCL);
    }

    if (m_oclCtxMap.contains(contextId))
    {
        pContextBranch = m_oclCtxMap[contextId];
    }
    else
    {
        pContextBranch = new acTimelineBranch;
        pContextBranch->setText(QString(GPU_STR_timeline_ContextBranchName).arg(contextId).arg(strContextHandle));
        m_oclCtxMap[contextId] = pContextBranch;
    }

    if (m_oclQueueMap.contains(queueId))
    {
        pRetVal = m_oclQueueMap[queueId];

        if (!deviceNameStr.isEmpty() && !strQueueHandle.isEmpty())
        {
            pRetVal->m_pQueueBranch->setText(QString(GPU_STR_timeline_ContextBranchNameWithParam).arg(queueId).arg(deviceNameStr).arg(strQueueHandle));
        }
    }
    else
    {
        pRetVal = new OCLQueueBranchInfo;


        // Create the queue branch
        pRetVal->m_pQueueBranch = new acTimelineBranch;
        pRetVal->m_pQueueBranch->setText(QString(tr("Queue %1 - %2 (%3)")).arg(queueId).arg(deviceNameStr).arg(strQueueHandle));

        pRetVal->m_pKernelBranch = new acTimelineBranch;
        pRetVal->m_pKernelBranch->setText(tr("Kernel Execution"));

        pRetVal->m_pMemoryBranch = new acTimelineBranch;
        pRetVal->m_pMemoryBranch->setText(tr("Data Transfer"));

        // Notice:
        // we do not initialize the fill operations branch by default. We only do it when there are fill operation functions

        pRetVal->m_pQueueBranch->addSubBranch(pRetVal->m_pMemoryBranch);
        pRetVal->m_pQueueBranch->addSubBranch(pRetVal->m_pKernelBranch);

        pContextBranch->addSubBranch(pRetVal->m_pQueueBranch);

        m_oclQueueMap[queueId] = pRetVal;
    }

    return pRetVal;
}


acTimelineBranch* gpTimeline::GetHostBranchForPerfMarker(osThreadId threadId, ProfileSessionDataItem* pItem)
{
    GT_UNREFERENCED_PARAMETER(threadId);
    GT_UNREFERENCED_PARAMETER(pItem);
#pragma message ("TODO: FA: implement")
    acTimelineBranch* pRetVal = nullptr;
    return pRetVal;
}

acTimelineBranch* gpTimeline::GetBranchForAPI(osThreadId threadId, const QString& queueName, const QString& branchText, ProfileSessionDataItem::ProfileItemAPIType itemType)
{
    acTimelineBranch* pRetVal = nullptr;

    if (m_pCPUTimelineBranch == nullptr)
    {
        m_pCPUTimelineBranch = new acTimelineBranch;
        m_pCPUTimelineBranch->setText(tr(GPU_STR_TraceViewCPU));
    }

    if (m_pGPUTimelineBranch == nullptr)
    {
        m_pGPUTimelineBranch = new acTimelineBranch;
        m_pGPUTimelineBranch->setText(tr(GPU_STR_TraceViewGPU));
    }

    if ((itemType == ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM) || (itemType == ProfileSessionDataItem::VK_GPU_PROFILE_ITEM))
    {
        QString queueTypeStr;

        // Create the queue table name
        QString queueDisplayName = m_pSessionDataContainer->QueueNameFromPointer(queueName);

        GT_IF_WITH_ASSERT(m_pSessionDataContainer != nullptr)
        {
            if (itemType == ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM)
            {
                int queueType = m_pSessionDataContainer->QueueType(queueName);
                queueTypeStr = gpTraceDataContainer::CommandListTypeAsString(queueType);
                queueDisplayName.prepend(AF_STR_SpaceA);
                queueDisplayName.prepend(queueTypeStr);
            }
        }

        // Get or create the queue root branch
        acTimelineBranch* pRootBranch = nullptr;

        if (m_gpuBranchesMap.contains(queueName))
        {
            pRootBranch = m_gpuBranchesMap[queueName].m_pQueueAPIBranch;
        }

        if (pRootBranch == nullptr)
        {
            pRootBranch = new acTimelineBranch;
            pRootBranch->SetBGColor(QColor::fromRgb(230, 230, 230));
            m_gpuBranchesMap[queueName].m_pQueueAPIBranch = pRootBranch;

            pRootBranch->setText(GPU_STR_timeline_QueueAPICallsBranchName);
        }

        pRetVal = pRootBranch;
    }
    else
    {
        // first add the root branch, if necessary
        QString cpuThreadLabel;
        gtString ThreadIdAsString;
        osThreadIdAsString(threadId, ThreadIdAsString);

        if (!branchText.isEmpty())
        {
            cpuThreadLabel = QString(branchText).append(" ").append(acGTStringToQString(ThreadIdAsString));
        }
        else
        {
            cpuThreadLabel = acGTStringToQString(ThreadIdAsString);
        }

        acTimelineBranch* pRootBranch = nullptr;

        if (m_cpuBranchesMap.contains(threadId))
        {
            pRootBranch = m_cpuBranchesMap[threadId];
        }

        if (pRootBranch == nullptr)
        {
            pRootBranch = new acTimelineBranch;
            pRootBranch->SetBGColor(QColor::fromRgb(230, 230, 230));

            pRootBranch->setText(cpuThreadLabel);
            m_cpuBranchesMap.insert(threadId, pRootBranch);
        }

        pRetVal = pRootBranch;
    }

    return pRetVal;
}


acTimelineBranch* gpTimeline::GetCommandListBranch(const QString& queueName)
{
    acTimelineBranch* pRetVal = nullptr;

    // Add or get the queue branch to the command lists branches as well
    if (m_gpuBranchesMap.contains(queueName))
    {
        pRetVal = m_gpuBranchesMap[queueName].m_pQueueCommandListsBranch;
    }

    if (pRetVal == nullptr)
    {
        m_gpuBranchesMap[queueName].m_pQueueCommandListsBranch = new acTimelineBranch;
        pRetVal = m_gpuBranchesMap[queueName].m_pQueueCommandListsBranch;
        pRetVal->SetBGColor(QColor::fromRgb(230, 230, 230));

        QString branchName = (m_pSessionDataContainer->SessionAPIType() == ProfileSessionDataItem::DX12_API_PROFILE_ITEM) ? GPU_STR_timeline_CmdListsBranchName : GPU_STR_timeline_CmdBuffersBranchName;
        pRetVal->setText(branchName);
    }

    return pRetVal;
}

acAPITimelineItem* gpTimeline::CreateCLGPUItem(ProfileSessionDataItem* pGPUIDataItem, acAPITimelineItem* pAPITimelineItem)
{
    acAPITimelineItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pGPUIDataItem != nullptr)
    {
        // Only GPU Items should be here
        GT_IF_WITH_ASSERT(pGPUIDataItem->ItemType().m_itemMainType == ProfileSessionDataItem::CL_GPU_PROFILE_ITEM)
        {
            QString globalWorkSize, groupWorkSize;
            QString strCmdType, strQueueHandle, strContextHandle, deviceNameStr;
            quint64 gpuStart = 0, gpuEnd = 0, gpuQueued = 0, gpuSubmit = 0;

            pGPUIDataItem->GetDispatchCommandWorkSizes(globalWorkSize, groupWorkSize);
            pGPUIDataItem->GetDispatchCommandDeviceNames(strQueueHandle, strContextHandle, deviceNameStr);

            // For CL GPU items - we always expect to get an API owner item
            GT_IF_WITH_ASSERT((pGPUIDataItem->OwnerAPIItem() != nullptr) && (pAPITimelineItem != nullptr))
            {
                ProfileSessionDataItem::GPUItemType gpuItemType = (ProfileSessionDataItem::GPUItemType)pGPUIDataItem->ItemType().m_itemSubType;
                bool isEnqueue = (pGPUIDataItem->OwnerAPIItem()->ItemType().m_itemSubType & CL_ENQUEUE_BASE_API) == CL_ENQUEUE_BASE_API;
                bool isEnququeMem = (gpuItemType == ProfileSessionDataItem::GPU_MEMORY_ITEM);
                bool isEnququeKernel = (gpuItemType == ProfileSessionDataItem::GPU_KERNEL_ITEM);
                bool isEnququeOther = (gpuItemType == ProfileSessionDataItem::GPU_OTHER_ITEM);

                unsigned int queueID = 0, contextID = 0;

                // This is an enqueue API
                if (isEnqueue)
                {
                    // Get the data from the profiled session item
                    bool rcData = false;
                    rcData = pGPUIDataItem->OwnerAPIItem()->GetEnqueueCommandType(strCmdType);
                    rcData = rcData && pGPUIDataItem->OwnerAPIItem()->GetEnqueueCommandGPUTimes(gpuStart, gpuEnd, gpuQueued, gpuSubmit);
                    rcData = rcData && pGPUIDataItem->OwnerAPIItem()->GetEnqueueCommandQueueID(contextID, queueID);


                    GT_IF_WITH_ASSERT(rcData)
                    {
                        // Get or create the branch for this queue:
                        OCLQueueBranchInfo* pBranchInfo = GetBranchInfo(contextID, queueID, strContextHandle, deviceNameStr, strQueueHandle);

                        if (isEnququeKernel)
                        {
                            // Sanity check:
                            GT_IF_WITH_ASSERT(rcData)
                            {
                                CLKernelTimelineItem* pKernelItem = new CLKernelTimelineItem(gpuStart, gpuEnd, pGPUIDataItem->OwnerAPIItem()->APICallIndex());

                                pKernelItem->setGlobalWorkSize(globalWorkSize);
                                pKernelItem->setLocalWorkSize(groupWorkSize);

                                pBranchInfo->m_pKernelBranch->addTimelineItem(pKernelItem);
                                pRetVal = pKernelItem;
                            }
                        }
                        else if (isEnququeMem)
                        {
                            quint64 transferSize = 0;
                            rcData = rcData && pGPUIDataItem->OwnerAPIItem()->GetEnqueueMemTransferSize(transferSize);
                            GT_IF_WITH_ASSERT(rcData && (pBranchInfo != nullptr) && (pBranchInfo->m_pMemoryBranch != nullptr))
                            {
                                pRetVal = new CLMemTimelineItem(gpuStart, gpuEnd, pGPUIDataItem->OwnerAPIItem()->APICallIndex());

                                ((CLMemTimelineItem*)pRetVal)->setDataTransferSize(transferSize);
                                pBranchInfo->m_pMemoryBranch->addTimelineItem(pRetVal);
                            }
                        }
                        else if (isEnququeOther)
                        {
                            if ((pGPUIDataItem->ItemType().m_itemSubType & CL_ENQUEUE_DATA_OPERATIONS) == CL_ENQUEUE_DATA_OPERATIONS)
                            {
                                pRetVal = new CLDataEnqueueOperationsTimelineItem(gpuStart, gpuEnd, pGPUIDataItem->OwnerAPIItem()->APICallIndex());

                                quint64 dataSize = 0;
                                rcData = rcData && pGPUIDataItem->OwnerAPIItem()->GetEnqueueOtherDataSize(dataSize);
                                ((CLDataEnqueueOperationsTimelineItem*)pRetVal)->setDataSize(dataSize);
                            }
                            else
                            {
                                pRetVal = new CLOtherEnqueueOperationsTimelineItem(gpuStart, gpuEnd, pGPUIDataItem->OwnerAPIItem()->APICallIndex());
                            }


                            if (pBranchInfo->m_pOtherEnqueueOperationBranch == nullptr)
                            {
                                pBranchInfo->m_pOtherEnqueueOperationBranch = new acTimelineBranch();
                                pBranchInfo->m_pOtherEnqueueOperationBranch->setText(GPU_STR_timeline_OtherEnqueueBranchName);
                                pBranchInfo->m_pQueueBranch->addSubBranch(pBranchInfo->m_pOtherEnqueueOperationBranch);
                            }

                            GT_IF_WITH_ASSERT(pBranchInfo->m_pOtherEnqueueOperationBranch != nullptr)
                            {
                                pBranchInfo->m_pOtherEnqueueOperationBranch->addTimelineItem(pRetVal);
                            }
                        }
                    }
                }

                if (pRetVal != nullptr)
                {
                    CLAPITimelineItem* pCLTimelineItem = dynamic_cast<CLAPITimelineItem*>(pRetVal);
                    GT_IF_WITH_ASSERT(pCLTimelineItem != nullptr)
                    {
                        pCLTimelineItem->setBackgroundColor(pAPITimelineItem->backgroundColor());
                        pCLTimelineItem->setForegroundColor(Qt::white);
                        pCLTimelineItem->setText(pGPUIDataItem->OwnerAPIItem()->GetColumnData(ProfileSessionDataItem::SESSION_ITEM_DEVICE_BLOCK_COLUMN).toString());
                        pCLTimelineItem->setQueueTime(gpuQueued);
                        pCLTimelineItem->setSubmitTime(gpuSubmit);
                        pCLTimelineItem->setDeviceType(deviceNameStr);
                        pCLTimelineItem->setCommandType(strCmdType);
                        pCLTimelineItem->setHostItem(pAPITimelineItem);
                    }
                }
            }
        }
    }

    return pRetVal;
}

acAPITimelineItem* gpTimeline::CreateHSAGPUItem(ProfileSessionDataItem* pGPUIDataItem)
{
    acAPITimelineItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pGPUIDataItem != nullptr)
    {
        // Only GPU Items should be here
        GT_IF_WITH_ASSERT(pGPUIDataItem->ItemType().m_itemMainType == ProfileSessionDataItem::HSA_GPU_PROFILE_ITEM)
        {
            QString globalWorkSize, groupWorkSize;
            QString strCmdType, strQueueHandle, strContextHandle, deviceNameStr;

            pGPUIDataItem->GetDispatchCommandWorkSizes(globalWorkSize, groupWorkSize);
            pGPUIDataItem->GetDispatchCommandDeviceNames(strQueueHandle, strContextHandle, deviceNameStr);

            unsigned int apiID = 0;
            bool rcData = pGPUIDataItem->GetAPIFunctionID(apiID);

            if (rcData && (apiID == HSA_API_Type_Non_API_Dispatch))
            {
                unsigned int seqID = 0;
                quint64 gpuStart = 0, gpuEnd = 0;
                QString kernelNameStr;
                rcData = pGPUIDataItem->GetHSACommandGPUTimes(gpuStart, gpuEnd);
                rcData = rcData && pGPUIDataItem->GetDisplaySeqID(seqID);
                rcData = rcData && pGPUIDataItem->GetDispatchCommandKernelName(kernelNameStr);
                GT_ASSERT(rcData);
                HSADispatchTimelineItem* pDispatchItem = new HSADispatchTimelineItem(gpuStart, gpuEnd, seqID);

                pDispatchItem->setText(kernelNameStr);
                pDispatchItem->setGlobalWorkSize(globalWorkSize);
                pDispatchItem->setLocalWorkSize(groupWorkSize);
                pDispatchItem->setDeviceType(deviceNameStr);
                pDispatchItem->setQueueHandle(strQueueHandle);
                pDispatchItem->setBackgroundColor(Qt::darkGreen);
                pDispatchItem->setForegroundColor(Qt::white);
                pDispatchItem->setHostItem(nullptr);

                pRetVal = pDispatchItem;

                // Setup a device row based on deviceNameStr OR m_strQueueHandle
                acTimelineBranch* pDeviceBranch = nullptr;

                // Create HSA branch if it does not yet exist
                if (m_pHSABranch == nullptr)
                {
                    m_pHSABranch = new acTimelineBranch();

                    m_pHSABranch->setText(GPU_STR_TraceViewHSA);
                }

                if (m_hsaQueueMap.contains(strQueueHandle))
                {
                    pDeviceBranch = m_hsaQueueMap[strQueueHandle];
                }
                else
                {
                    pDeviceBranch = new acTimelineBranch();

                    QString queueBranchText;

                    if (strQueueHandle == GPU_STR_timeline_UnknownQueueBranchName)
                    {
                        queueBranchText = deviceNameStr;
                    }
                    else
                    {
                        queueBranchText = QString(GPU_STR_timeline_QueueBranchNameWithParam).arg(strQueueHandle).arg(deviceNameStr);
                    }

                    pDeviceBranch->setText(queueBranchText);
                    m_hsaQueueMap[strQueueHandle] = pDeviceBranch;
                }

                // Sanity check:
                GT_IF_WITH_ASSERT(pDeviceBranch != nullptr)
                {
                    pDeviceBranch->addTimelineItem(pRetVal);
                }
            }
        }
    }

    return pRetVal;
}

/// string to display as the tool tip for a specific time
QString gpTimeline::GetTooltip(double iTime, QMouseEvent* pMouseEvent)
{
    GT_UNREFERENCED_PARAMETER(iTime);

    QString toolTipStr;

    if (pMouseEvent != nullptr)
    {
        acTimelineItem* pItem = getTimelineItem(pMouseEvent->x(), pMouseEvent->y());

        if (pItem != nullptr)
        {
            toolTipStr = pItem->tooltipText();
        }
    }

    return toolTipStr;
}

void gpTimeline::OnSetElementsNewWidth(int legendWidth, int timelineWidth)
{
    GT_IF_WITH_ASSERT(grid() != nullptr)
    {
        // the legend width is needed since this is the base width, but the chart has a margin before the time line
        // unlike this control that draws directly after the grid label space, so in order to match the two the chart margin is added
        // to match the two.
        // to force this control to recalc everything after the set a cal to recalc is needed
        grid()->setGridLabelSpace(legendWidth + GP_NAVIGATION_CHART_LEFT_MARGIN);
        recalcTitleWidth();
    }
    // this sets the width of the time line itself which is the legend + timeline but again there is a margin on the right side
    // so in order to match we need to remove it so the right side will match
    setFixedWidth(timelineWidth + legendWidth - GP_NAVIGATION_CHART_RIGHT_MARGIN);
}


void gpTimeline::UpdateCPUCaption()
{
    int numSubBranches = m_subBranches.size();

    if (numSubBranches > 0)
    {
        int visibleCPUThreadCount = 0;
        int CPUThreadCount = 0;

        // change caption of CPU branch: add num visible threads
        for (auto pBranch : m_subBranches)
        {
            if (pBranch->text().contains("CPU"))
            {
                CPUThreadCount = pBranch->subBranchCount();

                for (int i = 0; i < CPUThreadCount; i++)
                {
                    acTimelineBranch* pSubBranch = pBranch->getSubBranch(i);

                    if (pSubBranch->IsVisible())
                    {
                        visibleCPUThreadCount++;
                    }
                }
            }
        }

        acTimelineBranch* pCPUBranch = m_subBranches[0];
        QString str = QString(GPU_STR_TraceViewCPUThreads).arg(visibleCPUThreadCount).arg(CPUThreadCount);

        if (pCPUBranch != nullptr)
        {
            pCPUBranch->setText(str);
        }
    }
}

void gpTimeline::OnVisibilityFilterChanged(QMap<QString, bool>& threadNameVisibilityMap)
{
    int CPUThreadCount = threadNameVisibilityMap.size();
    int visibleCPUThreadCount = 0;

    for (auto e : threadNameVisibilityMap.keys())
    {
        if (threadNameVisibilityMap.value(e) == true)
        {
            visibleCPUThreadCount++;
        }
    }

    acTimelineBranch* pCPUBranch = m_subBranches[0];
    QString str = QString(GPU_STR_TraceViewCPUThreads).arg(visibleCPUThreadCount).arg(CPUThreadCount);

    if (pCPUBranch != nullptr)
    {
        pCPUBranch->setText(str);
    }
}

/// Overridden QWidget method called when this widget needs to be painted.
/// \param event the event parameters.
void gpTimeline::paintEvent(QPaintEvent* event)
{
    acTimeline::paintEvent(event);

    QPainter painter(this);

    // Set the dashed pen for the border
    painter.setPen(gpUIManager::Instance()->DashedLinePen());
    QColor bgColor(Qt::gray);
    bgColor.setAlpha(32);
    QBrush bgBrush(bgColor);
    painter.setBrush(bgBrush);
    QFont painterFont = painter.font();
    painterFont.setPixelSize(defaultBranchHeight() * 0.6);
    painter.setFont(painterFont);

    // Draw the present areas
    int startPixel = TimeToPixel(grid()->visibleStartTime());
    int presetGPUPixel = TimeToPixel(m_presentData[gpRibbonDataCalculator::ePresentGPU]);
    int endPixel = TimeToPixel(grid()->fullRange());

    int numPresents = m_presentData.size();
    // If we have a present call time
    if (numPresents > 1)
    {
        // find the latest CPU present
        double latestCPUPreset = 0;
        for (int nPresent = 1; nPresent < numPresents; nPresent++)
        {
            if (m_presentData[nPresent] > latestCPUPreset)
            {
                latestCPUPreset = m_presentData[nPresent];
            }
        }
        // Get the present call time as pixel
        int presetCPUPixel = TimeToPixel(latestCPUPreset);

        if (presetCPUPixel >= 0)
        {
            // find the branch of CPU
            acTimelineBranch* pCPUBranch = getBranchFromText("CPU", true);
            GT_IF_WITH_ASSERT(pCPUBranch != nullptr)
            {
                int cpuHeight = pCPUBranch->cumulativeHeight();
                QRect cpuRect(presetCPUPixel, pCPUBranch->top(), endPixel - presetCPUPixel, cpuHeight);
                cpuRect.adjust(+1, +1, -1, -1);

                if (cpuRect.width() > 0)
                {
                    painter.drawRect(cpuRect);

                    // Draw the text
                    QString fullCPUString(GPU_STR_TraceViewCPULaterFull);
                    QString shortCPUString(GPU_STR_TraceViewCPULaterShort);
                    DrawTextInRect(painter, cpuRect, fullCPUString, shortCPUString);
                }
            }
        }
    }

    acTimelineBranch* pGPUBranch = getBranchFromText("GPU", true);
    GT_IF_WITH_ASSERT(pGPUBranch != nullptr)
    {
        int gpuHeight = pGPUBranch->cumulativeHeight();
        QRect gpuRect(startPixel, pGPUBranch->top(), presetGPUPixel - startPixel, gpuHeight);
        gpuRect.adjust(+1, +1, -1, -1);

        if (gpuRect.width() > 0)
        {
            painter.drawRect(gpuRect);

            // Draw the text
            QString fullGPUString(GPU_STR_TraceViewGPUEarlierFull);
            QString shortGPUString(GPU_STR_TraceViewGPUEarlierShort);
            DrawTextInRect(painter, gpuRect, fullGPUString, shortGPUString);
        }
    }
}

void gpTimeline::DrawTextInRect(QPainter& painter, const QRect& stringRect, const QString& fullString, const QString& shortString)
{
    QPen currentPen = painter.pen();
    painter.setPen(QPen(QColor::fromRgb(99, 100, 102)));

    QRect largeStringRect = painter.boundingRect(stringRect, Qt::AlignCenter | Qt::TextWordWrap, fullString);

    if (stringRect.contains(largeStringRect))
    {
        painter.drawText(stringRect, Qt::AlignCenter | Qt::TextWordWrap, fullString);
    }
    else
    {
        QRect smallStringRect = painter.boundingRect(stringRect, Qt::AlignCenter | Qt::TextWordWrap, shortString);

        if (stringRect.contains(smallStringRect))
        {
            painter.drawText(stringRect, Qt::AlignCenter | Qt::TextWordWrap, shortString);
        }
    }

    painter.setPen(currentPen);
}

void gpTimeline::OnSummaryCmdListClicked(const QString& cmdList)
{

    QMap<QString, CommandListTimelineItem*>::iterator item = m_cmdListTimelineItemMap.find(cmdList);

    if (item != m_cmdListTimelineItemMap.end())
    {
        CommandListTimelineItem* pItem = item.value();
        if (pItem != nullptr)
        {
            ZoomToItem(pItem, true);
        }
    }
}