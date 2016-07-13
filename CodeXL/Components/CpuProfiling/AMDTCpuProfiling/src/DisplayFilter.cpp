//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisplayFilter.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Backend:
#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

/// Local:
#include <inc/CPUProfileUtils.h>
#include <inc/DisplayFilter.h>
#include <inc/StdAfx.h>
#include <inc/StringConstants.h>

// Static member initialization:
CPUGlobalDisplayFilter* CPUGlobalDisplayFilter::m_psMySingleInstance = nullptr;

TableDisplaySettings::TableDisplaySettings()
{
    m_amountOfItemsInDisplay = -1;
    m_hotSpotIndicatorColumnCaption = "";
    m_filterByPIDsList.clear();
    m_filterByModulePathsList.clear();
    m_allModulesFullPathsList.clear();
    m_shouldDisplaySystemDllInModulesDlg = true;
    m_isModule32BitList.clear();
    m_isSystemDllList.clear();
    m_lastSortOrder = Qt::DescendingOrder;
    m_lastSortColumnCaption = "";
}

SessionDisplaySettings::SessionDisplaySettings() :
    m_pCurrentEventsFile(nullptr),
    m_pEventArray(nullptr),
    m_pProfileInfo(nullptr),
    m_displayClu(false)
{
    m_displayFilterName = CP_strCPUProfileDisplayFilterAllData;
    m_separateBy = SEPARATE_BY_NONE;

}

TableDisplaySettings::TableDisplaySettings(const TableDisplaySettings& other)
{
    // Copy the displayed columns:
    for (int i = 0; i < (int)other.m_displayedColumns.size(); i++)
    {
        m_displayedColumns.push_back(other.m_displayedColumns[i]);
    }

    m_amountOfItemsInDisplay = other.m_amountOfItemsInDisplay;
    m_hotSpotIndicatorColumnCaption = other.m_hotSpotIndicatorColumnCaption;
    m_filterByPIDsList = other.m_filterByPIDsList;
    m_filterByModulePathsList = other.m_filterByModulePathsList;
    m_allModulesFullPathsList = other.m_allModulesFullPathsList;
    m_isModule32BitList = other.m_isModule32BitList;
    m_isSystemDllList = other.m_isSystemDllList;
    m_shouldDisplaySystemDllInModulesDlg = other.m_shouldDisplaySystemDllInModulesDlg;
}

bool SessionDisplaySettings::calculateDisplayedColumns(CoreTopologyMap* pTopology)
{
    bool retVal = false;

    // Check if this display filter exist:
    ConfigurationMap::const_iterator iterFind = m_configurationsMap.find(m_displayFilterName);
    GT_IF_WITH_ASSERT(iterFind != m_configurationsMap.end())
    {
        m_availableDataColumnTooltips.clear();
        m_availableDataColumnCaptions.clear();
        m_availableDataFullNames.clear();
        m_displayedDataColumnsIndices.clear();

        //m_eventToIndexMap.clear();
        m_calculatedDataColumns.clear();
        m_totalValuesMap.clear();
        m_simpleValuesVector.clear();

        getEventsFile();

        // Get the number of columns available from the requested config:
        int availableCount = m_configurationsMap[m_displayFilterName].pView->GetNumberOfColumns();

        // Allocate a new column spec array:
        ColumnSpec* columnArray = new ColumnSpec[availableCount];


        // Get the column spec for this view name:
        m_configurationsMap[m_displayFilterName].pView->GetColumnSpecs(columnArray);

        DupEventMap unitMap;
        m_listOfDuplicatedEvents.clear();

        for (int i = 0; i < availableCount; i++)
        {
            unsigned int evSelect = columnArray[i].dataSelectLeft.eventSelect;
            unsigned int evUnitMask = columnArray[i].dataSelectLeft.eventUnitMask
                                      + ((columnArray[i].dataSelectLeft.bitOs ? 1 : 0) << 9)
                                      + ((columnArray[i].dataSelectLeft.bitUsr ? 1 : 0) << 8);

            // If we haven't found this event yet, save the unit mask
            if (!unitMap.contains(evSelect))
            {
                unitMap[evSelect] = evUnitMask;
            }
            // If we have found the event, and the unit mask is different
            else if ((evUnitMask != unitMap[evSelect]) && (!m_listOfDuplicatedEvents.contains(evSelect)))
                //save the event in the list of duplicates which need unit masks
            {
                m_listOfDuplicatedEvents.append(evSelect);
            }
        }

        unitMap.clear();

        // Reset members used for calculations:
        m_nextIndex = 0;
        m_totalIndex = 0;
        m_complexMap.clear();
        m_groupMap.clear();
        m_pCurrentTopology = pTopology;

        int cpuCount = 0, eventsCount = 0;
        cpuCount = m_cpuCount;
        eventsCount = availableCount;

        retVal = true;

        for (int eventIndex = 0; eventIndex < eventsCount; eventIndex++)
        {
            for (int cpuIndex = 0; cpuIndex < cpuCount; cpuIndex++)
            {
                bool rc = handleSingleEvent(columnArray, cpuIndex, eventIndex);
                GT_ASSERT(rc);

                retVal = retVal && rc;
            }
        }

        delete[] columnArray;
        m_groupMap.clear();
        m_complexMap.clear();

        if (m_pProfileInfo->m_isProfilingCLU)
        {
            // KARTHIK :
            // The m_isProfilingCLU is a misnomer in the display world ( since we can filter out data
            // we might have profiled, but might not be intrested in displaying it).
            // So use the new variable to control the CLU data visibility from here on.

            CpuEventViewIndexMap::const_iterator itr = nullptr;
            //CpuEventViewIndexMap::const_iterator itr = m_eventToIndexMap.begin();
            m_displayClu = false;

            while (itr != nullptr)
                //while (itr != m_eventToIndexMap.end())
            {
                SampleKeyType skey = itr.key();
                EventMaskTypeEnc ev;
                ev.encodedEvent = skey.event;
                gtUInt16 event = ev.ucEventSelect;

                if (IsIbsCluEvent(event))
                {
                    m_displayClu = true;
                    break;
                }

                ++itr;
            }
        }
    }

    return retVal;

}

