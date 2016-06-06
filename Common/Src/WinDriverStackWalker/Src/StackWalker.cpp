#include "..\Include\StackWalker.hpp"
#include <ntimage.h>


#define MAX_CALL_INST_BYTES  ((ULONG_PTR)8)

/// \def MAX_STACK_DEPTH Arbitrary value of the maximum stack depth of call-stack sampling.
#define MAX_STACK_WALK_DEPTH 2048


static IMAGE_NT_HEADERS* ExtractNtHeader(ULONG_PTR imageLoadAddr, ULONG imageSize);
static ULONG GetContainingStackLimits(ULONG_PTR stackPtr, ULONG_PTR& lowLimit, ULONG_PTR& highLimit, MODE mode);


ModuleInfoList* StackWalker::s_pKernelModules = NULL;
AtomicCounter<ULONG>* StackWalker::s_pGlobalCount = NULL;


StackWalker::StackWalker() : m_processId(NULL), m_refCount(-1L), m_clientsMaxDepth(0UL)
{
}


StackWalker::~StackWalker()
{
    ASSERT(0L > m_refCount);
}


bool StackWalker::Initialize(HANDLE processId, const CSS_CodeRange pRanges[], ULONG countRanges)
{
    bool ret = false;

    LONG refCount = ++m_refCount;

    if (0L == refCount && m_modules.Grow((0UL < countRanges) ? countRanges : 1UL))
    {
        ModuleInfo moduleInfo;

        for (ULONG i = 0UL; i < countRanges; ++i, ++pRanges)
        {
            moduleInfo.Initialize(static_cast<ULONG_PTR>(pRanges->startAddr), pRanges->codeSize);
            m_modules.Insert(moduleInfo);
        }

        m_processId = processId;
        m_refCount++;
        ret = true;

        if (NULL != s_pGlobalCount)
        {
            (*s_pGlobalCount)++;
        }
    }
    else
    {
        // Release the reference as it is already initialized.
        m_refCount--;
    }

    return ret;
}


void StackWalker::Deinitialize()
{
    ASSERT(0L == m_refCount);

    m_processId = NULL;
    m_clients.ClearAll();
    m_modules.Clear();

    m_refCount = -1L;
    m_clientsMaxDepth = 0UL;

    if (NULL != s_pGlobalCount)
    {
        (*s_pGlobalCount)--;
    }
}


LONG StackWalker::AddRef()
{
    LONG refCount;

    do
    {
        refCount = m_refCount;

        if (0L >= refCount)
        {
            refCount--;
            break;
        }
    }
    while (!m_refCount.CompareAndSwap(refCount, refCount + 1L));

    refCount++;

    // The reference count should never reach a negative number other than -1.
    ASSERT(-1L <= refCount);

    return refCount;
}


LONG StackWalker::Release()
{
    LONG refCount = --m_refCount;

    // The reference count should never reach a negative number.
    ASSERT(0L <= refCount);

    if (0L == refCount)
    {
        Deinitialize();
    }

    return refCount;
}


bool StackWalker::IsValid() const
{
    return 0L < m_refCount;
}


bool StackWalker::IsInitialized() const
{
    return 0L <= m_refCount;
}


bool StackWalker::RegisterClient(ULONG clientId, ULONG maxDepth)
{
    ASSERT(MAX_CLIENTS_COUNT > clientId);
    bool registered = !m_clients.Set(static_cast<uint>(clientId));

    if (registered)
    {
        ULONG oldClientsMaxDepth;

        do
        {
            oldClientsMaxDepth = m_clientsMaxDepth;

            if (oldClientsMaxDepth >= maxDepth)
            {
                break;
            }
        }
        while (!m_clientsMaxDepth.CompareAndSwap(oldClientsMaxDepth, maxDepth));
    }

    return registered;
}


bool StackWalker::UnregisterClient(ULONG clientId)
{
    ASSERT(MAX_CLIENTS_COUNT > clientId);
    return m_clients.Clear(static_cast<uint>(clientId));
}


bool StackWalker::IsClientRegistered(ULONG clientId) const
{
    ASSERT(MAX_CLIENTS_COUNT > clientId);
    return m_clients.Test(static_cast<uint>(clientId));
}


