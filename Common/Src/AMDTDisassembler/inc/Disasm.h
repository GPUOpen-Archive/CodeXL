//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Disasm.h
/// \brief This file contains structures and definitions for the disassembler.
///
//==================================================================================

#ifndef _DISASM_H_
#define _DISASM_H_

#ifdef _WINDOWS
    #ifndef UINT64
        typedef unsigned __int64 UINT64;
    #endif
#else
    #include "typedefs_lin.h"
#endif

typedef enum
{
    NA = 0,
    a,          // two 1-word operands in memory or
    // two double-word operands in memory
    b,          // byte (regardless of operand size attribute)
    ib,         // Immediate byte (regardless of operand size attribute)
    c,          // byte or word (depending on operand size attribute),
    d,          // double word (regardless of operand size attribute)
    dw,         // 32-bit register, or 16-bit memory operand
    p,          // 32 or 48 bit pointer (depending on operand size attribute)
    q,          // quad word regardless of operand size
    s,          // 6-byte pseudo-descriptor
    t,          // 10-byte data value
    v,          // word or double word (depending on operand size attribute)
    w           // word (regardless of operand size attribute)
} eOperandTypes ;



typedef enum
{
    nA = 0,         // No memory access for this operand
    RR = 0x30,      // Operand causes a "read" from memory
    WW = 0x40,      // Operand causes a "write" to memory
    RW = 0x70,      // Operand results into a Read/Modify/Write to memory
    ER = 0xF0,      // This is an invalid flag.  It only happens in the case
    // of CMPXCHG & CMPXCHG8B instructions, and is used to flag
    // the UI to handle the case differently..
} eMemAccessType ;


typedef enum
{
    evNA = 0,
    evImmediate,
    evRegister,
    evPCRelative,
    evMemory,
} eAddrMode ;


// These are the different processor types

typedef enum
{
    K6          = 0x00000001,   // K6 with MMX support but no 3D-Now!
    K6_2        = 0x00000002,   // K6 with 3D-Now! support
    K6_3        = 0x00000004,   // K6 with 3D-Now! support, and L2 cache
    K6_ST50     = 0x00000008,   // K6 with 3D-Now! and L2 cache (0.18 micron)
    K7          = 0x00000010,   // K7
    K7_2        = 0x00000020,   // K7 Mustang
    K8          = 0x00000040,   // Sledge Hammer (supports REX instruction)
    ANY     = ((K6) | (K6_2) | (K6_3) | (K6_ST50) | (K7) | (K7_2) | (K8)),
    ALL     = K6
} eProcType ;




//--------------------------------------------------------------------------------------
// eInstSpecies
// Instructions are broken down into basic instructions (but not by addressing mode).
// This "species" classification allows instructions to be grouped as the caller wishes,
// for instance, by 3DNow!, SSE, SSE-2, etc (the original intent of this list),
// or by add instructions, subtract instructions, etc.  It is the lowest classification
// above the individual instructions.
//
#include "SpeciesEnum.h"



///////////////////////////////////////////////////////////////////////
//
// enumerated types used both inside and outside of Disassembler
typedef enum
{
    evSizeByte,
    evSizeWord,
    evSizeDWord,
    evSizeFWord,
    evSizeQWord,
    evSizeTByte,
    evSizeDQWord,   //128 bits;
    evSizeNone
} eOperandSize ;

