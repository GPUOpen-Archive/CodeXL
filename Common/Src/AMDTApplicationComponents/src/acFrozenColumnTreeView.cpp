//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFrozenColumnTreeView.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acFrozenColumnTreeView.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>


acFrozenColumnTreeView::acFrozenColumnTreeView(QWidget* pParent, QAbstractItemModel* pModel, int frozenColumn)
    : QTreeView(pParent), m_pFrozenTreeView(nullptr), m_frozenColumn(frozenColumn), m_pContextMenu(nullptr), m_isInResizeSections(false)
{
    // Set my Model:
    setModel(pModel);

    m_pFrozenTreeView = new acNonScrolledTree(this, m_frozenColumn);


    m_pFrozenTreeView->setModel(model());
    m_pFrozenTreeView->setFocusPolicy(Qt::NoFocus);

    m_pFrozenTreeView->header()->setSectionResizeMode(QHeaderView::Interactive);

    header()->setSectionResizeMode(QHeaderView::Interactive);

    viewport()->stackUnder(m_pFrozenTreeView);

    // Get the color for the background and selection background:
    QString selectionBGColor = this->palette().highlight().color().name();
    QString color = acGetSystemDefaultBackgroundColor().name();

    QString cssString = QString("QTreeView { border: none;" "background-color: %1;" "selection-background-color: %2}").arg(color).arg(selectionBGColor);

    // Prevent header items from moving:
    header()->setSectionsMovable(false);
    m_pFrozenTreeView->header()->setSectionsMovable(false);

    header()->setSectionsClickable(true);
    m_pFrozenTreeView->header()->setSectionsClickable(true);

    m_pFrozenTreeView->setStyleSheet(cssString);
    m_pFrozenTreeView->setSelectionModel(selectionModel());

    // Hide all columns except for the frozen one:
    for (int col = 0; col < model()->columnCount(); col++)
    {
        if (col > m_frozenColumn)
        {
            m_pFrozenTreeView->setColumnHidden(col, true);
        }
    }

    // Align column width with the main widget:
    for (int i = 0 ; i <= m_frozenColumn; i++)
    {
        m_pFrozenTreeView->setColumnWidth(i, columnWidth(i));
    }

    m_pFrozenTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pFrozenTreeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pFrozenTreeView->show();

    UpdateFrozenTableGeometry();

    setHorizontalScrollMode(ScrollPerPixel);
    setVerticalScrollMode(ScrollPerPixel);
    m_pFrozenTreeView->setVerticalScrollMode(ScrollPerPixel);

    // Connect the headers and scrollbars of both tree views together:
    bool rc = connect(header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(OnUpdateSectionWidth(int, int, int)));
    GT_ASSERT(rc);

    rc = connect(m_pFrozenTreeView->header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(OnUpdateSectionWidth(int, int, int)));
    GT_ASSERT(rc);

    rc = connect(m_pFrozenTreeView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(OnVerticalScrollPositionChanged(int)));
    GT_ASSERT(rc);

    rc = connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(OnVerticalScrollPositionChanged(int)));
    GT_ASSERT(rc);

    rc = connect(m_pFrozenTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(OnItemCollapsed(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(OnItemCollapsed(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(m_pFrozenTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(OnItemExpand(const QModelIndex&)));
    GT_ASSERT(rc);

    rc = connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(OnItemExpand(const QModelIndex&)));
    GT_ASSERT(rc);

    // Enable multi-line selection:
    setSelectionMode(ContiguousSelection);
    setSelectionBehavior(SelectRows);
    m_pFrozenTreeView->setSelectionMode(ContiguousSelection);
    m_pFrozenTreeView->setSelectionBehavior(SelectRows);

    // Initialize the context menu;
    InitContextMenu();
}

acFrozenColumnTreeView::~acFrozenColumnTreeView()
{
    delete m_pFrozenTreeView;
}

void acFrozenColumnTreeView::OnUpdateSectionWidth(int logicalIndex, int, int newSize)
{
    // Prevent an infinite loop:
    if (!m_isInResizeSections)
    {
        m_isInResizeSections = true;

        if (logicalIndex <= m_frozenColumn)
        {
            setColumnWidth(logicalIndex, newSize);
            m_pFrozenTreeView->setColumnWidth(logicalIndex, newSize);
            UpdateFrozenTableGeometry();
        }
        else
        {
            m_pFrozenTreeView->setColumnWidth(logicalIndex, newSize);
        }

        m_isInResizeSections = false;
    }
}

void acFrozenColumnTreeView::resizeEvent(QResizeEvent* event)
{
    QTreeView::resizeEvent(event);
    UpdateFrozenTableGeometry();
}

QModelIndex acFrozenColumnTreeView::moveCursor(CursorAction cursorAction,
                                               Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = QTreeView::moveCursor(cursorAction, modifiers);

    int frozenColsWidth = 0;

    for (int i = 0 ; i <= m_frozenColumn; i++)
    {
        frozenColsWidth += m_pFrozenTreeView->columnWidth(i);
    }

    if ((cursorAction == MoveLeft && current.column() > m_frozenColumn)
        && visualRect(current).topLeft().x() < frozenColsWidth)
    {
        const int newValue = horizontalScrollBar()->value() + visualRect(current).topLeft().x() - m_pFrozenTreeView->columnWidth(m_frozenColumn);
        horizontalScrollBar()->setValue(newValue);
    }

    return current;
}

void acFrozenColumnTreeView::scrollTo(const QModelIndex& index, ScrollHint hint)
{
    if (index.column() > m_frozenColumn)
    {
        QTreeView::scrollTo(index, hint);
    }
    else
    {
        m_pFrozenTreeView->scrollTo(index, hint);
    }
}

void acFrozenColumnTreeView::UpdateFrozenTableGeometry()
{
    int frozenColumnsWidths = 0;

    for (int i = 0 ; i <= m_frozenColumn; i++)
    {
        frozenColumnsWidths += columnWidth(i);
    }

    m_pFrozenTreeView->setGeometry(frameWidth(), frameWidth(), frozenColumnsWidths, viewport()->height() + header()->height());
}

void acFrozenColumnTreeView::AppendRowToCopiedText(QString& copiedText, const QModelIndex& rowIndex, QModelIndexList& copiedRows)
{
    // First copy all the columns for this row
    for (int col = 0; col < header()->count(); col++)
    {
        if (!isColumnHidden(col))
        {
            // Get the text for the current column
            QModelIndex colIndex = model()->index(rowIndex.row(), col, QModelIndex());
            QString currentColText = model()->data(colIndex, Qt::DisplayRole).toString();

            // Add the text
            copiedText.append(currentColText);

            if (col != model()->columnCount() - 1)
            {
                copiedText.append(", ");
            }
        }
    }

    copiedText.append("\n");
    copiedRows << rowIndex;

    // Now copy all the children rows for this row index
    int rowCount = model()->rowCount(rowIndex);
    QModelIndexList selectedItemsList = selectionModel()->selectedIndexes();

    for (int i = 0; i < rowCount; ++i)
    {
        // Go through each of the columns, and add only the non-hidden and selected indices
        for (int col = 0; col < header()->count(); col++)
        {
            if (!isColumnHidden(col))
            {
                QModelIndex idx = model()->index(i, col, rowIndex);

                if (idx.isValid() && selectedItemsList.contains(idx))
                {
                    QString currentColText = idx.data(Qt::DisplayRole).toString();
                    copiedText.append(currentColText);

                    if (col != model()->columnCount() - 1)
                    {
                        copiedText.append(", ");
                    }
                }

                copiedRows << idx;

            }
        }

        // Append a newline for each row
        copiedText.append("\n");

    }
}

void acFrozenColumnTreeView::OnContextMenuEvent(const QPoint& position)
{
    if (m_pContextMenu != nullptr)
    {
        m_pContextMenu->exec(acMapToGlobal(this, position));
    }
}

void acFrozenColumnTreeView::OnAboutToShowContextMenu()
{
    if (m_pContextMenu != nullptr)
    {
        // Check if the actions should be enabled / disabled:
        bool isCopyEnabled = (selectedIndexes().size() >= 1);
        bool isSelectAllEnabled = (model()->rowCount() > 1);

        // Find the actions in the menu:
        QAction* pCopyAction = nullptr;
        QAction* pSelectAllAction = nullptr;

        foreach (QAction* pAction, m_pContextMenu->actions())
        {
            if (pAction->text() == AC_STR_listCtrlCopy)
            {
                pCopyAction = pAction;
            }

            if (pAction->text() == AC_STR_listCtrlSelectAll)
            {
                pSelectAllAction = pAction;
            }
        }

        // Set the actions enable / disable state:
        if (pCopyAction != nullptr)
        {
            pCopyAction->setEnabled(isCopyEnabled);
        }

        if (pSelectAllAction != nullptr)
        {
            pSelectAllAction->setEnabled(isSelectAllEnabled);
        }
    }
}

void acFrozenColumnTreeView::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != nullptr)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != nullptr)
        {
            // Get the selected items list:
            QModelIndexList selectedRowsList = selectionModel()->selectedRows();
            QModelIndexList selectedItemsList = selectionModel()->selectedIndexes();

            QString selectedText;

            if (!selectedRowsList.isEmpty())
            {
                for (int i = 0 ; i < model()->columnCount(); i++)
                {
                    if (!isColumnHidden(i))
                    {
                        selectedText.append(model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
                    }

                    if (i == model()->columnCount() - 1)
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

            QModelIndexList copiedRows;

            foreach (QModelIndex rowIndex, selectedRowsList)
            {
                if (!copiedRows.contains(rowIndex))
                {
                    AppendRowToCopiedText(selectedText, rowIndex, copiedRows);
                }
            }

            // Set the copied text to the clipboard:
            pClipboard->setText(selectedText);
        }
    }
}

void acFrozenColumnTreeView::onEditFind()
{
    onEditFindNext();
}

void acFrozenColumnTreeView::onEditFindNext()
{
    if (!acFindParameters::Instance().m_findExpr.isEmpty())
    {
        m_lastFindIndex = currentIndex();

        if (!m_lastFindIndex.isValid())
        {
            m_lastFindIndex = model()->index(0, 0, QModelIndex());
        }

        Qt::CaseSensitivity isCaseSensitive = acFindParameters::Instance().m_isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        QModelIndex findIndex;
        bool findFromStart = acFindParameters::Instance().m_findFromStart;

        if (acFindParameters::Instance().m_isSearchUp)
        {
            findIndex = FindPrevText(model(), QModelIndex(), findFromStart, acFindParameters::Instance().m_findExpr, isCaseSensitive);
        }
        else
        {
            findIndex = FindNextText(model(), QModelIndex(), findFromStart, acFindParameters::Instance().m_findExpr, isCaseSensitive);
        }

        if (findIndex.isValid())
        {
            m_pFrozenTreeView->setCurrentIndex(findIndex);
            acFindParameters::Instance().m_lastResult = findIndex != m_lastFindIndex;
        }
        else
        {
            acFindParameters::Instance().m_lastResult = false;
            m_pFrozenTreeView->setCurrentIndex(m_lastFindIndex);
        }
    }

    // After results are updated, ask the find widget to update the UI:
    acFindWidget::Instance().UpdateUI();
}

QModelIndex acFrozenColumnTreeView::FindNextText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity)
{
    QModelIndex retval; // empty index

    int rowCount = model->rowCount(parent);
    int colCount = model->columnCount(parent);

    for (int i = 0; i < rowCount; i++)
    {
        // if its collapsed (row is not displayed) - skip iteration
        if (!parent.isValid() || isExpanded(parent))
        {
            QModelIndex index;

            for (int j = 0; j < colCount; j++)
            {
                index = model->index(i, j, parent);

                if (index.isValid() && !retval.isValid() &&
                    !isIndexHidden(index) && pastLastResult)
                {
                    QString curText = index.data(Qt::DisplayRole).toString();

                    if (curText.indexOf(text, 0, caseSensitivity) >= 0)
                    {
                        return index;
                    }
                }

                if (index == m_lastFindIndex)
                {
                    pastLastResult = true;
                }
            }

            if (!retval.isValid())
            {
                index = model->index(i, 0, parent);
                retval = FindNextText(model, index, pastLastResult, text, caseSensitivity);
            }
        }
    }

    return retval;
}

QModelIndex acFrozenColumnTreeView::FindPrevText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity)
{
    QModelIndex retval; // empty index

    int rowCount = model->rowCount(parent);
    int colCount = model->columnCount(parent);

    for (int i = rowCount - 1; i >= 0; i--)
    {
        // if its collapsed (row is not displayed) - skip iteration
        if (!parent.isValid() || isExpanded(parent))
        {

            QModelIndex index;

            if (!retval.isValid())
            {
                index = model->index(i, 0, parent);
                retval = FindPrevText(model, index, pastLastResult, text, caseSensitivity);
            }

            for (int j = colCount - 1; j >= 0; j--)
            {
                index = model->index(i, j, parent);

                if (index.isValid() && !retval.isValid() &&
                    !isIndexHidden(index) && pastLastResult)
                {
                    QString curText = index.data(Qt::DisplayRole).toString();

                    if (curText.indexOf(text, 0, caseSensitivity) >= 0)
                    {
                        return index;
                    }
                }

                if (index == m_lastFindIndex)
                {
                    pastLastResult = true;
                }
            }
        }
    }

    return retval;
}

