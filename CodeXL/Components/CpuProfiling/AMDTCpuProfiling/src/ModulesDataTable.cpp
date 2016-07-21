//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ModulesDataTable.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

/// Local:
#include <inc/CPUProfileUtils.h>
#include <inc/ModulesDataTable.h>
#include <inc/Auxil.h>
#include <inc/StringConstants.h>
#include <inc/SessionWindow.h>


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
bool ModulesDataTable::findModuleFilePath(int moduleRowIndex, QString& moduleFileName)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::MODULE_NAME_COL)
            {
                QTableWidgetItem* pItem = item(moduleRowIndex, i);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    // Get the module file path (the full path is stored in the tooltip):
                    moduleFileName = pItem->toolTip();

                    // For system dll's - remove the "System" postfix from the tooltip:
                    if (moduleFileName.endsWith(" (System)"))
                    {
                        moduleFileName = moduleFileName.replace(" (System)", "");
                    }

                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
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

                        else if (columnCount() != 0 &&
                                 (selectedItems().count() / columnCount()) > 1)
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

bool ModulesDataTable::fillSummaryTables(int counterIdx)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);
        GT_ASSERT(rc);

        AMDTProfileDataVec moduleProfileData;
        rc = m_pProfDataRdr->GetModuleSummary(counterDesc.at(counterIdx).m_id,
                                              moduleProfileData);
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (auto moduleData : moduleProfileData)
        {
            // create QstringList to hold the values
            QStringList list;

            osFilePath modulePath(moduleData.m_name);
            gtString filename;
            modulePath.getFileNameAndExtension(filename);

            list << filename.asASCIICharArray();

            QString modulefullPath(moduleData.m_name.asASCIICharArray());
            int precision = SAMPLE_VALUE_PRECISION;

            const AMDTSampleValueVec& sampleVector = moduleData.m_sampleValue;

            if (sampleVector.empty() || sampleVector.at(0).m_sampleCount == 0)
            {
                continue;
            }

            if (m_isCLU)
            {
                if (m_pDisplayFilter->isCLUPercentCaptionSet())
                {
                    precision = SAMPLE_PERCENT_PRECISION;
                }

                list << QString::number(sampleVector.at(0).m_sampleCount, 'f', precision);
            }
            else
            {
                QVariant sampleCount(sampleVector.at(0).m_sampleCount);
                list << sampleCount.toString();
                QVariant sampleCountPercent(sampleVector.at(0).m_sampleCountPercentage);
                list << QString::number(sampleVector.at(0).m_sampleCountPercentage, 'f', precision);
            }

            addRow(list, nullptr);

            AMDTProfileModuleInfoVec procInfo;
            m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, moduleData.m_moduleId, procInfo);

            // if module info null return
            if (procInfo.empty())
            {
                continue;
            }

            int row = rowCount() - 1;

            QPixmap* pIcon = CPUProfileDataTable::moduleIcon(modulePath, !procInfo.at(0).m_is64Bit);
            QTableWidgetItem* pNameItem = item(row, AMDT_MOD_TABLE_SUMMARY_MOD_NAME);

            if (pNameItem != nullptr)
            {
                if (pIcon != nullptr)
                {
                    pNameItem->setIcon(QIcon(*pIcon));
                }
            }

            QTableWidgetItem* pModuleNameItem = item(row, AMDT_MOD_TABLE_SUMMARY_MOD_NAME);

            if (pModuleNameItem != nullptr)
            {
                pModuleNameItem->setToolTip(moduleData.m_name.asASCIICharArray());
            }

            if (!m_isCLU)
            {
                rc = delegateSamplePercent(AMDT_MOD_TABLE_SUMMARY_SAMPLE_PER);
            }
            else
            {
                if (m_pDisplayFilter->isCLUPercentCaptionSet())
                {
                    delegateSamplePercent(AMDT_MOD_TABLE_CLU_HS_COL);
                }
                else
                {
                    setItemDelegateForColumn(AMDT_MOD_TABLE_CLU_HS_COL, &acNumberDelegateItem::Instance());
                }
            }
        }

        setSortingEnabled(true);
        setColumnWidth(AMDT_MOD_TABLE_SUMMARY_MOD_NAME, MAX_MODULE_NAME_LEN);
        retVal = true;
    }

    return retVal;
}

