//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This is the description of the compute unit classes.
//==============================================================================

#ifndef _CL_CU_INFO_BASE_H_
#define _CL_CU_INFO_BASE_H_

#include <string>
#include <map>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "Defs.h"
#include "DeviceInfo.h"

enum AmdCUParamsError
{
    AMD_CUPARAMS_DEVICE_NOT_GPU = 1,  //not an error, as CU info read (so not negative), but specify non-gpu device
    AMD_CUPARAMS_LOADED = 0,
    AMD_CUPARAMS_DEVICE_NOT_FOUND = -1,
    AMD_CUPARAMS_DEVICE_INFO_NOT_KNOWN = -2,
    AMD_CUPARAMS_WAVEFRONT_INFO_NOT_KNOWN = -3,
    AMD_CUPARAMS_VGPRS_INFO_NOT_KNOWN = -4,
    AMD_CUPARAMS_SGPRS_INFO_NOT_KNOWN = -5,
    AMD_CUPARAMS_LM_INFO_NOT_KNOWN = -6,
    AMD_CUPARAMS_STACK_INFO_NOT_KNOWN = -7,
    AMD_CUPARAMS_INVALID_PARAMETER = -8,
    AMD_CUPARAMS_NOT_SET = -9,
    AMD_CUPARAMS_WAVEFRONT_SIZE_INVALID = -10,
    AMD_CUPARAMS_WAVEFRONT_COUNT_INVALID = -11,
    AMD_CUPARAMS_WG_SIZE_NOT_KNOWN = -12,
    AMD_CUPARAMS_WG_SIZE_INVALID = -13,
    AMD_CUPARAMS_GLOBAL_SIZE_NOT_KNOWN = -14,
    AMD_CUPARAMS_DEVICE_INVALID_VALUE = -15,
    AMD_CUPARAMS_OCCUPANCY_INFO_NOT_AVAILABLE = -16
};

///enum listing the different compute unit parameters recorded by the compute unit data class
enum CLCUParamOptions
{
    CU_PARAMS_VECTOR_GPRS_MAX = 1,
    CU_PARAMS_SCALAR_GPRS_MAX,
    CU_PARAMS_LDS_MAX,
    CU_PARAMS_WAVEFRONT_PER_COMPUTE_UNIT,
    CU_PARAMS_VECTOR_GPRS_USED,
    CU_PARAMS_SCALAR_GPRS_USED,
    CU_PARAMS_LDS_USED,
    CU_PARAMS_DEVICE_NAME,
    CU_PARAMS_WAVEFRONT_SIZE,
    CU_PARAMS_NBR_COMPUTE_UNITS,
    CU_PARAMS_DEVICE_TYPE,
    CU_PARAMS_WF_MAX_VECTOR_GPRS,
    CU_PARAMS_WF_MAX_SCALAR_GPRS,
    CU_PARAMS_WF_MAX_LDS,
    CU_PARAMS_WF_MAX_WG,
    CU_PARAMS_KERNEL_WG_SIZE,
    CU_PARAMS_KERNEL_GLOBAL_SIZE,
    CU_PARAMS_KERNEL_OCCUPANCY,
    CU_PARAMS_KERNEL_NAME,
    CU_PARAMS_WG_SIZE_MAX,
    CU_PARAMS_GLOBAL_SIZE_MAX
};

//------------------------------------------------------------------------------------
// Theoretical CU occupancy calculator
// records compute unit parameters and computes performance parameters based on these
//------------------------------------------------------------------------------------
class CLCUInfoBase
{
public:

    ///Constructor
    CLCUInfoBase()
        : m_nNbrComputeUnits(0),
          m_nMaxWavefrontsPerCU(0),
          m_nWavefrontSize(0),
          m_nWorkgroupItemCount(0),
          m_nWorkgroupItemCountMax(0),
          m_nGlobalWorkItemCount(0),
          m_nGlobalWorkItemCountMax(0),
          m_fOccupancy(0)
    {
        m_strDeviceName.clear();
        m_strKernelName.clear();
    }

    ///Destructor
    virtual ~CLCUInfoBase() {}

