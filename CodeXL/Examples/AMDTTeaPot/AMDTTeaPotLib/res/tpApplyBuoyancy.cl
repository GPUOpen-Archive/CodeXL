//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpApplyBuoyancy.cl 
/// 
//==================================================================================

//------------------------------  tpApplyBuoyancy.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        applyBuoyancy
// Description: Calculate the vertical force created by the combination of
//              smoke particle density (gravity) and temperature (buoyancy).
//              Update the velocity field (z coordinate) for given timestep.
// ---------------------------------------------------------------------------
__kernel void applyBuoyancy(
    __global float* s,      // density (scalar)
    __global float* t,      // temperature (scalar)
    __global float4* u,                 // velocity (vector)
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float density = s[index] * (-p->buoyAlpha);
    float temp = (t[index] - p->ambiantTemperature) * p->buoyBeta;
    u[index].z += ((density + temp) * p->deltaTimeInSeconds);
}