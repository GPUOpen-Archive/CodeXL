//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDebugTemperature.cl 
/// 
//==================================================================================

//------------------------------ tpDebugTemperature.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------
#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        debugTemperature
// Description: Set the density texture to show the raw temperature field.
// ---------------------------------------------------------------------------
__kernel void debugTemperature(
    __global float* t,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float temp = t[index] - p->KminTemp;
    d[index] = (float4)(temp * 0.01f, 0.0f, (p->KmaxTemp - p->KminTemp - temp) * 0.01f, temp * 0.01f);
}