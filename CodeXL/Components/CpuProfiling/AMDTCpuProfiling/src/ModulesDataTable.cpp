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
    m_tableRowHasIcon = true;
}

ModulesDataTable::~ModulesDataTable()
{
}

bool ModulesDataTable::fillListData()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        // Clear the total values vector:
        m_totalDataValuesVector.clear();
        m_totalDataValuesVector.resize(m_pSessionDisplaySettings->m_totalValuesMap.size());

        NameModuleMap* pNameModuleMap = m_pProfileReader->getModuleMap();
        GT_IF_WITH_ASSERT(pNameModuleMap != nullptr)
        {
            retVal = true;

            gtVector<float> moduleDataVector;

            // For each module
            for (NameModuleMap::const_iterator it = pNameModuleMap->begin(), itEnd = pNameModuleMap->end(); it != itEnd; ++it)
            {
                // Get the current module:
                const CpuProfileModule* pCurrentModule = &(it->second);

                if (pCurrentModule->isIndirect())
                {
                    continue;
                }

                // Check if this modules should be displayed or filtered:
                bool shouldDisplayModule = shouldModuleBeDisplayed(pCurrentModule);

                if (shouldDisplayModule)
                {
                    // Add this module to the table:
                    bool rc = addModuleItem(pCurrentModule);
                    GT_ASSERT(rc);

                    retVal &= rc;
                }
            }
        }
    }

    retVal &= CPUProfileDataTable::fillListData();

    return retVal;
}

