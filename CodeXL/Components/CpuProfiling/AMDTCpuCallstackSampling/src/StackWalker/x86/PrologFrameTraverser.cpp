//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrologFrameTraverser.cpp
///
//==================================================================================

#include "PrologFrameTraverser.h"
#include "DisassemblerX86.h"
#include <dia2.h>

enum PrologPrefixType
{
    PROLOGPREFIX_NONE,
    PROLOGPREFIX_EH,
    PROLOGPREFIX_SEH
};

static PrologPrefixType DeterminePrefix(const wchar_t* pFuncName, unsigned int lenFuncName, unsigned int& lenPrefix);
static bool TrimAlignSuffix(const wchar_t* pFuncName, unsigned int& lenFuncName, unsigned int lenPrefix);


PrologFrameTraverser::PrologFrameTraverser(StackWalkContextX86& context) : m_context(context),
    m_pFrameData(NULL),
    m_stackPtr(0),
    m_codeLength(0),
    m_instCount(0)
{
}

PrologFrameTraverser::~PrologFrameTraverser()
{
    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }
}

void PrologFrameTraverser::Reset(IDiaFrameData* pFrameData, VAddrX86 stackPtr)
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

    if (0 == stackPtr)
    {
        m_context.GetRegister(REG_INDEX_ESP, m_stackPtr);
    }
    else
    {
        m_stackPtr = stackPtr;
    }

    m_codeLength = 0;
    m_instCount = 0;
}

