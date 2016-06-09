//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief AMDTProfilerDataAccess.cpp - APIs used to access the profile data stored in the db.
//
//=============================================================

#include <QtGui>

#include <string.h>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtSet.h>

#include "AMDTCpuProfilingDataAccess.h"
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuPerfEventUtils/inc/DcConfig.h>
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osCpuid.h>

#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <AMDTExecutableFormat/inc/SymbolEngine.h>
#include <AMDTDisassembler/inc/LibDisassembler.h>

//
//  Data Structures
//

struct ViewConfigInfo
{
    ViewConfig m_viewCfg;
    gtVector<AMDTCounterId>  m_counterIdVec;
};

//
//  Macros
//

#define CXL_REPORT_VIEW_ALL_DATA    "All Data"
#define CXL_REPORT_VIEW_ALL_DATAW   L"All Data"

#define IS_PROCESS_MODULE_QUERY(procId_, modId_)        ((procId_ != AMDT_PROFILE_ALL_PROCESSES) && (modId_ != AMDT_PROFILE_ALL_MODULES))
#define IS_PROCESS_QUERY(procId_)                       (procId_ != AMDT_PROFILE_ALL_PROCESSES)
#define IS_MODULE_QUERY(modId_)                         (modId_ != AMDT_PROFILE_ALL_MODULES)

#define CXL_ROOT_FUNCTION_ID        1
#define CXL_ROOT_FUNCTION_NAME      L"[ROOT]"

#define GET_PID_COUNTER_ID(id_, pid_, cid_)      id_ = pid_; id_ = id_ << 32 | cid_;

// Sampling Configuration
using AMDTCounterIdVec = gtVector<AMDTCounterId>;
using CounterIdSampleConfigMap = gtMap<AMDTCounterId, AMDTProfileSamplingConfig>;

// Report View Configurations
using ViewConfigVec = gtVector<ViewConfig>;
using ViewConfigInfoVec = gtVector<ViewConfigInfo>;
using EncodedeEventToIdMap = gtMap<EventMaskType, AMDTCounterId>;

// Computed Counters
using ComputedCounterNameIdMap = gtMap<gtString, AMDTCounterId>;
using ComputedCounterIdSpecMap = gtMap<AMDTCounterId, ColumnSpec>;

// Module Info
using ModuleIdInfoMap = gtMap<AMDTModuleId, AMDTProfileModuleInfo>;
using ModuleIdExecutableMap = gtMap<AMDTModuleId, ExecutableFile*>;

// Function Info
using FuncIdSrcInfoMap = gtMap<AMDTFunctionId, AMDTSourceAndDisasmInfoVec>;
using FuncIdInfoMap = gtMap<AMDTFunctionId, AMDTProfileFunctionInfo>;

// Sample Totals
using ProcessSampleTotalMap = gtMap<AMDTProcessId, AMDTSampleValueVec>;
using ModuleSampleTotalMap = gtMap<AMDTModuleId, AMDTSampleValueVec>;

//
// Helper Routines
// TODO: These helper routines need to be in common utils
//
static inline gtString ConvertQtToGTString(const QString& inputStr)
{
    gtString str;
    str.resize(inputStr.length());

    if (inputStr.length())
    {
        str.resize(inputStr.toWCharArray(&str[0]));
    }

    return str;
}

static inline QString ConvertToQString(const gtString& inputStr)
{
    return QString::fromWCharArray(inputStr.asCharArray(), inputStr.length());
}

//
// Impl used by cxlProfileDataReader to query the profile db
//
class cxlProfileDataReader::Impl
{
public:
    // Data Members
    amdtProfileDbAdapter*               m_pDbAdapter = nullptr;
    gtString                            m_dbPathStr;

    // Symbol Dir, Symbol Server and Cached Dir paths
    gtVector<gtString>                  m_symbolServerPath;
    gtVector<gtString>                  m_symbolFilePath;
    gtVector<gtString>                  m_cachePath;

    // Paths to locate the source files
    gtVector<gtString>                  m_sourceFilePath;

    AMDTProfileSessionInfo              m_sessionInfo;
    AMDTCpuTopologyVec                  m_cpuToplogyVec;

    ModuleIdInfoMap                     m_moduleIdInfoMap;
    ModuleIdExecutableMap               m_moduleIdExeMap;
    gtMap<AMDTModuleId, gtUInt32>       m_moduleIdMaxFuncIdMap; // Kludge

    AMDTProfileCounterDescVec           m_sampledCounterDescVec;
    CounterIdSampleConfigMap            m_sampleConfigMap;
    EncodedeEventToIdMap                m_eventToIdMap;

    ComputedCounterNameIdMap            m_computedCounterNameIdMap;
    ComputedCounterIdSpecMap            m_computedCounterIdSpecMap;
    AMDTCounterId                       m_computedCounterStartId = 0x100;
    AMDTProfileCounterDescVec           m_computedCounterDescVec;

    ViewConfigInfoVec                   m_viewConfigInfoVec;
    AMDTProfileReportConfigVec          m_viewConfigs;

    AMDTProfileDataOptions              m_options;

    FuncIdInfoMap                       m_funcIdInfoMap;
    FuncIdSrcInfoMap                    m_funcIdSrcInfoMap;

    // Totals
    ProcessSampleTotalMap               m_processSampleTotalMap;
    ModuleSampleTotalMap                m_moduleSampleTotalMap;

    // CallGraph
    bool                                  m_foundCssProcesses = false;
    gtVector<AMDTProcessId>               m_cssProcesses;
    gtMap<gtUInt64, functionIdcgNodeMap*> m_pidCgFunctionMap;
    AMDTCounterId                         m_cgCounterId = 0;
    gtMap<AMDTProcessId, AMDTProcessId>   m_handleUnknownLeafs;

    Impl()
    {
    }

    ~Impl()
    {
        Clear();
    }

    void Clear(void)
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
        m_sourceFilePath.clear();

        m_sessionInfo.Clear();
        m_cpuToplogyVec.clear();

        m_moduleIdInfoMap.clear();
        m_sampledCounterDescVec.clear();
        m_sampleConfigMap.clear();
        m_eventToIdMap.clear();

        m_computedCounterNameIdMap.clear();
        m_computedCounterIdSpecMap.clear();
        m_computedCounterDescVec.clear();

        m_viewConfigInfoVec.clear();
        m_viewConfigs.clear();

        m_options.Clear();

        m_funcIdInfoMap.clear();
        m_funcIdSrcInfoMap.clear();

        m_processSampleTotalMap.clear();
        m_moduleSampleTotalMap.clear();

        for (auto pExeIter : m_moduleIdExeMap)
        {
            pExeIter.second->Close();
        }
        m_moduleIdExeMap.clear();

        m_moduleIdMaxFuncIdMap.clear();

        m_cssProcesses.clear();
        m_handleUnknownLeafs.clear();

        for (auto pCgFuncMap : m_pidCgFunctionMap)
        {
            pCgFuncMap.second->clear();
            delete pCgFuncMap.second;
        }

