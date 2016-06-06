//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acListCtrl.h
///
//==================================================================================

//------------------------------ acListCtrl.h ------------------------------

#ifndef __ACLISTCTRL
#define __ACLISTCTRL

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFindParameters.h>

Q_DECLARE_METATYPE(void*);

// Forward declaration:
class acListCtrl;

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acListCtrl : public QTableWidget
// General Description: Defines a list control with Qt GUI
// Author:              Sigal Algranaty
// Creation Date:       6/9/2011
// ----------------------------------------------------------------------------------
class AC_API acListCtrl : public QTableWidget
{
    Q_OBJECT

public:
    /// Constructor:
    acListCtrl(QWidget* pParent, int rowHeight = AC_DEFAULT_LINE_HEIGHT, bool isDeleteEnabled = false, bool areItemsSelectable = true);

    /// Destructor:
    virtual ~acListCtrl();

    /// Row height
    void setRowHeight(int rowHeight);
    void setColumnCount(int colCount);
    int amountOfSelectedRows() const;

    /// Item data:
    void* getItemData(int row);
    void getItemData(int row, QVariant& retVal);

    /// Set / Get item text:
    bool setItemText(int row, int col, const QString& text);
    bool getItemText(int row, int col, QString& text);
    bool getItemText(int row, int col, gtString& text);

    /// Set item text properties:
    bool setItemTextColor(int row, int col, const QColor& color);
    bool setItemBackgroundColor(int row, int col, const QColor& bgcolor);
    bool setItemBold(int row, int col = -1);
    bool setItemUnderline(int row, int col = -1, bool val = true);

    /// Enable / Disable row deletion:
    void setEnableRowDeletion(bool isEnabled) {m_enableRowDeletion = isEnabled;};
    void setEnablePaste(bool isEnabled);

    /// Initialize the list columns:
    void initHeaders(const QStringList& columnCaptions, bool showVerticalHeaders);
    void initHeaders(const QStringList& columnCaptions, const gtVector<float>& columnWidths, bool showVerticalHeaders);
    bool addRow(const QStringList& rowTexts, void* pItemData, bool withCheckBox = false, Qt::CheckState checkState = Qt::Unchecked, QPixmap* pItemIcon = NULL, bool beforeLastRow = false);
    bool addRow(const QStringList& rowTexts, QPixmap* pItemIcon, int itemAlignment);
    bool addRow(const QString& text, void* pItemData, bool withCheckBox = false, Qt::CheckState checkState = Qt::Unchecked, QPixmap* pItemIcon = NULL, bool beforeLastRow = false);
    bool addRow(const QString& text, const QVariant& itemData);
    bool addRow(const QString& text);
    /// Add text and allow it to expand to more then a line if needed
    bool addMultiLineRow(const QString& text);
    bool addEmptyRows(int count);
    bool addEmptyRows(int position, int count);
    void initItem(QTableWidgetItem& witem, const QString& text, const QVariant* pItemData, bool withCheckBox = false, Qt::CheckState checkState = Qt::Unchecked, QPixmap* pItemIcon = NULL);
    QTableWidgetItem* initItem(int row, int column, const QString& text, const QVariant* pItemData, bool withCheckBox = false, Qt::CheckState checkState = Qt::Unchecked, QPixmap* pItemIcon = NULL);
    void initRowItems(int row, const QStringList& rowTexts, int itemAlignment);
    void clearList();

    /// Write the data to a file:
    bool writeListDataToFile(const gtString& filePathStr, const gtString& projectFileName, const gtString& dataDescription) const;
    void ensureRowVisible(int index, bool select);

    /// make sure a row is enabled for editing or not:
    void enableRowEditing(int row, bool enable);

    /// Let the user know if the control's text is currently being edited:
    bool isBeingEdited() {return state() == QAbstractItemView::EditingState;}

    /// Set selection mode:
    void SetSelectionMode(QAbstractItemView::SelectionMode selectMode);

    /// set the row items enabled/disabled
    /// \param row is the row number
    /// \param enabled if true-enable, false-disable
    void SetRowEnabled(int row, bool enabled);

    /// set item in table enabled/disabled
    /// \param row is the row number
    /// \param column is the column number
    /// \param enabled if true-enable, false-disable
    void SetItemEnabled(int row, int column, bool enabled);

    /// Add an action to the menu that will be handled by an outside handler:
    /// \param pAction to be added to the menu
    /// \param inBeggining if true the action will be added in the menu beginning
    void addMenuAction(QAction* pAction, bool inBeginning = true);

    /// addSeparator at the beginning of the menu:
    /// \param inBeggining if true the separator will be added in the menu beginning
    void addSeparator(bool inBeginning = true);

