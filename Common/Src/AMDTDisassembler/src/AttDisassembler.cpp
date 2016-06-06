//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AttDisassembler.cpp
/// \brief AT&T Style Disassembler class implementation.
///
//==================================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "typedefs_lin.h"

#include "AttDisassembler.h"

using namespace n_Disassembler;

bool CAttDisassembler::m_bTablesUpdated = false;        // only need to link tables once

const char* CAttDisassembler::S_size_att_operands[] = { "b", "w", "l", "", "q", "", "", "" };

const char* CAttDisassembler::S_PrefixByteStrings_att[] = {"", "", "", "", "", "", "", "",
                                                           "", "", "rex64Y ", "rex64YZ ", "rex64X", "rex64XZ", "rex64XY ", "rex64XYZ "
                                                          };

const char* CAttDisassembler::S_modrm16_str_att[] = { "(%bx,%si)", "(%bx,%di)", "(%bp,%si)", "(%bp,%di)", "(,%si)", "(,%di)", "(%bp)", "(%bx)" };
const char* CAttDisassembler::S_rex_byte_regs_att[] = { "%al", "%cl", "%dl", "%bl", "%spl", "%bpl", "%sil", "%dil",
                                                        "%r8b", "%r9b", "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b"
                                                      };
const char* CAttDisassembler::S_byte_regs_att[] = { "%al", "%cl", "%dl", "%bl", "%ah", "%ch", "%dh", "%bh" };
const char* CAttDisassembler::S_word_regs_att[] = { "%ax", "%cx", "%dx", "%bx", "%sp", "%bp", "%si", "%di",
                                                    "%r8w", "%r9w", "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w"
                                                  };
const char* CAttDisassembler::S_dword_regs_att[] = { "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi",
                                                     "%r8d", "%r9d", "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"
                                                   };
const char* CAttDisassembler::S_qword_regs_att[] = { "%rax", "%rcx", "%rdx",  "%rbx",  "%rsp",  "%rbp",  "%rsi",  "%rdi",
                                                     "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
                                                   };
const char* CAttDisassembler::S_control_regs_att[] = { "%cr0", "%cr1", "%cr2", "%cr3", "%cr4", "%cr5", "%cr6", "%cr7",
                                                       "%cr8", "%cr9", "%cr10", "%cr11", "%cr12", "%cr13", "%cr14", "%cr15"
                                                     };
const char* CAttDisassembler::S_debug_regs_att[] = { "%db0", "%db1", "%db2", "%db3", "%db4", "%db5", "%db6", "%db7",
                                                     "%db8", "%db9", "%db10", "%db11", "%db12", "%db13", "%db14", "%db15"
                                                   };
const char* CAttDisassembler::S_segment_regs_att[] = { "%es", "%cs", "%ss", "%ds", "%fs", "%gs", "%res", "%res" };
const char* CAttDisassembler::S_fpu_regs_att[] = { "%st(0)", "%st(1)", "%st(2)", "%st(3)", "%st(4)", "%st(5)", "%st(6)", "%st(7)", "%st" };
const char* CAttDisassembler::S_mmx_regs_att[] = { "%mm0", "%mm1", "%mm2", "%mm3", "%mm4", "%mm5", "%mm6", "%mm7" };
const char* CAttDisassembler::S_xmmx_regs_att[] = { "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7",
                                                    "%xmm8", "%xmm9", "%xmm10", "%xmm11", "%xmm12", "%xmm13", "%xmm14", "%xmm15"
                                                  };

char gInitialPostfix[] = "";
char gInitialImmedPrefix[] = "$\0";
char gHexPrefix[] = "0x\0";

