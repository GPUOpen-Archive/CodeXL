//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackWalkContextX64.h
///
//==================================================================================

#ifndef _STACKWALKCONTEXTX64_H_
#define _STACKWALKCONTEXTX64_H_

#include "../StackWalkContext.h"

#define REG_INDEX_RAX 0
#define REG_INDEX_RCX 1
#define REG_INDEX_RDX 2
#define REG_INDEX_RBX 3
#define REG_INDEX_RSP 4
#define REG_INDEX_RBP 5
#define REG_INDEX_RSI 6
#define REG_INDEX_RDI 7

#define REG_INDEX_R8  8
#define REG_INDEX_R9  9
#define REG_INDEX_R10 10
#define REG_INDEX_R11 11
#define REG_INDEX_R12 12
#define REG_INDEX_R13 13
#define REG_INDEX_R14 14
#define REG_INDEX_R15 15

#define REG_INDEX_RIP 16

#define NUM_X64_REGS  17


typedef gtUInt32 RVAddrX64;
typedef gtUInt64 VAddrX64;
typedef gtUInt64 ValueX64;

#define X64_REG_SIZE  (sizeof(ValueX64))
#define X64_STACK_SIZE  X64_REG_SIZE


struct AMD64_RELOCATED_PDATA_ENTRY
{
    VAddrX64 BeginAddress;
    VAddrX64 EndAddress;
    VAddrX64 UnwindInfoAddress;
};

#define RUNTIME_FUNCTION_INDIRECT 0x1

class StackWalkContextX64 : public StackWalkContext<ValueX64, NUM_X64_REGS>
{
    typedef StackWalkContext<ValueX64, NUM_X64_REGS> Parent;

public:
    StackWalkContextX64();

    HRESULT LookupPdataEntry(VAddrX64 virtualAddr, AMD64_RELOCATED_PDATA_ENTRY& pdataEntry) const;

    void Clear(VAddrX64 controlPc, VAddrX64 stackPtr, VAddrX64 framePtr);
    void Extract(StackFrameControlData& frameData);
};

#endif // _STACKWALKCONTEXTX64_H_