typedef enum
{
    evREG_EAX       = 0,
    evREG_ECX       = 1,
    evREG_EDX       = 2,
    evREG_EBX       = 3,
    evREG_ESP       = 4,
    evREG_EBP       = 5,
    evREG_ESI       = 6,
    evREG_EDI       = 7,

    evREG_R8        = 8,
    evREG_R9        = 9,
    evREG_R10       = 10,
    evREG_R11       = 11,
    evREG_R12       = 12,
    evREG_R13       = 13,
    evREG_R14       = 14,
    evREG_R15       = 15,

    evREG_ST0       = 16,
    evREG_ST1       = 17,
    evREG_ST2       = 18,
    evREG_ST3       = 19,
    evREG_ST4       = 20,
    evREG_ST5       = 21,
    evREG_ST6       = 22,
    evREG_ST7       = 23,

    evREG_MM0       = 24,
    evREG_MM1       = 25,
    evREG_MM2       = 26,
    evREG_MM3       = 27,
    evREG_MM4       = 28,
    evREG_MM5       = 29,
    evREG_MM6       = 30,
    evREG_MM7       = 31,

    evREG_XMM0      = 32,
    evREG_XMM1      = 33,
    evREG_XMM2      = 34,
    evREG_XMM3      = 35,
    evREG_XMM4      = 36,
    evREG_XMM5      = 37,
    evREG_XMM6      = 38,
    evREG_XMM7      = 39,
    evREG_XMM8      = 40,
    evREG_XMM9      = 41,
    evREG_XMM10     = 42,
    evREG_XMM11     = 43,
    evREG_XMM12     = 44,
    evREG_XMM13     = 45,
    evREG_XMM14     = 46,
    evREG_XMM15     = 47,

    evREG_CR0       = 48,
    evREG_CR1       = 49,
    evREG_CR2       = 50,
    evREG_CR3       = 51,
    evREG_CR4       = 52,
    evREG_CR5       = 53,
    evREG_CR6       = 54,
    evREG_CR7       = 55,
    evREG_CR8       = 56,
    evREG_CR9       = 57,
    evREG_CR10      = 58,
    evREG_CR11      = 59,
    evREG_CR12      = 60,
    evREG_CR13      = 61,
    evREG_CR14      = 62,
    evREG_CR15      = 63,

    evREG_DR0       = 64,
    evREG_DR1       = 65,
    evREG_DR2       = 66,
    evREG_DR3       = 67,
    evREG_DR4       = 68,
    evREG_DR5       = 69,
    evREG_DR6       = 70,
    evREG_DR7       = 71,
    evREG_DR8       = 72,
    evREG_DR9       = 73,
    evREG_DR10      = 74,
    evREG_DR11      = 75,
    evREG_DR12      = 76,
    evREG_DR13      = 77,
    evREG_DR14      = 78,
    evREG_DR15      = 79,

    // ordered by modrm reg values
    evREG_ES        = 80,
    evREG_CS        = 81,
    evREG_SS        = 82,
    evREG_DS        = 83,
    evREG_FS        = 84,
    evREG_GS        = 85,

    evREG_AL           ,
    evREG_CL           ,
    evREG_DL           ,
    evREG_BL           ,
    evREG_AH           ,
    evREG_CH           ,
    evREG_DH           ,
    evREG_BH           ,

    evREG_AX           ,
    evREG_CX           ,
    evREG_DX           ,
    evREG_BX           ,
    evREG_SP           ,
    evREG_BP           ,
    evREG_SI           ,
    evREG_DI           ,

    evREG_EFLAGS

} eRegType ;

typedef enum
{
    NoError = 0,
    ErrInvalidOpcode,
    ErrInvalidOperand,
    ErrWrongProcessor,
    ErrInDE,            // Error from Robert's disassembler engine
    ErrInTableHookup    // Error to hook up extra info table
} eErrorCodes ;

typedef enum
{
    evMODField,
    evRegField,
    evRMField,
    evFPOpField
} eMODRMByteFields ;

typedef enum
{
    evSSField,
    evIndexField,
    evBaseField
} eSIBByteFields ;


typedef enum
{
    evNoPrefix = 0,
    evREP,
    evREPE,
    evREPNE,
    evLOCK
} eInstrPrefixType ;


typedef struct
{
    BOOL                IsImplicit ;
    //  BOOL                bMemAccess ;
    eMemAccessType      MemAccessType ; // Access type "nA = 0" means operand doesn't acess memory
    //  eRegType            RegisterType ;
    eAddrMode           AddrMode ;
    BOOL                bBaseRegPresent ;
    eRegType            BaseReg ;
    BOOL                bIndexRegPresent ;
    eRegType            IndexReg ;
    BYTE                ScaleFactor ;
    eOperandSize        OpSize ;
} OperandType ;


