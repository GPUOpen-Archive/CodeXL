//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Disassembler.h
/// \brief Base Intel Disassembler class.
///
//==================================================================================

#ifndef _DISASSEMBLER_H_
#define _DISASSEMBLER_H_

#define MAJOR_VERSION 3
#define MINOR_VERSION 10
#define BUILD_LEVEL 0

#include <stdlib.h>

/******************************************************************************/
// This is the header file for the CDisassembler class.  This class provides
// a generic x86 disassembler.  It currently supports 3dnow, MMX, SSE, SSE2,
// SVM, SSE3, SSSE3, SSE4, AES, AVX, XOP, FMA4, CVT16, TBM0/1/2, and FMA3.
//
// The interfaces of interest can be found in the public sections of the
// CInstructionData class and the derived CDisassembler class.
/******************************************************************************/

#if defined( WIN32 )                // Assume WINxx implies Visual C++
    #pragma warning( disable: 4514 )    // "unreferenced inline function has been removed"
    #pragma warning( disable: 4127 )    // "conditional expression is constant"
    #pragma warning( disable: 4710 )    // "... not inlined"
    #pragma warning( disable: 4996 )
    #undef REG_NONE
#elif defined( _WIN64 )
    #pragma warning( disable: 4514 )    // "unreferenced inline function has been removed"
    #pragma warning( disable: 4127 )    // "conditional expression is constant"
    #pragma warning( disable: 4710 )    // "... not inlined"
    #pragma warning( disable: 4996 )
    #undef REG_NONE
#else
    #include "typedefs_lin.h"
#endif

#include <vector>
#include <list>
#include "DisassemblerDLLBuild.h"
#include "Utils/typedefs.h"
#include "Utils/StringRef.h"

// ---------------------------------------------------------------------------
//  Defines
// ---------------------------------------------------------------------------

#define MAX_INSTRUCTION_BYTES 15
#define MAX_MNEMONIC_LENGTH 128
#define MAX_OPCODES 5
#define MAX_OPERANDS 5

#define NUM_CONTROL_REGISTERS 9

#define PREFIX_ES       0x26
#define PREFIX_CS       0x2e
#define PREFIX_SS       0x36
#define PREFIX_DS       0x3e
#define PREFIX_FS       0x64
#define PREFIX_GS       0x65
#define PREFIX_DATA     0x66
#define PREFIX_ADDR     0x67
#define PREFIX_XOP      0x8F
#define PREFIX_VEX3     0xC4
#define PREFIX_VEX2     0xC5
#define PREFIX_LOCK     0xf0
#define PREFIX_REPNE    0xf2
#define PREFIX_REP      0xf3

// Used in the inst_flags bit vector (32 bits)
#define INST_LONGOPCODE_POS     0
#define INST_DATA16_POS         1
#define INST_ADDR16_POS         2
#define INST_DATA32_POS         3
#define INST_ADDR32_POS         4
#define INST_DATA64_POS         5
#define INST_ADDR64_POS         6
#define INST_DATAOVRD_POS       7
#define INST_ADDROVRD_POS       8
#define INST_SEGOVRD_POS        9
#define INST_REP_POS            10
#define INST_REPNE_POS          11
#define INST_LOCK_POS           12
#define INST_AMD3D_POS          13
#define INST_AMD64INVALID_POS   14
#define INST_LOCK_OVERUSED_POS  15
#define INST_NO_V128_POS        16
#define INST_NO_V256_POS        17
#define INST_NO_VEXW_POS        18
#define INST_NO_VVVV_POS        19
#define INST_VEXW_SWAP_POS      20
#define INST_VEXL_UNDEF_POS     21
#define INST_VEXWL_OPSIZE_POS   22
#define INST_GATHER_POS         23

#define INST_NONE           (0)
#define INST_LONGOPCODE     (1<<INST_LONGOPCODE_POS)
#define INST_DATA16         (1<<INST_DATA16_POS)
#define INST_ADDR16         (1<<INST_ADDR16_POS)
#define INST_DATA32         (1<<INST_DATA32_POS)
#define INST_ADDR32         (1<<INST_ADDR32_POS)
#define INST_DATA64         (1<<INST_DATA64_POS)
#define INST_ADDR64         (1<<INST_ADDR64_POS)
#define INST_DATAOVRD       (1<<INST_DATAOVRD_POS)
#define INST_ADDROVRD       (1<<INST_ADDROVRD_POS)
#define INST_SEGOVRD        (1<<INST_SEGOVRD_POS)
#define INST_REP            (1<<INST_REP_POS)
#define INST_REPNE          (1<<INST_REPNE_POS)
#define INST_LOCK           (1<<INST_LOCK_POS)
#define INST_AMD3D          (1<<INST_AMD3D_POS)
#define INST_AMD64INVALID   (1<<INST_AMD64INVALID_POS)
#define INST_LOCK_OVERUSED  (1<<INST_LOCK_OVERUSED_POS)
#define INST_NO_V128        (1<<INST_NO_V128_POS)
#define INST_NO_V256        (1<<INST_NO_V256_POS)
#define INST_NO_VEXW        (1<<INST_NO_VEXW_POS)
#define INST_NO_VVVV        (1<<INST_NO_VVVV_POS)
#define INST_VEXW_SWAP      (1<<INST_VEXW_SWAP_POS)
#define INST_VEXL_UNDEF     (1<<INST_VEXL_UNDEF_POS)
#define INST_VEXWL_OPSIZE   (1<<INST_VEXWL_OPSIZE_POS)
#define INST_GATHER         (1<<INST_GATHER_POS)

// Used to index the size qualifier array (S_size_qualifiers)
#define SIZE_BYTE   0
#define SIZE_WORD   1
#define SIZE_DWORD  2
#define SIZE_48BIT  3
#define SIZE_QWORD  4
#define SIZE_TWORD  5
#define SIZE_DQWORD 6
#define SIZE_NONE   7

////////////////////////////////////////////////////////////////////////////////
// x86 Register definitions
////////////////////////////////////////////////////////////////////////////////

namespace n_Disassembler
{
enum e_Registers
{
    // this block ordered by modrm rm values (including REX extensions)
    REG_EAX     = 0,
    REG_ECX     = 1,
    REG_EDX     = 2,
    REG_EBX     = 3,
    REG_ESP     = 4,
    REG_EBP     = 5,
    REG_ESI     = 6,
    REG_EDI     = 7,
    REG_R8      = 8,
    REG_R9      = 9,
    REG_R10     = 10,
    REG_R11     = 11,
    REG_R12     = 12,
    REG_R13     = 13,
    REG_R14     = 14,
    REG_R15     = 15,
    REG_R16     = 16,
    REG_R17     = 17,
    REG_R18     = 18,
    REG_R19     = 19,
    REG_R20     = 20,
    REG_R21     = 21,
    REG_R22     = 22,
    REG_R23     = 23,
    REG_R24     = 24,
    REG_R25     = 25,
    REG_R26     = 26,
    REG_R27     = 27,
    REG_R28     = 28,
    REG_R29     = 29,
    REG_R30     = 30,
    REG_R31     = 31,

    REG_ST0     = 32,
    REG_ST1     = 33,
    REG_ST2     = 34,
    REG_ST3     = 35,
    REG_ST4     = 36,
    REG_ST5     = 37,
    REG_ST6     = 38,
    REG_ST7     = 39,
    REG_ST      = 40,

    REG_MM0     = 41,
    REG_MM1     = 42,
    REG_MM2     = 43,
    REG_MM3     = 44,
    REG_MM4     = 45,
    REG_MM5     = 46,
    REG_MM6     = 47,
    REG_MM7     = 48,

    REG_XMM0    = 49,
    REG_XMM1    = 50,
    REG_XMM2    = 51,
    REG_XMM3    = 52,
    REG_XMM4    = 53,
    REG_XMM5    = 54,
    REG_XMM6    = 55,
    REG_XMM7    = 56,
    REG_XMM8    = 57,
    REG_XMM9    = 58,
    REG_XMM10   = 59,
    REG_XMM11   = 60,
    REG_XMM12   = 61,
    REG_XMM13   = 62,
    REG_XMM14   = 63,
    REG_XMM15   = 64,
    REG_XMM16   = 65,
    REG_XMM17   = 66,
    REG_XMM18   = 67,
    REG_XMM19   = 68,
    REG_XMM20   = 69,
    REG_XMM21   = 70,
    REG_XMM22   = 71,
    REG_XMM23   = 72,
    REG_XMM24   = 73,
    REG_XMM25   = 74,
    REG_XMM26   = 75,
    REG_XMM27   = 76,
    REG_XMM28   = 77,
    REG_XMM29   = 78,
    REG_XMM30   = 79,
    REG_XMM31   = 80,

    REG_CR0     = 81,
    REG_CR1     = 82,
    REG_CR2     = 83,
    REG_CR3     = 84,
    REG_CR4     = 85,
    REG_CR5     = 86,
    REG_CR6     = 87,
    REG_CR7     = 88,
    REG_CR8     = 89,
    REG_CR9     = 90,
    REG_CR10    = 91,
    REG_CR11    = 92,
    REG_CR12    = 93,
    REG_CR13    = 94,
    REG_CR14    = 95,
    REG_CR15    = 96,

    REG_DR0     = 97,
    REG_DR1     = 98,
    REG_DR2     = 99,
    REG_DR3     = 100,
    REG_DR4     = 101,
    REG_DR5     = 102,
    REG_DR6     = 103,
    REG_DR7     = 104,
    REG_DR8     = 105,
    REG_DR9     = 106,
    REG_DR10    = 107,
    REG_DR11    = 108,
    REG_DR12    = 109,
    REG_DR13    = 110,
    REG_DR14    = 111,
    REG_DR15    = 112,

    // ordered by modrm reg values
    REG_ES      = 113,
    REG_CS      = 114,
    REG_SS      = 115,
    REG_DS      = 116,
    REG_FS      = 117,
    REG_GS      = 118,

    REG_YMM0    = 119,
    REG_YMM1    = 120,
    REG_YMM2    = 121,
    REG_YMM3    = 122,
    REG_YMM4    = 123,
    REG_YMM5    = 124,
    REG_YMM6    = 125,
    REG_YMM7    = 126,
    REG_YMM8    = 127,
    REG_YMM9    = 128,
    REG_YMM10   = 129,
    REG_YMM11   = 130,
    REG_YMM12   = 131,
    REG_YMM13   = 132,
    REG_YMM14   = 133,
    REG_YMM15   = 134,
    REG_YMM16   = 135,
    REG_YMM17   = 136,
    REG_YMM18   = 137,
    REG_YMM19   = 138,
    REG_YMM20   = 139,
    REG_YMM21   = 140,
    REG_YMM22   = 141,
    REG_YMM23   = 142,
    REG_YMM24   = 143,
    REG_YMM25   = 144,
    REG_YMM26   = 145,
    REG_YMM27   = 146,
    REG_YMM28   = 147,
    REG_YMM29   = 148,
    REG_YMM30   = 149,
    REG_YMM31   = 150,

    REG_VR0     = 151,
    REG_VR1     = 152,
    REG_VR2     = 153,
    REG_VR3     = 154,
    REG_VR4     = 155,
    REG_VR5     = 156,
    REG_VR6     = 157,
    REG_VR7     = 158,
    REG_VR8     = 159,
    REG_VR9     = 160,
    REG_VR10    = 161,
    REG_VR11    = 162,
    REG_VR12    = 163,
    REG_VR13    = 164,
    REG_VR14    = 165,
    REG_VR15    = 166,
    REG_VR16    = 167,
    REG_VR17    = 168,
    REG_VR18    = 169,
    REG_VR19    = 170,
    REG_VR20    = 171,
    REG_VR21    = 172,
    REG_VR22    = 173,
    REG_VR23    = 174,
    REG_VR24    = 175,
    REG_VR25    = 176,
    REG_VR26    = 177,
    REG_VR27    = 178,
    REG_VR28    = 179,
    REG_VR29    = 180,
    REG_VR30    = 181,
    REG_VR31    = 182,

