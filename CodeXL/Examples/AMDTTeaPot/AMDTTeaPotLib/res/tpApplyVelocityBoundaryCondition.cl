//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpApplyVelocityBoundaryCondition.cl 
/// 
//==================================================================================

//------------------------------  tpApplyVelocityBoundaryCondition.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        applyVelocityBoundaryCondition
// Description: This is a generic boundary condition kernel. The host can
//              setup boundary conditions in any way, including for objects
//              inside the grid.
//              Here we are updating the velocity vector field. Most likely
//              the host will use the non-slip edge condition where the
//              velocity of a cell on the edge of the grid is set to the
//              opposite of the velocity vector of the neighboring cell inside
//              the grid, thus ensuring that velocity goes to zero at the edge
//              (smoke in a box).
//              See the section titled "Boundary conditions" at the start
//              of this file for more details.
// ---------------------------------------------------------------------------
__kernel void applyVelocityBoundaryCondition(
    __global int*    srcIndex,
    __global float4* srcScale,
    __global float4*             u)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int dstIndex = getIndex(coord);
    int index = srcIndex[dstIndex];
    float4 scale = srcScale[index];
    float4 value = u[index];
    u[dstIndex] = (float4)(value.x * scale.x, value.y * scale.y, value.z * scale.z, 0.0f);
}
