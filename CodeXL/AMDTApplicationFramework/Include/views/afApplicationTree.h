//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationTree.h
///
//==================================================================================

#ifndef __AFAPPLICATIONTREE_H
#define __AFAPPLICATIONTREE_H

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>


// Qt:
#include <QtWidgets>
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/views/afTreeCtrl.h>

class AF_API afApplicationTreeHandler : public QObject
{
    Q_OBJECT
public:
    virtual bool BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*>, QMenu& menu) = 0;
    virtual afApplicationTreeItemData* FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId) = 0;
    virtual bool BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent) = 0;
    virtual void SetItemsVisibility() = 0;
    virtual bool IsDragDropSupported(QWidget* receiver, QDropEvent* pEvent, QString& dragDropFile, bool& shouldAccpet) {(void)(receiver); (void)(pEvent); (void)(dragDropFile); (void)(shouldAccpet); return false; };
    virtual bool ExecuteDropEvent(QWidget* receiver, QDropEvent* pEvent, const QString& dragDropFile) { (void)(receiver);  (void)(pEvent); (void)(dragDropFile); return false; };
    virtual void PostContextMenuAction() {};

    /// Can this item be dropped into?
    virtual bool IsItemDroppable(QTreeWidgetItem* pItem) { (void)pItem; return false; }
    virtual bool CanItemBeDragged(QTreeWidgetItem* pItem) { (void)pItem; return false; }
protected:
    QList<gtString> m_unsupportedFileTypes;

};
// ----------------------------------------------------------------------------------
// Class Name:          afApplicationTree: public QWidget , public apIEventsObserver, public afBaseView
// General Description: A basic monitored objects navigation tree. The tree builds a monitored
//                      objects with the current objects existing in the API.
//                      The tree can be inherited and used for various navigation purposes.
// Author:              Sigal Algranaty
// Creation Date:       26/9/2010
// ----------------------------------------------------------------------------------
class AF_API afApplicationTree : public QWidget , public apIEventsObserver, public afBaseView
{
    Q_OBJECT

public:

    afApplicationTree(afProgressBarWrapper* pProgressBar, QWidget* pParent);
    virtual ~afApplicationTree();

    // Check if an item exist in the tree:
    bool doesItemExist(const afApplicationTreeItemData* pViewItem, afApplicationTreeItemData*& pOriginalItemData) const;
    virtual afApplicationTreeItemData* findMatchingTreeItem(const afApplicationTreeItemData& displayedItemId, bool getRootItemDataByDefault);

    virtual void updateTreeRootText();

    // Tree functionality:
    void collapseAll();
    void expandItem(QTreeWidgetItem* pItem);
    void clearSelection();
    QTreeWidgetItem* headerItem() const {return m_pHeaderItem;};
    QTreeWidgetItem* addTreeItem(const gtString& itemText, afApplicationTreeItemData* pItemData, QTreeWidgetItem* pParent);
    QTreeWidgetItem* insertTreeItem(const gtString& itemText, afApplicationTreeItemData* pItemData, QTreeWidgetItem* pParent, QTreeWidgetItem* pPreceding);
    int topLevelItemCount() const;
    QTreeWidgetItem* topLevelItem(int index) const;
    void scrollToItem(const QTreeWidgetItem* item, QAbstractItemView::ScrollHint hint = QTreeWidget::EnsureVisible);
    void editCurrentItem();
    afTreeCtrl* treeControl() const {return m_pTreeCtrl;};
    void removeTreeItem(QTreeWidgetItem* pTreeWidgetItem, bool removeParentIfEmpty);

    // Selection:
    void setIgnoreSelection(bool shouldIgnore) {m_ignoreSelections = shouldIgnore;}
    bool ignoreSelection() {return m_ignoreSelections;}
    bool selectItem(const afApplicationTreeItemData* pTreeItemData, bool shouldActivate);
    void addSelectedItem(afApplicationTreeItemData* pViewerItem, bool wasItemActivated);

    void debugPrintHistoryList();

    void clearNonExistingItemsFromSelection();

    /// Clear the requested tree item from the history list. Also remove all the item children from the list:
    /// \param pItemToBeRemove - the item which is about to be remove from tree
    void clearTreeItemFromSelectionHistoryList(QTreeWidgetItem* pItemToBeRemoved);

    void resetLastSelectedItem();
    const afApplicationTreeItemData* getCurrentlySelectedItemData() const ;
    void setNewSelection(QTreeWidgetItem* selectedTreeItem, bool& wasSelectionChanged);
    void setTreatSelectionAsActivation(bool shouldTreatSelectionAsActivation) {m_treatSelectAsActivate = shouldTreatSelectionAsActivation;}
    bool treatSelectionAsActivation() {return m_treatSelectAsActivate;}

    // Tree functionality:
    afApplicationTreeItemData* getTreeItemData(QTreeWidgetItem* pTreeItem) const;
    bool treeItemHasChildren(QTreeWidgetItem* pTreeItem) const;
    int getTreeChildrenCount(QTreeWidgetItem* pTreeItem, bool recursively) const;
    QTreeWidgetItem* getTreeRootItem() const;
    QTreeWidgetItem* getTreeItemParent(const QTreeWidgetItem* pTreeItem) const;
    QTreeWidgetItem* getTreeSelection() const;
    gtString getTreeItemText(const QTreeWidgetItem* pItem) const;
    QString TreeItemText(const QTreeWidgetItem* pItem) const;
    void setTreeItemText(QTreeWidgetItem* pTreeItem, const QString& itemText);
    void treeUnselectAll();
    void treeSelectItem(const QTreeWidgetItem* pTreeItem, bool select = true);
    void clearTreeItems(bool addNonAvailableMessage);
    void clearTreeItemsData(QTreeWidgetItem* pTreeItem);
    afApplicationTreeItemData* rootItemData() const {return m_pApplicationTreeItemData;};

