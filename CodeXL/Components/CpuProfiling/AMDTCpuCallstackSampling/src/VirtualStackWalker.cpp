//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file VirtualStackWalker.cpp
///
//==================================================================================

#include <VirtualStackWalker.h>
#include <CallGraph.h>
#include <CallStackBuilder.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include "StackWalker/x86/FrameChainWalker.h"
#include "StackWalker/x64/StackVirtualUnwinder.h"
#include <dia2.h>

#define MAX_RECURSION_COUNT 1U

template <class TStackTrav>
HRESULT GetFrameData(TStackTrav& stackTrav, StackFrameControlData& frame)
{
    StackFrameData frameData;
    HRESULT hr = stackTrav.GetFrameData(frameData);

    if (S_OK == hr)
    {
        stackTrav.GetContext().Extract(frame);
    }

    return hr;
}

template <class TStackTrav>
HRESULT TraverseNext(TStackTrav* pStackTrav, unsigned count, StackFrameControlData* pDataList, unsigned& countFetched)
{
    HRESULT hr;

    countFetched = 0U;

    if (0U != count)
    {
        hr = S_FALSE;

        unsigned numFetched;

        for (numFetched = 0U; numFetched < count; ++numFetched)
        {
            hr = pStackTrav->TraverseNext();

            if (FAILED(hr))
            {
                if (0U == numFetched && E_DIA_SYNTAX == hr)
                {
                    GetFrameData(*pStackTrav, pDataList[0]);
                }

                break;
            }

            HRESULT hrFrame = GetFrameData(*pStackTrav, pDataList[numFetched]);
            pStackTrav->GetContext().Propagate();

            if (S_FALSE == hr)
            {
                if (S_OK != hrFrame)
                {
                    hr = hrFrame;
                }

                break;
            }
        }

        countFetched = numFetched;
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

typedef HRESULT(*PfnTraverseNext)(void* stackTrav, unsigned count, StackFrameControlData* pDataList, unsigned& countFetched);


VirtualStackWalker::VirtualStackWalker() : m_pWalkerX86(NULL), m_pWalkerX64(NULL)
{
}

VirtualStackWalker::~VirtualStackWalker()
{
    if (NULL != m_pWalkerX86)
    {
        delete m_pWalkerX86;
    }

    if (NULL != m_pWalkerX64)
    {
        delete m_pWalkerX64;
    }
}

bool VirtualStackWalker::Reset(gtVAddr ip, gtVAddr bp, gtVAddr sp,
                               gtUInt32* pValues, gtUInt16* pOffsets, unsigned count,
                               ProcessWorkingSetQuery& workingSet)
{
    ExecutableFile* pExe = workingSet.FindModule(ip);
    bool ret = (NULL != pExe);

    if (ret)
    {
        unsigned ptrSize = pExe->Is64Bit() ? sizeof(gtUInt64) : sizeof(gtUInt32);
        ret = Reset(ip, bp, sp, pValues, pOffsets, count, workingSet, ptrSize);
    }

    return ret;
}

bool VirtualStackWalker::Reset(gtVAddr ip, gtVAddr bp, gtVAddr sp,
                               gtUInt32* pValues, gtUInt16* pOffsets, unsigned count,
                               ProcessWorkingSetQuery& workingSet, unsigned ptrSize)
{
    bool ret;

    m_stack.m_top = sp;
    m_stack.m_pValues = pValues;
    m_stack.m_pOffsets = pOffsets;
    m_stack.m_count = count;

    if (sizeof(gtUInt64) == ptrSize)
    {
        m_stack.m_ptrSize = ptrSize;

        if (NULL == m_pWalkerX64)
        {
            m_pWalkerX64 = new StackVirtualUnwinder();
        }

        m_pWalkerX64->Reset(workingSet, m_stack, static_cast<VAddrX64>(ip), static_cast<VAddrX64>(sp), static_cast<VAddrX64>(bp));
        ret = true;
    }
    else if (sizeof(gtUInt32) == ptrSize)
    {
        m_stack.m_ptrSize = ptrSize;

        if (NULL == m_pWalkerX86)
        {
            m_pWalkerX86 = new FrameChainWalker();
        }

        m_pWalkerX86->Reset(workingSet, m_stack, static_cast<VAddrX86>(ip), static_cast<VAddrX86>(sp), static_cast<VAddrX86>(bp));
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

unsigned VirtualStackWalker::BackTrace(gtUByte* pBuffer, unsigned bufferSize, gtUInt32* pFrameWalk, unsigned walkLength)
{
    GT_UNREFERENCED_PARAMETER(bufferSize);

    unsigned depth = 0U;

    void* pStackTrav;
    PfnTraverseNext pfnTraverseNext;

    gtUInt32* pFrameWalkEnd = pFrameWalk;
    gtUInt32* pCallstack32 = nullptr;
    gtUInt64* pCallstack64 = nullptr;

    if (sizeof(gtUInt64) == m_stack.m_ptrSize)
    {
        pStackTrav = m_pWalkerX64;
        pfnTraverseNext = reinterpret_cast<PfnTraverseNext>(TraverseNext<StackVirtualUnwinder>);

        pCallstack32 = NULL;
        pCallstack64 = reinterpret_cast<gtUInt64*>(pBuffer);
    }
    else if (sizeof(gtUInt32) == m_stack.m_ptrSize)
    {
        pStackTrav = m_pWalkerX86;
        pfnTraverseNext = reinterpret_cast<PfnTraverseNext>(TraverseNext<FrameChainWalker>);

        if (NULL == pFrameWalk)
        {
            walkLength = 0U;
        }
        else
        {
            if (1U < walkLength)
            {
                pFrameWalkEnd += walkLength;
                pFrameWalk++;
            }
        }

        pCallstack32 = reinterpret_cast<gtUInt32*>(pBuffer);
        pCallstack64 = NULL;
    }
    else
    {
        pStackTrav = NULL;
        pfnTraverseNext = NULL;
    }

    if (NULL != pStackTrav)
    {
        // Try to traverse from call graph on the first address
        bool traversed = false;//TraverseFromGraph(NULL, builder);

        unsigned num;
        StackFrameControlData frameData;

        if (S_OK == pfnTraverseNext(pStackTrav, 1, &frameData, num))
        {
            gtVAddr ip = frameData.m_programCounter;
            gtVAddr sp = frameData.m_stackPtr;
            gtVAddr bp = frameData.m_framePtr;

            // Only if we did not already traverse the beginning of the stack
            if (!traversed)
            {
                if (NULL != pCallstack32)
                {
                    *pCallstack32++ = static_cast<gtUInt32>(ip);
                }
                else
                {
                    *pCallstack64++ = ip;
                }

                // Try to traverse from call graph on the second address
                traversed = false;//TraverseFromGraph(pStackFramesEnum, builder);
            }

            gtUInt32 walkFramePtr;
            gtUInt32 walkStackPtr = ~0U;
            gtUInt32 walkRetAddr = 0U;

            if (pFrameWalkEnd != pFrameWalk)
            {
                walkFramePtr = static_cast<gtUInt32>(bp);
                walkRetAddr = *pFrameWalk;
                gtUInt32 retAddr = m_stack.RecoverFrame(walkFramePtr, walkStackPtr);

                if (retAddr != walkRetAddr)
                {
                    walkRetAddr = 0U;
                    walkStackPtr = ~0U;
                }
            }

            unsigned recursionCount = 0U;

            while (S_OK == pfnTraverseNext(pStackTrav, 1, &frameData, num))
            {
                bp = ip;
                ip = frameData.m_programCounter;

                if (0 != ip)
                {
                    if (bp == ip)
                    {
                        if (MAX_RECURSION_COUNT <= recursionCount)
                        {
                            break;
                        }

                        recursionCount++;
                    }
                    else
                    {
                        recursionCount = 0U;
                    }

                    bp = sp;
                    sp = frameData.m_stackPtr;

                    if (bp != sp)
                    {
                        bp = frameData.m_framePtr;

                        if (pFrameWalkEnd != pFrameWalk)
                        {
                            gtUInt32 retAddr;
                            gtUInt32 traversedStackPtr = static_cast<gtUInt32>(sp);

                            if (walkStackPtr < traversedStackPtr)
                            {
                                *pCallstack32++ = *pFrameWalk++;

                                while (pFrameWalkEnd != pFrameWalk)
                                {
                                    walkRetAddr = *pFrameWalk;
                                    retAddr = m_stack.RecoverFrame(walkFramePtr, walkStackPtr);

                                    if (retAddr != walkRetAddr)
                                    {
                                        walkRetAddr = 0U;
                                        walkStackPtr = ~0U;
                                        break;
                                    }

                                    if (walkStackPtr >= traversedStackPtr)
                                    {
                                        break;
                                    }

                                    *pCallstack32++ = *pFrameWalk++;
                                }
                            }
                        }

                        if (pFrameWalkEnd != pFrameWalk && static_cast<gtUInt32>(ip) == walkRetAddr)
                        {
                            *pCallstack32++ = *pFrameWalk++;

                            if (pFrameWalkEnd != pFrameWalk)
                            {
                                walkRetAddr = *pFrameWalk;
                                gtUInt32 retAddr = m_stack.RecoverFrame(walkFramePtr, walkStackPtr);

                                if (retAddr != walkRetAddr)
                                {
                                    walkRetAddr = 0U;
                                    walkStackPtr = ~0U;
                                    break;
                                }
                            }
                        }
                        else
                        {

                            // We do not stop traversing if the Push() fails, due to a bug in DIA:
                            //     When frameForVA() fails (even when we have a PDB),
                            //     then the traversal might have wrong offsets in the stack.
                            //
                            // * Noticed first when the the ip is in (the one line of) SetLastError() in kernel32.dll (32-bit).
                            //

                            if (NULL != pCallstack32)
                            {
                                *pCallstack32++ = static_cast<gtUInt32>(ip);
                            }
                            else
                            {
                                *pCallstack64++ = ip;
                            }
                        }
                    }
                    else
                    {
                        // Endless call stack!
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (pFrameWalkEnd != pFrameWalk && (reinterpret_cast<gtUInt32*>(pBuffer) + 1) < pCallstack32)
        {
            memcpy(pCallstack32, pFrameWalk, reinterpret_cast<gtUByte*>(pFrameWalkEnd) - reinterpret_cast<gtUByte*>(pFrameWalk));
        }

        if (NULL != pCallstack32)
        {
            depth = pCallstack32 - reinterpret_cast<gtUInt32*>(pBuffer);
        }
        else
        {
            depth = pCallstack64 - reinterpret_cast<gtUInt64*>(pBuffer);
        }
    }

    return depth;
}
/*
/// This function implements an algorithm for optimizing the traversal of virtual stacks.
/// Before traversing the virtual stack, the algorithm checks if it matches a part of a fully traversed stack
/// that has already been analyzed and verified with Symbol info and disassembly. If a match is found then we can avoid
/// performing the lengthy analysis all over again. Instead - we use the matching ready stack.
/// It is commented out since CodeXL 1.4 because of incompatibility with the kernel-mode CSS analysis. It would be good
/// to fix this and benefit from the performance boost that this algorithm can provide.
bool VirtualStackWalker::TraverseFromGraph(IDiaEnumStackFrames* pStackFramesEnum, CallStackBuilder& builder)
{
    bool ret = false;

    const gtUInt64 spBase = m_pContext->GetRegisterValue(VirtualFrameContext::REGT_RSP);
    int baseStackOffset = static_cast<int>(m_stack.m_ptrSize);

    if (!builder.IsEmpty())
    {
        baseStackOffset = static_cast<int>(spBase - builder.GetTopStackAddress()) - baseStackOffset;
    }

    unsigned count = m_stack.m_count;

    if (baseStackOffset <= static_cast<int>(m_stack.m_pOffsets[count - 1U]))
    {
        const gtUInt64 ipBase = m_pContext->GetRegisterValue(VirtualFrameContext::REGT_RIP);
        const gtUInt64 bpBase = m_pContext->GetRegisterValue(VirtualFrameContext::REGT_RBP);

        ret = builder.GetCallGraph().TraverseCallStack(ipBase,
                                                       bpBase,
                                                       spBase,
                                                       baseStackOffset,
                                                       m_stack.m_pValues,
                                                       m_stack.m_pOffsets,
                                                       count,
                                                       builder,
                                                       sizeof(gtUInt64) == m_stack.m_ptrSize);

        if (ret)
        {
//          m_stack.m_pValues += m_stack.m_count - count;
//          m_stack.m_pOffsets += m_stack.m_count - count;
//          m_stack.m_count = count;
            m_pContext->Reset(builder.GetTopTraverseAddress(), builder.GetTopFrameAddress(), builder.GetTopStackAddress());

            // Skip the current frame
            if (NULL != pStackFramesEnum)
            {
                ULONG num;
                IDiaStackFrame* pStackFrame = NULL;
                pStackFramesEnum->Reset();
                pStackFramesEnum->Next(1UL, &pStackFrame, &num);

                if (NULL != pStackFrame)
                {
                    pStackFrame->Release();
                }
            }
        }
    }

    return ret;
}
*/
