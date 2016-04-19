//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackVirtualUnwinder.h
///
//==================================================================================

#ifndef _STACKVIRTUALUNWINDER_H_
#define _STACKVIRTUALUNWINDER_H_

#include "StackWalkContextX64.h"

class StackVirtualUnwinder
{
public:
    StackVirtualUnwinder();

    void Reset(ProcessWorkingSetQuery& workingSet, VirtualStack& stack, VAddrX64 controlPc, VAddrX64 stackPtr, VAddrX64 framePtr);

    HRESULT TraverseNext();
    HRESULT GetFrameData(StackFrameData& frameData) const;

    const StackWalkContextX64& GetContext() const { return m_context; }
    StackWalkContextX64& GetContext()       { return m_context; }

private:
    StackWalkContextX64 m_context;
    gtUByte m_prologSize;
    gtUByte m_prologOffset;


    bool PopRegister(unsigned index);
    void AdjustStack(unsigned index, gtInt64 bytesCount);
    void RecoverNonVolatileRegisters();

    ValueX64 GetFrameValue(struct _AMD64_UNWIND_INFO* pUnwindInfo, bool a3);
    HRESULT GetPrimaryEntry(VAddrX64 controlPc, AMD64_RELOCATED_PDATA_ENTRY& funcEntry);

    HRESULT ProcessLeafFrame();
    HRESULT GetUnwindInfo(AMD64_RELOCATED_PDATA_ENTRY& funcEntry, struct _AMD64_UNWIND_INFO* pUnwindInfo, union _AMD64_UNWIND_CODE* pUnwindCode);
    HRESULT Unwind(AMD64_RELOCATED_PDATA_ENTRY* pFuncEntry, struct _AMD64_UNWIND_INFO* pUnwindInfo, union _AMD64_UNWIND_CODE* pUnwindCode);
    HRESULT UnwindEpilog(VAddrX64 controlPc, RVAddrX64 epilogOffset);
    bool UnwindByData(VAddrX64 controlPc, unsigned index, union _AMD64_UNWIND_CODE* pUnwindCode, struct _AMD64_UNWIND_INFO* pUnwindInfo);
    HRESULT UnwindByDisasm(const gtUByte* pCode, unsigned codeLength, struct _AMD64_UNWIND_INFO* pUnwindInfo);
};

#endif // _STACKVIRTUALUNWINDER_H_
