//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPhysicalMemorySampledData.h
///
//=====================================================================

//------------------------------ osPhysicalMemorySampledData.h ------------------------------

#ifndef __OSPHYSICALMEMORYSAMPLEDDATA_H
#define __OSPHYSICALMEMORYSAMPLEDDATA_H

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ----------------------------------------------------------------------------------
// Class Name:           osPhysicalMemorySampledData
//
// General Description:
//   Holds a single CPU's sampled data.
//   The data is measured in units of USER_HZ (1/100ths of a second on most architectures).
//
// Author:      AMD Developer Tools Team
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
struct osPhysicalMemorySampledData
{
public:
    // Total amount of physical memory:
    gtUInt64 _totalPhysicalMemory;

    // Amount of free physical memory:
    gtUInt64 _freePhysicalMemory;

    // Amount of used physical memory:
    gtUInt64 _usedPhysicalMemory;

    // Amount of shared memory:
    gtUInt64 _totalSharedMemory;

    // Amount of pages wired down:
    gtUInt64 _wiredPages;

    // Amount of active pages:
    gtUInt64 _activePages;

    // Amount of inactive pages:
    gtUInt64 _inactivePages;

    // Amount pageins:
    gtUInt64 _pageIns;

    // Amount pageouts:
    gtUInt64 _pageOuts;

    // Amount of memory used for implementing system buffers:
    gtUInt64 _bufferMemory;

    // Amount of system cached memory:
    gtUInt64 _cachedMemory;

    // Total amount of system swap (page) memory:
    gtUInt64 _totalSwapMemory;

    // Amount of available swap (page) memory:
    gtUInt64 _freeSwapMemory;

    // Total amount of system virtual memory:
    gtUInt64 _totalVirtualMemory;

    // Amount of available system virtual memory:
    gtUInt64 _freeVirtualMemory;

    unsigned int _hugePageSize;

public:
    osPhysicalMemorySampledData() : _totalPhysicalMemory(0), _freePhysicalMemory(0), _usedPhysicalMemory(0),
        _totalSharedMemory(0), _bufferMemory(0), _cachedMemory(0), _totalSwapMemory(0),
        _freeSwapMemory(0), _totalVirtualMemory(0), _freeVirtualMemory(0), _hugePageSize(0) {};
};


#endif //__OSPHYSICALMEMORYSAMPLEDDATA_H

