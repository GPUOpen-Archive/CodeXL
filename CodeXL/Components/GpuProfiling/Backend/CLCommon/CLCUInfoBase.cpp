//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This is the description of the compute unit classes.
//==============================================================================

#include <map>
#include <cmath>
#include <cstdlib>
#include "CLCUInfoBase.h"
#include "DeviceInfoUtils.h"
#include "Logger.h"

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace std;
using namespace GPULogger;

const unsigned long CU_DEVICE_INFO_BUFFER_SIZE = 512UL;

int CLCUInfoBase::ComputeCUOccupancy(unsigned int uiWorkGroupSize)
{
    // Adjust max wave per CU to meet software limit.
    size_t nMaxWGPerCU = GetMaxWorkgroupPerCU(uiWorkGroupSize);
    size_t nMaxActiveWavefrontsPerCU = nMaxWGPerCU * (size_t)ceil((float)m_nWorkgroupItemCountMax / (float)m_nWavefrontSize);
    m_nMaxWavefrontsPerCU = min(nMaxActiveWavefrontsPerCU, m_nMaxWavefrontsPerCU);

    size_t nActiveWaves = 0;
    int status = ComputeNumActiveWaves(uiWorkGroupSize, nActiveWaves);

    if (status != AMD_CUPARAMS_LOADED)
    {
        Log(logWARNING, "Unable to compute max active waves\n");
        return status;
    }

    m_fOccupancy = (float)nActiveWaves / (float)m_nMaxWavefrontsPerCU;
    m_fOccupancy = (m_fOccupancy * 100.0f);

    return status;
}

int CLCUInfoBase::ReadCUParam(unsigned long param, size_t& value) const
{
    AmdCUParamsError status = AMD_CUPARAMS_LOADED;

    switch (param)
    {
        case CU_PARAMS_WAVEFRONT_PER_COMPUTE_UNIT:
            value = m_nMaxWavefrontsPerCU;
            break;

        case CU_PARAMS_NBR_COMPUTE_UNITS:
            value = m_nNbrComputeUnits;
            break;

        case CU_PARAMS_KERNEL_WG_SIZE:
            value = m_nWorkgroupItemCount;
            break;

        case CU_PARAMS_KERNEL_GLOBAL_SIZE:
            value = m_nGlobalWorkItemCount;
            break;

        case CU_PARAMS_WG_SIZE_MAX:
            value = m_nWorkgroupItemCountMax;
            break;

        case CU_PARAMS_GLOBAL_SIZE_MAX:
            value = m_nGlobalWorkItemCountMax;
            break;

        case CU_PARAMS_WAVEFRONT_SIZE:
            value = m_nWavefrontSize;
            break;

        default:
            status = AMD_CUPARAMS_INVALID_PARAMETER;
            break;
    }

    return int (status);
}

int CLCUInfoBase::ReadCUParam(unsigned long param, float& value) const
{
    AmdCUParamsError status = AMD_CUPARAMS_LOADED;

    switch (param)
    {
        case CU_PARAMS_KERNEL_OCCUPANCY:
            value = m_fOccupancy;
            break;

        default:
            value = 0.0f;
            SpBreak("Unknown cuparam");
            status = AMD_CUPARAMS_INVALID_PARAMETER;
            break;
    }

    return int(status);
}

int CLCUInfoBase::ReadCUParam(unsigned long param, std::string& strValue) const
{
    AmdCUParamsError status = AMD_CUPARAMS_LOADED;

    switch (param)
    {
        case CU_PARAMS_KERNEL_NAME:
            strValue = m_strKernelName;
            break;

        case CU_PARAMS_DEVICE_NAME:
            strValue = m_strDeviceName;
            break;

        default:
            strValue.assign("");
            SpBreak("Unknown cuparam");
            status = AMD_CUPARAMS_INVALID_PARAMETER;
            break;
    }

    return int(status);
}

void CLCUInfoBase::ClearCUParams()
{
    m_fOccupancy = -100.0f;
    m_nGlobalWorkItemCount = 0;
    m_nGlobalWorkItemCountMax = 0;
    m_nWorkgroupItemCount = 0;
    m_nWorkgroupItemCountMax = 0;
    m_nNbrComputeUnits = 0;
    m_nMaxWavefrontsPerCU = 0;
}

