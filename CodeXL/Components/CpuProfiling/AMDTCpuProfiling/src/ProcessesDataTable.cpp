//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessesDataTable.cpp
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
#include <inc/DataTab.h>
#include <inc/CPUProfileUtils.h>
#include <inc/ProcessesDataTable.h>
#include <inc/SessionWindow.h>
#include <inc/SessionOverviewWindow.h>
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

    if (nullptr != m_pProfDataRdr)
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);

        AMDTProfileDataVec processProfileData;
        rc = m_pProfDataRdr->GetProcessSummary(counterDesc.at(counterIdx).m_id,
                                               processProfileData);

        if (rc)
        {
            setSortingEnabled(false);

            for (auto profData : processProfileData)
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

                if (false == SetSampleCountAndPercent(profData.m_sampleValue, list))
                {
                    continue;
                }

                addRow(list, nullptr);

                // for summary table
                SetDelegateItemColumn(PROCESS_SAMPLE_COL, true);

            }

            setSortingEnabled(true);
            setColumnWidth(PROCESS_NAME_COL, MAX_PROCESS_NAME_LEN);
            retVal = true;
        }
    }

    return retVal;
}

bool ProcessesDataTable::findProcessDetails(int rowIndex, ProcessIdType& pid, QString& processFileName)
{
    bool retVal = true;
    QTableWidgetItem* pidWidget = item(rowIndex, 1);
    int pidInt = pidWidget->text().toInt();
    pid = pidInt;
    (void)processFileName;

    QTableWidgetItem* qPid = item(rowIndex, 1);
    QTableWidgetItem* qProcName = item(rowIndex, 0);

    pid = qPid->text().toInt();
    processFileName = qProcName->text();

    return retVal;
}


CPUProfileDataTable::TableType ProcessesDataTable::GetTableType() const
{
    return CPUProfileDataTable::PROCESSES_DATA_TABLE;
}

bool ProcessesDataTable::AddRowToTable(const gtVector<AMDTProfileData>& allProcessData)
{
    setSortingEnabled(false);

    bool retVal = false;

    if (!allProcessData.empty())
    {
        for (auto profData : allProcessData)
        {
            QStringList list;

            CounterNameIdVec selectedCounterList;

            AMDTProfileProcessInfoVec procInfo;
            m_pProfDataRdr->GetProcessInfo(profData.m_id, procInfo);

            if (procInfo.empty())
            {
                continue;
            }


            list << procInfo.at(0).m_name.asASCIICharArray();
            list << QString::number(procInfo.at(0).m_pid);

            SetTableSampleCountAndPercent(list, PROCESS_ID_COL, profData);
            addRow(list, nullptr);
        }

        retVal = true;
    }

    setSortingEnabled(true);
    return retVal;
}

bool ProcessesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    (void)modIdVec;
    bool retVal = false;

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

            mergedProfileDataVectors(allProcessData);
        }

        AddRowToTable(allProcessData);
        IfTbpSetPercentCol(PROCESS_TBP_PER_COL);
        setColumnWidth(PROCESS_NAME_COL, MAX_PROCESS_NAME_LEN);

        retVal = true;
    }

    return retVal;
}