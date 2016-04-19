//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpSmokeSimulation.cl 
/// 
//==================================================================================

//------------------------------ tpSmokeSimulation.cl ------------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------

/**********************************************************************************
 *
 * tpSmokeSimulation
 * =================
 * These kernels carry out an OpenCL implementation of the Navier-stokes equations
 * on a 3D grid using the numerical method described by Jos Stam in his white
 * paper titled, "Real-time Fluid Dynamics for Games".
 *
 * Velocity field grid
 * -------------------
 * The velocity field is stored in an OpenCL memory buffer where the velocity of
 * a cell is stored as a float4. z-axis is the velocity in the vertical direction.
 * w is not used. The velocity units are normalized to grid coordinates. This means
 * that a point with velocity v will travel |v| * dt grid units after dt (delta time).
 *
 * Scalar field grids
 * ------------------
 * The scalar fields (density, temperature) are stored in OpenCL memory buffers
 * using a single float per cell. Temperature values are in degrees centrigrate
 * and density of 1.0 is set to indicate maximum density of smoke particles for
 * a cell.
 *
 * Smoke source
 * ------------
 * The first step of a simulation cycle is to add the temperature and density
 * sources to the existing temperature and density grids in memory. Since we
 * only introduce smoke from one location (the bottom slice - one up from the
 * bottom slice due to boundary conditions that prevent smoke on the edges
 * from propagating into the grid) we run the source kernel over this slice
 * and not over the entire grid. The smoke source location around the spout
 * and velocity is randomized by the host and stored as parameters in the
 * constants that are given to the kernel.
 *
 * Boundary conditions
 * -------------------
 * We use the same boundary conditions as defined by Jos Stam, namely that the
 * velocity vector on the boundary is the opposite of the vector in the
 * neighboring cell and pressure on the boundary is the same as the neighboring
 * cell. Here we provide a generic method for defining boundary conditions
 * that could be used in the future to place objects inside the smoke grid.
 *
 * Boundary conditions are generalized using two OpenCL memory buffers
 * constructed by the host. One of the buffers provides a single int per cell
 * that defines, for that cell, the index of the cell where it will take the
 * value - the source cell. Most cells will take their value from the same cell,
 * but boundary cells will take their value from a neighboring cell.
 *
 * The second memory buffer consists of a float4 for each grid cell. The x,y,z
 * components give the multiplier for each component of the source cell's
 * velocity. The w component gives the muliplier for the source pressure
 * value. For standard boundary conditions, x,y,z = -1.0f and x = 1.0f
 * where the source value is taken from a neighboring cell. All other cells
 * have x,y,z,w = 1.0f and the source cell is the same cell, thus leaving
 * the velocity and pressure unchanged.
 *
 * Iterations
 * ----------
 * We do not take a single kernel approach since many parts of the solution
 * involve performing the vector of spatial partial derivatives over a grid.
 * This means the result of a given cell at time T + dT will depend on the
 * value of neighboring grid cells at time T. Thus we usually have two
 * memory buffers for the fields, one being the source values at time T
 * and the other holding the new values at time T + dT. Then the new
 * new buffer becomes the source buffer in the next iteration (swapping
 * the buffers). This iteration happens many times in the simulation of
 * one time step, hence we run many kernels one are the other. For example,
 * we have two buffers for the velocity field, U0 and U1. One kernel will
 * advect the velocity field, thus the source is U0 and the destination is
 * U1. Then we need to use the new velocity field to advect density, so the
 * kernel that advects density will receive U1 as input.
 *
 * Memory objects
 * --------------
 * The following grid memory buffers need to be created:
 *
 *     S        - Density scalar field (float / cell).
 *     T        - Temperature scalar field (float / cell).
 *     U0       - Velocity field (float4 / cell).
 *     U1       - Velocity field (float4 / cell).
 *     P0       - Pressure field (float / cell)
 *     P1       - Pressure field (float / cell)
 *     B_IDX    - Boundary source index (int / cell)
 *     B_SCALE  - Boundary scaling (float4 / cell)
 *     TMP      - Temporary scalar field (float / cell)
 *     OUT      - Output rgba density (float4 / cell)
 *
 * The following additional buffers are created:
 *
 *     C        - Used to hold the simulation parameters.
 *
 * Sequence of kernel execution
 * ----------------------------
 * All kernels are executed with the following global 3D size array:
 *
 *     (num cells x, num cells y, num cells z)
 *
 * The one exception is the "applySources()" kernel which only runs over
 * one height slice (num cells x, num cells y, 1).
 *
 * Here is the sequence of kernels that are run for one simulation step
 * with the memory object arguments shown. You can find more details
 * in the function descriptions below.
 *
 *        applySources(S, T, U0, C)
 *        applyBuoyancy(S, T, U0, C)
 *        calculateCurlU(U0, U1, C)
 *        applyVorticity(U0, U1, C)
 *        advectFieldVelocity(U0, U1, C)
 *        applyVelocityBoundaryCondition(B_IDX, B_SCALE, U1)
 *        computeFieldPressurePrep(U1, TMP, P0, C)
 *    ->  computeFieldPressureIter(TMP, P0, P1, C)
 *    |
 *    | x20 (notice the swapping of P0 and P1 arguments)
 *    |
 *    --  computeFieldPressureIter(TMP, P1, P0, C)
 *        applyPressureBoundaryCondition(B_IDX, B_SCALE, P0)
 *        projectFieldVelocity(P0, U1, U0, C)
 *        advectFieldScalar(U0, S, TMP)
 *        dissipateDensity(TMP, S, C)
 *        advectFieldScalar(U0, T, TMP)
 *        dissipateTemperature(TMP, T, C)
 *        createDensityTexture(S, OUT, C)
 *
 *
 * OpenGL density texture
 * ----------------------
 * The final step of a simulation cycle is to output the density texture
 * that can be used for rendering. Unfortunately, OpenCL doesn't provide
 * 3D image writing built-ins so we would have to fill in the density
 * texture by breaking the problem into 2D slices and running the kernel
 * for each slice in the grid. Instead, we create a OpenCL float4
 * buffer that will hold the density texture values (rgb and alpha)
 * and then enqueue a copy of the buffer to the OpenGL 3D texture.
 * Each pixel of the texture has alpha set based on the smoke density
 * and rgb is affected by the ambient light source color.
 *
 * Compiling
 * ---------
 * The following macros need to be defined:
 *
 *     GRID_NUM_CELLS_X     - Width in cells
 *     GRID_NUM_CELLS_Y     - Depth in cells
 *     GRID_NUM_CELLS_Z     - Height in cells
 *     GRID_SHIFT_X         - Width as power of 2
 *     GRID_SHIFT_Y         - Depth as power of 2
 *     GRID_SHIFT_Z         - Height as power of 2
 *     GRID_SRIDE_Y         - Stride to move to next depth
 *     GRID_STRIDE_SHIFT_Y  - Depth stride as power of 2
 *     GRID_SRIDE_Z         - Stride to move to next height
 *     GRID_STRIDE_SHIFT_Y  - Height stride as power of 2
 *     GRID_SPACING         - Spacing  between cells (meters)
 *     GRID_INV_SPACING     - 1 / GRID_SPACING
 *
 **********************************************************************************/


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
    return index - step(1.0f, (float) coord.x);
}

