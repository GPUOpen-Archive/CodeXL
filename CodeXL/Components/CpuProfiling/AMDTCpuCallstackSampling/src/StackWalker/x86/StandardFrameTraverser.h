//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StandardFrameTraverser.h
///
//==================================================================================

#ifndef _STANDARDFRAMETRAVERSER_H_
#define _STANDARDFRAMETRAVERSER_H_

#include "StackFrameTraverser.h"

// Standard EBP stackframe.
class StandardFrameTraverser
{
public:
    StandardFrameTraverser(FrameChainWalker& walker);

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

    static HRESULT IsStandardFrame(IDiaFrameData* pFrameData);

private:
    StackFrameTraverser m_baseTrav;
};

class StandardFrameNoEbpTraverser
{
public:
    StandardFrameNoEbpTraverser(FrameChainWalker& walker);

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    StackFrameTraverser m_baseTrav;
};

#endif // _STANDARDFRAMETRAVERSER_H_