    REG_NONE    = 183
};

enum e_SetType
{
    SET_NONE       = 0,
    SET_X86        = 1,
    SET_X87        = 2,
    SET_MMX        = 3,
    SET_3DNOW      = 4,
    SET_3DNOW_PREF = 5,
    SET_SSE        = 6,
    SET_SSE2       = 7,
    SET_SSE3       = 8,
    SET_CMPXCHG16B = 9,
    SET_SSSE3      = 10,
    SET_SSE4A      = 11,
    SET_SSE4_1     = 12,
    SET_SSE4_2     = 13,
    SET_PCLMULQDQ  = 14,
    SET_AES        = 15,
    SET_XSAVE      = 16,
    SET_AVX        = 17,
    SET_XOP        = 18,
    SET_FMA4       = 19,
    SET_CVT16      = 20,
    SET_SVM        = 21,
    SET_TBM0       = 22, // 'original' TBM instructions, AMD-only
    SET_TBM1       = 23, // 'converged' TBM instructions, BMI1 (AMD+Intel)
    SET_TBM2       = 24, // 'extra' TBM instructions, AMD-only
    SET_LWP        = 25,
    SET_FMA3       = 26,
    SET_XSAVEOPT   = 27,
    SET_FSGSBASE   = 28,
    SET_INVPCID    = 29,
    SET_BMI2       = 30,
    SET_AVX2       = 31,
    SET_MOVBE      = 32,
    SET_RDRAND     = 33,
    SET_MWAITX     = 34,
    SET_MWAIT      = 35,
};

enum e_OperandType
{
    OPERANDTYPE_REGISTER    = 0,
    OPERANDTYPE_IMMEDIATE   = 1,
    OPERANDTYPE_PCOFFSET    = 2,
    OPERANDTYPE_MEMORY      = 3,
    OPERANDTYPE_RIPRELATIVE = 4,
    OPERANDTYPE_CONDITION   = 5,
    OPERANDTYPE_NONE        = 6
};

enum e_OperandSize
{
    OPERANDSIZE_8       = 0,
    OPERANDSIZE_16      = 1,
    OPERANDSIZE_32      = 2,
    OPERANDSIZE_48      = 3,
    OPERANDSIZE_64      = 4,
    OPERANDSIZE_80      = 5,
    OPERANDSIZE_128     = 6,
    OPERANDSIZE_256     = 7,
    OPERANDSIZE_NONE    = 8
};

enum e_OperandSpecifier
{
    OPRND_na        = 0,
    OPRND_1         = 1,
    OPRND_AL        = 2,
    OPRND_AX        = 3,
    OPRND_eAX       = 4,
    OPRND_Ap        = 5,
    OPRND_CL        = 6,
    OPRND_Cd        = 7,
    OPRND_Dd        = 8,
    OPRND_DX        = 9,
    OPRND_eDX       = 10,
    OPRND_Eb        = 11,
    OPRND_Ew        = 12,
    OPRND_Ed        = 13,
    OPRND_Ev        = 14,
    OPRND_Ep        = 15,
    OPRND_Mt        = 16,
    OPRND_Gb        = 17,
    OPRND_Gw        = 18,
    OPRND_Gd        = 19,
    OPRND_Gq        = 20,
    OPRND_Gv        = 21,
    OPRND_Ib        = 22,
    OPRND_Iz        = 23,
    OPRND_Iw        = 24,
    OPRND_Jb        = 25,
    OPRND_Jz        = 26,
    OPRND_M         = 27,
    OPRND_Mp        = 28,
    OPRND_Mq        = 29,
    OPRND_Ms        = 30,
    OPRND_Ob        = 31,
    OPRND_Ov        = 32,
    OPRND_Pq        = 33,
    OPRND_Pd        = 34,
    OPRND_Qq        = 35,
    OPRND_Rd        = 36,
    OPRND_Sw        = 37,
    OPRND_Xb        = 38,
    OPRND_Xv        = 39,
    OPRND_Yb        = 40,
    OPRND_Yv        = 41,
    OPRND_breg      = 42,
    OPRND_vreg      = 43,
    OPRND_ST        = 44, // stack top
    OPRND_ST0       = 45,
    OPRND_ST1       = 46,
    OPRND_ST2       = 47,
    OPRND_ST3       = 48,
    OPRND_ST4       = 49,
    OPRND_ST5       = 50,
    OPRND_ST6       = 51,
    OPRND_ST7       = 52,
    OPRND_Vps       = 53, // SSE additions
    OPRND_Vq        = 54,
    OPRND_Vss       = 55,
    OPRND_Wps       = 56,
    OPRND_Wq        = 57,
    OPRND_Wss       = 58,
    OPRND_Vpd       = 59,   // SSE2 additions
    OPRND_Wpd       = 60,
    OPRND_Vsd       = 61,
    OPRND_Wsd       = 62,
    OPRND_Vx        = 63,
    OPRND_Wx        = 64,
    OPRND_Vd        = 65,
    OPRND_FPU_AX    = 66,
    OPRND_Mw        = 67,
    OPRND_Md        = 68,
    OPRND_Iv        = 69,   // added to support 64 bit immediate moves (B8-BF)
    OPRND_eBXAl     = 70,   // added to support XLAT
    OPRND_Ed_q      = 71,   // added to support REX extensions to MOVD, CVTSI2SS, CVTSI2SD
    OPRND_Pd_q      = 72,   // added to support REX extensions to MOVD, CVTSI2SS, CVTSI2SD
    OPRND_Vd_q      = 73,   // added to support REX extensions to MOVD, CVTSI2SS, CVTSI2SD
    OPRND_Gd_q      = 74,   // added to support REX extensions to CVTTSI2SS, CVTSS2SI, CVTTSD2SI, CVTSD2SI, MOVNTI
    OPRND_Md_q      = 75,   // added to support REX extensions to MOVNTI
    OPRND_MwRv      = 76,   // added to support SLDT

    OPRND_Mb        = 77,

    OPRND_Hss       = 78,   // BNI additions
    OPRND_Hsd       = 79,
    OPRND_Hps       = 80,
    OPRND_Hpd       = 81,

    OPRND_RdMb      = 82,
    OPRND_RdMw      = 83,
    OPRND_UxMq      = 84,
    OPRND_UxMd      = 85,
    OPRND_UxMw      = 86,

    OPRND_Gz        = 87,
    OPRND_Nq        = 88,
    OPRND_Uq        = 89,
    OPRND_Ups       = 90,
    OPRND_Upd       = 91,
    OPRND_Ux        = 92,

    // AVX additions
    OPRND_Hx        = 93,
    OPRND_Xx        = 94,
    OPRND_Xss       = 95,
    OPRND_Xsd       = 96,
    OPRND_Wqdq      = 97, // 64 or 128 bits based on VEX.L

    // XOP additions
    OPRND_Bv        = 98,

    // INVPCID addition
    OPRND_Mdq       = 99,

    // AVX2 additions
    OPRND_UxMb      = 100,
    OPRND_Mx        = 101,
    OPRND_Mv        = 102,
    OPRND_Rv        = 103,
    OPRND_UxM8      = 104,
    OPRND_UxM4      = 105,
    OPRND_UxM2      = 106
};

enum e_DisassemblerErrorCodes { NO_EXCEPTION, FORMAT_EXCEPTION, LENGTH_EXCEPTION, TABLE_EXCEPTION };

enum e_AlternateDecodings
{
    ALTERNATE_NONE = 0,
    ALTERNATE_LWP = 1
};

enum e_RegisterOrigins
{
    REGORIGIN_OPCODE = 0,
    REGORIGIN_MODRM_REG = 1,
    REGORIGIN_MODRM_RM = 2,
    REGORIGIN_VVVV = 3, // VEX.VVVV
    REGORIGIN_IS4 = 4,  // imm8[7:4]
    REGORIGIN_NONE = 5
};
}

#define NORMALIZE_GPREG(reg) ((AMD_UINT8)(reg))
#define NORMALIZE_FPUREG(reg) ((AMD_UINT8)(reg - n_Disassembler::REG_ST0))
#define NORMALIZE_MMXREG(reg) ((AMD_UINT8)(reg - n_Disassembler::REG_MM0))
#define NORMALIZE_XMMREG(reg) ((AMD_UINT8)(reg - n_Disassembler::REG_XMM0))
#define NORMALIZE_CONTROLREG(reg) ((AMD_UINT8)(reg - n_Disassembler::REG_CR0))
#define NORMALIZE_DEBUGREG(reg) ((AMD_UINT8)(reg - n_Disassembler::REG_DR0))
#define NORMALIZE_SEGREG(reg) ((AMD_UINT8)(reg - n_Disassembler::REG_ES))

////////////////////////////////////////////////////////////////////////////////

#undef RM
#undef MOD
#undef REG
#undef MEM

#define MOD(modrm) ((AMD_UINT8)(((modrm) >> 6) & 0x3))
#define REG(modrm) ((AMD_UINT8)(((modrm) >> 3) & 0x7))
#define RM(modrm)  ((AMD_UINT8)((modrm) & 0x7))

#define MEM(modrm) ((modrm) < 0xC0)

#define SS(sib)    MOD(sib)
#define IDX(sib)   REG(sib)
#define BASE(sib)  RM(sib)

#define OPCODE3_OPCODE(opcode) ((AMD_UINT8)((opcode) >> 3))
#define OPCODE3_OC1(opcode) ((AMD_UINT8)(((opcode) >> 2) & 1))
#define OPCODE3_OPS(opcode) ((AMD_UINT8)((opcode) & 3))
#define OPCODE3_OC1_OPS(opcode) ((AMD_UINT8)((opcode) & 7))

#define VEX_R(vex)      ((AMD_UINT8)(((vex) >> 7) & 1))
#define VEX_X(vex)      ((AMD_UINT8)(((vex) >> 6) & 1))
#define VEX_B(vex)      ((AMD_UINT8)(((vex) >> 5) & 1))
#define VEX_W(vex)      ((AMD_UINT8)(((vex) >> 7) & 1))
#define VEX_L(vex)      ((AMD_UINT8)(((vex) >> 2) & 1))
#define VEX_VVVV(vex)   ((AMD_UINT8)(((vex) >> 3)) & 0xF)
#define VEX_MMMMM(vex)  ((AMD_UINT8)(((vex) & 0x1F)))
#define VEX_PP(vex)     ((AMD_UINT8)(((vex) & 0x3)))

////////////////////////////////////////////////////////////////////////////////
// 64-bit support
////////////////////////////////////////////////////////////////////////////////

// w r x b
#define REX_OPERANDSIZE_POS             3
#define REX_MODRM_REG_POS               2
#define REX_SIBINDEX_POS                1
#define REX_MODRMRM_SIBBASE_OPREG_POS   0

#define REX_PREFIX(x) ((x >= 0x40) && (x <= 0x4f))

#define REX_OPERAND32(x)        ((x & (1 << REX_OPERANDSIZE_POS)) == 0)
#define REX_OPERAND64(x)        ((x & (1 << REX_OPERANDSIZE_POS)) != 0)
#define REX_MODRM_REG(x)        ((x & (1 << REX_MODRM_REG_POS)) != 0)
#define REX_MODRM_RM(x)         ((x & (1 << REX_MODRMRM_SIBBASE_OPREG_POS)) != 0)
#define REX_OPCODE_REG(x)       ((x & (1 << REX_MODRMRM_SIBBASE_OPREG_POS)) != 0)
#define REX_SIB_BASE(x)         ((x & (1 << REX_MODRMRM_SIBBASE_OPREG_POS)) != 0)
#define REX_SIB_INDEX(x)        ((x & (1 << REX_SIBINDEX_POS)) != 0)

////////////////////////////////////////////////////////////////////////////////
//  Classes
////////////////////////////////////////////////////////////////////////////////

// This is a small support class used by the disassembler to handle operand
// information during decode/disassembly

// used in the COperandInfo flags field and the Inst_Info operand flags fields
#define OPF_NONE            0
#define OPF_SHOWSIZE        (1<<0)
#define OPF_FARPTR          (1<<1)
#define OPF_SPECIAL64       (1<<2)
#define OPF_LOCK            (1<<3)
#define OPF_IMPLICIT        (1<<4)      // use this flag to hide an operand in the mnemonic
#define OPF_REGISTER        (1<<5)
#define OPF_BYTEHIGH        (1<<6)      // OPF_ defines from here down (i.e. >= 7) apply only to COperandInfo.flags
#define OPF_STRING          (1<<7)
#define OPF_UDHIGH          (1<<8)      // indicates operand additionally leaves high word in an undefined state (e.g. insertq)
#define OPF_VEXXMM          (1<<9)      // indicates VEX-encoded operand is always XMM, regardless of VEX.L
#define OPF_VSIB            (1<<10)     // this memory operand uses VSIB

// a few terse defines to shorten flag manipulations
#define OPF_SS      OPF_SHOWSIZE
#define OPF_64      OPF_SPECIAL64
#define OPF_SS_64   (OPF_SHOWSIZE | OPF_SPECIAL64)
#define OPF_SS_LOCK (OPF_SHOWSIZE | OPF_LOCK)
#define OPF_REG     OPF_REGISTER
#define OPF_I       OPF_IMPLICIT

#define EXPLICIT_OPERAND(x) (((x).flags & OPF_IMPLICIT) == 0)

class DASM_API COperandInfo
{
public:
    AMD_UINT8 reg;      // only valid if type == OPERANDTYPE_REGISTER, or if type == OPERANDTYPE_MEMORY and ((flags & OPF_STRING) != 0), or if type == OPERANDTYPE_CONDITION
    AMD_UINT32 flags;   // see OPF_ defines above for values
    n_Disassembler::e_OperandType type;
    n_Disassembler::e_OperandSize size;
    n_Disassembler::e_Registers regBlock;       // offset into the e_Registers enumeration
    n_Disassembler::e_RegisterOrigins regOrigin;

