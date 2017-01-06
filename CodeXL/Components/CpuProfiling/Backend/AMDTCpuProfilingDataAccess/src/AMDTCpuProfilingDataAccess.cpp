//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief AMDTProfilerDataAccess.cpp - APIs used to access the profile data stored in the db.
//
//=============================================================

#include <cstring>
#include <algorithm>

//#include <qtIgnoreCompilerWarnings.h>

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

#include <AMDTProfilingAgentsData/inc/JavaJncReader.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#include <AMDTProfilingAgentsData/inc/Windows/ClrJncReader.h>
#include <AMDTProfilingAgentsData/inc/Windows/PjsReader.h>
#endif // AMDT_WINDOWS_OS

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
#define CXL_COMPUTE_PERCENTAGE(val1_, val2_)     (((val1_) > 0.0) && ((val2_) > 0.0)) ? (((val1_) / (val2_)) * 100.0) : 0.0

// CLU specific events
#define CXL_CLU_EVENT_CLU_PERCENTAGE              0xFF00UL
#define CXL_CLU_EVENT_LINE_BOUNDARY_CROSSINGS     0xFF01UL
#define CXL_CLU_EVENT_BYTES_PER_L1_EVICTION       0xFF02UL
#define CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION    0xFF03UL
#define CXL_CLU_EVENT_L1_EVICTIONS                0xFF04UL
#define CXL_CLU_EVENT_ACCESSES                    0xFF05UL
#define CXL_CLU_EVENT_BYTES_ACCESSED              0xFF06UL

#define CXL_CLU_EVENT_CLU_PERCENTAGE_NAME_WSTR              L"Cache Line Utilization"
#define CXL_CLU_EVENT_CLU_PERCENTAGE_NAME_STR               "Cache Line Utilization"
#define CXL_CLU_EVENT_BYTES_PER_L1_EVICTION_NAME_WSTR       L"Bytes/L1 Eviction"
#define CXL_CLU_EVENT_BYTES_PER_L1_EVICTION_NAME_STR        "Bytes/L1 Eviction"
#define CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION_NAME_WSTR    L"Accesses/L1 Eviction"
#define CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION_NAME_STR     "Accesses/L1 Eviction"

#define CXL_PROFILE_TYPE_CLU_NAME_STR                       L"Cache Line Utilization"
#define IS_PROFILE_TYPE(type_)  ((0 == m_sessionInfo.m_sessionType.compareNoCase(type_)) ? true : false)

#define CXL_COMPUTED_COUNTER_START_ID       0x100

#define CXL_DB_MODULEID_MASK            0xFFFF0000
#define CXL_DB_MODULEID_SHIFT_BITS      16
#define CXL_GET_DB_MODULE_ID(funcId_)   (((funcId_) & CXL_DB_MODULEID_MASK) >> CXL_DB_MODULEID_SHIFT_BITS)

#define CXL_UNKNOWN_FUNC_START_ID           0xF000
#define IS_UNKNOWN_FUNC(id_) (((id_) & 0x0000FFFF) == 0)
#define GET_MODOFFSET_FOR_UNKNOWN_FUNC(mod_, offset_, val_) val_ = mod_; val_ = (val_ << 32) | offset_;
#define GET_MODULEID_FROM_FUNCTIONID(id_) (((id_) & 0xFFFF0000) >> 16)

#define CXL_MAX_DISASM_INSTS            2048
#define CXL_MAX_UNKNOWN_FUNC_SIZE       4096

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#define LONG_FORMAT_HEX     L"%lx"
#else
#define LONG_FORMAT_HEX     L"%llx"
#endif

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
using ModOffsetFuncInfoMap = gtMap<gtUInt64, AMDTProfileFunctionInfo>;

// Sample Totals
using ProcessSampleTotalMap = gtMap<AMDTProcessId, AMDTSampleValueVec>;
using ModuleSampleTotalMap = gtMap<AMDTModuleId, AMDTSampleValueVec>;


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
    gtString                            m_symbolServerPath;
    gtString                            m_symbolFilePath;
    gtString                            m_symbolDownloadPath;
    bool                                m_isDebugSearchPathAvailable = false;
    // Paths to locate the source files
    gtString                            m_sourceFilePath;

    // Path to locate binaries (used in import case)
    gtString                            m_binaryFilePath;

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
    AMDTCounterId                       m_computedCounterStartId = CXL_COMPUTED_COUNTER_START_ID;
    AMDTProfileCounterDescVec           m_computedCounterDescVec;

    ViewConfigInfoVec                   m_viewConfigInfoVec;
    AMDTProfileReportConfigVec          m_viewConfigs;

    AMDTProfileDataOptions              m_options;

    // Unknown functions
    FuncIdInfoMap                       m_unknownFuncIdInfoMap;
    ModOffsetFuncInfoMap                m_modOffsetFuncInfoMap;
    AMDTFunctionId                      m_currentUnknownFuncId = CXL_UNKNOWN_FUNC_START_ID;
    bool                                m_readUnknownFuncs = false;

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
    gtVector<cgNode*>                     m_cgNodeVisitedList;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    bool                                  m_bGetSystemDir = true;
    BOOL                                  m_is64BitSys = false;
    wchar_t                               m_system32Dir[OS_MAX_PATH + 1];
#endif // AMDT_WINDOWS_OS

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

        m_symbolServerPath.makeEmpty();
        m_symbolFilePath.makeEmpty();
        m_sourceFilePath.makeEmpty();
        m_binaryFilePath.makeEmpty();
        m_symbolDownloadPath.makeEmpty();

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

        for (auto srcInfo : m_funcIdSrcInfoMap)
        {
            srcInfo.second.clear();
        }

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

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
            IsWow64Process(GetCurrentProcess(), &m_is64BitSys);
