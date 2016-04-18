//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSlowMotionDelay.cpp
///
//==================================================================================

//------------------------------ suSlowMotionDelay.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTServerUtilities/Include/suSlowMotionDelay.h>

// ---------------------------------------------------------------------------
// Name:        suPerformSlowMotionDelay
// Description: Perform the "Slow motion" functionality delay.
// Author:      Yaki Tebeka
// Date:        28/8/2007
// Implementation Notes:
//   a. The delay is performed using CPU iterations (we tried to use Sleep, but
//      its intervals (milliseconds) are too long for our requited usage).
//   b. This functionality is put in a different file, that is compiled without
//      compiler optimizations (the compiler optimizer sometimes removes iterations
//      that does not seem to do have any side effects; like our slow down code).
// ---------------------------------------------------------------------------
void suPerformSlowMotionDelay(unsigned long slowMotionDelayUnits)
{
    // Slow the program by using CPU iterations:
    int slowMotionAidVariable = 0;

    unsigned long iterationsAmount = slowMotionDelayUnits * 1000;

    for (unsigned long i = 0; i < iterationsAmount; i++)
    {
        slowMotionAidVariable = slowMotionAidVariable + i + 1;
        slowMotionAidVariable = slowMotionAidVariable - i - 3;
        slowMotionAidVariable = slowMotionAidVariable + 2;
    }
}

