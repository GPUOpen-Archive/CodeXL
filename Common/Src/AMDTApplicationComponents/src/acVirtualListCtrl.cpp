//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acVirtualListCtrl.cpp
///
//==================================================================================

//------------------------------ acVirtualListCtrl.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <inc/acStringConstants.h>

// Used to divide the find operation when searching backwards:
#define AC_FIND_BLOCK_SIZE 50

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::acVirtualListCtrl
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
acVirtualListCtrl::acVirtualListCtrl(QWidget* pParent, QAbstractTableModel* pTableModel, bool enableFindInContextMenu)
    : QTableView(pParent), m_pContextMenu(NULL), m_pCopyAction(NULL), m_pSelectAllAction(NULL),
      m_pFindAction(NULL), m_pFindNextAction(NULL), m_isFindEnabled(enableFindInContextMenu),
      m_isInFindOperation(false), m_shouldAdvanceFindNextLine(false)
{
    // Set the model it its not null:
    if (pTableModel != NULL)
    {
        setModel(pTableModel);
    }

    // Context menu:
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenuEvent(const QPoint&)));

    // Initialize the context menu:
    initContextMenu();

    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    // Connect the item selection events:
    bool rcConnect = connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(onRowSelected(const QModelIndex&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onRowSelected(const QModelIndex&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(pressed(const QModelIndex&)), this, SLOT(onRowSelected(const QModelIndex&)));
    GT_ASSERT(rcConnect);
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::~acVirtualListCtrl
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
acVirtualListCtrl::~acVirtualListCtrl()
{

}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::initContextMenu
// Description: Initialize the context menu
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::initContextMenu()
{
    // Allocate a menu:
    m_pContextMenu = new QMenu(this);


    // Create copy action:
    m_pCopyAction = m_pContextMenu->addAction(AC_STR_listCtrlCopy);
    GT_ASSERT(m_pCopyAction != NULL);

    // Create select all action:
    m_pSelectAllAction = m_pContextMenu->addAction(AC_STR_SelectAll);
    GT_ASSERT(m_pSelectAllAction != NULL);

    // Connect the action to delete slot:
    bool rcConnect = connect(m_pCopyAction, SIGNAL(triggered()), this, SLOT(onEditCopy()));
    GT_ASSERT(rcConnect);

    // Connect the action to delete slot:
    rcConnect = connect(m_pSelectAllAction, SIGNAL(triggered()), this, SLOT(onEditSelectAll()));
    GT_ASSERT(rcConnect);

    if (m_isFindEnabled)
    {
        // Insert find + find next actions:
        m_pFindAction = m_pContextMenu->addAction(AC_STR_listCtrlFind);
        GT_ASSERT(m_pFindAction != NULL);
        rcConnect = connect(m_pFindAction, SIGNAL(triggered()), this, SLOT(onFindClick()));
        GT_ASSERT(rcConnect);

        // Insert find + find next actions:
        m_pFindNextAction = m_pContextMenu->addAction(AC_STR_listCtrlFindNext);
        GT_ASSERT(m_pFindNextAction != NULL);

        rcConnect = connect(m_pFindAction, SIGNAL(triggered()), this, SLOT(onFindNext()));
        GT_ASSERT(rcConnect);
    }

    // Connect the menu to its slots:
    rcConnect = connect(m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowContextMenu()));
    GT_ASSERT(rcConnect);

}


// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onContextMenuEvent
// Description: Is connected to the context menu request signal - display the
//              context menu if it is initialized
// Arguments:   const QPoint &
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onContextMenuEvent(const QPoint& position)
{
    if (m_pContextMenu != NULL)
    {
        m_pContextMenu->exec(acMapToGlobal(this, position));
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onFindNext
// Description:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onFindNext()
{
    onFindClick();
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::onFindNext
// Description:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onFindPrev()
{
    // m_isSearchUp flag is up only when find prev button is pressed
    acFindParameters::Instance().m_isSearchUp = true;
    onFindNext();
    acFindParameters::Instance().m_isSearchUp = false;
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onFindClick
// Description: Handle find command
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onFindClick()
{

    // Get the model:
    QAbstractItemModel* pDataModel = model();

    // Sanity check:
    GT_IF_WITH_ASSERT(pDataModel != NULL)
    {
        // Define the Qt find flags matching the user selection:
        Qt::MatchFlags findFlags = Qt::MatchContains;

        if (acFindParameters::Instance().m_isCaseSensitive)
        {
            findFlags = findFlags | Qt::MatchCaseSensitive;
        }

        if (m_shouldAdvanceFindNextLine)
        {
            // Clear the find index:
            if (acFindParameters::Instance().m_findFirstLine >= 0)
            {
                if (acFindParameters::Instance().m_isSearchUp)
                {
                    acFindParameters::Instance().m_findFirstLine--;

                    if (acFindParameters::Instance().m_findFirstLine < 0)
                    {
                        acFindParameters::Instance().m_findFirstLine = pDataModel->rowCount() - 1;
                    }
                }
                else
                {
                    acFindParameters::Instance().m_findFirstLine++;

                    if (acFindParameters::Instance().m_findFirstLine >= pDataModel->rowCount())
                    {
                        acFindParameters::Instance().m_findFirstLine = 0;
                    }
                }
            }
        }

        // Get the item with the same string:
        findAndSelectNext(pDataModel, findFlags);
    }
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onAboutToShowContextMenu
// Description: Is called when the context menu is about to be shown
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onAboutToShowContextMenu()
{
    GT_IF_WITH_ASSERT(m_pCopyAction != NULL)
    {
        bool isEnabled;
        onUpdateEditCopy(isEnabled);
        m_pCopyAction->setEnabled(isEnabled);
    }

    GT_IF_WITH_ASSERT(m_pSelectAllAction != NULL)
    {
        bool isEnabled;
        onUpdateEditSelectAll(isEnabled);
        m_pSelectAllAction->setEnabled(isEnabled);
    }

    if (m_pFindAction != NULL)
    {
        bool isEnabled;
        onUpdateEditFind(isEnabled);
        m_pFindAction->setEnabled(isEnabled);
    }

    if (m_pFindNextAction != NULL)
    {
        bool isEnabled;
        onUpdateEditFindNext(isEnabled);
        m_pFindNextAction->setEnabled(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onEditCopy
// Description:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != NULL)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != NULL)
        {
            // Get the selected items list:
            QModelIndexList list = selectionModel()->selectedRows();
            int colCount = model()->columnCount(QModelIndex());

            // Get my data model:
            QAbstractItemModel* pDataModel = model();
            GT_IF_WITH_ASSERT(pDataModel != NULL)
            {
                QString selectedText;

                // Copy column headers:
                for (int col = 0; col < colCount; col++)
                {
                    // Get the data for the index:
                    QVariant var = model()->headerData(col, Qt::Horizontal, Qt::DisplayRole);
                    QString currentText = var.toString();

                    // Add the text:
                    selectedText.append(currentText);

                    if (col != colCount - 1)
                    {
                        selectedText.append(", ");
                    }
                }

                // Last column - add new line:
                selectedText.append("\n");

                QList<int> rowsCopied;

                foreach (QModelIndex modelIndex, list)
                {
                    // Get the current item row:
                    int row = modelIndex.row();

                    // If row wad not copied yet:
                    if (rowsCopied.indexOf(row) == -1)
                    {
                        // Copy all columns of this row:
                        for (int i = 0 ; i < colCount; i++)
                        {
                            QModelIndex currentColIndex = model()->index(row, i);
                            QVariant var = model()->data(currentColIndex, Qt::DisplayRole);
                            QString currentColText = var.toString();

                            // Add the text:
                            selectedText.append(currentColText);

                            if (i == colCount - 1)
                            {
                                // Last column - add new line:
                                selectedText.append("\n");
                            }
                            else
                            {
                                selectedText.append(", ");
                            }
                        }
                    }
                }

                // Set the copied text to the clipboard:
                pClipboard->setText(selectedText);
            }
        }
    }
}

void acVirtualListCtrl::onEditSelectAll()
{
    selectAll();
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onUpdateEditCopy
// Description: Check if the action should be enabled
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onUpdateEditCopy(bool& isEnabled)
{
    isEnabled = (selectedIndexes().size() > 0);
}

void acVirtualListCtrl::onUpdateEditSelectAll(bool& isEnabled)
{
    isEnabled = false;
    QAbstractItemModel* pModel = model();
    GT_IF_WITH_ASSERT(pModel != NULL)
    {
        isEnabled = (pModel->rowCount(QModelIndex()) > 0);
    }
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onUpdateEditFind
// Description: Check if the action should be enabled
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onUpdateEditFind(bool& isEnabled)
{
    isEnabled = false;
    QAbstractItemModel* pModel = model();
    GT_IF_WITH_ASSERT(pModel != NULL)
    {
        isEnabled = (pModel->rowCount(QModelIndex()) > 0);
    }
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onUpdateEditFindNext
// Description: Check if the action should be enabled
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onUpdateEditFindNext(bool& isEnabled)
{
    isEnabled = true;
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::onRowSelected
// Description: Set the selected row as the first line selected
// Arguments:   const QModelIndex& index
// Author:      Sigal Algranaty
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acVirtualListCtrl::onRowSelected(const QModelIndex& index)
{
    // Reset the find line index:
    acFindParameters::Instance().m_findFirstLine = index.row();

    if (!m_isInFindOperation)
    {
        m_shouldAdvanceFindNextLine = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::currentChanged
// Description: Is handling a selection chaged event
// Arguments:   const QModelIndex &current
//              const QModelIndex &previous
// Author:      Sigal Algranaty
// Date:        18/3/2012
// ---------------------------------------------------------------------------
void acVirtualListCtrl::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    onRowSelected(current);

    QTableView::currentChanged(current, previous);
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::findAndSelectNext
// Description: Find the item from the selected point to the end of the list, and
//              select if found
// Arguments:   QAbstractItemModel* pDataModel
//              Qt::MatchFlags findFlags
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/3/2012
// ---------------------------------------------------------------------------
bool acVirtualListCtrl::findAndSelectNext(QAbstractItemModel* pDataModel, Qt::MatchFlags findFlags)
{
    bool retVal = false;

    // Do not perform find operation while running one:
    if (!m_isInFindOperation)
    {
        // Get the next index matching the search string:
        int foundInRow = findNextMatchingIndex(pDataModel, findFlags);

        // Set the find result in the find parameters class:
        acFindParameters::Instance().m_lastResult = (foundInRow >= 0);

        if (foundInRow >= 0)
        {
            // Set the find index:
            acFindParameters::Instance().m_findFirstLine = foundInRow;

            // Clear selection:
            clearSelection();

            // Set the selection item:
            selectRow(acFindParameters::Instance().m_findFirstLine);

            // Next time advance the first line:
            m_shouldAdvanceFindNextLine = true;

            // Focus the list control:
            setFocus();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::findNextMatchingIndex
// Description: Find the next matching item
// Arguments:   QAbstractItemModel* pDataModel
//              Qt::MatchFlags findFlags
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        28/3/2012
// ---------------------------------------------------------------------------
int acVirtualListCtrl::findNextMatchingIndex(QAbstractItemModel* pDataModel, Qt::MatchFlags findFlags)
{
    int retVal = -1;

    // Sanity check
    GT_IF_WITH_ASSERT(pDataModel != NULL)
    {
        if (!acFindParameters::Instance().m_isSearchUp)
        {
            // When searching down, we use the usual match function, but we only ask for the next single hit:
            QModelIndexList matchingItems = pDataModel->match(pDataModel->index(acFindParameters::Instance().m_findFirstLine, 0), Qt::DisplayRole, acFindParameters::Instance().m_findExpr, 1, findFlags);

            if (!matchingItems.isEmpty())
            {
                // Select the first one:
                retVal = matchingItems.first().row();
            }
            else
            {
                // Search again from the beginning of the list:
                matchingItems = pDataModel->match(pDataModel->index(0, 0), Qt::DisplayRole, acFindParameters::Instance().m_findExpr, 1, findFlags);

                if (!matchingItems.isEmpty())
                {
                    // Select the first one:
                    retVal = matchingItems.first().row();
                }
            }
        }

        else
        {
            // Search up:
            // We do not want to use the match function, since when the list is big, we have performance issues, and match doesn't support reverse order.
            // Get the rows amount:
            int rowsAmount = pDataModel->rowCount();
            int firstFindIndex = acFindParameters::Instance().m_findFirstLine;

            for (int i = firstFindIndex; i >= 0; i--)
            {
                bool doesMatch = doesItemMatch(pDataModel, i, findFlags);

                if (doesMatch)
                {
                    retVal = i;
                    break;
                }
            }

            if (retVal < 0)
            {
                // Search from end:
                for (int i = rowsAmount - 1; i >= acFindParameters::Instance().m_findFirstLine; i--)
                {
                    bool doesMatch = doesItemMatch(pDataModel, i, findFlags);

                    if (doesMatch)
                    {
                        retVal = i;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::doesItemMatch
// Description: Does item row in data model matches the find expression?
// Arguments:   QAbstractItemModel* pDataModel
//              int row
//              Qt::MatchFlags findFlags
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/3/2012
// ---------------------------------------------------------------------------
bool acVirtualListCtrl::doesItemMatch(QAbstractItemModel* pDataModel, int row, Qt::MatchFlags findFlags)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(pDataModel != NULL)
    {
        QModelIndexList result;
        uint matchType = findFlags & 0x0F;
        Qt::CaseSensitivity cs = findFlags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        QString text;
        QModelIndex index = pDataModel->index(row, 0);
        QVariant valueVariant = pDataModel->data(index, Qt::DisplayRole);

        // QVariant based matching
        if (matchType == Qt::MatchExactly)
        {
            retVal = (valueVariant.toString() == acFindParameters::Instance().m_findExpr);
        }
        else if (matchType == Qt::MatchContains)
        {
            retVal = valueVariant.toString().contains(acFindParameters::Instance().m_findExpr, cs);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::isItemSelected
// Description: Return true iff the requested item is selected
// Arguments:   int row - the item row number
//              int col - the item column number
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
bool acVirtualListCtrl::isItemSelected(int row, int col)
{
    bool retVal = false;

    foreach (QModelIndex index, selectedIndexes())
    {
        // If this is the requested item, break;
        if ((index.row() == row) && (index.column() == col))
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::updateData
// Description: Update the data for the table
// Author:      Sigal Algranaty
// Date:        9/7/2012
// ---------------------------------------------------------------------------
void acVirtualListCtrl::updateData()
{
    // Get the relevant data model:
    QAbstractItemModel* pModel = model();

    if (pModel != NULL)
    {
        // Emit a datqa changed signal:
        QModelIndex indexTop = pModel->index(0, 0);
        QModelIndex indexBottom = pModel->index(pModel->rowCount(), pModel->columnCount());

        emit dataChanged(indexTop, indexBottom);
    }

    // Repaint the table:
    setDirtyRegion(visibleRegion()); QTableView::repaint();
}


// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrl::mouseMoveEvent
// Description: Overrides QAbstractItemView. Is used to emit a siganl that an
//              item was hovered
// Arguments:   QMouseEvent* pMouseEvent
// Author:      Sigal Algranaty
// Date:        30/7/2012
// ---------------------------------------------------------------------------
void acVirtualListCtrl::mouseMoveEvent(QMouseEvent* pMouseEvent)
{
    if (pMouseEvent != NULL)
    {
        // Get the item index for the mouse point:
        QModelIndex hoveredItemIndex = indexAt(pMouseEvent->pos());
        emit itemHovered(hoveredItemIndex);
    }

    // Call the base class implementation:
    QAbstractItemView::mouseMoveEvent(pMouseEvent);
}

