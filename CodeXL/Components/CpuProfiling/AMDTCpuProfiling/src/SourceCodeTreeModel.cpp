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
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <inc/DisplayFilter.h>
#include <inc/SourceCodeTreeModel.h>
#include <inc/StringConstants.h>


const gtUInt32 CLR_HIDDEN_LINE = 0x00feefee;
const gtUInt32 SAMPLE_PERCENT_PRECISION = 2;
const gtUInt32 SAMPLE_PRECISION = 6;

// The size of disassembly block to be fetched on first time.
const unsigned int FIRST_DISASSEMBLY_FETCH_BLOCK_SIZE = 1024;


static inline double CalculatePercent(double sampleValue, double totalSamples)
{
    return (sampleValue / totalSamples) * 100;
}

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
                                         std::shared_ptr<DisplayFilter> displayFilter) :
    QAbstractItemModel(nullptr),
    m_pProfDataRdr(pProfDataRdr),
    m_pDisplayFilter(displayFilter)
{
    m_sessionDir = sessionDir;
    m_sessionDir = m_sessionDir.replace('/', PATH_SLASH);

    // m_newAddress holds the addr value for the DisplayAddress.
    // DisplayAddress sets m_newAddress back to its default value i.e. 0.
    m_newAddress = 0;

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
    m_srcLineViewTreeMap.clear();
    m_sampleSrcLnViewTreeList.clear();

    delete m_pRootItem;
    m_pRootItem = nullptr;

    m_srcLineDataVec.clear();
    m_headerTooltips.clear();
    m_headerCaptions.clear();
    m_srcLinesCache.clear();

    m_pSessionSourceCodeTreeView = nullptr;
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
    GT_UNREFERENCED_PARAMETER(parent);
    int retVal = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

    GT_IF_WITH_ASSERT(m_pProfDataRdr != nullptr)
    {
        AMDTProfileCounterDescVec counterDesc;
        m_pProfDataRdr->GetSampledCountersList(counterDesc);
        retVal += counterDesc.size();
    }

    return retVal;
}

QVariant SourceCodeTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        switch (role)
        {
        case Qt::ForegroundRole:
        {
            SourceViewTreeItem* pItem = getItem(index);

            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                return pItem->forground(index.column());
            }
            break;
        }
        case Qt::BackgroundRole:
        {
            // Set alternate column background color to light gray.
            if (index.column() > SOURCE_VIEW_CODE_BYTES_COLUMN && (index.column() & 1) == 0)
            {
                return QColor(0xee, 0xee, 0xe6);
            }
            break;
        }
        case Qt::DisplayRole:
        {
            SourceViewTreeItem* pItem = getItem(index);

            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                return pItem->data(index.column()).toString();
            }
            break;
        }
        case Qt::ToolTipRole:
        {
            SourceViewTreeItem* pItem = getItem(index);

            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                return pItem->tooltip(index.column()).toString();
            }
            break;
        }
        case Qt::FontRole:
        {
            if (index.column() == SOURCE_VIEW_SOURCE_COLUMN ||
                index.column() == SOURCE_VIEW_CODE_BYTES_COLUMN)
            {
                QFont font("Courier New", 8);
                font.setStyleHint(QFont::Monospace);
                return font;
            }
            break;
        }
        case Qt::TextAlignmentRole:
        {
            if (index.column() > SOURCE_VIEW_CODE_BYTES_COLUMN)
            {
                return Qt::AlignCenter;
            }
            break;
        }
        }
    }

    return QVariant();
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
    SourceViewTreeItem* pRetVal = m_pRootItem;

    if (index.isValid())
    {
        pRetVal = static_cast<SourceViewTreeItem*>(index.internalPointer());
    }

    return pRetVal;
}

QVariant SourceCodeTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        if (orientation == Qt::Horizontal)
        {
            if (role == Qt::DisplayRole)
            {
                GT_IF_WITH_ASSERT((section >= 0) && (section < m_headerCaptions.size()))
                {
                    return m_headerCaptions.at(section);
                }
            }
            else if (role == Qt::ToolTipRole)
            {
                GT_IF_WITH_ASSERT((section >= 0) && (section < m_headerTooltips.size()))
                {
                    return m_headerTooltips.at(section);
                }
            }
        }
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

        if (pChildItem != nullptr)
        {
            return createIndex(row, column, pChildItem);
        }
    }

    return QModelIndex();
}

