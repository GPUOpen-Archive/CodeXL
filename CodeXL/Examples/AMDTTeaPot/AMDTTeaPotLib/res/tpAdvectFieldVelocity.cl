//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpAdvectFieldVelocity.cl 
/// 
//==================================================================================

//------------------------------  tpAdvectFieldVelocity.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationVecField.h>
// ---------------------------------------------------------------------------
// Name:        advectFieldVelocity
// Description: Move the velocity vector field along. Here we use Jos Stam's
//              method where the field at grid position (x, y, z) is found
//              by tracing a particle at this position back in time (dt) to
//              the position that it occupied and copying the velocity vector
//              into the current position. We trace back along the velocity
//              vector at the current position. Since this may not fall on
//              a cell center, we use tri-linear interpolation to compute
//              the velocity vector at the previous position. If the trace
//              goes outside the grid, we assume the velocity is zero.
// ---------------------------------------------------------------------------
__kernel void advectFieldVelocity(
    __global float4* u,
    __global float4* v,
    __constant SmokeSimConstants* p)
{
    int3 src = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(src);
    float4 srcU = u[index];

    // Trace grid position back based on velocity vector
    float3 coord = (float3)(
                       (float)src.x - srcU.x * p->deltaTimeInSeconds,
                       (float)src.y - srcU.y * p->deltaTimeInSeconds,
                       (float)src.z - srcU.z * p->deltaTimeInSeconds
                   );

    // Tri-linear interpolate to get the velocity vector at that position.
    v[index] = triLinearInterpolateVecField(u, coord);
}
