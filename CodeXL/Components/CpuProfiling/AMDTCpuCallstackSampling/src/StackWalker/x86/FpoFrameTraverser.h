//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FpoFrameTraverser.h
///
//==================================================================================

#ifndef _FPOFRAMETRAVERSER_H_
#define _FPOFRAMETRAVERSER_H_

#include "StackFrameTraverser.h"

// Frame pointer omitted, FPO info available.
class FpoFrameTraverser
{
public:
    FpoFrameTraverser(FrameChainWalker& walker);

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData);

private:
    StackFrameTraverser m_baseTrav;
};

#endif // _FPOFRAMETRAVERSER_H_
