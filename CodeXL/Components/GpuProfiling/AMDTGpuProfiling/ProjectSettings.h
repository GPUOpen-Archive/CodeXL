//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProjectSettings.h $
/// \version $Revision: #21 $
/// \brief  This file contains project settings class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProjectSettings.h#21 $
// Last checkin:   $DateTime: 2016/03/28 05:32:41 $
// Last edited by: $Author: rbober $
// Change list:    $Change: 565842 $
//=====================================================================
#ifndef _PROJECT_SETTING_H_
#define _PROJECT_SETTING_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QString>
#include <QList>
#include <QTextStream>

// Local:
#include <AMDTGpuProfiling/gpStringConstants.h>

/// API Trace mode enum
enum APITraceMode
{
    /// Not a APITraceMode
    NA_API_TRACE_MODE = 0,

    /// Time Out mode
    TIMEOUT,

    /// Normal mode
    NORMAL
};

/// OpenCL API Rule type
enum RulesType
{
    /// Not a APITraceMode
    NA_RULES_TYPE = 0,

    /// Rule type - Warning
    WARNING_RULES_TYPE,

    /// Rule type - Error
    ERROR_RULES_TYPE,

    /// Rule type - Best practice
    BEST_PRACTICE_RULES_TYPE
};

/// enum defining which API to trace
enum APIToTrace
{
    APIToTrace_Unknown = 0,                              ///< Uninitialized value
    APIToTrace_OPENCL = 0x1,                             ///< value indicating that OpenCL APIs should be traced
    APIToTrace_HSA = 0x2,                                ///< value indicating that HSA APIs should be traced
    APIToTrace_ALL = APIToTrace_OPENCL | APIToTrace_HSA  ///< value indicating that all APIs should be traced
};

// Forward declarations
class CLAPIRule;

#define DEFAULT_NUM_OF_API_CALLS_TO_TRACE 400000
class CLAPIFilterManager;
/// API Trace related options
class APITraceOptions
{
public:

    /// Initializes a new instance of the APITraceOptions class
    APITraceOptions(const APIToTrace& apiType = APIToTrace_OPENCL);

    /// Destructor
    ~APITraceOptions();

    /// Restore the settings to the default values:
    void RestoreDefault(const APIToTrace& apiType);

    /// Append the list of rules as string to listOfRules:
    void GetListOfRules(QStringList& listOfRules) const;

    /// Enables / disables a rule:
    /// \param ruleAsString string describing the rule
    /// \param isEnabled should the rule be enabled / disabled
    void EnableRule(const QString& ruleAsString, bool isEnabled);

    /// Set all rules:
    /// \param bool isEnabled should all rules be enabled / disabled
    void ResetRules(bool isEnabled);

    /// Checks if the rule is currently enabled:
    /// \param ruleName the rule name
    bool IsRuleEnabled(const QString& ruleName);
private:
    void ClearRules();

public:

    const unsigned int  m_defaultMaxAPIs;          ///< default value for the timeout interval
    APITraceMode        m_mode;                    ///< Contains API trace mode
    unsigned int        m_timeoutInterval;         ///< Contains Timeout Interval for timeout mode
    unsigned int        m_maxAPICalls;             ///< Contains maximum number of APIs to trace
    QMap<CLAPIRule*, bool>   m_rules;              ///< Contains map of rules -> is the rules enabled
    bool                m_generateSummaryPage;     ///< Indicates whether or not to generate summary page when performing an API trace
    bool                m_queryRetStat;            ///< Indicates whether or not to query errcode_ret
    bool                m_generateSymInfo;         ///< Indicates whether or not to generate symbol information
    bool                m_collapseClGetEventInfo;  ///< Indicates whether or not consecutive identical calls to clGetEventInfo should be collapsed
    bool                m_generateKernelOccupancy; ///< Indicates whether or not generate occupancy information
    bool                m_alwaysShowAPIErrorCode;  ///< Indicates whether or not show API error code
    bool                m_filterAPIsToTrace;       ///< Indicates whether there's a filter on the API trace
    bool                m_writeDataTimeOut;        ///< Indicates timeout option is enabled/disabled.
    CLAPIFilterManager* m_pFilterManager;          ///< Gets API Filter manager
    APIToTrace          m_apiToTrace;              ///< Indicates which API to trace

private:

