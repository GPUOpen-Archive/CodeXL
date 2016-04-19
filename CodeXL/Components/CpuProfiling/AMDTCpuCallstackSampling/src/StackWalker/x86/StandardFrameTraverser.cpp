//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StandardFrameTraverser.cpp
///
//==================================================================================

#include "StandardFrameTraverser.h"
#include <dia2.h>

static const wchar_t* s_pStandardFrameCommand = L"$T0 $ebp = $eip $T0 4 + ^ = $ebp $T0 ^ = $esp $T0 8 + =";

HRESULT StandardFrameTraverser::IsStandardFrame(IDiaFrameData* pFrameData)
{
    HRESULT hr = S_FALSE;

    DWORD type;

    if (S_OK == pFrameData->get_type(&type))
    {
        if (FrameTypeStandard == type)
        {
            hr = S_OK;
        }
        else if (FrameTypeFrameData == type)
        {
            BSTR pFrameCommand = NULL;
            hr = pFrameData->get_program(&pFrameCommand);

            if (SUCCEEDED(hr))
            {
                hr = S_FALSE;

                const unsigned int lenStdCmd = wcslen(s_pStandardFrameCommand);
                unsigned int lenFrameCmd = SysStringLen(pFrameCommand);

                while (lenStdCmd < lenFrameCmd)
                {
                    if (!isspace(pFrameCommand[lenFrameCmd - 1]))
                    {
                        break;
                    }

                    --lenFrameCmd;
                }

                if (lenFrameCmd == lenStdCmd && memcmp(pFrameCommand, s_pStandardFrameCommand, lenStdCmd * sizeof(wchar_t)))
                {
                    hr = S_OK;
                }

                SysFreeString(pFrameCommand);
            }
        }
    }

    return hr;
}

StandardFrameTraverser::StandardFrameTraverser(FrameChainWalker& walker) : m_baseTrav(walker)
{
}

void StandardFrameTraverser::Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr)
{
    m_baseTrav.Reset(s_pStandardFrameCommand, pFrameData, stackPtr);
}

HRESULT StandardFrameTraverser::TraverseNext()
{
    return m_baseTrav.TraverseNext();
}

HRESULT StandardFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    HRESULT hr = m_baseTrav.GetFrameData(frameData);

    if (FALSE != frameData.m_valid.base)
    {
        frameData.m_localsBase = frameData.m_base - X86_STACK_SIZE;
        frameData.m_valid.localsBase = TRUE;
    }

    return hr;
}


StandardFrameNoEbpTraverser::StandardFrameNoEbpTraverser(FrameChainWalker& walker) : m_baseTrav(walker)
{
}

void StandardFrameNoEbpTraverser::Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr)
{
    m_baseTrav.Reset(L"$T0 .raSearch 4 - = $ebp $T0 ^ = $eip $T0 4 + ^ = $esp $T0 8 + =", pFrameData, stackPtr);
}

HRESULT StandardFrameNoEbpTraverser::TraverseNext()
{
    return m_baseTrav.TraverseNext();
}

HRESULT StandardFrameNoEbpTraverser::GetFrameData(StackFrameData& frameData) const
{
    HRESULT hr = m_baseTrav.GetFrameData(frameData);

    if (FALSE != frameData.m_valid.base)
    {
        frameData.m_localsBase = frameData.m_base - X86_STACK_SIZE;
        frameData.m_valid.localsBase = TRUE;
    }

    return hr;
}