bool SourceCodeTreeModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        beginInsertColumns(parent, position, position + columns - 1);
        retVal = m_pRootItem->insertColumns(position, columns);
        endInsertColumns();
    }

    return retVal;
}

bool SourceCodeTreeModel::insertRows(int position, int rows, const QModelIndex& parent)
{
    bool retVal = false;
    SourceViewTreeItem* pParentItem = getItem(parent);

    GT_IF_WITH_ASSERT(pParentItem != nullptr && m_pRootItem != nullptr)
    {
        beginInsertRows(parent, position, position + rows - 1);
        retVal = pParentItem->insertChildren(position, rows, m_pRootItem->columnCount());
        endInsertRows();
    }

    return retVal;
}

QModelIndex SourceCodeTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        SourceViewTreeItem* pItem = getItem(index);

        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            SourceViewTreeItem* pParentItem = pItem->parent();

            if (pParentItem == m_pRootItem)
            {
                return QModelIndex();
            }
            else
            {
                return createIndex(pParentItem->row(), 0, pParentItem);
            }
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
    bool retVal = false;
    SourceViewTreeItem* pParentItem = getItem(parent);

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
    m_srcLineViewTreeMap.clear();
    m_sampleSrcLnViewTreeList.clear();
    m_srcLineDataVec.clear();
    m_funcFirstSrcLine = 0;

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

    // Fetch aggregated samples for each counter id
    AMDTSampleValueVec aggrSampleValueVec;
    m_pProfDataRdr->GetSampleCount(false, aggrSampleValueVec);

    AMDTUInt64 moduleBaseAddr = functionData.m_modBaseAddress;

    for (const auto& srcData : functionData.m_srcLineDataList)
    {
        bool isSamplePercentSet = m_pDisplayFilter->IsDisplaySamplePercent();

        gtVector<InstOffsetSize> instOffsetVec;
        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);

        int column = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

        for (const auto& instOffset : instOffsetVec)
        {
            SourceViewTreeItem* pAsmItem = new SourceViewTreeItem(SOURCE_VIEW_ASM_DEPTH, m_pRootItem);

            gtString disasm;
            gtString codeByte;
            GetDisasmString(instOffset.m_offset, srcInfoVec, disasm, codeByte);

            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, functionData.m_instDataList, sampleValue);

            column = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;
            bool isFirst = true;

            for (const auto& aSampleValue : sampleValue)
            {
                auto counterId = aSampleValue.m_counterId;
                auto sampleCount = aSampleValue.m_sampleCount;
                auto funcSamplePercent = aSampleValue.m_sampleCountPercentage;

                auto iter = std::find_if(aggrSampleValueVec.cbegin(), aggrSampleValueVec.cend(),
                    [&](const AMDTSampleValue& sampleInfo) {
                    return counterId == sampleInfo.m_counterId;
                });

                double totalSamples = iter->m_sampleCount;
                auto appSamplePercent = CalculatePercent(sampleCount, totalSamples);

                if (isFirst)
                {
                    QVariant var;

                    SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);
                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, funcSamplePercent);
                    pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                    SetTooltipFormat(sampleCount, funcSamplePercent, appSamplePercent, var);
                    pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, var);
                    pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, var);

                    isFirst = false;
                }

                if (!isSamplePercentSet)
                {
                    QVariant var;
                    SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                    pAsmItem->setData(column, var);
                }
                else
                {
                    QVariant var;
                    SetDisplayFormat(sampleCount, true, var, SAMPLE_PERCENT_PRECISION);
                    pAsmItem->setData(column, var);
                }

                QVariant tooltip;
                SetTooltipFormat(sampleCount, funcSamplePercent, appSamplePercent, tooltip);
                pAsmItem->setTooltip(column, tooltip);

                column++;
            }

            pAsmItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(instOffset.m_offset + moduleBaseAddr, 16));
            pAsmItem->setForeground(SOURCE_VIEW_ADDRESS_COLUMN, acQGREY_TEXT_COLOUR);

            pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, acGTStringToQString(disasm));
            pAsmItem->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acDARK_PURPLE);

            pAsmItem->setData(SOURCE_VIEW_CODE_BYTES_COLUMN, acGTStringToQString(codeByte));
            pAsmItem->setForeground(SOURCE_VIEW_CODE_BYTES_COLUMN, acDARK_GREEN);
        }
    }

    return true;
}