    COperandInfo() {};
    inline void Initialize(AMD_UINT32 _flags)
    {
        flags = _flags;
        reg = 0;
        type = n_Disassembler::OPERANDTYPE_NONE;
        size = n_Disassembler::OPERANDSIZE_NONE;
        regBlock = n_Disassembler::REG_NONE;
        regOrigin = n_Disassembler::REGORIGIN_NONE;
    }
    inline n_Disassembler::e_Registers GetRegister() { return ((n_Disassembler::e_Registers)(regBlock + reg)); }
    inline n_Disassembler::e_OperandSize GetSize() { return (size); }
    inline n_Disassembler::e_OperandType GetType() { return (type); }
    inline n_Disassembler::e_Registers GetRegisterBlock() { return regBlock; }
    inline n_Disassembler::e_RegisterOrigins GetRegisterOrigin() { return regOrigin; }
    inline bool IsHighByte() { return ((flags & OPF_BYTEHIGH) != 0); }
    inline bool IsString() { return ((flags & OPF_STRING) != 0); }
    inline bool IsUndefinedHighword() { return ((flags & OPF_UDHIGH) != 0); }
};

// CDisassembler derived classes wishing to extend CDisassembler's opcode
// tables should create arrays of objects derived from CExtraInstInfo and hook
// to CDisassembler's opcode table extension mechanism (i.e. GetExtraInfoPtr()).
// This requires initializing CDisassembler's opcode table entry "pExtraInfo" to
// point to the CDisassembler derived classes' arrays of CExtraInstInfo derived
// objects in the CDisassembler derived classes' constructor.
class CExtraInstInfo
{
public:
    virtual void Print() = 0;
    virtual ~CExtraInstInfo() {}
};

class DASM_API CDisassembler;

typedef void (CDisassembler::*PVOIDMEMBERFUNC)();

// This is the data type in the instruction opcode tables
class Inst_Info
{
public:
    const void* mnem;                   // either the mnemonic (char *) or a nested table ptr (Inst_Info *)

    // ptr to optional routine to access nested table
    // Some of the nested table access routines move bytes from the prefix
    // byte list to the opcode byte list to support overused prefix bytes
    // on a special case basis (categorized by nested table access routines).
    Inst_Info* (*GetInfoPtr)(CDisassembler* pThis);

    AMD_UINT32 instruction_flags;
    CExtraInstInfo* pExtraInfo;             // hook for derived classes to provide additional instruction info
    AMD_UINT16 set_type;
    AMD_UINT16 operands[MAX_OPERANDS];
    AMD_UINT16 operand_flags[MAX_OPERANDS];

    // This routine will report the number of entries in the Inst_Info table
    // pointed to by mnem when GetInfoPtr != NULL (i.e.  when there is
    // a nested table), zero when GetInfoPtr == NULL (i.e. when there is not a
    // nested table).  This can help to navigate nested tables.  Note that
    // when GetInfoPtr == NULL, mnem will not point to a nested table.
    int SizeofNestedTable();
};

// This is a small support class used by the disassembler to handle
// instruction prefix bytes.
class DASM_API CInstructionBytes
{
    AMD_UINT32 m_count;
    AMD_UINT8 m_bytes[MAX_INSTRUCTION_BYTES];

public:
    CInstructionBytes() { ClearBytes(); };
    inline void ClearBytes(void) { m_count = 0; }
    inline bool AddByte(AMD_UINT8 x_byte)
    {
        if (m_count < MAX_INSTRUCTION_BYTES) {  m_bytes[m_count++] = x_byte; return true;   }
        else { return false; }
    }
    inline bool GetLastByte(AMD_UINT8* x_byte)
    {
        if (m_count > 0) { *x_byte = m_bytes[m_count - 1]; return true; }
        else { return false; }
    }
    inline bool GetNextToLastByte(AMD_UINT8* x_byte)
    {
        if (m_count > 1) { *x_byte = m_bytes[m_count - 2]; return true; }
        else { return false; }
    }
    inline bool GetByte(AMD_UINT32 index, AMD_UINT8* x_byte)
    {
        if (index < m_count) { *x_byte = m_bytes[index]; return true; }
        else { return false; }
    }
    inline bool RemoveIndex(AMD_UINT32 index)
    {
        if (index < m_count)
        {
            for (AMD_UINT32 i = (index + 1); i < m_count; i++, index++) { m_bytes[index] = m_bytes[i]; }

            m_count--;
            return true;
        }
        else { return false; }
    }
    inline bool RemoveByte(AMD_UINT8 x_byte)
    {
        for (int i = (m_count - 1); i >= 0; i--)
            if (m_bytes[i] == x_byte)
            {
                for (int j = (i + 1); j < (int)m_count; j++, i++) { m_bytes[i] = m_bytes[j]; }

                m_count--;
                return true;
            }

        return false;
    }
    inline int FindByte(AMD_UINT8 x_byte)
    {
        for (int i = (m_count - 1); i >= 0; i--) if (m_bytes[i] == x_byte) { return i; }

        return -1;
    }
    inline bool IsLastByte(AMD_UINT8 x_byte) { return ((m_count > 0) && (m_bytes[m_count - 1] == x_byte)); }
    inline void RemoveLastByte(void) { if (m_count > 0) { m_count--; } }
    inline AMD_UINT32 GetCount() { return m_count; };
};

// Here it is!!!  The start of the useful public classes.
// CInstructionData is the parent class for CDisassembler and contains most of the
// instruction specific stuff.  CInstructionData contains most of the mechanisms
// for getting at the instruction pieces. CDisassembler contains most
// of the mechanisms for initializing CInstructionData from instruction bytes.
class DASM_API CInstructionData
{
protected:
    const AMD_UINT8* m_inst_buf;
    int m_len;
    AMD_UINT8 m_seg_reg;
    AMD_UINT8 m_modrm;
    AMD_UINT8 m_modrm_mod;
    AMD_UINT8 m_modrm_reg;
    AMD_UINT8 m_modrm_rm;
    AMD_UINT8 m_sib;
    AMD_UINT8 m_vex[3];
    AMD_UINT8 m_base;           // in longmode, this includes rex extensions (i.e. 4 bits instead of 3)
    AMD_UINT8 m_index;          // in longmode, this includes rex extensions (i.e. 4 bits instead of 3)
    AMD_UINT8 m_rex_prefix;
    AMD_UINT32 m_numOperands;
    AMD_UINT32 m_inst_flags;
    AMD_UINT64 m_immd;
    AMD_INT64 m_disp;           // m_disp doubles as relative offset because it is signed
    AMD_UINT64 m_rip;           // for rip-relative calculations
    bool m_bCalculateRipRelative;       // set this to generate rip-relative instead of $+ semantics
    bool m_dbit;
    bool m_longmode;
    bool m_svmmode;
    AMD_UINT64 m_extensions;    // indicates which isa extensions are supported
    bool m_bHasIndex;
    bool m_bHasBase;
    bool m_bModrmDecoded;
    MutableStringRef m_mnem;    // you can point this at your buffer instead of the builtin
    char m_mnem_buffer[MAX_MNEMONIC_LENGTH];
    CInstructionBytes m_prefix_bytes;
    COperandInfo m_operands[MAX_OPERANDS];
    int m_operand_indices[MAX_OPERANDS];        // internal support for bni operand reordering

    int m_numOpcodes;
    int m_opcodeOffsets[MAX_OPCODES];

    int m_modrmOffset;
    int m_sibOffset;
    int m_displacementOffset;
    int m_immediateOffset;

    int m_displacementLength;   // length in bytes
    int m_immediateLength;      // length in bytes

    void LogOpcodeOffset(int offset);

public:
    /**************************************************************************/
    // This class contains instruction data.  Use it to obtain data about an
    // instruction.
    /**************************************************************************/

    // These return true if the data size or address size is as queried
    inline bool IsData64() { return ((m_inst_flags & INST_DATA64) != 0); }
    inline bool IsAddr64() { return ((m_inst_flags & INST_ADDR64) != 0); }
    inline bool IsData32() { return ((m_inst_flags & INST_DATA32) != 0); }
    inline bool IsAddr32() { return ((m_inst_flags & INST_ADDR32) != 0); }
    inline bool IsData16() { return ((m_inst_flags & INST_DATA16) != 0); }
    inline bool IsAddr16() { return ((m_inst_flags & INST_ADDR16) != 0); }

    // Returns true if the instruction is a 3dnow instruction
    inline bool IsAmd3d() { return ((m_inst_flags & INST_AMD3D) != 0); }

    // Returns true if the instruction is a two byte escape instruction
    inline bool IsLongopcode() { return ((m_inst_flags & INST_LONGOPCODE) != 0); }

    // Returns true if the instruction has a MODRM byte
    inline bool HasModrm() { return (m_modrmOffset > -1); }

    // Returns true if the instruction has a SIB byte
    inline bool HasSib() { return (m_sibOffset > -1); }

    // Returns true if the instruction has a VEX prefix
    inline bool HasVex() { return (((m_vex[0] == PREFIX_VEX2) || (m_vex[0] == PREFIX_VEX3) || (m_vex[0] == PREFIX_XOP))); }

    // Returns true if the instruction has a VEX non-destructive operand
    inline bool HasVexNdOperand() { return (HasVex() && !(m_inst_flags & INST_NO_VVVV)); }

    // Returns true if the instruction has a SIB scale factor
    inline bool HasScale() { return (HasSib() && (SS(m_sib) != 0)); }

    // Returns true if the instruction has a modrm or SIB index register
    inline bool HasIndex() { return (m_bHasIndex); }

    // Returns true if the instruction has a modrm or SIB base register
    inline bool HasBase() { return (m_bHasBase); }

    // Returns true if the instruction has immediate data
    inline bool HasImmediate() { return (m_immediateOffset > -1); }

    // Returns true if the instruction has displacement data
    inline bool HasDisplacement() { return (m_displacementOffset > -1); }

    // Returns true if the instruction has at least one prefix byte
    inline bool HasPrefix() { return (m_prefix_bytes.GetCount() != 0); }

    // These return true if the instruction has the specific prefix byte queried
    // for, and the byte is used (i.e. in the case of conflicting prefixes, these
    // return true if the prefix was not overridden).
    inline bool HasSegOvrdPrefix() { return ((m_inst_flags & INST_SEGOVRD) != 0); }
    inline bool HasRepPrefix() { return ((m_inst_flags & INST_REP) != 0); }
    inline bool HasRepnePrefix() { return ((m_inst_flags & INST_REPNE) != 0); }
    inline bool HasLockPrefix() { return ((m_inst_flags & INST_LOCK) != 0); }
    inline bool HasRexPrefix() { return (REX_PREFIX(m_rex_prefix)); }
    inline bool HasDataOvrdPrefix() { return ((m_inst_flags & INST_DATAOVRD) != 0); }
    inline bool HasAddressOvrdPrefix() { return ((m_inst_flags & INST_ADDROVRD) != 0); }

    // These return the user settable mode status.
    inline bool IsLongMode() { return m_longmode; }
    inline bool IsSvmMode() { return m_svmmode; }

    // Use this to have the disassembler use your buffer for the mnemonic (instead
    // of CInstructionData's internal mnemonic buffer).  This will save you from having
    // to copy the mnemonic over.  Make sure the buffer you provide is long enough for
    // the longest mnemonic (MAX_MNEMONIC_LENGTH+1)
    inline MutableStringRef& SetMnemonicBuffer(char* pBuffer, int nSize) { m_mnem.reset(pBuffer, nSize - 1); return m_mnem; }
    inline void RestoreMnemonicBuffer() { m_mnem.reset(m_mnem_buffer, MAX_MNEMONIC_LENGTH - 1); }

    // This returns a pointer to the instruction mnemonic buffer
    inline char* GetMnemonic() { return (m_mnem.data()); }

    // Returns the overall length of the instruction in bytes
    inline int GetLength() { return (m_len); }

    // These routines return the portions of the instruction stream that are opcodes
    inline int GetNumOpcodeBytes() { return m_numOpcodes; }
    inline AMD_UINT8 GetOpcode(int index)
    {
        return (m_inst_buf[m_opcodeOffsets[(index >= 0) ? index : (m_numOpcodes + index)]]);
    }
    inline int GetOpcodeOffset(int index)
    {
        return (m_opcodeOffsets[(index >= 0) ? index : (m_numOpcodes + index)]);
    }

    // Returns the modrm byte (if applicable)
    inline AMD_UINT8 GetModrm() { return (m_modrm); }

    // Return pieces of the modrm byte
    inline AMD_UINT8 GetModrmMod() { return (m_modrm_mod); }
    inline AMD_UINT8 GetModrmReg() { return (m_modrm_reg); }
    inline AMD_UINT8 GetModrmRm() { return (m_modrm_rm); }

    // Returns the sib byte (if applicable)
    inline AMD_UINT8 GetSib() { return (m_sib); }

    // Return pieces of the sib byte
    inline AMD_UINT8 GetSibScale() { return (SS(m_sib)); }
    inline AMD_UINT8 GetSibIndex() { return (IDX(m_sib)); }
    inline AMD_UINT8 GetSibBase() { return (BASE(m_sib)); }

