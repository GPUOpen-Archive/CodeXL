//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDebugVelocityLength.cl 
/// 
//==================================================================================

//------------------------------ tpDebugVelocityLength.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------
#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        debugVelocityLength
// Description: Set the density texture to show the distance, in cell units,
//              that a point will move in one time step (maximum delta-time).
//              This is set in the alpha value - saturation means that the
//              point will move two grid cells or more.
// ---------------------------------------------------------------------------
__kernel void debugVelocityLength(
    __global float4* u,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    d[index] = (float4)(1.0f, 1.0f, 1.0f, length(u[index].xyz) * p->maxDeltaTimeInSeconds * 0.5f);
}