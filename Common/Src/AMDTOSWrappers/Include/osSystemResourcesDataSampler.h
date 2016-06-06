//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSystemResourcesDataSampler.h
///
//=====================================================================

//------------------------------ osSystemResourcesDataSampler.h ------------------------------

#ifndef __OSSYSTEMRESOURCESDATASAMPLER_H
#define __OSSYSTEMRESOURCESDATASAMPLER_H

// Forward declarations:
struct osCPUSampledData;
struct osPhysicalMemorySampledData;

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          osSystemResourcesDataSampler
// General Description: A base class for all classes that can sample osCPUSampledData
//                      and osPhysicalMemorySampledData structs.
// Author:      AMD Developer Tools Team
// Creation Date:       22/11/2009
// ----------------------------------------------------------------------------------
class OS_API osSystemResourcesDataSampler
{
public:
    osSystemResourcesDataSampler() {};
    virtual ~osSystemResourcesDataSampler() {};

    // CPUs
    virtual bool updateCPUsData() = 0;
    virtual int cpusAmount() const = 0;
    virtual bool getGlobalCPUData(osCPUSampledData& cpuStatistics) const = 0;
    virtual bool getCPUData(int cpuIndex, osCPUSampledData& cpuStatistics) const = 0;

    // Memory
    virtual bool updatePhysicalMemoryData() = 0;
    virtual bool getPhysicalMemoryData(osPhysicalMemorySampledData& memoryStatistics) const = 0;
};

#endif //__OSSYSTEMRESOURCESDATASAMPLER_H

