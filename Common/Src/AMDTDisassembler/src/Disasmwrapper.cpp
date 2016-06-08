//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Disasmwrapper.cpp
///
//==================================================================================

#ifdef _WINDOWS
    #include <windows.h>
#else
    #include <string.h>
    #include "typedefs_lin.h"
#endif

#include "Disasmwrapper.h"

static CInstr_ExtraCodes S_oneByteOpcodes_extra_tbl[256];
static CInstr_ExtraCodes S_twoByteOpcodes_extra_tbl[256];

//Group tables for single byte opcodes
static CInstr_ExtraCodes S_group_1_60_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_61_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_63_extra_tbl[2];
static CInstr_ExtraCodes S_group_1_6d_extra_tbl[2];
static CInstr_ExtraCodes S_group_1_6f_extra_tbl[2];
static CInstr_ExtraCodes S_group_1_80_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_81_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_82_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_83_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_90_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_98_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_99_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_9c_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_9d_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_a5_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_a7_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_ab_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_ad_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_af_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_c0_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_c1_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_c2_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_c3_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_ca_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_cb_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_cf_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_d0_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_d1_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_d2_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_d3_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_d8_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_d9_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_da_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_db_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_dc_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_dd_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_de_extra_tbl[72];
static CInstr_ExtraCodes S_group_1_df_extra_tbl[72];
// REX
static CInstr_ExtraCodes S_group_1_e3_extra_tbl[3];
static CInstr_ExtraCodes S_group_1_fa_extra_tbl[2];
static CInstr_ExtraCodes S_group_1_fb_extra_tbl[2];
static CInstr_ExtraCodes S_group_1_f6_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_f7_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_fe_extra_tbl[8];
static CInstr_ExtraCodes S_group_1_ff_extra_tbl[8];


// group tables for multi byte opcodes
static CInstr_ExtraCodes S_group_2_00_extra_tbl[8];
static CInstr_ExtraCodes S_group_2_01_extra_tbl[8];
static CInstr_ExtraCodes S_group_2_0d_extra_tbl[2]; // prefetch
static CInstr_ExtraCodes S_group_2_0f_extra_tbl[24];
//SSE
static CInstr_ExtraCodes S_group_2_10_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_11_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_12_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_12_00_extra_tbl[2];
static CInstr_ExtraCodes S_group_2_13_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_14_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_15_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_16_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_16_00_extra_tbl[2];
static CInstr_ExtraCodes S_group_2_17_extra_tbl[4];
// SSE and PPRO
static CInstr_ExtraCodes S_group_2_18_extra_tbl[16];        // includes special case handling of PPro nop
//SSE
static CInstr_ExtraCodes S_group_2_28_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_29_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_2a_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_2b_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_2c_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_2d_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_2e_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_2f_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_50_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_51_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_52_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_53_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_54_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_55_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_56_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_57_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_58_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_59_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_5a_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_5b_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_5c_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_5d_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_5e_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_5f_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_60_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_61_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_62_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_63_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_64_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_65_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_66_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_67_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_68_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_69_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_6a_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_6b_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_6c_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_6d_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_6e_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_6f_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_70_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_71_extra_tbl[8];
static CInstr_ExtraCodes S_group_2_71_02_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_71_04_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_71_06_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_72_extra_tbl[8];
static CInstr_ExtraCodes S_group_2_72_02_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_72_04_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_72_06_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_73_extra_tbl[8];
static CInstr_ExtraCodes S_group_2_73_02_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_73_03_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_73_06_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_73_07_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_74_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_75_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_76_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_7e_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_7f_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_ae_00_extra_tbl[16];
static CInstr_ExtraCodes S_group_2_ae_03_08_extra_tbl[2];
static CInstr_ExtraCodes S_group_2_ae_03_09_extra_tbl[2];
static CInstr_ExtraCodes S_group_2_ae_03_10_extra_tbl[2];
static CInstr_ExtraCodes S_group_2_ae_03_11_extra_tbl[2];
static CInstr_ExtraCodes S_group_2_ba_extra_tbl[8];
//SSE
static CInstr_ExtraCodes S_group_2_c2_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_c4_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_c5_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_c6_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_c7_extra_tbl[8];
static CInstr_ExtraCodes S_group_2_d1_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d2_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d3_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d4_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d5_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d6_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d7_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d8_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_d9_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_da_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_db_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_dc_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_dd_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_de_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_df_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e0_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e1_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e2_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e3_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e4_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e5_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e6_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e7_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e8_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_e9_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_ea_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_eb_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_ec_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_ed_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_ee_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_ef_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f1_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f2_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f3_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f4_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f5_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f6_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f7_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f8_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_f9_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_fa_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_fb_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_fc_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_fd_extra_tbl[4];
static CInstr_ExtraCodes S_group_2_fe_extra_tbl[4];

////////////////////////////////////////////////////////////////////////
// COperandField overloading
const COperandField& COperandField::operator = (const COperandField& right)
{
    if (&right != this)
    {
        AddrMethod = right.AddrMethod;
        OperType = right.OperType;
        MemAccessType = right.MemAccessType;
    }

    return *this;
}

BOOL COperandField::IsImplicit()
{
    if ((AddrMethod >= _AL) && (AddrMethod <= _EBXAL))
    {
        return TRUE;
    }
    else { return FALSE; }
}

eRegType COperandField::GetOperandFieldReg()
{
    if ((AddrMethod >= _AL) && (AddrMethod <= _DH))
    {
        return (eRegType)(AddrMethod - _AL + evREG_AL);
    }
    else if ((AddrMethod >= _AX) && (AddrMethod <= _DI))
    {
        return (eRegType)(AddrMethod - _AX + evREG_AX);
    }
    else if ((AddrMethod >= _EAX) && (AddrMethod <= _EDI))
    {
        return (eRegType)(AddrMethod - _EAX + evREG_EAX);
    }
    else if ((AddrMethod >= _ES) && (AddrMethod <= _GS))
    {
        return (eRegType)(AddrMethod - _ES + evREG_ES);
    }
    else if ((AddrMethod >= _ST) && (AddrMethod <= _ST7))
    {
        if (AddrMethod == _ST)
        {
            return evREG_ST0;
        }
        else
        {
            return (eRegType)(AddrMethod - _ST0 + evREG_ST0);
        }
    }
    else if (AddrMethod == _EBXAL)
    {
        return evREG_EBX;
    }
    else
    {
        return evREG_EFLAGS;
    }

}

eOperandSize COperandField::GetOperandFieldSize()
{
    if (((AddrMethod >= _AL) && (AddrMethod <= _DH)) || (AddrMethod == _EBXAL))
    {
        return evSizeByte;
    }
    else if ((AddrMethod >= _AX) && (AddrMethod <= _DI))
    {
        return evSizeWord;
    }
    else if ((AddrMethod >= _EAX) && (AddrMethod <= _EDI))
    {
        return evSizeDWord;
    }
    else if ((AddrMethod >= _ES) && (AddrMethod <= _GS))
    {
        return evSizeDWord;
    }
    else if ((AddrMethod >= _ST) && (AddrMethod <= _ST7))
    {
        return evSizeTByte;
    }
    else { return evSizeDWord; }

}


/////////////////////////////////////////////////////////////////////////////
// CInstr_Table overloading
const CInstr_Table& CInstr_Table::operator=(const CInstr_Table& right)
{
    if (&right != this)
    {
        strcpy_s(Mnemonic, MAX_MNEMONIC_LENGTH, right.Mnemonic);
        NumOperands = right.NumOperands;

        OpField1.AddrMethod = right.OpField1.AddrMethod;
        OpField1.OperType = right.OpField1.OperType;
        OpField1.MemAccessType = right.OpField1.MemAccessType;

        OpField2.AddrMethod = right.OpField2.AddrMethod;
        OpField2.OperType = right.OpField2.OperType;
        OpField2.MemAccessType = right.OpField2.MemAccessType;

        OpField3.AddrMethod = right.OpField3.AddrMethod;
        OpField3.OperType = right.OpField3.OperType;
        OpField3.MemAccessType = right.OpField3.MemAccessType;

        InstSpecies = right.InstSpecies;
        ProcessorType = right.ProcessorType;
    }

    return *this;
}

CInstr_ExtraCodes::CInstr_ExtraCodes()
{
    strcpy_s(instr_table.Mnemonic, MAX_MNEMONIC_LENGTH, "");
    instr_table.NumOperands = 0;

    instr_table.OpField1.AddrMethod = na;
    instr_table.OpField1.OperType = NA;
    instr_table.OpField1.MemAccessType = nA;

    instr_table.OpField2.AddrMethod = na;
    instr_table.OpField2.OperType = NA;
    instr_table.OpField2.MemAccessType = nA;

    instr_table.OpField3.AddrMethod = na;
    instr_table.OpField3.OperType = NA;
    instr_table.OpField3.MemAccessType = nA;

    instr_table.InstSpecies = evNASpecies;
    instr_table.ProcessorType = K6;
}

CInstr_ExtraCodes::CInstr_ExtraCodes(CInstr_Table intable)
{
    instr_table = intable;
}