typedef struct
{
    BYTE                NumBytesUsed ;
    BYTE                NumOperands ;
    BYTE                InstrPrefix ;           // This could be either any flavor of REP, or the LOCK prefix.
    eInstrPrefixType    InstrPfxType ;
    BYTE                SegmentPrefix ;     // if 0, no segment override prefix is present
    BOOL                bAddrSizePrefix ;       // if 0, no address size prefix is present
    BOOL                bOpSizePrefix ;         // if 0, no operand size prefix is present
    BOOL                bModRMPresent ;
    BYTE                ModRMValue ;
    BOOL                bSIBPresent ;
    BYTE                SIBValue ;
    BYTE                NumOpCodeBytes ;
    BYTE                OpCodeByte ;
    //  BYTE                NumBytesOfImmData;  // number of bytes of immediate data
    //  BYTE                NumBytesOfDispData; // number of bytes of displacement data
    BOOL                bHasImmData;
    BOOL                bHasDispData;
    BYTE                ImmDataOffset;      // Offset of immediate data value in the instruction
    BYTE                DispDataOffset;     // Offset of displacement data value in the instruction
    UINT64              ImmDataValue;       // actual immediate data value
    UINT64              ImmDataValue_2;     // This is used for very rare cases where the instruction has two immediate values, i.e. "enter"
    unsigned int        DispDataValue;      // actual displacement data value
    //  eInstClass          InstClass ;         // Type of the instruction
    //  eInstType           InstType ;          // Type of the instruction
    eSpecies            InstSpecies ;       // Instruction species (finer granularity than Class or Type).
    eProcType           InstProcType ;
    char                Mnemonic[MAX_MNEMONIC_LENGTH] ;
    OperandType         Operands[MAX_OPERANDS] ;
} InstructionType ;


typedef struct
{
    BYTE                NumBytesUsed ;
    BYTE                InstrPrefix ;   // This could be either any flavor of REP, or the LOCK prefix.
    eInstrPrefixType    InstrPfxType ;
    BYTE                NumOpCodeBytes ;
    BYTE                OpCodeByte ;
    //  eInstClass          InstClass ;             // Type of the instruction
    //  eInstType           InstType ;              // Type of the instruction
    eSpecies            InstSpecies;
} TgInstInfoType ;

typedef struct
{
    eOperandSize        OpSize ;
    BYTE                NumBytesUsed ;
    BYTE                InstrPrefix ;           // This could be either any flavor of REP, or the LOCK prefix.
    eInstrPrefixType    InstrPfxType ;
    BYTE                SegmentPrefix ;     // if 0, no segment override prefix is present
    BOOL                bAddrSizePrefix ;       // if 0, no address size prefix is present
    BOOL                bOpSizePrefix ;         // if 0, no operand size prefix is present
    BOOL                bModRMPresent ;
    BYTE                ModRMValue ;
    BOOL                bSIBPresent ;
    BYTE                SIBValue ;
    BYTE                NumOpCodeBytes ;
    BYTE                OpCodeByte ;
    //  BYTE                NumBytesOfImmData;  // number of bytes of immediate data
    //  BYTE                NumBytesOfDispData; // number of bytes of displacement data
    BOOL                bHasImmData;
    BOOL                bHasDispData;
    UINT64              ImmDataValue;           // actual immediate data value
    unsigned int        DispDataValue;          // actual displacement data value
} PipeInstInfoType ;

typedef struct
{
    BYTE                NumBytesUsed ;
    BYTE                NumOpCodeBytes ;
    BYTE                OpCodeByte ;
    BOOL                bHasDispData;
    unsigned int        DispDataValue;
    BOOL                bIsPCRelative;
    BOOL                bHasMemOp[MAX_OPERANDS];
    unsigned int        MemAccessSize[MAX_OPERANDS];
} UIInstInfoType ;

typedef struct
{
    eOperandSize        OpSize[MAX_OPERANDS];
    BYTE                NumOperands;
    BYTE                NumBytesUsed;
    eSpecies            InstSpecies;
    char                Mnemonic[MAX_MNEMONIC_LENGTH];
} SAInstInfoType;

#endif // _DISASM_H_