void acFrozenColumnTreeView::onEditSelectAll()
{
    selectAll();
}

void acFrozenColumnTreeView::InitContextMenu()
{
    // Allocate a menu:
    m_pContextMenu = new QMenu(this);


    // Create copy & select all action:
    m_pContextMenu->addAction(AC_STR_listCtrlCopy, this, SLOT(onEditCopy()));
    m_pContextMenu->addAction(AC_STR_listCtrlSelectAll, this, SLOT(onEditSelectAll()));

    // Connect the menu to its slots:
    bool rcConnect = connect(m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(OnAboutToShowContextMenu()));
    GT_ASSERT(rcConnect);

    // Create the context menu:
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_pFrozenTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    rcConnect = connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuEvent(const QPoint&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pFrozenTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuEvent(const QPoint&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnItemClick(const QModelIndex&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pFrozenTreeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnItemClick(const QModelIndex&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnItemDoubleClick(const QModelIndex&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pFrozenTreeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnItemDoubleClick(const QModelIndex&)));
    GT_ASSERT(rcConnect);

}

void acFrozenColumnTreeView::ExpandAll()
{
    GT_IF_WITH_ASSERT(m_pFrozenTreeView)
    {
        m_pFrozenTreeView->expandAll();
        expandAll();
    }
}

void acFrozenColumnTreeView::CollapseAll()
{
    GT_IF_WITH_ASSERT(m_pFrozenTreeView)
    {
        m_pFrozenTreeView->collapseAll();
        collapseAll();
    }
}

void acFrozenColumnTreeView::SetItemDelegate(QAbstractItemDelegate* pDelegate)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFrozenTreeView != nullptr) && (pDelegate != nullptr))
    {
        setItemDelegate(pDelegate);
        m_pFrozenTreeView->setItemDelegate(pDelegate);
    }
}

