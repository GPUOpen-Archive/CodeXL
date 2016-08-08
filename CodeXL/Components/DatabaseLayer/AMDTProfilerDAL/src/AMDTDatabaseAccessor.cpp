//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTDatabaseAccessor.cpp
///
//==================================================================================

// C++.
#include <string>
#include <sstream>

// Local.
#include <AMDTProfilerDAL/include/AMDTDatabaseAccessor.h>
#include <AMDTProfilerDAL/include/dbTxCommitThread.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// sqlite.
#include <sqlite3.h>

#define IS_PROCESS_MODULE_QUERY(procId_, modId_)        ((procId_ != AMDT_PROFILE_ALL_PROCESSES) && (modId_ != AMDT_PROFILE_ALL_MODULES))
#define IS_PROCESS_THREAD_QUERY(procId_, threadId_)     ((procId_ != AMDT_PROFILE_ALL_PROCESSES) && (threadId_ != AMDT_PROFILE_ALL_THREADS))
#define IS_PROCESS_QUERY(procId_)                       (procId_ != AMDT_PROFILE_ALL_PROCESSES)
#define IS_MODULE_QUERY(modId_)                         (modId_ != AMDT_PROFILE_ALL_MODULES)
#define IS_THREAD_QUERY(threadId_)                      (threadId_ != AMDT_PROFILE_ALL_THREADS)
#define IS_FUNCTION_QUERY(funcId_)                      (funcId_ != AMDT_PROFILE_ALL_FUNCTIONS)
#define IS_ALL_PROCESS_QUERY(procId_)                   (procId_ == AMDT_PROFILE_ALL_PROCESSES)
#define IS_ALL_MODULE_QUERY(modId_)                     (modId_ == AMDT_PROFILE_ALL_MODULES)
#define IS_ALL_THREAD_QUERY(threadId_)                  (threadId_ == AMDT_PROFILE_ALL_THREADS)
#define IS_ALL_FUNCTION_QUERY(funcId_)                  (funcId_ == AMDT_PROFILE_ALL_FUNCTIONS)

#define IS_COUNTER_CORE_QUERY(counterId_, coreMask_)    (counterId_ != AMDT_PROFILE_ALL_COUNTERS && coreMask_ != AMDT_PROFILE_ALL_CORES)
#define IS_COUNTER_QUERY(counterId_)                    (counterId_ != AMDT_PROFILE_ALL_COUNTERS)
#define IS_CORE_QUERY(coreMask_)                        (coreMask_ != AMDT_PROFILE_ALL_CORES)
#define IS_ALL_COUNTER_QUERY(counterId_)                (counterId_ == AMDT_PROFILE_ALL_COUNTERS)
#define IS_ALL_CORE_QUERY(coreMask_)                    (coreMask_ == AMDT_PROFILE_ALL_CORES)

#define IS_ALL_CALLSTACK_QUERY(cs_)                     (cs_ == AMDT_PROFILE_ALL_CALLPATHS)
#define IS_CALLSTACK_QUERY(cs_)                         (cs_ != AMDT_PROFILE_ALL_CALLPATHS)
#define IS_UNKNOWN_FUNC(id_)                            (((id_) & 0x0000ffff) == 0)

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#define LONG_FORMAT     L"%ld"
#define LONG_FORMAT_HEX     L"%lx"
#define STR_FORMAT L"%s"
#else
#define LONG_FORMAT     L"%lld"
#define LONG_FORMAT_HEX     L"%llx"
#define STR_FORMAT L"%S"
#endif

#define DB_MODULEID_MASK        0xFFFF0000UL
#define DB_FUNCTIONID_MASK      0x0000FFFFUL
#define DB_MODULEID_SHIFT_BITS  16
#define CXL_GET_DB_MODULE_ID(funcId_)   (((funcId_) & DB_MODULEID_MASK) >> DB_MODULEID_SHIFT_BITS)

// For JIT modules funtion-id is also the module-instance-id
#define CXL_GET_DB_JIT_MODULE_INSTANCE_ID(funcId_)   ((funcId_) & DB_FUNCTIONID_MASK)

#define CXL_CLU_EVENT_CLU_PERCENTAGE        0xFF00UL
#define CXL_CLU_EVENT_L1_EVICTIONS          0xFF04UL

#ifdef PP_DAL_TEST

    // Test.
    #include <ctime>
    #include <iostream>

#endif

namespace AMDTProfilerDAL
{

#ifdef PP_DAL_TEST

// This structure is used for measuring execution time.
struct QueryProfiler
{
    QueryProfiler(const char* functionName) : m_begin(clock()), m_functionName(functionName) {}
    ~QueryProfiler()
    {
        clock_t end = clock();
        double elapsed_secs = double(end - m_begin) / CLOCKS_PER_SEC;
        std::cout << "Query " << m_functionName << " took " << elapsed_secs << "[sec]" << std::endl;
    }

    clock_t m_begin;
    const char* m_functionName;
};

#endif // PP_DAL_TEST

// DB Creation Statements.
const std::vector<std::string> SQL_CREATE_DB_STMTS_COMMON =
{
    "CREATE TABLE sessionInfo (settingKey TEXT, settingValue TEXT);",
};

const std::vector<std::string> SQL_CREATE_DB_STMTS_TIMELINE =
{
    "CREATE TABLE counters (counterId INTEGER NOT NULL, deviceId INTEGER, counterName TEXT, counterDescription TEXT, counterCategoryId INTEGER, counterCategory TEXT, counterAggregationId INTEGER, counterAggregation TEXT, counterUnitId INTEGER, counterUnit TEXT, PRIMARY KEY(counterId));",
    "CREATE TABLE devices (deviceId INTEGER NOT NULL, deviceTypeId INTEGER, deviceType TEXT, deviceName TEXT, deviceDescription TEXT, PRIMARY KEY(deviceId));",
    "CREATE TABLE subDevices (parentDeviceId INTEGER NOT NULL, childDeviceId INTEGER);",
    "CREATE TABLE counterControlEvents (counterId INTEGER NOT NULL, quantizedTimeMs INTEGER, action TEXT);",
    "CREATE TABLE samplingIntervalSetting (quantizedTimeMs INTEGER, intervalMs INTEGER);",
    "CREATE TABLE samples (quantizedTimeMs INTEGER NOT NULL, counterId INTEGER NOT NULL, sampledValue REAL);",
    "CREATE INDEX sampleTimeIdx ON samples (quantizedTimeMs);",
    "CREATE INDEX sampleCounterIdx ON samples (counterId);",
    "CREATE INDEX counterNameIdx ON counters (counterName);"
};

const std::vector<std::string> SQL_CREATE_DB_STMTS_AGGREGATION =
{
    "CREATE TABLE Core (id INTEGER NOT NULL PRIMARY KEY, processorId INTEGER, numaNodeId INTEGER)",
    "CREATE TABLE SamplingCounter (id INTEGER NOT NULL PRIMARY KEY, name TEXT, abbrev TEXT, description TEXT)",
    "CREATE TABLE SamplingConfiguration (id INTEGER PRIMARY KEY, counterId INTEGER, samplingInterval INTEGER, unitMask INTEGER, isUserMode INTEGER, isOsMode INTEGER, edge INTEGER)",
    "CREATE TABLE CoreSamplingConfiguration (id INTEGER PRIMARY KEY, coreId INTEGER, samplingConfigurationId INTEGER)", // FOREIGN KEY(samplingConfigurationId) REFERENCES SamplingConfiguration(id), FOREIGN KEY(coreId) REFERENCES Core(id)
    "CREATE TABLE Process (id INTEGER NOT NULL PRIMARY KEY, name TEXT, is32Bit INTEGER, hasCSS INTEGER)",
    "CREATE TABLE Module (id INTEGER PRIMARY KEY, path TEXT, isSystemModule INTEGER, is32Bit INTEGER, type INTEGER, size INTEGER, foundDebugInfo INTEGER)",
    "CREATE TABLE ModuleInstance (id INTEGER PRIMARY KEY, processId INTEGER, moduleId INTEGER, loadAddress INTEGER)", // FOREIGN KEY(processId) REFERENCES Process(id), FOREIGN KEY(moduleId) REFERENCES Module(id)
    "CREATE TABLE ProcessThread (id INTEGER PRIMARY KEY, processId INTEGER, threadId INTEGER)", // FOREIGN KEY(processId) REFERENCES Process(id)
    "CREATE TABLE Function (id INTEGER PRIMARY KEY, moduleId INTEGER, name TEXT, startOffset INTEGER, size INTEGER)", // FOREIGN KEY(moduleId) REFERENCES module(id)
    "CREATE TABLE SampleContext (id INTEGER PRIMARY KEY AUTOINCREMENT, processThreadId INTEGER, moduleInstanceId INTEGER, coreSamplingConfigurationId INTEGER, functionId INTEGER, offset INTEGER, count INTEGER)", // FOREIGN KEY(processThreadId) REFERENCES ProcessThread(rowid), FOREIGN KEY(moduleInstanceId) REFERENCES ModuleInstance(id), FOREIGN KEY(coreSamplingConfigurationId) REFERENCES CoreSamplingConfiguration(id)
    "CREATE TABLE CallstackFrame (callstackId INTEGER, processId INTEGER, functionId INTEGER, offset INTEGER, depth INTEGER)", // FOREIGN KEY(functionId) REFERENCES Function(id)
    "CREATE TABLE CallstackLeaf (callstackId INTEGER, processId INTEGER, functionId INTEGER, offset INTEGER, samplingConfigurationId INTEGER, selfSamples INTEGER)", // FOREIGN KEY(functionId) REFERENCES Function(id)
    "CREATE TABLE JITInstance (jitId INTEGER, functionId INTEGER, processId INTEGER, loadAddress INTEGER, size INTEGER)", // FOREIGN KEY(jitId) REFERENCES JITCodeBlob(id), FOREIGN KEY(functionId) REFERENCES Function(id), FOREIGN KEY(processId) REFERENCES Process(id)
    "CREATE TABLE JITCodeBlob (id INTEGER PRIMARY KEY, srcFilePath TEXT, jncFilePath TEXT)",
    //"CREATE TABLE Callgraph (id INTEGER NOT NULL, callerId INTEGER, calleeId INTEGER, edgeLevel INTEGER)", // FOREIGN KEY(callerId) REFERENCES Function(id), FOREIGN KEY(calleeId) REFERENCES Function(id), FOREIGN KEY(samplingConfigurationId) REFERENCES SamplingConfiguration(id)
    //"CREATE TABLE CallgraphSampleAggregation (callgraphId INTEGER NOT NULL, sampleContextId INTEGER, selfSamples INTEGER, deepSamples INTEGER)", // FOREIGN KEY(callgraphId) REFERENCES Callgraph(id), FOREIGN KEY(sampleContextId) REFERENCES SampleContext(id)
    "CREATE UNIQUE INDEX 'unique_samples' ON SampleContext (processThreadId, moduleInstanceId, coreSamplingConfigurationId, functionId, offset)",
    "CREATE INDEX callStackLeafIdx ON CallstackLeaf (processId, samplingConfigurationId, callstackId)",
    "CREATE INDEX callStackLeafFunctionIdx ON CallstackLeaf (functionId, offset)",
    "CREATE INDEX callStackFrameIdx ON CallstackFrame (callstackId, processId)",
    "CREATE INDEX callStackFrameFunctionIdx ON CallstackFrame (functionId, offset)",
    "CREATE INDEX sampleContextIdx ON SampleContext (functionId, offset)",
    "CREATE INDEX processThreadIdx ON ProcessThread(processId, threadId)",
    "CREATE INDEX processThreadIdx1 ON ProcessThread(id)",
    "CREATE INDEX moduleInstanceIdx ON ModuleInstance(id)",
    "CREATE INDEX moduleIdx ON Module(id)",
    "CREATE INDEX SampleContextIdx1 ON SampleContext(coreSamplingConfigurationId)",
    "CREATE INDEX FunctionIdx ON Function(id)",
};

// Migrate table for version 1 - add new columns in tables "devices" and "counters"
const std::vector<std::string> SQL_ALTER_TABLE_STMTS_VERSION_1 =
{
    "ALTER TABLE devices add column deviceTypeId integer;",
    "ALTER TABLE counters add column counterCategoryId integer;",
    "ALTER TABLE counters add column counterUnitId integer;",
    "ALTER TABLE counters add column counterAggregationId integer;"
};

// Update the values for the new columns in tables "devices" and "counters" for version 1
const std::vector<std::string> SQL_UPDATE_TABLE_STMTS_VERSION_1 =
{
    "UPDATE devices set deviceTypeId = ? where deviceType = ?;",
    "UPDATE counters set counterCategoryId = ? where counterCategory = ?;",
    "UPDATE counters set counterAggregationId  = ? where counterAggregation  = ?;",
    "UPDATE counters set counterUnitId  = ? where counterUnit = ?;"
};

// Begin transaction Statement.
const char* SQL_CMD_TX_BEGIN = "BEGIN TRANSACTION";

// Commit transaction Statement.
const char* SQL_CMD_TX_COMMIT = "COMMIT TRANSACTION";

// Sqlite Version
const char* SQL_CMD_SET_USER_VERSION = "PRAGMA user_version=1";
const char* SQL_CMD_GET_USER_VERSION = "PRAGMA user_version";
const char* SQL_CMD_SET_SYNCHRONOUS = "PRAGMA synchronous=0"; // off

// A reference counter for number of sqlite connections.
// Will be used to decide whether to shutdown sqlite.
static int gs_SQLITE_NUM_OF_CLIENTS = 0;

class AmdtDatabaseAccessor::Impl
{
public:
    Impl(AmdtDatabaseAccessor* pParent) : m_pParent(pParent)
    {
        if (gs_SQLITE_NUM_OF_CLIENTS == 0)
        {
            // Configure sqlite to be fully thread-safe.
            int rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
            GT_ASSERT(rc == SQLITE_OK);

            // Initialize sqlite.
            rc = sqlite3_initialize();
            GT_ASSERT(rc == SQLITE_OK);

            if (rc != SQLITE_OK)
            {
                OS_OUTPUT_DEBUG_LOG(L"Could not initialize sqlite", OS_DEBUG_LOG_ERROR);
            }
        }

        // Increment the number of clients.
        ++gs_SQLITE_NUM_OF_CLIENTS;
    }

    ~Impl()
    {
        // Commit pending transactions.
        FlushData();

        // Terminate the DB transactions COMMIT thread.
        if (m_pDbTxCommitThread != nullptr)
        {
            m_pDbTxCommitThread->requestExit();

            osTimeInterval timeout;
            timeout.setAsMilliSeconds(200);
            m_pDbTxCommitThread->waitForThreadEnd(timeout);

            m_pDbTxCommitThread->terminate();
            delete m_pDbTxCommitThread;
            m_pDbTxCommitThread = nullptr;
        }

        if (m_isCreateDb)
        {
            // Finalize the statements for timeline related tables
            sqlite3_finalize(m_pValsInsertStmt);
            sqlite3_finalize(m_pCounterEnabledAtInsertStmt);
            sqlite3_finalize(m_pSamplingIntervalInsertStmt);
            sqlite3_finalize(m_pDecivceInsertStmt);
            sqlite3_finalize(m_pSubDevicesInsertStmt);
            sqlite3_finalize(m_pCountersInsertStmt);
            sqlite3_finalize(m_pSessionInfoInsertStmt);

            // Finalize the statements for aggregation related tables
            sqlite3_finalize(m_pCoreInfoInsertStmt);
            sqlite3_finalize(m_pSamplingCounterInsertStmt);
            sqlite3_finalize(m_pSamplingConfigInsertStmt);
            sqlite3_finalize(m_pCoreSamplingConfigInsertStmt);
            sqlite3_finalize(m_pProcessInfoInsertStmt);
            sqlite3_finalize(m_pModuleInfoInsertStmt);
            sqlite3_finalize(m_pModuleInstanceInsertStmt);
            sqlite3_finalize(m_pProcessThreadInsertStmt);
            sqlite3_finalize(m_pSampleContextInsertStmt);
            sqlite3_finalize(m_pSampleContextUpdateStmt);
            sqlite3_finalize(m_pFunctionInfoInsertStmt);
            sqlite3_finalize(m_pModuleIdQueryStmt);
            sqlite3_finalize(m_pSamplingConfigIdQueryStmt);
            sqlite3_finalize(m_pCoreSamplingConfigIdQueryStmt);
            sqlite3_finalize(m_pModuleInstanceIdQueryStmt);
            sqlite3_finalize(m_pProcessThreadIdQueryStmt);
            sqlite3_finalize(m_pFunctionIdQueryStmt);
            sqlite3_finalize(m_pCallStackFrameInsertStmt);
            sqlite3_finalize(m_pCallStackLeafInsertStmt);
            sqlite3_finalize(m_pJitCodeBlobInsertStmt);
            sqlite3_finalize(m_pJitInstanceInsertStmt);
        }

        if (m_isCurrentDbOpenForRead)
        {
            // Data Query Stmts for timeline tables
            sqlite3_finalize(m_pGetSessionRangeStmt);
            sqlite3_finalize(m_pGetAllCountersStmt);
            sqlite3_finalize(m_pGetCounterIdByNameStmt);
            sqlite3_finalize(m_pGetDeviceTypeStmt);
            sqlite3_finalize(m_pGetDeviceTypeIdStmt);
            sqlite3_finalize(m_pGetDeviceTypeByCounterIdStmt);
            sqlite3_finalize(m_pGetDeviceTypeIdByCounterIdStmt);
            sqlite3_finalize(m_pGetSessionCountersByDeviceAndCategoryStmt);
            sqlite3_finalize(m_pGetSessionCountersByDeviceAndCategoryIdsStmt);
            sqlite3_finalize(m_pGetSessionCountersByCategoryStmt);
            sqlite3_finalize(m_pGetSessionCountersByCategoryIdStmt);
            sqlite3_finalize(m_pGetAllSessionCountersStmt);
            sqlite3_finalize(m_pGetSessionInfoValueStmt);
            sqlite3_finalize(m_pGetAllSessionInfoStmt);
            sqlite3_finalize(m_pGetSessionSamplingIntervalStmt);
            sqlite3_finalize(m_pGetSessionCounterIdsByNameStmt);
        }

        // Close the write connection.
        if (m_pWriteDbConn != nullptr)
        {
            sqlite3_close(m_pWriteDbConn);
        }

        // Close the read connection.
        if (m_pReadDbConn != nullptr)
        {
            sqlite3_close(m_pReadDbConn);
        }

        if (--gs_SQLITE_NUM_OF_CLIENTS == 0)
        {
            // Shutdown sqlite.
            int rc = sqlite3_shutdown();
            GT_ASSERT(rc == SQLITE_OK);
        }

        m_moduleIdInfoMap.clear();
    }

