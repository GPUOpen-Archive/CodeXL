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

        retVal = true;
    }

    return retVal;
}

bool ProcessesDataTable::fillListData()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        m_pathToValuesMap.clear();

        PidProcessMap* pPidProcessMap = m_pProfileReader->getProcessMap();
        GT_IF_WITH_ASSERT(pPidProcessMap != nullptr)
        {
            retVal = true;

            m_totalDataValuesVector.clear();
            m_totalDataValuesVector.resize(m_pSessionDisplaySettings->m_totalValuesMap.size());

            // Go over the processes and add to the table:
            PidProcessMap::const_iterator pit = pPidProcessMap->begin();
            PidProcessMap::const_iterator p_end = pPidProcessMap->end();

            for (; pit != p_end; pit++)
            {
                // Get the current process details:
                ProcessIdType pid = pit->first;
                const CpuProfileProcess& currentProcess = pit->second;

                // Check if this process is filtered / not:
                bool shouldDisplayPID = shouldPIDBeDisplayed(pid);

                if (shouldDisplayPID)
                {
                    // Add this process to the table:
                    bool rc = addProcessItem(pid, currentProcess);
                    GT_ASSERT(rc);

                    retVal = retVal && rc;
                }
            }
        }
    }

    retVal &= CPUProfileDataTable::fillListData();
    return retVal;
}
bool ProcessesDataTable::addProcessItem(ProcessIdType pid, const CpuProfileProcess& process)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        QStringList processItemStringList;
        retVal = true;
        int pidItemColumn = -1;

        for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            switch (m_pTableDisplaySettings->m_displayedColumns[i])
            {

                case TableDisplaySettings::PROCESS_NAME_COL:
                {
                    osFilePath processPath(process.getPath());
                    gtString procName;
                    processPath.getFileNameAndExtension(procName);
                    QString processName = acGTStringToQString(procName);
                    processItemStringList << processName;
                    break;
                }

                case TableDisplaySettings::PID_COL:
                {
                    pidItemColumn = i;
                    // Do not set as string, so that column is sortable:
                    processItemStringList << "";
                    break;
                }

                default:
                {
                    // Field will be re-calculated in post process:
                    processItemStringList << "";
                    retVal = true;
                    break;
                }
            }
        }

        // Add dummy values for all the data columns:
        // The real data is set in setProcessDisplayedDataColumnValues:
        bool showPercentSeperateColumns = IsShowSeperatePercentColumns();

        if (m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
        {
            for (unsigned int i = 0; i < m_pSessionDisplaySettings->m_displayedDataColumnsIndices.size(); i++)
            {
                processItemStringList << "";

                if (showPercentSeperateColumns)
                {
                    processItemStringList << "";
                }
            }
        }

        // Add the module row:
        retVal = addRow(processItemStringList, nullptr, Qt::AlignVCenter | Qt::AlignLeft);
        GT_IF_WITH_ASSERT(retVal)
        {
            int rowIndex = rowCount() - 1;

            // Get the pid column and set the data (the data should be sortable):
            QTableWidgetItem* pPIDItem = item(rowIndex, pidItemColumn);

            if (pPIDItem != nullptr)
            {
                QVariant var;
                var.setValue(pid);
                pPIDItem->setData(Qt::DisplayRole, var);
            }

            // Collect the data for the displayed data columns:
            gtVector<float> processDataVector;
            bool rc = collectProcessDisplayedDataColumnValues(pid, process, rowIndex, processDataVector);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the values for this row:
                setTableDisplayedColumnsValues(rowIndex, processDataVector);

                // Add to the totals vector:
                CPUProfileUtils::AddDataArrays(m_totalDataValuesVector, processDataVector);
            }

            // Set items tooltips;
            for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
            {
                if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::PROCESS_NAME_COL)
                {
                    QString processPath = acGTStringToQString(process.getPath());
                    QTableWidgetItem* pItem = item(rowIndex, i);
                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        pItem->setToolTip(processPath);
                    }
                }
            }
        }
    }

    return retVal;
}


bool ProcessesDataTable::collectProcessDisplayedDataColumnValues(ProcessIdType pid, const CpuProfileProcess& process, int rowIndex, gtVector<float>& processDataVector)
{
    (void)(process); // unused
    (void)(rowIndex); // unused

    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        NameModuleMap* pNameModuleMap = m_pProfileReader->getModuleMap();
        GT_IF_WITH_ASSERT(pNameModuleMap != nullptr)
        {
            const int dataColsSize = m_pSessionDisplaySettings->m_availableDataColumnCaptions.size();

            if (dataColsSize > 0)
            {
                gtVector<float> moduleDataVector;
                CacheFileMap cached;

                // Prepare module data:
                processDataVector.clear();
                processDataVector.resize(dataColsSize + 1);

                gtVector<float> totalsDataVector;
                totalsDataVector.clear();
                totalsDataVector.resize(dataColsSize + 1);

                // For each module
                for (NameModuleMap::const_iterator mit = pNameModuleMap->begin(), mEnd = pNameModuleMap->end(); mit != mEnd; ++mit)
                {
                    const CpuProfileModule& module = mit->second;

                    if (module.isIndirect())
                    {
                        continue;
                    }

                    // Find pid:
                    PidAggregatedSampleMap::const_iterator pait = module.findSampleForPid(pid);

                    if (module.getEndSample() == pait)
                    {
                        continue;
                    }

                    // Prepare module data vector:
                    moduleDataVector.clear();
                    moduleDataVector.resize(dataColsSize + 1);
                    CPUProfileUtils::ConvertAggregatedSampleToArray(pait->second, moduleDataVector, totalsDataVector, m_pSessionDisplaySettings, false);

                    CPUProfileUtils::AddDataArrays(processDataVector, moduleDataVector);
                }


                if (m_pSessionDisplaySettings->m_displayClu)
                {
                    CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, processDataVector);
                }

                retVal = true;
            }
        }
    }

    return retVal;
}


