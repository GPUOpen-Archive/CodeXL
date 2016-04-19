//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpComputeFieldPressureIter.cl 
/// 
//==================================================================================

//------------------------------  tpComputeFieldPressureIter.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationIndex.h>
// ---------------------------------------------------------------------------
// Name:        computeFieldPressureIter
// Description: Performs one Jacobi Poisson iteration. This kernel is called
//              for at least 30 iterations with the memory pointers srcP
//              and dstP being swapped between each iteration.
// ---------------------------------------------------------------------------
__kernel void computeFieldPressureIter(
    __global float* divU,
    __global float* srcP,
    __global float* dstP,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);

    // The equation in 2D:
    // We are solving Pij from equation: laplacian(Pij) = b
    // where b = divergence(U), U = vector field
    // laplacian(Pi,j) = (Pi+1,j + Pi-1,j + Pi,j+1, Pi,j-1 - 4*Pi,j) / (dx^2)
    // => Pi,j = (Pi+1,j + Pi-1,j + Pi,j+1, Pi,j-1 - laplacian(Pij)*(dx^2)) / 4
    // => Pi,j = (Pi+1,j + Pi-1,j + Pi,j+1, Pi,j-1 - bi,j*(dx^2)
    //
    // In 2D, alpha = -(dx^2) and rBeta = 1/4
    // In 3D, alpha = -(dx^2) and rBeta = 1/6
    //
    // NOTE: this equation assumes dx = dy = dz, i.e that cell spacing is
    // the same in all directions.
    //
    // alpha and rBeta are passed in the SmokeSimConstants as
    // KpressureJacobiPoissonAlpha and KpressureJacobiPoissonInvBeta
    // respectively.

    int neighborIndex;

    // Pressure to left and right
    neighborIndex = getLeftIndex(index, coord);
    float qXBack = srcP[neighborIndex];
    neighborIndex = getRightIndex(index, coord);
    float qXForw = srcP[neighborIndex];

    // Vector field on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    float qYBack = srcP[neighborIndex];
    neighborIndex = getBackIndex(index, coord);
    float qYForw = srcP[neighborIndex];

    // Vector field on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    float qZBack = srcP[neighborIndex];
    neighborIndex = getTopIndex(index, coord);
    float qZForw = srcP[neighborIndex];

    // b sample from center
    float bAlpha = divU[index] * p->KpressureJacobiPoissonAlpha;

    // evaluate jacobi iteration, which computer the next value for q
    dstP[index] = (qXBack + qXForw + qYBack + qYForw + qZBack + qZForw + bAlpha) * p->KpressureJacobiPoissonInvBeta;
}