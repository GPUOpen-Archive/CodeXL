//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPUProfileDataTable.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acTableWidgetItem.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>
#include <AMDTCpuPerfEventUtils/inc/IbsEvents.h>
#include <AMDTCpuPerfEventUtils/inc/EventsFile.h>

/// Local:
#include <inc/CPUProfileDataTable.h>
#include <inc/CPUProfileUtils.h>
#include <inc/DebugUtils.h>
#include <inc/StdAfx.h>
#include <inc/Auxil.h>
#include <inc/StringConstants.h>
#include <inc/CPUProfileDataTableItem.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

#define MAX_COLUMN_WIDTH 350

// Static members:
gtPtrVector<QPixmap*> CPUProfileDataTable::m_sTableIcons;
bool CPUProfileDataTable::m_sIconsInitialized = false;
bool CPUProfileDataTable::m_CLUNoteShown = true;


CPUProfileDataTable::CPUProfileDataTable(QWidget* pParent, const gtVector<CPUProfileDataTable::TableContextMenuActionType>& additionalContextMenuActions, SessionTreeNodeData* pSessionData)
    : acListCtrl(pParent, CP_CPU_TABLE_ROW_HEIGHT), m_pTableDisplaySettings(nullptr), m_pSessionDisplaySettings(nullptr),
      m_pProfileReader(nullptr), m_pDisplaySessionData(pSessionData), m_pCLUDelegate(nullptr), m_pEmptyRowTableItem(nullptr),
      m_pOtherSamplesRowItem(nullptr)
{
    setSortingEnabled(true);
    setEnablePaste(false);
    m_totalSampleCount = 0;

    // Copy cell items with captions:
    m_shouldCopyColumnHeaders = true;

    // Extend the context menu with the requested actions:
    extendContextMenu(additionalContextMenuActions);

    // Connect the sort click slot:
    bool rc = connect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(sortIndicatorChanged(int, Qt::SortOrder)));

    m_tableRowHasIcon = false;

    for (int i = 0; i < CPU_TABLE_MAX_VALUES; i++)
    {
        m_elapsedTime[i] = 0;
        m_startTime[i] = 0;
    }

    GT_ASSERT(rc);
}

CPUProfileDataTable::~CPUProfileDataTable()
{

}

bool CPUProfileDataTable::fillListData()
{
    // Get the item for the empty table message:
    int newRowIndex = rowCount();

    // set row with empty row message
    QStringList list;

    for (int i = 0; i < columnCount(); ++i)
    {
        if (i == 0)
        {
            list << CP_emptyTableMessage;
        }
        else
        {
            list << "";
        }
    }

    addRow(list, nullptr);

    // save item of this row
    m_pEmptyRowTableItem = item(newRowIndex, 0);

    // hide the row
    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        setRowHidden(newRowIndex, true);
    }

    // add row with data about samples not in module - 6th row for top5 table ("others" row)
    QStringList samplesList;

    for (int i = 0; i < columnCount(); ++i)
    {
        samplesList << "";
    }

    newRowIndex = rowCount();
    addRow(samplesList, nullptr);

    // set all row items with "other" row data role
    m_pOtherSamplesRowItem = item(newRowIndex, 0);
    GT_IF_WITH_ASSERT(m_pOtherSamplesRowItem != nullptr)
    {
        m_pOtherSamplesRowItem->setData(AC_USER_ROLE_OTHER_ROW, CPUProfileDataTableItem::ASCENDING_ORDER);

        QTableWidgetItem* rowItem;

        for (int i = 1; i < columnCount(); ++i)
        {
            rowItem = nullptr;
            rowItem = item(newRowIndex, i);

            if (rowItem != nullptr)
            {
                rowItem->setData(AC_USER_ROLE_OTHER_ROW, CPUProfileDataTableItem::ASCENDING_ORDER);
            }
        }

        // save item
        setRowHidden(newRowIndex, true);
    }

    return true;
}

bool CPUProfileDataTable::displayProfileData(CpuProfileReader* pProfileReader)
{
    bool retVal = false;

    BEGIN_TICK_COUNT(DisplayProfileData);

    GT_IF_WITH_ASSERT(pProfileReader != nullptr)
    {
        // Set the profile reader:
        m_pProfileReader = pProfileReader;

        // Clear table items:
        clear();
        clearContents();
        setColumnCount(0);
        setRowCount(0);
        m_totalSampleCount = 0;
        m_hotSpotCellsMap.clear();

        // Set the headers:
        bool rcHeaders = true;

        if (horizontalHeader()->count() == 0)
        {
            rcHeaders = initializeListHeaders();
            GT_ASSERT(rcHeaders);
        }

        // Fill the list data:
        bool rcData = fillListData();
        GT_ASSERT(rcData);

        horizontalHeader()->setSortIndicatorShown(false);

        // Perform post process operations on the table:
        bool rcPost = setHotSpotIndicatorValues();
        GT_ASSERT(rcPost);

        // Hide the filtered columns specified in display filter:
        hideFilteredColumns();

        // Perform post process operations on the table:
        bool rcPercent = true;
        rcPercent = setPercentValues();
        GT_ASSERT(rcPercent);

        // Sort the table:
        sortTable();

        // Make sure that the CLU percent values are displayed as percent:
        setCLUPercentValues();

        retVal = rcHeaders && rcData && rcPost && rcPercent;
    }

    END_TICK_COUNT(DisplayProfileData);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: displayProfileData (%u ms)", m_elapsedTime[DisplayProfileData]);
#endif

    return retVal;
}

QTableWidgetItem* CPUProfileDataTable::allocateNewWidgetItem(const QString& text)
{
    // Allocate my own widget item:
    return new CPUProfileDataTableItem(text);
}

