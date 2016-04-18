//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OccupancyInfo.cpp $
/// \version $Revision: #5 $
/// \brief  This file contains OccupancyInfo class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OccupancyInfo.cpp#5 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include "OccupancyInfo.h"
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>


OccupancyInfo::OccupancyInfo(uint threadId,
                             const QString& kernelName,
                             const QString& deviceName,
                             uint computeUnits,
                             uint maxWavesPerComputeUnit,
                             uint maxWGPerComputeUnit,
                             uint maxVGPRs,
                             uint maxSGPRs,
                             uint maxLDS,
                             uint usedVGPRs,
                             uint usedSGPRs,
                             uint usedLDS,
                             uint wavefrontSize,
                             uint workGroupSize,
                             uint wavesPerWorkGroup,
                             uint maxWorkGroupSize,
                             uint maxWavesPerWorkGroup,
                             uint globalWorkSize,
                             uint maxGlobalWorkSize,
                             uint wavesLimitedByVGPR,
                             uint wavesLimitedBySGPR,
                             uint wavesLimitedByLDS,
                             uint wavesLimitedByWorkgroup,
                             double occupancy) :
    m_ThreadId(threadId),
    m_KernelName(kernelName),
    m_DeviceName(deviceName),
    m_ComputeUnits(computeUnits),
    m_MaxWavesPerComputeUnit(maxWavesPerComputeUnit),
    m_MaxLDS(maxLDS),
    m_UsedLDS(usedLDS),
    m_WavefrontSize(wavefrontSize),
    m_WorkGroupSize(workGroupSize),
    m_WavesPerWorkGroup(wavesPerWorkGroup),
    m_MaxWorkGroupSize(maxWorkGroupSize),
    m_MaxWavesPerWorkGroup(maxWavesPerWorkGroup),
    m_GlobalWorkSize(globalWorkSize),
    m_MaxGlobalWorkSize(maxGlobalWorkSize),
    m_WavesLimitedByLDS(wavesLimitedByLDS),
    m_WavesLimitedByWorkgroup(wavesLimitedByWorkgroup),
    m_Occupancy(occupancy),
    m_MaxWGPerComputeUnit(maxWGPerComputeUnit),
    m_MaxVGPRs(maxVGPRs),
    m_MaxSGPRs(maxSGPRs),
    m_UsedVGPRs(usedVGPRs),
    m_UsedSGPRs(usedSGPRs),
    m_WavesLimitedByVGPR(wavesLimitedByVGPR),
    m_WavesLimitedBySGPR(wavesLimitedBySGPR)
{

}

OccupancyInfo::OccupancyInfo(const QList<uint>& uintValues, double occupancy, const QString& kernelName, const QString& deviceName)
{
    GT_IF_WITH_ASSERT(uintValues.size() == OccupancyInfo::OCCUPANCY_DATA_fieldsCount)
    {
        m_ThreadId = uintValues[OCCUPANCY_DATA_threadId];
        m_KernelName = kernelName;
        m_DeviceName = deviceName;
        m_Occupancy = occupancy;
        m_ComputeUnits = uintValues[OCCUPANCY_DATA_computeUnits];
        m_MaxWavesPerComputeUnit = uintValues[OCCUPANCY_DATA_maxWavesPerComputeUnit];
        m_MaxLDS = uintValues[OCCUPANCY_DATA_maxLDS];
        m_UsedLDS = uintValues[OCCUPANCY_DATA_usedLDS];
        m_WavefrontSize = uintValues[OCCUPANCY_DATA_wavefrontSize];
        m_WorkGroupSize = uintValues[OCCUPANCY_DATA_workGroupSize];
        m_WavesPerWorkGroup = uintValues[OCCUPANCY_DATA_wavesPerWorkGroup];
        m_MaxWorkGroupSize = uintValues[OCCUPANCY_DATA_maxWorkGroupSize];
        m_MaxWavesPerWorkGroup = uintValues[OCCUPANCY_DATA_maxWavesPerWorkGroup];
        m_GlobalWorkSize = uintValues[OCCUPANCY_DATA_globalWorkSize];
        m_MaxGlobalWorkSize = uintValues[OCCUPANCY_DATA_maxGlobalWorkSize];
        m_WavesLimitedByLDS = uintValues[OCCUPANCY_DATA_wavesLimitedByLDS];
        m_WavesLimitedByWorkgroup = uintValues[OCCUPANCY_DATA_wavesLimitedByWorkgroup];
        m_MaxWGPerComputeUnit = uintValues[OCCUPANCY_DATA_maxWGPerComputeUnit];
        m_MaxVGPRs = uintValues[OCCUPANCY_DATA_maxVGPRs];
        m_MaxSGPRs = uintValues[OCCUPANCY_DATA_maxSGPRs];
        m_UsedVGPRs = uintValues[OCCUPANCY_DATA_usedVGPRs];
        m_UsedSGPRs = uintValues[OCCUPANCY_DATA_usedSGPRs];
        m_WavesLimitedByVGPR = uintValues[OCCUPANCY_DATA_wavesLimitedByVGPR];
        m_WavesLimitedBySGPR = uintValues[OCCUPANCY_DATA_wavesLimitedBySGPR];
    }
}

