//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FunctionsDataTable.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

#include <AMDTCpuPerfEventUtils/inc/IbsEvents.h>

/// Local:
#include <inc/Auxil.h>
#include <inc/CPUProfileUtils.h>
#include <inc/DebugUtils.h>
#include <inc/FunctionsDataTable.h>
#include <inc/StringConstants.h>
#include <inc/SessionWindow.h>
#include <SessionTreeNodeData.h>

#define FunctionDataIndexRole  ((int)(Qt::UserRole) + 0)

FunctionsDataTable::FunctionsDataTable(QWidget* pParent,
                                       const gtVector<TableContextMenuActionType>& additionalContextMenuActions,
                                       SessionTreeNodeData* pSessionData,
                                       CpuSessionWindow* pSessionWindow) :
    CPUProfileDataTable(pParent, additionalContextMenuActions, pSessionData),
    m_popupToBrowseMissingFiles(false),
    m_pParentSessionWindow(pSessionWindow)
{
    m_functionNameColIndex = -1;
    m_moduleNameColIndex = -1;

    m_tableRowHasIcon = true;
}

FunctionsDataTable::~FunctionsDataTable()
{
}

bool FunctionsDataTable::fillListData()
{
#if 0
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(/*(m_pProfileReader != nullptr) && */
        (m_pSessionDisplaySettings != nullptr) &&
        (m_pTableDisplaySettings != nullptr) &&
        (m_pProfDataRdr != nullptr))
    {
        for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::MODULE_NAME_COL)
            {
                m_moduleNameColIndex = i;
            }

            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::FUNCTION_NAME_COL)
            {
                m_functionNameColIndex = i;
            }
        }

        AMDTProfileDataVec funcProfileData;
        // all modules are selected
        int rc = m_pTableDisplaySettings->m_filterByModulePathsList.size();
        rc = m_pTableDisplaySettings->m_filterByPIDsList.size();

        // check if one process selected
        if ((m_pTableDisplaySettings->m_filterByPIDsList.size() > 1) &&
            (m_pTableDisplaySettings->m_filterByModulePathsList.empty()))
        {
            m_pProfDataRdr->GetFunctionProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, funcProfileData);
        }

        // not all process selected
        if (m_pTableDisplaySettings->m_filterByPIDsList.size() > 1)
        {

        }

        if (CpuProfileModule::UNMANAGEDPE == pModule->m_modType ||
            CpuProfileModule::JAVAMODULE == pModule->m_modType ||
            CpuProfileModule::MANAGEDPE == pModule->m_modType)
        {
            bool rc = addFunctionsForModule(pModule, modulePath, parentSamples);
            retVal &= rc;
        }

        afProgressBarWrapper::instance().incrementProgressBar();

        //bool rc = m_pProfDataRdr->GetFunctionProfileData(AMDTProcessId procId, AMDTModuleId modId, funcProfileData);
        //GT_ASSERT(rc);

#if 0
        NameModuleMap* pNameModuleMap = m_pProfileReader->getModuleMap();

        if (nullptr == pNameModuleMap)
        {
            return false;
        }

        retVal = true;

        // Update the progress bar + dialog:
        afProgressBarWrapper::instance().ShowProgressDialog(CP_functionsTableProgress, static_cast<int>(pNameModuleMap->size()));

        m_functionsInfosVec.clear();
        m_totalDataValuesVector.clear();
        m_threads.clear();
        m_totalDataValuesVector.resize(m_pSessionDisplaySettings->m_availableDataColumnCaptions.size() + 1);

        AggregatedSample parentSamples;

        QWidget* pParent = m_popupToBrowseMissingFiles ? parentWidget() : nullptr;

        NameModuleMap::const_iterator mit = pNameModuleMap->begin(), mEnd = pNameModuleMap->end();

        for (; mit != mEnd; ++mit)
        {
            // Get the current module:
            const CpuProfileModule* pModule = &(mit->second);

            if (pModule->isIndirect())
            {
                continue;
            }

            if (!shouldModuleBeDisplayed(*pModule))
            {
                continue;
            }

            // Get the current module name:
            QString modulePath = acGTStringToQString(pModule->getPath());

            if (!pModule->m_isImdRead)
            {
                pModule = m_pParentSessionWindow->getModuleDetail(modulePath, pParent);

                if (nullptr == pModule)
                {
                    continue;
                }
            }

            if (CpuProfileModule::UNMANAGEDPE == pModule->m_modType ||
                CpuProfileModule::JAVAMODULE  == pModule->m_modType ||
                CpuProfileModule::MANAGEDPE   == pModule->m_modType)
            {
                bool rc = addFunctionsForModule(pModule, modulePath, parentSamples);
                retVal &= rc;
            }

            afProgressBarWrapper::instance().incrementProgressBar();
        }