bool SessionDisplaySettings::handleSingleEvent(ColumnSpec* columnArray, int cpuIndex, int eventIndex)
{
    bool retVal = false;

    // Sanity check:
    SampleKeyType evKey;
    evKey.cpu = 0;
    int columnIndex;
    SampleKeyType columnKey;
    QString title;
    QString fullName;
    QString dupString;
    CpuEvent event;

    evKey.cpu = cpuIndex;

    if (m_cpuFilter.contains(evKey.cpu))
    {
        retVal = true;
    }
    else
    {
        // Set columnEvent to the left event select value PJD
        unsigned int columnEvent = columnArray[eventIndex].dataSelectLeft.eventSelect;
        columnKey.event = eventIndex;
        evKey.event = EncodeEvent(columnEvent,
                                  columnArray[eventIndex].dataSelectLeft.eventUnitMask,
                                  columnArray[eventIndex].dataSelectLeft.bitOs,
                                  columnArray[eventIndex].dataSelectLeft.bitUsr);


        if ((m_pCurrentEventsFile == nullptr) ||
            ((m_pCurrentEventsFile != nullptr) && (!m_pCurrentEventsFile->FindEventByValue(columnEvent, event))))
        {
            event.name = IsTimerEvent(columnEvent) ? "Timer" : "Unknown";
        }

        if (m_listOfDuplicatedEvents.contains(columnEvent))
        {
            dupString = " (0x" +
                        QString::number((columnArray[eventIndex].dataSelectLeft.eventUnitMask),
                                        16) + ")" + (columnArray[eventIndex].dataSelectLeft.bitOs ? ",Os" : "")
                        + (columnArray[eventIndex].dataSelectLeft.bitUsr ? ",Usr" : "");
        }

        //if sep by core
        QString postfixShort, postfixFull;

        if (m_separateBy == SEPARATE_BY_CORE)
        {
            columnKey.cpu = evKey.cpu;
            postfixShort = dupString + " - C" + QString::number(evKey.cpu);
            postfixFull = dupString + " (Core " + QString::number(evKey.cpu) + ")";
        }
        else if (m_separateBy == SEPARATE_BY_NUMA)
        {
            if (nullptr != m_pCurrentTopology)
            {
                columnKey.cpu = m_pCurrentTopology->at(evKey.cpu).numaNode;
            }
            else
            {
                columnKey.cpu = evKey.cpu;
            }

            postfixShort = dupString + " - N" + QString::number(columnKey.cpu);
            postfixFull = dupString + " (NUMA " + QString::number(columnKey.cpu) + ")";
        }
        else    //SEPARATE_BY_NONE
        {
            columnKey.cpu = 0;
            postfixShort = dupString;
            postfixFull = dupString;
        }

        title = columnArray[eventIndex].title + postfixShort;

        // Get the full name of the event / metric:
        fullName = CPUProfileUtils::GetColumnFullName(columnArray[eventIndex].type, columnArray[eventIndex].title, event, postfixFull);
        GT_ASSERT(!fullName.isEmpty());

        if (!m_groupMap.contains(columnKey))
        {
            columnIndex = m_nextIndex++;
            m_groupMap.insert(columnKey, columnIndex);

            m_displayedDataColumnsIndices.push_back(columnIndex);
            m_totalValuesMap[columnIndex] = m_totalIndex;

            // Push this title to the captions vector:
            m_availableDataColumnCaptions.push_back(title);
            m_availableDataFullNames.push_back(fullName);

            QString columnTitleFull = columnArray[eventIndex].title + dupString;

            if (m_filteredDataColumnsCaptions.contains(columnTitleFull) &&
                (columnTitleFull.compare(title) != 0))
            {
                m_filteredDataColumnsCaptions.push_back(title);
            }

            if (ColumnValue == columnArray[eventIndex].type)
            {
                m_availableDataColumnTooltips.push_back(event.description);
                m_simpleValuesVector.push_back(columnIndex);
                m_totalIndex++;
            }
            else
            {
                m_availableDataColumnTooltips.push_back(title);
            }

        }
        else
        {
            columnIndex = m_groupMap[columnKey];
        }

        // Store the index offset for the given event:
        if (ColumnValue == columnArray[eventIndex].type)
        {
            // Associate the column index with the event select value
            m_complexMap[evKey] = columnIndex;
            //m_eventToIndexMap[evKey] = columnIndex;
        }
        else
        {
            ComplexComponents complex;
            complex.columnIndex = columnIndex;
            complex.complexType = columnArray[eventIndex].type;
            complex.op1Index = m_complexMap[evKey];

            // Set otherEvent to the right event select value
            SampleKeyType otherKey;
            otherKey.cpu = evKey.cpu;
            otherKey.event = EncodeEvent(columnArray[eventIndex].dataSelectRight.eventSelect,
                                         columnArray[eventIndex].dataSelectRight.eventUnitMask,
                                         columnArray[eventIndex].dataSelectRight.bitOs,
                                         columnArray[eventIndex].dataSelectRight.bitUsr);
            complex.op2Index = m_complexMap[otherKey];

            if (m_normMap.contains(evKey.event))
            {
                complex.op1NormValue = m_normMap[evKey.event];
            }
            else
            {
                complex.op1NormValue = 1.0;
            }

            if (m_normMap.contains(otherKey.event))
            {
                complex.op2NormValue = m_normMap[otherKey.event];
            }
            else
            {
                complex.op2NormValue = 1.0;
            }

            m_calculatedDataColumns[complex.op1Index].append(complex);
            m_calculatedDataColumns[complex.op2Index].append(complex);
        }

        retVal = true;
    }

    return retVal;
}

