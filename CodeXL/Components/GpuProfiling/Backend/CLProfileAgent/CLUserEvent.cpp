//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief User Event wrapper. It maintains a list of kernels that are waiting for this user event
//==============================================================================

#include "CLUserEvent.h"

CLUserEvent::CLUserEvent(cl_event event)
{
    m_event = event;
}

CLUserEvent::~CLUserEvent(void)
{
}


void CLUserEvent::AddDependentKernel(CLDeferredKernel* pKernel)
{
    m_KernelList.push_back(pKernel);
}

void CLUserEvent::AddDependentEvent(cl_event event)
{
    m_DependentEventList.push_back(event);
}

/// Iterate all kernels that are dependent on and try to dispatch if they are ready.
void CLUserEvent::TryDispatch()
{
    for (std::vector<CLDeferredKernel*>::iterator it = m_KernelList.begin(); it != m_KernelList.end(); it++)
    {
        bool done = (*it)->TryProfile(m_event);

        if (done)
        {
            delete(*it);
        }
    }
}

bool CLUserEvent::CheckDependency(cl_event event)
{
    if (event == m_event)
    {
        return true;
    }

    for (std::vector<cl_event>::iterator it = m_DependentEventList.begin(); it != m_DependentEventList.end(); it++)
    {
        if (event == *it)
        {
            return true;
        }
    }

    return false;
}
