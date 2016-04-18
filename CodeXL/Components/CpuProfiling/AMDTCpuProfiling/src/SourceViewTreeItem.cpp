//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SourceViewTreeItem.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/SourceViewTreeItem.cpp#8 $
// Last checkin:   $DateTime: 2012/07/06 14:25:46 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 446204 $
//=============================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <inc/SourceViewTreeItem.h>
#include <inc/DisplayFilter.h>

SourceViewTreeItem::SourceViewTreeItem(SessionDisplaySettings* pDisplaySettings, SOURCE_TREE_ITEM_DEPTH dep, SourceViewTreeItem* pParentItem) :
    m_pSessionDisplaySettings(pDisplaySettings), m_depth(dep), m_pParentItem(pParentItem)
{
#ifdef CLU_TAB
    m_pCLUData = nullptr;
#endif
    clear();
    m_depth = dep;
    m_total = 0;

    if (pParentItem != nullptr)
    {
        pParentItem->appendChild(this);
    }
}

SourceViewTreeItem::SourceViewTreeItem(const QVector<QVariant>& data, SourceViewTreeItem* pParent)
{
    m_itemData = data;
    m_pParentItem = pParent;
}

SourceViewTreeItem::~SourceViewTreeItem()
{
#ifdef CLU_TAB

    if (nullptr != m_pCLUData)
    {
        delete m_pCLUData;
    }

    m_pCLUData = nullptr;
#endif
}


void SourceViewTreeItem::clear()
{
    m_depth = SOURCE_VIEW_DEFAULT_DEPTH;
    m_asmLength = 0;
#ifdef CLU_TAB

    if (nullptr != m_pCLUData)
    {
        delete m_pCLUData;
    }

    m_pCLUData = nullptr;
#endif
}

SourceViewTreeItem* SourceViewTreeItem::child(int index)
{
    SourceViewTreeItem* pRetVal = nullptr;

    if ((index >= 0) && (index < (int)m_childItems.size()))
    {
        pRetVal = m_childItems[index];
    }

    return pRetVal;
}

int SourceViewTreeItem::childIndex() const
{
    int retVal = -1;

    if (m_pParentItem != nullptr)
    {
        for (int i = 0 ; i < (int)m_childItems.size(); i++)
        {
            if (m_childItems[i] == this)
            {
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}

int SourceViewTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant SourceViewTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

QVariant SourceViewTreeItem::tooltip(int column) const
{
    return m_itemTooltip.value(column);
}

bool SourceViewTreeItem::appendChild(SourceViewTreeItem* pChild)
{
    bool retVal = true;

    // Insert the child in the last place:
    int numberOfChildren = m_childItems.size();
    m_childItems.insert(numberOfChildren, pChild);

    return retVal;
}


bool SourceViewTreeItem::insertChild(int position, SourceViewTreeItem* pChild)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((position >= 0) && (position < m_childItems.size()))
    {
        m_childItems.insert(position, pChild);
        retVal = true;
    }
    return retVal;
}


bool SourceViewTreeItem::insertChildren(int position, int count, int columns)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((position >= 0) && (position < (int)m_childItems.size()))
    {
        retVal = true;

        // Insert items to the list:
        for (int row = 0; row < count; ++row)
        {
            QVector<QVariant> data(columns);
            SourceViewTreeItem* pItem = new SourceViewTreeItem(data, this);


            // Insert the new child:
            m_childItems.insert(position, pItem);
        }
    }

    return retVal;
}

bool SourceViewTreeItem::insertColumns(int position, int columns)
{
    bool retVal = false;

    if (position < 0 || position > (int)m_itemData.size())
    {
        retVal = false;
    }
    else
    {
        for (int column = 0; column < columns; ++column)
        {
            m_itemData.insert(position, QVariant());
        }

        for (int i = 0 ; i < (int)m_childItems.size(); i++)
        {
            SourceViewTreeItem* pChild = m_childItems[i];
            GT_IF_WITH_ASSERT(pChild != nullptr)
            {
                pChild->insertColumns(position, columns);
            }
        }
    }

    return retVal;
}

SourceViewTreeItem* SourceViewTreeItem::parent()
{
    return m_pParentItem;
}

bool SourceViewTreeItem::removeChildren(int position, int count)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((position >= 0) && ((position + count - 1) < (int)m_childItems.size()))
    {
        for (int row = position + count - 1; row >= position; row--)
        {
            // Remove the item from the list, and delete it:
            delete m_childItems.takeAt(row);
        }

        retVal = true;
    }

    return retVal;

}

