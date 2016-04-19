//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallStack.h
///
//==================================================================================

#ifndef _CALLSTACK_H_
#define _CALLSTACK_H_

#include "CallSite.h"

struct EventSampleInfo
{
    CallSite* m_pSite;
    gtUInt32 m_eventId; // EventMaskType
    gtUInt32 m_threadId; // ThreadIdType
    gtUInt64 m_count;

    EventSampleInfo() : m_pSite(NULL) {}
};

template <>
inline bool gtSmallSList_node<EventSampleInfo>::IsAnchor() const
{
    return NULL == m_val.m_pSite;
}

class CP_CSS_API EventSampleList : public gtSmallSList<EventSampleInfo>
{
public:
    const_iterator FindFirst(gtUInt32 eventId) const;
};

class CP_CSS_API CallStack
{
private:
    typedef FlowGraph_Path<gtVAddr, CallSiteOffsets, EventSampleList, FLOW_GRAPH_UPSTREAM, FLOW_GRAPH_PATH_FAST_ITERATION> BasePath;

    BasePath m_path;

public:
    typedef FlowGraph_DelegateIterator<CallSite, BasePath::const_iterator, BasePath::iterator, true>  const_iterator;
    typedef FlowGraph_DelegateIterator<CallSite, BasePath::const_iterator, BasePath::iterator, false>       iterator;

    const_iterator begin() const;
    iterator begin();
    const_iterator end() const;
    iterator end();

    // Order by eventId -> threadId -> pSite
    void AddEventSample(const EventSampleInfo& eventSample);

    const EventSampleList& GetEventSampleList() const;

    unsigned GetDepth() const;

    unsigned CopySites(CallSite** ppSites, unsigned offset = 0U);
};

#endif // _CALLSTACK_H_