static const char S_MNEM_NAME_CBTW [] = "cbtw";
static const char S_MNEM_NAME_CWTL [] = "cwtl";
static const char S_MNEM_NAME_CLTQ [] = "cltq";
static const char S_MNEM_NAME_CWTD [] = "cwtd";
static const char S_MNEM_NAME_CLTD [] = "cltd";
static const char S_MNEM_NAME_CQTO [] = "cqto";
static const char S_MNEM_NAME_LJMP [] = "ljmp";
static const char S_MNEM_NAME_LCALL [] = "lcall";
static const char S_MNEM_NAME_LRET [] = "lret";
static const char S_MNEM_NAME_RETW [] = "retw";
static const char S_MNEM_NAME_RETL [] = "retl";
static const char S_MNEM_NAME_RETQ [] = "retq";
static const char S_MNEM_NAME_MOVSB [] = "movsb";
static const char S_MNEM_NAME_MOVSW [] = "movsw";
static const char S_MNEM_NAME_MOVSL [] = "movsl";
static const char S_MNEM_NAME_MOVS [] = "movs";
static const char S_MNEM_NAME_MOVZB [] = "movzb";
static const char S_MNEM_NAME_MOVZW [] = "movzw";
static const char S_MNEM_NAME_FADDS [] = "fadds";
static const char S_MNEM_NAME_FADDL [] = "faddl";
static const char S_MNEM_NAME_FIADDL [] = "fiaddl";
static const char S_MNEM_NAME_FNCLEX [] = "fnclex";
static const char S_MNEM_NAME_FCOMS [] = "fcoms";
static const char S_MNEM_NAME_FCOML [] = "fcoml";
static const char S_MNEM_NAME_FCOMPS [] = "fcomps";
static const char S_MNEM_NAME_FCOMPL [] = "fcompl";
static const char S_MNEM_NAME_FDIVRS [] = "fdivrs";
static const char S_MNEM_NAME_FDIVP [] = "fdivp";
static const char S_MNEM_NAME_FDIVRL [] = "fdivrl";
static const char S_MNEM_NAME_FDIVRP [] = "fdivrp";
static const char S_MNEM_NAME_FIDIVL [] = "fidivl";
static const char S_MNEM_NAME_FIDIVRL [] = "fidivrl";
static const char S_MNEM_NAME_FDIV [] = "fdiv";
static const char S_MNEM_NAME_FICOML [] = "ficoml";
static const char S_MNEM_NAME_FICOMPL [] = "ficompl";
static const char S_MNEM_NAME_FILDL [] = "fildl";
static const char S_MNEM_NAME_FILDLL [] = "fildll";
static const char S_MNEM_NAME_FIMULL [] = "fimull";
static const char S_MNEM_NAME_FMULS [] = "fmuls";
static const char S_MNEM_NAME_FMULL [] = "fmull";
static const char S_MNEM_NAME_FISTL [] = "fistl";
static const char S_MNEM_NAME_FISTPL [] = "fistpl";
static const char S_MNEM_NAME_FISTPLL [] = "fistpll";
static const char S_MNEM_NAME_FISUBL [] = "fisubl";
static const char S_MNEM_NAME_FISUBRL [] = "fisubrl";
static const char S_MNEM_NAME_FSUBS [] = "fsubs";
static const char S_MNEM_NAME_FSUBRS [] = "fsubrs";
static const char S_MNEM_NAME_FSUBL [] = "fsubl";
static const char S_MNEM_NAME_FSUBRL [] = "fsubrl";
static const char S_MNEM_NAME_FSUBR [] = "fsubr";
static const char S_MNEM_NAME_FSUBRP [] = "fsubrp";
static const char S_MNEM_NAME_FSUB [] = "fsub";
static const char S_MNEM_NAME_FSUBP [] = "fsubp";
static const char S_MNEM_NAME_FLDS [] = "flds";
static const char S_MNEM_NAME_FLDL [] = "fldl";
static const char S_MNEM_NAME_FLDT [] = "fldt";
static const char S_MNEM_NAME_FNSAVE [] = "fnsave";
static const char S_MNEM_NAME_FSTS [] = "fsts";
static const char S_MNEM_NAME_FSTL [] = "fstl";
static const char S_MNEM_NAME_FSTPS [] = "fstps";
static const char S_MNEM_NAME_FSTPL [] = "fstpl";
static const char S_MNEM_NAME_FSTPT [] = "fstpt";
static const char S_MNEM_NAME_FNSTCW [] = "fnstcw";
static const char S_MNEM_NAME_FNSTENV [] = "fnstenv";
static const char S_MNEM_NAME_FNSTSW [] = "fnstsw";
static const char S_MNEM_NAME_PFMULHRW [] = "pfmulhrw";
static const char S_MNEM_NAME_CMPS [] = "cmps";
static const char S_MNEM_NAME_INS [] = "ins";
static const char S_MNEM_NAME_OUTS [] = "outs";
static const char S_MNEM_NAME_SCAS [] = "scas";
static const char S_MNEM_NAME_LODS [] = "lods";
static const char S_MNEM_NAME_POPA [] = "popa";
static const char S_MNEM_NAME_PUSHA [] = "pusha";
static const char S_MNEM_NAME_PUSHF [] = "pushf";
static const char S_MNEM_NAME_POPF [] = "popf";
static const char S_MNEM_NAME_STOS [] = "stos";
static const char S_MNEM_NAME_SGDTL [] = "sgdtl";
static const char S_MNEM_NAME_SIDTL [] = "sidtl";
static const char S_MNEM_NAME_LGDTL [] = "lgdtl";
static const char S_MNEM_NAME_LIDTL [] = "lidtl";
static const char S_MNEM_NAME_SGDTQ [] = "sgdtq";
static const char S_MNEM_NAME_SIDTQ [] = "sidtq";
static const char S_MNEM_NAME_LGDTQ [] = "lgdtq";
static const char S_MNEM_NAME_LIDTQ [] = "lidtq";
static const char S_SIZE_QUALIFIER [] = "";
static const char S_MNEM_PUSH_ES [] = "push %es";
static const char S_MNEM_POP_ES [] = "pop %es";
static const char S_MNEM_PUSH_CS [] = "push %cs";
static const char S_MNEM_PUSH_SS [] = "push %ss";
static const char S_MNEM_POP_SS [] = "pop %ss";
static const char S_MNEM_PUSH_DS [] = "push %ds";
static const char S_MNEM_POP_DS [] = "pop %ds";
static const char S_MNEM_PUSH_FS [] = "push %fs";
static const char S_MNEM_POP_FS [] = "pop %fs";
static const char S_MNEM_PUSH_GS [] = "push %gs";
static const char S_MNEM_POP_GS [] = "pop %gs";

