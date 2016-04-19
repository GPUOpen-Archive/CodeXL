//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PostfixFrameEvaluator.cpp
///
//==================================================================================

#include "PostfixFrameEvaluator.h"
#include "FrameChainWalker.h"
#include <dia2.h>

#define FCC64(ch4) ((((gtUInt64)(ch4 & 0xFF)) << 48)   |    \
                    (((gtUInt64)(ch4 & 0xFF00)) << 24) |    \
                    (((gtUInt64)(ch4 & 0xFF0000)))     |    \
                    (((gtUInt64)(ch4 & 0xFF000000)) >> 24))

static const gtUInt64 s_aFrameCommandRegisters[NUM_X86_REGS] =
{
    FCC64('eax\0'),
    FCC64('ecx\0'),
    FCC64('edx\0'),
    FCC64('ebx\0'),
    FCC64('esp\0'),
    FCC64('ebp\0'),
    FCC64('esi\0'),
    FCC64('edi\0'),
    FCC64('eip\0')
};

#define ALLREG_INDEX_PARAMS  (NUM_X86_REGS + 1)
#define ALLREG_INDEX_LOCALS  (NUM_X86_REGS + 2)


#define TOKEN_SEARCH_EX_VALUE  0
#define TOKEN_SEARCH_EX_START  1


PostfixFrameEvaluator::PostfixFrameEvaluator(FrameChainWalker& walker, const wchar_t* pFrameCommand, IDiaFrameData* pFrameData) :
    m_stackTokensSize(0),
    m_pFrameCommandCurrent(pFrameCommand),
    m_pFrameCommand(pFrameCommand),
    m_lastError(S_OK),
    m_walker(walker),
    m_pFrameData(pFrameData)
{
    m_nextToken.m_type = TOKEN_UNKNOWN;
    m_nextToken.m_value = 0;

    memset(m_aTempVars, 0, sizeof(m_aTempVars));
}

