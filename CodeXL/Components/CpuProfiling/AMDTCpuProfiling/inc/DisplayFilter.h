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

// STANDARD INCLUDES
#include <memory>
#include <map>
#include <vector>
#include <utility>
#include <tuple>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// PROJECT INCLUDES
#include <AMDTCpuProfilingRawData/inc/CpuProfileInfo.h>
#include <AMDTCpuProfilingDataAccess/inc/AMDTCpuProfilingDataAccess.h>

// Local:
#include <inc/StdAfx.h>


enum SeparateByType
{
    SEPARATE_BY_NONE = 0,
    SEPARATE_BY_CORE = 1,
    SEPARATE_BY_NUMA = 2
};

/// Enumeration describing the change done to the session display settings:
enum CPUTableUpdateType
{
    UPDATE_TABLE_NONE = 0x0000,
    UPDATE_TABLE_COLUMN_DISPLAY = 0x0001,
    UPDATE_TABLE_COLUMNS_DATA = 0x0002,
    UPDATE_TABLE_REBUILD = 0x0004
};

// FORWARD DECLARATION
class cxlProfileDataReader;

// TYPEDEFS
typedef QVector<unsigned int> CoreFilter;
using CounterNameIdVec = std::vector<std::tuple<
      gtString,                                                         // counter name
      gtString,                                                         // counter abbreviation
      gtString,                                                         // counter desc
      AMDTUInt32,                                                       // counter-id
      AMDTUInt32>>;                                                     // counter-type
using configNameCounterMap = std::map<gtString, CounterNameIdVec>;
using configNameCounterPair = std::pair<gtString, CounterNameIdVec>;
using CounterNameIdMap = std::map<gtString, AMDTUInt64>;
using CounterIdNameMap = std::map<AMDTUInt64, gtString>;

const gtString CLU_PERCENTAGE_CAPTION = L"Cache Line Utilization Percentage";

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

    bool m_displaySystemModules = false;
    bool m_displayPercentageInColumn = false;

private:
    CPUGlobalDisplayFilter() = default;
    CPUGlobalDisplayFilter(const CPUGlobalDisplayFilter& other) = delete;

    static CPUGlobalDisplayFilter* m_psMySingleInstance;
};

enum AMDTProfileType
{
    AMDT_PROFILE_TYPE_CLU,
    AMDT_PROFILE_TYPE_TBP,
    AMDT_PROFILE_TYPE_OTHERS,
};

/// -----------------------------------------------------------------------------------------------
/// \class Name: TableDisplaySettings
/// \brief Description:  This class is supposed to represent a display filter for a CPU profile table
/// -----------------------------------------------------------------------------------------------
class TableDisplaySettings
{
public:
    TableDisplaySettings() = default;
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
        SELF_SAMPLE_PERCENTAGE_OF_MODULE_SAMPLE_COL,
        MODULE_ID,
        FUNCTION_ID
    };

    bool colTypeAsString(ProfileDataColumnType column, QString& colStr, QString& tooltip);

    // The maximum amount of items shown in table (-1 if we should display all):
    int m_amountOfItemsInDisplay = -1;

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

    /// List of booleans. Element i contain true iff the m_allModulesFullPathsList[i] is 32 bit module:
    QList<bool> m_isModule32BitList;

    /// List of booleans. Element i contain true iff the m_allModulesFullPathsList[i] is 32 bit module:
    QList<bool> m_isSystemModuleList;

    /// Contain true iff the system modules should be displayed in modules dialog:
    bool m_shouldDisplaySystemModuleInModulesDlg = true;

    /// Contain the list of hot spot indicator full names:
    gtMap<QString, int> m_hotSpotIndicatorToDataIndexMap;

    /// Contain the last column that the table was sorted by:
    Qt::SortOrder m_lastSortOrder = Qt::DescendingOrder;

    QString m_lastSortColumnCaption;
};

class DisplayFilter
{
public:
    DisplayFilter();
    bool InitToDefault();

