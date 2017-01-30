//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModulesDataTable.cpp
///
//==================================================================================

// STL
#include <unordered_map>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>

/// Local:
#include <inc/ModulesDataTable.h>


ModulesDataTable::ModulesDataTable(QWidget* pParent,
                                   const gtVector<TableContextMenuActionType>& additionalContextMenuActions,
                                   SessionTreeNodeData* pSessionData,
                                   CpuSessionWindow* pSessionWindow) :
    CPUProfileDataTable(pParent, additionalContextMenuActions, pSessionData),
    m_pParentSessionWindow(pSessionWindow)
{
}

ModulesDataTable::~ModulesDataTable()
{
}

bool ModulesDataTable::findModuleFilePath(int rowIndex, QString& moduleFileName)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
    {
        QTableWidgetItem* pItem = item(rowIndex, m_moduleNameColumn);

        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            // Get the module file path (the full path is stored in the tooltip):
            moduleFileName = pItem->toolTip();

            // For system modules - remove the "System" postfix from the tooltip:
            if (moduleFileName.endsWith(" (System)"))
            {
                moduleFileName = moduleFileName.replace(" (System)", "");
            }

            retVal = true;
        }
    }

    return retVal;
}

int ModulesDataTable::getEmptyMsgItemColIndex() const
{
    return m_moduleNameColumn;
}

