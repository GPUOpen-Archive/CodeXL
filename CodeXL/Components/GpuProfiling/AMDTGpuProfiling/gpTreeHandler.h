//------------------------------ gpTreeHandler.h ------------------------------

#ifndef __GPTREEHANDLER_H
#define __GPTREEHANDLER_H

#include <QtGui>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

class afApplicationTree;
class gpSessionTreeNodeData;
class TraceSession;

class AMDT_GPU_PROF_API gpTreeHandler : public SessionTypeTreeHandlerAbstract
{

public:

    static gpTreeHandler& Instance();
    ~gpTreeHandler();

    /// Build the tree structure for the requested session:
    /// \param the item data for the session node
    /// \param pTreeItem the tree widget item for the session
    virtual bool BuildSessionTree(const SessionTreeNodeData& sessionTreeNodeData, QTreeWidgetItem* pTreeItem);

    /// Extend the session properties HTML with the GPU Profile session data:
    /// \param pSessionData tree item data representing the session
    /// \param sessionTreeItemType the session tree item type
    /// \param htmlContent[out] the HTML content for the session
    virtual bool ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent);

    /// Initialize icons:
    virtual void InitializeProfileIcons();

    /// Get the tree item icon
    QPixmap* TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr);

    /// Get the profile type string with a prefix (GPU or CPU). return true if modified by handler
    /// \param sessionTypeAsStr - the profile type string
    /// \param sessionTypeWithPrefix - new string with the correct prefix
    virtual bool GetProfileTypeWithPrefix(const QString& sessionTypeAsStr, gtString& sessionTypeWithPrefix);

    /// returns if the tree type belongs to the handler
    /// \param itemType - the tree item type to be checked
    virtual bool DoesTreeItemTypeBelongsToHandler(const afTreeItemType itemType) const;

    /// returns if the session node belongs to the handler
    /// \param pSessionData - the tree item type to be checked
    virtual bool DoesTreeNodeDataBelongsToHandler(const SessionTreeNodeData* pSessionData) const;

    /// Add a timeline to the session tree node, and activate it if requested
    /// \param pSessionDatathe session item data
    /// \param apiTraceFilePath the API timeline trace file
    /// \param frameIndex the index of the frame that should be captured
    /// \param shouldActivate should the new timeline item be activated?
    void AddTimelineToSession(const osFilePath& apiTraceFilePath, int frameIndex, bool shouldActivate);

    /// Add a captured frame to the current running session
    /// \param frameIndex the index of the captured frame
    /// \param shouldExpand should the new frame item be expanded?
    void AddCapturedFrameToTree(int frameIndex, bool shouldExpand);

    /// Add a timeline to the session tree node, and activate it if requested
    /// \param countersFilePath the counters data XML file path
    /// \param frameIndex the index of the frame that should be captured
    /// \param shouldActivate should the new timeline item be activated?
    void AddCountersDataFileToSession(const osFilePath& countersFilePath, int frameIndex, bool shouldActivate);

    /// Get the matching file path for the child of type childType related to frameIndex
    /// \param pSessionData tree item data representing the session
    /// \param the frame child type
    /// \frameIndex the frame index
    static osFilePath BuildFrameChildFilePath(gpSessionTreeNodeData* pSessionData, afTreeItemType childType, int frameIndex);

    /// Find the file path for the owner frame of the item described in frameChildFilePath
    /// \param frameChildFilePath a file path to an item which is a child of a frame
    /// \param frameFilePath[out] the owner frame file path
    /// \return true if the frame was found
    static bool GetOwningFrameFilePath(const osFilePath& frameChildFilePath, osFilePath& frameFilePath);

    /// Get the frame tree item
    /// \param pSessionData the owner session item data
    /// \param frameIndex frame index
    static QTreeWidgetItem* GetFrameTreeItem(gpSessionTreeNodeData* pSessionData, int frameIndex);

    /// Update the file path for a frame related to a session
    /// \param pSessionData the owner session item data
    /// \param frameIndex frame index
    /// \param ltrFilePath the new atr file name
    /// \return true if the frame was found
    static bool UpdateFrameFilePath(gpSessionTreeNodeData* pSessionData, int frameIndex, const osFilePath& ltrFilePath);

    /// Return the file path for a frame child
    /// \param sessionFilePath the owner session file path
    /// \param frameIndex the frame index
    /// \param childType the child item tree type
    /// \return the child item file path
    osFilePath GetFrameChildFilePath(const osFilePath& sessionFilePath, int frameIndex, afTreeItemType childType);

    /// Export a file
    /// \param exportFilePath export file path
    /// \return True if success in export
    virtual bool ExportFile(const osDirectory& sessionDir, const QString& exportFilePath, SessionTreeNodeData* pSessionData);

    virtual bool IsExportEnabled()const;

    /// Refresh the current project's sessions from the remote server
    virtual bool RefreshSessionsFromServer();

    /// Prepares the trace for session frames
    bool PrepareTraceForSessionFrames(const osDirectory& sessionDir, SessionTreeNodeData* pSessionData);

    /// Returns true if na export action is currently running
    bool IsExportInProgress()const;

