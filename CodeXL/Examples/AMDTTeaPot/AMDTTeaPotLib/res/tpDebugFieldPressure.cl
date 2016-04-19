//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDebugFieldPressure.cl 
/// 
//==================================================================================

//------------------------------ tpDebugFieldPressure.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        debugFieldPressure
// Description: Set the density texture to show the raw pressure field. The
//              alpha value is set to show the velocity change that the
//              pressure field will create at each cell. Saturation means that
//              the pressure will cause a change in velocity by two or more
//              units for the maximum time step.
// ---------------------------------------------------------------------------
__kernel void debugFieldPressure(
    __global float* srcP,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    d[index] = (float4)(1.0f, 1.0f, 1.0f, fabs(srcP[index]) * p->maxDeltaTimeInSeconds * 0.5f);
}