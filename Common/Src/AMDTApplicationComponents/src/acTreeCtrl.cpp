//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTreeCtrl.cpp
///
//==================================================================================

//------------------------------ acTreeCtrl.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>


#define MAX_PEN_SIZE 3

// ---------------------------------------------------------------------------
// Name:        acTreeCtrl::acTreeCtrl
// Description: Constructor
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
acTreeCtrl::acTreeCtrl(QWidget* pParent,
                       int numberOfColumns,
                       bool addPasteAction,
                       bool addExpandCollapeAllActions) : QTreeWidget(pParent),
    _pContextMenu(nullptr),
    _pCopyAction(nullptr),
    _pPasteAction(nullptr),
    _pSelectAllAction(nullptr),
    _pExpandAllAction(nullptr),
    _pCollapseAllAction(nullptr),
    _isPasteActionEnabled(false),
    m_pathIndicatorEnabled(false),
    m_pathIndicatorLevelShown(false),
    m_pHoverItem(nullptr),
    m_colorPathSelected(acPATH_SELECTED_COLOR),
    m_colorPathHover(acPATH_HOVER_COLOR),
    m_pEditor(nullptr)
{
    // Set the number of Columns:
    setColumnCount(numberOfColumns);

    // Initialize the context menu:
    initContextMenu(addPasteAction, addExpandCollapeAllActions);

    bool rc = connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(onPathChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    GT_ASSERT(rc);

    viewport()->setAttribute(Qt::WA_Hover, true);

    // Set the row heights to be uniform:
    setUniformRowHeights(true);

    // Create the item delegate:
    acTreeItemDelegate* pItemDelegate = new acTreeItemDelegate(this);
    setItemDelegate(pItemDelegate);
}

// ---------------------------------------------------------------------------
// Name:        acTreeCtrl::~acTreeCtrl
// Description: Destructor
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
acTreeCtrl::~acTreeCtrl()
{

}


// ---------------------------------------------------------------------------
// Name:        acTreeCtrl::clearView
// Description: Clears the tree view
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::clearView()
{
    clear();
}

// ---------------------------------------------------------------------------
// Name:        addItem
// Description: Creates a tree item and adds it under the root or under the specified parent
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
QTreeWidgetItem* acTreeCtrl::addItem(const QStringList& rowTexts, void* pItemData, QTreeWidgetItem* pParent, QPixmap* pItemIcon)
{
    QTreeWidgetItem* retVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(rowTexts.size() == columnCount())
    {
        if (pParent != nullptr)
        {
            retVal = new QTreeWidgetItem(pParent, rowTexts);
        }
        else // pParent == nullptr
        {
            retVal = new QTreeWidgetItem(this, rowTexts);
            addTopLevelItem(retVal);
        }

        // Set the item icon:
        if (pItemIcon != nullptr)
        {
            retVal->setIcon(0, *pItemIcon);
        }

        if (pItemData != nullptr)
        {
            QVariant data1 = qVariantFromValue(pItemData);
            retVal->setData(0, Qt::UserRole, data1);
        }
    }

    // If this is a top-level item, adjust its line size (this will trickle down to other tree items:
    if ((nullptr == pParent) && (nullptr != retVal))
    {
        unsigned int scaledLineHeight = acScalePixelSizeToDisplayDPI(AC_DEFAULT_LINE_HEIGHT);

        retVal->setSizeHint(0, QSize(-1, (int)scaledLineHeight));
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acTreeCtrl::insertItem
// Description: Insert a tree item after a specific item
// Return Val:  QTreeWidgetItem*
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
QTreeWidgetItem* acTreeCtrl::insertItem(const QStringList& rowTexts, void* pItemData, int index, QTreeWidgetItem* pParent, QPixmap* pItemIcon)
{
    QTreeWidgetItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(rowTexts.size() == columnCount())
    {
        if (pParent != nullptr)
        {
            pRetVal = new(std::nothrow) QTreeWidgetItem;
            int numString = rowTexts.count();

            for (int nString = 0 ; nString < numString ; nString ++)
            {
                pRetVal->setText(nString, rowTexts[nString]);
            }

            pParent->insertChild(index, pRetVal);
        }
        else // pParent == nullptr
        {
            pRetVal = new(std::nothrow) QTreeWidgetItem(this, rowTexts);
            addTopLevelItem(pRetVal);
        }

        // Set the item icon:
        if (pItemIcon != nullptr)
        {
            pRetVal->setIcon(0, *pItemIcon);
        }

        if (pItemData != nullptr)
        {
            QVariant dataAsVairant = qVariantFromValue(pItemData);
            pRetVal->setData(0, Qt::UserRole, dataAsVairant);
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onAboutToShowContextMenu
// Description: Update actions visibility and enable state
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onAboutToShowContextMenu()
{
    // Sanity check:
    bool isEnabled = false, isVisible = false;
    GT_IF_WITH_ASSERT((_pCopyAction != nullptr) && (_pSelectAllAction != nullptr))
    {
        // Set the copy action enable state:
        onUpdateEditCopy(isEnabled);
        _pCopyAction->setEnabled(isEnabled);

        onUpdateEditSelectAll(isEnabled);
        _pSelectAllAction->setEnabled(isEnabled);

        if (_pPasteAction != nullptr)
        {
            // Set the paste action enable state:
            onUpdateEditPaste(isEnabled, isVisible);
            _pPasteAction->setEnabled(isEnabled);
            _pPasteAction->setVisible(isVisible);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onEditCopy
// Description: Handle copy command
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onEditCopy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != nullptr)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != nullptr)
        {
            // Get the selected items list:
            QList<QTreeWidgetItem*> selectedItemsList = selectedItems();
            QString selectedText;

            if (!selectedItemsList.isEmpty())
            {
                for (int i = 0 ; i < columnCount(); i++)
                {
                    if (!isColumnHidden(i))
                    {
                        selectedText.append(headerItem()->text(i));
                    }

                    if (i == columnCount() - 1)
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

            foreach (QTreeWidgetItem* pCurrentItem, selectedItemsList)
            {
                GT_IF_WITH_ASSERT(pCurrentItem != nullptr)
                {
                    // Get the items for this row:
                    int colCount = pCurrentItem->columnCount();

                    for (int i = 0 ; i < colCount; i++)
                    {
                        // Get the text for the current column:
                        QString currentColText = pCurrentItem->text(i);

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

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onEditSelectAll
// Description: Implement select all command
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onEditSelectAll()
{
    selectAll();
}

void acTreeCtrl::onCollapseAll()
{
    QList<QTreeWidgetItem*> selected = selectedItems();

    if (!selected.isEmpty())
    {
        for (QList<QTreeWidgetItem*>::iterator it = selected.begin(), itEnd = selected.end(); it != itEnd; ++it)
        {
            CollapseAllSubTree(*it);
        }
    }
    else
    {
        collapseAll();
    }
}

void acTreeCtrl::onExpandAll()
{
    QList<QTreeWidgetItem*> selected = selectedItems();

    if (!selected.isEmpty())
    {
        for (QList<QTreeWidgetItem*>::iterator it = selected.begin(), itEnd = selected.end(); it != itEnd; ++it)
        {
            ExpandAllSubTree(*it);
        }
    }
    else
    {
        expandAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditCopy
// Description: Check if copy command should be enabled
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onUpdateEditCopy(bool& isEnabled)
{
    isEnabled = (topLevelItemCount() > 0);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditPaste
// Description: Check if paste command should be enabled
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onUpdateEditPaste(bool& isEnabled, bool& isVisible)
{
    if (_pPasteAction != nullptr)
    {
        if (!_isPasteActionEnabled)
        {
            // Check if the current control does have an implementation for the
            // edit paste action:
            // Connect the action to the paste slot:
            _isPasteActionEnabled = connect(_pPasteAction, SIGNAL(triggered()), this, SLOT(onEditPaste()));
        }

        isVisible = _isPasteActionEnabled;
        isEnabled = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditSelectAll
// Description:  Check if select all command should be enabled
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onUpdateEditSelectAll(bool& isEnabled)
{
    isEnabled = (topLevelItemCount() > 0);
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onContextMenuEvent
// Description: Is connected to the context menu request signal - display the
//              context menu if it is initialized
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::onContextMenuEvent(const QPoint& position)
{
    if (_pContextMenu != nullptr)
    {
        _contextMenuPosition = position;
        _pContextMenu->exec(acMapToGlobal(this, position));
    }
}

// ---------------------------------------------------------------------------
// Name:        acTreeCtrl::initContextMenu
// Description: Initializes the context menu, with or without the paste action as
//              required
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::initContextMenu(bool addPasteAction, bool addExpandCollapeAllActions)
{
    // Allocate a menu:
    _pContextMenu = new QMenu(this);

    // Create copy action:
    _pCopyAction = new QAction(AC_STR_listCtrlCopy, this);

    // Connect the action to delete slot:
    bool rcConnect = connect(_pCopyAction, SIGNAL(triggered()), this, SLOT(onEditCopy()));
    GT_ASSERT(rcConnect);

    if (addPasteAction)
    {
        // Create paste action:
        _pPasteAction = new QAction(AC_STR_listCtrlPaste, this);
    }

    // Create select all action:
    _pSelectAllAction = new QAction(AC_STR_listCtrlSelectAll, this);

    // Connect the action to delete slot:
    rcConnect = connect(_pSelectAllAction, SIGNAL(triggered()), this, SLOT(onEditSelectAll()));
    GT_ASSERT(rcConnect);

    // Connect the menu to its slots:
    rcConnect = connect(_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowContextMenu()));
    GT_ASSERT(rcConnect);

    // Add the actions:
    _pContextMenu->addAction(_pCopyAction);

    if (addPasteAction)
    {
        _pContextMenu->addAction(_pPasteAction);
    }

    _pContextMenu->addAction(_pSelectAllAction);

    if (addExpandCollapeAllActions)
    {
        _pExpandAllAction = new QAction(AC_STR_treeCtrlExpandAll, this);
        rcConnect = connect(_pExpandAllAction, SIGNAL(triggered()), this, SLOT(onExpandAll()));
        GT_ASSERT(rcConnect);

        _pCollapseAllAction = new QAction(AC_STR_treeCtrlCollapseAll, this);
        rcConnect = connect(_pCollapseAllAction, SIGNAL(triggered()), this, SLOT(onCollapseAll()));
        GT_ASSERT(rcConnect);

        _pContextMenu->addSeparator();
        _pContextMenu->addAction(_pExpandAllAction);
        _pContextMenu->addAction(_pCollapseAllAction);
    }

    // Context menu:
    setContextMenuPolicy(Qt::CustomContextMenu);
    rcConnect = connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenuEvent(const QPoint&)));
    GT_ASSERT(rcConnect);
}


// ---------------------------------------------------------------------------
// Name:        acTreeCtrl::closeEditor
// Description: Overrides base class, emits a signal that the editor was closed
// Author:      Sigal Algranaty
// Date:        8/12/2012
// ---------------------------------------------------------------------------
void acTreeCtrl::closeEditor(QWidget* pEditor, QAbstractItemDelegate::EndEditHint hint)
{
    // Used to make sure we do not get the event twice:
    static QWidget* s_pIsInCloseEditor = nullptr;

    // Call base class implementation::
    QTreeWidget::closeEditor(pEditor, hint);

    // if there is still the line editor we opened then the event was reached through an enter key or something like that
    // there is a need for clean up also here to prevent double events
    if (nullptr != m_pEditor)
    {
        disconnect(m_pEditor, SIGNAL(editingFinished()), this, SLOT(closeEditor()));
        m_pEditor = nullptr;
    }

    if (pEditor != nullptr)
    {
        QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(pEditor);
        QString itemText = pLineEdit->text();

        if (nullptr != pLineEdit && s_pIsInCloseEditor != pEditor)
        {
            s_pIsInCloseEditor = pEditor;
            emit onItemCloseEditor(pLineEdit->text());
        }
    }
}

void acTreeCtrl::closeEditor()
{
    // done editing close the editor and disconnect
    if (nullptr != m_pEditor)
    {
        disconnect(m_pEditor, SIGNAL(editingFinished()), this, SLOT(closeEditor()));
        QList<QTreeWidgetItem*> treeSelectedItems = selectedItems();
        QTreeWidgetItem* pItem = nullptr;

        if (!treeSelectedItems.isEmpty())
        {
            pItem = treeSelectedItems.at(0);
        }

        if (nullptr != pItem)
        {
            closePersistentEditor(pItem, 0);
        }

        m_pEditor = nullptr;
    }
}

void acTreeCtrl::saveLineEdit()
{
    // done editing close the editor and disconnect
    if (nullptr != m_pEditor)
    {
        disconnect(m_pEditor, SIGNAL(editingFinished()), this, SLOT(closeEditor()));
        QList<QTreeWidgetItem*> treeSelectedItems = selectedItems();
        QTreeWidgetItem* pItem = nullptr;

        if (!treeSelectedItems.isEmpty())
        {
            pItem = treeSelectedItems.at(0);
        }

        if (nullptr != pItem)
        {
            pItem->setText(0, m_pEditor->text());
        }
    }
}

bool acTreeCtrl::isAncestor(const QTreeWidgetItem* pChild, const QTreeWidgetItem* pParent)
{
    bool retVal = false;

    const QTreeWidgetItem* pCurrentChild = pChild;

    while ((pCurrentChild != nullptr) && (pParent != nullptr))
    {
        if (pCurrentChild->parent() == pParent)
        {
            retVal = true;
            break;
        }

        pCurrentChild = pCurrentChild->parent();
    }

    return retVal;
}

bool acTreeCtrl::isItemSelected(const QModelIndex& index, bool& isFocused)
{
    bool retVal = false;

    QTreeWidgetItem* pItem = getItemFromIndex(index);

    if (pItem != nullptr)
    {
        retVal = pItem->isSelected();
    }

    isFocused = hasFocus();
    return retVal;
}

void acTreeCtrl::CollapseAllSubTree(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        collapseItem(pItem);

        for (int i = 0, childrenCount = pItem->childCount(); i < childrenCount; ++i)
        {
            CollapseAllSubTree(pItem->child(i));
        }
    }
}

void acTreeCtrl::ExpandAllSubTree(QTreeWidgetItem* pItem)
{
    if (nullptr != pItem)
    {
        for (int i = 0, childrenCount = pItem->childCount(); i < childrenCount; ++i)
        {
            ExpandAllSubTree(pItem->child(i));
        }

        expandItem(pItem);
    }
}

void acTreeCtrl::SetAutoExpandOnSingleChild()
{
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(onItemExpanded(QTreeWidgetItem*)));
    m_autoExpandOnSingleChild = true;
}

void acTreeCtrl::UnSetAutoExpandOnSingleChild()
{
    disconnect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(onItemExpanded(QTreeWidgetItem*)));
    m_autoExpandOnSingleChild = false;
}

void acTreeCtrl::onItemExpanded(QTreeWidgetItem* item)
{
    if (m_autoExpandOnSingleChild && item != nullptr && item->childCount() == 1)
    {
        expandItem(item->child(0));
    }
}
void acTreeCtrl::paintEvent(QPaintEvent* event)
{
    if (m_pathIndicatorEnabled)
    {
        QPainter painter(viewport());
        QTreeWidgetItem* pSelectedItem = currentItem();

        painter.setRenderHint(QPainter::Antialiasing, true);
        int selectedLevel = drawPath(&painter, pSelectedItem, m_colorPathSelected, 3);
        int hoverLevel = drawPath(&painter, m_pHoverItem, m_colorPathHover, 2);

        if (m_pathIndicatorLevelShown)
        {
            QFont font = painter.font();
            font.setPointSize(font.pointSize() - 1);
            painter.setFont(font);

            if (0 <= hoverLevel)
            {
                QRect rect = visualRect(indexFromItem(m_pHoverItem));
                painter.setPen(QPen(m_colorPathHover, 3, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
                painter.drawText(0, rect.y() + (rect.height() / 2), rect.x() - ((indentation() * 2) / 3), rect.height(),
                                 Qt::AlignRight | Qt::AlignTop, QString::number(hoverLevel));
            }

            if (pSelectedItem != m_pHoverItem && 0 <= selectedLevel)
            {
                QRect rect = visualRect(indexFromItem(pSelectedItem));
                painter.setPen(QPen(m_colorPathSelected, 3, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
                painter.drawText(0, rect.y() + (rect.height() / 2), rect.x() - ((indentation() * 2) / 3), rect.height(),
                                 Qt::AlignRight | Qt::AlignTop, QString::number(selectedLevel));
            }
        }
    }

    QTreeWidget::paintEvent(event);
}

int acTreeCtrl::drawPath(QPainter* painter, QTreeWidgetItem* pItem, const QColor& color, int lineWidth) const
{
    int level = -1;

    if (nullptr != pItem)
    {
        int indentCenter = indentation() / 2;

        QVector<QPoint> pathPoints;

        QPoint borderPoint;
        bool overlapBorder = false;
        bool overlapBorderPoint = false;
        int rightBorder = header()->sectionViewportPosition(0) + header()->sectionSize(0);

        do
        {
            QTreeWidgetItem* pItemParent = pItem->parent();

            if (nullptr != pItemParent && !pItemParent->isExpanded())
            {
                level = -1;
                pathPoints.clear();
                break;
            }

            QModelIndex itemIdx = indexFromItem(pItem);

            if (!itemIdx.isValid())
            {
                break;
            }

            QRect rect = visualRect(itemIdx);

            if (pathPoints.size() % 2 == 0 && !pathPoints.isEmpty())
            {
                QPoint point = pathPoints.back();
                pathPoints.push_back(point);
            }

            QPoint point(rect.x() - indentCenter, rect.y() + (rect.height() / 2));

            if (point.x() > rightBorder)
            {
                overlapBorder = true;
                overlapBorderPoint = true;
                borderPoint = point;
            }
            else
            {
                if (overlapBorderPoint)
                {
                    overlapBorderPoint = false;

                    borderPoint.setY(((rightBorder - point.x()) * (borderPoint.y() - point.y())) / (borderPoint.x() - point.x()) + point.y());
                    borderPoint.setX(rightBorder);
                    pathPoints.push_back(borderPoint);
                }

                pathPoints.push_back(point);
            }

            level++;
            pItem = pItemParent;
        }
        while (nullptr != pItem);

        if (overlapBorder)
        {
            level = -1;
        }

        if (!pathPoints.isEmpty())
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(color, Qt::SolidPattern));

            if (!overlapBorder)
            {
                painter->drawEllipse(pathPoints.front(), lineWidth + 1, lineWidth + 1);
            }

            if (2 <= pathPoints.size())
            {
                painter->setPen(QPen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
                painter->drawLines(pathPoints);
            }
        }
    }

    return level;
}

bool acTreeCtrl::viewportEvent(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::HoverMove:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
            if (m_pathIndicatorEnabled)
            {
                QTreeWidgetItem* pOldItem = m_pHoverItem;
                m_pHoverItem = itemAt(static_cast<QHoverEvent*>(event)->pos());

                if (pOldItem != m_pHoverItem)
                {
                    onPathChanged(m_pHoverItem, pOldItem);
                }
            }

            break;

        default:
            break;
    }

    return QTreeWidget::viewportEvent(event);
}

int acTreeCtrl::CalcItemLevel(const QTreeWidgetItem* pItem)
{
    int level = -1;

    while (nullptr != pItem)
    {
        level++;
        pItem = pItem->parent();
    }

    return level;
}

const QTreeWidgetItem* acTreeCtrl::FindLowestCommonAncestor(const QTreeWidgetItem* pItem1, const QTreeWidgetItem* pItem2,
                                                            const QTreeWidgetItem** ppDeepsetItem) const
{
    const QTreeWidgetItem* pCommonAncestor = nullptr;

    if (nullptr != pItem1 && nullptr != pItem2)
    {
        // First check the most common case of following items.
        if (pItem1->parent() == pItem2)
        {
            pCommonAncestor = pItem2;

            if (nullptr != ppDeepsetItem)
            {
                *ppDeepsetItem = pItem1;
            }
        }
        else if (pItem2->parent() == pItem1)
        {
            pCommonAncestor = pItem1;

            if (nullptr != ppDeepsetItem)
            {
                *ppDeepsetItem = pItem2;
            }
        }
        else
        {
            int level1 = CalcItemLevel(pItem1);
            int level2 = CalcItemLevel(pItem2);

            if (level2 < level1)
            {
                const QTreeWidgetItem* pItemTmp = pItem2;
                pItem2 = pItem1;
                pItem1 = pItemTmp;

                int levelTmp = level2;
                level2 = level1;
                level1 = levelTmp;
            }

            if (nullptr != ppDeepsetItem)
            {
                *ppDeepsetItem = pItem2;
            }

            while (level1 < level2)
            {
                if (pItem2 == pItem1)
                {
                    pCommonAncestor = pItem1;
                    break;
                }

                level2--;
                pItem2 = pItem2->parent();
            }

            if (nullptr == pCommonAncestor)
            {
                while (0 <= level1)
                {
                    if (pItem2 == pItem1)
                    {
                        pCommonAncestor = pItem1;
                        break;
                    }

                    level1--;
                    pItem1 = pItem1->parent();
                    pItem2 = pItem2->parent();
                }
            }
        }
    }

    return pCommonAncestor;
}

static void AdjustLevelVisualRect(const QRect& itemRect, QRect& levelRect)
{
    levelRect.setLeft(0);
    levelRect.setRight(itemRect.x() + MAX_PEN_SIZE + 1);
    levelRect.setTop(itemRect.y() - (itemRect.height() / 2) - MAX_PEN_SIZE);
    levelRect.setBottom(itemRect.y() + (itemRect.height() / 2) + itemRect.height() + MAX_PEN_SIZE + 1);
}

static void AdjustPathVisualRect(const QRect& itemRect, QRect& pathRect, int indent)
{
    pathRect.setRight(itemRect.x() + (indent / 2) + MAX_PEN_SIZE + 1);
    pathRect.setBottom(itemRect.y() + (itemRect.height() / 2) + MAX_PEN_SIZE + 1);
}

void acTreeCtrl::onPathChanged(QTreeWidgetItem* pCurrent, QTreeWidgetItem* pPrevious)
{
    if (m_pathIndicatorEnabled)
    {
        QRect changeRect;
        const QTreeWidgetItem* pAncestor = FindLowestCommonAncestor(pCurrent, pPrevious);
        QModelIndex ancestorIdx = indexFromItem(const_cast<QTreeWidgetItem*>(pAncestor));

        if (ancestorIdx.isValid())
        {
            QRect ancestorRect = visualRect(ancestorIdx);
            changeRect.setLeft(ancestorRect.x() - (indentation() / 2) - MAX_PEN_SIZE);
            changeRect.setTop(ancestorRect.y() - (ancestorRect.height() / 2) - MAX_PEN_SIZE);
        }

        QModelIndex previousIdx = indexFromItem(pPrevious);

        if (previousIdx.isValid())
        {
            QRect previousRect = visualRect(previousIdx);
            QRect levelRect;
            AdjustLevelVisualRect(previousRect, levelRect);
            viewport()->update(levelRect);

            AdjustPathVisualRect(previousRect, changeRect, indentation());
            viewport()->update(changeRect);
        }

        QModelIndex currentIdx = indexFromItem(pCurrent);

        if (currentIdx.isValid())
        {
            QRect currentRect = visualRect(currentIdx);
            QRect levelRect;
            AdjustLevelVisualRect(currentRect, levelRect);
            viewport()->update(levelRect);

            AdjustPathVisualRect(currentRect, changeRect, indentation());
            viewport()->update(changeRect);
        }
    }
}

void acTreeCtrl::SetPathIndicator(bool enable)
{
    if (enable != m_pathIndicatorEnabled)
    {
        m_pathIndicatorEnabled = enable;

        viewport()->update();
    }
}

void acTreeCtrl::SetPathIndicatorLevel(bool shown)
{
    if (shown != m_pathIndicatorLevelShown)
    {
        m_pathIndicatorLevelShown = shown;

        viewport()->update();
    }
}

void acTreeCtrl::setEditor(QLineEdit* pEditor)
{
    GT_IF_WITH_ASSERT(nullptr != pEditor)
    {
        m_pEditor = pEditor;
        bool rc = connect(m_pEditor, SIGNAL(returnPressed()), this, SLOT(saveLineEdit()));
        GT_ASSERT(rc);
        rc = connect(m_pEditor, SIGNAL(editingFinished()), this, SLOT(closeEditor()));
        GT_ASSERT(rc);
    }
}

QWidget* acTreeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    acTreeCtrl* pTreeParent = qobject_cast<acTreeCtrl*>(this->parent());

    QWidget* pEditor = QItemDelegate::createEditor(parent, option, index);

    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(pEditor);

    if (nullptr != pTreeParent && nullptr != lineEdit)
    {
        pTreeParent->setEditor(lineEdit);

        // get the selected item so the size of the label can be set correctly:
        QList<QTreeWidgetItem*> treeSelectedItems = pTreeParent->selectedItems();
        QTreeWidgetItem* pItem = nullptr;

        if (!treeSelectedItems.isEmpty())
        {
            pItem = treeSelectedItems.at(0);
        }

        if (nullptr != pItem)
        {
            int itemWidth = pTreeParent->fontMetrics().width(pItem->text(0));
            lineEdit->setMaximumWidth(itemWidth + 10);
        }
    }

    return pEditor;
}

void acTreeCtrl::onEditFind()
{
    onEditFindNext();
}

void acTreeCtrl::onEditFindNext()
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
            setCurrentIndex(findIndex);
            acFindParameters::Instance().m_lastResult = findIndex != m_lastFindIndex;
        }
        else
        {
            acFindParameters::Instance().m_lastResult = false;
            setCurrentIndex(m_lastFindIndex);
        }
    }

    // Update the toolbar UI:
    acFindWidget::Instance().UpdateUI();
}

QModelIndex acTreeCtrl::FindNextText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity)
{
    QModelIndex retval; // empty index

    int rowCount = model->rowCount(parent);
    int colCount = model->columnCount(parent);

    for (int i = 0; i < rowCount; i++)
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

    return retval;
}

QModelIndex acTreeCtrl::FindPrevText(QAbstractItemModel* model, const QModelIndex& parent, bool& pastLastResult, QString text, Qt::CaseSensitivity caseSensitivity)
{
    QModelIndex retval; // empty index

    int rowCount = model->rowCount(parent);
    int colCount = model->columnCount(parent);

    for (int i = rowCount - 1; i >= 0; i--)
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

    return retval;
}

QTreeWidgetItem* acTreeCtrl::FindChild(const QTreeWidgetItem* pParent, QString& searchNodeText)
{
    QTreeWidgetItem* pRetChild = nullptr;

    GT_IF_WITH_ASSERT(pParent != nullptr)
    {
        int numChildren = pParent->childCount();

        for (int nChild = 0; nChild < numChildren; nChild++)
        {
            QTreeWidgetItem* pCurrentChild = pParent->child(nChild);
            GT_IF_WITH_ASSERT(pCurrentChild != nullptr)
            {
                if (pCurrentChild->text(0) == searchNodeText)
                {
                    pRetChild = pCurrentChild;
                    break;
                }
            }
        }
    }
    return pRetChild;
}

QTreeWidgetItem* acTreeCtrl::FindDescendant(const QTreeWidgetItem* pParent, const QString& searchNodeText) const
{
    QTreeWidgetItem* pRetChild = nullptr;

    GT_IF_WITH_ASSERT(pParent != nullptr)
    {
        int numChildren = pParent->childCount();

        for (int nChild = 0; nChild < numChildren && pRetChild == nullptr; ++nChild)
        {
            QTreeWidgetItem* pCurrentChild = pParent->child(nChild);
            GT_IF_WITH_ASSERT(pCurrentChild != nullptr)
            {
                if (pCurrentChild->text(0) == searchNodeText)
                {
                    pRetChild = pCurrentChild;
                    break;
                }
            }
            pRetChild = FindDescendant(pCurrentChild, searchNodeText);
        }
    }
    return pRetChild;
}

void acTreeCtrl::FindDescendants(const QTreeWidgetItem* pParent, const QString& searchNodeText, list<QTreeWidgetItem*>& result) const
{
    GT_IF_WITH_ASSERT(pParent != nullptr)
    {
        int numChildren = pParent->childCount();

        for (int nChild = 0; nChild < numChildren; ++nChild)
        {
            QTreeWidgetItem* pCurrentChild = pParent->child(nChild);
            GT_IF_WITH_ASSERT(pCurrentChild != nullptr)
            {
                if (pCurrentChild->text(0) == searchNodeText)
                {
                    result.push_back(pCurrentChild);
                }
            }
            FindDescendants(pCurrentChild, searchNodeText, result);
        }
    }
}