void acFrozenColumnTreeView::SetItemDelegateForColumn(int column, QAbstractItemDelegate* pDelegate)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFrozenTreeView != nullptr) && (pDelegate != nullptr))
    {
        setItemDelegateForColumn(column, pDelegate);
        m_pFrozenTreeView->setItemDelegateForColumn(column, pDelegate);
    }
}


bool acFrozenColumnTreeView::isItemSelected(const QModelIndex& index, bool& isFocused)
{
    bool retVal = false;
    isFocused = hasFocus();
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFrozenTreeView != nullptr)
    {
        if (index.column() <= m_frozenColumn)
        {
            retVal = m_pFrozenTreeView->selectionModel()->isRowSelected(index.row(), QModelIndex());
        }
        else
        {
            retVal = selectionModel()->isRowSelected(index.row(), QModelIndex());
        }
    }
    return retVal;
}

void acFrozenColumnTreeView::ScrollToItem(const QModelIndex& index, ScrollHint hint)
{
    GT_IF_WITH_ASSERT(m_pFrozenTreeView != nullptr)
    {
        scrollTo(index, hint);
        m_pFrozenTreeView->scrollTo(index, hint);
    }
}

void acFrozenColumnTreeView::ScrollToBottom()
{
    GT_IF_WITH_ASSERT(m_pFrozenTreeView != nullptr)
    {
        scrollToBottom();
        m_pFrozenTreeView->scrollToBottom();
    }
}

