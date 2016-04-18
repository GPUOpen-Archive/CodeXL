//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDebugDensity.cl 
/// 
//==================================================================================

//------------------------------tpDebugDensity.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        debugDensity
// Description: Set the density texture to show the raw density field.
// ---------------------------------------------------------------------------
__kernel void debugDensity(
    __global float* s,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    d[index] = (float4)(1.0f, 1.0f, 1.0f, s[index]);
}