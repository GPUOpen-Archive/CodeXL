//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDissipateDensity.cl 
/// 
//==================================================================================

//------------------------------ tpDissipateDensity.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        dissipateDensity
// Description: Reduce density at every grid position according to rate
//              factor KdissipateDens. Apply minimum density constraint.
// ---------------------------------------------------------------------------
__kernel void dissipateDensity(
    __global  float* s0,
    __global float* s1,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    s1[index] = max(p->KminDens, s0[index] * p->KdissipateDens);
}