bool TableDisplaySettings::colTypeAsString(ProfileDataColumnType column, QString& colStr, QString& tooltip)
{
    bool retVal = true;

    switch (column)
    {
        case MODULE_NAME_COL:
            colStr = CP_colCaptionModuleName;
            tooltip = CP_colCaptionModuleNameTooltip;
            break;


        case PROCESS_NAME_COL:
            colStr = CP_colCaptionProcessName;
            tooltip = CP_colCaptionProcessNameTooltip;
            break;

        case FUNCTION_NAME_COL:
            colStr = CP_colCaptionFunctionName;
            tooltip = CP_colCaptionFunctionNameTooltip;
            break;

        case PID_COL:
            colStr = CP_colCaptionPID;
            tooltip = CP_colCaptionPIDTooltip;
            break;

        case TID_COL:
            colStr = CP_colCaptionTID;
            tooltip = CP_colCaptionTIDTooltip;
            break;

        case SAMPLES_COUNT_COL:
            colStr = CP_colCaptionSamples;
            tooltip = QString(CP_colCaptionSamplesTooltip).arg(m_hotSpotIndicatorColumnCaption);
            break;

        case SAMPLES_PERCENT_COL:
            colStr = CP_colCaptionSamplesPercent;
            tooltip = QString(CP_colCaptionSamplesPercentTooltip).arg(m_hotSpotIndicatorColumnCaption);

            break;

        case MODULE_SYMBOLS_LOADED:
            colStr = CP_colCaptionModuleSymbolsLoaded;
            tooltip = CP_colCaptionModuleSymbolsLoadedTooltip;
            break;

        case SOURCE_FILE_COL:
            colStr = CP_colCaptionSourceFile;
            tooltip = CP_colCaptionSourceFileTooltip;
            break;

        case MODULE_ID:
            colStr = CP_colCaptionModuleId;
            tooltip = CP_colCaptionModuleIdTooltip;
            break;

        case FUNCTION_ID:
            colStr = CP_colCaptionFuncId;
            tooltip = CP_colCaptionFuncIdTooltip;
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported column type");
            retVal = false;
            break;
    }

    return retVal;
}

void TableDisplaySettings::initHotspotIndicatorMap(SessionDisplaySettings* pSessionDisplaySettings)
{
    // This is an hot spot indicator table:
    GT_IF_WITH_ASSERT((m_amountOfItemsInDisplay > 0) && (pSessionDisplaySettings != nullptr))
    {

        pSessionDisplaySettings->m_displayFilterName = CP_strCPUProfileDisplayFilterAllData;

        // Now build the map for the hot spot indicator indices:
        for (int i = 0; i < (int)pSessionDisplaySettings->m_availableDataFullNames.size(); i++)
        {
            QString currentCaption = pSessionDisplaySettings->m_availableDataFullNames[i];
            m_hotSpotIndicatorToDataIndexMap[currentCaption] = i;
        }
    }
}


