//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File utils functions specifically for occupancy files
//==============================================================================

#include <fstream>
#include <iostream>
#include <string>
#include "OccupancyUtils.h"
#include "../Common/StringUtils.h"

using std::ifstream;

#define CHECKPARAM(param) \
    if (!b##param##Found) \
    { \
        strError = "Parameter \"" #param "\" not found in input file: " + strOccupancyFile; \
        return false; \
    }

#define PARSEPARAM(param, var) \
    if (strKey == #param) \
    { \
        if (StringUtils::Parse(strValue, params.var) == false) \
        { \
            strError = "Invalid value for \"" #param "\" in input file: " + strOccupancyFile; \
            return false; \
        } \
        b##param##Found = true; \
    }

const unsigned int MAX_PATH_LENGTH = 512;

bool OccupancyUtils::GetOccupancyParamsFromFile(const std::string& strOccupancyFile, OccupancyParams& params, std::string& strError)
{
    ifstream fin(strOccupancyFile.c_str());

    if (!fin.is_open())
    {
        strError = "Unable to open occupancy parameters file";
        return false;
    }

    char szBuffer[ MAX_PATH_LENGTH ];
    std::string strBuffer;
    std::string strKey;
    std::string strValue;
    size_t nIdx = 0;

    bool bThreadIDFound = false;
    bool bCallIndexFound = false;
    bool bKernelNameFound = false;
    bool bDeviceNameFound = false;
    bool bComputeUnitsFound = false;
    bool bMaxWavesPerComputeUnitFound = false;
    bool bMaxWorkGroupPerComputeUnitFound = false;
    bool bMaxVGPRsFound = false;
    bool bMaxSGPRsFound = false;
    bool bMaxLDSFound = false;
    bool bUsedVGPRsFound = false;
    bool bUsedSGPRsFound = false;
    bool bUsedLDSFound = false;
    bool bWavefrontSizeFound = false;
    bool bWorkGroupSizeFound = false;
    bool bWavesPerWorkGroupFound = false;
    bool bMaxWorkGroupSizeFound = false;
    bool bMaxWavesPerWorkGroupFound = false;
    bool bGlobalWorkSizeFound = false;
    bool bMaxGlobalWorkSizeFound = false;
    bool bWavesLimitedByVGPRFound = false;
    bool bWavesLimitedBySGPRFound = false;
    bool bWavesLimitedByLDSFound = false;
    bool bWavesLimitedByWorkgroupFound = false;
    bool bOccupancyFound = false;

    while (!fin.eof())
    {
        fin.getline(szBuffer, MAX_PATH_LENGTH);
        strBuffer.assign(szBuffer);
        nIdx = strBuffer.find("=");
        strKey = strBuffer.substr(0, nIdx);
        strValue = strBuffer.substr(nIdx + 1);

        PARSEPARAM(ThreadID, m_nTID);
        PARSEPARAM(CallIndex, m_nCallIndex);

        if (strKey.find("KernelName") != std::string::npos)
        {
            params.m_strKernelName = strValue;
            bKernelNameFound = true;
        }

        if (strKey.find("DeviceName") != std::string::npos)
        {
            params.m_strDeviceName = strValue;
            bDeviceNameFound = true;
        }

        PARSEPARAM(ComputeUnits, m_nNbrComputeUnits);
        PARSEPARAM(MaxWavesPerComputeUnit, m_nMaxWavesPerCU);
        PARSEPARAM(MaxWorkGroupPerComputeUnit, m_nMaxWGPerCU);
        PARSEPARAM(MaxVGPRs, m_nMaxVGPRS);
        PARSEPARAM(MaxSGPRs, m_nMaxSGPRS);
        PARSEPARAM(MaxLDS, m_nMaxLDS);
        PARSEPARAM(UsedVGPRs, m_nUsedVGPRS);
        PARSEPARAM(UsedSGPRs, m_nUsedSGPRS);
        PARSEPARAM(UsedLDS, m_nUsedLDS);
        PARSEPARAM(WavefrontSize, m_nWavefrontSize);
        PARSEPARAM(WorkGroupSize, m_nWorkgroupSize);
        PARSEPARAM(WavesPerWorkGroup, m_nWavesPerWG);
        PARSEPARAM(MaxWorkGroupSize, m_nMaxWGSize);
        PARSEPARAM(MaxWavesPerWorkGroup, m_nMaxWavesPerWG);
        PARSEPARAM(GlobalWorkSize, m_nGlobalWorkSize);
        PARSEPARAM(MaxGlobalWorkSize, m_nMaxGlobalWorkSize);
        PARSEPARAM(WavesLimitedByVGPR, m_nVGPRLimitedWaveCount);
        PARSEPARAM(WavesLimitedBySGPR, m_nSGPRLimitedWaveCount);
        PARSEPARAM(WavesLimitedByLDS, m_nLDSLimitedWaveCount);
        PARSEPARAM(WavesLimitedByWorkgroup, m_nWGLimitedWaveCount);
        PARSEPARAM(Occupancy, m_fOccupancy);
    }

    CHECKPARAM(ThreadID);
    CHECKPARAM(CallIndex);
    CHECKPARAM(KernelName);
    CHECKPARAM(DeviceName);
    CHECKPARAM(ComputeUnits);
    CHECKPARAM(MaxWavesPerComputeUnit);
    CHECKPARAM(MaxWorkGroupPerComputeUnit);
    CHECKPARAM(MaxVGPRs);
    CHECKPARAM(MaxSGPRs);
    CHECKPARAM(MaxLDS);
    CHECKPARAM(UsedVGPRs);
    CHECKPARAM(UsedSGPRs);
    CHECKPARAM(UsedLDS);
    CHECKPARAM(WavefrontSize);
    CHECKPARAM(WorkGroupSize);
    CHECKPARAM(WavesPerWorkGroup);
    CHECKPARAM(MaxWorkGroupSize);
    CHECKPARAM(MaxWavesPerWorkGroup);
    CHECKPARAM(GlobalWorkSize);
    CHECKPARAM(MaxGlobalWorkSize);
    CHECKPARAM(WavesLimitedByVGPR);
    CHECKPARAM(WavesLimitedBySGPR);
    CHECKPARAM(WavesLimitedByLDS);
    CHECKPARAM(WavesLimitedByWorkgroup);
    CHECKPARAM(Occupancy);

    if (!AMDTDeviceInfoUtils::Instance()->GetHardwareGeneration(params.m_strDeviceName.c_str(), params.m_gen))
    {
        return false;
    }

    strError.clear();
    return true;
}