bool StackWalker::AddModuleInfo(ULONG_PTR imageLoadAddr, ULONG imageSize)
{
    bool ret = false;

    IMAGE_NT_HEADERS* pNtHeader = ExtractNtHeader(imageLoadAddr, imageSize);

    if (NULL != pNtHeader)
    {
        KIRQL irql = KeGetCurrentIrql();

        ModuleInfo moduleInfo;

#ifdef _X86_
        ret = true;

        ULONG sectionNum = pNtHeader->FileHeader.NumberOfSections;
        IMAGE_SECTION_HEADER* pSection = IMAGE_FIRST_SECTION(pNtHeader);

        for (ULONG i = 0; i < sectionNum; ++i, ++pSection)
        {
            if (0UL != (pSection->Characteristics & (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE)))
            {
                // Insert the executable code section of current loaded module.
                moduleInfo.Initialize(imageLoadAddr + pSection->VirtualAddress, pSection->Misc.VirtualSize, irql);

                if (NULL == m_modules.Insert(moduleInfo))
                {
                    ret = false;
                    break;
                }
            }
        }

#else
        moduleInfo.Initialize(imageLoadAddr, imageSize, irql);
        ret = (NULL != m_modules.Insert(moduleInfo));
#endif
    }

    return ret;
}


ModuleInfo* StackWalker::LookupModuleInfo(ULONG_PTR addr, MODE mode)
{
    ModuleInfo* pModuleInfo = NULL;

    if (KernelMode == mode && NULL != s_pKernelModules)
    {
        if (!s_pKernelModules->IsEmpty() && s_pKernelModules->GetBegin()->GetAddress() <= addr && addr < (s_pKernelModules->GetEnd()->GetAddress() + s_pKernelModules->GetEnd()->GetSize()))
        {
            pModuleInfo = s_pKernelModules->Lookup(addr);
        }
    }

    if (NULL == pModuleInfo)
    {
        if (!m_modules.IsEmpty() && m_modules.GetBegin()->GetAddress() <= addr && addr < (m_modules.GetEnd()->GetAddress() + m_modules.GetEnd()->GetSize()))
        {
            pModuleInfo = m_modules.Lookup(addr);
        }
    }

    return pModuleInfo;
}


// Verify that the address is a potential return address.
bool StackWalker::IsPotentialReturnAddress(ULONG_PTR addr, MODE mode, KIRQL irql)
{
    bool ret = false;

    if (IS_PAGABLE(irql) ||
        (TRUE == MmIsAddressValid(reinterpret_cast<PVOID>(addr - MAX_CALL_INST_BYTES)) &&
         TRUE == MmIsAddressValid(reinterpret_cast<PVOID>(addr))))
    {
        ModuleInfo* pModuleInfo = LookupModuleInfo(addr, mode);

        if (NULL != pModuleInfo && (addr - MAX_CALL_INST_BYTES) >= pModuleInfo->GetAddress())
        {
            ret = IsAfterCallInst(addr);
        }
    }

    return ret;
}