    // Profile reader
    void SetProfDataReader(std::shared_ptr<cxlProfileDataReader> reader) { m_pProfDataReader = reader; }

    // Config
    bool CreateConfigCounterMap();
    bool GetConfigCounters(const QString& configName, CounterNameIdVec& counterDetails);
    void GetConfigName(std::vector<gtString>& configNameList) const { configNameList = m_configNameList; }
    const QString& GetCurrentConfigName() const { return m_configurationName; }

    // Options
    bool SetProfileDataOptions(AMDTProfileDataOptions opts);
    const AMDTProfileDataOptions& GetProfileDataOptions() const;

    // Reporter config
    bool SetReportConfig();
    const gtVector<AMDTProfileReportConfig>& GetReportConfig() const;

    // NUMA settings
    void SetSeperatedbyNuma(bool isSet) { m_options.m_isSeperateByNuma = isSet; }
    bool IsSeperatedByNumaEnabled() const { return m_options.m_isSeperateByNuma; }

    // Core settings
    void SetSeperatedbyCore(bool isSet) { m_options.m_isSeperateByCore = isSet; }
    bool IsSeperatedByCoreEnabled() const { return m_options.m_isSeperateByCore; }

    // Counter Description
    void SetCounterDescription(const gtVector<AMDTUInt32>& counterDesp) { m_options.m_counters = counterDesp; }
    const gtVector<AMDTUInt32> GetCounterDescription() const { return m_options.m_counters; }

    // Counter id
    AMDTUInt64 GetCounterId(const QString& counterName) const;
    gtString GetCounterName(AMDTUInt64 counterId) const;

    // Counter list
    void SetSelectedCounterList(const CounterNameIdVec& list) { m_selectedCountersIdList.clear();  m_selectedCountersIdList = list; }
    void GetSupportedCountersList(CounterNameIdVec& counterList) const;
    void GetSelectedCounterList(CounterNameIdVec& list) const { list = m_selectedCountersIdList; }

    // Core Mask
    void SetCoreMask(AMDTUInt64 mask) { m_options.m_coreMask = mask; }
    AMDTUInt64 GetCoreMask() const { return m_options.m_coreMask; }

    // Core Count
    AMDTUInt32 GetCoreCount() const;

    // System Module
    void setIgnoreSystemModule(bool isChecked);
    bool IsSystemModuleIgnored() const;

    // Sample Percent
    void SetDisplaySamplePercent(bool isSet);
    bool isDisplaySamplePercent() const;

    // CLU
    void SetCLUOVHdrName(const QString& name);
    const QString& GetCLUOVHdrName() const { return m_CLUOVHdrName; }
    bool isCLUPercentCaptionSet() const { return m_isCLUPercent; }

    // View
    void SetViewName(const QString& viewName) { m_viewName = viewName; }
    QString GetViewName() const { return m_viewName; }

    // ProfileType
    void SetProfileType(AMDTProfileType type) { m_profType = type; }
    AMDTProfileType GetProfileType() const { return m_profType; }

private:
    void SetProfileDataOption();

    std::shared_ptr<cxlProfileDataReader>   m_pProfDataReader;
    gtVector<AMDTProfileReportConfig>       m_reportConfigs;
    AMDTProfileDataOptions                  m_options;
    configNameCounterMap                    m_configCounterMap;
    QString                                 m_configurationName;
    std::vector<gtString>                   m_configNameList;
    CounterNameIdMap                        m_counterNameIdMap;
    CounterIdNameMap                        m_counterIdNameMap;
    CounterNameIdVec                        m_selectedCountersIdList;
    QString                                 m_CLUOVHdrName;
    bool                                    m_isCLUPercent = true;
    QString                                 m_viewName;
    gtUInt32                                m_coreCount = 0;
    bool                                    m_isTbp = false;
    AMDTProfileType                         m_profType = AMDT_PROFILE_TYPE_OTHERS;
};

#endif //__DISPLAYFILTER_H
