//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallGraph.h
///
//==================================================================================

#ifndef _CALLGRAPH_H_
#define _CALLGRAPH_H_

#include "CallStack.h"

class CallStackBuilder;

class CP_CSS_API CallGraph
{
private:
    typedef FlowGraph<gtVAddr, CallSiteOffsets, EventSampleList, FLOW_GRAPH_UPSTREAM, FLOW_GRAPH_PATH_FAST_ITERATION> BaseGraph;

    BaseGraph m_graph;

public:
    ~CallGraph();

    CallStack* GetCallStack(unsigned index) const;
    CallStack* GetEmptyCallStack(unsigned& index) const;
    CallStack& AddCallStack(CallSite** ppPathSites, unsigned length, unsigned& index);
    CallStack& AddCallStackSample(CallSite** ppPathSites, unsigned length, const EventSampleInfo& eventSample, unsigned& index);

    unsigned GetCallStacksCount() const;

    CallSite* AcquireCallSite(gtVAddr traverseAddr);
    CallSite* AcquireCallSite(gtVAddr traverseAddr, int stackOffset, int frameOffset);

    const CallSite* FindCallSite(gtVAddr traverseAddr) const;
    CallSite* FindCallSite(gtVAddr traverseAddr);

    bool InsertCallSite(gtVAddr traverseAddr, gtVAddr addrRet, int stackOffset = -1, int frameOffset = -1);

    bool TraverseCallStack(gtUInt64 ip,
                           gtUInt64 bp,
                           gtUInt64 sp,
                           int baseOffset,
                           const gtUInt32* pValues,
                           const gtUInt16* pOffsets,
                           unsigned& size,
                           CallStackBuilder& builder,
                           bool is64Bit);

    void Clear();

    unsigned GetOrder() const;


    typedef FlowGraph_DelegateIterator<CallSite, BaseGraph::const_node_iterator, BaseGraph::node_iterator, true>  const_site_iterator;
    typedef FlowGraph_DelegateIterator<CallSite, BaseGraph::const_node_iterator, BaseGraph::node_iterator, false>       site_iterator;

    const_site_iterator GetBeginCallSite() const { return m_graph.GetBeginNode(); }
    site_iterator GetBeginCallSite()       { return m_graph.GetBeginNode(); }
    const_site_iterator GetEndCallSite() const { return m_graph.GetEndNode(); }
    site_iterator GetEndCallSite()       { return m_graph.GetEndNode(); }


    typedef FlowGraph_DelegateIterator<CallStack*, BaseGraph::const_path_iterator, BaseGraph::path_iterator, true>  const_stack_iterator;
    typedef FlowGraph_DelegateIterator<CallStack*, BaseGraph::const_path_iterator, BaseGraph::path_iterator, false>       stack_iterator;

    const_stack_iterator GetBeginCallStack() const { return m_graph.GetBeginPath(); }
    stack_iterator GetBeginCallStack()       { return m_graph.GetBeginPath(); }
    const_stack_iterator GetEndCallStack() const { return m_graph.GetEndPath(); }
    stack_iterator GetEndCallStack()       { return m_graph.GetEndPath(); }
};

#endif // _CALLGRAPH_H_