bool CPUProfileDataTable::initializeListHeaders()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {

        // Vector that contain the columns strings according to the current displayed columns:
        QStringList columnsStringByObjectType;
        QStringList columnTooltipsByObjectType;

        int tableDispSettingsColsNum = (int)m_pTableDisplaySettings->m_displayedColumns.size();

        for (int i = 0; i < tableDispSettingsColsNum; i++)
        {
            QString colStr, colTooltip;

            if ((m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL) &&
                CPUProfileUtils::IsHotSpotCluMetric(m_pSessionDisplaySettings->getEventsFile(), m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption))
            {
                colStr = m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption;

                if (CPUProfileUtils::IsHotSpotBetterHigher(m_pSessionDisplaySettings->getEventsFile(), m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption))
                {
                    colStr.append(SUFFIX_HOTSPOT_BETTER_HIGH);
                }
            }
            else
            {
                bool rc = m_pTableDisplaySettings->colTypeAsString(m_pTableDisplaySettings->m_displayedColumns[i], colStr, colTooltip);
                GT_ASSERT(rc);
            }

            columnsStringByObjectType << colStr;
            columnTooltipsByObjectType << colTooltip;
        }

        // Now add the data columns (only if this is not a hot spot display:

        bool showPercentSeperateColumns = IsShowSeperatePercentColumns();

        m_percentColsNum.clear();

        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
        {
            for (int i = 0 ; i < (int)m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size(); i++)
            {
                int index = m_pSessionDisplaySettings->m_displayedDataColumnsIndices[i];
                GT_IF_WITH_ASSERT((index >= 0) && (index < (int) m_pSessionDisplaySettings->m_availableDataColumnCaptions.size()))
                {
                    QString currentCaption = m_pSessionDisplaySettings->m_availableDataColumnCaptions[index];
                    QString currentFullName = m_pSessionDisplaySettings->m_availableDataFullNames[index];
                    QString currentDescription = m_pSessionDisplaySettings->m_availableDataColumnTooltips[index];

                    // Format the tooltip:
                    QString tooltip;
                    acWrapAndBuildFormattedTooltip(currentFullName, currentDescription, tooltip);

                    columnsStringByObjectType << currentCaption;
                    columnTooltipsByObjectType << tooltip;

                    if (showPercentSeperateColumns)
                    {
                        currentCaption.append(" percent");
                        columnsStringByObjectType << currentCaption;
                        columnTooltipsByObjectType << tooltip;
                        m_percentColsNum.append(i * 2 + 1 + tableDispSettingsColsNum);
                    }
                }
            }
        }

        // Build the columns according to the selected item type:
        initHeaders(columnsStringByObjectType, false);

        for (int i = 0, colsAmount = columnCount(); i < colsAmount; i++)
        {
            QTableWidgetItem* pHeaderItem = horizontalHeaderItem(i);
            GT_IF_WITH_ASSERT((pHeaderItem != nullptr) && (i < columnTooltipsByObjectType.size()))
            {
                pHeaderItem->setToolTip(columnTooltipsByObjectType[i]);
            }
        }

        for (int i = 0; i < m_percentColsNum.size() - 1; i++)
        {
            setColumnHidden(m_percentColsNum[i], true);
        }

    }
    return retVal;
}

bool CPUProfileDataTable::setHotSpotIndicatorValues()
{
    bool retVal = true;

    BEGIN_TICK_COUNT(SetHotSpotIndicatorValues);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        // If items should be filtered by hot spot column:
        if (m_pTableDisplaySettings->m_amountOfItemsInDisplay > 0)
        {
            GT_IF_WITH_ASSERT(!m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
            {
                // Build a map containing the hot spot values by row:
                retVal = buildHotSpotIndicatorMap();
                GT_IF_WITH_ASSERT(retVal)
                {
                    HotSpotValuesMapCompareFunctor compare;
                    gtSort(m_hotSpotCellsMap.begin(), m_hotSpotCellsMap.end(), compare);

                    if (CPUProfileUtils::IsHotSpotBetterHigher(m_pSessionDisplaySettings->getEventsFile(),
                                                               m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption))
                    {
                        std::reverse(m_hotSpotCellsMap.begin(), m_hotSpotCellsMap.end());
                    }

                    int count = 0;

                    for (int i = 0; i < (int)m_hotSpotCellsMap.size(); i++)
                    {
                        // Get the current row index and value:
                        bool isValueZero = (0.0f == m_hotSpotCellsMap[i].m_percentValue);

                        if ((count >= m_pTableDisplaySettings->m_amountOfItemsInDisplay) || isValueZero)
                        {
                            setRowHidden(m_hotSpotCellsMap[i].m_index, true);
                        }
                        else
                        {
                            setRowHidden(m_hotSpotCellsMap[i].m_index, false);
                            count++;
                        }
                    }
                }
            }
        }

        int displayedCols = (int)m_pTableDisplaySettings->m_displayedColumns.size();

        // Set the item delegates:
        for (int i = 0; i < displayedCols; i++)
        {
            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_PERCENT_COL)
            {
                acTablePercentItemDelegate* pDelegate = new acTablePercentItemDelegate();

                pDelegate->SetOwnerTable(this);
                setItemDelegateForColumn(i, pDelegate);
            }

            else if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
            {
                setItemDelegateForColumn(i, &acNumberDelegateItem::Instance());
            }
        }

        // Display a message if the table is empty:
        bool isEmpty = HandleEmptyTable();
        SetLastRowForTop5(isEmpty);

        // Resize the columns to content:
        for (int col = 0, colsAmount = columnCount(); col <= colsAmount; ++col)
        {
            // have to unhide columns before resizing them (if it hidden the resize wont work)
            setColumnHidden(col, false);

            resizeColumnToContents(col);
        }

        // if content is too big have set upto some max size, except the last column:
        for (int col = 0, colsAmount = columnCount(); col < colsAmount; ++col)
        {
            if (columnWidth(col) > MAX_COLUMN_WIDTH)
            {
                setColumnWidth(col, MAX_COLUMN_WIDTH);
            }
        }
    }

    END_TICK_COUNT(SetHotSpotIndicatorValues);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: setHotSpotIndicatorValues (%u ms)", m_elapsedTime[SetHotSpotIndicatorValues]);
#endif

    return retVal;
}

