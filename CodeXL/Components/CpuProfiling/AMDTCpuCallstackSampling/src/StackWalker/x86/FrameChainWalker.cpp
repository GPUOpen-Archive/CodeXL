//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file FrameChainWalker.cpp
///
//==================================================================================

#include <AMDTOSWrappers/include/osDebugLog.h>
#include "FrameChainWalker.h"
#include "DisassemblerX86.h"
#include "DiaFrameData.h"

#define UNINITIALIZED_SECTION_INDEX ((unsigned)-2)

static unsigned int DeterminePrologType(const wchar_t* pFuncName, unsigned int lenFuncName);

static inline bool IsIndirectCallInst(const gtUByte* pCode)
{
    //
    // +--------+-------------+-------------------------------------------------------+
    // | Opcode | Instruction | Description                                           |
    // +--------+-------------+-------------------------------------------------------+
    // | FF /2  | CALL r/m32  | Call near, absolute indirect, address given in r/m32. |
    // | FF /3  | CALL m16:16 | Call far, absolute indirect, address given in m16:16. |
    // +--------+-------------+-------------------------------------------------------+
    //

    // (OPCODE_EXT == pCode[0] && (2 == (pCode[1] & MODRM_REG_MASK) || 3 == (pCode[1] & MODRM_REG_MASK)))
    return (MAKE_WORD(OPCODE_EXT, (2 << MODRM_OPCODE_EXT_OFFSET)) == (MAKE_WORD(0xFF, (6 << MODRM_OPCODE_EXT_OFFSET)) & *reinterpret_cast<const gtUInt16 UNALIGNED*>(pCode)));
}


FrameChainWalker::FrameChainWalker() : m_codeBufferSize(DEFAULT_CODE_BUFFER_SIZE),
    m_searchDepth(0U),
    m_sectionIndex(0U),
    m_addrOffset(0U),
    m_vaImageStart(0U),
    m_frameType(FRAME_TYPE_INVALID),
    m_lengthParams(0U),
    m_frameTravUnknown(*this),
    m_frameTravFpo(*this),
    m_frameTravStandard(*this),
    m_frameTravStandardNoEbp(*this),
    m_frameTravFrameData(*this),
    m_frameTravProlog(m_context),
    m_frameTravKiUserExceptionDispatcher(*this),
    m_frameTravTrap(m_context),
    m_frameTravTss(m_context),
    m_restoredContextRecord(false)
{
    m_codeBuffer.resize(m_codeBufferSize);
}

void FrameChainWalker::Reset(ProcessWorkingSetQuery& workingSet, VirtualStack& stack, VAddrX86 controlPc, VAddrX86 stackPtr, VAddrX86 framePtr)
{
    m_context.Clear(controlPc, stackPtr, framePtr);
    m_context.SetWorkingSet(&workingSet);
    m_context.SetVirtualStack(&stack);

    m_searchDepth = 0U;
    m_sectionIndex = 0U;
    m_addrOffset = 0U;
    m_vaImageStart = 0U;
    m_frameType = FRAME_TYPE_INVALID;
    m_lengthParams = 0U;
    m_restoredContextRecord = false;
}