int CLCUInfoBase::SetCUParam(unsigned long param, size_t value)
{
    AmdCUParamsError status = AMD_CUPARAMS_LOADED;

    switch (param)
    {
        case CU_PARAMS_KERNEL_GLOBAL_SIZE:
            m_nGlobalWorkItemCount = value;
            break;

        case CU_PARAMS_KERNEL_WG_SIZE:
            m_nWorkgroupItemCount = value;
            break;

        case CU_PARAMS_WG_SIZE_MAX:
            m_nWorkgroupItemCountMax = value;
            break;

        case CU_PARAMS_GLOBAL_SIZE_MAX:
            m_nGlobalWorkItemCountMax = value;
            break;

        case CU_PARAMS_NBR_COMPUTE_UNITS:
            m_nNbrComputeUnits = value;
            break;

        case CU_PARAMS_WAVEFRONT_PER_COMPUTE_UNIT:
            m_nMaxWavefrontsPerCU = value;
            break;

        default:
            status = AMD_CUPARAMS_INVALID_PARAMETER;
            break;
    }

    return int (status);
}

int CLCUInfoBase::SetCUParam(unsigned long param, std::string strValue)
{
    AmdCUParamsError status = AMD_CUPARAMS_LOADED;

    switch (param)
    {
        case CU_PARAMS_DEVICE_NAME:
            m_strDeviceName = strValue;
            AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(strValue.c_str(), m_deviceInfo);
            m_nMaxWavefrontsPerCU = m_deviceInfo.m_nMaxWavePerSIMD * m_deviceInfo.m_nNumSIMDPerCU;
            m_nWavefrontSize = m_deviceInfo.m_nWaveSize;
            break;

        case CU_PARAMS_KERNEL_NAME:
            m_strKernelName = strValue;
            break;

        default:
            SpBreak("Unknown cuparam");
            status = AMD_CUPARAMS_INVALID_PARAMETER;
            break;
    }

    return int(status);
}

