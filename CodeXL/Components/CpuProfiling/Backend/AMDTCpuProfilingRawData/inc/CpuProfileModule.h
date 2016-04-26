//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileModule.h
///
//==================================================================================

#ifndef _CPUPROFILEMODULE_H_
#define _CPUPROFILEMODULE_H_

#include <tuple>
#include <AMDTCommonProfileDataTypes.h>
#include "CpuProfileFunction.h"

CP_RAWDATA_API bool ExtractFileName(const gtString& fullPath, gtString& fileName);

/****************************************
 * class CpuProfileModule
 *
 * Description:
 * This class represent the item in [MODULE] section
 */
class CP_RAWDATA_API CpuProfileModule
{
public:

    enum MOD_MODE_TYPE
    {
        INVALIDMODTYPE = 0,
        UNMANAGEDPE,
        JAVAMODULE,
        MANAGEDPE,
        OCLMODULE,
        UNKNOWNMODULE,
        UNKNOWNKERNELSAMPLES
    };

    CpuProfileModule();

    bool operator< (const CpuProfileModule& m) const { return (m_base < m.m_base);  }
    bool operator==(const CpuProfileModule& m) const { return (m_base == m.m_base); }

    void setPath(const gtString& path) { m_path = path; }
    const gtString& getPath() const { return m_path; }

    int getImdIndex() const { return m_imdIndex; }
    void setImdIndex(int imdIndex) { m_imdIndex = imdIndex; }

    bool extractFileName(gtString& fileName) const;

    unsigned int getNumSubSection() const { return static_cast<unsigned int>(m_funcMap.size()); }

    gtUInt64 getTotal() const;
    gtVAddr getBaseAddr() const;

    gtUInt32 getModType() const { return m_modType; }

    bool isIndirect() const { return m_isIndirect; }
    void setIndirect(bool indirect) { m_isIndirect = indirect; }

    bool isSystemModule() const { return m_isSystemModule; }
    void setSystemModule(bool isSysMod) { m_isSystemModule = isSysMod; }

    gtUInt64 getChecksum() const { return m_checksum; }
    bool isDebugInfoAvailable() const { return m_isDebugInfoAvailable; }

    AddrFunctionMultMap& getFunctionMap() { return m_funcMap; }

    AddrFunctionMultMap::const_iterator getBeginFunction() const { return m_funcMap.begin(); }
    AddrFunctionMultMap::iterator       getBeginFunction()       { return m_funcMap.begin(); }
    AddrFunctionMultMap::const_iterator getEndFunction() const  { return m_funcMap.end(); }
    AddrFunctionMultMap::iterator       getEndFunction()        { return m_funcMap.end(); }

    const CpuProfileFunction* findFunction(gtVAddr addr) const;
    CpuProfileFunction* findFunction(gtVAddr addr);

    PidAggregatedSampleMap::const_iterator getBeginSample() const { return m_aggPidMap.begin(); }
    PidAggregatedSampleMap::const_iterator getEndSample() const { return m_aggPidMap.end(); }
    PidAggregatedSampleMap::const_iterator findSampleForPid(ProcessIdType pid) const { return m_aggPidMap.find(pid); }

    bool isUnchartedFunction(const CpuProfileFunction& func) const;
    const CpuProfileFunction* getUnchartedFunction() const;
    CpuProfileFunction* getUnchartedFunction();

    /*
     * NOTE:
     * This function is used by CaDataTranslator to
     * record sample after PRD translation. It adds
     *sample into the PidAggregatedSampleMap (for [MODULE] section)
     * and the AddrFunctionMultMap (for IMD file).
     */
    void recordSample(const SampleInfo& sampleInfo,
                      gtUInt32 sampleCnt,
                      gtVAddr funcAddr,
                      gtUInt32 funcSize,
                      const gtString& funcName,
                      const gtString& jncName,
                      const gtString& srcFile,
                      unsigned srcLine = 0U,
                      gtUInt32 functionId = 0);

    void recordSample(ProcessIdType pid, const AggregatedSample* pAggSample);
    void recordSample(gtVAddr addr, const CpuProfileFunction* pFunc);

    void recordFunction(gtVAddr addr, const CpuProfileFunction* pFunc);

    void insertModMetaData(const gtString& modPath, int imdIndex, ProcessIdType pid, const AggregatedSample& agSamp);
    void insertModMetaData(ProcessIdType pid, const AggregatedSample& agSamp);
    void insertModDetailData(gtVAddr addr, const CpuProfileFunction& func);

    bool doesAddressBelongToModule(gtVAddr addr, const CpuProfileFunction** ppFunc) const;

    bool compares(const CpuProfileModule& m) const;

    void clear();

public:
    gtVAddr m_base;
    gtUInt32 m_size;
    gtUInt32 m_modType;
    gtUInt64 m_checksum;

    bool m_isImdRead;
    bool m_is32Bit;
    bool m_isIndirect;
    bool m_isSystemModule;
    bool m_symbolsLoaded;           // FIXME. why do we need?
    bool m_isDebugInfoAvailable;    // Whether Debug Info (PDB/Dwarf/Stab) is available during data-translation

    // Unique module id for each module
    gtUInt32 m_moduleId = 0;
    // List of different module instances <pid,loadAddr,instanceId> for each module
    gtVector<std::tuple<gtUInt64, gtUInt64, gtUInt32>> m_moduleInstanceInfo;

private:
    int m_imdIndex;
    gtUInt64 m_total; // Updated by recordSample
    gtString m_path;

    // Used by CpuProfileReader/Writer to store
    // information from [MODDATA] section
    gtUInt64 m_aggTotal;
    PidAggregatedSampleMap m_aggPidMap;

    // Used by IMDReader/Writer to store
    // information from module detail section
    gtUInt64 m_funcTotal;
    AddrFunctionMultMap m_funcMap;
};

/***********************************************
 * Description:
 * This represent the [MODULE] section
 */
typedef gtMap<gtString, CpuProfileModule> NameModuleMap;

#endif // _CPUPROFILEMODULE_H_
