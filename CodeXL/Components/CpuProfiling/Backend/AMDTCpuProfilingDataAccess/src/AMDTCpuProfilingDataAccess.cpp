//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief AMDTProfilerDataAccess.cpp - APIs used to access the profile data stored in the db.
//
//=============================================================

#include "AMDTCpuProfilingDataAccess.h"
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

//
//  Data Structures
//

struct amdtProfileDataReader
{
    amdtProfileDbAdapter*               m_pDbAdapter = nullptr;
    gtString                            m_dbPathStr;

    gtVector<gtString>                  m_symbolServerPath;
    gtVector<gtString>                  m_symbolFilePath;
    gtVector<gtString>                  m_sourceFilePath;

    gtString                            m_reportConfig; // by default "All Data"

    gtVector<AMDTProfileCounterDesc>    m_counterDescVec;

    AMDTProfileDataOptions              m_options;

    void clear(void)
    {
        if (nullptr != m_pDbAdapter)
        {
            m_pDbAdapter->CloseDb();
            delete m_pDbAdapter;
            m_pDbAdapter = nullptr;
        }

        m_dbPathStr.makeEmpty();

        m_symbolServerPath.clear();
        m_symbolFilePath.clear();
        m_sourceFilePath.clear();

        m_reportConfig.makeEmpty();

        m_counterDescVec.clear();

        m_options.Clear();
    };
};

//
//  Macros
//

#define AMDT_GET_PROFILE_DATA_READER(readerHandle_)                                                     \
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;                                                          \
    amdtProfileDataReader* pDataReader = reinterpret_cast<amdtProfileDataReader*>(readerHandle);        \
    if (nullptr != pDataReader)                                                                         \


#define AMDT_GET_PROFILE_DB_ADAPTER(readerHandle_)                                                     \
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;                                                          \
    amdtProfileDataReader* pDataReader = reinterpret_cast<amdtProfileDataReader*>(readerHandle);        \
    amdtProfileDbAdapter* pDbAdapter = (nullptr != pDataReader) ? pDataReader->m_pDbAdapter : nullptr;  \
    if (nullptr != pDataReader && nullptr != pDbAdapter)                                                \

        //
        //  APIs
        //

        // profile file can either be raw PRD file or processed DB file
        AMDTResult AMDTOpenProfileData(gtString profileFilePath, AMDTProfileReaderHandle& readerHandle)
    {
        AMDTResult retVal = AMDT_ERROR_FAIL;
        amdtProfileDataReader* pDataReader = new amdtProfileDataReader;
        amdtProfileDbAdapter* pDbAdapter = nullptr;
        bool rc = false;

        if (nullptr != pDataReader)
        {
            pDataReader->m_dbPathStr = profileFilePath;
            pDbAdapter = new amdtProfileDbAdapter;

            if (nullptr != pDbAdapter)
            {
                rc = pDbAdapter->OpenDb(profileFilePath, AMDT_PROFILE_MODE_AGGREGATION);
            }
        }

        if (rc)
        {
            pDataReader->m_pDbAdapter = pDbAdapter;
            readerHandle = reinterpret_cast<AMDTProfileReaderHandle>(pDataReader);
            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult AMDTCloseProfileData(AMDTProfileReaderHandle readerHandle)
    {
        AMDT_GET_PROFILE_DATA_READER(readerHandle)
        {
            pDataReader->clear();

            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult AMDTGetProfileSessionInfo(AMDTProfileReaderHandle readerHandle, AMDTProfileSessionInfo& sessionInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetSessionInfo(sessionInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }


    AMDTResult AMDTGetCpuTopology(AMDTProfileReaderHandle readerHandle, gtVector<AMDTCpuTopology>& cpuToplogy)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetCpuTopology(cpuToplogy) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;
        }

        return retVal;
    }

    // Returns the information about the counters used for sampling to collect profile data.
    // Does NoT return the calculated counetrs like CPI, IPC etc
    AMDTResult AMDTGetSampledCountersList(AMDTProfileReaderHandle readerHandle,
                                          gtVector<AMDTProfileCounterDesc>& counterDesc)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetSampledCountersList(counterDesc) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;
        }

        return retVal;
    }

    AMDTResult AMDTGetSamplingConfiguration(AMDTProfileReaderHandle readerHandle,
                                            AMDTUInt32 counterId,
                                            AMDTProfileSamplingConfig& sampleConfig)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetSamplingConfiguration(counterId, sampleConfig) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;
        }

        return retVal;
    }

#if 0
    AMDTResult AMDTGetReportConfigurations(AMDTProfileReaderHandle readerHandle,
                                           gtVector<AMDTProfileReportConfig>& viewConfigDesc)
    {
        UNREFERENCED_PARAMETER(viewConfigDesc);

        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            //retVal = pDbAdapter->GetSamplingConfiguration(counterId, sampleConfig) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;
        }

        return retVal;
    }
