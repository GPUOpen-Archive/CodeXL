//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Disassembler.cpp
/// \brief Intel Disassembler class implementation.
///
//==================================================================================

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if _MSC_VER
    #define snprintf _snprintf
#endif

#ifndef DISASSEMBLER_STANDALONE
    #define _INDISASSEMBLERDLL
#endif

#include "Disassembler.h"

using namespace n_Disassembler;

unsigned int CExtraInstInfoDisassembler::m_Count = 0;   // number of derivations hooking this disassembler object
unsigned int CExtraInstInfoDisassembler::m_NumRegistered = 0;   // number of registered derivatives.  Vectors are deleted when this goes to zero
std::list <CExtraExtraInstInfo*> CExtraInstInfoDisassembler::m_Hooks;   // list of Vectors allocated

Inst_Info* Get3dnowIndex(CDisassembler* pThis);
Inst_Info* GetFpuIndex(CDisassembler* pThis);
Inst_Info* GetSSEIndex(CDisassembler* pThis);
Inst_Info* GetSSEHiToLoIndex(CDisassembler* pThis);
Inst_Info* GetGroupIndex(CDisassembler* pThis);
Inst_Info* GetLongModeIndex(CDisassembler* pThis);
Inst_Info* Get_2_b8_Index(CDisassembler* pThis);
Inst_Info* Get_2_bc_Index(CDisassembler* pThis);
Inst_Info* Get_2_bd_Index(CDisassembler* pThis);
Inst_Info* GetNopXchgPauseIndex(CDisassembler* pThis);
Inst_Info* GetWDIndex(CDisassembler* pThis);
Inst_Info* GetWDQIndex(CDisassembler* pThis);
Inst_Info* GetWDQIndex64(CDisassembler* pThis);
Inst_Info* GetNewGroupIndex(CDisassembler* pThis);
Inst_Info* GetGroupCIndex(CDisassembler* pThis);
Inst_Info* GetPrefetchIndex(CDisassembler* pThis);
Inst_Info* GetModRmIndex(CDisassembler* pThis);
Inst_Info* GetJcxIndex(CDisassembler* pThis);
Inst_Info* Get_2_38_Index(CDisassembler* pThis);
Inst_Info* Get_2_38_XX_Index(CDisassembler* pThis);
Inst_Info* Get_2_38_f01_Index(CDisassembler* pThis);
Inst_Info* Get_2_3a_Index(CDisassembler* pThis);
Inst_Info* Get_2_3a_XX_Index(CDisassembler* pThis);
Inst_Info* Get_RexOperandsize_Index(CDisassembler* pThis);
Inst_Info* Get_VexOperandsize_Index(CDisassembler* pThis);
Inst_Info* Get_VexW_Index(CDisassembler* pThis);
Inst_Info* Get_VexL_Index(CDisassembler* pThis);
Inst_Info* Get_VEX_Opcode(CDisassembler* pThis);
Inst_Info* GetGroupVexLIndex(CDisassembler* pThis);

#include "DisassemblerTables.h"

void CInstructionData::LogOpcodeOffset(int offset)
{
    if (m_numOpcodes < MAX_OPCODES)
    {
        for (int i = 0; i < m_numOpcodes; i++)
            if (offset < m_opcodeOffsets[i])
            {
                for (int j = m_numOpcodes++; j > i; j--)
                {
                    m_opcodeOffsets[j] = m_opcodeOffsets[j - 1];
                }

                m_opcodeOffsets[i] = offset;
                return;
            }

        m_opcodeOffsets[m_numOpcodes++] = offset;
    }
    else
    {
        printf("ERROR: LogOpcodeOffset: max opcodes (%d) exceeded!!!\n", MAX_OPCODES);
    }
}

CDisassembler::CDisassembler() : m_hexPostfix("h"), m_opcodeSeperator(" ")
{
    //  m_bUpperCase = true;
    m_maxInstructionBytes = MAX_INSTRUCTION_BYTES;
    m_bUpperCase = false;
    m_rex_prefix = 0;
    m_longmode = m_svmmode = false;
    m_extensions = 0xffffffffffffffff;
    m_dbit = true;
    m_bShowSize = false;
    m_bCalculateRipRelative = false;
    m_alternateDecodings = 0;
    RestoreMnemonicBuffer();
}

CDisassembler::~CDisassembler()
{
}

char* CDisassembler::Disassemble(const AMD_UINT8* inst_buf)
{
    m_bCalculateRipRelative = false;

    if (Decode(inst_buf))
    {
        Disassemble();
        return (m_mnem.data());
    }

    return (NULL);
}

char* CDisassembler::Disassemble(const AMD_UINT8* inst_buf, int bufferLength)
{
    m_maxInstructionBytes = bufferLength;
    char* rval = Disassemble(inst_buf);
    m_maxInstructionBytes = MAX_INSTRUCTION_BYTES;
    return rval;
}

char* CDisassembler::Disassemble(const AMD_UINT8* inst_buf, int bufferLength, AMD_UINT64 rip)
{
    m_maxInstructionBytes = bufferLength;
    char* rval = Disassemble(inst_buf, rip);
    m_maxInstructionBytes = MAX_INSTRUCTION_BYTES;
    return rval;
}

bool CDisassembler::Decode(const AMD_UINT8* inst_buf, int bufferLength)
{
    m_maxInstructionBytes = bufferLength;
    bool rval = Decode(inst_buf);
    m_maxInstructionBytes = MAX_INSTRUCTION_BYTES;
    return rval;
}

char* CDisassembler::Disassemble(const AMD_UINT8* inst_buf, int bufferLength, bool dbit)
{
    m_maxInstructionBytes = bufferLength;
    char* rval = Disassemble(inst_buf, dbit);
    m_maxInstructionBytes = MAX_INSTRUCTION_BYTES;
    return rval;
}

bool CDisassembler::Decode(const AMD_UINT8* inst_buf, int bufferLength, bool dbit)
{
    m_maxInstructionBytes = (bufferLength <= MAX_INSTRUCTION_BYTES) ? bufferLength : MAX_INSTRUCTION_BYTES;
    bool rval = Decode(inst_buf, dbit);
    m_maxInstructionBytes = MAX_INSTRUCTION_BYTES;
    return rval;
}

char* CDisassembler::Disassemble(const AMD_UINT8* inst_buf, AMD_UINT64 rip)
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

char* CDisassembler::Disassemble()
{
    m_mnem.clear();     // clear mnemonic string

    GetPrefixBytesString();

    m_mnem += (char*)(m_opcode_table_ptr->mnem);

    // The immediate value could potentially correspond to multiple operands. Consequently,
    // m_immediateData will be used to facilitate processing multiple operands.
    m_immediateData = m_immd;

    StringRef seperator = m_opcodeSeperator;

    for (int i = 0; i < MAX_OPERANDS; i++)
    {
        int index = m_operand_indices[i];
        m_pOperand = &m_operands[i];

        if (EXPLICIT_OPERAND(*m_pOperand) && (S_DisassembleOperandFnPtrs[m_opcode_table_ptr->operands[index]] != (PVOIDMEMBERFUNC)NULL))
        {
            m_mnem += seperator;
            seperator = ",";

            if (m_bShowSize) { m_pOperand->flags |= OPF_SHOWSIZE; }

            (this->*S_DisassembleOperandFnPtrs[m_opcode_table_ptr->operands[index]])();
        }
    }

    if (m_bUpperCase)
#if 1
        for (size_t i = 0; i < m_mnem.size(); i++)
        {
            m_mnem[i] = (char)toupper(m_mnem[i]);
        }

#else
        strupr(m_mnem);
#endif

    return (m_mnem.data());
}

bool CDisassembler::Decode(const AMD_UINT8* inst_buf)
{
    if (m_longmode && m_dbit)
    {
        return (false);
    }

    m_inst_buf = inst_buf;
    m_inst_flags = 0;
    m_len = 0;
    m_rex_prefix = 0;
    m_modrm = 0;
    m_sib = 0;
    m_vex[0] = m_vex[1] = m_vex[2] = 0;
    m_disp = (AMD_UINT64)0;
    m_immd = (AMD_UINT64)0;
    m_errorLevel = NO_EXCEPTION;

    m_numOperands = 0;
    m_numOpcodes = 0;
    memset(m_opcodeOffsets, -1, sizeof(m_opcodeOffsets));
    m_modrmOffset = -1;
    m_sibOffset = -1;
    m_displacementOffset = -1;
    m_immediateOffset = -1;
    m_bHasIndex = false;
    m_bHasBase = false;
    m_bModrmDecoded = false;

    m_displacementLength = 0;
    m_immediateLength = 0;

    m_prefix_bytes.ClearBytes();

    try
    {
        DecodeOpcodeBytes();

        if (GetInfoPtr() && (m_extensions & ((AMD_UINT64)1 << GetInstInfoPtr()->set_type)))
        {
            SetInstructionFlags();
            DecodeOperandBytes();
            return true;
        }

        m_errorLevel = FORMAT_EXCEPTION;        // if you got here, then the opcodes must be invalid
    }
    catch (CLengthException)
    {
        m_errorLevel = LENGTH_EXCEPTION;
    }
    catch (CFormatException)
    {
        m_errorLevel = FORMAT_EXCEPTION;
    }

    return false;
}

bool CDisassembler::GetInfoPtr()
{
    if (HasVex())
    {
        m_opcode_table_ptr = (GetVex(0) == PREFIX_XOP) ? &S_xop_tbl[GetVexTblIndex()] : &S_vex_tbl[GetVexTblIndex()];
    }
    else
        m_opcode_table_ptr =
            IsLongopcode()
            ? &S_twoByteOpcodes_tbl[GetOpcode(1)]
            : &S_oneByteOpcodes_tbl[GetOpcode(0)];

    while (m_opcode_table_ptr->GetInfoPtr != NULL)
    {
        Inst_Info *(*pIndexRoutine)(CDisassembler * pThis) = m_opcode_table_ptr->GetInfoPtr;

        if ((m_opcode_table_ptr = (pIndexRoutine)(this)) == NULL)
        {
            break;
        }
    }

    return ((m_opcode_table_ptr != NULL) && (m_opcode_table_ptr->mnem != NULL));
}

bool CDisassembler::IsVexPrefix(AMD_UINT8 prefix, AMD_UINT8 nextbyte)
{
    if ((m_extensions & (1 << SET_AVX)) && ((prefix == PREFIX_VEX2) || (prefix == PREFIX_VEX3)))
    {
        return (m_longmode || ((nextbyte & 0xC0) == 0xC0));
    }
    else if ((m_extensions & (1 << SET_XOP)) && (prefix == PREFIX_XOP))
    {
        return ((m_longmode || ((nextbyte & 0xC0) == 0xC0)) && (VEX_MMMMM(nextbyte) >= 8));
    }
    else
    {
        return false;
    }
}

void CDisassembler::DecodeOpcodeBytes()
{
    AMD_UINT32 escape = 0;

    while (1)
    {
        AMD_UINT8 instByte = GetByte();

        if (instByte == 0x0F)
        {
            LogOpcodeOffset(m_len - 1);

            if (escape)  // amd3d instructions (0x0f 0x0f)
            {
                m_inst_flags |= (INST_LONGOPCODE | INST_AMD3D);
                int offset = DecodeAmdOperandBytes();

                PeekByte(offset);
                LogOpcodeOffset(m_len + offset);

                break;      // exit the while loop
            }

            escape = 1;
        }
        else
        {
            if (!escape)
            {
                if (((instByte == PREFIX_VEX2) || (instByte == PREFIX_VEX3) || (instByte == PREFIX_XOP)) && !HasVex())
                {
                    // is VEX prefix!
                    AMD_UINT8 nextByte = PeekByte(0);

                    if (IsVexPrefix(instByte, nextByte))
                    {
                        m_vex[0] = instByte;
                        m_vex[1] = nextByte;
                        GetByte();

                        if (instByte != PREFIX_VEX2)  // 3-byte VEX
                        {
                            m_vex[2] = GetByte();

                            if (m_longmode && VEX_W(m_vex[2]))
                            {
                                m_inst_flags |= INST_DATA64;
                                m_inst_flags &= ~INST_DATA32;
                            }
                        }

                        GetByte();
                        LogOpcodeOffset(m_len - 1);
                        break;  // exit the while loop
                    }
                }

                if
                ((S_PrefixByteFnPtrs[instByte] != (PVOIDMEMBERFUNC)NULL)
                 && !HasVex()
                 && (m_longmode || (S_PrefixByteFnPtrs[instByte] != &CDisassembler::PrefixRex))
                )
                {
                    m_prefix_bytes.AddByte(instByte);
                }
                else
                {
                    LogOpcodeOffset(m_len - 1);
                    break;      // exit the while loop
                }
            }
            else
            {
                LogOpcodeOffset(m_len - 1);
                m_inst_flags |= INST_LONGOPCODE;

                switch (instByte)
                {
                    case 0x38: // MNI instruction
                    case 0x3a: // MNI instruction
                    {
                        GetByte();
                        LogOpcodeOffset(m_len - 1);

                        break;
                    }

                    case 0x24:
                    case 0x25:
                    {
                        if (HasRexPrefix())
                        {
                            throw CFormatException();
                        }

                        GetByte();
                        LogOpcodeOffset(m_len - 1);

                        break;
                    }

                    case 0x7A:
                    case 0x7B:
                    {
                        GetByte();
                        LogOpcodeOffset(m_len - 1);

                        break;
                    }
                }

                break;      // exit the while loop
            }
        }
    }

    ScanForRexPrefix();
}

// Determine the length of the operand bytes
int CDisassembler::DecodeAmdOperandBytes()
{
    int length = 0;
    AMD_UINT8 modrm = PeekByte(length++);
    int addressMode;

    if (!m_longmode)
    {
        if (m_dbit)
        {
            addressMode = ScanForAddressOverride() ? 16 : 32;
        }
        else
        {
            addressMode = ScanForAddressOverride() ? 32 : 16;
        }
    }
    else
    {
        addressMode = 32;
    }

    if (addressMode == 32)
    {
        if (MEM(modrm) && (RM(modrm) == 4))
        {
            AMD_UINT8 sib = PeekByte(length++);

            switch (MOD(modrm))
            {
                case 0:
                    if (BASE(sib) == 5)
                    {
                        length += 4;
                    }

                    break;

                case 1:
                    length += 1;
                    break;

                case 2:
                    length += 4;
                    break;
            }
        }
        else
        {
            switch (MOD(modrm))
            {
                case 0:
                    if (RM(modrm) == 5)
                    {
                        length += 4;
                    }

                    break;

                case 1:
                    length += 1;
                    break;

                case 2:
                    length += 4;
                    break;
            }
        }
    }
    else
    {
        if (((MOD(modrm) == 0) && (RM(modrm) == 6)) || (MOD(modrm) == 2))
        {
            length += 2;
        }
        else if (MOD(modrm) == 1)
        {
            length += 1;
        }
    }

    return length;
}

void CDisassembler::ScanForRexPrefix(void)
{
    // the REX prefix must be the last prefix or it is ignored
    if (m_longmode)
    {
        AMD_UINT8 prefix;

        if (m_prefix_bytes.GetLastByte(&prefix) && REX_PREFIX(prefix))
        {
            m_rex_prefix = prefix;
        }
    }
}

void CDisassembler::SetInstructionFlags(void)
{
    m_inst_flags |= m_opcode_table_ptr->instruction_flags;

    if (m_longmode)
    {
        m_inst_flags |= (INST_DATA32 | INST_ADDR64);
    }
    else if (m_dbit)
    {
        m_inst_flags |= (INST_DATA32 | INST_ADDR32);
    }
    else
    {
        m_inst_flags |= (INST_DATA16 | INST_ADDR16);
    }

    AMD_UINT8 prefix;

    for (int i = 0; m_prefix_bytes.GetByte(i, &prefix); i++)
    {
        (this->*S_PrefixByteFnPtrs[prefix])();
    }

    if (m_longmode && HasVex() && GetVexW())
    {
        m_inst_flags &= ~INST_DATA32;
        m_inst_flags |= INST_DATA64;
    }
}

void CDisassembler::SwapAVXOperands(void)
{
    if (GetVexW())
    {
        // swap last two register operands
        AMD_UINT32 lastOpIndex = m_numOperands - 1;
        e_OperandType lastType = m_operands[lastOpIndex].type;

        if ((lastType == OPERANDTYPE_IMMEDIATE) || (lastType == OPERANDTYPE_NONE))
        {
            lastOpIndex--;
        }

        COperandInfo tmp = m_operands[lastOpIndex];
        m_operands[lastOpIndex] = m_operands[lastOpIndex - 1];
        m_operands[lastOpIndex - 1] = tmp;
        m_operand_indices[lastOpIndex] = lastOpIndex - 1;
        m_operand_indices[lastOpIndex - 1] = lastOpIndex;
    }
}

// At this point m_len should index the byte following the opcode (except for 3dnow).
// At the end of this routine, m_len will contain the number of bytes
// in the instruction (i.e it will index the byte following the last byte).
void CDisassembler::DecodeOperandBytes()
{
    if (HasVex())
    {
        // #UD if REX, DATA, REP, or REPNE prefix is present
        // #UD if !VEX.L and no 128-bit form is valid
        // #UD if VEX.L and no 256-bit form is valid
        // #UD if VEX.W and VEX.W is reserved
        // #UD if VEX.VVVV is unused and not 1111b
        if (REX_PREFIX(m_rex_prefix) ||
            (m_prefix_bytes.FindByte(PREFIX_DATA) != -1) ||
            (m_prefix_bytes.FindByte(PREFIX_REPNE) != -1) ||
            (m_prefix_bytes.FindByte(PREFIX_REP) != -1) ||
            ((m_inst_flags & INST_NO_V128) && !GetVexL()) ||
            ((m_inst_flags & INST_NO_V256) && GetVexL()) ||
            ((m_inst_flags & INST_NO_VEXW) && GetVexW()) ||
            ((m_inst_flags & INST_NO_VVVV) && (GetVexvvvv() != 0xF)))
        {
            throw CFormatException();
        }

        if (m_inst_flags & INST_VEXWL_OPSIZE)
        {
            m_inst_flags &= ~(INST_DATA16 | INST_DATA32 | INST_DATA64);

            if (AlternateEnabled(ALTERNATE_LWP))
            {
                // original LWP/TBM0 VEX/XOP.{W/L}
                if (!GetVexW() && !GetVexL())
                {
                    m_inst_flags |= INST_DATA16;
                }
                else if (!GetVexW() && GetVexL())
                {
                    m_inst_flags |= INST_DATA32;
                }
                else if (GetVexW() && !GetVexL() && m_longmode)
                {
                    m_inst_flags |= INST_DATA64;
                }
                else
                {
                    throw CFormatException();
                }
            }
            else
            {
                // revised LWP/TBM0/TBM1/TBM2 VEX.XOP.{W/L}
                if (GetVexL())
                {
                    throw CFormatException();
                }
                else if (GetVexW() && m_longmode)
                {
                    m_inst_flags |= INST_DATA64;
                }
                else
                {
                    m_inst_flags |= INST_DATA32;
                }
            }
        }
    }

    int i;

    for (i = 0; i < MAX_OPERANDS; i++)
    {
        if (m_opcode_table_ptr->operands[i] == OPRND_na)
        {
            m_numOperands = i;

            for (; i < MAX_OPERANDS; i++)
            {
                m_operand_indices[i] = i;
                m_operands[i].Initialize(m_opcode_table_ptr->operand_flags[i]);
            }

            break;
        }

        if (i == (MAX_OPERANDS - 1))
        {
            m_numOperands = MAX_OPERANDS;
        }

        m_operand_indices[i] = i;
        m_pOperand = &m_operands[i];
        m_pOperand->Initialize(m_opcode_table_ptr->operand_flags[i]);

        if (S_DecodeOperandFnPtrs[m_opcode_table_ptr->operands[i]] != (PVOIDMEMBERFUNC)NULL)
        {
            (this->*S_DecodeOperandFnPtrs[m_opcode_table_ptr->operands[i]])();
        }
    }

    if ((m_inst_flags & INST_VEXW_SWAP) != 0)
    {
        SwapAVXOperands();
    }

    if (HasLockPrefix())
    {
        CheckLockPrefix();
    }

    // #UD if the same register is used more than once for dest,index,mask
    if ((m_inst_flags & INST_GATHER) != 0)
    {
        int dest = m_operands[0].GetRegister() - m_operands[0].GetRegisterBlock();
        int index = m_index - REG_VR0;
        int mask = m_operands[2].GetRegister() - m_operands[2].GetRegisterBlock();

        if ((dest == index) || (dest == mask) || (index == mask))
        {
            throw CFormatException();
        }
    }

    if (m_longmode && (m_inst_flags & INST_AMD64INVALID) != 0)
    {
        throw CFormatException();
    }

    // If this is an AMD instruction, we can now (after scanning remainder of instruction)
    //  increment m_len to account for the opcode byte at the end of the instruction bytes
    if ((m_inst_flags & INST_AMD3D) != 0)
    {
        m_len++;
    }
}