        m_pidCgFunctionMap.clear();
    };

    // profile file can either be raw PRD file or processed DB file
    bool OpenProfileData(gtString profileFilePath)
    {
        bool ret = false;

        m_dbPathStr = profileFilePath;

        if (nullptr == m_pDbAdapter)
        {
            m_pDbAdapter = new amdtProfileDbAdapter;

            if (nullptr != m_pDbAdapter)
            {
                ret = m_pDbAdapter->OpenDb(profileFilePath, AMDT_PROFILE_MODE_AGGREGATION, false);

                ret = ret && GetProfileSessionInfo(m_sessionInfo);

                ret = ret && InitializeReportOption();
            }
        }

        return ret;
    }

    bool CloseProfileData()
    {
        Clear();
        return true;
    }

    bool GetProfileSessionInfo(AMDTProfileSessionInfo& sessionInfo)
    {
        return (nullptr != m_pDbAdapter) ? m_pDbAdapter->GetSessionInfo(sessionInfo) : false;
    }

    bool GetCpuTopology(AMDTCpuTopologyVec& cpuTopology)
    {
        bool ret = false;

        if (!m_cpuToplogyVec.empty())
        {
            cpuTopology = m_cpuToplogyVec;
            ret = true;
        }
        else if (nullptr != m_pDbAdapter)
        {
            ret = m_pDbAdapter->GetCpuTopology(m_cpuToplogyVec);
            cpuTopology = m_cpuToplogyVec;
        }

        return ret;
    }

    bool GetSampledCountersList()
    {
        bool ret = (nullptr != m_pDbAdapter) ? true : false;

        if (ret && (m_sampledCounterDescVec.empty()))
        {
            ret = m_pDbAdapter->GetSampledCountersList(m_sampledCounterDescVec);
        }

        return ret;
    }

    bool GetCounterDescById(AMDTCounterId id, AMDTProfileCounterDesc& desc)
    {
        bool foundCounter = false;

        for (auto counterDesc : m_sampledCounterDescVec)
        {
            if (counterDesc.m_id == id)
            {
                desc = counterDesc;
                foundCounter = true;
                break;
            }
        }

        if (!foundCounter)
        {
            for (auto counterDesc : m_computedCounterDescVec)
            {
                if (counterDesc.m_id == id)
                {
                    desc = counterDesc;
                    foundCounter = true;
                    break;
                }
            }
        }

        return foundCounter;
    }

    bool GetSampledCountersList(AMDTProfileCounterDescVec& counterDesc)
    {
        bool ret = true;

        if (m_sampledCounterDescVec.empty())
        {
            ret = GetSampledCountersList();
        }

        if (ret)
        {
            counterDesc = m_sampledCounterDescVec;
        }

        return ret;
    }

    bool GetSamplingConfiguration(AMDTUInt32 counterId, AMDTProfileSamplingConfig& sampleConfig)
    {
        bool ret = true;
        auto aConfig = m_sampleConfigMap.find(counterId);

        if (m_sampleConfigMap.end() == aConfig)
        {
            ret = false;

            if (nullptr != m_pDbAdapter)
            {
                ret = m_pDbAdapter->GetSamplingConfiguration(counterId, sampleConfig);

                if (ret)
                {
                    m_sampleConfigMap.insert({ counterId, sampleConfig });
                }
            }
        }
        else
        {
            sampleConfig = aConfig->second;
        }

        return ret;
    }

    bool InitializeCounterData()
    {
        bool ret = GetSampledCountersList();

        if (ret)
        {
            for (auto& counterDesc : m_sampledCounterDescVec)
            {
                AMDTProfileSamplingConfig sampleConfig;

                if (GetSamplingConfiguration(counterDesc.m_id, sampleConfig))
                {
                    gtUInt16 eventSel = static_cast<gtUInt16>(sampleConfig.m_hwEventId);
                    EventMaskType encodedEvent = EncodeEvent(eventSel,
                        sampleConfig.m_unitMask,
                        sampleConfig.m_osMode,
                        sampleConfig.m_userMode);

                    m_eventToIdMap.insert({ encodedEvent, counterDesc.m_id });
                }
            }
        }

        return ret;
    }

    bool PrepareReportAllDataView()
    {
        bool ret = GetSampledCountersList();

        if (ret)
        {
            ViewConfigInfo viewCfgInfo;
            viewCfgInfo.m_viewCfg.SetConfigName(QString(CXL_REPORT_VIEW_ALL_DATA));

            gtUInt32 nbrEvents = m_sampledCounterDescVec.size();
            EventConfig* pEventConfig = new EventConfig[nbrEvents];

            if (nullptr != pEventConfig)
            {
                QStringList eventList;
                int i = 0;

                for (auto& event : m_sampledCounterDescVec)
                {
                    AMDTProfileSamplingConfig sampleConfig;
                    ret = GetSamplingConfiguration(event.m_id, sampleConfig);

                    if (ret)
                    {
                        eventList += QString(event.m_name.asASCIICharArray());

                        // Initialize event config array
                        pEventConfig[i].eventSelect = event.m_hwEventId;
                        pEventConfig[i].eventUnitMask = sampleConfig.m_unitMask;
                        pEventConfig[i].bitUsr = sampleConfig.m_userMode;
                        pEventConfig[i].bitOs = sampleConfig.m_osMode;

                        viewCfgInfo.m_counterIdVec.push_back(event.m_id);
                        i++;
                    }
                }

                // Make column specifications
                viewCfgInfo.m_viewCfg.MakeColumnSpecs(i, pEventConfig, eventList);
                viewCfgInfo.m_viewCfg.SetDescription("This special view has all of the data from the profile available.");

                m_viewConfigInfoVec.push_back(viewCfgInfo);

                delete[] pEventConfig;
            }
        }

        return ret;
    }

    bool GetRawCounterId(gtUInt16 eventSel, gtUByte unitMask, bool osMode, bool osUser, AMDTCounterId& counterId)
    {
        bool ret = false;
        bool bitOs = osMode;
        bool bitUsr = osUser;

        if (IsTimerEvent(eventSel) || IsIbsFetchEvent(eventSel) || IsIbsOpEvent(eventSel))
        {
            bitOs = false;
            bitUsr = false;
        }

        EventMaskType eventLeft = EncodeEvent(eventSel, unitMask, bitOs, bitUsr);

        auto it = m_eventToIdMap.find(eventLeft);

        if (it != m_eventToIdMap.end())
        {
            counterId = it->second;
            ret = true;
        }

        return ret;
    }

    bool GetCounterIdsFromColumnSpec(ColumnSpec*& columnArray, AMDTUInt32 nbrCols, AMDTCounterIdVec& counterVec)
    {
        bool ret = true;

        AMDTProfileCounterDesc counterDesc;

        for (gtUInt32 i = 0; i < nbrCols; i++)
        {
            if (columnArray[i].dataSelectLeft.eventSelect && columnArray[i].dataSelectRight.eventSelect)
            {
                // This is COMPUTED counter
                // check whether we have seen this already
                gtString computedCounterName = ConvertQtToGTString(columnArray[i].title);

                AMDTCounterId computedCounterId = 0;
                auto it = m_computedCounterNameIdMap.find(computedCounterName);

                if (it == m_computedCounterNameIdMap.end())
                {
                    // Construct dummy event decriptor
                    computedCounterId = m_computedCounterStartId++;

                    counterDesc.m_id = computedCounterId;
                    counterDesc.m_type = AMDT_PROFILE_COUNTER_TYPE_COMPUTED;
                    counterDesc.m_hwEventId = AMDT_PROFILE_ALL_COUNTERS;
                    counterDesc.m_deviceId = 0;
                    counterDesc.m_category = 0;
                    counterDesc.m_unit = AMDT_PROFILE_COUNTER_UNIT_PERCENT;

                    counterDesc.m_name = ConvertQtToGTString(columnArray[i].title);
                    counterDesc.m_abbrev = ConvertQtToGTString(columnArray[i].title);
                    counterDesc.m_description = counterDesc.m_name; // FIXME

                    m_computedCounterDescVec.push_back(counterDesc);
                    m_computedCounterIdSpecMap.insert({ computedCounterId, columnArray[i] });
                }
                else
                {
                    computedCounterId = it->second;
                }

                counterVec.push_back(computedCounterId);
            }
            else if (columnArray[i].dataSelectLeft.eventSelect)
            {
                // This is RAW counter
                AMDTCounterId counterId;

                ret = GetRawCounterId(columnArray[i].dataSelectLeft.eventSelect,
                                      columnArray[i].dataSelectLeft.eventUnitMask,
                                      columnArray[i].dataSelectLeft.bitOs,
                                      columnArray[i].dataSelectLeft.bitUsr,
                                      counterId);

                if (ret)
                {
                    counterVec.push_back(counterId);
                }
            }
        }

        return ret;
    }

    bool IsSamplingEventsAvailable(ViewConfig& viewConfig, AMDTCounterIdVec& counterVec)
    {
        bool ret = true;

        gtUInt32 nbrCols = viewConfig.GetNumberOfColumns();
        ColumnSpec* columnArray = new ColumnSpec[nbrCols];
        viewConfig.GetColumnSpecs(columnArray);

        // Now that we have got the columnspec from view config, check
        // whether all the events are in event vector
        for (gtUInt32 i = 0; ret && i < nbrCols; i++)
        {
            AMDTCounterId counterId;

            if (columnArray[i].dataSelectLeft.eventSelect)
            {
                ret = GetRawCounterId(columnArray[i].dataSelectLeft.eventSelect,
                                      columnArray[i].dataSelectLeft.eventUnitMask,
                                      columnArray[i].dataSelectLeft.bitOs,
                                      columnArray[i].dataSelectLeft.bitUsr,
                                      counterId);
            }

            if (ret && columnArray[i].dataSelectRight.eventSelect)
            {
                ret = GetRawCounterId(columnArray[i].dataSelectRight.eventSelect,
                                      columnArray[i].dataSelectRight.eventUnitMask,
                                      columnArray[i].dataSelectRight.bitOs,
                                      columnArray[i].dataSelectRight.bitUsr,
                                      counterId);
            }
        }

        // This ViewConfig can be supported for this profile run
        if (ret)
        {
            ret = GetCounterIdsFromColumnSpec(columnArray, nbrCols, counterVec);
        }

        if (columnArray)
        {
            delete[] columnArray;
        }

        return ret;
    }

    bool PrepareReportDataView(gtString viewConfigPath)
    {
        bool ret = GetSampledCountersList();

        if (ret)
        {
            ViewConfigInfo viewCfgInfo;
            ViewConfig& viewCfg = viewCfgInfo.m_viewCfg;

            viewCfg.ReadConfigFile(ConvertToQString(viewConfigPath));

            if (IsSamplingEventsAvailable(viewCfg, viewCfgInfo.m_counterIdVec))
            {
                m_viewConfigInfoVec.push_back(viewCfgInfo);
            }
        }

        return ret;
    }

    bool InitializeReportConfigurations()
    {
        if (m_viewConfigInfoVec.empty())
        {
            InitializeCounterData();

            PrepareReportAllDataView();

            // TODO: Add other view configs
            osFilePath basePath(osFilePath::OS_CODEXL_DATA_PATH);
            basePath.appendSubDirectory(L"Views");
            osDirectory baseDir(basePath);

            gtList<osFilePath> filePaths;

            // Read TBP
            if (baseDir.getContainedFilePaths(L"*.XML", osDirectory::SORT_BY_NAME_DESCENDING, filePaths))
            {
                for (auto& it : filePaths)
                {
                    PrepareReportDataView(it.asString());
                }
            }

            gtString subName;
            int cpuFamily = m_sessionInfo.m_cpuFamily;
            int cpuModel = m_sessionInfo.m_cpuModel;

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
                    for (auto& it : filePaths)
                    {
                        PrepareReportDataView(it.asString());
                    }
                }
            }
        }

        return true;
    }

    bool GetReportConfigurationByName(gtString configName, AMDTProfileReportConfig& reportConfig)
    {
        bool ret = false;

        if ((ret = InitializeReportConfigurations()))
        {
            // construct the report configs
            for (auto& viewConfigInfo : m_viewConfigInfoVec)
            {
                QString name;
                viewConfigInfo.m_viewCfg.GetConfigName(name);

                gtString cfgName = ConvertQtToGTString(name);

                if (cfgName.compareNoCase(configName) == 0)
                {
                    reportConfig.m_name = ConvertQtToGTString(name);

                    for (auto& counterId : viewConfigInfo.m_counterIdVec)
                    {
                        AMDTProfileCounterDesc counterDesc;

                        if (GetCounterDescById(counterId, counterDesc))
                        {
                            reportConfig.m_counterDescs.push_back(counterDesc);
                        }
                    }

                    ret = true;
                    break;
                }
            }
        }

        return ret;
    }

    bool GetReportConfigurations(AMDTProfileReportConfigVec& reportConfigs)
    {
        bool ret = false;

        if ((ret = InitializeReportConfigurations()))
        {
            // construct the report configs
            for (auto& viewConfigInfo : m_viewConfigInfoVec)
            {
                AMDTProfileReportConfig reportConfig;

                QString name;
                viewConfigInfo.m_viewCfg.GetConfigName(name);
                reportConfig.m_name = ConvertQtToGTString(name);

                for (auto& counterId : viewConfigInfo.m_counterIdVec)
                {
                    AMDTProfileCounterDesc counterDesc;

                    if (GetCounterDescById(counterId, counterDesc))
                    {
                        reportConfig.m_counterDescs.push_back(counterDesc);
                    }
                }

                reportConfigs.push_back(reportConfig);
            }
        }

        return ret;
    }

    bool SetDebugInfoPaths(gtVector<gtString>& symbolServer, gtVector<gtString>& symbolDirectory)
    {
        bool ret = true;

        m_symbolServerPath.insert(std::end(m_symbolServerPath), std::begin(symbolServer), std::end(symbolServer));
        m_symbolFilePath.insert(std::end(m_symbolFilePath), std::begin(symbolDirectory), std::end(symbolDirectory));

        return ret;
    }

    bool SetSourcePaths(gtVector<gtString>& sourceDirPath)
    {
        bool ret = true;

        m_sourceFilePath.insert(std::end(m_sourceFilePath), std::begin(sourceDirPath), std::end(sourceDirPath));

        return ret;
    }

    bool InitializeReportOption()
    {
        bool ret = true;

        m_options.m_coreMask = AMDT_PROFILE_ALL_CORES;
        m_options.m_isSeperateByCore = false;
        m_options.m_isSeperateByNuma = false;
        m_options.m_ignoreSystemModules = false; // FIXME
        m_options.m_doSort = true;
        m_options.m_othersEntryInSummary = true;
        m_options.m_summaryCount = 5;

        // Set the default view to "All Data"
        AMDTProfileReportConfig reportConfig;

        if (GetReportConfigurationByName(CXL_REPORT_VIEW_ALL_DATAW, reportConfig))
        {
            for (auto& counterDesc : reportConfig.m_counterDescs)
            {
                m_options.m_counters.push_back(counterDesc.m_id);
            }
        }

        return ret;
    }

    bool SetReportOption(AMDTProfileDataOptions& options)
    {
        bool ret = true;

        if (options.m_counters.size() > 0)
        {
            m_options.m_counters = options.m_counters;
        }

        // TODO: Write a  copy ctor/assignment operator for AMDTProfileDataOptions
        m_options.m_coreMask            = options.m_coreMask;
        m_options.m_isSeperateByCore    = options.m_isSeperateByCore;
        m_options.m_isSeperateByNuma    = options.m_isSeperateByCore;
        m_options.m_ignoreSystemModules = options.m_ignoreSystemModules;
        m_options.m_doSort              = options.m_doSort;
        m_options.m_summaryCount        = options.m_summaryCount;
        m_options.m_maxBytesToDisassemble = options.m_maxBytesToDisassemble;
        m_options.m_othersEntryInSummary = options.m_othersEntryInSummary;

        return ret;
    }

    bool SetReportOption(AMDTReportOptionType type, gtUInt64 value)
    {
        bool ret = true;

        switch (type)
        {
        case AMDT_REPORT_OPTION_COREMASK:
            m_options.m_coreMask = value;
            break;

        case AMDT_REPORT_OPTION_AGGREGATE_BY_CORE:
            m_options.m_isSeperateByCore = (value == 1ULL) ? true : false;
            break;

        case AMDT_REPORT_OPTION_AGGREGATE_BY_NUMA:
            m_options.m_isSeperateByNuma = (value == 1ULL) ? true : false;
            break;

        case AMDT_REPORT_OPTION_IGNORE_SYSTEM_MODULE:
            m_options.m_ignoreSystemModules = (value == 1ULL) ? true : false;
            break;

        case AMDT_REPORT_OPTION_SORT_PROFILE_DATA:
            m_options.m_doSort = (value == 1ULL) ? true : false;
            break;

        case AMDT_REPORT_OPTION_OTHERS_ENTRY_IN_SUMMARY:
            m_options.m_othersEntryInSummary = (value == 1ULL) ? true : false;
            break;

        case AMDT_REPORT_OPTION_SUMMARY_COUNT:
            m_options.m_summaryCount = static_cast<gtUInt16>(value);
            break;

        case AMDT_REPORT_OPTION_MAX_BYTES_TO_DISASM:
            m_options.m_maxBytesToDisassemble = static_cast<gtUInt16>(value);
            break;

        default:
            ret = false;
            break;
        }

        return ret;
    }

    bool SetReportCounters(gtVector<AMDTUInt32> countersList)
    {
        bool ret = false;

        if (!countersList.empty())
        {
            m_options.m_counters = countersList;
            ret = true;
        }

        return ret;
    }

    bool InitializeSymbolEngine(ExecutableFile* pExecutable)
    {
        bool retVal = false;

        if (NULL != pExecutable)
        {
            // TODO:
            const wchar_t* pSearchPath = NULL;
            const wchar_t* pServerList = NULL;
            const wchar_t* pCachePath = NULL;

            retVal = pExecutable->InitializeSymbolEngine(pSearchPath, pServerList, pCachePath);
        }

        return retVal;
    }

    bool GetProcessInfo(AMDTUInt32 pid, AMDTProfileProcessInfoVec& procInfo)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            ret = m_pDbAdapter->GetProcessInfo(pid, procInfo);
        }

        return ret;
    }

    bool GetModuleInfo(AMDTModuleId moduleId, AMDTProfileModuleInfo& modInfo)
    {
        bool ret = false;

        auto it = m_moduleIdInfoMap.find(moduleId);

        if (it == m_moduleIdInfoMap.end())
        {
            AMDTProfileModuleInfoVec modInfoVec;

            ret = m_pDbAdapter->GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, moduleId, modInfoVec);

            // FIXME: moduleId cannot be zero.. somehow we insert moduleId 0??
            if (ret && modInfoVec.size() > 0)
            {
                modInfo = modInfoVec[0];
                m_moduleIdInfoMap.insert({ moduleId, modInfo });
            }
        }
        else
        {
            modInfo = it->second;
            ret = true;
        }

        return ret;
    }

    bool GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, AMDTProfileModuleInfoVec& modInfo)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            ret = m_pDbAdapter->GetModuleInfo(pid, mid, modInfo);
        }

        return ret;
    }

    bool GetThreadInfo(AMDTUInt32 pid, AMDTThreadId tid, AMDTProfileThreadInfoVec& threadInfo)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            ret = m_pDbAdapter->GetThreadInfo(pid, tid, threadInfo);
        }

        return ret;
    }

    bool AddOthersEntry(AMDTProfileDataVec& summaryDataVec, AMDTCounterId counterId)
    {
        bool ret = false;
        double totalValue = 0.0;

        ret = GetProfileRunRawCounterTotal(counterId, totalValue);

        if (ret)
        {
            double currentTotal = 0.0;

            for (auto& aSummaryData : summaryDataVec)
            {
                for (auto& aSampleValue : aSummaryData.m_sampleValue)
                {
                    if (aSampleValue.m_counterId == counterId)
                    {
                        currentTotal += aSampleValue.m_sampleCount;
                    }
                }
            }

            if (static_cast<gtUInt64>(totalValue) > static_cast<gtUInt64>(currentTotal))
            {
                // Add an "Other" Entry
                AMDTProfileData otherEntry;

                otherEntry.m_id = AMDT_PROFILE_ALL_PROCESSES; // FIXME
                otherEntry.m_name = L"other";
                otherEntry.m_moduleId = AMDT_PROFILE_ALL_MODULES;   // FIXME
                otherEntry.m_type = summaryDataVec[0].m_type;

                AMDTSampleValue otherSampleValue;
                otherSampleValue.m_counterId = counterId;
                otherSampleValue.m_sampleCount = totalValue - currentTotal;

                otherEntry.m_sampleValue.push_back(otherSampleValue);

                summaryDataVec.push_back(otherEntry);
            }
        }

        return ret;
    }

    //      !!! SUMMARY APIs    !!!
    bool GetSummaryData(AMDTProfileDataType type, AMDTUInt32 counterId, AMDTProfileDataVec& summaryDataVec)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            gtVector<AMDTUInt32> counterIdList;
            counterIdList.push_back(counterId);

            ret = m_pDbAdapter->GetProfileData(type,
                                               AMDT_PROFILE_ALL_PROCESSES,
                                               AMDT_PROFILE_ALL_MODULES,
                                               AMDT_PROFILE_ALL_THREADS,
                                               counterIdList,
                                               m_options.m_coreMask,
                                               m_options.m_isSeperateByCore,
                                               false,  // separateByProcess
                                               m_options.m_doSort,
                                               m_options.m_summaryCount,
                                               summaryDataVec);

            ret = AddOthersEntry(summaryDataVec, counterId);

            // calculate the percentage
            ret = CalculateRawCounterPercentage(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, counterIdList, summaryDataVec);
        }

        return ret;
    }

    // Totals is always for all the cores and no separtateBCore
    bool GetTotals(AMDTProfileDataType type,
                   AMDTProcessId procId,
                   AMDTThreadId threadId,
                   AMDTModuleId moduleId,
                   AMDTFunctionId funcId,
                   gtUInt64 coreMask,
                   bool separateByCore,
                   AMDTCounterIdVec& counterIdList,
                   AMDTSampleValueVec& totalValueVec)
    {
        bool ret = false;

        switch (type)
        {
        case AMDT_PROFILE_DATA_PROCESS:
            {
                auto procTotalIt = m_processSampleTotalMap.find(procId);

                if (procTotalIt != m_processSampleTotalMap.end())
                {
                    totalValueVec = procTotalIt->second;
                    ret = true;
                }
            }
            break;

        case AMDT_PROFILE_DATA_MODULE:
            {
                // Note: Only if moduleId is not -1
                if (IS_MODULE_QUERY(moduleId))
                {
                    auto modTotalIt = m_moduleSampleTotalMap.find(moduleId);

                    if (modTotalIt != m_moduleSampleTotalMap.end())
                    {
                        totalValueVec = modTotalIt->second;
                        ret = true;
                    }
                }
            }
            break;

        case AMDT_PROFILE_DATA_THREAD:
        case AMDT_PROFILE_DATA_FUNCTION:
        default:
            break;
        }

        if ((false == ret) && (nullptr != m_pDbAdapter))
        {
            ret = m_pDbAdapter->GetCounterTotals(type,
                                                 procId,
                                                 threadId,
                                                 moduleId,
                                                 funcId,
                                                 counterIdList,
                                                 coreMask,
                                                 separateByCore,
                                                 totalValueVec);
        }

        // TODO: m_processSampleTotalMap may not have all the counters..
        //if (ret)
        //{
        //    if (AMDT_PROFILE_DATA_PROCESS == type)
        //    {
        //        m_processSampleTotalMap.insert({ procId, totalValueVec });
        //    }
        //    else if ((AMDT_PROFILE_DATA_MODULE == type) && IS_MODULE_QUERY(moduleId))
        //    {
        //        m_moduleSampleTotalMap.insert({ moduleId, totalValueVec });
        //    }
        //}

        return ret;
    }

    bool GetProfileRunRawCounterTotal(AMDTUInt32 counterId, double& totalValue)
    {
        bool ret = false;
        gtVector<AMDTUInt32> counterIdList;
        counterIdList.push_back(counterId);
        AMDTSampleValueVec totalValueVec;

        ret = GetTotals(AMDT_PROFILE_DATA_PROCESS,
                        AMDT_PROFILE_ALL_PROCESSES,
                        AMDT_PROFILE_ALL_THREADS,
                        AMDT_PROFILE_ALL_MODULES,
                        0,
                        AMDT_PROFILE_ALL_CORES,
                        false, // separateByCore
                        counterIdList,
                        totalValueVec);

        auto counterTotal = std::find_if(totalValueVec.begin(), totalValueVec.end(),
            [&counterId](AMDTSampleValue const& aData) { return aData.m_counterId == counterId; });

        totalValue = (counterTotal != totalValueVec.end()) ? counterTotal->m_sampleCount : 0.0;

        return ret;
    }

    //  For the entire process
    //  for a given processId
    //  for a given moduleId across all process
    //  for a given module with in the process
    bool CalculateRawCounterPercentage(AMDTProcessId procId,
                                       AMDTModuleId modId,
                                       gtVector<AMDTUInt32> counterIdList,
                                       AMDTProfileDataVec& summaryDataVec)
    {
        bool ret = false;
        AMDTSampleValueVec totalValueVec;

        AMDTProfileDataType type = (IS_MODULE_QUERY(modId)) ? AMDT_PROFILE_DATA_MODULE : AMDT_PROFILE_DATA_PROCESS;

        ret =  GetTotals(type,
                         procId,
                         AMDT_PROFILE_ALL_THREADS,
                         modId,
                         0,
                         AMDT_PROFILE_ALL_CORES,
                         false, // separateByCore
                         counterIdList,
                         totalValueVec);

        if (ret && !totalValueVec.empty())
        {
            for (auto& aSummaryData : summaryDataVec)
            {
                for (gtUInt32 idx = 0; idx < aSummaryData.m_sampleValue.size(); idx++)
                {
                    aSummaryData.m_sampleValue[idx].m_sampleCountPercentage =
                        (aSummaryData.m_sampleValue[idx].m_sampleCount / totalValueVec[idx].m_sampleCount) * 100.0;
                }
            }
        }

        return ret;
    }

    bool CalculateRawCounterPercentage(AMDTUInt32 functionId,
                                       AMDTProcessId procId,
                                       AMDTThreadId threadId,
                                       gtVector<AMDTUInt32> counterIdList,
                                       AMDTProfileFunctionData&  functionData)
    {
        bool ret = false;
        AMDTSampleValueVec totalValueVec;

        ret = GetTotals(AMDT_PROFILE_DATA_FUNCTION,
                        procId,
                        threadId,
                        AMDT_PROFILE_ALL_MODULES,
                        functionId,
                        AMDT_PROFILE_ALL_CORES,
                        false, // separateByCore
                        counterIdList,
                        totalValueVec);

        if (ret && !totalValueVec.empty())
        {
            for (auto& srcLineData : functionData.m_srcLineDataList)
            {
                for (gtUInt32 idx = 0; idx < srcLineData.m_sampleValues.size(); idx++)
                {
                    srcLineData.m_sampleValues[idx].m_sampleCountPercentage =
                        (srcLineData.m_sampleValues[idx].m_sampleCount / totalValueVec[idx].m_sampleCount) * 100.0;
                }
            }

            for (auto& instLineData : functionData.m_instDataList)
            {
                for (gtUInt32 idx = 0; idx < instLineData.m_sampleValues.size(); idx++)
                {
                    instLineData.m_sampleValues[idx].m_sampleCountPercentage =
                        (instLineData.m_sampleValues[idx].m_sampleCount / totalValueVec[idx].m_sampleCount) * 100.0;
                }
            }
        }

        return ret;
    }

    bool GetProcessSummary(AMDTUInt32 counterId, AMDTProfileDataVec& processSummaryData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_PROCESS, counterId, processSummaryData);
    }

    bool GetThreadSummary(AMDTUInt32 counterId, AMDTProfileDataVec& threadSummaryData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_THREAD, counterId, threadSummaryData);
    }

    bool GetModuleSummary(AMDTUInt32 counterId, AMDTProfileDataVec& moduleSummaryData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_MODULE, counterId, moduleSummaryData);
    }

    bool GetFunctionSummary(AMDTUInt32 counterId, AMDTProfileDataVec& funcSummaryData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_FUNCTION, counterId, funcSummaryData);
    }

    // based on m_options.counters..
    //      provides the RAW counters and the RAW counters for COMPUTED counters
    bool GetCountersList(AMDTCounterIdVec& counterList)
    {
        bool ret = true;

        if (nullptr != m_pDbAdapter)
        {
            gtSet<AMDTCounterId> uniqueCounterSet;

            for (auto& counter : m_options.m_counters)
            {
                AMDTProfileCounterDesc desc;

                if (GetCounterDescById(counter, desc))
                {
                    if (AMDT_PROFILE_COUNTER_TYPE_RAW == desc.m_type)
                    {
                        uniqueCounterSet.insert(desc.m_id);
                    }
                    else if (AMDT_PROFILE_COUNTER_TYPE_COMPUTED == desc.m_type)
                    {
                        auto it = m_computedCounterIdSpecMap.find(desc.m_id);

                        if (it != m_computedCounterIdSpecMap.end())
                        {
                            ColumnSpec& columnSpec = it->second;
                            AMDTCounterId lhsCounterId;
                            AMDTCounterId rhsCounterId;

                            ret = GetRawCounterId(columnSpec.dataSelectLeft.eventSelect,
                                                   columnSpec.dataSelectLeft.eventUnitMask,
                                                   columnSpec.dataSelectLeft.bitOs,
                                                   columnSpec.dataSelectLeft.bitUsr,
                                                   lhsCounterId);


                            ret = ret && GetRawCounterId(columnSpec.dataSelectRight.eventSelect,
                                                         columnSpec.dataSelectRight.eventUnitMask,
                                                         columnSpec.dataSelectRight.bitOs,
                                                         columnSpec.dataSelectRight.bitUsr,
                                                         rhsCounterId);

                            if (ret)
                            {
                                uniqueCounterSet.insert(lhsCounterId);
                                uniqueCounterSet.insert(rhsCounterId);
                            }
                        }
                    }
                }
            }

            for (int counterId : uniqueCounterSet)
            {
                counterList.push_back(counterId);
            }
        }

        return ret;
    }

    bool GetRawCounterValueFromSampleValueVec(AMDTSampleValueVec& sampleVec, AMDTCounterId counterId, double& sampleValue, double& samplePerc)
    {
        bool ret = false;

        for (auto& aSampleValue : sampleVec)
        {
            if (aSampleValue.m_counterId == counterId)
            {
                sampleValue = aSampleValue.m_sampleCount;
                samplePerc = aSampleValue.m_sampleCountPercentage;
                ret = true;
                break;
            }
        }

        return ret;
    }

    bool GetDataForColumnSpec(ColumnSpec& columnSpec, AMDTSampleValueVec& sampleVec, double& computedCounterValue)
    {
        bool ret = true;

        AMDTCounterId lhsCounterId = 0;
        AMDTCounterId rhsCounterId = 0;

        ret = GetRawCounterId(columnSpec.dataSelectLeft.eventSelect,
                              columnSpec.dataSelectLeft.eventUnitMask,
                              columnSpec.dataSelectLeft.bitOs,
                              columnSpec.dataSelectLeft.bitUsr,
                              lhsCounterId);

        ret = ret && GetRawCounterId(columnSpec.dataSelectRight.eventSelect,
                                     columnSpec.dataSelectRight.eventUnitMask,
                                     columnSpec.dataSelectRight.bitOs,
                                     columnSpec.dataSelectRight.bitUsr,
                                     rhsCounterId);

        double lhsCount = 0.0;
        double rhsCount = 0.0;
        double lhsCountPerc = 0.0;
        double rhsCountPerc = 0.0;

        ret = ret && GetRawCounterValueFromSampleValueVec(sampleVec, lhsCounterId, lhsCount, lhsCountPerc);
        ret = ret && GetRawCounterValueFromSampleValueVec(sampleVec, rhsCounterId, rhsCount, rhsCountPerc);

        if (ret)
        {
            switch (columnSpec.type)
            {
            case ColumnValue:
                computedCounterValue = lhsCount;
                break;

            case ColumnSum:
                computedCounterValue = lhsCount + rhsCount;
                break;

            case ColumnDifference:
                computedCounterValue = lhsCount - rhsCount;
                break;

            case ColumnProduct:
                computedCounterValue = lhsCount * rhsCount;
                break;

            case ColumnRatio:
                // The sampling interval should also be considered while computing the ration
                if (lhsCount > 0.0 && rhsCount > 0.0)
                {
                    AMDTProfileSamplingConfig lhsConfig;
                    AMDTProfileSamplingConfig rhsConfig;

                    GetSamplingConfiguration(lhsCounterId, lhsConfig);
                    GetSamplingConfiguration(rhsCounterId, rhsConfig);

                    computedCounterValue = (lhsCount * lhsConfig.m_samplingInterval) / (rhsCount * rhsConfig.m_samplingInterval);
                }
                break;

            case ColumnPercentage:
                //  Note: !!! Used  Only For CLU !!!
                if (lhsCount > 0.0 && rhsCount > 0.0)
                {
                    computedCounterValue = (lhsCount / 64.0) / (rhsCount * 100.0);
                }
                break;

            case ColumnInvalid:
                ret = false;
                break;
            } // switch column type
        }

        return ret;
    }

    bool CalculateComputedCounters(AMDTSampleValueVec& profileDataVec, AMDTSampleValueVec& newProfileDataVec)
    {
        bool ret = false;

        // Iterate over the counters and calculate the computed counter values
        for (auto& counterId : m_options.m_counters)
        {
            AMDTProfileCounterDesc desc;

            if (GetCounterDescById(counterId, desc))
            {
                AMDTSampleValue sampleData;
                double sampleValue = 0.0;
                double samplePerc = 0.0;

                if (AMDT_PROFILE_COUNTER_TYPE_RAW == desc.m_type)
                {
                    if ((ret = GetRawCounterValueFromSampleValueVec(profileDataVec, counterId, sampleValue, samplePerc)))
                    {
                        sampleData.m_counterId = counterId;
                        sampleData.m_sampleCount = sampleValue;
                        sampleData.m_sampleCountPercentage = samplePerc;
                    }

                    newProfileDataVec.push_back(sampleData);
                }
                else if (AMDT_PROFILE_COUNTER_TYPE_COMPUTED == desc.m_type)
                {
                    auto it = m_computedCounterIdSpecMap.find(desc.m_id);

                    if (it != m_computedCounterIdSpecMap.end())
                    {
                        ColumnSpec& columnSpec = it->second;

                        if ((ret = GetDataForColumnSpec(columnSpec, profileDataVec, sampleValue)))
                        {
                            sampleData.m_counterId = counterId;
                            sampleData.m_sampleCount = sampleValue;
                        }

                        newProfileDataVec.push_back(sampleData);
                    }
                }
            }
        }

        return ret;
    }

    bool CalculateComputedCounters(AMDTProfileDataVec& profileDataVec)
    {
        bool ret = true;

        for (auto& profileData : profileDataVec)
        {
            AMDTSampleValueVec sampleValueVec = profileData.m_sampleValue;
            profileData.m_sampleValue.clear();
            ret = ret && CalculateComputedCounters(sampleValueVec, profileData.m_sampleValue);
        }

        return ret;
    }

    bool CalculateComputedCounters(AMDTProfileSourceLineDataVec& srcLineDataVec)
    {
        bool ret = true;

        for (auto& srcLineData : srcLineDataVec)
        {
            AMDTSampleValueVec sampleValueVec = srcLineData.m_sampleValues;
            srcLineData.m_sampleValues.clear();
            ret = ret && CalculateComputedCounters(sampleValueVec, srcLineData.m_sampleValues);
        }

        return ret;
    }

    bool CalculateComputedCounters(AMDTProfileInstructionDataVec& instLineDataVec)
    {
        bool ret = true;

        for (auto& instLineData : instLineDataVec)
        {
            AMDTSampleValueVec sampleValueVec = instLineData.m_sampleValues;
            instLineData.m_sampleValues.clear();
            ret = ret && CalculateComputedCounters(sampleValueVec, instLineData.m_sampleValues);
        }

        return ret;
    }

    // Process/Module/Funtion View APIs
    bool GetSummaryData(AMDTProfileDataType type, AMDTProcessId procId, AMDTModuleId moduleId, AMDTProfileDataVec& summaryDataVec)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            AMDTCounterIdVec countersList;

            ret = GetCountersList(countersList);

            ret = ret && m_pDbAdapter->GetProfileData(type,
                                                      procId,
                                                      moduleId,
                                                      AMDT_PROFILE_ALL_THREADS,
                                                      countersList,
                                                      m_options.m_coreMask,
                                                      m_options.m_isSeperateByCore,
                                                      false,  // separateByProcess
                                                      m_options.m_doSort,
                                                      0,
                                                      summaryDataVec);

            // Calculate the percentage
            ret = CalculateRawCounterPercentage(procId, moduleId, countersList, summaryDataVec);

            // Iterate over the profileDataVec and calculate the computed counter values
            ret = ret && CalculateComputedCounters(summaryDataVec);
        }

        return ret;
    }

    bool GetProcessProfileData(AMDTProcessId procId, AMDTProfileDataVec& processProfileData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_PROCESS, procId, AMDT_PROFILE_ALL_MODULES, processProfileData);
    }

    bool GetModuleProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& moduleProfileData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_MODULE, procId, modId, moduleProfileData);
    }

    bool GetFunctionProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& funcProfileData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_FUNCTION, procId, modId, funcProfileData);
    }

    bool GetFunctionDetailedProfileData(AMDTFunctionId            funcId,
                                        AMDTProcessId             processId,
                                        AMDTThreadId              threadId,
                                        AMDTProfileFunctionData&  functionData)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            AMDTCounterIdVec countersList;
            ret = GetCountersList(countersList);

            ret = ret && m_pDbAdapter->GetFunctionProfileData(funcId,
                                                              processId,
                                                              threadId,
                                                              countersList,
                                                              m_options.m_coreMask,
                                                              m_options.m_isSeperateByCore,
                                                              functionData);

            if (ret)
            {
                // if function size is zero, compute the size from instruction data.
                gtUInt32 functionSize = functionData.m_functionInfo.m_size;
                gtUInt32 startOffset = functionData.m_functionInfo.m_startOffset;
                gtUInt32 nbrInsts = functionData.m_instDataList.size();

                if (functionSize == 0 && nbrInsts)
                {
                    gtUInt32 instStartOffset = functionData.m_instDataList[0].m_offset;
                    startOffset = (instStartOffset < startOffset) ? instStartOffset : startOffset;
                    functionSize = functionData.m_instDataList[nbrInsts - 1].m_offset - instStartOffset;

                    functionData.m_functionInfo.m_size = functionSize;
                    functionData.m_functionInfo.m_startOffset = startOffset;
                }

                auto it = m_funcIdInfoMap.find(funcId);

                if (it == m_funcIdInfoMap.end())
                {
                    m_funcIdInfoMap.insert({ funcId, functionData.m_functionInfo });
                }

                gtString srcFilePath;
                AMDTSourceAndDisasmInfoVec srcInfoVec;
                ret = GetFunctionSourceAndDisasmInfo(funcId, srcFilePath, srcInfoVec);

                AMDTProfileSourceLineDataVec& srcLineDataVec = functionData.m_srcLineDataList;
                AMDTProfileInstructionDataVec& instDataVec = functionData.m_instDataList;

                for (auto& srcLineInfo : srcInfoVec)
                {
                    gtUInt32 srcLine = srcLineInfo.m_sourceLine;
                    gtUInt32 offset = srcLineInfo.m_offset;

                    // check whether this offset has samples (in functionData.m_instDataList)
                    auto instData = std::find_if(instDataVec.begin(), instDataVec.end(),
                        [&offset](AMDTProfileInstructionData const& iData) { return iData.m_offset == offset; });

                    if (instData != instDataVec.end())
                    {
                        // Check wether we have see this srcline in srcLineDataVec
                        // if so, update the profile data otherwise add a new line
                        auto slData = std::find_if(srcLineDataVec.begin(), srcLineDataVec.end(),
                            [&srcLine](AMDTProfileSourceLineData const& sData) { return sData.m_sourceLineNumber == srcLine; });

                        if (slData == srcLineDataVec.end())
                        {
                            AMDTProfileSourceLineData srcLineData;
                            srcLineData.m_sourceLineNumber = srcLine;
                            srcLineData.m_sampleValues = instData->m_sampleValues;

                            srcLineDataVec.push_back(srcLineData);
                        }
                        else
                        {
                            for (gtUInt32 i = 0; i < instData->m_sampleValues.size(); i++)
                            {
                                slData->m_sampleValues[i].m_sampleCount += instData->m_sampleValues[i].m_sampleCount;
                            }
                        }
                    }
                }

                // Now calculate the percentage
                ret = CalculateRawCounterPercentage(funcId, processId, threadId, countersList, functionData);

                // Now calculate the computed counter values
                // Iterate over the profileDataVec and calculate the computed counter values
                ret = ret && CalculateComputedCounters(functionData.m_srcLineDataList);

                ret = ret && CalculateComputedCounters(functionData.m_instDataList);
            }
        }

        return ret;
    }

    bool GetSrcFilePathForVaddr(ExecutableFile& exeFile, gtVAddr rvAddr, gtString& srcFilePath, gtUInt32& srcLine)
    {
        bool ret = false;
        SymbolEngine* pSymbolEngine = exeFile.GetSymbolEngine();
        SourceLineInfo srcData;

        if (nullptr != pSymbolEngine)
        {
            if (pSymbolEngine->FindSourceLine(rvAddr, srcData))
            {
                srcFilePath = srcData.m_filePath;
                srcLine = srcData.m_line;

                ret = true;
            }
        }

        return ret;
    }

    bool GetSrcFilePathForFuncId(AMDTFunctionId funcId, gtString& srcFilePath, gtUInt32& srcLine)
    {
        bool ret = false;
        AMDTModuleId moduleId = 0;
        gtUInt32 offset = 0;

        auto funcInfoIt = m_funcIdInfoMap.find(funcId);

        if (funcInfoIt != m_funcIdInfoMap.end())
        {
            // profile data has been fetched for this function..
            AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;

            moduleId = funcInfo.m_moduleId;
            offset = funcInfo.m_startOffset;

            ret = true;
        }

        if (ret && nullptr != m_pDbAdapter)
        {
            AMDTProfileModuleInfo modInfo;

            ret = GetModuleInfo(moduleId, modInfo);

            gtVAddr loadAddress = modInfo.m_loadAddress;
            gtString exePath = modInfo.m_path;
            ExecutableFile* pExecutable = ExecutableFile::Open(exePath.asCharArray(), loadAddress);

            if (nullptr != pExecutable)
            {
                InitializeSymbolEngine(pExecutable);
                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                if (nullptr != pSymbolEngine)
                {
                    gtRVAddr startRVAddr = pExecutable->VaToRva(loadAddress + offset);
                    unsigned int sectionIndex = pExecutable->LookupSectionIndex(startRVAddr);

                    if (pExecutable->GetSectionsCount() > sectionIndex)
                    {
                        SourceLineInfo srcData;

                        if (pSymbolEngine->FindSourceLine(startRVAddr, srcData))
                        {
                            srcFilePath = srcData.m_filePath;
                            srcLine = srcData.m_line;

                            ret = true;
                        }
                    }
                }

                delete pExecutable;
            }
        }

        return ret;
    }

    bool GetSrcFilePathForFuncId(AMDTFunctionId funcId, gtString& srcFilePath)
    {
        gtUInt32 srcLine;
        return GetSrcFilePathForFuncId(funcId, srcFilePath, srcLine);
    }

    // TODO: Bool return is not sufficient
    bool GetFunctionSourceAndDisasmInfo(AMDTFunctionId funcId,
                                        gtString& srcFilePath,
                                        AMDTSourceAndDisasmInfoVec& srcInfoVec)
    {
        bool ret = false;
        bool foundSrcInfo = false;
        bool foundSrcFilePath = false;

        auto funcSrcInfoIt = m_funcIdSrcInfoMap.find(funcId);

        if (funcSrcInfoIt != m_funcIdSrcInfoMap.end())
        {
            srcInfoVec = funcSrcInfoIt->second;
            foundSrcInfo = true;
        }

        if (!foundSrcInfo)
        {
            AMDTModuleId moduleId = 0;
            gtUInt32 offset = 0;
            gtUInt32 size = 0;

            auto funcInfoIt = m_funcIdInfoMap.find(funcId);

            // TODO: Currently there is a limitation that GetFunctionDetailedProfileData()
            // should be called before GetFunctionSourceAndDisasmInfo()
            if (funcInfoIt != m_funcIdInfoMap.end())
            {
                // profile data has been fetched for this function..
                AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;

                moduleId = funcInfo.m_moduleId;
                offset = funcInfo.m_startOffset;
                size = funcInfo.m_size;

                ret = true;
            }

            ret = ret && GetDisassembly(moduleId, offset, size, srcInfoVec);
        }

        if (!foundSrcFilePath)
        {
            // Find the srcFilePath
            foundSrcFilePath = GetSrcFilePathForFuncId(funcId, srcFilePath);
        }

        return ret;
    }

    bool GetDisassembly(AMDTModuleId moduleId,
                        AMDTUInt32 offset,
                        AMDTUInt32 size,
                        AMDTSourceAndDisasmInfoVec& disasmInfoVec)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            AMDTProfileModuleInfo modInfo;

            ret = GetModuleInfo(moduleId, modInfo);

            gtString exePath = modInfo.m_path;

            gtVAddr loadAddress = modInfo.m_loadAddress;
            ExecutableFile* pExecutable = ExecutableFile::Open(exePath.asCharArray(), loadAddress);

            if (nullptr != pExecutable)
            {
                gtRVAddr startRVAddr = 0;
                const gtUByte* pCode = nullptr;
                gtRVAddr sectionStartRva = 0, sectionEndRva = 0;
                unsigned int prevSectionIndex = static_cast<unsigned int>(-1);

                // Setup disassembler
                LibDisassembler dasm;
                // if the code is 64-bits
                bool isLongMode = pExecutable->Is64Bit();
                dasm.SetLongMode(isLongMode);

                AMDTUInt32 bytesToRead = size;
                gtVAddr currOffset = offset;

                InitializeSymbolEngine(pExecutable);
                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                while (bytesToRead > 0)
                {
                    AMDTSourceAndDisasmInfo disasmInfo;

                    startRVAddr = pExecutable->VaToRva(loadAddress + currOffset);
                    unsigned int sectionIndex = pExecutable->LookupSectionIndex(startRVAddr);

                    BYTE error_code;
                    UIInstInfoType temp_struct;
                    memset(&temp_struct, 0, sizeof(temp_struct));
                    char dasmArray[256] = { 0 };
                    unsigned int strlength = 255;

                    gtRVAddr codeOffset = startRVAddr;
                    const gtUByte* pCurrentCode = pCode;
                    HRESULT hr = E_FAIL;
                    SourceLineInfo srcData;

                    if (nullptr != pSymbolEngine)
                    {
                        pSymbolEngine->FindSourceLine(startRVAddr, srcData);
                        disasmInfo.m_sourceLine = srcData.m_line;
                    }

                    if (pExecutable->GetSectionsCount() > sectionIndex)
                    {
                        if (sectionIndex == prevSectionIndex)
                        {
                            codeOffset = startRVAddr - sectionStartRva;
                            pCurrentCode = pCode + codeOffset;

                            // Get disassembly for the current pCode from the disassembler
                            hr = dasm.UIDisassemble((BYTE*)pCurrentCode, (unsigned int*)&strlength, (BYTE*)dasmArray, &temp_struct, &error_code);
                        }
                        else
                        {
                            pCode = pExecutable->GetSectionBytes(sectionIndex);
                            pExecutable->GetSectionRvaLimits(sectionIndex, sectionStartRva, sectionEndRva);

                            // GetCodeBytes return the pointer to the sectionStart
                            // We need to add the offset to the beginning of the function
                            codeOffset = startRVAddr - sectionStartRva;
                            pCurrentCode = pCode + codeOffset;

                            // Get disassembly for the current pCode from the disassembler
                            hr = dasm.UIDisassemble((BYTE*)pCurrentCode, (unsigned int*)&strlength, (BYTE*)dasmArray, &temp_struct, &error_code);

                            prevSectionIndex = sectionIndex;
                        }
                    }

                    if (S_OK == hr)
                    {
                        currOffset += temp_struct.NumBytesUsed;
                        bytesToRead -= temp_struct.NumBytesUsed;

                        gtString& disasmStr = disasmInfo.m_disasmStr;
                        // disasmInfo.m_offset = codeOffset + startRVAddr;
                        disasmInfo.m_offset = codeOffset + sectionStartRva;
                        disasmStr.fromASCIIString(dasmArray);

                        if (temp_struct.bIsPCRelative && temp_struct.bHasDispData)
                        {
                            disasmStr.appendFormattedString(L"(0x%lx)", disasmInfo.m_offset + temp_struct.NumBytesUsed + temp_struct.DispDataValue);
                        }

                        // Get codebytes
                        char codeBytes[16];
                        memcpy(codeBytes, pCurrentCode, temp_struct.NumBytesUsed);

                        for (int count = 0; count < temp_struct.NumBytesUsed; count++)
                        {
                            gtUByte byteCode = codeBytes[count];

                            gtUByte btHigh = (byteCode >> 4);
                            gtUByte btLow = (byteCode & 0xF);

                            disasmInfo.m_codeByteStr.append((btHigh <= 9) ? ('0' + btHigh) : ('A' + btHigh - 0xA));
                            disasmInfo.m_codeByteStr.append((btLow <= 9) ? ('0' + btLow) : ('A' + btLow - 0xA));

                            disasmInfo.m_codeByteStr << L" ";
                        }
                    }
                    else
                    {
                        disasmInfo.m_disasmStr.fromASCIIString("BAD DASM");

                        // Once hit, don't continue further
                        bytesToRead = 0;
                    }

                    disasmInfoVec.push_back(disasmInfo);
                }

                delete pExecutable;
            }
        }

        return ret;
    }

    bool GetCallGraphProcesses(gtVector<AMDTProcessId>& cssProcesses)
    {
        bool ret = true;

        if (!m_foundCssProcesses)
        {
            ret = m_pDbAdapter->GetProcessesWithCallstackSamples(m_cssProcesses);
            m_foundCssProcesses = true;
        }

        if (ret)
        {
            cssProcesses = m_cssProcesses;
        }

        return ret;
    }

    bool IsProcessHasCssSamples(AMDTProcessId pid)
    {
        gtVector<AMDTProcessId> cssProcesses;
        bool ret = GetCallGraphProcesses(cssProcesses);

        // Check whether this PID has callstack samples
        auto cssPid = std::find_if(cssProcesses.begin(), cssProcesses.end(),
            [&pid](AMDTProcessId const& aData) { return aData == pid; });

        ret = (cssPid != m_cssProcesses.end()) ? true : false;

        return ret;
    }

    bool GetMaxFunctionIdByModuleId(AMDTModuleId moduleId, gtUInt32& maxFuncId)
    {
        bool ret = false;
        gtUInt32 funcId = 0;

        auto it = m_moduleIdMaxFuncIdMap.find(moduleId);

        if (it == m_moduleIdMaxFuncIdMap.end())
        {
            ret = m_pDbAdapter->GetMaxFunctionId(moduleId, funcId);

            if (ret)
            {
                m_moduleIdMaxFuncIdMap.insert({ moduleId, funcId });
            }
        }
        else
        {
            funcId = it->second;
            ret = true;
        }

        maxFuncId = funcId;
        return ret;
    }

    bool GetFunctionNode(functionIdcgNodeMap& nodeMap, CallstackFrame& frame, gtUInt32 deepSamples, gtUInt32 pathCount, cgNode*& funcNode)
    {
        bool ret = false;
        AMDTFunctionId funcId = frame.m_funcInfo.m_functionId;

        Lookupfunction(frame.m_funcInfo);

        auto it = nodeMap.find(funcId);

        if (nodeMap.end() == it)
        {
            cgNode node;
            node.m_funcInfo = frame.m_funcInfo;

            nodeMap.insert({ funcId, node });

            it = nodeMap.find(funcId);
        }

        if (nodeMap.end() != it)
        {
            if (frame.m_isLeaf)
            {
                it->second.m_totalSelfSamples.m_sampleCount += frame.m_selfSamples;
                it->second.m_totalDeepSamples.m_sampleCount += frame.m_selfSamples;
            }
            else
            {
                // Set the deep samples in the function node
                it->second.m_totalDeepSamples.m_sampleCount += deepSamples;
            }

            it->second.m_pathCount += pathCount;
            it->second.m_moduleBaseAddr = frame.m_moduleBaseAddr;

            funcNode = &(it->second);
            ret = true;
        }

        return ret;
    }

    bool AddRootNode(functionIdcgNodeMap& nodeMap, CallstackFrame& frame, cgNode*& rootNode)
    {
        bool ret = false;

        frame.m_funcInfo.m_functionId = CXL_ROOT_FUNCTION_ID;  // "[ROOT]" node
        frame.m_funcInfo.m_name = CXL_ROOT_FUNCTION_NAME;
        frame.m_isLeaf = false;
        frame.m_callstackId = 0;
        frame.m_moduleBaseAddr = 0;
        frame.m_depth = 0;
        frame.m_selfSamples = 0;

        ret = GetFunctionNode(nodeMap, frame, 0, 0, rootNode);
        rootNode->m_pathCount = 0;

        return ret;
    }

    bool GetEdge(cgEdgeVec& edgeVec, AMDTProfileFunctionInfo& funcInfo, cgEdge*& pEdge)
    {
        bool ret = true;

        AMDTFunctionId funcId = funcInfo.m_functionId;

        auto itEdge = std::find_if(edgeVec.begin(), edgeVec.end(),
            [&funcId](cgEdge const& aEdge) { return aEdge.m_funcInfo.m_functionId == funcId; });

        // if edge is found
        if (edgeVec.end() != itEdge)
        {
            pEdge = &(*itEdge);
        }
        else
        {
            cgEdge aEdge;

            aEdge.m_funcInfo = funcInfo;

            edgeVec.push_back(aEdge);

            auto iterEdge = std::find_if(edgeVec.begin(), edgeVec.end(),
                [&funcId](cgEdge const& aEdge) { return aEdge.m_funcInfo.m_functionId == funcId; });

            pEdge = &(*iterEdge);
        }

        return ret;
    }

    bool AddCallerToCalleeEdge(cgNode*& caller, cgNode*& callee, double& sampleCount, bool isCalleeLeaf)
    {
        bool ret = false;

        // a directed forward edge from caller to callee (in caller's in callee-edge vector)
        cgEdge* pEdge = nullptr;
        ret = GetEdge(caller->m_calleeVec, callee->m_funcInfo, pEdge);

        if (ret && nullptr != pEdge)
        {
            pEdge->m_moduleBaseAddr = callee->m_moduleBaseAddr; // Hmm.. inefficient
            pEdge->m_selfSamples.m_sampleCount += (isCalleeLeaf) ? sampleCount : 0;

            pEdge->m_deepSamples.m_sampleCount += sampleCount;
        }

        return ret;
    }

    bool AddCalleeToCallerEdge(cgNode*& caller, cgNode*& callee, double& sampleCount)
    {
        bool ret = false;

        //  a directed backeward edge from callee to caller (in callee's caller-edge vector)
        cgEdge* pEdge = nullptr;
        ret = GetEdge(callee->m_callerVec, caller->m_funcInfo, pEdge);

        if (ret && nullptr != pEdge)
        {
            pEdge->m_moduleBaseAddr = caller->m_moduleBaseAddr; // Hmm.. inefficient
            pEdge->m_deepSamples.m_sampleCount += sampleCount;
        }

        return ret;
    }

    // Returns true if callgraph is constructed
    bool GetNodeFunctionMap(AMDTProcessId pid, AMDTCounterId cid, functionIdcgNodeMap*& pFuncNodeMap)
    {
        bool ret = false;
        functionIdcgNodeMap *pNodeMap = nullptr;

        gtUInt64 pidCounterId;
        GET_PID_COUNTER_ID(pidCounterId, pid, cid);

        auto funcMap = m_pidCgFunctionMap.find(pidCounterId);

        if (funcMap != m_pidCgFunctionMap.end())
        {
            pNodeMap = funcMap->second;
            ret = true;
        }
        else
        {
            pNodeMap = new functionIdcgNodeMap;
            m_pidCgFunctionMap.insert({ pidCounterId, pNodeMap });
        }

        pFuncNodeMap = pNodeMap;
        return ret;
    }

    // Initially the CallstackLeaf table may contain unknown functions
    // Fix them, if possible
    bool UpdateUnknownCallstackLeafs(AMDTProcessId pid)
    {
        bool ret = false;

        auto handleUnknownLeaf = m_handleUnknownLeafs.find(pid);

        if (handleUnknownLeaf == m_handleUnknownLeafs.end())
        {
            CallstackFrameVec leafs;
            ret = m_pDbAdapter->GetUnknownCallstackLeafsByProcessId(pid, leafs);

            if (ret)
            {
                for (auto& leaf : leafs)
                {
                    // Find the function info from debug info (if available) and update the tables
                    ret = Lookupfunction(leaf.m_funcInfo);
                }

                m_handleUnknownLeafs.insert({ pid, pid });
            }
        }

        return ret;
    }

    // Callgraph
    bool ConstructCallGraph(AMDTProcessId pid, AMDTCounterId counterId)
    {
        bool ret = false;

        if (IsProcessHasCssSamples(pid))
        {
            functionIdcgNodeMap* pCgFunctionMap = nullptr;

            ret = GetNodeFunctionMap(pid, counterId, pCgFunctionMap);

            // Callgraph is not yet constructed
            if (!ret && (nullptr != pCgFunctionMap))
            {
                // Get list of Leaf nodes for the given process
                // For each Leaf node
                //      get the callstack frames 
                //      construct the arc details and update node and edge details

                UpdateUnknownCallstackLeafs(pid);

                // Add [ROOT] Node
                cgNode* pRootNode = nullptr;
                CallstackFrame dummyRootFrame;
                ret = AddRootNode(*pCgFunctionMap, dummyRootFrame, pRootNode);

                CallstackFrameVec leafs;
                ret = m_pDbAdapter->GetCallstackLeafData(pid, counterId, AMDT_PROFILE_ALL_CALLPATHS, leafs);

                if (ret)
                {
                    AMDTFunctionId prevFunctionId = 0;
                    gtUInt32 prevCallstackId = 0;

                    for (auto& leaf : leafs)
                    {
                        // Get the function Node
                        cgNode *pLeafNode = nullptr;
                        double sampleCount = leaf.m_selfSamples;
                        gtUInt32 pathCount = ((prevFunctionId == leaf.m_funcInfo.m_functionId) && (prevCallstackId == leaf.m_callstackId)) ? 0 : 1;

                        ret = GetFunctionNode(*pCgFunctionMap, leaf, sampleCount, pathCount, pLeafNode);

                        if (ret && nullptr != pLeafNode)
                        {
                            // get the callstack frames (in reverse order - depth (n-1) to 1)
                            CallstackFrameVec frames;
                            ret = m_pDbAdapter->GetCallstackFrameData(pid, leaf.m_callstackId, frames);

                            if (ret)
                            {
                                cgNode* pCallerNode = nullptr;
                                cgNode* pCalleeNode = pLeafNode;
                                bool isCalleeLeaf = true;

                                for (auto& frame : frames)
                                {
                                    // Handle recursion.
                                    if (frame.m_funcInfo.m_functionId != pCalleeNode->m_funcInfo.m_functionId)
                                    {
                                        pCallerNode = nullptr;
                                        ret = GetFunctionNode(*pCgFunctionMap, frame, sampleCount, pathCount, pCallerNode);

                                        ret = ret && AddCallerToCalleeEdge(pCallerNode, pCalleeNode, sampleCount, isCalleeLeaf);
                                        ret = ret && AddCalleeToCallerEdge(pCallerNode, pCalleeNode, sampleCount);

                                        pCalleeNode = pCallerNode;
                                        isCalleeLeaf = false;
                                    }
                                }

                                // Connect this to [ROOT] node
                                ret = GetFunctionNode(*pCgFunctionMap, dummyRootFrame, sampleCount, pathCount, pCallerNode);
                                ret = ret && AddCallerToCalleeEdge(pCallerNode, pCalleeNode, sampleCount, false);
                                ret = ret && AddCalleeToCallerEdge(pCallerNode, pCalleeNode, sampleCount);
                            }

                            prevFunctionId = leaf.m_funcInfo.m_functionId;
                            prevCallstackId = leaf.m_callstackId;
                        }
                    }
                }
            }

            m_cgCounterId = (ret) ? counterId : 0;
        }

        return ret;
    }

    bool CopyCGNode(AMDTCallGraphFunction& cgFunc, cgNode& node, gtUInt32 totalDeepSamples)
    {
        cgFunc.m_functionInfo = node.m_funcInfo;
        cgFunc.m_moduleBaseAddr = node.m_moduleBaseAddr;
        cgFunc.m_pathCount = node.m_pathCount;
        
        cgFunc.m_totalDeepSamples = node.m_totalDeepSamples.m_sampleCount;
        cgFunc.m_totalSelfSamples = node.m_totalSelfSamples.m_sampleCount;

        if (cgFunc.m_totalDeepSamples > 0 && totalDeepSamples > 0)
        {
            cgFunc.m_deepSamplesPerc = (cgFunc.m_totalDeepSamples / (double)(totalDeepSamples)) * 100.0;
        }

        GetSrcFilePathForFuncId(node.m_funcInfo.m_functionId, cgFunc.m_srcFile, cgFunc.m_srcFileLine);

        return true;
    }

    bool CopyCGEdge(AMDTCallGraphFunction& cgFunc, cgEdge& edge, gtUInt32 totalDeepSamples)
    {
        cgFunc.m_functionInfo = edge.m_funcInfo;
        cgFunc.m_moduleBaseAddr = edge.m_moduleBaseAddr;
        cgFunc.m_pathCount = 0;

        cgFunc.m_totalDeepSamples = edge.m_deepSamples.m_sampleCount;
        cgFunc.m_totalSelfSamples = edge.m_selfSamples.m_sampleCount;

        if (cgFunc.m_totalDeepSamples > 0 && totalDeepSamples > 0)
        {
            cgFunc.m_deepSamplesPerc = (cgFunc.m_totalDeepSamples / (double)(totalDeepSamples)) * 100.0;
        }

        GetSrcFilePathForFuncId(edge.m_funcInfo.m_functionId, cgFunc.m_srcFile, cgFunc.m_srcFileLine);

        return true;
    }

    bool CopyCGSelf(AMDTCallGraphFunction& cgFunc, cgNode& node, gtUInt32 totalDeepSamples)
    {
        cgFunc.m_functionInfo = node.m_funcInfo;
        // override the name to [self]
        cgFunc.m_functionInfo.m_name = L"[self]";
        cgFunc.m_moduleBaseAddr = node.m_moduleBaseAddr;
        cgFunc.m_pathCount = node.m_pathCount;

        cgFunc.m_totalDeepSamples = node.m_totalDeepSamples.m_sampleCount;
        cgFunc.m_totalSelfSamples = node.m_totalSelfSamples.m_sampleCount;

        // !!! Note !!!
        //  For "[self]" entry the m_deepSamplesPerc denotes the percentage of self samples w.r.t to deep sample of the function
        if (cgFunc.m_totalSelfSamples > 0 && totalDeepSamples > 0)
        {
            cgFunc.m_deepSamplesPerc = (cgFunc.m_totalSelfSamples / (double)(totalDeepSamples)) * 100.0;
        }

        GetSrcFilePathForFuncId(node.m_funcInfo.m_functionId, cgFunc.m_srcFile, cgFunc.m_srcFileLine);

        return true;
    }

    bool CopyCGFunction(AMDTCallGraphFunction& cgFunc, CallstackFrame& frame, gtUInt32 samples)
    {
        cgFunc.m_functionInfo = frame.m_funcInfo;
        cgFunc.m_moduleBaseAddr = frame.m_moduleBaseAddr;
        cgFunc.m_pathCount = 0;

        cgFunc.m_totalDeepSamples = samples;
        cgFunc.m_totalSelfSamples = (frame.m_isLeaf) ? samples : 0;

        GetSrcFilePathForFuncId(frame.m_funcInfo.m_functionId, cgFunc.m_srcFile, cgFunc.m_srcFileLine);

        return true;
    }

    bool GetCallGraphFunctions(AMDTProcessId pid, AMDTCounterId counterId, AMDTCallGraphFunctionVec& cgFuncsVec)
    {
        bool ret = false;

        ret = ConstructCallGraph(pid, counterId);

        if (ret)
        {
            functionIdcgNodeMap* pCgFunctionMap = nullptr;

            ret = GetNodeFunctionMap(pid, counterId, pCgFunctionMap);

            if (ret && (nullptr != pCgFunctionMap))
            {
                auto rootFuncIter = pCgFunctionMap->find(CXL_ROOT_FUNCTION_ID);
                gtUInt32 totalDeepSamples = 0;

                if (pCgFunctionMap->end() != rootFuncIter)
                {
                    totalDeepSamples = rootFuncIter->second.m_totalDeepSamples.m_sampleCount;
                }

                for (auto& cgFunc : *pCgFunctionMap)
                {
                    AMDTCallGraphFunction func;
                    CopyCGNode(func, cgFunc.second, totalDeepSamples);
                    cgFuncsVec.push_back(func);
                }
            }
        }

        return ret;
    }

    bool GetCallGraphFunctionInfo(AMDTProcessId processId, AMDTFunctionId funcId, AMDTCallGraphFunctionVec& parents, AMDTCallGraphFunctionVec& children)
    {
        GT_UNREFERENCED_PARAMETER(processId);
        bool ret = false;

        functionIdcgNodeMap* pCgFunctionMap = nullptr;


        ret = GetNodeFunctionMap(processId, m_cgCounterId, pCgFunctionMap);

        if (ret && (nullptr != pCgFunctionMap))
        {
            auto cgFuncIter = pCgFunctionMap->find(funcId);

            if (pCgFunctionMap->end() != cgFuncIter)
            {
                gtUInt32 totalDeepSamples = cgFuncIter->second.m_totalDeepSamples.m_sampleCount;

                for (auto& parent : cgFuncIter->second.m_callerVec)
                {
                    AMDTCallGraphFunction parentFunc;
                    CopyCGEdge(parentFunc, parent, totalDeepSamples);

                    parents.push_back(parentFunc);
                }

                for (auto& child : cgFuncIter->second.m_calleeVec)
                {
                    AMDTCallGraphFunction childFunc;
                    CopyCGEdge(childFunc, child, totalDeepSamples);

                    children.push_back(childFunc);
                }

                // TBD: Should this [self] entry be added?
                // Add [self] in children vector
                AMDTCallGraphFunction self;
                CopyCGSelf(self, cgFuncIter->second, totalDeepSamples);
                children.push_back(self);

                ret = true;
            }
        }

        return ret;
    }

    bool GetCallGraphPaths(AMDTProcessId processId, AMDTFunctionId funcId, gtVector<AMDTCallGraphPath>& paths)
    {
        bool ret = false;
        gtVector<gtUInt32>  csIds;

        ret = m_pDbAdapter->GetCallstackIds(processId, funcId, csIds);

        for (auto& csId : csIds)
        {
            // Get the leaf node
            CallstackFrameVec leafs;    // There will only one entry
            ret = m_pDbAdapter->GetCallstackLeafData(processId, m_cgCounterId, csId, leafs);
            
            if (ret && leafs.size() > 0)
            {
                CallstackFrame leaf = leafs[0];
                gtUInt32 samples = leaf.m_selfSamples;

                // Get the callstack/callpath for this csId
                CallstackFrameVec frames;
                ret = m_pDbAdapter->GetCallstackFrameData(processId, csId, frames);

                // construct callpath
                AMDTCallGraphPath aPath;
                
                for (auto frameIt = frames.rbegin(); frameIt != frames.rend(); ++frameIt)
                {
                    AMDTCallGraphFunction cgFunc;

                    CopyCGFunction(cgFunc, (*frameIt), samples);
                    aPath.push_back(cgFunc);
                }

                // add the leaf node
                AMDTCallGraphFunction leafFunc;
                CopyCGFunction(leafFunc, leaf, samples);
                aPath.push_back(leafFunc);

                paths.push_back(aPath);
            }
        }

        return ret;
    }

    bool GetModuleExecutable(AMDTModuleId moduleId, ExecutableFile*& pExecutable, bool initSymbolEngine)
    {
        bool ret = true;
        ExecutableFile* pExe = nullptr;

        auto exeIt = m_moduleIdExeMap.find(moduleId);

        if (exeIt != m_moduleIdExeMap.end())
        {
            pExe = exeIt->second;
        }
        else
        {
            AMDTProfileModuleInfo modInfo;
            ret = GetModuleInfo(moduleId, modInfo);

            if (ret)
            {
                pExe = ExecutableFile::Open(modInfo.m_path.asCharArray(), modInfo.m_loadAddress);

                if (nullptr != pExe)
                {
                    ret = (initSymbolEngine) ? InitializeSymbolEngine(pExe) : true;
                    m_moduleIdExeMap.insert({ moduleId, pExe });
                }
            }
        }

        pExecutable = pExe;
        return ret;
    }

    bool Lookupfunction(AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = true;
        gtUInt32 symEngineFuncId = funcInfo.m_functionId & 0xFFFFUL;

        if (0 == symEngineFuncId)
        {
            ExecutableFile* pExecutable = nullptr;

            ret = GetModuleExecutable(funcInfo.m_moduleId, pExecutable, true);

            if (ret && nullptr != pExecutable)
            {
                ret = false;
                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                gtRVAddr funcRvaEnd = GT_INVALID_RVADDR;
                const FunctionSymbolInfo* pFuncSymbol = pSymbolEngine->LookupFunction(funcInfo.m_startOffset, &funcRvaEnd);

                if (NULL != pFuncSymbol)
                {
                    gtUInt32 funcSize = pFuncSymbol->m_size;

                    // TODO: if funcSize == 0 ?
                    funcSize = ((funcSize == 0) && (GT_INVALID_RVADDR != funcRvaEnd))
                                             ? funcRvaEnd - pFuncSymbol->m_rva : funcSize;

                    funcInfo.m_name = pFuncSymbol->m_pName;
                    funcInfo.m_startOffset = pFuncSymbol->m_rva;
                    funcInfo.m_size = funcSize;
                    gtUInt32 maxFuncId = 0;

                    GetMaxFunctionIdByModuleId(funcInfo.m_moduleId, maxFuncId);
                    funcInfo.m_functionId = pFuncSymbol->m_funcId + maxFuncId;

                    // TODO: Update CallstackFrame table and insert into Function table
                    m_pDbAdapter->InsertFunctionInfo(funcInfo);
                    m_pDbAdapter->UpdateCallstackLeaf(funcInfo);
                    m_pDbAdapter->UpdateCallstackFrame(funcInfo);

                    ret = true;
                }
            }
        }

        return ret;
    }

};


