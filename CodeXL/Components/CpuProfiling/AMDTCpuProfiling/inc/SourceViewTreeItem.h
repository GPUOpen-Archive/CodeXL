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

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>

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
class SourceCodeTreeView;
//------------------------------------------------------------------------------

class SourceViewTreeItem
{
public:

    // Constructors:
    SourceViewTreeItem(SOURCE_TREE_ITEM_DEPTH dep, SourceViewTreeItem* pParentItem);
    SourceViewTreeItem(const QVector<QVariant>& data, SourceViewTreeItem* pParent = nullptr);
    virtual ~SourceViewTreeItem();


    SourceViewTreeItem* child(int index);
    SourceViewTreeItem* parent();
    int indexOfChild(SourceViewTreeItem* pItem);
    int childCount() const {return m_childItems.size();};
    int columnCount() const;
    QVariant data(int column) const;
    QVariant tooltip(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool appendChild(SourceViewTreeItem* pChild);

    bool insertChild(int position, SourceViewTreeItem* pChild);
    bool insertColumns(int position, int columns);
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childIndex() const;
    bool setData(int column, const QVariant& value);
    bool setTooltip(int column, const QVariant& value);
    bool setForeground(int column, const QColor& color);
    QColor forground(int column) const ;

    void setAssemblyLine(int asmLine) {m_assmeblyLine = asmLine;};
    int assemblyLine()const {return m_assmeblyLine;};

    void clear();

    SOURCE_TREE_ITEM_DEPTH depth() { return m_depth; };

    unsigned int asmLength() { return m_asmLength; };

    void setAsmLength(unsigned int len) { m_asmLength = len; };

    double m_total;

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    void DebugPrintChildrenList();
#endif

private:

protected:
    SOURCE_TREE_ITEM_DEPTH m_depth;
    unsigned int m_asmLength;
#ifdef CLU_TAB
    CLUData*     m_pCLUData;
#endif

    QList<SourceViewTreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    QVector<QVariant> m_itemTooltip;
    QVector<QColor> m_itemForegrounds;
    SourceViewTreeItem* m_pParentItem;
    int m_assmeblyLine;
};


#endif //__SOURCEVIEWTREEITEM_H