//This static function will calculate a number, based on the predefined
//  complex relationship.  Currently, only +, -, *, and \ are allowed between
//  two basic values
// The complex component defines the relationship and holds the indexes into the
//  data given.
//Note that to make the values more meaningful, the raw data has been
//  normalized to a per-group/count value
float SessionDisplaySettings::setComplex(const ComplexComponents& complexComponent, gtVector<float>& dataVector)
{
    float calc = 0;

    GT_IF_WITH_ASSERT((complexComponent.op1Index >= 0) && (complexComponent.op1Index < (int)dataVector.size()) &&
                      (complexComponent.op2Index >= 0) && (complexComponent.op2Index < (int)dataVector.size()))
    {
        float normOp1 = dataVector[complexComponent.op1Index] * complexComponent.op1NormValue;
        float normOp2 = dataVector[complexComponent.op2Index] * complexComponent.op2NormValue;

        switch (complexComponent.complexType)
        {
            case ColumnRatio:

                if (0 != normOp2)
                {
                    calc = normOp1 / normOp2;
                }

                break;

            case ColumnSum:
                calc = normOp1 + normOp2;
                break;

            case ColumnDifference:
                calc = normOp1 - normOp2;
                break;

            case ColumnProduct:
                calc = normOp1 * normOp2;
                break;

            default:
                break;
        }
    }

    return calc;
}

EventsFile* SessionDisplaySettings::getEventsFile()
{
    if (m_pCurrentEventsFile == nullptr)
    {
        // Find and open the events file:
        osFilePath eventFilePath(osFilePath::OS_CODEXL_DATA_PATH);
        eventFilePath.appendSubDirectory(L"Events");

        EventEngine eventEngine;

        if (!eventEngine.Initialize(QString::fromWCharArray(eventFilePath.asString().asCharArray())))
        {
            QString errorMessage = "Unable to find the event files directory: " + QString::fromWCharArray(eventFilePath.asString().asCharArray());
            acMessageBox::instance().information(AF_STR_ErrorA, errorMessage);
        }
        else
        {
            m_pCurrentEventsFile = eventEngine.GetEventFile(m_cpuFamily, m_cpuModel);
        }
    }

    return m_pCurrentEventsFile;
}

QStringList SessionDisplaySettings::getListOfViews(int numberOfEvents, int* pDefault)
{
    //pAvailable is nullptr if we want a list of all views
    QStringList configList;
    ConfigurationMap::const_iterator it = m_configurationsMap.begin();

    for (; it != m_configurationsMap.end(); ++it)
    {
        //If the profile fits the available view
        if ((nullptr == m_pEventArray) || (it->second.pView->isViewDisplayable(m_pEventArray, numberOfEvents)))
        {
            configList += it->first;

            if ((nullptr != pDefault) && (it->second.pView->GetDefaultView()))
            {
                *pDefault = configList.size() - 1;
            }
        }
    }

    //If no default was set, and there are views available, use the 1st one.
    if ((nullptr != pDefault) && (-1 == *pDefault) && (configList.size() > 0))
    {
        *pDefault = 0;
    }

    return configList;
}

ColumnSpec SessionDisplaySettings::getColumnSpecFromEventName(QString& eventName)
{
    ColumnSpec spec;
    spec.type = ColumnInvalid;
    ConfigurationMap::const_iterator it = m_configurationsMap.find(CP_strCPUProfileDisplayFilterAllData);
    GT_IF_WITH_ASSERT(m_configurationsMap.end() != it)
    {
        const int availableCount = it->second.pView->GetNumberOfColumns();
        // Allocate a new column spec array:
        ColumnSpec* columnArray = new ColumnSpec[availableCount];

        // Get the column spec for this view name:
        it->second.pView->GetColumnSpecs(columnArray);

        for (int i = 0; i < availableCount; ++i)
        {
            if (columnArray[i].title == eventName)
            {
                spec = columnArray[i];
                break;
            }
        }

        //For IBS fetch/Op, do not require os, user, unit mask bits
        if ((IsIbsFetchEvent(spec.dataSelectLeft.eventSelect) && (GetIbsFetchEvent() == spec.dataSelectLeft.eventSelect)) ||
            (IsIbsOpEvent(spec.dataSelectLeft.eventSelect) && (GetIbsOpEvent() == spec.dataSelectLeft.eventSelect)) ||
            (IsTimerEvent(spec.dataSelectLeft.eventSelect)))
        {
            spec.dataSelectLeft.bitOs = 0;
            spec.dataSelectLeft.bitUsr = 0;
            spec.dataSelectLeft.eventUnitMask = 0;
        }

        delete[]columnArray;
    }
    return spec;
}

