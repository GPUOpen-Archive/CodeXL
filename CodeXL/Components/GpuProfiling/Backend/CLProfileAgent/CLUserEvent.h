//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief User Event wrapper. It maintains a list of kernels that are waiting for this user event
//==============================================================================

#ifndef _CL_USER_EVENT_H_
#define _CL_USER_EVENT_H_

#include <CL/opencl.h>
#include <vector>
#include "CLGPAProfiler.h"

//------------------------------------------------------------------------------------
/// User Event wrapper. It maintains a list of kernels that are waiting for this user event
/// It also keeps track of all events that depend on it
//------------------------------------------------------------------------------------
class CLUserEvent
{
    friend class CLGPAProfiler;
public:

    /// Constructor
    /// \param event cl event object
    CLUserEvent(cl_event event);

    /// Destructor
    ~CLUserEvent();

    /// In Mine_EnqueueNDRangeKernel, if user event found in wait_list, create deferred kernel object and add to the list
    /// \param pKernel Deferred kernel object
    void AddDependentKernel(CLDeferredKernel* pKernel);

    /// Add dependent event, keep track of event that is created by an Enqueue commend which depends on this user event
    /// \param event event object
    void AddDependentEvent(cl_event event);

    /// Check whether event object depends on this user event (event == m_event) or event depends on items in m_DependentEventList
    /// \param event cl event object
    /// \return trus is current event depends on this user event
    bool CheckDependency(cl_event event);

    /// Iterate all kernels that are dependent on and try to dispatch if they are ready.
    void TryDispatch();

private:
    CLUserEvent(const CLUserEvent& obj);
    CLUserEvent& operator= (const CLUserEvent& obj);

private:
    cl_event m_event; ///< user event object
    std::vector<CLDeferredKernel*> m_KernelList; ///< kernels that depend on this user event
    std::vector<cl_event> m_DependentEventList; ///< events that depend on this user event
};

#endif //_CL_USER_EVENT_H_