    // Returns a vex prefix byte (if applicable)
    inline AMD_UINT8 GetVex(int x_byte) { return (m_vex[x_byte]); }
    inline AMD_UINT8 GetVexR() { return (VEX_R(m_vex[1])); }
    inline AMD_UINT8 GetVexX() { return ((m_vex[0] != PREFIX_VEX2) ? VEX_X(m_vex[1]) : 1); }
    inline AMD_UINT8 GetVexB() { return ((m_vex[0] != PREFIX_VEX2) ? VEX_B(m_vex[1]) : 1); }
    inline AMD_UINT8 GetVexmmmmm() { return ((m_vex[0] != PREFIX_VEX2) ? VEX_MMMMM(m_vex[1]) : 1); }
    inline AMD_UINT8 GetVexW() { return ((m_vex[0] != PREFIX_VEX2) ? VEX_W(m_vex[2]) : 0); }
    inline AMD_UINT8 GetVexvvvv() { return (VEX_VVVV(m_vex[(m_vex[0] != PREFIX_VEX2) ? 2 : 1])); }
    inline AMD_UINT8 GetVexL() { return (VEX_L(m_vex[(m_vex[0] != PREFIX_VEX2) ? 2 : 1])); }
    inline AMD_UINT8 GetVexpp() { return (VEX_PP(m_vex[(m_vex[0] != PREFIX_VEX2) ? 2 : 1])); }

    // Returns the instruction immediate value (if applicable)
    // For instructions with more than one immediate, the immediates are concatenated
    // together in little endian order just like they appear in the instruction stream
    // e.g. "mnemonic eax,ib,iw" would produce an immediate = iwib (i.e. "ib iwlo iwhi")
    inline AMD_UINT64 GetImmediate() { return (m_immd); }

    // Returns the length (in bytes) of the immediate field
    inline AMD_UINT32 GetImmediateLength() { return m_immediateLength; }

    // Returns the instruction displacement value (if applicable)
    inline AMD_INT64 GetDisplacement() { return (m_disp); }

    // Returns the length (in bytes) of the displacement field
    inline AMD_UINT32 GetDisplacementLength() { return m_displacementLength; }

    // Returns the number of prefix bytes
    inline AMD_UINT32 GetPrefixCount() { return (m_prefix_bytes.GetCount()); }

    // Returns a specific prefix byte (if applicable)
    inline AMD_UINT8 GetPrefixByte(AMD_UINT32 index)
    {
        AMD_UINT8 prefixByte;
        return (m_prefix_bytes.GetByte(index, &prefixByte) ? prefixByte : (AMD_UINT8)0);
    }

    // Returns the segment register given that a segment override was used
    inline AMD_UINT8 GetSegmentRegister() { return (NORMALIZE_SEGREG(m_seg_reg)); }

    // Returns the rex prefix byte (will be zero if there is no rex prefix).
    inline AMD_UINT8 GetRexPrefix() { return (m_rex_prefix); }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Do not call the following routines with an operand index >= the number of operands as returned by GetNumOperands().
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // returns the number of explicit operands in the instruction.
    inline AMD_UINT32 GetNumOperands() { return m_numOperands; };

    // returns the type of the operand
    n_Disassembler::e_OperandType GetOperandType(AMD_UINT32 operand) { return (m_operands[operand].GetType()); }

    // returns the bit size of the operand
    inline n_Disassembler::e_OperandSize GetOperandSize(AMD_UINT32 operand) { return (m_operands[operand].GetSize()); }

    // returns the implicit bit size of the operand which includes 64-bit architecture zero extending
    inline n_Disassembler::e_OperandSize GetImplicitOperandSize(AMD_UINT32 operand)
    {
        if (m_operands[operand].IsUndefinedHighword())   // for now, only applies to 'insertq' which leaves highword undefined
        {
            return n_Disassembler::OPERANDSIZE_128;
        }

        n_Disassembler::e_OperandSize opsize = m_operands[operand].GetSize();

        if (m_longmode)
            if ((m_operands[operand].regBlock == n_Disassembler::REG_EAX) && (opsize == n_Disassembler::OPERANDSIZE_32))
            {
                return n_Disassembler::OPERANDSIZE_64;
            }

        return opsize;
    }

    // For register operands, returns the block of registers an operand references (e.g. GP (REG_EAX), MMX (REG_MM0), ...)
    inline n_Disassembler::e_Registers GetOperandRegBlock(AMD_UINT32 operand) { return (m_operands[operand].GetRegisterBlock()); }

    // For register operands, returns the origin of a register index (e.g. REGORIGIN_MODRM_REG, ...)
    inline n_Disassembler::e_RegisterOrigins GetOperandRegOrigin(AMD_UINT32 operand) { return (m_operands[operand].GetRegisterOrigin()); }

    // NOTE: For string instructions (movs, cmps, stos, ...), use OperandHasIndex() and
    // GetOperandIndexRegister(operand) to get the index register (esi or edi) for the appropriate operand.  HasIndex()
    // and GetIndexRegister only apply to instructions with a modrm and/or SIB.
    //
    // returns the e_Registers equivalent values
    // Do not call this routine with an operand index >= the number of operands as returned by GetNumOperands().
    inline n_Disassembler::e_Registers GetRegister(AMD_UINT32 operand) { return (m_operands[operand].GetRegister()); }
    inline n_Disassembler::e_Registers GetBaseRegister() { return (n_Disassembler::e_Registers)m_base; }
    inline n_Disassembler::e_Registers GetIndexRegister() { return (n_Disassembler::e_Registers)m_index; }

    // returns true if the indicated operand has an index register associated with it
    // Do not call this routine with an operand index >= the number of operands as returned by GetNumOperands().
    bool OperandHasIndex(AMD_UINT32 operand)
    {
        if (HasIndex())
        {
            return (GetOperandType(operand) == n_Disassembler::OPERANDTYPE_MEMORY);
        }
        else
        {
            return m_operands[operand].IsString();
        }
    }

    // returns the index register for the indicated operand
    // don't call this before calling OperandHasIndex(operand) and getting back "true"
    n_Disassembler::e_Registers GetOperandIndexRegister(AMD_UINT32 operand)
    {
        return (HasIndex() ? GetIndexRegister() : GetRegister(operand));
    }

    // returns true if the indicated operand is a far ptr (selector:offset)
    inline bool OperandIsFarPtr(AMD_UINT32 operand) { return ((m_operands[operand].flags & OPF_FARPTR) != 0); }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // End of operand specific routines
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // The following return the offset to each part of the instruction (where present)
    inline int GetModrmOffset() { return m_modrmOffset; }
    inline int GetSIBOffset() { return m_sibOffset; }
    inline int GetDisplacementOffset() { return m_displacementOffset; }
    inline int GetImmediateOffset() { return m_immediateOffset; }

#ifdef _DEBUG
    /**************************************************************************/
    // Debug only routines
    /**************************************************************************/
    void ShowOperands();
    void ShowOpcodes();
    const char* OperandSizeString(n_Disassembler::e_OperandSize size);
    const char* OperandTypeString(n_Disassembler::e_OperandType type);
    const char* RegisterString(n_Disassembler::e_Registers reg);
    /**************************************************************************/
#endif
};

class DASM_API CDisassembler : public CInstructionData
{
protected:
    AMD_UINT64 m_immediateData;     // internal: to support mnemonic generation
    Inst_Info* m_opcode_table_ptr;
    COperandInfo* m_pOperand;
    int m_maxInstructionBytes;

    StringRef m_hexPostfix;
    StringRef m_opcodeSeperator;
    bool m_bUpperCase;
    bool m_bShowSize;
    n_Disassembler::e_DisassemblerErrorCodes m_errorLevel;
    AMD_UINT32 m_alternateDecodings;    // by default, all alternate interpretations are disabled

    static const StringRef S_PrefixByteStrings[];
    void GetPrefixBytesString();

    static StringRef S_modrm16_str[];
    static StringRef S_rex_byte_regs[];
    static StringRef S_byte_regs[];
    static StringRef S_word_regs[];
    static StringRef S_dword_regs[];
    static StringRef S_qword_regs[];
    static StringRef S_control_regs[];
    static StringRef S_debug_regs[];
    static StringRef S_segment_regs[];
    static StringRef S_fpu_regs[];  // "st0", "st1", ..., "st"  (stack top is last)
    static StringRef S_mmx_regs[];
    static StringRef S_xmmx_regs[];
    static StringRef S_ymmx_regs[];
    static StringRef S_vr_regs[];

    static Inst_Info* S_3dnow_ptrs[256];

    static PVOIDMEMBERFUNC S_DecodeOperandFnPtrs[];
    static PVOIDMEMBERFUNC S_DisassembleOperandFnPtrs[];
    static PVOIDMEMBERFUNC S_PrefixByteFnPtrs[];

    AMD_UINT8 GetByte();
    AMD_UINT8 PeekByte(int offset = 0);

    int MnemBytesLeft();

    inline void GetByteModrm();
    inline void GetWordModrm();
    inline void GetDwordModrm();
    inline void GetWDQModrm();
    inline void GetWDQModrmMem();
    inline void GetWDQModrmReg();
    inline void GetWQModrm();
    inline void GetDQModrm();
    inline void GetOwordModrm();
    inline void GetByteModrmSS();
    inline void GetWordModrmSS();
    inline void GetDwordModrmSS();
    inline void GetWDQModrmSS();
    inline void GetWQModrmSS();
    inline void GetRegFromReg();
    inline void GetByteRegFromReg();
    inline void GetWordRegFromReg();
    inline void GetDwordRegFromReg();
    inline void GetQwordRegFromReg();
    inline void GetWDQRegFromReg();
    inline void GetWDRegFromReg();
    inline void GetDQRegFromReg();
    inline void GetByteImmediate();
    inline void GetByteImmediateSS();
    inline void GetWordImmediate();
    inline void GetWordOrDwordImmediate();
    inline void GetWordOrDwordImmediateSS();
    inline void GetWDQImmediate();
    inline void GetByteJump();
    inline void GetWordOrDwordJump();
    inline void GetByteRegFromOpcode();
    inline void GetWDQRegFromOpcode();
    inline void GetRegAL();
    inline void GetRegAX();
    inline void GetRegeAX();
    inline void GetRegCL();
    inline void GetRegDX();
    inline void GetRegeDX();
    inline void GetMemoryModrm();
    inline void GetDwordOrFwordMemory();
    inline void GetDwordOrFwordDirect();
    inline void GetOffsetByte();
    inline void GetOffsetWDQ();
    inline void GetMMXReg();
    inline void GetMMXDwordReg();
    inline void GetMMXQwordReg();
    inline void GetMMXDQwordReg();
    inline void GetMMXQwordModrm();
    inline void GetMMXQwordModrmRegister();
    inline void GetDwordOrQwordReg();
    inline void GetDebugRegister();
    inline void GetControlRegister();
    inline void GetWordSegmentRegister();
    inline void GetByteMemoryEsi();
    inline void GetWDQMemoryEsi();
    inline void GetByteMemoryEdi();
    inline void GetWDQMemoryEdi();
    inline void GetSTReg();
    inline void GetST0Reg();
    inline void GetST1Reg();
    inline void GetST2Reg();
    inline void GetST3Reg();
    inline void GetST4Reg();
    inline void GetST5Reg();
    inline void GetST6Reg();
    inline void GetST7Reg();
    inline void GetSimdOwordReg();
    inline void GetSimdDwordReg();
    inline void GetSimdQwordReg();
    inline void GetSimdDQwordReg();
    inline void GetSimdReg();
    inline void GetSimdDwordModrm();
    inline void GetSimdQwordModrm();
    inline void GetSimdQwordOrOwordModrm();
    inline void GetSimdQwordModrmRegister();
    inline void GetSimdOwordModrm();
    inline void GetSimdOwordModrmRegister();
    inline void GetSimdOwordModrmMem();
    inline void GetSimdModrm();
    inline void GetMemory();
    inline void GetByteMemory();
    inline void GetWordMemory();
    inline void GetDwordMemory();
    inline void GetWordOrDwordMemory();
    inline void GetDwordOrQwordMemory();
    inline void GetFwordMemory();
    inline void GetQwordMemory();
    inline void GetTwordMemory();
    inline void GetOwordMemory();
    inline void GetFpuAx();                     // Same as GetAX except it gets a modrm byte as well
    inline void GeteBXAndAL();
    inline void GetWordMemoryOrWDQRegModrm();
    inline void GetVexOpcodeOperand();
    inline void GetVexOwordOpcode();
    inline void GetDwordRegOrByteMemoryModrm();
    inline void GetDwordRegOrWordMemoryModrm();
    inline void GetSimdOwordRegOrQwordMemoryModrm();
    inline void GetSimdOwordRegOrDwordMemoryModrm();
    inline void GetSimdOwordRegOrWordMemoryModrm();
    inline void GetSimdOwordRegOrByteMemoryModrm();
    inline void GetSimdOwordRegOrEigthMemoryModrm();
    inline void GetSimdOwordRegOrQuarterMemoryModrm();
    inline void GetSimdOwordRegOrHalfMemoryModrm();
    inline void GetSimdOwordIs4();
    inline void GetVexWDQ();