void SourceCodeTreeModel::GetInstOffsets(gtUInt16 srcLine,
                                         const AMDTSourceAndDisasmInfoVec& srcInfoVec,
                                         gtVector<InstOffsetSize>& instOffsetVec) const
{
    for (const auto& srcInfo : srcInfoVec)
    {
        if (srcInfo.m_sourceLine == srcLine)
        {
            instOffsetVec.emplace_back(srcInfo.m_offset, srcInfo.m_size);
        }
    }
}

void SourceCodeTreeModel::GetDisasmString(gtVAddr offset,
                                          const AMDTSourceAndDisasmInfoVec& srcInfoVec,
                                          gtString& disasm,
                                          gtString& codeByte) const
{
    auto instData = std::find_if(srcInfoVec.begin(),
                                 srcInfoVec.end(),
                                 [&](const AMDTSourceAndDisasmInfo& srcInfo) {
                                     return srcInfo.m_offset == offset;
                                 });

    if (instData != srcInfoVec.end())
    {
        disasm = instData->m_disasmStr;
        codeByte = instData->m_codeByteStr;
    }
    else
    {
        disasm.makeEmpty();
        codeByte.makeEmpty();
    }
}

void SourceCodeTreeModel::GetDisasmSampleValue(const InstOffsetSize& instInfo,
                                               const AMDTProfileInstructionDataVec& dataVec,
                                               AMDTSampleValueVec& sampleValue) const
{
    auto checkOffsetRange = [&](const AMDTProfileInstructionData& data)
    {
        return (data.m_offset >= instInfo.m_offset) && (data.m_offset < (instInfo.m_offset + instInfo.m_size));
    };

    bool isFirst = true;
    auto instData = std::find_if(dataVec.begin(), dataVec.end(), checkOffsetRange);

    while (instData != dataVec.end())
    {
        if (isFirst)
        {
            sampleValue = instData->m_sampleValues;
            isFirst = false;
        }
        else
        {
            for (gtUInt32 i = 0; i < instData->m_sampleValues.size(); i++)
            {
                sampleValue[i].m_sampleCount += instData->m_sampleValues[i].m_sampleCount;
                sampleValue[i].m_sampleCountPercentage += instData->m_sampleValues[i].m_sampleCountPercentage;
            }
        }

        instData = std::find_if(++instData, dataVec.end(), checkOffsetRange);
    }
}

