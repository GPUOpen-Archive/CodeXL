//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file LibDisassembler.h
/// \brief The disassembler library interface.
///
//==================================================================================

#ifndef _LIBDISASSEMBLER_H
#define _LIBDISASSEMBLER_H

#ifdef _WINDOWS
    #include <windows.h>
#else
    #include "typedefs_lin.h"
#endif

#include "Disassembler.h"
#include "Disasm.h"

class CDisasmwrapper;

/////////////////////////////////////////////////////////////////////////////
// K86Disasm
/////////////////////////////////////////////////////////////////////////////
// CK86Disasm
class DASM_API LibDisassembler
{
public:
    //Interface
    LibDisassembler();
    ~LibDisassembler();

    HRESULT SADisassemble(/*[in]*/ BYTE* RawCodeBytes,
                                   /*[out]*/ SAInstInfoType* pDisasmInfo,
                                   /*[in,out]*/ unsigned int* StrLength,
                                   /*[out, size_is(*StrLength)]*/ BYTE* ASCIICode,
                                   /*[out]*/ BYTE* ErrorCode);
    HRESULT EtchDisassemble(/*[in]*/ BYTE* RawCodeBytes,
                                     /*[out]*/ InstructionType* pInstruction, /*[out]*/ BYTE* ErrorCode);
    HRESULT TGDisassemble(/*[in]*/ BYTE* RawCodeBytes,
                                   /*[out]*/ TgInstInfoType* pDisasmInfo,
                                   /*[in,size_is(17)]*/ UINT64 ContextRegs[],
                                   /*[in,out]*/ unsigned int* pBuffSize,
                                   /*[out,size_is(*pBuffSize)]*/ BYTE TraceRecs[],
                                   /*[out]*/ UINT64* pEffectiveAddrArray,
                                   /*[in, out]*/ DWORD* pEffectiveAddrCount,
                                   /*[out]*/ BYTE* ErrorCode);
    HRESULT UIDisassemble(/*[in]*/ BYTE* rawcode,
                                   /*[in, out]*/ unsigned int* StrLength,
                                   /*[out, size_is(*StrLength)]*/ BYTE* ASCIICode,
                                   /*[out]*/ UIInstInfoType* pDisasmInfo, /*[out]*/ BYTE* ErrorCode);
    HRESULT SetProcessorType(/*[in]*/ eProcType CurrentProcType);
    HRESULT SetLongMode(/*[in]*/ BOOL DefaultIs32Bits);

    //Not used by CodeAnalyst
    HRESULT SetDefaultSegSize(/*[in]*/ BOOL DefaultIs32Bits);

private:
    void ResetEffectiveAddr();
    void WriteEffectiveAddr(UINT64* pEffAddr);
    CDisasmwrapper*  m_pWrapper;
    BOOL            m_dbit;
    BOOL            m_lbit;
    eProcType       m_proctype;

    BOOL            m_bEffAddrBaseRegPresent;
    BOOL            m_bEffAddrIndexRegPresent;
    BYTE            m_EffAddrBaseReg;
    BYTE            m_EffAddrIndexReg;
    BYTE            m_EffAddrScale;
    INT64           m_EffAddrDisp;
    UINT64          m_EffectiveAddress;
    DWORD           m_EffectiveAddrCount;
    UINT64*          m_pContextRegs;
    unsigned int    m_TraceBuffSize;
    BYTE*            m_pTraceRecords;
    eOperandSize    m_OperandSize;
    eMemAccessType  m_MemAccessType;
};

#endif //_LIBDISASSEMBLER_H
