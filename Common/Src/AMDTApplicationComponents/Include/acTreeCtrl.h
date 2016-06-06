//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTreeCtrl.h
///
//==================================================================================

//------------------------------ acTreeCtrl.h ------------------------------

#ifndef __ACTREECTRL_H
#define __ACTREECTRL_H

// Qt:
#include <QtWidgets>
#include <QTreeWidget>

// Infra:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class acTreeItemDelegate;
class acFindWidget;
class acFindParameters;

class AC_API acTreeCtrl : public QTreeWidget
{
    Q_OBJECT

    friend class acTreeItemDelegate;

public:
    acTreeCtrl(QWidget* pParent, int numberOfColumns = 1, bool addPasteAction = true, bool addExpandCollapeAllActions = false);
    virtual ~acTreeCtrl();

    void clearView();
    QTreeWidgetItem* addItem(const QStringList& rowTexts, void* pItemData, QTreeWidgetItem* pParent = NULL, QPixmap* pItemIcon = NULL);
    QTreeWidgetItem* insertItem(const QStringList& rowTexts, void* pItemData, int index, QTreeWidgetItem* pParent = NULL, QPixmap* pItemIcon = NULL);

    /// Return true if pParent is an ancestor of pChild
    /// \param pChild - the child
    /// \param pParent - the parent
    bool isAncestor(const QTreeWidgetItem* pChild, const QTreeWidgetItem* pParent);

    void CollapseAllSubTree(QTreeWidgetItem* pItem);
    void ExpandAllSubTree(QTreeWidgetItem* pItem);

    void SetAutoExpandOnSingleChild();
    void UnSetAutoExpandOnSingleChild();
    bool IsPathIndicatorEnabled() const { return m_pathIndicatorEnabled; }
    void SetPathIndicator(bool enable);

    bool IsPathIndicatorLevelShown() const { return m_pathIndicatorLevelShown; }
    void SetPathIndicatorLevel(bool shown);

    const QColor& GetPathIndicatorSelectedColor() const { return m_colorPathSelected; }
    const QColor& GetPathIndicatorHoverColor() const { return m_colorPathHover; }
    void SetPathIndicatorSelectedColor(const QColor& color) { m_colorPathSelected = color; }
    void SetPathIndicatorHoverColor(const QColor& color) { m_colorPathHover = color; }

    const QTreeWidgetItem* FindLowestCommonAncestor(const QTreeWidgetItem* pItem1, const QTreeWidgetItem* pItem2,
                                                    const QTreeWidgetItem** ppDeepsetItem = NULL) const;

    static int CalcItemLevel(const QTreeWidgetItem* pItem);

    QTreeWidgetItem* getItemFromIndex(const QModelIndex& index) const { return itemFromIndex(index); }
    virtual bool isItemSelected(const QModelIndex& index, bool& isFocused);

    QTreeWidgetItem* FindChild(const QTreeWidgetItem* pParent, QString& searchNodeText);
    QTreeWidgetItem* FindDescendant(const QTreeWidgetItem* pParent, const QString& searchNodeText) const;
    void FindDescendants(const QTreeWidgetItem* pParent, const QString& searchNodeText, std::list<QTreeWidgetItem*>& result) const;
public slots:

    virtual void onAboutToShowContextMenu();

    // Edit commands implementations:
    void onEditCopy();
    void onEditSelectAll();
    void onEditFind();
    void onEditFindNext();

    void onCollapseAll();
    void onExpandAll();
    void onItemExpanded(QTreeWidgetItem* item);

    void onUpdateEditCopy(bool& isEnabled);
    void onUpdateEditPaste(bool& isEnabled, bool& isVisible);
    void onUpdateEditSelectAll(bool& isEnabled);

    void onPathChanged(QTreeWidgetItem* pCurrent, QTreeWidgetItem* pPrevious);

    QLineEdit* lineEditor() { return m_pEditor; };

signals:

    void onItemCloseEditor(const QString& itemNewName);

protected slots:
    virtual void onContextMenuEvent(const QPoint& point);
    virtual void closeEditor(QWidget* pEditor, QAbstractItemDelegate::EndEditHint hint);
    virtual void closeEditor();
    void saveLineEdit();

protected:
    void initContextMenu(bool addPasteAction, bool addExpandCollapeAllActions);
    virtual void paintEvent(QPaintEvent* event);
    int drawPath(QPainter* painter, QTreeWidgetItem* pItem, const QColor& color, int lineWidth) const;
    virtual bool viewportEvent(QEvent* event);
    /// sets the line editor created in the delegate:
    void setEditor(QLineEdit* pEditor);
    /// Find functions
    QModelIndex FindNextText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity);
    QModelIndex FindPrevText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity);

protected:
    // Context menu:
    QMenu* _pContextMenu;
    QAction* _pCopyAction;
    QAction* _pPasteAction;
    QAction* _pSelectAllAction;
    QAction* _pExpandAllAction;
    QAction* _pCollapseAllAction;
    QPoint _contextMenuPosition;
    bool _isPasteActionEnabled;
    bool m_pathIndicatorEnabled;
    bool m_pathIndicatorLevelShown;
    QTreeWidgetItem* m_pHoverItem;
    QColor m_colorPathSelected;
    QColor m_colorPathHover;
    QLineEdit* m_pEditor;
    /// Find parameters:
    QModelIndex m_lastFindIndex;
private:
    bool m_autoExpandOnSingleChild = false;


};

class acTreeItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    acTreeItemDelegate(QObject* parent = 0) : QItemDelegate(parent) { }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif //__ACTREECTRL_H