bool ProcessesDataTable::calculateProcessSamplesCount(ProcessIdType pid, QString& sampleCountStr, gtUInt32& sampleCount)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfileReader != nullptr) && (m_pSessionDisplaySettings != nullptr))
    {
        NameModuleMap* pNameModuleMap = m_pProfileReader->getModuleMap();
        GT_IF_WITH_ASSERT(pNameModuleMap != nullptr)
        {
            gtVector<float> processDataVector;
            gtVector<float> moduleDataVector;
            gtVector<float> totalDataVector;
            const int available = m_pSessionDisplaySettings->m_availableDataColumnCaptions.size();

            totalDataVector.clear();
            totalDataVector.resize(m_pSessionDisplaySettings->m_totalValuesMap.size());

            // Prepare module data
            moduleDataVector.clear();
            moduleDataVector.resize(available + 1);

            // For each module
            for (NameModuleMap::const_iterator mit = pNameModuleMap->begin(), mEnd = pNameModuleMap->end(); mit != mEnd; ++mit)
            {
                const CpuProfileModule& module = mit->second;

                if (module.isIndirect())
                {
                    continue;
                }

                // Find pid
                PidAggregatedSampleMap::const_iterator pait = module.findSampleForPid(pid);

                if (module.getEndSample() == pait)
                {
                    continue;
                }

                // For each process
                PidAggregatedSampleMap::const_iterator moduleIt = module.getBeginSample();
                PidAggregatedSampleMap::const_iterator moduleItEnd = module.getEndSample();

                sampleCount = 0;

                for (; moduleIt != moduleItEnd; ++moduleIt)
                {
                    // Prepare process data
                    processDataVector.clear();
                    processDataVector.resize(available + 1);
                    gtUInt32 current = CPUProfileUtils::ConvertAggregatedSampleToArray(moduleIt->second, processDataVector, totalDataVector, m_pSessionDisplaySettings);
                    sampleCount += current;

                }
            }

            sampleCountStr.sprintf("%u", sampleCount);
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

	QTableWidgetItem *qPid = item(rowIndex, 1);
	QTableWidgetItem *qProcName = item(rowIndex, 0);

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

bool ProcessesDataTable::shouldPIDBeDisplayed(ProcessIdType pid)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        if (!m_pTableDisplaySettings->m_filterByModulePathsList.isEmpty())
        {
            retVal = false;

            // Iterate through parents until data tab parent is found:
            DataTab* pParentTab = nullptr;
            QObject* pParent = parent();

            while ((pParent != nullptr) && (pParentTab == nullptr))
            {
                pParentTab = qobject_cast<DataTab*>(pParent);
                pParent = pParent->parent();
            }

            // Sanity check:
            GT_IF_WITH_ASSERT(pParentTab != nullptr)
            {
                GT_IF_WITH_ASSERT(pParentTab->parentSessionWindow() != nullptr)
                {
                    GT_IF_WITH_ASSERT(pParentTab->parentSessionWindow()->sessionOverviewWindow() != nullptr)
                    {
                        // Get the session overview window:
                        SessionOverviewWindow* pOverviewWindow = pParentTab->parentSessionWindow()->sessionOverviewWindow();
                        GT_IF_WITH_ASSERT(pOverviewWindow != nullptr)
                        {
                            // Go through all the modules in the filter list, and check for each if it is used by the requested process:
                            for (int i = 0, e = static_cast<int>(m_pTableDisplaySettings->m_filterByModulePathsList.size()); i < e; i++)
                            {
                                osFilePath filePath(acQStringToGTString(m_pTableDisplaySettings->m_filterByModulePathsList[i]));
                                const CpuProfileModule* pModule = pOverviewWindow->findModuleHandler(filePath);

                                if (nullptr != pModule)
                                {
                                    // Go through each of the samples in the module, and see if this process exists:
                                    PidAggregatedSampleMap::const_iterator pait = pModule->getBeginSample();
                                    PidAggregatedSampleMap::const_iterator paEnd = pModule->getEndSample();

                                    for (; pait != paEnd; ++pait)
                                    {
                                        if (pait->first == pid)
                                        {
                                            retVal = true;
                                            break;
                                        }
                                    }
                                }

                                if (retVal)
                                {
                                    // The process is using the current module:
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

CPUProfileDataTable::TableType ProcessesDataTable::GetTableType() const
{
    return CPUProfileDataTable::PROCESSES_DATA_TABLE;
}

bool ProcessesDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
                      (m_pSessionDisplaySettings != nullptr) &&
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

            std::vector<gtString> selectedCounterList;

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

        retVal = true;
    }

    return retVal;
}