void SessionDisplaySettings::readAvailableViews()
{
    osFilePath basePath(osFilePath::OS_CODEXL_DATA_PATH);
    basePath.appendSubDirectory(L"Views");
    osDirectory baseDir(basePath);

    gtList<osFilePath> filePaths;
    gtList<osFilePath>::iterator it;
    gtList<osFilePath>::iterator endIt;

    // Remove all but all data
    ConfigurationMap::iterator remIt = m_configurationsMap.begin();

    while (remIt != m_configurationsMap.end())
    {
        QString name = remIt->first;
        remIt++;

        if (name != CP_strCPUProfileDisplayFilterAllData)
        {
            m_configurationsMap.erase(name);
        }
    }

    // Read TBP
    if (baseDir.getContainedFilePaths(L"*.XML", osDirectory::SORT_BY_NAME_DESCENDING, filePaths))
    {
        it = filePaths.begin();

        for (; it != filePaths.end(); it++)
        {
            addConfiguration((*it).asString());
        }
    }

    gtString subName;
    int cpuFamily = FAMILY_UNKNOWN;
    int cpuModel = 0;

    if (nullptr != m_pProfileInfo)
    {
        cpuFamily = m_pProfileInfo->m_cpuFamily;
        cpuModel = m_pProfileInfo->m_cpuModel;
    }

    //If the model mask is needed
    if (cpuFamily >= FAMILY_OR)
    {
        //since the model is like 0x10-1f, just need the mask (like 0x10), so shift right by 4 bits
        subName.appendFormattedString(L"0x%x_0x%x", cpuFamily, (cpuModel >> 4));
    }
    else
    {
        subName.appendFormattedString(L"0x%x", cpuFamily);
    }

    basePath.appendSubDirectory(subName);
    baseDir.setDirectoryPath(basePath);

    if (baseDir.exists())
    {
        if (baseDir.getContainedFilePaths(L"*.xml", osDirectory::SORT_BY_NAME_DESCENDING, filePaths))
        {
            it = filePaths.begin();

            for (; it != filePaths.end(); it++)
            {
                addConfiguration((*it).asString());
            }
        }
    }
}


void SessionDisplaySettings::addConfiguration(const gtString& configFileName)
{
    ViewConfig* pTemp = new ViewConfig();
    RETURN_IF_NULL(pTemp);

    // If it's not a view configuration...
    if (!pTemp->ReadConfigFile(QString::fromWCharArray(configFileName.asCharArray())))
    {
        delete pTemp;
        return;
    }

    QString profileName;
    pTemp->GetConfigName(profileName);

    if (m_configurationsMap.find(profileName) != m_configurationsMap.end())
    {
        delete pTemp;
        return;
    }

    m_configurationsMap[profileName].pView = pTemp;
    m_configurationsMap[profileName].path = QString::fromWCharArray(configFileName.asCharArray());
    m_configurationsMap[profileName].modifiable = true;
}

// addAllDataView() creates the special view that shows all
// available event data. The array of EventConfig pAvailable
// is a list of all available event data.
void SessionDisplaySettings::addAllDataView()
{
    GT_IF_WITH_ASSERT((m_pProfileInfo != nullptr) || (m_pEventArray != nullptr))
    {

        int cpuFamily = m_pProfileInfo->m_cpuFamily;
        int cpuModel = m_pProfileInfo->m_cpuModel;

        // Find and open the events file.
        osFilePath eventFilePath(osFilePath::OS_CODEXL_DATA_PATH);
        eventFilePath.appendSubDirectory(L"Events");

        EventEngine eventEngine;

        if (!eventEngine.Initialize(QString::fromWCharArray(eventFilePath.asString().asCharArray())))
        {
            acMessageBox::instance().information("Error", "Unable to find the event files directory: " + QString::fromWCharArray(eventFilePath.asString().asCharArray()));
            return;
        }

        EventsFile* pEventFile = eventEngine.GetEventFile(cpuFamily, cpuModel);

        // Use the event definitions to validate event select values in the
        // list of available events and to associate readable (string) names
        // with the select values.
        ViewConfig* pTemp = new ViewConfig();
        RETURN_IF_NULL(pTemp);

        pTemp->SetConfigName(CP_strCPUProfileDisplayFilterAllData);
        QStringList eventList;
        int numberOfEvents = m_pProfileInfo->m_eventVec.size();

        // Walk the list of available events and add each event to the all data view
        for (int i = 0; i < numberOfEvents; i++)
        {
            CpuEvent event;

            // Check the event select value for validity
            if ((nullptr != pEventFile) && (pEventFile->FindEventByValue(m_pEventArray[i].eventSelect, event)))
            {
                // Add an event name to the all data view if the event select
                // value is valid (i.e., found in the event definition file)
                if (!event.abbrev.isEmpty())
                {
                    eventList += event.abbrev;
                }
                else
                {
                    eventList += event.name;
                }
            }
            else
            {
                // If the event select value was not found, check for the
                // special event value that identifies timer data PJD
                if (IsTimerEvent(m_pEventArray[i].eventSelect))
                {
                    eventList += "Timer";
                }
                else
                {
                    eventList += "Unknown";
                }
            }
        }

        if (nullptr != pEventFile)
        {
            delete pEventFile;
        }

        pEventFile = nullptr;

        // Make column specifications using the list of available events
        // and the names of those events
        pTemp->MakeColumnSpecs(numberOfEvents, m_pEventArray, eventList);
        pTemp->SetDescription("This special view has all of the data from the profile available.");
        m_configurationsMap[CP_strCPUProfileDisplayFilterAllData].pView = pTemp;
        m_configurationsMap[CP_strCPUProfileDisplayFilterAllData].modifiable = false;
    }
}


