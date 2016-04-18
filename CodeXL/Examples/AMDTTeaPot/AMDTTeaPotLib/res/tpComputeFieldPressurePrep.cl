//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpComputeFieldPressurePrep.cl 
/// 
//==================================================================================

//------------------------------  tpComputeFieldPressurePrep.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationIndex.h>
// ---------------------------------------------------------------------------
// Name:        computeFieldPressurePrep
// Description: Now that we have a new velocity vector field, we need to
//              compute the pressure at each point in the grid. This will
//              be done using Jacobi Poisson iteration based on the divergence
//              of the current velocity field. Here we perform the preparation
//              for the solver - set initial conditions (pressure = 0
//              everywhere) and compute the divergence of the velocity field.
// ---------------------------------------------------------------------------
__kernel void computeFieldPressurePrep(
    __global float4* u,
    __global float* divU,
    __global float* srcP,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float4 srcU = u[index];

    // First calculate the divergence using 'staggered grid' method by sampling
    // finite diffrences between q(i+0.5) - q(i-0.5) and dividing by difference
    // deltaX which is 'unbiased'
    float forw;
    float back;
    int neighborIndex;

    // Vector field on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    back = (srcU.x + u[neighborIndex].x) * 0.5f;
    neighborIndex = getRightIndex(index, coord);
    forw = (srcU.x + u[neighborIndex].x) * 0.5f;
    float ddx = (forw - back) * GRID_INV_SPACING;

    // Vector field on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    back = (srcU.y + u[neighborIndex].y) * 0.5f;
    neighborIndex = getBackIndex(index, coord);
    forw = (srcU.y + u[neighborIndex].y) * 0.5f;
    float ddy = (forw - back) * GRID_INV_SPACING;

    // Vector field on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    back = (srcU.z + u[neighborIndex].z) * 0.5f;
    neighborIndex = getTopIndex(index, coord);
    forw = (srcU.z + u[neighborIndex].z) * 0.5f;
    float ddz = (forw - back) * GRID_INV_SPACING;

    divU[index] = ddx + ddy + ddz;
    srcP[index] = 0.0f;
}