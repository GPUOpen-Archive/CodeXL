//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for detecting rendering stalls and
/// returning the stalled return code to the client
//==============================================================================

#ifndef RENDER_STALL_THREAD_H_
#define RENDER_STALL_THREAD_H_

#include <AMDTOSWrappers/Include/osThread.h>

/// Class to encapsulate a thread which detects if the server rendering has stalled.
class RenderStallThread : public osThread
{
public:
    /// constructor
    RenderStallThread(const gtString& threadName) : osThread(threadName)
    {
    }

protected:

    /// This is the main thread entry point. Code to be run in the
    /// thread should be placed here
    virtual int entryPoint()
    {
        CheckForRenderStall();
        return 0;
    }

    void CheckForRenderStall();
};

#endif //RENDER_STALL_THREAD_H_