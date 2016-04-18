//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackWalkContextX86.h
///
//==================================================================================

#ifndef _STACKWALKCONTEXTX86_H_
#define _STACKWALKCONTEXTX86_H_

#include "../StackWalkContext.h"
#include <bitset>

#define REG_INDEX_EAX 0
#define REG_INDEX_ECX 1
#define REG_INDEX_EDX 2
#define REG_INDEX_EBX 3
#define REG_INDEX_ESP 4
#define REG_INDEX_EBP 5
#define REG_INDEX_ESI 6
#define REG_INDEX_EDI 7
#define REG_INDEX_EIP 8

#define NUM_X86_REGS  9


typedef gtUInt32 RVAddrX86;
typedef gtUInt32 VAddrX86;
typedef gtUInt32 ValueX86;

#define X86_REG_SIZE  (sizeof(ValueX86))
#define X86_STACK_SIZE  X86_REG_SIZE


// The lowest user address reserves the low 64k.
#define MM_LOWEST_USER_ADDRESS 0x10000

// This value is also correct for kernel addresses, as they have a higher limit.
#define MM_LOWEST_VALID_ADDRESS MM_LOWEST_USER_ADDRESS


class StackWalkContextX86 : public StackWalkContext<ValueX86, NUM_X86_REGS>
{
    typedef StackWalkContext<ValueX86, NUM_X86_REGS> Parent;

public:
    StackWalkContextX86();

    ValueX86 GetRegister(unsigned index) const;
    bool GetRegister(unsigned index, ValueX86& val) const;

    void Clear(VAddrX86 controlPc, VAddrX86 stackPtr, VAddrX86 framePtr);
    void Reset();
    void Propagate();

    ValueX86 GetVariable(unsigned index) const;
    bool GetVariable(unsigned index, ValueX86& val) const;
    void SetVariable(unsigned index, ValueX86 val);


    void SetVFrame(ValueX86 val);
    ValueX86 GetVFrame() const;
    bool GetVFrame(ValueX86& val) const;

    void SetParams(ValueX86 val);
    ValueX86 GetParams() const;
    bool GetParams(ValueX86& val) const;

    void SetLocals(ValueX86 val);
    ValueX86 GetLocals() const;
    bool GetLocals(ValueX86& val) const;

    void Extract(StackFrameControlData& frameData);

    bool GetRegisterFailed() const { return m_registerNotFound; }

    HRESULT FindFrameInterface(ValueX86 virtualAddr, struct IDiaFrameData** ppFrame) const;

    enum { MAX_VARIABLES = 0x10000 };

private:
    std::bitset<MAX_VARIABLES> m_validVariables;
    ValueX86 m_aVariables[MAX_VARIABLES];

    enum
    {
        VALID_MASK_VFRAME = 1 << 0,
        VALID_MASK_PARAMS = 1 << 1,
        VALID_MASK_LOCALS = 1 << 2
    };

    ValueX86 m_vframe;
    ValueX86 m_vframePrev;
    ValueX86 m_params;
    ValueX86 m_paramsPrev;
    ValueX86 m_locals;
    ValueX86 m_localsPrev;
    gtUByte m_validExtra;
    gtUByte m_validExtraPrev;

    mutable bool m_registerNotFound;
};

#endif // _STACKWALKCONTEXTX86_H_