#endif
        afProgressBarWrapper::instance().hideProgressBar();
    }

    retVal &= CPUProfileDataTable::fillListData();
#endif
    return true;
}

bool FunctionsDataTable::addFunctionsForModule(const CpuProfileModule* pModule, 
	const QString& moduleFilePath, 
	AggregatedSample& parentSamples)
{
    bool retVal = false;

    // Check how many rows added so far:
    int currentRowsAmount = rowCount();

    // Sanity check:
    GT_IF_WITH_ASSERT((pModule != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        retVal = true;

        const int dataColsSize = m_pSessionDisplaySettings->m_availableDataColumnCaptions.size() + 1;
        gtVAddr lastParentAddr = GT_INVALID_VADDR;
        FunctionData currentFunctionData;
        currentFunctionData.m_pModule = pModule;

        // For each function
        AddrFunctionMultMap::const_iterator fit = pModule->getBeginFunction();
        AddrFunctionMultMap::const_iterator fEnd = pModule->getEndFunction();

        for (; fit != fEnd; ++fit)
        {
            const CpuProfileFunction& function = fit->second;

            // For each sample
            AptAggregatedSampleMap::const_iterator sit = function.getBeginSample();
            AptAggregatedSampleMap::const_iterator sEnd = function.getEndSample();

            for (; sit != sEnd; ++sit)
            {
                const AptKey& aptKey = sit->first;
                gtVAddr sampAddr = aptKey.m_addr;

                // Add the thread to the list:
                if (m_threads.indexOf(aptKey.m_tid) < 0)
                {
                    m_threads << aptKey.m_tid;
                }

                bool shouldDisplay = true;

                if (!m_pTableDisplaySettings->m_filterByPIDsList.isEmpty())
                {
                    int index = m_pTableDisplaySettings->m_filterByPIDsList.indexOf(aptKey.m_pid);
                    shouldDisplay = (index >= 0);
                }

                if (shouldDisplay)
                {
                    // Keep the list unique.
                    if (-1 == currentFunctionData.m_pidList.indexOf(aptKey.m_pid))
                    {
                        currentFunctionData.m_pidList.push_back(aptKey.m_pid);
                    }

                    QString functionName = function.getFuncName().isEmpty() ? CA_NO_SYMBOL :
                                           acGTStringToQString(function.getFuncName());

                    switch (pModule->m_modType)
                    {
                        // Normal PE module:
                        case CpuProfileModule::UNMANAGEDPE:
                            if (pModule->isUnchartedFunction(function))
                            {
                                sampAddr += pModule->getBaseAddr();

                                functionName.clear();
                                AuxGetParentFunctionName(pModule, &function, sampAddr, functionName);
                            }
                            else
                            {
                                sampAddr = function.getBaseAddr();
                            }

                            break;

                        case CpuProfileModule::JAVAMODULE:
                        case CpuProfileModule::MANAGEDPE:
                            // For now, Putting this specific check for Java/CLR
                            // At some point, need to re-structure this function to properly handle all module type
                            // For Java/CLR, func name/addr info is already present and just need to set in outer loop (for each func)
                            // Probably the better is to define separate functions for each module type like in old GUI code
                            sampAddr = function.getBaseAddr();
                            break;

                        case CpuProfileModule::UNKNOWNMODULE:
                        case CpuProfileModule::UNKNOWNKERNELSAMPLES:
                            // TODO: Handle Unknown Kernel Samples and Unknown Module here

                            // Convert the "No symbol" to a wide char:
                            break;

                        default:
                            break;
                    }

                    // This is new function:
                    if (lastParentAddr != sampAddr && !currentFunctionData.m_functionName.isEmpty())
                    {
                        currentFunctionData.m_dataVector.clear();
                        currentFunctionData.m_dataVector.resize(dataColsSize);
                        CPUProfileUtils::ConvertAggregatedSampleToArray(parentSamples,
                                                                        currentFunctionData.m_dataVector,
                                                                        m_totalDataValuesVector,
                                                                        m_pSessionDisplaySettings,
                                                                        false);


                        if (m_pSessionDisplaySettings->m_displayClu)
                        {
                            CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, currentFunctionData.m_dataVector);
                        }

                        m_functionsInfosVec.push_back(currentFunctionData);

                        parentSamples.clear();
                    }

                    // Reinitialize for next function
                    lastParentAddr = sampAddr;

                    currentFunctionData.m_functionAddress = sampAddr;
                    currentFunctionData.m_functionName = functionName;

                    // Aggregate samples:
                    parentSamples.addSamples(&(sit->second));
                }
            }
        }

        // Add the last function calculated:
        if (!currentFunctionData.m_functionName.isEmpty() && parentSamples.getSampleMapSize() > 0)
        {
            currentFunctionData.m_dataVector.clear();
            currentFunctionData.m_dataVector.resize(dataColsSize);

            CPUProfileUtils::ConvertAggregatedSampleToArray(parentSamples,
                                                            currentFunctionData.m_dataVector,
                                                            m_totalDataValuesVector,
                                                            m_pSessionDisplaySettings,
                                                            false);


            if (m_pSessionDisplaySettings->m_displayClu)
            {
                CPUProfileUtils::CalculateCluMetrics(m_pSessionDisplaySettings, currentFunctionData.m_dataVector);
            }

            m_functionsInfosVec.push_back(currentFunctionData);

            parentSamples.clear();
        }

        // Reserve function rows:
        int rowsToAdd = m_functionsInfosVec.size() - currentRowsAmount;

        if (rowsToAdd > 0)
        {
            retVal = addEmptyRows(rowsToAdd);
            GT_IF_WITH_ASSERT(retVal)
            {
                // Add the functions to the table after collecting it's data values:
                for (int currentRowToAdd = currentRowsAmount, maxRow = static_cast<int>(m_functionsInfosVec.size());
                     currentRowToAdd < maxRow; currentRowToAdd++)
                {
                    addFunctionItem(currentRowToAdd, m_functionsInfosVec[currentRowToAdd], moduleFilePath, pModule->m_is32Bit);
                }
            }
        }
    }

    return retVal;
}