    /// disabled default copy ctor
    APITraceOptions(const APITraceOptions&);

    /// disabled default assignment operator
    APITraceOptions& operator = (const APITraceOptions&);
};

/// CL API Rule
class CLAPIRule
{
public:

    /// Initializes a new instance of the CLAPIRule class
    /// \param name API Rule name
    /// \param displayName Name to display in the UI
    /// \param description Rule description
    /// \param type API Rule type
    CLAPIRule(const QString&  name, const QString&  displayName, const QString&  description, RulesType type);

    ~CLAPIRule() {}

    /// Gets APIRule name
    /// \return API Rule name
    QString GetName() const;

    /// Gets APIRule display name
    /// \return API Rule display name
    QString GetDisplayName() const;

    /// Gets APIRule description
    /// \return API Rule description
    QString GetDescription() const;

    /// Gets APIRule type
    /// \return API Rule type
    RulesType GetType() const;

    /// Returns a human-readable QString representation of the specified RulesType value
    /// \param type the type for which to return a QString
    /// \return human-readable QString representation of the specified RulesType value
    static QString GetRuleTypeString(RulesType type)
    {
        switch (type)
        {
            case WARNING_RULES_TYPE:
                return GPU_STR_ProjectSettingsRulesWarnings;

            case ERROR_RULES_TYPE:
                return GPU_STR_ProjectSettingsRulesErrors;

            case BEST_PRACTICE_RULES_TYPE:
                return GPU_STR_ProjectSettingsRulesBestPractices;

            default:
                return QString();
        }
    }


    QString   m_displayName; ///< Gets or sets the display name of the rule
    QString   m_name;        ///< Gets the name of the rule
    QString   m_description; ///< Gets rule description
    RulesType m_type;        ///< Gets rule type
    bool      m_enabled;     ///< Gets or sets a value indicating whether or not this rule is enabled

private:
    /// disabled default copy ctor
    CLAPIRule(const CLAPIRule&);

    /// disabled default assignment operator
    CLAPIRule& operator=(const CLAPIRule&);
};

/// Perf counter session related settings
class CounterSettingOption
{
public:
    /// Initializes a new instance of the CounterSettingOption class
    CounterSettingOption();

    /// Initializes a new instance of the CounterSettingOption class with API type
    CounterSettingOption(const APIToTrace& apiType);

    /// Restore the settings to the default values:
    /// \param counterNames list of counters to add to the settings
    void RestoreDefault(const QStringList& counterNames, const APIToTrace& apiType);

    bool m_generateKernelOccupancy; ///< Indicates whether or not generate occupancy information
    gtString m_specificKernels;     ///< List of specific kernels to profile, when empty profile all kernels
    QMap<QString, bool> m_checkedCounterList;   ///< List of counters which are checked in TreeWidget.
    bool m_measureKernelExecutionTime;          ///< Indicates checkbox to measure Kernel Execution Time
    APIToTrace m_api;                           ///< API to trace (HSA / OpenCL)
    bool m_shouldCallXInitThread;               ///< True iff the XInitThread should be initialized. This flag is used in order to
    ///  support a backend workaround

private:
    /// disabled default copy ctor
    CounterSettingOption(const CounterSettingOption&);

    /// disabled default assignment operator
    CounterSettingOption& operator = (const CounterSettingOption&);

};

/// Project settings for GPU Profiler
class ProjectSettings
{
public:

    /// Initializes a new instance of the ProjetSettings class
    ProjectSettings();

    QString              m_projectName;    ///< Contains current project name
    APITraceOptions      m_traceOptions;   ///< APITrace options
    CounterSettingOption m_counterOptions; ///< Counter options

private:
    /// disabled default copy ctor
    ProjectSettings(const ProjectSettings&);

    /// disabled default assignment operator
    ProjectSettings& operator=(const ProjectSettings&);
};

#endif //_PROJECT_SETTING_H_