#endif
#endif

            m_pDbAdapter = new amdtProfileDbAdapter;

            if (nullptr != m_pDbAdapter)
            {
                ret = m_pDbAdapter->OpenDb(profileFilePath, AMDT_PROFILE_MODE_AGGREGATION, false);

                ret = ret && PrepareDb();
            }
        }

        return ret;
    }

    // Handle the unknown IP samples and create the required views
    bool PrepareDb()
    {
        bool ret = false;

        ret = GetProfileSessionInfo(m_sessionInfo);

        ret = ret && InitializeReportOption();

        ret = GetUnknownFunctionsInfoFromIPSamples();

        ret = m_pDbAdapter->PrepareDb();

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

    bool isSessionTypeCLU()
    {
        return IS_PROFILE_TYPE(CXL_PROFILE_TYPE_CLU_NAME_STR);
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

    void ConstructEncodedEvent(AMDTProfileSamplingConfig& sampleConfig, EventMaskType& encodedEvent)
    {
        gtUInt16 eventSel = static_cast<gtUInt16>(sampleConfig.m_hwEventId);

        encodedEvent = EncodeEvent(eventSel,
                                   sampleConfig.m_unitMask,
                                   sampleConfig.m_osMode,
                                   sampleConfig.m_userMode);
    }

    void ConstructEventConfig(AMDTProfileSamplingConfig& sampleConfig, EventConfig& eventConfig)
    {
        eventConfig.eventSelect = static_cast<gtUInt16>(sampleConfig.m_hwEventId);
        eventConfig.eventUnitMask = sampleConfig.m_unitMask;
        eventConfig.bitOs = sampleConfig.m_osMode;
        eventConfig.bitUsr = sampleConfig.m_userMode;
    }

    bool GetEventConfigByEventId(AMDTUInt32 eventId, EventConfig& eventConfig)
    {
        bool ret = false;
        auto eventDesc = std::find_if(m_sampledCounterDescVec.begin(), m_sampledCounterDescVec.end(),
            [&eventId](AMDTProfileCounterDesc const& aEventDesc) { return aEventDesc.m_hwEventId == eventId; });

        if (eventDesc != m_sampledCounterDescVec.end())
        {
            AMDTProfileSamplingConfig sampleConfig;

            ret = GetSamplingConfiguration(eventDesc->m_id, sampleConfig);

            if (ret)
            {
                ConstructEventConfig(sampleConfig, eventConfig);
            }
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
                    EventMaskType encodedEvent;
                    ConstructEncodedEvent(sampleConfig, encodedEvent);
                    m_eventToIdMap.insert({ encodedEvent, counterDesc.m_id });
                }
            }
        }

        return ret;
    }

    bool PrepareReportAllDataView()
    {
        bool ret = false;

        if (!isSessionTypeCLU())
        {
            ret = PrepareReportAllDataView__();
        }
        else
        {
            ret = PrepareCLUDataView();
            ret = ret && UpdateSampledCountersForCLU();
        }

        return ret;
    }

    bool PrepareReportAllDataView__()
    {
        bool ret = GetSampledCountersList();

        if (ret)
        {
            ViewConfigInfo viewCfgInfo;
            viewCfgInfo.m_viewCfg.SetConfigName(CXL_REPORT_VIEW_ALL_DATA);

            gtUInt32 nbrEvents = m_sampledCounterDescVec.size();

            std::vector<EventConfig> eventConfigList;
            eventConfigList.reserve(nbrEvents);

            std::vector<std::string> eventTitleList;
            eventTitleList.reserve(nbrEvents);

            for (const auto& event : m_sampledCounterDescVec)
            {
                AMDTProfileSamplingConfig sampleConfig;
                ret = GetSamplingConfiguration(event.m_id, sampleConfig);

                if (ret)
                {
                    std::string title;
                    event.m_name.asUtf8(title);
                    eventTitleList.push_back(title);

                    // Initialize event config array
                    EventConfig eventConfig;
                    eventConfig.eventSelect = static_cast<gtUInt16>(event.m_hwEventId);
                    eventConfig.eventUnitMask = sampleConfig.m_unitMask;
                    eventConfig.bitUsr = sampleConfig.m_userMode;
                    eventConfig.bitOs = sampleConfig.m_osMode;
                    eventConfigList.push_back(eventConfig);

                    viewCfgInfo.m_counterIdVec.push_back(event.m_id);
                }
            }

            // Make column specifications
            viewCfgInfo.m_viewCfg.MakeColumnSpecs(eventConfigList, eventTitleList);
            viewCfgInfo.m_viewCfg.SetDescription("This special view has all of the data from the profile available.");

            m_viewConfigInfoVec.push_back(viewCfgInfo);
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

    bool GetCounterIdsFromColumnSpec(ColumnSpecList& columnArray, AMDTCounterIdVec& counterVec)
    {
        bool ret = true;

        AMDTProfileCounterDesc counterDesc;

        for (const auto& colSpec : columnArray)
        {
            if (colSpec.dataSelectLeft.eventSelect && colSpec.dataSelectRight.eventSelect)
            {
                // This is COMPUTED counter
                // check whether we have seen this already
                gtString computedCounterName;
                computedCounterName.fromUtf8String(colSpec.title);

                AMDTCounterId computedCounterId = 0;
                auto it = m_computedCounterNameIdMap.find(computedCounterName);

                if (it == m_computedCounterNameIdMap.end())
                {
                    // Construct dummy event descriptor
                    computedCounterId = m_computedCounterStartId++;

                    counterDesc.m_id = computedCounterId;
                    counterDesc.m_type = AMDT_PROFILE_COUNTER_TYPE_COMPUTED;
                    counterDesc.m_hwEventId = AMDT_PROFILE_ALL_COUNTERS;
                    counterDesc.m_deviceId = 0;
                    counterDesc.m_category = 0;

                    counterDesc.m_unit = (colSpec.type == ColumnRatio)
                                            ? AMDT_PROFILE_COUNTER_UNIT_RATIO : AMDT_PROFILE_COUNTER_UNIT_PERCENT;

                    counterDesc.m_name.fromUtf8String(colSpec.title);
                    counterDesc.m_abbrev = counterDesc.m_name;
                    counterDesc.m_description = counterDesc.m_name;

                    m_computedCounterDescVec.push_back(counterDesc);
                    m_computedCounterIdSpecMap.insert({ computedCounterId, colSpec });
                }
                else
                {
                    computedCounterId = it->second;
                }

                counterVec.push_back(computedCounterId);
            }
            else if (colSpec.dataSelectLeft.eventSelect)
            {
                // This is RAW counter
                AMDTCounterId counterId;

                ret = GetRawCounterId(colSpec.dataSelectLeft.eventSelect,
                                      colSpec.dataSelectLeft.eventUnitMask,
                                      colSpec.dataSelectLeft.bitOs,
                                      colSpec.dataSelectLeft.bitUsr,
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

        ColumnSpecList columnArray;
        columnArray.reserve(nbrCols);
        viewConfig.GetColumnSpecs(columnArray);

        // Now that we have got the columnspec from view config, check
        // whether all the events are in event vector
        for (auto& colSpec : columnArray)
        {
            AMDTCounterId counterId;

            if (colSpec.dataSelectLeft.eventSelect)
            {
                ret = GetRawCounterId(colSpec.dataSelectLeft.eventSelect,
                                      colSpec.dataSelectLeft.eventUnitMask,
                                      colSpec.dataSelectLeft.bitOs,
                                      colSpec.dataSelectLeft.bitUsr,
                                      counterId);
            }

            if (ret && colSpec.dataSelectRight.eventSelect)
            {
                ret = GetRawCounterId(colSpec.dataSelectRight.eventSelect,
                                      colSpec.dataSelectRight.eventUnitMask,
                                      colSpec.dataSelectRight.bitOs,
                                      colSpec.dataSelectRight.bitUsr,
                                      counterId);
            }
        }

        // This ViewConfig can be supported for this profile run
        if (ret)
        {
            ret = GetCounterIdsFromColumnSpec(columnArray, counterVec);
        }

        return ret;
    }

    bool AddViewConfig(ViewConfigInfo& viewCfgInfo)
    {
        bool ret = false;

        if ((ret = IsSamplingEventsAvailable(viewCfgInfo.m_viewCfg, viewCfgInfo.m_counterIdVec)) == true)
        {
            m_viewConfigInfoVec.push_back(viewCfgInfo);
        }

        return ret;
    }

    bool PrepareReportDataView(gtString viewConfigPath)
    {
        bool ret = GetSampledCountersList();

        if (ret)
        {
            ViewConfigInfo viewCfgInfo;

            std::string configFilePath;
            viewConfigPath.asUtf8(configFilePath);

            viewCfgInfo.m_viewCfg.ReadConfigFile(configFilePath);
            AddViewConfig(viewCfgInfo);
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

    bool PrepareComputedCounterForCLU(AMDTUInt16 eventId, ColumnSpec& columnSpec)
    {
        bool ret = false;

        columnSpec.type = ColumnRatio;
        columnSpec.sorting = NoSort;

        if (CXL_CLU_EVENT_CLU_PERCENTAGE == eventId)
        {
            GetEventConfigByEventId(CXL_CLU_EVENT_CLU_PERCENTAGE, columnSpec.dataSelectLeft);
            GetEventConfigByEventId(CXL_CLU_EVENT_L1_EVICTIONS, columnSpec.dataSelectRight);
            columnSpec.title = CXL_CLU_EVENT_CLU_PERCENTAGE_NAME_STR;
            columnSpec.type = ColumnPercentage;
            ret = true;
        }
        else if (CXL_CLU_EVENT_BYTES_PER_L1_EVICTION == eventId)
        {
            GetEventConfigByEventId(CXL_CLU_EVENT_BYTES_ACCESSED, columnSpec.dataSelectLeft);
            GetEventConfigByEventId(CXL_CLU_EVENT_L1_EVICTIONS, columnSpec.dataSelectRight);
            columnSpec.title = CXL_CLU_EVENT_BYTES_PER_L1_EVICTION_NAME_STR;
            ret = true;
        }
        else if (CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION == eventId)
        {
            GetEventConfigByEventId(CXL_CLU_EVENT_ACCESSES, columnSpec.dataSelectLeft);
            GetEventConfigByEventId(CXL_CLU_EVENT_L1_EVICTIONS, columnSpec.dataSelectRight);
            columnSpec.title = CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION_NAME_STR;
            ret = true;
        }

        return ret;
    }

    bool PrepareCLUDataView()
    {
        bool ret = GetSampledCountersList();

        if (ret)
        {
            ViewConfigInfo viewCfgInfo;
            viewCfgInfo.m_viewCfg.SetConfigName(CXL_REPORT_VIEW_ALL_DATA);

            gtUInt32 nbrEvents = m_sampledCounterDescVec.size();
            ColumnSpecList columnSpecList;
            columnSpecList.reserve(nbrEvents + 3);

            ColumnSpec cs;

            // First Add the derived counters?
            PrepareComputedCounterForCLU(CXL_CLU_EVENT_CLU_PERCENTAGE, cs);
            columnSpecList.push_back(cs);

            PrepareComputedCounterForCLU(CXL_CLU_EVENT_BYTES_PER_L1_EVICTION, cs);
            columnSpecList.push_back(cs);

            PrepareComputedCounterForCLU(CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION, cs);
            columnSpecList.push_back(cs);

            for (auto& event : m_sampledCounterDescVec)
            {
                if ((CXL_CLU_EVENT_CLU_PERCENTAGE == event.m_id)
                    || (CXL_CLU_EVENT_BYTES_PER_L1_EVICTION == event.m_id)
                    || (CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION == event.m_id))
                {
                    continue;
                }

                AMDTProfileSamplingConfig sampleConfig;
                ret = GetSamplingConfiguration(event.m_id, sampleConfig);

                if (ret)
                {
                    cs.type = ColumnValue;
                    cs.sorting = NoSort;
                    cs.title = std::string(event.m_name.asASCIICharArray());
                    cs.dataSelectRight.eventSelect = 0;
                    cs.dataSelectRight.eventUnitMask = 0;
                    cs.dataSelectRight.bitOs = 0;
                    cs.dataSelectRight.bitUsr = 0;

                    ConstructEventConfig(sampleConfig, cs.dataSelectLeft);

                    columnSpecList.push_back(cs);
                }
            }

            if (ret)
            {
                viewCfgInfo.m_viewCfg.SetColumnSpecs(columnSpecList, false);
                viewCfgInfo.m_viewCfg.SetDescription("This special view has all of the data from the profile available.");

                AddViewConfig(viewCfgInfo);
            }
        }

        return ret;
    }

    bool UpdateSampledCountersForCLU()
    {
        // Treat CXL_CLU_EVENT_CLU_PERCENTAGE as computed counter as this involves
        // computing the percentage based on evictions
        for (auto& eventDesc : m_sampledCounterDescVec)
        {
            AMDTProfileCounterDesc cluCounterDesc;

            if (eventDesc.m_id == CXL_CLU_EVENT_CLU_PERCENTAGE)
            {
                if (GetComputedCounterByName(CXL_CLU_EVENT_CLU_PERCENTAGE_NAME_WSTR, cluCounterDesc))
                {
                    eventDesc.m_id = cluCounterDesc.m_id;
                    eventDesc.m_type = cluCounterDesc.m_type;
                    eventDesc.m_unit = cluCounterDesc.m_unit;
                }
            }
            else if (eventDesc.m_id == CXL_CLU_EVENT_BYTES_PER_L1_EVICTION)
            {
                if (GetComputedCounterByName(CXL_CLU_EVENT_BYTES_PER_L1_EVICTION_NAME_WSTR, cluCounterDesc))
                {
                    eventDesc.m_id = cluCounterDesc.m_id;
                    eventDesc.m_type = cluCounterDesc.m_type;
                    eventDesc.m_unit = cluCounterDesc.m_unit;
                }
            }
            else if (eventDesc.m_id == CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION)
            {
                if (GetComputedCounterByName(CXL_CLU_EVENT_ACCESSES_PER_L1_EVICTION_NAME_WSTR, cluCounterDesc))
                {
                    eventDesc.m_id = cluCounterDesc.m_id;
                    eventDesc.m_type = cluCounterDesc.m_type;
                    eventDesc.m_unit = cluCounterDesc.m_unit;
                }
            }
        }

        return true;
    }

    bool GetComputedCounterByName(gtString counterName, AMDTProfileCounterDesc& desc)
    {
        bool ret = false;

        auto counterDesc = std::find_if(m_computedCounterDescVec.begin(), m_computedCounterDescVec.end(),
            [&counterName](AMDTProfileCounterDesc const& aEventDesc)
            { return 0 == aEventDesc.m_name.compareNoCase(counterName); });

        if (counterDesc != m_computedCounterDescVec.end())
        {
            desc = *counterDesc;
            ret = true;
        }

        return ret;
    }

    bool GetReportConfigurationByName(gtString configName, AMDTProfileReportConfig& reportConfig)
    {
        bool ret = false;

        if ((ret = InitializeReportConfigurations()) == true)
        {
            // construct the report configs
            for (auto& viewConfigInfo : m_viewConfigInfoVec)
            {
                std::string name;
                viewConfigInfo.m_viewCfg.GetConfigName(name);

                gtString cfgName;
                cfgName.fromUtf8String(name);

                if (cfgName.compareNoCase(configName) == 0)
                {
                    reportConfig.m_name.fromUtf8String(name);

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

        if ((ret = InitializeReportConfigurations()) == true)
        {
            // construct the report configs
            for (auto& viewConfigInfo : m_viewConfigInfoVec)
            {
                AMDTProfileReportConfig reportConfig;

                std::string name;
                viewConfigInfo.m_viewCfg.GetConfigName(name);
                reportConfig.m_name.fromUtf8String(name);

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

    bool SetDebugInfoPaths(gtString& symbolDirectory, gtString& symbolServer, gtString& cachePath)
    {
        bool ret = true;

        m_symbolServerPath = symbolServer;
        m_symbolFilePath = symbolDirectory;
        m_symbolDownloadPath = cachePath;

        m_isDebugSearchPathAvailable = (m_symbolServerPath.isEmpty() && m_symbolFilePath.isEmpty()) ? false : true;

        return ret;
    }

    bool SetSourcePaths(gtString& sourceDirPath)
    {
        bool ret = true;

        m_sourceFilePath = sourceDirPath;

        return ret;
    }

    bool SetBinaryPaths(gtString& binaryDirPath)
    {
        bool ret = true;

        m_binaryFilePath = binaryDirPath;

        return ret;
    }

    bool InitializeReportOption()
    {
        bool ret = true;

        m_options.m_coreMask = AMDT_PROFILE_ALL_CORES;
        m_options.m_isSeperateByCore = false;
        m_options.m_isSeperateByNuma = false;
        m_options.m_ignoreSystemModules = false;
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

        if (nullptr != pExecutable)
        {
            const wchar_t* pSearchPath = m_symbolFilePath.asCharArray();
            const wchar_t* pServerList = m_symbolServerPath.asCharArray();
            const wchar_t* pSymDownloadPath = m_symbolDownloadPath.asCharArray();

            retVal = pExecutable->InitializeSymbolEngine(pSearchPath, pServerList, pSymDownloadPath);
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

    bool GetFunctionInfoByModuleId(AMDTModuleId modId, AMDTProfileFunctionInfoVec& funcInfoVec, gtVAddr& modBaseAddr)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            ret = m_pDbAdapter->GetFunctionInfoByModuleId(modId, funcInfoVec);

            AMDTProfileModuleInfo modInfo;
            ret = ret && GetModuleInfo(modId, modInfo);

            modBaseAddr = (ret) ? modInfo.m_loadAddress : 0;

            // Add the unknown functions for this module
            for (auto& unknownFunc : m_unknownFuncIdInfoMap)
            {
                if (unknownFunc.second.m_moduleId == modId)
                {
                    funcInfoVec.push_back(unknownFunc.second);
                }
            }
        }

        return ret;
    }

    bool AddOthersEntry(AMDTProfileDataType type, AMDTProfileDataVec& summaryDataVec, AMDTCounterId counterId)
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
                otherEntry.m_type = type; /*summaryDataVec[0].m_type*/

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
                                               m_options.m_ignoreSystemModules,
                                               m_options.m_isSeperateByCore,
                                               false,  // separateByProcess
                                               m_options.m_doSort,
                                               m_options.m_summaryCount,
                                               summaryDataVec);

            ret = AddOthersEntry(type, summaryDataVec, counterId);

            // calculate the percentage
            ret = CalculateRawCounterPercentage(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, counterIdList, summaryDataVec);
        }

        return ret;
    }

    bool GetAllSampledCountersIdList(AMDTCounterIdVec& counterIdList)
    {
        AMDTProfileCounterDescVec sampledCounterDescVec;
        bool ret = GetSampledCountersList(sampledCounterDescVec);

        if (ret)
        {
            for (auto& aCounterDesc : sampledCounterDescVec)
            {
                counterIdList.push_back(aCounterDesc.m_id);
            }
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
                AMDTSampleValueVec totalSamplesValueVec;

                if (procTotalIt == m_processSampleTotalMap.end())
                {
                    AMDTCounterIdVec allCounterIdList;

                    ret = GetAllSampledCountersIdList(allCounterIdList);

                    ret = m_pDbAdapter->GetCounterTotals(type,
                                                         procId,
                                                         threadId,
                                                         moduleId,
                                                         funcId,
                                                         allCounterIdList,
                                                         coreMask,
                                                         separateByCore,
                                                         totalSamplesValueVec);

                    if (ret)
                    {
                        m_processSampleTotalMap.insert({ procId, totalSamplesValueVec });
                    }
                }
                else
                {
                    totalSamplesValueVec = procTotalIt->second;
                    ret = true;
                }

                if (ret)
                {
                    for (auto &id : counterIdList)
                    {
                        auto counterTotal = std::find_if(totalSamplesValueVec.begin(), totalSamplesValueVec.end(),
                            [&id](AMDTSampleValue const& aData) { return aData.m_counterId == id; });

                        if (counterTotal != totalSamplesValueVec.end())
                        {
                            totalValueVec.push_back(*counterTotal);
                        }
                    }
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
                        CXL_COMPUTE_PERCENTAGE(aSummaryData.m_sampleValue[idx].m_sampleCount, totalValueVec[idx].m_sampleCount);
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
                        CXL_COMPUTE_PERCENTAGE(srcLineData.m_sampleValues[idx].m_sampleCount, totalValueVec[idx].m_sampleCount);
                }
            }

            for (auto& instLineData : functionData.m_instDataList)
            {
                for (gtUInt32 idx = 0; idx < instLineData.m_sampleValues.size(); idx++)
                {
                    instLineData.m_sampleValues[idx].m_sampleCountPercentage =
                        CXL_COMPUTE_PERCENTAGE(instLineData.m_sampleValues[idx].m_sampleCount, totalValueVec[idx].m_sampleCount);
                }
            }
        }

        return ret;
    }

    bool RemoveUnnecessaryCounters(AMDTUInt32 counterId, AMDTProfileDataVec& profileData)
    {
        bool ret = false;

        for (auto& aData : profileData)
        {
            AMDTSampleValueVec newSampleValueVec;

            auto counterValue = std::find_if(aData.m_sampleValue.begin(), aData.m_sampleValue.end(),
                [&counterId](AMDTSampleValue const& aSample) { return aSample.m_counterId == counterId; });

            if (counterValue != aData.m_sampleValue.end())
            {
                newSampleValueVec.push_back(*counterValue);
                ret = true;
            }

            aData.m_sampleValue.clear();
            aData.m_sampleValue = newSampleValueVec;
        }

        return ret;
    }

    bool GetSampleCount(bool sepByCore, AMDTSampleValueVec& totalValueVec)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            AMDTCounterIdVec countersList;

            ret = GetAllSampledCountersIdList(countersList);

            ret = ret && m_pDbAdapter->GetCounterTotals(AMDT_PROFILE_DATA_PROCESS,
                                                        AMDT_PROFILE_ALL_PROCESSES,
                                                        AMDT_PROFILE_ALL_THREADS,
                                                        AMDT_PROFILE_ALL_MODULES,
                                                        0,
                                                        countersList,
                                                        AMDT_PROFILE_ALL_CORES,
                                                        sepByCore,
                                                        totalValueVec);
        }

        return ret;
    }

    bool GetProcessSummary(AMDTUInt32 counterId, AMDTProfileDataVec& processSummaryData)
    {
        bool ret = false;
        AMDTProfileCounterDesc desc;

        if ((ret = GetCounterDescById(counterId, desc)) == true)
        {
            if (AMDT_PROFILE_COUNTER_TYPE_RAW == desc.m_type)
            {
                ret = GetSummaryData(AMDT_PROFILE_DATA_PROCESS, counterId, processSummaryData);
            }
            else if (AMDT_PROFILE_COUNTER_TYPE_COMPUTED == desc.m_type)
            {
                ret = GetProcessProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, processSummaryData);

                ret = ret && RemoveUnnecessaryCounters(counterId, processSummaryData);

                // TODO: Return only the top five entriess
                if (processSummaryData.size() > 5)
                {
                    processSummaryData.erase(processSummaryData.begin() + 5, processSummaryData.end());
                }
            }
        }

        return ret;
    }

    bool GetThreadSummary(AMDTUInt32 counterId, AMDTProfileDataVec& threadSummaryData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_THREAD, counterId, threadSummaryData);
    }

    bool GetModuleSummary(AMDTUInt32 counterId, AMDTProfileDataVec& moduleSummaryData)
    {
        bool ret = false;
        AMDTProfileCounterDesc desc;

        if ((ret = GetCounterDescById(counterId, desc)) == true)
        {
            if (AMDT_PROFILE_COUNTER_TYPE_RAW == desc.m_type)
            {
                ret = GetSummaryData(AMDT_PROFILE_DATA_MODULE, counterId, moduleSummaryData);

            }
            else if (AMDT_PROFILE_COUNTER_TYPE_COMPUTED == desc.m_type)
            {
                ret = GetModuleProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, moduleSummaryData);
                ret = ret && RemoveUnnecessaryCounters(counterId, moduleSummaryData);

                // TODO: Return only the top five entriess
                if (moduleSummaryData.size() > 5)
                {
                    moduleSummaryData.erase(moduleSummaryData.begin() + 5, moduleSummaryData.end());
                }

            }
        }

        return ret;
    }

    bool GetFunctionSummary(AMDTUInt32 counterId, AMDTProfileDataVec& funcSummaryData)
    {
        bool ret = false;
        AMDTProfileCounterDesc desc;

        if ((ret = GetCounterDescById(counterId, desc)) == true)
        {
            if (AMDT_PROFILE_COUNTER_TYPE_RAW == desc.m_type)
            {
                ret = GetSummaryData(AMDT_PROFILE_DATA_FUNCTION, counterId, funcSummaryData);
            }
            else if (AMDT_PROFILE_COUNTER_TYPE_COMPUTED == desc.m_type)
            {
                ret = GetFunctionProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, funcSummaryData);
                ret = ret && RemoveUnnecessaryCounters(counterId, funcSummaryData);

                // TODO: Return only the top five entriess
                if (funcSummaryData.size() > 5)
                {
                    funcSummaryData.erase(funcSummaryData.begin() + 5, funcSummaryData.end());
                }
            }

            ret = HandleUnknownFunctions(funcSummaryData);
        }

        return ret;
    }

    bool GetUnknownFunctionsInfoFromIPSamples()
    {
        bool ret = false;

        if (!m_readUnknownFuncs && nullptr != m_pDbAdapter)
        {
            // Commit and create new transaction
            m_pDbAdapter->FlushDb();

            AMDTProfileFunctionInfoVec unknownFuncVec;
            ret = m_pDbAdapter->GetUnknownFunctionsByIPSamples(unknownFuncVec);

            for (auto& func : unknownFuncVec)
            {
                // Find the function info from debug info (if available) and update the tables
                ret = Lookupfunction(func, true, true);
            }

            m_readUnknownFuncs = ret;

            // Commit the updates
            m_pDbAdapter->FlushDb();
        }

        return ret;
    }

    bool AddUnknownFunctionsInfo(AMDTProfileFunctionInfoVec& funcInfoVec)
    {
        // construct the required maps
        for (auto& func : funcInfoVec)
        {
            AddUnknownFunctionInfo(func);
        }

        return true;
    }

    void AddUnknownFunctionInfo(AMDTProfileFunctionInfo& func)
    {
        if (IS_UNKNOWN_FUNC(func.m_functionId))
        {
            // Get the offset and moduleid
            gtUInt32 funcOffset = static_cast<gtUInt32>(func.m_startOffset);
            AMDTModuleId moduleId = func.m_moduleId;
            gtUInt64 unknownFuncModOffset;
            GET_MODOFFSET_FOR_UNKNOWN_FUNC(moduleId, funcOffset, unknownFuncModOffset);

            auto it = m_modOffsetFuncInfoMap.find(unknownFuncModOffset);

            if (it == m_modOffsetFuncInfoMap.end())
            {
                // Set the new function id
                func.m_functionId |= m_currentUnknownFuncId++;

                m_unknownFuncIdInfoMap.insert({ func.m_functionId , func });
                m_modOffsetFuncInfoMap.insert({ unknownFuncModOffset , func });
            }
            else
            {
                func.m_functionId = it->second.m_functionId;
                func.m_name = it->second.m_name;
                func.m_size = it->second.m_size;
            }
        }
    }

    // The unknown function ids will be generated here and they are not update in DB
    bool HandleUnknownFunctions(AMDTProfileDataVec& funcProfileData)
    {
        for (auto& funcData : funcProfileData)
        {
            if ((AMDT_PROFILE_DATA_FUNCTION == funcData.m_type) && (IS_UNKNOWN_FUNC(funcData.m_id)))
            {
                gtUInt32 funcOffset = funcData.m_moduleId;
                AMDTModuleId moduleId = (funcData.m_id & 0xffff0000) >> 16;
                gtUInt64 unknownFuncModOffset;
                GET_MODOFFSET_FOR_UNKNOWN_FUNC(moduleId, funcOffset, unknownFuncModOffset);

                auto it = m_modOffsetFuncInfoMap.find(unknownFuncModOffset);

                // update function info.
                if (it != m_modOffsetFuncInfoMap.end())
                {
                    funcData.m_id = it->second.m_functionId;
                    funcData.m_moduleId = moduleId;
                    funcData.m_name = it->second.m_name;
                }
            }
        }

        return true;
    }

    // returns true if the unknown was already seen
    bool HandleUnknownFunction(AMDTProfileFunctionInfo& func)
    {
        bool ret = false;

        if (IS_UNKNOWN_FUNC(func.m_functionId))
        {
            gtUInt32 funcOffset = static_cast<gtUInt32>(func.m_startOffset);
            AMDTModuleId moduleId = func.m_moduleId;
            gtUInt64 unknownFuncModOffset;
            GET_MODOFFSET_FOR_UNKNOWN_FUNC(moduleId, funcOffset, unknownFuncModOffset);

            auto it = m_modOffsetFuncInfoMap.find(unknownFuncModOffset);

            if (it != m_modOffsetFuncInfoMap.end())
            {
                func.m_functionId = it->second.m_functionId;
                func.m_name = it->second.m_name;
                func.m_size = it->second.m_size;

                ret = true;
            }
        }

        return ret;
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
                    computedCounterValue = (lhsCount / (static_cast<float>(64.0) * rhsCount)) * static_cast<float>(100.0);
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
                    if ((ret = GetRawCounterValueFromSampleValueVec(profileDataVec, counterId, sampleValue, samplePerc)) == true)
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

                        if ((ret = GetDataForColumnSpec(columnSpec, profileDataVec, sampleValue)) == true)
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
                                                      m_options.m_ignoreSystemModules,
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

    bool GetProcessProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& processProfileData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_PROCESS, procId, modId, processProfileData);
    }

    bool GetModuleProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& moduleProfileData)
    {
        return GetSummaryData(AMDT_PROFILE_DATA_MODULE, procId, modId, moduleProfileData);
    }

    bool GetFunctionProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& funcProfileData)
    {
        bool ret = GetSummaryData(AMDT_PROFILE_DATA_FUNCTION, procId, modId, funcProfileData);

        ret = ret && HandleUnknownFunctions(funcProfileData);

        return ret;
    }

    bool IsUnknownFunctionId(AMDTFunctionId funcId, AMDTFunctionId& origFuncId, gtUInt32& offset, gtString& name)
    {
        bool ret = false;

        auto it = m_unknownFuncIdInfoMap.find(funcId);

        if (it != m_unknownFuncIdInfoMap.end())
        {
            origFuncId = funcId & 0xffff0000;
            offset = static_cast<gtUInt32>(it->second.m_startOffset);
            name = it->second.m_name;
            ret = true;
        }

        return ret;
    }

    bool IsUnknownFunctionId(AMDTFunctionId funcId, AMDTFunctionId& origFuncId, AMDTProfileFunctionInfo& funcInfo)
    {
        bool ret = false;

        auto it = m_unknownFuncIdInfoMap.find(funcId);

        if (it != m_unknownFuncIdInfoMap.end())
        {
            origFuncId = funcId & 0xffff0000;
            funcInfo = it->second;
            ret = true;
        }

        return ret;
    }

    bool AddFuncInfoToFuncIdInfoMap(AMDTFunctionId funcId, AMDTProfileFunctionInfo& functionInfo)
    {
        auto it = m_funcIdInfoMap.find(funcId);

        if (it == m_funcIdInfoMap.end())
        {
            m_funcIdInfoMap.insert({ funcId, functionInfo });
        }

        return true;
    }

    bool GetFunctionData(AMDTFunctionId            funcId,
                         AMDTProcessId             processId,
                         AMDTThreadId              threadId,
                         AMDTProfileFunctionData&  functionData)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            AMDTCounterIdVec countersList;
            ret = GetCountersList(countersList);

            AMDTFunctionId dbFuncId = funcId;
            gtUInt32 funcOffset = 0;
            AMDTProfileFunctionInfo unknownFuncInfo;
            bool isUnkownFunc = IsUnknownFunctionId(funcId, dbFuncId, unknownFuncInfo);

            if (isUnkownFunc)
            {
                funcOffset = static_cast<gtUInt32>(unknownFuncInfo.m_startOffset);
            }

            ret = ret && m_pDbAdapter->GetFunctionProfileData(dbFuncId,
                                                              funcOffset,
                                                              processId,
                                                              threadId,
                                                              countersList,
                                                              m_options.m_coreMask,
                                                              m_options.m_isSeperateByCore,
                                                              functionData);

            if (ret)
            {
                if (isUnkownFunc)
                {
                    functionData.m_functionInfo.m_functionId = unknownFuncInfo.m_functionId;
                    functionData.m_functionInfo.m_name = unknownFuncInfo.m_name;
                    functionData.m_functionInfo.m_size = unknownFuncInfo.m_size;
                }

                // if function size is zero, compute the size from instruction data.
                gtUInt32 functionSize = functionData.m_functionInfo.m_size;
                gtUInt32 startOffset = static_cast<gtUInt32>(functionData.m_functionInfo.m_startOffset);
                gtUInt32 nbrInsts = functionData.m_instDataList.size();

                if (functionSize == 0 && nbrInsts)
                {
                    gtUInt32 instStartOffset = functionData.m_instDataList[0].m_offset;
                    startOffset = (instStartOffset < startOffset) ? instStartOffset : startOffset;

                    gtUInt32 tmpSize = functionData.m_instDataList[nbrInsts - 1].m_offset - instStartOffset;
                    // FIXME: setting 16 as funcsize for functions with size 0
                    functionSize = (tmpSize > 0) ? tmpSize : 16;
                    // FIXME: Should i update the functionInfo with this dummy size?
                    functionData.m_functionInfo.m_size = functionSize;

                    functionData.m_functionInfo.m_size = functionSize;
                    functionData.m_functionInfo.m_startOffset = startOffset;
                }

                AddFuncInfoToFuncIdInfoMap(funcId, functionData.m_functionInfo);
            }
        }

        return ret;
    }

    bool GetFunctionInfo(AMDTFunctionId            functionId,
                         AMDTProfileFunctionInfo&  functionInfo,
                         gtUInt64*                 pModLoadAddress,
                         gtVector<AMDTProcessId>*  pProcessList,
                         gtVector<AMDTThreadId>*   pThreadList)
    {
        bool ret = false;
        gtUInt32 funcStartOffset = 0;

        if (nullptr != m_pDbAdapter)
        {
            if (IS_UNKNOWN_FUNC(functionId))
            {
                functionInfo.m_functionId = functionId;
                functionInfo.m_moduleId = CXL_GET_DB_MODULE_ID(functionId);

                HandleUnknownFunction(functionInfo);
                funcStartOffset = static_cast<gtUInt32>(functionInfo.m_startOffset);
            }

            ret = m_pDbAdapter->GetFunctionInfo(functionId, funcStartOffset, functionInfo);

            if (ret)
            {
                AddFuncInfoToFuncIdInfoMap(functionId, functionInfo);
            }

            if (nullptr != pModLoadAddress)
            {
                AMDTProfileModuleInfo modInfo;
                ret = GetModuleInfo(functionInfo.m_moduleId, modInfo);
                *pModLoadAddress = modInfo.m_loadAddress;
            }

            if (nullptr != pProcessList && nullptr != pThreadList)
            {
                ret = m_pDbAdapter->GetProcessAndThreadListForFunction(functionId, funcStartOffset, *pProcessList, *pThreadList);
            }
        }

        return ret;
    }

    int GetFunctionDetailedProfileData(AMDTFunctionId            funcId,
                                       AMDTProcessId             processId,
                                       AMDTThreadId              threadId,
                                       AMDTProfileFunctionData&  functionData)
    {
        int retVal = 0;
        bool ret = false;

        ret = GetFunctionData(funcId, processId, threadId, functionData);

        if (ret)
        {
            gtString srcFilePath;
            AMDTSourceAndDisasmInfoVec srcInfoVec;
            AMDTProfileFunctionData* pCurrFunctionData = &functionData;
            retVal = GetFunctionSourceAndDisasmInfo__(funcId, srcFilePath, srcInfoVec, pCurrFunctionData);

            AMDTProfileSourceLineDataVec& srcLineDataVec = functionData.m_srcLineDataList;
            AMDTProfileInstructionDataVec& instDataVec = functionData.m_instDataList;

            for (auto& instData : instDataVec)
            {
                gtUInt32 srcLine = 0;

                if (CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE != static_cast<unsigned int>(retVal))
                {
                    // check whether this offset has samples (in functionData.m_instDataList)
                    auto srcData = std::find_if(srcInfoVec.begin(), srcInfoVec.end(),
                        [&instData](AMDTSourceAndDisasmInfo const& sData)
                    { return ((instData.m_offset >= sData.m_offset) && (instData.m_offset < (sData.m_offset + sData.m_size))); });

                    if (srcData != srcInfoVec.end())
                    {
                        srcLine = (*srcData).m_sourceLine;
                    }
                }

                // Check wether we have see this srcline in srcLineDataVec
                // if so, update the profile data otherwise add a new line
                auto slData = std::find_if(srcLineDataVec.begin(), srcLineDataVec.end(),
                    [&srcLine](AMDTProfileSourceLineData const& sData) { return sData.m_sourceLineNumber == srcLine; });

                if (slData == srcLineDataVec.end())
                {
                    AMDTProfileSourceLineData srcLineData;
                    srcLineData.m_sourceLineNumber = srcLine;
                    srcLineData.m_sampleValues = instData.m_sampleValues;

                    srcLineDataVec.push_back(srcLineData);
                }
                else
                {
                    for (gtUInt32 i = 0; i < instData.m_sampleValues.size(); i++)
                    {
                        slData->m_sampleValues[i].m_sampleCount += instData.m_sampleValues[i].m_sampleCount;
                        slData->m_sampleValues[i].m_sampleCountPercentage += instData.m_sampleValues[i].m_sampleCountPercentage;
                    }
                }
            }

            AMDTCounterIdVec countersList;
            ret = GetCountersList(countersList);

            // Now calculate the percentage
            ret = ret && CalculateRawCounterPercentage(funcId, processId, threadId, countersList, functionData);

            // Now calculate the computed counter values
            // Iterate over the profileDataVec and calculate the computed counter values
            ret = ret && CalculateComputedCounters(functionData.m_srcLineDataList);

            ret = ret && CalculateComputedCounters(functionData.m_instDataList);
        }

        return retVal;
    }

    bool GetSrcFilePathForVaddr(ExecutableFile& exeFile, gtRVAddr rvAddr, gtString& srcFilePath, gtUInt32& srcLine)
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

    bool GetSrcFilePathForModuleOffset(AMDTModuleId moduleId, gtUInt32 offset, gtString& srcFilePath, gtUInt32& srcLine)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            ExecutableFile* pExecutable = nullptr;
            ret = GetModuleExecutable(moduleId, pExecutable, true);

            if (ret && (nullptr != pExecutable))
            {
                AMDTProfileModuleInfo modInfo;
                ret = GetModuleInfo(moduleId, modInfo);
                gtVAddr loadAddress = modInfo.m_loadAddress;

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
            }
        }

        return ret;
    }

    bool IsJitFunction(AMDTFunctionId funcId)
    {
        bool ret = false;

        auto funcInfoIt = m_funcIdInfoMap.find(funcId);

        if (funcInfoIt != m_funcIdInfoMap.end())
        {
            // profile data has been fetched for this function..
            AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;
            AMDTProfileModuleInfo modInfo;

            if (GetModuleInfo(funcInfo.m_moduleId, modInfo))
            {
                ret = ((AMDT_MODULE_TYPE_JAVA == modInfo.m_type) || (AMDT_MODULE_TYPE_MANAGEDDPE == modInfo.m_type))
                            ? true : false;
            }
        }

        return ret;
    }

    bool IsJavaJitFunction(AMDTFunctionId funcId)
    {
        bool ret = false;

        auto funcInfoIt = m_funcIdInfoMap.find(funcId);

        if (funcInfoIt != m_funcIdInfoMap.end())
        {
            // profile data has been fetched for this function..
            AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;
            AMDTProfileModuleInfo modInfo;

            if (GetModuleInfo(funcInfo.m_moduleId, modInfo))
            {
                ret = (AMDT_MODULE_TYPE_JAVA == modInfo.m_type) ? true : false;
            }
        }

        return ret;
    }

    bool IsClrJitFunction(AMDTFunctionId funcId)
    {
        bool ret = false;

        auto funcInfoIt = m_funcIdInfoMap.find(funcId);

        if (funcInfoIt != m_funcIdInfoMap.end())
        {
            // profile data has been fetched for this function..
            AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;
            AMDTProfileModuleInfo modInfo;

            if (GetModuleInfo(funcInfo.m_moduleId, modInfo))
            {
                ret = (AMDT_MODULE_TYPE_MANAGEDDPE == modInfo.m_type) ? true : false;
            }
        }

        return ret;
    }

    bool GetSrcFilePathForFuncId(AMDTFunctionId funcId, gtString& srcFilePath, gtUInt32& srcLine)
    {
        bool ret = false;
        AMDTModuleId moduleId = GET_MODULEID_FROM_FUNCTIONID(funcId);
        gtUInt32 offset = 0;

        auto funcInfoIt = m_funcIdInfoMap.find(funcId);

        if (funcInfoIt != m_funcIdInfoMap.end())
        {
            // profile data has been fetched for this function..
            AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;
            AMDTProfileModuleInfo modInfo;

            if (GetModuleInfo(funcInfo.m_moduleId, modInfo))
            {
                if (AMDT_MODULE_TYPE_NATIVE == modInfo.m_type)
                {
                    moduleId = funcInfo.m_moduleId;
                    offset = static_cast<gtUInt32>(funcInfo.m_startOffset);

                    ret = GetSrcFilePathForModuleOffset(moduleId, offset, srcFilePath, srcLine);
                }
                else if (AMDT_MODULE_TYPE_JAVA == modInfo.m_type)
                {
                    gtString jncPath;
                    gtVAddr jitLoadAddr;

                    ret = m_pDbAdapter->GetJITFunctionInfo(funcId, jitLoadAddr, srcFilePath, jncPath);
                }
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                else if (AMDT_MODULE_TYPE_MANAGEDDPE == modInfo.m_type)
                {
                    ret = GetClrJitSrcFilePath(funcInfo.m_functionId, srcFilePath);
                }
#endif // AMDT_WINDOWS_OS
            }
        }

        return ret;
    }

    bool GetSrcFilePathForFuncId(AMDTFunctionId funcId, gtString& srcFilePath)
    {
        gtUInt32 srcLine;
        return GetSrcFilePathForFuncId(funcId, srcFilePath, srcLine);
    }

    int GetFunctionSourceAndDisasmInfo(AMDTFunctionId funcId,
        gtString& srcFilePath,
        AMDTSourceAndDisasmInfoVec& srcInfoVec)
    {
        AMDTProfileFunctionData* pFuncData = nullptr;

        return GetFunctionSourceAndDisasmInfo__(funcId, srcFilePath, srcInfoVec, pFuncData);
    }

    int GetFunctionSourceAndDisasmInfo__(AMDTFunctionId funcId,
                                         gtString& srcFilePath,
                                         AMDTSourceAndDisasmInfoVec& srcInfoVec,
                                         AMDTProfileFunctionData* pFuncData)
    {
        int rv = CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE;
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
            rv = GetDisassembly(funcId, srcInfoVec, pFuncData);

            if (rv)
            {
                m_funcIdSrcInfoMap.insert({ funcId, srcInfoVec });
            }

            if (!foundSrcFilePath)
            {
                // Find the srcFilePath
                foundSrcFilePath = GetSrcFilePathForFuncId(funcId, srcFilePath);
            }
        }

        return rv;
    }

    int GetDisassembly(AMDTFunctionId funcId, AMDTSourceAndDisasmInfoVec& disasmInfoVec, AMDTProfileFunctionData* pFuncData)
    {
        int retVal = CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE;

        // TODO: Currently there is a limitation that GetFunctionDetailedProfileData()
        // should be called before GetFunctionSourceAndDisasmInfo()
        auto funcInfoIt = m_funcIdInfoMap.find(funcId);

        if (funcInfoIt != m_funcIdInfoMap.end())
        {
            // profile data has been fetched for this function..
            AMDTProfileFunctionInfo& funcInfo = funcInfoIt->second;
            AMDTProfileModuleInfo modInfo;

            if (GetModuleInfo(funcInfo.m_moduleId, modInfo))
            {
                if (AMDT_MODULE_TYPE_NATIVE == modInfo.m_type)
                {
                    retVal = GetNativeDisassembly(funcInfo.m_moduleId,
                                                  static_cast<gtUInt32>(funcInfo.m_startOffset),
                                                  funcInfo.m_size,
                                                  pFuncData,
                                                  disasmInfoVec);

                }
                else if (AMDT_MODULE_TYPE_JAVA == modInfo.m_type)
                {
                    retVal = GetJavaJitDisassembly(funcInfo, disasmInfoVec);
                }
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                else if (AMDT_MODULE_TYPE_MANAGEDDPE == modInfo.m_type)
                {
                    retVal = GetClrJitDisassembly(funcInfo, disasmInfoVec);
                }
#endif // AMDT_WINDOWS_OS
            }
        }

        return retVal;
    }