int CDisassembler::GetDataSize()
{
    if (m_longmode)
    {
        AMD_UINT8 prefix;

        if (REX_PREFIX(m_rex_prefix) && REX_OPERAND64(m_rex_prefix))
        {
            return (64);
        }

        // going backwards (last prefix takes precedence)
        for (int i = m_prefix_bytes.GetCount() - 1; i >= 0; i--)
            if (m_prefix_bytes.GetByte(i, &prefix) && (prefix == PREFIX_DATA))
            {
                return (16);
            }

        return (32);
    }
    else
    {
        AMD_UINT8 prefix;

        // going backwards (last prefix takes precedence)
        for (int i = m_prefix_bytes.GetCount() - 1; i >= 0; i--)
            if (m_prefix_bytes.GetByte(i, &prefix) && (prefix == PREFIX_DATA))
            {
                return (m_dbit ? 16 : 32);
            }

        return (m_dbit ? 32 : 16);
    }
}

void CDisassembler::HandleExtraPrefixOpcode()
{
    int prefixDataIndex = -1;

    // going backwards (last prefix takes precedence)
    for (int i = m_prefix_bytes.GetCount() - 1; i >= 0; i--)
    {
        AMD_UINT8 prefix;

        if (m_prefix_bytes.GetByte(i, &prefix))
        {
            switch (prefix)
            {
                case PREFIX_REPNE:
                    m_prefix_bytes.RemoveIndex(i);
                    LogOpcodeOffset(i);
                    return;

                case PREFIX_REP:
                    m_prefix_bytes.RemoveIndex(i);
                    LogOpcodeOffset(i);
                    return;

                case PREFIX_DATA:
                    if (prefixDataIndex < 0)
                    {
                        prefixDataIndex = i;
                    }
            }
        }
    }

    if (prefixDataIndex >= 0)
    {
        m_prefix_bytes.RemoveIndex(prefixDataIndex);
        LogOpcodeOffset(prefixDataIndex);
    }
}

void CDisassembler::HandleExtraRepOrRepneOpcode()
{
    // going backwards (last prefix takes precedence)
    for (int i = m_prefix_bytes.GetCount() - 1; i >= 0; i--)
    {
        AMD_UINT8 prefix;

        if (m_prefix_bytes.GetByte(i, &prefix))
        {
            if (prefix == PREFIX_REP)
            {
                m_prefix_bytes.RemoveIndex(i);
                LogOpcodeOffset(i);
                return;
            }

            if (prefix == PREFIX_REPNE)
            {
                m_prefix_bytes.RemoveIndex(i);
                LogOpcodeOffset(i);
                return;
            }
        }
    }
}

void CDisassembler::HandleExtraRepOpcode()
{
    // going forwards (first prefix takes precedence)
    AMD_UINT32 count = m_prefix_bytes.GetCount();

    for (AMD_UINT32 i = 0; i < count; i++)
    {
        AMD_UINT8 prefix;

        if (m_prefix_bytes.GetByte(i, &prefix))
        {
            if (prefix == PREFIX_REP)
            {
                m_prefix_bytes.RemoveIndex(i);
                LogOpcodeOffset(i);
                return;
            }
        }
    }
}

void CDisassembler::HandleExtraF3Opcode()
{
    // going backward (last prefix takes precedence)
    int count = m_prefix_bytes.GetCount();

    for (int i = (count - 1); i >= 0; i--)
    {
        AMD_UINT8 prefix;

        if (m_prefix_bytes.GetByte(i, &prefix))
        {
            if (prefix == PREFIX_REP)
            {
                m_prefix_bytes.RemoveIndex(i);
                LogOpcodeOffset(i);
                return;
            }
            else if (prefix == PREFIX_REPNE)
            {
                return;
            }
        }
    }
}

AMD_UINT8 CDisassembler::GetByte()
{
    if (m_len < m_maxInstructionBytes)
    {
        return m_inst_buf[m_len++];
    }
    else
    {
        throw CLengthException();
    }
}

AMD_UINT8 CDisassembler::PeekByte(int offset)    // used only to get offset to 3dx opcode
{
    if ((m_len + offset) < m_maxInstructionBytes)
    {
        return m_inst_buf[m_len + offset];
    }
    else
    {
        throw CLengthException();
    }
}

const StringRef CDisassembler::S_PrefixByteStrings[] =
{
    /* 0x00-7 */    "", "", "", "", "", "", "", "",
    /* 0x08-f */    "", "", "", "", "", "", "", "",
    /* 0x10-7 */    "", "", "", "", "", "", "", "",
    /* 0x18-f */    "", "", "", "", "", "", "", "",
    /* 0x20-7 */    "", "", "", "", "", "", "", "",
    /* 0x28-f */    "", "", "", "", "", "", "", "",
    /* 0x30-7 */    "", "", "", "", "", "", "", "",
    /* 0x38-f */    "", "", "", "", "", "", "", "",
#if 0
    /* 0x40-7 */    "rex_0 ", "rex_1 ", "rex_2 ", "rex_3 ", "rex_4 ", "rex_5 ", "rex_6 ", "rex_7 ",
    /* 0x48-f */    "rex_8 ", "rex_9 ", "rex_a ", "rex_b ", "rex_c ", "rex_d ", "rex_e ", "rex_f ",
#else
    /* 0x40-7 */    "", "", "", "", "", "", "", "",
    /* 0x48-f */    "", "", "", "", "", "", "", "",
#endif
    /* 0x50-7 */    "", "", "", "", "", "", "", "",
    /* 0x58-f */    "", "", "", "", "", "", "", "",
    /* 0x60-7 */    "", "", "", "", "", "", "", "",
    /* 0x68-f */    "", "", "", "", "", "", "", "",
    /* 0x70-7 */    "", "", "", "", "", "", "", "",
    /* 0x78-f */    "", "", "", "", "", "", "", "",
    /* 0x80-7 */    "", "", "", "", "", "", "", "",
    /* 0x88-f */    "", "", "", "", "", "", "", "",
    /* 0x90-7 */    "", "", "", "", "", "", "", "",
    /* 0x98-f */    "", "", "", "", "", "", "", "",
    /* 0xa0-7 */    "", "", "", "", "", "", "", "",
    /* 0xa8-f */    "", "", "", "", "", "", "", "",
    /* 0xb0-7 */    "", "", "", "", "", "", "", "",
    /* 0xb8-f */    "", "", "", "", "", "", "", "",
    /* 0xc0-7 */    "", "", "", "", "", "", "", "",
    /* 0xc8-f */    "", "", "", "", "", "", "", "",
    /* 0xd0-7 */    "", "", "", "", "", "", "", "",
    /* 0xd8-f */    "", "", "", "", "", "", "", "",
    /* 0xe0-7 */    "", "", "", "", "", "", "", "",
    /* 0xe8-f */    "", "", "", "", "", "", "", "",
    /* 0xf0   */    "lock ",
    /* 0xf1   */    "",
    /* 0xf2   */    "repne ",
    /* 0xf3   */    "rep ",
    /* 0xf4-7 */    "", "", "", "",
    /* 0xf8-f */    "", "", "", "", "", "", "", ""
};

void CDisassembler::GetPrefixBytesString()
{
    for (AMD_UINT32 i = 0; i < m_prefix_bytes.GetCount(); i++)
    {
        AMD_UINT8 prefix;

        if (m_prefix_bytes.GetByte(i, &prefix))
            if ((prefix != PREFIX_LOCK) || ((m_inst_flags & INST_LOCK_OVERUSED) == 0))
            {
                m_mnem += S_PrefixByteStrings[prefix];
            }
    }
}

Inst_Info* CDisassembler::S_3dnow_ptrs[] =
{
    /* 0x00-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x08-b   */ NULL, NULL, NULL, NULL,
    /* 0x0c     */ &S_group_2_0f_tbl[0],
    /* 0x0d     */ &S_group_2_0f_tbl[1],
    /* 0x0e-f   */ NULL, NULL,
    /* 0x10-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x18-b   */ NULL, NULL, NULL, NULL,
    /* 0x1c     */ &S_group_2_0f_tbl[2],
    /* 0x1d     */ &S_group_2_0f_tbl[3],
    /* 0x1e-f   */ NULL, NULL,
    /* 0x20-8   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x28-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x30-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x38-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x40-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x48-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x50-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x58-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x60-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x68-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x70-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x78-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x80-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x88-f   */ NULL, NULL,
    /* 0x8a     */ &S_group_2_0f_tbl[4],
    /* 0x8b-d   */ NULL, NULL, NULL,
    /* 0x8e     */ &S_group_2_0f_tbl[5],
    /* 0x8f     */ NULL,
    /* 0x90     */ &S_group_2_0f_tbl[6],
    /* 0x91-3   */ NULL, NULL, NULL,
    /* 0x94     */ &S_group_2_0f_tbl[7],
    /* 0x95     */ NULL,
    /* 0x96     */ &S_group_2_0f_tbl[8],
    /* 0x97     */ &S_group_2_0f_tbl[9],
    /* 0x98-9   */ NULL, NULL,
    /* 0x9a     */ &S_group_2_0f_tbl[10],
    /* 0x9b-d   */ NULL, NULL, NULL,
    /* 0x9e     */ &S_group_2_0f_tbl[11],
    /* 0x9f     */ NULL,
    /* 0xa0     */ &S_group_2_0f_tbl[12],
    /* 0xa1-3   */ NULL, NULL, NULL,
    /* 0xa4     */ &S_group_2_0f_tbl[13],
    /* 0xa5     */ NULL,
    /* 0xa6     */ &S_group_2_0f_tbl[14],
    /* 0xa7     */ &S_group_2_0f_tbl[15],
    /* 0xa8-9   */ NULL, NULL,
    /* 0xaa     */ &S_group_2_0f_tbl[16],
    /* 0xab-d   */ NULL, NULL, NULL,
    /* 0xae     */ &S_group_2_0f_tbl[17],
    /* 0xaf     */ NULL,
    /* 0xb0     */ &S_group_2_0f_tbl[18],
    /* 0xb1-3   */ NULL, NULL, NULL,
    /* 0xb4     */ &S_group_2_0f_tbl[19],
    /* 0xb5     */ NULL,
    /* 0xb6     */ &S_group_2_0f_tbl[20],
    /* 0xb7     */ &S_group_2_0f_tbl[21],
    /* 0xb8-a   */ NULL, NULL, NULL,
    /* 0xbb     */ &S_group_2_0f_tbl[22],
    /* 0xbc-e   */ NULL, NULL, NULL,
    /* 0xbf     */ &S_group_2_0f_tbl[23],
    /* 0xc0-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xc8-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xd0-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xd8-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xe0-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xe8-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xf0-7   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xf8-f   */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

Inst_Info* Get3dnowIndex(CDisassembler* pThis)
{
    return (pThis->S_3dnow_ptrs[pThis->GetOpcode(2)]);
}

Inst_Info* GetFpuIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();

    if (pThis->m_modrm < 0xc0)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[pThis->m_modrm_reg]);
    }
    else
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[pThis->m_modrm - 0xc0 + 8]);
    }
}

Inst_Info* GetSSEIndex(CDisassembler* pThis)
{
    int index;

    pThis->HandleExtraPrefixOpcode();

    switch (pThis->GetOpcode(0))
    {
        case PREFIX_DATA:
            index = 1;
            break;

        case PREFIX_REPNE:
            index = 2;
            break;

        case PREFIX_REP:
            index = 3;
            break;

        default:
            index = 0;
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index]);
}

Inst_Info* GetSSEHiToLoIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();
    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[((pThis->m_modrm_mod == 3) ? 1 : 0)]);
}

Inst_Info* GetGroupIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();
    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[pThis->m_modrm_reg]);
}

Inst_Info* GetLongModeIndex(CDisassembler* pThis)
{
    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[pThis->IsLongMode() ? 1 : 0]);
}

Inst_Info* Get_2_b8_Index(CDisassembler* pThis)
{
    pThis->HandleExtraF3Opcode();

    if (pThis->GetOpcode(0) == PREFIX_REP)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_2_bc_Index(CDisassembler* pThis)
{
    // if TBM1 is not enabled, TZCNT is 'REP BSF'
    if (pThis->m_extensions & (1 << SET_TBM1))
    {
        pThis->HandleExtraF3Opcode();
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[(pThis->GetOpcode(0) == PREFIX_REP) ? 1 : 0]);
}

Inst_Info* Get_2_bd_Index(CDisassembler* pThis)
{
    // if SSE4A is not enabled, LZCNT is 'REP BSR'
    if (pThis->m_extensions & (1 << SET_SSE4A))
    {
        pThis->HandleExtraF3Opcode();
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[(pThis->GetOpcode(0) == PREFIX_REP) ? 1 : 0]);
}

Inst_Info* GetNopXchgPauseIndex(CDisassembler* pThis)
{
    if (REX_MODRM_RM(pThis->GetRexPrefix()))
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else
    {
        pThis->HandleExtraRepOpcode();

        if (pThis->GetOpcode(0) == PREFIX_REP)
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[2]);
        }
        else
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
        }
    }
}

Inst_Info* GetWDIndex(CDisassembler* pThis)
{
    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[((pThis->GetDataSize() >= 32) ? 1 : 0)]);
}

Inst_Info* GetWDQIndex(CDisassembler* pThis)
{
    int index;

    switch (pThis->GetDataSize())
    {
        case 64:
            index = 2;
            break;

        case 32:
            index = 1;
            break;

        default:
            index = 0;
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index]);
}

Inst_Info* GetWDQIndex64(CDisassembler* pThis)
{
    int index;

    switch (pThis->GetDataSize())
    {
        case 64:
            index = 2;
            break;

        case 32:
            index = pThis->IsLongMode() ? 2 : 1;
            break;

        default:
            index = 0;
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index]);
}

Inst_Info* GetNewGroupIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();
    int index = pThis->m_modrm_reg;

    if (pThis->m_modrm_mod == 0x3)
    {
        index |= 0x8;
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index]);
}

Inst_Info* GetGroupCIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();
    int index = pThis->m_modrm_reg;

    if (pThis->m_modrm_mod == 0x3)
    {
        index |= 0x8;
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index]);
}

Inst_Info* GetPrefetchIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();
    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[((pThis->m_modrm_reg == 1) ? 1 : 0)]);
}

Inst_Info* GetModRmIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();

    if (pThis->m_modrm_mod != 3)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
    else
    {
        AMD_UINT8 rm = pThis->m_modrm_rm;

        if (rm <= 7)
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[rm + 1]);
        }
    }

    return (NULL);
}

Inst_Info* GetJcxIndex(CDisassembler* pThis)
{
    if (pThis->m_longmode)
    {
        if (pThis->m_prefix_bytes.FindByte(PREFIX_ADDR) == -1)    // not found
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[2]);
        }
        else
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
        }
    }
    else if (pThis->m_dbit)
    {
        if (pThis->m_prefix_bytes.FindByte(PREFIX_ADDR) == -1)    // not found
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
        }
        else
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
        }
    }
    else
    {
        if (pThis->m_prefix_bytes.FindByte(PREFIX_ADDR) == -1)    // not found
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
        }
        else
        {
            return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
        }
    }
}

