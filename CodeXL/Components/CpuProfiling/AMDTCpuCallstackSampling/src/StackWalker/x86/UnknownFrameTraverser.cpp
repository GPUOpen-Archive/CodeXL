//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file UnknownFrameTraverser.cpp
///
//==================================================================================

#include "UnknownFrameTraverser.h"
#include "FrameChainWalker.h"

UnknownFrameTraverser::UnknownFrameTraverser(FrameChainWalker& walker) : m_baseTrav(walker), m_virtualAddress(0), m_esp(0)
{
}

void UnknownFrameTraverser::Reset(IDiaFrameData* pFrameData, VAddrX86 stackPtr, VAddrX86 virtualAddress)
{
    m_baseTrav.Reset(L"$T0 .raSearch = $eip $T0 ^ = $esp $T0 4 + =", pFrameData, stackPtr);
    m_virtualAddress = virtualAddress;
    m_esp = stackPtr;
}

HRESULT UnknownFrameTraverser::TraverseNext()
{
    HRESULT hr = m_baseTrav.TraverseNext();

    if (S_OK == hr)
    {
        if (m_virtualAddress < MM_LOWEST_USER_ADDRESS)
        {
            StackWalkContextX86& context = m_baseTrav.GetContext();

            if (context.GetRegister(REG_INDEX_ESP) == (m_esp + X86_STACK_SIZE))
            {
                context.RecoverRegister(REG_INDEX_EBP);
                context.RecoverRegister(REG_INDEX_EBX);
            }
        }
    }

    return hr;
}

HRESULT UnknownFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    HRESULT hr = m_baseTrav.GetFrameData(frameData);

    if (FALSE != frameData.m_valid.base)
    {
        frameData.m_localsBase = frameData.m_base - X86_STACK_SIZE;
        frameData.m_valid.localsBase = TRUE;
    }

    return hr;
}
