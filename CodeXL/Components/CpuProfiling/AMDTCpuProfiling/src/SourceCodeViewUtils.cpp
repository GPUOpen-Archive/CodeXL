//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SourceCodeViewUtils.cpp
///
//==================================================================================

/// Local:
#include <inc/SourceCodeViewUtils.h>


SourceLineKey::SourceLineKey(int lineNumber) : m_lineNumber(lineNumber)
{
}

bool SourceLineKey::operator<(const SourceLineKey& other) const
{
    if (m_lineNumber < other.m_lineNumber)
    {
        return true;
    }

    if (m_lineNumber > other.m_lineNumber)
    {
        return false;
    }

    if (m_functionStartAddress < other.m_functionStartAddress)
    {
        return true;
    }

    if (m_functionStartAddress > other.m_functionStartAddress)
    {
        return false;
    }

    if (m_functionName < other.m_functionName)
    {
        return true;
    }

    if (m_functionName > other.m_functionName)
    {
        return false;
    }

    if (m_fileName < other.m_fileName)
    {
        return true;
    }

    return false;
}

bool SourceLineKey::operator==(const SourceLineKey& other) const
{
    bool retVal = true;

    if (m_lineNumber != other.m_lineNumber)
    {
        return false;
    }

    if (m_functionStartAddress != other.m_functionStartAddress)
    {
        return false;
    }

    if (m_functionName != other.m_functionName)
    {
        return false;
    }

    if (m_fileName != other.m_fileName)
    {
        return false;
    }

    return retVal;
}

SourceLineAsmInfo::SourceLineAsmInfo(int lineNumber, int asmLineNumber) : m_sourceLineNumber(lineNumber), m_asmLineNumber(asmLineNumber)
{
}

bool SourceLineAsmInfo::operator<(const SourceLineAsmInfo& other) const
{
    if (m_sourceLineNumber < other.m_sourceLineNumber)
    {
        return true;
    }

    if (m_sourceLineNumber > other.m_sourceLineNumber)
    {
        return false;
    }

    if (m_asmLineNumber < other.m_asmLineNumber)
    {
        return true;
    }

    if (m_asmLineNumber > other.m_asmLineNumber)
    {
        return false;
    }

    return false;
}

bool SourceLineAsmInfo::operator==(const SourceLineAsmInfo& other) const
{
    if (m_sourceLineNumber != other.m_sourceLineNumber)
    {
        return false;
    }

    if (m_asmLineNumber != other.m_asmLineNumber)
    {
        return false;
    }

    return true;
}

UiFunctionSymbolInfo::UiFunctionSymbolInfo(const FunctionSymbolInfo& exeSym, gtVAddr baseVAddr) : m_size(exeSym.m_size),
    m_name(nullptr != exeSym.m_pName ? QString::fromWCharArray(exeSym.m_pName) : CA_NO_SYMBOL)
{
    m_va = baseVAddr + static_cast<gtVAddr>(exeSym.m_rva);
}

bool UiFunctionSymbolInfo::operator<(const UiFunctionSymbolInfo& other) const
{
    return m_va < other.m_va;
}

bool UiFunctionSymbolInfo::operator==(const UiFunctionSymbolInfo& other) const
{
    return m_va == other.m_va && m_size == other.m_size && m_name == other.m_name;
}

SourceCodeTreeView::SourceCodeTreeView(SessionSourceCodeView* pSourceCodeView, QAbstractItemModel* pModel, int frozenColumn) :
    acFrozenColumnTreeView(nullptr, pModel, frozenColumn), m_pSourceCodeView(pSourceCodeView)
{
    setModel(pModel);
}

void SourceCodeTreeView::FixColumnSizes()
{
    for (int i = 0; i < model()->columnCount(); i++)
    {
        resizeColumnToContents(i);
    }

    int sourceColWidth = width();

    for (int i = 0 ; i < model()->columnCount(); i++)
    {
        if (i != SOURCE_VIEW_SOURCE_COLUMN)
        {
            sourceColWidth -= columnWidth(i);
        }
    }

    // Calculate the width for the source code column:
    setColumnWidth(SOURCE_VIEW_SOURCE_COLUMN, sourceColWidth);
}

void SourceCodeTreeView::Repaint()
{
    QRect dirtyRegionRect = visibleRegion().boundingRect();
    setDirtyRegion(dirtyRegionRect);
    repaint();
}