// ---------------------------------------------------------------------------
// Name:        getLeftIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell to the right (x-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getRightIndex(const int index, const int3 coord)
{
    return index + step((float) coord.x, (float) GRID_NUM_CELLS_X - 2);
}

// ---------------------------------------------------------------------------
// Name:        getFrontIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell in front (y-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getFrontIndex(const int index, const int3 coord)
{
    return index - (((int)step(1.0f, (float) coord.y)) << GRID_STRIDE_SHIFT_Y);
}

// ---------------------------------------------------------------------------
// Name:        getBackIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell behind (y-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getBackIndex(const int index, const int3 coord)
{
    return index + (((int)step((float)coord.y, GRID_NUM_CELLS_Y - 2.0f)) << GRID_STRIDE_SHIFT_Y);
}

// ---------------------------------------------------------------------------
// Name:        getTopIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell above (z-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getTopIndex(const int index, const int3 coord)
{
    return index + (((int)step((float) coord.z, GRID_NUM_CELLS_Z - 2.0f)) << GRID_STRIDE_SHIFT_Z);
}

// ---------------------------------------------------------------------------
// Name:        getBottomIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell below (z-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getBottomIndex(const int index, const int3 coord)
{
    return index - (((int)step(1.0f, (float) coord.z)) << GRID_STRIDE_SHIFT_Z);
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
    __global const float4* field,
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
    float4 Axy0 = Ax00 * (1.0f - a.y) + Ax01 * a.y;      // lerp between Ax0 & Ax1

    int zindex = index + (steps.z << GRID_STRIDE_SHIFT_Z);
    float4 A100 = field[zindex];
    float4 A110 = field[zindex + steps.x];
    yindex = zindex + (steps.y << GRID_STRIDE_SHIFT_Y);
    float4 A101 = field[yindex];
    float4 A111 = field[yindex + steps.x];

    float4 Ax10 = A100 * (1.0f - a.x) + A110 * a.x;
    float4 Ax11 = A101 * (1.0f - a.x) + A111 * a.x;
    float4 Axy1 = Ax10 * (1.0f - a.y) + Ax11 * a.y;      // lerp between Ax0 & Ax1

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
    __global const float* field,
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
    float Axy0 = Ax00 * (1.0f - a.y) + Ax01 * a.y;   // lerp between Ax0 & Ax1

    int zindex = index + (steps.z << GRID_STRIDE_SHIFT_Z);
    float A100 = field[zindex];
    float A110 = field[zindex + steps.x];
    yindex = zindex + (steps.y << GRID_STRIDE_SHIFT_Y);
    float A101 = field[yindex];
    float A111 = field[yindex + steps.x];

    float Ax10 = A100 * (1.0f - a.x) + A110 * a.x;
    float Ax11 = A101 * (1.0f - a.x) + A111 * a.x;
    float Axy1 = Ax10 * (1.0f - a.y) + Ax11 * a.y;   // lerp between Ax0 & Ax1

    // now finally lerp between Axy0 & Axy1
    return limit * (Axy0 * (1.0f - a.z) + Axy1 * a.z);
}