bool FunctionsDataTable::addFunctionItem(int rowIndex, const FunctionData& functionData, const QString& moduleFilePath, bool is32BitModule)
{
    bool retVal = true;

    QStringList functionItemStringList;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        for (int i = 0; i < (int)m_pTableDisplaySettings->m_displayedColumns.size(); i++)
        {
            switch (m_pTableDisplaySettings->m_displayedColumns[i])
            {

                case TableDisplaySettings::MODULE_NAME_COL:
                {
                    // Calculate the samples count for this module:
                    QFileInfo fileInfo(moduleFilePath);
                    functionItemStringList << fileInfo.fileName();
                    break;
                }

                case TableDisplaySettings::PROCESS_NAME_COL:
                {
                    functionItemStringList << "TODO";
                    break;
                }

                case TableDisplaySettings::FUNCTION_NAME_COL:
                {
                    functionItemStringList << functionData.m_functionName;
                    break;
                }

                default:
                {
                    // Field will be re-calculated in post process:
                    functionItemStringList << "";
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
                functionItemStringList << "";

                if (showPercentSeperateColumns)
                {
                    functionItemStringList << "";
                }
            }
        }

        // Get the icon for this file:
        osFilePath modulePath(acQStringToGTString(moduleFilePath));
        QPixmap* pIcon = CPUProfileDataTable::moduleIcon(modulePath, is32BitModule);

        // Initialize the function row:
        initRowItems(rowIndex, functionItemStringList, Qt::AlignVCenter | Qt::AlignLeft);
        QTableWidgetItem* pNameItem = item(rowIndex, m_functionNameColIndex);

        if (pNameItem != nullptr)
        {
            // Set the original position in function vector:
            pNameItem->setData(FunctionDataIndexRole, QVariant(rowIndex));

            if (pIcon != nullptr)
            {
                pNameItem->setIcon(QIcon(*pIcon));
            }
        }

        QTableWidgetItem* pModuleNameItem = item(rowIndex, m_moduleNameColIndex);

        if (pModuleNameItem != nullptr)
        {
            pModuleNameItem->setToolTip(moduleFilePath);
        }

        // Set the values for this row:
        setTableDisplayedColumnsValues(rowIndex, functionData.m_dataVector);
    }

    return retVal;
}


