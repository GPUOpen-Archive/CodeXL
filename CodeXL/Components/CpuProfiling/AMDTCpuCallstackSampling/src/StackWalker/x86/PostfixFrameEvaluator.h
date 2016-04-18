//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PostfixFrameEvaluator.h
///
//==================================================================================

#ifndef _POSTFIXFRAMEEVALUATOR_H_
#define _POSTFIXFRAMEEVALUATOR_H_

#include "StackWalkContextX86.h"

class FrameChainWalker;

class PostfixFrameEvaluator
{
    enum TokenType
    {
        TOKEN_UNKNOWN,
        TOKEN_END,
        TOKEN_OPERATOR,
        TOKEN_REGISTER,
        TOKEN_TEMP,
        TOKEN_VARIABLE, //TODO: ?? is this the correct name??

        TOKEN_INTRINSIC = 7,
        TOKEN_SEARCH,
        TOKEN_NUMBER,
        TOKEN_ASSIGN,
    };

    struct Token
    {
        TokenType m_type;
        ValueX86 m_value;
    };

    enum OperatorType
    {
        OPERATOR_ADD,
        OPERATOR_SUBTRACT,
        OPERATOR_MULTIPLY,
        OPERATOR_DIVIDE,
        OPERATOR_DEREFERENCE,
        OPERATOR_ALIGN,

        OPERATOR_MODULUS // Unofficial!
    };

public:
    PostfixFrameEvaluator(FrameChainWalker& walker, const wchar_t* pFrameCommand, struct IDiaFrameData* pFrameData);
    PostfixFrameEvaluator& operator=(const PostfixFrameEvaluator&) = delete;
    HRESULT EvaluateNextFrame();

private:
    bool ParseOperator(OperatorType op);
    bool ParseRegister();
    bool ParseStorage();
    bool ParseIntrinsic();

    bool ReadNextToken(Token& token);

    HRESULT Statement();
    HRESULT Operator();
    HRESULT Operand();
    HRESULT EvaluateSearchToken(Token& token);

    bool PushToken(Token& token);
    bool PopToken(Token& token);

private:
    enum { STACK_TOKENS_CAP = 19, MAX_TEMP_VARS = 10 };
    Token m_nextToken; // dw0..3

    Token m_stackTokens[STACK_TOKENS_CAP];
    unsigned m_stackTokensSize;
    const wchar_t* m_pFrameCommandCurrent;
    const wchar_t* const m_pFrameCommand;

    ValueX86 m_aTempVars[MAX_TEMP_VARS];

    HRESULT m_lastError;
    FrameChainWalker& m_walker;
    struct IDiaFrameData* m_pFrameData;
};


#endif // _POSTFIXFRAMEEVALUATOR_H_
