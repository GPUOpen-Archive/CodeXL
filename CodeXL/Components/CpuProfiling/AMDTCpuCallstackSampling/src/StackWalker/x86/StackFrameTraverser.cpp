//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackFrameTraverser.cpp
///
//==================================================================================

#include "StackFrameTraverser.h"
#include "DisassemblerX86.h"
#include "PostfixFrameEvaluator.h"
#include "FrameChainWalker.h"
#include <dia2.h>

StackFrameTraverser::StackFrameTraverser(FrameChainWalker& walker) : m_walker(walker),
    m_pFrameCommand(NULL),
    m_pFrameData(NULL),
    m_stackPtr(0),
    m_needEvaluation(true)
{
}

StackFrameTraverser::~StackFrameTraverser()
{
    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }
}

void StackFrameTraverser::Reset(const wchar_t* pFrameCommand, IDiaFrameData* pFrameData, VAddrX86 stackPtr)
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
    m_pFrameCommand = pFrameCommand;
    m_stackPtr = stackPtr;
    m_needEvaluation = true;
}

HRESULT StackFrameTraverser::TraverseNext()
{
    HRESULT hr;

    if (NULL != m_pFrameCommand && NULL != m_pFrameData && 0 != m_stackPtr)
    {
        if (m_needEvaluation)
        {
            m_needEvaluation = false;
            PostfixFrameEvaluator postfixEval(m_walker, m_pFrameCommand, m_pFrameData);
            hr = postfixEval.EvaluateNextFrame();
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT StackFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    return GetFrameData(GetContext(), m_stackPtr, m_pFrameData, frameData);
}

HRESULT StackFrameTraverser::GetFrameData(const StackWalkContextX86& context, VAddrX86 stackPtr, IDiaFrameData* pFrameData, StackFrameData& frameData)
{
    HRESULT hr = E_FAIL;
    VAddrX86 baseAddr;

    if (context.GetRegister(REG_INDEX_ESP, baseAddr))
    {
        baseAddr -= X86_STACK_SIZE;
        frameData.m_base = baseAddr;
        frameData.m_valid.base = TRUE;

        frameData.m_size = baseAddr - stackPtr;
        frameData.m_valid.size = TRUE;

        VAddrX86 returnAddr;

        if (context.GetRegister(REG_INDEX_EIP, returnAddr))
        {
            frameData.m_returnAddress = returnAddr;
            frameData.m_valid.returnAddress = TRUE;

            if (NULL != pFrameData)
            {
                DWORD dwValue;
                hr = pFrameData->get_type(&dwValue);

                if (S_OK == hr)
                {
                    BOOL bValue;

                    frameData.m_type = static_cast<StackFrameTypeEnum>(dwValue);
                    frameData.m_valid.type = TRUE;

                    if (S_OK == pFrameData->get_lengthLocals(&dwValue))
                    {
                        frameData.m_lengthLocals = dwValue;
                        frameData.m_valid.lengthLocals = TRUE;
                    }

                    if (S_OK == pFrameData->get_lengthParams(&dwValue))
                    {
                        frameData.m_lengthParams = dwValue;
                        frameData.m_valid.lengthParams = TRUE;
                    }

                    if (S_OK == pFrameData->get_lengthProlog(&dwValue))
                    {
                        frameData.m_lengthProlog = dwValue;
                        frameData.m_valid.lengthProlog = TRUE;
                    }

                    if (S_OK == pFrameData->get_lengthSavedRegisters(&dwValue))
                    {
                        frameData.m_lengthSavedRegisters = dwValue;
                        frameData.m_valid.lengthSavedRegisters = TRUE;
                    }

                    if (S_OK == pFrameData->get_systemExceptionHandling(&bValue))
                    {
                        frameData.m_systemExceptionHandling = bValue;
                        frameData.m_valid.systemExceptionHandling = TRUE;
                    }

                    if (S_OK == pFrameData->get_cplusplusExceptionHandling(&bValue))
                    {
                        frameData.m_cplusplusExceptionHandling = bValue;
                        frameData.m_valid.cplusplusExceptionHandling = TRUE;
                    }

                    if (S_OK == pFrameData->get_functionStart(&bValue))
                    {
                        frameData.m_functionStart = bValue;
                        frameData.m_valid.functionStart = TRUE;
                    }

                    if (S_OK == pFrameData->get_allocatesBasePointer(&bValue))
                    {
                        frameData.m_allocatesBasePointer = bValue;
                        frameData.m_valid.allocatesBasePointer = TRUE;
                    }

                    if (S_OK == pFrameData->get_maxStack(&dwValue))
                    {
                        frameData.m_maxStack = dwValue;
                        frameData.m_valid.maxStack = TRUE;
                    }
                }
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    return hr;
}

const StackWalkContextX86& StackFrameTraverser::GetContext() const
{
    return m_walker.GetContext();
}

StackWalkContextX86& StackFrameTraverser::GetContext()
{
    return m_walker.GetContext();
}
