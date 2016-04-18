//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FrameDataFrameTraverser.cpp
///
//==================================================================================

#include "FrameDataFrameTraverser.h"
#include <dia2.h>

FrameDataFrameTraverser::FrameDataFrameTraverser(FrameChainWalker& walker) : m_baseTrav(walker)
{
}

void FrameDataFrameTraverser::Reset(IDiaFrameData* pFrameData, VAddrX86 stackPtr)
{
    m_baseTrav.Reset(NULL, pFrameData, stackPtr);
}

HRESULT FrameDataFrameTraverser::TraverseNext()
{
    BSTR pFrameCommand = nullptr;
    HRESULT hr = m_baseTrav.GetFrameInterface()->get_program(&pFrameCommand);

    if (S_OK == hr)
    {
        // Change .raSearchStart to .raSearch.
        if (0 == wcsncmp(pFrameCommand, L"$T2 $esp = $T0 .raSearchStart =", 31))
        {
            wmemset(pFrameCommand + wcslen(L"$T2 $esp = $T0 .raSearch"), L' ', wcslen(L"Start"));
        }

        m_baseTrav.ResetFrameCommand(pFrameCommand);
        hr = m_baseTrav.TraverseNext();

        if (SUCCEEDED(hr))
        {
            m_baseTrav.GetContext().RecoverRegister(REG_INDEX_EBP);
            m_baseTrav.GetContext().RecoverRegister(REG_INDEX_EBX);
        }

        m_baseTrav.ResetFrameCommand(NULL);
        SysFreeString(pFrameCommand);
    }
    else
    {
        return hr;
    }

    return hr;
}

HRESULT FrameDataFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    HRESULT hr = m_baseTrav.GetFrameData(frameData);

    ValueX86 vframe;

    if (m_baseTrav.GetContext().GetVFrame(vframe))
    {
        frameData.m_localsBase = vframe;
        frameData.m_valid.localsBase = TRUE;
    }

    return hr;
}