    void ModrmStr();
    inline void MemoryModrmStr();
    inline void MemoryModrmStrWithSIB();
    inline void MemoryModrmStrWithoutSIB();
    void RegisterModrmStr();
    inline void ByteRegStr();
    inline void WordRegStr();
    inline void DwordRegStr();
    inline void QwordRegStr();
    inline void WDQRegStr();
    virtual void ByteImmediateStr();
    virtual void WordImmediateStr();
    virtual void WordOrDwordImmediateStr();
    virtual void WDQImmediateStr();
    inline void SignedByteJumpStr();
    inline void SignedWordOrDwordJumpStr();
    inline void _1Str();
    inline void RegALStr();
    inline void RegAXStr();
    inline void RegeAXStr();
    inline void RegCLStr();
    inline void RegDXStr();
    inline void RegeDXStr();
    inline void AddressStr();
    inline void OffsetStr();
    void MMXRegStr();
    inline void MMXModrmStr();
    inline void RegisterStr();
    inline void ControlRegStr();
    inline void DebugRegStr();
    inline void SegmentRegStr();
    inline void DsEsiStr();
    inline void EsEdiStr();
    inline void FpuStr();
    inline void SimdModrmStr();
    inline void SimdRegStr();
    inline void RegeBXAndALStr();

    int GetVexTblIndex();
    const StringRef& GetXymmStr(COperandInfo* op);
    n_Disassembler::e_Registers GetXymmBlock();
    n_Disassembler::e_OperandSize GetXymmOperandSize();

    void PrefixRex();
    void PrefixEsSegOveride();
    void PrefixCsSegOveride();
    void PrefixSsSegOveride();
    void PrefixDsSegOveride();
    void PrefixFsSegOveride();
    void PrefixGsSegOveride();
    void PrefixDataSize();
    void PrefixAddressSize();
    void PrefixLock();
    void PrefixRepne();
    void PrefixRep();

protected:
    static Inst_Info S_oneByteOpcodes_tbl[256];
    static Inst_Info S_twoByteOpcodes_tbl[256];

    // group tables for single byte opcodes
    static Inst_Info S_group_1_60_tbl[3];
    static Inst_Info S_group_1_61_tbl[3];
    static Inst_Info S_group_1_63_tbl[2];
    static Inst_Info S_group_1_6d_tbl[2];
    static Inst_Info S_group_1_6f_tbl[2];
    static Inst_Info S_group_1_80_tbl[8];
    static Inst_Info S_group_1_81_tbl[8];
    static Inst_Info S_group_1_82_tbl[8];
    static Inst_Info S_group_1_83_tbl[8];
    static Inst_Info S_group_1_8e_tbl[8];
    static Inst_Info S_group_1_8f_tbl[8];
    static Inst_Info S_group_1_90_tbl[3];   // nop, xchg, and pause
    static Inst_Info S_group_1_98_tbl[3];
    static Inst_Info S_group_1_99_tbl[3];
    static Inst_Info S_group_1_9c_tbl[3];
    static Inst_Info S_group_1_9d_tbl[3];
    static Inst_Info S_group_1_a5_tbl[3];
    static Inst_Info S_group_1_a7_tbl[3];
    static Inst_Info S_group_1_ab_tbl[3];
    static Inst_Info S_group_1_ad_tbl[3];
    static Inst_Info S_group_1_af_tbl[3];
    static Inst_Info S_group_1_c0_tbl[8];
    static Inst_Info S_group_1_c1_tbl[8];
    static Inst_Info S_group_1_c2_tbl[3];
    static Inst_Info S_group_1_c3_tbl[3];
    static Inst_Info S_group_1_c6_tbl[8];
    static Inst_Info S_group_1_c7_tbl[8];
    static Inst_Info S_group_1_ca_tbl[3];
    static Inst_Info S_group_1_cb_tbl[3];
    static Inst_Info S_group_1_cf_tbl[3];
    static Inst_Info S_group_1_d0_tbl[8];
    static Inst_Info S_group_1_d1_tbl[8];
    static Inst_Info S_group_1_d2_tbl[8];
    static Inst_Info S_group_1_d3_tbl[8];
    static Inst_Info S_group_1_d8_tbl[72];
    static Inst_Info S_group_1_d9_tbl[72];
    static Inst_Info S_group_1_da_tbl[72];
    static Inst_Info S_group_1_db_tbl[72];
    static Inst_Info S_group_1_dc_tbl[72];
    static Inst_Info S_group_1_dd_tbl[72];
    static Inst_Info S_group_1_de_tbl[72];
    static Inst_Info S_group_1_df_tbl[72];
    static Inst_Info S_group_1_e3_tbl[3];
    static Inst_Info S_group_1_f6_tbl[8];
    static Inst_Info S_group_1_f7_tbl[8];
    static Inst_Info S_group_1_fe_tbl[8];
    static Inst_Info S_group_1_ff_tbl[8];

    // group tables for multi byte opcodes
    static Inst_Info S_group_2_00_tbl[8];
    static Inst_Info S_group_2_01_tbl[8];
    static Inst_Info S_group_2_01_01_tbl[9];
    static Inst_Info S_group_2_01_01_00_tbl[4];
    static Inst_Info S_group_2_01_01_01_tbl[4];
    static Inst_Info S_group_2_01_02_tbl[9];
    static Inst_Info S_group_2_01_02_000_tbl[4];
    static Inst_Info S_group_2_01_02_001_tbl[4];
    static Inst_Info S_group_2_01_03_tbl[9];
    static Inst_Info S_group_2_01_07_tbl[9];
    static Inst_Info S_group_2_01_07_m3rm0_tbl[2];
    static Inst_Info S_group_2_01_07_02_tbl[4];
    static Inst_Info S_group_2_01_07_03_tbl[4];
    static Inst_Info S_group_2_0d_tbl[2];
    static Inst_Info S_group_2_0f_tbl[24];
    static Inst_Info S_group_2_10_tbl[4];
    static Inst_Info S_group_2_11_tbl[4];
    static Inst_Info S_group_2_12_tbl[4];
    static Inst_Info S_group_2_12_00_tbl[2];
    static Inst_Info S_group_2_13_tbl[4];
    static Inst_Info S_group_2_14_tbl[4];
    static Inst_Info S_group_2_15_tbl[4];
    static Inst_Info S_group_2_16_tbl[4];
    static Inst_Info S_group_2_16_00_tbl[2];
    static Inst_Info S_group_2_17_tbl[4];
    static Inst_Info S_group_2_18_tbl[16];

    static Inst_Info S_group_2_28_tbl[4];
    static Inst_Info S_group_2_29_tbl[4];
    static Inst_Info S_group_2_2a_tbl[4];
    static Inst_Info S_group_2_2b_tbl[4];
    static Inst_Info S_group_2_2c_tbl[4];
    static Inst_Info S_group_2_2d_tbl[4];
    static Inst_Info S_group_2_2e_tbl[4];
    static Inst_Info S_group_2_2f_tbl[4];
    static Inst_Info S_group_2_38_tbl[256];
    static Inst_Info S_group_2_38_00_tbl[2];
    static Inst_Info S_group_2_38_01_tbl[2];
    static Inst_Info S_group_2_38_02_tbl[2];
    static Inst_Info S_group_2_38_03_tbl[2];
    static Inst_Info S_group_2_38_04_tbl[2];
    static Inst_Info S_group_2_38_05_tbl[2];
    static Inst_Info S_group_2_38_06_tbl[2];
    static Inst_Info S_group_2_38_07_tbl[2];
    static Inst_Info S_group_2_38_08_tbl[2];
    static Inst_Info S_group_2_38_09_tbl[2];
    static Inst_Info S_group_2_38_0a_tbl[2];
    static Inst_Info S_group_2_38_0b_tbl[2];
    static Inst_Info S_group_2_38_10_tbl[2];
    static Inst_Info S_group_2_38_14_tbl[2];
    static Inst_Info S_group_2_38_15_tbl[2];
    static Inst_Info S_group_2_38_17_tbl[2];
    static Inst_Info S_group_2_38_1c_tbl[2];
    static Inst_Info S_group_2_38_1d_tbl[2];
    static Inst_Info S_group_2_38_1e_tbl[2];
    static Inst_Info S_group_2_38_20_tbl[2];
    static Inst_Info S_group_2_38_21_tbl[2];
    static Inst_Info S_group_2_38_22_tbl[2];
    static Inst_Info S_group_2_38_23_tbl[2];
    static Inst_Info S_group_2_38_24_tbl[2];
    static Inst_Info S_group_2_38_25_tbl[2];
    static Inst_Info S_group_2_38_28_tbl[2];
    static Inst_Info S_group_2_38_29_tbl[2];
    static Inst_Info S_group_2_38_2a_tbl[2];
    static Inst_Info S_group_2_38_2b_tbl[2];
    static Inst_Info S_group_2_38_30_tbl[2];
    static Inst_Info S_group_2_38_31_tbl[2];
    static Inst_Info S_group_2_38_32_tbl[2];
    static Inst_Info S_group_2_38_33_tbl[2];
    static Inst_Info S_group_2_38_34_tbl[2];
    static Inst_Info S_group_2_38_35_tbl[2];
    static Inst_Info S_group_2_38_37_tbl[2];
    static Inst_Info S_group_2_38_38_tbl[2];
    static Inst_Info S_group_2_38_39_tbl[2];
    static Inst_Info S_group_2_38_3a_tbl[2];
    static Inst_Info S_group_2_38_3b_tbl[2];
    static Inst_Info S_group_2_38_3c_tbl[2];
    static Inst_Info S_group_2_38_3d_tbl[2];
    static Inst_Info S_group_2_38_3e_tbl[2];
    static Inst_Info S_group_2_38_3f_tbl[2];
    static Inst_Info S_group_2_38_40_tbl[2];
    static Inst_Info S_group_2_38_41_tbl[2];
    static Inst_Info S_group_2_38_82_tbl[2];
    static Inst_Info S_group_2_38_db_tbl[2];
    static Inst_Info S_group_2_38_dc_tbl[2];
    static Inst_Info S_group_2_38_dd_tbl[2];
    static Inst_Info S_group_2_38_de_tbl[2];
    static Inst_Info S_group_2_38_df_tbl[2];
    static Inst_Info S_group_2_38_f0_tbl[3];
    static Inst_Info S_group_2_38_f1_tbl[3];
    static Inst_Info S_group_2_3a_tbl[256];
    static Inst_Info S_group_2_3a_08_tbl[2];
    static Inst_Info S_group_2_3a_09_tbl[2];
    static Inst_Info S_group_2_3a_0a_tbl[2];
    static Inst_Info S_group_2_3a_0b_tbl[2];
    static Inst_Info S_group_2_3a_0c_tbl[2];
    static Inst_Info S_group_2_3a_0d_tbl[2];
    static Inst_Info S_group_2_3a_0e_tbl[2];
    static Inst_Info S_group_2_3a_0f_tbl[2];
    static Inst_Info S_group_2_3a_14_tbl[2];
    static Inst_Info S_group_2_3a_15_tbl[2];
    static Inst_Info S_group_2_3a_16_tbl[2];
    static Inst_Info S_group_2_3a_16_00_tbl[2];
    static Inst_Info S_group_2_3a_17_tbl[2];
    static Inst_Info S_group_2_3a_20_tbl[2];
    static Inst_Info S_group_2_3a_21_tbl[2];
    static Inst_Info S_group_2_3a_22_tbl[2];
    static Inst_Info S_group_2_3a_22_00_tbl[2];
    static Inst_Info S_group_2_3a_40_tbl[2];
    static Inst_Info S_group_2_3a_41_tbl[2];
    static Inst_Info S_group_2_3a_42_tbl[2];
    static Inst_Info S_group_2_3a_44_tbl[2];
    static Inst_Info S_group_2_3a_60_tbl[2];
    static Inst_Info S_group_2_3a_61_tbl[2];
    static Inst_Info S_group_2_3a_62_tbl[2];
    static Inst_Info S_group_2_3a_63_tbl[2];
    static Inst_Info S_group_2_3a_df_tbl[2];
    static Inst_Info S_group_2_50_tbl[4];
    static Inst_Info S_group_2_51_tbl[4];
    static Inst_Info S_group_2_52_tbl[4];
    static Inst_Info S_group_2_53_tbl[4];
    static Inst_Info S_group_2_54_tbl[4];
    static Inst_Info S_group_2_55_tbl[4];
    static Inst_Info S_group_2_56_tbl[4];
    static Inst_Info S_group_2_57_tbl[4];
    static Inst_Info S_group_2_58_tbl[4];
    static Inst_Info S_group_2_59_tbl[4];
    static Inst_Info S_group_2_5a_tbl[4];
    static Inst_Info S_group_2_5b_tbl[4];
    static Inst_Info S_group_2_5c_tbl[4];
    static Inst_Info S_group_2_5d_tbl[4];
    static Inst_Info S_group_2_5e_tbl[4];
    static Inst_Info S_group_2_5f_tbl[4];
    static Inst_Info S_group_2_60_tbl[4];
    static Inst_Info S_group_2_61_tbl[4];
    static Inst_Info S_group_2_62_tbl[4];
    static Inst_Info S_group_2_63_tbl[4];
    static Inst_Info S_group_2_64_tbl[4];
    static Inst_Info S_group_2_65_tbl[4];
    static Inst_Info S_group_2_66_tbl[4];
    static Inst_Info S_group_2_67_tbl[4];
    static Inst_Info S_group_2_68_tbl[4];
    static Inst_Info S_group_2_69_tbl[4];
    static Inst_Info S_group_2_6a_tbl[4];
    static Inst_Info S_group_2_6b_tbl[4];
    static Inst_Info S_group_2_6c_tbl[4];
    static Inst_Info S_group_2_6d_tbl[4];
    static Inst_Info S_group_2_6e_tbl[4];
    static Inst_Info S_group_2_6f_tbl[4];
    static Inst_Info S_group_2_70_tbl[4];
    static Inst_Info S_group_2_71_tbl[8];
    static Inst_Info S_group_2_71_02_tbl[4];
    static Inst_Info S_group_2_71_04_tbl[4];
    static Inst_Info S_group_2_71_06_tbl[4];
    static Inst_Info S_group_2_72_tbl[8];
    static Inst_Info S_group_2_72_02_tbl[4];
    static Inst_Info S_group_2_72_04_tbl[4];
    static Inst_Info S_group_2_72_06_tbl[4];
    static Inst_Info S_group_2_73_tbl[8];
    static Inst_Info S_group_2_73_02_tbl[4];
    static Inst_Info S_group_2_73_03_tbl[4];
    static Inst_Info S_group_2_73_06_tbl[4];
    static Inst_Info S_group_2_73_07_tbl[4];
    static Inst_Info S_group_2_74_tbl[4];
    static Inst_Info S_group_2_75_tbl[4];
    static Inst_Info S_group_2_76_tbl[4];
    static Inst_Info S_group_2_77_tbl[4];
    static Inst_Info S_group_2_78_tbl[4];
    static Inst_Info S_group_2_78_66_tbl[8];
    static Inst_Info S_group_2_79_tbl[4];

