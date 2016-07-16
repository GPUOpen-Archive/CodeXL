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

// AMDTCpuProfilingRawData:
#include <AMDTCpuProfilingRawData/inc/CpuProfileModule.h>


#include <AMDTProfilingAgentsData/inc/JavaJncReader.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTProfilingAgentsData/inc/Windows/ClrJncReader.h>
    #include <AMDTProfilingAgentsData/inc/Windows/PjsReader.h>
#endif // AMDT_WINDOWS_OS

#ifdef TBI
    #include <ProfilingAgentsData/inc/OclJncReader.h>
#endif

// Local:
#include <inc/SourceCodeViewUtils.h>
#include <inc/StdAfx.h>

#include <unordered_map>


class SessionDisplaySettings;
class CpuProfileReader;

class SourceCodeTreeModel : public QAbstractItemModel
{
    friend class SessionSourceCodeView;

public:
    SourceCodeTreeModel(SessionDisplaySettings* pSessionDisplaySettings, const QString& sessionDir,
                        shared_ptr<cxlProfileDataReader> pProfDataRdr, shared_ptr<DisplayFilter> displayFilter);

    ~SourceCodeTreeModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole);

    bool insertColumns(int position, int columns, const QModelIndex& parent = QModelIndex());
    bool removeColumns(int position, int columns, const QModelIndex& parent = QModelIndex());
    bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());

    /// Return the index of the top level item parent (or itself), and the index of the item within it's top level item parent:
    bool GetItemTopLevelIndex(const QModelIndex& index, int& indexOfTopLevelItem, int& indexOfTopLevelItemChild);

    void Clear();

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

    /// Insert the disassembly line at the end of the source code tree;
    void InsertDasmLines(gtVAddr displayAddress, unsigned int startIndex);

    void PopulateCurrentFunction(const QString& hotSpotCaption);
    void PopulateSourceLine(int lineIndex, SourceLineKey& sourceLineKey);
    void DiscoverAsmLine(int iAsm, SourceViewTreeItem* pLineItem, const SourceLineKey& sourceLineKey, QMap <gtVAddr, SourceViewTreeItem*>& asmItemMap);
    void PopulateDasmLines(const SourceLineKey& sourceLineKey, QMap <gtVAddr, SourceViewTreeItem*>& asmItemMap);

    /// Sets the samples and samples percent columns data and headers:
    void SetTreeSamples(const QString& hotSpotCaption);

    void SetHotSpotSamples(AMDTUInt32 counterIdx);

    /// Sets the tree items data as percent / values:
    //void SetDataPercentValues();

    /// Sets the value for the requested source view item:
    //void SetSingleItemDataValue(SourceViewTreeItem* pItem, int column, bool appendPercent);
    //void SetPercentFormat(double  val, bool appendPercent, QVariant& data);
    void SetDisplayFormat(double  val, bool appendPercent, QVariant& data, const int precision);


    /// Sets the data values on the data columns:
    bool SetDataValues(int lineNumber, int asmLineNumber, const gtVector<float>& dataVector);

    /// Update the displayed headers:
    bool UpdateHeaders();

    /// Store module details:
    //void SetModuleDetails(const CpuProfileModule* pModule);
    void SetModuleDetails(AMDTUInt32 moduleId, AMDTUInt32 pId);
    void BuildTree(const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap);

    void PrintFunctionDetailData(AMDTProfileFunctionData  functionData,
                                 gtString srcFilePath,
                                 AMDTSourceAndDisasmInfoVec srcInfoVec,
                                 const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap);
    void GetInstOffsets(gtUInt16 srcLine, AMDTSourceAndDisasmInfoVec& srcInfoVec, gtVector<gtVAddr>& instOffsetVec);
    void GetDisasmString(gtVAddr offset, AMDTSourceAndDisasmInfoVec& srcInfoVec, gtString& disasm, gtString& codeByte);
    void GetDisasmSampleValue(gtVAddr offset, AMDTProfileInstructionDataVec& dataVec, AMDTSampleValueVec& sampleValue);
    AMDTUInt64 GetFuncSrcFirstLnNum() const { return m_funcFirstSrcLine; }
    const std::vector<SourceViewTreeItem*> GetSrcLineViewMap() const { return m_srcLineViewTreeMap; }

