//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Enumeration and support functions for use in describing the current state of the acvtive graphics server
//==============================================================================

#include "GraphicsServerState.h"
#include "mymutex.h"

#ifdef CODEXL_GRAPHICS
    /// Set the default stall state to true (commands will only be accepted when the app has rendered once)
    static bool g_stalledState = true;
#else
    /// For GPS we do not use server status returns
    static bool g_stalledState = false;
#endif

static bool g_shutdownState = false; ///< Global variable to record the current shutdown state.

mutex stalledMutex; ///< Mutex for use when setting the stalled state. it is accessed from different threads.
mutex shutdownMutex; ///< Mutex for use when setting the shutdown state. it is accessed from different threads.

/// Sets the current server state
/// \param stalledState The current state of the server plugin.
void SetServerStalledState(bool stalledState)
{
    ScopeLock lock(stalledMutex);
    g_stalledState = stalledState;
}

/// Gets the current stalled state
/// \return True if rendering has stalled, false if not stalled.
bool GetServerStalledState()
{
    ScopeLock lock(stalledMutex);
    return g_stalledState;
}

/// Sets the current server state
/// \param shutdownState of the GPUPerfServer.
void SetServerShutdownState(bool shutdownState)
{
    ScopeLock lock(shutdownMutex);
    g_shutdownState = shutdownState;
}

/// Gets the current shutdown state
/// \return True if shutting down, false if running.
bool GetServerShutdownState()
{
    ScopeLock lock(shutdownMutex);
    return g_shutdownState;
}