CAttDisassembler::CAttDisassembler()
{
    int i;
    m_immediateData = 0;
    m_immd = 0;
    m_hexPrefix = gHexPrefix;
    m_hexPostfix = gInitialPostfix;
    m_immedPrefix = gInitialImmedPrefix;
    //Initialize the static variables

    if (!m_bTablesUpdated)
    {
        m_bTablesUpdated = true;

        //swap out menomics
        S_group_1_98_tbl[0].mnem = S_MNEM_NAME_CBTW;
        S_group_1_98_tbl[1].mnem = S_MNEM_NAME_CWTL;
        S_group_1_98_tbl[2].mnem = S_MNEM_NAME_CLTQ;
        S_group_1_99_tbl[0].mnem = S_MNEM_NAME_CWTD;
        S_group_1_99_tbl[1].mnem = S_MNEM_NAME_CLTD;
        S_group_1_99_tbl[2].mnem = S_MNEM_NAME_CQTO;
        S_oneByteOpcodes_tbl[0xea].mnem = S_MNEM_NAME_LJMP;
        S_group_1_ff_tbl[5].mnem = S_MNEM_NAME_LJMP;
        S_oneByteOpcodes_tbl[0x9a].mnem = S_MNEM_NAME_LCALL;
        S_group_1_ff_tbl[3].mnem = S_MNEM_NAME_LCALL;

        for (i = 0; i < 3; i++)
        {
            S_group_1_ca_tbl[i].mnem = S_MNEM_NAME_LRET;
            S_group_1_cb_tbl[i].mnem = S_MNEM_NAME_LRET;
        }

        S_group_1_c2_tbl[0].mnem = S_MNEM_NAME_RETW;
        S_group_1_c2_tbl[1].mnem = S_MNEM_NAME_RETL;
        S_group_1_c2_tbl[2].mnem = S_MNEM_NAME_RETQ;
        S_group_1_c3_tbl[0].mnem = S_MNEM_NAME_RETW;
        S_group_1_c3_tbl[1].mnem = S_MNEM_NAME_RETL;
        S_group_1_c3_tbl[2].mnem = S_MNEM_NAME_RETQ;
        S_twoByteOpcodes_tbl[0xbe].mnem = S_MNEM_NAME_MOVSB;
        S_twoByteOpcodes_tbl[0xbf].mnem = S_MNEM_NAME_MOVSW;
        S_group_1_63_tbl[1].mnem = S_MNEM_NAME_MOVSL;
        S_group_1_a5_tbl[1].mnem = S_MNEM_NAME_MOVS;
        S_twoByteOpcodes_tbl[0xb6].mnem = S_MNEM_NAME_MOVZB;
        S_twoByteOpcodes_tbl[0xb7].mnem = S_MNEM_NAME_MOVZW;
        S_group_1_d8_tbl[0x00].mnem = S_MNEM_NAME_FADDS;
        S_group_1_dc_tbl[0x00].mnem = S_MNEM_NAME_FADDL;
        S_group_1_da_tbl[0x00].mnem = S_MNEM_NAME_FIADDL;
        S_group_1_db_tbl[42].mnem = S_MNEM_NAME_FNCLEX;
        S_group_1_d8_tbl[0x02].mnem = S_MNEM_NAME_FCOMS;
        S_group_1_dc_tbl[0x02].mnem = S_MNEM_NAME_FCOML;
        S_group_1_d8_tbl[0x03].mnem = S_MNEM_NAME_FCOMPS;
        S_group_1_dc_tbl[0x03].mnem = S_MNEM_NAME_FCOMPL;
        S_group_1_d8_tbl[0x07].mnem = S_MNEM_NAME_FDIVRS;

        for (i = 56; i < 64; i++)
        {
            S_group_1_de_tbl[i].mnem = S_MNEM_NAME_FDIVP;
        }

        S_group_1_dc_tbl[0x07].mnem = S_MNEM_NAME_FDIVRL;

        for (i = 64; i < 72; i++)
        {
            S_group_1_de_tbl[i].mnem = S_MNEM_NAME_FDIVRP;
        }

        S_group_1_da_tbl[0x06].mnem = S_MNEM_NAME_FIDIVL;
        S_group_1_da_tbl[0x07].mnem = S_MNEM_NAME_FIDIVRL;

        for (i = 56; i < 64; i++)
        {
            S_group_1_dc_tbl[i].mnem = S_MNEM_NAME_FDIV;
        }

        S_group_1_da_tbl[0x02].mnem = S_MNEM_NAME_FICOML;
        S_group_1_da_tbl[0x03].mnem = S_MNEM_NAME_FICOMPL;
        S_group_1_db_tbl[0x00].mnem = S_MNEM_NAME_FILDL;
        S_group_1_df_tbl[0x05].mnem = S_MNEM_NAME_FILDLL;
        S_group_1_da_tbl[0x01].mnem = S_MNEM_NAME_FIMULL;
        S_group_1_d8_tbl[0x01].mnem = S_MNEM_NAME_FMULS;
        S_group_1_dc_tbl[0x01].mnem = S_MNEM_NAME_FMULL;
        S_group_1_db_tbl[0x02].mnem = S_MNEM_NAME_FISTL;
        S_group_1_db_tbl[0x03].mnem = S_MNEM_NAME_FISTPL;
        S_group_1_df_tbl[0x07].mnem = S_MNEM_NAME_FISTPLL;
        S_group_1_da_tbl[0x04].mnem = S_MNEM_NAME_FISUBL;
        S_group_1_da_tbl[0x05].mnem = S_MNEM_NAME_FISUBRL;
        S_group_1_d8_tbl[0x04].mnem = S_MNEM_NAME_FSUBS;
        S_group_1_d8_tbl[0x05].mnem = S_MNEM_NAME_FSUBRS;
        S_group_1_dc_tbl[0x04].mnem = S_MNEM_NAME_FSUBL;
        S_group_1_dc_tbl[0x05].mnem = S_MNEM_NAME_FSUBRL;

        for (i = 48; i < 56; i++)
        {
            S_group_1_dc_tbl[i].mnem = S_MNEM_NAME_FSUBR;
        }

        for (i = 48; i < 56; i++)
        {
            S_group_1_de_tbl[i].mnem = S_MNEM_NAME_FSUBRP;
        }

        for (i = 40; i < 48; i++)
        {
            S_group_1_dc_tbl[i].mnem = S_MNEM_NAME_FSUB;
        }

        for (i = 40; i < 48; i++)
        {
            S_group_1_de_tbl[i].mnem = S_MNEM_NAME_FSUBP;
        }

        S_group_1_d9_tbl[0x00].mnem = S_MNEM_NAME_FLDS;
        S_group_1_dd_tbl[0x00].mnem = S_MNEM_NAME_FLDL;
        S_group_1_db_tbl[0x05].mnem = S_MNEM_NAME_FLDT;
        S_group_1_dd_tbl[0x06].mnem = S_MNEM_NAME_FNSAVE;
        S_group_1_d9_tbl[0x02].mnem = S_MNEM_NAME_FSTS;
        S_group_1_dd_tbl[0x02].mnem = S_MNEM_NAME_FSTL;
        S_group_1_d9_tbl[0x03].mnem = S_MNEM_NAME_FSTPS;
        S_group_1_dd_tbl[0x03].mnem = S_MNEM_NAME_FSTPL;
        S_group_1_db_tbl[0x07].mnem = S_MNEM_NAME_FSTPT;
        S_group_1_d9_tbl[0x07].mnem = S_MNEM_NAME_FNSTCW;
        S_group_1_d9_tbl[0x06].mnem = S_MNEM_NAME_FNSTENV;
        S_group_1_dd_tbl[0x07].mnem = S_MNEM_NAME_FNSTSW;
        S_group_1_df_tbl[40].mnem = S_MNEM_NAME_FNSTSW;
        S_group_2_0f_tbl[21].mnem = S_MNEM_NAME_PFMULHRW;
        S_oneByteOpcodes_tbl[0xa6].mnem = S_MNEM_NAME_CMPS;
        S_group_1_a7_tbl[1].mnem = S_MNEM_NAME_CMPS;
        S_oneByteOpcodes_tbl[0x6c].mnem = S_MNEM_NAME_INS;
        S_group_1_6d_tbl[1].mnem = S_MNEM_NAME_INS;
        S_oneByteOpcodes_tbl[0x6e].mnem = S_MNEM_NAME_OUTS;
        S_group_1_6f_tbl[1].mnem = S_MNEM_NAME_OUTS;
        S_oneByteOpcodes_tbl[0xae].mnem = S_MNEM_NAME_SCAS;

        for (i = 0; i < 3; i++)
        {
            S_group_1_af_tbl[i].mnem = S_MNEM_NAME_SCAS;
        }

        S_oneByteOpcodes_tbl[0xac].mnem = S_MNEM_NAME_LODS;

        for (i = 0; i < 3; i++)
        {
            S_group_1_ad_tbl[i].mnem = S_MNEM_NAME_LODS;
        }

        S_group_1_61_tbl[1].mnem = S_MNEM_NAME_POPA;
        S_group_1_60_tbl[1].mnem = S_MNEM_NAME_PUSHA;
        S_group_1_9c_tbl[1].mnem = S_MNEM_NAME_PUSHF;
        S_group_1_9d_tbl[1].mnem = S_MNEM_NAME_POPF;
        S_oneByteOpcodes_tbl[0xa4].mnem = S_MNEM_NAME_MOVS;
        S_oneByteOpcodes_tbl[0xaa].mnem = S_MNEM_NAME_STOS;
        S_group_1_ab_tbl[1].mnem = S_MNEM_NAME_STOS;

        S_group_2_01_tbl[0].mnem    = S_MNEM_NAME_SGDTL;
        S_group_2_01_01_tbl[0].mnem = S_MNEM_NAME_SIDTL;
        S_group_2_01_02_tbl[0].mnem = S_MNEM_NAME_LGDTL;
        S_group_2_01_03_tbl[0].mnem = S_MNEM_NAME_LIDTL;

        //set case for showing size
        S_group_1_f7_tbl[6].operand_flags[0] |= OPF_SHOWSIZE;             //div
        S_group_1_ff_tbl[0].operand_flags[0] |= OPF_SHOWSIZE;             //inc
        S_group_1_ff_tbl[1].operand_flags[0] |= OPF_SHOWSIZE;             //dec
        S_oneByteOpcodes_tbl[0x8c].operand_flags[0] |= OPF_SHOWSIZE;      //mov
        S_twoByteOpcodes_tbl[0xbe].operand_flags[0] |= OPF_SHOWSIZE;      //movsb
        S_twoByteOpcodes_tbl[0xbf].operand_flags[0] |= OPF_SHOWSIZE;      //movsw
        S_twoByteOpcodes_tbl[0xb6].operand_flags[0] |= OPF_SHOWSIZE;      //movzb
        S_twoByteOpcodes_tbl[0xb7].operand_flags[0] |= OPF_SHOWSIZE;      //movzw

        for (i = 0; i < 8; i++)
        {
            S_size_qualifiers[i] = S_SIZE_QUALIFIER;
        }

        //Modify the registers to have the %
        for (i = 0; i < 8; i++)
        {
            S_modrm16_str[i] = S_modrm16_str_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_rex_byte_regs[i] = S_rex_byte_regs_att[i];
        }

        for (i = 0; i < 8; i++)
        {
            S_byte_regs[i] = S_byte_regs_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_word_regs[i] = S_word_regs_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_dword_regs[i] = S_dword_regs_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_qword_regs[i] = S_qword_regs_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_control_regs[i] = S_control_regs_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_debug_regs[i] = S_debug_regs_att[i];
        }

        for (i = 0; i < 8; i++)
        {
            S_segment_regs[i] = S_segment_regs_att[i];
        }

        S_oneByteOpcodes_tbl[0x6].mnem = S_MNEM_PUSH_ES;
        S_oneByteOpcodes_tbl[0x7].mnem = S_MNEM_POP_ES;
        S_oneByteOpcodes_tbl[0xe].mnem = S_MNEM_PUSH_CS;
        S_oneByteOpcodes_tbl[0x16].mnem = S_MNEM_PUSH_SS;
        S_oneByteOpcodes_tbl[0x17].mnem = S_MNEM_POP_SS;
        S_oneByteOpcodes_tbl[0x1e].mnem = S_MNEM_PUSH_DS;
        S_oneByteOpcodes_tbl[0x1f].mnem = S_MNEM_POP_DS;
        S_twoByteOpcodes_tbl[0xa0].mnem = S_MNEM_PUSH_FS;
        S_twoByteOpcodes_tbl[0xa1].mnem = S_MNEM_POP_FS;
        S_twoByteOpcodes_tbl[0xa8].mnem = S_MNEM_PUSH_GS;
        S_twoByteOpcodes_tbl[0xa9].mnem = S_MNEM_POP_GS;

        for (i = 0; i < 9; i++)
        {
            S_fpu_regs[i] = S_fpu_regs_att[i];
        }

        for (i = 0; i < 8; i++)
        {
            S_mmx_regs[i] = S_mmx_regs_att[i];
        }

        for (i = 0; i < 16; i++)
        {
            S_xmmx_regs[i] = S_xmmx_regs_att[i];
        }
    }

    //overwrite the function table with the current class
    //Note that this would cause two disassemblers opened at once to be risky
    for (i = 0; i <= OPRND_UxM2; i++)
    {
        restore_DisassembleOperandFnPtrs[i] = S_DisassembleOperandFnPtrs[i];
    }

    S_DisassembleOperandFnPtrs[OPRND_1] = (PVOIDMEMBERFUNC) &CAttDisassembler::_1Str;
    S_DisassembleOperandFnPtrs[OPRND_AL] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegALStr;
    S_DisassembleOperandFnPtrs[OPRND_AX] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegAXStr;
    S_DisassembleOperandFnPtrs[OPRND_eAX] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegeAXStr;
    S_DisassembleOperandFnPtrs[OPRND_Ap] = (PVOIDMEMBERFUNC) &CAttDisassembler::AddressStr;
    S_DisassembleOperandFnPtrs[OPRND_CL] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegCLStr;
    S_DisassembleOperandFnPtrs[OPRND_DX] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegDXStr;
    S_DisassembleOperandFnPtrs[OPRND_eDX] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegeDXStr;
    S_DisassembleOperandFnPtrs[OPRND_Eb] = (PVOIDMEMBERFUNC) &CAttDisassembler::ModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ew] = (PVOIDMEMBERFUNC) &CAttDisassembler::ModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ed] = (PVOIDMEMBERFUNC) &CAttDisassembler::ModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ev] = (PVOIDMEMBERFUNC) &CAttDisassembler::ModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ep] = (PVOIDMEMBERFUNC) &CAttDisassembler::ModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Mt] = (PVOIDMEMBERFUNC)  &CAttDisassembler::MemoryModrmStr;

    S_DisassembleOperandFnPtrs[OPRND_Ib] = (PVOIDMEMBERFUNC) &CAttDisassembler::ByteImmediateStr;
    S_DisassembleOperandFnPtrs[OPRND_Iz] = (PVOIDMEMBERFUNC) &CAttDisassembler::WordOrDwordImmediateStr;
    S_DisassembleOperandFnPtrs[OPRND_Iw] = (PVOIDMEMBERFUNC) &CAttDisassembler::WordImmediateStr;
    S_DisassembleOperandFnPtrs[OPRND_Jb] = (PVOIDMEMBERFUNC) &CAttDisassembler::SignedByteJumpStr;
    S_DisassembleOperandFnPtrs[OPRND_Jz] = (PVOIDMEMBERFUNC) &CAttDisassembler::SignedWordOrDwordJumpStr;
    S_DisassembleOperandFnPtrs[OPRND_Qq] = (PVOIDMEMBERFUNC) &CAttDisassembler::MMXModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_M] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Mp] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Mq] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ms] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ob] = (PVOIDMEMBERFUNC) &CAttDisassembler::OffsetStr;
    S_DisassembleOperandFnPtrs[OPRND_Ov] = (PVOIDMEMBERFUNC) &CAttDisassembler::OffsetStr;
    S_DisassembleOperandFnPtrs[OPRND_Xb] = (PVOIDMEMBERFUNC) &CAttDisassembler::DsEsiStr;
    S_DisassembleOperandFnPtrs[OPRND_Xv] = (PVOIDMEMBERFUNC) &CAttDisassembler::DsEsiStr;
    S_DisassembleOperandFnPtrs[OPRND_Yb] = (PVOIDMEMBERFUNC) &CAttDisassembler::EsEdiStr;
    S_DisassembleOperandFnPtrs[OPRND_Yv] = (PVOIDMEMBERFUNC) &CAttDisassembler::EsEdiStr;
    S_DisassembleOperandFnPtrs[OPRND_FPU_AX] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegAXStr;
    S_DisassembleOperandFnPtrs[OPRND_Mw] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Md] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Md_q] = (PVOIDMEMBERFUNC) &CAttDisassembler::MemoryModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wps] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wq] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wss] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wpd] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wsd] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wx] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wqdq] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_UxMq] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_UxMd] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_UxMw] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Uq] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ups] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Upd] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Ux] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Wqdq] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_UxMb] = (PVOIDMEMBERFUNC) &CAttDisassembler::SimdModrmStr;
    S_DisassembleOperandFnPtrs[OPRND_Iv] = (PVOIDMEMBERFUNC) &CAttDisassembler::WDQImmediateStr;
    S_DisassembleOperandFnPtrs[OPRND_eBXAl] = (PVOIDMEMBERFUNC) &CAttDisassembler::RegeBXAndALStr;
    S_DisassembleOperandFnPtrs[OPRND_Ed_q] = (PVOIDMEMBERFUNC) &CAttDisassembler::ModrmStr;
} //CAttDisassembler::CAttDisassembler

