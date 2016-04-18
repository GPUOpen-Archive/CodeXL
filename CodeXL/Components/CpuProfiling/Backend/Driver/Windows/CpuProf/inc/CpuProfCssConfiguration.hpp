#ifndef _CPUPROF_CSSCONFIGURATION_HPP_
#define _CPUPROF_CSSCONFIGURATION_HPP_
#pragma once

#include "CpuProfCommon.hpp"
#include "UserAccess\CpuProfDriver.h"

namespace CpuProf {

class CssConfiguration
{
private:
    // The depth of CSS to attempt to trace.
    ULONG m_maxDepth;
    // Whether to trace user mode or kernel mode (inclusive).
    UCHAR m_mode;
    // Whether to capture and build the virtual stack.
    bool m_captureVirtualStack;
    // The interval between call-stack samples.
    ULONG m_sampleInterval;
    // The current CSS interval.
    ULONG* m_pLeftIntervals;

public:
    CssConfiguration();
    ~CssConfiguration();

    void Initialize(const CSS_PROPERTIES& props);

    void Clear();

    bool IsValid() const { return (NULL != m_pLeftIntervals); }
    ULONG GetMaxDepth() const { return m_maxDepth; }
    ULONG GetSampleInterval() const { return m_sampleInterval; }

    UCHAR GetMode() const { return m_mode; }
    bool IsUserMode() const { return 0 != (CSS_USER_MODE & m_mode); }
    bool IsKernelMode() const { return 0 != (CSS_KERNEL_MODE & m_mode); }

    bool IsCaptureStackPotentialValuesEnabled() const { return m_captureVirtualStack; }

    bool UpdateInterval(ULONG core);
};

} // namespace CpuProf

#endif // _CPUPROF_CSSCONFIGURATION_HPP_
