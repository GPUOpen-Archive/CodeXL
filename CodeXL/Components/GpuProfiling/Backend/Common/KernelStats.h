//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages kernel statistics for kernel dispatches
//==============================================================================

#ifndef _KERNEL_STATS_H_
#define _KERNEL_STATS_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "OSUtils.h"

/// A const to indicate invalid field in the KernelInfo structure
const unsigned int KERNELINFO_NONE = (unsigned int) - 1; ///< to indicate that the field in KernelInfo is not valid

/// Kernel info provided by runtime
struct KernelInfo
{
    size_t m_nScratchReg;          ///< Number of scratch register used
    size_t m_nWavefrontPerSIMD;    ///< Number of Wavefront per SIMD on the fly
    size_t m_nWavefrontSize;       ///< Wavefront size - 64 for NI and EG
    size_t m_nAvailableGPRs;       ///< Available (vector) GPRs specific to ASIC
    size_t m_nUsedGPRs;            ///< Used (vector) GPR for this kernel
    size_t m_nAvailableLDSSize;    ///< Available LDS specific to ASIC
    size_t m_nUsedLDSSize;         ///< Used LDS
    size_t m_nAvailableStackSize;  ///< Available stack size specific to ASIC
    size_t m_nUsedStackSize;       ///< Used stack size
    size_t m_nAvailableScalarGPRs; ///< Available scalar GPRs specific to ASIC
    size_t m_nUsedScalarGPRs;      ///< Used scalar GPR

    /// Default Constructor
    KernelInfo();
};

/// \addtogroup CLProfileAgent
// @{

/// A class to manage the OpenCL kernel statistics
class KernelStats
{
public:
    /// default constructor.
    KernelStats();

    /// Destructor.
    ~KernelStats() {}

    /// copy constructor.
    /// \param ks  the rhs object
    KernelStats(const KernelStats& ks);

    /// operator assignment.
    /// \param ks  the rhs object
    /// \return the reference to the object
    KernelStats& operator=(const KernelStats& ks);

    std::string   m_strName;           ///< kernel name
    size_t        m_globalWorkSize[3]; ///< total work size for each dimension (the value in the unused dimension should be 1).
    size_t        m_workGroupSize[3];  ///< work group size for each dimension (the value in the unused dimension should be 1).
    unsigned int  m_uWorkDim;          ///< the work dimension (can be 1 to 3)
    double        m_dTime;             ///< time spent executing the kernel in milliseconds (does not include the kernel setup time)
    KernelInfo    m_kernelInfo;        ///< kernel info from the runtime
    osThreadId    m_threadId;          ///< thread id of the thread that enqueued the kernel
    unsigned int  m_uSequenceId;       ///< the sequence id of the enqueue api that enqueued the kernel
};

// @}

#endif // _KERNEL_STATS_H_
