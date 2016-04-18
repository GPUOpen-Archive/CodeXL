//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages kernel statistics for kernel dispatches
//==============================================================================

#include "KernelStats.h"

KernelInfo::KernelInfo() : m_nScratchReg(KERNELINFO_NONE),
    m_nWavefrontPerSIMD(KERNELINFO_NONE),
    m_nWavefrontSize(KERNELINFO_NONE),
    m_nAvailableGPRs(KERNELINFO_NONE),
    m_nUsedGPRs(KERNELINFO_NONE),
    m_nAvailableLDSSize(KERNELINFO_NONE),
    m_nUsedLDSSize(KERNELINFO_NONE),
    m_nAvailableStackSize(KERNELINFO_NONE),
    m_nUsedStackSize(KERNELINFO_NONE),
    m_nAvailableScalarGPRs(KERNELINFO_NONE),
    m_nUsedScalarGPRs(KERNELINFO_NONE)
{
}


KernelStats::KernelStats()
{
    m_strName.clear();

    for (unsigned int i = 0; i < 3; ++i)
    {
        m_globalWorkSize[i] = 1;
        m_workGroupSize[i] = 1;
    }

    m_uWorkDim = 0;
    m_dTime = 0.0;
    m_kernelInfo.m_nAvailableGPRs = KERNELINFO_NONE;
    m_kernelInfo.m_nScratchReg = KERNELINFO_NONE;
    m_kernelInfo.m_nWavefrontPerSIMD = KERNELINFO_NONE;
    m_kernelInfo.m_nWavefrontSize = KERNELINFO_NONE;
    m_kernelInfo.m_nAvailableGPRs = KERNELINFO_NONE;
    m_kernelInfo.m_nUsedGPRs = KERNELINFO_NONE;
    m_kernelInfo.m_nAvailableLDSSize = KERNELINFO_NONE;
    m_kernelInfo.m_nUsedLDSSize = KERNELINFO_NONE;
    m_kernelInfo.m_nAvailableStackSize = KERNELINFO_NONE;
    m_kernelInfo.m_nUsedStackSize = KERNELINFO_NONE;
    m_kernelInfo.m_nAvailableScalarGPRs = KERNELINFO_NONE;
    m_kernelInfo.m_nUsedScalarGPRs = KERNELINFO_NONE;
    m_threadId = 0;
    m_uSequenceId = 0;
}

KernelStats::KernelStats(const KernelStats& ks)
{
    m_strName = ks.m_strName;

    for (unsigned int i = 0; i < 3; ++i)
    {
        m_globalWorkSize[i] = ks.m_globalWorkSize[i];
        m_workGroupSize[i] = ks.m_workGroupSize[i];
    }

    m_uWorkDim                          = ks.m_uWorkDim;
    m_dTime                             = ks.m_dTime;
    m_kernelInfo.m_nScratchReg          = ks.m_kernelInfo.m_nScratchReg;
    m_kernelInfo.m_nWavefrontPerSIMD    = ks.m_kernelInfo.m_nWavefrontPerSIMD;
    m_kernelInfo.m_nWavefrontSize       = ks.m_kernelInfo.m_nWavefrontSize;
    m_kernelInfo.m_nAvailableGPRs       = ks.m_kernelInfo.m_nAvailableGPRs;
    m_kernelInfo.m_nUsedGPRs            = ks.m_kernelInfo.m_nUsedGPRs;
    m_kernelInfo.m_nAvailableLDSSize    = ks.m_kernelInfo.m_nAvailableLDSSize;
    m_kernelInfo.m_nUsedLDSSize         = ks.m_kernelInfo.m_nUsedLDSSize;
    m_kernelInfo.m_nAvailableStackSize  = ks.m_kernelInfo.m_nAvailableStackSize;
    m_kernelInfo.m_nUsedStackSize       = ks.m_kernelInfo.m_nUsedStackSize;
    m_kernelInfo.m_nAvailableScalarGPRs = ks.m_kernelInfo.m_nAvailableScalarGPRs;
    m_kernelInfo.m_nUsedScalarGPRs      = ks.m_kernelInfo.m_nUsedScalarGPRs;
    m_threadId                          = ks.m_threadId;
    m_uSequenceId                       = ks.m_uSequenceId;
}

KernelStats& KernelStats::operator=(const KernelStats& ks)
{
    if (this != &ks)
    {
        m_strName = ks.m_strName;

        for (unsigned int i = 0; i < 3; ++i)
        {
            m_globalWorkSize[i] = ks.m_globalWorkSize[i];
            m_workGroupSize[i] = ks.m_workGroupSize[i];
        }

        m_uWorkDim                          = ks.m_uWorkDim;
        m_dTime                             = ks.m_dTime;
        m_kernelInfo.m_nScratchReg          = ks.m_kernelInfo.m_nScratchReg;
        m_kernelInfo.m_nWavefrontPerSIMD    = ks.m_kernelInfo.m_nWavefrontPerSIMD;
        m_kernelInfo.m_nWavefrontSize       = ks.m_kernelInfo.m_nWavefrontSize;
        m_kernelInfo.m_nAvailableGPRs       = ks.m_kernelInfo.m_nAvailableGPRs;
        m_kernelInfo.m_nUsedGPRs            = ks.m_kernelInfo.m_nUsedGPRs;
        m_kernelInfo.m_nAvailableLDSSize    = ks.m_kernelInfo.m_nAvailableLDSSize;
        m_kernelInfo.m_nUsedLDSSize         = ks.m_kernelInfo.m_nUsedLDSSize;
        m_kernelInfo.m_nAvailableStackSize  = ks.m_kernelInfo.m_nAvailableStackSize;
        m_kernelInfo.m_nUsedStackSize       = ks.m_kernelInfo.m_nUsedStackSize;
        m_kernelInfo.m_nAvailableScalarGPRs = ks.m_kernelInfo.m_nAvailableScalarGPRs;
        m_kernelInfo.m_nUsedScalarGPRs      = ks.m_kernelInfo.m_nUsedScalarGPRs;
        m_threadId                          = ks.m_threadId;
        m_uSequenceId                       = ks.m_uSequenceId;
    }

    return *this;
}
