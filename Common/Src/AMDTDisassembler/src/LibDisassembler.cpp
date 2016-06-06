//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file LibDisassembler.cpp
///
//==================================================================================

#ifdef _WINDOWS
    #include <windows.h>
#else
    #include "typedefs_lin.h"
#endif

#if !defined(_WINDOWS)
    #define STDMETHODIMP    HRESULT
#endif

#include "Disasmwrapper.h"
#include "LibDisassembler.h"
#include <string.h>

/////////////////////////////////////////////////////////////////////////////
// LibDisassembler
LibDisassembler::LibDisassembler() : m_pWrapper(NULL), m_dbit(TRUE), m_lbit(FALSE), m_proctype(ANY)
{
    m_pWrapper = new CDisasmwrapper;

    if (m_pWrapper)
    {
        if (m_dbit)
        {
            m_pWrapper->SetDbit();
        }
        else
        {
            m_pWrapper->ClearDbit();
        }

        //set 64 bit mod
        if (m_lbit)
        {
            m_pWrapper->SetLongMode();
        }
        else
        {
            m_pWrapper->ClearLongMode();
        }
    }
}

LibDisassembler::~LibDisassembler()
{
    if (NULL != m_pWrapper)
    {
        delete m_pWrapper;
        m_pWrapper = NULL;
    }
}
HRESULT LibDisassembler::SetLongMode(BOOL DefaultIs32Bits)
{
    m_lbit = DefaultIs32Bits;

    if (m_pWrapper)
    {

        //set 64 bit mod
        if (m_lbit)
        {
            // If long mod bit is on, should set d bit off
            m_dbit = FALSE;
            m_pWrapper->ClearDbit();
            m_pWrapper->SetLongMode();
        }
        else
        {
            m_pWrapper->ClearLongMode();
        }
    }

    return S_OK;
}

HRESULT LibDisassembler::SetProcessorType(eProcType CurrentProcType)
{
    if (((CurrentProcType < K7) || (CurrentProcType > K8))
        && (CurrentProcType != ANY))
    {
        return S_FALSE ;
    }

    m_proctype = CurrentProcType ;

    return S_OK;
}

// *** TO DO *** return error code in HRESULT hr
HRESULT LibDisassembler::UIDisassemble(
    BYTE* rawcode,
    unsigned int* StrLength,
    BYTE* ASCIICode,
    UIInstInfoType* pDisasmInfo,
    BYTE* ErrorCode)
{
    char* pDisasmString;

    *ErrorCode = static_cast<BYTE>(-1);

    memset(ASCIICode, 0, *StrLength);

    pDisasmInfo->NumBytesUsed = 0x00;
    pDisasmInfo->NumOpCodeBytes = 0x00;
    pDisasmInfo->OpCodeByte = 0x00;
    pDisasmInfo->bHasDispData = FALSE;
    pDisasmInfo->DispDataValue = 0;

    pDisasmInfo->bIsPCRelative = FALSE;

    for (int ii = 0; ii < MAX_OPERANDS; ii++)
    {
        pDisasmInfo->bHasMemOp[ii] = FALSE;
        pDisasmInfo->MemAccessSize[ii] = 0;
    }

    if (m_pWrapper)
    {
        *ErrorCode = NoError;
        pDisasmString = m_pWrapper->Disassemble(rawcode);

        if (pDisasmString != NULL)
        {
            pDisasmInfo->NumBytesUsed = (BYTE)m_pWrapper->GetLength();

            pDisasmInfo->NumOpCodeBytes = (BYTE)m_pWrapper->GetNumOpcodeBytes();
            pDisasmInfo->OpCodeByte = m_pWrapper->GetOpcode(0);

            if (m_pWrapper->HasDisplacement())
            {
                pDisasmInfo->bHasDispData = TRUE;
                pDisasmInfo->DispDataValue = (unsigned int) m_pWrapper->GetDisplacement();
            }
            else
            {
                pDisasmInfo->bHasDispData = FALSE;
            }

            for (int i = 0; i < m_pWrapper->GetNumOperands(); i++)
            {
                int type = m_pWrapper->GetOperandType(i);

                if ((n_Disassembler::OPERANDTYPE_RIPRELATIVE == type)
                    || (n_Disassembler::OPERANDTYPE_PCOFFSET == type))
                {
                    pDisasmInfo->bIsPCRelative = TRUE;
                    //              break;
                }

                if ((n_Disassembler::OPERANDTYPE_RIPRELATIVE == type)
                    || (n_Disassembler::OPERANDTYPE_PCOFFSET == type)
                    || (n_Disassembler::OPERANDTYPE_MEMORY == type))
                {
                    if (pDisasmInfo->bHasMemOp[i])
                    {
                        continue;
                    }

                    pDisasmInfo->bHasMemOp[i] = TRUE;

                    switch (m_pWrapper->GetOperandSize(i))
                    {
                        case n_Disassembler::OPERANDSIZE_8:
                            pDisasmInfo->MemAccessSize[i] = 1;
                            break;

                        case n_Disassembler::OPERANDSIZE_16:
                            pDisasmInfo->MemAccessSize[i] = 2;
                            break;

                        case n_Disassembler::OPERANDSIZE_32:
                            pDisasmInfo->MemAccessSize[i] = 4;
                            break;

                        case n_Disassembler::OPERANDSIZE_48:
                            pDisasmInfo->MemAccessSize[i] = 6;
                            break;

                        case n_Disassembler::OPERANDSIZE_64:
                            pDisasmInfo->MemAccessSize[i] = 8;
                            break;

                        case n_Disassembler::OPERANDSIZE_80:
                            pDisasmInfo->MemAccessSize[i] = 10;
                            break;

                        case n_Disassembler::OPERANDSIZE_128:
                            pDisasmInfo->MemAccessSize[i] = 16;
                            break;

                        case n_Disassembler::OPERANDSIZE_256:
                            pDisasmInfo->MemAccessSize[i] = 32;
                            break;

                        case n_Disassembler::OPERANDSIZE_NONE:
                            pDisasmInfo->MemAccessSize[i] = 0;
                            break;

                        default:
                            pDisasmInfo->MemAccessSize[i] = 0;
                            break;
                    }

                    //              break;
                }
            }

            *ErrorCode = NoError;

            strncpy_s((char*)ASCIICode, *StrLength, pDisasmString, strlen(pDisasmString));
            *StrLength = static_cast<UINT>(strlen((char*)ASCIICode));
        }
        else
        {
            *ErrorCode = ErrInDE;
        }
    }

    if (*ErrorCode)
    {
        return E_FAIL;
    }
    else
    {
        return S_OK;
    }

}

