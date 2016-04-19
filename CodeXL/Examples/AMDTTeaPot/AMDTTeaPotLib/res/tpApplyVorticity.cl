//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpApplyVorticity.cl 
/// 
//==================================================================================

//------------------------------  tpApplyVorticity.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
#include <AMDTTeapotSmokeSimulationVecLength.h>
#include <AMDTTeapotSmokeSimulationIndex.h>
// ---------------------------------------------------------------------------
// Name:        applyVorticity
// Description: Using the vorticity vector field, calculate the force that it
//              will apply and update the velocity field for the given
//              timestep.
// ---------------------------------------------------------------------------
__kernel void applyVorticity(
    __global float4* u,         // This will be updated
    __global float4* curlU,    // Contains curlU
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);

    // Read in the vorticity vector at this location
    float3 ccU = curlU[index].xyz;

    float3 Neu;
    float forw;
    float back;
    int neighborIndex;

    // a. Calculate divergence operator of vorticity field lengths:

    // Vector field length on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    back = interpEdgeVecLength(ccU, curlU[neighborIndex].xyz);
    neighborIndex = getRightIndex(index, coord);
    forw = interpEdgeVecLength(ccU, curlU[neighborIndex].xyz);
    Neu.x = (forw - back) * GRID_INV_SPACING;

    // Vector field length on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    back = interpEdgeVecLength(ccU, curlU[neighborIndex].xyz);
    neighborIndex = getBackIndex(index, coord);
    forw = interpEdgeVecLength(ccU, curlU[neighborIndex].xyz);
    Neu.y = (forw - back) * GRID_INV_SPACING;

    // Vector field length on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    back = interpEdgeVecLength(ccU, curlU[neighborIndex].xyz);
    neighborIndex = getTopIndex(index, coord);
    forw = interpEdgeVecLength(ccU, curlU[neighborIndex].xyz);
    Neu.z = (forw - back) * GRID_INV_SPACING;

    // b. Sai = normalize(Neu)
    float3 Sai = normalize(Neu);

    // c. Fconf = Kvort * dx * (Sai X Omega)
    float4 vort = (float4)(cross(Sai, ccU) * p->vorticity,  0.0f);

    // Update velocity force to velocity vector field for given timestep.
    u[index] += (vort * p->deltaTimeInSeconds);
}
