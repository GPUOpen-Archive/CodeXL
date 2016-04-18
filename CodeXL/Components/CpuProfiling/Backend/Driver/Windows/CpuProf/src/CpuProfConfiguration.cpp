#include "..\inc\CpuProfConfiguration.hpp"
#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfInternal.h"
#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union
#include "..\inc\CpuProfHdMsr.h"
#pragma warning(pop)

namespace CpuProf
{

Configuration::Configuration()
{
    RtlZeroMemory(this, sizeof(Configuration));
}


Configuration::~Configuration()
{
    if (NULL != m_pCoresMask)
    {
        ExFreePoolWithTag(m_pCoresMask, ALLOC_POOL_TAG);
    }
}


bool Configuration::InitializeEvent(const EVENT_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback)
{
    bool ret = SetCoresMask(reinterpret_cast<ULONG64*>(props.ullCpuMask.QuadPart), props.ulCoreMaskCount);

    if (ret)
    {
        Device* pDevice = Device::GetInstance();
        PERF_CTL perfCtl;

        // Reinterpret as the Hardware MSR representation.
        perfCtl.perf_ctl = props.ullEventCfg;

        // Determine the appropriate Pcore type.
        // Note that we assume all NB events will have the NorthBridge source bits in the EventSelect.
        if (0UL != pDevice->GetResourceCount(NB_CTR) && NB_SRC_BITS == (perfCtl.ucEventSelect & NB_SRC_BITS))
        {
            m_type = NB_CTR;
        }
        else if (0UL != pDevice->GetResourceCount(L2I_CTR) && FAKE_L2I_MASK_VALUE == perfCtl.FakeL2IMask)
        {
            m_type = L2I_CTR;
        }
        else
        {
            m_type = EVENT_CTR;
        }

        m_resourceId = props.ulCounterIndex;
        m_coresMaskCount = props.ulCoreMaskCount;

        m_pcoreConfig.count = TwosComplement(props.ullEventCount);
        m_pcoreConfig.msrControlValue = props.ullEventCfg;

        // If the configuration is sampling.
        if (0 != perfCtl.bitSampleEvents)
        {
            // Use the callback to record the sample data.
            m_pcoreConfig.fnDataCallback = pfnSampleDataCallback;
        }
        else
        {
            // Otherwise, keep track of the counting events.
            m_pcoreConfig.count = 0;
            m_pcoreConfig.fnDataCallback = NULL;
        }

        m_pcoreConfig.callbackArgument = reinterpret_cast<PVOID>(props.ulClientId);
    }

    return ret;
}


void Configuration::GetEventConfiguration(EVENT_PROPERTIES& props) const
{
    props.ulCounterIndex = m_resourceId;
    props.ullEventCount = TwosComplement(m_pcoreConfig.count);
    props.ullEventCfg = m_pcoreConfig.msrControlValue;
    props.ulClientId = reinterpret_cast<ULONG>(m_pcoreConfig.callbackArgument);
}


bool Configuration::InitializeTimer(const TIMER_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback)
{
    bool ret = SetCoresMask(reinterpret_cast<ULONG64*>(props.ullCpuMask.QuadPart), props.ulCoreMaskCount);

    if (ret)
    {
        m_type = APIC;
        m_resourceId = 0;
        m_coresMaskCount = props.ulCoreMaskCount;

        m_pcoreConfig.msrControlValue = 0;
        m_pcoreConfig.count = props.ulGranularity;
        m_pcoreConfig.fnDataCallback = pfnSampleDataCallback;
        m_pcoreConfig.callbackArgument = reinterpret_cast<PVOID>(props.ulClientId);
    }

    return ret;
}


void Configuration::GetTimerConfiguration(TIMER_PROPERTIES& props) const
{
    props.ulGranularity = static_cast<ULONG>(m_pcoreConfig.count);
}


bool Configuration::InitializeIbsFetch(const IBS_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback)
{
    bool ret = SetCoresMask(reinterpret_cast<ULONG64*>(props.ullCpuMask.QuadPart), props.ulCoreMaskCount);

    if (ret)
    {
        m_type = IBS_FETCH;
        m_resourceId = 0;
        m_coresMaskCount = props.ulCoreMaskCount;

        // Initialize Fetch Control Register.
        m_pcoreConfig.msrControlValue = MakeIbsFetchCtl(props.ulIbsFetchMaxCnt, 1);

        // CpuProf will always get the linear address.
        m_pcoreConfig.count = FETCH_CONTROL_MASK | FETCH_LINEAR_ADDR_MASK | props.fetchDataMask;

        m_pcoreConfig.fnDataCallback = pfnSampleDataCallback;
        m_pcoreConfig.callbackArgument = reinterpret_cast<PVOID>(props.ulClientId);
    }

    return ret;
}


void Configuration::GetIbsFetchConfiguration(IBS_PROPERTIES& props) const
{
    props.bProfileFetch = TRUE;
    props.ulIbsFetchMaxCnt = (0xFFFF & m_pcoreConfig.msrControlValue) << 4;
    props.fetchDataMask = static_cast<CXL_IBS_FETCH_MASK>(static_cast<LONG>(m_pcoreConfig.count) &
                                                          ~(FETCH_CONTROL_MASK | FETCH_LINEAR_ADDR_MASK));
}


bool Configuration::InitializeIbsOp(const IBS_PROPERTIES& props, DATA_CALLBACK pfnSampleDataCallback)
{
    bool ret = SetCoresMask(reinterpret_cast<ULONG64*>(props.ullCpuMask.QuadPart), props.ulCoreMaskCount);

    if (ret)
    {
        m_type = IBS_OP;
        m_resourceId = 0;
        m_coresMaskCount = props.ulCoreMaskCount;

        ULONG dispatchBit = static_cast<ULONG>(props.bOpDispatch);
        m_pcoreConfig.msrControlValue = MakeExtIbsOpCtl(props.ulIbsOpMaxCnt, dispatchBit);
        // CpuProf will always get the logical address and data 1.
        m_pcoreConfig.count = OP_CONTROL_MASK | OP_LOGIC_ADDR_MASK | OP_DATA_1_MASK | props.opDataMask;
        m_pcoreConfig.fnDataCallback = pfnSampleDataCallback;
        m_pcoreConfig.callbackArgument = reinterpret_cast<PVOID>(props.ulClientId);
    }

    return ret;
}


void Configuration::GetIbsOpConfiguration(IBS_PROPERTIES& props) const
{
    props.bProfileOp = TRUE;

    // Check for whether the dispatch bit is set.
    props.bOpDispatch = (0 != (m_pcoreConfig.msrControlValue & MSR_IBS_OPS_DISPATCH_BIT));

    // Save the user data settings.
    props.opDataMask = static_cast<CXL_IBS_OP_MASK>(static_cast<LONG>(m_pcoreConfig.count) &
                                                    ~(OP_CONTROL_MASK | OP_LOGIC_ADDR_MASK | OP_DATA_1_MASK));

    // Handle the extended count.
    props.ulIbsOpMaxCnt = ((0x7F00000 & m_pcoreConfig.msrControlValue) |
                           ((0xFFFF & m_pcoreConfig.msrControlValue) << 4));
}


bool Configuration::IsValidCore(ULONG core) const
{
    return (0UL == m_coresMaskCount || NULL == m_pCoresMask || 0ULL != (m_pCoresMask[core / 64] & (1ULL << (core % 64))));
}


bool Configuration::SetCoresMask(const ULONG64* pCoresMask, ULONG coresCount)
{
    bool ret;

    if (NULL != m_pCoresMask)
    {
        ExFreePoolWithTag(m_pCoresMask, ALLOC_POOL_TAG);
    }

    if (0UL != coresCount)
    {
        //allocate core masks
        ULONG maskLength = ((coresCount - 1UL) / 64) + 1UL;
        m_pCoresMask = static_cast<ULONG64*>(ExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG64) * maskLength, ALLOC_POOL_TAG));

        ret = (NULL != m_pCoresMask);

        if (ret)
        {
            RtlCopyMemory(m_pCoresMask, pCoresMask, (maskLength * sizeof(ULONG64)));
        }
    }
    else
    {
        m_pCoresMask = NULL;
        ret = true;
    }

    return ret;
}


bool Configuration::IsEvent() const
{
    return (EVENT_CTR == m_type || NB_CTR == m_type || L2I_CTR == m_type);
}


bool Configuration::IsCountingEvent() const
{
    return (IsEvent() && (0 == (MSR_CTL_SAMPLE_BIT & m_pcoreConfig.msrControlValue)));
}


} // namespace CpuProf