void LibDisassembler::ResetEffectiveAddr()
{
    m_bEffAddrBaseRegPresent = FALSE;
    m_bEffAddrIndexRegPresent = FALSE;
    m_EffAddrBaseReg = 0x00;
    m_EffAddrIndexReg = 0x00;
    m_EffAddrScale = 0x01;
    m_EffAddrDisp = 0;
    m_EffectiveAddress = 0;
    m_OperandSize = evSizeNone;
    m_MemAccessType = nA;
}

// Write effective address in trace record
void LibDisassembler::WriteEffectiveAddr(UINT64* pEffAddr)
{
#pragma pack(1)
    struct _TraceRecord
    {
        BYTE Header ;
        unsigned int Low32BitsData ;
        unsigned int High32BitsData ;
    } * pRec ;
#pragma pack()
    pRec = (_TraceRecord*) m_pTraceRecords ;

    // calculate effective Address for both 64 bit and 32 bit format
    m_EffectiveAddress  = m_bEffAddrBaseRegPresent ?
                          m_pContextRegs[m_EffAddrBaseReg] : 0 ;
    m_EffectiveAddress += m_bEffAddrIndexRegPresent ?
                          m_pContextRegs[m_EffAddrIndexReg] * m_EffAddrScale : 0 ;
    m_EffectiveAddress += m_EffAddrDisp ;

    (*pEffAddr) = m_EffectiveAddress;
    m_EffectiveAddrCount++;

    // Set the access type, Header.7:4
    pRec->Header |= m_MemAccessType ;

    // Set the type of memory operand, Header.2:0
    switch (m_OperandSize)
    {
        case evSizeQWord  : pRec->Header |= 0x00 ; break ;

        case evSizeByte   : pRec->Header |= 0x01 ; break ;

        case evSizeWord   : pRec->Header |= 0x02 ; break ;

        case evSizeDWord  : pRec->Header |= 0x04 ; break ;

        // this case is for 64-bit trace format
        //case evSizeDQWord : pRec->Header |= 0x05 ; break ;

        case evSizeFWord  : pRec->Header |= 0x06 ; break ;

        case evSizeTByte  : pRec->Header |= 0x07 ; break ;

        default           : pRec->Header |= 0x04 ;
    }

    //According to the speedtracer 64 documentation, if we're running in
    //  compatibility mode (lma=1 && m_lbit = 0), the data length is usually
    //  4 bytes (32-bit format), but some hidden accesses to descriptor tables,
    //  page tables, etc will require 8 byte addresses.  As far as I can tell,
    //  the "hidden accesses" are actually from the 64-bit OS maintaining the
    //  32-bit compatibility.  As this disassembler is targeted mainly towards
    //  CodeAnalyst, and the applications traced will probably be either 32-bit
    //  or 64-bit, I don't think we have to worry about that exception here.
    if (m_lbit)
    {
        // 64 bit format
        // Header.3 = data length, 8-bytes equ 1;
        pRec->Header |= 0x08;

        if (evSizeDQWord == m_OperandSize)
        {
            pRec->Header |= 0x05;
        }

        pRec->Low32BitsData = (unsigned int) m_EffectiveAddress ;
        INT64 high32bits = m_EffectiveAddress >> 32;
        pRec->High32BitsData = (unsigned int) high32bits ;
        //Advance to the beginning of the next record to write, if any
        m_pTraceRecords += sizeof(pRec->Header) + sizeof(pRec->Low32BitsData) + sizeof(pRec->High32BitsData);

    }
    else
    {
        // 32 bit trace format

        //Header.3 = type of address, virtual equ 0; physical que 1;
        // default is virtual

        pRec->Low32BitsData = (unsigned int) m_EffectiveAddress ;
        //Advance to the beginning of the next record to write, if any
        m_pTraceRecords += sizeof(pRec->Header) + sizeof(pRec->Low32BitsData) ;
    }

    ResetEffectiveAddr();
}