bool CPUProfileDataTable::organizeTableByHotSpotIndicator()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        // Build the hot spot indicator map and update the table values:
        retVal = HandleHotSpotIndicatorSet();

        // Vector that contain the columns strings according to the current displayed columns:
        QStringList columnTooltipsByObjectType;

        for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            QString colStr, colTooltip;
            bool rc = m_pTableDisplaySettings->colTypeAsString(m_pTableDisplaySettings->m_displayedColumns[i], colStr, colTooltip);
            GT_ASSERT(rc);
            columnTooltipsByObjectType << colTooltip;
        }

        // Update the hot spot indicator tooltip (should contain the hot spot indicator title):
        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
        {
            for (int i = 0 ; i < (int)m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size(); i++)
            {
                int index = m_pSessionDisplaySettings->m_displayedDataColumnsIndices[i];
                GT_IF_WITH_ASSERT((index >= 0) && (index < (int) m_pSessionDisplaySettings->m_availableDataColumnCaptions.size()))
                {
                    QString currentCaption = m_pSessionDisplaySettings->m_availableDataColumnCaptions[index];
                    QString currentFullName = m_pSessionDisplaySettings->m_availableDataFullNames[index];
                    QString currentDescription = m_pSessionDisplaySettings->m_availableDataColumnTooltips[index];

                    // Format the tooltip:
                    QString tooltip;
                    acWrapAndBuildFormattedTooltip(currentFullName, currentDescription, tooltip);

                    columnTooltipsByObjectType << tooltip;
                }
            }
        }

        for (int i = 0, colsAmount = columnCount(); i < colsAmount; i++)
        {
            QTableWidgetItem* pHeaderItem = horizontalHeaderItem(i);
            GT_IF_WITH_ASSERT((pHeaderItem != nullptr) && (i < columnTooltipsByObjectType.size()))
            {
                pHeaderItem->setToolTip(columnTooltipsByObjectType[i]);
            }
        }

        // Make sure that the CLU percent columns are displayed as percent data:
        setCLUPercentValues();

        // Sort the table:
        sortTable();
    }

    return retVal;

}

void CPUProfileDataTable::sortTable()
{
    int sortByColIndex = -1;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        // Check if the user already sorted, and select the user's sort column:
        if (!m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
        {
            for (int i = 0 ; i < columnCount(); i++)
            {
                // Get the current header item:
                QTableWidgetItem* pHeaderItem = horizontalHeaderItem(i);

                if (pHeaderItem != nullptr)
                {
                    if (m_pTableDisplaySettings->m_lastSortColumnCaption == horizontalHeaderItem(i)->text())
                    {
                        sortByColIndex = i;
                        break;
                    }
                }
            }
        }

        // If sort by column index was not found:
        if (sortByColIndex < 0)
        {
            if (!m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
            {
                // When displaying a hot spot indicator - sort by samples count:
                for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns[i]; i++)
                {
                    if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
                    {
                        sortByColIndex = i;
                        break;
                    }
                }
            }
            else
            {
                // Sort by first data column:
                sortByColIndex = m_pTableDisplaySettings->m_displayedColumns.size();
            }

        }

        if ((sortByColIndex >= 0) && (sortByColIndex < columnCount()))
        {
            UpdateLastRowItemsSortOrder();

            sortItems(sortByColIndex, m_pTableDisplaySettings->m_lastSortOrder);
        }
    }
}

void CPUProfileDataTable::setTableRowCellValue(int rowIndex, int colIndex, int dataIndex, const gtVector<float>& dataVector)
{
    QString currentValue;

    // Get the current value:
    if ((dataIndex >= 0) && (dataIndex < (int)dataVector.size()))
    {
        float floatVal = dataVector[dataIndex];

        // If the value is zero, we don't draw anything
        if (floatVal > 0)
        {
            int precision = afGlobalVariablesManager::instance().floatingPointPrecision();

            // TODO: calculate the complex columns here and show the given data
            if (fmod(floatVal, (float)1.0) == 0.0)
            {
                precision = 0;
            }

            // This will display text with no decimal point if unneeded
            currentValue = QString::number(floatVal, 'f', precision);

            // Get the table cell to set it's value:
            QTableWidgetItem* pTableItem = item(rowIndex, colIndex);
            GT_IF_WITH_ASSERT(pTableItem != nullptr)
            {
                // Get the data for the current cell:
                QVariant dataVariant;

                if (precision > 0)
                {
                    dataVariant.setValue(currentValue.toDouble());
                }
                else
                {
                    dataVariant.setValue(currentValue.toULongLong());
                }

                pTableItem->setData(Qt::DisplayRole, dataVariant);
                pTableItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);

                QString tooltip = dataVariant.toString();
                pTableItem->setToolTip(tooltip);

                setItemDelegateForColumn(colIndex, &acNumberDelegateItem::Instance());
            }
        }
    }
}

bool CPUProfileDataTable::setTableItemPercentValue(int rowIndex, int colIndex, int dataColIndex)
{
    bool retVal = false;

    // Get the table item for the requested column and row:
    QTableWidgetItem* pTableItem = item(rowIndex, colIndex);
    GT_IF_WITH_ASSERT((pTableItem != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        // Get the table item value:
        double doubleVal = pTableItem->data(Qt::DisplayRole).toDouble();
        float percentValue = 0.0;

        int totalIndex = m_pSessionDisplaySettings->m_totalValuesMap[dataColIndex];

        if ((totalIndex >= 0) && (totalIndex < (int)m_totalDataValuesVector.size()))
        {
            if (m_totalDataValuesVector[totalIndex] > 0)
            {
                percentValue = (doubleVal * 100.0) / m_totalDataValuesVector[totalIndex];
            }

            // If the value is zero, we don't draw anything:
            if (percentValue > 0)
            {
                QVariant dataVariant(percentValue);
                GT_ASSERT(dataVariant.isValid());

                pTableItem->setData(Qt::EditRole, dataVariant);

                QString tooltip = QString::number(dataVariant.toFloat(), 'f', 2);
                tooltip.append("%");

                pTableItem->setToolTip(tooltip);
            }

            retVal = true;
        }
    }

    return retVal;
}

void CPUProfileDataTable::setTableDisplayedColumnsValues(int rowIndex, const gtVector<float>& dataVector)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        // Look for the hot spot indicator column:
        int hotSpotColIndexInDataVector = -1;
        int samplesCountIndex = -1;
        CpuEvent cluUtilEv;

        if (m_pSessionDisplaySettings->m_pProfileInfo)
        {
            if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
            {
                {
                    EventsFile* pEventsFile = m_pSessionDisplaySettings->getEventsFile();

                    if (pEventsFile != nullptr)
                    {
                        pEventsFile->FindEventByValue(DE_IBS_CLU_PERCENTAGE, cluUtilEv);
                    }
                }
            }
        }

        if (!m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
        {
            // Find the index of the hot spot indicator within the data vector:
            QString hotSpotCaption = m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption;

            if (m_pTableDisplaySettings->m_hotSpotIndicatorToDataIndexMap.find(hotSpotCaption) != m_pTableDisplaySettings->m_hotSpotIndicatorToDataIndexMap.end())
            {
                hotSpotColIndexInDataVector = m_pTableDisplaySettings->m_hotSpotIndicatorToDataIndexMap[hotSpotCaption];
            }

            for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns[i]; i++)
            {
                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
                {
                    samplesCountIndex = i;
                    break;
                }
            }
        }

        bool showPercentSeperateColumns = IsShowSeperatePercentColumns();

        // When there is no hot spot indicator - show all columns:
        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
        {
            // Iterate the shown indices and set their values:
            unsigned int firstDataIndexColumnIndex = m_pTableDisplaySettings->m_displayedColumns.size();
            unsigned int lastDataIndexColumnIndex = firstDataIndexColumnIndex + m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size() - 1;
            float total = 0;

            for (unsigned int dataColIndex = firstDataIndexColumnIndex, realIndex = firstDataIndexColumnIndex;
                 dataColIndex <= lastDataIndexColumnIndex;
                 dataColIndex++, realIndex++)
            {
                // Set this cell value:
                int dataIndexWithinDisplayedVector = dataColIndex - m_pTableDisplaySettings->m_displayedColumns.size();
                int dataIndexWithinDataVector = m_pSessionDisplaySettings->m_displayedDataColumnsIndices[dataIndexWithinDisplayedVector];

                if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
                {
                    if ((dataIndexWithinDataVector >= 0) && (dataIndexWithinDataVector < (int)dataVector.size()))
                    {
                        total += dataVector[dataIndexWithinDataVector];
                    }
                }

                // colIndex is the real table index,
                // dataIndexWithinDataVector is the index in the data table
                setTableRowCellValue(rowIndex, realIndex, dataIndexWithinDataVector, dataVector);

                // in case of copied percent col - copy orig value from col-1
                if (showPercentSeperateColumns)
                {
                    realIndex++;
                    setTableRowCellValue(rowIndex, realIndex, dataIndexWithinDataVector, dataVector);
                }
            }

            if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU && (0.0f == total))
            {
                // Sometime functions with no CLU data appears in function table; putting this hack to
                // hide those functions; later will fix it proparly
                setRowHidden(rowIndex, true);
            }
        }
        else
        {
            if (hotSpotColIndexInDataVector >= 0)
            {
                setTableRowCellValue(rowIndex, samplesCountIndex, hotSpotColIndexInDataVector, dataVector);
            }
        }
    }
}