ULONG StackWalker::CaptureStackPotentialValues(const KTRAP_FRAME& trapFrame, ULONG32* pValues, USHORT* pOffsets, ULONG maxCount, KIRQL irql)
{
    ASSERT(NULL != pValues && NULL != pOffsets);


    ULONG count = 0UL;

    ULONG_PTR stackPtr = KxGetStackPointerFromTrapFrame(&trapFrame);

    if (0 != stackPtr)
    {
        MODE mode = IS_KERNEL_MODE(trapFrame.SegCs) ? KernelMode : UserMode;

        ULONG_PTR stackStart = 0, stackEnd = 0;
        ULONG addressMode = GetContainingStackLimits(stackPtr, stackStart, stackEnd, mode);

        if (0UL != addressMode)
        {
            __try
            {
                // In kernel mode, the stack is already available.
                if (UserMode == mode && IS_PAGABLE(irql))
                {
                    ProbeForRead(reinterpret_cast<void*>(static_cast<ULONG_PTR>(stackStart)), stackEnd - stackStart, sizeof(UCHAR));
                }

#ifdef _AMD64_

                if (KernelMode == mode || sizeof(ULONG32) != addressMode)
                {
                    count = WalkStack64(pValues, pOffsets, maxCount, mode,
                                        stackPtr, stackStart, stackEnd, irql);
                }
                else
#endif
                {
                    count = WalkStack32(pValues, pOffsets, maxCount, mode,
                                        static_cast<ULONG32>(stackPtr), static_cast<ULONG32>(stackStart), static_cast<ULONG32>(stackEnd), irql);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
    }

    return count;
}


ULONG StackWalker::WalkStack32(ULONG32* pValues, USHORT* pOffsets, ULONG maxCount, MODE mode,
                               ULONG32 stackCurrent, ULONG32 stackStart, ULONG32 stackEnd, KIRQL irql)
{
    ULONG count = 0UL;

    if (stackStart <= stackCurrent && 0UL != maxCount)
    {
        int maxOffset = static_cast<int>(stackEnd - stackCurrent) / static_cast<int>(sizeof(ULONG32));

        if (maxOffset > MAX_STACK_WALK_DEPTH)
        {
            maxOffset = MAX_STACK_WALK_DEPTH;
        }

        ULONG32* pStack = reinterpret_cast<ULONG32*>(static_cast<ULONG_PTR>(stackCurrent));

        for (int stackOffset = 0; stackOffset < maxOffset; ++stackOffset, ++pStack)
        {
            if (!KxIsAddressValid(pStack, irql))
            {
                break;
            }

            ULONG32 stackValue = *pStack;

            if ((stackStart <= stackValue && stackValue <= stackEnd) ||
                IsPotentialReturnAddress(stackValue, mode, irql))
            {
                pValues [count] = stackValue;
                pOffsets[count] = static_cast<USHORT>(stackOffset);
                count++;

                if (count >= maxCount)
                {
                    break;
                }
            }
        }
    }

    return count;
}


#ifdef _AMD64_

ULONG StackWalker::WalkStack64(ULONG32* pValues, USHORT* pOffsets, ULONG maxCount, MODE mode,
                               ULONG64 stackCurrent, ULONG64 stackStart, ULONG64 stackEnd, KIRQL irql)
{
    ULONG count = 0UL;

    if (stackStart <= stackCurrent && 1UL < maxCount)
    {
        int maxOffset = static_cast<int>(stackEnd - sizeof(ULONG32) - stackCurrent) / static_cast<int>(sizeof(ULONG32));

        if (maxOffset > MAX_STACK_WALK_DEPTH)
        {
            maxOffset = MAX_STACK_WALK_DEPTH;
        }

        ULONG maxCount64 = maxCount - 1UL;

        // Increment only 4 bytes whether it is a 64-bit or 32-bit OS,
        // because the smallest value to be pushed on the stack is 16-bits.
        ULONG32* pStack = reinterpret_cast<ULONG32*>(stackCurrent);

        for (int stackOffset = 0; stackOffset < maxOffset; ++stackOffset, ++pStack)
        {
            if (!KxIsAddressValid(pStack, irql))
            {
                break;
            }

            ULARGE_INTEGER stackValue;
            stackValue.QuadPart = *reinterpret_cast<ULONG64*>(pStack);

            if ((stackStart <= stackValue.QuadPart && stackValue.QuadPart <= stackEnd) ||
                IsPotentialReturnAddress(stackValue.QuadPart, mode, irql))
            {
                pValues [count] = stackValue.LowPart;
                pOffsets[count] = static_cast<USHORT>(stackOffset);
                count++;

                // Iterate to next offset.
                stackOffset++;
                pStack++;

                pValues [count] = stackValue.HighPart;
                pOffsets[count] = static_cast<USHORT>(stackOffset);
                count++;

                if (count < maxCount)
                {
                    for (; stackOffset < maxOffset; ++stackOffset, ++pStack)
                    {
                        if (!KxIsAddressValid(pStack, irql))
                        {
                            break;
                        }

                        stackValue.QuadPart = *reinterpret_cast<ULONG64*>(pStack);

                        if (!((stackStart <= stackValue.QuadPart && stackValue.QuadPart <= stackEnd) ||
                              IsPotentialReturnAddress(stackValue.QuadPart, mode, irql)))
                        {
                            break;
                        }

                        pValues [count] = stackValue.HighPart;
                        pOffsets[count] = static_cast<USHORT>(stackOffset);
                        count++;

                        if (count >= maxCount)
                        {
                            break;
                        }
                    }
                }

                if (count >= maxCount64)
                {
                    break;
                }
            }
        }
    }

    return count;
}

#endif // _AMD64_


ULONG StackWalker::CaptureStackBackTrace(const KTRAP_FRAME& trapFrame, ULONG_PTR* pCallers, ULONG maxCount, BOOLEAN& is64Bit, KIRQL irql)
{
    ASSERT(NULL != pCallers);

    ULONG count = 0UL;

    if (count < maxCount)
    {
        ULONG_PTR stackPtr = KxGetStackPointerFromTrapFrame(&trapFrame);

        if (0 != stackPtr)
        {
            MODE mode = IS_KERNEL_MODE(trapFrame.SegCs) ? KernelMode : UserMode;

            ULONG_PTR stackStart = 0, stackEnd = 0;
            ULONG addressMode = GetContainingStackLimits(stackPtr, stackStart, stackEnd, mode);

            if (0UL != addressMode)
            {
                __try
                {
#ifdef _AMD64_

                    if (sizeof(ULONG32) != addressMode)
                    {
                        is64Bit = TRUE;
                        pCallers[count++] = KxGetInstructionPointerFromTrapFrame(&trapFrame);
                        count += VirtualStackUnwind(&trapFrame, pCallers + count, maxCount - count, mode,
                                                    stackStart, stackEnd, irql);
#if 0

                        if (UserMode == mode && NULL != PsGetCurrentProcessWow64Process())
                        {
                            const WOW64_CONTEXT* pWow64Context = KxGetWow64Context();

                            if (NULL != pWow64Context)
                            {
                                stackPtr = static_cast<ULONG_PTR>(pWow64Context->Esp);

                                if (KxGetUserWow64StackLimits(&stackStart, &stackEnd))
                                {
                                    if (stackStart <= stackPtr && stackPtr <= stackEnd)
                                    {
                                        ULONG wow64Count = 0UL;
                                        ULONG32* pWow64Callers = reinterpret_cast<ULONG32*>(pCallers + count);
                                        pWow64Callers[wow64Count++] = static_cast<ULONG_PTR>(pWow64Context->Eip);
                                        wow64Count += WalkFrameChain(pWow64Callers + wow64Count,
                                                                     maxCount - wow64Count,
                                                                     mode,
                                                                     pWow64Context->Ebp,
                                                                     pWow64Context->Esp,
                                                                     static_cast<ULONG32>(stackStart),
                                                                     static_cast<ULONG32>(stackEnd),
                                                                     irql);
                                    }
                                }
                            }
                        }

#endif
                    }
                    else
#endif
                    {
                        is64Bit = FALSE;
                        pCallers[count++] = KxGetInstructionPointerFromTrapFrame(&trapFrame);

                        ULONG32 framePtr;
#ifdef _X86_
                        framePtr = trapFrame.Ebp;
#else
                        framePtr = static_cast<ULONG32>(trapFrame.Rbp);
#endif

                        count += WalkFrameChain(reinterpret_cast<ULONG32*>(pCallers) + count,
                                                maxCount - count,
                                                mode,
                                                framePtr,
                                                static_cast<ULONG32>(stackPtr),
                                                static_cast<ULONG32>(stackStart),
                                                static_cast<ULONG32>(stackEnd),
                                                irql);
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }
        }

        if (0UL == count)
        {
#ifdef _AMD64_
            is64Bit = TRUE;
#else
            is64Bit = FALSE;
#endif
            pCallers[count++] = KxGetInstructionPointerFromTrapFrame(&trapFrame);
        }
    }

    return count;
}


bool StackWalker::RecoverFramePointer(ULONG32& framePtr, ULONG32 stackStart, ULONG32 stackEnd, ULONG32 stackCurrent,
                                      MODE mode, KIRQL irql)
{
    bool ret = false;

    ULONG32 currFramePtr = framePtr;

    if (stackStart <= currFramePtr && currFramePtr < stackEnd && (stackEnd - currFramePtr) >= (sizeof(ULONG32) * 2))
    {
        if (KxIsAddressValid(reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(currFramePtr + sizeof(ULONG32))), irql))
        {
            ULONG32 returnAddress = *reinterpret_cast<ULONG32*>(currFramePtr + sizeof(ULONG32));

            if (!(stackStart < returnAddress && returnAddress < stackEnd))
            {
                ret = IsPotentialReturnAddress(static_cast<ULONG_PTR>(returnAddress), mode, irql);
            }
        }
    }

    if (!ret && IS_PAGABLE(irql))
    {
        ULONG32 newFramePtr, returnAddress;

        currFramePtr = stackCurrent;

        while (stackStart <= currFramePtr && currFramePtr < stackEnd && (stackEnd - currFramePtr) >= (sizeof(ULONG32) * 2))
        {
            newFramePtr = *reinterpret_cast<ULONG32*>(currFramePtr + 0);

            if (currFramePtr < newFramePtr && newFramePtr < stackEnd)
            {
                returnAddress = *reinterpret_cast<ULONG32*>(currFramePtr + sizeof(ULONG32));

                if (!(stackStart < returnAddress && returnAddress < stackEnd))
                {
                    if (IsPotentialReturnAddress(static_cast<ULONG_PTR>(returnAddress), mode, irql))
                    {
                        framePtr = currFramePtr;
                        ret = true;
                        break;
                    }
                }
            }

            currFramePtr += sizeof(ULONG32);
        }
    }

    return ret;
}


ULONG StackWalker::WalkFrameChain(ULONG32* pCallers, ULONG maxCount, MODE mode,
                                  ULONG32 framePtr, ULONG32 stackCurrent, ULONG32 stackStart, ULONG32 stackEnd, KIRQL irql)
{
    ULONG count = 0UL;

    // In kernel mode, the stack is already available.
    if (UserMode == mode && IS_PAGABLE(irql))
    {
        ProbeForRead(reinterpret_cast<void*>(static_cast<ULONG_PTR>(stackStart)), stackEnd - stackStart, sizeof(UCHAR));
    }

    if (RecoverFramePointer(framePtr, stackStart, stackEnd, stackCurrent, mode, irql))
    {
        BOOLEAN invalidFramePtr = FALSE;
        ULONG32 newFramePtr, returnAddress;

        do
        {
            if (count >= maxCount)
            {
                break;
            }

            if (!IS_PAGABLE(irql) &&
                (FALSE == MmIsAddressValid(reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(framePtr + 0))) ||
                 FALSE == MmIsAddressValid(reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(framePtr + sizeof(ULONG32))))))
            {
                break;
            }

            newFramePtr   = *reinterpret_cast<ULONG32*>(framePtr + 0);
            returnAddress = *reinterpret_cast<ULONG32*>(framePtr + sizeof(ULONG32));

            //
            // Figure out if the new frame pointer is OK. This validation
            // should avoid all exceptions in kernel mode because we always
            // read within the current thread's stack and the stack is
            // guaranteed to be in memory (no page faults). It is also guaranteed
            // that we do not take random exceptions in user mode because we always
            // keep the frame pointer within stack limits.
            //

            if (!(framePtr < newFramePtr && newFramePtr < stackEnd))
            {
                invalidFramePtr = TRUE;
            }

            //
            // Figure out if the return address is OK. If return address
            // is a stack address or <64k then something is wrong. There is
            // no reason to return garbage to the caller therefore we stop.
            //

            if (stackStart < returnAddress && returnAddress < stackEnd)
            {
                break;
            }

            if (KernelMode == mode)
            {
                if (!IS_SYSTEM_ADDRESS(reinterpret_cast<PVOID>(returnAddress)))
                {
                    break;
                }
            }
            else
            {
                if (!IS_USER_ADDRESS(reinterpret_cast<PVOID>(returnAddress)))
                {
                    break;
                }
            }

            //
            // Store new fp and return address and move on.
            // If the new FP value is bogus but the return address
            // looks OK then we still save the address.
            //

            pCallers[count++] = returnAddress;

            if (invalidFramePtr)
            {
                break;
            }
            else
            {
                framePtr = newFramePtr;
            }
        }
        while (stackStart < framePtr && framePtr < stackEnd && (stackEnd - framePtr) >= (sizeof(ULONG32) * 2));
    }

    return count;
}


#ifdef _AMD64_

ULONG StackWalker::VirtualStackUnwind(const KTRAP_FRAME* pTrapFrame, ULONG64* pCallers, ULONG maxCount, MODE mode,
                                      ULONG64 stackStart, ULONG64 stackEnd, KIRQL irql)
{
    ULONG count = 0UL;

    RUNTIME_FUNCTION* pFunctionEntry;
    CONTEXT contextRecord;
    PVOID pHandlerData;
    ULONG64 establisherFrame;

    RtlZeroMemory(&contextRecord, sizeof(contextRecord));

    contextRecord.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_XSTATE;
    contextRecord.MxCsr = pTrapFrame->MxCsr;

    contextRecord.SegCs = pTrapFrame->SegCs;
    contextRecord.SegDs = pTrapFrame->SegDs;
    contextRecord.SegEs = pTrapFrame->SegEs;
    contextRecord.SegFs = pTrapFrame->SegFs;
    contextRecord.SegGs = pTrapFrame->SegGs;
    contextRecord.SegSs = pTrapFrame->SegSs;
    contextRecord.EFlags = pTrapFrame->EFlags;

    contextRecord.Rax = pTrapFrame->Rax;
    contextRecord.Rcx = pTrapFrame->Rcx;
    contextRecord.Rdx = pTrapFrame->Rdx;
    contextRecord.Rbx = pTrapFrame->Rbx;
    contextRecord.Rsp = pTrapFrame->Rsp;
    contextRecord.Rbp = pTrapFrame->Rbp;
    contextRecord.Rsi = pTrapFrame->Rsi;
    contextRecord.Rdi = pTrapFrame->Rdi;
    contextRecord.R8 = pTrapFrame->R8;
    contextRecord.R9 = pTrapFrame->R9;
    contextRecord.R10 = pTrapFrame->R10;
    contextRecord.R11 = pTrapFrame->R11;

    contextRecord.Rip = pTrapFrame->Rip;

    RtlCopyMemory(&contextRecord.Xmm0, &pTrapFrame->Xmm0, sizeof(M128A) * 6);


    if (KxIsAddressValid(reinterpret_cast<PVOID>(contextRecord.Rip), irql))
    {
        while (count < maxCount)
        {
            if (!(stackStart <= contextRecord.Rsp && contextRecord.Rsp < stackEnd))
            {
                break;
            }


            ModuleInfo* pModuleInfo = LookupModuleInfo(contextRecord.Rip, mode);

            if (NULL == pModuleInfo)
            {
                break;
            }

            if (!pModuleInfo->IsFunctionTableInitialized())
            {
                pModuleInfo->InitializeFunctionTable(irql);
            }


            pFunctionEntry = pModuleInfo->LookupFunctionEntry(contextRecord.Rip, irql);

            if (NULL == pFunctionEntry)
            {
                break;
            }


            UNWIND_INFO* pUnwindInfo =
                reinterpret_cast<UNWIND_INFO*>(pModuleInfo->m_baseAddr + static_cast<ULONG_PTR>(pFunctionEntry->UnwindData));

            if (!KxIsAddressValid(pUnwindInfo, irql))
            {
                break;
            }

            if (0 != pUnwindInfo->FrameRegister)
            {
                establisherFrame = (&contextRecord.Rax)[pUnwindInfo->FrameRegister];

                if (!(stackStart <= establisherFrame && establisherFrame < stackEnd))
                {
                    break;
                }
            }


            //
            // In kernel mode we only want a the call-stack down to the first interrupt.
            // We do not want to stack up interrupts.
            //

            if (KernelMode == mode)
            {
                bool isMachineFrame = false;

                for (ULONG index = 0UL, countOfCodes = pUnwindInfo->CountOfCodes; index < countOfCodes; ++index)
                {
                    if (UWOP_PUSH_MACHFRAME == pUnwindInfo->UnwindCode[index].UnwindOp)
                    {
                        isMachineFrame = true;
                        break;
                    }
                }

                if (isMachineFrame)
                {
                    break;
                }
            }


            RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                             pModuleInfo->m_baseAddr,
                             contextRecord.Rip,
                             pFunctionEntry,
                             &contextRecord,
                             &pHandlerData,
                             &establisherFrame,
                             NULL);

            // Check the next PC value to make sure it is valid.
            if (0 == contextRecord.Rip || !KxIsAddressValid(reinterpret_cast<PVOID>(contextRecord.Rip), irql))
            {
                break;
            }

            if (KernelMode == mode)
            {
                if (!IS_SYSTEM_ADDRESS(reinterpret_cast<PVOID>(contextRecord.Rip)))
                {
                    break;
                }
            }
            else
            {
                if (!IS_USER_ADDRESS(reinterpret_cast<PVOID>(contextRecord.Rip)))
                {
                    break;
                }
            }

            pCallers[count++] = contextRecord.Rip;
        }
    }

    return count;
}

#endif // _AMD64_


#define MODRM_RM_OFFSET  0
#define MODRM_REG_OFFSET 3
#define MODRM_MOD_OFFSET 6

#define MODRM_RM_MASK  (0x7 << MODRM_RM_OFFSET)
#define MODRM_REG_MASK (0x7 << MODRM_REG_OFFSET)
#define MODRM_MOD_MASK (0x3 << MODRM_MOD_OFFSET)


static FORCEINLINE bool Is2BytesCallInst(const UCHAR* pOpCode)
{
    bool ret = false;

    if (*pOpCode == 0xFF)
    {
        UCHAR modrm = pOpCode[1];
        modrm &= MODRM_MOD_MASK | MODRM_REG_MASK;

        //000007FF34295573: FF 1B              call        fword ptr [rbx]
        ret = ((0x3 << MODRM_REG_OFFSET) | (0x0 << MODRM_MOD_OFFSET)) == modrm ||
              // 000007FF3429509B: FF 11              call        qword ptr [rcx]
              ((0x2 << MODRM_REG_OFFSET) | (0x0 << MODRM_MOD_OFFSET)) == modrm ||
              // 000007FF3D5F431F: FF D1              call        rcx
              ((0x2 << MODRM_REG_OFFSET) | (0x3 << MODRM_MOD_OFFSET)) == modrm;
    }

    return ret;
}

static FORCEINLINE bool Is3BytesCallInst(const UCHAR* pOpCode)
{
    bool ret = false;

    if (*pOpCode == 0xFF)
    {
        UCHAR modrm = pOpCode[1];

        ret = (0x3 << MODRM_MOD_OFFSET) != (modrm & MODRM_MOD_MASK) &&
              //
              // 7C90E470: FF 14 90           call        dword ptr [eax+edx*4]
              // RM == 4; this has SIB bytes;
              //
              // (0x2 << MODRM_REG_OFFSET) == (modrm & MODRM_REG_MASK)
              //
              //
              //  7C915236: FF 55 18           call        dword ptr [ebp+18h]
              // 000007FF34293723: FF 58 10    call        fword ptr [rax+10h]
              //
              // (0x3 << MODRM_REG_OFFSET) == (modrm & MODRM_REG_MASK)
              //
              (0x2 << MODRM_REG_OFFSET) == (modrm & (0x6 << MODRM_REG_OFFSET));
    }

    return ret;
}

static FORCEINLINE bool Is4BytesCallInst(const UCHAR* pOpCode)
{
    bool ret = false;

    if (*pOpCode == 0xFF)
    {
        UCHAR modrm = pOpCode[1];

        // 000007FF3428EFFE: FF 54 24 48        call        qword ptr [rsp+48h]
        // 0x54 --> RM = 0x4; this had SIB
        ret = ((0x2 << MODRM_REG_OFFSET) | (0x4 << MODRM_RM_OFFSET)) == (modrm & (MODRM_REG_MASK | MODRM_RM_MASK)) &&
              (0x3 << MODRM_MOD_OFFSET) != (modrm & MODRM_MOD_MASK);
    }

    return ret;
}

static FORCEINLINE bool Is5BytesCallInst(const UCHAR* pOpCode)
{
    // E8 cw CALL rel16 Call near, relative, displacement relative to next instruction
    //  E8 cd CALL rel32 Call near, relative, displacement relative to next instruction
    // Ingore the 16 bits;
    bool ret = (*pOpCode == 0xE8);

    return ret;
}

static FORCEINLINE bool Is6BytesCallInst(const UCHAR* pOpCode)
{
    bool ret = false;

    // 000007FF2E5FF4C8: FF 90 A0 00 00 00  call        qword ptr [rax+000000A0h]
    if (*pOpCode == 0xFF)
    {
        UCHAR modrm = pOpCode[1];

        //  7C91047C: FF 15 88 04 91 7C  call        dword ptr ds:[7C910488h]
        ret = ((0x2 << MODRM_REG_OFFSET) | (0x0 << MODRM_MOD_OFFSET)) == (modrm & (MODRM_REG_MASK | MODRM_MOD_MASK));
    }

    return ret;
}

static FORCEINLINE bool Is7BytesCallInst(const UCHAR* pOpCode)
{
    // 7DDC9561: 9A DC 7D 25 0A E1  call        7DE1:0A257DDC
    //           7D
    bool ret = (*pOpCode == 0x9A);

    return ret;
}

bool StackWalker::IsAfterCallInst(ULONG_PTR addr)
{
    //
    //   FF /2 CALL r/m16 Call near, absolute indirect, address given in r/m16
    //   FF /2 CALL r/m32 Call near, absolute indirect, address given in r/m32
    //   9A cd CALL ptr16:16 Call far, absolute, address given in operand
    //   9A cp CALL ptr16:32 Call far, absolute, address given in operand
    //   FF /3 CALL m16:16 Call far, absolute indirect, address given in m16:16
    //   FF /3 CALL m16:32 Call far, absolute indirect, address given in m16:32
    //

    bool ret;

    __try
    {
        UINT64 instBytes = *reinterpret_cast<ULONG64*>(addr - MAX_CALL_INST_BYTES);
        UCHAR* pBytes = reinterpret_cast<UCHAR*>(&instBytes);

        ret = (Is5BytesCallInst(pBytes + (MAX_CALL_INST_BYTES - 5)) ||
               Is6BytesCallInst(pBytes + (MAX_CALL_INST_BYTES - 6)) ||
               Is2BytesCallInst(pBytes + (MAX_CALL_INST_BYTES - 2)) ||
               Is3BytesCallInst(pBytes + (MAX_CALL_INST_BYTES - 3)) ||
               Is4BytesCallInst(pBytes + (MAX_CALL_INST_BYTES - 4)) ||
               Is7BytesCallInst(pBytes + (MAX_CALL_INST_BYTES - 7)));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ret = false;
    }

    return ret;
}


ULONG StackWalker::InitializeKernelModulesInfo(ModuleInfoList& kernelModules)
{
    NTSTATUS status;
    ULONG bufferSize = 0UL;
    RTL_MODULE_INFO* pModules = NULL;
    ULONG numberOfModules = 0UL;

    s_pKernelModules = &kernelModules;
    kernelModules.Clear();

    do
    {
        ULONG modulesSize = 0UL;
        // Get the required array size.
        status = RtlQueryModuleInformation(&modulesSize, sizeof(RTL_MODULE_INFO), NULL);

        if (!NT_SUCCESS(status) || 0 == modulesSize)
        {
            break;
        }

        // Calculate the number of modules.
        numberOfModules = modulesSize / sizeof(RTL_MODULE_INFO);

        if (bufferSize < modulesSize)
        {
            if (NULL != pModules)
            {
                ExFreePoolWithTag(pModules, ALLOC_POOL_TAG);
                pModules = NULL;
            }

            bufferSize = modulesSize;
        }

        if (NULL == pModules)
        {
            // Allocate memory to receive data.
            pModules = reinterpret_cast<RTL_MODULE_INFO*>(ExAllocatePoolWithTag(PagedPool, modulesSize, ALLOC_POOL_TAG));

            if (NULL == pModules)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }

        RtlZeroMemory(pModules, modulesSize);

        // Obtain the module information.
        status = RtlQueryModuleInformation(&modulesSize, sizeof(RTL_MODULE_INFO), pModules);
    }
    while (!NT_SUCCESS(status));

    if (NULL != pModules)
    {
        if (kernelModules.Grow(numberOfModules))
        {
            ModuleInfo moduleInfo;

            for (ULONG i = 0UL; i < numberOfModules; ++i)
            {
                moduleInfo.Initialize(reinterpret_cast<ULONG_PTR>(pModules[i].ImageBase), pModules[i].ImageSize, DISPATCH_LEVEL);
                kernelModules.Insert(moduleInfo);
            }
        }

        ExFreePoolWithTag(pModules, ALLOC_POOL_TAG);
    }

    return kernelModules.GetLength();
}


void StackWalker::SetGlobalCounter(AtomicCounter<ULONG>* pCounter)
{
    s_pGlobalCount = pCounter;
}


static IMAGE_NT_HEADERS* ExtractNtHeader(ULONG_PTR imageLoadAddr, ULONG imageSize)
{
    IMAGE_NT_HEADERS* pNtHeader = NULL;

    if (sizeof(IMAGE_NT_HEADERS32) <= imageSize)
    {
        IMAGE_DOS_HEADER* pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(imageLoadAddr);

        if (IMAGE_DOS_SIGNATURE == pDosHeader->e_magic)
        {
            if (sizeof(IMAGE_DOS_HEADER) <= pDosHeader->e_lfanew)
            {
                ULONG pos = static_cast<ULONG>(pDosHeader->e_lfanew);

                if (pos < imageSize && sizeof(IMAGE_NT_HEADERS32) <= (imageSize - pos))
                {
                    pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(imageLoadAddr + pDosHeader->e_lfanew);
                }
            }
        }
        else if (IMAGE_NT_SIGNATURE == pDosHeader->e_magic)
        {
            pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pDosHeader);
        }
    }

    return pNtHeader;
}