    /// Get the item data for the child in childIndex:
    /// \param pTreeItem the tree item
    /// \param childIndex the index of the child
    /// \\return the item data for the child node or null
    afApplicationTreeItemData* GetChildItemData(QTreeWidgetItem* pTreeItem, int childIndex);

    // Sets the tree frame layout:
    void setFrameLayout(QWidget* pParent);

    // Debugged process events callback function:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // Event observer name:
    virtual const wchar_t* eventObserverName() const { return L"FrameworkObjectsNavigationTree"; };

    void updateToolbarButtons();

    // Edit menu commands:
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();
    void treeFocus();

    void registerApplicationTreeHandler(afApplicationTreeHandler* pTreeHandler) {m_applicationTreeHandlers.push_back(pTreeHandler);};


    // Drag & Drop:
    virtual bool ShouldAcceptDragDrop(QDropEvent* event, QString& dragDropFile);
    virtual bool ExecuteDropEvent(QWidget* receiver, QDropEvent* pEvent, QString& dragDropFile);

    // Execution changed:
    void onModeChanged();

    /// Display the properties for the item represented by pItemData:
    void DisplayItemProperties(const afApplicationTreeItemData* pItemData);

    /// Return an item with the requested file path
    /// \param the tree item file path
    /// \return the tree item data with an item that has the same file path, or null if the file was not found
    afApplicationTreeItemData* FindItemByFilePath(const osFilePath& filePath);

    // Handle drag and drop at the tree level so it can be executed in VS
    enum DragAction
    {
        DRAG_OPEN_FILES,
        DRAG_OPEN_PROJECT,
        DRAG_NEW_PROJECT,
        DRAG_ADD_ANALYZED_FILE_TO_TREE,
        DRAG_ADD_SESSION_TO_TREE,
        DRAG_SOURCE_FILE_ID,
        DRAG_NO_ACTION
    };

    /// Overrides QWidget drag & drop implementation:
    virtual void dropEvent(QDropEvent* pEvent);
    virtual void dragMoveEvent(QDragMoveEvent* pEvent);
    virtual void dragEnterEvent(QDragEnterEvent* pEvent);

signals:
    // emit this signal to be able to manipulate text before editing
    void EditorStarted(QLineEdit* pLineEdit);

protected slots:

    void onBackTool();
    void onForwardTool();
    void onPropertiesTimerTimeout();

    virtual void onContextMenuEvent(const QPoint& p);
    virtual void onObjectTreeActivation(QTreeWidgetItem* pActivated, int column);
    virtual void onItemSelectionChanged();

    void OnDragAttempt(QTreeWidgetItem* pItem, bool& canItemBeDragged);

    /// Handling tree elements drop
    void OnTreeElementDropEvent(QDropEvent* pEvent);

    /// Handling tree elements drag move
    void OnTreeElementDragMoveEvent(QDragMoveEvent* pEvent);

protected:


    bool doesItemExist(QTreeWidgetItem* rootTreeItemId, const afApplicationTreeItemData* pSearchedForItem, afApplicationTreeItemData*& pOriginalItemData) const;

    void updateBackCommand(bool& isEnabled);
    void updateForwardCommand(bool& isEnabled);
    void setTooltipsForNavigateButtons();
    void createTopPanel();
    void expandCurrentContext();

    class afApplicationTreeItemSelection
    {
    public:
        // True iff both selection and activation were performed:
        bool _wasItemActivated;

        // The selected item item data:
        afApplicationTreeItemData* _pItemData;
        afApplicationTreeItemSelection(afApplicationTreeItemData* pItemData, bool wasActivated)
        {
            _pItemData = pItemData;
            _wasItemActivated = wasActivated;
        }

        ~afApplicationTreeItemSelection()
        {
            if (_pItemData != nullptr)
            {
                delete _pItemData;
            }
        }
    };
    const afApplicationTreeItemSelection* getSelectedItem(int index) const;


    // Selections list data:
    gtPtrVector<afApplicationTreeItemSelection*> m_selectionHistoryVector;
    gtVector<afApplicationTreeHandler*> m_applicationTreeHandlers;

    int m_lastSelectedItemIndex;
    QTreeWidgetItem* m_pLastSelectedTreeItemId;

    // Contain true iff the user clicked a thumbnail item, and we should treat this
    // selection as activation:
    bool m_treatSelectAsActivate;


    // Contain true iff we should currently ignore selection events:
    bool m_ignoreSelections;
    static afApplicationTreeItemData* m_pStaticEmptySelectedItem;


protected:

    // Icons:
    QPixmap* m_pApplicationRootPixmap;

    // Tree control:
    afTreeCtrl* m_pTreeCtrl;

    // Tree top panel:
    QWidget* m_pTopPanel;
    QPushButton* m_pBackButton;
    QPushButton* m_pForwardButton;

    // Tree nodes for the application tree:
    QTreeWidgetItem* m_pHeaderItem;

    // Item data for root:
    afApplicationTreeItemData* m_pApplicationTreeItemData;

    // Saved project name
    gtString m_curProjectName;

    /// drag start position
    QPoint m_startPos;

    /// drag item
    QTreeWidgetItem* m_pDragItem;
};


#endif //__AFAPPLICATIONTREE_H