void SessionDisplaySettings::initialize(CpuProfileInfo* pProfileInfo)
{
    m_pProfileInfo = pProfileInfo;
    GT_IF_WITH_ASSERT(m_pProfileInfo != nullptr)
    {
        m_cpuCount = m_pProfileInfo->m_numCpus;
        m_cpuFamily = m_pProfileInfo->m_cpuFamily;
        m_cpuModel = m_pProfileInfo->m_cpuModel;
        m_displayClu = m_pProfileInfo->m_isProfilingCLU;

        // Clear the configuration map:
        m_configurationsMap.clear();

        readAvailableViews();

        // Set the m_normMap. There are no duplicated events.
        EventEncodeVec* pEventVec = (EventEncodeVec*) & (m_pProfileInfo->m_eventVec);
        EventEncodeVec::iterator it = pEventVec->begin();
        EventEncodeVec::iterator itEnd = pEventVec->end();

        for (; it != itEnd; it++)
        {
            EventMaskType tempEvent = it->eventMask;
            m_normMap[tempEvent] = it->eventCount;
        }

        // Create an array to hold event config information.
        int numberOfEvents = m_pProfileInfo->m_eventVec.size();
        m_pEventArray = new EventConfig[numberOfEvents];


        for (unsigned int i = 0; i < pProfileInfo->m_eventVec.size(); i++)
        {
            gtUInt16 event;
            gtUByte unitMask;
            bool bitOs;
            bool bitUsr;

            DecodeEvent(m_pProfileInfo->m_eventVec.at(i).eventMask, &event, &unitMask, &bitOs, &bitUsr);
            // Store event select value PJD
            m_pEventArray[i].eventSelect = event;
            m_pEventArray[i].eventUnitMask = unitMask;
            m_pEventArray[i].bitOs = bitOs;
            m_pEventArray[i].bitUsr = bitUsr;
        }

        addAllDataView();

    }
}

SessionDisplaySettings::~SessionDisplaySettings()
{
    m_configurationsMap.clear();

    if (m_pCurrentEventsFile != nullptr)
    {
        m_pCurrentEventsFile->Close();
        delete m_pCurrentEventsFile;
        m_pCurrentEventsFile = nullptr;
    }
}

SessionDisplaySettings& SessionDisplaySettings::operator=(const SessionDisplaySettings& other)
{
    CopyFrom(other);

    return *this;

}

unsigned int SessionDisplaySettings::CompareSettings(const SessionDisplaySettings& other)
{
    unsigned int retVal = UPDATE_TABLE_NONE;

    foreach (unsigned int cpu, other.m_cpuFilter)
    {
        if (!m_cpuFilter.contains(cpu))
        {
            retVal |= UPDATE_TABLE_COLUMNS_DATA;
            break;
        }
    }

    foreach (unsigned int cpu, m_cpuFilter)
    {
        if (!other.m_cpuFilter.contains(cpu))
        {
            retVal = retVal | UPDATE_TABLE_COLUMNS_DATA;
            break;
        }
    }

    if ((m_separateBy != other.m_separateBy) || (m_filteredDataColumnsCaptions != other.m_filteredDataColumnsCaptions)
        || (m_displayedDataColumnsIndices != other.m_displayedDataColumnsIndices) || (other.m_displayFilterName != m_displayFilterName))
    {
        retVal = retVal | UPDATE_TABLE_COLUMN_DISPLAY;
    }

    return retVal;
}

