//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackFrameTraverser.h
///
//==================================================================================

#ifndef _STACKFRAMETRAVERSER_H_
#define _STACKFRAMETRAVERSER_H_

#include "StackWalkContextX86.h"

class FrameChainWalker;

class StackFrameTraverser
{
public:
    StackFrameTraverser(FrameChainWalker& walker);
    ~StackFrameTraverser();
    StackFrameTraverser& operator=(const StackFrameTraverser&) = delete;

    void Reset(const wchar_t* pFrameCommand, struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    void ResetFrameCommand(const wchar_t* pFrameCommand) { m_pFrameCommand = pFrameCommand; }

    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

    static HRESULT GetFrameData(const StackWalkContextX86& context, VAddrX86 stackPtr, struct IDiaFrameData* pFrameData, StackFrameData& frameData);

    struct IDiaFrameData* GetFrameInterface() { return m_pFrameData; }

    const FrameChainWalker& GetWalker() const { return m_walker; }
    FrameChainWalker& GetWalker()       { return m_walker; }

    const StackWalkContextX86& GetContext() const;
    StackWalkContextX86& GetContext();

private:
    FrameChainWalker& m_walker;
    const wchar_t* m_pFrameCommand;
    struct IDiaFrameData* m_pFrameData;
    VAddrX86 m_stackPtr;
    bool m_needEvaluation;
};

#endif // _STACKFRAMETRAVERSER_H_
