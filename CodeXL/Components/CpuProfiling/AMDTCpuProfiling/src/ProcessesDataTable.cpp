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
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (auto profData : processProfileData)
        {
            // get the process info
            AMDTProfileProcessInfoVec procInfo;
            rc = m_pProfDataRdr->GetProcessInfo(profData.m_id, procInfo);

            QStringList list;

            if (profData.m_name != L"other")
            {
                list << procInfo.at(0).m_name.asASCIICharArray();
                list << QString::number(procInfo.at(0).m_pid);
            }
            else
            {
                list << "other";
                list << "";
            }

            QVariant sampleCount(profData.m_sampleValue.at(0).m_sampleCount);
            list << sampleCount.toString();

            QVariant sampleCountPercent(profData.m_sampleValue.at(0).m_sampleCountPercentage);
            list << QString::number(profData.m_sampleValue.at(0).m_sampleCountPercentage, 'f', 2);

            addRow(list, nullptr);

            delegateSamplePercent(3);
        }

        setSortingEnabled(true);
        resizeColumnToContents(0);
        retVal = true;
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

#if 0
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        // Find the process ID column index:
        bool rc1 = false, rc2 = false;

        for (int i = 0 ; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::PID_COL)
            {
                QTableWidgetItem* pItem = item(rowIndex, i);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    // Get the process ID:
                    pid = pItem->text().toUInt(&retVal);
                    rc1 = true;
                }
            }

            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::PROCESS_NAME_COL)
            {
                QTableWidgetItem* pItem = item(rowIndex, i);
                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    // Get the process ID:
                    processFileName = pItem->text();
                    rc2 = true;
                }
            }

            retVal = rc1 && rc2;

            if (retVal)
            {
                break;
            }
        }
    }

#endif

    return retVal;
}


CPUProfileDataTable::TableType ProcessesDataTable::GetTableType() const
{
    return CPUProfileDataTable::PROCESSES_DATA_TABLE;
}

bool ProcessesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    (void)modIdVec;
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pDisplayFilter != nullptr) &&
                      (m_pTableDisplaySettings != nullptr))
    {
        // get samples for Data cache access events
        AMDTProfileSessionInfo sessionInfo;

        bool rc = m_pProfDataRdr->GetProfileSessionInfo(sessionInfo);
        GT_ASSERT(rc);

        gtVector<AMDTProfileData> allProcessData;
        rc = m_pProfDataRdr->GetProcessProfileData(procId, modId, allProcessData);
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (auto profData : allProcessData)
        {
            QStringList list;

            CounterNameIdVec selectedCounterList;

            AMDTProfileProcessInfoVec procInfo;
            rc = m_pProfDataRdr->GetProcessInfo(profData.m_id, procInfo);

            list << procInfo.at(0).m_name.asASCIICharArray();
            list << QString::number(procInfo.at(0).m_pid);


            m_pDisplayFilter->GetSelectedCounterList(selectedCounterList);
            int i = 0;

            for (auto counter : selectedCounterList)
            {
                QVariant sampleCount(profData.m_sampleValue.at(i++).m_sampleCount);
                list << sampleCount.toString();

            }

            addRow(list, nullptr);
        }

        setSortingEnabled(true);
        resizeColumnToContents(0);

        retVal = true;
    }

    return retVal;
}
