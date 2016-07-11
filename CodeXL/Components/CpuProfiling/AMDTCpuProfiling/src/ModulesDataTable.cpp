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

#if 0
                            // Enable the action if one of the selected moeuls has loaded symbols:
                            isActionEnabled = false;

                            for (int rowIndex : selectedRows)
                            {
                                if (AreModuleSymbolsLoaded(rowIndex))
                                {
                                    isActionEnabled = true;
                                    break;
                                }
                            }

#endif
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

            // TODO: to get Function name instead of complete path.
            osFilePath modulePath(moduleData.m_name);
            gtString filename;
            gtString extName;
            gtString seperator(L".");

            modulePath.getFileName(filename);
            modulePath.getFileExtension(extName);

            filename += seperator;
            filename += extName;

            list << filename.asASCIICharArray();

            QString modulefullPath(moduleData.m_name.asASCIICharArray());

            QVariant sampleCount(moduleData.m_sampleValue.at(0).m_sampleCount);
            list << sampleCount.toString();

            QVariant sampleCountPercent(moduleData.m_sampleValue.at(0).m_sampleCountPercentage);
            list << QString::number(moduleData.m_sampleValue.at(0).m_sampleCountPercentage, 'f', 2);

            addRow(list, nullptr);

            AMDTProfileModuleInfoVec procInfo;
            m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, moduleData.m_moduleId, procInfo);

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

            rc = delegateSamplePercent(AMDT_MOD_TABLE_SUMMARY_SAMPLE_PER);
        }

        setSortingEnabled(true);
        resizeColumnToContents(AMDT_MOD_TABLE_SUMMARY_MOD_NAME);
        retVal = true;
    }

    return retVal;
}

bool ModulesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    (void)modIdVec; //unused
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfDataRdr.get() != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        // get samples for Data cache access events
        AMDTProfileSessionInfo sessionInfo;

        bool rc = m_pProfDataRdr->GetProfileSessionInfo(sessionInfo);
        GT_ASSERT(rc);

        gtVector<AMDTProfileData> allProcessData;
        rc = m_pProfDataRdr->GetModuleProfileData(procId, modId, allProcessData);
        GT_ASSERT(rc);

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
            list << acGTStringToQString(procInfo.at(0).m_name);

            // TODO : need to discuss
            gtString symbols = procInfo.at(0).m_loadAddress ? L"Loaded" : L"Not Loaded";
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
                        list << QString::number(profData.m_sampleValue.at(i++).m_sampleCountPercentage, 'f', 2);
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
                        QVariant sampleCount(sampleCnt);
                        list << sampleCount.toString();
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

        }

        hideColumn(AMDT_MOD_TABLE_MOD_ID);
        resizeColumnToContents(AMDT_MOD_TABLE_MOD_NAME);
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