#if 0
    int GetDisassembly(AMDTModuleId moduleId,
                       AMDTUInt32 offset,
                       AMDTUInt32 size,
                       AMDTSourceAndDisasmInfoVec& disasmInfoVec)
    {
        int retVal = CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE;
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

                ret = InitializeSymbolEngine(pExecutable);
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
                        ret = pSymbolEngine->FindSourceLine(startRVAddr, srcData);

                        if (ret)
                        {
                            disasmInfo.m_sourceLine = srcData.m_line;
                            retVal = CXL_DATAACCESS_SUCCESS;
                        }
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
            else
            {
                retVal = CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE;
            }
        }

        return retVal;
    }
#endif // 0

    int GetNativeDisassembly(AMDTModuleId moduleId,
                             AMDTUInt32 offset,
                             AMDTUInt32 size,
                             AMDTProfileFunctionData* pFuncData,
                             AMDTSourceAndDisasmInfoVec& disasmInfoVec)
    {
        int retVal = CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE;
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            ExecutableFile* pExecutable = nullptr;
            ret = GetModuleExecutable(moduleId, pExecutable, true);

            if (ret && (nullptr != pExecutable))
            {
                AMDTProfileModuleInfo modInfo;
                ret = GetModuleInfo(moduleId, modInfo);
                gtVAddr loadAddress = modInfo.m_loadAddress;

                gtRVAddr startRVAddr = 0;
                const gtUByte* pCode = nullptr;
                gtRVAddr sectionStartRva = 0, sectionEndRva = 0;
                unsigned int prevSectionIndex = static_cast<unsigned int>(-1);

                bool isLongMode = pExecutable->Is64Bit();
                AMDTUInt32 bytesToRead = size;
                gtVAddr currOffset = offset;

                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();
                gtUInt32 nbrInst = 0;
                startRVAddr = pExecutable->VaToRva(loadAddress + currOffset);
                bool hasSrcLineInfo = false;

                if (nullptr != pSymbolEngine)
                {
                    SourceLineInfo sInfo;
                    hasSrcLineInfo = pSymbolEngine->FindSourceLine(startRVAddr, sInfo);

                    if (!hasSrcLineInfo && size > CXL_MAX_UNKNOWN_FUNC_SIZE)
                    {
                        if (nullptr != pFuncData && pFuncData->m_instDataList.size() > 0)
                        {
                            currOffset = pFuncData->m_instDataList.front().m_offset;
                            startRVAddr = pExecutable->VaToRva(loadAddress + currOffset);

                            bytesToRead = pFuncData->m_instDataList.back().m_offset - pFuncData->m_instDataList.front().m_offset;
                            bytesToRead += 16; // in case if there is only one inst
                        }
                    }
                }

                while (bytesToRead > 0)
                {
                    AMDTSourceAndDisasmInfo disasmInfo;

                    startRVAddr = pExecutable->VaToRva(loadAddress + currOffset);
                    unsigned int sectionIndex = pExecutable->LookupSectionIndex(startRVAddr);

                    gtRVAddr codeOffset = startRVAddr;
                    const gtUByte* pCurrentCode = pCode;
                    SourceLineInfo srcData;

                    if (hasSrcLineInfo)
                    {
                        ret = pSymbolEngine->FindSourceLine(startRVAddr, srcData);

                        if (ret)
                        {
                            disasmInfo.m_sourceLine = static_cast<gtUInt16>(srcData.m_line);
                            retVal = CXL_DATAACCESS_SUCCESS;
                        }
                    }

                    AMDTUInt32 NumBytesUsed = 0;

                    if (pExecutable->GetSectionsCount() > sectionIndex)
                    {
                        if (sectionIndex == prevSectionIndex)
                        {
                            codeOffset = startRVAddr - sectionStartRva;
                            pCurrentCode = pCode + codeOffset;
                        }
                        else
                        {
                            pCode = pExecutable->GetSectionBytes(sectionIndex);
                            pExecutable->GetSectionRvaLimits(sectionIndex, sectionStartRva, sectionEndRva);

                            // GetCodeBytes return the pointer to the sectionStart
                            // We need to add the offset to the beginning of the function
                            codeOffset = startRVAddr - sectionStartRva;
                            pCurrentCode = pCode + codeOffset;
                            prevSectionIndex = sectionIndex;
                        }

                        disasmInfo.m_offset = codeOffset + sectionStartRva;

                        // Get disassembly for the current pCode from the disassembler
                        ret = GetDisassemblyString((BYTE*)(pCurrentCode), isLongMode, disasmInfo, NumBytesUsed);
                    }

                    if (ret)
                    {
                        currOffset += NumBytesUsed;
                        bytesToRead = (bytesToRead > NumBytesUsed) ? (bytesToRead - NumBytesUsed) : 0;
                    }
                    else
                    {
                        // FIXME
                        NumBytesUsed = 1;
                        currOffset += NumBytesUsed;
                        bytesToRead = (bytesToRead > NumBytesUsed) ? (bytesToRead - NumBytesUsed) : 0;
                    }

                    disasmInfoVec.push_back(disasmInfo);

                    if ((CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE == static_cast<unsigned int>(retVal)) && ((++nbrInst) > CXL_MAX_DISASM_INSTS))
                    {
                        bytesToRead = 0;
                    }
                }
            }
        }
        else
        {
            retVal = CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE;
        }

        return retVal;
    }

    int GetJavaJitDisassembly(AMDTProfileFunctionInfo& funcInfo, AMDTSourceAndDisasmInfoVec& srcInfoVec)
    {
        int retVal = CXL_DATAACCESS_SUCCESS;

        if (nullptr != m_pDbAdapter)
        {
            // Get the loadaddress, srcfilepath, jncfilepath for the given function id
            gtString jncPath;
            gtString srcFilePath;
            gtVAddr jitLoadAddr;

            if (m_pDbAdapter->GetJITFunctionInfo(funcInfo.m_functionId, jitLoadAddr, srcFilePath, jncPath))
            {
                JavaJncReader jncReader;
                AMDTProfileSessionInfo sessionInfo;

                GetProfileSessionInfo(sessionInfo);
                gtString jncFilePath = sessionInfo.m_sessionDir;
                jncFilePath.append(osFilePath::osPathSeparator);
                jncFilePath.append(jncPath);

                if (jncReader.Open(jncFilePath.asCharArray()))
                {
                    unsigned int sectionSize = 0;

                    const gtUByte* pCode = jncReader.GetCodeBytesOfTextSection(&sectionSize);

                    if (pCode != nullptr)
                    {
                        int version = 0;
                        bool isLongMode = true;
                        bool isNativeMethod = (srcFilePath.compareNoCase(L"Unknown Source File") == 0) ? true : false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                        isLongMode = jncReader.Is64Bit();
                        version = jncReader.GetJNCVersion();
#endif

                        OffsetLinenumMap funcOffsetLinenumMap;

                        if (version <= 0x3 || (isNativeMethod))
                        {
                            // This is for JNC version <= 3
                            funcOffsetLinenumMap = jncReader.GetOffsetLines();
                            retVal = CXL_DATAACCESS_WARN_SRC_INFO_NOTAVAILABLE;
                        }
                        else
                        {
                            gtString funcName = funcInfo.m_name;
                            std::wstring javaFuncName;

                            int pos = funcName.reverseFind(L"::");

                            if (pos != -1)
                            {
                                gtString retStr;
                                funcName.getSubString(pos + 2, funcName.length() - 1, retStr);
                                javaFuncName = retStr.asCharArray();
                            }
                            else
                            {
                                javaFuncName = funcName.asCharArray();
                            }

                            funcOffsetLinenumMap = jncReader.GetOffsetLines(javaFuncName);
                        }

                        AMDTUInt32 bytesToRead = 0;
                        AMDTUInt32 bytesUsed = 0;
                        AMDTUInt32 currOffset = 0;
                        AMDTUInt32 currLineNumber = 0;

                        if (! funcOffsetLinenumMap.empty())
                        {
                            currLineNumber = funcOffsetLinenumMap.begin()->second;

                            for (auto& offsetLineIt : funcOffsetLinenumMap)
                            {
                                AMDTUInt32 nextOffset = offsetLineIt.first;
                                AMDTUInt32 nextLineNumber = offsetLineIt.second;
                                bytesToRead = nextOffset - currOffset;
                                bytesUsed = 0;

                                while (bytesToRead > 0)
                                {
                                    AMDTSourceAndDisasmInfo instInfo;

                                    instInfo.m_offset = currOffset;
                                    instInfo.m_sourceLine = static_cast<gtUInt16>(currLineNumber);

                                    const gtUByte* pCurrentCode = pCode + instInfo.m_offset;

                                    bool rc = GetDisassemblyString((BYTE*)pCurrentCode, isLongMode, instInfo, bytesUsed);

                                    if (rc)
                                    {
                                        srcInfoVec.push_back(instInfo);
                                        bytesToRead = (bytesToRead > bytesUsed) ? (bytesToRead - bytesUsed) : 0;
                                        currOffset += bytesUsed;
                                    }
                                    else
                                    {
                                        bytesToRead = 0;
                                    }
                                }

                                currOffset = nextOffset;
                                currLineNumber = nextLineNumber;
                            }
                        }
                        else
                        {
                            // Hmm..
                            AMDTProfileFunctionData functionData;
                            GetFunctionData(funcInfo.m_functionId, AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_THREADS, functionData);

                            for (auto& instData : functionData.m_instDataList)
                            {
                                AMDTUInt32 instOffset = instData.m_offset;

                                if (currOffset > instOffset)
                                {
                                    continue;
                                }

                                currOffset = instOffset;
                                bytesToRead = sectionSize - currOffset;

                                while (bytesToRead > 0)
                                {
                                    AMDTSourceAndDisasmInfo instInfo;

                                    instInfo.m_offset = currOffset;
                                    instInfo.m_sourceLine = static_cast<gtUInt16>(currLineNumber);

                                    const gtUByte* pCurrentCode = pCode + instInfo.m_offset;

                                    bool rc = GetDisassemblyString((BYTE*)pCurrentCode, isLongMode, instInfo, bytesUsed);

                                    if (rc)
                                    {
                                        srcInfoVec.push_back(instInfo);
                                        bytesToRead = (bytesToRead > bytesUsed) ? (bytesToRead - bytesUsed) : 0;
                                        currOffset += bytesUsed;
                                    }
                                    else
                                    {
                                        bytesToRead = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                retVal = CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE;
            }
        }

        return retVal;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    bool GetClrOffsetFromSymbol(ClrJncReader& clrJncReader, ExecutableFile& exe, gtRVAddr& offset)
    {
        bool ret = false;
        SymbolEngine* pSymbolEngine = exe.GetSymbolEngine();

        // Get IL info from CLR JNC file
        const gtUByte* pILMetaData = clrJncReader.GetILMetaData();
        ret = (nullptr != pSymbolEngine) && (nullptr != pILMetaData);

        unsigned int offsetToIl = 0;
        unsigned int ilSize = 0;
        ret = ret && clrJncReader.GetILInfo(&offsetToIl, &ilSize);

        if (ret)
        {
            const IMAGE_COR_ILMETHOD* pMethodHeader = reinterpret_cast<const IMAGE_COR_ILMETHOD*>(pILMetaData);
            bool bFatHeader = ((pMethodHeader->Fat.Flags & CorILMethod_FormatMask) == CorILMethod_FatFormat);

            unsigned int headerSize = (bFatHeader) ? 0xC : 0x1;
            unsigned int methodSize = ilSize - headerSize;

            wchar_t funcName[MAX_PATH];
            clrJncReader.GetFunctionName(funcName, MAX_PATH);

            offset = pSymbolEngine->LoadSymbol(funcName, methodSize);
            ret = GT_INVALID_RVADDR != offset;
        }

        return ret;
    }

    bool GetSourceLineInfoForCLR(ClrJncReader& clrJncReader, ExecutableFile& exe, gtUInt32 clrSymOffset, OffsetLinenumMap& jitLineMap)
    {
        bool ret = false;
        const gtUInt32 CLR_HIDDEN_LINE = 0x00feefee;
        //get map of il to native code
        const COR_DEBUG_IL_TO_NATIVE_MAP* pIlNativeMap = clrJncReader.GetILNativeMapInfo();

        if (nullptr != pIlNativeMap)
        {
            const COR_DEBUG_IL_TO_NATIVE_MAP* pILLineMap = pIlNativeMap;

            unsigned int ilNativeMapCount = clrJncReader.GetILNativeMapCount();

            jitLineMap.clear();

            //read line number map
            // Note: the key of map is the offset, the data is line number
            gtUInt32 minline = 0xFFFFFFFF;
            gtUInt32 maxline = 0;
            gtUInt32 previousLine = 1;
            gtUInt32 ilOffset = clrSymOffset;
            gtUInt32 ilPrevious = clrSymOffset;
            gtUInt32 maxNativeOffset = 0;

            for (unsigned int mapI = 0; mapI < ilNativeMapCount; mapI++, pILLineMap++)
            {
                switch (pILLineMap->ilOffset)
                {
                case NO_MAPPING:
                case PROLOG:
                    //negative, map to first or previous source line for source file purposes
                    ilOffset = ilPrevious;
                    break;

                case EPILOG:
                    //should use previous line's offset, as last bit
                    break;

                default:
                    ilOffset = clrSymOffset + pILLineMap->ilOffset;
                    break;
                }

                ilPrevious = ilOffset;

                // this is the IL that address belongs to
                SourceLineInfo srcLine;
                SymbolEngine* pSymbolEngine = exe.GetSymbolEngine();

                if (pSymbolEngine != nullptr && pSymbolEngine->FindSourceLine(ilOffset, srcLine))
                {
                    if (CLR_HIDDEN_LINE == srcLine.m_line)
                    {
                        srcLine.m_line = previousLine;
                    }

                    previousLine = srcLine.m_line;
                    jitLineMap[pILLineMap->nativeStartOffset] = srcLine.m_line;

                    if (srcLine.m_line < minline)
                    {
                        minline = srcLine.m_line;
                    }

                    if (srcLine.m_line > maxline)
                    {
                        maxline = srcLine.m_line;
                    }
                }
                else
                {
                    srcLine.m_line = previousLine;
                }

                if (pILLineMap->nativeEndOffset > maxNativeOffset)
                {
                    maxNativeOffset = pILLineMap->nativeEndOffset;
                }
            }

            ret = true;
        }

        return ret;
    }

    bool GetClrJitSrcFilePath(AMDTFunctionId funcId, gtString& srcFilePath)
    {
        bool ret = false;
        ExecutableFile* pExecutable = nullptr;
        ClrJncReader jncReader;
        gtRVAddr clrSymOffset;
        SourceLineInfo srcLine;

        ret = GetClrJitSrcLineInfo__(funcId, jncReader, pExecutable, clrSymOffset, srcLine);

        if (ret)
        {
            srcFilePath = srcLine.m_filePath;
            jncReader.Close();
        }

        return ret;
    }

    bool GetClrJitSrcLineInfo__(AMDTFunctionId funcId, ClrJncReader& jncReader, ExecutableFile*& pExecutable,
                                gtRVAddr& clrSymOffset,
                                SourceLineInfo& srcLine)
    {
        bool ret = false;

        if (nullptr != m_pDbAdapter)
        {
            // Get the loadaddress, srcfilepath, jncfilepath for the given function id
            gtString jncPath;
            gtVAddr jitLoadAddr;
            gtString srcFilePath; // This is from DB - which always contains "UnknownJITSource"

            if (m_pDbAdapter->GetJITFunctionInfo(funcId, jitLoadAddr, srcFilePath, jncPath))
            {
                AMDTModuleId modId = (funcId & 0xFFFF0000) >> 16;
                ret = GetModuleExecutable(modId, pExecutable, true);

                if (ret && (nullptr != pExecutable))
                {
                    AMDTProfileSessionInfo sessionInfo;

                    GetProfileSessionInfo(sessionInfo);
                    gtString jncFilePath = sessionInfo.m_sessionDir;
                    jncFilePath.append(osFilePath::osPathSeparator);
                    jncFilePath.append(jncPath);

                    if (jncReader.Open(jncFilePath.asCharArray()))
                    {
                        ret = GetClrOffsetFromSymbol(jncReader, *pExecutable, clrSymOffset);
                        SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();
                        ret = ret && (pSymbolEngine != nullptr) && pSymbolEngine->FindSourceLine(clrSymOffset, srcLine);
                    }
                }
            }
        }

        return ret;
    }

    int GetClrJitDisassembly(AMDTProfileFunctionInfo& funcInfo, AMDTSourceAndDisasmInfoVec& srcInfoVec)
    {
        // Get the loadaddress, srcfilepath, jncfilepath for the given function id
        int retVal = CXL_DATAACCESS_ERROR_DASM_INFO_NOTAVAILABLE;

        if (nullptr != m_pDbAdapter)
        {
            ExecutableFile* pExecutable = nullptr;
            ClrJncReader jncReader;
            gtRVAddr clrSymOffset;
            SourceLineInfo srcLine;

            bool ret = GetClrJitSrcLineInfo__(funcInfo.m_functionId, jncReader, pExecutable, clrSymOffset, srcLine);

            if (ret)
            {
                unsigned int codeOffset = 0;
                unsigned int codeSize = 0;
                const gtUByte* pCode = (ret) ? jncReader.GetCodeBytesOfTextSection(&codeOffset, &codeSize) : nullptr;

                if (pCode != nullptr)
                {
                    bool isLongMode = jncReader.Is64Bit();
                    OffsetLinenumMap funcOffsetLinenumMap;

                    GetSourceLineInfoForCLR(jncReader, *pExecutable, clrSymOffset, funcOffsetLinenumMap);

                    AMDTUInt32 currOffset = 0;
                    AMDTUInt32 currLineNumber = funcOffsetLinenumMap.begin()->second;

                    for (auto& offsetLineIt : funcOffsetLinenumMap)
                    {
                        AMDTUInt32 nextOffset = offsetLineIt.first;
                        AMDTUInt32 nextLineNumber = offsetLineIt.second;
                        AMDTUInt32 bytesToRead = nextOffset - currOffset;
                        AMDTUInt32 bytesUsed = 0;

                        while (bytesToRead > 0)
                        {
                            AMDTSourceAndDisasmInfo instInfo;

                            instInfo.m_offset = currOffset;
                            instInfo.m_sourceLine = static_cast<gtUInt16>(currLineNumber);

                            const gtUByte* pCurrentCode = pCode + instInfo.m_offset;

                            bool rc = GetDisassemblyString((BYTE*)pCurrentCode, isLongMode, instInfo, bytesUsed);

                            if (rc)
                            {
                                srcInfoVec.push_back(instInfo);
                                bytesToRead = (bytesToRead > bytesUsed) ? (bytesToRead - bytesUsed) : 0;
                                currOffset += bytesUsed;
                            }
                            else
                            {
                                bytesToRead = 0;
                            }
                        }

                        currOffset = nextOffset;
                        currLineNumber = nextLineNumber;
                    }

                    retVal = CXL_DATAACCESS_SUCCESS;
                }

                jncReader.Close();
            }
        }

        return retVal;
    }
#endif // AMDT_WINDOWS_OS

    bool GetDisassemblyString(BYTE* pCurrentCode, bool isLongMode, AMDTSourceAndDisasmInfo& disasmInfo, AMDTUInt32& bytesUsed)
    {
        bool ret = false;
        BYTE errorCode;
        UIInstInfoType instInfo;
        char dasmArray[256] = { 0 };
        unsigned int strlength = 255;

        if (nullptr != pCurrentCode)
        {
            // Setup disassembler
            LibDisassembler dasm;

            // if the code is 64-bits
            dasm.SetLongMode(isLongMode);

            memset(&instInfo, 0, sizeof(instInfo));

            // Get disassembly for the current pCode from the disassembler
            int hr = dasm.UIDisassemble((BYTE*)pCurrentCode, (unsigned int*)&strlength, (BYTE*)dasmArray, &instInfo, &errorCode);

            if (S_OK == hr)
            {
                bytesUsed = instInfo.NumBytesUsed;
                gtString& disasmStr = disasmInfo.m_disasmStr;
                disasmStr.fromASCIIString(dasmArray);

                if (instInfo.bIsPCRelative && instInfo.bHasDispData)
                {
                    disasmStr.appendFormattedString(L"(0x%lx)", disasmInfo.m_offset + instInfo.NumBytesUsed + instInfo.DispDataValue);
                }

                // Get codebytes
                char codeBytes[16];
                memcpy(codeBytes, pCurrentCode, instInfo.NumBytesUsed);

                for (int count = 0; count < instInfo.NumBytesUsed; count++)
                {
                    gtUByte byteCode = codeBytes[count];
                    gtUByte btHigh = (byteCode >> 4);
                    gtUByte btLow = (byteCode & 0xF);

                    disasmInfo.m_codeByteStr.append((btHigh <= 9) ? ('0' + btHigh) : ('A' + btHigh - 0xA));
                    disasmInfo.m_codeByteStr.append((btLow <= 9) ? ('0' + btLow) : ('A' + btLow - 0xA));
                    disasmInfo.m_codeByteStr << L" ";
                }

                disasmInfo.m_size = static_cast<gtUInt16>(bytesUsed);
                ret = true;
            }
            else
            {
                disasmInfo.m_size = 1;
                disasmInfo.m_disasmStr.fromASCIIString("BAD DASM");
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

        ret = (cssPid != cssProcesses.end()) ? true : false;

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

    bool ResetVisitFlagInFunctionNodes()
    {
        for (auto& aFuncNode : m_cgNodeVisitedList)
        {
            aFuncNode->m_isVisited = false;
        }

        m_cgNodeVisitedList.clear();
        return true;
    }

    bool GetFunctionNode(functionIdcgNodeMap& nodeMap,
                         CallstackFrame& frame,
                         double deepSamples,
                         gtUInt32 pathCount,
                         cgNode*& funcNode)
    {
        bool ret = false;
        AMDTFunctionId funcId = frame.m_funcInfo.m_functionId;

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
                it->second.m_pathCount += pathCount;
            }
            else
            {
                // Set the deep samples in the function node
                if (!it->second.m_isVisited)
                {
                    it->second.m_totalDeepSamples.m_sampleCount += deepSamples;
                    it->second.m_isVisited = true;
                    it->second.m_pathCount += pathCount;
                    m_cgNodeVisitedList.push_back(&(it->second));
                }
            }

            it->second.m_moduleBaseAddr = frame.m_moduleBaseAddr;
            it->second.m_isSystemModule = frame.m_isSystemodule;

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
        frame.m_isSystemodule = false;
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

        //  a directed backward edge from callee to caller (in callee's caller-edge vector)
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
                    ret = Lookupfunction(leaf.m_funcInfo, false, true);
                }

                m_handleUnknownLeafs.insert({ pid, pid });
            }
        }

        return ret;
    }

    bool UpdateUnknownCallstackFrames(CallstackFrameVec& frames)
    {
        bool ret = false;

        for (auto& frame : frames)
        {
            // Find the function info from debug info (if available) and update the tables
            ret = Lookupfunction(frame.m_funcInfo, false, false);
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
                // Commit and create new transaction
                m_pDbAdapter->FlushDb();

                m_cgNodeVisitedList.clear();

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
                ret = m_pDbAdapter->GetCallstackLeafData(pid,
                                                         counterId,
                                                         AMDT_PROFILE_ALL_CALLPATHS,
                                                         AMDT_PROFILE_ALL_FUNCTIONS,
                                                         0, // funcOffset
                                                         true, // aggregate all the leafs that corresponds to a callstackid
                                                         leafs);

                for (auto& leaf : leafs)
                {
                    // get the unique leafs of this callstack id...
                    CallstackFrameVec uniqueleafs;
                    gtUInt32 funcOffset = 0;
                    AMDTFunctionId funcId = AMDT_PROFILE_ALL_FUNCTIONS;

                    if (IS_UNKNOWN_FUNC(leaf.m_funcInfo.m_functionId))
                    {
                        funcId = leaf.m_funcInfo.m_functionId;
                        funcOffset = static_cast<gtUInt32>(leaf.m_funcInfo.m_startOffset);
                    }

                    ret = m_pDbAdapter->GetCallstackLeafData(pid,
                                                             counterId,
                                                             leaf.m_callstackId,
                                                             funcId,
                                                             funcOffset,
                                                             false, // don't aggregate all the leafs that corresponds to a callstackid
                                                             uniqueleafs);

                    double sampleCount = leaf.m_selfSamples; // deep samples
                    gtUInt32 pathCount = uniqueleafs.size();

                    // Set all the nodes as "not visited"
                    ResetVisitFlagInFunctionNodes();

                    // get the callstack frames (in order - 1 to depth (n-1))
                    CallstackFrameVec frames;
                    ret = m_pDbAdapter->GetCallstackFrameData(pid, leaf.m_callstackId, frames, false);

                    if (ret)
                    {
                        cgNode* pCallerNode = nullptr;
                        cgNode* pCalleeNode = nullptr;

                        UpdateUnknownCallstackFrames(frames);

                        // Connect this to [ROOT] node
                        ret = GetFunctionNode(*pCgFunctionMap, dummyRootFrame, sampleCount, pathCount, pCallerNode);

                        for (auto& frame : frames)
                        {
                            if (IS_UNKNOWN_FUNC(frame.m_funcInfo.m_functionId))
                            {
                                HandleUnknownFunction(frame.m_funcInfo);
                            }

                            // Handle recursion.
                            if (frame.m_funcInfo.m_functionId != pCallerNode->m_funcInfo.m_functionId)
                            {
                                ret = GetFunctionNode(*pCgFunctionMap, frame, sampleCount, pathCount, pCalleeNode);

                                if (nullptr != pCalleeNode)
                                {
                                    ret = ret && AddCallerToCalleeEdge(pCallerNode, pCalleeNode, sampleCount, false);
                                    ret = ret && AddCalleeToCallerEdge(pCallerNode, pCalleeNode, sampleCount);

                                    pCallerNode = pCalleeNode;
                                }
                            }

                            pCalleeNode = nullptr;
                        }

                        // Now add the unique leafs of this callstack id...
                        for (auto& uniqueleaf : uniqueleafs)
                        {
                            cgNode *pLeafNode = nullptr;
                            pathCount = 1;
                            sampleCount = static_cast<gtUInt32>(uniqueleaf.m_selfSamples);

                            if (IS_UNKNOWN_FUNC(uniqueleaf.m_funcInfo.m_functionId))
                            {
                                HandleUnknownFunction(uniqueleaf.m_funcInfo);
                            }

                            ret = GetFunctionNode(*pCgFunctionMap, uniqueleaf, sampleCount, 1, pLeafNode);

                            if (nullptr != pLeafNode)
                            {
                                ret = ret && AddCallerToCalleeEdge(pCallerNode, pLeafNode, sampleCount, true);
                                ret = ret && AddCalleeToCallerEdge(pCallerNode, pLeafNode, sampleCount);
                            }
                        }

                        uniqueleafs.clear();
                        frames.clear();
                    }
                }

                // Commit the changes
                m_pDbAdapter->FlushDb();
            }

            m_cgCounterId = (ret) ? counterId : 0;
        }

        return ret;
    }

#if 0
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
                ret = m_pDbAdapter->GetCallstackLeafData(pid,
                                                         counterId,
                                                         AMDT_PROFILE_ALL_CALLPATHS,
                                                         AMDT_PROFILE_ALL_FUNCTIONS,
                                                         0, // funcOffset
                                                         false,
                                                         leafs);

                if (ret)
                {
                    AMDTFunctionId prevFunctionId = 0;
                    gtUInt32 prevCallstackId = 0;

                    for (auto& leaf : leafs)
                    {
                        if (IS_UNKNOWN_FUNC(leaf.m_funcInfo.m_functionId))
                        {
                            HandleUnknownFunction(leaf.m_funcInfo);
                        }

                        // Get the function Node
                        cgNode *pLeafNode = nullptr;
                        double sampleCount = leaf.m_selfSamples;
                        gtUInt32 pathCount = ((prevFunctionId == leaf.m_funcInfo.m_functionId) && (prevCallstackId == leaf.m_callstackId)) ? 0 : 1;

                        // Set all the nodes as "not visited"
                        ResetVisitFlagInFunctionNodes(*pCgFunctionMap);

                        ret = GetFunctionNode(*pCgFunctionMap, leaf, sampleCount, pathCount, pLeafNode);

                        if (ret && nullptr != pLeafNode)
                        {
                            // get the callstack frames (in reverse order - depth (n-1) to 1)
                            CallstackFrameVec frames;
                            ret = m_pDbAdapter->GetCallstackFrameData(pid, leaf.m_callstackId, frames, true);

                            if (ret)
                            {
                                UpdateUnknownCallstackFrames(frames);

                                cgNode* pCallerNode = nullptr;
                                cgNode* pCalleeNode = pLeafNode;
                                bool isCalleeLeaf = true;

                                for (auto& frame : frames)
                                {
                                    if (IS_UNKNOWN_FUNC(frame.m_funcInfo.m_functionId))
                                    {
                                        HandleUnknownFunction(frame.m_funcInfo);
                                    }

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
#endif //0

    bool CopyCGNode(AMDTCallGraphFunction& cgFunc, cgNode& node, double totalDeepSamples)
    {
        cgFunc.m_functionInfo = node.m_funcInfo;
        cgFunc.m_moduleBaseAddr = node.m_moduleBaseAddr;
        cgFunc.m_pathCount = node.m_pathCount;

        cgFunc.m_totalDeepSamples = node.m_totalDeepSamples.m_sampleCount;
        cgFunc.m_totalSelfSamples = node.m_totalSelfSamples.m_sampleCount;

        if (cgFunc.m_totalDeepSamples > 0 && totalDeepSamples > 0)
        {
            cgFunc.m_deepSamplesPerc = CXL_COMPUTE_PERCENTAGE(cgFunc.m_totalDeepSamples, totalDeepSamples);
        }

        GetSrcFilePathForModuleOffset(node.m_funcInfo.m_moduleId,
                                      static_cast<gtUInt32>(node.m_funcInfo.m_startOffset),
                                      cgFunc.m_srcFile,
                                      cgFunc.m_srcFileLine);

        return true;
    }

    bool CopyCGEdge(AMDTCallGraphFunction& cgFunc, cgEdge& edge, double totalDeepSamples)
    {
        cgFunc.m_functionInfo = edge.m_funcInfo;
        cgFunc.m_moduleBaseAddr = edge.m_moduleBaseAddr;
        cgFunc.m_pathCount = 0;

        cgFunc.m_totalDeepSamples = edge.m_deepSamples.m_sampleCount;
        cgFunc.m_totalSelfSamples = edge.m_selfSamples.m_sampleCount;

        if (cgFunc.m_totalDeepSamples > 0 && totalDeepSamples > 0)
        {
            cgFunc.m_deepSamplesPerc = CXL_COMPUTE_PERCENTAGE(cgFunc.m_totalDeepSamples, totalDeepSamples);
        }

        return true;
    }

    bool CopyCGSelf(AMDTCallGraphFunction& cgFunc, cgNode& node, double totalDeepSamples)
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
            cgFunc.m_deepSamplesPerc = CXL_COMPUTE_PERCENTAGE(cgFunc.m_totalSelfSamples, totalDeepSamples);
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
                double totalDeepSamples = 0.0;

                if (pCgFunctionMap->end() != rootFuncIter)
                {
                    totalDeepSamples = rootFuncIter->second.m_totalDeepSamples.m_sampleCount;
                }

                for (auto& cgFunc : *pCgFunctionMap)
                {
#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
                    if (   (CXL_ROOT_FUNCTION_ID != cgFunc.first)
                        && (!(m_options.m_ignoreSystemModules && cgFunc.second.m_isSystemModule)))
#else
                    if (!(m_options.m_ignoreSystemModules && cgFunc.second.m_isSystemModule))
#endif
                    {
                        AMDTCallGraphFunction func;
                        CopyCGNode(func, cgFunc.second, totalDeepSamples);
                        cgFuncsVec.push_back(func);
                    }
                }
            }
        }

        if (ret)
        {
            std::sort(cgFuncsVec.begin(), cgFuncsVec.end(),
                [](AMDTCallGraphFunction const& a, AMDTCallGraphFunction const& b) { return a.m_totalDeepSamples > b.m_totalDeepSamples; });
        }

        return ret;
    }

    bool GetCallGraphFunctionInfo(AMDTProcessId processId, AMDTFunctionId funcId, AMDTCallGraphFunctionVec& parents, AMDTCallGraphFunctionVec& children)
    {
        bool ret = false;

        functionIdcgNodeMap* pCgFunctionMap = nullptr;

        ret = GetNodeFunctionMap(processId, m_cgCounterId, pCgFunctionMap);

        if (ret && (nullptr != pCgFunctionMap))
        {
            auto cgFuncIter = pCgFunctionMap->find(funcId);

            if (pCgFunctionMap->end() != cgFuncIter)
            {
                double totalDeepSamples = cgFuncIter->second.m_totalDeepSamples.m_sampleCount;

                for (auto& parent : cgFuncIter->second.m_callerVec)
                {
#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
                    if (parent.m_funcInfo.m_functionId != CXL_ROOT_FUNCTION_ID)
#endif
                    {
                        AMDTCallGraphFunction parentFunc;
                        CopyCGEdge(parentFunc, parent, totalDeepSamples);

                        parents.push_back(parentFunc);
                    }
                }

                for (auto& child : cgFuncIter->second.m_calleeVec)
                {
#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
                    if (child.m_funcInfo.m_functionId != CXL_ROOT_FUNCTION_ID)
#endif
                    {
                        AMDTCallGraphFunction childFunc;
                        CopyCGEdge(childFunc, child, totalDeepSamples);

                        children.push_back(childFunc);
                    }
                }

                // TBD: Should this [self] entry be added?
                // Add [self] in children vector
                AMDTCallGraphFunction self;
                CopyCGSelf(self, cgFuncIter->second, totalDeepSamples);
                children.push_back(self);

                ret = true;
            }
        }

        // Sort the parents and children based on DeepSamples
        if (ret)
        {
            std::sort(parents.begin(), parents.end(),
                [](AMDTCallGraphFunction const& a, AMDTCallGraphFunction const& b) { return a.m_totalDeepSamples > b.m_totalDeepSamples; });

            std::sort(children.begin(), children.end(),
                [](AMDTCallGraphFunction const& a, AMDTCallGraphFunction const& b) { return a.m_totalDeepSamples > b.m_totalDeepSamples; });
        }

        return ret;
    }

    bool GetCallGraphPaths(AMDTProcessId processId, AMDTFunctionId funcId, gtVector<AMDTCallGraphPath>& paths)
    {
        bool ret = false;
        AMDTFunctionId dbFuncId = funcId;
        AMDTProfileFunctionInfo unknownFuncInfo;

        bool isUnkownFunc = IsUnknownFunctionId(funcId, dbFuncId, unknownFuncInfo);

        AMDTUInt32 funcOffset = (isUnkownFunc) ? static_cast<gtUInt32>(unknownFuncInfo.m_startOffset) : 0;

        ret = GetCallGraphPathsForNonLeafFunction(processId, dbFuncId, funcOffset, paths);

        ret = GetCallGraphPathsForLeafFunction(processId, dbFuncId, funcOffset, paths);

        return ret;
    }

    bool GetCallGraphPathsForLeafFunction(AMDTProcessId processId, AMDTFunctionId funcId, gtUInt32 funcOffset, gtVector<AMDTCallGraphPath>& paths)
    {
        bool ret = false;
        gtVector<gtUInt32>  csIds;
        bool isLeafEntries = true;

        // This returns the unique callstack-ids in which the function is in the
        // callpath as a leaf
        ret = m_pDbAdapter->GetCallstackIds(processId, funcId, funcOffset, isLeafEntries, csIds);

        for (auto& csId : csIds)
        {
            // Get the leaf node
            CallstackFrameVec leafs;    // There will only one entry
            ret = m_pDbAdapter->GetCallstackLeafData(processId,
                                                     m_cgCounterId,
                                                     csId,
                                                     funcId,
                                                     funcOffset,
                                                     false,
                                                     leafs);

            // Ideally, there will be only one leaf
            for (auto& leaf : leafs)
            {
                if (IS_UNKNOWN_FUNC(leaf.m_funcInfo.m_functionId))
                {
                    HandleUnknownFunction(leaf.m_funcInfo);
                }

                gtUInt32 samples = leaf.m_selfSamples;

                // Get the callstack/callpath for this csId
                CallstackFrameVec frames;
                ret = m_pDbAdapter->GetCallstackFrameData(processId, csId, frames, true);

                // construct callpath
                AMDTCallGraphPath aPath;

                for (auto frameIt = frames.rbegin(); frameIt != frames.rend(); ++frameIt)
                {
                    AMDTProfileFunctionInfo& funcInfo = (*frameIt).m_funcInfo;

                    if (IS_UNKNOWN_FUNC(funcInfo.m_functionId))
                    {
                        HandleUnknownFunction(funcInfo);
                    }

                    AMDTCallGraphFunction cgFunc;
                    CopyCGFunction(cgFunc, (*frameIt), samples);
                    aPath.push_back(cgFunc);
                }

                // add the leaf node
                AMDTCallGraphFunction leafFunc;
                CopyCGFunction(leafFunc, leaf, samples);
                aPath.push_back(leafFunc);

                // push this callpath into output vector
                paths.push_back(aPath);
            }
        }

        return ret;
    }

    bool GetCallGraphPathsForNonLeafFunction(AMDTProcessId processId,
                                             AMDTFunctionId funcId,
                                             gtUInt32 funcOffset,
                                             gtVector<AMDTCallGraphPath>& paths)
    {
        bool ret = false;
        bool isLeafEntries = false;
        gtVector<gtUInt32> csIds;

        // This returns the unique callstack-ids in which the function is in the
        // callpath as a non-leaf
        ret = m_pDbAdapter->GetCallstackIds(processId, funcId, funcOffset, isLeafEntries, csIds);

        for (auto& csId : csIds)
        {
            // Fetch all the leaf entries for this callstack
            CallstackFrameVec leafs;
            ret = m_pDbAdapter->GetCallstackLeafData(processId,
                                                     m_cgCounterId,
                                                     csId,
                                                     AMDT_PROFILE_ALL_FUNCTIONS,
                                                     funcOffset,
                                                     false,
                                                     leafs);

            for (auto &leaf : leafs)
            {
                if (IS_UNKNOWN_FUNC(leaf.m_funcInfo.m_functionId))
                {
                    HandleUnknownFunction(leaf.m_funcInfo);
                }

                // Ignore the leaf-function with passed "funcId" as that callpath will be fetched by
                // GetCallGraphPathsForLeafFunction()
                if (leaf.m_funcInfo.m_functionId != funcId)
                {
                    gtUInt32 samples = leaf.m_selfSamples;

                    // Get the callstack/callpath for this csId
                    CallstackFrameVec frames;
                    ret = m_pDbAdapter->GetCallstackFrameData(processId, csId, frames, true);

                    // construct callpath
                    AMDTCallGraphPath aPath;

                    for (auto frameIt = frames.rbegin(); frameIt != frames.rend(); ++frameIt)
                    {
                        AMDTProfileFunctionInfo& funcInfo = (*frameIt).m_funcInfo;

                        if (IS_UNKNOWN_FUNC(funcInfo.m_functionId))
                        {
                            HandleUnknownFunction(funcInfo);
                        }

                        AMDTCallGraphFunction cgFunc;
                        CopyCGFunction(cgFunc, (*frameIt), samples);
                        aPath.push_back(cgFunc);
                    }

                    if (aPath.size() > 0)
                    {
                        // add the leaf node
                        AMDTCallGraphFunction leafFunc;
                        CopyCGFunction(leafFunc, leaf, samples);
                        aPath.push_back(leafFunc);

                        // push this callpath into output vector
                        paths.push_back(aPath);
                    }
                }
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
                gtString exePath = modInfo.m_path;

                // For CLR
                if (exePath.endsWith(L".jit"))
                {
                    exePath.truncate(0, (exePath.length() - 4));
                }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                bool b64bitSystemDir = false;
                PVOID oldValue = nullptr;

                if (m_is64BitSys)
                {
                    if (m_bGetSystemDir)
                    {
                        m_system32Dir[0] = L'\0';
                        // C:\windows\system32 or C:\winnt\system32
                        GetSystemDirectoryW(m_system32Dir, OS_MAX_PATH);
                        m_bGetSystemDir = false;
                    }

                    if (0 == _wcsnicmp(exePath.asCharArray(), m_system32Dir, wcslen(m_system32Dir)))
                    {
                        b64bitSystemDir = true;
                    }

                    if (b64bitSystemDir)
                    {
                        b64bitSystemDir = (Wow64DisableWow64FsRedirection(&oldValue) == TRUE);
                    }
                }
#endif // AMDT_WINDOWS_OS

                pExe = ExecutableFile::Open(exePath.asCharArray(), modInfo.m_loadAddress);

                if (nullptr != pExe)
                {
                    ret = (initSymbolEngine) ? InitializeSymbolEngine(pExe) : true;
                    m_moduleIdExeMap.insert({ moduleId, pExe });
                }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                if (b64bitSystemDir)
                {
                    Wow64RevertWow64FsRedirection(oldValue);
                }
#endif // AMDT_WINDOWS_OS
            }
        }

        pExecutable = pExe;
        return ret;
    }

    bool ConstructFuncNameByModName(AMDTModuleId modId, gtUInt64 offset, gtString& funcName)
    {
        AMDTProfileModuleInfo modInfo;
        bool ret = GetModuleInfo(modId, modInfo);

        if (ret && !modInfo.m_path.isEmpty())
        {
            osFilePath aPath(modInfo.m_path);
            aPath.getFileNameAndExtension(funcName);
            funcName.appendFormattedString(L"!0x" LONG_FORMAT_HEX, offset + modInfo.m_loadAddress);
        }
        else
        {
            // Add the unknownfunction
            funcName = L"Unknown Function";
            funcName.appendFormattedString(L"!0x" LONG_FORMAT_HEX, offset);
            ret = true; //?
        }

        return ret;
    }

    bool Lookupfunction(AMDTProfileFunctionInfo& funcInfo, bool updateIPSample, bool updateLeafs)
    {
        bool ret = true;
        gtUInt32 symEngineFuncId = funcInfo.m_functionId & 0xFFFFUL;

        if (0 == symEngineFuncId)
        {
            ret = HandleUnknownFunction(funcInfo);
        }

        // If it is system module and the user hasn't provide any path for debug-info/symbol-server,
        // then don't continue further
        //if (!ret)
        //{
        //    AMDTProfileModuleInfo modInfo;
        //    AMDTModuleId modId = CXL_GET_DB_MODULE_ID(funcInfo.m_functionId);

        //    if (GetModuleInfo(modId, modInfo) && modInfo.m_isSystemModule && !m_isDebugSearchPathAvailable)
        //    {
        //        ret = true;
        //    }
        //}

        // If we haven't seen this unknown function yet, process it
        if (!ret)
        {
            ExecutableFile* pExecutable = nullptr;

            ret = GetModuleExecutable(funcInfo.m_moduleId, pExecutable, true);
            ret = (ret && (nullptr != pExecutable)) ? true : false;

            if (ret)
            {
                ret = false;
                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();
                gtRVAddr funcRvaEnd = GT_INVALID_RVADDR;
                const FunctionSymbolInfo* pFuncSymbol = nullptr;

                if (pSymbolEngine != nullptr)
                {
                    pFuncSymbol = pSymbolEngine->LookupFunction(static_cast<gtRVAddr>(funcInfo.m_startOffset), &funcRvaEnd);
                }

                if (nullptr != pFuncSymbol)
                {
                    gtUInt32 funcSize = pFuncSymbol->m_size;

                    if ((funcSize == 0) && (GT_INVALID_RVADDR != funcRvaEnd))
                    {
                        funcSize = funcRvaEnd - pFuncSymbol->m_rva;
                    }

                    if (funcSize == 0)
                    {
                        // FIXME
                        gtUInt32 offset = static_cast<gtUInt32>(funcInfo.m_startOffset);
                        funcSize = (offset > pFuncSymbol->m_rva) ? (offset + 16) - pFuncSymbol->m_rva : 0;
                    }

                    // Only if we have found the function
                    if ((pFuncSymbol->m_rva <= funcInfo.m_startOffset) && ((pFuncSymbol->m_rva + funcSize) > funcInfo.m_startOffset))
                    {
                        if (nullptr != pFuncSymbol->m_pName && L'!' != pFuncSymbol->m_pName[0])
                        {
                            funcInfo.m_name = pFuncSymbol->m_pName;
                        }
                        else
                        {
                            ConstructFuncNameByModName(funcInfo.m_moduleId, pFuncSymbol->m_rva, funcInfo.m_name);
                        }

                        funcInfo.m_startOffset = pFuncSymbol->m_rva;
                        funcInfo.m_size = funcSize;
                        gtUInt32 maxFuncId = 0;

                        GetMaxFunctionIdByModuleId(funcInfo.m_moduleId, maxFuncId);
                        funcInfo.m_functionId = pFuncSymbol->m_funcId + maxFuncId;

                        m_pDbAdapter->InsertFunctionInfo(funcInfo);

                        if (updateIPSample)
                        {
                            m_pDbAdapter->UpdateIPSample(funcInfo);
                        }

                        if (updateLeafs)
                        {
                            m_pDbAdapter->UpdateCallstackLeaf(funcInfo);
                        }

                        m_pDbAdapter->UpdateCallstackFrame(funcInfo);

                        ret = true;
                    }
                }
            }

            if (!ret)
            {
                ConstructFuncNameByModName(funcInfo.m_moduleId, funcInfo.m_startOffset, funcInfo.m_name);

                funcInfo.m_size = 16; // FIXME
                AddUnknownFunctionInfo(funcInfo);
                ret = true;
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

bool cxlProfileDataReader::SetDebugInfoPaths(gtString& symbolDirectory, gtString& symbolServer, gtString& downloadPath)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetDebugInfoPaths(symbolDirectory, symbolServer, downloadPath);
    }

    return ret;
}

bool cxlProfileDataReader::SetSourcePaths(gtString& sourceDirPath)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetSourcePaths(sourceDirPath);
    }

    return ret;
}

bool cxlProfileDataReader::SetBinaryPaths(gtString& sourceDirPath)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->SetBinaryPaths(sourceDirPath);
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

bool cxlProfileDataReader::GetModuleInfoForFunction(AMDTFunctionId funcId, AMDTProfileModuleInfo& modInfo)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        AMDTModuleId modId = (funcId & 0xFFFF0000) >> 16;
        ret = m_pImpl->GetModuleInfo(modId, modInfo);
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

bool cxlProfileDataReader::GetFunctionInfoByModuleId(AMDTModuleId modId, AMDTProfileFunctionInfoVec& funcInfoVec, gtVAddr& modBaseAddr)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetFunctionInfoByModuleId(modId, funcInfoVec, modBaseAddr);
    }

    return ret;
}

bool cxlProfileDataReader::GetFunctionInfo(
    AMDTFunctionId             functionId,
    AMDTProfileFunctionInfo&   functionInfo,
    gtUInt64*                  pModLoadAddress,
    gtVector<AMDTProcessId>*  pProcessList,
    gtVector<AMDTThreadId>*   pThreadList)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetFunctionInfo(functionId, functionInfo, pModLoadAddress, pProcessList, pThreadList);
    }

    return ret;
}

bool cxlProfileDataReader::SessionHasSamples(void)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        AMDTSampleValueVec totalValueVec;

        ret = m_pImpl->GetSampleCount(false, totalValueVec);

        ret = ret && (totalValueVec.size() > 0);
    }

    return ret;
}

