//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TrapFrameTraverser.h
///
//==================================================================================

#ifndef _TRAPFRAMETRAVERSER_H_
#define _TRAPFRAMETRAVERSER_H_

#include "StackWalkContextX86.h"

// Kernel Trap frame
class TrapFrameTraverser
{
public:
    TrapFrameTraverser(StackWalkContextX86& context);
    ~TrapFrameTraverser();
    TrapFrameTraverser& operator=(const TrapFrameTraverser&) = delete;

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);

    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    StackWalkContextX86& m_context;
    struct IDiaFrameData* m_pFrameData;
    VAddrX86 m_stackPtr;
};

#endif // _TRAPFRAMETRAVERSER_H_
