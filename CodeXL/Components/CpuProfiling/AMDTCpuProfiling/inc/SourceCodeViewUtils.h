//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SourceCodeViewUtils.h
///
//==================================================================================

#ifndef __SOURCECODEVIEWUTILS_H
#define __SOURCECODEVIEWUTILS_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acFrozenColumnTreeView.h>
#include <AMDTExecutableFormat/inc/SymbolEngine.h>

// Local:
#include <inc/SourceViewTreeItem.h>
#include <inc/StdAfx.h>

class SourceLineKey
{
public:
    QString m_fileName;
    int m_lineNumber = 0;
    QString m_functionName;
    gtVAddr m_functionStartAddress = 0;

    SourceLineKey(int lineNumber);
    bool operator<(const SourceLineKey& other) const;
    bool operator==(const SourceLineKey& other) const;
};

class SourceLineAsmInfo
{
public:
    int m_sourceLineNumber = -1;
    int m_asmLineNumber = -1;

    SourceLineAsmInfo() = default;
    SourceLineAsmInfo(int lineNumber, int asmLineNumber);
    bool operator<(const SourceLineAsmInfo& other) const;
    bool operator==(const SourceLineAsmInfo& other) const;
};

class SessionSourceCodeView;

class SourceCodeTreeView : public acFrozenColumnTreeView
{
public:
    SourceCodeTreeView(SessionSourceCodeView* pSourceCodeView, QAbstractItemModel* pModel, int frozenColumn);
    void FixColumnSizes();
    void Repaint();

protected:
    SessionSourceCodeView* m_pSourceCodeView = nullptr;
};

class UiFunctionSymbolInfo
{
public:
    gtVAddr  m_va = GT_INVALID_VADDR;
    gtUInt32 m_size = 0;
    QString  m_name = CA_NO_SYMBOL;

    UiFunctionSymbolInfo() = default;
    UiFunctionSymbolInfo(const FunctionSymbolInfo& exeSym, gtVAddr baseVAddr);
    bool operator<(const UiFunctionSymbolInfo& other) const;
    bool operator==(const UiFunctionSymbolInfo& other) const;
};

typedef QMap<gtVAddr, gtVector<float>> AddressToDataMap;
typedef gtMap<SourceLineKey, gtVector<float>> SourceLineToDataMap;
typedef gtList<UiFunctionSymbolInfo> FuncSymbolsList;
typedef gtMap<SourceLineAsmInfo, QString> SourceLineToCodeBytesMap;
typedef gtMap<SourceLineAsmInfo, SourceViewTreeItem*> SourceViewTreeItemMap;
typedef QMap<SourceViewTreeItem*, SourceLineAsmInfo> SourceLineToItemMap;
typedef gtMap<SourceLineAsmInfo, gtVector<float>> SourceLineAsmInfoToDataMap;


#endif //__SOURCECODEVIEWUTILS_H