bool SourceViewTreeItem::removeColumns(int position, int columns)
{
    bool retVal = false;

    if (position < 0 || (position + columns) > m_itemData.size())
    {
        retVal = false;
    }
    else
    {
        for (int column = 0; column < columns; ++column)
        {
            m_itemData.remove(position);
        }

        foreach (SourceViewTreeItem* pChild, m_childItems)
        {
            pChild->removeColumns(position, columns);
        }
    }

    return retVal;
}

bool SourceViewTreeItem::setData(int column, const QVariant& value)
{
    bool retVal = false;

    if (column >= m_itemData.size())
    {
        m_itemData.resize(column + 1);
    }

    GT_IF_WITH_ASSERT((column >= 0) && (column < (int)m_itemData.size()))
    {
        m_itemData[column] = value;
        retVal = true;
    }

    return retVal;

}

bool SourceViewTreeItem::setTooltip(int column, const QVariant& value)
{
    bool retVal = false;

    if (column >= m_itemTooltip.size())
    {
        m_itemTooltip.resize(column + 1);
    }

    GT_IF_WITH_ASSERT((column >= 0) && (column < (int)m_itemTooltip.size()))
    {
        m_itemTooltip[column] = value;
        retVal = true;
    }

    return retVal;

}

bool SourceViewTreeItem::setForeground(int column, const QColor& color)
{
    bool retVal = false;

    if (column >= m_itemForegrounds.size())
    {
        for (int i = m_itemForegrounds.size(); i <= column; i++)
        {
            m_itemForegrounds << Qt::black;
        }
    }

    GT_IF_WITH_ASSERT((column >= 0) && (column < (int)m_itemForegrounds.size()))
    {
        m_itemForegrounds[column] = color;
    }

    return retVal;
}

int SourceViewTreeItem::indexOfChild(SourceViewTreeItem* pItem)
{
    int retVal = m_childItems.indexOf(pItem);

    return retVal;
}

QColor SourceViewTreeItem::forground(int column) const
{
    QColor retVal = Qt::black;

    if ((column >= 0) && (column < (int)m_itemForegrounds.size()))
    {
        retVal = m_itemForegrounds[column];
    }

    return retVal;
}

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

void SourceViewTreeItem::DebugPrintChildrenList()
{
    gtString outputStr;

    if (m_pParentItem != nullptr)
    {
        outputStr.append(L"Parent data: ");
        outputStr.append(m_pParentItem->data(SOURCE_VIEW_LINE_COLUMN).toString().toStdWString().c_str());
        outputStr.append(L"\nParent children: \n");

        // Print all the children of the item's parent:
        for (int i = 0 ; i < m_pParentItem->m_childItems.size(); i++)
        {
            SourceViewTreeItem* pChild = m_pParentItem->m_childItems[i];

            if (pChild != nullptr)
            {
                outputStr.appendFormattedString(L"Child %d: %ls\n", i, pChild->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().toStdWString().c_str());
            }

        }
    }

    outputStr.append(L"Children:\n");

    for (int i = 0 ; i < m_childItems.size(); i++)
    {
        SourceViewTreeItem* pChild = m_childItems[i];

        if (pChild != nullptr)
        {
            QString dataStr = pChild->data(SOURCE_VIEW_ADDRESS_COLUMN).toString();

            if (dataStr.isEmpty())
            {
                dataStr = pChild->data(SOURCE_VIEW_SOURCE_COLUMN).toString();
            }

            outputStr.appendFormattedString(L"Child %d: %ls\n", i, dataStr.toStdWString().c_str());
        }
    }

    osOutputDebugString(outputStr);

}

#endif