// ---------------------------------------------------------------------------
// Name:        applySources
// Description: Update the density, temperature and velocity field with new
//              smoke leaving the teapot. This kernel is only run over one
//              height slice (the second one from the bottom) which intersects
//              the teapot spout.
//              Density and temperature are increased using a standard
//              probability distribution centered around an exit point and
//              limited to be inside the teaspout. The exit point is
//              randomized by the host.
//              The velocity vector in the region of smoke is set according
//              to values given by the host (generally movement up with
//              a side-wise movement that points from the center of the
//              teapot spout to the exit point.
// ---------------------------------------------------------------------------
__kernel void applySources(
    __global float* s,                  // density (scalar)
    __global float* t,                  // temperature (scalar)
    __global float4* u,                 // velocity (vector)
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2) + 1);
    int index = getIndex(coord);
    float2 pos = (float2)((float)coord.x, (float)coord.y);

    // Set limit to 1.0 if this grid point is inside the teaspout, 0.0 otherwise.
    float2 normPos = pos - p->spoutCenter;
    normPos.x *= p->spoutInvExtent.x;
    normPos.y *= p->spoutInvExtent.y;
    float limit = step(dot(normPos, normPos), 1.0f);

    // Calculate the intensity of smoke as a probability distribution centered around
    // the exit point defined by p->sourceCenter. Anything outide the teapot spout
    // is cut off.
    normPos = pos - p->sourceCenter;
    normPos.x *= p->spoutInvExtent.x;
    normPos.y *= p->spoutInvExtent.y;
    float intensity = limit * p->sourceDistributionBeta * exp(p->sourceDistributionAlpha * dot(normPos, normPos));

    // Increase density and temperature by calculated intensity at this point.
    s[index] += (p->KsDens * p->deltaTimeInSeconds * intensity);
    t[index] = clamp(t[index] + p->KsTemp * p->deltaTimeInSeconds * intensity, p->KminTemp, p->KmaxTemp);

    // Set velocity vector everywhere inside the teapot spout.
    u[index] = (p->sourceVelocity * limit);
}