HRESULT PrologFrameTraverser::TraverseNext()
{
    HRESULT hr = E_FAIL;
    VAddrX86 retAddr;
    ULARGE_INTEGER frameAddr;

    if (m_context.GetRegister(REG_INDEX_EIP, retAddr) && S_OK == (hr = m_pFrameData->get_virtualAddress(&frameAddr.QuadPart)))
    {
        hr = ProcessProlog(frameAddr.LowPart, retAddr);

        if (SUCCEEDED(hr))
        {
            VAddrX86 stackPtr;

            if (m_context.GetRegister(REG_INDEX_ESP, stackPtr))
            {
                m_context.SetVFrame(stackPtr);
                hr = PopRegister(REG_INDEX_EIP);

                if (S_OK == hr)
                {
                    if (m_context.GetRegister(REG_INDEX_ESP, stackPtr))
                    {
                        DWORD lengthParams;
                        hr = m_pFrameData->get_lengthParams(&lengthParams);

                        if (SUCCEEDED(hr))
                        {
                            m_context.SetParams(stackPtr + lengthParams);
                            hr = S_OK;
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessProlog(VAddrX86 frameAddr, VAddrX86 retAddr)
{
    HRESULT hr = CrackInstructions(frameAddr, retAddr);

    if (S_OK == hr)
    {
        if (0 != m_instCount)
        {
            const gtUByte** ppInst = m_aInst + m_instCount;

            for (unsigned i = 0; i < m_instCount; ++i)
            {
                const gtUByte* pInst = *(--ppInst);
                VAddrX86 instAddr = frameAddr + static_cast<RVAddrX86>(pInst - m_aCode);
                hr = ProcessInstruction(instAddr, pInst);

                if (S_OK != hr)
                {
                    break;
                }
            }
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::CrackInstructions(VAddrX86 frameAddr, VAddrX86 retAddr)
{
    HRESULT hr;

    if ((retAddr - frameAddr) <= sizeof(m_aCode))
    {
        if (retAddr == frameAddr)
        {
            hr = S_OK;
        }
        else
        {
            m_codeLength = m_context.ReadMemory(MEM_TYPE_CODE, frameAddr, m_aCode, static_cast<int>(retAddr - frameAddr));

            if (0 <= m_codeLength)
            {
                DisassemblerX86 dasm;
                m_instCount = 0;

                gtUByte const* pCode = m_aCode;
                gtUByte const* const pCodeEnd = m_aCode + m_codeLength;

                while (pCode < pCodeEnd)
                {
                    m_aInst[m_instCount++] = pCode;
                    pCode = dasm.CrackInstruction(pCode);
                }

                // Validate that we have reached the end, which means that we are in a valid location.
                hr = (pCode == pCodeEnd) ? S_OK : E_DIA_FRAME_ACCESS;
            }
            else
            {
                hr = E_DIA_FRAME_ACCESS;
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT PrologFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    HRESULT hr = E_FAIL;
    ValueX86 regVal;

    if (m_context.GetRegister(REG_INDEX_ESP, regVal))
    {
        frameData.m_base = regVal - sizeof(DWORD);
        frameData.m_valid.base = TRUE;

        frameData.m_size = static_cast<DWORD>(regVal - m_stackPtr - X86_STACK_SIZE);
        frameData.m_valid.size = TRUE;

        if (m_context.GetRegister(REG_INDEX_EIP, regVal))
        {
            frameData.m_returnAddress = regVal;
            frameData.m_valid.returnAddress = TRUE;

            if (m_context.GetVFrame(regVal))
            {
                frameData.m_localsBase = regVal;
                frameData.m_valid.localsBase = TRUE;
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::PopRegister(unsigned index)
{
    HRESULT hr = E_DIA_FRAME_ACCESS;
    VAddrX86 stackPtr;

    if (m_context.GetRegister(REG_INDEX_ESP, stackPtr))
    {
        ValueX86 val;

        if (m_context.ReadFullMemory(MEM_TYPE_STACK, stackPtr, val))
        {
            m_context.SetRegister(index, val);
            m_context.SetRegister(REG_INDEX_ESP, stackPtr + X86_STACK_SIZE);
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::AdjustRegister(unsigned index, gtInt32 bytesCount)
{
    HRESULT hr;
    VAddrX86 stackPtr;

    if (m_context.GetRegister(index, stackPtr))
    {
        m_context.SetRegister(index, stackPtr + bytesCount);
        hr = S_OK;
    }
    else
    {
        hr = E_DIA_FRAME_ACCESS;
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessInstruction(VAddrX86 instAddr, const gtUByte* pInst)
{
    HRESULT hr;

    if (S_FALSE == (hr = ProcessPushInstruction(pInst))     &&
        S_FALSE == (hr = ProcessPopInstruction(pInst))      &&
        S_FALSE == (hr = ProcessAddSubInstruction(pInst))   &&
        S_FALSE == (hr = ProcessMovInstruction(pInst))      &&
        S_FALSE == (hr = ProcessCallInstruction(instAddr, pInst)))
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessPushInstruction(const gtUByte* pCode)
{
    //
    // +--------+-------------+-------------+
    // | Opcode | Instruction | Description |
    // +--------+-------------+-------------+
    // | 50+rd  | PUSH r32    | Push r32.   |
    // | 6A id  | PUSH imm8   | Push imm8.  |
    // | 68 id  | PUSH imm32  | Push imm32. |
    // +--------+-------------+-------------+
    //

    HRESULT hr = S_FALSE;

    gtUByte reg = (pCode[0] - ((gtUByte)0x50));

    if (reg <= ((gtUByte)7))
    {
        hr = PopRegister(REG_INDEX_EAX + reg); //TODO: Shouldn't this be a PUSH instead of POP??
    }
    else if (0x6A == pCode[0] || 0x68 == pCode[0])
    {
        hr = AdjustRegister(REG_INDEX_ESP, X86_STACK_SIZE);
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessPopInstruction(const gtUByte* pCode)
{
    //
    // +--------+-------------+-----------------------------------------------------+
    // | Opcode | Instruction | Description                                         |
    // +--------+-------------+-----------------------------------------------------+
    // | 58+ rd | POP r32     | Pop top of stack into r32; increment stack pointer. |
    // +--------+-------------+-----------------------------------------------------+
    //

    HRESULT hr = S_FALSE;

    gtUByte reg = (pCode[0] - ((gtUByte)0x58));

    if (reg <= ((gtUByte)7))
    {
        hr = AdjustRegister(REG_INDEX_ESP, -static_cast<gtInt32>(X86_STACK_SIZE));
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessAddSubInstruction(const gtUByte* pCode)
{
    //
    // +----------+------------------+-----------------------------------------+
    // | Opcode   | Instruction      | Description                             |
    // +----------+------------------+-----------------------------------------+
    // | 81 /0 id | ADD r/m32, imm32 | Add imm32 from r/m32.                   |
    // | 83 /0 ib | ADD r/m32, imm8  | Add sign-extended imm8 from r/m32.      |
    // |          |                  |                                         |
    // | 81 /5 id | SUB r/m32, imm32 | Subtract imm32 from r/m32.              |
    // | 83 /5 ib | SUB r/m32, imm8  | Subtract sign-extended imm8 from r/m32. |
    // +----------+------------------+-----------------------------------------+
    //

    HRESULT hr = S_FALSE;

    if (0x83 == pCode[0])
    {
        gtUByte opcodeExt = ((MODRM_OPCODE_EXT_MASK | MODRM_MOD_MASK) & pCode[1]);

        // ADD r/m32, imm8
        if (((0 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER) == opcodeExt)
        {
            gtUByte reg = (MODRM_RM_OFFSET & pCode[1]);
            gtInt32 addend = static_cast<gtInt32>(*reinterpret_cast<const signed char*>(pCode + 2));
            hr = AdjustRegister(REG_INDEX_EAX + reg, -addend);
        }
        // SUB r/m32, imm8
        else if (((5 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER) == opcodeExt)
        {
            gtUByte reg = (MODRM_RM_OFFSET & pCode[1]);
            gtInt32 subtrahend = static_cast<gtInt32>(*reinterpret_cast<const signed char*>(pCode + 2));
            hr = AdjustRegister(REG_INDEX_EAX + reg, subtrahend);
        }
    }
    else if (0x81 == pCode[0])
    {
        gtUByte opcodeExt = ((MODRM_OPCODE_EXT_MASK | MODRM_MOD_MASK) & pCode[1]);

        // ADD r/m32, imm32
        if (((0 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER) == opcodeExt)
        {
            gtUByte reg = (MODRM_RM_OFFSET & pCode[1]);
            gtInt32 addend = *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 2);
            hr = AdjustRegister(REG_INDEX_EAX + reg, -addend);
        }
        // SUB r/m32, imm32
        else if (((5 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER) == opcodeExt)
        {
            gtUByte reg = (MODRM_RM_OFFSET & pCode[1]);
            gtInt32 subtrahend = *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 2);
            hr = AdjustRegister(REG_INDEX_EAX + reg, subtrahend);
        }
    }

    return hr;
}


HRESULT PrologFrameTraverser::ProcessMovInstruction(const gtUByte* pCode)
{
    //
    // +----------+------------------+-----------------------------------------+
    // | Opcode   | Instruction      | Description                             |
    // +----------+------------------+-----------------------------------------+
    // | 89 /r    | MOV r/m32,r32    | Move r32 to r/m32.                      |
    // | 8B /r    | MOV r32,r/m32    | Move r/m32 to r32.                      |
    // +----------+------------------+-----------------------------------------+
    //

    HRESULT hr = (0x89 == pCode[0] || 0x8B == pCode[0]) ? S_OK : S_FALSE;

    if (S_OK == hr)
    {
        if (MODRM_MOD32_REGISTER == (MODRM_MOD_MASK & pCode[1]))
        {
            unsigned src, dst;

            if (0x89 == pCode[0])
            {
                src = (MODRM_REG_MASK & pCode[1]) >> MODRM_REG_OFFSET;
                dst = (MODRM_RM_MASK  & pCode[1]) >> MODRM_RM_OFFSET;
            }
            else
            {
                src = (MODRM_RM_MASK  & pCode[1]) >> MODRM_RM_OFFSET;
                dst = (MODRM_REG_MASK & pCode[1]) >> MODRM_REG_OFFSET;
            }

            if (src == REGISTER_X86_ESP || src == REGISTER_X86_EBP || src == REGISTER_X86_EBX)
            {
                ValueX86 val;

                if (m_context.GetRegister(REG_INDEX_EAX + dst, val))
                {
                    m_context.SetRegister(REG_INDEX_EAX + src, val);
                }
            }
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessCallInstruction(VAddrX86 instAddr, const gtUByte* pInst)
{
    //
    // +----------+------------------+-----------------------------------------------------------------+
    // | Opcode   | Instruction      | Description                                                     |
    // +----------+------------------+-----------------------------------------------------------------+
    // | E8 cd    | CALL rel32       | Call near, relative, displacement relative to next instruction. |
    // +----------+------------------+-----------------------------------------------------------------+
    //

    HRESULT hr = S_FALSE;

    if (OPCODE_CALL_REL == pInst[0])
    {
        VAddrX86 calledFuncAddr = instAddr + INST_CALL_REL_SIZE + *reinterpret_cast<const gtInt32 UNALIGNED*>(pInst + 1);
        IDiaSymbol* pFuncSymbol = NULL;

        m_context.FindSymbolInterface(calledFuncAddr, &pFuncSymbol);

        if (NULL != pFuncSymbol)
        {
            BSTR pFuncName = NULL;
            pFuncSymbol->get_name(&pFuncName);
            pFuncSymbol->Release();

            if (NULL != pFuncName)
            {
                unsigned int lenFuncName = SysStringLen(pFuncName);
                unsigned int lenPrefix;
                PrologPrefixType type = DeterminePrefix(pFuncName, lenFuncName, lenPrefix);

                if (PROLOGPREFIX_EH == type)
                {
                    // __EH_prolog
                    // _EH_prolog
                    if (L'\0' == pFuncName[lenPrefix])
                    {
                        hr = ProcessEhProlog();
                    }
                    // __EH_prolog2
                    // _EH_prolog2
                    else if (L'2' == pFuncName[lenPrefix] && L'\0' == pFuncName[lenPrefix + 1])
                    {
                        hr = ProcessEhProlog2();
                    }
                    else if (L'3' == pFuncName[lenPrefix])
                    {
                        lenPrefix++;
                        bool isAlign = TrimAlignSuffix(pFuncName, lenFuncName, lenPrefix);
                        unsigned int lenSuffix = lenFuncName - lenPrefix;

#define EQ_SUFFIX(lit) (wcslen(lit) == lenSuffix && memcmp(pFuncName + lenPrefix, lit, wcslen(lit) * sizeof(wchar_t)))

                        if (0U == lenSuffix ||
                            EQ_SUFFIX(L"_catch") ||
                            EQ_SUFFIX(L"_GS") ||
                            EQ_SUFFIX(L"_catch_GS"))
                        {
                            // __EH_prolog3_align
                            // _EH_prolog3_align
                            // __EH_prolog3_catch_align
                            // _EH_prolog3_catch_align
                            // __EH_prolog3_GS_align
                            // _EH_prolog3_GS_align
                            // __EH_prolog3_catch_GS_align
                            // _EH_prolog3_catch_GS_align
                            if (isAlign)
                            {
                                hr = ProcessEhProlog3Align();
                            }
                            // __EH_prolog3
                            // _EH_prolog3
                            // __EH_prolog3_catch
                            // _EH_prolog3_catch
                            // __EH_prolog3_GS
                            // _EH_prolog3_GS
                            // __EH_prolog3_catch_GS
                            // _EH_prolog3_catch_GS
                            else
                            {
                                hr = ProcessEhProlog3();
                            }
                        }
                    }
                }
                else if (PROLOGPREFIX_SEH == type)
                {
                    unsigned int lenSuffix = lenFuncName - lenPrefix;

                    // __SEH_prolog
                    // _SEH_prolog
                    // __SEH_prolog4
                    // _SEH_prolog4
                    // __SEH_prolog4_GS
                    // _SEH_prolog4_GS
                    if ((0U == lenSuffix) ||
                        (1U == lenSuffix && L'4' == pFuncName[lenPrefix]) ||
                        (4U == lenSuffix && memcmp(pFuncName + lenPrefix, L"4_GS", 4 * sizeof(wchar_t))))
                    {
                        hr = ProcessSehProlog();
                    }
                }

                SysFreeString(pFuncName);
            }
        }

        if (S_FALSE == hr)
        {
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessEhProlog()
{
    HRESULT hr = AdjustRegister(REG_INDEX_ESP, 3 * sizeof(DWORD));

    if (SUCCEEDED(hr))
    {
        hr = PopRegister(REG_INDEX_EBP);
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessEhProlog2()
{
    HRESULT hr = E_FAIL;
    ValueX86 val;

    if (m_context.GetRegister(REG_INDEX_EBX, val))
    {
        m_context.SetRegister(REG_INDEX_ESP, val);
        ValueX86 stackVal = 0;

        if (m_context.ReadFullMemory(MEM_TYPE_STACK, val, stackVal))
        {
            m_context.SetRegister(REG_INDEX_EBX, stackVal);

            if (!m_context.GetRegister(REG_INDEX_EBP, val))
            {
                val = stackVal;
            }

            stackVal = 0;

            if (m_context.ReadFullMemory(MEM_TYPE_STACK, val, stackVal))
            {
                m_context.SetRegister(REG_INDEX_EBP, stackVal);
                hr = S_OK;
            }
        }
    }

    return hr;
}


HRESULT PrologFrameTraverser::ProcessEhProlog3()
{
    HRESULT hr = E_FAIL;
    ValueX86 val;

    if (m_context.GetRegister(REG_INDEX_EBP, val))
    {
        m_context.SetRegister(REG_INDEX_ESP, val);
        ValueX86 stackVal = 0;

        if (m_context.ReadFullMemory(MEM_TYPE_STACK, val, stackVal))
        {
            m_context.SetRegister(REG_INDEX_EBP, stackVal);
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessEhProlog3Align()
{
    HRESULT hr = E_FAIL;
    ValueX86 val;

    if (m_context.GetRegister(REG_INDEX_EBX, val))
    {
        m_context.SetRegister(REG_INDEX_ESP, val - X86_STACK_SIZE);
        ValueX86 stackVal = 0;

        if (m_context.ReadFullMemory(MEM_TYPE_STACK, val, stackVal))
        {
            m_context.SetRegister(REG_INDEX_EBX, stackVal);

            if (m_context.GetRegister(REG_INDEX_EBP, val))
            {
                if (S_OK != hr)
                {
                    val = stackVal;
                }

                stackVal = 0;

                if (m_context.ReadFullMemory(MEM_TYPE_STACK, val, stackVal))
                {
                    m_context.SetRegister(REG_INDEX_EBP, stackVal);
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}

HRESULT PrologFrameTraverser::ProcessSehProlog()
{
    HRESULT hr = E_FAIL;
    ValueX86 val;

    if (m_context.GetRegister(REG_INDEX_EBP, val))
    {
        m_context.SetRegister(REG_INDEX_ESP, val - X86_STACK_SIZE);
        ValueX86 stackVal = 0;

        if (m_context.ReadFullMemory(MEM_TYPE_STACK, val, stackVal))
        {
            m_context.SetRegister(REG_INDEX_EBP, stackVal);
            hr = S_OK;
        }
    }

    return hr;
}

static PrologPrefixType DeterminePrefix(const wchar_t* pFuncName, unsigned int lenFuncName, unsigned int& lenPrefix)
{
    PrologPrefixType type = PROLOGPREFIX_NONE;
    lenPrefix = 0U;

    if (10U <= lenFuncName && L'_' == *pFuncName)
    {
        unsigned int lenFuncNameOrig = lenFuncName;
        pFuncName++;
        lenFuncName--;

        if (L'_' == *pFuncName)
        {
            pFuncName++;
            lenFuncName--;
        }

        PrologPrefixType assumedType;

        if (L'S' == *pFuncName)
        {
            assumedType = PROLOGPREFIX_SEH;
            pFuncName++;
            lenFuncName--;
        }
        else
        {
            assumedType = PROLOGPREFIX_EH;
        }

        const unsigned int lenProlog = wcslen(L"EH_prolog");

        if (lenProlog <= lenFuncName && memcmp(pFuncName, L"EH_prolog", lenProlog * sizeof(wchar_t)))
        {
            lenPrefix = (lenFuncNameOrig - lenFuncName) + lenProlog;
            type = assumedType;
        }
    }

    return type;
}

static bool TrimAlignSuffix(const wchar_t* pFuncName, unsigned int& lenFuncName, unsigned int lenPrefix)
{
    bool ret = false;

    pFuncName += lenPrefix;
    lenFuncName -= lenPrefix;

    const unsigned int lenAlign = wcslen(L"_align");

    if (lenAlign <= lenFuncName && memcmp(pFuncName + lenFuncName - lenAlign, L"_align", lenAlign * sizeof(wchar_t)))
    {
        lenFuncName -= lenAlign;
        ret = true;
    }

    return ret;
}
