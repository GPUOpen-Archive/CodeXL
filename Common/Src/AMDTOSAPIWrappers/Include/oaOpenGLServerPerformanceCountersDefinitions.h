//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osOpenGLServerPerformanceCountersDefinitions.h ------------------------------

#ifndef __OSOPENGLSERVERPERFORMANCECOUNTERSDEFINITIONS_H
#define __OSOPENGLSERVERPERFORMANCECOUNTERSDEFINITIONS_H


// ------------------------------------------------------------------------------------------
//  File: This file contains definitions related to the OpenGLServer performance counters
// ------------------------------------------------------------------------------------------

// Enumerates the available render context performance counters:
enum osRenderContextPerformanceCounters
{
    GS_FPS_COUNTER,                             // Frames per second performance counter.
    GS_FRAME_TIME_COUNTER,                      // Frame time counter (effectively 1/FPS), see http://www.mvps.org/directx/articles/fps_versus_frame_time.htm.
    GS_FOUNCTION_CALLS_PS_COUNTER,              // Function calls per second performance counter.
    GS_TEXTURE_OBJECTS_AMOUNT_COUNTER,          // Allocated texture objects amount.
    GS_RENDERED_VERTICES_COUNTER,               // Rendered vertices per frame amount
    GS_RENDERED_TRIANGLES_COUNTER,              // Rendered triangles per frame amount
    GS_RENDERED_LINES_COUNTER,                  // Rendered lines per frame amount
    GS_RENDERED_POINTS_COUNTER,                 // Rendered points per frame amount
    GS_RENDERED_PRIMITIVES_COUNTER,             // Rendered primitives per frame amount
    GS_LEVEL_0_LOADED_TEXELS_AMOUNT_COUNTER,    // Amount of texture objects level 0 loaded texels.
    GS_RENDER_CONTEXTS_COUNTERS_AMOUNT          // The amount of render context performance counters.
};


#endif //__OSOPENGLSERVERPERFORMANCECOUNTERSDEFINITIONS_H