bool FunctionsDataTable::shouldModuleBeDisplayed(const CpuProfileModule& module)
{

    bool retVal = true;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTableDisplaySettings != nullptr)
    {
        if (!m_pTableDisplaySettings->m_filterByModulePathsList.isEmpty())
        {
            retVal = m_pTableDisplaySettings->m_filterByModulePathsList.contains(acGTStringToQString(module.getPath()));
        }

        // Filter out system modules:
        if (retVal && !CPUGlobalDisplayFilter::instance().m_displaySystemDLLs)
        {
            // Check if this is a system module:
            if (module.m_isImdRead)
            {
                retVal = !module.isSystemModule();
            }
            else
            {
                retVal = !AuxIsSystemModule(module.getPath());
            }
        }
    }
    return retVal;
}

const FunctionsDataTable::FunctionData* FunctionsDataTable::getFunctionData(int rowIndex) const
{
    const FunctionData* pFuncData = nullptr;

    GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
    {
        QTableWidgetItem* pNameTableItem = item(rowIndex, 0);

        GT_IF_WITH_ASSERT(nullptr != pNameTableItem)
        {
            QVariant dataVar = pNameTableItem->data(FunctionDataIndexRole);

            if (dataVar.isValid())
            {
                int indexWithinFunctionsVector = dataVar.toInt();
                GT_IF_WITH_ASSERT(0 <= indexWithinFunctionsVector &&
                                  static_cast<int>(m_functionsInfosVec.size()) > indexWithinFunctionsVector)
                {
                    pFuncData = &m_functionsInfosVec[indexWithinFunctionsVector];
                }
            }
        }
    }

    return pFuncData;
}

const QList<ProcessIdType>* FunctionsDataTable::getFunctionPidList(int rowIndex) const
{
    const QList<ProcessIdType>* pPidList = nullptr;

    const FunctionData* pFuncData = getFunctionData(rowIndex);

    GT_IF_WITH_ASSERT(nullptr != pFuncData)
    {
        pPidList = &pFuncData->m_pidList;
    }

    return pPidList;
}

gtVAddr FunctionsDataTable::getFunctionAddress(int rowIndex, const CpuProfileModule*& pModule) const
{
    gtVAddr funcAddr = 0;

    const FunctionData* pFuncData = getFunctionData(rowIndex);

    GT_IF_WITH_ASSERT(nullptr != pFuncData)
    {
        funcAddr = pFuncData->m_functionAddress;
        pModule = pFuncData->m_pModule;
    }

    return funcAddr;
}

QString FunctionsDataTable::getModuleName(int rowIndex) const
{
	QString name;

	GT_IF_WITH_ASSERT((rowIndex >= 0) && (rowIndex < rowCount()))
	{
		QTableWidgetItem* pNameTableItem = item(rowIndex, 4);

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
		QTableWidgetItem* pNameTableItem = item(rowIndex, 1);

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
		QTableWidgetItem* pNameTableItem = item(rowIndex, 0);

		GT_IF_WITH_ASSERT(nullptr != pNameTableItem)
		{
			name = pNameTableItem->text();
		}
	}

	return name;
}

