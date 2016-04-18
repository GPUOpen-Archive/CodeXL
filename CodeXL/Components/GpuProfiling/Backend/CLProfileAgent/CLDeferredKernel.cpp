//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Deferred kernel It records the kernelDispatch parameters so that we can replay it later
//==============================================================================

#include "CLDeferredKernel.h"

CLDeferredKernel::CLDeferredKernel(void)
{
    command_queue = NULL;
    kernel = NULL;
    work_dim = 0;
    global_work_offset = NULL;
    global_work_size = NULL;
    local_work_size = NULL;
    num_events_in_wait_list = 0;
    event_wait_list = NULL;
    event = NULL;
}

CLDeferredKernel::~CLDeferredKernel(void)
{
}

bool CLDeferredKernel::TryProfile(cl_event eventIn)
{
    std::vector<cl_event>::iterator found = userEventWaitList.end();

    for (std::vector<cl_event>::iterator it = userEventWaitList.begin(); it != userEventWaitList.end(); it++)
    {
        if (*it == eventIn)
        {
            found = it;
            break;
        }
    }

    if (found != userEventWaitList.end())
    {
        userEventWaitList.erase(found);

        if (userEventWaitList.size() == 0)
        {
            // No one in waiting list
            // Kick off
            return true;
        }
    }

    // else, kernel is not waiting for this event
    return false;
}