CAttDisassembler::~CAttDisassembler()
{
    //reset the static pointers
    for (int i = 0; i <= OPRND_UxM2; i++)
    {
        S_DisassembleOperandFnPtrs[i] = restore_DisassembleOperandFnPtrs[i];
    }
}

char* CAttDisassembler::Disassemble(const UINT8* inst_buf)
{
    m_bCalculateRipRelative = false;

    if (Decode(inst_buf))
    {
        Disassemble();
        return (m_mnem.data());
    }

    return (NULL);
}

char* CAttDisassembler::Disassemble(const UINT8* inst_buf, UINT64 rip)
{
    m_rip = rip;
    m_bCalculateRipRelative = true;

    if (Decode(inst_buf))
    {
        Disassemble();
        return (m_mnem.data());
    }

    return (NULL);
}

bool CAttDisassembler::GetNoSizeOp(UINT8 ui8Operand)
{
    bool b_no_size = false;

    switch (ui8Operand)
    {
        //mmx
        case OPRND_Pq:
        case OPRND_Pd:
        case OPRND_Qq:
        case OPRND_Pd_q:

        //sse
        case OPRND_Vps:
        case OPRND_Vq:
        case OPRND_Vss:
        case OPRND_Wps:
        case OPRND_Wq:
        case OPRND_Wss:

        //sse 2
        case OPRND_Vpd:
        case OPRND_Wpd:
        case OPRND_Vsd:
        case OPRND_Wsd:
        case OPRND_Vd_q:
        case OPRND_Wx:
        case OPRND_Wqdq:
        case OPRND_Vd:
            b_no_size = true;
            break;

        default: break;
    }

    return b_no_size;
}

