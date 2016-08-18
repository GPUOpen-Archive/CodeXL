//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SourceCodeTreeModel.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Cpu Profile BE:
#include <AMDTDisassembler/inc/LibDisassembler.h>

#include <AMDTCpuPerfEventUtils/inc/IbsEvents.h>

// Local:
#include <inc/CPUProfileUtils.h>
#include <inc/CpuProfilingOptions.h>
#include <inc/DisplayFilter.h>
#include <inc/SourceCodeTreeModel.h>
#include <inc/StringConstants.h>
#include <inc/Auxil.h>

const gtUInt32 CLR_HIDDEN_LINE = 0x00feefee;
const gtUInt32 SAMPLE_PERCENT_PRECISION = 2;
const gtUInt32 SAMPLE_PRECISION = 6;

// The size of disassembly block to be fetched on first time.
const unsigned int FIRST_DISASSEMBLY_FETCH_BLOCK_SIZE = 1024;

void DebugPrintMapToOutput(const OffsetLinenumMap& map)
{
    gtString mapStr = L"map:\n";
    OffsetLinenumMap::const_iterator iter = map.begin();

    for (; iter != map.end(); iter++)
    {
        mapStr.appendFormattedString(L"%d, %d\n", iter.value(), iter.key());
    }

    osOutputDebugString(mapStr);
}

SourceCodeTreeModel::SourceCodeTreeModel(const QString& sessionDir,
                                         std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                         std::shared_ptr<DisplayFilter> displayFilter) : QAbstractItemModel(nullptr),
    m_pSessionSourceCodeTreeView(nullptr),
    m_isDisplayingOnlyDasm(false),
    m_pExecutable(nullptr),
    m_isLongMode(true),
    m_pCode(nullptr),
    m_codeSize(0),
    m_startLine(1),
    m_stopLine(0),
    m_sessionDir(""),
    m_moduleName(""),
    m_pid(AMDT_PROFILE_ALL_PROCESSES),
    m_newPid(AMDT_PROFILE_ALL_PROCESSES),
    m_tid(AMDT_PROFILE_ALL_THREADS),
    m_newTid(AMDT_PROFILE_ALL_THREADS),
    m_pModule(nullptr),
    m_pDisplayedFunction(nullptr),
    m_isModuleCached(false),
    m_moduleTotalSamplesCount(0),
    m_sessionTotalSamplesCount(0),
    m_srcFile(""),
    m_pRootItem(nullptr),
    m_pProfDataRdr(pProfDataRdr),
    m_pDisplayFilter(displayFilter)
{
    m_sessionDir = sessionDir;
    m_sessionDir = m_sessionDir.replace('/', PATH_SLASH);

    // m_newAddress holds the addr value for the DisplayAddress.
    // DisplayAddress sets m_newAddress back to its default value i.e. 0.
    m_newAddress = 0;
    m_currentSymbolIterator = m_symbolsInfoList.end();
    m_lastSymIt = m_symbolsInfoList.end();
    m_isLongMode = true;
    m_pCode = nullptr;
    m_codeSize = 0;

    m_isModuleCached = false;

    // Create the tree root item:
    m_pRootItem = new SourceViewTreeItem(SOURCE_VIEW_DEFAULT_DEPTH, nullptr);

    // Update the headers:
    UpdateHeaders();

    // Set the gray forground color:
    QColor color = acGetSystemDefaultBackgroundColor();
    m_forgroundColor = QColor::fromRgb(color.red() * 8 / 10, color.green() * 8 / 10, color.blue() * 8 / 10);
}

SourceCodeTreeModel::~SourceCodeTreeModel()
{
    if (m_pExecutable != nullptr)
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
    }
}

void SourceCodeTreeModel::Clear()
{
    // Remove all rows:
    removeRows(0, rowCount());
}

int SourceCodeTreeModel::rowCount(const QModelIndex& parent) const
{
    int retVal = 0;
    SourceViewTreeItem* pParentItem = getItem(parent);
    GT_IF_WITH_ASSERT(pParentItem != nullptr)
    {
        retVal = pParentItem->childCount();
    }
    return retVal;
}

