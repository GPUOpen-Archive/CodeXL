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

// The size of disassembly block to be fetched on first time.
const unsigned int FIRST_DISASSEMBLY_FETCH_BLOCK_SIZE = 1024;

// INTERNALLY LINKED UTILITY FUNCTIONS - START

// Helper function used by GetDisassemblyInstructionsChunkSize() to fetch
// disassembly block size from the global settings.
static bool ExtractDisasssemblyInstructionsChunkSizeFromSettings(unsigned int& dasmDisplayRange, unsigned int& dasmMaxDisplayRange)
{
    bool ret = false;

    // Extract the disassembly instructions chunk size selected by the user.
    PROFILE_OPTIONS* pOptions = CpuProfilingOptions::instance().options();

    if (pOptions != nullptr)
    {
        dasmDisplayRange = pOptions->disassemblyInstrcutionsChunkSize;
        dasmMaxDisplayRange = dasmDisplayRange * 4;
        ret = true;
    }

    return ret;
}

// Extracts the disassembly instructions count from the global settings,
// and the max disassembly instructions count from the global settings.
static void GetDisassemblyInstructionsChunkSize(unsigned int& dasmDisplayRange, unsigned int& dasmMaxDisplayRange)
{
    // Set default values.
    dasmDisplayRange      = 1024;
    dasmMaxDisplayRange   = dasmDisplayRange * 4;

    // Extract from the global settings.
    bool isOk = ExtractDisasssemblyInstructionsChunkSizeFromSettings(dasmDisplayRange, dasmMaxDisplayRange);
    GT_ASSERT_EX(isOk, CP_sourceCodeViewDisassemblyChunkSizeExtractionFailure);
}

// INTERNALLY LINKED UTILITY FUNCTIONS - END


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

static void AppendCodeByte(QString& str, gtUByte byteCode)
{
    gtUByte btHigh = (byteCode >> 4);
    gtUByte btLow  = (byteCode & 0xF);
    str.append((btHigh <= 9) ? ('0' + btHigh) : ('A' + btHigh - 0xA));
    str.append((btLow  <= 9) ? ('0' + btLow) : ('A' + btLow  - 0xA));
}

SourceCodeTreeModel::SourceCodeTreeModel(SessionDisplaySettings* pSessionDisplaySettings,
                                         const QString& sessionDir,
                                         CpuProfileReader* pProfileReader) : QAbstractItemModel(nullptr),
    m_pSessionDisplaySettings(pSessionDisplaySettings),
    m_pSessionSourceCodeTreeView(nullptr),
    m_pProfileReader(pProfileReader),
    m_isDisplayingOnlyDasm(false),
    m_pExecutable(nullptr),
    m_isLongMode(true),
    m_pCode(nullptr),
    m_codeSize(0),
    m_startLine(1),
    m_stopLine(0),
    m_sessionDir(""),
    m_moduleName(""),
    m_pid(SHOW_ALL_PIDS),
    m_newPid(SHOW_ALL_PIDS),
    m_tid(SHOW_ALL_TIDS),
    m_newTid(SHOW_ALL_PIDS),
    m_pModule(nullptr),
    m_pDisplayedFunction(nullptr),
    m_isModuleCached(false),
    m_moduleTotalSamplesCount(0),
    m_sessionTotalSamplesCount(0),
    m_srcFile(""),
    m_pRootItem(nullptr)
{
    m_sessionDir = sessionDir;
    m_sessionDir = m_sessionDir.replace('/', PATH_SLASH);

    // m_newAddress holds the addr value for the DisplayAddress.
    // DisplayAddress sets m_newAddress back to its default value i.e. 0.
    m_newAddress = 0;
    m_currentSymbolIterator = m_symbolsInfoList.end();
    m_lastSymIt = m_symbolsInfoList.end();

    m_pid = SHOW_ALL_PIDS;
    m_newPid = SHOW_ALL_PIDS;
    m_tid = SHOW_ALL_TIDS;
    m_newTid = SHOW_ALL_TIDS;

    m_isLongMode = true;
    m_pCode = nullptr;
    m_codeSize = 0;

    m_isModuleCached = false;

    // Create the tree root item:
    m_pRootItem = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_DEFAULT_DEPTH, nullptr);


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
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
    {
        retVal += m_pSessionDisplaySettings->m_availableDataColumnCaptions.size();
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
    SourceViewTreeItem* pCurrentLineItem = nullptr;
    SourceViewTreeItem* pAsmItem = nullptr;

    // Setup disassembler
    LibDisassembler dasm;

    dasm.SetLongMode(m_isLongMode);

    gtVAddr dasmStartOffset = 0;
    gtVAddr dasmStopOffset = m_codeSize;
    gtUInt32 codeOffset = 0;
    gtUInt32 currentLineNumber = 0;
    gtUInt32 oldLineNumber = GT_INT32_MAX;
    OffsetLinenumMap::Iterator offsetLineIter = m_funOffsetLinenumMap.end();

    // The first fetch should be of 1KB.
    unsigned int dasmDisplayRange    = FIRST_DISASSEMBLY_FETCH_BLOCK_SIZE;
    unsigned int dasmMaxDisplayRange = FIRST_DISASSEMBLY_FETCH_BLOCK_SIZE * 4;

    if (!m_funOffsetLinenumMap.isEmpty())
    {
        // Initialize the OffsetLinenumMap iterator.
        offsetLineIter = m_funOffsetLinenumMap.begin();
        currentLineNumber = offsetLineIter.value();
    }
    else if (m_codeSize >= dasmMaxDisplayRange)
    {
        // Displays dasm in [-2*DASM_DISPLAY_RANGE, +2*DASM_DISPLAY_RANGE] window around the m_newAddress
        dasmStartOffset = m_newAddress - m_loadAddr;

        // Set the end address
        if (dasmStartOffset + dasmDisplayRange < m_codeSize)
        {
            dasmStopOffset = dasmStartOffset + dasmDisplayRange;
        }

        // Set the start address
        if (dasmStartOffset < dasmDisplayRange)
        {
            dasmStartOffset = 0;
        }
        else
        {
            dasmStartOffset -= dasmDisplayRange;
        }

        if ((0 < dasmStartOffset) && (pAsmItem != nullptr))
        {
            // Add the "Double click to view..." item to the tree:
            pAsmItem = AddDoubleClickItem(pAsmItem);
        }
    }

    afProgressBarWrapper::instance().setProgressDetails(CP_sourceCodeViewProgressDisassemblyUpdate, dasmStopOffset - codeOffset);

    // While there are instructions left
    while (codeOffset < dasmStopOffset)
    {
        afProgressBarWrapper::instance().incrementProgressBar();

        gtVAddr address = m_loadAddr + codeOffset;

        // Here, we go through line info map which is sorted by offset.
        // For each offset, we get the sourceline with the corresponded
        // line number from the sourceline map, and add the dasm.
        if (!m_isDisplayingOnlyDasm && (offsetLineIter != m_funOffsetLinenumMap.end()))
        {
            // If the current code offset matches the offsetLineIter,
            // advance to new line.
            if (codeOffset == offsetLineIter.key())
            {
                currentLineNumber = offsetLineIter.value();
                offsetLineIter++;
            }

            // If the source line is different from the last iteration,
            // find the source line item and locate the last dasm if any.
            if (currentLineNumber != oldLineNumber)
            {
                oldLineNumber = currentLineNumber;

                SourceLineAsmInfo info(currentLineNumber, -1);

                // Get the source line item to add dasm
                pCurrentLineItem = m_sourceTreeItemsMap[info];

                // If source line is missing, something is wrong.
                // Just stop adding dasm.
                if (!pCurrentLineItem)
                {
                    break;
                }

                // If this line not already has the address, set one
                if (pCurrentLineItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().isEmpty())
                {
                    pCurrentLineItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(address, 16));
                }

                // Get the last dasm line of the current source line to append to
                int childIndex = pCurrentLineItem->childCount() - 1;
                pAsmItem = (SourceViewTreeItem*)pCurrentLineItem->child(childIndex);

                if (childIndex > 0)
                {
                    // If already existing dasm line, we add dasm break to
                    // help separate the dasm block
                    pAsmItem = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_ASM_DEPTH, pCurrentLineItem);


                    int dasmLineNumber = pCurrentLineItem->childCount() - 1;

                    SourceLineAsmInfo asmInfo(currentLineNumber, dasmLineNumber);
                    m_sourceTreeItemsMap[asmInfo] = pAsmItem;
                    m_sourceLineToTreeItemsMap[pAsmItem] = asmInfo;

                    pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, CP_sourceCodeViewBreak);
                    pAsmItem->setForeground(SOURCE_VIEW_SOURCE_COLUMN, acRED_NUMBER_COLOUR);
                }
            }
        }

        // Get disassembly for the current opcode from the disassembler.
        BYTE error_code;
        UIInstInfoType temp_struct;
        char dasmstring[256];
        dasmstring[0] = 0;
        unsigned strlength = 255;

        HRESULT hr = dasm.UIDisassemble((unsigned char*)m_pCode, (unsigned int*) &strlength, (unsigned char*)dasmstring, &temp_struct, &error_code);

        if (S_OK != hr)
        {
            temp_struct.NumBytesUsed = 1;
            strcpy(dasmstring, "BAD DASM");
        }

        // Add disassembly to line:
        if (pCurrentLineItem != nullptr)
        {
            pAsmItem = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_ASM_DEPTH, pCurrentLineItem);


            int currentDasmLineNumber = pCurrentLineItem->childCount() - 1;

            SourceLineAsmInfo info(currentLineNumber, currentDasmLineNumber);
            m_sourceTreeItemsMap[info] = pAsmItem;
            m_sourceLineToTreeItemsMap[pAsmItem] = info;
        }
        else
        {
            if (codeOffset >= dasmStartOffset)
            {
                // Calculate the current dasm line number (the number of root children):
                int currentDasmLineNumber = m_pRootItem->childCount();

                // Create an item for the current disassembly line:
                pAsmItem = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_ASM_DEPTH, m_pRootItem);


                SourceLineAsmInfo info(-1, currentDasmLineNumber);
                m_sourceTreeItemsMap[info] = pAsmItem;
                m_sourceLineToTreeItemsMap[pAsmItem] = info;
            }
        }

        if ((nullptr != pAsmItem) && (codeOffset >= dasmStartOffset) && (m_pSessionSourceCodeTreeView != nullptr))
        {
            for (int j = 0; j < columnCount(); j++)
            {
                pAsmItem->setForeground(j, acQGREY_TEXT_COLOUR);
            }

            pAsmItem->setAsmLength(temp_struct.NumBytesUsed);
            pAsmItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(address, 16));

            // Get code bytes
            char codeBytes[16];
            memcpy(codeBytes, m_pCode, temp_struct.NumBytesUsed);

            int byteCount = 0;
            QString codeByteString;

            for (byteCount = 0; byteCount < temp_struct.NumBytesUsed; byteCount++)
            {
                AppendCodeByte(codeByteString, static_cast<gtUByte>(codeBytes[byteCount]));
                codeByteString += ' ';
            }

            if (0 < temp_struct.NumBytesUsed)
            {
                codeByteString.chop(1);
            }

            // Get the current line and dasm line numbers:
            int dasmLineNumber = topLevelItemCount() - 1;
            int lineNumber = currentLineNumber;

            if (pCurrentLineItem != nullptr)
            {
                dasmLineNumber = pCurrentLineItem->childCount() - 1;
            }
            else
            {
                lineNumber = -1;
            }

            // Map the current dasm line code bytes string:
            SourceLineAsmInfo dasmLineInfo(lineNumber, dasmLineNumber);
            m_sourceLineToCodeBytesMap[dasmLineInfo] = codeByteString;

            // Add pc relative address
            QString disasmString;

            if (temp_struct.bIsPCRelative && temp_struct.bHasDispData)
            {
                disasmString = dasmstring + QString(" (0x") +
                               QString::number((address +
                                                static_cast<gtVAddr>(temp_struct.NumBytesUsed) +
                                                static_cast<gtVAddr>(static_cast<int>(temp_struct.DispDataValue))), 16) + ")";
            }
            else
            {
                disasmString = dasmstring;
            }

            pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, disasmString);
        }

        //set current to next address
        m_pCode += temp_struct.NumBytesUsed;
        codeOffset += temp_struct.NumBytesUsed;
    }

    if ((nullptr == pCurrentLineItem) && (m_codeSize > dasmStopOffset) && (m_pSessionSourceCodeTreeView != nullptr))
    {
        // Add "Double click to render..." item to the tree:
        AddDoubleClickItem(m_pRootItem);
    }


    afProgressBarWrapper::instance().hideProgressBar();

    return true;
}


