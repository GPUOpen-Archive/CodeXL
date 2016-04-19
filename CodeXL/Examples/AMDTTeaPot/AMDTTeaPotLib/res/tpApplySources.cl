//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpApplySources.cl 
/// 
//==================================================================================

//------------------------------  tpApplySources.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        applySources
// Description: Update the density, temperature and velocity field with new
//              smoke leaving the teapot. This kernel is only run over one
//              height slice (the second one from the bottom) which intersects
//              the teapot spout.
//              Density and temperature are increased using a standard
//              probability distribution centered around an exit point and
//              limited to be inside the teaspout. The exit point is
//              randomized by the host.
//              The velocity vector in the region of smoke is set according
//              to values given by the host (generally movement up with
//              a side-wise movement that points from the center of the
//              teapot spout to the exit point.
// ---------------------------------------------------------------------------
__kernel void applySources(
    __global float* s,                  // density (scalar)
    __global float* t,                  // temperature (scalar)
    __global float4* u,                 // velocity (vector)
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2) + 1);
    int index = getIndex(coord);
    float2 pos = (float2)((float)coord.x, (float)coord.y);

    // Set limit to 1.0 if this grid point is inside the teaspout, 0.0 otherwise.
    float2 normPos = pos - p->spoutCenter;
    normPos.x *= p->spoutInvExtent.x;
    normPos.y *= p->spoutInvExtent.y;
    float limit = step(dot(normPos, normPos), 1.0f);

    // Calculate the intensity of smoke as a probability distribution centered around
    // the exit point defined by p->sourceCenter. Anything outide the teapot spout
    // is cut off.
    normPos = pos - p->sourceCenter;
    normPos.x *= p->spoutInvExtent.x;
    normPos.y *= p->spoutInvExtent.y;
    float intensity = limit * p->sourceDistributionBeta * exp(p->sourceDistributionAlpha * dot(normPos, normPos));

    // Increase density and temperature by calculated intensity at this point.
    s[index] += (p->KsDens * p->deltaTimeInSeconds * intensity);
    t[index] = clamp(t[index] + p->KsTemp * p->deltaTimeInSeconds * intensity, p->KminTemp, p->KmaxTemp);

    // Set velocity vector everywhere inside the teapot spout.
    u[index] = (p->sourceVelocity * limit);
}