void acFrozenColumnTreeView::ShowColumn(int columnIndex, bool shouldShow)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pFrozenTreeView != nullptr)
    {
        if (shouldShow)
        {
            showColumn(columnIndex);
            m_pFrozenTreeView->showColumn(columnIndex);
            m_pFrozenTreeView->resizeColumnToContents(columnIndex);
            resizeColumnToContents(columnIndex);
        }
        else
        {
            hideColumn(columnIndex);
            m_pFrozenTreeView->hideColumn(columnIndex);
        }
    }
}

void acFrozenColumnTreeView::OnItemClick(const QModelIndex& index)
{
    // Emit a signal stating that the item was clicked:
    emit ItemClicked(index);
}

void acFrozenColumnTreeView::OnItemDoubleClick(const QModelIndex& index)
{
    // Emit a signal stating that the item was double-clicked:
    emit ItemDoubleClicked(index);
}


void acFrozenColumnTreeView::OnItemExpand(const QModelIndex& index)
{
    // Align both trees expand state:
    if (sender() == m_pFrozenTreeView)
    {
        expand(index);
    }
    else
    {
        m_pFrozenTreeView->expand(index);
    }


    emit ItemExpanded(index);
}

void acFrozenColumnTreeView::OnItemCollapsed(const QModelIndex& index)
{
    // Align both trees expand state:
    if (sender() == m_pFrozenTreeView)
    {
        collapse(index);
    }
    else
    {
        m_pFrozenTreeView->collapse(index);
    }


    emit ItemCollapsed(index);
}

void acFrozenColumnTreeView::OnVerticalScrollPositionChanged(int value)
{
    GT_IF_WITH_ASSERT(m_pFrozenTreeView != nullptr)
    {
        // Align both scrolls values:
        if (sender() == m_pFrozenTreeView->verticalScrollBar())
        {
            verticalScrollBar()->setValue(value);
        }
        else
        {
            m_pFrozenTreeView->verticalScrollBar()->setValue(value);
        }
    }

    // Emit a signal for vertical scroll position change:
    emit VerticalScrollPositionChanged(value);
}

void acFrozenColumnTreeView::SetItemExpanded(const QModelIndex& index, bool expanded)
{
    GT_IF_WITH_ASSERT(m_pFrozenTreeView)
    {
        m_pFrozenTreeView->setExpanded(index, expanded);
        setExpanded(index, expanded);
    }
}

void acNonScrolledTree::scrollTo(const QModelIndex& index, ScrollHint hint)
{
    if (index.isValid())
    {
        // Block the scroll for the columns that are hidden:
        if (index.column() <= m_frozenColumnIndex)
        {
            QTreeView::scrollTo(index, hint);
        }
    }
}