const CInstr_ExtraCodes& CInstr_ExtraCodes::operator=(const CInstr_Table& right)
{
    instr_table = right;
    return *this;
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDisasmwrapper::CDisasmwrapper()
{
    AssignTableContent();
    Hookuptables();
}

CDisasmwrapper::~CDisasmwrapper()
{
}

void CDisasmwrapper::Hookuptables()
{
    int i;

    for (i = 0; i < 256; i++)
    {
        S_oneByteOpcodes_tbl[i].pExtraInfo = &S_oneByteOpcodes_extra_tbl[i];
        S_twoByteOpcodes_tbl[i].pExtraInfo = &S_twoByteOpcodes_extra_tbl[i];
    }

    for (i = 0; i < 3; i++)
    {
        S_group_1_60_tbl[i].pExtraInfo = &S_group_1_60_extra_tbl[i];
        S_group_1_61_tbl[i].pExtraInfo = &S_group_1_61_extra_tbl[i];
        S_group_1_90_tbl[i].pExtraInfo = &S_group_1_90_extra_tbl[i];
        S_group_1_98_tbl[i].pExtraInfo = &S_group_1_98_extra_tbl[i];
        S_group_1_99_tbl[i].pExtraInfo = &S_group_1_99_extra_tbl[i];
        S_group_1_9c_tbl[i].pExtraInfo = &S_group_1_9c_extra_tbl[i];
        S_group_1_9d_tbl[i].pExtraInfo = &S_group_1_9d_extra_tbl[i];
        S_group_1_a5_tbl[i].pExtraInfo = &S_group_1_a5_extra_tbl[i];
        S_group_1_a7_tbl[i].pExtraInfo = &S_group_1_a7_extra_tbl[i];
        S_group_1_ab_tbl[i].pExtraInfo = &S_group_1_ab_extra_tbl[i];
        S_group_1_ad_tbl[i].pExtraInfo = &S_group_1_ad_extra_tbl[i];
        S_group_1_af_tbl[i].pExtraInfo = &S_group_1_af_extra_tbl[i];
        S_group_1_c2_tbl[i].pExtraInfo = &S_group_1_c2_extra_tbl[i];
        S_group_1_c3_tbl[i].pExtraInfo = &S_group_1_c3_extra_tbl[i];
        S_group_1_ca_tbl[i].pExtraInfo = &S_group_1_ca_extra_tbl[i];
        S_group_1_cb_tbl[i].pExtraInfo = &S_group_1_cb_extra_tbl[i];
        S_group_1_cf_tbl[i].pExtraInfo = &S_group_1_cf_extra_tbl[i];
        // added for version 1.1
        S_group_1_e3_tbl[i].pExtraInfo = &S_group_1_e3_extra_tbl[i];
    }

    for (i = 0; i < 2; i++)
    {
        S_group_1_63_tbl[i].pExtraInfo = &S_group_1_63_extra_tbl[i];
        S_group_1_6d_tbl[i].pExtraInfo = &S_group_1_6d_extra_tbl[i];
        S_group_1_6f_tbl[i].pExtraInfo = &S_group_1_6f_extra_tbl[i];
        S_group_2_0d_tbl[i].pExtraInfo = &S_group_2_0d_extra_tbl[i];
        // added for version 1.1
        S_group_2_12_00_tbl[i].pExtraInfo = &S_group_2_12_00_extra_tbl[i];
        S_group_2_16_00_tbl[i].pExtraInfo = &S_group_2_16_00_extra_tbl[i];
    }

    for (i = 0; i < 8; i++)
    {
        S_group_1_80_tbl[i].pExtraInfo = &S_group_1_80_extra_tbl[i];
        S_group_1_81_tbl[i].pExtraInfo = &S_group_1_81_extra_tbl[i];
        S_group_1_82_tbl[i].pExtraInfo = &S_group_1_82_extra_tbl[i];
        S_group_1_83_tbl[i].pExtraInfo = &S_group_1_83_extra_tbl[i];
        S_group_1_c0_tbl[i].pExtraInfo = &S_group_1_c0_extra_tbl[i];
        S_group_1_c1_tbl[i].pExtraInfo = &S_group_1_c1_extra_tbl[i];
        S_group_1_d0_tbl[i].pExtraInfo = &S_group_1_d0_extra_tbl[i];
        S_group_1_d1_tbl[i].pExtraInfo = &S_group_1_d1_extra_tbl[i];
        S_group_1_d2_tbl[i].pExtraInfo = &S_group_1_d2_extra_tbl[i];
        S_group_1_d3_tbl[i].pExtraInfo = &S_group_1_d3_extra_tbl[i];
        S_group_1_f6_tbl[i].pExtraInfo = &S_group_1_f6_extra_tbl[i];
        S_group_1_f7_tbl[i].pExtraInfo = &S_group_1_f7_extra_tbl[i];
        S_group_1_fe_tbl[i].pExtraInfo = &S_group_1_fe_extra_tbl[i];
        S_group_1_ff_tbl[i].pExtraInfo = &S_group_1_ff_extra_tbl[i];
        S_group_2_00_tbl[i].pExtraInfo = &S_group_2_00_extra_tbl[i];
        S_group_2_01_tbl[i].pExtraInfo = &S_group_2_01_extra_tbl[i];
        S_group_2_ba_tbl[i].pExtraInfo = &S_group_2_ba_extra_tbl[i];
        S_group_2_c7_tbl[i].pExtraInfo = &S_group_2_c7_extra_tbl[i];
        S_group_2_71_tbl[i].pExtraInfo = &S_group_2_71_extra_tbl[i];
        S_group_2_72_tbl[i].pExtraInfo = &S_group_2_72_extra_tbl[i];
        S_group_2_73_tbl[i].pExtraInfo = &S_group_2_73_extra_tbl[i];
    }



    for (i = 0; i < 72; i++)
    {
        S_group_1_d8_tbl[i].pExtraInfo = &S_group_1_d8_extra_tbl[i];
        S_group_1_d9_tbl[i].pExtraInfo = &S_group_1_d9_extra_tbl[i];
        S_group_1_da_tbl[i].pExtraInfo = &S_group_1_da_extra_tbl[i];
        S_group_1_db_tbl[i].pExtraInfo = &S_group_1_db_extra_tbl[i];
        S_group_1_dc_tbl[i].pExtraInfo = &S_group_1_dc_extra_tbl[i];
        S_group_1_dd_tbl[i].pExtraInfo = &S_group_1_dd_extra_tbl[i];
        S_group_1_de_tbl[i].pExtraInfo = &S_group_1_de_extra_tbl[i];
        S_group_1_df_tbl[i].pExtraInfo = &S_group_1_df_extra_tbl[i];
    }

    for (i = 0; i < 24; i++)
    {
        S_group_2_0f_tbl[i].pExtraInfo = &S_group_2_0f_extra_tbl[i];
    }

    for (i = 0; i < 16; i++)
    {
        S_group_2_18_tbl[i].pExtraInfo = &S_group_2_18_extra_tbl[i];    // includes special case handling of PPro nop
        S_group_2_ae_np_tbl[i].pExtraInfo = &S_group_2_ae_00_extra_tbl[i];
    }

    for (i = 0; i < 2; i++)
    {
        S_group_2_ae_f3_m3r0_tbl[i].pExtraInfo = &S_group_2_ae_03_08_extra_tbl[i];
        S_group_2_ae_f3_m3r1_tbl[i].pExtraInfo = &S_group_2_ae_03_09_extra_tbl[i];
        S_group_2_ae_f3_m3r2_tbl[i].pExtraInfo = &S_group_2_ae_03_10_extra_tbl[i];
        S_group_2_ae_f3_m3r3_tbl[i].pExtraInfo = &S_group_2_ae_03_11_extra_tbl[i];
    }

    for (i = 0; i < 4; i++)
    {
        S_group_2_10_tbl[i].pExtraInfo = &S_group_2_10_extra_tbl[i];
        S_group_2_11_tbl[i].pExtraInfo = &S_group_2_11_extra_tbl[i];
        // added for version 1.1
        S_group_2_12_tbl[i].pExtraInfo = &S_group_2_12_extra_tbl[i];
        S_group_2_13_tbl[i].pExtraInfo = &S_group_2_13_extra_tbl[i];

        S_group_2_14_tbl[i].pExtraInfo = &S_group_2_14_extra_tbl[i];
        S_group_2_15_tbl[i].pExtraInfo = &S_group_2_15_extra_tbl[i];
        //added for version 1.1
        S_group_2_16_tbl[i].pExtraInfo = &S_group_2_16_extra_tbl[i];
        S_group_2_17_tbl[i].pExtraInfo = &S_group_2_17_extra_tbl[i];

        S_group_2_28_tbl[i].pExtraInfo = &S_group_2_28_extra_tbl[i];
        S_group_2_29_tbl[i].pExtraInfo = &S_group_2_29_extra_tbl[i];
        S_group_2_2a_tbl[i].pExtraInfo = &S_group_2_2a_extra_tbl[i];
        S_group_2_2b_tbl[i].pExtraInfo = &S_group_2_2b_extra_tbl[i];
        S_group_2_2c_tbl[i].pExtraInfo = &S_group_2_2c_extra_tbl[i];
        S_group_2_2d_tbl[i].pExtraInfo = &S_group_2_2d_extra_tbl[i];
        S_group_2_2e_tbl[i].pExtraInfo = &S_group_2_2e_extra_tbl[i];
        S_group_2_2f_tbl[i].pExtraInfo = &S_group_2_2f_extra_tbl[i];
        S_group_2_50_tbl[i].pExtraInfo = &S_group_2_50_extra_tbl[i];
        S_group_2_51_tbl[i].pExtraInfo = &S_group_2_51_extra_tbl[i];
        S_group_2_52_tbl[i].pExtraInfo = &S_group_2_52_extra_tbl[i];
        S_group_2_53_tbl[i].pExtraInfo = &S_group_2_53_extra_tbl[i];
        S_group_2_54_tbl[i].pExtraInfo = &S_group_2_54_extra_tbl[i];
        S_group_2_55_tbl[i].pExtraInfo = &S_group_2_55_extra_tbl[i];
        S_group_2_56_tbl[i].pExtraInfo = &S_group_2_56_extra_tbl[i];
        S_group_2_57_tbl[i].pExtraInfo = &S_group_2_57_extra_tbl[i];
        S_group_2_58_tbl[i].pExtraInfo = &S_group_2_58_extra_tbl[i];
        S_group_2_59_tbl[i].pExtraInfo = &S_group_2_59_extra_tbl[i];
        S_group_2_5a_tbl[i].pExtraInfo = &S_group_2_5a_extra_tbl[i];
        S_group_2_5b_tbl[i].pExtraInfo = &S_group_2_5b_extra_tbl[i];
        S_group_2_5c_tbl[i].pExtraInfo = &S_group_2_5c_extra_tbl[i];
        S_group_2_5d_tbl[i].pExtraInfo = &S_group_2_5d_extra_tbl[i];
        S_group_2_5e_tbl[i].pExtraInfo = &S_group_2_5e_extra_tbl[i];
        S_group_2_5f_tbl[i].pExtraInfo = &S_group_2_5f_extra_tbl[i];
        S_group_2_60_tbl[i].pExtraInfo = &S_group_2_60_extra_tbl[i];
        S_group_2_61_tbl[i].pExtraInfo = &S_group_2_61_extra_tbl[i];
        S_group_2_62_tbl[i].pExtraInfo = &S_group_2_62_extra_tbl[i];
        S_group_2_63_tbl[i].pExtraInfo = &S_group_2_63_extra_tbl[i];
        S_group_2_64_tbl[i].pExtraInfo = &S_group_2_64_extra_tbl[i];
        S_group_2_65_tbl[i].pExtraInfo = &S_group_2_65_extra_tbl[i];
        S_group_2_66_tbl[i].pExtraInfo = &S_group_2_66_extra_tbl[i];
        S_group_2_67_tbl[i].pExtraInfo = &S_group_2_67_extra_tbl[i];
        S_group_2_68_tbl[i].pExtraInfo = &S_group_2_68_extra_tbl[i];
        S_group_2_69_tbl[i].pExtraInfo = &S_group_2_69_extra_tbl[i];
        S_group_2_6a_tbl[i].pExtraInfo = &S_group_2_6a_extra_tbl[i];
        S_group_2_6b_tbl[i].pExtraInfo = &S_group_2_6b_extra_tbl[i];
        S_group_2_6c_tbl[i].pExtraInfo = &S_group_2_6c_extra_tbl[i];
        S_group_2_6d_tbl[i].pExtraInfo = &S_group_2_6d_extra_tbl[i];
        S_group_2_6e_tbl[i].pExtraInfo = &S_group_2_6e_extra_tbl[i];
        S_group_2_6f_tbl[i].pExtraInfo = &S_group_2_6f_extra_tbl[i];
        S_group_2_70_tbl[i].pExtraInfo = &S_group_2_70_extra_tbl[i];
        S_group_2_71_02_tbl[i].pExtraInfo = &S_group_2_71_02_extra_tbl[i];
        S_group_2_71_04_tbl[i].pExtraInfo = &S_group_2_71_04_extra_tbl[i];
        S_group_2_71_06_tbl[i].pExtraInfo = &S_group_2_71_06_extra_tbl[i];
        S_group_2_72_02_tbl[i].pExtraInfo = &S_group_2_72_02_extra_tbl[i];
        S_group_2_72_04_tbl[i].pExtraInfo = &S_group_2_72_04_extra_tbl[i];
        S_group_2_72_06_tbl[i].pExtraInfo = &S_group_2_72_06_extra_tbl[i];
        S_group_2_73_02_tbl[i].pExtraInfo = &S_group_2_73_02_extra_tbl[i];
        S_group_2_73_03_tbl[i].pExtraInfo = &S_group_2_73_03_extra_tbl[i];
        S_group_2_73_06_tbl[i].pExtraInfo = &S_group_2_73_06_extra_tbl[i];
        S_group_2_73_07_tbl[i].pExtraInfo = &S_group_2_73_07_extra_tbl[i];
        S_group_2_74_tbl[i].pExtraInfo = &S_group_2_74_extra_tbl[i];
        S_group_2_75_tbl[i].pExtraInfo = &S_group_2_75_extra_tbl[i];
        S_group_2_76_tbl[i].pExtraInfo = &S_group_2_76_extra_tbl[i];
        S_group_2_7e_tbl[i].pExtraInfo = &S_group_2_7e_extra_tbl[i];
        S_group_2_7f_tbl[i].pExtraInfo = &S_group_2_7f_extra_tbl[i];
        S_group_2_c2_tbl[i].pExtraInfo = &S_group_2_c2_extra_tbl[i];
        S_group_2_c4_tbl[i].pExtraInfo = &S_group_2_c4_extra_tbl[i];
        S_group_2_c5_tbl[i].pExtraInfo = &S_group_2_c5_extra_tbl[i];
        S_group_2_c6_tbl[i].pExtraInfo = &S_group_2_c6_extra_tbl[i];
        S_group_2_d1_tbl[i].pExtraInfo = &S_group_2_d1_extra_tbl[i];
        S_group_2_d2_tbl[i].pExtraInfo = &S_group_2_d2_extra_tbl[i];
        S_group_2_d3_tbl[i].pExtraInfo = &S_group_2_d3_extra_tbl[i];
        S_group_2_d4_tbl[i].pExtraInfo = &S_group_2_d4_extra_tbl[i];
        S_group_2_d5_tbl[i].pExtraInfo = &S_group_2_d5_extra_tbl[i];
        S_group_2_d6_tbl[i].pExtraInfo = &S_group_2_d6_extra_tbl[i];
        S_group_2_d7_tbl[i].pExtraInfo = &S_group_2_d7_extra_tbl[i];
        S_group_2_d8_tbl[i].pExtraInfo = &S_group_2_d8_extra_tbl[i];
        S_group_2_d9_tbl[i].pExtraInfo = &S_group_2_d9_extra_tbl[i];
        S_group_2_da_tbl[i].pExtraInfo = &S_group_2_da_extra_tbl[i];
        S_group_2_db_tbl[i].pExtraInfo = &S_group_2_db_extra_tbl[i];
        S_group_2_dc_tbl[i].pExtraInfo = &S_group_2_dc_extra_tbl[i];
        S_group_2_dd_tbl[i].pExtraInfo = &S_group_2_dd_extra_tbl[i];
        S_group_2_de_tbl[i].pExtraInfo = &S_group_2_de_extra_tbl[i];
        S_group_2_df_tbl[i].pExtraInfo = &S_group_2_df_extra_tbl[i];
        S_group_2_e0_tbl[i].pExtraInfo = &S_group_2_e0_extra_tbl[i];
        S_group_2_e1_tbl[i].pExtraInfo = &S_group_2_e1_extra_tbl[i];
        S_group_2_e2_tbl[i].pExtraInfo = &S_group_2_e2_extra_tbl[i];
        S_group_2_e3_tbl[i].pExtraInfo = &S_group_2_e3_extra_tbl[i];
        S_group_2_e4_tbl[i].pExtraInfo = &S_group_2_e4_extra_tbl[i];
        S_group_2_e5_tbl[i].pExtraInfo = &S_group_2_e5_extra_tbl[i];
        S_group_2_e6_tbl[i].pExtraInfo = &S_group_2_e6_extra_tbl[i];
        S_group_2_e7_tbl[i].pExtraInfo = &S_group_2_e7_extra_tbl[i];
        S_group_2_e8_tbl[i].pExtraInfo = &S_group_2_e8_extra_tbl[i];
        S_group_2_e9_tbl[i].pExtraInfo = &S_group_2_e9_extra_tbl[i];
        S_group_2_ea_tbl[i].pExtraInfo = &S_group_2_ea_extra_tbl[i];
        S_group_2_eb_tbl[i].pExtraInfo = &S_group_2_eb_extra_tbl[i];
        S_group_2_ec_tbl[i].pExtraInfo = &S_group_2_ec_extra_tbl[i];
        S_group_2_ed_tbl[i].pExtraInfo = &S_group_2_ed_extra_tbl[i];
        S_group_2_ee_tbl[i].pExtraInfo = &S_group_2_ee_extra_tbl[i];
        S_group_2_ef_tbl[i].pExtraInfo = &S_group_2_ef_extra_tbl[i];
        S_group_2_f1_tbl[i].pExtraInfo = &S_group_2_f1_extra_tbl[i];
        S_group_2_f2_tbl[i].pExtraInfo = &S_group_2_f2_extra_tbl[i];
        S_group_2_f3_tbl[i].pExtraInfo = &S_group_2_f3_extra_tbl[i];
        S_group_2_f4_tbl[i].pExtraInfo = &S_group_2_f4_extra_tbl[i];
        S_group_2_f5_tbl[i].pExtraInfo = &S_group_2_f5_extra_tbl[i];
        S_group_2_f6_tbl[i].pExtraInfo = &S_group_2_f6_extra_tbl[i];
        S_group_2_f7_tbl[i].pExtraInfo = &S_group_2_f7_extra_tbl[i];
        S_group_2_f8_tbl[i].pExtraInfo = &S_group_2_f8_extra_tbl[i];
        S_group_2_f9_tbl[i].pExtraInfo = &S_group_2_f9_extra_tbl[i];
        S_group_2_fa_tbl[i].pExtraInfo = &S_group_2_fa_extra_tbl[i];
        S_group_2_fb_tbl[i].pExtraInfo = &S_group_2_fb_extra_tbl[i];
        S_group_2_fc_tbl[i].pExtraInfo = &S_group_2_fc_extra_tbl[i];
        S_group_2_fd_tbl[i].pExtraInfo = &S_group_2_fd_extra_tbl[i];
        S_group_2_fe_tbl[i].pExtraInfo = &S_group_2_fe_extra_tbl[i];
        // end of new added
    }
}

void CDisasmwrapper::AssignTableContent()
{

    CInstr_Table S_oneByteOpcodes_table[256] =
    {
        //           Mnemonic NumOperands OpField1  OpField2      OpField3    InstSpecies   ProcessorType
        /* 0x00 */  {      "add",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evAddSpecies,     K6},
        /* 0x01 */  {      "add",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evAddSpecies,     K6},
        /* 0x02 */  {      "add",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evAddSpecies,     K6},
        /* 0x03 */  {      "add",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evAddSpecies,     K6},
        /* 0x04 */  {      "add",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evAddSpecies,     K6},
        /* 0x05 */  {      "add",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evAddSpecies,     K6},
        /* 0x06 */  {     "push",  2, {_ESP, w, WW},  {ES, w, nA},    {na, NA, nA}, evPushSpecies,    K6},
        /* 0x07 */  {      "pop",  2, {ES, w, nA},    {_ESP, NA, RR}, {na, NA, nA}, evPopSpecies,     K6},
        /* 0x08 */  {       "or",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evPopSpecies,     K6},
        /* 0x09 */  {       "or",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evOrSpecies,      K6},
        /* 0x0a */  {       "or",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evOrSpecies,      K6},
        /* 0x0b */  {       "or",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evOrSpecies,      K6},
        /* 0x0c */  {       "or",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evOrSpecies,      K6},
        /* 0x0d */  {       "or",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evOrSpecies,      K6},
        /* 0x0e */  {     "push",  2, {_ESP, w, WW},  {CS, w, nA},    {na, NA, nA}, evPushSpecies,    K6},
        /* 0x0f */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x10 */  {      "adc",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evAdcSpecies,     K6},
        /* 0x11 */  {      "adc",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evAdcSpecies,     K6},
        /* 0x12 */  {      "adc",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evAdcSpecies,     K6},
        /* 0x13 */  {      "adc",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evAdcSpecies,     K6},
        /* 0x14 */  {      "adc",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evAdcSpecies,     K6},
        /* 0x15 */  {      "adc",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evAdcSpecies,     K6},
        /* 0x16 */  {     "push",  2, {_ESP, w, WW},  {SS, w, nA},    {na, NA, nA}, evPushSpecies,    K6},
        /* 0x17 */  {      "pop",  2, {SS, w, nA},    {_ESP, NA, RR}, {na, NA, nA}, evPopSpecies,     K6},
        /* 0x18 */  {      "sbb",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evSbbSpecies,     K6},
        /* 0x19 */  {      "sbb",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evSbbSpecies,     K6},
        /* 0x1a */  {      "sbb",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evSbbSpecies,     K6},
        /* 0x1b */  {      "sbb",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evSbbSpecies,     K6},
        /* 0x1c */  {      "sbb",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evSbbSpecies,     K6},
        /* 0x1d */  {      "sbb",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evSbbSpecies,     K6},
        /* 0x1e */  {     "push",  2, {_ESP, w, WW},  {DS, w, nA},    {na, NA, nA}, evPushSpecies,    K6},
        /* 0x1f */  {      "pop",  2, {DS, w, nA},    {_ESP, NA, RR}, {na, NA, nA}, evPopSpecies,     K6},
        /* 0x20 */  {      "and",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evAndSpecies,     K6},
        /* 0x21 */  {      "and",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evAndSpecies,     K6},
        /* 0x22 */  {      "and",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evAndSpecies,     K6},
        /* 0x23 */  {      "and",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evAndSpecies,     K6},
        /* 0x24 */  {      "and",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evAndSpecies,     K6},
        /* 0x25 */  {      "and",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evAndSpecies,     K6},
        /* 0x26 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x27 */  {      "daa",  1, {_AL, b, nA},   {na, NA, nA},   {na, NA, nA}, evDaaSpecies,     K6},
        /* 0x28 */  {      "sub",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evSubSpecies,     K6},
        /* 0x29 */  {      "sub",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evSubSpecies,     K6},
        /* 0x2a */  {      "sub",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evSubSpecies,     K6},
        /* 0x2b */  {      "sub",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evSubSpecies,     K6},
        /* 0x2c */  {      "sub",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evSubSpecies,     K6},
        /* 0x2d */  {      "sub",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evSubSpecies,     K6},
        /* 0x2e */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x2f */  {      "das",  1, {_AL, b, nA},   {na, NA, nA},   {na, NA, nA}, evDasSpecies,     K6},
        /* 0x30 */  {      "xor",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evXorSpecies,     K6},
        /* 0x31 */  {      "xor",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evXorSpecies,     K6},
        /* 0x32 */  {      "xor",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evXorSpecies,     K6},
        /* 0x33 */  {      "xor",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evXorSpecies,     K6},
        /* 0x34 */  {      "xor",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evXorSpecies,     K6},
        /* 0x35 */  {      "xor",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evXorSpecies,     K6},
        /* 0x36 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x37 */  {      "aaa",  1, {_AL, b, nA},   {na, NA, nA},   {na, NA, nA}, evAaaSpecies,     K6},
        /* 0x38 */  {      "cmp",  2, {E, b, RR},     {G, b, nA},     {na, NA, nA}, evCmpSpecies,     K6},
        /* 0x39 */  {      "cmp",  2, {E, v, RR},     {G, v, nA},     {na, NA, nA}, evCmpSpecies,     K6},
        /* 0x3a */  {      "cmp",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evCmpSpecies,     K6},
        /* 0x3b */  {      "cmp",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmpSpecies,     K6},
        /* 0x3c */  {      "cmp",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evCmpSpecies,     K6},
        /* 0x3d */  {      "cmp",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evCmpSpecies,     K6},
        /* 0x3e */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x3f */  {      "aas",  1, {_AL, b, nA},   {na, NA, nA},   {na, NA, nA}, evAasSpecies,     K6},
        /* 0x40 */  {      "inc",  1, {eAX, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x41 */  {      "inc",  1, {eCX, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x42 */  {      "inc",  1, {eDX, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x43 */  {      "inc",  1, {eBX, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x44 */  {      "inc",  1, {eSP, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x45 */  {      "inc",  1, {eBP, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x46 */  {      "inc",  1, {eSI, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x47 */  {      "inc",  1, {eDI, v, nA},   {na, NA, nA},   {na, NA, nA}, evIncSpecies,     K6},
        /* 0x48 */  {      "dec",  1, {eAX, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x49 */  {      "dec",  1, {eCX, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x4a */  {      "dec",  1, {eDX, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x4b */  {      "dec",  1, {eBX, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x4c */  {      "dec",  1, {eSP, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x4d */  {      "dec",  1, {eBP, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x4e */  {      "dec",  1, {eSI, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x4f */  {      "dec",  1, {eDI, v, nA},   {na, NA, nA},   {na, NA, nA}, evDecSpecies,     K6},
        /* 0x50 */  {     "push",  2, {_ESP, v, WW},  {eAX, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x51 */  {     "push",  2, {_ESP, v, WW},  {eCX, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x52 */  {     "push",  2, {_ESP, v, WW},  {eDX, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x53 */  {     "push",  2, {_ESP, v, WW},  {eBX, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x54 */  {     "push",  2, {_ESP, v, WW},  {eSP, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x55 */  {     "push",  2, {_ESP, v, WW},  {eBP, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x56 */  {     "push",  2, {_ESP, v, WW},  {eSI, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x57 */  {     "push",  2, {_ESP, v, WW},  {eDI, v, nA},   {na, NA, nA}, evPushSpecies,    K6},
        /* 0x58 */  {      "pop",  2, {eAX, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x59 */  {      "pop",  2, {eCX, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x5a */  {      "pop",  2, {eDX, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x5b */  {      "pop",  2, {eBX, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x5c */  {      "pop",  2, {eSP, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x5d */  {      "pop",  2, {eBP, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x5e */  {      "pop",  2, {eSI, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x5f */  {      "pop",  2, {eDI, v, nA},   {_ESP, v, RR},  {na, NA, nA}, evPopSpecies,     K6},
        /* 0x60 */  {  "_pushad",  1, {_eSP, v, WW},  {na, NA, nA},   {na, NA, nA}, evPushSpecies,    K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x61 */  {   "_popad",  1, {_eSP, v, RR},  {na, NA, nA},   {na, NA, nA}, evPopSpecies,     K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x62 */  {    "bound",  2, {G, v, nA},     {M, v, RR},     {na, NA, nA}, evBoundSpecies,   K6}, // Intel book says GvMa, but GvMv should do!   TODO: Reads memory twice.
        /* 0x63 */  {     "arpl",  2, {E, w, RW},     {G, w, nA},     {na, NA, nA}, evArplSpecies,    K6},
        /* 0x64 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x65 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x66 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x67 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x68 */  {     "push",  2, {_ESP, v, WW},  {I, v, nA},     {na, NA, nA}, evPushSpecies,    K6},
        /* 0x69 */  {     "imul",  3, {G, v, nA},     {E, v, RR},     {I, v, nA},   evImulSpecies,    K6},
        /* 0x6a */  {     "push",  2, {_ESP, b, WW},  {I, b, nA},     {na, NA, nA}, evPushSpecies,    K6},
        /* 0x6b */  {     "imul",  3, {G, v, nA},     {E, v, RR},     {I, ib, nA},  evImulSpecies,    K6},
        /* 0x6c */  {     "insb",  2, {Y, b, WW},     {_DX, b, nA},   {na, NA, nA}, evInsbSpecies,    K6},
        /* 0x6d */  {    "_insd",  2, {Y, v, WW},     {_DX, v, nA},   {na, NA, nA}, evInsbSpecies,    K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x6e */  {    "outsb",  2, {_DX, b, nA},   {X, b, RR},     {na, NA, nA}, evOutsbSpecies,   K6},
        /* 0x6f */  {   "_outsd",  2, {_DX, v, nA},   {X, v, RR},     {na, NA, nA}, evOutsdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x70 */  {       "jo",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJoSpecies,      K6},
        /* 0x71 */  {      "jno",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnoSpecies,     K6},
        /* 0x72 */  {       "jb",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJbSpecies,      K6},
        /* 0x73 */  {      "jnb",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnbSpecies,     K6},
        /* 0x74 */  {       "jz",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJzSpecies,      K6},
        /* 0x75 */  {      "jnz",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnzSpecies,     K6},
        /* 0x76 */  {      "jbe",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJbeSpecies,     K6},
        /* 0x77 */  {     "jnbe",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnbeSpecies,    K6},
        /* 0x78 */  {       "js",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJsSpecies,      K6},
        /* 0x79 */  {      "jns",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnsSpecies,     K6},
        /* 0x7a */  {       "jp",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJpSpecies,      K6},
        /* 0x7b */  {      "jnp",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnpSpecies,     K6},
        /* 0x7c */  {       "jl",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJlSpecies,      K6},
        /* 0x7d */  {      "jnl",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnlSpecies,     K6},
        /* 0x7e */  {      "jle",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJleSpecies,     K6},
        /* 0x7f */  {     "jnle",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJnleSpecies,    K6},
        /* 0x80 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x81 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x82 */  {     "movb",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evMovbSpecies,    K6}, // TODO: Intel books say this is reserved.  AMD opcode map doesn't show it at all!!!
        /* 0x83 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0x84 */  {     "test",  2, {E, b, RR},     {G, b, nA},     {na, NA, nA}, evTestSpecies,    K6},
        /* 0x85 */  {     "test",  2, {E, v, RR},     {G, v, nA},     {na, NA, nA}, evTestSpecies,    K6},
        /* 0x86 */  {     "xchg",  2, {G, b, nA},     {E, b, RW},     {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x87 */  {     "xchg",  2, {G, v, nA},     {E, v, RW},     {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x88 */  {      "mov",  2, {E, b, WW},     {G, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0x89 */  {      "mov",  2, {E, v, WW},     {G, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0x8a */  {      "mov",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0x8b */  {      "mov",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0x8c */  {      "mov",  2, {E, w, WW},     {S, w, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0x8d */  {      "lea",  2, {G, v, nA},     {M, v, RR},     {na, NA, nA}, evLeaSpecies,     K6},
        /* 0x8e */  {      "mov",  2, {S, w, nA},     {E, w, RR},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0x8f */  {      "pop",  2, {E, v, WW},     {_ESP, NA, RR}, {na, NA, nA}, evPopSpecies,     K6},
        /* 0x90 */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,     K6}, // This is an alias mnemonic for "XCHG (E)AX, (E)AX"
        /* 0x91 */  {     "xchg",  2, {eCX, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x92 */  {     "xchg",  2, {eDX, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x93 */  {     "xchg",  2, {eBX, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x94 */  {     "xchg",  2, {eSP, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x95 */  {     "xchg",  2, {eBP, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x96 */  {     "xchg",  2, {eSI, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x97 */  {     "xchg",  2, {eDI, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies,    K6},
        /* 0x98 */  {    "_cwde",  1, {eAX, v, nA},   {na, NA, nA},   {na, NA, nA}, evCwdeSpecies,    K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x99 */  {     "_cdq",  2, {_eDX, v, nA},  {_eAX, v, nA},  {na, NA, nA}, evCdqSpecies,     K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x9a */  {     "call",  1, {A, p, nA},     {na, NA, nA},   {na, NA, nA}, evCallSpecies,    K6}, // TODO: Verify NA access mode
        /* 0x9b */  {     "wait",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evWaitSpecies,    K6},
        /* 0x9c */  {  "_pushfd",  1, {_eSP, v, WW},  {na, NA, nA},   {na, NA, nA}, evPushfdSpecies,  K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x9d */  {   "_popfd",  1, {_eSP, v, RR},  {na, NA, nA},   {na, NA, nA}, evPopfdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0x9e */  {     "sahf",  1, {_AH, b, nA},   {na, NA, nA},   {na, NA, nA}, evSahfSpecies,    K6},
        /* 0x9f */  {     "lahf",  1, {_AH, b, nA},   {na, NA, nA},   {na, NA, nA}, evLahfSpecies,    K6},
        /* 0xa0 */  {      "mov",  2, {AL, b, nA},    {O, b, RR},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xa1 */  {      "mov",  2, {eAX, v, nA},   {O, v, RR},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xa2 */  {      "mov",  2, {O, b, WW},     {AL, b, nA},    {na, NA, nA}, evMovSpecies,     K6},
        /* 0xa3 */  {      "mov",  2, {O, v, WW},     {eAX, v, nA},   {na, NA, nA}, evMovSpecies,     K6},
        /* 0xa4 */  {    "movsb",  2, {X, b, WW},     {Y, b, RR},     {na, NA, nA}, evMovsbSpecies,   K6},
        /* 0xa5 */  {   "_movsd",  2, {X, v, WW},     {Y, v, RR},     {na, NA, nA}, evMovsdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0xa6 */  {    "cmpsb",  2, {X, b, RR},     {Y, b, RR},     {na, NA, nA}, evCmpsbSpecies,   K6},
        /* 0xa7 */  {   "_cmpsd",  2, {X, v, RR},     {Y, v, RR},     {na, NA, nA}, evCmpsdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0xa8 */  {     "test",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evTestSpecies,    K6},
        /* 0xa9 */  {     "test",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evTestSpecies,    K6},
        /* 0xaa */  {    "stosb",  2, {Y, b, WW},     {_AL, b, nA},   {na, NA, nA}, evStosbSpecies,   K6},
        /* 0xab */  {   "_stosd",  2, {Y, v, WW},     {_eAX, v, nA},  {na, NA, nA}, evStosdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0xac */  {    "lodsb",  2, {_AL, b, nA},   {X, b, RR},     {na, NA, nA}, evLodsbSpecies,   K6},
        /* 0xad */  {   "_lodsd",  2, {_eAX, v, nA},  {X, v, RR},     {na, NA, nA}, evLodsdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0xae */  {    "scasb",  2, {_AL, b, nA},   {Y, b, RR},     {na, NA, nA}, evScasbSpecies,   K6},
        /* 0xaf */  {   "_scasd",  2, {_eAX, v, nA},  {Y, v, RR},     {na, NA, nA}, evScasdSpecies,   K6}, // TODO: Mnemonic depends on the operand size!
        /* 0xb0 */  {      "mov",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb1 */  {      "mov",  2, {CL, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb2 */  {      "mov",  2, {DL, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb3 */  {      "mov",  2, {BL, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb4 */  {      "mov",  2, {AH, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb5 */  {      "mov",  2, {CH, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb6 */  {      "mov",  2, {DH, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb7 */  {      "mov",  2, {BH, b, nA},    {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb8 */  {      "mov",  2, {eAX, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xb9 */  {      "mov",  2, {eCX, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xba */  {      "mov",  2, {eDX, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xbb */  {      "mov",  2, {eBX, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xbc */  {      "mov",  2, {eSP, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xbd */  {      "mov",  2, {eBP, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xbe */  {      "mov",  2, {eSI, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xbf */  {      "mov",  2, {eDI, v, nA},   {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xc0 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xc1 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xc2 */  {      "ret",  2, {I, w, nA},     {_ESP, w, RR},  {na, NA, nA}, evRetSpecies,     K6}, //TODO: Should ESP be the 1st or 2nd operand?
        /* 0xc3 */  {      "ret",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evRetSpecies,     K6},
        /* 0xc4 */  {      "les",  3, {_ES, v, nA},   {G, v, nA},     {M, p, RR},   evLesSpecies,     K6},
        /* 0xc5 */  {      "lds",  3, {_DS, v, nA},   {G, v, nA},     {M, p, RR},   evLdsSpecies,     K6},
        /* 0xc6 */  {      "mov",  2, {E, b, WW},     {I, b, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xc7 */  {      "mov",  2, {E, v, WW},     {I, v, nA},     {na, NA, nA}, evMovSpecies,     K6},
        /* 0xc8 */  {    "enter",  3, {_ESP, w, WW},  {I, w, nA},     {I, ib, nA},  evEnterSpecies,   K6},
        /* 0xc9 */  {    "leave",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evLeaveSpecies,   K6},
        /* 0xca */  {      "ret",  2, {I, w, nA},     {_ESP, w, RR},  {na, NA, nA}, evRetSpecies,     K6},
        /* 0xcb */  {      "ret",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evRetSpecies,     K6},
        /* 0xcc */  {    "int 3",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evInt3Species,    K6},
        /* 0xcd */  {      "int",  1, {I, b, nA},     {na, NA, nA},   {na, NA, nA}, evIntSpecies,     K6},
        /* 0xce */  {     "into",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evIntoSpecies,    K6},
        /* 0xcf */  {     "iret",  1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA}, evIretSpecies,    K6},
        /* 0xd0 */  {          "", 0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xd1 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xd2 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xd3 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xd4 */  {      "aam",  2, {_AX, w, nA},   {I, b, nA},     {na, NA, nA}, evAamSpecies,     K6}, //TODO: Intel manual says no operands
        /* 0xd5 */  {      "aad",  2, {_AX, w, nA},   {I, b, nA},     {na, NA, nA}, evAadSpecies,     K6}, //TODO: Intel manual says no operands
        /* 0xd6 */  {     "salc",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evSalcSpecies,    K6},
        /* 0xd7 */  {    "xlatb",  2, {_EBXAL, b, RR}, {_AL, b, nA},   {na, NA, nA}, evXlatbSpecies,   K6}, //The XLAT instruction implicitely accesses memory location DS:[(R/E)BX + AL]
        /* 0xd8 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xd9 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xda */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xdb */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xdc */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xdd */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xde */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xdf */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xe0 */  {   "loopne",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evLoopneSpecies,  K6},
        /* 0xe1 */  {    "loope",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evLoopeSpecies,   K6},
        /* 0xe2 */  {     "loop",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evLoopSpecies,    K6},
        /* 0xe3 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xe4 */  {       "in",  2, {AL, b, nA},    {I, b, nA},     {na, NA, nA}, evInSpecies,      K6},
        /* 0xe5 */  {       "in",  2, {eAX, v, nA},   {I, b, nA},     {na, NA, nA}, evInSpecies,      K6},
        /* 0xe6 */  {      "out",  2, {I, b, nA},     {AL, b, nA},    {na, NA, nA}, evOutSpecies,     K6}, /* Ib instead of PORT */
        /* 0xe7 */  {      "out",  2, {I, b, nA},     {eAX, v, nA},   {na, NA, nA}, evOutSpecies,     K6}, /* Ib instead of PORT */
        /* 0xe8 */  {     "call",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evCallSpecies,    K6},
        /* 0xe9 */  {      "jmp",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJmpSpecies,     K6},
        /* 0xea */  {      "jmp",  1, {A, p, nA},     {na, NA, nA},   {na, NA, nA}, evJmpSpecies,     K6},
        /* 0xeb */  {      "jmp",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJmpSpecies,     K6},
        /* 0xec */  {       "in",  2, {AL, b, nA},    {DX, b, nA},    {na, NA, nA}, evInSpecies,      K6},
        /* 0xed */  {       "in",  2, {eAX, v, nA},   {DX, v, nA},    {na, NA, nA}, evInSpecies,      K6},
        /* 0xee */  {      "out",  2, {DX, b, nA},    {AL, b, nA},    {na, NA, nA}, evOutSpecies,     K6}, /* DX instead of PORT */
        /* 0xef */  {      "out",  2, {DX, v, nA},    {eAX, v, nA},   {na, NA, nA}, evOutSpecies,     K6}, /* DX instead of PORT */
        /* 0xf0 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xf1 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xf2 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xf3 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xf4 */  {      "hlt",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evHltSpecies,     K6},
        /* 0xf5 */  {      "cmc",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evCmcSpecies,     K6},
        /* 0xf6 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xf7 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xf8 */  {      "clc",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evClcSpecies,     K6},
        /* 0xf9 */  {      "stc",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evStcSpecies,     K6},
        /* 0xfa */  {      "cli",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evCliSpecies,     K6},
        /* 0xfb */  {      "sti",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evStiSpecies,     K6},
        /* 0xfc */  {      "cld",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evCldSpecies,     K6},
        /* 0xfd */  {      "std",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evStdSpecies,     K6},
        /* 0xfe */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
        /* 0xff */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,      K6},
    };


    CInstr_Table S_twoByteOpcodes_table[256] =
    {
        //                Mnemonic NumOperands OpField1  OpField2      OpField3    InstSpecies  ProcessorType
        /* 0x0f 0x00 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x01 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x02 */  {      "lar",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evLarSpecies,    K6},
        /* 0x0f 0x03 */  {      "lsl",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evLslSpecies,    K6},
        /* 0x0f 0x04 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x05 */  {  "syscall",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evSyscallSpecies, K6},
        /* 0x0f 0x06 */  {     "clts",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evCltsSpecies,   K6},
        /* 0x0f 0x07 */  {   "sysret",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evSysretSpecies, K6},
        /* 0x0f 0x08 */  {     "invd",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evInvdSpecies,   K6},
        /* 0x0f 0x09 */  {   "wbinvd",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evWbinvdSpecies, K6},
        /* 0x0f 0x0a */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x0b */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x0c */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x0d */  {         "",  1, {M, v, RR},     {na, NA, nA},   {na, NA, nA}, evPrefetchSpecies,   K6_2},
        /* 0x0f 0x0e */  {    "femms",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evFemmsSpecies,  K6},
        /* 0x0f 0x0f */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x10 */  {      "mov",  2, {E, b, WW},     {G, b, nA},     {na, NA, nA}, evMovSpecies,    K6}, // TODO: I cannot find this instr in any opcode table!!!
        /* 0x0f 0x11 */  {      "mov",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evMovSpecies,    K6}, // TODO: I cannot find this instr in any opcode table!!!
        /* 0x0f 0x12 */  {      "mov",  2, {G, b, nA},     {E, b, RR},     {na, NA, nA}, evMovSpecies,    K6}, // TODO: I cannot find this instr in any opcode table!!!
        /* 0x0f 0x13 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x14 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x15 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x16 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x17 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K7},
        /* 0x0f 0x18 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K7},
        /* 0x0f 0x19 */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x1a */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x1b */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x1c */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x1d */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x1e */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x1f */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies,    K7},
        /* 0x0f 0x20 */  {      "mov",  2, {R, d, nA},     {C, d, nA},     {na, NA, nA}, evMovSpecies,    K6},
        /* 0x0f 0x21 */  {      "mov",  2, {R, d, nA},     {D, d, nA},     {na, NA, nA}, evMovSpecies,    K6},
        /* 0x0f 0x22 */  {      "mov",  2, {C, d, nA},     {R, d, nA},     {na, NA, nA}, evMovSpecies,    K6},
        /* 0x0f 0x23 */  {      "mov",  2, {D, d, nA},     {R, d, nA},     {na, NA, nA}, evMovSpecies,    K6},
        /* 0x0f 0x24 */  {      "mov",  2, {R, d, nA},     {T, d, nA},     {na, NA, nA}, evMovSpecies,    K6},
        /* 0x0f 0x25 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x26 */  {      "mov",  2, {T, d, nA},     {R, d, nA},     {na, NA, nA}, evMovSpecies,    K6},
        /* 0x0f 0x27 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x28 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x29 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x2a */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x2b */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x2c */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x2d */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x2e */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x2f */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        /* 0x0f 0x30 */  {    "wrmsr",  3, {_EDX, q, nA},  {_EAX, q, nA},  {_ECX, q, nA}, evWrmsrSpecies,      K6},
        /* 0x0f 0x31 */  {    "rdtsc",  2, {_EDX, q, nA},  {_EAX, q, nA},  {na, NA, nA}, evRdtscSpecies,      K6},
        /* 0x0f 0x32 */  {    "rdmsr",  3, {_EDX, q, nA},  {_EAX, q, nA},  {_ECX, q, nA}, evRdmsrSpecies,      K6},
        /* 0x0f 0x33 */  {    "rdpmc",  3, {_EDX, d, nA},  {_EAX, d, nA},  {_ECX, d, nA}, evRdpmcSpecies,      K6},
        /* 0x0f 0x34 */  { "sysenter",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evSysenterSpecies,   K7},
        /* 0x0f 0x35 */  {  "sysexit",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evSysexitSpecies,    K7},
        /* 0x0f 0x36 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x37 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x38 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x39 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x3a */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x3b */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x3c */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x3d */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x3e */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x3f */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x40 */  {    "cmovo",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovoSpecies,      K7},
        /* 0x0f 0x41 */  {   "cmovno",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovnoSpecies,     K7},
        /* 0x0f 0x42 */  {    "cmovb",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovbSpecies,      K7},
        /* 0x0f 0x43 */  {   "cmovae",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovaeSpecies,     K7},
        /* 0x0f 0x44 */  {    "cmove",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmoveSpecies,      K7},
        /* 0x0f 0x45 */  {   "cmovne",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovneSpecies,     K7},
        /* 0x0f 0x46 */  {   "cmovbe",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovbeSpecies,     K7},
        /* 0x0f 0x47 */  {    "cmova",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovaSpecies,      K7},
        /* 0x0f 0x48 */  {    "cmovs",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovsSpecies,      K7},
        /* 0x0f 0x49 */  {   "cmovns",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovnsSpecies,     K7},
        /* 0x0f 0x4a */  {    "cmovp",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovpSpecies,      K7},
        /* 0x0f 0x4b */  {   "cmovnp",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovnpSpecies,     K7},
        /* 0x0f 0x4c */  {    "cmovl",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovlSpecies,      K7},
        /* 0x0f 0x4d */  {   "cmovge",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovgeSpecies,     K7},
        /* 0x0f 0x4e */  {   "cmovle",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovleSpecies,     K7},
        /* 0x0f 0x4f */  {    "cmovg",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evCmovgSpecies,      K7},
        /* 0x0f 0x50 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x51 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x52 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x53 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x54 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x55 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x56 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x57 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x58 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x59 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x5a */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x5b */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x5c */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x5d */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x5e */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x5f */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x60 */  {"punpcklbw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPunpcklbwSpecies,  K6},
        /* 0x0f 0x61 */  {"punpcklwd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPunpcklwdSpecies,  K6},
        /* 0x0f 0x62 */  {"punpckldq",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPunpckldqSpecies,  K6},
        /* 0x0f 0x63 */  { "packsswb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPacksswbSpecies,   K6},
        /* 0x0f 0x64 */  {  "pcmpgtb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPcmpgtbSpecies,    K6},
        /* 0x0f 0x65 */  {  "pcmpgtw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPcmpgtwSpecies,    K6},
        /* 0x0f 0x66 */  {  "pcmpgtd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPcmpgtdSpecies,    K6},
        /* 0x0f 0x67 */  { "packuswb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPackuswbSpecies,   K6},
        /* 0x0f 0x68 */  {"punpckhbw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPunpckhbwSpecies,  K6},
        /* 0x0f 0x69 */  {"punpckhwd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPunpckhwdSpecies,  K6},
        /* 0x0f 0x6a */  {"punpckhdq",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPunpckhdqSpecies,  K6},
        /* 0x0f 0x6b */  { "packssdw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPackssdwSpecies,   K6},
        /* 0x0f 0x6c */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x6d */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x6e */  {     "movd",  2, {P, q, nA},     {E, d, RR},     {na, NA, nA}, evMovdSpecies,       K6},
        /* 0x0f 0x6f */  {     "movq",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evMovqSpecies,       K6},
        /* 0x0f 0x70 */  {   "pshufw",  3, {P, q, nA},     {Q, q, RR},     {I, ib, nA},  evPshufwSpecies,     K7},
        /* 0x0f 0x71 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x72 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x73 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x74 */  {  "pcmpeqb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPcmpeqbSpecies,    K6},
        /* 0x0f 0x75 */  {  "pcmpeqw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPcmpeqwSpecies,    K6},
        /* 0x0f 0x76 */  {  "pcmpeqd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPcmpeqdSpecies,    K6},
        /* 0x0f 0x77 */  {     "emms",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evEmmsSpecies,       K6},
        /* 0x0f 0x78 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x79 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x7a */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x7b */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x7c */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x7d */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0x7e */  {     "movd",  2, {E, d, WW},     {P, q, nA},     {na, NA, nA}, evMovdSpecies,       K6},
        /* 0x0f 0x7f */  {     "movq",  2, {Q, q, WW},     {P, q, nA},     {na, NA, nA}, evMovqSpecies,       K6},
        /* 0x0f 0x80 */  {       "jo",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJoSpecies,         K6},
        /* 0x0f 0x81 */  {      "jno",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnoSpecies,        K6},
        /* 0x0f 0x82 */  {       "jb",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJbSpecies,         K6},
        /* 0x0f 0x83 */  {      "jnb",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnbSpecies,        K6},
        /* 0x0f 0x84 */  {       "jz",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJzSpecies,         K6},
        /* 0x0f 0x85 */  {      "jnz",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnzSpecies,        K6},
        /* 0x0f 0x86 */  {      "jbe",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJbeSpecies,        K6},
        /* 0x0f 0x87 */  {     "jnbe",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnbeSpecies,       K6},
        /* 0x0f 0x88 */  {       "js",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJsSpecies,         K6},
        /* 0x0f 0x89 */  {      "jns",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnsSpecies,        K6},
        /* 0x0f 0x8a */  {       "jp",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJpSpecies,         K6},
        /* 0x0f 0x8b */  {      "jnp",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnpSpecies,        K6},
        /* 0x0f 0x8c */  {       "jl",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJlSpecies,         K6},
        /* 0x0f 0x8d */  {      "jnl",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnlSpecies,        K6},
        /* 0x0f 0x8e */  {      "jle",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJleSpecies,        K6},
        /* 0x0f 0x8f */  {     "jnle",  1, {J, v, nA},     {na, NA, nA},   {na, NA, nA}, evJnleSpecies,       K6},
        /* 0x0f 0x90 */  {     "seto",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetoSpecies,       K6},
        /* 0x0f 0x91 */  {    "setno",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnoSpecies,      K6},
        /* 0x0f 0x92 */  {     "setb",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetbSpecies,       K6},
        /* 0x0f 0x93 */  {    "setnb",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnbSpecies,      K6},
        /* 0x0f 0x94 */  {     "setz",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetzSpecies,       K6},
        /* 0x0f 0x95 */  {    "setnz",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnzSpecies,      K6},
        /* 0x0f 0x96 */  {    "setbe",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetbeSpecies,      K6},
        /* 0x0f 0x97 */  {   "setnbe",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnbeSpecies,     K6},
        /* 0x0f 0x98 */  {     "sets",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetsSpecies,       K6},
        /* 0x0f 0x99 */  {    "setns",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnsSpecies,      K6},
        /* 0x0f 0x9a */  {     "setp",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetpSpecies,       K6},
        /* 0x0f 0x9b */  {    "setnp",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnpSpecies,      K6},
        /* 0x0f 0x9c */  {     "setl",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetlSpecies,       K6},
        /* 0x0f 0x9d */  {    "setnl",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnlSpecies,      K6},
        /* 0x0f 0x9e */  {    "setle",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetleSpecies,      K6},
        /* 0x0f 0x9f */  {   "setnle",  1, {E, b, WW},     {na, NA, nA},   {na, NA, nA}, evSetnleSpecies,     K6},
        /* 0x0f 0xa0 */  {     "push",  2, {_ESP, w, WW},  {FS, w, nA},    {na, NA, nA}, evPushSpecies,       K6},
        /* 0x0f 0xa1 */  {      "pop",  2, {FS, w, nA},    {_ESP, w, RR},  {na, NA, nA}, evPopSpecies,        K6},
        /* 0x0f 0xa2 */  {    "cpuid",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evCpuidSpecies,      K6},
        /* 0x0f 0xa3 */  {       "bt",  2, {E, v, RR},     {G, v, nA},     {na, NA, nA}, evBtSpecies,         K6},
        /* 0x0f 0xa4 */  {     "shld",  3, {E, v, RW},     {G, v, nA},     {I, ib, nA},  evShldSpecies,       K6},
        /* 0x0f 0xa5 */  {     "shld",  3, {E, v, RW},     {G, v, nA},     {CL, NA, nA}, evShldSpecies,       K6},
        /* 0x0f 0xa6 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xa7 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xa8 */  {     "push",  2, {_ESP, w, WW},  {GS, w, nA},    {na, NA, nA}, evPushSpecies,       K6},
        /* 0x0f 0xa9 */  {      "pop",  2, {GS, w, nA},    {_ESP, w, RR},  {na, NA, nA}, evPopSpecies,        K6},
        /* 0x0f 0xaa */  {      "rsm",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evRsmSpecies,        K6},
        /* 0x0f 0xab */  {      "bts",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evBtsSpecies,        K6},
        /* 0x0f 0xac */  {     "shrd",  3, {E, v, RW},     {G, v, nA},     {I, ib, nA},  evShrdSpecies,       K6},
        /* 0x0f 0xad */  {     "shrd",  3, {E, v, RW},     {G, v, nA},     {CL, NA, nA}, evShrdSpecies,       K6},
        /* 0x0f 0xae */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K7},
        /* 0x0f 0xaf */  {     "imul",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evImulSpecies,       K6},
        /* 0x0f 0xb0 */  {  "cmpxchg",  3, {_AL, b, nA},   {E, b, RW},     {G, b, nA},   evCmpxchgSpecies,    K6}, //TODO: What shdb the access type? TSIM requires to set type to RW
        /* 0x0f 0xb1 */  {  "cmpxchg",  3, {_EAX, v, nA},  {E, v, RW},     {G, v, nA},   evCmpxchgSpecies,    K6}, //TODO: What shdb the access type? TSIM requires to set type to RW
        /* 0x0f 0xb2 */  {      "lss",  3, {_SS, v, nA},   {G, v, nA},     {M, p, RR},   evLssSpecies,        K6}, // AMD opcode map says Mp, Intel's says Gv,Mp.  Intel's makes more sense.
        /* 0x0f 0xb3 */  {      "btr",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evBtrSpecies,        K6},
        /* 0x0f 0xb4 */  {      "lfs",  3, {_FS, v, nA},   {G, v, nA},     {M, p, RR},   evLfsSpecies,        K6}, // AMD opcode map says Mp, Intel's says Gv,Mp.  Intel's makes more sense.
        /* 0x0f 0xb5 */  {      "lgs",  3, {_GS, v, nA},   {G, v, nA},     {M, p, RR},   evLgsSpecies,        K6}, // AMD opcode map says Mp, Intel's says Gv,Mp.  Intel's makes more sense.
        /* 0x0f 0xb6 */  {    "movzx",  2, {G, v, nA},     {E, b, RR},     {na, NA, nA}, evMovzxSpecies,      K6},
        /* 0x0f 0xb7 */  {    "movzx",  2, {G, v, nA},     {E, w, RR},     {na, NA, nA}, evMovzxSpecies,      K6},
        /* 0x0f 0xb8 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xb9 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xba */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xbb */  {      "btc",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evBtcSpecies,        K6},
        /* 0x0f 0xbc */  {      "bsf",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evBsfSpecies,        K6},
        /* 0x0f 0xbd */  {      "bsr",  2, {G, v, nA},     {E, v, RR},     {na, NA, nA}, evBsrSpecies,        K6},
        /* 0x0f 0xbe */  {    "movsx",  2, {G, v, nA},     {E, b, RR},     {na, NA, nA}, evMovsxSpecies,      K6},
        /* 0x0f 0xbf */  {    "movsx",  2, {G, v, nA},     {E, w, RR},     {na, NA, nA}, evMovsxSpecies,      K6},
        /* 0x0f 0xc0 */  {     "xadd",  2, {E, b, RW},     {G, b, nA},     {na, NA, nA}, evXaddSpecies,       K6},
        /* 0x0f 0xc1 */  {     "xadd",  2, {E, v, RW},     {G, v, nA},     {na, NA, nA}, evXaddSpecies,       K6},
        /* 0x0f 0xc2 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xc3 */  {   "movnti",  2, {na, NA, WW},   {na, NA, nA},   {na, NA, nA}, evMovntiSpecies,     K7_2}, //TODO Operand addressing mode
        /* 0x0f 0xc4 */  {   "pinsrw",  3, {P, q, nA},     {E, dw, RR},    {I, ib, nA},  evPinsrwSpecies,     K7},
        /* 0x0f 0xc5 */  {   "pextrw",  3, {G, d, nA},     {R, q, nA},     {I, ib, nA},  evPextrwSpecies,     K7},
        /* 0x0f 0xc6 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xc7 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xc8 */  {    "bswap",  1, {EAX, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xc9 */  {    "bswap",  1, {ECX, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xca */  {    "bswap",  1, {EDX, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xcb */  {    "bswap",  1, {EBX, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xcc */  {    "bswap",  1, {ESP, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xcd */  {    "bswap",  1, {EBP, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xce */  {    "bswap",  1, {ESI, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xcf */  {    "bswap",  1, {EDI, d, nA},   {na, NA, nA},   {na, NA, nA}, evBswapSpecies,      K6},
        /* 0x0f 0xd0 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xd1 */  {    "psrlw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsrlwSpecies,      K6},
        /* 0x0f 0xd2 */  {    "psrld",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsrldSpecies,      K6},
        /* 0x0f 0xd3 */  {    "psrlq",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsrlqSpecies,      K6},
        /* 0x0f 0xd4 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xd5 */  {   "pmullw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPmullwSpecies,     K6},
        /* 0x0f 0xd6 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xd7 */  { "pmovmskb",  2, {G, d, nA},     {R, q, nA},     {na, NA, nA}, evPmovmskbSpecies,   K7},
        /* 0x0f 0xd8 */  {  "psubusb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubusbSpecies,    K6},
        /* 0x0f 0xd9 */  {  "psubusw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubuswSpecies,    K6},
        /* 0x0f 0xda */  {   "pminub",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPminubSpecies,     K7},
        /* 0x0f 0xdb */  {     "pand",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPandSpecies,       K6},
        /* 0x0f 0xdc */  {  "paddusb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPaddusbSpecies,    K6},
        /* 0x0f 0xdd */  {  "paddusw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPadduswSpecies,    K6},
        /* 0x0f 0xde */  {   "pmaxub",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPmaxubSpecies,     K7},
        /* 0x0f 0xdf */  {    "pandn",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPandnSpecies,      K6},
        /* 0x0f 0xe0 */  {    "pavgb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPavgbSpecies,      K7},
        /* 0x0f 0xe1 */  {    "psraw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsrawSpecies,      K6},
        /* 0x0f 0xe2 */  {    "psrad",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsradSpecies,      K6},
        /* 0x0f 0xe3 */  {    "pavgw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPavgwSpecies,      K7},
        /* 0x0f 0xe4 */  {  "pmulhuw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPmulhuwSpecies,    K7},
        /* 0x0f 0xe5 */  {   "pmulhw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPmulhwSpecies,     K6},
        /* 0x0f 0xe6 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xe7 */  {   "movntq",  2, {M, q, WW},     {P, q, nA},     {na, NA, nA}, evMovntqSpecies,     K7},
        /* 0x0f 0xe8 */  {   "psubsb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubsbSpecies,     K6},
        /* 0x0f 0xe9 */  {   "psubsw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubswSpecies,     K6},
        /* 0x0f 0xea */  {   "pminsw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPminswSpecies,     K7},
        /* 0x0f 0xeb */  {      "por",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPorSpecies,        K6},
        /* 0x0f 0xec */  {   "paddsb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPaddsbSpecies,     K6},
        /* 0x0f 0xed */  {   "paddsw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPaddswSpecies,     K6},
        /* 0x0f 0xee */  {   "pmaxsw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPmaxswSpecies,     K7},
        /* 0x0f 0xef */  {     "pxor",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPxorSpecies,       K6},
        /* 0x0f 0xf0 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xf1 */  {    "psllw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsllwSpecies,      K6},
        /* 0x0f 0xf2 */  {    "pslld",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPslldSpecies,      K6},
        /* 0x0f 0xf3 */  {    "psllq",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsllqSpecies,      K6},
        /* 0x0f 0xf4 */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xf5 */  {  "pmaddwd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPmaddwdSpecies,    K6},
        /* 0x0f 0xf6 */  {   "psadbw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsadbwSpecies,     K7},
        /* 0x0f 0xf7 */  { "maskmovq",  2, {P, q, nA},     {R, q, nA},     {na, NA, nA}, evMaskmovqSpecies,   K7}, // eDI is implied first operand here!   i.e. maskmovq EDI, MM1, MM2
        /* 0x0f 0xf8 */  {    "psubb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubbSpecies,      K6},
        /* 0x0f 0xf9 */  {    "psubw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubwSpecies,      K6},
        /* 0x0f 0xfa */  {    "psubd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPsubdSpecies,      K6},
        /* 0x0f 0xfb */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6},
        /* 0x0f 0xfc */  {    "paddb",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPaddbSpecies,      K6},
        /* 0x0f 0xfd */  {    "paddw",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPaddwSpecies,      K6},
        /* 0x0f 0xfe */  {    "paddd",  2, {P, q, nA},     {Q, q, RR},     {na, NA, nA}, evPadddSpecies,      K6},
        /* 0x0f 0xff */  {         "",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,         K6}
    };

    CInstr_Table S_group_1_60_table[3] =
    {
        {   "pusha",  1, {_SP, v, WW},   {na, NA, nA},   {na, NA, nA}, evPushaSpecies,    K6},
        {   "pushad", 1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA}, evPushadSpecies,   K6},
        {   "pushaq", 1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA}, evPushaqSpecies,   K8},
    };

    CInstr_Table S_group_1_61_table[3] =
    {
        {   "popa",  1, {_SP, v, RR},   {na, NA, nA},   {na, NA, nA}, evPopaSpecies,  K6},
        {  "popad",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evPopadSpecies, K6},
        {  "popaq",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evPopaqSpecies, K8}
    };

    CInstr_Table S_group_1_63_table[2] =
    {
        /* 0x63 */  {     "arpl",   2, {E, w, RW},    {G, w, nA},   {na, NA, nA}, evArplSpecies,      K6},
        /* 0x63 */  {   "movsxd",   2, {X, v, WW},    {Y, v, RR},   {na, NA, nA}, evMovsxdSpecies,    K8}
    };

    CInstr_Table S_group_1_6d_table[2] =
    {
        /* 0x6d*/
        {    "insw",  2, {Y, v, WW},     {_DX, v, nA},   {na, NA, nA}, evInswSpecies, K6},
        {    "insd",  2, {Y, v, WW},     {_DX, v, nA},   {na, NA, nA}, evInsdSpecies, K6}
    };

    CInstr_Table S_group_1_6f_table[2] =
    {
        /* 0x6f */
        {   "outsw",  2, {_DX, v, nA},   {X, v, RR},     {na, NA, nA}, evOutswSpecies,    K6},
        {   "outsd",  2, {_DX, v, nA},   {X, v, RR},     {na, NA, nA}, evOutsdSpecies,    K6}

    };

    CInstr_Table S_group_1_80_table[8] =
    {
        {"add",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evAddSpecies,  K6},
        { "or",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evOrSpecies,   K6},
        {"adc",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evAdcSpecies,  K6},
        {"sbb",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evSbbSpecies,  K6},
        {"and",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evAndSpecies,  K6},
        {"sub",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evSubSpecies,  K6},
        {"xor",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evXorSpecies,  K6},
        {"cmp",       2, {E, b, RR},     {I, b, nA},     {na, NA, nA}, evCmpSpecies,  K6}
    };


    CInstr_Table S_group_1_81_table[8] =
    {
        {"add",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evAddSpecies,  K6},
        { "or",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evOrSpecies,   K6},
        {"adc",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evAdcSpecies,  K6},
        {"sbb",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evSbbSpecies,  K6},
        {"and",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evAndSpecies,  K6},
        {"sub",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evSubSpecies,  K6},
        {"xor",       2, {E, v, RW},     {I, v, nA},     {na, NA, nA}, evXorSpecies,  K6},
        {"cmp",       2, {E, v, RR},     {I, v, nA},     {na, NA, nA}, evCmpSpecies,  K6}
    };

    CInstr_Table S_group_1_82_table[8] =
    {
        {"add",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evAddSpecies,  K6},
        { "or",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evOrSpecies,   K6},
        {"adc",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evAdcSpecies,  K6},
        {"sbb",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evSbbSpecies,  K6},
        {"and",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evAndSpecies,  K6},
        {"sub",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evSubSpecies,  K6},
        {"xor",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evXorSpecies,  K6},
        {"cmp",       2, {E, b, RR},     {I, b, nA},     {na, NA, nA}, evCmpSpecies,  K6}
    };

    CInstr_Table S_group_1_83_table[8] =
    {
        {"add",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evAddSpecies,  K6},
        { "or",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evOrSpecies,   K6},
        {"adc",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evAdcSpecies,  K6},
        {"sbb",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evSbbSpecies,  K6},
        {"and",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evAndSpecies,  K6},
        {"sub",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evSubSpecies,  K6},
        {"xor",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evXorSpecies,  K6},
        {"cmp",       2, {E, v, RR},     {I, ib, nA},    {na, NA, nA}, evCmpSpecies,  K6}
    };

    CInstr_Table S_group_1_90_table[3] =
    {
        /* !REX */  {      "nop",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNopSpecies, K6},
        /* REX */   {     "xchg",  2, {eAX, v, nA},   {eAX, v, nA},   {na, NA, nA}, evXchgSpecies, K8},
        /* f3 */    {    "pause",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies, K7_2}
    };

    CInstr_Table S_group_1_98_table[] =
    {
        /* 0x98 */  {     "cbw",  1, {AX, v, nA},    {na, NA, nA},   {na, NA, nA}, evCbwSpecies,  K6},
        /* 0x98 */  {    "cwde",  1, {EAX, v, nA},   {na, NA, nA},   {na, NA, nA}, evCwdeSpecies, K6},
        //TODO addressing mode and species;
        /* 0x98 */  {    "cdqe",  1, {EAX, v, nA},   {na, NA, nA},   {na, NA, nA}, evCwdeSpecies, K8}
    };

    CInstr_Table S_group_1_99_table[3] =
    {
        /* 0x99 */  {     "cwd",  2, {_DX, v, nA},   {_AX, v, nA},  {na, NA, nA},  evCwdSpecies,  K6},
        /* 0x99 */  {     "cdq",  2, {_EDX, v, nA},  {_EAX, v, nA}, {na, NA, nA},  evCdqSpecies,  K6},
        //TODO addressing mode and species;
        /* 0x99 */  {     "cqo",  2, {_EDX, v, nA},  {_EAX, v, nA}, {na, NA, nA},  evCdqSpecies,  K8}
    };


    CInstr_Table S_group_1_9c_table[3] =
    {
        //TODO operand addressing mode species
        /* 0x9c */  {  "pushfw",  1, {_SP, v, WW},   {na, NA, nA},   {na, NA, nA}, evPushfdSpecies,   K6},
        /* 0x9c */  {  "pushfd",  1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA}, evPushfdSpecies,   K6},
        // TODO addressing mode and species;
        /* 0x9c */  {  "pushfq",  1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA}, evPushfqSpecies,   K8}
    };

    CInstr_Table S_group_1_9d_table[3] =
    {
        /* 0x9d */  {   "popfw",  1, {_SP, v, RR},   {na, NA, nA},   {na, NA, nA}, evPopfSpecies,     K6},
        /* 0x9d */  {   "popfd",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evPopfdSpecies,    K6},
        //TODO addressing mode and species;
        /* 0x9d */  {   "popfq",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evPopfqSpecies,    K8}
    };

    CInstr_Table S_group_1_a5_table[3] =
    {
        /* 0xa5 */  {   "movsw",  2, {X, v, WW},     {Y, v, RR},     {na, NA, nA}, evMovswSpecies,    K6},
        /* 0xa5 */  {   "movsd",  2, {X, v, WW},     {Y, v, RR},     {na, NA, nA}, evMovsdSpecies,    K6},
        /* 0xa5 */  {   "movsq",  2, {X, v, WW},     {Y, v, RR},     {na, NA, nA}, evMovsdSpecies,    K6}
    };

    CInstr_Table S_group_1_a7_table[3] =
    {
        /* 0xa7 */  {   "cmpsw",  2, {X, v, RR},     {Y, v, RR},     {na, NA, nA}, evCmpsdSpecies,    K6},
        /* 0xa7 */  {   "cmpsd",  2, {X, v, RR},     {Y, v, RR},     {na, NA, nA}, evCmpsdSpecies,    K6},
        /* 0xa7 */  {   "cmpsq",  2, {X, v, RR},     {Y, v, RR},     {na, NA, nA}, evCmpsdSpecies,    K6}
    };

    CInstr_Table S_group_1_ab_table[3] =
    {
        /* 0xab */  {   "stosw",  2, {Y, v, WW},     {_AX, v, nA},   {na, NA, nA}, evStoswSpecies,    K6},
        /* 0xab */  {   "stosd",  2, {Y, v, WW},     {_EAX, v, nA},  {na, NA, nA}, evStosdSpecies,    K6},
        /* 0xab */  {   "stosq",  2, {Y, v, WW},     {_EAX, v, nA},  {na, NA, nA}, evStosdSpecies,    K6}
    };

    CInstr_Table S_group_1_ad_table[3] =
    {
        /* 0xad */  {   "lodsw",  2, {_AX, v, nA},   {X, v, RR},     {na, NA, nA}, evLodsdSpecies,    K6},
        /* 0xad */  {   "lodsd",  2, {_EAX, v, nA},  {X, v, RR},     {na, NA, nA}, evLodsdSpecies,    K6},
        /* 0xad */  {   "lodsq",  2, {_EAX, v, nA},  {X, v, RR},     {na, NA, nA}, evLodsdSpecies,    K6}
    };

    CInstr_Table S_group_1_af_table[3] =
    {
        /* 0xaf */  {   "scasw",  2, {_AX, v, nA},   {Y, v, RR},     {na, NA, nA}, evScasdSpecies,    K6},
        /* 0xaf */  {   "scasd",  2, {_EAX, v, nA},  {Y, v, RR},     {na, NA, nA}, evScasdSpecies,    K6},
        /* 0xaf */  {   "scasq",  2, {_EAX, v, nA},  {Y, v, RR},     {na, NA, nA}, evScasdSpecies,    K6}
    };

    CInstr_Table S_group_1_c0_table[8] =
    {
        /* 0xc0 */
        {"rol",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evRolSpecies,    K6},
        {"ror",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evRorSpecies,    K6},
        {"rcl",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evRclSpecies,    K6},
        {"rcr",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evRcrSpecies,    K6},
        {"shl",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evShlSpecies,    K6},
        {"shr",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evShrSpecies,    K6},
        //TODO: verify it with Robert
        {"sal",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evSalSpecies,    K6_2},
        {"sar",       2, {E, b, RW},     {I, b, nA},     {na, NA, nA}, evSarSpecies,    K6}
    };

    CInstr_Table S_group_1_c1_table[8] =
    {
        /*  0xc1 */
        {"rol",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evRolSpecies,    K6},
        {"ror",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evRorSpecies,    K6},
        {"rcl",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evRclSpecies,    K6},
        {"rcr",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evRcrSpecies,    K6},
        {"shl",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evShlSpecies,    K6},
        {"shr",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evShrSpecies,    K6},
        {"sal",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evSalSpecies,    K6_2},
        {"sar",       2, {E, v, RW},     {I, ib, nA},    {na, NA, nA}, evSarSpecies,    K6}
    };

    CInstr_Table S_group_1_c2_table[3] =
    {
        /* 0xc2 */  {      "retnw",  2, {I, w, nA},     {_SP, w, RR},   {na, NA, nA}, evRetSpecies,   K6}, //TODO: Should ESP be the 1st or 2nd operand?
        /* 0xc2 */  {      "retnd",  2, {I, w, nA},     {_ESP, w, RR},  {na, NA, nA}, evRetSpecies,   K6}, //TODO: Should ESP be the 1st or 2nd operand?
        /* 0xc2 */  {      "retnq",  2, {I, w, nA},     {_ESP, w, RR},  {na, NA, nA}, evRetSpecies,   K6} //TODO: Should ESP be the 1st or 2nd operand?
    };

    CInstr_Table S_group_1_c3_table[3] =
    {
        /* 0xc3 */  {      "retnw",  1, {_SP, v, RR},   {na, NA, nA},   {na, NA, nA}, evRetSpecies,   K6},
        /* 0xc3 */  {      "retnd",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evRetSpecies,   K6},
        /* 0xc3 */  {      "retnq",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evRetSpecies,   K6}
    };

    CInstr_Table S_group_1_ca_table[3] =
    {
        /* 0xca */  {      "retfq",  2, {I, w, nA},     {_SP, w, RR},   {na, NA, nA}, evRetSpecies,   K6},
        /* 0xca */  {      "retfd",  2, {I, w, nA},     {_ESP, w, RR},  {na, NA, nA}, evRetSpecies,   K6},
        /* 0xca */  {      "retfq",  2, {I, w, nA},     {_ESP, w, RR},  {na, NA, nA}, evRetSpecies,   K6}
    };

    CInstr_Table S_group_1_cb_table[3] =
    {
        /* 0xcb */  {      "retfw",  1, {_SP, v, RR},   {na, NA, nA},   {na, NA, nA}, evRetSpecies,   K6},
        /* 0xcb */  {      "retfd",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evRetSpecies,   K6},
        /* 0xcb */  {      "retfq",  1, {_ESP, v, RR},  {na, NA, nA},   {na, NA, nA}, evRetSpecies,   K6}
    };

    CInstr_Table S_group_1_cf_table[3] =
    {
        /* 0xcf */  {     "iretw",  1, {_SP, v, WW},   {na, NA, nA},   {na, NA, nA},  evIretSpecies,  K6},
        /* 0xcf */  {     "iretd",  1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA},  evIretSpecies,  K6},
        /* 0xcf */  {     "iretq",  1, {_ESP, v, WW},  {na, NA, nA},   {na, NA, nA},  evIretSpecies,  K6},
    };

    CInstr_Table S_group_1_d0_table[8] =
    {
        /* 0xd0 */
        {"rol",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evRolSpecies,        K6},
        {"ror",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evRorSpecies,        K6},
        {"rcl",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evRclSpecies,        K6},
        {"rcr",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evRcrSpecies,        K6},
        {"shl",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evShlSpecies,        K6},
        {"shr",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evShrSpecies,        K6},
        {"sal",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evSalSpecies,        K6_2},
        {"sar",       2, {E, b, RW},     {I1, NA, nA},   {na, NA, nA}, evSarSpecies,        K6}
    };

    CInstr_Table S_group_1_d1_table[8] =
    {
        /* 0xd1 */
        {"rol",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evRolSpecies,        K6},
        {"ror",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evRorSpecies,        K6},
        {"rcl",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evRclSpecies,        K6},
        {"rcr",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evRcrSpecies,        K6},
        {"shl",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evShlSpecies,        K6},
        {"shr",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evShrSpecies,        K6},
        {"sal",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evSalSpecies,        K6_2},
        {"sar",       2, {E, v, RW},     {I1, NA, nA},   {na, NA, nA}, evSarSpecies,        K6}
    };

    CInstr_Table S_group_1_d2_table[8] =
    {
        /* 0xd2 */
        {"rol",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evRolSpecies,        K6},
        {"ror",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evRorSpecies,        K6},
        {"rcl",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evRclSpecies,        K6},
        {"rcr",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evRcrSpecies,        K6},
        {"shl",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evShlSpecies,        K6},
        {"shr",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evShrSpecies,        K6},
        {"sal",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evSalSpecies,        K6_2},
        {"sar",       2, {E, b, RW},     {CL, NA, nA},   {na, NA, nA}, evSarSpecies,        K6}
    };

    CInstr_Table S_group_1_d3_table[8] =
    {
        /* 0xd3 */
        {"rol",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evRolSpecies,        K6},
        {"ror",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evRorSpecies,        K6},
        {"rcl",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evRclSpecies,        K6},
        {"rcr",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evRcrSpecies,        K6},
        {"shl",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evShlSpecies,        K6},
        {"shr",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evShrSpecies,        K6},
        {"sal",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evSalSpecies,        K6_2},
        {"sar",       2, {E, v, RW},     {CL, NA, nA},   {na, NA, nA}, evSarSpecies,        K6}
    };

    CInstr_Table S_group_1_d8_table[72] =
    {
        /* 0xd8, size 72 */
        //ESC LOW
        {   "fadd",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        {   "fmul",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        {   "fcom",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        {  "fcomp",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        {   "fsub",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        {  "fsubr",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        {   "fdiv",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        {  "fdivr",  1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        // ESC HIGH
        /* 0x00 */  {    "fadd", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x01 */  {    "fadd", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x02 */  {    "fadd", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x03 */  {    "fadd", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x04 */  {    "fadd", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x05 */  {    "fadd", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x06 */  {    "fadd", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x07 */  {    "fadd", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x08 */  {    "fmul", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x09 */  {    "fmul", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0a */  {    "fmul", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0b */  {    "fmul", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0c */  {    "fmul", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0d */  {    "fmul", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0e */  {    "fmul", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0f */  {    "fmul", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x10 */  {    "fcom", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x11 */  {    "fcom", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x12 */  {    "fcom", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x13 */  {    "fcom", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x14 */  {    "fcom", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x15 */  {    "fcom", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x16 */  {    "fcom", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x17 */  {    "fcom", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        /* 0x18 */  {   "fcomp", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x19 */  {   "fcomp", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x1a */  {   "fcomp", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x1b */  {   "fcomp", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x1c */  {   "fcomp", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x1d */  {   "fcomp", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x1e */  {   "fcomp", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x1f */  {   "fcomp", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        /* 0x20 */  {    "fsub", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x21 */  {    "fsub", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x22 */  {    "fsub", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x23 */  {    "fsub", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x24 */  {    "fsub", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x25 */  {    "fsub", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x26 */  {    "fsub", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x27 */  {    "fsub", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x28 */  {   "fsubr", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x29 */  {   "fsubr", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x2a */  {   "fsubr", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x2b */  {   "fsubr", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x2c */  {   "fsubr", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x2d */  {   "fsubr", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x2e */  {   "fsubr", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x2f */  {   "fsubr", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x30 */  {    "fdiv", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x31 */  {    "fdiv", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x32 */  {    "fdiv", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x33 */  {    "fdiv", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x34 */  {    "fdiv", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x35 */  {    "fdiv", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x36 */  {    "fdiv", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x37 */  {    "fdiv", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x38 */  {   "fdivr", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x39 */  {   "fdivr", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x3a */  {   "fdivr", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x3b */  {   "fdivr", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x3c */  {   "fdivr", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x3d */  {   "fdivr", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x3e */  {   "fdivr", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x3f */  {   "fdivr", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFdivrSpecies,     K6}
    };

    CInstr_Table S_group_1_d9_table[72] =
    {
        /* 0xd9 */
        // ESC LOW
        {     "fld", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {     "fst", 1, {E, d, WW},   {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        {    "fstp", 1, {E, d, WW},   {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        {  "fldenv", 1, {E, v, RR},   {na, NA, nA}, {na, NA, nA}, evFldenvSpecies,    K6}, // TODO: Data size is m14/28 byte.  How should this be done?!
        {   "fldcw", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFldcwSpecies,     K6},
        {  "fstenv", 1, {E, v, WW},   {na, NA, nA}, {na, NA, nA}, evFstenvSpecies,    K6}, // TODO: Data size is m14/28 byte.  How should this be done?!
        {   "fstcw", 1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evFstcwSpecies,     K6},
        //ESC HIGH
        /* 0x00 */  {     "fld", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6}, // TODO: Make sure PUSH/POP to FP stack doesn't report ST as an operand.
        /* 0x01 */  {     "fld", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6}, // TODO: Report ST as an operand only if it is operated on!
        /* 0x02 */  {     "fld", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x03 */  {     "fld", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x04 */  {     "fld", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x05 */  {     "fld", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x06 */  {     "fld", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x07 */  {     "fld", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x08 */  {    "fxch", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x09 */  {    "fxch", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x0a */  {    "fxch", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x0b */  {    "fxch", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x0c */  {    "fxch", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x0d */  {    "fxch", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x0e */  {    "fxch", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x0f */  {    "fxch", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        /* 0x10 */  {    "fnop", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFnopSpecies,      K6},
        /* 0x11 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x12 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x13 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x14 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x15 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x16 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x17 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x18 */  {   "fstp1", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x19 */  {   "fstp1", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x1a */  {   "fstp1", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x1b */  {   "fstp1", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x1c */  {   "fstp1", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x1d */  {   "fstp1", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x1e */  {   "fstp1", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x1f */  {   "fstp1", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp1Species,     K6},
        /* 0x20 */  {    "fchs", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFchsSpecies,      K6},
        /* 0x21 */  {    "fabs", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFabsSpecies,      K6},
        /* 0x22 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x23 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x24 */  {    "ftst", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFtstSpecies,      K6},
        /* 0x25 */  {    "fxam", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFxamSpecies,      K6},
        /* 0x26 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x27 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x28 */  {    "fld1", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFld1Species,      K6},
        /* 0x29 */  {  "fldl2t", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFldl2tSpecies,    K6},
        /* 0x2a */  {  "fldl2e", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFldl2eSpecies,    K6},
        /* 0x2b */  {   "fldpi", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFldpiSpecies,     K6},
        /* 0x2c */  {  "fldlg2", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFldlg2Species,    K6},
        /* 0x2d */  {  "fldln2", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFldln2Species,    K6},
        /* 0x2e */  {    "fldz", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFldzSpecies,      K6},
        /* 0x2f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x30 */  {   "f2xm1", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evF2xm1Species,     K6},
        /* 0x31 */  {   "fyl2x", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFyl2xSpecies,     K6},
        /* 0x32 */  {   "fptan", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFptanSpecies,     K6},
        /* 0x33 */  {  "fpatan", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFpatanSpecies,    K6},
        /* 0x34 */  { "fxtract", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFxtractSpecies,   K6},
        /* 0x35 */  {  "fprem1", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFprem1Species,    K6},
        /* 0x36 */  { "fdecstp", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFdecstpSpecies,   K6},
        /* 0x37 */  { "fincstp", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFincstpSpecies,   K6},
        /* 0x38 */  {   "fprem", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFpremSpecies,     K6},
        /* 0x39 */  { "fyl2xp1", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFyl2xp1Species,   K6},
        /* 0x3a */  {   "fsqrt", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFsqrtSpecies,     K6},
        /* 0x3b */  { "fsincos", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFsincosSpecies,   K6},
        /* 0x3c */  { "frndint", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFrndintSpecies,   K6},
        /* 0x3d */  {  "fscale", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFscaleSpecies,    K6},
        /* 0x3e */  {    "fsin", 0, {ST, t, nA},  {na, NA, nA}, {na, NA, nA}, evFsinSpecies,      K6},
        /* 0x3f */  {    "fcos", 0, {ST, t, nA},  {na, NA, nA}, {na, NA, nA}, evFcosSpecies,      K6}
    };

    CInstr_Table S_group_1_da_table[72] =
    {
        /* 0xda*/
        // ESC LOW
        {   "fiadd", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFiaddSpecies,     K6},
        {   "fimul", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFimulSpecies,     K6},
        {   "ficom", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFicomSpecies,     K6},
        {  "ficomp", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFicompSpecies,    K6},
        {   "fisub", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFisubSpecies,     K6},
        {  "fisubr", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFisubrSpecies,    K6},
        {   "fidiv", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFidivSpecies,     K6},
        {  "fidivr", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFidivrSpecies,    K6},
        //ESC HIGH
        /* 0x00 */  {  "fcmovb", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x01 */  {  "fcmovb", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x02 */  {  "fcmovb", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x03 */  {  "fcmovb", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x04 */  {  "fcmovb", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x05 */  {  "fcmovb", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x06 */  {  "fcmovb", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x07 */  {  "fcmovb", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovbSpecies,    K7},
        /* 0x08 */  {  "fcmove", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x09 */  {  "fcmove", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x0a */  {  "fcmove", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x0b */  {  "fcmove", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x0c */  {  "fcmove", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x0d */  {  "fcmove", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x0e */  {  "fcmove", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x0f */  {  "fcmove", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmoveSpecies,    K7},
        /* 0x10 */  { "fcmovbe", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x11 */  { "fcmovbe", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x12 */  { "fcmovbe", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x13 */  { "fcmovbe", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x14 */  { "fcmovbe", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x15 */  { "fcmovbe", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x16 */  { "fcmovbe", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x17 */  { "fcmovbe", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovbeSpecies,   K7},
        /* 0x18 */  {  "fcmovu", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x19 */  {  "fcmovu", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x1a */  {  "fcmovu", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x1b */  {  "fcmovu", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x1c */  {  "fcmovu", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x1d */  {  "fcmovu", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x1e */  {  "fcmovu", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x1f */  {  "fcmovu", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovuSpecies,    K7},
        /* 0x20 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x21 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x22 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x23 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x24 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x25 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x26 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x27 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x28 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x29 */  { "fucompp", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFucomppSpecies,   K6},
        /* 0x2a */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x2b */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x2c */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x2d */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x2e */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x2f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x30 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x31 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x32 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x33 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x34 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x35 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x36 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x37 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x38 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x39 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3a */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3b */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3c */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3d */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3e */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6}
    };

    CInstr_Table S_group_1_db_table[72] =
    {
        /*  0xdb */
        // ESC LOW
        {    "fild", 1, {E, d, RR},   {na, NA, nA}, {na, NA, nA}, evFildSpecies,      K6},
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {    "fist", 1, {E, d, WW},   {na, NA, nA}, {na, NA, nA}, evFistSpecies,      K6},
        {   "fistp", 1, {E, d, WW},   {na, NA, nA}, {na, NA, nA}, evFistpSpecies,     K6},
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {     "fld", 1, {E, t, RR},   {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6}, //TODO: type is 80bit?!!
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {    "fstp", 1, {E, t, WW},   {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        //ESC HIGH
        /* 0x00 */  { "fcmovnb", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x01 */  { "fcmovnb", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x02 */  { "fcmovnb", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x03 */  { "fcmovnb", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x04 */  { "fcmovnb", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x05 */  { "fcmovnb", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x06 */  { "fcmovnb", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x07 */  { "fcmovnb", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovnbSpecies,   K7},
        /* 0x08 */  { "fcmovne", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x09 */  { "fcmovne", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x0a */  { "fcmovne", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x0b */  { "fcmovne", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x0c */  { "fcmovne", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x0d */  { "fcmovne", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x0e */  { "fcmovne", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x0f */  { "fcmovne", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovneSpecies,   K7},
        /* 0x10 */  {"fcmovnbe", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x11 */  {"fcmovnbe", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x12 */  {"fcmovnbe", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x13 */  {"fcmovnbe", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x14 */  {"fcmovnbe", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x15 */  {"fcmovnbe", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x16 */  {"fcmovnbe", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x17 */  {"fcmovnbe", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovnbeSpecies,  K7},
        /* 0x18 */  { "fcmovnu", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x19 */  { "fcmovnu", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x1a */  { "fcmovnu", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x1b */  { "fcmovnu", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x1c */  { "fcmovnu", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x1d */  { "fcmovnu", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x1e */  { "fcmovnu", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x1f */  { "fcmovnu", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcmovnuSpecies,   K7},
        /* 0x20 */  {    "feni", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFeniSpecies,      K6}, // TODO:  Cannot find this instr anywhere, but MASM understands it!
        /* 0x21 */  {   "fdisi", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFdisiSpecies,     K6}, // TODO:  Cannot find this instr anywhere, but MASM understands it!
        /* 0x22 */  {   "fclex", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFclexSpecies,     K6},
        /* 0x23 */  {   "finit", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFinitSpecies,     K6},
        /* 0x24 */  {  "fsetpm", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFsetpmSpecies,    K6}, // TODO:  Cannot find this instr anywhere, but MASM understands it!
        /* 0x25 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x26 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x27 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x28 */  {  "fucomi", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6}, // TODO:  Cannot find this instr anywhere, but MASM understands it!
        /* 0x29 */  {  "fucomi", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x2a */  {  "fucomi", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x2b */  {  "fucomi", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x2c */  {  "fucomi", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x2d */  {  "fucomi", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x2e */  {  "fucomi", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x2f */  {  "fucomi", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFucomiSpecies,    K6},
        /* 0x30 */  {   "fcomi", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6}, // TODO:  Cannot find this instr anywhere, but MASM understands it!
        /* 0x31 */  {   "fcomi", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x32 */  {   "fcomi", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x33 */  {   "fcomi", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x34 */  {   "fcomi", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x35 */  {   "fcomi", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x36 */  {   "fcomi", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x37 */  {   "fcomi", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcomiSpecies,     K6},
        /* 0x38 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x39 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3a */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3b */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3c */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3d */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3e */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6}
    };

    CInstr_Table S_group_1_dc_table[72] =
    {
        /* 0xdc */
        // ESC LOW
        {    "fadd", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFaddSpecies,      K6},
        {    "fmul", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFmulSpecies,      K6},
        {    "fcom", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFcomSpecies,      K6},
        {   "fcomp", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFcompSpecies,     K6},
        {    "fsub", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFsubSpecies,      K6},
        {   "fsubr", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFsubrSpecies,     K6},
        {    "fdiv", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFdivSpecies,      K6},
        {   "fdivr", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFdivrSpecies,     K6},
        //ESC HIGH
        /* 0x00 */  {    "fadd", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x01 */  {    "fadd", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x02 */  {    "fadd", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x03 */  {    "fadd", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x04 */  {    "fadd", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x05 */  {    "fadd", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x06 */  {    "fadd", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x07 */  {    "fadd", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFaddSpecies,      K6},
        /* 0x08 */  {    "fmul", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x09 */  {    "fmul", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0a */  {    "fmul", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0b */  {    "fmul", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0c */  {    "fmul", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0d */  {    "fmul", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0e */  {    "fmul", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x0f */  {    "fmul", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFmulSpecies,      K6},
        /* 0x10 */  {   "fcom2", 1, {ST0, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x11 */  {   "fcom2", 1, {ST1, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x12 */  {   "fcom2", 1, {ST2, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x13 */  {   "fcom2", 1, {ST3, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x14 */  {   "fcom2", 1, {ST4, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x15 */  {   "fcom2", 1, {ST5, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x16 */  {   "fcom2", 1, {ST6, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x17 */  {   "fcom2", 1, {ST7, t, nA}, {na, t, nA},  {na, NA, nA}, evFcom2Species,     K6},
        /* 0x18 */  {  "fcomp3", 1, {ST0, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x19 */  {  "fcomp3", 1, {ST1, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x1a */  {  "fcomp3", 1, {ST2, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x1b */  {  "fcomp3", 1, {ST3, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x1c */  {  "fcomp3", 1, {ST4, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x1d */  {  "fcomp3", 1, {ST5, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x1e */  {  "fcomp3", 1, {ST6, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x1f */  {  "fcomp3", 1, {ST7, t, nA}, {na, t, nA},  {na, NA, nA}, evFcomp3Species,    K6},
        /* 0x20 */  {   "fsubr", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x21 */  {   "fsubr", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x22 */  {   "fsubr", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x23 */  {   "fsubr", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x24 */  {   "fsubr", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x25 */  {   "fsubr", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x26 */  {   "fsubr", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x27 */  {   "fsubr", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrSpecies,     K6},
        /* 0x28 */  {    "fsub", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x29 */  {    "fsub", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x2a */  {    "fsub", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x2b */  {    "fsub", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x2c */  {    "fsub", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x2d */  {    "fsub", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x2e */  {    "fsub", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x2f */  {    "fsub", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubSpecies,      K6},
        /* 0x30 */  {   "fdivr", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x31 */  {   "fdivr", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x32 */  {   "fdivr", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x33 */  {   "fdivr", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x34 */  {   "fdivr", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x35 */  {   "fdivr", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x36 */  {   "fdivr", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x37 */  {   "fdivr", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrSpecies,     K6},
        /* 0x38 */  {    "fdiv", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x39 */  {    "fdiv", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x3a */  {    "fdiv", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x3b */  {    "fdiv", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x3c */  {    "fdiv", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x3d */  {    "fdiv", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x3e */  {    "fdiv", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6},
        /* 0x3f */  {    "fdiv", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivSpecies,      K6}
    };
    CInstr_Table S_group_1_dd_table[72] =
    {
        /* 0xdd */
        //ESC LOW
        {     "fld", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFldSpecies,       K6},
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {     "fst", 1, {E, q, WW},   {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        {    "fstp", 1, {E, q, WW},   {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        {  "frstor", 1, {E, v, RR},   {na, NA, nA}, {na, NA, nA}, evFrstorSpecies,    K6}, // TODO: How should m94/108byte be done?
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {   "fsave", 1, {E, v, WW},   {na, NA, nA}, {na, NA, nA}, evFsaveSpecies,     K6}, // TODO: How should m94/108byte be done?
        {   "fstsw", 1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evFstswSpecies,     K6},
        //ESC HIGH
        /* 0x00 */  {   "ffree", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(0)
        /* 0x01 */  {   "ffree", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(1)
        /* 0x02 */  {   "ffree", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(2)
        /* 0x03 */  {   "ffree", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(3)
        /* 0x04 */  {   "ffree", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(4)
        /* 0x05 */  {   "ffree", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(5)
        /* 0x06 */  {   "ffree", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(6)
        /* 0x07 */  {   "ffree", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreeSpecies,     K6}, // st(7)
        /* 0x08 */  {   "fxch4", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x09 */  {   "fxch4", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x0a */  {   "fxch4", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x0b */  {   "fxch4", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x0c */  {   "fxch4", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x0d */  {   "fxch4", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x0e */  {   "fxch4", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x0f */  {   "fxch4", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch4Species,     K6},
        /* 0x10 */  {     "fst", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x11 */  {     "fst", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x12 */  {     "fst", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x13 */  {     "fst", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x14 */  {     "fst", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x15 */  {     "fst", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x16 */  {     "fst", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x17 */  {     "fst", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstSpecies,       K6},
        /* 0x18 */  {    "fstp", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x19 */  {    "fstp", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x1a */  {    "fstp", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x1b */  {    "fstp", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x1c */  {    "fstp", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x1d */  {    "fstp", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x1e */  {    "fstp", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x1f */  {    "fstp", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstpSpecies,      K6},
        /* 0x20 */  {   "fucom", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x21 */  {   "fucom", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x22 */  {   "fucom", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x23 */  {   "fucom", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x24 */  {   "fucom", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x25 */  {   "fucom", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x26 */  {   "fucom", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x27 */  {   "fucom", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucomSpecies,     K6},
        /* 0x28 */  {  "fucomp", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x29 */  {  "fucomp", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x2a */  {  "fucomp", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x2b */  {  "fucomp", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x2c */  {  "fucomp", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x2d */  {  "fucomp", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x2e */  {  "fucomp", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x2f */  {  "fucomp", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFucompSpecies,    K6},
        /* 0x30 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x31 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x32 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x33 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x34 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x35 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x36 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x37 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x38 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x39 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3a */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3b */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3c */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3d */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3e */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6}
    };

    CInstr_Table S_group_1_de_table[72] =
    {
        // 0xde
        // ESC LOW
        {   "fiadd", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFiaddSpecies,     K6},
        {   "fimul", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFimulSpecies,     K6},
        {   "ficom", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFicomSpecies,     K6},
        {  "ficomp", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFicompSpecies,    K6},
        {   "fisub", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFisubSpecies,     K6},
        {  "fisubr", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFisubrSpecies,    K6},
        {   "fidiv", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFidivSpecies,     K6},
        {  "fidivr", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFidivrSpecies,    K6},
        // ESC HIGH
        /* 0x00 */  {   "faddp", 2, {ST0, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x01 */  {   "faddp", 2, {ST1, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x02 */  {   "faddp", 2, {ST2, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x03 */  {   "faddp", 2, {ST3, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x04 */  {   "faddp", 2, {ST4, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x05 */  {   "faddp", 2, {ST5, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x06 */  {   "faddp", 2, {ST6, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x07 */  {   "faddp", 2, {ST7, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFaddpSpecies,     K6},
        /* 0x08 */  {   "fmulp", 2, {ST0, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x09 */  {   "fmulp", 2, {ST1, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x0a */  {   "fmulp", 2, {ST2, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x0b */  {   "fmulp", 2, {ST3, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x0c */  {   "fmulp", 2, {ST4, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x0d */  {   "fmulp", 2, {ST5, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x0e */  {   "fmulp", 2, {ST6, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x0f */  {   "fmulp", 2, {ST7, t, nA}, {ST, NA, nA}, {na, NA, nA}, evFmulpSpecies,     K6},
        /* 0x10 */  {  "fcomp5", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x11 */  {  "fcomp5", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x12 */  {  "fcomp5", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x13 */  {  "fcomp5", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x14 */  {  "fcomp5", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x15 */  {  "fcomp5", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x16 */  {  "fcomp5", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x17 */  {  "fcomp5", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFcomp5Species,    K6},
        /* 0x18 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x19 */  {  "fcompp", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evFcomppSpecies,    K6},
        /* 0x1a */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x1b */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x1c */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x1d */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x1e */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x1f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x20 */  {  "fsubrp", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x21 */  {  "fsubrp", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x22 */  {  "fsubrp", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x23 */  {  "fsubrp", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x24 */  {  "fsubrp", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x25 */  {  "fsubrp", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x26 */  {  "fsubrp", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x27 */  {  "fsubrp", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubrpSpecies,    K6},
        /* 0x28 */  {   "fsubp", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x29 */  {   "fsubp", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x2a */  {   "fsubp", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x2b */  {   "fsubp", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x2c */  {   "fsubp", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x2d */  {   "fsubp", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x2e */  {   "fsubp", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x2f */  {   "fsubp", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFsubpSpecies,     K6},
        /* 0x30 */  {  "fdivrp", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x31 */  {  "fdivrp", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x32 */  {  "fdivrp", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x33 */  {  "fdivrp", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x34 */  {  "fdivrp", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x35 */  {  "fdivrp", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x36 */  {  "fdivrp", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x37 */  {  "fdivrp", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivrpSpecies,    K6},
        /* 0x38 */  {   "fdivp", 2, {ST0, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x39 */  {   "fdivp", 2, {ST1, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x3a */  {   "fdivp", 2, {ST2, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x3b */  {   "fdivp", 2, {ST3, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x3c */  {   "fdivp", 2, {ST4, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x3d */  {   "fdivp", 2, {ST5, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x3e */  {   "fdivp", 2, {ST6, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6},
        /* 0x3f */  {   "fdivp", 2, {ST7, t, nA}, {ST, t, nA},  {na, NA, nA}, evFdivpSpecies,     K6}
    };

    CInstr_Table S_group_1_df_table[72] =
    {
        // 0xdf
        //ESC LOW
        {    "fild", 1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evFildSpecies,      K6},
        {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        {    "fist", 1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evFistSpecies,      K6},
        {   "fistp", 1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evFistpSpecies,     K6},
        {    "fbld", 1, {E, t, RR},   {na, NA, nA}, {na, NA, nA}, evFbldSpecies,      K6}, //TODO: type is 80bit?!!  Shouldn't there be an operand?
        {    "fild", 1, {E, q, RR},   {na, NA, nA}, {na, NA, nA}, evFildSpecies,      K6},
        {   "fbstp", 1, {E, t, WW},   {na, NA, nA}, {na, NA, nA}, evFbstpSpecies,     K6}, //TODO: type is 80bit?!!  Shouldn't there be an operand?
        {   "fistp", 1, {E, q, WW},   {na, NA, nA}, {na, NA, nA}, evFistpSpecies,     K6},
        //ESC HIGH
        /* 0x00 */  {  "ffreep", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x01 */  {  "ffreep", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x02 */  {  "ffreep", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x03 */  {  "ffreep", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x04 */  {  "ffreep", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x05 */  {  "ffreep", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x06 */  {  "ffreep", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x07 */  {  "ffreep", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFfreepSpecies,    K6},
        /* 0x08 */  {   "fxch7", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x09 */  {   "fxch7", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x0a */  {   "fxch7", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x0b */  {   "fxch7", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x0c */  {   "fxch7", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x0d */  {   "fxch7", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x0e */  {   "fxch7", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x0f */  {   "fxch7", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFxch7Species,     K6},
        /* 0x10 */  {   "fstp8", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x11 */  {   "fstp8", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x12 */  {   "fstp8", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x13 */  {   "fstp8", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x14 */  {   "fstp8", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x15 */  {   "fstp8", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x16 */  {   "fstp8", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x17 */  {   "fstp8", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp8Species,     K6},
        /* 0x18 */  {   "fstp9", 1, {ST0, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6}, // TODO:  Cannot find this instr anywhere, and MASM doesn't understands it!
        /* 0x19 */  {   "fstp9", 1, {ST1, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x1a */  {   "fstp9", 1, {ST2, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x1b */  {   "fstp9", 1, {ST3, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x1c */  {   "fstp9", 1, {ST4, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x1d */  {   "fstp9", 1, {ST5, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x1e */  {   "fstp9", 1, {ST6, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x1f */  {   "fstp9", 1, {ST7, t, nA}, {na, NA, nA}, {na, NA, nA}, evFstp9Species,     K6},
        /* 0x20 */  {   "fstsw", 1, {AX, w, nA},  {na, NA, nA}, {na, NA, nA}, evFstswSpecies,     K6},
        /* 0x21 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x22 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x23 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x24 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x25 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x26 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x27 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x28 */  { "fucomip", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x29 */  { "fucomip", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x2a */  { "fucomip", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x2b */  { "fucomip", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x2c */  { "fucomip", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x2d */  { "fucomip", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x2e */  { "fucomip", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x2f */  { "fucomip", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFucomipSpecies,   K6},
        /* 0x30 */  {  "fcomip", 2, {ST, t, nA},  {ST0, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x31 */  {  "fcomip", 2, {ST, t, nA},  {ST1, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x32 */  {  "fcomip", 2, {ST, t, nA},  {ST2, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x33 */  {  "fcomip", 2, {ST, t, nA},  {ST3, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x34 */  {  "fcomip", 2, {ST, t, nA},  {ST4, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x35 */  {  "fcomip", 2, {ST, t, nA},  {ST5, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x36 */  {  "fcomip", 2, {ST, t, nA},  {ST6, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x37 */  {  "fcomip", 2, {ST, t, nA},  {ST7, t, nA}, {na, NA, nA}, evFcomipSpecies,    K6},
        /* 0x38 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x39 */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3a */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3b */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3c */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3d */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3e */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6},
        /* 0x3f */  {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6}
    };

    CInstr_Table S_group_1_e3_table[3] =
    {
        /*16-bits*/ {     "jcxz",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJcxzSpecies,    K6},
        /*32-bits*/ {    "jecxz",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJcxzSpecies,    K6},
        /*64-bits*/ {    "jrcxz",  1, {J, b, nA},     {na, NA, nA},   {na, NA, nA}, evJcxzSpecies,    K8}
    };


    CInstr_Table S_group_1_fa_table[2] =
    {
        /* 0xfa */  {     "cli", 0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evCliSpecies,   K6},
        // TODO REX instruciton: "slx"{ }
        {     "slx", 0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evSlxSpecies,   K8}
    };

    CInstr_Table S_group_1_fb_table[2] =
    {
        /* 0xfb */  {      "sti",  0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evStiSpecies, K6},
        // TODO REX instruction: "stx" { }
        {     "stx", 0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evStxSpecies,   K8}
    };

    CInstr_Table S_group_1_f6_table[8] =
    {
        // 0xf6
        {"test",      2, {E, b, RR},     {I, b, nA},     {na, NA, nA}, evTestSpecies,   K6},
        {"test",      2, {E, b, RR},     {I, b, nA},     {na, NA, nA}, evTestSpecies,   K6}, // Is Ib correct?
        {"not",       1, {E, b, RW},     {na, NA, nA},   {na, NA, nA}, evNotSpecies,    K6},
        {"neg",       1, {E, b, RW},     {na, NA, nA},   {na, NA, nA}, evNegSpecies,    K6},
        {"mul",       2, {_AX, b, nA},   {E, b, RR},     {na, NA, nA}, evMulSpecies,    K6},
        {"imul",      2, {_AX, b, nA},   {E, b, RR},     {na, NA, nA}, evImulSpecies,   K6},
        {"div",       2, {_AX, b, nA},   {E, b, RR},     {na, NA, nA}, evDivSpecies,    K6},
        {"idiv",      2, {_AX, b, nA},   {E, b, RR},     {na, NA, nA}, evIdivSpecies,   K6}
    };

    CInstr_Table S_group_1_f7_table[8] =
    {
        // 0xf7
        {"test",      2, {E, v, RR},     {I, v, nA},     {na, NA, nA}, evTestSpecies,   K6},
        {"test",      2, {E, v, RR},     {I, v, nA},     {na, NA, nA}, evTestSpecies,   K6}, // Is Iv correct?
        {"not",       1, {E, v, RW},     {na, NA, nA},   {na, NA, nA}, evNotSpecies,    K6},
        {"neg",       1, {E, v, RW},     {na, NA, nA},   {na, NA, nA}, evNegSpecies,    K6},
        {"mul",       3, {_EDX, v, nA},  {_EAX, v, nA},  {E, v, RR},   evMulSpecies,    K6},
        {"imul",      3, {_EDX, v, nA},  {_EAX, v, nA},  {E, v, RR},   evImulSpecies,   K6},
        {"div",       3, {_EDX, v, nA},  {_EAX, v, nA},  {E, v, RR},   evDivSpecies,    K6},
        {"idiv",      3, {_EDX, v, nA},  {_EAX, v, nA},  {E, v, RR},   evIdivSpecies,   K6}
    };

    CInstr_Table S_group_1_fe_table[8] =
    {
        // 0xfe
        {"inc",       1, {E, b, RW},     {na, NA, nA},   {na, NA, nA}, evIncSpecies,    K6},
        {"dec",       1, {E, b, RW},     {na, NA, nA},   {na, NA, nA}, evDecSpecies,    K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6}
    };

    CInstr_Table S_group_1_ff_table[8] =
    {
        {"inc",       1, {E, v, RW},     {na, NA, nA},   {na, NA, nA}, evIncSpecies,    K6},
        {"dec",       1, {E, v, RW},     {na, NA, nA},   {na, NA, nA}, evDecSpecies,    K6},
        {"call",      1, {E, v, RR},     {na, NA, nA},   {na, NA, nA}, evCallSpecies,   K6},
        {"call",      1, {E, p, RR},     {na, NA, nA},   {na, NA, nA}, evCallSpecies,   K6},
        {"jmp",       1, {E, v, RR},     {na, NA, nA},   {na, NA, nA}, evJmpSpecies,    K6},
        {"jmp",       1, {E, p, RR},     {na, NA, nA},   {na, NA, nA}, evJmpSpecies,    K6},
        {"push",      2, {_ESP, NA, WW}, {E, v, RR},     {na, NA, nA}, evPushSpecies,   K6},
        {"",          0, {na, NA, nA},   {na, NA, nA},   {na, NA, nA}, evNASpecies,     K6}
    };


    ///////////////////////////////////////////////////////////////
    // Group_2
    CInstr_Table S_group_2_00_table[8] =
    {
        // 0x0F00
        {"sldt",      1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evSldtSpecies,   K6},
        { "str",      1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evStrSpecies,    K6},
        {"lldt",      1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evLldtSpecies,   K6},
        {"ltr",       1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evLtrSpecies,    K6},
        {"verr",      1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evVerrSpecies,   K6},
        {"verw",      1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evVerwSpecies,   K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6}
    };

    CInstr_Table S_group_2_01_table[8] =
    {
        // 0x0f01
        {"sgdt",      1, {M, s, WW},   {na, NA, nA}, {na, NA, nA}, evSgdtSpecies,   K6},
        {"sidt",      1, {M, s, WW},   {na, NA, nA}, {na, NA, nA}, evSldtSpecies,   K6},
        {"lgdt",      1, {M, s, RR},   {na, NA, nA}, {na, NA, nA}, evLgdtSpecies,   K6},
        {"lidt",      1, {M, s, RR},   {na, NA, nA}, {na, NA, nA}, evLidtSpecies,   K6},
        {"smsw",      1, {E, w, WW},   {na, NA, nA}, {na, NA, nA}, evSmswSpecies,   K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        {"lmsw",      1, {E, w, RR},   {na, NA, nA}, {na, NA, nA}, evLmswSpecies,   K6},
        {"invlpg",    1, {M, v, RR},   {na, NA, nA}, {na, NA, nA}, evInvlpgSpecies, K6}
    };

    CInstr_Table S_group_2_0d_table[2] =
    {
        // 0x0f0d
        {  "prefetch",  1, {M, v, RR},     {na, NA, nA},   {na, NA, nA}, evPrefetchSpecies,   K6_2},
        { "prefetchw",  1, {M, v, RR},     {na, NA, nA},   {na, NA, nA}, evPrefetchwSpecies, K6_2}
    };

    CInstr_Table S_group_2_0f_table[24] =
    {
        // 3DNOW instruction
        /* 0x0f 0x0f 0x0c,*/   {    "pi2fw", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPi2fwSpecies,     K6_ST50 },
        /* 0x0f 0x0f 0x0d,*/   {    "pi2fd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPi2fdSpecies,     K6_2    },
        /* 0x0f 0x0f 0x1c,*/   {    "pf2iw", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPf2iwSpecies,     K6_ST50 },
        /* 0x0f 0x0f 0x1d,*/   {    "pf2id", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPf2idSpecies,     K6_2    },
        /* 0x0f 0x0f 0x8a,*/   {   "pfnacc", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfnaccSpecies,    K6_ST50 },
        /* 0x0f 0x0f 0x8e,*/   {  "pfpnacc", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfpnaccSpecies,   K6_ST50 },
        /* 0x0f 0x0f 0x90,*/   {  "pfcmpge", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfcmpgeSpecies,   K6_2    },
        /* 0x0f 0x0f 0x94,*/   {    "pfmin", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfminSpecies,     K6_2    },
        /* 0x0f 0x0f 0x96,*/   {    "pfrcp", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfrcpSpecies,     K6_2    },
        /* 0x0f 0x0f 0x97,*/   {  "pfrsqrt", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfrsqrtSpecies,   K6_2    },
        /* 0x0f 0x0f 0x9a,*/   {    "pfsub", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfsubSpecies,     K6_2    },
        /* 0x0f 0x0f 0x9e,*/   {    "pfadd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfaddSpecies,     K6_2    },
        /* 0x0f 0x0f 0xa0,*/   {  "pfcmpgt", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfcmpgtSpecies,   K6_2    },
        /* 0x0f 0x0f 0xa4,*/   {    "pfmax", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfmaxSpecies,     K6_2    },
        /* 0x0f 0x0f 0xa6,*/   { "pfrcpit1", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfrcpit1Species,  K6_2    },
        /* 0x0f 0x0f 0xa7,*/   { "pfrsqit1", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfrsqit1Species,  K6_2    },
        /* 0x0f 0x0f 0xaa,*/   {   "pfsubr", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfsubrSpecies,    K6_2    },
        /* 0x0f 0x0f 0xae,*/   {    "pfacc", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfaccSpecies,     K6_2    },
        /* 0x0f 0x0f 0xb0,*/   {  "pfcmpeq", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfcmpeqSpecies,   K6_2    },
        /* 0x0f 0x0f 0xb4,*/   {    "pfmul", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfmulSpecies,     K6_2    },
        /* 0x0f 0x0f 0xb6,*/   { "pfrcpit2", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPfrcpit2Species,  K6_2    },
        /* 0x0f 0x0f 0xb7,*/   {  "pmulhrw", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPmulhrwSpecies,   K6_2    },
        /* 0x0f 0x0f 0xbb,*/   {   "pswapd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPswapdSpecies,    K6_ST50 },
        /* 0x0f 0x0f 0xbf,*/   {  "pavgusb", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evPavgusbSpecies,   K6_2    }
    };

    CInstr_Table S_group_2_10_table[4] =
    {
        /* no prefix */ {    "movups", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovupsSpecies,      K7_2 },
        /* 66 prefix */ {    "movupd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovupdSpecies,      K7_2 },
        /* f2 prefix */ {     "movsd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovsSpecies,        K7_2 },
        /* f3 prefix */ {     "movss", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovssSpecies,       K7_2 }
    };

    CInstr_Table S_group_2_11_table[4] =
    {
        /* 0x0f 0x11 */
        /* no prefix */ {    "movups", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovupsSpecies,      K7_2 },
        /* 66 prefix */ {    "movupd", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovupdSpecies,      K7_2 },
        /* f2 prefix */ {     "movsd", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovsSpecies,        K7_2 },
        /* f3 prefix */ {     "movss", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovssSpecies,       K7_2 }
    };

    CInstr_Table S_group_2_12_table[4] =
    {
        /* no prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6 }, // this entry will go to S_group_2_12_00_table entries
        /* 66 prefix */ {  "movlpd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovlpsSpecies,    K7_2 },
        /* f2 prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6 },
        /* f3 prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6 }
    };

    CInstr_Table S_group_2_12_00_table[2] =
    {
        /* mod !=3 */   {    "movlps", 2, {G, b, nA},   {E, b, RR},   {na, NA, nA}, evMovlpsSpecies,      K7_2 },
        /* mod == 3 */  {   "movhlps", 2, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evMovhlpsSpecies,     K7_2 }
    };

    CInstr_Table S_group_2_13_table[4] =
    {
        /* no prefix */ {   "movlps",  2, {E, v, WW},   {G, v, nA},   {na, NA, nA}, evMovlpsSpecies,  K7_2},
        /* 66 prefix */ {   "movlpd",  2, {E, v, WW},   {G, v, nA},   {na, NA, nA}, evMovlpsSpecies,  K7_2},
        /* f2 prefix */ {         "",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,      K6 },
        /* f3 prefix */ {         "",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,      K6 }
    };

    CInstr_Table S_group_2_14_table[4] =
    {
        /* no prefix */ {  "unpcklps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evUnpcklpsSpecies,    K7_2 },
        /* 66 prefix */ {  "unpcklpd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evUnpcklpdSpecies,    K7_2 },
        /* f2 prefix */ {          "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,          K7_2 },
        /* f3 prefix */ {          "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,          K7_2 }
    };

    CInstr_Table S_group_2_15_table[4] =
    {
        /* no prefix */ {"unpckhps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evUnpcklpsSpecies,  K7_2 },
        /* 66 prefix */ {"unpckhpd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evUnpcklpdSpecies,  K7_2 },
        /* f2 prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K7_2 },
        /* f3 prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K7_2 }

    };

    CInstr_Table S_group_2_16_table[4] =
    {
        /* no prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6 },
        /* 66 prefix */ {  "movhpd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovhpsSpecies,    K7_2 },
        /* f2 prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6 },
        /* f3 prefix */ {        "", 0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,        K6 }
    };

    CInstr_Table S_group_2_16_00_table[2] =
    {
        /* mod !=3 */   {  "movhps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovhpsSpecies,    K7_2 },
        /* mod == 3 */  { "movlhps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovlhpsSpecies,   K7_2 }
    };

    CInstr_Table S_group_2_17_table[4] =
    {
        /* no prefix */ {   "movhps",  2, {E, v, WW},   {G, v, nA},   {na, NA, nA}, evMovhpsSpecies,  K7_2},
        /* 66 prefix */ {   "movhpd",  2, {E, v, WW},   {G, v, nA},   {na, NA, nA}, evMovhpsSpecies,  K7_2},
        /* f2 prefix */ {         "",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,      K6 },
        /* f3 prefix */ {         "",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,      K6 }
    };

    CInstr_Table S_group_2_18_table[16] =
    {
        {"prefetchnta", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evPrefetchntaSpecies,   K7},
        {"prefetcht0", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evPrefetcht0Species,    K7},
        {"prefetcht1", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evPrefetcht1Species,    K7},
        {"prefetcht2", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evPrefetcht2Species,    K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7},
        {"nop",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,            K7}
    };

    CInstr_Table S_group_2_28_table[4] =
    {
        /* no prefix */ {"movaps",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovapsSpecies, K7_2 },
        /* 66 prefix */ {"movapd",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovapdSpecies, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K7_2 }
    };

    CInstr_Table S_group_2_29_table[4] =
    {
        /* no prefix */ {"movaps",  2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovapsSpecies, K7_2 },
        /* 66 prefix */ {"movapd",  2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovapdSpecies, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K7_2 }
    };

    CInstr_Table S_group_2_2a_table[4] =
    {
        /* no prefix */ {"cvtpi2ps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtpi2psSpecies,   K7_2 },
        /* 66 prefix */ {"cvtpi2pd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtpi2pdSpecies,   K7_2 },
        /* f2 prefix */ {"cvtsi2sd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtsi2sdSpecies,   K7_2 },
        /* f3 prefix */ {"cvtsi2ss", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtsi2ssSpecies,   K7_2 }
    };

    CInstr_Table S_group_2_2b_table[4] =
    {
        /* no prefix */ {"movntps", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovntpsSpecies,    K7_2 },
        /* 66 prefix */ {"movntpd", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovntpdSpecies,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_2c_table[4] =
    {
        /* no prefix */ {"cvttps2pi", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvttps2piSpecies, K7_2 },
        /* 66 prefix */ {"cvttpd2pi", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvttpd2piSpecies, K7_2 },
        /* f2 prefix */ {"cvttsd2si", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvttsd2siSpecies, K7_2 },
        /* f3 prefix */ {"cvttss2si", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvttss2siSpecies, K7_2 }
    };

    CInstr_Table S_group_2_2d_table[4] =
    {
        /* no prefix */ {"cvtps2pi", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtps2piSpecies,   K7_2 },
        /* 66 prefix */ {"cvtpd2pi", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtpd2piSpecies,   K7_2 },
        /* f2 prefix */ {"cvtsd2si", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtsd2siSpecies,   K7_2 },
        /* f3 prefix */ {"cvtss2si", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtss2siSpecies,   K7_2 }
    };

    CInstr_Table S_group_2_2e_table[4] =
    {
        /* no prefix */ {"Ucomiss", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evUcomissSpecies,    K7_2 },
        /* 66 prefix */ {"Ucomisd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evUcomisdSpecies,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_2f_table[4] =
    {
        /* no prefix */ {"comiss",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evComissSpecies,     K7_2 },
        /* 66 prefix */ {"comisd",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evComisdSpecies,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_50_table[4] =
    {
        /* no prefix */ {"movmskps", 2, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evMovmskpsSpecies,   K7_2 },
        /* 66 prefix */ {"movmskpd", 2, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evMovmskpdSpecies,   K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_51_table[4] =
    {
        /* no prefix */ {"sqrtps",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSqrtpsSpecies,     K7_2 },
        /* 66 prefix */ {"sqrtpd",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSqrtpdSpecies,     K7_2 },
        /* f2 prefix */ {"sqrtsd",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSqrtsdSpecies,     K7_2 },
        /* f3 prefix */ {"sqrtss",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSqrtssSpecies,     K7_2 }
    };

    CInstr_Table S_group_2_52_table[4] =
    {
        /* no prefix */ {"rsqrtps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evRsqrtpsSpecies,    K7_2 },
        /* 66 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {"rsqrtss", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evRsqrtssSpecies,    K7_2 }
    };

    CInstr_Table S_group_2_53_table[4] =
    {
        /* no prefix */ {"rcpps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evRcppsSpecies,      K7_2 },
        /* 66 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {"rcpss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evRcpssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_54_table[4] =
    {
        /* no prefix */ {"andps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAndpsSpecies,      K7_2 },
        /* 66 prefix */ {"andpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAndpdSpecies,      K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_55_table[4] =
    {
        /* no prefix */ {"andnps",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAndnpsSpecies,     K7_2 },
        /* 66 prefix */ {"andnpd",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAndnpdSpecies,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_56_table[4] =
    {
        /* no prefix */ {"orps",    2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evOrpsSpecies,       K7_2 },
        /* 66 prefix */ {"orpd",    2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evOrpdSpecies,       K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_57_table[4] =
    {
        /* no prefix */ {"xorps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evXorpsSpecies,      K7_2 },
        /* 66 prefix */ {"xorpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evXorpdSpecies,      K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 }
    };

    CInstr_Table S_group_2_58_table[4] =
    {
        /* no prefix */ {"addps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddpsSpecies,      K7_2 },
        /* 66 prefix */ {"addpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddpdSpecies,      K7_2 },
        /* f2 prefix */ {"addsd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddsdSpecies,      K7_2 },
        /* f3 prefix */ {"addss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_59_table[4] =
    {
        /* no prefix */ {"mulpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddpsSpecies,      K7_2 },
        /* 66 prefix */ {"mulpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddpdSpecies,      K7_2 },
        /* f2 prefix */ {"mulsd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddsdSpecies,      K7_2 },
        /* f3 prefix */ {"mulss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evAddssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_5a_table[4] =
    {
        /* no prefix */ {"cvtps2pd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtps2pdSpecies,   K7_2 },
        /* 66 prefix */ {"cvtpd2ps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtpd2psSpecies,   K7_2 },
        /* f2 prefix */ {"cvtsd2ss", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtsd2ssSpecies,   K7_2 },
        /* f3 prefix */ {"cvtss2sd", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtss2sdSpecies,   K7_2 }
    };

    CInstr_Table S_group_2_5b_table[4] =
    {
        /* no prefix */ {"cvtdq2ps", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtdq2psSpecies,   K7_2 },
        /* 66 prefix */ {"cvtps2dq", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtps2dqSpecies,   K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K7_2 },
        /* f3 prefix */ {"cvttps2dq", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvttps2dqSpecies,  K7_2 }
    };

    CInstr_Table S_group_2_5c_table[4] =
    {
        /* no prefix */ {"subps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSubpsSpecies,      K7_2 },
        /* 66 prefix */ {"subpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSubpdSpecies,      K7_2 },
        /* f2 prefix */ {"subsd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSubsdSpecies,      K7_2 },
        /* f3 prefix */ {"subss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evSubssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_5d_table[4] =
    {
        /* no prefix */ {"minps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMinpsSpecies,      K7_2 },
        /* 66 prefix */ {"minpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMinpdSpecies,      K7_2 },
        /* f2 prefix */ {"minsd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMinsdSpecies,      K7_2 },
        /* f3 prefix */ {"minss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMinssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_5e_table[4] =
    {
        /* no prefix */ {"divps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evDivpsSpecies,      K7_2 },
        /* 66 prefix */ {"divpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evDivpdSpecies,      K7_2 },
        /* f2 prefix */ {"divsd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evDivsdSpecies,      K7_2 },
        /* f3 prefix */ {"divss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evDivssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_5f_table[4] =
    {
        /* no prefix */ {"maxps",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMaxpsSpecies,      K7_2 },
        /* 66 prefix */ {"maxpd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMaxpdSpecies,      K7_2 },
        /* f2 prefix */ {"maxsd",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMaxsdSpecies,      K7_2 },
        /* f3 prefix */ {"maxss",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMaxssSpecies,      K7_2 }
    };

    CInstr_Table S_group_2_60_table[4] =
    {
        /* no prefix */ {"punpcklbw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpcklbwSpecies,          K6},
        /* 66 prefix */ {"punpcklbw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpcklbw_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_61_table[4] =
    {
        /* no prefix */ {"punpcklwd",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpcklwdSpecies,          K6},
        /* 66 prefix */ {"punpcklwd",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpcklwd_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_62_table[4] =
    {
        /* no prefix */ {"punpckldq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckldqSpecies,          K6},
        /* 66 prefix */ {"punpckldq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckldq_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_63_table[4] =
    {
        /* no prefix */ {"packsswb",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPacksswbSpecies,           K6},
        /* 66 prefix */ {"packsswb",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPacksswb_MMX_SSE2Species,  K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_64_table[4] =
    {
        /* no prefix */ {"pcmpgtb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpgtbSpecies,                K6},
        /* 66 prefix */ {"pcmpgtb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpgtb_MMX_SSE2Species,       K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6 },
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6 }
    };


    CInstr_Table S_group_2_65_table[4] =
    {
        /* no prefix */ {"pcmpgtw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpgtwSpecies,                K6},
        /* 66 prefix */ {"pcmpgtw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpgtw_MMX_SSE2Species,       K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6 },
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6 }
    };

    CInstr_Table S_group_2_66_table[4] =
    {
        /* no prefix */ {"pcmpgtd", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpgtdSpecies,                K6},
        /* 66 prefix */ {"pcmpgtd", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpgtd_MMX_SSE2Species,       K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6 },
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6 }
    };

    CInstr_Table S_group_2_67_table[4] =
    {
        /* no prefix */ {"packuswb",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPackuswbSpecies,           K6},
        /* 66 prefix */ {"packuswb",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPackuswb_MMX_SSE2Species,  K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_68_table[4] =
    {
        /* no prefix */ {"punpckhbw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhbwSpecies,          K6},
        /* 66 prefix */ {"punpckhbw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhbw_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_69_table[4] =
    {
        /* no prefix */ {"punpckhwd",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhwdSpecies,          K6},
        /* 66 prefix */ {"punpckhwd",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhwd_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_6a_table[4] =
    {
        /* no prefix */ {"punpckhdq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhdqSpecies,          K6},
        /* 66 prefix */ {"punpckhdq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhdq_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_6b_table[4] =
    {
        /* no prefix */ {"packssdw",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPackssdwSpecies,           K6},
        /* 66 prefix */ {"packssdw",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPackssdw_MMX_SSE2Species,  K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_6c_table[4] =
    {
        /* no prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* 66 prefix */ {"punpcklqdq",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhbwSpecies,          K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_6d_table[4] =
    {
        /* no prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* 66 prefix */ {"punpckhqdq",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPunpckhdqSpecies,          K7_2},
        /* f2 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_6e_table[4] =
    {
        /* no prefix */ {"movd",    2, {P, q, nA},   {E, d, RR},   {na, NA, nA}, evMovdSpecies,                   K6},
        /* 66 prefix */ {"movd",    2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovd_MMX_SSE2Species,          K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K7_2 },
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K7_2 }
    };

    CInstr_Table S_group_2_6f_table[4] =
    {
        /* no prefix */ {"movq",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evMovqSpecies,                   K6},
        /* 66 prefix */ {"movdqa",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovdqaSpecies,                 K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                     K6},
        /* f3 prefix */ {"movdqu",  2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovdquSpecies,                 K7_2 }
    };

    CInstr_Table S_group_2_70_table[4] =
    {
        /* no prefix */ {"pshufw",  3, {P, q, nA},   {Q, q, RR},   {I, ib, nA},  evPshufwSpecies,                 K7},
        /* 66 prefix */ {"pshufd",  3, {P, q, nA},   {Q, q, RR},   {I, ib, nA},  evPshufdSpecies,                 K7_2},
        /* f2 prefix */ {"pshuflw", 3, {P, q, nA},   {Q, q, RR},   {I, ib, nA},  evPshuflwSpecies,                K7_2},
        /* f3 prefix */ {"pshufhw", 3, {P, q, nA},   {Q, q, RR},   {I, ib, nA},  evPshufhwSpecies,                K7_2}
    };

    CInstr_Table S_group_2_71_table[8] =
    {
        /* 0x00 */  {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        /* 0x01 */  {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        /* 0x02 */  {"psrlw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrlwSpecies,  K6},
        /* 0x03 */  {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        /* 0x04 */  {"psraw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrawSpecies,  K6},
        /* 0x05 */  {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        /* 0x06 */  {"psllw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsllwSpecies,  K6},
        /* 0x07 */  {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
    };

    CInstr_Table S_group_2_71_02_table[4] =
    {
        /* no prefix */ {"psrlw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrlwSpecies,          K6},
        /* 66 prefix */ {"psrlw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrlw_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_71_04_table[4] =
    {
        /* no prefix */ {"psraw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrawSpecies,          K6},
        /* 66 prefix */ {"psraw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsraw_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_71_06_table[4] =
    {
        /* no prefix */ {"psllw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsllwSpecies,          K6},
        /* 66 prefix */ {"psllw",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsllwSpecies,          K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_72_table[8] =
    {
        /* 0x00 */  {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,               K6},
        /* 0x01 */  {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,               K6},
        /* 0x02 */  {"psrld",     2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrldSpecies,            K6},
        /* 0x03 */  {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,               K6},
        /* 0x04 */  {"psrad",     2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsradSpecies,            K6},
        /* 0x05 */  {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,               K6},
        /* 0x06 */  {"pslld",     2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPslldSpecies,            K6},
        /* 0x07 */  {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,               K6}
    };

    CInstr_Table S_group_2_72_02_table[4] =
    {
        /* no prefix */ {"psrld",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrldSpecies,          K6},
        /* 66 prefix */ {"psrld",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrld_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_72_04_table[4] =
    {
        /* no prefix */ {"psrad",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsradSpecies,          K6},
        /* 66 prefix */ {"psrad",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrad_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_72_06_table[4] =
    {
        /* no prefix */ {"pslld",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPslldSpecies,          K6},
        /* 66 prefix */ {"pslld",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPslld_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_73_table[8] =
    {
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        {"psrlq",     2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrlqSpecies,  K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        {"psraq",     2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsraqSpecies,  K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6},
        {"psllq",     2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsllqSpecies,  K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,     K6}
    };

    CInstr_Table S_group_2_73_02_table[4] =
    {
        /* no prefix */ {"psrlq",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrlqSpecies,          K6},
        /* 66 prefix */ {"psrlq",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrlq_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_73_03_table[4] =
    {
        /* no prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* 66 prefix */ {"psrldq",  2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsrldqSpecies,         K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_73_06_table[4] =
    {
        /* no prefix */ {"psllq",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsllqSpecies,          K6},
        /* 66 prefix */ {"psllq",   2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPsllq_MMX_SSE2Species, K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_73_07_table[4] =
    {
        /* no prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* 66 prefix */ {"pslldq",  2, {R, q, nA},   {I, ib, nA},  {na, NA, nA}, evPslldqSpecies,         K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K6}
    };

    CInstr_Table S_group_2_74_table[4] =
    {
        /* no prefix */ {"pcmpeqb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpeqbSpecies,            K6},
        /* 66 prefix */ {"pcmpeqb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpeqb_MMX_SSE2Species,   K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_75_table[4] =
    {
        /* no prefix */ {"pcmpeqw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpeqwSpecies,            K6},
        /* 66 prefix */ {"pcmpeqw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpeqw_MMX_SSE2Species,   K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_76_table[4] =
    {
        /* no prefix */ {"pcmpeqd", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpeqdSpecies,            K6},
        /* 66 prefix */ {"pcmpeqd", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPcmpeqd_MMX_SSE2Species,   K7_2},
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_7e_table[4] =
    {
        /* no prefix */ {"movd",    2, {E, d, WW},   {P, q, nA},   {na, NA, nA}, evMovdSpecies,               K6},
        /* 66 prefix */ {"movd",    2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovdSpecies,               K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {"movq",    0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evMovq_MMX_SSE2Species,      K7_2}
    };

    CInstr_Table S_group_2_7f_table[4] =
    {
        /* no prefix */ {"movq",    2, {Q, q, WW},   {P, q, nA},   {na, NA, nA}, evMovqSpecies,               K6},
        /* 66 prefix */ {"movdqa",  2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovdqaSpecies,             K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K7_2 },
        /* f3 prefix */ {"movdqu",  2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovdquSpecies,             K7_2 }
    };

    CInstr_Table S_group_2_ae_00_table[16] =
    {
        {"fxsave",  1, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evFxsaveSpecies,     K7_2},
        {"fxrstor", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evFxrstorSpecies,    K7_2},
        {"ldmxcsr", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evLdmxcsrSpecies,    K7_2}, // Load Streaming SIMD Extension Control/Status
        {"stmxcsr", 1, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evStmxcsrSpecies,    K7_2}, //TODO: what's this? Store Streaming SIMD Extention Control/Status
        {"xsave",   1, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evXsaveSpecies,      K8},
        {"xrstor",  1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evXrstorSpecies,     K8},
        {"xsaveopt", 1, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evXsaveoptSpecies,  K8},
        {"clflush", 1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evClflushSpecies,    K7_2},
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"lfence",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evLfenceSpecies,     K8},
        {"mfence",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evMfenceSpecies,     K8},
        {"sfence",  0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evSfenceSpecies,     K7}
    };
    CInstr_Table S_group_2_ae_03_08_table[2] =
    {
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"rdfsbase",  1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evNASpecies,       K8}
    };
    CInstr_Table S_group_2_ae_03_09_table[2] =
    {
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"rdgsbase",  1, {na, NA, RR}, {na, NA, nA}, {na, NA, nA}, evNASpecies,       K8}
    };
    CInstr_Table S_group_2_ae_03_10_table[2] =
    {
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"wrfsbase",  1, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evNASpecies,       K8}
    };
    CInstr_Table S_group_2_ae_03_11_table[2] =
    {
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"wrgsbase",  1, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evNASpecies,       K8}
    };

    CInstr_Table S_group_2_ba_table[8] =
    {
        {"",          0, {na, NA, nA}, {I, b, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA}, {I, b, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA}, {I, b, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"",          0, {na, NA, nA}, {I, b, nA},   {na, NA, nA}, evNASpecies,     K6},
        {"bt",        2, {E, v, RR},   {I, ib, nA},  {na, NA, nA}, evBtSpecies,     K6},
        {"bts",       2, {E, v, RW},   {I, ib, nA},  {na, NA, nA}, evBtsSpecies,    K6},
        {"btr",       2, {E, v, RW},   {I, ib, nA},  {na, NA, nA}, evBtrSpecies,    K6},
        {"btc",       2, {E, v, RW},   {I, ib, nA},  {na, NA, nA}, evBtcSpecies,    K6}
    };

    CInstr_Table S_group_2_c2_table[4] =
    {
        /* no prefix */ {"cmpps",   3, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCmppsSpecies,              K7_2 },
        /* 66 prefix */ {"cmppd",   3, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCmppdSpecies,              K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K7_2 },
        /* f3 prefix */ {"cmpss",   3, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCmpssSpecies,              K7_2 }
    };

    CInstr_Table S_group_2_c4_table[4] =
    {
        /* no prefix */ {"pinsrw",  3, {P, q, nA},   {E, dw, RR},  {I, ib, nA},  evPinsrwSpecies,             K7},
        /* 66 prefix */ {"pinsrw",  3, {P, q, nA},   {E, dw, RR},  {I, ib, nA},  evPinsrw_MMXExt_SSE2Species, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_c5_table[4] =
    {
        /* no prefix */ {"pextrw",  3, {G, d, nA},   {R, q, nA},   {I, ib, nA},  evPextrwSpecies,             K7},
        /* 66 prefix */ {"pextrw",  3, {G, d, nA},   {R, q, nA},   {I, ib, nA},  evPextrw_MMXExt_SSE2Species, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 }
    };

    CInstr_Table S_group_2_c6_table[4] =
    {
        /* no prefix */ {"shufps",  3, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evShufpsSpecies,             K7_2 },
        /* 66 prefix */ {"shufpd",  3, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evShufpdSpecies,             K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_c7_table[8] =
    {
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},

        // TODO: Not sure about the access type?! TSIM knows it if we set type to RW.
        {"cmpxchg8b", 3, {_EDX, q, nA}, {_EAX, q, nA}, {M, q, RW},   evCmpxchg8bSpecies,  K6},

        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",          0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6}
    };

    CInstr_Table S_group_2_d1_table[4] =
    {
        /* no prefix */ {"psrlw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrlwSpecies,              K6},
        /* 66 prefix */ {"psrlw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrlw_MMX_SSE2Species,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d2_table[4] =
    {
        /* no prefix */ {"psrld",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrldSpecies,              K6},
        /* 66 prefix */ {"psrld",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrld_MMX_SSE2Species,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d3_table[4] =
    {
        /* no prefix */ {"psrlq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrlqSpecies,              K6},
        /* 66 prefix */ {"psrlq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrlq_MMX_SSE2Species,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d4_table[4] =
    {
        /* no prefix */ {"paddq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddqSpecies,              K7},
        /* 66 prefix */ {"paddq",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddq_SSE2_MMXSpecies,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d5_table[4] =
    {
        /* no prefix */ {"pmullw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmullwSpecies,             K6},
        /* 66 prefix */ {"pmullw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmullw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d6_table[4] =
    {
        /* no prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6 },
        /* 66 prefix */ {"movq",    2, {Q, q, WW},   {P, q, nA},   {na, NA, nA}, evMovq_MMX_SSE2Species,      K7_2 },
        /* f2 prefix */ {"movdq2q", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovdq2qSpecies,            K7_2 },
        /* f3 prefix */ {"movq2dq", 2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evMovq2dqSpecies,            K7_2 }
    };

    CInstr_Table S_group_2_d7_table[4] =
    {
        /* no prefix */ {"pmovmskb",    2, {G, d, nA},   {R, q, nA},   {na, NA, nA}, evPmovmskbSpecies,           K7},
        /* 66 prefix */ {"pmovmskb",    2, {G, d, nA},   {R, q, nA},   {na, NA, nA}, evPmovmskb_MMXExt_SSE2Species,   K7_2 },
        /* f2 prefix */ {     "",       0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",       0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d8_table[4] =
    {
        /* no prefix */ {"psubusb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubusbSpecies,            K6},
        /* 66 prefix */ {"psubusb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubusb_MMX_SSE2Species,   K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_d9_table[4] =
    {
        /* no prefix */ {"psubusw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubuswSpecies,            K6},
        /* 66 prefix */ {"psubusw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubusw_MMX_SSE2Species,   K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_da_table[4] =
    {
        /* no prefix */ {"pminub",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPminubSpecies,             K7},
        /* 66 prefix */ {"pminub",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPminub_MMXExt_SSE2Species, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_db_table[4] =
    {
        /* no prefix */ {"pand",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPandSpecies,               K6},
        /* 66 prefix */ {"pand",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPand_MMX_SSE2Species,      K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_dc_table[4] =
    {
        /* no prefix */ {"paddusb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddusbSpecies,            K6},
        /* 66 prefix */ {"paddusb", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddusb_MMX_SSE2Species,   K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_dd_table[4] =
    {
        /* no prefix */ {"paddusw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPadduswSpecies,            K6},
        /* 66 prefix */ {"paddusw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddusw_MMX_SSE2Species,   K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_de_table[4] =
    {
        /* no prefix */ {"pmaxub",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmaxubSpecies,             K7},
        /* 66 prefix */ {"pmaxub",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmaxub_MMXExt_SSE2Species, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_df_table[4] =
    {
        /* no prefix */ {"pandn",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPandnSpecies,              K6},
        /* 66 prefix */ {"pandn",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPandn_MMX_SSE2Species,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e0_table[4] =
    {
        /* no prefix */ {"pavgb",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPavgbSpecies,              K7},
        /* 66 prefix */ {"pavgb",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPavgb_MMXExt_SSE2Species,  K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e1_table[4] =
    {
        /* no prefix */ {"psraw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrawSpecies,              K6},
        /* 66 prefix */ {"psraw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsraw_MMX_SSE2Species,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e2_table[4] =
    {
        /* no prefix */ {"psrad",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsradSpecies,              K6},
        /* 66 prefix */ {"psrad",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsrad_MMX_SSE2Species,     K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e3_table[4] =
    {
        /* no prefix */ {"pavgw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPavgwSpecies,              K7},
        /* 66 prefix */ {"pavgw",   2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPavgw_MMXExt_SSE2Species,  K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e4_table[4] =
    {
        /* no prefix */ {"pmulhuw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmulhuwSpecies,            K7},
        /* 66 prefix */ {"pmulhuw", 2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmulhuw_MMXExt_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e5_table[4] =
    {
        /* no prefix */ {"pmulhw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmulhwSpecies,             K6},
        /* 66 prefix */ {"pmulhw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmulhw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e6_table[4] =
    {
        /* no prefix */ {"",            0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,             K7_2 },
        /* 66 prefix */ {"cvttpd2dq",   2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvttpd2dqSpecies,      K7_2 },
        /* f2 prefix */ {"cvtpd2dq",    2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtpd2dqSpecies,       K7_2},
        /* f3 prefix */ {"cvtdq2pd",    2, {na, NA, nA}, {na, NA, RR}, {na, NA, nA}, evCvtdq2pdSpecies,       K7_2 }
    };

    CInstr_Table S_group_2_e7_table[4] =
    {
        {"movntq",  2, {M, q, WW},   {P, q, nA},   {na, NA, nA}, evMovntqSpecies,     K7},
        {"movntdq", 2, {na, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMovntdqSpecies,    K7_2 },
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6},
        {"",        0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,         K6}
    };

    CInstr_Table S_group_2_e8_table[4] =
    {
        /* no prefix */ {"psubsb",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubsbSpecies,             K6},
        /* 66 prefix */ {"psubsb",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubsb_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_e9_table[4] =
    {
        /* no prefix */ {"psubsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubswSpecies,             K6},
        /* 66 prefix */ {"psubsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPsubsw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_ea_table[4] =
    {
        /* no prefix */ {"pminsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPminswSpecies,             K7},
        /* 66 prefix */ {"pminsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPminsw_MMXExt_SSE2Species, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_eb_table[4] =
    {
        /* no prefix */ {"por",     2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPorSpecies,                K6},
        /* 66 prefix */ {"por",     2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPor_MMX_SSE2Species,       K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_ec_table[4] =
    {
        /* no prefix */ {"paddsb",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddsbSpecies,             K6},
        /* 66 prefix */ {"paddsb",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddsb_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_ed_table[4] =
    {
        /* no prefix */ {"paddsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddswSpecies,             K6},
        /* 66 prefix */ {"paddsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPaddsw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_ee_table[4] =
    {
        /* no prefix */ {"pmaxsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmaxswSpecies,             K7},
        /* 66 prefix */ {"pmaxsw",  2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPmaxsw_MMXExt_SSE2Species, K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_ef_table[4] =
    {
        /* no prefix */ {"pxor",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPxorSpecies,               K6},
        /* 66 prefix */ {"pxor",    2, {P, q, nA},   {Q, q, RR},   {na, NA, nA}, evPxor_MMX_SSE2Species,      K7_2 },
        /* f2 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6},
        /* f3 prefix */ {     "",   0, {na, NA, nA}, {na, NA, nA}, {na, NA, nA}, evNASpecies,                 K6}
    };

    CInstr_Table S_group_2_f1_table[4] =
    {
        /* no prefix */ {"psllw",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsllwSpecies,             K6},
        /* 66 prefix */ {"psllw",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsllw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f2_table[4] =
    {
        /* no prefix */ {"pslld",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPslldSpecies,             K6},
        /* 66 prefix */ {"pslld",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPslld_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f3_table[4] =
    {
        /* no prefix */ {"psllq",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsllqSpecies,             K6},
        /* 66 prefix */ {"psllq",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsllq_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f4_table[4] =
    {
        /* no prefix */ {"pmuludq", 2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPmuludq_SSE2_SSESpecies,  K6},
        /* 66 prefix */ {"pmuludq", 2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPmuludqSpecies,           K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f5_table[4] =
    {
        /* no prefix */ {"pmaddwd", 2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPmaddwdSpecies,           K6},
        /* 66 prefix */ {"pmaddwd", 2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPmaddwd_MMX_SSE2Species,  K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f6_table[4] =
    {
        /* no prefix */ {"psadbw",  2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsadbwSpecies,            K7},
        /* 66 prefix */ {"psadbw",  2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsadbw_MMXExt_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f7_table[4] =
    {
        /* no prefix */ {"maskmovq",    3, {_EDI, NA, WW}, {P, q, nA},   {R, q, nA},   evMaskmovqSpecies,      K7}, //TODO: eDI is implied first operand here!   i.e. maskmovq EDI, MM1, MM2
        /* 66 prefix */ {"maskmovdqu",  3, {_EDI, NA, WW}, {na, NA, nA}, {na, NA, nA}, evMaskmovdquSpecies,    K7_2 }, // TODO: EDI implicit memeory.
        /* f2 prefix */ {"",            0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,            K6},
        /* f3 prefix */ {"",            0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,            K6}
    };

    CInstr_Table S_group_2_f8_table[4] =
    {
        /* no prefix */ {"psubb",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubbSpecies,             K6},
        /* 66 prefix */ {"psubb",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubb_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_f9_table[4] =
    {
        /* no prefix */ {"psubw",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubwSpecies,             K6},
        /* 66 prefix */ {"psubw",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_fa_table[4] =
    {
        /* no prefix */ {"psubd",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubdSpecies,             K6},
        /* 66 prefix */ {"psubd",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubd_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_fb_table[4] =
    {
        /* no prefix */ {"psubq",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubqSpecies,             K6},
        /* 66 prefix */ {"psubq",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPsubq_SSE2_SSESpecies,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_fc_table[4] =
    {
        /* no prefix */ {"paddb",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPaddbSpecies,             K6},
        /* 66 prefix */ {"paddb",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPaddb_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_fd_table[4] =
    {
        /* no prefix */ {"paddw",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPaddwSpecies,             K6},
        /* 66 prefix */ {"paddw",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPaddw_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    CInstr_Table S_group_2_fe_table[4] =
    {
        /* no prefix */ {"paddd",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPadddSpecies,             K6},
        /* 66 prefix */ {"paddd",   2, {P, q, nA},    {Q, q, RR},   {na, NA, nA}, evPaddd_MMX_SSE2Species,    K7_2 },
        /* f2 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6},
        /* f3 prefix */ {"",        0, {na, NA, nA},  {na, NA, nA}, {na, NA, nA}, evNASpecies,                K6}
    };

    int i;

    for (i = 0; i < 256; i++)
    {
        S_oneByteOpcodes_extra_tbl[i] = S_oneByteOpcodes_table[i];
        S_twoByteOpcodes_extra_tbl[i] = S_twoByteOpcodes_table[i];
    }

    for (i = 0; i < 3; i++)
    {
        S_group_1_60_extra_tbl[i] = S_group_1_60_table[i];
        S_group_1_61_extra_tbl[i] = S_group_1_61_table[i];
        S_group_1_90_extra_tbl[i] = S_group_1_90_table[i];
        S_group_1_98_extra_tbl[i] = S_group_1_98_table[i];
        S_group_1_99_extra_tbl[i] = S_group_1_99_table[i];
        S_group_1_9c_extra_tbl[i] = S_group_1_9c_table[i];
        S_group_1_9d_extra_tbl[i] = S_group_1_9d_table[i];
        S_group_1_a5_extra_tbl[i] = S_group_1_a5_table[i];
        S_group_1_a7_extra_tbl[i] = S_group_1_a7_table[i];
        S_group_1_ab_extra_tbl[i] = S_group_1_ab_table[i];
        S_group_1_ad_extra_tbl[i] = S_group_1_ad_table[i];
        S_group_1_af_extra_tbl[i] = S_group_1_af_table[i];
        S_group_1_c2_extra_tbl[i] = S_group_1_c2_table[i];
        S_group_1_c3_extra_tbl[i] = S_group_1_c3_table[i];
        S_group_1_ca_extra_tbl[i] = S_group_1_ca_table[i];
        S_group_1_cb_extra_tbl[i] = S_group_1_cb_table[i];
        S_group_1_cf_extra_tbl[i] = S_group_1_cf_table[i];
        S_group_1_e3_extra_tbl[i] = S_group_1_e3_table[i];
    }

    for (i = 0; i < 2; i++)
    {
        S_group_1_63_extra_tbl[i] = S_group_1_63_table[i];
        S_group_1_6d_extra_tbl[i] = S_group_1_6d_table[i];
        S_group_1_6f_extra_tbl[i] = S_group_1_6f_table[i];
        S_group_1_fa_extra_tbl[i] = S_group_1_fa_table[i];
        S_group_1_fb_extra_tbl[i] = S_group_1_fb_table[i];
        S_group_2_0d_extra_tbl[i] = S_group_2_0d_table[i];
        S_group_2_12_00_extra_tbl[i] = S_group_2_12_00_table[i];
        S_group_2_16_00_extra_tbl[i] = S_group_2_16_00_table[i];
    }

    for (i = 0; i < 8; i++)
    {
        S_group_1_80_extra_tbl[i] = S_group_1_80_table[i];
        S_group_1_81_extra_tbl[i] = S_group_1_81_table[i];
        S_group_1_82_extra_tbl[i] = S_group_1_82_table[i];
        S_group_1_83_extra_tbl[i] = S_group_1_83_table[i];
        S_group_1_c0_extra_tbl[i] = S_group_1_c0_table[i];
        S_group_1_c1_extra_tbl[i] = S_group_1_c1_table[i];
        S_group_1_d0_extra_tbl[i] = S_group_1_d0_table[i];
        S_group_1_d1_extra_tbl[i] = S_group_1_d1_table[i];
        S_group_1_d2_extra_tbl[i] = S_group_1_d2_table[i];
        S_group_1_d3_extra_tbl[i] = S_group_1_d3_table[i];
        S_group_1_f6_extra_tbl[i] = S_group_1_f6_table[i];
        S_group_1_f7_extra_tbl[i] = S_group_1_f7_table[i];
        S_group_1_fe_extra_tbl[i] = S_group_1_fe_table[i];
        S_group_1_ff_extra_tbl[i] = S_group_1_ff_table[i];
        S_group_2_00_extra_tbl[i] = S_group_2_00_table[i];
        S_group_2_01_extra_tbl[i] = S_group_2_01_table[i];
        S_group_2_ba_extra_tbl[i] = S_group_2_ba_table[i];
        S_group_2_c7_extra_tbl[i] = S_group_2_c7_table[i];
        S_group_2_71_extra_tbl[i] = S_group_2_71_table[i];
        S_group_2_72_extra_tbl[i] = S_group_2_72_table[i];
        S_group_2_73_extra_tbl[i] = S_group_2_73_table[i];
    }



    for (i = 0; i < 72; i++)
    {
        S_group_1_d8_extra_tbl[i] = S_group_1_d8_table[i];
        S_group_1_d9_extra_tbl[i] = S_group_1_d9_table[i];
        S_group_1_da_extra_tbl[i] = S_group_1_da_table[i];
        S_group_1_db_extra_tbl[i] = S_group_1_db_table[i];
        S_group_1_dc_extra_tbl[i] = S_group_1_dc_table[i];
        S_group_1_dd_extra_tbl[i] = S_group_1_dd_table[i];
        S_group_1_de_extra_tbl[i] = S_group_1_de_table[i];
        S_group_1_df_extra_tbl[i] = S_group_1_df_table[i];
    }

    for (i = 0; i < 24; i++)
    {
        S_group_2_0f_extra_tbl[i] = S_group_2_0f_table[i];
    }

    for (i = 0; i < 16; i++)
    {
        S_group_2_18_extra_tbl[i] = S_group_2_18_table[i];    // includes special case handling of PPro nop
        S_group_2_ae_00_extra_tbl[i] = S_group_2_ae_00_table[i];
    }

    for (i = 0; i < 2; i++)
    {
        S_group_2_ae_03_08_extra_tbl[i] = S_group_2_ae_03_08_table[i];
        S_group_2_ae_03_09_extra_tbl[i] = S_group_2_ae_03_09_table[i];
        S_group_2_ae_03_10_extra_tbl[i] = S_group_2_ae_03_10_table[i];
        S_group_2_ae_03_11_extra_tbl[i] = S_group_2_ae_03_11_table[i];
    }

    for (i = 0; i < 4; i++)
    {
        S_group_2_10_extra_tbl[i] = S_group_2_10_table[i];
        S_group_2_11_extra_tbl[i] = S_group_2_11_table[i];
        S_group_2_12_extra_tbl[i] = S_group_2_12_table[i];
        S_group_2_13_extra_tbl[i] = S_group_2_13_table[i];
        S_group_2_14_extra_tbl[i] = S_group_2_14_table[i];
        S_group_2_15_extra_tbl[i] = S_group_2_15_table[i];
        S_group_2_16_extra_tbl[i] = S_group_2_16_table[i];
        S_group_2_17_extra_tbl[i] = S_group_2_17_table[i];
        S_group_2_28_extra_tbl[i] = S_group_2_28_table[i];
        S_group_2_29_extra_tbl[i] = S_group_2_29_table[i];
        S_group_2_2a_extra_tbl[i] = S_group_2_2a_table[i];
        S_group_2_2b_extra_tbl[i] = S_group_2_2b_table[i];
        S_group_2_2c_extra_tbl[i] = S_group_2_2c_table[i];
        S_group_2_2d_extra_tbl[i] = S_group_2_2d_table[i];
        S_group_2_2e_extra_tbl[i] = S_group_2_2e_table[i];
        S_group_2_2f_extra_tbl[i] = S_group_2_2f_table[i];
        S_group_2_50_extra_tbl[i] = S_group_2_50_table[i];
        S_group_2_51_extra_tbl[i] = S_group_2_51_table[i];
        S_group_2_52_extra_tbl[i] = S_group_2_52_table[i];
        S_group_2_53_extra_tbl[i] = S_group_2_53_table[i];
        S_group_2_54_extra_tbl[i] = S_group_2_54_table[i];
        S_group_2_55_extra_tbl[i] = S_group_2_55_table[i];
        S_group_2_56_extra_tbl[i] = S_group_2_56_table[i];
        S_group_2_57_extra_tbl[i] = S_group_2_57_table[i];
        S_group_2_58_extra_tbl[i] = S_group_2_58_table[i];
        S_group_2_59_extra_tbl[i] = S_group_2_59_table[i];
        S_group_2_5a_extra_tbl[i] = S_group_2_5a_table[i];
        S_group_2_5b_extra_tbl[i] = S_group_2_5b_table[i];
        S_group_2_5c_extra_tbl[i] = S_group_2_5c_table[i];
        S_group_2_5d_extra_tbl[i] = S_group_2_5d_table[i];
        S_group_2_5e_extra_tbl[i] = S_group_2_5e_table[i];
        S_group_2_5f_extra_tbl[i] = S_group_2_5f_table[i];
        S_group_2_60_extra_tbl[i] = S_group_2_60_table[i];
        S_group_2_61_extra_tbl[i] = S_group_2_61_table[i];
        S_group_2_62_extra_tbl[i] = S_group_2_62_table[i];
        S_group_2_63_extra_tbl[i] = S_group_2_63_table[i];
        S_group_2_64_extra_tbl[i] = S_group_2_64_table[i];
        S_group_2_65_extra_tbl[i] = S_group_2_65_table[i];
        S_group_2_66_extra_tbl[i] = S_group_2_66_table[i];
        S_group_2_67_extra_tbl[i] = S_group_2_67_table[i];
        S_group_2_68_extra_tbl[i] = S_group_2_68_table[i];
        S_group_2_69_extra_tbl[i] = S_group_2_69_table[i];
        S_group_2_6a_extra_tbl[i] = S_group_2_6a_table[i];
        S_group_2_6b_extra_tbl[i] = S_group_2_6b_table[i];
        S_group_2_6c_extra_tbl[i] = S_group_2_6c_table[i];
        S_group_2_6d_extra_tbl[i] = S_group_2_6d_table[i];
        S_group_2_6e_extra_tbl[i] = S_group_2_6e_table[i];
        S_group_2_6f_extra_tbl[i] = S_group_2_6f_table[i];
        S_group_2_70_extra_tbl[i] = S_group_2_70_table[i];
        S_group_2_71_02_extra_tbl[i] = S_group_2_71_02_table[i];
        S_group_2_71_04_extra_tbl[i] = S_group_2_71_04_table[i];
        S_group_2_71_06_extra_tbl[i] = S_group_2_71_06_table[i];
        S_group_2_72_02_extra_tbl[i] = S_group_2_72_02_table[i];
        S_group_2_72_04_extra_tbl[i] = S_group_2_72_04_table[i];
        S_group_2_72_06_extra_tbl[i] = S_group_2_72_06_table[i];
        S_group_2_73_02_extra_tbl[i] = S_group_2_73_02_table[i];
        S_group_2_73_03_extra_tbl[i] = S_group_2_73_03_table[i];
        S_group_2_73_06_extra_tbl[i] = S_group_2_73_06_table[i];
        S_group_2_73_07_extra_tbl[i] = S_group_2_73_07_table[i];
        S_group_2_74_extra_tbl[i] = S_group_2_74_table[i];
        S_group_2_75_extra_tbl[i] = S_group_2_75_table[i];
        S_group_2_76_extra_tbl[i] = S_group_2_76_table[i];
        S_group_2_7e_extra_tbl[i] = S_group_2_7e_table[i];
        S_group_2_7f_extra_tbl[i] = S_group_2_7f_table[i];
        S_group_2_c2_extra_tbl[i] = S_group_2_c2_table[i];
        S_group_2_c4_extra_tbl[i] = S_group_2_c4_table[i];
        S_group_2_c5_extra_tbl[i] = S_group_2_c5_table[i];
        S_group_2_c6_extra_tbl[i] = S_group_2_c6_table[i];
        S_group_2_d1_extra_tbl[i] = S_group_2_d1_table[i];
        S_group_2_d2_extra_tbl[i] = S_group_2_d2_table[i];
        S_group_2_d3_extra_tbl[i] = S_group_2_d3_table[i];
        S_group_2_d4_extra_tbl[i] = S_group_2_d4_table[i];
        S_group_2_d5_extra_tbl[i] = S_group_2_d5_table[i];
        S_group_2_d6_extra_tbl[i] = S_group_2_d6_table[i];
        S_group_2_d7_extra_tbl[i] = S_group_2_d7_table[i];
        S_group_2_d8_extra_tbl[i] = S_group_2_d8_table[i];
        S_group_2_d9_extra_tbl[i] = S_group_2_d9_table[i];
        S_group_2_da_extra_tbl[i] = S_group_2_da_table[i];
        S_group_2_db_extra_tbl[i] = S_group_2_db_table[i];
        S_group_2_dc_extra_tbl[i] = S_group_2_dc_table[i];
        S_group_2_dd_extra_tbl[i] = S_group_2_dd_table[i];
        S_group_2_de_extra_tbl[i] = S_group_2_de_table[i];
        S_group_2_df_extra_tbl[i] = S_group_2_df_table[i];
        S_group_2_e0_extra_tbl[i] = S_group_2_e0_table[i];
        S_group_2_e1_extra_tbl[i] = S_group_2_e1_table[i];
        S_group_2_e2_extra_tbl[i] = S_group_2_e2_table[i];
        S_group_2_e3_extra_tbl[i] = S_group_2_e3_table[i];
        S_group_2_e4_extra_tbl[i] = S_group_2_e4_table[i];
        S_group_2_e5_extra_tbl[i] = S_group_2_e5_table[i];
        S_group_2_e6_extra_tbl[i] = S_group_2_e6_table[i];
        S_group_2_e7_extra_tbl[i] = S_group_2_e7_table[i];
        S_group_2_e8_extra_tbl[i] = S_group_2_e8_table[i];
        S_group_2_e9_extra_tbl[i] = S_group_2_e9_table[i];
        S_group_2_ea_extra_tbl[i] = S_group_2_ea_table[i];
        S_group_2_eb_extra_tbl[i] = S_group_2_eb_table[i];
        S_group_2_ec_extra_tbl[i] = S_group_2_ec_table[i];
        S_group_2_ed_extra_tbl[i] = S_group_2_ed_table[i];
        S_group_2_ee_extra_tbl[i] = S_group_2_ee_table[i];
        S_group_2_ef_extra_tbl[i] = S_group_2_ef_table[i];
        S_group_2_f1_extra_tbl[i] = S_group_2_f1_table[i];
        S_group_2_f2_extra_tbl[i] = S_group_2_f2_table[i];
        S_group_2_f3_extra_tbl[i] = S_group_2_f3_table[i];
        S_group_2_f4_extra_tbl[i] = S_group_2_f4_table[i];
        S_group_2_f5_extra_tbl[i] = S_group_2_f5_table[i];
        S_group_2_f6_extra_tbl[i] = S_group_2_f6_table[i];
        S_group_2_f7_extra_tbl[i] = S_group_2_f7_table[i];
        S_group_2_f8_extra_tbl[i] = S_group_2_f8_table[i];
        S_group_2_f9_extra_tbl[i] = S_group_2_f9_table[i];
        S_group_2_fa_extra_tbl[i] = S_group_2_fa_table[i];
        S_group_2_fb_extra_tbl[i] = S_group_2_fb_table[i];
        S_group_2_fc_extra_tbl[i] = S_group_2_fc_table[i];
        S_group_2_fd_extra_tbl[i] = S_group_2_fd_table[i];
        S_group_2_fe_extra_tbl[i] = S_group_2_fe_table[i];
    }
}



/////////////////////////////////////////////////////////
// The following are function overloading.
//
int CDisasmwrapper::GetLength()
{
    return CDisassembler::GetLength();
}

int CDisasmwrapper::GetNumOpcodeBytes()
{
    return CDisassembler::GetNumOpcodeBytes();
}

AMD_UINT8 CDisasmwrapper::GetOpcode(int index)
{
    return CDisassembler::GetOpcode(index);
}

AMD_UINT8 CDisasmwrapper::GetModrm()
{
    return CDisassembler::GetModrm();
}

AMD_INT64 CDisasmwrapper::GetDisplacement()
{
    return CDisassembler::GetDisplacement();
}

int CDisasmwrapper::GetNumOperands()
{
    return CDisassembler::GetNumOperands();
}

n_Disassembler::e_OperandType CDisasmwrapper::GetOperandType(int operand)
{
#ifdef _WINDOWS
    return CDisassembler::GetOperandType(operand);
#else
    return CAttDisassembler::GetOperandType(operand);
#endif
}

n_Disassembler::e_OperandSize CDisasmwrapper::GetOperandSize(int operand)
{
#ifdef _WINDOWS
    return CDisassembler::GetOperandSize(operand);
#else
    return CAttDisassembler::GetOperandSize(operand);
#endif
}

n_Disassembler::e_Registers CDisasmwrapper::GetRegister(int operand)
{
#ifdef _WINDOWS
    return CDisassembler::GetRegister(operand);
#else
    return CAttDisassembler::GetRegister(operand);
#endif
}

void CDisasmwrapper::SetDbit(bool dbit)
{
    return CDisassembler::SetDbit(dbit);
}

void CDisasmwrapper::SetLongMode(bool longmode)
{
#ifdef _WINDOWS
    return CDisassembler::SetLongMode(longmode);
#else
    return CAttDisassembler::SetLongMode(longmode);
#endif
}