void SourceCodeTreeModel::PopulateFunctionSampleData(const AMDTProfileFunctionData& data,
                                                     gtString srcFilePath,
                                                     AMDTSourceAndDisasmInfoVec srcInfoVec,
                                                     const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap)
{
    GT_UNREFERENCED_PARAMETER(srcFilePath);

    SourceViewTreeItem* pLineItem = nullptr;
    bool samplePercentSet = m_pDisplayFilter->IsDisplaySamplePercent();
    AMDTUInt64 moduleBaseAddr = data.m_modBaseAddress;

    // Fetch aggregated samples for each counter id
    AMDTSampleValueVec aggrSampleValueVec;
    m_pProfDataRdr->GetSampleCount(false, aggrSampleValueVec);

    m_srcLineDataVec.clear();
    m_srcLineDataVec = data.m_srcLineDataList;

    m_funcFirstSrcLine = 0;

    for (const auto& srcData : data.m_srcLineDataList)
    {
        // Validating file contains the requested line number.
        if (srcLineViewTreeMap.size() >= srcData.m_sourceLineNumber - 1)
        {
            pLineItem = srcLineViewTreeMap.at(srcData.m_sourceLineNumber - 1);
        }

        if (m_funcFirstSrcLine == 0)
        {
            // set the address for first line with samples of the function
            m_funcFirstSrcLine = srcData.m_sourceLineNumber;
        }

        m_sampleSrcLnViewTreeList.emplace_back(srcData, pLineItem);

        if (srcData.m_sampleValues.empty())
        {
            continue;
        }

        int column = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;
        bool isFirst = true;

        for (const auto& sample : srcData.m_sampleValues)
        {
            auto iter = std::find_if(aggrSampleValueVec.cbegin(), aggrSampleValueVec.cend(),
                [&](const AMDTSampleValue& sampleInfo) {
                return sample.m_counterId == sampleInfo.m_counterId;
            });

            double totalSamples = iter->m_sampleCount;

            auto sampleValue = sample.m_sampleCount;
            auto sampleValuePer = sample.m_sampleCountPercentage;
            auto appSamplePercent = CalculatePercent(sampleValue, totalSamples);

            if (isFirst)
            {
                // by default the hotspot is always DC Access(index = 0)
                QVariant var;
                SetDisplayFormat(sampleValue, false, var, SAMPLE_PRECISION);
                pLineItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);
                pLineItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleValuePer);
                pLineItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                SetTooltipFormat(sampleValue, sampleValuePer, appSamplePercent, var);
                pLineItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, var);
                pLineItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, var);

                isFirst = false;
            }

            if (false == samplePercentSet)
            {
                QVariant var;
                SetDisplayFormat(sample.m_sampleCount, false, var, SAMPLE_PRECISION);
                pLineItem->setData(column, var);
            }
            else
            {
                QVariant var;
                SetDisplayFormat(sample.m_sampleCountPercentage, true, var, SAMPLE_PERCENT_PRECISION);
                pLineItem->setData(column, var);
            }

            QVariant tooltip;
            SetTooltipFormat(sample.m_sampleCount, sample.m_sampleCountPercentage, appSamplePercent, tooltip);
            pLineItem->setTooltip(column, tooltip);

            column++;
        }

        // For this srcLine get the list of inst offsets.
        gtVector<InstOffsetSize> instOffsetVec;
        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);

        bool isAddrSet = false;

        for (const auto& instOffset : instOffsetVec)
        {
            SourceViewTreeItem* pAsmItem = new SourceViewTreeItem(SOURCE_VIEW_ASM_DEPTH, pLineItem);

            gtString disasm;
            gtString codeByte;
            GetDisasmString(instOffset.m_offset, srcInfoVec, disasm, codeByte);

            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, data.m_instDataList, sampleValue);

            if (!sampleValue.empty())
            {
                // by default the hotspot is always DC Access(index = 0)
                column = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;
                bool isFirstSample = true;

                for (const auto& aSampleValue : sampleValue)
                {
                    auto sampleCount = aSampleValue.m_sampleCount;
                    auto sampleCountPer = aSampleValue.m_sampleCountPercentage;

                    auto iter = std::find_if(aggrSampleValueVec.cbegin(), aggrSampleValueVec.cend(),
                        [&](const AMDTSampleValue& sampleInfo) {
                        return aSampleValue.m_counterId == sampleInfo.m_counterId;
                    });

                    auto totalSamples = iter->m_sampleCount;
                    auto appSamplePercent = CalculatePercent(sampleCount, totalSamples);

                    if (isFirstSample)
                    {
                        QVariant var;
                        SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                        pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);
                        pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleCountPer);
                        pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                        SetTooltipFormat(sampleCount, sampleCountPer, appSamplePercent, var);
                        pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, var);
                        pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, var);

                        isFirstSample = false;
                    }

                    if (false == samplePercentSet)
                    {
                        QVariant var;
                        SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                        pAsmItem->setData(column, var);
                    }
                    else
                    {
                        QVariant var;
                        SetDisplayFormat(sampleCount, true, var, SAMPLE_PERCENT_PRECISION);
                        pAsmItem->setData(column, var);
                    }

                    QVariant tooltip;
                    SetTooltipFormat(sampleCount, sampleCountPer, appSamplePercent, tooltip);
                    pAsmItem->setTooltip(column, tooltip);

                    column++;
                }

                if (!isAddrSet)
                {
                    pLineItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(moduleBaseAddr + instOffset.m_offset, 16));
                    isAddrSet = true;
                }
            }

            pAsmItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(moduleBaseAddr + instOffset.m_offset, 16));
            pAsmItem->setForeground(SOURCE_VIEW_ADDRESS_COLUMN, acQGREY_TEXT_COLOUR);

            pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, acGTStringToQString(disasm));
            pAsmItem->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acDARK_PURPLE);

            pAsmItem->setData(SOURCE_VIEW_CODE_BYTES_COLUMN, acGTStringToQString(codeByte));
            pAsmItem->setForeground(SOURCE_VIEW_CODE_BYTES_COLUMN, acDARK_GREEN);
        }
    }
}