void SourceCodeTreeModel::InsertDasmLines(gtVAddr displayAddress, unsigned int startIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        // Remove the last item from the tree (the double click item):
        int amountOfTopLevelItems = m_pRootItem->childCount();
        removeRows(amountOfTopLevelItems - 1, 1);

        SourceViewTreeItem* pAsmItem = nullptr;

        // Setup disassembler:
        LibDisassembler dasm;
        dasm.SetLongMode(m_isLongMode);

        // Extract the disassembly instructions chunk size.
        unsigned int dasmDisplayRange      = 0;
        unsigned int dasmMaxDisplayRange   = 0;
        bool isLastSection(false);

        GetDisassemblyInstructionsChunkSize(dasmDisplayRange, dasmMaxDisplayRange);

        gtVAddr dasmStartOffset = displayAddress - m_loadAddr;
        gtVAddr dasmStopOffset = dasmStartOffset;
        gtUInt32 codeOffset = 0;

        if (1 == startIndex)
        {
            // Set start address
            if (dasmStartOffset < dasmDisplayRange)
            {
                dasmStartOffset = 0;
            }
            else
            {
                dasmStartOffset -= dasmDisplayRange;
            }
        }
        else
        {
            // Set end address
            if (dasmStopOffset + dasmDisplayRange > m_codeSize)
            {
                dasmStopOffset = m_codeSize;
                isLastSection = true;
            }
            else
            {
                dasmStopOffset += dasmDisplayRange;
            }
        }

        // Remove the last item (
        // While there are instructions left
        while (codeOffset + dasmStartOffset  < dasmStopOffset)
        {
            gtVAddr address = m_loadAddr + codeOffset + dasmStartOffset;

            // Get disassembly for the current pcode from the disassembler:
            BYTE error_code;
            UIInstInfoType temp_struct;
            char dasmstring[256];
            dasmstring[0] = 0;
            unsigned strlength = 255;

            // Disassemble the code:
            HRESULT hr = dasm.UIDisassemble((unsigned char*)m_pCode, (unsigned int*) &strlength, (unsigned char*)dasmstring, &temp_struct, &error_code);

            if (S_OK != hr)
            {
                temp_struct.NumBytesUsed = 1;
                strcpy(dasmstring, "BAD DASM");
                /* KARTHIK: The Disassembler could not decode the m_pCode string, but we dont have
                    a track of how many bytes we might have read. Lets bail out for now
                    and go back since we are in a error conndition anyway
                    and try to process the next section.*/
                // TODO: Improve the disassembler to give us the number of bytes read even for a error condition.

                return;
            }

            int currentDasmLineNumber = rowCount(QModelIndex());

            pAsmItem = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_ASM_DEPTH, m_pRootItem);

            SourceLineAsmInfo info(-1, currentDasmLineNumber);
            m_sourceTreeItemsMap[info] = pAsmItem;
            m_sourceLineToTreeItemsMap[pAsmItem] = info;

            if (nullptr != pAsmItem)
            {
                for (int j = 0; j < columnCount(); j++)
                {
                    pAsmItem->setForeground(j, acQGREY_TEXT_COLOUR);
                }

                pAsmItem->setAsmLength(temp_struct.NumBytesUsed);
                pAsmItem->setData(SOURCE_VIEW_ADDRESS_COLUMN, "0x" + QString::number(address, 16));

                // Get codebytes
                char codeBytes[16];
                memcpy(codeBytes, m_pCode, temp_struct.NumBytesUsed);

                int byteCount = 0;
                QString codeByteString;

                for (byteCount = 0; byteCount < temp_struct.NumBytesUsed; byteCount++)
                {
                    AppendCodeByte(codeByteString, static_cast<gtUByte>(codeBytes[byteCount]));
                    codeByteString += ' ';
                }

                if (0 < temp_struct.NumBytesUsed)
                {
                    codeByteString.chop(1);
                }

                // Add pc relative address
                QString disasmString;

                if (temp_struct.bIsPCRelative && temp_struct.bHasDispData)
                {
                    disasmString = dasmstring + QString(" (0x") + QString::number((address +
                                                                                   static_cast<gtVAddr>(temp_struct.NumBytesUsed) +
                                                                                   static_cast<gtVAddr>(static_cast<int>(temp_struct.DispDataValue))), 16) + ")";
                }
                else
                {
                    disasmString = dasmstring;
                }

                // Map the current dasm line code bytes string:
                SourceLineAsmInfo dasmLineInfo(-1, currentDasmLineNumber);
                m_sourceLineToCodeBytesMap[dasmLineInfo] = codeByteString;


                pAsmItem->setData(SOURCE_VIEW_SOURCE_COLUMN, disasmString);
            }

            //set current to next address
            m_pCode += temp_struct.NumBytesUsed;
            codeOffset += temp_struct.NumBytesUsed;
        }

        // Add "Double click to view..." item:
        // Only if we have more code to display. else stop.
        if (!isLastSection)
        {
            AddDoubleClickItem(m_pRootItem);
        }
    }
}


