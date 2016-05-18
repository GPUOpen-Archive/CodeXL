//------------------------------ gpTimeline.h ------------------------------

#ifndef _GPTIMELINE_H_
#define _GPTIMELINE_H_

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/acRibbonManager.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>

class gpTraceDataContainer;
class gpTraceView;
// ----------------------------------------------------------------------------------
// Class Name:          gpTimeline
// General Description: An object handling the fill and draw of a timeline table for
//                      the DX API trace
// ----------------------------------------------------------------------------------
class AMDT_GPU_PROF_API gpTimeline : public acTimeline, public acIRibbonTime
{
    Q_OBJECT

public:

    gpTimeline(QWidget* pParent, gpTraceView* pSessionView);
    ~gpTimeline();

    /// Build the timeline from the data stored in m_pDataInterface
    void BuildTimeline(gpTraceDataContainer* pDataContainer);

    /// Add the API functions for the requested thread
    void AddThreadAPIFunctions(osThreadId threadId);

    /// Add the API functions for the requested queue
    void AddQueueGPUApis(const QString& queueName);

    /// Add the command lists for the requested queue to the timeline
    void AddCommandListsToTimeline();

    /// Add the performance markers for the thread to the timeline
    /// \param threadId thread ID
    void AddPerformanceMarkersToTimeline(osThreadId threadId);

    /// Add the GPU timeline items to the timeline
    void AddGPUItemsToTimeline();

    /// Implement the acIRibbonTime interface:

    /// string to display as the tool tip for a specific time
    virtual QString GetTooltip(double iTime, QMouseEvent* pMouseEvent);

    /// Set the present data
    void SetPresentData(QVector<double>& presentData) { m_presentData = presentData; }

    /// Overridden QWidget method called when this widget needs to be painted.
    /// \param event the event parameters.
    void paintEvent(QPaintEvent* event);

protected slots:

    /// handle the setting of elements width
    /// \param legendWidth is the width of the legend part of the view
    /// \param timelineWidth is the width of the time line part of the view
    void OnSetElementsNewWidth(int legendWidth, int timelineWidth);

    void OnVisibilityFilterChanged(QMap<QString, bool>& threadNameVisibilityMap);

private:

    /// struct that holds the queue, data transfer and kernel execution branches
    struct OCLQueueBranchInfo
    {
        acTimelineBranch* m_pQueueBranch;          ///< the queue branch
        acTimelineBranch* m_pKernelBranch;         ///< the kernel execution branch
        acTimelineBranch* m_pMemoryBranch;         ///< the data transfer branch
        acTimelineBranch* m_pOtherEnqueueOperationBranch;  ///< the fill operations branch
        OCLQueueBranchInfo() : m_pQueueBranch(nullptr), m_pKernelBranch(nullptr), m_pMemoryBranch(nullptr), m_pOtherEnqueueOperationBranch(nullptr)
        {

        }
    };

    /// Initializes or creates branch info for the requested queue:
    /// \param contextId the id of the context for which the branch is created
    /// \param queueId the id for the queue for which the branch is created
    /// \strContextHandle the string for the context
    /// \deviceNameStr the device name
    /// \return the branch info for the queue
    OCLQueueBranchInfo* GetBranchInfo(unsigned int contextId, unsigned int queueId, const QString& strContextHandle, const QString& deviceNameStr, const QString& strQueueHandle);

    /// Gets or creates an API branch for the host in threadId
    /// \param threadId the host thread id
    /// \param queueName the queue name for GPU items
    /// \param branchText the branch text
    /// \param the item type for which the timeline item should be added
    /// \return the branch
    acTimelineBranch* GetBranchForAPI(osThreadId threadId, const QString& queueName, const QString& branchText, ProfileSessionDataItem::ProfileItemAPIType itemType);

    /// Gets or creates an branch for the queue in the command lists branch
    /// \param queueName the queue name for GPU items
    /// \return the branch
    acTimelineBranch* GetCommandListBranch(const QString& queueName);

    /// Gets or creates a performance markers branch for the host in threadId
    /// \param threadId the host thread id
    /// \param ProfileSessionDataItem the item representing the performance marker
    acTimelineBranch* GetHostBranchForPerfMarker(osThreadId threadId, ProfileSessionDataItem* pItem);

    /// Create the timeline items (Host + GPU items) for the requested session item
    /// \param pItem the item for which a timeline item should be created
    /// \return the created API timeline item
    acAPITimelineItem* CreateTimelineItem(ProfileSessionDataItem* pItem);

    /// Create a timeline item for the performance marker item
    /// \pItem the data container item representing the performance marker
    /// \return the timeline item representing the performance item on the timeline
    acTimelineItem* CreatePerfMarkerTimelineItem(ProfileSessionDataItem* pItem);

    /// Create the GPU timeline item for the requested GPU item
    /// \param pGPUIDataItem the session item for which a timeline item should be created
    /// \param pGPUIDataItem the session item representing the API item which owns this GPU item
    /// \return the created GPU timeline item (or null if the GPU item should not be created)
    acAPITimelineItem* CreateCLGPUItem(ProfileSessionDataItem* pGPUIDataItem, acAPITimelineItem* pAPITimelineItem);

    /// Create the GPU timeline item for the requested GPU item
    /// \param Session the item for which a timeline item should be created
    /// \return the created GPU timeline item (or null if the GPU item should not be created)
    acAPITimelineItem* CreateHSAGPUItem(ProfileSessionDataItem* pGPUIDataItem);

    void UpdateCPUCaption();

    /// Draw a string in the bounding rect using a defined painter. First try the full string and if there is not
    /// enough space try the short string. if still no space do not draw anything.
    /// \param painter to use
    /// \param bounding rect
    /// \param full string to try and display
    /// \param short string to try if full string can not be displayed
    void DrawTextInRect(QPainter& painter, const QRect& stringRect, const QString& fullString, const QString& shortString);

private:

    /// The session data container object
    gpTraceDataContainer* m_pSessionDataContainer;

    /// A pointer to the parent session view
    gpTraceView* m_pSessionView;

    /// The main OpenCL branch in the timeline
    acTimelineBranch* m_pOpenCLBranch;

    /// The main HSA branch in the timeline
    acTimelineBranch* m_pHSABranch;

    /// Map from OCL context id to the branch for that context
    QMap<unsigned int, acTimelineBranch*>    m_oclCtxMap;

    /// Map from OCL queue id to the QueueBranchInfo for that queue
    QMap<unsigned int, OCLQueueBranchInfo*>  m_oclQueueMap;

    /// Map from HSA queue handle string to the branch for that queue
    QMap<QString, acTimelineBranch*> m_hsaQueueMap;

    /// Map from thread id to the host timeline branch for that thread
    QMap<osThreadId, acTimelineBranch*> m_cpuBranchesMap;

    /// Map from queue name to the timeline branch for that queue
    struct QueueBranches
    {
        acTimelineBranch* m_pQueueRootBranch;
        acTimelineBranch* m_pQueueAPIBranch;
        acTimelineBranch* m_pQueueCommandListsBranch;
    };
    QMap<QString, QueueBranches> m_gpuBranchesMap;

    /// CPU timeline branch
    acTimelineBranch* m_pCPUTimelineBranch;

    /// GPU timeline branch
    acTimelineBranch* m_pGPUTimelineBranch;

    /// the present data
    QVector<double> m_presentData;
};


#endif // _GPTIMELINE_H_