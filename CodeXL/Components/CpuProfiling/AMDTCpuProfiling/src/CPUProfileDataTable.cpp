//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPUProfileDataTable.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

/// Local:
#include <inc/CPUProfileDataTable.h>
#include <inc/Auxil.h>
#include <inc/StringConstants.h>
#include <inc/CPUProfileDataTableItem.h>

#define MAX_COLUMN_WIDTH 350


// Static members:
gtPtrVector<QPixmap*> CPUProfileDataTable::m_sTableIcons;
bool CPUProfileDataTable::m_sIconsInitialized = false;
bool CPUProfileDataTable::m_CLUNoteShown = true;


CPUProfileDataTable::CPUProfileDataTable(QWidget* pParent,
                                         const gtVector<CPUProfileDataTable::TableContextMenuActionType>& additionalContextMenuActions,
                                         SessionTreeNodeData* pSessionData)
    : acListCtrl(pParent, CP_CPU_TABLE_ROW_HEIGHT),
      m_pDisplaySessionData(pSessionData)
{
    setSortingEnabled(true);
    setEnablePaste(false);

    // Copy cell items with captions:
    m_shouldCopyColumnHeaders = true;

    // Extend the context menu with the requested actions:
    extendContextMenu(additionalContextMenuActions);

    // Connect the sort click slot:
    bool rc = connect(horizontalHeader(),
                      SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
                      this,
                      SLOT(sortIndicatorChanged(int, Qt::SortOrder)));
    GT_ASSERT(rc);
}

CPUProfileDataTable::~CPUProfileDataTable()
{
}

bool CPUProfileDataTable::AddEmptyAndOtherRow()
{
    QStringList list;

    for (int i = 0; i < columnCount(); ++i)
    {
        list << "";
    }

    // add "empty-message" row
    int newRowIndex = rowCount();
    addRow(list, nullptr);

    // save item of this row
    m_pEmptyRowTableItem = item(newRowIndex, 0);

    // hide the empty row, show this row if table is empty
    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        setRowHidden(newRowIndex, true);
    }

    // add "other" row
    newRowIndex = rowCount();
    addRow(list, nullptr);

    // save item of this row
    m_pOtherSamplesRowItem = item(newRowIndex, 0);

    // set all row items with "other" row data role
    GT_IF_WITH_ASSERT(m_pOtherSamplesRowItem != nullptr)
    {
        m_pOtherSamplesRowItem->setData(AC_USER_ROLE_OTHER_ROW, CPUProfileDataTableItem::ASCENDING_ORDER);

        for (int i = 1; i < columnCount(); ++i)
        {
            auto rowItem = item(newRowIndex, i);

            if (rowItem != nullptr)
            {
                rowItem->setData(AC_USER_ROLE_OTHER_ROW, CPUProfileDataTableItem::ASCENDING_ORDER);
            }
        }

        // hide the other row, show this row if there other samples
        setRowHidden(newRowIndex, true);
    }

    return true;
}

bool CPUProfileDataTable::displayTableSummaryData(std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
                                                  std::shared_ptr<DisplayFilter> pDisplayFilter,
                                                  int counterIdx,
                                                  bool isCLU)
{
    bool retVal = false;

    //TODO : for CLU backend support req, temp hack need to be removed
    if (isCLU)
    {
        pDisplayFilter->InitToDefault();
    }

    if (nullptr != pProfDataRdr)
    {
        m_pProfDataRdr = pProfDataRdr;
        m_pDisplayFilter = pDisplayFilter;
        m_isCLU = isCLU;

        // Clear table items:
        clear();
        clearContents();
        setColumnCount(0);
        setRowCount(0);

        if (horizontalHeader()->count() == 0)
        {
            initializeTableHeaders(pDisplayFilter, true);
        }

        AddEmptyAndOtherRow();
        fillSummaryTable(counterIdx);
        HandleEmptyTable();

        sortTable();

        retVal = true;
    }

    return retVal;
}