bool ModulesDataTable::addModuleItem(const CpuProfileModule* pModule)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((pModule != nullptr) && (m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        QStringList moduleItemStringList;
        QString modulePathStr = acGTStringToQString(pModule->getPath());
        osFilePath modulePath;
        retVal = true;

        if (!pModule->m_isImdRead)
        {
            m_pParentSessionWindow->getModuleDetail(modulePathStr);
        }

        for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            switch (m_pTableDisplaySettings->m_displayedColumns[i])
            {

                case TableDisplaySettings::MODULE_NAME_COL:
                {
                    modulePath.setFullPathFromString(pModule->getPath());
                    gtString moduleName;
                    modulePath.getFileNameAndExtension(moduleName);
                    QString modName = acGTStringToQString(moduleName);
                    moduleItemStringList << modName;
                    break;
                }

                case TableDisplaySettings::MODULE_SYMBOLS_LOADED:
                {
                    // Check if the symbols are loaded for this module:
                    bool isModuleUnknown = ((pModule->m_modType == CpuProfileModule::UNKNOWNMODULE) || (pModule->m_modType == CpuProfileModule::UNKNOWNKERNELSAMPLES));
                    QString symbolsStr = (pModule->m_symbolsLoaded && !isModuleUnknown) ? CP_strLoaded : CP_strNotLoaded;
                    moduleItemStringList << symbolsStr;
                    break;
                }

                default:
                {
                    // Field will be re-calculated in post process:
                    moduleItemStringList << "";
                    retVal = true;
                    break;
                }
            }
        }

        // Add dummy values for all the data columns:
        // The real data is set in collectModuleDisplayedDataColumnValues:
        bool showPercentSeperateColumns = IsShowSeperatePercentColumns();

        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
        {
            for (unsigned int i = 0; i < m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size(); i++)
            {
                moduleItemStringList << "";

                if (showPercentSeperateColumns)
                {
                    moduleItemStringList << "";
                }
            }
        }

        // Get the icon for this file:
        QPixmap* pIcon = CPUProfileDataTable::moduleIcon(modulePath, pModule->m_is32Bit);

        // Add the module row:
        retVal = addRow(moduleItemStringList, pIcon, Qt::AlignVCenter | Qt::AlignLeft);
        GT_IF_WITH_ASSERT(retVal)
        {
            int rowIndex = rowCount() - 1;

            // Collect the data for the displayed data columns:
            gtVector<float> moduleDataVector;
            bool rc = collectModuleDisplayedDataColumnValues(pModule, moduleDataVector);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the values for this row:
                setTableDisplayedColumnsValues(rowIndex, moduleDataVector);
            }

            // Set items tooltips;
            for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
            {
                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::MODULE_NAME_COL)
                {
                    QTableWidgetItem* pItem = item(rowIndex, i);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        if (pModule->isSystemModule())
                        {
                            pItem->setToolTip(modulePathStr + " (System)");
                        }
                        else
                        {
                            pItem->setToolTip(modulePathStr);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

bool ModulesDataTable::collectModuleDisplayedDataColumnValues(const CpuProfileModule* pModule, gtVector<float>& moduleDataVector)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (pModule != nullptr) && (m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {

        const int dataColsSize = m_pSessionDisplaySettings->m_availableDataColumnCaptions.size();

        if (dataColsSize > 0)
        {
            gtVector<float> pidDataVector;
            CacheFileMap cached;

            // Prepare module data:
            moduleDataVector.clear();
            moduleDataVector.resize(dataColsSize + 1);

            // For each process
            PidAggregatedSampleMap::const_iterator pait = pModule->getBeginSample();
            PidAggregatedSampleMap::const_iterator pa_end = pModule->getEndSample();

            for (; pait != pa_end; ++pait)
            {
                // Get the current pid:
                ProcessIdType pid = pait->first;

                // Check if this pid should be calculated:
                bool shouldPIDBeCalculated = true;

                if (!m_pTableDisplaySettings->m_filterByPIDsList.isEmpty())
                {
                    shouldPIDBeCalculated = m_pTableDisplaySettings->m_filterByPIDsList.contains(pid);
                }

                // Add this pid samples if it is not filtered:
                if (shouldPIDBeCalculated)
                {
                    // Prepare process data
                    pidDataVector.clear();
                    pidDataVector.resize(dataColsSize + 1);
                    CPUProfileUtils::ConvertAggregatedSampleToArray(pait->second, pidDataVector, m_totalDataValuesVector, m_pSessionDisplaySettings, false);

                    // Add the data for this pid to the module data array:
                    CPUProfileUtils::AddDataArrays(moduleDataVector, pidDataVector);
                }
            }


            if (m_pSessionDisplaySettings->m_displayClu)
            {
                CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, moduleDataVector);
            }
        }

        retVal = true;
    }

    return retVal;
}



bool ModulesDataTable::findModuleFilePath(int moduleRowIndex, QString& moduleFileName)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
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

bool ModulesDataTable::shouldModuleBeDisplayed(const CpuProfileModule* pModule)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT((pModule != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        // Check if the module should be filtered by PID:
        if (!m_pTableDisplaySettings->m_filterByPIDsList.isEmpty())
        {
            retVal = false;

            // Go through the list of PIDs for filter, and check against each of them if it is sampled in the current module:
            foreach (ProcessIdType pid, m_pTableDisplaySettings->m_filterByPIDsList)
            {
                // Check if the requested filter PID is sampled in this module:
                PidAggregatedSampleMap::const_iterator iter = pModule->findSampleForPid(pid);

                if (iter != pModule->getEndSample())
                {
                    retVal = true;
                    break;
                }
            }
        }

        // Filter out system dlls:
        if (retVal && !CPUGlobalDisplayFilter::instance().m_displaySystemDLLs)
        {
            // Check if this is a system module:
            retVal = !AuxIsSystemModule(pModule->getPath());
        }
    }

    return retVal;
}

void ModulesDataTable::onAboutToShowContextMenu()
{
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

bool ModulesDataTable::AreModuleSymbolsLoaded(int moduleRowIndex)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        // Check if there is a column stating if the symbols are loaded for the module:
        int symbolsLoadedColumn = -1;

        for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::MODULE_SYMBOLS_LOADED)
            {
                symbolsLoadedColumn = i;
                break;
            }
        }

        if (symbolsLoadedColumn >= 0)
        {
            QTableWidgetItem* pModuleLoadedColItem = item(moduleRowIndex, symbolsLoadedColumn);
            {
                // If one of the selected modules is loaded, the action should be enabled:
                retVal = (pModuleLoadedColItem->text() == CP_strLoaded);
            }
        }
        else
        {
            // Get the module file path:
            QString moduleFilePath;
            retVal = findModuleFilePath(moduleRowIndex, moduleFilePath);
            GT_IF_WITH_ASSERT(retVal)
            {
                // Get the module structure:
                CpuProfileModule* pModule = m_pParentSessionWindow->getModuleDetail(moduleFilePath);
                GT_IF_WITH_ASSERT(pModule != nullptr)
                {
                    // Only known modules has loaded symbols:
                    retVal = ((pModule->m_modType != CpuProfileModule::UNKNOWNMODULE) && (pModule->m_modType != CpuProfileModule::UNKNOWNKERNELSAMPLES));
                }
            }
        }

        // Get the module
    }

    return retVal;
}

CPUProfileDataTable::TableType ModulesDataTable::GetTableType() const
{
    return CPUProfileDataTable::PROCESSES_DATA_TABLE;
}