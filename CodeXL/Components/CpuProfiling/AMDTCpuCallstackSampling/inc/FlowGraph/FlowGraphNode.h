//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FlowGraphNode.h
///
//==================================================================================

#ifndef _FLOWGRAPHNODE_H_
#define _FLOWGRAPHNODE_H_

#include <AMDTBaseTools/Include/gtFlatSet.h>
#include <AMDTBaseTools/Include/gtSmallSList.h>

class PathIndexSet : public gtFlatSet<unsigned>
{
public:
    void Add(unsigned index)
    {
        insert(index);
    }
};

template <class TKey, class TVal, FlowGraphDirection DIRECT>
struct FlowGraph_Node;

template <class TKey, class TVal, FlowGraphDirection DIRECT>
class FlowGraph_NodeList;

template <class TKey, class TVal, FlowGraphDirection DIRECT>
struct FlowGraph_NodeData;

template <class TKey, class TVal>
struct FlowGraph_NodeData<TKey, TVal, FLOW_GRAPH_UPSTREAM>
{
    typedef FlowGraph_NodeList<TKey, TVal, FLOW_GRAPH_UPSTREAM> NodeList;

    TVal m_val;
    NodeList m_parents;
    PathIndexSet m_pathIndices;

    FlowGraph_Node<TKey, TVal, FLOW_GRAPH_UPSTREAM>* SimpleWalkNext() { return *m_parents.begin(); }

    void AddWalkNext(FlowGraph_Node<TKey, TVal, FLOW_GRAPH_UPSTREAM>* pNode)
    {
        pNode->m_parents.AddUnique(*static_cast<FlowGraph_Node<TKey, TVal, FLOW_GRAPH_UPSTREAM>*>(this));
    }
};

template <class TKey, class TVal>
struct FlowGraph_NodeData<TKey, TVal, FLOW_GRAPH_DOWNSTREAM>
{
    typedef FlowGraph_NodeList<TKey, TVal, FLOW_GRAPH_DOWNSTREAM> NodeList;

    TVal m_val;
    NodeList m_children;
    PathIndexSet m_pathIndices;

    FlowGraph_Node<TKey, TVal, FLOW_GRAPH_DOWNSTREAM>* SimpleWalkNext() { return *m_children.begin(); }

    void AddWalkNext(FlowGraph_Node<TKey, TVal, FLOW_GRAPH_DOWNSTREAM>* pNode)
    {
        m_children.AddUnique(*pNode);
    }
};

template <class TKey, class TVal>
struct FlowGraph_NodeData<TKey, TVal, FLOW_GRAPH_FULLSTREAM>
{
    typedef FlowGraph_NodeList<TKey, TVal, FLOW_GRAPH_FULLSTREAM> NodeList;

    TVal m_val;
    NodeList m_parents;
    NodeList m_children;
    PathIndexSet m_pathIndices;

    FlowGraph_Node<TKey, TVal, FLOW_GRAPH_FULLSTREAM>* SimpleWalkNext() { return *m_children.begin(); }

    void AddWalkNext(FlowGraph_Node<TKey, TVal, FLOW_GRAPH_FULLSTREAM>* pNode)
    {
        pNode->m_parents.AddUnique(*static_cast<FlowGraph_Node<TKey, TVal, FLOW_GRAPH_FULLSTREAM>*>(this));
        m_children.AddUnique(*pNode);
    }
};

template <class TKey>
struct FlowGraph_NodeKey
{
    const TKey m_key;
    FlowGraph_NodeKey() = delete;
    FlowGraph_NodeKey& operator=(const FlowGraph_NodeKey&) = delete;
};

template <class TKey, class TVal, FlowGraphDirection DIRECT>
struct FlowGraph_Node : public FlowGraph_NodeKey<TKey>, public FlowGraph_NodeData<TKey, TVal, DIRECT>
{
    bool operator==(const FlowGraph_Node& right) const { return this->m_key == right.m_key; }
    bool operator!=(const FlowGraph_Node& right) const { return this->m_key != right.m_key; }
    bool operator< (const FlowGraph_Node& right) const { return this->m_key <  right.m_key; }
    bool operator> (const FlowGraph_Node& right) const { return this->m_key >  right.m_key; }
    bool operator<=(const FlowGraph_Node& right) const { return this->m_key <= right.m_key; }
    bool operator>=(const FlowGraph_Node& right) const { return this->m_key >= right.m_key; }
};

template <class TKey, class TVal, FlowGraphDirection DIRECT>
class FlowGraph_NodeList : public gtSmallSList<FlowGraph_Node<TKey, TVal, DIRECT>*>
{
public:
    typedef typename gtSmallSList<FlowGraph_Node<TKey, TVal, DIRECT>*>::const_iterator const_iterator;
    typedef typename gtSmallSList<FlowGraph_Node<TKey, TVal, DIRECT>*>::iterator             iterator;

    bool AddUnique(FlowGraph_Node<TKey, TVal, DIRECT>& node)
    {
        bool ret = (Find(node.m_key) == this->end());

        if (ret)
        {
            this->InsertAfterHead(&node);
        }

        return ret;
    }

    iterator Find(const TKey& key)
    {
        iterator it = this->begin();

        for (iterator itEnd = this->end(); it != itEnd; ++it)
        {
            if (key == (*it)->m_key)
            {
                break;
            }
        }

        return it;
    }

    const_iterator Find(const TKey& key) const
    {
        const_iterator it = this->begin();

        for (const_iterator itEnd = this->end(); it != itEnd; ++it)
        {
            if (key == (*it)->m_key)
            {
                break;
            }
        }

        return it;
    }
};

#endif // _FLOWGRAPHNODE_H_
