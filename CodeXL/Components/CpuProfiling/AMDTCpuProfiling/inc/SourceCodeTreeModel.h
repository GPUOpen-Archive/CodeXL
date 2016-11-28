//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SourceCodeTreeModel.h
///
//==================================================================================

#ifndef __SOURCECODETREEMODEL_H
#define __SOURCECODETREEMODEL_H

// Qt:
#include <QAbstractItemModel>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

// Backend
#include <AMDTCpuProfilingRawData/inc/CpuProfileFunction.h>

// Local:
#include <inc/SourceCodeViewUtils.h>
#include <inc/StdAfx.h>

struct InstOffsetSize
{
    gtVAddr     m_offset = 0;
    gtUInt32    m_size = 0;
};

class SourceCodeTreeModel : public QAbstractItemModel
{
    friend class SessionSourceCodeView;

public:
    SourceCodeTreeModel(const QString& sessionDir,
                        std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                        std::shared_ptr<DisplayFilter> displayFilter);

    ~SourceCodeTreeModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole);
    /// Update the displayed headers:
    bool UpdateHeaders();

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

    bool insertColumns(int position, int columns, const QModelIndex& parent = QModelIndex());
    bool removeColumns(int position, int columns, const QModelIndex& parent = QModelIndex());
    bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());

    /// Return the index of the top level item parent (or itself), and the index of the item within it's top level item parent:
    bool GetItemTopLevelIndex(const QModelIndex& index, int& indexOfTopLevelItem, int& indexOfTopLevelItemChild);

    /// Return the amount of tree top level items:
    int topLevelItemCount();

    /// Return the top level item for the requested index:
    SourceViewTreeItem* topLevelItem(int index);
    bool isItemTopLevel(SourceViewTreeItem* pItem);

    /// Update the model data:
    void UpdateModel();

    /// Return the index for the top level item:
    int indexOfTopLevelItem(SourceViewTreeItem* pItem);

    QModelIndex indexOfItem(SourceViewTreeItem* pItem);

    /// Fill the current file source lines:
    void BuildSourceLinesTree(std::vector<SourceViewTreeItem*>& srcLineViewTreeMap);

    /// Fill the current file disassembly lines:
    bool BuildDisassemblyTree();

    /// Add the "double click to view..." item to the tree:
    SourceViewTreeItem* AddDoubleClickItem(SourceViewTreeItem* pAsmItem);

    /// Prepare the tree items for the current file source and dasm lines:
    bool BuildSourceAndDASMTree();

    void SetHotSpotSamples(AMDTUInt32 counterIdx);

    void SetDisplayFormat(double val, bool appendPercent, QVariant& data, const int precision);

    /// Store module details:
    void SetModuleDetails(AMDTUInt32 moduleId, AMDTUInt32 pId);
    void BuildTree(const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap);

    void PrintFunctionDetailData(const AMDTProfileFunctionData&  functionData,
                                 gtString srcFilePath,
                                 AMDTSourceAndDisasmInfoVec srcInfoVec,
                                 const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap);
    void GetInstOffsets(gtUInt16 srcLine, const AMDTSourceAndDisasmInfoVec& srcInfoVec, gtVector<InstOffsetSize>& instOffsetVec);
    void GetDisasmString(gtVAddr offset, const AMDTSourceAndDisasmInfoVec& srcInfoVec, gtString& disasm, gtString& codeByte);
    void GetDisasmSampleValue(const InstOffsetSize& instInfo,
                              const AMDTProfileInstructionDataVec& dataVec,
                              AMDTSampleValueVec& sampleValue);

    AMDTUInt64 GetFuncSrcFirstLnNum() const { return m_funcFirstSrcLine; }
    const std::vector<SourceViewTreeItem*> GetSrcLineViewMap() const { return m_srcLineViewTreeMap; }

    void Clear();

private:
    bool SetSourceLines(const QString& filepath, unsigned int startLine, unsigned int stopLine);
    bool SetupSourceInfo();

    SourceViewTreeItem* getItem(const QModelIndex& index) const;
    void SetHotSpotsrcLnSamples(AMDTUInt32 counterId,
                                const AMDTProfileFunctionData&  functionData,
                                const AMDTSourceAndDisasmInfoVec& srcInfoVec);

    void SetHotSpotDisamOnlySamples(AMDTUInt32 counterId,
                                    const AMDTProfileFunctionData&  functionData,
                                    const AMDTSourceAndDisasmInfoVec& srcInfoVec);

private:
    SourceCodeTreeView* m_pSessionSourceCodeTreeView = nullptr;

    /// Source line -> tree item map:
    SourceViewTreeItemMap m_sourceTreeItemsMap;

    /// Source line -> data map (for optimization):
    SourceLineToItemMap m_sourceLineToTreeItemsMap;

    /// Source line -> data map:
    SourceLineAsmInfoToDataMap m_sourceLinesToDataMap;

    /// Contain a map from dasm lines to the code bytes:
    SourceLineToCodeBytesMap m_sourceLineToCodeBytesMap;

    bool m_isDisplayingOnlyDasm = false;

    // This map contains the current offset to
    // source line information for current function
    OffsetLinenumMap m_funOffsetLinenumMap;

    QVector<QString> m_srcLinesCache;
    unsigned int m_startLine = 1;
    unsigned int m_stopLine = 0;

    // Session and module details:
    QString m_sessionDir;
    QString m_moduleName;

    ProcessIdType m_pid = AMDT_PROFILE_ALL_PROCESSES;
    ProcessIdType m_newPid = AMDT_PROFILE_ALL_PROCESSES;
    ThreadIdType m_tid = AMDT_PROFILE_ALL_THREADS;
    ThreadIdType m_newTid = AMDT_PROFILE_ALL_THREADS;
    AMDTFunctionId m_funcId = 0;

    const CpuProfileFunction* m_pDisplayedFunction = nullptr;

    AMDTModuleType m_modType = AMDT_MODULE_TYPE_NONE;

    bool m_isModuleCached = false;

    QString m_srcFile;

    gtVAddr m_newAddress = 0;

    FuncSymbolsList m_symbolsInfoList;
    FuncSymbolsList::iterator m_currentSymbolIterator;
    FuncSymbolsList::iterator m_lastSymIt;

    // Contain the tree root item:
    SourceViewTreeItem* m_pRootItem = nullptr;

    QStringList m_headerCaptions;
    QStringList m_headerTooltips;

    QColor m_forgroundColor;

    AMDTProfileSourceLineDataVec m_srcLineDataVec;

    std::shared_ptr<cxlProfileDataReader> m_pProfDataRdr;
    std::shared_ptr<DisplayFilter> m_pDisplayFilter;
    AMDTUInt64  m_funcFirstSrcLine = 0;

    // srcLineNumber for sampled  --> SourceViewTreeItem
    std::vector<std::pair<AMDTProfileSourceLineData, SourceViewTreeItem*>> m_sampleSrcLnViewTreeList;
    std::vector<SourceViewTreeItem*> m_srcLineViewTreeMap;
};

#endif //__SOURCECODETREEMODEL_H