QTableWidgetItem* CPUProfileDataTable::allocateNewWidgetItem(const QString& text)
{
    // Allocate my own widget item:
    return new CPUProfileDataTableItem(text);
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
                    if (m_pTableDisplaySettings->m_lastSortColumnCaption == pHeaderItem->text())
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
                for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
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

bool CPUProfileDataTable::SetPIDColumnValue(int row, int column, AMDTUInt32 pid)
{
    QTableWidgetItem *pPIDItem = item(row, column);

    if (pPIDItem != nullptr)
    {
        QVariant dataVariant;
        dataVariant.setValue(pid);

        pPIDItem->setData(Qt::DisplayRole, dataVariant);
        pPIDItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QString tooltip = dataVariant.toString();
        pPIDItem->setToolTip(tooltip);
    }

    return true;
}

bool CPUProfileDataTable::SetSampleColumnValue(int row, int column, double sampleValue)
{
    QTableWidgetItem *pSampleItem = item(row, column);

    if (pSampleItem != nullptr && sampleValue > 0)
    {
        QVariant dataVariant;
        dataVariant.setValue(sampleValue);

        pSampleItem->setData(Qt::DisplayRole, dataVariant);
        pSampleItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QString tooltip = dataVariant.toString();
        pSampleItem->setToolTip(tooltip);
    }

    return true;
}

bool CPUProfileDataTable::SetSamplePercentColumnValue(int row, int column, double samplePercentValue)
{
    QTableWidgetItem *pSamplePercentItem = item(row, column);

    if (pSamplePercentItem != nullptr && samplePercentValue > 0)
    {
        QString currentValue = QString::number(samplePercentValue, 'f', SAMPLE_PERCENT_PRECISION);

        QVariant dataVariant;
        dataVariant.setValue(currentValue.toFloat());

        pSamplePercentItem->setData(Qt::EditRole, dataVariant);
        pSamplePercentItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QString tooltip = QString::number(samplePercentValue, 'f', SAMPLE_VALUE_PRECISION);
        tooltip.append("%");

        pSamplePercentItem->setToolTip(tooltip);
    }

    return true;
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

bool CPUProfileDataTable::setTableItemPercentValue(int rowIndex, int colIndex, int percentValue)
{
    bool retVal = false;

    // Get the table item for the requested column and row:
    QTableWidgetItem* pTableItem = item(rowIndex, colIndex);
    GT_IF_WITH_ASSERT(pTableItem != nullptr)
    {
        // Get the table item value:
        //double doubleVal = pTableItem->data(Qt::DisplayRole).toDouble();

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
    bool isSystem = osIsSystemModule(filePath.asString());

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
                        // All the actions but the CLU notes action are actions that are dependent on the selected items:
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
                                        // module name is at column 1
                                        pItem = item(selectedIndex, 1);

                                        if (pItem != nullptr)
                                        {
                                            itemText = QString(CA_STR_MENU_DISPLAY_BY_MODULE_ARG).arg(pItem->text());
                                        }
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
    return displayTableData(m_pProfDataRdr, m_pDisplayFilter, AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES);
}

void CPUProfileDataTable::GetCluDataInRow(int row, int sampleIndex, gtVector<float>& cluData)
{
    GT_UNREFERENCED_PARAMETER(row);
    GT_UNREFERENCED_PARAMETER(sampleIndex);
    GT_UNREFERENCED_PARAMETER(cluData);
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

bool CPUProfileDataTable::IsTableEmpty() const
{
    // Check if there is at least one row shown
    bool isOneRowShown = false;

    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        int rows = rowCount();

        for (int i = 0 ; i < rows ; i++)
        {
            if (!isRowHidden(i) && (i != m_pEmptyRowTableItem->row()))
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
    // if at least one row shown: hide "empty table" row
    // if no: unhide "empty table" row
    GT_IF_WITH_ASSERT(m_pEmptyRowTableItem != nullptr)
    {
        if (IsTableEmpty())
        {
            int rowIdx = m_pEmptyRowTableItem->row();
            int colIdx = getEmptyMsgItemColIndex();

            QTableWidgetItem* msgItem = item(rowIdx, colIdx);

            if (msgItem != nullptr)
            {
                msgItem->setText(CP_emptyTableMessage);
            }

            setRowHidden(rowIdx, false);
        }
    }

    return true;
}

void CPUProfileDataTable::UpdateLastRowItemsSortOrder()
{
    // disable sorting and signals
    blockSignals(true);
    setSortingEnabled(false);

    GT_IF_WITH_ASSERT(nullptr != m_pTableDisplaySettings)
    {
        //get table sort order

        if (nullptr != m_pOtherSamplesRowItem)
        {
            CPUProfileDataTableItem::SortOrder order =
                CPUProfileDataTableItem::GetTableSortOrder(m_pTableDisplaySettings->m_lastSortOrder);

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
    GT_IF_WITH_ASSERT(m_pOtherSamplesRowItem != nullptr)
    {
        retVal = m_pOtherSamplesRowItem->row();
    }
    return retVal;
}

bool CPUProfileDataTable::delegateSamplePercent(int colNum)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        acTablePercentItemDelegate* pDelegate = new acTablePercentItemDelegate();

        pDelegate->SetOwnerTable(this);
        setItemDelegateForColumn(colNum, pDelegate);

        retVal = true;
    }

    return retVal;
}

bool CPUProfileDataTable::displayTableData(
    std::shared_ptr<cxlProfileDataReader> pProfDataRdr,
    std::shared_ptr<DisplayFilter> diplayFilter,
    AMDTProcessId procId,
    AMDTModuleId modId,
    std::vector<AMDTUInt64> moduleIdVec)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pProfDataRdr != nullptr)
    {
        // Set the profile reader:
        m_pProfDataRdr = pProfDataRdr;
        m_pDisplayFilter = diplayFilter;

        // Clear table items:
        clear();
        clearContents();
        setColumnCount(0);
        setRowCount(0);

        // Set the headers:
        bool rcHeaders = true;

        if (horizontalHeader()->count() == 0)
        {
            rcHeaders = initializeTableHeaders(diplayFilter);
            GT_ASSERT(rcHeaders);
        }

        AddEmptyAndOtherRow();

        // Fill the list data:
        bool rcData = fillTableData(procId, modId, moduleIdVec);
        GT_ASSERT(rcData);

        HandleEmptyTable();
        sortTable();

        retVal = true;
    }

    END_TICK_COUNT(DisplayProfileData);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: displayProfileData (%u ms)", m_elapsedTime[DisplayProfileData]);
#endif

    return retVal;
}

bool CPUProfileDataTable::initializeTableHeaders(std::shared_ptr<DisplayFilter> diplayFilter, bool isSummary)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT((diplayFilter != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        // Vector that contain the columns strings according to the current displayed columns:
        QStringList columnsStringByObjectType;
        QStringList columnTooltipsByObjectType;

        int tableDispSettingsColsNum = (int)m_pTableDisplaySettings->m_displayedColumns.size();

        CounterNameIdVec selectedCounterList;
        diplayFilter->GetSelectedCounterList(selectedCounterList);

        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty() && selectedCounterList.size() > 0)
        {
            m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption = acGTStringToQString(std::get<0>(selectedCounterList[0]));
        }

        for (int i = 0; i < tableDispSettingsColsNum; ++i)
        {
            QString colStr, colTooltip;

            bool rc = m_pTableDisplaySettings->colTypeAsString(m_pTableDisplaySettings->m_displayedColumns[i],
                                                               colStr, colTooltip);
            GT_ASSERT(rc);

            if (m_isCLU && (colStr == CP_colCaptionSamples))
            {
                colStr = diplayFilter->GetCLUOVHdrName();
                colTooltip = colStr;
            }

            columnsStringByObjectType << colStr;
            columnTooltipsByObjectType << colTooltip;
        }

        if (!isSummary)
        {
            CounterNameIdVec selectedCounterList;
            diplayFilter->GetSelectedCounterList(selectedCounterList);

            for (const auto& counter : selectedCounterList)
            {
                // Append event abbreviation as column name
                columnsStringByObjectType << acGTStringToQString(std::get<1>(counter));

                // Fetch event name and description
                QString counterName = acGTStringToQString(std::get<0>(counter));
                QString counterDesc = acGTStringToQString(std::get<2>(counter));

                // Prepare tooltip string
                QString tooltip;
                acWrapAndBuildFormattedTooltip(counterName, counterDesc, tooltip);

                // Append column tooltip
                columnTooltipsByObjectType << tooltip;

                // If TBP, append percent column and tooltip
                if (diplayFilter->GetProfileType() == AMDT_PROFILE_TYPE_TBP)
                {
                    QString columnName("Timer Percentage");
                    QString columnDesc("Percent of samples collected for Timer.");

                    QString tooltip;
                    acWrapAndBuildFormattedTooltip(columnName, columnDesc, tooltip);

                    columnsStringByObjectType << columnName;
                    columnTooltipsByObjectType << tooltip;
                }
            }
        }

        initHeaders(columnsStringByObjectType, false);

        int column = 0;
        for (const auto& tooltipStr : columnTooltipsByObjectType)
        {
            auto headerItem = horizontalHeaderItem(column++);

            if (headerItem != nullptr)
            {
                headerItem->setToolTip(tooltipStr);
            }
        }
    }
    return retVal;
}

void CPUProfileDataTable::SetIcon(gtString modulePath,
                                  AMDTUInt32 rowIndex,
                                  AMDTUInt32 iconColIndex,
                                  AMDTUInt32 toolTipColidx,
                                  bool is32Bit,
                                  int idxRole)
{
    QString modulefullPath(acGTStringToQString(modulePath));
    QTableWidgetItem* pModuleNameItem = item(rowIndex, toolTipColidx);

    if (pModuleNameItem != nullptr)
    {
        pModuleNameItem->setToolTip(modulefullPath);
    }

    QPixmap* pIcon = CPUProfileDataTable::moduleIcon(modulePath, is32Bit);
    QTableWidgetItem* pNameItem = item(rowIndex, iconColIndex);

    if (pNameItem != nullptr)
    {
        // Set the original position in function vector:
        pNameItem->setData(idxRole, QVariant(rowIndex));

        if (pIcon != nullptr)
        {
            pNameItem->setIcon(QIcon(*pIcon));
        }
    }
}

void CPUProfileDataTable::SetTableSampleCountAndPercent(int row, int delegateColumn, const AMDTProfileData& profData)
{
    // TODO: Move column delegation functions outside as they are row-invariant
    if (nullptr != m_pDisplayFilter)
    {
        CounterNameIdVec selectedCounterList;
        m_pDisplayFilter->GetSelectedCounterList(selectedCounterList);

        int i = 0;

        for (const auto& counter : selectedCounterList)
        {
            // get counter type
            AMDTProfileCounterType counterType = static_cast<AMDTProfileCounterType>(std::get<4>(counter));
            auto sampleCount = profData.m_sampleValue.at(i).m_sampleCount;
            bool setPercentInColumn = m_pDisplayFilter->IsDisplaySamplePercent();

            // TBP, EBP, IBS
            if (counterType == AMDT_PROFILE_COUNTER_TYPE_RAW)
            {
                auto samplePercent = profData.m_sampleValue.at(i).m_sampleCountPercentage;

                // EBP, IBS
                if (m_pDisplayFilter->GetProfileType() != AMDT_PROFILE_TYPE_TBP)
                {
                    if (setPercentInColumn == true)
                    {
                        SetSamplePercentColumnValue(row, delegateColumn + i, samplePercent);
                        delegateSamplePercent(delegateColumn + i);
                    }
                    else
                    {
                        SetSampleColumnValue(row, delegateColumn + i, sampleCount);
                        setItemDelegateForColumn(delegateColumn + i, &acNumberDelegateItem::Instance());
                    }
                }
                else // TBP
                {
                    SetSampleColumnValue(row, delegateColumn + i, sampleCount);
                    setItemDelegateForColumn(delegateColumn + i, &acNumberDelegateItem::Instance());

                    SetSamplePercentColumnValue(row, delegateColumn + i + 1, samplePercent);
                    delegateSamplePercent(delegateColumn + i + 1);
                }
            }
            else // CLU
            {
                SetSampleColumnValue(row, delegateColumn + i, sampleCount);

                // First CLU sample column shows the data in percentage.
                if (i == 0)
                {
                    delegateSamplePercent(delegateColumn + i);
                }
                else
                {
                    setItemDelegateForColumn(delegateColumn + i, &acNumberDelegateItem::Instance());
                }
            }

            ++i;
        }
    }
}

void CPUProfileDataTable::HandleTBPPercentCol(int column)
{
    if (m_pDisplayFilter->GetProfileType() == AMDT_PROFILE_TYPE_TBP)
    {
        if (!m_pDisplayFilter->IsDisplaySamplePercent())
        {
            hideColumn(column);
        }
    }
}