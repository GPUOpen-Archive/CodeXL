//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OccupancyInfo.h $
/// \version $Revision: #6 $
/// \brief :  This file contains OccupancyInfo class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OccupancyInfo.h#6 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _OCCUPANCYINFO_H_
#define _OCCUPANCYINFO_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qtextstream.h>

class OccupancyInfo;
typedef QMap<uint, QList<OccupancyInfo*> > OccupancyTable;

/// Class that contains occupancy information for a OpenCL kernel dispatch. Also provides the ability to save to a temp file that can be passed to CodeXLGpuProfiler to generate the Occupancy HTML file.
class OccupancyInfo
{
public:

    enum OccuppancyDataField
    {
        // uint
        OCCUPANCY_DATA_threadId,
        // string
        OCCUPANCY_DATA_kernelName,
        OCCUPANCY_DATA_deviceName,
        // uint
        OCCUPANCY_DATA_computeUnits,
        OCCUPANCY_DATA_maxWavesPerComputeUnit,
        OCCUPANCY_DATA_maxWGPerComputeUnit,
        OCCUPANCY_DATA_maxVGPRs,
        OCCUPANCY_DATA_maxSGPRs,
        OCCUPANCY_DATA_maxLDS,
        OCCUPANCY_DATA_usedVGPRs,
        OCCUPANCY_DATA_usedSGPRs,
        OCCUPANCY_DATA_usedLDS,
        OCCUPANCY_DATA_wavefrontSize,
        OCCUPANCY_DATA_workGroupSize,
        OCCUPANCY_DATA_wavesPerWorkGroup,
        OCCUPANCY_DATA_maxWorkGroupSize,
        OCCUPANCY_DATA_maxWavesPerWorkGroup,
        OCCUPANCY_DATA_globalWorkSize,
        OCCUPANCY_DATA_maxGlobalWorkSize,
        OCCUPANCY_DATA_wavesLimitedByVGPR,
        OCCUPANCY_DATA_wavesLimitedBySGPR,
        OCCUPANCY_DATA_wavesLimitedByLDS,
        OCCUPANCY_DATA_wavesLimitedByWorkgroup,
        OCCUPANCY_DATA_occupancy,
        OCCUPANCY_DATA_fieldsCount
    };

    /// Initializes a new instance of the OccupancyInfo class
    /// \param threadId the thread id of this kernel dispatch
    /// \param kernelName the kernel name of this kernel dispatch
    /// \param deviceName the device name of this kernel dispatch
    /// \param computeUnits the number of compute units of this kernel dispatch
    /// \param maxWavesPerComputeUnit max waves per compute unit for the compute unit the kernel executed on
    /// \param maxWGPerComputeUnit max workgroups per compute unit for the compute unit the kernel executed on
    /// \param maxVGPRs max number of vector GPRs for the compute unit the kernel executed on
    /// \param maxSGPRs max number of scalar GPRs for the compute unit the kernel executed on
    /// \param maxLDS max amount of LDS for the compute unit the kernel executed on
    /// \param usedVGPRs number of vector GPRs used by the kernel
    /// \param usedSGPRs number of scalar GPRs used by the kernel
    /// \param usedLDS amount of LDS used by the kernel
    /// \param wavefrontSize the wavefront size for the compute unit the kernel executed on
    /// \param workGroupSize the workgroup size of this kernel dispatch
    /// \param wavesPerWorkGroup the number of waves per workgroup of this kernel dispatch
    /// \param maxWorkGroupSize the max workgroup size for the compute unit the kernel executed on
    /// \param maxWavesPerWorkGroup the max waves per workgroup for the compute unit the kernel executed on
    /// \param globalWorkSize the global work size of this kernel dispatch
    /// \param maxGlobalWorkSize the max global work size for the compute unit the kernel executed on
    /// \param wavesLimitedByVGPR the max waves of this kernel dispatch if vector GPR usage was the only limiting factor
    /// \param wavesLimitedBySGPR the max waves of this kernel dispatch if scalar GPR usage was the only limiting factor
    /// \param wavesLimitedByLDS the max waves of this kernel dispatch if LDS usage was the only limiting factor
    /// \param wavesLimitedByWorkgroup the max waves of this kernel dispatch if Workgroup size was the only limiting factor
    /// \param occupancy the occupancy percentage of this kernel dispatch
    /// Initializes a new instance of the OccupancyInfo class
    OccupancyInfo(uint threadId,
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
                  double occupancy);