void CPUProfileDataTable::getRowHotSpotPosition(float percentValue, int& pos, int& numberOfCells)
{
    // Assume that the map is sorted:
    pos = -1;
    numberOfCells = m_hotSpotCellsMap.size();

    for (int i = 0 ; i < (int)m_hotSpotCellsMap.size(); i++)
    {
        float myValue = m_hotSpotCellsMap[i].m_percentValue;
        float diff = fabs(myValue - percentValue);

        if (diff < 0.0001)
        {
            pos = i;
            break;
        }
    }

}

bool CPUProfileDataTable::setPercentValues()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        gtVector<int>::const_iterator valsItBegin = m_pSessionDisplaySettings->m_simpleValuesVector.begin();
        gtVector<int>::const_iterator valsItEnd = m_pSessionDisplaySettings->m_simpleValuesVector.end();

        // When there is no hot spot indicator - show all columns:
        // Iterate the shown indices and set their values:
        unsigned int firstDataIndexColumnIndex = m_pTableDisplaySettings->m_displayedColumns.size();
        // totle number of cols = display cols _ data cols
        unsigned int lastDataIndexColumnIndex = firstDataIndexColumnIndex + m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size() - 1;

        // Do not show percentage value when profiled CLU:
        bool isProfilingCLU = false;

        if (m_pSessionDisplaySettings->m_pProfileInfo != nullptr)
        {
            isProfilingCLU = m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU;
        }

        bool displayPercentageInColumn = CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn && (!isProfilingCLU);
        int tmpColIndex = 0;

        bool showPercentSeperateColumns = IsShowSeperatePercentColumns();

        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty() &&
            displayPercentageInColumn)
        {
            // go over data columns
            // ALL data columns will be percent columns
            for (unsigned int colIndex = firstDataIndexColumnIndex, readColIndex = firstDataIndexColumnIndex;
                 colIndex <= lastDataIndexColumnIndex;
                 colIndex++, readColIndex++)
            {
                // Check if this column is a simple data column, or a complex one. For simple values we display
                // percentage. For complex we do not:
                int displayedIndex = -1;

                //index is the index of data col
                int index = colIndex - firstDataIndexColumnIndex;

                GT_IF_WITH_ASSERT((index >= 0) && (index < (int)m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size()))
                {
                    // Get the displayed index:
                    displayedIndex = m_pSessionDisplaySettings->m_displayedDataColumnsIndices[index];

                    gtVector<int>::const_iterator findIt = gtFind(valsItBegin, valsItEnd, displayedIndex);

                    if (valsItEnd != findIt)
                    {
                        bool hasDelegate = false;

                        // in case of copied percent col - in time base profiling - show the col
                        tmpColIndex = colIndex;

                        if (showPercentSeperateColumns)
                        {
                            readColIndex++;
                            tmpColIndex = readColIndex;
                        }

                        for (int rowIndex = 0, rowsAmount = rowCount(); rowIndex < rowsAmount; rowIndex++)
                        {
                            bool rc = setTableItemPercentValue(rowIndex, tmpColIndex, index);
                            GT_IF_WITH_ASSERT(rc)
                            {
                                if (!hasDelegate)
                                {
                                    hasDelegate = true;
                                    acTablePercentItemDelegate* pDelegate = new acTablePercentItemDelegate();

                                    pDelegate->SetOwnerTable(this);
                                    setItemDelegateForColumn(tmpColIndex, pDelegate);
                                }
                            }

                            retVal &= rc;
                        }
                    }
                    else
                    {
                        setItemDelegateForColumn(colIndex, &acNumberDelegateItem::Instance());
                    }
                }
            }
        }
        else if (!displayPercentageInColumn)
        {
            for (unsigned int colIndex = firstDataIndexColumnIndex; colIndex <= lastDataIndexColumnIndex; colIndex++)
            {
                setItemDelegateForColumn(colIndex, &acNumberDelegateItem::Instance());
            }
        }
    }

    return retVal;

}


