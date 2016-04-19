//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulation.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulation.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATION_H
#define __AMDTTEAPOTSMOKESIMULATION_H

// This structure is identical to the one used by the host. What about endianness?
// It is used to pass constants to the kernels. Some constants stay the same between
// cycles, others change each cycle. This structure is transferred to the device
// memory once per cycle, accessed by many kernels each cycle.
typedef struct _SmokeSimConstants
{
    float    buoyAlpha;         // Gravity buoyancy constant
    float    buoyBeta;          // Thermal buoyancy constant
    float    vorticity;         // Vorticity constant

    float    KsDens;             // Multiplier for density source
    float    KsTemp;             // Multiplier for temperature source

    // dissipation constants and coefficients
    float    KminDens;
    float    KmaxDens;
    float    KminTemp;
    float    KmaxTemp;

    float    KdrDens;
    float    KdrTemp;
    float    KdissipateDens;     // 1.0f / (1.0f + dTime * KdrDens)
    float    KdissipateTemp;     // 1.0f / (1.0f + dTime * KdrTemp)

    // constants for Jacobi Poisson iteration used to deduce pressure.
    float    KpressureJacobiPoissonAlpha;
    float    KpressureJacobiPoissonInvBeta;

    // Ambient temperature
    float    ambiantTemperature;

    // Delta time (in seconds) for the simulation step
    float    deltaTimeInSeconds;

    // Maximum delta time. If time between cycles is longer than this,
    // the simulation will run with this value.
    float    maxDeltaTimeInSeconds;

    // Smoke source. Smoke leaves the teapot spout. The spout is elliptical,
    // centered at spoutCenter (grid coordinates) and with radius maxXRadius
    // in one direction and maxYRadius in the other dimension. spoutInvExtent
    // shrinks the dimensions around the spoutCenter so that if a grid point
    // (x,y) is inside the spout, then (x - spoutCenter.x) * spoutInvExtent.x <=1
    // and similarly for y. sourceCenter is the center of the source of
    // smoke and sourceVelocity is a vector that expresses the velocity of
    // the smoke leaving the tea spout. These two values are randomized.
    // The smoke density is expressed as a probability distribution centered
    // around the smoke center:
    //
    //   sourceDistributionBeta * exp(sourceDistributionAlpha * radius^2)
    //
    float    sourceDistributionAlpha;
    float    sourceDistributionBeta;
    float2   spoutCenter;        // Center of tea spout in grid coords
    float2   spoutInvExtent;     // (1 / maxXRadius, 1 / maxYRadius)
    float2   sourceCenter;       // Center of maximum smoke density
    float4   sourceVelocity;     // Velocity vector of smoke leaving spout.
} SmokeSimConstants;

/// ---------------------------------------------------------------------------
// Name:        clampGridCoord
// Description: Clamp coord to be inside the grid space.
// ---------------------------------------------------------------------------
int3 clampGridCoord(int3 coord)
{
    return clamp(
               coord,
               (int3)(0, 0, 0),
               (int3)(GRID_NUM_CELLS_X - 1, GRID_NUM_CELLS_Y - 1, GRID_NUM_CELLS_Z - 1));
}

// ---------------------------------------------------------------------------
// Name:        getIndex
// Description: Convert a grid coordinate into memory buffer index.
// ---------------------------------------------------------------------------
int getIndex(const int3 coord)
{
    return
        (coord.z << GRID_STRIDE_SHIFT_Z)
        + (coord.y << GRID_STRIDE_SHIFT_Y)
        + coord.x;
}

// ---------------------------------------------------------------------------
// Name:        getLeftIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell to the left (x-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getLeftIndex(const int index, const int3 coord)
{
    return index - step(1, coord.x);
}

// ---------------------------------------------------------------------------
// Name:        getLeftIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell to the right (x-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getRightIndex(const int index, const int3 coord)
{
    return index + step(coord.x, GRID_NUM_CELLS_X - 2);
}

// ---------------------------------------------------------------------------
// Name:        getFrontIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell in front (y-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getFrontIndex(const int index, const int3 coord)
{
    return index - (((int)step(1, coord.y)) << GRID_STRIDE_SHIFT_Y);
}

// ---------------------------------------------------------------------------
// Name:        getBackIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell behind (y-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getBackIndex(const int index, const int3 coord)
{
    return index + (((int)step(coord.y, GRID_NUM_CELLS_Y - 2)) << GRID_STRIDE_SHIFT_Y);
}

// ---------------------------------------------------------------------------
// Name:        getTopIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell above (z-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getTopIndex(const int index, const int3 coord)
{
    return index + (((int)step(coord.z, GRID_NUM_CELLS_Z - 2)) << GRID_STRIDE_SHIFT_Z);
}

// ---------------------------------------------------------------------------
// Name:        getBottomIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell below (z-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getBottomIndex(const int index, const int3 coord)
{
    return index - (((int)step(1, coord.z)) << GRID_STRIDE_SHIFT_Z);
}

// ---------------------------------------------------------------------------
// Name:        interpEdgeVec
// Description: Interpolate the velocity vector on the cell edge.
// ---------------------------------------------------------------------------
float3 interpEdgeVec(float3 u1, float3 u2)
{
    return (u1 + u2) * 0.5f;
}

// ---------------------------------------------------------------------------
// Name:        interpEdgeVecLength
// Description: Interpolate the velocity (vector length) on the cell edge.
// ---------------------------------------------------------------------------
float interpEdgeVecLength(float3 u1, float3 u2)
{
    return length((u1 + u2) * 0.5f);
}

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

// ---------------------------------------------------------------------------
// Name:        triLinearInterpolateScalarField
// Description: For arbitrary, non-integer grid coordinate, use tri-linear
//              interpolation to return the scalar field at that position.
//              If the coorinate is outside the grid, then the field strength
//              is assumed to be zero.
// ---------------------------------------------------------------------------
float triLinearInterpolateScalarField(
    __global float* field,
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
    float A000 = field[index];
    float A010 = field[index + steps.x];
    int yindex = index + (steps.y << GRID_STRIDE_SHIFT_Y);
    float A001 = field[yindex];
    float A011 = field[yindex + steps.x];

    float Ax00 = A000 * (1.0f - a.x) + A010 * a.x;
    float Ax01 = A001 * (1.0f - a.x) + A011 * a.x;
    float Axy0 = Ax00 * (1.0f - a.y) + Ax01 * a.y;  // lerp between Ax0 & Ax1

    int zindex = index + (steps.z << GRID_STRIDE_SHIFT_Z);
    float A100 = field[zindex];
    float A110 = field[zindex + steps.x];
    yindex = zindex + (steps.y << GRID_STRIDE_SHIFT_Y);
    float A101 = field[yindex];
    float A111 = field[yindex + steps.x];

    float Ax10 = A100 * (1.0f - a.x) + A110 * a.x;
    float Ax11 = A101 * (1.0f - a.x) + A111 * a.x;
    float Axy1 = Ax10 * (1.0f - a.y) + Ax11 * a.y;  // lerp between Ax0 & Ax1

    // now finally lerp between Axy0 & Axy1
    return limit * (Axy0 * (1.0f - a.z) + Axy1 * a.z);
}

#endif //__AMDTTEAPOTSMOKESIMULATION_H