bool CAttDisassembler::GetNoSizeInst()
{
    bool b_no_size = false;

    //call,jmp
    if ((&(S_group_1_ff_tbl[2]) == m_opcode_table_ptr) ||
        (&(S_group_1_ff_tbl[4]) == m_opcode_table_ptr) ||
        (&(S_group_2_01_tbl[6]) == m_opcode_table_ptr) || //lmsw
        (&(S_group_2_01_tbl[0]) == m_opcode_table_ptr) || //sgdt
        (&(S_group_2_01_tbl[1]) == m_opcode_table_ptr) || //sidt
        (&(S_group_2_01_tbl[2]) == m_opcode_table_ptr) || // lgdt
        (&(S_group_2_01_tbl[3]) == m_opcode_table_ptr))   //lidt
    {
        b_no_size = true;
    }

    return b_no_size;
}

char* CAttDisassembler::Disassemble()
{
    bool b_printed_first = false;
    bool b_no_size = false;
    m_mnem.clear();        // clear mnemonic string

    GetPrefixBytesString();

    m_mnem += (char*)(m_opcode_table_ptr->mnem);
    m_immediateData = m_immd;

    //check if we shouldn't show the size
    if ((GetNoSizeOp(m_opcode_table_ptr->operands[0]))
        || (GetNoSizeOp(m_opcode_table_ptr->operands[1])))
    {
        b_no_size = true;
    }

    if (m_mnem[0] == 'f')
    {
        b_no_size = true;
    }

    if (GetNoSizeInst())
    {
        b_no_size = true;
    }

    //write the size
    if ((((m_operands[0].type == OPERANDTYPE_RIPRELATIVE) && (HasImmediate()))
         || (OPF_SHOWSIZE & m_opcode_table_ptr->operand_flags[0])
         || (m_operands[0].type == OPERANDTYPE_MEMORY))
        && (!b_no_size))
    {
        m_mnem += GetAttSize(m_operands[0].size);
    }

    m_mnem += m_opcodeSeperator;

    //absolute addressing
    for (int i = 2; i <= 5; i++)
    {
        if ((m_opcode_table_ptr == &(S_group_1_ff_tbl[i])))
        {
            m_mnem += '*';
            break;
        }
    }

    //If not the enter assembly command
    if (m_opcode_table_ptr != &(S_oneByteOpcodes_tbl[0xc8]))
    {
        //the operands should be shown "source, destination"
        for (int i = (MAX_OPERANDS - 1); i >= 0; i--)
        {
            m_pOperand = &m_operands[i];

            if (EXPLICIT_OPERAND(*m_pOperand) && (S_DisassembleOperandFnPtrs[m_opcode_table_ptr->operands[i]] != (PVOIDMEMBERFUNC)NULL))
            {
                if (b_printed_first)
                {
                    m_mnem += ',';
                }
                else
                {
                    b_printed_first = true;
                }

                (this->*S_DisassembleOperandFnPtrs[m_opcode_table_ptr->operands[i]])();

            }
        }
    }
    else
    {
        //If enter, source & dest don't apply to the operands
        for (int i = 0; i < MAX_OPERANDS; i++)
        {
            m_pOperand = &m_operands[i];

            if (EXPLICIT_OPERAND(*m_pOperand) && (S_DisassembleOperandFnPtrs[m_opcode_table_ptr->operands[i]] != (PVOIDMEMBERFUNC)NULL))
            {
                if (b_printed_first)
                {
                    m_mnem += ',';
                }
                else
                {
                    b_printed_first = true;
                }

                (this->*S_DisassembleOperandFnPtrs[m_opcode_table_ptr->operands[i]])();

            }
        }
    }

    if (m_bUpperCase)
    {
        for (size_t i = 0; i < m_mnem.size(); i++)
        {
            m_mnem[i] = (char)toupper(m_mnem[i]);
        }
    }

    return (m_mnem.data());
} //CAttDisassembler::Disassemble()