QPixmap* CPUProfileDataTable::moduleIcon(const osFilePath& filePath, bool is32Bit)
{
    QPixmap* pRetVal = nullptr;

    if (!m_sIconsInitialized)
    {
        // Create the file item icons:
        QPixmap* pUser32FileIcon = new QPixmap;
        acSetIconInPixmap(*pUser32FileIcon, AC_ICON_PROFILE_CPU_MOUDLE_USER_32);

        QPixmap* pUser64FileIcon = new QPixmap;
        acSetIconInPixmap(*pUser64FileIcon, AC_ICON_PROFILE_CPU_MOUDLE_USER_64);

        QPixmap* pSystem32FileIcon = new QPixmap;
        acSetIconInPixmap(*pSystem32FileIcon, AC_ICON_PROFILE_CPU_MOUDLE_SYSTEM_32);

        QPixmap* pSystem64FileIcon = new QPixmap;
        acSetIconInPixmap(*pSystem64FileIcon, AC_ICON_PROFILE_CPU_MOUDLE_SYSTEM_64);

        // Add the icons to the list
        m_sTableIcons.push_back(pUser32FileIcon);        // 0
        m_sTableIcons.push_back(pUser64FileIcon);        // 1
        m_sTableIcons.push_back(pSystem32FileIcon);      // 2
        m_sTableIcons.push_back(pSystem64FileIcon);      // 3

        m_sIconsInitialized = true;
    }

    int iconIndex = 0;

    // Is this a system module?
    bool isSystem = AuxIsSystemModule(filePath);

    if (isSystem && is32Bit)
    {
        // System 32:
        iconIndex = 2;
    }
    else if (isSystem && !is32Bit)
    {
        // System 64:
        iconIndex = 3;
    }

    else if (!isSystem && !is32Bit)
    {
        // User 64:
        iconIndex = 1;
    }

    GT_IF_WITH_ASSERT((iconIndex >= 0) && (iconIndex < (int)m_sTableIcons.size()))
    {
        pRetVal = m_sTableIcons[iconIndex];
    }
    return pRetVal;
}

void CPUProfileDataTable::extendContextMenu(const gtVector<CPUProfileDataTable::TableContextMenuActionType>& additionalContextMenuActions)
{
    // Sanity check:
    if (!additionalContextMenuActions.empty() && (m_pContextMenu != nullptr))
    {
        QAction* pBeforeAction = nullptr;

        if (!m_pContextMenu->actions().empty())
        {
            pBeforeAction = m_pContextMenu->actions()[0];
        }

        for (int i = 0; i < (int)additionalContextMenuActions.size(); i++)
        {
            TableContextMenuActionType actionType = additionalContextMenuActions[i];

            // Get the action string:
            QString str = actionTypeToString(actionType);
            GT_IF_WITH_ASSERT(!str.isEmpty())
            {
                // Create an action for the "Add Watch" menu item:
                QAction* pAction = new QAction(str, m_pContextMenu);

                pAction->setData(QVariant((int)actionType));

                if (i == 0)
                {
                    QFont font = pAction->font();
                    font.setBold(true);
                    pAction->setFont(font);
                }

                // Add the action to the context menu:
                m_pContextMenu->insertAction(pBeforeAction, pAction);

                // Connect the action to its handler:
                bool rcConnect = connect(pAction, SIGNAL(triggered()), this, SLOT(onContextMenuAction()));
                GT_ASSERT(rcConnect);
            }
        }

        // Insert a separator:
        m_pContextMenu->insertSeparator(pBeforeAction);

        // Connect the menu to an about to show slot:
        bool rc = connect(m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowContextMenu()));
        GT_ASSERT(rc);
    }
}

void CPUProfileDataTable::onContextMenuEvent(const QPoint& position)
{
    // if no row selected - dont show context menu
    if (selectedItems().count() > 0)
    {
        acListCtrl::onContextMenuEvent(position);
    }
}

void CPUProfileDataTable::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acListCtrl::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT(m_pContextMenu != nullptr)
    {
        foreach (QAction* pAction, m_pContextMenu->actions())
        {
            if (pAction != nullptr)
            {
                if (pAction->data().isValid())
                {
                    TableContextMenuActionType actionType = (TableContextMenuActionType)pAction->data().toInt();

                    if (actionType == DISPLAY_CLU_NOTES)
                    {
                        QString strCluNotes = actionTypeToString(actionType);
                        pAction->setText(strCluNotes);
                    }

                    bool isActionEnabled = true;

                    if (actionType != DISPLAY_CLU_NOTES)
                    {
                        // All the actions but the CLU notes action are actions that are dependant on the selected items:
                        isActionEnabled = !selectedIndexes().isEmpty();
                        pAction->setEnabled(isActionEnabled);
                    }

                    if ((actionType == DISPLAY_BY_PROCESS_NAME) || (actionType == DISPLAY_BY_MODULE_NAME))
                    {
                        // Get the current selected item text:
                        QList<int> selectedRows;

                        for (const QModelIndex& index : selectedIndexes())
                        {
                            if (selectedRows.indexOf(index.row()) < 0)
                            {
                                selectedRows << index.row();
                            }
                        }

                        if (selectedRows.size() != 1)
                        {
                            pAction->setEnabled(false);
                            pAction->setVisible(false);
                        }
                        else
                        {
                            // Get the first selected item:
                            int selectedIndex = selectedRows.first();
                            QTableWidgetItem* pItem = item(selectedIndex, 0);

                            if (pItem != nullptr)
                            {
                                if (nullptr != m_pOtherSamplesRowItem && m_pOtherSamplesRowItem->row() == pItem->row())
                                {
                                    pAction->setEnabled(false);
                                }
                                else
                                {
                                    QString itemText;

                                    if (actionType == DISPLAY_BY_PROCESS_NAME)
                                    {
                                        itemText = QString(CA_STR_MENU_DISPLAY_BY_PROCESS_ARG).arg(pItem->text());
                                    }
                                    else
                                    {
                                        itemText = QString(CA_STR_MENU_DISPLAY_BY_MODULE_ARG).arg(pItem->text());
                                    }

                                    pAction->setText(itemText);
                                }
                            }
                        }
                    }

                    // check if more than 1 row selected
                    if (columnCount() != 0 &&
                        (selectedItems().count() / columnCount()) > 1)
                    {
                        pAction->setEnabled(false);
                    }
                }
            }
        }
    }
}

