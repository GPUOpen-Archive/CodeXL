//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppTreeHandler.h
///
//==================================================================================

//------------------------------ ppTreeHandler.h ------------------------------

#ifndef __PPTREEHANDLER_H
#define __PPTREEHANDLER_H

// Forward declaration:
class afApplicationTree;

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTypeTreeHandlerAbstract.h>

enum PowerProfilerTreeHandleSessions
{
    PP_TREE_ONLINE_SESSION = 0,
    PP_TREE_OFFLINE_SESSION,
};

class PP_API ppTreeHandler : public SessionTypeTreeHandlerAbstract
{
public:

    static ppTreeHandler& instance();
    ~ppTreeHandler();

    /// implement SessionTypeTreeHanderAbstract interface:

    /// Build the tree structure for the requested session:
    virtual bool BuildSessionTree(const SessionTreeNodeData& sessionData, QTreeWidgetItem* pTreeItem);

    /// Extend the session properties HTML with the power profile specific data:
    /// \param pSessionData tree item data representing the session
    /// \param sessionTreeItemType the session tree item type
    /// \param htmlContent[out] the HTML content for the session
    virtual bool ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent);

    /// Initialize icons:
    virtual void InitializeProfileIcons();

    /// Get the tree item icon
    virtual QPixmap* TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr);

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

    /// Get the application tree
    afApplicationTree* GetApplicationTree();

    /// Create an empty session (by default this is not supported):
    /// \param pParentData the parent item data
    virtual SessionTreeNodeData* CreateEmptySessionData(afApplicationTreeItemData* pParentData);

protected:
    /// Do not allow the use of my default constructor:
    ppTreeHandler();

    // Static member:
    static ppTreeHandler* m_psMySingleInstance;

    // Save the application tree pointer:
    afApplicationTree* m_pApplicationTree;

};

#endif // __PPTREEHANDLER_H
