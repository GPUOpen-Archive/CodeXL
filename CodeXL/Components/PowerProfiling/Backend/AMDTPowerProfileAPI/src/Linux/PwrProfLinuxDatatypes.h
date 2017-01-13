//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PwrProfLinuxDatatypes.h
///
//==================================================================================

#ifndef PWR_PROF_LINUX_DATATYPES_H_
#define PWR_PROF_LINUX_DATATYPES_H_

namespace PwrProfLinuxDatatypes
{
    #define PP_MAX_MODULE_COUNT (500)

    // Module information
    typedef struct ModInfo
    {
        AMDTUInt64  m_startAddress = 0;
        AMDTUInt64  m_endAddress = 0;
        char        m_pModulename[OS_MAX_PATH];
        AMDTUInt64  m_moduleId;
    } ModInfo;

    // table of Process => modules in a process
    typedef struct ProcPidModInfo
    {
        AMDTUInt64 m_processId;
        std::vector<ModInfo> m_modInfoTable;

        ProcPidModInfo()
        {
            m_modInfoTable.reserve(PP_MAX_MODULE_COUNT);
        }

        ~ProcPidModInfo()
        {
            m_modInfoTable.clear();
        }
    } ProcPidModInfo;

    // module profiling info
    typedef struct ModuleSampleData
    {
        AMDTUInt32  m_processId = 0;
        AMDTUInt32  m_threadId = 0;
        bool        m_isKernel = false;
        AMDTUInt64  m_ip = 0;
        AMDTFloat32 m_ipc = 0;
        AMDTUInt32  m_sampleCnt = 0;
        AMDTFloat32 m_power;
    } ModuleSampleData;

    using ProcPidModTable           = std::vector<ProcPidModInfo>;
    using ProcPidModTableItr        = std::vector<ProcPidModInfo>::iterator;
    using ModuleSampleDataMap       = std::map<AMDTUInt64, ModuleSampleData>;
    using ComponentModSampleTable   = std::vector<ModuleSampleDataMap>;
    using ModuleIdInfoMap           = std::map<AMDTUInt64, ModInfo>;
};

#endif