int SourceCodeTreeModel::columnCount(const QModelIndex& parent) const
{
    (void)(parent); // Unused

    int retVal = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfDataRdr != nullptr)
    {
        // only raw counters supported
        AMDTProfileCounterDescVec counterDesc;
        m_pProfDataRdr->GetSampledCountersList(counterDesc);

        retVal += counterDesc.size();
    }

    return retVal;
}


QVariant SourceCodeTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::ForegroundRole)
    {
        SourceViewTreeItem* pItem = getItem(index);
        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            return pItem->forground(index.column());
        }
    }

    if ((role != Qt::DisplayRole) && (role != Qt::ToolTipRole))
    {
        return QVariant();
    }

    QString retVal;

    if (index.isValid())
    {
        // Get the string for the item:
        if (role == Qt::DisplayRole)
        {
            SourceViewTreeItem* pItem = getItem(index);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                retVal = pItem->data(index.column()).toString();
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            SourceViewTreeItem* pItem = getItem(index);
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                retVal = pItem->tooltip(index.column()).toString();
            }
        }
    }

    return retVal;
}

Qt::ItemFlags SourceCodeTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

SourceViewTreeItem* SourceCodeTreeModel::getItem(const QModelIndex& index) const
{
    SourceViewTreeItem* pRetVal = nullptr;

    if (index.isValid())
    {
        SourceViewTreeItem* pItem = static_cast<SourceViewTreeItem*>(index.internalPointer());

        if (pItem != nullptr)
        {
            pRetVal = pItem;
        }
    }
    else
    {
        pRetVal = m_pRootItem;
    }

    return pRetVal;
}

QVariant SourceCodeTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            GT_IF_WITH_ASSERT((section >= 0) && (section < m_headerCaptions.size()))
            {
                return m_headerCaptions.at(section);
            }
        }

        if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
        {
            GT_IF_WITH_ASSERT((section >= 0) && (section < m_headerTooltips.size()))
            {
                return m_headerTooltips.at(section);
            }
        }

        return QVariant();
    }

    return QVariant();

}

QModelIndex SourceCodeTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return QModelIndex();
    }

    SourceViewTreeItem* pParentItem = getItem(parent);

    if (pParentItem != nullptr)
    {
        SourceViewTreeItem* pChildItem = pParentItem->child(row);

        if (pChildItem)
        {
            return createIndex(row, column, pChildItem);
        }
        else
        {
            return QModelIndex();
        }
    }

    return QModelIndex();

}

bool SourceCodeTreeModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
    bool retVal = false;

    beginInsertColumns(parent, position, position + columns - 1);
    retVal = m_pRootItem->insertColumns(position, columns);
    endInsertColumns();

    return retVal;
}

bool SourceCodeTreeModel::insertRows(int position, int rows, const QModelIndex& parent)
{
    bool retVal = false;

    SourceViewTreeItem* pParentItem = getItem(parent);
    GT_IF_WITH_ASSERT(pParentItem != nullptr)
    {
        beginInsertRows(parent, position, position + rows - 1);
        retVal = pParentItem->insertChildren(position, rows, m_pRootItem->columnCount());
        endInsertRows();
    }

    return retVal;
}

QModelIndex SourceCodeTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    SourceViewTreeItem* pChildItem = getItem(index);
    GT_IF_WITH_ASSERT(pChildItem != nullptr)
    {
        SourceViewTreeItem* pParentItem = pChildItem->parent();

        if (pParentItem == m_pRootItem)
        {
            return QModelIndex();
        }
        else
        {
            return createIndex(pParentItem->childCount(), 0, pParentItem);
        }
    }

    return QModelIndex();
}

bool SourceCodeTreeModel::removeColumns(int position, int columns, const QModelIndex& parent)
{

    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        beginRemoveColumns(parent, position, position + columns - 1);
        retVal = m_pRootItem->removeColumns(position, columns);
        endRemoveColumns();

        if (m_pRootItem->columnCount() == 0)
        {
            removeRows(0, rowCount());
        }
    }

    return retVal;
}

bool SourceCodeTreeModel::removeRows(int position, int rows, const QModelIndex& parent)
{
    SourceViewTreeItem* pParentItem = getItem(parent);
    bool retVal = false;
    GT_IF_WITH_ASSERT(pParentItem != nullptr)
    {
        retVal = true;

        if (rows > 0)
        {
            beginRemoveRows(parent, position, position + rows - 1);
            retVal = pParentItem->removeChildren(position, rows);
            endRemoveRows();
        }
    }

    return retVal;
}