cxlProfileDataReader::cxlProfileDataReader() : m_pImpl(new cxlProfileDataReader::Impl())
{

}

cxlProfileDataReader::~cxlProfileDataReader()
{
    delete m_pImpl;
    m_pImpl = nullptr;
}

bool cxlProfileDataReader::OpenProfileData(gtString profileFilePath)
{
    bool ret = false;

    // TODO: Check for validity of the profile file path

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->OpenProfileData(profileFilePath);
    }

    return ret;
}

bool cxlProfileDataReader::CloseProfileData()
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->CloseProfileData();
    }

    return ret;
}

bool cxlProfileDataReader::GetProfileSessionInfo(AMDTProfileSessionInfo& sessionInfo)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetProfileSessionInfo(sessionInfo);
    }

    return ret;
}

bool cxlProfileDataReader::GetCpuTopology(AMDTCpuTopologyVec& cpuTopologyVec)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetCpuTopology(cpuTopologyVec);
    }

    return ret;
}

bool cxlProfileDataReader::GetSampledCountersList(AMDTProfileCounterDescVec& counterDesc)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetSampledCountersList(counterDesc);
    }

    return ret;
}

bool cxlProfileDataReader::GetSamplingConfiguration(AMDTUInt32 counterId, AMDTProfileSamplingConfig& samplingConfig)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetSamplingConfiguration(counterId, samplingConfig);
    }

    return ret;
}