// ---------------------------------------------------------------------------
// Name:        applyBuoyancy
// Description: Calculate the vertical force created by the combination of
//              smoke particle density (gravity) and temperature (buoyancy).
//              Update the velocity field (z coordinate) for given timestep.
// ---------------------------------------------------------------------------
__kernel void applyBuoyancy(
    __global const float* s,      // density (scalar)
    __global const float* t,      // temperature (scalar)
    __global float4* u,                 // velocity (vector)
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float density = s[index] * (-p->buoyAlpha);
    float temp = (t[index] - p->ambiantTemperature) * p->buoyBeta;
    u[index].z += ((density + temp) * p->deltaTimeInSeconds);
}

// ---------------------------------------------------------------------------
// Name:        calculateCurlU
// Description: Take the current velocity vector field and calculate the
//              vorticity vector field
// ---------------------------------------------------------------------------
__kernel void calculateCurlU(
    __global float4* u,                 // velocity (vector)
    __global float4* v,                 // Storage for CurlU (used later for vorticity)
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float3 srcU = u[index].xyz;

    int neighborIndex;

    // Vector field length on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    float3 pXBack = interpEdgeVec(srcU, u[neighborIndex].xyz);
    neighborIndex = getRightIndex(index, coord);
    float3 pXForw = interpEdgeVec(srcU, u[neighborIndex].xyz);

    // Vector field length on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    float3 pYBack = interpEdgeVec(srcU, u[neighborIndex].xyz);
    neighborIndex = getBackIndex(index, coord);
    float3 pYForw = interpEdgeVec(srcU, u[neighborIndex].xyz);

    // Vector field length on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    float3 pZBack = interpEdgeVec(srcU, u[neighborIndex].xyz);
    neighborIndex = getTopIndex(index, coord);
    float3 pZForw = interpEdgeVec(srcU, u[neighborIndex].xyz);

    // Calculate vorticity (curl operator applied to velocity vector field)
    float4 curlU;
    curlU.x = (pYForw.z - pYBack.z) * GRID_INV_SPACING - (pZForw.y - pZBack.y) * GRID_INV_SPACING;
    curlU.y = (pZForw.x - pZBack.x) * GRID_INV_SPACING - (pXForw.z - pXBack.z) * GRID_INV_SPACING;
    curlU.z = (pXForw.y - pXBack.y) * GRID_INV_SPACING - (pYForw.x - pYBack.x) * GRID_INV_SPACING;
    curlU.w = 0.0f;

    v[index] = curlU;
}

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

// ---------------------------------------------------------------------------
// Name:        advectFieldVelocity
// Description: Move the velocity vector field along. Here we use Jos Stam's
//              method where the field at grid position (x, y, z) is found
//              by tracing a particle at this position back in time (dt) to
//              the position that it occupied and copying the velocity vector
//              into the current position. We trace back along the velocity
//              vector at the current position. Since this may not fall on
//              a cell center, we use tri-linear interpolation to compute
//              the velocity vector at the previous position. If the trace
//              goes outside the grid, we assume the velocity is zero.
// ---------------------------------------------------------------------------
__kernel void advectFieldVelocity(
    __global const float4* u,
    __global float4* v,
    __constant SmokeSimConstants* p)
{
    int3 src = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(src);
    float4 srcU = u[index];

    // Trace grid position back based on velocity vector
    float3 coord = (float3)(
                       (float)src.x - srcU.x * p->deltaTimeInSeconds,
                       (float)src.y - srcU.y * p->deltaTimeInSeconds,
                       (float)src.z - srcU.z * p->deltaTimeInSeconds
                   );

    // Tri-linear interpolate to get the velocity vector at that position.
    v[index] = triLinearInterpolateVecField(u, coord);
}

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
    __global const int*    srcIndex,
    __global const float4* srcScale,
    __global float4*             u)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int dstIndex = getIndex(coord);
    int index = srcIndex[dstIndex];
    float4 scale = srcScale[index];
    float4 value = u[index];
    u[dstIndex] = (float4)(value.x * scale.x, value.y * scale.y, value.z * scale.z, 0.0f);
}

