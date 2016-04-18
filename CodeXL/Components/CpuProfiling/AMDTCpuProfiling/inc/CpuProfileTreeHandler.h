//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileTreeHandler.h
///
//==================================================================================

#ifndef __CPUPROFILETREEHANDLER_H
#define __CPUPROFILETREEHANDLER_H

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTypeTreeHandlerAbstract.h>

enum CpuProfilerTreeHandleSessions
{
    CPU_TREE_TIME_BASED_SESSION = 0,
    CPU_TREE_ASSESS_PERFORMANCE_SESSION,
    CPU_TREE_INSTRUCTION_BASED_SAMPLING_SESSION,
    CPU_TREE_INVESTIGATE_BRANCHING_SESSION,
    CPU_TREE_INVESTIGATE_DATA_ACCESS_SESSION,
    CPU_TREE_INSTRUCTION_ACCESS_SESSION,
    CPU_TREE_L2_CACHE_ACCESS_SESSION,
    CPU_TREE_CUSTOM_PROFILE_SESSION,
    CPU_TREE_CACHE_LINE_SESSION,
    CPU_TREE_INSTRUCTION_PROFILING_SESSION,

};

/// A base class for a profiling tree session handler.
// This class is handling a profile tree management for anything that is under the session type structure
class CpuProfileTreeHandler : public SessionTypeTreeHandlerAbstract
{

public:

    static CpuProfileTreeHandler& instance();
    ~CpuProfileTreeHandler();

    /// Build the tree structure for the requested session:
    virtual bool BuildSessionTree(const SessionTreeNodeData& sessionTreeNodeData, QTreeWidgetItem* pTreeItem);

    /// Extend the session properties HTML with the CPU profile session specific data:
    /// \param pSessionData tree item data representing the session
    /// \param sessionTreeItemType the session tree item type
    /// \param htmlContent[out] the HTML content for the session
    virtual bool ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent);

    /// Initialize icons:
    virtual void InitializeProfileIcons();

    /// Get the tree item icon
    QPixmap* TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr);

    afApplicationTreeItemData* findChildItemData(const afApplicationTreeItemData* pParentItemData, afTreeItemType childType);

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

    /// Adds tree node for a session source code
    /// \param pSourceCodeItemSessionData - the item data for the source code node
    /// \return the created node item data
    afApplicationTreeItemData* AddSourceCodeToSessionNode(SessionTreeNodeData* pSourceCodeItemSessionData);

    /// Go over the children of the session (recursively), and look for a child with the requested item type:
    /// \param pSessionItemData - the session item data
    /// \param childItemType - the item type of the child
    afApplicationTreeItemData* FindSessionSourceCodeItemData(const afApplicationTreeItemData* pSessionItemData, const QString& moduleFilePath);

protected:

    /// Create the source code node for a session
    /// \param pSessionItemData - the requested session item data
    /// \return the create node item data
    afApplicationTreeItemData* createSourceCodeNodeForSession(afApplicationTreeItemData* pSessionItemData);

private:

    // Do not allow the use of my default constructor:
    CpuProfileTreeHandler();

    // Static member:
    static CpuProfileTreeHandler* m_psMySingleInstance;

    // Save the application tree pointer:
    afApplicationTree* m_pApplicationTree;
};


#endif //__CPUPROFILETREEHANDLER_H

