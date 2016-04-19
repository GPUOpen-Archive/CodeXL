//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FunctionGraph.cpp
///
//==================================================================================

#include <FunctionGraph.h>

unsigned FunctionGraph::AddFunctionPath(Node** ppPathNodes, unsigned length, const LeafFunction& leaf)
{
    unsigned index;
    Path& path = AcquirePath(ppPathNodes, length, index);

    AddLeafNode(path, leaf);
    leaf.m_pNode->m_pathIndices.Add(index);

    return index;
}

void FunctionGraph::AddLeafNode(Path& path, const LeafFunction& leaf)
{
    bool aggregated = false;
    LeafFunctionList& leaves = path.GetData();
    LeafFunctionList::iterator it = leaves.begin(), itEnd = leaves.end();
    LeafFunctionList::iterator itPrev = itEnd;

    while (it != itEnd && leaf.m_pNode->m_key > it->m_pNode->m_key)
    {
        itPrev = it++;
    }

    while (it != itEnd && leaf.m_pNode->m_key == it->m_pNode->m_key)
    {
        if (leaf.m_eventId == it->m_eventId)
        {
            it->m_count += leaf.m_count;
            aggregated = true;
            break;
        }

        itPrev = it++;
    }

    if (!aggregated)
    {
        leaves.InsertAfter(itPrev, leaf);
    }
}

FunctionPathBuilder::FunctionPathBuilder(gtUByte* pBuffer, unsigned size, unsigned initLength)
{
    GT_UNREFERENCED_PARAMETER(size);

    m_ppNodes = reinterpret_cast<FunctionGraph::Node**>(pBuffer);
    m_length = initLength;
}

void FunctionPathBuilder::Clear()
{
    m_length = 0U;
}

void FunctionPathBuilder::Initialize(FunctionGraph::Node& node)
{
    m_ppNodes[m_length++] = &node;
}

void FunctionPathBuilder::Push(FunctionGraph::Node& node)
{
    node.AddWalkNext(m_ppNodes[m_length - 1U]);
    m_ppNodes[m_length++] = &node;
}

FunctionGraph::Node* FunctionPathBuilder::GetNodeAt(unsigned pos) const
{
    return m_ppNodes[pos];
}

unsigned FunctionPathBuilder::GetLength() const
{
    return m_length;
}

unsigned FunctionPathBuilder::CalcRequiredBufferSize(unsigned depth)
{
    return (depth + 1U) * sizeof(FunctionGraph::Node*) + sizeof(FunctionGraph::Path);
}

gtUByte* FunctionPathBuilder::GetBuffer() const
{
    return reinterpret_cast<gtUByte*>(m_ppNodes);
}

unsigned FunctionPathBuilder::Finalize(FunctionGraph& funcGraph, const LeafFunction& leaf)
{
    if (0U != m_length)
    {
        m_ppNodes[0U]->AddWalkNext(leaf.m_pNode);
    }

    return funcGraph.AddFunctionPath(m_ppNodes, m_length, leaf);
}