QString CPUProfileDataTable::actionTypeToString(TableContextMenuActionType actionType)
{
    QString retVal;

    switch (actionType)
    {
        case DISPLAY_FUNCTION_IN_FUNCTIONS_VIEW:
        case DISPLAY_MODULE_IN_FUNCTIONS_VIEW:
        case DISPLAY_PROCESS_IN_FUNCTIONS_VIEW:
        {
            retVal = CA_STR_MENU_DISPLAY_IN_FUNCTIONS_VIEW;
            break;
        }

        case DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW:
        {
            retVal = CA_STR_MENU_DISPLAY_IN_CALL_GRAPH_VIEW;
            break;
        }

        case DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW:
        {
            retVal = CA_STR_MENU_DISPLAY_IN_SOURCE_CODE_VIEW;
            break;
        }

        case DISPLAY_PROCESS_IN_MODULE_VIEW:
        case DISPLAY_MODULE_IN_MODULES_VIEW:
        {
            retVal = CA_STR_MENU_DISPLAY_IN_MODULES_VIEW;
            break;
        }

        case DISPLAY_BY_MODULE_NAME:
        case DISPLAY_BY_PROCESS_NAME:
        {
            retVal = CA_STR_MENU_DISPLAY_BY;
            break;
        }

        case DISPLAY_CLU_NOTES:
        {
            if (m_CLUNoteShown)
            {
                retVal = CA_STR_MENU_HIDE_CLU_NOTE;
            }
            else
            {
                retVal = CA_STR_MENU_SHOW_CLU_NOTE;
            }

            break;
        }
    }

    return retVal;
}

void CPUProfileDataTable::onContextMenuAction()
{
    QAction* pAction = qobject_cast<QAction*>(sender());

    if (pAction != nullptr)
    {
        if (pAction->data().isValid())
        {
            TableContextMenuActionType actionType = (TableContextMenuActionType)pAction->data().toInt();

            if (DISPLAY_CLU_NOTES == actionType)
            {
                emit contextMenuActionTriggered(actionType, nullptr);
            }
            else
            {
                // Get the first selected item:
                QModelIndexList selectedList = selectedIndexes();

                if (0 == selectedList.size())
                {
                    acMessageBox::instance().critical("CodeXL Error", "No row selected!");

                }
                else
                {
                    int selectedIndex = selectedList.first().row();
                    QTableWidgetItem* pItem = item(selectedIndex, 0);

                    if (pItem != nullptr)
                    {
                        // Emit a signal that the action was triggered:
                        emit contextMenuActionTriggered(actionType, pItem);
                    }
                }
            }
        }
    }
}

bool CPUProfileDataTable::HandleHotSpotIndicatorSet()
{
    return displayProfileData(m_pProfileReader);
}