HRESULT LibDisassembler::TGDisassemble(
    BYTE* RawCodeBytes,
    TgInstInfoType* pDisasmInfo,
    UINT64 ContextRegs[],
    unsigned int* pBuffSize,
    BYTE TraceRecs[],
    UINT64* pEffectiveAddrArray,
    DWORD* pEffectiveAddrCount,
    BYTE* ErrorCode)
{
    *ErrorCode = static_cast<BYTE>(-1);
    pDisasmInfo->NumBytesUsed = 0;
    pDisasmInfo->InstrPrefix = 0x00;
    pDisasmInfo->InstrPfxType = evNoPrefix;
    pDisasmInfo->NumOpCodeBytes = 0;
    pDisasmInfo->OpCodeByte = 0x00;
    pDisasmInfo->InstSpecies = evNASpecies;

    m_EffectiveAddrCount = 0;

    char* pDisasmString;

    if (m_pWrapper)
    {
        *ErrorCode = NoError;
        pDisasmString = m_pWrapper->Disassemble(RawCodeBytes);

        if (pDisasmString != NULL)
        {
            pDisasmInfo->NumBytesUsed = (BYTE)m_pWrapper->GetLength();

            pDisasmInfo->NumOpCodeBytes = (BYTE)m_pWrapper->GetNumOpcodeBytes();
            pDisasmInfo->OpCodeByte = m_pWrapper->GetOpcode(0);

            if (m_pWrapper->HasLockPrefix())
            {
                pDisasmInfo->InstrPfxType = evLOCK;
                pDisasmInfo->InstrPrefix = 0xF0;
            }

            if (m_pWrapper->HasRepnePrefix())
            {
                pDisasmInfo->InstrPfxType = evREPNE;
                pDisasmInfo->InstrPrefix = 0xF2;
            }

            if (m_pWrapper->HasRepPrefix())
            {
                pDisasmInfo->InstrPrefix = 0xF3;
                pDisasmInfo->InstrPfxType = evNoPrefix;

                if ((0xA6 == pDisasmInfo->OpCodeByte)
                    || (0xA7 == pDisasmInfo->OpCodeByte)
                    || (0xAE == pDisasmInfo->OpCodeByte)
                    || (0xAF == pDisasmInfo->OpCodeByte))
                {
                    pDisasmInfo->InstrPfxType = evREPE ;
                }
                else
                {
                    pDisasmInfo->InstrPfxType = evREP ;
                }

            }

            CInstr_ExtraCodes* extracodes = (CInstr_ExtraCodes*)(m_pWrapper->GetExtraInfoPtr());

            if (NULL == extracodes)
            {
                *ErrorCode = ErrInTableHookup;
            }
            else
            {
                pDisasmInfo->InstSpecies = extracodes->instr_table.InstSpecies;
                *ErrorCode = NoError;

                // Write TraceRecord;
                m_bEffAddrBaseRegPresent = FALSE;
                m_bEffAddrIndexRegPresent = FALSE;
                m_EffAddrBaseReg = 0x00;
                m_EffAddrIndexReg = 0x00;
                m_EffAddrScale = 0x01;
                m_EffAddrDisp = 0;
                m_EffectiveAddress = 0;
                m_TraceBuffSize = 0;
                m_OperandSize = evSizeNone;
                m_MemAccessType = nA;

                memset(TraceRecs, 0, *pBuffSize) ;
                m_pContextRegs = ContextRegs ;
                m_TraceBuffSize = *pBuffSize ;
                m_pTraceRecords = TraceRecs ;

                ////////////////////////////////////////////

                int NumOperandsInWrapper, NumOperandsInDE, numimplicit;
                NumOperandsInWrapper = extracodes->instr_table.NumOperands;
                NumOperandsInDE = m_pWrapper->GetNumOperands();

                if (NumOperandsInDE > NumOperandsInWrapper)
                {
                    *ErrorCode = ErrInTableHookup;
                }
                else if (NumOperandsInDE == NumOperandsInWrapper)
                {
                    for (int j = 0; j < NumOperandsInDE; j++)
                    {
                        if ((n_Disassembler::OPERANDTYPE_MEMORY == m_pWrapper->GetOperandType(j)) ||
                            (((n_Disassembler::OPERANDTYPE_RIPRELATIVE == m_pWrapper->GetOperandType(j)) ||
                              (n_Disassembler::OPERANDTYPE_PCOFFSET == m_pWrapper->GetOperandType(j))) &&
                             m_pWrapper->HasModrm()))
                        {
                            if (m_pWrapper->HasBase())
                            {
                                m_bEffAddrBaseRegPresent = TRUE;
                                m_EffAddrBaseReg = (BYTE)m_pWrapper->GetBaseRegister();
                            }

                            if (m_pWrapper->OperandHasIndex(j))
                            {
                                m_bEffAddrIndexRegPresent = TRUE;
                                m_EffAddrIndexReg = (BYTE)m_pWrapper->GetOperandIndexRegister(j);
                            }

                            if (m_pWrapper->HasScale())
                            {
                                m_EffAddrScale = 0x01 << m_pWrapper->GetSibScale();
                            }

                            if (m_pWrapper->HasDisplacement())
                            {
                                m_EffAddrDisp =  m_pWrapper->GetDisplacement();
                            }

                            //If needed, add the current ip to make pc relative
                            if (n_Disassembler::OPERANDTYPE_RIPRELATIVE == m_pWrapper->GetOperandType(j))
                            {
                                m_EffAddrDisp += m_pContextRegs[/*Rip*/16] + m_pWrapper->GetLength();
                            }

                            m_OperandSize = (eOperandSize)(m_pWrapper->GetOperandSize(j));

                            switch (j)
                            {
                                case 0:
                                    m_MemAccessType = extracodes->instr_table.OpField1.MemAccessType;
                                    break;

                                case 1:
                                    m_MemAccessType = extracodes->instr_table.OpField2.MemAccessType;
                                    break;

                                case 2:
                                    m_MemAccessType = extracodes->instr_table.OpField3.MemAccessType;
                                    break;
                            }

                            if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                            {
                                WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                            }
                        }
                    }// end of for loop
                }
                else    // there is implicit reg/mem
                {
                    numimplicit = 0;

                    for (int i = 0; i < NumOperandsInWrapper; i++)
                    {
                        COperandField tempoperand;

                        switch (i)
                        {
                            case 0:
                                tempoperand = extracodes->instr_table.OpField1;
                                break;

                            case 1:
                                tempoperand = extracodes->instr_table.OpField2;
                                break;

                            case 2:
                                tempoperand = extracodes->instr_table.OpField3;
                                break;
                        }   // end of switch (i)

                        if (tempoperand.IsImplicit())
                        {
                            numimplicit += 1;
                            //This is implicit memory access, we are going to write trace recorder base on it's evInstSpecies later.
                        }
                        else
                        {
                            int j = i - numimplicit;

                            if (n_Disassembler::OPERANDTYPE_MEMORY == m_pWrapper->GetOperandType(j))
                            {
                                if (m_pWrapper->HasBase())
                                {
                                    m_bEffAddrBaseRegPresent = TRUE;
                                    m_EffAddrBaseReg = (BYTE)m_pWrapper->GetBaseRegister();
                                }

                                if (m_pWrapper->OperandHasIndex(j))
                                {
                                    m_bEffAddrIndexRegPresent = TRUE;
                                    m_EffAddrIndexReg = (BYTE)m_pWrapper->GetOperandIndexRegister(j);
                                }

                                if (m_pWrapper->HasScale())
                                {
                                    m_EffAddrScale = 0x01 << m_pWrapper->GetSibScale();
                                }

                                if (m_pWrapper->HasDisplacement())
                                {
                                    m_EffAddrDisp =  m_pWrapper->GetDisplacement();
                                }

                                //m_OperandSize = (eOperandSize) (m_pWrapper->GetOperandSize(j) - n_Disassembler::OPERANDSIZE_NONE);
                                m_OperandSize = (eOperandSize)(m_pWrapper->GetOperandSize(j));

                                switch (i)
                                {
                                    case 0:
                                        m_MemAccessType = extracodes->instr_table.OpField1.MemAccessType;
                                        break;

                                    case 1:
                                        m_MemAccessType = extracodes->instr_table.OpField2.MemAccessType;
                                        break;

                                    case 2:
                                        m_MemAccessType = extracodes->instr_table.OpField3.MemAccessType;
                                        break;
                                }

                                if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                                {
                                    WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                                }
                            }
                        }
                    }
                }

                // The implicit operands which access memory
                switch (extracodes->instr_table.InstSpecies)
                {
                    case evPushaSpecies:        // 16 bits ignore now
                    case evPushaqSpecies:       // 64 bits
                    case evPushadSpecies:       // 32 bits
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is ESP/RSP

                        if (evPushaqSpecies == extracodes->instr_table.InstSpecies)
                        {
                            // invalid for 64bits(from AMD menu), but Robert's Engine supprots. Leave it here for future
                            m_OperandSize = evSizeQWord;
                            m_pContextRegs[m_EffAddrBaseReg] -= 64;     // pre-decrement stack pointer by 64
                        }
                        else if (evPushadSpecies == extracodes->instr_table.InstSpecies)
                        {
                            m_OperandSize = evSizeDWord;
                            m_pContextRegs[m_EffAddrBaseReg] -= 32;     // pre-decrement stack pointer by 32
                        }
                        else
                        {
                            m_OperandSize = evSizeWord;
                            m_pContextRegs[m_EffAddrBaseReg] -= 16;     // pre-decrement stack pointer by 16
                        }

                        m_MemAccessType = WW;
                        m_pContextRegs[m_EffAddrBaseReg] -= (m_OperandSize == evSizeDWord) ? 32 : 16;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evPushSpecies:
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is ESP/RSP

                        if (0x6A == m_pWrapper->GetOpcode(0))
                        {
                            m_OperandSize = evSizeByte;
                            m_pContextRegs[m_EffAddrBaseReg] -= 1;          // pre-decrement stack pointer by 1 byte;
                        }
                        else if (!m_lbit)
                        {
                            if (!m_dbit)
                            {
                                m_OperandSize = evSizeWord;
                                m_pContextRegs[m_EffAddrBaseReg] -= 2;          // pre-decrement stack pointer by 2 byte;
                            }
                            else
                            {
                                m_OperandSize = evSizeDWord;
                                m_pContextRegs[m_EffAddrBaseReg] -= 4;          // pre-decrement stack pointer by 4 byte;
                            }
                        }
                        else
                        {
                            m_OperandSize = evSizeQWord;
                            m_pContextRegs[m_EffAddrBaseReg] -= 8;          // pre-decrement stack pointer by 8 byte;
                        }

                        m_MemAccessType = WW;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evPushfqSpecies:
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is RSP
                        m_OperandSize = evSizeQWord;
                        m_pContextRegs[m_EffAddrBaseReg] -= 8;          // pre-decrement stack pointer by 8 byte;
                        m_MemAccessType = WW;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evPushfdSpecies:
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is ESP
                        m_OperandSize = evSizeQWord;
                        m_pContextRegs[m_EffAddrBaseReg] -= 4;          // pre-decrement stack pointer by 4 byte;
                        m_MemAccessType = WW;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evEnterSpecies:
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is ESP

                        // No 16 bits support ENTER, PUSH[EBP]
                        m_OperandSize = evSizeDWord;
                        m_MemAccessType = WW;
                        m_pContextRegs[m_EffAddrBaseReg] -= 4;      // pre-decrement stack pointer by 4 ;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evCallSpecies:
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is ESP/RSP

                        if (m_lbit)
                        {
                            m_OperandSize = evSizeQWord;
                            m_pContextRegs[m_EffAddrBaseReg] -= 8 ;     // pre-decrement stack pointer by 8;
                        }
                        else
                        {
                            m_OperandSize = evSizeDWord;
                            m_pContextRegs[m_EffAddrBaseReg] -= 4 ;     // pre-decrement stack pointer by 4;
                        }

                        m_MemAccessType = WW;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evPopSpecies:
                    case evPopadSpecies:
                    case evPopaqSpecies:
                    case evPopfdSpecies:
                    case evLeaveSpecies:
                    case evRetSpecies:
                    case evIretSpecies:
                        m_bEffAddrBaseRegPresent = TRUE ;
                        m_EffAddrBaseReg = 4;   // base register is ESP/RSP

                        if (m_lbit)
                        {
                            m_OperandSize = evSizeQWord;
                        }
                        else
                        {
                            m_OperandSize = evSizeDWord;
                        }

                        if (evPopaqSpecies == extracodes->instr_table.InstSpecies)
                        {
                            m_OperandSize = evSizeQWord;
                        }

                        if (evIretSpecies == extracodes->instr_table.InstSpecies)
                        {
                            m_MemAccessType = WW;
                        }
                        else
                        {
                            m_MemAccessType = RR;
                        }

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evXlatbSpecies:
                        m_MemAccessType = RR;
                        m_OperandSize = evSizeByte;
                        //The XLAT instruction implicitely accesses memory location DS:[(E)BX + AL]
                        m_EffAddrDisp = m_pContextRegs[3] + (unsigned int)(m_pContextRegs[0] & 0xFF) ;

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    case evBoundSpecies:
                        m_MemAccessType = RR ;  // Read access mode
                        m_OperandSize = evSizeDWord;
                        m_EffAddrDisp = *((INT64*)(m_pTraceRecords - 4)) + 4 ;     // Major kludge!!!

                        if (m_EffectiveAddrCount < *pEffectiveAddrCount)
                        {
                            WriteEffectiveAddr(&pEffectiveAddrArray[m_EffectiveAddrCount]);
                        }

                        break;

                    default:
                        break;      // break switch
                }

                *pBuffSize = static_cast<UINT>(m_pTraceRecords - TraceRecs) ;
            }
        }
        else
        {
            *ErrorCode = ErrInDE;
        }

        (*pEffectiveAddrCount) = m_EffectiveAddrCount;
    }

    if (*ErrorCode)
    {
        return E_FAIL;
    }
    else
    {
        return S_OK;
    }
}


