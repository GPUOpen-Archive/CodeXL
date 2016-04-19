//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Enumeration and support functions for use in describing the current state of the acvtive graphics server
//==============================================================================

#ifndef _GRAPHICS_SERVER_STATE_
#define _GRAPHICS_SERVER_STATE_

/// Server state enumerations. Used to let the client know when the server rendering has stalled or if the process has been terminated.
enum GRAPHICS_SERVER_STATE
{
    GRAPHICS_SERVER_STATE_ACTIVE = 0,
    GRAPHICS_SERVER_STATE_STALLED = 1,
    GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING = 2
};

#define GRAPHICS_SERVER_STATUS_STALL_LOOP_SLEEP_TIME 20 ///< Define how long the sleep time is between loop iterations
#define GRAPHICS_SERVER_STATUS_STALL_THRESHOLD_TIME 500 ///< If there server does not render in under this threshold time, the stalled state will be set.

/// Uncomment the following to allow the graphics server to return server status codes
#define USE_GRAPHICS_SERVER_STATUS_RETURN_CODES

/// Sets the current server state
/// \param stalledState The current state of the server plugin.
void SetServerStalledState(bool stalledState);

/// Sets the current server state
/// \param shutdownState of the GPUPerfServer.
void SetServerShutdownState(bool shutdownState);

/// Gets the current stalled state
/// \return True if rendering has stalled, false if not stalled.
bool GetServerStalledState();

/// Gets the current shutdown state
/// \return True if shutting down, false if running.
bool GetServerShutdownState();

#endif