private:

    // Symbol engine stuff
    //bool InitializeSymbolEngine();

    //bool SourceLineInstancesToOffsetLinenumMap(SrcLineInstanceMap* pInstances);
    bool SetSourceLines(const QString& filepath, unsigned int startLine, unsigned int stopLine);

    bool IsSourceLineMapped(const SourceLineKey& sourceLineKey);

    /// Calculate the current module total sample count vector:
    void CalculateTotalModuleCountVector(CpuProfileReader* pProfileReader);

    // Symbol list utilities:
    //bool SetupSymbolInfoListUnmanaged(AMDTUInt32 modId, AMDTUInt32 pId);
    //bool SetupSymbolInfoListUnmanaged();
    //void SetupSymbolInfoListManaged();
    //void SetupSymbolInfoListManaged(AMDTUInt32 modId, AMDTUInt32 pId);
    //bool SetupSourceInfoForUnManaged();
    //bool SetupSourceInfoForJava(gtVAddr address);

    bool SetupSymbolInfoList(AMDTUInt32 modId, AMDTUInt32 pId);
    bool SetupSourceInfo();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    bool SetupSymbolInfoNgen(QString pjsFile);
    //bool SetupSourceInfoForClr(gtVAddr address);
    bool GetSourceLineInfoForCLR(gtUInt32 clrSymOffset, OffsetLinenumMap& jitLineMap);
    bool GetClrOffsetFromSymbol(gtRVAddr& offset);
#endif // AMDT_WINDOWS_OS

#ifdef TBI
    bool SetupSourceInfoForOcl(gtVAddr address);
#endif

    //void CreateSymbolInfoList(AMDTUInt32 modId, AMDTUInt32 pId);
    //void CreateSymbolInfoList();

    SourceViewTreeItem* getItem(const QModelIndex& index) const;
private:

    SessionDisplaySettings* m_pSessionDisplaySettings;
    SourceCodeTreeView* m_pSessionSourceCodeTreeView;

    /// Source line -> tree item map:
    SourceViewTreeItemMap m_sourceTreeItemsMap;

    /// Source line -> data map (for optimization):
    SourceLineToItemMap m_sourceLineToTreeItemsMap;

    /// Source line -> data map:
    SourceLineAsmInfoToDataMap m_sourceLinesToDataMap;

    SourceLineToDataMap m_sourceLinesData;

    /// Contain a map from dasm lines to the code bytes:
    SourceLineToCodeBytesMap m_sourceLineToCodeBytesMap;

    /// Contain the accumulated data values for the current displayed function:
    gtVector<float> m_currentFunctionTotalDataVector;

    /// Contain the accumulated data values for the current displayed module:
    gtVector<float> m_currentModuleTotalDataVector;

    bool m_isDisplayingOnlyDasm;

    AddressToDataMap m_addressData;

    ExecutableFile* m_pExecutable;

    bool m_isLongMode;
    const gtUByte* m_pCode;
    unsigned int m_codeSize;

    // This map contains the current offset to
    // source line information for current function
    OffsetLinenumMap m_funOffsetLinenumMap;

    QVector<QString> m_srcLinesCache;
    unsigned int m_startLine;
    unsigned int m_stopLine;


    // Session and module details:
    QString m_sessionDir;
    QString m_moduleName;

    ProcessIdType m_pid;
    ProcessIdType m_newPid;
    ThreadIdType m_tid;
    ThreadIdType m_newTid;
    AMDTFunctionId m_funcId;

    CpuProfileModule::MOD_MODE_TYPE m_moduleType;
    AggregatedSample m_moduleAggregatedSample;
    const CpuProfileModule* m_pModule;
    const CpuProfileFunction* m_pDisplayedFunction;

    AMDTModuleType m_modType;

    bool m_isModuleCached;
    long m_moduleTotalSamplesCount;
    long m_sessionTotalSamplesCount;

    // Java JNC reader
    JavaJncReader m_javaJncReader;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    ClrJncReader m_clrJncReader;
#endif // AMDT_WINDOWS_OS

#ifdef TBI
    OclJncReader m_oclJncReader;
#endif


    QString m_srcFile;

    gtVAddr m_loadAddr;
    gtVAddr m_newAddress;

    FuncSymbolsList m_symbolsInfoList;
    FuncSymbolsList::iterator m_currentSymbolIterator;
    FuncSymbolsList::iterator m_lastSymIt;

    /// Contain the tree root item:
    SourceViewTreeItem* m_pRootItem;

    QStringList m_headerCaptions;
    QStringList m_headerTooltips;

    QColor m_forgroundColor;

    AMDTProfileSourceLineDataVec m_srcLineDataVec;

    shared_ptr<cxlProfileDataReader> m_pProfDataRdr;
    shared_ptr<DisplayFilter> m_pDisplayFilter;
    AMDTUInt64  m_funcFirstSrcLine = 0;

    // srcLineNumber for sampled  --> SourceViewTreeItem
    std::vector<std::pair<AMDTProfileSourceLineData, SourceViewTreeItem*>> m_sampleSrcLnViewTreeList;
    std::vector<SourceViewTreeItem*> m_srcLineViewTreeMap;

};

#endif //__SOURCECODETREEMODEL_H