void SessionDisplaySettings::CopyFrom(const SessionDisplaySettings& other)
{
    m_pCurrentEventsFile = other.m_pCurrentEventsFile;

    // Copy configuration map:
    m_configurationsMap.clear();
    ConfigurationMap::const_iterator iterConfig = other.m_configurationsMap.begin();

    for (; iterConfig != other.m_configurationsMap.end(); iterConfig++)
    {
        if (iterConfig->second.pView != nullptr)
        {
            m_configurationsMap[iterConfig->first].pView = new ViewConfig(*iterConfig->second.pView);
        }

        m_configurationsMap[iterConfig->first].path = iterConfig->second.path;
        m_configurationsMap[iterConfig->first].modifiable = iterConfig->second.modifiable;

    }

    m_normMap = other.m_normMap;
    m_pEventArray = other.m_pEventArray;
    m_pProfileInfo = other.m_pProfileInfo;

    m_cpuCount = other.m_cpuCount;
    m_cpuFamily = other.m_cpuFamily;
    m_cpuModel = other.m_cpuModel;
    m_cpuFilter = other.m_cpuFilter;


    m_availableDataColumnCaptions.clear();
    m_availableDataFullNames.clear();
    m_availableDataColumnTooltips.clear();
    m_areAvailableDataColumnDisplayed.clear();
    m_displayedDataColumnsIndices.clear();
    m_simpleValuesVector.clear();

    for (unsigned int i = 0; i < other.m_availableDataColumnCaptions.size(); i++)
    {
        m_availableDataColumnCaptions.push_back(other.m_availableDataColumnCaptions[i]);
    }

    for (unsigned int i = 0; i < other.m_availableDataFullNames.size(); i++)
    {
        m_availableDataFullNames.push_back(other.m_availableDataFullNames[i]);
    }

    for (unsigned int i = 0; i < other.m_availableDataColumnTooltips.size(); i++)
    {
        m_availableDataColumnTooltips.push_back(other.m_availableDataColumnTooltips[i]);
    }

    for (unsigned int i = 0; i < other.m_areAvailableDataColumnDisplayed.size(); i++)
    {
        m_areAvailableDataColumnDisplayed.push_back(other.m_areAvailableDataColumnDisplayed[i]);
    }

    for (unsigned int i = 0; i < other.m_displayedDataColumnsIndices.size(); i++)
    {
        m_displayedDataColumnsIndices.push_back(other.m_displayedDataColumnsIndices[i]);
    }

    m_filteredDataColumnsCaptions = other.m_filteredDataColumnsCaptions;
    m_displayFilterName = other.m_displayFilterName;
    m_calculatedDataColumns = other.m_calculatedDataColumns;

    m_totalValuesMap.clear();
    TotalIndicesMap::const_iterator iterInd = other.m_totalValuesMap.begin();

    for (; iterInd != other.m_totalValuesMap.end(); iterInd++)
    {
        m_totalValuesMap[iterInd->first] = iterInd->second;
    }

    for (unsigned int i = 0; i < other.m_simpleValuesVector.size(); i++)
    {
        m_simpleValuesVector.push_back(other.m_simpleValuesVector[i]);
    }

    //m_eventToIndexMap = other.m_eventToIndexMap;

    m_listOfDuplicatedEvents = other.m_listOfDuplicatedEvents;
    m_nextIndex = other.m_nextIndex;
    m_totalIndex = other.m_totalIndex;
    m_groupMap = other.m_groupMap;
    m_complexMap = other.m_complexMap;
    m_pCurrentTopology = other.m_pCurrentTopology;
    m_separateBy = other.m_separateBy;
    m_displayClu = other.m_displayClu;
}


CPUGlobalDisplayFilter& CPUGlobalDisplayFilter::instance()
{
    // If this class single instance was not already created:
    if (nullptr == m_psMySingleInstance)
    {
        // Create it:
        m_psMySingleInstance = new CPUGlobalDisplayFilter;
    }

    return *m_psMySingleInstance;
}

CPUGlobalDisplayFilter::~CPUGlobalDisplayFilter()
{
}

void CPUGlobalDisplayFilter::reset()
{
    // "if" to avoid infinite loop
    if (nullptr != m_psMySingleInstance)
    {
        delete m_psMySingleInstance;
        m_psMySingleInstance = nullptr;
    }
}

CPUGlobalDisplayFilter::CPUGlobalDisplayFilter()
{
    m_displaySystemDLLs = false;
    m_displayPercentageInColumn = false;
}

static bool g_isDisplaySystemModule = false;
static bool g_isSamplePercent = false;

DisplayFilter::DisplayFilter()
{

}

bool DisplayFilter::CreateConfigCounterMap()
{
    bool ret = false;

    m_reportConfigs.clear();

    if (nullptr != m_pProfDataReader.get())
    {
        ret = m_pProfDataReader->GetReportConfigurations(m_reportConfigs);

        if (true == ret)
        {
            m_configCounterMap.clear();

            for (const auto& config : m_reportConfigs)
            {
                CounterNameIdVec counters;
                counters.clear();

                for (const auto& counter : config.m_counterDescs)
                {
                    auto nameIdDesc = make_tuple(counter.m_name,
                                                 counter.m_abbrev,
                                                 counter.m_description,
                                                 counter.m_id,
                                                 counter.m_type);
                    counters.push_back(nameIdDesc);
                    m_counterNameIdMap.insert(make_pair(counter.m_name, counter.m_id));
                    m_counterIdNameMap.insert(make_pair(counter.m_id, counter.m_name));
                }

                m_configCounterMap.insert(cofigNameCounterPair(config.m_name, counters));
                m_configNameList.push_back(config.m_name);
            }
        }
    }

    return ret;
}

bool DisplayFilter::
GetConfigCounters(const QString& configName, CounterNameIdVec& counterDetails)
{
    bool ret = false;

    // set the member variable
    m_configurationName = configName;

    auto itr = m_configCounterMap.find(acQStringToGTString(configName));

    if (m_configCounterMap.end() != itr)
    {
        counterDetails = itr->second;
        ret = true;
    }

    return ret;
}