QString OccupancyInfo::SaveToTempFile(int callIndex)
{
    QString retVal;
    QTemporaryFile tempFile;

    if (tempFile.open())
    {
        tempFile.setTextModeEnabled(true);
        tempFile.setAutoRemove(false);
        retVal = tempFile.fileName();

        QTextStream sw(&tempFile);

        sw << QString("%1=%2\n").arg("ThreadID").arg(m_ThreadId);
        sw << QString("%1=%2\n").arg("CallIndex").arg(callIndex);
        sw << QString("%1=%2\n").arg("KernelName").arg(m_KernelName);
        sw << QString("%1=%2\n").arg("DeviceName").arg(m_DeviceName);
        sw << QString("%1=%2\n").arg("ComputeUnits").arg(m_ComputeUnits);
        sw << QString("%1=%2\n").arg("MaxWavesPerComputeUnit").arg(m_MaxWavesPerComputeUnit);
        sw << QString("%1=%2\n").arg("MaxWorkGroupPerComputeUnit").arg(m_MaxWGPerComputeUnit);
        sw << QString("%1=%2\n").arg("MaxVGPRs").arg(m_MaxVGPRs);
        sw << QString("%1=%2\n").arg("MaxSGPRs").arg(m_MaxSGPRs);
        sw << QString("%1=%2\n").arg("MaxLDS").arg(m_MaxLDS);
        sw << QString("%1=%2\n").arg("UsedVGPRs").arg(m_UsedVGPRs);
        sw << QString("%1=%2\n").arg("UsedSGPRs").arg(m_UsedSGPRs);
        sw << QString("%1=%2\n").arg("UsedLDS").arg(m_UsedLDS);
        sw << QString("%1=%2\n").arg("WavefrontSize").arg(m_WavefrontSize);
        sw << QString("%1=%2\n").arg("WorkGroupSize").arg(m_WorkGroupSize);
        sw << QString("%1=%2\n").arg("WavesPerWorkGroup").arg(m_WavesPerWorkGroup);
        sw << QString("%1=%2\n").arg("MaxWorkGroupSize").arg(m_MaxWorkGroupSize);
        sw << QString("%1=%2\n").arg("MaxWavesPerWorkGroup").arg(m_MaxWavesPerWorkGroup);
        sw << QString("%1=%2\n").arg("GlobalWorkSize").arg(m_GlobalWorkSize);
        sw << QString("%1=%2\n").arg("MaxGlobalWorkSize").arg(m_MaxGlobalWorkSize);
        sw << QString("%1=%2\n").arg("WavesLimitedByVGPR").arg(m_WavesLimitedByVGPR);
        sw << QString("%1=%2\n").arg("WavesLimitedBySGPR").arg(m_WavesLimitedBySGPR);
        sw << QString("%1=%2\n").arg("WavesLimitedByLDS").arg(m_WavesLimitedByLDS);
        sw << QString("%1=%2\n").arg("WavesLimitedByWorkgroup").arg(m_WavesLimitedByWorkgroup);
        sw << QString("%1=%2\n").arg("Occupancy").arg(m_Occupancy);
    }

    return retVal;
}

uint OccupancyInfo::GetThreadId()
{
    return m_ThreadId;
}