Inst_Info* Get_2_38_Index(CDisassembler* pThis)
{
    AMD_UINT8 index = pThis->GetOpcode(2);
    return &((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index];
}

Inst_Info* Get_2_38_XX_Index(CDisassembler* pThis)
{
    // TODO: Need to determine how other overused prefix bytes (f2 and f3 (ie not 66)) affect decode
    pThis->HandleExtraPrefixOpcode();

    AMD_UINT8 firstOpcode = pThis->GetOpcode(0);

    if (firstOpcode == PREFIX_DATA)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else // if( firstOpcode == 0x0f )
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_2_38_f01_Index(CDisassembler* pThis)
{
    pThis->HandleExtraRepOrRepneOpcode();

    AMD_UINT8 firstOpcode = pThis->GetOpcode(0);

    if (firstOpcode == PREFIX_REP)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[2]);
    }
    else if (firstOpcode == PREFIX_REPNE)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else // if( firstOpcode == 0x0f )
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_2_3a_Index(CDisassembler* pThis)
{
    AMD_UINT8 index = pThis->GetOpcode(2);
    return &((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index];
}

Inst_Info* Get_2_3a_XX_Index(CDisassembler* pThis)
{
    // TODO: Need to determine how other overused prefix bytes (f2 and f3 (ie not 66)) affect decode
    pThis->HandleExtraPrefixOpcode();

    AMD_UINT8 firstOpcode = pThis->GetOpcode(0);

    if (firstOpcode == PREFIX_DATA)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else // if( firstOpcode == 0x0f )
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_RexOperandsize_Index(CDisassembler* pThis)
{
    if (REX_OPERAND32(pThis->GetRexPrefix()))
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
    else
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
}

Inst_Info* Get_VexOperandsize_Index(CDisassembler* pThis)
{
    if (pThis->GetVexW() && pThis->m_longmode)
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_VexW_Index(CDisassembler* pThis)
{
    if (pThis->GetVexW())
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_VexL_Index(CDisassembler* pThis)
{
    if (pThis->GetVexL())
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[1]);
    }
    else
    {
        return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[0]);
    }
}

Inst_Info* Get_VEX_Opcode(CDisassembler* pThis)
{
    AMD_UINT8 index = pThis->GetOpcode(0);
    return &((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index];
}

Inst_Info* GetGroupVexLIndex(CDisassembler* pThis)
{
    pThis->GetModrmByte();
    int index = pThis->m_modrm_reg;

    if (pThis->GetVexL())
    {
        index |= 0x8;
    }

    return (&((Inst_Info*)pThis->m_opcode_table_ptr->mnem)[index]);
}

int Inst_Info::SizeofNestedTable()
{
    if (GetInfoPtr == GetFpuIndex)
    {
        return 72;
    }
    else if (GetInfoPtr == GetSSEIndex)
    {
        return 4;
    }
    else if (GetInfoPtr == GetSSEHiToLoIndex)
    {
        return 2;
    }
    else if (GetInfoPtr == Get3dnowIndex)
    {
        return 24;
    }
    else if (GetInfoPtr == GetGroupIndex)
    {
        return 8;
    }
    else if (GetInfoPtr == GetGroupCIndex)
    {
        return 16;
    }
    else if (GetInfoPtr == GetNewGroupIndex)
    {
        return 17;
    }
    else if (GetInfoPtr == GetLongModeIndex)
    {
        return 2;
    }
    else if (GetInfoPtr == GetNopXchgPauseIndex)
    {
        return 3;
    }
    else if (GetInfoPtr == GetWDIndex)
    {
        return 2;
    }
    else if (GetInfoPtr == GetWDQIndex)
    {
        return 3;
    }
    else if (GetInfoPtr == GetWDQIndex64)
    {
        return 3;
    }
    else if (GetInfoPtr == GetPrefetchIndex)
    {
        return 2;
    }
    else if (GetInfoPtr == GetModRmIndex)
    {
        return 9;
    }
    else if (GetInfoPtr == GetJcxIndex)
    {
        return 3;
    }
    else if (GetInfoPtr == Get_2_38_Index)
    {
        return 256;
    }
    else if (GetInfoPtr == Get_2_38_XX_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_2_38_f01_Index)
    {
        return 3;
    }
    else if (GetInfoPtr == Get_2_3a_Index)
    {
        return 256;
    }
    else if (GetInfoPtr == Get_2_3a_XX_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_2_b8_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_2_bc_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_2_bd_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_RexOperandsize_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_VexOperandsize_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_VexW_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_VexL_Index)
    {
        return 2;
    }
    else if (GetInfoPtr == Get_VEX_Opcode)
    {
        return 256;
    }
    else
    {
        return 0;
    }
}

PVOIDMEMBERFUNC CDisassembler::S_PrefixByteFnPtrs[] =
{
    /* 0x00-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x08-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x10-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x18-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x20-5   */  NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x26     */  &CDisassembler::PrefixEsSegOveride,
    /* 0x27     */  NULL,
    /* 0x28-d   */  NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x2e     */  &CDisassembler::PrefixCsSegOveride,
    /* 0x2f     */  NULL,
    /* 0x30-5   */  NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x36     */  &CDisassembler::PrefixSsSegOveride,
    /* 0x37     */  NULL,
    /* 0x38-d   */  NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x3e     */  &CDisassembler::PrefixDsSegOveride,
    /* 0x3f     */  NULL,
    /* 0x40-3   */  &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex,
    /* 0x44-7   */  &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex,
    /* 0x48-b   */  &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex,
    /* 0x4c-f   */  &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex, &CDisassembler::PrefixRex,
    /* 0x50-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x58-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x60-3   */  NULL, NULL, NULL, NULL,
    /* 0x64     */  &CDisassembler::PrefixFsSegOveride,
    /* 0x65     */  &CDisassembler::PrefixGsSegOveride,
    /* 0x66     */  &CDisassembler::PrefixDataSize,
    /* 0x67     */  &CDisassembler::PrefixAddressSize,
    /* 0x68-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x70-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x78-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x80-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x88-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x90-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x98-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xa0-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xa8-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xb0-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xb8-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xc0-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xc8-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xd0-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xd8-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xe0-7   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xe8-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xf0     */  &CDisassembler::PrefixLock,
    /* 0xf1     */  NULL,
    /* 0xf2     */  &CDisassembler::PrefixRepne,
    /* 0xf3     */  &CDisassembler::PrefixRep,
    /* 0xf4-7   */  NULL, NULL, NULL, NULL,
    /* 0xf8-f   */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

void CDisassembler::PrefixRex()
{
    if (REX_OPERAND64(m_rex_prefix))
    {
        m_inst_flags |= INST_DATA64;
        m_inst_flags &= ~INST_DATA32;
    }
}

void CDisassembler::PrefixEsSegOveride()
{
    if (!m_longmode)
    {
        m_inst_flags |= INST_SEGOVRD;
        m_seg_reg = REG_ES;
    }
}

void CDisassembler::PrefixCsSegOveride()
{
    if (!m_longmode)
    {
        m_inst_flags |= INST_SEGOVRD;
        m_seg_reg = REG_CS;
    }
}

void CDisassembler::PrefixSsSegOveride()
{
    if (!m_longmode)
    {
        m_inst_flags |= INST_SEGOVRD;
        m_seg_reg = REG_SS;
    }
}

void CDisassembler::PrefixDsSegOveride()
{
    if (!m_longmode)
    {
        m_inst_flags |= INST_SEGOVRD;
        m_seg_reg = REG_DS;
    }
}

void CDisassembler::PrefixFsSegOveride()
{
    m_inst_flags |= INST_SEGOVRD;
    m_seg_reg = REG_FS;
}

void CDisassembler::PrefixGsSegOveride()
{
    m_inst_flags |= INST_SEGOVRD;
    m_seg_reg = REG_GS;
}

void CDisassembler::PrefixDataSize()
{
    if (m_longmode)
    {
        if (!REX_OPERAND64(m_rex_prefix))
        {
            m_inst_flags |= INST_DATAOVRD;
            m_inst_flags |= INST_DATA16;
            m_inst_flags &= ~INST_DATA32;
        }
    }
    else
    {
        m_inst_flags |= INST_DATAOVRD;

        if (m_dbit)
        {
            m_inst_flags |= INST_DATA16;
            m_inst_flags &= ~INST_DATA32;
        }
        else
        {
            m_inst_flags |= INST_DATA32;
            m_inst_flags &= ~INST_DATA16;
        }
    }
}

void CDisassembler::PrefixAddressSize()
{
    m_inst_flags |= INST_ADDROVRD;

    if (m_longmode)
    {
        m_inst_flags |= INST_ADDR32;
        m_inst_flags &= ~INST_ADDR64;
    }
    else
    {
        if (m_dbit)
        {
            m_inst_flags |= INST_ADDR16;
            m_inst_flags &= ~INST_ADDR32;
        }
        else
        {
            m_inst_flags |= INST_ADDR32;
            m_inst_flags &= ~INST_ADDR16;
        }
    }
}

void CDisassembler::PrefixLock()
{
    m_inst_flags |= INST_LOCK;
}

void CDisassembler::PrefixRepne()
{
    m_inst_flags &= ~INST_REP;      // last rep/repne prefix takes precedence
    m_inst_flags |= INST_REPNE;
}

void CDisassembler::PrefixRep()
{
    m_inst_flags &= ~INST_REPNE;    // last rep/repne prefix takes precedence
    m_inst_flags |= INST_REP;
}

PVOIDMEMBERFUNC CDisassembler::S_DisassembleOperandFnPtrs[] =
{
    /* OPRND_na */      NULL,
    /* OPRND_1 */       &CDisassembler::_1Str,
    /* OPRND_AL */      &CDisassembler::RegALStr,
    /* OPRND_AX */      &CDisassembler::RegAXStr,
    /* OPRND_eAX */     &CDisassembler::RegeAXStr,
    /* OPRND_Ap */      &CDisassembler::AddressStr,
    /* OPRND_CL */      &CDisassembler::RegCLStr,
    /* OPRND_Cd */      &CDisassembler::ControlRegStr,
    /* OPRND_Dd */      &CDisassembler::DebugRegStr,
    /* OPRND_DX */      &CDisassembler::RegDXStr,
    /* OPRND_eDX */     &CDisassembler::RegeDXStr,
    /* OPRND_Eb */      &CDisassembler::ModrmStr,
    /* OPRND_Ew */      &CDisassembler::ModrmStr,
    /* OPRND_Ed */      &CDisassembler::ModrmStr,
    /* OPRND_Ev */      &CDisassembler::ModrmStr,
    /* OPRND_Ep */      &CDisassembler::ModrmStr,
    /* OPRND_Mt */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Gb */      &CDisassembler::ByteRegStr,
    /* OPRND_Gw */      &CDisassembler::WordRegStr,
    /* OPRND_Gd */      &CDisassembler::DwordRegStr,
    /* OPRND_Gq */      &CDisassembler::QwordRegStr,
    /* OPRND_Gv */      &CDisassembler::WDQRegStr,
    /* OPRND_Ib */      &CDisassembler::ByteImmediateStr,
    /* OPRND_Iz */      &CDisassembler::WordOrDwordImmediateStr,
    /* OPRND_Iw */      &CDisassembler::WordImmediateStr,
    /* OPRND_Jb */      &CDisassembler::SignedByteJumpStr,
    /* OPRND_Jz */      &CDisassembler::SignedWordOrDwordJumpStr,
    /* OPRND_M */       &CDisassembler::MemoryModrmStr,
    /* OPRND_Mp */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Mq */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Ms */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Ob */      &CDisassembler::OffsetStr,
    /* OPRND_Ov */      &CDisassembler::OffsetStr,
    /* OPRND_Pq */      &CDisassembler::MMXRegStr,
    /* OPRND_Pd */      &CDisassembler::MMXRegStr,
    /* OPRND_Qq */      &CDisassembler::MMXModrmStr,
    /* OPRND_Rd */      &CDisassembler::RegisterStr,
    /* OPRND_Sw */      &CDisassembler::SegmentRegStr,
    /* OPRND_Xb */      &CDisassembler::DsEsiStr,
    /* OPRND_Xv */      &CDisassembler::DsEsiStr,
    /* OPRND_Yb */      &CDisassembler::EsEdiStr,
    /* OPRND_Yv */      &CDisassembler::EsEdiStr,
    /* OPRND_breg */    &CDisassembler::ByteRegStr,
    /* OPRND_vreg */    &CDisassembler::WDQRegStr,
    /* OPRND_ST */      &CDisassembler::FpuStr,
    /* OPRND_ST0 */     &CDisassembler::FpuStr,
    /* OPRND_ST1 */     &CDisassembler::FpuStr,
    /* OPRND_ST2 */     &CDisassembler::FpuStr,
    /* OPRND_ST3 */     &CDisassembler::FpuStr,
    /* OPRND_ST4 */     &CDisassembler::FpuStr,
    /* OPRND_ST5 */     &CDisassembler::FpuStr,
    /* OPRND_ST6 */     &CDisassembler::FpuStr,
    /* OPRND_ST7 */     &CDisassembler::FpuStr,
    /* OPRND_Vps */     &CDisassembler::SimdRegStr,
    /* OPRND_Vq */      &CDisassembler::SimdRegStr,
    /* OPRND_Vss */     &CDisassembler::SimdRegStr,
    /* OPRND_Wps */     &CDisassembler::SimdModrmStr,
    /* OPRND_Wq */      &CDisassembler::SimdModrmStr,
    /* OPRND_Wss */     &CDisassembler::SimdModrmStr,
    /* OPRND_Vpd */     &CDisassembler::SimdRegStr,
    /* OPRND_Wpd */     &CDisassembler::SimdModrmStr,
    /* OPRND_Vsd */     &CDisassembler::SimdRegStr,
    /* OPRND_Wsd */     &CDisassembler::SimdModrmStr,
    /* OPRND_Vx */      &CDisassembler::SimdRegStr,
    /* OPRND_Wx */      &CDisassembler::SimdModrmStr,
    /* OPRND_Vd */      &CDisassembler::SimdRegStr,
    /* OPRND_FPU_AX */  &CDisassembler::RegAXStr,
    /* OPRND_Mw */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Md */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Iv */      &CDisassembler::WDQImmediateStr,
    /* OPRND_eBXAl */   &CDisassembler::RegeBXAndALStr,
    /* OPRND_Ed_q */    &CDisassembler::ModrmStr,
    /* OPRND_Pd_q */    &CDisassembler::MMXRegStr,
    /* OPRND_Vd_q */    &CDisassembler::SimdRegStr,
    /* OPRND_Gd_q */    &CDisassembler::WDQRegStr,
    /* OPRND_Md_q */    &CDisassembler::MemoryModrmStr,
    /* OPRND_MwRv */    &CDisassembler::ModrmStr,
    /* OPRND_Mb */      &CDisassembler::MemoryModrmStr,

    /* OPRND_Hss */     &CDisassembler::SimdRegStr,
    /* OPRND_Hsd */     &CDisassembler::SimdRegStr,
    /* OPRND_Hps */     &CDisassembler::SimdRegStr,
    /* OPRND_Hpd */     &CDisassembler::SimdRegStr,

    /* OPRND_RdMb */    &CDisassembler::ModrmStr,
    /* OPRND_RdMw */    &CDisassembler::ModrmStr,
    /* OPRND_UxMq */    &CDisassembler::SimdModrmStr,
    /* OPRND_UxMd */    &CDisassembler::SimdModrmStr,
    /* OPRND_UxMw */    &CDisassembler::SimdModrmStr,

    /* OPRND_Gz */      &CDisassembler::WDQRegStr,
    /* OPRND_Nq */      &CDisassembler::MMXModrmStr,
    /* OPRND_Uq */      &CDisassembler::SimdModrmStr,
    /* OPRND_Ups */     &CDisassembler::SimdModrmStr,
    /* OPRND_Upd */     &CDisassembler::SimdModrmStr,
    /* OPRND_Ux */      &CDisassembler::SimdModrmStr,

    /* OPRND_Hx */      &CDisassembler::SimdRegStr,
    /* OPRND_Xx */      &CDisassembler::SimdRegStr,
    /* OPRND_Xss */     &CDisassembler::SimdRegStr,
    /* OPRND_Xsd */     &CDisassembler::SimdRegStr,
    /* OPRND_Wqdq */    &CDisassembler::SimdModrmStr,
    /* OPRND_Bv */      &CDisassembler::WDQRegStr,
    /* OPRND_Mdq */     &CDisassembler::MemoryModrmStr,
    /* OPRND_UxMb */    &CDisassembler::SimdModrmStr,
    /* OPRND_Mx */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Mv */      &CDisassembler::MemoryModrmStr,
    /* OPRND_Rv */      &CDisassembler::ModrmStr,
    /* OPRND_UxM8 */    &CDisassembler::SimdModrmStr,
    /* OPRND_UxM4 */    &CDisassembler::SimdModrmStr,
    /* OPRND_UxM2 */    &CDisassembler::SimdModrmStr,
};

StringRef CDisassembler::S_modrm16_str[]    = { "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx" };
StringRef CDisassembler::S_rex_byte_regs[]  = { "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil",
                                                "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b",
                                                "r16b", "r17b", "r18b", "r19b", "r20b", "r21b", "r22b", "r23b",
                                                "r24b", "r25b", "r26b", "r27b", "r28b", "r29b", "r30b", "r31b"
                                              };
StringRef CDisassembler::S_byte_regs[]      = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
StringRef CDisassembler::S_word_regs[]      = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
                                                "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w",
                                                "r16w", "r17w", "r18w", "r19w", "r20w", "r21w", "r22w", "r23w",
                                                "r24w", "r25w", "r26w", "r27w", "r28w", "r29w", "r30w", "r31w"
                                              };
StringRef CDisassembler::S_dword_regs[]     = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
                                                "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
                                                "r16d", "r17d", "r18d", "r19d", "r20d", "r21d", "r22d", "r23d",
                                                "r24d", "r25d", "r26d", "r27d", "r28d", "r29d", "r30d", "r31d"
                                              };
StringRef CDisassembler::S_qword_regs[]     = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                                                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                                                "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
                                                "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
                                              };
StringRef CDisassembler::S_control_regs[]   = { "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7",
                                                "cr8", "cr9", "cr10", "cr11", "cr12", "cr13", "cr14", "cr15"
                                              };
StringRef CDisassembler::S_debug_regs[]     = { "dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7",
                                                "dr8", "dr9", "dr10", "dr11", "dr12", "dr13", "dr14", "dr15"
                                              };
StringRef CDisassembler::S_segment_regs[]   = { "es", "cs", "ss", "ds", "fs", "gs", "res", "res" };
StringRef CDisassembler::S_fpu_regs[]       = { "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7", "st" };
StringRef CDisassembler::S_mmx_regs[]       = { "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7" };
StringRef CDisassembler::S_xmmx_regs[]      = { "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                                                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                                                "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23",
                                                "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31"
                                              };
StringRef CDisassembler::S_ymmx_regs[]      = { "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
                                                "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
                                                "ymm16", "ymm17", "ymm18", "ymm19", "ymm20", "ymm21", "ymm22", "ymm23",
                                                "ymm24", "ymm25", "ymm26", "ymm27", "ymm28", "ymm29", "ymm30", "ymm31"
                                              };
StringRef CDisassembler::S_vr_regs[]        = { "vr0", "vr1", "vr2", "vr3", "vr4", "vr5", "vr6", "vr7",
                                                "vr8", "vr9", "vr10", "vr11", "vr12", "vr13", "v14", "vr15",
                                                "vr16", "vr17", "vr18", "vr19", "vr20", "vr21", "vr22", "vr23",
                                                "vr24", "vr25", "vr26", "vr27", "vr28", "vr29", "vr30", "vr31"
                                              };


StringRef CDisassembler::S_size_qualifiers[]    = { "byte ", "word ", "dword ", "fword ", "qword ", "tword ", "dqword ", "qqword" };

int CDisassembler::MnemBytesLeft()
{
    return (int)(m_mnem.capacity() - m_mnem.size());
}

void CDisassembler::ModrmStr()
{
    (m_modrm_mod == 3) ? RegisterModrmStr() : MemoryModrmStr();
}

void CDisassembler::MemoryModrmStrWithoutSIB()
{
    if (m_pOperand->flags & OPF_SHOWSIZE)
    {
        m_mnem += GetSizeQualifier();
    }

    if (m_inst_flags & INST_SEGOVRD)
    {
        m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
        m_mnem += ':';
    }

    m_mnem += '[';

    if (m_modrm_mod == 0)
    {
        if (m_modrm_rm == 5)
        {
            if (m_longmode)
            {
                if (m_bCalculateRipRelative)
                {
                    m_mnem += "loc_";
                    m_mnem.appendUInt((m_rip + m_disp + GetLength()), GetAddressWidth(), m_hexPostfix);
                }
                else
                {
                    m_mnem += '$';
                    m_mnem.appendSInt((m_disp + GetLength()), 8, m_hexPostfix);
                }
            }
            else
            {
                m_mnem.appendUInt((AMD_UINT32)m_disp, 8, m_hexPostfix);
            }
        }
        else
        {
            m_mnem += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);
        }
    }
    else
    {
        m_mnem += ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);

        if (m_modrm_mod == 1)
        {
            m_mnem.appendSInt(m_disp, 2, m_hexPostfix);
        }
        else
        {
            m_mnem.appendSInt(m_disp, 8, m_hexPostfix);
        }
    }

    m_mnem += ']';
}

void CDisassembler::MemoryModrmStrWithSIB()
{
    char disp_buf[16] = "";
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
            base_str = ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);
        }

        if (m_modrm_mod == 1)
        {
            disp_str.appendSInt(m_disp, 2, m_hexPostfix);
        }
        else if (m_modrm_mod == 2)
        {
            disp_str.appendSInt(m_disp, 8, m_hexPostfix);
        }
        else if (m_modrm_mod == 0)
        {
            if ((IDX(m_sib) != 4) || (IsLongMode() && HasVex() && !GetVexX()) || REX_SIB_INDEX(m_rex_prefix))
            {
                disp_str.appendSInt(m_disp, 8, m_hexPostfix);
            }
            else
            {
                disp_str.appendUInt(m_disp, 8, m_hexPostfix);
            }
        }
    }
    else
    {
        base_str = ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_base] : S_dword_regs[m_base]);

        if (m_modrm_mod == 1)
        {
            disp_str.appendSInt(m_disp, 2, m_hexPostfix);
        }
        else if (m_modrm_mod == 2)
        {
            disp_str.appendSInt(m_disp, 8, m_hexPostfix);
        }
    }

    if (m_bHasIndex)
    {
        if ((m_index >= REG_VR0) && (m_index < REG_VR31))
        {
            idx_str = S_vr_regs[m_index - REG_VR0];
        }
        else
        {
            idx_str = ((m_inst_flags & INST_ADDR64) ? S_qword_regs[m_index] : S_dword_regs[m_index]);
        }

        if (SS(m_sib) == 1)
        {
            scale_str = "*2";
        }
        else if (SS(m_sib) == 2)
        {
            scale_str = "*4";
        }
        else if (SS(m_sib) == 3)
        {
            scale_str = "*8";
        }
    }

    // concat all these strings together

    if (m_pOperand->flags & OPF_SHOWSIZE)
    {
        m_mnem += GetSizeQualifier();
    }

    if (m_inst_flags & INST_SEGOVRD)
    {
        m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
        m_mnem += ':';
    }

    m_mnem += '[';

    m_mnem += base_str;

    if (!idx_str.empty())
    {
        if (base_str[0] != '\0')
        {
            m_mnem += '+';
        }

        m_mnem += idx_str;
    }

    if (!scale_str.empty())
    {
        m_mnem += scale_str;
    }

    if (!disp_str.empty())
    {
        m_mnem += disp_str;
    }

    m_mnem += ']';
}

void CDisassembler::MemoryModrmStr()
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
        if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += GetSizeQualifier();
        }

        if (m_inst_flags & INST_SEGOVRD)
        {
            m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
            m_mnem += ':';
        }

        m_mnem += '[';

        if (m_modrm_mod == 0)
        {
            if (m_modrm_rm == 6)
            {
                m_mnem.appendUInt(m_disp, 4, m_hexPostfix);
            }
            else
            {
                m_mnem += S_modrm16_str[m_modrm_rm];
            }
        }
        else
        {
            m_mnem += S_modrm16_str[m_modrm_rm];

            if (m_modrm_mod == 1)
            {
                m_mnem.appendSInt(m_disp, 2, m_hexPostfix);
            }
            else
            {
                m_mnem.appendSInt(m_disp, 4, m_hexPostfix);
            }
        }

        m_mnem += ']';
    }
}

void CDisassembler::RegisterModrmStr()
{
    if (m_pOperand->flags & OPF_FARPTR)
    {
        if (m_pOperand->size == OPERANDSIZE_48)
        {
            m_mnem += S_dword_regs[m_pOperand->reg];
        }
        else
        {
            m_mnem += S_word_regs[m_pOperand->reg];
        }
    }
    else
    {
        if (m_pOperand->size == OPERANDSIZE_8)
        {
            if (REX_PREFIX(m_rex_prefix) || HasVex())
            {
                m_mnem += S_rex_byte_regs[m_pOperand->reg];
            }
            else if (m_pOperand->IsHighByte())
            {
                m_mnem += S_byte_regs[m_pOperand->reg + 4];
            }
            else
            {
                m_mnem += S_byte_regs[m_pOperand->reg];
            }
        }
        else if (m_pOperand->size == OPERANDSIZE_16)
        {
            m_mnem += S_word_regs[m_pOperand->reg];
        }
        else if (m_pOperand->size == OPERANDSIZE_32)
        {
            m_mnem += S_dword_regs[m_pOperand->reg];
        }
        else
        {
            m_mnem += S_qword_regs[m_pOperand->reg];
        }
    }
}

