//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for shutting down the web server
//==============================================================================

#ifndef SHUTDOWN_THREAD_H_
#define SHUTDOWN_THREAD_H_

#include <AMDTOSWrappers/Include/osThread.h>

/// A class describing the shutdown listener thread
class ShutdownThread : public osThread
{
public:
    /// constructor
    ShutdownThread(const gtString& threadName, osThread* pluginThread, osThread* renderStallThread)
        : osThread(threadName)
        , m_pluginThread(pluginThread)
        , m_renderStallThread(renderStallThread)
    {
    }

protected:
    /// This is the main thread entry point. Code to be run in the
    /// thread should be placed here
    virtual int entryPoint()
    {
        WaitForShutdownResponse(m_pluginThread, m_renderStallThread);
        return 0;
    }

    void WaitForShutdownResponse(void* pluginThread, void* renderStallThread);

private:

    osThread*   m_pluginThread;     ///< pointer to the plugin thread
    osThread*   m_renderStallThread;     ///< pointer to the render stall thread
};

#endif // SHUTDOWN_THREAD_H_