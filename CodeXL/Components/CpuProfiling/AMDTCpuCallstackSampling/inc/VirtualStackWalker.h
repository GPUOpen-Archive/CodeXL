//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file VirtualStackWalker.h
///
//==================================================================================

#ifndef _VIRTUALSTACKWALKER_H_
#define _VIRTUALSTACKWALKER_H_

#include "CpuCallstackSamplingDLLBuild.h"
#include "VirtualStack.h"

class CP_CSS_API VirtualStackWalker
{
public:
    VirtualStackWalker();
    ~VirtualStackWalker();

    VirtualStack& GetVirtualStack() { return m_stack; }
    bool Reset(gtVAddr ip, gtVAddr bp, gtVAddr sp, gtUInt32* pValues, gtUInt16* pOffsets, unsigned count, ProcessWorkingSetQuery& workingSet);
    bool Reset(gtVAddr ip, gtVAddr bp, gtVAddr sp, gtUInt32* pValues, gtUInt16* pOffsets, unsigned count, ProcessWorkingSetQuery& workingSet, unsigned ptrSize);

    unsigned BackTrace(gtUByte* pBuffer, unsigned bufferSize, gtUInt32* pFrameWalk, unsigned walkLength);

    unsigned BackTrace(gtUByte* pBuffer, unsigned bufferSize)
    {
        return BackTrace(pBuffer, bufferSize, NULL, 0U);
    }

private:
    class FrameChainWalker* m_pWalkerX86;
    class StackVirtualUnwinder* m_pWalkerX64;
    VirtualStack m_stack;
};

#endif // _VIRTUALSTACKWALKER_H_
