//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file KiUserExceptionDispatcherFrameTraverser.h
///
//==================================================================================

#ifndef _KIUSEREXCEPTIONDISPATCHERFRAMETRAVERSER_H_
#define _KIUSEREXCEPTIONDISPATCHERFRAMETRAVERSER_H_

#include "StackWalkContextX86.h"

class FrameChainWalker;

class KiUserExceptionDispatcherFrameTraverser
{
public:
    KiUserExceptionDispatcherFrameTraverser(FrameChainWalker& walker);
    ~KiUserExceptionDispatcherFrameTraverser();
    KiUserExceptionDispatcherFrameTraverser& operator=(const KiUserExceptionDispatcherFrameTraverser&) = delete;

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    FrameChainWalker& m_walker;
    struct IDiaFrameData* m_pFrameData;
    VAddrX86 m_stackPtr;
};

#endif // _KIUSEREXCEPTIONDISPATCHERFRAMETRAVERSER_H_
