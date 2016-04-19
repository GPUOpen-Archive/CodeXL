//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FlowGraphPath.h
///
//==================================================================================

#ifndef _FLOWGRAPHPATH_H_
#define _FLOWGRAPHPATH_H_

#include <AMDTBaseTools/Include/gtFragmentedVector.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtAssert.h>

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    #pragma warning(disable : 4200 4201) // zero-sized array in struct/union, nameless struct/union
#endif

template <class TKey, class TVal, FlowGraphDirection DIRECT, FlowGraphPathOptimization OPT, bool isConst>
class FlowGraph_PathWalkIterator;

template <class TKey, class TVal, FlowGraphDirection DIRECT, bool isConst>
class FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION, isConst>
{
public:
    typedef FlowGraph_Node<TKey, TVal, DIRECT> Node;

    typedef std::ptrdiff_t difference_type;
    typedef typename ConditionalType<isConst, const Node&, Node&>::type reference;
    typedef typename ConditionalType<isConst, const Node*, Node*>::type pointer;

    FlowGraph_PathWalkIterator() {}
    FlowGraph_PathWalkIterator(const pointer* pWalk) : m_pWalk(pWalk) {}
    FlowGraph_PathWalkIterator(const FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION, false>& i) :
        m_pWalk(i.m_pWalk)
    {}

    reference operator*() const
    {
        return **m_pWalk;
    }

    pointer operator->() const
    {
        return *m_pWalk;
    }

    FlowGraph_PathWalkIterator& operator++()
    {
        ++m_pWalk;
        return *this;
    }

    FlowGraph_PathWalkIterator operator++(int)
    {
        FlowGraph_PathWalkIterator tmp(*this);
        ++*this;
        return tmp;
    }

    FlowGraph_PathWalkIterator& operator--()
    {
        --m_pWalk;
        return *this;
    }

    FlowGraph_PathWalkIterator operator--(int)
    {
        FlowGraph_PathWalkIterator tmp(*this);
        --*this;
        return tmp;
    }

    friend bool operator==(const FlowGraph_PathWalkIterator& itLeft, const FlowGraph_PathWalkIterator& itRight)
    {
        return itLeft.m_pWalk == itRight.m_pWalk;
    }

    friend bool operator!=(const FlowGraph_PathWalkIterator& itLeft, const FlowGraph_PathWalkIterator& itRight)
    {
        return itLeft.m_pWalk != itRight.m_pWalk;
    }

private:
    const pointer* m_pWalk;
};

template <class TKey, class TVal, FlowGraphDirection DIRECT, bool isConst>
class FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION, isConst>
{
public:
    typedef FlowGraph_Node<TKey, TVal, DIRECT> Node;

    typedef std::ptrdiff_t difference_type;
    typedef typename ConditionalType<isConst, const Node&, Node&>::type reference;
    typedef typename ConditionalType<isConst, const Node*, Node*>::type pointer;

private:
    union Entry
    {
        gtIntPtr m_pos;
        pointer m_pNode;

        gtIntPtr GetPos() const
        {
            return (0 > m_pos) ? static_cast<unsigned>(-m_pos) : 0U;
        }
    };

    typedef typename ConditionalType<isConst, const Entry*, Entry*>::type entry_pointer;
    typedef typename ConditionalType<isConst, const void*, void*>::type void_pointer;

    entry_pointer m_pWalk;
    pointer m_pNode;
    gtIntPtr m_pos;
    const gtIntPtr m_posLimit;

public:
    FlowGraph_PathWalkIterator(void_pointer pWalk, pointer pNode, gtIntPtr pos, gtIntPtr posLimit) :
        m_pWalk(static_cast<entry_pointer>(pWalk)), m_pNode(pNode), m_pos(pos), m_posLimit(posLimit)
    {}
    FlowGraph_PathWalkIterator(const FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION, false>& i) :
        m_pWalk((entry_pointer)i.m_pWalk), m_pNode(i.m_pNode), m_pos(i.m_pos), m_posLimit(i.m_posLimit)
    {}

    reference operator*() const
    {
        return *m_pNode;
    }

    pointer operator->() const
    {
        return m_pNode;
    }