HRESULT PostfixFrameEvaluator::EvaluateNextFrame()
{
    HRESULT hr = S_OK;
    m_stackTokensSize = 0;

    Token token;
    ReadNextToken(token);

    while (TOKEN_END != m_nextToken.m_type)
    {
        hr = Statement();

        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}

bool PostfixFrameEvaluator::ParseOperator(OperatorType op)
{
    bool ret = true;

    m_nextToken.m_type = TOKEN_OPERATOR;
    m_nextToken.m_value = op;
    m_pFrameCommandCurrent++;

    return ret;
}

bool PostfixFrameEvaluator::ParseRegister()
{
    bool ret = false;

    const wchar_t* pRegister = m_pFrameCommandCurrent + 1;

    if (pRegister[0] != '\0' && pRegister[1] != '\0' && pRegister[2] != '\0')
    {
        gtUInt64 regName = ((*(gtUInt64*)(pRegister)) & 0x0000FFFFFFFFFFFFULL);

        for (unsigned i = 0; i < NUM_X86_REGS; ++i)
        {
            if (s_aFrameCommandRegisters[i] == regName)
            {
                m_nextToken.m_value = i;
                m_pFrameCommandCurrent += 3 + 1;
                ret = true;
                break;
            }
        }
    }

    return ret;
}

bool PostfixFrameEvaluator::ParseStorage()
{
    bool ret = true;
    wchar_t* pNumEnd = const_cast<wchar_t*>(m_pFrameCommandCurrent);

    m_nextToken.m_type = TOKEN_REGISTER;
    m_nextToken.m_value = 0;

    switch (m_pFrameCommandCurrent[1])
    {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            m_nextToken.m_value = wcstoul(m_pFrameCommandCurrent + 1, &pNumEnd, 10) - CV_REG_EAX;

            //
            // Convert from CV_HREG_e to REG_INDEX_*.
            //

            if (m_nextToken.m_value < REG_INDEX_EIP)
            {
                m_pFrameCommandCurrent = pNumEnd;
            }
            else if (m_nextToken.m_value == (CV_REG_EIP - CV_REG_EAX))
            {
                m_nextToken.m_value = REG_INDEX_EIP;
                m_pFrameCommandCurrent = pNumEnd;
            }
            else
            {
                ret = false;
            }

            break;

        case 'T':
            m_nextToken.m_type = TOKEN_TEMP;
            m_nextToken.m_value = wcstoul(m_pFrameCommandCurrent + 2, &pNumEnd, 10);

            ret = (MAX_TEMP_VARS > m_nextToken.m_value);

            if (ret)
            {
                m_pFrameCommandCurrent = pNumEnd;
            }

            break;

        case 'L':
            m_nextToken.m_value = ALLREG_INDEX_LOCALS;
            m_pFrameCommandCurrent += 1 + 1;
            break;

        case 'P':
            m_nextToken.m_value = ALLREG_INDEX_PARAMS;
            m_pFrameCommandCurrent += 1 + 1;
            break;

        case 'V':
            m_nextToken.m_type = TOKEN_VARIABLE;
            m_nextToken.m_value = wcstoul(m_pFrameCommandCurrent + 2, &pNumEnd, 10);

            ret = (StackWalkContextX86::MAX_VARIABLES > m_nextToken.m_value);

            if (ret)
            {
                m_pFrameCommandCurrent = pNumEnd;
            }

            break;

        default:
            ret = ParseRegister();
            break;
    }

    return ret;
}

bool PostfixFrameEvaluator::ParseIntrinsic()
{
    bool ret = true;

    const wchar_t* pIntrinsic = m_pFrameCommandCurrent + 1;

    m_nextToken.m_type = TOKEN_INTRINSIC;

    if (!wcsncmp(pIntrinsic, L"ulRvaStart", 0xA))
    {
        DWORD ulRvaStart = 0;
        m_pFrameData->get_relativeVirtualAddress(&ulRvaStart);
        m_pFrameCommandCurrent += 0xA + 1;
        m_nextToken.m_value = ulRvaStart;
    }
    else if (!wcsncmp(pIntrinsic, L"cbBlock", 7))
    {
        DWORD cbBlock = 0;
        m_pFrameData->get_lengthBlock(&cbBlock);
        m_pFrameCommandCurrent += 7 + 1;
        m_nextToken.m_value = cbBlock;
    }
    else if (!wcsncmp(pIntrinsic, L"cbLocals", 8))
    {
        DWORD cbLocals = 0;
        m_pFrameData->get_lengthLocals(&cbLocals);
        m_pFrameCommandCurrent += 8 + 1;
        m_nextToken.m_value = cbLocals;
    }
    else if (!wcsncmp(pIntrinsic, L"cbParams", 8))
    {
        DWORD cbParams = 0;
        m_pFrameData->get_lengthParams(&cbParams);
        m_pFrameCommandCurrent += 8 + 1;
        m_nextToken.m_value = cbParams;
    }
    else if (!wcsncmp(pIntrinsic, L"cbStkMax", 8))
    {
        DWORD cbStkMax = 0;
        m_pFrameData->get_maxStack(&cbStkMax);
        m_pFrameCommandCurrent += 8 + 1;
        m_nextToken.m_value = cbStkMax;
    }
    else if (!wcsncmp(pIntrinsic, L"frameFunc", 9))
    {
        m_pFrameCommandCurrent += 9 + 1;
    }

    if (!wcsncmp(pIntrinsic, L"cbProlog", 8))
    {
        DWORD cbProlog = 0;
        m_pFrameData->get_lengthProlog(&cbProlog);
        m_pFrameCommandCurrent += 8 + 1;
        m_nextToken.m_value = cbProlog;
    }
    else if (!wcsncmp(pIntrinsic, L"cbSavedRegs", 0xB))
    {
        DWORD cbSavedRegs = 0;
        m_pFrameData->get_lengthSavedRegisters(&cbSavedRegs);
        m_pFrameCommandCurrent += 0xB + 1;
        m_nextToken.m_value = cbSavedRegs;
    }
    else if (!wcsncmp(pIntrinsic, L"raSearchStart", 0xD))
    {
        m_pFrameCommandCurrent += 0xD + 1;
        m_nextToken.m_type = TOKEN_SEARCH;
        m_nextToken.m_value = TOKEN_SEARCH_EX_START;
    }
    else if (!wcsncmp(pIntrinsic, L"raSearch", 8))
    {
        m_pFrameCommandCurrent += 8 + 1;
        m_nextToken.m_type = TOKEN_SEARCH;
        m_nextToken.m_value = TOKEN_SEARCH_EX_VALUE;
    }
    else
    {
        ret = false;
    }

    return ret;
}


bool PostfixFrameEvaluator::ReadNextToken(Token& token)
{
    bool ret = false;

    token = m_nextToken;

    while (isspace(*m_pFrameCommandCurrent))
    {
        ++m_pFrameCommandCurrent;
    }

    if (*m_pFrameCommandCurrent <= '^')
    {
        switch (*m_pFrameCommandCurrent)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                m_nextToken.m_type = TOKEN_NUMBER;
                m_nextToken.m_value = wcstoul(m_pFrameCommandCurrent, const_cast<wchar_t**>(&m_pFrameCommandCurrent), 10);
                ret = true;
                break;

            case '=':
                m_nextToken.m_type = TOKEN_ASSIGN;
                m_nextToken.m_value = TOKEN_ASSIGN;
                m_pFrameCommandCurrent++;
                ret = true;
                break;

            case '$':
                ret = ParseStorage();
                break;

            case '.':
                ret = ParseIntrinsic();
                break;

            case '+':
                ret = ParseOperator(OPERATOR_ADD);
                break;

            case '-':
                ret = ParseOperator(OPERATOR_SUBTRACT);
                break;

            case '*':
                ret = ParseOperator(OPERATOR_MULTIPLY);
                break;

            case '/':
                ret = ParseOperator(OPERATOR_DIVIDE);
                break;

            case '%':
                ret = ParseOperator(OPERATOR_MODULUS);
                break;

            case '@':
                ret = ParseOperator(OPERATOR_ALIGN);
                break;

            case '^':
                ret = ParseOperator(OPERATOR_DEREFERENCE);
                break;

            case '\0':
                m_nextToken.m_type = TOKEN_END;
                ret = true;
                break;

            default:
                break;
        }
    }

    return ret;
}

