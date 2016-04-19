//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FpoFrameTraverser.cpp
///
//==================================================================================

#include "FpoFrameTraverser.h"
#include <dia2.h>

FpoFrameTraverser::FpoFrameTraverser(FrameChainWalker& walker) : m_baseTrav(walker)
{
}

void FpoFrameTraverser::Reset(IDiaFrameData* pFrameData, VAddrX86 stackPtr)
{
    m_baseTrav.Reset(L"$T0 .raSearch = $eip $T0 ^ = $esp $T0 4 + =", pFrameData, stackPtr);
}

HRESULT FpoFrameTraverser::TraverseNext()
{
    HRESULT hr = m_baseTrav.TraverseNext();

    if (S_OK == hr)
    {
        BOOL allocatesBasePtr = TRUE;

        if (S_OK == m_baseTrav.GetFrameInterface()->get_allocatesBasePointer(&allocatesBasePtr) && FALSE == allocatesBasePtr)
        {
            VAddrX86 framePtr;

            if (m_baseTrav.GetContext().GetRegister(REG_INDEX_EBP, framePtr))
            {
                m_baseTrav.GetContext().SetRegister(REG_INDEX_EBP, framePtr);
            }
        }
    }

    return hr;
}

HRESULT FpoFrameTraverser::GetFrameData(StackFrameData& frameData)
{
    HRESULT hr = m_baseTrav.GetFrameData(frameData);
    StackWalkContextX86& context = m_baseTrav.GetContext();

    if (FALSE != frameData.m_valid.base)
    {
        if (FALSE != frameData.m_valid.systemExceptionHandling && FALSE != frameData.m_systemExceptionHandling)
        {
            //
            // 0: OldEBP
            // 4: Barrier
            // 8: SCOPETABLE*
            // 12: EXCEPTION_REGISTRATION_RECORD
            // 20: #locals#
            // 20 + #locals#: ebx
            // 24 + #locals#: esi
            // 28 + #locals#: edi
            // 32 + #locals#: DWORD// Canary
            //

            if (FALSE != frameData.m_valid.lengthLocals)
            {
                struct
                {
                    ValueX86  Edi;
                    ValueX86  Esi;
                    ValueX86  Ebx;
                } nonVolatileRegs;

                if (context.ReadFullMemory(MEM_TYPE_STACK,
                                           static_cast<VAddrX86>(frameData.m_base) - frameData.m_lengthLocals - 36, //TODO: Why 36 and not 32?
                                           nonVolatileRegs))
                {
                    context.SetRegister(REG_INDEX_EDI, nonVolatileRegs.Edi);
                    context.SetRegister(REG_INDEX_ESI, nonVolatileRegs.Esi);
                    context.SetRegister(REG_INDEX_EBX, nonVolatileRegs.Ebx);
                }
            }

            VAddrX86 framePtr = 0;

            if (context.ReadFullMemory(MEM_TYPE_STACK, static_cast<VAddrX86>(frameData.m_base) - X86_STACK_SIZE, framePtr))
            {
                context.SetRegister(REG_INDEX_EBP, framePtr);
            }

            frameData.m_localsBase = frameData.m_base - 36;
        }
        else
        {
            frameData.m_localsBase = frameData.m_base;
        }

        frameData.m_valid.localsBase = TRUE;
    }

    return hr;
}
