//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrologFrameTraverser.h
///
//==================================================================================

#ifndef _PROLOGFRAMETRAVERSER_H_
#define _PROLOGFRAMETRAVERSER_H_

#include "StackWalkContextX86.h"

class PrologFrameTraverser
{
public:
    PrologFrameTraverser(StackWalkContextX86& context);
    ~PrologFrameTraverser();
    PrologFrameTraverser& operator=(const PrologFrameTraverser&) = delete;

    void Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr);
    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

private:
    enum { MAX_PROLOG_LEN = 256 };

    StackWalkContextX86& m_context;
    struct IDiaFrameData* m_pFrameData;
    VAddrX86 m_stackPtr;

    gtUByte m_aCode[MAX_PROLOG_LEN];
    int m_codeLength;

    const gtUByte* m_aInst[MAX_PROLOG_LEN];
    unsigned m_instCount;


    HRESULT PopRegister(unsigned index);
    HRESULT AdjustRegister(unsigned index, gtInt32 bytesCount);

    HRESULT ProcessProlog(VAddrX86 frameAddr, VAddrX86 retAddr);
    HRESULT CrackInstructions(VAddrX86 frameAddr, VAddrX86 retAddr);
    HRESULT ProcessInstruction(VAddrX86 instAddr, const gtUByte* pInst);

    HRESULT ProcessPushInstruction(const gtUByte* pCode);
    HRESULT ProcessPopInstruction(const gtUByte* pCode);
    HRESULT ProcessAddSubInstruction(const gtUByte* pCode);
    HRESULT ProcessMovInstruction(const gtUByte* pCode);
    HRESULT ProcessCallInstruction(VAddrX86 codeAddr, const gtUByte* pCode);

    HRESULT ProcessEhProlog();
    HRESULT ProcessEhProlog2();
    HRESULT ProcessEhProlog3();
    HRESULT ProcessEhProlog3Align();
    HRESULT ProcessSehProlog();
};

#endif // _PROLOGFRAMETRAVERSER_H_