// ---------------------------------------------------------------------------
// Name:        computeFieldPressurePrep
// Description: Now that we have a new velocity vector field, we need to
//              compute the pressure at each point in the grid. This will
//              be done using Jacobi Poisson iteration based on the divergence
//              of the current velocity field. Here we perform the preparation
//              for the solver - set initial conditions (pressure = 0
//              everywhere) and compute the divergence of the velocity field.
// ---------------------------------------------------------------------------
__kernel void computeFieldPressurePrep(
    __global const float4* u,
    __global float* divU,
    __global float* srcP,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float4 srcU = u[index];

    // First calculate the divergence using 'staggered grid' method by sampling
    // finite diffrences between q(i+0.5) - q(i-0.5) and dividing by difference
    // deltaX which is 'unbiased'
    float forw;
    float back;
    int neighborIndex;

    // Vector field on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    back = (srcU.x + u[neighborIndex].x) * 0.5f;
    neighborIndex = getRightIndex(index, coord);
    forw = (srcU.x + u[neighborIndex].x) * 0.5f;
    float ddx = (forw - back) * GRID_INV_SPACING;

    // Vector field on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    back = (srcU.y + u[neighborIndex].y) * 0.5f;
    neighborIndex = getBackIndex(index, coord);
    forw = (srcU.y + u[neighborIndex].y) * 0.5f;
    float ddy = (forw - back) * GRID_INV_SPACING;

    // Vector field on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    back = (srcU.z + u[neighborIndex].z) * 0.5f;
    neighborIndex = getTopIndex(index, coord);
    forw = (srcU.z + u[neighborIndex].z) * 0.5f;
    float ddz = (forw - back) * GRID_INV_SPACING;

    divU[index] = ddx + ddy + ddz;
    srcP[index] = 0.0f;
}

// ---------------------------------------------------------------------------
// Name:        computeFieldPressureIter
// Description: Performs one Jacobi Poisson iteration. This kernel is called
//              for at least 30 iterations with the memory pointers srcP
//              and dstP being swapped between each iteration.
// ---------------------------------------------------------------------------
__kernel void computeFieldPressureIter(
    __global const float* divU,
    __global const float* srcP,
    __global float* dstP,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);

    // The equation in 2D:
    // We are solving Pij from equation: laplacian(Pij) = b
    // where b = divergence(U), U = vector field
    // laplacian(Pi,j) = (Pi+1,j + Pi-1,j + Pi,j+1, Pi,j-1 - 4*Pi,j) / (dx^2)
    // => Pi,j = (Pi+1,j + Pi-1,j + Pi,j+1, Pi,j-1 - laplacian(Pij)*(dx^2)) / 4
    // => Pi,j = (Pi+1,j + Pi-1,j + Pi,j+1, Pi,j-1 - bi,j*(dx^2)
    //
    // In 2D, alpha = -(dx^2) and rBeta = 1/4
    // In 3D, alpha = -(dx^2) and rBeta = 1/6
    //
    // NOTE: this equation assumes dx = dy = dz, i.e that cell spacing is
    // the same in all directions.
    //
    // alpha and rBeta are passed in the SmokeSimConstants as
    // KpressureJacobiPoissonAlpha and KpressureJacobiPoissonInvBeta
    // respectively.

    int neighborIndex;

    // Pressure to left and right
    neighborIndex = getLeftIndex(index, coord);
    float qXBack = srcP[neighborIndex];
    neighborIndex = getRightIndex(index, coord);
    float qXForw = srcP[neighborIndex];

    // Vector field on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    float qYBack = srcP[neighborIndex];
    neighborIndex = getBackIndex(index, coord);
    float qYForw = srcP[neighborIndex];

    // Vector field on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    float qZBack = srcP[neighborIndex];
    neighborIndex = getTopIndex(index, coord);
    float qZForw = srcP[neighborIndex];

    // b sample from center
    float bAlpha = divU[index] * p->KpressureJacobiPoissonAlpha;

    // evaluate jacobi iteration, which computer the next value for q
    dstP[index] = (qXBack + qXForw + qYBack + qYForw + qZBack + qZForw + bAlpha) * p->KpressureJacobiPoissonInvBeta;
}

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
    __global const int*    srcIndex,
    __global const float4* srcScale,
    __global float*              pressure)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int dstIndex = getIndex(coord);
    int index = srcIndex[dstIndex];
    float4 scale = srcScale[index];
    float value = pressure[index];
    pressure[dstIndex] = value * scale.w;
}