    static Inst_Info S_group_2_7c_tbl[4];
    static Inst_Info S_group_2_7d_tbl[4];
    static Inst_Info S_group_2_7e_tbl[4];
    static Inst_Info S_group_2_7f_tbl[4];
    static Inst_Info S_group_2_ae_tbl[4];
    static Inst_Info S_group_2_ae_np_tbl[16];
    static Inst_Info S_group_2_ae_f3_tbl[16];
    static Inst_Info S_group_2_ae_f3_m3r0_tbl[2];
    static Inst_Info S_group_2_ae_f3_m3r1_tbl[2];
    static Inst_Info S_group_2_ae_f3_m3r2_tbl[2];
    static Inst_Info S_group_2_ae_f3_m3r3_tbl[2];
    static Inst_Info S_group_2_b8_tbl[2];
    static Inst_Info S_group_2_ba_tbl[8];
    static Inst_Info S_group_2_bc_tbl[2];
    static Inst_Info S_group_2_bd_tbl[2];
    static Inst_Info S_group_2_c2_tbl[4];
    static Inst_Info S_group_2_c3_tbl[4];
    static Inst_Info S_group_2_c4_tbl[4];
    static Inst_Info S_group_2_c5_tbl[4];
    static Inst_Info S_group_2_c6_tbl[4];
    static Inst_Info S_group_2_c7_tbl[8];
    static Inst_Info S_group_2_c7_01_tbl[2];
    static Inst_Info S_group_2_c7_06_tbl[3];
    static Inst_Info S_group_2_d0_tbl[4];
    static Inst_Info S_group_2_d1_tbl[4];
    static Inst_Info S_group_2_d2_tbl[4];
    static Inst_Info S_group_2_d3_tbl[4];
    static Inst_Info S_group_2_d4_tbl[4];
    static Inst_Info S_group_2_d5_tbl[4];
    static Inst_Info S_group_2_d6_tbl[4];
    static Inst_Info S_group_2_d7_tbl[4];
    static Inst_Info S_group_2_d8_tbl[4];
    static Inst_Info S_group_2_d9_tbl[4];
    static Inst_Info S_group_2_da_tbl[4];
    static Inst_Info S_group_2_db_tbl[4];
    static Inst_Info S_group_2_dc_tbl[4];
    static Inst_Info S_group_2_dd_tbl[4];
    static Inst_Info S_group_2_de_tbl[4];
    static Inst_Info S_group_2_df_tbl[4];
    static Inst_Info S_group_2_e0_tbl[4];
    static Inst_Info S_group_2_e1_tbl[4];
    static Inst_Info S_group_2_e2_tbl[4];
    static Inst_Info S_group_2_e3_tbl[4];
    static Inst_Info S_group_2_e4_tbl[4];
    static Inst_Info S_group_2_e5_tbl[4];
    static Inst_Info S_group_2_e6_tbl[4];
    static Inst_Info S_group_2_e7_tbl[4];
    static Inst_Info S_group_2_e8_tbl[4];
    static Inst_Info S_group_2_e9_tbl[4];
    static Inst_Info S_group_2_ea_tbl[4];
    static Inst_Info S_group_2_eb_tbl[4];
    static Inst_Info S_group_2_ec_tbl[4];
    static Inst_Info S_group_2_ed_tbl[4];
    static Inst_Info S_group_2_ee_tbl[4];
    static Inst_Info S_group_2_ef_tbl[4];
    static Inst_Info S_group_2_f0_tbl[4];
    static Inst_Info S_group_2_f1_tbl[4];
    static Inst_Info S_group_2_f2_tbl[4];
    static Inst_Info S_group_2_f3_tbl[4];
    static Inst_Info S_group_2_f4_tbl[4];
    static Inst_Info S_group_2_f5_tbl[4];
    static Inst_Info S_group_2_f6_tbl[4];
    static Inst_Info S_group_2_f7_tbl[4];
    static Inst_Info S_group_2_f8_tbl[4];
    static Inst_Info S_group_2_f9_tbl[4];
    static Inst_Info S_group_2_fa_tbl[4];
    static Inst_Info S_group_2_fb_tbl[4];
    static Inst_Info S_group_2_fc_tbl[4];
    static Inst_Info S_group_2_fd_tbl[4];
    static Inst_Info S_group_2_fe_tbl[4];

    static Inst_Info S_xop_tbl[128];    // mmmmmpp
    static Inst_Info S_xop_m8_tbl[256];
    static Inst_Info S_xop_m9_tbl[256];
    static Inst_Info S_xop_ma_tbl[256];

    static Inst_Info S_vex_tbl[128];        // mmmmmpp
    static Inst_Info S_vex_m1_np_tbl[256];  // 0f
    static Inst_Info S_vex_m1_66_tbl[256];  // 66 0f
    static Inst_Info S_vex_m1_f3_tbl[256];  // f3 0f
    static Inst_Info S_vex_m1_f2_tbl[256];  // f2 0f
    static Inst_Info S_vex_m2_np_tbl[256];  // 0f 38
    static Inst_Info S_vex_m2_66_tbl[256];  // 66 0f 38
    static Inst_Info S_vex_m2_f2_tbl[256];  // f2 0f 38
    static Inst_Info S_vex_m2_f3_tbl[256];  // f3 0f 38
    static Inst_Info S_vex_m3_np_tbl[256];  // 0f 3a
    static Inst_Info S_vex_m3_66_tbl[256];  // 66 0f 3a
    static Inst_Info S_vex_m3_f2_tbl[256];  // f2 0f 3a
    static Inst_Info S_vex_m3_f3_tbl[256];  // f3 0f 3a

