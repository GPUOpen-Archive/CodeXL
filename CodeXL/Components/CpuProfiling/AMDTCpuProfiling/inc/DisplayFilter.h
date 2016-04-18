//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisplayFilter.h
///
//==================================================================================

#ifndef __DISPLAYFILTER_H
#define __DISPLAYFILTER_H

// Qt:
#include <QVector>
#include <QStringList>
#include <QMultiMap>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>

#include <AMDTCpuProfilingRawData/inc/CpuProfileInfo.h>

// Local:
#include <inc/StdAfx.h>
class CpuProfileInfo;

typedef QVector<unsigned int> CoreFilter;
enum SeparateByType
{
    SEPARATE_BY_NONE = 0,
    SEPARATE_BY_CORE = 1,
    SEPARATE_BY_NUMA = 2
};

/// Forward declaration:
class EventsFile;
typedef gtMap <int, int> TotalIndicesMap;

/// Enumeration describing the change done to the session display settings:
enum CPUTableUpdateType
{
    UPDATE_TABLE_NONE = 0x0000,
    UPDATE_TABLE_COLUMN_DISPLAY = 0x0001,
    UPDATE_TABLE_COLUMNS_DATA = 0x0002,
    UPDATE_TABLE_REBUILD = 0x0004
};

/// -----------------------------------------------------------------------------------------------
/// \class Name: CPUGlobalDisplayFilter
/// \brief Description:  This class is supposed to represent a global display filter for all profile
///  tables
/// -----------------------------------------------------------------------------------------------
class CPUGlobalDisplayFilter
{
public:
    static CPUGlobalDisplayFilter& instance();
    static void reset();
    ~CPUGlobalDisplayFilter();

    bool m_displaySystemDLLs;
    bool m_displayPercentageInColumn;

private:

    CPUGlobalDisplayFilter();                                     ///< Constructor
    CPUGlobalDisplayFilter(const CPUGlobalDisplayFilter& other);        ///< Copy constructor

private:
    static CPUGlobalDisplayFilter* m_psMySingleInstance;
};

typedef gtMap<QString, ViewElementType> ConfigurationMap;
class SessionDisplaySettings
{

public:


    SessionDisplaySettings();
    virtual ~SessionDisplaySettings();

    // Assignment operator:
    SessionDisplaySettings& operator=(const SessionDisplaySettings& other);

    // Copy from other settings:
    void CopyFrom(const SessionDisplaySettings& other);

    /// Compare another settings to me and return the difference type:
    /// \param other the settings to compare to
    /// \return the difference type
    unsigned int CompareSettings(const SessionDisplaySettings& other);

    void initialize(CpuProfileInfo* pProfileInfo);
    ColumnSpec getColumnSpecFromEventName(QString& eventName);
    QStringList getListOfViews(int numberOfEvents, int* pDefault = nullptr);

    /// Find the event file for the current configuration:
    EventsFile* getEventsFile();

    /// Calculate the displayed columns according to the separate, view name and other parameters:
    bool calculateDisplayedColumns(CoreTopologyMap* pTopology);

    static float setComplex(const ComplexComponents& complexComponent, gtVector<float>& dataVector);

    /// Contain the events file relevant for the current view;
    EventsFile* m_pCurrentEventsFile;

    ConfigurationMap m_configurationsMap;
    EventNormValueMap m_normMap;
    EventConfig* m_pEventArray;
    CpuProfileInfo* m_pProfileInfo;

    /// Cpu information:
    int m_cpuCount;
    int m_cpuFamily;
    int m_cpuModel;
    CoreFilter m_cpuFilter;

    // The column captions for the all the available data:
    // The captions in this vector are in short form ("ret inst" for example):
    gtVector<QString> m_availableDataColumnCaptions;

    // The captions for the all the available data:
    // The captions in this vector are in short form ("Retired Instructions" for example):
    gtVector<QString> m_availableDataFullNames;

    // The tooltips for the all the available data columns:
    gtVector<QString> m_availableDataColumnTooltips;

    // True / false for each available data column if it should be displayed:
    gtVector<bool> m_areAvailableDataColumnDisplayed;

