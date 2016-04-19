//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDissipateTemperature.cl 
/// 
//==================================================================================

//------------------------------ tpDissipateTemperature.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        dissipateTemperature
// Description: Reduce temperature at every grid position according to rate
//              factor KdissipateTemp. Apply minimum temperature constraint.
// ---------------------------------------------------------------------------
__kernel void dissipateTemperature(
    __global  float* t0,
    __global float* t1,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    t1[index] = max(p->KminTemp, t0[index] * p->KdissipateTemp);
}