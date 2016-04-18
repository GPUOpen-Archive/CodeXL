//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProjectSettings.cpp $
/// \version $Revision: #19 $
/// \brief  This file contains project settings class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProjectSettings.cpp#19 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

//infra
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include "ProjectSettings.h"
#include <AMDTGpuProfiling/Util.h>
#include "CLAPIFilterManager.h"


ProjectSettings::ProjectSettings()
{
}

//-------------------------------------------------------------------
// APITraceOptions
//-------------------------------------------------------------------

APITraceOptions::APITraceOptions() :
    m_defaultMaxAPIs(DEFAULT_NUM_OF_API_CALLS_TO_TRACE),
#ifdef _LINUX
    m_mode(TIMEOUT),  //timeout is the default on Linux
#else
    m_mode(NORMAL),
#endif
    m_timeoutInterval(100),
    m_maxAPICalls(m_defaultMaxAPIs),
    m_generateSummaryPage(true),
    m_queryRetStat(false),
    m_generateSymInfo(false),
    m_collapseClGetEventInfo(true),
    m_generateKernelOccupancy(true),
    m_alwaysShowAPIErrorCode(false),
    m_filterAPIsToTrace(false),
    m_writeDataTimeOut(false),
    m_pFilterManager(NULL),
    m_apiToTrace(APIToTrace_OPENCL)
{

    osFilePath filterFileName;
    Util::GetProfilerAppDataDir(filterFileName);
    filterFileName.setFileName(L"apifilter.txt");
    gtString filterFileNameAsStr = filterFileName.asString();
    m_pFilterManager = new CLAPIFilterManager(acGTStringToQString(filterFileNameAsStr));

    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesWarning1Name, GPU_STR_ProjectSettingsRulesWarning1DisplayName, GPU_STR_ProjectSettingsRulesWarning1Description, WARNING_RULES_TYPE), true);
    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesBestPractice1Name, GPU_STR_ProjectSettingsRulesBestPractice1DisplayName, GPU_STR_ProjectSettingsRulesBestPractice1Description, BEST_PRACTICE_RULES_TYPE), true);
    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesBestPractice2Name, GPU_STR_ProjectSettingsRulesBestPractice2DisplayName, GPU_STR_ProjectSettingsRulesBestPractice2Description, BEST_PRACTICE_RULES_TYPE), true);
    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesError1Name, GPU_STR_ProjectSettingsRulesError1DisplayName, GPU_STR_ProjectSettingsRulesError1Description, ERROR_RULES_TYPE), true);
    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesBestPractice3Name, GPU_STR_ProjectSettingsRulesBestPractice3DisplayName, GPU_STR_ProjectSettingsRulesBestPractice3Description, BEST_PRACTICE_RULES_TYPE), true);
    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesBestPractice4Name, GPU_STR_ProjectSettingsRulesBestPractice4DisplayName, GPU_STR_ProjectSettingsRulesBestPractice4Description, BEST_PRACTICE_RULES_TYPE), true);
    m_rules.insert(new CLAPIRule(GPU_STR_ProjectSettingsRulesWarning2Name, GPU_STR_ProjectSettingsRulesWarning2DisplayName, GPU_STR_ProjectSettingsRulesWarning2Description, WARNING_RULES_TYPE), true);
}

APITraceOptions::~APITraceOptions()
{
    SAFE_DELETE(m_pFilterManager);

    ClearRules();

}

void APITraceOptions::ClearRules()
{
    for (QMap<CLAPIRule*, bool>::iterator i = m_rules.begin(); i != m_rules.end(); ++i)
    {
        CLAPIRule* pRule = i.key();
        SAFE_DELETE(pRule);
    }

    m_rules.clear();
}

void APITraceOptions::RestoreDefault()
{
    APITraceOptions opts;
    (*this) = opts;
}

