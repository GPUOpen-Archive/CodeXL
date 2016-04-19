//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpCalculateCurlU.cl 
/// 
//==================================================================================

//------------------------------  tpCalculateCurlU.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationVec.h>
#include <AMDTTeapotSmokeSimulationIndex.h>
// ---------------------------------------------------------------------------
// Name:        calculateCurlU
// Description: Take the current velocity vector field and calculate the
//              vorticity vector field
// ---------------------------------------------------------------------------
__kernel void calculateCurlU(
    __global float4* u,                 // velocity (vector)
    __global float4* v,                 // Storage for CurlU (used later for vorticity)
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float3 srcU = u[index].xyz;

    int neighborIndex;

    // Vector field length on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    float3 pXBack = interpEdgeVec(srcU, u[neighborIndex].xyz);
    neighborIndex = getRightIndex(index, coord);
    float3 pXForw = interpEdgeVec(srcU, u[neighborIndex].xyz);

    // Vector field length on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    float3 pYBack = interpEdgeVec(srcU, u[neighborIndex].xyz);
    neighborIndex = getBackIndex(index, coord);
    float3 pYForw = interpEdgeVec(srcU, u[neighborIndex].xyz);

    // Vector field length on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    float3 pZBack = interpEdgeVec(srcU, u[neighborIndex].xyz);
    neighborIndex = getTopIndex(index, coord);
    float3 pZForw = interpEdgeVec(srcU, u[neighborIndex].xyz);

    // Calculate vorticity (curl operator applied to velocity vector field)
    float4 curlU;
    curlU.x = (pYForw.z - pYBack.z) * GRID_INV_SPACING - (pZForw.y - pZBack.y) * GRID_INV_SPACING;
    curlU.y = (pZForw.x - pZBack.x) * GRID_INV_SPACING - (pXForw.z - pXBack.z) * GRID_INV_SPACING;
    curlU.z = (pXForw.y - pXBack.y) * GRID_INV_SPACING - (pYForw.x - pYBack.x) * GRID_INV_SPACING;
    curlU.w = 0.0f;

    v[index] = curlU;
}
