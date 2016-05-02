//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktDefines.h
/// \brief  A global include file for the Vulkan server.
//=============================================================================

#ifndef __VKT_DEFINES_H__
#define __VKT_DEFINES_H__

//-----------------------------------------------------------------------------
// The following defines are experimental switches to tweak server behavior.
//-----------------------------------------------------------------------------

/// A flag used to enable threaded worker profiler collection.
#define GATHER_PROFILER_RESULTS_WITH_WORKERS 0

/// Enable tracking of each command inserted into Command Buffers.
#define TRACK_CMD_BUF_COMMANDS               0

/// Enable dynamically resizing the buffer used to store profiler results.
#define DYNAMIC_PROFILER_GROUP_SIZING        0

/// The timeout in Milliseconds to wait on queue's profiler results.
#define QUEUE_RESULTS_WORKER_TIMEOUT         1000

/// How long to wait for a queue to be done before retrieving results, in nanoseconds
#define FENCE_TIMEOUT_TIME                   1000000

/// Inject timestamps at the very beginning and at the very end of each command buffer
#define MEASURE_WHOLE_CMD_BUFS               1

#endif // __VKT_DEFINES_H__