bool CPUProfileDataTable::buildHotSpotIndicatorMap()
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        GT_IF_WITH_ASSERT(retVal)
        {
            // Clear the map:
            m_hotSpotCellsMap.clear();
            retVal = false;
            int precision = afGlobalVariablesManager::instance().floatingPointPrecision();

            // Find the index of the sample count and sample count percent indices:
            int sampleCountIndex = -1;
            int samplesCountPercentIndex = -1;

            for (int i = 0, sz = (int)m_pTableDisplaySettings->m_displayedColumns.size(); i < sz; i++)
            {
                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_PERCENT_COL)
                {
                    samplesCountPercentIndex = i;
                }

                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
                {
                    sampleCountIndex = i;
                }
            }

            const int rowsAmount = rowCount();
            GT_IF_WITH_ASSERT(sampleCountIndex >= 0)
            {
                m_totalSampleCount = 0;

                // Go over all the lines and summarize the total values of the samples:
                for (int rowIndex = 0; rowIndex < rowsAmount; rowIndex++)
                {
                    QTableWidgetItem* pTableItem = item(rowIndex, sampleCountIndex);
                    GT_IF_WITH_ASSERT(pTableItem != nullptr)
                    {
                        double doubleVal;
                        doubleVal = pTableItem->data(Qt::DisplayRole).toDouble();

                        m_totalSampleCount += doubleVal;
                    }
                }

                if (0 < rowsAmount && 0 < m_pTableDisplaySettings->m_amountOfItemsInDisplay)
                {
                    m_hotSpotCellsMap.reserve(rowsAmount);
                }

                // Go over all the lines for the hot spot index and find the values:
                for (int rowIndex = 0; rowIndex < rowsAmount; rowIndex++)
                {
                    QTableWidgetItem* pTableItem = item(rowIndex, sampleCountIndex);
                    GT_IF_WITH_ASSERT(pTableItem != nullptr)
                    {
                        // Fix the value for the percent item:
                        float percentF = 0.0;
                        double doubleVal;
                        doubleVal = pTableItem->data(Qt::DisplayRole).toDouble();

                        unsigned long long ullVal = 0;

                        if (m_totalSampleCount > 0)
                        {
                            if (precision > 0)
                            {
                                // doubleVal = pTableItem->data(Qt::DisplayRole).toDouble();
                                percentF = doubleVal / (double)m_totalSampleCount * 100;
                            }
                            else
                            {
                                ullVal = pTableItem->data(Qt::DisplayRole).toULongLong();
                                percentF = (double)ullVal / (double)m_totalSampleCount * 100;
                                doubleVal = (double)ullVal;
                            }
                        }

                        if (samplesCountPercentIndex >= 0)
                        {
                            QTableWidgetItem* pSamplesPercentTableItem = item(rowIndex, samplesCountPercentIndex);
                            GT_IF_WITH_ASSERT(pSamplesPercentTableItem != nullptr)
                            {
                                // Get the data for the current cell:
                                QString strPrecision = QString::number(percentF, 'f', precision);
                                double valuePrecision = strPrecision.toDouble();
                                QVariant dataVariant;
                                dataVariant.setValue(valuePrecision);
                                pSamplesPercentTableItem->setData(Qt::DisplayRole, dataVariant);

                                QString tooltip = dataVariant.toString();
                                pSamplesPercentTableItem->setToolTip(tooltip);
                            }
                        }

                        // Update the hot spot map:
                        if (0 < m_pTableDisplaySettings->m_amountOfItemsInDisplay)
                        {
                            HotSpotValue val;
                            val.m_index = rowIndex;
                            val.m_percentValue = percentF;
                            m_hotSpotCellsMap.push_back(val);
                        }

                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

int CPUProfileDataTable::amountOfShownRows() const
{
    int retVal = 0;

    for (int i = 0; i < rowCount(); i++)
    {
        if (!isRowHidden(i))
        {
            retVal ++;
        }
    }

    return retVal;
}

void CPUProfileDataTable::GetCluDataInRow(int row, int sampleIndex, gtVector<float>& cluData)
{
    gtMap<int, int> idxList;
    int cluEndOffset = IBS_CLU_OFFSET(IBS_CLU_END);
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
    {
        for (int cpuId = 0; cpuId < m_pSessionDisplaySettings->m_cpuCount; cpuId++)
        {
            if ((m_pSessionDisplaySettings->m_separateBy == SEPARATE_BY_NONE) && (cpuId > 0))
            {
                // If samples are not separated by core/numa, no need to loop for each cpu
                break;
            }

            for (int i = 0; i <= cluEndOffset; i++)
            {
                EventMaskType evClu = EncodeEvent((IBS_CLU_BASE + i), 0, true, true);
                SampleKeyType key;
                key.event = evClu;
                key.cpu = cpuId;
                CpuEventViewIndexMap::const_iterator idxIt = m_pSessionDisplaySettings->m_eventToIndexMap.find(key);

                if (idxIt != m_pSessionDisplaySettings->m_eventToIndexMap.end())
                {

                    int idx = *idxIt;

                    if (idxList.find(idx) == idxList.end())
                    {
                        // The check is to avoid duplicate data in case the samples are separated by numa:
                        idxList.insert(std::pair<char, int>(idx, 0));

                        QTableWidgetItem* pItem = this->item(row, sampleIndex + idx);
                        float data1 = pItem->text().toFloat();
                        cluData.push_back(data1);
                    }

                }
                else
                {
                    cluData.push_back(0);
                }
            }
        }
    }
}

void CPUProfileDataTable::hideFilteredColumns()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
    {
        // Iterate each of the columns and hide the filtered ones:
        for (int col = 0, colsAmount = columnCount(); col < colsAmount; ++col)
        {
            QTableWidgetItem* pHeaderItem = horizontalHeaderItem(col);
            GT_IF_WITH_ASSERT(pHeaderItem != nullptr)
            {
                bool shouldShow = !m_pSessionDisplaySettings->m_filteredDataColumnsCaptions.contains(pHeaderItem->text());

                if (shouldShow)
                {
                    showColumn(col);
                }
                else
                {
                    hideColumn(col);
                }
            }
        }
    }
}

void CPUProfileDataTable::sortIndicatorChanged(int sortColumn, Qt::SortOrder order)
{
    // Save the sort caption & order:
    QTableWidgetItem* pItem = horizontalHeaderItem(sortColumn);

    if ((pItem != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        m_pTableDisplaySettings->m_lastSortColumnCaption = pItem->text();
        m_pTableDisplaySettings->m_lastSortOrder = order;
    }

    UpdateLastRowItemsSortOrder();
}

void CPUProfileDataTable::setCLUPercentValues()
{
    // Do not show percentage value when profiled CLU:
    bool isProfilingCLU = false;

    if (m_pSessionDisplaySettings->m_pProfileInfo != nullptr)
    {
        isProfilingCLU = m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU;
    }

    if (isProfilingCLU)
    {

        QList<int> cluSamplePercentIndexList;
        bool isCLUHotSpot = false;
        findCLUPercentColumn(cluSamplePercentIndexList, "", isCLUHotSpot);

        if (cluSamplePercentIndexList.size() > 0)
        {
            m_pCLUDelegate = new acTablePercentItemDelegate;
            m_pCLUDelegate->SetOwnerTable(this);

            foreach (int index, cluSamplePercentIndexList)
            {
                setItemDelegateForColumn(index, m_pCLUDelegate);
            }
        }
        else
        {
            // Get the column index for the column that contains the "samples" content, and set it's delegate to null:
            int samplesColIndex = -1;

            for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns[i]; i++)
            {
                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
                {
                    samplesColIndex = i;
                    break;
                }
            }

            GT_IF_WITH_ASSERT(samplesColIndex >= 0)
            {
                setItemDelegateForColumn(samplesColIndex, &acNumberDelegateItem::Instance());
            }
        }
    }
}


void CPUProfileDataTable::findCLUPercentColumn(QList<int>& cluSampleColumnIndexList, const QString& hotSpotCaption, bool& isHotSpotCluPercent)
{
    if ((m_pSessionDisplaySettings != nullptr) && (m_pSessionDisplaySettings->m_pProfileInfo != nullptr))
    {
        if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU)
        {
            if (!hotSpotCaption.isEmpty())
            {
                EventsFile* pEventsFile = m_pSessionDisplaySettings->getEventsFile();

                if (pEventsFile != nullptr)
                {
                    CpuEvent ev;
                    pEventsFile->FindEventByName(hotSpotCaption, ev);

                    if (ev.value == DE_IBS_CLU_PERCENTAGE)
                    {
                        isHotSpotCluPercent = true;
                    }
                }
            }

            for (int i = 0; i < columnCount(); i++)
            {
                QTableWidgetItem* pHeaderItem = horizontalHeaderItem(i);

                if (pHeaderItem != nullptr && pHeaderItem->text().contains(PM_profileTypeCLU))
                {
                    cluSampleColumnIndexList << i;
                }
            }
        }
    }
}

bool CPUProfileDataTable::IsTableEmpty() const
{
    // Check if there is at least one shown row:
    bool isOneRowShown = false;
    int rows = rowCount();

    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        for (int i = 0 ; i < rows ; i++)
        {
            if (!isRowHidden(i) && (i != m_pEmptyRowTableItem->row()) && (i != m_pOtherSamplesRowItem->row()))
            {
                isOneRowShown = true;
                break;
            }
        }
    }

    return !isOneRowShown;
}
bool CPUProfileDataTable::HandleEmptyTable()
{
    // if at least one shown row: hide "empty tbale" row
    // if no: unhide "empty tbale" row
    bool isTableEmpty = false;
    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        isTableEmpty = IsTableEmpty();
        setRowHidden(m_pEmptyRowTableItem->row(), !isTableEmpty);
    }

    return isTableEmpty;
}

