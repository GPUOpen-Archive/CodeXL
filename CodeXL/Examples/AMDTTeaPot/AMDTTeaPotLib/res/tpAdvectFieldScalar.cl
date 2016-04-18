//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpAdvectFieldScalar.cl 
/// 
//==================================================================================

//------------------------------  tpAdvectFieldScalar.cl ----------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationScalarField.h>
// ---------------------------------------------------------------------------
// Name:        advectFieldScalar
// Description: This kernel advects scalar fields (density, temperature) based
//              on the velocity vector field. Physically, this is the movement
//              of smoke particles through the vector field.
//              Here we use Jos Stam's method where the field at grid position
//              (x, y, z) is found by tracing a particle at this position back
//              in time (dt) along the velocity vector at this position to
//              the position that it occupied then and copy the value of the
//              the field at that previous position to the current position.
//              Since the previous position may not fall on a cell center,
//              we use tri-linear interpolation to compute the field
//              position. If the trace goes outside the grid, we assume
//              that the field is zero. The following steps (dissipation)
//              will force minimum values of the field at every cell position
//              thus it is acceptible to set a value of zero for both density
//              and temperature.
// ---------------------------------------------------------------------------
__kernel void advectFieldScalar(
    __global  float4* u,
    __global  float* s0,
    __global float* s1,
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
    s1[index] = triLinearInterpolateScalarField(s0, coord);
}