HRESULT PostfixFrameEvaluator::Statement()
{
    HRESULT hr = E_DIA_SYNTAX;
    Token tokenLeft;

    if (ReadNextToken(tokenLeft))
    {
        //
        // If this is a storage typed token, then we need to evaluate the rvalue.
        //

        if (TOKEN_REGISTER == tokenLeft.m_type || TOKEN_TEMP == tokenLeft.m_type || TOKEN_VARIABLE == tokenLeft.m_type)
        {
            do
            {
                if (TOKEN_OPERATOR == m_nextToken.m_type)
                {
                    hr = Operator();
                }
                else
                {
                    hr = Operand();
                }

                if (FAILED(hr))
                {
                    hr = E_DIA_SYNTAX;
                    break;
                }

            }
            while (TOKEN_ASSIGN != m_nextToken.m_type);

            if (S_OK == hr)
            {
                Token tokenRight;

                if (ReadNextToken(tokenRight) && TOKEN_ASSIGN == tokenRight.m_type && PopToken(tokenRight))
                {
                    if (TOKEN_REGISTER == tokenLeft.m_type)
                    {
                        if (ALLREG_INDEX_LOCALS == tokenLeft.m_value)
                        {
                            m_walker.GetContext().SetLocals(tokenRight.m_value);
                        }
                        else if (ALLREG_INDEX_PARAMS == tokenLeft.m_value)
                        {
                            m_walker.GetContext().SetParams(tokenRight.m_value);
                        }
                        else
                        {
                            m_walker.GetContext().SetRegister(tokenLeft.m_value, tokenRight.m_value);
                        }
                    }
                    else if (TOKEN_TEMP == tokenLeft.m_type)
                    {
                        m_aTempVars[tokenLeft.m_value] = tokenRight.m_value;

                        if (0 == tokenLeft.m_value)
                        {
                            m_walker.GetContext().SetVFrame(tokenRight.m_value);
                        }
                    }
                    else
                    {
                        m_walker.GetContext().SetVariable(tokenLeft.m_value, tokenRight.m_value);
                    }

                    // By the time we have finished evaluating the statement we should not have any pending tokens on the stack.
                    if (0 != m_stackTokensSize)
                    {
                        hr = E_DIA_SYNTAX;
                    }
                }
                else
                {
                    hr = E_DIA_SYNTAX;
                }
            }
        }
    }

    if (S_OK != hr)
    {
        m_lastError = hr;
    }

    return hr;
}