bool SourceCodeTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    SourceViewTreeItem* item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
    {
        emit dataChanged(index, index);
    }

    return result;
}

bool SourceCodeTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
    {
        return false;
    }

    bool result = m_pRootItem->setData(section, value);

    if (result)
    {
        emit headerDataChanged(orientation, section, section);
    }

    return result;
}

bool SourceCodeTreeModel::BuildDisassemblyTree()
{
    gtVector<AMDTProfileCounterDesc> counterDesc;
    gtString srcFilePath;
    AMDTSourceAndDisasmInfoVec srcInfoVec;

    AMDTProfileFunctionData  functionData;
    int retVal = m_pProfDataRdr->GetFunctionDetailedProfileData(m_funcId,
                                                                m_pid,
                                                                m_tid,
                                                                functionData);

    if (retVal != static_cast<int>(CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE))
    {
        retVal = m_pProfDataRdr->GetFunctionSourceAndDisasmInfo(m_funcId, srcFilePath, srcInfoVec);
    }

    AMDTUInt64 moduleBaseAddr = functionData.m_modBaseAddress;

    for (const auto& srcData : functionData.m_srcLineDataList)
    {
        bool samplePercentSet = m_pDisplayFilter->GetSamplePercent();
        gtVector<InstOffsetSize> instOffsetVec;

        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);

        int idx = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

        for (auto& instOffset : instOffsetVec)
        {
            SourceViewTreeItem* pAsmItem = new SourceViewTreeItem(SOURCE_VIEW_ASM_DEPTH,
                                                                  m_pRootItem);

            gtString disasm;
            gtString codeByte;

            GetDisasmString(instOffset.m_offset, srcInfoVec, disasm, codeByte);
            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, functionData.m_instDataList, sampleValue);

            idx = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;
            bool flag = true;

            for (auto& aSampleValue : sampleValue)
            {
                if (true == flag)
                {
                    QVariant var;
                    SetDisplayFormat(aSampleValue.m_sampleCount, false, var, SAMPLE_PRECISION);
                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);

                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, aSampleValue.m_sampleCountPercentage);
                    pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                    flag = false;
                }

                if (false == samplePercentSet)
                {
                    QVariant var;
                    SetDisplayFormat(aSampleValue.m_sampleCount, false, var, SAMPLE_PRECISION);
                    pAsmItem->setData(idx, var);
                }
                else
                {
                    QVariant var;
                    SetDisplayFormat(aSampleValue.m_sampleCountPercentage, true, var, SAMPLE_PERCENT_PRECISION);
                    pAsmItem->setData(idx, var);
                }

                idx++;
            }

            pAsmItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(instOffset.m_offset + moduleBaseAddr, 16));
            pAsmItem->setForeground(SOURCE_VIEW_ADDRESS_COLUMN, acQGREY_TEXT_COLOUR);

            pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, acGTStringToQString(disasm));
            pAsmItem->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acQGREY_TEXT_COLOUR);

            pAsmItem->setData(SOURCE_VIEW_CODE_BYTES_COLUMN, acGTStringToQString(codeByte));
            pAsmItem->setForeground(SOURCE_VIEW_CODE_BYTES_COLUMN, acQGREY_TEXT_COLOUR);

        }
    }

    return true;
}

void SourceCodeTreeModel::GetInstOffsets(gtUInt16 srcLine,
                                         const AMDTSourceAndDisasmInfoVec& srcInfoVec,
                                         gtVector<InstOffsetSize>& instOffsetVec)
{
    for (auto& srcInfo : srcInfoVec)
    {
        if (srcInfo.m_sourceLine == srcLine)
        {
            InstOffsetSize instOffset;
            instOffset.m_offset = srcInfo.m_offset;
            instOffset.m_size = srcInfo.m_size;

            instOffsetVec.emplace_back(instOffset);
        }
    }

    return;
}

