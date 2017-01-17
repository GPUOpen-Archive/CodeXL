//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTDbAdapter.cpp
///
//==================================================================================

#include <AMDTDbAdapter.h>


AmdtDatabaseAdapter::AmdtDatabaseAdapter() :
    m_pDbAccessor(new AMDTProfilerDAL::AmdtDatabaseAccessor()),
    m_isDbToBeClosed(false),
    m_profileMode(AMDT_PROFILE_MODE_NONE)
{
}

AmdtDatabaseAdapter::~AmdtDatabaseAdapter()
{
    if (m_isDbToBeClosed && (m_pDbAccessor != nullptr))
    {
        // Flush the DB and clean.
        m_pDbAccessor->FlushData();

        // Reset the flag.
        m_isDbToBeClosed = false;
    }

    if (m_pDbAccessor != nullptr)
    {
        delete m_pDbAccessor;
        m_pDbAccessor = nullptr;
    }
}

bool AmdtDatabaseAdapter::CreateDb(const gtString& dbName, AMDTProfileMode profileMode)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        // Create the session DB.
        ret = m_pDbAccessor->CreateProfilingDatabase(dbName, profileMode);

        if (ret)
        {
            // Turn on the flag so that we will know that the DB has to be closed.
            m_isDbToBeClosed = true;
            m_profileMode = profileMode;
        }
    }

    return ret;
}

bool AmdtDatabaseAdapter::OpenDb(const gtString& dbName, AMDTProfileMode profileMode, bool isReadOnly)
{
    m_profileMode = profileMode;
    return m_pDbAccessor->OpenProfilingDatabase(dbName, profileMode, isReadOnly);
}

bool AmdtDatabaseAdapter::MigrateDb(const gtString& dbName)
{
    // This add the new columns and update the version
    return m_pDbAccessor->MigrateProfilingDatabase(dbName);
}

bool AmdtDatabaseAdapter::CloseDb()
{
    return m_pDbAccessor->CloseAllConnections();
}

bool AmdtDatabaseAdapter::FlushDb()
{
    return m_pDbAccessor->FlushData();
}

bool AmdtDatabaseAdapter::FlushDbAsync()
{
    return m_pDbAccessor->FlushDataAsync();
}

bool AmdtDatabaseAdapter::GetDbFileExtension(AMDTProfileMode profileMode, gtString& extension) const
{
    if (profileMode == AMDT_PROFILE_MODE_AGGREGATION)
    {
        extension.assign(CPUP_DB_FILE_EXTENSION);
    }
    else if (profileMode == AMDT_PROFILE_MODE_TIMELINE)
    {
        extension.assign(PWRP_DB_FILE_EXTENSION);
    }

    return true;
}

bool AmdtDatabaseAdapter::GetDbVersion(int& version)
{
    return m_pDbAccessor->GetDbVersion(version);
}

int AmdtDatabaseAdapter::GetSupportedDbVersion(void)
{
    return AMDT_CURRENT_PROFILE_DB_VERSION;
}

bool AmdtDatabaseAdapter::PrepareDb()
{
    bool ret = false;

    if (IsAggregateMode())
    {
        ret = m_pDbAccessor->PrepareProfilingDatabase();
    }

    return ret;
}

bool AmdtDatabaseAdapter::InsertSessionInfoKeyValue(const gtString& key, gtString& value)
{
    return m_pDbAccessor->InsertSessionInfoKeyValue(key, value);
}

