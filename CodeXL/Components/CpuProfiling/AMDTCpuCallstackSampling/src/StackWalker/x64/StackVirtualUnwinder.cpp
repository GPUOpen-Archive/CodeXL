//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackVirtualUnwinder.cpp
///
//==================================================================================

#include "StackVirtualUnwinder.h"
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

//
// Define unwind operation codes.
//

typedef enum _UNWIND_OP_CODES
{
    UWOP_PUSH_NONVOL = 0,
    UWOP_ALLOC_LARGE,
    UWOP_ALLOC_SMALL,
    UWOP_SET_FPREG,
    UWOP_SAVE_NONVOL,
    UWOP_SAVE_NONVOL_FAR,
    UWOP_EPILOG,
    UWOP_SPARE_CODE2,
    UWOP_SAVE_XMM128,
    UWOP_SAVE_XMM128_FAR,
    UWOP_PUSH_MACHFRAME
} UNWIND_OP_CODES, *PUNWIND_OP_CODES;

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    #pragma warning(disable : 4201) // nameless struct/union
#endif
//
// Define unwind code structure.
//
typedef union _AMD64_UNWIND_CODE
{
    struct
    {
        UCHAR CodeOffset;
        UCHAR UnwindOp : 4;
        UCHAR OpInfo : 4;
    };

    USHORT FrameOffset;
} AMD64_UNWIND_CODE, *PAMD64_UNWIND_CODE;

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif
//
// Define unwind information flags.
//

#define UNW_FLAG_NHANDLER 0x0
#define UNW_FLAG_EHANDLER 0x1
#define UNW_FLAG_UHANDLER 0x2
#define UNW_FLAG_CHAININFO 0x4

//
// Define unwind information structure.
//

typedef struct _AMD64_UNWIND_INFO
{
    UCHAR Version : 3;
    UCHAR Flags : 5;
    UCHAR SizeOfProlog;
    UCHAR CountOfCodes;
    UCHAR FrameRegister : 4;
    UCHAR FrameOffset : 4;
    //AMD64_UNWIND_CODE UnwindCode[1];

    //
    // The unwind codes are followed by an optional ULONG aligned field that
    // contains the exception handler address or a function table entry if
    // chained unwind information is specified. If an exception handler address
    // is specified, then it is followed by the language specified exception
    // handler data.
    //
    //  union {
    //      struct {
    //          ULONG ExceptionHandler;
    //          ULONG ExceptionData[];
    //      };
    //
    //      RUNTIME_FUNCTION FunctionEntry;
    //  };
    //

} AMD64_UNWIND_INFO, *PAMD64_UNWIND_INFO;

//
// Define function table entry - a function table entry is generated for
// each frame function.
//

typedef struct _AMD64_RUNTIME_FUNCTION
{
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} AMD64_RUNTIME_FUNCTION, *PAMD64_RUNTIME_FUNCTION;


// AMD_UNWIND_INFO::CountOfCodes is a byte.
#define MAX_UNWIND_CODE_LENGTH  (0xFF + 1)
#define MAX_UNWIND_CODE_BUFFER_SIZE  (MAX_UNWIND_CODE_LENGTH + (sizeof(AMD64_RUNTIME_FUNCTION) / sizeof(AMD64_UNWIND_INFO)))

#define SIZE64_PREFIX 0x48
#define ADD_IMM8_OP 0x83
#define ADD_IMM32_OP 0x81
#define JMP_IMM8_OP 0xEB
#define JMP_IMM32_OP 0xE9
#define JMP_IND_OP 0xFF
#define LEA_OP 0x8D
#define REP_PREFIX 0xF3
#define POP_OP 0x58
#define RET_OP 0xC3
#define RET_OP_2 0xC2

#define IS_REX_PREFIX(x) (((x) & 0xF0) == 0x40)


static unsigned GetUnwindCodeSlotsCount(AMD64_UNWIND_CODE unwindCode);


StackVirtualUnwinder::StackVirtualUnwinder() : m_prologSize(0), m_prologOffset(0)
{
}

void StackVirtualUnwinder::Reset(ProcessWorkingSetQuery& workingSet, VirtualStack& stack, VAddrX64 controlPc, VAddrX64 stackPtr, VAddrX64 framePtr)
{
    m_context.Clear(controlPc, stackPtr, framePtr);
    m_context.SetWorkingSet(&workingSet);
    m_context.SetVirtualStack(&stack);

    m_prologSize = 0;
    m_prologOffset = 0;
}