HRESULT LibDisassembler::EtchDisassemble(
    BYTE* RawCodeBytes,
    InstructionType* pInstruction,
    BYTE* ErrorCode)
{
    memset(pInstruction, 0x00, sizeof(InstructionType));

    char* pDisasmString;

    *ErrorCode = static_cast<BYTE>(-1);

    if (m_pWrapper)
    {
        pDisasmString = m_pWrapper->Disassemble(RawCodeBytes);

        *ErrorCode = NoError;

        while (1)
        {
            if (pDisasmString == NULL)
            {
                *ErrorCode = ErrInDE;
                break;  // break while
            }

            pInstruction->NumBytesUsed = (BYTE)m_pWrapper->GetLength();

            pInstruction->NumOpCodeBytes = (BYTE)m_pWrapper->GetNumOpcodeBytes();
            pInstruction->OpCodeByte = m_pWrapper->GetOpcode(0);

            // Instruction prefix
            if (m_pWrapper->HasLockPrefix())
            {
                pInstruction->InstrPfxType = evLOCK;
                pInstruction->InstrPrefix = 0xF0;
            }

            if (m_pWrapper->HasRepnePrefix())
            {
                pInstruction->InstrPfxType = evREPNE;
                pInstruction->InstrPrefix = 0xF2;
            }

            if (m_pWrapper->HasRepPrefix())
            {
                pInstruction->InstrPrefix = 0xF3;
                pInstruction->InstrPfxType = evNoPrefix;

                if ((0xA6 == pInstruction->OpCodeByte)
                    || (0xA7 == pInstruction->OpCodeByte)
                    || (0xAE == pInstruction->OpCodeByte)
                    || (0xAF == pInstruction->OpCodeByte))
                {
                    pInstruction->InstrPfxType = evREPE ;
                }
                else
                {
                    pInstruction->InstrPfxType = evREP ;
                }

            }

            // segment prefix
            if (m_pWrapper->HasSegOvrdPrefix())
            {
                switch (m_pWrapper->GetSegmentRegister())
                {
                    case 0:
                        pInstruction->SegmentPrefix = 0x26; // ES
                        break;

                    case 1:
                        pInstruction->SegmentPrefix = 0x2E; // CS
                        break;

                    case 2:
                        pInstruction->SegmentPrefix = 0x36; // SS
                        break;

                    case 3:
                        pInstruction->SegmentPrefix = 0x3E; // DS
                        break;

                    case 4:
                        pInstruction->SegmentPrefix = 0x64; // FS
                        break;

                    case 5:
                        pInstruction->SegmentPrefix = 0x65; // GS
                        break;

                    default:
                        //error if go here
                        break;
                }
            }
            else
            {
                pInstruction->SegmentPrefix = 0x00;
            }

            // Address size override prefix
            if (m_pWrapper->HasAddressOvrdPrefix())
            {
                pInstruction->bAddrSizePrefix = TRUE;
            }
            else
            {
                pInstruction->bAddrSizePrefix = FALSE;
            }

            // operand size override prefix
            if (m_pWrapper->HasDataOvrdPrefix())
            {
                pInstruction->bOpSizePrefix = TRUE;
            }
            else
            {
                pInstruction->bOpSizePrefix = FALSE;
            }

            // ModRM
            if (m_pWrapper->HasModrm())
            {
                pInstruction->bModRMPresent = TRUE;
                pInstruction->ModRMValue = m_pWrapper->GetModrm();
            }
            else
            {
                pInstruction->bModRMPresent = FALSE;
            }

            // SIB
            if (m_pWrapper->HasSib())
            {
                pInstruction->bSIBPresent = TRUE;
                pInstruction->SIBValue = m_pWrapper->GetSib();
            }
            else
            {
                pInstruction->bSIBPresent = FALSE;
            }

            // Immediate data
            if (m_pWrapper->HasImmediateData())
            {
                pInstruction->bHasImmData = TRUE;
                pInstruction->ImmDataValue = m_pWrapper->GetImmediate();
                pInstruction->ImmDataOffset = (BYTE)m_pWrapper->GetImmediateOffset();
            }
            else
            {
                pInstruction->bHasImmData = FALSE;
            }


            if (m_pWrapper->HasDisplacement())
            {
                pInstruction->bHasDispData = TRUE;
                pInstruction->DispDataValue = (unsigned int) m_pWrapper->GetDisplacement();
                pInstruction->DispDataOffset = (BYTE)m_pWrapper->GetDisplacementOffset();
            }
            else
            {
                pInstruction->bHasDispData = FALSE;
            }

            CInstr_ExtraCodes* extracodes = (CInstr_ExtraCodes*)(m_pWrapper->GetExtraInfoPtr());

            if (NULL == extracodes)
            {
                *ErrorCode = ErrInTableHookup;
                break;  // break while
            }

            pInstruction->InstSpecies = extracodes->instr_table.InstSpecies;
            pInstruction->InstProcType = extracodes->instr_table.ProcessorType;
            strcpy_s(pInstruction->Mnemonic, MAX_MNEMONIC_LENGTH, extracodes->instr_table.Mnemonic);

            int NumOperandsInWrapper, NumOperandsInDE, numimplicit;
            NumOperandsInWrapper = extracodes->instr_table.NumOperands;
            NumOperandsInDE = m_pWrapper->GetNumOperands();

            if (NumOperandsInDE > NumOperandsInWrapper)
            {
                *ErrorCode = ErrInTableHookup;
                break;  // break while
            }
            else if (NumOperandsInDE == NumOperandsInWrapper)
            {
                pInstruction->NumOperands = (BYTE)NumOperandsInDE;

                for (int i = 0; i < NumOperandsInDE; i++)
                {
                    pInstruction->Operands[i].OpSize = (eOperandSize)(m_pWrapper->GetOperandSize(i));

                    switch (m_pWrapper->GetOperandType(i))
                    {
                        case n_Disassembler::OPERANDTYPE_REGISTER:

                            switch (pInstruction->Operands[i].OpSize)
                            {
                                case evSizeByte:
                                    pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i)
                                                                                   - evREG_EAX + evREG_AL);
                                    break;

                                case evSizeWord:
                                    pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i)
                                                                                   - evREG_EAX + evREG_AX);
                                    break;

                                case evSizeDWord:
                                    pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i));
                                    break;

                                case evSizeTByte:
                                    pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i));
                                    break;

                                default:
                                    pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i));
                                    break;
                            }

                            pInstruction->Operands[i].AddrMode = evRegister;
                            break;

                        case n_Disassembler::OPERANDTYPE_IMMEDIATE:
                            pInstruction->Operands[i].AddrMode = evImmediate;
                            break;

                        case n_Disassembler::OPERANDTYPE_RIPRELATIVE:
                            pInstruction->Operands[i].AddrMode = evPCRelative;
                            break;

                        case n_Disassembler::OPERANDTYPE_NONE:

                            switch (extracodes->instr_table.InstSpecies)
                            {
                                case evRclSpecies:
                                case evRcrSpecies:
                                case evRolSpecies:
                                case evRorSpecies:
                                    pInstruction->Operands[i].AddrMode = evImmediate;
                                    pInstruction->bHasImmData = TRUE;
                                    pInstruction->ImmDataValue = 1;
                                    pInstruction->ImmDataOffset = 0;
                                    pInstruction->Operands[i].OpSize = evSizeByte;
                                    pInstruction->Operands[i].IsImplicit = TRUE;
                                    break;

                                default:
                                    break;
                            }

                            break;

                        case n_Disassembler::OPERANDTYPE_MEMORY:
                            pInstruction->Operands[i].AddrMode = evMemory;

                            if (m_pWrapper->HasBase())
                            {
                                pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetBaseRegister());
                                pInstruction->Operands[i].bBaseRegPresent = TRUE;
                            }
                            else
                            {
                                pInstruction->Operands[i].bBaseRegPresent = FALSE;
                            }

                            if (m_pWrapper->OperandHasIndex(i))
                            {
                                switch (pInstruction->Operands[i].OpSize)
                                {
                                    case evSizeDWord:
                                        pInstruction->Operands[i].IndexReg = (eRegType) m_pWrapper->GetOperandIndexRegister(i);
                                        break;

                                    default:
                                        pInstruction->Operands[i].IndexReg = (eRegType) m_pWrapper->GetOperandIndexRegister(i);
                                        break;
                                }

                                pInstruction->Operands[i].bIndexRegPresent = TRUE;
                            }
                            else
                            {
                                pInstruction->Operands[i].bIndexRegPresent = FALSE;
                            }

                            pInstruction->Operands[i].ScaleFactor = 0x01;

                            if (m_pWrapper->HasScale())
                            {
                                pInstruction->Operands[i].ScaleFactor = 0x01 << m_pWrapper->GetSibScale();
                            }


                            switch (i)
                            {
                                case 0:
                                    pInstruction->Operands[i].MemAccessType =
                                        extracodes->instr_table.OpField1.MemAccessType;

                                    if (nA == pInstruction->Operands[i].MemAccessType)
                                    {
                                        *ErrorCode = ErrInTableHookup;
                                    }

                                    break;

                                case 1:
                                    pInstruction->Operands[i].MemAccessType =
                                        extracodes->instr_table.OpField2.MemAccessType;

                                    if (nA == pInstruction->Operands[i].MemAccessType)
                                    {
                                        *ErrorCode = ErrInTableHookup;
                                    }

                                    break;

                                case 2:
                                    pInstruction->Operands[i].MemAccessType =
                                        extracodes->instr_table.OpField3.MemAccessType;

                                    if (nA == pInstruction->Operands[i].MemAccessType)
                                    {
                                        *ErrorCode = ErrInTableHookup;
                                    }

                                    break;
                            }   // end of swithc (i)

                            break;

                        default:
                            break;
                    } // end of switch
                }   // end of for
            }
            else        // there are implicit operands
            {
                numimplicit = 0;
                pInstruction->NumOperands = (BYTE)NumOperandsInWrapper;

                for (int i = 0; i < NumOperandsInWrapper; i++)
                {
                    COperandField tempoperand;

                    switch (i)
                    {
                        case 0:
                            tempoperand = extracodes->instr_table.OpField1;
                            break;

                        case 1:
                            tempoperand = extracodes->instr_table.OpField2;
                            break;

                        case 2:
                            tempoperand = extracodes->instr_table.OpField3;
                            break;
                    }   // end of switch (i)

                    if (tempoperand.IsImplicit())
                    {
                        numimplicit += 1;
                        pInstruction->Operands[i].OpSize = tempoperand.GetOperandFieldSize();
                        pInstruction->Operands[i].BaseReg = tempoperand.GetOperandFieldReg();
                        pInstruction->Operands[i].IsImplicit = TRUE;

                        if (nA == tempoperand.MemAccessType)
                        {
                            pInstruction->Operands[i].AddrMode = evRegister;
                        }
                        else
                        {
                            pInstruction->Operands[i].AddrMode = evMemory;
                            pInstruction->Operands[i].bBaseRegPresent = TRUE;

                            if (tempoperand.AddrMethod != _EBXAL)
                            {
                                pInstruction->Operands[i].bIndexRegPresent = FALSE;
                            }
                            else
                            {
                                pInstruction->Operands[i].bIndexRegPresent = TRUE;
                                pInstruction->Operands[i].IndexReg = evREG_AL;
                            }

                            pInstruction->Operands[i].MemAccessType = tempoperand.MemAccessType;
                        }
                    }
                    else
                    {
                        int j = i - numimplicit;        // operand index in DE
                        pInstruction->Operands[i].OpSize = (eOperandSize)(m_pWrapper->GetOperandSize(j));

                        switch (m_pWrapper->GetOperandType(j))
                        {
                            case n_Disassembler::OPERANDTYPE_REGISTER:

                                switch (pInstruction->Operands[i].OpSize)
                                {
                                    case evSizeByte:
                                        pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(j)
                                                                                       - evREG_EAX + evREG_AL);
                                        break;

                                    case evSizeWord:
                                        pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(j)
                                                                                       - evREG_EAX + evREG_AX);
                                        break;

                                    case evSizeDWord:
                                        pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(j));
                                        break;

                                    case evSizeTByte:
                                        pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i));
                                        break;

                                    default:
                                        pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetRegister(i));
                                        break;
                                }

                                pInstruction->Operands[i].AddrMode = evRegister;
                                break;

                            case n_Disassembler::OPERANDTYPE_IMMEDIATE:
                                pInstruction->Operands[i].AddrMode = evImmediate;
                                break;

                            case n_Disassembler::OPERANDTYPE_RIPRELATIVE:
                                pInstruction->Operands[i].AddrMode = evPCRelative;
                                break;

                            case n_Disassembler::OPERANDTYPE_MEMORY:
                                pInstruction->Operands[i].AddrMode = evMemory;

                                if (m_pWrapper->HasBase())
                                {
                                    pInstruction->Operands[i].BaseReg = (eRegType)(m_pWrapper->GetBaseRegister());
                                    pInstruction->Operands[i].bBaseRegPresent = TRUE;
                                }
                                else
                                {
                                    pInstruction->Operands[i].bBaseRegPresent = FALSE;
                                }

                                if (m_pWrapper->OperandHasIndex(j))
                                {
                                    pInstruction->Operands[i].IndexReg = (eRegType)(m_pWrapper->GetOperandIndexRegister(j));
                                    pInstruction->Operands[i].bIndexRegPresent = TRUE;
                                }
                                else
                                {
                                    pInstruction->Operands[i].bIndexRegPresent = FALSE;
                                }

                                pInstruction->Operands[i].ScaleFactor = 1;

                                if (m_pWrapper->HasScale())
                                {
                                    pInstruction->Operands[i].ScaleFactor = 0x01 << m_pWrapper->GetSibScale();
                                }

                                pInstruction->Operands[i].MemAccessType = tempoperand.MemAccessType;
                                break;

                            default:
                                break;
                        } // end of switch
                    }
                }   // end of for
            }

            break;
        }   // end of while
    }

    if (*ErrorCode)
    {
        return E_FAIL;
    }
    else
    {
        return S_OK;
    }
}