bool ModulesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> processIdVec)
{
    bool retVal = false;
    GT_UNREFERENCED_PARAMETER(modId);

    GT_IF_WITH_ASSERT((m_pProfDataRdr.get() != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        gtVector<AMDTProfileData> allProcessData;

        if (processIdVec.empty())
        {
            bool rc = m_pProfDataRdr->GetModuleProfileData(procId, modId, allProcessData);
            GT_ASSERT(rc);
        }
        else
        {
            gtVector<AMDTProfileData> processData;

            for (const auto& processId : processIdVec)
            {
                processData.clear();
                m_pProfDataRdr->GetModuleProfileData(processId, modId, processData);
                allProcessData.insert(allProcessData.end(), processData.begin(), processData.end());
            }

            mergedProfileModuleData(allProcessData);
        }

        AddRowToTable(allProcessData);

        hideColumn(AMDT_MOD_TABLE_MOD_ID);
        setColumnWidth(AMDT_MOD_TABLE_SUMMARY_MOD_NAME, MAX_MODULE_NAME_LEN);
        retVal = true;
    }

    return retVal;
}

bool ModulesDataTable::findModueId(int rowIndex, AMDTModuleId& modId)
{
    QTableWidgetItem* pidWidget = item(rowIndex, 0);
    int moduleId = pidWidget->text().toInt();
    modId = moduleId;

    return true;
}

bool ModulesDataTable::AddRowToTable(const gtVector<AMDTProfileData>& allProcessData)
{
    bool retVal = false;

    if (!allProcessData.empty())
    {
        setSortingEnabled(false);

        for (auto profData : allProcessData)
        {
            QStringList list;

            CounterNameIdVec selectedCounterList;

            // insert module id
            QVariant mId(profData.m_moduleId);
            list << mId.toString();

            AMDTProfileModuleInfoVec procInfo;
            m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, procInfo);

            if (procInfo.empty())
            {
                continue;
            }

            list << acGTStringToQString(procInfo.at(0).m_name);

            gtString symbols = procInfo.at(0).m_foundDebugInfo ? L"Loaded" : L"Not Loaded";

            list << acGTStringToQString(symbols);

            m_pDisplayFilter->GetSelectedCounterList(selectedCounterList);
            int i = 0;

            for (auto counter : selectedCounterList)
            {
                // get counter type
                AMDTProfileCounterType counterType = static_cast<AMDTProfileCounterType>(std::get<4>(counter));
                bool setSampleValue = true;

                if (counterType == AMDT_PROFILE_COUNTER_TYPE_RAW)
                {
                    if (m_pDisplayFilter->GetSamplePercent() == true)
                    {
                        list << QString::number(profData.m_sampleValue.at(i++).m_sampleCountPercentage, 'f', SAMPLE_PERCENT_PRECISION);
                        delegateSamplePercent(AMDT_MOD_TABLE_SYMBOL_LOADED + i);
                        setSampleValue = false;
                    }
                }

                if (true == setSampleValue)
                {
                    double sampleCnt = profData.m_sampleValue.at(i++).m_sampleCount;
                    setItemDelegateForColumn(AMDT_MOD_TABLE_SYMBOL_LOADED + i, &acNumberDelegateItem::Instance());

                    if (0 == sampleCnt)
                    {
                        list << "";
                    }
                    else
                    {
                        list << QString::number(sampleCnt, 'f', SAMPLE_VALUE_PRECISION);
                    }
                }
            }

            addRow(list, nullptr);

            AMDTProfileModuleInfoVec modInfo;
            m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, modInfo);

            int row = rowCount() - 1;

            QPixmap* pIcon = CPUProfileDataTable::moduleIcon(modInfo.at(0).m_path, !procInfo.at(0).m_is64Bit);
            QTableWidgetItem* pNameItem = item(row, AMDT_MOD_TABLE_MOD_NAME);

            if (pNameItem != nullptr)
            {
                if (pIcon != nullptr)
                {
                    pNameItem->setIcon(QIcon(*pIcon));
                }
            }

            QTableWidgetItem* pModuleNameItem = item(row, AMDT_MOD_TABLE_MOD_NAME);

            if (pModuleNameItem != nullptr)
            {
                pModuleNameItem->setToolTip(acGTStringToQString(modInfo.at(0).m_path));
            }

            retVal = true;
        }
    }

    return retVal;
}

void mergedProfileModuleData(gtVector<AMDTProfileData>& data)
{
    if (data.empty())
    {
        return;
    }

    // create map from longest vector
    std::map<AMDTUInt64, AMDTProfileData> mIdProfileDataMap;

    for (const auto& elem : data)
    {
        auto itr = mIdProfileDataMap.find(elem.m_moduleId);

        if (itr == mIdProfileDataMap.end())
        {
            mIdProfileDataMap.insert(std::make_pair(elem.m_moduleId, elem));
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

    data.clear();

    for (const auto& elem : mIdProfileDataMap)
    {
        data.push_back(elem.second);
    }
}