    /// Initializes a new instance of the OccupancyInfo class
    /// \param uintValues contain all the uint values as define in the OccuppancyDataField enumeration
    /// \param kernelName the kernel name of this kernel dispatch
    /// \param deviceName the device name of this kernel dispatch
    /// \param occupancy the occupancy percentage of this kernel dispatch
    OccupancyInfo(const QList<uint>& uintValues, double occupancy, const QString& kernelName, const QString& deviceName);


    /// Saves the kernel occupancy info to a temp file in a name=value format
    /// \param "callIndex" the index of the call this is being saved for
    /// \return the temp filename
    QString SaveToTempFile(int callIndex);

    /// To get occupancy.
    double GetOccupancy() const { return m_Occupancy; }

    /// To get kernel name.
    QString GetKernelName() const { return m_KernelName; }

    /// Gets the device name
    QString GetDeviceName() const { return m_DeviceName; }

    /// To get thread Id
    uint GetThreadId();

private:

    /// the thread id of this kernel dispatch
    uint m_ThreadId;

    /// the kernel name of this kernel dispatch
    QString m_KernelName;

    /// the device name of this kernel dispatch
    QString m_DeviceName;

    /// the number of compute units for this kernel dispatch
    uint m_ComputeUnits;

    /// the max waves per compute unit for the compute unit the kernel executed on
    uint m_MaxWavesPerComputeUnit;

    /// the max amount of LDS for the compute unit the kernel executed on
    uint m_MaxLDS;

    /// the amount of LDS used by the kernel
    uint m_UsedLDS;

    /// the wavefront size for the compute unit the kernel executed on
    uint m_WavefrontSize;

    /// the workgroup size of this kernel dispatch
    uint m_WorkGroupSize;

    /// the number of waves per workgroup of this kernel dispatch
    uint m_WavesPerWorkGroup;

    /// the max workgroup size for the compute unit the kernel executed on
    uint m_MaxWorkGroupSize;

    /// the max waves per workgroup for the compute unit the kernel executed on
    uint m_MaxWavesPerWorkGroup;

    /// the global work size of this kernel dispatch
    uint m_GlobalWorkSize;

    /// the max global work size for the compute unit the kernel executed on
    uint m_MaxGlobalWorkSize;

    /// the max waves of this kernel dispatch if LDS usage was the only limiting factor
    uint m_WavesLimitedByLDS;

    /// the max waves of this kernel dispatch if Workgroup size was the only limiting factor
    uint m_WavesLimitedByWorkgroup;

    /// the occupancy percentage of this kernel dispatch
    double m_Occupancy;

    /// the max work group per compute unit for the compute unit the kernel executed on
    uint m_MaxWGPerComputeUnit;

    /// the max vector GPRs for the compute unit the kernel executed on
    uint m_MaxVGPRs;

    /// the max scalar GPRs for the compute unit the kernel executed on
    uint m_MaxSGPRs;

    /// the used vector GPRs for the compute unit the kernel executed on
    uint m_UsedVGPRs;

    /// the used scalar GPRs for the compute unit the kernel executed on
    uint m_UsedSGPRs;

    /// the max waves of this kernel dispatch if vector GPR usage was the only limiting factor
    uint m_WavesLimitedByVGPR;

    /// the max waves of this kernel dispatch if scalar GPR usage was the only limiting factor
    uint m_WavesLimitedBySGPR;

};


#endif // _OCCUPANCYINFO_H_