    // A vector of indices (from m_availableDataColumnCaptions) which should be displayed:
    gtVector<int> m_displayedDataColumnsIndices;

    // A vector of indices (from m_availableDataColumnCaptions) which should be displayed:
    QStringList m_filteredDataColumnsCaptions;

    // The display filter name (a display filter represents a list of displayed columns):
    QString m_displayFilterName;

    // The map of columns that are calculated from other columns (used to be calculated):
    ComplexDependentMap m_calculatedDataColumns;

    /// Map shown-value columns to the index of the event total (used to be totals):
    TotalIndicesMap m_totalValuesMap;

    /// Contains which columns are simple values (used to be values):
    gtVector<int> m_simpleValuesVector;

    /// Event to index map (used to be indexes):
    CpuEventViewIndexMap m_eventToIndexMap;

    /// Member variables used while calculating the columns:
    QVector<unsigned int> m_listOfDuplicatedEvents;
    int m_nextIndex;
    int m_totalIndex;
    TempEventIndexMap m_groupMap;
    TempEventIndexMap m_complexMap;
    CoreTopologyMap* m_pCurrentTopology;

    /// Should the data columns be separated by (core / numa node):
    SeparateByType m_separateBy;

    // Is the CLU data being filtered out or not?
    bool m_displayClu;
private:

    /// Read the available views from XML files:
    void readAvailableViews();
    void addAllDataView();
    void addConfiguration(const gtString& configFileName);
    bool handleSingleEvent(ColumnSpec* columnArray, int cpuIndex, int eventIndex);

};


/// -----------------------------------------------------------------------------------------------
/// \class Name: TableDisplaySettings
/// \brief Description:  This class is supposed to represent a display filter for a CPU profile table
/// -----------------------------------------------------------------------------------------------
class TableDisplaySettings
{
public:

    TableDisplaySettings();
    TableDisplaySettings(const TableDisplaySettings& other);

    enum ProfileDataColumnType
    {
        UNKNOWN_COL,
        MODULE_NAME_COL,
        PROCESS_NAME_COL,
        PID_COL,
        TID_COL,
        FUNCTION_NAME_COL,
        SAMPLES_COUNT_COL,
        SAMPLES_PERCENT_COL,
        MODULE_SYMBOLS_LOADED,
        SOURCE_FILE_COL,
        SELF_SAMPLE_PERCENTAGE_OF_ALL_SAMPLE_COL,
        SELF_SAMPLE_PERCENTAGE_OF_MODULE_SAMPLE_COL
    };

    bool colTypeAsString(ProfileDataColumnType column, QString& colStr, QString& tooltip);

    void initHotspotIndicatorMap(SessionDisplaySettings* pSessionDisplaySettings);

    // The maximum amount of items shown in table (-1 if we should display all):
    int m_amountOfItemsInDisplay;

    // The Information columns to display:
    gtVector<ProfileDataColumnType> m_displayedColumns;

    // The caption for the data column that is supposed to be the hot spot column:
    QString m_hotSpotIndicatorColumnCaption;

    /// Only show modules / functions that are related to this PID:
    QList<ProcessIdType> m_filterByPIDsList;

    /// The name of the module to filter by:
    QStringList m_filterByModulePathsList;

    /// A list of all modules for the current session:
    QStringList m_allModulesFullPathsList;

    /// List of booleans. Element i contain true iff the m_allModulesFullPathsList[i] is 32 bit dll:
    QList<bool> m_isModule32BitList;

    /// List of booleans. Element i contain true iff the m_allModulesFullPathsList[i] is 32 bit dll:
    QList<bool> m_isSystemDllList;

    /// Contain true iff the system modules should be displayed in modules dialog:
    bool m_shouldDisplaySystemDllInModulesDlg;

    /// Contain the list of hot spot indicator full names:
    gtMap<QString, int> m_hotSpotIndicatorToDataIndexMap;

    /// Contain the last column that the table was sorted by:
    Qt::SortOrder m_lastSortOrder;

    QString m_lastSortColumnCaption;
};

#endif //__DISPLAYFILTER_H

