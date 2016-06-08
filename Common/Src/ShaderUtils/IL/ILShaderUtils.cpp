// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.

/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/ShaderUtils/IL/ILShaderUtils.cpp $
/// \version $Revision: #18 $
/// \brief  IL Utilities.

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/IL/ILShaderUtils.cpp#18 $
// Last checkin:   $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569084 $
//=====================================================================

#include <cassert>
#include <string>

#include "ILShaderUtils.h"

#include "ILParser.h"

// include file generates lots of warnings so disable them
#ifdef _WIN32
    #pragma warning( push )
    #pragma warning( disable : 4100 4127 4512 4324 )
#endif

#include "ILFormatDecode.hpp"

#include "il_ops_table.h"

#ifdef _WIN32
    #pragma warning( pop )
#endif


bool ILShaderUtils::IsDCL(ILOpCode opCode)
{
    switch (opCode)
    {
        // decls
        case IL_OP_DCLARRAY:
        case IL_DCL_CONST_BUFFER:
        case IL_OP_DCLDEF:
        case IL_OP_DEF:
        case IL_OP_DEFB:
        case IL_DCL_INDEXED_TEMP_ARRAY:
        case IL_DCL_INPUT:
        case IL_DCL_INPUT_PRIMITIVE:
        case IL_DCL_LITERAL:
        case IL_DCL_MAX_OUTPUT_VERTEX_COUNT:
        case IL_DCL_ODEPTH:
        case IL_DCL_OUTPUT_TOPOLOGY:
        case IL_DCL_OUTPUT:
        case IL_DCL_VPRIM:
        case IL_DCL_PERSIST:
        case IL_OP_DCL_SHARED_TEMP:
        case IL_OP_DCL_NUM_THREAD_PER_GROUP:
        case IL_DCL_MAX_THREAD_PER_GROUP:
        case IL_OP_DCL_TOTAL_NUM_THREAD_GROUP:
        case IL_DCL_NUM_ICP:
        case IL_DCL_NUM_OCP:
        case IL_DCL_NUM_INSTANCES:
        case IL_DCL_STREAM:
        case IL_DCL_GLOBAL_FLAGS:
        case IL_DCL_TS_DOMAIN:
        case IL_DCL_TS_PARTITION:
        case IL_DCL_TS_OUTPUT_PRIMITIVE:
        case IL_OP_DCLPI:
        case IL_OP_DCLPIN:
        case IL_OP_DCLPP:
        case IL_OP_DCLPT:
        case IL_DCL_RESOURCE:
        case IL_OP_DCLV:
        case IL_OP_DCLVOUT:

        // lds
        case IL_DCL_LDS:
        case IL_DCL_STRUCT_LDS:
        case IL_OP_DCL_LDS_SIZE_PER_THREAD:
        case IL_OP_DCL_LDS_SHARING_MODE:

        // gds
        case IL_DCL_GDS:
        case IL_DCL_STRUCT_GDS:

        // uav
        case IL_OP_DCL_UAV:
        case IL_OP_DCL_RAW_UAV:
        case IL_OP_DCL_STRUCT_UAV:
        case IL_OP_DCL_TYPELESS_UAV:
        case IL_OP_DCL_TYPED_UAV:
        case IL_OP_DCL_ARENA_UAV:

        // srv
        case IL_OP_DCL_RAW_SRV:
        case IL_OP_DCL_STRUCT_SRV:

        // virtual function/interface support
        case IL_OP_DCL_FUNCTION_BODY:
        case IL_OP_DCL_FUNCTION_TABLE:
        case IL_OP_DCL_INTERFACE_PTR:

        // isn't in IL spec but is defined in enum... ???
        case IL_DCL_MAX_TESSFACTOR:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsDCLInput(ILOpCode opCode)
{
    if (opCode == IL_DCL_INPUT)
    {
        return true;
    }

    return false;
}


bool ILShaderUtils::IsDCLOutput(ILOpCode opCode)
{
    if (opCode == IL_DCL_OUTPUT)
    {
        return true;
    }

    return false;
}


bool ILShaderUtils::IsDCLLiteral(ILOpCode opCode)
{
    if (opCode == IL_DCL_LITERAL)
    {
        return true;
    }

    return false;
}


bool ILShaderUtils::IsDCL_UAV(ILOpCode opCode)
{
    switch (opCode)
    {
        case IL_OP_DCL_UAV:
        case IL_OP_DCL_RAW_UAV:
        case IL_OP_DCL_STRUCT_UAV:
        case IL_OP_DCL_ARENA_UAV:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsInstruction(ILOpCode opCode)
{
    if (IsDCL(opCode) ||
        opCode == IL_OP_COMMENT ||
        // macros definitions are sort of instructions but for now
        // we will be ignoring macros so treat them as any other
        // DCL or DEF, i.e. not an instruction
        opCode == IL_OP_MACRODEF ||
        opCode == IL_OP_MACROEND)
    {
        return false;
    }

    //TODO( "check if these are instructions or not" )
    switch (opCode)
    {
        // phase
        case IL_OP_HS_CP_PHASE:
        case IL_OP_HS_FORK_PHASE:
        case IL_OP_HS_JOIN_PHASE:
        case IL_OP_ENDPHASE:
            assert(!"Instruction or not???");
            return false;

        default:
            break;
    }

    return true;
}


bool ILShaderUtils::IsFlowControlOp(ILOpCode opCode)
{
    switch (opCode)
    {
        // break
        case IL_OP_BREAK:
        case IL_OP_BREAKC:
        case IL_OP_BREAK_LOGICALZ:
        case IL_OP_BREAK_LOGICALNZ:

        // call
        case IL_OP_CALL:
        case IL_OP_CALLNZ:
        case IL_OP_CALL_LOGICALZ:
        case IL_OP_CALL_LOGICALNZ:

        // continue
        case IL_OP_CONTINUE:
        case IL_OP_CONTINUEC:
        case IL_OP_CONTINUE_LOGICALZ:
        case IL_OP_CONTINUE_LOGICALNZ:

        // subroutine
        case IL_OP_FUNC:
        case IL_OP_ENDFUNC:
        case IL_OP_RET:
        case IL_OP_RET_DYN:
        case IL_OP_RET_LOGICALNZ:
        case IL_OP_RET_LOGICALZ:

        // if
        case IL_OP_IFC:
        case IL_OP_IFNZ:
        case IL_OP_IF_LOGICALZ:
        case IL_OP_IF_LOGICALNZ:
        case IL_OP_ELSE:
        case IL_OP_ENDIF:

        // iteration
        case IL_OP_LOOP:
        case IL_OP_WHILE:
        case IL_OP_ENDLOOP:

        // switch
        case IL_OP_SWITCH:
        case IL_OP_CASE:
        case IL_OP_DEFAULT:
        case IL_OP_ENDSWITCH:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsFlowControlBlockOp(ILOpCode opCode)
{
    switch (opCode)
    {
        // if
        case IL_OP_IFC:
        case IL_OP_IFNZ:
        case IL_OP_IF_LOGICALZ:
        case IL_OP_IF_LOGICALNZ:
        case IL_OP_ELSE:
        case IL_OP_ENDIF:

        // iteration
        case IL_OP_LOOP:
        case IL_OP_WHILE:
        case IL_OP_ENDLOOP:

        // switch
        case IL_OP_SWITCH:

        //case IL_OP_CASE:
        //case IL_OP_DEFAULT:
        case IL_OP_ENDSWITCH:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::HasInstructionModifier(ILOpCode opCode)
{
    switch (opCode)
    {
        case IL_OP_MACRODEF:
        case IL_OP_MACROCALL:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::UsesStandardModifiers(ILOpCode opCode)
{
    switch (opCode)
    {
        case IL_DCL_RESOURCE:
            return false;

        case IL_OP_DCL_TYPELESS_UAV:
            return false;

        case IL_OP_DCL_TYPED_UAV:
            return false;

        case IL_OP_UAV_ARENA_LOAD:
        case IL_OP_UAV_ARENA_STORE:
        case IL_OP_UAV_RAW_LOAD:
        case IL_OP_SRV_RAW_LOAD:
        case IL_OP_SRV_STRUCT_LOAD:
            return false;

        // mdef should obey standard modifiers and pri and sec should always be
        // zero.... but they're not. If pri and sec are set, there are still no modifiers
        // following the mdef token. Therefore they must be used for something else
        // that the IL spec doesn't mention.
        case IL_OP_MACRODEF:
            return false;

        default:
            return true;
    }
}


/// This table is formed from the commented out sections
/// from ILParser.h in SC. I'm not sure why they are commented
/// out in SC or what the correct values 'should' be but the
/// values below serve our purposes.

// Notes from 28-Feb-2011 (rgorton)
// There are multiple places in the SC code where instruction parsing
// gets done; the attempt has been made to regularize, but was incomplete
// simply due to time, which is why some entries are commented out, etc.
// For the OPCODE name field, this is basically a convention wherein
// they decode to OPCODE<number of output registers><number of input registers>
// Example: dcl_output is OPCODE10, one output register, zero input registers
// The apparent extension here is that "LITn" means n literals.
// And the value numbering of these constants is completely arbitrary.

// So that the table below has a little more meaning, here's a copy.
// struct _key_words
// {
//   const char* text;
//   int lval;
//   int token;
//   int options;
//   int required;
// };
// The commented out parts seemingly used a different struct!

# define OPCODE00LIT1   2001
# define OPCODE10LIT4   2002
# define OPCODE01LIT4   2003
#define OPCODE00LIT2    2004
#define OPCODE01LIT1    2005
#define OPCODE_DCL_1TO3_INT   2006

const struct _key_words keywords_2[] =
{
    // 4.4.3 Adding dcldef cause CAL to infine loop below DisassembleHWShader.
    //  {"dcldef",                IL_OP_DCLDEF,               OPCODE10, NOFLAGS},
    // 4.4.4 Adding def cause CAL to execute an "int 3" breakpoint instruction.
    //  {"def",                   IL_OP_DEF,                  OPCODE_SPECIAL_DEF, 0,0,0},
    // 4.4.5   parse error, unexpected FLOAT_LITERAL
    //  {"defb",                  IL_OP_DEFB,                 OPCODE, 0,0,0},
    // 4.4.6
    {"dcl_indexed_temp_array",    IL_DCL_INDEXED_TEMP_ARRAY,  OPCODE01LIT1, NOFLAGS},
    // 4.4.7
    {"dcl_input",                 IL_DCL_INPUT,               OPCODE10, NOFLAGS},
    // 4.4.8 geometry shaders only
    //  {"dcl_input_primitive",
    // 4.4.9
    {"dcl_literal",               IL_DCL_LITERAL,             OPCODE01LIT4, NOFLAGS},
    // 4.4.10 geometry shaders only
    //  {"dcl_max_output",        IL_DCL_MAX_OUTPUT_VERTEX_COUNT, OPCODE_NO_OPS, 0,0,0},
    // 4.4.11 pixel shaders only
    //  {"dcl_odepth",            IL_DCL_ODEPTH,              OPCODE_NO_OPS, 0,0,0},
    // 4.4.12 geometry shaders only
    //  {"dcl_output_topology",   IL_DCL_OUTPUT_TOPOLOGY,     OPCODE_NO_OPS, 0,0,0},
    // 4.4.13
    {"dcl_output",                IL_DCL_OUTPUT,              OPCODE10,  NOFLAGS},
    // 4.4.14 geometry shaders only
    //  {"dcl_vprim",             IL_DCL_VPRIM,               OPCODE_NO_OPS, 0,0,0},
    // 4.4.15 boom hardware only
    //  {"dcl_persistent",        IL_DCL_PERSIST,             OPCODE_SPECIAL_OP, 0,0,0},
    // 4.4.16    parse error, unexpected $undefined.
    //  {"dcl_shared_temp",       IL_OP_DCL_SHARED_TEMP,      OPCODE_SPECIAL_OP, 0,0,0},
    // 4.4.17 R7xx only
    //  {"init_shared_registers", IL_OP_INIT_SR,              OPCODE_NO_OPS, 0,0,0},
    // No longer documented
    //  {"dcl_lds_sharing_mode",  IL_OP_DCL_LDS_SHARING_MODE, OPCODE_NO_OPS, 0,0,0},
    // 4.4.18
    {"dcl_num_thread_per_group",  IL_OP_DCL_NUM_THREAD_PER_GROUP, OPCODE_DCL_1TO3_INT, NOFLAGS},
    // 4.4.19 dcl_max_thread_per_group is in the IL Spec, but is missing from the comments in ILParser.cpp.
    {"dcl_max_thread_per_group",  IL_DCL_MAX_THREAD_PER_GROUP, OPCODE00LIT1, NOFLAGS},
    //  {"dcl_total_num_thread_group", IL_OP_DCL_TOTAL_NUM_THREAD_GROUP,
    //                                                         OPCODE_DCL_1TO3_INT, 0,0,0},
    // 4.4.21 hull shader only
    //  {"dcl_num_icp",           IL_DCL_NUM_ICP,             OPCODE, 0,0,0},
    // 4.4.22 hull shader only
    //  {"dcl_num_ocp",           IL_DCL_NUM_OCP,             OPCODE, 0,0,0},
    // 4.4.26 hull shader only
    //    dcl_ts_domain
    // 4.4.27 hull shader only
    //    dcl_ts_partition
    // 4.4.28 hull shader only
    //    dcl_ts_output_primitive
    // 4.4.29 hull shader only
    //    dcl_max_tessfactor
    // 4.4.30 pre4.0 pixel and vertex shaders only
    //  {"dclpi",                 IL_OP_DCLPI,                ENTER_DCL_STATE | OPCODE, 0,0,0},
    // 4.4.31 pixel shaders only
    //    dclpin
    // Missing from documentation
    //  {"dclpp",                 IL_OP_DCLPP,                OPCODE, 0,0,0},
    // 4.4.32 shader model 3 texture shaders only
    //  {"dclpt",                 IL_OP_DCLPT,                OPCODE_NO_OPS,  xmod_type,xmod_type},
    // 4.4.34 vertex shaders only
    //    dclv
    // 4.4.35 vertex shaders only
    //    dclvout
    // Missing from documenation
    //  {"initv",                 IL_OP_INITV,                OPCODE, 0,0,0},

    // section 4.5 - INPUT/OUTPUT instructions
    // 4.5.2 texture shaders only?
    //  {"kill",                  IL_OP_KILL,                 Kw_kill, 0,0,0},
    // 4.5.9 shader model 3 texture shaders only
    //  {"lod",                   IL_OP_LOD,                  Kw_lod, 0,0,0},
    // 4.5.11 Yamamo only
    //  {"memexport",             IL_OP_MEMEXPORT,            Kw_memexport, 0,0,0},
    // 4.5.12 R520 only
    //  {"scatter",               IL_OP_SCATTER,              Kw_scatter, 0,0,0},
    // 4.5.13 Yamamo only
    //  {"memimport",             IL_OP_MEMIMPORT,            Kw_memimport, 0,0,0},
    // 4.5.32 shader model 3 texture shaders only
    //  {"texld",                 IL_OP_TEXLD,                OPCODE, 0,0,0},
    // 4.5.33 shader model 3 texture shaders only
    //  {"texldb",                IL_OP_TEXLDB,               OPCODE, 0,0,0},
    // 4.5.34 shader model 3 texture shaders only
    //  {"texldd",                IL_OP_TEXLDD,               OPCODE, 0,0,0},
    // 4.5.35 shader model 3 texture shaders only
    //  {"texldms",               IL_OP_TEXLDMS,              OPCODE, 0,0,0},
    // 4.5.36 shader model 3 texture shaders only
    //  {"texweight",             IL_OP_TEXWEIGHT,            OPCODE, 0,0,0},
    //  {"lds_read_vec",          IL_OP_LDS_READ_VEC,         OPCODE, 0,0,0},
    //  {"lds_write_vec",         IL_OP_LDS_WRITE_VEC,        OPCODE, 0,0,0},
    {"fence",                     IL_OP_FENCE,                OPCODE00, NOFLAGS},

    // section 4.6 - Logical Operations
    // section 4.7 - Bit Operations
    // section 4.8 - Simple arithmetic instructions
    // Section 4.9 - Simple 64 bit Integer operations

    // section - 4.10, 4.12, 4.13, 4.14, 4.15 (Floating Point)
    // 4.13 Noise is listed as opcode reserved
    // noise takes special types
    //  {"noise",                 IL_OP_NOISE,                OPCODE,  xmod_type, xmod_type},
    // Not documented.
    //  {"project",               IL_OP_PROJECT,              OPCODE, 0,0},
    // 4.13 funny floating point operation on 3 component vectors.
    // There are control word flags, but I'm not going to worry about them here.
    {"reflect",                   IL_OP_REFLECT,              OPCODE12, NOFLAGS},

    // section 4.11, 4.12, 4.13, 4.15 (Double Precision)
    // The next 4 are not documented.  there are single precision versions.
    //  dexp
    //  dlog
    //  dsin
    //  dcos
    // section 4.17 - Conversions
    // section 4.18 - Move instructions
    // Section 4.20 - multi-media instructions
    // Section 4.21 - special instructions
    {"comment",                   IL_OP_COMMENT,              OPCODE00,  NOFLAGS},

    // Section 4.22 - LDS memory operations
    // Why lit2 here?  There are 4 bytes of size after the opcode.
    {"dcl_lds",                   IL_DCL_LDS,                 OPCODE00LIT2, NOFLAGS},
    // In fact I don't think the OPCODE field matters at all for ShaderDebugging.
    // We copy the text of the line whole & don't try to understand it.
    // Nor do we disassemble a byte stream using it (in which case it would matter).
    {"dcl_struct_lds",            IL_DCL_STRUCT_LDS,          OPCODE00, NOFLAGS},

    // Section 4.23 - GDS memory operations
    {"dcl_gds",                   IL_DCL_GDS,                 OPCODE00, NOFLAGS},
    {"dcl_struct_gds",            IL_DCL_STRUCT_GDS,          OPCODE00, NOFLAGS},

    // Section 4.24 - UAV memory operations
    {"dcl_uav",                   IL_OP_DCL_UAV,              OPCODE00,     NOFLAGS},
    {"dcl_raw_uav",               IL_OP_DCL_RAW_UAV,          OPCODE00,     NOFLAGS},
    {"dcl_struct_uav",            IL_OP_DCL_STRUCT_UAV,       OPCODE00LIT2, NOFLAGS},
    {"dcl_typeless_uav",          IL_OP_DCL_TYPELESS_UAV,     OPCODE00,     NOFLAGS},
    {"dcl_typed_uav",             IL_OP_DCL_TYPED_UAV,        OPCODE00LIT1, NOFLAGS},
    {"dcl_arena_uav",             IL_OP_DCL_ARENA_UAV,        OPCODE00,     NOFLAGS},

    // section 4.25.1
    {"dcl_function_body",         IL_OP_DCL_FUNCTION_BODY,    OPCODE00, NOFLAGS},
    {"dcl_function_table",        IL_OP_DCL_FUNCTION_TABLE,   OPCODE00, NOFLAGS},
    {"dcl_interface_ptr",         IL_OP_DCL_INTERFACE_PTR,    OPCODE00, NOFLAGS},
    {"fcall",                     IL_OP_FCALL,                OPCODE00, NOFLAGS},
};

const int key_word_count_2 = sizeof(keywords_2) / sizeof(_key_words);


bool ILShaderUtils::HasDestReg(IL_OpCode op,
                               IL mod)
{
    bool bFound = false;
    bool bHasDestReg = false;

    // get the opcode
    ILOpCode opCode = ILFormatDecode::GetOp((IL*)&op);

    // look in the SC table
    for (int i = 0; i < key_word_count && !bFound; i++)
    {
        if (opCode == keywords[i].lval)
        {
            switch (keywords[i].token)
            {
                case IOOPCODE10:
                case IOOPCODE11:
                case IOOPCODE12:
                case IOOPCODE13:
                case IOOPCODE14:
                case IOOPCODE15:
                case IOOPCODE16:
                case OPCODE10:
                case OPCODE11:
                case OPCODE12:
                case OPCODE13:
                case OPCODE14:
                    bHasDestReg = true;
                    bFound = true;
                    break;

                case OPCODE00:
                case OPCODE00L:
                case OPCODE01:
                case OPCODE01L:
                case OPCODE02:
                case OPCODE03:
                case OPCODE00INT:
                case OPCODE00FMT4:
                case Kw_defmacro:
                    bHasDestReg = false;
                    bFound = true;
                    break;

                case Kw_mcall:
                {
                    unsigned int nOut = (mod >> 16) & 0xffff;
                    // only handle up to one output for now
                    // TODO handle multiple outputs
                    assert(nOut < 2);
                    bHasDestReg = nOut > 0 ? true : false;
                    bFound = true;
                    break;
                }

                default:
                    assert(!"Didn't find opcode token description");
                    return false;
            }
        }
    }

    // look in our own extra table
    for (int i = 0; i < key_word_count_2 && !bFound; i++)
    {
        if (opCode == keywords_2[i].lval)
        {
            switch (keywords_2[i].token)
            {
                case IOOPCODE10:
                case IOOPCODE11:
                case IOOPCODE12:
                case IOOPCODE13:
                case IOOPCODE14:
                case IOOPCODE15:
                case IOOPCODE16:
                case OPCODE10:
                case OPCODE11:
                case OPCODE12:
                case OPCODE13:
                case OPCODE14:
                case OPCODE10LIT4:
                    bHasDestReg = true;
                    bFound = true;
                    break;

                case OPCODE00:
                case OPCODE00L:
                case OPCODE01:
                case OPCODE01L:
                case OPCODE02:
                case OPCODE03:
                case OPCODE00INT:
                case OPCODE00FMT4:
                case OPCODE00LIT1:
                case OPCODE01LIT4:
                case OPCODE00LIT2:
                case OPCODE01LIT1:
                case OPCODE_DCL_1TO3_INT:
                    bHasDestReg = false;
                    bFound = true;
                    break;

                default:
                    assert(!"Didn't find opcode token description");
                    return false;
            }
        }
    }

    if (!bFound)
    {
        assert(!"HasDestReg() - unable to find opcode");
        return 0;
    }

    return bHasDestReg;
}


unsigned int ILShaderUtils::GetDestRegCount(IL_OpCode op,
                                            IL mod)
{
    return HasDestReg(op, mod) ? 1 : 0;
}


unsigned int ILShaderUtils::GetSrcRegCount(IL_OpCode op,
                                           IL mod)
{
    bool bFound = false;
    unsigned int numSrcRegs = 0;

    // get the opcode
    ILOpCode opCode = ILFormatDecode::GetOp((IL*)&op);

    // look in the SC table
    for (int i = 0; i < key_word_count && !bFound; i++)
    {
        if (opCode == keywords[i].lval)
        {
            switch (keywords[i].token)
            {
                case IOOPCODE10:
                case OPCODE10:
                case OPCODE00:
                case OPCODE00L:
                case OPCODE00INT:
                case OPCODE00FMT4:
                case Kw_defmacro:
                    numSrcRegs = 0;
                    bFound = true;
                    break;

                case IOOPCODE11:
                case OPCODE11:
                case OPCODE01:
                case OPCODE01L:
                    numSrcRegs = 1;
                    bFound = true;
                    break;

                case IOOPCODE12:
                case OPCODE12:
                case OPCODE02:
                    numSrcRegs = 2;
                    bFound = true;
                    break;

                case IOOPCODE13:
                case OPCODE13:
                case OPCODE03:
                    numSrcRegs = 3;
                    bFound = true;
                    break;

                case IOOPCODE14:
                case OPCODE14:
                    numSrcRegs = 4;
                    bFound = true;
                    break;

                case IOOPCODE15:
                    numSrcRegs = 5;
                    bFound = true;
                    break;

                case IOOPCODE16:
                    numSrcRegs = 6;
                    bFound = true;
                    break;

                case Kw_mcall:
                {
                    numSrcRegs = mod & 0xffff;
                    bFound = true;
                    break;
                }

                default:
                    assert(!"Didn't find opcode token description");
                    return 0;
            }
        }
    }

    // look in our own extra table
    for (int i = 0; i < key_word_count_2 && !bFound; i++)
    {
        if (opCode == keywords_2[i].lval)
        {
            switch (keywords_2[i].token)
            {
                case IOOPCODE10:
                case OPCODE10:
                case OPCODE00:
                case OPCODE00L:
                case OPCODE00INT:
                case OPCODE00FMT4:
                case OPCODE00LIT1:
                case OPCODE10LIT4:
                case OPCODE00LIT2:
                case OPCODE_DCL_1TO3_INT:
                    numSrcRegs = 0;
                    bFound = true;
                    break;

                case IOOPCODE11:
                case OPCODE11:
                case OPCODE01:
                case OPCODE01L:
                case OPCODE01LIT4:
                case OPCODE01LIT1:
                    numSrcRegs = 1;
                    bFound = true;
                    break;

                case IOOPCODE12:
                case OPCODE12:
                case OPCODE02:
                    numSrcRegs = 2;
                    bFound = true;
                    break;

                case IOOPCODE13:
                case OPCODE13:
                case OPCODE03:
                    numSrcRegs = 3;
                    bFound = true;
                    break;

                case IOOPCODE14:
                case OPCODE14:
                    numSrcRegs = 4;
                    bFound = true;
                    break;

                case IOOPCODE15:
                    numSrcRegs = 5;
                    bFound = true;
                    break;

                case IOOPCODE16:
                    numSrcRegs = 6;
                    bFound = true;
                    break;

                default:
                    assert(!"Didn't find opcode token description");
                    return 0;
            }
        }
    }

    if (!bFound)
    {
        assert(!"GetSrcRegCount() - unable to find opcode");
        return 0;
    }

    // special cases -  adjust number of source regs as needed
    if (opCode == IL_DCL_CONST_BUFFER && op.bits.pri_modifier_present == 1)
    {
        // immediate constant buffer so no source reg
        numSrcRegs = 0;
    }

    return numSrcRegs;
}

/*
int ILShaderUtils::GetOperandLength( IL* pToken )
{
//#pragma TODO( "implement this correctly" )
   return( ILFormatDecode::SrcTokenLength( pToken ) +
           ILFormatDecode::DstTokenLength( pToken ) +
           ILFormatDecode::OpcodeTokenLength( pToken ) );
}


int ILShaderUtils::GetComponentMask( D3D10_SB_4_COMPONENT_NAME component )
{
   switch ( component )
   {
   case D3D10_SB_4_COMPONENT_X:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
   case D3D10_SB_4_COMPONENT_Y:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;
   case D3D10_SB_4_COMPONENT_Z:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;
   case D3D10_SB_4_COMPONENT_W:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_W;
   default:                      return 0;
   }
}
*/

std::string ILShaderUtils::GetOpCodeName(ILOpCode opCode)
{
    for (int i = 0; i < key_word_count; i++)
    {
        if (opCode == keywords[i].lval)
        {
            return std::string(keywords[i].text);
        }
    }

    // And for the odd case where it does not appear in the sc table
    // (fence), search our keywords table
    for (int i = 0; i < key_word_count_2; i++)
    {
        if (opCode == keywords_2[i].lval)
        {
            return std::string(keywords_2[i].text);
        }
    }

    assert(!"Didn't find opcode");
    return "ERROR";
}


ILRegType ILShaderUtils::GetOpCodeDestType(IL_Dst dst)
{
    return ILFormatDecode::RegisterType(&dst);
}


ILRegType ILShaderUtils::GetOpCodeDestType(IL token)
{
    IL_Dst* dst = ILFormatDecode::AsDest(&token);
    ILRegType destType = GetOpCodeDestType(*dst);

    return destType;
}


ILRegType ILShaderUtils::GetOpCodeSourceType(IL_Src src)
{
    return ILFormatDecode::RegisterType(&src);
}


ILRegType ILShaderUtils::GetOpCodeSourceType(IL token)
{
    IL_Src* src = ILFormatDecode::AsSource(&token);
    ILRegType srcType = GetOpCodeSourceType(*src);

    return srcType;
}


bool ILShaderUtils::IsGlobalStore(ILOpCode opcode)
{
    switch (opcode)
    {
        case IL_OP_UAV_STORE:
        case IL_OP_UAV_RAW_STORE:
        case IL_OP_UAV_STRUCT_STORE:
        case IL_OP_UAV_ARENA_STORE:
        case IL_OP_UAV_BYTE_STORE:
        case IL_OP_UAV_SHORT_STORE:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsGlobalLoad(ILOpCode opcode)
{
    switch (opcode)
    {
        case IL_OP_UAV_LOAD:
        case IL_OP_UAV_RAW_LOAD:
        case IL_OP_UAV_STRUCT_LOAD:
        case IL_OP_UAV_ARENA_LOAD:
        case IL_OP_UAV_BYTE_LOAD:
        case IL_OP_UAV_SHORT_LOAD:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsGlobalMemAccessOp(ILOpCode code)
{
    return ILShaderUtils::IsGlobalStore(code)    ||
           ILShaderUtils::IsGlobalLoad(code)     ||
           ILShaderUtils::IsGlobalAtomicOp(code) ||
           ILShaderUtils::IsSampleOp(code)       ||
           ILShaderUtils::IsFetch4Op(code);
}


bool ILShaderUtils::IsGlobalAtomicOp(ILOpCode opcode)
{
    switch (opcode)
    {
        case IL_OP_UAV_ADD:
        case IL_OP_UAV_SUB:
        case IL_OP_UAV_RSUB:
        case IL_OP_UAV_MIN:
        case IL_OP_UAV_MAX:
        case IL_OP_UAV_UMIN:
        case IL_OP_UAV_UMAX:
        case IL_OP_UAV_AND:
        case IL_OP_UAV_OR:
        case IL_OP_UAV_XOR:
        case IL_OP_UAV_CMP:
        case IL_OP_UAV_READ_ADD:
        case IL_OP_UAV_READ_SUB:
        case IL_OP_UAV_READ_RSUB:
        case IL_OP_UAV_READ_MIN:
        case IL_OP_UAV_READ_MAX:
        case IL_OP_UAV_READ_UMIN:
        case IL_OP_UAV_READ_UMAX:
        case IL_OP_UAV_READ_AND:
        case IL_OP_UAV_READ_OR:
        case IL_OP_UAV_READ_XOR:
        case IL_OP_UAV_READ_XCHG:
        case IL_OP_UAV_READ_CMP_XCHG:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsSampleOp(ILOpCode opcode)
{
    switch (opcode)
    {
        case IL_OP_SAMPLE:
        case IL_OP_SAMPLE_B:
        case IL_OP_SAMPLE_G:
        case IL_OP_SAMPLE_L:
        case IL_OP_SAMPLE_C:
        case IL_OP_SAMPLE_C_LZ:
        case IL_OP_SAMPLE_C_L:
        case IL_OP_SAMPLE_C_G:
        case IL_OP_SAMPLE_C_B:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsFetch4Op(ILOpCode opcode)
{
    switch (opcode)
    {
        case IL_OP_FETCH4:
        case IL_OP_FETCH4_C:
        case IL_OP_FETCH4_PO:
        case IL_OP_FETCH4_PO_C:
            return true;

        default:
            return false;
    }
}


bool ILShaderUtils::IsReadAtomicOp(ILOpCode opcode)
{
    switch (opcode)
    {
        // LDS atomic ops that return a value
        case IL_OP_LDS_READ_ADD:
        case IL_OP_LDS_READ_AND:
        case IL_OP_LDS_READ_CMP_XCHG:
        case IL_OP_LDS_READ_DEC:
        case IL_OP_LDS_READ_INC:
        case IL_OP_LDS_READ_MAX:
        case IL_OP_LDS_READ_MIN:
        case IL_OP_LDS_READ_MSKOR:
        case IL_OP_LDS_READ_OR:
        case IL_OP_LDS_READ_RSUB:
        case IL_OP_LDS_READ_SUB:
        case IL_OP_LDS_READ_UMAX:
        case IL_OP_LDS_READ_UMIN:
        case IL_OP_LDS_READ_XCHG:
        case IL_OP_LDS_READ_XOR:

        case IL_OP_LDS_READ_ADD_B64:
        case IL_OP_LDS_READ_AND_B64:
        case IL_OP_LDS_READ_CMP_XCHG_B64:
        case IL_OP_LDS_READ_DEC_B64:
        case IL_OP_LDS_READ_INC_B64:
        case IL_OP_LDS_READ_MAX_B64:
        case IL_OP_LDS_READ_MIN_B64:
        case IL_OP_LDS_READ_MSKOR_B64:
        case IL_OP_LDS_READ_OR_B64:
        case IL_OP_LDS_READ_RSUB_B64:
        case IL_OP_LDS_READ_SUB_B64:
        case IL_OP_LDS_READ_UMAX_B64:
        case IL_OP_LDS_READ_UMIN_B64:
        case IL_OP_LDS_READ_XCHG_B64:
        case IL_OP_LDS_READ_XOR_B64:

        // GDS atomic ops that return a value
        case IL_OP_GDS_READ_ADD:
        case IL_OP_GDS_READ_AND:
        case IL_OP_GDS_READ_CMP_XCHG:
        case IL_OP_GDS_READ_DEC:
        case IL_OP_GDS_READ_INC:
        case IL_OP_GDS_READ_MAX:
        case IL_OP_GDS_READ_MIN:
        case IL_OP_GDS_READ_MSKOR:
        case IL_OP_GDS_READ_OR:
        case IL_OP_GDS_READ_RSUB:
        case IL_OP_GDS_READ_SUB:
        case IL_OP_GDS_READ_UMAX:
        case IL_OP_GDS_READ_UMIN:
        case IL_OP_GDS_READ_XCHG:
        case IL_OP_GDS_READ_XOR:

        case IL_OP_GDS_READ_ADD_B64:
        case IL_OP_GDS_READ_AND_B64:
        case IL_OP_GDS_READ_CMP_XCHG_B64:
        case IL_OP_GDS_READ_DEC_B64:
        case IL_OP_GDS_READ_INC_B64:
        case IL_OP_GDS_READ_MAX_B64:
        case IL_OP_GDS_READ_MIN_B64:
        case IL_OP_GDS_READ_MSKOR_B64:
        case IL_OP_GDS_READ_OR_B64:
        case IL_OP_GDS_READ_RSUB_B64:
        case IL_OP_GDS_READ_SUB_B64:
        case IL_OP_GDS_READ_UMAX_B64:
        case IL_OP_GDS_READ_UMIN_B64:
        case IL_OP_GDS_READ_XCHG_B64:
        case IL_OP_GDS_READ_XOR_B64:

        // UAV atomic ops that return a value
        case IL_OP_UAV_READ_ADD:
        case IL_OP_UAV_READ_AND:
        case IL_OP_UAV_READ_CMP_XCHG:
        case IL_OP_UAV_READ_MAX:
        case IL_OP_UAV_READ_MIN:
        case IL_OP_UAV_READ_OR:
        case IL_OP_UAV_READ_RSUB:
        case IL_OP_UAV_READ_SUB:
        case IL_OP_UAV_READ_UDEC:
        case IL_OP_UAV_READ_UINC:
        case IL_OP_UAV_READ_UMAX:
        case IL_OP_UAV_READ_UMIN:
        case IL_OP_UAV_READ_XCHG:
        case IL_OP_UAV_READ_XOR:

        // These are in the UAV section of the IL spec yet seem to resolve
        // to GDS instructions in the ISA. Either way, they're atomic and
        // return a value.
        case IL_OP_APPEND_BUF_ALLOC:
        case IL_OP_APPEND_BUF_CONSUME:
            return true;

        default:
            return false;
    }
}
