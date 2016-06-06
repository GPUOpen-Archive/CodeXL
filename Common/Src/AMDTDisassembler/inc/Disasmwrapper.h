//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Disasmwrapper.h
/// \brief A wrapper interface to the Disassembler class.
///
//==================================================================================

#if !defined(_DISASMWRAPPER_H_INCLUDED_)
#define _DISASMWRAPPER_H_INCLUDED_

#include "DisassemblerDLLBuild.h"

#ifdef _WINDOWS

    #if _MSC_VER > 1000
        #pragma once
    #endif // _MSC_VER > 1000
    #include "Disassembler.h"
#else
    #include "AttDisassembler.h"
#endif

#include "Disasm.h"
#include <stdio.h>

//
// Operand fields
#ifdef _SP
    #undef _SP
    #undef _DI
#endif

typedef enum
{
    na = 0,
    A,         // Direct address
    C,         // Reg field of modR/M selects a control register
    D,         // Reg field of modR/M selects a debug register
    E,         // modR/M specifies operand
    F,         // Flags register (Doesn't appear in tables)
    G,         // Reg field of modR/M selects a general register
    I,         // Immediate data
    J,         // Relative offset
    M,         // modR/M refers only to memory (may be the same as MO?)
    O,         // No modR/M byte
    P,         // Reg field of modR/M selects packed qword MMX register
    // Used to be MM1, which meant that operand is an MMX register
    // specified by Reg field of MODR/M
    Q,         // ModR/M specifies either MMX reg or memory addres
    // Used to be MM2, which meant that operand is MMX reg or
    // memory as determined by MOD field of MODR/M
    R,         // mod field of modR/M may refer only to a general register
    S,         // reg field of modR/M selects a segment register
    T,         // reg field of modR/M selects a test register
    X,         // memory addressed by DS:SI (Doesn't appear in tables)
    Y,         // memory addressed by ES:DI (Doesn't appear in tables)
    AL, BL, CL, DL,   // width always 8 bits
    AH, BH, CH, DH,
    AX, BX, CX, DX,   // width always 16 bits
    SP, BP, SI, DI,
    SS, CS, DS, ES, FS, GS,
    eAX, eBX, eCX, eDX, // Width dependent on operand size
    eSP, eBP, eSI, eDI,
    EAX, ECX, EDX, EBX, // Width always 32 bits
    ESP, EBP, ESI, EDI,
    ST,               // Floating point register
    ST0, ST1, ST2, ST3, // ST0 = top of floating point stack
    ST4, ST5, ST6, ST7, // ST7 = bottom of floating point stack
    PFSC,      // For prefetch instruction only
    I1,         // operand is immediate one (1)

    // The following enum values starting with an underscore are used to
    // indicate implicit operands that should not be used in the disassembly,
    // but should be reported to Etch.
    _AL, _CL, _DL, _BL,
    _AH, _CH, _DH, _BH,
    _AX, _CX, _DX,
    _BX,
    _SP,
    _BP,
    _SI,
    _DI,
    _ES, _CS, _SS, _DS, _FS, _GS,
    _eAX, _eBX, _eCX, _eDX, // Width dependent on operand size
    _eSP, _eBP, _eSI, _eDI,
    _EAX, _ECX, _EDX, _EBX,
    _ESP, _EBP, _ESI, _EDI,
    _ST, _ST0, _ST1, _ST2, _ST3,
    _ST4, _ST5, _ST6, _ST7,
    _EBXAL               //for xlat implicitly accessing DS:[EBX+AL]
} eAddressingMethods ;


///////////////////////////////////////////////////////////////////////
//*********************************************************************
// The following are the tables holding the instructions and operands
//*********************************************************************

///////////////////////////////////////////////////////////////////////
// This class is used to specify an operand

class COperandField
{
public:
    eAddressingMethods   AddrMethod;
    eOperandTypes       OperType;
    eMemAccessType      MemAccessType ;
public :
    BOOL IsImplicit();
    eRegType GetOperandFieldReg();
    eOperandSize GetOperandFieldSize();

    const COperandField& operator = (const COperandField&);
};

//////////////////////////////////////////////////////////////////////
//This is instruction extra info data

class CInstr_Table
{
public:
    char            Mnemonic[MAX_MNEMONIC_LENGTH];
    BYTE            NumOperands ;
    COperandField   OpField1 ;
    COperandField   OpField2 ;
    COperandField   OpField3 ;
    eSpecies        InstSpecies ;
    eProcType       ProcessorType ;
    const CInstr_Table& operator = (const CInstr_Table&);
};

//////////////////////////////////////////////////////
//
//static CInstr_Table S_twoByteOpcodes_table[256];

#ifdef _WIN32
    #pragma warning (disable : 4275)
#endif

class DASM_API CInstr_ExtraCodes : public CExtraInstInfo
{
public:
    CInstr_Table instr_table;

    virtual void Print() { printf("test\n");};

    CInstr_ExtraCodes();
    CInstr_ExtraCodes(CInstr_Table in_table);
    const CInstr_ExtraCodes& operator = (const CInstr_Table&);
    //  eSpecies    GetSepcies() {return instr_table.InstSpecies;};

};


#ifdef _WINDOWS
class DASM_API CDisasmwrapper : public CDisassembler
#else
class DASM_API CDisasmwrapper : public CAttDisassembler
#endif
{
public:
    BOOL HasDisplacement() const {return m_displacementOffset != -1;}
    BOOL HasImmediateData() const {return m_immediateOffset != -1;}
    int GetLength();
    int GetNumOpcodeBytes();
    AMD_UINT8 GetOpcode(int index);
    AMD_UINT8 GetModrm();
    AMD_INT64 GetDisplacement();
    int GetNumOperands();
    n_Disassembler::e_OperandType GetOperandType(int operand);
    n_Disassembler::e_OperandSize GetOperandSize(int operand);
    n_Disassembler::e_Registers GetRegister(int operand);
    void SetDbit(bool dbit = true);
    void SetLongMode(bool longmode = true);

    CDisasmwrapper();
    virtual ~CDisasmwrapper();

    void AssignTableContent();
    void Hookuptables();

};

#endif // !defined(_DISASMWRAPPER_H_INCLUDED_)
