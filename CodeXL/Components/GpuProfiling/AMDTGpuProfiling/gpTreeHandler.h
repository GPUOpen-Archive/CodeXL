//=============================================================
// Copyright (c) 2015-2018 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file
/// \brief GPU Profiler Tree Handler
//=============================================================

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

    // Static member:
    static gpTreeHandler* m_psMySingleInstance;
};
#endif // __GPTREEHANDLER_H