void ModulesDataTable::onAboutToShowContextMenu()
{
    //TODO: Is required ??
    // Call the base class implementation:
    CPUProfileDataTable::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT((m_pContextMenu != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        foreach (QAction* pAction, m_pContextMenu->actions())
        {
            if (pAction != nullptr)
            {
                bool isActionEnabled = true;

                if (pAction->data().isValid())
                {
                    TableContextMenuActionType actionType = (TableContextMenuActionType)pAction->data().toInt();

                    if (selectedItems().count() > 0)
                    {
                        // get the selected item (only one item)
                        QTableWidgetItem* pModuleItem = selectedItems().first();

                        //if its empty or is the "empty row" string item
                        if (nullptr == pModuleItem ||
                            (m_pEmptyRowTableItem && (pModuleItem->row() == m_pEmptyRowTableItem->row())) ||
                            (m_pOtherSamplesRowItem && (pModuleItem->row() == m_pOtherSamplesRowItem->row())))
                        {
                            isActionEnabled = false;
                        }
                        else if (columnCount() != 0 && (selectedItems().count() / columnCount()) > 1)
                        {
                            isActionEnabled = false;
                        }
                        else if (actionType == DISPLAY_MODULE_IN_FUNCTIONS_VIEW)
                        {
                            // Get the list of selected rows:
                            QList<int> selectedRows;

                            for (QTableWidgetItem* pItem : selectedItems())
                            {
                                if (!selectedRows.contains(pItem->row()))
                                {
                                    selectedRows << pItem->row();
                                }
                            }
                        }
                    }
                    else
                    {
                        // if no items selected
                        isActionEnabled = false;
                    }

                    // Enable / disable the action:
                    pAction->setEnabled(isActionEnabled);
                }
            }
        }
    }
}

CPUProfileDataTable::TableType ModulesDataTable::GetTableType() const
{
    return CPUProfileDataTable::PROCESSES_DATA_TABLE;
}

bool ModulesDataTable::fillSummaryTable(int counterIdx)
{
    bool retVal = false;

    m_moduleNameColumn = CXL_MOD_SUMMARY_MOD_NAME_COL;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) && (m_pDisplayFilter != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);
        GT_ASSERT(rc);

        AMDTProfileDataVec moduleProfileData;
        rc = m_pProfDataRdr->GetModuleSummary(counterDesc.at(counterIdx).m_id, moduleProfileData);
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (const auto& moduleData : moduleProfileData)
        {
            if (moduleData.m_sampleValue.empty() || moduleData.m_sampleValue.at(0).m_sampleCount <= 0.0)
            {
                continue;
            }

            bool isOther = (0 == moduleData.m_name.compare(L"other") && AMDT_PROFILE_ALL_MODULES == moduleData.m_moduleId);

            if (!isOther)
            {
                AMDTProfileModuleInfoVec moduleInfo;
                m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, moduleData.m_id, moduleInfo);

                if (moduleInfo.empty())
                {
                    continue;
                }

                QStringList list;

                osFilePath modulePath(moduleData.m_name);
                gtString moduleName;
                modulePath.getFileNameAndExtension(moduleName);

                list << moduleName.asASCIICharArray();

                // Insert blank sample and sample percent
                list << "" << "";

                addRow(list, nullptr);

                QTableWidgetItem* pNameItem = item(rowCount() - 1, CXL_MOD_SUMMARY_MOD_NAME_COL);
                QPixmap* pIcon = CPUProfileDataTable::moduleIcon(moduleInfo[0].m_path, !moduleInfo[0].m_is64Bit);

                if (pNameItem != nullptr && pIcon != nullptr)
                {
                    pNameItem->setIcon(QIcon(*pIcon));
                }

                // tooltip
                QTableWidgetItem* pModuleNameItem = item(rowCount() - 1, CXL_MOD_SUMMARY_MOD_NAME_COL);

                if (pModuleNameItem != nullptr)
                {
                    pModuleNameItem->setToolTip(moduleData.m_name.asASCIICharArray());
                }

                if (m_isCLU)
                {
                    SetSamplePercentColumnValue(rowCount() - 1,
                        CXL_MOD_SUMMARY_SAMPLE_COL,
                        moduleData.m_sampleValue.at(0).m_sampleCount);
                }
                else
                {
                    SetSampleColumnValue(rowCount() - 1,
                        CXL_MOD_SUMMARY_SAMPLE_COL,
                        moduleData.m_sampleValue.at(0).m_sampleCount);

                    SetSamplePercentColumnValue(rowCount() - 1,
                        CXL_MOD_SUMMARY_SAMPLE_PER_COL,
                        moduleData.m_sampleValue.at(0).m_sampleCountPercentage);
                }
            }
            else
            {
                int rowNum = m_pOtherSamplesRowItem->row();
                QTableWidgetItem* rowItem;
                QString tmpStr;

                // Set "other" row name column item
                tmpStr = CP_strOther;
                rowItem = item(rowNum, CXL_MOD_SUMMARY_MOD_NAME_COL);
                rowItem->setText(tmpStr);
                rowItem->setTextColor(QColor(Qt::gray));

                // Set empty icon for "other" row
                QPixmap emptyIcon;
                acSetIconInPixmap(emptyIcon, AC_ICON_EMPTY);
                rowItem->setIcon(QIcon(emptyIcon));

                // Set "other" row samples column item
                rowItem = item(rowNum, CXL_MOD_SUMMARY_SAMPLE_COL);
                rowItem->setText(tmpStr.setNum(moduleData.m_sampleValue.at(0).m_sampleCount));
                rowItem->setTextColor(QColor(Qt::gray));

                // Set "other" row percent column item
                rowItem = item(rowNum, CXL_MOD_SUMMARY_SAMPLE_PER_COL);
                rowItem->setText(tmpStr.setNum(moduleData.m_sampleValue.at(0).m_sampleCountPercentage));
                rowItem->setTextColor(QColor(Qt::gray));

                // Show "other" row
                setRowHidden(rowNum, false);
            }
        }

        delegateSamplePercent(CXL_MOD_SUMMARY_SAMPLE_PER_COL);

        if (m_isCLU)
        {
            if (m_pDisplayFilter->IsCLUPercentCaptionSet())
            {
                delegateSamplePercent(CXL_MOD_SUMMARY_SAMPLE_COL);
            }
            else
            {
                setItemDelegateForColumn(CXL_MOD_SUMMARY_SAMPLE_COL, &acNumberDelegateItem::Instance());
            }

            hideColumn(CXL_MOD_SUMMARY_SAMPLE_PER_COL);
        }
        else
        {
            setItemDelegateForColumn(CXL_MOD_SUMMARY_SAMPLE_COL, &acNumberDelegateItem::Instance());
        }

        setColumnWidth(CXL_MOD_SUMMARY_MOD_NAME_COL, MAX_MODULE_NAME_LEN);

        if (m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
        {
            m_pTableDisplaySettings->m_lastSortColumnCaption = horizontalHeaderItem(CXL_MOD_SUMMARY_SAMPLE_COL)->text();
        }

        setSortingEnabled(true);

        retVal = true;
    }

    return retVal;
}