HRESULT FrameChainWalker::TraverseNext()
{
    HRESULT hr;
    gtUInt32 controlPc;

    if (m_context.GetRegister(REG_INDEX_EIP, controlPc))
    {
        IDiaFrameData* pFrameData = GetFrameDataByVA(controlPc);
        hr = TraverseFrame(pFrameData);
        pFrameData->Release();
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT FrameChainWalker::GetFrameData(StackFrameData& frameData)
{
    HRESULT hr;

    if (FRAME_TYPE_INVALID != m_frameType)
    {
        switch (m_frameType)
        {
            case FRAME_TYPE_UNKNOWN:
                hr = m_frameTravUnknown.GetFrameData(frameData);
                break;

            case FRAME_TYPE_FPO:
                hr = m_frameTravFpo.GetFrameData(frameData);
                break;

            case FRAME_TYPE_STANDARD:
                hr = m_frameTravStandard.GetFrameData(frameData);
                break;

            case FRAME_TYPE_STANDARD_NO_EBP:
                hr = m_frameTravStandardNoEbp.GetFrameData(frameData);
                break;

            case FRAME_TYPE_FRAME_DATA:
                hr = m_frameTravFrameData.GetFrameData(frameData);
                break;

            case FRAME_TYPE_PROLOG:
                hr = m_frameTravProlog.GetFrameData(frameData);
                break;

            case FRAME_TYPE_KIUSEREXCEPTIONDISPATCHER:
                hr = m_frameTravKiUserExceptionDispatcher.GetFrameData(frameData);
                break;

            case FRAME_TYPE_TRAP:
                hr = m_frameTravTrap.GetFrameData(frameData);
                break;

            case FRAME_TYPE_TSS:
                hr = m_frameTravTss.GetFrameData(frameData);
                break;

            default:
                hr = E_FAIL;
                break;
        }

        m_lengthParams = (FALSE != frameData.m_valid.lengthParams) ? frameData.m_lengthParams : 0U;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

IDiaFrameData* FrameChainWalker::GetFrameDataByVA(VAddrX86 virtualAddress)
{
    IDiaFrameData* pFrameData = NULL;
    m_context.FindFrameInterface(virtualAddress, &pFrameData);

    if (NULL == pFrameData)
    {
        CDiaFrameData* pFakeFrameData = new CDiaFrameData();
        pFakeFrameData->AddRef();

        if (!GenerateFakeFrame(virtualAddress, *pFakeFrameData))
        {
            pFakeFrameData->m_type = FrameTypeUnknown;
            pFakeFrameData->m_valid.type = TRUE;

            pFakeFrameData->m_functionStart = TRUE;
            pFakeFrameData->m_valid.functionStart = TRUE;
        }

        pFrameData = static_cast<IDiaFrameData*>(pFakeFrameData);
    }

    return pFrameData;
}

bool FrameChainWalker::GenerateFakeFrame(VAddrX86 virtualAddress, CDiaFrameData& frameData)
{
    bool ret = false;

    IDiaSymbol* pSymbol = NULL;
    HRESULT hr = m_context.FindSymbolInterface(virtualAddress, &pSymbol);

    if (NULL != pSymbol)
    {
        ULONGLONG symbolVa = 0ULL;

        if (S_OK == hr && S_OK == pSymbol->get_virtualAddress(&symbolVa))
        {
            frameData.m_type = FrameTypeStandard;
            frameData.m_valid.type = TRUE;

            frameData.m_lengthProlog = CODE_SAVE_FRAME_SIZE;
            frameData.m_valid.lengthProlog = TRUE;

            frameData.m_virtualAddress = symbolVa;
            frameData.m_valid.virtualAddress = TRUE;

            frameData.m_functionStart = TRUE;
            frameData.m_valid.functionStart = TRUE;

            ULONGLONG symbolLength;

            if (S_OK == pSymbol->get_length(&symbolLength))
            {
                frameData.m_lengthBlock = static_cast<DWORD>(symbolLength);
                frameData.m_valid.lengthBlock = TRUE;
            }

            if (CalcCbLocalsForFunc(static_cast<VAddrX86>(symbolVa), reinterpret_cast<RVAddrX86&>(frameData.m_lengthLocals)))
            {
                frameData.m_valid.lengthLocals = TRUE;
            }

            ret = true;
        }
        else
        {
            bool isSystemCallStub = (0x7FFE0300 <= virtualAddress && virtualAddress <= 0x7FFE031F);

            const wchar_t* pFrameCommand = nullptr;
            unsigned int lenFrameCommand = 0;

            if (isSystemCallStub)
            {
                pFrameCommand = L"$T2 $esp = $T0 .raSearchStart = $eip $T0 ^ = $esp $T0 4 + = $ebp $ebp = $ebx $ebx =";
                lenFrameCommand = wcslen(pFrameCommand);
            }
            else if (0 == virtualAddress)
            {
                pFrameCommand = L"$T0 $esp = $eip $T0 ^ = $esp $T0 4 + = $ebp $ebp = $ebx $ebx = $eax $eax = $ecx $ecx = $edx $edx = $esi $esi = $edi $edi =";
                lenFrameCommand = wcslen(pFrameCommand);
            }
            else
            {
                pFrameCommand = NULL;
            }

            if (NULL != pFrameCommand)
            {
                frameData.m_type = FrameTypeFrameData;
                frameData.m_valid.type = TRUE;

                frameData.m_lengthProlog = 0;
                frameData.m_valid.lengthProlog = TRUE;

                frameData.m_virtualAddress = virtualAddress;
                frameData.m_valid.virtualAddress = TRUE;

                frameData.m_allocatesBasePointer = isSystemCallStub;

                frameData.m_valid.allocatesBasePointer = TRUE;

                frameData.m_lengthLocals = 0;
                frameData.m_valid.lengthLocals = TRUE;

                frameData.m_valid.lengthSavedRegisters = TRUE;

                frameData.m_lengthParams = 0;
                frameData.m_valid.lengthParams = TRUE;

                frameData.m_pFrameCommand = SysAllocStringLen(pFrameCommand, lenFrameCommand);
                frameData.m_valid.program = TRUE;

                frameData.m_functionStart = TRUE;
                frameData.m_valid.functionStart = TRUE;

                ret = true;
            }
        }

        pSymbol->Release();
    }

    return ret;
}

HRESULT FrameChainWalker::TraverseFrame(IDiaFrameData* pFrameData)
{
    gtUInt32 stackPtr;

    if (!m_context.GetRegister(REG_INDEX_ESP, stackPtr))
    {
        return E_FAIL;
    }

    gtUInt32 controlPc;

    if (!m_context.GetRegister(REG_INDEX_EIP, controlPc))
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    gtUByte aMemory[INST_CALL_MAX_SIZE + 1];
    bool hasPrevCode = (0 < m_context.ReadMemory(MEM_TYPE_CODE, controlPc - INST_CALL_MAX_SIZE, aMemory, INST_CALL_MAX_SIZE));
    bool isUserExceptionDispatcher = false;

    IDiaSymbol* pIpSymbol = NULL;

    if (S_OK == m_context.FindSymbolInterface(controlPc, &pIpSymbol))
    {
        BSTR pSymbolName = NULL;

        if (S_OK == pIpSymbol->get_name(&pSymbolName))
        {
            const unsigned int len = wcslen(L"_KiUserExceptionDispatcher@8");
            isUserExceptionDispatcher = (len == SysStringLen(pSymbolName) &&
                                         0 == memcmp(pSymbolName, L"_KiUserExceptionDispatcher@8", len * sizeof(wchar_t)));
            SysFreeString(pSymbolName);
        }

        pIpSymbol->Release();
    }

    if (!isUserExceptionDispatcher)
    {
        if (FRAME_TYPE_INVALID != m_frameType)
        {
            if (!IsContextRecordRestored())
            {
                stackPtr += m_lengthParams;
                m_context.SetRegister(REG_INDEX_ESP, stackPtr);
            }
            else
            {
                SetContextRecordRestored(false);
            }

            m_frameType = FRAME_TYPE_INVALID;
        }

        DWORD type;
        hr = pFrameData->get_type(&type);

        if (S_OK != hr)
        {
            return hr;
        }

        IDiaFrameData* pFrameProlog = NULL;
        hr = IsIpInFunctionProlog(type, pFrameData, &pFrameProlog);

        if (S_OK == hr)
        {
            m_frameType = FRAME_TYPE_PROLOG;
            m_frameTravProlog.Reset(pFrameProlog, stackPtr);
            hr = m_frameTravProlog.TraverseNext();

            if (S_OK == hr)
            {
                m_context.RecoverRegister(REG_INDEX_EBP);
                m_context.RecoverRegister(REG_INDEX_EBX);

                hr = StandardFrameTraverser::IsStandardFrame(pFrameProlog);

                if (S_OK == hr)
                {
                    gtUInt32 vframe;

                    if (m_context.GetVFrame(vframe))
                    {
                        m_context.SetVFrame(vframe - 4);
                    }
                }
                else if (SUCCEEDED(hr))
                {
                    hr = S_OK;
                }
            }

            pFrameProlog->Release();
        }
        else if (SUCCEEDED(hr)) // S_FALSE == hr
        {
            hr = StandardFrameTraverser::IsStandardFrame(pFrameProlog);
            pFrameProlog->Release();

            if (SUCCEEDED(hr))
            {
                bool isStandardFrame = (S_OK == hr);

                if (isStandardFrame || hasPrevCode || FrameTypeStandard >= type)
                {
                    gtUInt32 framePtr;

                    if (m_context.GetRegister(REG_INDEX_EBP, framePtr) && (MM_LOWEST_USER_ADDRESS > framePtr || stackPtr > framePtr))
                    {
                        m_context.RemoveBaseRegister(REG_INDEX_EBP);

                        if (hasPrevCode)
                        {
                            type = FrameTypeStandard;
                        }
                    }
                }

                switch (type)
                {
                    case FrameTypeFPO:
                        m_frameType = FRAME_TYPE_FPO;
                        m_frameTravFpo.Reset(pFrameData, stackPtr);
                        hr = m_frameTravFpo.TraverseNext();
                        break;

                    case FrameTypeTrap:
                        m_frameType = FRAME_TYPE_TRAP;
                        m_frameTravTrap.Reset(pFrameData, stackPtr);
                        hr = m_frameTravTrap.TraverseNext();
                        break;

                    case FrameTypeTSS:
                        m_frameType = FRAME_TYPE_TSS;
                        m_frameTravTss.Reset(pFrameData, stackPtr);
                        hr = m_frameTravTss.TraverseNext();
                        break;

                    case FrameTypeStandard:
                        if (m_context.FindBaseRegister(REG_INDEX_EBP))
                        {
                            m_frameType = FRAME_TYPE_STANDARD;
                            m_frameTravStandard.Reset(pFrameData, stackPtr);
                            hr = m_frameTravStandard.TraverseNext();
                        }
                        else
                        {
                            m_frameType = FRAME_TYPE_STANDARD_NO_EBP;
                            m_frameTravStandardNoEbp.Reset(pFrameData, stackPtr);
                            hr = m_frameTravStandardNoEbp.TraverseNext();
                        }

                        break;

                    case FrameTypeFrameData:
                        m_frameType = FRAME_TYPE_FRAME_DATA;
                        m_frameTravFrameData.Reset(pFrameData, stackPtr);
                        hr = m_frameTravFrameData.TraverseNext();
                        break;

                    case FrameTypeUnknown:
                        m_frameType = FRAME_TYPE_UNKNOWN;
                        m_frameTravUnknown.Reset(pFrameData, stackPtr, controlPc);
                        hr = m_frameTravUnknown.TraverseNext();
                        break;

                    default:
                        hr = E_FAIL;
                        break;
                }

                if (FRAME_TYPE_INVALID != m_frameType && S_OK != hr)
                {
                    if (m_context.GetRegisterFailed())
                    {
                        m_context.Reset();
                        m_context.SetRegister(REG_INDEX_ESP, stackPtr);

                        if (isStandardFrame)
                        {
                            m_frameType = FRAME_TYPE_STANDARD_NO_EBP;
                            m_frameTravStandardNoEbp.Reset(pFrameData, stackPtr);
                            hr = m_frameTravStandardNoEbp.TraverseNext();
                        }
                        else
                        {
                            m_frameType = FRAME_TYPE_UNKNOWN;
                            m_frameTravUnknown.Reset(pFrameData, stackPtr, controlPc);
                            hr = m_frameTravUnknown.TraverseNext();
                        }
                    }
                }

                if (S_OK == hr)
                {
                    SetReturnAddress();
                    m_context.RecoverRegister(REG_INDEX_ESI);
                    m_context.RecoverRegister(REG_INDEX_EDI);
                    m_context.RecoverRegister(REG_INDEX_EAX);
                    m_context.RecoverRegister(REG_INDEX_ECX);
                    m_context.RecoverRegister(REG_INDEX_EDX);
                }
            }
        }
    }
    else
    {
        SetContextRecordRestored(false);
        m_frameType = FRAME_TYPE_KIUSEREXCEPTIONDISPATCHER;
        m_frameTravKiUserExceptionDispatcher.Reset(pFrameData, stackPtr);
        hr = m_frameTravKiUserExceptionDispatcher.TraverseNext();
    }

    return hr;
}

bool FrameChainWalker::CalcCbLocalsForFunc(VAddrX86 codeAddr, RVAddrX86& lengthLocals)
{
    lengthLocals = 0;

    // 5 for the length of the MOV instruction +
    // 5 for the length of the CALL instruction +
    // 6 for the maximum length of the SUB instruction.
    gtUByte aCode[5 + 5 + 6];
    memset(aCode, OPCODE_NOP, sizeof(aCode));
    int cbData = m_context.ReadMemory(MEM_TYPE_CODE, codeAddr, aCode);

    if (0 >= cbData)
    {
        return false;
    }

    const gtUByte* pCode = aCode;

    if ((CODE_SAVE_FRAME_SIZE + 1) < cbData && CODE_SAVE_FRAME == (CODE_SAVE_FRAME_MASK & *reinterpret_cast<const gtUInt32 UNALIGNED*>(pCode)))
    {
        return 0 != ExtractSubEsp(pCode + CODE_SAVE_FRAME_SIZE + 1, reinterpret_cast<gtInt32&>(lengthLocals));
    }


    if (10 >= cbData)
    {
        return false;
    }

    if ((OPCODE_MOV_REG_IMM + REGISTER_X86_EAX) != pCode[0])
    {
        return false;
    }

    pCode += 5;


    if (OPCODE_CALL_REL != pCode[0])
    {
        return false;
    }

    gtInt32 funcOffset = *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 1);
    pCode += 5;

    // 5 for the length of the MOV instruction +
    // 5 for the length of the CALL instruction.
    VAddrX86 funcVa = codeAddr + 5 + 5 + funcOffset;

    bool ret = false;

    IDiaSymbol* pFuncSymbol = NULL;

    if (S_OK == m_context.FindSymbolInterface(funcVa, &pFuncSymbol))
    {
        BSTR pFuncName = NULL;

        if (S_OK == pFuncSymbol->get_name(&pFuncName))
        {
            const unsigned int lenFuncName = SysStringLen(pFuncName);

            if (0U != DeterminePrologType(pFuncName, lenFuncName))
            {
                ret = (0 != ExtractSubEsp(pCode, reinterpret_cast<gtInt32&>(lengthLocals)));
            }

            SysFreeString(pFuncName);
        }

        pFuncSymbol->Release();
    }

    return ret;
}

HRESULT FrameChainWalker::IsIpInFunctionProlog(unsigned type, IDiaFrameData* pFrameData, IDiaFrameData** ppFrameProlog)
{
    *ppFrameProlog = NULL;

    HRESULT hr;

    BOOL functionStart = FALSE;

    if (FrameTypeFrameData == type && S_OK == pFrameData->get_functionStart(&functionStart) && FALSE == functionStart)
    {
        hr = pFrameData->get_functionParent(ppFrameProlog);

        if (NULL == *ppFrameProlog)
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = pFrameData->QueryInterface(ppFrameProlog);
    }

    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;
        VAddrX86 controlPc;

        if (m_context.GetRegister(REG_INDEX_EIP, controlPc))
        {
            DWORD lengthProlog = 0;
            ULARGE_INTEGER virtualAddress;
            virtualAddress.QuadPart = 0ULL;

            if (S_OK == (*ppFrameProlog)->get_lengthProlog(&lengthProlog) && S_OK == (*ppFrameProlog)->get_virtualAddress(&virtualAddress.QuadPart))
            {
                if (virtualAddress.LowPart == controlPc)
                {
                    hr = S_OK;
                }
                else if (virtualAddress.LowPart < controlPc)
                {
                    virtualAddress.LowPart += lengthProlog;

                    if (controlPc < virtualAddress.LowPart)
                    {
                        hr = S_OK;
                    }
                }
            }
        }
    }

    return hr;
}

bool FrameChainWalker::SetReturnAddress()
{
    VAddrX86 controlPc = m_context.GetRegister(REG_INDEX_EIP);
    bool ret = (0 != controlPc);

    if (ret)
    {
        IDiaSymbol* pSymbol = NULL;

        if (S_OK == m_context.FindSymbolInterface(controlPc, &pSymbol))
        {
            controlPc--;
            IDiaSymbol* pSymbolPrev = NULL;

            if (S_OK == m_context.FindSymbolInterface(controlPc, &pSymbolPrev))
            {
                ULONGLONG vaSymbol = 0ULL, vaSymbolPrev = 0ULL;

                if (S_OK == pSymbol->get_virtualAddress(&vaSymbol) && S_OK == pSymbolPrev->get_virtualAddress(&vaSymbolPrev))
                {
                    if (vaSymbol != vaSymbolPrev)
                    {
                        m_context.SetRegister(REG_INDEX_EIP, controlPc);
                    }
                }
                else
                {
                    BSTR pNameSymbol = NULL;

                    if (S_OK == pSymbol->get_name(&pNameSymbol))
                    {
                        BSTR pNameSymbolPrev = NULL;

                        if (S_OK == pSymbolPrev->get_name(&pNameSymbolPrev))
                        {
                            const unsigned int lenName = SysStringLen(pNameSymbol);

                            if (lenName != SysStringLen(pNameSymbolPrev) || 0 != memcmp(pNameSymbol, pNameSymbolPrev, lenName * sizeof(wchar_t)))
                            {
                                m_context.SetRegister(REG_INDEX_EIP, controlPc);
                            }

                            SysFreeString(pNameSymbolPrev);
                        }

                        SysFreeString(pNameSymbol);
                    }
                }

                pSymbolPrev->Release();
            }

            pSymbol->Release();
        }
    }

    return ret;
}

HRESULT FrameChainWalker::SearchForReturnAddress(IDiaFrameData* pFrame, VAddrX86& retAddrPtr)
{
    HRESULT hr = E_FAIL;
    VAddrX86 stackPtr;

    if (m_context.GetRegister(REG_INDEX_ESP, stackPtr))
    {
        //
        // Get required base frame data.
        //

        VAddrX86 framePtr = 0;
        bool hasFramePtr = m_context.FindBaseRegister(REG_INDEX_EBP);

        if (!hasFramePtr || m_context.GetRegister(REG_INDEX_EBP, framePtr))
        {
            DWORD addLen = 0;

            if (S_OK == pFrame->get_lengthLocals(&addLen))
            {
                stackPtr += addLen;
            }

            addLen = 0;

            if (S_OK == pFrame->get_lengthSavedRegisters(&addLen))
            {
                stackPtr += addLen;
            }

            hr = SearchForReturnAddressInternal(pFrame, stackPtr, framePtr, hasFramePtr, retAddrPtr);
        }
    }

    return hr;
}

HRESULT FrameChainWalker::SearchForReturnAddressStart(IDiaFrameData* pFrame, VAddrX86 startAddress, VAddrX86& retAddrPtr)
{
    return SearchForReturnAddressInternal(pFrame, startAddress, 0, false, retAddrPtr);
}

HRESULT FrameChainWalker::SearchForReturnAddressInternal(IDiaFrameData* pFrame, VAddrX86 stackPtr, VAddrX86 framePtr, bool hasFramePtr, VAddrX86& retAddrPtr)
{
    HRESULT hr;
    VAddrX86 controlPc;

    if (m_context.GetRegister(REG_INDEX_EIP, controlPc))
    {
        BOOL funcStart = FALSE;
        IDiaFrameData* pParent = NULL;

        if (S_OK != pFrame->get_functionStart(&funcStart) || FALSE != funcStart || S_OK != pFrame->get_functionParent(&pParent))
        {
            pFrame->AddRef();
            pParent = pFrame;
        }

        bool estimated = false;
        ULARGE_INTEGER virtualAddress;
        ULARGE_INTEGER length;

        if (pParent->get_functionStart(&funcStart) || FALSE != funcStart)
        {
            DWORD lenBlock;

            if (S_OK != pParent->get_lengthBlock(&lenBlock) || S_OK != pParent->get_virtualAddress(&virtualAddress.QuadPart))
            {
                estimated = true;
                length.LowPart = INST_CALL_MAX_SIZE;
                virtualAddress.LowPart = controlPc - INST_CALL_MAX_SIZE;
            }
            else
            {
                length.LowPart = lenBlock;
            }
        }
        else
        {
            IDiaSymbol* pSymbol = NULL;

            if (S_OK != m_context.FindSymbolInterface(controlPc, &pSymbol) ||
                S_OK != pSymbol->get_length(&length.QuadPart) ||
                S_OK != pSymbol->get_virtualAddress(&virtualAddress.QuadPart))
            {
                estimated = true;
                length.LowPart = INST_CALL_MAX_SIZE;
                virtualAddress.LowPart = controlPc - INST_CALL_MAX_SIZE;
            }

            if (NULL != pSymbol)
            {
                pSymbol->Release();
            }
        }


        hr = SearchForReturnAddressInternal(stackPtr, virtualAddress.LowPart, length.LowPart, retAddrPtr, estimated);

        if (S_OK != hr && hasFramePtr && !estimated)
        {
            hr = SearchForReturnAddressInternal(framePtr, virtualAddress.LowPart, length.LowPart, retAddrPtr, false);
        }

        pParent->Release();
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT FrameChainWalker::SearchForReturnAddressInternal(VAddrX86 stackPtr, VAddrX86 calleeAddr, RVAddrX86 calleeBlockLen, VAddrX86& retAddrPtr, bool estimated)
{
    retAddrPtr = 0;
    HRESULT hr = E_FAIL;

    gtUByte aStack[2048];
    int stackSize = m_context.ReadMemory(MEM_TYPE_STACK, stackPtr, aStack);

    if (0 < stackSize)
    {
        if (estimated || 0 != (m_vaImageStart = m_context.GetImageLoadAddress(calleeAddr)))
        {
            hr = S_FALSE;
            m_sectionIndex = UNINITIALIZED_SECTION_INDEX;

            if (X86_STACK_SIZE <= stackSize)
            {
                VAddrX86 framePtr = 0;

                if (m_context.FindBaseRegister(REG_INDEX_EBP))
                {
                    framePtr = m_context.GetRegister(REG_INDEX_EBP);
                }

                bool assumedIndirectAddrPtrSet = false;
                bool frameAddrPtrSet = false;
                bool potentialRetAddrSet = false;

                VAddrX86 frameAddrPtr = 0;
                VAddrX86 assumedIndirectAddrPtr = 0;
                VAddrX86 potentialRetAddrPtr = 0;
                VAddrX86 potentialRetAddr = 0;

                VAddrX86 stackLimit = stackPtr + 40;
                ValueX86* pStack = reinterpret_cast<ValueX86*>(aStack);

                for (RVAddrX86 i = 0, len = stackSize / X86_STACK_SIZE; i < len; ++i, ++pStack, stackPtr += X86_STACK_SIZE)
                {
                    ValueX86 stackValue = *pStack;

                    if (MM_LOWEST_VALID_ADDRESS > stackValue)
                    {
                        continue;
                    }

                    // We need a bit more space than actually needed to make sure we do not touch unallocated memory in later disassembly algorithms.
                    gtUByte aCode[(INST_CALL_MAX_SIZE + 1) + CODE_BUFFER_OVERRUN_SIZE];
                    memset(aCode, OPCODE_NOP, sizeof(aCode));

                    int codeLength = INST_CALL_MAX_SIZE + 1;

                    if (0 < (codeLength = m_context.ReadMemory(MEM_TYPE_CODE, stackValue - codeLength, aCode, codeLength)))
                    {
                        if (!estimated && stackPtr > stackLimit)
                        {
                            if (frameAddrPtrSet)
                            {
                                retAddrPtr = frameAddrPtr;
                                hr = S_OK;
                                break;
                            }

                            if (assumedIndirectAddrPtrSet)
                            {
                                retAddrPtr = assumedIndirectAddrPtr;
                                hr = S_OK;
                                break;
                            }
                        }

                        const gtUByte* pCode = aCode;

                        if (INST_CALL_REL_SIZE <= codeLength)
                        {
                            if ((INST_CALL_MAX_SIZE + 1) == codeLength)
                            {
                                if (INST_UNDEFINED == *reinterpret_cast<const gtUInt16*>(pCode))
                                {
                                    retAddrPtr = stackPtr;
                                    hr = S_OK;
                                    break;
                                }

                                codeLength = INST_CALL_MAX_SIZE;
                                pCode++;
                            }

                            // Try to back trace from a relative call instruction.
                            if (OPCODE_CALL_REL == pCode[INST_CALL_MAX_SIZE - INST_CALL_REL_SIZE])
                            {
                                VAddrX86 potentialRelRetAddr = stackValue;
                                VAddrX86 calledFuncAddr = potentialRelRetAddr + *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + (INST_CALL_MAX_SIZE - INST_CALL_REL_SIZE + 1));

                                if (estimated)
                                {
                                    if (calledFuncAddr == (calleeAddr + calleeBlockLen))
                                    {
                                        retAddrPtr = stackPtr;
                                        hr = S_OK;
                                        break;
                                    }
                                }
                                else if (CheckCallTarget(potentialRelRetAddr - INST_CALL_REL_SIZE, calledFuncAddr, calleeAddr, calleeBlockLen))
                                {
                                    retAddrPtr = stackPtr;
                                    hr = S_OK;
                                    break;
                                }

                                VAddrX86 jumpTargetAddr = static_cast<VAddrX86>(~0);

                                // Skip jump chains.
                                if (S_OK == ChaseAndValidateJumpChain(calleeAddr, calleeBlockLen, calledFuncAddr, estimated, jumpTargetAddr))
                                {
                                    retAddrPtr = stackPtr;
                                    hr = S_OK;
                                    break;
                                }

                                if (estimated && (~0) != jumpTargetAddr)
                                {
                                    if (potentialRetAddrSet)
                                    {
                                        if (jumpTargetAddr < potentialRetAddr)
                                        {
                                            potentialRetAddr = jumpTargetAddr;
                                            potentialRetAddrPtr = stackPtr;
                                        }
                                    }
                                    else
                                    {
                                        potentialRetAddrSet = true;
                                        potentialRetAddr = jumpTargetAddr;
                                        potentialRetAddrPtr = stackPtr;
                                    }
                                }
                            }
                        }

                        // Try to back trace from an indirect call instruction.
                        if (INST_CALL_MIN_SIZE <= codeLength)
                        {
                            VAddrX86 aCalledFuncAddr[5];
                            unsigned countTargets = ExtractIndirectCallTargets(pCode + codeLength, codeLength, stackValue, stackPtr, aCalledFuncAddr);

                            for (unsigned j = 0; j < countTargets; ++j)
                            {
                                VAddrX86 calledFuncAddr = aCalledFuncAddr[j];

                                if (0 != calledFuncAddr)
                                {
                                    if ((estimated ? calleeAddr + calleeBlockLen : calleeAddr) == calledFuncAddr)
                                    {
                                        retAddrPtr = stackPtr;
                                        hr = S_OK;
                                        break;
                                    }

                                    VAddrX86 jumpTargetAddr = static_cast<VAddrX86>(~0);

                                    // Skip jump chains.
                                    if (S_OK == ChaseAndValidateJumpChain(calleeAddr, calleeBlockLen, calledFuncAddr, estimated, jumpTargetAddr))
                                    {
                                        retAddrPtr = stackPtr;
                                        hr = S_OK;
                                        break;
                                    }

                                    if (estimated && (~0) != jumpTargetAddr)
                                    {
                                        if (0 == jumpTargetAddr)
                                        {
                                            retAddrPtr = stackPtr;
                                            hr = S_OK;
                                            break;
                                        }

                                        if (potentialRetAddrSet)
                                        {
                                            if (jumpTargetAddr < potentialRetAddr)
                                            {
                                                potentialRetAddr = jumpTargetAddr;
                                                potentialRetAddrPtr = stackPtr;
                                            }
                                        }
                                        else
                                        {
                                            potentialRetAddrSet = true;
                                            potentialRetAddr = jumpTargetAddr;
                                            potentialRetAddrPtr = stackPtr;
                                        }
                                    }
                                }
                                else
                                {
                                    if (stackPtr == (framePtr + X86_STACK_SIZE))
                                    {
                                        frameAddrPtrSet = true;
                                        frameAddrPtr = stackPtr;
                                    }
                                    else if (!assumedIndirectAddrPtrSet)
                                    {
                                        assumedIndirectAddrPtrSet = true;
                                        assumedIndirectAddrPtr = stackPtr;
                                    }
                                }
                            }

                            if (S_OK == hr)
                            {
                                break;
                            }
                        }
                    }
                }

                if (S_FALSE == hr)
                {
                    if (frameAddrPtrSet)
                    {
                        retAddrPtr = frameAddrPtr;
                        hr = S_OK;
                    }
                    else if (assumedIndirectAddrPtrSet)
                    {
                        if (potentialRetAddrSet && assumedIndirectAddrPtr > potentialRetAddrPtr)
                        {
                            retAddrPtr = potentialRetAddrPtr;
                        }
                        else
                        {
                            retAddrPtr = assumedIndirectAddrPtr;
                        }

                        hr = S_OK;
                    }
                    else if (potentialRetAddrSet)
                    {
                        retAddrPtr = potentialRetAddrPtr;
                        hr = S_OK;
                    }
                }
            }
        }
    }

    return hr;
}

bool FrameChainWalker::CheckCallTarget(VAddrX86 addr1, VAddrX86 addr2, VAddrX86 addr3, RVAddrX86 length)
{
    bool ret = (addr2 == addr3);

    if (!ret)
    {
        if (UNINITIALIZED_SECTION_INDEX == m_sectionIndex)
        {
            m_sectionIndex = m_context.GetSectionInfo(addr3, m_addrOffset);
        }

        if (INVALID_SECTION_INDEX != m_sectionIndex)
        {
            gtRVAddr addrOffset;

            ret = ((m_context.GetImageLoadAddress(addr1) == m_vaImageStart) &&

                   (m_context.GetImageLoadAddress(addr2) == m_vaImageStart) &&

                   (m_context.GetSectionInfo(addr1, addrOffset) == m_sectionIndex &&
                    (m_addrOffset <= addrOffset && addrOffset < (m_addrOffset + length))) &&

                   (m_context.GetSectionInfo(addr2, addrOffset) == m_sectionIndex &&
                    (m_addrOffset <= addrOffset && addrOffset < (m_addrOffset + length))));
        }
    }

    return ret;
}

unsigned FrameChainWalker::ExtractIndirectCallTargets(const gtUByte* pCode, unsigned len, VAddrX86 codeAddr, VAddrX86 stackPtr, VAddrX86* pTargets) const
{
    //
    // +--------+-------------+-------------------------------------------------------+
    // | Opcode | Instruction | Description                                           |
    // +--------+-------------+-------------------------------------------------------+
    // | FF /2  | CALL r/m32  | Call near, absolute indirect, address given in r/m32. |
    // | FF /3  | CALL m16:16 | Call far, absolute indirect, address given in m16:16. |
    // +--------+-------------+-------------------------------------------------------+
    //

    unsigned count = 0;

    // FF D4    call  esp
    if (2 <= len && 0xFF == (pCode - 2)[0] && ((2 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER | REGISTER_X86_ESP) == (pCode - 2)[1])
    {
        pTargets[count++] = stackPtr + X86_STACK_SIZE;
    }
    // FF 14 24    call  DWORD PTR[esp]
    else if (3 <= len && 0xFF == (pCode - 3)[0] && MAKE_WORD(((2 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_MEM | MODRM_MEM_USE_SIB), (SIB_INDEX_NONE | (REGISTER_X86_ESP << SIB_BASE_OFFSET))) == *reinterpret_cast<const WORD UNALIGNED*>((pCode - 3) + 1))
    {
        if (!m_context.ReadFullMemory(MEM_TYPE_ANY, stackPtr + X86_STACK_SIZE, pTargets[count]))
        {
            pTargets[count] = 0;
        }

        count++;
    }
    // FF 54 24 12    call  DWORD PTR[esp + 12h]
    else if (4 <= len && 0xFF == (pCode - 4)[0] && MAKE_WORD(((2 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_MEM_DISP8 | MODRM_MEM_USE_SIB), (SIB_INDEX_NONE | (REGISTER_X86_ESP << SIB_BASE_OFFSET))) == *reinterpret_cast<const WORD UNALIGNED*>((pCode - 4) + 1))
    {
        if (!m_context.ReadFullMemory(MEM_TYPE_ANY, stackPtr + static_cast<gtInt32>(*reinterpret_cast<const gtByte*>((pCode - 4) + 3)), pTargets[count]))
        {
            pTargets[count] = 0;
        }

        count++;
    }
    // FF 15 78 56 34 12    call  DWORD PTR ds:[12345678h]
    else if (6 <= len && 0xFF == (pCode - 6)[0] && ((2 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_MEM | MODRM_MEM_DISP32) == (pCode - 6)[1])
    {
        pTargets[count] = 0;

        //
        // We need to relocate the absolute address, according to the module's loading address.
        //

        VAddrX86 imageBaseAddr;
        VAddrX86 imageLoadAddr = m_context.GetImageLoadAddress(codeAddr - 6, imageBaseAddr);

        if (0 != imageLoadAddr)
        {
            VAddrX86 indirectFuncAddr = *reinterpret_cast<const gtUInt32 UNALIGNED*>((pCode - 6) + 2);
            indirectFuncAddr = (indirectFuncAddr - imageBaseAddr) + imageLoadAddr;
            m_context.ReadFullMemory(MEM_TYPE_CODE, indirectFuncAddr, pTargets[count]);
        }

        count++;
    }
    // FF 94 24 78 56 34 12    call  DWORD PTR[esp + 12345678h]
    else if (7 <= len && 0xFF == (pCode - 7)[0] && MAKE_WORD(((2 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_MEM_DISP32 | MODRM_MEM_USE_SIB), (SIB_INDEX_NONE | (REGISTER_X86_ESP << SIB_BASE_OFFSET))) == *reinterpret_cast<const WORD UNALIGNED*>((pCode - 7) + 1))
    {
        if (!m_context.ReadFullMemory(MEM_TYPE_ANY, stackPtr + *reinterpret_cast<const gtInt32 UNALIGNED*>((pCode - 7) + 3), pTargets[count]))
        {
            pTargets[count] = 0;
        }

        count++;
    }

    return count;
}

unsigned FrameChainWalker::ExtractJumpTarget(const gtUByte* pCode, VAddrX86 codeAddr, VAddrX86& targetAddr) const
{
    //
    // +--------+-------------+------------------------------------------------------------------+
    // | Opcode | Instruction | Description                                                      |
    // +--------+-------------+------------------------------------------------------------------+
    // | EB cb  | JMP rel8    | Jump short, relative, displacement relative to next instruction. |
    // | E9 cd  | JMP rel32   | Jump near, relative, displacement relative to next instruction.  |
    // | FF /4  | JMP r/m32   | Jump near, absolute indirect, address given in r/m32.            |
    // +--------+-------------+------------------------------------------------------------------+
    //

    unsigned length = 0;

    // EB 12    jmp  12h
    if (OPCODE_JMP_REL8 == pCode[0])
    {
        targetAddr = codeAddr + static_cast<gtInt32>(*reinterpret_cast<const gtByte*>(pCode + 1)) + 2;
        length = 2;
    }
    // E9 78 56 34 12    jmp  12345678h
    else if (OPCODE_JMP_REL == pCode[0])
    {
        targetAddr = codeAddr + *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 1) + 5;
        length = 5;
    }
    // FF 25 78 56 34 12    jmp  DWORD PTR ds:[12345678h]
    else if (MAKE_WORD(OPCODE_EXT, ((4 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_MEM | MODRM_MEM_DISP32)) == *reinterpret_cast<const WORD UNALIGNED*>(pCode))
    {
        //
        // We need to relocate the absolute address, according to the module's loading address.
        //

        VAddrX86 imageBaseAddr;
        VAddrX86 imageLoadAddr = m_context.GetImageLoadAddress(codeAddr, imageBaseAddr);

        if (0 != imageLoadAddr)
        {
            VAddrX86 indirectAddr = *reinterpret_cast<const gtUInt32 UNALIGNED*>(pCode + 2);
            indirectAddr = (indirectAddr - imageBaseAddr) + imageLoadAddr;

            if (m_context.ReadFullMemory(MEM_TYPE_CODE, indirectAddr, targetAddr))
            {
                length = 6;
            }
        }
    }

    return length;
}

unsigned FrameChainWalker::ExtractSubEsp(const gtUByte* pCode, gtInt32& subtrahend) const
{
    //
    // +----------+------------------+-----------------------------------------+
    // | Opcode   | Instruction      | Description                             |
    // +----------+------------------+-----------------------------------------+
    // | 81 /5 id | SUB r/m32, imm32 | Subtract imm32 from r/m32.              |
    // | 83 /5 ib | SUB r/m32, imm8  | Subtract sign-extended imm8 from r/m32. |
    // +----------+------------------+-----------------------------------------+
    //

    unsigned length = 0;

    // 83 EC 12    sub  esp, 12h
    if (MAKE_WORD(OPCODE_SUB_RM_IMM8, ((5 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER | REGISTER_X86_ESP)) == *reinterpret_cast<const gtUInt16 UNALIGNED*>(pCode))
    {
        subtrahend = static_cast<gtInt32>(*reinterpret_cast<const gtByte*>(pCode + 2));
        length = 3;
    }
    // 81 EC 78 56 34 12    sub  esp, 12345678h
    else if (MAKE_WORD(OPCODE_SUB_RM_IMM32, ((5 << MODRM_OPCODE_EXT_OFFSET) | MODRM_MOD32_REGISTER | REGISTER_X86_ESP)) == *reinterpret_cast<const gtUInt16 UNALIGNED*>(pCode))
    {
        subtrahend = *reinterpret_cast<const gtInt32 UNALIGNED*>(pCode + 2);
        length = 6;
    }

    return length;
}

HRESULT FrameChainWalker::ChaseAndValidateJumpChain(VAddrX86 endAddr, RVAddrX86 endBlockLen, VAddrX86 startAddr, bool estimated, VAddrX86& jumpTargetAddr)
{
    m_searchDepth = 0;
    return ChaseAndValidateJumpChain(endAddr, endBlockLen, startAddr, estimated, &jumpTargetAddr, 0);
}

HRESULT FrameChainWalker::ChaseAndValidateJumpChain(VAddrX86 endAddr, RVAddrX86 endBlockLen, VAddrX86 startAddr, bool estimated, VAddrX86* pJumpTargetAddr, unsigned depth)
{
    if (m_searchDepth++ > MAX_SEARCH_DEPTH)
    {
        return S_FALSE;
    }

    if (MAX_JUMP_CHAIN_DEPTH <= depth)
    {
        return S_FALSE;
    }

    for (unsigned i = 0; i < depth; ++i)
    {
        // We found an endless jump chain!
        if (startAddr == m_jumpChain[i])
        {
            return S_FALSE;
        }
    }

    m_jumpChain[depth++] = startAddr;

    HRESULT hr;
    VAddrX86 addr = startAddr;
    IDiaFrameData* pFrame = NULL;

    while (S_OK != m_context.FindFrameInterface(addr, &pFrame))
    {
        gtUByte aCode[INST_CALL_MAX_SIZE + CODE_BUFFER_OVERRUN_SIZE];
        memset(aCode, OPCODE_NOP, sizeof(aCode));

        gtUByte* pCode = aCode;
        int codeLength = m_context.ReadMemory(MEM_TYPE_CODE, addr, pCode, INST_CALL_MAX_SIZE);

        if (0 >= codeLength)
        {
            if (estimated && 1 == depth)
            {
                VAddrX86 borderAddr = endAddr + endBlockLen;

                if (borderAddr > addr)
                {
                    *pJumpTargetAddr = borderAddr - addr;
                }
            }

            return E_FAIL;
        }

        VAddrX86 jumpTargetAddr = 0;

        if (0 == ExtractJumpTarget(pCode, addr, jumpTargetAddr))
        {
            if (estimated && 1 == depth)
            {
                VAddrX86 borderAddr = endAddr + endBlockLen;

                if (borderAddr > addr)
                {
                    *pJumpTargetAddr = borderAddr - addr;
                }
            }

            hr = S_FALSE;
            return hr;
        }

        addr = jumpTargetAddr;

        VAddrX86 borderAddr = estimated ? endAddr + endBlockLen : endAddr;

        if (borderAddr == addr)
        {
            return S_OK;
        }
    }

    BOOL funcStart = FALSE;
    DWORD lengthBlock = 0;
    DWORD lengthProlog = 0;

    hr = pFrame->get_functionStart(&funcStart);

    if (FALSE == funcStart ||
        S_OK != (hr = pFrame->get_lengthBlock(&lengthBlock)) ||
        S_OK != (hr = pFrame->get_lengthProlog(&lengthProlog)) ||
        lengthBlock <= lengthProlog)
    {
        pFrame->Release();

        if (S_OK == hr)
        {
            hr = S_FALSE;
        }

        return hr;
    }

    pFrame->Release();

    int codeLength = static_cast<int>(lengthBlock - lengthProlog);

    gtUByte* pCode = getCodeBuffer(codeLength + CODE_BUFFER_OVERRUN_SIZE);

    if (NULL == pCode)
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

    memset(pCode, OPCODE_NOP, codeLength + CODE_BUFFER_OVERRUN_SIZE);
    VAddrX86 codeAddr = addr + lengthProlog;
    codeLength = m_context.ReadMemory(MEM_TYPE_CODE, codeAddr, pCode, codeLength); //TODO: This may be changed with PeFile::GetMemoryBlock(), so we won't have to allocate an array and copy the block to it.

    if (0 < codeLength)
    {
        hr = S_FALSE;

        DisassemblerX86 dasm;
        const gtUByte* pInst = pCode;
        const gtUByte* pInstEnd;
        const gtUByte* pCodeEnd = pCode + codeLength;

        while ((pInstEnd = dasm.CrackInstruction(pInst)) <= pCodeEnd)
        {
            VAddrX86 codeOffset = static_cast<VAddrX86>(pInst - pCode);
            VAddrX86 jumpTargetAddr = 0;

            if (static_cast<unsigned>(pInstEnd - pInst) == ExtractJumpTarget(pInst, codeAddr + codeOffset, jumpTargetAddr))
            {
                if (jumpTargetAddr == endAddr)
                {
                    hr = S_OK;
                    break;
                }

                if (!(addr <= jumpTargetAddr && jumpTargetAddr < (addr + lengthBlock)))
                {
                    if (S_OK == ChaseAndValidateJumpChain(endAddr, endBlockLen, jumpTargetAddr, 0, NULL, depth))
                    {
                        hr = S_OK;
                        break;
                    }
                }
            }

            if (pInstEnd >= pCodeEnd)
            {
                break;
            }

            pInst = pInstEnd;
        }
    }
    else
    {
        hr = (0 == codeLength) ? S_FALSE : E_FAIL;
    }

    return hr;
}

gtUByte* FrameChainWalker::getCodeBuffer(size_t requestedSize)
{
    gtUByte* retval = NULL;

    if (requestedSize <= m_codeBufferSize)
    {
        retval = m_codeBuffer.data();
    }
    else // buffer is not big enough - try to increase its size
    {
        try
        {
            m_codeBuffer.resize(requestedSize);
            m_codeBufferSize = requestedSize;
            retval = m_codeBuffer.data();
        }
        catch (std::exception e)
        {
            OS_OUTPUT_DEBUG_LOG(L"Memory allocation failed in FrameChainWalker::getCodeBuffer()", OS_DEBUG_LOG_ERROR);
        }
    }

    return retval;
}

static unsigned int DeterminePrologType(const wchar_t* pFuncName, unsigned int lenFuncName)
{
    unsigned int type = 0U;

    if (10U <= lenFuncName && L'_' == *pFuncName)
    {
        pFuncName++;
        lenFuncName--;

        if (L'_' == *pFuncName)
        {
            pFuncName++;
            lenFuncName--;
        }

        const unsigned int lenProlog = wcslen(L"EH_prolog");

        if (lenProlog <= lenFuncName && memcmp(pFuncName, L"EH_prolog", lenProlog * sizeof(wchar_t)))
        {
            pFuncName += lenProlog;

            // __EH_prolog
            // _EH_prolog
            if (L'\0' == pFuncName[0])
            {
                type = 1U;
            }
            // __EH_prolog2
            // _EH_prolog2
            else if (L'2' == pFuncName[0] && L'\0' == pFuncName[1])
            {
                type = 2U;
            }

            // TODO: support prolog3 and prolog4
        }
    }

    return type;
}