HRESULT PostfixFrameEvaluator::Operator()
{
    HRESULT hr = E_DIA_SYNTAX;
    Token tokenOp, tokenRight;

    if (ReadNextToken(tokenOp) && PopToken(tokenRight))
    {
        Token tokenLeft = { TOKEN_UNKNOWN, 0 };

        if (OPERATOR_DEREFERENCE == tokenOp.m_value || PopToken(tokenLeft))
        {
            Token tokenResult;
            tokenResult.m_type = TOKEN_NUMBER;

            switch (tokenOp.m_value)
            {
                case OPERATOR_ADD:
                    tokenResult.m_value = tokenLeft.m_value + tokenRight.m_value;
                    PushToken(tokenResult);
                    hr = S_OK;
                    break;

                case OPERATOR_SUBTRACT:
                    tokenResult.m_value = tokenLeft.m_value - tokenRight.m_value;
                    PushToken(tokenResult);
                    hr = S_OK;
                    break;

                case OPERATOR_MULTIPLY:
                    tokenResult.m_value = tokenLeft.m_value * tokenRight.m_value;
                    PushToken(tokenResult);
                    hr = S_OK;
                    break;

                case OPERATOR_DIVIDE:
                    if (0 != tokenRight.m_value)
                    {
                        tokenResult.m_value = tokenLeft.m_value / tokenRight.m_value;
                        PushToken(tokenResult);
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }

                    break;

                case OPERATOR_MODULUS:
                    if (0 != tokenRight.m_value)
                    {
                        tokenResult.m_value = tokenLeft.m_value % tokenRight.m_value;
                        PushToken(tokenResult);
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }

                    break;

                case OPERATOR_ALIGN:
                    if (0 != tokenRight.m_value)
                    {
                        tokenResult.m_value = tokenLeft.m_value & ~(tokenRight.m_value - 1);
                        PushToken(tokenResult);
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }

                    break;

                case OPERATOR_DEREFERENCE:
                    if (m_walker.GetContext().ReadFullMemory(MEM_TYPE_ANY, tokenRight.m_value, tokenResult.m_value))
                    {
                        PushToken(tokenResult);
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_DIA_FRAME_ACCESS;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    if (S_OK != hr)
    {
        m_lastError = hr;
    }

    return hr;
}

HRESULT PostfixFrameEvaluator::Operand()
{
    HRESULT hr;
    Token tokenOp;

    if (ReadNextToken(tokenOp))
    {
        Token tokenResult;
        tokenResult.m_type = TOKEN_NUMBER;

        switch (tokenOp.m_type)
        {
            case TOKEN_REGISTER:
                if (m_walker.GetContext().GetRegister(tokenOp.m_value, tokenResult.m_value))
                {
                    PushToken(tokenResult);
                    hr = S_OK;
                }
                else
                {
                    m_lastError = E_DIA_FRAME_ACCESS;
                    hr = E_DIA_FRAME_ACCESS;
                }

                break;

            case TOKEN_TEMP:
                tokenResult.m_value = m_aTempVars[tokenOp.m_value];
                PushToken(tokenResult);
                hr = S_OK;
                break;

            case TOKEN_VARIABLE:
                if (m_walker.GetContext().GetVariable(tokenOp.m_value, tokenResult.m_value))
                {
                    PushToken(tokenResult);
                    hr = S_OK;
                }
                else
                {
                    m_lastError = E_DIA_FRAME_ACCESS;
                    hr = E_DIA_FRAME_ACCESS;
                }

                break;

            case TOKEN_SEARCH:
                hr = EvaluateSearchToken(tokenOp);

                if (FAILED(hr))
                {
                    break;
                }

            case TOKEN_INTRINSIC:
            case TOKEN_NUMBER:
                tokenResult.m_value = tokenOp.m_value;
                PushToken(tokenResult);
                hr = S_OK;
                break;

            default:
                hr = E_DIA_SYNTAX;
                m_lastError = hr;
                break;
        }
    }
    else
    {
        hr = E_DIA_SYNTAX;
        m_lastError = hr;
    }

    return hr;
}

HRESULT PostfixFrameEvaluator::EvaluateSearchToken(Token& token)
{
    HRESULT hr;
    VAddrX86 returnAddress = static_cast<VAddrX86>(-1);

    if (TOKEN_SEARCH_EX_VALUE == token.m_value)
    {
        hr = m_walker.SearchForReturnAddress(m_pFrameData, returnAddress);
    }
    else if (TOKEN_SEARCH_EX_START == token.m_value)
    {
        hr = m_walker.SearchForReturnAddressStart(m_pFrameData, m_aTempVars[2], returnAddress);
    }
    else
    {
        hr = E_DIA_SYNTAX;
    }

    if (S_OK == hr)
    {
        token.m_value = returnAddress;
    }

    return hr;
}

bool PostfixFrameEvaluator::PushToken(Token& token)
{
    bool ret = (STACK_TOKENS_CAP > m_stackTokensSize);

    if (ret)
    {
        m_stackTokens[m_stackTokensSize++] = token;
    }

    return ret;
}

bool PostfixFrameEvaluator::PopToken(Token& token)
{
    bool ret = (0 < m_stackTokensSize);

    if (ret)
    {
        token = m_stackTokens[--m_stackTokensSize];
    }

    return ret;
}
