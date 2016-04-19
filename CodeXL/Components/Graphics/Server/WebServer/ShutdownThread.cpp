//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for shutting down the web server
//==============================================================================

#include "ShutdownThread.h"
#include "../Common/NamedEvent.h"
#include "../Common/GraphicsServerState.h"
#include "../Common/SharedMemoryManager.h"

//--------------------------------------------------------------
/// Wait for a shutdown event
/// \param pluginThread pointer to plugin thread
/// \param renderStallThread pointer to render stall thread
//--------------------------------------------------------------
void ShutdownThread::WaitForShutdownResponse(void* pluginThread, void* renderStallThread)
{
    NamedEvent shutdownEvent;
    shutdownEvent.Open("GPS_SHUTDOWN_SERVER");

    shutdownEvent.Wait();

    // shutdown event was signaled.
    SetServerShutdownState(true);

    // Terminate the plugin response thread
    osThread* thread = (osThread*)pluginThread;
    thread->terminate();

    while (thread->isAlive())
    {
        ;
    }

    // Terminate the render stall detection thread
    thread = (osThread*)renderStallThread;
    thread->terminate();

    while (thread->isAlive())
    {
        ;
    }

    // shutdown the shared memory
    smClose("PLUGINS_TO_GPS");
    shutdownEvent.Close();
}