// ---------------------------------------------------------------------------
// Name:        projectFieldVelocity
// Description: Now that we have solved the pressure field, we "project" it
//              onto the velocity field. This is the mass-conservation part
//              of the Navier-Stokes fluid equations.
// ---------------------------------------------------------------------------
__kernel void projectFieldVelocity(
    __global const  float* srcP,
    __global const float4* u,
    __global float4* v,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float centerP = srcP[index];
    float4 srcU = u[index];

    // 'staggered' grid style sampling with bilinear sampling
    float forw;
    float back;
    int neighborIndex;
    float4 gradP;

    // Pressure on left & right faces
    neighborIndex = getLeftIndex(index, coord);
    back = (centerP + srcP[neighborIndex]) * 0.5f;
    neighborIndex = getRightIndex(index, coord);
    forw = (centerP + srcP[neighborIndex]) * 0.5f;
    gradP.x = (forw - back) * GRID_INV_SPACING;

    // Pressure on front & back faces
    neighborIndex = getFrontIndex(index, coord);
    back = (centerP + srcP[neighborIndex]) * 0.5f;
    neighborIndex = getBackIndex(index, coord);
    forw = (centerP + srcP[neighborIndex]) * 0.5f;
    gradP.y = (forw - back) * GRID_INV_SPACING;

    // Pressure on bottom & top faces
    neighborIndex = getBottomIndex(index, coord);
    back = (centerP + srcP[neighborIndex]) * 0.5f;
    neighborIndex = getTopIndex(index, coord);
    forw = (centerP + srcP[neighborIndex]) * 0.5f;
    gradP.z = (forw - back) * GRID_INV_SPACING;

    gradP.w = 0.0f;

    v[index] = srcU - gradP;
}

// ---------------------------------------------------------------------------
// Name:        advectFieldScalar
// Description: This kernel advects scalar fields (density, temperature) based
//              on the velocity vector field. Physically, this is the movement
//              of smoke particles through the vector field.
//              Here we use Jos Stam's method where the field at grid position
//              (x, y, z) is found by tracing a particle at this position back
//              in time (dt) along the velocity vector at this position to
//              the position that it occupied then and copy the value of the
//              the field at that previous position to the current position.
//              Since the previous position may not fall on a cell center,
//              we use tri-linear interpolation to compute the field
//              position. If the trace goes outside the grid, we assume
//              that the field is zero. The following steps (dissipation)
//              will force minimum values of the field at every cell position
//              thus it is acceptible to set a value of zero for both density
//              and temperature.
// ---------------------------------------------------------------------------
__kernel void advectFieldScalar(
    __global const  float4* u,
    __global const  float* s0,
    __global float* s1,
    __constant SmokeSimConstants* p)
{
    int3 src = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(src);
    float4 srcU = u[index];

    // Trace grid position back based on velocity vector
    float3 coord = (float3)(
                       (float)src.x - srcU.x * p->deltaTimeInSeconds,
                       (float)src.y - srcU.y * p->deltaTimeInSeconds,
                       (float)src.z - srcU.z * p->deltaTimeInSeconds
                   );

    // Tri-linear interpolate to get the velocity vector at that position.
    s1[index] = triLinearInterpolateScalarField(s0, coord);
}