bool AmdtDatabaseAdapter::InsertSessionInfo(const gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
{
    return m_pDbAccessor->InsertSessionInfo(sessionInfoKeyValueVec);
}

bool AmdtDatabaseAdapter::InsertSessionInfo(const AMDTProfileSessionInfo& sessionInfo)
{
    bool ret = false;

    if (m_pDbAccessor != nullptr)
    {
        // FIXME: why?.. shouldn't the DAL take care of this
        // Begin a new transaction.
        m_pDbAccessor->FlushData();

        // Insert the other session info.
        ret = InsertSessionConfiguration(sessionInfo);
    }

    // FIXME: why?.. shouldn't the DAL take care of this
    // Commit the transaction.
    m_pDbAccessor->FlushData();

    return ret;
}

bool AmdtDatabaseAdapter::GetSessionInfoValue(const gtString& key, gtString& value)
{
    return m_pDbAccessor->GetSessionInfoValue(key, value);
}

bool AmdtDatabaseAdapter::GetAllSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
{
    return m_pDbAccessor->GetAllSessionInfo(sessionInfoKeyValueVec);
}

bool AmdtDatabaseAdapter::GetSessionInfo(AMDTProfileSessionInfo& sessionInfo)
{
    return GetSessionConfiguration(sessionInfo);
}

bool AmdtDatabaseAdapter::IsTimelineMode()
{
    return ((m_profileMode & AMDT_PROFILE_MODE_TIMELINE) == AMDT_PROFILE_MODE_TIMELINE) ? true : false;
}

bool AmdtDatabaseAdapter::IsAggregateMode()
{
    return ((m_profileMode & AMDT_PROFILE_MODE_AGGREGATION) == AMDT_PROFILE_MODE_AGGREGATION) ? true : false;
}

bool AmdtDatabaseAdapter::InsertSessionConfiguration(const AMDTProfileSessionInfo& sessionInfo)
{
    // Insert the target machine name.
    bool isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_TARGET_MACHINE, sessionInfo.m_targetMachineName);
    GT_ASSERT(isOk);

    // Insert the target application's working dir.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_TARGET_APP_WORKING_DIR, sessionInfo.m_targetAppWorkingDir);
    GT_ASSERT(isOk);

    // Insert the target application's path.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_TARGET_APP_PATH, sessionInfo.m_targetAppPath);
    GT_ASSERT(isOk);

    // Insert the target application's working dir.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SESSION_DIR, sessionInfo.m_sessionDir);
    GT_ASSERT(isOk);

    // Insert the target application's command line arguments.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_TARGET_APP_CMD_LINE_ARGS, sessionInfo.m_targetAppCmdLineArgs);
    GT_ASSERT(isOk);

    // Insert the target application's environment variables.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_TARGET_APP_ENV_VARS, sessionInfo.m_targetAppEnvVars);
    GT_ASSERT(isOk);

    // Insert the session type.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SESSION_TYPE, sessionInfo.m_sessionType);
    GT_ASSERT(isOk);

    // Insert the session scope.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SESSION_SCOPE, sessionInfo.m_sessionScope);
    GT_ASSERT(isOk);

    // Insert the system details.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SYSTEM_DETAILS, sessionInfo.m_systemDetails);
    GT_ASSERT(isOk);

    // Insert the session start time.
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SESSION_START_TIME, sessionInfo.m_sessionStartTime);
    GT_ASSERT(isOk);

    // Insert the session end time.
    if (IsTimelineMode())
    {
        isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SESSION_END_TIME, sessionInfo.m_sessionEndTime);
        GT_ASSERT(isOk);
    }

    if (IsAggregateMode())
    {
        gtString valueStr;

        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_SESSION_END_TIME, sessionInfo.m_sessionEndTime);

        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CSS_ENABLED, sessionInfo.m_cssEnabled ? AMDT_SESSION_INFO_VALUE_YES : AMDT_SESSION_INFO_VALUE_NO);

        valueStr << sessionInfo.m_unwindDepth;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CSS_UNWIND_DEPTH, valueStr);

        valueStr.makeEmpty();
        valueStr << sessionInfo.m_unwindScope;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CSS_UNWIND_SCOPE, valueStr);

        valueStr.makeEmpty();
        valueStr << sessionInfo.m_cssInterval;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CSS_INTERVAL, valueStr);

        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_FPO_ENABLED, sessionInfo.m_cssFPOEnabled ? AMDT_SESSION_INFO_VALUE_YES : AMDT_SESSION_INFO_VALUE_NO);

        valueStr.makeEmpty();
        valueStr << sessionInfo.m_cpuFamily;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CPU_FAMILY, valueStr);

        valueStr.makeEmpty();
        valueStr << sessionInfo.m_cpuModel;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CPU_MODEL, valueStr);

        valueStr.makeEmpty();
        valueStr << sessionInfo.m_coreCount;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CORE_COUNT, valueStr);

        valueStr.makeEmpty();
        valueStr << (unsigned long)sessionInfo.m_coreAffinity;
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_CORE_AFFNITY, valueStr);

        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_COLLECTOR_VERSION, sessionInfo.m_codexlCollectorVer);
        m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_TRANSLATOR_VERSION, sessionInfo.m_codexlTranslatorVer);
    }

    // Insert the DB scheme version.
    // This is useless.. rather we should use PRAGMA user_version
    isOk = m_pDbAccessor->InsertSessionInfoKeyValue(AMDT_SESSION_INFO_KEY_DB_SCHEME_VERSION, AMDT_SESSION_INFO_VALUE_DB_SCHEME_VERSION);
    GT_ASSERT(isOk);

    return isOk;
}