//The format for att is seg:disp(base, index, scale)
void CAttDisassembler::MemoryModrmStr()
{
    if (m_inst_flags & (INST_ADDR32 | INST_ADDR64))
    {
        if (m_modrm_rm == 4)
        {
            MemoryModrmStrWithSIB();
        }
        else
        {
            MemoryModrmStrWithoutSIB();
        }
    }
    else        // 16 bit mode
    {
        if (m_inst_flags & INST_SEGOVRD)
        {
            m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
            m_mnem += ':';
        }

        if (m_modrm_mod == 0)
        {
            if (m_modrm_rm == 6)
            {
                m_mnem.appendUInt(m_hexPrefix, m_disp, 4);
            }
            else
            {
                m_mnem += S_modrm16_str[m_modrm_rm];
            }
        }
        else if (m_modrm_mod == 1)
        {
            m_mnem.appendSInt(m_hexPrefix, m_disp, 2);
            m_mnem += S_modrm16_str[m_modrm_rm];
        }
        else
        {
            m_mnem.appendSInt(m_hexPrefix, m_disp, 4);
            m_mnem += S_modrm16_str[m_modrm_rm];
        }
    }
} //CAttDisassembler::MemoryModrmStr()

//The format for att is seg:disp(base, index, scale)
void CAttDisassembler::MemoryModrmStrWithSIB()
{
    char disp_buf[21] = "";
    char base_buf[16] = "";
    char idx_buf[16] = "";
    char scale_buf[16] = "";
    MutableStringRef disp_str(disp_buf, sizeof(disp_buf) - 1);
    MutableStringRef base_str(base_buf, sizeof(base_buf) - 1);
    MutableStringRef idx_str(idx_buf, sizeof(idx_buf) - 1);
    MutableStringRef scale_str(scale_buf, sizeof(scale_buf) - 1);

    if (BASE(m_sib) == 5)
    {
        if (m_modrm_mod != 0)
        {
            base_str += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);
        }

        if (m_modrm_mod == 1)
        {
            disp_str.appendSInt(m_hexPrefix, m_disp, 2);
        }
        else if (m_modrm_mod == 2)
        {
            disp_str.appendSInt(m_hexPrefix, m_disp, 8);
        }
        else if (m_modrm_mod == 0)
        {
            if ((IDX(m_sib) != 4) || REX_SIB_INDEX(m_rex_prefix))
            {
                disp_str.appendSInt(m_hexPrefix, m_disp, 8);
            }
            else
            {
                disp_str.appendUInt(m_hexPrefix, m_disp, 8);
            }
        }
    }
    else
    {
        base_str += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);

        if (m_modrm_mod == 1)
        {
            disp_str.appendSInt(m_hexPrefix, m_disp, 2);
        }
        else if (m_modrm_mod == 2)
        {
            disp_str.appendSInt(m_hexPrefix, m_disp, 8);
        }
    }

    if ((m_inst_flags & INST_ADDR64) && (!disp_str.empty()))
    {
        disp_str.appendSInt(m_hexPrefix, m_disp, 16);
    }

    if ((IDX(m_sib) != 4) || REX_SIB_INDEX(m_rex_prefix))
    {
        idx_str += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_index] : S_dword_regs[m_index]);

        if (SS(m_sib) == 1)
        {
            scale_str += '2';
        }
        else if (SS(m_sib) == 2)
        {
            scale_str += '4';
        }
        else if (SS(m_sib) == 3)
        {
            scale_str += '8';
        }
        else
        {
            scale_str += '1';
        }
    }

    // concat all these strings together

    if (m_inst_flags & INST_SEGOVRD)
    {
        m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
        m_mnem += ':';
    }

    if (!disp_str.empty())
    {
        m_mnem += disp_str;
    }

    m_mnem += '(';
    m_mnem += base_str;

    if (!idx_str.empty())
    {
        m_mnem += ',';
        m_mnem += idx_str;
    }
    else if (!scale_str.empty())
    {
        m_mnem += ',';
    }

    if (!scale_str.empty())
    {
        m_mnem += ',';
        m_mnem += scale_str;
    }

    m_mnem += ')';
} //CAttDisassembler::MemoryModrmStrWithSIB()

//The format for att is seg:disp(base, index, scale)
void CAttDisassembler::MemoryModrmStrWithoutSIB()
{

    if (m_inst_flags & INST_SEGOVRD)
    {
        m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
        m_mnem += ':';
    }

    if (m_modrm_mod == 0)
    {
        if (m_modrm_rm == 5)
        {
            if (m_longmode)
            {
                if (m_bCalculateRipRelative)
                {
                    m_mnem += "loc_";
                    m_mnem.appendUInt(m_hexPrefix, (m_rip + m_disp + GetLength()), GetAddressWidth());
                }
                else
                {
                    m_mnem.appendSInt(m_hexPrefix, m_disp, 8);
                    m_mnem += "(%rip)";
                }
            }
            else
            {
                m_mnem.appendUInt(m_hexPrefix, (UINT64)m_disp, 8);
            }
        }
        else
        {
            m_mnem += '(';
            m_mnem += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);
            m_mnem += ')';
        }
    }
    else if (m_modrm_mod == 1)
    {
        m_mnem.appendSInt(m_hexPrefix, m_disp, 2);
        m_mnem += '(';
        m_mnem += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);
        m_mnem += ')';
    }
    else
    {
        m_mnem.appendSInt(m_hexPrefix, m_disp, 8);
        m_mnem += '(';
        m_mnem += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);
        m_mnem += ')';
    }
} //CAttDisassembler::MemoryModrmStrWithoutSIB()

