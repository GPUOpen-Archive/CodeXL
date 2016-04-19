//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File utils functions specifically for occupancy files
//==============================================================================

#ifndef _OCCUPANCY_UTILS_H_
#define _OCCUPANCY_UTILS_H_

#include "DeviceInfoUtils.h"

namespace OccupancyUtils
{

struct OccupancyParams
{
    OccupancyParams() :
        m_nTID(0),
        m_nCallIndex(0),
        m_nNbrComputeUnits(0),
        m_nMaxWavesPerCU(0),
        m_nMaxWGPerCU(0),
        m_nMaxVGPRS(0),
        m_nMaxSGPRS(0),
        m_nMaxLDS(0),
        m_nUsedVGPRS(0),
        m_nUsedSGPRS(0),
        m_nUsedLDS(0),
        m_nWavefrontSize(0),
        m_nWorkgroupSize(0),
        m_nWavesPerWG(0),
        m_nMaxWGSize(0),
        m_nMaxWavesPerWG(0),
        m_nGlobalWorkSize(0),
        m_nMaxGlobalWorkSize(0),
        m_nVGPRLimitedWaveCount(0),
        m_nSGPRLimitedWaveCount(0),
        m_nLDSLimitedWaveCount(0),
        m_nWGLimitedWaveCount(0),
        m_fOccupancy(0.0f),
        m_gen(GDT_HW_GENERATION_NONE)
    {
        m_strKernelName.clear();
        m_strDeviceName.clear();
    }

    unsigned int      m_nTID;                   ///< Thread ID
    unsigned int      m_nCallIndex;             ///< Call index
    std::string       m_strKernelName;          ///< kernel name
    std::string       m_strDeviceName;          ///< device name
    unsigned int      m_nNbrComputeUnits;       ///< number of compute units on device
    unsigned int      m_nMaxWavesPerCU;         ///< max number of waves per compute unit
    unsigned int      m_nMaxWGPerCU;            ///< max number of work group per compute unit
    unsigned int      m_nMaxVGPRS;              ///< Max number of vector GPR on compute unit
    unsigned int      m_nMaxSGPRS;              ///< Max number of scalar GPR on compute unit
    unsigned int      m_nMaxLDS;                ///< Max amount of LDS on compute unit
    unsigned int      m_nUsedVGPRS;             ///< Number of vector GPR used by kernel
    unsigned int      m_nUsedSGPRS;             ///< Number of scalar GPR used by kernel
    unsigned int      m_nUsedLDS;               ///< Amount of LDS used by kernel (per work-group)
    unsigned int      m_nWavefrontSize;         ///< Number of work-items per work-group
    unsigned int      m_nWorkgroupSize;         ///< Number of work-items in work-group
    unsigned int      m_nWavesPerWG;            ///< Number of wavefronts in work-group
    unsigned int      m_nMaxWGSize;             ///< Max. number of work-items in a work-group
    unsigned int      m_nMaxWavesPerWG;         ///< Max. number of waves per work-group
    unsigned int      m_nGlobalWorkSize;        ///< Global number work items
    unsigned int      m_nMaxGlobalWorkSize;     ///< Global max. number of work-items
    unsigned int      m_nVGPRLimitedWaveCount;  ///< Number of wavefronts when the VGPR is the only limit on resources
    unsigned int      m_nSGPRLimitedWaveCount;  ///< Number of wavefronts when the SGPR is the only limit on resources
    unsigned int      m_nLDSLimitedWaveCount;   ///< Number of wavefronts when the shared memory is the only limit on resources
    unsigned int      m_nWGLimitedWaveCount;    ///< Number of wavefronts when the work-group size is the only constraint on resources
    float             m_fOccupancy;             ///< compute unit occupancy
    GDT_HW_GENERATION m_gen;                    ///< ASIC generation
};


bool GetOccupancyParamsFromFile(const std::string& strOccupancyFile, OccupancyParams& params, std::string& strError);
}
#endif
