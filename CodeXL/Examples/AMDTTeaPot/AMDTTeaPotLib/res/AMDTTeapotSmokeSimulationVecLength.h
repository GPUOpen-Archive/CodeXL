//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulationVecLength.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulationVecLength.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATIONVECLENGTH_H
#define __AMDTTEAPOTSMOKESIMULATIONVECLENGTH_H

// ---------------------------------------------------------------------------
// Name:        interpEdgeVecLength
// Description: Interpolate the velocity (vector length) on the cell edge.
// ---------------------------------------------------------------------------
float interpEdgeVecLength(float3 u1, float3 u2)
{
    return length((u1 + u2) * 0.5f);
}

#endif //__AMDTTEAPOTSMOKESIMULATIONVECLENGTH_H

