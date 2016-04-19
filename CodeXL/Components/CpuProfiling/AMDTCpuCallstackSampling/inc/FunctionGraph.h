//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FunctionGraph.h
///
//==================================================================================

#ifndef _FUNCTIONGRAPH_H_
#define _FUNCTIONGRAPH_H_

#include "CpuCallstackSamplingDLLBuild.h"
#include "FlowGraph/FlowGraph.h"

struct LeafFunction;

typedef gtSmallSList<LeafFunction> LeafFunctionList;

class CP_CSS_API FunctionGraph : public FlowGraph<gtVAddr, void*, LeafFunctionList, FLOW_GRAPH_FULLSTREAM, FLOW_GRAPH_PATH_FAST_ITERATION>
{
public:
    unsigned AddFunctionPath(Node** ppPathNodes, unsigned length, const LeafFunction& leaf);

    // Order by pNode::m_key -> eventId
    static void AddLeafNode(Path& path, const LeafFunction& leaf);
};

struct LeafFunction
{
    FunctionGraph::Node* m_pNode;
    gtUInt32 m_eventId; // EventMaskType
    gtUInt64 m_count;

    LeafFunction() : m_pNode(NULL) {}
};

class CP_CSS_API FunctionPathBuilder
{
public:
    FunctionPathBuilder(gtUByte* pBuffer, unsigned size, unsigned initLength = 0U);

    void Clear();
    void Initialize(FunctionGraph::Node& node);
    void Push(FunctionGraph::Node& node);

    FunctionGraph::Node* GetNodeAt(unsigned pos) const;

    static unsigned CalcRequiredBufferSize(unsigned depth);
    gtUByte* GetBuffer() const;

    unsigned GetLength() const;

    unsigned Finalize(FunctionGraph& funcGraph, const LeafFunction& leaf);

private:
    FunctionGraph::Node** m_ppNodes;
    unsigned m_length;
};

template <>
inline bool gtSmallSList_node<FunctionGraph::Node*>::IsAnchor() const
{
    return NULL == m_val;
}

template <>
inline bool gtSmallSList_node<LeafFunction>::IsAnchor() const
{
    return NULL == m_val.m_pNode;
}

#endif // _FUNCTIONGRAPH_H_