bool AmdtDatabaseAdapter::GetSessionConfiguration(AMDTProfileSessionInfo& sessionInfo)
{
    bool isOk = false;

    if (m_pDbAccessor != nullptr)
    {
        // Get the target machine name.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_TARGET_MACHINE), sessionInfo.m_targetMachineName);
        GT_ASSERT(isOk);

        // Get the target application's working dir.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_TARGET_APP_WORKING_DIR), sessionInfo.m_targetAppWorkingDir);
        GT_ASSERT(isOk);

        // Get the target application's path.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_TARGET_APP_PATH), sessionInfo.m_targetAppPath);
        GT_ASSERT(isOk);

        // Get the target application's working dir.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_SESSION_DIR), sessionInfo.m_sessionDir);
        GT_ASSERT(isOk);

        // Get the target application's command line arguments.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_TARGET_APP_CMD_LINE_ARGS), sessionInfo.m_targetAppCmdLineArgs);
        GT_ASSERT(isOk);

        // Get the target application's environment variables.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_TARGET_APP_ENV_VARS), sessionInfo.m_targetAppEnvVars);
        GT_ASSERT(isOk);

        // Get the session type.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_SESSION_TYPE), sessionInfo.m_sessionType);
        GT_ASSERT(isOk);

        // Get the session scope.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_SESSION_SCOPE), sessionInfo.m_sessionScope);
        GT_ASSERT(isOk);

        // Get the system details.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_SYSTEM_DETAILS), sessionInfo.m_systemDetails);
        GT_ASSERT(isOk);

        // Get the session start time.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_SESSION_START_TIME), sessionInfo.m_sessionStartTime);
        GT_ASSERT(isOk);

        // Get the session end time.
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_SESSION_END_TIME), sessionInfo.m_sessionEndTime);
        GT_ASSERT(isOk);

    }

    if (isOk && IsTimelineMode())
    {
        // Calculate the sessionEndTime.
        osTime startTime;
        startTime.setFromDateTimeString(osTime::LOCAL, sessionInfo.m_sessionStartTime, osTime::NAME_SCHEME_FILE);

        // Calculate the session duration.
        SamplingTimeRange sessionTimeRange(0, 0);
        isOk = m_pDbAccessor->GetSamplesTimeRange(sessionTimeRange);

        GT_IF_WITH_ASSERT(isOk)
        {
            unsigned int sessionDurationInSeconds = sessionTimeRange.m_toTime / 1000;
            startTime.addSeconds(sessionDurationInSeconds);
            startTime.dateAsString(sessionInfo.m_sessionEndTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);
        }
    }

    if (isOk && IsAggregateMode())
    {
        gtString valueStr;

        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CPU_FAMILY), valueStr);

        if (isOk)
        {
            valueStr.toUnsignedIntNumber(sessionInfo.m_cpuFamily);
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CPU_MODEL), valueStr);

        if (isOk)
        {
            valueStr.toUnsignedIntNumber(sessionInfo.m_cpuModel);
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CORE_AFFNITY), valueStr);

        if (isOk)
        {
            valueStr.toUnsignedInt64Number(sessionInfo.m_coreAffinity);
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CSS_UNWIND_DEPTH), valueStr);

        if (isOk)
        {
            gtUInt32 cssDepth = 0;
            valueStr.toUnsignedIntNumber(cssDepth);
            sessionInfo.m_unwindDepth = static_cast<gtUInt16>(cssDepth);
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CSS_UNWIND_SCOPE), valueStr);

        if (isOk)
        {
            gtUInt32 cssScope = 0;
            valueStr.toUnsignedIntNumber(cssScope);
            sessionInfo.m_unwindScope = static_cast<gtUInt16>(cssScope);
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CSS_ENABLED), valueStr);

        if (isOk)
        {
            sessionInfo.m_cssEnabled = (valueStr.compareNoCase(L"YES") == 0) ? true : false;
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_FPO_ENABLED), valueStr);

        if (isOk)
        {
            sessionInfo.m_cssFPOEnabled = (valueStr.compareNoCase(L"YES") == 0) ? true : false;
        }

        valueStr.makeEmpty();
        isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_KEY_DB_SCHEME_VERSION), valueStr);

        if (isOk)
        {
            gtUInt16 major = CODEXL_DB_SCHEME_VERSION_MAJOR;
            gtUInt16 minor = CODEXL_DB_SCHEME_VERSION_MINOR;

            if (valueStr.length() > 2)
            {
                major = valueStr[0] - L'0';
                minor = valueStr[2] - L'0';
            }

            bool isEqOrHigherVersion_2_1 = (major > 2 || (major == 2 && minor >= 1)) ? true : false;

            if (isEqOrHigherVersion_2_1)
            {
                valueStr.makeEmpty();
                isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CORE_COUNT), valueStr);

                if (isOk)
                {
                    gtUInt32 coreCount = 0;
                    valueStr.toUnsignedIntNumber(coreCount);
                    sessionInfo.m_coreCount = coreCount;
                }

                valueStr.makeEmpty();
                isOk = m_pDbAccessor->GetSessionInfoValue(gtString(AMDT_SESSION_INFO_CSS_INTERVAL), valueStr);

                if (isOk)
                {
                    gtUInt32 cssInterval = 0;
                    valueStr.toUnsignedIntNumber(cssInterval);
                    sessionInfo.m_cssInterval = static_cast<gtUInt16>(cssInterval);
                }
            }
            else
            {
                gtVector<AMDTCpuTopology> cpuTopology;
                m_pDbAccessor->GetCpuTopology(cpuTopology);

                sessionInfo.m_coreCount = cpuTopology.size();
                sessionInfo.m_cssInterval = 1;
            }
        }
    }

    return isOk;
}