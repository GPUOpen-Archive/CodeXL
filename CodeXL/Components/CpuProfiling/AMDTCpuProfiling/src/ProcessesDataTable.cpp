//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessesDataTable.cpp
///
//==================================================================================

// Infra:
#include <AMDTApplicationComponents/Include/acItemDelegate.h>

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

bool ProcessesDataTable::fillSummaryTable(int counterIdx)
{
    bool retVal = false;

    m_processIdColumn = CXL_PROC_SUMMARY_PROC_ID_COL;
    m_processNameColumn = CXL_PROC_SUMMARY_PROC_NAME_COL;

    if (nullptr != m_pProfDataRdr && nullptr != m_pDisplayFilter && nullptr != m_pTableDisplaySettings)
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
                bool isOther = (profData.m_name.compare(L"other") == 0);

                if (!isOther)
                {
                    AMDTProfileProcessInfoVec procInfo;
                    rc = m_pProfDataRdr->GetProcessInfo(profData.m_id, procInfo);

                    if (procInfo.empty())
                    {
                        continue;
                    }

                    QStringList list;

                    list << procInfo.at(0).m_name.asASCIICharArray();

                    // Insert blank pid, sample and sample percent
                    list << "" << "" << "";

                    addRow(list, nullptr);

                    // tooltip
                    QTableWidgetItem* pNameItem = item(rowCount() - 1, CXL_PROC_SUMMARY_PROC_NAME_COL);

                    if (pNameItem != nullptr)
                    {
                        pNameItem->setToolTip(procInfo.at(0).m_path.asASCIICharArray());
                    }

                    SetPIDColumnValue(rowCount() - 1,
                        CXL_PROC_SUMMARY_PROC_ID_COL,
                        procInfo.at(0).m_pid);

                    if (m_isCLU)
                    {
                        SetSamplePercentColumnValue(rowCount() - 1,
                            CXL_PROC_SUMMARY_SAMPLE_COL,
                            profData.m_sampleValue.at(0).m_sampleCount);
                    }
                    else
                    {
                        SetSampleColumnValue(rowCount() - 1,
                            CXL_PROC_SUMMARY_SAMPLE_COL,
                            profData.m_sampleValue.at(0).m_sampleCount);

                        SetSamplePercentColumnValue(rowCount() - 1,
                            CXL_PROC_SUMMARY_SAMPLE_PER_COL,
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
                    rowItem = item(rowNum, CXL_PROC_SUMMARY_PROC_NAME_COL);
                    rowItem->setText(tmpStr);
                    rowItem->setTextColor(QColor(Qt::gray));

                    // Set "other" row samples column item
                    rowItem = item(rowNum, CXL_PROC_SUMMARY_SAMPLE_COL);
                    rowItem->setText(tmpStr.setNum(profData.m_sampleValue.at(0).m_sampleCount));
                    rowItem->setTextColor(QColor(Qt::gray));

                    // Set "other" row percent column item
                    rowItem = item(rowNum, CXL_PROC_SUMMARY_SAMPLE_PER_COL);
                    rowItem->setText(tmpStr.setNum(profData.m_sampleValue.at(0).m_sampleCountPercentage));
                    rowItem->setTextColor(QColor(Qt::gray));

                    // Show "other" row
                    setRowHidden(rowNum, false);
                }
            }

            delegateSamplePercent(CXL_PROC_SUMMARY_SAMPLE_PER_COL);

            if (m_isCLU)
            {
                if (m_pDisplayFilter->IsCLUPercentCaptionSet())
                {
                    delegateSamplePercent(CXL_PROC_SUMMARY_SAMPLE_COL);
                }
                else
                {
                    setItemDelegateForColumn(CXL_PROC_SUMMARY_SAMPLE_COL, &acNumberDelegateItem::Instance());
                }

                hideColumn(CXL_PROC_SUMMARY_SAMPLE_PER_COL);
            }
            else
            {
                setItemDelegateForColumn(CXL_PROC_SUMMARY_SAMPLE_COL, &acNumberDelegateItem::Instance());
            }

            setColumnWidth(CXL_PROC_SUMMARY_PROC_NAME_COL, MAX_PROCESS_NAME_LEN);

            if (m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
            {
                m_pTableDisplaySettings->m_lastSortColumnCaption = horizontalHeaderItem(CXL_PROC_SUMMARY_SAMPLE_COL)->text();
            }

            setSortingEnabled(true);

            retVal = true;
        }
    }

    return retVal;
}

bool ProcessesDataTable::findProcessDetails(int rowIndex, AMDTProcessId& pid, QString& processFileName)
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
            // insert blank pid item
            list << "";

            CounterNameIdVec selectedCounterList;
            m_pDisplayFilter->GetSelectedCounterList(selectedCounterList);

            for (size_t i = 0; i < selectedCounterList.size(); ++i)
            {
                // inert blank sample item
                list << "";
            }

            if (m_pDisplayFilter->GetProfileType() == AMDT_PROFILE_TYPE_TBP)
            {
                // insert blank sample percent item
                list << "";
            }

            addRow(list, nullptr);

            // Tooltip
            QTableWidgetItem* pNameItem = item(rowCount() - 1, CXL_PROC_TAB_PROC_NAME_COL);

            if (pNameItem != nullptr)
            {
                pNameItem->setToolTip(procInfo.at(0).m_path.asASCIICharArray());
            }

            SetPIDColumnValue(rowCount() - 1, CXL_PROC_TAB_PROC_ID_COL, procInfo.at(0).m_pid);
            SetTableSampleCountAndPercent(rowCount() - 1, CXL_PROC_TAB_SAMPLE_START_COL, profData);
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

        AddRowToTable(allProcessData);

        HandleTBPPercentCol(CXL_PROC_TAB_TBP_SAMPLE_PER_COL);

        setColumnWidth(CXL_PROC_TAB_PROC_NAME_COL, MAX_PROCESS_NAME_LEN);

        if (m_pTableDisplaySettings->m_lastSortColumnCaption.isEmpty())
        {
            m_pTableDisplaySettings->m_lastSortColumnCaption = horizontalHeaderItem(CXL_PROC_TAB_SAMPLE_START_COL)->text();
        }

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