bool cxlProfileDataReader::GetReportConfigurations(AMDTProfileReportConfigVec& reportConfig)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetReportConfigurations(reportConfig);
    }

    return ret;
}

bool cxlProfileDataReader::SetDebugInfoPaths(gtVector<gtString>& symbolServer, gtVector<gtString>& symbolDirectory)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetDebugInfoPaths(symbolServer, symbolDirectory);
    }

    return ret;
}

bool cxlProfileDataReader::SetSourcePaths(gtVector<gtString>& sourceDirPath)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetSourcePaths(sourceDirPath);
    }

    return ret;
}

bool cxlProfileDataReader::SetReportOption(AMDTProfileDataOptions& options)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetReportOption(options);
    }

    return ret;
}

bool cxlProfileDataReader::SetReportOption(AMDTReportOptionType type, gtUInt64 value)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetReportOption(type, value);
    }

    return ret;
}

bool cxlProfileDataReader::SetReportCounters(gtVector<AMDTUInt32>& countersList)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetReportCounters(countersList);
    }

    return ret;
}


bool cxlProfileDataReader::GetProcessInfo(AMDTUInt32 pid, AMDTProfileProcessInfoVec& procInfo)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetProcessInfo(pid, procInfo);
    }

    return ret;
}

bool cxlProfileDataReader::GetModuleInfo(AMDTUInt32 pid, AMDTModuleId mid, AMDTProfileModuleInfoVec& modInfo)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetModuleInfo(pid, mid, modInfo);
    }

    return ret;
}