bool SourceCodeTreeModel::BuildSourceAndDASMTree()
{
    bool retVal = false;

    // Build the source lines tree:
    BuildSourceLinesTree();

    // Add the disassembly lines for the current displayed function:
    retVal = BuildDisassemblyTree();

    return retVal;

}

void SourceCodeTreeModel::SetDataPercentValues()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
    {
        // Get the data columns count:
        int dataColumnsCount = (int)m_pSessionDisplaySettings->m_availableDataColumnCaptions.size();

        CpuEvent cluUtilEv;

        if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
        {
            EventsFile* pEventsFile = m_pSessionDisplaySettings->getEventsFile();

            if (pEventsFile != nullptr)
            {
                pEventsFile->FindEventByValue(DE_IBS_CLU_PERCENTAGE, cluUtilEv);
            }
        }

        // Update the progress bar + dialog:
        afProgressBarWrapper::instance().setProgressDetails(CP_sourceCodeViewProgressSamplesUpdate, m_sourceTreeItemsMap.size());

        // Get the first data column index:
        for (int col = SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1 ; col <= SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + dataColumnsCount; col++)
        {
            afProgressBarWrapper::instance().incrementProgressBar();

            SourceViewTreeItemMap::iterator iter = m_sourceTreeItemsMap.begin();
            SourceViewTreeItemMap::iterator iterEnd = m_sourceTreeItemsMap.end();

            int positionInDataVector = col - (SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1);
            bool appendPercent = false;

            if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
            {
                if (m_pSessionDisplaySettings->m_availableDataColumnCaptions[positionInDataVector].contains(cluUtilEv.abbrev))
                {
                    appendPercent = true;
                }
            }

            // Fix the values for the data table to contain the percent values:
            for (; iter != iterEnd; iter++)
            {
                SourceViewTreeItem* pItem = iter->second;

                if (pItem != nullptr)
                {
                    // Set this item value (percent / not percent):
                    SetSingleItemDataValue(pItem, col, appendPercent);
                }
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();

    }
}

bool SourceCodeTreeModel::UpdateHeaders()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
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

        for (int i = 0; i < (int) m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size(); i++)
        {
            // Get the displayed column index:
            int displayedColIndex = m_pSessionDisplaySettings->m_displayedDataColumnsIndices[i];

            m_headerCaptions << m_pSessionDisplaySettings->m_availableDataColumnCaptions[displayedColIndex];

            QString currentCaption = m_pSessionDisplaySettings->m_availableDataColumnCaptions[displayedColIndex];
            QString currentFullName = m_pSessionDisplaySettings->m_availableDataFullNames[displayedColIndex];
            QString currentDescription = m_pSessionDisplaySettings->m_availableDataColumnTooltips[displayedColIndex];

            // Format the tooltip:
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

void SourceCodeTreeModel::PopulateCurrentFunction(const QString& hotSpotCaption)
{
    m_sourceLinesData.clear();
    m_sourceLinesToDataMap.clear();
    m_addressData.clear();
    m_currentFunctionTotalDataVector.clear();

    if (m_currentSymbolIterator == m_symbolsInfoList.end())
    {
        return;
    }

    SourceLineKey sourceLineKey(0);
    sourceLineKey.m_fileName = m_srcFile;
    sourceLineKey.m_functionName = m_currentSymbolIterator->m_name;
    sourceLineKey.m_functionStartAddress = m_loadAddr;

    if (!m_isDisplayingOnlyDasm)
    {
        // For each source line
        for (int lineNumber = 0; lineNumber < topLevelItemCount(); lineNumber++)
        {
            PopulateSourceLine(lineNumber, sourceLineKey);
        }
    }
    else
    {
        // For each dasm
        QMap<gtVAddr, SourceViewTreeItem*> asmItemMap;

        for (int iAsm = 0; iAsm < topLevelItemCount(); iAsm++)
        {
            SourceLineKey dummyLineKey(-1);
            DiscoverAsmLine(iAsm, nullptr, dummyLineKey, asmItemMap);
        }

        SourceLineKey dummyLineKey(-1);
        PopulateDasmLines(dummyLineKey, asmItemMap);
    }

    // Set the samples values for each item in the tree:
    SetTreeSamples(hotSpotCaption);
}


void SourceCodeTreeModel::PopulateSourceLine(int lineIndex, SourceLineKey& sourceLineKey)
{
    SourceViewTreeItem* pLineItem = (SourceViewTreeItem*)topLevelItem(lineIndex);
    // Sanity check:

    GT_IF_WITH_ASSERT(pLineItem != nullptr)
    {
        sourceLineKey.m_lineNumber = pLineItem->data(SOURCE_VIEW_LINE_COLUMN).toUInt();
        SourceLineAsmInfo info(sourceLineKey.m_lineNumber, -1);

        // Create source line map item, if lacking
        bool isSourceLineExist = IsSourceLineMapped(sourceLineKey);

        if (!isSourceLineExist)
        {
            // Create lines with 0 values:
            for (int i = 0; i < ((int)m_pSessionDisplaySettings->m_availableDataFullNames.size() + 1); i++)
            {
                m_sourceLinesData[sourceLineKey].push_back(0);
                m_sourceLinesToDataMap[info].push_back(0);
            }
        }

        // For each asm
        QMap<gtVAddr, SourceViewTreeItem*> asmItemMap;

        for (int iAsm = 0; iAsm < pLineItem->childCount(); iAsm++)
        {
            DiscoverAsmLine(iAsm, pLineItem, sourceLineKey, asmItemMap);
        }

        // Add each of the disassembly lines:
        PopulateDasmLines(sourceLineKey, asmItemMap);

        // Set the data values for this line of code:
        bool rc = SetDataValues(sourceLineKey.m_lineNumber, -1, m_sourceLinesData[sourceLineKey]);
        GT_ASSERT(rc);

        pLineItem->m_total = (m_sourceLinesData[sourceLineKey])[m_pSessionDisplaySettings->m_availableDataFullNames.size()];
    }
}

void SourceCodeTreeModel::DiscoverAsmLine(int iAsm, SourceViewTreeItem* pLineItem, const SourceLineKey& sourceLineKey, QMap<gtVAddr, SourceViewTreeItem*>& asmItemMap)
{
    (void)(sourceLineKey); // Unused
    SourceViewTreeItem* pAsmItem = nullptr;

    SourceLineAsmInfo info(-1, iAsm);

    if (pLineItem)
    {
        pAsmItem = (SourceViewTreeItem*)pLineItem->child(iAsm);
        info.m_sourceLineNumber = indexOfTopLevelItem(pLineItem);
    }
    else
    {
        pAsmItem = (SourceViewTreeItem*)topLevelItem(iAsm);
    }

    // Set the item's assembly line:
    pAsmItem->setAssemblyLine(iAsm);

    QString lineSourceData = pAsmItem->data(SOURCE_VIEW_SOURCE_COLUMN).toString();

    if ((m_pDisplayedFunction != nullptr) && (lineSourceData != CP_sourceCodeViewBreak) && (lineSourceData != CP_sourceCodeViewDoubleClickText))
    {
        gtVAddr addr = pAsmItem->data(SOURCE_VIEW_ADDRESS_COLUMN).toString().toULongLong(nullptr, 16);
        gtVAddr offset = addr - m_pDisplayedFunction->getBaseAddr();

        if (!m_addressData.contains(offset))
        {
            // Add line with 0 values:
            for (int i = 0; i < ((int)m_pSessionDisplaySettings->m_availableDataFullNames.size() + 1); i++)
            {
                m_addressData[offset].push_back(0);
            }
        }

        asmItemMap[offset] = pAsmItem;
    }

}


void SourceCodeTreeModel::PopulateDasmLines(const SourceLineKey& sourceLineKey, QMap <gtVAddr, SourceViewTreeItem*>& asmItemMap)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pDisplayedFunction != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        QMap <gtVAddr, SourceViewTreeItem*>::iterator asmIter = asmItemMap.begin();
        SourceLineAsmInfo info(sourceLineKey.m_lineNumber, -1);

        if (asmIter == asmItemMap.end())
        {
            return;
        }

        AptKey ak(asmIter.key(), 0, 0);
        AptAggregatedSampleMap::const_iterator ait = m_pDisplayedFunction->getLowerBoundSample(ak);

        while (ait != m_pDisplayedFunction->getEndSample())
        {
            if (asmIter == asmItemMap.end())
            {
                break;
            }

            SourceViewTreeItem* pItem = asmIter.value();

            if (pItem != nullptr)
            {
                QString lineSource = asmIter.value()->data(SOURCE_VIEW_SOURCE_COLUMN).toString();

                if (lineSource == CP_sourceCodeViewBreak)
                {
                    asmIter++;
                    continue;
                }
            }

            if (((SHOW_ALL_PIDS != m_pid) && (ait->first.m_pid != m_pid)) || ((SHOW_ALL_TIDS != m_tid) && (ait->first.m_tid != m_tid)))
            {
                ait++;
                continue;
            }

            gtVAddr sampleAddr = ait->first.m_addr;
            gtVAddr asmAddr = asmIter.key();
            unsigned int asmLen = asmIter.value()->asmLength();

            if (sampleAddr < asmAddr)
            {
                ait++;
                continue;
            }
            else if (sampleAddr >= asmAddr + asmLen)
            {
                asmIter++;
                continue;
            }

            CpuProfileSampleMap::const_iterator sampleIt = ait->second.getBeginSample();

            for (; sampleIt != ait->second.getEndSample(); sampleIt++)
            {
                SampleKeyType key(sampleIt->first.cpu, sampleIt->first.event);

                //if event is not show for this view, skip
                if (!m_pSessionDisplaySettings->m_eventToIndexMap.contains(key))
                {
                    continue;
                }

                //given cpu/event select from profile, find column index
                int index = m_pSessionDisplaySettings->m_eventToIndexMap[key];

                // The aggregation won't affect the complex columns, since the raw data would be 0:

                // If this is a real line, aggregate it:
                if (sourceLineKey.m_lineNumber >= 0)
                {
                    (m_sourceLinesData[sourceLineKey])[index] += sampleIt->second;
                    (m_sourceLinesToDataMap[info])[index] += sampleIt->second;
                }

                (m_addressData[asmAddr])[index] += sampleIt->second;

                // If part of complex column, re-calculate:
                ComplexDependentMap::const_iterator complexItEnd = m_pSessionDisplaySettings->m_calculatedDataColumns.end();
                ComplexDependentMap::const_iterator complexIt = m_pSessionDisplaySettings->m_calculatedDataColumns.find(index);

                if (complexItEnd == complexIt)
                {
                    continue;
                }

                ListOfComplex::const_iterator cpxIt = (*complexIt).begin();

                for (; cpxIt != (*complexIt).end(); ++cpxIt)
                {
                    float calc;
                    int complexIndex = (*cpxIt).columnIndex;

                    if (CpuProfileModule::UNMANAGEDPE == m_moduleType && (sourceLineKey.m_lineNumber >= 0))
                    {
                        calc = m_pSessionDisplaySettings->setComplex(*cpxIt, m_sourceLinesData[sourceLineKey]);
                        (m_sourceLinesData[sourceLineKey])[complexIndex] = calc;
                        (m_sourceLinesToDataMap[info])[complexIndex] = calc;
                    }

                    calc = m_pSessionDisplaySettings->setComplex(*cpxIt, m_addressData[asmAddr]);
                    (m_addressData[asmAddr])[complexIndex] = calc;
                }
            }

            if (sourceLineKey.m_lineNumber >= 0)
            {
                (m_sourceLinesData[sourceLineKey])[m_pSessionDisplaySettings->m_availableDataFullNames.size()] += ait->second.getTotal();
                (m_sourceLinesToDataMap[info])[m_pSessionDisplaySettings->m_availableDataFullNames.size()] += ait->second.getTotal();
            }

            (m_addressData[asmAddr])[m_pSessionDisplaySettings->m_availableDataFullNames.size()] += ait->second.getTotal();
            ait++;
        }


        if ((sourceLineKey.m_lineNumber >= 0) && m_pSessionDisplaySettings->m_displayClu)
        {
            CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, m_sourceLinesData[sourceLineKey]);
            CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, m_sourceLinesToDataMap[info]);
        }

        // Assign dasm data:
        asmIter = asmItemMap.begin();

        // Update the progress bar + dialog:
        afProgressBarWrapper::instance().setProgressDetails(CP_sourceCodeViewProgressSamplesUpdate, asmItemMap.size());

        for (; asmIter != asmItemMap.end(); ++asmIter)
        {
            afProgressBarWrapper::instance().incrementProgressBar();

            // Get the current address and source view item:
            gtVAddr address = asmIter.key();
            SourceViewTreeItem* pAsmItem = asmIter.value();

            if (pAsmItem->data(SOURCE_VIEW_SOURCE_COLUMN).toString() == CP_sourceCodeViewBreak)
            {
                // Do not save data for the "break" assembly line:
                continue;
            }


            if (m_pSessionDisplaySettings->m_displayClu)
            {
                CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, m_addressData[address]);
            }

            const gtVector<float>& dataVector = m_addressData[address];

            pAsmItem->m_total = dataVector[(int)m_pSessionDisplaySettings->m_availableDataFullNames.size()];

            // assign source file data:
            bool rc = SetDataValues(sourceLineKey.m_lineNumber, pAsmItem->assemblyLine(), dataVector);
            GT_ASSERT(rc);

            SourceLineAsmInfo asmInfo(sourceLineKey.m_lineNumber, pAsmItem->assemblyLine());

            m_sourceLinesToDataMap[asmInfo].clear();

            for (int i = 0 ; i < (int)dataVector.size(); i++)
            {
                m_sourceLinesToDataMap[asmInfo].push_back(dataVector[i]);
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();

    }
}

