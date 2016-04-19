//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDebugVelocityVector.cl 
/// 
//==================================================================================

//------------------------------ tpDebugVelocityVector.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        debugVelocityVector
// Description: Set the density texture to show the raw velocity vector field.
//              Red shows movement in x, green shows movement in y and
//              blue shows movement in z and alpha set to the length of the
//              vector. "Movement" is expressed as the distance, in cell
//              units, that a particle will travel at maximum delta-time.
// ---------------------------------------------------------------------------
__kernel void debugVelocityVector(
    __global float4* u,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float3 au = fabs(u[index].xyz) * p->maxDeltaTimeInSeconds * 0.5f;

    d[index] = (float4)(
                   au, length(au)
               );
}