bool cxlProfileDataReader::GetThreadInfo(AMDTUInt32 pid, AMDTThreadId tid, AMDTProfileThreadInfoVec& threadInfo)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetThreadInfo(pid, tid, threadInfo);
    }

    return ret;
}

bool cxlProfileDataReader::GetProcessSummary(AMDTUInt32 counterId, AMDTProfileDataVec& processSummaryData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetProcessSummary(counterId, processSummaryData);
    }

    return ret;
}

bool cxlProfileDataReader::GetThreadSummary(AMDTUInt32 counterId, AMDTProfileDataVec& threadSummaryData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetThreadSummary(counterId, threadSummaryData);
    }

    return ret;
}

bool cxlProfileDataReader::GetModuleSummary(AMDTUInt32 counterId, AMDTProfileDataVec& moduleSummaryData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetModuleSummary(counterId, moduleSummaryData);
    }

    return ret;
}

bool cxlProfileDataReader::GetFunctionSummary(AMDTUInt32 counterId, AMDTProfileDataVec& funcSummaryData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetFunctionSummary(counterId, funcSummaryData);
    }

    return ret;
}

// Process/Module/Funtion View APIs
bool cxlProfileDataReader::GetProcessProfileData(AMDTProcessId procId, AMDTProfileDataVec& processProfileData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetProcessProfileData(procId, processProfileData);
    }

    return ret;
}

