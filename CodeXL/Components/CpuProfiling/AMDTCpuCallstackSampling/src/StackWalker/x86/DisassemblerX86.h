//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisassemblerX86.h
///
//==================================================================================

#ifndef _DISASSEMBLERX86_H_
#define _DISASSEMBLERX86_H_

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#define MAKE_WORD(a, b)         ((gtUInt16)(((gtUByte)(((gtUInt32)(a)) & 0xFF)) | ((gtUInt16)((gtUByte)(((gtUInt32)(b)) & 0xFF))) << 8))
#define MAKE_DWORD(a, b, c, d)  ((gtUInt32)(MAKE_WORD(a, b) | ((gtUInt32)(MAKE_WORD(c, d))) << 16))

#define OPCODE_INVALID 0xD6
#define OPCODE_EXT 0xFF
#define OPCODE_NOP 0x90
#define OPCODE_CALL_REL 0xE8
#define OPCODE_MOV_REG32 0x8B
#define OPCODE_MOV_REG_IMM 0xB8

#define OPCODE_SUB_RM_IMM32 0x81
#define OPCODE_SUB_RM_IMM8 0x83

#define OPCODE_JMP_REL8 0xEB
#define OPCODE_JMP_REL 0xE9


#define INST_UNDEFINED 0xC4C4

// BIOS Operation
#define INST_BOP_PREFIX INST_UNDEFINED

#define INST_PUSH_EBP 0x55
#define INST_MOV_EBP_ESP  (MAKE_WORD(OPCODE_MOV_REG32, 0xEC))

#define INST_CALL_REL_SIZE 5
#define INST_CALL_MIN_SIZE 2
#define INST_CALL_MAX_SIZE 7
#define CODE_BUFFER_OVERRUN_SIZE 16

#define CODE_SAVE_FRAME (INST_PUSH_EBP | (INST_MOV_EBP_ESP << 8))
#define CODE_SAVE_FRAME_SIZE 3
#define CODE_SAVE_FRAME_MASK 0xFFFFFF


#define VALUE_INVALID 0xDEADBEEF
#define REGISTER_INVALID 0xFFFFFFFF

#define REGISTER_X86_EAX 0
#define REGISTER_X86_ECX 1
#define REGISTER_X86_EDX 2
#define REGISTER_X86_EBX 3
#define REGISTER_X86_ESP 4
#define REGISTER_X86_EBP 5
#define REGISTER_X86_ESI 6
#define REGISTER_X86_EDI 7


#define MODRM_RM_OFFSET  0
#define MODRM_REG_OFFSET 3
#define MODRM_MOD_OFFSET 6

#define MODRM_RM_MASK  (7 << MODRM_RM_OFFSET)
#define MODRM_REG_MASK (7 << MODRM_REG_OFFSET)
#define MODRM_MOD_MASK (3 << MODRM_MOD_OFFSET)


#define MODRM_MOD_00 (0 << MODRM_MOD_OFFSET)
#define MODRM_MOD_01 (1 << MODRM_MOD_OFFSET)
#define MODRM_MOD_10 (2 << MODRM_MOD_OFFSET)
#define MODRM_MOD_11 (3 << MODRM_MOD_OFFSET)

#define MODRM_MOD32_MEM         MODRM_MOD_00
#define MODRM_MOD32_MEM_DISP8   MODRM_MOD_01
#define MODRM_MOD32_MEM_DISP32  MODRM_MOD_10
#define MODRM_MOD32_REGISTER    MODRM_MOD_11

#define MODRM_OPCODE_EXT_OFFSET  MODRM_REG_OFFSET
#define MODRM_OPCODE_EXT_MASK    MODRM_REG_MASK

#define MODRM_MEM_USE_SIB  (4 << MODRM_RM_OFFSET)
#define MODRM_MEM_DISP32   (5 << MODRM_RM_OFFSET)


#define SIB_BASE_OFFSET  0
#define SIB_INDEX_OFFSET 3
#define SIB_SCALE_OFFSET 6

#define SIB_BASE_MASK  (7 << SIB_BASE_OFFSET)
#define SIB_INDEX_MASK (7 << SIB_INDEX_OFFSET)
#define SIB_SCALE_MASK (3 << SIB_SCALE_OFFSET)

#define SIB_BASE_NONE  (5 << SIB_BASE_OFFSET)
#define SIB_INDEX_NONE (4 << SIB_INDEX_OFFSET)

#define SIB_SCALE_1 (0 << SIB_SCALE_OFFSET)
#define SIB_SCALE_2 (1 << SIB_SCALE_OFFSET)
#define SIB_SCALE_4 (2 << SIB_SCALE_OFFSET)
#define SIB_SCALE_8 (3 << SIB_SCALE_OFFSET)


class DisassemblerX86
{
public:
    enum
    {
        MODRM_INFO_SIZE_MASK = 0xF,
        MODRM_INFO_SIB_NO_BASE_ALLOWED = 0x10,
    };

    DisassemblerX86();

    const gtUByte* CrackInstruction(const gtUByte* pBytes);

    static unsigned CrackInstructionLength(const gtUByte* pBytes);

    static bool CrackInstructionOperands(const gtUByte* pBytes, gtUInt32& value, gtUInt32& reg);

    static unsigned CrackInstructionRegister(const gtUByte* pBytes);

    static unsigned CrackInstructionOperandValue(const gtUByte* pBytes);

protected:
    struct OpcodeEntry
    {
        gtUInt32 m_opcodeInfo;
        const gtUByte* (DisassemblerX86::*m_pfnCrackBytes)(const OpcodeEntry&, const gtUByte*);
        gtUInt32 m_operandInfo;
    };

    const gtUByte* CrackBytes(const struct OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* Invalid(const OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* Crack0F(const OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* Crack66(const OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* Crack67(const OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* CrackF6(const OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* CrackF7(const OpcodeEntry& entry, const gtUByte* pBytes);
    const gtUByte* CrackBytesPrefix(const OpcodeEntry& entry, const gtUByte* pBytes);

    static bool CrackInstructionOperands(const gtUByte* pBytes, gtUInt32& value, gtUInt32& reg, const struct OpcodeEntry& entry);

    unsigned m_dw0;
    unsigned m_dw1;
    bool m_hasOperands = 0; // b8 | dw2
    unsigned m_opcode = 0; // dw3
    gtUInt32 m_value; // dw4
    gtUInt32 m_register; // dw5

    static const OpcodeEntry s_rceCopyTable0F[0x100];
    static const OpcodeEntry s_rceCopyTable[0x100];
};

#endif // _DISASSEMBLERX86_H_
