//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulationGen.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulationGen.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATIONGEN_H
#define __AMDTTEAPOTSMOKESIMULATIONGEN_H

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

#endif //__AMDTTEAPOTSMOKESIMULATIONGEN_H