HRESULT StackVirtualUnwinder::GetFrameData(StackFrameData& frameData) const
{
    frameData.m_returnAddress = m_context.GetRegister(REG_INDEX_RIP);
    frameData.m_valid.returnAddress = TRUE;

    frameData.m_base = m_context.GetRegister(REG_INDEX_RSP) - X64_STACK_SIZE;
    frameData.m_valid.base = TRUE;
    return S_OK;
}

bool StackVirtualUnwinder::PopRegister(unsigned index)
{
    VAddrX64 stackPtr = m_context.GetRegister(REG_INDEX_RSP);

    ValueX64 stackVal = 0;
    bool ret = m_context.ReadFullMemory(MEM_TYPE_STACK, stackPtr, stackVal);

    if (ret)
    {
        m_context.SetRegister(REG_INDEX_RSP, stackPtr + X64_STACK_SIZE);
        m_context.SetRegister(index, stackVal);
    }

    return ret;
}

void StackVirtualUnwinder::AdjustStack(unsigned index, gtInt64 bytesCount)
{
    m_context.SetRegister(REG_INDEX_RSP, m_context.GetRegister(index) + bytesCount);
}

void StackVirtualUnwinder::RecoverNonVolatileRegisters()
{
    m_context.RecoverRegister(REG_INDEX_RDI);
    m_context.RecoverRegister(REG_INDEX_RSI);
    m_context.RecoverRegister(REG_INDEX_RBX);
    m_context.RecoverRegister(REG_INDEX_RBP);
    m_context.RecoverRegister(REG_INDEX_R12);
    m_context.RecoverRegister(REG_INDEX_R13);
    m_context.RecoverRegister(REG_INDEX_R14);
    m_context.RecoverRegister(REG_INDEX_R15);
}

