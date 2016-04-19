//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file KiUserExceptionDispatcherFrameTraverser.cpp
///
//==================================================================================

#include "KiUserExceptionDispatcherFrameTraverser.h"
#include "FrameChainWalker.h"
#include "StackFrameTraverser.h"
#include <dia2.h>

KiUserExceptionDispatcherFrameTraverser::KiUserExceptionDispatcherFrameTraverser(FrameChainWalker& walker) : m_walker(walker),
    m_pFrameData(NULL),
    m_stackPtr(0)
{
}

KiUserExceptionDispatcherFrameTraverser::~KiUserExceptionDispatcherFrameTraverser()
{
    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }
}

void KiUserExceptionDispatcherFrameTraverser::Reset(IDiaFrameData* pFrameData, VAddrX86 stackPtr)
{
    if (NULL != pFrameData)
    {
        pFrameData->AddRef();
    }

    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }

    m_pFrameData = pFrameData;
    m_stackPtr = stackPtr;
}

HRESULT KiUserExceptionDispatcherFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    return StackFrameTraverser::GetFrameData(m_walker.GetContext(), m_stackPtr, m_pFrameData, frameData);
}

HRESULT KiUserExceptionDispatcherFrameTraverser::TraverseNext()
{
    HRESULT hr = E_FAIL;
    StackWalkContextX86& context = m_walker.GetContext();

    VAddrX86 stackPtr;

    if (context.GetRegister(REG_INDEX_ESP, stackPtr))
    {
        VAddrX86 ptrContext;

        if (context.ReadFullMemory(MEM_TYPE_STACK, stackPtr + X86_STACK_SIZE, ptrContext))
        {
            gtUByte aUnextendedContextBuffer[offsetof(CONTEXT, ExtendedRegisters)];

            if (context.ReadFullMemory(MEM_TYPE_STACK, ptrContext, aUnextendedContextBuffer))
            {
                const CONTEXT* pUnextendedContext = reinterpret_cast<CONTEXT*>(aUnextendedContextBuffer);

                context.SetRegister(REG_INDEX_EIP, pUnextendedContext->Eip);
                context.SetRegister(REG_INDEX_ESP, pUnextendedContext->Esp);
                context.SetRegister(REG_INDEX_EBP, pUnextendedContext->Ebp);
                context.SetRegister(REG_INDEX_EAX, pUnextendedContext->Eax);
                context.SetRegister(REG_INDEX_EBX, pUnextendedContext->Ebx);
                context.SetRegister(REG_INDEX_ECX, pUnextendedContext->Ecx);
                context.SetRegister(REG_INDEX_EDX, pUnextendedContext->Edx);
                context.SetRegister(REG_INDEX_ESI, pUnextendedContext->Esi);
                context.SetRegister(REG_INDEX_EDI, pUnextendedContext->Edi);

                m_walker.SetContextRecordRestored(true);
            }
        }
    }

    return hr;
}