void SourceCodeTreeModel::GetDisasmString(gtVAddr offset,
                                          const AMDTSourceAndDisasmInfoVec& srcInfoVec,
                                          gtString& disasm, gtString& codeByte)
{
    auto instData = std::find_if(srcInfoVec.begin(), srcInfoVec.end(),
    [&offset](AMDTSourceAndDisasmInfo const & srcInfo) { return srcInfo.m_offset == offset; });

    if (instData != srcInfoVec.end())
    {
        disasm = instData->m_disasmStr;
        codeByte = instData->m_codeByteStr;
    }

    return;
}

void SourceCodeTreeModel::GetDisasmSampleValue(const InstOffsetSize& instInfo,
                                               const AMDTProfileInstructionDataVec& dataVec,
                                               AMDTSampleValueVec& sampleValue)
{
    auto instData = std::find_if(dataVec.begin(), dataVec.end(),
                                 [&instInfo](AMDTProfileInstructionData const & data)
    { return ((data.m_offset >= instInfo .m_offset) && (data.m_offset < (instInfo.m_offset + instInfo.m_size))); });

    bool found = false;

    while (instData != dataVec.end())
    {
        if (!found)
        {
            sampleValue = instData->m_sampleValues;
            found = true;
        }
        else
        {
            for (gtUInt32 i = 0; i < instData->m_sampleValues.size(); i++)
            {
                sampleValue[i].m_sampleCount += instData->m_sampleValues[i].m_sampleCount;
                sampleValue[i].m_sampleCountPercentage += instData->m_sampleValues[i].m_sampleCountPercentage;
            }
        }

        instData = std::find_if(++instData, dataVec.end(),
                                [&instInfo](AMDTProfileInstructionData const & data)
        { return ((data.m_offset >= instInfo.m_offset) && (data.m_offset < (instInfo.m_offset + instInfo.m_size))); });
    }

    return;
}

void SourceCodeTreeModel::PrintFunctionDetailData(const AMDTProfileFunctionData& data,
                                                  gtString srcFilePath,
                                                  AMDTSourceAndDisasmInfoVec srcInfoVec,
                                                  const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap)
{
    GT_UNREFERENCED_PARAMETER(srcFilePath);

    SourceViewTreeItem* pLineItem = nullptr;
    m_srcLineDataVec.clear();
    bool samplePercentSet = m_pDisplayFilter->GetSamplePercent();
    AMDTUInt64 moduleBaseAddr = data.m_modBaseAddress;

    m_srcLineDataVec = data.m_srcLineDataList;

    // set the address for first line of a function
    bool flag = true;

    for (const auto& srcData : data.m_srcLineDataList)
    {
        // error check: validating file contains the requested
        // line number.
        if (srcLineViewTreeMap.size() < srcData.m_sourceLineNumber - 1)
        {
            return;
        }
        else
        {
            pLineItem = srcLineViewTreeMap.at(srcData.m_sourceLineNumber - 1);
        }

        if (true == flag)
        {
            m_funcFirstSrcLine = srcData.m_sourceLineNumber;
            flag = false;
        }

        m_sampleSrcLnViewTreeList.push_back(std::make_pair(srcData, pLineItem));

        // For this srcLine get the list of inst offsets..
        gtVector<InstOffsetSize> instOffsetVec;
        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);
        bool flag = true;

        if (srcData.m_sampleValues.empty())
        {
            continue;
        }

        // by default the hotspot is always DC Access(index = 0)
        auto sampleValue = srcData.m_sampleValues.at(0).m_sampleCount;
        auto sampleValuePer = srcData.m_sampleValues.at(0).m_sampleCountPercentage;

        QVariant var;
        SetDisplayFormat(sampleValue, false, var, SAMPLE_PRECISION);
        pLineItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);

        pLineItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleValuePer);

        pLineItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
        int idx = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

        for (const auto& sample : srcData.m_sampleValues)
        {
            if (false == samplePercentSet)
            {
                QVariant var;
                SetDisplayFormat(sample.m_sampleCount, false, var, SAMPLE_PRECISION);
                pLineItem->setData(idx, var);
            }
            else
            {
                QVariant var;
                SetDisplayFormat(sample.m_sampleCountPercentage, true, var, SAMPLE_PERCENT_PRECISION);
                pLineItem->setData(idx, var);
            }

            idx++;
        }


        for (auto& instOffset : instOffsetVec)
        {
            SourceViewTreeItem* pAsmItem = new SourceViewTreeItem(SOURCE_VIEW_ASM_DEPTH,
                                                                  pLineItem);

            gtString disasm;
            gtString codeByte;

            GetDisasmString(instOffset.m_offset, srcInfoVec, disasm, codeByte);
            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, data.m_instDataList, sampleValue);

            if (!sampleValue.empty())
            {
                // by default the hotspot is always DC Access(index = 0)
                auto sampleCount = sampleValue.at(0).m_sampleCount;
                auto sampleCountPer = sampleValue.at(0).m_sampleCountPercentage;

                QVariant var;
                SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);

                pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleCountPer);

                pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
                idx = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

                for (auto& aSampleValue : sampleValue)
                {
                    if (false == samplePercentSet)
                    {
                        QVariant var;
                        SetDisplayFormat(aSampleValue.m_sampleCount, false, var, SAMPLE_PRECISION);
                        pAsmItem->setData(idx, var);
                    }
                    else
                    {
                        QVariant var;
                        SetDisplayFormat(aSampleValue.m_sampleCountPercentage, true, var, SAMPLE_PERCENT_PRECISION);
                        pAsmItem->setData(idx, var);
                    }

                    idx++;
                }

                if (true == flag)
                {
                    pLineItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(moduleBaseAddr + instOffset.m_offset, 16));
                    flag = false;
                }
            }

            pAsmItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(moduleBaseAddr + instOffset.m_offset, 16));
            pAsmItem->setForeground(SOURCE_VIEW_ADDRESS_COLUMN, acQGREY_TEXT_COLOUR);

            pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, acGTStringToQString(disasm));
            pAsmItem->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acQGREY_TEXT_COLOUR);

            pAsmItem->setData(SOURCE_VIEW_CODE_BYTES_COLUMN, acGTStringToQString(codeByte));
            pAsmItem->setForeground(SOURCE_VIEW_CODE_BYTES_COLUMN, acQGREY_TEXT_COLOUR);
        }

    }
}

