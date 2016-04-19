//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallStackBuilder.h
///
//==================================================================================

#ifndef _CALLSTACKBUILDER_H_
#define _CALLSTACKBUILDER_H_

#include "CallStack.h"

class CallGraph;

class CP_CSS_API CallStackBuilder
{
public:
    CallStackBuilder(CallGraph& callGraph, gtUByte* pBuffer, unsigned size, unsigned initDepth = 0U);
    CallStackBuilder& operator=(const CallStackBuilder&) = delete;

    void Initialize(gtUInt64 ip, gtUInt64 bp, gtUInt64 sp);
    bool Push(gtUInt64 ip);
    bool Push(gtUInt64 ip, gtUInt64 bp, gtUInt64 sp);
    bool Push(CallSite* pSite);
    bool Push(CallSite* pSite, gtUInt64 bp, gtUInt64 sp);
    bool PushLinked(CallSite* pSite);
    bool PushLinked(CallSite* pSite, gtUInt64 bp, gtUInt64 sp);

    bool Push(CallStackBuilder& builder, unsigned offset = 0U);
    bool Push(CallStack& callStack, unsigned offset = 0U);

    CallSite* GetSiteAt(unsigned pos) const;
    unsigned GetDepth() const;
    bool IsEmpty() const;

    static unsigned CalcRequiredBufferSize(unsigned depth);
    gtUByte* GetBuffer() const;

    const CallGraph& GetCallGraph() const { return m_callGraph; }
    CallGraph& GetCallGraph()       { return m_callGraph; }

    gtVAddr GetTopTraverseAddress() const;
    gtVAddr GetTopFrameAddress() const;
    gtVAddr GetTopStackAddress() const;

    CallStack* Finalize(unsigned* pCallStackIndex = NULL);
    CallStack* Finalize(EventSampleInfo& eventSample);

private:
    CallGraph& m_callGraph;
    CallSite** m_ppStackSites;
    unsigned m_depth;
    gtVAddr m_frameAddr;
    gtVAddr m_stackAddr;

    bool UpdateOffsets(gtUInt64 bp, gtUInt64 sp);
};

#endif // _CALLSTACKBUILDER_H_
