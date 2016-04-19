#ifndef _CPUPROF_CONFIGURATION_HPP_
#define _CPUPROF_CONFIGURATION_HPP_
#pragma once

#include "CpuProfCommon.hpp"

namespace CpuProf {

// Holds the information for one configuration
class Configuration : public NonPagedObject
{
public:
    ///The list link field
    StackEntry m_linkField;

private:
    /// The count of cores for the mask passed in.  0 means all cores
    ULONG m_coresMaskCount;
    /// The array of masks of cores for which the configuration is valid
    /// The array size is (\ref coreMaskCount / 64)
    ULONG64* m_pCoresMask;
    ///What type of resource does this configuration apply to
    PCORERESOURCETYPES m_type;
    ///The resource counter (mostly only applies to EVENT_CTR, NB_CTR and L2I_CTR)
    ULONG m_resourceId;
    ///The configuration to profile
    PCORE_CONFIGURATION m_pcoreConfig;

public:
    Configuration();
    ~Configuration();

    bool InitializeEvent(const EVENT_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback);
    bool InitializeTimer(const TIMER_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback);
    bool InitializeIbsFetch(const IBS_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback);
    bool InitializeIbsOp(const IBS_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback);

    void GetEventConfiguration(EVENT_PROPERTIES& props) const;
    void GetTimerConfiguration(TIMER_PROPERTIES& props) const;
    void GetIbsFetchConfiguration(IBS_PROPERTIES& props) const;
    void GetIbsOpConfiguration(IBS_PROPERTIES& props) const;

    PCORERESOURCETYPES GetType() const { return m_type; }
    ULONG GetResourceId() const { return m_resourceId; }

    const PCORE_CONFIGURATION& GetPcoreConfiguration() const { return m_pcoreConfig; }
          PCORE_CONFIGURATION& GetPcoreConfiguration()       { return m_pcoreConfig; }

    bool IsValidCore(ULONG core) const;
    bool SetCoresMask(const ULONG64* pCoresMask, ULONG coresCount);
    const ULONG64* GetCoresMask() const { return m_pCoresMask; }
    ULONG GetCoresMaskCount() const { return m_coresMaskCount; }

    bool IsEvent() const;

    bool IsCountingEvent() const;
    void SetEventCount(ULONG64 count) { m_pcoreConfig.count = count; }
    ULONG64 GetEventCount() const { return m_pcoreConfig.count; }
};

} // namespace CpuProf

#endif // _CPUPROF_CONFIGURATION_HPP_
