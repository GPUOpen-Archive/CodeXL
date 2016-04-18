//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallSite.h
///
//==================================================================================

#ifndef _CALLSITE_H_
#define _CALLSITE_H_

#include "CpuCallstackSamplingDLLBuild.h"
#include "FlowGraph/FlowGraph.h"

struct CallSiteOffsets
{
    int m_stackOffset;
    int m_frameOffset;

    CallSiteOffsets() : m_stackOffset(-1), m_frameOffset(-1) {}
};

struct CallSite;

template <>
inline bool gtSmallSList_node<FlowGraph_Node<gtVAddr, CallSiteOffsets, FLOW_GRAPH_UPSTREAM>*>::IsAnchor() const
{
    return NULL == m_val;
}

template <>
inline bool gtSmallSList_node<CallSite*>::IsAnchor() const
{
    return NULL == m_val;
}

class CP_CSS_API CallSiteList : public gtSmallSList<CallSite*>
{
public:
    bool AddUnique(CallSite& site);

    const_iterator Find(gtVAddr traverseAddr) const;
    iterator Find(gtVAddr traverseAddr);
};

struct CallSite
{
    // This field contains one of two possible addresses:
    // 1. The return address - if this is not sampled data, i.e. this position was
    //    reached by walking the stack down from the sampled address.
    // 2. The sample address - if this is sampled data which is at the top of the call stack.
    const gtVAddr m_traverseAddr;

    // The following 2 fields were used in CodeXL 1.3, not used in CodeXL 1.4, 1.5 and 1.6.
    // Ehud removed the use of these fields because the algorithm that used them to match a given
    // virtual stack with a traversed part of the call-graph, contained bugs. This algorithm is currently
    // commented out in the function VirtualStackWalker::TraverseFromGraph().
    int m_stackOffset;
    int m_frameOffset;

    CallSiteList m_parents;
    PathIndexSet m_callStackIndices;

    CallSite(gtVAddr traverseAddr, int stackOffset = -1, int frameOffset = -1) : m_traverseAddr(traverseAddr),
        m_stackOffset(stackOffset),
        m_frameOffset(frameOffset)
    {
    }
};

#endif // _CALLSITE_H_