    FlowGraph_PathWalkIterator& operator++()
    {
        if (m_posLimit < --m_pos)
        {
            const bool isFork = 0 > m_pWalk->m_pos;

            if (isFork && m_pos != m_pWalk->m_pos)
            {
                m_pNode = *m_pNode->m_parents.begin();
            }
            else
            {
                m_pWalk += static_cast<size_t>(isFork);
                m_pNode = m_pWalk->m_pNode;
                ++m_pWalk;
            }
        }

        return *this;
    }

    FlowGraph_PathWalkIterator operator++(int)
    {
        FlowGraph_PathWalkIterator tmp(*this);
        ++*this;
        return tmp;
    }

    friend bool operator==(const FlowGraph_PathWalkIterator& itLeft, const FlowGraph_PathWalkIterator& itRight)
    {
        return itLeft.m_pos == itRight.m_pos;
    }

    friend bool operator!=(const FlowGraph_PathWalkIterator& itLeft, const FlowGraph_PathWalkIterator& itRight)
    {
        return itLeft.m_pos != itRight.m_pos;
    }
};

template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT, FlowGraphPathOptimization OPT>
class FlowGraph_Path;

template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT>
class FlowGraph_Path<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION>
{
public:
    typedef FlowGraph_Node<TKey, TVal, DIRECT> Node;

private:
    TData m_data;
    unsigned m_length;
    Node* m_walk[0];

public:
    FlowGraph_Path() : m_length(0U) {}

    FlowGraph_Path(unsigned length, Node** ppWalk, unsigned countEntries) : m_length(length)
    {
        if (0U != countEntries)
        {
            memcpy(m_walk, ppWalk, countEntries * sizeof(Node*));
        }
    }

    static unsigned OptimizeWalk(Node** ppWalkNodes, unsigned length)
    {
        GT_UNREFERENCED_PARAMETER(ppWalkNodes);
        return length;
    }

    typedef FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION, true>  const_iterator;
    typedef FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION, false>       iterator;

    const_iterator begin() const { return const_iterator(m_walk); }
    iterator begin()       { return       iterator(m_walk); }
    const_iterator end() const { return const_iterator(m_walk + m_length); }
    iterator end()       { return       iterator(m_walk + m_length); }

    unsigned GetLength() const { return m_length; }

    const TData& GetData() const { return m_data; }
    TData& GetData()       { return m_data; }

    bool IsEqual(unsigned length, Node** ppWalk, unsigned countEntries) const
    {
        return m_length == length && 0 == memcmp(m_walk, ppWalk, countEntries * sizeof(Node*));
    }

    unsigned CopyFullWalk(Node** ppWalk, unsigned offset = 0U) const
    {
        unsigned countEntries = 0U;

        if (offset < m_length)
        {
            countEntries = (m_length - offset);
            memcpy(ppWalk, m_walk + offset, countEntries * sizeof(Node*));
        }

        return countEntries;
    }
};

template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT>
class FlowGraph_Path<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION>
{
public:
    typedef FlowGraph_Node<TKey, TVal, DIRECT> Node;

private:
    union Entry
    {
        gtIntPtr m_pos;
        Node* m_pNode;

        gtIntPtr GetPos() const
        {
            return (0 > m_pos) ? static_cast<unsigned>(-m_pos) : 0U;
        }
    };

    TData m_data;

    union
    {
        struct
        {
            gtUInt16 m_length;
            gtUInt16 m_countEntries;
        };

        unsigned m_hashValue;
    };

    Entry m_walk[0];

public:
    FlowGraph_Path() : m_hashValue(0U) {}

    FlowGraph_Path(unsigned length, Node** ppWalk, unsigned countEntries)
    {
        m_length = static_cast<gtUInt16>(length);
        m_countEntries = static_cast<gtUInt16>(countEntries);

        if (0U != countEntries)
        {
            memcpy(m_walk, ppWalk, countEntries * sizeof(Entry));
        }
    }