void SourceCodeTreeModel::BuildTree(const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap)
{
    gtVector<AMDTProfileCounterDesc> counterDesc;
    bool ret = m_pProfDataRdr->GetSampledCountersList(counterDesc);

    if (true == ret)
    {
        AMDTProfileFunctionData  functionData;
        int retVal = m_pProfDataRdr->GetFunctionDetailedProfileData(m_funcId,
                                                                    m_pid,
                                                                    m_tid,
                                                                    functionData);

        if (retVal != static_cast<int>(CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE))
        {
            gtString srcFilePath;
            AMDTSourceAndDisasmInfoVec srcInfoVec;
            retVal = m_pProfDataRdr->GetFunctionSourceAndDisasmInfo(m_funcId, srcFilePath, srcInfoVec);

            PrintFunctionDetailData(functionData, srcFilePath, srcInfoVec, srcLineViewTreeMap);
        }
    }
}

bool SourceCodeTreeModel::BuildSourceAndDASMTree()
{
    m_srcLineViewTreeMap.clear();
    m_sampleSrcLnViewTreeList.clear();

    // Build the source lines tree:
    BuildSourceLinesTree(m_srcLineViewTreeMap);
    BuildTree(m_srcLineViewTreeMap);
    return true;

}

bool SourceCodeTreeModel::UpdateHeaders()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDisplayFilter != nullptr)
    {
        // Build the header captions and header tooltips:
        m_headerCaptions.clear();
        m_headerTooltips.clear();

        // Add the data header captions and tooltips:
        m_headerCaptions << CP_colCaptionLineNumber;
        m_headerCaptions << CP_colCaptionAddress;
        m_headerCaptions << CP_colCaptionLineSourceCode;
        m_headerCaptions << CP_colCaptionCodeBytes;
        m_headerCaptions << CP_colCaptionHotSpotSamples;
        m_headerCaptions << CP_colCaptionSamplesPercent;

        m_headerTooltips << CP_colCaptionLineNumberTooltip;
        m_headerTooltips << CP_colCaptionAddressTooltip;
        m_headerTooltips << CP_colCaptionLineSourceCodeTooltip;
        m_headerTooltips << CP_colCaptionCodeBytesTooltip;
        m_headerTooltips << CP_colCaptionSamplesTooltip;
        m_headerTooltips << CP_colCaptionSamplesPercentTooltip;

        CounterNameIdVec counterDesc;
        m_pDisplayFilter->GetSelectedCounterList(counterDesc);

        for (const auto& counter : counterDesc)
        {
            m_headerCaptions << acGTStringToQString(std::get<1>(counter));

            QString currentCaption = acGTStringToQString(std::get<1>(counter));
            QString currentFullName = acGTStringToQString(std::get<0>(counter));
            QString currentDescription = acGTStringToQString(std::get<2>(counter));

            QString tooltip;
            acBuildFormattedTooltip(currentFullName, currentDescription, tooltip);

            m_headerTooltips << tooltip;
        }

        retVal = true;
    }

    return retVal;
}

