//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file UnknownFrameTraverser.h
///
//==================================================================================

#ifndef _UNKNOWNFRAMETRAVERSER_H_
#define _UNKNOWNFRAMETRAVERSER_H_

#include "StackFrameTraverser.h"

// Frame which does not have any debug info.
class UnknownFrameTraverser
{
public:
    UnknownFrameTraverser(FrameChainWalker& walker);

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr, VAddrX86 virtualAddress);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    StackFrameTraverser m_baseTrav;
    VAddrX86 m_virtualAddress;
    ValueX86 m_esp;
};

#endif // _UNKNOWNFRAMETRAVERSER_H_
