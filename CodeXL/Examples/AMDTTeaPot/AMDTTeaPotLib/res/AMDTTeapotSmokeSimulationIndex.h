//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulationIndex.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulationIndex.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATIONINDEX_H
#define __AMDTTEAPOTSMOKESIMULATIONINDEX_H

// ---------------------------------------------------------------------------
// Name:        getLeftIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell to the left (x-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getLeftIndex(const int index, const int3 coord)
{
    return index - step((float)1, (float)coord.x);
}

// ---------------------------------------------------------------------------
// Name:        getLeftIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell to the right (x-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getRightIndex(const int index, const int3 coord)
{
    return index + step((float)coord.x, (float)(GRID_NUM_CELLS_X - 2));
}

// ---------------------------------------------------------------------------
// Name:        getFrontIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell in front (y-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getFrontIndex(const int index, const int3 coord)
{
    return index - (((int)step((float)1, (float)coord.y)) << GRID_STRIDE_SHIFT_Y);
}

// ---------------------------------------------------------------------------
// Name:        getBackIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell behind (y-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getBackIndex(const int index, const int3 coord)
{
    return index + (((int)step((float)coord.y, (float)(GRID_NUM_CELLS_Y - 2))) << GRID_STRIDE_SHIFT_Y);
}

// ---------------------------------------------------------------------------
// Name:        getTopIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell above (z-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getTopIndex(const int index, const int3 coord)
{
    return index + (((int)step((float)coord.z, (float)(GRID_NUM_CELLS_Z - 2))) << GRID_STRIDE_SHIFT_Z);
}

// ---------------------------------------------------------------------------
// Name:        getBottomIndex
// Description: For a given grid coordinate, return the memory buffer index
//              of the cell below (z-axis). If the point is already
//              on the grid edge, return the same index.
// ---------------------------------------------------------------------------
int getBottomIndex(const int index, const int3 coord)
{
    return index - (((int)step((float)1, (float)coord.z)) << GRID_STRIDE_SHIFT_Z);
}

#endif //__AMDTTEAPOTSMOKESIMULATIONINDEX_H