void SourceCodeTreeModel::SetupSymbolInfoListManaged()
{
    // In case of Managed (i.e. Java, CLR, .NET), the symbol
    // which we care about are listed in the IMD file which
    // is in the m_pModule.

    m_symbolsInfoList.clear();
    //m_currentSymbolIterator = m_symbolsInfoList.end();
    //m_lastSymIt = m_symbolsInfoList.end();

    // For each function
    for (AddrFunctionMultMap::const_iterator fit  = m_pModule->getBeginFunction(), fEnd = m_pModule->getEndFunction();
         fit != fEnd; ++fit)
    {
        const CpuProfileFunction& function = fit->second;

        // Here we use the function name and base address from IMD file
        UiFunctionSymbolInfo symbol;
        symbol.m_va = function.getBaseAddr();
        symbol.m_name = acGTStringToQString(function.getFuncName());
        m_symbolsInfoList.push_back(symbol);
    }
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
bool SourceCodeTreeModel::SetupSymbolInfoNgen(QString pjsFile)
{
    PjsReader pjsReader;

    m_symbolsInfoList.clear();
    //m_currentSymbolIterator = m_symbolsInfoList.end();
    //m_lastSymIt = m_symbolsInfoList.end();

    if (!pjsReader.Open(pjsFile.toStdWString().c_str()))
    {
        return false;
    }

    gtUInt64 loadAddr = pjsReader.GetLoadAddress();
    unsigned int numSym = pjsReader.GetNumberOfRecords();

    if (0 == numSym)
    {
        return false;
    }

    // this will be released in destructor.
    UiFunctionSymbolInfo* pFunctions = new UiFunctionSymbolInfo[numSym];

    for (unsigned int i = 0; i < numSym; i++)
    {
        gtUInt64 startAddr;
        gtUInt32 size;
        char symbolName[2048];
        symbolName[0] = '\0';
        pjsReader.GetNextRecord(&startAddr, &size, symbolName, sizeof(symbolName));

        pFunctions[i].m_va = startAddr + loadAddr;
        pFunctions[i].m_size = size;
        pFunctions[i].m_name = symbolName;
    }

    gtVAddr oldStartAddr = GT_INVALID_VADDR;

    UiFunctionSymbolInfo* pFunc = pFunctions;
    UiFunctionSymbolInfo* const pFunctionsEnd = pFunctions + numSym;

    gtSort(pFunctions, pFunctionsEnd);

    // For each function
    AddrFunctionMultMap::const_iterator fit  = m_pModule->getBeginFunction(), fEnd = m_pModule->getEndFunction();

    for (; fit != fEnd; ++fit)
    {
        const CpuProfileFunction& function = fit->second;

        // For each sample
        AptAggregatedSampleMap::const_iterator sit  = function.getBeginSample(), sEnd = function.getEndSample();

        for (; sit != sEnd; ++sit)
        {

            while (sit->first.m_addr >= ((pFunc->m_va + pFunc->m_size) - loadAddr))
            {
                //if there are no more symbols to add, done
                if (pFunc == pFunctionsEnd)
                {
                    delete [] pFunctions;
                    return true;
                }

                ++pFunc;
            }

            if (pFunc->m_va != oldStartAddr)
            {
                oldStartAddr = pFunc->m_va;
                m_symbolsInfoList.push_back(*pFunc);
            }
        }
    }

    delete [] pFunctions;
    return true;
}


bool SourceCodeTreeModel::GetClrOffsetFromSymbol(gtRVAddr& offset)
{
    SymbolEngine* pSymbolEngine = nullptr;

    if (nullptr != m_pExecutable)
    {
        pSymbolEngine = m_pExecutable->GetSymbolEngine();
    }

    if (nullptr == pSymbolEngine)
    {
        return false;
    }

    // Get IL info from CLR JNC file
    unsigned int offsetToIl = 0;
    unsigned int ilSize = 0;
    const gtUByte* pILMetaData = m_clrJncReader.GetILMetaData();

    if (nullptr == pILMetaData)
    {
        return false;
    }

    if (!m_clrJncReader.GetILInfo(&offsetToIl, &ilSize))
    {
        return false;
    }

    const IMAGE_COR_ILMETHOD* pMethodHeader = reinterpret_cast<const IMAGE_COR_ILMETHOD*>(pILMetaData);
    bool bFatHeader = ((pMethodHeader->Fat.Flags & CorILMethod_FormatMask) == CorILMethod_FatFormat);

    unsigned int headerSize = (bFatHeader) ? 0xC : 0x1;
    unsigned int methodSize = ilSize - headerSize;

    wchar_t funName[MAX_PATH];
    m_clrJncReader.GetFunctionName(funName, MAX_PATH);

    offset = pSymbolEngine->LoadSymbol(funName, methodSize);
    return GT_INVALID_RVADDR != offset;
}
#endif // AMDT_WINDOWS_OS

bool SourceCodeTreeModel::SetupSymbolInfoListUnmanaged()
{
    if (!InitializeSymbolEngine())
    {
        return false;
    }

    SymbolEngine* pSymbolEngine = nullptr;

    if (nullptr != m_pExecutable)
    {
        pSymbolEngine = m_pExecutable->GetSymbolEngine();
    }

    GT_IF_WITH_ASSERT(pSymbolEngine != nullptr)
    {
        m_symbolsInfoList.clear();
        //m_currentSymbolIterator = m_symbolsInfoList.end();
        //m_lastSymIt = m_symbolsInfoList.end();

        // For each function
        for (AddrFunctionMultMap::const_iterator it = m_pModule->getBeginFunction(), itEnd = m_pModule->getEndFunction(); it != itEnd; ++it)
        {
            const CpuProfileFunction& function = it->second;

            // Do not add the uncharted function.
            if (!m_pModule->isUnchartedFunction(function))
            {
                UiFunctionSymbolInfo tmpSymbol;
                tmpSymbol.m_va = function.getBaseAddr();
                tmpSymbol.m_size = function.getSize();
                tmpSymbol.m_name = acGTStringToQString(function.getFuncName());
                m_symbolsInfoList.push_back(tmpSymbol);
            }
        }
    }

    return true;
}



bool SourceCodeTreeModel::SetupSourceInfoForJava(gtVAddr address)
{
    if (m_currentSymbolIterator == m_symbolsInfoList.end())
    {
        return false;
    }

    // If the JNC file does not change, we can just return
    QString jncFilePath(m_sessionDir + "/" + acGTStringToQString(m_pDisplayedFunction->getJncFileName()));

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_javaJncReader.GetJncFilePath().compare(jncFilePath) == 0)
    {
        // Set the code pointer to the java reader text section:
        unsigned int sectionSize = 0;

        m_pCode = m_javaJncReader.GetCodeBytesOfTextSection(&sectionSize);
        m_codeSize = sectionSize;

        return true;
    }

#endif

    // In case the JNC file changes, we open the new JNC file
    m_javaJncReader.Close();

    // Try open the Java JNC file
    wchar_t tmpStr[OS_MAX_PATH] = { L'\0' };
    jncFilePath.toWCharArray(tmpStr);

    if (!m_javaJncReader.Open(tmpStr))
    {
        return false;
    }

    gtVAddr jitLoadAddr;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //jitLoadAddr = m_javaJncReader.GetLoadAddr();
    jitLoadAddr = m_currentSymbolIterator->m_va;
#else

    if (! m_javaJncReader.GetJittedStartAddr(jitLoadAddr))
    {
        jitLoadAddr = m_currentSymbolIterator->m_va;
    }

#endif

    unsigned int offset = address - jitLoadAddr;
    unsigned int sectionSize = 0;

    m_pCode = m_javaJncReader.GetCodeBytesOfTextSection(&sectionSize);

    if (m_pCode == nullptr)
    {
        return false;
    }

    // Check if the code offset is larger than the function code size
    if (offset >= sectionSize)
    {
        m_javaJncReader.Close();
        return false;
    }

    // In Java, the symbol size is basically the same as
    // the size of the code section of the jitted module.
    m_currentSymbolIterator->m_size = sectionSize;
    m_codeSize         = sectionSize;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_isLongMode            = m_javaJncReader.Is64Bit();
    m_funOffsetLinenumMap.clear();

    int version = 0;
    version = m_javaJncReader.GetJNCVersion();

    if (version <= 0x3)
    {
        // This is for JNC version <= 3
        m_funOffsetLinenumMap = m_javaJncReader.GetOffsetLines();
    }
    else
    {
        gtString funcName = m_pDisplayedFunction->getFuncName();
        wstring javaFuncName;

        int pos = funcName.reverseFind(L"::");

        if (pos != -1)
        {
            gtString retStr;
            funcName.getSubString(pos + 2, funcName.length() - 1, retStr);
            javaFuncName = wstring(retStr.asCharArray());
        }
        else
        {
            javaFuncName = wstring(funcName.asCharArray());
        }

        m_funOffsetLinenumMap = m_javaJncReader.GetOffsetLines(javaFuncName);
    }

#else
    m_isLongMode            = true;
    m_funOffsetLinenumMap.clear();

    bool isNativeMethod = (m_pDisplayedFunction->getSourceFile() == L"Unknown Source File");

    if (false == isNativeMethod)
    {
        gtString funcName = m_pDisplayedFunction->getFuncName();
        wstring javaFuncName;

        int pos = funcName.reverseFind(L"::", -1);

        if (-1 != pos)
        {
            gtString retStr;
            funcName.getSubString(pos + 2, funcName.length() - 1, retStr);
            javaFuncName = wstring(retStr.asCharArray());
        }
        else
        {
            javaFuncName = wstring(funcName.asCharArray());
        }

        m_funOffsetLinenumMap = m_javaJncReader.GetOffsetLines(javaFuncName);
    }

#endif

    return true;
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
bool SourceCodeTreeModel::SetupSourceInfoForClr(gtVAddr address)
{
    GT_UNREFERENCED_PARAMETER(address);

    // Initialize the symbol engine
    if (!InitializeSymbolEngine())
    {
        return false;
    }

    // If the JNC file does not change, we can just return
    QString jncFilePath(m_sessionDir + "/" + acGTStringToQString(m_pDisplayedFunction->getJncFileName()));

    wchar_t modName[OS_MAX_PATH];

    if (m_clrJncReader.GetModuleName(modName, OS_MAX_PATH) && jncFilePath.compare(QString::fromWCharArray(modName)) == 0)
    {
        return true;
    }

    // In case the JNC file changes, we open the new JNC file
    m_pCode = nullptr;
    m_clrJncReader.Close();

    // Try open the CLR JNC file
    if (!m_clrJncReader.Open(jncFilePath.toStdWString().c_str()))
    {
        return false;
    }

    gtRVAddr clrSymOffset;

    if (!GetClrOffsetFromSymbol(clrSymOffset))
    {
        return false;
    }

    // Update Source file
    SourceLineInfo srcLine;

    if (!m_pExecutable->GetSymbolEngine()->FindSourceLine(clrSymOffset, srcLine))
    {
        return false;
    }

    // Get code pointer and size
    unsigned int codeOffset = 0;
    m_pCode = m_clrJncReader.GetCodeBytesOfTextSection(&codeOffset, &m_codeSize);

    if (m_pCode == nullptr)
    {
        m_clrJncReader.Close();
        return false;
    }

    // In CLR, the symbol size is the same as the size of the code section
    m_currentSymbolIterator->m_size = m_codeSize;

    m_isLongMode = m_clrJncReader.Is64Bit();

    // In CLR, we get the source line information from the symbol engine.
    m_funOffsetLinenumMap.clear();
    GetSourceLineInfoForCLR(clrSymOffset, m_funOffsetLinenumMap);

    m_srcFile.clear();
    m_srcFile = QString::fromWCharArray(srcLine.m_filePath);
    return true;
}
#endif
bool SourceCodeTreeModel::SetupSourceInfoForUnManaged()
{
    if (m_currentSymbolIterator == m_symbolsInfoList.end())
    {
        return false;
    }

    if (CA_NO_SYMBOL == m_currentSymbolIterator->m_name)
    {
        return false;
    }

    m_srcFile.clear();

    SymbolEngine* pSymbolEngine = nullptr;

    if (nullptr != m_pExecutable)
    {
        pSymbolEngine = m_pExecutable->GetSymbolEngine();
    }

    if (nullptr != pSymbolEngine)
    {
        // If has source info and not yet populated the line info map, we populate the line info map.

        SourceLineInfo srcData;

        if (pSymbolEngine->FindSourceLine(static_cast<gtRVAddr>(m_loadAddr - m_pModule->getBaseAddr()), srcData))
        {
            SrcLineInstanceMap srcLineInstances;
            pSymbolEngine->EnumerateSourceLineInstances(srcData.m_filePath, srcLineInstances);

            if (!srcLineInstances.empty())
            {
                if (SourceLineInstancesToOffsetLinenumMap(&srcLineInstances))
                {
                    int filePathLen = static_cast<int>(wcslen(srcData.m_filePath));
                    gtString convertedPath;

                    if (afUtils::ConvertCygwinPath(srcData.m_filePath, filePathLen, convertedPath))
                    {
                        m_srcFile = acGTStringToQString(convertedPath);
                    }
                    else
                    {
                        m_srcFile = QString::fromWCharArray(srcData.m_filePath, filePathLen);
                    }
                }
            }
        }
    }

    return true;
}


bool SourceCodeTreeModel::InitializeSymbolEngine()
{
    bool retVal = false;

    if (nullptr != m_pExecutable)
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
    }

    switch (m_moduleType)
    {
        case CpuProfileModule::UNMANAGEDPE:
        case CpuProfileModule::OCLMODULE:
        {
            // Get the cached binary path:
            QString exePath;

            CpuProfileModule* pModule = m_pProfileReader->getModuleDetail(acQStringToGTString(m_moduleName));

            if (nullptr != pModule && AuxGetExecutablePath(exePath, *m_pProfileReader, m_sessionDir, m_moduleName, nullptr, pModule))
            {
                m_pExecutable = ExecutableFile::Open(exePath.toStdWString().c_str(), pModule->getBaseAddr());
            }
            else
            {
                m_pExecutable = nullptr;
            }

            if (m_pExecutable != nullptr)
            {
                // Initialize the symbols engine:
                retVal = AuxInitializeSymbolEngine(m_pExecutable);
            }
            else
            {
                exePath.replace('/', PATH_SLASH);
                QString msg = QString(CP_sourceCodeErrorCouldNotOpenFile).arg(exePath);
                acMessageBox::instance().information(AF_STR_ErrorA, msg);
                retVal = false;
            }
        }
        break;

        case CpuProfileModule::JAVAMODULE:
        {
            retVal = true;
        }
        break;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        case CpuProfileModule::MANAGEDPE:
        {
            QString clrModulePath = m_moduleName;

            if (clrModulePath.contains(".jit"))
            {
                QString exePath;

                CpuProfileModule* pModule = m_pProfileReader->getModuleDetail(acQStringToGTString(clrModulePath));

                clrModulePath.remove(QChar('\0')).chop(4);

                if (nullptr != pModule && AuxGetExecutablePath(exePath, *m_pProfileReader, m_sessionDir, clrModulePath, nullptr, pModule))
                {
                    m_pExecutable = ExecutableFile::Open(exePath.toStdWString().c_str(), pModule->getBaseAddr());
                }
                else
                {
                    if (exePath.isEmpty())
                    {
                        exePath = clrModulePath;
                    }

                    m_pExecutable = nullptr;
                }

                if (m_pExecutable != nullptr)
                {
                    // Initialize the symbols engine:
                    retVal = AuxInitializeSymbolEngine(m_pExecutable);
                }
                else
                {
                    exePath.replace('/', PATH_SLASH);
                    QString msg = QString(CP_sourceCodeErrorCouldNotOpenFile).arg(exePath);
                    acMessageBox::instance().information(AF_STR_ErrorA, msg);
                    retVal = false;
                }
            }
        }
        break;
#endif // AMDT_WINDOWS_OS

        default:
            break;
    }

    return retVal;
}