#endif //0

    AMDTResult AMDTSetDebugInfoPaths(AMDTProfileReaderHandle readerHandle,
                                     gtVector<gtString>& symbolServer,
                                     gtVector<gtString>& symbolDirectory)
    {
        AMDT_GET_PROFILE_DATA_READER(readerHandle)
        {
            pDataReader->m_symbolServerPath  = symbolServer;
            pDataReader->m_symbolFilePath    = symbolDirectory;

            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult AMDTSetSourcePaths(AMDTProfileReaderHandle readerHandle,
                                  gtVector<gtString>& sourceDirPath)
    {
        AMDT_GET_PROFILE_DATA_READER(readerHandle)
        {
            pDataReader->m_sourceFilePath = sourceDirPath;
            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    // If counter list is empty all the counters pertaining to to the view-config will be selected
    AMDTResult AMDTSetReportOptions(AMDTProfileReaderHandle readerHandle,
                                    AMDTProfileDataOptions& options)
    {
        AMDT_GET_PROFILE_DATA_READER(readerHandle)
        {
            // TODO: Hmm.. Write a  copy ctor for AMDTProfileDataOptions
            if (options.m_counters.size() > 0)
            {
                pDataReader->m_options.m_counters = options.m_counters;
            }

            pDataReader->m_options.m_coreMask = options.m_coreMask;
            pDataReader->m_options.m_isSeperateByCore = options.m_isSeperateByCore;
            pDataReader->m_options.m_ignoreSystemModules = options.m_ignoreSystemModules;
            pDataReader->m_options.m_doSort = options.m_doSort;
            pDataReader->m_options.m_summaryCount = options.m_summaryCount;

            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    AMDTResult AMDTSetReportOption(AMDTProfileReaderHandle readerHandle,
                                   AMDTReportOptionType type,
                                   gtUInt64 value)
    {
        AMDT_GET_PROFILE_DATA_READER(readerHandle)
        {
            retVal = AMDT_STATUS_OK;

            switch (type)
            {
                case AMDT_REPORT_OPTION_COREMASK:
                    pDataReader->m_options.m_coreMask = value;
                    break;

                case AMDT_REPORT_OPTION_SEP_BY_CORE:
                    pDataReader->m_options.m_isSeperateByCore = (value == 1ULL) ? true : false;
                    break;

                case AMDT_REPORT_OPTION_IGNORE_SYSTEM_MODULE:
                    pDataReader->m_options.m_ignoreSystemModules = (value == 1ULL) ? true : false;
                    break;

                case AMDT_REPORT_OPTION_SORT_PROFILE_DATA:
                    pDataReader->m_options.m_doSort = (value == 1ULL) ? true : false;
                    break;

                case AMDT_REPORT_OPTION_SUMMARY_COUNT:
                    pDataReader->m_options.m_summaryCount = static_cast<size_t>(value);
                    break;

                default:
                    retVal = AMDT_ERROR_INVALIDARG;
                    break;
            }
        }

        return retVal;
    }

    // If counter list is empty all the counters pertaining to to the view-config will be selected
    AMDTResult AMDTSetReportCounters(AMDTProfileReaderHandle readerHandle,
                                     gtVector<AMDTUInt32> countersList)
    {
        AMDT_GET_PROFILE_DATA_READER(readerHandle)
        {
            if (!countersList.empty())
            {
                pDataReader->m_options.m_counters = countersList;
            }

            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }

    // TBD: Should we have
    //      AMDTGetProcessData(hdl, pid, vector<procinfo>); pid == -1 means all the process
    //              (or)
    //      AMDTGetAllProcessData(hdl, vector<procinfo>)
    //      AMDTGetProcessData(hdl, pid, procinfo)
    //
    AMDTResult AMDTGetProcessInfo(AMDTProfileReaderHandle readerHandle,
                                  gtVector<AMDTProfileProcessInfo>& procInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetProcessInfo(AMDT_PROFILE_ALL_PROCESSES, procInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetModuleInfo(AMDTProfileReaderHandle readerHandle,
                                 gtVector<AMDTProfileModuleInfo>& modInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, modInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetThreadInfo(AMDTProfileReaderHandle readerHandle,
                                 gtVector<AMDTProfileThreadInfo>& threadInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetThreadInfo(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_THREADS, threadInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    // UNUSED ?? or replaced  by AMDTGetProcessData?
#if 0
    AMDTResult AMDTGetProcessInfo(AMDTProfileReaderHandle readerHandle,
                                  AMDTUInt32 pid,
                                  gtVector<AMDTProfileProcessInfo>& procInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetProcessInfo(pid, procInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetModuleInfo(AMDTProfileReaderHandle readerHandle,
                                 AMDTUInt32 pid,
                                 AMDTModuleId modId,
                                 gtVector<AMDTProfileModuleInfo>& modInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetModuleInfo(pid, modId, modInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetThreadInfo(AMDTProfileReaderHandle readerHandle,
                                 AMDTUInt32 pid,
                                 AMDTThreadId threadId,
                                 gtVector<AMDTProfileThreadInfo>& threadInfo)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetThreadInfo(pid, threadId, threadInfo) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }
#endif // 0

    AMDTResult AMDTGetProcessSummary(AMDTProfileReaderHandle readerHandle,
                                     AMDTUInt32 counterId,      // This sampleConfigId
                                     size_t count,
                                     gtVector<AMDTProfileData>& processProfileData)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetProfileData(AMDT_PROFILE_DATA_PROCESS,
                                                AMDT_PROFILE_ALL_PROCESSES,
                                                AMDT_PROFILE_ALL_MODULES,
                                                AMDT_PROFILE_ALL_THREADS,
                                                counterId,
                                                AMDT_PROFILE_ALL_CORES,
                                                false, // separateByCore
                                                false,  // separateByProcess
                                                true,   // doSort
                                                count,
                                                processProfileData) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetThreadSummary(AMDTProfileReaderHandle readerHandle,
                                    AMDTUInt32 counterId,      // This sampleConfigId
                                    size_t count,
                                    gtVector<AMDTProfileData>& processProfileData)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetProfileData(AMDT_PROFILE_DATA_THREAD,
                                                AMDT_PROFILE_ALL_PROCESSES,
                                                AMDT_PROFILE_ALL_MODULES,
                                                AMDT_PROFILE_ALL_THREADS,
                                                counterId,
                                                AMDT_PROFILE_ALL_CORES,
                                                false, // separateByCore
                                                false,  // separateByProcess
                                                true,   // doSort
                                                count,
                                                processProfileData) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetModuleSummary(AMDTProfileReaderHandle readerHandle,
                                    AMDTUInt32 counterId,      // This sampleConfigId
                                    size_t count,
                                    gtVector<AMDTProfileData>& processProfileData)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetProfileData(AMDT_PROFILE_DATA_MODULE,
                                                AMDT_PROFILE_ALL_PROCESSES,
                                                AMDT_PROFILE_ALL_MODULES,
                                                AMDT_PROFILE_ALL_THREADS,
                                                counterId,
                                                AMDT_PROFILE_ALL_CORES,
                                                false, // separateByCore
                                                false,  // separateByProcess
                                                true,   // doSort
                                                count,
                                                processProfileData) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetFunctionSummary(AMDTProfileReaderHandle readerHandle,
                                      AMDTUInt32 counterId,      // This sampleConfigId
                                      size_t count,
                                      gtVector<AMDTProfileData>& processProfileData)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetProfileData(AMDT_PROFILE_DATA_FUNCTION,
                                                AMDT_PROFILE_ALL_PROCESSES,
                                                AMDT_PROFILE_ALL_MODULES,
                                                AMDT_PROFILE_ALL_THREADS,
                                                counterId,
                                                AMDT_PROFILE_ALL_CORES,
                                                false, // separateByCore
                                                false,  // separateByProcess
                                                true,   // doSort
                                                count,
                                                processProfileData) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetFunctionProfileData(AMDTProfileReaderHandle   readerHandle,
                                          AMDTFunctionId            funcId,
                                          AMDTProcessId             processId,
                                          AMDTThreadId              threadId,
                                          AMDTProfileFunctionData&  functionData)
    {
        AMDT_GET_PROFILE_DB_ADAPTER(readerHandle)
        {
            retVal = pDbAdapter->GetFunctionProfileData(funcId,
                                                        processId,
                                                        threadId,
                                                        pDataReader->m_options.m_counters,
                                                        pDataReader->m_options.m_coreMask,
                                                        pDataReader->m_options.m_isSeperateByCore,
                                                        functionData) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
        }

        return retVal;
    }

    AMDTResult AMDTGetFunctionDisassembly(AMDTProfileReaderHandle readerHandle,
                                          AMDTFunctionId functionId,
                                          AMDTUInt32 lineNbr,
                                          gtVector<gtString>& disasmString)
    {
        UNREFERENCED_PARAMETER(functionId);
        UNREFERENCED_PARAMETER(lineNbr);
        UNREFERENCED_PARAMETER(disasmString);
        AMDTResult retVal = AMDT_ERROR_INVALIDARG;
        amdtProfileDataReader* pDataReader = nullptr;
        //bool rc = false;

        pDataReader = reinterpret_cast<amdtProfileDataReader*>(readerHandle);

        if (nullptr != pDataReader)
        {
            amdtProfileDbAdapter* pDbAdapter = pDataReader->m_pDbAdapter;

            if (nullptr != pDbAdapter)
            {
                //rc = pDbAdapter->GetThreadInfo(pid, procInfo);
            }

            retVal = AMDT_STATUS_OK;
        }

        return retVal;
    }
