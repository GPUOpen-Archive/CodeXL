//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotSmokeSimulationVec.h
///
//==================================================================================

//------------------------------ AMDTTeapotSmokeSimulationVec.h ------------------------------

#ifndef __AMDTTEAPOTSMOKESIMULATIONVEC_H
#define __AMDTTEAPOTSMOKESIMULATIONVEC_H


// ---------------------------------------------------------------------------
// Name:        interpEdgeVec
// Description: Interpolate the velocity vector on the cell edge.
// ---------------------------------------------------------------------------
float3 interpEdgeVec(float3 u1, float3 u2)
{
    return (u1 + u2) * 0.5f;
}

#endif //__AMDTTEAPOTSMOKESIMULATIONVEC_H

