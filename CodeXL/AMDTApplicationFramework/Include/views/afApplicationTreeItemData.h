//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationTreeItemData.h
///
//==================================================================================

#ifndef __AFAPPLICATIONTREEITEMDATA_H
#define __AFAPPLICATIONTREEITEMDATA_H

// Qt:
#include <QtWidgets>
#include <QObject>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class afApplicationTreeItemData;

/// -----------------------------------------------------------------------------------------------
/// \class Name: AF_API afTreeDataExtension
/// \brief Description:  Is used for extending the tree item data
/// -----------------------------------------------------------------------------------------------
class AF_API afTreeDataExtension : public QObject
{
    Q_OBJECT

public:
    afTreeDataExtension() { m_pParentData = nullptr;}
    virtual bool isSameObject(afTreeDataExtension* pOtherItemData) const = 0;
    virtual void copyID(afTreeDataExtension*& pOtherItemData) const = 0;

    afApplicationTreeItemData* m_pParentData;
};

/// -----------------------------------------------------------------------------------------------
/// \class Name: AF_API afApplicationTreeItemData
/// \brief Description:  This class is a base class for an item data to the application tree
///                      When the tree is expanded, this data should be used as base for each item
//                       data set to tree widget items
/// -----------------------------------------------------------------------------------------------
class AF_API afApplicationTreeItemData
{
public:
    afApplicationTreeItemData(bool handleExtendedDataMemory = false);
    virtual ~afApplicationTreeItemData();

    virtual bool isSameObject(const afApplicationTreeItemData* pOtherItemData) const;

    /// Item load & display status:
    static bool itemTypeAsString(afTreeItemType itemType, gtString& itemTypeStr);
    bool getItemStatusAsString(afTreeItemType itemType, gtString& statusTitle, gtString& statusStr);
    void setItemLoadStatus(afItemLoadStatusType itemLoadStatus, afItemLoadFailureDescription loadDescription = AF_ITEM_ERROR_UNKNOWN);
    bool isItemDisplayed();

    /// Item type queries:
    static afTreeItemType itemTypeToParent(afTreeItemType itemType) ;
    static bool isItemTypeRoot(afTreeItemType itemType) ;
    static bool isItemImageOrBuffer(afTreeItemType itemType);
    static bool isItemImage(afTreeItemType itemType);
    static bool isItemSource(afTreeItemType itemType);
    static bool isItemThumbnail(afTreeItemType itemType);
    static bool isItemBuffer(afTreeItemType itemType);

    /// Extended data:
    void setExtendedData(afTreeDataExtension* pExtensionData);
    afTreeDataExtension* extendedItemData() const {return m_pExtendedItemData;};

    virtual void copyID(afApplicationTreeItemData& other) const;

    /// Object type:
    afTreeItemType m_itemType;

    /// Contain true if the item should be removed:
    bool m_isItemRemovable;

    /// Tree item ID:
    QTreeWidgetItem* m_pTreeWidgetItem;

    /// The tree item ID for the original item (used for shortcut items):
    QTreeWidgetItem* m_pOriginalItemTreeItem;

    /// The item index within other children on the tree:
    int m_itemTreeIndex;

    /// Is this the tree root data?
    bool m_isRoot;

    /// Objects amount (under this item):
    int m_objectCount;

    /// Object size (In KB):
    gtSize_t m_objectMemorySize;

    /// Item load status:
    afItemLoadStatus _itemLoadStatus;

    /// True iff the extended memory should be handled:
    bool m_handleExtendedDataMemory;

    /// Contain the file that is representing the element. If not empty, will be opened in object activation. Can be empty:
    osFilePath m_filePath;

    /// Contain the line number the view that is supposed to be opened for m_filePath:
    int m_filePathLineNumber;

protected:
    /// Contain an extended data:
    afTreeDataExtension* m_pExtendedItemData;
};

/// -----------------------------------------------------------------------------------------------
/// \class Name: AF_API afApplicationTreeMessageItemData
/// \brief Description:  Tree item data extension for AF_TREE_ITEM_MESSAGE.
/// -----------------------------------------------------------------------------------------------
class AF_API afApplicationTreeMessageItemData : public afTreeDataExtension
{
    Q_OBJECT

public:
    virtual bool isSameObject(afTreeDataExtension* pOtherItemData) const;
    virtual void copyID(afTreeDataExtension*& pOtherItemData) const;

public:
    gtString m_messageTitle;
    gtString m_messageText;
};

AF_API afApplicationTreeItemData* afCreateMessageTreeItemData(const gtString& messageTitle, const gtString& messageText);


#endif //__AFAPPLICATIONTREEITEMDATA_H

