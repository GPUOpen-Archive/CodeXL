//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FlowGraphTraits.h
///
//==================================================================================

#ifndef _FLOWGRAPHTRAITS_H_
#define _FLOWGRAPHTRAITS_H_

enum FlowGraphDirection
{
    FLOW_GRAPH_UPSTREAM   = 1 << 0,
    FLOW_GRAPH_DOWNSTREAM = 1 << 1,
    FLOW_GRAPH_FULLSTREAM = FLOW_GRAPH_UPSTREAM | FLOW_GRAPH_DOWNSTREAM
};

enum FlowGraphPathOptimization
{
    FLOW_GRAPH_PATH_FAST_ITERATION,
    FLOW_GRAPH_PATH_FAST_INSERTION
};

#endif // _FLOWGRAPHTRAITS_H_