// ---------------------------------------------------------------------------
// Name:        dissipateDensity
// Description: Reduce density at every grid position according to rate
//              factor KdissipateDens. Apply minimum density constraint.
// ---------------------------------------------------------------------------
__kernel void dissipateDensity(
    __global const  float* s0,
    __global float* s1,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    s1[index] = max(p->KminDens, s0[index] * p->KdissipateDens);
}

// ---------------------------------------------------------------------------
// Name:        dissipateTemperature
// Description: Reduce temperature at every grid position according to rate
//              factor KdissipateTemp. Apply minimum temperature constraint.
// ---------------------------------------------------------------------------
__kernel void dissipateTemperature(
    __global const  float* t0,
    __global float* t1,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    t1[index] = max(p->KminTemp, t0[index] * p->KdissipateTemp);
}

// ---------------------------------------------------------------------------
// Name:        createDensityTexture
// Description: Setup the density (float RGBA) buffer that will be copied into
//              the OpenGL texture. We set the RGB to (1.0, 1.0, 1.0) and the
//              alpha to the negative exponent of the field density.
// ---------------------------------------------------------------------------
__kernel void createDensityTexture(
    __global const float* s,
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

// ---------------------------------------------------------------------------
// Name:        debugDensity
// Description: Set the density texture to show the raw density field.
// ---------------------------------------------------------------------------
__kernel void debugDensity(
    __global const float* s,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    d[index] = (float4)(1.0f, 1.0f, 1.0f, s[index]);
}

// ---------------------------------------------------------------------------
// Name:        debugTemperature
// Description: Set the density texture to show the raw temperature field.
// ---------------------------------------------------------------------------
__kernel void debugTemperature(
    __global const float* t,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float temp = t[index] - p->KminTemp;
    d[index] = (float4)(temp * 0.01f, 0.0f, (p->KmaxTemp - p->KminTemp - temp) * 0.01f, temp * 0.01f);
}

// ---------------------------------------------------------------------------
// Name:        debugVelocityVector
// Description: Set the density texture to show the raw velocity vector field.
//              Red shows movement in x, green shows movement in y and
//              blue shows movement in z and alpha set to the length of the
//              vector. "Movement" is expressed as the distance, in cell
//              units, that a particle will travel at maximum delta-time.
// ---------------------------------------------------------------------------
__kernel void debugVelocityVector(
    __global const float4* u,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    float3 au = fabs(u[index].xyz) * p->maxDeltaTimeInSeconds * 0.5f;

    d[index] = (float4)(
                   au, length(au)
               );
}

// ---------------------------------------------------------------------------
// Name:        debugVelocityLength
// Description: Set the density texture to show the distance, in cell units,
//              that a point will move in one time step (maximum delta-time).
//              This is set in the alpha value - saturation means that the
//              point will move two grid cells or more.
// ---------------------------------------------------------------------------
__kernel void debugVelocityLength(
    __global const float4* u,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    d[index] = (float4)(1.0f, 1.0f, 1.0f, length(u[index].xyz) * p->maxDeltaTimeInSeconds * 0.5f);
}

// ---------------------------------------------------------------------------
// Name:        debugFieldPressure
// Description: Set the density texture to show the raw pressure field. The
//              alpha value is set to show the velocity change that the
//              pressure field will create at each cell. Saturation means that
//              the pressure will cause a change in velocity by two or more
//              units for the maximum time step.
// ---------------------------------------------------------------------------
__kernel void debugFieldPressure(
    __global const float* srcP,
    __global float4* d,
    __constant SmokeSimConstants* p)
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    int index = getIndex(coord);
    d[index] = (float4)(1.0f, 1.0f, 1.0f, fabs(srcP[index]) * p->maxDeltaTimeInSeconds * 0.5f);
}