void CDisassembler::ByteRegStr()
{
    if (REX_PREFIX(m_rex_prefix) || HasVex())
    {
        m_mnem += S_rex_byte_regs[m_pOperand->reg];
    }
    else if (m_pOperand->IsHighByte())
    {
        m_mnem += S_byte_regs[m_pOperand->reg + 4];
    }
    else
    {
        m_mnem += S_byte_regs[m_pOperand->reg];
    }
}

void CDisassembler::WordRegStr()
{
    m_mnem += S_word_regs[m_pOperand->reg];
}

void CDisassembler::DwordRegStr()
{
    m_mnem += S_dword_regs[m_pOperand->reg];
}

void CDisassembler::QwordRegStr()
{
    m_mnem += S_qword_regs[m_pOperand->reg];
}

void CDisassembler::WDQRegStr()
{
    if (m_pOperand->size == OPERANDSIZE_64)
    {
        m_mnem += S_qword_regs[m_pOperand->reg];
    }
    else if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem += S_dword_regs[m_pOperand->reg];
    }
    else
    {
        m_mnem += S_word_regs[m_pOperand->reg];
    }
}

void CDisassembler::ByteImmediateStr()
{
    if (m_pOperand->flags & OPF_SPECIAL64)
    {
        if (m_longmode)
        {
            m_mnem += S_size_qualifiers[SIZE_QWORD];
            m_mnem.appendUInt((m_immediateData & 0xff), 2, m_hexPostfix);
        }
        else
        {
            if (m_pOperand->flags & OPF_SHOWSIZE)
            {
                m_mnem += S_size_qualifiers[SIZE_BYTE];
            }

            m_mnem.appendUInt(m_immediateData, 2, m_hexPostfix);
        }
    }
    else
    {
        if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += S_size_qualifiers[SIZE_BYTE];
        }

        m_mnem.appendUInt((m_immediateData & 0xff), 2, m_hexPostfix);
    }

    m_immediateData >>= 8;
}

void CDisassembler::WordImmediateStr()
{
    if (m_pOperand->flags & OPF_SHOWSIZE)
    {
        m_mnem += S_size_qualifiers[SIZE_WORD];
    }

    m_mnem.appendUInt((m_immediateData & 0xffff), 4, m_hexPostfix);

    m_immediateData >>= 16;
}

void CDisassembler::WordOrDwordImmediateStr()
{
    if ((m_inst_flags & (INST_DATA32 | INST_DATA64)) == 0)
    {
        if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += S_size_qualifiers[SIZE_WORD];
        }

        m_mnem.appendUInt((m_immediateData & 0xffff), 4, m_hexPostfix);

        m_immediateData >>= 16;
    }
    else
    {
        if (m_longmode && ((m_pOperand->flags & OPF_SPECIAL64) != 0))
        {
            m_mnem += S_size_qualifiers[SIZE_QWORD];
        }
        else if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += S_size_qualifiers[SIZE_DWORD];
        }

        m_mnem.appendUInt((m_immediateData & 0xffffffff), 8, m_hexPostfix);

        m_immediateData >>= 32;
    }
}

void CDisassembler::WDQImmediateStr()
{
    if (m_inst_flags & INST_DATA64)
    {
        if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += S_size_qualifiers[SIZE_QWORD];
        }

        m_mnem.appendUInt(m_immediateData, 16, m_hexPostfix);

        m_immediateData = 0;
    }
    else if (m_inst_flags & INST_DATA32)
    {
        if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += S_size_qualifiers[SIZE_DWORD];
        }

        m_mnem.appendUInt((m_immediateData & 0xffffffff), 8, m_hexPostfix);

        m_immediateData >>= 32;
    }
    else
    {
        if (m_pOperand->flags & OPF_SHOWSIZE)
        {
            m_mnem += S_size_qualifiers[SIZE_WORD];
        }

        m_mnem.appendUInt((m_immediateData & 0xffff), 4, m_hexPostfix);

        m_immediateData >>= 16;
    }
}

void CDisassembler::SignedByteJumpStr()
{
    if (m_bCalculateRipRelative)
    {
        m_mnem += "loc_";
        m_mnem.appendUInt((m_rip + m_disp + GetLength()), GetAddressWidth(), m_hexPostfix);
    }
    else
    {
        m_mnem += '$';
        m_mnem.appendSInt((m_disp + GetLength()), 2, m_hexPostfix);
    }
}

void CDisassembler::SignedWordOrDwordJumpStr()
{
    if (m_bCalculateRipRelative)
    {
        m_mnem += "loc_";

        if (m_pOperand->size == OPERANDSIZE_16)
        {
            m_mnem.appendUInt((AMD_UINT16)(m_rip + m_disp + GetLength()), GetAddressWidth(), m_hexPostfix);
        }
        else
        {
            m_mnem.appendUInt((m_rip + m_disp + GetLength()), GetAddressWidth(), m_hexPostfix);
        }
    }
    else
    {
        m_mnem += '$';

        if (m_pOperand->size == OPERANDSIZE_16)
        {
            m_mnem.appendSInt((m_disp + GetLength()), 4, m_hexPostfix);
        }
        else
        {
            m_mnem.appendSInt((m_disp + GetLength()), 8, m_hexPostfix);
        }
    }
}

void CDisassembler::_1Str()
{
    m_mnem += '1';
}

void CDisassembler::RegCLStr()
{
    m_mnem += "cl";
}

void CDisassembler::RegALStr()
{
    m_mnem += "al";
}

void CDisassembler::RegAXStr()
{
    m_mnem += "ax";
}

void CDisassembler::RegeAXStr()
{
    if (m_pOperand->size == OPERANDSIZE_16)
    {
        m_mnem += "ax";
    }
    else if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem += "eax";
    }
    else
    {
        m_mnem += "rax";
    }
}

void CDisassembler::RegDXStr()
{
    m_mnem += "dx";
}

void CDisassembler::RegeDXStr()
{
    if (m_pOperand->size == OPERANDSIZE_16)
    {
        m_mnem += "dx";
    }
    else if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem += "edx";
    }
    else
    {
        m_mnem += "rdx";
    }
}

void CDisassembler::AddressStr()
{
    // can't use appendUInt/appendSInt multiple times in sprintf calls (overwrites static buffer)
    if (m_pOperand->size == OPERANDSIZE_32)
    {
        m_mnem.appendUInt((AMD_UINT16)(m_immd >> 16), 4, "");
        m_mnem += ':';
        m_mnem.appendUInt((AMD_UINT16)m_immd, 4, "");
    }
    else
    {
        m_mnem.appendUInt((AMD_UINT16)(m_immd >> 32), 4, "");
        m_mnem += ':';
        m_mnem.appendUInt((AMD_UINT32)m_immd, 8, "");
    }

    m_mnem += m_hexPostfix;
}

void CDisassembler::OffsetStr()
{
    if (m_pOperand->flags & OPF_SHOWSIZE)
    {
        m_mnem += GetSizeQualifier();
    }

    if (m_inst_flags & INST_SEGOVRD)
    {
        m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
        m_mnem += ":[";

        if (m_inst_flags & INST_ADDR64)
        {
            m_mnem.appendUInt(m_immd, 16, m_hexPostfix);
        }
        else if (m_inst_flags & INST_ADDR32)
        {
            m_mnem.appendUInt(m_immd, 8, m_hexPostfix);
        }
        else
        {
            m_mnem.appendUInt(m_immd, 4, m_hexPostfix);
        }
    }
    else
    {
        m_mnem += '[';

        if (m_inst_flags & INST_ADDR64)
        {
            m_mnem.appendUInt(m_immd, 16, m_hexPostfix);
        }
        else if (m_inst_flags & INST_ADDR32)
        {
            m_mnem.appendUInt(m_immd, 8, m_hexPostfix);
        }
        else
        {
            m_mnem.appendUInt(m_immd, 4, m_hexPostfix);
        }
    }

    m_mnem += ']';
}

void CDisassembler::MMXRegStr()
{
    m_mnem += S_mmx_regs[m_pOperand->reg];
}

void CDisassembler::MMXModrmStr()
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

void CDisassembler::RegisterStr()
{
    m_mnem += (m_longmode ? S_qword_regs[m_pOperand->reg] : S_dword_regs[m_pOperand->reg]);
}

void CDisassembler::ControlRegStr()
{
    m_mnem += S_control_regs[m_pOperand->reg];
}

void CDisassembler::DebugRegStr()
{
    m_mnem += S_debug_regs[m_pOperand->reg];
}

void CDisassembler::SegmentRegStr()
{
    m_mnem += S_segment_regs[m_pOperand->reg];
}

void CDisassembler::DsEsiStr()
{
    if (m_inst_flags & INST_ADDR64)
    {
        m_mnem += "[rsi]";
    }
    else
    {
        if (m_inst_flags & INST_SEGOVRD)
        {
            m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
            m_mnem += ":[";

            if (m_inst_flags & INST_ADDR32)
            {
                m_mnem += "esi]";
            }
            else
            {
                m_mnem += "si]";
            }
        }
        else if (m_inst_flags & INST_ADDR32)
        {
            m_mnem += "ds:[esi]";
        }
        else
        {
            m_mnem += "ds:[si]";
        }
    }
}

void CDisassembler::EsEdiStr()
{
    if (m_inst_flags & INST_ADDR64)
    {
        m_mnem += "[rdi]";
    }
    else if (m_inst_flags & INST_ADDR32)
    {
        m_mnem += "es:[edi]";
    }
    else
    {
        m_mnem += "es:[di]";
    }
}

void CDisassembler::FpuStr()
{
    m_mnem += S_fpu_regs[m_pOperand->reg];
}

void CDisassembler::SimdModrmStr()
{
    if (m_modrm_mod == 3)
    {
        m_mnem += GetXymmStr(m_pOperand);
    }
    else
    {
        MemoryModrmStr();
    }
}

void CDisassembler::SimdRegStr()
{
    m_mnem += GetXymmStr(m_pOperand);
}

void CDisassembler::RegeBXAndALStr()
{
    if (m_inst_flags & (INST_SEGOVRD | INST_ADDROVRD))
    {
        if (m_inst_flags & INST_SEGOVRD)
        {
            m_mnem += S_segment_regs[NORMALIZE_SEGREG(m_seg_reg)];
            m_mnem += ':';
        }

        if (m_inst_flags & INST_ADDR64)
        {
            m_mnem += "[rbx+al]";
        }
        else if (m_inst_flags & INST_ADDR32)
        {
            m_mnem += "[ebx+al]";
        }
        else
        {
            m_mnem += "[bx+al]";
        }
    }
}

int CDisassembler::GetVexTblIndex()
{
    return ((GetVexmmmmm() << 2) | GetVexpp());
}

const StringRef& CDisassembler::GetXymmStr(COperandInfo* op)
{
    const StringRef* mmregs = (op->regBlock == REG_YMM0) ? S_ymmx_regs : S_xmmx_regs;
    return (mmregs[op->reg]);
}

n_Disassembler::e_Registers CDisassembler::GetXymmBlock()
{
    return ((HasVex() && GetVexL() && !(m_pOperand->flags & OPF_VEXXMM) && !(m_inst_flags & INST_VEXL_UNDEF)) ? REG_YMM0 : REG_XMM0);
}

n_Disassembler::e_OperandSize CDisassembler::GetXymmOperandSize()
{
    return ((HasVex() && GetVexL() && !(m_pOperand->flags & OPF_VEXXMM) && !(m_inst_flags & INST_VEXL_UNDEF)) ? OPERANDSIZE_256 : OPERANDSIZE_128);
}

PVOIDMEMBERFUNC CDisassembler::S_DecodeOperandFnPtrs[] =
{
    /* OPRND_na */      NULL,
    /* OPRND_1  */      NULL,
    /* OPRND_AL */      &CDisassembler::GetRegAL,
    /* OPRND_AX */      &CDisassembler::GetRegAX,
    /* OPRND_eAX */     &CDisassembler::GetRegeAX,
    /* OPRND_Ap */      &CDisassembler::GetDwordOrFwordDirect,
    /* OPRND_CL */      &CDisassembler::GetRegCL,
    /* OPRND_Cd */      &CDisassembler::GetControlRegister,
    /* OPRND_Dd */      &CDisassembler::GetDebugRegister,
    /* OPRND_DX */      &CDisassembler::GetRegDX,
    /* OPRND_eDX */     &CDisassembler::GetRegeDX,
    /* OPRND_Eb */      &CDisassembler::GetByteModrm,
    /* OPRND_Ew */      &CDisassembler::GetWordModrm,
    /* OPRND_Ed */      &CDisassembler::GetDwordModrm,
    /* OPRND_Ev */      &CDisassembler::GetWDQModrm,
    /* OPRND_Ep */      &CDisassembler::GetDwordOrFwordMemory,
    /* OPRND_Mt */      &CDisassembler::GetTwordMemory,
    /* OPRND_Gb */      &CDisassembler::GetByteRegFromReg,
    /* OPRND_Gw */      &CDisassembler::GetWordRegFromReg,
    /* OPRND_Gd */      &CDisassembler::GetDwordRegFromReg,
    /* OPRND_Gq */      &CDisassembler::GetQwordRegFromReg,
    /* OPRND_Gv */      &CDisassembler::GetWDQRegFromReg,
    /* OPRND_Ib */      &CDisassembler::GetByteImmediate,
    /* OPRND_Iz */      &CDisassembler::GetWordOrDwordImmediate,
    /* OPRND_Iw */      &CDisassembler::GetWordImmediate,
    /* OPRND_Jb */      &CDisassembler::GetByteJump,
    /* OPRND_Jz */      &CDisassembler::GetWordOrDwordJump,
    /* OPRND_M  */      &CDisassembler::GetMemoryModrm,
    /* OPRND_Mp */      &CDisassembler::GetDwordOrFwordMemory,
    /* OPRND_Mq */      &CDisassembler::GetQwordMemory,
    /* OPRND_Ms */      &CDisassembler::GetFwordMemory,
    /* OPRND_Ob */      &CDisassembler::GetOffsetByte,
    /* OPRND_Ov */      &CDisassembler::GetOffsetWDQ,
    /* OPRND_Pq */      &CDisassembler::GetMMXQwordReg,
    /* OPRND_Pd */      &CDisassembler::GetMMXDwordReg,
    /* OPRND_Qq */      &CDisassembler::GetMMXQwordModrm,
    /* OPRND_Rd */      &CDisassembler::GetDwordOrQwordReg,
    /* OPRND_Sw */      &CDisassembler::GetWordSegmentRegister,
    /* OPRND_Xb */      &CDisassembler::GetByteMemoryEsi,
    /* OPRND_Xv */      &CDisassembler::GetWDQMemoryEsi,
    /* OPRND_Yb */      &CDisassembler::GetByteMemoryEdi,
    /* OPRND_Yv */      &CDisassembler::GetWDQMemoryEdi,
    /* OPRND_breg */    &CDisassembler::GetByteRegFromOpcode,
    /* OPRND_vreg */    &CDisassembler::GetWDQRegFromOpcode,
    /* OPRND_ST */      &CDisassembler::GetSTReg,
    /* OPRND_ST0 */     &CDisassembler::GetST0Reg,
    /* OPRND_ST1 */     &CDisassembler::GetST1Reg,
    /* OPRND_ST2 */     &CDisassembler::GetST2Reg,
    /* OPRND_ST3 */     &CDisassembler::GetST3Reg,
    /* OPRND_ST4 */     &CDisassembler::GetST4Reg,
    /* OPRND_ST5 */     &CDisassembler::GetST5Reg,
    /* OPRND_ST6 */     &CDisassembler::GetST6Reg,
    /* OPRND_ST7 */     &CDisassembler::GetST7Reg,
    /* OPRND_Vps */     &CDisassembler::GetSimdOwordReg,
    /* OPRND_Vq */      &CDisassembler::GetSimdQwordReg,
    /* OPRND_Vss */     &CDisassembler::GetSimdDwordReg,
    /* OPRND_Wps */     &CDisassembler::GetSimdOwordModrm,
    /* OPRND_Wq */      &CDisassembler::GetSimdQwordModrm,
    /* OPRND_Wss */     &CDisassembler::GetSimdDwordModrm,
    /* OPRND_Vpd */     &CDisassembler::GetSimdOwordReg,
    /* OPRND_Wpd */     &CDisassembler::GetSimdOwordModrm,
    /* OPRND_Vsd */     &CDisassembler::GetSimdQwordReg,
    /* OPRND_Wsd */     &CDisassembler::GetSimdQwordModrm,
    /* OPRND_Vx */      &CDisassembler::GetSimdOwordReg,
    /* OPRND_Wx */      &CDisassembler::GetSimdOwordModrm,
    /* OPRND_Vd */      &CDisassembler::GetSimdDwordReg,
    /* OPRND_FPU_AX */  &CDisassembler::GetFpuAx,
    /* OPRND_Mw */      &CDisassembler::GetWordMemory,
    /* OPRND_Md */      &CDisassembler::GetDwordMemory,
    /* OPRND_Iv */      &CDisassembler::GetWDQImmediate,
    /* OPRND_eBXAl */   &CDisassembler::GeteBXAndAL,
    /* OPRND_Ed_q */    &CDisassembler::GetDQModrm,
    /* OPRND_Pd_q */    &CDisassembler::GetMMXDQwordReg,
    /* OPRND_Vd_q */    &CDisassembler::GetSimdDQwordReg,
    /* OPRND_Gd_q */    &CDisassembler::GetDQRegFromReg,
    /* OPRND_Md_q */    &CDisassembler::GetDwordOrQwordMemory,
    /* OPRND_MwRv */    &CDisassembler::GetWordMemoryOrWDQRegModrm,
    /* OPRND_Mb */      &CDisassembler::GetByteMemory,

    /* OPRND_Hss */     &CDisassembler::GetVexOwordOpcode,
    /* OPRND_Hsd */     &CDisassembler::GetVexOwordOpcode,
    /* OPRND_Hps */     &CDisassembler::GetVexOwordOpcode,
    /* OPRND_Hpd */     &CDisassembler::GetVexOwordOpcode,

    /* OPRND_RdMb */    &CDisassembler::GetDwordRegOrByteMemoryModrm,
    /* OPRND_RdMw */    &CDisassembler::GetDwordRegOrWordMemoryModrm,
    /* OPRND_UxMq */    &CDisassembler::GetSimdOwordRegOrQwordMemoryModrm,
    /* OPRND_UxMd */    &CDisassembler::GetSimdOwordRegOrDwordMemoryModrm,
    /* OPRND_UxMw */    &CDisassembler::GetSimdOwordRegOrWordMemoryModrm,

    /* OPRND_Gz */      &CDisassembler::GetWDRegFromReg,
    /* OPRND_Nq */      &CDisassembler::GetMMXQwordModrmRegister,
    /* OPRND_Uq */      &CDisassembler::GetSimdQwordModrmRegister,
    /* OPRND_Ups */     &CDisassembler::GetSimdOwordModrmRegister,
    /* OPRND_Upd */     &CDisassembler::GetSimdOwordModrmRegister,
    /* OPRND_Ux */      &CDisassembler::GetSimdOwordModrmRegister,

    /* OPRND_Hx */      &CDisassembler::GetVexOwordOpcode,
    /* OPRND_Xx */      &CDisassembler::GetSimdOwordIs4,
    /* OPRND_Xss */     &CDisassembler::GetSimdOwordIs4,
    /* OPRND_Xsd */     &CDisassembler::GetSimdOwordIs4,
    /* OPRND_Wqdq */    &CDisassembler::GetSimdQwordOrOwordModrm,
    /* OPRND_Bv */      &CDisassembler::GetVexWDQ,
    /* OPRND_Mdq */     &CDisassembler::GetOwordMemory,
    /* OPRND_UxMb */    &CDisassembler::GetSimdOwordRegOrByteMemoryModrm,
    /* OPRND_Mx */      &CDisassembler::GetSimdOwordModrmMem,
    /* OPRND_Mv */      &CDisassembler::GetWDQModrmMem,
    /* OPRND_Rv */      &CDisassembler::GetWDQModrmReg,
    /* OPRND_UxM8 */    &CDisassembler::GetSimdOwordRegOrEigthMemoryModrm,
    /* OPRND_UxM4 */    &CDisassembler::GetSimdOwordRegOrQuarterMemoryModrm,
    /* OPRND_UxM2 */    &CDisassembler::GetSimdOwordRegOrHalfMemoryModrm,
};

