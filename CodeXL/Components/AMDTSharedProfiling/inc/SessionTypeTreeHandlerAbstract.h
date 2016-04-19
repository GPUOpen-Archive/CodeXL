//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionTypeTreeHandlerAbstract.h
///
//==================================================================================

#ifndef __SESSIONTYPETREEHANDLERABSTRACT_H
#define __SESSIONTYPETREEHANDLERABSTRACT_H

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include "SessionTreeNodeData.h"
#include "LibExport.h"


/// A base class for a profiling tree session handler.
// This class is handling a profile tree management for anything that is under the session type structure
class AMDTSHAREDPROFILING_API SessionTypeTreeHandlerAbstract
{

public:

    SessionTypeTreeHandlerAbstract() {};
    ~SessionTypeTreeHandlerAbstract() {};

    /// Build the tree structure for the requested session:
    virtual bool BuildSessionTree(const SessionTreeNodeData& sessionData, QTreeWidgetItem* pTreeItem) = 0;

    /// Extend the session properties HTML with the specific plug-in profile session data:
    /// \param pSessionData tree item data representing the session
    /// \param sessionTreeItemType the session tree item type
    /// \param htmlContent[out] the HTML content for the session
    virtual bool ExtendSessionHTMLPropeties(afTreeItemType sessionTreeItemType, const SessionTreeNodeData* pSessionData, afHTMLContent& htmlContent) = 0;

    /// Initialize icons:
    virtual void InitializeProfileIcons() = 0;

    /// Get the tree item icon
    virtual QPixmap* TreeItemTypeToPixmap(afTreeItemType itemType, const QString& sessionTypeAsStr) = 0;

    /// Get the profile type string with a prefix (GPU or CPU). return true if modified by handler
    /// \param sessionTypeAsStr - the profile type string
    /// \param sessionTypeWithPrefix - new string with the correct prefix
    virtual bool GetProfileTypeWithPrefix(const QString& sessionTypeAsStr, gtString& sessionTypeWithPrefix) = 0;

    /// returns if the tree type belongs to the handler
    /// \param itemType - the tree item type to be checked
    virtual bool DoesTreeItemTypeBelongsToHandler(const afTreeItemType itemType) const = 0;

    /// returns if the session node belongs to the handler
    /// \param pSessionData - the tree item type to be checked
    virtual bool DoesTreeNodeDataBelongsToHandler(const SessionTreeNodeData* pSessionData) const = 0;

    /// Create an empty session (by default this is not supported):
    /// \return afApplicationTreeItemData* representing the created empty session item data (or NULL)
    /// \param pParentData the parent item data
    virtual SessionTreeNodeData* CreateEmptySessionData(afApplicationTreeItemData* pParentData) { GT_UNREFERENCED_PARAMETER(pParentData); return NULL; };

    virtual bool ExportFile(const osDirectory& sessionDir, const QString& exportFilePath, SessionTreeNodeData* pSessionData) { GT_UNREFERENCED_PARAMETER(sessionDir); GT_UNREFERENCED_PARAMETER(exportFilePath); GT_UNREFERENCED_PARAMETER(pSessionData); return false; }

    virtual bool IsExportEnabled()const { return false; }

    /// Refresh the current project's sessions from the remote server
    virtual bool RefreshSessionsFromServer() { return false; };
protected:

    /// This map is loaded once the hander is created, and holds all the icons relevant to the tree structure for this handler
    gtMap<acIconId, QPixmap*> m_iconsMap;

    /// Adds an icon id to the icons map
    /// \param iconId the icon ID
    void AddTreeIconToMap(acIconId iconId)
    {
        QPixmap* pPixmap = new QPixmap;
        acSetIconInPixmap(*pPixmap, iconId);
        m_iconsMap[iconId] = pPixmap;
    }
};

#endif //__SESSIONTYPETREEHANDLERABSTRACT_H