void CPUProfileDataTable::SetLastRowForTop5(bool isTableEmpty)
{
    setSortingEnabled(false);
    blockSignals(true);

    if (nullptr != m_pOtherSamplesRowItem)
    {
        // By default hide row of "others" message. Unhide only if there is more data in the table that is not shown
        // !!! has to do this row in case the table is empty
        setRowHidden(m_pOtherSamplesRowItem->row(), true);
    }

    // Don't fill and unhide "other" row when table is empty or in cache line profiling
    if (!isTableEmpty && !IsCacheLineProfiling())
    {
        int percentColIndex = TableDisplaySettings::UNKNOWN_COL;
        int samplesColIndex = TableDisplaySettings::UNKNOWN_COL;
        int nameColIndex    = TableDisplaySettings::UNKNOWN_COL;

        GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
        {
            //get percent column number
            for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns.size() ; i++)
            {
                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_PERCENT_COL)
                {
                    percentColIndex = i;
                }
                else if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
                {
                    samplesColIndex = i;
                }
                else if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings:: FUNCTION_NAME_COL)
                {
                    nameColIndex = i;
                }

                // Stop loop if got the index for both columns
                if (samplesColIndex != TableDisplaySettings::UNKNOWN_COL &&
                    percentColIndex != TableDisplaySettings::UNKNOWN_COL &&
                    nameColIndex    != TableDisplaySettings::UNKNOWN_COL)
                {
                    break;
                }
            }

            int rows = rowCount();
            QString text;
            double totalPercent = 0;
            int totalShownSamples = 0;

            // Get data from shown rows - total percent and total samples num
            for (int i = 0 ; i < rows ; i++)
            {
                if (!isRowHidden(i))
                {
                    getItemText(i, percentColIndex, text);

                    if (text != "")
                    {
                        totalPercent += text.toDouble();
                    }

                    getItemText(i, samplesColIndex, text);

                    if (text != "")
                    {
                        totalShownSamples += text.toDouble();
                    }
                }
            }

            double leftPercent = 0.0;
            int hiddenSamples = (int)m_totalSampleCount - totalShownSamples;

            // Show message row only if there are hidden rows. in case of less then 5 rows - don't show message
            if (hiddenSamples > 0)
            {
                GT_IF_WITH_ASSERT(nullptr != m_pOtherSamplesRowItem)
                {
                    leftPercent = 100.0 - totalPercent;

                    int rowNum = m_pOtherSamplesRowItem->row();
                    QFont font;
                    QTableWidgetItem* rowItem;
                    QString tmpStr, tmp;

                    // Set "others" row name column items
                    tmpStr = CP_strOther;
                    rowItem = item(rowNum, nameColIndex);
                    rowItem->setText(tmpStr);
                    rowItem->setTextColor(QColor(Qt::gray));

                    // If the other table rows has icon - insert empty icon in the same size
                    if (m_tableRowHasIcon)
                    {
                        QPixmap emptyIcon;
                        acSetIconInPixmap(emptyIcon, AC_ICON_EMPTY);
                        rowItem->setIcon(QIcon(emptyIcon));
                    }

                    // Set "others" row percent column items
                    rowItem = item(rowNum, percentColIndex);
                    rowItem->setText(tmpStr.setNum(leftPercent));
                    rowItem->setTextColor(QColor(Qt::gray));

                    // set "others" row samples column items
                    rowItem = item(rowNum, samplesColIndex);
                    rowItem->setText(tmpStr.setNum(hiddenSamples));
                    rowItem->setTextColor(QColor(Qt::gray));
                    rowItem->setFont(font);

                    // Show row
                    int testRow = m_pOtherSamplesRowItem->row();
                    setRowHidden(testRow, false);
                }
            }
        }
    }

    blockSignals(false);
    setSortingEnabled(true);
}

void CPUProfileDataTable::UpdateLastRowItemsSortOrder()
{
    // disable sorting and signals
    blockSignals(true);
    setSortingEnabled(false);

    GT_IF_WITH_ASSERT(m_pTableDisplaySettings)
    {
        //get table sort order
        CPUProfileDataTableItem::SortOrder order = CPUProfileDataTableItem::GetTableSortOrder(m_pTableDisplaySettings->m_lastSortOrder);
        GT_IF_WITH_ASSERT(nullptr != m_pOtherSamplesRowItem)
        {
            //update all "other" row items
            int rowNum = m_pOtherSamplesRowItem->row();
            QTableWidgetItem* rowItem;

            for (int i = 0; i < columnCount(); ++i)
            {
                rowItem = item(rowNum, i);
                GT_IF_WITH_ASSERT(rowItem)
                {
                    rowItem->setData(AC_USER_ROLE_OTHER_ROW, order);
                }
            }
        }
    }

    // enable sorting and signals
    blockSignals(false);
    setSortingEnabled(true);
}

bool CPUProfileDataTable::IsBaseTimeProfiling() const
{
    bool isTimeBaseProfiling = false;

    // check if base time profile
    GT_IF_WITH_ASSERT(m_pProfileReader != nullptr)
    {
        gtString str = m_pProfileReader->getProfileInfo()->m_profType;
        QString qstr = acGTStringToQString(str);

        if (qstr == QString(PM_profileTypeTimeBased))
        {
            isTimeBaseProfiling = true;
        }
    }

    // check the table type
    TableType type = GetTableType();
    bool isTableType = (MODULES_DATA_TABLE == type ||
                        PROCESSES_DATA_TABLE == type ||
                        FUNCTION_DATA_TABLE == type);

    return isTimeBaseProfiling && isTableType;
}

bool CPUProfileDataTable::IsShowSeperatePercentColumns() const
{
    bool isTimeBaseProfiling = IsBaseTimeProfiling();

    // Do not show percentage value when profiled CLU:
    bool isProfilingCLU = false;

    if (m_pSessionDisplaySettings != nullptr && m_pSessionDisplaySettings->m_pProfileInfo != nullptr)
    {
        isProfilingCLU = m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU;
    }

    bool displayPercentageInColumn = CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn && (!isProfilingCLU);

    return isTimeBaseProfiling && displayPercentageInColumn;
}

bool CPUProfileDataTable::IsCacheLineProfiling() const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pProfileReader != nullptr)
    {
        gtString str = m_pProfileReader->getProfileInfo()->m_profType;
        QString qstr = acGTStringToQString(str);

        if (qstr == QString(PM_profileTypeCLU))
        {
            retVal = true;
        }
    }

    return retVal;
}

int CPUProfileDataTable::GetEmptyTableItemRowNum() const
{
    int retVal = -1;
    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        retVal = m_pEmptyRowTableItem->row();
    }
    return retVal;
}

int CPUProfileDataTable::GetOtherSamplesItemRowNum() const
{
    int retVal = -1;
    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        retVal = m_pOtherSamplesRowItem->row();
    }
    return retVal;
}