void CDisassembler::DecodeModrm()
{
    if (!m_bModrmDecoded)
    {
        m_bModrmDecoded = true;

        if (m_inst_flags & (INST_ADDR32 | INST_ADDR64))
        {
            if (m_modrm_mod != 3)
            {
                if (m_modrm_rm != 4)
                {
                    // no SIB
                    if (HasVex())
                    {
                        if ((m_modrm_mod != 0) || (m_modrm_rm != 5))
                        {
                            int regOffset = (IsLongMode() && !GetVexB()) ? 8 : 0;
                            SetBase((AMD_UINT8)(m_modrm_rm + regOffset));
                        }
                    }
                    else
                    {
                        if ((m_modrm_mod != 0) || (m_modrm_rm != 5))
                        {
                            int regOffset = REX_MODRM_RM(m_rex_prefix) ? 8 : 0;
                            SetBase((AMD_UINT8)(m_modrm_rm + regOffset));
                        }
                    }

                    if ((m_modrm_mod == 0) && (m_modrm_rm == 5))
                    {
                        GetDisplacementDword();
                    }
                    else if (m_modrm_mod == 2)
                    {
                        GetDisplacementDword();
                    }
                    else if (m_modrm_mod == 1)
                    {
                        GetDisplacementByte();
                    }
                }
                else
                {
                    // SIB
                    m_sib = GetByte();
                    m_sibOffset = m_len;

                    if (HasVex())
                    {
                        int idxBase = (m_pOperand->flags & OPF_VSIB) ? REG_VR0 : REG_EAX;

                        if (IsLongMode() && !GetVexX())
                        {
                            SetIndex((AMD_UINT8)(idxBase + IDX(m_sib) + 8));
                        }
                        else if ((IDX(m_sib) != 4) || (idxBase == REG_VR0))
                        {
                            SetIndex((AMD_UINT8)(idxBase + IDX(m_sib)));
                        }
                    }
                    else
                    {
                        if (REX_SIB_INDEX(m_rex_prefix))
                        {
                            SetIndex((AMD_UINT8)(IDX(m_sib) + 8));
                        }
                        else if (IDX(m_sib) != 4)
                        {
                            SetIndex(IDX(m_sib));
                        }
                    }

                    if ((m_modrm_mod != 0) || (BASE(m_sib) != 5))
                    {
                        int regOffset;

                        if (HasVex())
                        {
                            regOffset = (IsLongMode() && !GetVexB()) ? 8 : 0;
                        }
                        else
                        {
                            regOffset = REX_SIB_BASE(m_rex_prefix) ? 8 : 0;
                        }

                        SetBase((AMD_UINT8)(BASE(m_sib) + regOffset));
                    }

                    if (m_modrm_mod == 1)
                    {
                        GetDisplacementByte();
                    }
                    else if ((m_modrm_mod == 2) || (BASE(m_sib) == 5))
                    {
                        GetDisplacementDword();
                    }
                }
            }
        }
        else
        {
            if (m_modrm_mod != 3)
            {
                switch (m_modrm_rm)
                {
                    case 0:
                        SetBase(REG_EBX);
                        SetIndex(REG_ESI);
                        break;

                    case 1:
                        SetBase(REG_EBX);
                        SetIndex(REG_EDI);
                        break;

                    case 2:
                        SetBase(REG_EBP);
                        SetIndex(REG_ESI);
                        break;

                    case 3:
                        SetBase(REG_EBP);
                        SetIndex(REG_EDI);
                        break;

                    case 4:
                        SetIndex(REG_ESI);
                        break;

                    case 5:
                        SetIndex(REG_EDI);
                        break;

                    case 6:
                        if (m_modrm_mod != 0)
                        {
                            SetBase(REG_EBP);
                        }

                        break;

                    case 7:
                        SetBase(REG_EBX);
                        break;
                }

                switch (m_modrm_mod)
                {
                    case 0:
                    {
                        if (m_modrm_rm == 6)
                        {
                            GetDisplacementWord();
                        }

                        break;
                    }

                    case 1:
                    {
                        GetDisplacementByte();
                        break;
                    }

                    case 2:
                    {
                        GetDisplacementWord();
                        break;
                    }
                }
            }
        }
    }
}

void CDisassembler::OperandModrm()
{
    GetModrmByte();

    if (m_modrm_mod == 3)
    {
        OperandRegisterModrm();
    }
    else
    {
        OperandMemoryModrm();
    }
}

void CDisassembler::OperandMemoryModrm()
{
    GetModrmByte();
    DecodeModrm();

    if (m_modrm_mod == 3)
    {
        throw CFormatException();
    }

    if ((m_pOperand->flags & OPF_VSIB) && (m_modrm_rm != 4))
    {
        throw CFormatException();
    }

    if ((m_pOperand->flags & OPF_VSIB) && (m_inst_flags & INST_ADDR16))
    {
        throw CFormatException();
    }

    if (m_pOperand->flags & OPF_REGISTER)
    {
        throw CFormatException();
    }

    m_pOperand->type = OPERANDTYPE_MEMORY;      // may be overwritten in longmode (see below)

    if (m_inst_flags & (INST_ADDR32 | INST_ADDR64))
    {
        if (m_modrm_rm != 4)
        {
            // no SIB
            if ((m_modrm_mod == 0) && (m_modrm_rm == 5))
                if (m_longmode)
                {
                    m_pOperand->type = OPERANDTYPE_RIPRELATIVE;
                }
        }
    }
}

void CDisassembler::OperandRegisterModrm()
{
    GetModrmByte();
    DecodeModrm();

    if (m_modrm_mod != 3)
    {
        throw CFormatException();
    }

    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_MODRM_RM;

    if (HasVex())
    {
        int regOffset = (IsLongMode() && !GetVexB()) ? 8 : 0;
        m_pOperand->reg = (AMD_UINT8)(m_modrm_rm + regOffset);
    }
    else if (REX_PREFIX(m_rex_prefix) || (m_pOperand->size != OPERANDSIZE_8))
    {
        int regOffset = REX_MODRM_RM(m_rex_prefix) ? 8 : 0;
        m_pOperand->reg = (AMD_UINT8)(m_modrm_rm + regOffset);
    }
    else
    {
        m_pOperand->reg = (AMD_UINT8)(m_modrm_rm & 0x3);

        if ((m_modrm_rm & 0x4) != 0)
        {
            m_pOperand->flags |= OPF_BYTEHIGH;
        }
    }
}

void CDisassembler::GetModrmByte()
{
    if (m_modrmOffset == -1)
    {
        m_modrmOffset = m_len;
        m_modrm = GetByte();

        m_modrm_mod = MOD(m_modrm);
        m_modrm_reg = REG(m_modrm);
        m_modrm_rm = RM(m_modrm);
    }
}

void CDisassembler::GetImmediateByte()
{
    if (m_immediateOffset == -1)
    {
        m_immediateOffset = m_len;
        m_immediateLength = 1;
        m_immd = GetByte();
    }
    else
    {
        // this must be the second immediate
        switch (m_len - m_immediateOffset)
        {
            case 0:
                // Is4 sets m_immediateOffset, m_immediateLength, m_immd
                break;

            case 1:
            {
                m_immd |= (GetByte() << 8);
                m_immediateLength++;
                break;
            }

            case 2:
            {
                m_immd |= (GetByte() << 16);
                m_immediateLength++;
                break;
            }

            default:
                throw CFormatException();
        }
    }
}

void CDisassembler::GetImmediateWord()
{
    m_immediateOffset = m_len;
    m_immediateLength = 2;
    m_immd = GetByte();
    m_immd |= GetByte() << 8;
}

void CDisassembler::GetImmediateDword()
{
    m_immediateOffset = m_len;
    m_immediateLength = 4;
    m_immd = GetByte();
    m_immd |= GetByte() << 8;
    m_immd |= GetByte() << 16;
    m_immd |= ((AMD_UINT64)GetByte()) << 24;
}

void CDisassembler::GetImmediateFword()
{
    m_immediateOffset = m_len;
    m_immediateLength = 6;
    m_immd = GetByte();
    m_immd |= GetByte() << 8;
    m_immd |= GetByte() << 16;
    m_immd |= ((AMD_UINT64)GetByte()) << 24;
    m_immd |= (((AMD_UINT64)GetByte()) << 32);
    m_immd |= (((AMD_UINT64)GetByte()) << 40);
}

void CDisassembler::GetImmediateQword()
{
    m_immediateOffset = m_len;
    m_immediateLength = 8;
    m_immd = GetByte();
    m_immd |= GetByte() << 8;
    m_immd |= GetByte() << 16;
    m_immd |= (((AMD_UINT64)GetByte()) << 24);
    m_immd |= (((AMD_UINT64)GetByte()) << 32);
    m_immd |= (((AMD_UINT64)GetByte()) << 40);
    m_immd |= (((AMD_UINT64)GetByte()) << 48);
    m_immd |= (((AMD_UINT64)GetByte()) << 56);
}

void CDisassembler::GetDisplacementByte()
{
    m_displacementOffset = m_len;
    m_displacementLength = 1;
    m_disp = GetByte();
    m_disp = (AMD_INT8)m_disp;      // sign extend
}

void CDisassembler::GetDisplacementWord()
{
    m_displacementOffset = m_len;
    m_displacementLength = 2;
    m_disp = GetByte();
    m_disp |= GetByte() << 8;
    m_disp = (AMD_INT16)m_disp;     // sign extend
}

void CDisassembler::GetDisplacementDword()
{
    m_displacementOffset = m_len;
    m_displacementLength = 4;
    m_disp = GetByte();
    m_disp |= GetByte() << 8;
    m_disp |= GetByte() << 16;
    m_disp |= ((AMD_UINT64)GetByte()) << 24;
    m_disp = (AMD_INT32)m_disp;     // sign extend
}

void CDisassembler::GetDisplacementQword()
{
    m_displacementOffset = m_len;
    m_displacementLength = 8;
    m_disp = GetByte();
    m_disp |= GetByte() << 8;
    m_disp |= GetByte() << 16;
    m_disp |= GetByte() << 24;
    m_disp |= (((AMD_UINT64)GetByte()) << 32);
    m_disp |= (((AMD_UINT64)GetByte()) << 40);
    m_disp |= (((AMD_UINT64)GetByte()) << 48);
    m_disp |= (((AMD_UINT64)GetByte()) << 56);
}

void CDisassembler::GetByteModrm()
{
    m_pOperand->size = OPERANDSIZE_8;
    OperandModrm();
}

void CDisassembler::GetWordModrm()
{
    m_pOperand->size = OPERANDSIZE_16;
    OperandModrm();
}

void CDisassembler::GetDwordModrm()
{
    m_pOperand->size = OPERANDSIZE_32;
    OperandModrm();
}

void CDisassembler::GetWordMemoryOrWDQRegModrm()
{
    GetModrmByte();
    m_pOperand->size = (m_modrm_mod == 3) ? GetWDQOperandSize() : OPERANDSIZE_16;
    DecodeModrm();
    OperandModrm();
}

void CDisassembler::GetWDQModrm()
{
    if (m_pOperand->flags & OPF_SPECIAL64)
    {
        if (m_longmode)
        {
            m_pOperand->size = (m_inst_flags & INST_DATA16) ? OPERANDSIZE_16 : OPERANDSIZE_64;
        }
        else
        {
            m_pOperand->size = (m_inst_flags & INST_DATA32) ? OPERANDSIZE_32 : OPERANDSIZE_16;
        }
    }
    else
    {
        m_pOperand->size = GetWDQOperandSize();
    }

    OperandModrm();
}

void CDisassembler::GetWDQModrmMem()
{
    if (m_pOperand->flags & OPF_SPECIAL64)
    {
        if (m_longmode)
        {
            m_pOperand->size = (m_inst_flags & INST_DATA16) ? OPERANDSIZE_16 : OPERANDSIZE_64;
        }
        else
        {
            m_pOperand->size = (m_inst_flags & INST_DATA32) ? OPERANDSIZE_32 : OPERANDSIZE_16;
        }
    }
    else
    {
        m_pOperand->size = GetWDQOperandSize();
    }

    OperandMemoryModrm();
}

void CDisassembler::GetWDQModrmReg()
{
    m_pOperand->size = GetWDQOperandSize();
    OperandRegisterModrm();
}

void CDisassembler::GetDQModrm()
{
    if (m_longmode)
    {
        m_pOperand->size = (m_inst_flags & INST_DATA64) ? OPERANDSIZE_64 : OPERANDSIZE_32;
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_32;
    }

    OperandModrm();
}

void CDisassembler::GetOwordModrm()
{
    m_pOperand->size = OPERANDSIZE_128;
    OperandModrm();
}

void CDisassembler::GetRegFromReg()
{
    GetModrmByte();

    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_MODRM_REG;

    int regOffset = 0;

    if (HasVex())
    {
        regOffset = (IsLongMode() && HasVex() && !GetVexR()) ? 8 : 0;
    }
    else if (REX_PREFIX(m_rex_prefix))
    {
        regOffset = REX_MODRM_REG(m_rex_prefix) ? 8 : 0;
    }

    m_pOperand->reg = (AMD_UINT8)(m_modrm_reg + regOffset);
}

void CDisassembler::GetByteRegFromReg()
{
    GetModrmByte();

    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_MODRM_REG;

    if (REX_PREFIX(m_rex_prefix))
    {
        int regOffset = REX_MODRM_REG(m_rex_prefix) ? 8 : 0;
        m_pOperand->reg = (AMD_UINT8)(m_modrm_reg + regOffset);
    }
    else
    {
        m_pOperand->reg = (AMD_UINT8)(m_modrm_reg & 0x3);

        if ((m_modrm_reg & 0x4) != 0)
        {
            m_pOperand->flags |= OPF_BYTEHIGH;
        }
    }
}

void CDisassembler::GetWordRegFromReg()
{
    m_pOperand->size = OPERANDSIZE_16;
    GetRegFromReg();
}

void CDisassembler::GetDwordRegFromReg()
{
    m_pOperand->size = OPERANDSIZE_32;
    GetRegFromReg();
}

void CDisassembler::GetQwordRegFromReg()
{
    m_pOperand->size = OPERANDSIZE_64;
    GetRegFromReg();
}

void CDisassembler::GetWDQRegFromReg()
{
    m_pOperand->size = GetWDQOperandSize();
    GetRegFromReg();
}

void CDisassembler::GetWDRegFromReg()
{
    m_pOperand->size = GetWDOperandSize();
    GetRegFromReg();
}

void CDisassembler::GetDQRegFromReg()
{
    if (m_longmode)
    {
        if (m_pOperand->flags & OPF_SPECIAL64)
        {
            m_pOperand->size = OPERANDSIZE_64;
        }
        else
        {
            m_pOperand->size = (m_inst_flags & INST_DATA32) ? OPERANDSIZE_32 : OPERANDSIZE_64;
        }
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_32;
    }

    GetRegFromReg();
}

void CDisassembler::GetByteImmediate()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_IMMEDIATE;
    GetImmediateByte();
}

void CDisassembler::GetWordImmediate()
{
    m_pOperand->size = OPERANDSIZE_16;
    m_pOperand->type = OPERANDTYPE_IMMEDIATE;
    GetImmediateWord();
}

void CDisassembler::GetWordOrDwordImmediate()
{
    m_pOperand->type = OPERANDTYPE_IMMEDIATE;

    if (m_inst_flags & (INST_DATA32 | INST_DATA64))
    {
        m_pOperand->size = OPERANDSIZE_32;
        GetImmediateDword();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_16;
        GetImmediateWord();
    }
}

void CDisassembler::GetWDQImmediate()
{
    m_pOperand->type = OPERANDTYPE_IMMEDIATE;

    if (m_inst_flags & INST_DATA64)
    {
        m_pOperand->size = OPERANDSIZE_64;
        GetImmediateQword();
    }
    else if (m_inst_flags & INST_DATA32)
    {
        m_pOperand->size = OPERANDSIZE_32;
        GetImmediateDword();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_16;
        GetImmediateWord();
    }
}

void CDisassembler::GetByteJump()
{
    m_pOperand->type = OPERANDTYPE_PCOFFSET;
    m_pOperand->size = OPERANDSIZE_8;
    GetDisplacementByte();
}

void CDisassembler::GetWordOrDwordJump()
{
    m_pOperand->type = OPERANDTYPE_PCOFFSET;

    if (m_inst_flags & (INST_DATA32 | INST_DATA64))
    {
        m_pOperand->size = OPERANDSIZE_32;
        GetDisplacementDword();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_16;
        GetDisplacementWord();
    }
}

void CDisassembler::GetByteRegFromOpcode()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;

    AMD_UINT8 opcode = GetOpcode(0);

    if (REX_PREFIX(m_rex_prefix))
    {
        int regOffset = REX_OPCODE_REG(m_rex_prefix) ? 8 : 0;
        m_pOperand->reg = (AMD_UINT8)(RM(opcode) + regOffset);
    }
    else
    {
        m_pOperand->reg = (AMD_UINT8)(RM(opcode) & 0x3);

        if ((RM(opcode) & 0x4) != 0)
        {
            m_pOperand->flags |= OPF_BYTEHIGH;
        }
    }
}

void CDisassembler::GetWDQRegFromOpcode()
{
    if ((m_pOperand->flags & OPF_SPECIAL64) && m_longmode)
    {
        m_pOperand->size = (m_inst_flags & INST_DATA16) ? OPERANDSIZE_16 : OPERANDSIZE_64;
    }
    else
    {
        m_pOperand->size = GetWDQOperandSize();
    }

    AMD_UINT8 opcode = GetOpcode(IsLongopcode() ? 1 : 0);
    m_pOperand->type = OPERANDTYPE_REGISTER;
    int regOffset = REX_OPCODE_REG(m_rex_prefix) ? 8 : 0;
    m_pOperand->reg = (AMD_UINT8)(RM(opcode) + regOffset);
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetRegAL()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->reg = REG_EAX;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetRegAX()
{
    m_pOperand->size = OPERANDSIZE_16;
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->reg = REG_EAX;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetRegeAX()
{
    m_pOperand->size = GetWDQOperandSize();
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->reg = REG_EAX;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetRegCL()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->reg = REG_ECX;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetRegDX()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_16;
    m_pOperand->reg = REG_EDX;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetRegeDX()
{
    m_pOperand->size = GetWDQOperandSize();
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->reg = REG_EDX;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetFpuAx()
{
    GetModrmByte();
    GetRegAX();
}

// No size associated with this memory (either sizeless or irregularly sized)
void CDisassembler::GetMemoryModrm()
{
    OperandMemoryModrm();
}

void CDisassembler::GetDwordOrFwordMemory()
{
    m_pOperand->flags |= OPF_FARPTR;
    m_pOperand->size = (m_inst_flags & (INST_DATA32 | INST_DATA64)) ? OPERANDSIZE_48 : OPERANDSIZE_32;
    OperandMemoryModrm();
}

void CDisassembler::GetFwordMemory()
{
    m_pOperand->flags |= OPF_FARPTR;
    m_pOperand->size = OPERANDSIZE_48;
    OperandMemoryModrm();
}

void CDisassembler::GetDwordOrFwordDirect()
{
    m_pOperand->size = (m_inst_flags & (INST_DATA32 | INST_DATA64)) ? OPERANDSIZE_48 : OPERANDSIZE_32;
    m_pOperand->type = OPERANDTYPE_IMMEDIATE;
    (m_pOperand->size == OPERANDSIZE_48) ? GetImmediateFword() : GetImmediateDword();
}

void CDisassembler::GetOffsetByte()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_MEMORY;

    if (m_inst_flags & INST_ADDR64)
    {
        GetImmediateQword();
    }
    else if (m_inst_flags & INST_ADDR32)
    {
        GetImmediateDword();
    }
    else
    {
        GetImmediateWord();
    }
}

void CDisassembler::GetOffsetWDQ()
{
    m_pOperand->size = GetWDQOperandSize();
    m_pOperand->type = OPERANDTYPE_MEMORY;

    if (m_inst_flags & INST_ADDR64)
    {
        GetImmediateQword();
    }
    else if (m_inst_flags & INST_ADDR32)
    {
        GetImmediateDword();
    }
    else
    {
        GetImmediateWord();
    }
}

void CDisassembler::GetMMXReg()
{
    GetRegFromReg();
    UndoRegisterExtensions();
    m_pOperand->regBlock = REG_MM0;
}

void CDisassembler::GetMMXDwordReg()
{
    m_pOperand->size = OPERANDSIZE_32;
    GetMMXReg();
}

void CDisassembler::GetMMXQwordReg()
{
    m_pOperand->size = OPERANDSIZE_64;
    GetMMXReg();
}

void CDisassembler::GetMMXDQwordReg()
{
    if (m_longmode)
    {
        m_pOperand->size = (m_inst_flags & INST_DATA32) ? OPERANDSIZE_32 : OPERANDSIZE_64;
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_32;
    }

    GetMMXReg();
}

void CDisassembler::GetMMXQwordModrm()
{
    m_pOperand->size = OPERANDSIZE_64;

    OperandModrm();
    UndoRegisterExtensions();
    m_pOperand->regBlock = REG_MM0;
}

void CDisassembler::GetMMXQwordModrmRegister()
{
    m_pOperand->flags |= OPF_REGISTER;
    GetMMXQwordModrm();
}

// this is for the Control Registers and Debug Registers mov instructions (GP register operand)
void CDisassembler::GetDwordOrQwordReg()
{
    m_pOperand->size = m_longmode ? OPERANDSIZE_64 : OPERANDSIZE_32;
    GetModrmByte();

    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_MODRM_RM;

    int regOffset = REX_MODRM_RM(m_rex_prefix) ? 8 : 0;
    m_pOperand->reg = (AMD_UINT8)(m_modrm_rm + regOffset);
}

void CDisassembler::GetDebugRegister()
{
    m_pOperand->size = m_longmode ? OPERANDSIZE_64 : OPERANDSIZE_32;
    GetRegFromReg();
    m_pOperand->regBlock = REG_DR0;
}

void CDisassembler::GetControlRegister()
{
    m_pOperand->size = m_longmode ? OPERANDSIZE_64 : OPERANDSIZE_32;
    GetRegFromReg();

    if (HasLockPrefix() && !REX_MODRM_REG(m_rex_prefix))
    {
        m_pOperand->reg += 8;
    }

    if (m_pOperand->reg >= NUM_CONTROL_REGISTERS)
    {
        throw CFormatException();
    }

    m_pOperand->regBlock = REG_CR0;
}

void CDisassembler::GetWordSegmentRegister()
{
    m_pOperand->size = OPERANDSIZE_16;
    GetRegFromReg();
    UndoRegisterExtensions();
    m_pOperand->regBlock = REG_ES;
}

void CDisassembler::GetByteMemoryEsi()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_MEMORY;
    m_pOperand->reg = REG_ESI;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
    m_pOperand->flags |= OPF_STRING;
}

void CDisassembler::GetWDQMemoryEsi()
{
    m_pOperand->size = GetWDQOperandSize();
    m_pOperand->type = OPERANDTYPE_MEMORY;
    m_pOperand->reg = REG_ESI;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
    m_pOperand->flags |= OPF_STRING;
}

void CDisassembler::GetByteMemoryEdi()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_MEMORY;
    m_pOperand->reg = REG_EDI;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
    m_pOperand->flags |= OPF_STRING;
}

void CDisassembler::GetWDQMemoryEdi()
{
    m_pOperand->size = GetWDQOperandSize();
    m_pOperand->type = OPERANDTYPE_MEMORY;
    m_pOperand->reg = REG_EDI;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
    m_pOperand->flags |= OPF_STRING;
}

void CDisassembler::GetST0Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 0;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST1Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 1;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST2Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 2;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST3Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 3;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST4Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 4;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST5Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 5;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST6Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 6;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetST7Reg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 7;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetSTReg()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = OPERANDSIZE_80;
    m_pOperand->reg = 8;
    m_pOperand->regBlock = REG_ST0;
    m_pOperand->regOrigin = REGORIGIN_OPCODE;
}