const CpuProfileModule* FunctionsDataTable::findModuleHandler(const osFilePath& filePath) const
{
    const CpuProfileModule* pRetVal = nullptr;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    GT_IF_WITH_ASSERT(nullptr != m_pProfileReader)
    {
        // In Windows the path is case insensitive, meaning that a simple "find" operation within the names map won't work.
        const NameModuleMap* pNameModuleMap = m_pProfileReader->getModuleMap();

        if (nullptr != pNameModuleMap)
        {
            // Go over all the mapped modules and look for the one with the requested file path:
            for (NameModuleMap::const_iterator mit = pNameModuleMap->begin(), mEnd = pNameModuleMap->end(); mit != mEnd; ++mit)
            {
                osFilePath currentModulePath(mit->first);

                if (currentModulePath == filePath)
                {
                    pRetVal = &mit->second;
                    break;
                }
            }
        }
    }

#else // Other operating systems are case sensitive!

    GT_IF_WITH_ASSERT(nullptr != m_pParentSessionWindow)
    {
        pRetVal = m_pParentSessionWindow->getModuleDetail(acGTStringToQString(filePath.asString()));
    }

#endif

    return pRetVal;
}

bool FunctionsDataTable::HandleHotSpotIndicatorSet()
{
    // call directly to setHotSpotIndicatorValues, without calling to displayProfileData first
    // boolean to function has no meaning
    return setHotSpotIndicatorValues();
}