    static unsigned OptimizeWalk(Node** ppWalkNodes, unsigned length)
    {
        Entry* pEntry = reinterpret_cast<Entry*>(ppWalkNodes);

        if (0U != length)
        {
            // Always insert the first node
            (pEntry++)->m_pNode = ppWalkNodes[0];

            unsigned forkDistance = 0U;
            const unsigned lastPos = length - 1U;

            for (unsigned i = 1U; i < lastPos; ++i)
            {
                ++forkDistance;

                // Insert only if we have a fork
                if (ppWalkNodes[i] != ppWalkNodes[i - 1U]->SimpleWalkNext())
                {
                    if (1U < forkDistance)
                    {
                        (pEntry++)->m_pos = -static_cast<gtIntPtr>(i);
                    }

                    (pEntry++)->m_pNode = ppWalkNodes[i];
                    forkDistance = 0U;
                }
            }

            // Always insert the last node
            if (1U < forkDistance)
            {
                (pEntry++)->m_pos = -static_cast<gtIntPtr>(lastPos);
            }
            else if (1U == forkDistance)
            {
                (pEntry++)->m_pNode = ppWalkNodes[lastPos - 1U];
            }

            (pEntry++)->m_pNode = ppWalkNodes[lastPos];
        }

        return pEntry - reinterpret_cast<Entry*>(ppWalkNodes);
    }

    typedef FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION, true>  const_iterator;
    typedef FlowGraph_PathWalkIterator<TKey, TVal, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION, false>       iterator;

    const_iterator begin() const
    {
        const Node* pFirstNode = (0 != m_length) ? m_walk[0].m_pNode : NULL;
        return const_iterator(m_walk + 1, pFirstNode, 0, -static_cast<gtIntPtr>(m_length));
    }

    iterator begin()
    {
        Node* pFirstNode = (0 != m_length) ? m_walk[0].m_pNode : NULL;
        return iterator(m_walk + 1, pFirstNode, 0, -static_cast<gtIntPtr>(m_length));
    }

    const_iterator end() const
    {
        return const_iterator(NULL, NULL, -static_cast<gtIntPtr>(m_length), -static_cast<gtIntPtr>(m_length));
    }

    iterator end()
    {
        return iterator(NULL, NULL, -static_cast<gtIntPtr>(m_length), -static_cast<gtIntPtr>(m_length));
    }

    unsigned GetLength() const { return m_length; }

    const TData& GetData() const { return m_data; }
    TData& GetData()       { return m_data; }

    bool IsEqual(unsigned length, Node** ppWalk, unsigned countEntries) const
    {
        union HashValue
        {
            struct
            {
                gtUInt16 m_length;
                gtUInt16 m_countEntries;
            };

            unsigned m_hashValue;
        };

        HashValue walkInfo;
        walkInfo.m_length = static_cast<gtUInt16>(length);
        walkInfo.m_countEntries = static_cast<gtUInt16>(countEntries);
        return m_hashValue == walkInfo.m_hashValue && 0 == memcmp(m_walk, ppWalk, countEntries * sizeof(Entry));
    }

    unsigned CopyFullWalk(Node** ppWalk, unsigned offset = 0U) const
    {
        unsigned countEntries = 0U;

        if (offset < static_cast<unsigned>(m_length))
        {
            const_iterator it = begin(), itEnd = end();

            for (; 0U != offset; --offset)
            {
                ++it;
            }

            for (; it != itEnd; ++it)
            {
                *ppWalk++ = const_cast<Node*>(&*it);
                countEntries++;
            }
        }

        return countEntries;
    }
};


template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT, FlowGraphPathOptimization OPT>
struct FlowGraph_PathVector;

template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT>
class FlowGraph_PathVector<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION> :
    public gtVector<FlowGraph_Path<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION>*>
{
public:
    typedef gtVector<FlowGraph_Path<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_ITERATION>*> BaseVector;

    FlowGraph_PathVector()
    {
        BaseVector::reserve(4096);
    }
};

template <class TKey, class TVal, class TData, FlowGraphDirection DIRECT>
struct FlowGraph_PathVector<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION> :
    public gtFragmentedVector<FlowGraph_Path<TKey, TVal, TData, DIRECT, FLOW_GRAPH_PATH_FAST_INSERTION>*, 2048>
{
};

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif

#endif // _FLOWGRAPHPATH_H_
