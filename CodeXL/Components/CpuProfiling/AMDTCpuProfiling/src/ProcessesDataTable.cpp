//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessesDataTable.cpp
///
//==================================================================================

/// Local:
#include <inc/ProcessesDataTable.h>
#include <inc/StringConstants.h>


ProcessesDataTable::ProcessesDataTable(QWidget* pParent, const gtVector<TableContextMenuActionType>& additionalContextMenuActions, SessionTreeNodeData* pSessionData)
    : CPUProfileDataTable(pParent, additionalContextMenuActions, pSessionData)
{
}

ProcessesDataTable::~ProcessesDataTable()
{
}

bool ProcessesDataTable::fillSummaryTables(int counterIdx)
{
    bool retVal = false;

    m_processIdColumn = CXL_PROC_SUMMMARY_PROC_ID_COL;
    m_processNameColumn = CXL_PROC_SUMMMARY_PROC_NAME_COL;

    if (nullptr != m_pProfDataRdr)
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);

        AMDTProfileDataVec processProfileData;
        rc = m_pProfDataRdr->GetProcessSummary(counterDesc.at(counterIdx).m_id, processProfileData);

        if (rc)
        {
            setSortingEnabled(false);

            for (const auto& profData : processProfileData)
            {
                AMDTProfileProcessInfoVec procInfo;
                rc = m_pProfDataRdr->GetProcessInfo(profData.m_id, procInfo);

                QStringList list;

                if (procInfo.empty())
                {
                    continue;
                }

                if (profData.m_id != AMDT_PROFILE_ALL_MODULES)
                {
                    list << procInfo.at(0).m_name.asASCIICharArray();
                    list << QString::number(procInfo.at(0).m_pid);
                }
                else
                {
                    list << CP_strOther;
                    list << "";
                }

                if (!SetSampleCountAndPercent(profData.m_sampleValue, list))
                {
                    continue;
                }

                addRow(list, nullptr);

                // for summary table
                SetDelegateItemColumn(CXL_PROC_SUMMMARY_SAMPLE_COL, true);

            }

            setSortingEnabled(true);
            setColumnWidth(CXL_PROC_SUMMMARY_PROC_NAME_COL, MAX_PROCESS_NAME_LEN);
            retVal = true;
        }
    }

    return retVal;
}

bool ProcessesDataTable::findProcessDetails(int rowIndex, ProcessIdType& pid, QString& processFileName)
{
    QTableWidgetItem* pidWidget = item(rowIndex, m_processIdColumn);
    pid = pidWidget->text().toUInt();

    QTableWidgetItem* procNameWidget = item(rowIndex, m_processNameColumn);
    processFileName = procNameWidget->text();

    return true;
}

CPUProfileDataTable::TableType ProcessesDataTable::GetTableType() const
{
    return CPUProfileDataTable::PROCESSES_DATA_TABLE;
}

bool ProcessesDataTable::AddRowToTable(const gtVector<AMDTProfileData>& allProcessData)
{
    bool retVal = false;

    setSortingEnabled(false);

    if (!allProcessData.empty())
    {
        for (const auto& profData : allProcessData)
        {
            QStringList list;

            AMDTProfileProcessInfoVec procInfo;
            m_pProfDataRdr->GetProcessInfo(profData.m_id, procInfo);

            if (procInfo.empty())
            {
                continue;
            }

            list << procInfo.at(0).m_name.asASCIICharArray();
            list << QString::number(procInfo.at(0).m_pid);

            SetTableSampleCountAndPercent(list, CXL_PROC_TAB_SAMPLE_START_COL, profData);
            addRow(list, nullptr);
        }

        retVal = true;
    }

    setSortingEnabled(true);
    return retVal;
}

bool ProcessesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    bool retVal = false;

    m_processIdColumn = CXL_PROC_TAB_PROC_ID_COL;
    m_processNameColumn = CXL_PROC_TAB_PROC_NAME_COL;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        gtVector<AMDTProfileData> allProcessData;

        if (modIdVec.empty())
        {
            bool rc = m_pProfDataRdr->GetProcessProfileData(procId, modId, allProcessData);
            GT_ASSERT(rc);
        }
        else
        {
            gtVector<AMDTProfileData> moduleData;

            for (const auto& moduleId : modIdVec)
            {
                moduleData.clear();
                bool rc = m_pProfDataRdr->GetProcessProfileData(procId, moduleId, moduleData);
                GT_ASSERT(rc);
                allProcessData.insert(allProcessData.end(), moduleData.begin(), moduleData.end());
            }

            if (modIdVec.size() > 1)
            {
                mergeProfileProcessData(allProcessData);
            }
        }

        IfTbpSetPercentCol(CXL_PROC_TAB_TBP_SAMPLE_PER_COL);
        AddRowToTable(allProcessData);

        setColumnWidth(CXL_PROC_TAB_PROC_NAME_COL, MAX_PROCESS_NAME_LEN);

        retVal = true;
    }

    return retVal;
}

void ProcessesDataTable::mergeProfileProcessData(gtVector<AMDTProfileData>& processData) const
{
    if (processData.empty())
    {
        return;
    }

    std::map<AMDTUInt64, AMDTProfileData> mIdProfileDataMap;

    for (const auto& elem : processData)
    {
        auto itr = mIdProfileDataMap.find(elem.m_id);

        if (itr == mIdProfileDataMap.end())
        {
            mIdProfileDataMap.insert(std::make_pair(elem.m_id, elem));
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

    processData.clear();

    for (const auto& elem : mIdProfileDataMap)
    {
        processData.push_back(elem.second);
    }
}