    /// method to access the compute unit parameters - 2 overloaded versions: one to read out
    /// unsigned long data and the other one to read out string data
    ///\param param parameter being queried
    ///\param value value being retrieved
    ///\return status of retrieval operation
    virtual int ReadCUParam(unsigned long param, size_t& value) const;

    /// method to access the compute unit parameters - 2 overloaded versions: one to read out
    /// unsigned long data and the other one to read out string data
    ///\param param parameter being queried
    ///\param value value being retrieved
    ///\return status of retrieval operation
    virtual int ReadCUParam(unsigned long param, std::string& value) const;

    /// method to access the compute unit parameters - 2 overloaded versions: one to read out
    /// unsigned long data and the other one to read out string data
    ///\param param parameter being queried
    ///\param value value being retrieved
    ///\return status of retrieval operation
    virtual int ReadCUParam(unsigned long param, float& value) const;

    /// Set size_t CU parameter
    /// \param param Parameter type
    /// \param value Value to be set
    /// \return AMD_CUPARAMS_LOADED if succeeded
    virtual int SetCUParam(unsigned long param, size_t value);

    /// Set string CU parameter
    /// \param param Parameter type
    /// \param strValue Value to be set
    /// \return AMD_CUPARAMS_LOADED if succeeded
    virtual int SetCUParam(unsigned long param, std::string strValue);

    /// Get max number of active workgroup per CU
    /// \param nWorkgroupSize Flattened work group size
    /// \return max number of active workgroup per CU
    virtual size_t GetMaxWorkgroupPerCU(size_t nWorkgroupSize) const = 0;

    /// ComputeCUOccupancy
    /// Method to compute the occupancy of the compute units
    /// \param [in] uiWorkGroupSize workgroup size
    /// \return status of calculation (success or error)
    int ComputeCUOccupancy(unsigned int uiWorkGroupSize);

    /// Compute number of active waves
    /// \param[in] uiWorkGroupSize Work group size
    /// \param[out] nNumActiveWave output number of active waves
    /// \return status of calculation
    virtual int ComputeNumActiveWaves(unsigned int uiWorkGroupSize, size_t& nNumActiveWave) = 0;

protected:
    /// Clear CU paramters
    virtual void ClearCUParams();

    std::string m_strDeviceName;     ///< Device name
    std::string m_strKernelName;     ///< Kernel name

    size_t m_nNbrComputeUnits;       ///< Number of CU
    size_t m_nMaxWavefrontsPerCU;    ///< compute unit: max. number of wavefronts supported
    size_t m_nWavefrontSize;         ///< compute unit: number of work-items per wave front
    size_t m_nWorkgroupItemCount;    ///< size of work-group (number of work-items)
    size_t m_nWorkgroupItemCountMax; ///< maximum size of work-group
    size_t m_nGlobalWorkItemCount;   ///< total number of work-items
    size_t m_nGlobalWorkItemCountMax;///< maximum number of work-items supported by compute unit

    float m_fOccupancy;              ///< compute unit: occupancy
    GDT_DeviceInfo m_deviceInfo;     ///< Device info object
};

//------------------------------------------------------------------------------------
// Evergreen, NorthernIsland theoretical CU occupancy calculator
//------------------------------------------------------------------------------------
class CLCUInfoEGNI : public CLCUInfoBase
{
public:
    /// Constructor
    CLCUInfoEGNI()
        : CLCUInfoBase(),
          m_nMaxVGPRSPerSIMD(0),
          m_nMaxLDSPerCU(0),
          m_nVGPRSPerSIMD(0),
          m_nLDSPerCU(0),
          m_nMaxWavesVGPRSLimited(0),
          m_nMaxWavesLDSLimited(0),
          m_nMaxWavesWGLimited(0)
    {}

    virtual int ReadCUParam(unsigned long param, size_t& value) const;
    virtual int SetCUParam(unsigned long param, size_t value);

    virtual size_t GetMaxWorkgroupPerCU(size_t nWorkgroupSize) const
    {
        SP_UNREFERENCED_PARAMETER(nWorkgroupSize);
        return 8;
    }