int SourceCodeTreeModel::topLevelItemCount()
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        retVal = m_pRootItem->childCount();
    }

    return retVal;
}


void SourceCodeTreeModel::UpdateModel()
{
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rowCount() - 1, columnCount() - 1);
    emit dataChanged(topLeft, bottomRight);

    beginResetModel();
    endResetModel();
}


int SourceCodeTreeModel::indexOfTopLevelItem(SourceViewTreeItem* pItem)
{
    int retVal = -1;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        retVal = m_pRootItem->indexOfChild(pItem);
    }
    return retVal;
}

SourceViewTreeItem* SourceCodeTreeModel::topLevelItem(int index)
{
    SourceViewTreeItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        pRetVal = m_pRootItem->child(index);
    }
    return pRetVal;
}

bool SourceCodeTreeModel::SetupSourceInfo()
{
    m_srcFile.clear();

    gtString srcFile;
    bool ret = m_pProfDataRdr->GetSourceFilePathForFunction(m_funcId, srcFile);

    if (ret && (srcFile != L"Unknown Source File") && (srcFile != L"UnknownJITSource"))
    {
        m_srcFile = acGTStringToQString(srcFile);
    }

    return ret;
}

bool SourceCodeTreeModel::IsSourceLineMapped(const SourceLineKey& sourceLineKey)
{
    bool retVal = false;

    SourceLineToDataMap::iterator mappedSourceLinesIter = m_sourceLinesData.begin();
    SourceLineToDataMap::iterator mappedSourceLinesIterEnd = m_sourceLinesData.begin();

    for (; mappedSourceLinesIter != mappedSourceLinesIterEnd; mappedSourceLinesIter++)
    {
        if (mappedSourceLinesIter->first == sourceLineKey)
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

void SourceCodeTreeModel::BuildSourceLinesTree(std::vector<SourceViewTreeItem*>& srcLineViewTreeMap)
{
    SourceViewTreeItem* pLineItem = nullptr;

    m_sourceTreeItemsMap.clear();
    m_sourceLineToTreeItemsMap.clear();

    for (gtUInt32 line = m_startLine; line <= m_stopLine; line++)
    {
        pLineItem = new SourceViewTreeItem(SOURCE_VIEW_LINE_DEPTH, m_pRootItem);

        // NOTE: Cache index start from 0 but line start from 1.
        QString lineStr = m_srcLinesCache[line - 1];

        // Add source:
        if (line <= (gtUInt32)m_srcLinesCache.size())
        {
            pLineItem->setData(SOURCE_VIEW_SOURCE_COLUMN, lineStr.replace("\t", "    "));
        }

        // Add line number:
        pLineItem->setData(SOURCE_VIEW_LINE_COLUMN, QVariant((uint)line));
        pLineItem->setForeground(SOURCE_VIEW_LINE_COLUMN, AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
        srcLineViewTreeMap.push_back(pLineItem);
    }
}


bool SourceCodeTreeModel::SetSourceLines(const QString& filePath, unsigned int startLine, unsigned int stopLine)
{
    unsigned int count = 0;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QTextStream srcStream(&file);
    srcStream.setCodec("UTF-8");

    m_srcLinesCache.clear();

    while (!srcStream.atEnd())
    {
        m_srcLinesCache.push_back(srcStream.readLine());
        count++;
    }

    m_startLine = (startLine <= 0) ? 1 : startLine;
    m_stopLine = stopLine;

    if (m_stopLine > count)
    {
        m_stopLine = count;
    }

    file.close();
    return true;
}

QModelIndex SourceCodeTreeModel::indexOfItem(SourceViewTreeItem* pItem)
{
    return createIndex(m_funcFirstSrcLine - 1, 0, pItem);
}

void SourceCodeTreeModel::SetDisplayFormat(double  val, bool appendPercent, QVariant& data, const int precision)
{
    if (val > 0)
    {
        QString strPrecision = QString::number(val, 'f', precision);
        data = QVariant(strPrecision);

        if (appendPercent)
        {
            data.setValue(data.toString().append("%"));
        }
    }
    else if (val == 0)
    {
        QString emptyStr = "";
        data = QVariant(emptyStr);
    }
}

SourceViewTreeItem* SourceCodeTreeModel::AddDoubleClickItem(SourceViewTreeItem* pAsmItem)
{
    SourceViewTreeItem* pRetVal = nullptr;
    // Sanity check:
    GT_IF_WITH_ASSERT(pAsmItem != nullptr)
    {
        pRetVal = new SourceViewTreeItem(SOURCE_VIEW_ASM_DEPTH, pAsmItem);


        int currentDasmLineNumber = pAsmItem->childCount() - 1;

        SourceLineAsmInfo info(-1, currentDasmLineNumber);
        m_sourceTreeItemsMap[info] = pRetVal;
        m_sourceLineToTreeItemsMap[pRetVal] = info;

        // Set the text and tooltip for the item:
        QVariant textData = CP_sourceCodeViewDoubleClickText;
        QVariant tooltipData = CP_sourceCodeViewDoubleClickTooltip;
        pRetVal->setData(SOURCE_VIEW_SOURCE_COLUMN, textData);
        pRetVal->setTooltip(SOURCE_VIEW_SOURCE_COLUMN, tooltipData);
        pRetVal->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acRED_NUMBER_COLOUR);
    }
    return pRetVal;
}

bool SourceCodeTreeModel::GetItemTopLevelIndex(const QModelIndex& index, int& indexOfTopLevelItem, int& indexOfTopLevelItemChild)
{
    bool retVal = false;
    indexOfTopLevelItem = -1;
    indexOfTopLevelItemChild = -1;

    // Get the item for this index:
    SourceViewTreeItem* pItem = getItem(index);

    if (pItem != nullptr)
    {
        if ((pItem != nullptr) && (m_pRootItem != nullptr))
        {
            // Check if this is a top level item:
            int topLevelIndex = m_pRootItem->indexOfChild(pItem);

            if (topLevelIndex >= 0)
            {
                indexOfTopLevelItem = topLevelIndex;
                retVal = true;
            }
            else
            {
                // Should be a child of a top level item:
                topLevelIndex = m_pRootItem->indexOfChild(pItem->parent());

                if (topLevelIndex >= 0)
                {
                    indexOfTopLevelItem = topLevelIndex;
                    indexOfTopLevelItemChild = pItem->parent()->indexOfChild(pItem);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

bool SourceCodeTreeModel::isItemTopLevel(SourceViewTreeItem* pItem)
{
    bool retVal = false;

    if (pItem != nullptr)
    {
        retVal = (pItem->parent() == m_pRootItem);
    }

    return retVal;
}

void SourceCodeTreeModel::SetModuleDetails(AMDTUInt32 moduleId, AMDTUInt32 processId)
{
    GT_IF_WITH_ASSERT((nullptr != m_pProfDataRdr) && (nullptr != m_pDisplayFilter))
    {
        AMDTProfileModuleInfoVec modInfo;

        // Baskar: FIXME what to do if the moduleId is ALL_MODULES?
        bool ret = m_pProfDataRdr->GetModuleInfo(processId, moduleId, modInfo);

        if (true == ret)
        {
            for (const auto& module : modInfo)
            {
                // Fill in the details from the CpuProfileModule class:
                m_modType = module.m_type;
                //TODO: setting to 0 for now
                m_moduleTotalSamplesCount = 0;
                m_moduleName = acGTStringToQString(module.m_path);
            }
        }
    }
}

void SourceCodeTreeModel::SetHotSpotsrcLnSamples(AMDTUInt32 counterId,
                                                 const AMDTProfileFunctionData&  functionData,
                                                 const AMDTSourceAndDisasmInfoVec& srcInfoVec)
{
    SourceViewTreeItem* pLineItem = nullptr;

    for (const auto& srcLn : m_sampleSrcLnViewTreeList)
    {
        int lineNumber = srcLn.first.m_sourceLineNumber;

        pLineItem = srcLn.second;

        for (const auto& counter : srcLn.first.m_sampleValues)
        {
            if (counterId == counter.m_counterId)
            {
                gtVector<InstOffsetSize> instOffsetVec;
                GetInstOffsets(lineNumber, srcInfoVec, instOffsetVec);

                auto sampleValuePer = counter.m_sampleCountPercentage;

                QVariant var;
                SetDisplayFormat(counter.m_sampleCount, false, var, SAMPLE_PRECISION);
                pLineItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);

                pLineItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleValuePer);
                pLineItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                int idx = 0;

                for (auto& instOffset : instOffsetVec)
                {
                    SourceViewTreeItem* pAsmItem = pLineItem->child(idx++);

                    if (nullptr != pAsmItem)
                    {
                        gtString disasm;
                        gtString codeByte;

                        GetDisasmString(instOffset.m_offset, srcInfoVec, disasm, codeByte);
                        AMDTSampleValueVec sampleValue;
                        GetDisasmSampleValue(instOffset, functionData.m_instDataList, sampleValue);

                        if (!sampleValue.empty())
                        {
                            for (const auto sample : sampleValue)
                            {
                                if (sample.m_counterId == counterId)
                                {
                                    auto sampleCount = sample.m_sampleCount;
                                    auto sampleCountPer = sample.m_sampleCountPercentage;

                                    QVariant var;
                                    SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);

                                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleCountPer);
                                    pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
                                }
                            }
                        }
                    }
                }

            }
        }
    }
}


void SourceCodeTreeModel::SetHotSpotDisamOnlySamples(AMDTUInt32 counterId,
                                                     const AMDTProfileFunctionData&  functionData,
                                                     const AMDTSourceAndDisasmInfoVec& srcInfoVec)
{
    int childIdx = 0;

    for (const auto& srcData : functionData.m_srcLineDataList)
    {
        gtVector<InstOffsetSize> instOffsetVec;
        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);

        for (auto& instOffset : instOffsetVec)
        {
            gtString disasm;
            gtString codeByte;

            GetDisasmString(instOffset.m_offset, srcInfoVec, disasm, codeByte);
            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, functionData.m_instDataList, sampleValue);

            for (auto& aSampleValue : sampleValue)
            {
                if (counterId == aSampleValue.m_counterId)
                {
                    SourceViewTreeItem* pAsmItem = m_pRootItem->child(childIdx);

                    QVariant var;
                    SetDisplayFormat(aSampleValue.m_sampleCount, false, var, SAMPLE_PRECISION);
                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);

                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, aSampleValue.m_sampleCountPercentage);
                    pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
                }
            }

            childIdx++;
        }
    }
}


void SourceCodeTreeModel::SetHotSpotSamples(AMDTUInt32 counterId)
{
    GT_UNREFERENCED_PARAMETER(counterId);

    if (m_pDisplayFilter != nullptr)
    {
        AMDTProfileFunctionData  functionData;
        int retVal = m_pProfDataRdr->GetFunctionDetailedProfileData(m_funcId,
                                                                    m_pid,
                                                                    m_tid,
                                                                    functionData);

        if (retVal != static_cast<int>(CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE))
        {
            AMDTSourceAndDisasmInfoVec srcInfoVec;
            gtString srcFilePath;

            if (retVal != static_cast<int>(CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE))
            {
                retVal = m_pProfDataRdr->GetFunctionSourceAndDisasmInfo(m_funcId, srcFilePath, srcInfoVec);
            }

            if (m_sampleSrcLnViewTreeList.empty())
            {
                SetHotSpotDisamOnlySamples(counterId, functionData, srcInfoVec);
            }
            else
            {
                SetHotSpotsrcLnSamples(counterId, functionData, srcInfoVec);
            }

        }
    }
}