void CAttDisassembler::ByteImmediateStr()
{
    m_mnem += m_immedPrefix;
    m_mnem.appendUInt(m_hexPrefix, (m_immediateData & 0xff), 2);
    m_immediateData >>= 8;
}

void CAttDisassembler::WordImmediateStr()
{
    // Word immediates are the only immediates which could potentially be sharing immediate
    // data with both operands.  This occurs only for the "enter imm16,imm8" instruction.
    // Consequently, m_immd must be masked to a word to insure only a word is displayed.
    m_mnem += m_immedPrefix;
    m_mnem.appendUInt(m_hexPrefix, (m_immediateData & 0xffff), 4);
    m_immediateData >>= 16;
}

void CAttDisassembler::WordOrDwordImmediateStr()
{
    if ((m_inst_flags & (INST_DATA32 | INST_DATA64)) == 0)
    {
        m_mnem += m_immedPrefix;
        m_mnem.appendUInt(m_hexPrefix, (m_immediateData & 0xffff), 4);
        m_immediateData >>= 16;
    }
    else
    {
        m_mnem += m_immedPrefix;
        m_mnem.appendUInt(m_hexPrefix, (m_immediateData & 0xffffffff), 8);
        m_immediateData >>= 32;
    }
} //CAttDisassembler::WordOrDwordImmediateStr()

void CAttDisassembler::WDQImmediateStr()
{
    if (m_inst_flags & INST_DATA64)
    {
        m_mnem += m_immedPrefix;
        m_mnem.appendUInt(m_hexPrefix, m_immediateData, 16);
        m_immediateData = 0;
    }
    else if (m_inst_flags & INST_DATA32)
    {
        m_mnem += m_immedPrefix;
        m_mnem.appendUInt(m_hexPrefix, (m_immediateData & 0xffffffff), 8);
        m_immediateData >>= 32;
    }
    else
    {
        m_mnem += m_immedPrefix;
        m_mnem.appendUInt(m_hexPrefix, (m_immediateData & 0xffff), 4);
        m_immediateData >>= 16;
    }
}
void CAttDisassembler::SignedByteJumpStr()
{
    if (m_bCalculateRipRelative)
    {
        m_mnem += "loc_";
        m_mnem.appendUInt(m_hexPrefix, (m_rip + m_disp + GetLength()), GetAddressWidth());
    }
    else
    {
        m_mnem += '$';
        m_mnem.appendSInt(m_hexPrefix, (m_rip + m_disp + GetLength()), GetAddressWidth());
    }
}

void CAttDisassembler::SignedWordOrDwordJumpStr()
{
    if (m_bCalculateRipRelative)
    {
        m_mnem += "loc_";

        if (m_pOperand->size == OPERANDSIZE_16)
        {
            m_mnem.appendUInt(m_hexPrefix, (m_rip + m_disp + GetLength()), GetAddressWidth());
        }
        else
        {
            m_mnem.appendUInt(m_hexPrefix, (m_rip + m_disp + GetLength()), GetAddressWidth());
        }
    }
    else
    {
        m_mnem += '$';

        if (m_pOperand->size == OPERANDSIZE_16)
        {
            m_mnem.appendSInt(m_hexPrefix, (m_disp + GetLength()), 4);
        }
        else
        {

            m_mnem.appendSInt(m_hexPrefix, (m_disp + GetLength()), 8);
        }
    }
}


void CAttDisassembler::_1Str()
{
    m_mnem += "$1";
}

void CAttDisassembler::RegALStr()
{
    m_mnem += S_byte_regs[REG_EAX];   //al
}

void CAttDisassembler::RegAXStr()
{
    m_mnem += S_word_regs[REG_EAX];   //ax
}

void CAttDisassembler::RegeAXStr()
{
    if (m_pOperand->size == OPERANDSIZE_16)
    {
        m_mnem += S_word_regs[REG_EAX];    //ax
    }
    else if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem += S_dword_regs[REG_EAX];    //eax
    }
    else
    {
        m_mnem += S_qword_regs[REG_EAX];    //rax
    }
}

void CAttDisassembler::RegCLStr()
{
    m_mnem += S_byte_regs[REG_ECX];   //cl
}

void CAttDisassembler::RegDXStr()
{
    m_mnem += S_word_regs[REG_EDX];   //dx
}

void CAttDisassembler::RegeDXStr()
{
    if (m_pOperand->size == OPERANDSIZE_16)
    {
        m_mnem += S_word_regs[REG_EDX];    //dx
    }
    else if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem += S_dword_regs[REG_EDX];    //edx
    }
    else
    {
        m_mnem += S_qword_regs[REG_EDX];    //rdx
    }
}

void CAttDisassembler::AddressStr()
{
    // can't use UnsignedString/SignedString multiple times in sprintf calls (overwrites static buffer)
    m_mnem += '$';

    if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem.appendUInt(m_hexPrefix, (UINT16)(m_immd >> 16), 4);
        m_mnem += ',';
        m_mnem += '$';
        m_mnem.appendUInt(m_hexPrefix, (UINT16)m_immd, 4);
    }
    else
    {
        m_mnem.appendUInt(m_hexPrefix, (UINT16)(m_immd >> 32), 4);
        m_mnem += ',';
        m_mnem += '$';
        m_mnem.appendUInt(m_hexPrefix, (UINT32)m_immd, 8);
    }
}

void CAttDisassembler::OffsetStr()
{
    if (m_inst_flags & INST_SEGOVRD)
    {
        m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
        m_mnem += ':';
    }

    if (m_inst_flags & INST_ADDR64)
    {
        m_mnem.appendUInt(m_hexPrefix, m_immd, 16);
    }
    else if (m_inst_flags & INST_ADDR32)
    {
        m_mnem.appendUInt(m_hexPrefix, m_immd, 8);
    }
    else
    {
        m_mnem.appendUInt(m_hexPrefix, m_immd, 4);
    }
} //CAttDisassembler::OffsetStr()

void CAttDisassembler::MMXRegAndByteImmediateStr()
{
    ByteImmediateStr();
    m_mnem += ',';
    MMXRegStr();
}