int CLCUInfoEGNI::ReadCUParam(unsigned long param, size_t& value) const
{
    int status = CLCUInfoBase::ReadCUParam(param, value);

    if (status == AMD_CUPARAMS_INVALID_PARAMETER)
    {
        status = AMD_CUPARAMS_LOADED;

        switch (param)
        {
            case CU_PARAMS_WF_MAX_VECTOR_GPRS:
                SpAssertRet(m_nMaxWavesVGPRSLimited != 0) AMD_CUPARAMS_OCCUPANCY_INFO_NOT_AVAILABLE;
                value = m_nMaxWavesVGPRSLimited;
                break;

            case CU_PARAMS_WF_MAX_LDS:
                SpAssertRet(m_nMaxWavesLDSLimited != 0) AMD_CUPARAMS_OCCUPANCY_INFO_NOT_AVAILABLE;
                value = m_nMaxWavesLDSLimited;
                break;

            case CU_PARAMS_WF_MAX_WG:
                SpAssertRet(m_nMaxWavesWGLimited != 0) AMD_CUPARAMS_OCCUPANCY_INFO_NOT_AVAILABLE;
                value = m_nMaxWavesWGLimited;
                break;

            case CU_PARAMS_VECTOR_GPRS_MAX:
                value = m_nMaxVGPRSPerSIMD;
                break;

            case CU_PARAMS_LDS_MAX:
                value = m_nMaxLDSPerCU;
                break;

            case CU_PARAMS_VECTOR_GPRS_USED:
                value = m_nVGPRSPerSIMD;
                break;

            case CU_PARAMS_LDS_USED:
                value = m_nLDSPerCU;
                break;

            default:
                status = AMD_CUPARAMS_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

int CLCUInfoEGNI::SetCUParam(unsigned long param, size_t value)
{
    int status = CLCUInfoBase::SetCUParam(param, value);

    if (status == AMD_CUPARAMS_INVALID_PARAMETER)
    {
        status = AMD_CUPARAMS_LOADED;

        switch (param)
        {
            case CU_PARAMS_VECTOR_GPRS_MAX:
                m_nMaxVGPRSPerSIMD = value;
                break;

            case CU_PARAMS_LDS_MAX:
                m_nMaxLDSPerCU = value;
                break;

            case CU_PARAMS_VECTOR_GPRS_USED:
                m_nVGPRSPerSIMD = value;
                break;

            case CU_PARAMS_LDS_USED:
                m_nLDSPerCU = value;
                break;

            default:
                status = AMD_CUPARAMS_INVALID_PARAMETER;
                break;
        }
    }

    return int (status);
}

void CLCUInfoEGNI::ClearCUParams()
{
    CLCUInfoBase::ClearCUParams();
    m_nVGPRSPerSIMD = 0;
    m_nLDSPerCU = 0;
    m_nMaxVGPRSPerSIMD = 0;
    m_nMaxLDSPerCU = 0;
    m_nMaxWavesVGPRSLimited = 0;
    m_nMaxWavesLDSLimited = 0;
    m_nMaxWavesWGLimited = 0;
    m_nWavefrontSize = 0;
}

int CLCUInfoEGNI::ComputeNumActiveWaves(unsigned int uiWorkGroupSize, size_t& numActiveWave)
{
    //parameters we need to compute the occupancy
    AmdCUParamsError status = AMD_CUPARAMS_LOADED;

    // Initialize the occupancy to -1.0 (which will signal a failure in the output
    // file if the calculation fails)
    m_fOccupancy = -1.0f;

    // a. wavefronts per work-group
    if (m_nWavefrontSize < 1)
    {
        Log(logWARNING, "m_nWavefrontSize < 1\n");
        ClearCUParams();
        return (int)AMD_CUPARAMS_WAVEFRONT_SIZE_INVALID;
    }

    size_t nMaxWGPerCU = GetMaxWorkgroupPerCU(uiWorkGroupSize);

    m_nWorkgroupItemCount = uiWorkGroupSize;

    // The user of the CLCUInfoBase object is responsible for handling the case where the work-group size is set to
    // zero (since the CL runtime will use a default work-group size and the enqueue call will not fail).  However, as this
    // object should not have any dependence on the run-time, we will not query the default work-group size
    // (using clGetKernelWorkGroupInfo) and instead signal an error.
    if (m_nWorkgroupItemCount == 0)
    {
        Log(logWARNING, "m_nWorkgroupItemCount == 0\n");
        ClearCUParams();
        return (int)AMD_CUPARAMS_WG_SIZE_INVALID;
    }

    size_t nWavefrontsPerWG = (size_t)ceil(double(m_nWorkgroupItemCount) / double(m_nWavefrontSize));

    if (nWavefrontsPerWG < 1)
    {
        Log(logWARNING, "nWavefrontsPerWG < 1\n");
        ClearCUParams();
        return (int) AMD_CUPARAMS_WAVEFRONT_COUNT_INVALID;
    }

    size_t nMinResources = 1;
    size_t nMinWavefronts = 1;

    //b. wavefronts per CU - register limits
    // WF(register_limited) = Nbr_Registers_per_CU/Nbr_registers_per_kernel
    if (m_nMaxVGPRSPerSIMD < 1)   //A compute unit should have AT LEAST 1 GPR; otherwise something is definitely wrong
    {
        Log(logWARNING, "m_nMaxVGPRSPerSIMD < 1\n");
        ClearCUParams();
        return (int)AMD_CUPARAMS_DEVICE_INVALID_VALUE;
    }

    m_nMaxWavesVGPRSLimited = m_nMaxVGPRSPerSIMD / max(m_nVGPRSPerSIMD, nMinResources);

    if (m_nMaxWavesVGPRSLimited > (nMaxWGPerCU * nWavefrontsPerWG))
    {
        m_nMaxWavesVGPRSLimited = nMaxWGPerCU * nWavefrontsPerWG;

        if (m_nMaxWavesVGPRSLimited > m_nMaxWavefrontsPerCU)
        {
            m_nMaxWavesVGPRSLimited = m_nMaxWavefrontsPerCU;
        }
    }

    //kernels are scheduled with wavegroup granularity, so the limiting number of wavefronts is set by the
    //max. number of wavegroups that can be mapped onto the compute unit
    m_nMaxWavesVGPRSLimited = (size_t)floor((double)m_nMaxWavesVGPRSLimited / (double)max(nWavefrontsPerWG, nMinWavefronts)) *
                              nWavefrontsPerWG;


    //c. wavefronts per CU - LDS limits
    // WF(local memory limited) = (local_mem(max)/local_mem_per_workgroup) * wavefronts_per_workgroup
    size_t nWorkgroupLDSLimited = (size_t)floor((double)m_nMaxLDSPerCU / (double)max(m_nLDSPerCU, nMinResources));

    // Check that there is no problem with the driver such that the LDS is reported as being zero (should be greater than
    // one if there is a physical device that is working and the driver is not broken)
    if (m_nMaxLDSPerCU < 1)
    {
        Log(logWARNING, "m_nMaxLDSPerCU < 1\n");
        ClearCUParams();
        return (int)AMD_CUPARAMS_DEVICE_INVALID_VALUE;
    }

    if (nWorkgroupLDSLimited > nMaxWGPerCU)
    {
        nWorkgroupLDSLimited = nMaxWGPerCU;
    }

    m_nMaxWavesLDSLimited = nWavefrontsPerWG * nWorkgroupLDSLimited;

    if (m_nMaxWavesLDSLimited > m_nMaxWavefrontsPerCU)
    {
        m_nMaxWavesLDSLimited = m_nMaxWavefrontsPerCU;
    }


    //max number of wavefronts is determined by the smallest of:
    //  -local mem limited wavefront count (which is limited by the number of work-groups on the SIMD due to local memory)
    //  -GPRS limited wavefront count
    if (m_nMaxWavefrontsPerCU < 1)
    {
        Log(logWARNING, "m_nMaxWavefrontsPerCU < 1\n");
        ClearCUParams();
        return (int) AMD_CUPARAMS_WAVEFRONT_SIZE_INVALID;
    }

    //assuming no register limits, stack size limits, or LDS limits, calculate the max number of wavefronts supported
    //on the compute unit based on work-group size (and the number of wavefronts supported by the compute unit)
    m_nMaxWavesWGLimited = (size_t)floor((double)m_nMaxWavefrontsPerCU / (double)nWavefrontsPerWG);
    m_nMaxWavesWGLimited = min(m_nMaxWavesWGLimited * nWavefrontsPerWG, nMaxWGPerCU * nWavefrontsPerWG);

    //the number of potential wavefronts on the compute unit is the minimum of the wavefront limits
    size_t tmp_min = (size_t)min(m_nMaxWavesLDSLimited, m_nMaxWavesWGLimited);
    numActiveWave = min(m_nMaxWavesVGPRSLimited, tmp_min);

    return status;
}

int CLCUInfoSI::ComputeNumActiveWaves(unsigned int uiWorkGroupSize, size_t& numActiveWave)
{
    size_t nMinResources = 1;
    size_t nMinWavefronts = 1;

    // Compute the work-group size limited number of wavefronts
    // Max. number of wavefronts = MAX_WF = 4 SIMD/CU * 10 WF/SIMD = 40
    // if WG-size = 1 WF/WG, Max WF = 40
    // if WG-size > 1 WF/WG, Max WF = floor [ min ( WG*WF/WG, MAX_WF )/(WF/WG)] * (WF/WG)
    size_t nNbrWavesPerWG = max((size_t)ceil((float)uiWorkGroupSize / (float)m_nWavefrontSize), nMinResources);

    size_t nMaxNbrFreeWaves = 0;
    size_t nMaxNbrWG = 0;

    nMaxNbrFreeWaves = nNbrWavesPerWG * GetMaxWorkgroupPerCU(uiWorkGroupSize);
    m_nMaxWavesWGLimited = min(nMaxNbrFreeWaves, (m_deviceInfo.m_nNumSIMDPerCU * m_deviceInfo.m_nMaxWavePerSIMD));
    nMaxNbrWG = (size_t)floor((float)m_nMaxWavesWGLimited / (float)nNbrWavesPerWG);
    m_nMaxWavesWGLimited = nMaxNbrWG * nNbrWavesPerWG;

    //A compute unit should have AT LEAST 1 GPR; otherwise something is definitely wrong
    ///TODO: Check ClearCUParams and clean up any additional resources used for the SI occupancy
    if (m_nMaxVGPRSPerSIMD < 1)
    {
        Log(logWARNING, "m_nMaxVGPRSPerSIMD < 1\n");
        ClearCUParams();
        return (int)AMD_CUPARAMS_DEVICE_INVALID_VALUE;
    }

    // Compute VGPR limited wavefronts, per SIMD
    // WF(register_limited) = Nbr_Registers_per_SIMD/Nbr_registers_per_kernel
    size_t numVGPRs = max(m_nVGPRSPerSIMD, nMinResources);
    numVGPRs = RoundUpVGPRs(numVGPRs); // take into account the allocation granularity for the hardware

    m_nMaxWavesVGPRSLimited = m_nMaxVGPRSPerSIMD / numVGPRs;
    m_nMaxWavesVGPRSLimited *= m_deviceInfo.m_nNumSIMDPerCU;

    if (m_nMaxWavesVGPRSLimited > m_nMaxWavesWGLimited)
    {
        m_nMaxWavesVGPRSLimited = m_nMaxWavesWGLimited;
    }

    //kernels are scheduled with workgroup granularity, so the limiting number of wavefronts is set by the
    //max. number of wavegroups that can be mapped onto the compute unit
    m_nMaxWavesVGPRSLimited = (size_t)floor((double)m_nMaxWavesVGPRSLimited / (double)max(nNbrWavesPerWG, nMinWavefronts)) *
                              nNbrWavesPerWG;


    // Compute SGPR limited wavefronts per SIMD
    size_t numSGPRs = max(this->m_nSGPRSPerSIMD + 2, nMinResources); // adding 2 so we take into account the 2 VCC registers (may be fixed in driver 15.20)
    numSGPRs = RoundUpSGPRs(numSGPRs); // take into account the allocation granularity for the hardware
    m_nMaxWavesSGPRSLimited = GetMaxSGPRsInHardware() / numSGPRs;
    m_nMaxWavesSGPRSLimited *= m_deviceInfo.m_nNumSIMDPerCU;

    if (m_nMaxWavesSGPRSLimited > m_nMaxWavesWGLimited)
    {
        m_nMaxWavesSGPRSLimited = m_nMaxWavesWGLimited;
    }

    //kernels are scheduled with workgroup granularity, so the limiting number of wavefronts is set by the
    //max. number of wavegroups that can be mapped onto the compute unit
    m_nMaxWavesSGPRSLimited = (size_t)floor((double)m_nMaxWavesSGPRSLimited / (double)max(nNbrWavesPerWG, nMinWavefronts)) *
                              nNbrWavesPerWG;

    // Compute LDS-limited wavefronts
    // WF(local memory limited) = (local_mem(max)/local_mem_per_workgroup) * wavefronts_per_workgroup
    size_t nWorkgroupLDSLimited = (size_t)floor((double)m_nMaxLDSPerCU / (double)max(m_nLDSPerCU, nMinResources));

    // Check that there is no problem with the driver such that the LDS is reported as being zero (should be greater than
    // one if there is a physical device that is working and the driver is not broken)
    ///TODO: Check ClearCUParams and clean up any additional resources used for the SI occupancy
    if (m_nMaxLDSPerCU < 1)
    {
        Log(logWARNING, "m_nMaxLDSPerCU < 1\n");
        ClearCUParams();
        return (int)AMD_CUPARAMS_DEVICE_INVALID_VALUE;
    }

    m_nMaxWavesLDSLimited = nNbrWavesPerWG * nWorkgroupLDSLimited;

    if (m_nMaxWavesLDSLimited > m_nMaxWavesWGLimited)
    {
        m_nMaxWavesLDSLimited = m_nMaxWavesWGLimited;
    }

    //max number of wavefronts is determined by the smallest of:
    //  -local mem limited wavefront count
    //  -GPRS limited wavefront count
    ///TODO: Check ClearCUParams and clean up any additional resources used for the SI occupancy
    if (m_nMaxWavefrontsPerCU < 1)
    {
        Log(logWARNING, "m_nMaxWavefrontsPerCU < 1\n");
        ClearCUParams();
        return (int) AMD_CUPARAMS_WAVEFRONT_SIZE_INVALID;
    }

    //the number of potential wavefronts on the compute unit is the minimum of the wavefront limits
    numActiveWave = 0;
    size_t tmp_min = (size_t)min(m_nMaxWavesLDSLimited, m_nMaxWavesWGLimited);
    numActiveWave = min(m_nMaxWavesVGPRSLimited, tmp_min);
    numActiveWave = min(m_nMaxWavesSGPRSLimited, numActiveWave);

    return 0;
}

int CLCUInfoSI::ReadCUParam(unsigned long param, size_t& value) const
{
    int status = CLCUInfoEGNI::ReadCUParam(param, value);

    if (status == AMD_CUPARAMS_INVALID_PARAMETER)
    {
        status = AMD_CUPARAMS_LOADED;

        switch (param)
        {
            case CU_PARAMS_SCALAR_GPRS_USED:
                value = m_nSGPRSPerSIMD;
                break;

            case CU_PARAMS_SCALAR_GPRS_MAX:
                value = m_nMaxSGPRSPerSIMD;
                break;

            case CU_PARAMS_WF_MAX_SCALAR_GPRS:
                SpAssertRet(m_nMaxWavesSGPRSLimited != 0) AMD_CUPARAMS_OCCUPANCY_INFO_NOT_AVAILABLE;
                value = m_nMaxWavesSGPRSLimited;
                break;

            default:
                SpBreak("Unknown cuparam");
                status = AMD_CUPARAMS_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

int CLCUInfoSI::SetCUParam(unsigned long param, size_t value)
{
    int status = CLCUInfoEGNI::SetCUParam(param, value);

    if (status == AMD_CUPARAMS_INVALID_PARAMETER)
    {
        status = AMD_CUPARAMS_LOADED;

        switch (param)
        {
            case CU_PARAMS_SCALAR_GPRS_USED:
                m_nSGPRSPerSIMD = value;
                break;

            case CU_PARAMS_SCALAR_GPRS_MAX:
                m_nMaxSGPRSPerSIMD = value;
                break;

            default:
                SpBreak("Unknown cuparam");
                status = AMD_CUPARAMS_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

void CLCUInfoSI::ClearCUParams()
{
    CLCUInfoEGNI::ClearCUParams();
    m_nMaxSGPRSPerSIMD = 0;
    m_nSGPRSPerSIMD = 0;
    m_nMaxWavesSGPRSLimited = 0;
}

size_t CLCUInfoSI::GetMaxSGPRsInHardware() const
{
    // Runtime reports max sgpr a kernel can allocate which is 102,
    // but it doesn't report MAX_SGPR per compute unit, we hardcode it here.
    // When calculating occupancy, we are calculating how many work groups can fit into a CU,
    return 512;
}

size_t CLCUInfoSI::GetSGPRAllocationGranularity() const
{
    // For SI/CI, SGPRs must be allocated in groups of 8
    return 8;
}

size_t CLCUInfoSI::RoundUpSGPRs(size_t input) const
{
    size_t retVal = input;
    size_t allocationGranularity = GetSGPRAllocationGranularity();

    if (0 < allocationGranularity)
    {
        size_t remainder = input % allocationGranularity;

        if (0 < remainder)
        {
            retVal = input + allocationGranularity - remainder;
        }
    }

    return retVal;
}

size_t CLCUInfoSI::RoundUpVGPRs(size_t input) const
{
    size_t retVal = input;
    const size_t VGPR_ALLOCATION_GRANULARITY = 4; // 4 is used for all GCN (up to VI so far)
    size_t remainder = input % VGPR_ALLOCATION_GRANULARITY;

    if (0 < remainder)
    {
        retVal = input + VGPR_ALLOCATION_GRANULARITY - remainder;
    }

    return retVal;
}

size_t CLCUInfoVI::GetMaxSGPRsInHardware() const
{
    // Runtime reports max sgpr a kernel can allocate which is 102,
    // but it doesn't report MAX_SGPR per compute unit, we hardcode it here.
    // When calculating occupancy, we are calculating how many work groups can fit into a CU,
    return 800;
}

size_t CLCUInfoVI::GetSGPRAllocationGranularity() const
{
    // For GFX8/GFX9, SGPRs must be allocated in groups of 16
    return 16;
}