    bool PrepareInsertPowerProfilingSampleStatement()
    {
        bool ret = false;

        // Insert power profiling sample statement.
        const char* pCsSqlCmd = "INSERT INTO samples(quantizedTimeMs, counterId, sampledValue) VALUES(?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pValsInsertStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertSubDeviceStatement()
    {
        bool ret = false;

        // Insert sub device statement.
        const char* pCsSqlCmd = "INSERT INTO subDevices(parentDeviceId, childDeviceId) VALUES(?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSubDevicesInsertStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertPowerProfilingDeviceStatement()
    {
        bool ret = false;

        // Insert power profiling device statement.
        const char* pCsSqlCmd = "INSERT INTO devices(deviceId, deviceTypeId, deviceType, deviceName, deviceDescription) VALUES(?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pDecivceInsertStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertPowerProfilingCounterStatement()
    {
        bool ret = false;

        // Insert power profiling counter statement.
        const char* pCsSqlCmd = "INSERT INTO counters(counterId, deviceId, counterName, counterDescription, counterCategoryId, counterCategory, counterAggregationId, counterAggregation, counterUnitId, counterUnit) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pCountersInsertStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertCounterControlEventStatement()
    {
        bool ret = false;

        // Insert counter sampling interval statement.
        const char* pCsSqlCmd = "INSERT INTO counterControlEvents(counterId, quantizedTimeMs, action) VALUES(?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pCounterEnabledAtInsertStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertSessionSamplingIntervalStatement()
    {
        bool ret = false;

        // Insert counter sampling interval setting (ms) statement.
        const char* pCsSqlCmd = "INSERT INTO samplingIntervalSetting(quantizedTimeMs, intervalMs) VALUES(?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSamplingIntervalInsertStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertSessionInfoStatement()
    {
        bool ret = false;

        if (m_pSessionInfoInsertStmt == nullptr)
        {
            const char* pCsSqlCmd = "INSERT INTO sessionInfo VALUES(?, ?);";
            int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSessionInfoInsertStmt, nullptr);
            ret = (rc == SQLITE_OK);
        }
        else
        {
            // already statement prepared
            ret = true;
        }

        return ret;
    }

    bool PrepareAllPwrProfWriteStatements()
    {
        // Init the isFisrtInsert flag.
        m_isFirstInsert = true;

        bool ret = PrepareInsertSessionInfoStatement()               &&
                   PrepareInsertPowerProfilingSampleStatement()      &&
                   PrepareInsertSubDeviceStatement()                 &&
                   PrepareInsertPowerProfilingDeviceStatement()      &&
                   PrepareInsertPowerProfilingCounterStatement()     &&
                   PrepareInsertCounterControlEventStatement()       &&
                   PrepareInsertSessionSamplingIntervalStatement();

        return ret;
    }

    bool PrepareInsertCoreInfoStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO Core(id, processorId, numaNodeId) VALUES(?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pCoreInfoInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertSamplingCounterStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO SamplingCounter(id, name, abbrev, description) VALUES(?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSamplingCounterInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertSamplingConfigStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO SamplingConfiguration(id, counterId, samplingInterval, unitMask, isUserMode, isOsMode, edge) VALUES(?, ?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSamplingConfigInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertCoreSamplingConfigStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO CoreSamplingConfiguration(id, coreId, samplingConfigurationId) VALUES(?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pCoreSamplingConfigInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertProcessInfoStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO Process(id, name, is32Bit, hasCSS) VALUES(?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pProcessInfoInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertModuleInfoStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO Module(id, path, isSystemModule, is32Bit, type, size, foundDebugInfo) VALUES(?, ?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pModuleInfoInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertModuleInstanceStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO ModuleInstance(id, processId, moduleId, loadAddress) VALUES(?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pModuleInstanceInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertProcessThreadStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO ProcessThread(id, processId, threadId) VALUES(?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pProcessThreadInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertSampleContextStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO SampleContext(processThreadId, moduleInstanceId, coreSamplingConfigurationId, functionId, offset, count) VALUES(?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSampleContextInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareUpdateSampleContextStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "UPDATE OR IGNORE SampleContext SET count=count+? WHERE processThreadId=? AND moduleInstanceId=? AND coreSamplingConfigurationId=? AND functionId=? AND offset=?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pSampleContextUpdateStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertFunctionInfoStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO Function(id, moduleId, name, startOffset, size) VALUES(?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pFunctionInfoInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertCallStackFrameStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO CallstackFrame(callstackId, processId, functionId, offset, depth) VALUES(?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pCallStackFrameInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertCallStackLeafStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO CallstackLeaf(callstackId, processId, functionId, offset, samplingConfigurationId, selfSamples) VALUES(?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pCallStackLeafInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertJitInstanceStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO JITInstance (jitId, functionId, processId, loadAddress, size) VALUES(?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pJitInstanceInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareInsertJitCodeBlobStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "INSERT INTO JITCodeBlob (id, srcFilePath, jncFilePath) VALUES(?, ?, ?);";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pJitCodeBlobInsertStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetModuleIdStatement()
    {
        bool ret = false;

        const char* pQuerySqlCmd = "SELECT id FROM Module WHERE path = ?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pQuerySqlCmd, -1, &m_pModuleIdQueryStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetModuleInstanceIdStatement()
    {
        bool ret = false;

        const char* pCsSqlCmd = "SELECT id FROM ModuleInstance WHERE processId = ? AND moduleId = ? AND loadAddress = ?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pCsSqlCmd, -1, &m_pModuleInstanceIdQueryStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetCoreSamplingConfigIdStatement()
    {
        bool ret = false;

        const char* pQuerySqlCmd = "SELECT id FROM CoreSamplingConfiguration WHERE coreId = ? AND samplingConfigurationId = ?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pQuerySqlCmd, -1, &m_pCoreSamplingConfigIdQueryStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetProcessThreadIdStatement()
    {
        bool ret = false;

        const char* pQuerySqlCmd = "SELECT id FROM ProcessThread WHERE processId = ? AND threadId = ?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pQuerySqlCmd, -1, &m_pProcessThreadIdQueryStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSamplingConfigIdStatement()
    {
        bool ret = false;

        const char* pQuerySqlCmd = "SELECT id FROM SamplingConfiguration WHERE counterId = ? AND unitMask = ? AND isUserMode = ? AND isOsMode = ?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pQuerySqlCmd, -1, &m_pSamplingConfigIdQueryStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetFunctionIdStatement()
    {
        bool ret = false;

        const char* pQuerySqlCmd = "SELECT id FROM Function WHERE moduleId = ? AND startOffset = ?;";
        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pQuerySqlCmd, -1, &m_pFunctionIdQueryStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareAllCpuProfWriteStatements()
    {
        // Init the isFisrtInsert flag.
        m_isFirstInsert = true;

        bool ret = PrepareInsertSessionInfoStatement() &&
                   PrepareInsertCoreInfoStatement() &&
                   PrepareInsertSamplingCounterStatement() &&
                   PrepareInsertSamplingConfigStatement() &&
                   PrepareInsertCoreSamplingConfigStatement() &&
                   PrepareInsertProcessInfoStatement() &&
                   PrepareInsertModuleInfoStatement() &&
                   PrepareGetModuleIdStatement() &&
                   PrepareInsertModuleInstanceStatement() &&
                   PrepareInsertProcessThreadStatement() &&
                   PrepareGetSamplingConfigIdStatement() &&
                   PrepareGetModuleInstanceIdStatement() &&
                   PrepareGetCoreSamplingConfigIdStatement() &&
                   PrepareGetProcessThreadIdStatement() &&
                   PrepareInsertFunctionInfoStatement() &&
                   PrepareGetFunctionIdStatement() &&
                   PrepareInsertSampleContextStatement() &&
                   PrepareUpdateSampleContextStatement() &&
                   PrepareInsertCallStackFrameStatement() &&
                   PrepareInsertCallStackLeafStatement() &&
                   PrepareInsertJitInstanceStatement() &&
                   PrepareInsertJitCodeBlobStatement();

        return ret;
    }

    bool PrepareGetSessionTimeRangeStatement()
    {
        bool ret = false;

        // Get frequencies histogram statement.
        const char* pQuerySqlCmd = "SELECT MIN(quantizedTimeMs), MAX(quantizedTimeMs) FROM samples;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionRangeStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetDeviceTypeStatement()
    {
        bool ret = false;

        // Get device type statement.
        const char* pQuerySqlCmd = "SELECT deviceType FROM devices WHERE deviceId = ?;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetDeviceTypeStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetDeviceTypeIdStatement()
    {
        bool ret = false;

        // Get device type statement.
        const char* pQuerySqlCmd = "SELECT deviceTypeId FROM devices WHERE deviceId = ?;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetDeviceTypeIdStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetDeviceTypeByCounterIdStatement()
    {
        bool ret = false;

        // Get device type by counter id statement.
        const char* pQuerySqlCmd = "SELECT deviceType FROM devices, counters WHERE counters.deviceId = devices.deviceId AND counters.counterId = ?;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetDeviceTypeByCounterIdStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetDeviceTypeIdByCounterIdStatement()
    {
        bool ret = false;

        // Get device type by counter id statement.
        const char* pQuerySqlCmd = "SELECT deviceTypeId FROM devices, counters WHERE counters.deviceId = devices.deviceId AND counters.counterId = ?;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetDeviceTypeIdByCounterIdStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionCountersByDeviceAndCategoryStatement()
    {
        bool ret = false;

        // Get session counters statement.
        const char* pQuerySqlCmd = "SELECT DISTINCT counters.counterId FROM counters, counterControlEvents, devices WHERE (counters.counterId = counterControlEvents.counterId AND counters.deviceId = devices.deviceId AND counterControlEvents.action = 'E' AND counters.counterCategory = ? AND devices.deviceType = ?);";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionCountersByDeviceAndCategoryStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionCountersByDeviceAndCategoryIdsStatement()
    {
        bool ret = false;

        // Get session counters statement.
        const char* pQuerySqlCmd = "SELECT DISTINCT counters.counterId FROM counters, counterControlEvents, devices WHERE (counters.counterId = counterControlEvents.counterId AND counters.deviceId = devices.deviceId AND counterControlEvents.action = 'E' AND counters.counterCategoryId = ? AND devices.deviceTypeId = ?);";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionCountersByDeviceAndCategoryIdsStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionCountersByCategoryStatement()
    {
        bool ret = false;

        // Get session counters statement.
        const char* pQuerySqlCmd = "SELECT DISTINCT counters.counterId FROM counters, counterControlEvents WHERE (counters.counterId = counterControlEvents.counterId AND counterControlEvents.action = 'E' AND counters.counterCategory = ?);";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionCountersByCategoryStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionCountersByCategoryIdStatement()
    {
        bool ret = false;

        // Get session counters statement.
        //const char* pQuerySqlCmd = "SELECT DISTINCT counters.counterId FROM counters, counterControlEvents WHERE counters.counterId = counterControlEvents.counterId AND counterControlEvents.action = 'E' AND counters.counterCategoryId = ?;";
        const char* pQuerySqlCmd = "SELECT DISTINCT counters.counterId FROM counters, counterControlEvents WHERE (counters.counterId = counterControlEvents.counterId AND counterControlEvents.action = 'E' AND counters.counterCategoryId = ?);";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionCountersByCategoryIdStmt, nullptr);
        GT_ASSERT(rc == SQLITE_OK);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetAllSessionCountersStatement()
    {
        bool ret = false;

        // Get session counters statement.
        const char* pQuerySqlCmd = "SELECT DISTINCT counters.*  FROM counters, counterControlEvents WHERE (counters.counterId = counterControlEvents.counterId AND counterControlEvents.action = 'E');";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetAllSessionCountersStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetCounterIdByNameStatement()
    {
        bool ret = false;

        // This query would have to be extended in future CodeXL versions if the name of the counter that
        // represents APU Power will be changed.
        const char* pQuerySqlCmd = "SELECT counterId FROM counters WHERE counterName = ?;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetCounterIdByNameStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionInfoValueStatement()
    {
        bool ret = false;

        // Get session info value statement.
        const char* pQuerySqlCmd = "SELECT settingValue FROM SessionInfo WHERE settingKey = ?;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionInfoValueStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetAllSessionInfoValueStatement()
    {
        bool ret = false;

        // Get session info value statement.
        const char* pQuerySqlCmd = "SELECT settingKey, settingValue FROM SessionInfo;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetAllSessionInfoStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionSamplingIntervalMsStatement()
    {
        bool ret = false;

        // Get session sampling interval statement.
        const char* pQuerySqlCmd = "SELECT intervalMs FROM samplingIntervalSetting WHERE quantizedTimeMs = 0;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionSamplingIntervalStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareGetSessionCounterNames()
    {
        bool ret = false;

        // Prepare the session time range statement.
        const char* pQuerySqlCmd = "SELECT counterName, counterId FROM counters;";
        int rc = sqlite3_prepare_v2(m_pReadDbConn, pQuerySqlCmd, -1, &m_pGetSessionCounterIdsByNameStmt, nullptr);
        ret = (rc == SQLITE_OK);

        return ret;
    }

    bool PrepareCommonReadStatements()
    {
        bool ret = PrepareGetSessionInfoValueStatement()        &&
                   PrepareGetAllSessionInfoValueStatement();

        return ret;
    }

    bool PrepareTimelineReadStatements()
    {
        bool ret = PrepareGetSessionSamplingIntervalMsStatement()               &&
                   PrepareGetCounterIdByNameStatement()                         &&
                   PrepareGetSessionCountersByCategoryStatement()               &&
                   PrepareGetSessionCountersByCategoryIdStatement()             &&
                   PrepareGetAllSessionCountersStatement()                      &&
                   PrepareGetSessionCountersByDeviceAndCategoryStatement()      &&
                   PrepareGetSessionCountersByDeviceAndCategoryIdsStatement()   &&
                   PrepareGetDeviceTypeStatement()                              &&
                   PrepareGetDeviceTypeIdStatement()                            &&
                   PrepareGetDeviceTypeByCounterIdStatement()                   &&
                   PrepareGetDeviceTypeIdByCounterIdStatement()                 &&
                   PrepareGetSessionTimeRangeStatement()                        &&
                   PrepareGetSessionCounterNames();
        return ret;
    }

    bool PrepareAggregationReadStatements()
    {
        bool ret = true;

        // Create the required Views
        ret = ret && CreateProcessTotalsView();

        // Process/Thread/Module summary View
        ret = ret && CreateProcessSummaryView();

        // Function Summary View
        ret = ret && CreateFunctionSummaryView();

        return ret;
    }

    //
    //      !!! Control APIs !!!
    //

    bool CreateTables(const std::vector<std::string>& createStmts)
    {
        bool ret = false;

        for (const std::string& createStr : createStmts)
        {
            sqlite3_stmt* pStmt = nullptr;

            // Prepare the common table creation statements
            int rc = sqlite3_prepare_v2(m_pWriteDbConn, createStr.c_str(), createStr.size(), &pStmt, nullptr);

            if (SQLITE_OK == rc)
            {
                rc = sqlite3_step(pStmt);
                sqlite3_finalize(pStmt);
            }

            ret = (SQLITE_DONE == rc) ? true : false;

            if (!ret)
            {
                break;
            }
        }

        return ret;
    }

    bool CreateProfilingDatabase(const char* pDbName, gtUInt64 profileType)
    {
        m_profileType = static_cast<AMDTProfileMode>(profileType);

        bool ret = false;

        GT_IF_WITH_ASSERT(pDbName != nullptr)
        {
            // Open a DB connection.
            int rc = sqlite3_open_v2(pDbName, &m_pWriteDbConn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

            GT_IF_WITH_ASSERT(rc == SQLITE_OK)
            {
                ret = CreateTables(SQL_CREATE_DB_STMTS_COMMON);

                if (ret && ((profileType & AMDT_PROFILE_MODE_TIMELINE) == AMDT_PROFILE_MODE_TIMELINE))
                {
                    ret = CreateTables(SQL_CREATE_DB_STMTS_TIMELINE);

                    ret = ret && PrepareAllPwrProfWriteStatements();
                }

                if (ret && ((profileType & AMDT_PROFILE_MODE_AGGREGATION) == AMDT_PROFILE_MODE_AGGREGATION))
                {
                    ret = CreateTables(SQL_CREATE_DB_STMTS_AGGREGATION);

                    ret = ret && PrepareAllCpuProfWriteStatements();
                }

                if (ret)
                {
                    // Set ther user version - PRAGMA user_version=1
                    SetDbVersion();
                    m_isCreateDb = true;

                    // Dispatch the transaction COMMIT thread.
                    if (m_pDbTxCommitThread != nullptr)
                    {
                        // If there is already a transaction thread, terminate it.
                        m_pDbTxCommitThread->terminate();
                        delete m_pDbTxCommitThread;
                        m_pDbTxCommitThread = nullptr;
                    }

                    m_pDbTxCommitThread = new dbTxCommitThread(m_pParent);

                    if (!m_pDbTxCommitThread->execute())
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Could not dispatch DB COMMIT thread", OS_DEBUG_LOG_ERROR);
                    }
                }
            }
        }

        return ret;
    }

    bool OpenProfilingDatabase(const char* dbNameAsUtf8, gtUInt64 profileType, bool isReadOnly)
    {
        bool ret = false;
        int flags = (isReadOnly) ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
        m_canUpdateDB = !isReadOnly;

        if (sqlite3_open_v2(dbNameAsUtf8, &m_pReadDbConn, flags, nullptr) == SQLITE_OK)
        {
            m_profileType = static_cast<AMDTProfileMode>(profileType);

            // Get the dbvesion (PRAGMA user_version)
            GetDbVersion(m_dbVersion);

            // Prepare the read statements only if the DB is the latest version.
            if (AMDT_CURRENT_PROFILE_DB_VERSION == m_dbVersion)
            {
                // Turn off synchronous
                SetSynchronousOff();

                // Now the database is open. Let's prepare all the read statements.
                ret = PrepareCommonReadStatements();

                if (ret && ((profileType & AMDT_PROFILE_MODE_TIMELINE) == AMDT_PROFILE_MODE_TIMELINE))
                {
                    ret = PrepareTimelineReadStatements();
                }

                if (ret && ((m_profileType & AMDT_PROFILE_MODE_AGGREGATION) == AMDT_PROFILE_MODE_AGGREGATION))
                {
                    // create event-core sampling config view
                    ret = CreateSampledCounterCoreConfig();

                    ret = ret && CreateModuleInfoView();
                }

                m_isCurrentDbOpenForRead = ret;
            }
        }

        return ret;
    }

    bool MigrateProfilingDatabase(const char* dbNameAsUtf8)
    {
        bool ret = false;

        if (nullptr == m_pWriteDbConn)
        {
            if (sqlite3_open_v2(dbNameAsUtf8, &m_pWriteDbConn, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK)
            {
                ret = AlterTables();

                if (ret)
                {
                    ret = SetDbVersion();

                    GetDbVersion(m_dbVersion);

                    // Does this reqd ?
                    FlushData();
                }
            }
        }

        return ret;
    }

    bool FlushDataAsync()
    {
        bool ret = false;

        GT_IF_WITH_ASSERT(m_pDbTxCommitThread != nullptr)
        {
            ret = m_pDbTxCommitThread->TriggerDbTxCommit();
        }

        return ret;
    }

    bool SetSynchronousOff()
    {
        bool ret = true;

        if (m_canUpdateDB && (nullptr != m_pReadDbConn))
        {
            sqlite3_exec(m_pReadDbConn, SQL_CMD_SET_SYNCHRONOUS, nullptr, nullptr, nullptr);
        }

        return ret;
    }

    bool SetDbVersion()
    {
        bool ret = true;

        sqlite3_exec(m_pWriteDbConn, SQL_CMD_SET_USER_VERSION, nullptr, nullptr, nullptr);

        return ret;
    }

    bool GetDbVersion(int& dbVersion)
    {
        bool ret = false;
        sqlite3_stmt* pVersionStmt = nullptr;
        sqlite3* pDbConn = (m_pReadDbConn != nullptr) ? m_pReadDbConn : m_pWriteDbConn;

        if (nullptr != pDbConn)
        {
            int rc = sqlite3_prepare_v2(pDbConn, "PRAGMA user_version;", -1, &pVersionStmt, NULL);

            if ((SQLITE_OK == rc) && (sqlite3_step(pVersionStmt) == SQLITE_ROW))
            {
                dbVersion = sqlite3_column_int(pVersionStmt, 0);
                ret = true;
            }

            sqlite3_finalize(pVersionStmt);
            pVersionStmt = nullptr;
        }

        return ret;
    }

    // This will create the required views
    bool PrepareProfilingDatabase()
    {
        bool ret = false;

        if (    m_isCurrentDbOpenForRead
            && ((m_profileType & AMDT_PROFILE_MODE_AGGREGATION) == AMDT_PROFILE_MODE_AGGREGATION))
        {
            ret = PrepareAggregationReadStatements();
        }

        return ret;
    }

    //
    //  !!! Alter Tables !!!
    //

    // Note this happens while reporting?
    bool AlterTables()
    {
        bool ret = true;
        int version = -1;

        GetDbVersion(version);

        if ((nullptr != m_pWriteDbConn) && (0 == version))
        {
            for (const std::string& queryStr : SQL_ALTER_TABLE_STMTS_VERSION_1)
            {
                sqlite3_stmt* pStmt = nullptr;

                int rc = sqlite3_prepare_v2(m_pWriteDbConn, queryStr.c_str(), queryStr.size(), &pStmt, nullptr);

                if (SQLITE_OK == rc)
                {
                    rc = sqlite3_step(pStmt);

                    sqlite3_finalize(pStmt);
                }

                ret = ret && ((SQLITE_DONE == rc) ? true : false);

                // break for read only DB - SQLITE_READONLY
                if (!ret)
                {
                    if (SQLITE_READONLY == rc)
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Read Only Database", OS_DEBUG_LOG_ERROR);
                    }

                    break;
                }
            }
        }

        return ret;
    }

    //
    //  !!! Insert APIs !!!
    //

    bool InsertSessionInfoKeyValue(const gtString& keyStr, const gtString& valueStr)
    {
        bool ret = false;

        // Bind the values to the parameters.
        // TODO: utf8 ?
        std::string key(keyStr.asASCIICharArray());
        std::string value(valueStr.asASCIICharArray());

        sqlite3_bind_text(m_pSessionInfoInsertStmt, 1, key.c_str(), key.size(), nullptr);
        sqlite3_bind_text(m_pSessionInfoInsertStmt, 2, value.c_str(), value.size(), nullptr);

        // Execute the query.
        int rc = sqlite3_step(m_pSessionInfoInsertStmt);
        GT_IF_WITH_ASSERT(rc == SQLITE_DONE)
        {
            ret = true;
        }

        // Reset the statement so that it can be reused.
        sqlite3_reset(m_pSessionInfoInsertStmt);

        return ret;
    }

    bool InsertSessionInfo(const gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
    {
        bool ret = true;

        for (auto sessionInfo : sessionInfoKeyValueVec)
        {
            ret = InsertSessionInfoKeyValue(sessionInfo.first, sessionInfo.second);
        }

        return ret;
    }

    bool InsertSamples(const gtVector<PPSampleData>& samples)
    {
        bool ret = false;

        GT_IF_WITH_ASSERT(m_pWriteDbConn != nullptr)
        {
            if (m_isFirstInsert)
            {
                // Begin a transaction.
                sqlite3_exec(m_pWriteDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);

                // Turn off the first insert flag.
                m_isFirstInsert = false;
            }

            for (const auto& sample : samples)
            {
                // Bind the values to the parameters.
                sqlite3_bind_int(m_pValsInsertStmt, 1, sample.m_quantizedTime);
                sqlite3_bind_int(m_pValsInsertStmt, 2, sample.m_counterID);
                sqlite3_bind_double(m_pValsInsertStmt, 3, sample.m_sampleValue);

                // Execute the query.
                int rc = sqlite3_step(m_pValsInsertStmt);
                GT_ASSERT(rc == SQLITE_DONE);

                // Reset the statement so that it can be reused.
                sqlite3_reset(m_pValsInsertStmt);
            }

            ret = true;
        }

        return ret;
    }

    bool InsertSubDevices(int parentDeviceId, const gtVector<int>& subDevices)
    {
        bool isOk = false;

        for (const int subDeviceId : subDevices)
        {
            isOk = InsertSubDevice(parentDeviceId, subDeviceId);
            GT_ASSERT_EX(isOk, L"Inserting sub device to DB");
        }

        // Currently we will always return true (the assertion inside the loop should suffice).
        return true;
    }

    bool InsertSubDevice(int parentDeviceId, int subDeviceId)
    {
        bool ret = false;
        int rc = 0;

        GT_IF_WITH_ASSERT(m_pWriteDbConn != nullptr)
        {
            // Bind the values to the parameters.
            sqlite3_bind_int(m_pSubDevicesInsertStmt, 1, parentDeviceId);
            sqlite3_bind_int(m_pSubDevicesInsertStmt, 2, subDeviceId);

            // Execute the query.
            rc = sqlite3_step(m_pSubDevicesInsertStmt);
            GT_IF_WITH_ASSERT(rc == SQLITE_DONE)
            {
                ret = true;
            }

            // Reset the statement so that it can be reused.
            sqlite3_reset(m_pSubDevicesInsertStmt);
        }
        return ret;
    }

    bool InsertDevice(const AMDTProfileDevice& device)
    {
        bool ret = false;
        int rc = 0;

        GT_IF_WITH_ASSERT(m_pWriteDbConn != nullptr)
        {
            std::string deviceName(device.m_name.asASCIICharArray());
            std::string deviceDesc(device.m_description.asASCIICharArray());
            std::string deviceTypeStr(device.m_deviceTypeStr.asASCIICharArray());

            // Bind the values to the parameters.
            sqlite3_bind_int(m_pDecivceInsertStmt, 1, device.m_deviceId);
            sqlite3_bind_int(m_pDecivceInsertStmt, 2, device.m_deviceType);
            sqlite3_bind_text(m_pDecivceInsertStmt, 3, deviceTypeStr.c_str(), deviceTypeStr.size(), nullptr);
            sqlite3_bind_text(m_pDecivceInsertStmt, 4, deviceName.c_str(), deviceName.size(), nullptr);
            sqlite3_bind_text(m_pDecivceInsertStmt, 5, deviceDesc.c_str(), deviceDesc.size(), nullptr);

            // Execute the query.
            rc = sqlite3_step(m_pDecivceInsertStmt);
            GT_IF_WITH_ASSERT(rc == SQLITE_DONE)
            {
                // Now insert the device's sub-devices.
                gtVector<int> subDevices;

                for (const AMDTUInt32 subDeviceId : device.m_subDeviceIds)
                {
                    subDevices.push_back(subDeviceId);
                }

                InsertSubDevices(device.m_deviceId, subDevices);
            }

            // Reset the statement so that it can be reused.
            sqlite3_reset(m_pDecivceInsertStmt);
            ret = true;
        }

        return ret;
    }

    bool InsertCounter(const AMDTProfileCounterDesc& counterDescription)
    {
        bool ret = false;
        int rc = 0;

        GT_IF_WITH_ASSERT(m_pWriteDbConn != nullptr)
        {
            std::string counterName(counterDescription.m_name.asASCIICharArray());
            std::string counterDesc(counterDescription.m_description.asASCIICharArray());
            std::string typeStr(counterDescription.m_typeStr.asASCIICharArray());
            std::string categoryStr(counterDescription.m_categoryStr.asASCIICharArray());
            std::string unitStr(counterDescription.m_unitStr.asASCIICharArray());

            // Bind the values to the parameters.
            sqlite3_bind_int(m_pCountersInsertStmt, 1, counterDescription.m_id);
            sqlite3_bind_int(m_pCountersInsertStmt, 2, counterDescription.m_deviceId);
            sqlite3_bind_text(m_pCountersInsertStmt, 3, counterName.c_str(), counterName.size(), nullptr);
            sqlite3_bind_text(m_pCountersInsertStmt, 4, counterDesc.c_str(), counterDesc.size(), nullptr);
            sqlite3_bind_int(m_pCountersInsertStmt, 5, counterDescription.m_category);
            sqlite3_bind_text(m_pCountersInsertStmt, 6, categoryStr.c_str(), categoryStr.size(), nullptr);
            sqlite3_bind_int(m_pCountersInsertStmt, 7, counterDescription.m_type);
            sqlite3_bind_text(m_pCountersInsertStmt, 8, typeStr.c_str(), typeStr.size(), nullptr);
            sqlite3_bind_int(m_pCountersInsertStmt, 9, counterDescription.m_unit);
            sqlite3_bind_text(m_pCountersInsertStmt, 10, unitStr.c_str(), unitStr.size(), nullptr);

            // Execute the query.
            rc = sqlite3_step(m_pCountersInsertStmt);
            GT_IF_WITH_ASSERT(rc == SQLITE_DONE)
            {
                ret = true;
            }

            // Reset the statement so that it can be reused.
            sqlite3_reset(m_pCountersInsertStmt);
        }

        return ret;
    }

    bool InsertCounterControl(int counterId, int quantizedTime, std::string& actionStr)
    {
        bool ret = false;
        int rc = 0;

        GT_IF_WITH_ASSERT(m_pWriteDbConn != nullptr)
        {
            // Bind the values to the parameters.
            sqlite3_bind_int(m_pCounterEnabledAtInsertStmt, 1, counterId);
            sqlite3_bind_int(m_pCounterEnabledAtInsertStmt, 2, quantizedTime);
            sqlite3_bind_text(m_pCounterEnabledAtInsertStmt, 3, actionStr.c_str(), actionStr.size(), nullptr);

            // Execute the query.
            rc = sqlite3_step(m_pCounterEnabledAtInsertStmt);
            GT_IF_WITH_ASSERT(rc == SQLITE_DONE)
            {
                ret = true;
            }

            // Reset the statement so that it can be reused.
            sqlite3_reset(m_pCounterEnabledAtInsertStmt);

        }
        return ret;
    }

    bool InsertSamplingInterval(unsigned samplingIntervalMs, unsigned quantizedTime)
    {
        bool ret = false;
        int rc = 0;

        GT_IF_WITH_ASSERT(m_pWriteDbConn != nullptr)
        {
            // Bind the values to the parameters.
            sqlite3_bind_int(m_pSamplingIntervalInsertStmt, 1, quantizedTime);
            sqlite3_bind_int(m_pSamplingIntervalInsertStmt, 2, samplingIntervalMs);

            // Execute the query.
            rc = sqlite3_step(m_pSamplingIntervalInsertStmt);
            GT_IF_WITH_ASSERT(rc == SQLITE_DONE)
            {
                ret = true;
            }

            // Reset the statement so that it can be reused.
            sqlite3_reset(m_pSamplingIntervalInsertStmt);
        }
        return ret;
    }

    bool InsertCoreinfo(gtUInt32 coreId, gtUInt32 processor, gtUInt32 numaNode)
    {
        bool ret = false;

        sqlite3_bind_int(m_pCoreInfoInsertStmt, 1, coreId);
        sqlite3_bind_int(m_pCoreInfoInsertStmt, 2, processor);
        sqlite3_bind_int(m_pCoreInfoInsertStmt, 3, numaNode);

        if (SQLITE_DONE == sqlite3_step(m_pCoreInfoInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pCoreInfoInsertStmt);
        return ret;
    }

    bool InsertSamplingCounter(gtUInt32 eventId,
                               const std::string& nameAsUtf8Str,
                               const std::string& abbrevAsUtf8Str,
                               const std::string& descriptionAsUtf8Str)
    {
        bool ret = false;

        sqlite3_bind_int(m_pSamplingCounterInsertStmt, 1, eventId);
        sqlite3_bind_text(m_pSamplingCounterInsertStmt, 2, nameAsUtf8Str.c_str(), nameAsUtf8Str.size(), nullptr);
        sqlite3_bind_text(m_pSamplingCounterInsertStmt, 3, abbrevAsUtf8Str.c_str(), abbrevAsUtf8Str.size(), nullptr);
        sqlite3_bind_text(m_pSamplingCounterInsertStmt, 4, descriptionAsUtf8Str.c_str(), descriptionAsUtf8Str.size(), nullptr);

        if (SQLITE_DONE == sqlite3_step(m_pSamplingCounterInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pSamplingCounterInsertStmt);
        return ret;
    }

    bool InsertSamplingConfig(gtUInt32 id, gtUInt16 counterId, gtUInt64 samplingInterval, gtUInt16 unitMask, int isUserMode, int isOsMode, int edge)
    {
        bool ret = false;

        sqlite3_bind_int(m_pSamplingConfigInsertStmt, 1, id);
        sqlite3_bind_int(m_pSamplingConfigInsertStmt, 2, counterId);
        sqlite3_bind_int64(m_pSamplingConfigInsertStmt, 3, samplingInterval);
        sqlite3_bind_int(m_pSamplingConfigInsertStmt, 4, unitMask);
        sqlite3_bind_int(m_pSamplingConfigInsertStmt, 5, isUserMode);
        sqlite3_bind_int(m_pSamplingConfigInsertStmt, 6, isOsMode);
        sqlite3_bind_int(m_pSamplingConfigInsertStmt, 7, edge);

        if (SQLITE_DONE == sqlite3_step(m_pSamplingConfigInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pSamplingConfigInsertStmt);
        return ret;
    }

    bool InsertCoreSamplingConfig(gtUInt64 id, gtUInt16 coreId, gtUInt32 samplingConfigId)
    {
        bool ret = false;

        sqlite3_bind_int64(m_pCoreSamplingConfigInsertStmt, 1, id);
        sqlite3_bind_int(m_pCoreSamplingConfigInsertStmt, 2, coreId);
        sqlite3_bind_int(m_pCoreSamplingConfigInsertStmt, 3, samplingConfigId);

        if (SQLITE_DONE == sqlite3_step(m_pCoreSamplingConfigInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pCoreSamplingConfigInsertStmt);
        return ret;
    }

    bool InsertProcessInfo(gtUInt64 pid, const std::string& pathAsUtf8Str, int is32Bit, int hasCSS)
    {
        bool ret = false;

        sqlite3_bind_int64(m_pProcessInfoInsertStmt, 1, static_cast<sqlite3_int64>(pid));
        sqlite3_bind_text(m_pProcessInfoInsertStmt, 2, pathAsUtf8Str.c_str(), pathAsUtf8Str.size(), nullptr);
        sqlite3_bind_int(m_pProcessInfoInsertStmt, 3, is32Bit);
        sqlite3_bind_int(m_pProcessInfoInsertStmt, 4, hasCSS);

        if (SQLITE_DONE == sqlite3_step(m_pProcessInfoInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pProcessInfoInsertStmt);
        return ret;
    }

    bool InsertModuleInfo(gtUInt32 id, const std::string& pathAsUtf8Str, int isSystemModule, int is32Bit, gtUInt32 type, gtUInt32 size, int foundDebugInfo)
    {
        bool ret = false;

        sqlite3_bind_int(m_pModuleInfoInsertStmt, 1, id);
        sqlite3_bind_text(m_pModuleInfoInsertStmt, 2, pathAsUtf8Str.c_str(), pathAsUtf8Str.size(), nullptr);
        sqlite3_bind_int(m_pModuleInfoInsertStmt, 3, isSystemModule);
        sqlite3_bind_int(m_pModuleInfoInsertStmt, 4, is32Bit);
        sqlite3_bind_int(m_pModuleInfoInsertStmt, 5, type);
        sqlite3_bind_int(m_pModuleInfoInsertStmt, 6, size);
        sqlite3_bind_int(m_pModuleInfoInsertStmt, 7, foundDebugInfo);

        if (SQLITE_DONE == sqlite3_step(m_pModuleInfoInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pModuleInfoInsertStmt);
        return ret;
    }

    bool InsertProcessThreadInfo(gtUInt64 id, gtUInt64 pid, gtUInt64 threadId)
    {
        bool ret = false;

        sqlite3_bind_int64(m_pProcessThreadInsertStmt, 1, id);
        sqlite3_bind_int64(m_pProcessThreadInsertStmt, 2, pid);
        sqlite3_bind_int64(m_pProcessThreadInsertStmt, 3, threadId);

        if (SQLITE_DONE == sqlite3_step(m_pProcessThreadInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pProcessThreadInsertStmt);
        return ret;
    }

    bool InsertSamples(gtUInt64 ptId, gtUInt32 moduleInstanceId, gtUInt64 coreSamplingConfigId, gtUInt32 functionId, gtUInt64 offset, gtUInt64 count)
    {
        bool ret = false;

        if (m_pWriteDbConn != nullptr)
        {
            if (m_isFirstInsert)
            {
                // Begin a transaction.
                sqlite3_exec(m_pWriteDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);

                // Turn off the first insert flag.
                m_isFirstInsert = false;
            }

            // Try to update the row assuming it is already inserted.
            sqlite3_bind_int64(m_pSampleContextUpdateStmt, 1, count);
            sqlite3_bind_int64(m_pSampleContextUpdateStmt, 2, ptId);
            sqlite3_bind_int(m_pSampleContextUpdateStmt, 3, moduleInstanceId);
            sqlite3_bind_int64(m_pSampleContextUpdateStmt, 4, coreSamplingConfigId);
            sqlite3_bind_int(m_pSampleContextUpdateStmt, 5, functionId);
            sqlite3_bind_int64(m_pSampleContextUpdateStmt, 6, offset);

            if (SQLITE_DONE == sqlite3_step(m_pSampleContextUpdateStmt))
            {
                int changes = sqlite3_changes(m_pWriteDbConn);

                // Row update attempt failed, insert a new row.
                if (0 == changes)
                {
                    sqlite3_bind_int64(m_pSampleContextInsertStmt, 1, ptId);
                    sqlite3_bind_int(m_pSampleContextInsertStmt, 2, moduleInstanceId);
                    sqlite3_bind_int64(m_pSampleContextInsertStmt, 3, coreSamplingConfigId);
                    sqlite3_bind_int(m_pSampleContextInsertStmt, 4, functionId);
                    sqlite3_bind_int64(m_pSampleContextInsertStmt, 5, offset);
                    sqlite3_bind_int64(m_pSampleContextInsertStmt, 6, count);

                    if (SQLITE_DONE == sqlite3_step(m_pSampleContextInsertStmt))
                    {
                        ret = true;
                    }

                    sqlite3_reset(m_pSampleContextInsertStmt);
                }
                else // Row update was successful.
                {
                    ret = true;
                }
            }

            sqlite3_reset(m_pSampleContextUpdateStmt);
        }

        return ret;
    }

    bool InsertFunctionInfo(gtUInt32 functionId, gtUInt32 moduleId, const std::string& funcNameAsUtf8Str, gtUInt64 offset, gtUInt64 size)
    {
        bool ret = false;

        sqlite3_bind_int(m_pFunctionInfoInsertStmt, 1, functionId);
        sqlite3_bind_int(m_pFunctionInfoInsertStmt, 2, moduleId);
        sqlite3_bind_text(m_pFunctionInfoInsertStmt, 3, funcNameAsUtf8Str.c_str(), funcNameAsUtf8Str.size(), nullptr);
        sqlite3_bind_int64(m_pFunctionInfoInsertStmt, 4, offset);
        sqlite3_bind_int64(m_pFunctionInfoInsertStmt, 5, size);

        if (SQLITE_DONE == sqlite3_step(m_pFunctionInfoInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pFunctionInfoInsertStmt);
        return ret;
    }

    // This routine is called while report case
    //  - if the user has specified the debug file path while reporting
    //  - while processing css leaf/frame entires
    bool InsertFunctionInfo(const AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;

        if (m_canUpdateDB)
        {
            // Begin a transaction.
            sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);

            sqlite3_stmt* pStmt = nullptr;
            const char* pCsSqlCmd = "INSERT INTO Function(id, moduleId, name, startOffset, size) VALUES(?, ?, ?, ?, ?);";
            int rc = sqlite3_prepare_v2(m_pReadDbConn, pCsSqlCmd, -1, &pStmt, nullptr);

            sqlite3_bind_int(pStmt, 1, funcInfo.m_functionId);
            sqlite3_bind_int(pStmt, 2, funcInfo.m_moduleId);

            std::string funcNameAsUtf8Str;
            funcInfo.m_name.asUtf8(funcNameAsUtf8Str);

            sqlite3_bind_text(pStmt, 3, funcNameAsUtf8Str.c_str(), funcNameAsUtf8Str.size(), nullptr);
            sqlite3_bind_int64(pStmt, 4, funcInfo.m_startOffset);
            sqlite3_bind_int64(pStmt, 5, funcInfo.m_size);

            rc = sqlite3_step(pStmt);
            ret = (SQLITE_DONE == rc) ? true : false;

            sqlite3_reset(pStmt);

            // Commit the transaction.
            sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_COMMIT, nullptr, nullptr, nullptr);
        }

        return ret;
    }

    bool InsertModuleInstanceInfo(gtUInt32 modInstanceId, gtUInt32 moduleId, gtUInt64 pid, gtUInt64 loadAddr)
    {
        bool ret = false;

        sqlite3_bind_int(m_pModuleInstanceInsertStmt, 1, modInstanceId);
        sqlite3_bind_int64(m_pModuleInstanceInsertStmt, 2, pid);
        sqlite3_bind_int(m_pModuleInstanceInsertStmt, 3, moduleId);
        sqlite3_bind_int64(m_pModuleInstanceInsertStmt, 4, loadAddr);

        if (SQLITE_DONE == sqlite3_step(m_pModuleInstanceInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pModuleInstanceInsertStmt);
        return ret;
    }

    bool InsertJitInstanceInfo(gtUInt32 jitId, gtUInt64 functionId, gtUInt64 pid, gtUInt64 loadAddr, gtUInt32 size)
    {
        bool ret = false;

        sqlite3_bind_int(m_pJitInstanceInsertStmt, 1, jitId);
        sqlite3_bind_int64(m_pJitInstanceInsertStmt, 2, functionId);
        sqlite3_bind_int64(m_pJitInstanceInsertStmt, 3, pid);
        sqlite3_bind_int64(m_pJitInstanceInsertStmt, 4, loadAddr);
        sqlite3_bind_int(m_pJitInstanceInsertStmt, 5, size);

        if (SQLITE_DONE == sqlite3_step(m_pJitInstanceInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pJitInstanceInsertStmt);
        return ret;
    }

    bool InsertJitCodeBlobInfo(gtUInt32 id, const std::string& sourceFilePath, const std::string& jncFilePath)
    {
        bool ret = false;

        sqlite3_bind_int(m_pJitCodeBlobInsertStmt, 1, id);
        sqlite3_bind_text(m_pJitCodeBlobInsertStmt, 2, sourceFilePath.c_str(), sourceFilePath.size(), SQLITE_STATIC);
        //sqlite3_bind_blob(m_pJitCodeBlobInsertStmt, 3, pBlob, blobLength, SQLITE_STATIC);
        sqlite3_bind_text(m_pJitCodeBlobInsertStmt, 3, jncFilePath.c_str(), jncFilePath.size(), SQLITE_STATIC);

        if (SQLITE_DONE == sqlite3_step(m_pJitCodeBlobInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pJitCodeBlobInsertStmt);
        return ret;
    }

    bool InsertCallStackFrame(gtUInt32 callStackId, gtUInt64 processId, gtUInt64 funcId, gtUInt64 offset, gtUInt16 depth)
    {
        bool ret = false;

        sqlite3_bind_int(m_pCallStackFrameInsertStmt, 1, callStackId);
        sqlite3_bind_int64(m_pCallStackFrameInsertStmt, 2, processId);
        sqlite3_bind_int64(m_pCallStackFrameInsertStmt, 3, funcId);
        sqlite3_bind_int64(m_pCallStackFrameInsertStmt, 4, offset);
        sqlite3_bind_int(m_pCallStackFrameInsertStmt, 5, depth);

        if (SQLITE_DONE == sqlite3_step(m_pCallStackFrameInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pCallStackFrameInsertStmt);
        return ret;
    }

    bool InsertCallStackLeaf(gtUInt32 callStackId, gtUInt64 processId, gtUInt64 funcId, gtUInt64 offset, gtUInt32 counterId, gtUInt64 selfSamples)
    {
        bool ret = false;

        sqlite3_bind_int(m_pCallStackLeafInsertStmt, 1, callStackId);
        sqlite3_bind_int64(m_pCallStackLeafInsertStmt, 2, processId);
        sqlite3_bind_int64(m_pCallStackLeafInsertStmt, 3, funcId);
        sqlite3_bind_int64(m_pCallStackLeafInsertStmt, 4, offset);
        sqlite3_bind_int(m_pCallStackLeafInsertStmt, 5, counterId);
        sqlite3_bind_int64(m_pCallStackLeafInsertStmt, 6, selfSamples);

        if (SQLITE_DONE == sqlite3_step(m_pCallStackLeafInsertStmt))
        {
            ret = true;
        }

        sqlite3_reset(m_pCallStackLeafInsertStmt);
        return ret;
    }

    //
    //      !!! Update APIs !!!
    //

    bool UpdateTablesForVersion1(int id, const gtMap<gtString, int>& updateInfoMap)
    {
        sqlite3_stmt* pStmt = nullptr;
        const char* pUpdateStmt = SQL_UPDATE_TABLE_STMTS_VERSION_1[id].c_str();

        int rc = sqlite3_prepare_v2(m_pWriteDbConn, pUpdateStmt, -1, &pStmt, nullptr);
        bool ret = (SQLITE_OK == rc) ? true : false;

        for (const auto& info : updateInfoMap)
        {
            if (ret)
            {
                // Bind the values to the parameters.
                sqlite3_bind_int(pStmt, 1, info.second);

                std::string keyStr = info.first.asASCIICharArray();
                sqlite3_bind_text(pStmt, 2, keyStr.c_str(), keyStr.size(), nullptr);

                // Execute the query.
                rc = sqlite3_step(pStmt);

                // FIXME: if selection criteria does not match?
                ret = (SQLITE_DONE == rc) ? true : false;

                // Reset the statement so that it can be reused.
                sqlite3_reset(pStmt);
            }
        }

        if (nullptr != pStmt)
        {
            sqlite3_finalize(pStmt);
        }

        return ret;
    }

    bool UpdateIPSample(const AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;

        if (m_canUpdateDB)
        {
            sqlite3_stmt* pStmt = nullptr;
            const char* pUpdateStmt = "UPDATE SampleContext set functionId = ? where functionId = ? AND offset >= ? AND offset < ? ;";

            int rc = sqlite3_prepare_v2(m_pReadDbConn, pUpdateStmt, -1, &pStmt, nullptr);
            ret = (SQLITE_OK == rc) ? true : false;

            if (ret)
            {
                // Begin a transaction.
                //sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);

                AMDTFunctionId unknownFuncID = funcInfo.m_functionId & DB_MODULEID_MASK;

                sqlite3_bind_int(pStmt, 1, funcInfo.m_functionId);
                sqlite3_bind_int(pStmt, 2, unknownFuncID);
                sqlite3_bind_int64(pStmt, 3, funcInfo.m_startOffset);
                sqlite3_bind_int64(pStmt, 4, funcInfo.m_startOffset + funcInfo.m_size);

                rc = sqlite3_step(pStmt);
                ret = (SQLITE_DONE == rc) ? true : false;

                sqlite3_finalize(pStmt);

                // Commit the transaction.
                //sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_COMMIT, nullptr, nullptr, nullptr);
            }
        }

        return ret;
    }

    bool UpdateCallstackLeaf(const AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;

        if (m_canUpdateDB)
        {
            sqlite3_stmt* pStmt = nullptr;
            const char* pUpdateStmt = "UPDATE CallstackLeaf set functionId = ? where functionId = ? AND offset >= ? AND offset < ? ;";

            int rc = sqlite3_prepare_v2(m_pReadDbConn, pUpdateStmt, -1, &pStmt, nullptr);
            ret = (SQLITE_OK == rc) ? true : false;

            if (ret)
            {
                // Begin a transaction.
                //sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);

                AMDTFunctionId unknownFuncID = funcInfo.m_functionId & DB_MODULEID_MASK;

                sqlite3_bind_int(pStmt, 1, funcInfo.m_functionId);
                sqlite3_bind_int(pStmt, 2, unknownFuncID);
                sqlite3_bind_int64(pStmt, 3, funcInfo.m_startOffset);
                sqlite3_bind_int64(pStmt, 4, funcInfo.m_startOffset + funcInfo.m_size);

                rc = sqlite3_step(pStmt);
                ret = (SQLITE_DONE == rc) ? true : false;

                sqlite3_finalize(pStmt);

                // Commit the transaction.
                //sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_COMMIT, nullptr, nullptr, nullptr);
            }
        }

        return ret;
    }

    bool UpdateCallstackFrame(const AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;

        if (m_canUpdateDB)
        {
            sqlite3_stmt* pStmt = nullptr;
            const char* pUpdateStmt = "UPDATE CallstackFrame set functionId = ? where functionId = ? AND offset >= ? AND offset < ? ;";

            int rc = sqlite3_prepare_v2(m_pReadDbConn, pUpdateStmt, -1, &pStmt, nullptr);
            ret = (SQLITE_OK == rc) ? true : false;

            if (ret)
            {
                // Begin a transaction.
                //sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);

                AMDTFunctionId unknownFuncID = funcInfo.m_functionId & DB_MODULEID_MASK;

                sqlite3_bind_int(pStmt, 1, funcInfo.m_functionId);
                sqlite3_bind_int(pStmt, 2, unknownFuncID);
                sqlite3_bind_int64(pStmt, 3, funcInfo.m_startOffset);
                sqlite3_bind_int64(pStmt, 4, funcInfo.m_startOffset + funcInfo.m_size);

                rc = sqlite3_step(pStmt);
                ret = (SQLITE_DONE == rc) ? true : false;

                sqlite3_finalize(pStmt);

                // Commit the transaction.
                //sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_COMMIT, nullptr, nullptr, nullptr);
            }
        }

        return ret;
    }

    //
    //      !!! Query APIs !!!
    //

    bool GetSessionInfoValue(const gtString& key, gtString& infoValue)
    {
        bool ret = false;
        std::string keyStr = key.asASCIICharArray();

        // Bind the values to the parameters.
        sqlite3_bind_text(m_pGetSessionInfoValueStmt, 1, keyStr.c_str(), keyStr.size(), nullptr);

        // Execute the query.
        int rc = sqlite3_step(m_pGetSessionInfoValueStmt);

        if (rc == SQLITE_ROW)
        {
            std::string infoValueAsUtf8;
            const unsigned char* pValue = sqlite3_column_text(m_pGetSessionInfoValueStmt, 0);

            if (pValue != nullptr)
            {
                infoValueAsUtf8 = std::string((const char*)pValue);
                infoValue.fromUtf8String(infoValueAsUtf8);
                ret = true;
            }
        }

        // Reset the statement so that it can be reused.
        sqlite3_reset(m_pGetSessionInfoValueStmt);

        return ret;
    }

    bool GetAllSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
    {
        bool ret = false;

        // Execute the query.
        while (sqlite3_step(m_pGetAllSessionInfoStmt) == SQLITE_ROW)
        {
            //std::string infoValueAsUtf8;
            const unsigned char* pKey = sqlite3_column_text(m_pGetAllSessionInfoStmt, 0);
            const unsigned char* pValue = sqlite3_column_text(m_pGetAllSessionInfoStmt, 1);

            if (pValue != nullptr && pKey != nullptr)
            {
                gtString key;
                key.fromUtf8String((const char*)pKey);

                gtString value;
                value.fromUtf8String((const char*)pValue);

                sessionInfoKeyValueVec.push_back(std::make_pair(key, value));

                ret = true;
            }
        }

        // Reset the statement so that it can be reused.
        sqlite3_reset(m_pGetAllSessionInfoStmt);

        return ret;
    }

#if 0
    bool GetModuleIdByName(const std::string& moduleName, gtUInt64& moduleId)
    {
        bool ret = false;
        moduleId = 0;

        sqlite3_bind_text(m_pModuleIdQueryStmt, 1, moduleName.c_str(), moduleName.size(), nullptr);

        if (SQLITE_ROW == sqlite3_step(m_pModuleIdQueryStmt))
        {
            moduleId = sqlite3_column_int64(m_pModuleIdQueryStmt, 0);
            ret = true;
        }

        sqlite3_reset(m_pModuleIdQueryStmt);
        return ret;
    }
#endif

    bool GetSamplingConfigId(gtUInt16 eventId, gtUByte unitMask, bool bitOs, bool bitUsr, gtUInt64& samplingConfigId)
    {
        bool ret = false;
        samplingConfigId = 0;

        sqlite3_bind_int(m_pSamplingConfigIdQueryStmt, 1, eventId);
        sqlite3_bind_int(m_pSamplingConfigIdQueryStmt, 2, unitMask);
        sqlite3_bind_int(m_pSamplingConfigIdQueryStmt, 3, bitUsr ? 1 : 0);
        sqlite3_bind_int(m_pSamplingConfigIdQueryStmt, 4, bitOs ? 1 : 0);

        if (SQLITE_ROW == sqlite3_step(m_pSamplingConfigIdQueryStmt))
        {
            samplingConfigId = sqlite3_column_int64(m_pSamplingConfigIdQueryStmt, 0);
            ret = true;
        }

        sqlite3_reset(m_pSamplingConfigIdQueryStmt);
        return ret;
    }

    bool GetSamplingInterval(unsigned& samplingIntervalMs)
    {
        bool ret = false;
        samplingIntervalMs = 0;

        // Execute the query.
        int rc = sqlite3_step(m_pGetSessionSamplingIntervalStmt);

        if (SQLITE_ROW == rc)
        {
            samplingIntervalMs = sqlite3_column_int(m_pGetSessionSamplingIntervalStmt, 0);
            ret = true;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Could not get the sampling interval", OS_DEBUG_LOG_ERROR);
        }

        // Reset the statement so that it can be reused.
        sqlite3_reset(m_pGetSessionSamplingIntervalStmt);

        return ret;
    }

    bool GetCounterNames(gtMap<gtString, int>& counterNames)
    {
        bool ret = false;

        // Execute the query.
        while (sqlite3_step(m_pGetSessionCounterIdsByNameStmt) == SQLITE_ROW)
        {
            const unsigned char* pCounterName = sqlite3_column_text(m_pGetSessionCounterIdsByNameStmt, 0);
            int currCounterId = sqlite3_column_int(m_pGetSessionCounterIdsByNameStmt, 1);
            gtString currCounterName;

            if (pCounterName != nullptr)
            {
                std::string counterNameAsUtf8 = std::string((const char*)pCounterName);
                currCounterName.fromUtf8String(counterNameAsUtf8);

                counterNames[currCounterName] = currCounterId;
            }

            ret = true;
        }

        // Reset the statement so that it can be reused.
        sqlite3_reset(m_pGetSessionCounterIdsByNameStmt);

        return ret;
    }

    bool GetBucketizedSamplesByCounterId(unsigned int bucketWidth, const gtVector<int>& counterIds, gtVector<int>& dbCids, gtVector<double>& dbBucketBottoms, gtVector<int>& dbBucketCount)
    {
        const char* QUERY_SELECT_CLAUSE = "SELECT counterId, cast(sampledValue / ? as int) * ? AS bucket, COUNT(*) AS CNT FROM samples ";
        const char* QUERY_WHERE_CLAUSE_BEGIN = "WHERE counterId IN(";
        const char* QUERY_WHERE_CLAUSE_END = ") ";
        const char* QUERY_GROUP_BY_CLAUSE = "GROUP BY counterId, bucket ";
        const char* QUERY_ORDER_BY_CLAUSE = "ORDER BY bucket;";

        const size_t numOfCounterIds = counterIds.size();

        if (numOfCounterIds > 0)
        {
            // Dynamically generate the query.
            std::stringstream whereClause;
            whereClause << QUERY_SELECT_CLAUSE;
            whereClause << QUERY_WHERE_CLAUSE_BEGIN;
            const size_t lastItemIndex = numOfCounterIds - 1;

            for (size_t i = 0; i < numOfCounterIds; ++i)
            {
                whereClause << counterIds[i];

                if (i != lastItemIndex)
                {
                    whereClause << ", ";
                }
            }

            whereClause << QUERY_WHERE_CLAUSE_END;
            whereClause << QUERY_GROUP_BY_CLAUSE;
            whereClause << QUERY_ORDER_BY_CLAUSE;

            // Prepare the statement.
            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string wholeQuery(whereClause.str());
            int rc = sqlite3_prepare_v2(m_pReadDbConn, wholeQuery.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Bind the parameters.
                sqlite3_bind_int(pQueryStmt, 1, bucketWidth);
                sqlite3_bind_int(pQueryStmt, 2, bucketWidth);

                // Execute the query.
                while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
                {
                    dbCids.push_back(sqlite3_column_int(pQueryStmt, 0));
                    dbBucketBottoms.push_back(sqlite3_column_double(pQueryStmt, 1));
                    dbBucketCount.push_back(sqlite3_column_int(pQueryStmt, 2));
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);
            }
        }

        return true;
    }

    void FlushData()
    {
        if (m_pWriteDbConn != nullptr)
        {
            // Commit the transaction.
            sqlite3_exec(m_pWriteDbConn, SQL_CMD_TX_COMMIT, nullptr, nullptr, nullptr);

            // Begin a new transaction right away.
            sqlite3_exec(m_pWriteDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);
        }

        if (m_canUpdateDB)
        {
            // Commit the transaction.
            sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_COMMIT, nullptr, nullptr, nullptr);

            // Begin a new transaction right away.
            sqlite3_exec(m_pReadDbConn, SQL_CMD_TX_BEGIN, nullptr, nullptr, nullptr);
        }
    }

    bool GetSamplesTimeRange(SamplingTimeRange& samplingTimeRange)
    {
        bool ret = false;
        int rc = sqlite3_step(m_pGetSessionRangeStmt);
        GT_IF_WITH_ASSERT(rc == SQLITE_ROW)
        {
            int fromQuantizedTime = sqlite3_column_int(m_pGetSessionRangeStmt, 0);
            int toQuantizedTime = sqlite3_column_int(m_pGetSessionRangeStmt, 1);
            samplingTimeRange = SamplingTimeRange(fromQuantizedTime, toQuantizedTime);
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetSessionRangeStmt);

        return ret;
    }

    bool GetDeviceType(int deviceId, std::string& deviceType)
    {
        bool ret = false;

        // Bind the parameters.
        sqlite3_bind_int(m_pGetDeviceTypeStmt, 1, deviceId);

        // Execute the query.
        int rc = sqlite3_step(m_pGetDeviceTypeStmt);

        GT_IF_WITH_ASSERT(rc == SQLITE_ROW)
        {
            // Translate the string to the corresponding data type.
            const unsigned char* pDevType = sqlite3_column_text(m_pGetDeviceTypeStmt, 0);
            deviceType.assign(reinterpret_cast<const char*>(pDevType));
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetDeviceTypeStmt);

        return ret;
    }

    bool GetDeviceType(int deviceId, int& deviceTypeId)
    {
        bool ret = false;

        // Bind the parameters.
        sqlite3_bind_int(m_pGetDeviceTypeIdStmt, 1, deviceId);

        // Execute the query.
        int rc = sqlite3_step(m_pGetDeviceTypeIdStmt);

        if (SQLITE_ROW == rc)
        {
            deviceTypeId = sqlite3_column_int(m_pGetDeviceTypeIdStmt, 0);
            ret = true;
        }
        else
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Could not get the deviceType for device(%d), rc(%d)", deviceId, rc);
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetDeviceTypeIdStmt);

        return ret;
    }

    bool GetDeviceTypeByCounterId(int counterId, std::string& deviceType)
    {
        bool ret = false;

        // Bind the parameters.
        sqlite3_bind_int(m_pGetDeviceTypeByCounterIdStmt, 1, counterId);

        // Execute the query.
        int rc = sqlite3_step(m_pGetDeviceTypeByCounterIdStmt);

        GT_IF_WITH_ASSERT(rc == SQLITE_ROW)
        {
            // Translate the string to the corresponding data type.
            const unsigned char* pDevType = sqlite3_column_text(m_pGetDeviceTypeByCounterIdStmt, 0);
            deviceType.assign(reinterpret_cast<const char*>(pDevType));
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetDeviceTypeByCounterIdStmt);

        return ret;
    }

    bool GetDeviceTypeByCounterId(int counterId, int& deviceTypeId)
    {
        bool ret = false;

        // Bind the parameters.
        sqlite3_bind_int(m_pGetDeviceTypeIdByCounterIdStmt, 1, counterId);

        // Execute the query.
        int rc = sqlite3_step(m_pGetDeviceTypeIdByCounterIdStmt);

        GT_IF_WITH_ASSERT(rc == SQLITE_ROW)
        {
            deviceTypeId = sqlite3_column_int(m_pGetDeviceTypeIdByCounterIdStmt, 0);
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetDeviceTypeIdByCounterIdStmt);

        return ret;
    }

    // By Device and Category.
    bool GetCountersByDeviceAndCategory(const std::string& deviceTypeAsStr, const std::string& counterCategoryAsStr, gtVector<int>& counterIds)
    {
        bool ret = false;
        counterIds.clear();

        // Bind the parameters.
        sqlite3_bind_text(m_pGetSessionCountersByDeviceAndCategoryStmt, 1, counterCategoryAsStr.c_str(), counterCategoryAsStr.size(), nullptr);
        sqlite3_bind_text(m_pGetSessionCountersByDeviceAndCategoryStmt, 2, deviceTypeAsStr.c_str(), deviceTypeAsStr.size(), nullptr);

        // Execute the query.
        int cid = -1;

        while (sqlite3_step(m_pGetSessionCountersByDeviceAndCategoryStmt) == SQLITE_ROW)
        {
            cid = sqlite3_column_int(m_pGetSessionCountersByDeviceAndCategoryStmt, 0);
            counterIds.push_back(cid);
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetSessionCountersByDeviceAndCategoryStmt);

        return ret;
    }

    bool GetCountersByDeviceAndCategory(int deviceTypeId, int counterCategoryId, gtVector<int>& counterIds)
    {
        bool ret = false;
        counterIds.clear();

        // Bind the parameters.
        sqlite3_bind_int(m_pGetSessionCountersByDeviceAndCategoryIdsStmt, 1, counterCategoryId);
        sqlite3_bind_int(m_pGetSessionCountersByDeviceAndCategoryIdsStmt, 2, deviceTypeId);

        // Execute the query.
        int cid = -1;

        while (sqlite3_step(m_pGetSessionCountersByDeviceAndCategoryIdsStmt) == SQLITE_ROW)
        {
            cid = sqlite3_column_int(m_pGetSessionCountersByDeviceAndCategoryIdsStmt, 0);
            counterIds.push_back(cid);
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetSessionCountersByDeviceAndCategoryIdsStmt);

        return ret;
    }

    // Only by Category.
    bool GetCountersByCategory(const std::string& counterCategoryAsStr, gtVector<int>& counterIds)
    {
        bool ret = false;
        counterIds.clear();

        // Bind the parameters.
        sqlite3_bind_text(m_pGetSessionCountersByCategoryStmt, 1,
                          counterCategoryAsStr.c_str(), counterCategoryAsStr.size(), nullptr);

        // Execute the query.
        int cid = -1;

        while (sqlite3_step(m_pGetSessionCountersByCategoryStmt) == SQLITE_ROW)
        {
            cid = sqlite3_column_int(m_pGetSessionCountersByCategoryStmt, 0);
            counterIds.push_back(cid);
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetSessionCountersByCategoryStmt);

        return ret;
    }

    // Only by Category.
    bool GetCountersByCategory(int counterCategoryId, gtVector<int>& counterIds)
    {
        bool ret = false;
        counterIds.clear();

        // Bind the parameters.
        sqlite3_bind_int(m_pGetSessionCountersByCategoryIdStmt, 1, counterCategoryId);

        // Execute the query.
        int cid = -1;

        while (sqlite3_step(m_pGetSessionCountersByCategoryIdStmt) == SQLITE_ROW)
        {
            cid = sqlite3_column_int(m_pGetSessionCountersByCategoryIdStmt, 0);
            counterIds.push_back(cid);
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetSessionCountersByCategoryIdStmt);

        return ret;
    }

    // No filter (all counters).
    bool GetCountersDescription(gtMap<int, AMDTProfileCounterDesc*>& counterDetails)
    {
        bool ret = false;
        counterDetails.clear();

        // Execute the query.
        while (sqlite3_step(m_pGetAllSessionCountersStmt) == SQLITE_ROW)
        {
            // FIXME: dynamically memory allocated here (AMDTPwrCounterDesc, m_name, m_description)
            // which need to be deallocated by API caller.
            // OR a better approach would be to use std::string instead of char*
            AMDTProfileCounterDesc* pCurrCounterDesc = new AMDTProfileCounterDesc;

            // Counter id.
            pCurrCounterDesc->m_id = sqlite3_column_int(m_pGetAllSessionCountersStmt, 0);

            // Device id.
            pCurrCounterDesc->m_deviceId = sqlite3_column_int(m_pGetAllSessionCountersStmt, 1);

            // Counter name.
            const unsigned char* pCounterName = sqlite3_column_text(m_pGetAllSessionCountersStmt, 2);

            if (pCounterName != nullptr)
            {
                pCurrCounterDesc->m_name.fromUtf8String(reinterpret_cast<const char*>(pCounterName));
            }

            // Counter description.
            const unsigned char* pCounterDescription = sqlite3_column_text(m_pGetAllSessionCountersStmt, 3);

            if (pCounterDescription != nullptr)
            {
                pCurrCounterDesc->m_description.fromUtf8String(reinterpret_cast<const char*>(pCounterDescription));
            }

            // Counter category Id
            pCurrCounterDesc->m_category = sqlite3_column_int(m_pGetAllSessionCountersStmt, 4);

            // Counter category.
            const unsigned char* pCounterCategory = sqlite3_column_text(m_pGetAllSessionCountersStmt, 5);

            if (pCounterCategory != nullptr)
            {
                pCurrCounterDesc->m_categoryStr.fromUtf8String(reinterpret_cast<const char*>(pCounterCategory));
            }

            // Counter aggregation Id
            pCurrCounterDesc->m_type = sqlite3_column_int(m_pGetAllSessionCountersStmt, 6);

            // Counter aggregation.
            const unsigned char* pCounterAggregation = sqlite3_column_text(m_pGetAllSessionCountersStmt, 7);

            if (pCounterAggregation != nullptr)
            {
                pCurrCounterDesc->m_typeStr.fromUtf8String(reinterpret_cast<const char*>(pCounterAggregation));
            }

            // Counter units Id
            pCurrCounterDesc->m_unit = sqlite3_column_int(m_pGetAllSessionCountersStmt, 8);

            // Units.
            const unsigned char* pCounterUnit = sqlite3_column_text(m_pGetAllSessionCountersStmt, 9);

            if (pCounterUnit != nullptr)
            {
                pCurrCounterDesc->m_unitStr.fromUtf8String(reinterpret_cast<const char*>(pCounterUnit));
            }

            // Insert the structure to the map.
            counterDetails[pCurrCounterDesc->m_id] = pCurrCounterDesc;

            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetAllSessionCountersStmt);

        return ret;
    }

    bool GetCounterIdByName(const std::string& counterName, int& counterId)
    {
        bool ret = false;
        counterId = -1;

        sqlite3_bind_text(m_pGetCounterIdByNameStmt, 1, counterName.c_str(), counterName.size(), nullptr);

        // Execute the query.
        int rc = sqlite3_step(m_pGetCounterIdByNameStmt);

        if (rc == SQLITE_ROW)
        {
            counterId = sqlite3_column_int(m_pGetCounterIdByNameStmt, 0);
            ret = true;
        }

        // Reset the query so that it can be reused.
        sqlite3_reset(m_pGetCounterIdByNameStmt);

        return ret;
    }

    bool GetSamplesGroupByCounterId(const gtVector<int>& counterIds, gtMap<int, double>& consumptionPerCounterId)
    {
        bool ret = false;
        const char* QUERY_SELECT_CLAUSE = "SELECT counterId, SUM(sampledValue) FROM samples ";
        const char* QUERY_WHERE_CLAUSE_BEGIN = "WHERE (counterId IN(";
        const char* QUERY_WHERE_CLAUSE_END = "))";
        const char* QUERY_GROUP_BY_CLAUSE = " GROUP BY counterId;";

        const size_t numOfCounterIds = counterIds.size();
        GT_IF_WITH_ASSERT(numOfCounterIds > 0)
        {
            // Dynamically generate the query.
            std::stringstream whereClause;
            whereClause << QUERY_SELECT_CLAUSE;
            whereClause << QUERY_WHERE_CLAUSE_BEGIN;
            const size_t lastItemIndex = numOfCounterIds - 1;

            for (size_t i = 0; i < numOfCounterIds; ++i)
            {
                whereClause << counterIds[i];

                if (i != lastItemIndex)
                {
                    whereClause << ", ";
                }
            }

            whereClause << QUERY_WHERE_CLAUSE_END;
            whereClause << QUERY_GROUP_BY_CLAUSE;

            // Prepare the statement.
            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string wholeQuery(whereClause.str());
            int rc = sqlite3_prepare_v2(m_pReadDbConn, wholeQuery.c_str(), -1, &pQueryStmt, nullptr);
            GT_IF_WITH_ASSERT(rc == SQLITE_OK)
            {
                // Execute the query.
                while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
                {
                    int counterId = sqlite3_column_int(pQueryStmt, 0);
                    double cumulativeEnergy = sqlite3_column_double(pQueryStmt, 1);
                    consumptionPerCounterId.insert(std::pair<int, double>(counterId, cumulativeEnergy));
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);
                ret = true;
            }
        }
        return ret;
    }

    bool GetSampleCountByCounterId(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter)
    {
        bool ret = false;
        const char* QUERY_SELECT_CLAUSE = "SELECT counterId, COUNT(counterId) FROM samples ";
        const char* QUERY_WHERE_CLAUSE_BEGIN = "WHERE counterId IN(";
        const char* QUERY_WHERE_CLAUSE_END = ")";
        const char* QUERY_GROUP_BY_CLAUSE = " GROUP BY counterId;";
        const size_t numOfCounterIds = counterIds.size();

        GT_IF_WITH_ASSERT(numOfCounterIds > 0)
        {
            // Dynamically generate the query.
            std::stringstream whereClause;
            whereClause << QUERY_SELECT_CLAUSE;
            whereClause << QUERY_WHERE_CLAUSE_BEGIN;
            const size_t lastItemIndex = numOfCounterIds - 1;

            for (size_t i = 0; i < numOfCounterIds; ++i)
            {
                whereClause << counterIds[i];

                if (i != lastItemIndex)
                {
                    whereClause << ", ";
                }
            }

            whereClause << QUERY_WHERE_CLAUSE_END;
            whereClause << QUERY_GROUP_BY_CLAUSE;

            // Prepare the statement.
            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string wholeQuery(whereClause.str());
            int rc = sqlite3_prepare_v2(m_pReadDbConn, wholeQuery.c_str(), -1, &pQueryStmt, nullptr);
            GT_IF_WITH_ASSERT(rc == SQLITE_OK)
            {
                // Execute the query.
                while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
                {
                    int counterId = sqlite3_column_int(pQueryStmt, 0);
                    int numOfSamples = sqlite3_column_int(pQueryStmt, 1);
                    numberOfSamplesPerCounter.insert(std::pair<int, int>(counterId, numOfSamples));
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);
                ret = true;
            }
        }
        return ret;
    }

    bool GetSamplesByCounterIdAndRange(const gtVector<int>& counterIds, const SamplingTimeRange& samplingTimeRange,
                                       gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
    {
        bool ret = false;
        const char* QUERY_SELECT_CLAUSE = "SELECT quantizedTimeMs, counterId, sampledValue FROM samples ";
        const char* QUERY_WHERE_CLAUSE_BEGIN = "WHERE (quantizedTimeMs BETWEEN ? AND ?) AND counterId IN(";
        const char* QUERY_WHERE_CLAUSE_END = ") ORDER BY quantizedTimeMs;";
        const size_t numOfCounterIds = counterIds.size();

        GT_IF_WITH_ASSERT(numOfCounterIds > 0)
        {
            // Dynamically generate the query.
            std::stringstream whereClause;
            whereClause << QUERY_SELECT_CLAUSE;
            whereClause << QUERY_WHERE_CLAUSE_BEGIN;
            const size_t lastItemIndex = numOfCounterIds - 1;

            for (size_t i = 0; i < numOfCounterIds; ++i)
            {
                whereClause << counterIds[i];

                if (i != lastItemIndex)
                {
                    whereClause << ", ";
                }
            }

            whereClause << QUERY_WHERE_CLAUSE_END;

            // Prepare the statement.
            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string wholeQuery(whereClause.str());
            int rc = sqlite3_prepare_v2(m_pReadDbConn, wholeQuery.c_str(), -1, &pQueryStmt, nullptr);
            GT_IF_WITH_ASSERT(rc == SQLITE_OK)
            {
                // Bind the parameters.
                sqlite3_bind_int(pQueryStmt, 1, samplingTimeRange.m_fromTime);
                sqlite3_bind_int(pQueryStmt, 2, samplingTimeRange.m_toTime);

                int quantizedTime = 0;
                int counterId = 0;
                double sampleValue = 0.0;

                // Execute the query.
                while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
                {
                    quantizedTime = sqlite3_column_int(pQueryStmt, 0);
                    counterId = sqlite3_column_int(pQueryStmt, 1);
                    sampleValue = sqlite3_column_double(pQueryStmt, 2);
                    sampledValuesPerCounter[counterId].push_back(SampledValue(quantizedTime, sampleValue));
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);
                ret = true;
            }
        }
        return ret;
    }

    bool GetMinMaxSampleByCounterId(const gtVector<int>& counterIds, const SamplingTimeRange& samplingTimeRange,
                                    double& minValue, double& maxValue)
    {
        bool ret = false;
        const char* QUERY_SELECT_CLAUSE = "SELECT MIN(sampledValue), MAX(sampledValue) FROM samples ";
        const char* QUERY_WHERE_CLAUSE_BEGIN = "WHERE (quantizedTimeMs BETWEEN ? AND ?) AND counterId IN(";
        const char* QUERY_WHERE_CLAUSE_END = ");";
        const size_t numOfCounterIds = counterIds.size();

        GT_IF_WITH_ASSERT(numOfCounterIds > 0)
        {
            // Dynamically generate the query.
            std::stringstream whereClause;
            whereClause << QUERY_SELECT_CLAUSE;
            whereClause << QUERY_WHERE_CLAUSE_BEGIN;
            const size_t lastItemIndex = numOfCounterIds - 1;

            for (size_t i = 0; i < numOfCounterIds; ++i)
            {
                whereClause << counterIds[i];

                if (i != lastItemIndex)
                {
                    whereClause << ", ";
                }
            }

            whereClause << QUERY_WHERE_CLAUSE_END;

            // Prepare the statement.
            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string wholeQuery(whereClause.str());
            int rc = sqlite3_prepare_v2(m_pReadDbConn, wholeQuery.c_str(), -1, &pQueryStmt, nullptr);
            GT_IF_WITH_ASSERT(rc == SQLITE_OK)
            {
                // Bind the parameters.
                sqlite3_bind_int(pQueryStmt, 1, samplingTimeRange.m_fromTime);
                sqlite3_bind_int(pQueryStmt, 2, samplingTimeRange.m_toTime);

                // Execute the query.
                rc = sqlite3_step(pQueryStmt);
                GT_IF_WITH_ASSERT(rc == SQLITE_ROW)
                {
                    minValue = sqlite3_column_double(pQueryStmt, 0);
                    maxValue = sqlite3_column_double(pQueryStmt, 1);
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);
                ret = true;
            }
        }

        return ret;
    }

    bool GetCpuTopology(gtVector<AMDTCpuTopology>& cpuTopology)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        const char* rawQuery = "SELECT id, processorId, numaNodeId FROM Core;";

        int rc = sqlite3_prepare_v2(m_pReadDbConn, rawQuery, -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            AMDTUInt32  coreId;
            AMDTUInt32  processorId;
            AMDTUInt32  numaNodeId;

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                coreId = sqlite3_column_int(pQueryStmt, 0);
                processorId = sqlite3_column_int(pQueryStmt, 1);
                numaNodeId = sqlite3_column_int(pQueryStmt, 2);

                cpuTopology.emplace_back(coreId, static_cast<AMDTUInt16>(processorId), static_cast<AMDTUInt16>(numaNodeId));
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    // TODO: Use a single API for both PP and CP for counter table
    // Note: This is the actual PMC eventId
    bool GetCounterNameAndDescription(AMDTUInt32 counterId, std::string& nameStr, std::string& abbrevStr, std::string& descStr)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;

        const char* rawQuery = "SELECT name, abbrev, description    \
                                FROM SamplingCounter        \
                                WHERE id = ?;";

        int rc = sqlite3_prepare_v2(m_pReadDbConn, rawQuery, -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            sqlite3_bind_int(pQueryStmt, 1, counterId);

            const unsigned char* name = nullptr;
            const unsigned char* abbrev = nullptr;
            const unsigned char* desc = nullptr;

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                name = sqlite3_column_text(pQueryStmt, 0);
                abbrev = sqlite3_column_text(pQueryStmt, 1);
                desc = sqlite3_column_text(pQueryStmt, 2);

                nameStr.assign(reinterpret_cast<const char*>(name));
                abbrevStr.assign(reinterpret_cast<const char*>(abbrev));
                descStr.assign(reinterpret_cast<const char*>(desc));
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    // TODO: Use a single API for both PP and CP for counter table
    bool GetSampledCountersList(gtVector<AMDTProfileCounterDesc>& counterDescList)
    {
        //SELECT DISTINCT SampledCounterCoreConfig.samplingConfigurationId,
        //    SampledCounterCoreConfig.counterId,
        //    FROM SampledCounterCoreConfig;

        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;

        const char* rawQuery = "SELECT DISTINCT SampledCounterCoreConfig.samplingConfigurationId,   \
                                                SampledCounterCoreConfig.counterId                  \
                                FROM SampledCounterCoreConfig;";

        int rc = sqlite3_prepare_v2(m_pReadDbConn, rawQuery, -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTProfileCounterDesc counterDesc;
                std::string name;
                std::string abbrev;
                std::string desc;

                counterDesc.m_id = sqlite3_column_int(pQueryStmt, 0);
                counterDesc.m_hwEventId = sqlite3_column_int(pQueryStmt, 1);
                counterDesc.m_type = AMDT_PROFILE_COUNTER_TYPE_RAW;
                counterDesc.m_unit = AMDT_PROFILE_COUNTER_UNIT_COUNT;
                counterDesc.m_category = 0;
                counterDesc.m_deviceId = 0;

                if (GetCounterNameAndDescription(counterDesc.m_hwEventId, name, abbrev, desc))
                {
                    counterDesc.m_name.fromUtf8String(name.c_str());
                    counterDesc.m_abbrev.fromUtf8String(abbrev.c_str());
                    counterDesc.m_description.fromUtf8String(desc.c_str());
                }

                // TODO: Add Category, Type, Unit

                counterDescList.emplace_back(counterDesc);
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetSamplingConfiguration(AMDTUInt32 samplingConfigId, AMDTProfileSamplingConfig& samplingConfig)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;

        //const char* rawQuery = "SELECT samplingInterval, unitMask, isUserMode, isOsMode FROM SamplingConfiguration WHERE counterId = ?;";

        const char* rawQuery = "SELECT  SampledCounterCoreConfig.counterId,         \
                                        SampledCounterCoreConfig.samplingInterval,  \
                                        SampledCounterCoreConfig.unitMask,          \
                                        SampledCounterCoreConfig.isUserMode,        \
                                        SampledCounterCoreConfig.isOsMode           \
                                FROM SampledCounterCoreConfig                       \
                                WHERE SampledCounterCoreConfig.samplingConfigurationId = ?;";

        int rc = sqlite3_prepare_v2(m_pReadDbConn, rawQuery, -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Bind the parameters.
            sqlite3_bind_int(pQueryStmt, 1, samplingConfigId);

            // Execute the query.
            if (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt32 hwEventId = sqlite3_column_int(pQueryStmt, 0);
                AMDTUInt64 samplingInterval = sqlite3_column_int64(pQueryStmt, 1);
                AMDTInt32 unitMask = sqlite3_column_int(pQueryStmt, 2);
                AMDTInt32 userMode = sqlite3_column_int(pQueryStmt, 3);
                AMDTInt32 osMode = sqlite3_column_int(pQueryStmt, 4);

                samplingConfig.m_id               = samplingConfigId;
                samplingConfig.m_hwEventId        = hwEventId;
                samplingConfig.m_samplingInterval = samplingInterval;
                samplingConfig.m_unitMask         = static_cast<AMDTInt8>(unitMask);
                samplingConfig.m_userMode         = (userMode != 0) ? true : false;
                samplingConfig.m_osMode           = (osMode != 0) ? true : false;
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, gtVector<AMDTProfileModuleInfo>& moduleInfoList)
    {
        bool ret = false;
        AMDTProfileModuleInfo moduleInfo;

        ret = GetCachedModuleInfo(mid, moduleInfo);

        if (ret)
        {
            moduleInfoList.push_back(moduleInfo);
        }
        else
        {
            ret = GetModuleInfo__(pid, mid, moduleInfoList);

            if (ret && moduleInfoList.size() > 0)
            {
                CacheModuleInfo(mid, moduleInfoList[0]);
            }
            else
            {
                ret = GetModuleInfo__(mid, moduleInfo);

                if (ret)
                {
                    CacheModuleInfo(mid, moduleInfo);
                    moduleInfoList.push_back(moduleInfo);
                }
            }
        }

        return ret;
    }

    bool GetCachedModuleInfo(AMDTModuleId mid, AMDTProfileModuleInfo& moduleInfo)
    {
        bool ret = false;

        if (IS_MODULE_QUERY(mid))
        {
            auto mInfo = m_moduleIdInfoMap.find(mid);

            if (mInfo != m_moduleIdInfoMap.end())
            {
                moduleInfo = mInfo->second;
                ret = true;
            }
        }

        return ret;
    }

    bool CacheModuleInfo(AMDTModuleId mid, AMDTProfileModuleInfo& moduleInfo)
    {
        bool ret = false;

        if (IS_MODULE_QUERY(mid))
        {
            auto mInfo = m_moduleIdInfoMap.find(mid);

            if (mInfo == m_moduleIdInfoMap.end())
            {
                m_moduleIdInfoMap.insert({ mid, moduleInfo });
                ret = true;
            }
        }

        return ret;
    }

    bool GetModuleInfo__(AMDTUInt32 pid, AMDTModuleId mid, gtVector<AMDTProfileModuleInfo>& moduleInfoList)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream partialQuery;

        partialQuery << "SELECT id,                      \
                                path,                    \
                                loadAddress,             \
                                size,                    \
                                is32Bit,                 \
                                foundDebugInfo,          \
                                type,                    \
                                isSystemModule           \
                         FROM ModuleInfo ";

        if (IS_PROCESS_MODULE_QUERY(pid, mid))
        {
            partialQuery << " WHERE processId = ? AND id = ? ";
        }
        else if (IS_PROCESS_QUERY(pid))
        {
            partialQuery << " WHERE processId = ? ";
        }
        else if (IS_MODULE_QUERY(mid))
        {
            partialQuery << " WHERE id = ? ";
        }

        partialQuery << ";";
        const std::string& queryStr = partialQuery.str();

        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Bind the parameters.
            if (IS_PROCESS_MODULE_QUERY(pid, mid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
                sqlite3_bind_int(pQueryStmt, 2, mid);
            }
            else if (IS_PROCESS_QUERY(pid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
            }
            else if (IS_MODULE_QUERY(mid))
            {
                sqlite3_bind_int(pQueryStmt, 1, mid);
            }

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt64 moduleId = sqlite3_column_int64(pQueryStmt, 0);
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 1);
                AMDTUInt64 loadAddress = sqlite3_column_int64(pQueryStmt, 2);
                AMDTUInt32 size = sqlite3_column_int(pQueryStmt, 3);
                AMDTUInt32 is32Bit = sqlite3_column_int(pQueryStmt, 4);
                AMDTUInt32 foundDebugInfo = sqlite3_column_int(pQueryStmt, 5);
                AMDTUInt32 type = sqlite3_column_int(pQueryStmt, 6);
                AMDTUInt32 systemModule = sqlite3_column_int(pQueryStmt, 7);

                AMDTProfileModuleInfo moduleInfo;
                moduleInfo.m_moduleId = static_cast<AMDTUInt32>(moduleId);
                moduleInfo.m_path.fromUtf8String(reinterpret_cast<const char*>(path));

                // Set the module name
                GetNameFromPath(moduleInfo.m_path, moduleInfo.m_name);

                moduleInfo.m_loadAddress = loadAddress;
                moduleInfo.m_size = size;
                moduleInfo.m_is64Bit = (is32Bit == 0) ? true : false;
                moduleInfo.m_foundDebugInfo = (foundDebugInfo == 0) ? false : true;
                moduleInfo.m_type = static_cast<AMDTModuleType>(type);
                moduleInfo.m_isSystemModule = (systemModule == 0) ? false : true;

                moduleInfoList.emplace_back(moduleInfo);
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    // For some weird reasons we only have an entry in Module table but there is no
    // corresponding entry in ModuleInstance table.. to handle those scenarios...
    bool GetModuleInfo__(AMDTModuleId mid, AMDTProfileModuleInfo& moduleInfo)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream partialQuery;

        partialQuery << "SELECT DISTINCT Module.id,                      \
                                         Module.path,                    \
                                         Module.size,                    \
                                         Module.is32Bit,                 \
                                         Module.foundDebugInfo,          \
                                         Module.type,                    \
                                         Module.isSystemModule           \
                         FROM Module                                     \
                         WHERE id = ? ";

        const std::string& queryStr = partialQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            sqlite3_bind_int(pQueryStmt, 1, mid);

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                moduleInfo.m_moduleId = sqlite3_column_int(pQueryStmt, 0);
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 1);
                moduleInfo.m_size = sqlite3_column_int(pQueryStmt, 2);
                AMDTUInt32 is32Bit = sqlite3_column_int(pQueryStmt, 3);
                AMDTUInt32 foundDebugInfo = sqlite3_column_int(pQueryStmt, 4);
                AMDTUInt32 type = sqlite3_column_int(pQueryStmt, 5);
                AMDTUInt32 systemModule = sqlite3_column_int(pQueryStmt, 6);

                moduleInfo.m_path.fromUtf8String(reinterpret_cast<const char*>(path));

                // Set the module name
                GetNameFromPath(moduleInfo.m_path, moduleInfo.m_name);

                // loadAddress is valid only if the PID is specified.
                moduleInfo.m_loadAddress = 0; // AMDT_PROFILE_INVALID_ADDR;
                moduleInfo.m_is64Bit = (is32Bit == 0) ? true : false;
                moduleInfo.m_foundDebugInfo = (foundDebugInfo == 0) ? false : true;
                moduleInfo.m_type = static_cast<AMDTModuleType>(type);
                moduleInfo.m_isSystemModule = (systemModule == 0) ? false : true;
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetThreadInfo(AMDTUInt32 pid, gtUInt32 tid, gtVector<AMDTProfileThreadInfo>& threadInfoList)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream partialQuery;

        partialQuery << "SELECT threadId, processId FROM ProcessThread ";

        if (IS_PROCESS_THREAD_QUERY(pid, tid))
        {
            partialQuery << " WHERE processId = ? AND threadId = ?";
        }
        else if (IS_PROCESS_QUERY(pid))
        {
            partialQuery << " WHERE processId = ?";
        }
        else if (IS_THREAD_QUERY(tid))
        {
            partialQuery << " WHERE threadId = ?";
        }

        partialQuery << ";";
        const std::string& queryStr = partialQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Bind the parameters.
            if (IS_PROCESS_THREAD_QUERY(pid, tid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
                sqlite3_bind_int(pQueryStmt, 2, tid);
            }
            else if (IS_PROCESS_QUERY(pid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
            }
            else if (IS_THREAD_QUERY(tid))
            {
                sqlite3_bind_int(pQueryStmt, 1, tid);
            }

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt32 threadId = sqlite3_column_int(pQueryStmt, 0);
                AMDTUInt32 processId = sqlite3_column_int(pQueryStmt, 1);

                AMDTProfileThreadInfo threadInfo;
                threadInfo.m_threadId = threadId;
                threadInfo.m_pid = processId;
                threadInfoList.emplace_back(threadInfo);
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetNameFromPath(gtString& pathStr, gtString& nameStr)
    {
        if (!pathStr.isEmpty())
        {
            int startPos = pathStr.findLastOf(osFilePath::osPathSeparator);

            if (-1 != startPos)
            {
                pathStr.getSubString(startPos + 1, pathStr.length(), nameStr);
            }
            else
            {
                nameStr = pathStr;
            }
        }

        return true;
    }

    bool GetProcessInfo(AMDTUInt32 pid, gtVector<AMDTProfileProcessInfo>& processInfoList)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream partialQuery;

        partialQuery << "SELECT id, name, is32Bit FROM Process";

        if (IS_PROCESS_QUERY(pid))
        {
            // fetch process info for given pid
            partialQuery << " WHERE id = ?;";
        }
        else
        {
            partialQuery << ";";
        }

        const std::string& queryStr = partialQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Bind the parameters.
            if (IS_PROCESS_QUERY(pid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
            }

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt32 processId = sqlite3_column_int(pQueryStmt, 0);
                const unsigned char* pName = sqlite3_column_text(pQueryStmt, 1);
                int is32Bit = sqlite3_column_int(pQueryStmt, 2);

                AMDTProfileProcessInfo processInfo;
                processInfo.m_pid = processId;
                processInfo.m_path.fromUtf8String(reinterpret_cast<const char*>(pName));

                // Set the process name
                GetNameFromPath(processInfo.m_path, processInfo.m_name);

                processInfo.m_is64Bit = (is32Bit == 0) ? true : false;

                GetModuleInfo(processInfo.m_pid, AMDT_PROFILE_ALL_MODULES, processInfo.m_modulesList);
                GetThreadInfo(processInfo.m_pid, AMDT_PROFILE_ALL_THREADS, processInfo.m_threadsList);

                processInfoList.emplace_back(processInfo);
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetProcessThreadIds(AMDTProcessId pid, AMDTThreadId tid, gtVector<AMDTUInt32>& procesThreadIds)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream partialQuery;

        partialQuery << "SELECT id FROM ProcessThread";

        if (IS_PROCESS_THREAD_QUERY(pid, tid))
        {
            partialQuery << " WHERE processId = ? AND threadId = ?";
        }
        else if (IS_PROCESS_QUERY(pid))
        {
            partialQuery << " WHERE processId = ?";
        }
        else if (IS_THREAD_QUERY(tid))
        {
            partialQuery << " WHERE threadId = ?";
        }

        partialQuery << ";";
        const std::string& queryStr = partialQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Bind the parameters.
            if (IS_PROCESS_THREAD_QUERY(pid, tid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
                sqlite3_bind_int(pQueryStmt, 2, tid);
            }
            else if (IS_PROCESS_QUERY(pid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
            }
            else if (IS_THREAD_QUERY(tid))
            {
                sqlite3_bind_int(pQueryStmt, 1, tid);
            }

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt64 id = sqlite3_column_int64(pQueryStmt, 0);
                procesThreadIds.emplace_back(static_cast<AMDTUInt32>(id));
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    // return the list of process-ids that has call-stack data
    bool GetProcessesWithCallstackSamples(gtVector<AMDTProcessId>& cssProcessVec)
    {
        bool ret = false;

        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream query;

        query << "SELECT id FROM Process WHERE hasCSS = 1;";

        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt32 processId = sqlite3_column_int(pQueryStmt, 0);

                cssProcessVec.emplace_back(processId);
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetModuleInstanceIds(AMDTProcessId pid, AMDTModuleId mid, gtVector<AMDTUInt32>& moduleInstanceIds)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;
        std::stringstream partialQuery;

        partialQuery << "SELECT id FROM ModuleInstance";

        if (IS_PROCESS_MODULE_QUERY(pid, mid))
        {
            partialQuery << " WHERE processId = ? AND moduleId = ?";
        }
        else if (IS_PROCESS_QUERY(pid))
        {
            partialQuery << " WHERE processId = ?";
        }
        else if (IS_MODULE_QUERY(mid))
        {
            partialQuery << " WHERE moduleId = ?";
        }

        partialQuery << ";";
        const std::string& queryStr = partialQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            // Bind the parameters.
            if (IS_PROCESS_MODULE_QUERY(pid, mid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
                sqlite3_bind_int(pQueryStmt, 2, mid);
            }
            else if (IS_PROCESS_QUERY(pid))
            {
                sqlite3_bind_int(pQueryStmt, 1, pid);
            }
            else if (IS_MODULE_QUERY(mid))
            {
                sqlite3_bind_int(pQueryStmt, 1, mid);
            }

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt64 id = sqlite3_column_int64(pQueryStmt, 0);
                moduleInstanceIds.emplace_back(static_cast<AMDTUInt32>(id));
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    bool GetCoreSamplingConfigIds(AMDTUInt32 counterId, AMDTUInt64 coreMask, gtVector<AMDTUInt64>& coreSamplingConfigIds)
    {
        bool ret = false;
        sqlite3_stmt* pQueryStmt = nullptr;

        gtVector<AMDTUInt32> coreIds;

        if (IS_CORE_QUERY(coreMask))
        {
            int i = 0;

            while (coreMask)
            {
                if (coreMask & 1)
                {
                    coreIds.push_back(i);
                }

                ++i;
                coreMask >>= 1;
            }
        }

        std::stringstream partialQuery;

        partialQuery << "SELECT SampledCounterCoreConfig.id     \
                         FROM SampledCounterCoreConfig";

        if (IS_COUNTER_CORE_QUERY(counterId, coreMask))
        {
            //partialQuery << " WHERE counterId = ? AND coreId IN (";
            partialQuery << " WHERE samplingConfigurationId = ? AND coreId IN (";

            for (size_t i = 0; i < coreIds.size(); ++i)
            {
                partialQuery << coreIds[i];

                if (i != (coreIds.size() - 1))
                {
                    partialQuery << ",";
                }
            }

            partialQuery << ");";
        }
        else if (IS_COUNTER_QUERY(counterId))
        {
            partialQuery << " WHERE samplingConfigurationId = ? ";
        }
        else if (IS_CORE_QUERY(coreMask))
        {
            partialQuery << " WHERE coreId IN (";

            for (size_t i = 0; i < coreIds.size(); ++i)
            {
                partialQuery << coreIds[i];

                if (i != (coreIds.size() - 1))
                {
                    partialQuery << ",";
                }
            }

            partialQuery << ")";
        }

        partialQuery << ";";
        const std::string& queryStr = partialQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            // Bind the parameters.
            if (IS_COUNTER_QUERY(counterId))
            {
                sqlite3_bind_int(pQueryStmt, 1, counterId);
            }

            // Execute the query.
            while (sqlite3_step(pQueryStmt) == SQLITE_ROW)
            {
                AMDTUInt64 id = sqlite3_column_int64(pQueryStmt, 0);
                coreSamplingConfigIds.emplace_back(id);
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
            ret = true;
        }

        return ret;
    }

    /*
    drop view sampledCounterCoreConfig;
    create view sampledCounterConfig as
    SELECT CoreSamplingConfiguration.id,
           CoreSamplingConfiguration.coreId,
           SamplingConfiguration.counterId,
           SamplingConfiguration.samplingInterval,
           SamplingConfiguration.unitMask,
           SamplingConfiguration.isUserMode,
           SamplingConfiguration.isOsMode
    FROM CoreSamplingConfiguration
    INNER JOIN SamplingConfiguration ON samplingConfigurationId = SamplingConfiguration.id;

    drop view sampleProcessSummaryData;
    create view sampleProcessSummaryData as
    SELECT  ProcessThread.processId,
            ProcessThread.threadId,
                 ModuleInstance.moduleId as moduleId,
            SampleContext.coreSamplingConfigurationId,
                 sum(count) as count
         FROM SampleContext
         INNER JOIN ProcessThread  ON processThreadId = ProcessThread.id
         INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id
        group by moduleId, SampleContext.coreSamplingConfigurationId

    // Process/Module/Thread overviw, process/modules view (for ALL RAW profile data)
    drop view sampleProcessSummaryAllData;
    create view sampleProcessSummaryAllData as
    SELECT sampleProcessSummaryData.processId,
    sampleProcessSummaryData.threadId,
    sampleProcessSummaryData.moduleId,
    MAX(CASE WHEN CoreSamplingConfiguration.id = 1 THEN sampleProcessSummaryData.count END) AS 'e1',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 2 THEN sampleProcessSummaryData.count END) AS 'e2',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 3 THEN sampleProcessSummaryData.count END) AS 'e3',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 4 THEN sampleProcessSummaryData.count END) AS 'e4',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 5 THEN sampleProcessSummaryData.count END) AS 'e5',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 6 THEN sampleProcessSummaryData.count END) AS 'e6',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 7 THEN sampleProcessSummaryData.count END) AS 'e7',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 8 THEN sampleProcessSummaryData.count END) AS 'e8'
    FROM sampleProcessSummaryData
    INNER JOIN CoreSamplingConfiguration ON sampleProcessSummaryData.coreSamplingConfigurationId = CoreSamplingConfiguration.id
    group by moduleId;

    // Overview - Modules
    select moduleId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    from sampleProcessSummaryAllData group by moduleId order by total desc

    // Overview - Processes
    select processId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    from sampleProcessSummaryAllData group by processId order by total desc

    //  Process/Modules VIEW
    // Overview - Modules
    select moduleId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    (sum(ifnull(e2,0)) + sum(ifnull(e4,0)) + sum(ifnull(e6,0)) + sum(ifnull(e8,0))) as total2 from sampleProcessSummaryAllData
    group by moduleId order by total desc, total2 desc

    // Processes
    select processId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    (sum(ifnull(e2,0)) + sum(ifnull(e4,0)) + sum(ifnull(e6,0)) + sum(ifnull(e8,0))) as total2 from sampleProcessSummaryAllData
    group by processId order by total desc, total2 desc

    // Get the modules for the given process - TODO

    // Get the processes for the given module - TODO

    // Function Overview
    drop view sampleFunctionSummaryData;
    create view sampleFunctionSummaryData as
    SELECT  ProcessThread.processId,
    ProcessThread.threadId,
    ModuleInstance.moduleId as moduleId,
    SampleContext.functionId,
    SampleContext.coreSamplingConfigurationId,
    sum(count) as totalSamples
    FROM SampleContext
    INNER JOIN ProcessThread  ON processThreadId = ProcessThread.id
    INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id
    group by functionId, SampleContext.coreSamplingConfigurationId

    drop view sampleFunctionSummaryAllData;
    create view sampleFunctionSummaryAllData as
    SELECT sampleFunctionSummaryData.processId,
    sampleFunctionSummaryData.threadId,
    sampleFunctionSummaryData.moduleId,
    sampleFunctionSummaryData.functionId,
    MAX(CASE WHEN CoreSamplingConfiguration.id = 1 THEN sampleFunctionSummaryData.totalSamples END) AS 'e1',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 2 THEN sampleFunctionSummaryData.totalSamples END) AS 'e2',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 3 THEN sampleFunctionSummaryData.totalSamples END) AS 'e3',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 4 THEN sampleFunctionSummaryData.totalSamples END) AS 'e4',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 5 THEN sampleFunctionSummaryData.totalSamples END) AS 'e5',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 6 THEN sampleFunctionSummaryData.totalSamples END) AS 'e6',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 7 THEN sampleFunctionSummaryData.totalSamples END) AS 'e7',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 8 THEN sampleFunctionSummaryData.totalSamples END) AS 'e8'
    FROM sampleFunctionSummaryData
    INNER JOIN CoreSamplingConfiguration ON sampleFunctionSummaryData.coreSamplingConfigurationId = CoreSamplingConfiguration.id
    group by functionId;

    // Function Overview
    select processId, moduleId, functionId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    from sampleFunctionSummaryAllData  group by functionId order by total desc;

    // function view - add "where processId / threadId / moduleId = ?"
    select processId, threadId moduleId, functionId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    (sum(ifnull(e2,0)) + sum(ifnull(e4,0)) + sum(ifnull(e6,0)) + sum(ifnull(e8,0))) as total2 from sampleFunctionSummaryAllData
    group by functionId order by total desc, total2 desc;

    select processId, moduleId, functionId, (sum(ifnull(e1,0)) + sum(ifnull(e3,0)) + sum(ifnull(e5,0)) + sum(ifnull(e7,0))) as total,
    (sum(ifnull(e2,0)) + sum(ifnull(e4,0)) + sum(ifnull(e6,0)) + sum(ifnull(e8,0))) as total2 from sampleFunctionSummaryAllData
    where moduleId = 1
    group by functionId order by total desc, total2 desc;

    // Detailed function view - ADD "where processId / threadId  = ?"
    drop view functionDetailedData;
    create view functionDetailedData as
    select
    ProcessThread.processId,
    ProcessThread.threadId,
    ModuleInstance.moduleId,
    SampleContext.functionId,
    SampleContext.offset,
    SampleContext.coreSamplingConfigurationId,
    SampleContext.count
    from SampleContext
    INNER JOIN ProcessThread ON processThreadId = ProcessThread.id
    INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id
    where SampleContext.functionId = 1
    group by offset;

    select
    functionDetailedData.processId,
    functionDetailedData.threadId,
    functionDetailedData.moduleId,
    functionDetailedData.functionId,
    functionDetailedData.offset,
    MAX(CASE WHEN CoreSamplingConfiguration.id = 1 THEN functionDetailedData.count END) AS 'e1',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 2 THEN functionDetailedData.count END) AS 'e2',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 3 THEN functionDetailedData.count END) AS 'e3',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 4 THEN functionDetailedData.count END) AS 'e4',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 5 THEN functionDetailedData.count END) AS 'e5',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 6 THEN functionDetailedData.count END) AS 'e6',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 7 THEN functionDetailedData.count END) AS 'e7',
    MAX(CASE WHEN CoreSamplingConfiguration.id = 8 THEN functionDetailedData.count END) AS 'e8'
    from functionDetailedData
    INNER JOIN CoreSamplingConfiguration ON functionDetailedData.coreSamplingConfigurationId = CoreSamplingConfiguration.id
    where functionDetailedData.functionId = 1 // fucntion-id given by the user
    group by offset;
    */

    // This is used while creating VIEWs
    bool GetAllEventCorePartialQuery(std::stringstream& partialQuery, std::string& fromCol, std::string& toCol)
    {
        gtVector<AMDTUInt64> coreSamplingConfigIds;

        GetCoreSamplingConfigIds(AMDT_PROFILE_ALL_COUNTERS, AMDT_PROFILE_ALL_CORES, coreSamplingConfigIds);
        bool addComma = false;

        for (auto eventId : coreSamplingConfigIds)
        {
            if (addComma)
            {
                partialQuery << ", ";
            }

            gtString query;
            query.appendFormattedString(L" MAX(CASE WHEN " STR_FORMAT L" = " LONG_FORMAT L" THEN " STR_FORMAT L" END) AS event" LONG_FORMAT, fromCol.c_str() , eventId, toCol.c_str(), eventId);

            partialQuery << query.asASCIICharArray();
            addComma = true;
        }

        return addComma;
    }

    // This helper function is used while constructing the query to generate process/thread/module summary data
    // This is seperate By Core
    bool GetEventSeparateByCorePartialQuery(AMDTUInt32 counterId, AMDTUInt64 coreMask, std::stringstream& partialQuery, std::string& firstCountColName)
    {
        gtVector<AMDTUInt64> coreSamplingConfigIds;

        GetCoreSamplingConfigIds(counterId, coreMask, coreSamplingConfigIds);

        bool addComma = false;
        gtString query;
        std::string comma(" , ");

        for (auto id : coreSamplingConfigIds)
        {
            if (addComma)
            {
                query << comma.c_str();
            }

            query.appendFormattedString(L"SUM(IFNULL(event" LONG_FORMAT L", 0)) AS ", id);

            gtString countColName;
            countColName.appendFormattedString(L"eventTotal" LONG_FORMAT L" ", id);

            if (firstCountColName.empty())
            {
                firstCountColName = countColName.asASCIICharArray();
            }

            query << countColName;

            addComma = true;
        }

        if (addComma)
        {
            partialQuery << query.asASCIICharArray();
        }

        return addComma;
    } // GetEventSeparateByCorePartialQuery

    // This helper function is used while constructing the query to generate process/thread/module summary data
    // This is called when seperateByCore is false
    bool GetEventAllCorePartialQuery(AMDTUInt32 counterId, AMDTUInt64 coreMask, std::stringstream& partialQuery,
                                     std::string& firstCountColName,
                                     AMDTSampleValue& sampleInfo)
    {
        gtVector<AMDTUInt64> coreSamplingConfigIds;
        gtString query;

        GetCoreSamplingConfigIds(counterId, coreMask, coreSamplingConfigIds);

        bool addAddStr = false;

        query << L" (";
        std::string addString(" + ");

        for (auto id : coreSamplingConfigIds)
        {
            if (addAddStr)
            {
                query << addString.c_str();
            }

            query.appendFormattedString(L"SUM(IFNULL(event" LONG_FORMAT L", 0)) ", id);

            addAddStr = true;
        }

        if (addAddStr)
        {
            gtString countColName;
            countColName.appendFormattedString(L"eventTotal%d ", counterId);

            if (firstCountColName.empty())
            {
                firstCountColName = countColName.asASCIICharArray();
            }

            query << L" ) AS " << countColName << L" ";

            sampleInfo.m_coreId = AMDT_PROFILE_ALL_CORES;
            sampleInfo.m_counterId = counterId;
        }

        if (addAddStr)
        {
            partialQuery << query.asASCIICharArray();
        }

        return addAddStr;
    } // GetEventAllCorePartialQuery

    bool GetEventCorePartialQuery(const gtVector<AMDTUInt32>& counterIdsList, AMDTUInt64 coreMask, bool separateByCore,
                                  std::stringstream& partialQuery, std::string& firstCountColName,
                                  gtVector<AMDTSampleValue>& sampleInfoVec)
    {
        bool ret = true;
        std::stringstream query;
        bool addComma = false;

        for (auto counterId : counterIdsList)
        {
            AMDTSampleValue sampleInfo;

            if (addComma && ret)
            {
                query << " , ";
            }

            if (separateByCore)
            {
                ret = ret && GetEventSeparateByCorePartialQuery(counterId, coreMask, query, firstCountColName);
            }
            else
            {
                ret = ret && GetEventAllCorePartialQuery(counterId, coreMask, query, firstCountColName, sampleInfo);

                if (ret)
                {
                    sampleInfoVec.emplace_back(sampleInfo);
                }
            }

            addComma = ret;
        }

        if (ret)
        {
            partialQuery << query.str();
        }

        //fprintf(stderr, " %s \n", query.str().c_str());

        return ret;
    }

    bool CreateSampledCounterCoreConfig()
    {
        //drop view SampledCounterCoreConfig;
        //create view SampledCounterCoreConfig as
        //    SELECT CoreSamplingConfiguration.id,
        //    CoreSamplingConfiguration.samplingConfigurationId,
        //    CoreSamplingConfiguration.coreId,
        //    SamplingConfiguration.counterId,
        //    SamplingConfiguration.samplingInterval,
        //    SamplingConfiguration.unitMask,
        //    SamplingConfiguration.isUserMode,
        //    SamplingConfiguration.isOsMode
        //    FROM CoreSamplingConfiguration
        //    INNER JOIN SamplingConfiguration ON samplingConfigurationId = SamplingConfiguration.id;

        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        viewCreateQuery << "CREATE TEMP VIEW IF NOT EXISTS SampledCounterCoreConfig AS                                 \
                            SELECT CoreSamplingConfiguration.id AS id,                                    \
                                   CoreSamplingConfiguration.samplingConfigurationId,               \
                                   CoreSamplingConfiguration.coreId,                                \
                                   SamplingConfiguration.counterId,                                 \
                                   SamplingConfiguration.samplingInterval,                          \
                                   SamplingConfiguration.unitMask,                                  \
                                   SamplingConfiguration.isUserMode,                                \
                                   SamplingConfiguration.isOsMode                                   \
                            FROM CoreSamplingConfiguration                                          \
                            INNER JOIN SamplingConfiguration ON samplingConfigurationId = SamplingConfiguration.id;";

        const std::string& queryStr = viewCreateQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;

    } // CreateSampledCounterCoreConfig

    bool CreateModuleInfoView()
    {
        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        viewCreateQuery << "CREATE TEMP VIEW IF NOT EXISTS ModuleInfo AS     \
                            SELECT Module.id,                      \
                                   Module.path,                    \
                                   ModuleInstance.processId,           \
                                   ModuleInstance.id AS instanceId,    \
                                   ModuleInstance.loadAddress,     \
                                   Module.size,                    \
                                   Module.is32Bit,                 \
                                   Module.foundDebugInfo,          \
                                   Module.type,                    \
                                   Module.isSystemModule           \
                         FROM Module                               \
                         INNER JOIN ModuleInstance ON Module.id = ModuleInstance.moduleId; ";

        int rc = sqlite3_prepare_v2(m_pReadDbConn, viewCreateQuery.str().c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;
    }

    bool CreateProcessSummaryView()
    {
        //create view sampleProcessSummaryData as
        //    SELECT  ProcessThread.processId,
        //    ProcessThread.threadId,
        //    ModuleInstance.moduleId as moduleId,
        //    SampleContext.coreSamplingConfigurationId,
        //    sum(count) as count
        //    FROM SampleContext
        //    INNER JOIN ProcessThread  ON processThreadId = ProcessThread.id
        //    INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id
        //    group by threadId, moduleId, SampleContext.coreSamplingConfigurationId

        //    // Process/Module/Thread overviw, process/modules view (for ALL RAW profile data)
        //    drop view sampleProcessSummaryAllData;
        //create view sampleProcessSummaryAllData as
        //    SELECT sampleProcessSummaryData.processId,
        //    sampleProcessSummaryData.threadId,
        //    sampleProcessSummaryData.moduleId,
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 1 THEN sampleProcessSummaryData.count END) AS 'e1',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 2 THEN sampleProcessSummaryData.count END) AS 'e2',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 3 THEN sampleProcessSummaryData.count END) AS 'e3',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 4 THEN sampleProcessSummaryData.count END) AS 'e4',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 5 THEN sampleProcessSummaryData.count END) AS 'e5',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 6 THEN sampleProcessSummaryData.count END) AS 'e6',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 7 THEN sampleProcessSummaryData.count END) AS 'e7',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 8 THEN sampleProcessSummaryData.count END) AS 'e8'
        //    FROM sampleProcessSummaryData
        //    INNER JOIN CoreSamplingConfiguration ON sampleProcessSummaryData.coreSamplingConfigurationId = CoreSamplingConfiguration.id
        //    group by threadId, moduleId;

        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        viewCreateQuery << "CREATE TEMP VIEW SampleProcessSummaryData AS        \
                            SELECT  ProcessThread.processId,                    \
                                    ProcessThread.threadId,                     \
                                    ModuleInstance.moduleId,                    \
                                    Module.isSystemModule,                      \
                                    Module.foundDebugInfo,                      \
                                    SampleContext.coreSamplingConfigurationId,  \
                                    sum(count) as sampleCount                   \
                            FROM SampleContext                                  \
                            INNER JOIN ProcessThread ON processThreadId = ProcessThread.id       \
                            INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id    \
                            INNER JOIN Module ON ModuleInstance.moduleId = Module.id             \
                            GROUP BY threadId, moduleId, SampleContext.coreSamplingConfigurationId;";

        const std::string& queryStr = viewCreateQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        if (ret)
        {
            std::stringstream summaryViewCreate;
            pViewCreateStmt = nullptr;

            summaryViewCreate << "CREATE TEMP VIEW SampleProcessSummaryAllData AS     \
                                  SELECT SampleProcessSummaryData.processId,          \
                                         SampleProcessSummaryData.threadId,           \
                                         SampleProcessSummaryData.moduleId,           \
                                         SampleProcessSummaryData.isSystemModule,     \
                                         SampleProcessSummaryData.foundDebugInfo, ";

            std::stringstream partialQuery;
            std::string fromCol("CoreSamplingConfiguration.id");
            std::string toCol("SampleProcessSummaryData.sampleCount");
            GetAllEventCorePartialQuery(partialQuery, fromCol, toCol);

            summaryViewCreate << partialQuery.str();

            summaryViewCreate << " FROM SampleProcessSummaryData  \
                                   INNER JOIN CoreSamplingConfiguration ON SampleProcessSummaryData.coreSamplingConfigurationId = CoreSamplingConfiguration.id    \
                                   group by threadId, moduleId;";

            //fprintf(stderr, " %s\n", summaryViewCreate.str().c_str());

            const std::string& queryStr1 = summaryViewCreate.str();
            rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr1.c_str(), -1, &pViewCreateStmt, nullptr);

            if (SQLITE_OK == rc)
            {
                rc = sqlite3_step(pViewCreateStmt);
                sqlite3_finalize(pViewCreateStmt);
            }

            ret = (SQLITE_DONE == rc) ? true : false;
        }

        return ret;
    } // CreateProcessSummaryView

    bool CreateProcessTotalsView()
    {
        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        viewCreateQuery << "CREATE TEMP VIEW SampleProcessTotalsData AS        \
                            SELECT  ProcessThread.processId,                    \
                                    SampleContext.coreSamplingConfigurationId,  \
                                    sum(count) as sampleCount                   \
                            FROM SampleContext                                  \
                            INNER JOIN ProcessThread ON processThreadId = ProcessThread.id       \
                            GROUP BY SampleContext.coreSamplingConfigurationId;";

        int rc = sqlite3_prepare_v2(m_pReadDbConn, viewCreateQuery.str().c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        if (ret)
        {
            std::stringstream summaryViewCreate;
            pViewCreateStmt = nullptr;

            summaryViewCreate << "CREATE TEMP VIEW SampleProcessTotalsAllData AS     \
                                  SELECT SampleProcessTotalsData.processId,  ";

            std::stringstream partialQuery;
            std::string fromCol("CoreSamplingConfiguration.id");
            std::string toCol("SampleProcessTotalsData.sampleCount");
            GetAllEventCorePartialQuery(partialQuery, fromCol, toCol);

            summaryViewCreate << partialQuery.str();

            summaryViewCreate << " FROM SampleProcessTotalsData  \
                                   INNER JOIN CoreSamplingConfiguration ON SampleProcessTotalsData.coreSamplingConfigurationId = CoreSamplingConfiguration.id;";

            //fprintf(stderr, " %s\n", summaryViewCreate.str().c_str());

            rc = sqlite3_prepare_v2(m_pReadDbConn, summaryViewCreate.str().c_str(), -1, &pViewCreateStmt, nullptr);

            if (SQLITE_OK == rc)
            {
                rc = sqlite3_step(pViewCreateStmt);
                sqlite3_finalize(pViewCreateStmt);
            }

            ret = (SQLITE_DONE == rc) ? true : false;
        }

        return ret;
    } // SampleProcessTotalsData

    bool CreateFunctionSummaryView()
    {
        //create view sampleFunctionSummaryData as
        //    SELECT  ProcessThread.processId,
        //    ProcessThread.threadId,
        //    ModuleInstance.moduleId as moduleId,
        //    SampleContext.functionId,
        //    SampleContext.coreSamplingConfigurationId,
        //    sum(count) as totalSamples
        //    FROM SampleContext
        //    INNER JOIN ProcessThread  ON processThreadId = ProcessThread.id
        //    INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id
        //    group by threadId, functionId, SampleContext.coreSamplingConfigurationId

        //create view sampleFunctionSummaryAllData as
        //    SELECT sampleFunctionSummaryData.processId,
        //    sampleFunctionSummaryData.threadId,
        //    sampleFunctionSummaryData.moduleId,
        //    sampleFunctionSummaryData.functionId,
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 1 THEN sampleFunctionSummaryData.totalSamples END) AS 'e1',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 2 THEN sampleFunctionSummaryData.totalSamples END) AS 'e2',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 3 THEN sampleFunctionSummaryData.totalSamples END) AS 'e3',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 4 THEN sampleFunctionSummaryData.totalSamples END) AS 'e4',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 5 THEN sampleFunctionSummaryData.totalSamples END) AS 'e5',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 6 THEN sampleFunctionSummaryData.totalSamples END) AS 'e6',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 7 THEN sampleFunctionSummaryData.totalSamples END) AS 'e7',
        //    MAX(CASE WHEN CoreSamplingConfiguration.id = 8 THEN sampleFunctionSummaryData.totalSamples END) AS 'e8'
        //    FROM sampleFunctionSummaryData
        //    INNER JOIN CoreSamplingConfiguration ON sampleFunctionSummaryData.coreSamplingConfigurationId = CoreSamplingConfiguration.id
        //    group by threadId, functionId;

        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        viewCreateQuery << "CREATE TEMP VIEW SampleFunctionSummaryData AS       \
                            SELECT  ProcessThread.processId,                    \
                                    ProcessThread.threadId,                     \
                                    ModuleInstance.moduleId,                    \
                                    Module.isSystemModule,                      \
                                    SampleContext.offset,                       \
                                    SampleContext.functionId,                   \
                                    SampleContext.coreSamplingConfigurationId,  \
                                    sum(count) as sampleCount                   \
                            FROM SampleContext                                  \
                            INNER JOIN ProcessThread ON processThreadId = ProcessThread.id       \
                            INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id    \
                            INNER JOIN Module ON ModuleInstance.moduleId = Module.id             \
                            GROUP BY threadId, functionId, offset, SampleContext.coreSamplingConfigurationId;";

        const std::string& queryStr = viewCreateQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        if (ret)
        {
            std::stringstream summaryViewCreate;
            pViewCreateStmt = nullptr;

            summaryViewCreate << "CREATE TEMP VIEW SampleFunctionSummaryAllData AS  \
                                  SELECT SampleFunctionSummaryData.processId,       \
                                         SampleFunctionSummaryData.threadId,        \
                                         SampleFunctionSummaryData.moduleId,        \
                                         SampleFunctionSummaryData.isSystemModule,  \
                                         SampleFunctionSummaryData.offset,          \
                                         SampleFunctionSummaryData.functionId,";

            std::stringstream partialQuery;
            std::string fromCol("CoreSamplingConfiguration.id");
            std::string toCol("SampleFunctionSummaryData.sampleCount");
            GetAllEventCorePartialQuery(partialQuery, fromCol, toCol);

            summaryViewCreate << partialQuery.str();

            summaryViewCreate << " FROM SampleFunctionSummaryData  \
                                   INNER JOIN CoreSamplingConfiguration ON SampleFunctionSummaryData.coreSamplingConfigurationId = CoreSamplingConfiguration.id    \
                                   group by threadId, functionId, offset ;";

            //fprintf(stderr, " %s\n", summaryViewCreate.str().c_str());

            const std::string& queryStr1 = summaryViewCreate.str();
            rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr1.c_str(), -1, &pViewCreateStmt, nullptr);

            if (SQLITE_OK == rc)
            {
                rc = sqlite3_step(pViewCreateStmt);
                sqlite3_finalize(pViewCreateStmt);
            }

            ret = (SQLITE_DONE == rc) ? true : false;
        }

        return ret;
    } // CreateFunctionSummaryView

    bool CreateFunctionDetailedDataView(AMDTUInt32 functionId)
    {
        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        gtString funcIdStr;
        funcIdStr.appendFormattedString(L"%d", functionId);

#if 0
        viewCreateQuery << "CREATE TEMP VIEW SampleFunctionDetailedData AS      \
                            SELECT  ProcessThread.processId,                    \
                                    ProcessThread.threadId,                     \
                                    ModuleInstance.moduleId,                    \
                                    SampleContext.offset,                       \
                                    SampleContext.coreSamplingConfigurationId,  \
                                   count as sampleCount                        \
                            FROM SampleContext                                  \
                            INNER JOIN ProcessThread ON processThreadId = ProcessThread.id      \
                            INNER JOIN ModuleInstance ON moduleInstanceId = ModuleInstance.id   \
                            WHERE SampleContext.functionId = ";
#endif //0

        viewCreateQuery << "CREATE TEMP VIEW IF NOT EXISTS SampleFunctionDetailedData AS    \
                            SELECT ProcessThread.processId,             \
                                   ProcessThread.threadId,              \
                                   SampleContext.offset, ";

        std::stringstream partialQuery;
        std::string fromCol("CoreSamplingConfiguration.id");
        std::string toCol("SampleContext.count");
        GetAllEventCorePartialQuery(partialQuery, fromCol, toCol);

        viewCreateQuery << partialQuery.str();

        viewCreateQuery << " FROM SampleContext  \
                             INNER JOIN ProcessThread ON processThreadId = ProcessThread.id  \
                             INNER JOIN CoreSamplingConfiguration ON coreSamplingConfigurationId = CoreSamplingConfiguration.id  \
                             WHERE functionId = ";

        viewCreateQuery << funcIdStr.asASCIICharArray();
        viewCreateQuery << " GROUP BY processId, threadId, offset ";

        viewCreateQuery << ";";

        //fprintf(stderr, " %s\n", viewCreateQuery.str().c_str());

        const std::string& queryStr = viewCreateQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;
    }

    bool DropFunctionDetailedDataView()
    {
        bool ret = false;
        sqlite3_stmt* pViewCreateStmt = nullptr;
        std::stringstream viewCreateQuery;

        viewCreateQuery << "DROP VIEW SampleFunctionDetailedData;";

        const std::string& queryStr = viewCreateQuery.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pViewCreateStmt, nullptr);

        if (SQLITE_OK == rc)
        {
            rc = sqlite3_step(pViewCreateStmt);
            sqlite3_finalize(pViewCreateStmt);
        }

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;
    }


    //
    // FunctionId is unique across all the modules
    // ModuleInstanceId is internal only
    // SourceLine in SampleContex - should be OK as the backend finds the sourceline for a given module offset
    //
    // Possible Cases
    //  for all the processes (processId = -1)
    //  for all the given process (processId = <pid>)
    //
    bool GetProcessSummaryData(
        AMDTProcessId               processId,
        AMDTModuleId                moduleId,
        const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        bool                        doSort,
        size_t                      count,
        gtVector<AMDTProfileData>&  dataList)
    {
        bool ret = false;

        // select processId,
        //        (sum(ifnull(e1, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e5, 0)) + sum(ifnull(e7, 0))) as eventTotal-1,
        //        (sum(ifnull(e2, 0)) + sum(ifnull(e4, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e8, 0))) as eventTotal-2
        //        from SampleProcessSummaryAllData group by processId order by total desc;

        std::stringstream query;
        query << "SELECT processId, ";

        std::stringstream partialQuery;
        std::string firstCountColName;

        gtVector<AMDTSampleValue> sampleInfoVec;

        ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);
        query << partialQuery.str();
        query << " FROM SampleProcessSummaryAllData ";

        if (ret)
        {
            gtString partQuery;

            if (IS_PROCESS_MODULE_QUERY(processId, moduleId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d AND moduleId = %d ", processId, moduleId);
            }
            else if (IS_PROCESS_QUERY(processId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d ", processId);
            }
            else if (IS_MODULE_QUERY(moduleId))
            {
                partQuery.appendFormattedString(L" WHERE moduleId = %d ", moduleId);
            }

            query << partQuery.asASCIICharArray();
            query << " GROUP BY processID ";

            if (doSort)
            {
                query << " ORDER BY " << firstCountColName << " DESC ";
            }

            //fprintf(stderr, " %s \n", query.str().c_str());

            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string& queryStr = query.str();
            int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Execute the query.
                size_t tmpCount = (count > 0) ? count : INT_MAX;

                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW && tmpCount > 0)
                {
                    AMDTProfileData profileData;
                    profileData.m_type = AMDT_PROFILE_DATA_PROCESS;

                    AMDTProcessId pid = sqlite3_column_int(pQueryStmt, 0);
                    profileData.m_id = pid;
                    profileData.m_moduleId = AMDT_PROFILE_ALL_MODULES;

                    GetProcessName(pid, profileData.m_name);

                    int idx = 1;

                    for (auto& sample : sampleInfoVec)
                    {
                        sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                        idx++;

                        profileData.m_sampleValue.emplace_back(sample);
                    }

                    dataList.emplace_back(profileData);
                    --tmpCount;
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    } // GetProcessSummaryData

    // Possible Cases
    //  for all modules from all the processes (moduleId = -1, processId = -1)
    //  for all modules from the given process (moduleId = -1, processId = <pid>)
    //  for the given module from all the processes (moduleId = <mid>, processId = -1)
    //  for the given module from the given process (moduleId = <mid>, processId = <pid>)
    //
    // TODO: !!! module specific profile data can be queried for a given thread too -> Do we need it? !!!
    //
    bool GetModuleSummaryData(
        AMDTModuleId                moduleId,
        AMDTProcessId               processId,           // for a given process or for all processes
        const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        ignoreSystemModules,
        bool                        separateByCore,
        bool                        doSort,
        size_t                      count,
        gtVector<AMDTProfileData>&  dataList)
    {
        bool ret = false;

        // select moduleId, processId
        //        (sum(ifnull(e1, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e5, 0)) + sum(ifnull(e7, 0))) as eventTotal-1,
        //        (sum(ifnull(e2, 0)) + sum(ifnull(e4, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e8, 0))) as eventTotal-2
        //        from SampleProcessSummaryAllData group by moduleId order by eventTotal-1 desc;

        std::stringstream query;
        query << "SELECT moduleId, processId, ";

        std::stringstream partialQuery;
        std::string firstCountColName;

        gtVector<AMDTSampleValue> sampleInfoVec;

        ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);
        query << partialQuery.str();
        query << " FROM SampleProcessSummaryAllData ";

        if (ret)
        {
            gtString partQuery;
            bool hasWhereClause = false;

            if (IS_PROCESS_MODULE_QUERY(processId, moduleId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d AND moduleId = %d ", processId, moduleId);
                hasWhereClause = true;
            }
            else if (IS_PROCESS_QUERY(processId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d ", processId);
                hasWhereClause = true;
            }
            else if (IS_MODULE_QUERY(moduleId))
            {
                partQuery.appendFormattedString(L" WHERE moduleId = %d ", moduleId);
                hasWhereClause = true;
            }

            if (ignoreSystemModules)
            {
                partQuery.append(hasWhereClause ? L" AND isSystemModule = 0 " : L" WHERE isSystemModule = 0 ");
            }

            query << partQuery.asASCIICharArray();
            query << " GROUP BY moduleId ";

            if (doSort)
            {
                query << " ORDER BY " << firstCountColName << " DESC ";
            }

            //fprintf(stderr, " %s \n", query.str().c_str());

            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string& queryStr = query.str();
            int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Execute the query.
                size_t tmpCount = (count > 0) ? count : INT_MAX;

                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW && tmpCount > 0)
                {
                    AMDTProfileData profileData;
                    profileData.m_type = AMDT_PROFILE_DATA_MODULE;

                    AMDTModuleId mid = sqlite3_column_int(pQueryStmt, 0);
                    profileData.m_moduleId = mid; // module ID

                    AMDTProcessId pid = sqlite3_column_int(pQueryStmt, 1);
                    profileData.m_id = pid; // process ID

                    GetModulePath(mid, profileData.m_name);

                    int idx = 2;

                    for (auto& sample : sampleInfoVec)
                    {
                        sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                        idx++;

                        profileData.m_sampleValue.emplace_back(sample);
                    }

                    dataList.emplace_back(profileData);
                    --tmpCount;
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    } // GetModuleSummaryData

    // Possible Cases
    //  for all threads from all the processes (threadId = -1, processId = -1)
    //  for all threads from the given process (threadId = -1, processId = <pid>)
    //  for the given thread from all the processes (threadId = <mid>, processId = -1)
    //  for the given thread from the given process (threadId = <mid>, processId = <pid>)
    //
    // TODO: !!! thread specific profile data can be queried for a given module too -> Do we need it? !!!
    //
    bool GetThreadSummaryData(
        AMDTProcessId               processId,           // for a given process or for all processes
        AMDTThreadId                threadId,
        const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        bool                        doSort,
        size_t                      count,
        gtVector<AMDTProfileData>&  dataList)
    {
        bool ret = false;

        // select threadId, processId
        //        (sum(ifnull(e1, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e5, 0)) + sum(ifnull(e7, 0))) as eventTotal-1,
        //        (sum(ifnull(e2, 0)) + sum(ifnull(e4, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e8, 0))) as eventTotal-2
        //        from SampleProcessSummaryAllData group by threadId order by eventTotal-1 desc;

        std::stringstream query;
        query << "SELECT threadId, ";

        std::stringstream partialQuery;
        std::string firstCountColName;

        gtVector<AMDTSampleValue> sampleInfoVec;

        ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);
        query << partialQuery.str();
        query << " FROM SampleProcessSummaryAllData ";

        if (ret)
        {
            gtString partQuery;

            if (IS_PROCESS_THREAD_QUERY(processId, threadId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d AND threadId = %d ", processId, threadId);
            }
            else if (IS_PROCESS_QUERY(processId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d ", processId);
            }
            else if (IS_THREAD_QUERY(threadId))
            {
                partQuery.appendFormattedString(L" WHERE threadId = %d ", threadId);
            }

            query << partQuery.asASCIICharArray();
            query << " GROUP BY threadId ";

            if (doSort)
            {
                query << " ORDER BY " << firstCountColName << " DESC ";
            }

            //fprintf(stderr, " %s \n", query.str().c_str());

            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string& queryStr = query.str();
            int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Execute the query.
                size_t tmpCount = (count > 0) ? count : INT_MAX;

                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW && tmpCount > 0)
                {
                    AMDTProfileData profileData;
                    profileData.m_type = AMDT_PROFILE_DATA_THREAD;

                    AMDTThreadId tid = sqlite3_column_int(pQueryStmt, 0);
                    profileData.m_id = tid;
                    profileData.m_moduleId = AMDT_PROFILE_ALL_MODULES;

                    int idx = 1;

                    for (auto& sample : sampleInfoVec)
                    {
                        sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                        idx++;

                        profileData.m_sampleValue.emplace_back(sample);
                    }

                    dataList.emplace_back(profileData);
                    --tmpCount;
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    }

    // if processId == -1, entire profile run
    // otherwise, given process
    bool GetProcessTotals(
        AMDTProcessId               processId,           // for a given process or for all processes
        const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        AMDTSampleValueVec&         sampleValueVec)
    {
        bool ret = false;

        // select threadId, processId
        //        (sum(ifnull(e1, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e5, 0)) + sum(ifnull(e7, 0))) as eventTotal-1,
        //        (sum(ifnull(e2, 0)) + sum(ifnull(e4, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e8, 0))) as eventTotal-2
        //        from SampleProcessSummaryAllData group by threadId order by eventTotal-1 desc;

        std::stringstream query;
        query << "SELECT processId, ";

        std::stringstream partialQuery;
        std::string firstCountColName;

        gtVector<AMDTSampleValue> sampleInfoVec;

        ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);
        query << partialQuery.str();
        query << " FROM SampleProcessTotalsAllData ";

        if (ret)
        {
            gtString partQuery;

            if (IS_PROCESS_QUERY(processId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d ", processId);
                query << partQuery.asASCIICharArray();
            }

            //fprintf(stderr, " %s \n", query.str().c_str());

            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string& queryStr = query.str();
            int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Execute the query.
                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                {
                    AMDTProcessId pid = sqlite3_column_int(pQueryStmt, 0);
                    pid = pid; // avoid compiler warning

                    int idx = 1;
                    for (auto& sample : sampleInfoVec)
                    {
                        sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                        idx++;

                        sampleValueVec.emplace_back(sample);
                    }
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    }

    // moduleId cannot be AMDT_PROFILE_ALL_MODULES
    //   for the given module among all process
    //   for the given module within given process
    bool GetModuleTotals(
        AMDTModuleId                moduleId,
        AMDTProcessId               processId,           // for a given process or for all processes
        const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        AMDTSampleValueVec&         sampleValueVec)
    {
        bool ret = IS_MODULE_QUERY(moduleId);

        if (ret)
        {
            // select moduleId,
            //        (sum(ifnull(e1, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e5, 0)) + sum(ifnull(e7, 0))) as eventTotal-1,
            //        (sum(ifnull(e2, 0)) + sum(ifnull(e4, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e8, 0))) as eventTotal-2
            //        from SampleProcessSummaryAllData
            //       where SampleProcessSummaryAllData.moduleId = moduleId;

            std::stringstream query;
            query << "SELECT moduleId, ";

            std::stringstream partialQuery;
            std::string firstCountColName;

            gtVector<AMDTSampleValue> sampleInfoVec;

            ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);

            if (ret)
            {
                query << partialQuery.str();
                query << " FROM SampleProcessSummaryAllData ";

                gtString partQuery;

                partQuery.appendFormattedString(L" WHERE moduleId = %d ", moduleId);

                if (IS_PROCESS_QUERY(processId))
                {
                    partQuery.appendFormattedString(L" AND processId = %d ", processId);
                }

                query << partQuery.asASCIICharArray();

                //fprintf(stderr, " %s \n", query.str().c_str());

                sqlite3_stmt* pQueryStmt = nullptr;
                const std::string& queryStr = query.str();
                int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

                if (rc == SQLITE_OK)
                {
                    // Execute the query.
                    while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                    {
                        AMDTModuleId mid = sqlite3_column_int(pQueryStmt, 0);
                        GT_UNREFERENCED_PARAMETER(mid);

                        int idx = 1;
                        for (auto& sample : sampleInfoVec)
                        {
                            sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                            idx++;

                            sampleValueVec.emplace_back(sample);
                        }
                    }
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);

                ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
            }
        }

        return ret;
    } // GetModuleTotals

    bool GetProcessName(AMDTProcessId procId, gtString& procName)
    {
        bool ret = false;

        std::stringstream query;
        query << "SELECT name from Process where id = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, procId);

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 0);

                procName.fromUtf8String(reinterpret_cast<const char*>(path));
            }
        }

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;

        return ret;
    }

    bool GetModulePath(AMDTModuleId modId, gtString& modPath)
    {
        bool ret = false;

        std::stringstream query;
        query << "SELECT path from Module where id = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, modId);

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 0);

                modPath.fromUtf8String(reinterpret_cast<const char*>(path));
            }
        }

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;

        return ret;
    }

    bool GetModuleName(AMDTModuleId modId, gtString& modName)
    {
        bool ret = false;
        gtString modPath;

        ret = GetModulePath(modId, modPath);

        if (ret && !modPath.isEmpty())
        {
            osFilePath aPath(modPath);
            aPath.getFileNameAndExtension(modName);
        }

        return ret;
    }

    bool GetModuleBaseAddressByModuleId__(AMDTModuleId modId, AMDTProcessId procId, AMDTUInt64& baseAddr, bool isInstanceId)
    {
        bool ret = false;

        std::stringstream query;
        query << "SELECT loadAddress        \
                  FROM ModuleInstance       \
                  WHERE ";

        if (IS_PROCESS_QUERY(procId))
        {
            query << " processId = ? AND ";
        }

        if (!isInstanceId)
        {
            query << " moduleID = ? LIMIT 1; ";
        }
        else
        {
            query << " id = ? LIMIT 1; ";
        }

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            if (IS_PROCESS_QUERY(procId))
            {
                sqlite3_bind_int(pQueryStmt, 1, procId);
                sqlite3_bind_int(pQueryStmt, 2, modId);
            }
            else
            {
                sqlite3_bind_int(pQueryStmt, 1, modId);
            }

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                baseAddr = sqlite3_column_int64(pQueryStmt, 0);
            }
        }

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        return ret;
    }


    bool GetModuleBaseAddressByModuleId(AMDTModuleId modId, AMDTProcessId procId, AMDTUInt64& baseAddr)
    {
        return GetModuleBaseAddressByModuleId__(modId, procId, baseAddr, false);
    }

    bool GetModuleBaseAddressByModuleInstanceId(AMDTModuleId modInstId, AMDTProcessId procId, AMDTUInt64& baseAddr)
    {
        return GetModuleBaseAddressByModuleId__(modInstId, procId, baseAddr, true);
    }

    bool GetModuleBaseAddress(AMDTFunctionId funcId, AMDTProcessId procId, AMDTUInt64& baseAddr)
    {
        bool ret = false;

        std::stringstream query;
        //select loadAddress from ModuleInstance where processId = 7828 and moduleId = (select moduleId from Function where id = 6);
        query << "SELECT loadAddress        \
                  FROM ModuleInstance       \
                  WHERE ";

        if (IS_PROCESS_QUERY(procId))
        {
            query << " processId = ? AND ";
        }

        query << " moduleID = (SELECT moduleId from Function WHERE id = ? LIMIT 1);";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            if (IS_PROCESS_QUERY(procId))
            {
                sqlite3_bind_int(pQueryStmt, 1, procId);
                sqlite3_bind_int(pQueryStmt, 2, funcId);
            }
            else
            {
                sqlite3_bind_int(pQueryStmt, 1, funcId);
            }

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                baseAddr = sqlite3_column_int64(pQueryStmt, 0);
            }
        }

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        return ret;
    }

    bool IsSystemModule(AMDTModuleId modId, bool& isSysMod)
    {
        bool ret = false;

        std::stringstream query;
        query << "SELECT isSystemModule        \
                  FROM Module       \
                  WHERE id = ? ; ";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, modId);

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                int i = sqlite3_column_int(pQueryStmt, 0);
                isSysMod = (i == 1) ? true : false;
            }
        }

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        return ret;
    }

    bool ConstructUnknownFunctionInfo(AMDTFunctionId funcId, gtUInt64 offset, AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;

        AMDTModuleId modId = (funcId & 0xffff0000) >> 16;

        funcInfo.m_functionId = funcId;
        funcInfo.m_moduleId = modId;
        funcInfo.m_size = 0; // CXL_UNKNOWN_FUNC_SIZE;
        funcInfo.m_startOffset = offset;

        ret = GetModulePath(modId, funcInfo.m_modulePath);

        // TODO: if required construct name from modulepath and offset

        return ret;
    }

    bool ConstructUnknownFunctionName(AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = GetModuleName(funcInfo.m_moduleId, funcInfo.m_name);

        // TODO:
        // AMDTUInt64 baseAddr = 0;
        // ret = GetModuleBaseAddressByModuleId(funcInfo.m_moduleId, procId, baseAddr);

       funcInfo.m_name.appendFormattedString(L"!0x" LONG_FORMAT_HEX, funcInfo.m_startOffset);

        return ret;
    }

    bool GetFunctionName(AMDTFunctionId funcId, gtString& funcName)
    {
        bool ret = false;

        std::stringstream query;
        query << "SELECT name from Function where id = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, funcId);

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 0);

                funcName.fromUtf8String(reinterpret_cast<const char*>(path));
                ret = true;
            }
        }

        sqlite3_finalize(pQueryStmt);
        // ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        return ret;
    }

    bool GetFunctionInfo(AMDTFunctionId funcId, gtUInt32 startOffset, AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;
        std::stringstream query;
        query << "SELECT name, moduleId, startOffset, size from Function where id = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        gtUInt32 offset = 0;
        gtUInt32 size = 0;
        AMDTModuleId moduleId = (funcId & DB_MODULEID_MASK) >> 16;

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, funcId);

            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 0);
                funcInfo.m_name.fromUtf8String(reinterpret_cast<const char*>(path));

                moduleId = sqlite3_column_int(pQueryStmt, 1);
                offset = sqlite3_column_int(pQueryStmt, 2);
                size = sqlite3_column_int(pQueryStmt, 3);
            }

            GetModulePath(moduleId, funcInfo.m_modulePath);
        }

        funcInfo.m_functionId = funcId;
        funcInfo.m_moduleId = moduleId;
        funcInfo.m_startOffset = (offset > 0) ? offset : startOffset;
        funcInfo.m_size = size;

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        return ret;
    }

    bool GetFunctionInfoByModuleId(AMDTModuleId moduleId, AMDTProfileFunctionInfoVec& funcInfoVec)
    {
        bool ret = false;
        std::stringstream query;
        query << "SELECT id, name, startOffset, size from Function where moduleId = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, moduleId);

            gtString modulePath;
            GetModulePath(moduleId, modulePath);

            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                AMDTProfileFunctionInfo funcInfo;

                funcInfo.m_functionId = sqlite3_column_int(pQueryStmt, 0);
                const unsigned char* path = sqlite3_column_text(pQueryStmt, 1);
                funcInfo.m_name.fromUtf8String(reinterpret_cast<const char*>(path));

                funcInfo.m_startOffset = sqlite3_column_int(pQueryStmt, 2);
                funcInfo.m_size = sqlite3_column_int(pQueryStmt, 3);
                funcInfo.m_moduleId = moduleId;
                funcInfo.m_modulePath = modulePath;

                funcInfoVec.push_back(funcInfo);
            }
        }

        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc) ? true : false;
        return ret;
    }

    bool GetUnknownFunctionsByIPSamples(gtVector<AMDTProfileFunctionInfo>& funcList)
    {
        bool ret = false;
        std::stringstream query;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        query << "SELECT functionId, offset " \
            "FROM SampleContext "    \
            "WHERE functionId & 0x0000ffff = 0;";
#else
        query << "SELECT functionId, offset " \
            "FROM SampleContext "    \
            "WHERE (functionId & 65535) = 0;";
#endif

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            // Execute the query.
            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                AMDTProfileFunctionInfo funcInfo;
                funcInfo.m_functionId = sqlite3_column_int(pQueryStmt, 0);
                funcInfo.m_startOffset = sqlite3_column_int(pQueryStmt, 1);
                funcInfo.m_moduleId = ((funcInfo.m_functionId & DB_MODULEID_MASK) >> 16);

                ConstructUnknownFunctionInfo(funcInfo.m_functionId, funcInfo.m_startOffset, funcInfo);

                funcList.emplace_back(funcInfo);
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;

        return ret;
    }

    // Possible Cases
    //  for all threads from all the processes (functionId = -1, threadId = -1, processId = -1)
    //  for all threads from the given process (threadId = -1, processId = <pid>)
    //  for the given thread from all the processes (threadId = <mid>, processId = -1) // happens only if threadID is recycled
    //  for the given thread from the given process (threadId = <mid>, processId = <pid>)
    //
    bool GetFunctionSummaryData(
        AMDTProcessId               processId,           // for a given process or for all processes
        AMDTThreadId                threadId,
        AMDTModuleId                moduleId,
        gtVector<AMDTUInt32>&       counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        ignoreSystemModules,
        bool                        separateByCore,
        bool                        separateByProcess,
        bool                        doSort,
        size_t                      count,
        gtVector<AMDTProfileData>&  dataList)
    {
        bool ret = false;

        ret = GetFunctionSummaryData__(processId,
                                       threadId,
                                       moduleId,
                                       counterIdsList,
                                       coreMask,
                                       ignoreSystemModules,
                                       separateByCore,
                                       separateByProcess,
                                       doSort,
                                       count,
                                       dataList,
                                       false);

        return ret;
    }

    bool HasCLU(gtVector<AMDTUInt32>& counterIdsList, gtString& lhsName, gtString& rhsName)
    {
        bool foundCLU = false;
        bool foundL1Evictions = false;

        for (auto& cid : counterIdsList)
        {
            if (CXL_CLU_EVENT_CLU_PERCENTAGE == cid)
            {
                foundCLU = true;
                lhsName.appendFormattedString(L"eventTotal%d ", cid);
            }
            else if (CXL_CLU_EVENT_L1_EVICTIONS == cid)
            {
                foundL1Evictions = true;
                rhsName.appendFormattedString(L"eventTotal%d ", cid);
            }
        }

        return (foundCLU & foundL1Evictions);
    }

    bool GetFunctionSummaryData__(
        AMDTProcessId               processId,           // for a given process or for all processes
        AMDTThreadId                threadId,
        AMDTModuleId                moduleId,
        gtVector<AMDTUInt32>&       counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        ignoreSystemModules,
        bool                        separateByCore,
        bool                        separateByProcess,
        bool                        doSort,
        size_t                      count,
        gtVector<AMDTProfileData>&  dataList,
        bool                        queryUnknownFuncs)
    {
        bool ret = false;

        //select sampleFunctionSummaryAllData.functionId, sampleFunctionSummaryAllData.threadId, sampleFunctionSummaryAllData.processId,
        //    (sum(ifnull(e1, 0)) + sum(ifnull(e2, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e4, 0))) as eventTotal1,
        //    (sum(ifnull(e5, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e7, 0)) + sum(ifnull(e8, 0))) as eventTotal2
        //    from sampleFunctionSummaryAllData
        //    group by functionId, processId order by eventTotal1 desc;

        std::stringstream query;
        query << "SELECT offset, functionId, moduleId, ";

        if (separateByProcess)
        {
            query << " processId,  ";
        }

        std::stringstream partialQuery;
        std::string firstCountColName;

        gtVector<AMDTSampleValue> sampleInfoVec;

        ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);
        query << partialQuery.str();
        query << " FROM SampleFunctionSummaryAllData ";

        if (ret)
        {
            gtString partQuery;
            bool hasWhereClause = false;

            if (IS_PROCESS_THREAD_QUERY(processId, threadId) && IS_MODULE_QUERY(moduleId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d AND threadId = %d AND moduleID = %d ", processId, threadId, moduleId);
                hasWhereClause = true;
            }
            else if (IS_PROCESS_THREAD_QUERY(processId, threadId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d AND threadId = %d ", processId, threadId);
                hasWhereClause = true;
            }
            else if (IS_PROCESS_MODULE_QUERY(processId, moduleId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d AND moduleId = %d ", processId, moduleId);
                hasWhereClause = true;
            }
            else if (IS_PROCESS_QUERY(processId))
            {
                partQuery.appendFormattedString(L" WHERE processId = %d ", processId);
                hasWhereClause = true;
            }
            else if (IS_THREAD_QUERY(threadId))
            {
                partQuery.appendFormattedString(L" WHERE threadId = %d ", threadId);
                hasWhereClause = true;
            }
            else if (IS_MODULE_QUERY(moduleId))
            {
                partQuery.appendFormattedString(L" WHERE moduleId = %d ", moduleId);
                hasWhereClause = true;
            }

            if (ignoreSystemModules)
            {
                partQuery.append(hasWhereClause ? L" AND isSystemModule = 0 " : L" WHERE isSystemModule = 0 ");
            }

            query << partQuery.asASCIICharArray();

            if (!queryUnknownFuncs)
            {
                // query << " GROUP BY functionId HAVING functionId & 0x0000ffff > 0 ";  // Don't aggregate for unknown functions
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                query << " GROUP BY (CASE WHEN (functionId & 0x0000FFFF) > 0 THEN functionId ELSE offset END) ";
#else
                query << " GROUP BY (CASE WHEN (functionId & 65535) > 0 THEN functionId ELSE offset END) ";
#endif
            }
            else
            {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                query << " GROUP BY functionId, offset HAVING functionId & 0x0000ffff = 0 ";
#else
                query << " GROUP BY functionId, offset HAVING (functionId & 65535) = 0 ";
#endif
            }

            if (doSort)
            {
                // Klude: handling proper sorting for CLU here?
                gtString lhsEventName;
                gtString rhsEventName;

                if (HasCLU(counterIdsList, lhsEventName, rhsEventName))
                {
                    query << " ORDER BY ( ";
                    query << lhsEventName.asASCIICharArray();
                    query << " / ";
                    query << rhsEventName.asASCIICharArray();
                    query << " ) ASC ";
                }
                else
                {
                    query << " ORDER BY " << firstCountColName << " DESC ";
                }
            }

            query << " ; ";

            const std::string& queryStr = query.str();
            // fprintf(stderr, " %s \n", queryStr.c_str());

            sqlite3_stmt* pQueryStmt = nullptr;
            int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Execute the query.
                size_t tmpCount = (count > 0) ? count : INT_MAX;

                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW && tmpCount > 0)
                {
                    AMDTProfileData profileData;
                    profileData.m_type = AMDT_PROFILE_DATA_FUNCTION;

                    AMDTUInt32 offset = sqlite3_column_int(pQueryStmt, 0);

                    AMDTFunctionId id = sqlite3_column_int(pQueryStmt, 1);
                    profileData.m_id = id;

                    GetFunctionName(id, profileData.m_name);

                    AMDTModuleId mid = sqlite3_column_int(pQueryStmt, 2);

                    // FIXME: kludge. using m_moduleId to pass the offset to report layer for unknown functions.
                    // the report layer in turn will use this to construct the id for unknown-functions
                    // and replace this with the proper module-id value..
                    //profileData.m_moduleId = mid;
                    profileData.m_moduleId = ((id & 0x0000FFFF) == 0) ? offset : mid;
                  
                    int idx = 3;
                    if (separateByProcess)
                    {
                        AMDTProcessId pid = sqlite3_column_int(pQueryStmt, idx);
                        GT_UNREFERENCED_PARAMETER(pid);
                        idx++;
                    }

                    for (auto& sample : sampleInfoVec)
                    {
                        sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                        idx++;

                        profileData.m_sampleValue.emplace_back(sample);
                    }

                    dataList.emplace_back(profileData);
                    --tmpCount;
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    }

    bool GetUnknownFunctions(gtVector<AMDTProfileFunctionInfo>& funcList)
    {
        bool ret = false;
        std::stringstream query;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        query << "SELECT functionId, moduleId, offset " \
                 "FROM SampleFunctionSummaryAllData "    \
                 "WHERE functionId & 0x0000ffff = 0;";
#else
        query << "SELECT functionId, moduleId, offset " \
                 "FROM SampleFunctionSummaryAllData "    \
                 "WHERE (functionId & 65535) = 0;";
#endif

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            // Execute the query.
            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                AMDTProfileFunctionInfo funcInfo;
                funcInfo.m_functionId = sqlite3_column_int(pQueryStmt, 0);
                funcInfo.m_moduleId = sqlite3_column_int(pQueryStmt, 1);
                funcInfo.m_startOffset = sqlite3_column_int(pQueryStmt, 2);

                ConstructUnknownFunctionInfo(funcInfo.m_functionId, funcInfo.m_startOffset, funcInfo);

                funcList.emplace_back(funcInfo);
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;

        return ret;
    }

    // Retrives the list of processes and threads for which the given function-id has samples
    bool GetProcessAndThreadListForFunction(
        AMDTFunctionId              funcId,
        gtVector<AMDTUInt32>        counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        gtVector<AMDTProcessId>&    processList,
        gtVector<AMDTThreadId>&     threadList)
    {
        GT_UNREFERENCED_PARAMETER(coreMask);
        GT_UNREFERENCED_PARAMETER(counterIdsList);
        GT_UNREFERENCED_PARAMETER(funcId);
        bool ret = false;

        if (IS_FUNCTION_QUERY(funcId))
        {
            std::stringstream query;
            sqlite3_stmt* pQueryStmt = nullptr;

            // TODO: Need to use the counterIdsList and coreMask
            // query << "SELECT DISTINCT processId, threadId FROM SampleFunctionSummaryAllData WHERE functionId = ? ;";
            query << "SELECT DISTINCT processId, threadId FROM SampleFunctionDetailedData ;";
            int rc = sqlite3_prepare_v2(m_pReadDbConn, query.str().c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                gtSet<AMDTProcessId> pidUniqueSet;
                gtSet<AMDTThreadId> tidUniqueSet;

                //sqlite3_bind_int(pQueryStmt, 1, funcId);

                // Execute the query.
                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                {
                    pidUniqueSet.insert(sqlite3_column_int(pQueryStmt, 0));
                    tidUniqueSet.insert(sqlite3_column_int(pQueryStmt, 1));
                }

                for (auto& pid : pidUniqueSet)
                {
                    processList.push_back(pid);
                }

                for (auto& tid : tidUniqueSet)
                {
                    threadList.push_back(tid);
                }

                pidUniqueSet.clear();
                tidUniqueSet.clear();
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    }

    // Retrives the list of processes and threads for which the given function-id has samples
    bool GetProcessAndThreadListForFunction(
        AMDTFunctionId              funcId,
        AMDTUInt32                  funcStartOffset,
        gtVector<AMDTProcessId>&    processList,
        gtVector<AMDTThreadId>&     threadList)
    {
        bool ret = false;

        if (IS_FUNCTION_QUERY(funcId))
        {
            std::stringstream query;
            sqlite3_stmt* pQueryStmt = nullptr;
            bool isUnknownFunc = IS_UNKNOWN_FUNC(funcId);

            query << "SELECT DISTINCT processId, threadId FROM SampleFunctionSummaryAllData WHERE functionId = ? ";

            if (isUnknownFunc && (funcStartOffset > 0))
            {
                query << " and offset = ? ";
            }

            query << " ; ";

            int rc = sqlite3_prepare_v2(m_pReadDbConn, query.str().c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                gtSet<AMDTProcessId> pidUniqueSet;
                gtSet<AMDTThreadId> tidUniqueSet;

                sqlite3_bind_int(pQueryStmt, 1, funcId);

                if (isUnknownFunc)
                {
                    sqlite3_bind_int(pQueryStmt, 2, funcStartOffset);
                }

                // Execute the query.
                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                {
                    processList.push_back(sqlite3_column_int(pQueryStmt, 0));
                    threadList.push_back(sqlite3_column_int(pQueryStmt, 1));
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc) ? true : false;
        }

        return ret;
    }

    // Supported
    //      process (all processes, given process)
    //      thread (all processes, given process)
    bool GetFunctionTotals(
        AMDTFunctionId              funcId,
        AMDTProcessId               processId,
        AMDTThreadId                threadId,
        const gtVector<AMDTUInt32>& counterIdsList,
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        AMDTSampleValueVec&         sampleValueVec)
    {
        bool ret = false;

        //select sampleFunctionSummaryAllData.functionId,
        //    (sum(ifnull(e1, 0)) + sum(ifnull(e2, 0)) + sum(ifnull(e3, 0)) + sum(ifnull(e4, 0))) as eventTotal1,
        //    (sum(ifnull(e5, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e7, 0)) + sum(ifnull(e8, 0))) as eventTotal2
        //    from sampleFunctionSummaryAllData
        //    where functionId = funcId;

        std::stringstream query;
        query << "SELECT functionId, ";

        std::stringstream partialQuery;
        std::string firstCountColName;
        AMDTSampleValueVec sampleInfoVec;

        ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);

        if (ret)
        {
            query << partialQuery.str();
            query << " FROM SampleFunctionSummaryAllData ";

            gtString partQuery;
            partQuery.appendFormattedString(L" WHERE functionId = %d ", funcId);

            if (IS_PROCESS_QUERY(processId))
            {
                partQuery.appendFormattedString(L" AND processId = %d ", processId);
            }

            if (IS_THREAD_QUERY(threadId))
            {
                partQuery.appendFormattedString(L" AND threadId = %d ", threadId);
            }

            query << partQuery.asASCIICharArray();

            //fprintf(stderr, " %s \n", query.str().c_str());

            sqlite3_stmt* pQueryStmt = nullptr;
            const std::string& queryStr = query.str();
            int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                // Execute the query.
                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                {
                    AMDTProfileData profileData;
                    profileData.m_type = AMDT_PROFILE_DATA_FUNCTION;

                    int idx = 0;
                    AMDTFunctionId id = sqlite3_column_int(pQueryStmt, idx++);
                    id = id; // avoid compiler warning

                    for (auto& sample : sampleInfoVec)
                    {
                        sample.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                        idx++;

                        sampleValueVec.emplace_back(sample);
                    }
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);

            ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;
        }

        return ret;
    }

    bool GetJITFunctionInfo(AMDTFunctionId funcId, gtUInt64& loadAddr, gtString& srcFilePath, gtString& jncFilePath)
    {
        bool ret = false;
        std::stringstream query;

        query << "SELECT JITInstance.loadAddress, " \
                        "JITCodeBlob.srcFilePath, " \
                        "JITCodeBlob.jncFilePath "  \
                  "FROM JITInstance "               \
                  "INNER JOIN JITCodeBlob ON JITInstance.jitId = JITCodeBlob.id "   \
                  "WHERE JITInstance.functionId = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, funcId);

            // Execute the query.
            if ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                loadAddr = sqlite3_column_int64(pQueryStmt, 0);
                const unsigned char* pSrcFilePath = sqlite3_column_text(pQueryStmt, 1);
                const unsigned char* pJncFilePath = sqlite3_column_text(pQueryStmt, 2);

                if (nullptr != pSrcFilePath && nullptr != pJncFilePath)
                {
                    srcFilePath.fromUtf8String((const char*)pSrcFilePath);
                    jncFilePath.fromUtf8String((const char*)pJncFilePath);
                }
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);
        ret = (SQLITE_DONE == rc || SQLITE_ROW == rc) ? true : false;

        return ret;
    }

    bool GetFunctionProfileData(
        AMDTFunctionId              functionId,
        gtUInt32                    funcStartOffset,
        AMDTProcessId               processId,
        AMDTThreadId                threadId,
        const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
        AMDTUInt64                  coreMask,
        bool                        separateByCore,
        AMDTProfileFunctionData&    functionData)
    {
        bool ret = false;
        bool isUnknownFunc = IS_UNKNOWN_FUNC(functionId);

        // function detailed data view
        ret = CreateFunctionDetailedDataView(functionId);

        if (ret)
        {
            //select SampleFunctionDetailedData.processId,
            //      SampleFunctionDetailedData.threadId,
            //      SampleFunctionDetailedData.offset,
            //      (sum(ifnull(e5, 0)) + sum(ifnull(e6, 0)) + sum(ifnull(e7, 0)) + sum(ifnull(e8, 0))) as etotal
            //from SampleFunctionDetailedData
            //group by offset;

            std::stringstream query;
            //query << "SELECT SampleFunctionDetailedData.processId, SampleFunctionDetailedData.threadId, SampleFunctionDetailedData.offset, ";

            query << "SELECT SampleFunctionDetailedData.offset, ";

            std::stringstream partialQuery;
            std::string firstCountColName;

            gtVector<AMDTSampleValue> sampleInfoVec;

            ret = GetEventCorePartialQuery(counterIdsList, coreMask, separateByCore, partialQuery, firstCountColName, sampleInfoVec);
            query << partialQuery.str();
            query << " FROM SampleFunctionDetailedData ";

            if (ret)
            {
                gtString partQuery;
                bool addedWhere = true;

                if (IS_PROCESS_THREAD_QUERY(processId, threadId))
                {
                    partQuery.appendFormattedString(L" WHERE processId = %d AND threadId = %d ", processId, threadId);
                }
                else if (IS_PROCESS_QUERY(processId))
                {
                    partQuery.appendFormattedString(L" WHERE processId = %d ", processId);
                }
                else if (IS_THREAD_QUERY(threadId))
                {
                    partQuery.appendFormattedString(L" WHERE threadId = %d ", threadId);
                }
                else
                {
                    addedWhere = false;
                }

                if (isUnknownFunc && (funcStartOffset > 0))
                {
                    if (addedWhere)
                    {
                        partQuery.appendFormattedString(L" AND offset = %d ", funcStartOffset);
                    }
                    else
                    {
                        partQuery.appendFormattedString(L" WHERE offset = %d ", funcStartOffset);
                    }
                }

                if (!partQuery.isEmpty())
                {
                    query << partQuery.asASCIICharArray();
                }

                query << " GROUP BY SampleFunctionDetailedData.offset ";

                //fprintf(stderr, " %s \n", query.str().c_str());

                sqlite3_stmt* pQueryStmt = nullptr;
                const std::string& queryStr = query.str();
                int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

                if (rc == SQLITE_OK)
                {
                    AMDTModuleId modId = (functionId & 0xFFFF0000) >> 16;
                    functionData.m_functionInfo.m_functionId = functionId;
                    functionData.m_functionInfo.m_moduleId = modId;

                    GetFunctionInfo(functionId, funcStartOffset, functionData.m_functionInfo);

                    gtVector<AMDTProfileModuleInfo> moduleInfoList;
                    bool isJitModule = false;
                    ret = GetModuleInfo(processId, modId, moduleInfoList);

                    if (ret && moduleInfoList.size() > 0)
                    {
                        isJitModule = ((AMDT_MODULE_TYPE_JAVA == moduleInfoList[0].m_type) || (AMDT_MODULE_TYPE_MANAGEDDPE == moduleInfoList[0].m_type))
                                            ? true : false;
                    }

                    if (!isJitModule)
                    {
                        // GetModuleBaseAddress(functionId, processId, functionData.m_modBaseAddress);
                        GetModuleBaseAddressByModuleId(modId, processId, functionData.m_modBaseAddress);
                    }
                    else
                    {
                        AMDTModuleId modInstId = (functionId & 0x0000FFFF);
                        GetModuleBaseAddressByModuleInstanceId(modInstId, processId, functionData.m_modBaseAddress);
                        functionData.m_functionInfo.m_startOffset = 0; // FIXME
                    }

                    // Execute the query.
                    while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                    {
                        AMDTProfileInstructionData instData;

                        AMDTUInt32 offset = sqlite3_column_int(pQueryStmt, 0);
                        instData.m_offset = offset;

                        int idx = 1;

                        for (auto& sample : sampleInfoVec)
                        {
                            AMDTSampleValue sampleValue;

                            sampleValue.m_sampleCount = sqlite3_column_int(pQueryStmt, idx);
                            sampleValue.m_counterId = sample.m_counterId;
                            sampleValue.m_coreId = sample.m_coreId;

                            idx++;
                            instData.m_sampleValues.emplace_back(sampleValue);
                        }

                        functionData.m_instDataList.emplace_back(instData);
                    }
                }

                // Finalize the statement.
                sqlite3_finalize(pQueryStmt);

                ret = (SQLITE_DONE == rc) ? true : false;
            }

            // Get the list of processes and threads for which this function has samples
            ret = ret && GetProcessAndThreadListForFunction(functionId,
                                                            counterIdsList,
                                                            coreMask,
                                                            functionData.m_pidsList,
                                                            functionData.m_threadsList);

            // Drop the view
            ret = DropFunctionDetailedDataView();
        }

        return ret;
    }

#if 0
    bool GetFunctionInfo(
        AMDTFunctionId              functionId,
        gtUInt32                    funcStartOffset,
        AMDTProfileFunctionData&    functionData)
    {
        bool ret = true;
        bool isUnknownFunc = IS_UNKNOWN_FUNC(functionId);

        if (!isUnknownFunc)
        {
            ret = GetFunctionInfo(functionId, funcStartOffset, functionData.m_functionInfo);
        }

        if (ret)
        {
            gtVector<AMDTProfileModuleInfo> moduleInfoList;
            AMDTModuleId modId = CXL_GET_DB_MODULE_ID(functionId);
            AMDTProcessId procId = AMDT_PROFILE_ALL_PROCESSES;

            bool isJitModule = false;
            ret = GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, modId, moduleInfoList);

            if (ret && moduleInfoList.size() > 0)
            {
                isJitModule = ((AMDT_MODULE_TYPE_JAVA == moduleInfoList[0].m_type) || (AMDT_MODULE_TYPE_MANAGEDDPE == moduleInfoList[0].m_type))
                    ? true : false;
            }

            if (!isJitModule)
            {
                GetModuleBaseAddressByModuleId(modId, procId, functionData.m_modBaseAddress);
            }
            else
            {
                AMDTModuleId modInstId = CXL_GET_DB_JIT_MODULE_INSTANCE_ID(functionId);
                GetModuleBaseAddressByModuleInstanceId(modInstId, procId, functionData.m_modBaseAddress);
                functionData.m_functionInfo.m_startOffset = 0;
            }
        }

        if (ret)
        {
            // Get the list of processes and threads for which this function has samples
            ret = ret && GetProcessAndThreadListForFunction(functionId,
                                                            funcStartOffset,
                                                            functionData.m_pidsList,
                                                            functionData.m_threadsList);
        }

        return ret;
    }
#endif //0

    // Query CallStackLeaf to get the unknown functions
    bool GetUnknownCallstackLeafsByProcessId(
        AMDTProcessId       processId,
        CallstackFrameVec&  leafs)
    {
        bool ret = false;

        std::stringstream query;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        query << "SELECT callstackId, functionId, offset, samplingConfigurationId, selfSamples "  \
                 "FROM  CallstackLeaf "     \
                 "WHERE processId = ? AND functionId & 0x0000ffff = 0 ;";
#else
        query << "SELECT callstackId, functionId, offset, samplingConfigurationId, selfSamples "  \
                 "FROM  CallstackLeaf "     \
                 "WHERE processId = ? AND (functionId & 65535) = 0 ;";
#endif

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, processId);

            // Execute the query.
            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                CallstackFrame aLeaf;

                aLeaf.m_callstackId = sqlite3_column_int(pQueryStmt, 0);
                AMDTFunctionId funcId = sqlite3_column_int(pQueryStmt, 1);
                gtUInt32 offset = sqlite3_column_int(pQueryStmt, 2);
                aLeaf.m_counterId = sqlite3_column_int(pQueryStmt, 3);
                aLeaf.m_selfSamples = sqlite3_column_int(pQueryStmt, 4);
                aLeaf.m_depth = 0;
                aLeaf.m_isLeaf = true;

                ConstructUnknownFunctionInfo(funcId, offset, aLeaf.m_funcInfo);
                GetModuleBaseAddressByModuleId(aLeaf.m_funcInfo.m_moduleId, processId, aLeaf.m_moduleBaseAddr);

                leafs.push_back(aLeaf);
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;
    }

    bool GetCallstackLeafData(
        AMDTProcessId       processId,
        AMDTUInt32          counterId,   // TODO: Is there a need to support ALL_COUNTERS?
        gtUInt32            callstackId, // AMDT_PROFILE_ALL_CALLPATHS
        AMDTFunctionId      funcId,
        gtUInt32            funcOffset,
        bool                isGroupByCSId,  // group by callstack id
        CallstackFrameVec&  leafs)
    {
        bool ret = false;

        ret = GetCallstackLeafData__(processId,
                                     counterId,
                                     callstackId,
                                     funcId,
                                     funcOffset,
                                     false,
                                     isGroupByCSId,
                                     leafs);

        ret = GetCallstackLeafData__(processId,
                                     counterId,
                                     callstackId,
                                     funcId,
                                     funcOffset,
                                     true,
                                     isGroupByCSId,
                                     leafs);

        return ret;
    }

    // Query CallStackLeaf
    bool GetCallstackLeafData__(
        AMDTProcessId       processId,
        AMDTUInt32          counterId,   // TODO: Is there a need to support ALL_COUNTERS?
        gtUInt32            callstackId, // AMDT_PROFILE_ALL_CALLPATHS
        AMDTFunctionId      functionId,
        gtUInt32            funcOffset,
        bool                queryUnknownFuncs,
        bool                isGroupByCSId,
        CallstackFrameVec&  leafs)
    {
        bool ret = false;

        std::stringstream query;

        if (!queryUnknownFuncs)
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            query << "SELECT callstackId, functionId, Module.isSystemModule, offset, SUM(selfSamples) "  \
                "FROM  CallstackLeaf "     \
                "INNER JOIN Module on ((CallstackLeaf.functionId & 0xFFFF0000) >> 16) = Module.id " \
                "WHERE processId = ? AND samplingConfigurationId = ? ";
#else
            query << "SELECT callstackId, functionId, Module.isSystemModule, offset, SUM(selfSamples) "  \
                "FROM  CallstackLeaf "     \
                "INNER JOIN Module on ((CallstackLeaf.functionId & 4294901760) >> 16) = Module.id " \
                "WHERE processId = ? AND samplingConfigurationId = ? ";
#endif


            if (IS_CALLSTACK_QUERY(callstackId))
            {
                query << "AND callstackId = ? ";
            }

            if (IS_FUNCTION_QUERY(functionId))
            {
                query << "AND functionId = ? ";
            }

            if (isGroupByCSId)
            {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                query << "GROUP BY callstackId HAVING (functionId & 0x0000ffff) > 0 ";
#else
                query << "GROUP BY callstackId HAVING (functionId & 65535) > 0 ";
#endif
            }
            else
            {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                query << "GROUP BY callstackId, functionId HAVING functionId & 0x0000ffff > 0 ";
#else
                query << "GROUP BY callstackId, functionId HAVING (functionId & 65535) > 0 ";
#endif
            }
        }
        else
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            query << "SELECT callstackId, functionId, Module.isSystemModule, offset, selfSamples "  \
                     "FROM  CallstackLeaf "     \
                     "INNER JOIN Module on ((CallstackLeaf.functionId & 0xFFFF0000) >> 16) = Module.id " \
                     "WHERE processId = ? AND samplingConfigurationId = ? AND (functionId & 0x0000ffff) = 0 ";
#else
            query << "SELECT callstackId, functionId, Module.isSystemModule, offset, selfSamples "  \
                     "FROM  CallstackLeaf "     \
                     "INNER JOIN Module on ((CallstackLeaf.functionId & 4294901760) >> 16) = Module.id " \
                     "WHERE processId = ? AND samplingConfigurationId = ? AND (functionId & 65535) = 0 ";
#endif

            if (IS_CALLSTACK_QUERY(callstackId))
            {
                query << " AND callstackId = ? ";
            }

            if (IS_FUNCTION_QUERY(functionId))
            {
                query << " AND functionId = ? AND offset = ? ";
            }
        }

        query << " ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, processId);
            sqlite3_bind_int(pQueryStmt, 2, counterId);

            if (IS_CALLSTACK_QUERY(callstackId))
            {
                sqlite3_bind_int(pQueryStmt, 3, callstackId);
            }

            if (IS_FUNCTION_QUERY(functionId))
            {
                sqlite3_bind_int(pQueryStmt, 4, functionId);

                if (queryUnknownFuncs)
                {
                    sqlite3_bind_int(pQueryStmt, 5, funcOffset);
                }
            }

            // Execute the query.
            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                CallstackFrame aLeaf;

                aLeaf.m_callstackId = sqlite3_column_int(pQueryStmt, 0);

                AMDTFunctionId funcId = sqlite3_column_int(pQueryStmt, 1);
                gtUInt32 isSysMod = sqlite3_column_int(pQueryStmt, 2);
                gtUInt32 offset = sqlite3_column_int(pQueryStmt, 3);

                double value = sqlite3_column_double(pQueryStmt, 4);
                aLeaf.m_selfSamples = static_cast<gtUInt32>(value);
                aLeaf.m_counterId = counterId;
                aLeaf.m_depth = 0;
                aLeaf.m_isLeaf = true;
                aLeaf.m_isSystemodule = (isSysMod == 1) ? true : false;

                if (queryUnknownFuncs)
                {
                    ConstructUnknownFunctionInfo(funcId, offset, aLeaf.m_funcInfo);
                    GetModuleBaseAddressByModuleId(aLeaf.m_funcInfo.m_moduleId, processId, aLeaf.m_moduleBaseAddr);
                }
                else
                {
                    GetFunctionInfo(funcId, offset, aLeaf.m_funcInfo);
                    GetModuleBaseAddress(funcId, processId, aLeaf.m_moduleBaseAddr);
                }

                leafs.push_back(aLeaf);
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;
    }

    // Query CallStackFrame
    bool GetCallstackFrameData(
        AMDTProcessId       processId,
        gtUInt32            callstackId,
        CallstackFrameVec&  frames,
        bool                ascendingOrder)
    {
        bool ret = false;

        std::stringstream query;
        query << "SELECT callstackId, functionId, offset, depth "  \
                 "FROM  CallstackFrame "     \
                 "WHERE callstackId = ? AND processId = ? ";

        if (ascendingOrder)
        {
            query << " ORDER BY depth ASC ;";
        }
        else
        {
            query << " ORDER BY depth DESC ;";
        }

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, callstackId);
            sqlite3_bind_int(pQueryStmt, 2, processId);

            // Execute the query.
            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                CallstackFrame aLeaf;

                aLeaf.m_callstackId = sqlite3_column_int(pQueryStmt, 0);
                AMDTFunctionId funcId  = sqlite3_column_int(pQueryStmt, 1);
                gtUInt32 offset = sqlite3_column_int(pQueryStmt, 2);

                aLeaf.m_depth = sqlite3_column_int(pQueryStmt, 3);
                aLeaf.m_selfSamples = 0;
                aLeaf.m_counterId = 0; // FIXME
                aLeaf.m_isLeaf = false;

                AMDTModuleId modId = ((funcId & 0xFFFF0000) >> 16);
                IsSystemModule(modId, aLeaf.m_isSystemodule);

                bool isUnknownFunc = ((funcId & 0x0000ffff) == 0) ? true : false;

                if (isUnknownFunc)
                {
                    ConstructUnknownFunctionInfo(funcId, offset, aLeaf.m_funcInfo);
                    GetModuleBaseAddressByModuleId(aLeaf.m_funcInfo.m_moduleId, processId, aLeaf.m_moduleBaseAddr);
                }
                else
                {
                    GetFunctionInfo(funcId, offset, aLeaf.m_funcInfo);
                    GetModuleBaseAddress(funcId, processId, aLeaf.m_moduleBaseAddr);
                }

                frames.push_back(aLeaf);
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);

        ret = (SQLITE_DONE == rc) ? true : false;

        return ret;
    }

    // Query CallStackPath
    // FIXME: This will not work for *unknown* functions
    // Note: Callstack and Callpath denotes the same
    bool GetCallstackIds (
        AMDTProcessId        processId,
        AMDTFunctionId       funcId,
        gtUInt32             funcOffset,
        bool                 isLeafEntries, // if true, fetch from the CallstackLeaf table, otherwise from CallstackFrame
        gtVector<gtUInt32>&  csIds)
    {
        bool ret = false;
        gtSet<gtUInt32> uniqueSet;
        gtUInt32 csId = 0;
        std::stringstream query;
        sqlite3_stmt* pQueryStmt = nullptr;
        int rc = 0;

        if (!isLeafEntries)
        {
            query << "SELECT DISTINCT callstackId "        \
                "FROM  CallstackFrame "     \
                "WHERE functionId = ? AND processId = ? ";

            if ((IS_UNKNOWN_FUNC(funcId)) && (funcOffset > 0))
            {
                query << " AND offset = ? ";
            }

            query << " ; ";

            const std::string& queryStr = query.str();
            rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                sqlite3_bind_int(pQueryStmt, 1, funcId);
                sqlite3_bind_int(pQueryStmt, 2, processId);

                if ((IS_UNKNOWN_FUNC(funcId)) && (funcOffset > 0))
                {
                    sqlite3_bind_int(pQueryStmt, 3, funcOffset);
                }

                // Execute the query.
                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                {
                    csId = sqlite3_column_int(pQueryStmt, 0);
                    uniqueSet.insert(csId);
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
        }
        else
        {
            // Recursive function which has selfSamples, there will be duplicate callstackIds.
            // Hence using unique set to avoid duplicate callstackIds
            query.str("");
            query << "SELECT DISTINCT callstackId "        \
                "FROM  CallstackLeaf "     \
                "WHERE functionId = ? AND processId = ? ;";

            if ((IS_UNKNOWN_FUNC(funcId)) && (funcOffset > 0))
            {
                query << " AND offset = ? ";
            }

            pQueryStmt = nullptr;
            const std::string& queryStr = query.str();
            rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

            if (rc == SQLITE_OK)
            {
                sqlite3_bind_int(pQueryStmt, 1, funcId);
                sqlite3_bind_int(pQueryStmt, 2, processId);

                if ((IS_UNKNOWN_FUNC(funcId)) && (funcOffset > 0))
                {
                    sqlite3_bind_int(pQueryStmt, 3, funcOffset);
                }

                // Execute the query.
                while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
                {
                    csId = sqlite3_column_int(pQueryStmt, 0);
                    uniqueSet.insert(csId);
                }
            }

            // Finalize the statement.
            sqlite3_finalize(pQueryStmt);
        }

        // TODO: Why do i need to use uniqueSet here, already only DISTINCT callstackids are returned
        // by db layer
        for (auto& id : uniqueSet)
        {
            csIds.push_back(id);
        }

        ret = (SQLITE_DONE == rc) ? true : false;
        return ret;
    }

    // Helpers
    bool GetMaxFunctionId(AMDTModuleId moduleId, gtUInt32& maxFuncId)
    {
        bool ret = false;
        gtUInt32 funcId = 0;

        std::stringstream query;
        query << "SELECT MAX(id) "        \
                 "FROM  Function "        \
                 "WHERE moduleId = ? ;";

        sqlite3_stmt* pQueryStmt = nullptr;
        const std::string& queryStr = query.str();
        int rc = sqlite3_prepare_v2(m_pReadDbConn, queryStr.c_str(), -1, &pQueryStmt, nullptr);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(pQueryStmt, 1, moduleId);

            // Execute the query.
            while ((rc = sqlite3_step(pQueryStmt)) == SQLITE_ROW)
            {
                funcId = sqlite3_column_int(pQueryStmt, 0);
            }
        }

        // Finalize the statement.
        sqlite3_finalize(pQueryStmt);

        maxFuncId = (funcId) ? funcId : (moduleId << 16);
        ret = (SQLITE_DONE == rc) ? true : false;
        return ret;
    }

    //
    // Data members of impl class
    //

    // A pointer to our parent.
    AmdtDatabaseAccessor* m_pParent = nullptr;
    // Profiler Type
    AMDTProfileMode m_profileType = AMDT_PROFILE_MODE_NONE;
    bool m_canUpdateDB = false;

    // These objects will be split into two different classes: reader and writer.
    sqlite3*      m_pWriteDbConn = nullptr;
    sqlite3_stmt* m_pValsInsertStmt = nullptr;
    sqlite3_stmt* m_pCounterEnabledAtInsertStmt = nullptr;
    sqlite3_stmt* m_pSamplingIntervalInsertStmt = nullptr;
    sqlite3_stmt* m_pDecivceInsertStmt = nullptr;
    sqlite3_stmt* m_pSubDevicesInsertStmt = nullptr;
    sqlite3_stmt* m_pCountersInsertStmt = nullptr;
    sqlite3_stmt* m_pSessionInfoInsertStmt = nullptr;

    sqlite3*      m_pReadDbConn = nullptr;
    sqlite3_stmt* m_pGetSessionRangeStmt = nullptr;
    sqlite3_stmt* m_pGetAllCountersStmt = nullptr;
    sqlite3_stmt* m_pGetCounterIdByNameStmt = nullptr;
    sqlite3_stmt* m_pGetDeviceTypeStmt = nullptr;
    sqlite3_stmt* m_pGetDeviceTypeIdStmt = nullptr;
    sqlite3_stmt* m_pGetDeviceTypeByCounterIdStmt = nullptr;
    sqlite3_stmt* m_pGetDeviceTypeIdByCounterIdStmt = nullptr;
    sqlite3_stmt* m_pGetSessionCountersByDeviceAndCategoryStmt = nullptr;
    sqlite3_stmt* m_pGetSessionCountersByDeviceAndCategoryIdsStmt = nullptr;
    sqlite3_stmt* m_pGetSessionCountersByCategoryStmt = nullptr;
    sqlite3_stmt* m_pGetSessionCountersByCategoryIdStmt = nullptr;
    sqlite3_stmt* m_pGetAllSessionCountersStmt = nullptr;
    sqlite3_stmt* m_pGetSessionInfoValueStmt = nullptr;
    sqlite3_stmt* m_pGetAllSessionInfoStmt = nullptr;
    sqlite3_stmt* m_pGetSessionSamplingIntervalStmt = nullptr;
    sqlite3_stmt* m_pGetSessionCounterIdsByNameStmt = nullptr;


    sqlite3_stmt* m_pCoreInfoInsertStmt = nullptr;
    sqlite3_stmt* m_pSamplingCounterInsertStmt = nullptr;
    sqlite3_stmt* m_pSamplingConfigInsertStmt = nullptr;
    sqlite3_stmt* m_pCoreSamplingConfigInsertStmt = nullptr;
    sqlite3_stmt* m_pProcessInfoInsertStmt = nullptr;
    sqlite3_stmt* m_pModuleInfoInsertStmt = nullptr;
    sqlite3_stmt* m_pModuleInstanceInsertStmt = nullptr;
    sqlite3_stmt* m_pProcessThreadInsertStmt = nullptr;
    sqlite3_stmt* m_pSampleContextInsertStmt = nullptr;
    sqlite3_stmt* m_pSampleContextUpdateStmt = nullptr;
    sqlite3_stmt* m_pFunctionInfoInsertStmt = nullptr;
    sqlite3_stmt* m_pModuleIdQueryStmt = nullptr;
    sqlite3_stmt* m_pSamplingConfigIdQueryStmt = nullptr;
    sqlite3_stmt* m_pCoreSamplingConfigIdQueryStmt = nullptr;
    sqlite3_stmt* m_pModuleInstanceIdQueryStmt = nullptr;
    sqlite3_stmt* m_pProcessThreadIdQueryStmt = nullptr;
    sqlite3_stmt* m_pFunctionIdQueryStmt = nullptr;
    sqlite3_stmt* m_pCallStackFrameInsertStmt = nullptr;
    sqlite3_stmt* m_pCallStackLeafInsertStmt = nullptr;
    sqlite3_stmt* m_pJitCodeBlobInsertStmt = nullptr;
    sqlite3_stmt* m_pJitInstanceInsertStmt = nullptr;

    // This thread is used to commit data to the database (which might take time).
    // As we would like to avoid stalls in the main thread.
    dbTxCommitThread* m_pDbTxCommitThread = nullptr;

    bool m_isFirstInsert = true;
    bool m_isCreateDb = false;
    bool m_isCurrentDbOpenForRead = false;
    int m_dbVersion = -1;

    gtMap<AMDTModuleId, AMDTProfileModuleInfo> m_moduleIdInfoMap;
};

AmdtDatabaseAccessor::AmdtDatabaseAccessor() : m_pImpl(new AmdtDatabaseAccessor::Impl(this))
{

}

AmdtDatabaseAccessor::~AmdtDatabaseAccessor()
{
    delete m_pImpl;
    m_pImpl = nullptr;
}

bool AmdtDatabaseAccessor::CreateProfilingDatabase(const gtString& dbName, gtUInt64 profileType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        std::string tmpUtf8Buffer;
        GT_IF_WITH_ASSERT(dbName.asUtf8(tmpUtf8Buffer) == 0)
        {
            ret = m_pImpl->CreateProfilingDatabase(tmpUtf8Buffer.c_str(), profileType);
        }
    }
    return ret;
}

bool AmdtDatabaseAccessor::OpenProfilingDatabase(const gtString& dbName, gtUInt64 profileType, bool isReadOnly)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        std::string tmpUtf8Buffer;
        GT_IF_WITH_ASSERT(dbName.asUtf8(tmpUtf8Buffer) == 0)
        {
            ret = m_pImpl->OpenProfilingDatabase(tmpUtf8Buffer.c_str(), profileType, isReadOnly);
        }
    }
    return ret;
}

bool AmdtDatabaseAccessor::MigrateProfilingDatabase(const gtString& dbName)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        std::string tmpUtf8Buffer;
        GT_IF_WITH_ASSERT(dbName.asUtf8(tmpUtf8Buffer) == 0)
        {
            ret = m_pImpl->MigrateProfilingDatabase(tmpUtf8Buffer.c_str());
        }
    }
    return ret;
}

bool AmdtDatabaseAccessor::PrepareProfilingDatabase()
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->PrepareProfilingDatabase();
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetDbVersion(int& version)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        m_pImpl->GetDbVersion(version);
        ret = true;
    }

    return ret;
}

bool AmdtDatabaseAccessor::FlushData()
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        m_pImpl->FlushData();
        ret = true;
    }

    return ret;
}

bool AmdtDatabaseAccessor::FlushDataAsync()
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->FlushDataAsync();
    }

    return ret;
}

bool AmdtDatabaseAccessor::CloseAllConnections()
{
    delete m_pImpl;
    m_pImpl = new Impl(this);
    return true;
}

bool AmdtDatabaseAccessor::InsertSessionInfoKeyValue(const gtString& key, const gtString& value)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertSessionInfoKeyValue(key, value);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertSessionInfo(const gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertSessionInfo(sessionInfoKeyValueVec);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertSamples(const gtVector<PPSampleData>& samples)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertSamples(samples);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertCounterControl(int counterId, int quantizedTime, std::string& action)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertCounterControl(counterId, quantizedTime, action);
    }
    return ret;
}

bool AmdtDatabaseAccessor::InsertSamplingInterval(unsigned samplingIntervalMs, unsigned quantizedTime)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertSamplingInterval(samplingIntervalMs, quantizedTime);
    }
    return ret;
}

bool AmdtDatabaseAccessor::InsertDevice(const AMDTProfileDevice& device)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertDevice(device);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertCounter(const AMDTProfileCounterDesc& counterDescription)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertCounter(counterDescription);
    }
    return ret;
}

bool AmdtDatabaseAccessor::InsertCoreInfo(gtUInt32 coreId, gtUInt32 processor, gtUInt32 numaNode)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertCoreinfo(coreId, processor, numaNode);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertSamplingCounter(gtUInt32 eventId, const gtString& name, const gtString& abbrev, const gtString& description)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        std::string nameAsUtf8Str;
        name.asUtf8(nameAsUtf8Str);

        std::string abbrevAsUtf8Str;
        abbrev.asUtf8(abbrevAsUtf8Str);

        std::string descriptionAsUtf8Str;
        description.asUtf8(descriptionAsUtf8Str);

        ret = m_pImpl->InsertSamplingCounter(eventId, nameAsUtf8Str, abbrevAsUtf8Str, descriptionAsUtf8Str);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertSamplingConfig(gtUInt32 id, gtUInt16 counterId, gtUInt64 samplingInterval, gtUInt16 unitMask, bool isUserMode, bool isOsMode, bool edge)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertSamplingConfig(id,
                                            counterId,
                                            samplingInterval,
                                            unitMask,
                                            (isUserMode ? 1 : 0),
                                            (isOsMode ? 1 : 0),
                                            (edge ? 1 : 0));
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertCoreSamplingConfig(gtUInt64 id, gtUInt16 coreId, gtUInt32 samplingConfigId)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertCoreSamplingConfig(id, coreId, samplingConfigId);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertProcessInfo(gtUInt64 pid, const gtString& path, bool is32Bit, bool hasCSS)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        std::string pathAsUtf8Str;
        path.asUtf8(pathAsUtf8Str);

        ret = m_pImpl->InsertProcessInfo(pid, pathAsUtf8Str, (is32Bit ? 1 : 0), (hasCSS ? 1 : 0));
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertModuleInfo(gtUInt32 id, const gtString& path, bool isSystemModule, bool is32Bit, gtUInt32 type, gtUInt32 size, bool foundDebugInfo)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        std::string pathAsUtf8Str;
        path.asUtf8(pathAsUtf8Str);

        ret = m_pImpl->InsertModuleInfo(id,
                                        pathAsUtf8Str,
                                        (isSystemModule ? 1 : 0),
                                        (is32Bit ? 1 : 0),
                                        type,
                                        size,
                                        (foundDebugInfo ? 1 : 0));
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertModuleInstanceInfo(gtUInt32 moduleInstanceId, gtUInt32 moduleId, gtUInt64 pid, gtUInt64 loadAddr)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertModuleInstanceInfo(moduleInstanceId, moduleId, pid, loadAddr);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertProcessThreadInfo(gtUInt64 id, gtUInt64 pid, gtUInt64 threadId)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertProcessThreadInfo(id, pid, threadId);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertSamples(const CPSampleData& sampleData)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertSamples(sampleData.m_processThreadId,
                                     sampleData.m_moduleInstanceId,
                                     sampleData.m_coreSamplingConfigId,
                                     sampleData.m_functionId,
                                     sampleData.m_offset,
                                     sampleData.m_count);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertFunction(gtUInt32 functionId, gtUInt32 moduleId, const gtString& funcName, gtUInt64 offset, gtUInt64 size)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        std::string funcNameAsUtf8Str;
        funcName.asUtf8(funcNameAsUtf8Str);
        ret = m_pImpl->InsertFunctionInfo(functionId, moduleId, funcNameAsUtf8Str, offset, size);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertCallStackFrame(gtUInt32 callStackId, gtUInt64 processId, gtUInt64 funcId, gtUInt64 offset, gtUInt16 depth)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertCallStackFrame(callStackId, processId, funcId, offset, depth);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertCallStackLeaf(gtUInt32 callStackId, gtUInt64 processId, gtUInt64 funcId, gtUInt64 offset, gtUInt32 counterId, gtUInt64 selfSamples)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertCallStackLeaf(callStackId, processId, funcId, offset, counterId, selfSamples);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertJitInstance(gtUInt32 jitId, gtUInt64 functionId, gtUInt64 pid, gtUInt64 loadAddr, gtUInt32 size)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertJitInstanceInfo(jitId, functionId, pid, loadAddr, size);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertJitCodeBlob(gtUInt32 id, const gtString& sourceFilePath, const gtString& jncFilePath)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        std::string srcFileNameAsUtf8Str;
        sourceFilePath.asUtf8(srcFileNameAsUtf8Str);

        std::string jncFilePathAsUtf8Str;
        jncFilePath.asUtf8(jncFilePathAsUtf8Str);

        ret = m_pImpl->InsertJitCodeBlobInfo(id, srcFileNameAsUtf8Str, jncFilePathAsUtf8Str);
    }

    return ret;
}


//
//  Update APIs
//

bool AmdtDatabaseAccessor::UpdateDeviceTypeId(const gtMap<gtString, int>& deviceInfo)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateTablesForVersion1(0, deviceInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::UpdateCounterCategoryId(const gtMap<gtString, int>& counterInfo)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateTablesForVersion1(1, counterInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::UpdateCounterAggregationId(const gtMap<gtString, int>& counterInfo)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateTablesForVersion1(2, counterInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::UpdateCounterUnitId(const gtMap<gtString, int>& counterInfo)
{
    bool ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateTablesForVersion1(3, counterInfo);
    }

    return ret;
}

//
//  Query APIs
//

bool AmdtDatabaseAccessor::GetSamplesTimeRange(SamplingTimeRange& samplingTimeRange)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSamplesTimeRange(samplingTimeRange);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetSampleCountByCounterId(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSampleCountByCounterId(counterIds, numberOfSamplesPerCounter);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetSamplesByCounterIdAndRange(const gtVector<int>& counterIds, const SamplingTimeRange& samplingTimeRange,
                                                         gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSamplesByCounterIdAndRange(counterIds, samplingTimeRange, sampledValuesPerCounter);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetMinMaxSampleByCounterId(const gtVector<int>& counterIds, const SamplingTimeRange& samplingTimeRange, double& minValue, double& maxValue)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetMinMaxSampleByCounterId(counterIds, samplingTimeRange, minValue, maxValue);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetBucketizedSamplesByCounterId(unsigned int bucketWidth, const gtVector<int>& counterIds, gtVector<int>& cIds, gtVector<double>& dbBucketBottoms, gtVector<int>& dbBucketCount)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetBucketizedSamplesByCounterId(bucketWidth, counterIds, cIds, dbBucketBottoms, dbBucketCount);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetDeviceType(int deviceId, std::string& deviceType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetDeviceType(deviceId, deviceType);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetDeviceType(int deviceId, int& deviceType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetDeviceType(deviceId, deviceType);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetSamplingInterval(unsigned& samplingIntervalMs)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSamplingInterval(samplingIntervalMs);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCounterIdByName(const std::string& counterName, int& counterId)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCounterIdByName(counterName, counterId);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCounterNames(gtMap<gtString, int>& counterNames)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCounterNames(counterNames);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCountersDescription(gtMap<int, AMDTProfileCounterDesc*>& counterDetails)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCountersDescription(counterDetails);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetDeviceTypeByCounterId(int counterId, std::string& deviceType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetDeviceTypeByCounterId(counterId, deviceType);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetDeviceTypeByCounterId(int counterId, int& deviceType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetDeviceTypeByCounterId(counterId, deviceType);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetSessionInfoValue(const gtString& key, gtString& infoValue)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSessionInfoValue(key, infoValue);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetAllSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetAllSessionInfo(sessionInfoKeyValueVec);
    }
    return ret;
}


bool AmdtDatabaseAccessor::GetSamplesGroupByCounterId(const gtVector<int>& counterIds, gtMap<int, double>& consumptionPerCounterId)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSamplesGroupByCounterId(counterIds, consumptionPerCounterId);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCountersByCategory(const std::string& counterCategoryStr, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCountersByCategory(counterCategoryStr, counterIds);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCountersByCategory(int counterCategoryId, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCountersByCategory(counterCategoryId, counterIds);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCountersByDeviceAndCategory(const std::string& deviceTypeStr, const std::string& counterCategoryStr, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCountersByDeviceAndCategory(deviceTypeStr, counterCategoryStr, counterIds);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCountersByDeviceAndCategory(int deviceTypeId, int counterCategoryId, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCountersByDeviceAndCategory(deviceTypeId, counterCategoryId, counterIds);
    }
    return ret;
}

bool AmdtDatabaseAccessor::GetCpuTopology(gtVector<AMDTCpuTopology>& cpuTopology)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCpuTopology(cpuTopology);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetSampledCountersList(gtVector<AMDTProfileCounterDesc>& counterDesc)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSampledCountersList(counterDesc);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetSamplingConfiguration(AMDTUInt32 counterId, AMDTProfileSamplingConfig& pCounterDesc)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetSamplingConfiguration(counterId, pCounterDesc);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetProcessesWithCallstackSamples(gtVector<AMDTProcessId>& cssProcessVec)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetProcessesWithCallstackSamples(cssProcessVec);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetProcessTotals(
    AMDTProcessId               procId,
    const gtVector<AMDTUInt32>& counterIdsList,
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    AMDTSampleValueVec&         sampleValueVec)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetProcessTotals(procId, counterIdsList, coreMask, separateByCore, sampleValueVec);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetModuleTotals(
    AMDTModuleId                moduleId,
    AMDTProcessId               processId,
    const gtVector<AMDTUInt32>& counterIdsList,
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    AMDTSampleValueVec&         sampleValueVec)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetModuleTotals(moduleId, processId, counterIdsList, coreMask, separateByCore, sampleValueVec);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetFunctionTotals(
    AMDTFunctionId         funcId,
    AMDTProcessId          processId,
    AMDTThreadId           threadId,
    const gtVector<AMDTUInt32>&  counterIdsList,
    AMDTUInt64             coreMask,
    bool                   separateByCore,
    AMDTSampleValueVec&    sampleValueVec)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetFunctionTotals(funcId, processId, threadId, counterIdsList, coreMask, separateByCore, sampleValueVec);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetProcessInfo(AMDTUInt32 pid, gtVector<AMDTProfileProcessInfo>& processInfoList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetProcessInfo(pid, processInfoList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, gtVector<AMDTProfileModuleInfo>& moduleInfoList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetModuleInfo(pid, mid, moduleInfoList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetThreadInfo(AMDTUInt32 pid, gtUInt32 tid, gtVector<AMDTProfileThreadInfo>& threadInfoList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetThreadInfo(pid, tid, threadInfoList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetFunctionInfoByModuleId(AMDTModuleId moduleId, AMDTProfileFunctionInfoVec& funcInfoVec)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetFunctionInfoByModuleId(moduleId, funcInfoVec);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetFunctionInfo(
    AMDTFunctionId              functionId,
    gtUInt32                    funcStartOffset,
    AMDTProfileFunctionInfo&    functionInfo)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetFunctionInfo(functionId, funcStartOffset, functionInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetProcessAndThreadListForFunction(
    AMDTFunctionId              funcId,
    AMDTUInt32                  funcStartOffset,
    gtVector<AMDTProcessId>&    processList,
    gtVector<AMDTThreadId>&     threadList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetProcessAndThreadListForFunction(funcId, funcStartOffset, processList, threadList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetProcessSummaryData(
    AMDTProcessId               processId,
    AMDTModuleId                moduleId,
    const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    bool                        doSort,
    size_t                      count,
    gtVector<AMDTProfileData>&  dataList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetProcessSummaryData(processId,
                                             moduleId,
                                             counterIdsList,
                                             coreMask,
                                             separateByCore,
                                             doSort,
                                             count,
                                             dataList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetModuleSummaryData(
    AMDTProcessId               processId,           // for a given process or for all processes
    AMDTModuleId                moduleId,
    const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
    AMDTUInt64                  coreMask,
    bool                        ignoreSystemModules,
    bool                        separateByCore,
    bool                        doSort,
    size_t                      count,
    gtVector<AMDTProfileData>&  dataList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetModuleSummaryData(moduleId,
                                            processId,
                                            counterIdsList,
                                            coreMask,
                                            ignoreSystemModules,
                                            separateByCore,
                                            doSort,
                                            count,
                                            dataList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetThreadSummaryData(
    AMDTProcessId               processId,           // for a given process or for all processes
    AMDTThreadId                threadId,
    const gtVector<AMDTUInt32>& counterIdsList,      // samplingConfigId
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    bool                        doSort,
    size_t                      count,
    gtVector<AMDTProfileData>&  dataList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetThreadSummaryData(processId,
                                            threadId,
                                            counterIdsList,
                                            coreMask,
                                            separateByCore,
                                            doSort,
                                            count,
                                            dataList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetFunctionSummaryData(
    AMDTProcessId               processId,           // for a given process or for all processes
    AMDTThreadId                threadId,
    AMDTModuleId                moduleId,
    gtVector<AMDTUInt32>&       counterIdsList,      // samplingConfigId
    AMDTUInt64                  coreMask,
    bool                        ignoreSystemModules,
    bool                        separateByCore,
    bool                        separateByProcess,
    bool                        doSort,
    size_t                      count,
    gtVector<AMDTProfileData>&  dataList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetFunctionSummaryData(processId,
                                              threadId,
                                              moduleId,
                                              counterIdsList,
                                              coreMask,
                                              ignoreSystemModules,
                                              separateByCore,
                                              separateByProcess,
                                              doSort,
                                              count,
                                              dataList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetJITFunctionInfo(AMDTFunctionId funcId, gtUInt64& loadAddr, gtString& srcFilePath, gtString& jncFilePath)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetJITFunctionInfo(funcId,
                                          loadAddr,
                                          srcFilePath,
                                          jncFilePath);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetFunctionProfileData(
    AMDTFunctionId              funcId,
    gtUInt32                    funcStartOffset,
    AMDTProcessId               processId,
    AMDTThreadId                threadId,
    const gtVector<AMDTUInt32>& counterIdsList,
    AMDTUInt64                  coreMask,
    bool                        separateByCore,
    AMDTProfileFunctionData&    functionData)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetFunctionProfileData(funcId,
                                              funcStartOffset,
                                              processId,
                                              threadId,
                                              counterIdsList,
                                              coreMask,
                                              separateByCore,
                                              functionData);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetUnknownFunctionsByIPSamples(AMDTProfileFunctionInfoVec& funcList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetUnknownFunctionsByIPSamples(funcList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetUnknownFunctions(gtVector<AMDTProfileFunctionInfo>& funcList)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetUnknownFunctions(funcList);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetUnknownCallstackLeafsByProcessId(AMDTProcessId       processId,
                                                               CallstackFrameVec&  leafs)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetUnknownCallstackLeafsByProcessId(processId, leafs);
    }

    return ret;
}

// Query CallStackLeaf
bool AmdtDatabaseAccessor::GetCallstackLeafData(AMDTProcessId       processId,
                                                AMDTCounterId       counterId,
                                                gtUInt32            callStackId,
                                                AMDTFunctionId      funcId,
                                                gtUInt32            funcOffset,
                                                bool                groupByCSId,
                                                CallstackFrameVec&  leafs)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCallstackLeafData(processId, counterId, callStackId, funcId, funcOffset, groupByCSId, leafs);
    }

    return ret;
}

// Query CallStackFrame to retrieve the callpath for the given callstackIdx
bool AmdtDatabaseAccessor::GetCallstackFrameData(AMDTProcessId       processId,
                                                 gtUInt32            callstackId,
                                                 CallstackFrameVec&  frames,
                                                 bool                ascendingOrder)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCallstackFrameData(processId, callstackId, frames, ascendingOrder);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetCallstackIds(AMDTProcessId        processId,
                                           AMDTFunctionId       funcId,
                                           gtUInt32             funcOffset,
                                           bool                 isLeafEntries,
                                           gtVector<gtUInt32>&  csIds)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetCallstackIds(processId, funcId, funcOffset, isLeafEntries, csIds);
    }

    return ret;
}

bool AmdtDatabaseAccessor::UpdateIPSample(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateIPSample(funcInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::UpdateCallstackLeaf(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateCallstackLeaf(funcInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::UpdateCallstackFrame(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->UpdateCallstackFrame(funcInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::InsertFunctionInfo(const AMDTProfileFunctionInfo& funcInfo)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->InsertFunctionInfo(funcInfo);
    }

    return ret;
}

bool AmdtDatabaseAccessor::GetMaxFunctionId(AMDTModuleId moduleId, gtUInt32& maxFuncId)
{
    bool ret = false;

    if (m_pImpl != nullptr)
    {
        ret = m_pImpl->GetMaxFunctionId(moduleId, maxFuncId);
    }

    return ret;
}

}