void CDisassembler::GetSimdReg()
{
    GetRegFromReg();
    m_pOperand->regBlock = GetXymmBlock();
}

void CDisassembler::GetSimdDwordReg()
{
    m_pOperand->size = OPERANDSIZE_32;
    GetSimdReg();
}

void CDisassembler::GetSimdQwordReg()
{
    m_pOperand->size = OPERANDSIZE_64;
    GetSimdReg();
}

void CDisassembler::GetSimdDQwordReg()
{
    if (m_longmode)
    {
        m_pOperand->size = (m_inst_flags & INST_DATA32) ? OPERANDSIZE_32 : OPERANDSIZE_64;
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_32;
    }

    GetSimdReg();
}

void CDisassembler::GetSimdOwordIs4()
{
    m_immediateOffset = m_len;
    m_immediateLength = 1;
    m_immd = GetByte();

    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->size = GetXymmOperandSize();
    m_pOperand->regBlock = GetXymmBlock();
    m_pOperand->regOrigin = REGORIGIN_IS4;
    m_pOperand->reg = IsLongMode() ? (AMD_UINT8)(m_immd >> 4) : (AMD_UINT8)((m_immd >> 4) & 0x07);
}

void CDisassembler::GetSimdOwordReg()
{
    m_pOperand->size = GetXymmOperandSize();
    GetSimdReg();
}

void CDisassembler::GetSimdModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->regBlock = GetXymmBlock();
    }
}

void CDisassembler::GetSimdDwordModrm()
{
    m_pOperand->size = OPERANDSIZE_32;
    GetSimdModrm();
}

void CDisassembler::GetSimdQwordModrm()
{
    m_pOperand->size = OPERANDSIZE_64;
    GetSimdModrm();
}

void CDisassembler::GetSimdQwordOrOwordModrm()
{
    m_pOperand->size = (HasVex() && GetVexL()) ? OPERANDSIZE_128 : OPERANDSIZE_64;
    GetSimdModrm();
    m_pOperand->regBlock = REG_XMM0;
}

void CDisassembler::GetSimdQwordModrmRegister()
{
    m_pOperand->flags |= OPF_REGISTER;
    GetSimdQwordModrm();
}

void CDisassembler::GetSimdOwordModrm()
{
    m_pOperand->size = GetXymmOperandSize();
    GetSimdModrm();
}

void CDisassembler::GetSimdOwordModrmRegister()
{
    m_pOperand->flags |= OPF_REGISTER;
    GetSimdOwordModrm();
}

void CDisassembler::GetSimdOwordModrmMem()
{
    m_pOperand->size = GetXymmOperandSize();
    OperandMemoryModrm();
}

void CDisassembler::GetMemory()
{
    OperandMemoryModrm();
}

void CDisassembler::GetByteMemory()
{
    m_pOperand->size = OPERANDSIZE_8;
    GetMemory();
}

void CDisassembler::GetWordMemory()
{
    m_pOperand->size = OPERANDSIZE_16;
    GetMemory();
}

void CDisassembler::GetDwordMemory()
{
    m_pOperand->size = OPERANDSIZE_32;
    GetMemory();
}

void CDisassembler::GetDwordOrQwordMemory()
{
    if (m_longmode)
    {
        m_pOperand->size = (m_inst_flags & INST_DATA32) ? OPERANDSIZE_32 : OPERANDSIZE_64;
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_32;
    }

    GetMemory();
}

void CDisassembler::GetWordOrDwordMemory()
{
    m_pOperand->size = (m_inst_flags & INST_DATA16) ? OPERANDSIZE_16 : OPERANDSIZE_32;
    GetMemory();
}

void CDisassembler::GetQwordMemory()
{
    m_pOperand->size = OPERANDSIZE_64;
    GetMemory();
}

void CDisassembler::GetTwordMemory()
{
    m_pOperand->size = OPERANDSIZE_80;
    GetMemory();
}

void CDisassembler::GetOwordMemory()
{
    m_pOperand->size = OPERANDSIZE_128;
    GetMemory();
}

void CDisassembler::GeteBXAndAL()
{
    m_pOperand->size = OPERANDSIZE_8;
    m_pOperand->type = OPERANDTYPE_MEMORY;
}

void CDisassembler::GetVexOpcodeOperand()
{
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = GetXymmBlock();
    m_pOperand->size = GetXymmOperandSize();

    m_pOperand->reg = ~GetVexvvvv() & (IsLongMode() ? 0x0F : 0x07);
    m_pOperand->regOrigin = REGORIGIN_VVVV;
}

void CDisassembler::GetVexOwordOpcode()
{
    GetVexOpcodeOperand();
}

void CDisassembler::GetVexWDQ()
{
    m_pOperand->size = GetWDQOperandSize();
    m_pOperand->type = OPERANDTYPE_REGISTER;
    m_pOperand->regBlock = REG_EAX;
    m_pOperand->regOrigin = REGORIGIN_VVVV;
    m_pOperand->reg = ~GetVexvvvv() & (IsLongMode() ? 0x0F : 0x07);
}

void CDisassembler::GetDwordRegOrByteMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = OPERANDSIZE_32;
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_8;
    }
}

void CDisassembler::GetDwordRegOrWordMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = OPERANDSIZE_32;
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_16;
    }
}

void CDisassembler::GetSimdOwordRegOrQwordMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_64;
    }
}

void CDisassembler::GetSimdOwordRegOrDwordMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_32;
    }
}

void CDisassembler::GetSimdOwordRegOrWordMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_16;
    }
}

void CDisassembler::GetSimdOwordRegOrByteMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = OPERANDSIZE_8;
    }
}

void CDisassembler::GetSimdOwordRegOrEigthMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = GetVexL() ? OPERANDSIZE_32 : OPERANDSIZE_16;
    }
}

void CDisassembler::GetSimdOwordRegOrQuarterMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = GetVexL() ? OPERANDSIZE_64 : OPERANDSIZE_32;
    }
}

void CDisassembler::GetSimdOwordRegOrHalfMemoryModrm()
{
    OperandModrm();

    if (m_pOperand->type == OPERANDTYPE_REGISTER)
    {
        m_pOperand->size = GetXymmOperandSize();
        m_pOperand->regBlock = GetXymmBlock();
    }
    else
    {
        m_pOperand->size = GetVexL() ? OPERANDSIZE_128 : OPERANDSIZE_64;
    }
}

#define NELEMS(x) (sizeof(x) / sizeof(x[0]))
#define _CHECK_HOOKS(x) { for(unsigned i = 0; i < NELEMS(x); i++) CheckHook(x[i], #x); }

