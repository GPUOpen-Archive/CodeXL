//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acListCtrl.cpp
///
//==================================================================================

//------------------------------ acListCtrl.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acHeaderView.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acTableWidgetItem.h>
#include <inc/acStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        acListCtrl::acListCtrl
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
acListCtrl::acListCtrl(QWidget* pParent, int rowHeight, bool isDeleteEnabled, bool areItemsSelectable)
    : QTableWidget(pParent), m_rowHeight(acScaleSignedPixelSizeToDisplayDPI(rowHeight)), m_displayOnlyFirstColumn(false), m_removeThousandSeparatorOnCopy(false),
      m_enableRowDeletion(isDeleteEnabled), m_areItemsSelectable(areItemsSelectable), m_shouldCopyColumnHeaders(false),
      m_pContextMenu(nullptr), m_pCopyAction(nullptr), m_pPasteAction(nullptr), m_pSelectAllAction(nullptr), m_pDeleteAction(nullptr),
      m_isPasteActionEnabled(false), m_ignoreResizeEvents(false), m_shouldEnableDeletionOfEditableRow(false)
{
    // Create customized horizontal header:
    acHeaderView* pHeaderView = new acHeaderView(Qt::Horizontal, this);

    pHeaderView->setSectionsClickable(true);

    // Enable header columns re-order:
    pHeaderView->setSectionsMovable(true);

    // Set my header:
    setHorizontalHeader(pHeaderView);

    verticalHeader()->setDefaultSectionSize(m_rowHeight);

    // Default table properties:
    setShowGrid(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // Context menu:
    setContextMenuPolicy(Qt::CustomContextMenu);
    bool rcConnect = connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenuEvent(const QPoint&)));
    GT_ASSERT(rcConnect);

    // Initialize the context menu:
    initContextMenu();

    // Enable context menu:
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    horizontalHeader()->setSelectionMode(QAbstractItemView::SingleSelection);

    // Connect the header context menu slot:
    rcConnect = connect(horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnHeaderContextMenu(const QPoint&)));
    GT_ASSERT(rcConnect);

}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::~acListCtrl
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
acListCtrl::~acListCtrl()
{

}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::initHeaders
// Description: Initializes the columns
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::initHeaders(const QStringList& columnCaptions, bool showVerticalHeaders)
{
    // Set the column count:
    setColumnCount(columnCaptions.size());

    // One column special case:
    if (columnCaptions.size() == 1)
    {
        m_displayOnlyFirstColumn = true;
    }

    // Sanity check:
    int numberOfColumnCaptions = (int)columnCaptions.size();

    for (int i = 0; i < numberOfColumnCaptions; i++)
    {
        // For each of the columns set the item data:
        QTableWidgetItem* pColumnHeader = allocateNewWidgetItem(columnCaptions[i]);

        // Set the default text alignment:
        pColumnHeader->setTextAlignment(Qt::AlignLeft);

        // Set the header item data:
        setHorizontalHeaderItem(i, pColumnHeader);

        horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
    }

    // Set the last section to stretch:
    horizontalHeader()->setStretchLastSection(true);

    // Hide / Show the vertical headers:
    if (!showVerticalHeaders)
    {
        verticalHeader()->hide();
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::initHeaders
// Description: Initializes the columns (and column widths)
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void acListCtrl::initHeaders(const QStringList& columnCaptions, const gtVector<float>&  columnWidths, bool showVerticalHeaders)
{
    GT_IF_WITH_ASSERT(columnWidths.size() == (unsigned int)columnCaptions.size())
    {
        // Clear the columns widths:
        m_columnsWidths.clear();

        // Set the column count:
        setColumnCount(columnCaptions.size());

        // One column special case:
        if (columnCaptions.size() == 1)
        {
            m_displayOnlyFirstColumn = true;
        }

        // Sanity check:
        int numberOfColumnCaptions = (int)columnCaptions.size();

        for (int i = 0; i < numberOfColumnCaptions; i++)
        {
            // For each of the columns set the item data:
            QTableWidgetItem* pColumnHeader = allocateNewWidgetItem(columnCaptions[i]);

            // Set the default text alignment:
            pColumnHeader->setTextAlignment(Qt::AlignLeft);

            // Set the header item data:
            setHorizontalHeaderItem(i, pColumnHeader);

            horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);

            // Set column width:
            m_columnsWidths.push_back(columnWidths[i]);
        }

        // Set the last section to stretch:
        horizontalHeader()->setStretchLastSection(true);

        // Hide / Show the vertical headers:
        if (!showVerticalHeaders)
        {
            verticalHeader()->hide();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::initItem
// Description: Initializes the item
// Author:      Ehud Katz
// Date:        13/5/2013
// ---------------------------------------------------------------------------
void acListCtrl::initItem(QTableWidgetItem& witem, const QString& text, const QVariant* pItemData, bool withCheckBox, Qt::CheckState checkState, QPixmap* pItemIcon)
{
    // Set the text as tooltip:
    witem.setToolTip(text);

    // Set flags:
    // Set flags:
    Qt::ItemFlags flags = witem.flags() & (~Qt::ItemIsEditable);

    if (m_areItemsSelectable)
    {
        flags = flags | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    witem.setFlags(flags);


    // Set the first column attributes:

    // Set the item icon:
    if (nullptr != pItemIcon)
    {
        witem.setIcon(*pItemIcon);
    }

    if (withCheckBox)
    {
        // Set check state:
        witem.setCheckState(checkState);
    }

    if (nullptr != pItemData)
    {
        witem.setData(Qt::UserRole, *pItemData);
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::initItem
// Description: Initializes the item
// Author:      Ehud Katz
// Date:        13/5/2013
// ---------------------------------------------------------------------------
QTableWidgetItem* acListCtrl::initItem(int row, int column, const QString& text, const QVariant* pItemData, bool withCheckBox, Qt::CheckState checkState, QPixmap* pItemIcon)
{
    QTableWidgetItem* pItem = nullptr;
    // Sanity check:
    GT_IF_WITH_ASSERT(row < rowCount() && column < columnCount())
    {
        pItem = item(row, column);

        if (nullptr != pItem)
        {
            pItem->setText(text);
            initItem(*pItem, text, pItemData, withCheckBox, checkState, pItemIcon);
        }
    }
    return pItem;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::initRowItems
// Description: Initializes the items in the row
// Author:      Ehud Katz
// Date:        13/5/2013
// ---------------------------------------------------------------------------
void acListCtrl::initRowItems(int row, const QStringList& rowTexts, int itemAlignment)
{
    const int colsAmount = columnCount();
    // Sanity check:
    GT_IF_WITH_ASSERT(row < rowCount() && rowTexts.size() == colsAmount)
    {
        for (int col = 0; col < colsAmount; col++)
        {
            QTableWidgetItem* pItem = item(row, col);

            if (nullptr != pItem)
            {
                const QString& text = rowTexts[col];
                pItem->setText(text);
                initItem(*pItem, text, nullptr, false, Qt::Unchecked, nullptr);
                pItem->setTextAlignment(itemAlignment);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::addRow
// Description: add a simple row with one column
// Arguments:   const QString& text
//              const QVariant& itemData
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        23/4/2012
// ---------------------------------------------------------------------------
bool acListCtrl::addRow(const QString& text, const QVariant& itemData)
{
    bool retVal = false;
    // Sanity check:
    GT_IF_WITH_ASSERT(columnCount() == 1)
    {
        int row = rowCount();
        insertRow(row);
        // Initialize a table widget item:
        QTableWidgetItem* pItem = allocateNewWidgetItem(text);

        initItem(*pItem, text, &itemData);

        // Set the item:
        setItem(row, 0, pItem);

        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::addRow
// Description: add a row using a void pointer
// Arguments:   const QString& text
//              void* pItemData
//              bool withCheckBox
//              Qt::CheckState checkState
//              QPixmap* pItemIcon
//              bool beforeLastRow
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        21/6/2012
// ---------------------------------------------------------------------------
bool acListCtrl::addRow(const QString& text, void* pItemData, bool withCheckBox, Qt::CheckState checkState, QPixmap* pItemIcon, bool beforeLastRow)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(columnCount() == 1)
    {
        retVal = true;
        int row = rowCount();

        if ((row > 0) && (beforeLastRow)) { row--; }

        insertRow(row);

        // Initialize a table widget item:
        QTableWidgetItem* pItem = allocateNewWidgetItem(text);

        const QVariant* pData = nullptr;
        QVariant data1;

        if (pItemData != nullptr)
        {
            data1 = qVariantFromValue(pItemData);
            pData = &data1;
        }

        initItem(*pItem, text, pData, withCheckBox, checkState, pItemIcon);

        // Set the item:
        setItem(row, 0, pItem);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::addRow
// Description: Add a row with multiple columns
// Arguments:   const QStringList& rowTexts
//              void* pItemData
//              bool withCheckBox
//              Qt::CheckState checkState
//              QPixmap* pItemIcon
//              bool beforeLastRow
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        21/6/2012
// ---------------------------------------------------------------------------
bool acListCtrl::addRow(const QStringList& rowTexts, void* pItemData, bool withCheckBox, Qt::CheckState checkState, QPixmap* pItemIcon, bool beforeLastRow)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(rowTexts.size() == columnCount())
    {
        retVal = true;

        // Display more then one column if we have more then one:
        if (columnCount() != 1)
        {
            m_displayOnlyFirstColumn = false;
        }

        int row = rowCount();

        if ((row > 0) && (beforeLastRow)) { row--; }

        insertRow(row);

        for (int i = 0; i < (int)rowTexts.size(); i++)
        {
            const QString& text = rowTexts[i];
            // Initialize a table widget item:
            QTableWidgetItem* pItem = allocateNewWidgetItem(text);

            // Set the first column attributes:
            if (i == 0)
            {
                const QVariant* pData = nullptr;
                QVariant data1;

                if (pItemData != nullptr)
                {
                    data1 = qVariantFromValue(pItemData);
                    pData = &data1;
                }

                initItem(*pItem, text, pData, withCheckBox, checkState, pItemIcon);
            }
            else
            {
                initItem(*pItem, text, nullptr);
            }

            // Set the item:
            setItem(row, i, pItem);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::addRow
// Description: Add row with only single column
// Arguments:   const QString& text
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/1/2012
// ---------------------------------------------------------------------------
bool acListCtrl::addRow(const QString& text)
{
    QStringList list;
    int colsNum = columnCount();

    for (int i = 0; i < colsNum; ++i)
    {
        if (i == 0)
        {
            list << text;
        }
        else
        {
            list << "";
        }
    }

    return addRow(list, nullptr);
}

bool acListCtrl::addMultiLineRow(const QString& text)
{
    bool retVal = false;
    int row = rowCount();

    // Using addRow to create a new row and then fill it with a text edit to allow word wrap
    GT_ASSERT(addRow(text))
    {
        QTextEdit* pTextEdit = new QTextEdit();
        pTextEdit->setWordWrapMode(QTextOption::WordWrap);
        pTextEdit->setReadOnly(true);
        pTextEdit->setText(text);
        setCellWidget(row, 0, pTextEdit);
        QTableView::setRowHeight(row, pTextEdit->height());

        retVal = true;
    }

    return retVal;
}

bool acListCtrl::addRow(const QStringList& rowTexts, QPixmap* pItemIcon, int itemAlignment)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(rowTexts.size() == columnCount())
    {
        retVal = true;

        // Display more then one column if we have more then one:
        if (columnCount() != 1)
        {
            m_displayOnlyFirstColumn = false;
        }

        int row = rowCount();
        insertRow(row);

        for (int i = 0; i < (int)rowTexts.size(); i++)
        {
            const QString& text = rowTexts[i];

            // Initialize a table widget item:
            QTableWidgetItem* pItem = allocateNewWidgetItem(text);

            // Set the first column attributes:
            if (i == 0)
            {
                initItem(*pItem, text, nullptr, false, Qt::Unchecked, pItemIcon);
            }
            else
            {
                initItem(*pItem, text, nullptr);
            }

            // Set the text alignment:
            pItem->setTextAlignment(itemAlignment);

            // Set the item:
            setItem(row, i, pItem);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::addEmptyRows
// Description: Add the new empty rows
// Return Val:  bool - Success / failure.
// Author:      Ehud Katz
// Date:        13/5/2013
// ---------------------------------------------------------------------------
bool acListCtrl::addEmptyRows(int count)
{
    bool retVal = false;
    const int rowsAmount = rowCount();
    const int colsAmount = columnCount();

    if (model()->insertRows(rowsAmount, count))
    {
        retVal = true;

        // Display more then one column if we have more then one:
        if (colsAmount != 1)
        {
            m_displayOnlyFirstColumn = false;
        }

        const QString text = tr("");
        count += rowsAmount;

        for (int i = rowsAmount; i < count; i++)
        {
            for (int j = 0; j < colsAmount; j++)
            {
                QTableWidgetItem* pItem = allocateNewWidgetItem(text);

                // Set the item:
                setItem(i, j, pItem);
            }
        }
    }

    return retVal;
}
bool acListCtrl::addEmptyRows(int position, int count)
{
    bool retVal = false;
    const int colsAmount = columnCount();

    if (model()->insertRows(position, count))
    {
        retVal = true;

        // Display more then one column if we have more then one:
        if (colsAmount != 1)
        {
            m_displayOnlyFirstColumn = false;
        }

        const QString text = tr("");
        count += position;

        for (int i = position; i < count; i++)
        {
            for (int j = 0; j < colsAmount; j++)
            {
                QTableWidgetItem* pItem = allocateNewWidgetItem(text);

                // Set the item:
                setItem(i, j, pItem);
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acListCtrl::getItemData
// Description: Get the item data stored on the item
// Arguments:   int row
// Return Val:  void*
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
void* acListCtrl::getItemData(int row)
{
    void* pRetVal = nullptr;

    // Item data is stored only on column 0:
    QTableWidgetItem* pRowItem = item(row, 0);

    if (pRowItem != nullptr)
    {
        // Get the item data:
        QVariant itemData = pRowItem->data(Qt::UserRole);
        pRetVal = itemData.value<void*>();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::getItemData
// Description: Returns data via the retVal parameter as a QVariant
// Arguments:   int row
//              QVariant& retVal
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void acListCtrl::getItemData(int row, QVariant& retVal)
{
    // Item data is stored only on column 0:
    QTableWidgetItem* pRowItem = item(row, 0);

    {
        // Get the item data:
        retVal = pRowItem->data(Qt::UserRole);
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::clearBoldItems
// Description: Clears all bold highlights from the view
// Author:      Uri Shomroni
// Date:        17/11/2013
// ---------------------------------------------------------------------------
void acListCtrl::clearBoldItems()
{
    // Clear all the currently highlighted text:
    int rowsAmount = rowCount();
    int colsAmount = columnCount();

    for (int i = 0; i < rowsAmount; i++)
    {
        for (int j = 0; j < colsAmount; j++)
        {
            // Get the item in the current row / col:
            QTableWidgetItem* pItem = item(i, j);

            if (pItem != nullptr)
            {
                // Remove the bold attribute from this item:
                QFont font = pItem->font();
                font.setBold(false);
                pItem->setFont(font);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::highlightTextItem
// Description: Highlight the requested text
// Arguments:   gtString text
//              bool isBold
//              QBrush* pForeground
//              QBrush* pBackground
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::highlightTextItem(const gtString& text, bool isBold, bool select, QBrush* pForeground, QBrush* pBackground)
{
    clearBoldItems();

    // Get the item with the same string:
    QList<QTableWidgetItem*> matchingItems;
    FindText(acGTStringToQString(text), Qt::MatchExactly, matchingItems);

    GT_IF_WITH_ASSERT(matchingItems.size() == 1)
    {
        QTableWidgetItem* pItem = matchingItems[0];
        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            // Get the item font:
            if (isBold)
            {
                QFont font = pItem->font();
                font.setBold(true);
                pItem->setFont(font);
            }

            if (pForeground != nullptr)
            {
                // Set the foreground:
                pItem->setForeground(*pForeground);
            }

            if (pBackground != nullptr)
            {
                // Set the background:
                pItem->setBackground(*pBackground);
            }

            // Select the item:
            if (select)
            {
                // Select the item:
                selectRow(pItem->row());

                // Scroll to the selected item:
                scrollToItem(pItem);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::findHighlightedItemText
// Description: Get the text of the currently highlighted text
// Return Val:  QString
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
gtString acListCtrl::findHighlightedItemText()
{
    gtString retVal;
    // Get amount of rows and columns:
    int rowsAmount = rowCount();
    int colAmount = columnCount();

    for (int i = 0; i < rowsAmount; i++)
    {
        for (int j = 0 ; j < colAmount; j++)
        {
            // Get the current item:
            QTableWidgetItem* pCurrentItem = item(i, j);

            if (pCurrentItem != nullptr)
            {
                bool isBold = pCurrentItem->font().bold();

                if (isBold)
                {
                    // This is the item, return it:
                    retVal.fromASCIIString(pCurrentItem->text().toLatin1().data());
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::clearList
// Description: Clears the entire list and removes all its lines
// Author:      Uri Shomroni
// Date:        11/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::clearList()
{
    // Clear all breakpoints from list:
    clearContents();

    int amountOfRows = rowCount();

    for (int i = amountOfRows - 1; i >= 0; i--)
    {
        removeRow(i);
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditCopy
// Description: Check if copy command should be enabled
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onUpdateEditCopy(bool& isEnabled)
{
    isEnabled = (rowCount() >= 1);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onEditCopy
// Description: Handle copy command
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != nullptr)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != nullptr)
        {
            // Get the selected items list:
            QList<QTableWidgetItem*> selectedItemsList = selectedItems();

            QString selectedText;
            QList<QModelIndex> indexesList = selectedIndexes();
            QList<int> rowsCopied;

            if (m_shouldCopyColumnHeaders)
            {
                // Copy column headers:
                QList<int> colHeadersCopied;

                foreach (QModelIndex modelIndex, indexesList)
                {
                    // Get the current item row:
                    int col = modelIndex.column();

                    // If row wad not copied yet:
                    if (colHeadersCopied.indexOf(col) == -1)
                    {
                        // Add to the list of copied columns:
                        colHeadersCopied.append(col);

                        // Add this widget item text:
                        QTableWidgetItem* pWidgetItem = horizontalHeaderItem(col);

                        if (pWidgetItem != nullptr)
                        {
                            // Add the text:
                            selectedText.append(pWidgetItem->text());

                            if ((indexesList.size() != 1) && (col < indexesList.at(indexesList.size() - 1).column()))
                            {
                                selectedText.append(", ");
                            }
                        }
                    }
                }

                // Last column - add new line:
                selectedText.append("\n");
            }

            foreach (QModelIndex modelIndex, indexesList)
            {
                // Get the current item row:
                int row = modelIndex.row();

                // If row wad not copied yet:
                if (rowsCopied.indexOf(row) == -1)
                {
                    // Get the items for this row:
                    int colCount = columnCount();

                    for (int i = 0 ; i < colCount; i++)
                    {
                        // Try to get table item widget:
                        QTableWidgetItem* pWidgetItem = item(row, i);

                        if (pWidgetItem != nullptr)
                        {
                            int selectedItemIndex = selectedItemsList.indexOf(pWidgetItem);

                            if (selectedItemIndex != -1)
                            {
                                // Get current item widget text:
                                QString currentItemWidgetText = pWidgetItem->text();

                                if (currentItemWidgetText.isEmpty())
                                {
                                    if (cellWidget(row, i) != nullptr)
                                    {
                                        // Check if the widget is QLabel
                                        QLabel* pLabel = qobject_cast<QLabel*>(cellWidget(row, i));

                                        if (pLabel != nullptr)
                                        {
                                            // Check if QLabel uses RichText format
                                            if (pLabel->textFormat() == Qt::RichText)
                                            {
                                                QTextDocument doc;
                                                doc.setHtml(pLabel->text());
                                                // Remove html tags from RichText
                                                currentItemWidgetText = doc.toPlainText();
                                            }
                                            else
                                            {
                                                currentItemWidgetText = pLabel->text();
                                            }
                                        }
                                    }
                                }

                                // Remove thousand separator is requested:
                                if (m_removeThousandSeparatorOnCopy)
                                {
                                    currentItemWidgetText.remove(GT_THOUSANDS_SEPARATOR);
                                }

                                // Add the text:
                                selectedText.append(currentItemWidgetText);

                                // Check if there is a postfix to the text:
                                if (i < (int)m_columnsPostfixes.size())
                                {
                                    if (!m_columnsPostfixes[i].isEmpty())
                                    {
                                        selectedText.append(m_columnsPostfixes[i].asCharArray());
                                    }
                                }

                                if (indexesList.size() != 1)
                                {
                                    if ((i == colCount - 1) || (i == indexesList.at(indexesList.size() - 1).column()))
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
                    }

                    rowsCopied.append(row);
                }
            }

            // Set the copied text to the clipboard:
            pClipboard->setText(selectedText);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditPaste
// Description: Check if paste command should be enabled
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onUpdateEditPaste(bool& isEnabled, bool& isVisible)
{
    isVisible = m_isPasteActionEnabled;
    isEnabled = m_isPasteActionEnabled;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditFind
// Description: Check if find command should be enabled
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onUpdateEditFind(bool& isEnabled)
{
    isEnabled = (rowCount() > 1);
}

void acListCtrl::onFindClick()
{
    // Clear the find index:
    if (acFindParameters::Instance().m_findFirstLine >= 0)
    {
        if (acFindParameters::Instance().m_isSearchUp)
        {
            acFindParameters::Instance().m_findFirstLine--;

            if (acFindParameters::Instance().m_findFirstLine < 0)
            {
                acFindParameters::Instance().m_findFirstLine = rowCount() - 1;
            }
        }
        else
        {
            acFindParameters::Instance().m_findFirstLine++;

            if (acFindParameters::Instance().m_findFirstLine >= rowCount())
            {
                acFindParameters::Instance().m_findFirstLine = 0;
            }
        }
    }

    if (!acFindParameters::Instance().m_findExpr.isEmpty())
    {
        // Define the Qt find flags matching the user selection:
        Qt::MatchFlags findFlags = Qt::MatchContains;

        if (acFindParameters::Instance().m_isCaseSensitive)
        {
            findFlags = findFlags | Qt::MatchCaseSensitive;
        }

        findAndSelectNext(findFlags);
    }

    // After results are updated, ask the find widget to update the UI:
    acFindWidget::Instance().UpdateUI();
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
bool acListCtrl::findAndSelectNext(Qt::MatchFlags findFlags)
{
    bool retVal = false;

    // Get the item with the same string:
    int foundInRow = -1;
    QList<QTableWidgetItem*> matchingItems;
    FindText(acFindParameters::Instance().m_findExpr, findFlags, matchingItems);

    if (acFindParameters::Instance().m_isSearchUp)
    {
        // Search from the end:
        QListIterator<QTableWidgetItem*> iter(matchingItems);
        iter.toBack();

        while (iter.hasPrevious())
        {
            // Go back:
            QTableWidgetItem* pPreviousItem = iter.peekPrevious();

            if (pPreviousItem != nullptr)
            {
                int previousRow = pPreviousItem->row();
                iter.previous();

                if (previousRow <= acFindParameters::Instance().m_findFirstLine)
                {
                    foundInRow = previousRow;
                    break;
                }
            }
        }

        if (foundInRow < 0)
        {
            // Take the last found item:
            QListIterator<QTableWidgetItem*> iter1(matchingItems);
            iter1.toBack();

            if (iter1.hasPrevious())
            {
                QTableWidgetItem* pPreviousItem = iter1.previous();

                if (pPreviousItem != nullptr)
                {
                    foundInRow = pPreviousItem->row();
                }
            }
        }
    }
    else
    {
        foreach (QTableWidgetItem* pItem, matchingItems)
        {
            // Check if the row is in forward to last one found:
            if (pItem != nullptr)
            {
                if (pItem->row() >= acFindParameters::Instance().m_findFirstLine)
                {
                    foundInRow = pItem->row();
                    break;
                }
            }
        }

        if (foundInRow < 0)
        {
            // Take the first found item:
            if (!matchingItems.isEmpty())
            {
                QTableWidgetItem* pFirstItem = matchingItems.first();

                if (pFirstItem != nullptr)
                {
                    foundInRow = pFirstItem->row();
                }
            }
        }
    }

    // Set the find result in the find parameters class:
    acFindParameters::Instance().m_lastResult = (foundInRow >= 0);

    // Select the result's line:
    if (foundInRow >= 0)
    {
        // Set the find index:
        acFindParameters::Instance().m_findFirstLine = foundInRow;

        // Clear selection:
        clearSelection();

        // Set the selection item:
        selectRow(acFindParameters::Instance().m_findFirstLine);

        // Focus the list control:
        setFocus();

        retVal = true;
    }

    // After results are updated, ask the find widget to update the UI:
    acFindWidget::Instance().UpdateUI();

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditFindNext
// Description:  Check if find next command should be enabled
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onUpdateEditFindNext(bool& isEnabled)
{
    isEnabled = (rowCount() > 1);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditSelectAll
// Description:  Check if select all command should be enabled
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onUpdateEditSelectAll(bool& isEnabled)
{
    isEnabled = (rowCount() >= 1);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onEditSelectAll
// Description: Implement select all command
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onEditSelectAll()
{
    selectAll();
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateDeleteSelected
// Description: Check if Delete Selected and enable/disable it accordingly
// Return Val:  void
// Author:      Yoni Rabin
// Date:        16/4/2012
// ---------------------------------------------------------------------------
void acListCtrl::onUpdateDeleteSelected()
{
    bool isEnabled = false;

    if (m_enableRowDeletion)
    {
        QList<QTableWidgetItem*> currentSelectedItems = selectedItems();

        foreach (QTableWidgetItem* pCurrentItem, currentSelectedItems)
        {
            if (nullptr != pCurrentItem)
            {
                // Enable the deletion of non editable items only:
                Qt::ItemFlags flags = pCurrentItem->flags();
                QString text = pCurrentItem->text();
                bool isItemEditable = (flags & Qt::ItemIsEditable);

                bool enableDeletion = !isItemEditable || m_shouldEnableDeletionOfEditableRow;

                if (enableDeletion && !text.isEmpty())
                {
                    isEnabled = true;
                    break;
                }
            }
        }
    }

    m_pDeleteAction->setEnabled(isEnabled);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::keyPressEvent
// Description: Overrides the base class key event
// Arguments:   QKeyEvent *pEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::keyPressEvent(QKeyEvent* pEvent)
{
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        //Emitting the keyPressed signal
        emit keyPressed(pEvent);

        // Check if the key matched delete key:
        if (pEvent->matches(QKeySequence::Delete))
        {
            onDeleteSelected();
        }
        // Check if this is a space:
        else if (pEvent->key() == Qt::Key_Space)
        {
            onItemsCheckedChange();
        }
        // Check if this is a space:
        else if ((pEvent->key() == Qt::Key_A) && (pEvent->modifiers() == Qt::ControlModifier))
        {
            selectAll();
        }
    }

    // Call the base class implementation:
    QTableWidget::keyPressEvent(pEvent);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onContextMenuEvent
// Description: Is connected to the context menu request signal - display the
//              context menu if it is initialized
// Arguments:   const QPoint &
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onContextMenuEvent(const QPoint& position)
{
    if (m_pContextMenu != nullptr)
    {
        m_contextMenuPosition = position;
        m_pContextMenu->exec(acMapToGlobal(viewport(), position));
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::onDeleteSelected
// Description: Deletes the currently selected rows
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onDeleteSelected()
{
    if (m_enableRowDeletion)
    {
        int numberOfRows = rowCount();

        if (numberOfRows > 0)
        {
            // Prepare an array to know which items to delete:
            bool* pShouldDeleteRow = new bool[numberOfRows];
            ::memset(pShouldDeleteRow, 0, sizeof(bool) * numberOfRows);

            // Get the selected rows:
            QList<QTableWidgetItem*> currentSelectedItems = selectedItems();

            foreach (QTableWidgetItem* pCurrentItem, currentSelectedItems)
            {
                if (pCurrentItem != nullptr)
                {
                    pShouldDeleteRow[pCurrentItem->row()] = true;
                }
            }

            // Delete the rows that were selected for deletion:
            for (int i = numberOfRows - 1; i >= 0; i--)
            {
                if (pShouldDeleteRow[i])
                {
                    // Perform before remove row operations:
                    onBeforeRemoveRow(i);

                    // Delete the current row:
                    removeRow(i);

                    // Perform before remove row operations:
                    onAfterRemoveRow(i - 1);
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::onBeforeRemoveRow
// Description: Before an item row deletion
// Arguments:   int row
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onBeforeRemoveRow(int row)
{
    //Emitting the removingRow signal
    emit removingRow(row);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onBeforeRemoveRow
// Description: After an item row deletion
// Arguments:   int row
// Author:      Yoni Rabin
// Date:        1/7/2012
// ---------------------------------------------------------------------------
void acListCtrl::onAfterRemoveRow(int row)
{
    //Emitting the removingRow signal
    emit afterRemovingRow(row);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::initContextMenu
// Description: Initialize the context menu
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::initContextMenu()
{
    // Allocate a menu:
    m_pContextMenu = new QMenu(this);

    // Create copy action:
    m_pCopyAction = new QAction(AC_STR_listCtrlCopy, this);

    // Connect the action to delete slot:
    bool rcConnect = connect(m_pCopyAction, SIGNAL(triggered()), this, SLOT(onEditCopy()));
    GT_ASSERT(rcConnect);

    // Create paste action:
    m_pPasteAction = new QAction(AC_STR_listCtrlPaste, this);

    // Create select all action:
    m_pSelectAllAction = new QAction(AC_STR_listCtrlSelectAll, this);

    // Connect the action to delete slot:
    rcConnect = connect(m_pSelectAllAction, SIGNAL(triggered()), this, SLOT(onEditSelectAll()));
    GT_ASSERT(rcConnect);

    // Connect the menu to its slots:
    rcConnect = connect(m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowContextMenu()));
    GT_ASSERT(rcConnect);

    // Add the actions:
    m_pContextMenu->addAction(m_pCopyAction);
    m_pContextMenu->addAction(m_pPasteAction);
    m_pContextMenu->addAction(m_pSelectAllAction);

    // Add separator above delete action:
    m_pContextMenu->addSeparator();
    m_pDeleteAction = m_pContextMenu->addAction(AC_STR_listCtrlDeleteSelected, this, SLOT(onDeleteSelected()));

    m_pDeleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
}


void acListCtrl::onFindNext()
{
    onFindClick();
}


void acListCtrl::onFindPrev()
{
    // m_isSearchUp flag is up only when find prev button is pressed
    acFindParameters::Instance().m_isSearchUp = true;
    onFindNext();
    acFindParameters::Instance().m_isSearchUp = false;
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::onItemClicked
// Description: Handle the item click event
// Arguments:   QTableWidgetItem * pItem
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onItemClicked(QTableWidgetItem* pItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        // Reset the find index:
        acFindParameters::Instance().m_findFirstLine = pItem->row();
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::onAboutToShowContextMenu
// Description: Update actions visibility and enable state
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void acListCtrl::onAboutToShowContextMenu()
{
    // Sanity check:
    bool isEnabled = false, isVisible = false;
    GT_IF_WITH_ASSERT((m_pPasteAction != nullptr) && (m_pCopyAction != nullptr) && (m_pSelectAllAction != nullptr) && (m_pDeleteAction != nullptr))
    {
        // Set the copy action enable state:
        onUpdateEditCopy(isEnabled);
        m_pCopyAction->setEnabled(isEnabled);

        // Set the paste action enable state:
        onUpdateEditPaste(isEnabled, isVisible);
        m_pPasteAction->setEnabled(isEnabled);
        m_pPasteAction->setVisible(isVisible);

        // Set the select all action enable state:
        onUpdateEditSelectAll(isEnabled);
        m_pSelectAllAction->setEnabled(isEnabled);

        // Set the delete action enable state:
        onUpdateDeleteSelected();
        m_pDeleteAction->setVisible(m_enableRowDeletion);
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::setItemText
// Description: Sets an existing item text.
// Arguments:   int row - the item row
//              int col - the item column
//              const QString& text - the requested string
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::setItemText(int row, int col, const QString& text)
{
    bool retVal = false;

    // Get the table widget item:
    QTableWidgetItem* pItem = item(row, col);

    if (pItem != nullptr)
    {
        pItem->setText(text);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::getItemText
// Description: Get the text for item in specified row and column
// Arguments:   int row - the item row
//              int col - the item column
//              QString& text - the item text (output)
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::getItemText(int row, int col, QString& text)
{
    bool retVal = false;

    // Get the table widget item:
    QTableWidgetItem* pItem = item(row, col);

    if (pItem != nullptr)
    {
        text = pItem->text();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::getItemText
// Description: Return the text for certain item
// Arguments:   int row
//              int col
//              gtString& text
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::getItemText(int row, int col, gtString& text)
{
    bool retVal = false;

    // Empty the string:
    text.makeEmpty();

    // Get the table widget item:
    QTableWidgetItem* pItem = item(row, col);

    if (pItem != nullptr)
    {
        text.fromASCIIString(pItem->text().toLatin1());
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::setItemTextColor
// Description: Set an item text color
// Arguments:   int row - the item row
//              int col - the item column
//              const QColor& color - the requested color
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::setItemTextColor(int row, int col, const QColor& color)
{
    bool retVal = false;

    // Get the table widget item:
    QTableWidgetItem* pItem = item(row, col);

    if (pItem != nullptr)
    {
        // Get the item brush:
        QBrush brush = pItem->foreground();

        // Set the brush color:
        brush.setColor(color);

        // Set the item brush:
        pItem->setForeground(brush);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::setItemBackgroundColor
// Description: Set the requested item background color
// Arguments:   int row
//              int col
//              const QColor& bgcolor
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::setItemBackgroundColor(int row, int col, const QColor& bgcolor)
{
    bool retVal = false;

    if (col > 0)
    {
        // Get the table widget item:
        QTableWidgetItem* pItem = item(row, col);

        if (pItem != nullptr)
        {
            // Set the item brush:
            pItem->setBackgroundColor(bgcolor);
            retVal = true;
        }
    }

    // Else color all row
    {
        for (int i = 0, colsAmount = columnCount(); i < colsAmount; i++)
        {
            // Get the table widget item:
            QTableWidgetItem* pItem = item(row, i);

            if (pItem != nullptr)
            {
                // Set the item brush:
                pItem->setBackgroundColor(bgcolor);
                retVal = true;
            }
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::setItemBold
// Description: Set an item text as bold
// Arguments:   int row - the item row
//              int col - the item column (-1 for the whole row)
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::setItemBold(int row, int col)
{
    bool retVal = false;

    if (col > -1)
    {
        // Get the table widget item:
        QTableWidgetItem* pItem = item(row, col);

        if (pItem != nullptr)
        {
            // Get the current font:
            QFont font = pItem->font();
            font.setBold(true);
            pItem->setFont(font);
            retVal = true;
        }
    }
    else
    {
        // Else color all rows:
        for (int i = 0, colsAmount = columnCount(); i < colsAmount; i++)
        {
            // Get the table widget item:
            QTableWidgetItem* pItem = item(row, i);

            if (pItem != nullptr)
            {
                // Set the item brush:
                QFont font = pItem->font();
                font.setBold(true);
                pItem->setFont(font);
            }

            retVal = true;
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::setItemUnderline
// Description: Set an item text underline
// Arguments:   int row - the item row
//              int col - the item column (-1 for the whole row)
//              bool val - 'true' - adds an underline, 'false' removes it
// Return Val:  bool - Success / failure.
// Author:      Naama Zur
// Date:        29/11/2015
// ---------------------------------------------------------------------------
bool acListCtrl::setItemUnderline(int row, int col, bool val)
{
    bool retVal = false;

    if (col > -1)
    {
        // Get the table widget item:
        QTableWidgetItem* pItem = item(row, col);

        if (pItem != nullptr)
        {
            // Get the current font:
            QFont font = pItem->font();
            font.setUnderline(val);
            pItem->setFont(font);
            retVal = true;
        }
    }
    else
    {
        // Else color all rows:
        for (int i = 0, colsAmount = columnCount(); i < colsAmount; i++)
        {
            // Get the table widget item:
            QTableWidgetItem* pItem = item(row, i);

            if (pItem != nullptr)
            {
                // Set the item brush:
                QFont font = pItem->font();
                font.setUnderline(val);
                pItem->setFont(font);
            }

            retVal = true;
        }
    }


    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acListCtrl::writeListDataToFile
// Description: Writes a data string to a comma separated file
// Arguments:   filePathStr - file path
//              projectFileName, dataDescription - string added to the file header
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
bool acListCtrl::writeListDataToFile(const gtString& filePathStr, const gtString& projectFileName, const gtString& dataDescription) const
{
    bool retVal = true;

    gtString dataStr;

    // Get the data string:
    exportListToString(dataStr);

    gtString fileHeader;
    osTime fileSavedDateAndTime;
    gtString fileSavedDate;
    gtString fileSavedTime;

    osFilePath filePath(filePathStr);
    osFile listDataFile;

    retVal = listDataFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (retVal)
    {
        fileSavedDateAndTime.setFromCurrentTime();

        fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
        fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);

        fileHeader.append(AC_STR_ExportFileHeaderSeperator);
        fileHeader.append(AC_STR_ExportFileHeaderTitlePrefix);
        fileHeader.append(dataDescription);
        fileHeader.append(AC_STR_ExportFileHeaderProjectName);
        fileHeader.append(projectFileName);
        fileHeader.append(AC_STR_NewLine);
        fileHeader.append(AC_STR_ExportFileHeaderGenerationDate) += fileSavedDate;
        fileHeader.append(AC_STR_NewLine);
        fileHeader.append(AC_STR_ExportFileHeaderGenerationTime) += fileSavedTime;
        fileHeader.append(AC_STR_NewLine);
        fileHeader.append(AC_STR_ExportFileHeaderGeneratedBy);
        fileHeader.append(AC_STR_ExportFileHeaderWebSite);
        fileHeader.append(AC_STR_ExportFileHeaderSeperator);
        fileHeader.append(AC_STR_NewLine);

        listDataFile << fileHeader;
        listDataFile << dataStr;
        listDataFile.close();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::exportListToString
// Description: Export the list to a comma separated file string
// Arguments:   gtString& exportedListStr - output
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
void acListCtrl::exportListToString(gtString& exportedListStr) const
{
    // Get the amount of columns:
    int colAmount = columnCount();

    // Get the horizontal header:
    for (int i = 0; i < colAmount; i++)
    {
        QTableWidgetItem* pHeaderItem = horizontalHeaderItem(i);
        GT_IF_WITH_ASSERT(pHeaderItem != nullptr)
        {
            gtString colTitle;
            colTitle.fromASCIIString(pHeaderItem->text().toLatin1());

            // Add comma if not empty:
            if (!colTitle.isEmpty() && (i > 0))
            {
                exportedListStr.append(AC_STR_Comma);
                exportedListStr.append(AC_STR_Space);
            }

            // Append the column text to the string:
            exportedListStr.append(colTitle.asCharArray());
        }
    }

    // Add a new line (after the table header):
    exportedListStr.append(AC_STR_NewLine);

    // Now we create the table body, looping through the list:
    int rowsAmount = rowCount();

    for (int rowIndex = 0 ; rowIndex < rowsAmount; rowIndex++)
    {
        for (int colIndex = 0; colIndex < colAmount; colIndex++)
        {
            // Get the text for the current item:
            QTableWidgetItem* pItem = item(rowIndex, colIndex);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                gtString currentItemStr;
                currentItemStr.fromASCIIString(pItem->text().toLatin1());
                currentItemStr.removeChar(GT_THOUSANDS_SEPARATOR);

                if ((colIndex > 0) && (!currentItemStr.isEmpty()))
                {
                    // Add comma before the item (except for the first one):
                    exportedListStr.append(AC_STR_Comma);
                    exportedListStr.append(AC_STR_Space);
                }

                // Add the item string to the exported string:s
                exportedListStr.append(currentItemStr);
            }
        }

        // Add a line break between the table body rows
        exportedListStr.append(AC_STR_NewLine);
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::resizeEvent
// Description: Is handling the control resize
// Arguments:   QResizeEvent* pResizeEvent
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void acListCtrl::resizeEvent(QResizeEvent* pResizeEvent)
{
    // Ignore resize while we're in manual resize:
    if (!m_ignoreResizeEvents)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pResizeEvent != nullptr)
        {
            // Resize columns:
            int controlWidth = pResizeEvent->size().width();

            if (m_displayOnlyFirstColumn)
            {
                setColumnWidth(0, controlWidth);
            }
            else
            {
                for (int i = 0 ; i < (int)m_columnsWidths.size(); i++)
                {
                    float colWidth = controlWidth * m_columnsWidths[i];
                    setColumnWidth(i, (int)colWidth);
                }
            }
        }
    }

    // Call the base class implementation:
    QTableWidget::resizeEvent(pResizeEvent);

}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::allocateNewWidgetItem
// Description: Used for customized widget item
// Return Val:  QTableWidgetItem*
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
QTableWidgetItem* acListCtrl::allocateNewWidgetItem(const QString& text)
{
    QTableWidgetItem* pRetVal = nullptr;

    pRetVal = new acTableWidgetItem(text);

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::selectRow
// Description: Make sure that a row is visible
// Arguments:   int index
// Author:      Sigal Algranaty
// Date:        26/12/2011
// ---------------------------------------------------------------------------
void acListCtrl::ensureRowVisible(int index, bool select)
{
    // Get the matching widget item:
    QTableWidgetItem* pItem = item(index, 0);

    if (pItem != nullptr)
    {
        // Scroll to item:
        scrollToItem(pItem);
    }

    if (select)
    {
        selectRow(index);
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::enableRowEditing
// Description: Enable/Disable editing of a row
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
void acListCtrl::enableRowEditing(int row, bool enable)
{
    GT_ASSERT(row < rowCount())
    {
        int numColumns = columnCount();

        for (int nColumn = 0 ; nColumn < numColumns ; nColumn++)
        {
            QTableWidgetItem* pCurrentItem = item(row, nColumn);
            Qt::ItemFlags itemFlags = pCurrentItem->flags();

            if (enable)
            {
                itemFlags |= Qt::ItemIsEditable;
            }
            else
            {
                itemFlags &= !Qt::ItemIsEditable;
            }

            pCurrentItem->setFlags(itemFlags);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::amountOfSelectedRows
// Description: Find how many rows are selected
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        23/1/2012
// ---------------------------------------------------------------------------
int acListCtrl::amountOfSelectedRows() const
{
    int retVal = 0;
    QList<int> selectedRows;

    foreach (QModelIndex index, selectedIndexes())
    {
        int existingIndex = selectedRows.indexOf(index.row());

        if (existingIndex == -1)
        {
            selectedRows.push_back(index.row());
        }
    }

    retVal = (int)selectedRows.size();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::setRowHeight
// Description:
// Arguments:   int rowHeight
// Author:      Ehud Katz
// Date:        12/5/2013
// ---------------------------------------------------------------------------
void acListCtrl::setRowHeight(int rowHeight)
{
    m_rowHeight = (int)acScaleSignedPixelSizeToDisplayDPI((unsigned int)rowHeight);
    verticalHeader()->setDefaultSectionSize(m_rowHeight);

    // Set the row heights:
    for (int row = 0, rowsAmount = rowCount(); row < rowsAmount; row++)
    {
        QTableView::setRowHeight(row, m_rowHeight);
    }
}


// ---------------------------------------------------------------------------
// Name:        acListCtrl::setColumnCount
// Description:
// Arguments:   int colCount
// Author:      Sigal Algranaty
// Date:        29/1/2012
// ---------------------------------------------------------------------------
void acListCtrl::setColumnCount(int colCount)
{
    if (colCount == 1)
    {
        m_displayOnlyFirstColumn = true;
    }

    QTableWidget::setColumnCount(colCount);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::setEnablePaste
// Description:
// Arguments:   bool isEnabled
// Return Val:  void
// Author:      Yoni Rabin
// Date:        24/5/2012
// ---------------------------------------------------------------------------
void acListCtrl::setEnablePaste(bool isEnabled)
{
    if (isEnabled != m_isPasteActionEnabled)
    {
        m_isPasteActionEnabled = isEnabled;

        if (m_isPasteActionEnabled)
        {
            // Check if the current control does have an implementation for th e
            // edit paste action:
            // Connect the action to the paste slot:
            bool rc = connect(m_pPasteAction, SIGNAL(triggered()), this, SLOT(onEditPaste()));
            GT_ASSERT(rc);
        }
        else
        {
            disconnect(m_pPasteAction, SIGNAL(triggered()), this, SLOT(onEditPaste()));
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onEditPaste
// Description: Handle onEditPaste command
// Author:      Yoni Rabin
// Date:        16/4/2012
// ---------------------------------------------------------------------------
void acListCtrl::onEditPaste()
{
}

// ---------------------------------------------------------------------------
void acListCtrl::SetSelectionMode(QAbstractItemView::SelectionMode selectMode)
{
    setSelectionMode(selectMode);

    if (selectMode == QAbstractItemView::SingleSelection)
    {
        if ((m_pSelectAllAction != nullptr) && (m_pContextMenu != nullptr))
        {
            m_pContextMenu->removeAction(m_pSelectAllAction);
        }
    }
}

// ---------------------------------------------------------------------------
void acListCtrl::GetSelectedText(gtString& selectedText)
{
    selectedText = findHighlightedItemText();
}

// ---------------------------------------------------------------------------
void acListCtrl::SetRowEnabled(int row, bool enabled)
{
    GT_IF_WITH_ASSERT(row < rowCount())
    {
        int columnsNum = columnCount();

        // set all items of row enabled/disabled
        for (int i = 0; i < columnsNum; i++)
        {
            SetItemEnabled(row, i, enabled);
        }
    }
}

// ---------------------------------------------------------------------------
void acListCtrl::SetItemEnabled(int row, int column, bool enabled)
{
    GT_IF_WITH_ASSERT(row < rowCount() && column < columnCount())
    {
        QTableWidgetItem* pItem = item(row, column);
        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            Qt::ItemFlags itemFlags = pItem->flags();

            if (enabled)
            {
                // set enabled
                itemFlags |= Qt::ItemIsEnabled;
            }
            else
            {
                // set disabled
                itemFlags &= ~Qt::ItemIsEnabled;
            }

            pItem->setFlags(itemFlags);
        }
    }
}

// --------------------------------------------------------------------------
void acListCtrl::addMenuAction(QAction* pAction, bool inBeginning)
{
    GT_IF_WITH_ASSERT(m_pContextMenu != nullptr && pAction != nullptr)
    {
        // Add the action. The connection the slot should be done by who ever called to add this action:
        if (inBeginning)
        {
            // add action first in menu
            QAction* pFirstAction = m_pContextMenu->actions().at(0);
            GT_IF_WITH_ASSERT(NULL != pFirstAction)
            {
                m_pContextMenu->insertAction(pFirstAction, pAction);
            }
        }
        else
        {
            // add action last last in menu
            m_pContextMenu->addAction(pAction);
        }
    }
}

// --------------------------------------------------------------------------
void acListCtrl::addSeparator(bool inBeginning)
{
    GT_IF_WITH_ASSERT(m_pContextMenu != nullptr)
    {
        QAction* pFirstAction = m_pContextMenu->actions().at(0);

        if (inBeginning)
        {
            // add separator first in menu
            GT_IF_WITH_ASSERT(NULL != pFirstAction)
            {
                m_pContextMenu->insertSeparator(pFirstAction);
            }
        }
        else
        {
            // add separator last in menu
            m_pContextMenu->addSeparator();
        }
    }
}

void acListCtrl::OnHeaderContextMenu(const QPoint& position)
{
    QMenu headerContextMenu;

    for (int i = 0; i < horizontalHeader()->count(); i++)
    {
        QTableWidgetItem* pHeaderItem = horizontalHeaderItem(i);
        GT_IF_WITH_ASSERT(pHeaderItem != nullptr)
        {
            QAction* pAction = new QAction(&headerContextMenu);
            pAction->setCheckable(true);
            pAction->setChecked(!horizontalHeader()->isSectionHidden(i));
            pAction->setText(pHeaderItem->text());
            headerContextMenu.addAction(pAction);
            pAction->setEnabled(i != 0);

            // Set the column index as variant on the column:
            pAction->setData(QVariant(i));

            // Connect the action to the slot:
            bool rc = connect(pAction, SIGNAL(triggered(bool)), this, SLOT(OnHeaderItemShow(bool)));
            GT_ASSERT(rc);
        }
    }

    // Show the menu:
    headerContextMenu.exec(acMapToGlobal(viewport(), position));
}

void acListCtrl::OnHeaderItemShow(bool show)
{
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT((pAction != nullptr) && (horizontalHeader() != nullptr))
    {
        int colIndex = pAction->data().toInt();

        if (show)
        {
            horizontalHeader()->showSection(colIndex);
        }
        else
        {
            horizontalHeader()->hideSection(colIndex);
        }

        for (int i = 0; i < horizontalHeader()->count(); i++)
        {
            resizeColumnToContents(i);
        }
    }
}

void acListCtrl::SetCellWidget(int row, int column, QWidget* pWidget)
{
    // Set the widget:
    QTableWidget::setCellWidget(row, column, pWidget);

    if (!m_columnsWithWidgets.contains(column))
    {
        m_columnsWithWidgets << column;
    }
}

void acListCtrl::FindText(const QString& text, Qt::MatchFlags findFlags, QList<QTableWidgetItem*>& matchingItems)
{
    // Find the items in the regular text items:
    matchingItems = findItems(text, findFlags);

    // Look for the items in the widgets cells:
    for (int col = 0; col < m_columnsWithWidgets.size(); col++)
    {
        for (int row = 0; row < rowCount(); row++)
        {
            // Get the widget and try to cast it to a label:
            if (cellWidget(row, col) != nullptr)
            {
                QLabel* pLabel = qobject_cast<QLabel*>(cellWidget(row, col));
                GT_IF_WITH_ASSERT(pLabel != nullptr)
                {
                    // Convert the text:
                    if (pLabel->textFormat() == Qt::RichText)
                    {
                        QTextDocument doc;
                        doc.setHtml(pLabel->text());

                        if (doc.toPlainText().contains(text, Qt::CaseSensitive))
                        {
                            matchingItems << item(row, col);
                        }
                    }
                }
            }
        }
    }
}

void acListCtrl::SetSelectionBackgroundColor(const QColor& color)
{
    setStyleSheet(QString("QTableWidget::item:selected::!active{ background-color: %1;}").arg(color.name()));
}