bool ModulesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> processIdVec)
{
    bool retVal = false;

    m_moduleIdColumn = CXL_MOD_TAB_MOD_ID_COL;
    m_moduleNameColumn = CXL_MOD_TAB_MOD_NAME_COL;

    GT_IF_WITH_ASSERT((m_pProfDataRdr.get() != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        gtVector<AMDTProfileData> allModulesData;

        if (processIdVec.empty())
        {
            bool rc = m_pProfDataRdr->GetModuleProfileData(procId, modId, allModulesData);
            GT_ASSERT(rc);
        }
        else
        {
            gtVector<AMDTProfileData> moduleData;

            for (const auto& processId : processIdVec)
            {
                moduleData.clear();
                m_pProfDataRdr->GetModuleProfileData(processId, modId, moduleData);
                allModulesData.insert(allModulesData.end(), moduleData.begin(), moduleData.end());
            }

            if (processIdVec.size() > 1)
            {
                mergeProfileModuleData(allModulesData);
            }
        }

        // Compute total samples per counter for non-CLU profile.
        // (Not getting expected percentage from reporter layer. Need to fix this in DB layer later.)
        if (m_pDisplayFilter->GetProfileType() != AMDT_PROFILE_TYPE_CLU)
        {
            std::unordered_map<AMDTCounterId, double> sampleMap;

            // Compute the total samples per counter for given process.
            for (const auto& profileData : allModulesData)
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
            for (auto& profileData : allModulesData)
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

        AddRowToTable(allModulesData);

        HandleTBPPercentCol(CXL_MOD_TAB_TBP_SAMPLE_PER_COL);
        hideColumn(CXL_MOD_TAB_MOD_ID_COL);

        setColumnWidth(CXL_MOD_TAB_MOD_NAME_COL, MAX_MODULE_NAME_LEN);

        if (m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
        {
            m_pTableDisplaySettings->m_lastSortColumnCaption = horizontalHeaderItem(CXL_MOD_TAB_SAMPLE_START_COL)->text();
        }

        retVal = true;
    }

    return retVal;
}

bool ModulesDataTable::findModuleId(int rowIndex, AMDTModuleId& modId)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
    {
        QTableWidgetItem* pidWidget = item(rowIndex, m_moduleIdColumn);

        GT_IF_WITH_ASSERT(pidWidget != nullptr)
        {
            modId = pidWidget->text().toUInt();
            retVal = true;
        }
    }

    return retVal;
}

bool ModulesDataTable::AddRowToTable(const gtVector<AMDTProfileData>& allModulesData)
{
    bool retVal = false;

    if (!allModulesData.empty())
    {
        setSortingEnabled(false);

        for (const auto& profData : allModulesData)
        {
            double totalSamples = 0.0;

            for (const auto& sampleVal : profData.m_sampleValue)
            {
                totalSamples += sampleVal.m_sampleCount;
            }

            if (totalSamples <= 0.0)
            {
                continue;
            }

            QStringList list;

            // insert module id
            list << QString::number(profData.m_moduleId);

            AMDTProfileModuleInfoVec moduleInfoList;
            m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, moduleInfoList);

            if (moduleInfoList.empty())
            {
                continue;
            }

            // insert module name
            list << moduleInfoList.at(0).m_name.asASCIICharArray();

            // insert debug info load status
            QString symbols = moduleInfoList.at(0).m_foundDebugInfo ? "Loaded" : "Not Loaded";
            list << symbols;

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

            SetTableSampleCountAndPercent(rowCount() - 1, CXL_MOD_TAB_SAMPLE_START_COL, profData);

            int idxRole = static_cast<int>(Qt::UserRole + 0);

            SetIcon(moduleInfoList.at(0).m_path,
                    rowCount() - 1,
                    CXL_MOD_TAB_MOD_NAME_COL,
                    CXL_MOD_TAB_MOD_NAME_COL,
                    !moduleInfoList.at(0).m_is64Bit,
                    idxRole);

            retVal = true;
        }

        setSortingEnabled(true);
    }

    return retVal;
}

void ModulesDataTable::mergeProfileModuleData(gtVector<AMDTProfileData>& moduleData) const
{
    if (moduleData.empty())
    {
        return;
    }

    std::unordered_map<AMDTUInt64, AMDTProfileData> mIdProfileDataMap;

    for (const auto& elem : moduleData)
    {
        auto itr = mIdProfileDataMap.find(elem.m_moduleId);

        if (itr == mIdProfileDataMap.end())
        {
            mIdProfileDataMap.emplace(elem.m_moduleId, elem);
        }
        else
        {
            AMDTProfileData& profData = itr->second;
            int idx = 0;

            for (auto& counter : profData.m_sampleValue)
            {
                counter.m_sampleCount += elem.m_sampleValue.at(idx).m_sampleCount;
                counter.m_sampleCountPercentage += elem.m_sampleValue.at(idx).m_sampleCountPercentage;
                idx++;
            }
        }
    }

    moduleData.clear();

    for (const auto& elem : mIdProfileDataMap)
    {
        moduleData.push_back(elem.second);
    }
}