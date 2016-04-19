//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Deferred kernel It records the kernelDispatch parameters so that we can replay it later
//==============================================================================

#ifndef _CL_DEFERRED_KERNEL_H_
#define _CL_DEFERRED_KERNEL_H_

#include <vector>
#include <CL/opencl.h>

//------------------------------------------------------------------------------------
/// Deferred kernel It records the kernelDispatch parameters so that we can replay it later
/// !!! Not yet implemented
//------------------------------------------------------------------------------------
class CLDeferredKernel
{
public:
    /// Constructor
    CLDeferredKernel(void);

    /// Destructor
    ~CLDeferredKernel(void);

    /// Remove user event from waitEventWaitList, try to dispatch is waitEventWaitList is empty
    bool TryProfile(cl_event event);

    /// Save the parameter values
    /// \param command_queue Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param kernel Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param work_dim Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param global_work_offset Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param global_work_size Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param local_work_size Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param event_wait_list Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param event Parameter for CLAPI_clEnqueueNDRangeKernel
    void Create(
        cl_command_queue  command_queue,
        cl_kernel   kernel,
        cl_uint  work_dim,
        const size_t*  global_work_offset,
        const size_t*  global_work_size,
        const size_t*  local_work_size,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    CLDeferredKernel(const CLDeferredKernel& obj);
    CLDeferredKernel& operator= (const CLDeferredKernel& obj);

private:
    std::vector<cl_event> userEventWaitList;
    cl_command_queue  command_queue;
    cl_kernel         kernel;
    cl_uint           work_dim;
    size_t*           global_work_offset;
    size_t*           global_work_size;
    size_t*           local_work_size;
    cl_uint           num_events_in_wait_list;
    cl_event*         event_wait_list;
    cl_event        event;
};

#endif // _CL_DEFERRED_KERNEL_H_
