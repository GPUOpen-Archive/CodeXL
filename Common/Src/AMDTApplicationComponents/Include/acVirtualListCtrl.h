//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acVirtualListCtrl.h
///
//==================================================================================

//------------------------------ acVirtualListCtrl.h ------------------------------

#ifndef __ACVIRTUALLISTCTRL_H
#define __ACVIRTUALLISTCTRL_H

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acFindParameters.h>

class acFindWidget;

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acVirtualListCtrl : public QTableWidget
// General Description: Defines a virtual list control with Qt GUI
// Author:              Sigal Algranaty
// Creation Date:       25/12/2011
// ----------------------------------------------------------------------------------
class AC_API acVirtualListCtrl : public QTableView
{
    Q_OBJECT

public:
    // Constructor:
    acVirtualListCtrl(QWidget* pParent, QAbstractTableModel* pTableModel, bool enableFindInContextMenu = true);

    // Destructor:
    ~acVirtualListCtrl();

    virtual void onUpdateEditCopy(bool& isEnabled);
    virtual void onUpdateEditSelectAll(bool& isEnabled);
    virtual void onUpdateEditFind(bool& isEnabled);
    virtual void onUpdateEditFindNext(bool& isEnabled);

    void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {QTableView::setSelection(rect, command);};
    QModelIndexList selectedIndexes() const {return QTableView::selectedIndexes();};
    bool isItemSelected(int row, int col);

    void updateData();

    QMenu* contextMenu() {return m_pContextMenu;}

Q_SIGNALS:

    void itemHovered(const QModelIndex& hoveredItem);

public slots:


    /// Slots implementing the find command. Notice: This slot names cannot be changed, since it is connected in the construction of the main window
    /// Is called when the main window find is clicked:
    virtual void onFindClick();

    /// Is called when the main window find next is clicked:
    virtual void onFindNext();

    /// Is called when the main window find previous is clicked:
    virtual void onFindPrev();

    virtual void onEditCopy();
    virtual void onEditSelectAll();
    virtual void onRowSelected(const QModelIndex& index);

protected slots:

    virtual void onContextMenuEvent(const QPoint& point);
    virtual void onAboutToShowContextMenu();
    virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous);

protected:

    void initContextMenu();
    bool findAndSelectNext(QAbstractItemModel* pDataModel, Qt::MatchFlags findFlags);

    int findNextMatchingIndex(QAbstractItemModel* pDataModel, Qt::MatchFlags findFlags);
    bool doesItemMatch(QAbstractItemModel* pDataModel, int row, Qt::MatchFlags findFlags);


    // Overriding QAbstractItemView:
    virtual void mouseMoveEvent(QMouseEvent* pMouseEvent);

protected:

    // Context menu:
    QMenu* m_pContextMenu;
    QAction* m_pCopyAction;
    QAction* m_pSelectAllAction;
    QAction* m_pFindAction;
    QAction* m_pFindNextAction;

    // Contain true iff find operation is enabled:
    bool m_isFindEnabled;

    // Contain true while we're in find operation:
    bool m_isInFindOperation;

    //
    bool m_shouldAdvanceFindNextLine;
};

#endif //__ACVIRTUALLISTCTRL_H

