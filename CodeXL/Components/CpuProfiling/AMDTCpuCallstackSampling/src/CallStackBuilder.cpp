//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallStackBuilder.cpp
///
//==================================================================================

#include <CallStackBuilder.h>
#include <CallGraph.h>

CallStackBuilder::CallStackBuilder(CallGraph& callGraph, gtUByte* pBuffer, unsigned size, unsigned initDepth) : m_callGraph(callGraph)
{
    GT_UNREFERENCED_PARAMETER(size);

    m_ppStackSites = reinterpret_cast<CallSite**>(pBuffer);
    m_depth = initDepth;
}

void CallStackBuilder::Initialize(gtUInt64 ip, gtUInt64 bp, gtUInt64 sp)
{
    m_ppStackSites[m_depth++] = m_callGraph.AcquireCallSite(ip);
    m_frameAddr = bp;
    m_stackAddr = sp;
}

bool CallStackBuilder::PushLinked(CallSite* pSite)
{
    GT_ASSERT(m_depth > 0);
    CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
    m_ppStackSites[m_depth++] = pSite;
    m_frameAddr += static_cast<gtVAddr>(pPrevSite->m_frameOffset);
    m_stackAddr += static_cast<gtVAddr>(pPrevSite->m_stackOffset);
    return true;
}

bool CallStackBuilder::PushLinked(CallSite* pSite, gtUInt64 bp, gtUInt64 sp)
{
    bool ret = UpdateOffsets(bp, sp);
    m_ppStackSites[m_depth++] = pSite;
    return ret;
}

bool CallStackBuilder::Push(CallSite* pSite)
{
    GT_ASSERT(m_depth > 0);
    CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
    m_ppStackSites[m_depth++] = pSite;
    pPrevSite->m_parents.AddUnique(*pSite);
    m_frameAddr += static_cast<gtVAddr>(pPrevSite->m_frameOffset);
    m_stackAddr += static_cast<gtVAddr>(pPrevSite->m_stackOffset);
    return true;
}

bool CallStackBuilder::Push(CallSite* pSite, gtUInt64 bp, gtUInt64 sp)
{
    bool ret = UpdateOffsets(bp, sp);
    GT_ASSERT(m_depth > 0);
    CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
    pPrevSite->m_parents.AddUnique(*pSite);
    m_ppStackSites[m_depth++] = pSite;
    return ret;
}

bool CallStackBuilder::Push(gtUInt64 ip)
{
    CallSite* pSite = m_callGraph.AcquireCallSite(ip);
    GT_ASSERT(m_depth > 0);
    CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
    pPrevSite->m_parents.AddUnique(*pSite);
    m_ppStackSites[m_depth++] = pSite;
    return true;
}

bool CallStackBuilder::Push(gtUInt64 ip, gtUInt64 bp, gtUInt64 sp)
{
    bool ret = UpdateOffsets(bp, sp);
    CallSite* pSite = m_callGraph.AcquireCallSite(ip);
    GT_ASSERT(m_depth > 0);
    CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
    pPrevSite->m_parents.AddUnique(*pSite);
    m_ppStackSites[m_depth++] = pSite;
    return ret;
}

bool CallStackBuilder::UpdateOffsets(gtUInt64 bp, gtUInt64 sp)
{
    bool ret = true;
    GT_ASSERT(m_depth > 0);
    CallSite* pLastSite = m_ppStackSites[m_depth - 1U];
    int stackOffset = static_cast<int>(sp - m_stackAddr);

    if (0 > pLastSite->m_stackOffset)
    {
        pLastSite->m_stackOffset = stackOffset;
        pLastSite->m_frameOffset = static_cast<int>(bp - m_frameAddr);
    }
    else if (stackOffset != pLastSite->m_stackOffset)
    {
        ret = false;
    }

    m_frameAddr = bp;
    m_stackAddr = sp;
    return ret;
}

bool CallStackBuilder::Push(CallStackBuilder& builder, unsigned offset)
{
    if (offset < builder.m_depth)
    {
        unsigned additionalDepth = (builder.m_depth - offset);
        memcpy(&m_ppStackSites[m_depth], &builder.m_ppStackSites[offset], sizeof(CallSite*) * additionalDepth);

        GT_ASSERT(m_depth > 0);
        CallSite* pSite = m_ppStackSites[m_depth];
        CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
        pPrevSite->m_parents.AddUnique(*pSite);
        m_depth += additionalDepth;

        //TODO: Update offsets!
    }

    return true;
}

bool CallStackBuilder::Push(CallStack& callStack, unsigned offset)
{
    unsigned additionalDepth = callStack.CopySites(&m_ppStackSites[m_depth], offset);

    if (0U != additionalDepth)
    {
        GT_ASSERT(m_depth > 0);
        CallSite* pSite = m_ppStackSites[m_depth];
        CallSite* pPrevSite = m_ppStackSites[m_depth - 1U];
        pPrevSite->m_parents.AddUnique(*pSite);
        m_depth += additionalDepth;

        //TODO: Update offsets!
    }

    return true;
}

CallSite* CallStackBuilder::GetSiteAt(unsigned pos) const
{
    return m_ppStackSites[pos];
}

unsigned CallStackBuilder::GetDepth() const
{
    return m_depth;
}

bool CallStackBuilder::IsEmpty() const
{
    return 0U == m_depth;
}

unsigned CallStackBuilder::CalcRequiredBufferSize(unsigned depth)
{
    return (depth + 1U) * sizeof(CallSite*) + sizeof(CallStack);
}

gtUByte* CallStackBuilder::GetBuffer() const
{
    return reinterpret_cast<gtUByte*>(m_ppStackSites);
}

gtVAddr CallStackBuilder::GetTopTraverseAddress() const
{
    GT_ASSERT(m_depth > 0);
    return m_ppStackSites[m_depth - 1U]->m_traverseAddr;
}

gtVAddr CallStackBuilder::GetTopFrameAddress() const
{
    return m_frameAddr;
}

gtVAddr CallStackBuilder::GetTopStackAddress() const
{
    return m_stackAddr;
}

CallStack* CallStackBuilder::Finalize(unsigned* pCallStackIndex)
{
    unsigned callStackIndex;
    CallStack* pCallStack = &m_callGraph.AddCallStack(m_ppStackSites, m_depth, callStackIndex);

    if (NULL != pCallStackIndex)
    {
        *pCallStackIndex = callStackIndex;
    }

    return pCallStack;
}

CallStack* CallStackBuilder::Finalize(EventSampleInfo& eventSample)
{
    if (0U != m_depth)
    {
        eventSample.m_pSite->m_parents.AddUnique(*m_ppStackSites[0U]);
    }

    unsigned callStackIndex;
    return &m_callGraph.AddCallStackSample(m_ppStackSites, m_depth, eventSample, callStackIndex);
}