    virtual int ComputeNumActiveWaves(unsigned int uiWorkGroupSize, size_t& nNumActiveWave);
protected:
    virtual void ClearCUParams();

    size_t m_nMaxVGPRSPerSIMD;       ///< compute unit: max. number of vector registers
    size_t m_nMaxLDSPerCU;           ///< compute unit: max. amount of shared memory (LDS)

    size_t m_nVGPRSPerSIMD;          ///< compute unit: number of vector GPRS used by kernel
    size_t m_nLDSPerCU;              ///< compute unit: amount of LDS used by kernel

    size_t m_nMaxWavesVGPRSLimited;  ///< max number of wavefronts - limited by the number of vector GPRS
    size_t m_nMaxWavesLDSLimited;    ///< max number of wavefronts - limited by the LDS
    size_t m_nMaxWavesWGLimited;     ///< max number of wavefronts - limited by wave-group size
};

//------------------------------------------------------------------------------------
// SouthernIsland theoretical CU occupancy calculator
//   Also used for SeaIslands
//------------------------------------------------------------------------------------
class CLCUInfoSI : public CLCUInfoEGNI
{
public:
    ///Constructor
    CLCUInfoSI()
        : CLCUInfoEGNI(),
          m_nMaxSGPRSPerSIMD(0),
          m_nSGPRSPerSIMD(0),
          m_nMaxWavesSGPRSLimited(0)
    {
    }

    virtual size_t GetMaxWorkgroupPerCU(size_t nWorkgroupSize) const
    {
        if (nWorkgroupSize == 0)
        {
            return 0;
        }

        size_t nNbrWavesPerWG = std::max((size_t)std::ceil((float)nWorkgroupSize / (float)m_nWavefrontSize), (size_t)1);

        if (nNbrWavesPerWG == 1)
        {
            return 40;   // set to max wave size
        }
        else
        {
            return 16;
        }
    }

    virtual int ReadCUParam(unsigned long param, size_t& value) const;
    virtual int SetCUParam(unsigned long param, size_t value);

    virtual int ComputeNumActiveWaves(unsigned int uiWorkGroupSize, size_t& nNumActiveWave);

protected:
    virtual void ClearCUParams();

    /// Gets the max number of SGPRs available in the hardware
    /// \return the max number of SGPRs available in the hardware
    virtual size_t GetMaxSGPRsInHardware() const;

    /// Gets the allocation granularity for SGPRs (SGPRs have to be allocated in groups of 8, 16, etc...)
    /// \return the allocation granularity for SGPRs
    virtual size_t GetSGPRAllocationGranularity() const;

    /// Rounds the input number up SGPRs to the nearest multiple of the allocation granularity
    /// \param input the input number
    /// \return the input rounded up to the nearest multiple of the allocation granularity
    size_t RoundUpSGPRs(size_t input) const;

    /// Rounds the input number up VGPRs to the nearest multiple of the allocation granularity
    /// \param input the input number
    /// \return the input rounded up to the nearest multiple of the allocation granularity
    size_t RoundUpVGPRs(size_t input) const;

    size_t m_nMaxSGPRSPerSIMD;       ///< SIMD: max. number of scalar registers
    size_t m_nSGPRSPerSIMD;          ///< SIMD: number of scalar GPRS used by kernel
    size_t m_nMaxWavesSGPRSLimited;  ///< max number of wavefronts per CU - limited by the number of scalar GPRS
};

//------------------------------------------------------------------------------------
// VolcanicIsland (GFX8) theoretical CU occupancy calculator
//    Also used for GFX9
//------------------------------------------------------------------------------------
class CLCUInfoVI : public CLCUInfoSI
{
protected:
    /// Gets the max number of SGPRs available in the hardware
    /// \return the max number of SGPRs available in the hardware
    virtual size_t GetMaxSGPRsInHardware() const;

    /// Gets the allocation granularity for SGPRs (SGPRs have to be allocated in groups of 8, 16, etc...)
    /// \return the allocation granularity for SGPRs
    virtual size_t GetSGPRAllocationGranularity() const;
};

#endif