    static Inst_Info S_vex_m1_np_12_tbl[2];
    static Inst_Info S_vex_m1_np_16_tbl[2];
    static Inst_Info S_vex_m1_np_77_tbl[2];
    static Inst_Info S_vex_m1_np_ae_tbl[16];
    static Inst_Info S_vex_m1_66_60_tbl[2];
    static Inst_Info S_vex_m1_66_61_tbl[2];
    static Inst_Info S_vex_m1_66_62_tbl[2];
    static Inst_Info S_vex_m1_66_63_tbl[2];
    static Inst_Info S_vex_m1_66_64_tbl[2];
    static Inst_Info S_vex_m1_66_65_tbl[2];
    static Inst_Info S_vex_m1_66_66_tbl[2];
    static Inst_Info S_vex_m1_66_67_tbl[2];
    static Inst_Info S_vex_m1_66_68_tbl[2];
    static Inst_Info S_vex_m1_66_69_tbl[2];
    static Inst_Info S_vex_m1_66_6a_tbl[2];
    static Inst_Info S_vex_m1_66_6b_tbl[2];
    static Inst_Info S_vex_m1_66_6c_tbl[2];
    static Inst_Info S_vex_m1_66_6d_tbl[2];
    static Inst_Info S_vex_m1_66_6e_tbl[2];
    static Inst_Info S_vex_m1_66_70_tbl[2];
    static Inst_Info S_vex_m1_66_71_tbl[16];
    static Inst_Info S_vex_m1_66_72_tbl[16];
    static Inst_Info S_vex_m1_66_73_tbl[16];
    static Inst_Info S_vex_m1_66_74_tbl[2];
    static Inst_Info S_vex_m1_66_75_tbl[2];
    static Inst_Info S_vex_m1_66_76_tbl[2];
    static Inst_Info S_vex_m1_66_7e_tbl[2];
    static Inst_Info S_vex_m1_66_d1_tbl[2];
    static Inst_Info S_vex_m1_66_d2_tbl[2];
    static Inst_Info S_vex_m1_66_d3_tbl[2];
    static Inst_Info S_vex_m1_66_d4_tbl[2];
    static Inst_Info S_vex_m1_66_d5_tbl[2];
    static Inst_Info S_vex_m1_66_d7_tbl[2];
    static Inst_Info S_vex_m1_66_d8_tbl[2];
    static Inst_Info S_vex_m1_66_d9_tbl[2];
    static Inst_Info S_vex_m1_66_da_tbl[2];
    static Inst_Info S_vex_m1_66_db_tbl[2];
    static Inst_Info S_vex_m1_66_dc_tbl[2];
    static Inst_Info S_vex_m1_66_dd_tbl[2];
    static Inst_Info S_vex_m1_66_de_tbl[2];
    static Inst_Info S_vex_m1_66_df_tbl[2];
    static Inst_Info S_vex_m1_66_e0_tbl[2];
    static Inst_Info S_vex_m1_66_e1_tbl[2];
    static Inst_Info S_vex_m1_66_e2_tbl[2];
    static Inst_Info S_vex_m1_66_e3_tbl[2];
    static Inst_Info S_vex_m1_66_e4_tbl[2];
    static Inst_Info S_vex_m1_66_e5_tbl[2];
    static Inst_Info S_vex_m1_66_e8_tbl[2];
    static Inst_Info S_vex_m1_66_e9_tbl[2];
    static Inst_Info S_vex_m1_66_ea_tbl[2];
    static Inst_Info S_vex_m1_66_eb_tbl[2];
    static Inst_Info S_vex_m1_66_ec_tbl[2];
    static Inst_Info S_vex_m1_66_ed_tbl[2];
    static Inst_Info S_vex_m1_66_ee_tbl[2];
    static Inst_Info S_vex_m1_66_ef_tbl[2];
    static Inst_Info S_vex_m1_66_f1_tbl[2];
    static Inst_Info S_vex_m1_66_f2_tbl[2];
    static Inst_Info S_vex_m1_66_f3_tbl[2];
    static Inst_Info S_vex_m1_66_f4_tbl[2];
    static Inst_Info S_vex_m1_66_f5_tbl[2];
    static Inst_Info S_vex_m1_66_f6_tbl[2];
    static Inst_Info S_vex_m1_66_f7_tbl[2];
    static Inst_Info S_vex_m1_66_f8_tbl[2];
    static Inst_Info S_vex_m1_66_f9_tbl[2];
    static Inst_Info S_vex_m1_66_fa_tbl[2];
    static Inst_Info S_vex_m1_66_fb_tbl[2];
    static Inst_Info S_vex_m1_66_fc_tbl[2];
    static Inst_Info S_vex_m1_66_fd_tbl[2];
    static Inst_Info S_vex_m1_66_fe_tbl[2];
    static Inst_Info S_vex_m1_f3_10_tbl[2];
    static Inst_Info S_vex_m1_f3_11_tbl[2];
    static Inst_Info S_vex_m1_f3_70_tbl[2];
    static Inst_Info S_vex_m1_f2_10_tbl[2];
    static Inst_Info S_vex_m1_f2_11_tbl[2];
    static Inst_Info S_vex_m1_f2_70_tbl[2];
    static Inst_Info S_vex_m2_np_f3_tbl[8];
    static Inst_Info S_vex_m2_66_00_tbl[2];
    static Inst_Info S_vex_m2_66_01_tbl[2];
    static Inst_Info S_vex_m2_66_02_tbl[2];
    static Inst_Info S_vex_m2_66_03_tbl[2];
    static Inst_Info S_vex_m2_66_04_tbl[2];
    static Inst_Info S_vex_m2_66_05_tbl[2];
    static Inst_Info S_vex_m2_66_06_tbl[2];
    static Inst_Info S_vex_m2_66_07_tbl[2];
    static Inst_Info S_vex_m2_66_08_tbl[2];
    static Inst_Info S_vex_m2_66_09_tbl[2];
    static Inst_Info S_vex_m2_66_0a_tbl[2];
    static Inst_Info S_vex_m2_66_0b_tbl[2];
    static Inst_Info S_vex_m2_66_18_tbl[2];
    static Inst_Info S_vex_m2_66_19_tbl[2];
    static Inst_Info S_vex_m2_66_1c_tbl[2];
    static Inst_Info S_vex_m2_66_1d_tbl[2];
    static Inst_Info S_vex_m2_66_1e_tbl[2];
    static Inst_Info S_vex_m2_66_20_tbl[2];
    static Inst_Info S_vex_m2_66_21_tbl[2];
    static Inst_Info S_vex_m2_66_22_tbl[2];
    static Inst_Info S_vex_m2_66_23_tbl[2];
    static Inst_Info S_vex_m2_66_24_tbl[2];
    static Inst_Info S_vex_m2_66_25_tbl[2];
    static Inst_Info S_vex_m2_66_28_tbl[2];
    static Inst_Info S_vex_m2_66_29_tbl[2];
    static Inst_Info S_vex_m2_66_2a_tbl[2];
    static Inst_Info S_vex_m2_66_2b_tbl[2];
    static Inst_Info S_vex_m2_66_30_tbl[2];
    static Inst_Info S_vex_m2_66_31_tbl[2];
    static Inst_Info S_vex_m2_66_32_tbl[2];
    static Inst_Info S_vex_m2_66_33_tbl[2];
    static Inst_Info S_vex_m2_66_34_tbl[2];
    static Inst_Info S_vex_m2_66_35_tbl[2];
    static Inst_Info S_vex_m2_66_37_tbl[2];
    static Inst_Info S_vex_m2_66_38_tbl[2];
    static Inst_Info S_vex_m2_66_39_tbl[2];
    static Inst_Info S_vex_m2_66_3a_tbl[2];
    static Inst_Info S_vex_m2_66_3b_tbl[2];
    static Inst_Info S_vex_m2_66_3c_tbl[2];
    static Inst_Info S_vex_m2_66_3d_tbl[2];
    static Inst_Info S_vex_m2_66_3e_tbl[2];
    static Inst_Info S_vex_m2_66_3f_tbl[2];
    static Inst_Info S_vex_m2_66_40_tbl[2];
    static Inst_Info S_vex_m2_66_45_tbl[2];
    static Inst_Info S_vex_m2_66_46_tbl[2];
    static Inst_Info S_vex_m2_66_47_tbl[2];
    static Inst_Info S_vex_m2_66_8c_tbl[2];
    static Inst_Info S_vex_m2_66_8e_tbl[2];
    static Inst_Info S_vex_m2_66_90_tbl[2];
    static Inst_Info S_vex_m2_66_91_tbl[2];
    static Inst_Info S_vex_m2_66_92_tbl[2];
    static Inst_Info S_vex_m2_66_93_tbl[2];
    static Inst_Info S_vex_m2_66_96_tbl[2];
    static Inst_Info S_vex_m2_66_97_tbl[2];
    static Inst_Info S_vex_m2_66_98_tbl[2];
    static Inst_Info S_vex_m2_66_99_tbl[2];
    static Inst_Info S_vex_m2_66_9a_tbl[2];
    static Inst_Info S_vex_m2_66_9b_tbl[2];
    static Inst_Info S_vex_m2_66_9c_tbl[2];
    static Inst_Info S_vex_m2_66_9d_tbl[2];
    static Inst_Info S_vex_m2_66_9e_tbl[2];
    static Inst_Info S_vex_m2_66_9f_tbl[2];
    static Inst_Info S_vex_m2_66_a6_tbl[2];
    static Inst_Info S_vex_m2_66_a7_tbl[2];
    static Inst_Info S_vex_m2_66_a8_tbl[2];
    static Inst_Info S_vex_m2_66_a9_tbl[2];
    static Inst_Info S_vex_m2_66_aa_tbl[2];
    static Inst_Info S_vex_m2_66_ab_tbl[2];
    static Inst_Info S_vex_m2_66_ac_tbl[2];
    static Inst_Info S_vex_m2_66_ad_tbl[2];
    static Inst_Info S_vex_m2_66_ae_tbl[2];
    static Inst_Info S_vex_m2_66_af_tbl[2];
    static Inst_Info S_vex_m2_66_b6_tbl[2];
    static Inst_Info S_vex_m2_66_b7_tbl[2];
    static Inst_Info S_vex_m2_66_b8_tbl[2];
    static Inst_Info S_vex_m2_66_b9_tbl[2];
    static Inst_Info S_vex_m2_66_ba_tbl[2];
    static Inst_Info S_vex_m2_66_bb_tbl[2];
    static Inst_Info S_vex_m2_66_bc_tbl[2];
    static Inst_Info S_vex_m2_66_bd_tbl[2];
    static Inst_Info S_vex_m2_66_be_tbl[2];
    static Inst_Info S_vex_m2_66_bf_tbl[2];
    static Inst_Info S_vex_m3_66_00_tbl[2];
    static Inst_Info S_vex_m3_66_01_tbl[2];
    static Inst_Info S_vex_m3_66_0e_tbl[2];
    static Inst_Info S_vex_m3_66_0f_tbl[2];
    static Inst_Info S_vex_m3_66_16_tbl[2];
    static Inst_Info S_vex_m3_66_22_tbl[2];
    static Inst_Info S_vex_m3_66_42_tbl[2];
    static Inst_Info S_vex_m3_66_4c_tbl[2];
    static Inst_Info S_xop_m9_00_tbl[8];
    static Inst_Info S_xop_m9_01_tbl[8];
    static Inst_Info S_xop_m9_02_tbl[8];
    static Inst_Info S_xop_m9_12_tbl[16];
    static Inst_Info S_xop_ma_12_tbl[8];

    static StringRef S_size_qualifiers[8];      // "byte ", "word ", "dword ", "48 bit", "qword ", "tword ", "dqword ", "none "

    void GetModrmByte();
    void GetImmediateByte();
    void GetImmediateWord();
    void GetImmediateDword();
    void GetImmediateFword();
    void GetImmediateQword();
    void GetDisplacementByte();
    void GetDisplacementWord();
    void GetDisplacementDword();
    void GetDisplacementQword();

    bool GetInfoPtr();
    void SetInstructionFlags(void);
    int DecodeAmdOperandBytes();
    void DecodeOpcodeBytes();
    void DecodeOperandBytes();
    void SwapAVXOperands(void);
    void ScanForRexPrefix(void);
    bool IsVexPrefix(AMD_UINT8 prefix, AMD_UINT8 nextbyte);

    inline bool ScanForAddressOverride()
    {
        for (int i = (GetPrefixCount() - 1); i >= 0; i--)
            if (GetPrefixByte(i) == PREFIX_ADDR)
            {
                return true;
            }

        return false;
    }

    //  inline void DecodeModrm() { (m_modrm_mod == 3) ? DecodeRegisterModrm() : DecodeMemoryModrm(); }
    void DecodeModrm();
    void OperandModrm();
    void OperandMemoryModrm();
    void OperandRegisterModrm();

    inline const StringRef& GetSizeQualifier() { return (S_size_qualifiers[m_pOperand->size]); }

    // use this after decoding instructions which ignore REX extensions
    inline void UndoRegisterExtensions() { m_pOperand->reg &= ~(0x8); }

    // verifies lock usage, throws exception if not used correctly
    inline void CheckLockPrefix()
    {
        if
        (!(((m_operands[0].type == n_Disassembler::OPERANDTYPE_MEMORY) || (m_operands[0].type == n_Disassembler::OPERANDTYPE_RIPRELATIVE)) && ((m_operands[0].flags & OPF_LOCK) != 0))
         && !(((m_operands[1].type == n_Disassembler::OPERANDTYPE_MEMORY) || (m_operands[1].type == n_Disassembler::OPERANDTYPE_RIPRELATIVE)) && ((m_operands[1].flags & OPF_LOCK) != 0))
         && !(m_inst_flags & INST_LOCK_OVERUSED)
        )
        {
            throw CFormatException();
        }
    }

    inline n_Disassembler::e_OperandSize GetWDQOperandSize()
    {
        if (m_inst_flags & INST_DATA64)
        {
            return n_Disassembler::OPERANDSIZE_64;
        }
        else if (m_inst_flags & INST_DATA32)
        {
            return n_Disassembler::OPERANDSIZE_32;
        }
        else
        {
            return n_Disassembler::OPERANDSIZE_16;
        }
    }

    inline n_Disassembler::e_OperandSize GetWDOperandSize()
    {
        if (m_inst_flags & (INST_DATA32 | INST_DATA64))
        {
            return n_Disassembler::OPERANDSIZE_32;
        }
        else
        {
            return n_Disassembler::OPERANDSIZE_16;
        }
    }

    // when tables are hooked in derived classes, the base table entries should pass the following checks
    inline void CheckHook(Inst_Info& entry, const char* tablename)
    {
        // if table is not nested and it has a mnem, then it needs to be hooked
        if ((entry.GetInfoPtr == NULL) && (entry.mnem != NULL))
            if (entry.pExtraInfo == NULL)
            {
                m_errorLevel = n_Disassembler::TABLE_EXCEPTION;
                throw CTableException(tablename);
            }
    }

    // This exception class helps the disassembler avoid walking past the
    // end of an invalid instruction (instructions are at most 15 bytes
    // long and can be restricted to shorter lengths).
    class CLengthException
    {
    public:
        CLengthException() {}
        ~CLengthException() {}
    };

    // This exception class helps the disassembler signal errors in for invalid
    // instruction (e.g. receiving register modrm instruction bytes for instructions
    // which accept only memory modrm values).
    class CFormatException
    {
    public:
        CFormatException() {}
        ~CFormatException() {}
    };

    // This exception class helps the disassembler signal table hook errors in
    // a derived classes.
    class CTableException
    {
    public:
        const char* m_tablename;

        CTableException() { m_tablename = (const char*) 0; }
        CTableException(const char* tablename) { m_tablename = tablename; }
        ~CTableException() {}
        const char* GetTablename(void) { return m_tablename; }
    };

public:
    /**************************************************************************/
    // These routines are intended for use by derived and friend classes.
    /**************************************************************************/

    // This routine provides access to a generic pointer maintained for every
    // instruction.  This pointer can be set to point at user defined data
    // in derived classes to allow the disassembler to be extensable.
    inline CExtraInstInfo* GetExtraInfoPtr() { return (m_opcode_table_ptr ? m_opcode_table_ptr->pExtraInfo : NULL); };

    // If you hook the base class's tables, you should call this routine after doing
    // so to verify that you hooked all of the relevant table entries.  This will
    // help flag the need to update your tables and initialization code to handle
    // new instructions added in future releases of the disassembler.
    bool TablesHooked();

    // This routine provides access to nested table entries
    Inst_Info* GetInstInfoPtr() { return m_opcode_table_ptr; };

    // These routines provide the algorithms necessary to index into nested
    // table entries.
    friend Inst_Info* GetFpuIndex(CDisassembler* pThis);
    friend Inst_Info* GetSSEIndex(CDisassembler* pThis);
    friend Inst_Info* GetSSEHiToLoIndex(CDisassembler* pThis);
    friend Inst_Info* Get3dnowIndex(CDisassembler* pThis);
    friend Inst_Info* GetGroupIndex(CDisassembler* pThis);
    friend Inst_Info* GetGroupCIndex(CDisassembler* pThis);          // includes special case handling of PPro nop
    friend Inst_Info* GetNewGroupIndex(CDisassembler* pThis);
    friend Inst_Info* GetLongModeIndex(CDisassembler* pThis);
    friend Inst_Info* GetNopXchgPauseIndex(CDisassembler* pThis);
    friend Inst_Info* GetWDIndex(CDisassembler* pThis);
    friend Inst_Info* GetWDQIndex(CDisassembler* pThis);
    friend Inst_Info* GetWDQIndex64(CDisassembler* pThis);       // handles "64 bit data as default" under longmode
    friend Inst_Info* GetPrefetchIndex(CDisassembler* pThis);
    friend Inst_Info* GetModRmIndex(CDisassembler* pThis);   // index of 0 if mod != 3, index of rm+1 for the remainder
    friend Inst_Info* GetJcxIndex(CDisassembler* pThis);
    friend Inst_Info* Get_2_38_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_38_XX_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_38_f01_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_3a_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_3a_XX_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_b8_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_bc_Index(CDisassembler* pThis);
    friend Inst_Info* Get_2_bd_Index(CDisassembler* pThis);
    friend Inst_Info* Get_RexOperandsize_Index(CDisassembler* pThis);
    friend Inst_Info* Get_VexOperandsize_Index(CDisassembler* pThis);
    friend Inst_Info* Get_VexW_Index(CDisassembler* pThis);
    friend Inst_Info* Get_VexL_Index(CDisassembler* pThis);
    friend Inst_Info* Get_VEX_Opcode(CDisassembler* pThis);
    friend Inst_Info* GetGroupVexLIndex(CDisassembler* pThis);