void CAttDisassembler::MMXModrmAndByteImmediateStr()
{
    ByteImmediateStr();
    m_mnem += ',';
    MMXModrmStr();
}

void CAttDisassembler::DsEsiStr()
{
    if (m_inst_flags & INST_ADDR64)
    {
        m_mnem += S_segment_regs[REG_DS - REG_ES];  //ds
        m_mnem += ":(";
        m_mnem += S_qword_regs[REG_ESI];  //rsi
    }
    else
    {
        if (m_inst_flags & INST_SEGOVRD)
        {
            m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
            m_mnem += ":(";

            if (m_inst_flags & INST_ADDR32)
            {
                m_mnem += S_dword_regs[REG_ESI];    //esi
            }
            else
            {
                m_mnem += S_word_regs[REG_ESI];    //si
            }

        }
        else
        {
            m_mnem += S_segment_regs[REG_DS - REG_ES];  //ds
            m_mnem += ":(";

            if (m_inst_flags & INST_ADDR32)
            {
                m_mnem += S_dword_regs[REG_ESI];   //esi
            }
            else
            {
                m_mnem += S_word_regs[REG_ESI];   //si
            }
        }
    }

    m_mnem += ')';
} //CAttDisassembler::DsEsiStr()

void CAttDisassembler::EsEdiStr()
{
    m_mnem += S_segment_regs[REG_ES - REG_ES];  //es
    m_mnem += ":(";

    if (m_inst_flags & INST_ADDR64)
    {
        m_mnem += S_qword_regs[REG_EDI];  //rdi
    }
    else if (m_inst_flags & INST_ADDR32)
    {
        m_mnem += S_dword_regs[REG_EDI];   //edi
    }
    else
    {
        m_mnem += S_word_regs[REG_EDI];   //di
    }

    m_mnem += ')';
} //CAttDisassembler::EsEdiStr()

void CAttDisassembler::RegeBXAndALStr()
{
    if (m_inst_flags & (INST_SEGOVRD | INST_ADDROVRD))
    {
        if (m_inst_flags & INST_SEGOVRD)
        {
            m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
            m_mnem += ':';
        }

        m_mnem += '(';

        if (m_inst_flags & INST_ADDR64)
        {
            m_mnem += S_qword_regs[REG_EBX];    //rbx
        }
        else if (m_inst_flags & INST_ADDR32)
        {
            m_mnem += S_dword_regs[REG_EBX];    //ebx
        }
        else
        {
            m_mnem += S_word_regs[REG_EBX];    //bx
        }

        m_mnem += ',';
        m_mnem += S_byte_regs[REG_EAX];  //al
        m_mnem += ",$1)";
    }
}

void CAttDisassembler::MMXModrmStr()
{
    if (m_modrm_mod == 3)
    {
        m_mnem += S_mmx_regs[m_pOperand->reg];
    }
    else
    {
        MemoryModrmStr();
    }
}

void CAttDisassembler::SimdModrmStr()
{
    if (m_modrm_mod == 3)
    {
        m_mnem += S_xmmx_regs[m_pOperand->reg];
    }
    else
    {
        MemoryModrmStr();
    }
}

void CAttDisassembler::SetLongMode(bool longmode)
{
    m_longmode = longmode;

    if (longmode)
    {
        S_group_2_01_tbl[0].mnem    = S_MNEM_NAME_SGDTQ;
        S_group_2_01_01_tbl[0].mnem = S_MNEM_NAME_SIDTQ;
        S_group_2_01_02_tbl[0].mnem = S_MNEM_NAME_LGDTQ;
        S_group_2_01_03_tbl[0].mnem = S_MNEM_NAME_LIDTQ;
    }
    else
    {
        S_group_2_01_tbl[0].mnem    = S_MNEM_NAME_SGDTL;
        S_group_2_01_01_tbl[0].mnem = S_MNEM_NAME_SIDTL;
        S_group_2_01_02_tbl[0].mnem = S_MNEM_NAME_LGDTL;
        S_group_2_01_03_tbl[0].mnem = S_MNEM_NAME_LIDTL;
    }
}

void CAttDisassembler::ClearLongMode()
{
    m_longmode = false;
    S_group_2_01_tbl[0].mnem    = S_MNEM_NAME_SGDTL;
    S_group_2_01_01_tbl[0].mnem = S_MNEM_NAME_SIDTL;
    S_group_2_01_02_tbl[0].mnem = S_MNEM_NAME_LGDTL;
    S_group_2_01_03_tbl[0].mnem = S_MNEM_NAME_LIDTL;
}


#ifdef ATTDISASSEMBLER_STANDALONE

CAttDisassembler disassembler;
bool showMemorySize = false;

// gets ptr to the option value: handles options of the form "-x=value" or "-x value"
#define GET_OPTION_PTR(argv,argc,i,j) ((argv[i][j] == '=') ? &argv[i][j+1] : ((++i < argc) ? argv[i] : NULL))

void Usage()
{
    puts("Usage: attstandalone [-h] [-d] [-i] [-m] [-o] [filename]");
    puts("\t-h\t\tPrint usage statement");
    puts("\t-m\t\tShow memory size qualifiers");
}

bool ParseCommandLine(int argc, char* argv[])
{
    char* argptr;

    for (int i = 1; i < argc; i++)
    {
        // It's not in the Usage statement, but you can also use "/" as a command line switch indicator
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (argv[i][1])
            {
                case 'h':
                    Usage();
                    return false;

                case 'm':
                    showMemorySize = true;
                    break;

                default:
                    Usage();
                    return false;
            }
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    if (ParseCommandLine(argc, argv))
    {
        if (showMemorySize)
        {
            disassembler.ShowMemorySize();
        }

        unsigned count = 0;
        char line[256];

        while (gets(line))
        {
            count++;

            UINT8 instBuf[16];
            int len = 0;
            char* ptr = line;

#ifdef CONVEY_RIP
            UINT64 rip = strtoul(ptr, &ptr, 16);
#endif

            while (*ptr != '\0')
            {
                instBuf[len++] = strtoul(ptr, &ptr, 16);
            }

            disassembler.SetDbit(instBuf[len - 2]);
            disassembler.SetLongMode(instBuf[len - 1]);

#ifdef CONVEY_RIP

            if (disassembler.Disassemble(instBuf, rip) != NULL)
#else
            if (disassembler.Disassemble(instBuf) != NULL)
#endif
                printf("%s\n", disassembler.GetMnemonic());
            else
            {
                printf("%08d: Unable to disassemble\n", count);
            }
        }
    }
}
#endif