private:

    // Do not allow the use of my default constructor:
    gpTreeHandler();

    /// Does the session folder contain html summary pages:
    /// \param pGPUTreeItemData the session item data
    bool DoesSummaryPagesExist(const SessionTreeNodeData* pGPUTreeItemData);

    /// Does the session folder contain "File Name" html summary page:
    /// \param pGPUTreeItemData the session item data
    bool DoesSingleSummaryPageExist(const SessionTreeNodeData* pGPUTreeItemData, const gtString& fileName);

    /// Builds a session tree for an application trace session
    /// \param pTreeItem the tree widget item related to the session
    /// \param the TraceSession data representing the session
    void BuildApplicationTraceSessionTree(QTreeWidgetItem* pTreeItem, TraceSession* pGPUTreeItemData);

    /// Add a summary item node to a trace session tree item
    /// \param pGPUTreeItemData the item data for the trace session
    /// \param pParent the summary item parent
    /// \param summaryFileName the summary item file name (pattern)
    /// \param summaryItemType the summary tree item type
    void AddSummaryFileTreeItem(TraceSession* pGPUTreeItemData, QTreeWidgetItem* pParent, const gtString& summaryFileName, afTreeItemType summaryItemType);

    /// Builds a session tree for a DX profile session
    /// \param pTreeItem the tree widget item related to the session
    /// \param pSessionData the item data representing the session
    void BuildFrameAnalysisSessionTree(QTreeWidgetItem* pTreeItem, gpSessionTreeNodeData* pSessionData);

    /// Add a child to the tree item
    /// \param pParent the node for which the new child should be added
    /// \param nodeName the node name
    /// \param itemType the new item type
    /// \param shouldCreateFile by default false. Should be used for tree items with dummy file (such as overview)
    static QTreeWidgetItem* AddTreeItem(gpSessionTreeNodeData* pSessionData, QTreeWidgetItem* pParent, const gtString& nodeName, afTreeItemType itemType, int frameIndex, bool shouldCreateFile = false);

    /// Adds the existing performance counters files under the performance counters node, for the frame in the input tree widget item
    /// \param pSessionData tree item data representing the session
    /// \param pFrameTreeItem the tree item related to the frame
    /// \param frameIndex the frame index
    static void AddExistingFileToPerformanceCountersNode(gpSessionTreeNodeData* pSessionData, QTreeWidgetItem* pFrameTreeItem, int frameIndex);

    /// Add performance counters data file to the performance counters tree node
    /// \param pSessionData tree item data representing the session
    /// \param performanceFilePath the performance counters data file path
    /// \param pPerformanceCountersItem the tree widget item representing the performance counters node
    /// \param frameIndex the frame index
    static void AddPerformanceCountersFileToTree(gpSessionTreeNodeData* pSessionData, const QFileInfo& performanceFilePath, QTreeWidgetItem* pPerformanceCountersItem, int frameIndex);

    /// Get a frame child node item data
    /// \param pFrameItem the tree item representing the frame
    /// \param childItemType the tree item type of the requested child
    /// \return the item data for the child with the requested type
    static afApplicationTreeItemData* GetFrameChildItemData(QTreeWidgetItem* pFrameItem, afTreeItemType childItemType);

    // Static member:
    static gpTreeHandler* m_psMySingleInstance;
};
#endif // __GPTREEHANDLER_H