APITraceOptions& APITraceOptions::operator=(const APITraceOptions& other)
{
    m_mode = other.m_mode;
    m_timeoutInterval = other.m_timeoutInterval;
    m_maxAPICalls = other.m_maxAPICalls;

    ClearRules();

    for (QMap<CLAPIRule*, bool>::const_iterator i = other.m_rules.begin(); i != other.m_rules.end(); ++i)
    {
        CLAPIRule* pRule = i.key();
        CLAPIRule* pNewRule = new CLAPIRule(pRule->m_name, pRule->m_displayName, pRule->m_description, pRule->m_type);
        m_rules[pNewRule] = i.value();
    }

    m_generateSummaryPage = other.m_generateSummaryPage;
    m_queryRetStat = other.m_queryRetStat;
    m_generateSymInfo = other.m_generateSymInfo;
    m_collapseClGetEventInfo = other.m_collapseClGetEventInfo;
    m_generateKernelOccupancy = other.m_generateKernelOccupancy;
    m_alwaysShowAPIErrorCode = other.m_alwaysShowAPIErrorCode;
    m_filterAPIsToTrace = other.m_filterAPIsToTrace;
    m_writeDataTimeOut = other.m_writeDataTimeOut;
    m_apiToTrace = other.m_apiToTrace;

    if ((m_pFilterManager != nullptr) && (other.m_pFilterManager != nullptr))
    {
        m_pFilterManager->SetAPIFilterSet(other.m_pFilterManager->APIFilterSet());
    }

    return (*this);
}

void APITraceOptions::GetListOfRules(QStringList& listOfRules) const
{
    for (QMap<CLAPIRule*, bool>::const_iterator i = m_rules.begin(); i != m_rules.end(); ++i)
    {
        if (i.value())
        {
            QString ruleName = i.key()->GetDisplayName();
            ruleName = ruleName.replace(' ', "_");
            listOfRules << ruleName;
        }
    }
}

void APITraceOptions::EnableRule(const QString& ruleAsString, bool isEnabled)
{
    QMap<CLAPIRule*, bool>::const_iterator iter = m_rules.constBegin();

    for (; iter != m_rules.constEnd(); iter++)
    {
        CLAPIRule* pRule = iter.key();

        if (pRule != nullptr)
        {
            if (pRule->GetDisplayName() == ruleAsString)
            {
                m_rules[pRule] = isEnabled;
                break;
            }
        }
    }
}

void APITraceOptions::ResetRules(bool isEnabled)
{
    // Enable all rules:
    for (QMap<CLAPIRule*, bool>::iterator i = m_rules.begin(); i != m_rules.end(); ++i)
    {
        m_rules[i.key()] = isEnabled;
    }
}

bool APITraceOptions::IsRuleEnabled(const QString& ruleName)
{
    bool retVal = false;

    QMap<CLAPIRule*, bool>::const_iterator iter = m_rules.constBegin();

    for (; iter != m_rules.constEnd(); iter++)
    {
        CLAPIRule* pRule = iter.key();

        if (pRule != nullptr)
        {
            if (pRule->GetDisplayName() == ruleName)
            {
                retVal = m_rules[pRule];
                break;
            }
        }
    }

    return retVal;
}

//-------------------------------------------------------------------
// CLAPIRule
//-------------------------------------------------------------------

CLAPIRule::CLAPIRule(const QString& name, const QString& displayName, const QString& description, RulesType type) :
    m_displayName(displayName),
    m_description(description),
    m_type(type),
    m_enabled(true)
{
    m_name = "APITrace.APIRules." + name;
}

QString CLAPIRule::GetName() const
{
    return m_name;
}

QString CLAPIRule::GetDescription() const
{
    return m_description;
}

RulesType CLAPIRule::GetType() const
{
    return m_type;
}

QString CLAPIRule::GetDisplayName() const
{
    return m_displayName;
}

CounterSettingOption::CounterSettingOption() :
    m_generateKernelOccupancy(true), m_specificKernels(L""), m_measureKernelExecutionTime(true), m_api(APIToTrace_OPENCL), m_shouldCallXInitThread(false)
{
    m_checkedCounterList.clear();
}

void CounterSettingOption::RestoreDefault(const QStringList& counterNames)
{
    CounterSettingOption restore;
    (*this) = restore;

    // Enable all counters by default:
    foreach (QString counter, counterNames)
    {
        m_checkedCounterList[counter] = true;
    }
}

CounterSettingOption& CounterSettingOption::operator=(const CounterSettingOption& other)
{
    m_generateKernelOccupancy = other.m_generateKernelOccupancy;
    m_specificKernels = other.m_specificKernels;
    m_checkedCounterList = other.m_checkedCounterList;
    m_measureKernelExecutionTime = other.m_measureKernelExecutionTime;
    m_shouldCallXInitThread = other.m_shouldCallXInitThread;
    m_api = other.m_api;

    return (*this);
}