bool DisplayFilter::
SetProfileDataOptions(AMDTProfileDataOptions opts)
{
    m_options.m_coreMask = opts.m_coreMask;
    m_options.m_doSort = opts.m_doSort;
    m_options.m_ignoreSystemModules = opts.m_ignoreSystemModules;
    m_options.m_isSeperateByCore = opts.m_isSeperateByCore;
    m_options.m_isSeperateByNuma = opts.m_isSeperateByNuma;
    m_options.m_othersEntryInSummary = opts.m_othersEntryInSummary;
    m_options.m_summaryCount = opts.m_summaryCount;
    m_options.m_counters = opts.m_counters;

    return true;
}

const AMDTProfileDataOptions&
DisplayFilter::GetProfileDataOptions() const
{
    return m_options;
}

bool DisplayFilter::
SetReportConfig()
{
    bool ret = false;

    if (nullptr != m_pProfDataReader.get())
    {
        SetProfileDataOption();
        AMDTUInt64 mask = (1 << GetCoreCount()) - 1;

        if (GetCoreMask() == mask)
        {
            SetCoreMask(AMDT_PROFILE_ALL_CORES);
        }

        m_pProfDataReader->SetReportOption(m_options);
        ret = true;
    }

    return ret;
}

const  gtVector<AMDTProfileReportConfig>&
DisplayFilter::GetReportConfig() const
{
    return m_reportConfigs;
}

bool
DisplayFilter::InitToDefault()
{
    bool retVal = true;
    m_reportConfigs.clear();

    if (nullptr != m_pProfDataReader.get())
    {
        retVal = m_pProfDataReader->GetReportConfigurations(m_reportConfigs);
        m_options.m_coreMask = AMDT_PROFILE_ALL_CORES;
        m_options.m_doSort = true;
        m_options.m_summaryCount = 5;
        m_options.m_isSeperateByCore = false;
        m_options.m_isSeperateByNuma = false;
        m_options.m_ignoreSystemModules = !g_isDisplaySystemModule;

        //setting all counters for config "All Data"
        for (auto const& counter : m_reportConfigs[0].m_counterDescs)
        {
            m_options.m_counters.push_back(counter.m_id);
            m_selectedCountersIdList.push_back(make_tuple(counter.m_name,
                                                          counter.m_abbrev,
                                                          counter.m_description,
                                                          counter.m_id,
                                                          counter.m_type));
        }

        retVal = m_pProfDataReader->SetReportOption(m_options);
    }

    return retVal;
}

void DisplayFilter::GetSupportedCountersList(CounterNameIdVec& counterList)
{
    counterList = m_selectedCountersIdList;
}

int DisplayFilter::GetCpuCoreCnt() const
{
    int coresCnt = 0;

    if (nullptr != m_pProfDataReader.get())
    {
        AMDTCpuTopologyVec topologyVec;
        m_pProfDataReader->GetCpuTopology(topologyVec);
        coresCnt = topologyVec.size();
    }

    return coresCnt;
}

AMDTUInt64 DisplayFilter::GetCounterId(const QString& counterName) const
{
    AMDTUInt64 id = 0;
    auto idItr = m_counterNameIdMap.find(acQStringToGTString(counterName));

    if (m_counterNameIdMap.end() != idItr)
    {
        id = idItr->second;
    }

    return id;
}

gtString DisplayFilter::GetCounterName(AMDTUInt64 counterId) const
{
    gtString counterName;

    auto nameItr = m_counterIdNameMap.find(counterId);

    if (m_counterIdNameMap.end() != nameItr)
    {
        counterName = nameItr->second;
    }

    return counterName;
}

void DisplayFilter::SetProfileDataOption()
{
    m_options.m_counters.clear();

    for (auto const& selCounter : m_selectedCountersIdList)
    {
        auto foundCountId = m_counterNameIdMap.find(get<0>(selCounter));

        if (m_counterNameIdMap.end() != foundCountId)
        {
            m_options.m_counters.push_back(foundCountId->second);
        }
    }
}

bool DisplayFilter::IsSystemModuleIgnored()
{
    return m_options.m_ignoreSystemModules;
}

void DisplayFilter::SetSamplePercent(bool isSet)
{
    g_isSamplePercent = isSet;
}

void DisplayFilter::setIgnoreSysDLL(bool isChecked)
{
    m_options.m_ignoreSystemModules = isChecked;
    g_isDisplaySystemModule = !isChecked;
}

bool DisplayFilter::GetSamplePercent()
{
    return g_isSamplePercent;
}

AMDTUInt32 DisplayFilter::GetCoreCount() const
{
    AMDTUInt32 coreCount = 0;
    AMDTCpuTopologyVec cpuToplogy;
    bool rc = m_pProfDataReader->GetCpuTopology(cpuToplogy);

    if (true == rc)
    {
        coreCount = cpuToplogy.size();
    }

    return coreCount;
}
