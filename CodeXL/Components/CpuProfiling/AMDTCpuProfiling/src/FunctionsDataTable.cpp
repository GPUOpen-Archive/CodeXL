//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FunctionsDataTable.cpp
///
//==================================================================================

// STL
#include <unordered_map>

// Infra
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>

// Local
#include <inc/FunctionsDataTable.h>


#define FunctionDataIndexRole  ((int)(Qt::UserRole) + 0)


const AMDTFunctionId INVALID_FUNCTION_ID = 0;


FunctionsDataTable::FunctionsDataTable(QWidget* pParent,
                                       const gtVector<TableContextMenuActionType>& additionalContextMenuActions,
                                       SessionTreeNodeData* pSessionData,
                                       CpuSessionWindow* pSessionWindow) :
    CPUProfileDataTable(pParent, additionalContextMenuActions, pSessionData),
    m_pParentSessionWindow(pSessionWindow)
{
}

FunctionsDataTable::~FunctionsDataTable()
{
}

QString FunctionsDataTable::getModuleName(int rowIndex) const
{
    QString name;

    GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
    {
        QTableWidgetItem* pNameTableItem = item(rowIndex, m_moduleNameColumn);

        GT_IF_WITH_ASSERT(nullptr != pNameTableItem)
        {
            name = pNameTableItem->toolTip();
        }
    }

    return name;
}

QString FunctionsDataTable::getFunctionName(int rowIndex) const
{
    QString name;

    GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
    {
        QTableWidgetItem* pNameTableItem = item(rowIndex, m_functionNameColumn);

        GT_IF_WITH_ASSERT(nullptr != pNameTableItem)
        {
            name = pNameTableItem->text();
        }
    }

    return name;
}

QString FunctionsDataTable::getFunctionId(int rowIndex) const
{
    QString name;

    GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
    {
        QTableWidgetItem* pNameTableItem = item(rowIndex, m_functionIdColumn);

        GT_IF_WITH_ASSERT(nullptr != pNameTableItem)
        {
            name = pNameTableItem->text();
        }
    }

    return name;
}

