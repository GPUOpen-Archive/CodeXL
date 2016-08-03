//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktDefines.h
/// \brief  A global include file for the Vulkan server.
//=============================================================================

#ifndef __VKT_DEFINES_H__
#define __VKT_DEFINES_H__

#include <AMDTBaseTools/Include/gtStringConstants.h>

//-----------------------------------------------------------------------------
// The following defines are experimental switches to tweak server behavior.
//-----------------------------------------------------------------------------

#ifdef WIN32
    /// Capture render target by issuing a new queue submit. Else, piggyback off of the application's next queue submit.
    #define INSTANT_FRAMEBUFFER_CAPTURE      1
#else
    /// Capture render target by issuing a new queue submit. Else, piggyback off of the application's next queue submit.
    #define INSTANT_FRAMEBUFFER_CAPTURE      0
#endif

/// A flag used to enable threaded worker profiler collection.
#define GATHER_PROFILER_RESULTS_WITH_WORKERS 1

/// Enable tracking of each command inserted into command buffers. For debugging only.
#define TRACK_CMD_BUF_COMMANDS               0

/// Enable dynamically resizing the buffer used to store profiler results.
#define DYNAMIC_PROFILER_GROUP_SIZING        1

/// The timeout in Milliseconds to wait on queue's profiler results.
#define QUEUE_RESULTS_WORKER_TIMEOUT         1000

/// Use fences to wait on profiler results to come back. Else, wait for idle queue.
#define GPU_FENCES_FOR_PROFILER_WAIT         1

/// How long to wait for a queue to be done before retrieving results, in nanoseconds.
#define GPU_FENCE_TIMEOUT_TIME               100000000

/// Inject timestamps at the very beginning and at the very end of each command buffer.
#define MEASURE_WHOLE_CMD_BUFS               1

/// Since Vulkan does not provide function to calibrate CPU/GPU timestamp, do it ourselves manually.
#define MANUAL_TIMESTAMP_CALIBRATION         1

/// Specify how old static profiler memory may get before our worker code releases it.
#define DEFERRED_RELEASE_FRAME_COUNT         8

#endif // __VKT_DEFINES_H__
