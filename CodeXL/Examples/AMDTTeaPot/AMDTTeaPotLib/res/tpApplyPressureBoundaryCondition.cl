//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpApplyPressureBoundaryCondition.cl 
/// 
//==================================================================================

//------------------------------  tpApplyPressureBoundaryCondition.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        applyPressureBoundaryCondition
// Description: This is a generic boundary condition kernel. The host can
//              setup boundary conditions in any way, including for objects
//              inside the grid.
//              Here we update the pressure field. Most like the host will
//              set the pressure of cells on the edge to be the same as
//              the pressure neighboring cells, thus ensuring that there
//              is not force pulling smoke out of the grid.
//              See the section titled "Boundary conditions" at the start
//              of this file for more details.
// ---------------------------------------------------------------------------
__kernel void applyPressureBoundaryCondition(
    __global int*    srcIndex,
    __global float4* srcScale,
    __global float*              pressure)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int dstIndex = getIndex(coord);
    int index = srcIndex[dstIndex];
    float4 scale = srcScale[index];
    float value = pressure[index];
    pressure[dstIndex] = value * scale.w;
}