bool FunctionsDataTable::buildHotSpotIndicatorMap()
{
    BEGIN_TICK_COUNT(BuildHotSpot);
    bool retVal = true;

    if (m_pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU &&
        !m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption.isEmpty())
    {
        initializeListHeaders();
    }

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionDisplaySettings != nullptr) && (m_pTableDisplaySettings != nullptr))
    {
        // Clear the map:
        m_hotSpotCellsMap.clear();

        // Find the index of the hot spot indicator within the data vector:
        int hotSpotColIndexInDataVector = -1;
        QString hotSpotCaption = m_pTableDisplaySettings->m_hotSpotIndicatorColumnCaption;

        if (m_pTableDisplaySettings->m_hotSpotIndicatorToDataIndexMap.find(hotSpotCaption) != m_pTableDisplaySettings->m_hotSpotIndicatorToDataIndexMap.end())
        {
            hotSpotColIndexInDataVector = m_pTableDisplaySettings->m_hotSpotIndicatorToDataIndexMap[hotSpotCaption];
        }

        int precision = afGlobalVariablesManager::instance().floatingPointPrecision();

        // Find the index of the sample count and sample count percent indices:
        int sampleCountIndex = -1;
        QList<int> samplesCountPercentIndexList;

        for (int i = 0, sz = (int)m_pTableDisplaySettings->m_displayedColumns.size(); i < sz; i++)
        {
            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_PERCENT_COL)
            {
                samplesCountPercentIndexList << i;
            }

            if (m_pTableDisplaySettings->m_displayedColumns[i] == TableDisplaySettings::SAMPLES_COUNT_COL)
            {
                sampleCountIndex = i;
            }
        }

        // Look for the CLU percent column:
        bool isHotSpotCluPercent = false;
        findCLUPercentColumn(samplesCountPercentIndexList, hotSpotCaption, isHotSpotCluPercent);

        const int rowsAmount = rowCount();
        GT_IF_WITH_ASSERT(sampleCountIndex >= 0)
        {
            // Show all rows (otherwise it's problematic to set it's data):
            for (int i = 0; i < rowsAmount; i++)
            {
                setRowHidden(i, false);
            }

            // Block the table model signals, otherwise the table is sorted while setting the data (which causes a mess):
            blockSignals(true);
            setSortingEnabled(false);

            m_totalSampleCount = 0;

            int functionsCount = (int)m_functionsInfosVec.size();

            // Go over all the lines and summarize the total values of the samples:
            for (int rowIndex = 0; rowIndex < functionsCount; rowIndex++)
            {
                QTableWidgetItem* pTableItem = item(rowIndex, sampleCountIndex);
                GT_IF_WITH_ASSERT(pTableItem != nullptr)
                {
                    const FunctionData& currentData = m_functionsInfosVec[rowIndex];
                    GT_IF_WITH_ASSERT((hotSpotColIndexInDataVector >= 0) && (hotSpotColIndexInDataVector < (int)currentData.m_dataVector.size()))
                    {
                        float floatVal = currentData.m_dataVector[hotSpotColIndexInDataVector];
                        m_totalSampleCount += floatVal;
                    }
                }
            }

            if (0 < rowsAmount && 0 < m_pTableDisplaySettings->m_amountOfItemsInDisplay)
            {
                m_hotSpotCellsMap.reserve(rowsAmount);
            }

            // Update the progress bar + dialog:
            afProgressBarWrapper::instance().ShowProgressDialog(CP_functionsTableProgress, rowsAmount);

            // Calculate progress bar update frequency
            int step = (rowsAmount > 100) ? (rowsAmount / 100) : 1;

            // Go over all the lines for the hot spot index and find the values:
            for (int rowIndex = 0; rowIndex < rowsAmount; rowIndex++)
            {
                QTableWidgetItem* pSampleCountTableItem = item(rowIndex, sampleCountIndex);
                QTableWidgetItem* pNameTableItem = item(rowIndex, m_functionNameColIndex);
                GT_IF_WITH_ASSERT(nullptr != pSampleCountTableItem && nullptr != pNameTableItem)
                {
                    // Find the index within the functions vector:
                    QVariant dataVar = pNameTableItem->data(FunctionDataIndexRole);
                    int indexWithinFunctionsVector = -1;

                    if (dataVar.isValid())
                    {
                        indexWithinFunctionsVector = dataVar.toInt();
                    }

                    if ((indexWithinFunctionsVector >= 0) && (indexWithinFunctionsVector < (int)m_functionsInfosVec.size()))
                    {
                        const FunctionData& currentData = m_functionsInfosVec[indexWithinFunctionsVector];
                        GT_IF_WITH_ASSERT((hotSpotColIndexInDataVector >= 0) && (hotSpotColIndexInDataVector < (int)currentData.m_dataVector.size()))
                        {
                            // Fix the value for the percent item:
                            float percentF = 0.0;
                            float floatVal = currentData.m_dataVector[hotSpotColIndexInDataVector];

                            // Set the value with the correct precision:
                            QString strPrecision = QString::number(floatVal, 'f', precision);
                            floatVal = strPrecision.toFloat();

                            if (m_totalSampleCount > 0)
                            {
                                percentF = floatVal / (float)m_totalSampleCount * 100;

                                if (isHotSpotCluPercent)
                                {
                                    percentF = floatVal;
                                }
                            }

                            // Set the data for the current cell (according to the new hot spot indicator):
                            QVariant dataVariant;
                            dataVariant.setValue(floatVal);
                            pSampleCountTableItem->setData(Qt::DisplayRole, dataVariant);

                            QString tooltip = dataVariant.toString();
                            pSampleCountTableItem->setToolTip(tooltip);

                            if (samplesCountPercentIndexList.size() > 0 || isHotSpotCluPercent)
                            {
                                foreach (int index, samplesCountPercentIndexList)
                                {
                                    QTableWidgetItem* pSamplePercentTableItem = item(rowIndex, index);
                                    GT_IF_WITH_ASSERT(nullptr != pSamplePercentTableItem)
                                    {
                                        strPrecision = QString::number(percentF, 'f', precision);
                                        float valuePrecision = strPrecision.toFloat();
                                        dataVariant.setValue(valuePrecision);
                                        pSamplePercentTableItem->setData(Qt::DisplayRole, dataVariant);

                                        QString tooltip1 = dataVariant.toString();
                                        pSamplePercentTableItem->setToolTip(tooltip1);
                                    }
                                }
                            }

                            // Update the hot spot map:
                            HotSpotValue val;
                            val.m_index = rowIndex;
                            val.m_percentValue = percentF;
                            m_hotSpotCellsMap.push_back(val);
                        }
                    }

                    // Update the progress bar:
                    BEGIN_TICK_COUNT(IncrementProgressBar);

                    if ((rowIndex % step) == 0 || rowIndex == rowsAmount)
                    {
                        afProgressBarWrapper::instance().incrementProgressBar(step);
                    }

                    END_TICK_COUNT(IncrementProgressBar);
                }
                retVal = true;

                afProgressBarWrapper::instance().hideProgressBar();
            }

            // Unblock the signals:
            blockSignals(false);
            setSortingEnabled(true);
        }
    }

    END_TICK_COUNT(BuildHotSpot);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: buildHotSpotIndicatorMap (%u ms)", m_elapsedTime[BuildHotSpot]);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Elapsed Time: incrementProgressBar (%u ms)", m_elapsedTime[IncrementProgressBar]);