    /**************************************************************************/
    // These routines are for use during processing of instruction bytes.
    /**************************************************************************/

    // Returns true if the last instruction prefix was as queried (i.e. == prefix)
    bool IsLastPrefix(AMD_UINT8 prefix) { return (m_prefix_bytes.IsLastByte(prefix)); }

    // Returns true if there is at least one prefix and sets prefix = last prefix, else
    // returns false
    bool GetLastPrefix(AMD_UINT8* prefix) { return (m_prefix_bytes.GetLastByte(prefix)); }

    // Returns true if there is at least two prefixes and sets prefix = next to last prefix, else
    // returns false
    bool GetNextToLastPrefix(AMD_UINT8* prefix) { return (m_prefix_bytes.GetNextToLastByte(prefix)); }

    // Removes the last prefix from the list of prefixes (e.g. use to remove a LOCK prefix).
    void RemoveLastPrefix() { m_prefix_bytes.RemoveLastByte(); }

    // Removes the last prefix matching 'prefix' from the list of prefixes (use when
    // prefixes aren't prefixes and need to be removed from the list of instruction prefix
    // bytes (e.g. "lock cli" == "clx", not "lock clx").
    bool RemovePrefixByte(AMD_UINT8 prefix) { return (m_prefix_bytes.RemoveByte(prefix)); }

    // Call this to establish data size from prefix bytes and longmode/dbit prior to setting
    // m_inst_flags
    int GetDataSize();

    // Returns the address width (in hexadecimal characters)
    inline int GetAddressWidth() { return (m_longmode ? 16 : (m_dbit ? 8 : 4)); }
    inline int GetDbit() { return m_dbit; }

    bool AlternateEnabled(n_Disassembler::e_AlternateDecodings alternate)
    {
        return ((m_alternateDecodings & (1 << alternate)) != 0);
    }
    void EnableAlternate(n_Disassembler::e_AlternateDecodings alternate)
    {
        m_alternateDecodings |= (1 << alternate);
    }
    void DisableAlternate(n_Disassembler::e_AlternateDecodings alternate)
    {
        m_alternateDecodings &= ~(1 << alternate);
    }

    // Use these to locate and remove bytes from the list of prefix bytes
    // and move it into the list of opcode bytes on a special case basis.
    void HandleExtraPrefixOpcode();
    void HandleExtraRepOpcode();
    void HandleExtraRepOrRepneOpcode();
    void HandleExtraF3Opcode();

    inline void SetIndex(AMD_UINT8 index) { m_bHasIndex = true; m_index = index; }
    inline void SetBase(AMD_UINT8 base) { m_bHasBase = true; m_base = base; }

public:
    /**************************************************************************/
    // These routines are intended for use by disassembler clients.
    /**************************************************************************/

    /**************************************************************************/
    // Disassembly involves telling the disassembler the dbit setting, the
    // long mode setting (64-bit), and the instruction bytes.  The dbit and
    // long mode settings are modal and are not typically changed on every
    // instruction: call SetDbit and SetLongMode to convey those
    // values.  After setting up those modes, call the disassembler with a
    // pointer to the instruction bytes.  The instruction bytes will be either
    // Decoded, or Decoded and Disassembled.  Decoding involves breaking the
    // instruction bytes into their constituent parts (prefix bytes, opcode
    // bytes, operand bytes, ...), Disassembly involves Decode as well as
    // mnemonic generation.  Either way (Decode or Disassemble), you can query
    // the disassembler afterwards for data about the instruction bytes (if
    // Decode or Disassembly succeeded).
    //
    // CDisassembler derives from CInstructionData (defined above).  Look at
    // that class to see how to query the CDisassembler object for data about
    // the instruction bytes after obtaining a successful return value from
    // the Decode/Disassemble routines.
    //
    // Note: The buffer containing the instruction bytes needs to be either at
    // least 15 bytes long (the maximum valid instruction length), or contain
    // enough bytes for one valid instruction.  The disassembler will not look
    // past the last valid instruction byte: however, note that if you pass the
    // disassembler a pointer to a short buffer (less than 15 bytes) containing
    // nothing but instruction prefix bytes, the disassembler will look past
    // the end of that short buffer until it looks at 15 bytes (at which time
    // it would indicate that the bytes could not be disassembled) or one
    // complete instruction (presumably nonsense since some of the bytes came
    // from beyond the end of the buffer) unless you limit the disassembler to
    // less 15 bytes.
    /**************************************************************************/

    CDisassembler();
    virtual ~CDisassembler();

    // This routine is the main api to the disassembler.  It returns a pointer
    // to CDisassembler's internal mnemonics buffer on success, NULL on
    // failure.  The CDisassembler object will return meaningless values
    // unless you query it after this routine returns a success.  inst_buf
    // should point to an array of instruction bytes.
    virtual char* Disassemble(const AMD_UINT8* inst_buf);

    // This routine is the same as Disassembler( AMD_UINT8 *inst_buf ) except that
    // it allows the caller to convey the instruction pointer value.  This
    // value will be used to calculate and display memory locations instead of
    // relative offsets (i.e. mem_00401022 instead of $+1022)
    virtual char* Disassemble(const AMD_UINT8* inst_buf, AMD_UINT64 rip);

    // This routine provides a short circuit to the disassembler's decoder
    // without generating a mnemonic.  This will save time if instruction
    // decoding is all that is required (i.e. no mnemonics needed).  The
    // CDisassembler object will return meaningless values unless you query
    // it after this routine returns a success.  inst_buf should point to an
    // array of instruction bytes.
    virtual bool Decode(const AMD_UINT8* inst_buf);

    // This routine provides a one call solution to disassembly.  Normally,
    // the dbit is toggled only when it is changed: this routine allows you to
    // control it on an instruction by instruction basis in one call.  Same
    // caveats as char *Disassemble( AMD_UINT8 *inst_buf ) with regards to the
    // return values.  inst_buf should point to an array of instruction bytes.
    virtual char* Disassemble(const AMD_UINT8* inst_buf, bool dbit) { SetDbit(dbit); return Disassemble(inst_buf); }

    // This routine provides a one call solution to decoding.  Normally, the
    // dbit is toggled only when it is changed: this routine allows you to
    // control it on an instruction by instruction basis in one call.  Same
    // caveats as bool *Decode( AMD_UINT8 *inst_buf ) with regards to the return
    // values.  inst_buf should point to an array of instruction bytes.
    virtual bool Decode(const AMD_UINT8* inst_buf, bool dbit) { SetDbit(dbit); return Decode(inst_buf); }

    // These routines allow callers to limit inst_buf accesses to only the
    // specified number of bytes (bufferLength).  A CLengthException will be
    // thrown rather than access beyond the specified bufferLength.
    virtual char* Disassemble(const AMD_UINT8* inst_buf, int bufferLength);
    virtual char* Disassemble(const AMD_UINT8* inst_buf, int bufferLength, bool dbit);
    virtual char* Disassemble(const AMD_UINT8* inst_buf, int bufferLength, AMD_UINT64 rip);
    virtual bool Decode(const AMD_UINT8* inst_buf, int bufferLength);
    virtual bool Decode(const AMD_UINT8* inst_buf, int bufferLength, bool dbit);

    // This routine is provided to allow decoding and disassembly in two steps
    // First call Decode to break up the instruction bytes.  Then call this
    // routine to generate the mnemonic.
    virtual char* Disassemble();

    // Use these routines to control the Dbit and long mode settings
    inline void SetDbit(bool dbit = true) { m_dbit = dbit; };
    inline void ClearDbit() { m_dbit = false; };
    inline void SetLongMode(bool longmode = true) { m_longmode = longmode; }
    inline void ClearLongMode() { m_longmode = false; }
    inline void SetSvmMode(bool svmmode = true) { m_svmmode = svmmode; }
    inline void ClearSvmMode() { m_svmmode = false; }
    inline void SetExtensions(AMD_UINT64 extensions) { m_extensions = extensions; }
    inline AMD_UINT64 GetExtensions() { return m_extensions; }

    // Use this routine to turn on/off size qualifier strings for memory references
    // When off, size qualifiers will be provided for ambiguous cases only.
    inline void ShowMemorySize(bool showSize = true) { m_bShowSize = showSize; }

    // These routines provide control over the case of the mnemonics generated
    inline void SetLowerCase(bool lowercase = true) { m_bUpperCase = (lowercase == false); }
    inline void SetUpperCase(bool uppercase = true) { m_bUpperCase = uppercase; }

    // This routine provides control over the character used to seperate the
    // opcode from the operands.  The default seperator is a space " ".
    inline void SetOpcodeSeperator(const char* seperator) { m_opcodeSeperator = seperator; }

    // This routine provides control over the string appended to indicate
    // hexadecimal.  The string pointed to by hexPostfix must remain valid
    // while the disassembler is active.  The default hex post fix is an "h":
    // you can set it to "" to eliminate the hex postfix.
    inline void SetHexPostfix(const char* hexPostfix) { m_hexPostfix = hexPostfix; }

    inline void GetVersion(int& major, int& minor, int& build)
    {
        major = MAJOR_VERSION;
        minor = MINOR_VERSION;
        build = BUILD_LEVEL;
    }

    // Returns the error level for failed decode attempts
    inline n_Disassembler::e_DisassemblerErrorCodes GetLastError() { return m_errorLevel; }

    // Look at CDisassembler's parent class CInstructionData to see how
    // to query the disassembler object for instruction data.
};

class CExtraExtraInstInfo : public CExtraInstInfo, public std::vector <CExtraInstInfo*>
{
public:
    CExtraInstInfo** m_pExtraInfoPtr;

    CExtraExtraInstInfo(CExtraInstInfo** pPtr, size_type t, CExtraInstInfo* ptr) : std::vector <CExtraInstInfo * > (t, ptr)
    {
        m_pExtraInfoPtr = pPtr;
        *pPtr = (CExtraInstInfo*)this;
    }
    virtual ~CExtraExtraInstInfo()
    {
        *m_pExtraInfoPtr = NULL;
    }
    void Print() {}
};

class CExtraInstInfoDisassembler : public CDisassembler
{
    static std::list <CExtraExtraInstInfo*> m_Hooks;    // list of allocated Vectors
    static unsigned int m_Count;    // number of indices (arraysize) of the vectors of ExtraInfoPtrs
    static unsigned int m_NumRegistered;    // number of registered derivatives.  Vectors are deleted when this goes to zero

public:
    CExtraInstInfoDisassembler() {}

    static int RegisterExtraInstInfo() { m_NumRegistered++; return m_Count++; }
    static void UnregisterExtraInstInfo()
    {
        if (--m_NumRegistered == 0)
        {
            std::list <CExtraExtraInstInfo*>::iterator pos = m_Hooks.begin();

            while (pos != m_Hooks.end())
            {
                delete * pos++;
            }

            m_Hooks.clear();
            m_Count = 0;
        }
    }

    virtual ~CExtraInstInfoDisassembler()
    {
    }

    CExtraInstInfo* GetExtraInfoPtr(int index)
    {
        CExtraInstInfo* ptr = m_opcode_table_ptr ? m_opcode_table_ptr->pExtraInfo : NULL;

        if (ptr != NULL)
        {
            CExtraExtraInstInfo& extraPtrs = *(CExtraExtraInstInfo*)ptr;

            if ((index >= 0) && ((int)extraPtrs.size() > index))
            {
                return extraPtrs[index];
            }
        }

        return NULL;
    }
    static void HookTableEntry(int index, Inst_Info* pTableEntry, CExtraInstInfo* ptr)
    {
        CExtraExtraInstInfo* extraPtrs;

        if (pTableEntry->pExtraInfo == NULL)
        {
            extraPtrs = new CExtraExtraInstInfo(&(pTableEntry->pExtraInfo), m_Count, NULL);
            m_Hooks.push_back(extraPtrs);
        }
        else
        {
            extraPtrs = (CExtraExtraInstInfo*)(pTableEntry->pExtraInfo);

            if (extraPtrs->size() <= (unsigned int)index)
            {
                extraPtrs->resize((index + 1), NULL);
            }
        }

        (*extraPtrs)[index] = ptr;
    }
};

#endif
