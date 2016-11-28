//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SourceViewTreeItem.h
///
//==================================================================================

#ifndef __SOURCEVIEWTREEITEM_H
#define __SOURCEVIEWTREEITEM_H

// Qt:
#include <QtWidgets>
#include <QVariant>
#include <QVector>


enum SOURCE_TREE_ITEM_DEPTH
{
    SOURCE_VIEW_DEFAULT_DEPTH = 0,
    SOURCE_VIEW_LINE_DEPTH = 0,
    SOURCE_VIEW_ASM_DEPTH
};

enum SOURCE_VIEW_COLUMNS
{
    SOURCE_VIEW_LINE_COLUMN = 0,
    SOURCE_VIEW_ADDRESS_COLUMN,
    SOURCE_VIEW_SOURCE_COLUMN,
    SOURCE_VIEW_CODE_BYTES_COLUMN,
    SOURCE_VIEW_LAST_FIXED_COLUMN = SOURCE_VIEW_CODE_BYTES_COLUMN,
    SOURCE_VIEW_SAMPLES_COLUMN,
    SOURCE_VIEW_SAMPLES_PERCENT_COLUMN,
    SOURCE_VIEW_INVALID
};

class SourceViewTreeItem
{
public:
    // Constructors:
    SourceViewTreeItem(SOURCE_TREE_ITEM_DEPTH dep, SourceViewTreeItem* pParentItem);
    SourceViewTreeItem(const QVector<QVariant>& data, SourceViewTreeItem* pParent = nullptr);
    virtual ~SourceViewTreeItem();

    SourceViewTreeItem* parent();

    SourceViewTreeItem* child(int index);
    int indexOfChild(SourceViewTreeItem* pItem);
    int childCount() const { return m_childItems.size(); };
    bool insertChildren(int position, int count, int columns);
    bool removeChildren(int position, int count);
    bool appendChild(SourceViewTreeItem* pChild);
    bool insertChild(int position, SourceViewTreeItem* pChild);

    QVariant data(int column) const;
    bool setData(int column, const QVariant& value);

    QVariant tooltip(int column) const;
    bool setTooltip(int column, const QVariant& value);

    int columnCount() const;
    bool insertColumns(int position, int columns);
    bool removeColumns(int position, int columns);

    QColor forground(int column) const;
    bool setForeground(int column, const QColor& color);

    void clear();

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    void DebugPrintChildrenList();
#endif

protected:
#ifdef CLU_TAB
    CLUData*     m_pCLUData = nullptr;
#endif

    QList<SourceViewTreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    QVector<QVariant> m_itemTooltip;
    QVector<QColor> m_itemForegrounds;
    SourceViewTreeItem* m_pParentItem = nullptr;
};


#endif //__SOURCEVIEWTREEITEM_H

