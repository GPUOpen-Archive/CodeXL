//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallGraph.cpp
///
//==================================================================================

#include <CallGraph.h>
#include <CallStack.h>
#include <CallStackBuilder.h>

CallStack* CallGraph::GetCallStack(unsigned index) const
{
    return reinterpret_cast<CallStack*>(m_graph.GetPath(index));
}

CallStack* CallGraph::GetEmptyCallStack(unsigned& index) const
{
    return reinterpret_cast<CallStack*>(m_graph.GetEmptyPath(index));
}

CallStack& CallGraph::AddCallStack(CallSite** ppPathSites, unsigned length, unsigned& index)
{
    return reinterpret_cast<CallStack&>(m_graph.AcquirePath(reinterpret_cast<BaseGraph::Node**>(ppPathSites), length, index));
}

CallStack& CallGraph::AddCallStackSample(CallSite** ppPathSites, unsigned length, const EventSampleInfo& eventSample, unsigned& index)
{
    CallStack& stack = reinterpret_cast<CallStack&>(m_graph.AcquirePath(reinterpret_cast<BaseGraph::Node**>(ppPathSites), length, index));
    stack.AddEventSample(eventSample);
    eventSample.m_pSite->m_callStackIndices.Add(index);
    return stack;
}

unsigned CallGraph::GetCallStacksCount() const
{
    return m_graph.GetPathCount();
}

CallSite* CallGraph::AcquireCallSite(gtVAddr traverseAddr)
{
    return &*site_iterator(m_graph.AcquireNode(traverseAddr));
}

CallSite* CallGraph::AcquireCallSite(gtVAddr traverseAddr, int stackOffset, int frameOffset)
{
    CallSite* pSite;
    site_iterator it = m_graph.FindNode(traverseAddr);

    if (GetEndCallSite() == it)
    {
        it = m_graph.InsertNode(traverseAddr);
        pSite = &*it;
        pSite->m_stackOffset = stackOffset;
        pSite->m_frameOffset = frameOffset;
    }
    else
    {
        pSite = &*it;

        if (stackOffset != pSite->m_stackOffset)
        {
            pSite = NULL;
        }
    }

    return pSite;
}

bool CallGraph::InsertCallSite(gtVAddr traverseAddr, gtVAddr addrRet, int stackOffset, int frameOffset)
{
    bool inserted = false;
    site_iterator itRet = m_graph.FindNode(addrRet);

    if (GetEndCallSite() != itRet)
    {
        CallSite* pSite = AcquireCallSite(traverseAddr, stackOffset, frameOffset);

        if (NULL != pSite)
        {
            pSite->m_parents.AddUnique(*itRet);
            inserted = true;
        }
    }

    return inserted;
}

const CallSite* CallGraph::FindCallSite(gtVAddr traverseAddr) const
{
    const_site_iterator it = m_graph.FindNode(traverseAddr);
    return (GetEndCallSite() != it) ? &*it : NULL;
}

CallSite* CallGraph::FindCallSite(gtVAddr traverseAddr)
{
    site_iterator it = m_graph.FindNode(traverseAddr);
    return (GetEndCallSite() != it) ? &*it : NULL;
}

template <typename TValue>
static CallSite* FindNextCallSite(CallSite* pSite,
                                  const gtUInt32* pValues,
                                  const gtUInt16* pOffsets,
                                  unsigned size,
                                  int baseOffset,
                                  unsigned& pos)
{
    CallSite* pNextSite = NULL;

    if (!pSite->m_parents.IsEmpty())
    {
        const gtUInt16 nextOffset = static_cast<gtUInt16>((pSite->m_stackOffset + baseOffset) / sizeof(gtUInt32));
        bool is64bits = (sizeof(gtUInt64) == sizeof(TValue));

        if (is64bits)
        {
            size--;
        }

        for (unsigned i = 0U; i < size; ++i)
        {
            if (nextOffset < pOffsets[i])
            {
                break;
            }

            if (nextOffset == pOffsets[i])
            {
                TValue value = 0;
                bool valid = false;

                if (is64bits)
                {
                    // For 64-bit we must have 2 32-bit consecutive values.
                    valid = ((nextOffset + 1U) == pOffsets[i + 1U]);

                    if (valid)
                    {
                        value = static_cast<gtUInt64>(pValues[i + 0U]) | (static_cast<gtUInt64>(pValues[i + 1U]) << 32);
                    }
                }
                else
                {
                    valid = true;
                    value = pValues[i];
                }

                if (valid)
                {
                    CallSiteList::iterator s = pSite->m_parents.Find(static_cast<gtVAddr>(value));

                    if (pSite->m_parents.end() != s)
                    {
                        pos = i;
                        pNextSite = *s;
                    }
                }

                break;
            }
        }
    }

    return pNextSite;
}

