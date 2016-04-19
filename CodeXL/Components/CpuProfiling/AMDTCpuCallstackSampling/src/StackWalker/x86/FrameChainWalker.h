//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FrameChainWalker.h
///
//==================================================================================

#ifndef _FRAMECHAINWALKER_H_
#define _FRAMECHAINWALKER_H_

#include <AMDTBaseTools/Include/gtVector.h>
#include "StackWalkContextX86.h"
#include "TssFrameTraverser.h"
#include "TrapFrameTraverser.h"
#include "StandardFrameTraverser.h"
#include "PrologFrameTraverser.h"
#include "FrameDataFrameTraverser.h"
#include "FpoFrameTraverser.h"
#include "KiUserExceptionDispatcherFrameTraverser.h"
#include "UnknownFrameTraverser.h"

class FrameChainWalker
{
public:
    FrameChainWalker();

    void Reset(ProcessWorkingSetQuery& workingSet, VirtualStack& stack, VAddrX86 controlPc, VAddrX86 stackPtr, VAddrX86 framePtr);

    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData);

    HRESULT SearchForReturnAddress(struct IDiaFrameData* pFrame, VAddrX86& retAddrPtr);
    HRESULT SearchForReturnAddressStart(struct IDiaFrameData* pFrame, VAddrX86 startAddress, VAddrX86& retAddrPtr);

    bool IsContextRecordRestored() const { return m_restoredContextRecord; }
    void SetContextRecordRestored(bool isRestored) { m_restoredContextRecord = isRestored; }

    const StackWalkContextX86& GetContext() const { return m_context; }
    StackWalkContextX86& GetContext()       { return m_context; }

private:
    struct IDiaFrameData* GetFrameDataByVA(VAddrX86 virtualAddress);
    bool GenerateFakeFrame(VAddrX86 virtualAddress, class CDiaFrameData& frameData);
    HRESULT TraverseFrame(struct IDiaFrameData* pFrameData);

    bool CalcCbLocalsForFunc(VAddrX86 codeAddr, RVAddrX86& lengthLocals);
    HRESULT IsIpInFunctionProlog(unsigned type, struct IDiaFrameData* pFrameData, struct IDiaFrameData** ppFrameProlog);
    bool SetReturnAddress();

    HRESULT SearchForReturnAddressInternal(struct IDiaFrameData* pFrame, VAddrX86 stackPtr, VAddrX86 framePtr, bool hasFramePtr, VAddrX86& retAddrPtr);
    HRESULT SearchForReturnAddressInternal(VAddrX86 stackPtr, VAddrX86 virtualAddress, RVAddrX86 length, VAddrX86& retAddrPtr, bool estimated);

    bool CheckCallTarget(VAddrX86 addr1, VAddrX86 addr2, VAddrX86 addr3, RVAddrX86 length);


    unsigned ExtractIndirectCallTargets(const gtUByte* pCode, unsigned len, VAddrX86 codeAddr, VAddrX86 stackPtr, VAddrX86* pTarget) const;
    unsigned ExtractJumpTarget(const gtUByte* pCode, VAddrX86 codeAddr, VAddrX86& targetAddr) const;
    unsigned ExtractSubEsp(const gtUByte* pCode, gtInt32& subtrahend) const;

    HRESULT ChaseAndValidateJumpChain(VAddrX86 endAddr, RVAddrX86 endBlockLen, VAddrX86 startAddr, bool estimated, VAddrX86& jumpTargetAddr);
    HRESULT ChaseAndValidateJumpChain(VAddrX86 endAddr, RVAddrX86 endBlockLen, VAddrX86 startAddr, bool estimated, VAddrX86* pJumpTargetAddr, unsigned depth);
    gtUByte* getCodeBuffer(size_t requestedSize);
    enum {DEFAULT_CODE_BUFFER_SIZE = 10240};
    gtVector<gtUByte> m_codeBuffer;
    size_t m_codeBufferSize;

    enum
    {
        MAX_SEARCH_DEPTH = 512,
        MAX_JUMP_CHAIN_DEPTH = 64
    };

    StackWalkContextX86 m_context;
    unsigned m_searchDepth;
    unsigned m_sectionIndex;
    RVAddrX86 m_addrOffset;

    VAddrX86 m_jumpChain[MAX_JUMP_CHAIN_DEPTH];
    VAddrX86 m_vaImageStart;

    enum FrameType
    {
        FRAME_TYPE_UNKNOWN,
        FRAME_TYPE_FPO,
        FRAME_TYPE_STANDARD,
        FRAME_TYPE_STANDARD_NO_EBP,
        FRAME_TYPE_FRAME_DATA,
        FRAME_TYPE_PROLOG,
        FRAME_TYPE_KIUSEREXCEPTIONDISPATCHER,
        FRAME_TYPE_TRAP,
        FRAME_TYPE_TSS,

        NUM_FRAME_TYPES,

        FRAME_TYPE_INVALID = NUM_FRAME_TYPES
    };

    FrameType m_frameType;
    unsigned m_lengthParams;

    UnknownFrameTraverser m_frameTravUnknown;
    FpoFrameTraverser m_frameTravFpo;
    StandardFrameTraverser m_frameTravStandard;
    StandardFrameNoEbpTraverser m_frameTravStandardNoEbp;
    FrameDataFrameTraverser m_frameTravFrameData;
    PrologFrameTraverser m_frameTravProlog;
    KiUserExceptionDispatcherFrameTraverser m_frameTravKiUserExceptionDispatcher;
    TrapFrameTraverser m_frameTravTrap;
    TssFrameTraverser m_frameTravTss;

    bool m_restoredContextRecord;
};


#endif // _FRAMECHAINWALKER_H_
