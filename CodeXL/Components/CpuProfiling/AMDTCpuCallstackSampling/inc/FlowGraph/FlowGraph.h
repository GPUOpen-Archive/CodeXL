//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FlowGraph.h
///
//==================================================================================

#ifndef _FLOWGRAPH_H_
#define _FLOWGRAPH_H_

#include "FlowGraphTraits.h"
#include "FlowGraphNode.h"
#include "FlowGraphPath.h"
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <new>

template <class Ty, class TConstBaseIter, class TBaseIter, bool isConst>
class FlowGraph_DelegateIterator
{
public:
    typedef Ty value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename ConditionalType<isConst, TConstBaseIter, TBaseIter>::type base_iterator;
    typedef typename ConditionalType<isConst, const Ty&, Ty&>::type reference;
    typedef typename ConditionalType<isConst, const Ty*, Ty*>::type pointer;

    FlowGraph_DelegateIterator() {}
    FlowGraph_DelegateIterator(const base_iterator& base) : m_base(base) {}
    FlowGraph_DelegateIterator(const FlowGraph_DelegateIterator<Ty, TConstBaseIter, TBaseIter, false>& i) : m_base(i.m_base) {}

    reference operator*() const
    {
        return reinterpret_cast<reference>(*m_base);
    }

    pointer operator->() const
    {
        return reinterpret_cast<pointer>(&*m_base);
    }

    FlowGraph_DelegateIterator& operator++()
    {
        ++m_base;
        return *this;
    }

    FlowGraph_DelegateIterator operator++(int)
    {
        FlowGraph_DelegateIterator tmp(*this);
        ++*this;
        return tmp;
    }

    friend bool operator==(const FlowGraph_DelegateIterator& itLeft, const FlowGraph_DelegateIterator& itRight)
    {
        return itLeft.m_base == itRight.m_base;
    }

    friend bool operator!=(const FlowGraph_DelegateIterator& itLeft, const FlowGraph_DelegateIterator& itRight)
    {
        return itLeft.m_base != itRight.m_base;
    }

private:
    base_iterator m_base;

    friend class FlowGraph_DelegateIterator<Ty, TConstBaseIter, TBaseIter, false>;
    friend class FlowGraph_DelegateIterator<Ty, TConstBaseIter, TBaseIter, true>;
};

template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT, FlowGraphPathOptimization OPT>
class FlowGraph
{
public:
    typedef FlowGraph_NodeData<TKey, TVal, DIRECT> NodeData;
    typedef FlowGraph_Node<TKey, TVal, DIRECT> Node;
    typedef FlowGraph_NodeList<TKey, TVal, DIRECT> NodeList;
    typedef FlowGraph_Path<TKey, TVal, TData, DIRECT, OPT> Path;
    typedef FlowGraph_PathVector<TKey, TVal, TData, DIRECT, OPT> PathVector;

private:
    typedef gtHashMap<TKey, NodeData> NodeMap;
    typedef typename NodeMap::const_iterator NodeMap_const_iterator;
    typedef typename NodeMap::iterator NodeMap_iterator;
    typedef typename NodeMap::value_type NodeMap_value_type;

    NodeMap m_nodes;

    PathVector m_paths;
    unsigned m_emptyPathIndex;

public:
    FlowGraph() : m_emptyPathIndex(unsigned(-1)) {}
    ~FlowGraph() { Clear(); }

    Path* GetPath(unsigned index) const
    {
        return (index < static_cast<unsigned>(m_paths.size())) ? m_paths.at(index) : NULL;
    }

    Path* GetEmptyPath(unsigned& index) const
    {
        index = m_emptyPathIndex;
        return unsigned(-1) != m_emptyPathIndex ? GetPath(m_emptyPathIndex) : NULL;
    }

    Path& AcquirePath(Node** ppWalkNodes, unsigned length, unsigned& pathIndex)
    {
        unsigned countEntries = Path::OptimizeWalk(ppWalkNodes, length);
        return AcquirePath(length, ppWalkNodes, countEntries, pathIndex);
    }

    Path* FindPath(unsigned length, Node** ppWalk, unsigned countEntries, unsigned& pathIndex)
    {
        Path* pPath;

        if (0U != length)
        {
            pPath = NULL;
            Node* pNode = ppWalk[0];

            for (PathIndexSet::const_iterator it = pNode->m_pathIndices.begin(), itEnd = pNode->m_pathIndices.end(); it != itEnd; ++it)
            {
                Path* pIndexedPath = GetPath(*it);

                if (NULL != pIndexedPath && pIndexedPath->IsEqual(length, ppWalk, countEntries))
                {
                    pPath = pIndexedPath;
                    pathIndex = *it;
                    break;
                }
            }
        }
        else
        {
            pPath = GetEmptyPath(pathIndex);
        }

        return pPath;
    }