void SourceCodeTreeModel::BuildTree(const std::vector<SourceViewTreeItem*>& srcLineViewTreeMap)
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

        PopulateFunctionSampleData(functionData, srcFilePath, srcInfoVec, srcLineViewTreeMap);
    }
}

bool SourceCodeTreeModel::BuildSourceAndDASMTree()
{
    m_srcLineViewTreeMap.clear();
    m_sampleSrcLnViewTreeList.clear();

    BuildSourceLinesTree(m_srcLineViewTreeMap);
    BuildTree(m_srcLineViewTreeMap);

    return true;
}

bool SourceCodeTreeModel::UpdateHeaders(AMDTUInt32 hotspotCounterId)
{
    bool retVal = false;

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

        CounterNameIdVec counterDesc;
        m_pDisplayFilter->GetSelectedCounterList(counterDesc);

        // If hotspot counter id is 0, pick the first counter as hotspot counter.
        if (hotspotCounterId == 0)
        {
            if (counterDesc.size() > 0)
            {
                QString counterName = acGTStringToQString(std::get<0>(counterDesc[0]));
                m_headerTooltips << QString(CP_colCaptionSamplesTooltip).arg(counterName);
                m_headerTooltips << QString(CP_colCaptionSamplesPercentTooltip).arg(counterName);
            }
        }
        else
        {
            gtString counterNameStr = m_pDisplayFilter->GetCounterName(hotspotCounterId);
            QString counterName = acGTStringToQString(counterNameStr);
            m_headerTooltips << QString(CP_colCaptionSamplesTooltip).arg(counterName);
            m_headerTooltips << QString(CP_colCaptionSamplesPercentTooltip).arg(counterName);
        }

        for (const auto& counter : counterDesc)
        {
            QString currentFullName = acGTStringToQString(std::get<0>(counter));
            QString currentCaption = acGTStringToQString(std::get<1>(counter));
            QString currentDescription = acGTStringToQString(std::get<2>(counter));

            QString tooltip;
            acWrapAndBuildFormattedTooltip(currentFullName, currentDescription, tooltip);

            m_headerCaptions << acGTStringToQString(std::get<1>(counter));
            m_headerTooltips << tooltip;
        }

        retVal = true;
    }

    return retVal;
}

int SourceCodeTreeModel::topLevelItemCount() const
{
    int retVal = 0;

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

int SourceCodeTreeModel::indexOfTopLevelItem(SourceViewTreeItem* pItem) const
{
    int retVal = -1;

    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        retVal = m_pRootItem->indexOfChild(pItem);
    }

    return retVal;
}

