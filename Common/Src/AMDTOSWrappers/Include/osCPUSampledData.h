//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCPUSampledData.h
///
//=====================================================================

//------------------------------ osCPUSampledData.h ------------------------------

#ifndef __OSCPUSAMPLEDDATA_H
#define __OSCPUSAMPLEDDATA_H

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ----------------------------------------------------------------------------------
// Class Name:           osCPUSampledData
//
// General Description:
//   Holds a single CPU's sampled data.
//   The data is measured in units of USER_HZ (1/100ths of a second on most architectures).
//
// Author:      AMD Developer Tools Team
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
struct osCPUSampledData
{
public:
    // Number of clock ticks the system spent in user mode:
    gtUInt64 _userClockTicks;

    // Number of clock ticks the system spent in user mode (nice):
    gtUInt64 _niceClockTicks;

    // Number of clock ticks the system spent in system mode:
    gtUInt64 _sysClockTicks;

    // Number of clock ticks the system spent in the idle task:
    gtUInt64 _idleClockTicks;

    // Number of clock ticks the system spent waiting for I/O to complete:
    gtUInt64 _IOWaitClockTicks;

    // Number of clock ticks the system spent servicing interrupts:
    gtUInt64 _IRQClockTicks;

    // Number of clock ticks the system spent servicing software interrupts:
    gtUInt64 _softIRQClockTicks;

    // The total number of clock ticks elapsed since system boot:
    gtUInt64 _totalClockTicks;

public:
    osCPUSampledData() : _userClockTicks(0), _niceClockTicks(0), _sysClockTicks(0), _idleClockTicks(0),
        _IOWaitClockTicks(0), _IRQClockTicks(0), _softIRQClockTicks(0), _totalClockTicks(0) {};
};


#endif //__OSCPUSAMPLEDDATA_H