bool CDisassembler::TablesHooked()
{
    try
    {
        // extra blocks to avoid Visual Studio 'BUG: Too many unnested loops incorrectly causes a C1061 compiler error'
        _CHECK_HOOKS(S_oneByteOpcodes_tbl);
        _CHECK_HOOKS(S_twoByteOpcodes_tbl);

        _CHECK_HOOKS(S_group_1_60_tbl);
        _CHECK_HOOKS(S_group_1_61_tbl);
        _CHECK_HOOKS(S_group_1_63_tbl);
        _CHECK_HOOKS(S_group_1_6d_tbl);
        _CHECK_HOOKS(S_group_1_6f_tbl);
        _CHECK_HOOKS(S_group_1_80_tbl);
        _CHECK_HOOKS(S_group_1_81_tbl);
        _CHECK_HOOKS(S_group_1_82_tbl);
        _CHECK_HOOKS(S_group_1_83_tbl);
        _CHECK_HOOKS(S_group_1_8e_tbl);
        _CHECK_HOOKS(S_group_1_8f_tbl);
        _CHECK_HOOKS(S_group_1_90_tbl);
        _CHECK_HOOKS(S_group_1_98_tbl);
        _CHECK_HOOKS(S_group_1_99_tbl);
        _CHECK_HOOKS(S_group_1_9c_tbl);
        _CHECK_HOOKS(S_group_1_9d_tbl);
        _CHECK_HOOKS(S_group_1_a5_tbl);
        _CHECK_HOOKS(S_group_1_a7_tbl);
        _CHECK_HOOKS(S_group_1_ab_tbl);
        _CHECK_HOOKS(S_group_1_ad_tbl);
        _CHECK_HOOKS(S_group_1_af_tbl);
        _CHECK_HOOKS(S_group_1_c0_tbl);
        _CHECK_HOOKS(S_group_1_c1_tbl);
        _CHECK_HOOKS(S_group_1_c2_tbl);
        _CHECK_HOOKS(S_group_1_c3_tbl);
        _CHECK_HOOKS(S_group_1_c6_tbl);
        _CHECK_HOOKS(S_group_1_c7_tbl);
        _CHECK_HOOKS(S_group_1_ca_tbl);
        _CHECK_HOOKS(S_group_1_cb_tbl);
        _CHECK_HOOKS(S_group_1_cf_tbl);
        _CHECK_HOOKS(S_group_1_d0_tbl);
        _CHECK_HOOKS(S_group_1_d1_tbl);
        _CHECK_HOOKS(S_group_1_d2_tbl);
        _CHECK_HOOKS(S_group_1_d3_tbl);
        _CHECK_HOOKS(S_group_1_d8_tbl);
        _CHECK_HOOKS(S_group_1_d9_tbl);
        _CHECK_HOOKS(S_group_1_da_tbl);
        _CHECK_HOOKS(S_group_1_db_tbl);
        _CHECK_HOOKS(S_group_1_dc_tbl);
        _CHECK_HOOKS(S_group_1_dd_tbl);
        _CHECK_HOOKS(S_group_1_de_tbl);
        _CHECK_HOOKS(S_group_1_df_tbl);
        _CHECK_HOOKS(S_group_1_e3_tbl);
        _CHECK_HOOKS(S_group_1_f6_tbl);
        _CHECK_HOOKS(S_group_1_f7_tbl);
        _CHECK_HOOKS(S_group_1_fe_tbl);
        _CHECK_HOOKS(S_group_1_ff_tbl);

        _CHECK_HOOKS(S_group_2_00_tbl);
        _CHECK_HOOKS(S_group_2_01_tbl);
        _CHECK_HOOKS(S_group_2_01_01_tbl);
        _CHECK_HOOKS(S_group_2_01_01_00_tbl);
        _CHECK_HOOKS(S_group_2_01_01_01_tbl);
        _CHECK_HOOKS(S_group_2_01_02_tbl);
        _CHECK_HOOKS(S_group_2_01_02_000_tbl);
        _CHECK_HOOKS(S_group_2_01_02_001_tbl);
        _CHECK_HOOKS(S_group_2_01_03_tbl);
        _CHECK_HOOKS(S_group_2_01_07_tbl);
        _CHECK_HOOKS(S_group_2_01_07_m3rm0_tbl);
        _CHECK_HOOKS(S_group_2_01_07_02_tbl);
        _CHECK_HOOKS(S_group_2_01_07_03_tbl);
        _CHECK_HOOKS(S_group_2_0d_tbl);
        _CHECK_HOOKS(S_group_2_0f_tbl);
        _CHECK_HOOKS(S_group_2_10_tbl);
        _CHECK_HOOKS(S_group_2_11_tbl);
        _CHECK_HOOKS(S_group_2_12_tbl);
        _CHECK_HOOKS(S_group_2_12_00_tbl);
        _CHECK_HOOKS(S_group_2_13_tbl);
        _CHECK_HOOKS(S_group_2_14_tbl);
        _CHECK_HOOKS(S_group_2_15_tbl);
        _CHECK_HOOKS(S_group_2_16_tbl);
        _CHECK_HOOKS(S_group_2_16_00_tbl);
        _CHECK_HOOKS(S_group_2_17_tbl);
        _CHECK_HOOKS(S_group_2_18_tbl);
        _CHECK_HOOKS(S_group_2_28_tbl);
        _CHECK_HOOKS(S_group_2_29_tbl);
        _CHECK_HOOKS(S_group_2_2a_tbl);
        _CHECK_HOOKS(S_group_2_2b_tbl);
        _CHECK_HOOKS(S_group_2_2c_tbl);
        _CHECK_HOOKS(S_group_2_2d_tbl);
        _CHECK_HOOKS(S_group_2_2e_tbl);
        _CHECK_HOOKS(S_group_2_2f_tbl);
        _CHECK_HOOKS(S_group_2_38_tbl);
        _CHECK_HOOKS(S_group_2_38_00_tbl);
        _CHECK_HOOKS(S_group_2_38_01_tbl);
        _CHECK_HOOKS(S_group_2_38_02_tbl);
        _CHECK_HOOKS(S_group_2_38_03_tbl);
        _CHECK_HOOKS(S_group_2_38_04_tbl);
        _CHECK_HOOKS(S_group_2_38_05_tbl);
        _CHECK_HOOKS(S_group_2_38_06_tbl);
        _CHECK_HOOKS(S_group_2_38_07_tbl);
        _CHECK_HOOKS(S_group_2_38_08_tbl);
        _CHECK_HOOKS(S_group_2_38_09_tbl);
        _CHECK_HOOKS(S_group_2_38_0a_tbl);
        _CHECK_HOOKS(S_group_2_38_0b_tbl);
        _CHECK_HOOKS(S_group_2_38_10_tbl);
        _CHECK_HOOKS(S_group_2_38_14_tbl);
        _CHECK_HOOKS(S_group_2_38_15_tbl);
        _CHECK_HOOKS(S_group_2_38_17_tbl);
        _CHECK_HOOKS(S_group_2_38_1c_tbl);
        _CHECK_HOOKS(S_group_2_38_1d_tbl);
        _CHECK_HOOKS(S_group_2_38_1e_tbl);
        _CHECK_HOOKS(S_group_2_38_20_tbl);
        _CHECK_HOOKS(S_group_2_38_21_tbl);
        _CHECK_HOOKS(S_group_2_38_22_tbl);
        _CHECK_HOOKS(S_group_2_38_23_tbl);
        _CHECK_HOOKS(S_group_2_38_24_tbl);
        _CHECK_HOOKS(S_group_2_38_25_tbl);
        _CHECK_HOOKS(S_group_2_38_28_tbl);
        _CHECK_HOOKS(S_group_2_38_29_tbl);
        _CHECK_HOOKS(S_group_2_38_2a_tbl);
        _CHECK_HOOKS(S_group_2_38_2b_tbl);
        _CHECK_HOOKS(S_group_2_38_30_tbl);
        _CHECK_HOOKS(S_group_2_38_31_tbl);
        _CHECK_HOOKS(S_group_2_38_32_tbl);
        _CHECK_HOOKS(S_group_2_38_33_tbl);
        _CHECK_HOOKS(S_group_2_38_34_tbl);
        _CHECK_HOOKS(S_group_2_38_35_tbl);
        _CHECK_HOOKS(S_group_2_38_37_tbl);
        _CHECK_HOOKS(S_group_2_38_38_tbl);
        _CHECK_HOOKS(S_group_2_38_39_tbl);
        _CHECK_HOOKS(S_group_2_38_3a_tbl);
        _CHECK_HOOKS(S_group_2_38_3b_tbl);
        _CHECK_HOOKS(S_group_2_38_3c_tbl);
        _CHECK_HOOKS(S_group_2_38_3d_tbl);
        _CHECK_HOOKS(S_group_2_38_3e_tbl);
        _CHECK_HOOKS(S_group_2_38_3f_tbl);
        _CHECK_HOOKS(S_group_2_38_40_tbl);
        _CHECK_HOOKS(S_group_2_38_41_tbl);
        _CHECK_HOOKS(S_group_2_38_82_tbl);
        _CHECK_HOOKS(S_group_2_38_db_tbl);
        _CHECK_HOOKS(S_group_2_38_dc_tbl);
        _CHECK_HOOKS(S_group_2_38_dd_tbl);
        _CHECK_HOOKS(S_group_2_38_de_tbl);
        _CHECK_HOOKS(S_group_2_38_df_tbl);
        _CHECK_HOOKS(S_group_2_38_f0_tbl);
        _CHECK_HOOKS(S_group_2_38_f1_tbl);
        _CHECK_HOOKS(S_group_2_3a_tbl);
        _CHECK_HOOKS(S_group_2_3a_08_tbl);
        _CHECK_HOOKS(S_group_2_3a_09_tbl);
        _CHECK_HOOKS(S_group_2_3a_0a_tbl);
        _CHECK_HOOKS(S_group_2_3a_0b_tbl);
        _CHECK_HOOKS(S_group_2_3a_0c_tbl);
        _CHECK_HOOKS(S_group_2_3a_0d_tbl);
        _CHECK_HOOKS(S_group_2_3a_0e_tbl);
        _CHECK_HOOKS(S_group_2_3a_0f_tbl);
        _CHECK_HOOKS(S_group_2_3a_14_tbl);
        _CHECK_HOOKS(S_group_2_3a_15_tbl);
        _CHECK_HOOKS(S_group_2_3a_16_tbl);
        _CHECK_HOOKS(S_group_2_3a_16_00_tbl);
        _CHECK_HOOKS(S_group_2_3a_17_tbl);
        _CHECK_HOOKS(S_group_2_3a_20_tbl);
        _CHECK_HOOKS(S_group_2_3a_21_tbl);
        _CHECK_HOOKS(S_group_2_3a_22_tbl);
        _CHECK_HOOKS(S_group_2_3a_22_00_tbl);
        _CHECK_HOOKS(S_group_2_3a_40_tbl);
        _CHECK_HOOKS(S_group_2_3a_41_tbl);
        _CHECK_HOOKS(S_group_2_3a_42_tbl);
        _CHECK_HOOKS(S_group_2_3a_44_tbl);
        _CHECK_HOOKS(S_group_2_3a_60_tbl);
        _CHECK_HOOKS(S_group_2_3a_61_tbl);
        _CHECK_HOOKS(S_group_2_3a_62_tbl);
        _CHECK_HOOKS(S_group_2_3a_63_tbl);
        _CHECK_HOOKS(S_group_2_3a_df_tbl);
        _CHECK_HOOKS(S_group_2_50_tbl);
        _CHECK_HOOKS(S_group_2_51_tbl);
        _CHECK_HOOKS(S_group_2_52_tbl);
        _CHECK_HOOKS(S_group_2_53_tbl);
        _CHECK_HOOKS(S_group_2_54_tbl);
        _CHECK_HOOKS(S_group_2_55_tbl);
        _CHECK_HOOKS(S_group_2_56_tbl);
        _CHECK_HOOKS(S_group_2_57_tbl);
        _CHECK_HOOKS(S_group_2_58_tbl);
        _CHECK_HOOKS(S_group_2_59_tbl);
        _CHECK_HOOKS(S_group_2_5a_tbl);
        _CHECK_HOOKS(S_group_2_5b_tbl);
        _CHECK_HOOKS(S_group_2_5c_tbl);
        _CHECK_HOOKS(S_group_2_5d_tbl);
        _CHECK_HOOKS(S_group_2_5e_tbl);
        _CHECK_HOOKS(S_group_2_5f_tbl);
        _CHECK_HOOKS(S_group_2_60_tbl);
        _CHECK_HOOKS(S_group_2_61_tbl);
        _CHECK_HOOKS(S_group_2_62_tbl);
        _CHECK_HOOKS(S_group_2_63_tbl);
        _CHECK_HOOKS(S_group_2_64_tbl);
        _CHECK_HOOKS(S_group_2_65_tbl);
        _CHECK_HOOKS(S_group_2_66_tbl);
        _CHECK_HOOKS(S_group_2_67_tbl);
        _CHECK_HOOKS(S_group_2_68_tbl);
        _CHECK_HOOKS(S_group_2_69_tbl);
        _CHECK_HOOKS(S_group_2_6a_tbl);
        _CHECK_HOOKS(S_group_2_6b_tbl);
        _CHECK_HOOKS(S_group_2_6c_tbl);
        _CHECK_HOOKS(S_group_2_6d_tbl);
        _CHECK_HOOKS(S_group_2_6e_tbl);
        _CHECK_HOOKS(S_group_2_6f_tbl);
        _CHECK_HOOKS(S_group_2_70_tbl);
        _CHECK_HOOKS(S_group_2_71_tbl);
        _CHECK_HOOKS(S_group_2_71_02_tbl);
        _CHECK_HOOKS(S_group_2_71_04_tbl);
        _CHECK_HOOKS(S_group_2_71_06_tbl);
        _CHECK_HOOKS(S_group_2_72_tbl);
        _CHECK_HOOKS(S_group_2_72_02_tbl);
        _CHECK_HOOKS(S_group_2_72_04_tbl);
        _CHECK_HOOKS(S_group_2_72_06_tbl);
        _CHECK_HOOKS(S_group_2_73_tbl);
        _CHECK_HOOKS(S_group_2_73_02_tbl);
        _CHECK_HOOKS(S_group_2_73_03_tbl);
        _CHECK_HOOKS(S_group_2_73_06_tbl);
        _CHECK_HOOKS(S_group_2_73_07_tbl);
        _CHECK_HOOKS(S_group_2_74_tbl);
        _CHECK_HOOKS(S_group_2_75_tbl);
        _CHECK_HOOKS(S_group_2_76_tbl);
        _CHECK_HOOKS(S_group_2_77_tbl);
        _CHECK_HOOKS(S_group_2_78_tbl);
        _CHECK_HOOKS(S_group_2_78_66_tbl);
        _CHECK_HOOKS(S_group_2_79_tbl);
        _CHECK_HOOKS(S_group_2_7c_tbl);
        _CHECK_HOOKS(S_group_2_7d_tbl);
        _CHECK_HOOKS(S_group_2_7e_tbl);
        _CHECK_HOOKS(S_group_2_7f_tbl);
        _CHECK_HOOKS(S_group_2_ae_tbl);
        _CHECK_HOOKS(S_group_2_ae_np_tbl);
        _CHECK_HOOKS(S_group_2_ae_f3_tbl);
        _CHECK_HOOKS(S_group_2_ae_f3_m3r0_tbl);
        _CHECK_HOOKS(S_group_2_ae_f3_m3r1_tbl);
        _CHECK_HOOKS(S_group_2_ae_f3_m3r2_tbl);
        _CHECK_HOOKS(S_group_2_ae_f3_m3r3_tbl);
        _CHECK_HOOKS(S_group_2_b8_tbl);
        _CHECK_HOOKS(S_group_2_ba_tbl);
        _CHECK_HOOKS(S_group_2_bc_tbl);
        _CHECK_HOOKS(S_group_2_bd_tbl);
        _CHECK_HOOKS(S_group_2_c2_tbl);
        _CHECK_HOOKS(S_group_2_c3_tbl);
        _CHECK_HOOKS(S_group_2_c4_tbl);
        _CHECK_HOOKS(S_group_2_c5_tbl);
        _CHECK_HOOKS(S_group_2_c6_tbl);
        _CHECK_HOOKS(S_group_2_c7_tbl);
        _CHECK_HOOKS(S_group_2_c7_01_tbl);
        _CHECK_HOOKS(S_group_2_c7_06_tbl);
        _CHECK_HOOKS(S_group_2_d0_tbl);
        _CHECK_HOOKS(S_group_2_d1_tbl);
        _CHECK_HOOKS(S_group_2_d2_tbl);
        _CHECK_HOOKS(S_group_2_d3_tbl);
        _CHECK_HOOKS(S_group_2_d4_tbl);
        _CHECK_HOOKS(S_group_2_d5_tbl);
        _CHECK_HOOKS(S_group_2_d6_tbl);
        _CHECK_HOOKS(S_group_2_d7_tbl);
        _CHECK_HOOKS(S_group_2_d8_tbl);
        _CHECK_HOOKS(S_group_2_d9_tbl);
        _CHECK_HOOKS(S_group_2_da_tbl);
        _CHECK_HOOKS(S_group_2_db_tbl);
        _CHECK_HOOKS(S_group_2_dc_tbl);
        _CHECK_HOOKS(S_group_2_dd_tbl);
        _CHECK_HOOKS(S_group_2_de_tbl);
        _CHECK_HOOKS(S_group_2_df_tbl);
        _CHECK_HOOKS(S_group_2_e0_tbl);
        _CHECK_HOOKS(S_group_2_e1_tbl);
        _CHECK_HOOKS(S_group_2_e2_tbl);
        _CHECK_HOOKS(S_group_2_e3_tbl);
        _CHECK_HOOKS(S_group_2_e4_tbl);
        _CHECK_HOOKS(S_group_2_e5_tbl);
        _CHECK_HOOKS(S_group_2_e6_tbl);
        _CHECK_HOOKS(S_group_2_e7_tbl);
        _CHECK_HOOKS(S_group_2_e8_tbl);
        _CHECK_HOOKS(S_group_2_e9_tbl);
        _CHECK_HOOKS(S_group_2_ea_tbl);
        _CHECK_HOOKS(S_group_2_eb_tbl);
        _CHECK_HOOKS(S_group_2_ec_tbl);
        _CHECK_HOOKS(S_group_2_ed_tbl);
        _CHECK_HOOKS(S_group_2_ee_tbl);
        _CHECK_HOOKS(S_group_2_ef_tbl);
        _CHECK_HOOKS(S_group_2_f0_tbl);
        _CHECK_HOOKS(S_group_2_f1_tbl);
        _CHECK_HOOKS(S_group_2_f2_tbl);
        _CHECK_HOOKS(S_group_2_f3_tbl);
        _CHECK_HOOKS(S_group_2_f4_tbl);
        _CHECK_HOOKS(S_group_2_f5_tbl);
        _CHECK_HOOKS(S_group_2_f6_tbl);
        _CHECK_HOOKS(S_group_2_f7_tbl);
        _CHECK_HOOKS(S_group_2_f8_tbl);
        _CHECK_HOOKS(S_group_2_f9_tbl);
        _CHECK_HOOKS(S_group_2_fa_tbl);
        _CHECK_HOOKS(S_group_2_fb_tbl);
        _CHECK_HOOKS(S_group_2_fc_tbl);
        _CHECK_HOOKS(S_group_2_fd_tbl);
        _CHECK_HOOKS(S_group_2_fe_tbl);

        _CHECK_HOOKS(S_xop_tbl);    // mmmmmpp
        _CHECK_HOOKS(S_xop_m8_tbl);
        _CHECK_HOOKS(S_xop_m9_tbl);
        _CHECK_HOOKS(S_xop_ma_tbl);

        _CHECK_HOOKS(S_vex_tbl);        // mmmmmpp
        _CHECK_HOOKS(S_vex_m1_np_tbl);  // 0f
        _CHECK_HOOKS(S_vex_m1_66_tbl);  // 66 0f
        _CHECK_HOOKS(S_vex_m1_f3_tbl);  // f3 0f
        _CHECK_HOOKS(S_vex_m1_f2_tbl);  // f2 0f
        _CHECK_HOOKS(S_vex_m2_np_tbl);  // 0f 38
        _CHECK_HOOKS(S_vex_m2_66_tbl);  // 66 0f 38
        _CHECK_HOOKS(S_vex_m2_f2_tbl);  // f2 0f 38
        _CHECK_HOOKS(S_vex_m2_f3_tbl);  // f3 0f 38
        _CHECK_HOOKS(S_vex_m3_np_tbl);  // 0f 3a
        _CHECK_HOOKS(S_vex_m3_66_tbl);  // 66 0f 3a
        _CHECK_HOOKS(S_vex_m3_f2_tbl);  // f2 0f 3a
        _CHECK_HOOKS(S_vex_m3_f3_tbl);  // f3 0f 3a

        _CHECK_HOOKS(S_vex_m1_np_12_tbl);
        _CHECK_HOOKS(S_vex_m1_np_16_tbl);
        _CHECK_HOOKS(S_vex_m1_np_77_tbl);
        _CHECK_HOOKS(S_vex_m1_np_ae_tbl);
        _CHECK_HOOKS(S_vex_m1_66_60_tbl);
        _CHECK_HOOKS(S_vex_m1_66_61_tbl);
        _CHECK_HOOKS(S_vex_m1_66_62_tbl);
        _CHECK_HOOKS(S_vex_m1_66_63_tbl);
        _CHECK_HOOKS(S_vex_m1_66_64_tbl);
        _CHECK_HOOKS(S_vex_m1_66_65_tbl);
        _CHECK_HOOKS(S_vex_m1_66_66_tbl);
        _CHECK_HOOKS(S_vex_m1_66_67_tbl);
        _CHECK_HOOKS(S_vex_m1_66_68_tbl);
        _CHECK_HOOKS(S_vex_m1_66_69_tbl);
        _CHECK_HOOKS(S_vex_m1_66_6a_tbl);
        _CHECK_HOOKS(S_vex_m1_66_6b_tbl);
        _CHECK_HOOKS(S_vex_m1_66_6c_tbl);
        _CHECK_HOOKS(S_vex_m1_66_6d_tbl);
        _CHECK_HOOKS(S_vex_m1_66_6e_tbl);
        _CHECK_HOOKS(S_vex_m1_66_70_tbl);
        _CHECK_HOOKS(S_vex_m1_66_71_tbl);
        _CHECK_HOOKS(S_vex_m1_66_72_tbl);
        _CHECK_HOOKS(S_vex_m1_66_73_tbl);
        _CHECK_HOOKS(S_vex_m1_66_74_tbl);
        _CHECK_HOOKS(S_vex_m1_66_75_tbl);
        _CHECK_HOOKS(S_vex_m1_66_76_tbl);
        _CHECK_HOOKS(S_vex_m1_66_7e_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d1_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d2_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d3_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d4_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d5_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d7_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d8_tbl);
        _CHECK_HOOKS(S_vex_m1_66_d9_tbl);
        _CHECK_HOOKS(S_vex_m1_66_da_tbl);
        _CHECK_HOOKS(S_vex_m1_66_db_tbl);
        _CHECK_HOOKS(S_vex_m1_66_dc_tbl);
        _CHECK_HOOKS(S_vex_m1_66_dd_tbl);
        _CHECK_HOOKS(S_vex_m1_66_de_tbl);
        _CHECK_HOOKS(S_vex_m1_66_df_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e0_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e1_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e2_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e3_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e4_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e5_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e8_tbl);
        _CHECK_HOOKS(S_vex_m1_66_e9_tbl);
        _CHECK_HOOKS(S_vex_m1_66_ea_tbl);
        _CHECK_HOOKS(S_vex_m1_66_eb_tbl);
        _CHECK_HOOKS(S_vex_m1_66_ec_tbl);
        _CHECK_HOOKS(S_vex_m1_66_ed_tbl);
        _CHECK_HOOKS(S_vex_m1_66_ee_tbl);
        _CHECK_HOOKS(S_vex_m1_66_ef_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f1_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f2_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f3_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f4_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f5_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f6_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f7_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f8_tbl);
        _CHECK_HOOKS(S_vex_m1_66_f9_tbl);
        _CHECK_HOOKS(S_vex_m1_66_fa_tbl);
        _CHECK_HOOKS(S_vex_m1_66_fb_tbl);
        _CHECK_HOOKS(S_vex_m1_66_fc_tbl);
        _CHECK_HOOKS(S_vex_m1_66_fd_tbl);
        _CHECK_HOOKS(S_vex_m1_66_fe_tbl);
        _CHECK_HOOKS(S_vex_m1_f3_10_tbl);
        _CHECK_HOOKS(S_vex_m1_f3_11_tbl);
        _CHECK_HOOKS(S_vex_m1_f3_70_tbl);
        _CHECK_HOOKS(S_vex_m1_f2_10_tbl);
        _CHECK_HOOKS(S_vex_m1_f2_11_tbl);
        _CHECK_HOOKS(S_vex_m1_f2_70_tbl);

        _CHECK_HOOKS(S_vex_m2_np_f3_tbl);
        _CHECK_HOOKS(S_vex_m2_66_00_tbl);
        _CHECK_HOOKS(S_vex_m2_66_01_tbl);
        _CHECK_HOOKS(S_vex_m2_66_02_tbl);
        _CHECK_HOOKS(S_vex_m2_66_03_tbl);
        _CHECK_HOOKS(S_vex_m2_66_04_tbl);
        _CHECK_HOOKS(S_vex_m2_66_05_tbl);
        _CHECK_HOOKS(S_vex_m2_66_06_tbl);
        _CHECK_HOOKS(S_vex_m2_66_07_tbl);
        _CHECK_HOOKS(S_vex_m2_66_08_tbl);
        _CHECK_HOOKS(S_vex_m2_66_09_tbl);
        _CHECK_HOOKS(S_vex_m2_66_0a_tbl);
        _CHECK_HOOKS(S_vex_m2_66_0b_tbl);
        _CHECK_HOOKS(S_vex_m2_66_18_tbl);
        _CHECK_HOOKS(S_vex_m2_66_19_tbl);
        _CHECK_HOOKS(S_vex_m2_66_1c_tbl);
        _CHECK_HOOKS(S_vex_m2_66_1d_tbl);
        _CHECK_HOOKS(S_vex_m2_66_1e_tbl);
        _CHECK_HOOKS(S_vex_m2_66_20_tbl);
        _CHECK_HOOKS(S_vex_m2_66_21_tbl);
        _CHECK_HOOKS(S_vex_m2_66_22_tbl);
        _CHECK_HOOKS(S_vex_m2_66_23_tbl);
        _CHECK_HOOKS(S_vex_m2_66_24_tbl);
        _CHECK_HOOKS(S_vex_m2_66_25_tbl);
        _CHECK_HOOKS(S_vex_m2_66_28_tbl);
        _CHECK_HOOKS(S_vex_m2_66_29_tbl);
        _CHECK_HOOKS(S_vex_m2_66_2a_tbl);
        _CHECK_HOOKS(S_vex_m2_66_2b_tbl);
        _CHECK_HOOKS(S_vex_m2_66_30_tbl);
        _CHECK_HOOKS(S_vex_m2_66_31_tbl);
        _CHECK_HOOKS(S_vex_m2_66_32_tbl);
        _CHECK_HOOKS(S_vex_m2_66_33_tbl);
        _CHECK_HOOKS(S_vex_m2_66_34_tbl);
        _CHECK_HOOKS(S_vex_m2_66_35_tbl);
        _CHECK_HOOKS(S_vex_m2_66_37_tbl);
        _CHECK_HOOKS(S_vex_m2_66_38_tbl);
        _CHECK_HOOKS(S_vex_m2_66_39_tbl);
        _CHECK_HOOKS(S_vex_m2_66_3a_tbl);
        _CHECK_HOOKS(S_vex_m2_66_3b_tbl);
        _CHECK_HOOKS(S_vex_m2_66_3c_tbl);
        _CHECK_HOOKS(S_vex_m2_66_3d_tbl);
        _CHECK_HOOKS(S_vex_m2_66_3e_tbl);
        _CHECK_HOOKS(S_vex_m2_66_3f_tbl);
        _CHECK_HOOKS(S_vex_m2_66_40_tbl);
        _CHECK_HOOKS(S_vex_m2_66_45_tbl);
        _CHECK_HOOKS(S_vex_m2_66_46_tbl);
        _CHECK_HOOKS(S_vex_m2_66_47_tbl);
        _CHECK_HOOKS(S_vex_m2_66_8c_tbl);
        _CHECK_HOOKS(S_vex_m2_66_8e_tbl);
        _CHECK_HOOKS(S_vex_m2_66_90_tbl);
        _CHECK_HOOKS(S_vex_m2_66_91_tbl);
        _CHECK_HOOKS(S_vex_m2_66_92_tbl);
        _CHECK_HOOKS(S_vex_m2_66_93_tbl);
        _CHECK_HOOKS(S_vex_m2_66_96_tbl);
        _CHECK_HOOKS(S_vex_m2_66_97_tbl);
        _CHECK_HOOKS(S_vex_m2_66_98_tbl);
        _CHECK_HOOKS(S_vex_m2_66_99_tbl);
        _CHECK_HOOKS(S_vex_m2_66_9a_tbl);
        _CHECK_HOOKS(S_vex_m2_66_9b_tbl);
        _CHECK_HOOKS(S_vex_m2_66_9c_tbl);
        _CHECK_HOOKS(S_vex_m2_66_9d_tbl);
        _CHECK_HOOKS(S_vex_m2_66_9e_tbl);
        _CHECK_HOOKS(S_vex_m2_66_9f_tbl);
        _CHECK_HOOKS(S_vex_m2_66_a6_tbl);
        _CHECK_HOOKS(S_vex_m2_66_a7_tbl);
        _CHECK_HOOKS(S_vex_m2_66_a8_tbl);
        _CHECK_HOOKS(S_vex_m2_66_a9_tbl);
        _CHECK_HOOKS(S_vex_m2_66_aa_tbl);
        _CHECK_HOOKS(S_vex_m2_66_ab_tbl);
        _CHECK_HOOKS(S_vex_m2_66_ac_tbl);
        _CHECK_HOOKS(S_vex_m2_66_ad_tbl);
        _CHECK_HOOKS(S_vex_m2_66_ae_tbl);
        _CHECK_HOOKS(S_vex_m2_66_af_tbl);
        _CHECK_HOOKS(S_vex_m2_66_b6_tbl);
        _CHECK_HOOKS(S_vex_m2_66_b7_tbl);
        _CHECK_HOOKS(S_vex_m2_66_b8_tbl);
        _CHECK_HOOKS(S_vex_m2_66_b9_tbl);
        _CHECK_HOOKS(S_vex_m2_66_ba_tbl);
        _CHECK_HOOKS(S_vex_m2_66_bb_tbl);
        _CHECK_HOOKS(S_vex_m2_66_bc_tbl);
        _CHECK_HOOKS(S_vex_m2_66_bd_tbl);
        _CHECK_HOOKS(S_vex_m2_66_be_tbl);
        _CHECK_HOOKS(S_vex_m2_66_bf_tbl);
        _CHECK_HOOKS(S_vex_m3_66_00_tbl);
        _CHECK_HOOKS(S_vex_m3_66_01_tbl);
        _CHECK_HOOKS(S_vex_m3_66_0e_tbl);
        _CHECK_HOOKS(S_vex_m3_66_0f_tbl);
        _CHECK_HOOKS(S_vex_m3_66_16_tbl);
        _CHECK_HOOKS(S_vex_m3_66_22_tbl);
        _CHECK_HOOKS(S_vex_m3_66_42_tbl);
        _CHECK_HOOKS(S_vex_m3_66_4c_tbl);
        _CHECK_HOOKS(S_xop_m9_00_tbl);
        _CHECK_HOOKS(S_xop_m9_01_tbl);
        _CHECK_HOOKS(S_xop_m9_02_tbl);
        _CHECK_HOOKS(S_xop_m9_12_tbl);
        _CHECK_HOOKS(S_xop_ma_12_tbl);

        return true;
    }
    catch (CTableException exc)
    {
        fprintf(stderr, "CTableException: bad table '%s'\n", exc.GetTablename());
        return false;
    }
}
//////////////////////////////////////////////////////////
#ifdef _DEBUG
const char* CInstructionData::OperandSizeString(e_OperandSize size)
{
    static const char* operandSizeStrings [] =
    {
        "8",
        "16",
        "32",
        "48",
        "64",
        "80",
        "128",
        "256",
        "NO SIZE"
    };

    return operandSizeStrings[size];
}