template <typename TValue>
static bool TraverseCallStack(gtUInt64 ip,
                              gtUInt64 bp,
                              gtUInt64 sp,
                              int baseOffset,
                              CallSite* pSite,
                              const gtUInt32* pValues,
                              const gtUInt16* pOffsets,
                              unsigned& size,
                              CallStackBuilder& builder)
{
    unsigned pos = 0U;
    int prevOffset = baseOffset;

    CallSite* pNextSite = FindNextCallSite<TValue>(pSite, pValues, pOffsets, size, prevOffset, pos);
    bool ret = (NULL != pNextSite);

    if (ret)
    {
        if (builder.IsEmpty())
        {
            builder.Initialize(ip, bp, sp);
        }
        else
        {
            builder.Push(ip, bp, sp);
        }

        do
        {
            builder.PushLinked(pNextSite);

            prevOffset = pOffsets[pos];

            ++pos;
            pValues += pos;
            pOffsets += pos;
            size -= pos;

            pSite = pNextSite;
            pNextSite = FindNextCallSite<TValue>(pSite, pValues, pOffsets, size, prevOffset, pos);
        }
        while (NULL != pNextSite);
    }

    return ret;
}

bool CallGraph::TraverseCallStack(gtUInt64 ip,
                                  gtUInt64 bp,
                                  gtUInt64 sp,
                                  int baseOffset,
                                  const gtUInt32* pValues,
                                  const gtUInt16* pOffsets,
                                  unsigned& size,
                                  CallStackBuilder& builder,
                                  bool is64Bit)
{
    bool ret = false;
    CallGraph::site_iterator it = m_graph.FindNode(ip);

    if (GetEndCallSite() != it)
    {
        CallSite* pSite = &*it;

        if (is64Bit)
        {
            ret = ::TraverseCallStack<gtUInt64>(ip, bp, sp, baseOffset, pSite, pValues, pOffsets, size, builder);
        }
        else
        {
            ret = ::TraverseCallStack<gtUInt32>(ip, bp, sp, baseOffset, pSite, pValues, pOffsets, size, builder);
        }
    }

    return ret;
}

CallGraph::~CallGraph()
{
    Clear();
}

void CallGraph::Clear()
{
    m_graph.Clear();
}

unsigned CallGraph::GetOrder() const
{
    return m_graph.GetOrder();
}
//
// void CallGraph::Insert(const CallGraph& other, gtUByte* pBuffer, unsigned bufferSize)
// {
//     //
//     // A CallGraph is actually a collection of CallStacks. Therefore we can just copy the CallStacks and the corresponding CallSites
//     // will be copied along the way.
//     //
//
//     CallStackBuilder builder(*this, pBuffer, bufferSize);
//
//     for (const_stack_iterator itStack = other.GetBeginCallStack(), itStackEnd = other.GetEndCallStack(); itStack != itStackEnd; ++itStack)
//     {
//         CallStack* pOtherStack = *itStack;
//         CallStack::const_iterator itSite = pOtherStack->begin(), itSiteEnd = pOtherStack->end();
//
//         const CallSite* pOtherSite = NULL;
//
//         if (pOtherStack->GetEventSampleList().IsEmpty())
//         {
//             if (itSite != itSiteEnd)
//             {
//                 pOtherSite = &*itSite;
//                 ++itSite;
//             }
//         }
//         else
//         {
//             EventSampleList::const_iterator itSample = pOtherStack->GetEventSampleList().begin();
//             pOtherSite = itSample->m_pSite;
//         }
//
//         if (NULL != pOtherSite)
//         {
//             CallSite* pPrevSite = AcquireCallSite(pOtherSite->m_traverseAddr);
//
//             for (; itSite != itSiteEnd; ++itSite)
//             {
//                 pOtherSite = &*itSite;
//                 CallSite* pThisSite = AcquireCallSite(pOtherSite->m_traverseAddr);
//
//                 pPrevSite->m_parents.AddUnique(*pThisSite);
//                 pPrevSite = pThisSite;
//             }
//
//             if (!pOtherStack->GetEventSampleList().IsEmpty())
//             {
//                 EventSampleList::const_iterator itSample    = pOtherStack->GetEventSampleList().begin(),
//                                                 itSampleEnd = pOtherStack->GetEventSampleList().end();
//
//                 while (++itSample != itSampleEnd)
//                 {
//                     pOtherSite = itSample->m_pSite;
//                     CallSite* pThisSite = AcquireCallSite(pOtherSite->m_traverseAddr);
//                 }
//             }
//         }
//         CallStackBuilder
//         if (0U != m_depth)
//         {
//             CallSite* pSite = GetSampleSite();
//
//             unsigned callStackIndex;
//             pCallStack = &m_callGraph.AddCallStack(m_ppStackSites + 1, m_depth - 1U, callStackIndex);
//             pSite->m_callStackIndices.Add(callStackIndex);
//         }
//     }
// }
