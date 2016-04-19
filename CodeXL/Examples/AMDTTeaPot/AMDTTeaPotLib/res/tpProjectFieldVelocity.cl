//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpProjectFieldVelocity.cl 
/// 
//==================================================================================

//------------------------------ tpProjectFieldVelocity.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationIndex.h>
// ---------------------------------------------------------------------------
// Name:        projectFieldVelocity
// Description: Now that we have solved the pressure field, we "project" it
//              onto the velocity field. This is the mass-conservation part
//              of the Navier-Stokes fluid equations.
// ---------------------------------------------------------------------------
__kernel void projectFieldVelocity(
    __global float* srcP,
    __global float4* u,
    __global float4* v,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float centerP = srcP[index];
    float4 srcU = u[index];

    // 'staggered' grid style sampling with bilinear sampling
    float forw;
    float back;
    int neighborIndex;
    float4 gradP;

    // Pressure on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    back = (centerP + srcP[neighborIndex]) * 0.5f;
    neighborIndex = getRightIndex(index, coord);
    forw = (centerP + srcP[neighborIndex]) * 0.5f;
    gradP.x = (forw - back) * GRID_INV_SPACING;

    // Pressure on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    back = (centerP + srcP[neighborIndex]) * 0.5f;
    neighborIndex = getBackIndex(index, coord);
    forw = (centerP + srcP[neighborIndex]) * 0.5f;
    gradP.y = (forw - back) * GRID_INV_SPACING;

    // Pressure on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    back = (centerP + srcP[neighborIndex]) * 0.5f;
    neighborIndex = getTopIndex(index, coord);
    forw = (centerP + srcP[neighborIndex]) * 0.5f;
    gradP.z = (forw - back) * GRID_INV_SPACING;

    gradP.w = 0.0f;

    v[index] = srcU - gradP;
}