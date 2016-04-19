//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for gathering request data sent back
/// from the server plugin and sending it back to the client
//==============================================================================

#ifndef PLUGIN_RESPONSE_THREAD_H_
#define PLUGIN_RESPONSE_THREAD_H_

/// Worker thread to pass request data back to the client.
class PluginResponseThread : public osThread
{
public:
    /// constructor
    PluginResponseThread(const gtString& threadName) : osThread(threadName)
    {
    }

protected:
    /// This is the main thread entry point. Code to be run in the
    /// thread should be placed here
    virtual int entryPoint()
    {
        WaitForPluginResponses(NULL);
        return 0;
    }

    void WaitForPluginResponses(void* pData);
};

#endif