SourceViewTreeItem* SourceCodeTreeModel::topLevelItem(int index) const
{
    SourceViewTreeItem* pRetVal = nullptr;

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

void SourceCodeTreeModel::BuildSourceLinesTree(std::vector<SourceViewTreeItem*>& srcLineViewTreeMap)
{
    // NOTE: Cache index start from 0 but line start from 1.
    if (m_stopLine <= static_cast<gtUInt32>(m_srcLinesCache.size()))
    {
        for (gtUInt32 line = m_startLine; line <= m_stopLine; line++)
        {
            auto pLineItem = new SourceViewTreeItem(SOURCE_VIEW_LINE_DEPTH, m_pRootItem);

            // Add source:
            QString lineStr = m_srcLinesCache[line - 1];
            pLineItem->setData(SOURCE_VIEW_SOURCE_COLUMN, lineStr.replace("\t", "    "));

            // Add line number:
            pLineItem->setData(SOURCE_VIEW_LINE_COLUMN, QVariant((uint)line));
            pLineItem->setForeground(SOURCE_VIEW_LINE_COLUMN, AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
            srcLineViewTreeMap.push_back(pLineItem);
        }
    }
}

bool SourceCodeTreeModel::SetSourceLines(const QString& filePath, unsigned int startLine, unsigned int stopLine)
{
    if (filePath.isEmpty())
    {
        // Clear cached source line info
        m_lastSrcFile.clear();
        m_srcLinesCache.clear();
        m_startLine = m_stopLine = 0;
    }
    else if (m_lastSrcFile != filePath)
    {
        // Don't read the source file again if it is already cached.
        unsigned int count = 0;
        QFile file(filePath);
        m_lastSrcFile = filePath;

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

        m_startLine = (startLine > 0) ? startLine : 1;
        m_stopLine = (stopLine < count) ? stopLine : count;

        file.close();
    }

    return true;
}

QModelIndex SourceCodeTreeModel::indexOfItem(SourceViewTreeItem* pItem) const
{
    if (pItem != nullptr && pItem != m_pRootItem)
    {
        SourceViewTreeItem *pParentItem = pItem->parent();

        if (pParentItem != nullptr)
        {
            int row = pParentItem->indexOfChild(pItem);
            int col = 0;

            return createIndex(row, col, pItem);
        }
    }

    return QModelIndex();
}

void SourceCodeTreeModel::SetDisplayFormat(double val, bool appendPercent, QVariant& data, const int precision)
{
    if (val > 0)
    {
        QString strPrecision = QString::number(val, 'f', precision);

        if (appendPercent)
        {
            strPrecision.append("%");
        }

        data = strPrecision;
    }
}

void SourceCodeTreeModel::SetTooltipFormat(double val, double funcPercent, double appPercent, QVariant& tooltip)
{
    const size_t MaxTooltipLength = 100;

    if (val > 0)
    {
        char strTooltip[MaxTooltipLength + 1] = { 0 };
        snprintf(strTooltip, MaxTooltipLength, CP_sourceCodeViewFunctionsPercentageTooltip, funcPercent, appPercent);
        tooltip = QString(strTooltip);
    }
}

SourceViewTreeItem* SourceCodeTreeModel::AddDoubleClickItem(SourceViewTreeItem* pAsmItem)
{
    SourceViewTreeItem* pRetVal = nullptr;

    GT_IF_WITH_ASSERT(pAsmItem != nullptr)
    {
        pRetVal = new SourceViewTreeItem(SOURCE_VIEW_ASM_DEPTH, pAsmItem);

        // Set the text and tooltip for the item:
        QVariant textData = CP_sourceCodeViewDoubleClickText;
        QVariant tooltipData = CP_sourceCodeViewDoubleClickTooltip;
        pRetVal->setData(SOURCE_VIEW_SOURCE_COLUMN, textData);
        pRetVal->setTooltip(SOURCE_VIEW_SOURCE_COLUMN, tooltipData);
        pRetVal->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acRED_NUMBER_COLOUR);
    }

    return pRetVal;
}

bool SourceCodeTreeModel::GetItemTopLevelIndex(const QModelIndex& index, int& indexOfTopLevelItem, int& indexOfTopLevelItemChild) const
{
    bool retVal = false;
    indexOfTopLevelItem = -1;
    indexOfTopLevelItemChild = -1;

    // Get the item for this index:
    SourceViewTreeItem* pItem = getItem(index);

    if ((pItem != nullptr) && (m_pRootItem != pItem))
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

    return retVal;
}

bool SourceCodeTreeModel::isItemTopLevel(SourceViewTreeItem* pItem) const
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
    GT_IF_WITH_ASSERT(moduleId != AMDT_PROFILE_ALL_MODULES)
    {
        GT_IF_WITH_ASSERT((nullptr != m_pProfDataRdr) && (nullptr != m_pDisplayFilter))
        {
            AMDTProfileModuleInfoVec modInfo;
            bool ret = m_pProfDataRdr->GetModuleInfo(processId, moduleId, modInfo);

            if (ret && (modInfo.size() > 0))
            {
                m_modType = modInfo[0].m_type;
                m_moduleName = acGTStringToQString(modInfo[0].m_path);
            }
        }
    }
}

