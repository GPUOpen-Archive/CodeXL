//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpCreateDensityTexture.cl 
/// 
//==================================================================================

//------------------------------  tpCreateDensityTexture.cl ----------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

#include <AMDTTeapotSmokeSimulationGen.h>
// ---------------------------------------------------------------------------
// Name:        createDensityTexture
// Description: Setup the density (float RGBA) buffer that will be copied into
//              the OpenGL texture. We set the RGB to (1.0, 1.0, 1.0) and the
//              alpha to the negative exponent of the field density.
// ---------------------------------------------------------------------------
__kernel void createDensityTexture(
    __global float* s,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);

    // Coefficient to control amount of transparency
    float Kd = 1.0f;

    // Compute transparency based on the density value
    // rho=0 => T=1 => Fully transparent.
    float T = exp(-s[index] * Kd);

    // Light intensity at this grid position. For the moment we are
    // not computing this so we assume that all light gets through.
    float4 L = (float4)(1.0f, 1.0f, 1.0f, 1.0f);

    // Setup the color and alpha:
    // in OpenGL 0 = fully transparent, 1 = fully opaque
    float4 C = (float4)(1.0f, 1.0f, 1.0f, 1.0f - T);

    // Apply light intensity at this position.
    C *= L;

    // Output RGBA float values.
    d[index] = C;
}