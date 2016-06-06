//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AttDisassembler.h
/// \brief AT&T Style implementation of the Disassembler class.
///
//==================================================================================

#ifndef _AT_T_DISASSEMBLER_H_
#define _AT_T_DISASSEMBLER_H_
//

#include "Disassembler.h"


class CAttDisassembler : public CDisassembler
{
private:
    static bool m_bTablesUpdated;
    char* m_hexPrefix;
    char* m_immedPrefix;

    static const char* S_size_att_operands[8]; //"b", "w", "l", none, "q", ,none, none, none
    PVOIDMEMBERFUNC restore_DisassembleOperandFnPtrs[n_Disassembler::OPRND_UxM2 + 1];
    inline const char* GetAttSize(n_Disassembler::e_OperandSize size) { return (S_size_att_operands[size]); };
    inline bool GetNoSizeOp(UINT8 ui8Operand);
    inline bool GetNoSizeInst();

    static const char* S_PrefixByteStrings_att[];
    static const char* S_modrm16_str_att[];
    static const char* S_rex_byte_regs_att[];
    static const char* S_byte_regs_att[];
    static const char* S_word_regs_att[];
    static const char* S_dword_regs_att[];
    static const char* S_qword_regs_att[];
    static const char* S_control_regs_att[];
    static const char* S_debug_regs_att[];
    static const char* S_segment_regs_att[];
    static const char* S_fpu_regs_att[];    // "st0", "st1", ..., "st"  (stack top is last)
    static const char* S_mmx_regs_att[];
    static const char* S_xmmx_regs_att[];

public:
    CAttDisassembler();
    virtual ~CAttDisassembler();
    //Overrides for AT&T syntax
    char* Disassemble();
    char* Disassemble(const UINT8* inst_buf);
    char* Disassemble(const UINT8* inst_buf, UINT64 rip);
    char* Disassemble(const UINT8* inst_buf, bool dbit) { SetDbit(dbit); return Disassemble(inst_buf); };

    void SetLongMode(bool longmode = true);
    void ClearLongMode();

    inline void ModrmStr() { (m_modrm_mod == 3) ? RegisterModrmStr() : MemoryModrmStr(); }
    void MemoryModrmStr();
    inline void MemoryModrmStrWithSIB();
    inline void MemoryModrmStrWithoutSIB();

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
    inline void MMXModrmStr();
    inline void MMXRegAndByteImmediateStr();
    inline void MMXModrmAndByteImmediateStr();
    inline void DsEsiStr();
    inline void EsEdiStr();
    inline void SimdModrmStr();
    inline void RegeBXAndALStr();

    // Overrides to filter out errant queries
    n_Disassembler::e_OperandType GetOperandType(UINT32 operand)
    {
        return ((operand < GetNumOperands()) ? CDisassembler::GetOperandType(operand) : n_Disassembler::OPERANDTYPE_NONE);
    }

    inline n_Disassembler::e_OperandSize GetOperandSize(UINT32 operand)
    {
        return ((operand < GetNumOperands()) ? CDisassembler::GetOperandSize(operand) : n_Disassembler::OPERANDSIZE_NONE);
    }

    inline n_Disassembler::e_Registers GetRegister(UINT32 operand)
    {
        return ((operand < GetNumOperands()) ? CDisassembler::GetRegister(operand) : n_Disassembler::REG_NONE);
    }

    bool OperandHasIndex(UINT32 operand)
    {
        return ((operand < GetNumOperands()) ? CDisassembler::OperandHasIndex(operand) : false);
    }

    n_Disassembler::e_Registers GetOperandIndexRegister(UINT32 operand)
    {
        return ((operand < GetNumOperands()) ? CDisassembler::GetOperandIndexRegister(operand) : n_Disassembler::REG_NONE);
    }


#ifdef _DEBUG
    // debug routines print out operand info
    void ShowOperands();
    char* OperandSizeString(n_Disassembler::e_OperandSize size);
    char* OperandTypeString(n_Disassembler::e_OperandType type);
    char* RegisterString(n_Disassembler::e_Registers reg);
#endif
};

#endif //_AT_T_DISASSEMBLER_H_