HRESULT StackVirtualUnwinder::ProcessLeafFrame()
{
    HRESULT hr;

    if (PopRegister(REG_INDEX_RIP))
    {
        RecoverNonVolatileRegisters();
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

HRESULT StackVirtualUnwinder::TraverseNext()
{
    HRESULT hr;
    VAddrX64 controlPc = m_context.GetRegister(REG_INDEX_RIP);
    VAddrX64 stackPtr;

    if (m_context.GetRegister(REG_INDEX_RSP, stackPtr))
    {
        AMD64_RELOCATED_PDATA_ENTRY funcEntry;
        hr = m_context.LookupPdataEntry(controlPc, funcEntry);

        if (S_OK == hr)
        {
            AMD64_UNWIND_INFO unwindInfo;
            AMD64_UNWIND_CODE aUnwindCode[MAX_UNWIND_CODE_BUFFER_SIZE];
            hr = GetUnwindInfo(funcEntry, &unwindInfo, aUnwindCode);

            if (S_OK == hr)
            {
                hr = Unwind(&funcEntry, &unwindInfo, aUnwindCode);

                if (SUCCEEDED(hr))
                {
                    RecoverNonVolatileRegisters();
                }
            }
        }
        else if (S_FALSE == hr)
        {
            hr = ProcessLeafFrame();
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT StackVirtualUnwinder::GetUnwindInfo(AMD64_RELOCATED_PDATA_ENTRY& funcEntry, AMD64_UNWIND_INFO* pUnwindInfo, AMD64_UNWIND_CODE* pUnwindCode)
{
    HRESULT hr = E_ACCESSDENIED;

    if (m_context.ReadFullMemory(MEM_TYPE_CODE, funcEntry.UnwindInfoAddress, *pUnwindInfo))
    {
        // We currently support only versions 1 and 2.
        if (1 == pUnwindInfo->Version || 2 == pUnwindInfo->Version)
        {
            VAddrX64 unwindCodeAddr = funcEntry.UnwindInfoAddress + sizeof(AMD64_UNWIND_INFO);
            int lenUnwindCode = sizeof(AMD64_UNWIND_CODE) * static_cast<unsigned>(pUnwindInfo->CountOfCodes);

            if (m_context.ReadFullMemory(MEM_TYPE_CODE, unwindCodeAddr, pUnwindCode, lenUnwindCode))
            {
                //
                // If the UNW_FLAG_CHAININFO flag is set, then an unwind info structure is a secondary one,
                // and the shared exception-handler/chained-info address field contains the primary unwind information.
                //

                if (0 != (pUnwindInfo->Flags & UNW_FLAG_CHAININFO))
                {
                    // The UNWIND_INFO structure must be DWORD aligned in memory.
                    RVAddrX64 offset = sizeof(AMD64_UNWIND_CODE) * ((pUnwindInfo->CountOfCodes + 1) & ~1);
                    unwindCodeAddr += offset;

                    AMD64_RUNTIME_FUNCTION* pPimaryFuncEntry = reinterpret_cast<AMD64_RUNTIME_FUNCTION*>(reinterpret_cast<gtUByte*>(pUnwindCode) + offset);

                    if (m_context.ReadFullMemory(MEM_TYPE_CODE, unwindCodeAddr, *pPimaryFuncEntry))
                    {
                        hr = S_OK;
                    }
                }
                else
                {
                    hr = S_OK;
                }
            }
        }
        else
        {
            hr = E_NOTIMPL;
        }
    }

    return hr;
}

ValueX64 StackVirtualUnwinder::GetFrameValue(AMD64_UNWIND_INFO* pUnwindInfo, bool a3)
{
    ValueX64 establisherFrame;

    //
    // If the specified function uses a frame pointer and control left the function outside of the prologue or
    // the unwind information contains a chained information structure, then the establisher frame is the
    // contents of the frame pointer.
    //
    if ((0 != pUnwindInfo->FrameRegister) &&
        (!a3 || 0 != (pUnwindInfo->Flags & UNW_FLAG_CHAININFO) || m_prologOffset >= m_prologSize))
    {
        establisherFrame = m_context.GetRegister(pUnwindInfo->FrameRegister);
        establisherFrame -= pUnwindInfo->FrameOffset * (2 * X64_STACK_SIZE);
    }
    else
    {
        establisherFrame = m_context.GetRegister(REG_INDEX_RSP);
    }

    return establisherFrame;
}

HRESULT StackVirtualUnwinder::GetPrimaryEntry(VAddrX64 controlPc, AMD64_RELOCATED_PDATA_ENTRY& funcEntry)
{
    HRESULT hr = m_context.LookupPdataEntry(controlPc, funcEntry);

    if (S_OK == hr)
    {
        VAddrX64 imageLoadAddr = m_context.GetImageLoadAddress(controlPc);

        AMD64_UNWIND_INFO unwindInfo;
        AMD64_UNWIND_CODE aUnwindCode[MAX_UNWIND_CODE_BUFFER_SIZE];

        while (S_OK == (hr = GetUnwindInfo(funcEntry, &unwindInfo, aUnwindCode)) && 0 != (unwindInfo.Flags & UNW_FLAG_CHAININFO))
        {
            AMD64_RUNTIME_FUNCTION* pPimaryFuncEntry = reinterpret_cast<AMD64_RUNTIME_FUNCTION*>(&aUnwindCode[unwindInfo.CountOfCodes]);
            funcEntry.BeginAddress      = imageLoadAddr + pPimaryFuncEntry->BeginAddress;
            funcEntry.EndAddress        = imageLoadAddr + pPimaryFuncEntry->EndAddress;
            funcEntry.UnwindInfoAddress = imageLoadAddr + pPimaryFuncEntry->UnwindData;
        }
    }

    return hr;
}

HRESULT StackVirtualUnwinder::Unwind(AMD64_RELOCATED_PDATA_ENTRY* pFuncEntry, AMD64_UNWIND_INFO* pUnwindInfo, AMD64_UNWIND_CODE* pUnwindCode)
{
    HRESULT hr = S_OK;

    VAddrX64 controlPc = m_context.GetRegister(REG_INDEX_RIP);

    if (0 == (pUnwindInfo->Flags & UNW_FLAG_CHAININFO))
    {
        m_context.SetRegister(REG_INDEX_RIP, 0);
    }

    m_prologSize = pUnwindInfo->SizeOfProlog;
    m_prologOffset = static_cast<gtUByte>(controlPc - pFuncEntry->BeginAddress);

    bool inEpilogue = false;
    unsigned firstIndex = 0;

    // If control left the function from within the prologue.
    if (pFuncEntry->BeginAddress <= controlPc && pUnwindInfo->SizeOfProlog > m_prologOffset)
    {
        firstIndex = static_cast<unsigned int>(~0);
        unsigned index = 0;

        for (unsigned countOfCodes = pUnwindInfo->CountOfCodes; index < countOfCodes; index += GetUnwindCodeSlotsCount(pUnwindCode[index]))
        {
            AMD64_UNWIND_CODE unwindCode = pUnwindCode[index];

            if (UWOP_EPILOG != unwindCode.UnwindOp)
            {
                // If we have found the first relevant unwind code.
                // * Note that the correctness of this assumption is based on the descending order of unwind codes.
                if (m_prologOffset >= unwindCode.CodeOffset && ~0 == firstIndex)
                {
                    firstIndex = index;
                }

                if (UWOP_SET_FPREG == unwindCode.UnwindOp)
                {
                    m_prologSize = unwindCode.CodeOffset;
                }
            }
        }

        if (~0 == firstIndex)
        {
            firstIndex = index;
        }
    }
    else if (1 == pUnwindInfo->Version)
    {
        gtUByte aCode[40];
        int codeLength = m_context.ReadMemory(MEM_TYPE_ANY, controlPc, aCode);

        if (0 >= codeLength)
        {
            return S_OK;
        }

        if (codeLength != sizeof(aCode))
        {
            memset(aCode, 0, sizeof(aCode) - codeLength);
        }

        const gtUByte* pCode = aCode;

        //
        // Check for one of:
        //
        //   add rsp, imm8
        //       or
        //   add rsp, imm32
        //       or
        //   lea rsp, -disp8[fp]
        //       or
        //   lea rsp, -disp32[fp]
        //

        if ((pCode[0] == SIZE64_PREFIX) &&
            (pCode[1] == ADD_IMM8_OP) &&
            (pCode[2] == 0xC4))
        {
            //
            // add rsp, imm8.
            //

            pCode += 4;
        }
        else if ((pCode[0] == SIZE64_PREFIX) &&
                 (pCode[1] == ADD_IMM32_OP) &&
                 (pCode[2] == 0xC4))
        {
            //
            // add rsp, imm32.
            //

            pCode += 7;
        }
        else if (((pCode[0] & 0xFE) == SIZE64_PREFIX) && (pCode[1] == LEA_OP))
        {
            unsigned frameRegister = ((pCode[0] & 0x1) << 3) | (pCode[2] & 0x7);

            if ((frameRegister != 0) && (frameRegister == pUnwindInfo->FrameRegister))
            {
                if ((pCode[2] & 0xF8) == 0x60)
                {
                    //
                    // lea rsp, disp8[fp].
                    //

                    pCode += 4;
                }
                else if ((pCode[2] & 0xF8) == 0xA0)
                {
                    //
                    // lea rsp, disp32[fp].
                    //

                    pCode += 7;
                }
            }
        }


        //
        // Check for any number of:
        //
        //   pop nonvolatile-integer-register[0..15].
        //

        for (;;)
        {
            if ((pCode[0] & 0xF8) == POP_OP)
            {
                pCode += 1;
            }
            else if (IS_REX_PREFIX(pCode[0]) && ((pCode[1] & 0xF8) == POP_OP))
            {
                pCode += 2;
            }
            else
            {
                break;
            }
        }

        //
        // If the next instruction is a return or an appropriate jump, then
        // control is currently in an epilogue and execution of the epilogue
        // should be emulated. Otherwise, execution is not in an epilogue and
        // the prologue should be unwound.
        //

        if ((pCode[0] == RET_OP) ||
            (pCode[0] == RET_OP_2) ||
            ((pCode[0] == REP_PREFIX) && (pCode[1] == RET_OP)))
        {
            //
            // A return is an unambiguous indication of an epilogue.
            //

            inEpilogue = true;
        }
        else if ((pCode[0] == JMP_IMM8_OP) || (pCode[0] == JMP_IMM32_OP))
        {
            //
            // An unconditional branch to a target that is equal to the start of
            // or outside of this routine is logically a call to another function.
            //

            VAddrX64 branchTarget = controlPc + (pCode - aCode);

            if (pCode[0] == JMP_IMM8_OP)
            {
                branchTarget += 2 + static_cast<gtByte>(pCode[1]);
            }
            else
            {
                branchTarget += 5 + *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 1);
            }


            //
            // Determine whether the branch target refers to code within this
            // function. If not, then it is an epilogue indicator.
            //
            // A branch to the start of self implies a recursive call, so
            // is treated as an epilogue.
            //

            if (branchTarget < pFuncEntry->BeginAddress || branchTarget >= pFuncEntry->EndAddress)
            {
                //
                // The branch target is outside of the region described by this function entry.
                // See whether it is contained within an indirect function entry associated with this same function.
                //
                // If not, then the branch target really is outside of this function.
                //

                AMD64_RELOCATED_PDATA_ENTRY primaryFunctionEntry;
                HRESULT hr2 = GetPrimaryEntry(controlPc, primaryFunctionEntry);

                if (S_OK == hr2)
                {
                    if (branchTarget == primaryFunctionEntry.BeginAddress)
                    {
                        inEpilogue = true;
                    }
                    else
                    {
                        AMD64_RELOCATED_PDATA_ENTRY branchPrimaryFunctionEntry;
                        hr2 = GetPrimaryEntry(branchTarget, branchPrimaryFunctionEntry);

                        if (S_OK == hr2)
                        {
                            if (branchPrimaryFunctionEntry.BeginAddress != primaryFunctionEntry.BeginAddress)
                            {
                                inEpilogue = true;
                            }
                        }
                    }
                }
            }
            else if ((branchTarget == pFuncEntry->BeginAddress) && (0 == (pUnwindInfo->Flags & UNW_FLAG_CHAININFO)))
            {
                inEpilogue = true;
            }
        }
        else if ((pCode[0] == JMP_IND_OP) && (pCode[1] == 0x25))
        {
            //
            // An unconditional jump indirect.
            //
            // This is a jmp outside of the function, probably a tail call
            // to an import function.
            //

            inEpilogue = true;
        }
        else if (((pCode[0] & 0xF8) == SIZE64_PREFIX) &&
                 (pCode[1] == 0xFF) &&
                 (pCode[2] & 0x38) == 0x20)
        {
            //
            // This is an indirect jump opcode: 0x48 0xFF /4.  The 64-bit
            // flag (REX.W) is always redundant here, so its presence is
            // overloaded to indicate a branch out of the function - a tail
            // call.
            //
            // Such an opcode is an unambiguous epilogue indication.
            //

            inEpilogue = true;
        }


        if (inEpilogue)
        {
            hr = UnwindByDisasm(aCode, codeLength, pUnwindInfo);
        }
    }
    else
    {
        if (0 != pUnwindInfo->CountOfCodes)
        {
            if (UWOP_EPILOG == pUnwindCode->UnwindOp)
            {
                RVAddrX64 offset = 0;
                RVAddrX64 epilogEndOffset = pUnwindCode[0].CodeOffset;

                if (0 != (pUnwindCode->OpInfo & 1))
                {
                    VAddrX64 epilogAddr = pFuncEntry->EndAddress - epilogEndOffset;

                    if (controlPc >= epilogAddr)
                    {
                        offset = static_cast<RVAddrX64>(controlPc - epilogAddr);

                        if (offset < epilogEndOffset)
                        {
                            inEpilogue = true;
                        }
                    }
                }

                if (!inEpilogue)
                {
                    for (unsigned index = 1, countOfCodes = pUnwindInfo->CountOfCodes; index < countOfCodes; ++index)
                    {
                        AMD64_UNWIND_CODE unwindCode = pUnwindCode[index];

                        if (UWOP_EPILOG != unwindCode.UnwindOp)
                        {
                            break;
                        }

                        VAddrX64 epilogAddr = pFuncEntry->EndAddress -
                                              (static_cast<RVAddrX64>(unwindCode.CodeOffset) | (static_cast<RVAddrX64>(unwindCode.OpInfo) * 0x100));

                        if (controlPc >= epilogAddr)
                        {
                            offset = static_cast<RVAddrX64>(controlPc - epilogAddr);

                            if (offset < epilogEndOffset)
                            {
                                inEpilogue = true;
                                break;
                            }
                        }
                    }
                }

                if (inEpilogue)
                {
                    hr = UnwindEpilog(controlPc, offset);
                }
            }
        }
    }

    if (!inEpilogue && S_OK == hr)
    {
        hr = UnwindByData(controlPc, firstIndex, pUnwindCode, pUnwindInfo) ? S_OK : E_ACCESSDENIED;
    }

    return hr;
}

HRESULT StackVirtualUnwinder::UnwindEpilog(VAddrX64 controlPc, RVAddrX64 epilogOffset)
{
    AMD64_RELOCATED_PDATA_ENTRY funcEntry;
    AMD64_UNWIND_INFO unwindInfo;
    AMD64_UNWIND_CODE aUnwindCode[MAX_UNWIND_CODE_BUFFER_SIZE];

    HRESULT hr = GetPrimaryEntry(controlPc, funcEntry);

    if (S_OK == hr)
    {
        hr = GetUnwindInfo(funcEntry, &unwindInfo, aUnwindCode);

        if (S_OK == hr)
        {
            const unsigned countOfCodes = unwindInfo.CountOfCodes;
            unsigned index = 0;

            while (index < countOfCodes)
            {
                AMD64_UNWIND_CODE unwindCode = aUnwindCode[index];

                if (UWOP_PUSH_NONVOL == unwindCode.UnwindOp || UWOP_PUSH_MACHFRAME == unwindCode.UnwindOp)
                {
                    break;
                }

                index += GetUnwindCodeSlotsCount(unwindCode);
            }

            unsigned offset = 0;

            while (index < countOfCodes)
            {
                AMD64_UNWIND_CODE unwindCode = aUnwindCode[index];

                //
                // Push nonvolatile integer register.
                //
                // The operation information is the register number of the
                // register than was pushed.
                //

                if (UWOP_PUSH_NONVOL == unwindCode.UnwindOp)
                {
                    if (offset >= epilogOffset)
                    {
                        if (!PopRegister(unwindCode.OpInfo))
                        {
                            return E_ACCESSDENIED;
                        }
                    }

                    offset += (7 < unwindCode.OpInfo) ? 2 : 1;
                    index++;
                }
                else
                {
                    if (UWOP_ALLOC_SMALL == unwindCode.UnwindOp && 0 == unwindCode.OpInfo)
                    {
                        if (offset >= epilogOffset)
                        {
                            AdjustStack(REG_INDEX_RSP, X64_STACK_SIZE);
                        }

                        index++;
                    }

                    break;
                }
            }

            if (index < countOfCodes)
            {
                AMD64_UNWIND_CODE unwindCode = aUnwindCode[index];

                if (UWOP_PUSH_MACHFRAME == unwindCode.UnwindOp)
                {
                    if (PopRegister(REG_INDEX_RIP))
                    {
                        AdjustStack(REG_INDEX_RSP, 2 * X64_STACK_SIZE);

                        if (!PopRegister(REG_INDEX_RSP))
                        {
                            hr = E_ACCESSDENIED;
                        }
                    }
                    else
                    {
                        hr = E_ACCESSDENIED;
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                if (!PopRegister(REG_INDEX_RIP))
                {
                    hr = E_ACCESSDENIED;
                }
            }
        }
    }

    return hr;
}

bool StackVirtualUnwinder::UnwindByData(VAddrX64 controlPc, unsigned index, AMD64_UNWIND_CODE* pUnwindCode, AMD64_UNWIND_INFO* pUnwindInfo)
{
    bool res = true;
    bool isMachineFrame = false;
    bool xxx = 0 != index; //TODO: this should actually be (~0 != index)
    const unsigned countOfCodes = pUnwindInfo->CountOfCodes;

    while (res && index < countOfCodes)
    {
        AMD64_UNWIND_CODE unwindCode = pUnwindCode[index];

        switch (unwindCode.UnwindOp)
        {
            // Push a nonvolatile integer register.
            case UWOP_PUSH_NONVOL:
                // The operation info is the number of the register.
                res = PopRegister(unwindCode.OpInfo);
                index++;
                break;

            // Allocate a large-sized area on the stack.
            case UWOP_ALLOC_LARGE:
                index++;

                // If the operation info equals 0, then the size of the allocation divided by 8 is recorded in the next slot.
                if (0 == unwindCode.OpInfo)
                {
                    AdjustStack(REG_INDEX_RSP, pUnwindCode[index].FrameOffset * X64_STACK_SIZE);
                    index++;
                }
                // If the operation info equals 1, then the unscaled size of the allocation is recorded in the next two slots
                // in little-endian format.
                else if (1 == unwindCode.OpInfo)
                {
                    AdjustStack(REG_INDEX_RSP, static_cast<gtUInt64>(*reinterpret_cast<gtUInt32 UNALIGNED*>(&pUnwindCode[index])));
                    index += 2;
                }
                else
                {
                    res = false;
                }

                break;

            // Allocate a small-sized area on the stack.
            case UWOP_ALLOC_SMALL:
                // The size of the allocation is the operation info field * 8 + 8.
                AdjustStack(REG_INDEX_RSP, (unwindCode.OpInfo * X64_STACK_SIZE) + X64_STACK_SIZE);
                index++;
                break;

            // Establish the frame pointer register.
            case UWOP_SET_FPREG:
                // Set the frame pointer register to some offset of the current RSP.
                // The offset is equal to the Frame Register offset (scaled) field in the UNWIND_INFO * 16.
                // Note that the operation info field is reserved and should not be used.
                AdjustStack(pUnwindInfo->FrameRegister, -(static_cast<gtInt32>(pUnwindInfo->FrameOffset) * static_cast<gtInt32>(2 * X64_STACK_SIZE)));
                index++;
                break;

            // Save a nonvolatile integer register on the stack using a MOV instead of a PUSH.
            case UWOP_SAVE_NONVOL:
                // The operation info is the number of the register.
                // The scaled-by-8 stack offset is recorded in the next unwind operation code slot.
            {
                VAddrX64 frameBase = GetFrameValue(pUnwindInfo, xxx);
                ValueX64 integerRegVal = 0;

                index++;
                res = m_context.ReadFullMemory(MEM_TYPE_ANY, frameBase + (pUnwindCode[index].FrameOffset * X64_STACK_SIZE), integerRegVal);

                if (res)
                {
                    m_context.SetRegister(unwindCode.OpInfo, integerRegVal);
                }

                index++;
                break;
            }

            // Save a nonvolatile integer register on the stack with a long offset, using a MOV instead of a PUSH.
            case UWOP_SAVE_NONVOL_FAR:
                // The operation info is the number of the register.
                // The unscaled stack offset is recorded in the next two unwind operation code slots.
            {
                VAddrX64 frameBase = GetFrameValue(pUnwindInfo, xxx);
                ValueX64 integerRegVal = 0;

                index++;
                res = m_context.ReadFullMemory(MEM_TYPE_ANY, frameBase + *reinterpret_cast<gtUInt32 UNALIGNED*>(&pUnwindCode[index]), integerRegVal);

                if (res)
                {
                    m_context.SetRegister(unwindCode.OpInfo, integerRegVal);
                }

                index += 2;
                break;
            }

            // Spare unused codes.
            case UWOP_EPILOG:
                index++;
                break;

            case UWOP_SPARE_CODE2:
                index += 3;
                break;

            // Save all 128 bits of a nonvolatile XMM register on the stack.
            case UWOP_SAVE_XMM128:
                index += 2;
                break;

            case UWOP_SAVE_XMM128_FAR:
                index += 3;
                break;

            // Push a machine frame on the stack.
            case UWOP_PUSH_MACHFRAME:
                // The operation info determines whether the machine frame contains an error code or not.
                isMachineFrame = true;

                res = false;

                if (PopRegister(REG_INDEX_RIP))
                {
                    if (0 == unwindCode.OpInfo || PopRegister(REG_INDEX_RIP))
                    {
                        AdjustStack(REG_INDEX_RSP, 2 * X64_STACK_SIZE);

                        if (PopRegister(REG_INDEX_RSP))
                        {
                            res = true;
                        }
                    }
                }

                index++;
                break;

            default:
                res = false;
                break;
        }
    }

    if (res)
    {
        if (0 != (pUnwindInfo->Flags & UNW_FLAG_CHAININFO))
        {
            VAddrX64 imageBase = m_context.GetImageLoadAddress(controlPc);

            res = false;

            if (0 != imageBase)
            {
                AMD64_RUNTIME_FUNCTION* pPimaryFuncEntry = reinterpret_cast<AMD64_RUNTIME_FUNCTION*>(&pUnwindCode[countOfCodes]);

                AMD64_RELOCATED_PDATA_ENTRY primaryFuncEntry;
                primaryFuncEntry.BeginAddress      = imageBase + pPimaryFuncEntry->BeginAddress;
                primaryFuncEntry.EndAddress        = imageBase + pPimaryFuncEntry->EndAddress;
                primaryFuncEntry.UnwindInfoAddress = imageBase + pPimaryFuncEntry->UnwindData;

                AMD64_UNWIND_INFO primaryUnwindInfo;
                AMD64_UNWIND_CODE aPrimaryUnwindCode[MAX_UNWIND_CODE_BUFFER_SIZE];

                if (S_OK == GetUnwindInfo(primaryFuncEntry, &primaryUnwindInfo, aPrimaryUnwindCode))
                {
                    if (S_OK == Unwind(&primaryFuncEntry, &primaryUnwindInfo, aPrimaryUnwindCode))
                    {
                        res = true;
                    }
                }
            }
        }
        else if (!isMachineFrame)
        {
            res = PopRegister(REG_INDEX_RIP);
        }
    }

    return res;
}

HRESULT StackVirtualUnwinder::UnwindByDisasm(const BYTE* pCode, unsigned codeLength, AMD64_UNWIND_INFO* pUnwindInfo)
{
    BYTE const* const pCodeEnd = pCode + codeLength;

    //
    // Emulate one of (if any):
    //
    //   add rsp, imm8
    //       or
    //   add rsp, imm32
    //       or
    //   lea rsp, disp8[frame-register]
    //       or
    //   lea rsp, disp32[frame-register]
    //

    if ((pCode[0] == SIZE64_PREFIX) &&
        (pCode[1] == ADD_IMM8_OP) &&
        (pCode[2] == 0xC4))
    {
        //
        // add rsp, imm8.
        //

        AdjustStack(REG_INDEX_RSP, static_cast<gtInt64>(static_cast<gtByte>(pCode[3])));
        pCode += 4;

    }
    else if ((pCode[0] == SIZE64_PREFIX) &&
             (pCode[1] == ADD_IMM32_OP) &&
             (pCode[2] == 0xC4))
    {
        //
        // add rsp, imm32.
        //

        AdjustStack(REG_INDEX_RSP, static_cast<gtInt64>(*reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 3)));
        pCode += 7;
    }
    else if (((pCode[0] & 0xFE) == SIZE64_PREFIX) && (pCode[1] == LEA_OP))
    {
        unsigned frameRegister = ((pCode[0] & 0x1) << 3) | (pCode[2] & 0x7);

        if ((frameRegister != 0) && (frameRegister == pUnwindInfo->FrameRegister))
        {
            if ((pCode[2] & 0xF8) == 0x60)
            {
                //
                // lea rsp, disp8[frame-register].
                //

                AdjustStack(frameRegister, static_cast<gtInt64>(static_cast<gtByte>(pCode[3])));
                pCode += 4;
            }
            else if ((pCode[2] & 0xF8) == 0xA0)
            {
                //
                // lea rsp, disp32[frame-register].
                //

                AdjustStack(frameRegister, static_cast<gtInt64>(*reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 3)));
                pCode += 7;
            }
        }
    }


    //
    // Emulate any number of (if any):
    //
    //   pop nonvolatile-integer-register.
    //

    while (pCode < pCodeEnd)
    {
        if ((pCode[0] & 0xF8) == POP_OP)
        {
            //
            // pop nonvolatile-integer-register[0..7]
            //

            unsigned registerNumber = pCode[0] & 7;
            PopRegister(registerNumber);
            pCode += 1;
        }
        else if (IS_REX_PREFIX(pCode[0]) && ((pCode[1] & 0xF8) == POP_OP))
        {
            //
            // pop nonvolatile-integer-register[8..15]
            //

            unsigned registerNumber = ((pCode[0] & 1) << 3) | (pCode[1] & 0x7);
            PopRegister(registerNumber);
            pCode += 2;
        }
        else
        {
            break;
        }
    }

    //
    // Emulate return and return null exception handler.
    //
    // Note: this instruction might in fact be a jmp, however
    //       we want to emulate a return regardless.
    //

    PopRegister(REG_INDEX_RIP);

    return S_OK;
}

static unsigned GetUnwindCodeSlotsCount(AMD64_UNWIND_CODE unwindCode)
{
    static const gtUByte s_aOpCodeSlot[] =
    {
        1, // UWOP_PUSH_NONVOL      (0)
        2, // UWOP_ALLOC_LARGE      (1)
        1, // UWOP_ALLOC_SMALL      (2)
        1, // UWOP_SET_FPREG        (3)
        2, // UWOP_SAVE_NONVOL      (4)
        3, // UWOP_SAVE_NONVOL_FAR  (5)
        1, // UWOP_SPARE_CODE1      (6)
        3, // UWOP_SPARE_CODE2      (7)
        2, // UWOP_SAVE_XMM128      (8)
        3, // UWOP_SAVE_XMM128_FAR  (9)
        1  // UWOP_PUSH_MACHFRAME   (10)
    };

    unsigned count = s_aOpCodeSlot[unwindCode.UnwindOp];

    // Allocate a large-sized area on the stack.
    if (UWOP_ALLOC_LARGE == unwindCode.UnwindOp)
    {
        // A non-zero operation information indicates that an additional slot is consumed.
        if (0 != unwindCode.OpInfo)
        {
            count++;
        }
    }

    return count;
}
