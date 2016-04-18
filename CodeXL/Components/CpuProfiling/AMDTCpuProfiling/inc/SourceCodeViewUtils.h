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

// CPU Profile BE:
#include <ProfilingAgents/Utils/ExecutableReader.h>

// Local:
#include <inc/SourceViewTreeItem.h>

class SourceChartSample
{
public:
    gtVAddr m_firstAddress;
    gtVector<float> m_samples;
    QString m_functionName;
};


typedef QMap < gtVAddr, SourceChartSample> SourceChartSampMap;

class SourceLineKey
{
public:
    QString m_fileName;
    int m_lineNumber;
    QString m_functionName;
    gtVAddr m_functionStartAddress;

    SourceLineKey(int lineNumber);
    bool operator< (const SourceLineKey& other) const;;
    bool operator==(const SourceLineKey& other) const;;
};

class SourceLineAsmInfo
{
public:

    int m_sourceLineNumber;
    int m_asmLineNumber;
    SourceLineAsmInfo();
    SourceLineAsmInfo(int lineNumber, int asmLineNumber);
    bool operator< (const SourceLineAsmInfo& other) const;;
    bool operator== (const SourceLineAsmInfo& other) const;;

};
class SessionSourceCodeView;
class acTablePercentItemDelegate;
class TreeItemDelegate;
class SourceCodeTreeView : public acFrozenColumnTreeView
{
public:
    SourceCodeTreeView(SessionSourceCodeView* pSourceCodeView, QAbstractItemModel* pModel, int frozenColumn);

    void FixColumnSizes();

    void Repaint();

protected:

    SessionSourceCodeView* m_pSourceCodeView;

};

class UiFunctionSymbolInfo
{
public:
    gtVAddr  m_va;
    gtUInt32 m_size;
    QString m_name;

    UiFunctionSymbolInfo();
    UiFunctionSymbolInfo(const FunctionSymbolInfo& exeSym, gtVAddr baseVAddr);
    bool operator<(const UiFunctionSymbolInfo& other) const;
    bool operator==(const UiFunctionSymbolInfo& other) const;
};

typedef QMap <gtVAddr, gtVector<float> > AddressToDataMap;
typedef gtMap <SourceLineKey, gtVector<float> > SourceLineToDataMap;
typedef gtList<UiFunctionSymbolInfo> FuncSymbolsList;
typedef gtMap<SourceLineAsmInfo, int> SourceLineToTableRowMap;
typedef gtMap<SourceLineAsmInfo, QString> SourceLineToCodeBytesMap;
typedef gtMap<SourceLineAsmInfo, SourceViewTreeItem*> SourceViewTreeItemMap;
typedef QMap<SourceViewTreeItem*, SourceLineAsmInfo> SourceLineToItemMap;
typedef gtMap<SourceLineAsmInfo, gtVector<float> > SourceLineAsmInfoToDataMap;


#endif //__SOURCECODEVIEWUTILS_H