#endif

    return retVal;
}

void FunctionsDataTable::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    CPUProfileDataTable::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT((m_pContextMenu != nullptr) && (m_pTableDisplaySettings != nullptr) && (m_pParentSessionWindow != nullptr))
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
                        QTableWidgetItem* pItem = selectedItems().first();

                        // if its empty or is the "empty row" string item
                        if (nullptr == pItem ||
                            pItem->row() == -1)
                        {
                            isActionEnabled = false;
                        }

                        //if more then one row selected
                        else if (columnCount() != 0 &&
                                 (selectedItems().count() / columnCount()) > 1)
                        {
                            isActionEnabled = false;
                        }

                        else if (DISPLAY_FUNCTION_IN_CALLGRAPH_VIEW == actionType)
                        {
#if 0
                            // Only enable display in call graph view, if the process css data is collected:
                            isActionEnabled = false;

                            int rowIndex = pItem->row();

                            // get list of process Ids  for this function item
                            const QList<ProcessIdType>* pSelectedPidList;
                            pSelectedPidList = getFunctionPidList(rowIndex);

                            if (nullptr != pSelectedPidList)
                            {
                                // get pIds of call graph table
                                const QMap<ProcessIdType, QString>& cssProcesses = m_pParentSessionWindow->CollectedProcessesMap();

                                // foreach pid in the selected item list
                                for (ProcessIdType pid : *pSelectedPidList)
                                {
                                    // check if in call graph list
                                    if (cssProcesses.contains(pid))
                                    {
                                        // if yes - enable the "display in call graph" selection
                                        isActionEnabled = true;
                                        break;
                                    }
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

CPUProfileDataTable::TableType FunctionsDataTable::GetTableType() const
{
    return CPUProfileDataTable::FUNCTION_DATA_TABLE;
}

bool FunctionsDataTable::fillSummaryTables(int counterIdx)
{
    bool retVal = false;

    if (nullptr != m_pProfDataRdr)
    {
        AMDTProfileCounterDescVec counterDesc;
        bool rc = m_pProfDataRdr->GetSampledCountersList(counterDesc);

        AMDTProfileDataVec funcProfileData;
        rc = m_pProfDataRdr->GetFunctionSummary(counterDesc.at(counterIdx).m_id,
                                                funcProfileData);
#if 1 //test code aalok
		for (auto const& func : funcProfileData)
		{
			AMDTProfileFunctionData  functionData;
			m_pProfDataRdr->GetFunctionDetailedProfileData(func.m_id,
				AMDT_PROFILE_ALL_PROCESSES,
				AMDT_PROFILE_ALL_THREADS,
				functionData);

			gtString srcFilePath;
			AMDTSourceAndDisasmInfoVec srcInfoVec;
			m_pProfDataRdr->GetFunctionSourceAndDisasmInfo(func.m_id, srcFilePath, srcInfoVec);
		}
#endif
        GT_ASSERT(rc);

        setSortingEnabled(false);

        for (auto profData : funcProfileData)
        {
            // create QstringList to hold the values
			QStringList list;

			// insert the function id 
			QVariant mId(profData.m_id);
			list << mId.toString();

            list << profData.m_name.asASCIICharArray();

            QVariant sampleCount(profData.m_sampleValue.at(0).m_sampleCount);
            list << sampleCount.toString();

            QVariant sampleCountPercent(profData.m_sampleValue.at(0).m_sampleCountPercentage);
            list << QString::number(profData.m_sampleValue.at(0).m_sampleCountPercentage, 'f', 2);

            AMDTProfileModuleInfoVec procInfo;
            rc = m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_MAX_VALUE, profData.m_moduleId, procInfo);
            GT_ASSERT(rc);
            list << procInfo.at(0).m_name.asASCIICharArray();

            addRow(list, nullptr);

			QString modulefullPath(procInfo.at(0).m_path.asASCIICharArray());
			int row = rowCount()-1;
			int col = 4;

			QTableWidgetItem* pModuleNameItem = item(row, col);
			if (pModuleNameItem != nullptr)
			{
				pModuleNameItem->setToolTip(modulefullPath);
			}


#if 0
			QString modulePath(procInfo.at(0).m_path.asASCIICharArray());

            rc = setToolTip(row, sampleCountPercent.toString(), modulePath);

			if (true == rc)
            {
                rc = setModuleIcon(row, procInfo.at(0));
            }

#endif

            if (true == rc)
            {
                rc = delegateSamplePercent(3);
            }
        }

        setSortingEnabled(true);

        retVal = true;
    }

    return retVal;
}

bool FunctionsDataTable::AddRowToTable(const gtVector<AMDTProfileData>& allModuleData)
{
	if (!allModuleData.empty())
	{
		setSortingEnabled(false);

		for (auto profData : allModuleData)
		{
			QStringList list;

			std::vector<gtString> selectedCounterList;

			// insert the function id 
			QVariant mId(profData.m_id);
			list << mId.toString();

			// Insert function name
			list << profData.m_name.asASCIICharArray();

			// insert module name
			AMDTProfileModuleInfoVec procInfo;
			m_pProfDataRdr->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, profData.m_moduleId, procInfo);

			list << acGTStringToQString(procInfo.at(0).m_name);

			m_pDisplayFilter->GetSelectedCounterList(selectedCounterList);
			int i = 0;

			for (auto counter : selectedCounterList)
			{
				QVariant sampleCount(profData.m_sampleValue.at(i++).m_sampleCount);
				list << sampleCount.toString();
			}

			addRow(list, nullptr);
		}
	}

	return true;
}
bool FunctionsDataTable::fillTableData(AMDTProcessId procId, AMDTModuleId modId, std::vector<AMDTUInt64> modIdVec)
{
    bool retVal = false;

	GT_IF_WITH_ASSERT((m_pProfDataRdr != nullptr) &&
		(m_pSessionDisplaySettings != nullptr) &&
		(m_pTableDisplaySettings != nullptr))
	{
		gtVector<AMDTProfileData> allModuleData;

		if (modIdVec.empty())
		{
			AMDTProfileSessionInfo sessionInfo;

			bool rc = m_pProfDataRdr->GetProfileSessionInfo(sessionInfo);
			GT_ASSERT(rc);

			rc = m_pProfDataRdr->GetFunctionProfileData(procId, modId, allModuleData);
			GT_ASSERT(rc);

		}
		else
		{
			gtVector<AMDTProfileData> moduleData;
			for (const auto& moduleId : modIdVec)
			{
				moduleData.clear();
				bool rc = m_pProfDataRdr->GetFunctionProfileData(procId, moduleId, moduleData);
				GT_ASSERT(rc);
				allModuleData.insert(allModuleData.end(), moduleData.begin(), moduleData.end());
			}
			retVal = true;
		}

		AddRowToTable(allModuleData);

	}

    return retVal;
}