static ULONG GetContainingStackLimits(ULONG_PTR stackPtr, ULONG_PTR& lowLimit, ULONG_PTR& highLimit, MODE mode)
{
    ULONG addressMode = 0UL;

    if (KernelMode == mode)
    {
        if (IS_SYSTEM_ADDRESS(reinterpret_cast<PVOID>(stackPtr)) && TRUE == MmIsAddressValid(reinterpret_cast<PVOID>(stackPtr)))
        {
            IoGetStackLimits(&lowLimit, &highLimit);

            if (0 != lowLimit && ((ULONG_PTR)(-1)) != highLimit && (lowLimit <= stackPtr && stackPtr <= highLimit))
            {
                addressMode = sizeof(PVOID);
            }
            else
            {
                addressMode = KxGetKernelStackLimits(&lowLimit, &highLimit);
                addressMode *= sizeof(PVOID);
            }
        }
    }
    else
    {
        if (IS_USER_ADDRESS(reinterpret_cast<PVOID>(stackPtr)))
        {
            if (KxGetUserStackLimits(&lowLimit, &highLimit))
            {
                if (lowLimit <= stackPtr && stackPtr <= highLimit)
                {
                    addressMode = sizeof(PVOID);
                }

#ifdef _AMD64_
                else
                {
                    if (KxGetUserWow64StackLimits(&lowLimit, &highLimit))
                    {
                        if (lowLimit <= stackPtr && stackPtr <= highLimit)
                        {
                            addressMode = sizeof(ULONG32);
                        }
                    }
                }

#endif
            }

#if 0

            if (0UL == addressMode)
            {
                addressMode = sizeof(PVOID);

                if (0 == lowLimit || lowLimit >= stackPtr)
                {
                    lowLimit = stackPtr;
                }

                if (stackPtr >= highLimit)
                {
                    highLimit = ALIGN_UP_BY(stackPtr, PAGE_SIZE);
                }
            }

#endif
        }
    }

    return addressMode;
}