const char* CInstructionData::OperandTypeString(e_OperandType type)
{
    static const char* operandTypeStrings [] =
    {
        "REGISTER",
        "IMMEDIATE",
        "PCOFFSET",
        "MEMORY",
        "RIPRELATIVE",
        "NONE"
    };

    return operandTypeStrings[type];
}

const char* CInstructionData::RegisterString(e_Registers reg)
{
    static const char* registerStrings [] =
    {
        "REG_EAX",
        "REG_ECX",
        "REG_EDX",
        "REG_EBX",
        "REG_ESP",
        "REG_EBP",
        "REG_ESI",
        "REG_EDI",
        "REG_R8",
        "REG_R9",
        "REG_R10",
        "REG_R11",
        "REG_R12",
        "REG_R13",
        "REG_R14",
        "REG_R15",
        "REG_R16",
        "REG_R17",
        "REG_R18",
        "REG_R19",
        "REG_R20",
        "REG_R21",
        "REG_R22",
        "REG_R23",
        "REG_R24",
        "REG_R25",
        "REG_R26",
        "REG_R27",
        "REG_R28",
        "REG_R29",
        "REG_R30",
        "REG_R31",

        "REG_ST0",
        "REG_ST1",
        "REG_ST2",
        "REG_ST3",
        "REG_ST4",
        "REG_ST5",
        "REG_ST6",
        "REG_ST7",
        "REG_ST",

        "REG_MM0",
        "REG_MM1",
        "REG_MM2",
        "REG_MM3",
        "REG_MM4",
        "REG_MM5",
        "REG_MM6",
        "REG_MM7",

        "REG_XMM0",
        "REG_XMM1",
        "REG_XMM2",
        "REG_XMM3",
        "REG_XMM4",
        "REG_XMM5",
        "REG_XMM6",
        "REG_XMM7",
        "REG_XMM8",
        "REG_XMM9",
        "REG_XMM10",
        "REG_XMM11",
        "REG_XMM12",
        "REG_XMM13",
        "REG_XMM14",
        "REG_XMM15",
        "REG_XMM16",
        "REG_XMM17",
        "REG_XMM18",
        "REG_XMM19",
        "REG_XMM20",
        "REG_XMM21",
        "REG_XMM22",
        "REG_XMM23",
        "REG_XMM24",
        "REG_XMM25",
        "REG_XMM26",
        "REG_XMM27",
        "REG_XMM28",
        "REG_XMM29",
        "REG_XMM30",
        "REG_XMM31",

        "REG_CR0",
        "REG_CR1",
        "REG_CR2",
        "REG_CR3",
        "REG_CR4",
        "REG_CR5",
        "REG_CR6",
        "REG_CR7",
        "REG_CR8",
        "REG_CR9",
        "REG_CR10",
        "REG_CR11",
        "REG_CR12",
        "REG_CR13",
        "REG_CR14",
        "REG_CR15",

        "REG_DR0",
        "REG_DR1",
        "REG_DR2",
        "REG_DR3",
        "REG_DR4",
        "REG_DR5",
        "REG_DR6",
        "REG_DR7",
        "REG_DR8",
        "REG_DR9",
        "REG_DR10",
        "REG_DR11",
        "REG_DR12",
        "REG_DR13",
        "REG_DR14",
        "REG_DR15",

        "REG_ES",
        "REG_CS",
        "REG_SS",
        "REG_DS",
        "REG_FS",
        "REG_GS",

        "REG_YMM0",
        "REG_YMM1",
        "REG_YMM2",
        "REG_YMM3",
        "REG_YMM4",
        "REG_YMM5",
        "REG_YMM6",
        "REG_YMM7",
        "REG_YMM8",
        "REG_YMM9",
        "REG_YMM10",
        "REG_YMM11",
        "REG_YMM12",
        "REG_YMM13",
        "REG_YMM14",
        "REG_YMM15",
        "REG_YMM16",
        "REG_YMM17",
        "REG_YMM18",
        "REG_YMM19",
        "REG_YMM20",
        "REG_YMM21",
        "REG_YMM22",
        "REG_YMM23",
        "REG_YMM24",
        "REG_YMM25",
        "REG_YMM26",
        "REG_YMM27",
        "REG_YMM28",
        "REG_YMM29",
        "REG_YMM30",
        "REG_YMM31",
    };

    return registerStrings[reg];
}

void CInstructionData::ShowOperands()
{
    int numOperands = GetNumOperands();

    for (int i = 0; i < numOperands; i++)
    {
        e_OperandType opType = GetOperandType(i);

        printf("Operand%d(%s): ", (i + 1), OperandTypeString(opType));

        switch (opType)
        {
            case OPERANDTYPE_REGISTER:
                printf("Size(%s) ", OperandSizeString(GetOperandSize(i)));
                printf("Register(%s) ", RegisterString(GetRegister(i)));
                printf("\n");
                break;

            case OPERANDTYPE_IMMEDIATE:
                printf("Size(%s) ", OperandSizeString(GetOperandSize(i)));

                if (HasImmediate())
                {
                    printf("Immediate(%llx) ", GetImmediate());
                }
                else
                {
                    printf("Error: OPERANDTYPE == IMMEDIATE but HasImmediate() returns false");
                }

                printf("\n");
                break;

            case OPERANDTYPE_PCOFFSET:
                printf("Size(%s) ", OperandSizeString(GetOperandSize(i)));

                if (HasDisplacement())
                {
                    printf("Displacement(%llx) ", GetDisplacement());
                }
                else
                {
                    printf("Error: OPERANDTYPE == PCOFFSET but HasDisplacement() returns false");
                }

                printf("\n");
                break;

            case OPERANDTYPE_MEMORY:
                printf("Size(%s) ", OperandSizeString(GetOperandSize(i)));

                if (HasModrm())
                {
                    printf("Modrm(%02x) ", GetModrm());

                    if (HasSib())
                    {
                        printf("SIB(true) ");

                        if (HasScale())
                        {
                            printf("Scale(%d) ", GetSibScale());
                        }
                    }

                    if (HasBase())
                    {
                        printf("Base(%s) ", RegisterString(GetBaseRegister()));
                    }

                    if (HasIndex())
                    {
                        printf("Index(%s) ", RegisterString(GetIndexRegister()));
                    }

                    if (HasDisplacement())
                    {
                        printf("Displacement(%llx) ", GetDisplacement());
                    }
                }
                else if (HasImmediate())
                {
                    printf("Offset(%llx) ", GetImmediate());
                }
                else if (OperandHasIndex(i))
                {
                    printf("Index(%s) ", RegisterString(GetOperandIndexRegister(i)));
                }
                else
                {
                    printf("Error: OPERANDTYPE == MEMORY but HasModrm(), HasIndex(), and HasImmediate() returns false");
                }

                printf("\n");
                break;

            case OPERANDTYPE_RIPRELATIVE:
                printf("Size(%s) ", OperandSizeString(GetOperandSize(i)));

                if (HasDisplacement())
                {
                    printf("Displacement(%llx) ", GetDisplacement());
                }
                else
                {
                    printf("Error: OPERANDTYPE == RIPRELATIVE but HasDisplacement() returns false");
                }

                printf("\n");
                break;

            default:
                break;
        }
    }
}

void CInstructionData::ShowOpcodes()
{
    for (int i = 0; i < m_numOpcodes; i++)
    {
        if (i > 0) { printf(", "); }

        printf("Opcode%d(%02x), Offset(%02d)", i, GetOpcode(i), GetOpcodeOffset(i));
    }

    printf("\n");
}

#ifdef DISASSEMBLER_STANDALONE
void main(void)
{
    CDisassembler disassembler;

    unsigned count = 0;
    char line[256];

    while (gets(line))
    {
        count++;

        AMD_UINT8 instBuf[16];
        int len = 0;
        char* ptr = line;

#ifdef CONVEY_RIP
        AMD_UINT64 rip = strtoul(ptr, &ptr, 16);
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
#endif
#endif

#if 0
PVOIDMEMBERFUNC CTestDisassembler::S_EncodeOperandFnPtrs[] =
{
    /* OPRND_na */      NULL,
    /* OPRND_1  */      NULL,
    /* OPRND_AL */      NULL,
    /* OPRND_AX */      NULL,
    /* OPRND_eAX */     NULL,
    /* OPRND_Ap */      &CTestDisassembler::EncodeDwordOrFwordDirect,
    /* OPRND_CL */      NULL,
    /* OPRND_Cd */      &CTestDisassembler::EncodeControlRegister,
    /* OPRND_Dd */      &CTestDisassembler::EncodeDebugRegister,
    /* OPRND_DX */      NULL,
    /* OPRND_eDX */     NULL,
    /* OPRND_Eb */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Ew */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Ed */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Ev */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Ep */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Mt */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Gb */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Gw */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Gd */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Gq */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Gv */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Ib */      &CTestDisassembler::EncodeImmediateByte,
    /* OPRND_Iz */      &CTestDisassembler::EncodeWordOrDwordImmediate,
    /* OPRND_Iw */      &CTestDisassembler::EncodeImmediateWord,
    /* OPRND_Jb */      &CTestDisassembler::EncodeByteJump,
    /* OPRND_Jz */      &CTestDisassembler::EncodeWordOrDwordJump,
    /* OPRND_M  */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Mp */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Mq */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Ms */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Ob */      &CTestDisassembler::EncodeOffset,
    /* OPRND_Ov */      &CTestDisassembler::EncodeOffset,
    /* OPRND_Pq */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Pd */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Qq */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Rd */      &CTestDisassembler::EncodeModrmRm,
    /* OPRND_Sw */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Xb */      NULL,
    /* OPRND_Xv */      NULL,
    /* OPRND_Yb */      NULL,
    /* OPRND_Yv */      NULL,
    /* OPRND_breg */    &CTestDisassembler::EncodeByteRegFromOpcode,
    /* OPRND_vreg */    &CTestDisassembler::EncodeModrmRm,
    /* OPRND_ST */      &CTestDisassembler::EncodeSTReg,
    /* OPRND_ST0 */     &CTestDisassembler::EncodeST0Reg,
    /* OPRND_ST1 */     &CTestDisassembler::EncodeST1Reg,
    /* OPRND_ST2 */     &CTestDisassembler::EncodeST2Reg,
    /* OPRND_ST3 */     &CTestDisassembler::EncodeST3Reg,
    /* OPRND_ST4 */     &CTestDisassembler::EncodeST4Reg,
    /* OPRND_ST5 */     &CTestDisassembler::EncodeST5Reg,
    /* OPRND_ST6 */     &CTestDisassembler::EncodeST6Reg,
    /* OPRND_ST7 */     &CTestDisassembler::EncodeST7Reg,
    /* OPRND_Vps */     &CTestDisassembler::EncodeSimdOwordReg,
    /* OPRND_Vq */      &CTestDisassembler::EncodeSimdQwordReg,
    /* OPRND_Vss */     &CTestDisassembler::EncodeSimdDwordReg,
    /* OPRND_Wps */     &CTestDisassembler::EncodeModrm,
    /* OPRND_Wq */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Wss */     &CTestDisassembler::EncodeModrm,
    /* OPRND_Vpd */     &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Wpd */     &CTestDisassembler::EncodeModrm,
    /* OPRND_Vsd */     &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Wsd */     &CTestDisassembler::EncodeModrm,
    /* OPRND_Vx */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Wx */      &CTestDisassembler::EncodeModrm,
    /* OPRND_Vd */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_FPU_AX */  &CTestDisassembler::EncodeFpuAx,
    /* OPRND_Mw */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Md */      &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Iv */      &CTestDisassembler::EncodeWDQImmediate,
    /* OPRND_eBXAl */   NULL,
    /* OPRND_Ed_q */    &CTestDisassembler::EncodeDQModrm,
    /* OPRND_Pd_q */    &CTestDisassembler::EncodeMMXDQwordReg,
    /* OPRND_Vd_q */    &CTestDisassembler::EncodeSimdDQwordReg,
    /* OPRND_Gd_q */    &CTestDisassembler::EncodeDQRegFromReg,
    /* OPRND_Md_q */    &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_MwRv */    &CTestDisassembler::EncodeWordMemoryOrWDQRegModrm,
    /* OPRND_Mq_dq */   &CTestDisassembler::EncodeMemoryModrm,
    /* OPRND_Gz */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Nq */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Uq */      &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Ups */     &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Upd */     &CTestDisassembler::EncodeModrmReg,
    /* OPRND_Ux */      &CTestDisassembler::EncodeModrmReg,
}

class CEncodedBytes
{
    list <UINT8> m_Bytes;

public:
    CEncodedBytes(UINT8* pByte, ...)
    {
        va_list pArg;
        va_start(pArg, pByte);
        m_Bytes.push_back(pByte, pArg);
        va_end(pArg);
    }
};

list <CEncodeBytes> m_EncodedOperandBytes;
static list <CEncodedBytes> m_EncodedMemoryModrmBytes[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedRegisterModrmBytes[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedImmediateBytes[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedImmediateWords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedImmediateDwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedImmediateFwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedImmediateQwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedOffsetBytes[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedOffsetWords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedOffsetDwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedOffsetQwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedDisplacementBytes[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedDisplacementWords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedDisplacementDwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

static list <CEncodedBytes> m_EncodedDisplacementQwords[] =
{
    CEncodedBytes(0xf8),
    CEncodedBytes(0xC0, 0xFE, 0xED, 0xFA, 0xCE),
};

void CTestDisassembler::EncodeMemoryModrm()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedMemoryModrmBytes.begin();

    while (extract != m_EncodedMemoryModrmBytes.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeRegisterModrm()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedRegisterModrmBytes.begin();

    while (extract != m_EncodedRegisterModrmBytes.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeModrm()
{
    if (m_pOperand->flags & OPF_REGISTER)
    {
        EncodeRegisterModrm();
    }
    else
    {
        EncodeRegisterModrm();
        EncodeMemoryModrm();
    }
}

void CTestDisassembler::EncodeImmediateByte()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedImmediateBytes.begin();

    while (extract != m_EncodedImmediateBytes.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeImmediateWord()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedImmediateWords.begin();

    while (extract != m_EncodedImmediateWords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeImmediateDword()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedImmediateDwords.begin();

    while (extract != m_EncodedImmediateDwords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeImmediateFword()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedImmediateFwords.begin();

    while (extract != m_EncodedImmediateFwords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeImmediateQword()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedImmediateQwords.begin();

    while (extract != m_EncodedImmediateQwords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeDisplacementByte()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedDisplacementBytes.begin();

    while (extract != m_EncodedDisplacementBytes.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeDisplacementWord()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedDisplacementWords.begin();

    while (extract != m_EncodedDisplacementWords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeDisplacementDword()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedDisplacementDwords.begin();

    while (extract != m_EncodedDisplacementDwords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeDisplacementQword()
{
    insert_iterator <list <CEncodedBytes>> insert(m_EncodedOperandBytes, m_EncodedOperandBytes.end());
    std::list <CEncodedBytes>::iterator extract = m_EncodedDisplacementQwords.begin();

    while (extract != m_EncodedDisplacementQwords.end())
    {
        *insert++ = *extract++;
    }
}

void CTestDisassembler::EncodeWordOrDwordImmediate()
{
    if (m_inst_flags & (INST_DATA32 | INST_DATA64))
    {
        EncodeImmediateDword();
    }
    else
    {
        EncodeImmediateWord;
    }
}

void CTestDisassembler::EncodeWDQImmediate()
{
    if (m_inst_flags & INST_DATA64)
    {
        EncodeImmediateQword();
    }
    else if (m_inst_flags & INST_DATA32)
    {
        EncodeImmediateDword();
    }
    else
    {
        EncodeImmediateWord;
    }
}

void CTestDisassembler::EncodeByteJump()
{
    EncodeOffsetByte();
}

void CTestDisassembler::EncodeWordOrDwordJump()
{
    if (m_inst_flags & (INST_DATA32 | INST_DATA64))
    {
        EncodeOffsetDword();
    }
    else
    {
        EncodeOffsetWord();
    }
}

void CTestDisassembler::EncodeFpuAx()
{
    EncodeModrm();
}

void CTestDisassembler::EncodeDwordOrFwordDirect()
{
    (m_pOperand->size == OPERANDSIZE_48) ? EncodeImmediateFword() : EncodeImmediateDword();
}

void CTestDisassembler::EncodeOffset()
{
    if (m_inst_flags & INST_ADDR64)
    {
        EncodeImmediateQword();
    }
    else if (m_inst_flags & INST_ADDR32)
    {
        EncodeImmediateDword();
    }
    else
    {
        EncodeImmediateWord();
    }
}

void CTestDisassembler::SetMMXDwordReg()
{
    EncodeModrmRegByte();
}

void CTestDisassembler::EncodeMMXQwordReg()
{
    EncodeModrmRegByte();
}

void CTestDisassembler::EncodeMMXDQwordReg()
{
    EncodeModrmRegByte();
}

void CTestDisassembler::EncodeMMXQwordModrm()
{
    if (m_pOperand->flags & OPF_REGISTER)
    {
        EncodeRegisterModrm();
    }
    else
    {
        EncodeModrm();
    }
}

// this is for the Control Registers and Debug Registers
void CTestDisassembler::EncodeDwordOrQwordReg()
{
    EncodeModrmRm();
}

void CTestDisassembler::EncodeDebugRegister()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeControlRegister()
{
    EncodeModrmReg();

    if (IsSvmMode() && IsLastEncodedPrefix(PREFIX_LOCK))
    {
        // convert the LOCK prefix from a prefix byte to an opcode byte
        RemoveLastEncodedPrefix();
    }
}

void CTestDisassembler::EncodeWordSegmentRegister()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeST0Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST1Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST2Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST3Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST4Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST5Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST6Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeST7Reg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeSTReg()
{
    EncodeFpuModrm();
}

void CTestDisassembler::EncodeSimdReg()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeSimdDwordReg()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeSimdQwordReg()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeSimdDQwordReg()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeSimdOwordReg()
{
    EncodeModrmReg();
}

void CTestDisassembler::EncodeSimdDwordModrm()
{
    EncodeModrm();
}

void CTestDisassembler::EncodeSimdQwordModrm()
{
    EncodeModrm();
}

void CTestDisassembler::EncodeSimdOwordModrm()
{
    EncodeModrm();
}


void CTestDisassembler::EncodeWordMemoryOrWDQRegModrm()
{
    EncodeModrm();
}

void CTestDisassembler::EncodeFmacOpcodeOperand()
{
    EncodeFmacOpcodeOperand();  // last two opcode bytes at the end
}

void CTestDisassembler::EncodeInstructionBytes()
{
    for (i = 0; i < NUM_PREFIX_BYTES; i++)
    {
        if ((m_TestPrefixFlags & (1 << 1)) != 0)
        {
            EncodePrefixByte(m_TestPrefixBytes[i]);
        }

        cout << opcode;

        switch (operandBytes)
        {
        }
    }
}

void CTestDisassembler::EncodeInstructions()
{
    int opcode;

    // first do one opcode byte instructions
    for (opcode = 0; opcode < 256; opcode++)
    {
        m_opcode_table_ptr = &S_oneByteOpcodes_tbl[opcode];

        while (m_opcode_table_ptr->GetInfoPtr != NULL)
        {
            Inst_Info *(*pIndexRoutine)(CDisassembler * pThis) = m_opcode_table_ptr->GetInfoPtr;

            //          if( pIndexRoutine == GetSSEIndex )
            //              HandleExtraPrefixOpcode();
            //          else if( pIndexRoutine == GetNopXchgPauseIndex )
            //              HandleExtraRepOpcode();

            if ((m_opcode_table_ptr = (pIndexRoutine)(this)) == NULL)
            {
                break;
            }
        }

        // EncodePrefixBytes();
        // EncodeOperandBytes();
        // GenerateInstructionsBytes();
    }
}
#endif