    /// acListCtrl class should know when a widget is set for a column, since the find functionality
    /// is implemented different in this case
    /// \param row the item row
    /// \param column the item column
    /// \param pWidget the widget for the cell
    void SetCellWidget(int row, int column, QWidget* pWidget);

    /// set the color of the selection background (does not affect focus)
    void SetSelectionBackgroundColor(const QColor& color);

Q_SIGNALS:
    void keyPressed(QKeyEvent* pEvent);
    void removingRow(int row);
    void afterRemovingRow(int row);

public slots:

    virtual void onAboutToShowContextMenu();

    /// Edit commands implementations:
    virtual void onEditPaste();
    /// \param[out] selectedText returns the text currently selected
    virtual void GetSelectedText(gtString& selectedText);

    void onEditCopy();
    void onEditSelectAll();

    void onUpdateEditCopy(bool& isEnabled);
    void onUpdateEditPaste(bool& isEnabled, bool& isVisible);
    void onUpdateEditFind(bool& isEnabled);
    void onUpdateEditFindNext(bool& isEnabled);
    void onUpdateEditSelectAll(bool& isEnabled);
    void onUpdateDeleteSelected();
    void setIgnoreResize(bool ignoreIn) {m_ignoreResizeEvents = ignoreIn;};

    /// Deletion of editable rows:
    void EnableEditableRowDeletion() {m_shouldEnableDeletionOfEditableRow = true;};

    /// Slots implementing the find command. Notice: This slot names cannot be changed, since it is connected in the construction of the main window
    /// Is called when the main window find is clicked:
    void onFindClick();

    /// Is called when the main window find next is clicked:
    void onFindNext();

    /// Is called when the main window find previous is clicked:
    void onFindPrev();

    /// Enables headers copy
    void enableColumnHeaderCopy(bool enableHeaderCopy) { m_shouldCopyColumnHeaders = enableHeaderCopy; }

protected:

    gtString findHighlightedItemText();
    void clearBoldItems();
    void highlightTextItem(const gtString& text, bool isBold, bool select, QBrush* pForeground = NULL, QBrush* pBackground = NULL);
    void onItemClicked(QTableWidgetItem* pItem);
    void exportListToString(gtString& exportedListStr) const;

    /// Override when using inherited widget item (can be used for customized sort for instance):
    virtual QTableWidgetItem* allocateNewWidgetItem(const QString& text);

    void initContextMenu();

    /// QTableWidget overrides:
    void keyPressEvent(QKeyEvent* pEvent);

    /// This function can be override:
    virtual void onBeforeRemoveRow(int row);
    virtual void onAfterRemoveRow(int row);

    bool findAndSelectNext(Qt::MatchFlags findFlags);

    /// Search for a text in columns with widgets. Add the items in which the text is found to matchingItems:
    void FindText(const QString& text, Qt::MatchFlags findFlags, QList<QTableWidgetItem*>& matchingItems);

protected slots:

    /// Context menu slot:
    virtual void onContextMenuEvent(const QPoint& point);

    virtual void onDeleteSelected();
    virtual void onItemsCheckedChange() {};
    virtual void resizeEvent(QResizeEvent* pResizeEvent);


protected slots:

    /// Header context menu slot:
    virtual void OnHeaderContextMenu(const QPoint& point);

    /// Is called when the header context menu items are clicked:
    void OnHeaderItemShow(bool show);


protected:

    /// Row height:
    int m_rowHeight;

    /// Contain the relative column width (all numbers are less then 1):
    /// Also, if the user wants to display only the first column, use m_displayOnlyFirstColumn:
    gtVector<float> m_columnsWidths;
    bool m_displayOnlyFirstColumn;

    /// Contain a vector of postfixes for the columns - to be used when copying:
    gtVector<gtASCIIString> m_columnsPostfixes;
    bool m_removeThousandSeparatorOnCopy;

    /// Context menu flags:
    bool m_enableRowDeletion;
    bool m_areItemsSelectable;

    /// True iff the copy string should contain header strings:
    bool m_shouldCopyColumnHeaders;

    /// Context menu:
    QMenu* m_pContextMenu;
    QAction* m_pCopyAction;
    QAction* m_pPasteAction;
    QAction* m_pSelectAllAction;
    QAction* m_pDeleteAction;
    QPoint m_contextMenuPosition;
    bool m_isPasteActionEnabled;

    /// Ignore resize events:
    bool m_ignoreResizeEvents;

    /// Deletion of editable rows:
    bool m_shouldEnableDeletionOfEditableRow;

    /// List of columns in which widgets are set:
    QList<int> m_columnsWithWidgets;

};


#endif  // __ACLISTCTRL