void SourceCodeTreeModel::SetHotSpotsrcLnSamples(AMDTUInt32 counterId,
                                                 const AMDTProfileFunctionData&  functionData,
                                                 const AMDTSourceAndDisasmInfoVec& srcInfoVec)
{
    // Fetch aggregated samples for each counter id
    AMDTSampleValueVec aggrSampleValueVec;
    m_pProfDataRdr->GetSampleCount(false, aggrSampleValueVec);

    for (const auto& srcLn : m_sampleSrcLnViewTreeList)
    {
        gtUInt16 lineNumber = static_cast<gtUInt16>(srcLn.first.m_sourceLineNumber);
        SourceViewTreeItem* pLineItem = srcLn.second;

        for (const auto& counter : srcLn.first.m_sampleValues)
        {
            if (counterId == counter.m_counterId)
            {
                gtVector<InstOffsetSize> instOffsetVec;
                GetInstOffsets(lineNumber, srcInfoVec, instOffsetVec);

                auto sampleCounterId = counter.m_counterId;
                auto sampleCount = counter.m_sampleCount;
                auto sampleValuePer = counter.m_sampleCountPercentage;

                QVariant var;
                SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                pLineItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);
                pLineItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, sampleValuePer);
                pLineItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                auto iter = std::find_if(aggrSampleValueVec.cbegin(), aggrSampleValueVec.cend(),
                    [&](const AMDTSampleValue& sampleInfo) {
                        return sampleCounterId == sampleInfo.m_counterId;
                    });
                double totalSamples = iter->m_sampleCount;

                SetTooltipFormat(sampleCount, sampleValuePer, totalSamples, var);
                pLineItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, var);
                pLineItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, var);

                int idx = 0;

                for (const auto& instOffset : instOffsetVec)
                {
                    SourceViewTreeItem* pAsmItem = pLineItem->child(idx++);

                    if (nullptr != pAsmItem)
                    {
                        AMDTSampleValueVec sampleValue;
                        GetDisasmSampleValue(instOffset, functionData.m_instDataList, sampleValue);

                        for (const auto& sample : sampleValue)
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

                                auto iter = std::find_if(aggrSampleValueVec.cbegin(), aggrSampleValueVec.cend(),
                                    [&](const AMDTSampleValue& sampleInfo) {
                                        return sample.m_counterId == sampleInfo.m_counterId;
                                    });
                                double totalSamples = iter->m_sampleCount;
                                double appSamplePercent = CalculatePercent(sampleCount, totalSamples);

                                SetTooltipFormat(sampleCount, sampleCountPer, appSamplePercent, var);
                                pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, var);
                                pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, var);
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

    // Fetch aggregated samples for each counter id
    AMDTSampleValueVec aggrSampleValueVec;
    m_pProfDataRdr->GetSampleCount(false, aggrSampleValueVec);

    for (const auto& srcData : functionData.m_srcLineDataList)
    {
        gtVector<InstOffsetSize> instOffsetVec;
        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);

        for (const auto& instOffset : instOffsetVec)
        {
            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, functionData.m_instDataList, sampleValue);

            for (const auto& aSampleValue : sampleValue)
            {
                if (aSampleValue.m_counterId == counterId)
                {
                    SourceViewTreeItem* pAsmItem = m_pRootItem->child(childIdx);

                    auto sampleCount = aSampleValue.m_sampleCount;
                    auto samplePercent = aSampleValue.m_sampleCountPercentage;

                    QVariant var;
                    SetDisplayFormat(sampleCount, false, var, SAMPLE_PRECISION);
                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, var);
                    pAsmItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, samplePercent);
                    pAsmItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);

                    auto iter = std::find_if(aggrSampleValueVec.cbegin(), aggrSampleValueVec.cend(),
                        [&](const AMDTSampleValue& sampleInfo) {
                            return counterId == sampleInfo.m_counterId;
                        });
                    double totalSamples = iter->m_sampleCount;
                    double appSamplesPercent = CalculatePercent(sampleCount, totalSamples);

                    SetTooltipFormat(sampleCount, samplePercent, appSamplesPercent, var);
                    pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, var);
                    pAsmItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, var);
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
        UpdateHeaders(counterId);

        AMDTProfileFunctionData  functionData;
        int retVal = m_pProfDataRdr->GetFunctionDetailedProfileData(m_funcId,
                                                                    m_pid,
                                                                    m_tid,
                                                                    functionData);

        if (retVal != static_cast<int>(CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE))
        {
            AMDTSourceAndDisasmInfoVec srcInfoVec;
            gtString srcFilePath;
            m_pProfDataRdr->GetFunctionSourceAndDisasmInfo(m_funcId, srcFilePath, srcInfoVec);

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
