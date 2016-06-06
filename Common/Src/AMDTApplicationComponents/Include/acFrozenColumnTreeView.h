//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFrozenColumnTreeView.h
///
//==================================================================================

#ifndef __ACFREEZETREEWIDGET_H
#define __ACFREEZETREEWIDGET_H


// Qt:
#include <QHeaderView>
#include <QTreeView>
#include <QItemDelegate>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class acFrozenColumnTreeView;
class acFindWidget;
class acFindParameters;

/// -----------------------------------------------------------------------------------------------
/// \class Name: AC_API acNonScrolledTree : public QTreeView
/// \brief Description: The frozen tree is constructed from 2 trees. acNonScrolledTree will show
///                     the cells 0 - m_frozenColumnIndex, and acFrozenColumnTreeView will show the
///                     rest. The problem with this solution, is that for some reason, sometimes Qt
///                     scrolls to item with column > m_frozenColumnIndex, and that causes the tree to
///                     look corrupted. For this, we override QTreeView and use it to block the scroll
///                     when one of the hidden columns is requested.
/// -----------------------------------------------------------------------------------------------
class AC_API acNonScrolledTree : public QTreeView
{
public:
    acNonScrolledTree(QWidget* pParent, int frozenColumnIndex) : QTreeView(pParent), m_frozenColumnIndex(frozenColumnIndex) {};
    ~acNonScrolledTree() {};

    // Override QTreeView:
    virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);

private:

    /// Contain the index of the cell from which the tree should be hidden:
    int m_frozenColumnIndex;

};
/// -----------------------------------------------------------------------------------------------
/// \class Name: acFrozenColumnTreeView : public QTreeView
/// \brief Description:
/// -----------------------------------------------------------------------------------------------
class AC_API acFrozenColumnTreeView : public QTreeView
{
    Q_OBJECT
    friend class acFrozenTreeHeader;
public:

    acFrozenColumnTreeView(QWidget* pParent, QAbstractItemModel* pModel, int frozenColumn);
    ~acFrozenColumnTreeView();

    QMenu* ContextMenu() {return m_pContextMenu;}
    void ExpandAll();
    void CollapseAll();
    void SetItemExpanded(const QModelIndex& index, bool expanded);
    void ScrollToItem(const QModelIndex& index, ScrollHint hint = EnsureVisible);
    void ScrollToBottom();

    /// Shows / hide the requested column
    /// \param columnIndex the column index
    /// \param shouldShow true iff the column should be shown
    void ShowColumn(int columnIndex, bool shouldShow);


    // Delegate:
    void SetItemDelegate(QAbstractItemDelegate* pDelegate);
    void SetItemDelegateForColumn(int column, QAbstractItemDelegate* pDelegate);

    virtual bool isItemSelected(const QModelIndex& index, bool& isFocused);

signals:
    void ItemClicked(const QModelIndex& index);
    void ItemDoubleClicked(const QModelIndex& index);
    void VerticalScrollPositionChanged(int value);
    void ItemExpanded(const QModelIndex& index);
    void ItemCollapsed(const QModelIndex& index);

public slots:

    void onEditCopy();
    void onEditSelectAll();
    void onEditFind();
    void onEditFindNext();

protected:

    /// Override QTreeVire
    virtual void resizeEvent(QResizeEvent* event);
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);
    void InitContextMenu();
    QModelIndex FindNextText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity);
    QModelIndex FindPrevText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity);

private slots:

    void OnUpdateSectionWidth(int logicalIndex, int, int newSize);

    /// Context menu slots:
    void OnContextMenuEvent(const QPoint& position);
    void OnAboutToShowContextMenu();
    void OnItemClick(const QModelIndex& index);
    void OnItemDoubleClick(const QModelIndex& index);
    void OnItemExpand(const QModelIndex& index);
    void OnItemCollapsed(const QModelIndex& index);
    void OnVerticalScrollPositionChanged(int value);

private:

    void UpdateFrozenTableGeometry();
    void AppendRowToCopiedText(QString& copiedText, const QModelIndex& rowIndex, QModelIndexList& copiedRows);
private:

    /// Contain the frozen column tree view:
    QTreeView* m_pFrozenTreeView;

    /// Contain the frozen column index:
    int m_frozenColumn;

    /// Context menu:
    QMenu* m_pContextMenu;

    QModelIndex m_lastFindIndex;

    /// True iff we are in the function that is handling the resize of the header sections:
    bool m_isInResizeSections;

};

#endif //__ACFREEZETREEWIDGET_H