void SourceCodeTreeModel::SetTreeSamples(const QString& hotSpotCaption)
{
    int hotSpotDataVectorIndex = -1;

    // Look for the position of the hot spot indicator data within the data vector:
    for (int i = 0; i < (int)m_pSessionDisplaySettings->m_availableDataFullNames.size(); i++)
    {
        if (hotSpotCaption == m_pSessionDisplaySettings->m_availableDataFullNames[i])
        {
            hotSpotDataVectorIndex = i;
            break;
        }
    }

    // Set the tooltip for the samples and % samples columns with the matching hot spot indicator:
    QString samplesTooltip = QString(CP_colCaptionSamplesTooltip).arg(hotSpotCaption);
    QString samplesPrecentTooltip = QString(CP_colCaptionSamplesPercentTooltip).arg(hotSpotCaption);

    m_headerTooltips[SOURCE_VIEW_SAMPLES_COLUMN] = samplesTooltip;
    m_headerTooltips[SOURCE_VIEW_SAMPLES_PERCENT_COLUMN] = samplesPrecentTooltip;

    GT_IF_WITH_ASSERT(hotSpotDataVectorIndex >= 0)
    {
        // Go through each of the tree items and set the relevant value to the samples node:
        SourceViewTreeItemMap::iterator iter = m_sourceTreeItemsMap.begin();
        SourceViewTreeItemMap::iterator iterEnd = m_sourceTreeItemsMap.end();

        CpuEvent cluUtilEv;

        bool isHotSpotCluPercent = false;

        if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
        {
            EventsFile* pEventsFile = m_pSessionDisplaySettings->getEventsFile();

            if (pEventsFile != nullptr)
            {
                pEventsFile->FindEventByValue(DE_IBS_CLU_PERCENTAGE, cluUtilEv);
            }
        }

        if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
        {
            if (hotSpotCaption.compare(cluUtilEv.name) == 0)
            {
                isHotSpotCluPercent = true;
            }
        }

        // Update the progress bar + dialog:
        afProgressBarWrapper::instance().setProgressDetails(CP_sourceCodeViewProgressSamplesUpdate, m_sourceTreeItemsMap.size());


        for (; iter != iterEnd; ++iter)
        {
            afProgressBarWrapper::instance().incrementProgressBar();

            const SourceLineAsmInfo& info = iter->first;
            SourceViewTreeItem* pItem = iter->second;
            GT_IF_WITH_ASSERT(pItem != nullptr)
            {
                const gtVector<float>& dataVector = m_sourceLinesToDataMap[info];

                // Get the global floating point precision:
                int precision = isHotSpotCluPercent ?  afGlobalVariablesManager::instance().percentagePointPrecision() :
                                afGlobalVariablesManager::instance().floatingPointPrecision();

                //[BUG433340 ]Percentage tooltip needs to be prepared for valid items in the table
                //Run the percentage calculation and set the tool tip for all columns.
                for (int i = 0; i < (int)m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size() ; i++)
                {
                    // The following if block should only be executed for lines that have meaningful data for the hotspot indicator column
                    // This condition may fail, for example for dasm lines that contain the string "---- break ----"
                    if ((hotSpotDataVectorIndex < (int)dataVector.size()) && (hotSpotDataVectorIndex < (int)m_currentFunctionTotalDataVector.size()))
                    {
                        // If the value is zero, we don't draw anything
                        float percentFromFunction = 0.0;
                        float percentFromModule = 0.0;
                        float floatVal = dataVector.at(i);
                        float totalFromFunctionVal = m_currentFunctionTotalDataVector.at(i);
                        float totalFromModuleVal = m_currentModuleTotalDataVector.at(i);

                        if (floatVal <= 0)
                        {
                            if (i == hotSpotDataVectorIndex)
                            {
                                pItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, "");
                                pItem->setForeground(SOURCE_VIEW_SAMPLES_COLUMN, acRED_NUMBER_COLOUR);

                                pItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, "");
                                pItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
                            }

                            continue;
                        }

                        if (fmod(floatVal, (float)1.0) == 0.0)
                        {
                            precision = 0;
                        }

                        // Get the data for the current cell:
                        QString strPrecision = QString::number(floatVal, 'f', precision);
                        double valuePrecision = strPrecision.toDouble();

                        if (totalFromFunctionVal > 0)
                        {
                            percentFromFunction = valuePrecision / (double)totalFromFunctionVal * 100;
                        }

                        if (totalFromModuleVal > 0)
                        {
                            percentFromModule = valuePrecision / (double)totalFromModuleVal * 100;
                        }

                        QVariant tooltipVar;
                        QString tooltipStr;
                        tooltipStr.sprintf(CP_sourceCodeViewFunctionsPercentageTooltip, percentFromFunction, percentFromModule);
                        tooltipVar.setValue(tooltipStr);

                        if (i == hotSpotDataVectorIndex)
                        {
                            QVariant dataVariant;
                            dataVariant.setValue(valuePrecision);
                            QVariant dataVariantPercent;
                            dataVariantPercent.setValue(percentFromFunction);

                            if (isHotSpotCluPercent)
                            {
                                dataVariant.setValue(dataVariant.toString().append("%"));
                            }

                            pItem->setData(SOURCE_VIEW_SAMPLES_COLUMN, dataVariant);
                            pItem->setForeground(SOURCE_VIEW_SAMPLES_COLUMN, acRED_NUMBER_COLOUR);

                            pItem->setData(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, dataVariantPercent);
                            pItem->setForeground(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, acRED_NUMBER_COLOUR);
                            pItem->setTooltip(SOURCE_VIEW_SAMPLES_COLUMN, tooltipVar);
                            pItem->setTooltip(SOURCE_VIEW_SAMPLES_PERCENT_COLUMN, tooltipVar);
                        }

                        //Set the tool tip from first dynamic event (after SOURCE_VIEW_SAMPLES_PERCENT_COLUMN) item
                        int treeColumnIndex = i + SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;
                        pItem->setTooltip(treeColumnIndex, tooltipVar);
                    }

                }
            }
        }

        afProgressBarWrapper::instance().hideProgressBar();

    }
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