bool cxlProfileDataReader::GetSampleCount(bool sepByCore, AMDTSampleValueVec& totalValueVec)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetSampleCount(sepByCore, totalValueVec);
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
bool cxlProfileDataReader::GetProcessProfileData(AMDTProcessId procId, AMDTModuleId modId, AMDTProfileDataVec& processProfileData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetProcessProfileData(procId, modId, processProfileData);
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

bool cxlProfileDataReader::GetFunctionData(AMDTFunctionId            funcId,
                                           AMDTProcessId             processId,
                                           AMDTThreadId              threadId,
                                           AMDTProfileFunctionData&  functionData)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        if (funcId != 0xFFFFFFFFUL)
        {
            ret = m_pImpl->GetFunctionData(funcId, processId, threadId, functionData);
        }
    }

    return ret;
}

int cxlProfileDataReader::GetFunctionDetailedProfileData(AMDTFunctionId            funcId,
                                                          AMDTProcessId             processId,
                                                          AMDTThreadId              threadId,
                                                          AMDTProfileFunctionData&  functionData)
{
    int ret = false;

    if (nullptr != m_pImpl)
    {
        if (funcId != 0xFFFFFFFFUL)
        {
            ret = m_pImpl->GetFunctionDetailedProfileData(funcId, processId, threadId, functionData);
        }
    }

    return ret;
}

bool cxlProfileDataReader::GetSourceFilePathForFunction(AMDTFunctionId funcId, gtString& srcFilePath)
{
    bool ret = false;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetSrcFilePathForFuncId(funcId, srcFilePath);
    }

    return ret;
}

int cxlProfileDataReader::GetFunctionSourceAndDisasmInfo(AMDTFunctionId funcId, gtString& srcFilePath, AMDTSourceAndDisasmInfoVec& srcInfoVec)
{
    int ret = 0;

    if (nullptr != m_pImpl)
    {
        ret = m_pImpl->GetFunctionSourceAndDisasmInfo(funcId, srcFilePath, srcInfoVec);
    }

    return ret;
}

//int cxlProfileDataReader::GetDisassembly(AMDTModuleId moduleId,
//                                         AMDTUInt32 offset,
//                                         AMDTUInt32 size,
//                                         AMDTSourceAndDisasmInfoVec& disasmInfoVec)
//{
//    int ret = false;
//
//    if (nullptr != m_pImpl)
//    {
//        ret = m_pImpl->GetDisassembly(moduleId, offset, size, disasmInfoVec);
//    }
//
//    return ret;
//}

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