bool cxlProfileDataReader::GetModuleProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& moduleProfileData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetModuleProfileData(procId, modId, moduleProfileData);
    }

    return ret;
}

bool cxlProfileDataReader::GetFunctionProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& funcProfileData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetFunctionProfileData(procId, modId, funcProfileData);
    }

    return ret;
}

bool cxlProfileDataReader::GetFunctionDetailedProfileData(AMDTFunctionId            funcId,
                                                          AMDTProcessId             processId,
                                                          AMDTThreadId              threadId,
                                                          AMDTProfileFunctionData&  functionData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        if (funcId != 0xFFFFFFFFUL)
        {
            ret = m_pImpl->GetFunctionDetailedProfileData(funcId, processId, threadId, functionData);
        }
    }

    return ret;
}

bool cxlProfileDataReader::GetFunctionSourceAndDisasmInfo(AMDTFunctionId funcId, gtString& srcFilePath, AMDTSourceAndDisasmInfoVec& srcInfoVec)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetFunctionSourceAndDisasmInfo(funcId, srcFilePath, srcInfoVec);
    }

    return ret;
}

bool cxlProfileDataReader::GetDisassembly(AMDTModuleId moduleId,
                                          AMDTUInt32 offset,
                                          AMDTUInt32 size,
                                          AMDTSourceAndDisasmInfoVec& disasmInfoVec)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetDisassembly(moduleId, offset, size, disasmInfoVec);
    }

    return ret;
}

bool cxlProfileDataReader::GetCallGraphProcesses(gtVector<AMDTProcessId>& cssProcesses)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetCallGraphProcesses(cssProcesses);
    }

    return ret;
}

bool cxlProfileDataReader::IsProcessHasCssSamples(AMDTProcessId pid)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->IsProcessHasCssSamples(pid);
    }

    return ret;
}

bool cxlProfileDataReader::GetCallGraphFunctions(AMDTProcessId pid, AMDTCounterId counterId, AMDTCallGraphFunctionVec& cgFuncsVec)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetCallGraphFunctions(pid, counterId, cgFuncsVec);
    }

    return ret;
}

bool cxlProfileDataReader::GetCallGraphFunctionInfo(AMDTProcessId processId, AMDTFunctionId funcId, AMDTCallGraphFunctionVec& caller, AMDTCallGraphFunctionVec& callee)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetCallGraphFunctionInfo(processId, funcId, caller, callee);
    }

    return ret;
}

bool cxlProfileDataReader::GetCallGraphPaths(AMDTProcessId processId, AMDTFunctionId funcId, gtVector<AMDTCallGraphPath>& paths)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetCallGraphPaths(processId, funcId, paths);
    }

    return ret;
}
