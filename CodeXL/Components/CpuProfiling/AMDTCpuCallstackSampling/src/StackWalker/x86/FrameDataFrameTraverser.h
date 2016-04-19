//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FrameDataFrameTraverser.h
///
//==================================================================================

#ifndef _FRAMEDATAFRAMETRAVERSER_H_
#define _FRAMEDATAFRAMETRAVERSER_H_

#include "StackFrameTraverser.h"

// Frame pointer omitted, FrameData info available.
class FrameDataFrameTraverser
{
public:
    FrameDataFrameTraverser(FrameChainWalker& walker);

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    StackFrameTraverser m_baseTrav;
};

#endif // _FRAMEDATAFRAMETRAVERSER_H_
