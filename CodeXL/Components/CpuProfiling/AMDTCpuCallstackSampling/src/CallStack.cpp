//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallStack.cpp
///
//==================================================================================

#include "CallStack.h"
#include <cstdio>
#include <new>

CallStack::const_iterator CallStack::begin() const
{
    return m_path.begin();
}

CallStack::iterator CallStack::begin()
{
    return m_path.begin();
}

CallStack::const_iterator CallStack::end() const
{
    return m_path.end();
}

CallStack::iterator CallStack::end()
{
    return m_path.end();
}

void CallStack::AddEventSample(const EventSampleInfo& eventSample)
{
    bool aggregated = false;
    EventSampleList& samples = m_path.GetData();
    EventSampleList::iterator it = samples.begin(), itEnd = samples.end();
    EventSampleList::iterator itPrev = itEnd;

    while (it != itEnd && eventSample.m_eventId > it->m_eventId)
    {
        itPrev = it++;
    }

    if (it != itEnd && eventSample.m_eventId == it->m_eventId)
    {
        while (eventSample.m_threadId > it->m_threadId)
        {
            itPrev = it++;

            if (it == itEnd || eventSample.m_eventId != it->m_eventId)
            {
                break;
            }
        }

        while (it != itEnd && eventSample.m_threadId == it->m_threadId && eventSample.m_eventId == it->m_eventId)
        {
            if (eventSample.m_pSite == it->m_pSite)
            {
                it->m_count += eventSample.m_count;
                aggregated = true;
                break;
            }

            itPrev = it++;
        }
    }

    if (!aggregated)
    {
        samples.InsertAfter(itPrev, eventSample);
    }
}


const EventSampleList& CallStack::GetEventSampleList() const
{
    return m_path.GetData();
}

unsigned CallStack::GetDepth() const
{
    return m_path.GetLength();
}

unsigned CallStack::CopySites(CallSite** ppSites, unsigned offset)
{
    return m_path.CopyFullWalk(reinterpret_cast<BasePath::Node**>(ppSites), offset);
}