bool SourceCodeTreeModel::SetDataValues(int lineNumber, int asmLineNumber, const gtVector<float>& dataVector)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
    {
        // Get the table item for the code bytes item:
        SourceLineAsmInfo info(lineNumber, asmLineNumber);
        SourceViewTreeItem* pItem = m_sourceTreeItemsMap[info];

        if (pItem != nullptr)
        {
            // Aggregate this line values to the total vector data:
            if (m_currentFunctionTotalDataVector.size() < dataVector.size())
            {
                for (int i = m_currentFunctionTotalDataVector.size(); i < (int)dataVector.size(); i++)
                {
                    m_currentFunctionTotalDataVector.push_back(0);
                }
            }

            if ((asmLineNumber < 0) || m_isDisplayingOnlyDasm)
            {
                // Accumulate only the samples of the line numbers (otherwise the total vector is doubled):
                CPUProfileUtils::AddDataArrays(m_currentFunctionTotalDataVector, dataVector);
            }

            SourceLineAsmInfo asmInfo(lineNumber, asmLineNumber);

            SourceLineToCodeBytesMap::const_iterator itCodeBytes = m_sourceLineToCodeBytesMap.find(asmInfo);

            if (m_sourceLineToCodeBytesMap.end() != itCodeBytes)
            {
                pItem->setData(SOURCE_VIEW_CODE_BYTES_COLUMN, itCodeBytes->second);
            }
            else
            {
                pItem->setData(SOURCE_VIEW_CODE_BYTES_COLUMN, "");
            }

            pItem->setForeground(SOURCE_VIEW_CODE_BYTES_COLUMN, acQGREY_TEXT_COLOUR);

            retVal = true;

            CpuEvent cluUtilEv;

            if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
            {
                EventsFile* pEventsFile = m_pSessionDisplaySettings->getEventsFile();

                if (pEventsFile != nullptr)
                {
                    pEventsFile->FindEventByValue(DE_IBS_CLU_PERCENTAGE, cluUtilEv);
                }
            }

            // Go through that added table items and set each cell's data:
            for (int i = 0; i < (int)m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size() ; i++)
            {
                bool appendPercent = false;

                if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
                {
                    if (m_pSessionDisplaySettings->m_availableDataColumnCaptions[i].contains(cluUtilEv.abbrev))
                    {
                        appendPercent = true;
                    }
                }

                // Get the tree column index:
                int treeColumnIndex = i + SOURCE_VIEW_SAMPLES_PERCENT_COLUMN + 1;

                int indexInDataVector = m_pSessionDisplaySettings->m_displayedDataColumnsIndices[i];

                if (indexInDataVector >= (int)dataVector.size())
                {
                    continue;
                }

                // If the value is zero, we don't draw anything:
                if (dataVector.at(indexInDataVector) <= 0)
                {
                    pItem->setData(treeColumnIndex, "");
                    continue;
                }

                // Get the global floating point precision:

                int onePrecision = appendPercent ?  afGlobalVariablesManager::instance().percentagePointPrecision() :
                                   afGlobalVariablesManager::instance().floatingPointPrecision();

                if (fmod(dataVector.at(indexInDataVector), (float)1.0) == 0.0)
                {
                    onePrecision = 0;
                }

                // This will display text with no decimal point if unneeded
                QString val = QString::number(dataVector.at(indexInDataVector), 'f', onePrecision);

                QVariant unitTest;

                if (onePrecision > 0)
                {
                    unitTest = val.toDouble();
                }
                else
                {
                    unitTest = val.toULongLong();
                }

                if (appendPercent)
                {
                    unitTest.setValue(unitTest.toString().append("%"));
                }

                pItem->setData(treeColumnIndex, unitTest);
            }
        }
    }

    return retVal;
}

