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

    GT_IF_WITH_ASSERT((m_pContextMenu != nullptr) &&
                      (m_pTableDisplaySettings != nullptr) &&
                      (m_pParentSessionWindow != nullptr) &&
                      (m_pProfDataRdr != nullptr))
    {
        foreach (QAction* pAction, m_pContextMenu->actions())
        {
            if (pAction != nullptr)
            {
                bool isActionEnabled = false;

                if (pAction->data().isValid())
                {
                    TableContextMenuActionType actionType = (TableContextMenuActionType)pAction->data().toInt();

                    if (selectedItems().count() > 0)
                    {
                        AMDTFunctionId funcId = INVALID_FUNCTION_ID;

                        // get the selected item (only one item)
                        QTableWidgetItem* pItem = selectedItems().first();

                        if (nullptr != pItem)
                        {
                            int rowIndex = pItem->row();
                            gtString funcName = acQStringToGTString(getFunctionName(rowIndex));
                            QString funcIdStr = getFunctionId(rowIndex);
                            funcId = funcIdStr.toInt();

                            // if its empty or is the "empty row" string item
                            if ((pItem->row() == -1) ||
                                (funcName.startsWith(L"Unknown Module")) ||
                                ((columnCount() != 0 &&
                                  (selectedItems().count() / columnCount()) > 1)))
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
                        }
                    }

                    // Enable / disable the action:
                    pAction->setEnabled(isActionEnabled);
                }
            }
        }
    }
}

CPUProfileDataTable::TableType FunctionsDataTable::GetTableType() const
{
    return CPUProfileDataTable::FUNCTION_DATA_TABLE;
}

bool FunctionsDataTable::fillSummaryTables(int counterIdx)
{
    bool retVal = false;

    // Set the columns for function summary table
    m_functionIdColumn = CXL_FUNC_SUMMMARY_FUNC_ID_COL;
    m_functionNameColumn = CXL_FUNC_SUMMMARY_FUNC_NAME_COL;
    m_moduleNameColumn = CXL_FUNC_SUMMMARY_MODULE_COL;

    if (nullptr != m_pProfDataRdr)
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);

        AMDTProfileDataVec funcProfileData;
        rc = m_pProfDataRdr->GetFunctionSummary(counterDesc.at(counterIdx).m_id, funcProfileData);
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (auto profData : funcProfileData)
        {
            // Create QstringList to hold the values
            QStringList list;

            // Insert the function id
            list << QString::number(profData.m_id);

            // Insert the function name
            list << profData.m_name.asASCIICharArray();

            //Insert sample and sample percent
            if (!SetSampleCountAndPercent(profData.m_sampleValue, list))
            {
                continue;
            }

            // Insert module name
            AMDTProfileModuleInfoVec procInfo;
            rc = m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, procInfo);

            if (procInfo.empty())
            {
                continue;
            }

            if (profData.m_moduleId == AMDT_PROFILE_ALL_MODULES)
            {
                list << "";
            }
            else
            {
                list << procInfo.at(0).m_name.asASCIICharArray();
            }

            addRow(list, nullptr);

            // for summary table
            SetDelegateItemColumn(CXL_FUNC_SUMMMARY_SAMPLE_COL, true);

            if (!SetSummaryTabIcon(CXL_FUNC_SUMMMARY_FUNC_NAME_COL,
                                   CXL_FUNC_SUMMMARY_SAMPLE_PER_COL,
                                   CXL_FUNC_SUMMMARY_SAMPLE_COL,
                                   profData.m_moduleId,
                                   procInfo.at(0).m_path))
            {
                continue;
            }

            // Set tooltip for module column
            QString modulefullPath(acGTStringToQString(procInfo.at(0).m_path));
            QTableWidgetItem* pModuleNameItem = item(rowCount() - 1, CXL_FUNC_SUMMMARY_MODULE_COL);

            if (pModuleNameItem != nullptr)
            {
                pModuleNameItem->setToolTip(modulefullPath);
            }
        }

        hideColumn(CXL_FUNC_SUMMMARY_FUNC_ID_COL);

        setColumnWidth(CXL_FUNC_SUMMMARY_FUNC_NAME_COL, MAX_FUNCTION_NAME_LEN);
        resizeColumnToContents(CXL_FUNC_SUMMMARY_SAMPLE_COL);
        resizeColumnToContents(CXL_FUNC_SUMMMARY_SAMPLE_PER_COL);
        resizeColumnToContents(CXL_FUNC_SUMMMARY_MODULE_COL);

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

            SetTableSampleCountAndPercent(list, CXL_FUNC_TAB_SAMPLE_START_COL, profData);
            addRow(list, nullptr);

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

        IfTbpSetPercentCol(CXL_FUNC_TAB_TBP_SAMPLE_PER_COL);
        AddRowToTable(functionProfileData);
        hideColumn(CXL_FUNC_TAB_FUNC_ID_COL);

        setColumnWidth(CXL_FUNC_TAB_FUNC_NAME_COL, MAX_FUNCTION_NAME_LEN);
        setColumnWidth(CXL_FUNC_TAB_MOD_NAME_COL, MAX_MODULE_NAME_LEN);

        retVal = true;
    }

    return retVal;
}
