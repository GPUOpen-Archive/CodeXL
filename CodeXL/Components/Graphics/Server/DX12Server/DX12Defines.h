//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12Defines.h
/// \brief  A global include file for the DX12 server.
//=============================================================================
#ifndef DX12DEFINES_H
#define DX12DEFINES_H

// Include all D3D headers.
#include <d3d12.h>

/// The name of the D3D library DLL that we hook or replace.
#define D3D12_DLL "d3d12.dll"

//-----------------------------------------------------------------------------
// The following defines are "Experiment switches." Flip them to alter server behavior.
//-----------------------------------------------------------------------------

/// Allow timestamping of Direct Command Lists.
#define TIMESTAMP_DIRECT_COMMAND_LISTS       1

/// Allow timestamping of Compute Command Lists.
#define TIMESTAMP_COMPUTE_COMMAND_LISTS      1

/// Serialize a multithreaded application by locking at the start of each D3D12 function.
#define SERIALIZE_DX12_ENTRY_POINTS          0

/// A flag used to enable threaded worker profiler collection.
#define GATHER_PROFILER_RESULTS_WITH_WORKERS 1

/// The timeout in Milliseconds to wait on Compute Queue's profiler results.
#define COMPUTE_QUEUE_RESULTS_TIMEOUT        1000

/// The timeout in Milliseconds to wait on a Direct Queue's profiler results.
#define DIRECT_QUEUE_RESULTS_TIMEOUT         INFINITE

/// Enable tracking of each command inserted into Command Lists.
#define TRACK_CMD_LIST_COMMANDS              0

/// Enable dynamically resizing the buffer used to store profiler results.
#define DYNAMIC_PROFILER_GROUP_SIZING        1

#endif // DX12DEFINES_H
