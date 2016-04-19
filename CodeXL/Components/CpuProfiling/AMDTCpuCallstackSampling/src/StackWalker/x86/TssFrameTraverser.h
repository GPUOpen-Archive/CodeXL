//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TssFrameTraverser.h
///
//==================================================================================

#ifndef _TSSFRAMETRAVERSER_H_
#define _TSSFRAMETRAVERSER_H_

#include "StackWalkContextX86.h"

// Kernel Trap frame - TSS (Task State Segment).
class TssFrameTraverser
{
public:
    TssFrameTraverser(StackWalkContextX86& context);
    ~TssFrameTraverser();
    TssFrameTraverser& operator=(const TssFrameTraverser&) = delete;

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);

    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    HRESULT GetSegmentDescriptor(struct _X86_DESCRIPTOR_TABLE_ENTRY& descTableEntry);

    StackWalkContextX86& m_context;
    struct IDiaFrameData* m_pFrameData;
    VAddrX86 m_stackPtr;
};

#endif // _TSSFRAMETRAVERSER_H_