HRESULT LibDisassembler::SADisassemble(
    BYTE* RawCodeBytes,
    SAInstInfoType* pDisasmInfo,
    unsigned int* StrLength,
    BYTE* ASCIICode,
    BYTE* ErrorCode)
{
    char* pDisasmString;

    memset(ASCIICode, 0, *StrLength);
    *ErrorCode = static_cast<BYTE>(-1);

    if (m_pWrapper)
    {
        pDisasmString = m_pWrapper->Disassemble(RawCodeBytes);

        *ErrorCode = NoError;
        pDisasmInfo->OpSize[0] = evSizeNone;
        pDisasmInfo->OpSize[1] = evSizeNone;
        pDisasmInfo->OpSize[2] = evSizeNone;
        pDisasmInfo->NumBytesUsed = 0x00;
        pDisasmInfo->NumOperands = 0x00;
        pDisasmInfo->InstSpecies = evNASpecies;
        memset(pDisasmInfo->Mnemonic, 0, MAX_MNEMONIC_LENGTH);

        if (pDisasmString != NULL)
        {
            // Get Operand size
            pDisasmInfo->NumOperands = (BYTE) m_pWrapper->GetNumOperands();

            for (int i = 0; i < m_pWrapper->GetNumOperands(); i++)
            {
                pDisasmInfo->OpSize[i] = (eOperandSize)(m_pWrapper->GetOperandSize(i));
            }

            pDisasmInfo->NumBytesUsed = (BYTE)m_pWrapper->GetLength();

            CInstr_ExtraCodes* extracodes = (CInstr_ExtraCodes*)(m_pWrapper->GetExtraInfoPtr());

            if (NULL == extracodes)
            {
                *ErrorCode = ErrInTableHookup;
            }
            else
            {
                pDisasmInfo->InstSpecies = extracodes->instr_table.InstSpecies;
                *ErrorCode = NoError;
                strcpy_s(pDisasmInfo->Mnemonic, MAX_MNEMONIC_LENGTH, extracodes->instr_table.Mnemonic);
            }

            strcpy_s((char*) ASCIICode, *StrLength, pDisasmString);
            *StrLength = static_cast<UINT>(strlen(pDisasmString));
            *ErrorCode = NoError;
        }
        else
        {
            *ErrorCode = ErrInDE;
        }
    }

    if (*ErrorCode)
    {
        return E_FAIL;
    }
    else
    {
        return S_OK;
    }
}

HRESULT LibDisassembler::SetDefaultSegSize(BOOL DefaultIs32Bits)
{
    m_dbit = DefaultIs32Bits;

    if (m_pWrapper)
    {

        if (m_dbit)
        {
            // if D bit is On, long mod bit should be off
            m_lbit = FALSE;
            m_pWrapper->ClearLongMode();

            m_pWrapper->SetDbit();
        }
        else
        {
            m_pWrapper->ClearDbit();
        }
    }

    return S_OK;
}
