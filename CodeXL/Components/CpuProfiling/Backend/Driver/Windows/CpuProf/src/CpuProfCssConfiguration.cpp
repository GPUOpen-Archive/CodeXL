#include "..\inc\CpuProfCssConfiguration.hpp"
#include "..\inc\CpuProfDevice.hpp"

namespace CpuProf
{

CssConfiguration::CssConfiguration() : m_maxDepth(0UL),
    m_mode(0),
    m_captureVirtualStack(false),
    m_sampleInterval(0UL)
{
    ULONG coresCount = GetCoresCount();

    m_pLeftIntervals = reinterpret_cast<ULONG*>(ExAllocatePoolWithTag(NonPagedPool, (coresCount * sizeof(ULONG)), ALLOC_POOL_TAG));

    if (NULL != m_pLeftIntervals)
    {
        RtlZeroMemory(m_pLeftIntervals, (coresCount * sizeof(ULONG)));
    }
}


CssConfiguration::~CssConfiguration()
{
    if (NULL != m_pLeftIntervals)
    {
        ExFreePoolWithTag(m_pLeftIntervals, ALLOC_POOL_TAG);
    }
}


void CssConfiguration::Initialize(const CSS_PROPERTIES& props)
{
    ASSERT(IsValid());

    m_maxDepth = props.ulCSSDepth;
    m_mode = props.ucTargetSamplingMode;
    m_captureVirtualStack = (FALSE != props.bCaptureVirtualStack);
    m_sampleInterval = props.ulCSSInterval;
}


void CssConfiguration::Clear()
{
    m_maxDepth = 0UL;
    m_mode = 0;
    m_sampleInterval = 0UL;

    if (NULL != m_pLeftIntervals)
    {
        RtlZeroMemory(m_pLeftIntervals, (GetCoresCount() * sizeof(ULONG)));
    }
}


bool CssConfiguration::UpdateInterval(ULONG core)
{
    ASSERT(IsValid());
    ASSERT(GetCoresCount() > core);

    bool ret = (0 == m_pLeftIntervals[core]);

    // If at call-stack Interval.
    if (ret)
    {
        // Reset call-stack interval.
        m_pLeftIntervals[core] = m_sampleInterval;
    }

    m_pLeftIntervals[core]--;

    return ret;
}

} // namespace CpuProf
