//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulationClamp.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulationClamp.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATIONCLAMP_H
#define __AMDTTEAPOTSMOKESIMULATIONCLAMP_H


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

#endif //__AMDTTEAPOTSMOKESIMULATIONCLAMP_H