void FunctionsDataTable::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    CPUProfileDataTable::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT((m_pContextMenu != nullptr) && (m_pProfDataRdr != nullptr))
    {
        if (selectedItems().count() > 0)
        {
            // get the selected item (only one item)
            QTableWidgetItem* pItem = selectedItems().first();

            if (nullptr != pItem)
            {
                int rowIndex = pItem->row();

                gtString funcName = acQStringToGTString(getFunctionName(rowIndex));
                bool isUnknownModule = funcName.startsWith(L"Unknown Module");

                AMDTFunctionId funcId = getFunctionId(rowIndex).toInt();

                int selectedCount = selectedItems().count();
                int colCount = columnCount();

                // if its empty or is the "empty row" string item
                bool isBadSelection = (rowIndex == -1) || ((colCount > 0 && (selectedCount / colCount) > 1)) || isUnknownModule;

                foreach(QAction* pAction, m_pContextMenu->actions())
                {
                    if (pAction != nullptr && pAction->data().isValid())
                    {
                        TableContextMenuActionType actionType = (TableContextMenuActionType)pAction->data().toInt();
                        bool isActionEnabled = false;

                        if (isBadSelection)
                        {
                            isActionEnabled = false;
                        }
                        else if (DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW == actionType)
                        {
                            //get supported call graph process
                            gtVector<AMDTProcessId> cssProcesses;
                            bool rc = m_pProfDataRdr->GetCallGraphProcesses(cssProcesses);

                            if ((INVALID_FUNCTION_ID != funcId) && (true == rc))
                            {
                                for (auto const& process : cssProcesses)
                                {
                                    AMDTProfileFunctionData  functionData;
                                    bool retVal = m_pProfDataRdr->GetFunctionData(funcId,
                                        process,
                                        AMDT_PROFILE_ALL_THREADS,
                                        functionData);

                                    if (retVal)
                                    {
                                        if (!functionData.m_pidsList.empty())
                                        {
                                            if (process == functionData.m_pidsList.at(0))
                                            {
                                                isActionEnabled = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (DISPLAY_FUNCTION_IN_SOURCE_CODE_VIEW == actionType)
                        {
                            if (INVALID_FUNCTION_ID != funcId)
                            {
                                isActionEnabled = true;
                            }
                        }
                        else if (DISPLAY_FUNCTION_IN_FUNCTIONS_VIEW == actionType)
                        {
                            bool isOther = funcName.isEqual("other") && (INVALID_FUNCTION_ID == funcId);

                            if (isOther || (INVALID_FUNCTION_ID != funcId))
                            {
                                isActionEnabled = true;
                            }
                        }

                        // Enable / disable the action:
                        pAction->setEnabled(isActionEnabled);
                    }
                }
            }
        }
    }
}

CPUProfileDataTable::TableType FunctionsDataTable::GetTableType() const
{
    return CPUProfileDataTable::FUNCTION_DATA_TABLE;
}

bool FunctionsDataTable::fillSummaryTable(int counterIdx)
{
    bool retVal = false;

    // Set the columns for function summary table
    m_functionIdColumn = CXL_FUNC_SUMMARY_FUNC_ID_COL;
    m_functionNameColumn = CXL_FUNC_SUMMARY_FUNC_NAME_COL;
    m_moduleNameColumn = CXL_FUNC_SUMMARY_MODULE_COL;

    if (nullptr != m_pProfDataRdr && nullptr != m_pDisplayFilter && nullptr != m_pTableDisplaySettings)
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);

        AMDTProfileDataVec funcProfileData;
        rc = m_pProfDataRdr->GetFunctionSummary(counterDesc.at(counterIdx).m_id, funcProfileData);
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (auto profData : funcProfileData)
        {
            bool isOther = (profData.m_name.compare(L"other") == 0);

            if (!isOther)
            {
                // Create QstringList to hold the values
                QStringList list;

                // Insert the function id
                list << QString::number(profData.m_id);

                // Insert the function name
                list << profData.m_name.asASCIICharArray();

                // Insert blank sample and sample percent
                list << "" << "";

                // Insert module name
                AMDTProfileModuleInfoVec procInfo;
                m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, procInfo);

                if (procInfo.empty())
                {
                    continue;
                }

                list << procInfo.at(0).m_name.asASCIICharArray();

                addRow(list, nullptr);

                // Set module type icon
                QTableWidgetItem* pNameItem = item(rowCount() - 1, CXL_FUNC_SUMMARY_FUNC_NAME_COL);
                QPixmap* pIcon = moduleIcon(procInfo.at(0).m_path, !procInfo.at(0).m_is64Bit);

                if (pNameItem != nullptr && pIcon != nullptr)
                {
                    pNameItem->setIcon(QIcon(*pIcon));
                }

                // Set tooltip for module column
                QString modulefullPath(acGTStringToQString(procInfo.at(0).m_path));
                QTableWidgetItem* pModuleNameItem = item(rowCount() - 1, CXL_FUNC_SUMMARY_MODULE_COL);

                if (pModuleNameItem != nullptr)
                {
                    pModuleNameItem->setToolTip(modulefullPath);
                }

                if (m_isCLU)
                {
                    SetSampleColumnValue(rowCount() - 1,
                        CXL_FUNC_SUMMARY_SAMPLE_COL,
                        profData.m_sampleValue.at(0).m_sampleCount);
                }
                else
                {
                    SetSampleColumnValue(rowCount() - 1,
                        CXL_FUNC_SUMMARY_SAMPLE_COL,
                        profData.m_sampleValue.at(0).m_sampleCount);

                    SetSamplePercentColumnValue(rowCount() - 1,
                        CXL_FUNC_SUMMARY_SAMPLE_PER_COL,
                        profData.m_sampleValue.at(0).m_sampleCountPercentage);
                }
            }
            else
            {
                int rowNum = m_pOtherSamplesRowItem->row();
                QTableWidgetItem* rowItem;
                QString tmpStr;

                // Set "other" row name column item
                tmpStr = CP_strOther;
                rowItem = item(rowNum, CXL_FUNC_SUMMARY_FUNC_NAME_COL);
                rowItem->setText(tmpStr);
                rowItem->setTextColor(QColor(Qt::gray));

                // Set empty icon for "other" row
                QPixmap emptyIcon;
                acSetIconInPixmap(emptyIcon, AC_ICON_EMPTY);
                rowItem->setIcon(QIcon(emptyIcon));

                // Set "other" row samples column item
                rowItem = item(rowNum, CXL_FUNC_SUMMARY_SAMPLE_COL);
                rowItem->setText(tmpStr.setNum(profData.m_sampleValue.at(0).m_sampleCount));
                rowItem->setTextColor(QColor(Qt::gray));

                // Set "other" row percent column item
                rowItem = item(rowNum, CXL_FUNC_SUMMARY_SAMPLE_PER_COL);
                rowItem->setText(tmpStr.setNum(profData.m_sampleValue.at(0).m_sampleCountPercentage));
                rowItem->setTextColor(QColor(Qt::gray));

                // Show "other" row
                setRowHidden(rowNum, false);
            }
        }

        delegateSamplePercent(CXL_FUNC_SUMMARY_SAMPLE_PER_COL);
        hideColumn(CXL_FUNC_SUMMARY_FUNC_ID_COL);

        if (m_isCLU)
        {
            if (m_pDisplayFilter->IsCLUPercentCaptionSet())
            {
                delegateSamplePercent(CXL_FUNC_SUMMARY_SAMPLE_COL);
            }
            else
            {
                setItemDelegateForColumn(CXL_FUNC_SUMMARY_SAMPLE_COL, &acNumberDelegateItem::Instance());
            }

            hideColumn(CXL_FUNC_SUMMARY_SAMPLE_PER_COL);
        }
        else
        {
            setItemDelegateForColumn(CXL_FUNC_SUMMARY_SAMPLE_COL, &acNumberDelegateItem::Instance());
        }

        setColumnWidth(CXL_FUNC_SUMMARY_FUNC_NAME_COL, MAX_FUNCTION_NAME_LEN);
        resizeColumnToContents(CXL_FUNC_SUMMARY_SAMPLE_COL);
        resizeColumnToContents(CXL_FUNC_SUMMARY_SAMPLE_PER_COL);
        resizeColumnToContents(CXL_FUNC_SUMMARY_MODULE_COL);

        if (m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
        {
            m_pTableDisplaySettings->m_lastSortColumnCaption = horizontalHeaderItem(CXL_FUNC_SUMMARY_SAMPLE_COL)->text();
        }

        setSortingEnabled(true);

        retVal = true;
    }

    return retVal;
}

bool FunctionsDataTable::AddRowToTable(const gtVector<AMDTProfileData>& functionProfileData)
{
    setSortingEnabled(false);

    for (const auto& profData : functionProfileData)
    {
        QStringList list;
        CounterNameIdVec selectedCounterList;

        // insert the function id
        list << QString::number(profData.m_id);

        // Insert function name
        list << profData.m_name.asASCIICharArray();

        // insert module name
        AMDTProfileModuleInfoVec moduleInfoList;
        m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, moduleInfoList);

        if (!moduleInfoList.empty())
        {
            list << acGTStringToQString(moduleInfoList.at(0).m_name);

            CounterNameIdVec selectedCounterList;
            m_pDisplayFilter->GetSelectedCounterList(selectedCounterList);

            for (size_t i = 0; i < selectedCounterList.size(); ++i)
            {
                list << "";
            }

            if (m_pDisplayFilter->GetProfileType() == AMDT_PROFILE_TYPE_TBP)
            {
                list << "";
            }

            addRow(list, nullptr);

            SetTableSampleCountAndPercent(rowCount() - 1, CXL_FUNC_TAB_SAMPLE_START_COL, profData);

            SetIcon(moduleInfoList.at(0).m_path,
                    rowCount() - 1,
                    m_functionNameColumn,
                    m_moduleNameColumn,
                    !moduleInfoList.at(0).m_is64Bit,
                    FunctionDataIndexRole);
        }
    }

    setSortingEnabled(true);

    return true;
}

bool FunctionsDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    bool retVal = false;

    // Set the columns for function Tab
    m_functionIdColumn = CXL_FUNC_TAB_FUNC_ID_COL;
    m_functionNameColumn = CXL_FUNC_TAB_FUNC_NAME_COL;
    m_moduleNameColumn = CXL_FUNC_TAB_MOD_NAME_COL;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        gtVector<AMDTProfileData> functionProfileData;

        if (modIdVec.empty())
        {
            bool rc = m_pProfDataRdr->GetFunctionProfileData(procId, modId, functionProfileData);
            GT_ASSERT(rc);
        }
        else
        {
            gtVector<AMDTProfileData> profileData;

            for (const auto& moduleId : modIdVec)
            {
                profileData.clear();
                m_pProfDataRdr->GetFunctionProfileData(procId, moduleId, profileData);
                functionProfileData.insert(functionProfileData.end(), profileData.begin(), profileData.end());
            }
        }

        // Compute total samples per counter for non-CLU profile.
        // (Not getting expected percentage from reporter layer. Need to fix this in DB layer later.)
        if (m_pDisplayFilter->GetProfileType() != AMDT_PROFILE_TYPE_CLU)
        {
            std::unordered_map<AMDTCounterId, double> sampleMap;

            // Compute the total samples per counter for given process.
            for (const auto& profileData : functionProfileData)
            {
                for (const auto& sampleData : profileData.m_sampleValue)
                {
                    if (sampleMap.find(sampleData.m_counterId) == sampleMap.end())
                    {
                        sampleMap[sampleData.m_counterId] = sampleData.m_sampleCount;
                    }
                    else
                    {
                        sampleMap[sampleData.m_counterId] += sampleData.m_sampleCount;
                    }
                }
            }

            // Compute percentage of samples per counter per function for given process.
            for (auto& profileData : functionProfileData)
            {
                for (auto& sampleData : profileData.m_sampleValue)
                {
                    sampleData.m_sampleCountPercentage = 0;

                    if (sampleMap[sampleData.m_counterId] > 0)
                    {
                        sampleData.m_sampleCountPercentage = (sampleData.m_sampleCount / sampleMap[sampleData.m_counterId]) * 100;
                    }
                }
            }

            sampleMap.clear();
        }

        AddRowToTable(functionProfileData);

        HandleTBPPercentCol(CXL_FUNC_TAB_TBP_SAMPLE_PER_COL);
        hideColumn(CXL_FUNC_TAB_FUNC_ID_COL);

        setColumnWidth(CXL_FUNC_TAB_FUNC_NAME_COL, MAX_FUNCTION_NAME_LEN);
        setColumnWidth(CXL_FUNC_TAB_MOD_NAME_COL, MAX_MODULE_NAME_LEN);

        if (m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
        {
            m_pTableDisplaySettings->m_lastSortColumnCaption = horizontalHeaderItem(CXL_FUNC_TAB_SAMPLE_START_COL)->text();
        }

        retVal = true;
    }

    return retVal;
}