bool SourceCodeTreeModel::SourceLineInstancesToOffsetLinenumMap(SrcLineInstanceMap* pInstances)
{
    // For sanity
    m_funOffsetLinenumMap.clear();

    if (m_currentSymbolIterator == m_symbolsInfoList.end())
    {
        return false;
    }

    for (SrcLineInstanceMap::iterator iter = pInstances->begin(), itEnd = pInstances->end(); iter !=  itEnd; ++iter)
    {
        gtVAddr lineRVA = iter->first;
        gtUInt32 lineNum = iter->second;
        gtVAddr funOffset = m_currentSymbolIterator->m_va - m_pModule->getBaseAddr();
        gtVAddr funEndOffset = funOffset + m_currentSymbolIterator->m_size;

        if (lineRVA < funOffset)
        {
            continue;
        }

        if (lineRVA > funEndOffset)
        {
            break;
        }

        m_funOffsetLinenumMap[lineRVA - funOffset] = lineNum;
    }

    return true;
}

void SourceCodeTreeModel::BuildSourceLinesTree()
{
    SourceViewTreeItem* pLineItem = nullptr;

    m_sourceTreeItemsMap.clear();
    m_sourceLineToTreeItemsMap.clear();

    for (gtUInt32 line = m_startLine ; line <= m_stopLine; line++)
    {
        pLineItem = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_LINE_DEPTH, m_pRootItem);


        // Adding the pLineItem to the map.
        SourceLineAsmInfo info(line, -1);
        m_sourceTreeItemsMap[info] = pLineItem;
        m_sourceLineToTreeItemsMap[pLineItem] = info;

        // NOTE: Cache index start from 0 but line start from 1.
        QString lineStr = m_srcLinesCache[line - 1];

        // Add source:
        if (line <= (gtUInt32)m_srcLinesCache.size())
        {
            pLineItem->setData(SOURCE_VIEW_SOURCE_COLUMN, lineStr.replace("\t", "    "));
        }

        // Add line number:
        pLineItem->setData(SOURCE_VIEW_LINE_COLUMN, QVariant((uint) line));
        pLineItem->setForeground(SOURCE_VIEW_LINE_COLUMN, AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
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


void SourceCodeTreeModel::CreateSymbolInfoList()
{
    switch (m_moduleType)
    {
        case CpuProfileModule::UNMANAGEDPE:
        case CpuProfileModule::OCLMODULE:
        {
#ifdef TBI
            bool bPreJITMod = IsPreJITModule(m_moduleName);
            bool bPJSFileExist = false;
            QString t_PJSFileName;

            if (bPreJITMod)
            {
                t_PJSFileName = m_sessionDir;
                t_PJSFileName.append(PATH_SLASH);
                t_PJSFileName.append(m_moduleName.section(PATH_SLASH, -1));
                t_PJSFileName.remove(QChar('\0'));
                t_PJSFileName.append(".pjs");

                bPJSFileExist = QFile::exists(t_PJSFileName);
            }

            if (bPreJITMod && bPJSFileExist)
            {
                bool rc = SetupSymbolInfoNgen(t_PJSFileName);
                GT_ASSERT(rc);
            }
            else
            {
#endif
                bool rc = SetupSymbolInfoListUnmanaged();
                GT_ASSERT(rc);
#ifdef TBI
            }

#endif
        }
        break;

        case CpuProfileModule::JAVAMODULE:
        case CpuProfileModule::MANAGEDPE:
            SetupSymbolInfoListManaged();
            break;

        default:
            break;
    }
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

bool SourceCodeTreeModel::GetSourceLineInfoForCLR(gtUInt32 clrSymOffset, OffsetLinenumMap& jitLineMap)
{
    //get map of il to native code
    const COR_DEBUG_IL_TO_NATIVE_MAP* pIlNativeMap = m_clrJncReader.GetILNativeMapInfo();

    if (!pIlNativeMap)
    {
        return false;
    }

    const COR_DEBUG_IL_TO_NATIVE_MAP* pILLineMap = pIlNativeMap;

    unsigned int ilNativeMapCount = m_clrJncReader.GetILNativeMapCount();

    jitLineMap.clear();

    //read line number map
    // Note: the key of map is the offset, the data is line number
    gtUInt32 minline = 0xFFFFFFFF;
    gtUInt32 maxline = 0;
    gtUInt32 previousLine = 1;
    gtUInt32 ilOffset = clrSymOffset;
    gtUInt32 ilPrevious = clrSymOffset;
    gtUInt32 maxNativeOffset = 0;

    for (unsigned int mapI = 0; mapI < ilNativeMapCount; mapI++, pILLineMap++)
    {
        switch (pILLineMap->ilOffset)
        {
            case NO_MAPPING:
            case PROLOG:
                //negative, map to first or previous source line for source file purposes
                ilOffset = ilPrevious;
                break;

            case EPILOG:
                //should use previous line's offset, as last bit
                break;

            default:
                ilOffset = clrSymOffset + pILLineMap->ilOffset;
                break;
        }

        ilPrevious = ilOffset;

        // this is the IL that address belongs to
        SourceLineInfo srcLine;

        if (m_pExecutable->GetSymbolEngine()->FindSourceLine(ilOffset, srcLine))
        {
            if (CLR_HIDDEN_LINE == srcLine.m_line)
            {
                srcLine.m_line = previousLine;
            }

            previousLine = srcLine.m_line;
            jitLineMap[pILLineMap->nativeStartOffset] = srcLine.m_line;

            if (srcLine.m_line < minline)
            {
                minline = srcLine.m_line;
            }

            if (srcLine.m_line > maxline)
            {
                maxline = srcLine.m_line;
            }
        }
        else
        {
            srcLine.m_line = previousLine;
        }

        if (pILLineMap->nativeEndOffset > maxNativeOffset)
        {
            maxNativeOffset = pILLineMap->nativeEndOffset;
        }
    }

    return true;
}

#endif

QModelIndex SourceCodeTreeModel::indexOfItem(SourceViewTreeItem* pItem)
{
    int row = -1;
    int col = 0;
    int asmLine = -1;

    SourceViewTreeItemMap::iterator iter = m_sourceTreeItemsMap.begin();
    SourceViewTreeItemMap::iterator iterEnd = m_sourceTreeItemsMap.end();

    for (int i = 0 ; iter != iterEnd; iter++, i++)
    {
        if (iter->second == pItem)
        {
            row = iter->first.m_sourceLineNumber - 1;
            asmLine = iter->first.m_asmLineNumber;
            break;
        }
    }

    if (asmLine >= 0)
    {
        row = asmLine;
        col = 1;
    }

    return createIndex(row, col, pItem);
}

void SourceCodeTreeModel::SetSingleItemDataValue(SourceViewTreeItem* pItem, int column, bool appendPercent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
        // Find the info matching this item:
        SourceLineAsmInfo info(-1, -1);
        bool wasItemFound = false;

        // Find the matching source line for this item:
        wasItemFound = m_sourceLineToTreeItemsMap.contains(pItem);

        if (wasItemFound)
        {
            info = m_sourceLineToTreeItemsMap[pItem];
            wasItemFound = true;
        }

        if (wasItemFound)
        {
            // Find the data value for this item:
            const gtVector<float>& dataVector = m_sourceLinesToDataMap[info];
            int precision = appendPercent ?  afGlobalVariablesManager::instance().percentagePointPrecision() :
                            afGlobalVariablesManager::instance().floatingPointPrecision();

            // Get the data for the column:
            int positionInDataVector = column - SOURCE_VIEW_SAMPLES_PERCENT_COLUMN - 1;

            // The following if block should only be executed for lines that have meaningful data for the counter columns
            // This condition may fail, for example for dasm lines that contain the string "---- break ----"
            if ((positionInDataVector >= 0) && (positionInDataVector < (int)dataVector.size()))
            {
                float valueToSetOnItem = dataVector[positionInDataVector];

                if (valueToSetOnItem > 0)
                {
                    // Set the item delegate for the current column:
                    bool isProfilingCLU = m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU;
                    bool displayPercentageInColumn = CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn && (!isProfilingCLU);

                    if (displayPercentageInColumn)
                    {
                        gtVector<int>::const_iterator valsItBegin = m_pSessionDisplaySettings->m_simpleValuesVector.begin();
                        gtVector<int>::const_iterator valsItEnd = m_pSessionDisplaySettings->m_simpleValuesVector.end();

                        gtVector<int>::const_iterator findIt = gtFind(valsItBegin, valsItEnd, positionInDataVector);

                        if (valsItEnd != findIt)
                        {
                            int totalIndex = m_pSessionDisplaySettings->m_totalValuesMap[positionInDataVector];

                            if (totalIndex < (int)m_currentFunctionTotalDataVector.size())
                            {
                                if (m_currentFunctionTotalDataVector.at(totalIndex) > 0)
                                {
                                    valueToSetOnItem = (valueToSetOnItem * 100.0) / m_currentFunctionTotalDataVector.at(totalIndex);
                                }

                                precision = afGlobalVariablesManager::instance().floatingPointPrecision();

                                if (fmod(valueToSetOnItem, (float)1.0) == 0.0)
                                {
                                    precision = 0;
                                }
                            }
                        }
                    }

                    QVariant data = QVariant(QString::number(valueToSetOnItem, 'f', precision).toDouble());

                    if (appendPercent)
                    {
                        data.setValue(data.toString().append("%"));
                    }

                    // Set the data on the item:
                    pItem->setData(column, data);
                }
            }
        }
    }
}

SourceViewTreeItem* SourceCodeTreeModel::AddDoubleClickItem(SourceViewTreeItem* pAsmItem)
{
    SourceViewTreeItem* pRetVal = nullptr;
    // Sanity check:
    GT_IF_WITH_ASSERT(pAsmItem != nullptr)
    {
        pRetVal = new SourceViewTreeItem(m_pSessionDisplaySettings, SOURCE_VIEW_ASM_DEPTH, pAsmItem);


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

void SourceCodeTreeModel::CalculateTotalModuleCountVector(CpuProfileReader* pProfileReader)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (pProfileReader != nullptr))
    {
        // Aggregate this line values to the total vector data:
        if (m_currentModuleTotalDataVector.size() < m_pSessionDisplaySettings->m_availableDataColumnCaptions.size())
        {
            for (int i = m_currentModuleTotalDataVector.size(); i < (int)m_pSessionDisplaySettings->m_availableDataColumnCaptions.size(); i++)
            {
                m_currentModuleTotalDataVector.push_back(0);
            }
        }

        // Fill the vector with zeros:
        std::fill(m_currentModuleTotalDataVector.begin(), m_currentModuleTotalDataVector.end(), 0.0f);

        NameModuleMap* pModulesMap = pProfileReader->getModuleMap();
        GT_IF_WITH_ASSERT(pModulesMap != nullptr)
        {
            NameModuleMap::const_iterator moduleIter = pModulesMap->begin();
            NameModuleMap::const_iterator moduleIterEnd = pModulesMap->end();

            for (; moduleIter != moduleIterEnd; ++moduleIter)
            {
                // Get the current module:
                const CpuProfileModule* pModule = &(moduleIter->second);

                if (pModule != nullptr)
                {
                    // Iterate the functions in the current module, and accumulate it's total values:
                    AddrFunctionMultMap::const_iterator functionIter = pModule->getBeginFunction();
                    AddrFunctionMultMap::const_iterator functionEndIter = pModule->getEndFunction();

                    for (; functionIter != functionEndIter; ++functionIter)
                    {
                        // Get the current function:
                        const CpuProfileFunction& function = functionIter->second;

                        // Iterate the function samples and accumulate their sample count for each of the displayed counters:
                        AptAggregatedSampleMap::const_iterator aggregatedSampleIter = function.getBeginSample();
                        AptAggregatedSampleMap::const_iterator aggregatedSampleIterEnd = function.getEndSample();

                        for (; aggregatedSampleIter != aggregatedSampleIterEnd; aggregatedSampleIter++)
                        {
                            if (((SHOW_ALL_PIDS != m_pid) && (aggregatedSampleIter->first.m_pid != m_pid)) || ((SHOW_ALL_TIDS != m_tid) && (aggregatedSampleIter->first.m_tid != m_tid)))
                            {
                                continue;
                            }

                            CpuProfileSampleMap::const_iterator sampleIt = aggregatedSampleIter->second.getBeginSample();

                            for (; sampleIt != aggregatedSampleIter->second.getEndSample(); sampleIt++)
                            {
                                SampleKeyType key(sampleIt->first.cpu, sampleIt->first.event);

                                // If this event should not be shown with the current settings, skip it:
                                if (!m_pSessionDisplaySettings->m_eventToIndexMap.contains(key))
                                {
                                    continue;
                                }

                                //given cpu/event select from profile, find column index
                                int index = m_pSessionDisplaySettings->m_eventToIndexMap[key];

                                // The aggregation won't affect the complex columns, since the raw data would be 0:

                                // If this is a real line, aggregate it:
                                m_currentModuleTotalDataVector[index] += sampleIt->second;
                            }
                        }
                    }
                }
            }
        }
    }
}

void SourceCodeTreeModel::SetModuleDetails(const CpuProfileModule* pModule)
{
    // Sanity Check:
    GT_IF_WITH_ASSERT((pModule != nullptr) && (m_pSessionDisplaySettings != nullptr) && (m_pSessionDisplaySettings->m_pProfileInfo != nullptr))
    {
        // Fill in the details from the CpuProfileModule class:
        m_pModule = pModule;
        m_moduleType = (CpuProfileModule::MOD_MODE_TYPE) m_pModule->m_modType;
        m_moduleTotalSamplesCount = m_pModule->getTotal();
        m_moduleName = acGTStringToQString(m_pModule->getPath());

        m_sessionTotalSamplesCount = m_pSessionDisplaySettings->m_pProfileInfo->m_numSamples;
    }
}
