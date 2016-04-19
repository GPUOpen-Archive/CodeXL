//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulationVecField.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulationVecField.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATIONVECFIELD_H
#define __AMDTTEAPOTSMOKESIMULATIONVECFIELD_H

#include <AMDTTeapotSmokeSimulationClamp.h>
// ---------------------------------------------------------------------------
// Name:        triLinearInterpolateVecField
// Description: For arbitrary, non-integer grid coordinate, use tri-linear
//              interpolation to return the vector field at that position.
//              If the coorinate is outside the grid, then the field vector
//              is assumed to be zero.
// ---------------------------------------------------------------------------
float4 triLinearInterpolateVecField(
    __global float4* field,
    float3 coord
)
{
    float3 icoord;
    float3 a = fract(coord, &icoord);
    // i0 is the grid coordinate of the left-forward-bottom cell closest to the given
    // coordinate.
    int3 i0 = clampGridCoord((int3)(icoord.x, icoord.y, icoord.z));
    // i1 is the grid coordinate of the right-back-top cell.
    int3 i1 = clampGridCoord((int3)(floor(coord.x + 1.0f), floor(coord.y + 1.0f), floor(coord.z + 1.0f)));
    // Calculate how much we step from i0 to i1. Generally this will be (1,1,1) unless
    // we are on or outside the grid boundary.
    int3 steps = i1 - i0;
    // Limit will be zero if the coordinate is outside the grid.
    float limit = (float)(abs(steps.x) * abs(steps.y) * abs(steps.z));

    int index = (i0.z << GRID_STRIDE_SHIFT_Z) + (i0.y << GRID_STRIDE_SHIFT_Y) + i0.x;
    float4 A000 = field[index];
    float4 A010 = field[index + steps.x];
    int yindex = index + (steps.y << GRID_STRIDE_SHIFT_Y);
    float4 A001 = field[yindex];
    float4 A011 = field[yindex + steps.x];

    float4 Ax00 = A000 * (1.0f - a.x) + A010 * a.x;
    float4 Ax01 = A001 * (1.0f - a.x) + A011 * a.x;
    float4 Axy0 = Ax00 * (1.0f - a.y) + Ax01 * a.y; // lerp between Ax0 & Ax1

    int zindex = index + (steps.z << GRID_STRIDE_SHIFT_Z);
    float4 A100 = field[zindex];
    float4 A110 = field[zindex + steps.x];
    yindex = zindex + (steps.y << GRID_STRIDE_SHIFT_Y);
    float4 A101 = field[yindex];
    float4 A111 = field[yindex + steps.x];

    float4 Ax10 = A100 * (1.0f - a.x) + A110 * a.x;
    float4 Ax11 = A101 * (1.0f - a.x) + A111 * a.x;
    float4 Axy1 = Ax10 * (1.0f - a.y) + Ax11 * a.y; // lerp between Ax0 & Ax1

    // now finally lerp between Axy0 & Axy1
    return limit * (Axy0 * (1.0f - a.z) + Axy1 * a.z);
}

#endif //__AMDTTEAPOTSMOKESIMULATIONVECFIELD_H