    Path& AcquirePath(unsigned length, Node** ppWalk, unsigned countEntries, unsigned& pathIndex)
    {
        Path* pPath;

        if (0U != length)
        {
            pPath = NULL;
            Node* pNode = ppWalk[0];

            for (PathIndexSet::const_iterator it = pNode->m_pathIndices.begin(), itEnd = pNode->m_pathIndices.end(); it != itEnd; ++it)
            {
                Path* pIndexedPath = GetPath(*it);

                if (NULL != pIndexedPath && pIndexedPath->IsEqual(length, ppWalk, countEntries))
                {
                    pPath = pIndexedPath;
                    pathIndex = *it;
                    break;
                }
            }

            if (NULL == pPath)
            {
                gtUByte* pBuffer = new gtUByte[sizeof(Path) + countEntries * sizeof(Node*)];
                pPath = new(pBuffer) Path(length, ppWalk, countEntries);
                pathIndex = RegisterPath(*pPath);
            }
        }
        else
        {
            pPath = &AcquireEmptyPath(pathIndex);
        }

        return *pPath;
    }

    Path* GetEmptyPath(unsigned& pathIndex)
    {
        pathIndex = m_emptyPathIndex;
        return (unsigned(-1) != m_emptyPathIndex) ? GetPath(m_emptyPathIndex) : NULL;
    }

    Path& AcquireEmptyPath(unsigned& pathIndex)
    {
        Path* pPath;

        if (unsigned(-1) == m_emptyPathIndex)
        {
            gtUByte* pBuffer = new gtUByte[sizeof(Path)];
            pPath = new(pBuffer) Path();
            RegisterPath(*pPath);
        }
        else
        {
            pPath = GetPath(m_emptyPathIndex);
        }

        pathIndex = m_emptyPathIndex;
        return *pPath;
    }

    unsigned GetPathCount() const
    {
        return static_cast<unsigned>(m_paths.size());
    }

    // Returns the new Path index
    unsigned RegisterPath(Path& path)
    {
        const unsigned index = static_cast<unsigned>(m_paths.size());
        m_paths.push_back(&path);

        if (0U == path.GetLength())
        {
            m_emptyPathIndex = index;
        }

        for (typename Path::iterator it = path.begin(), itEnd = path.end(); it != itEnd; ++it)
        {
            it->m_pathIndices.Add(index);
        }

        return index;
    }

    void Clear()
    {
        for (typename PathVector::iterator it = m_paths.begin(), itEnd = m_paths.end(); it != itEnd; ++it)
        {
            Path* pPath = *it;

            pPath->~Path();
            gtUByte* pBuffer = reinterpret_cast<gtUByte*>(pPath);
            delete [] pBuffer;
        }

        m_paths.clear();
        m_nodes.clear();
        m_emptyPathIndex = unsigned(-1);
    }

    typedef FlowGraph_DelegateIterator<Node, NodeMap_const_iterator, NodeMap_iterator, true>  const_node_iterator;
    typedef FlowGraph_DelegateIterator<Node, NodeMap_const_iterator, NodeMap_iterator, false>       node_iterator;

    const_node_iterator GetBeginNode() const { return m_nodes.begin(); }
    node_iterator GetBeginNode()       { return m_nodes.begin(); }
    const_node_iterator GetEndNode() const { return m_nodes.end(); }
    node_iterator GetEndNode()       { return m_nodes.end(); }

    const_node_iterator FindNode(const TKey& key) const { return m_nodes.find(key); }
    node_iterator FindNode(const TKey& key)       { return m_nodes.find(key); }

    node_iterator InsertNode(const TKey& key)
    {
        return m_nodes.insert(NodeMap_value_type(key, NodeData())).first;
    }

    node_iterator AcquireNode(const TKey& key)
    {
        node_iterator it = m_nodes.find(key);

        if (m_nodes.end() == it)
        {
            it = m_nodes.insert(NodeMap_value_type(key, NodeData())).first;
        }

        return it;
    }

    unsigned GetOrder() const { return static_cast<unsigned>(m_nodes.size()); }


    typedef typename PathVector::const_iterator const_path_iterator;
    typedef typename PathVector::iterator             path_iterator;

    const_path_iterator GetBeginPath() const { return m_paths.begin(); }
    path_iterator GetBeginPath()       { return m_paths.begin(); }
    const_path_iterator GetEndPath() const { return m_paths.end(); }
    path_iterator GetEndPath()       { return m_paths.end(); }
};

#endif // _FLOWGRAPH_H_
