//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTDbAdapter.h
///
//==================================================================================

#ifndef _AMDTDBADAPTER_H_
#define _AMDTDBADAPTER_H_

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>

#include <AMDTProfilerDAL/include/AMDTDatabaseAccessor.h>

#if defined(_WIN32)
    #if defined(AMDTDBADAPTER_EXPORTS)
        #define AMDTDBADAPTER_API __declspec(dllexport)
    #else
        #define AMDTDBADAPTER_API __declspec(dllimport)
    #endif
#else
    #define AMDTDBADAPTER_API
#endif

// DB file name extension.
const gtString CPUP_DB_FILE_EXTENSION = L".cxlcpdb";
const gtString PWRP_DB_FILE_EXTENSION = L".cxldb";

class AMDTDBADAPTER_API AmdtDatabaseAdapter
{
public:
    AmdtDatabaseAdapter();
    virtual ~AmdtDatabaseAdapter();

    // Prevent copy operation
    AmdtDatabaseAdapter(const AmdtDatabaseAdapter& other) = delete;

    // Prevent assignment operation
    AmdtDatabaseAdapter& operator=(const AmdtDatabaseAdapter& other) = delete;

    // Creates a DB to store profile data
    virtual bool CreateDb(const gtString& dbName, AMDTProfileMode profileMode);

    // Open the DB
    virtual bool OpenDb(const gtString& dbName, AMDTProfileMode profileMode, bool isReadOnly = true);

    // Open the older db for migration so that the latest codexl version will
    // support importing data from older profile dbs
    virtual bool MigrateDb(const gtString& dbName);

    // Closes the DB connection
    virtual bool CloseDb();

    // Commits all pending transactions to the DB.
    virtual bool FlushDb();

    // Commits all pending transactions to the DB asynchronously
    virtual bool FlushDbAsync();

    // Prepare the required views for aggregate profile modes
    // Should be called after OpenDb();
    virtual bool PrepareDb();

    // Get DB file extension
    virtual bool GetDbFileExtension(AMDTProfileMode profileMode, gtString& extension) const;

    virtual bool InsertSessionInfoKeyValue(const gtString& key, gtString& value);

    virtual bool InsertSessionInfo(const gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec);

    // TODO: This should be deprecated to use InsertSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
    virtual bool InsertSessionInfo(const AMDTProfileSessionInfo& sessionInfo);

    // Common Query APIs
    virtual bool GetSessionInfoValue(const gtString& key, gtString& value);

    virtual bool GetAllSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec);

    // TODO: This should be deprecated to use GetAllSessionInfo(gtVector<std::pair<gtString, gtString>>& sessionInfoKeyValueVec)
    virtual bool GetSessionInfo(AMDTProfileSessionInfo& sessionInfo);

    bool GetDbVersion(int& version);

    int GetSupportedDbVersion(void);

protected:
    // The adapter to the DB.
    AMDTProfilerDAL::AmdtDatabaseAccessor* m_pDbAccessor;

    // If the database needs to be closed.
    bool                m_isDbToBeClosed;
    AMDTProfileMode     m_profileMode;

    //
    // Helper Functions
    //

    bool IsTimelineMode();
    bool IsAggregateMode();
    bool InsertSessionConfiguration(const AMDTProfileSessionInfo& sessionInfo);
    bool GetSessionConfiguration(AMDTProfileSessionInfo& sessionInfo);
};

#endif // _AMDTDBADAPTER_H_
